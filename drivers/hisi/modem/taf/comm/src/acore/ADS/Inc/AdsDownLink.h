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

#ifndef __ADSDOWNLINK_H__
#define __ADSDOWNLINK_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "vos.h"
#include "AdsIntraMsg.h"
#include "AdsCtx.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/

#define ADS_DL_ERROR_PACKET_NUM_THRESHOLD           (VOS_NULL_DWORD/100)

#define ADS_DL_IPF_RESULT_PKT_LEN_ERR_MASK          (0x8000)    /* bit 15 */
#define ADS_DL_IPF_RESULT_BDCD_LEN_ERR_MASK         (0x4000)    /* bit 14 */
#define ADS_DL_IPF_RESULT_PKT_PARSE_ERR_MASK        (0x2000)    /* bit 13 */
#define ADS_DL_IPF_RESULT_BD_PKT_LEN_ERR_MASK       (0x1000)    /* bit 12 */
#define ADS_DL_IPF_RESULT_HDR_LEN_ERR_MASK          (0x0800)    /* bit 11 */
#define ADS_DL_IPF_RESULT_IP_VER_ERR_MASK           (0x0400)    /* bit 10 */

#define ADS_DL_IPF_RESULT_PKT_ERR_MASK              (ADS_DL_IPF_RESULT_BDCD_LEN_ERR_MASK \
                                                        | ADS_DL_IPF_RESULT_PKT_PARSE_ERR_MASK \
                                                        | ADS_DL_IPF_RESULT_BD_PKT_LEN_ERR_MASK \
                                                        | ADS_DL_IPF_RESULT_HDR_LEN_ERR_MASK \
                                                        | ADS_DL_IPF_RESULT_IP_VER_ERR_MASK)

#define ADS_DL_IPF_RESULT_PKT_ERR_FEEDBACK_MASK     (ADS_DL_IPF_RESULT_HDR_LEN_ERR_MASK\
                                                        | ADS_DL_IPF_RESULT_IP_VER_ERR_MASK)

/*lint -emacro({717}, ADS_DL_SAVE_RD_DESC_TO_IMM)*/
#define ADS_DL_SAVE_RD_DESC_TO_IMM(pstImmZc, pstRdDesc)\
            do\
            {\
                ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0] = (pstRdDesc)->u16Result;\
                ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0] = ((ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0]) << 16) | ((pstRdDesc)->u16UsrField1);\
            } while(0)

/* 从IPF Result获取IP包类型 */
#define ADS_DL_GET_IPTYPE_FROM_IPF_RESULT(usRslt)   ((ADS_PKT_TYPE_ENUM_UINT8)(((ADS_DL_IPF_RESULT_STRU *)&(usRslt))->usIpType))

/* 从IPF Result获取Beared ID */
#define ADS_DL_GET_BEAREDID_FROM_IPF_RESULT(usRslt) ((ADS_PKT_TYPE_ENUM_UINT8)(((ADS_DL_IPF_RESULT_STRU *)&(usRslt))->usBearedId))

/* RD的user field 1，高1byte为Modem id，低1byte为Rab Id */
#define ADS_DL_GET_IPF_RESULT_FORM_IMM(pstImmZc)    ((VOS_UINT16)(((ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0]) & 0xFFFF0000) >> 16))
#define ADS_DL_GET_MODEMID_FROM_IMM(pstImmZc)       ((VOS_UINT32)(((ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0]) & 0x0000FF00) >> 8))
#define ADS_DL_GET_RABID_FROM_IMM(pstImmZc)         ((VOS_UINT32)((ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0]) & 0x000000FF))

#define ADS_DL_IMM_PRIV_CB(pstImmZc)                ((ADS_DL_IMM_PRIV_CB_STRU *)(&(ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0])))

#define ADS_DL_GET_IPF_RSLT_USR_FIELD1_FROM_IMM(pstImmZc)   (ADS_IMM_MEM_CB(pstImmZc)->aulPriv[0])

#ifndef CONFIG_NEW_PLATFORM
#define ADS_DL_GET_IPF_RD_DESC_OUT_PTR(pstRdDesc)   ((pstRdDesc)->u32OutPtr)
#else
#define ADS_DL_GET_IPF_RD_DESC_OUT_PTR(pstRdDesc)   ((pstRdDesc)->OutPtr)
#endif

#define ADS_DL_GET_RNIC_NAPI_POLL_QUE_LEN(ulExParam)        ((ulExParam < RNIC_RMNET_ID_BUTT) ? g_ulRnicNapiPollQueLen[ulExParam] : 0)

/* 下行内存cache invalidate (map) */
/*lint -emacro({717}, ADS_IPF_DL_MEM_MAP)*/
#define ADS_IPF_DL_MEM_MAP(pstImmZc, ulLen)\
            do\
            {\
                if (VOS_FALSE == ADS_IPF_IsSpeMem(pstImmZc))\
                {\
                    ADS_IPF_MemMapRequset(pstImmZc, ulLen, 0);\
                }\
            } while(0)

/* 下行内存cache invalidate (unmap) */
/*lint -emacro({717}, ADS_IPF_DL_MEM_UNMAP)*/
#define ADS_IPF_DL_MEM_UNMAP(pstImmZc, ulLen)\
            do\
            {\
                if (VOS_FALSE == ADS_IPF_IsSpeMem(pstImmZc))\
                {\
                    ADS_IPF_MemUnmapRequset(pstImmZc, ulLen, 0);\
                }\
            } while(0)


/* 封装OSA申请消息接口 */
#define ADS_DL_ALLOC_MSG_WITH_HDR(ulMsgLen)\
            PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_ADS_DL, (ulMsgLen))

/* 封装OSA消息头 */
#define ADS_DL_CFG_MSG_HDR(pstMsg, ulRecvPid, ulMsgId)\
            ((MSG_HEADER_STRU *)(pstMsg))->ulSenderCpuId   = VOS_LOCAL_CPUID;\
            ((MSG_HEADER_STRU *)(pstMsg))->ulSenderPid     = ACPU_PID_ADS_DL;\
            ((MSG_HEADER_STRU *)(pstMsg))->ulReceiverCpuId = VOS_LOCAL_CPUID;\
            ((MSG_HEADER_STRU *)(pstMsg))->ulReceiverPid   = (ulRecvPid);\
            ((MSG_HEADER_STRU *)(pstMsg))->ulMsgName       = (ulMsgId)

/* 封装OSA消息头(ADS内部消息) */
#define ADS_DL_CFG_INTRA_MSG_HDR(pstMsg, ulMsgId)\
            ADS_DL_CFG_MSG_HDR(pstMsg, ACPU_PID_ADS_DL, ulMsgId)

/* 封装OSA消息头(CDS内部消息) */
#define ADS_DL_CFG_CDS_MSG_HDR(pstMsg, ulMsgId)\
            ADS_DL_CFG_MSG_HDR(pstMsg, UEPS_PID_CDS, ulMsgId)

/* 获取OSA消息内容 */
#define ADS_DL_GET_MSG_ENTITY(pstMsg)\
            ((VOS_VOID *)&(((MSG_HEADER_STRU *)(pstMsg))->ulMsgName))

/* 获取OSA消息长度 */
#define ADS_DL_GET_MSG_LENGTH(pstMsg)\
            (((MSG_HEADER_STRU *)(pstMsg))->ulLength)

/* 封装OSA发送消息接口 */
/*lint -emacro({717}, ADS_DL_SEND_MSG)*/
#define ADS_DL_SEND_MSG(pstMsg)\
            do\
            {\
                if (VOS_OK != PS_SEND_MSG(ACPU_PID_ADS_DL, pstMsg))\
                {\
                    ADS_ERROR_LOG(ACPU_PID_ADS_DL, "ADS_DL_SEND_MSG: Send message fail!\n");\
                }\
            } while(0)


/*****************************************************************************
  3 枚举定义
*****************************************************************************/


/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern VOS_UINT32                       g_ulRnicNapiPollQueLen[RNIC_RMNET_ID_BUTT];

/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/


typedef struct
{
    VOS_UINT16                          usBearedId:6;
    VOS_UINT16                          usIpSegInfo:2;
    VOS_UINT16                          usUpperLayer:1;
    VOS_UINT16                          usIpType:1;
    VOS_UINT16                          usVerErr:1;
    VOS_UINT16                          usHdrLenErr:1;
    VOS_UINT16                          usBdPktNotEq:1;
    VOS_UINT16                          usPktParseErr:1;
    VOS_UINT16                          usBdCdNotEq:1;
    VOS_UINT16                          usPktLenErr:1;

} ADS_DL_IPF_RESULT_STRU;

/*****************************************************************************
 结构名称  : ADS_IMM_MEM_CB_STRU
 结构说明  : ADS下行IMM私有结构
*****************************************************************************/
typedef struct
{
    VOS_UINT8                           ucRabId;
    VOS_UINT8                           ucModemId;
    union
    {
        VOS_UINT16                      usIpfResult;
        ADS_DL_IPF_RESULT_STRU          stIpfResult;
    } u;

} ADS_DL_IMM_PRIV_CB_STRU;


/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/


/*****************************************************************************
  10 函数声明
*****************************************************************************/

VOS_VOID ADS_DL_ProcMsg(MsgBlock* pMsg);
VOS_VOID ADS_DL_ProcTxData(IMM_ZC_STRU *pstImmZc);
VOS_UINT32 ADS_DL_RcvAtMsg(MsgBlock* pMsg);
VOS_UINT32 ADS_DL_RcvTafPdpStatusInd(MsgBlock *pMsg);
VOS_UINT32 ADS_DL_RcvCdsMsg(MsgBlock *pMsg);
VOS_INT32 ADS_DL_IpfIntCB(VOS_VOID);
VOS_VOID ADS_DL_ProcIpfResult(VOS_VOID);
VOS_INT32 ADS_DL_IpfAdqEmptyCB(IPF_ADQ_EMPTY_E eAdqEmpty);
VOS_VOID ADS_DL_RcvTiAdqEmptyExpired(VOS_UINT32 ulParam, VOS_UINT32 ulTimerName);
VOS_UINT32 ADS_DL_ConfigAdq(IPF_AD_TYPE_E enAdType, VOS_UINT ulIpfAdNum);
VOS_VOID ADS_DL_AllocMemForAdq(VOS_VOID);
VOS_INT ADS_DL_CCpuResetCallback(DRV_RESET_CB_MOMENT_E enParam, VOS_INT lUserData);
VOS_UINT32 ADS_DL_RcvCcpuResetStartInd(VOS_VOID);
VOS_UINT32 ADS_DL_RcvCcpuResetEndInd(MsgBlock *pMsg);
VOS_VOID ADS_DL_FreeIpfUsedAd0(VOS_VOID);
VOS_VOID ADS_DL_FreeIpfUsedAd1(VOS_VOID);
VOS_VOID ADS_DL_SendNdClientDataInd(IMM_ZC_STRU *pstImmZc);
VOS_VOID ADS_DL_SendDhcpClientDataInd(IMM_ZC_STRU *pstImmZc);
VOS_VOID ADS_DL_RcvTiCcoreResetDelayExpired(VOS_UINT32 ulParam, VOS_UINT32 ulTimerName);
VOS_VOID ADS_DL_RcvPacketErrorInd(MsgBlock* pMsg);
#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif




#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of AdsDlProcData.h */
