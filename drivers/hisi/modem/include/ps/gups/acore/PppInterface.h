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

#ifndef __PPP_INTERFACE_H__
#define __PPP_INTERFACE_H__

#include "pppa_pppc_at_type.h"
#include "product_config.h"
#include "v_typdef.h"
#include "PsTypeDef.h"
#if (OSA_CPU_ACPU == VOS_OSA_CPU) || (VOS_WIN32 == VOS_OS_VER)
#include "ImmInterface.h"
#endif
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*系统最多需要的PPP ID的数目 */
#define PPP_MAX_ID_NUM                      (1)

#ifndef IPV6_ADDR_LEN
#define   IPV6_ADDR_LEN                     (16)      /*IPV6地址长度*/
#endif

/******************************************************************************
  3 枚举定义
******************************************************************************/
enum PPP_AT_CTRL_OPER_TYPE_ENUM
{
    PPP_AT_CTRL_REL_PPP_REQ = 0,              /* 释放IP类型PPP链路操作 */
    PPP_AT_CTRL_REL_PPP_RAW_REQ = 1,          /* 释放PPP类型PPP链路操作 */
    PPP_AT_CTRL_HDLC_DISABLE = 2,             /* HDLC硬化模块去使能操作 */

    PPP_AT_CTRL_BUTT
};
typedef VOS_UINT32 PPP_AtCtrlOperTypeUint32;

/******************************************************************************
  4 全局变量声明
******************************************************************************/


/******************************************************************************
  5 消息头定义
******************************************************************************/


/******************************************************************************
  6 消息定义
******************************************************************************/


/******************************************************************************
  7 STRUCT定义
******************************************************************************/
#pragma pack(4)

typedef VOS_UINT16 PPP_ID;


/* PPP向AT发送的消息定义如下 */
#define AT_PPP_RELEASE_IND_MSG          0x00    /* 释放PDP连接的请求 */
#define AT_PPP_MODEM_MSC_IND_MSG        0x01    /* 向AT发送管脚信号指示(底软透传管脚信号，
                                                         由于无法模拟AT给自己发消息,故模拟PPP给AT发送该消息.) */
#define AT_PPP_PROTOCOL_REL_IND_MSG     0x02    /* PPP链路完成释放的指示 */

typedef struct
{
    MSG_HEADER_STRU                     MsgHeader;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucReserve[3];
}AT_PPP_RELEASE_IND_MSG_STRU;


typedef struct
{
    MSG_HEADER_STRU                     MsgHeader;
    VOS_UINT8                           ucIndex;
    VOS_UINT8                           ucDlci;
    VOS_UINT8                           ucReserve1;
    VOS_UINT8                           ucReserve2;
    VOS_UINT8                           aucMscInd[4];  /* 管脚信号数据 */
}AT_PPP_MODEM_MSC_IND_MSG_STRU;

/* 以下结构为C核TAF使用，PPPA不使用，待TAF整改后删除 */
typedef struct
{
    MSG_HEADER_STRU                     MsgHeader;
    VOS_UINT16                          usPppId;
    VOS_UINT8                           ucReserve[2];
}AT_PPP_PROTOCOL_REL_IND_MSG_STRU;

typedef struct
{
    VOS_UINT16  usPapReqLen;                    /*request长度: 24.008要求在[3,253]字节*/
    VOS_UINT8   aucReserve[2];                  /* 对齐保留 */
    VOS_UINT8  *pPapReq;                        /*request*/
} PPP_AUTH_PAP_CONTENT_STRU;

typedef struct
{
    VOS_UINT16  usChapChallengeLen;             /*challenge长度: 24.008要求在[3,253]字节*/
    VOS_UINT16  usChapResponseLen;              /*response长度: 24.008要求在[3,253]字节*/
    VOS_UINT8  *pChapChallenge;                 /*challenge*/
    VOS_UINT8  *pChapResponse;                  /*response*/
} PPP_AUTH_CHAP_CONTENT_STRU;

typedef struct
{
    PPP_AUTH_TYPE_ENUM_UINT8  ucAuthType;
    VOS_UINT8                 aucReserve[3];

    union
    {
        PPP_AUTH_PAP_CONTENT_STRU  PapContent;
        PPP_AUTH_CHAP_CONTENT_STRU ChapContent;
    } AuthContent;
} PPP_REQ_AUTH_CONFIG_INFO_STRU;

typedef struct
{
    VOS_UINT16  usIpcpLen;                      /*Ipcp帧长度*/
    VOS_UINT8   aucReserve[2];                  /* 对齐保留 */
    VOS_UINT8  *pIpcp;                          /*Ipcp帧*/
} PPP_REQ_IPCP_CONFIG_INFO_STRU;

typedef struct
{
    PPP_REQ_AUTH_CONFIG_INFO_STRU stAuth;
    PPP_REQ_IPCP_CONFIG_INFO_STRU stIPCP;
} PPP_REQ_CONFIG_INFO_STRU;


#pragma pack()

/******************************************************************************
  8 UNION定义
******************************************************************************/


/******************************************************************************
  9 OTHERS定义
******************************************************************************/


/******************************************************************************
  10 函数声明
******************************************************************************/
/* PPP提供给AT的接口 */
extern VOS_UINT32 Ppp_CreatePppReq ( PPP_ID *pusPppId);
extern VOS_UINT32 Ppp_CreateRawDataPppReq ( PPP_ID *pusPppId);
extern VOS_UINT32 Ppp_RcvConfigInfoInd(PPP_ID usPppId, PPPA_PDP_ACTIVE_RESULT_STRU *pPppIndConfigInfo);
extern VOS_VOID   PPP_UpdateWinsConfig(VOS_UINT8 ucWins);
extern VOS_VOID   PPP_SetRawDataByPassMode(VOS_UINT32 ulRawDataByPassMode);

#if (OSA_CPU_ACPU == VOS_OSA_CPU) || (VOS_WIN32 == VOS_OS_VER)
/* IP方式下提供的上行数据接收接口 */
extern VOS_UINT32 PPP_PullPacketEvent(VOS_UINT16 usPppId, IMM_ZC_STRU *pstImmZc);

/* PPP方式下提供的上行数据接收接口 */
extern VOS_UINT32 PPP_PullRawDataEvent(VOS_UINT16 usPppId, IMM_ZC_STRU *pstImmZc);
#endif

extern VOS_UINT32 PPP_RcvAtCtrlOperEvent(VOS_UINT16 usPppId, PPP_AtCtrlOperTypeUint32 ulCtrlOperType);

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* PppInterface.h */

