#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>

#include "misc.h"

static gchar table[16] = "0123456789abcdef";

gboolean
mdsfs_misc_check_key(gchar *key)
{
	gboolean ret = TRUE;
	gchar *src = g_strdup(key);
	gchar *ptr = src;

	/* convert upper character to lower */
	while(*ptr != '\0') {
		if (*ptr >= 'A' && *ptr <= 'F')
			*ptr += 32;

		ptr++;
	}

	ptr = src;
	while(*ptr != '\0') {
		if (*ptr < '0' || (*ptr > '9' && *ptr < 'a') || *ptr > 'f')
			ret = FALSE;

		ptr++;
	}

	g_free(src);

	return ret;
}

gint
mdsfs_misc_hexc_to_binary(gint c)
{
	gint i;

	for (i = 0; i < 16; ++i) {
		if (table[i] == c)
			return i;
	}

	return 0;
}

guint8 *
mdsfs_misc_hexstrings_to_binary(gchar *src)
{
	gint i;
	guint8 *c;
	gchar *source = g_strdup(src);
	gchar *ptr = source;
	guint8 *reptr;
	guint8 *result;

	/* convert upper character to lower */
	while(*ptr != '\0') {
		if (*ptr >= 'A' && *ptr <= 'F')
			*ptr += 32;

		ptr++;
	}

	result = (gchar *)g_new0(gchar, (strlen(source) / 2) + 1);
	c = result;
	for (ptr = source; *ptr != '\0'; ptr += 2, ++c) {
		for (i = 0; i < 16; ++i) {
			if (table[i] == *(ptr+1)) {
				*c = (guint8)i;
				break;
			}
		}

		for (i = 0; i < 16; ++i) {
			if (table[i] == *ptr) {
				*c += i * 16;
				break;
			}
		}
	}

	return result;
}

