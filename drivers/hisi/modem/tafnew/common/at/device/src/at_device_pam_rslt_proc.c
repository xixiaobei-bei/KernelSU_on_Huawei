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
#include "at_device_pam_rslt_proc.h"
#include "securec.h"

#include "taf_app_mma.h"
#include "at_ctx.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "at_event_report.h"
#include "at_oam_interface.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_PAM_RSLT_PROC_C

#define AT_SIMLOCKCODE_LAST2CHAR_LEN 2

#define AT_GET_EMAT_AT_CNF_LEN_WITHOUT_UNION \
    (sizeof(MN_APP_EmatAtCnf) - sizeof(((MN_APP_EmatAtCnf *)16)->ematAtCnf.ematEvent))

static const AT_EmatRspProcFunc g_atEmatRspProcFuncTbl[] = {
    { SI_EMAT_EVENT_ESIM_CLEAN_CNF, 0, VOS_NULL_PTR, At_PrintEsimCleanProfileInfo },
    { SI_EMAT_EVENT_ESIM_CHECK_CNF,  sizeof(SI_EMAT_EventEsimCheckCnf), VOS_NULL_PTR, At_PrintEsimCheckProfileInfo },
    { SI_EMAT_EVENT_GET_ESIMEID_CNF, sizeof(SI_EMAT_EventEsimEidCnf),   VOS_NULL_PTR, At_PrintGetEsimEidInfo },
    { SI_EMAT_EVENT_GET_ESIMPKID_CNF, sizeof(SI_EMAT_EventEsimPkidCnf), VOS_NULL_PTR, At_PrintGetEsimPKIDInfo },
};

VOS_UINT32 AT_RcvDrvAgentIdentifyStartSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                 *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_IdentifystartSetCnf *event  = VOS_NULL_PTR;
    VOS_UINT8                      indexNum;
    VOS_UINT32                     result;
    VOS_UINT16                     length;
    VOS_UINT32                     i;

    AT_PR_LOGI("enter");

    /* 初始化 */
    rcvMsg   = (DRV_AGENT_Msg *)msg;
    event    = (DRV_AGENT_IdentifystartSetCnf *)rcvMsg->content;
    indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    length   = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_IDENTIFYSTART_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        if (event->otaFlag == VOS_TRUE) {
            for (i = 0; i < TAF_OTA_SIMLOCK_PUBLIC_ID_NUM; i++) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "0x%X,", event->publicId[i]);
            }
        }
        for (i = 0; i < DRV_AGENT_RSA_CIPHERTEXT_LEN; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", event->rsaText[i]);
        }
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentHukSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg       *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_HukSetCnf *event  = VOS_NULL_PTR;
    VOS_UINT8            indexNum;
    VOS_UINT32           result;

    AT_PR_LOGI("enter");

    /* 初始化 */
    rcvMsg   = (DRV_AGENT_Msg *)msg;
    event    = (DRV_AGENT_HukSetCnf *)rcvMsg->content;
    indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    result   = VOS_NULL;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_HUK_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = 0;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSimlockDataReadExReadCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                      *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_SimlockdatareadexReadCnf *event  = VOS_NULL_PTR;
    VOS_UINT32                          result;
    VOS_UINT32                          loop;
    VOS_UINT16                          length;
    VOS_UINT8                           indexNum;

    AT_PR_LOGI("enter");

    /* 初始化消息变量 */
    length   = 0;
    indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    rcvMsg   = (DRV_AGENT_Msg *)msg;
    event    = (DRV_AGENT_SimlockdatareadexReadCnf *)rcvMsg->content;

    result = event->result;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_SIMLOCKDATAREADEX_READ_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SIMLOCKDATAREADEX_READ_SET) {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadExReadCnf: CmdCurrentOpt ERR.");
        return VOS_ERR;
    }

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;

        /* 添加<layer>,<index>,<total>打印 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d,%d,%d,", g_parseContext[indexNum].cmdElement->cmdName, event->layer,
            event->indexNum, event->total);

        /* 添加<simlock_data>打印 */
        for (loop = 0; loop < event->dataLen; loop++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02x", event->data[loop]);
        }
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = length;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_PhoneSimlockInfoPrintCmdName(DRV_AGENT_PersonalizationCategoryUint8 category, VOS_UINT8 indexNum,
    VOS_UINT16 *lengthTemp)
{
    /* ^PHONESIMLOCKINFO: <cat>,<indicator> [,<total_group_num>,<flag>,(<code_begin>,<code_end>)...] */
    *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    switch (category) {
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "NET");
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK_SUBSET:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "NETSUB");
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_SERVICE_PROVIDER:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "SP");
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_CORPORATE:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "CP");
            break;

        case DRV_AGENT_PERSONALIZATION_CATEGORY_SIM_USIM:
        default:
            return VOS_ERR;
    }
    return VOS_OK;
}

VOS_CHAR AT_Num2AsciiNum(VOS_UINT8 num)
{
    if (num <= AT_DEC_MAX_NUM) {
        return (VOS_CHAR)('0' + num);
    } else if (num <= 0x0F) {
        return (VOS_CHAR)('A' + (num - 0x0A));
    } else {
        return '*';
    }
}

VOS_UINT32 AT_CheckSimlockCodeLast2Char(DRV_AGENT_PersonalizationCategoryUint8 category, VOS_CHAR *pcStrLast2Char)
{
    VOS_UINT32 i = 0;

    /* 检测最后两位的有效性, NET无需检测 */
    if (category > DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK) {
        for (i = 0; i < AT_SIMLOCKCODE_LAST2CHAR_LEN; i++) {
            /* NETSUB支持范围: 0x00~0x99 */
            if ((pcStrLast2Char[i] >= '0') && (pcStrLast2Char[i] <= '9')) {
                continue;
            }
            /* SP支持范围: 0x00~0xFF */
            else if ((category == DRV_AGENT_PERSONALIZATION_CATEGORY_SERVICE_PROVIDER) && (pcStrLast2Char[i] >= 'A') &&
                     (pcStrLast2Char[i] <= 'F')) {
                continue;
            } else {
                return VOS_ERR;
            }
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_CheckSimlockCodeStr(DRV_AGENT_PersonalizationCategoryUint8 category, VOS_CHAR *pcStrCode)
{
    VOS_UINT32 i = 0;
    VOS_UINT32 j = 0;
    VOS_UINT8 len; /* 锁网号码长度 */
    VOS_CHAR *pcTmpStr = VOS_NULL_PTR;

    pcTmpStr = pcStrCode;

    switch (category) {
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK:
            len = AT_PERSONALIZATION_NET_CODE_LEN;
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_CORPORATE:
            len = AT_PERSONALIZATION_CP_CODE_LEN;
            break;
        default:
            len = AT_PERSONALIZATION_CODE_LEN;
            break;
    }

    /* 检测PLMN合法性及其位数 */
    for (i = 0; i < AT_PERSONALIZATION_NET_CODE_LEN; i++) {
        if ((pcTmpStr[i] >= '0') && (pcTmpStr[i] <= '9')) {
            continue;
        } else if ((i == (AT_PERSONALIZATION_NET_CODE_LEN - 1)) && (pcTmpStr[i] == 'F')) {
            for (j = i + 1; j < len; j++) {
                pcTmpStr[j - 1] = pcTmpStr[j];
            }
            len = len - 1;
            break;
        } else {
            return VOS_ERR;
        }
    }

    pcTmpStr[len] = 0;

    /* 检测锁网锁卡号码最后两位的合法性 */
    if (AT_CheckSimlockCodeLast2Char(category, &pcTmpStr[len - AT_SIMLOCKCODE_LAST2CHAR_LEN]) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SimlockCodeBcd2Str(DRV_AGENT_PersonalizationCategoryUint8 category, VOS_UINT8 *bcdNum, VOS_CHAR *pcStrNum,
                                 VOS_UINT32 strBufLen)
{
    VOS_UINT32 i = 0;
    VOS_UINT8 decodeLen;
    VOS_UINT8 firstNum;
    VOS_UINT8 secondNum;
    VOS_CHAR *pcStrTmp = pcStrNum;
    VOS_CHAR  tmpChar;

    switch (category) {
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK:
            decodeLen = AT_PERSONALIZATION_NET_CODE_BCD_LEN;
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK_SUBSET:
            decodeLen = AT_PERSONALIZATION_SUBNET_CODE_BCD_LEN;
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_SERVICE_PROVIDER:
            decodeLen = AT_PERSONALIZATION_SP_CODE_BCD_LEN;
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_CORPORATE:
            decodeLen = AT_PERSONALIZATION_CP_CODE_BCD_LEN;
            break;
        default:
            return VOS_ERR;
    }

    if (strBufLen <= AT_PERSONALIZATION_CODE_LEN_EX) {
        return VOS_ERR;
    }

    /* 对锁网锁卡号码进行高低字节转换 */
    for (i = 0; i < decodeLen; i++) {
        /* 分别取出高低字节 */
        firstNum  = (VOS_UINT8)((*(bcdNum + i)) & 0x0F);
        secondNum = (VOS_UINT8)(((*(bcdNum + i)) >> 4) & 0x0F);

        *pcStrTmp++ = AT_Num2AsciiNum(firstNum);
        *pcStrTmp++ = AT_Num2AsciiNum(secondNum);
    }

    /* 将第四位号码后移到第六位(与产线对接) */
    pcStrTmp    = &pcStrNum[AT_PERSONALIZATION_CODE_FOURTH_CHAR_INDEX];
    tmpChar     = pcStrTmp[0];
    pcStrTmp[0] = pcStrTmp[1];
    pcStrTmp[1] = pcStrTmp[2];
    pcStrTmp[2] = tmpChar;

    /* 对高低字节转换后的锁网锁卡号码进行合法性检查 */
    if (AT_CheckSimlockCodeStr(category, pcStrNum) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_PhoneSimlockInfoPrint(DRV_AGENT_PhonesimlockinfoQryCnfNew *phoneSimlockInfo, VOS_UINT8 indexNum,
                                    VOS_UINT16 *length)
{
    VOS_UINT16 lengthTemp;
    VOS_CHAR   acCodeBegin[AT_PERSONALIZATION_CODE_LEN_EX + 1];
    VOS_CHAR   acCodeEnd[AT_PERSONALIZATION_CODE_LEN_EX + 1];
    VOS_UINT32 codeBeginRslt;
    VOS_UINT32 codeEndRslt;
    VOS_UINT32 printGroupNum;
    VOS_UINT32 catIndex;
    VOS_UINT32 groupIndex;
    errno_t    memResult;
    VOS_UINT8  catNum;

    /* 局部变量初始化 */
    lengthTemp = *length;
    catIndex   = 0;
    groupIndex = 0;

    memResult = memset_s(acCodeBegin, sizeof(acCodeBegin), 0x00, (AT_PERSONALIZATION_CODE_LEN_EX + 1));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acCodeBegin), (AT_PERSONALIZATION_CODE_LEN_EX + 1));
    memResult = memset_s(acCodeEnd, sizeof(acCodeEnd), 0x00, (AT_PERSONALIZATION_CODE_LEN_EX + 1));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acCodeEnd), (AT_PERSONALIZATION_CODE_LEN_EX + 1));

    codeBeginRslt = VOS_NULL;
    codeEndRslt   = VOS_NULL;
    catNum        = AT_MIN(phoneSimlockInfo->supportCategoryNum, DRV_AGENT_SUPPORT_CATEGORY_NUM_EXTERED);

    for (catIndex = 0; catIndex < catNum; catIndex++) {
        if (catIndex != 0) {
            lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "%s", g_atCrLf);
        }

        /* ^PHONESIMLOCKINFO: <cat>,<indicator> [,<total_group_num>,<flag>,(<code_begin>,<code_end>)...] */
        if (AT_PhoneSimlockInfoPrintCmdName(phoneSimlockInfo->categoryInfo[catIndex].category, indexNum, &lengthTemp)
            != VOS_OK) {
            return VOS_ERR;
        }

        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", phoneSimlockInfo->categoryInfo[catIndex].indicator);

        if (phoneSimlockInfo->categoryInfo[catIndex].indicator == DRV_AGENT_PERSONALIZATION_INDICATOR_ACTIVE) {
            lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", phoneSimlockInfo->categoryInfo[catIndex].groupNum);

            lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", phoneSimlockInfo->categoryInfo[catIndex].flag);

            /* 最多打印上报10组号段 */
            printGroupNum = AT_MIN(phoneSimlockInfo->categoryInfo[catIndex].groupNum,
                                   DRV_AGENT_PH_LOCK_CODE_GROUP_NUM_EXTERED);

            for (groupIndex = 0; groupIndex < printGroupNum; groupIndex++) {
                codeBeginRslt =
                    AT_SimlockCodeBcd2Str(phoneSimlockInfo->categoryInfo[catIndex].category,
                                          phoneSimlockInfo->categoryInfo[catIndex].lockCode[groupIndex].phLockCodeBegin,
                                          acCodeBegin, sizeof(acCodeBegin));
                codeEndRslt =
                    AT_SimlockCodeBcd2Str(phoneSimlockInfo->categoryInfo[catIndex].category,
                                          phoneSimlockInfo->categoryInfo[catIndex].lockCode[groupIndex].phLockCodeEnd,
                                          acCodeEnd, sizeof(acCodeEnd));
                if ((codeBeginRslt == VOS_OK) && (codeEndRslt == VOS_OK)) {
                    lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",(%s,%s)", acCodeBegin, acCodeEnd);
                } else {
                    return VOS_ERR;
                }
            }
        }
    }
    *length = lengthTemp;
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentPhoneSimlockInfoQryCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                       *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_PhonesimlockinfoQryCnfNew *event  = VOS_NULL_PTR;
    VOS_UINT8                            indexNum;
    VOS_UINT32                           result;
    VOS_UINT16                           length;

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;

    event = (DRV_AGENT_PhonesimlockinfoQryCnfNew *)rcvMsg->content;

    indexNum = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    length   = 0;

    AT_PR_LOGI("enter");

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_PHONESIMLOCKINFO_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
        /* 打印结果 */
        if (AT_PhoneSimlockInfoPrint(event, indexNum, &length) != VOS_OK) {
            result = AT_PERSONALIZATION_OTHER_ERROR;
        }
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_SimlockDataReadPrintCategory(DRV_AGENT_PersonalizationCategoryUint8  category, VOS_UINT16 *lengthTemp)
{
    switch (category) {
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "NET");
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_NETWORK_SUBSET:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "NETSUB");
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_SERVICE_PROVIDER:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "SP");
            break;
        case DRV_AGENT_PERSONALIZATION_CATEGORY_CORPORATE:
            *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, "CP");
            break;
        default:
            return VOS_ERR;
    }
    return VOS_OK;
}

VOS_UINT32 AT_SimlockDataReadPrintStatus(DRV_AGENT_SimlockdatareadQryCnf simlockDataRead, VOS_UINT32 catIndex,
    VOS_UINT16 *lengthTemp)
{
    if (simlockDataRead.categoryData[catIndex].indicator == DRV_AGENT_PERSONALIZATION_INDICATOR_ACTIVE) {
        switch (simlockDataRead.categoryData[catIndex].status) {
            case DRV_AGENT_PERSONALIZATION_STATUS_READY:
                *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",READY,,");
                break;
            case DRV_AGENT_PERSONALIZATION_STATUS_PIN:
                *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",PIN,%d,%d",
                    simlockDataRead.categoryData[catIndex].maxUnlockTimes,
                    simlockDataRead.categoryData[catIndex].remainUnlockTimes);
                break;
            case DRV_AGENT_PERSONALIZATION_STATUS_PUK:
                *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",PUK,,");
                break;
            default:
                return VOS_ERR;
        }
    } else {
        *lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *lengthTemp, ",,,");
    }
    return VOS_OK;
}

VOS_UINT32 AT_SimlockDataReadPrint(DRV_AGENT_SimlockdatareadQryCnf simlockDataRead, VOS_UINT8 indexNum,
                                   VOS_UINT16 *length)
{
    VOS_UINT16 lengthTemp = *length;
    VOS_UINT32 catIndex   = 0;
    VOS_UINT8  catNum;

    catNum = AT_MIN(simlockDataRead.supportCategoryNum, DRV_AGENT_SUPPORT_CATEGORY_NUM);

    for (catIndex = 0; catIndex < catNum; catIndex++) {
        if (catIndex != 0) {
            lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "%s", g_atCrLf);
        }

        /* ^SIMLOCKDATAREAD: <cat>,<indicator>,<lock_status>,<max_times>,<remain_times> */
        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
        if (AT_SimlockDataReadPrintCategory(simlockDataRead.categoryData[catIndex].category, &lengthTemp) != VOS_OK) {
            return VOS_ERR;
        }

        lengthTemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthTemp, ",%d", simlockDataRead.categoryData[catIndex].indicator);
        if (AT_SimlockDataReadPrintStatus(simlockDataRead, catIndex, &lengthTemp) != VOS_OK) {
            return VOS_ERR;
        }
    }
    *length = lengthTemp;
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSimlockDataReadQryCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                   *rcvMsg = VOS_NULL_PTR;
    DRV_AGENT_SimlockdatareadQryCnf *event  = VOS_NULL_PTR;
    VOS_UINT8                        indexNum;
    VOS_UINT8                        simlockType;
    VOS_UINT32                       result;
    VOS_UINT16                       length;

    AT_PR_LOGI("enter");

    /* 初始化消息变量 */
    rcvMsg      = (DRV_AGENT_Msg *)msg;
    event       = (DRV_AGENT_SimlockdatareadQryCnf *)rcvMsg->content;
    indexNum    = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    simlockType = (VOS_UINT8)AT_GetSimlockUnlockCategoryFromClck();
    length      = 0;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentSimlockDataReadQryCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SIMLOCKDATAREAD_READ &&
        g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CLCK_SIMLOCKDATAREAD) {
        return VOS_ERR;
    }

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        result = AT_OK;
        /* 打印结果 */
        if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CLCK_SIMLOCKDATAREAD) {
            if (AT_SimlockDataReadPrint(*event, indexNum, &length) != VOS_OK) {
                result = AT_PERSONALIZATION_OTHER_ERROR;
            }
        } else {
            /* 读越界保护,取值超过最大范围时，不能读越界 */
            if (simlockType >= DRV_AGENT_SUPPORT_CATEGORY_NUM) {
                AT_WARN_LOG1("AT_RcvDrvAgentSimlockDataReadQryCnf: error simlockType", simlockType);
                simlockType = 0;
            }
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                event->categoryData[simlockType].indicator);
        }
    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = length;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

/*
 * 功能描述: ^GETMODEMSCID命令查询回复处理函数
 */
VOS_UINT32 AT_RcvDrvAgentGetSimlockEncryptIdQryCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg                          *rcvMsg  = VOS_NULL_PTR;
    DRV_AGENT_GetSimlockEncryptIdQryCnf    *event = VOS_NULL_PTR;
    VOS_UINT32                              result;
    VOS_UINT32                              scIdNum;
    VOS_UINT32                              i;
    VOS_UINT16                              length;
    VOS_UINT8                               indexNum;

    AT_PR_LOGI("enter");

    /* 初始化消息变量 */
    rcvMsg       = (DRV_AGENT_Msg *)msg;
    event        = (DRV_AGENT_GetSimlockEncryptIdQryCnf *)rcvMsg->content;
    indexNum     = AT_BROADCAST_CLIENT_INDEX_MODEM_0;
    length = 0;
    result = AT_OK;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_GETMODEMSCID_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_PERSONALIZATION_NO_ERROR) {
        /* 输出设置结果 */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%u,", event->scEncryptIdInfo.scAlgorithm);

        scIdNum = AT_MIN(event->scEncryptIdInfo.scEncryptIdNum, DRV_AGENT_SIMLOCK_ENCRYPT_KEYID_NUM);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%u", scIdNum);

        for (i = 0; i < scIdNum; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",0x%X", event->scEncryptIdInfo.scEncryptId[i]);
        }

    } else {
        /* 异常情况, 转换错误码 */
        result = AT_PERSONALIZATION_ERR_BEGIN + event->result;
    }

    g_atSendDataBuff.bufLen = length;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)

#else
VOS_UINT32 At_RcvUeCbtRfIcIdExQryCnf(struct MsgCB *msg)
{
    UECBT_AT_DieIdQueryInd *rcvMsg      = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32              i;
    VOS_UINT32              j;
    VOS_UINT16              length = 0;
    VOS_UINT8               indexNum;
    VOS_UINT8               dataSize = 0;
    VOS_UINT8               atCrLfLen;

    /* 初始化局部变量 */
    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    rcvMsg      = (UECBT_AT_DieIdQueryInd *)msg;

    AT_PR_LOGH("At_RcvUeCbtRfIcIdExQryCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("At_RcvUeCbtRfIcIdExQryCnf: indexNum err!");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_RFIC_DIE_ID_EX_QRY) {
        AT_WARN_LOG("AT_RcvMtaRficDieIDQryCnf: OPTION ERR!");
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if ((rcvMsg->errorCode != MT_OK) || (rcvMsg->chipNum > DIE_ID_QUERY_CHIP_MAX_NUM)) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    } else {
        atCrLfLen = (VOS_UINT8)strlen((VOS_CHAR *)g_atCrLf);
        for (i = 0; i < rcvMsg->chipNum; i++) {
            /* 打印命令名和序号,IC type */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,\"", g_parseContext[indexNum].cmdElement->cmdName,
                i, rcvMsg->dieIdInfo[i].chipType);
            /* RFIC ID 使用低八位数据 */
            dataSize = (VOS_UINT8)TAF_MIN((rcvMsg->dieIdInfo[i].infoSize), DIE_ID_MAX_LEN_BYTE);
            for (j = 0; j < dataSize; j++) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%02x", rcvMsg->dieIdInfo[i].data[j]);
            }

            /* 打印一个换行 */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s", g_atCrLf);
        }

        /* 将最后一个换行符去掉 */
        g_atSendDataBuff.bufLen = length - atCrLfLen;
        At_FormatResultData(indexNum, AT_OK);
    }

    return VOS_OK;
}
#endif

VOS_UINT32 At_PrintSetEsimSwitchInfo(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CmdEsimSwitch_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ESIMSWITCH_SET) {
        AT_WARN_LOG("At_PrintSetEsimSwitchInfo : CmdCurrentOpt is not AT_CmdEsimSwitch_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_PrintQryEsimSwitchInfo(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_ESIMSWITCH_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ESIMSWITCH_QRY) {
        AT_WARN_LOG("At_PrintQryEsimSwitchInfo : CmdCurrentOpt is not AT_CMD_ESIMSWITCH_QRY!");
        return AT_ERROR;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ESIMSWITCH: %d,%d%s", g_atCrLf, event->pihEvent.slotCardType.slot0CardType,
        event->pihEvent.slotCardType.slot1CardType, g_atCrLf);

    return AT_OK;
}

VOS_UINT32 At_PrintEsimCleanProfileInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CmdEsimSwitch_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ESIMCLEAN_SET) {
        AT_WARN_LOG("At_PrintEsimCleanProfileInfo: CmdCurrentOpt is not AT_CMD_ESIMCLEAN_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_PrintEsimCheckProfileInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_ESIMCHECK_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ESIMCHECK_QRY) {
        AT_WARN_LOG("At_PrintEsimCheckProfileInfo: CmdCurrentOpt is not AT_CMD_ESIMCHECK_QRY!");
        return AT_ERROR;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ESIMCHECK: ", g_atCrLf);

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "%d", event->ematEvent.esimCheckCnf.hasProfile);
    return AT_OK;
}

VOS_UINT32 At_PrintGetEsimEidInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_ESIMEID_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ESIMEID_QRY) {
        AT_WARN_LOG("At_PrintGetEsimEidInfo : CmdCurrentOpt is not AT_CMD_ESIMEID_QRY!");
        return AT_ERROR;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "%s^ESIMEID:\"", g_atCrLf);

    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     (TAF_UINT8 *)event->ematEvent.eidCnf.esimEID,
                                                     SI_EMAT_ESIM_EID_MAX_LEN);

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"%s", g_atCrLf);

    return AT_OK;
}

VOS_UINT32 At_PrintGetEsimPKIDInfo(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event, VOS_UINT16 *length)
{
    VOS_UINT32 pKIDNum;
    VOS_UINT32 loop = 0;

    /* 判断当前操作类型是否为AT_CMD_ESIMPKID_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ESIMPKID_QRY) {
        AT_WARN_LOG("At_PrintGetEsimPKIDInfo : CmdCurrentOpt is not AT_CMD_ESIMPKID_QRY!");
        return AT_ERROR;
    }

    pKIDNum = (event->ematEvent.pkIdCnf.pkIdNum > SI_EMAT_ESIM_PKID_GROUP_MAX_NUM) ? SI_EMAT_ESIM_PKID_GROUP_MAX_NUM :
                                                                                     event->ematEvent.pkIdCnf.pkIdNum;

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "%s^PKID:%d", g_atCrLf, pKIDNum);

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",\"");

    for (loop = 0; loop < pKIDNum; loop++) {
        (*length) += (VOS_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                         (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                         (TAF_UINT8 *)event->ematEvent.pkIdCnf.pkId[loop].esimPKID,
                                                         SI_EMAT_ESIM_PKID_VALUE_LEN);

        if (loop < pKIDNum - 1) {
            (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + (*length), "\",\"");
        }
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), "\"%s", g_atCrLf);

    return AT_OK;
}

TAF_VOID At_EMATIndProc(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event)
{
    return;
}

TAF_UINT32 At_EMATNotBroadIndProc(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event)
{
    return VOS_ERR;
}

TAF_VOID At_EMATRspProc(TAF_UINT8 indexNum, SI_EMAT_EventInfo *event)
{
    TAF_UINT32 result = AT_FAILURE;
    TAF_UINT16 length = 0;
    TAF_UINT32 tmp;
    TAF_UINT32 i;

    if (event->ematError != TAF_ERR_NO_ERROR) {
        AT_StopTimerCmdReady(indexNum);

        result = At_ChgTafErrorCode(indexNum, (TAF_UINT16)(event->ematError));

        g_atSendDataBuff.bufLen = length;

        At_FormatResultData(indexNum, result);

        return;
    }

    tmp = (sizeof(g_atEmatRspProcFuncTbl) / sizeof(g_atEmatRspProcFuncTbl[0]));

    for (i = 0; i < tmp; i++) {
        /* 找到处理函数，进行输出相关处理 */
        if (event->eventType == g_atEmatRspProcFuncTbl[i].eventType) {
            result = g_atEmatRspProcFuncTbl[i].atEMATRspProcFunc(indexNum, event, &length);

            if (result == AT_ERROR) {
                AT_WARN_LOG("At_EMATRspProc : pAtEMATRspProcFunc is return error!");
                return;
            }

            break;
        }
    }

    /* 没找到处理函数，直接返回 */
    if (i == tmp) {
        AT_WARN_LOG("At_EMATRspProc : no find AT Proc Func!");
        return;
    }

    result = AT_OK;

    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, result);
}

VOS_UINT32 AT_ChkEmatAtEventCnfMsgLen(const MSG_Header *msgHeader)
{
    const MN_APP_EmatAtCnf  *ematAtCnf  = VOS_NULL_PTR;
    const SI_EMAT_EventInfo *event = VOS_NULL_PTR;
    VOS_UINT32               size, i;

    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < AT_GET_EMAT_AT_CNF_LEN_WITHOUT_UNION) {
        AT_WARN_LOG("AT_ChkEmatAtEventCnfMsgLen: message length is invalid!");
        return VOS_FALSE;
    }
    ematAtCnf = (const MN_APP_EmatAtCnf *)msgHeader;
    event     = &ematAtCnf->ematAtCnf;

    size = (sizeof(g_atEmatRspProcFuncTbl) / sizeof(g_atEmatRspProcFuncTbl[0]));
    /*
     * 1、单板场景下，EMAT_AT_EVENT_CNF消息的检查原则是消息申请按照union最大申请或者该消息实际大小
     * 申请，只有(VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < fixdedPartLen这种情况认为检查失败
     * 2、PC环境下，EMAT_AT_EVENT_CNF消息的检查原则，消息申请的大小即不是按照union最大申请，也不是
     * 消息实际大小申请时，认为检查失败
     */
#if (VOS_OS_VER == VOS_WIN32)
    for (i = 0; i < size; i++) {
        if (event->eventType == g_atEmatRspProcFuncTbl[i].eventType) {
            if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH != sizeof(MN_APP_EmatAtCnf)) &&
                (TAF_RunChkMsgLenFunc(msgHeader,
                    (AT_GET_EMAT_AT_CNF_LEN_WITHOUT_UNION + g_atEmatRspProcFuncTbl[i].fixdedPartLen),
                    g_atEmatRspProcFuncTbl[i].chkFunc) != VOS_TRUE)) {
                AT_WARN_LOG("AT_ChkEmatAtEventCnfMsgLen: message length is invalid!");
                return VOS_FALSE;
            }
            break;
        }
    }
#else
    for (i = 0; i < size; i++) {
        if (event->eventType == g_atEmatRspProcFuncTbl[i].eventType) {
            if (TAF_RunChkMsgLenFunc(msgHeader,
                    (AT_GET_EMAT_AT_CNF_LEN_WITHOUT_UNION + g_atEmatRspProcFuncTbl[i].fixdedPartLen),
                    g_atEmatRspProcFuncTbl[i].chkFunc) != VOS_TRUE) {
                AT_WARN_LOG("AT_ChkEmatAtEventCnfMsgLen: message length is invalid!");
                return VOS_FALSE;
            }
            break;
        }
    }
#endif
    return VOS_TRUE;
}

TAF_VOID At_EMATMsgProc(struct MsgCB *msg)
{
    MN_APP_EmatAtCnf *msgTemp  = VOS_NULL_PTR;
    TAF_UINT8         indexNum = 0;

    msgTemp = (MN_APP_EmatAtCnf *)msg;

    if (msgTemp->msgId != EMAT_AT_EVENT_CNF) {
        AT_ERR_LOG1("At_EMATMsgProc: The Msg Id is Wrong", msgTemp->msgId);
        return;
    }

    AT_LOG1("At_EMATMsgProc pEvent->ClientId", msgTemp->ematAtCnf.clientId);
    AT_LOG1("At_EMATMsgProc EventType", msgTemp->ematAtCnf.eventType);
    AT_LOG1("At_EMATMsgProc SIM Event Error", msgTemp->ematAtCnf.ematError);

    if (At_ClientIdToUserId(msgTemp->ematAtCnf.clientId, &indexNum) == AT_FAILURE) {
        AT_ERR_LOG("At_EMATMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        At_EMATIndProc(indexNum, &msgTemp->ematAtCnf);
        AT_WARN_LOG("At_EMATMsgProc : AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_EMATMsgProc ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    /* 非广播的主动上报 */
    if (At_EMATNotBroadIndProc(indexNum, &msgTemp->ematAtCnf) == VOS_OK) {
        return;
    }

    At_EMATRspProc(indexNum, &msgTemp->ematAtCnf);
}

