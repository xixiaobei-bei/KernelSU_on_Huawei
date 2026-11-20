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

#ifndef _OSL_GENERIC_H
#define _OSL_GENERIC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "osl_types.h"
#define OSL_ROUND_UP(a, align)     (((long) (a) + (align - 1)) & ~(align - 1))
#define OSL_ROUND_DOWN(a, align)   ((long)(a) & ~(align - 1))

#ifdef __KERNEL__
#include <linux/kernel.h>

#elif defined(__OS_RTOSCK__) ||defined(__OS_RTOSCK_SMP__)|| defined(__CMSIS_RTOS) || defined(__FASTBOOT__) 

#ifndef typeof
#define typeof __typeof__
#endif



/*lint +rw( typeof ) */
/*lint +rw( __typeof__ ) */
/*lint -dmin(a,b)=(((a)<(b))? (a):(b))*/


#define max(a, b) /*lint +rw( typeof ) -e(160) -e(1058)*/ ({	\
	typeof(a) _max1 = (a);			\
	typeof(b) _max2 = (b);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
    
#define min(a, b) /*lint +rw( typeof ) -e(160) */  ({	\
        typeof(a) _min1 = (a);          \
        typeof(b) _min2 = (b);          \
        (void) (&_min1 == &_min2);      \
        _min1 < _min2 ? _min1 : _min2; }) 


#define __constant_swab32(a) ((u32)(				\
	(((u32)(a) & (u32)0x000000ffUL) << 24) |		\
	(((u32)(a) & (u32)0x0000ff00UL) <<  8) |		\
	(((u32)(a) & (u32)0x00ff0000UL) >>  8) |		\
	(((u32)(a) & (u32)0xff000000UL) >> 24)))
    
#define __constant_swab16(a) ((u16)(				\
        (((u16)(a) & (u16)0x00ffU) << 8) |          \
        (((u16)(a) & (u16)0xff00U) >> 8)))

#define be32_to_cpu __constant_swab32
#define cpu_to_be32 __constant_swab32


static inline u32 be32_to_cpup(const __be32 *p)
{
	return __constant_swab32(*p);
}

static inline u16 be16_to_cpup(const __be16 *p)
{
	return __constant_swab16(*p);
}

#endif /* __KERNEL__ */


#ifdef __cplusplus
}
#endif


#endif /* _OSL_GENERIC_H */
