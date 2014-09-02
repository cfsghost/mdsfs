#ifndef MDSFS_ENCRYPT_H
#define MDSFS_ENCRYPT_H

typedef struct {
	char tags[15];
	size_t size;
} EncryptHeader;

#define BLOWFISH_TAG "BLOWFISH CIPHER"

#endif
