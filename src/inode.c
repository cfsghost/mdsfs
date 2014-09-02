#include <glib.h>
#include <glib/gi18n.h>
#include <strings.h>
#include <sys/stat.h>

#include "mdsfs.h"
#include "inode.h"

MDSINode *
mdsfs_inode_new(const gchar *name)
{
	MDSINode *inode;

	/* Allocate a new inode */
	inode = (MDSINode *)g_new0(MDSINode, 1);
	strncpy(inode->name, name, strlen(name));
	encryption_encode_to_data_from_data(inode->name, inode->name, 256);

	return inode;
}

inline MDSINode *
mdsfs_inode_openchild(MDSFS *mdsfs, MDSINode *inode)
{
	if (inode->child_id >= mdsfs->header->count)
		return NULL;

	return &mdsfs->inode[inode->child_id];
}

inline MDSINode *
mdsfs_inode_openparent(MDSFS *mdsfs, MDSINode *inode)
{
	if (inode->parent_id >= mdsfs->header->count)
		return NULL;

	return &mdsfs->inode[inode->parent_id];
}

inline MDSINode *
mdsfs_inode_set_position(MDSFS *mdsfs, off_t id)
{
	if (id >= mdsfs->header->count)
		return NULL;

	return &mdsfs->inode[id];
}

MDSINode *
mdsfs_inode_find_with_name(MDSFS *mdsfs, MDSINode *pos, const gchar *name, off_t limit_count)
{
	guint8 encode_name[256] = {0};
	off_t i = pos->id;
	off_t n = 0;

	/* find inode with encoded name */
	strncpy(encode_name, name, strlen(name));
	encryption_encode_to_data_from_data(encode_name, encode_name, 256);

	while(i < mdsfs->header->count && n <= limit_count) {
		if (memcmp(encode_name, mdsfs->inode[i].name, 256) == 0)
			return &mdsfs->inode[i];

		++i;
		++n;
	}

	return NULL;
}

MDSINode *
mdsfs_inode_find_with_path(MDSFS *mdsfs, const gchar *path)
{
	MDSINode *inode;
	gchar *dirpath;
	gchar *part;
	off_t count;

	/* root inode is the first */
	if (strcmp(path, "/") == 0)
		return &mdsfs->inode[0];

	/* from root directory */
	count = mdsfs->inode[0].count;
	inode = &mdsfs->inode[1];

	/* each part */
	dirpath = g_strdup(path);
	part = strtok(dirpath, "/");
	while(part) {
		/* find inode with filename in the directory */
		inode = mdsfs_inode_find_with_name(mdsfs, inode, part, count);
		if (!inode)
			break;

		part = strtok(NULL, "/");
		if (part) {
			/* path doesn't exist */
			if (inode->child_id == 0 || inode->count == 0) {
				inode = NULL;
				break;
			}

			/* enter directory */
			count = inode->count;
			inode = mdsfs_inode_openchild(mdsfs, inode);
			if (inode == NULL)
				break;
		} else {
			/* end of path */
			break;
		}
	}

	g_free(dirpath);
	return inode;
}

MDSINode *
mdsfs_inode_list_with_path(MDSFS *mdsfs, const gchar *path, off_t *count)
{
	MDSINode *inode;

	/* find inode of target path */
	inode = mdsfs_inode_find_with_path(mdsfs, path);
	if (!inode)
		goto noinode;

	/* enter directory */
	*count = inode->count;
	inode = mdsfs_inode_openchild(mdsfs, inode);
	if (!inode)
		goto noinode;

	return inode;

noinode:
	*count = 0;
	return NULL;
}

