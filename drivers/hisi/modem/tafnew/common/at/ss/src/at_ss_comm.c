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
#include "at_ss_comm.h"
#include "securec.h"
#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "taf_std_lib.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SS_COMM_C

/* +CLCK命令参数CLASS与Service Type Code对应扩展表 */
static const AT_ClckClassServiceTbl g_clckClassServiceExtTbl[] = {
    { AT_CLCK_PARA_CLASS_VOICE, TAF_SS_TELE_SERVICE, TAF_ALL_SPEECH_TRANSMISSION_SERVICES_TSCODE },
    { AT_CLCK_PARA_CLASS_VOICE, TAF_SS_TELE_SERVICE, TAF_TELEPHONY_TSCODE },
    { AT_CLCK_PARA_CLASS_VOICE, TAF_SS_TELE_SERVICE, TAF_EMERGENCY_CALLS_TSCODE },
    { AT_CLCK_PARA_CLASS_DATA, TAF_SS_BEARER_SERVICE, TAF_ALL_BEARERSERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA, TAF_SS_TELE_SERVICE, TAF_ALL_DATA_TELESERVICES_TSCODE },
    { AT_CLCK_PARA_CLASS_FAX, TAF_SS_TELE_SERVICE, TAF_ALL_FACSIMILE_TRANSMISSION_SERVICES_TSCODE },
    { AT_CLCK_PARA_CLASS_FAX, TAF_SS_TELE_SERVICE, TAF_FACSIMILE_GROUP3_AND_ALTER_SPEECH_TSCODE },
    { AT_CLCK_PARA_CLASS_FAX, TAF_SS_TELE_SERVICE, TAF_AUTOMATIC_FACSIMILE_GROUP3_TSCODE },
    { AT_CLCK_PARA_CLASS_FAX, TAF_SS_TELE_SERVICE, TAF_FACSIMILE_GROUP4_TSCODE },
    { AT_CLCK_PARA_CLASS_VOICE_DATA_FAX, TAF_SS_TELE_SERVICE, TAF_ALL_TELESERVICES_EXEPTSMS_TSCODE },
    { AT_CLCK_PARA_CLASS_SMS, TAF_SS_TELE_SERVICE, TAF_ALL_SMS_SERVICES_TSCODE },
    { AT_CLCK_PARA_CLASS_SMS, TAF_SS_TELE_SERVICE, TAF_SMS_MT_PP_TSCODE },
    { AT_CLCK_PARA_CLASS_SMS, TAF_SS_TELE_SERVICE, TAF_SMS_MO_PP_TSCODE },
    { AT_CLCK_PARA_CLASS_VOICE_DATA_FAX_SMS, TAF_SS_TELE_SERVICE, TAF_ALL_TELESERVICES_TSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_DATA_CIRCUIT_SYNCHRONOUS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_DATACDS_SERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDS_1200BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDS_2400BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDS_4800BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDS_9600BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_ALTERNATE_SPEECH_DATACDS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_SPEECH_FOLLOWED_BY_DATACDS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC, TAF_SS_BEARER_SERVICE, TAF_GENERAL_DATACDS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_DATA_CIRCUIT_ASYNCHRONOUS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_DATACDA_SERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDA_300BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDA_1200BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDA_1200_75BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDA_2400BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDA_4800BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_DATACDA_9600BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_ALTERNATE_SPEECH_DATACDA_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC, TAF_SS_BEARER_SERVICE, TAF_ALL_SPEECH_FOLLOWED_BY_DATACDA_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PKT, TAF_SS_BEARER_SERVICE, TAF_ALL_DATAPDS_SERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PKT, TAF_SS_BEARER_SERVICE, TAF_DATAPDS_2400BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PKT, TAF_SS_BEARER_SERVICE, TAF_DATAPDS_4800BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PKT, TAF_SS_BEARER_SERVICE, TAF_DATAPDS_9600BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC_PKT, TAF_SS_BEARER_SERVICE, TAF_ALL_SYNCHRONOUS_SERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_ALL_PADACCESSCA_SERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_PADACCESSCA_300BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_PADACCESSCA_1200BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_PADACCESSCA_1200_75BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_PADACCESSCA_2400BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_PADACCESSCA_4800BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_PAD, TAF_SS_BEARER_SERVICE, TAF_PADACCESSCA_9600BPS_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_ASYNC_PAD, TAF_SS_BEARER_SERVICE, TAF_ALL_ASYNCHRONOUS_SERVICES_BSCODE },
    { AT_CLCK_PARA_CLASS_DATA_SYNC_ASYNC_PKT_PKT, TAF_SS_BEARER_SERVICE, TAF_ALL_BEARERSERVICES_BSCODE },
};

TAF_UINT8 At_GetClckClassFromBsCode(TAF_SS_BasicService *bs)
{
    VOS_UINT32 loop;
    VOS_UINT32 itemsNum;

    itemsNum = sizeof(g_clckClassServiceExtTbl) / sizeof(AT_ClckClassServiceTbl);

    /* 查表获取服务类型及服务码对应的Class */
    for (loop = 0; loop < itemsNum; loop++) {
        if ((g_clckClassServiceExtTbl[loop].serviceType == bs->bsType) &&
            (g_clckClassServiceExtTbl[loop].serviceCode == bs->bsServiceCode)) {
            return g_clckClassServiceExtTbl[loop].classType;
        }
    }

    return AT_UNKNOWN_CLCK_CLASS;
}

VOS_UINT32 AT_ConvertTafSsaErrorCode(VOS_UINT8 indexNum, TAF_ERROR_CodeUint32 errorCode)
{
    VOS_UINT32 result;

    if (errorCode == TAF_ERR_NO_ERROR) {
        return AT_OK;
    }

    result = AT_ERROR;

    if ((errorCode >= TAF_ERR_LCS_BASE) && (errorCode <= TAF_ERR_LCS_UNKNOWN_ERROR)) {
        result = errorCode - TAF_ERR_LCS_BASE + AT_CMOLRE_ERR_ENUM_BEGIN + 1;
        return result;
    }

    result = At_ChgTafErrorCode(indexNum, errorCode);

    return result;
}

VOS_VOID AT_CnapConvertNameStr(TAF_CALL_Cnap *nameIndicator, VOS_UINT8 *nameStr, VOS_UINT8 nameStrMaxLen)
{
    errno_t    memResult;
    VOS_UINT32 ascIILen;

    (VOS_VOID)memset_s(nameStr, nameStrMaxLen, 0x00, nameStrMaxLen);
    ascIILen = 0;

    /* 7bit转换为8bit */
    if (nameIndicator->dcs == TAF_CALL_DCS_7BIT) {
        /* 7bit转换为8bit */
        if (nameStrMaxLen < nameIndicator->length) {
            AT_WARN_LOG("AT_CnapConvertNameStr: NameStr Space Too Short!");
            return;
        }

        if (TAF_STD_UnPack7Bit(nameIndicator->nameStr, nameIndicator->length, 0, nameStr) != VOS_OK) {
            AT_WARN_LOG("AT_CnapConvertNameStr: TAF_STD_UnPack7Bit Err!");
            return;
        }

        /* Default Alphabet转换为ASCII码 */
        TAF_STD_ConvertDefAlphaToAscii(nameStr, nameIndicator->length, nameStr, &ascIILen);

        if (ascIILen < nameIndicator->length) {
            AT_WARN_LOG("AT_CnapConvertNameStr: TAF_STD_ConvertDefAlphaToAscii Err!");
            return;
        }
    } else {
        if (nameIndicator->length > 0) {
            memResult = memcpy_s(nameStr, nameStrMaxLen, nameIndicator->nameStr,
                                 TAF_MIN(nameIndicator->length, TAF_CALL_CNAP_NAME_STR_MAX_LENGTH));
            TAF_MEM_CHK_RTN_VAL(memResult, nameStrMaxLen,
                                TAF_MIN(nameIndicator->length, TAF_CALL_CNAP_NAME_STR_MAX_LENGTH));
        }
    }
}

VOS_VOID AT_ReportCnapInfo(VOS_UINT8 indexNum, TAF_CALL_Cnap *nameIndicator)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;
    VOS_UINT8      dstName[AT_PARA_CNAP_MAX_NAME_LEN];

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);
    (VOS_VOID)memset_s(dstName, (VOS_SIZE_T)sizeof(dstName), 0, (VOS_SIZE_T)sizeof(dstName));

    if ((ssCtx->cnapType != AT_CNAP_ENABLE_TYPE) || (nameIndicator->existFlag == VOS_FALSE)) {
        return;
    }

    /* 7bit和8bit转换后上报 */
    if ((nameIndicator->dcs == TAF_CALL_DCS_7BIT) || (nameIndicator->dcs == TAF_CALL_DCS_8BIT)) {
        AT_CnapConvertNameStr(nameIndicator, dstName, AT_PARA_CNAP_MAX_NAME_LEN);
    } else {
        AT_NORM_LOG1("AT_ReportCnapInfo: Dcs:", nameIndicator->dcs);
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s+CNAP: \"%s\",%d%s", g_atCrLf, (VOS_CHAR *)dstName,
        nameIndicator->cniValidity, g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
}

