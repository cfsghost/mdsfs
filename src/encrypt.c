#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>

#include "blowfish.h"
#include "encrypt.h"

void
encrypt_encode_file(const gchar *src_path, const gchar *dest_path)
{
	gint i;
	FILE *dest;
	FILE *src;
	size_t size;
	size_t filesize = 0;
	guint8 buffer[1024];
	EncryptHeader header;

	src = fopen(src_path, "r");
	dest = fopen(dest_path, "w");

	/* Store original information */
	memset(&header, 0, sizeof(EncryptHeader));
	strncpy(header.tags, BLOWFISH_TAG, strlen(BLOWFISH_TAG));

	/* get original file size */
	fseek(src, 0, SEEK_END);
	header.size = ftell(src);
	fseek(src, 0, SEEK_SET);

	/* Write header */
	fwrite(&header, sizeof(EncryptHeader), 1, dest);

	/* encode with blowfish */
	while(!feof(src)) {
		memset(buffer, 0, 1024);
		size = fread(buffer, sizeof(guint8), 1024, src);
		filesize += size;
		for (i = 0; i < size; i += 8) {
			mdsfs_blowfish_encode(buffer+i, buffer+i);
		}

		fwrite(buffer, sizeof(gchar), i, dest);
	}

	fclose(dest);
	fclose(src);
}

void
encrypt_decode_file(const gchar *src_path, const gchar *dest_path)
{
	gint i;
	FILE *dest;
	FILE *src;
	size_t size;
	size_t filesize = 0;
	gchar buffer[1024];
	EncryptHeader header;

	src = fopen(src_path, "r");
	dest = fopen(dest_path, "w");

	/* Read header */
	fread(&header, sizeof(EncryptHeader), 1, src);

	while(!feof(src)) {
		size = fread(buffer, sizeof(gchar), 1024, src);
		filesize += size;
		for (i = 0; i < size; i += 8) {
			mdsfs_blowfish_decode(buffer+i, buffer+i);
		}

		if (filesize > header.size)
			i -= filesize - header.size;

		fwrite(buffer, sizeof(gchar), i, dest);
	}

	fclose(dest);
	fclose(src);
}

int main(int argc, char *argv[])
{
	guint8 *key; 

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if (argc != 5) {
		goto usage;
	}

	if (strlen(argv[2]) != 96) {
		printf("Secret key length is incorrect, we need 384-bit key.\n");
		return 1;
	}

	if (!mdsfs_misc_check_key(argv[2])) {
		printf("Secret key is incorrect, please using hex.\n");
		return 1;
	}

	if (!g_file_test(argv[3], G_FILE_TEST_EXISTS)) {
		printf("%s doesn't exist.\n", argv[3]);
		return 1;
	}

	if (g_file_test(argv[3], G_FILE_TEST_IS_DIR)) {
		printf("%s is a directory.\n", argv[3]);
		return 1;
	}

	if (g_file_test(argv[4], G_FILE_TEST_IS_DIR)) {
		printf("%s is a directory.\n", argv[4]);
		return 1;
	}

	/* Initializing blowfish */
	key = mdsfs_misc_hexstrings_to_binary(argv[2]);
	mdsfs_blowfish_init(key);

	if (strcmp(argv[1], "-e") == 0) {
		encrypt_encode_file(argv[3], argv[4]);
	} else if (strcmp(argv[1], "-d") == 0) {
		encrypt_decode_file(argv[3], argv[4]);
	} else {
		goto usage;
	}

	return 0;

usage:
	printf("Usage: encrypt [-e|-d] [key] [input file] [output file]\n");
	return 1;
}

