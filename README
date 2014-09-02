mdsfs - Middle Stone Filesystem

A filesystem with encryption, based on FUSE.

Utilities
-

Here is some utilities mdsfs provided:

1. mdsfs - Utility to mount filesystem
2. mkmdsfs - Used to format target image or block
3. lsmdsfs - Used to list files in the specific directory from mdsfs image or block (Without mount)
4. readmdsfs - Read specific file in mdsfs image or block (Without mount)
5. encrypt - A tool to encrypt/decrypt file with blowfish for testing algorithm
6. genkey - A convenient tool to generate a random 384-bit key

Encryption Support
-

mdsfs supported encryption mechanism based on blowfish algorithm, it's possible to encrypt filesystem with key (Length is 384-bit).

The easy way to pack a directory and create a encrypted mdsfs image with a key:

		./mkmdsfs [key] [source path] [output filepath]

_example_

		./mkmdsfs 3754b703a399552f6610e7f43c6f76aadfe86fb124a89c7029000f53cb231ceecc62b13e5e6fadfd6fd6f29bebef8ce4 /home/fred/test test.mds


### Accessing with Several Keys

mdsfs is also supports access with several keys at the same time. Once user has one of keys, having access is possible and allowed.

If you would like to create a image contains several keys, you must prepare a key file which contains several keys looked like below:

_keys.list_

		3754b703a399552f6610e7f43c6f76aadfe86fb124a89c7029000f53cb231ceecc62b13e5e6fadfd6fd6f29bebef8ce4
		7d7d7450bf1b17813a8d420359cfcfb3c304754245e5c67f0fd41d867453316f473bc8d1db6a1d92d66e4e5b2ae5b45f
		fce13b298a55707781615a607b271a30711ac334d89581c02327d8744ab54e5bf65ba80719c9a9accc394be859d98257
		223a9e285bfdb3ef179863cde29714f472e00096b836c15d8e6e12bf4575969087187ae2218e3dcbb2ac45c8a3d49741

Then creating image with this key file:

		./mkmdsfs keys.lst /home/fred/test test.mds

### Mount Encrypted MDSFS

Just using 'mdsfs' utility with a key:

		./mdsfs test.mds /mnt 3754b703a399552f6610e7f43c6f76aadfe86fb124a89c7029000f53cb231ceecc62b13e5e6fadfd6fd6f29bebef8ce4

Author
-

Fred Chien <fred@mandice.com>

License
-

MIT
