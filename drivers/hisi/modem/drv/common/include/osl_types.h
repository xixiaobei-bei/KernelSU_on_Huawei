/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __OSL_TYPES_H
#define __OSL_TYPES_H

#ifdef __cplusplus /* __cplusplus */
extern "C"
{
#else
#if defined(__CMSIS_RTOS) || defined(__OS_VXWORKS__) ||defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__) /* linux中有bool枚举定义 */
#ifndef bool
typedef enum {
	false	= 0,
	true	= 1
}bool;
#endif /* bool */
#endif

#endif /* __cplusplus */


/*lint -save -e761*/

#ifndef BSP_U32
typedef unsigned int        BSP_U32;
#endif

#ifdef __KERNEL__
#include <linux/types.h>
#ifndef __ASSEMBLY__
#ifndef UNUSED
#define UNUSED(a) (void)(a)
#endif
#endif

#ifndef __inline__
#define __inline__ inline
#endif

#elif defined(_WRS_KERNEL) || defined(__OS_VXWORKS__) ||  defined(__OS_RTOSCK__)||defined(__OS_RTOSCK_SMP__)||defined(__OS_RTOSCK_TSP__) || defined(__FASTBOOT__) || defined(__NR_LL1C_)

#ifndef __ASSEMBLY__
#ifdef __OS_VXWORKS__
#ifndef inline
#define inline __inline__
#endif
#else

#endif

#ifndef __u8_defined
#define __u8_defined
typedef signed int s32;
typedef unsigned int u32;
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long long s64;
typedef unsigned long long u64;
#endif
typedef unsigned short __be16;
typedef unsigned int __be32;

#if defined(__OS_RTOSCK__)  ||defined(__OS_RTOSCK_SMP__) ||defined(__OS_RTOSCK_TSP__)
#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint64_t
typedef unsigned long long uint64_t;
#endif

#ifndef size_t
typedef unsigned int size_t;/*lint !e410 !e452*/
#endif
#endif

#ifndef UNUSED
#define UNUSED(a) (void)(a)
#endif

#else /* __ASSEMBLY__ */


#endif /* __ASSEMBLY__ */

#ifndef __ASSEMBLY__

#define __asmeq(x, y)  ".ifnc " x "," y " ; .err ; .endif\n\t"

#undef NULL
#define NULL ((void *)0)

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)
#endif

#endif /* __ASSEMBLY__ */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif
/*lint -esym(666,container_of)*/
#define container_of(pointer, str_type, member)  ((str_type *)((char *)(pointer) - offsetof(str_type,member)))


#ifndef printk
#define printk  printf
#endif

#elif defined(__CMSIS_RTOS) || defined(__FASTBOOT__) || defined(__NR_LL1C_)

#ifndef __ASSEMBLY__

#undef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((u32) &((TYPE *)0)->MEMBER)
#endif

#define container_of(pointer, str_type, member)  ((str_type *)((char *)(pointer) - offsetof(str_type,member)))
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a)/(sizeof((a)[0])))
#endif

#ifndef __u8_defined
#define __u8_defined
typedef signed int s32;
typedef unsigned int u32;
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long long s64;
typedef unsigned long long u64;
#endif

#undef NULL
#define NULL ((void *)0)
#ifndef UNUSED
#define UNUSED(a) ((void)(a))
#endif
#endif

#else

#ifndef __ASSEMBLY__

#ifndef __u8_defined
#define __u8_defined
typedef signed int s32;
typedef unsigned int u32;
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long long s64;
typedef unsigned long long u64;
#endif

#endif

#endif /* __KERNEL__ */

/*lint -restore +e761*/

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif

