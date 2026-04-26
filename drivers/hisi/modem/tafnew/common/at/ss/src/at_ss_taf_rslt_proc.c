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
#include "at_ss_taf_rslt_proc.h"
#include "at_ss_event_report.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "taf_type_def.h"
#include "at_data_proc.h"
#include "at_device_cmd.h"
#include "at_event_report.h"
#include "at_ss_comm.h"
#include "mnmsgcbencdec.h"
#include "taf_std_lib.h"
#include "at_external_module_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SS_TAF_RSLT_PROC_C

#define AT_PROCESS_USS_REQ_CNF_TMP 2
#define AT_CLI_SS_UNKNOWN_REASON 2
#define AT_CLIR_SS_PROVIDED 0
#define AT_CLIR_SS_PERMANENT 1
#define AT_CLIR_SS_UNKNOWN_REASON 2
#define AT_CLIR_SS_TMP_DEFAULT_RESTRICTED 3
#define AT_CLIR_SS_TMP_DEFAULT_ALLOWED 4
#define AT_CMD_CNAP_NO_OP_SS_STATUS 2

STATIC VOS_UINT32 AT_ChkTafSsaMolrNtfMsgLen(const MSG_Header *msgHeader);

static const AT_SS_EvtFuncTbl g_atSsEvtFuncTbl[] = {
    /* 事件ID */ /* 消息处理函数 */
    { ID_TAF_SSA_SET_LCS_MOLR_CNF,  sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_SetLcsMolrCnf) - 4,
      VOS_NULL_PTR, AT_RcvSsaSetLcsMolrCnf },
    { ID_TAF_SSA_GET_LCS_MOLR_CNF,  sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_GetLcsMolrSnf) - 4,
      VOS_NULL_PTR, AT_RcvSsaGetLcsMolrCnf },
    { ID_TAF_SSA_LCS_MOLR_NTF,      0, AT_ChkTafSsaMolrNtfMsgLen, AT_RcvSsaLcsMolrNtf },
    { ID_TAF_SSA_SET_LCS_MTLR_CNF,  sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_SetLcsMtlrCnf) - 4,
      VOS_NULL_PTR, AT_RcvSsaSetLcsMtlrCnf },
    { ID_TAF_SSA_GET_LCS_MTLR_CNF,  sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_GetLcsMtlrCnf) - 4,
      VOS_NULL_PTR, AT_RcvSsaGetLcsMtlrCnf },
    { ID_TAF_SSA_LCS_MTLR_NTF,      sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_LcsMtlrNtf) - 4,
      VOS_NULL_PTR, AT_RcvSsaLcsMtlrNtf },
    { ID_TAF_SSA_SET_LCS_MTLRA_CNF, sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_SetLcsMtlraCnf) - 4,
      VOS_NULL_PTR, AT_RcvSsaSetLcsMtlraCnf },
    { ID_TAF_SSA_GET_LCS_MTLRA_CNF, sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_GetLcsMtlraCnf) - 4,
      VOS_NULL_PTR, AT_RcvSsaGetLcsMtlraCnf },
};

STATIC VOS_UINT32 AT_ChkTafSsaMolrNtfMsgLen(const MSG_Header *msgHeader)
{
    const TAF_SSA_Evt        *msg = VOS_NULL_PTR;
    const TAF_SSA_LcsMolrNtf *ntf = VOS_NULL_PTR;
    VOS_UINT32 minLen;
    VOS_UINT32 expectedLen;

    /* 8表示TAF_SSA_LcsMolrNtf.locationStr[8]的大小 */
    minLen = sizeof(TAF_SSA_Evt) + sizeof(TAF_SSA_LcsMolrNtf) -
        8 - VOS_MSG_HEAD_LENGTH - TAF_DEFAULT_CONTENT_LEN;
    if (VOS_GET_MSG_LEN(msgHeader) < minLen) {
        return VOS_FALSE;
    }

    msg = (const TAF_SSA_Evt*)msgHeader;
    ntf = (const TAF_SSA_LcsMolrNtf*)(msg->content);
    if (ntf->locationStrLen < sizeof(ntf->locationStr)) {
        expectedLen = minLen + sizeof(ntf->locationStr);
    } else {
        expectedLen = minLen + ntf->locationStrLen;
    }
    return TAF_ChkMsgLenWithExpectedLen(msgHeader, expectedLen);
}

VOS_VOID AT_RcvTafSsaEvt(TAF_SSA_Evt *event)
{
    MN_AT_IndEvt  *msg      = VOS_NULL_PTR;
    TAF_Ctrl      *ctrl     = VOS_NULL_PTR;
    AT_SS_EVT_FUNC evtFunc  = VOS_NULL_PTR;
    VOS_UINT8      indexNum = 0;
    VOS_UINT32     i;

    /* 判断事件是否是历史SSA代码上报 */
    if (event->evtExt != 0) {
        msg = (MN_AT_IndEvt *)event;
        At_SsMsgProc(msg->content, msg->len);
        return;
    }

    /* 初始化 */
    ctrl = (TAF_Ctrl *)(event->content);

    if (At_ClientIdToUserId(ctrl->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvTafSsaEvt: At_ClientIdToUserId FAILURE");
        return;
    }

    /* 在事件处理表中查找处理函数 */
    for (i = 0; i < AT_ARRAY_SIZE(g_atSsEvtFuncTbl); i++) {
        if (event->evtId == g_atSsEvtFuncTbl[i].evtId) {
            if (TAF_RunChkMsgLenFunc((const MSG_Header *)event, g_atSsEvtFuncTbl[i].msgLength,
                    g_atSsEvtFuncTbl[i].chkFunc) == VOS_FALSE) {
                AT_ERR_LOG("AT_RcvTafSsaEvt: Check MsgLength Err");
                return;
            }
            /* 事件ID匹配 */
            evtFunc = g_atSsEvtFuncTbl[i].evtFunc;
            break;
        }
    }

    /* 如果处理函数存在则调用 */
    if (evtFunc != VOS_NULL_PTR) {
        evtFunc(indexNum, (TAF_Ctrl *)event->content);
    } else {
        AT_ERR_LOG1("AT_RcvTafSsaEvt: Unexpected event received! <EvtId>", event->evtId);
    }
}

VOS_UINT32 AT_ProcMtaUnsolicitedRptQryCnf(VOS_UINT8 indexNum, MTA_AT_UnsolicitedRptQryCnf *qryUnsolicitedCnf)
{
    VOS_UINT32                   result = AT_OK;

    switch (qryUnsolicitedCnf->reqType) {
        /* TIME查询的处理 */
        case AT_MTA_QRY_TIME_RPT_TYPE:
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                qryUnsolicitedCnf->u.timeRptFlg);
            break;

        /* pstQryUnsolicitedCnf数据结构体修改 */
        /* CTZR查询的处理 */
        case AT_MTA_QRY_CTZR_RPT_TYPE:
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                qryUnsolicitedCnf->u.ctzrRptFlg);
            break;

        /* CSSN查询的处理 */
        case AT_MTA_QRY_CSSN_RPT_TYPE:
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                qryUnsolicitedCnf->u.cssnRptFlg.cssiRptFlg, qryUnsolicitedCnf->u.cssnRptFlg.cssuRptFlg);
            break;

        /* CUSD查询的处理 */
        case AT_MTA_QRY_CUSD_RPT_TYPE:
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                qryUnsolicitedCnf->u.cusdRptFlg);
            break;

        default:
            result = AT_CME_UNKNOWN;
    }

    return result;
}

VOS_UINT32 AT_RcvMtaQryUnsolicitedRptCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                  *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_UnsolicitedRptQryCnf *qryUnsolicitedCnf = (MTA_AT_UnsolicitedRptQryCnf *)mtaMsg->content;
    VOS_UINT32                   result;

    /* 格式化查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (qryUnsolicitedCnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_CME_UNKNOWN;
    } else {
        /* ucIndex参数带下来 */
        result = AT_ProcMtaUnsolicitedRptQryCnf(indexNum, qryUnsolicitedCnf);
    }

    return result;
}

VOS_UINT32 At_ProcReportUssdStr_Nontrans(TAF_SS_CallIndependentEvent *event, VOS_UINT16 printOffSet)
{
    TAF_SS_UssdString ussdStrBuff;
    MN_MSG_CbdcsCode  dcsInfo;
    VOS_UINT32        defAphaLen;
    VOS_UINT32        asciiStrLen;
    errno_t           memResult;
    VOS_UINT16        outPrintOffSet = 0;
    VOS_UINT32        ret;

    if (event->ussdString.cnt == 0) {
        AT_WARN_LOG("At_ProcReportUssdStr_Nontrans: UssdString Cnt is 0.");
        return outPrintOffSet;
    }

    memset_s(&dcsInfo, sizeof(dcsInfo), 0x00, sizeof(dcsInfo));

    /* USSD与CBS的DCS的协议相同，调用CBS的DCS解析函数解码，详细情况参考23038 */
    ret = MN_MSG_DecodeCbsDcs(event->dataCodingScheme, event->ussdString.ussdStr, event->ussdString.cnt, &dcsInfo);

    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("At_ProcReportUssdStr_Nontrans:WARNING: Decode Failure");
        return outPrintOffSet;
    }

    /* 先处理UCS2码流 */
    if (dcsInfo.msgCoding == MN_MSG_MSG_CODING_UCS2) {
        outPrintOffSet = (TAF_UINT16)At_Unicode2UnicodePrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                             g_atSndCodeAddress + printOffSet,
                                                             event->ussdString.ussdStr, event->ussdString.cnt);
    } else {
        /* 7Bit需要额外先解码，归一化到Ascii码流 */
        if (dcsInfo.msgCoding == MN_MSG_MSG_CODING_7_BIT) {
            memset_s(&ussdStrBuff, sizeof(TAF_SS_UssdString), 0, sizeof(TAF_SS_UssdString));

            defAphaLen = (VOS_UINT32)event->ussdString.cnt * 8 / 7;

            (VOS_VOID)TAF_STD_UnPack7Bit(event->ussdString.ussdStr, defAphaLen, 0, ussdStrBuff.ussdStr);

            if ((ussdStrBuff.ussdStr[defAphaLen - 1]) == 0x0d) {
                defAphaLen--;
            }

            asciiStrLen = 0;

            TAF_STD_ConvertDefAlphaToAscii(ussdStrBuff.ussdStr, defAphaLen, ussdStrBuff.ussdStr, &asciiStrLen);

            ussdStrBuff.cnt = (VOS_UINT16)asciiStrLen;
        }
        /* 其他情况:8Bit 直接拷贝 */
        else {
            memResult = memcpy_s(&ussdStrBuff, sizeof(TAF_SS_UssdString), &(event->ussdString),
                                 sizeof(TAF_SS_UssdString));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TAF_SS_UssdString), sizeof(TAF_SS_UssdString));
        }

        /* 非透传模式处理 */
        if (g_atCscsType == AT_CSCS_UCS2_CODE) { /* +CSCS:UCS2 */
            outPrintOffSet = (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                               g_atSndCodeAddress + printOffSet, ussdStrBuff.ussdStr,
                                                               ussdStrBuff.cnt);
        } else {
            if (ussdStrBuff.cnt > 0) {
                memResult = memcpy_s((TAF_CHAR *)g_atSndCodeAddress + printOffSet, ussdStrBuff.cnt, ussdStrBuff.ussdStr,
                                     ussdStrBuff.cnt);
                TAF_MEM_CHK_RTN_VAL(memResult, ussdStrBuff.cnt, ussdStrBuff.cnt);
            }
            outPrintOffSet = ussdStrBuff.cnt;
        }
    }

    return outPrintOffSet;
}

VOS_UINT16 AT_PrintUssdStr(TAF_SS_CallIndependentEvent *event, VOS_UINT8 indexNum, VOS_UINT16 length)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;
    VOS_UINT16     printOffSet;

    /* 没有USSD STRING需要打印 */
    /* 如果有USSD 字符串上报，必带DCS项 */
    if (event->opDataCodingScheme == 0) {
        AT_WARN_LOG("AT_PrintUssdStr: No DCS.");
        return length;
    }

    /* 如果主动上报的字符，放在USSDSting中 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        if (event->opUssdString == 0) {
            AT_WARN_LOG("AT_PrintUssdStr: BroadCast,No UssdString.");
            return length;
        }

    } else {
        /* 如果非主动上报的字符，可放在USSDSting中，也可放在USSData中 */
        /* 当网络29拒绝后，重发请求，网络的回复是放在USSData中 */
        if ((event->opUssdString == 0) && (event->opUSSData == 0)) {
            AT_WARN_LOG("AT_PrintUssdStr: No UssdSting & UssData.");
            return length;
        }
    }

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    printOffSet = length;
    printOffSet += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + printOffSet, ",\"");

    if (event->ussdString.cnt > sizeof(event->ussdString.ussdStr)) {
        AT_WARN_LOG1("AT_PrintUssdStr: Invalid pstEvent->UssdString.usCnt: ", event->ussdString.cnt);
        event->ussdString.cnt = sizeof(event->ussdString.ussdStr);
    }

    switch (ssCtx->ussdTransMode) {
        case AT_USSD_TRAN_MODE:
            printOffSet += (TAF_UINT16)At_HexString2AsciiNumPrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                                  g_atSndCodeAddress + printOffSet,
                                                                  event->ussdString.ussdStr, event->ussdString.cnt);
            break;

        case AT_USSD_NON_TRAN_MODE:
            /* 处理非透传模式下上报的7 8Bit UssdString */
            printOffSet += (TAF_UINT16)At_ProcReportUssdStr_Nontrans(event, printOffSet);
            break;

        default:
            break;
    }

    /* <dcs> */
    printOffSet += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + printOffSet, "\",%d", event->dataCodingScheme);

    return printOffSet;
}

TAF_VOID At_SsIndProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event)
{
    TAF_UINT16 length          = 0;
    TAF_UINT8  tmp             = 0;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    switch (event->ssEvent) {
        /* 其它事件 */
        case TAF_SS_EVT_USS_NOTIFY_IND:           /* 通知用户不用进一步操作 */
        case TAF_SS_EVT_USS_REQ_IND:              /* 通知用户进一步操作 */
        case TAF_SS_EVT_USS_RELEASE_COMPLETE_IND: /* 通知用户网络释放 */
        case TAF_SS_EVT_PROCESS_USS_REQ_CNF:
            /* <m> */
            if (event->ssEvent == TAF_SS_EVT_USS_NOTIFY_IND) {
                tmp = 0;
            } else if (event->ssEvent == TAF_SS_EVT_USS_REQ_IND) {
                tmp = 1;
            } else {
                if (*systemAppConfig == SYSTEM_APP_ANDROID) {
                    tmp = AT_PROCESS_USS_REQ_CNF_TMP;
                } else {
                    tmp = 0;
                }
            }
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s+CUSD: ", g_atCrLf);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d", tmp);
            /* <str> */

            length = AT_PrintUssdStr(event, indexNum, length);

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            At_SendResultData(indexNum, g_atSndCodeAddress, length);
            return;

        case TAF_SS_EVT_ERROR:
            if (event->errorCode == TAF_ERR_USSD_NET_TIMEOUT) {
                length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress, "%s+CUSD: %d%s", g_atCrLf, AT_CUSD_M_NETWORK_TIMEOUT, g_atCrLf);

                At_SendResultData(indexNum, g_atSndCodeAddress, length);

                return;
            }

            if (event->errorCode == TAF_ERR_USSD_USER_TIMEOUT) {
                length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress, "%s+CUSD: %d%s", g_atCrLf, AT_CUSD_M_NETWORK_CANCEL, g_atCrLf);

                At_SendResultData(indexNum, g_atSndCodeAddress, length);

                return;
            }

            break;

        default:
            return;
    }
}

TAF_UINT32 At_SsRspCusdProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event)
{
    TAF_UINT32 result;

    AT_StopTimerCmdReady(indexNum);

    if (event->ssEvent == TAF_SS_EVT_ERROR) {
        /* 本地发生错误: 清除+CUSD状态 */
        result = At_ChgTafErrorCode(indexNum, event->errorCode); /* 发生错误 */
    } else {
        /* 先报OK再发网络字符串 */
        result = AT_OK;
    }

    At_FormatResultData(indexNum, result);

    return result;
}

/*
 * 格式化输出+CCFC: <status>
 */
LOCAL VOS_VOID AT_ReportCcfcStatus(TAF_UINT32 tmp, TAF_SS_CallIndependentEvent *event, TAF_UINT16 *length)
{
    if (event->fwdFeaturelist.fwdFtr[tmp].opSsStatus == 1) {
        *length +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "%d",
                (TAF_SS_ACTIVE_STATUS_MASK & (event->fwdFeaturelist.fwdFtr[tmp].ssStatus)));
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "0");
    }
}
/*
 * 格式化输出+CCFC: <class1>
 */
LOCAL VOS_VOID AT_ReportCcfcClass1(TAF_UINT32 tmp, TAF_SS_CallIndependentEvent *event, TAF_UINT16 *length)
{
    if (event->fwdFeaturelist.fwdFtr[tmp].opBsService == 1) {
        *length +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d",
                At_GetClckClassFromBsCode(&(event->fwdFeaturelist.fwdFtr[tmp].bsService)));
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d", AT_CC_CALSS_TYPE_INVALID);
    }
}
/*
 * 格式化输出+CCFC: <type>
 */
LOCAL VOS_VOID AT_ReportCcfcType(TAF_UINT32 tmp, TAF_SS_CallIndependentEvent *event, TAF_UINT16 *length)
{
    if (event->fwdFeaturelist.fwdFtr[tmp].opNumType == 1) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d", event->fwdFeaturelist.fwdFtr[tmp].numType);
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d",
            At_GetCodeType(event->fwdFeaturelist.fwdFtr[tmp].fwdToNum[0]));
    }
}
/*
 * 格式化输出+CCFC: <subaddr><satype>
 */
LOCAL VOS_VOID AT_ReportCcfcSubAddrSatType(TAF_UINT32 tmp, TAF_SS_CallIndependentEvent *event, TAF_UINT16 *length)
{
    if (event->fwdFeaturelist.fwdFtr[tmp].opFwdToSubAddr == 1) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",\"%s\"",
            event->fwdFeaturelist.fwdFtr[tmp].fwdToSubAddr);

        /* <satype> */
        if (event->fwdFeaturelist.fwdFtr[tmp].opSubAddrType == 1) {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d",
                event->fwdFeaturelist.fwdFtr[tmp].subAddrType);
        } else {
            *length +=
                (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d",
                    At_GetCodeType(event->fwdFeaturelist.fwdFtr[tmp].fwdToSubAddr[0]));
        }
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",,");
    }
}
/*
 * 格式化输出+CCFC: <time>
 */
LOCAL VOS_VOID AT_ReportCcfcTime(TAF_UINT32 tmp, TAF_SS_CallIndependentEvent *event, TAF_UINT16 *length)
{
    if (event->fwdFeaturelist.fwdFtr[tmp].opNoRepCondTime == 1) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",%d",
            event->fwdFeaturelist.fwdFtr[tmp].noRepCondTime);
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, ",");
    }
}

TAF_UINT32 At_CcfcQryReport(TAF_SS_CallIndependentEvent *event, TAF_UINT8 indexNum)
{
    TAF_UINT32 tmp    = 0;
    TAF_UINT16 length = 0;

    /*
     * +CCFC: <status>,<class1>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]]
     */
    if (event->opSsStatus == 1) {
        /* +CCFC: <status>,<class1> */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d,%d", (TAF_SS_ACTIVE_STATUS_MASK & event->ssStatus),
            AT_CC_CALSS_TYPE_INVALID);

        return length;
    }

    if (event->opFwdFeaturelist == 1) {
        event->fwdFeaturelist.cnt = AT_MIN(event->fwdFeaturelist.cnt, TAF_SS_MAX_NUM_OF_FW_FEATURES);
        for (tmp = 0; tmp < event->fwdFeaturelist.cnt; tmp++) {
            if (tmp != 0) {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }

            /* +CCFC:  */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

            /* <status> */
            AT_ReportCcfcStatus(tmp, event, &length);

            /* <class1> */
            AT_ReportCcfcClass1(tmp, event, &length);

            /* <number> */
            if (event->fwdFeaturelist.fwdFtr[tmp].opFwdToNum == 1) {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, ",\"%s\"", event->fwdFeaturelist.fwdFtr[tmp].fwdToNum);

                /* <type> */
                AT_ReportCcfcType(tmp, event, &length);

                /* <subaddr> */
                AT_ReportCcfcSubAddrSatType(tmp, event, &length);

                /* <time> */
                AT_ReportCcfcTime(tmp, event, &length);
            }
        }
    }

    return length;
}

TAF_VOID At_SsRspInterrogateCnfClipProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    TAF_UINT8      tmp   = 0;
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* +CLIP: <n>,<m> */
    if (event->opSsStatus == 1) { /* 查到状态 */
        tmp = (TAF_SS_PROVISIONED_STATUS_MASK & event->ssStatus) ? 1 : 0;
    } else { /* 没有查到状态 */
        tmp = AT_CLI_SS_UNKNOWN_REASON;
    }

    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ssCtx->clipType, tmp);

    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCnfColpProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    TAF_UINT8      tmp   = 0;
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (event->opSsStatus == 1) { /* 查到状态 */
        tmp = (TAF_SS_ACTIVE_STATUS_MASK & event->ssStatus);
    } else { /* 没有查到状态 */
        tmp = AT_CLI_SS_UNKNOWN_REASON;
    }

    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ssCtx->colpType, tmp);

    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCnfClirProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    TAF_UINT8                    tmp = 0;
    TAF_SS_CLI_RESTRICION_OPTION clirTmp;
    TAF_UINT8                    cliSsStatus;
    AT_ModemSsCtx               *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (event->opGenericServiceInfo == 1) { /* 查到状态 */
        cliSsStatus = TAF_SS_ACTIVE_STATUS_MASK & event->genericServiceInfo.ssStatus;
        if (cliSsStatus) {
            if (event->genericServiceInfo.opCliStrictOp == 1) {
                clirTmp = event->genericServiceInfo.cliRestrictionOp;
                if (clirTmp == TAF_SS_CLI_PERMANENT) {
                    tmp = AT_CLIR_SS_PERMANENT;
                } else if (clirTmp == TAF_SS_CLI_TMP_DEFAULT_RESTRICTED) {
                    tmp = AT_CLIR_SS_TMP_DEFAULT_RESTRICTED;
                } else if (clirTmp == TAF_SS_CLI_TMP_DEFAULT_ALLOWED) {
                    tmp = AT_CLIR_SS_TMP_DEFAULT_ALLOWED;
                } else {
                    tmp = AT_CLIR_SS_UNKNOWN_REASON;
                }
            } else {
                tmp = AT_CLIR_SS_UNKNOWN_REASON;
            }
        } else {
            tmp = AT_CLIR_SS_PROVIDED;
        }
    } else if (event->opSsStatus == 1) {
        tmp = AT_CLIR_SS_PROVIDED;
    } else { /* 没有查到状态 */
        tmp = AT_CLIR_SS_UNKNOWN_REASON;
    }

    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ssCtx->clirType, tmp);

    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCnfClckProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    TAF_UINT8  tmp = 0;
    TAF_UINT32 i;
    VOS_UINT32 customizeFlag;
    VOS_UINT32 successFlg;

    /* +CLCK: <status>,<class1> */
    if (event->opError == 1) {                                    /* 需要首先判断错误码 */
        *result = At_ChgTafErrorCode(indexNum, event->errorCode); /* 发生错误 */
        return;
    }

    if (event->opSsStatus == 1) { /* 查到状态 */
        tmp = (TAF_SS_ACTIVE_STATUS_MASK & event->ssStatus);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, tmp,
            AT_CLCK_PARA_CLASS_ALL);

        /* 输出网络IE SS-STATUS值给用户 */
        customizeFlag = AT_GetSsCustomizePara(AT_SS_CUSTOMIZE_CLCK_QUERY);
        if (customizeFlag == VOS_TRUE) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", event->ssStatus);
        }
    } else if (event->opBsServGroupList == 1) {
        successFlg = VOS_FALSE;
        event->bsServGroupList.cnt = AT_MIN(event->bsServGroupList.cnt, TAF_SS_MAX_NUM_OF_BASIC_SERVICES);
        for (i = 0; i < event->bsServGroupList.cnt; i++) {
            /* 此处用ucTmp保存class，而不是status参数 */
            tmp = At_GetClckClassFromBsCode(&event->bsServGroupList.bsService[i]);
            if (tmp != AT_UNKNOWN_CLCK_CLASS) {
                successFlg = VOS_TRUE;
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d%s",
                    g_parseContext[indexNum].cmdElement->cmdName, 1, tmp, g_atCrLf);
            }
        }

        if (successFlg == VOS_TRUE) {
            *length -= (TAF_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
            (VOS_VOID)memset_s((VOS_UINT8 *)g_atSndCodeAddress + *length, AT_CMD_MAX_LEN - *length, 0x0,
                               AT_CMD_MAX_LEN - *length);
        }

        if (successFlg == VOS_FALSE) {
            AT_WARN_LOG("+CLCK - Unknown class.");
            *result = AT_ERROR;
            return;
        }

    } else { /* 没有查到状态 */
        tmp = 0;
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, tmp);
    }

    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCnfCcwaProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    TAF_UINT8  tmp = 0;
    TAF_UINT32 i;
    VOS_UINT32 customizeFlag;
    VOS_UINT32 successFlg;

    /* +CCWA: <status>,<class1> */
    if (event->opError == 1) {                                    /* 需要首先判断错误码 */
        *result = At_ChgTafErrorCode(indexNum, event->errorCode); /* 发生错误 */
        return;
    }

    if (event->opSsStatus == 1) {
        /* 状态为激活 */
        tmp = (TAF_SS_ACTIVE_STATUS_MASK & event->ssStatus);
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, tmp,
            AT_CLCK_PARA_CLASS_ALL);

        /* 输出网络IE SS-STATUS值给用户 */
        customizeFlag = AT_GetSsCustomizePara(AT_SS_CUSTOMIZE_CCWA_QUERY);
        if (customizeFlag == VOS_TRUE) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, ",%d", event->ssStatus);
        }
    } else if (event->opBsServGroupList == 1) {
        successFlg = VOS_FALSE;
        event->bsServGroupList.cnt = AT_MIN(event->bsServGroupList.cnt, TAF_SS_MAX_NUM_OF_BASIC_SERVICES);
        for (i = 0; i < event->bsServGroupList.cnt; i++) {
            /* 此处用ucTmp保存class，而不是status参数 */
            tmp = At_GetClckClassFromBsCode(&event->bsServGroupList.bsService[i]);
            if (tmp != AT_UNKNOWN_CLCK_CLASS) {
                successFlg = VOS_TRUE;
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d%s",
                    g_parseContext[indexNum].cmdElement->cmdName, 1, tmp, g_atCrLf);
            }
        }

        if (successFlg == VOS_TRUE) {
            *length -= (TAF_UINT16)VOS_StrLen((VOS_CHAR *)g_atCrLf);
            (VOS_VOID)memset_s((VOS_UINT8 *)g_atSndCodeAddress + *length, AT_CMD_MAX_LEN - *length, 0x0,
                               AT_CMD_MAX_LEN - *length);
        }

        if (successFlg == VOS_FALSE) {
            AT_WARN_LOG("+CCWA - Unknown class.");
            *result = AT_ERROR;
            return;
        }
    } else { /* 状态为未激活 */
        tmp = 0;
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, tmp);
    }

    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCcbsCnfProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    VOS_UINT32 i = 0;

    if (event->opGenericServiceInfo == 1) {
        if (TAF_SS_PROVISIONED_STATUS_MASK & event->genericServiceInfo.ssStatus) {
            if (event->genericServiceInfo.opCcbsFeatureList == 1) {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "Queue of Ccbs requests is: ");
                event->genericServiceInfo.ccbsFeatureList.cnt = AT_MIN(event->genericServiceInfo.ccbsFeatureList.cnt,
                                                                       TAF_SS_MAX_NUM_OF_CCBS_FEATURE);
                for (i = 0; i < event->genericServiceInfo.ccbsFeatureList.cnt; i++) {
                    if (event->genericServiceInfo.ccbsFeatureList.ccBsFeature[i].opCcbsIndex == VOS_TRUE) {
                        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                            (TAF_CHAR *)g_atSndCodeAddress + *length, "%s", g_atCrLf);
                        *length +=
                            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                                (TAF_CHAR *)g_atSndCodeAddress + *length, "Index:%d",
                                event->genericServiceInfo.ccbsFeatureList.ccBsFeature[i].ccbsIndex);
                    }
                }
            } else {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "Queue of Ccbs is empty");
            }
        } else {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "CCBS not provisioned");
        }
    } else if (event->opSsStatus == 1) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "CCBS not provisioned");
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "Unknown");
    }
    *result = AT_OK;
}
/*
 * Description: SS中查询结果针对Clip的上报处理
 */
LOCAL VOS_VOID AT_SsRspInterrogateCnfCmmiClipProc(TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
    TAF_UINT16 *length)
{
    if ((event->opSsStatus == 1) && (TAF_SS_PROVISIONED_STATUS_MASK & event->ssStatus)) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "CLIP provisioned");
    } else if (event->opSsStatus == 0) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "Unknown");
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "CLIP not provisioned");
    }

    *result = AT_OK;
}

LOCAL VOS_VOID AT_SsRspInterrogateCnfCmmiCliRestrictPrint(TAF_SS_CLI_RESTRICION_OPTION cliRestrictionOp,
    TAF_UINT16 *length)
{
    switch (cliRestrictionOp) {
        case TAF_SS_CLI_PERMANENT:
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + *length,
                "CLIR provisioned in permanent mode");
            break;

        case TAF_SS_CLI_TMP_DEFAULT_RESTRICTED:
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + *length,
                "CLIR temporary mode presentation restricted");
            break;

        case TAF_SS_CLI_TMP_DEFAULT_ALLOWED:
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + *length,
                "CLIR temporary mode presentation allowed");
            break;

        default:
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)g_atSndCodeAddress + *length, "Unknown");
            break;
    }
}
LOCAL VOS_VOID AT_SsRspInterrogateCnfCmmiClirProc(TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
    TAF_UINT16 *length)
{
    if (event->opGenericServiceInfo == 1) {
        if (TAF_SS_PROVISIONED_STATUS_MASK & event->genericServiceInfo.ssStatus) {
            if (event->genericServiceInfo.opCliStrictOp == 1) {
                AT_SsRspInterrogateCnfCmmiCliRestrictPrint(event->genericServiceInfo.cliRestrictionOp, length);
            } else {
                *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + *length, "Unknown");
            }
        } else {
            *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + *length, "CLIR not provisioned");
        }
    } else if (event->opSsStatus == 1) {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "CLIR not provisioned");
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "Unknown");
    }
    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCnfCmmiProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    if (event->opError == 1) {                                    /* 需要首先判断错误码 */
        *result = At_ChgTafErrorCode(indexNum, event->errorCode); /* 发生错误 */
        return;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMMI_QUERY_CLIP) {
        AT_SsRspInterrogateCnfCmmiClipProc(event, result, length);
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMMI_QUERY_CLIR) {
        AT_SsRspInterrogateCnfCmmiClirProc(event, result, length);
    } else if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_SS_INTERROGATE_CCBS) {
        At_SsRspInterrogateCcbsCnfProc(indexNum, event, result, length);
    } else {
        *result = AT_ERROR;
    }
}

TAF_VOID AT_SsRspInterrogateCnfCnapProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                        TAF_UINT16 *length)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;
    TAF_UINT8      tmp   = 0;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 需要首先判断错误码 */
    if (event->opError == 1) {
        *result = At_ChgTafErrorCode(indexNum, event->errorCode);
        return;
    }

    /* 判断当前操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CNAP_QRY) {
        AT_WARN_LOG("AT_SsRspInterrogateCnfCnapProc: WARNING: Not AT_CMD_CNAP_QRY!");
        return;
    }

    /* 查到状态 */
    if (event->opSsStatus == 1) {
        tmp = (TAF_SS_PROVISIONED_STATUS_MASK & event->ssStatus) ? 1 : 0;
    }
    /* 没有查到状态 */
    else {
        tmp = AT_CMD_CNAP_NO_OP_SS_STATUS;
    }

    /* +CNAP: <n>,<m> */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        ssCtx->cnapType, tmp);

    *result = AT_OK;
}

TAF_VOID At_SsRspInterrogateCnfProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT32 *result,
                                    TAF_UINT16 *length)
{
    if (g_parseContext[indexNum].cmdElement == VOS_NULL_PTR) {
        return;
    }
    switch (g_parseContext[indexNum].cmdElement->cmdIndex) {
        case AT_CMD_CLIP:
            At_SsRspInterrogateCnfClipProc(indexNum, event, result, length);
            break;

        case AT_CMD_COLP:
            At_SsRspInterrogateCnfColpProc(indexNum, event, result, length);
            break;

        case AT_CMD_CLIR:
            At_SsRspInterrogateCnfClirProc(indexNum, event, result, length);
            break;

        case AT_CMD_CLCK:
            At_SsRspInterrogateCnfClckProc(indexNum, event, result, length);
            break;

        case AT_CMD_CCWA:
            At_SsRspInterrogateCnfCcwaProc(indexNum, event, result, length);
            break;

        case AT_CMD_CCFC:
            /* +CCFC: <status>,<class1>[,<number>,<type>[,<subaddr>,<satype>[,<time>]]] */
            if (event->opError == 1) {                                    /* 需要首先判断错误码 */
                *result = At_ChgTafErrorCode(indexNum, event->errorCode); /* 发生错误 */
                break;
            }

            *length = (TAF_UINT16)At_CcfcQryReport(event, indexNum);
            *result = AT_OK;
            break;

        case AT_CMD_CMMI:
            At_SsRspInterrogateCnfCmmiProc(indexNum, event, result, length);
            break;

        case AT_CMD_CNAP:
            AT_SsRspInterrogateCnfCnapProc(indexNum, event, result, length);
            break;

        /* 直接返回 防止错误清除其他命令处理状态 */
        default:
            return;
    }
    AT_StopTimerCmdReady(indexNum);
}


TAF_VOID At_SsRspUssdProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event, TAF_UINT16 *length)
{
    TAF_UINT8  tmp             = 0;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* <m> 通知用户不用进一步操作 */
    if (event->ssEvent == TAF_SS_EVT_USS_NOTIFY_IND) {
        tmp = 0;
    /* 通知用户进一步操作 */
    } else if (event->ssEvent == TAF_SS_EVT_USS_REQ_IND) {
        tmp = 1;
    } else {
        if (*systemAppConfig == SYSTEM_APP_ANDROID) {
            tmp = AT_PROCESS_USS_REQ_CNF_TMP;
        } else if ((event->opUssdString == 0) && (event->opUSSData == 0)) {
            tmp = AT_PROCESS_USS_REQ_CNF_TMP;
        } else {
            tmp = 0;
        }
    }

    /* +CUSD: <m>[,<str>,<dcs>] */
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "+CUSD: %d", tmp);

    /* <str> */
    /* 部分判断移到函数中了 */
    if (event->opError == 0) {
        *length = AT_PrintUssdStr(event, indexNum, *length);
    }
}

VOS_UINT32 AT_GetSsEventErrorCode(VOS_UINT8 indexNum, TAF_SS_CallIndependentEvent *event)
{
    if (event->opSsStatus == VOS_TRUE) {
        if ((TAF_SS_PROVISIONED_STATUS_MASK & event->ssStatus) == 0) {
            /* 返回业务未签约对应的错误码 */
            return AT_CME_SERVICE_NOT_PROVISIONED;
        }
    }

    return At_ChgTafErrorCode(indexNum, event->errorCode);
}

TAF_VOID At_SsRspProc(TAF_UINT8 indexNum, TAF_SS_CallIndependentEvent *event)
{
    TAF_UINT32 result = AT_FAILURE;
    TAF_UINT16 length = 0;

    /* CLIP CCWA CCFC CLCK CUSD CPWD */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CUSD_REQ) {
        (VOS_VOID)At_SsRspCusdProc(indexNum, event);
        return;
    }

    if (event->ssEvent == TAF_SS_EVT_ERROR) { /* 如果是ERROR事件，则直接判断错误码 */
        if (event->errorCode == TAF_ERR_USSD_NET_TIMEOUT) {
            length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s+CUSD: %d%s", g_atCrLf, AT_CUSD_M_NETWORK_TIMEOUT, g_atCrLf);

            At_SendResultData(indexNum, g_atSndCodeAddress, length);

            return;
        }

        if (event->errorCode == TAF_ERR_USSD_USER_TIMEOUT) {
            length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s+CUSD: %d%s", g_atCrLf, AT_CUSD_M_NETWORK_CANCEL, g_atCrLf);

            At_SendResultData(indexNum, g_atSndCodeAddress, length);

            return;
        }

        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CURRENT_OPT_BUTT) {
            return;
        }

        /* 从SS Event中获取用于AT返回的错误码 */
        result = AT_GetSsEventErrorCode(indexNum, event);

        AT_StopTimerCmdReady(indexNum);
    } else {
        switch (event->ssEvent) {
            /* 其它事件 */
            case TAF_SS_EVT_INTERROGATESS_CNF: /* 查询结果上报 */
                At_SsRspInterrogateCnfProc(indexNum, event, &result, &length);
                break;

            case TAF_SS_EVT_ERASESS_CNF:
            case TAF_SS_EVT_REGISTERSS_CNF:
            case TAF_SS_EVT_ACTIVATESS_CNF:
            case TAF_SS_EVT_DEACTIVATESS_CNF:
            case TAF_SS_EVT_REG_PASSWORD_CNF:
            case TAF_SS_EVT_ERASE_CC_ENTRY_CNF:
                if (event->opError == 0) {
                    result = AT_OK;
                } else {
                    result = At_ChgTafErrorCode(indexNum, event->errorCode); /* 发生错误 */
                }
                AT_StopTimerCmdReady(indexNum);
                break;

            case TAF_SS_EVT_USS_NOTIFY_IND:           /* 通知用户不用进一步操作 */
            case TAF_SS_EVT_USS_REQ_IND:              /* 通知用户进一步操作 */
            case TAF_SS_EVT_PROCESS_USS_REQ_CNF:      /* 通知用户网络释放 */
            case TAF_SS_EVT_USS_RELEASE_COMPLETE_IND: /* 通知用户网络释放 */
                At_SsRspUssdProc(indexNum, event, &length);
                break;

            /* Delete TAF_SS_EVT_GET_PASSWORD_IND分支 */
            default:
                return;
        }
    }

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

TAF_VOID At_SsMsgProc(TAF_UINT8 *data, TAF_UINT16 len)
{
    errno_t                      memResult;
    TAF_SS_CallIndependentEvent *event    = TAF_NULL_PTR;
    TAF_UINT8                    indexNum = 0;
    /*lint -save -e830 */
    event = (TAF_SS_CallIndependentEvent *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_SS_CallIndependentEvent));
    /*lint -restore */
    if (event == TAF_NULL_PTR) {
        AT_WARN_LOG("At_SsMsgProc Mem Alloc FAILURE");
        return;
    }
    memset_s(event, sizeof(TAF_SS_CallIndependentEvent), 0x00, sizeof(TAF_SS_CallIndependentEvent));

    if (len > sizeof(TAF_SS_CallIndependentEvent)) {
        AT_WARN_LOG1("At_SsMsgProc: Invalid Para usLen: ", len);
        len = sizeof(TAF_SS_CallIndependentEvent);
    }

    if (len > 0) {
        memResult = memcpy_s(event, sizeof(TAF_SS_CallIndependentEvent), data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(TAF_SS_CallIndependentEvent), len);
    }

    AT_LOG1("At_SsMsgProc pEvent->ClientId", event->clientId);
    AT_LOG1("At_SsMsgProc pEvent->SsEvent", event->ssEvent);
    AT_LOG1("At_SsMsgProc pEvent->OP_Error", event->opError);
    AT_LOG1("At_SsMsgProc pEvent->ErrorCode", event->errorCode);
    AT_LOG1("At_SsMsgProc pEvent->SsCode", event->ssCode);
    AT_LOG1("At_SsMsgProc pEvent->Cause", event->cause);

    if (At_ClientIdToUserId(event->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_SsMsgProc At_ClientIdToUserId FAILURE");
        /*lint -save -e830 */
        PS_MEM_FREE(WUEPS_PID_AT, event);
        /*lint -restore */
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        At_SsIndProc(indexNum, event);
    } else {
        AT_LOG1("At_SsMsgProc ucIndex", indexNum);
        AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

        At_SsRspProc(indexNum, event);
    }

    PS_MEM_FREE(WUEPS_PID_AT, event);
}

VOS_UINT32 AT_RcvTafCcmCnapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CnapQryCnf *cnapQryCnf = VOS_NULL_PTR;
    VOS_UINT16          length = 0;
    VOS_UINT8           nameStr[AT_PARA_CNAP_MAX_NAME_LEN];

    cnapQryCnf = (TAF_CCM_CnapQryCnf *)msg;

    if (cnapQryCnf->nameIndicator.existFlag == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: \"", g_parseContext[indexNum].cmdElement->cmdName);

        if ((cnapQryCnf->nameIndicator.length > 0) &&
            (cnapQryCnf->nameIndicator.length < TAF_CALL_CNAP_NAME_STR_MAX_LENGTH) &&
            (cnapQryCnf->nameIndicator.dcs < TAF_CALL_DCS_BUTT)) {
            /* UCS2格式直接打印输出 */
            if (cnapQryCnf->nameIndicator.dcs == TAF_CALL_DCS_UCS2) {
                if ((cnapQryCnf->nameIndicator.length * AT_DOUBLE_LENGTH) < TAF_CALL_CNAP_NAME_STR_MAX_LENGTH) {
                    length += (VOS_UINT16)At_Unicode2UnicodePrint(AT_CMD_MAX_LEN, (VOS_INT8 *)g_atSndCodeAddress,
                                                              g_atSndCodeAddress + length,
                                                              cnapQryCnf->nameIndicator.nameStr,
                                                              cnapQryCnf->nameIndicator.length * AT_DOUBLE_LENGTH);
                }
            } else {
                AT_CnapConvertNameStr(&cnapQryCnf->nameIndicator, nameStr, AT_PARA_CNAP_MAX_NAME_LEN);

                /* AscII转换为UCS2并打印输出 */
                length += (TAF_UINT16)At_Ascii2UnicodePrint(AT_CMD_MAX_LEN, (VOS_INT8 *)g_atSndCodeAddress,
                                                            g_atSndCodeAddress + length, nameStr,
                                                            (VOS_UINT16)VOS_StrLen((VOS_CHAR *)nameStr));
            }
        }

        /* <CNI_Validity> */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\",%d", cnapQryCnf->nameIndicator.cniValidity);
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmSetAlsCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SetAlsCnf *setAlsCnf = (TAF_CCM_SetAlsCnf *)msg;
    VOS_UINT32         result = AT_OK;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafCcmSetAlsCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    g_atSendDataBuff.bufLen = 0;

    if (setAlsCnf->ret != TAF_ERR_NO_ERROR) {
        result = AT_CME_UNKNOWN;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvTafCcmQryAlsCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_QryAlsCnf *alsCnf   = VOS_NULL_PTR;
    VOS_UINT32         result;
    VOS_UINT16         len;
    AT_ModemSsCtx     *modemSsCtx = VOS_NULL_PTR;

    alsCnf = (TAF_CCM_QryAlsCnf *)msg;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafCcmQryAlsCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    len                     = 0;
    result                  = AT_ERROR;
    g_atSendDataBuff.bufLen = 0;

    modemSsCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    if (alsCnf->qryAlsPara.ret == TAF_ERR_NO_ERROR) {
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            modemSsCtx->salsType);

        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, ",%d", alsCnf->qryAlsPara.alsLine);

        g_atSendDataBuff.bufLen = len;

        result = AT_OK;
    } else {
        /* 当不支持ALS特性时，由call上报错误时间，AT返回error，AT不区分是否支持该特性 */
        result = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 At_RcvTafCcmCallModifyCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallModifyCnf *modifyCnf = (TAF_CCM_CallModifyCnf *)msg;

    /* 判断操作是否成功 */
    if (modifyCnf->cause != TAF_CS_CAUSE_SUCCESS) {
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}

VOS_UINT32 At_RcvTafCcmCallAnswerRemoteModifyCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallAnswerRemoteModifyCnf *modifyCnf = msg;

    /* 判断操作是否成功 */
    if (modifyCnf->cause != TAF_CS_CAUSE_SUCCESS) {
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}
#endif

VOS_VOID AT_RcvSsaSetLcsMolrCnf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_SetLcsMolrCnf *setLcsMolrCnf = VOS_NULL_PTR;
    VOS_UINT32             result;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMolrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* 判断当前操作类型是否为AT_CMD_CMOLR_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMOLR_SET) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMolrCnf: WARNING:Not AT_CMD_CMOLR_SET!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    setLcsMolrCnf = (TAF_SSA_SetLcsMolrCnf *)event;
    result        = AT_ConvertTafSsaErrorCode(indexNum, setLcsMolrCnf->result);

    if (result != AT_OK) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMolrCnf: WARNING:Set ^CMOLR Failed!");
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_RcvSsaGetLcsMtlraCnf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_GetLcsMtlraCnf *getLcsMtlraCnf = VOS_NULL_PTR;
    VOS_UINT32              result;
    VOS_UINT16              length;
    VOS_UINT32              i;
    VOS_UINT8               cnt;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlraCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* 判断当前操作类型是否为AT_CMD_CMTLRA_READ */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMTLRA_READ) {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlraCnf: WARNING:Not AT_CMD_CMTLRA_READ!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    getLcsMtlraCnf = (TAF_SSA_GetLcsMtlraCnf *)event;
    result         = AT_ConvertTafSsaErrorCode(indexNum, getLcsMtlraCnf->result);
    length         = 0;

    /* 判断查询操作是否成功 */
    if (result == AT_OK) {
        cnt = (getLcsMtlraCnf->cnt > TAF_SSA_LCS_MTLR_MAX_NUM) ? TAF_SSA_LCS_MTLR_MAX_NUM : getLcsMtlraCnf->cnt;
        for (i = 0; i < cnt; i++) {
            if (i != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", (VOS_CHAR *)g_atCrLf);
            }

            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                getLcsMtlraCnf->allow[i], getLcsMtlraCnf->handleId[i]);
        }
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_RcvSsaSetLcsMtlraCnf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_SetLcsMtlraCnf *setLcsMtlraCnf = VOS_NULL_PTR;
    VOS_UINT32              result;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlraCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* 判断当前操作类型是否为AT_CMD_CMTLRA_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMTLRA_SET) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlraCnf: WARNING:Not AT_CMD_CMTLRA_SET!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    setLcsMtlraCnf = (TAF_SSA_SetLcsMtlraCnf *)event;
    result         = AT_ConvertTafSsaErrorCode(indexNum, setLcsMtlraCnf->result);

    if (result != AT_OK) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlraCnf: WARNING:Set ^CMTLRA Failed!");
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_RcvSsaGetLcsMtlrCnf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_GetLcsMtlrCnf *getLcsMtlrCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT16             length;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* 判断当前操作类型是否为AT_CMD_CMTLR_READ */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMTLR_READ) {
        AT_WARN_LOG("AT_RcvSsaGetLcsMtlrCnf: WARNING:Not AT_CMD_CMTLR_READ!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    getLcsMtlrCnf = (TAF_SSA_GetLcsMtlrCnf *)event;
    result        = AT_ConvertTafSsaErrorCode(indexNum, getLcsMtlrCnf->result);
    length        = 0;

    /* 判断查询操作是否成功 */
    if (result == AT_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            getLcsMtlrCnf->subscribe);
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_RcvSsaSetLcsMtlrCnf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_SetLcsMtlrCnf *setLcsMtlrCnf = VOS_NULL_PTR;
    VOS_UINT32             result;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* 判断当前操作类型是否为AT_CMD_CMTLR_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMTLR_SET) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlrCnf: WARNING:Not AT_CMD_CMTLR_SET!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    setLcsMtlrCnf = (TAF_SSA_SetLcsMtlrCnf *)event;
    result        = AT_ConvertTafSsaErrorCode(indexNum, setLcsMtlrCnf->result);

    if (result != AT_OK) {
        AT_WARN_LOG("AT_RcvSsaSetLcsMtlrCnf: WARNING:Set ^CMTLR Failed!");
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
}

/*
 * 功能描述: 打印LCS特性MO-LR流程参数NMEA-rep
 */
VOS_VOID AT_PrintLcsMolrParaNmeaRep(TAF_SSA_GetLcsMolrSnf *getLcsMolrCnf, VOS_UINT16 *length)
{
    VOS_BOOL               bNmeaFlg      = VOS_FALSE;

    if ((getLcsMolrCnf->enable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA) ||
        (getLcsMolrCnf->enable == TAF_SSA_LCS_MOLR_ENABLE_TYPE_NMEA_GAD)) {
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"");

        if (getLcsMolrCnf->nmeaRep.gpgga == VOS_TRUE) {
            bNmeaFlg = VOS_TRUE;
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s,", AT_PARA_NMEA_GPGGA);
        }

        if (getLcsMolrCnf->nmeaRep.gprmc == VOS_TRUE) {
            bNmeaFlg = VOS_TRUE;
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s,", AT_PARA_NMEA_GPRMC);
        }

        if (getLcsMolrCnf->nmeaRep.gpgll == VOS_TRUE) {
            bNmeaFlg = VOS_TRUE;
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "%s,", AT_PARA_NMEA_GPGLL);
        }

        /* 删除多打印的一个字符 */
        *length -= 1;

        if (bNmeaFlg == VOS_TRUE) {
            *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *length, "\"");
        }
    }
}
/*
 * 功能描述: 打印ThirdPartyAddr
 */
VOS_VOID AT_PrintLcsMolrParaThirdPartyAddr(TAF_SSA_GetLcsMolrSnf *getLcsMolrCnf, VOS_UINT16 *length)
{
    VOS_UINT8              tPAStr[LCS_CLIENT_EXID_MAX_LEN + 1];
    errno_t                memResult;

    if (((getLcsMolrCnf->molrPara.method == LCS_MOLR_METHOD_TRANSFER_TP_ADDR) ||
         (getLcsMolrCnf->molrPara.method == LCS_MOLR_METHOD_RETRIEVAL_TP_ADDR)) &&
        ((getLcsMolrCnf->molrPara.tPAddr.length > 0) &&
         (getLcsMolrCnf->molrPara.tPAddr.length <= LCS_CLIENT_EXID_MAX_LEN))) {
        memResult = memset_s(tPAStr, sizeof(tPAStr), 0x00, LCS_CLIENT_EXID_MAX_LEN + 1);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tPAStr), LCS_CLIENT_EXID_MAX_LEN + 1);
        memResult = memcpy_s(tPAStr, sizeof(tPAStr), getLcsMolrCnf->molrPara.tPAddr.value,
                             getLcsMolrCnf->molrPara.tPAddr.length);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(tPAStr), getLcsMolrCnf->molrPara.tPAddr.length);

        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%s\"", tPAStr);
    }
}

VOS_VOID AT_RcvSsaGetLcsMolrCnf(VOS_UINT8 indexNum, TAF_Ctrl *event)
{
    TAF_SSA_GetLcsMolrSnf *getLcsMolrCnf = VOS_NULL_PTR;
    VOS_UINT32             result;
    VOS_UINT16             length        = 0;

    /* 不能为广播通道 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvSsaGetLcsMolrCnf: WARNING:AT_BROADCAST_INDEX!");
        return;
    }

    /* 判断当前操作类型是否为AT_CMD_CMOLR_READ */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMOLR_READ) {
        AT_WARN_LOG("AT_RcvSsaGetLcsMolrCnf: WARNING:Not AT_CMD_CMOLR_READ!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    getLcsMolrCnf = (TAF_SSA_GetLcsMolrSnf *)event;
    result        = AT_ConvertTafSsaErrorCode(indexNum, getLcsMolrCnf->result);

    /* 判断查询操作是否成功 */
    if (result == AT_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,", g_parseContext[indexNum].cmdElement->cmdName,
            getLcsMolrCnf->enable, getLcsMolrCnf->molrPara.method, getLcsMolrCnf->molrPara.horAccSet);

        if (getLcsMolrCnf->molrPara.horAccSet == LCS_HOR_ACC_SET_PARAM) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d", getLcsMolrCnf->molrPara.horAcc);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,", getLcsMolrCnf->molrPara.verReq);

        if (getLcsMolrCnf->molrPara.verReq == LCS_VER_REQUESTED) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", getLcsMolrCnf->molrPara.verAccSet);

            if (getLcsMolrCnf->molrPara.verAccSet == LCS_VER_ACC_SET_PARAM) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%d", getLcsMolrCnf->molrPara.verAcc);
            }
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,%d,", getLcsMolrCnf->molrPara.velReq,
            getLcsMolrCnf->molrPara.repMode, getLcsMolrCnf->molrPara.timeOut);

        if (getLcsMolrCnf->molrPara.repMode == LCS_REP_MODE_PERIODIC_RPT) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d", getLcsMolrCnf->molrPara.interval);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,", getLcsMolrCnf->molrPara.u.shapeRep,
            getLcsMolrCnf->plane);

        AT_PrintLcsMolrParaNmeaRep(getLcsMolrCnf, &length);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");

        AT_PrintLcsMolrParaThirdPartyAddr(getLcsMolrCnf, &length);
    }

    /* 输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_UINT32 AT_RcvTafCcmQryUus1InfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_QryUus1InfoCnf *uus1Cnf  = VOS_NULL_PTR;
    VOS_UINT32              result;
    VOS_UINT32              i;
    VOS_UINT16              len;
    VOS_UINT32              uus1IFlg;
    VOS_UINT32              uus1UFlg;

    uus1Cnf = (TAF_CCM_QryUus1InfoCnf *)msg;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafCcmQryUus1InfoCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    len                     = 0;
    result                  = AT_ERROR;
    g_atSendDataBuff.bufLen = 0;

    /* 变量初始化为打开主动上报 */
    uus1IFlg = VOS_TRUE;
    uus1UFlg = VOS_TRUE;

    /* UUS1I是否打开 */
    if (uus1Cnf->qryUss1Info.setType[0] == MN_CALL_CUUS1_DISABLE) {
        uus1IFlg = VOS_FALSE;
    }

    /* UUS1U是否打开 */
    if (uus1Cnf->qryUss1Info.setType[1] == MN_CALL_CUUS1_DISABLE) {
        uus1UFlg = VOS_FALSE;
    }

    if (uus1Cnf->qryUss1Info.ret == TAF_ERR_NO_ERROR) {
        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s", g_atCrLf);

        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + len, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)(g_atSndCodeAddress + len), "%d,%d", uus1IFlg, uus1UFlg);

        uus1Cnf->qryUss1Info.actNum = AT_MIN(uus1Cnf->qryUss1Info.actNum, MN_CALL_MAX_UUS1_MSG_NUM);
        for (i = 0; i < uus1Cnf->qryUss1Info.actNum; i++) {
            len += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)(g_atSndCodeAddress + len), ",%d,", uus1Cnf->qryUss1Info.uus1Info[i].msgType);

            len += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (VOS_INT8 *)g_atSndCodeAddress,
                                                       (VOS_UINT8 *)g_atSndCodeAddress + len,
                                                       uus1Cnf->qryUss1Info.uus1Info[i].uuie,
                                                       uus1Cnf->qryUss1Info.uus1Info[i].uuie[MN_CALL_LEN_POS] +
                                                           MN_CALL_UUIE_HEADER_LEN);
        }

        g_atSendDataBuff.bufLen = len;

        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvTafCcmSetUus1InfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SetUus1InfoCnf *uus1Cnf  = (TAF_CCM_SetUus1InfoCnf *)msg;
    VOS_UINT32              result;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvTafCcmSetUus1InfoCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (uus1Cnf->ret == TAF_ERR_NO_ERROR) {
        result = AT_OK;
    } else {
        result = AT_CME_INCORRECT_PARAMETERS;
    }

    g_atSendDataBuff.bufLen = 0;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

