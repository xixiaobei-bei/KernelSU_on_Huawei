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

#include "at_data_lnas_rslt_proc.h"
#include "securec.h"
#include "taf_ps_api.h"
#include "taf_api.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_LNAS_RSLT_PROC_C

#define AT_BIT_RATE_TRAFFIC_FLOWS_MAX_VALUE_RANGE 4

VOS_UINT32 AT_RcvTafPsEvtSetEpsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetEpsQosInfoCnf *setEpsqosInfoCnf = VOS_NULL_PTR;

    setEpsqosInfoCnf = (TAF_PS_SetPrimPdpContextInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQOS_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setEpsqosInfoCnf->cause);

    return VOS_OK;
}

VOS_VOID AT_PrintDynamicEpsQosInfo(TAF_PS_GetDynamicEpsQosInfoCnf *getDynamicEpsQosInfoCnf, VOS_UINT8 indexNum)
{
    errno_t        memResult;
    TAF_EPS_QosExt cgeqos;
    VOS_UINT16     length = 0;
    VOS_UINT32     tmp    = 0;

    (VOS_VOID)memset_s(&cgeqos, sizeof(cgeqos), 0x00, sizeof(TAF_EPS_QosExt));

    for (tmp = 0; tmp < getDynamicEpsQosInfoCnf->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgeqos, sizeof(cgeqos), &getDynamicEpsQosInfoCnf->epsQosInfo[tmp],
                             sizeof(TAF_EPS_QosExt));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgeqos), sizeof(TAF_EPS_QosExt));

        /* +CGEQOSRDP:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cgeqos.cid);
        /* <QCI> */
        if (cgeqos.opQci == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.qci);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        if (cgeqos.qci <= AT_BIT_RATE_TRAFFIC_FLOWS_MAX_VALUE_RANGE) {
            /* <DL GBR> */
            if (cgeqos.opDlgbr == 1) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.dlgbr);
            } else {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",");
            }
            /* <UL GBR> */
            if (cgeqos.opUlgbr == 1) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.ulgbr);
            } else {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",");
            }
            /* <DL MBR> */
            if (cgeqos.opDlmbr == 1) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.dlmbr);
            } else {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",");
            }
            /* <UL MBR> */
            if (cgeqos.opUlmbr == 1) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.ulmbr);
            }
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_RcvTafPsEvtGetDynamicEpsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_GetDynamicEpsQosInfoCnf *getDynamicEpsQosInfoCnf = VOS_NULL_PTR;
    VOS_UINT32                      result                  = AT_FAILURE;

    getDynamicEpsQosInfoCnf = (TAF_PS_GetDynamicEpsQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQOSRDP_SET) {
        return VOS_ERR;
    }

    if (getDynamicEpsQosInfoCnf->cause == VOS_OK) {
        AT_PrintDynamicEpsQosInfo(getDynamicEpsQosInfoCnf, indexNum);
        result = AT_OK;
    }
    else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetEpsQosInfoCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    errno_t    memResult;
    VOS_UINT32 result;
    VOS_UINT16 length = 0;
    VOS_UINT32 tmp    = 0;

    TAF_EPS_QosExt           cgeqos;
    TAF_PS_GetEpsQosInfoCnf *getEpsQosInfoCnf = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&cgeqos, sizeof(cgeqos), 0x00, sizeof(TAF_EPS_QosExt));

    getEpsQosInfoCnf = (TAF_PS_GetEpsQosInfoCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGEQOS_READ) {
        return VOS_ERR;
    }

    for (tmp = 0; tmp < getEpsQosInfoCnf->cidNum; tmp++) {
        if (tmp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        memResult = memcpy_s(&cgeqos, sizeof(cgeqos), &getEpsQosInfoCnf->epsQosInfo[tmp], sizeof(TAF_EPS_QosExt));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgeqos), sizeof(TAF_EPS_QosExt));

        /* +CGEQOS:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        /* <cid> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", cgeqos.cid);
        /* <QCI> */
        if (cgeqos.opQci == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.qci);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <DL GBR> */
        if (cgeqos.opDlgbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.dlgbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <UL GBR> */
        if (cgeqos.opUlgbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.ulgbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <DL MBR> */
        if (cgeqos.opDlmbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.dlmbr);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
        /* <UL MBR> */
        if (cgeqos.opUlmbr == 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgeqos.ulmbr);
        }
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

