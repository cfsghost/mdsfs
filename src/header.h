#ifndef MDSFS_HEADER_H
#define MDSFS_HEADER_H

#include <sys/types.h>

typedef struct {
	char verify[64];      /* Verifying version of filesystem - 128bits */
	off_t count;          /* Total file count */
	off_t key_count;
	off_t inode_length;
	off_t data_length;
} MDSHeader;

MDSHeader *mdsfs_header_new(const gchar *sign);

#endif
