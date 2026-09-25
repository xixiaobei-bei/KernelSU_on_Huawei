/*
 * uecc.h
 *
 * micro ECC operations
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

#ifndef _UECC_H_
#define _UECC_H_

#include <linux/types.h>

/* Elliptic Curve Cryptography ECDH and ECDSA implementation */
#define UECC_WORD_SIZE 4

typedef int8_t wordcount_t;
typedef int16_t bitcount_t;
typedef int8_t cmpresult_t;

typedef uint32_t uecc_word_t;
typedef uint64_t uecc_dword_t;

#define HIGH_BIT_SET 0x80000000
#define UECC_WORD_BITS 32
#define UECC_WORD_BITS_SHIFT 5
#define UECC_WORD_BITS_MASK 0x01F

#define NUM_WORDS_SECP224R1 7

struct uecc_curve_t;

/*
 * Verify an ECDSA signature.
 *
 * Usage: Compute the hash of the signed data using the same hash function
 * as the signer and pass it to this function along with the signer's public
 * key and the signature.
 * return	1 if signature is valid
 *		0 if signature is invalid
 */
uint32_t uecc_verify(const uecc_word_t *public_key,
		const uint8_t *message_hash,
		uint32_t hash_size,
		const uint8_t *signature,
		bool signature_is_der_encoded,
		const struct uecc_curve_t *curve);

const struct uecc_curve_t *uecc_secp224r1(void);

#endif /* _UECC_H_ */
