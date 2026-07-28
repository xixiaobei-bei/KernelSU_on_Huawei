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


#ifndef __DIAG_FRAME_H__
#define __DIAG_FRAME_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push)
#pragma pack(4)


/*****************************************************************************
  1 Include Headfile
*****************************************************************************/
#include <product_config.h>
#include <vos.h>
#include <msp.h>

/*****************************************************************************
  2 macro
*****************************************************************************/
#ifdef DIAG_SYSTEM_5G
#define SERVICE_HEAD_SID(pData)   ((VOS_UINT32)(((DIAG_SERVICE_HEAD_STRU *)pData)->sid4b))
#define DIAG_4G_FRAME_HEAD_LEN    (sizeof(DIAG_SERVICE_HEAD_STRU) + sizeof(VOS_UINT32))
#define DIAG_SERVICE_HEAD_VER(pData) (((DIAG_SERVICE_HEAD_STRU *)pData)->ver4b)
#else
#define SERVICE_HEAD_SID(pData)         ((VOS_UINT32)(((DIAG_SERVICE_HEAD_STRU *)pData)->sid8b))
#define DIAG_4G_FRAME_HEAD_LEN          (sizeof(DIAG_SERVICE_HEAD_STRU))
#define DIAG_SERVICE_HEAD_VER(pData)    ((((DIAG_SERVICE_HEAD_STRU *)pData)->sid8b)&0xF0)//sid8b在5G中为sid4b ver4b 取
#endif

/* DIAG_SERVICE_HEAD_STRU:sessionid8b */
#define MSP_SERVICE_SESSION_ID        (0x1) /* 标识Service与Client之间的连接,固定为1*/

/* DIAG二级三级头的长度 */
#define DIAG_MESSAGE_DATA_HEADER_LEN  (sizeof(DIAG_SERVICE_HEAD_STRU)   \
                                     + sizeof(DIAG_MSG_INFO_STRU)       \
                                     + sizeof(MSP_DIAG_DATA_REQ_STRU))

/*****************************************************************************
  3 Enum
*****************************************************************************/
/* ==============服务层头结构枚举值定义==================================== */

/* DIAG_SERVICE_HEAD_STRU:ssid4b */
enum DIAG_SSID_TYPE
{
#ifdef DIAG_SYSTEM_5G
    DIAG_SSID_DEFAULT       = 0x0,  /* 0x00：未指定（仅下发REQ使用，表示由UE确定分发目标，CNF和IND不能使用该值） */
    DIAG_SSID_APP_CPU       = 0x1,  /* 0x01：A-CPU */
    DIAG_SSID_MODEM_CPU     = 0x2,  /* 0x02：2G/3G/4.5G子系统C-CPU（底软、PS使用） */
    DIAG_SSID_TLDSP_BBE_NX  = 0x3,  /* 0x03：BBE NX（TL DSP使用） */
    DIAG_SSID_BBP_DEBUG     = 0x4,  /* 0x04：2G/3G/4.5G BBP Debug   //原来是LTE BBP */
    DIAG_SSID_GUC_BBE_NX    = 0x5,  /* 0x05：BBE NX（GUC DSP SDR使用）  //因GUC PHY与TL PHY没有融合，仍需单独分配 */
    DIAG_SSID_HIFI          = 0x6,
    DIAG_SSID_LTE_V_DSP     = 0x7,  /* 0x07：LTE-V DSP（预留）//原来是TDS DSP */
    DIAG_SSID_RESERVE0      = 0x8,
    DIAG_SSID_MCU           = 0x9,
    DIAG_SSID_TEE           = 0xA,  /* 0x0A：TEE        //原来是GPU */
    DIAG_SSID_RESERVE1      = 0xB,  /* 0x0B：保留       //原来是GUX BBP */
    DIAG_SSID_IOM3          = 0xC,
    DIAG_SSID_EASYRF0       = 0xD,
    DIAG_SSID_X_DSP         = 0xE,
    DIAG_SSID_GUC_L1C       = 0xE,  /* 0x0E：2G/3G/4.5G子系统C-CPU（GUC L1C使用）   //因GUC L1C没有通过C核MSP进行OAM数据上报，仍需单独分配 */
    DIAG_SSID_RESERVE2      = 0xF,
    DIAG_SSID_5G_CCPU       = 0x10,
    DIAG_SSID_L2HAC         = 0x11,
    DIAG_SSID_HL1C          = 0x12,
    DIAG_SSID_LL1C_CORE0    = 0x13,
    DIAG_SSID_LL1C_CORE1    = 0x14,
    DIAG_SSID_LL1C_CORE2    = 0x15,
    DIAG_SSID_LL1C_CORE3    = 0x16,
    DIAG_SSID_LL1C_CORE4    = 0x17,
    DIAG_SSID_LL1C_CORE5    = 0x18,
    DIAG_SSID_SDR_CORE0     = 0x19,
    DIAG_SSID_SDR_CORE1     = 0x1A,
    DIAG_SSID_SDR_CORE2     = 0x1B,
    DIAG_SSID_SDR_CORE3     = 0x1C,
    DIAG_SSID_5G_BBP_DEBUG  = 0x1D,
    DIAG_SSID_EASYRF1       = 0x1E,
    DIAG_SSID_BBP_ACCESS_DEBUG= 0x1F,
    DIAG_SSID_BUTT,
#else
    DIAG_SSID_APP_CPU   = 0x1,
    DIAG_SSID_MODEM_CPU,
    DIAG_SSID_LTE_DSP,
    DIAG_SSID_LTE_BBP,
    DIAG_SSID_GU_DSP,
    DIAG_SSID_HIFI,
    DIAG_SSID_TDS_DSP,
    DIAG_SSID_TDS_BBP,
    DIAG_SSID_MCU,
    DIAG_SSID_GPU,
    DIAG_SSID_GU_BBP,
    DIAG_SSID_IOM3,
    DIAG_SSID_ISP,
    DIAG_SSID_X_DSP,
    DIAG_SSID_BUTT
#endif
};
typedef VOS_UINT32 DIAG_SSID_TYPE_U32;

/* DIAG_SERVICE_HEAD_STRU:mt2b */
enum DIAG_MSGTYPE_TYPE
{
    DIAG_MT_RSV   = 0x0,
    DIAG_MT_REQ   = 0x1,
    DIAG_MT_CNF   = 0x2,
    DIAG_MT_IND   = 0x3
};
typedef VOS_UINT32 DIAG_DIRECTION_TYPE_U32;

/* MSP_DIAG_STID_STRU:pri4b */
enum DIAG_MSG_SUB_TYPE_TYPE
{
    DIAG_MSG_CMD        = 0x0,
    DIAG_MSG_PS_AIR     = 0x1,
    DIAG_MSG_PS_LAYER   = 0x2,
    DIAG_MSG_PS_PRINT   = 0x3,
    DIAG_MSG_PS_EVENT   = 0x4,
    DIAG_MSG_PS_USER    = 0x5,
    DIAG_MSG_PS_VOLTE   = 0x6,
    DIAG_MSG_PS_STRUCT  = 0x7,
    DIAG_MSG_PS_DOT     = 0x8,
    DIAG_MSG_DSP_PRINT  = 0x9,
    DIAG_MSG_RAE_CNF    = 0xa,
    DIAG_MSG_RAE_IND    = 0xb,
    DIAG_MSG_STAT       = 0x1f
};
typedef VOS_UINT32 DIAG_MSG_SUB_TYPE_U32;

/* DIAG_SERVICE_HEAD_STRU:sid8b */
enum DIAG_SID_TYPE
{
    MSP_SID_DEFAULT   = 0x0,
    MSP_SID_AT_SERVICE,
    MSP_SID_DIAG_SERVICE,
    MSP_SID_DATA_SERVICE,
    MSP_SID_NV_SERVICE,
    MSP_SID_USIM_SERVICE,
    MSP_SID_DM_SERVICE,
    MSP_SID_CBT_SERVICE,
    MSP_SID_BUTT
};

typedef unsigned int MSP_SID_TYPE_U32;

enum DIAG_FRAME_VER_TYPE
{
    DIAG_FRAME_VER_4G = 0,
    DIAG_FRAME_VER_5G = 1,
    DIAG_FRAME_VER_BUTT
};

/* ======================================================================== */

/*****************************************************************************
  4 UNION
*****************************************************************************/

/*****************************************************************************
  5 struct
*****************************************************************************/
/* ==============帧结构描述================================================ */

#ifdef DIAG_SYSTEM_5G
/* 描述 :5G 一级头: service头 */
typedef struct
{
    VOS_UINT32    sid4b       :4;   /* service id, value:DIAG_SID_TYPE */
    VOS_UINT32    ver4b       :4;   /* version , value:DIAG_SERVICE_HEAD_VER_TYPE */
    VOS_UINT32    mdmid3b     :3;   /* modem id dfd*/
    VOS_UINT32    rsv5b       :5;
    VOS_UINT32    ssid8b      :8;   /* sub system id , DIAG_SSID_TYPE, CCPU/ACPU/BBE NX/Audio Dsp/LTE-V DSP..... */
    VOS_UINT32    mt2b        :2;
    VOS_UINT32    index4b     :4;
    VOS_UINT32    eof1b       :1;
    VOS_UINT32    ff1b        :1;

    VOS_UINT16    ulMsgTransId;
    VOS_UINT16    ulSocpEncDstId;

    VOS_UINT8     aucTimeStamp[4];
}DIAG_SERVICE_HEAD_STRU;
#else
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
}DIAG_SERVICE_HEAD_STRU;
#endif

/* 描述 :二级头: DIAG消息头 */
typedef struct
{
    VOS_UINT32    cmdid19b:19;
    VOS_UINT32    sec5b   :5;
    VOS_UINT32    mode4b  :4;
    VOS_UINT32    pri4b   :4;
} MSP_DIAG_STID_STRU;

/* 描述 :三级头: 工具软件信息头，用于REQ/CNF消息 */
typedef struct
{
    VOS_UINT32 ulAuid;         /* 原AUID*/
    VOS_UINT32 ulSn;           /* HSO分发，插件命令管理*/
    VOS_UINT8  ucData[0];      /* 参数的数据*/  /*lint !e43 */
} MSP_DIAG_DATA_REQ_STRU;

typedef MSP_DIAG_DATA_REQ_STRU MSP_DIAG_DATA_CNF_STRU;

typedef struct
{
    union
    {
        VOS_UINT32          ulID;           /* 结构化ID */
        MSP_DIAG_STID_STRU  stID;
    };

    VOS_UINT32          ulDataSize;     /* ucData的长度 */
    VOS_UINT8           aucData[0];     /*lint !e43 */
}DIAG_MSG_INFO_STRU;

/* 描述 :整体帧结构 */
typedef struct
{
    DIAG_SERVICE_HEAD_STRU      stService;

    union
    {
        VOS_UINT32          ulCmdId;           /* 结构化ID */
        MSP_DIAG_STID_STRU  stID;
    };

    VOS_UINT32                  ulMsgLen;
    VOS_UINT8                   aucData[0];     /*lint !e43 */
}DIAG_FRAME_INFO_STRU;

/* ======================================================================== */
/*****************************************************************************
  6 Extern Global Variable
*****************************************************************************/

/*****************************************************************************
  7 Fuction Extern
*****************************************************************************/
#pragma pack(pop)

#ifdef __cplusplus
    #if __cplusplus
                }
    #endif
#endif

#endif

