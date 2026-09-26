/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2006-2019. All rights reserved.
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

#ifndef __TTF_UTIL_H__
#define __TTF_UTIL_H__

#include "vos.h"
#include "mdrv_nvim.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* TTF SYS ERR ID */
#define TTF_MEM_RB_TASK_FAIL_SEM_ERR 0x30000002
#define TTF_CICOM_SYSTEM_ERROR_ID 0x30000003
#define TTF_MEM_ALLOC_FAIL_SYSTEM_ERROR 0x30000004
#define TTF_MEM_COPY_ALLOC_FAIL_ERROR 0x30000005
#define TTF_MEM_SET_FAIL_ERROR 0x30000006    /* 由于内存异常，MEM_SET安全函数导致的复位 */
#define TTF_MEM_CPY_FAIL_ERROR 0x30000007    /* 由于内存异常，MEM_CPY安全函数导致的复位 */
#define TTF_MEM_MOVE_FAIL_ERROR 0x30000008   /* 由于内存异常，MEM_MOVE安全函数导致的复位 */
#define TTF_GET_CRYPTO_TRNG_ERROR 0x30000009 /* 调用底软接口获取熵源和随机因子失败导致的复位 */

#define TTF_NOC_SYSTEM_ERROR 0x31000000 /* 提供给底软触发HDLC/CICOM Noc复位使用 */

/* TTF Alloc Cache/Uncache Mem ID */
#define TTF_CPU_VIEW_CACHE_MEM 0xE3500001    /* CPU VIEW申请CACHE内存使用 */
#define TTF_MEM_ALLOC_CACHE_MEM 0xE3500002   /* TTF MEME申请CACHE内存使用 */
#define TTF_CIPHER_CACHE_MEM 0xE3500003      /* CIPHER申请CACHE内存使用 */
#define TTF_HDLC_UNCACHE_MEM 0xE3500004      /* HDLC申请UNCACHE内存使用 */
#define TTF_MEM_ALLOC_UNCACHE_MEM 0xE3500005 /* TTFMEM申请UNCACHE内存使用 */

/* CTTF SYS ERR ID */
#define TTF_NODE_ALLOC_SYSTEM_ERROR_ID 0x3F000001
#define PS_QNODE_GET_SYSTEM_ERROR_ID 0x3F000002
#define HDLC_DEF_SYSTEM_ERROR_ID 0x3F000003
#define HDLC_FRM_SYSTEM_ERROR_ID 0x3F000004
#define TTF_DDR_RAM_BASE_ADDR_ERROR_ID 0x3F000005
#define TTF_HRPD_EDMA_ERROR_ID 0x3F000006
#define TTF_MAC_LOW_POWER_ERROR 0x3F000007 /* 低功耗下访问掉电区导致复位 */
#define CTTF_TIMER_CHECK_ERROR_ID 0x3F000008
#define CTTF_MEM_OP_ERROR_ID 0x3F000009 /* 内存操作失败导致复位 */

#define WTTF_GET_PHY_NULL_PTR_ERROR_ID 0x3F00000A /* WTT初始化获得PHY共享内存为空指针导致复位 */
#define WTTF_MSG_SND_ERROR_ID 0x3F00000B          /* WTTF消息发送失败导致复位 */

/*
 * 强制ARM执行前面所有指令，解决ARM指令乱序问题
 * 使用OM接口自适配L1,L2 Cache打开，需要flush buffer的问题
 */
#define TTF_FORCE_ARM_INSTUCTION() VOS_FlushCpuWriteBuf()

/* NV接口转定义 */
#define GUCTTF_NV_GetLength(id, len) mdrv_nv_get_length(id, len)
#define GUCTTF_NV_Read(modemid, id, item, len) mdrv_nv_readex(modemid, id, item, len)
#define GUCTTF_NV_Write(modemid, id, item, len) mdrv_nv_writeex(modemid, id, item, len)
#define GUCTTF_NV_ReadPart(modemid, id, off, item, len) mdrv_nv_read_partex(modemid, id, off, item, len)
#define GUCTTF_NV_WritePart(modemid, id, off, item, len) mdrv_nv_write_partex(modemid, id, off, item, len)
#define GUCTTF_NVM_Read(modemid, id, item, len) mdrv_nv_readex(modemid, id, item, len)
#define GUCTTF_NVM_Write(modemid, id, item, len) mdrv_nv_writeex(modemid, id, item, len)
#define GUCTTF_NV_ReadEx(modemid, id, item, len) mdrv_nv_readex(modemid, id, item, len)
#define GUCTTF_NV_WriteEx(modemid, id, item, len) mdrv_nv_writeex(modemid, id, item, len)
#define GUCTTF_NV_ReadPartEx(modemid, id, off, item, len) mdrv_nv_read_partex(modemid, id, off, item, len)
#define GUCTTF_NV_WritePartEx(modemid, id, off, item, len) mdrv_nv_write_partex(modemid, id, off, item, len)
#define GUCTTF_NVM_Flush() mdrv_nv_flush()

#pragma pack(push, 4)

/* TTF静态队列管理定义 */
typedef struct tagTtfQLink {
    struct tagTtfQLink* next;
} TtfQLink;

typedef struct {
    TtfQLink *head; /* 队列头指针 */
    TtfQLink *tail; /* 队列尾指针 */
} TtfQLinkHdr;

typedef struct {
    TtfQLinkHdr hdr;
    VOS_UINT32  cnt;
} TtfQ;

/* TTF内存管理定义 */
/* TTF内存单元定义 */
typedef struct tagTtfMBuf {
    TtfQLink           link; /* 队列节点 */
    struct tagTtfMBuf* next; /* 该数据的下一段 */
    VOS_UINT16         len;  /* 数据的总长度，但不一定全部被使用 */
    VOS_UINT16         used; /* 已经使用的数据长度 */
    VOS_UINT8         *data; /* 存放数据的指针，物理上指向结构体的后续内存 */
} TtfMBuf;

typedef struct {
    VOS_UINT8 *baseAddr;
    VOS_UINT32 size;
} TtfRingBuf;

/*
 * TTF 安全函数返回值校验，校验失败会触发复位。
 * 支持函数： memset_s memcpy_s memmove_s
 */
VOS_VOID TTF_CheckSafeFuncRslt(VOS_BOOL result, VOS_UINT32 fileNo, VOS_UINT32 lineNo);
#define TTF_SF_CHK(result) TTF_CheckSafeFuncRslt(((result) != EOK) ? VOS_TRUE : VOS_FALSE, THIS_FILE_ID, __LINE__)

/*
 * TTF 安全函数返回值校验，校验失败会打印错误，不能复位。根据业务需求，无需复位的场景，可以使用本函数记录日志。
 * 支持函数： memset_s memcpy_s memmove_s
 */
#define TTF_SF_LOG(result) TTF_RecordSafeFuncFailLog(((result) != EOK) ? VOS_TRUE : VOS_FALSE, THIS_FILE_ID, __LINE__)

/*
 * TTF Acore安全函数返回值校验，校验失败会打印错误，不能复位。
 * 支持函数： memset_s memcpy_s memmove_s
 */
MODULE_EXPORTED VOS_VOID TTF_RecordSafeFuncFailLog(VOS_BOOL result, VOS_UINT32 fileNo, VOS_UINT32 lineNo);
#define TTF_ACORE_SF_CHK(result) \
    TTF_RecordSafeFuncFailLog(((result) != EOK) ? VOS_TRUE : VOS_FALSE, THIS_FILE_ID, __LINE__)

#pragma pack(pop)

#ifndef PCLINT_MEM
/* 申请指定长度的内存单元，数据附在TTF_BLK_ST结构之后 */
TtfMBuf* TTF_MbufNew(VOS_UINT32 pid, VOS_UINT16 len);

/* 释放指定TTF_MBUF_ST结构的内存 */
VOS_VOID TTF_MbufFree(VOS_UINT32 pid, TtfMBuf *mem);

VOS_UINT32 TTF_QIn(VOS_UINT32 pid, TtfQ *queue, TtfQLink *link);
VOS_VOID*  TTF_QOut(VOS_UINT32 pid, TtfQ *queue);
#endif

VOS_UINT16 TTF_MbufGetLen(VOS_UINT32 pid, TtfMBuf *mem);
VOS_VOID   TTF_QLink(VOS_UINT32 pid, VOS_VOID *item, TtfQLink *link);
VOS_VOID   TTF_QInit(VOS_UINT32 pid, TtfQ *queue);

VOS_UINT32 TTF_SetByBit(VOS_UINT32 pid, VOS_UINT32 orgValue, VOS_UINT8 startBit, VOS_UINT8 endBit, VOS_UINT32 setValue);
VOS_UINT32 TTF_GetByBit(VOS_UINT32 pid, VOS_UINT32 orgValue, VOS_UINT8 startBit, VOS_UINT8 endBit);

VOS_UINT32 TTF_Sum(const VOS_UINT32 *data, VOS_UINT32 num);

VOS_VOID TTF_RingBufRead(VOS_UINT32 pid, TtfRingBuf *buf, VOS_UINT32 offset, VOS_UINT8 *dstData, VOS_UINT16 dataLen);
VOS_VOID TTF_RingBufWrite(VOS_UINT32 pid, TtfRingBuf *buf, VOS_UINT16 offset, const VOS_UINT8 *srcData,
    VOS_UINT16 dataLen);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
