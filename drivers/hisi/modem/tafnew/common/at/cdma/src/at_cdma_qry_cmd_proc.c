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

#include "at_cdma_qry_cmd_proc.h"
#include "at_cdma_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "ps_lib.h"




/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CDMA_QRY_CMD_PROC_C

VOS_UINT32 AT_QryCasState(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_ProcStateQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_GETSTA_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
#else
    return AT_ERROR;
#endif
}

VOS_UINT32 AT_QryHighVer(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_ProcCHVerQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CHIGHVER_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
#else
    return AT_ERROR;
#endif
}

VOS_UINT32 AT_QryCtaPara(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 执行命令操作 */
    if (TAF_PS_GetCtaInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CTA_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;

#else
    return AT_ERROR;
#endif
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_QryCFreqLockInfo(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_ProcCFreqLockQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFREQLOCK_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
#else
    return AT_ERROR;
#endif
}
#endif

VOS_UINT32 AT_QryHdrCsqPara(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_ProcHdrCsqQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HDR_CSQ_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;

#else
    return AT_ERROR;
#endif
}

VOS_UINT32 AT_QryCurrSidNid(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = TAF_MMA_QryCurrSidNid(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSNID_QRY;

    return AT_WAIT_ASYNC_RETURN;
#else
    return AT_ERROR;
#endif
}

VOS_UINT32 AT_QryProRevInUse(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_ProcProRevInUseQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CVER_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
#else
    return AT_ERROR;
#endif
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 At_TestCBurstDTMFPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length,
        "^CBURSTDTMF: (1-7),(0-9,*,#),(95,150,200,250,300,350),(60,100,150,200)");
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 At_TestCContinuousDTMFPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^CCONTDTMF: (1-7),(0,1),(0-9,*,#)");
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_Qry1xChanPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_Proc1xChanQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_1XCHAN_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 AT_QryCdmaCsqPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    if (TAF_MMA_ProcCdmaCsqQryReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CDMACSQ_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCLocInfo(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 向MMA发消息请求运营商信息 */
    ret = TAF_MMA_QryCLocInfoReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLOCINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)
VOS_UINT32 AT_QryEncryptCallCap(VOS_UINT8 indexNum)
{
    VOS_UINT32    ret;
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_ECC_SRV_CAP_QRY_REQ消息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_ECC_SRV_CAP_QRY_REQ, 0, modemId);

    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCAP_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
VOS_UINT32 AT_QryEncryptCallRandom(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_GET_EC_RANDOM_REQ消息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_GET_EC_RANDOM_REQ, 0, modemId);
    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECRANDOM_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryEncryptCallKmc(VOS_UINT8 indexNum)
{
    VOS_UINT32    ret;
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_GET_EC_KMC_REQ消息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_GET_EC_KMC_REQ, 0, modemId);
    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECKMC_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryEccTestMode(VOS_UINT8 indexNum)
{
    VOS_UINT32    ret;
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_GET_EC_TEST_MODE_REQ消息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_GET_EC_TEST_MODE_REQ, 0, modemId);
    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCTEST_QRY;

    return AT_WAIT_ASYNC_RETURN;
}
#endif
#endif

VOS_UINT32 AT_QryCtRoamInfo(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = TAF_MMA_QryCtRoamInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CTROAMINFO_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryPrivacyModePreferred(VOS_UINT8 indexNum)
{
    TAF_Ctrl      ctrl;
    ModemIdUint16 modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 检查当前命令操作类型 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_PRIVACY_MODE_QRY_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, VOS_NULL_PTR, ID_TAF_CCM_PRIVACY_MODE_QRY_REQ, 0, modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPMP_QRY;

        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_QryPRLID(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = TAF_MMA_QryPrlIdInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PRLID_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryNoCardMode(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_NO_CARD_MODE_QRY_REQ, VOS_NULL_PTR, 0,
                                 I0_UEPS_PID_MTA);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryNoCardMode: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NOCARDMODE_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryRatCombinedMode(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    ret = TAF_MMA_QryRatCombinedMode(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0);
    if (ret != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RATCOMBINEDMODE_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestCclprPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-7)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaQryCurrSidNidCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_CurrSidNidQryCnf *qryCurrSidNidCnf = (TAF_MMA_CurrSidNidQryCnf *)msg;

    /* 格式化AT^CSNID查询命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (qryCurrSidNidCnf->errorCode == TAF_ERR_NO_ERROR) {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            qryCurrSidNidCnf->sid, qryCurrSidNidCnf->nid);
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCdmaDormTimerVal(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 执行命令操作 */
    if (TAF_PS_ProcCdmaDormTimerQryReq(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0) !=
        VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DORMTIMER_QRY;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryProGetEsn(VOS_UINT8 indexNum)
{
    NV_ESN_Meid esnMeId;
    VOS_INT32   loop;
    VOS_UINT32  esn = 0;

    (VOS_VOID)memset_s(&esnMeId, sizeof(esnMeId), 0x00, sizeof(NV_ESN_Meid));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 读取en_NV_Item_ESN_MEID */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_ESN_MEID, &esnMeId, sizeof(NV_ESN_Meid)) != NV_OK) {
        AT_WARN_LOG("AT_QryProGetEsn:Read Nvim Failed");
        return AT_ERROR;
    }

    /* 若未使能，回复失败还是填0 ??? */
    if ((esnMeId.esnMeIDFlag == ESN_ME_ENABLE_FLAG) || (esnMeId.esnMeIDFlag == ESN_MEID_ENABLE_BOTH)) {
        /* Read from high to low */
        for (loop = PS_MIN(esnMeId.esn[0], sizeof(esnMeId.esn) - 1); loop > 0; loop--) {
            /* Update the current value, 手机ESN内容，第一个字节表示长度信息，是Uint32类型，所以乘8 */
            esn <<= sizeof(esnMeId.esn[0]) * 8;

            /* Add lower bits */
            esn |= esnMeId.esn[loop];
        }

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, esn);

    } else {
        AT_WARN_LOG("AT_QryProGetEsn: ESN is disabled");
    }

    return AT_OK;
}

VOS_UINT32 AT_QryProGetMeid(VOS_UINT8 indexNum)
{
    NV_ESN_Meid esnMeId;
    VOS_UINT8   meId[2 * AT_MEID_OCTET_NUM + 1];

    (VOS_VOID)memset_s(&esnMeId, sizeof(esnMeId), 0x00, sizeof(NV_ESN_Meid));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 读取en_NV_Item_ESN_MEID */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_ESN_MEID, &esnMeId, sizeof(NV_ESN_Meid)) != NV_OK) {
        AT_WARN_LOG("AT_QryProGetMeid:Read Nvim Failed");
        return AT_ERROR;
    }

    /* 若未使能，回复失败还是填0 ??? */
    if ((esnMeId.esnMeIDFlag == MEID_ME_ENABLE_FLAG) || (esnMeId.esnMeIDFlag == ESN_MEID_ENABLE_BOTH)) {
        AT_Hex2Ascii_Revers(&(esnMeId.meId[1]), AT_MEID_OCTET_NUM, meId);
        meId[2 * AT_MEID_OCTET_NUM] = '\0';

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s%s", g_parseContext[indexNum].cmdElement->cmdName, meId, g_atCrLf);

    } else {
        AT_WARN_LOG("AT_QryProGetMeid: MEID is disabled");
    }

    return AT_OK;
}

VOS_UINT32 AT_QryMeidPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_MEID_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MEID_QRY;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("AT_QryMeidPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
}

#if (MULTI_MODEM_NUMBER >= 2)
VOS_UINT32 AT_CheckCModeSupport(VOS_UINT32 *modem0SupportCMode,
#if (MULTI_MODEM_NUMBER == 3)
    VOS_UINT32 *modem2SupportCMode,
#endif
    VOS_UINT32 *modem1SupportCMode)
{
    PLATAFORM_RatCapability modem0PlatRat = {0};
    VOS_UINT32              modem0Support1XFlg;
    VOS_UINT32              modem0SupportHrpdFlg;
    PLATAFORM_RatCapability modem1PlatRat = {0};
    VOS_UINT32              modem1Support1XFlg;
    VOS_UINT32              modem1SupportHrpdFlg;
#if (MULTI_MODEM_NUMBER == 3)
    PLATAFORM_RatCapability modem2PlatRat = {0};
    VOS_UINT32              modem2Support1XFlg;
    VOS_UINT32              modem2SupportHrpdFlg;
#endif

    /*  查询读取nv，修改为从使用区读取，不从工作区读取 */
    if (NV_OK !=
        TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PLATFORM_RAT_CAP, &modem0PlatRat, sizeof(PLATAFORM_RatCapability))) {
        return AT_ERROR;
    } else {
        AT_NORM_LOG("Read PlatForm from success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, &modem0PlatRat);
    }

    if (NV_OK !=
        TAF_ACORE_NV_READ(MODEM_ID_1, NV_ITEM_PLATFORM_RAT_CAP, &modem1PlatRat, sizeof(PLATAFORM_RatCapability))) {
        return AT_ERROR;
    } else {
        AT_NORM_LOG("Read PlatForm from success: ");
        AT_ReadPlatFormPrint(MODEM_ID_1, &modem1PlatRat);
    }

#if (MULTI_MODEM_NUMBER == 3)
    if (TAF_ACORE_NV_READ(MODEM_ID_2, NV_ITEM_PLATFORM_RAT_CAP, &modem2PlatRat, sizeof(PLATAFORM_RatCapability)) !=
        NV_OK) {
        return AT_ERROR;
    }
#endif

    modem0Support1XFlg   = AT_IsPlatformSupport1XMode(&modem0PlatRat);
    modem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem0PlatRat);
    modem1Support1XFlg   = AT_IsPlatformSupport1XMode(&modem1PlatRat);
    modem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem1PlatRat);
#if (MULTI_MODEM_NUMBER == 3)
    modem2Support1XFlg   = AT_IsPlatformSupport1XMode(&modem2PlatRat);
    modem2SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem2PlatRat);
#endif
    *modem0SupportCMode = (modem0Support1XFlg || modem0SupportHrpdFlg);
    *modem1SupportCMode = (modem1Support1XFlg || modem1SupportHrpdFlg);
#if (MULTI_MODEM_NUMBER == 3)
    *modem2SupportCMode = (modem2Support1XFlg || modem2SupportHrpdFlg);
#endif
    return AT_OK;
}

LOCAL VOS_UINT32 At_QryCdmaModemSwitchCdmaCapCheck(VOS_UINT32 modem0SupportCMode, VOS_UINT32 modem1SupportCMode,
#if (MULTI_MODEM_NUMBER == 3)
                                                   VOS_UINT32 ulModem2SupportCMode,
#endif
                                                   VOS_UINT32 *pulAllModemNotSupportCMode)
{
    VOS_UINT32 checkRlt;

    checkRlt = AT_SUCCESS;

    /* 如果modem0和modem1同时满足支持，则认为设置失败 */
    if ((modem0SupportCMode == VOS_TRUE) && (modem1SupportCMode == VOS_TRUE)) {
        checkRlt = AT_ERROR;
    }

#if (MULTI_MODEM_NUMBER == 3)
    /* 如果modem0和modem2同时满足支持，则认为设置失败 */
    if ((modem0SupportCMode == VOS_TRUE) && (ulModem2SupportCMode == VOS_TRUE)) {
        checkRlt = AT_ERROR;
    }

    /* 如果modem1和modem2同时满足支持，则认为设置失败 */
    if ((modem1SupportCMode == VOS_TRUE) && (ulModem2SupportCMode == VOS_TRUE)) {
        checkRlt = AT_ERROR;
    }
#endif

    if ((modem0SupportCMode == VOS_FALSE) && (modem1SupportCMode == VOS_FALSE)
#if (MULTI_MODEM_NUMBER == 3)
        && (ulModem2SupportCMode == VOS_FALSE))
#else
    )
#endif
    {
        checkRlt                    = AT_ERROR;
        *pulAllModemNotSupportCMode = VOS_TRUE;
    }

    return checkRlt;
}

#endif

VOS_UINT32 At_QryCdmaModemSwitch(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    VOS_UINT32              modem0SupportCMode;
    VOS_UINT16              length;
#if (MULTI_MODEM_NUMBER >= 2)
    VOS_UINT32              modem1SupportCMode;
    VOS_UINT32              checkRlt;
    VOS_UINT32              allModemNotSupportCMode;
#if (MULTI_MODEM_NUMBER == 3)
    VOS_UINT32              modem2SupportCMode;
#endif

    checkRlt                = AT_SUCCESS;
    allModemNotSupportCMode = VOS_FALSE;

    if (AT_CheckCModeSupport(&modem0SupportCMode,
#if (MULTI_MODEM_NUMBER == 3)
                             &modem2SupportCMode,
#endif
                             &modem1SupportCMode) == AT_ERROR) {
        return AT_ERROR;
    }

    checkRlt = At_QryCdmaModemSwitchCdmaCapCheck(modem0SupportCMode, modem1SupportCMode,
#if (MULTI_MODEM_NUMBER == 3)
                                                 modem2SupportCMode,
#endif
                                                 &allModemNotSupportCMode);

    if (checkRlt == AT_ERROR) {
        if (allModemNotSupportCMode == VOS_TRUE) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: -1", g_parseContext[indexNum].cmdElement->cmdName);

            g_atSendDataBuff.bufLen = length;

            return AT_OK;
        }

        return AT_ERROR;
    }

    /* 1X,DO在Modem0 */
    if (modem0SupportCMode == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 0", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }

    /* 1X,DO在Modem1 */
    if (modem1SupportCMode == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 1", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }

#if (MULTI_MODEM_NUMBER == 3)
    /* 1X,DO在Modem2 */
    if (modem2SupportCMode == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 2", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }
#endif
#elif (MULTI_MODEM_NUMBER == 1)
    PLATAFORM_RatCapability modem0PlatRat;
    VOS_UINT32              modem0Support1XFlg;
    VOS_UINT32              modem0SupportHrpdFlg;
    (VOS_VOID)memset_s(&modem0PlatRat, sizeof(modem0PlatRat), 0x00, sizeof(modem0PlatRat));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PLATFORM_RAT_CAP, &modem0PlatRat, sizeof(PLATAFORM_RatCapability))
        != NV_OK) {
        return AT_ERROR;
    } else {
        AT_NORM_LOG("Read PlatForm from success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, &modem0PlatRat);
    }

    modem0Support1XFlg   = AT_IsPlatformSupport1XMode(&modem0PlatRat);
    modem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem0PlatRat);
    modem0SupportCMode = (modem0Support1XFlg || modem0SupportHrpdFlg);

    /* 1X,DO在Modem0 */
    if (modem0SupportCMode == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 0", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    } else {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: -1", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }
#endif
#endif

    return AT_OK;
}

/*
 * 功能描述: 获取MODEM0和MODEM1支持CDMA模的情况
 */
LOCAL VOS_UINT32 At_GetFactoryCdmaMode(VOS_UINT32 *modem0SupportCMode, VOS_UINT32 *modem1SupportCMode)
{
    PLATAFORM_RatCapability modem0PlatRat;
    PLATAFORM_RatCapability modem1PlatRat;
    VOS_UINT32              modem0Support1XFlg;
    VOS_UINT32              modem0SupportHrpdFlg;
    VOS_UINT32              modem1Support1XFlg;
    VOS_UINT32              modem1SupportHrpdFlg;

    (VOS_VOID)memset_s(&modem0PlatRat, sizeof(modem0PlatRat), 0x00, sizeof(modem0PlatRat));
    (VOS_VOID)memset_s(&modem1PlatRat, sizeof(modem1PlatRat), 0x00, sizeof(modem1PlatRat));

    /*  查询读取nv，修改为从备份区读取，不从工作区读取 */
    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_0, NV_ITEM_PLATFORM_RAT_CAP, &modem0PlatRat,
                                  sizeof(PLATAFORM_RatCapability)) != NV_OK) {
        AT_WARN_LOG("Read PlatForm from Factory fail: MODEM_ID_0");
        return VOS_FALSE;
    } else {
        AT_NORM_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, &modem0PlatRat);
    }

    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_1, NV_ITEM_PLATFORM_RAT_CAP, &modem1PlatRat,
                                  sizeof(PLATAFORM_RatCapability)) != NV_OK) {
        AT_WARN_LOG("Read PlatForm from Factory fail: MODEM_ID_1");
        return VOS_FALSE;
    } else {
        AT_NORM_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_1, &modem1PlatRat);
    }

    modem0Support1XFlg   = AT_IsPlatformSupport1XMode(&modem0PlatRat);
    modem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem0PlatRat);
    modem1Support1XFlg   = AT_IsPlatformSupport1XMode(&modem1PlatRat);
    modem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem1PlatRat);

    AT_NORM_LOG2("At_GetFactoryCdmaMode modem0 cdma cpa: ", modem0Support1XFlg, modem0SupportHrpdFlg);
    AT_NORM_LOG2("At_GetFactoryCdmaMode modem1 cdma cpa: ", modem1Support1XFlg, modem1SupportHrpdFlg);

    (*modem0SupportCMode) = (modem0Support1XFlg | modem0SupportHrpdFlg);
    (*modem1SupportCMode) = (modem1Support1XFlg | modem1SupportHrpdFlg);

    return VOS_TRUE;
}

VOS_UINT32 At_QryFactoryCdmaCap(VOS_UINT8 indexNum)
{
    VOS_UINT32              modem0SupportCMode;
    VOS_UINT32              modem1SupportCMode;
    VOS_UINT32              checkRlt;
    VOS_UINT32              allModemNotSupportCMode;
    VOS_UINT16              length;

    checkRlt                = AT_SUCCESS;
    allModemNotSupportCMode = VOS_FALSE;

    AT_NORM_LOG("At_QryFactoryCdmaCap Entry: ");

    if (At_GetFactoryCdmaMode(&modem0SupportCMode, &modem1SupportCMode) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 如果modem0和modem1同时满足支持，则认为设置失败 */
    if ((modem0SupportCMode == VOS_TRUE) && (modem1SupportCMode == VOS_TRUE)) {
        checkRlt = AT_ERROR;
    }

    if ((modem0SupportCMode == VOS_FALSE) && (modem1SupportCMode == VOS_FALSE)) {
        checkRlt                = AT_ERROR;
        allModemNotSupportCMode = VOS_TRUE;
    }

    if (checkRlt == AT_ERROR) {
        if (allModemNotSupportCMode == VOS_TRUE) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: -1", g_parseContext[indexNum].cmdElement->cmdName);

            g_atSendDataBuff.bufLen = length;

            return AT_OK;
        }

        return AT_ERROR;
    }

    /* 1X,DO在Modem0 */
    if (modem0SupportCMode == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 0", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;

        return AT_OK;
    }

    /* 1X,DO在Modem1 */
    if (modem1SupportCMode == VOS_TRUE) {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 1", g_parseContext[indexNum].cmdElement->cmdName);

        g_atSendDataBuff.bufLen = length;
    }

    return AT_OK;
}

VOS_UINT32 AT_TestClDbDomainStatusPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_TEST_CMD) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-1),(0-1),(0-1)", g_parseContext[indexNum].cmdElement->cmdName);

    return AT_OK;
}

#endif


