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
#include "at_voice_hifi_set_cmd_proc.h"
#include "securec.h"

#include "at_snd_msg.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "at_event_report.h"
#include "at_voice_comm.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "ccore_nv_stru_hifi.h"
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_HIFI_SET_CMD_PROC_C

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_SetEclpushPara(VOS_UINT8 indexNum)
{
    VOS_UINT32          rst;
    APP_VC_MsgSetMsdReq eclmsdPara;

    /* 不带参数的设置命令 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_ERROR;
    }

    /* 当前非Ecall激活状态返回ERROR */
    if (AT_HaveEcallActive(indexNum, VOS_TRUE) == VOS_FALSE) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(&eclmsdPara, sizeof(APP_VC_MsgSetMsdReq), 0x00, sizeof(APP_VC_MsgSetMsdReq));
    /* 往VC发送APP_VC_MSG_ECALL_PUSH_REQ命令 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 APP_VC_MSG_SET_ECALL_PUSH_REQ, (VOS_UINT8 *)&eclmsdPara, sizeof(eclmsdPara),
                                 I0_WUEPS_PID_VC);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLPUSH_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif


VOS_UINT32 At_CheckClvlPara(VOS_UINT8 indexNum)
{
    if (g_atParaIndex != 1) {
        return VOS_FALSE;
    }

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_TRUE) {
        if ((g_atParaList[0].paraValue > AT_CMD_CLVL_TELEMATIC_MAX_PARA) || (g_atParaList[0].paraValue == 0)) {
            return VOS_FALSE;
        }

    } else {
        if (g_atParaList[0].paraValue > AT_CMD_CLVL_NON_TELEMATIC_MAX_PARA) {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

VOS_UINT32 At_SetClvlPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 *intraVolume = VOS_NULL_PTR;
    VOS_UINT8 telematicIntraVolumeData[] = { AT_CMD_TELE_CLVL_LEV_0, AT_CMD_TELE_CLVL_LEV_1, AT_CMD_TELE_CLVL_LEV_2,
        AT_CMD_TELE_CLVL_LEV_3, AT_CMD_TELE_CLVL_LEV_4, AT_CMD_TELE_CLVL_LEV_5, AT_CMD_TELE_CLVL_LEV_6,
        AT_CMD_TELE_CLVL_LEV_7, AT_CMD_TELE_CLVL_LEV_8, AT_CMD_TELE_CLVL_LEV_9, AT_CMD_TELE_CLVL_LEV_10,
        AT_CMD_TELE_CLVL_LEV_11, AT_CMD_TELE_CLVL_LEV_12 };

    VOS_UINT8 nonTelematicIntraVolumeData[] = { AT_CMD_CLVL_LEV_0, AT_CMD_CLVL_LEV_1, AT_CMD_CLVL_LEV_2,
            AT_CMD_CLVL_LEV_3, AT_CMD_CLVL_LEV_4,AT_CMD_CLVL_LEV_5 };

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_TRUE) {
        intraVolume = telematicIntraVolumeData;
    } else {
        intraVolume = nonTelematicIntraVolumeData;
    }

    if ((g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) ||
        (At_CheckClvlPara(indexNum) == VOS_FALSE)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * 这里做了一个转换，目前后台要求的范围是0~5，而我们对应的范围是0~92，
     * 根据和媒体的确认，设定范围是0~80，线性对应
     */

    /* 返回值为AT_SUCCESS改为VOS_OK，对应起来  */
    if (APP_VC_SetVoiceVolume(g_atClientTab[indexNum].clientId, 0, intraVolume[g_atParaList[0].paraValue]) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLVL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetVMSETPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 voiceMode;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不等于1 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* B5000平台不支持模式5 */
    if ((AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_TRUE) &&
        ((g_atParaList[0].paraValue == VC_PHY_DEVICE_MODE_PCVOICE) ||
        (g_atParaList[0].paraValue > VC_PHY_DEVICE_MODE_HEADPHONE))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    voiceMode = (VOS_UINT8)g_atParaList[0].paraValue;

    /* AP下发的UAC转换为和VC交互的UAC */
    if (voiceMode == AT_SET_VMSET_DEVICE_MODEM_UAC) {
        voiceMode = VC_PHY_DEVICE_MODEM_UAC;
    }

#if (MULTI_MODEM_NUMBER >= 2)
    if (APP_VC_SetVoiceMode(g_atClientTab[indexNum].clientId, 0, voiceMode, I1_WUEPS_PID_VC) != VOS_OK) {
        return AT_ERROR;
    }

#if (MULTI_MODEM_NUMBER == 3)
    if (APP_VC_SetVoiceMode(g_atClientTab[indexNum].clientId, 0, voiceMode, I2_WUEPS_PID_VC) != VOS_OK) {
        return AT_ERROR;
    }
#endif
#endif

    if (APP_VC_SetVoiceMode(g_atClientTab[indexNum].clientId, 0, voiceMode, I0_WUEPS_PID_VC) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VMSET_SET;

    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 AT_SetCmutPara(VOS_UINT8 indexNum)
{
    APP_VC_MuteStatusUint8 muteStatus;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    VOS_UINT8 numOfCalls;
    VOS_UINT32 result;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];

    (void)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));
#endif

    /* 无参数 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多或者错误 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取命令参数, 提取要设置的静音状态 */
    if (g_atParaList[0].paraValue == 0) {
        muteStatus = APP_VC_MUTE_STATUS_UNMUTED;
    } else {
        muteStatus = APP_VC_MUTE_STATUS_MUTED;
    }

#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 通话中不支持设置 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_TRUE) {
        result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);
        if (result != VOS_OK) {
            return AT_ERROR;
        }

        if (numOfCalls == 0) {
            return AT_ERROR;
        }
    }
#endif

    /* 执行命令操作 */
    if (APP_VC_SetMuteStatus(g_atClientTab[indexNum].clientId, 0, muteStatus) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMUT_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SetModemLoopPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    VOS_UINT8  modemLoop;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    VOS_UINT8 numOfCalls;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];

    (void)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));
#endif

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    modemLoop = (VOS_UINT8)g_atParaList[0].paraValue;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 通话中不支持设置 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_TRUE) {
        result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);
        if (result != VOS_OK) {
            return AT_ERROR;
        }

        if (numOfCalls != 0) {
            return AT_ERROR;
        }

        /* 语音回环开启和关闭需要配对，否则直接返回OK */
        if (modemLoop == AT_IsModemLoopOn()) {
            return AT_OK;
        }
    }
#endif

    result = APP_VC_SetModemLoop(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, modemLoop);
    if (result == TAF_SUCCESS) {
#if (FEATURE_MBB_CUST == FEATURE_ON)
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_TRUE) {
        AT_WriteModemLoop(modemLoop);
    }
#endif
        return AT_OK; /* 此命令不需要等待C核回复，直接返回OK */
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 At_SetCmicPara(VOS_UINT8 indexNum)
{
    const VOS_UINT8 intraVolume[] = { AT_CMD_CMIC_LEV_1, AT_CMD_CMIC_LEV_2, AT_CMD_CMIC_LEV_3, AT_CMD_CMIC_LEV_4,
        AT_CMD_CMIC_LEV_5, AT_CMD_CMIC_LEV_6, AT_CMD_CMIC_LEV_7, AT_CMD_CMIC_LEV_8, AT_CMD_CMIC_LEV_9,
        AT_CMD_CMIC_LEV_10, AT_CMD_CMIC_LEV_11, AT_CMD_CMIC_LEV_12 };

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex != 1) || (g_atParaList[0].paraValue > sizeof(intraVolume) / sizeof(intraVolume[0]))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (APP_VC_SetUpVoiceVolume(g_atClientTab[indexNum].clientId, 0,
        intraVolume[g_atParaList[0].paraValue - 1]) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMIC_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetSpeakerMutePara(VOS_UINT8 indexNum)
{
    APP_VC_MuteStatusUint8     muteStatus;
    VOS_UINT8                  numOfCalls;
    TAFAGENT_CALL_InfoParam    callInfos[MN_CALL_MAX_NUM];
    VOS_UINT32                 result;

    (void)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue == 0) {
        muteStatus = APP_VC_MUTE_STATUS_UNMUTED;
    } else if (g_atParaList[0].paraValue == 1) {
        muteStatus = APP_VC_MUTE_STATUS_MUTED;
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 从同步API获取通话信息 */
    result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);
    if (result != VOS_OK) {
        return AT_ERROR;
    }

    if (numOfCalls != 0) {
        /* 只能在通话中设置SMUT */
        if (APP_VC_SetSpeakerMuteStatus(g_atClientTab[indexNum].clientId, 0, muteStatus) != VOS_OK) {
            return AT_ERROR;
        }
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SMUT_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetEchoSwitchPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 echoState;
    VOS_UINT32 ret;
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数值检查 */
    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    echoState = (VOS_UINT16)g_atParaList[0].paraValue;
    ret = APP_VC_SetEchoSwitch(g_atClientTab[indexNum].clientId, 0, echoState);
    if (ret != VOS_OK) {
        return AT_ERROR;
    } else {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECHOSWITCH_SET;
        return AT_WAIT_ASYNC_RETURN;
    }
}

VOS_UINT32 AT_SetPcmFrPara(VOS_UINT8 indexNum)
{
    APP_VC_PcmFrReq pcmFrInfo;
    VOS_UINT8 numOfCalls;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];
    VOS_UINT32 result;

    (void)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));
    (void)memset_s(&pcmFrInfo, sizeof(APP_VC_PcmFrReq), 0, sizeof(APP_VC_PcmFrReq));

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen != MN_CALL_PCM_FR_INPUT_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiString2HexSimple(pcmFrInfo.pcmFr, g_atParaList[0].para, MN_CALL_PCM_FR_INPUT_NUM) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);
    if (result != VOS_OK) {
        return AT_ERROR;
    }

    if (numOfCalls != 0) {
        return AT_ERROR;
    }

    if (APP_VC_SetPcmFr(g_atClientTab[indexNum].clientId, 0, pcmFrInfo) != VOS_OK) {
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PCMFR_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetEchoSuppressionPara(VOS_UINT8 indexNum)
{
    AppVcEchoexReq echoEx;
    VOS_UINT8 numOfCalls;
    VOS_UINT32 result;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];

    (void)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));
    (void)memset_s(&echoEx, sizeof(AppVcEchoexReq), 0, sizeof(AppVcEchoexReq));

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    if ((g_atParaIndex != ECHOEX_PARA_INDEX_NUM) || (g_atParaList[0].paraLen == 0) ||
        (g_atParaList[1].paraLen == 0) || (g_atParaList[2].paraLen == 0) ||
        (g_atParaList[3].paraLen == 0) || (g_atParaList[4].paraLen == 0)) {
        AT_ERR_LOG("AT_SetEchoSuppressionPara: para error");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    echoEx.echoEnable = (VOS_INT16)g_atParaList[0].paraValue;
    /* 安富算法NV参数范围-12到12，转换为AT输入参数范围0到24 */
    echoEx.echoInputGain = (VOS_INT16)(g_atParaList[1].paraValue - ECHOEX_MAP_VALUE);
    echoEx.echoLevelControl = (VOS_INT16)(g_atParaList[2].paraValue - ECHOEX_MAP_VALUE);
    echoEx.echoClearFilterButton = (VOS_INT16)(g_atParaList[3].paraValue);
    echoEx.echoVadInputLevel = (VOS_INT16)(g_atParaList[4].paraValue - ECHOEX_MAP_VALUE);

    result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);
    if (result != VOS_OK) {
        AT_ERR_LOG("TAF_AGENT_GetCallInfoReq failure");
        return AT_ERROR;
    }

    if (numOfCalls != 0) {
        AT_ERR_LOG("the number of calls is not 0");
        return AT_ERROR;
    }

    if (APP_VC_SetEchoSuppression(g_atClientTab[indexNum].clientId, 0, echoEx) != VOS_OK) {
        AT_ERR_LOG("At_SetEchoSuppressionPara failure");
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECHOEX_SET;

    return AT_WAIT_ASYNC_RETURN;
}

LOCAL VOS_UINT32 At_CheckCpcmPara(VOS_UINT8 indexNum, MBB_CONFIG_STRU *mbbCfg)
{
    VOS_UINT32 result;
    VOS_UINT32 index;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("At_CheckCpcmPara: cmdOptType Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != CPCM_PARA_INDEX_NUMER) {
        AT_ERR_LOG("At_CheckCpcmPara: para num Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <format>,<frame>,<offset>三个参数的值只支持设置为0 */
    if ((g_atParaList[1].paraValue != 0) || (g_atParaList[3].paraValue != 0) || (g_atParaList[4].paraValue != 0)) {
        AT_ERR_LOG("At_CheckCpcmPara: para value Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_MbbConfig, mbbCfg, sizeof(MBB_CONFIG_STRU));
    if (result != NV_OK) {
        AT_ERR_LOG("At_CheckCpcmPara: NV read fail!");
        return AT_ERROR;
    }

    /* 获取命令参数, 提取要设置的主从模式 */
    if (g_atParaList[0].paraValue == CPCM_MAIN_MODE) {
        mbbCfg->is_main = APP_VC_CPCM_MODE_MAIN;
    } else if (g_atParaList[0].paraValue == CPCM_SECONDRY_MODE) {
        mbbCfg->is_main = APP_VC_CPCM_MODE_SECONDRY;
    } else {
        AT_ERR_LOG("At_CheckCpcmPara: para mode ERROR!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取命令参数, 提取要设置的CODEC时钟 */
    for (index = 0; index < (sizeof(g_codecClockMode) / sizeof(CodecClockMode)); index++) {
        if (g_atParaList[2].paraValue == g_codecClockMode[index].clockMode) {
            mbbCfg->codec_clk = g_codecClockMode[index].codecClock;
            break;
        }
    }

    if (index >= (sizeof(g_codecClockMode) / sizeof(CodecClockMode))) {
        AT_ERR_LOG("At_CheckCpcmPara: para codec_clock ERROR!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetCpcmPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 callCount;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];
    VOS_UINT32 result;
    MBB_CONFIG_STRU mbbCfg;

    (void)memset_s(&callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));
    (void)memset_s(&mbbCfg, sizeof(mbbCfg), 0x00, sizeof(mbbCfg));

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    result = At_CheckCpcmPara(indexNum, &mbbCfg);
    if (result != AT_OK) {
        if (result == AT_CME_INCORRECT_PARAMETERS) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            return AT_ERROR;
        }
    }

    /* 从同步API获取通话信息 */
    result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &callCount, callInfos);
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_SetCpcmPara: get callinfo fail!");
        return AT_ERROR;
    }

    if (callCount == 0) {
        if (TAF_ACORE_NV_WRITE(MODEM_ID_0, en_NV_MbbConfig, (VOS_UINT8 *)&mbbCfg, sizeof(mbbCfg)) != NV_OK) {
            AT_ERR_LOG("AT_SetCpcmPara: NV write fail!");
            return AT_ERROR;
        }
        return AT_OK;
    } else {
        AT_ERR_LOG("AT_SetCpcmPara: call count != 0!");
        return AT_ERROR;
    }
}

#endif

