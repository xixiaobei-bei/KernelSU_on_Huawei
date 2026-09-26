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

#ifndef __PPP_PUBLIC_H__
#define __PPP_PUBLIC_H__

#include "vos.h"
#include "at_ppp_interface.h"
#include "product_config.h"
#include "ttf_util.h"
#include "v_io.h"
#include "ttf_comm.h"
#include "imm_interface.h"
#include "mdrv_nvim.h"
#include "ppps_pppa_interface.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#ifndef __STATIC_
#if defined(DMT) || defined(__UT_CENTER__)
#define __STATIC_
#else
#define __STATIC_ static
#endif
#endif

#define PPPA_PPP_ID 1

#define PPPA_IP   0x0021
#define PPPA_IPV6 0x0057
#define PPP_IP_VER_IPV4  4
#define PPP_IP_VER_IPV6  6
/* 以太网IPV4协议类型为0x0800, 网络序即为0x0008 */
#define PPPA_ETH_IPV4_PROTO 0x0008
#define PPPA_ETH_IPV6_PROTO 0xDD86

#ifndef VOS_NTOHL
#if VOS_BYTE_ORDER == VOS_BIG_ENDIAN
#define VOS_NTOHL(x) (x)
#define VOS_HTONL(x) (x)
#define VOS_NTOHS(x) (x)
#define VOS_HTONS(x) (x)
#else
#define VOS_NTOHL(x) \
    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))

#define VOS_HTONL(x) \
    ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24))

#define VOS_NTOHS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

#define VOS_HTONS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

#endif

#endif

#ifndef INADDR_ANY
#define INADDR_ANY (VOS_UINT32)0x00000000L
#endif

#ifndef INADDR_BROADCAST
#define INADDR_BROADCAST (VOS_UINT32)0xffffffffL /* must be masked */
#endif

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffffUL /* -1 return */
#endif

/* 系统最多需要的PPP ID的数目，实际需要的空间是该值加一，因为头是不被分配的 */
#define PPP_MAX_ID_NUM_ALLOC (PPP_MAX_ID_NUM + 1)

/* 接收到的报文向系统申请空间，需要预留的添加头部的 */
#define PPP_RECIEVE_RESERVE_FOR_HEAD 4

/* 接收到的报文向系统申请空间，需要预留的添加尾部的 */
#define PPP_RECIEVE_RESERVE_FOR_TAIL 2

#define PPPoE_RESERVE_HEADER_LEN 20 /* 预留的字节数，供PPPoE填写以太帧头部和PPPoE头部 */

#define PPP_FEATURE_PPP 0 /* 使用PPP */

#define PPP_FEATURE PPP_FEATURE_PPP /* 当前选择PPP */

#define PPP_MAX_DATA_CNT_IN_QUEUE 2000

#define PPP_RAB_TO_PPPID(pppId, rabId) (At_PsRab2PppId((rabId), pppId) == TAF_SUCCESS)

#define PPP_PPPID_TO_RAB(pppId, rabId) (At_PppId2PsRab((pppId), rabId) == TAF_SUCCESS)

#define PPP_MNTN_LOG(printLevel, str) TTF_LOG(PS_PID_APP_PPP, DIAG_MODE_COMM, printLevel, str)
#define PPP_MNTN_LOG1(printLevel, str, para1) TTF_LOG1(PS_PID_APP_PPP, DIAG_MODE_COMM, printLevel, str, para1)
#define PPP_MNTN_LOG2(printLevel, str, para1, para2) \
    TTF_LOG2(PS_PID_APP_PPP, DIAG_MODE_COMM, printLevel, str, para1, para2)

#define PPP_MNTN_LOG3(printLevel, str, para1, para2, para3) \
    TTF_LOG3(PS_PID_APP_PPP, DIAG_MODE_COMM, printLevel, str, para1, para2, para3)

#define PPP_MNTN_LOG4(printLevel, str, para1, para2, para3, para4) \
    TTF_LOG4(PS_PID_APP_PPP, DIAG_MODE_COMM, printLevel, str, para1, para2, para3, para4)

/* 零拷贝操作相关宏 */
typedef IMM_Zc       PPP_Zc;
typedef IMM_ZcHeader PPP_ZcQueue;

#define PPP_ZC_MAX_DATA_LEN (IMM_MAX_ETH_FRAME_LEN)    /* A核内可申请内存块最大长度 */

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#define PPP_ZC_UL_RESERVE_LEN 0
#else
#define PPP_ZC_UL_RESERVE_LEN (IMM_MAC_HEADER_RES_LEN) /* MAC头长度，上行方向需要保留 */
#endif
#define PPP_ZC_DL_RESERVE_LEN 0                        /* 下行方向不需要保留 */

#define PPP_ZC_MEM_ALLOC(len) (IMM_ZcStaticAlloc(len))
#define PPP_ZC_MEM_FREE(mem) (IMM_ZcFreeAny(mem))
#define PPP_ZC_GET_DATA_PTR(immZc) ((VOS_UINT8*)IMM_ZcGetDataPtr(immZc))
#define PPP_ZC_GET_DATA_LEN(immZc) ((VOS_UINT16)IMM_ZcGetUsedLen(immZc))
#define PPP_ZC_GET_DATA_APP(immZc) ((VOS_UINT16)IMM_ZcGetUserApp(immZc))
#define PPP_ZC_SET_DATA_APP(immZc, app) (IMM_ZcSetUserApp(immZc, app))
#define PPP_ZC_SET_DATA_LEN(immZc, len) (IMM_ZcPut(immZc, len))
#define PPP_ZC_REMOVE_HDR(immZc, len) (IMM_ZcPull(immZc, len))
#define PPP_ZC_RESERVE(mem, reserveLen) (IMM_ZcReserve(mem, reserveLen))
#define PPP_ZC_GET_RESERVE_ROOM(mem) (IMM_ZcGetReserveRoom(mem))
#define PPP_ZC_TRACE_MEM(mem) (IMM_ZcTraceMem(mem))
#define PPP_ZC_SET_PRIORITY(mem, pri) (IMM_ZcSetPriority(mem, pri))

#define PPP_ZC_QUEUE_INIT(zcQueue) (IMM_ZcQueueHeadInit(zcQueue))
#define PPP_ZC_ENQUEUE_HEAD(zcQueue, mem) (IMM_ZcQueueHead(zcQueue, mem))
#define PPP_ZC_ENQUEUE_TAIL(zcQueue, mem) (IMM_ZcQueueTail(zcQueue, mem))
#define PPP_ZC_DEQUEUE_HEAD(zcQueue) (IMM_ZcDequeueHead(zcQueue))
#define PPP_ZC_DEQUEUE_TAIL(zcQueue) (IMM_ZcDequeueTail(zcQueue))
#define PPP_ZC_PEEK_QUEUE_HEAD(zcQueue) (IMM_ZcQueuePeek(zcQueue))
#define PPP_ZC_GET_QUEUE_LEN(zcQueue) (IMM_ZcQueueLen(zcQueue))
#define PPP_GET_IP_VERSION(data) ((data)[0] >> 4)
#define PPP_GET_IP_PROTOCOL(ipVersion) ((ipVersion == PPP_IP_VER_IPV4) ? PPPA_IP : PPPA_IPV6)

/* A核可维可测接口 */
#define PPP_MNTN_TRACE_MSG(msg) mdrv_diag_trace_report(msg)

VOS_VOID PPPA_CheckSafeFuncRslt(VOS_BOOL result, VOS_UINT32 fileNo, VOS_UINT32 lineNo);
#define PPPA_SF_CHK(result) PPPA_CheckSafeFuncRslt(((result) != EOK) ? VOS_TRUE : VOS_FALSE, (THIS_FILE_ID), (__LINE__))

typedef struct {
    /* 上行数据包总个数 = 上行释放包数 + 上行丢包数 */
    VOS_UINT32 uplinkInputCnt;        /* 上行数据包总个数 */
    VOS_UINT32 uplinkInputFreeCnt;    /* 上行释放包数 */
    VOS_UINT32 uplinkInputDropCnt;    /* 上行丢包数 */

    /* 上行输出内存申请个数 = 上行输出发送数据包数 + 上行输出发送协商包数 + 上行输出丢包数 */
    VOS_UINT32 uplinkOutputAllocMemCnt; /* 上行输出内存申请个数 */
    VOS_UINT32 uplinkOutputSndDataCnt;  /* 上行输出发送数据包数 */
    VOS_UINT32 uplinkOutputSendNegoCnt; /* 上行输出发送协商包数 */
    VOS_UINT32 uplinkOutputDropCnt;     /* 上行输出丢包数 */

    /* 下行数据包总个数 =    下行数据包释放包数 + 下行丢包数 */
    VOS_UINT32 downlinkInputCnt;        /* 下行数据包总个数 */
    VOS_UINT32 downlinkInputFreeCnt;    /* 下行数据包释放包数 */
    VOS_UINT32 downlinkInputDropCnt;    /* 下行丢包数 */

    /* 下行输出申请内存数 = 下行输出发送数据包数 + 下行输出发送前释放数据包数 + 下行输出丢包数 */
    VOS_UINT32 downlinkOutputAllocMemCnt; /* 下行输出申请内存数 */
    VOS_UINT32 downlinkOutputSndCnt;      /* 下行输出发送数据包数 */
    VOS_UINT32 downlinkOutputFreeCntBeforeSend; /* 下行输出发送前释放数据包数 */
    VOS_UINT32 downlinkOutputDropCnt; /* 下行输出丢包数 */

    VOS_UINT32 memAllocSuccCnt;     /* 内存申请次数 */
    VOS_UINT32 memAllocFailCnt; /* 内存申请失败次数 */
    VOS_UINT32 memTotalFreeCnt;         /* 其他内存释放次数 */

    VOS_UINT32 queueFullDropCnt; /* 队列满丢包数 */

    VOS_UINT32 qMaxCnt;    /* 队列中出现过的最大结点个数 */
    VOS_UINT32 sndMsgCnt;  /* DataNotify消息发送数 */
    VOS_UINT32 procMsgCnt; /* DataNotify消息处理数 */
} PPP_DataQStat;

/* 定时器消息 */
#define PPPA_DATA_DELAY_PROC_TIMER 0x0001
#define PPPA_DATA_Q_STAT_REPORT_TIMER 0x0002

extern PPP_Zc* PPP_MemAlloc(VOS_UINT16 len, VOS_UINT16 reserveLen);
extern PPP_Zc* PPP_MemClone(const VOS_UINT8 *src, VOS_UINT16 len, VOS_UINT16 reserveLen);
extern VOS_VOID PPP_MemFree(PPP_Zc *data);
extern VOS_UINT32 PPP_MemGet(PPP_Zc *memSrc, VOS_UINT16 offset, VOS_UINT8 *dest, VOS_UINT16 len);
extern VOS_VOID PPPA_SetDataStatAddr(PPP_DataQStat *dataStat);
extern VOS_VOID PPPA_SendPppsCommMsg(PPPS_PPPA_MsgTypeUint16 msgId);

#if (VOS_OSA_CPU == OSA_CPU_ACPU)
extern VOS_VOID* PPPA_FilterPppaPppsMsg(MsgBlock *msg);
#else
extern MsgBlock* PPPA_FilterPppaPppsMsg(const MsgBlock* msg);
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
