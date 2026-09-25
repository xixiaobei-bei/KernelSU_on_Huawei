/*
 * sha224.h
 *
 * FIPS 180-2 SHA-224/256/384/512 implementation
 * Issue date:  04/30/2005
 * http://www.ouah.org/ogay/sha2/
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

#ifndef _SHA224_H_
#define _SHA224_H_

#include <linux/types.h>

/*!
 * sha224
 * SHA224 algorithms. Implements SHA224 hash generation, HMAC and HKDF.
 */

/* Size of SHA224 hash (in byte) */
#define SHA224_HASH_SIZE 28
/* Size of SHA224 block (in byte) */
#define SHA224_BLOCK_SIZE 64

/* SHA224 context */
struct sha224_t {
	uint32_t m_tot_len;
	uint32_t m_len;
	uint8_t m_block[2 * SHA224_BLOCK_SIZE];
	uint32_t m_h[8];
};

/*
 * Initialize SHA224 context.
 *
 * context		Pointer to SHA224 context to initialize
 */
void sha224_init(struct sha224_t *context);

/*
 * Update SHA224 context with specified message.
 *
 * context		Pointer to SHA224 context to update
 * message		Pointer to input message
 * len			Length of message
 */
void sha224_update(struct sha224_t *context,
	const uint8_t *message, const uint32_t len);

/*
 * Finalize SHA224 context and return digest.
 *
 * context		Pointer to SHA224 context to finalize
 * digest		Pointer to store final digest
 */
void sha224_final(struct sha224_t *context, uint8_t *digest);

#endif /* _SHA224_H_ */
