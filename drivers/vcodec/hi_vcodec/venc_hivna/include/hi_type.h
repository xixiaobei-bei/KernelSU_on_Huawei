/*
 * hi_type.h
 *
 * This is The common data type defination.
 *
 * Copyright (c) 2008-2020 Huawei Technologies CO., Ltd.
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

#include "securec.h"

#ifdef HIVCODECV500
#define UADDR uint64_t
#else
#define UADDR uint32_t
#endif

#ifndef NULL
#define NULL             0L
#endif
#define HI_NULL          0L

#define HI_SUCCESS       0
#define HI_FAILURE       (-1)

/* magic number 0-255 */
#define IOC_TYPE_VENC     'V'
#define VERSION_STRING    "1234"
#define HI_INVALID_HANDLE 0xffffffff
#endif
