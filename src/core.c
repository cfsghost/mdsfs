#include <glib.h>
#include <glib/gi18n.h>

#include "mdsfs.h"
#include "core.h"

MDSFS *
mdsfs_core_open(const gchar *path, const gchar *key)
{
	MDSFS *mdsfs;
	gchar *cursor;

	/* Allocate and mapping */
	mdsfs = (MDSFS *)g_new0(MDSFS, 1);
	mdsfs_io_open(mdsfs, path);
	mdsfs->map = mdsfs_io_mmap(mdsfs->fd, mdsfs->length);
	if (mdsfs->map) {
		cursor = mdsfs->map;
		mdsfs->header = (MDSHeader *)cursor;

		cursor += sizeof(MDSHeader);
		mdsfs->key = (MDSKey *)cursor;

		cursor += sizeof(MDSKey) * mdsfs->header->key_count;
		mdsfs->inode = (MDSINode *)cursor;

		cursor += sizeof(MDSINode) * mdsfs->header->count;
		mdsfs->data = (MDSINode *)cursor;
	}

	/* Initializing lock manager for encryption */
	mdsfs_lockmgr_init(mdsfs, key);

	return mdsfs;
}
