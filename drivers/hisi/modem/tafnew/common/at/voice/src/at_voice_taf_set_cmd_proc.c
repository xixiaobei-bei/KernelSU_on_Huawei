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
#include "at_voice_taf_set_cmd_proc.h"
#include "securec.h"
#include "nv_stru_gucnas.h"
#include "at_ctx.h"
#include "at_type_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "at_input_proc.h"
#include "at_data_proc.h"
#include "at_check_func.h"
#include "at_voice_comm.h"
#include "at_mbb_cmd.h"
#include "at_msg_print.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_TAF_SET_CMD_PROC_C

#define AT_CBST_MAX_PARA_NUM 3
#define AT_CBST_CONN_ELEM 2
#define AT_DTMF_MAX_PARA_NUM 4
#define AT_DTMF_KEY 1
#define AT_DTMF_ON_LENGTH 2
#define AT_DTMF_CALL_ID 0
#define AT_DTMF_OFF_LENGTH 3
#define AT_DATA_DIAL_LEN 3
#define AT_D_DIAL_STRING 0
#define AT_D_BY_MEM_N_POSITION 1
#define AT_D_BY_NAME 2
#define AT_D_BY_N 2
#define AT_D_PARA_INDEX_3 3
#define AT_D_PARA_INDEX_4 4
#define AT_D_MEM_N_NUM 3
#define AT_D_L2P 2
#define AT_D_MEM_TYPE 2
#define AT_D_CID 4
#define AT_STAR_NUM1 1
#define AT_STAR_NUM2 2
#define AT_STAR_NUM3 3
#define AT_STAR_NUM4 4
#define AT_D_BY_DIAL_STRING 1
#define AT_D_PARA_INDEX_2 2
#define AT_APDS_DIAL_STRING 0
#define AT_APDS_SUB_STRING 1
#define AT_APDS_I 2
#define AT_APDS_G 3
#define AT_APDS_CALL_TYPE 4
#define AT_APDS_CALLDOMAIN 5
#define AT_APDS_SRV_TYPE 6
#define AT_APDS_RTTFLG 7
#define AT_APDS_CALLPULLDIALOGID 8
#define AT_APDS_ENCRYPTFLAG 9
#define AT_APDS_MAX_PARA_NUM 10
#define AT_CACMIMS_TOTAL_CALLNUM 0
#define AT_CACMIMS_CALL_ADDRESS 2
#define AT_CACMIMS_IS_ECON 1
#define AT_ECONFDIAL_DIAL_COUNT 0
#define AT_ECONFDIAL_DIAL_I 2
#define AT_ECONFDIAL_CALL_TYPE 3
#define AT_ECONFDIAL_CALLDOMAIN 4
#define AT_ECONFDIAL_ISECONF 5
#define AT_ECONFDIAL_FIRST_SIX_PARA_NUM 6
#define AT_CALL_SRV_CLASS_VOICE 1
#define AT_CALL_SRV_CLASS_VIDEO 2
#define AT_CALL_SRV_CLASS_SET_NW 3
#define AT_CCWAI_PARA_NUM 2
#define AT_CCWAI_MODE 0
#define AT_CCWAI_SERVICE_CLASS 1
#define AT_REJCALL_PARA_NUM 2
#define AT_ECLSTART_PARA_MAX_NUM 4
#define AT_ECLSTART_ACTIVATION_TYPE 0
#define AT_ECLSTART_TYPE_OF_CALL 1
#define AT_ECLSTART_DIAL_NUM 2
#define AT_ECLSTART_OPRT_MADE 3
#define AT_ECLCFG_PARA_MAX_NUM 3
#define AT_ECLCFG_MODE 0
#define AT_ECLCFG_VOC_CONFIG 1
#define AT_ECLCFG_REDIAL_CONFIG 2
#define AT_ECLMSD_ECALL_MSD 0
#define AT_ECLMODE_ECALLFORCE_MODE 0
#define AT_ECLMODE_ECALLFORCE_MODE_MAX_VALUE 2

TAF_UINT32 At_SetS0Para(TAF_UINT8 indexNum)
{
    TAF_CCA_TelePara timeInfo;
    AT_ModemCcCtx   *ccCtx = VOS_NULL_PTR;
    ModemIdUint16    modemId = MODEM_ID_0;
    VOS_UINT32       rslt;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 删除通道数传模式标识存在直接返回OK的处理 */

    rslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("At_SetS0Para: Get modem id fail.");
        return AT_ERROR;
    }

    ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);

    ccCtx->s0TimeInfo.s0TimerLen = (TAF_UINT8)g_atParaList[0].paraValue;
    ccCtx->s0TimeInfo.clientId   = g_atClientTab[indexNum].clientId;

    (VOS_VOID)memset_s(&timeInfo, sizeof(timeInfo), 0x00, sizeof(timeInfo));

    if (TAF_ACORE_NV_READ(modemId, NV_ITEM_CCA_TELE_PARA, &timeInfo, sizeof(TAF_CCA_TelePara)) == NV_OK) {
        if (timeInfo.s0TimerLen == ccCtx->s0TimeInfo.s0TimerLen) {
            AT_INFO_LOG("At_SetS0Para():The content to write is same as NV's");
            return AT_OK;
        }
    }
    timeInfo.s0TimerLen = ccCtx->s0TimeInfo.s0TimerLen;
    if (TAF_ACORE_NV_WRITE(modemId, NV_ITEM_CCA_TELE_PARA, (VOS_UINT8 *)&timeInfo, sizeof(TAF_CCA_TelePara)) != NV_OK) {
        return AT_ERROR;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCmodPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    ssCtx->cModType = (TAF_UINT8)g_atParaList[0].paraValue;

    return AT_OK;
}

TAF_UINT32 At_SetCbstPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CBST_MAX_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen != 0) {
        ssCtx->cbstDataCfg.speed = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        ssCtx->cbstDataCfg.speed = MN_CALL_CSD_SPD_64K_MULTI;
    }
    if (g_atParaList[1].paraLen != 0) {
        ssCtx->cbstDataCfg.name = (TAF_UINT8)g_atParaList[1].paraValue;
    } else {
        ssCtx->cbstDataCfg.name = MN_CALL_CSD_NAME_SYNC_UDI;
    }
    if (g_atParaList[AT_CBST_CONN_ELEM].paraLen != 0) {
        ssCtx->cbstDataCfg.connElem = (TAF_UINT8)g_atParaList[AT_CBST_CONN_ELEM].paraValue;
    } else {
        ssCtx->cbstDataCfg.connElem = MN_CALL_CSD_CE_T;
    }

    return AT_OK;
}

VOS_UINT32 At_SetClccPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                  ret;
    TAF_Ctrl                    ctrl;
    TAF_CALL_QryCallInfoReqPara qryCallInfoPara;
    ModemIdUint16               modemId;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&qryCallInfoPara, sizeof(qryCallInfoPara), 0x00, sizeof(qryCallInfoPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    qryCallInfoPara.getCallInfoType = TAF_CALL_GET_CALL_INFO_TYPE_CLCC;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发消息到C核获取当前所有通话信息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, &qryCallInfoPara, ID_TAF_CCM_QRY_CALL_INFO_REQ, sizeof(qryCallInfoPara),
                                modemId);

    if (ret != VOS_OK) {
        AT_WARN_LOG("At_SetClccPara: MN_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLCC_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SetChupPara(VOS_UINT8 indexNum)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;
    TAF_Ctrl          ctrl;
    MN_CALL_SupsParam supsCmdPara;
    ModemIdUint16     modemId;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&supsCmdPara, sizeof(supsCmdPara), 0x00, sizeof(supsCmdPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    supsCmdPara.callSupsCmd = MN_CALL_SUPS_CMD_REL_ALL_CALL;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    if (TAF_CCM_CallCommonReq(&ctrl, &supsCmdPara, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(supsCmdPara), modemId) ==
        VOS_OK) {
        /* 停止自动接听 */
        ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            AT_StopRelTimer(ccCtx->s0TimeInfo.timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CHUP_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

/*
 * Description: +CSTA=[<type>]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetCstaPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atCstaNumType = (AT_CSTA_NumTypeUint8)g_atParaList[0].paraValue;

    return AT_OK;
}

VOS_UINT32 AT_CheckDtmfKey(AT_ParseParaType *atPara)
{
    VOS_CHAR key;

    if (atPara->paraLen != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (At_UpString(atPara->para, atPara->paraLen) != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    key = (VOS_CHAR)atPara->para[0];
    if (!(((key >= '0') && (key <= '9')) || (key == '*') || (key == '#') || (key == 'A') || (key == 'B') ||
          (key == 'C') || (key == 'D'))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetVtsPara(TAF_UINT8 indexNum)
{
    VOS_UINT32         result;
    TAF_Ctrl           ctrl;
    TAF_CALL_DtmfParam dtmf;
    ModemIdUint16      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&dtmf, sizeof(dtmf), 0x00, sizeof(dtmf));

    /* 参数类型个数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不是1或2，或者第1个参数长度为0，或者参数个数为2但第2个参数长度为0，均返回错误 */
    if (((g_atParaIndex != 1) && (g_atParaIndex != 2)) || (g_atParaList[0].paraLen == 0) ||
        ((g_atParaIndex == 2) && (g_atParaList[1].paraLen == 0))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数值有效性检查 */
    result = AT_CheckDtmfKey(&g_atParaList[0]);

    if (result != AT_SUCCESS) {
        return result;
    }

    /* 发送START DTMF请求 */
    dtmf.key      = (VOS_CHAR)g_atParaList[0].para[0];
    /* 只有1个参数，时长为默认值 */
    if (g_atParaIndex == 1) {
        dtmf.onLength = AT_VTS_DEFAULT_DTMF_LENGTH;
    } else {
        dtmf.onLength = (VOS_UINT16)(g_atParaList[1].paraValue * AT_VTS_DEFAULT_DTMF_LENGTH);
    }

    dtmf.offLength = 0;

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送消息 */
    result = TAF_CCM_CallCommonReq(&ctrl, &dtmf, ID_TAF_CCM_START_DTMF_REQ, sizeof(dtmf), modemId);

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VTS_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_CheckDtmfPara(VOS_VOID)
{

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex > AT_DTMF_MAX_PARA_NUM) || (g_atParaList[AT_DTMF_ON_LENGTH].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * 发送DTMF时,需要带参数ckey,需要检查参数ckey是否合法
     * 停止DTMF时,不需要带参数ckey,不需要检查参数ckey是否合法
     */
    if (g_atParaList[AT_DTMF_ON_LENGTH].paraValue != AT_DTMF_STOP) {
        if (AT_CheckDtmfKey(&g_atParaList[AT_DTMF_KEY]) != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_SetDtmfPara(VOS_UINT8 indexNum)
{
    VOS_UINT32         result;
    TAF_Ctrl           ctrl;
    TAF_CALL_DtmfParam dtmf;
    ModemIdUint16      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&dtmf, sizeof(dtmf), 0x00, sizeof(dtmf));

    /* 参数有效性检查 */
    result = At_CheckDtmfPara();
    if (result != AT_SUCCESS) {
        return result;
    }

    dtmf.callId = (VOS_UINT8)g_atParaList[AT_DTMF_CALL_ID].paraValue;
    dtmf.key    = (VOS_CHAR)g_atParaList[1].para[0];

    /* 如果时长为1，则使用默认DTMF音时长 */
    if (g_atParaList[AT_DTMF_ON_LENGTH].paraValue == AT_DTMF_START) {
        dtmf.onLength = AT_DTMF_DEFAULT_DTMF_LENGTH;
    } else {
        dtmf.onLength = (VOS_UINT16)g_atParaList[AT_DTMF_ON_LENGTH].paraValue;
    }

    if ((g_atParaList[AT_DTMF_OFF_LENGTH].paraValue == 0) || (g_atParaIndex < AT_DTMF_MAX_PARA_NUM)) {
        dtmf.offLength = 0;
    } else {
        dtmf.offLength = (VOS_UINT16)g_atParaList[AT_DTMF_OFF_LENGTH].paraValue;

        if (dtmf.offLength < AT_DTMF_MIN_DTMF_OFF_LENGTH) {
            /* offlength取值为0或者10ms-60000ms，小于10ms，osa无法启动小于10ms定时器，返回参数错误 */
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    if (dtmf.onLength != AT_DTMF_STOP) {
        result = TAF_CCM_CallCommonReq(&ctrl, &dtmf, ID_TAF_CCM_START_DTMF_REQ, sizeof(dtmf), modemId);
    } else {
        result = TAF_CCM_CallCommonReq(&ctrl, &dtmf, ID_TAF_CCM_STOP_DTMF_REQ, sizeof(dtmf), modemId);
    }

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DTMF_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SendCcmSupsCmdReq(TAF_UINT8 indexNum)
{
    AT_ModemCcCtx          *ccCtx = VOS_NULL_PTR;
    TAF_Ctrl                ctrl;
    MN_CALL_SupsParam       supsCmdPara;
    ModemIdUint16           modemId;
    VOS_UINT32              timerId;

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&supsCmdPara, sizeof(supsCmdPara), 0x00, sizeof(supsCmdPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    supsCmdPara.callSupsCmd = MN_CALL_SUPS_CMD_HOLD_ACT_ACPT_OTH;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    if (TAF_CCM_CallCommonReq(&ctrl, &supsCmdPara, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(supsCmdPara), modemId) ==
        VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_A_SET;
        if (ccCtx->s0TimeInfo.timerStart == TAF_TRUE) {
            timerId = ccCtx->s0TimeInfo.timerName;

            AT_StopRelTimer(timerId, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_NO_CARRIER;
    }
}

TAF_UINT32 At_SetAPara(TAF_UINT8 indexNum)
{
    VOS_UINT8  numOfCalls;
    VOS_UINT8  tmp;
    VOS_UINT32 rlst;
    AT_ModemCcCtx          *ccCtx = VOS_NULL_PTR;
    TAFAGENT_CALL_InfoParam callInfos[MN_CALL_MAX_NUM];

    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(callInfos, sizeof(callInfos), 0x00, sizeof(callInfos));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 从同步API获取通话信息 */
    rlst = TAF_AGENT_GetCallInfoReq(g_atClientTab[indexNum].clientId, &numOfCalls, callInfos);

    if (rlst != VOS_OK) {
        return AT_ERROR;
    }

    if (numOfCalls == 0) {
        return AT_NO_CARRIER;
    }

    numOfCalls = AT_MIN(numOfCalls, MN_CALL_MAX_NUM);
    for (tmp = 0; tmp < numOfCalls; tmp++) {
        /* 设置了自动接听的情况下如果呼叫已经处于incoming态，再触发ATA操作，返回ok */
        if ((callInfos[tmp].callState == MN_CALL_S_INCOMING) && (ccCtx->s0TimeInfo.s0TimerLen != 0) &&
            (ccCtx->s0TimeInfo.timerStart == TAF_FALSE)) {
            return AT_OK;
        }
        /* 设置了自动接听的情况下如果呼叫已经处于非incoming态，再触发ATA操作，返回error */
        else if ((ccCtx->s0TimeInfo.s0TimerLen != 0) && (ccCtx->s0TimeInfo.timerStart == TAF_FALSE)) {
            return AT_NO_CARRIER;
        }
        /* 呼叫已经是active状态再收到ata直接回复ok */
        else if (callInfos[tmp].callState == MN_CALL_S_ACTIVE) {
            return AT_OK;
        } else {
        }
    }

    /* AT向CCM发送补充业务请求 */
    return AT_SendCcmSupsCmdReq(indexNum);
}

LOCAL VOS_UINT32 At_HangupCall(VOS_UINT8 indexNum)
{
    AT_ModemCcCtx    *ccCtx = VOS_NULL_PTR;
    TAF_Ctrl          ctrl;
    MN_CALL_SupsParam supsCmdPara;
    ModemIdUint16     modemId;

    /* 获取ModemID */
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_LOG1("At_SetHPara AT_GetModemIdFromClient fail", indexNum);
        return AT_ERROR;
    }

    /* 获取CC上下文 */
    ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);

    /* 根据协议27007，如果CVHU设置为1时，不支持ATH挂断语音，所以在这里直接返回OK。如果CVHU为其他值，支持ATH挂断语音 */
    if (ccCtx->cvhuMode == CVHU_MODE_1) {
        AT_LOG1("At_SetHPara pstCcCtx->enCvhuMode", ccCtx->cvhuMode);
        return AT_OK;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&supsCmdPara, sizeof(supsCmdPara), 0x00, sizeof(supsCmdPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    supsCmdPara.callSupsCmd = MN_CALL_SUPS_CMD_REL_ALL_CALL;

    /* AT向CCM发送补充业务请求 */
    if (TAF_CCM_CallCommonReq(&ctrl, &supsCmdPara, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(supsCmdPara), modemId) ==
        VOS_OK) {
        /* 停止自动接听 */
        if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
            AT_StopRelTimer(ccCtx->s0TimeInfo.timerName, &(ccCtx->s0TimeInfo.s0Timer));
            ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
            ccCtx->s0TimeInfo.timerName  = 0;
        }

        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_H_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)

VOS_UINT32 AT_CMUX_HangupCall(VOS_UINT8 indexNum)
{
    AT_ClientManage          *clientManage = VOS_NULL_PTR;
    VOS_UINT8                 dataId;
    DMS_PortModeUint8         mode;
    DMS_PortDataModeUint8     dataMode;

     if (AT_CheckCmuxUser(indexNum) == VOS_TRUE) {
        dataId       = g_atClientTab[indexNum].dataId;
        clientManage = AT_GetClientManage(dataId);
        mode         = DMS_PORT_GetMode(clientManage->portNo);
        dataMode     = DMS_PORT_GetDataMode(clientManage->portNo);

        if ((mode == DMS_PORT_MODE_ONLINE_CMD) || ((mode == DMS_PORT_MODE_ONLINE_DATA) &&
            (dataMode == DMS_PORT_DATA_PPP))) {
            (VOS_VOID)TAF_PS_CallEnd(WUEPS_PID_AT,
                                     g_atClientTab[dataId].clientId,
                                     0,
                                     g_atClientTab[dataId].cid);
            g_atClientTab[dataId].cmdCurrentOpt = AT_CMD_H_PS_SET;
            return  AT_WAIT_ASYNC_RETURN;
        }
    }

    return AT_OK;
}
#endif

VOS_UINT32 AT_ModemHangupCall(VOS_UINT8 indexNum)
{
    AT_ClientManage  *clientManage = VOS_NULL_PTR;

    VOS_UINT32        rslt;
    DMS_PortModeUint8 mode;

    clientManage = AT_GetClientManage(indexNum);
    mode         = DMS_PORT_GetMode(clientManage->portNo);

    /* ONLINE-COMMAND模式下, 需要断开PPP拨号连接 */
    if (mode == DMS_PORT_MODE_ONLINE_CMD) {
        /* 正在处理PPP断开请求, 直接继承, 设置命令操作类型为AT_CMD_H_PS_SET */
        if (clientManage->cmdCurrentOpt == AT_CMD_PS_DATA_CALL_END_SET) {
            AT_StopTimerCmdReady(indexNum);
            clientManage->cmdCurrentOpt = AT_CMD_H_PS_SET;
            rslt                        = AT_WAIT_ASYNC_RETURN;
        }
        /* 正在等待PPP的释放响应, 直接释放, 返回OK */
        else if (clientManage->cmdCurrentOpt == AT_CMD_WAIT_PPP_PROTOCOL_REL_SET) {
            AT_StopTimerCmdReady(indexNum);
            DMS_PORT_ResumeCmdMode(clientManage->portNo);
            At_FormatResultData(indexNum, AT_OK);

            if (DMS_PORT_IsDcdModeConnectOn() == VOS_TRUE) {
                DMS_PORT_DeassertDcd(clientManage->portNo);
            }

            rslt = AT_SUCCESS;
        } else {
            (VOS_VOID)TAF_PS_PPP_DIAL_DOWN(WUEPS_PID_AT, AT_PS_BuildExClientId(clientManage->clientId), 0,
                                     clientManage->cid);

            clientManage->cmdCurrentOpt = AT_CMD_H_PS_SET;
            rslt                        = AT_WAIT_ASYNC_RETURN;
        }
    } else if (mode == DMS_PORT_MODE_CMD) {
        return At_HangupCall(indexNum);
    } else {
#if (FEATURE_IOT_CMUX == FEATURE_ON)
        return AT_CMUX_HangupCall(indexNum);
#endif
        rslt = AT_OK;
    }

    return rslt;
}

VOS_UINT32 At_SetHPara(VOS_UINT8 indexNum)
{
    /*
     * 对Modem口进行特殊处理: 收到ATH直接返回OK，
     * 即使Modem口发起过呼叫也不执行挂断操作
     */
    if ((g_atClientTab[indexNum].userType == AT_MODEM_USER) && (g_atClientTab[indexNum].used == AT_CLIENT_USED)) {
        return AT_OK;
    }

    /*
     * 增加通道模式判断:
     * (1) ONLINE-COMMAND模式 - 断开当前通道的PPP拨号连接
     * (2) COMMAND模式        - 直接返回OK
     */
    if (g_atClientTab[indexNum].userType == AT_HSUART_USER) {
        return AT_ModemHangupCall(indexNum);
    }

#if (FEATURE_IOT_UART_CUST == FEATURE_ON)
    if (g_atClientTab[indexNum].userType == AT_UART_USER) {
        return AT_ModemHangupCall(indexNum);
    }
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if (AT_CheckCmuxUser(indexNum) == VOS_TRUE) {
        return AT_ModemHangupCall(indexNum);
    }
#endif

    return At_HangupCall(indexNum);
}

TAF_VOID At_SetDialIGPara(TAF_UINT8 indexNum, TAF_UINT8 paraIndex, TAF_UINT8 *clir, MN_CALL_CugCfg *ccug)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* ->A32D08138 */
    /* 默认情况是使用CCA设置的默认值 */

    *clir = ssCtx->clirType;
    *ccug = ssCtx->ccugCfg;
    /* <-A32D08138 */

    switch (g_atParaList[paraIndex].para[0]) {
        case 'i':
            *clir = MN_CALL_CLIR_SUPPRESS;
            break;

        case 'I':
            *clir = MN_CALL_CLIR_INVOKE;
            break;

        case 'g':
        case 'G':
            ccug->enable = 1;
            break;

        default:
            break;
    }

    switch (g_atParaList[paraIndex + 1].para[0]) {
        case 'g':
        case 'G':
            ccug->enable = 1;
            break;

        default:
            break;
    }
}

/*
 * Description: D><n>[I][G][;]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetDialVoiceIndexPara(TAF_UINT8 indexNum, MN_CALL_TypeUint8 callType)
{
    TAF_UINT32              uLIndex = 0;
    SI_PB_StorateTypeUint32 storage = SI_PB_STORAGE_UNSPECIFIED;
    /* 从当前存储器的存储位置<n>获取被叫号码，并用该号码发起呼叫。 */
    if (At_Auc2ul(g_atParaList[AT_D_BY_N].para, g_atParaList[AT_D_BY_N].paraLen, &uLIndex) == AT_FAILURE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].atCallInfo.callType = callType;

    At_SetDialIGPara(indexNum, AT_D_PARA_INDEX_3, &g_atClientTab[indexNum].atCallInfo.clirInfo,
                     &g_atClientTab[indexNum].atCallInfo.cugMode);

    if (SI_PB_Read(g_atClientTab[indexNum].clientId, 0, storage, (TAF_UINT16)uLIndex, (TAF_UINT16)uLIndex) ==
        AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_GET_NUMBER_BEFORE_CALL;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}
/*
 * Description: D><name>[I][G][;]
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetDialVoiceNamePara(TAF_UINT8 indexNum, MN_CALL_TypeUint8 callType)
{
    /* 根据电话本中存储的姓名<name>获取被叫号码，并用该号码发起呼叫。 */
    if (At_CheckNumLen(SI_PB_ALPHATAG_MAX_LEN, g_atParaList[AT_D_BY_NAME].paraLen) == AT_FAILURE) {
        return AT_ERROR;
    }

    if (At_CheckStringPara(&g_atParaList[AT_D_BY_NAME]) == AT_FAILURE) {
        return AT_ERROR;
    }

    /* 设置ucAlphaTagType */
    if (g_atCscsType == AT_CSCS_UCS2_CODE) {
        if (At_UnicodePrint2Unicode(g_atParaList[AT_D_BY_NAME].para,
                                    &g_atParaList[AT_D_BY_NAME].paraLen) == AT_FAILURE) {
            return AT_ERROR;
        }
    }

    g_atClientTab[indexNum].atCallInfo.callType = callType;

    At_SetDialIGPara(indexNum, AT_D_PARA_INDEX_3, &g_atClientTab[indexNum].atCallInfo.clirInfo,
                     &g_atClientTab[indexNum].atCallInfo.cugMode);


    return AT_ERROR;
}

TAF_UINT32 At_SetDialVoiceMemPara(TAF_UINT8 indexNum, MN_CALL_TypeUint8 callType)
{
    TAF_UINT32              uLIndex = 0;
    SI_PB_StorateTypeUint32 storage = SI_PB_STORAGE_UNSPECIFIED;
    /* 从指定存储器mem的存储位置<n>获取被叫号码，并用该号码发起呼叫，存储器可以通过命令+CPBS=?查询。 */
    if (At_Auc2ul(g_atParaList[AT_D_MEM_N_NUM].para, g_atParaList[AT_D_MEM_N_NUM].paraLen, &uLIndex) == AT_FAILURE) {
        return AT_ERROR;
    }

    if (VOS_StrCmp((TAF_CHAR *)"SM", (TAF_CHAR *)g_atParaList[AT_D_MEM_TYPE].para) == 0) {
        storage = SI_PB_STORAGE_SM;
    } else {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].atCallInfo.callType = callType;

    At_SetDialIGPara(indexNum, AT_D_PARA_INDEX_4, &g_atClientTab[indexNum].atCallInfo.clirInfo,
                     &g_atClientTab[indexNum].atCallInfo.cugMode);

    if (SI_PB_Read(g_atClientTab[indexNum].clientId, 0, storage, (TAF_UINT16)uLIndex, (TAF_UINT16)uLIndex) ==
        AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_GET_NUMBER_BEFORE_CALL;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_CheckAtDPin(VOS_UINT8 indexNum)
{
    TAF_UINT32       rst;
    ModemIdUint16    modemId = MODEM_ID_0;
    VOS_UINT32       getModemIdRslt;
    AT_USIM_InfoCtx *usimInfoCtx = VOS_NULL_PTR;

    getModemIdRslt = AT_GetModemIdFromClient(indexNum, &modemId);

    if (getModemIdRslt != VOS_OK) {
        AT_ERR_LOG("At_CheckAtDPin:Get Modem Id fail!");
        return AT_ERROR;
    }

    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);

    /* SIM卡状态判断 */
    switch (usimInfoCtx->cardStatus) {
        /* 仅替换消息类型 */
        case USIMM_CARDAPP_SERVIC_AVAILABLE:
            rst = AT_SUCCESS;
            break;
        case USIMM_CARDAPP_SERVIC_SIM_PIN:
            rst = AT_CME_SIM_PIN_REQUIRED;
            break;
        case USIMM_CARDAPP_SERVIC_SIM_PUK:
            rst = AT_CME_SIM_PUK_REQUIRED;
            break;
        case USIMM_CARDAPP_SERVIC_UNAVAILABLE:
        case USIMM_CARDAPP_SERVIC_NET_LCOK:
        case USIMM_CARDAPP_SERVICE_IMSI_LOCK:
            rst = AT_CME_SIM_FAILURE;
            break;
        case USIMM_CARDAPP_SERVIC_ABSENT:
            rst = AT_CME_SIM_NOT_INSERTED;
            break;
        default:
            rst = AT_ERROR;
            break;
    }

    return rst;
}

TAF_UINT32 At_SetDial98Para(TAF_UINT8 indexNum)
{
    TAF_ATD_Para atdPara;

    /* 初始化 */
    (VOS_VOID)memset_s(&atdPara, sizeof(atdPara), 0x00, sizeof(TAF_ATD_Para));

    /* CID存在 */
    /* 发送MMI字符串mmi_string。 */
    if (g_atParaList[AT_D_CID].paraLen != 0) {
        if (At_Auc2ul(g_atParaList[AT_D_CID].para, g_atParaList[AT_D_CID].paraLen,
                      &g_atParaList[AT_D_CID].paraValue) == AT_FAILURE) {
            return AT_ERROR;
        }

        if (g_atParaList[AT_D_CID].paraValue > 0xFF) { /* 需要检查，否则过界 */
            return AT_ERROR;
        }

        /* 获取当前用户输入的CID */
        atdPara.cid = (VOS_UINT8)g_atParaList[AT_D_CID].paraValue;
    } else {
        /* 设置默认CID */
        atdPara.cid = AT_DIAL_DEFAULT_CID;
    }

    /* 获取GPRS激活类型 */
    if (TAF_PS_GetGprsActiveType(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, &atdPara) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_GPRS_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_AnalyDial99ParaList(VOS_UINT8 *l2pIndex, VOS_UINT8 *cidIndex)
{
    VOS_UINT8    tmp = 0;
    VOS_UINT8    star = 0;

    for (tmp = 1; tmp < g_atParaIndex; tmp++) {
        if (g_atParaList[tmp].para[0] == '*') {
            star++;
        } else {
            switch (star) {
                case AT_STAR_NUM1:
                case AT_STAR_NUM2:
                    break;

                case AT_STAR_NUM3:
                    *l2pIndex = tmp;
                    break;

                case AT_STAR_NUM4:
                    *cidIndex = tmp;
                    break;

                default:
                    return AT_ERROR;
            }
        }
    }

    return AT_OK;
}

VOS_UINT32 At_SetDial99Para(VOS_UINT8 indexNum)
{
    VOS_INT32    paraPPPFlg;
    VOS_INT32    paraNULLFlg;
    VOS_UINT8    l2pIndex = 0;
    VOS_UINT8    cidIndex = 0;
    TAF_ATD_Para atdPara;

    /* 初始化 */
    (VOS_VOID)memset_s(&atdPara, sizeof(atdPara), 0x00, sizeof(TAF_ATD_Para));

    /* 解析参数列表 */
    if (AT_AnalyDial99ParaList(&l2pIndex, &cidIndex) != AT_OK) {
        return AT_ERROR;
    }

    /* 获取CID: 不存在CID参数时, 取默认值1 */
    if (cidIndex != 0) {
        if (At_Auc2ul(g_atParaList[cidIndex].para, g_atParaList[cidIndex].paraLen, &g_atParaList[cidIndex].paraValue) ==
            AT_FAILURE) {
            return AT_ERROR;
        }

        /* 需要检查，否则过界 */
        if (g_atParaList[cidIndex].paraValue > 0xFF) {
            return AT_ERROR;
        }

        atdPara.cid = (VOS_UINT8)g_atParaList[cidIndex].paraValue;
    } else {
        atdPara.cid = AT_DIAL_DEFAULT_CID;
    }

    /* 获取L2P协议类型: 兼容不支持的协议类型 */
    if (l2pIndex != 0) {
        paraPPPFlg  = VOS_StrCmp((TAF_CHAR *)g_atParaList[l2pIndex].para, "1");
        paraNULLFlg = VOS_StrCmp((TAF_CHAR *)g_atParaList[l2pIndex].para, "0");

        if ((VOS_StrCmp((TAF_CHAR *)g_atParaList[l2pIndex].para, "PPP") == 0) || (paraPPPFlg == 0)) {
            atdPara.opL2p = VOS_TRUE;
            atdPara.l2p   = TAF_L2P_PPP;
        } else if ((VOS_StrCmp((TAF_CHAR *)g_atParaList[l2pIndex].para, "NULL") == 0) || (paraNULLFlg == 0)) {
            atdPara.opL2p = VOS_TRUE;
            atdPara.l2p   = TAF_L2P_NULL;
        } else {

        }
    }

    /* 获取GPRS激活类型 */
    if (TAF_PS_GetGprsActiveType(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, &atdPara) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_GPRS_SET;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_SetDial777Para(TAF_UINT8 indexNum)
{
    TAF_ATD_Para atdPara;

    /* 初始化 */
    (VOS_VOID)memset_s(&atdPara, sizeof(atdPara), 0x00, sizeof(TAF_ATD_Para));

    /* 设置默认CID */
    atdPara.cid = AT_DIAL_DEFAULT_CID;

    /* 获取GPRS激活类型 */
    if (TAF_PS_GetGprsActiveType(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, &atdPara) !=
        VOS_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_GPRS_SET;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_PsDPara(TAF_UINT8 indexNum)
{
    TAF_UINT32 rst;

    rst = At_CheckAtDPin(indexNum);
    if (rst != AT_SUCCESS) {
        return rst;
    }

    /* 如果带分号,D*752#+436644101453; */
    if (At_CheckSemicolon(g_atParaList[0].para[g_atParaList[0].paraLen - 1]) == AT_SUCCESS) {
        /* D*752#+436644101453; */
        g_atParaList[0].para[g_atParaList[0].paraLen - 1] = 0;
        g_atParaList[0].paraLen--;
    } else if (g_atParaList[0].para[g_atParaList[0].paraLen - 1] == '#') {
        if (VOS_StrNiCmp((TAF_CHAR *)g_atParaList[AT_D_L2P].para, "98", g_atParaList[AT_D_L2P].paraLen) == 0) {
            /* D*<GPRS_SC_IP>[*<cid>[,<cid>]]# */
            return At_SetDial98Para(indexNum);
        } else if (VOS_StrNiCmp((TAF_CHAR *)g_atParaList[AT_D_L2P].para, "99", g_atParaList[AT_D_L2P].paraLen) == 0) {
            /* D*<GPRS_SC>[*[<called_address>][*[<L2P>][*[<cid>]]]]# */
            return At_SetDial99Para(indexNum);
        }
        else if (VOS_StrNiCmp((TAF_CHAR *)g_atParaList[AT_D_L2P].para, "777", g_atParaList[AT_D_L2P].paraLen) == 0) {
            return At_SetDial777Para(indexNum);
        }
        else {
        }
    } else {
    }
    return AT_ERROR;
}

VOS_UINT8 At_JudgeIfIsPsCall(VOS_VOID)
{
    VOS_UINT8 psCallFlg;

    if (g_atParaList[0].para[g_atParaList[0].paraLen - 1] != '#') {
        psCallFlg = VOS_FALSE;
    } else if (AT_CheckStrStartWith(&g_atParaList[0].para[1], g_atParaList[0].paraLen, (VOS_UINT8 *)"*98",
                                    AT_DATA_DIAL_LEN)) {
        psCallFlg = VOS_TRUE;
    } else if (AT_CheckStrStartWith(&g_atParaList[0].para[1], g_atParaList[0].paraLen, (VOS_UINT8 *)"*99",
                                    AT_DATA_DIAL_LEN)) {
        psCallFlg = VOS_TRUE;
    }
    else if (AT_CheckStrStartWith(&g_atParaList[1].para[0], g_atParaList[1].paraLen, (VOS_UINT8 *)"#777",
                                  AT_CDATA_DIAL_777_LEN)) {
        psCallFlg = VOS_TRUE;
    }
    else {
        psCallFlg = VOS_FALSE;
    }

    return psCallFlg;
}

TAF_UINT32 At_SetDialNumPara(TAF_UINT8 indexNum, MN_CALL_TypeUint8 callType)
{
    MN_CALL_OrigParam origParam;
    AT_ModemSsCtx    *ssCtx = VOS_NULL_PTR;
    TAF_Ctrl          ctrl;
    ModemIdUint16     modemId;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 目前华为移动的AT命令的需求，应该只要满足一般的语音呼叫即可 */

    /* 初始化 */
    (VOS_VOID)memset_s(&origParam, sizeof(origParam), 0x00, sizeof(origParam));

    /* 设置<dial_string> */
    if (AT_FillCalledNumPara(g_atParaList[AT_D_BY_DIAL_STRING].para,
                             g_atParaList[AT_D_BY_DIAL_STRING].paraLen, &origParam.dialNumber) != VOS_OK) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    origParam.callType = callType;

    At_SetDialIGPara(indexNum, AT_D_PARA_INDEX_2, &origParam.clirCfg, &origParam.cugCfg);

    origParam.callMode = ssCtx->cModType;

    origParam.dataCfg = ssCtx->cbstDataCfg;

    /* 考虑数据卡的接口，这里的定义视频呼叫域为3GPP的 */
    if (callType == MN_CALL_TYPE_VIDEO) {
        origParam.voiceDomain = TAF_CALL_VOICE_DOMAIN_3GPP;
    } else {
        origParam.voiceDomain = TAF_CALL_VOICE_DOMAIN_AUTO;
    }

    /* 遗留问题6、呼叫接口，需要增加设置I、G的参数；需要修改D命令实现 */

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &origParam, ID_TAF_CCM_CALL_ORIG_REQ, sizeof(origParam), modemId) == VOS_OK) {
        if (callType == MN_CALL_TYPE_VOICE) {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_CS_VOICE_CALL_SET;
        } else {
            g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_D_CS_DATA_CALL_SET;
        }
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_UNKNOWN, VOS_NULL_PTR);

        return AT_ERROR;
    }
}

TAF_UINT32 At_SetDPara(TAF_UINT8 indexNum)
{
    /* cS call */
    if (At_JudgeIfIsPsCall() != VOS_TRUE) {
        MN_CALL_TypeUint8 callType = MN_CALL_TYPE_VOICE;

#if (FEATURE_ECALL == FEATURE_ON)
        /* 当前有ecall正在通话或者发起 */
        if (AT_HaveEcallActive(indexNum, VOS_FALSE) == VOS_TRUE) {
            return AT_NO_CARRIER;
        }
#endif /* FEATURE_ECALL == FEATURE_ON */
        /* 如果命令末尾有分号，表示发起语音呼叫；如果没有分号，则表示发起数据呼叫。 */
        if (g_atParaList[AT_D_DIAL_STRING].para[g_atParaList[AT_D_DIAL_STRING].paraLen - 1] != ';') {
            callType = MN_CALL_TYPE_VIDEO;
        }
        /* 从指定存储器mem的存储位置<n>获取被叫号码，并用该号码发起呼叫，存储器可以通过命令+CPBS=?查询。 */
        if (g_atParaList[AT_D_BY_MEM_N_POSITION].para[0] == '>') {
            /* 根据电话本中存储的姓名<name>获取被叫号码，并用该号码发起呼叫。 */
            if ((g_atParaList[AT_D_BY_NAME].para[0] >= 'A') && (g_atParaList[AT_D_BY_NAME].para[0] <= 'Z')) {
                /* D>mem<n>[I][G][;] */
                return At_SetDialVoiceMemPara(indexNum, callType);
             /* 从当前存储器的存储位置<n>获取被叫号码，并用该号码发起呼叫。 */
            } else if ((g_atParaList[AT_D_BY_N].para[0] >= '0') && (g_atParaList[AT_D_BY_N].para[0] <= '9')) {
                /* D><n>[I][G][;] */
                return At_SetDialVoiceIndexPara(indexNum, callType);
            } else {
                /* D><name>[I][G][;] */
                return At_SetDialVoiceNamePara(indexNum, callType);
            }
        } else {
            /* D[<dial_string>][I][G][;] */
            return At_SetDialNumPara(indexNum, callType);
        }
    } else {
        return At_PsDPara(indexNum);
    }
}

VOS_UINT32 AT_SetCdurPara(VOS_UINT8 indexNum)
{
    VOS_UINT32           ret;
    TAF_Ctrl             ctrl;
    TAF_CALL_GetCdurPara getCdurPara;
    ModemIdUint16        modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&getCdurPara, sizeof(getCdurPara), 0x00, sizeof(getCdurPara));

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0) || (g_atParaList[0].paraValue > AT_CALL_MAX_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    getCdurPara.callId = (VOS_UINT8)g_atParaList[0].paraValue;

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发消息到C核获取通话时长 */
    ret = TAF_CCM_CallCommonReq(&ctrl, &getCdurPara, ID_TAF_CCM_GET_CDUR_REQ, sizeof(getCdurPara), modemId);

    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetCdurPara: MN_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CDUR_READ;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_VOID AT_FillOrigParamApdsPara(MN_CALL_OrigParam *origParam, VOS_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 默认情况是使用CCA设置的默认值 */
    origParam->callMode = ssCtx->cModType;
    origParam->dataCfg  = ssCtx->cbstDataCfg;
    origParam->clirCfg  = ssCtx->clirType;
    origParam->cugCfg   = ssCtx->ccugCfg;
}

VOS_UINT32 AT_FillApdsPara(VOS_UINT8 indexNum, MN_CALL_OrigParam *origParam)
{
    errno_t        memResult;

    AT_FillOrigParamApdsPara (origParam, indexNum);

    /* 设置<dial_string> */
    if (g_atParaList[AT_APDS_DIAL_STRING].paraLen == 0) {
        return VOS_ERR;
    } else {
        if (AT_FillCalledNumPara(g_atParaList[AT_APDS_DIAL_STRING].para, g_atParaList[AT_APDS_DIAL_STRING].paraLen,
                                 &origParam->dialNumber) != VOS_OK) {
            return VOS_ERR;
        }
    }

    /* 检查被叫号码子地址的长度 */
    if (g_atParaList[AT_APDS_SUB_STRING].paraLen > MN_CALL_MAX_SUBADDR_INFO_LEN) {
        return VOS_ERR;
    }

    /* 设置<sub_string> */
    if (g_atParaList[AT_APDS_SUB_STRING].paraLen != 0) {
        origParam->subaddr.isExist = VOS_TRUE;

        /* 偏移量 = 子地址编码的长度 + Octet3的长度 */
        origParam->subaddr.lastOctOffset = (VOS_UINT8)g_atParaList[AT_APDS_SUB_STRING].paraLen +
                                           sizeof(origParam->subaddr.octet3);
        origParam->subaddr.octet3        = (0x80 | (MN_CALL_SUBADDR_NSAP << 4));
        memResult = memcpy_s(origParam->subaddr.subAddrInfo, MN_CALL_MAX_SUBADDR_INFO_LEN,
                             g_atParaList[AT_APDS_SUB_STRING].para, g_atParaList[AT_APDS_SUB_STRING].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, MN_CALL_MAX_SUBADDR_INFO_LEN, g_atParaList[AT_APDS_SUB_STRING].paraLen);
    }

    /* 设置<I><G><call_type><voice_domain>参数 */
    if (g_atParaList[AT_APDS_I].paraLen != 0) {
        if (g_atParaList[AT_APDS_I].paraValue == 0) {
            origParam->clirCfg = AT_CLIR_INVOKE;
        } else if (g_atParaList[AT_APDS_I].paraValue == 1) {
            origParam->clirCfg = AT_CLIR_SUPPRESS;
        } else {
            return VOS_ERR;
        }
    }

    if (g_atParaList[AT_APDS_G].paraLen != 0) {
        origParam->cugCfg.enable = g_atParaList[AT_APDS_G].paraValue;
    }

    if (g_atParaList[AT_APDS_CALL_TYPE].paraLen != 0) {
        origParam->callType = (VOS_UINT8)g_atParaList[AT_APDS_CALL_TYPE].paraValue;
    }

    if (g_atParaList[AT_APDS_CALLDOMAIN].paraLen != 0) {
        origParam->voiceDomain = (VOS_UINT8)g_atParaList[AT_APDS_CALLDOMAIN].paraValue;
    }

    if (g_atParaList[AT_APDS_SRV_TYPE].paraLen != 0) {
        if (g_atParaList[AT_APDS_SRV_TYPE].paraValue == 1) {
            return VOS_ERR;
        }
        origParam->serviceType = (VOS_UINT8)g_atParaList[AT_APDS_SRV_TYPE].paraValue;
    }

    return VOS_OK;
}

VOS_UINT32 AT_FillApdsPara_Part2(VOS_UINT8 indexNum, MN_CALL_OrigParam *origParam)
{
    if (g_atParaList[AT_APDS_RTTFLG].paraLen != 0) {
        origParam->rttFlg = (VOS_UINT8)g_atParaList[AT_APDS_RTTFLG].paraValue;
    }

    if (g_atParaList[AT_APDS_CALLPULLDIALOGID].paraLen != 0) {
        origParam->callPullDialogId = (VOS_UINT8)g_atParaList[AT_APDS_CALLPULLDIALOGID].paraValue;
        if (origParam->callPullDialogId == TAF_APDS_CALL_PULL_DIALOG_ID) {
            origParam->callPullFlg = VOS_FALSE;
        } else {
            origParam->callPullFlg = VOS_TRUE;
        }
    }

    if (g_atParaList[AT_APDS_ENCRYPTFLAG].paraLen != 0) {
        origParam->encryptFlag = (VOS_UINT8)g_atParaList[AT_APDS_ENCRYPTFLAG].paraValue;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetApdsPara(VOS_UINT8 indexNum)
{
    MN_CALL_OrigParam origParam;
    TAF_Ctrl          ctrl;
    VOS_UINT32        relt;
    ModemIdUint16     modemId;

    /* 初始化 */
    (VOS_VOID)memset_s(&origParam, sizeof(origParam), 0x00, sizeof(MN_CALL_OrigParam));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }
    /* 参数过多 */
    if (g_atParaIndex > AT_APDS_MAX_PARA_NUM) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查并填写输入的参数 */
    relt = AT_FillApdsPara(indexNum, &origParam);
    if (relt == VOS_OK) {
        relt = AT_FillApdsPara_Part2(indexNum, &origParam);
    }

    if (relt != VOS_OK) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息原语 */
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &origParam, ID_TAF_CCM_CALL_ORIG_REQ, sizeof(origParam), modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_APDS_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_UNKNOWN, VOS_NULL_PTR);
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetClprPara(VOS_UINT8 indexNum)
{
    VOS_UINT32           result;
    TAF_Ctrl             ctrl;
    TAF_CALL_QryClprPara qryClprPara;
    ModemIdUint16        modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    qryClprPara.callId          = (MN_CALL_ID_T)g_atParaList[0].paraValue;
    qryClprPara.qryClprModeType = TAF_CALL_QRY_CLPR_MODE_GUL;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送跨核消息ID_TAF_CCM_QRY_CLPR_REQ到C核 */
    result = TAF_CCM_CallCommonReq(&ctrl, &qryClprPara, ID_TAF_CCM_QRY_CLPR_REQ, sizeof(qryClprPara), modemId);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetClprPara: TAF_CCM_CallCommonReq fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLPR_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_ProcNormalMpTyCall(TAF_UINT8 indexNum, ModemIdUint16 modemId)
{
    AT_ModemSsCtx         *ssCtx = VOS_NULL_PTR;
    MN_CALL_OrigParam      origParam;
    TAF_Ctrl               ctrl;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&origParam, sizeof(origParam), 0x00, sizeof(MN_CALL_OrigParam));
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;
    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 普通多方通话只能邀请一个用户 */
    if (g_atParaList[AT_CACMIMS_TOTAL_CALLNUM].paraValue != 1) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置<dial_string> */
    if (AT_FillCalledNumPara(g_atParaList[AT_CACMIMS_CALL_ADDRESS].para,
                             g_atParaList[AT_CACMIMS_CALL_ADDRESS].paraLen, &origParam.dialNumber) != VOS_OK) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    origParam.callType          = MN_CALL_TYPE_VOICE;
    origParam.callMode          = ssCtx->cModType;
    origParam.dataCfg           = ssCtx->cbstDataCfg;
    origParam.imsInvitePtptFlag = VOS_TRUE;

    origParam.voiceDomain = TAF_CALL_VOICE_DOMAIN_AUTO;

    /* AT向CCM发送呼叫消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &origParam, ID_TAF_CCM_CALL_ORIG_REQ, sizeof(origParam), modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CACMIMS_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_UNKNOWN, VOS_NULL_PTR);
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ProcEconfMptyCall(TAF_UINT8 indexNum, ModemIdUint16 modemId)
{
    AT_ModemSsCtx         *ssCtx = VOS_NULL_PTR;
    TAF_CALL_EconfDialInfo econfDialInfo;
    TAF_Ctrl               ctrl;
    VOS_UINT32             indexValue;
    VOS_UINT32             callNum;
    VOS_UINT32             i;
    VOS_UINT32             result;

    (VOS_VOID)memset_s(&econfDialInfo, sizeof(econfDialInfo), 0x00, sizeof(econfDialInfo));
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);
    callNum = g_atParaList[AT_CACMIMS_TOTAL_CALLNUM].paraValue;

    /* 默认情况是使用CCA设置的默认值 */
    econfDialInfo.callMode = ssCtx->cModType;
    econfDialInfo.dataCfg  = ssCtx->cbstDataCfg;
    econfDialInfo.clirCfg  = ssCtx->clirType;
    econfDialInfo.cugCfg   = ssCtx->ccugCfg;

    for (i = 0; i < callNum; i++) {
        indexValue = 2 + i; /* 2表示CACMIMS指令参数total_callnum 和 isEcon占下标索引0和1，call_address下标从2开始 */

        if (g_atParaList[indexValue].paraLen == 0) {
            AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);

            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (AT_FillCalledNumPara(g_atParaList[indexValue].para, g_atParaList[indexValue].paraLen,
                                 &econfDialInfo.econfCalllist.dialNumber[i]) != VOS_OK) {
            AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    econfDialInfo.econfCalllist.callNum = callNum;
    econfDialInfo.callType              = MN_CALL_TYPE_VOICE;
    econfDialInfo.voiceDomain           = TAF_CALL_VOICE_DOMAIN_IMS;
    econfDialInfo.imsInvitePtptFlag     = VOS_TRUE;

    /* AT向CCM发送增强多方通话消息 */
    result = TAF_CCM_CallCommonReq(&ctrl, &econfDialInfo, ID_TAF_CCM_ECONF_DIAL_REQ, sizeof(econfDialInfo),
                                   modemId);

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CACMIMS_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_UNKNOWN, VOS_NULL_PTR);
        return AT_ERROR;
    }
}

TAF_UINT32 AT_SetCacmimsPara(TAF_UINT8 indexNum)
{
    ModemIdUint16          modemId;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不对 */
    /* 参数的个数应该是total_callnum+2 */
    if ((g_atParaList[AT_CACMIMS_TOTAL_CALLNUM].paraValue + 2) != g_atParaIndex) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 普通多方通话邀请 */
    if (g_atParaList[AT_CACMIMS_IS_ECON].paraValue == 0) {
        return AT_ProcNormalMpTyCall(indexNum, modemId);
    }
    /* 增强型多方通话邀请 */
    else {
        return AT_ProcEconfMptyCall(indexNum, modemId);
    }
}

VOS_UINT32 AT_FillEconfDialPara(VOS_UINT8 indexNum, TAF_CALL_EconfDialInfo *dialParam)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;
    VOS_UINT32     callNum;
    VOS_UINT32     i;
    VOS_UINT32     indexValue;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    /* 默认情况是使用CCA设置的默认值 */
    dialParam->callMode = ssCtx->cModType;
    dialParam->dataCfg  = ssCtx->cbstDataCfg;
    dialParam->clirCfg  = ssCtx->clirType;
    dialParam->cugCfg   = ssCtx->ccugCfg;

    /* 第二个参数为会议中心号码，目前不关注 */
    if ((g_atParaList[AT_ECONFDIAL_DIAL_COUNT].paraLen == 0) || (g_atParaList[AT_ECONFDIAL_DIAL_I].paraLen == 0) ||
        (g_atParaList[AT_ECONFDIAL_CALL_TYPE].paraLen == 0) || (g_atParaList[AT_ECONFDIAL_CALLDOMAIN].paraLen == 0) ||
        (g_atParaList[AT_ECONFDIAL_ISECONF].paraLen   == 0)) {
        return VOS_ERR;
    } else {
        /* 从第7个参数开始为与会者号码，第一个参数指明与会者人数 */
        callNum = g_atParaList[AT_ECONFDIAL_DIAL_COUNT].paraValue;

        for (i = 0; i < callNum; i++) {
            indexValue = AT_ECONFDIAL_FIRST_SIX_PARA_NUM + i;

            if (g_atParaList[indexValue].paraLen == 0) {
                return VOS_ERR;
            }

            if (AT_FillCalledNumPara(g_atParaList[indexValue].para, g_atParaList[indexValue].paraLen,
                                     &dialParam->econfCalllist.dialNumber[i]) != VOS_OK) {
                return VOS_ERR;
            }
        }

        dialParam->econfCalllist.callNum = callNum;
    }

    /* 设置<I>参数 */
    if (g_atParaList[AT_ECONFDIAL_DIAL_I].paraValue == 0) {
        dialParam->clirCfg = AT_CLIR_INVOKE;
    } else if (g_atParaList[AT_ECONFDIAL_DIAL_I].paraValue == 1) {
        dialParam->clirCfg = AT_CLIR_SUPPRESS;
    } else {
        return VOS_ERR;
    }

    dialParam->callType          = (MN_CALL_TypeUint8 )g_atParaList[AT_ECONFDIAL_CALL_TYPE].paraValue;
    dialParam->voiceDomain       = (TAF_CALL_VoiceDomainUint8 )g_atParaList[AT_ECONFDIAL_CALLDOMAIN].paraValue;
    dialParam->imsInvitePtptFlag = VOS_FALSE;

    return VOS_OK;
}

VOS_UINT32 AT_SetEconfDialPara(VOS_UINT8 indexNum)
{
    TAF_CALL_EconfDialInfo econfDialInfo;
    TAF_Ctrl               ctrl;
    VOS_UINT32             result;
    ModemIdUint16          modemId;

    (VOS_VOID)memset_s(&econfDialInfo, sizeof(econfDialInfo), 0x00, sizeof(econfDialInfo));

    /* 参数检查 */
    if ((g_atParaIndex < AT_CMD_ECONF_DIAL_MIN_PARA_NUM) || (g_atParaIndex > AT_CMD_ECONF_DIAL_MAX_PARA_NUM)) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不对 */
    /* 参数的个数应该是total_callnum+6 */
    if ((g_atParaList[AT_ECONFDIAL_DIAL_COUNT].paraValue + 6) != g_atParaIndex) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查并填写输入的参数 */
    result = AT_FillEconfDialPara(indexNum, &econfDialInfo);
    if (result != VOS_OK) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送增强多方通话消息 */
    result = TAF_CCM_CallCommonReq(&ctrl, &(econfDialInfo), ID_TAF_CCM_ECONF_DIAL_REQ, sizeof(econfDialInfo), modemId);

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECONF_DIAL_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_VOID At_MapInputValueToCcwaiMode(VOS_UINT32 value, TAF_CALL_CcwaiModeUint8 *mode)
{
    switch (value) {
        /* ccwaimode disable */
        case 0:
            *mode = TAF_CALL_CCWAI_MODE_DISABLE;
            break;
        /* ccwaimode enable */
        case 1:
            *mode = TAF_CALL_CCWAI_MODE_ENABLE;
            break;

        default:
            *mode = TAF_CALL_CCWAI_MODE_BUTT;
            break;
    }
}

VOS_VOID At_MapInputValueToCcwaiSrvClass(VOS_UINT32 value, TAF_CALL_CcwaiSrvClassUint8 *srvClass)
{
    switch (value) {
        case AT_CALL_SRV_CLASS_VOICE:
            *srvClass = TAF_CALL_CCWAI_SRV_CLASS_VOICE;
            break;

        case AT_CALL_SRV_CLASS_VIDEO:
            *srvClass = TAF_CALL_CCWAI_SRV_CLASS_VIDEO;
            break;

        case AT_CALL_SRV_CLASS_SET_NW:
            *srvClass = TAF_CALL_CCWAI_SRV_CLASS_SET_NW;
            break;

        default:
            *srvClass = TAF_CALL_CCWAI_SRV_CLASS_BUTT;
            break;
    }
}

TAF_UINT32 AT_SetCcwaiPara(TAF_UINT8 indexNum)
{
    VOS_UINT32           result;
    TAF_Ctrl             ctrl;
    TAF_CALL_CcwaiSetReq ccwaiSet;
    ModemIdUint16        modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&ccwaiSet, sizeof(ccwaiSet), 0x00, sizeof(ccwaiSet));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    /* 参数过多 */
    if (g_atParaIndex > AT_CCWAI_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((g_atParaList[AT_CCWAI_MODE].paraLen == 0) || (g_atParaList[AT_CCWAI_SERVICE_CLASS].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充消息结构体 */
    At_MapInputValueToCcwaiMode(g_atParaList[AT_CCWAI_MODE].paraValue, &ccwaiSet.mode);
    At_MapInputValueToCcwaiSrvClass(g_atParaList[AT_CCWAI_SERVICE_CLASS].paraValue, &ccwaiSet.srvClass);

    if ((ccwaiSet.mode == TAF_CALL_CCWAI_MODE_BUTT) || (ccwaiSet.srvClass == TAF_CALL_CCWAI_SRV_CLASS_BUTT)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 给CCM发送^CCWAI设置请求 */
    result = TAF_CCM_CallCommonReq(&ctrl, &ccwaiSet, ID_TAF_CCM_CCWAI_SET_REQ, sizeof(ccwaiSet), modemId);

    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCWAI_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif

VOS_UINT32 AT_SetRejCallPara(VOS_UINT8 indexNum)
{
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;

    TAF_Ctrl          ctrl;
    MN_CALL_SupsParam callSupsPara;
    ModemIdUint16     modemId;

    /* 指令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetRejCallPara : Current Option is not AT_CMD_REJCALL!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (g_atParaIndex != AT_REJCALL_PARA_NUM) {
        AT_WARN_LOG("AT_SetRejCallPara : The number of input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        AT_WARN_LOG("AT_SetRejCallPara : Input parameters is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&callSupsPara, sizeof(callSupsPara), 0x00, sizeof(callSupsPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    callSupsPara.callSupsCmd  = MN_CALL_SUPS_CMD_REL_INCOMING_OR_WAITING;
    callSupsPara.callId       = (VOS_UINT8)g_atParaList[0].paraValue;
    callSupsPara.callRejCause = (VOS_UINT8)g_atParaList[1].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    if (TAF_CCM_CallCommonReq(&ctrl, &callSupsPara, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(callSupsPara), modemId) !=
        VOS_OK) {
        AT_WARN_LOG("AT_SetRejCallPara : Send Msg fail!");
        return AT_ERROR;
    }

    /* 停止自动接听 */
    ccCtx = AT_GetModemCcCtxAddrFromClientId(indexNum);

    if (ccCtx->s0TimeInfo.timerStart == VOS_TRUE) {
        AT_StopRelTimer(ccCtx->s0TimeInfo.timerName, &(ccCtx->s0TimeInfo.s0Timer));
        ccCtx->s0TimeInfo.timerStart = TAF_FALSE;
        ccCtx->s0TimeInfo.timerName  = 0;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_REJCALL_SET;

    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 AT_SetCvhuPara(VOS_UINT8 indexNum)
{
    ModemIdUint16  modemId;
    AT_ModemCcCtx *ccCtx = VOS_NULL_PTR;

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数错误 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将设置参数保存到CC上下文中 */
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        AT_LOG1("AT_SetCvhuPara AT_GetModemIdFromClient fail", indexNum);
        return AT_ERROR;
    }

    ccCtx = AT_GetModemCcCtxAddrFromModemId(modemId);

    /* 如果参数为空，默认按照CVHU_MODE_0处理 */
    if (g_atParaIndex == 0) {
        ccCtx->cvhuMode = CVHU_MODE_0;
    } else {
        ccCtx->cvhuMode = (AT_CvhuModeUint8)(g_atParaList[0].paraValue);
    }

    return AT_OK;
}

#if (FEATURE_ECALL == FEATURE_ON)
LOCAL VOS_UINT32 AT_SetEclstartParaCheckArgs(VOS_UINT8 indexNum)
{
    /*  判断1: 必选参数的长度不能为0 */
    if ((g_atParaList[AT_ECLSTART_ACTIVATION_TYPE].paraLen == 0) ||
        (g_atParaList[AT_ECLSTART_TYPE_OF_CALL].paraLen == 0)) {
        return AT_ERROR;
    }

    /* 判断2: 参数个数不对, 判断1已保证参数个数>=2 */
    if (g_atParaIndex > AT_ECLSTART_PARA_MAX_NUM) {
        return AT_ERROR;
    }

    /* 判断3:  AT^ECLSTART=1,1, 这种情况是错的 */
    if ((g_atParaIndex == 3) && (g_atParaList[AT_ECLSTART_DIAL_NUM].paraLen == 0)) {
        return AT_ERROR;
    }

    /* 判断4:  AT^ECLSTART=1,1,, 这种情况是错的 */
    if ((g_atParaIndex == 4) && (g_atParaList[AT_ECLSTART_OPRT_MADE].paraLen == 0)) {
        return AT_ERROR;
    }

    /* 判断5:  当前ecall通话中 */
    if (AT_HaveEcallActive(indexNum, VOS_FALSE) == VOS_TRUE) {
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetEclstartPara(VOS_UINT8 indexNum)
{
    TAF_Ctrl                            ctrl = {0};
    MN_CALL_OrigParam                   callOrigPara = {0};
    MN_CALL_CalledNum                   dialNumber = {0};
    APP_VC_SetOprtmodeReq               eclOprtModepara = {0};
    VOS_UINT32                          rst;
    ModemIdUint16                       modemId;
    errno_t                             memResult;

    if (AT_SetEclstartParaCheckArgs(indexNum) == AT_ERROR) {
        return AT_ERROR;
    }

    /* oprt_mode 默认为PUSH模式 */
    if ((g_atParaList[AT_ECLSTART_OPRT_MADE].paraLen == 0) ||
        (g_atParaList[AT_ECLSTART_OPRT_MADE].paraValue == APP_VC_ECALL_OPRT_PUSH)) {
        eclOprtModepara.ecallOpMode = APP_VC_ECALL_OPRT_PUSH;
    } else {
        eclOprtModepara.ecallOpMode = APP_VC_ECALL_OPRT_PULL;
    }

    /* 发送VC模块保存当前请求模式 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 APP_VC_MSG_SET_ECALL_OPRTMODE_REQ, (VOS_UINT8 *)&eclOprtModepara,
                                 sizeof(eclOprtModepara), I0_WUEPS_PID_VC);

    if (rst == TAF_FAILURE) {
        return AT_ERROR;
    }

    /* 带拨号号码 */
    if (g_atParaList[AT_ECLSTART_DIAL_NUM].paraLen > 0) {
        /* 检查并转换电话号码 */
        if (AT_FillCalledNumPara(g_atParaList[AT_ECLSTART_DIAL_NUM].para,
                                 g_atParaList[AT_ECLSTART_DIAL_NUM].paraLen, &dialNumber) != VOS_OK) {
            AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
            return AT_ERROR;
        }
    }

    /* ^ECLSTART=x,0  发起测试call */
    if (g_atParaList[AT_ECLSTART_TYPE_OF_CALL].paraValue == 0) {
        callOrigPara.callType = MN_CALL_TYPE_TEST;
        callOrigPara.serviceType = TAF_CALL_SERVICE_TYPE_TEST_ECALL;
        /* 只有测试呼叫才下发电话号码，紧急呼叫不下发电话号码 */
        memResult = memcpy_s(&callOrigPara.dialNumber, sizeof(callOrigPara.dialNumber), &dialNumber, sizeof(dialNumber));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(callOrigPara.dialNumber), sizeof(dialNumber));
    } else if (g_atParaList[AT_ECLSTART_TYPE_OF_CALL].paraValue == 1) {
        /* ^ECLSTART=0,1  用户发起紧急call */
        if (g_atParaList[AT_ECLSTART_ACTIVATION_TYPE].paraValue == 0) {
            callOrigPara.callType = MN_CALL_TYPE_MIEC;
            callOrigPara.serviceType = TAF_CALL_SERVICE_TYPE_MIEC;
        }
        /* ^ECLSTART=1,1  自动发起紧急call */
        else {
            callOrigPara.callType = MN_CALL_TYPE_AIEC;
            callOrigPara.serviceType = TAF_CALL_SERVICE_TYPE_AIEC;
        }
    } else {
        callOrigPara.callType = MN_CALL_TYPE_RECFGURATION;
        callOrigPara.serviceType = TAF_CALL_SERVICE_TYPE_RECFGURATION_ECALL;
        /* 只有测试呼叫才下发电话号码，紧急呼叫不下发电话号码 */
        memResult = memcpy_s(&callOrigPara.dialNumber, sizeof(callOrigPara.dialNumber), &dialNumber, sizeof(dialNumber));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(callOrigPara.dialNumber), sizeof(dialNumber));
    }
    callOrigPara.voiceDomain = TAF_CALL_VOICE_DOMAIN_AUTO;
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, &callOrigPara, ID_TAF_CCM_CALL_ORIG_REQ, sizeof(callOrigPara), modemId);
    if (rst != VOS_OK) {
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLSTART_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetEclstopPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    TAF_Ctrl          ctrl;
    MN_CALL_SupsParam callSupsPara;
    ModemIdUint16     modemId;

    /* 不带参数的设置命令 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_ERROR;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&callSupsPara, sizeof(callSupsPara), 0x00, sizeof(callSupsPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    callSupsPara.callSupsCmd = MN_CALL_SUPS_CMD_REL_ECALL;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* AT向CCM发送呼叫消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, &callSupsPara, ID_TAF_CCM_CALL_SUPS_CMD_REQ, sizeof(callSupsPara), modemId);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLSTOP_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEclcfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32               rst;
    APP_VC_MsgSetEcallCfgReq eclcfgSetPara;

    /* 判断一: 必选参数的长度不能为0 */
    if (g_atParaList[AT_ECLCFG_MODE].paraLen == 0) {
        return AT_ERROR;
    }

    /* 判断二: 参数个数不对, 判断一已经保证参数个数>=1 */
    if (g_atParaIndex > AT_ECLCFG_PARA_MAX_NUM) {
        return AT_ERROR;
    }

    /* 判断3:  AT^ECLCFG=0, 或者 AT^ECLCFG=0,1, 这种情况是错的 */
    if (((g_atParaIndex == AT_ECLCFG_PARA_MAX_NUM - 1) && (g_atParaList[AT_ECLCFG_VOC_CONFIG].paraLen == 0)) ||
        ((g_atParaIndex == AT_ECLCFG_PARA_MAX_NUM) && (g_atParaList[AT_ECLCFG_REDIAL_CONFIG].paraLen == 0))) {
        return AT_ERROR;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&eclcfgSetPara, sizeof(eclcfgSetPara), 0x00, sizeof(eclcfgSetPara));

    eclcfgSetPara.mode = (APP_VC_EcallMsdModeUint16)g_atParaList[AT_ECLCFG_MODE].paraValue;

    if (g_atParaList[AT_ECLCFG_VOC_CONFIG].paraLen == 0) {
        eclcfgSetPara.vocConfig = VOC_CONFIG_NO_CHANGE;
    } else {
        eclcfgSetPara.vocConfig = (APP_VC_EcallVocConfigUint16)g_atParaList[AT_ECLCFG_VOC_CONFIG].paraValue;
    }

    /* 取消ECALL重拨功能，保留第三个参数, 目前只能下发关闭命令 */
    if ((g_atParaList[AT_ECLCFG_REDIAL_CONFIG].paraLen != 0) &&
        (g_atParaList[AT_ECLCFG_REDIAL_CONFIG].paraValue == VOS_TRUE)) {
        return AT_ERROR;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 APP_VC_MSG_SET_ECALL_CFG_REQ, (VOS_UINT8 *)&eclcfgSetPara, sizeof(eclcfgSetPara),
                                 I0_WUEPS_PID_VC);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEclmsdPara(VOS_UINT8 indexNum)
{
    VOS_UINT32          rst;
    APP_VC_MsgSetMsdReq eclmsdPara;

    /*
     * 参数必须为1。
     * 参数个数为1时，参数长度不可能为0，
     * 即"AT+CELMSD="情况下，g_atParaIndex为0
     */
    if (g_atParaIndex != 1) {
        return AT_ERROR;
    }

    /*
     * 参数长度不对
     * 十六进制文本字符串，采用十六进制数据编码方式，字符串长度为280个字节，表示的是MSD协议要求的140个字节原始数据
     * 需给MSD协议的原始数据乘2
     */
    if (((APP_VC_MSD_DATA_LEN * 2) != g_atParaList[AT_ECLMSD_ECALL_MSD].paraLen)) {
        return AT_ERROR;
    }

    if (At_AsciiString2HexSimple(eclmsdPara.msdData, g_atParaList[AT_ECLMSD_ECALL_MSD].para,
                                 APP_VC_MSD_DATA_LEN * 2) == AT_FAILURE) {
        return AT_ERROR;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, APP_VC_MSG_SET_MSD_REQ,
                                 (VOS_UINT8 *)&eclmsdPara, sizeof(eclmsdPara), I0_WUEPS_PID_VC);
    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLMSD_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEclAbortPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    VOS_UINT32 abortReason = 0;

    /* 不带参数的设置命令 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_ERROR;
    }

    /* 当前非Ecall激活状态返回ERROR */
    if (AT_HaveEcallActive(indexNum, VOS_TRUE) == VOS_FALSE) {
        return AT_ERROR;
    }

    /* 往VC发送APP_VC_MSG_ECALL_ABORT_REQ命令 */
    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 APP_VC_MSG_SET_ECALL_ABORT_REQ, (VOS_UINT8 *)&abortReason, sizeof(abortReason),
                                 I0_WUEPS_PID_VC);

    if (ret == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ABORT_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEclModePara(VOS_UINT8 indexNum)
{
    TAF_NVIM_CustomEcallCfg ecallCfg;
    VOS_UINT32              result;

    (VOS_VOID)memset_s(&ecallCfg, sizeof(ecallCfg), 0x00, sizeof(ecallCfg));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetEclModePara: ucCmdOptType Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        AT_ERR_LOG("AT_SetEclModePara: num Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <mode>的值只支持设置为0-2 */
    if (g_atParaList[AT_ECLMODE_ECALLFORCE_MODE].paraValue > AT_ECLMODE_ECALLFORCE_MODE_MAX_VALUE) {
        AT_ERR_LOG("AT_SetEclModePara: value Error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 先读取NV的值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CUSTOM_ECALL_CFG, &ecallCfg, sizeof(ecallCfg));

    /* NV读取失败或NV未激活时，返回error */
    if (result != NV_OK) {
        AT_ERR_LOG("AT_SetEclModePara: NV read!");
        return AT_ERROR;
    }

    if (ecallCfg.ecallForceMode != g_atParaList[AT_ECLMODE_ECALLFORCE_MODE].paraValue) {
        ecallCfg.ecallForceMode = (VOS_UINT8)g_atParaList[AT_ECLMODE_ECALLFORCE_MODE].paraValue;
        /* 写入NV的值 */
        result = TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_CUSTOM_ECALL_CFG, (VOS_UINT8 *)&ecallCfg, sizeof(ecallCfg));

        if (result != NV_OK) {
            return AT_ERROR;
        }

        AT_SetEclModeValue(ecallCfg.ecallForceMode);
    }

    return AT_OK;
}
#endif

/* AT^TRUSTCNUM设置命令处理函数 */
VOS_UINT32 AT_SetTrustCallNumPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_MtCallNumTrustList callTrustList;
    VOS_UINT32                  listIndex;
    VOS_UINT32                  result;
    errno_t                     memResult;

    /* 参数合法性检查 */
    if ((g_atParaIndex > 2) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 从NV中读取语音Trust名单列表 */
    (VOS_VOID)memset_s(&callTrustList, sizeof(callTrustList), 0, sizeof(callTrustList));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MT_CALL_NUM_TRUST_LIST, &callTrustList, sizeof(callTrustList)) != NV_OK) {
        AT_ERR_LOG("AT_SetTrustCallNumPara: Read NV Fail.");
        return AT_ERROR;
    }

    listIndex = g_atParaList[0].paraValue;
    /* 参数个数为1个时，表示读取对应索引的语音Trust名单号码 */
    if (g_atParaIndex == 1) {
        if (VOS_StrLen((VOS_CHAR *)callTrustList.recordList[listIndex].number) != 0) {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,\"%s\"", g_parseContext[indexNum].cmdElement->cmdName,
                listIndex, callTrustList.recordList[listIndex].number);
            return AT_OK;
        } else {
            AT_INFO_LOG("The call trust List record of the specified index is empty.");
            return AT_OK;
        }
    } else {
        /* 参数个数为2个时,Trust名单号码合法性检查 */
        result = AT_TrustNumParaProc(&callTrustList.recordList[listIndex]);
        if (result != AT_SUCCESS) {
            return result;
        }
        /* Trust名单号码重复性检查 */
        if (AT_CheckDeduplicationTrustNum(callTrustList.recordList, TAF_TRUSTLIST_MAX_LIST_SIZE) == VOS_TRUE) {
            return AT_OK;
        }
        /* 更新指定索引的Trust名单号码 */
        callTrustList.recordList[listIndex].index = (VOS_UINT8)g_atParaList[0].paraValue;
        (VOS_VOID)memset_s(callTrustList.recordList[listIndex].number,
                           sizeof(callTrustList.recordList[listIndex].number),
                           0,
                           sizeof(callTrustList.recordList[listIndex].number));
        memResult = memcpy_s(callTrustList.recordList[listIndex].number,
                             sizeof(callTrustList.recordList[listIndex].number),
                             g_atParaList[1].para,
                             g_atParaList[1].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(callTrustList.recordList[listIndex].number), g_atParaList[1].paraLen);
    }

    /* 更新语音Trust名单NV */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_MT_CALL_NUM_TRUST_LIST, (VOS_UINT8 *)(&callTrustList), sizeof(callTrustList)) != NV_OK) {
        AT_ERR_LOG("AT_SetTrustCallNumPara: Write NV Fail.");
        return AT_ERROR;
    }
    return AT_OK;
}

VOS_UINT32 At_SetCpasPara(VOS_UINT8 indexNum)
{
    VOS_UINT32                  ret;
    TAF_Ctrl                    ctrl;
    TAF_CALL_QryCallInfoReqPara qryCallInfoPara;
    ModemIdUint16               modemId;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&qryCallInfoPara, sizeof(qryCallInfoPara), 0x00, sizeof(qryCallInfoPara));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    qryCallInfoPara.getCallInfoType = TAF_CALL_GET_CALL_INFO_TYPE_CLCC;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发消息到C核获取当前所有通话信息 */
    ret = TAF_CCM_CallCommonReq(&ctrl, &qryCallInfoPara, ID_TAF_CCM_QRY_CALL_INFO_REQ, sizeof(qryCallInfoPara),
                                modemId);

    if (ret != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetCpasPara: MN_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPAS_SET;

    return AT_WAIT_ASYNC_RETURN;
}

