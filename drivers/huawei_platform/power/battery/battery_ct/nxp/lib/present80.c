/*
 * present80.c
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

#include "present80.h"

static const uint64_t spbox[] = {
	0x0001000100000000,  /* 0xC */
	0x0000000100000001,  /* 0x5 */
	0x0000000100010000,  /* 0x6 */
	0x0001000000010001,  /* 0xB */
	0x0001000000000001,  /* 0x9 */
	0x0000000000000000,  /* 0x0 */
	0x0001000000010000,  /* 0xA */
	0x0001000100000001,  /* 0xD */
	0x0000000000010001,  /* 0x3 */
	0x0001000100010000,  /* 0xE */
	0x0001000100010001,  /* 0xF */
	0x0001000000000000,  /* 0x8 */
	0x0000000100000000,  /* 0x4 */
	0x0000000100010001,  /* 0x7 */
	0x0000000000000001,  /* 0x1 */
	0x0000000000010000   /* 0x2 */
};

static const uint8_t sbox4[] = {
	0xC0, 0x50, 0x60, 0xB0, 0x90, 0x00, 0xA0, 0xD0,
	0x30, 0xE0, 0xF0, 0x80, 0x40, 0x70, 0x10, 0x20
};

void present80_setkey(present80_t context, const uint8_t *key)
{
	int i;

	if (!context || !key)
		return;
	/* copy key as little-endian */
	for (i = 0; i < 10; i++)
		context[i] = key[9 - i];
}

void present80(const present80_t context, const uint8_t *ptext, uint8_t *ctext)
{
	uint64_t a, b;
	uint8_t *s = (uint8_t *)&a;
	int i, j;
	uint64_t r[2] = {0};
	uint64_t w;

	if (!context || !ptext || !ctext)
		return;
	/* copy reversed key */
	memcpy(r, (uint8_t *)context, sizeof(present80_t));
	/* copy plain-text in byte reverse order into state */
	for (j = 0; j < 8; j++)
		s[j] = ptext[7 - j];
	for (i = 0; i < 31; i++) {
	/* addRoundKey (i) */
		a ^= (((r[1]) << 48) | ((r[0]) >> 16));
		/* copy 19 right most bits */
		w = ((r[0]) & 0x7FFFF);
		/* rotate right by 19 bits */
		r[0] = ((w & 7) << 61) | ((r[1]) << 45) | ((r[0]) >> 19);
		r[1] = (w >> 3);
		/* apply sbox on top 4 bits */
		((uint8_t *)r)[9] = ((((uint8_t *)r)[9]) & 0x0F) |
				sbox4[(int)((((uint8_t *)r)[9]) >> 4)];
		/* xor with round corner on bits 15-19 inclusive */
		(r[0]) ^= (((uint64_t)((i + 1) & 0x1F)) << 15);
		b = a;
		a = spbox[(int)(b & 0xfUL)];
		for (j = 1; j < 16; j++) {
			b >>= 4;
			a |= (spbox[(int)(b & 0xfUL)] << j);
		}
	}
	/* addRoundKey (31) */
	a ^= (((r[1]) << 48) | ((r[0]) >> 16));
	/* copy state in byte reverse order into cipher-text */
	for (j = 0; j < 8; j++)
		ctext[j] = s[7 - j];
}

void present80_cbcmac(const present80_t context, const uint8_t *data,
		const uint32_t datalen, const bool clrtag, uint8_t *tag)
{
	uint8_t *t;
	const uint8_t *s = (tag + 8);
	const uint8_t *p = data;
	const uint8_t *q = (data + datalen);


	/* clear tag */
	if (clrtag)
		for (t = tag; t != s; t++)
			*t = 0;
	if (datalen <= 0)
		return;
	/* preform CBCMAC on data */
	while (p != q) {
		/* XOR plain with cipher-text */
		if (data) {
			for (t = tag; t != s && p != q; t++, p++)
				*t ^= *p;
		} else {
			p += 8;
			if (((int)(p - q)) > 0)
				p = q;
		}
		/* encrypt block */
		present80(context, tag, tag);
	}
}
