/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020.All rights reserved..
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

#include "netfilter_buffer.h"
#include "ttf_comm.h"
#include "ttf_util.h"

#define THIS_FILE_ID PS_FILE_ID_ACPU_NF_BUF_C

NFEXT_Buff* NFEXT_BufferCreate(VOS_VOID)
{
    NFEXT_Buff* nfExtBuff = (NFEXT_Buff *)VOS_CacheMemAllocDebug(sizeof(NFEXT_Buff), TTF_MEM_ALLOC_CACHE_MEM);
    if (nfExtBuff == VOS_NULL_PTR) {
        return VOS_NULL_PTR;
    }
    nfExtBuff->tail = 0;
    nfExtBuff->head = 0;
    VOS_SpinLockInit(&(nfExtBuff->lockTxTask));
    return nfExtBuff;
}

STATIC VOS_BOOL NFEXT_RingBufferIsFull(const NFEXT_Buff* ringBuff)
{
    return (ringBuff->head == PS_MOD_GET(ringBuff->tail + 1, MAX_NF_EXT_RING_BUF_NUM));
}

VOS_BOOL NFEXT_RingBufferIsEmpty(NFEXT_Buff* ringBuff)
{
    VOS_ULONG flag = 0UL;
    VOS_BOOL  result;

    VOS_SpinLockIntLock(&(ringBuff->lockTxTask), flag);
    result = (ringBuff->tail == ringBuff->head);
    VOS_SpinUnlockIntUnlock(&(ringBuff->lockTxTask), flag);
    return result;
}

/* 将数据从ring buffer里取出 */
VOS_UINT32 NFEXT_RingBufferDequeue(NFEXT_Buff* ringBuff, NFEXT_DataRingBuf* buffer)
{
    VOS_ULONG flag = 0UL;
    VOS_UINT32 result;

    VOS_SpinLockIntLock(&(ringBuff->lockTxTask), flag);
    if (ringBuff->tail != ringBuff->head) {
        *buffer = ringBuff->buf[ringBuff->head];
        ringBuff->head = PS_MOD_ADD(ringBuff->head, 1, MAX_NF_EXT_RING_BUF_NUM);
        result = VOS_OK;
    } else {
        result = VOS_ERR;
    }
    VOS_SpinUnlockIntUnlock(&(ringBuff->lockTxTask), flag);
    return result;
}


/* 将数据放到ring buffer里 */
VOS_UINT32 NFEXT_RingBufferEnqueue(NFEXT_Buff* ringBuff, const NFEXT_DataRingBuf* buffer)
{
    VOS_ULONG flag = 0UL;
    VOS_UINT32 result;

    VOS_SpinLockIntLock(&(ringBuff->lockTxTask), flag);
    if (!NFEXT_RingBufferIsFull(ringBuff)) {
        ringBuff->buf[ringBuff->tail] = *buffer;
        ringBuff->tail = PS_MOD_ADD(ringBuff->tail, 1, MAX_NF_EXT_RING_BUF_NUM);
        result = VOS_OK;
    } else {
        result = VOS_ERR;
    }
    VOS_SpinUnlockIntUnlock(&(ringBuff->lockTxTask), flag);
    return result;
}

