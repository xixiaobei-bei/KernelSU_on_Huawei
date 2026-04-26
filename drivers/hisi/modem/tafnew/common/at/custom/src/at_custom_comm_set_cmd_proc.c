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
#include "at_custom_comm_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"

#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "at_taf_agent_interface.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_COMM_SET_CMD_PROC_C

#define AT_COMMBOOSTER_MODULEID 1
#define AT_COMMBOOSTER_LEN 2
#define AT_COMMBOOSTER_PARAM 3
#define AT_COMMBOOSTER_PARA_NUM 4
#define AT_CHRALARMRLATCFG_MIN_PARA_NUM 1
#define AT_CHRALARMRLATCFG_MAX_PARA_NUM 2
#define AT_CHRALARMRLATCFG_ALARMID 0
#define AT_CHRALARMRLATCFG_ALARMID_DETAIL 1


VOS_UINT32 AT_CommBoosterParaCheck(VOS_VOID)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return VOS_ERR;
    }

    if (g_atParaIndex != AT_COMMBOOSTER_PARA_NUM) {
        return VOS_ERR;
    }

    if ((g_atParaList[AT_COMMBOOSTER_MODULEID].paraLen == 0) || (g_atParaList[AT_COMMBOOSTER_LEN].paraLen == 0) ||
        (g_atParaList[AT_COMMBOOSTER_PARAM].paraLen  == 0)) {
        return VOS_ERR;
    }

    /* g_atParaList[1]记录字节数，g_atParaList[2]是字符个数，一个字节占2个字符，这里做一下校验 */
    if ((g_atParaList[AT_COMMBOOSTER_LEN].paraValue != g_atParaList[AT_COMMBOOSTER_PARAM].paraLen / 2) ||
        (g_atParaList[AT_COMMBOOSTER_PARAM].paraLen % 2 != 0)) {
        return VOS_ERR;
    }

    /* 字符的个数大于500都是错误参数 */
    if (g_atParaList[AT_COMMBOOSTER_PARAM].paraLen > AT_CMD_COMM_BOOSTER_BS_MAX_LENGTH) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetCommBoosterPara(VOS_UINT8 indexNum)
{
    AT_MTA_CommBoosterSetReq   commBoosterSet;
    AT_MTA_CommBoosterQueryReq commBoosterQuery;
    VOS_UINT32 result;

    if (AT_CommBoosterParaCheck() != VOS_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraValue == 1) {
        (VOS_VOID)memset_s(&commBoosterSet, sizeof(commBoosterSet), 0x00, sizeof(commBoosterSet));
        commBoosterSet.pidIndex = (VOS_UINT16)g_atParaList[AT_COMMBOOSTER_MODULEID].paraValue;
        commBoosterSet.len      = g_atParaList[AT_COMMBOOSTER_LEN].paraValue;
        (VOS_VOID)memset_s(commBoosterSet.date, AT_CMD_COMM_BOOSTER_BS_MAX_LENGTH, 0x00, sizeof(commBoosterSet.date));

        if (At_AsciiString2HexSimple(commBoosterSet.date, g_atParaList[AT_COMMBOOSTER_PARAM].para,
                                     g_atParaList[AT_COMMBOOSTER_PARAM].paraLen) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_COMM_BOOSTER_SET_REQ,
                                        (VOS_UINT8 *)&commBoosterSet, sizeof(AT_MTA_CommBoosterSetReq),
                                        I0_UEPS_PID_MTA);
    } else {
        (VOS_VOID)memset_s(&commBoosterQuery, sizeof(commBoosterQuery), 0x00, sizeof(commBoosterQuery));
        commBoosterQuery.pidIndex = (VOS_UINT16)g_atParaList[AT_COMMBOOSTER_MODULEID].paraValue;
        commBoosterQuery.len      = g_atParaList[AT_COMMBOOSTER_LEN].paraValue;
        (VOS_VOID)memset_s(commBoosterQuery.date, AT_CMD_COMM_BOOSTER_BS_MAX_LENGTH, 0x00,
                 sizeof(commBoosterQuery.date));

        if (At_AsciiString2HexSimple(commBoosterQuery.date, g_atParaList[AT_COMMBOOSTER_PARAM].para,
                                     g_atParaList[AT_COMMBOOSTER_PARAM].paraLen) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_COMM_BOOSTER_QRY_REQ,
                                        (VOS_UINT8 *)&commBoosterQuery, sizeof(AT_MTA_CommBoosterQueryReq),
                                        I0_UEPS_PID_MTA);
    }

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    if (g_atParaList[0].paraValue == 1) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COMM_BOOSTER_SET;
    } else {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_COMM_BOOSTER_QRY;
    }

    return AT_WAIT_ASYNC_RETURN;
}


VOS_UINT32 AT_SetSARReduction(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    VOS_UINT16 uETestMode = 0;

    AT_ModemMtInfoCtx *mtInfoCtx = VOS_NULL_PTR;
    ModemIdUint16      modemId;
    VOS_UINT16         antState;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetSARReduction: Cmd Opt Type is wrong.");
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 读取NV项判定是否为测试模式，该NV项只有一份 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, en_NV_Item_RF_SAR_BACKOFF_TESTMODE, &uETestMode, sizeof(uETestMode));

    if (result != NV_OK) {
        AT_ERR_LOG("AT_SetSARReduction:Read NV fail");
        return AT_ERROR;
    }

    modemId = MODEM_ID_0;

    result = AT_GetModemIdFromClient(indexNum, &modemId);

    if (result != VOS_OK) {
        AT_ERR_LOG("AT_SetSARReduction:Get modem id fail");

        return AT_ERROR;
    }

    mtInfoCtx = AT_GetModemMtInfoCtxAddrFromModemId(modemId);

    result = TAF_AGENT_GetAntState(indexNum, &antState);
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_QrySARReduction:Get modem id fail");
        return AT_ERROR;
    }

#ifndef FEATURE_ANTEN_DETECT
    if ((antState == AT_ANT_CONDUCTION_MODE) && (uETestMode != VOS_TRUE))
#else
    if ((antState != AT_ANT_AIRWIRE_MODE) && (uETestMode != VOS_TRUE))
#endif
    {
        AT_ERR_LOG("AT_SetSARReduction: Test mode wrong");
        return AT_ERROR;
    }

    mtInfoCtx->sarReduction = g_atParaList[0].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SARREDUCTION_NTF,
                                    (VOS_UINT8 *)&mtInfoCtx->sarReduction, sizeof(VOS_UINT32), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetSARReduction: Send Msg Fail");

        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_SetSwverPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    AT_PR_LOGI("Rcv Msg");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        AT_WARN_LOG("At_SetSwverPara: ucCmdOptType error.");

        return AT_ERROR;
    }

    /* 发消息到C核获取充电状态和电池电量 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_SWVER_SET_REQ, VOS_NULL_PTR, 0, I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetSwverPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SWVER_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetChrAlarmRlatCnfInfoPara(VOS_UINT8 indexNum)
{
    AT_MTA_ChrAlarmRlatCfgSetReq chrAlarmRlatCfgSetReq;
    VOS_UINT32                   result = 0;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetChrAlarmRlatCnfInfoPara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多或者过少 */
    if ((g_atParaIndex != AT_CHRALARMRLATCFG_MIN_PARA_NUM) && (g_atParaIndex != AT_CHRALARMRLATCFG_MAX_PARA_NUM)) {
        AT_WARN_LOG("AT_SetChrAlarmRlatCnfInfoPara : Para Num is error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数1长度为0 */
    if (g_atParaList[AT_CHRALARMRLATCFG_ALARMID].paraLen == 0) {
        AT_WARN_LOG("AT_SetChrAlarmRlatCnfInfoPara : Para 1 length is 0.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数2长度为0 */
    if ((g_atParaIndex == AT_CHRALARMRLATCFG_MAX_PARA_NUM) &&
        (g_atParaList[AT_CHRALARMRLATCFG_ALARMID_DETAIL].paraLen == 0)) {
        AT_WARN_LOG("AT_SetChrAlarmRlatCnfInfoPara : Para 2 length is 0.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&chrAlarmRlatCfgSetReq, sizeof(chrAlarmRlatCfgSetReq), 0x00, sizeof(chrAlarmRlatCfgSetReq));

    chrAlarmRlatCfgSetReq.alarmId = g_atParaList[AT_CHRALARMRLATCFG_ALARMID].paraValue;

    if (g_atParaIndex == AT_CHRALARMRLATCFG_MIN_PARA_NUM) {
        chrAlarmRlatCfgSetReq.alarmOp = AT_MTA_CHR_ALARM_RLAT_OP_READ;
    } else {
        chrAlarmRlatCfgSetReq.alarmOp     = AT_MTA_CHR_ALARM_RLAT_OP_WRITE;
        chrAlarmRlatCfgSetReq.alarmDetail = g_atParaList[AT_CHRALARMRLATCFG_ALARMID_DETAIL].paraValue;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_CHRALARMRLAT_CFG_SET_REQ, (VOS_UINT8 *)&chrAlarmRlatCfgSetReq,
                                    (VOS_SIZE_T)sizeof(chrAlarmRlatCfgSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetChrAlarmRlatCnfInfoPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CHRALARMRLATCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 函数功能:  检查设置参数的有效性
 */
LOCAL VOS_UINT32 AT_CheckVoipKeySetCmdParameter(void)
{
    VOS_UINT16 maxLength;
    if (g_atDataLocked == VOS_TRUE) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : not unlock.");
        return AT_ERROR;
    }
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : not set cmd.");
        return AT_ERROR;
    }
    /* 第一个参数不存在 */
    if (g_atParaList[0].paraLen== 0) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : no first parameter.");
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 第二个参数不存在 */
    if (g_atParaList[1].paraLen== 0) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : no second parameter.");
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 参数过多 */
    if (g_atParaIndex != AT_APN_KEY_PARA_NUM) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : parameter number is not 2!");
        return AT_ERROR;
    }
    /* 检查首个参数的有效性 */
    if (g_atParaList[0].paraValue>= AT_MAX_APN_KEY_GROUP) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : first parameter too large!");
        return AT_ERROR;
    }
    /* 输入的字符不能超过95个 */
    maxLength = AT_MAX_APN_KEY_LENGTH - 1;
    if (g_atParaList[1].paraLen> maxLength) {
        AT_WARN_LOG("AT_CheckVoipKeySetCmdParameter : second parameter lenghth error!");
        return AT_ERROR;
    }
    return AT_OK;
}

/*
 * 函数功能: 用来加密保存装备协议的APN KEY使用C核提供的加密算法
 */
VOS_UINT32 AT_SetVoipApnKeyPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_ApnKeyList encrptPwd;
    VOS_INT32 ret;

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->voipApnKey == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 检查参数的有效性 */
    if (AT_CheckVoipKeySetCmdParameter() != AT_OK) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(&encrptPwd, sizeof(TAF_NVIM_ApnKeyList), 0, sizeof(TAF_NVIM_ApnKeyList));
    encrptPwd.useFlag = 1; /* 有效位置为1 */
    ret = memcpy_s(encrptPwd.apnKey, MAX_ENCRPT_APN_KEY_LENGTH, g_atParaList[1].para, g_atParaList[1].paraLen);
    if (ret != EOK) {
        AT_WARN_LOG("AT_SetVoipApnKeyPara : copy voip apn key err!");
        (VOS_VOID)memset_s(&encrptPwd, sizeof(TAF_NVIM_ApnKeyList), 0, sizeof(TAF_NVIM_ApnKeyList));
        return AT_ERROR;
    }
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_VOIP_APN_KEY, (VOS_UINT8 *)&encrptPwd, sizeof(TAF_NVIM_ApnKeyList)) != NV_OK) {
        AT_WARN_LOG("AT_SetVoipApnKeyPara : write NV voip apn key err!");
        (VOS_VOID)memset_s(&encrptPwd, sizeof(TAF_NVIM_ApnKeyList), 0, sizeof(TAF_NVIM_ApnKeyList));
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(&encrptPwd, sizeof(TAF_NVIM_ApnKeyList), 0, sizeof(TAF_NVIM_ApnKeyList));

    return AT_OK;
}
#endif

VOS_UINT32 AT_SetWifiGlobalMacPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    VOS_UINT8  e5GwMacAddr[AT_MAC_ADDR_LEN + 1]; /* MAC地址 */
    errno_t    stringRet;

    if (g_atParaIndex != 1) {
        AT_NORM_LOG("AT_SetApMacPara: the number of parameters is error.");
        return AT_ERROR;
    }

    if (g_atParaList[0].paraLen > AT_MAC_ADDR_LEN) {
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(e5GwMacAddr, sizeof(e5GwMacAddr), 0x00, sizeof(e5GwMacAddr));

    /* 写MAC地址参数到NV */
    stringRet = strncpy_s((VOS_CHAR *)e5GwMacAddr, sizeof(e5GwMacAddr), (VOS_CHAR *)g_atParaList[0].para,
                          g_atParaList[0].paraLen);
    TAF_STRCPY_CHK_RTN_VAL_CONTINUE(stringRet, sizeof(e5GwMacAddr), g_atParaList[0].paraLen);

    ret = TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_WIFI_MAC_ADDR, e5GwMacAddr, AT_MAC_ADDR_LEN);

    if (ret != NV_OK) {
        AT_WARN_LOG("AT_SetApMacPara: Fail to write nv.");
        return AT_ERROR;
    }

    return AT_OK;
}

