/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#ifndef __RNIC_LOG_H__
#define __RNIC_LOG_H__

#include "vos.h"
#include "product_config.h"
#if (!defined(MODEM_FUSION_VERSION)) || (defined(LLT_OS_VER))
#include "msp_diag_comm.h"
#else
#include "mdrv_diag.h"
#endif

#include "rnic_private.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

#ifndef SUBMOD_NULL
#define SUBMOD_NULL (0)
#endif

#define RNIC_INFO_LOG(Mod, String)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (Mod), \
                                   NULL, __LINE__, (String " \r\n"))
#define RNIC_INFO_LOG1(Mod, String, Para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (Mod), \
                                   NULL, __LINE__, (String " %d \r\n"), (VOS_INT32)(Para1))
#define RNIC_INFO_LOG2(Mod, String, Para1, Para2)                                                                    \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d \r\n"), (VOS_INT32)(Para1),                  \
                                   (VOS_INT32)(Para2))
#define RNIC_INFO_LOG3(Mod, String, Para1, Para2, Para3)                                                             \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d, %d \r\n"), (VOS_INT32)(Para1),              \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3))
#define RNIC_INFO_LOG4(Mod, String, Para1, Para2, Para3, Para4)                                                      \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_INFO)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d, %d, %d \r\n"), (VOS_INT32)(Para1),          \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#define RNIC_NORMAL_LOG(Mod, String)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_NORMAL)), (Mod), \
                                   NULL, __LINE__, (String " \r\n"))
#define RNIC_NORMAL_LOG1(Mod, String, Para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_NORMAL)), (Mod), \
                                   NULL, __LINE__, (String " %d \r\n"), (VOS_INT32)(Para1))
#define RNIC_NORMAL_LOG2(Mod, String, Para1, Para2)                                                                    \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_NORMAL)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d \r\n"), (VOS_INT32)(Para1),                    \
                                   (VOS_INT32)(Para2))
#define RNIC_NORMAL_LOG3(Mod, String, Para1, Para2, Para3)                                                             \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_NORMAL)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d, %d \r\n"), (VOS_INT32)(Para1),                \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3))
#define RNIC_NORMAL_LOG4(Mod, String, Para1, Para2, Para3, Para4)                                                      \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_NORMAL)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d, %d, %d \r\n"), (VOS_INT32)(Para1),            \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#define RNIC_WARNING_LOG(Mod, String)                                                                            \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), \
                                   (Mod), NULL, __LINE__, (String " \r\n"))
#define RNIC_WARNING_LOG1(Mod, String, Para1)                                                                    \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), \
                                   (Mod), NULL, __LINE__, (String " %d \r\n"), (VOS_INT32)(Para1))
#define RNIC_WARNING_LOG2(Mod, String, Para1, Para2)                                                             \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), \
                                   (Mod), NULL, __LINE__, (String " %d, %d \r\n"), (VOS_INT32)(Para1),       \
                                   (VOS_INT32)(Para2))
#define RNIC_WARNING_LOG3(Mod, String, Para1, Para2, Para3)                                                      \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)), \
                                   (Mod), NULL, __LINE__, (String " %d, %d, %d \r\n"), (VOS_INT32)(Para1),   \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3))
#define RNIC_WARNING_LOG4(Mod, String, Para1, Para2, Para3, Para4)                                                 \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_WARNING)),   \
                                   (Mod), NULL, __LINE__, (String " %d, %d, %d, %d \r\n"), (VOS_INT32)(Para1), \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#define RNIC_ERROR_LOG(Mod, String)                                                                                   \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (Mod), \
                                   NULL, __LINE__, (String " \r\n"))
#define RNIC_ERROR_LOG1(Mod, String, Para1)                                                                           \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (Mod), \
                                   NULL, __LINE__, (String " %d \r\n"), (VOS_INT32)(Para1))
#define RNIC_ERROR_LOG2(Mod, String, Para1, Para2)                                                                    \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d \r\n"), (VOS_INT32)(Para1),                   \
                                   (VOS_INT32)(Para2))
#define RNIC_ERROR_LOG3(Mod, String, Para1, Para2, Para3)                                                             \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d, %d \r\n"), (VOS_INT32)(Para1),               \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3))
#define RNIC_ERROR_LOG4(Mod, String, Para1, Para2, Para3, Para4)                                                      \
    (VOS_VOID)mdrv_diag_log_report(MDRV_DIAG_GEN_LOG_MODULE(MODEM_ID_0, DIAG_MODE_UMTS, (PS_LOG_LEVEL_ERROR)), (Mod), \
                                   NULL, __LINE__, (String " %d, %d, %d, %d \r\n"), (VOS_INT32)(Para1),           \
                                   (VOS_INT32)(Para2), (VOS_INT32)(Para3), (VOS_INT32)(Para4))

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __RNIC_LOG_H__ */
