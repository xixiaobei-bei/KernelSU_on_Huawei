/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020.All rights reserved.
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

#ifndef NETFILTER_BUF_H
#define NETFILTER_BUF_H

#include "vos.h"
#include "PsTypeDef.h"
#include "ttf_oam_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define MAX_NF_EXT_RING_BUF_NUM  1024

typedef struct {
    VOS_UINT8* data;
} NFEXT_DataRingBuf;

typedef struct
{
    VOS_UINT32     tail;       /* offset from start of buffer where to write next */
    VOS_UINT32     head;       /* offset from start of buffer where to read next */
    VOS_SPINLOCK   lockTxTask; /* 自旋锁，用于环形buff操作的互斥保护 */
    NFEXT_DataRingBuf  buf[MAX_NF_EXT_RING_BUF_NUM];
} NFEXT_Buff;

extern NFEXT_Buff* NFEXT_BufferCreate(VOS_VOID);
extern VOS_BOOL NFEXT_RingBufferIsEmpty(NFEXT_Buff* ringBuff);
extern VOS_UINT32 NFEXT_RingBufferDequeue(NFEXT_Buff* ringBuff, NFEXT_DataRingBuf* buffer);
extern VOS_UINT32 NFEXT_RingBufferEnqueue(NFEXT_Buff* ringBuff, const NFEXT_DataRingBuf* buffer);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
