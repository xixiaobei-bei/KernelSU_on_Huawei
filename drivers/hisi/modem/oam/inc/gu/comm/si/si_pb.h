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
 * 版权所有（c）华为技术有限公司 2006-2019
 * 功能描述: 电话本的的头文件
 * 日    期: 2006年11月14日
 */
#ifndef __SI_PB_H__
#define __SI_PB_H__

#include "vos.h"
#include "si_app_pb.h"
#include "nv_stru_pam.h"
#include "usimm_ps_interface.h"

#ifdef MODEM_FUSION_VERSION
#include "mdrv_diag.h"
#else
#include "msp_diag_comm.h"
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "pam_om.h"
#include "ccore_nv_stru_pam.h"
#include "ps_comm_func.h"
#include "usimm_print.h"
#include "usimm_api.h"
#endif

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
#include "si_acore_api.h"
#endif

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) && (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF))
#include "pam_app_om.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#if (FEATURE_PHONE_USIM == FEATURE_ON)

#define SI_PB_XDN_SPACE 60000
#define SI_PB_XDN_SUPPORT_NUM 1000

#define SI_PB_ADNMAX 10 /* 最多支持ADN电话本个数 */
#define SI_PB_ANRMAX 3
#define SI_PB_MAX_NUMBER 7 /* 最多支持电话本个数 */
#define SI_PB_MAX_XDN 16   /* 初始化时支持的电话本总数 */
#define SI_PB_ANR_MAX 3    /* ANR文件的个数 */

#define SI_PB_FILE_NOT_EXIST 0
#define SI_PB_FILE_EXIST 1

#define SI_PB_EXCEPT_NAME 14
#define SI_PB_ECC_EXCEPT_NAME 4
#define SI_PB_NUM_LEN 20
#define SI_PB_ECC_LEN 3

#define SI_PB_ANR_LEN 17
#define SI_PB_EXT_LEN 13
#define SI_PB_EXT_NUM_LEN 20
#define SI_PB_NUM_WITHEXT (SI_PB_NUM_LEN + SI_PB_EXT_NUM_LEN)
#define SI_PB_ADDITIONAL_NUM 2
#define SI_PB_READ_STR_LEN 256

#define SI_PB_TAGNOTFOUND 0xFFFFFFFF

#define SI_CLEARALL 0x01
#define SI_CLEARXDN 0x02
#define SI_CLEARSPB 0x03

#define EFADNDO_TAG 0xC0
#define EFIAPDO_TAG 0xC1
#define EFEXT1DO_TAG 0xC2
#define EFSNEDO_TAG 0xC3
#define EFANRDO_TAG 0xC4
#define EFPBCDO_TAG 0xC5
#define EFGRPDO_TAG 0xC6
#define EFAASDO_TAG 0xC7
#define EFGASDO_TAG 0xC8
#define EFUIDDO_TAG 0xC9
#define EFEMAILDO_TAG 0xCA
#define EFCCP1DO_TAG 0xCB

#define EFPBR 0x4F30
#define SIM_EFADN 0x6F3A
#define EFSDN 0x6F49

#define EFEXT1 0x6F4A      /* 用于ADN文件 */
#define EFEXT2 0x6F4B      /* 用于FDN文件 */
#define EFEXT3 0x6F4C      /* 用于SDN文件 */
#define EFEXT4_USIM 0x6F55 /* 用于BDN文件 */
#define EFEXT4_SIM 0x6F4E  /* 用于BDN文件 */
#define EFEXT5 0x6F4E      /* 仅存在于USIM卡中的MSISDN文件 */
#define EFEXT6 0x6FC8      /* 用于MBDN文件 */
#define EFEXT7 0x6FCC      /* 用于CFIS文件 */

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (VOS_OS_VER == VOS_WIN32)  || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))

#define PB_GEN_LOG_MODULE(Level) (MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_COMM, Level))

#define PB_KEY_INFO_LOG(string)                                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_KEY_NORMAL_LOG(string)                                                                                     \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_KEY_WARNING_LOG(string)                                                                                     \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_KEY_ERROR_LOG(string)                                                                                      \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_KEY_INFO1_LOG(string, para1)                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)

#define PB_KEY_NORMAL1_LOG(string, para1)                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)

#define PB_KEY_WARNING1_LOG(string, para1)                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)

#define PB_KEY_ERROR1_LOG(string, para1)                                                                       \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)

#define PB_INFO_LOG(string)                                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_NORMAL_LOG(string)                                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_WARNING_LOG(string)                                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_ERROR_LOG(string)                                                                                     \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_INFO1_LOG(string, para1)                                                                             \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string ",%d", para1)

#define PB_NORMAL1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string ",%d", para1)

#define PB_WARNING1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string ",%d", para1)

#define PB_ERROR1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string ",%d", para1)

#elif (OSA_CPU_CCPU == VOS_OSA_CPU)
#define PB_GEN_LOG_MODULE(slotId, Level) \
    (MDRV_DIAG_GEN_LOG_MODULE(USIMM_GetModemIdBySlotId(slotId), DIAG_MODE_COMM, Level))

#define PB_KEY_INFO_LOG(string) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_INFO), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Info:" string)

#define PB_KEY_NORMAL_LOG(string) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_NORMAL), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Normal:" string)

#define PB_KEY_WARNING_LOG(string) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_WARNING), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Warning:" string)

#define PB_KEY_ERROR_LOG(string) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_ERROR), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Error:" string)

#define PB_KEY_INFO1_LOG(string, para1) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_INFO), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Info:" string ",%d", para1)

#define PB_KEY_NORMAL1_LOG(string, para1) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_NORMAL), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Normal:" string ",%d", para1)

#define PB_KEY_WARNING1_LOG(string, para1) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_WARNING), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Warning:" string ",%d", para1)

#define PB_KEY_ERROR1_LOG(string, para1) \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_ERROR), \
        USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), VOS_NULL_PTR, __LINE__, "Error:" string ",%d", para1)

#ifndef MODEM_FUSION_VERSION

#define PB_INFO_LOG(string) USIMM_LogPrint(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_INFO),                              \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s", \
        (VOS_CHAR *)__FUNCTION__)
#define PB_NORMAL_LOG(string)                                                                 \
    USIMM_LogPrint(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_NORMAL),                            \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s", \
        (VOS_CHAR *)__FUNCTION__)
#define PB_WARNING_LOG(string)                                                                \
    USIMM_LogPrint(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_WARNING),                           \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s", \
        (VOS_CHAR *)__FUNCTION__)
#define PB_ERROR_LOG(string) USIMM_LogPrint(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_ERROR),                             \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s", \
        (VOS_CHAR *)__FUNCTION__)

#define PB_INFO1_LOG(string, para1)                                                               \
    USIMM_LogPrint1(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_INFO),                                 \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s: %d", \
        (VOS_CHAR *)__FUNCTION__, para1)
#define PB_NORMAL1_LOG(string, para1)                                                             \
    USIMM_LogPrint1(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_NORMAL),                               \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s: %d", \
        (VOS_CHAR *)__FUNCTION__, para1)
#define PB_WARNING1_LOG(string, para1)                                                            \
    USIMM_LogPrint1(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_WARNING),                              \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s: %d", \
        (VOS_CHAR *)__FUNCTION__, para1)
#define PB_ERROR1_LOG(string, para1)                                                              \
    USIMM_LogPrint1(PB_GEN_LOG_MODULE(slotId, PS_LOG_LEVEL_ERROR),                                \
        USIMM_GEN_LOG_PID_LINE(USIMM_GetPidBySlotId(I0_MAPS_PB_PID, slotId), __LINE__), "%s: %d", \
        (VOS_CHAR *)__FUNCTION__, para1)
#else
#define PB_INFO_LOG(string) PB_KEY_INFO_LOG(string)

#define PB_NORMAL_LOG(string) PB_KEY_NORMAL_LOG(string)

#define PB_WARNING_LOG(string) PB_KEY_WARNING_LOG(string)

#define PB_ERROR_LOG(string) PB_KEY_ERROR_LOG(string)

#define PB_INFO1_LOG(string, para1) PB_KEY_INFO1_LOG(string, para1)

#define PB_NORMAL1_LOG(string, para1) PB_KEY_NORMAL1_LOG(string, para1)

#define PB_WARNING1_LOG(string, para1) PB_KEY_WARNING1_LOG(string, para1)

#define PB_ERROR1_LOG(string, para1) PB_KEY_ERROR1_LOG(string, para1)
#endif
#endif

#if (VOS_WIN32 == VOS_OS_VER)
#define PB_MALLOC(LENGTH) VOS_MemAlloc(MAPS_PB_PID, DYNAMIC_MEM_PT, LENGTH)
#define PB_FREE(MEMADDR) VOS_MemFree(MAPS_PB_PID, MEMADDR)
#else
#define PB_MALLOC(LENGTH) VOS_CacheMemAllocDebug((VOS_UINT32)LENGTH, (VOS_UINT32)SI_PB_MEM_ALLOC)
#define PB_FREE(MEMADDR) VOS_CacheMemFree(MEMADDR)
#endif

enum SI_PB_InitStep {
    PB_INIT_PB_STATUS = 0, /* 初始化各个电话本的激活状态 */
    PB_INIT_XDN_SPACE,     /* 初始化XDN空间 */
    PB_INIT_EXT_SPACE,     /* 初始化EXT空间 */
    PB_INIT_XDN_CONTENT,   /* 初始化XDN内容 */
    PB_INIT_EXT_CONTENT,   /* 初始化EXT内容 */
    PB_INIT_XDN_SEARCH,    /* 查找XDN记录 */
    PB_INIT_XDN_CONTENT2,  /* 根据查找的结果来初始化XDN文件的内容 */
    PB_INIT_INFO_APP,      /* 回调上报 */
    PB_INIT_FINISHED,      /* 初始化结束 */
    PB_INIT_BUTT
};
typedef VOS_UINT32 SI_PB_InitStepUint32;

enum SI_PB_Search {
    PB_SEARCH_DISABLE = 0,
    PB_SEARCH_ENABLE = 1,
    PB_SEARCH_BUTT
};
typedef VOS_UINT32 SI_PB_SearchUint32;

enum SI_PB_Lock {
    PB_UNLOCK = 0,
    PB_LOCKED = 1,
    PB_LOCK_BUTT
};
typedef VOS_UINT32 SI_PB_LockUint32;

enum SI_PB_FileType {
    PB_FILE_UNKNOWN = 0,
    PB_FILE_TYPE1 = 0xA8,
    PB_FILE_TYPE2 = 0xA9,
    PB_FILE_TYPE3 = 0xAA,
    PB_FILE_BUTT
};
typedef VOS_UINT32 SI_PB_FileTypeUint32;

/* 这里的定义要与SI_PB_STORAGE_SM待的定义保持一致 */
enum SI_PB_Type {
    PB_ADN = 1,
    PB_MISDN = 4,
    PB_FDN = 5,
    PB_ECC = 6,
    PB_BDN = 7,
    PB_SDN = 8,
    PB_XECC = 9,
    PB_NULL = 0xFF,
    PB_TYPE_BUTT
};
typedef VOS_UINT32 SI_PB_TypeUint32;

enum SI_PB_ContentType {
    PB_ECC_CONTENT = 0,
    PB_ADN_CONTENT = 1,
    PB_FDN_CONTENT = 2,
    PB_BDN_CONTENT = 3,
    PB_MSISDN_CONTENT = 4,
    PB_SDN_CONTENT = 5,
    PB_XECC_CONTENT = 6,
    PB_CONTENT_BUTT /* 需要和SI_PB_MAX_NUMBER保持一致，目前最多支持7个电话本 */
};
typedef VOS_UINT32 SI_PB_ContentTypeUint32;

enum SI_PB_SendReqStatus {
    PB_REQ_SEND = 0,
    PB_REQ_NOT_SEND = 1,
    PB_SEND_REQ_BUTT
};
typedef VOS_UINT32 SI_PB_SendReqStatusUint32;

enum SI_PB_InitStateType {
    PB_NOT_INITIALISED = 0, /* 未初始化完成 */
    PB_INITIALISED = 1,     /* 已初始化完成 */
    PB_FILE_NOT_EXIST = 2,  /* 电话本文件不存在 */
    PB_INIT_STATE_BUTT
};
typedef VOS_UINT8 SI_PB_InitStateUint8;

enum SI_PB_ActiveState {
    SI_PB_DEACTIVE = USIMM_EFSTATUS_DEACTIVE,
    SI_PB_ACTIVE = USIMM_EFSTATUS_ACTIVE,
    SI_PB_ACTIVE_STATE_BUTT
};
typedef VOS_UINT32 SI_PB_ActiveStateUint32;

enum SI_PB_Refresh {
    SI_PB_REFRESH_ALL = 0,
    SI_PB_REFRESH_FILE = 1,
    SI_PB_REFRESH_BUTT
};
typedef VOS_UINT32 SI_PB_RefreshUnit32;

enum SI_PB_InitStatus {
    SI_PB_INIT_STATUS_OK = 0,
    SI_PB_INIT_PBMALLOC_FAIL = 1,
    SI_PB_INIT_SPBMALLOC_FAIL = 2,
    SI_PB_INIT_PBTYPE_UNKOWN = 3,
    SI_PB_INIT_USIMPBCNF_ERR = 4,
    SI_PB_INIT_USIMSPBCNF_ERR = 5,
    SI_PB_INIT_RECORD_REMAIN = 6,
    SI_PB_INIT_EXT_RECORD = 7,
    SI_PB_INIT_EXT_ERR = 8,
    SI_PB_INIT_RECORD_EXCEED = 9,
    SI_PB_INIT_FILE_DECATIVE = 10,
    SI_PB_INIT_SEARCH_OK = 11,
    SI_PB_INIT_SEARCH_ERR = 12,
    SI_PB_INIT_MSGTYPE_ERR = 13,
    SI_PB_INIT_PROC_EXCEPTION = 14,
    SI_PB_INIT_ERR_BUTT
};
typedef VOS_UINT32 SI_PB_InitStatusUnit32;

/* 读取号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    VOS_UINT16 index1;
    VOS_UINT16 index2;
} SI_PB_ReadReq;

/* 查询号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
} SI_PB_QueryReq;

/* 设置号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名     */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
} SI_PB_SetReq;

/* 添加号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    SI_PB_Record record;
} SI_PB_AddRep;

/* 更改号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    SI_PB_Record record;
} SI_PB_ModifyRep;

/* 删除号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    VOS_UINT16 index;
    VOS_UINT16 usReserved;
} SI_PB_DeleteReq;

/* 更新号码文件请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;    /* 消息名 */
    VOS_UINT32 fileNumber; /* 更新文件个数 */
    SI_PB_RefreshUnit32 refreshType;
    VOS_UINT16 file[SI_PB_MAX_XDN];
} SI_PB_RefreshReq;

/* 查找姓名请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    VOS_UINT8 length;       /* 要匹配字段的长度 */
    VOS_UINT8 content[243]; /* 要匹配的字段内容 */
} SI_PB_SearchReq;

/* EXT控制信息 */
typedef struct {
    VOS_UINT16 extFlag;
    VOS_UINT16 extFileId;
    VOS_UINT16 extTotalNum;
    VOS_UINT16 extUsedNum;
    VOS_UINT32 contentSize;
    VOS_UINT8 *extContent;
} SI_EXT_Content;

/* 电话本信息 */
typedef struct {
    SI_PB_TypeUint32 pbType;
    SI_PB_ActiveStateUint32 activeStatus;
    VOS_UINT16 totalNum;
    VOS_UINT16 usedNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 nameLen;
    VOS_UINT8 numberLen;
    SI_PB_InitStateUint8 initialState;
    VOS_UINT32 extInfoNum; /* 对应的EXT文件控制信息编号 */
    VOS_UINT32 indexSize;
    VOS_UINT8 *index;
    VOS_UINT32 contentSize;
    VOS_UINT8 *content;
} SI_PB_Content;

/* 补充电话本信息 */
typedef struct {
    SI_PB_ActiveStateUint32 activeStatus;
    VOS_UINT16 totalNum;
    VOS_UINT16 usedNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 numberLen;
    SI_PB_InitStateUint8 initialState;
    VOS_UINT8 rsv;
    VOS_UINT32 contentSize;
    VOS_UINT8 *content;
} SI_ANR_Content;

/* Email电话本信息 */
typedef struct {
    SI_PB_ActiveStateUint32 activeStatus;
    VOS_UINT16 totalNum;
    VOS_UINT16 usedNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 dataLen;
    SI_PB_InitStateUint8 initialState;
    VOS_UINT8 rsv;
    VOS_UINT32 contentSize;
    VOS_UINT8 *content;
} SI_EML_Content;

/* IAP文件信息 */
typedef struct {
    VOS_UINT16 totalNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 rsv;
    VOS_UINT32 contentSize;
    VOS_UINT8 *iapContent;
} SI_IAP_Content;

/* 复合电话本请求信息存储 */
typedef struct {
    VOS_UINT16 anrExtIndex[SI_PB_ANR_MAX + 1];
    VOS_UINT8 anrContent[SI_PB_ANR_MAX][SI_PB_ANR_LEN + 3]; /* ANR文件最长为17，不超过20 */
    VOS_UINT8 anrExtContent[SI_PB_ANR_MAX][SI_PB_EXT_LEN + 3];
    VOS_UINT8 emlContent[SI_PB_EMAIL_MAX_LEN];
    VOS_UINT8 iapContent[20];
} SI_SPB_ReqUint;

/* 外部请求信息存储 */
typedef struct {
    SI_PB_CnfUint32 pbEventType;
    SI_PB_StorateTypeUint32 pbStoateType;
    SI_PB_LockUint32 pbLock;
    SI_SPB_ReqUint spbReq;
    VOS_UINT16 clientID;
    VOS_UINT8 opId;
    VOS_UINT8 equalFlag;
    VOS_UINT16 index1;
    VOS_UINT16 index2;
    VOS_UINT16 curIndex;
    VOS_UINT16 searchLen;
    VOS_UINT16 extIndex;
    VOS_UINT8 xdnContent[100];
    VOS_UINT8 extContent[SI_PB_EXT_LEN + 1];
} SI_PB_ReqUint;

/* ADN、FDN、BDN或MSISDN信息存储 */
typedef struct {
    VOS_UINT16 fileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 sfi;
    VOS_UINT8 rsv[3];
} SI_PB_XdnInfo;

/* ANR信息存储 */
typedef struct {
    SI_PB_FileTypeUint32 anrType;
    VOS_UINT32 anrTagNum; /* ANR的Tag在A9Tag中的位置，根据此位置，
                            结合IAP文件确定ANR和ADN记录对应关系 */
    VOS_UINT16 anrFileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
} SI_PB_AnrInfo;

/* EMAIL信息存储 */
typedef struct {
    SI_PB_FileTypeUint32 emlType;
    VOS_UINT32 emlTagNum; /* Email的Tag在A9Tag中的位置，根据此位置，
                            结合IAP文件确定Email和ADN记录对应关系 */
    VOS_UINT16 emlFileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
} SI_PB_EmlInfo;

/* IAP信息存储，见31.102第94页 */
typedef struct {
    VOS_UINT16 iapFileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
} SI_PB_IapInfo;

/* UID信息存储 */
typedef struct {
    VOS_UINT16 fileId;
    VOS_UINT16 usRsv;
} SI_PB_UidInfo;

/* PBC信息存储，用于3G重同步 */
typedef struct {
    VOS_UINT16 fileId;
    VOS_UINT16 usRsv;
} SI_PB_PbcInfo;

typedef struct {
    VOS_UINT8 entryChangeNum; /* entry of change in 2G Terminal */
    VOS_UINT8 pbCUpdateFlag;
    VOS_UINT8 record[258];
} SI_PB_PbcUpdate;

/* 电话本SEARCH控制信息 */
typedef struct {
    VOS_UINT16 readOffset; /* 以SEARCH方式得到的要初始化记录的偏移量 */
    VOS_UINT16 readNum;    /* 以SEARCH方式得到的要初始的记录数 */
    VOS_UINT16 efId;       /* 以SEARCH方式处理的当前文件ID */
    VOS_UINT16 usRsv;
    VOS_UINT8 readString[SI_PB_READ_STR_LEN]; /* 以SEARCH方式得到的要初始化的记录 */
} SI_PB_SearchCtrl;

/* 电话本控制信息 */
typedef struct {
    VOS_UINT32 adnFileNum;
    VOS_UINT32 anrFileNum;
    VOS_UINT32 emlFileNum;
    VOS_UINT32 iapFileNum;
    VOS_UINT32 anrStorageNum;
    SI_PB_XdnInfo adnInfo[SI_PB_ADNMAX];
    SI_PB_AnrInfo anrInfo[SI_PB_ADNMAX][SI_PB_ANRMAX];
    SI_PB_EmlInfo emlInfo[SI_PB_ADNMAX];
    SI_PB_IapInfo iapInfo[SI_PB_ADNMAX];
    SI_PB_UidInfo uidInfo[SI_PB_ADNMAX];
    SI_PB_PbcInfo pbCInfo[SI_PB_ADNMAX];
    VOS_UINT32 pSCValue;
    VOS_UINT32 totalUsed; /* 复合电话本使用的总记录数 */
    SI_PB_StorateTypeUint32 pbCurType;
    VOS_UINT16 cCValue;
    VOS_UINT16 pUIDValue;
    VOS_UINT16 uIDMaxValue;
    VOS_UINT16 ext1FileId;
    VOS_UINT16 pbCRecordNum;
    VOS_UINT8 rcv[2];
} SI_PB_Control;

typedef struct {
    SI_PB_InitStepUint32 pbInitStep;
    SI_PB_SendReqStatusUint32 reqStatus;
    SI_PB_InitStatusUnit32 pbInitState;
    SI_PB_SearchUint32 pbSearchState;
    VOS_UINT16 fileId[SI_PB_MAX_XDN];
    SI_PB_PbcUpdate pbCUpdate;
    USIMM_CardTypeUint32 cardType;
    USIMM_CardAppServicUint32 cardService;
} SI_PB_InitState;

typedef struct {
    VOS_UINT16 adnFileId;
    VOS_UINT16 extFileId;
    VOS_UINT16 emlFileId;
    VOS_UINT16 anrFileId[SI_PB_ANR_MAX];
    VOS_UINT8 recordNum;
    VOS_UINT8 rsv[3];
} SI_PB_SpbUpdate;

typedef VOS_UINT32 (*PPBPROC)(SI_PIH_CardSlotUint32 slotId, PBMsgBlock *msg);
typedef VOS_VOID (*PPBINITPROC)(SI_PIH_CardSlotUint32 slotId, const PBMsgBlock *msg);
typedef VOS_UINT32 (*PPBREQ)(SI_PIH_CardSlotUint32 slotId);

typedef struct {
    VOS_UINT32 msgType; /* 处理消息类型 */
    PPBPROC procFun;    /* 对应处理函数 */
} SI_PB_ProcList;

typedef struct {
    VOS_UINT32 msgType;      /* 处理消息类型 */
    PPBREQ reqFun;           /* 消息分发函数 */
    PPBINITPROC initProcFun; /* 对应处理函数 */
} SI_PB_InitList;

/* Ccpu发送的同步全局变量的消息 */
typedef struct {
    VOS_UINT32 indexAddr;
    VOS_UINT32 contentAddr;
} SI_PB_ContentPtr;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    SI_PB_Control pbCtrlInfo;
    SI_PB_NVCtrlInfo pbConfigInfo;
    SI_PB_Content pbContent[SI_PB_MAX_NUMBER];
    SI_EXT_Content extContent[SI_PB_MAX_NUMBER];
    SI_ANR_Content anrContent[SI_PB_ANRMAX];
    SI_EML_Content emlContent;
    SI_IAP_Content iapContent;
    SI_PB_InitState pbInitState;

    SI_PB_ContentPtr pbContentAddr[SI_PB_MAX_NUMBER];
    VOS_UINT32 extContentAddr[SI_PB_MAX_NUMBER];
    VOS_UINT32 anrContentAddr[SI_PB_ANRMAX];
    VOS_UINT32 emlContentAddr;
    VOS_UINT32 iapContentAddr;
} SI_PB_UpdateglobalInd;

/* Ccpu发送的当前电话本设置的消息 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    SI_PB_StorateTypeUint32 pbCurType;
} SI_PB_SetpbInd;

typedef struct {
    VOS_UINT16 efId;     /* 文件ID */
    VOS_UINT8 recordNum; /* 文件记录号，二进制文件填0 */
    VOS_UINT8 rsv;       /* 保留 */
} SI_PB_GetfileInfo;

typedef struct {
    VOS_UINT16 efId;      /* 文件ID */
    VOS_UINT8 recordNum;  /* 文件记录号，二进制文件填0 */
    VOS_UINT8 rsv;        /* 保留 */
    VOS_UINT32 efLen;     /* 更新数据长度 */
    VOS_UINT8 *efContent; /* 更新数据内容 */
} SI_PB_SetfileInfo;

typedef struct {
    VOS_UINT16 fileId[14];
    VOS_UINT16 dataLen[14];
    VOS_UINT8 recordNum[14];
    VOS_UINT16 fileNum;
    VOS_UINT8 *content[14];
} SI_PB_Spbdata;

#else /* (FEATURE_PHONE_USIM == FEATURE_ON) */

#define SI_PB_XDN_SPACE (60000)
#define SI_PB_XDN_SUPPORT_NUM (1000)

#define SI_PB_ADNMAX 10 /* 最多支持ADN电话本个数 */
#define SI_PB_ANRMAX 3
#define SI_PB_MAX_NUMBER 7 /* 最多支持电话本个数 */
#define SI_PB_MAX_XDN 16   /* 初始化时支持的电话本总数 */
#define SI_PB_ANR_MAX 3    /* ANR文件的个数 */

#define SI_PB_FILE_NOT_EXIST 0
#define SI_PB_FILE_EXIST 1

#define SI_PB_EXCEPT_NAME 14
#define SI_PB_ECC_EXCEPT_NAME 4
#define SI_PB_NUM_LEN 20
#define SI_PB_ECC_LEN 3

#define SI_PB_ANR_LEN 17
#define SI_PB_EXT_LEN 13
#define SI_PB_EXT_NUM_LEN 20
#define SI_PB_NUM_WITHEXT (SI_PB_NUM_LEN + SI_PB_EXT_NUM_LEN)
#define SI_PB_ADDITIONAL_NUM 2
#define SI_PB_SEARCH_MAX_LEN 243
#define SI_PB_READ_STR_LEN 256

#define SI_PB_TAGNOTFOUND 0xFFFFFFFF

#define SI_CLEARALL 0x01
#define SI_CLEARXDN 0x02
#define SI_CLEARSPB 0x03

#define EFADNDO_TAG 0xC0
#define EFIAPDO_TAG 0xC1
#define EFEXT1DO_TAG 0xC2
#define EFSNEDO_TAG 0xC3
#define EFANRDO_TAG 0xC4
#define EFPBCDO_TAG 0xC5
#define EFGRPDO_TAG 0xC6
#define EFAASDO_TAG 0xC7
#define EFGASDO_TAG 0xC8
#define EFUIDDO_TAG 0xC9
#define EFEMAILDO_TAG 0xCA
#define EFCCP1DO_TAG 0xCB

#define EFPBR 0x4F30
#define SIM_EFADN 0x6F3A
#define EFSDN 0x6F49

#define EFEXT1 0x6F4A      /* 用于ADN文件 */
#define EFEXT2 0x6F4B      /* 用于FDN文件 */
#define EFEXT3 0x6F4C      /* 用于SDN文件 */
#define EFEXT4_USIM 0x6F55 /* 用于BDN文件 */
#define EFEXT4_SIM 0x6F4E  /* 用于BDN文件 */
#define EFEXT5 0x6F4E      /* 仅存在于USIM卡中的MSISDN文件 */
#define EFEXT6 0x6FC8      /* 用于MBDN文件 */
#define EFEXT7 0x6FCC      /* 用于CFIS文件 */

#define INVALIDE_FILEID 0xFFFF

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
#define PB_GEN_LOG_MODULE(Level) (MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_COMM, Level))

#define PB_INFO_LOG(string)                                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_NORMAL_LOG(string)                                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_WARNING_LOG(string)                                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_ERROR_LOG(string) (VOS_VOID)               \
        mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, string)

#define PB_INFO1_LOG(string, para1)                                                                             \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)
#define PB_NORMAL1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)
#define PB_WARNING1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)
#define PB_ERROR1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), ACPU_PID_PB, VOS_NULL_PTR, __LINE__, \
        string "%d",  para1)
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#define PB_GEN_LOG_MODULE(Level) (MDRV_DIAG_GEN_LOG_MODULE(PS_GetModemIdFromPid(MAPS_PB_PID), DIAG_MODE_COMM, Level))

#define PB_INFO_LOG(string)                                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, string)

#define PB_NORMAL_LOG(string)                                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, string)

#define PB_WARNING_LOG(string)                                                                                         \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, string)

#define PB_ERROR_LOG(string) (VOS_VOID)               \
        mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, string)

#define PB_INFO1_LOG(string, para1)                                                                             \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, \
        string "%d",  para1)
#define PB_NORMAL1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_NORMAL), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)
#define PB_WARNING1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_WARNING), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)
#define PB_ERROR1_LOG(string, para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(PB_GEN_LOG_MODULE(PS_LOG_LEVEL_ERROR), MAPS_PB_PID, VOS_NULL_PTR, __LINE__, \
        string "%d", para1)
#endif

#if (VOS_WIN32 == VOS_OS_VER)
#define PB_MALLOC(LENGTH) VOS_MemAlloc(MAPS_PB_PID, DYNAMIC_MEM_PT, LENGTH)
#define PB_FREE(MEMADDR) VOS_MemFree(MAPS_PB_PID, MEMADDR)
#elif (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#define PB_MALLOC(LENGTH) VOS_CacheMemAllocDebug((VOS_UINT32)LENGTH, (VOS_UINT32)SI_PB_MEM_ALLOC)
#define PB_FREE(MEMADDR) VOS_CacheMemFree(MEMADDR)
#else
#define PB_MALLOC(LENGTH) mdrv_smalloc((VOS_UINT32)LENGTH, MEM_ICC_DDR_POOL)
#define PB_FREE(MEMADDR) mdrv_sfree(MEMADDR)
#endif

enum SI_PB_InitStep {
    PB_INIT_PB_STATUS = 0, /* 初始化各个电话本的激活状态 */
    PB_INIT_EFPBR,         /* 初始化PBR内容 */
    PB_INIT_XDN_SPACE,     /* 初始化XDN空间 */
    PB_INIT_ANR_SPACE,     /* 初始化ANR空间 */
    PB_INIT_EML_SPACE,     /* 初始化email空间 */
    PB_INIT_IAP_SPACE,     /* 初始化IAP空间 */
    PB_INIT_EXT_SPACE,     /* 初始化EXT空间 */
    PB_INIT_IAP_CONTENT,   /* 初始化IAP中Email记录和AND记录的对应关系 */
    PB_INIT_EML_CONTENT,   /* 初始化Email内容 */
    PB_INIT_ANR_CONTENT,   /* 初始化ANR内容 */
    PB_INIT_EXTR_CONTENT,  /* 初始化ANR EXT内容 */
    PB_INIT_XDN_CONTENT,   /* 初始化XDN内容 */
    PB_INIT_EXT_CONTENT,   /* 初始化EXT内容 */
    PB_INIT_PBC_CONTENT,   /* 初始化PBC内容 */
    PB_INIT_IAP_SEARCH,    /* 查找IAP记录 */
    PB_INIT_IAP_CONTENT2,  /* 根据查找的结果来初始化IAP文件的内容 */
    PB_INIT_EML_SEARCH,    /* 查找EMAIL记录 */
    PB_INIT_EML_CONTENT2,  /* 根据查找的结果来初始化EMAIL文件的内容 */
    PB_INIT_ANR_SEARCH,    /* 查找ANR记录 */
    PB_INIT_ANR_CONTENT2,  /* 根据查找的结果来初始化ANR文件的内容 */
    PB_INIT_XDN_SEARCH,    /* 查找XDN记录 */
    PB_INIT_XDN_CONTENT2,  /* 根据查找的结果来初始化XDN文件的内容 */
    PB_INIT_PBC_SEARCH,    /* 查找PBC记录 */
    PB_INIT_PBC_CONTENT2,  /* 根据查找的结果来初始化PBC文件的内容 */
    PB_INIT_SYNCH,         /* 3G电话本同步 */
    PB_INIT_INFO_APP,      /* 回调上报 */
    PB_INIT_FINISHED,      /* 初始化结束 */
    PB_INIT_BUTT
};
typedef VOS_UINT32 SI_PB_InitStepUint32;

enum SI_PB_Search {
    PB_SEARCH_DISABLE = 0,
    PB_SEARCH_ENABLE = 1,
    PB_SEARCH_BUTT
};
typedef VOS_UINT32 SI_PB_SearchUint32;

enum SI_PB_Lock {
    PB_UNLOCK = 0,
    PB_LOCKED = 1,
    PB_LOCK_BUTT
};
typedef VOS_UINT32 SI_PB_LockUint32;

enum SI_PB_FileType {
    PB_FILE_UNKNOWN = 0,
    PB_FILE_TYPE1 = 0xA8,
    PB_FILE_TYPE2 = 0xA9,
    PB_FILE_TYPE3 = 0xAA,
    PB_FILE_BUTT
};
typedef VOS_UINT32 SI_PB_FileTypeUint32;

/* 这里的定义要与SI_PB_STORAGE_SM待的定义保持一致 */
enum SI_PB_Type {
    PB_ADN = 1,
    PB_MISDN = 4,
    PB_FDN = 5,
    PB_ECC = 6,
    PB_BDN = 7,
    PB_SDN = 8,
    PB_XECC = 9,
    PB_NULL = 0xFF,
    PB_TYPE_BUTT
};
typedef VOS_UINT32 SI_PB_TypeUint32;

enum SI_PB_ContentType {
    PB_ECC_CONTENT = 0,
    PB_ADN_CONTENT = 1,
    PB_FDN_CONTENT = 2,
    PB_BDN_CONTENT = 3,
    PB_MSISDN_CONTENT = 4,
    PB_SDN_CONTENT = 5,
    PB_XECC_CONTENT = 6,
    PB_CONTENT_BUTT
};
typedef VOS_UINT32 SI_PB_ContentTypeUint32;

enum SI_PB_SendReqStatus {
    PB_REQ_SEND = 0,
    PB_REQ_NOT_SEND = 1,
    PB_SEND_REQ_BUTT
};
typedef VOS_UINT32 SI_PB_SendReqStatusUint32;

enum SI_PB_InitStateType {
    PB_NOT_INITIALISED = 0, /* 未初始化完成 */
    PB_INITIALISED = 1,     /* 已初始化完成 */
    PB_FILE_NOT_EXIST = 2,  /* 电话本文件不存在 */
    PB_INIT_STATE_BUTT
};
typedef VOS_UINT8 SI_PB_InitStateUint8;

enum SI_PB_ActiveState {
    SI_PB_DEACTIVE = USIMM_EFSTATUS_DEACTIVE,
    SI_PB_ACTIVE = USIMM_EFSTATUS_ACTIVE,
    SI_PB_ACTIVE_STATE_BUTT
};
typedef VOS_UINT32 SI_PB_ActiveStateUint32;

enum SI_PB_Refresh {
    SI_PB_REFRESH_ALL = 0,
    SI_PB_REFRESH_FILE = 1,
    SI_PB_REFRESH_BUTT
};
typedef VOS_UINT32 SI_PB_RefreshUnit32;

enum SI_PB_InitStatus {
    SI_PB_INIT_STATUS_OK = 0,
    SI_PB_INIT_PBMALLOC_FAIL = 1,
    SI_PB_INIT_SPBMALLOC_FAIL = 2,
    SI_PB_INIT_PBTYPE_UNKOWN = 3,
    SI_PB_INIT_USIMPBCNF_ERR = 4,
    SI_PB_INIT_USIMSPBCNF_ERR = 5,
    SI_PB_INIT_RECORD_REMAIN = 6,
    SI_PB_INIT_EXT_RECORD = 7,
    SI_PB_INIT_EXT_ERR = 8,
    SI_PB_INIT_RECORD_EXCEED = 9,
    SI_PB_INIT_FILE_DECATIVE = 10,
    SI_PB_INIT_SEARCH_OK = 11,
    SI_PB_INIT_SEARCH_ERR = 12,
    SI_PB_INIT_MSGTYPE_ERR = 13,
    SI_PB_INIT_ERR_BUTT
};
typedef VOS_UINT32 SI_PB_InitStatusUnit32;

/* 读取号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    VOS_UINT16 index1;
    VOS_UINT16 index2;
} SI_PB_ReadReq;

/* 查询号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
} SI_PB_QueryReq;

/* 设置号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名     */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
} SI_PB_SetReq;

/* 添加号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    SI_PB_Record record;
} SI_PB_AddRep;

/* 更改号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名   */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    SI_PB_Record record;
} SI_PB_ModifyRep;

/* 删除号码请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    VOS_UINT16 index;
    VOS_UINT16 usReserved;
} SI_PB_DeleteReq;

/* 更新号码文件请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName;    /* 消息名 */
    VOS_UINT32 fileNumber; /* 更新文件个数 */
    SI_PB_RefreshUnit32 refreshType;
    VOS_UINT16 file[SI_PB_MAX_XDN];
} SI_PB_RefreshReq;

/* 查找姓名请求消息结构 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    VOS_UINT16 client;  /* 客户端ID */
    VOS_UINT8 opId;
    VOS_UINT8 rsv;
    SI_PB_StorateTypeUint32 storage;
    VOS_UINT8 length;                        /* 要匹配字段的长度 */
    VOS_UINT8 content[SI_PB_SEARCH_MAX_LEN]; /* 要匹配的字段内容 */
} SI_PB_SearchReq;

/* EXT控制信息 */
typedef struct {
    VOS_UINT16 extFlag;
    VOS_UINT16 extFileId;
    VOS_UINT16 extTotalNum;
    VOS_UINT16 extUsedNum;
    VOS_UINT32 contentSize;
    VOS_UINT8 *extContent;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) && (FEATURE_ON == FEATURE_MBB_ACORE_64BIT_ENABLE))
    VOS_UINT32 rec64BitHighAddr;
#endif
} SI_EXT_Content;

/* 电话本信息 */
typedef struct {
    SI_PB_TypeUint32 pbType;
    SI_PB_ActiveStateUint32 activeStatus;
    VOS_UINT16 totalNum;
    VOS_UINT16 usedNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 nameLen;
    VOS_UINT8 numberLen;
    SI_PB_InitStateUint8 initialState;
    VOS_UINT32 extInfoNum; /* 对应的EXT文件控制信息编号 */
    VOS_UINT32 indexSize;
    VOS_UINT8 *index;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) && (FEATURE_ON == FEATURE_MBB_ACORE_64BIT_ENABLE))
    VOS_UINT32 rec64BitHighAddr1;
#endif
    VOS_UINT32 contentSize;
    VOS_UINT8 *content;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) && (FEATURE_ON == FEATURE_MBB_ACORE_64BIT_ENABLE))
    VOS_UINT32 rec64BitHighAddr2;
#endif
} SI_PB_Content;

/* 补充电话本信息 */
typedef struct {
    SI_PB_ActiveStateUint32 activeStatus;
    VOS_UINT16 totalNum;
    VOS_UINT16 usedNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 numberLen;
    SI_PB_InitStateUint8 initialState;
    VOS_UINT8 rsv;
    VOS_UINT32 contentSize;
    VOS_UINT8 *content;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) && (FEATURE_ON == FEATURE_MBB_ACORE_64BIT_ENABLE))
    VOS_UINT32 rec64BitHighAddr;
#endif
} SI_ANR_Content;

/* Email电话本信息 */
typedef struct {
    SI_PB_ActiveStateUint32 activeStatus;
    VOS_UINT16 totalNum;
    VOS_UINT16 usedNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 dataLen;
    SI_PB_InitStateUint8 initialState;
    VOS_UINT8 rsv;
    VOS_UINT32 contentSize;
    VOS_UINT8 *content;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) && (FEATURE_ON == FEATURE_MBB_ACORE_64BIT_ENABLE))
    VOS_UINT32 rec64BitHighAddr;
#endif
} SI_EML_Content;

/* IAP文件信息 */
typedef struct {
    VOS_UINT16 totalNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 rsv;
    VOS_UINT32 contentSize;
    VOS_UINT8 *iapContent;
#if ((OSA_CPU_CCPU == VOS_OSA_CPU) && (FEATURE_ON == FEATURE_MBB_ACORE_64BIT_ENABLE))
    VOS_UINT32 rec64BitHighAddr;
#endif
} SI_IAP_Content;

/* 复合电话本请求信息存储 */
typedef struct {
    VOS_UINT16 anrExtIndex[SI_PB_ANR_MAX + 1];
    VOS_UINT8 anrContent[SI_PB_ANR_MAX][SI_PB_ANR_LEN + 3]; /* ANR文件最长为17，不超过20 */
    VOS_UINT8 anrExtContent[SI_PB_ANR_MAX][SI_PB_EXT_LEN + 3];
    VOS_UINT8 emlContent[SI_PB_EMAIL_MAX_LEN];
    VOS_UINT8 iapContent[20];
} SI_SPB_ReqUint;

/* 外部请求信息存储 */
typedef struct {
    SI_PB_CnfUint32 pbEventType;
    SI_PB_StorateTypeUint32 pbStoateType;
    SI_PB_LockUint32 pbLock;
    SI_SPB_ReqUint spbReq;
    VOS_UINT16 clientId;
    VOS_UINT8 opId;
    VOS_UINT8 equalFlag;
    VOS_UINT16 index1;
    VOS_UINT16 index2;
    VOS_UINT16 curIndex;
    VOS_UINT16 searchLen;
    VOS_UINT16 extIndex;
    VOS_UINT8 xdnContent[100];
    VOS_UINT8 extContent[SI_PB_EXT_LEN + 1];
} SI_PB_ReqUint;

/* ADN、FDN、BDN或MSISDN信息存储 */
typedef struct {
    VOS_UINT16 fileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
    VOS_UINT8 sfi;
    VOS_UINT8 rsv[3];
} SI_PB_XdnInfo;

/* ANR信息存储 */
typedef struct {
    SI_PB_FileTypeUint32 anrType;
    VOS_UINT32 anrTagNum; /* ANR的Tag在A9Tag中的位置，根据此位置，
                            结合IAP文件确定ANR和ADN记录对应关系 */
    VOS_UINT16 anrFileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
} SI_PB_AnrInfo;

/* EMAIL信息存储 */
typedef struct {
    SI_PB_FileTypeUint32 emlType;
    VOS_UINT32 emlTagNum; /* Email的Tag在A9Tag中的位置，根据此位置，
                            结合IAP文件确定Email和ADN记录对应关系 */
    VOS_UINT16 emlFileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
} SI_PB_EmlInfo;

/* IAP信息存储，见31.102第94页 */
typedef struct {
    VOS_UINT16 iapFileId;
    VOS_UINT8 recordNum;
    VOS_UINT8 recordLen;
} SI_PB_IapInfo;

/* UID信息存储 */
typedef struct {
    VOS_UINT16 fileId;
    VOS_UINT16 usRsv;
} SI_PB_UidInfo;

/* PBC信息存储，用于3G重同步 */
typedef struct {
    VOS_UINT16 fileId;
    VOS_UINT16 usRsv;
} SI_PB_PbcInfo;

typedef struct {
    VOS_UINT8 entryChangeNum; /* entry of change in 2G Terminal */
    VOS_UINT8 pbCUpdateFlag;
    VOS_UINT8 record[258];
} SI_PB_PbcUpdate;

/* 电话本SEARCH控制信息 */
typedef struct {
    VOS_UINT16 readOffset; /* 以SEARCH方式得到的要初始化记录的偏移量 */
    VOS_UINT16 readNum;    /* 以SEARCH方式得到的要初始的记录数 */
    VOS_UINT16 efId;       /* 以SEARCH方式处理的当前文件ID */
    VOS_UINT16 usRsv;
    VOS_UINT8 readString[SI_PB_READ_STR_LEN]; /* 以SEARCH方式得到的要初始化的记录 */
} SI_PB_SearchCtrl;

/* 电话本控制信息 */
typedef struct {
    VOS_UINT32 adnFileNum;
    VOS_UINT32 anrFileNum;
    VOS_UINT32 emlFileNum;
    VOS_UINT32 iapFileNum;
    VOS_UINT32 anrStorageNum;
    SI_PB_XdnInfo adnInfo[SI_PB_ADNMAX];
    SI_PB_AnrInfo anrInfo[SI_PB_ADNMAX][SI_PB_ANRMAX];
    SI_PB_EmlInfo emlInfo[SI_PB_ADNMAX];
    SI_PB_IapInfo iapInfo[SI_PB_ADNMAX];
    SI_PB_UidInfo uidInfo[SI_PB_ADNMAX];
    SI_PB_PbcInfo pbCInfo[SI_PB_ADNMAX];
    VOS_UINT32 pSCValue;
    VOS_UINT32 totalUsed; /* 复合电话本使用的总记录数 */
    SI_PB_StorateTypeUint32 pbCurType;
    VOS_UINT16 cCValue;
    VOS_UINT16 pUIDValue;
    VOS_UINT16 uIDMaxValue;
    VOS_UINT16 ext1FileId;
    VOS_UINT16 pbCRecordNum;
    VOS_UINT8 rsv[2];
} SI_PB_Control;

typedef struct {
    SI_PB_InitStepUint32 pbInitStep;
    SI_PB_SendReqStatusUint32 reqStatus;
    SI_PB_InitStatusUnit32 pbInitState;
    SI_PB_SearchUint32 pbSearchState;
    VOS_UINT16 fileId[SI_PB_MAX_XDN];
    SI_PB_PbcUpdate pbCUpdate;
    USIMM_CardTypeUint32 cardType;
    USIMM_CardAppServicUint32 cardService;
} SI_PB_InitState;

typedef struct {
    VOS_UINT16 adnFileId;
    VOS_UINT16 extFileId;
    VOS_UINT16 emlFileId;
    VOS_UINT16 anrFileId[SI_PB_ANR_MAX];
    VOS_UINT8 recordNum;
    VOS_UINT8 rsv[3];
} SI_PB_SpbUpdate;

typedef VOS_UINT32 (*PPBPROC)(PBMsgBlock *msg);
typedef VOS_VOID (*PPBINITPROC)(const PBMsgBlock *msg);
typedef VOS_UINT32 (*PPBREQ)(VOS_VOID);

typedef struct {
    VOS_UINT32 msgType; /* 处理消息类型 */
    PPBPROC procFun;    /* 对应处理函数 */
} SI_PB_ProcList;

typedef struct {
    VOS_UINT32 msgType;      /* 处理消息类型 */
    PPBREQ reqFun;           /* 消息分发函数 */
    PPBINITPROC initProcFun; /* 对应处理函数 */
} SI_PB_InitList;

/* Ccpu发送的同步全局变量的消息 */
typedef struct {
    VOS_UINT32 indexAddr;
    VOS_UINT32 contentAddr;
} SI_PB_ContentPtr;

typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    SI_PB_Control pbCtrlInfo;
    SI_PB_NVCtrlInfo pbConfigInfo;
    SI_PB_Content pbContent[SI_PB_MAX_NUMBER];
    SI_EXT_Content extContent[SI_PB_MAX_NUMBER];
    SI_ANR_Content anrContent[SI_PB_ANRMAX];
    SI_EML_Content emlContent;
    SI_IAP_Content iapContent;
    SI_PB_InitState pbInitState;

    SI_PB_ContentPtr pbContentAddr[SI_PB_MAX_NUMBER];
    VOS_UINT32 extContentAddr[SI_PB_MAX_NUMBER];
    VOS_UINT32 anrContentAddr[SI_PB_ANRMAX];
    VOS_UINT32 emlContentAddr;
    VOS_UINT32 iapContentAddr;
} SI_PB_UpdateglobalInd;

/* Ccpu发送的当前电话本设置的消息 */
typedef struct {
    VOS_MSG_HEADER
    VOS_UINT32 msgName; /* 消息名 */
    SI_PB_StorateTypeUint32 pbCurType;
} SI_PB_SetpbInd;

typedef struct {
    VOS_UINT16 efId;     /* 文件ID */
    VOS_UINT8 recordNum; /* 文件记录号，二进制文件填0 */
    VOS_UINT8 rsv;       /* 保留 */
} SI_PB_GetfileInfo;

typedef struct {
    VOS_UINT16 efId;      /* 文件ID */
    VOS_UINT8 recordNum;  /* 文件记录号，二进制文件填0 */
    VOS_UINT8 rsv;        /* 保留 */
    VOS_UINT32 efLen;     /* 更新数据长度 */
    VOS_UINT8 *efContent; /* 更新数据内容 */
} SI_PB_SetfileInfo;

typedef struct {
    VOS_UINT16 fileId[14];
    VOS_UINT16 dataLen[14];
    VOS_UINT8 recordNum[14];
    VOS_UINT16 fileNum;
    VOS_UINT8 *content[14];
} SI_PB_Spbdata;

typedef struct {
    VOS_UINT32 index;    /* ANR文件索引号 */
    VOS_UINT8 extRecord; /* ANR的EXT文件记录号 */
    VOS_UINT8 reserve[3];
} AnrUpdateProcPara;

extern SI_PB_Content g_pbContent[SI_PB_MAX_NUMBER];

extern SI_EXT_Content g_extContent[SI_PB_MAX_NUMBER];

extern SI_ANR_Content g_anrContent[SI_PB_ANRMAX];

extern SI_EML_Content g_emlContent;

extern SI_IAP_Content g_iapContent;

extern SI_PB_Control g_pbCtrlInfo;

extern SI_PB_NVCtrlInfo g_pbConfigInfo;

extern SI_PB_InitState g_pbInitState;

extern SI_PB_ReqUint g_pbReqUnit;

extern SI_PB_SearchCtrl g_pbSearchCtrlInfo;

extern VOS_UINT32 g_pbFileCnt;

extern VOS_UINT32 g_pbRecordCnt;

extern VOS_UINT32 g_pbInitFileNum;

extern VOS_UINT32 g_pbInitExtFileNum;

extern VOS_UINT32 g_pbExtRecord;

extern VOS_UINT8 g_pbCStatus;

#if ((OSA_CPU_ACPU == VOS_OSA_CPU) || (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON))
extern VOS_VOID SI_PB_BcdToAscii(VOS_UINT8 bcdNumLen, const VOS_UINT8 *bcdNum, VOS_UINT32 asciiBuffSize,
    VOS_UINT8 *asciiNum, VOS_UINT8 *len);

extern VOS_VOID SI_PB_DecodePBName(VOS_UINT8 nameMax, const VOS_UINT8 *name, VOS_UINT32 *alphaType, VOS_UINT8 *nameLen);

extern VOS_VOID SI_PB_TransPBFromate(const SI_PB_Content *pbContent, VOS_UINT16 index, const VOS_UINT8 *content,
    SI_PB_Record *record);

extern VOS_UINT32 SI_PB_GetFileCntFromIndex(VOS_UINT16 index, VOS_UINT8 *eMLFileCnt);

extern VOS_VOID SI_PB_TransEMLFromate(VOS_UINT8 emailMaxLen, const VOS_UINT8 *emlContent, SI_PB_Record *record);

extern VOS_VOID SI_PB_TransANRFromate(VOS_UINT8 aNROffset, const VOS_UINT8 *anrContent, VOS_UINT32 anrContentLen,
    SI_PB_Record *record);
#endif

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
extern VOS_BOOL SI_PB_CheckSupportAP(VOS_VOID);

extern VOS_VOID SI_PB_DecodeEFPBCRecord(VOS_UINT8 content, VOS_UINT8 recordNum);

extern VOS_VOID SI_PB_DecodeEFUIDRecord(VOS_UINT8 value1, VOS_UINT8 value2);

extern VOS_UINT32 SI_PB_GetXDNPBType(VOS_UINT32 *pbType, VOS_UINT16 fileId);

extern VOS_VOID SI_PB_InitStatusInd(USIMM_PbInitStatusUint16 pbInitStatus);

extern VOS_UINT32 SI_PB_GetANRFid(VOS_UINT32 anrFileNum, VOS_UINT16 *anrFileId);

extern VOS_UINT16 SI_PB_TransferFileCnt2ExtFileID(VOS_UINT32 pBFileCnt, VOS_UINT32 *pbType);

extern VOS_UINT32 SI_PB_GetANRSuffix(VOS_UINT8 *xSuffix, VOS_UINT8 *ySuffix, VOS_UINT16 anrFileId);

extern VOS_VOID SI_PB_IncreaceCCValue(VOS_UINT16 incValue, VOS_UINT8 adnIndex);

extern VOS_VOID SI_PBSendGlobalToAcpu(VOS_VOID);

extern VOS_VOID SI_PB_EcallInitIndFunc(VOS_VOID);

extern VOS_VOID SI_PB_InitGlobeVariable(VOS_VOID);

extern VOS_UINT32 SI_PB_DecodeEFPBR(VOS_UINT8 recordNum, VOS_UINT8 recordLen, const VOS_UINT8 *content);

extern VOS_VOID SI_PB_ReleaseAll(VOS_VOID);

extern VOS_UINT32 SI_PB_GheckANRLast(VOS_UINT8 xSuffix, VOS_UINT8 ySuffix);

extern VOS_UINT32 SI_PB_CountXDNIndex(VOS_UINT32 pbType, VOS_UINT16 fileId, VOS_UINT8 recordNum, VOS_UINT16 *index);

extern VOS_VOID SI_PB_SetBitToBuf(VOS_UINT8 *dataBuf, VOS_UINT32 dataBufSize, VOS_UINT32 bitNo, VOS_UINT32 value);

extern VOS_VOID SI_PB_SearchResultProc(SI_PB_SearchCtrl *searchCtrlInfo, const VOS_UINT8 *searchResult,
    VOS_UINT16 matchNum, VOS_UINT8 totalNum);

extern VOS_UINT32 SI_PB_EMLContentProc(const USIMM_ReadFileCnf *pbMsg, VOS_UINT8 suffix, VOS_UINT16 emailIndex);

extern VOS_UINT32 SI_PB_CheckANRValidity(VOS_UINT8 value1, VOS_UINT8 value2);

extern VOS_UINT32 SI_PB_InitANRType2ValidJudge(VOS_UINT8 xSuffix, VOS_UINT8 ySuffix, VOS_UINT8 recordNum,
    VOS_UINT16 *realIndex);

extern VOS_VOID SI_PB_AsciiToBcd(const VOS_UINT8 *asciiNum, VOS_UINT8 asciiNumLen, VOS_UINT8 *bcdNum,
    VOS_UINT32 bcdBuffSize, VOS_UINT8 *bcdNumLen);

extern VOS_UINT32 SI_PB_FindUnusedExtRecord(const SI_EXT_Content *pbContent, VOS_UINT8 *record, VOS_UINT8 recordCount);

extern VOS_UINT32 SI_PB_SetFileReq(const SI_PB_SetfileInfo *setFileInfo);

extern VOS_UINT32 SI_PB_GetFreeANRRecordNum(VOS_UINT16 aNRFid, VOS_UINT8 *recordNum, VOS_UINT16 *iapFid);

extern VOS_UINT32 SI_PB_GetIAPFidFromANR(VOS_UINT16 anrFileId, VOS_UINT16 *iAPFileId);

extern VOS_UINT32 SI_PB_GetADNSfi(VOS_UINT8 *sfi, VOS_UINT16 fileId);

extern VOS_UINT32 SI_PB_GetFreeEMLRecordNum(VOS_UINT16 eMLFid, VOS_UINT8 *recordNum, VOS_UINT16 *iapFid);

extern VOS_UINT32 SI_PB_GetIAPFidFromEML(VOS_UINT16 emlFileId, VOS_UINT16 *iAPFileId);

extern VOS_UINT32 SI_PB_SetSPBFileReq(const SI_PB_Spbdata *spbReq);

extern VOS_UINT32 SI_PB_GetANRFidFromADN(VOS_UINT8 aNROffset, VOS_UINT16 adfFileId, VOS_UINT16 *anrFileId);

extern VOS_UINT32 SI_PB_GetEMLFIdFromADN(VOS_UINT16 *emlFileId, VOS_UINT16 aDNId);

extern VOS_UINT32 SI_PB_GetEMLRecord(VOS_UINT8 **ppEMLContent, VOS_UINT16 emlFileId, VOS_UINT8 recordNum,
    VOS_UINT32 *freeSize);

extern VOS_VOID SI_PB_ClearPBContent(VOS_UINT32 clearType);

extern VOS_VOID SI_PB_ClearSPBContent(VOS_UINT32 clearType);

extern VOS_UINT32 SI_PB_InitPBStatusJudge(VOS_VOID);

extern VOS_UINT32 SI_PB_InitEFpbrReq(VOS_VOID);

extern VOS_UINT32 SI_PB_InitXDNSpaceReq(VOS_VOID);

extern VOS_UINT32 SI_PB_CheckFdn(const VOS_UINT8 *num, VOS_UINT32 numLength);

extern VOS_UINT32 SI_PB_EcallNumberErrProc(SI_PB_StorateTypeUint32 storage, VOS_UINT8 listLen, const VOS_UINT8 *list,
    VOS_UINT8 *pbOffset);

extern VOS_VOID SI_PB_GetEcallNumber(const SI_PB_Content *xdnContent, SI_PB_EcallNum *ecallNum, VOS_UINT8 listLen,
    const VOS_UINT8 *list);

extern VOS_UINT32 SI_PB_JudgeADNFid(VOS_UINT16 fileId);

extern VOS_UINT32 SI_PB_JudgeEXTFid(VOS_UINT16 fileId, VOS_UINT16 offset);

extern VOS_UINT32 SI_PB_GetEXTContentFromReq(VOS_UINT8 recordNum, VOS_UINT8 **ppucContent);

extern VOS_UINT32 SI_PB_JudgeIAPFid(VOS_UINT16 fileId);

extern VOS_VOID SI_PB_GetEFIDFromPath(const USIMM_FilePathInfo *filePath, VOS_UINT16 *efId);

extern VOS_UINT32 SI_PB_CheckADNFileID(VOS_UINT16 fileId);

extern VOS_VOID SI_PB_TransPBFromate(const SI_PB_Content *pbContent, VOS_UINT16 pbIndex, const VOS_UINT8 *content,
    SI_PB_Record *record);

extern VOS_UINT32 SI_PB_GetFileReq(const SI_PB_GetfileInfo *getFileInfo);

extern VOS_UINT32 SI_PB_CheckSYNCHFileID(VOS_UINT16 fileId);

extern VOS_UINT32 SI_PB_CheckExtFileID(VOS_UINT16 fileId);

extern VOS_UINT32 SI_PB_InitEccProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_IsExsitExtFile(VOS_UINT8 extIndexValue, VOS_UINT32 extInfoNum);

#if (FEATURE_ON == FEATURE_UE_MODE_CDMA)
extern VOS_UINT32 SI_PB_InitXeccProc(const PBMsgBlock *msg);
#endif

extern VOS_VOID SI_PB_InitPBStatusProc(const PBMsgBlock *msg);

extern VOS_VOID SI_PB_InitEFpbrProc(const PBMsgBlock *msg);

extern VOS_VOID SI_PB_InitXDNSpaceMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitANRSpaceReq(VOS_VOID);

extern VOS_VOID SI_PB_InitANRSpaceMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEMLSpaceReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEMLSpaceMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitIAPSpaceReq(VOS_VOID);

extern VOS_VOID SI_PB_InitIAPSpaceMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEXTSpaceReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEXTSpaceMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitIAPContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitIAPContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEMLContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEMLContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitANRContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitANRContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEXTRContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEXTRContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitXDNContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitXDNContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEXTContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEXTContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitPBCContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitPBCContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitIAPSearchReq(VOS_VOID);

extern VOS_VOID SI_PB_InitIAPSearchMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitIAPContentReq2(VOS_VOID);

extern VOS_VOID SI_PB_InitIAPContentMsgProc2(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEmailSearchReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEmailSearchMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitEmailContentReq(VOS_VOID);

extern VOS_VOID SI_PB_InitEmailContentMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitANRSearchReq(VOS_VOID);

extern VOS_VOID SI_PB_InitANRSearchMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitANRContentReq2(VOS_VOID);

extern VOS_VOID SI_PB_InitANRContentMsgProc2(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitXDNSearchReq(VOS_VOID);

extern VOS_VOID SI_PB_InitXDNSearchMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitXDNContentReq2(VOS_VOID);

extern VOS_VOID SI_PB_InitXDNContentMsgProc2(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitPBCSearchReq(VOS_VOID);

extern VOS_VOID SI_PB_InitPBCSearchMsgProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitPBCContentReq2(VOS_VOID);

extern VOS_VOID SI_PB_InitPBCContentMsgProc2(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitSYNCHReq(VOS_VOID);

extern VOS_VOID SI_PB_InitSYNCHProc(const PBMsgBlock *msg);

extern VOS_UINT32 SI_PB_InitInfoApp(VOS_VOID);

extern VOS_VOID SI_PB_InitInfoAppDebug(const PBMsgBlock *msg);

extern VOS_VOID SI_PB_InitPBStateProc(SI_PB_InitStatusUnit32 pbInitState);
#endif

extern VOS_UINT32 SI_PB_LocateRecord(SI_PB_TypeUint32 pbType, VOS_UINT16 index1, VOS_UINT16 index2, VOS_UINT8 *number);

extern VOS_UINT32 SI_PB_CheckEccValidity(VOS_UINT8 content);

extern VOS_UINT32 SI_PB_CheckContentValidity(const SI_PB_Content *pbContent, const VOS_UINT8 *content,
    VOS_UINT32 contentSize);

extern VOS_UINT32 SI_PBCallback(SI_PB_EventInfo *event);

extern VOS_UINT32 SI_PBSendAtEventCnf(SI_PB_EventInfo *event);

extern VOS_UINT32 SI_PB_GetBitFromBuf(const VOS_UINT8 *dataBuf, VOS_UINT32 dataBufSize, VOS_UINT32 bitNo);

extern VOS_UINT32 SI_PB_FindPBOffset(SI_PB_TypeUint32 pbType, VOS_UINT8 *offset);

extern VOS_UINT32 SI_PB_CountADNRecordNum(VOS_UINT16 index, VOS_UINT16 *fileId, VOS_UINT8 *recordNum);

extern VOS_UINT32 SI_PB_GetXDNFileID(VOS_UINT32 storage, VOS_UINT16 *fileId);

VOS_UINT32 SI_PB_PbStatusInd(VOS_BOOL bPbReady);

VOS_UINT32 WuepsPBPidInit(enum VOS_InitPhaseDefine initPhrase);
VOS_VOID I0_SI_PB_PidMsgProc(struct MsgCB *pbMsg);

#endif /* (FEATURE_PHONE_USIM == FEATURE_ON) */

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
