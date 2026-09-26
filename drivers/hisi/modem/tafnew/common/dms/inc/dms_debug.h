/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#ifndef __DMS_DEBUG_H__
#define __DMS_DEBUG_H__

#include "vos.h"
#include "mdrv.h"

#include "ps_tag.h"
#include "ps_common_def.h"
#include "dms_mntn.h"
#if (!defined(MODEM_FUSION_VERSION)) || (defined(LLT_OS_VER))
#include "msp_diag_comm.h"
#else
#include "mdrv_diag.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* Log Print Module Define */
#ifndef THIS_MODU
#define THIS_MODU ps_nas
#endif

#define DMS_LOG_LEVEL_NONE 0x00000000
#define DMS_LOG_LEVEL_INFO 0x00000001
#define DMS_LOG_LEVEL_WARN 0x00000002
#define DMS_LOG_LEVEL_ERROR 0x00000004
#define DMS_LOG_LEVEL_TOP (DMS_LOG_LEVEL_INFO | DMS_LOG_LEVEL_WARN | DMS_LOG_LEVEL_ERROR)

/* 针对64bit处添加8字节对齐，主要针对5010平台，处理器无法no-align访问 */
#if (VOS_OS_VER != VOS_WIN32)
#pragma pack(8)
#else
#pragma pack(push, 8)
#endif


struct DMS_PortDebugCfg {
    VOS_UINT32 debugLevel;  /* DMS_PORT DEBUG级别:ERR, WARNING, INFO, DEBUG */
    VOS_UINT64 portIdMask;  /* DMS_PortId端口ID: DMS_PORT0-DMS_PORT63 */
};

#if (VOS_OS_VER != VOS_WIN32)
#pragma pack()
#else
#pragma pack(pop)
#endif

extern struct DMS_PortDebugCfg g_dmsPortDebugCfg;

VOS_BOOL DMS_TestBit(VOS_UINT64 bitMaskValue, VOS_UINT32 bitNum);
struct DMS_PortDebugCfg *DMS_PORT_GetDebugCfg(VOS_VOID);

/* kmsg打印接口 */
#ifdef _lint
#define DMS_LOGI(fmt, ...)
#define DMS_LOGW(fmt, ...)
#define DMS_LOGE(fmt, ...)

#define DMS_PORT_LOGI(indexNum, fmt, ...)
#define DMS_PORT_LOGW(indexNum, fmt, ...)
#define DMS_PORT_LOGE(indexNum, fmt, ...)
#else
#define DMS_LOGI(fmt, ...) do { \
    if (g_dmsPortDebugCfg.debugLevel & DMS_LOG_LEVEL_INFO) { \
        PS_PRINTF_INFO("[%s][LINE:%d] " fmt "\n", \
                       __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DMS_LOGW(fmt, ...) do { \
    if (g_dmsPortDebugCfg.debugLevel & DMS_LOG_LEVEL_WARN) { \
        PS_PRINTF_WARNING("[%s][LINE:%d] " fmt "\n", \
                          __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DMS_LOGE(fmt, ...) do { \
    if (g_dmsPortDebugCfg.debugLevel & DMS_LOG_LEVEL_ERROR) { \
        PS_PRINTF_ERR("[%s][LINE:%d] " fmt "\n", \
                      __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DMS_PORT_LOGI(indexNum, fmt, ...) do { \
    if ((g_dmsPortDebugCfg.debugLevel & DMS_LOG_LEVEL_INFO) && \
        DMS_TestBit(g_dmsPortDebugCfg.portIdMask, indexNum)) { \
        PS_PRINTF_INFO("[PORT:%2d][%s][LINE:%d] " fmt "\n", \
                       indexNum, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DMS_PORT_LOGW(indexNum, fmt, ...) do { \
    if ((g_dmsPortDebugCfg.debugLevel & DMS_LOG_LEVEL_WARN) && \
        DMS_TestBit(g_dmsPortDebugCfg.portIdMask, indexNum)) { \
        PS_PRINTF_WARNING("[PORT:%2d][%s][LINE:%d] " fmt "\n", \
                          indexNum, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)

#define DMS_PORT_LOGE(indexNum, fmt, ...) do { \
    if ((g_dmsPortDebugCfg.debugLevel & DMS_LOG_LEVEL_ERROR) && \
        DMS_TestBit(g_dmsPortDebugCfg.portIdMask, indexNum)) { \
        PS_PRINTF_ERR("[PORT:%2d][%s][LINE:%d] " fmt "\n", \
                       indexNum, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
    } \
} while (0)
#endif

/*hids log接口*/
#define DMS_INFO_LOG(String)                                                                             \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s \r\n", (String))
#define DMS_INFO_LOG1(String, Para1)                                                                     \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d \r\n", (String), (VOS_INT32)(Para1))
#define DMS_INFO_LOG2(String, Para1, Para2) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2))
#define DMS_INFO_LOG3(String, Para1, Para2, Para3)                                               \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2),         \
             (VOS_INT32)(Para3))
#define DMS_INFO_LOG4(String, Para1, Para2, Para3, Para4)                                        \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2),     \
             (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#define DMS_WARNING_LOG(String) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s \r\n", (String))
#define DMS_WARNING_LOG1(String, Para1) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d \r\n", (String), (VOS_INT32)(Para1))
#define DMS_WARNING_LOG2(String, Para1, Para2) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2))
#define DMS_WARNING_LOG3(String, Para1, Para2, Para3) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2),         \
             (VOS_INT32)(Para3))
#define DMS_WARNING_LOG4(String, Para1, Para2, Para3, Para4) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2),     \
             (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#define DMS_ERROR_LOG(String) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s \r\n", (String))
#define DMS_ERROR_LOG1(String, Para1) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d \r\n", (String), (VOS_INT32)(Para1))
#define DMS_ERROR_LOG2(String, Para1, Para2) \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2))
#define DMS_ERROR_LOG3(String, Para1, Para2, Para3)                                               \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2),         \
             (VOS_INT32)(Para3))
#define DMS_ERROR_LOG4(String, Para1, Para2, Para3, Para4)                                        \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (PS_PID_DMS), NULL, __LINE__, \
                             "%s, %d, %d, %d, %d \r\n", (String), (VOS_INT32)(Para1), (VOS_INT32)(Para2),     \
             (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /*__DMS_DEBUG_H__ */
