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

/*
 * 版权所有（c）华为技术有限公司 2008-2019
 * 功能描述: SIM卡的头文件
 * 日   期: 2008年10月18日
 */
#ifndef __SI_PIH_H__
#define __SI_PIH_H__

#include "vos.h"
#include "usimm_ps_interface.h"
#include "si_typedef.h"
#include "si_app_pih.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#define SI_PIH_PIN_CODE_LEN (USIMM_PINNUMBER_LEN)

#define SI_PIH_SEC_ICC_VSIM_VER 10000

enum SI_PIH_Req {
    SI_PIH_NULL_REQ = 0,
    SI_PIH_FDN_ENABLE_REQ = 1,
    SI_PIH_FDN_DISALBE_REQ = 2,
    SI_PIH_GACCESS_REQ = 3,
    SI_PIH_BDN_QUERY_REQ = 4,
    SI_PIH_FDN_QUERY_REQ = 5,
    SI_PIH_ISDB_ACCESS_REQ = 7,
    SI_PIH_HVSST_QUERY_REQ = 8,
    SI_PIH_HVSST_SET_REQ = 9,
    SI_PIH_HVSDH_SET_REQ = 10,
    SI_PIH_HVSDH_QRY_REQ = 11,
    SI_PIH_HVSCONT_QRY_REQ = 12,
    SI_PIH_FILE_WRITE_REQ = 13,
    SI_PIH_CCHO_SET_REQ = 14,
    SI_PIH_CCHC_SET_REQ = 15,
    SI_PIH_CGLA_SET_REQ = 16,
    SI_PIH_CARD_ATR_QRY_REQ = 17,
    SI_PIH_UICCAUTH_REQ = 18,
    SI_PIH_URSM_REQ = 19,
    SI_PIH_CARDTYPE_QUERY_REQ = 20,
    SI_PIH_CRSM_SET_REQ = 21,
    SI_PIH_CRLA_SET_REQ = 22,
    SI_PIH_SESSION_QRY_REQ = 23,
    SI_PIH_SCICFG_SET_REQ = 24,
    SI_PIH_SCICFG_QUERY_REQ = 25,
    SI_PIH_HVTEE_SET_REQ = 26,
    SI_PIH_HVCHECKCARD_REQ = 27,
    SI_PIH_CIMI_QRY_REQ = 28,
    SI_PIH_CCIMI_QRY_REQ = 29,

    SI_PIH_CCHP_SET_REQ = 30,
    SI_PIH_CARDVOLTAGE_QUERY_REQ = 31,
    SI_PIH_PRIVATECGLA_SET_REQ = 32,

    SI_PIH_CARDTYPEEX_QUERY_REQ = 33,

#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
    SI_PIH_SILENT_PIN_SET_REQ = 34,
    SI_PIH_SILENT_PININFO_SET_REQ = 35,
#endif
    SI_PIH_ESIMSWITCH_SET_REQ = 36,
    SI_PIH_ESIMSWITCH_QRY_REQ = 37,
    SI_PIH_BWT_SET_REQ = 38,
    SI_PIH_PRIVATECCHO_SET_REQ = 39,
    SI_PIH_PRIVATECCHP_SET_REQ = 40,
    SI_PIH_RCV_REFRESH_PID_IND = 41,
    SI_PIH_PASSTHROUGH_SET_REQ = 42,
    SI_PIH_PASSTHROUGH_QUERY_REQ = 43,
#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
    SI_PIH_SINGLEMODEMDUALSLOT_SET_REQ = 44,
    SI_PIH_SINGLEMODEMDUALSLOT_QUERY_REQ= 45,
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
    SI_PIH_SET_NOCARD_REQ = 46,
#endif
#endif

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))
    SI_PIH_CARD_APP_AID_SET_REQ = 47,
#endif
    SI_PIH_CARD_HISTORY_ATR_QRY_REQ = 48,

    SI_PIH_REQ_BUTT
};
typedef VOS_UINT32 SI_PIH_ReqUint32;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PIH_EventUint32 eventType;
} SI_PIH_MsgHeader;

typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_UiccAuth authData;
} SI_PIH_UiccauthReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader;
    VOS_UINT8 pin2[SI_PIH_PIN_CODE_LEN]; /* PIN2码，如上层调用未带入PIN2码则为全0 */
} SI_PIH_FdnEnableReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader;
    VOS_UINT8 pin2[SI_PIH_PIN_CODE_LEN]; /* PIN2码，如上层调用未带入PIN2码则为全0 */
} SI_PIH_FdnDisableReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader;
    VOS_UINT32 dataLen;
    VOS_UINT8 data[0]; /*lint !e43 */
} SI_PIH_GaccessReq;

/*
* 结构说明: 打开逻辑通道请求数据结构
* 修改历史:
* 1.日    期: 2013年05月14日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader;           /* PIH消息头 */
    VOS_UINT32 aidLen;                    /* AID的长度 */
    VOS_UINT8 adfName[USIMM_AID_LEN_MAX]; /* 考虑到中移动不对AID长度检测的需求将长度增大1倍(和usimm代码不匹配) */
} SI_PIH_CchoSetReqMsg;

/*
* 结构说明: 打开逻辑通道请求数据结构,带APDU的参数P2
* 修改历史:
* 1.日    期: 2016年09月26日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader;           /* PIH消息头 */
    VOS_UINT32 aidLen;                    /* AID的长度 */
    VOS_UINT8 adfName[USIMM_AID_LEN_MAX]; /* 考虑到中移动不对AID长度检测的需求将长度增大1倍(和usimm代码不匹配) */
    VOS_UINT8 apdup2;                     /* APDU命令的P2参数 */
    VOS_UINT8 rsv[3];
} SI_PIH_CchpSetReqMsg;

/*
* 结构说明: 打开逻辑通道请求数据结构
* 修改历史:
* 1.日    期: 2013年05月14日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    VOS_UINT32 sessionId;       /* 逻辑通道号 */
} SI_PIH_CchcSetReqMsg;

typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_HvsstSet hvSSTData;
} SI_PIH_HvsstReq;

/*
 * 结构说明: passthrough模式请求数据结构
 * 修改历史:
 * 1. 日    期: 2020年04月10日
 *    修改内容: 创建
 */
typedef struct {
    SI_PIH_MsgHeader msgHeader;             /* PIH消息头 */
    SI_PIH_PassThroughStateUint32 state;    /* 上电或下电 */
} SI_PIH_PassThroughReqMsg;

/*
 * 结构说明: SingleModemDualSlot请求数据结构
 * 修改历史:
 * 1. 日    期: 2020年04月10日
 *    修改内容: 创建
 */
typedef struct {
    SI_PIH_MsgHeader msgHeader;
    SI_PIH_CardSlotUint32 slotId;
} SI_PIH_SingleModemDualSlotReqMsg;

/*
* 结构说明: AT^ESIMSWITCH设置请求消息结构
* 修改历史:
* 1.日    期: 2019年1月18日
*   修改内容: eSIM适配项目新增
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_EsimSwitchSet esimSwitchData;
} SI_PIH_EsimswitchReq;

/*
* 结构说明: 透传逻辑通道APDU的请求数据结构
* 修改历史:
* 1.日    期: 2013年05月14日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader;          /* PIH消息头    */
    VOS_UINT32 sessionId;                /* 逻辑通道号 */
    VOS_UINT32 dataLen;                  /* 命令长度  */
    VOS_UINT8 data[SI_APDU_MAX_LEN + 1]; /* 命令内容, 带LE字段多一个字节 */
    VOS_UINT8 rsv[3];
} SI_PIH_CglaReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_CardSlotUint32 card0Slot;
    SI_PIH_CardSlotUint32 card1Slot;
    SI_PIH_CardSlotUint32 card2Slot;
} SI_PIH_ScicfgSetReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    VOS_UINT16 protectTime;
    VOS_UINT16 usRsv;
} SI_PIH_BwtSetReq;

/*
* 结构说明: CRSM请求数据结构
* 修改历史:
* 1.日    期: 2015年04月08日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_Crsm msgContent;
} SI_PIH_CrsmSetReqMsg;

/*
* 结构说明: CRLA请求数据结构
* 修改历史:
* 1.日    期: 2015年04月08日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_Crla msgContent;
} SI_PIH_CrlaSetReqMsg;

/*
* 结构说明: AT^SILENTPIN处理结构体
* 修改历史:
* 1.日    期: 2017年09月06日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader;
    SI_PIH_CryptoPin cryptoPin;
} SI_PIH_SilentPinReq;

/*
* 结构说明: AT^SILENTPIN处理结构体
* 修改历史:
* 1.日    期: 2017年09月06日
*   修改内容: 新建
*/
typedef struct {
    SI_PIH_MsgHeader msgHeader;
    VOS_UINT32 dataLen;
    VOS_UINT8 data[USIMM_PINNUMBER_LEN];
} SI_PIH_SilentPininfoReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader;
    VOS_UINT32 dataLen;
    VOS_UINT8 data[4];
} SI_PIH_HvsdhSetReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader;
} SI_PIH_HvsQryReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_AccessFile cmdData;
} SI_PIH_AccessfileReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    SI_PIH_HvteeSet hvtee;
} SI_PIH_HvteeSetReq;

typedef struct {
    SI_PIH_MsgHeader msgHeader;
} SI_PIH_HvcheckcardReq;

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))
typedef struct {
    SI_PIH_MsgHeader msgHeader; /* PIH消息头 */
    VOS_UINT32 enable; /* 0:默认值，不使能；1:使能 */
    VOS_UINT32 appType; /* 需要配置的应用的类型。取值范围是0-2。0:USIM 1:ISIM 2:CSIM */
    VOS_UINT32 aidLen; /* 配置AID长度 */
    VOS_UINT8 aid[USIMM_AID_LEN_MAX]; /* 配置AID内容 */
} SI_PIH_CardAppAidSetReq;
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
typedef struct {
    SI_PIH_MsgHeader msgHeader;
    VOS_UINT32 noCardMode;
} SI_PIH_SetNoCardReq;
#endif
#endif

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
