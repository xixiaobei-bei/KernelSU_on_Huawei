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

#include "at_cdma_rslt_proc.h"
#include "securec.h"
#include "at_cdma_event_report.h"
#include "at_cdma_set_cmd_proc.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_event_report.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_drv_agent.h"
#include "at_file_handle.h"
#include "at_event_report.h"
#include "at_external_module_msg_proc.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CDMA_RSLT_PROC_C
#define UIMID_DATA_LEN 4

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
AT_CagpsCmdNameTlb g_atCagpsCmdNameTlb[] = {
    { ID_XPDS_AT_GPS_TIME_INFO_IND, 0, "^CAGPSTIMEINFO" },
    { ID_XPDS_AT_GPS_REFLOC_INFO_CNF, 0, "^CAGPSREFLOCINFO" },
    { ID_XPDS_AT_GPS_PDE_POSI_INFO_IND, 0, "^CAGPSPDEPOSINFO" },
    { ID_XPDS_AT_GPS_NI_SESSION_IND, 0, "^CAGPSNIREQ" },
    { ID_XPDS_AT_GPS_START_REQ, 0, "^CAGPSSTARTREQ" },
    { ID_XPDS_AT_GPS_CANCEL_IND, 0, "^CAGPSCANCELIND" },
    { ID_XPDS_AT_GPS_ACQ_ASSIST_DATA_IND, 0, "^CAGPSACQASSISTINFO" },
    { ID_XPDS_AT_GPS_ABORT_IND, 0, "^CAGPSABORT" },
    { ID_XPDS_AT_GPS_ION_INFO_IND, 0, "^CAGPSIONINFO" },
    { ID_XPDS_AT_GPS_EPH_INFO_IND, 0, "^CAGPSEPHINFO" },
    { ID_XPDS_AT_GPS_DEL_ASSIST_DATA_IND, 0, "^CAGPSDELASSISTDATAIND" },

    { ID_XPDS_AT_GPS_ALM_INFO_IND, 0, "^CAGPSALMINFO" },

    { ID_XPDS_AT_GPS_NI_CP_START, 0, "^CAGPSNICPSTART" },
    { ID_XPDS_AT_GPS_NI_CP_STOP, 0, "^CAGPSNICPSTOP" },
    { ID_XPDS_AT_GPS_UTS_TEST_START_REQ, 0, "^CGPSCONTROLSTART" },
    { ID_XPDS_AT_GPS_UTS_TEST_STOP_REQ, 0, "^CGPSCONTROLSTOP" },

    { ID_XPDS_AT_UTS_GPS_POS_INFO_IND, 0, "^UTSGPSPOSINFO" },
    { ID_XPDS_AT_GPS_OM_TEST_START_REQ, 0, "^CGPSTESTSTART" },
    { ID_XPDS_AT_GPS_OM_TEST_STOP_REQ, 0, "^CGPSTESTSTOP" },
};

AT_CagpsCmdOptTlb g_atCagpsCmdOptTbl[] = {
    { ID_XPDS_AT_GPS_START_CNF, AT_CMD_CAGPSSTART_SET },
    { ID_XPDS_AT_GPS_CFG_MPC_ADDR_CNF, AT_CMD_CAGPSCFGMPCADDR_SET },
    { ID_XPDS_AT_GPS_CFG_PDE_ADDR_CNF, AT_CMD_CAGPSCFGPDEADDR_SET },
    { ID_XPDS_AT_GPS_CFG_MODE_CNF, AT_CMD_CAGPSCFGPOSMODE_SET },
    { ID_XPDS_AT_GPS_STOP_CNF, AT_CMD_CAGPSSTOP_SET },
};
#endif

VOS_VOID AT_ReadPlatformNV(VOS_VOID)
{
    ModemIdUint16           modemID;
    PLATAFORM_RatCapability platFormRat;
    VOS_UINT8               ratIndex;
    AT_ModemSptRat         *atSptRatList = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&platFormRat, sizeof(platFormRat), 0x00, sizeof(PLATAFORM_RatCapability));

    for (modemID = 0; modemID < MODEM_ID_BUTT; modemID++) {
        atSptRatList = AT_GetSptRatFromModemId(modemID);

        /* 读取平台NV成功 */
        if (TAF_ACORE_NV_READ(modemID, NV_ITEM_PLATFORM_RAT_CAP, &platFormRat, sizeof(PLATAFORM_RatCapability)) ==
            NV_OK) {
            atSptRatList->platformSptGsm      = VOS_FALSE;
            atSptRatList->platformSptWcdma    = VOS_FALSE;
            atSptRatList->platformSptLte      = VOS_FALSE;
            atSptRatList->platformSptUtralTdd = VOS_FALSE;

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
            atSptRatList->platformSptNr = VOS_FALSE;
#endif

            platFormRat.ratNum = AT_MIN(platFormRat.ratNum, PLATFORM_MAX_RAT_NUM);
            for (ratIndex = 0; ratIndex < platFormRat.ratNum; ratIndex++) {
#if (FEATURE_LTE == FEATURE_ON)
                /* 平台支持LTE */
                if (platFormRat.ratList[ratIndex] == PLATFORM_RAT_LTE) {
                    atSptRatList->platformSptLte = VOS_TRUE;
                }
#endif
#if (FEATURE_UE_MODE_W == FEATURE_ON)
                /* 平台支持WCDMA */
                if (platFormRat.ratList[ratIndex] == PLATFORM_RAT_WCDMA) {
                    atSptRatList->platformSptWcdma = VOS_TRUE;
                }
#endif

#if (FEATURE_UE_MODE_TDS == FEATURE_ON)
                /* 平台支持TDS */
                if (platFormRat.ratList[ratIndex] == PLATFORM_RAT_TDS) {
                    atSptRatList->platformSptUtralTdd = VOS_TRUE;
                }
#endif

#if (FEATURE_UE_MODE_G == FEATURE_ON)
                /* 平台支持GSM */
                if (platFormRat.ratList[ratIndex] == PLATFORM_RAT_GSM) {
                    atSptRatList->platformSptGsm = VOS_TRUE;
                }
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
                /* 平台支持NR */
                if (platFormRat.ratList[ratIndex] == PLATFORM_RAT_NR) {
                    atSptRatList->platformSptNr = VOS_TRUE;
                }
#endif
            }
        }
    }
}

VOS_UINT32 At_ProcPihCcimiQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_CCIMI_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CCIMI_SET) {
        AT_WARN_LOG("At_ProcPihCcimiQryCnf: CmdCurrentOpt is not AT_CMD_CCIMI_SET!");
        return AT_ERROR;
    }

    /* buf从外部接口返回的，增加结束符，防止缓冲区溢出 */
    event->pihEvent.imsi.imsi[SI_IMSI_MAX_LEN - 1] = '\0';

    g_logPrivacyAtCmd = TAF_LOG_PRIVACY_AT_CMD_CIMI;
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s", event->pihEvent.imsi.imsi);

    return AT_OK;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmSndFlashRslt(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SendFlashCnf *sndFlashRslt = (TAF_CCM_SendFlashCnf *)msg;

    /* 根据临时响应的错误码打印命令的结果 */
    if (sndFlashRslt->result.result == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvTafCcmSndBurstDTMFCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SendBurstDtmfCnf *burstDtmfCnf = (TAF_CCM_SendBurstDtmfCnf *)msg;

    /* 根据临时响应的错误码打印命令的结果 */
    if (burstDtmfCnf->burstDtmfCnfPara.result != TAF_CALL_SEND_BURST_DTMF_CNF_RESULT_SUCCESS) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmSndContinuousDTMFCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SendContDtmfCnf *contDtmfCnf = (TAF_CCM_SendContDtmfCnf *)msg;

    /* According to the error code of temporary respond, printf the result of command */
    if (contDtmfCnf->contDtmfCnfPara.result != TAF_CALL_SEND_CONT_DTMF_CNF_RESULT_SUCCESS) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaCFreqLockSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CfreqLockSetCnf *cFreqLockSetCnf = (TAF_MMA_CfreqLockSetCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (cFreqLockSetCnf->rslt != VOS_TRUE) {
        return AT_ERROR;
    } else {
        return AT_OK;
    }
}

VOS_UINT32 AT_RcvMmaCFreqLockQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CfreqLockQueryCnf *cFreqLockQryCnf = (TAF_MMA_CfreqLockQueryCnf *)msg;
    VOS_UINT16                 length = 0;

    /* 格式化AT^CFREQLOCK查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (cFreqLockQryCnf->cFreqLockPara.freqLockMode == TAF_MMA_CFREQ_LOCK_MODE_OFF) {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            cFreqLockQryCnf->cFreqLockPara.freqLockMode);
    } else {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, cFreqLockQryCnf->cFreqLockPara.freqLockMode,
            cFreqLockQryCnf->cFreqLockPara.sid, cFreqLockQryCnf->cFreqLockPara.nid,
            cFreqLockQryCnf->cFreqLockPara.cdmaBandClass, cFreqLockQryCnf->cFreqLockPara.cdmaFreq,
            cFreqLockQryCnf->cFreqLockPara.cdmaPn, cFreqLockQryCnf->cFreqLockPara.evdoBandClass,
            cFreqLockQryCnf->cFreqLockPara.evdoFreq, cFreqLockQryCnf->cFreqLockPara.evdoPn);
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvMma1xChanQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_1XchanQueryCnf *pst1xChanQueryCnf = (TAF_MMA_1XchanQueryCnf *)msg;
    VOS_UINT16              length = 0;
    VOS_INT16               invalidChan = -1;

    if (pst1xChanQueryCnf->rslt == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            pst1xChanQueryCnf->channel);

        g_atSendDataBuff.bufLen = length;
    } else {
        /* 查询失败，上报^1XCHAN: BUTT */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            invalidChan);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaCdmaCsqSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CdmacsqSetCnf *cdmaCsqCnf = (TAF_MMA_CdmacsqSetCnf *)msg;
    VOS_UINT32             result;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvMmaCdmaCsqSetCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (cdmaCsqCnf->rslt == TAF_MMA_APP_OPER_RESULT_SUCCESS) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvMmaCdmaCsqQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CdmacsqQueryCnf *cdmaCsqQueryCnf = (TAF_MMA_CdmacsqQueryCnf *)msg;

    g_atSendDataBuff.bufLen  = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        cdmaCsqQueryCnf->mode, cdmaCsqQueryCnf->timeInterval, cdmaCsqQueryCnf->rssiRptThreshold,
        cdmaCsqQueryCnf->ecIoRptThreshold, cdmaCsqQueryCnf->sigQualityInfo.cdmaRssi,
        cdmaCsqQueryCnf->sigQualityInfo.cdmaEcIo);

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaHdrCsqSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    MMA_TAF_HdrCsqSetCnf *hdrCsqCnf = (MMA_TAF_HdrCsqSetCnf *)msg;
    VOS_UINT32            result;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvMmaHdrCsqSetCnf : AT_BROADCAST_INDEX.");
        return AT_CMD_NO_NEED_FORMAT_RSLT;
    }

    if (hdrCsqCnf->rslt == TAF_MMA_APP_OPER_RESULT_SUCCESS) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = 0;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_UINT32 AT_RcvMmaHdrCsqQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    MMA_TAF_HdrCsqQrySettingCnf *hdrCsqQueryCnf = (MMA_TAF_HdrCsqQrySettingCnf *)msg;

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d,%d,%d,%d",
        g_parseContext[indexNum].cmdElement->cmdName, hdrCsqQueryCnf->hdrCsq.mode, hdrCsqQueryCnf->hdrCsq.timeInterval,
        hdrCsqQueryCnf->hdrCsq.rssiThreshold, hdrCsqQueryCnf->hdrCsq.snrThreshold,
        hdrCsqQueryCnf->hdrCsq.ecioThreshold, hdrCsqQueryCnf->hdrRssi, hdrCsqQueryCnf->hdrSnr,
        hdrCsqQueryCnf->hdrEcio);

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaCLocInfoQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CdmaLocinfoQryCnf *cLocInfoQueryCnf = (TAF_MMA_CdmaLocinfoQryCnf *)msg;
    VOS_UINT16                 length = 0;

    if (cLocInfoQueryCnf->rslt == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName,  cLocInfoQueryCnf->clocinfoPara.ratMode,
            cLocInfoQueryCnf->clocinfoPara.prevInUse, cLocInfoQueryCnf->clocinfoPara.mcc,
            cLocInfoQueryCnf->clocinfoPara.mnc, cLocInfoQueryCnf->clocinfoPara.sid,
            cLocInfoQueryCnf->clocinfoPara.nid, cLocInfoQueryCnf->clocinfoPara.baseId,
            cLocInfoQueryCnf->clocinfoPara.baseLatitude, cLocInfoQueryCnf->clocinfoPara.baseLongitude);

        g_atSendDataBuff.bufLen = length;
    } else {
        /* 查询失败，上报^CLOCINFO: 0 */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            VOS_FALSE);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmEncryptCallCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EncryptVoiceCnf   *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32                 result;
    AT_EncryptVoiceErrorUint32 ecnErr;
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
    TAF_CHAR  acNewDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN];
    VOS_INT32 length;

    (VOS_VOID)memset_s(acNewDocName, (VOS_SIZE_T)sizeof(acNewDocName), 0x00, (VOS_SIZE_T)sizeof(acNewDocName));
    length = 0;
#endif

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_EncryptVoiceCnf *)msg;

    g_atSendDataBuff.bufLen = 0;
    /* 格式化AT^ECCALL  SET命令返回 */
    if (rcvMsg->eccVoiceStatus == TAF_CALL_ENCRYPT_VOICE_SUCC) {
        result = AT_OK;
    } else {
        result = AT_ERROR;
        ecnErr = AT_MapEncVoiceErr(rcvMsg->eccVoiceStatus);

        /* 密话主叫结果其他原因值上报 */
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCALL: %d%s", g_atCrLf, ecnErr, g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
        length = snprintf_s(acNewDocName, (VOS_SIZE_T)sizeof(acNewDocName), (VOS_SIZE_T)(sizeof(acNewDocName) - 1),
                            MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_data%d", g_atCurrEncVoiceTestFileNum);
        if (length <= 0) {
            AT_WARN_LOG("AT_RcvTafCcmEccCapQryCnf(): snprintf_s len <= 0");

            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

        length = snprintf_s((VOS_CHAR *)(acNewDocName + length), ((VOS_SIZE_T)sizeof(acNewDocName) - length),
                            ((VOS_SIZE_T)sizeof(acNewDocName) - length - 1), "_[%d]records.txt",
                            g_atCurrEncVoiceDataCount);

        if (length <= 0) {
            AT_WARN_LOG("AT_RcvTafCcmEccCapQryCnf(): snprintf_s len <= 0 (2)");

            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

        (VOS_VOID)at_file_rename(g_currEncVoiceDataWriteFilePath[g_currEncVoiceDataWriteFileNum], acNewDocName);
#endif
    }

    return result;
}

VOS_UINT32 AT_RcvTafCcmRemoteCtrlAnsCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_RemoteCtrlAnswerCnf *rcvMsg = (TAF_CCM_RemoteCtrlAnswerCnf *)msg;

    g_atSendDataBuff.bufLen = 0;
    /* 格式化AT^ECCTRL  SET命令返回 */
    if (rcvMsg->result == TAF_CALL_SEND_RESULT_SUCC) {
        return AT_OK;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_RcvTafCcmEccCapSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EccSrvCapCfgCnf *rcvMsg = (TAF_CCM_EccSrvCapCfgCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^ECCAP  SET命令返回 */
    if (rcvMsg->result != TAF_CALL_ECC_SRV_CAP_CFG_RESULT_SUCC) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmEccCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_EccSrvCapQryCnf *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32               result;
    rcvMsg   = (TAF_CCM_EccSrvCapQryCnf *)msg;

    g_atSendDataBuff.bufLen = 0;
    if (rcvMsg->eccSrvCapQryCnfPara.eccSrvCap == TAF_CALL_ECC_SRV_CAP_DISABLE) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCAP: %d,%d%s", g_atCrLf, rcvMsg->eccSrvCapQryCnfPara.eccSrvCap,
            TAF_CALL_ECC_SRV_STATUS_CLOSE, g_atCrLf);
    } else if (rcvMsg->eccSrvCapQryCnfPara.eccSrvCap == TAF_CALL_ECC_SRV_CAP_ENABLE) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCAP: %d,%d%s", g_atCrLf, rcvMsg->eccSrvCapQryCnfPara.eccSrvCap,
            rcvMsg->eccSrvCapQryCnfPara.eccSrvStatus, g_atCrLf);
    } else {
        result = AT_ERROR;
    }

    return result;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_RcvTafCcmSetEccKmcCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SetEcKmcCnf *rcvMsg = (TAF_CCM_SetEcKmcCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^ECKMC  SET命令返回 */
    if (rcvMsg->result != VOS_TRUE) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmQryEccKmcCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_GetEcKmcCnf *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32           result;
    VOS_UINT8            kmcAsciiData[TAF_CALL_ECC_KMC_PUBLIC_KEY_MAX_ASCII_LENGTH + 1];

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_GetEcKmcCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^ECKMC  QRY命令返回 */
    if (rcvMsg->kmcCnfPara.result == VOS_TRUE) {
        result = AT_OK;

        /* 把kmc data转换为ascii码 */
        if (AT_HexToAsciiString(kmcAsciiData, sizeof(kmcAsciiData), rcvMsg->kmcCnfPara.kmcData.eccKmcData,
                                rcvMsg->kmcCnfPara.kmcData.eccKmcLength) != AT_OK) {
            AT_WARN_LOG("AT_RcvTafCcmQryEccKmcCnf(): AT_HexToAsciiString err");
            return AT_CMD_NO_NEED_FORMAT_RSLT;
        }

        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECKMC: %d,\"%s\"%s", g_atCrLf, rcvMsg->kmcCnfPara.kmcData.eccKmcVer,
            kmcAsciiData, g_atCrLf);
        At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    } else {
        result = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvTafCcmQryEccRandomCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_GetEcRandomCnf *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32              result;
    VOS_UINT32              loop;
    VOS_UINT16              length;
    VOS_UINT8               randomIndex;

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_GetEcRandomCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^ECRANDOM  QRY命令返回 */
    if (rcvMsg->ecRandomData.result == VOS_TRUE) {
        result      = AT_OK;
        randomIndex = 0;

        for (loop = 1; loop < TAF_CALL_APP_EC_RANDOM_MAX_GROUP + 1; loop++) {
            length = 0;
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s^ECRANDOM: %d,%s", g_atCrLf, loop,
                rcvMsg->ecRandomData.eccRandom[randomIndex].eccData);
            randomIndex++;

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", rcvMsg->ecRandomData.eccRandom[randomIndex].eccData);
            randomIndex++;

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", rcvMsg->ecRandomData.eccRandom[randomIndex].eccData);
            randomIndex++;

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s%s", rcvMsg->ecRandomData.eccRandom[randomIndex].eccData,
                g_atCrLf);
            randomIndex++;
            g_atSendDataBuff.bufLen = length;
            At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
        }

    } else {
        result = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvTafCcmSetEccTestModeCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_SetEcTestModeCnf *rcvMsg = VOS_NULL_PTR;

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_SetEcTestModeCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^ECCTEST  SET命令返回 */
    if (rcvMsg->result == VOS_TRUE) {
        return AT_OK;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_RcvTafCcmQryEccTestModeCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_GetEcTestModeCnf *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32                result;

    /* 初始化消息变量 */
    rcvMsg   = (TAF_CCM_GetEcTestModeCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^ECCTEST  SET命令返回 */
    if (rcvMsg->testModeCnfPara.result == VOS_TRUE) {
        result                  = AT_OK;
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^ECCTEST: %d%s", g_atCrLf, rcvMsg->testModeCnfPara.eccTestModeStatus,
            g_atCrLf);
    } else {
        result = AT_ERROR;
    }

    return result;
}
#endif
#endif

VOS_UINT32 AT_RcvMmaCtRoamInfoCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CtccRoamingNwInfoQryCnf *qryMultiModeSidMcc = (TAF_MMA_CtccRoamingNwInfoQryCnf *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%x%x%x,%x%x%x,%d", g_parseContext[indexNum].cmdElement->cmdName,
        qryMultiModeSidMcc->sid, (qryMultiModeSidMcc->ul3Gpp2Mcc & 0x0f00) >> 8,
        (qryMultiModeSidMcc->ul3Gpp2Mcc & 0xf0) >> 4, (qryMultiModeSidMcc->ul3Gpp2Mcc & 0x0f),
        (qryMultiModeSidMcc->ul3GppMcc & 0x0f00) >> 8, (qryMultiModeSidMcc->ul3GppMcc & 0xf0) >> 4,
        (qryMultiModeSidMcc->ul3GppMcc & 0x0f), qryMultiModeSidMcc->modeType);

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmPrivacyModeSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_PrivacyModeSetCnf *rcvMsg = (TAF_CCM_PrivacyModeSetCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* 格式化AT^CPMP  SET命令返回 */
    if (rcvMsg->result != TAF_CALL_RESULT_TYPE_SUCCESS) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmPrivacyModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_PrivacyModeQryCnf *rcvMsg = (TAF_CCM_PrivacyModeQryCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    /* X模下，只有一路激活的呼叫 */
    if (rcvMsg->privacyModeQryCnfPara.callNums != 0) {
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s^CPMP: %d,%d,%d%s", g_atCrLf,
                rcvMsg->privacyModeQryCnfPara.privacyMode,
                rcvMsg->privacyModeQryCnfPara.callVoicePrivacyInfo[0].callId,
                rcvMsg->privacyModeQryCnfPara.callVoicePrivacyInfo[0].privacyMode, g_atCrLf);
    } else {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s^CPMP: %d%s", g_atCrLf, rcvMsg->privacyModeQryCnfPara.privacyMode,
            g_atCrLf);
    }

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaPrlIdQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PrlidQryCnf *rcvMsg = (TAF_MMA_PrlidQryCnf *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^PRLID: %d,%d,%d,%d,%d%s", g_atCrLf, rcvMsg->prlSrcType, rcvMsg->prlId,
        rcvMsg->mlplMsplSrcType, rcvMsg->mlplId, rcvMsg->msplId, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaNoCardModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg              *rcvMsg = VOS_NULL_PTR;
    MTA_AT_NoCardModeQryCnf *mtaCnf = VOS_NULL_PTR;
    VOS_UINT32               result;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    mtaCnf   = (MTA_AT_NoCardModeQryCnf *)rcvMsg->content;
    result   = AT_OK;

    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                mtaCnf->enableFlag);
    }

    return result;
}

VOS_UINT32 AT_RcvMmaRatCombinedModeQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RatCombinedModeQryCnf *rcvMsg = (TAF_MMA_RatCombinedModeQryCnf *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^RATCOMBINEDMODE: %d%s", g_atCrLf, rcvMsg->combinedMode, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

    return AT_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSetPktCdataInactivityTimeLenCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_SetCtaInfoCnf *setPktCdataInactivityTimeLenCnf = VOS_NULL_PTR;

    setPktCdataInactivityTimeLenCnf = (TAF_PS_SetCtaInfoCnf *)evtInfo;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CTA_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, setPktCdataInactivityTimeLenCnf->rslt);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGetPktCdataInactivityTimeLenCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    VOS_UINT16            length;
    TAF_PS_GetCtaInfoCnf *getPktCdataInactivityTimeLenCnf = VOS_NULL_PTR;

    getPktCdataInactivityTimeLenCnf = (TAF_PS_GetCtaInfoCnf *)evtInfo;
    length                          = 0;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CTA_QRY) {
        return VOS_ERR;
    }

    /* 检查错误码 */
    if (getPktCdataInactivityTimeLenCnf->rslt != TAF_PS_CAUSE_SUCCESS) {
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_ERR;
    }

    /* 上报查询结果 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        getPktCdataInactivityTimeLenCnf->pktCdataInactivityTmrLen);

    g_atSendDataBuff.bufLen = length;
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtSet1xDormTimerCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_Set1xDormTimerCnf *set1xDormTimerCnf = VOS_NULL_PTR;

    set1xDormTimerCnf = (TAF_PS_Set1xDormTimerCnf *)evtInfo;

    /* 检查当前命令的操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DORMTIMER_SET) {
        return VOS_ERR;
    }

    /* 处理错误码 */
    AT_PrcoPsEvtErrCode(indexNum, set1xDormTimerCnf->cause);

    return VOS_OK;
}

VOS_UINT32 AT_RcvTafPsEvtGet1xDormTimerCnf(VOS_UINT8 indexNum, TAF_Ctrl *evtInfo)
{
    TAF_PS_Get1XDormTimerCnf *get1xDormTiCnf = VOS_NULL_PTR;
    VOS_UINT16                length;

    get1xDormTiCnf = (TAF_PS_Get1XDormTimerCnf *)evtInfo;
    length         = 0;

    /* 检查当前AT操作类型 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DORMTIMER_QRY) {
        return VOS_ERR;
    }

    /* 上报查询结果 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        get1xDormTiCnf->socmDormTiVal, get1xDormTiCnf->userCfgDormTival);

    g_atSendDataBuff.bufLen = length;

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvMmaCVerQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CverQueryCnf  *cVerQueryCnf = (TAF_MMA_CverQueryCnf *)msg;
    VOS_UINT16             length = 0;
    TAF_MMA_1XCasPRevUint8 invalidVersion = TAF_MMA_1X_CAS_P_REV_ENUM_BUTT;

    if (cVerQueryCnf->rslt == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            cVerQueryCnf->prevInUse);

        g_atSendDataBuff.bufLen = length;
    } else {
        /* 查询失败，上报^CVER: BUTT */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            invalidVersion);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaStateQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_StateQueryCnf *stQryCnf = (TAF_MMA_StateQueryCnf *)msg;
    VOS_UINT16             length = 0;
    VOS_INT8               invalidSta = -1;

    if (stQryCnf->rslt == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            stQryCnf->handsetSta.casState, stQryCnf->handsetSta.casSubSta);

        g_atSendDataBuff.bufLen = length;
    } else {
        /* 查询失败，上报^GETSTA: BUTT */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            invalidSta, invalidSta);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaCHverQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ChighverQueryCnf *staQryCnf = (TAF_MMA_ChighverQueryCnf *)msg;
    VOS_UINT16                length = 0;
    TAF_MMA_1XCasPRevUint8    invalidVer = TAF_MMA_1X_CAS_P_REV_ENUM_BUTT;

    if (staQryCnf->rslt == VOS_OK) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            staQryCnf->highRev);

        g_atSendDataBuff.bufLen = length;
    } else {
        /* 查询失败，上报^CHIGHVER: BUTT */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            invalidVer);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaMeidSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg       *rcvMsg = VOS_NULL_PTR;
    MTA_AT_ResultCnf *setCnf = VOS_NULL_PTR;
    VOS_UINT32        rslt;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    setCnf   = (MTA_AT_ResultCnf *)rcvMsg->content;

    /* 格式化AT^MEID命令返回 */
    g_atSendDataBuff.bufLen = 0;

    switch (setCnf->result) {
        case MTA_AT_RESULT_INCORRECT_PARAMETERS:
            rslt = AT_DEVICE_INVALID_PARAMETERS;
            break;

        case MTA_AT_RESULT_DEVICE_SEC_NV_ERROR:
            rslt = AT_DEVICE_NV_WRITE_FAIL_UNKNOWN;
            break;

        case MTA_AT_RESULT_NO_ERROR:
            rslt = AT_OK;
            break;

        default:
            rslt = AT_ERROR;
            break;
    }

    return rslt;
}

VOS_UINT32 AT_RcvMtaMeidQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg        *rcvMsg = VOS_NULL_PTR;
    MTA_AT_MeidQryCnf *qryCnf = VOS_NULL_PTR;
    VOS_UINT8          meId[2 * MTA_AT_MEID_NV_DATA_LEN_NEW + 1];
    VOS_UINT8          aucpEsn[2 * MTA_AT_PESN_NV_DATA_LEN + 1];
    VOS_UINT8          aucpUimID[2 * UIMID_DATA_LEN + 1];

    memset_s(meId, sizeof(meId), 0x00, sizeof(meId));

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    qryCnf   = (MTA_AT_MeidQryCnf *)rcvMsg->content;

    /* 格式化AT^MEID命令返回 */
    g_atSendDataBuff.bufLen = 0;

    /* 读取en_NV_Item_MEID */
    if (qryCnf->meIdReadRst != NV_OK) {
        AT_WARN_LOG("AT_RcvMtaMeidQryCnf:Read NV_ITEM_MEID Nvim Failed");
        return AT_DEVICE_NV_READ_FAILURE;
    }

    /* 读取en_NV_Item_PESN */
    if (qryCnf->pEsnReadRst != NV_OK) {
        AT_WARN_LOG("AT_RcvMtaMeidQryCnf:Read NV_ITEM_PESN Nvim Failed");
        return AT_DEVICE_NV_READ_FAILURE;
    }

    /* 读取UIMID失败 */
    if (qryCnf->result != MTA_AT_RESULT_NO_ERROR) {
        AT_WARN_LOG("AT_RcvMtaMeidQryCnf:Read UIMID Failed");
    }

    AT_Hex2Ascii_Revers(&(qryCnf->efruimid[1]), UIMID_DATA_LEN, aucpUimID);
    aucpUimID[2 * UIMID_DATA_LEN] = '\0'; /* 数组最后一位为'\0' */

    AT_Hex2Ascii_Revers(&(qryCnf->pEsn[0]), MTA_AT_PESN_NV_DATA_LEN, aucpEsn);
    aucpEsn[2 * MTA_AT_PESN_NV_DATA_LEN] = '\0'; /* 数组最后一位为'\0' */

    AT_Hex2Ascii_Revers(&(qryCnf->meId[0]), MTA_AT_MEID_NV_DATA_LEN_NEW, meId);
    meId[2 * MTA_AT_MEID_NV_DATA_LEN_NEW] = '\0'; /* 数组最后一位为'\0' */

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s,%s,%s", g_parseContext[indexNum].cmdElement->cmdName, meId,
            aucpEsn, aucpUimID);

    return AT_OK;
}

VOS_UINT32 AT_RcvCdmaModemSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                *rcvMsg = VOS_NULL_PTR;
    MTA_AT_CdmaModemCapSetCnf *setCnf = VOS_NULL_PTR;
    AT_MTA_ModemCapUpdateReq   atMtaModemCapUpdate;
    VOS_UINT32                 result;
    VOS_UINT8                  isCdmaModemSwitchNotResetFlg;

    /* 初始化 */
    rcvMsg = (AT_MTA_Msg *)msg;
    setCnf = (MTA_AT_CdmaModemCapSetCnf *)rcvMsg->content;
    result = AT_ERROR;
    g_atSendDataBuff.bufLen = 0;

    AT_NORM_LOG1("AT_RcvCdmaModemSetCnf : return Result ", setCnf->result);

    if (setCnf->result == MTA_AT_RESULT_ERROR) {
        AT_WARN_LOG1("AT_RcvCdmaModemSetCnf : MTA Return Error. ucResult", setCnf->result);

        return AT_CME_WRITE_NV_TimeOut;
    } else if (setCnf->result != MTA_AT_RESULT_NO_ERROR) {
        AT_WARN_LOG1("AT_RcvCdmaModemSetCnf : MTA Return Error. ucResult", setCnf->result);

        return AT_ERROR;
    } else {
        result = AT_OK;
    }
    AT_ReadPlatformNV();

    /*  at 命令入口处已经检查，到了此处，必定时modem0 */
    isCdmaModemSwitchNotResetFlg = AT_GetModemCdmaModemSwitchCtxAddrFromModemId(MODEM_ID_0)->enableFlg;

    if (isCdmaModemSwitchNotResetFlg == VOS_FALSE) {
        return result;
    }

    (VOS_VOID)memset_s(&atMtaModemCapUpdate, (VOS_UINT32)sizeof(AT_MTA_ModemCapUpdateReq), 0x00,
                       (VOS_UINT32)sizeof(AT_MTA_ModemCapUpdateReq));

    /* 更新了平台能力，通知AT->MTA->RRM，进行底层平台能力更新 */
    /* AT发送至MTA的消息结构赋值 */
    atMtaModemCapUpdate.modemCapUpdateType = AT_MTA_MODEM_CAP_UPDATE_TYPE_CDMA_MODEM_SWITCH;

    /* 发送消息给C核处理 */
    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_MODEM_CAP_UPDATE_REQ,
                               (VOS_UINT8 *)&atMtaModemCapUpdate, (VOS_UINT32)sizeof(AT_MTA_ModemCapUpdateReq),
                               I0_UEPS_PID_MTA) != AT_SUCCESS) {
        AT_WARN_LOG("AT_RcvCdmaModemSetCnf: AT_FillAndSndAppReqMsg Fail!");

        return result;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MODEM_CAP_UPDATE_SET;

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
AT_CmdCurOptUint32 AT_SearchCagpsATCmdOpt(AT_XPDS_MsgTypeUint32 msgType)
{
    VOS_UINT32 i;

    for (i = 0; i < sizeof(g_atCagpsCmdOptTbl) / sizeof(AT_CagpsCmdOptTlb); i++) {
        if (msgType == g_atCagpsCmdOptTbl[i].msgType) {
            return g_atCagpsCmdOptTbl[i].cmdOpt;
        }
    }

    return AT_CMD_CURRENT_OPT_BUTT;
}

VOS_UINT32 AT_RcvXpdsCagpsRlstCnf(struct MsgCB *msg)
{
    XPDS_AT_ResultCnf *msgRsltCnf = VOS_NULL_PTR;
    VOS_UINT32         result;
    VOS_UINT8          indexNum;
    AT_CmdCurOptUint32 cmdOpt;

    /* 初始化 */
    msgRsltCnf = (XPDS_AT_ResultCnf *)msg;
    result     = AT_OK;
    indexNum   = 0;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(msgRsltCnf->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsCagpsRlstCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    cmdOpt = AT_SearchCagpsATCmdOpt(msgRsltCnf->msgId);

    /* 当前AT是否在等待该命令返回 */
    if (cmdOpt != g_atClientTab[indexNum].cmdCurrentOpt) {
        AT_WARN_LOG("AT_RcvXpdsCagpsRlstCnf : Current Option is not correct.");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = 0;

    if (msgRsltCnf->result != VOS_OK) {
        result = AT_ERROR;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_CHAR* AT_SearchCagpsATCmd(AT_XPDS_MsgTypeUint32 msgType)
{
    VOS_UINT32 i;

    for (i = 0; i < sizeof(g_atCagpsCmdNameTlb) / sizeof(AT_CagpsCmdNameTlb); i++) {
        if (msgType == g_atCagpsCmdNameTlb[i].msgType) {
            return g_atCagpsCmdNameTlb[i].pcATCmd;
        }
    }

    return "UNKOWN-MSG";
}

VOS_UINT32 AT_RcvXpdsCagpsCnf(struct MsgCB *msg)
{
    AT_XPDS_Msg *rcvMsg = VOS_NULL_PTR;
    VOS_UINT8    indexNum;
    VOS_UINT32   contentLen;
    VOS_UINT16   length = 0;

    /* 初始化 */
    rcvMsg   = (AT_XPDS_Msg *)msg;
    indexNum = 0;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(rcvMsg->appCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvXpdsCagpsCnf : WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s%s", g_atCrLf, AT_SearchCagpsATCmd(rcvMsg->msgId));

    /* 获取消息内容长度 */
    contentLen = VOS_GET_MSG_LEN(rcvMsg) - (sizeof(rcvMsg->msgId) + sizeof(AT_APPCTRL));

    if (contentLen != 0) {
        /* <length>, */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ": %d,\"", contentLen * AT_ASCII_AND_HEX_CONVERSION_FACTOR);

        /* <command>, */
        length += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                      (TAF_UINT8 *)g_atSndCodeAddress + length, rcvMsg->content,
                                                      (TAF_UINT16)contentLen);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "\"");
    }

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);

    return VOS_OK;
}
#endif






#endif


