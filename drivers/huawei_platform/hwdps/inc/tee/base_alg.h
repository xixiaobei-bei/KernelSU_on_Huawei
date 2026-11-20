/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations about
 *              basic algorithm.
 * Create: 2020-06-16
 */

#ifndef _BASE_ALG_H
#define _BASE_ALG_H
#include <linux/types.h>
#include "inc/base/hwdps_defines.h"

/*
 * AES-256-CBC encryption and decryption.
 *
 * Input: key: aes-256 input buffer key
 * Input: iv: aes-256 buffer iv
 * Input: in: input data, should be multiple of AES_BLOCK_SIZE buffer
 * Output: out: buffer to cipher text(decryption) or plain text(encryption)
 * Input: enc: true(encryption) or false(decryption)
 *
 * @return 0 for success, < 0 if an error occurred
 */
s32 aes_cbc(buffer_t key, buffer_t iv, buffer_t in,
	secondary_buffer_t out, bool enc);

/*
 * This is something different with general SHA256. We have strict
 * limitation to the input parameters.
 *
 * Input: hmac_key: 'Salt' in HKDF, should be SHA256_DIGEST_SIZE
 * Input: msg: Message needs to be digested. limited max value to 512
 * Output: tag: Tag for method, can not be null, should have enough space
 *
 * @return 0 for success, < 0 if an error occurred
 */
s32 hash_generate_mac(buffer_t *hmac_key, buffer_t *msg, buffer_t *tag);

s32 hkdf_256(const buffer_t *salt, const buffer_t *ikm,
	const buffer_t *info, buffer_t *okm);

#endif
