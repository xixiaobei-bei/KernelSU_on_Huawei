#include <linux/types.h>
#include <linux/lz4m.h>

#define memcpy __builtin_memcpy
#define USHRT_MAX 4096U

#define LZ4M_MATCH_SEARCH_LOOP_SIZE 4


void store2(void *ptr, uint16_t data)
{
	uint16_t *ptr1 = (uint16_t *)ptr;
	*ptr1 = data;
}

void store4(void *ptr, uint32_t data)
{
	uint32_t *ptr1 = (uint32_t *)ptr;
	*ptr1 = data;
}

void store8(void *ptr, uint64_t data)
{
	uint64_t *ptr1 = (uint64_t *)ptr;
	*ptr1 = data;
}

uint16_t load2(const void *ptr)
{
	uint16_t data;

	data = (uint16_t)*(uint16_t *)ptr;
	return data;
}

uint32_t load4(const void *ptr)
{
	uint32_t data;

	data = (uint32_t)*(uint32_t *)ptr;
	return data;
}

uint64_t load8(const void *ptr)
{
	uint64_t data;

	data = (uint64_t)*(uint64_t *)ptr;
	return data;
}

void copy16(void *dst, const void *src) { memcpy(dst, src, 16); }
void copy32(void *dst, const void *src) { memcpy(dst, src, 32); }

static inline uint32_t clamp(uint32_t x, uint32_t max)
{
	return x > max ? max : x;
}

static inline uint32_t lz4m_hash(uint32_t x)
{
	return (x * 2654435761U) >> (32 - LZ4M_COMPRESS_HASH_BITS);
}

static inline uint8_t *copy_literal(uint8_t *dst,
		const uint8_t *__restrict src, uint32_t L) {
	uint8_t *end = dst + L;

	copy16(dst, src);
	dst += 16;
	src += 16;

	while (dst < end) {
		copy32(dst, src);
		dst += 32;
		src += 32;
	}

	return end;
}

static inline void lz4m_fill16(uint8_t *ptr)
{
	store8(ptr, -1);
	store8(ptr + 8, -1);
}

static inline uint8_t *lz4m_store_length(uint8_t *dst,
		const uint8_t * const end, uint32_t L) {
	(void)end;
	while (L >= 17 * 255) {
		lz4m_fill16(dst);
		dst += 16;
		L -= 16 * 255;
	}
	lz4m_fill16(dst);
	dst += L / 255;
	*dst++ = L % 255;
	return dst;
}



static uint8_t *lz4m_emit_match(uint32_t L, uint32_t M, uint32_t D,
		uint8_t *__restrict dst,
		const uint8_t *const end,
		const uint8_t *__restrict src)
{
	uint32_t token = 0;

	L /= 4;
	M /= 4;
	D /= 4;

	token = (clamp(L, LZ4M_LMASK) << LZ4M_LSHIFT) |
		(clamp(M, LZ4M_MMASK) << LZ4M_MSHIFT) | D;
	store2(dst, token);
	dst += 2;
	if (L >= LZ4M_LMASK) {
		dst = lz4m_store_length(dst, end, L - LZ4M_LMASK);
		if (dst == 0 || dst + L >= end)
			return NULL;
	}
	dst = copy_literal(dst, src, L<<2);
	if (M >= LZ4M_MMASK) {
		dst = lz4m_store_length(dst, end, M - LZ4M_MMASK);
		if (dst == 0)
			return NULL;
	}
	return dst;
}

static void _lz4m_encode(uint8_t **dst_ptr,
		size_t dst_size,
		unsigned char **src_ptr,
		const unsigned char *src_begin,
		size_t src_size,
		lz4m_hash_entry_t hash_table[LZ4M_COMPRESS_HASH_ENTRIES],
		int skip_final_literals)
{
	uint8_t *dst = *dst_ptr;
	uint8_t *end = dst + dst_size - LZ4M_GOFAST_SAFETY_MARGIN;

	const uint8_t *src = *src_ptr;
	const uint8_t *src_end = src + src_size - LZ4M_GOFAST_SAFETY_MARGIN;
	const uint8_t *match_begin = 0;
	const uint8_t *match_end = 0;

	while (dst < end) {
		ptrdiff_t match_distance = 0;

		uint64_t this;
		int tmp;
		uint32_t hashx;
		uint32_t token = 0;
		size_t src_remaining = 0;

		for (match_begin = src; match_begin < src_end;
				match_begin += 8) {
			int pos = (int)(match_begin - src_begin);

			this = load8(match_begin);
			tmp = this&0xffffffff;
			hashx = lz4m_hash(tmp);
			if (hash_table[hashx].word == tmp &&
				hash_table[hashx].offset != 0x80000000) {

				match_distance = pos - hash_table[hashx].offset;
				hash_table[hashx].offset = pos;
				hash_table[hashx].word = tmp;
				match_end = match_begin + 4;
				goto GOT_MATCH;
			}
			hash_table[hashx].offset = pos;
			hash_table[hashx].word = tmp;
			tmp = this >> 32;
			hashx = lz4m_hash(tmp);
			pos += 4;

			if (hash_table[hashx].word == tmp &&
				hash_table[hashx].offset != 0x80000000) {

				match_distance = pos - hash_table[hashx].offset;
				hash_table[hashx].offset = pos;
				hash_table[hashx].word = tmp;
				match_begin += 4;
				match_end = match_begin + 4;
				goto GOT_MATCH;
			}
			hash_table[hashx].offset = pos;
			hash_table[hashx].word = tmp;
		}

		if (skip_final_literals) {
			*src_ptr = (uint8_t *) src;
			*dst_ptr = dst;
			return;
		}

		src_remaining = src_end + LZ4M_GOFAST_SAFETY_MARGIN - src;
		token = 0;
		src_remaining = (src_remaining >> 2);
		if (src_remaining < LZ4M_LMASK) {
			token |= src_remaining << LZ4M_LSHIFT;
			store2(dst, token);
			dst += 2;
			src_remaining <<= 2;
			memcpy(dst, src, src_remaining);
			dst += src_remaining;
		} else {
			token |= LZ4M_LMASK << LZ4M_LSHIFT;
			store2(dst, token);
			dst += 2;
			dst = lz4m_store_length(dst, end,
					(uint32_t)(src_remaining - LZ4M_LMASK));
			src_remaining <<= 2;
			if (dst == 0 || dst + src_remaining >= end)
				return;

			memcpy(dst, src, src_remaining);
			dst += src_remaining;
		}
		*dst_ptr = dst;
		*src_ptr = (uint8_t *)src + src_remaining;

		return;

GOT_MATCH:
		{
			const uint8_t *ref_end = match_end - match_distance;
			const uint8_t *match_begin_min = src_begin +
				match_distance;
			const uint8_t *ref_begin = match_begin -
				match_distance;

			match_begin_min = (match_begin_min < src) ?
				src : match_begin_min;

			while (match_end < src_end) {
				uint32_t ref_value = load4(ref_end);
				uint32_t matchend_value = load4(match_end);

				if (ref_value != matchend_value)
					break;
				match_end += LZ4M_MATCH_SEARCH_LOOP_SIZE;
				ref_end += LZ4M_MATCH_SEARCH_LOOP_SIZE;
			}

			while (match_begin > match_begin_min+4
				&& load4(ref_begin-4) == load4(match_begin-4)) {
				match_begin -= LZ4M_MATCH_SEARCH_LOOP_SIZE;
				ref_begin -= LZ4M_MATCH_SEARCH_LOOP_SIZE;
			}
		}
		dst = lz4m_emit_match((uint32_t)(match_begin - src),
				(uint32_t)(match_end - match_begin),
				(uint32_t)match_distance, dst, end, src);
		if (!dst)
			return;

		src = match_end;

		*dst_ptr = dst;
		*src_ptr = (uint8_t *)src;
	}
}


size_t lz4m_fast_encode(const unsigned char *src_buffer, size_t src_size,
		unsigned char *dst_buffer, size_t *dst_size,
		lz4m_hash_entry_t hash_table[LZ4M_COMPRESS_HASH_ENTRIES])
{
	const lz4m_hash_entry_t HASH_FILL = {	.offset = 0x80000000,
						.word = 0x0 };
	const unsigned char *src = src_buffer;
	unsigned char *dst = dst_buffer;
	const size_t BLOCK_SIZE = 0x7ffff000;
	int i;
	unsigned long src_to_encode;
	size_t  dst_used, src_used;
	unsigned char *dst_start;
	unsigned char *src_start;

	if (src_size % 4096 != 0)
		return -1;

	while (src_size > 0) {
		for (i = 0; i < LZ4M_COMPRESS_HASH_ENTRIES;) {
			hash_table[i++] = HASH_FILL;
			hash_table[i++] = HASH_FILL;
			hash_table[i++] = HASH_FILL;
			hash_table[i++] = HASH_FILL;
		}

		src_to_encode = src_size > BLOCK_SIZE ? BLOCK_SIZE : src_size;

		dst_start = dst;
		src_start = (unsigned char *)src;
		_lz4m_encode(&dst, *dst_size, (unsigned char **)&src,
				src, src_to_encode, hash_table,
				src_to_encode < src_size);

		//src_to_encode: size of content to compress this time
		//src_used: size of content after this compression process

		dst_used = dst - dst_start;
		src_used = (size_t)(src - src_start);
		//src_to_encode == src_size means the last block to compress;

		if (src_to_encode == src_size && src_used < src_to_encode)
			return -1;

		if (src_to_encode < src_size &&
				src_to_encode - src_used >= (1<<16))
			return -2;
		// Update counters (SRC and DST already have been updated)
		src_size -= src_used;
		*dst_size -= dst_used;
	}

	*dst_size = (size_t)(dst - dst_buffer); // bytes produced
	return 0;
}
