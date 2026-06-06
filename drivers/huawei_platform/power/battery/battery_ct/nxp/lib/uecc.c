/*
 * uecc.c
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

#include "uecc.h"

#define BITS_TO_WORDS(num_bits) \
	((num_bits + ((UECC_WORD_SIZE * 8) - 1)) / (UECC_WORD_SIZE * 8))
#define BITS_TO_BYTES(num_bits) ((num_bits + 7) / 8)

#define UECC_MAX_WORDS 7

struct uecc_curve_t {
	wordcount_t num_words;
	wordcount_t num_bytes;
	bitcount_t num_n_bits;
	uecc_word_t p[UECC_MAX_WORDS];
	uecc_word_t n[UECC_MAX_WORDS];
	uecc_word_t G[UECC_MAX_WORDS * 2];
	uecc_word_t b[UECC_MAX_WORDS];
	void (*double_jacobian)(uecc_word_t *X1, uecc_word_t *Y1,
		uecc_word_t *Z1, const struct uecc_curve_t *curve);
	void (*x_side)(uecc_word_t *result, const uecc_word_t *x,
		const struct uecc_curve_t *curve);
	void (*mmod_fast)(uecc_word_t *result, uecc_word_t *product);
};

static cmpresult_t uecc_vli_cmp_unsafe(const uecc_word_t *left,
	const uecc_word_t *right, wordcount_t num_words);
static void uecc_vli_bytes_to_native(uecc_word_t *native,
	const uint8_t *bytes, int num_bytes);

static void uecc_bcopy(uint8_t *dst,
	const uint8_t *src, const uint32_t num_bytes_arg)
{
	uint32_t num_bytes = num_bytes_arg;

	while (num_bytes != 0) {
		num_bytes--;
		dst[num_bytes] = src[num_bytes];
	}
}

static void uecc_vli_clear(uecc_word_t *vli, wordcount_t num_words)
{
	wordcount_t i;

	for (i = 0; i < num_words; ++i)
		vli[i] = 0;
}

/* Constant-time comparison to zero - secure way to compare long integers */
/* Returns 1 if vli == 0, 0 otherwise. */
static uecc_word_t uecc_vli_iszero(const uecc_word_t *vli,
		wordcount_t num_words)
{
	uecc_word_t bits = 0;
	wordcount_t i;

	for (i = 0; i < num_words; ++i)
		bits |= vli[i];
	return (bits == 0);
}

/* Returns nonzero if bit 'bit' of vli is set. */
static uecc_word_t uecc_vli_testbit(const uecc_word_t *vli,
		bitcount_t bit)
{
	return (vli[bit >> UECC_WORD_BITS_SHIFT] &
			((uecc_word_t)1 << (bit & UECC_WORD_BITS_MASK)));
}

/* Counts the number of words in vli. */
static wordcount_t vli_num_digits(const uecc_word_t *vli,
		const wordcount_t max_words)
{
	wordcount_t i;

	/* Search from the end until we find a non-zero digit.
	 * We do it in reverse because we expect that most digits
	 * will be nonzero.
	 */
	for (i = max_words - 1; i >= 0 && vli[i] == 0; --i)
		;
	return (i + 1);
}

/* Counts the number of bits required to represent vli. */
static bitcount_t uecc_vli_numbits(const uecc_word_t *vli,
	const wordcount_t max_words)
{
	uecc_word_t i;
	uecc_word_t digit;
	wordcount_t num_digits = vli_num_digits(vli, max_words);

	if (num_digits == 0)
		return 0;

	digit = vli[num_digits - 1];
	for (i = 0; digit; ++i)
		digit >>= 1;

	return (((bitcount_t)(num_digits - 1) << UECC_WORD_BITS_SHIFT) + i);
}

/* Sets dest = src. */
static void uecc_vli_set(uecc_word_t *dest,
		const uecc_word_t *src, wordcount_t num_words)
{
	wordcount_t i;

	for (i = 0; i < num_words; ++i)
		dest[i] = src[i];

}

/* Returns sign of left - right. */
static cmpresult_t uecc_vli_cmp_unsafe(const uecc_word_t *left,
	const uecc_word_t *right, wordcount_t num_words)
{
	wordcount_t i;

	for (i = num_words - 1; i >= 0; --i) {
		if (left[i] > right[i])
			return 1;
		else if (left[i] < right[i])
			return -1;
	}
	return 0;
}

/* Constant-time comparison function - secure way to compare long integers */
/* Returns one if left == right, zero otherwise. */
static uecc_word_t uecc_vli_equal(const uecc_word_t *left,
	const uecc_word_t *right, wordcount_t num_words)
{
	uecc_word_t diff = 0;
	wordcount_t i;

	for (i = num_words - 1; i >= 0; --i)
		diff |= (left[i] ^ right[i]);
	return (diff == 0);
}

static uecc_word_t uecc_vli_sub(uecc_word_t *result,
				const uecc_word_t *left,
				const uecc_word_t *right,
				wordcount_t num_words);

/* Computes vli = vli >> 1. */
static void uecc_vli_rshift1(uecc_word_t *vli, wordcount_t num_words)
{
	uecc_word_t *end = vli;
	uecc_word_t carry = 0;

	vli += num_words;
	while (vli-- > end) {
		uecc_word_t temp = *vli;
		*vli = (temp >> 1) | carry;
		carry = temp << (UECC_WORD_BITS - 1);
	}
}

/* Computes result = left + right, returning carry. Can modify in place. */
static uecc_word_t uecc_vli_add(uecc_word_t *result,
				const uecc_word_t *left,
				const uecc_word_t *right,
				wordcount_t num_words)
{
	uecc_word_t carry = 0;
	wordcount_t i;

	for (i = 0; i < num_words; ++i) {
		uecc_word_t sum = left[i] + right[i] + carry;

		if (sum != left[i])
			carry = (sum < left[i]);
		result[i] = sum;
	}
	return carry;
}

/* Computes result = left - right, returning borrow. Can modify in place. */
static uecc_word_t uecc_vli_sub(uecc_word_t *result,
				const uecc_word_t *left,
				const uecc_word_t *right,
				wordcount_t num_words)
{
	uecc_word_t borrow = 0;
	wordcount_t i;

	for (i = 0; i < num_words; ++i) {
		uecc_word_t diff = left[i] - right[i] - borrow;

		if (diff != left[i])
			borrow = (diff > left[i]);
		result[i] = diff;
	}
	return borrow;
}

static void muladd(uecc_word_t a, uecc_word_t b,
		uecc_word_t *r0, uecc_word_t *r1, uecc_word_t *r2)
{
	uecc_dword_t p = (uecc_dword_t)a * b;
	uecc_dword_t r01 = ((uecc_dword_t)(*r1) << UECC_WORD_BITS) | *r0;

	r01 += p;
	*r2 += (r01 < p);
	*r1 = r01 >> UECC_WORD_BITS;
	*r0 = (uecc_word_t)r01;
}

static void uecc_vli_mult(uecc_word_t *result,
		const uecc_word_t *left,
		const uecc_word_t *right,
		wordcount_t num_words)
{
	uecc_word_t r0 = 0;
	uecc_word_t r1 = 0;
	uecc_word_t r2 = 0;
	wordcount_t i, k;

	/* Compute each digit of result in sequence, maintaining the carries. */
	for (k = 0; k < num_words; ++k) {
		for (i = 0; i <= k; ++i)
			muladd(left[i], right[k - i], &r0, &r1, &r2);
		result[k] = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	for (k = num_words; k < num_words * 2 - 1; ++k) {
		for (i = (k + 1) - num_words; i < num_words; ++i)
			muladd(left[i], right[k - i], &r0, &r1, &r2);
		result[k] = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}
	result[num_words * 2 - 1] = r0;
}

static void mul2add(uecc_word_t a, uecc_word_t b,
		uecc_word_t *r0, uecc_word_t *r1, uecc_word_t *r2)
{
	uecc_dword_t p = (uecc_dword_t)a * b;
	uecc_dword_t r01 = ((uecc_dword_t)(*r1) << UECC_WORD_BITS) | *r0;

	*r2 += (p >> (UECC_WORD_BITS * 2 - 1));
	p *= 2;
	r01 += p;
	*r2 += (r01 < p);
	*r1 = r01 >> UECC_WORD_BITS;
	*r0 = (uecc_word_t)r01;
}

static void uecc_vli_square(uecc_word_t *result,
		const uecc_word_t *left, wordcount_t num_words)
{
	uecc_word_t r0 = 0;
	uecc_word_t r1 = 0;
	uecc_word_t r2 = 0;
	wordcount_t i, k;

	for (k = 0; k < num_words * 2 - 1; ++k) {
		uecc_word_t min = (k < num_words ? 0 : (k + 1) - num_words);

		for (i = min; i <= k && i <= k - i; ++i) {
			if (i < k - i)
				mul2add(left[i], left[k - i], &r0, &r1, &r2);
			else
				muladd(left[i], left[k - i], &r0, &r1, &r2);
		}
		result[k] = r0;
		r0 = r1;
		r1 = r2;
		r2 = 0;
	}

	result[num_words * 2 - 1] = r0;
}

/*
 * Computes result = (left + right) % mod.
 * Assumes that left < mod and right < mod,
 * and that result does not overlap mod.
 */
static void uecc_vli_modadd(uecc_word_t *result,
		const uecc_word_t *left,
		const uecc_word_t *right,
		const uecc_word_t *mod,
		wordcount_t num_words)
{
	uecc_word_t carry = uecc_vli_add(result, left, right, num_words);

	if (carry || uecc_vli_cmp_unsafe(mod, result, num_words) != 1)
		/* result > mod (result = mod + remainder),
		 * so subtract mod to get remainder.
		 */
		uecc_vli_sub(result, result, mod, num_words);
}

/* Computes result = (left - right) % mod.
 * Assumes that left < mod and right < mod,
 * and that result does not overlap mod.
 */
static void uecc_vli_modsub(uecc_word_t *result,
		const uecc_word_t *left,
		const uecc_word_t *right,
		const uecc_word_t *mod,
		wordcount_t num_words)
{
	uecc_word_t l_borrow = uecc_vli_sub(result, left, right, num_words);

	if (l_borrow)
		/* In this case, result == -diff == (max int) - diff.
		 * Since -x % d == d - x, we can get the correct result\
		 * from result + mod (with overflow).
		 */
		uecc_vli_add(result, result, mod, num_words);
}

/* Computes result = product % mod, where product is 2N words long. */
/* Currently only designed to work for curve_p or curve_n. */
static void uecc_vli_mmod(uecc_word_t *result,
		uecc_word_t *product,
		const uecc_word_t *mod,
		wordcount_t num_words)
{
	uecc_word_t mod_multiple[2 * UECC_MAX_WORDS];
	uecc_word_t tmp[2 * UECC_MAX_WORDS];
	uecc_word_t *v[2];
	uecc_word_t index;

	/* Shift mod so its highest set bit is at the maximum position. */
	bitcount_t shift = (num_words * 2 * UECC_WORD_BITS) -
		uecc_vli_numbits(mod, num_words);
	wordcount_t word_shift = shift / UECC_WORD_BITS;
	wordcount_t bit_shift = shift % UECC_WORD_BITS;
	uecc_word_t carry = 0;

	v[0] = tmp;
	v[1] = product;
	uecc_vli_clear(mod_multiple, word_shift);
	if (bit_shift > 0) {
		for (index = 0; index < (uecc_word_t)num_words; ++index) {
			mod_multiple[word_shift + index] =
				(mod[index] << bit_shift) | carry;
			carry = mod[index] >> (UECC_WORD_BITS - bit_shift);
		}
	} else
		uecc_vli_set(mod_multiple + word_shift, mod, num_words);

	for (index = 1; shift >= 0; --shift) {
		uecc_word_t borrow = 0;
		wordcount_t i;

		for (i = 0; i < num_words * 2; ++i) {
			uecc_word_t diff = v[index][i] -
				mod_multiple[i] - borrow;
			if (diff != v[index][i])
				borrow = (diff > v[index][i]);
			v[1 - index][i] = diff;
		}
		/* Swap the index if there was no borrow */
		index = !(index ^ borrow);
		uecc_vli_rshift1(mod_multiple, num_words);
		mod_multiple[num_words - 1] |=
			mod_multiple[num_words] << (UECC_WORD_BITS - 1);

		uecc_vli_rshift1(mod_multiple + num_words, num_words);
	}
	uecc_vli_set(result, v[index], num_words);
}

/* Computes result = (left * right) % mod. */
static void uecc_vli_modmult(uecc_word_t *result,
		const uecc_word_t *left,
		const uecc_word_t *right,
		const uecc_word_t *mod,
		wordcount_t num_words)
{
	uecc_word_t product[2 * UECC_MAX_WORDS];

	uecc_vli_mult(product, left, right, num_words);
	uecc_vli_mmod(result, product, mod, num_words);
}

static void uecc_vli_modmult_fast(uecc_word_t *result,
		const uecc_word_t *left,
		const uecc_word_t *right,
		const struct uecc_curve_t *curve)
{
	uecc_word_t product[2 * UECC_MAX_WORDS];

	uecc_vli_mult(product, left, right, curve->num_words);
	curve->mmod_fast(result, product);
}

static void uecc_vli_modsquare_fast(uecc_word_t *result,
		const uecc_word_t *left, const struct uecc_curve_t *curve)
{
	uecc_word_t product[2 * UECC_MAX_WORDS];

	uecc_vli_square(product, left, curve->num_words);
	curve->mmod_fast(result, product);
}

#define EVEN(vli) (!(vli[0] & 1))
static void vli_modinv_update(uecc_word_t *uv,
		const uecc_word_t *mod, wordcount_t num_words)
{
	uecc_word_t carry = 0;

	if (!EVEN(uv))
		carry = uecc_vli_add(uv, uv, mod, num_words);
	uecc_vli_rshift1(uv, num_words);
	if (carry)
		uv[num_words - 1] |= HIGH_BIT_SET;
}

/* Computes result = (1 / input) % mod. All VLIs are the same size.
 * See "From Euclid's GCD to Montgomery Multiplication to the Great Divide"
 */
static void uecc_vli_modinv(uecc_word_t *result,
		const uecc_word_t *input,
		const uecc_word_t *mod,
		wordcount_t num_words)
{
	uecc_word_t a[UECC_MAX_WORDS], b[UECC_MAX_WORDS];
	uecc_word_t u[UECC_MAX_WORDS], v[UECC_MAX_WORDS];
	cmpresult_t cmpResult;

	if (uecc_vli_iszero(input, num_words)) {
		uecc_vli_clear(result, num_words);
		return;
	}

	uecc_vli_set(a, input, num_words);
	uecc_vli_set(b, mod, num_words);
	uecc_vli_clear(u, num_words);
	u[0] = 1;
	uecc_vli_clear(v, num_words);
	while ((cmpResult = uecc_vli_cmp_unsafe(a, b, num_words)) != 0) {
		if (EVEN(a)) {
			uecc_vli_rshift1(a, num_words);
			vli_modinv_update(u, mod, num_words);
		} else if (EVEN(b)) {
			uecc_vli_rshift1(b, num_words);
			vli_modinv_update(v, mod, num_words);
		} else if (cmpResult > 0) {
			uecc_vli_sub(a, a, b, num_words);
			uecc_vli_rshift1(a, num_words);
			if (uecc_vli_cmp_unsafe(u, v, num_words) < 0)
				uecc_vli_add(u, u, mod, num_words);
			uecc_vli_sub(u, u, v, num_words);
			vli_modinv_update(u, mod, num_words);
		} else {
			uecc_vli_sub(b, b, a, num_words);
			uecc_vli_rshift1(b, num_words);
			if (uecc_vli_cmp_unsafe(v, u, num_words) < 0)
				uecc_vli_add(v, v, mod, num_words);
			uecc_vli_sub(v, v, u, num_words);
			vli_modinv_update(v, mod, num_words);
		}
	}
	uecc_vli_set(result, u, num_words);
}

/* ------ Point operations ------ */

/* Returns 1 if 'point' is the point at infinity, 0 otherwise. */
#define ECCPOINT_ISZERO(point, curve)	\
		uecc_vli_iszero((point), (curve)->num_words * 2)

/*
 * Point multiplication algorithm
 * using Montgomery's ladder with co-Z coordinates.
 * From http://eprint.iacr.org/2011/338.pdf
 */

/* Modify (x1, y1) => (x1 * z^2, y1 * z^3) */
static void apply_z(uecc_word_t *x1, uecc_word_t *y1,
		const uecc_word_t *const z, const struct uecc_curve_t *curve)
{
	uecc_word_t t1[UECC_MAX_WORDS];

	uecc_vli_modsquare_fast(t1, z, curve);    /* z^2 */
	uecc_vli_modmult_fast(x1, x1, t1, curve); /* x1 * z^2 */
	uecc_vli_modmult_fast(t1, t1, z, curve);  /* z^3 */
	uecc_vli_modmult_fast(y1, y1, t1, curve); /* y1 * z^3 */
}

/*
 * Input P = (x1, y1, Z), Q = (x2, y2, Z)
 * Output P' = (x1', y1', Z3), P + Q = (x3, y3, Z3)
 * or P => P', Q => P + Q
 */
static void xycz_add(uecc_word_t *x1, uecc_word_t *y1,
		     uecc_word_t *x2, uecc_word_t *y2,
		     const struct uecc_curve_t *curve)
{
	/* t1 = x1, t2 = y1, t3 = x2, t4 = y2 */
	uecc_word_t t5[UECC_MAX_WORDS];
	wordcount_t num_words = curve->num_words;

	/* t5 = x2 - x1 */
	uecc_vli_modsub(t5, x2, x1, curve->p, num_words);
	/* t5 = (x2 - x1)^2 = A */
	uecc_vli_modsquare_fast(t5, t5, curve);
	/* t1 = x1*A = B */
	uecc_vli_modmult_fast(x1, x1, t5, curve);
	/* t3 = x2*A = C */
	uecc_vli_modmult_fast(x2, x2, t5, curve);
	/* t4 = y2 - y1 */
	uecc_vli_modsub(y2, y2, y1, curve->p, num_words);
	/* t5 = (y2 - y1)^2 = D */
	uecc_vli_modsquare_fast(t5, y2, curve);

	/* t5 = D - B */
	uecc_vli_modsub(t5, t5, x1, curve->p, num_words);
	/* t5 = D - B - C = x3 */
	uecc_vli_modsub(t5, t5, x2, curve->p, num_words);
	/* t3 = C - B */
	uecc_vli_modsub(x2, x2, x1, curve->p, num_words);
	/* t2 = y1*(C - B) */
	uecc_vli_modmult_fast(y1, y1, x2, curve);
	/* t3 = B - x3 */
	uecc_vli_modsub(x2, x1, t5, curve->p, num_words);
	/* t4 = (y2 - y1)*(B - x3) */
	uecc_vli_modmult_fast(y2, y2, x2, curve);
	/* t4 = y3 */
	uecc_vli_modsub(y2, y2, y1, curve->p, num_words);

	uecc_vli_set(x2, t5, num_words);
}

/* -------- ECDSA code -------- */
static void bits2int(uecc_word_t *native, const uint8_t *bits,
		uint32_t bits_size, const struct uecc_curve_t *curve)
{
	uint32_t num_n_bytes = BITS_TO_BYTES(curve->num_n_bits);
	uint32_t num_n_words = BITS_TO_WORDS(curve->num_n_bits);
	uint32_t shift;
	uecc_word_t carry;
	uecc_word_t *ptr;

	if (bits_size > num_n_bytes)
		bits_size = num_n_bytes;

	uecc_vli_clear(native, num_n_words);
	/* Convert big-endian input to little endian vli */
	uecc_vli_bytes_to_native(native, bits, bits_size);
	if (bits_size * 8 <= (uint32_t)curve->num_n_bits)
		return;
	shift = bits_size * 8 - curve->num_n_bits;
	carry = 0;
	ptr = native + num_n_words;
	while (ptr-- > native) {
		uecc_word_t temp = *ptr;
		*ptr = (temp >> shift) | carry;
		carry = temp << (UECC_WORD_BITS - shift);
	}

	/* Reduce mod curve_n */
	if (uecc_vli_cmp_unsafe(curve->n, native, num_n_words) != 1)
		uecc_vli_sub(native, native, curve->n, num_n_words);
}

#define NUM_BYTES_SECP224R1 28
#define BYTES_TO_WORDS_4(a, b, c, d) (0x##d##c##b##a)

static void double_jacobian_default(uecc_word_t *X1,
		uecc_word_t *Y1,
		uecc_word_t *Z1,
		const struct uecc_curve_t *curve)
{
	/* t1 = X, t2 = Y, t3 = Z */
	uecc_word_t t4[UECC_MAX_WORDS];
	uecc_word_t t5[UECC_MAX_WORDS];
	wordcount_t num_words = curve->num_words;

	if (uecc_vli_iszero(Z1, num_words))
		return;

	/* t4 = y1^2 */
	uecc_vli_modsquare_fast(t4, Y1, curve);
	/* t5 = x1*y1^2 = A */
	uecc_vli_modmult_fast(t5, X1, t4, curve);
	/* t4 = y1^4 */
	uecc_vli_modsquare_fast(t4, t4, curve);
	/* t2 = y1*z1 = z3 */
	uecc_vli_modmult_fast(Y1, Y1, Z1, curve);
	/* t3 = z1^2 */
	uecc_vli_modsquare_fast(Z1, Z1, curve);
	/* t1 = x1 + z1^2 */
	uecc_vli_modadd(X1, X1, Z1, curve->p, num_words);
	/* t3 = 2*z1^2 */
	uecc_vli_modadd(Z1, Z1, Z1, curve->p, num_words);
	/* t3 = x1 - z1^2 */
	uecc_vli_modsub(Z1, X1, Z1, curve->p, num_words);
	/* t1 = x1^2 - z1^4 */
	uecc_vli_modmult_fast(X1, X1, Z1, curve);
	/* t3 = 2*(x1^2 - z1^4) */
	uecc_vli_modadd(Z1, X1, X1, curve->p, num_words);
	/* t1 = 3*(x1^2 - z1^4) */
	uecc_vli_modadd(X1, X1, Z1, curve->p, num_words);
	if (uecc_vli_testbit(X1, 0)) {
		uecc_word_t l_carry = uecc_vli_add(X1, X1, curve->p, num_words);

		uecc_vli_rshift1(X1, num_words);
		X1[num_words - 1] |= l_carry << (UECC_WORD_BITS - 1);
	} else
		uecc_vli_rshift1(X1, num_words);
	/* t1 = 3/2*(x1^2 - z1^4) = B */
	/* t3 = B^2 */
	uecc_vli_modsquare_fast(Z1, X1, curve);
	/* t3 = B^2 - A */
	uecc_vli_modsub(Z1, Z1, t5,
			curve->p, num_words);
	/* t3 = B^2 - 2A = x3 */
	uecc_vli_modsub(Z1, Z1, t5,
			curve->p, num_words);
	/* t5 = A - x3 */
	uecc_vli_modsub(t5, t5, Z1,
			curve->p, num_words);
	/* t1 = B * (A - x3) */
	uecc_vli_modmult_fast(X1, X1, t5, curve);
	/* t4 = B * (A - x3) - y1^4 = y3 */
	uecc_vli_modsub(t4, X1, t4,
			curve->p, num_words);

	uecc_vli_set(X1, Z1, num_words);
	uecc_vli_set(Z1, Y1, num_words);
	uecc_vli_set(Y1, t4, num_words);
}

/* Computes result = x^3 + ax + b. result must not overlap x. */
static void x_side_default(uecc_word_t *result,
		const uecc_word_t *x, const struct uecc_curve_t *curve)
{
	/* -a = 3 */
	uecc_word_t _3[UECC_MAX_WORDS] = {3};
	wordcount_t num_words = curve->num_words;

	/* r = x^2 */
	uecc_vli_modsquare_fast(result, x, curve);
	/* r = x^2 - 3 */
	uecc_vli_modsub(result, result, _3, curve->p, num_words);
	/* r = x^3 - 3x */
	uecc_vli_modmult_fast(result, result, x, curve);
	/* r = x^3 - 3x + b */
	uecc_vli_modadd(result, result, curve->b,
			curve->p, num_words);
}

static void vli_mmod_fast_secp224r1(uecc_word_t *result,
		uecc_word_t *product);

static const struct uecc_curve_t curve_secp224r1 = {
	NUM_WORDS_SECP224R1,
	NUM_BYTES_SECP224R1,
	224, /* num_n_bits */
	{
		BYTES_TO_WORDS_4(01, 00, 00, 00),
		BYTES_TO_WORDS_4(00, 00, 00, 00),
		BYTES_TO_WORDS_4(00, 00, 00, 00),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
	},
	{
		BYTES_TO_WORDS_4(3D, 2A, 5C, 5C),
		BYTES_TO_WORDS_4(45, 29, DD, 13),
		BYTES_TO_WORDS_4(3E, F0, B8, E0),
		BYTES_TO_WORDS_4(A2, 16, FF, FF),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
		BYTES_TO_WORDS_4(FF, FF, FF, FF),
	},
	{
		BYTES_TO_WORDS_4(21, 1D, 5C, 11),
		BYTES_TO_WORDS_4(D6, 80, 32, 34),
		BYTES_TO_WORDS_4(22, 11, C2, 56),
		BYTES_TO_WORDS_4(D3, C1, 03, 4A),
		BYTES_TO_WORDS_4(B9, 90, 13, 32),
		BYTES_TO_WORDS_4(7F, BF, B4, 6B),
		BYTES_TO_WORDS_4(BD, 0C, 0E, B7),
		BYTES_TO_WORDS_4(34, 7E, 00, 85),
		BYTES_TO_WORDS_4(99, 81, D5, 44),
		BYTES_TO_WORDS_4(64, 47, 07, 5A),
		BYTES_TO_WORDS_4(A0, 75, 43, CD),
		BYTES_TO_WORDS_4(E6, DF, 22, 4C),
		BYTES_TO_WORDS_4(FB, 23, F7, B5),
		BYTES_TO_WORDS_4(88, 63, 37, BD),
	},
	{
		BYTES_TO_WORDS_4(B4, FF, 55, 23),
		BYTES_TO_WORDS_4(43, 39, 0B, 27),
		BYTES_TO_WORDS_4(BA, D8, BF, D7),
		BYTES_TO_WORDS_4(B7, B0, 44, 50),
		BYTES_TO_WORDS_4(56, 32, 41, F5),
		BYTES_TO_WORDS_4(AB, B3, 04, 0C),
		BYTES_TO_WORDS_4(85, 0A, 05, B4),
	},
	&double_jacobian_default,
	&x_side_default,
	&vli_mmod_fast_secp224r1,
};

const struct uecc_curve_t *uecc_secp224r1(void)
{
	return &curve_secp224r1;
}

/*
 * Computes result = product % curve_p
 * from http://www.nsa.gov/ia/_files/nist-routines.pdf
 */
static void vli_mmod_fast_secp224r1(uint32_t *result, uint32_t *product)
{
	uint32_t tmp[NUM_WORDS_SECP224R1];
	int carry;

	/* t */
	uecc_vli_set(result, product, NUM_WORDS_SECP224R1);

	/* s1 */
	tmp[0] = tmp[1] = tmp[2] = 0;
	tmp[3] = product[7];
	tmp[4] = product[8];
	tmp[5] = product[9];
	tmp[6] = product[10];
	carry = uecc_vli_add(result, result, tmp, NUM_WORDS_SECP224R1);

	/* s2 */
	tmp[3] = product[11];
	tmp[4] = product[12];
	tmp[5] = product[13];
	tmp[6] = 0;
	carry += uecc_vli_add(result, result, tmp, NUM_WORDS_SECP224R1);

	/* d1 */
	tmp[0] = product[7];
	tmp[1] = product[8];
	tmp[2] = product[9];
	tmp[3] = product[10];
	tmp[4] = product[11];
	tmp[5] = product[12];
	tmp[6] = product[13];
	carry -= uecc_vli_sub(result, result, tmp, NUM_WORDS_SECP224R1);

	/* d2 */
	tmp[0] = product[11];
	tmp[1] = product[12];
	tmp[2] = product[13];
	tmp[3] = tmp[4] = tmp[5] = tmp[6] = 0;
	carry -= uecc_vli_sub(result, result, tmp, NUM_WORDS_SECP224R1);

	if (carry < 0) {
		do {
			carry += uecc_vli_add(result, result,
				curve_secp224r1.p, NUM_WORDS_SECP224R1);
		} while (carry < 0);
	} else {
		while (carry || uecc_vli_cmp_unsafe(curve_secp224r1.p,
				result, NUM_WORDS_SECP224R1) != 1) {
			carry -= uecc_vli_sub(result, result,
				curve_secp224r1.p, NUM_WORDS_SECP224R1);
		}
	}
}

static int uecc_parse_der_encoded_signature(uint8_t *r, uint8_t *s,
		const uint8_t *dersig, wordcount_t num_bytes)
{
	wordcount_t outerLength;
	wordcount_t innerLength;
	wordcount_t i;
	wordcount_t idx;

/* Parse DER encoded ECDSA signature into little endian (r, s) padded values */
	idx = 0;
	if (dersig[idx++] != 0x30)
		return 0;
	outerLength = dersig[idx++];
	if (outerLength < 8 || outerLength > ((num_bytes * 2) + 6))
		return 0;
	if (dersig[idx++] != 0x02)
		return 0;
	innerLength = dersig[idx++];
	if (dersig[idx] == 0x00) {
		idx++;
		innerLength--;
	}
	if (innerLength < 8 || innerLength > num_bytes)
		return 0;
	for (i = 0; i < (num_bytes - innerLength); i++)
		r[num_bytes - 1 - i] = 0x00;
	for (; i < num_bytes; i++)
		r[num_bytes - 1 - i] = dersig[idx++];
	if (dersig[idx++] != 0x02)
		return 0;
	innerLength = dersig[idx++];
	if (dersig[idx] == 0x00) {
		idx++;
		innerLength--;
	}
	if (innerLength < 8 || innerLength > num_bytes || (idx + innerLength)
			!= (2 + outerLength))
		return 0;
	for (i = 0; i < (num_bytes - innerLength); i++)
		s[num_bytes - 1 - i] = 0x00;
	for (; i < num_bytes; i++)
		s[num_bytes - 1 - i] = dersig[idx++];

	return 1;
}

static void uecc_vli_bytes_to_native(uecc_word_t *native,
		const uint8_t *bytes, int num_bytes)
{
	wordcount_t i;
	unsigned int b;

	uecc_vli_clear(native, (num_bytes + (UECC_WORD_SIZE - 1)) /
		UECC_WORD_SIZE);
	for (i = 0; i < num_bytes; ++i) {
		b = num_bytes - 1 - i;
		native[b / UECC_WORD_SIZE] |=
			(uecc_word_t)bytes[i] << (8 * (b % UECC_WORD_SIZE));
	}
}

uint32_t uecc_verify(const uecc_word_t *public_key, const uint8_t *message_hash,
		     uint32_t hash_size, const uint8_t *signature,
		     bool signature_is_der_encoded,
		     const struct uecc_curve_t *curve)
{
	uecc_word_t u1[UECC_MAX_WORDS], u2[UECC_MAX_WORDS],
		z[UECC_MAX_WORDS], sum[UECC_MAX_WORDS * 2],
		rx[UECC_MAX_WORDS], ry[UECC_MAX_WORDS],
		tx[UECC_MAX_WORDS], ty[UECC_MAX_WORDS], tz[UECC_MAX_WORDS],
		r[UECC_MAX_WORDS], s[UECC_MAX_WORDS];
	const uecc_word_t *points[4] = { 0 };
	const uecc_word_t *point = NULL;
	bitcount_t num_bits, i;
	wordcount_t num_words;
	wordcount_t num_n_words;

	if (!public_key || !message_hash || !signature || !curve)
		return 0;
	num_words = curve->num_words;
	num_n_words = BITS_TO_WORDS(curve->num_n_bits);
	rx[num_n_words - 1] = 0;
	r[num_n_words - 1] = 0;
	s[num_n_words - 1] = 0;

	if (signature_is_der_encoded == false) {
		uecc_bcopy((uint8_t *)r, signature, curve->num_bytes);
		uecc_bcopy((uint8_t *)s, signature + curve->num_bytes,
			curve->num_bytes);
	} else {
		if (uecc_parse_der_encoded_signature((uint8_t *)r,
		    (uint8_t *)s, signature, curve->num_bytes) == 0)
			return 0;
	}

	/* r, s must not be 0. */
	if (uecc_vli_iszero(r, num_words) || uecc_vli_iszero(s, num_words))
		return 0;

	/* r, s must be < n. */
	if (uecc_vli_cmp_unsafe(curve->n, r, num_n_words) != 1 ||
	    uecc_vli_cmp_unsafe(curve->n, s, num_n_words) != 1)
		return 0;

	/* Calculate u1 and u2. */
	uecc_vli_modinv(z, s, curve->n, num_n_words); /* z = 1/s */
	u1[num_n_words - 1] = 0;
	bits2int(u1, message_hash, hash_size, curve);
	uecc_vli_modmult(u1, u1, z, curve->n, num_n_words); /* u1 = e/s */
	uecc_vli_modmult(u2, r, z, curve->n, num_n_words);  /* u2 = r/s */

	/* Calculate sum = G + Q. */
	uecc_vli_set(sum, public_key, num_words);
	uecc_vli_set(sum + num_words, public_key + num_words, num_words);
	uecc_vli_set(tx, curve->G, num_words);
	uecc_vli_set(ty, curve->G + num_words, num_words);
	uecc_vli_modsub(z, sum, tx, curve->p, num_words); /* z = x2 - x1 */
	xycz_add(tx, ty, sum, sum + num_words, curve);
	uecc_vli_modinv(z, z, curve->p, num_words); /* z = 1/z */
	apply_z(sum, sum + num_words, z, curve);

	/* Use Shamir's trick to calculate u1*G + u2*Q */
	points[0] = 0;
	points[1] = curve->G;
	points[2] = public_key;
	points[3] = sum;

	i = uecc_vli_numbits(u1, num_n_words);
	num_bits = uecc_vli_numbits(u2, num_n_words);
	num_bits = (i > num_bits) ? i : num_bits;

	point = points[(!!uecc_vli_testbit(u1, num_bits - 1)) |
		((!!uecc_vli_testbit(u2, num_bits - 1)) << 1)];
	uecc_vli_set(rx, point, num_words);
	uecc_vli_set(ry, point + num_words, num_words);
	uecc_vli_clear(z, num_words);
	z[0] = 1;

	for (i = num_bits - 2; i >= 0; --i) {
		uecc_word_t index;

		curve->double_jacobian(rx, ry, z, curve);

		index = (!!uecc_vli_testbit(u1, i)) |
			((!!uecc_vli_testbit(u2, i)) << 1);
		point = points[index];
		if (point) {
			uecc_vli_set(tx, point, num_words);
			uecc_vli_set(ty, point + num_words, num_words);
			apply_z(tx, ty, z, curve);
			/* Z = x2 - x1 */
			uecc_vli_modsub(tz, rx, tx, curve->p, num_words);
			xycz_add(tx, ty, rx, ry, curve);
			uecc_vli_modmult_fast(z, z, tz, curve);
		}
	}

	uecc_vli_modinv(z, z, curve->p, num_words); /* Z = 1/Z */
	apply_z(rx, ry, z, curve);

	/* v = x1 (mod n) */
	if (uecc_vli_cmp_unsafe(curve->n, rx, num_n_words) != 1)
		uecc_vli_sub(rx, rx, curve->n, num_n_words);

	/* Accept only if v == r */
	return (uint32_t)(uecc_vli_equal(rx, r, num_words));
}

