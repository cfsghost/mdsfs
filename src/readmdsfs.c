#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include "mdsfs.h"
#include "inode.h"
#include "io.h"
#include "readmdsfs.h"

void
readmdsfs_read(MDSFS *mdsfs, const gchar *path)
{
	FILE *fp;
	off_t count;
	off_t i;
	MDSINode *inode = NULL;

	inode = mdsfs_inode_find_with_path(mdsfs, path);
	if (!inode) 
		return;

	if (inode->size == 0)
		return;

	if (inode->mode & S_IFREG) {
		fp = fdopen(STDOUT_FILENO, "w");
		encryption_decode_to_stream_from_data(fp, inode->size, mdsfs->data + inode->offset, inode->length);
		fclose(fp);
	}
}

int main(int argc, char *argv[])
{
	MDSFS *mdsfs;
	guint8 *key; 

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if (argc != 4) {
		goto usage;
	}

	if (!mdsfs_lockmgr_check_key(argv[1]))
		return 1;

	if (!g_file_test(argv[2], G_FILE_TEST_EXISTS)) {
		printf("%s doesn't exist.\n", argv[2]);
		return 1;
	}

	/* convert hex strings to binary key */
	key = mdsfs_misc_hexstrings_to_binary(argv[1]);

	/* Initiallizing */
	mdsfs = mdsfs_core_open(argv[2], key);

	readmdsfs_read(mdsfs, argv[3]);

	/* Release */
	g_free(key);

	return 0;

usage:
	printf("Usage: readmdsfs [key] [image file] [path]\n");
	return 1;
}
