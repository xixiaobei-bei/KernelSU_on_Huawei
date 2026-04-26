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
#include "at_device_pam_set_cmd_proc.h"

#include "at_cmd_proc.h"
#include "securec.h"
#include "taf_type_def.h"
#include "taf_drv_agent.h"
#include "at_mta_interface.h"
#include "osm.h"
#include "at_check_func.h"
#include "at_init.h"
#include "nv_stru_gucnas.h"
#include "at_device_cmd.h"
#include "at_mdrv_interface.h"
#include "at_device_comm.h"
#include "cbt_scm_soft_decode.h"
#include "at_mt_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_PAM_SET_CMD_PROC_C

#define AT_ESIMSWITCH_PARA_NUM 2
#define AT_SIMLOCKUNLOCK_PARA_NUM 2
#define AT_SIMLOCKUNLOCK_CAT 0
#define AT_SIMLOCKUNLOCK_PWD 1
#define AT_SIMLOCKDATAREADEX_VALID_PARA_NUM 2

VOS_UINT32 AT_SetIdentifyStartPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    VOS_UINT8  otaFlag = VOS_FALSE;

    AT_PR_LOGI("Rcv Msg");

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    if (VOS_StrCmp((VOS_CHAR *)g_atParseCmd.cmdName.cmdName, "^IDENTIFYOTASTART") == 0) {
        otaFlag = VOS_TRUE;
    }

    /* 发送跨核消息到C核, 发起鉴权握手 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_IDENTIFYSTART_SET_REQ, &otaFlag, sizeof(VOS_UINT8),
                                    I0_WUEPS_PID_DRV_AGENT);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetIdentifyStartPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IDENTIFYSTART_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetHukPara(VOS_UINT8 indexNum)
{
    errno_t             memResult;
    VOS_UINT32          result;
    VOS_UINT16          length;
    DRV_AGENT_HukSetReq hukSetReq;

    AT_PR_LOGI("Rcv Msg");

    /* 局部变量初始化 */
    result = VOS_NULL;
    length = g_atParaList[0].paraLen;
    (VOS_VOID)memset_s(&hukSetReq, sizeof(hukSetReq), 0x00, sizeof(DRV_AGENT_HukSetReq));

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    /* 检查码流参数长度 */
    if (length != AT_HUK_PARA_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串参数转换为码流 */
    result = At_AsciiNum2HexString(g_atParaList[0].para, &length);
    if ((result != AT_SUCCESS) || (length != DRV_AGENT_HUK_LEN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    memResult = memcpy_s(hukSetReq.huk, sizeof(hukSetReq.huk), g_atParaList[0].para, DRV_AGENT_HUK_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(hukSetReq.huk), DRV_AGENT_HUK_LEN);

    /* 转换成功, 发送跨核消息到C核, 设置HUK */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_HUK_SET_REQ, (VOS_UINT8 *)&hukSetReq, sizeof(DRV_AGENT_HukSetReq),
                                    I0_WUEPS_PID_DRV_AGENT);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetHukPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    AT_PR_LOGI("Snd Msg");

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HUK_SET;
    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_PHONE_SC == FEATURE_ON)
VOS_UINT32 AT_SetSimlockUnlockPara(VOS_UINT8 indexNum)
{
    VOS_UINT32              result;
    AT_MTA_SimlockunlockReq simlockUnlockSetReq;

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParaIndex != AT_SIMLOCKUNLOCK_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 解析锁网锁卡解锁参数 */
    if ((g_atParaList[AT_SIMLOCKUNLOCK_CAT].paraLen == 0) ||
        (g_atParaList[AT_SIMLOCKUNLOCK_PWD].paraLen != AT_SIMLOCKUNLOCK_PWD_PARA_LEN) ||
        (g_atParaList[AT_SIMLOCKUNLOCK_CAT].paraValue > AT_MTA_PERS_CATEGORY_SERVICE_PROVIDER)) {
        (VOS_VOID)memset_s(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para),
            0x00, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para));
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00, sizeof(AT_MTA_SimlockunlockReq));
    simlockUnlockSetReq.category = (AT_MTA_PersCategoryUint8)g_atParaList[AT_SIMLOCKUNLOCK_CAT].paraValue;

    /* 密码的合法性检查，密码为16个“0”-“9”ASCII字符,密码由ASCII码转换为十进制数字 */
    result = At_AsciiNum2Num(simlockUnlockSetReq.password, g_atParaList[AT_SIMLOCKUNLOCK_PWD].para,
                             g_atParaList[AT_SIMLOCKUNLOCK_PWD].paraLen);
    if (result != AT_SUCCESS) {
        (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00, sizeof(AT_MTA_SimlockunlockReq));
        (VOS_VOID)memset_s(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para),
            0x00, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para));
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送跨核消息到C核, 解锁锁网锁卡 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_SIMLOCKUNLOCK_SET_REQ, (VOS_UINT8 *)&simlockUnlockSetReq,
                                    sizeof(AT_MTA_SimlockunlockReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00, sizeof(AT_MTA_SimlockunlockReq));
        (VOS_VOID)memset_s(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para),
            0x00, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para));
        AT_WARN_LOG("AT_SetSimlockUnlockPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIMLOCKUNLOCK_SET;
    (VOS_VOID)memset_s(&simlockUnlockSetReq, sizeof(simlockUnlockSetReq), 0x00, sizeof(AT_MTA_SimlockunlockReq));
    (VOS_VOID)memset_s(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para, sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para), 0x00,
             sizeof(g_atParaList[AT_SIMLOCKUNLOCK_PWD].para));
    return AT_WAIT_ASYNC_RETURN;
}
#endif

TAF_UINT32 At_SetAt2OmPara(TAF_UINT8 indexNum)
{
    CBTCPM_RCV_FUNC     cbtRcvFunc = VOS_NULL_PTR;
    DMS_PortIdUint16    portId;

    portId = AT_GetDmsPortIdByClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 该命令无输入参数 */
    if (g_atParaIndex != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ucIndex所对应的AT通道不是AT/OM通道 */
    if (
#if (FEATURE_AT_HSUART == FEATURE_ON)
        (portId != DMS_PORT_HSUART) &&
#endif
        (portId != DMS_PORT_PCUI) && (portId != DMS_PORT_UART) &&
        (portId != DMS_PORT_CTRL) && (portId != DMS_PORT_PCUI2)) {
        return AT_ERROR;
    }

    /* 检查是否已有处于OM模式的AT通道 */
    if (DMS_PORT_IsExistCbtPort() == VOS_TRUE) {
        return AT_ERROR;
    }


    DMS_PORT_SwitchToOmDataMode(portId, cbtRcvFunc);

    return AT_OK;
}

#if (FEATURE_PHONE_SC == FEATURE_ON)
VOS_UINT32 AT_SimLockDataReadExPara(VOS_UINT8 indexNum)
{
    DRV_AGENT_SimlockdatareadexReadReq simLockDataReadExReq;
    VOS_UINT32                         result;

    AT_PR_LOGI("enter");

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&simLockDataReadExReq, sizeof(simLockDataReadExReq), 0x00, sizeof(simLockDataReadExReq));

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        AT_WARN_LOG("AT_SimLockDataReadExPara: It Is not Ap Port.");
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SimLockDataReadExPara: ucCmdOptType is not AT_CMD_OPT_SET_PARA_CMD.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数错误 */
    if (g_atParaIndex != AT_SIMLOCKDATAREADEX_VALID_PARA_NUM) {
        AT_WARN_LOG("AT_SimLockDataReadExPara: g_atParaIndex ERR.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数的长度不能为0 */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SimLockDataReadExPara: g_atParaList[0].paraLen err.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    simLockDataReadExReq.layer    = (VOS_UINT8)g_atParaList[0].paraValue;
    simLockDataReadExReq.indexNum = (VOS_UINT8)g_atParaList[1].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_SIMLOCKDATAREADEX_READ_REQ, (VOS_UINT8 *)&simLockDataReadExReq,
                                    (VOS_UINT32)sizeof(simLockDataReadExReq), I0_WUEPS_PID_DRV_AGENT);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SimLockDataReadExPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIMLOCKDATAREADEX_READ_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 At_SetEsimSwitchPara(VOS_UINT8 indexNum)
{
    VOS_UINT32           result;
    SI_PIH_EsimSwitchSet esimSwitchSet = {0};

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_ESIMSWITCH_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    esimSwitchSet.slot     = (VOS_UINT8)g_atParaList[0].paraValue;
    esimSwitchSet.cardType = (VOS_UINT8)g_atParaList[1].paraValue;

    result = SI_PIH_EsimSwitchSetFunc(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &esimSwitchSet);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetEsimSwitchPara: Set Esim Switch fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ESIMSWITCH_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SetEsimCleanProfile(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_EMAT_EsimCleanProfile(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetEsimCleanProfile: Set Esim Clean fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ESIMCLEAN_SET;

    return AT_WAIT_ASYNC_RETURN;
}



