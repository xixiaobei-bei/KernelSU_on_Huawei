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
#include "at_voice_hifi_rslt_proc.h"
#include "at_voice_comm.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "at_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_HIFI_RSLT_PROC_C

VOS_UINT32 At_ProcVcGetVolumeEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvt)
{
    VOS_UINT8 *intraVolume = VOS_NULL_PTR;
    VOS_UINT8 atCmdClvlNumMax = 0;
    VOS_UINT8 volumnLvl;
    VOS_UINT32 i;
    VOS_UINT8 telematicIntraVolume[] = { AT_CMD_TELE_CLVL_LEV_0, AT_CMD_TELE_CLVL_LEV_1, AT_CMD_TELE_CLVL_LEV_2,
        AT_CMD_TELE_CLVL_LEV_3, AT_CMD_TELE_CLVL_LEV_4, AT_CMD_TELE_CLVL_LEV_5, AT_CMD_TELE_CLVL_LEV_6,
        AT_CMD_TELE_CLVL_LEV_7, AT_CMD_TELE_CLVL_LEV_8, AT_CMD_TELE_CLVL_LEV_9, AT_CMD_TELE_CLVL_LEV_10,
        AT_CMD_TELE_CLVL_LEV_11, AT_CMD_TELE_CLVL_LEV_12 };
    VOS_UINT8 nonTelematicIntraVolume[] = { AT_CMD_CLVL_LEV_0, AT_CMD_CLVL_LEV_1,
        AT_CMD_CLVL_LEV_2, AT_CMD_CLVL_LEV_3, AT_CMD_CLVL_LEV_4, AT_CMD_CLVL_LEV_5 };

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_TRUE) {
        intraVolume = telematicIntraVolume;
        atCmdClvlNumMax = AT_CMD_TELE_CLVL_VAL_MAX;
    } else {
        intraVolume = nonTelematicIntraVolume;
        atCmdClvlNumMax = AT_CMD_CLVL_VAL_MAX;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("APP_VC_AppQryVolumeProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CLVL_READ) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (vcEvt->success == VOS_TRUE) {
        /* 格式化AT+CLVL命令返回 */
        g_atSendDataBuff.bufLen = 0;

        volumnLvl = 0;
        for (i = 0; i < atCmdClvlNumMax; i++) {
            if (intraVolume[i] == vcEvt->volume) {
                volumnLvl = (VOS_UINT8)i;
                break;
            }
        }

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, volumnLvl);

        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}
VOS_UINT32 At_ProcVcSetVolumeEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvt)
{
    TAF_UINT32 ret;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_VcEventProc : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (vcEvt->success != TAF_TRUE) {
        ret = AT_ERROR;
    } else {
        ret = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 At_ProcVcSetVoiceMode(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvt)
{
    AT_VmsetCmdCtx *vmSetCmdCtx = VOS_NULL_PTR;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcSetVoiceMode : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 状态判断 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_VMSET_SET) {
        AT_WARN_LOG("At_ProcVcSetVoiceMode : opt error.");
        return VOS_ERR;
    }

    vmSetCmdCtx = AT_GetCmdVmsetCtxAddr();

    if (vcEvt->success != VOS_TRUE) {
        vmSetCmdCtx->result = AT_ERROR;
    }
#if (MULTI_MODEM_NUMBER >= 2)
    /* VMSET收齐所有MODEM回复后再上报结果 */
    vmSetCmdCtx->reportedModemNum++;
    if (vmSetCmdCtx->reportedModemNum < MULTI_MODEM_NUMBER) {
        return VOS_ERR;
    }
#endif

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, vmSetCmdCtx->result);

    /* 初始化设置结果全局变量 */
    AT_InitVmSetCtx();
    return VOS_OK;
}

VOS_VOID At_RcvVcMsgQryModeCnfProc(MN_AT_IndEvt *data)
{
    MN_AT_IndEvt     *rcvMsg   = VOS_NULL_PTR;
    APP_VC_EventInfo *event    = VOS_NULL_PTR;
    VOS_UINT8         indexNum = 0;
    VOS_UINT16        devMode;
    VOS_UINT32        ret;

    /* 初始化 */
    rcvMsg = data;
    event  = (APP_VC_EventInfo *)data->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(rcvMsg->clientId, AT_CMD_VMSET_READ, &indexNum) != VOS_OK) {
        return;
    }
    devMode = event->devMode;

    /* VC上报的UAC转换为和AP交互的UAC */
    if (devMode == VC_PHY_DEVICE_MODEM_UAC) {
        devMode = AT_SET_VMSET_DEVICE_MODEM_UAC;
    }
    /* 无效的模式，直接返回ERROR */
    if (devMode >= VC_PHY_DEVICE_MODE_BUTT) {
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, devMode);
        ret                     = AT_OK;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_UINT32 At_ProcVcSetMuteStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 rslt;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMUT_SET) {
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

VOS_UINT32 At_ProcVcGetMuteStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 rslt;
    VOS_UINT16 length = 0;

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcSetMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMUT_READ) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            vcEvtInfo->muteStatus);

        rslt = AT_OK;

        g_atSendDataBuff.bufLen = length;
    } else {
        rslt = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_ProcVcReportEcallAlackEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    NAS_MM_InfoInd      localAtTimeInfo = {0};
    NAS_MM_InfoInd      cCLKTimeInfo = {0};
    ModemIdUint16       modemId            = MODEM_ID_0;
    AT_ECALL_AlackInfo *ecallAlackInfoAddr = VOS_NULL_PTR;
    VOS_UINT32          writeNum = 0;
    VOS_UINT16          length   = 0;
    AT_ModemNetCtx     *netCtx   = VOS_NULL_PTR;
    errno_t             memResult;

    if (vcEvtInfo == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_ERR_LOG("AT_ProcVcReportEcallAlackEvent: Get modem id fail.");
        return VOS_ERR;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    if ((netCtx->timeInfo.ieFlg & NAS_MM_INFO_IE_UTLTZ) == NAS_MM_INFO_IE_UTLTZ) {
        /* 首先根据网络或者CCLK的时间转换为localAT时间 */
        AT_GetLiveTime(&netCtx->timeInfo, &localAtTimeInfo, netCtx->nwSecond);
    } else {
        memResult = memcpy_s(&cCLKTimeInfo, sizeof(cCLKTimeInfo), &netCtx->timeInfo, sizeof(NAS_MM_InfoInd));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(cCLKTimeInfo), sizeof(NAS_MM_InfoInd));
        cCLKTimeInfo.ieFlg = NAS_MM_INFO_IE_UTLTZ;
        AT_GetLiveTime(&cCLKTimeInfo, &localAtTimeInfo, 1);
    }

    /* 存储时间和ALACK 值 */
    ecallAlackInfoAddr = AT_EcallAlAckInfoAddr();
    if (ecallAlackInfoAddr->ecallAlackNum < AT_ECALL_ALACK_NUM) {
        ecallAlackInfoAddr->ecallAlackNum++;
        writeNum = (VOS_UINT32)ecallAlackInfoAddr->ecallAlackNum - 1;
    } else {
        ecallAlackInfoAddr->ecallAlackBeginNum += 1;
        ecallAlackInfoAddr->ecallAlackBeginNum %= AT_ECALL_ALACK_NUM;
        /*
         * EcallAlAckList的BeginNum等于0时，writeNum表示循环数组EcallAlAckList的前一个索引，
         * 应该等于AT_ECALL_ALACK_NUM - 1
         */
        if (ecallAlackInfoAddr->ecallAlackBeginNum == 0) {
            writeNum = AT_ECALL_ALACK_NUM - 1;
        } else {
            writeNum = (VOS_UINT32)ecallAlackInfoAddr->ecallAlackBeginNum - 1;
        }
    }

    memResult = memcpy_s(&ecallAlackInfoAddr->ecallAlackInfo[writeNum].ecallAlackTimeInfo, sizeof(NAS_MM_InfoInd),
        &localAtTimeInfo, sizeof(localAtTimeInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(NAS_MM_InfoInd), sizeof(localAtTimeInfo));
    ecallAlackInfoAddr->ecallAlackInfo[writeNum].ecallAlackValue = vcEvtInfo->ecallReportAlackValue;

    /* 显示时间和ALACK 值 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^ECLREC: ", g_atCrLf);
    AT_EcallAlackDisplay(ecallAlackInfoAddr->ecallAlackInfo[writeNum], &length);

    /* 回车换行 */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    At_SendResultData(indexNum, g_atSndCodeAddress, length);
    At_SendResultData(AT_CLIENT_ID_CTRL, g_atSndCodeAddress, length);
    return VOS_OK;
}
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 At_ProcVcSetSpeakerMuteStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 result;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcSetSpeakerMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SMUT_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 At_ProcVcGetSpeakerMuteStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 result;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcGetSpeakerMuteStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SMUT_READ) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (vcEvtInfo->success == VOS_TRUE) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            vcEvtInfo->speakerMuteStatus);

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 At_ProcVcGetCmicVolumeEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    const VOS_UINT8 intraVolume[] = { AT_CMD_CMIC_LEV_1, AT_CMD_CMIC_LEV_2, AT_CMD_CMIC_LEV_3, AT_CMD_CMIC_LEV_4,
        AT_CMD_CMIC_LEV_5, AT_CMD_CMIC_LEV_6, AT_CMD_CMIC_LEV_7, AT_CMD_CMIC_LEV_8, AT_CMD_CMIC_LEV_9,
        AT_CMD_CMIC_LEV_10, AT_CMD_CMIC_LEV_11, AT_CMD_CMIC_LEV_12 };
    VOS_UINT8 volumeLevel;
    VOS_UINT32 i;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcGetCmicVolumeEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMIC_READ) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (vcEvtInfo->success == VOS_TRUE) {
        g_atSendDataBuff.bufLen = 0;

        volumeLevel = 0;
        for (i = 0; i < sizeof(intraVolume) / sizeof(intraVolume[0]); i++) {
            if (vcEvtInfo->upVolume == intraVolume[i]) {
                volumeLevel = (VOS_UINT8)i + 1;
                break;
            }
        }

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, volumeLevel);

        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

VOS_UINT32 At_ProcVcSetCmicStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 result;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("At_ProcVcSetCmicStatusEvent: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CMIC_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_ProcVcSetEchoSuppressionEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 result;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcSetEchoSuppressionEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ECHOEX_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_ProcVcGetEchoSuppressionEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcGetEchoSuppressionEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ECHOEX_READ) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (vcEvtInfo->success == VOS_TRUE) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            vcEvtInfo->echo[0], vcEvtInfo->echo[1], vcEvtInfo->echo[2], vcEvtInfo->echo[3], vcEvtInfo->echo[4]);

        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }
    return VOS_OK;
}

VOS_UINT32 AT_ProcVcSetPcmFrStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 result;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcSetPcmFrStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PCMFR_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_ProcVcGetPcmFrStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT16 length = 0;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }


    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcGetPcmFrStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PCMFR_READ) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);
    if (vcEvtInfo->success == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^PCMFR: ", g_atCrLf);

        length += (VOS_UINT16)At_HexText2AsciiStringSimple(AT_CMD_MAX_LEN,
                                                           (VOS_INT8 *)g_atSndCodeAddress,
                                                           (VOS_UINT8 *)g_atSndCodeAddress + length,
                                                           MN_CALL_PCM_FR_NUM,
                                                           (VOS_UINT8 *)vcEvtInfo->pcmFr);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}


/* 处理APP_VC_EVT_SET_ECHOSWITCH事件 */
VOS_UINT32 AT_ProcVcSetEchoSwitchStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{
    VOS_UINT32 result;

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcSetEchoSwitchStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ECHOSWITCH_SET) {
        return VOS_ERR;
    }

    if (vcEvtInfo->success != VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

/* 处理APP_VC_EVT_GET_ECHOSWITCH事件 */
VOS_UINT32 AT_ProcVcGetEchoSwitchStatusEvent(VOS_UINT8 indexNum, APP_VC_EventInfo *vcEvtInfo)
{

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_FALSE) {
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_ProcVcGetEchoSwitchStatusEvent : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 当前AT是否在等待该命令返回 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_ECHOSWITCH_READ) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (vcEvtInfo->success == VOS_TRUE) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d",g_parseContext[indexNum].cmdElement->cmdName,
            vcEvtInfo->currEchoSwitchStatus);

        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}
#endif

