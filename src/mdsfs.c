#ifdef HAVE_CONFIG_H
	#include <config.h>
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <glib.h>
#include <glib/gi18n.h>

#include "mdsfs.h"

MDSFS *mdsfs;

static int mdsfs_getattr(const char *path, struct stat *stbuf)
{
	MDSINode *inode;

	memset(stbuf, 0, sizeof(struct stat));

	inode = mdsfs_inode_find_with_path(mdsfs, path);
	if (!inode)
		return -ENOENT;

	stbuf->st_ino = inode->id;
	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_mode = inode->mode;
	stbuf->st_size = inode->size;
	stbuf->st_atime = inode->atime;
	stbuf->st_mtime = inode->mtime;
	stbuf->st_ctime = inode->ctime;

	if (inode->id == 0)
		stbuf->st_nlink = 2;
	else
		stbuf->st_nlink = 1;

	return 0;
}

static int mdsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	off_t count;
	off_t i;
	gchar name[256];
	MDSINode *inode;

	inode = mdsfs_inode_list_with_path(mdsfs, path, &count);
	if (!inode)
		return -ENOENT;

	/* Directory list */
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for (i = 0; i < count; ++i) {
		encryption_decode_to_data_from_data(name, 256, (inode+i)->name, 256);
		filler(buf, name, NULL, 0);
		//filler(buf, (inode+i)->name, NULL, 0);
	}

    return 0;
}

static int mdsfs_open(const char *path, struct fuse_file_info *fi)
{
	if ((fi->flags & 3) != O_RDONLY)
		return -EACCES;

	return 0;
}

static int mdsfs_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
	MDSINode *inode;

	inode = mdsfs_inode_find_with_path(mdsfs, path);
	if (!inode)
		return -ENOENT;

	if (offset < inode->size) {
		if (offset + size > inode->size)
			size = inode->size - offset;

		encryption_decode_to_data_with_offset_from_data(buf, offset, size, mdsfs->data + inode->offset + offset, inode->length);
//		encryption_decode_to_data_from_data(buf, size, mdsfs->data + inode->offset + offset, inode->length);
//		memcpy(buf, mdsfs->data + inode->offset + offset, size);
	} else {
		size = 0;
	}

	return size;
}

static struct fuse_operations mdsfs_op = {
	.getattr    = mdsfs_getattr,
	.readdir    = mdsfs_readdir,
	.open       = mdsfs_open,
	.read       = mdsfs_read,
};

int main(int argc, char *argv[])
{
	struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
	guint8 *key; 

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if (argc < 3 || argc > 4) {
		goto usage;
	}

	if (!g_file_test(argv[1], G_FILE_TEST_EXISTS)) {
		printf("%s doesn't exist.\n", argv[1]);
		return 1;
	}

	if (!g_file_test(argv[2], G_FILE_TEST_EXISTS)) {
		printf("%s doesn't exist.\n", argv[2]);
		return 1;
	}

	if (argc == 4) {
		if (!mdsfs_lockmgr_check_key(argv[3]))
			return 1;

		/* Initializing blowfish */
		key = mdsfs_misc_hexstrings_to_binary(argv[3]);
	}

	/* args */
	fuse_opt_add_arg(&args, argv[0]);
	fuse_opt_add_arg(&args, argv[2]);

	/* Initiallizing */
	mdsfs = mdsfs_core_open(argv[1], key);

	/* main of FUSE */
	return fuse_main(args.argc, args.argv, &mdsfs_op, NULL);

usage:
	printf("Usage: mdsfs [image file] [mount point] {key}\n");
	return 1;
}
