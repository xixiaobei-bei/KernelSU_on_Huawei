/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/20
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

#ifndef __NDISULPROC_H__
#define __NDISULPROC_H__

/*
 * 1 Include Headfile
 */
#include "vos.h"

/*
 * 1.1 Cplusplus Announce
 */
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*
 * #pragma pack(*)    设置字节对齐方式
 */
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(4)
#else
#pragma pack(push, 4)
#endif

#define NDIS_SPE_CACHE_HDR_SIZE (IMM_MAC_HEADER_RES_LEN + sizeof(ETH_Ipfixhdr))
#define NDIS_ARP_FRAME_REV_OFFSET (((VOS_UINT64)(VOS_UINT_PTR)(&(((ETH_ArpFrame *)0)->rev[0]))) & 0xFFFFFFFF)

#if (defined(CONFIG_BALONG_SPE))
#define NDIS_SPE_MEM_CB(immZc) ((NDIS_SPE_MemCb *)&((immZc)->dma))

/*lint -emacro({717}, NDIS_SPE_MEM_MAP)*/
#define NDIS_SPE_MEM_MAP(immZc, memLen) do { \
    if (VOS_TRUE == NDIS_IsSpeMem(immZc)) {  \
        NDIS_SpeMemMapRequset(immZc, memLen); \
    }                                           \
} while (0)

/*lint -emacro({717}, NDIS_SPE_MEM_UNMAP)*/
#define NDIS_SPE_MEM_UNMAP(immZc, memLen) do { \
    if (VOS_TRUE == NDIS_IsSpeMem(immZc)) {    \
        NDIS_SpeMemUnmapRequset(immZc, memLen); \
    }                                             \
} while (0)
#else
#define NDIS_SPE_MEM_MAP(immZc, memLen)
#define NDIS_SPE_MEM_UNMAP(immZc, memLen)
#endif

/* skb内存，得到数据块使用的字节数。 返回数据块使用的字节数 */
#define NDIS_MemGetUsedLen(ndisMem) (IMM_ZcGetUsedLen(ndisMem))
/* 得到数据块首地址。 返回数据块首地址 */
#define NDIS_MemGetDataPtr(ndisMem) (IMM_ZcGetDataPtr(ndisMem))
#define NDIS_MemFree(ndisMem) (IMM_ZcFree(ndisMem))

typedef struct sk_buff NDIS_Mem;

#if (VOS_OS_VER != VOS_WIN32) /* 单字节对齐 */
#pragma pack(1)
#else
#pragma pack(push, 1)
#endif


#if (defined(CONFIG_BALONG_SPE))
/*
 * 结构名: NDIS_SPE_MemCb
 * 协议表格:
 * ASN.1描述:
 * 结构说明: SPE MEM CB结构
 */
typedef struct {
    dma_addr_t dmaAddr;
} NDIS_SPE_MemCb;
#endif

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

VOS_VOID Ndis_UlNcmFrmProc(VOS_UINT8 exRabId, NDIS_Mem *pktNode);

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of ndis_ul_proc*/

