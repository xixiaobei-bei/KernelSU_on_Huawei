/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#ifndef __BSP_NVIM_MEM_H__
#define __BSP_NVIM_MEM_H__

#include <bsp_shared_ddr.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern unsigned long g_nv_global_addr;
extern unsigned int g_nv_global_size;
extern unsigned long g_nv_unsec_addr;

/* global ddr distribution */
#ifdef CONFIG_SHARED_MEMORY
#define SHM_MEM_NV_ADDR              ((uintptr_t)(g_nv_global_addr))
#define SHM_MEM_NV_SIZE              (g_nv_global_size)
#else
#define SHM_MEM_NV_ADDR              ((uintptr_t)((uintptr_t)SHM_BASE_ADDR + (unsigned long)SHM_OFFSET_NV))
#define SHM_MEM_NV_SIZE              (SHM_SIZE_NV)
#endif

#define NV_GLOBAL_START_ADDR         ((uintptr_t)SHM_MEM_NV_ADDR)
#define NV_GLOBAL_END_ADDR           ((uintptr_t)((uintptr_t)SHM_MEM_NV_ADDR + (unsigned long)SHM_MEM_NV_SIZE))

/* reg global info */
#ifdef CONFIG_SHARED_MEMORY
#define NV_GLOBAL_INFO_ADDR         ((uintptr_t)g_nv_unsec_addr)
#else
#ifdef SHM_SIZE_NV_UNSAFE
#define NV_GLOBAL_INFO_ADDR         ((uintptr_t)((uintptr_t)SHM_BASE_ADDR + (unsigned long)SHM_OFFSET_NV_UNSAFE))
#else
#define NV_GLOBAL_INFO_ADDR         (NV_GLOBAL_START_ADDR)
#endif
#endif

#define NV_GLOBAL_INFO_SIZE         ((unsigned long)0x400)               /* 1 K */

#define NV_GLOBAL_CTRL_INFO_ADDR    ((uintptr_t)(NV_GLOBAL_START_ADDR + NV_GLOBAL_INFO_SIZE))


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  /*__BSP_NVIM_MEM_H__*/
