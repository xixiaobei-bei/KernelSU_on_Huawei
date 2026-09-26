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
#include "at_sms_comm.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SMS_COMM_C
MN_MSG_SendAckParm g_atMsgAckParm;
MN_MSG_TsDataInfo  g_atMsgTsDataInfo;

static const AT_CMS_SmsErrCodeMap g_atCmsSmsErrCodeMapTbl[] = {
    { AT_CMS_U_SIM_BUSY, MN_ERR_CLASS_SMS_UPDATE_USIM },
    { AT_CMS_U_SIM_NOT_INSERTED, MN_ERR_CLASS_SMS_NOUSIM },
    { AT_CMS_INVALID_MEMORY_INDEX, MN_ERR_CLASS_SMS_EMPTY_REC },
    { AT_CMS_MEMORY_FULL, MN_ERR_CLASS_SMS_STORAGE_FULL },
    { AT_CMS_U_SIM_PIN_REQUIRED, MN_ERR_CLASS_SMS_NEED_PIN1 },
    { AT_CMS_U_SIM_PUK_REQUIRED, MN_ERR_CLASS_SMS_NEED_PUK1 },
    { AT_CMS_U_SIM_FAILURE, MN_ERR_CLASS_SMS_UNAVAILABLE },
    { AT_CMS_OPERATION_NOT_ALLOWED, MN_ERR_CLASS_SMS_FEATURE_INAVAILABLE },
    { AT_CMS_SMSC_ADDRESS_UNKNOWN, MN_ERR_CLASS_SMS_INVALID_SCADDR },
    { AT_CMS_INVALID_PDU_MODE_PARAMETER, MN_ERR_CLASS_SMS_MSGLEN_OVERFLOW },
    { AT_CMS_FDN_DEST_ADDR_FAILED, MN_ERR_CLASS_FDN_CHECK_DN_FAILURE },
    { AT_CMS_FDN_SERVICE_CENTER_ADDR_FAILED, MN_ERR_CLASS_FDN_CHECK_SC_FAILURE },
    { AT_CMS_MO_SMS_CONTROL_FAILED, MN_ERR_CLASS_SMS_MO_CTRL_ACTION_NOT_ALLOWED },
    { AT_CMS_MO_SMS_CONTROL_FAILED, MN_ERR_CLASS_SMS_MO_CTRL_USIM_PARA_ERROR },
    { AT_CMS_MEMORY_FAILURE, MN_ERR_NOMEM }
};

MN_MSG_SendAckParm* At_GetAckMsgMem(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_atMsgAckParm, sizeof(g_atMsgAckParm), 0x00, sizeof(g_atMsgAckParm));
    return &g_atMsgAckParm;
}

MN_MSG_TsDataInfo* At_GetMsgMem(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_atMsgTsDataInfo, sizeof(g_atMsgTsDataInfo), 0x00, sizeof(g_atMsgTsDataInfo));
    return &g_atMsgTsDataInfo;
}

VOS_UINT32 At_GetSmsStorage(VOS_UINT8 indexNum, MN_MSG_MemStoreUint8 memReadorDelete,
                            MN_MSG_MemStoreUint8 memSendorWrite, MN_MSG_MemStoreUint8 memRcv)
{
    MN_MSG_GetStorageStatusParm memParm;

    (VOS_VOID)memset_s(&memParm, sizeof(MN_MSG_GetStorageStatusParm), 0x00, sizeof(MN_MSG_GetStorageStatusParm));

    if ((memReadorDelete == MN_MSG_MEM_STORE_SIM) || (memSendorWrite == MN_MSG_MEM_STORE_SIM) ||
        (memRcv == MN_MSG_MEM_STORE_SIM)) {
        memParm.mem1Store                                          = MN_MSG_MEM_STORE_SIM;
        g_atClientTab[indexNum].atSmsData.waitForUsimStorageStatus = VOS_TRUE;
    } else {
        memParm.mem1Store                                          = MN_MSG_MEM_STORE_NONE;
        g_atClientTab[indexNum].atSmsData.waitForUsimStorageStatus = VOS_FALSE;
    }
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST == FEATURE_ON)
    if ((memReadorDelete == MN_MSG_MEM_STORE_ME) || (memSendorWrite == MN_MSG_MEM_STORE_ME) ||
        (memRcv == MN_MSG_MEM_STORE_ME)) {
        memParm.mem2Store                                        = MN_MSG_MEM_STORE_ME;
        g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus = VOS_TRUE;
    } else {
        memParm.mem2Store                                        = MN_MSG_MEM_STORE_NONE;
        g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus = VOS_FALSE;
    }
#else
    memParm.mem2Store                                        = MN_MSG_MEM_STORE_NONE;
    g_atClientTab[indexNum].atSmsData.waitForNvStorageStatus = VOS_FALSE;
#endif

    if (MN_MSG_GetStorageStatus(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &memParm) !=
        MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    return AT_OK;
}

TAF_VOID AT_StubClearSpecificAutoRelyMsg(VOS_UINT8 clientIndex, TAF_UINT32 bufferIndex)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(clientIndex);

    if (smsCtx->smsMtBuffer[bufferIndex].event != VOS_NULL_PTR) {
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, smsCtx->smsMtBuffer[bufferIndex].event);
        smsCtx->smsMtBuffer[bufferIndex].event = VOS_NULL_PTR;
        /*lint -restore */
    }

    if (smsCtx->smsMtBuffer[bufferIndex].tsDataInfo != VOS_NULL_PTR) {
        PS_MEM_FREE(WUEPS_PID_AT, smsCtx->smsMtBuffer[bufferIndex].tsDataInfo);
        smsCtx->smsMtBuffer[bufferIndex].tsDataInfo = VOS_NULL_PTR;
    }
}

VOS_UINT32 At_ChgMnErrCodeToAt(VOS_UINT8 indexNum, VOS_UINT32 mnErrorCode)
{
    VOS_UINT32            rtn;
    const AT_CMS_SmsErrCodeMap *smsErrMapTblPtr = VOS_NULL_PTR;
    VOS_UINT32            smsErrMapTblSize;
    VOS_UINT32            cnt;
    AT_ModemSmsCtx       *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smsErrMapTblPtr  = AT_GET_CMS_SMS_ERR_CODE_MAP_TBL_PTR();
    smsErrMapTblSize = AT_GET_CMS_SMS_ERR_CODE_MAP_TBL_SIZE();

    rtn = AT_CMS_UNKNOWN_ERROR;

    for (cnt = 0; cnt < smsErrMapTblSize; cnt++) {
        if (smsErrMapTblPtr[cnt].smsCause == mnErrorCode) {
            rtn = smsErrMapTblPtr[cnt].cmsCode;

            if ((smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) && (rtn == AT_CMS_INVALID_PDU_MODE_PARAMETER)) {
                rtn = AT_CMS_INVALID_TEXT_MODE_PARAMETER;
            }

            break;
        }
    }

    return rtn;
}

TAF_VOID At_PrintCsmsInfo(TAF_UINT8 indexNum)
{
    AT_MSG_Serv msgServInfo = { AT_MSG_SERV_STATE_SUPPORT, AT_MSG_SERV_STATE_SUPPORT, AT_MSG_SERV_STATE_SUPPORT };

    g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + g_atSendDataBuff.bufLen), "%d,%d,%d", msgServInfo.smsMT, msgServInfo.smsMO,
        msgServInfo.smsBM);
}

TAF_UINT32 At_MsgDeleteCmdProc(TAF_UINT8 indexNum, MN_OPERATION_ID_T opId, MN_MSG_DeleteParam deleteInfo,
                               TAF_UINT32 deleteTypes)
{
    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes & AT_MSG_DELETE_SINGLE) {
        deleteInfo.deleteType = MN_MSG_DELETE_SINGLE;
        if (MN_MSG_Delete(g_atClientTab[indexNum].clientId, opId, &deleteInfo) != MN_ERR_NO_ERROR) {
            return AT_ERROR;
        }
        return AT_OK;
    }

    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes & AT_MSG_DELETE_READ) {
        deleteInfo.deleteType = MN_MSG_DELETE_READ;
        if (MN_MSG_Delete(g_atClientTab[indexNum].clientId, opId, &deleteInfo) != MN_ERR_NO_ERROR) {
            return AT_ERROR;
        }
        return AT_OK;
    }

    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes & AT_MSG_DELETE_SENT) {
        deleteInfo.deleteType = MN_MSG_DELETE_SENT;
        if (MN_MSG_Delete(g_atClientTab[indexNum].clientId, opId, &deleteInfo) != MN_ERR_NO_ERROR) {
            return AT_ERROR;
        }
        return AT_OK;
    }

    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes & AT_MSG_DELETE_UNSENT) {
        deleteInfo.deleteType = MN_MSG_DELETE_NOT_SENT;
        if (MN_MSG_Delete(g_atClientTab[indexNum].clientId, opId, &deleteInfo) != MN_ERR_NO_ERROR) {
            return AT_ERROR;
        }
        return AT_OK;
    }

    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes & AT_MSG_DELETE_ALL) {
        deleteInfo.deleteType = MN_MSG_DELETE_ALL;
        if (MN_MSG_Delete(g_atClientTab[indexNum].clientId, opId, &deleteInfo) != MN_ERR_NO_ERROR) {
            return AT_ERROR;
        }
        return AT_OK;
    }

    return AT_ERROR;
}

TAF_VOID AT_PrintTimeZone(TAF_INT8 timezone, TAF_UINT8 *dst, TAF_UINT16 *length)
{
    TAF_UINT8  timeZone;
    TAF_UINT16 lengthTemp;

    if (timezone < 0) {
        lengthTemp = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)dst,
            "-");
        timeZone   = (TAF_UINT8)(timezone * (-1));
    } else {
        lengthTemp = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)dst,
            "+");
        timeZone   = (TAF_UINT8)timezone;
    }

    if (timeZone > MN_MSG_MAX_TIMEZONE_VALUE) {
        AT_WARN_LOG("AT_PrintTimeZone: Time zone is invalid.");
        timeZone = 0;
    }

    /* 除10是为了取高位数，余10是为了取低位 */
    lengthTemp += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(dst + lengthTemp), "%d%d\"", (0x0f & (timeZone / 10)), (timeZone % 10));

    *length = lengthTemp;
}

TAF_UINT32 At_SmsPrintScts(const MN_MSG_Timestamp *timeStamp, TAF_UINT8 *dst)
{
    TAF_UINT16                  length;
    TAF_UINT16                  timeZoneLength;
    MN_MSG_DateInvalidTypeUint8 dateInvalidType;
    TAF_UINT32                  ret;

    ret = MN_MSG_ChkDate(timeStamp, &dateInvalidType);
    if (ret != MN_ERR_NO_ERROR) {
        AT_WARN_LOG("At_SmsPrintScts: Date is invalid.");
    }

    /* "yy/MM/dd,hh:mm:ss±zz" */
    if ((MN_MSG_DATE_INVALID_YEAR & dateInvalidType) == 0) {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)dst,
            "\"%d%d/", ((timeStamp->year >> 4) & 0x0f), (timeStamp->year & 0x0f));
    } else {
        length = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress, (TAF_CHAR *)dst,
            "\"00/");
    }

    /* MM */
    if ((MN_MSG_DATE_INVALID_MONTH & dateInvalidType) == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "%d%d/", ((timeStamp->month >> 4) & 0x0f), (timeStamp->month & 0x0f));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "01/");
    }

    /* dd */
    if ((MN_MSG_DATE_INVALID_DAY & dateInvalidType) == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "%d%d,", ((timeStamp->day >> 4) & 0x0f), (timeStamp->day & 0x0f));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "01,");
    }

    /* hh */
    if ((MN_MSG_DATE_INVALID_HOUR & dateInvalidType) == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "%d%d:", ((timeStamp->hour >> 4) & 0x0f), (timeStamp->hour & 0x0f));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "00:");
    }

    /* mm */
    if ((MN_MSG_DATE_INVALID_MINUTE & dateInvalidType) == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "%d%d:", ((timeStamp->minute >> 4) & 0x0f), (timeStamp->minute & 0x0f));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "00:");
    }

    /* ss */
    if ((MN_MSG_DATE_INVALID_SECOND & dateInvalidType) == 0) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "%d%d", ((timeStamp->second >> 4) & 0x0f), (timeStamp->second & 0x0f));
    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)dst + length, "00");
    }

    /* ±zz */
    AT_PrintTimeZone(timeStamp->timezone, (dst + length), &timeZoneLength);
    length += timeZoneLength;

    return length;
}

TAF_UINT16 At_MsgPrintVp(MN_MSG_ValidPeriod *validPeriod, TAF_UINT8 *dst)
{
    TAF_UINT16 length = 0;

    switch (validPeriod->validPeriod) {
        case MN_MSG_VALID_PERIOD_RELATIVE:
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)dst + length, "%d", validPeriod->u.otherTime);
            break;
        case MN_MSG_VALID_PERIOD_ABSOLUTE:
            length += (TAF_UINT16)At_SmsPrintScts(&validPeriod->u.absoluteTime, dst);
            break;
        default:
            break;
    }
    return length;
}

