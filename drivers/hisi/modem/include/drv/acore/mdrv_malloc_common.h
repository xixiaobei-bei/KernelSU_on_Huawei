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
#ifndef __MDRV_COMMON_MALLOC_H__
#define __MDRV_COMMON_MALLOC_H__
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   malloc 模块在acore上的对外头文件
 * @file    mdrv_malloc_common.h
 * @author  chuguangyang
 * @version v2.0
 * @date    2019.11.28
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.05.30|创建文件</li></ul>
 * @since
 */
/**
 * flags, for mdrv_mem_alloc
 */
#define __MEM_ALIGN_008 0x01u /**< 8字节对齐 */
#define __MEM_ALIGN_016 0x02u /**< 16字节对齐 */
#define __MEM_ALIGN_032 0x04u /**< 32字节对齐 */
#define __MEM_ALIGN_064 0x08u /**< 64字节对齐 */
#define __MEM_ALIGN_128 0x10u /**< 128字节对齐 */
#define __MEM_ALIGN_256 0x20u /**< 256字节对齐 */
#define __MEM_ALIGN_512 0x40u /**< 512字节对齐 */
#define __MEM_ALIGN_1K 0x80u  /**< 1K字节对齐 */
#define __MEM_ALIGN_MSK 0xffu

#define __MEM_DMA 0x100u /**< 分配uncached内存 */

enum mdrv_flag {
    MDRV_GFP_KERNEL
};
/**
 * 5010 begin use
 */
enum mem_align {
    MEM_ADDR_ALIGN_004 = 2,  /**< 4字节对齐 */
    MEM_ADDR_ALIGN_008 = 3,  /**< 8字节对齐 */
    MEM_ADDR_ALIGN_016 = 4,  /**< 16字节对齐 */
    MEM_ADDR_ALIGN_032 = 5,  /**< 32字节对齐 */
    MEM_ADDR_ALIGN_064 = 6,  /**< 64字节对齐 */
    MEM_ADDR_ALIGN_128 = 7,  /**< 128字节对齐 */
    MEM_ADDR_ALIGN_256 = 8,  /**< 256字节对齐 */
    MEM_ADDR_ALIGN_512 = 9,  /**< 512字节对齐 */
    MEM_ADDR_ALIGN_1K = 10,  /**< 1K字节对齐 */
    MEM_ADDR_ALIGN_2K = 11,  /**< 2K字节对齐 */
    MEM_ADDR_ALIGN_4K = 12,  /**< 4K字节对齐 */
    MEM_ADDR_ALIGN_8K = 13,  /**< 8K字节对齐 */
    MEM_ADDR_ALIGN_16K = 14, /**< 16K字节对齐 */
    MEM_ADDR_BUTT            /**< 字节对齐非法 */
};

typedef enum {
    MEM_UNCACHE = 0x1000u,
    MEM_CACHE = 0x0u
} mdrv_mem_attribute_e;

typedef enum {
    MEM_SLUB = 0x000,
    MEM_BLOCK = 0x100u
} mdrv_mem_algorithm_e;


/**
 * @brief 分配cached内存
 * @par 描述:
 * 分配cached内存 对应free接口mdrv_cached_mem_free
 * @param[in] size:分配内存大小
 * @retval NULL   分配失败。
 * @retval 其他   已分配内存地址。
 */
void *mdrv_cached_mem_malloc(unsigned int size);

/**
 * @brief 释放指定cached内存。
 * @par 描述:
 * 释放指定cached内存。
 * @param[in] addr: 内存首地址
 * @retval 无。
 */
void mdrv_cached_mem_free(const void *addr);

/**
 * @brief 分配uncached内存
 * @par 描述:
 * 分配uncached内存 对应free接口mdrv_uncached_mem_free
 * @param[in] size:分配内存大小
 * @retval NULL   分配失败。
 * @retval 其他   已分配内存地址。
 */
void *mdrv_uncached_mem_malloc(unsigned int size);

/**
 * @brief 释放指定uncached内存。
 * @par 描述:
 * 释放指定uncached内存。
 * @param[in] addr: 内存首地址
 * @retval 无。
 */
int mdrv_uncached_mem_free(const void *addr);

/**
 * @brief 分配指定属性内存
 * @par 描述:
 * 分配指定属性内存 对应free接口mdrv_mem_free或mdrv_free
 * @param[in] mid:  模块ID(acore无效)
 * @param[in] size: 分配内存大小
 * @param[in] flag: (acore 无效)
 * @retval NULL   分配失败。
 * @retval 其他   已分配内存地址。
 */
void *mdrv_mem_alloc(unsigned int mid, unsigned int size, unsigned int flag);

/**
 * @brief 释放指定内存。
 * @par 描述:
 * 释放指定内存。
 * @param[in] mid:  模块ID(acore无效)
 * @param[in] addr: 内存首地址
 * @retval 0 释放成功。
 */
unsigned int mdrv_mem_free(unsigned int mid, const void *addr);

/**
 * @brief 释放指定内存。
 * @par 描述:
 * 释放指定内存。
 * @param[in] mid:  模块ID(acore无效)
 * @param[in] addr: 内存首地址
 * @retval 0 释放成功。
 */
unsigned int mdrv_free(unsigned int mid, const void *addr);

#ifdef __cplusplus
}
#endif
#endif
