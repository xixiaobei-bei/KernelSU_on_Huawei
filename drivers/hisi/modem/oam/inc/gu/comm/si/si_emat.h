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


#ifndef __SI_EMAT_H__
#define __SI_EMAT_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "si_app_emat.h"
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "pamom.h"
#include "UsimmApi.h"
#include "ccore_nv_stru_pam.h"
#endif

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#include "pamappom.h"
#include "SiAcoreApi.h"
#endif

#include "msp_nvim.h"
#include "nv_stru_pam.h"
#include "AtOamInterface.h"
#include "MnErrorCode.h"
#include "msp_diag_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)

/*****************************************************************************
  2 宏定义
*****************************************************************************/
#if  ((OSA_CPU_ACPU == VOS_OSA_CPU) || (VOS_OS_VER == VOS_WIN32))
#define EMAT_GEN_LOG_MODULE(Level)           (DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_COMM, Level))

#define EMAT_KEY_INFO_LOG(string)            (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s", string)
#define EMAT_KEY_NORMAL_LOG(string)          (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s", string)
#define EMAT_KEY_WARNING_LOG(string)         (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s", string)
#define EMAT_KEY_ERROR_LOG(string)           (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "ERROR:%s", string)

#define EMAT_KEY_INFO_LOG1(string, para1)    (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s,%d", string, para1)
#define EMAT_KEY_NORMAL_LOG1(string, para1)  (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s,%d", string, para1)
#define EMAT_KEY_WARNING_LOG1(string, para1) (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s,%d", string, para1)
#define EMAT_KEY_ERROR_LOG1(string, para1)   (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "ERROR:%s,%d", string, para1)

#define EMAT_INFO_LOG(string)                (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s", string)
#define EMAT_NORMAL_LOG(string)              (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s", string)
#define EMAT_WARNING_LOG(string)             (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s", string)
#define EMAT_ERROR_LOG(string)               (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "ERROR:%s", string)

#define EMAT_INFO_LOG1(string, para1)        (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_INFO),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s,%d", string, para1)
#define EMAT_NORMAL_LOG1(string, para1)      (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "NORMAL:%s,%d", string, para1)
#define EMAT_WARNING_LOG1(string, para1)     (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s,%d", string, para1)
#define EMAT_ERROR_LOG1(string, para1)       (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "ERROR:%s,%d", string, para1)

#define EMAT_WARNING_NOSLOTID_LOG(string)    (VOS_VOID)DIAG_LogReport(EMAT_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING),I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s", string)

#elif  (OSA_CPU_CCPU == VOS_OSA_CPU)

#define EMAT_KEY_WARNING_LOG(string)         USIMM_LogPrint(enSlotId, PS_LOG_LEVEL_WARNING, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s", string)
#define EMAT_KEY_ERROR_LOG(string)           USIMM_LogPrint(enSlotId, PS_LOG_LEVEL_ERROR, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "ERROR:%s", string)

#define EMAT_KEY_INFO_LOG1(string, para1)    USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_INFO, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "Info:%s,%d", string, para1)
#define EMAT_KEY_NORMAL_LOG1(string, para1)  USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_NORMAL, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "Normal:%s,%d", string, para1)
#define EMAT_KEY_WARNING_LOG1(string, para1) USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_WARNING, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "WARNING:%s,%d", string, para1)
#define EMAT_KEY_ERROR_LOG1(string, para1)   USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_ERROR, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "ERROR:%s,%d", string, para1)

#define EMAT_WARNING_NOSLOTID_LOG(string)    USIMM_LogPrint(USIMM_GetCurrInstanceSlotId(), PS_LOG_LEVEL_WARNING, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "Warning:%s", string)

#define EMAT_INFO_LOG(string)                USIMM_LogPrint(enSlotId, PS_LOG_LEVEL_INFO, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s", (VOS_CHAR*)__FUNCTION__)
#define EMAT_NORMAL_LOG(string)              USIMM_LogPrint(enSlotId, PS_LOG_LEVEL_NORMAL, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__,"%s", (VOS_CHAR*)__FUNCTION__)
#define EMAT_WARNING_LOG(string)             USIMM_LogPrint(enSlotId, PS_LOG_LEVEL_WARNING, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s", (VOS_CHAR*)__FUNCTION__)
#define EMAT_ERROR_LOG(string)               USIMM_LogPrint(enSlotId, PS_LOG_LEVEL_ERROR, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s", (VOS_CHAR*)__FUNCTION__)

#define EMAT_INFO_LOG1(string, para1)        USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_INFO, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s: %d", (VOS_CHAR*)__FUNCTION__, para1)
#define EMAT_NORMAL_LOG1(string, para1)      USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_NORMAL, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s: %d", (VOS_CHAR*)__FUNCTION__, para1)
#define EMAT_WARNING_LOG1(string, para1)     USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_WARNING, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s: %d", (VOS_CHAR*)__FUNCTION__, para1)
#define EMAT_ERROR_LOG1(string, para1)       USIMM_LogPrint1(enSlotId, PS_LOG_LEVEL_ERROR, I0_MAPS_EMAT_PID, VOS_NULL_PTR, __LINE__, "%s: %d", (VOS_CHAR*)__FUNCTION__, para1)
#endif

#define EMAT_TAGNOTFOUND                     0xFFFFFFFF

#define EMAT_ARRAYSIZE(array)                (sizeof(array)/sizeof(array[0]))

/* PKID ListForVerification的TAG */
#define EMAT_ESIM_PKID_LFV_TAG               0xA9
/* PKID ListForSigning的TAG */
#define EMAT_ESIM_PKID_LFS_TAG               0xAA

#define EMAT_ESIM_EID_DATA_LEN               21
#define EMAT_ESIM_PKID_DATA_MIN_LEN          17
#define EMAT_ESIM_PKID_DATA_MAX_LEN          256
#define EMAT_ESIM_CLEAN_DATA_LEN             6
#define EMAT_ESIM_CHECK_DATA_MIN_LEN         5

/*******************************************************************************
  3 枚举定义
*******************************************************************************/
enum SI_EMAT_REQ_ENUM
{
    SI_EMAT_ESIM_EID_QUERY_REQ      = 0,
    SI_EMAT_ESIM_PKID_QUERY_REQ     = 1,
    SI_EMAT_ESIM_CLEAN_REQ          = 2,
    SI_EMAT_ESIM_CHECK_REQ          = 3,

    SI_EMAT_REQ_BUTT
};
typedef VOS_UINT32      SI_EMAT_REQ_ENUM_UINT32;

enum SI_EMAT_STATUS_ENUM
{
    SI_EMAT_STATUS_IDLE             = 0,
    SI_EMAT_STATUS_OPEN_CHANNEL     = 1,
    SI_EMAT_STATUS_EID_WORKING      = 2,
    SI_EMAT_STATUS_PKID_WORKING     = 3,
    SI_EMAT_STATUS_CLEAN_WORKING    = 4,
    SI_EMAT_STATUS_CHECK_WORKING    = 5,
    SI_EMAT_STATUS_CLOSE_CHANNEL    = 6,
    SI_EMAT_STATUS_EID_CNF          = 7,
    SI_EMAT_STATUS_PKID_CNF         = 8,
    SI_EMAT_STATUS_CLEAN_CNF        = 9,
    SI_EMAT_STATUS_CHECK_CNF        = 10,

    SI_EMAT_STATUS_BUTT
};
typedef VOS_UINT32     SI_EMAT_STATUS_ENUM_UINT32;

/*****************************************************************************
  4 单一数据结构定义
*****************************************************************************/

typedef VOS_UINT32 (*PFSIEMATPIDMSGPROC)(SI_PIH_CARD_SLOT_ENUM_UINT32 enSlotId, PS_SI_MSG_STRU *pMsg);


typedef VOS_UINT32 (*PFSIEMATSTATUSMSGPROC)(SI_PIH_CARD_SLOT_ENUM_UINT32 enSlotId);


typedef struct
{
    VOS_UINT32                          ulMsgPid;
    PFSIEMATPIDMSGPROC                  pProcFunc;        /* 处理函数 */
}SI_EMAT_PIDMSGPROC_FUNC;


typedef struct
{
    SI_EMAT_STATUS_ENUM_UINT32          enCurStatus;      /* 当前的处理状态 */
    PFSIEMATSTATUSMSGPROC               pProcFunc;        /* 处理函数 */
}SI_EMAT_STATUS_MSGPROC_FUNC;



typedef struct
{
    VOS_MSG_HEADER
    VOS_UINT32                          ulMsgName;       /* 消息名 */
    VOS_UINT16                          usClient;        /* 客户端ID */
    VOS_UINT8                           ucOpID;
    VOS_UINT8                           ucRsv;
    SI_EMAT_EVENT_ENUM_UINT32           enEventType;
}SI_EMAT_MSG_HEADER_STRU;


typedef struct
{
    SI_EMAT_REQ_ENUM_UINT32             enCurReq;        /* 当前正在处理的AT命令消息 */
    SI_EMAT_STATUS_ENUM_UINT32          enCurStatus;     /* 当前AT命令的处理状态 */
    VOS_UINT32                          ulAppType;       /* 当前通道的应用类型 */
    SI_EMAT_EVENT_ENUM_UINT32           enEventType;     /* 当前事件类型 */
    VOS_UINT16                          usClient;        /* 客户端ID */
    VOS_UINT8                           ucOpID;          /* 操作ID */
    VOS_UINT8                           ucChannelID;     /* 当前通道的channelId */
}SI_EMAT_CURRNET_STATUS_STRU;

/*****************************************************************************
  5 全局变量声明
*****************************************************************************/

/*****************************************************************************
  6 函数声明
*****************************************************************************/
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
VOS_VOID I0_SI_EMAT_PidMsgProc(PS_SI_MSG_STRU *pMsg);

VOS_UINT32 I0_WuepsEMATPidInit(enum VOS_INIT_PHASE_DEFINE InitPhrase);

VOS_VOID I1_SI_EMAT_PidMsgProc(PS_SI_MSG_STRU *pMsg);

VOS_UINT32 I1_WuepsEMATPidInit(enum VOS_INIT_PHASE_DEFINE InitPhrase);

VOS_VOID I2_SI_EMAT_PidMsgProc(PS_SI_MSG_STRU *pMsg);

VOS_UINT32 I2_WuepsEMATPidInit(enum VOS_INIT_PHASE_DEFINE InitPhrase);

#endif

VOS_UINT32 SI_COMM_BerFindTag(
    VOS_UINT8                            ucTag,
    VOS_UINT8                           *pucData,
    VOS_UINT32                           ulSpareBuffSize,
    VOS_UINT32                           ulDataLen,
    VOS_UINT32                           ulFindNum);

#if ((VOS_OS_VER == VOS_WIN32) || (VOS_OS_VER == VOS_NUCLEUS))
#pragma pack()
#else
#pragma pack(0)
#endif

#ifdef __cplusplus
#if __cplusplus
        }
#endif
#endif

#endif

