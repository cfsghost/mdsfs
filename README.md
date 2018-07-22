# mdsfs - Middle Stone Filesystem

A filesystem with encryption, based on FUSE.

Utilities
-

These utilities are provided:

1. mdsfs - Utility to mount filesystem
2. mkmdsfs - Used to format target image or block
3. lsmdsfs - Used to list files in the specific directory from mdsfs image or block (Without mount)
4. readmdsfs - Read specific file in mdsfs image or block (Without mount)
5. encrypt - A tool to encrypt/decrypt file with blowfish for testing algorithm
6. genkey - A convenient tool to generate a random 384-bit key

Encryption Support
-

mdsfs supported encryption mechanism based on `blowfish` algorithm, it's possible to encrypt filesystem with key (Length is 384-bit).

The easy way to pack a directory and create a encrypted mdsfs image with a key:

	./mkmdsfs [key] [source path] [output filepath]

_example_

	./mkmdsfs 3754b703a399552f6610e7f43c6f76aadfe86fb124a89c7029000f53cb231ceecc62b13e5e6fadfd6fd6f29bebef8ce4 /home/fred/test test.mds


### Accessing with Multiple Key

Multiple access key is supported. That means you can grant more than one user to access this filesystem.

Here is a key file for creating a filesystem image which allows multiple access keys: 

_keys.list_

	3754b703a399552f6610e7f43c6f76aadfe86fb124a89c7029000f53cb231ceecc62b13e5e6fadfd6fd6f29bebef8ce4
	7d7d7450bf1b17813a8d420359cfcfb3c304754245e5c67f0fd41d867453316f473bc8d1db6a1d92d66e4e5b2ae5b45f
	fce13b298a55707781615a607b271a30711ac334d89581c02327d8744ab54e5bf65ba80719c9a9accc394be859d98257
	223a9e285bfdb3ef179863cde29714f472e00096b836c15d8e6e12bf4575969087187ae2218e3dcbb2ac45c8a3d49741

Then creating image with this key file:

	./mkmdsfs keys.lst /home/fred/test test.mds

_Note that remember the ordering number(00-ff) of the access key, it is required when we access filesystem with key._

#### Using Access Key

When you access such filesystem with the key, Adding ordering number in the head of the key is needed.

For example, we are trying to use the second key in previous file `keys.lst`. We should add `01` in the head of key (7d7d74...) when accessing:

```
	017d7d7450bf1b17813a8d420359cfcfb3c304754245e5c67f0fd41d867453316f473bc8d1db6a1d92d66e4e5b2ae5b45f
```

That's wrong without `01` in the head:

```
	7d7d7450bf1b17813a8d420359cfcfb3c304754245e5c67f0fd41d867453316f473bc8d1db6a1d92d66e4e5b2ae5b45f
```


### Mount Encrypted MDSFS

To mount MDSFS to specific path, just using `mdsfs` utility with a key as below:

	./mdsfs test.mds /mnt 003754b703a399552f6610e7f43c6f76aadfe86fb124a89c7029000f53cb231ceecc62b13e5e6fadfd6fd6f29bebef8ce4


Author
-

Fred Chien <fred@mandice.com>

License
-

MIT
