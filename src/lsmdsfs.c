#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>

#include "mdsfs.h"
#include "inode.h"
#include "lsmdsfs.h"

void
lsmdsfs_list_all(MDSFS *mdsfs)
{
	gchar orig_name[256] = {0};
	off_t i;

	for (i = 0; i < mdsfs->header->count; ++i) {
		printf("%10d\t", mdsfs->inode[i].id);

		encryption_decode_to_data_from_data(orig_name, 256, mdsfs->inode[i].name, 256);
		printf("%s\n", orig_name);
	}
}

void
lsmdsfs_list(MDSFS *mdsfs, const gchar *path)
{
	gchar orig_name[256] = {0};
	off_t count;
	off_t i;
	MDSINode *inode = NULL;

	inode = mdsfs_inode_list_with_path(mdsfs, path, &count);
	if (inode) {
		for (i = 0; i < count; ++i) {
			printf("%10d\t", (inode+i)->id);
			encryption_decode_to_data_from_data(orig_name, 256, (inode+i)->name, 256);
			printf("%s\n", orig_name);
		}
	}
}

int main(int argc, char *argv[])
{
	MDSFS *mdsfs;
	gchar *path;
	guint8 *key; 

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if (argc < 4 || argc > 5) {
		goto usage;
	}

	if (!mdsfs_lockmgr_check_key(argv[1]))
		return 1;

	if (!g_file_test(argv[3], G_FILE_TEST_EXISTS)) {
		printf("%s doesn't exist.\n", argv[3]);
		return 1;
	}

	/* convert hex strings to binary key */
	key = mdsfs_misc_hexstrings_to_binary(argv[1]);

	/* Initiallizing */
	mdsfs = mdsfs_core_open(argv[3], key);
#if 0
	mdsfs = (MDSFS *)g_new0(MDSFS, 1);
	mdsfs_io_open(mdsfs, src);
	mdsfs->map = mdsfs_io_mmap(mdsfs->fd, mdsfs->length);
	if (mdsfs->map) {
		mdsfs->header = (MDSHeader *)mdsfs->map;
		mdsfs->key = (MDSINode *)(mdsfs->map + sizeof(MDSHeader));
		mdsfs->inode = (MDSINode *)(mdsfs->map + sizeof(MDSHeader));
	}
#endif
	/* options */
	if (strcmp(argv[2], "-f") == 0) {
		if (argc != 5)
			goto usage;

		path = argv[4];
		lsmdsfs_list(mdsfs, path);
	} else if (strcmp(argv[2], "-a") == 0) {
		if (argc != 4)
			goto usage;

		lsmdsfs_list_all(mdsfs);
	} else
		goto usage;

	/* Release */
	g_free(key);

	return 0;

usage:
	printf("Usage: lsmdsfs [key] [-f] [image file] [path]\n");
	printf("       lsmdsfs [key] [-a] [image file]\n");
	return 1;
}
