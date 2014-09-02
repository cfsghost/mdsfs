#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <sys/stat.h>

#include "blowfish.h"
#include "encryption.h"

size_t
encryption_encode_to_stream_from_file(FILE *dest, const gchar *src_path)
{
	size_t i;
	FILE *src;
	size_t size;
	size_t filesize = 0;
	guint8 buffer[1024];

	src = fopen(src_path, "r");

	/* encode with blowfish */
	while(!feof(src)) {
		memset(buffer, 0, 1024);
		size = fread(buffer, sizeof(guint8), 1024, src);
		filesize += size;
		for (i = 0; i < size; i += 8) {
			mdsfs_blowfish_encode(buffer+i, buffer+i);
		}

		fwrite(buffer, sizeof(guint8), i, dest);
	}

	fclose(src);

	return filesize;
}

void
encryption_encode_to_stream_from_data(FILE *dest, const gchar *src, size_t src_length)
{
	size_t i;
	guint8 buffer[8];

	/* ignore last stack */
	for (i = 0; i < (src_length - 8); i += 8) {
		mdsfs_blowfish_encode(src+i, buffer);
		fwrite(buffer, sizeof(guint8), 8, dest);
	}

	/* last stack */
	memset(buffer, 0, 8);
	memcpy(buffer, src+i, src_length - i);
	mdsfs_blowfish_encode(buffer, buffer);
	fwrite(buffer, sizeof(guint8), 8, dest);
}

void
encryption_encode_to_data_from_data(const gchar *dest, const gchar *src, size_t src_length)
{
	size_t i;
	guint8 buffer[8];

	/* ignore last stack */
	for (i = 0; i < (src_length - 8); i += 8) {
		mdsfs_blowfish_encode(src+i, dest+i);
	}

	/* last stack */
	memset(buffer, 0, 8);
	memcpy(buffer, src+i, src_length - i);
	mdsfs_blowfish_encode(buffer, buffer);
	memcpy(src+i, buffer, 8);
}

void
encryption_decode_to_stream_from_data(FILE *dest, size_t dest_length, const gchar *src, size_t src_length)
{
	size_t i, j;
	gchar *ptr = src;
	guint8 buffer[1024];

	i = 0;
	if (src_length > 1024) {
		/* ignore last stack */
		for (; i < src_length - 1024; i += 1024, ptr += 1024) {
			memset(buffer, 0, 1024);
			for (j = 0; j < 1024; j += 8) {
				mdsfs_blowfish_decode(ptr+j, buffer+j);
			}

			fwrite(buffer, sizeof(guint8), 1024, dest);
		}
	}

	/* Decode last big stack */
	memset(buffer, 0, 1024);
	j = dest_length - i;
	for (i = 0; i < j; i += 8) {
		mdsfs_blowfish_decode(ptr+i, buffer+i);
	}

	fwrite(buffer, sizeof(gchar), j, dest);
}

void
encryption_decode_to_data_from_data(gchar *dest, size_t dest_length, const gchar *src, size_t src_length)
{
	size_t i;

	/* ignore last stack */
	for (i = 0; i < src_length; i += 8)
		mdsfs_blowfish_decode(src+i, dest+i);

	if (dest_length < src_length)
		*(dest+dest_length) = '\0';
}

void
encryption_decode_to_data_with_offset_from_data(const gchar *dest, off_t offset, size_t size, const gchar *src, size_t src_length)
{
	size_t i = 0;
	size_t j = 0;
	size_t n;
	off_t base_offset;
	gchar *base;
	gchar *curpos;
	guint8 buffer[8];

	/* get address */
	base_offset = offset % 8;
	base = src + offset - base_offset;
	curpos = base;

	/* figure block count */
	n = ((size + base_offset) / 8) + (((size + base_offset) % 8) ? 1 : 0);

	if (base_offset > 0) {
		j = 8 - base_offset;

		memset(buffer, 0, 8);
		mdsfs_blowfish_decode(base, buffer);
		memcpy(dest, buffer, j);

		i = 1;
		curpos += 8;
	}

	for (; i < n - 1; ++i, j += 8, curpos += 8) {
		mdsfs_blowfish_decode(curpos, dest+j);
	}

	/* last stack */
	memset(buffer, 0, 8);
	mdsfs_blowfish_decode(curpos, buffer);
	memcpy(dest+j, buffer, base + size - curpos);
}
