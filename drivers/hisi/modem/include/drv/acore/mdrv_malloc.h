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
#ifndef __MDRV_MALLOC_H__
#define __MDRV_MALLOC_H__
#ifdef __cplusplus
extern "C"
{
#endif

/**
 *  @brief   malloc 模块在acore上的对外头文件
 *  @file    mdrv_malloc.h
 *  @author  chuguangyang
 *  @version v2.0
 *  @date    2019.11.28
 *  @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.05.30|创建文件</li></ul>
 *  @since
 */

enum malloc_reserved_req_id
{
    MALLOC_SEC_SHM_REQ_MIN = 0,
    MALLOC_SEC_SHM_REQ_MAX,

    MALLOC_NONSEC_SHM_REQ_MIN = 0x100,
    MALLOC_NONSEC_SHM_REQ_MAX,

    MALLOC_TCM_REQ_MIN = 0x200,
    MALLOC_TCM_REQ_MAX,

    MALLOC_LLRAM_REQ_MIN = 0x300,
    MALLOC_LLRAM_REQ_MAX,

    MALLOC_AXIMEM_REQ_MIN = 0x400,
    MALLOC_AXIMEM_REQ_MAX,

    MALLOC_MEMBUF_REQ_MIN = 0x500,
    MALLOC_MEMBUF_REQ_MAX
};

/**
 * @brief 虚拟地址转换物理地址
 * @par 描述:
 * 虚拟地址转换物理地址
 * @param[in] 虚拟地址 虚拟地址是通过mdrv_cached_mem_malloc或mdrv_uncached_mem_malloc申请得到
 * @retval NULL，转换失败，转换地址不在malloc范围
 * @retval 转换后的物理地址
 */
void* mdrv_malloc_virt_to_phy(const void *);

#include "mdrv_malloc_common.h"

#ifdef __cplusplus
}
#endif
#endif
