#include <glib.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "mdsfs.h"
#include "io.h"

void
mdsfs_io_open(MDSFS *mdsfs, const gchar *filename)
{
	struct stat s;

	mdsfs->fd = open(filename, O_RDWR);

	stat(filename, &s);
	mdsfs->length = s.st_size;
}

void
mdsfs_io_close(MDSFS *mdsfs)
{
	msync((void *)mdsfs->map, mdsfs->length, MS_SYNC);
	munmap((void *)mdsfs->map, mdsfs->length);
	close(mdsfs->fd);
}

const gchar *
mdsfs_io_mmap(int fd, size_t length)
{
	gchar *map;

	map = (gchar *)mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED)
		return NULL;

	return map;
}
