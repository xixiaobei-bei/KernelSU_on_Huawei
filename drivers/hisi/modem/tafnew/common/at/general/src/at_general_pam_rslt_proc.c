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

#include "at_general_pam_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_drv_agent.h"
#include "at_external_module_msg_proc.h"
#include "at_custom_mm_rslt_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_PMM_RSLT_PROC_C

#define TAF_BCD_NUM_TO_CHAR_LEN 2

/* 结构的最后一项不做处理，仅表示结尾 */
static const AT_QueryTypeFunc g_atQryTypeProcFuncTbl[] = {
    { TAF_PH_IMSI_ID_PARA, At_QryParaRspCimiProc },
    { TAF_PH_MS_CLASS_PARA, At_QryParaRspCgclassProc },

    { TAF_PH_ICC_ID, At_QryParaRspIccidProc },
    { TAF_PH_PNN_PARA, At_QryParaRspPnnProc },
    { TAF_PH_CPNN_PARA, At_QryParaRspCPnnProc },
    { TAF_PH_OPL_PARA, At_QryParaRspOplProc },

    { TAF_PH_PNN_RANGE_PARA, At_QryRspUsimRangeProc },
    { TAF_PH_OPL_RANGE_PARA, At_QryRspUsimRangeProc },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { TAF_PH_CELLROAM_PARA, AT_QryParaRspCellRoamProc },
#endif
    { TAF_TELE_PARA_BUTT, TAF_NULL_PTR }
};

/*
 * Description: 参数查询结果Cimi的上报处理
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_VOID At_QryParaRspCimiProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    errno_t    memResult;
    TAF_UINT32 result;
    TAF_UINT16 length = 0;

    TAF_PH_Imsi cimi;

    memResult = memcpy_s(&cimi, sizeof(cimi), para, sizeof(TAF_PH_Imsi));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cimi), sizeof(TAF_PH_Imsi));
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", cimi.imsi);

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

TAF_UINT8 At_IsOplRecPrintable(TAF_PH_UsimOplRecord *oplRec, VOS_CHAR wildCard)
{
    TAF_UINT32 i;

    VOS_UINT8 wildCardTemp = 0x00;

    AT_ConvertCharToHex((VOS_UINT8)wildCard, &wildCardTemp);

    if (oplRec->pnnIndex == 0xFF) {
        return VOS_FALSE;
    }

    for (i = 0; i < (VOS_UINT32)AT_MIN(oplRec->plmnLen, AT_PLMN_MCC_MNC_LEN); i++) {
        if ((oplRec->plmn[i] >= 0xA) && (wildCardTemp != oplRec->plmn[i])) {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

/*
 * Description: 参数查询结果Opl的上报处理
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_VOID At_QryParaRspOplProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    TAF_UINT32         result;
    TAF_UINT16         length = 0;
    TAF_UINT32         i;
    TAF_UINT32         j;
    TAF_PH_UsimOplCnf *opl           = VOS_NULL_PTR;
    TAF_UINT32         recCntPrinted = 0;

    opl = (TAF_PH_UsimOplCnf *)para;

    /* 查询PNN记录数和记录长度 */
    for (i = 0; i < opl->totalRecordNum; i++) {
        if (At_IsOplRecPrintable((opl->oplRecord + i), opl->wildCard) == VOS_FALSE) {
            continue;
        }
        if (recCntPrinted != 0) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        opl->oplRecord[i].plmnLen = AT_MIN(opl->oplRecord[i].plmnLen, AT_PLMN_MCC_MNC_LEN);
        for (j = 0; j < opl->oplRecord[i].plmnLen; j++) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%X", opl->oplRecord[i].plmn[j]);
        }
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",0x%X-0x%X,%d", opl->oplRecord[i].lACLow,
            opl->oplRecord[i].lACHigh, opl->oplRecord[i].pnnIndex);

        ++recCntPrinted;
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

TAF_VOID At_QryParaRspCgclassProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    errno_t    memResult;
    TAF_UINT32 result;
    TAF_UINT16 length = 0;

    TAF_PH_MS_CLASS_TYPE cgclass = TAF_PH_MS_CLASS_NULL;

    memResult = memcpy_s(&cgclass, sizeof(cgclass), para, sizeof(TAF_PH_MS_CLASS_TYPE));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cgclass), sizeof(TAF_PH_MS_CLASS_TYPE));
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    if (cgclass == TAF_PH_MS_CLASS_A) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"A\"");
    } else if (cgclass == TAF_PH_MS_CLASS_B) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"B\"");
    } else if (cgclass == TAF_PH_MS_CLASS_CG) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"CG\"");
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"CC\"");
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_PrintPnnOperLongName(TAF_PH_UsimPnnCnf *pnn, TAF_UINT16 *length, TAF_UINT8 fullNameLen, TAF_UINT32 i)
{
    TAF_UINT8             firstByte;
    TAF_UINT8             tag;
    VOS_UINT8             pnnOperNameLen;

    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
    tag = FULL_NAME_IEI;
    *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + *length, &tag, 1);
    pnnOperNameLen = pnn->pnnRecord[i].operNameLong.length + 1;
    *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + *length, &pnnOperNameLen, 1);
    firstByte = (TAF_UINT8)((pnn->pnnRecord[i].operNameLong.ext << 7) |
                            (pnn->pnnRecord[i].operNameLong.coding << 4) |
                            (pnn->pnnRecord[i].operNameLong.addCi << 3) | (pnn->pnnRecord[i].operNameLong.spare));
    *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + *length, &firstByte, 1);
    *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                  (TAF_UINT8 *)g_atSndCodeAddress + *length,
                                                  pnn->pnnRecord[i].operNameLong.operatorName, fullNameLen);
    *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
}

VOS_VOID AT_PrintPnnOperShortName(TAF_PH_UsimPnnCnf *pnn, TAF_UINT16 *length, TAF_UINT8 shortNameLen, TAF_UINT32 i)
{
    TAF_UINT8             firstByte;
    TAF_UINT8             tag;
    VOS_UINT8             pnnOperNameLen;

    if (shortNameLen != 0) {
        tag = SHORT_NAME_IEI;
        /* 打印短名,需要加上TAG,长度和编码格式 */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length, &tag, 1);
        pnnOperNameLen = pnn->pnnRecord[i].operNameShort.length + 1;
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length, &pnnOperNameLen, 1);
        firstByte =
            (TAF_UINT8)((pnn->pnnRecord[i].operNameShort.ext << 7) | (pnn->pnnRecord[i].operNameShort.coding << 4) |
                        (pnn->pnnRecord[i].operNameShort.addCi << 3) | (pnn->pnnRecord[i].operNameShort.spare));
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length, &firstByte, 1);
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length,
                                                      pnn->pnnRecord[i].operNameShort.operatorName, shortNameLen);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"\"");
    }
}

VOS_VOID AT_PrintPnnOperAdditionalInfo(TAF_PH_UsimPnnCnf *pnn, TAF_UINT16 *length, TAF_UINT32 i)
{
    TAF_UINT8             tag;

    if (pnn->pnnRecord[i].plmnAdditionalInfoLen != 0) {
        /* PNN的其它信息,需要加上tag和长度 */
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
        tag = PLMN_ADDITIONAL_INFO_IEI;
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length, &tag, 1);
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length,
                                                      &pnn->pnnRecord[i].plmnAdditionalInfoLen, 1);
        *length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + *length,
                                                      pnn->pnnRecord[i].plmnAdditionalInfo,
                                                      pnn->pnnRecord[i].plmnAdditionalInfoLen);
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"");
    } else {
        *length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + *length, "\"\"");
    }
}

TAF_VOID At_QryParaRspPnnProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    TAF_UINT16 length = 0;

    TAF_PH_UsimPnnCnf    *pnn = VOS_NULL_PTR;
    TAF_UINT8             fullNameLen;
    TAF_UINT8             shortNameLen;
    TAF_UINT32            i;
    TAF_UINT32            ret;
    TAF_PH_QryUsimInfo    usimInfo;
    MN_CLIENT_OperationId clientOperationId = {0};
    memset_s(&usimInfo, sizeof(usimInfo), 0x00, sizeof(usimInfo));

    pnn = (TAF_PH_UsimPnnCnf *)para;

    /* 查询PNN记录数和记录长度 */
    for (i = 0; i < pnn->totalRecordNum; i++) {
        fullNameLen  = 0;
        shortNameLen = 0;

        fullNameLen = pnn->pnnRecord[i].operNameLong.length;

        if (fullNameLen == 0) {
            continue;
        }
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        /* 打印长名,需要加上TAG,长度和编码格式 */
        AT_PrintPnnOperLongName(pnn, &length, fullNameLen, i);

        shortNameLen = pnn->pnnRecord[i].operNameShort.length;

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",");
        /* 打印短名 */
        AT_PrintPnnOperShortName(pnn, &length, shortNameLen, i);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",");
         /* 打印PNN的其它信息 */
        AT_PrintPnnOperAdditionalInfo(pnn, &length, i);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        At_BufferorSendResultData(indexNum, g_atSndCodeAddress, length);
        length = 0;
    }

    /* 如果本次上报的PNN数目与要求的相同，则认为C核仍有PNN没有上报，要继续发送请求进行查询 */
    if (pnn->totalRecordNum == TAF_MMA_PNN_INFO_MAX_NUM) {
        usimInfo.recNum                   = 0;
        usimInfo.efId                     = TAF_PH_PNN_FILE;
        usimInfo.icctype                  = pnn->icctype;
        usimInfo.pnnQryIndex.pnnNum       = TAF_MMA_PNN_INFO_MAX_NUM;
        usimInfo.pnnQryIndex.pnnCurrIndex = pnn->pnnCurrIndex + TAF_MMA_PNN_INFO_MAX_NUM;

        clientOperationId.clientId = g_atClientTab[indexNum].clientId;
        clientOperationId.opId     = 0;

        ret = MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_USIM_INFO, &usimInfo, sizeof(TAF_PH_QryUsimInfo),
                                     I0_WUEPS_PID_MMA);

        if (ret != TAF_SUCCESS) {
            /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
            AT_StopTimerCmdReady(indexNum);
            g_atSendDataBuff.bufLen = 0;
            At_FormatResultData(indexNum, AT_ERROR);
        }
    } else {
        /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
        AT_StopTimerCmdReady(indexNum);
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_OK);
    }
}

TAF_VOID At_QryParaRspCPnnProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    TAF_UINT32         result;
    TAF_UINT16         length = 0;
    TAF_UINT8          codingScheme;
    TAF_PH_UsimPnnCnf *pnn = VOS_NULL_PTR;
    TAF_UINT8          fullNameLen;
    TAF_UINT8          shortNameLen;
    TAF_UINT8          tag;

    pnn = (TAF_PH_UsimPnnCnf *)para;

    if (pnn->totalRecordNum != 0) {
        fullNameLen  = 0;
        shortNameLen = 0;

        fullNameLen = pnn->pnnRecord[0].operNameLong.length;

        if (fullNameLen != 0) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

            /* 打印长名 */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"");
            tag = FULL_NAME_IEI;
            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                          (TAF_UINT8 *)g_atSndCodeAddress + length, &tag, 1);
            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                          (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                          pnn->pnnRecord[0].operNameLong.operatorName, fullNameLen);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"");

            codingScheme = pnn->pnnRecord[0].operNameLong.coding;
            if (pnn->pnnRecord[0].operNameLong.coding != 0) {
                codingScheme = 1;
            }

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",%d,%d", codingScheme, pnn->pnnRecord[0].operNameLong.addCi);

            shortNameLen = pnn->pnnRecord[0].operNameShort.length;

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, ",");

            if (shortNameLen != 0) {
                /* 打印短名 */
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "\"");
                length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                              (TAF_UINT8 *)g_atSndCodeAddress + length,
                                                              pnn->pnnRecord[0].operNameShort.operatorName,
                                                              shortNameLen);
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "\"");

                codingScheme = pnn->pnnRecord[0].operNameShort.coding;
                if (codingScheme != 0) {
                    codingScheme = 1;
                }

                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, ",%d,%d", codingScheme,
                    pnn->pnnRecord[0].operNameShort.addCi);
            } else {
                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, "\"\"");

                length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                    (TAF_CHAR *)g_atSndCodeAddress + length, ",0,0");
            }
        }
    }

    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

#if (FEATURE_MBB_CUST == FEATURE_ON)

/*
 * 功能说明: 按照NV设置的传输模式处理ICCID结果
 * 输入参数: iccid: ICCID字符串
 *           length: ICCID的完整长度
 * 输出参数: iccid: ICCID处理后的字符串
 * 返回结果: ICCID处理后的长度
 */
LOCAL VOS_UINT16 AT_IccidTransModeHandle(VOS_UINT8 *iccid, VOS_UINT16 length)
{
    VOS_UINT16 tmpLen = length;
    AT_ModemIccidTransMode *iccidTansMode = AT_GetModemMbbIccidTransMode(MODEM_ID_0);

    /* NV配置为非截断模式，直接返回 */
    if (iccidTansMode->transMode != AT_ICCID_TRANS_TRUNCATION) {
        return length;
    }

    while (tmpLen != 0) {
        /* 从末尾判断F位，0x46是F的ASCII码值 */
        if (*(iccid + tmpLen - 1) == 'F') {
            *(iccid + tmpLen - 1) = '\0';
            tmpLen--;
        } else {
            break;
        }
    }

    return tmpLen;
}
/*
 * 功能说明: 将16进制的ICCID内容转换为字符串
 * 输入参数: curLength: 当前输出字符串中的已有内容长度
 *           src: ICCID 16进制内容
 *           srcLen: ICCID长度
 * 返回结果: 增加ICCID内容后的输出字符串长度
 */
VOS_UINT16 AT_Hex2AsciiStrForIccId(VOS_UINT16 curLength, VOS_UINT8 *src, VOS_UINT16 srcLen)
{
    VOS_UINT16 iccidLen = 0;
    VOS_UINT16 counter = 0;
    VOS_UINT16 totalLen = curLength;
    VOS_UINT8 tmpByte;
    VOS_UINT8 iccidIdx = 0;
    VOS_UINT8 iccid[TAF_PH_ICC_ID_MAX * TAF_BCD_NUM_TO_CHAR_LEN + 1] = {0};

    /* 入参校验 */
    if ((src == VOS_NULL_PTR) || (srcLen == 0) || (srcLen > TAF_PH_ICC_ID_MAX)) {
        return curLength;
    }

    while (counter++ < srcLen) {
        /* 先取低四位的值 */
        tmpByte = *src & 0x0F;
        if (tmpByte <= 0x09) {
            iccid[iccidIdx++] = tmpByte + 0x30; /* 转换为ASCII码 */
        } else {
            iccid[iccidIdx++] = tmpByte + 0x37; /* 转换为ASCII码 */
        }

        /* 后取高四位的值 */
        tmpByte = (*src >> 4) & 0x0F;
        if (tmpByte <= 0x09) {
            iccid[iccidIdx++] = tmpByte + 0x30; /* 转换为ASCII码 */
        } else {
            iccid[iccidIdx++] = tmpByte + 0x37; /* 转换为ASCII码 */
        }

        src++;
    }
    iccidLen = AT_IccidTransModeHandle(iccid, iccidIdx);
    /* 在输出字符串中增加ICCID的内容 */
    if (iccidLen != 0) {
        totalLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + curLength, "%s", iccid);
    }

    return totalLen;
}
#endif

TAF_VOID At_QryParaRspIccidProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    errno_t      memResult;
    TAF_UINT32   result;
    TAF_UINT16   length;
    TAF_PH_IccId iccId;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ICCID_READ) {
        return;
    }

    length = 0;
    memset_s(&iccId, sizeof(iccId), 0x00, sizeof(TAF_PH_IccId));
    memResult = memcpy_s(&iccId, sizeof(iccId), para, sizeof(TAF_PH_IccId));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(iccId), sizeof(TAF_PH_IccId));

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
#if (FEATURE_MBB_CUST == FEATURE_ON)
    length = AT_Hex2AsciiStrForIccId(length, iccId.iccId, iccId.len);
#else
    length += (VOS_UINT16)AT_Hex2AsciiStrLowHalfFirst(AT_CMD_MAX_LEN, (VOS_INT8 *)g_atSndCodeAddress,
                                                      (VOS_UINT8 *)g_atSndCodeAddress + length, iccId.iccId, iccId.len);
#endif
    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

TAF_VOID At_QryRspUsimRangeProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_UINT8 *para)
{
    TAF_PH_QryUsimRangeInfo *usimRangeInfo = VOS_NULL_PTR;
    TAF_UINT16               length        = 0;
    TAF_UINT32               result;
    TAF_UINT8                simValue;
    VOS_BOOL                 bUsimInfoPrinted = VOS_FALSE;

    usimRangeInfo = (TAF_PH_QryUsimRangeInfo *)para;
    if ((usimRangeInfo->usimInfo.fileExist == VOS_TRUE) && (usimRangeInfo->usimInfo.icctype == TAF_PH_ICC_USIM)) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        simValue = 1;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d,(1,%d),%d", simValue, usimRangeInfo->usimInfo.totalRecNum,
            usimRangeInfo->usimInfo.recordLen);
        bUsimInfoPrinted = VOS_TRUE;
    }
    if ((usimRangeInfo->simInfo.fileExist == VOS_TRUE) && (usimRangeInfo->simInfo.icctype == TAF_PH_ICC_SIM)) {
        if (bUsimInfoPrinted == VOS_TRUE) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);
        simValue = 0;
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%d,(1,%d),%d", simValue, usimRangeInfo->simInfo.totalRecNum,
            usimRangeInfo->simInfo.recordLen);
    }
    result                  = AT_OK;
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, result);
}

VOS_UINT32 At_ProcPihCimiQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_CIMI_READ */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CIMI_READ) {
        AT_WARN_LOG("At_ProcPihCimiQryCnf: CmdCurrentOpt is not AT_CMD_CIMI_READ!");
        return AT_ERROR;
    }

    /* buf从外部接口返回的，增加结束符，防止缓冲区溢出 */
    event->pihEvent.imsi.imsi[SI_IMSI_MAX_LEN - 1] = '\0';

    g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_CIMI;
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s", event->pihEvent.imsi.imsi);

    return AT_OK;
}

TAF_VOID At_QryParaRspProc(TAF_UINT8 indexNum, TAF_UINT8 opId, TAF_PARA_TYPE queryType, TAF_UINT16 errorCode,
                           TAF_UINT8 *para)
{
    TAF_UINT32 result = AT_FAILURE;
    TAF_UINT16 length = 0;
    TAF_UINT32 tmp;
    TAF_UINT32 i;

    if (errorCode != 0) { /* 错误 */
        AT_StopTimerCmdReady(indexNum);

        result                  = At_ChgTafErrorCode(indexNum, errorCode); /* 发生错误 */
        g_atSendDataBuff.bufLen = length;
        At_FormatResultData(indexNum, result);
        return;
    }

    if (para == TAF_NULL_PTR) { /* 如果查询出错 */
        AT_StopTimerCmdReady(indexNum);

        At_FormatResultData(indexNum, AT_CME_UNKNOWN);

        return;
    }

    tmp = (sizeof(g_atQryTypeProcFuncTbl) / sizeof(g_atQryTypeProcFuncTbl[0]));
    for (i = 0; i != tmp; i++) {
        if (queryType == g_atQryTypeProcFuncTbl[i].queryType) {
            if (queryType != TAF_PH_ICC_ID) {
                AT_StopTimerCmdReady(indexNum);
            }

            g_atQryTypeProcFuncTbl[i].atQryParaProcFunc(indexNum, opId, para);

            return;
        }
    }

    AT_StopTimerCmdReady(indexNum);

    AT_WARN_LOG("At_QryParaRspProc QueryType FAILURE");
}

TAF_VOID At_QryMsgProc(TAF_QryRslt *qryRslt)
{
    TAF_UINT8     indexNum;

    AT_LOG1("At_QryMsgProc ClientId", qryRslt->clientId);
    AT_LOG1("At_QryMsgProc QueryType", qryRslt->queryType);
    AT_LOG1("At_QryMagProc usErrorCode", qryRslt->errorCode);
    if (qryRslt->clientId == AT_BUTT_CLIENT_ID) {
        AT_WARN_LOG("At_QryMsgProc Error ucIndex");
        return;
    }

    indexNum = 0;
    if (At_ClientIdToUserId(qryRslt->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("At_QryMsgProc At_ClientIdToUserId FAILURE");
        return;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_QryMsgProc: AT_BROADCAST_INDEX.");
        return;
    }

    AT_LOG1("At_QryMsgProc ucIndex", indexNum);
    AT_LOG1("g_atClientTab[ucIndex].cmdCurrentOpt", g_atClientTab[indexNum].cmdCurrentOpt);

    At_QryParaRspProc(indexNum, qryRslt->opId, qryRslt->queryType, qryRslt->errorCode, qryRslt->para);
}

VOS_UINT32 AT_RcvDrvAgentSetAdcRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg       *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_AdcSetCnf *adcCnf   = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;
    VOS_UINT32           result;

    /* 初始化消息 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    adcCnf = (DRV_AGENT_AdcSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(adcCnf->atAppCtrl.clientId, AT_CMD_ADC_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 设置门限值是否成功 */
    if (adcCnf->fail != VOS_TRUE) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    /* 调用AT_FormATResultDATa发送命令结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentQryAdcRsp(struct MsgCB *msg)
{
    VOS_UINT8 indexNum = 0;
    VOS_UINT32 result;

    /* 初始化消息 */
    DRV_AGENT_Msg *rcvMsg  = (DRV_AGENT_Msg *)msg;
    DRV_AGENT_AdcQryCnf *adcCnf = (DRV_AGENT_AdcQryCnf *)rcvMsg->content;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(adcCnf->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentQryAdcRsp:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentQryAdcRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_ADCTEMP_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ADCTEMP_QRY) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    result = (adcCnf->fail == VOS_TRUE) ? AT_OK : AT_ERROR;

    /* 调用AT_FormATResultDATa发送命令结果 */
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR*)g_atSndCodeAddress, "^ADCTEMP: %d,%d,%d,%d", adcCnf->tempProtect.isEnable,
        adcCnf->tempProtect.closeAdcThreshold, adcCnf->tempProtect.alarmAdcThreshold,
        adcCnf->tempProtect.resumeAdcThreshold);

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 At_ProcPihCglaSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_CGLA_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CGLA_SET) {
        AT_WARN_LOG("At_ProcPihCglaSetCnf : CmdCurrentOpt is not AT_CMD_CGLA_SET!");
        return AT_ERROR;
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    /* <length>, */
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%d,\"",
        (event->pihEvent.cglaCmdCnf.len + AT_DOUBLE_QUO_MARKS_LENGTH) * AT_DOUBLE_LENGTH);
    if (event->pihEvent.cglaCmdCnf.len != 0) {
        /* <command>, */
        (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                         (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                         event->pihEvent.cglaCmdCnf.command,
                                                         event->pihEvent.cglaCmdCnf.len);
    }

    /* SW1 */
    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     &event->pihEvent.cglaCmdCnf.sw1, sizeof(TAF_UINT8));

    /* SW2 */
    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     &event->pihEvent.cglaCmdCnf.sw2, sizeof(TAF_UINT8));
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                                                   (TAF_CHAR *)g_atSndCodeAddress + (*length), "\"");

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaCgsnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg        *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_CgsnQryCnf *cgsn = (MTA_AT_CgsnQryCnf *)mtaMsg->content;
    VOS_UINT32         i;
    VOS_UINT8          checkData = 0;
    VOS_UINT8          imeiAscii[TAF_PH_IMEI_LEN];

    /* 初始化 */
    (VOS_VOID)memset_s(imeiAscii, sizeof(imeiAscii), 0x00, sizeof(imeiAscii));

    /* 读取IMEI信息，先减去最后2个元素的长度，在最后另外赋值，一次循环赋值2个元素 */
    for (i = 0; i < (TAF_PH_IMEI_LEN - 2); i += 2) {
        imeiAscii[i]       = cgsn->imei[i] + AT_HEX_CONVERT_ASCII_NUM_BASE_VALUE;
        imeiAscii[i + 1UL] = cgsn->imei[i + 1UL] + AT_HEX_CONVERT_ASCII_NUM_BASE_VALUE;
        /* 除10是为了取高位，余10是为了取低位 */
        checkData += (TAF_UINT8)(cgsn->imei[i] + ((cgsn->imei[i + 1UL] * AT_DOUBLE_LENGTH) / AT_DECIMAL_BASE_NUM) +
                                ((cgsn->imei[i + 1UL] * AT_DOUBLE_LENGTH) % AT_DECIMAL_BASE_NUM));
    }
    checkData = (AT_DECIMAL_BASE_NUM - (checkData % AT_DECIMAL_BASE_NUM)) % AT_DECIMAL_BASE_NUM;

    /* 给最后两位元素另外赋值 */
    imeiAscii[TAF_PH_IMEI_LEN - 2] = checkData + AT_HEX_CONVERT_ASCII_NUM_BASE_VALUE;
    imeiAscii[TAF_PH_IMEI_LEN - 1] = 0;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s", imeiAscii);

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_CGSN;
    At_FormatResultData(indexNum, AT_OK);
    g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_BUTT;

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

