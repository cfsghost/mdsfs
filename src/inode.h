#ifndef MDSFS_INODE_H
#define MDSFS_INODE_H

#include <glib.h>
#include <sys/types.h>

typedef struct {
	off_t id;
	off_t parent_id;
	off_t next_id;
	off_t prev_id;
	off_t child_id;

	char name[256];
	size_t size;  /* origin file size */
	off_t length; /* data length */
	off_t offset; /* data position */
	off_t count;  /* Total file count */

	time_t atime;
	time_t mtime;
	time_t ctime;

	mode_t mode;
} MDSINode;

MDSINode *mdsfs_inode_new(const gchar *name);


#endif
