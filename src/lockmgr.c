#include <glib.h>
#include <glib/gi18n.h>
#include <string.h>

#include "mdsfs.h"
#include "blowfish.h"
#include "encryption.h"
#include "lockmgr.h"

guint8 *
mdsfs_lockmgr_keygen()
{
	gint len = 48;
	guint8 *key, *ptr;

	/* allocate a memboy for 448 bits */
	key = (guint8 *)g_new0(guint8, len + 1);

	/* seed */
	srand(time(0)+getpid());
	for (ptr = key; len > 0; --len, ++ptr)
		*ptr = (guint8)(rand() % 256);

	return key;
}

guint8 *
mdsfs_lockmgr_keygen_with_hex()
{
	gint len = 96;
	guint8 *key, *ptr;
	gchar table[16] = "0123456789abcdef";

	/* allocate a memboy for 448 bits */
	key = (guint8 *)g_new0(guint8, len + 1);

	/* seed */
	srand(time(0)+getpid());
	for (ptr = key; len > 0; --len, ++ptr)
		*ptr = table[rand() % 16];

	return key;
}

guint8 *
mdsfs_lockmgr_get(MDSFS *mdsfs, off_t num, const guint8 *key)
{
	guint8 newkey[48];

	mdsfs_blowfish_init(key);
	encryption_decode_to_data_from_data(newkey, 48, mdsfs->key[num].key, 48);

	return g_strndup(newkey, 48);
}

guint8 *
mdsfs_lockmgr_get_last(MDSFS *mdsfs, off_t num, const guint8 *key)
{
	off_t i;
	guint8 *curkey;
	guint8 *newkey;

	curkey = g_strndup(key, 48);
	for (i = num; i >= 0; --i) {
		newkey = mdsfs_lockmgr_get(mdsfs, i, curkey);

		g_free(curkey);
		curkey = newkey;
	}

	return curkey;
}

gboolean
mdsfs_lockmgr_check_key(const gchar *key)
{
	/* 8 bits for level and 384 bits for blowfish key */
	if (strlen(key) != 98) {
		printf("Secret key length is incorrect, we need 392-bit key.\n");
		return FALSE;
	}

	if (!mdsfs_misc_check_key(key)) {
		printf("Secret key is incorrect, please using hex strings.\n");
		return FALSE;
	}

	return TRUE;
}

void
mdsfs_lockmgr_init(MDSFS *mdsfs, const guint8 *key)
{
	guint8 *final_key;
	off_t num;

	/* figure level */
	num = *key;

	final_key = mdsfs_lockmgr_get_last(mdsfs, num, key+1);

	mdsfs_blowfish_init(final_key);

	g_free(final_key);
}
