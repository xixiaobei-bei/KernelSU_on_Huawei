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
#include "at_voice_taf_rslt_proc.h"
#include "at_event_report.h"
#include "taf_type_def.h"
#include "securec.h"

#include "ppp_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "at_oam_interface.h"
#include "mn_comm_api.h"

#include "at_input_proc.h"
#include "at_cmd_msg_proc.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "gen_msg.h"
#include "at_lte_common.h"
#endif

#include "taf_app_mma.h"

#include "app_vc_api.h"
#include "taf_app_rabm.h"

#include "product_config.h"

#include "taf_std_lib.h"

#include "at_msg_print.h"

#include "mnmsgcbencdec.h"
#include "dms_file_node_i.h"
#include "at_cmd_msg_proc.h"
#include "at_mdrv_interface.h" /* DRV_OS_STATUS_SWITCH AT_SetModemState FREE_MEM_SIZE_GET */
#include "dms_msg_chk.h"
#include "dms_port_i.h"
#include "at_external_module_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_TAF_RSLT_PROC_C

VOS_VOID At_ProcSetClccResult(VOS_UINT8 numOfCalls, TAF_CCM_QryCallInfoCnf *qryCallInfoCnf, VOS_UINT8 indexNum)
{
    VOS_UINT32        tmp;
    AT_CLCC_ModeUint8 clccMode;
    VOS_UINT8         asciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];
    VOS_UINT16        length = 0;

    VOS_UINT8 numberType = AT_NUMBER_TYPE_UNKOWN;


    if ((numOfCalls != 0) && (numOfCalls <= AT_CALL_MAX_NUM)) {
        for (tmp = 0; tmp < numOfCalls; tmp++) {
            /* <CR><LF> */
            if (tmp != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            AT_MapCallTypeModeToClccMode(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callType, &clccMode);

            /* +CLCC:  */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName,              /* +CLCC: */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callId,     /* <id1>, */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callDir,    /* <dir>, */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callState,  /* <stat>, */
                clccMode,                                                  /* <mode>, */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].mptyState); /* <mpty>, */

            if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callDir == MN_CALL_DIR_MO) {
                if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.numLen != 0) {
                    /* <number>, */
                    AT_BcdNumberToAscii(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.bcdNum,
                                        qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.numLen,
                                        (TAF_CHAR *)asciiNum);

                    AT_FillClccRptPara(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.numType, &length,
                        asciiNum);
                } else if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.numLen != 0) {
                    /* <number>, */
                    AT_BcdNumberToAscii(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.bcdNum,
                                        qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.numLen,
                                        (TAF_CHAR *)asciiNum);

                    AT_FillClccRptPara(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.numType, &length,
                        asciiNum);
                } else {
                    /* <type>,不报<alpha>,<priority> */
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",\"\",%d", numberType);
                }
            } else {
                if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.numLen != 0) {
                    /* <number>, */
                    AT_BcdNumberToAscii(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.bcdNum,
                                        qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.numLen,
                                        (VOS_CHAR *)asciiNum);

                    AT_FillClccRptPara(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.numType, &length,
                        asciiNum);
                } else {
                    /* <type>,不报<alpha>,<priority> */
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",\"\",%d", numberType);
                }
            }
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_VOID At_ReportClccDisplayName(MN_CALL_DisplayName *displayName, VOS_UINT16 *length)
{
    VOS_UINT32 i;

    /* ,<display name> */
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");

    if (displayName->numLen != 0) {
        /* 以UTF8格式显示，如中国对应E4B8ADE59BBD */
        displayName->numLen = AT_MIN(displayName->numLen, MN_CALL_DISPLAY_NAME_STRING_SZ);
        for (i = 0; i < displayName->numLen; i++) {
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "%X", (VOS_UINT8)displayName->displayName[i]);
        }
    }
}

VOS_VOID At_ReportPeerVideoSupport(MN_CALL_InfoParam *callInfo, VOS_UINT16 *length)
{
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");

    if (callInfo->opPeerVideoSupport == VOS_TRUE) {
        /* <terminal video support> */
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", callInfo->peerVideoSupport);
    }
}

VOS_VOID At_ReportClccImsDomain(MN_CALL_InfoParam *callInfo, VOS_UINT16 *length)
{
    AT_IMS_CallDomainUint8 imsDomain = AT_IMS_CALL_DOMAIN_LTE;

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");

    if (callInfo->imsDomain != TAF_CALL_IMS_DOMAIN_NULL) {
        switch (callInfo->imsDomain) {
            case TAF_CALL_IMS_DOMAIN_LTE:
                imsDomain = AT_IMS_CALL_DOMAIN_LTE;
                break;

            case TAF_CALL_IMS_DOMAIN_WIFI:
                imsDomain = AT_IMS_CALL_DOMAIN_WIFI;
                break;
            case TAF_CALL_IMS_DOMAIN_NR:
                imsDomain = AT_IMS_CALL_DOMAIN_NR;
                break;

            default:
                break;
        }

        /* <imsDomain> */
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", imsDomain);
    }
}

VOS_VOID At_ReportClccRttPara(MN_CALL_InfoParam *callInfo, VOS_UINT16 *length)
{
    /* 不管是否为RTT通话，都需要输出 <RttFlg>,<RttChannelId>,<cps> 三个参数  */
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%d,%d,%d", callInfo->rtt, callInfo->rttChannelId, callInfo->cps);
}

VOS_VOID At_ReportClccEncryptPara(MN_CALL_InfoParam *callInfo, VOS_UINT16 *length)
{
    /* ,<isEncrypt> */
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%d", callInfo->encryptFlag);
}

VOS_VOID At_ProcQryClccResult(VOS_UINT8 numOfCalls, TAF_CCM_QryCallInfoCnf *qryCallInfoCnf, VOS_UINT8 indexNum)
{
    VOS_UINT32                tmp;
    VOS_UINT8                 numberType;
    VOS_UINT8                 asciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];
    VOS_UINT16                length;
    AT_CLCC_ModeUint8         clccMode;
    TAF_CALL_VoiceDomainUint8 voiceDomain;

    numberType = AT_NUMBER_TYPE_UNKOWN;
    length     = 0;

    if ((numOfCalls != 0) && (numOfCalls <= AT_CALL_MAX_NUM)) {
        for (tmp = 0; tmp < numOfCalls; tmp++) {
            /* <CR><LF> */
            if (tmp != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            AT_MapCallTypeModeToClccMode(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callType, &clccMode);

            if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].voiceDomain == TAF_CALL_VOICE_DOMAIN_3GPP2) {
                voiceDomain = TAF_CALL_VOICE_DOMAIN_3GPP;
            } else {
                voiceDomain = qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].voiceDomain;
            }

            /* ^CLCC:  */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d,%d,%d",
                g_parseContext[indexNum].cmdElement->cmdName,                    /* ^CLCC: */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callId,           /* <id1>, */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callDir,          /* <dir>, */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callState,        /* <stat>, */
                clccMode,                                                        /* <mode>, */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].mptyState,        /* <mpty>, */
                voiceDomain,                                                     /* <voice_domain> */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callType,         /* <call_type> */
                qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].eConferenceFlag); /* <isEConference> */

            if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callDir == MN_CALL_DIR_MO) {
                if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.numLen != 0) {
                    AT_BcdNumberToAscii(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.bcdNum,
                        AT_MIN(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.numLen,
                        MN_CALL_MAX_BCD_NUM_LEN), (TAF_CHAR *)asciiNum);

                    /* ,<number>,<type> */
                    length +=
                        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\",%d", asciiNum,
                            (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].connectNumber.numType |
                             AT_NUMBER_TYPE_EXT));
                } else if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.numLen != 0) {
                    AT_BcdNumberToAscii(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.bcdNum,
                        AT_MIN(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.numLen,
                        MN_CALL_MAX_CALLED_BCD_NUM_LEN), (TAF_CHAR *)asciiNum);

                    /* ,<number>,<type> */
                    length +=
                        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\",%d", asciiNum,
                            (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].calledNumber.numType | AT_NUMBER_TYPE_EXT));
                } else {
                    /* ,,<type> */
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",\"\",%d", numberType);
                }
            } else {
                if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.numLen != 0) {
                    AT_BcdNumberToAscii(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.bcdNum,
                        AT_MIN(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.numLen,
                        MN_CALL_MAX_BCD_NUM_LEN), (VOS_CHAR *)asciiNum);

                    /* ,<number>,<type> */
                    length +=
                        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                            (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\",%d", asciiNum,
                            (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callNumber.numType | AT_NUMBER_TYPE_EXT));
                } else {
                    /* ,,<type> */
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",\"\",%d", numberType);
                }
            }

            At_ReportClccDisplayName(&(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].displayName), &length);

            At_ReportPeerVideoSupport(&(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp]), &length);

            At_ReportClccImsDomain(&(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp]), &length);

            At_ReportClccRttPara(&(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp]), &length);

            At_ReportClccEncryptPara(&(qryCallInfoCnf->qryCallInfoPara.callInfos[tmp]), &length);
        }
    }

    g_atSendDataBuff.bufLen = length;
}

VOS_UINT32 AT_RcvTafCcmQryCallInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    VOS_UINT8               numOfCalls;
    VOS_UINT32              tmp;
    AT_CPAS_StatusUint8     cpas;
    VOS_UINT32              ret;
    TAF_CCM_QryCallInfoCnf *qryCallInfoCnf = (TAF_CCM_QryCallInfoCnf *)msg;

    /* 获取当前所有不为IDLE态的呼叫信息 */
    numOfCalls = qryCallInfoCnf->qryCallInfoPara.numOfCalls;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafCcmQryCallInfoCnf: AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    /* 格式化命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CLCC_SET) {
        /* CLCC命令的结果回复 */
        At_ProcSetClccResult(numOfCalls, qryCallInfoCnf, indexNum);

        ret = AT_OK;
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CLCC_QRY) {
        /* ^CLCC?命令的结果回复 */
        At_ProcQryClccResult(numOfCalls, qryCallInfoCnf, indexNum);

        ret = AT_OK;
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CPAS_SET) {
        /* CPAS命令的结果回复 */
        if (numOfCalls > AT_CALL_MAX_NUM) {
            At_FormatResultData(indexNum, AT_CME_UNKNOWN);
            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

        if (numOfCalls == 0) {
            cpas = AT_CPAS_STATUS_READY;
        } else {
            cpas = AT_CPAS_STATUS_CALL_IN_PROGRESS;
            for (tmp = 0; tmp < numOfCalls; tmp++) {
                if (qryCallInfoCnf->qryCallInfoPara.callInfos[tmp].callState == MN_CALL_S_INCOMING) {
                    cpas = AT_CPAS_STATUS_RING;
                    break;
                }
            }
        }

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, cpas);

        ret = AT_OK;

    } else {
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

PS_BOOL_ENUM_UINT8 AT_CheckCurrentOptType_SupsCmdSuccess(VOS_UINT8 ataReportOkAsyncFlag, TAF_UINT8 indexNum)
{
    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_H_SET:
        case AT_CMD_CHUP_SET:
        case AT_CMD_REJCALL_SET:
#if (FEATURE_ECALL == FEATURE_ON)
        case AT_CMD_ECLSTOP_SET:
#endif
            return PS_TRUE;
        case AT_CMD_A_SET:
        case AT_CMD_CHLD_SET:
        case AT_CMD_CHLD_EX_SET:
            if (ataReportOkAsyncFlag == VOS_TRUE) {
                return PS_TRUE;
            }

            return PS_FALSE;
        default:
            return PS_FALSE;
    }
}

PS_BOOL_ENUM_UINT8 AT_CheckCurrentOptType_SupsCmdOthers(TAF_UINT8 indexNum)
{
    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_CHLD_SET:
        case AT_CMD_CHUP_SET:
        case AT_CMD_A_SET:
        case AT_CMD_CHLD_EX_SET:
        case AT_CMD_H_SET:
        case AT_CMD_REJCALL_SET:
            return PS_TRUE;

        default:
            return PS_FALSE;
    }
}

VOS_UINT32 At_RcvTafCcmCallSupsCmdCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallSupsCmdCnf *callSupsCmdCnf = (TAF_CCM_CallSupsCmdCnf *)msg;
    TAF_UINT32              result = AT_FAILURE;

    AT_LOG1("At_RcvTafCallOrigCnf pEvent->ClientId", callSupsCmdCnf->ctrl.clientId);
    AT_LOG1("At_RcvTafCallOrigCnf usMsgName", callSupsCmdCnf->msgName);

    AT_PR_LOGH("At_RcvTafCcmCallSupsCmdCnf Enter : enCause = %d", callSupsCmdCnf->supsCmdPara.cause);

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_RcvTafCcmCallSupsCmdCnf: AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    /* AT口已经释放 */
    if (g_parseContext[indexNum].clientStatus == AT_FW_CLIENT_STATUS_READY) {
        AT_WARN_LOG("At_RcvTafCallSupsCmdCnf : AT command entity is released.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    /* 挂断电话成功时回复OK */
    if (callSupsCmdCnf->supsCmdPara.cause == TAF_CS_CAUSE_SUCCESS) {
        if (AT_CheckCurrentOptType_SupsCmdSuccess(callSupsCmdCnf->supsCmdPara.ataReportOkAsyncFlag, indexNum) ==
            PS_TRUE) {
            result = AT_OK;
        } else {
            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }
    } else {
        if (AT_CheckCurrentOptType_SupsCmdOthers(indexNum) == PS_TRUE) {
            result = AT_ConvertCallError(callSupsCmdCnf->supsCmdPara.cause);
        } else {
            result = AT_CME_UNKNOWN;
        }
    }

    g_atSendDataBuff.bufLen = 0;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvTafCcmCallSupsCmdRsltInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallSupsCmdRslt *callSupsCmdRslt = VOS_NULL_PTR;
    VOS_UINT32               result          = 0;
    TAF_UINT16               length          = 0;

    callSupsCmdRslt = (TAF_CCM_CallSupsCmdRslt *)msg;

    AT_PR_LOGH("AT_RcvTafCcmCallSupsCmdRsltInd Enter : enCause = %d", callSupsCmdRslt->supsCmdRsltPara.ssResult);

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafCcmCallSupsCmdRsltInd: AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CURRENT_OPT_BUTT) {
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (callSupsCmdRslt->supsCmdRsltPara.ssResult == MN_CALL_SS_RES_SUCCESS) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }
    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvTafCcmGetCdurCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_GetCdurCnf *cdurCnf = (TAF_CCM_GetCdurCnf *)msg;
    VOS_UINT32          result = AT_OK;

    if (cdurCnf->getCdurPara.cause == TAF_CS_CAUSE_SUCCESS) {
        /* 输出查询结果: 构造结构为^CDUR: <CurCallTime>格式 */
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            cdurCnf->getCdurPara.callId, cdurCnf->getCdurPara.curCallTime);

        result = AT_OK;

    } else {
        result = AT_ConvertCallError(cdurCnf->getCdurPara.cause);

        /* 调用At_FormatResultData输出结果 */
        g_atSendDataBuff.bufLen = 0;
    }

    return result;
}

VOS_UINT16 At_PrintClprInfo(VOS_UINT8 indexNum, TAF_CCM_QryClprCnf *clprGetCnf)
{
    errno_t    memResult;
    VOS_UINT16 length;
    VOS_UINT8  type;
    VOS_CHAR   asciiNum[(MN_CALL_MAX_BCD_NUM_LEN * 2) + 1];
    VOS_UINT32 asiciiLen;
    VOS_UINT8  bcdNumLen;

    /* 初始化 */
    length = 0;
    memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    /* 输出<RedirectNumPI>参数 */
    if (clprGetCnf->clprCnf.clprInfo.opCallingNumPi == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", clprGetCnf->clprCnf.clprInfo.callingNumPi);

    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* 输出<no_CLI_cause>参数 */
    if (clprGetCnf->clprCnf.clprInfo.opNoCliCause == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", clprGetCnf->clprCnf.clprInfo.noCliCause);

    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* 输出<redirect_num>和<num_type>参数 */
    if (clprGetCnf->clprCnf.clprInfo.redirectInfo.opRedirectNum == VOS_TRUE) {
        bcdNumLen = TAF_MIN(clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectNum.numLen, MN_CALL_MAX_BCD_NUM_LEN);
        AT_BcdNumberToAscii(clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectNum.bcdNum, bcdNumLen, asciiNum);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\",%d,", asciiNum,
            (clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectNum.numType | AT_NUMBER_TYPE_EXT));

    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"\",,");
    }

    /* 输出<redirect_subaddr>和<num_type>参数 */
    type = (MN_CALL_IS_EXIT | (MN_CALL_SUBADDR_NSAP << 4));
    if ((clprGetCnf->clprCnf.clprInfo.redirectInfo.opRedirectSubaddr == VOS_TRUE) &&
        (type == clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.octet3)) {
        if (clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.lastOctOffset <
            sizeof(clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.octet3)) {
            AT_WARN_LOG1("At_PrintClprInfo: pstClprGetCnf->stRedirectInfo.stRedirectSubaddr.LastOctOffset: ",
                         clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.lastOctOffset);
            clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.lastOctOffset =
                sizeof(clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.octet3);
        }

        asiciiLen = clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.lastOctOffset -
                    sizeof(clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.octet3);

        asiciiLen = TAF_MIN(asiciiLen, sizeof(clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.subAddrInfo));
        if (asiciiLen > 0) {
            memResult = memcpy_s(asciiNum, sizeof(asciiNum),
                                 clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.subAddrInfo, asiciiLen);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(asciiNum), asiciiLen);
        }

        asciiNum[asiciiLen] = '\0';

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\",%d,", asciiNum,
            clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectSubaddr.octet3);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"\",,");
    }

    /* 输出<CallingNumPI>参数 */
    if (clprGetCnf->clprCnf.clprInfo.redirectInfo.opRedirectNumPi == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", clprGetCnf->clprCnf.clprInfo.redirectInfo.redirectNumPi);
    }

    return length;
}

VOS_UINT32 AT_RcvTafCcmQryClprCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_QryClprCnf *clprGetCnf = VOS_NULL_PTR;
    VOS_UINT16          length;

    /* 初始化 */
    clprGetCnf = (TAF_CCM_QryClprCnf *)msg;

    /* 判断查询操作是否成功 */
    if (clprGetCnf->clprCnf.ret != TAF_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    length = 0;

    if (clprGetCnf->clprCnf.qryClprModeType == TAF_CALL_QRY_CLPR_MODE_GUL) {
        length = At_PrintClprInfo(indexNum, clprGetCnf);
    } else {
        if (clprGetCnf->clprCnf.pi != TAF_CALL_PRESENTATION_BUTT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                clprGetCnf->clprCnf.pi);
        }
    }

    /* 打印结果 */
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_IsCmdCurrentOptSendedOrigReq(AT_CmdCurOptUint32 cmdCurrentOpt)
{
    switch (cmdCurrentOpt) {
        case AT_CMD_APDS_SET:
        case AT_CMD_D_CS_VOICE_CALL_SET:
        case AT_CMD_CECALL_SET:
        case AT_CMD_ECLSTART_SET:
        case AT_CMD_CACMIMS_SET:
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
        case AT_CMD_CUSTOMDIAL_SET:
#endif
            return VOS_TRUE;

        default:
            return VOS_FALSE;
    }
}

#if (FEATURE_ECALL == FEATURE_ON)
VOS_VOID At_MapEcallType(MN_CALL_TypeUint8 callType, AT_ECALL_TypeUint8 *ecallType)
{
    switch (callType) {
        case MN_CALL_TYPE_TEST:
            *ecallType = AT_ECALL_TYPE_TEST;
            break;
        case MN_CALL_TYPE_RECFGURATION:
            *ecallType = AT_ECALL_TYPE_RECFGURATION;
            break;
        case MN_CALL_TYPE_MIEC:
            *ecallType = AT_ECALL_TYPE_MIEC;
            break;
        case MN_CALL_TYPE_AIEC:
            *ecallType = AT_ECALL_TYPE_AIEC;
            break;
        default:
            *ecallType = AT_ECALL_TYPE_BUTT;
            break;
    }
}

#endif

PS_BOOL_ENUM_UINT8 At_CheckOrigCnfCallType(MN_CALL_TypeUint8 callType, VOS_UINT8 indexNum)
{
    switch (callType) {
        case MN_CALL_TYPE_VOICE:
        case MN_CALL_TYPE_EMERGENCY:
        case MN_CALL_TYPE_VIDEO_RX:
        case MN_CALL_TYPE_VIDEO_TX:
        case MN_CALL_TYPE_MIEC:
        case MN_CALL_TYPE_AIEC:
        case MN_CALL_TYPE_TEST:
        case MN_CALL_TYPE_RECFGURATION:
            if (At_IsCmdCurrentOptSendedOrigReq(g_atClientTab[indexNum].cmdCurrentOpt) == VOS_TRUE) {
                return PS_TRUE;
            }

            return PS_FALSE;
        case MN_CALL_TYPE_VIDEO:
            if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_APDS_SET) {
                return PS_TRUE;
            } else {
                return PS_FALSE;
            }
            /* fall-through */

        default:
            return PS_FALSE;
    }
}

VOS_UINT32 At_RcvTafCcmCallOrigCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallOrigCnf *origCnf = (TAF_CCM_CallOrigCnf *)msg;
    TAF_UINT32           result = AT_FAILURE;

    AT_LOG1("At_RcvTafCallOrigCnf pEvent->ClientId", origCnf->ctrl.clientId);
    AT_LOG1("At_RcvTafCallOrigCnf usMsgName", origCnf->msgName);

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_RcvTafCcmCallOrigCnf: index Is Broadcast!");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    /* 成功时，回复OK；失败时，回复NO CARRIER */
    if (origCnf->origCnfPara.cause == TAF_CS_CAUSE_SUCCESS) {
        /* 可视电话里面，这里不能上报OK，因此只有普通语音和紧急呼叫的情况下，才上报OK，AT命令在这个阶段相当于阻塞一段时间 */
        if (At_CheckOrigCnfCallType(origCnf->origCnfPara.callType, indexNum) == PS_TRUE) {
            result = AT_OK;
        } else {
            if (origCnf->origCnfPara.callType == MN_CALL_TYPE_VIDEO) {
                AT_StopTimerCmdReady(indexNum);
            }
            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

    } else {
        if (origCnf->origCnfPara.cause == TAF_CS_CAUSE_NO_CALL_ID) {
            result = AT_ERROR;
        } else {
            result = AT_NO_CARRIER;
        }

        AT_UpdateCallErrInfo(indexNum, origCnf->origCnfPara.cause, &(origCnf->origCnfPara.errInfoText));
    }

    g_atSendDataBuff.bufLen = 0;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEconfDialCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EconfDialCnf *econfDialCnf = (TAF_CCM_EconfDialCnf *)msg;

    /* 判断操作是否成功 */
    if (econfDialCnf->econfDialCnf.cause != TAF_CS_CAUSE_SUCCESS) {
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}

VOS_UINT32 At_ProcQryClccEconfResult(TAF_CCM_QryEconfCalledInfoCnf *callInfos, VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    VOS_UINT32 tmp;
    VOS_CHAR   asciiNum[MN_CALL_MAX_CALLED_ASCII_NUM_LEN + 1];

    memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));

    /* ^CLCCECONF: Maximum-user-count, n_address */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress,
        "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, /* ^CLCCECONF: */
        callInfos->numOfMaxCalls,                                  /* Maximum-user-count */
        callInfos->numOfCalls);

    callInfos->numOfCalls = AT_MIN(callInfos->numOfCalls, TAF_CALL_MAX_ECONF_CALLED_NUM);
    if (callInfos->numOfCalls != 0) {
        /* n_address */
        for (tmp = 0; tmp < callInfos->numOfCalls; tmp++) {
            /* 转换电话号码 */
            if (callInfos->callInfo[tmp].callNumber.numLen != 0) {
                /* <number>, */
                AT_BcdNumberToAscii(callInfos->callInfo[tmp].callNumber.bcdNum,
                    AT_MIN(callInfos->callInfo[tmp].callNumber.numLen, MN_CALL_MAX_CALLED_BCD_NUM_LEN),
                    asciiNum);
            }

            /* entity, Display-text,Status */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",\"%s\",\"%s\",%d",
                asciiNum,                             /* 电话号码 */
                callInfos->callInfo[tmp].displaytext, /* display-text */
                callInfos->callInfo[tmp].callState);  /* Call State */
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmQryEconfCalledInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_QryEconfCalledInfoCnf *econfInfo = (TAF_CCM_QryEconfCalledInfoCnf *)msg;

    return At_ProcQryClccEconfResult(econfInfo, indexNum);
}
#endif

VOS_UINT32 AT_RcvTafCcmCSChannelInfoQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_QryChannelInfoCnf *channelInfo = VOS_NULL_PTR;
    VOS_UINT32                 result;

    channelInfo = (TAF_CCM_QryChannelInfoCnf *)msg;

    if (channelInfo->para.result != VOS_OK) {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    } else {
        result = AT_OK;

        /* 输出查询结果 */
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            channelInfo->para.channelType, channelInfo->para.voiceDomain);
    }

    return result;
}

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 At_ProcVcSetEcallCfgEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 rslt;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ECLCFG_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_VOID AT_RcvVcMsgQryEcallCfgCnfProc(MN_AT_IndEvt *data)
{
    APP_VC_MsgQryEcallCfgCnf *qryCfg = VOS_NULL_PTR;
    VOS_UINT8                 indexNum;
    VOS_UINT32                ret;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_ECLCFG_QRY, &indexNum) != VOS_OK) {
        return;
    }

    /* 初始化 */
    qryCfg = (APP_VC_MsgQryEcallCfgCnf *)data->content;

    if (qryCfg->qryRslt == VOS_OK) {
        /* 输出查询结果 */
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, qryCfg->mode,
            qryCfg->vocConfig);
        ret                     = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_ERROR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_VOID AT_RcvVcMsgSetMsdCnfProc(MN_AT_IndEvt *data)
{
    VOS_UINT8         indexNum;
    VOS_UINT32        ret;
    APP_VC_SetMsdCnf *rslt = VOS_NULL_PTR;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_ECLMSD_SET, &indexNum) != VOS_OK) {
        return;
    }

    rslt = (APP_VC_SetMsdCnf *)data->content;

    if (rslt->rslt == VOS_OK) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_VOID AT_RcvVcMsgQryMsdCnfProc(MN_AT_IndEvt *data)
{
    APP_VC_MsgQryMsdCnf *qryMsd = VOS_NULL_PTR;
    VOS_UINT8            indexNum;
    VOS_UINT32           ret;
    VOS_UINT32           i;
    VOS_UINT16           length;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_ECLMSD_QRY, &indexNum) != VOS_OK) {
        return;
    }

    /* 初始化 */
    qryMsd = (APP_VC_MsgQryMsdCnf *)data->content;

    if (qryMsd->qryRslt == VOS_OK) {
        /* 输出查询结果 */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: \"", g_parseContext[indexNum].cmdElement->cmdName);

        for (i = 0; i < APP_VC_MSD_DATA_LEN; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02x", qryMsd->msdData[i]);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");

        g_atSendDataBuff.bufLen = length;
        ret                     = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_ERROR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_UINT32 AT_ProcVcEcallAbortCnf(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 rslt = AT_ERROR;

    /* 参数有效性判断 */
    if (vcEvtInfo == VOS_NULL) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcEcallAbortCnf : AT_BROADCAST_INDEX.");

        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ABORT_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 atReadLtecsCnfProc(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_LtecsCnf *ltecsReadCnf = NULL;
    VOS_UINT16       length       = 0;
    VOS_UINT32       result;

    ltecsReadCnf = (TAF_PS_LtecsCnf *)evtInfo;

    if (ltecsReadCnf->cause == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "^LTECS:");
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d", ltecsReadCnf->lteCs.sg, ltecsReadCnf->lteCs.ims);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d", ltecsReadCnf->lteCs.csfb, ltecsReadCnf->lteCs.vcc);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", ltecsReadCnf->lteCs.voLga);

        result                  = AT_OK;
        g_atSendDataBuff.bufLen = length;
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}
#endif

/*
 * MN_CALLBACK_CMD_CNF消息ATD/APDS命令处理
 */
VOS_UINT32 AT_ProcMnCallBackCmdCnfApdsCmd(TAF_UINT8 indexNum, TAF_UINT32 errorCode)
{
    TAF_UINT32      result = AT_FAILURE;

    if (errorCode == TAF_CS_CAUSE_NO_CALL_ID) {
        result = AT_ERROR;
        AT_StopTimerCmdReady(indexNum);
    }
    return result;
}

/*
 * MN_CALLBACK_CMD_CNF消息ATD CS DATA CALL命令处理
 */
VOS_UINT32 AT_ProcMnCallBackCmdCnfCsDataCall(TAF_UINT8 indexNum, TAF_UINT32 errorCode)
{
    TAF_UINT32      result = AT_FAILURE;

    result = AT_NO_CARRIER;
    AT_StopTimerCmdReady(indexNum);

    return result;
}

VOS_UINT32 AT_RcvMmaQryEmcCallBackCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_EmcCallBackQryCnf *qryEmcCallBackCnf = (TAF_MMA_EmcCallBackQryCnf *)msg;
    VOS_UINT32                 result;

    /* 格式化AT^QCCB查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (qryEmcCallBackCnf->errorCode == TAF_ERR_NO_ERROR) {
        result = AT_OK;
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            qryEmcCallBackCnf->isInCallBackMode);
    } else {
        result = AT_CME_UNKNOWN;
    }

    return result;
}

/*
 * 功能说明: 公共接口，上报CLCC的返回结果中的电话号码
 * 输入参数: numType: 号码类型，国际号码还是本地号码
 *           length: 当前输出结果字符串中的字符长度
 *           asciiNum: 号码字符串
 */
VOS_VOID AT_FillClccRptPara(VOS_UINT8 numType, VOS_UINT16 *length, const VOS_UINT8 *asciiNum)
{
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 如果是国际号码，在打印号码前加"+"号 */
    /* 在numType中，比特5~7表示号码地址类型，001表示国际号码 */
    if (((numType >> 4) & 0x07) == MN_MSG_TON_INTERNATIONAL) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"+%s\",%d", asciiNum, numType | AT_NUMBER_TYPE_EXT);
    } else {
        /* <type>,不报<alpha>,<priority> */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\",%d", asciiNum, numType | AT_NUMBER_TYPE_EXT);
    }
#else
    /* <type>,不报<alpha>,<priority> */
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *length, ",\"%s\",%d,\"\",", asciiNum, numType | AT_NUMBER_TYPE_EXT);

#endif
}

#if (FEATURE_ECALL == FEATURE_ON)
VOS_VOID AT_RcvVcMsgEcallPushCnfProc(MN_AT_IndEvt *data)
{
    VOS_UINT8         indexNum = 0;
    VOS_UINT32        ret      = VOS_ERR;
    APP_VC_SetMsdCnf *rslt     = VOS_NULL_PTR;

    if (data == VOS_NULL_PTR) {
        return;
    }

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(data->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvVcMsgEcallPushCnfProc:WARNING:AT INDEX NOT FOUND!");

        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvVcMsgEcallPushCnfProc : AT_BROADCAST_INDEX.");

        return;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ECLPUSH_SET) {
        AT_WARN_LOG("AT_RcvVcMsgEcallPushCnfProc:WARNING:AT ARE WAITING ANOTHER CMD!");

        return;
    }

    rslt = (APP_VC_SetMsdCnf *)data->content;

    if (rslt->rslt == VOS_OK) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    At_FormatResultData(indexNum, ret);
}
#endif


