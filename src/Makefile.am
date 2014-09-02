## Process this file with automake to produce Makefile.in

bin_PROGRAMS = mdsfs mkmdsfs readmdsfs lsmdsfs encrypt genkey

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_BIN_DIR=\""$(bindir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	$(PACKAGE_CFLAGS) \
	$(G_CAST_CHECKS)

mdsfs_SOURCES = \
	core.c \
	core.h \
	lockmgr.c \
	lockmgr.h \
	misc.c \
	misc.h \
	encryption.c \
	encryption.h \
	blowfish.c \
	blowfish.h \
	io.c \
	io.h \
	header.c \
	header.h \
	inode.c \
	inode.h \
	mdsfs.h \
	mdsfs.c

mdsfs_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

mkmdsfs_SOURCES = \
	core.c \
	core.h \
	lockmgr.c \
	lockmgr.h \
	misc.c \
	misc.h \
	encryption.c \
	encryption.h \
	blowfish.c \
	blowfish.h \
	inode.c \
	inode.h \
	header.c \
	header.h \
	io.c \
	io.h \
	mkmdsfs.c \
	mkmdsfs.h

mkmdsfs_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

readmdsfs_SOURCES = \
	core.c \
	core.h \
	lockmgr.c \
	lockmgr.h \
	misc.c \
	misc.h \
	encryption.c \
	encryption.h \
	blowfish.c \
	blowfish.h \
	io.c \
	io.h \
	header.c \
	header.h \
	inode.c \
	inode.h \
	readmdsfs.c \
	readmdsfs.h

readmdsfs_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

lsmdsfs_SOURCES = \
	core.c \
	core.h \
	lockmgr.c \
	lockmgr.h \
	misc.c \
	misc.h \
	encryption.c \
	encryption.h \
	blowfish.c \
	blowfish.h \
	io.c \
	io.h \
	header.c \
	header.h \
	inode.c \
	inode.h \
	lsmdsfs.c \
	lsmdsfs.h

lsmdsfs_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

encrypt_SOURCES = \
	misc.c \
	misc.h \
	blowfish.c \
	blowfish.h \
	encrypt.c \
	encrypt.h

encrypt_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

genkey_SOURCES = \
	genkey.c \
	genkey.h

genkey_LDADD = \
		$(PACKAGE_LIBS) \
		$(INTLLIBS)

