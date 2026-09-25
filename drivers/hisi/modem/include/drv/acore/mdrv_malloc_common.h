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
#ifndef __MDRV_COMMON_MALLOC_H__
#define __MDRV_COMMON_MALLOC_H__
#ifdef __cplusplus
extern "C"
{
#endif

/**
 * flags, for mdrv_mem_alloc
 */
#define __MEM_ALIGN_008 0x01u /**< 8字节对齐*/
#define __MEM_ALIGN_016 0x02u/**< 16字节对齐*/
#define __MEM_ALIGN_032 0x04u/**< 32字节对齐*/
#define __MEM_ALIGN_064 0x08u/**< 64字节对齐*/
#define __MEM_ALIGN_128 0x10u/**< 128字节对齐*/
#define __MEM_ALIGN_256 0x20u/**< 256字节对齐*/
#define __MEM_ALIGN_512 0x40u/**< 512字节对齐*/
#define __MEM_ALIGN_1K  0x80u/**< 1K字节对齐*/
#define __MEM_ALIGN_MSK 0xffu

#define __MEM_DMA       0x100u/*分配uncached内存*/

enum MDRV_FLAG
{
	MDRV_GFP_KERNEL
};


/*****************************************************************************
 *  函 数 名  : mdrv_reserved_mem_malloc
 *  功能描述  : 分配预定区内存，该区域内存只分配不释放
 *  输入参数  :
 *              size: 分配内存大小
 *              request_id: 需求ID
 *
 *  输出参数  :
 *  返 回 值  :  NULL   分配失败。
 *               其他   已分配内存地址。
 *
 ******************************************************************************/
void* mdrv_reserved_mem_malloc(unsigned int size, enum MALLOC_RESERVED_REQ_ID request_id);
 
 /*****************************************************************************
  *  函 数 名  : mdrv_cached_mem_malloc
  *  功能描述  : 分配cached内存
  *  输入参数  :
  * 			 size: 分配内存大小
  * 			 request_id: 需求ID
  *
  *  输出参数  :
  *  返 回 值  :  NULL	 分配失败。
  * 			  其他	 已分配内存地址。
  *
  ******************************************************************************/
void* mdrv_cached_mem_malloc(unsigned int size); 
 
/*****************************************************************************
*  函 数 名  : mdrv_cached_mem_free
*  功能描述  : 释放指定cached内存。
*  输入参数  :
* 			 addr: 内存首地址
*
*  输出参数  : 无
*  返 回 值  : 无
*
******************************************************************************/
void mdrv_cached_mem_free(const void* addr); 

/*****************************************************************************
 *  函 数 名  : mdrv_uncached_mem_malloc
 *  功能描述  : 分配uncached内存
 *  输入参数  :
 *              size: 分配内存大小
 *              request_id: 需求ID
 *
 *  输出参数  :
 *  返 回 值  :  NULL   分配失败。
 *               其他   已分配内存地址。
 *
 ******************************************************************************/
void* mdrv_uncached_mem_malloc(unsigned int size); 

/*****************************************************************************
 *  函 数 名  : mdrv_uncached_mem_free
 *  功能描述  : 释放指定uncached内存。
 *  输入参数  :
 *              addr: 内存首地址
 *
 *  输出参数  : 无
 *  返 回 值  : 无
 *
 ******************************************************************************/
int mdrv_uncached_mem_free(const void* addr); 

/*****************************************************************************
 *  函 数 名  : mdrv_mem_alloc
 *  功能描述  : 分配指定属性内存
 *  输入参数  : mid:  模块ID(acore无效)
 *              size: 分配内存大小
 *              flag: 属性配置
 *
 *  输出参数  :
 *  返 回 值  :  NULL   分配失败。
 *               其他   已分配内存地址。
 *
 ******************************************************************************/
void* mdrv_mem_alloc(unsigned int mid, unsigned int size, unsigned int flag);

/*****************************************************************************
 *  函 数 名  : mdrv_mem_free
 *  功能描述  : 释放指定内存。
 *  输入参数  : mid:  模块ID(acore无效)
 *              addr: 内存首地址
 *
 *  输出参数  : 无
 *  返 回 值  : MDRV_OK
 *
 ******************************************************************************/
unsigned int mdrv_mem_free(unsigned int mid, const void* addr);

#ifdef __cplusplus
}
#endif
#endif
