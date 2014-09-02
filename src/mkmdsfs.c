#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

#include "mdsfs.h"
#include "header.h"
#include "inode.h"
#include "io.h"
#include "lockmgr.h"
#include "mkmdsfs.h"

MDSFSTemp *
mkmdsfs_mktmpfile()
{
	MDSFSTemp *temp;

	temp = (MDSFSTemp *)g_new0(MDSFSTemp, 1);
	temp->path = g_strdup_printf("%s/.mkmdsfs.%d.tmp", g_get_tmp_dir(), getpid());
	temp->fp = fopen(temp->path, "w+");

	return temp;
}

void
mkmdsfs_tmpfile_close(MDSFSTemp *temp)
{
	fclose(temp->fp);
	unlink(temp->path);

	g_free(temp->path);
	g_free(temp);
}

void
mkmdsfs_key_add(MDSFSImage *image, const gchar *key)
{
	image->header->key_count++;

	/* Allocate */
	if (!image->keys)
		image->keys = g_new0(MDSKey, 1);
	else
		image->keys = g_renew(MDSKey, image->keys, image->header->key_count);

	memcpy(image->keys[image->header->key_count - 1].key, key, 48);
}

void
mkmdsfs_key_add_from_file(MDSFSImage *image, const gchar *path)
{
	FILE *fp;
	guint8 hexkey[97] = {0};
	gchar *key;
	gint size;

	fp = fopen(path, "r");
	if (!fp)
		return;

	while(fscanf(fp, "%s", &hexkey) != -1) {
		printf("Add Key: %48s\n", hexkey);
		key = mdsfs_misc_hexstrings_to_binary(hexkey);
		mkmdsfs_key_add(image, key);
		g_free(key);
	}

	fclose(fp);
}

void
mkmdsfs_key_write(MDSFSImage *image, const gchar *final_key)
{
	guint8 key[49] = {0};
	off_t i;

	for (i = 0; i < image->header->key_count - 1; ++i) {
		memcpy(key, image->keys[i+1].key, 48);
		mdsfs_blowfish_init(key);
		encryption_encode_to_stream_from_data(image->dirfp, image->keys[i].key, 48);
	}

	/* last key */
	mdsfs_blowfish_init(final_key);
	encryption_encode_to_stream_from_data(image->dirfp, image->keys[i].key, 48);
}

void
mkmdsfs_key_init(MDSFSImage *image)
{
	/* Generate a new key for the image */
	image->key = mdsfs_lockmgr_keygen();
	mkmdsfs_key_add(image, image->key);
}

gulong
mkmdsfs_count_directory(const gchar *path)
{
	gulong count = 0;
	struct dirent *file;
	DIR *dir;

	if ((dir = opendir(path)) != NULL) {
		while((file = readdir(dir))) {
			if (strncmp(file->d_name, ".", 1) == 0)
				continue;

			if (strncmp(file->d_name, "..", 1) == 0)
				continue;

			count++;
		}
	}

	closedir(dir);

	return count;
}

void
mkmdsfs_inode_new(MDSFSImage *image, const gchar *path, const gchar *filename, off_t parent_id)
{
	MDSINode *inode;
	gchar *fullpath;
	struct stat s;

	/* inode conter */
	image->header->count++;

	/* update inode length of header */
	image->header->inode_length += sizeof(MDSINode);

	/* get full path */
	fullpath = g_strdup_printf("%s/%s", path, filename);
//	printf("%s\n", filename);

	/* Create a inode */
	inode = mdsfs_inode_new(filename);
	inode->id = image->counter++;
	inode->parent_id = parent_id;

	/* file status */
	stat(fullpath, &s);
	inode->size = s.st_size;
	inode->mode = s.st_mode;
	inode->atime = s.st_atime;
	inode->mtime = s.st_mtime;
	inode->ctime = s.st_ctime;

	/* Scan subdirectory */
	if (s.st_mode & S_IFDIR) {
		inode->count = mkmdsfs_count_directory(fullpath);
	}

	/* Write to image file */
	fwrite(inode, sizeof(MDSINode), 1, image->dirfp);

	/* Release */
	g_free(inode);
	g_free(fullpath);
}

void
mkmdsfs_scan_directory(MDSFSImage *image, const gchar *path, off_t id)
{
	gchar *fullpath = NULL;
	struct dirent *file;
	DIR *dir;
	off_t curid;

	if ((dir = opendir(path)) != NULL) {
		/* store beginning of inode number */
		curid = image->counter;

		/* scan directory */
		while((file = readdir(dir))) {
			if (strncmp(file->d_name, ".", 1) == 0)
				continue;

			if (strncmp(file->d_name, "..", 2) == 0)
				continue;

			mkmdsfs_inode_new(image, path, file->d_name, id);
		}
	}

	closedir(dir);

	if ((dir = opendir(path)) != NULL) {
		/* Scan subdirectory */
		while((file = readdir(dir))) {
			if (strncmp(file->d_name, ".", 1) == 0)
				continue;

			if (strncmp(file->d_name, "..", 2) == 0)
				continue;

			if (file->d_type & DT_DIR) {
				fullpath = g_strdup_printf("%s/%s", path, file->d_name);
				mkmdsfs_scan_directory(image, fullpath, curid);
				g_free(fullpath);
			}

			curid++;
		}
	}

	closedir(dir);
}

void
mkmdsfs_inode_generate(MDSFSImage *image)
{
	struct stat s;
	MDSINode *inode;

	/* Create root inode */
	inode = mdsfs_inode_new("/");
	inode->id = image->counter++;

	/* file status */
	stat(image->path, &s);
	inode->size = s.st_size;
	inode->mode = s.st_mode;
	inode->atime = s.st_atime;
	inode->mtime = s.st_mtime;
	inode->ctime = s.st_ctime;
	inode->count = mkmdsfs_count_directory(image->path);

	/* Write to image file */
	fwrite(inode, sizeof(MDSINode), 1, image->dirfp);

	/* header */
	image->header->count = 1;
	image->header->inode_length += sizeof(MDSINode);

	/* Scan sub-directory */
	mkmdsfs_scan_directory(image, image->path, inode->id);

	g_free(inode);
}

void
mkmdsfs_generate_index(const gchar *src_path, const gchar *dest_path, const gchar *key, gboolean use_keyfile)
{
	MDSFSImage *image;

	/* Initializing */
	image = (MDSFSImage *)g_new0(MDSFSImage, 1);
	image->path = realpath(src_path, NULL);

	/* Create header of filesystem */
	image->header = mdsfs_header_new("Middle-Stone Filesystem 1.0");

	/* Initializing lock manager */
	mkmdsfs_key_init(image);

	if (use_keyfile)
		mkmdsfs_key_add_from_file(image, key);

	/* Create a new image file */
	image->dirfp = fopen(dest_path, "w+");
	if (image->dirfp) {
		/* Skip Header */
		fseek(image->dirfp, sizeof(MDSHeader), SEEK_SET);

		/* keys */
		mkmdsfs_key_write(image, key);
		mdsfs_blowfish_init(image->key);

		/* Generate inode */
		mkmdsfs_inode_generate(image);

		/* Write header */
		fseek(image->dirfp, 0, SEEK_SET);
		fwrite(image->header, sizeof(MDSHeader), 1, image->dirfp);

		fclose(image->dirfp);
	}
}

void
mkmdsfs_data_new(MDSFSData *mdsdata, const gchar *root, const gchar *path)
{
	struct stat s;
	MDSINode *inode;
	gchar *realpath;

	if (strcmp(root, path) == 0)
		return;

	stat(path, &s);
	if (s.st_mode & S_IFREG) {
		/* Empty file */
		if (s.st_size == 0)
			return;

		/* Remove real path to get own rootpath */
		realpath = path + strlen(root);

		/* get inode from index */
		inode = mdsfs_inode_find_with_path(mdsdata->mdsfs, realpath);
		inode->offset = ftell(mdsdata->fp);

		/* Write encoded data to image file */
		inode->length = encryption_encode_to_stream_from_file(mdsdata->fp, path);

		printf("%5d\t", inode->id);
		printf("%10d\t", inode->offset);
		printf("%10d\t", inode->size);
		printf("%10d\t", inode->length);
		printf("%s\n", path);

		/* update data length of header */
		mdsdata->mdsfs->header->data_length += inode->length;
	}
}

void
mkmdsfs_scan_file(MDSFSData *mdsdata, const gchar *root, const gchar *path)
{
	gchar *fullpath;
	struct dirent *file;
	DIR *dir;

	if ((dir = opendir(path)) != NULL) {
		while((file = readdir(dir))) {
			if (strncmp(file->d_name, ".", 1) == 0)
				continue;

			if (strncmp(file->d_name, "..", 2) == 0)
				continue;

			/* get full path */
			fullpath = g_strdup_printf("%s/%s", path, file->d_name);

			if (file->d_type & DT_DIR)
				mkmdsfs_scan_file(mdsdata, root, fullpath);
			else
				mkmdsfs_data_new(mdsdata, root, fullpath);

			g_free(fullpath);
		}
	}

	closedir(dir);
}

MDSFSTemp *
mkmdsfs_generate_data(const gchar *src_path, const gchar *dest_path)
{
	MDSFSData *mdsdata;
	MDSFSTemp *temp;
	gchar *real_path = realpath(src_path, NULL);
	FILE *fp;

	mdsdata = (MDSFSData *)g_new0(MDSFSData, 1);

	/* Open current header and inode */
	mdsdata->mdsfs = (MDSFS *)g_new0(MDSFS, 1);
	mdsfs_io_open(mdsdata->mdsfs, dest_path);
	mdsdata->mdsfs->map = mdsfs_io_mmap(mdsdata->mdsfs->fd, mdsdata->mdsfs->length);
	if (mdsdata->mdsfs->map) {
		mdsdata->mdsfs->header = (MDSHeader *)mdsdata->mdsfs->map;
		mdsdata->mdsfs->key = (MDSKey *)(mdsdata->mdsfs->map + sizeof(MDSHeader));
		mdsdata->mdsfs->inode = (MDSINode *)(mdsdata->mdsfs->map + sizeof(MDSHeader) + sizeof(MDSKey) * mdsdata->mdsfs->header->key_count);
	}

	/* Create a temp file to contain data */
	temp = mkmdsfs_mktmpfile();
	mdsdata->fp = temp->fp;

	mkmdsfs_scan_file(mdsdata, real_path, real_path);

	fsync(mdsdata->fp);

	/* Release */
	mdsfs_io_close(mdsdata->mdsfs);
	g_free(mdsdata->mdsfs);
	g_free(mdsdata);
	g_free(real_path);

	return temp;
}

void
mkmdsfs_package(const gchar *dest_path, FILE *src)
{
	FILE *dest;
	size_t size;
	gchar buffer[65536];

	dest = fopen(dest_path, "a");
	fseek(src, 0, SEEK_SET);

	while(!feof(src)) {
		size = fread(buffer, sizeof(gchar), 65536, src);
		if (size > 0)
			fwrite(buffer, sizeof(gchar), size, dest);
	}

	fclose(dest);
}

void
mkmdsfs_inode_build(const gchar *path)
{
	MDSFS *mdsfs;
	off_t i;
	off_t id;

	/* Open current header and inode */
	mdsfs = (MDSFS *)g_new0(MDSFS, 1);
	mdsfs_io_open(mdsfs, path);
	mdsfs->map = mdsfs_io_mmap(mdsfs->fd, mdsfs->length);
	if (mdsfs->map) {
		mdsfs->header = (MDSHeader *)mdsfs->map;
		mdsfs->key = (MDSKey *)(mdsfs->map + sizeof(MDSHeader));
		mdsfs->inode = (MDSINode *)(mdsfs->map + sizeof(MDSHeader) + sizeof(MDSKey) * mdsfs->header->key_count);
		//mdsfs->data = (gchar *)(mdsfs->map + sizeof(MDSHeader) + mdsfs->header->inode_length);
	}

	/* Ignore first inode(root) and last inode */
	for (i = 1; i < mdsfs->header->count; ++i) {
		/* previous inode */
		if (mdsfs->inode[i].parent_id == mdsfs->inode[i-1].parent_id) {
			/* avoid root */
			if (mdsfs->inode[i-1].id != mdsfs->inode[i-1].parent_id)
				mdsfs->inode[i].prev_id = mdsfs->inode[i-1].id;
		}

		/* last inode has no next inode */
		if (i + 1 == mdsfs->header->count)
			break;

		/* next inode */
		if (mdsfs->inode[i].parent_id == mdsfs->inode[i+1].parent_id)
			mdsfs->inode[i].next_id = mdsfs->inode[i+1].id;
	}

	/* Set child of parent inode */
	for (i = 1; i < mdsfs->header->count; ++i) {
		/* first inode in the directory */
		if (mdsfs->inode[i].prev_id == 0) {
			id = mdsfs->inode[i].parent_id;
			mdsfs->inode[id].child_id = mdsfs->inode[i].id;
		}
	}

	/* Release */
	mdsfs_io_close(mdsfs);
	g_free(mdsfs);
}

int main(int argc, char *argv[])
{
	guint8 *key; 
	MDSFSTemp *temp;

#ifdef ENABLE_NLS
	bindtextdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);
#endif

	if (argc < 4 || argc > 5) {
		goto usage;
	}

	if (!g_file_test(argv[2], G_FILE_TEST_EXISTS)) {
		printf("%s doesn't exist.\n", argv[2]);
		return 1;
	}

	if (argc == 4) {
		if (strlen(argv[1]) != 96) {
			printf("Secret key length is incorrect, we need 384-bit key.\n");
			return 1;
		}

		if (!mdsfs_misc_check_key(argv[1])) {
			printf("Secret key is incorrect, please using hex strings.\n");
			return 1;
		}

		/* Generate index first */
		key = mdsfs_misc_hexstrings_to_binary(argv[1]);
		mkmdsfs_generate_index(argv[2], argv[3], key, FALSE);
		g_free(key);

		mkmdsfs_inode_build(argv[3]);

		/* Generate data image */
		temp = mkmdsfs_generate_data(argv[2], argv[3]);
		mkmdsfs_package(argv[3], temp->fp);
		mkmdsfs_tmpfile_close(temp);
	} else {
		if (!g_file_test(argv[3], G_FILE_TEST_EXISTS)) {
			printf("%s doesn't exist.\n", argv[3]);
			return 1;
		}

		mkmdsfs_generate_index(argv[3], argv[4], argv[2], TRUE);

		mkmdsfs_inode_build(argv[4]);

		/* Generate data image */
		temp = mkmdsfs_generate_data(argv[3], argv[4]);
		mkmdsfs_package(argv[4], temp->fp);
		mkmdsfs_tmpfile_close(temp);
	}

	return 0;

usage:
	printf("Usage: mkmdsfs [key] [source path] [output path]\n");
	printf("       mkmdsfs -f [key file] [source path] [output path]\n");
	return 1;
}
