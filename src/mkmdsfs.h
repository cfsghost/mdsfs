#ifndef MDSFS_MKMDSFS_H
#define MDSFS_MKMDSFS_H

typedef struct {
	off_t counter;
	FILE *dirfp;
	FILE *filefp;
	gchar *path;
	gchar *key;
	MDSHeader *header;
	MDSKey *keys;
} MDSFSImage;

typedef struct {
	FILE *fp;
	MDSFS *mdsfs;
} MDSFSData;

typedef struct {
	FILE *fp;
	gchar *path;
} MDSFSTemp;

#endif
