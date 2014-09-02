#ifndef MDSFS_H
#define MDSFS_H

#ifdef _DEBUG
#define DEBUG(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define DEBUG(args...)
#endif

#include "header.h"
#include "inode.h"
#include "lockmgr.h"

typedef struct _MDSFS MDSFS;
struct _MDSFS {
	/* image information */
	int fd;
	size_t length;
	const gchar *map;

	MDSHeader *header;
	MDSKey *key;
	MDSINode *inode;
	gchar *data;
};


#endif
