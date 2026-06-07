/*
 * present80.h
 *
 * PRESENT80 implementation designed from original paper:
 * -- PRESENT: An Ultra-Lightweight Block Cipher
 * --   http://lightweightcrypto.org/present/present_ches2007.pdf
 * --   https://www.iacr.org/archive/ches2007/47270450/47270450.pdf
 * note: following implementation is only for
 * little-endian architectures supporting 64 bit unsigned integers
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _PRESENT80_H_
#define _PRESENT80_H_

#include <linux/string.h>
#include <linux/types.h>

/*
 * present80
 * present-80 cipher implementation.
 */

/*
 * Module version
 * Version 1.0.0
 */
#define PRESENT80_VERSION    ((PRESENT80_VERSION_MAJOR << 16) | \
		(PRESENT80_VERSION_MINOR << 8) | PRESENT80_VERSION_REVISION)
#define PRESENT80_VERSION_MAJOR 1
#define PRESENT80_VERSION_MINOR 0
#define PRESENT80_VERSION_REVISION 0

/*
 * Present80 context containing cached 32 round keys
 * for given key value of just the key value itself.
 */
typedef uint8_t present80_t[10];

/*
 * Generate the 32 round keys into context from specified key.
 *
 * context		Context to load keys into
 * key			Key to generate round keys from
 */
void present80_setkey(present80_t context, const uint8_t *key);

/*
 * Encrypt a single 64 bit block of plain text into 64 bit of
 * cipher text using present80.
 *
 * context		Context to use for encryption
 * ptext		Plain text input to encrypt
 * ctext		Encrypted data output
 */
void present80(const present80_t context, const uint8_t *ptext, uint8_t *ctext);

/*
 * Compute PRESENT80-CBCMAC tag. If length is not 64-bit block aligned,
 * it will be right zero padded to the next block.
 *
 * context		Context to use for encryption
 * data			Data to compute present80-CBCMAC over
 * datalen		Length of data in bytes
 * clrTag		Pre-clears tag when true.
 *				Set to true when invoking function
 *				with non-zero tag buffer
 * tag			Pointer where to store the tag
 */
void present80_cbcmac(const present80_t context, const uint8_t *data,
	const uint32_t datalen, const bool clrtag, uint8_t *tag);

#endif /* _PRESENT80_H_ */
