/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Functions for perform wrapping operations on 32bit values. The implementation
 * strives to be correct from the C language's point of view thus the code may
 * look quite clumsy.
 */

#ifndef D2D_WRAPPERS_H
#define D2D_WRAPPERS_H

#include <linux/bug.h>
#include <linux/types.h>

/* Use this typedef to mark ordinary u32 values as wrapped values */
typedef u32 wrap_t;

#define U32_HALF (U32_MAX / 2)

static inline s32 __d2d_wrap_sub(wrap_t x, wrap_t y)
{
	/* this partially-defined function must be called with x >= y */
	WARN_ON(x < y);

	return (x - y <= U32_HALF) ? (x - y) : -(s32)(U32_MAX - x + y) - 1;
}

static inline s32 d2d_wrap_sub(wrap_t x, wrap_t y)
{
	/* this pair is incomparable by the RFC, callers should work around */
	if (x - y == U32_HALF + 1)
		return S32_MIN;

	return (x >= y) ? __d2d_wrap_sub(x, y) : -__d2d_wrap_sub(y, x);
}

static inline bool d2d_wrap_eq(wrap_t x, wrap_t y)
{
	return x == y;
}

static inline bool d2d_wrap_ge(wrap_t x, wrap_t y)
{
	return d2d_wrap_sub(x, y) >= 0;
}

static inline bool d2d_wrap_gt(wrap_t x, wrap_t y)
{
	return d2d_wrap_sub(x, y) > 0;
}

static inline bool d2d_wrap_le(wrap_t x, wrap_t y)
{
	return d2d_wrap_sub(x, y) <= 0;
}

static inline bool d2d_wrap_lt(wrap_t x, wrap_t y)
{
	return d2d_wrap_sub(x, y) < 0;
}

#endif /* D2D_WRAPPERS_H */
