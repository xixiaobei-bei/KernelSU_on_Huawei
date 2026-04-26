/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "at_safety_pam_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_drv_agent.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAFETY_PAM_RSLT_PROC_C

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_UiccAuthCnf(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_UICCAUTH_SET/AT_CMD_KSNAFAUTH_SET */
    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_UICCAUTH_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_KSNAFAUTH_SET)) {
        AT_WARN_LOG("AT_UiccAuthCnf : CmdCurrentOpt is not AT_CMD_UICCAUTH_SET/AT_CMD_KSNAFAUTH_SET!");
        return AT_ERROR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_UICCAUTH_SET) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "^UICCAUTH:");

        /* <result> */
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", event->pihEvent.uiccAuthCnf.status);

        if (event->pihEvent.uiccAuthCnf.status == SI_PIH_AUTH_SUCCESS) {
            /* ,<Res> */
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"");
            (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                             (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                             &event->pihEvent.uiccAuthCnf.akaData.authRes[1],
                                                             event->pihEvent.uiccAuthCnf.akaData.authRes[0]);
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"");

            if (event->pihEvent.uiccAuthCnf.authType == SI_PIH_UICCAUTH_AKA) {
                /* ,<ck> */
                (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"");
                (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                                 (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                                 &event->pihEvent.uiccAuthCnf.akaData.ck[1],
                                                                 event->pihEvent.uiccAuthCnf.akaData.ck[0]);
                (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"");

                /* ,<ik> */
                (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"");
                (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                                 (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                                 &event->pihEvent.uiccAuthCnf.akaData.ik[1],
                                                                 event->pihEvent.uiccAuthCnf.akaData.ik[0]);
                (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"");
            }
        }

        if (event->pihEvent.uiccAuthCnf.status == SI_PIH_AUTH_SYNC) {
            /* ,"","","",<autn> */
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"\",\"\",\"\",\"");
            (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                             (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                             &event->pihEvent.uiccAuthCnf.akaData.auts[1],
                                                             event->pihEvent.uiccAuthCnf.akaData.auts[0]);
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"");
        }
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_KSNAFAUTH_SET) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "^KSNAFAUTH:");

        /* <status> */
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", event->pihEvent.uiccAuthCnf.status);

        if (event->pihEvent.uiccAuthCnf.nAFData.ksExtNaf[0] != VOS_NULL) {
            /* ,<ks_Naf> */
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"");
            (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                             (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                             &event->pihEvent.uiccAuthCnf.nAFData.ksExtNaf[1],
                                                             event->pihEvent.uiccAuthCnf.nAFData.ksExtNaf[0]);
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"");
        }
    }

    return AT_OK;
}
#endif

VOS_UINT32 AT_RcvMtaImeiVerifyQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg *mtaMsg = (AT_MTA_Msg *)msg;
    VOS_UINT32 *imeiVerify = (VOS_UINT32 *)mtaMsg->content;
    VOS_UINT16  length = 0;
    /* 打印^IMEIVERIFY */

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^IMEIVERIFY: ");

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%d", *imeiVerify);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

