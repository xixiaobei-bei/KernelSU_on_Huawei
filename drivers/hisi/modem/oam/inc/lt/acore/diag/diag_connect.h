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

#ifndef __DIAG_CONNECT_H__
#define __DIAG_CONNECT_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include  "vos.h"
#include  "VosPidDef.h"
#include  "nv_stru_msp.h"
#include  "msp_diag.h"
#include  "diag_common.h"

#pragma pack(push)
#pragma pack(4)

/* 连接消息定时器时长 5s超时 */
#define     DIAG_CONNECT_TIMER_LEN          (5*1000)

#define     DIAG_CONNECT_TIMER_NAME         (0x00004004)

#define     DIAG_CONNECT_TIMER_PARA         (0x0000EFFE)

#define     DIAG_CHANN_DEFAULT              (0x5C5C5C5C)
#define     DIAG_CHANN_NOT_IN_USE           (0x5A5A5A5A)
#define     DIAG_CHANN_REGISTER             (0x5B5B5B5B)
/* 通道断开时填写固定的SN */
#define     DIAG_CHANN_DIS_SN               (0x5D5D5D5D)

#ifdef DIAG_SYSTEM_5G
#define     DIAG_SRC_CHANNLE_NUM            (64)
#else
#define     DIAG_SRC_CHANNLE_NUM            (32)
#endif

enum{
    DIAG_CONN_STATE_DISCONNECTED    = 0,
    DIAG_CONN_STATE_CONNECTED       = 1,
};

enum DIAG_CONN_ID_TYPE_E{
    DIAG_CONN_ID_ACPU_DEFALUT = 0,
    DIAG_CONN_ID_ACPU_PS     = 1,
    DIAG_CONN_ID_ACPU_BSP    = 2,
    DIAG_CONN_ID_ACPU_APP    = 3,
    DIAG_CONN_ID_ACPU_TEE    = 4,
    DIAG_CONN_ID_ACPU_LPM3   = 5,
    DIAG_CONN_ID_ACPU_LRM    = 6,
    DIAG_CONN_ID_ACPU_NRM    = 7,
    DIAG_CONN_ID_BUTT
};

enum DIAG_CNF_STATE_TYPE_E
{
    DIAG_CNF_STATE_DEFAULT  = 0,
    DIAG_CNF_STATE_FINISH   = 1,
    DIAG_CNF_BUTT,
};

typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/

    VOS_UINT32 ulMsgTransId;       /* Trans ID */
    VOS_UINT32 ulMsgId;
} DIAG_CMD_CONNECT_HEAD_STRU;

struct CONNECT_CTRL_STRU
{
    HTIMER                      stTimer;
    VOS_UINT32                  ulState;
    DIAG_CMD_CONNECT_HEAD_STRU  stConnectCmd;
    union
    {
        VOS_UINT32          ulID;           /* 结构化ID */
        MSP_DIAG_STID_STRU  stID;
    }connectCommandId;
};

typedef struct{
    VOS_UINT32  ulChannelId;
    VOS_UINT32  ulOwner;
    VOS_UINT32  ulResult;
}DIAG_CONN_CHANN_TBL;

typedef VOS_UINT32 (*DIAG_SEND_PROC_FUNC)(VOS_UINT8 * pData);

typedef struct
{
    VOS_UINT32              ulConnId;       /* DIAG_CONN_ID_TYPE_E */
    VOS_CHAR                cName[16];   /* 子系统名称  */
    DIAG_SEND_PROC_FUNC     pSendFunc;      /* 接收函数 */
    VOS_UINT32              ulResult;       /* connect id的处理结果 */
    VOS_UINT32              ulChannelCount;
    VOS_UINT32              ulHasCnf;
}DIAG_CONNECT_PROC_STRU;

/* 建连失败时的回复 */
typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/
    VOS_UINT32 ulRc;            /* 结果码*/

    VOS_UINT32 ulChannelNum;    /* 通道数量 */
    DIAG_CONNECT_RESULT stResult[0];
}DIAG_CMD_HOST_CONNECT_FAIL_CNF_STRU;


/*****************************************************************************
 描述 : HSO连接UE设备
ID   : DIAG_CMD_HOST_CONNECT
REQ  : DIAG_CMD_HOST_CONNECT_REQ_STRU
CNF  : DIAG_CMD_HOST_CONNECT_CNF_STRU
*****************************************************************************/
typedef struct
{
    VOS_UINT32 ulMajorMinorVersion; /* 主版本号.次版本号*/
    VOS_UINT32 ulRevisionVersion;   /* 修正版本号*/
    VOS_UINT32 ulBuildVersion;      /* 内部版本号*/
} DIAG_CMD_UE_SOFT_VERSION_STRU;

typedef struct
{
    VOS_UINT16 usVVerNo;           /* V部分*/
    VOS_UINT16 usRVerNo;           /* R部分*/
    VOS_UINT16 usCVerNo;           /* C部分*/
    VOS_UINT16 usBVerNo;           /* B部分*/
    VOS_UINT16 usSpcNo;            /* SPC部分*/
    VOS_UINT16 usHardwareVerNo;    /* 硬件PCB号和印制板版本号*/
    VOS_UINT32 ulProductNo;        /* 产品类型编号，即不同外设组合的硬件平台*/
} DIAG_CMD_UE_BUILD_VER_STRU; /* 内部版本*/

typedef struct
{
    /* 010: OM通道融合的版本        */
    /* 110: OM融合GU未融合的版本    */
    /* 100: OM完全融合的版本        */
    VOS_UINT32  ulDrxControlFlag:1; /* DRX部分*/
    VOS_UINT32  ulPortFlag      :1; /* OM Port flag 0:old,1:new */
    VOS_UINT32  ulOmUnifyFlag   :1; /* OM */
    VOS_UINT32  ul5GFrameFlag    :1;
    VOS_UINT32  ulReserved      :28;
}DIAG_CONTROLFLAG_STRU;

/* 建连成功时的回复  */
typedef struct
{
    VOS_UINT32 ulAuid;          /* 原AUID*/
    VOS_UINT32 ulSn;            /* HSO分发，插件命令管理*/

    VOS_UINT32 ulRc;            /* 结果码*/
    VOS_CHAR szImei[16];
    DIAG_CMD_UE_SOFT_VERSION_STRU stUeSoftVersion;
    DIAG_CMD_UE_BUILD_VER_STRU stBuildVersion;
    VOS_UINT32 ulChipBaseAddr;
    union
    {
        VOS_UINT32              UintValue;
        DIAG_CONTROLFLAG_STRU   CtrlFlag;
    } diag_cfg;/* B135新增，标示低功耗特性版本: 1:低功耗版本；0：正常版本；0xFFFFFFFF:MSP读取NV项失败，HSO会认为连接不成功并给出提示，要求重新进行连接*/
    VOS_UINT32 ulLpdMode;
    NV_ITEM_AGENT_FLAG_STRU stAgentFlag;
    VOS_CHAR szProduct[64];
} DIAG_CMD_HOST_CONNECT_CNF_STRU;

typedef struct
{
    VOS_UINT32  ulMsgId;    /* ID_MSG_DIAG_CMD_CONNECT_REQ ID_MSG_DIAG_CMD_DISCONNECT_REQ */
    VOS_UINT32  ulSn;
}DIAG_CONN_MSG_SEND_T;

/* 任何情况下工具下发的链接命令都是旧的贞格式 */
/* 描述 :4G 一级头: service头 */
typedef struct
{
    VOS_UINT32    sid8b       :8;   /* service id, value:DIAG_SID_TYPE */
    VOS_UINT32    mdmid3b     :3;
    VOS_UINT32    rsv1b       :1;
    VOS_UINT32    ssid4b      :4;
    VOS_UINT32    sessionid8b :8;
    VOS_UINT32    mt2b        :2;
    VOS_UINT32    index4b     :4;
    VOS_UINT32    eof1b       :1;
    VOS_UINT32    ff1b        :1;

    VOS_UINT32    ulMsgTransId;
    VOS_UINT8     aucTimeStamp[8];
}DIAG_CONNECT_SERVICE_HEAD_STRU;

typedef struct
{
    DIAG_CONNECT_SERVICE_HEAD_STRU      stService;

    union
    {
        VOS_UINT32          ulCmdId;           /* 结构化ID */
        MSP_DIAG_STID_STRU  stID;
    };

    VOS_UINT32                  ulMsgLen;
#if (VOS_OS_VER == VOS_WIN32)
    VOS_UINT8                   aucData[4];
#else
    VOS_UINT8                   aucData[0];
#endif

}DIAG_CONNECT_FRAME_INFO_STRU;

/* 建链核间通信结构体 */
typedef struct
{
     VOS_MSG_HEADER                     /*VOS头 */
     VOS_UINT32                     ulMsgId;
     VOS_UINT32                     ulCmdId;
     DIAG_CMD_HOST_CONNECT_CNF_STRU stConnInfo;
}DIAG_MSG_MSP_CONN_STRU;

VOS_UINT32 diag_ConnMgrSendFuncReg (VOS_UINT32 ulConnId, VOS_UINT32 ulChannelNum, VOS_UINT32 *ulChannel,DIAG_SEND_PROC_FUNC pSendFuc);
VOS_UINT32 diag_CnfIsFinish(VOS_VOID);
VOS_UINT32 diag_GetChannelResult(VOS_VOID);
VOS_UINT32 diag_ConnectFailReport(VOS_UINT32 ulRet);
VOS_VOID diag_ResetConnectInfo(VOS_VOID);
VOS_UINT32 diag_ConnectSucessReport(VOS_VOID);
VOS_UINT32 diag_ConnCnfFinish(VOS_VOID);
VOS_UINT32 diag_ConnCnf (VOS_UINT32  ulConnId,  VOS_UINT32 ulSn, VOS_UINT32 ulCount, DIAG_CONNECT_RESULT *pstResult);
VOS_VOID diag_GetModemInfo(DIAG_CONNECT_FRAME_INFO_STRU *pstDiagHead);
VOS_UINT32 diag_SendConnectMsg(VOS_UINT8 * pstSendMsg);
VOS_UINT32 diag_GetConnSn(VOS_VOID);
VOS_VOID diag_SetChannelTimeOut(VOS_VOID);
VOS_UINT32 diag_ConnTimerProc(VOS_VOID);
VOS_UINT32 diag_ConnProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_DisConnectSuccessReport(VOS_VOID);
VOS_UINT32 diag_DisConnProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_SetChanDisconn(MsgBlock* pMsgBlock);
VOS_UINT32 diag_ConnMgrProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_DisConnMgrProc(VOS_UINT8* pstReq);
VOS_UINT32 diag_SetChanDisconnProc(MsgBlock* pMsgBlock);


#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif /* end of diag_cfg.h */

