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
#include "at_cdma_event_report.h"
#include "at_cdma_rslt_proc.h"
#include "taf_type_def.h"
#include "securec.h"
#include "at_file_handle.h"

#include "ppp_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "at_oam_interface.h"
#include "mn_comm_api.h"

#include "at_input_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_msg_print.h"


#include "at_event_report.h"
#include "at_ss_comm.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CDMA_EVENT_REPORT_C

#define AT_DIGIT_MAX_LENGTH 2

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
#define AT_XPDS_AGPS_DATAUPLEN_MAX 240
#define AT_AGPS_EPH_INFO_MAX_SEG_NUM 5
#define AT_AGPS_EPH_INFO_FIRST_SEG_STR_LEN 8
#define AT_AGPS_EPH_INFO_NOT_FIRST_SEG_STR_LEN 960
#define AT_AGPS_ALM_INFO_MAX_SEG_NUM 3
#define AT_AGPS_ALM_INFO_FIRST_SEG_STR_LEN 8
#define AT_AGPS_ALM_INFO_NOT_FIRST_SEG_STR_LEN 896
#endif

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON))
static const AT_EncryptVoiceErrCodeMap g_atEncVoiceErrCodeMapTbl[] = {
    { AT_ENCRYPT_VOICE_SUCC, TAF_CALL_ENCRYPT_VOICE_SUCC },
    { AT_ENCRYPT_VOICE_TIMEOUT, TAF_CALL_ENCRYPT_VOICE_TIMEOUT },
    { AT_ENCRYPT_VOICE_TIMEOUT, TAF_CALL_ENCRYPT_VOICE_TX01_TIMEOUT },
    { AT_ENCRYPT_VOICE_TIMEOUT, TAF_CALL_ENCRYPT_VOICE_TX02_TIMEOUT },
    { AT_ENCRYPT_VOICE_LOCAL_TERMINAL_NO_AUTHORITY, TAF_CALL_ENCRYPT_VOICE_LOCAL_TERMINAL_NO_AUTHORITY },
    { AT_ENCRYPT_VOICE_REMOTE_TERMINAL_NO_AUTHORITY, TAF_CALL_ENCRYPT_VOICE_REMOTE_TERMINAL_NO_AUTHORITY },
    { AT_ENCRYPT_VOICE_LOCAL_TERMINAL_ILLEGAL, TAF_CALL_ENCRYPT_VOICE_LOCAL_TERMINAL_ILLEGAL },
    { AT_ENCRYPT_VOICE_REMOTE_TERMINAL_ILLEGAL, TAF_CALL_ENCRYPT_VOICE_REMOTE_TERMINAL_ILLEGAL },
    { AT_ENCRYPT_VOICE_UNKNOWN_ERROR, TAF_CALL_ENCRYPT_VOICE_UNKNOWN_ERROR },
    { AT_ENCRYPT_VOICE_SIGNTURE_VERIFY_FAILURE, TAF_CALL_ENCRYPT_VOICE_SIGNTURE_VERIFY_FAILURE },
    { AT_ENCRYPT_VOICE_MT_CALL_NOTIFICATION, TAF_CALL_ENCRYPT_VOICE_MT_CALL_NOTIFICATION },

    /* Internal err code */
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_FAIL, TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_FAIL },
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_POOL_FULL, TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_POOL_FULL },
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_LINK_ERR, TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_LINK_ERR },
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_NO_TL_ACK, TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_NO_TL_ACK },
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_ENCODE_ERR, TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_ENCODE_ERR },
    { AT_ENCRYPT_VOICE_XSMS_SEND_RESULT_UNKNOWN, TAF_CALL_ENCRYPT_VOICE_XSMS_SEND_RESULT_UNKNOWN },
    { AT_ENCRYPT_VOICE_SO_NEGO_FAILURE, TAF_CALL_ENCRYPT_VOICE_SO_NEGO_FAILURE },
    { AT_ENCRYPT_VOICE_TWO_CALL_ENTITY_EXIST, TAT_CALL_APP_ENCRYPT_VOICE_TWO_CALL_ENTITY_EXIST },
    { AT_ENCRYPT_VOICE_NO_MO_CALL, TAF_CALL_ENCRYPT_VOICE_NO_MO_CALL },
    { AT_ENCRYPT_VOICE_NO_MT_CALL, TAF_CALL_ENCRYPT_VOICE_NO_MT_CALL },
    { AT_ENCRYPT_VOICE_NO_CALL_EXIST, TAF_CALL_ENCRYPT_VOICE_NO_CALL_EXIST },
    { AT_ENCRYPT_VOICE_CALL_STATE_NOT_ALLOWED, TAF_CALL_ENCRYPT_VOICE_CALL_STATE_NOT_ALLOWED },
    { AT_ENCRYPT_VOICE_CALL_NUM_MISMATCH, TAF_CALL_ENCRYPT_VOICE_CALL_NUM_MISMATCH },
    { AT_ENCRYPT_VOICE_ENC_VOICE_STATE_MISMATCH, TAF_CALL_ENCRYPT_VOICE_ENC_VOICE_STATE_MISMATCH },
    { AT_ENCRYPT_VOICE_MSG_ENCODE_FAILUE, TAF_CALL_ENCRYPT_VOICE_MSG_ENCODE_FAILUE },
    { AT_ENCRYPT_VOICE_MSG_DECODE_FAILUE, TAF_CALL_ENCRYPT_VOICE_MSG_DECODE_FAILUE },
    { AT_ENCRYPT_VOICE_GET_TEMP_PUB_PIVA_KEY_FAILURE, TAF_CALL_ENCRYPT_VOICE_GET_TEMP_PUB_PIVA_KEY_FAILURE },
    { AT_ENCRYPT_VOICE_FILL_CIPHER_TEXT_FAILURE, TAF_CALL_ENCRYPT_VOICE_FILL_CIPHER_TEXT_FAILURE },
    { AT_ENCRYPT_VOICE_ECC_CAP_NOT_SUPPORTED, TAF_CALL_ENCRYPT_VOICE_ECC_CAP_NOT_SUPPORTED },
    { AT_ENCRYPT_VOICE_ENC_VOICE_MODE_UNKNOWN, TAF_CALL_ENCRYPT_VOICE_ENC_VOICE_MODE_UNKNOWN },
    { AT_ENCRYPT_VOICE_ENC_VOICE_MODE_MIMATCH, TAF_CALL_ENCRYPT_VOICE_ENC_VOICE_MODE_MIMATCH },
    { AT_ENCRYPT_VOICE_CALL_RELEASED, TAF_CALL_ENCRYPT_VOICE_CALL_RELEASED },
    { AT_ENCRYPT_VOICE_CALL_ANSWER_REQ_FAILURE, TAF_CALL_ENCRYPT_VOICE_CALL_ANSWER_REQ_FAILURE },
    { AT_ENCRYPT_VOICE_DECRYPT_KS_FAILURE, TAF_CALL_ENCRYPT_VOICE_DECRYPT_KS_FAILURE },
    { AT_ENCRYPT_VOICE_FAILURE_CAUSED_BY_INCOMING_CALL, TAF_CALL_ENCRYPT_VOICE_FAILURE_CAUSED_BY_INCOMING_CALL },
    { AT_ENCRYPT_VOICE_INIT_VOICE_FUNC_FAILURE, TAF_CALL_ENCRYPT_VOICE_INIT_VOICE_FUNC_FAILURE },
    { AT_ENCRYPT_VOICE_ERROR_ENUM_BUTT, TAF_CALL_ENCRYPT_VOICE_STATUS_ENUM_BUTT }
};
#endif


#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
static const AT_PROC_MsgFromXpds g_atProcMsgFromXpdsTab[] = {
    { ID_XPDS_AT_GPS_START_CNF, AT_RcvXpdsCagpsRlstCnf },
    { ID_XPDS_AT_GPS_CFG_MPC_ADDR_CNF, AT_RcvXpdsCagpsRlstCnf },
    { ID_XPDS_AT_GPS_CFG_PDE_ADDR_CNF, AT_RcvXpdsCagpsRlstCnf },
    { ID_XPDS_AT_GPS_CFG_MODE_CNF, AT_RcvXpdsCagpsRlstCnf },
    { ID_XPDS_AT_GPS_STOP_CNF, AT_RcvXpdsCagpsRlstCnf },

    { ID_XPDS_AT_GPS_TIME_INFO_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_REFLOC_INFO_CNF, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_PDE_POSI_INFO_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_NI_SESSION_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_START_REQ, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_CANCEL_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_ACQ_ASSIST_DATA_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_ABORT_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_DEL_ASSIST_DATA_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_ION_INFO_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_EPH_INFO_IND, AT_RcvXpdsEphInfoInd },
    { ID_XPDS_AT_GPS_ALM_INFO_IND, AT_RcvXpdsAlmInfoInd },
    { ID_XPDS_AT_GPS_NI_CP_STOP, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_NI_CP_START, AT_RcvXpdsCagpsCnf },

    { ID_XPDS_AT_AP_DATA_CALL_REQ, AT_RcvXpdsAgpsDataCallReq },
    { ID_XPDS_AT_AP_SERVER_BIND_REQ, AT_RcvXpdsAgpsServerBindReq },
    { ID_XPDS_AT_AP_REVERSE_DATA_IND, AT_RcvXpdsAgpsReverseDataInd },

    { ID_XPDS_AT_GPS_UTS_TEST_START_REQ, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_UTS_TEST_STOP_REQ, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_UTS_GPS_POS_INFO_IND, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_OM_TEST_START_REQ, AT_RcvXpdsCagpsCnf },
    { ID_XPDS_AT_GPS_OM_TEST_STOP_REQ, AT_RcvXpdsCagpsCnf },
};
#endif

VOS_UINT32 AT_RcvTafCcmDispInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_DisplayInfoInd *displayInfo = VOS_NULL_PTR;
    VOS_UINT8               digit[TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM + 1];
    errno_t                 memResult;

    displayInfo = (TAF_CCM_DisplayInfoInd *)msg;

    displayInfo->disPlayInfoIndPara.digitNum = AT_MIN(displayInfo->disPlayInfoIndPara.digitNum,
                                                      TAF_CALL_MAX_DISPALY_CHARI_OCTET_NUM);
    /* 初始化 */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
    if (displayInfo->disPlayInfoIndPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), displayInfo->disPlayInfoIndPara.digit,
                             displayInfo->disPlayInfoIndPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), displayInfo->disPlayInfoIndPara.digitNum);
    }

    /* 在pstDisplayInfo->aucDigit的最后一位加'\0',防止因pstDisplayInfo->aucDigit无结束符，导致AT多上报 */
    digit[displayInfo->disPlayInfoIndPara.digitNum] = '\0';

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s,,,%s", g_atCrLf, g_atStringTab[AT_STRING_CDISP].text, digit, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmExtDispInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_ExtDisplayInfoInd *extDispInfo          = VOS_NULL_PTR;
    VOS_UINT8                  digit[TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM + 1];
    VOS_UINT32                 loop;
    VOS_UINT32                 digitNum;
    errno_t                    memResult;
    VOS_UINT32                 infoRecsDataSmallNum = 0;

    extDispInfo = (TAF_CCM_ExtDisplayInfoInd *)msg;

    infoRecsDataSmallNum =
        (VOS_UINT32)TAF_MIN(extDispInfo->disPlayInfoIndPara.infoRecsDataNum, TAF_CALL_MAX_EXT_DISPLAY_DATA_NUM);
    /* 初始化 */
    for (loop = 0; loop < infoRecsDataSmallNum; loop++) {
        /*
         * 在pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit的最后一位加'\0',
         * 防止因pstExtDispInfo->aucInfoRecsData[ulLoop].aucDigit无结束符，导致AT多上报
         */
        digitNum = AT_MIN(extDispInfo->disPlayInfoIndPara.infoRecsData[loop].digitNum,
                          TAF_CALL_MAX_EXTENDED_DISPALY_CHARI_OCTET_NUM + 1);
        (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
        if (digitNum > 0) {
            memResult = memcpy_s(digit, sizeof(digit), extDispInfo->disPlayInfoIndPara.infoRecsData[loop].digit,
                                 digitNum);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), digitNum);
        }
        digit[digitNum] = '\0';

        /* 输出查询结果 */
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s%s%s,%d,%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_CDISP].text,
                digit, extDispInfo->disPlayInfoIndPara.extDispInd, extDispInfo->disPlayInfoIndPara.displayType,
                extDispInfo->disPlayInfoIndPara.infoRecsData[loop].dispalyTag, g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmConnNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_ConnNumInfoInd *connNumInfo = VOS_NULL_PTR;
    VOS_UINT8               digit[TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                 memResult;

    connNumInfo = (TAF_CCM_ConnNumInfoInd *)msg;

    connNumInfo->connNumInfoIndPara.digitNum = AT_MIN(connNumInfo->connNumInfoIndPara.digitNum,
                                                      TAF_CALL_MAX_CONNECTED_NUMBER_CHARI_OCTET_NUM);
    /* 初始化 */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
    if (connNumInfo->connNumInfoIndPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), connNumInfo->connNumInfoIndPara.digit,
                             connNumInfo->connNumInfoIndPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), connNumInfo->connNumInfoIndPara.digitNum);
    }

    /* 在pstConnNumInfo->aucDigit的最后一位加'\0',防止因pstConnNumInfo->aucDigit无结束符，导致AT多上报 */
    digit[connNumInfo->connNumInfoIndPara.digitNum] = '\0';

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d,%d,%d,%s%s", g_atCrLf, g_atStringTab[AT_STRING_CCONNNUM].text,
            connNumInfo->connNumInfoIndPara.numType, connNumInfo->connNumInfoIndPara.numPlan,
            connNumInfo->connNumInfoIndPara.pi, connNumInfo->connNumInfoIndPara.si, digit, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCalledNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CalledNumInfoInd *calledNum = VOS_NULL_PTR;
    VOS_UINT8                 digit[TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                   memResult;

    calledNum = (TAF_CCM_CalledNumInfoInd *)msg;

    calledNum->calledNumInfoPara.digitNum = AT_MIN(calledNum->calledNumInfoPara.digitNum,
                                                   TAF_CALL_MAX_CALLED_NUMBER_CHARI_OCTET_NUM);
    /* 初始化 */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
    if (calledNum->calledNumInfoPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), calledNum->calledNumInfoPara.digit,
                             calledNum->calledNumInfoPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), calledNum->calledNumInfoPara.digitNum);
    }

    /* 在pstCalledNum->aucDigit的最后一位加'\0',防止因pstCalledNum->aucDigit无结束符，导致AT多上报 */
    digit[calledNum->calledNumInfoPara.digitNum] = '\0';

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d,%s%s", g_atCrLf, g_atStringTab[AT_STRING_CCALLEDNUM].text,
        calledNum->calledNumInfoPara.numType, calledNum->calledNumInfoPara.numPlan, digit, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCallingNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CallingNumInfoInd *callingNum = VOS_NULL_PTR;
    VOS_UINT8                  digit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                    memResult;

    callingNum = (TAF_CCM_CallingNumInfoInd *)msg;

    callingNum->callIngNumInfoPara.digitNum = AT_MIN(callingNum->callIngNumInfoPara.digitNum,
                                                     TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM);
    /* 初始化 */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));

    if (callingNum->callIngNumInfoPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), callingNum->callIngNumInfoPara.digit,
                             callingNum->callIngNumInfoPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), callingNum->callIngNumInfoPara.digitNum);
    }

    /* 在pstCallingNum->aucDigit的最后一位加'\0',防止因pstCallingNum->aucDigit无结束符，导致AT多上报 */
    digit[callingNum->callIngNumInfoPara.digitNum] = '\0';

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d,%d,%d,%s%s", g_atCrLf,
            g_atStringTab[AT_STRING_CCALLINGNUM].text, callingNum->callIngNumInfoPara.numType,
            callingNum->callIngNumInfoPara.numPlan, callingNum->callIngNumInfoPara.pi,
            callingNum->callIngNumInfoPara.si, digit, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmRedirNumInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_RedirNumInfoInd *redirNumInfo = VOS_NULL_PTR;
    VOS_UINT8                digit[TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t                  memResult;
    VOS_UINT16               length = 0;

    redirNumInfo = (TAF_CCM_RedirNumInfoInd *)msg;

    redirNumInfo->redirNumInfoIndPara.digitNum = AT_MIN(redirNumInfo->redirNumInfoIndPara.digitNum,
                                                        TAF_CALL_MAX_REDIRECTING_NUMBER_CHARI_OCTET_NUM);
    /* 初始化 */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
    if (redirNumInfo->redirNumInfoIndPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), redirNumInfo->redirNumInfoIndPara.digitNumArray,
                             redirNumInfo->redirNumInfoIndPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), redirNumInfo->redirNumInfoIndPara.digitNum);
    }

    /* 在pstRedirNumInfo->aucDigitNum的最后一位加'\0',防止因pstRedirNumInfo->aucDigitNum无结束符，导致AT多上报 */
    digit[redirNumInfo->redirNumInfoIndPara.digitNum] = '\0';

    /* 输出查询结果，根据EXTENSIONBIT1，EXTENSIONBIT2输出可选项 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d,%d,%s", g_atCrLf, g_atStringTab[AT_STRING_CREDIRNUM].text,
        redirNumInfo->redirNumInfoIndPara.numType, redirNumInfo->redirNumInfoIndPara.numPlan, digit);

    if (redirNumInfo->redirNumInfoIndPara.opPi == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", redirNumInfo->redirNumInfoIndPara.pi);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    if (redirNumInfo->redirNumInfoIndPara.opSi == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", redirNumInfo->redirNumInfoIndPara.si);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    if (redirNumInfo->redirNumInfoIndPara.opRedirReason == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", redirNumInfo->redirNumInfoIndPara.redirReason);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmSignalInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SignalInfoInd *pstsignalInfo = (TAF_CCM_SignalInfoInd *)msg;

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s%s%d,%d,%d%s", g_atCrLf, g_atStringTab[AT_STRING_CSIGTONE].text,
            pstsignalInfo->signalInfoIndPara.signalType, pstsignalInfo->signalInfoIndPara.alertPitch,
            pstsignalInfo->signalInfoIndPara.signal, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmLineCtrlInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_LineCtrlInfoInd *lineCtrlInfo = VOS_NULL_PTR;
    VOS_UINT16               length = 0;

    lineCtrlInfo = (TAF_CCM_LineCtrlInfoInd *)msg;

    /* 输出查询结果 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%d", g_atCrLf, g_atStringTab[AT_STRING_CLCTR].text,
        lineCtrlInfo->lineCtrlInfoIndPara.polarityIncluded);

    if (lineCtrlInfo->lineCtrlInfoIndPara.toggleModePresent == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", lineCtrlInfo->lineCtrlInfoIndPara.toggleMode);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    if (lineCtrlInfo->lineCtrlInfoIndPara.reversePolarityPresent == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", lineCtrlInfo->lineCtrlInfoIndPara.reversePolarity);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d%s", lineCtrlInfo->lineCtrlInfoIndPara.powerDenialTime, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmCCWACInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_CcwacInfoInd *ccwac = VOS_NULL_PTR;
    VOS_UINT8             digit[TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1];
    errno_t               memResult;
    VOS_UINT16            length = 0;

    ccwac  = (TAF_CCM_CcwacInfoInd *)msg;

    ccwac->ccwacInfoPara.digitNum = AT_MIN(ccwac->ccwacInfoPara.digitNum,
                                           TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM);
    /* 初始化 */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
    if (ccwac->ccwacInfoPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), ccwac->ccwacInfoPara.digit, ccwac->ccwacInfoPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), ccwac->ccwacInfoPara.digitNum);
    }

    /* 在pstCCWAC->aucDigit的最后一位加'\0',防止因pstCCWAC->aucDigit无结束符，导致AT多上报 */
    if (ccwac->ccwacInfoPara.digitNum < (TAF_CALL_MAX_CALLING_NUMBER_CHARI_OCTET_NUM + 1)) {
        digit[ccwac->ccwacInfoPara.digitNum] = '\0';
    }

    /* 输出查询结果 */
    if (ccwac->ccwacInfoPara.signalIsPresent == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%s,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
            g_atStringTab[AT_STRING_CCWAC].text, digit, ccwac->ccwacInfoPara.pi, ccwac->ccwacInfoPara.si,
            ccwac->ccwacInfoPara.numType, ccwac->ccwacInfoPara.numPlan, ccwac->ccwacInfoPara.signalIsPresent,
            ccwac->ccwacInfoPara.signalType, ccwac->ccwacInfoPara.alertPitch, ccwac->ccwacInfoPara.signal, g_atCrLf);

    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s%s,%d,%d,%d,%d,%d,,,%s", g_atCrLf,
            g_atStringTab[AT_STRING_CCWAC].text, digit, ccwac->ccwacInfoPara.pi, ccwac->ccwacInfoPara.si,
            ccwac->ccwacInfoPara.numType, ccwac->ccwacInfoPara.numPlan, ccwac->ccwacInfoPara.signalIsPresent,
            g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}

VOS_VOID At_XsmsIndProc(VOS_UINT8 indexNum, TAF_XSMS_AppMsgTypeUint32 eventType, TAF_XSMS_AppAtEventInfo *event)
{
    VOS_UINT16    length = 0;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    rslt;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_XsmsIndProc: Get modem id fail.");
        return;
    }

    switch (eventType) {
        case TAF_XSMS_APP_MSG_TYPE_SEND_SUCC_IND:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^CCMGSS: %d%s", g_atCrLf, event->xSmsEvent.sndSuccInd.mr,
                g_atCrLf);
            break;

        case TAF_XSMS_APP_MSG_TYPE_SEND_FAIL_IND:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^CCMGSF: %d%s", g_atCrLf,
                event->xSmsEvent.sndFailInd.courseCode, g_atCrLf);
            break;

        case TAF_XSMS_APP_MSG_TYPE_RCV_IND:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^CCMT:", g_atCrLf);
            /* <length>, */
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%d,\"",
                AT_ASCII_AND_HEX_CONVERSION_FACTOR * sizeof(TAF_XSMS_Message)); /*lint !e559 */
            /* <PDU> */
            length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN,
                (TAF_INT8 *)g_atSndCodeAddress, (TAF_UINT8 *)g_atSndCodeAddress + length,
                (TAF_UINT8 *)&event->xSmsEvent.rcvInd.rcvMsg, sizeof(TAF_XSMS_Message));

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "\"%s", g_atCrLf);

            break;

        case TAF_XSMS_APP_MSG_TYPE_UIM_FULL_IND:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s^CSMMEMFULL: \"SM\"%s", g_atCrLf, g_atCrLf);
            break;

        default:
            AT_WARN_LOG("At_XsmsIndProc: Abnormal EventType.");
            return;
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
}

TAF_UINT32 At_ChgXsmsErrorCodeToAt(TAF_UINT32 xsmsError)
{
    switch (xsmsError) {
        case TAF_XSMS_APP_SUCCESS:
            return AT_SUCCESS;

        case TAF_XSMS_APP_FAILURE:
            return AT_ERROR;

        case TAF_XSMS_APP_INIT_NOT_FINISH:
            return AT_CME_SIM_BUSY;

        case TAF_XSMS_APP_INVALID_INDEX:
            return AT_CME_INVALID_INDEX;

        case TAF_XSMS_APP_UIM_FAILURE:
            return AT_CME_SIM_FAILURE;

        case TAF_XSMS_APP_STORE_FULL:
            return AT_CMS_SIM_SMS_STORAGE_FULL;

        case TAF_XSMS_APP_NOT_SUPPORT_1X:
            return AT_CME_1X_RAT_NOT_SUPPORTED;

        case TAF_XSMS_APP_NORMAL_VOLTE:
        case TAF_XSMS_APP_UIM_LOCK_LOCKED:
        case TAF_XSMS_APP_ENCODE_ERROR:
        case TAF_XSMS_APP_UIM_MSG_STATUS_WRONG:
        case TAF_XSMS_APP_INSERT_SDND_POOL_FAIL:
        case TAF_XSMS_APP_NOT_INSERT_TO_POOL:
        case TAF_XSMS_APP_NOT_SUPPORT_LTE:
            return AT_ERROR;

        default:
            return AT_ERROR;
    }
}

VOS_VOID At_XsmsCnfProc(VOS_UINT8 indexNum, TAF_XSMS_AppMsgTypeUint32 eventType, TAF_XSMS_AppAtEventInfo *event)
{
    TAF_UINT32 result = AT_FAILURE;
    TAF_UINT16 length = 0;

    if (event->error != TAF_ERR_NO_ERROR) { /* 错误 */
        AT_StopTimerCmdReady(indexNum);

        result = At_ChgXsmsErrorCodeToAt(event->error); /* 发生错误 */

        g_atSendDataBuff.bufLen = length;

        At_FormatResultData(indexNum, result);

        return;
    }

    switch (eventType) {
        /* 什么都不做，就等打印OK */
        case TAF_XSMS_APP_MSG_TYPE_SEND_CNF:
            break;

        case TAF_XSMS_APP_MSG_TYPE_WRITE_CNF:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "^CCMGW: %d", event->xSmsEvent.writeCnf.indexNum - 1);
            break;

        /* 什么都不做，就等打印OK */
        case TAF_XSMS_APP_MSG_TYPE_DELETE_CNF:

            break;

        /* 什么都不做，就等打印OK */
        case TAF_XSMS_APP_MSG_TYPE_UIM_MEM_FULL_CNF:

            break;

        default:
            return;
    }

    result = AT_OK;

    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, result);
}

VOS_VOID AT_ProcXsmsMsg(struct MsgCB *msg)
{
    TAF_XSMS_AppAtCnf *msgTemp  = VOS_NULL_PTR;
    VOS_UINT8          indexNum = 0;

    msgTemp = (TAF_XSMS_AppAtCnf *)msg;
    /* 消息类型不正确 */
    if (msgTemp->eventType >= TAF_XSMS_APP_MSG_TYPE_BUTT) {
        AT_ERR_LOG1("AT_ProcXsmsMsg: The Msg Id is Wrong", msgTemp->eventType);

        return;
    }

    AT_LOG1("AT_ProcXsmsMsg ClientId", msgTemp->xsmsAtEvent.clientId);
    AT_LOG1("AT_ProcXsmsMsg OpId", msgTemp->xsmsAtEvent.opId);

    if (At_ClientIdToUserId(msgTemp->xsmsAtEvent.clientId, &indexNum) == AT_FAILURE) {
        AT_ERR_LOG("AT_ProcXsmsMsg At_ClientIdToUserId FAILURE");

        return;
    }

    /* 广播消息 */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        At_XsmsIndProc(indexNum, msgTemp->eventType, &msgTemp->xsmsAtEvent);

        AT_NORM_LOG("At_PIHMsgProc : AT_BROADCAST_INDEX.");

        return;
    }

    AT_LOG1("At_PbMsgProc ucIndex", indexNum);

    /* AT命令回复处理 */
    At_XsmsCnfProc(indexNum, msgTemp->eventType, &msgTemp->xsmsAtEvent);
}

VOS_UINT32 AT_RcvMmaCTimeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CtimeInd *cTimeInd = (TAF_MMA_CtimeInd *)msg;
    VOS_INT8          timeZone;

    /* 底层上报的cTimeZone是以30分钟为单位的，AP要求以15分钟为单位，乘2是为了将15分钟为单位转化为30分钟为单位 */
    if (cTimeInd->timezoneCTime.timeZone < 0) {
        timeZone = (VOS_INT8)(-1 * 2 * cTimeInd->timezoneCTime.timeZone); /* -1是为了将负的cTimeZone转换为正值 */
    } else {
        timeZone = (VOS_INT8)(2 * cTimeInd->timezoneCTime.timeZone);
    }

    /* 时间显示格式: ^CTIME: "yy/mm/dd,hh:mm:ss(+/-)tz,dst" */
    if (cTimeInd->timezoneCTime.timeZone >= 0) {
         /* cTimeInd->timezoneCTime.year % 100是为了将年份四位表示的形式转换为两位的形式表示 */
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s^CTIME: \"%02d/%02d/%02d,%02d:%02d:%02d+%d,%02d\"%s", g_atCrLf,
                cTimeInd->timezoneCTime.year % 100, cTimeInd->timezoneCTime.month, cTimeInd->timezoneCTime.day,
                cTimeInd->timezoneCTime.hour, cTimeInd->timezoneCTime.minute, cTimeInd->timezoneCTime.second, timeZone,
                cTimeInd->timezoneCTime.dayltSavings, g_atCrLf);
    } else {
        /* cTimeInd->timezoneCTime.year % 100是为了将年份四位表示的形式转换为两位的形式表示 */
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s^CTIME: \"%02d/%02d/%02d,%02d:%02d:%02d-%d,%02d\"%s", g_atCrLf,
                cTimeInd->timezoneCTime.year % 100, cTimeInd->timezoneCTime.month, cTimeInd->timezoneCTime.day,
                cTimeInd->timezoneCTime.hour, cTimeInd->timezoneCTime.minute, cTimeInd->timezoneCTime.second, timeZone,
                cTimeInd->timezoneCTime.dayltSavings, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmRcvContinuousDtmfInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_ContDtmfInd *rcvContDtmf = VOS_NULL_PTR;
    VOS_UINT8            digit[AT_DIGIT_MAX_LENGTH];

    rcvContDtmf = (TAF_CCM_ContDtmfInd *)msg;

    /*
     * Initialize aucDigit[0] with pstRcvContDtmf->ucDigit and  aucDigit[1] = '\0'
     * Because AT_FormatReportString does not allow to print pstRcvContDtmf->ucDigit with %c
     * Hence, need to convert digit into string and print as string
     */
    digit[0] = rcvContDtmf->contDtmfIndPara.digit;
    digit[1] = '\0';

    /* Output the inquire result */
    if (rcvContDtmf->contDtmfIndPara.dtmfSwitch == TAF_CALL_CONT_DTMF_START) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CCONTDTMF: %d,%d,\"%s\"%s", g_atCrLf, rcvContDtmf->ctrl.callId,
            rcvContDtmf->contDtmfIndPara.dtmfSwitch, digit, g_atCrLf);
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CCONTDTMF: %d,%d%s", g_atCrLf, rcvContDtmf->ctrl.callId,
            rcvContDtmf->contDtmfIndPara.dtmfSwitch, g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
AT_EncryptVoiceErrorUint32 AT_MapEncVoiceErr(TAF_CALL_EncryptVoiceStatusUint32 tafEncVoiceErr)
{
    VOS_UINT32                          i;
    const AT_EncryptVoiceErrCodeMap    *atEncVoiceErrMapTbl = VOS_NULL_PTR;
    VOS_UINT32                          atEncVoiceErrMapSize;

    atEncVoiceErrMapTbl  = AT_GET_ENC_VOICE_ERR_CODE_MAP_TBL_PTR();
    atEncVoiceErrMapSize = AT_GET_ENC_VOICE_ERR_CODE_MAP_TBL_SIZE();

    for (i = 0; i < atEncVoiceErrMapSize; i++) {
        if (atEncVoiceErrMapTbl[i].tafEncErr == tafEncVoiceErr) {
            return atEncVoiceErrMapTbl[i].atEncErr;
        }
    }
    return AT_ENCRYPT_VOICE_ERROR_ENUM_BUTT;
}

VOS_UINT32 AT_RcvTafCcmEncryptCallInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EncryptVoiceInd   *rcvMsg = VOS_NULL_PTR;
    VOS_UINT8                  asciiNum[MN_CALL_MAX_CALLED_BCD_NUM_LEN + 1];
    AT_EncryptVoiceErrorUint32 ecnErr;
    errno_t                    memResult;
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
    TAF_CHAR  acNewDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN];
    VOS_INT32 length = 0;

    (VOS_VOID)memset_s(acNewDocName, (VOS_SIZE_T)sizeof(acNewDocName), 0x00, (VOS_SIZE_T)sizeof(acNewDocName));
#endif

    /* 初始化消息变量 */
    rcvMsg    = (TAF_CCM_EncryptVoiceInd *)msg;
    memResult = memset_s(asciiNum, sizeof(asciiNum), 0x00, MN_CALL_MAX_CALLED_BCD_NUM_LEN + 1);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(asciiNum), MN_CALL_MAX_CALLED_BCD_NUM_LEN + 1);

    g_atSendDataBuff.bufLen = 0;

    ecnErr = AT_MapEncVoiceErr(rcvMsg->encryptVoiceIndPara.eccVoiceStatus);

    /* AT^ECCALL ind命令返回 */
    if (ecnErr == AT_ENCRYPT_VOICE_MT_CALL_NOTIFICATION) {
        /* 密话被叫主动上报 */
        AT_BcdNumberToAscii(rcvMsg->encryptVoiceIndPara.callingNumber.bcdNum,
                            AT_MIN(rcvMsg->encryptVoiceIndPara.callingNumber.numLen, TAF_ECC_CALL_MAX_BCD_NUM_LEN),
                            (VOS_CHAR *)asciiNum);
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCALL: %d,\"%s\"%s", g_atCrLf, ecnErr, asciiNum, g_atCrLf);
    } else if (ecnErr == AT_ENCRYPT_VOICE_ERROR_ENUM_BUTT) {
        /* 异常场景 */
        return VOS_ERR;
    } else {
        /* 密话主叫结果其他原因值上报 */
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCALL: %d%s", g_atCrLf, ecnErr, g_atCrLf);

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
        length = snprintf_s(acNewDocName, (VOS_SIZE_T)sizeof(acNewDocName), (VOS_SIZE_T)(sizeof(acNewDocName) - 1),
                            MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data%d", g_atCurrEncVoiceTestFileNum);
        if (length <= 0) {
            AT_WARN_LOG("AT_RcvTafCcmEncryptCallInd(): snprintf_s len <= 0");

            return VOS_ERR;
        }

        length = snprintf_s((VOS_CHAR *)(acNewDocName + length), ((VOS_SIZE_T)sizeof(acNewDocName) - length),
                            ((VOS_SIZE_T)sizeof(acNewDocName) - length - 1), "_[%d]records.txt",
                            g_atCurrEncVoiceDataCount);

        if (length <= 0) {
            AT_WARN_LOG("AT_RcvTafCcmEncryptCallInd(): snprintf_s len <= 0 (2)");

            return VOS_ERR;
        }

        if (ecnErr != AT_ENCRYPT_VOICE_SUCC) {
            (VOS_VOID)at_file_rename(g_currEncVoiceDataWriteFilePath[g_currEncVoiceDataWriteFileNum], acNewDocName);

            memResult = memcpy_s(g_atCurrDocName, (VOS_SIZE_T)sizeof(g_atCurrDocName), acNewDocName,
                                 (VOS_SIZE_T)sizeof(acNewDocName));
            TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(g_atCurrDocName), (VOS_SIZE_T)sizeof(acNewDocName));
        }
#endif
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmEccRemoteCtrlInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EcRemoteCtrlInd *rcvMsg = VOS_NULL_PTR;

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_EcRemoteCtrlInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCTRL: %d%s", g_atCrLf, rcvMsg->remoteCtrlType, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEncryptedVoiceDataInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    VOS_INT32 ret;
    int       fd;
    TAF_CHAR *dirPath = MODEM_LOG_ROOT "/ECC_TEST";
    TAF_CCM_EncryptedVoiceDataInd *rcvMsg = VOS_NULL_PTR;
    TAF_CHAR  acNewDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN];
    TAF_CHAR  acTimeDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN];
    VOS_INT32 length;

    rcvMsg = (TAF_CCM_EncryptedVoiceDataInd *)msg;
    (VOS_VOID)memset_s(acNewDocName, (VOS_SIZE_T)sizeof(acNewDocName), 0x00, (VOS_SIZE_T)sizeof(acNewDocName));
    length = 0;

    /* 如果目录不存在则先创建目录 */
    if (at_file_access(dirPath, 0) != VOS_OK) {
        if (at_file_mkdir(dirPath) != VOS_OK) {
            AT_WARN_LOG("AT_RcvTafCcmEncryptedVoiceDataInd: mkdir /data/hisi_logs/modem_log/ECC_TEST failed.\n");
            return VOS_ERR;
        }
    }

    if (rcvMsg->encVoiceDataIndPara.isTimeInfo == VOS_TRUE) {
        (VOS_VOID)memset_s(acTimeDocName, (VOS_SIZE_T)sizeof(acTimeDocName), 0x00, (VOS_SIZE_T)sizeof(acTimeDocName));

        length = snprintf_s(acTimeDocName, (VOS_SIZE_T)sizeof(acTimeDocName), (VOS_SIZE_T)(sizeof(acTimeDocName) - 1),
                            MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_execute_time%d.txt", g_atCurrEncVoiceTestFileNum);
        if (length <= 0) {
            AT_WARN_LOG("AT_RcvTafCcmEncryptedVoiceDataInd(): snprintf_s len <= 0");

            return VOS_ERR;
        }

        fd = at_file_open(acTimeDocName, "a+");
    } else {
        if (PS_MEM_CMP(acNewDocName, g_atCurrDocName, sizeof(acNewDocName)) != 0) {
            fd = at_file_open(g_atCurrDocName, "a+");
        } else {
            fd = at_file_open(g_currEncVoiceDataWriteFilePath[g_currEncVoiceDataWriteFileNum], "a+");
        }
    }

    if (fd < 0) {
        AT_WARN_LOG("AT_RcvTafCcmEncryptedVoiceDataInd: open file failed.\n");
        return VOS_ERR;
    }

    ret = at_file_write(fd, rcvMsg->encVoiceDataIndPara.data, rcvMsg->encVoiceDataIndPara.length);
    if (ret != rcvMsg->encVoiceDataIndPara.length) {
        AT_WARN_LOG("AT_RcvTafCcmEncryptedVoiceDataInd: write file failed.\n");
        at_file_close(fd);
        return VOS_ERR;
    }

    if (rcvMsg->encVoiceDataIndPara.isTimeInfo == VOS_FALSE) {
        g_atCurrEncVoiceDataCount++;
    }

    at_file_close(fd);


    return VOS_OK;
}
#endif
#endif

VOS_UINT32 AT_RcvTafCcmPrivacyModeInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_PrivacyModeInd *rcvMsg = VOS_NULL_PTR;

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_PrivacyModeInd *)msg;

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CPMP: %d,%d,%d%s", g_atCrLf, rcvMsg->privacyModeIndPara.privacyMode,
            rcvMsg->privacyModeIndPara.callVoicePrivacyInfo.callId,
            rcvMsg->privacyModeIndPara.callVoicePrivacyInfo.privacyMode, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaRoamingModeSwitchInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ROAMINGMODESWITCH%s", g_atCrLf, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaIccAppTypeSwitchInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_IccAppTypeSwitchInd *iccAppTypeSwitchInd = (TAF_MMA_IccAppTypeSwitchInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ICCAPPTYPESWITCH: %d%s", g_atCrLf, iccAppTypeSwitchInd->switchState,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaCLImsCfgInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ClimsCfgInfoInd *clImsCfgInfo = (TAF_MMA_ClimsCfgInfoInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CLIMSCFG: %d,%d,%d%s", g_atCrLf, clImsCfgInfo->clImsSupportFlag,
        clImsCfgInfo->clVolteMode, clImsCfgInfo->lteImsSupportFlag, g_atCrLf);

    /* 调用At_SendResultData发送命令结果 */
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}
#endif

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
VOS_UINT32 AT_RcvXpdsEphInfoInd(struct MsgCB *msg)
{
    VOS_UINT8              indexNum;
    VOS_UINT16             atCmdLength;
    VOS_UINT32             ephDataStrLen;
    VOS_UINT32             ephDataByteOffset;
    VOS_UINT8              loop;
    XPDS_AT_GpsEphInfoInd *msgEphInfoInd = VOS_NULL_PTR;
    VOS_UINT8             *ephData       = VOS_NULL_PTR;

    /* 初始化 */
    msgEphInfoInd = (XPDS_AT_GpsEphInfoInd *)msg;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msgEphInfoInd->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsEphInfoInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvXpdsEphInfoInd: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    ephData = ((VOS_UINT8 *)(msgEphInfoInd)) + VOS_MSG_HEAD_LENGTH + sizeof(VOS_UINT32) + sizeof(AT_APPCTRL);

    ephDataStrLen = AT_AGPS_EPH_INFO_FIRST_SEG_STR_LEN;

    /* 第1包数据 */
    atCmdLength = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSEPHINFO: %d,%d,%d,", g_atCrLf, AT_AGPS_EPH_INFO_MAX_SEG_NUM, 0,
        ephDataStrLen);

    /* 第1包数据 */
    /* 将16进制数转换为ASCII码后输入主动命令内容 */
    /* 第1包数据字节长度为8 / 2 = 4 */
    atCmdLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                            ((TAF_UINT8 *)g_atSndCodeAddress) + atCmdLength,
                                                            ephDataStrLen / 2, ephData);

    atCmdLength += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + atCmdLength, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, atCmdLength);

    ephDataByteOffset = ephDataStrLen / 2;

    /* 从第2包到第5包的数据 */
    /* 第2包到第5包数据的字节长度均为960 / 2 = 480 */
    for (loop = 0; loop < AT_AGPS_EPH_INFO_MAX_SEG_NUM - 1; loop++) {
        ephDataStrLen = AT_AGPS_EPH_INFO_NOT_FIRST_SEG_STR_LEN;

        atCmdLength = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSEPHINFO: %d,%d,%d,", g_atCrLf, AT_AGPS_EPH_INFO_MAX_SEG_NUM,
            loop + 1, ephDataStrLen);

        /* 将16进制数转换为ASCII码后输入主动命令内容 */
        atCmdLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                                ((TAF_UINT8 *)g_atSndCodeAddress) + atCmdLength,
                                                                ephDataStrLen / 2, ephData + ephDataByteOffset);

        atCmdLength += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + atCmdLength, "%s", g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, atCmdLength);

        ephDataByteOffset += ephDataStrLen / 2;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvXpdsAlmInfoInd(struct MsgCB *msg)
{
    VOS_UINT8              indexNum;
    VOS_UINT16             atCmdLength;
    VOS_UINT32             almDataStrLen;
    VOS_UINT32             almDataByteOffset;
    VOS_UINT8              loop;
    XPDS_AT_GpsAlmInfoInd *msgAlmInfoInd = VOS_NULL_PTR;
    VOS_UINT8             *almData       = VOS_NULL_PTR;

    /* 初始化 */
    msgAlmInfoInd = (XPDS_AT_GpsAlmInfoInd *)msg;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msgAlmInfoInd->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsAlmInfoInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvXpdsAlmInfoInd: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    almData = ((VOS_UINT8 *)(msgAlmInfoInd)) + VOS_MSG_HEAD_LENGTH + sizeof(VOS_UINT32) + sizeof(AT_APPCTRL);

    almDataStrLen = AT_AGPS_ALM_INFO_FIRST_SEG_STR_LEN;

    /* 第1包数据 */
    atCmdLength = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSALMINFO: %d,%d,%d,", g_atCrLf, AT_AGPS_ALM_INFO_MAX_SEG_NUM, 0,
        almDataStrLen);

    /* 第1包数据 */
    /* 将16进制数转换为ASCII码后输入主动命令内容 */
    /* 第1包数据字节长度为8 / 2 = 4 */
    atCmdLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                            ((TAF_UINT8 *)g_atSndCodeAddress) + atCmdLength,
                                                            almDataStrLen / 2, almData);

    atCmdLength += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + atCmdLength, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, atCmdLength);

    almDataByteOffset = almDataStrLen / 2;

    /* 从第2包到第3包的数据 */
    /* 第2包数据和第3包数据的字节长度为896 / 2 = 448 */
    for (loop = 0; loop < AT_AGPS_ALM_INFO_MAX_SEG_NUM - 1; loop++) {
        almDataStrLen = AT_AGPS_ALM_INFO_NOT_FIRST_SEG_STR_LEN;

        atCmdLength = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSALMINFO: %d,%d,%d,", g_atCrLf, AT_AGPS_ALM_INFO_MAX_SEG_NUM,
            loop + 1, almDataStrLen);

        /* 将16进制数转换为ASCII码后输入主动命令内容 */
        atCmdLength += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                                ((TAF_UINT8 *)g_atSndCodeAddress) + atCmdLength,
                                                                almDataStrLen / 2, almData + almDataByteOffset);

        atCmdLength += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + atCmdLength, "%s", g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, atCmdLength);

        almDataByteOffset += almDataStrLen / 2;
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvXpdsAgpsDataCallReq(struct MsgCB *msg)
{
    VOS_UINT8              indexNum;
    XPDS_AT_ApDataCallReq *msgDataCallReq = VOS_NULL_PTR;

    /* 初始化 */
    msgDataCallReq = (XPDS_AT_ApDataCallReq *)msg;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msgDataCallReq->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsAgpsDataCallReq: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSDATACALLREQ: %d,%d%s", g_atCrLf, msgDataCallReq->agpsMode,
        msgDataCallReq->agpsOper, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvXpdsAgpsServerBindReq(struct MsgCB *msg)
{
    VOS_UINT8                indexNum;
    XPDS_AT_ApServerBindReq *msgBindReq = VOS_NULL_PTR;

    /* 初始化 */
    msgBindReq = (XPDS_AT_ApServerBindReq *)msg;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msgBindReq->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsAgpsServerBindReq: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSBINDCONN: %d,%d,%08x,%d%s", g_atCrLf, msgBindReq->serverMode,
        msgBindReq->dataCallCtrl, msgBindReq->ipAddr, msgBindReq->portNum, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvXpdsAgpsReverseDataInd(struct MsgCB *msg)
{
    VOS_UINT8                 indexNum;
    VOS_UINT32                curNum;
    VOS_UINT32                totalNum;
    VOS_UINT32                offset;
    VOS_UINT32                outputLen;
    VOS_UINT32                remainLen;
    VOS_UINT16                length;
    VOS_UINT8                *src        = VOS_NULL_PTR;
    XPDS_AT_ApReverseDataInd *msgDataInd = VOS_NULL_PTR;

    /* 初始化 */
    msgDataInd = (XPDS_AT_ApReverseDataInd *)msg;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msgDataInd->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsAgpsReverseDataInd: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    totalNum = (msgDataInd->dataLen / AT_XPDS_AGPS_DATAUPLEN_MAX);

    if ((msgDataInd->dataLen % AT_XPDS_AGPS_DATAUPLEN_MAX) != 0) {
        totalNum++;
    }

    if (totalNum >= VOS_NULL_WORD) {
        AT_WARN_LOG("AT_RcvXpdsAgpsReverseDataInd: Data Len Error.");
        return VOS_ERR;
    }

    offset = 0;

    src = msgDataInd->data;

    outputLen = 0;

    remainLen = msgDataInd->dataLen;

    for (curNum = 1; curNum <= totalNum; curNum++) {
        if (remainLen <= AT_XPDS_AGPS_DATAUPLEN_MAX) {
            outputLen = remainLen;
        } else {
            outputLen = AT_XPDS_AGPS_DATAUPLEN_MAX;

            remainLen -= AT_XPDS_AGPS_DATAUPLEN_MAX;
        }

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s^CAGPSREVERSEDATA: %d,%d,%d,%d,", g_atCrLf, msgDataInd->serverMode,
            totalNum, curNum, outputLen * AT_ASCII_AND_HEX_CONVERSION_FACTOR);

        /* 将16进制数转换为ASCII码后输入主动命令内容 */
        length += (TAF_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                           (TAF_UINT8 *)g_atSndCodeAddress + length, outputLen,
                                                           (src + offset));

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        offset += AT_XPDS_AGPS_DATAUPLEN_MAX;

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    return VOS_OK;
}

VOS_VOID AT_ProcXpdsMsg(struct MsgCB *msg)
{
    VOS_UINT32 i;
    VOS_UINT32 msgCnt;
    VOS_UINT32 msgId;
    VOS_UINT32 rst;

    /* 从g_atProcMsgFromXpdsTab中获取消息个数 */
    msgCnt = sizeof(g_atProcMsgFromXpdsTab) / sizeof(AT_PROC_MsgFromXpds);

    /* 从消息包中获取MSG ID */
    msgId = ((AT_XPDS_Msg *)msg)->msgId;

    /* g_atProcMsgFromXpdsTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcMsgFromXpdsTab[i].msgType == msgId) {
            rst = g_atProcMsgFromXpdsTab[i].procMsgFunc(msg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcXpdsMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("At_ProcXpdsMsg: Msg Id is invalid!");
    }
}
#endif

VOS_UINT32 AT_RcvMmaCtRoamingInfoChgInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CtccRoamingNwInfoReportInd *multiModeSidMccChgInd = (TAF_MMA_CtccRoamingNwInfoReportInd *)msg;

    /* 格式化AT^CSNID查询命令返回 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CTROAMINFO: %d,%x%x%x,%x%x%x,%d%s", g_atCrLf, multiModeSidMccChgInd->sid,
        (multiModeSidMccChgInd->ul3Gpp2Mcc & 0x0f00) >> 8, (multiModeSidMccChgInd->ul3Gpp2Mcc & 0xf0) >> 4,
        (multiModeSidMccChgInd->ul3Gpp2Mcc & 0x0f), (multiModeSidMccChgInd->ul3GppMcc & 0x0f00) >> 8,
        (multiModeSidMccChgInd->ul3GppMcc & 0xf0) >> 4, (multiModeSidMccChgInd->ul3GppMcc & 0x0f),
        multiModeSidMccChgInd->modeType, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafCcmRcvBurstDtmfInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_BurstDtmfInd *rcvBurstDtmf = VOS_NULL_PTR;
    VOS_UINT8             digit[TAF_CALL_MAX_BURST_DTMF_NUM + 1];
    errno_t               memResult;

    rcvBurstDtmf = (TAF_CCM_BurstDtmfInd *)msg;

    rcvBurstDtmf->burstDtmfIndPara.digitNum = (VOS_UINT8)AT_MIN(rcvBurstDtmf->burstDtmfIndPara.digitNum,
                                                     TAF_CALL_MAX_BURST_DTMF_NUM);
    /* initialization */
    (VOS_VOID)memset_s(digit, sizeof(digit), 0x00, sizeof(digit));
    if (rcvBurstDtmf->burstDtmfIndPara.digitNum > 0) {
        memResult = memcpy_s(digit, sizeof(digit), rcvBurstDtmf->burstDtmfIndPara.digit,
                             rcvBurstDtmf->burstDtmfIndPara.digitNum);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(digit), rcvBurstDtmf->burstDtmfIndPara.digitNum);
    }

    /* Add the '\0' to the last byte of pstRcvBurstDtmf->aucDigit */
    digit[rcvBurstDtmf->burstDtmfIndPara.digitNum] = '\0';

    /* Output the inquire result */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CBURSTDTMF: %d,\"%s\",%d,%d%s", g_atCrLf, rcvBurstDtmf->ctrl.callId, digit,
        rcvBurstDtmf->burstDtmfIndPara.onLength, rcvBurstDtmf->burstDtmfIndPara.offLength, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaEvdoRevARLinkInfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    VOS_UINT32                   dataLen;
    VOS_UINT32                   rslt;
    AT_MTA_Msg                  *mtaMsg    = VOS_NULL_PTR;
    MTA_AT_EvdoRevaRlinkInfoInd *rlinkInfo = VOS_NULL_PTR;
    VOS_UINT8                   *data      = VOS_NULL_PTR;

    /* 初始化消息变量 */
    mtaMsg    = (AT_MTA_Msg *)msg;
    rlinkInfo = (MTA_AT_EvdoRevaRlinkInfoInd *)mtaMsg->content;

    /* 前后两个双引号加\0的长度为3 */
    dataLen = rlinkInfo->paraLen * sizeof(VOS_UINT8) * AT_DOUBLE_LENGTH + (AT_DOUBLE_QUO_MARKS_LENGTH + 1);
    /*lint -save -e516 */
    data = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, dataLen);
    /*lint -restore */
    if (data == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd(): mem alloc Fail!");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s(data, dataLen, 0x00, dataLen);
    data[dataLen - 1] = '\0';

    /* 前后两个双引号加\0的长度为3 */
    rslt = AT_HexToAsciiString(&data[1], (dataLen - (AT_DOUBLE_QUO_MARKS_LENGTH + 1)),
                               rlinkInfo->content, rlinkInfo->paraLen);

    data[0]                                    = '"';
    data[dataLen - AT_DOUBLE_QUO_MARKS_LENGTH] = '"';

    if (rslt != AT_OK) {
        AT_WARN_LOG("AT_RcvMtaEvdoRevARLinkInfoInd: WARNING: Hex to Ascii trans fail!");

        PS_MEM_FREE(WUEPS_PID_AT, data);
        data = VOS_NULL_PTR;

        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^DOREVARLINK: %d,%s%s",
        g_atCrLf, rlinkInfo->paraLen * AT_DOUBLE_LENGTH, data, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    PS_MEM_FREE(WUEPS_PID_AT, data);
    data = VOS_NULL_PTR;

    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaEvdoSigExEventInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *mtaMsg     = VOS_NULL_PTR;
    MTA_AT_EvdoSigExeventInd *sigExEvent = VOS_NULL_PTR;
    VOS_UINT8                *data       = VOS_NULL_PTR;
    VOS_UINT32                dataLen;
    VOS_UINT32                rslt;

    /* 初始化消息变量 */
    mtaMsg     = (AT_MTA_Msg *)msg;
    sigExEvent = (MTA_AT_EvdoSigExeventInd *)mtaMsg->content;

    /* 前后两个双引号加\0的长度为3 */
    dataLen = sigExEvent->paraLen * sizeof(VOS_UINT8) * AT_DOUBLE_LENGTH + (AT_DOUBLE_QUO_MARKS_LENGTH + 1);
    /*lint -save -e516 */
    data = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, dataLen);
    /*lint -restore */
    if (data == VOS_NULL_PTR) {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd(): mem alloc Fail!");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s(data, dataLen, 0x00, dataLen);
    data[dataLen - 1] = '\0';

    /* 前后两个双引号加\0的长度为3 */
    rslt = AT_HexToAsciiString(&data[1], (dataLen - (AT_DOUBLE_QUO_MARKS_LENGTH + 1)),
                               sigExEvent->content, sigExEvent->paraLen);

    data[0]           = '"';
    data[dataLen - AT_DOUBLE_QUO_MARKS_LENGTH] = '"';

    if (rslt != AT_OK) {
        AT_WARN_LOG("AT_RcvMtaEvdoSigExEventInd: WARNING: Hex to Ascii trans fail!");

        PS_MEM_FREE(WUEPS_PID_AT, data);
        data = VOS_NULL_PTR;

        return VOS_ERR;
    }

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^DOSIGEXEVENT: %d,%s%s", g_atCrLf,
        sigExEvent->paraLen * AT_DOUBLE_LENGTH, data, g_atCrLf);

    PS_MEM_FREE(WUEPS_PID_AT, data);
    data = VOS_NULL_PTR;

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaCSidInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CsidInd *cSidInd = (TAF_MMA_CsidInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CSID: %d%s", g_atCrLf, cSidInd->sid, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaCombinedModeSwitchInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CombinedModeSwitchInd *combinedModeSwitchInd = (TAF_MMA_CombinedModeSwitchInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^RATCOMBINEDMODE: %d%s", g_atCrLf, combinedModeSwitchInd->combinedMode,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaClocinfoInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ClocinfoInd *clocinfoInd = (TAF_MMA_ClocinfoInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CLOCINFO: %d,%d,%d,%d,%d,%d,%d,%d,%d%s", g_atCrLf,
        clocinfoInd->clocinfoPara.ratMode, clocinfoInd->clocinfoPara.prevInUse, clocinfoInd->clocinfoPara.mcc,
        clocinfoInd->clocinfoPara.mnc, clocinfoInd->clocinfoPara.sid, clocinfoInd->clocinfoPara.nid,
        clocinfoInd->clocinfoPara.baseId, clocinfoInd->clocinfoPara.baseLatitude,
        clocinfoInd->clocinfoPara.baseLongitude, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaCdmaCsqInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CdmacsqInd *cdmaCsqInd = (TAF_MMA_CdmacsqInd *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CDMACSQ: %d,%d%s", g_atCrLf, cdmaCsqInd->cdmaRssi, cdmaCsqInd->cdmaEcIo,
        g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaCLModInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ClmodeInd *cLModeInd = (TAF_MMA_ClmodeInd *)msg;
    ModemIdUint16      modemId;
    VOS_UINT32         rslt;
    VOS_UINT8         *isCLMode = VOS_NULL_PTR;

    modemId = MODEM_ID_0;

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RcvMmaCLModInd: Get modem id fail.");
        return VOS_ERR;
    }

    isCLMode = AT_GetModemCLModeCtxAddrFromModemId(modemId);

    *isCLMode = cLModeInd->isCLMode;
    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaClDbdomainStatusReportInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ClDbdomainStatusInfoInd *clDbdomainStaRInd = (TAF_MMA_ClDbdomainStatusInfoInd *)msg;

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CLDBDOMAINSTATUS: %d,%d,%d%s", g_atCrLf,
            clDbdomainStaRInd->enableReportFlag, clDbdomainStaRInd->uc1xSrvStatus, clDbdomainStaRInd->imsVoiceCap,
            g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaHdrCsqInd(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    MMA_TAF_HdrCsqValueInd *hdrcsqInd = (MMA_TAF_HdrCsqValueInd *)msg;

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^HDRCSQ: %d,%d,%d%s", g_atCrLf, hdrcsqInd->hdrRssi, hdrcsqInd->hdrSnr,
        hdrcsqInd->hdrEcio, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtCgmtuValueChgInd(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_CgmtuValueChgInd *cgmtuChgInd = VOS_NULL_PTR;
    VOS_UINT16               length;

    cgmtuChgInd = (TAF_PS_CgmtuValueChgInd *)evtInfo;
    length      = 0;

    /* ^CGMTU: <curr_mtu_value> */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s^CGMTU: %d%s", g_atCrLf, cgmtuChgInd->mtuValue, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif

