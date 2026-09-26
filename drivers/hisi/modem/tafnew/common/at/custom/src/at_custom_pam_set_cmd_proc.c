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
#include "at_custom_pam_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "si_app_stk.h"
#include "at_input_proc.h"
#include "at_csimagent.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PAM_SET_CMD_PROC_C

#define AT_STGR_VALID_NUM 2
#define AT_CSTR_LEN 0
#define AT_CSTR_CMD_DATE 1
#define AT_CSEN_LEN 0
#define AT_DATA_MAX_LEN 256
#define AT_SCICHG_MAX_PARA_NUM 3
#define AT_SCICHG_MODEM0 0
#define AT_SCICHG_MODEM1 1
#define AT_SCICHG_MODEM2 2
#define AT_CSIM_LENGTH 0
#define AT_CSIM_COMMAND 1
#define AT_CGLA_PARA_NUM 3
#define AT_CGLA_LENGTH 1
#define AT_CGLA_COMMAND 2
#define AT_CGLA_SESSIONID 0
#define AT_CSIM_PARA_MAX_NUM 2
#define AT_SILENTPIN_PARA_NUM 3
#define AT_SILENTPIN_PARA_CRYTOPIN  0 /* SERDESTESTASYNC的第一个参数CRYTOPIN */
#define AT_SILENTPIN_PARA_PINIV     1 /* SERDESTESTASYNC的第二个参数PINIV */
#define AT_SILENTPIN_PARA_HMACVALUE 2 /* SERDESTESTASYNC的第三个参数HMACVALUE */
#define AT_MODEMSTATUS_PARA_NUM 2

/*
 * Description: ^STGR=<Len>,<data>
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCstrPara(TAF_UINT8 indexNum)
{
    VOS_UINT8  data[AT_DATA_MAX_LEN];
    VOS_UINT8  dataLen;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        return AT_ERROR;
    }
    if (g_atParaIndex != AT_STGR_VALID_NUM) { /* the para munber is too many or too few */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* the para1 and para2 is must be inputed */
    if ((g_atParaList[AT_CSTR_LEN].paraLen == 0) || (g_atParaList[AT_CSTR_CMD_DATE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* the value of para1 and the len of para2 must be equal */
    if (g_atParaList[AT_CSTR_LEN].paraValue != g_atParaList[AT_CSTR_CMD_DATE].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiString2HexSimple(data, g_atParaList[AT_CSTR_CMD_DATE].para,
        (VOS_UINT16)g_atParaList[AT_CSTR_LEN].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    dataLen = (TAF_UINT8)(g_atParaList[AT_CSTR_LEN].paraValue / 2);

    if (SI_STK_DataSendSimple(g_atClientTab[indexNum].clientId, 0, SI_STK_TRSEND, dataLen, data) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSTR_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

/*
 * Description: ^CSIN命令处理函数
 * History:
 *  1.Date: 2012-05-17
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCsinPara(TAF_UINT8 indexNum)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 非ANDROID系统不支持 */
    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        return AT_ERROR;
    }

    /* 获取最后一次收到的主动命令，不将命令类型传入 */
    if (SI_STK_GetSTKCommand(g_atClientTab[indexNum].clientId, 0, 0) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSEN_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

/*
 * Description: ^STGR=<Len>,<data>
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCsenPara(TAF_UINT8 indexNum)
{
    VOS_UINT8  data[AT_DATA_MAX_LEN];
    VOS_UINT8  dataLen;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        return AT_ERROR;
    }

    if (g_atParaIndex != AT_STGR_VALID_NUM) { /* the para munber is too many or too few */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* the para1 and para2 is must be inputed */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* the value of para1 and the len of para2 must be equal */
    if (g_atParaList[0].paraValue != g_atParaList[1].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiString2HexSimple(data, g_atParaList[1].para, (VOS_UINT16)g_atParaList[0].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    dataLen = (TAF_UINT8)(g_atParaList[AT_CSEN_LEN].paraValue / 2);

    if (SI_STK_DataSendSimple(g_atClientTab[indexNum].clientId, 0, SI_STK_ENVELOPESEND, dataLen, data) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSEN_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

/*
 * Description: ^CSMN
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCsmnPara(TAF_UINT8 indexNum)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;
#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    VOS_UINT8 tmpSystemAppConfig;
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_OFF)
    AT_SetSystemAppConfig((VOS_UINT16)indexNum, &tmpSystemAppConfig);
    systemAppConfig = &tmpSystemAppConfig;
#else
    systemAppConfig = AT_GetSystemAppConfigAddr();
#endif

    if (*systemAppConfig != SYSTEM_APP_ANDROID) {
        return AT_ERROR;
    }

    if (g_atParaIndex != 0) { /* the para munber is too many or too few */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (SI_STK_GetMainMenu(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSEN_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCstcPara(TAF_UINT8 indexNum)
{
    if (g_atParaIndex != 1) { /* the para munber is too many or too few */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (SI_STK_SetUpCallConfirm(g_atClientTab[indexNum].clientId, g_atParaList[0].paraValue) == AT_SUCCESS) {
        return AT_OK;
    }

    return AT_ERROR;
}

/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCcinPara(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * 如果 参数是配置为启动主动上报模式；
     * 返回OK，并切换当前用户为主动上报模式；
     * 如果 参数是配置为停止主动上报模式；
     * 返回OK，并切换当前用户为普通命令模式；
     */

    g_atClientTab[indexNum].indMode = (TAF_UINT8)g_atParaList[0].paraValue;
    return AT_OK;
}

TAF_UINT32 At_SetCardATRPara(TAF_UINT8 indexNum)
{
    if (SI_PIH_GetCardATRReq(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARD_ATR_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_FillAndSndCSIMAMsg(VOS_UINT16 clinetID, VOS_UINT32 modemStatus)
{
    AT_CSIMA_ResetStatusIndMsg *aTCSIMAIndMsg = VOS_NULL_PTR;
    ModemIdUint16               modemID;

    /* 调用接口获取Modem ID */
    if (AT_GetModemIdFromClient(clinetID, &modemID) != VOS_OK) {
        AT_ERR_LOG("AT_FillAndSndCSIMAMsg:ERROR: AT_GetModemIdFromClient Error");
        return VOS_ERR;
    }

    /* 申请内存  */
    /*lint -save -e516 */
    aTCSIMAIndMsg =
        (AT_CSIMA_ResetStatusIndMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT,
                                                   sizeof(AT_CSIMA_ResetStatusIndMsg) - VOS_MSG_HEAD_LENGTH);
    /*lint -restore */
    if (aTCSIMAIndMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_FillAndSndCSIMAMsg:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    /* 填写相关参数 */
    if (modemID == MODEM_ID_1) {
        TAF_CfgMsgHdr((MsgBlock *)aTCSIMAIndMsg, WUEPS_PID_AT, I1_WUEPS_PID_CSIMA,
                      sizeof(AT_CSIMA_ResetStatusIndMsg) - VOS_MSG_HEAD_LENGTH);
    } else {
        TAF_CfgMsgHdr((MsgBlock *)aTCSIMAIndMsg, WUEPS_PID_AT, I0_WUEPS_PID_CSIMA,
                      sizeof(AT_CSIMA_ResetStatusIndMsg) - VOS_MSG_HEAD_LENGTH);
    }

    aTCSIMAIndMsg->msgId          = AT_CSIMA_RESET_IND_MSG;
    aTCSIMAIndMsg->viaModemStatus = (CBP_MODEM_ResetStatusUint32)modemStatus;

    /* 调用VOS发送原语 */
    return TAF_TraceAndSendMsg(WUEPS_PID_AT, aTCSIMAIndMsg);
}

VOS_UINT32 AT_SetModemStatusPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    /* 参数不符合 */
    if (g_atParaIndex != AT_MODEMSTATUS_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 内容长度不符合 */
    if ((g_atParaList[0].paraLen != 1) || (g_atParaList[1].paraLen != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果是balong modem复位，不需要处理该命令 */
    if (g_atParaList[0].paraValue == 0) {
        return AT_OK;
    }

    rst = AT_FillAndSndCSIMAMsg(g_atClientTab[indexNum].clientId, g_atParaList[1].paraValue);

    if (rst == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
VOS_UINT32 AT_SetSpwordPara(VOS_UINT8 indexNum)
{
    errno_t memResult;
    VOS_UINT32 i;
    DRV_AGENT_SpwordSetReq spwordSetReq;

    /* 如果超过三次，则在系统重新启动前不再处理该命令，直接返回Error */
    if (g_spWordCtx.errTimes >= AT_SHELL_PWD_VERIFY_MAX_TIMES) {
        AT_WARN_LOG("AT_SetSpwordPara:WARNING: Verify Max Times!");
        return AT_ERROR;
    }

    /* 一旦AT^SPWORD命令校验成功，系统不再对用户以后输入的AT^SPWORD命令进行处理，直接返回ERROR */
    if (g_spWordCtx.shellPwdCheckFlag == VOS_TRUE) {
        AT_WARN_LOG("AT_SetSpwordPara:WARNING: Not need Verified!");
        return AT_ERROR;
    }

    /* 有且仅有一个参数，参数长度为8 */
    if ((g_atParaIndex > 1) || (g_atParaList[0].paraLen != AT_SHELL_PWD_LEN)) {
        AT_WARN_LOG("AT_SetSpwordPara:WARNING: Parameter error!");
        return AT_ERROR;
    }

    /* 参数只能为字符0-9 */
    for (i = 0; i < AT_SHELL_PWD_LEN; i++) {
        if ((g_atParaList[0].para[i] < '0') || (g_atParaList[0].para[i] > '9')) {
            AT_WARN_LOG("AT_SetSpwordPara:WARNING: PWD NOT 0-9!");
            return AT_ERROR;
        }
    }

    /* 密码校验需要在C核实现 */
    memResult = memcpy_s(spwordSetReq.shellPwd, (VOS_SIZE_T)sizeof(spwordSetReq.shellPwd),
                         (VOS_CHAR *)g_atParaList[0].para, AT_SHELL_PWD_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, (VOS_SIZE_T)sizeof(spwordSetReq.shellPwd), AT_SHELL_PWD_LEN);

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, DRV_AGENT_SPWORD_SET_REQ,
                               (VOS_UINT8 *)&spwordSetReq, sizeof(spwordSetReq),
                               I0_WUEPS_PID_DRV_AGENT) == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SPWORD_SET; /* 设置当前操作模式 */
        (VOS_VOID)memset_s(&spwordSetReq, sizeof(spwordSetReq), 0x00, sizeof(spwordSetReq));
        return AT_WAIT_ASYNC_RETURN; /* 等待异步事件返回 */
    } else {
        (VOS_VOID)memset_s(&spwordSetReq, sizeof(spwordSetReq), 0x00, sizeof(spwordSetReq));
        return AT_ERROR;
    }
}
#endif

VOS_UINT32 At_SetPassThrough(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_PIH_PassThroughSet(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        g_atParaList[0].paraValue);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetPassThrough: SI_PIH_PassThroughSet fail");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PASSTHROUGH_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
/*
 * 功能描述: (AT^SINGLEMODEMDUALSLOT)单MODEM，多卡槽场景设置卡槽与USIM任务对应关系
 */
VOS_UINT32 At_SetSingleModemDualSlot(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_PIH_SingleModemDualSlotSet(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        g_atParaList[0].paraValue);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetSingleModemDualSlot: SI_PIH_SingleModemDualSlotSet fail");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SINGLEMODEMDUALSLOT_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 At_SetSciChgPara(VOS_UINT8 indexNum)
{
#if (MULTI_MODEM_NUMBER != 1)
    VOS_UINT32 result;
#endif
    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_SCICHG_MAX_PARA_NUM) {
        return AT_TOO_MANY_PARA;
    }

    /* 参数检查 */
    if ((g_atParaList[AT_SCICHG_MODEM0].paraLen == 0) || (g_atParaList[AT_SCICHG_MODEM1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 单modem不支持卡槽切换 */
#if (MULTI_MODEM_NUMBER == 1)
    return AT_CME_OPERATION_NOT_ALLOWED;
#else

    /* 三卡形态第3个参数不能为空，其余形态默认为卡槽2 */
#if (MULTI_MODEM_NUMBER == 3)
    if (g_atParaList[AT_SCICHG_MODEM2].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 任意两个Modem不能同时配置为同一卡槽 */
    if ((g_atParaList[AT_SCICHG_MODEM0].paraValue == g_atParaList[AT_SCICHG_MODEM1].paraValue) ||
        (g_atParaList[AT_SCICHG_MODEM0].paraValue == g_atParaList[AT_SCICHG_MODEM2].paraValue) ||
        (g_atParaList[AT_SCICHG_MODEM1].paraValue == g_atParaList[AT_SCICHG_MODEM2].paraValue)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#else
    g_atParaList[AT_SCICHG_MODEM2].paraValue = SI_PIH_CARD_SLOT_2;

    /* 任意两个Modem不能同时配置为同一卡槽 */
    if (g_atParaList[AT_SCICHG_MODEM0].paraValue == g_atParaList[AT_SCICHG_MODEM1].paraValue) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#endif

    result = SI_PIH_SciCfgSet(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                              g_atParaList[AT_SCICHG_MODEM0].paraValue, g_atParaList[AT_SCICHG_MODEM1].paraValue,
                              g_atParaList[AT_SCICHG_MODEM2].paraValue);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetSciChgPara: SI_PIH_HvSstSet fail.");
        return AT_CME_PHONE_FAILURE;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SCICHG_SET;

    return AT_WAIT_ASYNC_RETURN;
#endif
}

#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))
VOS_UINT32 At_SetSilentPin(VOS_UINT8 indexNum)
{
    errno_t          memResult;
    VOS_UINT32       result;
    VOS_UINT16       length;
    SI_PIH_CryptoPin cryptoPin;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("At_SetSilentPinInfo: CmdOptType fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  参数不为3 */
    if (g_atParaIndex != AT_SILENTPIN_PARA_NUM) {
        AT_WARN_LOG1("At_SetSilentPinInfo: para num  %d.", g_atParaIndex);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_SILENTPIN_PARA_CRYTOPIN].paraLen != (DRV_AGENT_PIN_CRYPTO_DATA_LEN * 2)) ||
        (g_atParaList[AT_SILENTPIN_PARA_PINIV].paraLen != (DRV_AGENT_PIN_CRYPTO_IV_LEN * 2)) ||
        (g_atParaList[AT_SILENTPIN_PARA_HMACVALUE].paraLen != (DRV_AGENT_HMAC_DATA_LEN * 2))) {
        AT_WARN_LOG2("At_SetSilentPinInfo: 0 %d %d.", g_atParaList[AT_SILENTPIN_PARA_CRYTOPIN].paraLen,
                     g_atParaList[AT_SILENTPIN_PARA_PINIV].paraLen);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&cryptoPin, sizeof(cryptoPin), 0, sizeof(cryptoPin));

    /* 将密文PIN字符串参数转换为码流 */
    length = g_atParaList[AT_SILENTPIN_PARA_CRYTOPIN].paraLen;
    result = At_AsciiNum2HexString(g_atParaList[AT_SILENTPIN_PARA_CRYTOPIN].para, &length);
    if ((result != AT_SUCCESS) || (length != DRV_AGENT_PIN_CRYPTO_DATA_LEN)) {
        AT_WARN_LOG1("At_SetSilentPinInfo: Encpin fail %d.", length);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    memResult = memcpy_s(cryptoPin.cryptoPin, DRV_AGENT_PIN_CRYPTO_DATA_LEN,
                         g_atParaList[AT_SILENTPIN_PARA_CRYTOPIN].para, DRV_AGENT_PIN_CRYPTO_DATA_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, DRV_AGENT_PIN_CRYPTO_DATA_LEN, DRV_AGENT_PIN_CRYPTO_DATA_LEN);

    /* 将IV字符串参数转换为码流 */
    length = g_atParaList[AT_SILENTPIN_PARA_PINIV].paraLen;
    result = At_AsciiNum2HexString(g_atParaList[AT_SILENTPIN_PARA_PINIV].para, &length);
    if ((result != AT_SUCCESS) || (length != DRV_AGENT_PIN_CRYPTO_IV_LEN)) {
        AT_WARN_LOG1("At_SetSilentPinInfo: IV Len fail %d.", length);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    memResult = memcpy_s(cryptoPin.pinIv, DRV_AGENT_PIN_CRYPTO_IV_LEN, g_atParaList[AT_SILENTPIN_PARA_PINIV].para,
                         DRV_AGENT_PIN_CRYPTO_IV_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, DRV_AGENT_PIN_CRYPTO_IV_LEN, DRV_AGENT_PIN_CRYPTO_IV_LEN);

    /* 将HMAC字符串参数转换为码流 */
    length = g_atParaList[AT_SILENTPIN_PARA_HMACVALUE].paraLen;
    result = At_AsciiNum2HexString(g_atParaList[AT_SILENTPIN_PARA_HMACVALUE].para, &length);
    if ((result != AT_SUCCESS) || (length != DRV_AGENT_HMAC_DATA_LEN)) {
        AT_WARN_LOG1("At_SetSilentPinInfo: hmac Len fail %d.", length);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    memResult = memcpy_s(cryptoPin.hmacValue, DRV_AGENT_HMAC_DATA_LEN,
                         g_atParaList[AT_SILENTPIN_PARA_HMACVALUE].para, DRV_AGENT_HMAC_DATA_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, DRV_AGENT_HMAC_DATA_LEN, DRV_AGENT_HMAC_DATA_LEN);

    result = SI_PIH_SetSilentPinReq(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &cryptoPin);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetSilentPinInfo: SI_PIH_SetSilentPinReq fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SILENTPIN_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_VOID At_ClearPIN(VOS_UINT8 *mem, VOS_UINT8 len)
{
    VOS_UINT32 i;

    for (i = 0; i < len; i++) {
        mem[i] = 0XFF;
    }
}

VOS_UINT32 At_SetSilentPinInfo(VOS_UINT8 indexNum)
{
    VOS_UINT8  pin[TAF_PH_PINCODELENMAX + 1] = {0};
    VOS_UINT32 result;
    errno_t    memResult;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为1 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if ((g_atParaList[0].paraLen > TAF_PH_PINCODELENMAX) || (g_atParaList[0].paraLen < TAF_PH_PINCODELENMIN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(pin, TAF_PH_PINCODELENMAX, 0xFF, TAF_PH_PINCODELENMAX);

    memResult = memcpy_s(pin, TAF_PH_PINCODELENMAX, g_atParaList[0].para, g_atParaList[0].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, TAF_PH_PINCODELENMAX, g_atParaList[0].paraLen);

    result = SI_PIH_GetSilentPinInfoReq(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, pin,
                                        TAF_PH_PINCODELENMAX);
    /* 敏感信息置0xFF */
    At_ClearPIN(pin, TAF_PH_PINCODELENMAX);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetSilentPinInfo: SI_PIH_SetSilentPinReq fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SILENTPININFO_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

TAF_UINT32 At_SetPrivateCglaPara(TAF_UINT8 indexNum)
{
    SI_PIH_CglaCommand cglaCmd;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_CGLA_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <length>需要为2的整数倍 */
    if ((g_atParaList[AT_CGLA_LENGTH].paraValue % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不为2的整数倍 */
    if ((g_atParaList[AT_CGLA_COMMAND].paraLen % 2) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CGLA_COMMAND].para,
                              &g_atParaList[AT_CGLA_COMMAND].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetCglaCmdPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* length字段是实际命令长度的2倍 */
    if (g_atParaList[AT_CGLA_LENGTH].paraValue !=(TAF_UINT32)(g_atParaList[AT_CGLA_COMMAND].paraLen * AT_CONST_NUM_2)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    cglaCmd.sessionID = g_atParaList[AT_CGLA_SESSIONID].paraValue;
    cglaCmd.len       = g_atParaList[AT_CGLA_COMMAND].paraLen;
    cglaCmd.command   = g_atParaList[AT_CGLA_COMMAND].para;

    /* 执行命令操作 */
    if (SI_PIH_PrivateCglaSetReq(g_atClientTab[indexNum].clientId, 0, &cglaCmd) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PRIVATECGLA_REQ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

/*
 * Description: ^STGR=<cmdnum>,<cmdtype>,<result>,<data>
 * History:
 *  1.Date: 2010-02-11
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetImsichgPara(TAF_UINT8 indexNum)
{
    if (SI_STKIsDualImsiSupport() == VOS_FALSE) {
        return AT_CMD_NOT_SUPPORT;
    }

    if (g_atParaIndex != 1) { /* the para munber is too many or too few */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) { /* the para1 must be inputed */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (SI_STKDualIMSIChangeReq(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSICHG_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
/*
 * 功能描述: 设置有卡模式与无卡模式
 * 修改历史:
 *  1.日    期: 2020年5月21日
 *    修改内容: 新生成函数
 */
VOS_UINT32 AT_SetNoCard(VOS_UINT8 index)
{
    VOS_UINT32 result;
    MN_CLIENT_OperationId clientOperationId;

    /* 参数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发消息通知MMA模块修改NOCARD状态 */
    clientOperationId.clientId = g_atClientTab[index].clientId;
    clientOperationId.opId = 0;
    clientOperationId.reserve = 0;
    result = MN_FillAndSndAppReqMsg(&clientOperationId, TAF_MSG_MMA_NOCARD_SET_REQ, &(g_atParaList[0].paraValue),
        sizeof(g_atParaList[0].paraValue), I0_WUEPS_PID_MMA);
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetNoCard:Set MMA Notify fail!");
        return AT_ERROR;
    }

    /* 发消息通知USIMM模块修改NOCARD状态 */
    result = SI_PIH_SendNoCardReq(g_atClientTab[index].clientId, g_atClientTab[index].opId, g_atParaList[0].paraValue);
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetNoCard:Set USIMM Notify fail!");
        return AT_ERROR;
    }

    g_atClientTab[index].cmdCurrentOpt = AT_CMD_NOCARD_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif
#endif

TAF_UINT32 At_SetCsimPara(TAF_UINT8 indexNum)
{
    SI_PIH_CsimCommand command;
    errno_t            memResult;

    /* 参数检查 */
    if ((g_atParaList[AT_CSIM_LENGTH].paraLen == 0) || (g_atParaList[AT_CSIM_COMMAND].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CSIM_PARA_MAX_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&command, sizeof(command), 0x00, sizeof(command));

    if ((g_atParaList[AT_CSIM_LENGTH].paraValue % AT_DOUBLE_LENGTH) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_CSIM_COMMAND].paraLen % AT_DOUBLE_LENGTH) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(g_atParaList[AT_CSIM_COMMAND].para,
                              &g_atParaList[AT_CSIM_COMMAND].paraLen) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_CSIM_LENGTH].paraValue !=
        (TAF_UINT32)(g_atParaList[AT_CSIM_COMMAND].paraLen * AT_DOUBLE_LENGTH)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<length> */
    command.len = g_atParaList[AT_CSIM_COMMAND].paraLen;

    /* 设置<command> */
    memResult = memcpy_s((TAF_VOID *)command.command, sizeof(command.command),
                         (TAF_VOID *)g_atParaList[AT_CSIM_COMMAND].para, (VOS_UINT16)command.len);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(command.command), (VOS_UINT16)command.len);

    /* 执行命令操作 */
    if (SI_PIH_GenericAccessReq(g_atClientTab[indexNum].clientId, 0, &command) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSIM_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetMemInfoPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;

    /* 参数检查 */
    if ((g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) || (g_atParaIndex > 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发消息到C核获取 MEMINFO 信息, g_atParaList[0].ulParaValue 中为查询的类型 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 DRV_AGENT_MEMINFO_QRY_REQ, (VOS_UINT8 *)&(g_atParaList[0].paraValue),
                                 sizeof(g_atParaList[0].paraValue), I0_WUEPS_PID_DRV_AGENT);
    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryMemInfoPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MEMINFO_READ;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetApSimStPara(VOS_UINT8 indexNum)
{
    VOS_UINT32              result;
    DRV_AGENT_ApSimstSetReq apSimStSetReq;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&apSimStSetReq, sizeof(apSimStSetReq), 0x00, sizeof(DRV_AGENT_ApSimstSetReq));

    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_ERROR;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_ERROR;
    }

    /* 参数设置为1时，目前不支持，直接返回OK */
    if (g_atParaList[0].paraValue == 1) {
        apSimStSetReq.usimState = DRV_AGENT_USIM_OPERATE_ACT;
    } else {
        apSimStSetReq.usimState = DRV_AGENT_USIM_OPERATE_DEACT;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    DRV_AGENT_AP_SIMST_SET_REQ, (VOS_UINT8 *)&apSimStSetReq,
                                    sizeof(DRV_AGENT_ApSimstSetReq), I0_WUEPS_PID_DRV_AGENT);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetApSimStPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APSIMST_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))
VOS_UINT32 AT_SetCardAppAidPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;
    SI_PIH_CardInitAppAidCfg command;
    VOS_UINT8 aid[AT_PARA_MAX_LEN + 1];
    VOS_UINT16 aidLength;

    (VOS_VOID)memset_s(&command, sizeof(SI_PIH_CardInitAppAidCfg), 0, sizeof(SI_PIH_CardInitAppAidCfg));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != AT_CONST_NUM_4) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (memcpy_s(aid, sizeof(aid), g_atParaList[AT_COMMAND_PARA_INDEX_3].para,
        g_atParaList[AT_COMMAND_PARA_INDEX_3].paraLen) != EOK) {
        AT_ERR_LOG("AT_SetCardAppAidPara: memcpy_s fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    aidLength = g_atParaList[AT_COMMAND_PARA_INDEX_3].paraLen;

    if (((aidLength % AT_ASCII2STRING_BASE) != 0) ||
        ((aidLength / AT_ASCII2STRING_BASE) != g_atParaList[AT_COMMAND_PARA_INDEX_2].paraValue)) {
        AT_ERR_LOG("AT_SetCardAppAidPara: aid length not match.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_AsciiNum2HexString(aid, &aidLength) == AT_FAILURE) {
        AT_ERR_LOG("AT_SetCardAppAidPara: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    command.enable = g_atParaList[AT_COMMAND_PARA_INDEX_0].paraValue;
    command.appType = g_atParaList[AT_COMMAND_PARA_INDEX_1].paraValue;
    command.aidLen = g_atParaList[AT_COMMAND_PARA_INDEX_2].paraValue;
    if (memcpy_s(command.aid, sizeof(command.aid), aid, aidLength) != EOK) {
        AT_ERR_LOG("AT_CardAppAidSetPara: memcpy_s fail");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = SI_PIH_CardAppAidSet(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &command);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetCardAppAidPara: SI_PIH_CardAppAidSet fail.");

        return AT_CME_PHONE_FAILURE;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CARDAPPAID_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif


