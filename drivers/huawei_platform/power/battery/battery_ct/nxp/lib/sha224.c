/*
 * sha224.c
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

#include "sha224.h"

#define SHA2_SHFR(x, n) (x >> n)
#define SHA2_ROTR(x, n) ((x >> n) | (x << ((sizeof(x) << 3) - n)))
#define SHA2_ROTL(x, n) ((x << n) | (x >> ((sizeof(x) << 3) - n)))
#define SHA2_CH(x, y, z) ((x & y) ^ (~x & z))
#define SHA2_MAJ(x, y, z) ((x & y) ^ (x & z) ^ (y & z))
#define SHA224_F1(x) (SHA2_ROTR(x, 2) ^ SHA2_ROTR(x, 13) ^ SHA2_ROTR(x, 22))
#define SHA224_F2(x) (SHA2_ROTR(x, 6) ^ SHA2_ROTR(x, 11) ^ SHA2_ROTR(x, 25))
#define SHA224_F3(x) (SHA2_ROTR(x, 7) ^ SHA2_ROTR(x, 18) ^ SHA2_SHFR(x, 3))
#define SHA224_F4(x) (SHA2_ROTR(x, 17) ^ SHA2_ROTR(x, 19) ^ SHA2_SHFR(x, 10))
#define SHA2_UNPACK32(x, str)			\
do {						\
	*((str) + 3) = (uint8_t)((x));		\
	*((str) + 2) = (uint8_t)((x) >> 8);	\
	*((str) + 1) = (uint8_t)((x) >> 16);	\
	*((str) + 0) = (uint8_t)((x) >> 24);	\
} while (0)
#define SHA2_PACK32(str, x) {			\
	*(x) = (((uint32_t)*((str) + 3)) |	\
		((uint32_t)*((str) + 2) << 8) |	\
		((uint32_t)*((str) + 1) << 16) |\
		((uint32_t)*((str) + 0) << 24));\
}
static const uint32_t sha224_k[] = {
	0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5,
	0x3956C25B, 0x59F111F1, 0x923F82A4, 0xAB1C5ED5,
	0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
	0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174,
	0xE49B69C1, 0xEFBE4786, 0x0FC19DC6, 0x240CA1CC,
	0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
	0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7,
	0xC6E00BF3, 0xD5A79147, 0x06CA6351, 0x14292967,
	0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
	0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85,
	0xA2BFE8A1, 0xA81A664B, 0xC24B8B70, 0xC76C51A3,
	0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
	0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5,
	0x391C0CB3, 0x4ED8AA4A, 0x5B9CCA4F, 0x682E6FF3,
	0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
	0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2
};

static void sha224_transform(struct sha224_t *context,
		const uint8_t *message, uint32_t block_nb)
{
	uint32_t w[64];
	uint32_t wv[8];
	uint32_t t1, t2;
	const uint8_t *sub_block = NULL;
	uint32_t i, j;

	for (i = 0; i < (uint32_t)block_nb; i++) {
		sub_block = message + (i << 6);
		for (j = 0; j < 16; j++)
			SHA2_PACK32(&sub_block[j << 2], &w[j]);
		for (j = 16; j < 64; j++)
			w[j] = SHA224_F4(w[j - 2]) + w[j - 7]
				+ SHA224_F3(w[j - 15]) + w[j - 16];
		for (j = 0; j < 8; j++)
			wv[j] = (context->m_h)[j];
		for (j = 0; j < 64; j++) {
			t1 = wv[7] + SHA224_F2(wv[4]) +
			SHA2_CH(wv[4], wv[5], wv[6]) + sha224_k[j] + w[j];
			t2 = SHA224_F1(wv[0]) + SHA2_MAJ(wv[0], wv[1], wv[2]);
			wv[7] = wv[6];
			wv[6] = wv[5];
			wv[5] = wv[4];
			wv[4] = wv[3] + t1;
			wv[3] = wv[2];
			wv[2] = wv[1];
			wv[1] = wv[0];
			wv[0] = t1 + t2;
		}
		for (j = 0; j < 8; j++)
			(context->m_h)[j] += wv[j];
	}
}

void sha224_init(struct sha224_t *context)
{
	if (!context)
		return;
	(context->m_h)[0] = 0xc1059ed8;
	(context->m_h)[1] = 0x367cd507;
	(context->m_h)[2] = 0x3070dd17;
	(context->m_h)[3] = 0xf70e5939;
	(context->m_h)[4] = 0xffc00b31;
	(context->m_h)[5] = 0x68581511;
	(context->m_h)[6] = 0x64f98fa7;
	(context->m_h)[7] = 0xbefa4fa4;
	(context->m_len) = 0;
	(context->m_tot_len) = 0;
}

void sha224_update(struct sha224_t *context,
		const uint8_t *message, const uint32_t len)
{
	uint32_t block_nb;
	uint32_t new_len, rem_len, tmp_len;
	const uint8_t *shifted_message = NULL;
	uint32_t j;

	if (!context || !message)
		return;
	tmp_len = SHA224_BLOCK_SIZE - (context->m_len);
	rem_len = len < tmp_len ? len : tmp_len;
	for (j = 0; j < rem_len; j++)
		(context->m_block)[(context->m_len) + j] = message[j];
	if ((context->m_len) + len < SHA224_BLOCK_SIZE) {
		(context->m_len) += len;
		return;
	}
	new_len = len - rem_len;
	block_nb = new_len / SHA224_BLOCK_SIZE;
	shifted_message = message + rem_len;
	sha224_transform(context, context->m_block, 1);
	sha224_transform(context, shifted_message, block_nb);
	rem_len = new_len % SHA224_BLOCK_SIZE;
	for (j = 0; j < rem_len; j++)
		(context->m_block)[j] = shifted_message[(block_nb << 6) + j];
	(context->m_len) = rem_len;
	(context->m_tot_len) += (block_nb + 1) << 6;
}

void sha224_final(struct sha224_t *context, uint8_t *digest)
{
	uint32_t block_nb;
	uint32_t pm_len;
	uint32_t len_b;
	uint32_t i;

	if (!context || !digest)
		return;
	block_nb = (1 + ((SHA224_BLOCK_SIZE - 9) <
		((context->m_len) % SHA224_BLOCK_SIZE)));
	len_b = ((context->m_tot_len) + (context->m_len)) << 3;
	pm_len = block_nb << 6;
	for (i = 0; i < (pm_len - (context->m_len)); i++)
		((context->m_block) + (context->m_len))[i] = 0;
	(context->m_block)[(context->m_len)] = 0x80;
	SHA2_UNPACK32(len_b, (context->m_block) + pm_len - 4);
	sha224_transform(context, (context->m_block), block_nb);
	for (i = 0; i < 7; i++)
		SHA2_UNPACK32((context->m_h)[i], &digest[i << 2]);
}
