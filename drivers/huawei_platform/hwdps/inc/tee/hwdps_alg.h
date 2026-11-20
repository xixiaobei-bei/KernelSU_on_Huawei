/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations about
 *              hwdps algorithm.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_ALG_H
#define _HWDPS_ALG_H

#include <linux/types.h>
#include <linux/fscrypt_common.h>
#include "inc/base/hwdps_defines.h"

#define SHA256_HASH_BYTES 32
#define VERSION_1 0x01
#define VERSION_2 0x02
#define VERSION_3 0x03
#define PHASE2_INDEX_LENGTH 32
#define AES_IV_LENGTH 16
#define FEK_LENGTH 64
#define SHA256_TAG_LENGTH 32
#define VERSION_LENGTH 1
#define RESERVED_LENGTH 22
#define PERMS_LENGTH 8
#define SHA256_LEN 32
#define AES256_KEY_LEN 32
#define PHASE3_CIPHERTEXT_LENGTH (VERSION_LENGTH + \
	AES_IV_LENGTH + FS_KEY_DERIVATION_CIPHER_SIZE)
#define AAD_V1_LEN 106

#pragma pack(1)
struct xattribs_t {
	u8 version[VERSION_LENGTH];
	u8 iv[AES_IV_LENGTH];
	u8 enc_fek[FEK_LENGTH];
};

struct xattribs_v3_t {
	u8 version[VERSION_LENGTH];
	u8 iv[AES_IV_LENGTH];
	u8 enc_fek[FS_KEY_DERIVATION_CIPHER_SIZE];
};

struct xattr_v1_t {
	u8 version[VERSION_LENGTH];
	u8 index[PHASE2_INDEX_LENGTH];
	u8 iv[AES_IV_LENGTH];
	u8 enc_fek[FEK_LENGTH];
	u8 aad[AAD_V1_LEN];
	u8 tag[SHA256_TAG_LENGTH];
};
#pragma pack()

s32 hwdps_generate_enc(buffer_t aes_key, buffer_t ciphertext,
	buffer_t plaintext_fek, bool is_update);

s32 hwdps_generate_dec(buffer_t aes_key,
	buffer_t ciphertext, buffer_t plaintext_fek);

s32 hwdps_dec_fek_v1(const u8 *phase1_key, u32 phase1_key_len,
	const buffer_t *encoded_wfek, u8 *plaintext_fek,
	u32 *plaintext_fek_len);

s32 hwdps_refresh_enc(buffer_t aes_key, buffer_t ciphertext,
	buffer_t plaintext_fek);
#endif
