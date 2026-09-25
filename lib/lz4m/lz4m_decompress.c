/*
 * Copyright (C) 2018-2019 HUAWEI, Inc.
 * http://www.huawei.com/
 */

#include <stddef.h>
#include <linux/types.h>
#include <asm/simd.h>
#include "lz4m_decompress.h"
#define LZ4M_FAST_MARGIN		(128)

#if defined(CONFIG_ARM64) && defined(CONFIG_KERNEL_MODE_NEON)
#include "lz4mneon.h"
#endif


extern int _lz4m_decode_asm(uint8_t **dst_ptr, uint8_t *dst_begin,
		uint8_t *dst_end, const uint8_t **src_ptr,
		const uint8_t *src_end);

size_t lz4m_decompress(const uint8_t *src_buffer, size_t src_size,
		uint8_t *dst_buffer, size_t *dst_size)
{
	const uint8_t *src = src_buffer;
	uint8_t *dst = (uint8_t *)dst_buffer;
	int ret;

	/* Go fast if we can, keeping away from the end of buffers */
	if (*dst_size > LZ4M_FAST_MARGIN && src_size > LZ4M_FAST_MARGIN &&
	    may_use_simd() && system_supports_fpsimd()) {
		kernel_neon_begin();
		ret = _lz4m_decode_asm(&dst, (uint8_t *)dst_buffer,
			(uint8_t *)dst_buffer + *dst_size - LZ4M_FAST_MARGIN,
			&src,
			src_buffer + src_size - LZ4M_FAST_MARGIN);
		kernel_neon_end();
		if (ret)
			return -1;
	}
	ret = __lz4m_decompress_safe_partial((uint8_t **)&dst, &src,
			dst_buffer, *dst_size, src_buffer, src_size, 1);

	if (ret)
		return -2;

	*dst_size = (size_t)(dst - dst_buffer);
	return 0;
}

