/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef PS_LIB_H
#define PS_LIB_H

/* 先包含下述文件,供PC测试使用 */
/*lint -e553*/
#if (VOS_OS_VER == VOS_WIN32)
#include <stdlib.h>
#include <search.h>
#endif
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

#define PS_FATAL_ERROR() (void)0

#define PS_STATS

/* 获取[0, ulUpperLimit]区间范围内的随机数 */
#define PS_RAND(upperLimit) VOS_Rand(upperLimit)

#define PS_MIN(x, y) (((x) < (y)) ? (x) : (y))

#define PS_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define PS_OFFSET_OF(s, m) ((VOS_SIZE_T)(&(((s*)0)->m)))

#define PS_MOD_GET(x, y) ((x) % (y))

#define PS_MOD_ADD(x, y, z) (((x) + (y)) % (z))

#define PS_MOD_SUB(x, y, z) ((((x) + (z)) - (y)) % (z))

/* 默认DEBUG */
#ifndef PDCP_RELEASE
#define PDCP_DEBUG 1
#endif
#ifndef RLC_RELEASE
#define RLC_DEBUG 1
#endif
#ifndef MAC_RELEASE
#define MAC_DEBUG 1
#endif
#ifndef L2_RELEASE
#define L2_DEBUG 1
#endif
#ifndef PS_RELEASE
#define PS_DEBUG 1
#endif

#if (0 != PDCP_DEBUG)
#define PDCP_ASSERT_ON
#endif

#if (0 != RLC_DEBUG)
#define RLC_ASSERT_ON
#endif

#if (0 != MAC_DEBUG)
#define MAC_ASSERT_ON
#endif

#if (0 != L2_DEBUG)
#define L2_ASSERT_ON
#endif

#if (0 != PS_DEBUG)
#define PS_ASSERT_ON
#endif

#if (VOS_WIN32 == VOS_OS_VER)
#define LPS_ASSERT vos_printf("Assert, File: %s, Line:%d\n", __FILE__, __LINE__)
#else
#define LPS_ASSERT \
    (VOS_VOID)LogPrint6("Assert, File: %s, Line: %d\n", (VOS_INT)(VOS_UINT_PTR)__FILE__, __LINE__, 0, 0, 0, 0)
#endif

#ifdef PDCP_ASSERT_ON

#define PDCP_ASSERT(exp) do { \
    if (!(exp)) {    \
        LPS_ASSERT;  \
        return;      \
    }                \
} while (0)
#define PDCP_ASSERT_RTN(exp, ret) do { \
    if (!(exp)) {             \
        LPS_ASSERT;           \
        return ret;           \
    }                         \
} while (0)
#else
#define PDCP_ASSERT(exp) ((VOS_VOID)0)
#define PDCP_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

#ifdef RLC_ASSERT_ON
#define RLC_ASSERT(exp) do { \
    if (!(exp)) {   \
        LPS_ASSERT; \
        return;     \
    }               \
} while (0)
#define RLC_ASSERT_RTN(exp, ret) do { \
    if (!(exp)) {            \
        LPS_ASSERT;          \
        return ret;          \
    }                        \
} while (0)
#else
#define RLC_ASSERT(exp) ((VOS_VOID)0)
#define RLC_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

#ifdef MAC_ASSERT_ON
#define MAC_ASSERT(exp) do { \
    if (!(exp)) {   \
        LPS_ASSERT; \
        return;     \
    }               \
} while (0)
#define MAC_ASSERT_RTN(exp, ret) do { \
    if (!(exp)) {            \
        LPS_ASSERT;          \
        return ret;          \
    }                        \
} while (0)
#else
#define MAC_ASSERT(exp) ((VOS_VOID)0)
#define MAC_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

#ifdef L2_ASSERT_ON
#define L2_ASSERT(exp) do { \
    if (!(exp)) {   \
        LPS_ASSERT; \
        return;     \
    }               \
} while (0)
#define L2_ASSERT_RTN(exp, ret) do { \
    if (!(exp)) {           \
        LPS_ASSERT;         \
        return ret;         \
    }                       \
} while (0)
#else
#define L2_ASSERT(exp) ((VOS_VOID)0)
#define L2_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

#ifdef PS_ASSERT_ON
#define PS_ASSERT(exp) do { \
    if (!(exp)) {   \
        LPS_ASSERT; \
        return;     \
    }               \
} while (0)
#define PS_ASSERT_RTN(exp, ret) do { \
    if (!(exp)) {           \
        LPS_ASSERT;         \
        return ret;         \
    }                       \
} while (0)
#else
#define PS_ASSERT(exp) ((VOS_VOID)0)
#define PS_ASSERT_RTN(exp, ret) ((VOS_VOID)0)
#endif

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
