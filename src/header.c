#include <glib.h>
#include <glib/gi18n.h>

#include "header.h"

MDSHeader *
mdsfs_header_new(const gchar *sign)
{
	MDSHeader *header;
	gchar *checksum;

	/* Allocate a new header of filesystem */
	header = (MDSHeader *)g_new0(MDSHeader, 1);

	/* Generate checksum */
	checksum = g_compute_checksum_for_string(G_CHECKSUM_SHA256, sign, strlen(sign));

	strncpy(header->verify, checksum, 64);

	g_free(checksum);

	return header;
}
