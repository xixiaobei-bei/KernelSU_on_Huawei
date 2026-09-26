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
#include "at_voice_hifi_qry_cmd_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "ccore_nv_stru_hifi.h"
#include "at_taf_agent_interface.h"
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_HIFI_QRY_CMD_PROC_C

VOS_UINT32 At_QryClvlPara(VOS_UINT8 indexNum)
{
    if (APP_VC_GetVoiceVolume(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLVL_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestClvlPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (1-12)", g_parseContext[indexNum].cmdElement->cmdName);


    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-5)", g_parseContext[indexNum].cmdElement->cmdName);
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}


VOS_UINT32 At_QryVMSETPara(VOS_UINT8 indexNum)
{
    /* 不是查询命令返回参数错误 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取当前语音设备模式：0 手持；1 手持免提；2 车载免提；3 耳机；4 蓝牙；5 PC语音模式 */
    if (APP_VC_GetVoiceMode(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VMSET_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_QryCmutPara(VOS_UINT8 indexNum)
{
    if (APP_VC_GetMuteStatus(g_atClientTab[indexNum].clientId, 0) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMUT_READ;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_TestCmutPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-1)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_TestVmsetPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length = 0;
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-6)", g_parseContext[indexNum].cmdElement->cmdName);


    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: (0-10)", g_parseContext[indexNum].cmdElement->cmdName);
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}


#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 AT_QrySpeakerMutePara(VOS_UINT8 indexNum)
{
    VOS_UINT8                  numOfCalls;
    TAFAGENT_CALL_InfoParam    callInfos[MN_CALL_MAX_NUM];
    VOS_UINT32                 result;

    (void)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));

    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 从同步API获取通话信息 */
    result = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);
    if (result != VOS_OK) {
        return AT_ERROR;
    }

    if (numOfCalls == 0) {
        /* 重新初始化静音状态 */
        if (APP_VC_UpdateSpeakerMuteStatus(g_atClientTab[indexNum].clientId, 0, 1) != VOS_OK) {
            return AT_ERROR;
        }

        /* 非通话过程中查询到的是默认值非静音状态 */
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        APP_VC_MUTE_STATUS_UNMUTED);
        return AT_OK;
    } else {
        /* 通话过程中取用户配置的静音状态 */
        if (APP_VC_GetSpeakerMuteStatus(g_atClientTab[indexNum].clientId, 0) != VOS_OK) {
            return AT_ERROR;
        }
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SMUT_READ;
        return AT_WAIT_ASYNC_RETURN;
    }
}

VOS_UINT32 At_QryCmicPara(VOS_UINT8 indexNum)
{
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    if (APP_VC_GetUpVoiceVolume(g_atClientTab[indexNum].clientId, 0) != VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMIC_READ;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QryEchoSuppressionPara(VOS_UINT8 indexNum)
{
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (APP_VC_GetEchoSuppression(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECHOEX_READ;
        return AT_WAIT_ASYNC_RETURN;
    }
    return AT_ERROR;
}

VOS_UINT32 AT_QryPcmFrPara(VOS_UINT8 indexNum)
{
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (APP_VC_GetPcmFr(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PCMFR_READ;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_TestPcmFrPara(VOS_UINT8 indexNum)
{
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 输出测试命令结果 */
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %u", g_parseContext[indexNum].cmdElement->cmdName,
        (MN_CALL_PCM_FR_NUM << 1));

    return AT_OK;
}

VOS_UINT32 AT_QryEchoSwitchPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 如果不是查询命令返回参数错误 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 获取当前回声消除开关状态 */
    ret = APP_VC_GetEchoSwitch(g_atClientTab[indexNum].clientId, 0);
    if (ret == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECHOSWITCH_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_QryCpcmPara(VOS_UINT8 indexNum)
{
    MBB_CONFIG_STRU mbbCfg;
    VOS_UINT8 index;

    (void)memset_s(&mbbCfg, sizeof(mbbCfg), 0x00, sizeof(mbbCfg));
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->pcmAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_MbbConfig,
        &(mbbCfg), sizeof(mbbCfg)) != NV_OK) {
        AT_ERR_LOG("AT_QryCpcmPara: NV read fail!");
        return AT_ERROR;
    }

    if (mbbCfg.is_main == APP_VC_CPCM_MODE_SECONDRY) {
        mbbCfg.is_main = CPCM_SECONDRY_MODE;
    } else if (mbbCfg.is_main == APP_VC_CPCM_MODE_MAIN) {
        mbbCfg.is_main = CPCM_MAIN_MODE;
    } else {
        AT_ERR_LOG("AT_QryCpcmPara: mode ERROR!");
        return AT_ERROR;
    }

    for (index = 0; index < (sizeof(g_codecClockMode) / sizeof(CodecClockMode)); index++) {
        if (mbbCfg.codec_clk == g_codecClockMode[index].codecClock) {
            break;
        }
    }

    if (index >= (sizeof(g_codecClockMode) / sizeof(CodecClockMode))) {
        AT_ERR_LOG("AT_QryCpcmPara: para codec_clock ERROR!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,0,%u,0,0", g_parseContext[indexNum].cmdElement->cmdName,
        mbbCfg.is_main, g_codecClockMode[index].clockMode);

    return AT_OK;
}

VOS_UINT32 AT_QryModemLoopPara(VOS_UINT8 indexNum)
{
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->telematicAudioFlag == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }
    /* 不是查询命令返回参数错误 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %u", g_parseContext[indexNum].cmdElement->cmdName, g_modemLoop);

    return AT_OK;
}
#endif

