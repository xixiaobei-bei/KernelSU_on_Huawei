/*
 * hi_types.h
 *
 * This is omx self define type.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HI_TYPE_H__
#define __HI_TYPE_H__

#if defined(__KERNEL__)
#include <linux/version.h>
#endif
#include "securec.h"

typedef unsigned char           hi_u8;
typedef unsigned int            hi_u32;
typedef unsigned short          hi_u16;
typedef unsigned long           hi_ulong;

typedef signed char             hi_s8;
typedef signed int              hi_s32;
typedef short                   hi_s16;

#ifndef _M_IX86
typedef unsigned long long      hi_u64;
typedef long long               hi_s64;
#else
typedef __int64                 hi_u64;
typedef __int64                 hi_s64;
#endif

typedef char                    hi_char;
typedef char                   *hi_pchar;
typedef unsigned long           hi_size_t;

typedef unsigned long           hi_virt_addr_t;
typedef hi_u32                  hi_handle;

#define UADDR         hi_u32

typedef enum {
	HI_FALSE = 0,
	HI_TRUE = 1,
} hi_bool;

#define HI_NULL           0L

#define HI_SUCCESS        0
#define HI_FAILURE        (-1)
#define unused_param(p) (void)(p)

#endif
