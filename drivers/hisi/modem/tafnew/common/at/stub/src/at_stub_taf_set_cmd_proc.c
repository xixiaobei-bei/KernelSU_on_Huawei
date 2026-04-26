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

#include "at_stub_taf_set_cmd_proc.h"
#include "at_set_para_cmd.h"
#include "securec.h"
#include "at_mdrv_interface.h"
#include "taf_type_def.h"
#include "at_cmd_proc.h"
#include "at_input_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "ppp_interface.h"
#include "at_msg_print.h"
#include "si_app_stk.h"
#include "si_app_pih.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "mn_comm_api.h"
#include "at_cmd_msg_proc.h"
#include "at_rabm_interface.h"
#include "at_snd_msg.h"
#include "at_device_cmd.h"
#include "at_rnic_interface.h"
#include "at_taf_agent_interface.h"
#include "taf_agent_interface.h"
#include "taf_app_xsms_interface.h"

#include "taf_std_lib.h"
#include "mnmsgcbencdec.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "gen_msg.h"
#include "at_lte_common.h"
#endif
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

#include "nv_stru_gucnas.h"
#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_pam.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "mn_call_api.h"
#include "at_ctx.h"
#include "app_vc_api.h"
#include "at_imsa_interface.h"
#include "at_init.h"
#include "at_file_handle.h"

#include "gunas_errno.h"

#include "css_at_interface.h"
#include "taf_app_mma.h"
#include "taf_mtc_api.h"
#include "taf_ccm_api.h"

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (VOS_OS_VER != VOS_LINUX)
#include "Linuxstub.h"
#endif
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
#include "ads_dev_i.h"
#else
#include "ads_device_interface.h"
#endif
#else
#include "cds_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#include "UsimmApi.h"
#endif

#include "imm_interface.h"
#include "throt_at_interface.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_STUB_TAF_SET_CMD_PROC_C

#define AT_CREDIALSTUB_SSREDIALIMSTOCS 2
#define AT_CREDIALSTUB_CALLREDIALCSTOIMS 3
#define AT_CREDIALSTUB_SMSREDIALCSTOTMS 4
#define AT_CREDIALSTUB_SSREDIALCSTOIMS 5
#define AT_DOMAINSTUB_MMLVOICEDOMAINFLAG 2
#define AT_IMSCAPSTUB_SMS 2
#define AT_SIMWRITESTUB_MSGLENGTH 1
#define AT_SIMWRITESTUB_RECORDNUM 2
#define AT_NVSTUB_READ_OR_WRITE 0
#define AT_NVSTUB_WRITE 2
#define AT_USIMWRITESTUB_MSG 1
#define AT_USIMWRITESTUB_RECORDNUM 2
#define AT_USIMWRITESTUB_APPTYPE 3
#define AT_SIMWRITESTUB_PARA_MAX_NUM 3
#define AT_PDPISRSTUB_PARA_MAX_NUM 3
#define AT_IMSRATSTUB_PARA_MAX_NUM 2
#define AT_IMSCAPSTUB_PARA_MAX_NUM 3
#define AT_DOMAINSTUB_PARA_MAX_NUM 3
#define AT_CREDIALSTUB_PARA_MAX_NUM 7
#define MN_CALL_MAX_ASCII_NUM_LEN (MN_CALL_MAX_BCD_NUM_LEN * 2)
#define AT_USIMWRITESTUB_MIN_PARA_NUM 3
#define AT_USIMWRITESTUB_MAX_PARA_NUM 4
#define AT_NVSTUB_MIN_PARA_NUM 2
#define AT_NVSTUB_MAX_PARA_NUM 3
#define AT_NVSTUB_NVIM_ID 1
#define AT_USIM_PARA_MAX_NUM 2
#define AT_NVSTUB_WRITE_ITEM_RAT_PRIO_LIST_PARA_LEN 12
#if (VOS_WIN32 == VOS_OS_VER)
EXTERN const AT_USIMM_FileNumToId g_atUsimFileNumToIdTab[];
EXTERN const VOS_UINT32 g_atUsimFileNumToIdTabLen;
EXTERN const AT_USIMM_FileNumToId g_atSimFileNumToIdTab[];
EXTERN const VOS_UINT32 g_atSimFileNumToIdTabLen;
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
EXTERN const AT_USIMM_FileNumToId g_atCsimFileNumToIdTab[];

EXTERN VOS_UINT32 g_atCsimFileNumToIdTabLen;
#endif

#endif

#if (VOS_WIN32 == VOS_OS_VER)
VOS_UINT32 AT_ReadValueFromNvim(const NV_IdU16 nVItemType, VOS_UINT8 *data, VOS_UINT32 dataLen, ModemIdUint16 modemId)
{
    VOS_UINT32 rst;
    rst = TAF_ACORE_NV_READ(modemId, nVItemType, data, dataLen);
    switch (rst) {
        case NV_OK:
            return AT_OK;
        case NV_ID_NOT_EXIST:
        case NV_BUFFER_TOO_LONG:
            return AT_CME_INCORRECT_PARAMETERS;
        default:
            return AT_ERROR;
    }
}

VOS_UINT32 AT_SetPsBearIsrFlgStub(VOS_UINT8 indexNum)
{
    if (g_atParaIndex > AT_PDPISRSTUB_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

#if (FEATURE_LTE == FEATURE_ON)

    /* 设置usapi 对应的pdp是在ISR激活前激活的还是在ISR激活后激活的 */
    NAS_SetPsBearerIsrFlg(g_atParaList[AT_COMMAND_PARA_INDEX_0].paraValue,
                          g_atParaList[AT_COMMAND_PARA_INDEX_1].paraValue,
                          g_atParaList[AT_COMMAND_PARA_INDEX_2].paraValue);
#endif

    return AT_OK;
}

VOS_UINT32 AT_SetImsRatStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 lteImsSupportFlg;
    VOS_UINT8 lteEmsSupportFlg;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_IMSRATSTUB_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数 LTE IMS是否支持 */
    lteImsSupportFlg = g_atParaList[0].paraValue;

    /* 第二个参数 LTE EMS是否支持 */
    lteEmsSupportFlg = g_atParaList[1].paraValue;

    NAS_SetLteImsSupportFlag(lteImsSupportFlg, lteEmsSupportFlg);

    return AT_OK;
}

VOS_UINT32 AT_SetImsCapabilityStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 voiceCallOnImsSupportFlg;
    VOS_UINT8 videoCallOnImsSupportFlg;
    VOS_UINT8 smsOnImsSupportFlg;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_IMSCAPSTUB_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数表示voice call是否支持 0:不支持;1:支持 */
    voiceCallOnImsSupportFlg = g_atParaList[0].paraValue;

    /* 第二个参数表示video call是否支持 0:不支持;1:支持 */
    videoCallOnImsSupportFlg = g_atParaList[1].paraValue;

    /* 第三个参数表示sms是否支持 0:不支持;1:支持 */
    smsOnImsSupportFlg = g_atParaList[AT_IMSCAPSTUB_SMS].paraValue;

    NAS_SetImsLteCapabilitySupportFlag(voiceCallOnImsSupportFlg, videoCallOnImsSupportFlg, smsOnImsSupportFlg);

    return AT_OK;
}

VOS_UINT32 AT_SetDomainStub(VOS_UINT8 indexNum)
{
    VOS_UINT8  voiceDomain;
    VOS_UINT8  smsDomain;
    VOS_UINT32 updateMmlVoiceDomainFlag;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_DOMAINSTUB_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数 voice domain时，0:CS ONLY; 1:IMS PS ONLY; 2:CS PREFERRED; 3:IMS PS PREFERRED */
    voiceDomain = g_atParaList[0].paraValue;

    /* 第二个参数 sms domain时，  0:NOT USE SMS OVER IP;    1:PREFERR TO USE SMS OVER IP */
    smsDomain = g_atParaList[1].paraValue;

    updateMmlVoiceDomainFlag = g_atParaList[AT_DOMAINSTUB_MMLVOICEDOMAINFLAG].paraValue;

    NAS_SetPreferDomain(voiceDomain, smsDomain, updateMmlVoiceDomainFlag);

    return AT_OK;
}

VOS_UINT32 AT_SetCmdImsRoamingStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 imsRoamingFlag;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取漫游支持参数 */
    imsRoamingFlag = g_atParaList[0].paraValue;

    NAS_SetImsRoaming(imsRoamingFlag);

    return AT_OK;
}

VOS_UINT32 AT_SetCmdRedailStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 callRedialImsToCs;
    VOS_UINT8 smsRedialImsToCs;
    VOS_UINT8 ssRedialImsToCs;
    VOS_UINT8 callRedialCsToIms;
    VOS_UINT8 smsRedialCsToIms;
    VOS_UINT8 ssRedialCsToIms;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_CREDIALSTUB_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* IMS到CS呼叫换域重拨配置 */
    callRedialImsToCs = g_atParaList[0].paraValue;

    /* IMS到CS短信换域重拨配置 */
    smsRedialImsToCs = g_atParaList[1].paraValue;

    /* IMS到CS补充换域重拨配置 */
    ssRedialImsToCs = g_atParaList[AT_CREDIALSTUB_SSREDIALIMSTOCS].paraValue;

    /* CS到IMS呼叫换域重拨配置 */
    callRedialCsToIms = g_atParaList[AT_CREDIALSTUB_CALLREDIALCSTOIMS].paraValue;

    /* CS到IMS短信换域重拨配置 */
    smsRedialCsToIms = g_atParaList[AT_CREDIALSTUB_SMSREDIALCSTOTMS].paraValue;

    /* CS到IMS补充换域重拨配置 */
    ssRedialCsToIms = g_atParaList[AT_CREDIALSTUB_SSREDIALCSTOIMS].paraValue;

    NAS_SetRedialCfg(callRedialImsToCs, smsRedialImsToCs, ssRedialImsToCs, callRedialCsToIms, smsRedialCsToIms,
                     ssRedialCsToIms);

    return AT_OK;
}

VOS_UINT32 AT_SetCmdImsUssdStub(VOS_UINT8 indexNum)
{
    VOS_UINT8 imsUssdFlag;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取漫游支持参数 */
    imsUssdFlag = g_atParaList[0].paraValue;

    NAS_SetImsUssd(imsUssdFlag);

    return AT_OK;
}

VOS_UINT32 At_SetReadUsimStub(VOS_UINT8 indexNum)
{

    return AT_OK;
}

VOS_UINT32 AT_SetNvimPara(VOS_UINT8 indexNum)
{
    NV_IdU16      eNvimTempId;
    VOS_UINT32    tempLen = 0;
    VOS_UINT32    rst;
    VOS_UINT8    *nvTemp = VOS_NULL_PTR;
    VOS_UINT16    length;
    ModemIdUint16 modemId = MODEM_ID_0;
    VOS_UINT32    ret;

    ret = AT_GetModemIdFromClient(indexNum, &modemId);
    if (ret != VOS_OK) {
        AT_ERR_LOG("At_SetCgsnPara: Get modem id fail.");
        return AT_ERROR;
    }

    /* 参数不正确 */
    /* 第一个参数为0表示读NV项值，为1表示写NV项 */
    if (!(((g_atParaList[AT_NVSTUB_READ_OR_WRITE].paraValue == 0) && (g_atParaIndex == AT_NVSTUB_MIN_PARA_NUM)) ||
          ((g_atParaList[AT_NVSTUB_READ_OR_WRITE].paraValue == 1) && (g_atParaIndex == AT_NVSTUB_MAX_PARA_NUM)))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第二个参数为NV项ID */
    eNvimTempId = (NV_IdU16)g_atParaList[AT_NVSTUB_NVIM_ID].paraValue;

    /* 第一个参数为0表示读NV项值，为1表示写NV项 */
    if (g_atParaList[AT_NVSTUB_READ_OR_WRITE].paraValue == 0) {
        (VOS_VOID)TAF_ACORE_NV_GET_LENGTH(eNvimTempId, &tempLen);

         if (tempLen == 0) {
            return AT_ERROR;
        }

        nvTemp = (VOS_UINT8 *)PS_MEM_ALLOC(I0_WUEPS_PID_TAF, tempLen);

        if (nvTemp == VOS_NULL_PTR) {
            AT_ERR_LOG("AT_SetNvimPara:ERROR:ALLOC MEMORY FAIL.");
            return AT_ERROR;
        }

        (VOS_VOID)memset_s(nvTemp, tempLen, 0x00, tempLen);

        rst = AT_ReadValueFromNvim(eNvimTempId, nvTemp, tempLen, modemId);
        if (rst == AT_OK) {
            length = 0;
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "^NVSTUB:");
            length += (TAF_UINT16)At_HexString2AsciiNumPrint(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                             g_atSndCodeAddress + length, nvTemp, (VOS_UINT16)tempLen);

            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            At_SendResultData(indexNum, g_atSndCodeAddress, length);
            rst = AT_SUCCESS;
        }

        /* 增加内存释放 */
        PS_MEM_FREE(I0_WUEPS_PID_TAF, nvTemp);
        nvTemp = VOS_NULL_PTR;
        return rst;
    } else {
        if (At_AsciiNum2HexString(g_atParaList[AT_NVSTUB_WRITE].para,
                                  &g_atParaList[AT_NVSTUB_WRITE].paraLen) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (TAF_ACORE_NV_WRITE_OLD(modemId, eNvimTempId, g_atParaList[AT_NVSTUB_WRITE].para,
                                   g_atParaList[AT_NVSTUB_WRITE].paraLen) != NV_OK) {
            AT_WARN_LOG("AT_SetNvimPara():WARNING:Invoke NVIM Write function falied");
            return AT_ERROR;
        }

        return AT_OK;
    }
}

VOS_UINT32 AT_SetPidReinitPara(VOS_UINT8 indexNum)
{
    return AT_OK;
}

VOS_UINT32 AT_CheckUsimPara(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[AT_USIMWRITESTUB_MSG].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_USIMWRITESTUB_MAX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_USIMWRITESTUB_MSG].para,
                              &g_atParaList[AT_USIMWRITESTUB_MSG].paraLen) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_VOID AT_SetRecordNumAndAppType(VOS_UINT8 *recordNum, USIMM_CardAppUint32 *appType)
{
    if (g_atParaIndex == AT_USIMWRITESTUB_MIN_PARA_NUM) {
        *recordNum = g_atParaList[AT_USIMWRITESTUB_RECORDNUM].paraValue;
    }

    if (g_atParaIndex == AT_USIMWRITESTUB_MAX_PARA_NUM) {
        *recordNum = g_atParaList[AT_USIMWRITESTUB_RECORDNUM].paraValue;

        *appType = (USIMM_CardAppUint32)g_atParaList[AT_USIMWRITESTUB_APPTYPE].paraValue;
    }
}

USIMM_DefFileidUint32 AT_FindUsimFileId(USIMM_CardAppUint32 appType, VOS_UINT8 *foundFlag)
{
    USIMM_DefFileidUint32   fileId = USIMM_DEF_FILEID_BUTT;
    VOS_UINT32              i;

    if (appType == USIMM_GUTL_APP) {
        for (i = 0; i < g_atUsimFileNumToIdTabLen; i++) {
            if (g_atParaList[0].paraValue == g_atUsimFileNumToIdTab[i].fileNum) {
                fileId = g_atUsimFileNumToIdTab[i].fileId;

                *foundFlag = VOS_TRUE;
                break;
            }
        }
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (appType == USIMM_CDMA_APP) {
        for (i = 0; i < g_atCsimFileNumToIdTabLen; i++) {
            if (g_atParaList[0].paraValue == g_atCsimFileNumToIdTab[i].fileNum) {
                fileId = g_atCsimFileNumToIdTab[i].fileId;

                *foundFlag = VOS_TRUE;
                break;
            }
        }
    }
#endif

    return fileId;
}

VOS_UINT32 AT_SetUsimParaSendUpdateFile(VOS_CHAR *filePathStr, VOS_UINT8 recordNum, TAF_UINT8 indexNum,
    USIMM_CardAppUint32 appType)
{
    USIMM_UpdateFileReq    *msg        = VOS_NULL_PTR;
    VOS_UINT32              pathLength = 0;
    VOS_UINT32              msgLength = 0;
    VOS_UINT16              modemId;
    errno_t                 memResult;

    modemId   = MODEM_ID_0;
    AT_GetModemIdFromClient(indexNum, &modemId);

    /* 文件路径长度保护 */
    pathLength = VOS_StrLen(filePathStr);
    if (pathLength == 0) {
        return AT_ERROR;
    }

    msgLength = (sizeof(USIMM_UpdateFileReq) - VOS_MSG_HEAD_LENGTH) +
                 ((VOS_UINT32)g_atParaList[AT_USIMWRITESTUB_MSG].paraLen - AT_THREE_TIMES_LENGTH * sizeof(VOS_UINT8));

    if (msgLength < (sizeof(USIMM_UpdateFileReq) - VOS_MSG_HEAD_LENGTH)) {
        msgLength = sizeof(USIMM_UpdateFileReq) - VOS_MSG_HEAD_LENGTH;
    }

    msg = (USIMM_UpdateFileReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);
    if (msg == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_USIM, msgLength);
    if (modemId == MODEM_ID_1) {
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, I1_WUEPS_PID_USIM, msgLength);
    }

    msg->msgHeader.msgName  = USIMM_UPDATEFILE_REQ;
    msg->msgHeader.appType  = appType;
    msg->msgHeader.sendPara = 0;
    msg->filePath.pathLen   = pathLength;
    memResult = memcpy_s(msg->filePath.path, sizeof(msg->filePath.path), filePathStr, msg->filePath.pathLen + 1);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->filePath.path), msg->filePath.pathLen + 1);
    msg->dataLen   = g_atParaList[AT_USIMWRITESTUB_MSG].paraLen;
    msg->recordNum = recordNum;
    if (g_atParaList[AT_USIMWRITESTUB_MSG].paraLen > 0) {
        memResult = memcpy_s(msg->content, g_atParaList[AT_USIMWRITESTUB_MSG].paraLen,
                             g_atParaList[AT_USIMWRITESTUB_MSG].para,
                             g_atParaList[AT_USIMWRITESTUB_MSG].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_USIMWRITESTUB_MSG].paraLen,
                            g_atParaList[AT_USIMWRITESTUB_MSG].paraLen);
    }

    /* 调用VOS发送原语 */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetUsimParaSendStkRefresh(TAF_UINT8 indexNum)
{
    USIMM_StkrefreshReq    *refreshMsg = VOS_NULL_PTR;
    USIMM_StkCommandDetails cMDDetail;
    USIMM_ResetInfo         rstInfo;
    VOS_UINT32              msgLength = 0;
    VOS_UINT16              modemId;

    modemId   = MODEM_ID_0;
    AT_GetModemIdFromClient(indexNum, &modemId);

    if (g_atParaList[0].paraValue == 0x6F07) {
        (VOS_VOID)memset_s(&cMDDetail, sizeof(cMDDetail), 0x00, sizeof(USIMM_StkCommandDetails));

        cMDDetail.commandQua  = USIMM_RESET;
        cMDDetail.commandNum  = 1;
        cMDDetail.commandType = 1;

        (VOS_VOID)memset_s(&rstInfo, sizeof(rstInfo), 0x00, sizeof(rstInfo));

        /* 分配消息内存并初始化 */
        msgLength  = sizeof(USIMM_StkrefreshReq) - VOS_MSG_HEAD_LENGTH;
        refreshMsg = (USIMM_StkrefreshReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);
        if (refreshMsg == VOS_NULL_PTR) {
            return AT_ERROR;
        }

        /* 填写消息头 */
        TAF_CfgMsgHdr((MsgBlock *)refreshMsg, WUEPS_PID_AT, WUEPS_PID_USIM, msgLength);
        if (modemId == MODEM_ID_1) {
            TAF_CfgMsgHdr((MsgBlock *)refreshMsg, WUEPS_PID_AT, I1_WUEPS_PID_USIM, msgLength);
        }

        refreshMsg->msgHeader.msgName  = USIMM_STKREFRESH_REQ;
        refreshMsg->msgHeader.sendPara = 0;
        refreshMsg->msgHeader.appType  = USIMM_GUTL_APP;

        /* 填写消息体 */
        refreshMsg->commandNum  = cMDDetail.commandNum;
        refreshMsg->commandType = cMDDetail.commandType;
        refreshMsg->refreshType = cMDDetail.commandQua;

        /* 调用VOS发送原语 */
        if (TAF_TraceAndSendMsg(WUEPS_PID_AT, refreshMsg) != VOS_OK) {
            return AT_ERROR;
        }
    }

    return AT_OK;
}

TAF_UINT32 AT_SetUsimPara(TAF_UINT8 indexNum)
{
    VOS_UINT8               recordNum;
    VOS_CHAR               *filePathStr = VOS_NULL_PTR;
    USIMM_DefFileidUint32   fileId = USIMM_DEF_FILEID_BUTT;
    VOS_UINT8               foundFlag;
    USIMM_CardAppUint32     appType;
    VOS_UINT32              result;

    recordNum = 0;
    appType   = USIMM_GUTL_APP;

    result = AT_CheckUsimPara();
    if (result != AT_SUCCESS) {
        return result;
    }

    AT_SetRecordNumAndAppType(&recordNum, &appType);

    /* 当读到定制需求更新0x4F36和0x4F34文件时，要设置enAppType */
    /* 文件Id转文件路径 */
    foundFlag = VOS_FALSE;

    fileId = AT_FindUsimFileId(appType, &foundFlag);

    if (foundFlag == VOS_FALSE) {
        return AT_ERROR;
    }

    if (USIMM_ChangeDefFileToPath(fileId, &filePathStr) != VOS_OK) {
        return AT_ERROR;
    }

    if (AT_SetUsimParaSendUpdateFile(filePathStr, recordNum, indexNum, appType) != AT_OK) {
        return AT_ERROR;
    }

    return AT_SetUsimParaSendStkRefresh(indexNum);
}

VOS_UINT32 AT_CheckSimPara(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen == 0)) {
        return VOS_FALSE;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_SIMWRITESTUB_PARA_MAX_NUM) {
        return VOS_FALSE;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].para,
                              &g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen) == AT_FAILURE) {
        return VOS_FALSE;
    }
    return VOS_TRUE;
}
/*
 * 功能描述: 设置sim卡参数时发送消息USIMM_UPDATEFILE_REQ的处理
 */
VOS_UINT32 AT_SetSimParaSendUpdateFile(VOS_CHAR *filePathStr, VOS_UINT8 recordNum)
{
    USIMM_UpdateFileReq    *msg        = VOS_NULL_PTR;
    VOS_UINT32              pathLength = 0;
    VOS_UINT32              msgLength = 0;
    errno_t                 memResult;

    /* 文件路径长度保护 */
    pathLength = VOS_StrLen(filePathStr);
    if (pathLength == 0) {
        return VOS_FALSE;
    }

    msgLength = (sizeof(USIMM_UpdateFileReq) - VOS_MSG_HEAD_LENGTH) +
        ((VOS_UINT32)g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen - AT_THREE_TIMES_LENGTH * sizeof(VOS_UINT8));

    if (msgLength < (sizeof(USIMM_UpdateFileReq) - VOS_MSG_HEAD_LENGTH)) {
        msgLength = (sizeof(USIMM_UpdateFileReq) - VOS_MSG_HEAD_LENGTH);
    }

    msg = (USIMM_UpdateFileReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);
    if (msg == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_USIM, msgLength);

    msg->msgHeader.msgName  = USIMM_UPDATEFILE_REQ;
    msg->msgHeader.appType  = USIMM_GUTL_APP;
    msg->msgHeader.sendPara = 0;
    msg->filePath.pathLen   = pathLength;
    memResult = memcpy_s(msg->filePath.path, sizeof(msg->filePath.path), filePathStr, msg->filePath.pathLen + 1);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->filePath.path), msg->filePath.pathLen + 1);
    msg->dataLen   = g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen;
    msg->recordNum = recordNum;
    memResult      = memcpy_s(msg->content, g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen,
                              g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].para,
                              g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen,
                        g_atParaList[AT_SIMWRITESTUB_MSGLENGTH].paraLen);

    /* 调用VOS发送原语 */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_SetSimParaSendStkRefresh(VOS_VOID)
{
    USIMM_StkrefreshReq    *refreshMsg = VOS_NULL_PTR;
    USIMM_StkCommandDetails cMDDetail;
    USIMM_ResetInfo         rstInfo;
    VOS_UINT32              msgLength;

    if (g_atParaList[0].paraValue == 0x6F07) {
        (VOS_VOID)memset_s(&cMDDetail, sizeof(cMDDetail), 0x00, sizeof(USIMM_StkCommandDetails));

        cMDDetail.commandQua  = USIMM_RESET;
        cMDDetail.commandNum  = 1;
        cMDDetail.commandType = 1;

        (VOS_VOID)memset_s(&rstInfo, sizeof(rstInfo), 0x00, sizeof(rstInfo));

        /* 分配消息内存并初始化 */
        msgLength  = sizeof(USIMM_StkrefreshReq) - VOS_MSG_HEAD_LENGTH;
        refreshMsg = (USIMM_StkrefreshReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);
        if (refreshMsg == VOS_NULL_PTR) {
            return AT_ERROR;
        }

        /* 填写消息头 */
        TAF_CfgMsgHdr((MsgBlock *)refreshMsg, WUEPS_PID_AT, WUEPS_PID_USIM, msgLength);

        refreshMsg->msgHeader.msgName  = USIMM_STKREFRESH_REQ;
        refreshMsg->msgHeader.sendPara = 0;
        refreshMsg->msgHeader.appType  = USIMM_GUTL_APP;

        /* 填写消息体 */
        refreshMsg->commandNum  = cMDDetail.commandNum;
        refreshMsg->commandType = cMDDetail.commandType;
        refreshMsg->refreshType = cMDDetail.commandQua;

        /* 调用VOS发送原语 */
        if (TAF_TraceAndSendMsg(WUEPS_PID_AT, refreshMsg) != VOS_OK) {
            return AT_ERROR;
        }
    }

    return AT_OK;
}

TAF_UINT32 AT_SetSimPara(TAF_UINT8 indexNum)
{
    VOS_UINT8               recordNum = 0;
    VOS_CHAR               *filePathStr = VOS_NULL_PTR;
    USIMM_DefFileidUint32   fileId;
    VOS_UINT32              i;
    VOS_UINT8               foundFlag;


    if (AT_CheckSimPara() != VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex == AT_SIMWRITESTUB_PARA_MAX_NUM) {
        recordNum = g_atParaList[AT_SIMWRITESTUB_RECORDNUM].paraValue;
    }

    /* 当读到定制需求更新0x4F36和0x4F34文件时，要设置enAppType */
    /* 文件Id转文件路径 */
    foundFlag = VOS_FALSE;
    for (i = 0; i < g_atSimFileNumToIdTabLen; i++) {
        if (g_atParaList[0].paraValue == g_atSimFileNumToIdTab[i].fileNum) {
            fileId    = g_atSimFileNumToIdTab[i].fileId;
            foundFlag = VOS_TRUE;
            break;
        }
    }

    if (foundFlag == VOS_FALSE) {
        return AT_ERROR;
    }

    if (USIMM_ChangeDefFileToPath(fileId, &filePathStr) != VOS_OK) {
        return AT_ERROR;
    }

    if (AT_SetSimParaSendUpdateFile(filePathStr, recordNum) != VOS_TRUE) {
        return AT_ERROR;
    }

    return AT_SetSimParaSendStkRefresh();
}

/* AT_SetSTKParaStub无调用点，删除 */

VOS_UINT32 AT_RefreshUsimPara(VOS_UINT8 indexNum)
{
    USIMM_STKRefreshInd *mmcUsimRefreshMsg = VOS_NULL_PTR;
    USIMM_STKRefreshInd *mmaUsimRefreshMsg = VOS_NULL_PTR;
    VOS_UINT32           rslt;

    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_USIM_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[1].para, &g_atParaList[1].paraLen) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 申请内存  */
    mmcUsimRefreshMsg = (USIMM_STKRefreshInd *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                            sizeof(USIMM_STKRefreshInd) - VOS_MSG_HEAD_LENGTH);

    if (mmcUsimRefreshMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_RefreshUsimPara:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)mmcUsimRefreshMsg, MAPS_PIH_PID, AT_GetDestPid(indexNum, I0_WUEPS_PID_MMC),
                  sizeof(USIMM_STKRefreshInd) - VOS_MSG_HEAD_LENGTH);

    /* 模拟USIM给MMC模块发送消息 */
    mmcUsimRefreshMsg->indHdr.msgName = USIMM_STKREFRESH_IND;
    mmcUsimRefreshMsg->efId[0].fileId = g_atParaList[0].paraValue;
    mmcUsimRefreshMsg->efNum          = 1;
    mmcUsimRefreshMsg->refreshType    = USIMM_REFRESH_FILE_LIST;

    /* 调用VOS发送原语 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, mmcUsimRefreshMsg);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RefreshUsimPara:ERROR:TAF_TraceAndSendMsg ");
        return VOS_ERR;
    }

    /* 申请内存  */
    mmaUsimRefreshMsg = (USIMM_STKRefreshInd *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                            sizeof(USIMM_STKRefreshInd) - VOS_MSG_HEAD_LENGTH);

    if (mmaUsimRefreshMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_RefreshUsimPara:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    TAF_CfgMsgHdr((MsgBlock *)mmaUsimRefreshMsg, MAPS_PIH_PID, AT_GetDestPid(indexNum, I0_WUEPS_PID_MMA),
                  sizeof(USIMM_STKRefreshInd) - VOS_MSG_HEAD_LENGTH);

    /* 模拟USIM给MMA模块发送消息 */
    mmaUsimRefreshMsg->indHdr.msgName = USIMM_STKREFRESH_IND;
    mmaUsimRefreshMsg->efId[0].fileId = g_atParaList[0].paraValue;
    mmaUsimRefreshMsg->efNum          = 1;
    mmaUsimRefreshMsg->refreshType    = USIMM_REFRESH_FILE_LIST;

    /* 调用VOS发送原语 */
    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, mmaUsimRefreshMsg);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("AT_RefreshUsimPara:ERROR:TAF_TraceAndSendMsg ");
        return VOS_ERR;
    }

    return AT_OK;
}

#endif


