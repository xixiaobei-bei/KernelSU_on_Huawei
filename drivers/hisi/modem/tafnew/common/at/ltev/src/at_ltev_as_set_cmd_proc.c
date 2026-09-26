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
#include "at_ltev_as_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_snd_msg.h"
#include "at_data_proc.h"

#include "at_ltev_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_AS_SET_CMD_PROC_C

#if (FEATURE_LTEV == FEATURE_ON)
#define AT_CATM_PARA_MAX_NUM 2
#define AT_CCUTLE_PARA_MAX_NUM 5
#define AT_CCUTLE_TEST_MODE_CLOSE_STATUS 0
#define AT_CCUTLE_COMMUNICATION_DIRECTION 1
#define AT_CCUTLE_MONITOR_LIST_FORMAT 2
#define AT_CCUTLE_MONITOR_LIST_LEN 3
#define AT_CCUTLE_MONITOR_LIST_CONTEXT 4
#define AT_CV2XDTS_PARA_MAX_NUM 3
#define AT_CV2XDTS_SEND_DATA_ACTION 0
#define AT_CV2XDTS_SEND_DATA_SIZE 1
#define AT_CV2XDTS_SEND_DATA_PERIODICITY 2
#define AT_CATM_PARA_STATE 0 /* CATM的第一个参数STATE */
#define AT_CATM_PARA_TYPE  1 /* CATM的第二个参数TYPE */
#define AT_CV2XL2ID_PARA_NUM 2
#define AT_SLINFO_PARA_MAX_NUM 3
#define AT_SLINFO_FREQUENCY_BAND_INDEX 0
#define AT_SLINFO_BAND_WIDTHh_INDEX 1
#define AT_SLINFO_CENTRAL_FREQUENCY_INDEX 2

#define AT_PHYR_PARA_MAX_NUM 5
#define AT_PHYR_MAX_TXPWR_INDEX 0
#define AT_PHYR_MIN_MCS_INDEX 1
#define AT_PHYR_MAX_MCS_INDEX 2
#define AT_PHYR_MIN_SUBCHN_INDEX 3
#define AT_PHYR_MAX_SUBCHN_INDEX 4
#define AT_VSYNCSRCRPT_PARA_MAX_NUM 1
#define AT_VSYNCSRCRPT_PARA_STATUS  0 /* VSYNCSRCRPT的第一个参数STATUS */
#define AT_VSYNCMODE_PARA_MAX_NUM 1
#define AT_VSYNCMODE_PARA_MODE    0 /* VSYNCMODE的第一个参数MODE */
#define AT_RPPCFG_SET_PARAM_NUM 10
#define AT_RPPCFG_QRY_PARAM_NUM 2
#define AT_RPPCFG_CLEAR_RESPOOL_CMD 3
#define AT_RPPCFG_GROUP_TYPE_INDEX 0
#define AT_RPPCFG_SUB_CHAN_BITMAP_INDEX 4
#define AT_RPPCFG_BITMAP_LEN_MIN_VALUE 16
#define AT_RPPCFG_BITMAP_LEN_MID_VALUE 20
#define AT_RPPCFG_BITMAP_LEN_MAX_VALUE 100

#define AT_RPPCFG_GROUP_RESPOOL_NUM 8
#define AT_RPPCFG_GROUP_RESPOOL_NUM_SUM 16
#define AT_SET_PHYR_TX_POWER_MAX 23
#define AT_SET_PHYR_TX_POWER_MIN (-30)

#define AT_RPPCFG_RT_TYPE_INDEX 1
#define AT_RPPCFG_POOL_ID_INDEX 2
#define AT_RPPCFG_SL_OFFSET_INDEX 3
#define AT_RPPCFG_ADJACENCY_INDEX 5
#define AT_RPPCFG_SUB_CHN_SIZE_INDEX 6
#define AT_RPPCFG_SUB_CHN_NUM_INDEX 7
#define AT_RPPCFG_SUB_CHN_STARTRB_INDEX 8
#define AT_RPPCFG_PSCCH_POOL_STARTRB_INDEX 9
#define AT_VPHYSTATCLR_PARA_NUM               1
#define AT_VPHYSTATCLR_PARA_VPHY_STAT_CLR_FLG 0 /* VPHYSTATCLR的第一个参数VPHY_STAT_CLR_FLG */
#define AT_TX_POWER_PARA_LEN_MAX 7
#define AT_PC5SYNC_PARA_MAX_NUM           10
#define AT_PC5SYNC_PARA_SYNC_SWITCH          0 /* PC5SYNC的第一个参数SYNC_SWITCH */
#define AT_PC5SYNC_PARA_SYNC_DFN_SWITCH      1 /* PC5SYNC的第二个参数SYNC_DFN_SWITCH */
#define AT_PC5SYNC_PARA_GNSS_VALID_TIMER_LEN 2 /* PC5SYNC的第三个参数GNSS_VALID_TIMER_LEN */
#define AT_PC5SYNC_PARA_SYNC_OFFSET_IND1     3 /* PC5SYNC的第四个参数SYNC_OFFSET_IND1  */
#define AT_PC5SYNC_PARA_SYNC_OFFSET_IND2     4 /* PC5SYNC的第五个参数SYNC_OFFSET_IND2 */
#define AT_PC5SYNC_PARA_SYNC_OFFSET_IND3     5 /* PC5SYNC的第六个参数SYNC_OFFSET_IND3 */
#define AT_PC5SYNC_PARA_SYNC_TX_THRESH_OOC   6 /* PC5SYNC的第七个参数SYNC_TX_THRESH_OOC */
#define AT_PC5SYNC_PARA_FLT_COEFFICIENT      7 /* PC5SYNC的第八个参数FLT_COEFFICIENT */
#define AT_PC5SYNC_PARA_SYNC_REF_MIN_HYST    8 /* PC5SYNC的第九个参数SYNC_REF_MIN_HYST */
#define AT_PC5SYNC_PARA_SYNC_REF_DIFF_HYST   9 /* PC5SYNC的第十个参数SYNC_REF_DIFF_HYST */
#define AT_PC5SYNC_PARA_SYNC_SWITCH_INVALID_VAL     0 /* PC5SYNC的第一个参数SYNC_SWITCH的无效取值 */
#define AT_PC5SYNC_PARA_SYNC_DFN_SWITCH_INVALID_VAL 1 /* PC5SYNC的第二个参数SYNC_DFN_SWITCH的无效取值 */

VOS_UINT32 AT_SetCatm(VOS_UINT8 indexNum)
{
    AT_VTC_TestModeActiveStatePara para = {0};
    VOS_UINT32                     result;

    if (g_atParaIndex > AT_CATM_PARA_MAX_NUM || g_atParaList[AT_CATM_PARA_STATE].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    para.state = (AT_VTC_TestModeActiveStateUint8)g_atParaList[AT_CATM_PARA_STATE].paraValue;

    /* 激活测试模式，要求携带两个参数，第二个参数指定测试模式类型 */
    if (para.state == AT_VTC_TEST_MODE_STATE_ACTIVATED) {
        if (g_atParaList[AT_CATM_PARA_TYPE].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        para.type = (AT_VTC_TestModeTypeUint8)g_atParaList[AT_CATM_PARA_TYPE].paraValue;
    }
    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_TEST_MODE_ACTIVE_STATE_SET_REQ, (VOS_UINT8 *)&para,
                          sizeof(AT_VTC_TestModeActiveStatePara));
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CATM_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCv2xL2Id(VOS_UINT8 indexNum)
{
    AT_VTC_TestModeModifyL2IdPara para = {0};
    VOS_UINT32 result;

    if (g_atParaIndex != AT_CV2XL2ID_PARA_NUM || g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    para.srcId = g_atParaList[0].paraValue;
    para.dstId = g_atParaList[1].paraValue;

    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_LAYER_TWO_ID_SET_REQ, (VOS_UINT8 *)&para,
                          sizeof(AT_VTC_TestModeModifyL2IdPara));
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CV2XL2ID_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCcutle(VOS_UINT8 indexNum)
{
    AT_VTC_TestModeECloseStatePara para = {0};
    errno_t                        memResult;
    VOS_UINT32                     result;

    if (g_atParaIndex > AT_CCUTLE_PARA_MAX_NUM || g_atParaList[AT_CCUTLE_TEST_MODE_CLOSE_STATUS].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    para.status = (AT_VTC_TestModeCloseStatusUint8)g_atParaList[AT_CCUTLE_TEST_MODE_CLOSE_STATUS].paraValue;
    if (para.status == AT_VTC_TEST_MODE_STATUS_CLOSE) {
        if (g_atParaList[AT_CCUTLE_COMMUNICATION_DIRECTION].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        para.direction = (AT_VTC_CommunicationDirectionUint8)g_atParaList[AT_CCUTLE_COMMUNICATION_DIRECTION].paraValue;
        if (para.direction == AT_VTC_COMMUNICATION_DIRECTION_RECEIVE) {
            if (g_atParaList[AT_CCUTLE_MONITOR_LIST_FORMAT].paraLen == 0 ||
                g_atParaList[AT_CCUTLE_MONITOR_LIST_LEN].paraLen == 0) {
                return AT_CME_INCORRECT_PARAMETERS;
            }
            para.format = (AT_VTC_MonitorListFormatUint8)g_atParaList[AT_CCUTLE_MONITOR_LIST_FORMAT].paraValue;
            para.length = g_atParaList[AT_CCUTLE_MONITOR_LIST_LEN].paraValue;
            /* 第四个参数描述Layer-2 ID数量，由于每个Layer-2 ID占用6个字符，所以第四个参数和第五个参数存在6倍关系 */
            if (g_atParaList[AT_CCUTLE_MONITOR_LIST_CONTEXT].paraLen !=
                para.length * AT_CCUTLE_MONITOR_LIST_CONTEXT_TO_ID) {
                return AT_CME_INCORRECT_PARAMETERS;
            }
            memResult = memcpy_s(para.monitorList, sizeof(para.monitorList),
                                 g_atParaList[AT_CCUTLE_MONITOR_LIST_CONTEXT].para,
                                 g_atParaList[AT_CCUTLE_MONITOR_LIST_CONTEXT].paraLen);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(para.monitorList), g_atParaList[AT_CCUTLE_MONITOR_LIST_CONTEXT].paraLen);
        }
    }
    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_TEST_LOOP_MODE_E_CLOSE_STATE_SET_REQ, (VOS_UINT8 *)&para,
                          sizeof(AT_VTC_TestModeECloseStatePara));
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCUTLE_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCuspcreq(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* AT命令格式没有问号，但根据协议实际是一个查询命令 */
    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_SIDELINK_PACKET_COUNTER_QRY_REQ, VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CUSPCREQ_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCutcr(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, AT_VTC_RESET_UTC_TIME_SET_REQ,
                          VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CUTCR_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCcbrreq(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* AT命令格式没有问号，但根据协议实际是一个查询命令 */
    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, AT_VTC_CBR_QRY_REQ,
                          VOS_NULL_PTR, 0);
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCBRREQ_QRY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCv2xdts(VOS_UINT8 indexNum)
{
    AT_VTC_SendingDataActionPara para = {0};
    VOS_UINT32                   result;

    if (g_atParaIndex > AT_CV2XDTS_PARA_MAX_NUM || g_atParaList[AT_CV2XDTS_SEND_DATA_ACTION].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    para.action = (AT_VTC_SendingDataActionUint8)g_atParaList[AT_CV2XDTS_SEND_DATA_ACTION].paraValue;
    if (para.action == AT_VTC_SENDING_DATA_ACTION_START) {
        if (g_atParaList[AT_CV2XDTS_SEND_DATA_SIZE].paraLen == 0 ||
            g_atParaList[AT_CV2XDTS_SEND_DATA_PERIODICITY].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        para.dataSize    = g_atParaList[AT_CV2XDTS_SEND_DATA_SIZE].paraValue;
        para.periodicity = g_atParaList[AT_CV2XDTS_SEND_DATA_PERIODICITY].paraValue;
    }
    result = AT_SndVtcMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                          AT_VTC_SENDING_DATA_ACTION_SET_REQ, (VOS_UINT8 *)&para, sizeof(AT_VTC_SendingDataActionPara));
    if (result == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CV2XDTS_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetVsyncsrcrpt(VOS_UINT8 indexNum)
{
    AT_VRRC_SyncSourceRptSwitchPara para = {0};
    VOS_UINT32                      result;

    if (g_atParaIndex > AT_VSYNCSRCRPT_PARA_MAX_NUM || g_atParaList[AT_VSYNCSRCRPT_PARA_STATUS].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    para.status = (AT_VRRC_SyncSourceRptSwitchUint8)g_atParaList[AT_VSYNCSRCRPT_PARA_STATUS].paraValue;
    result      = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        AT_VRRC_SYNC_SOURCE_RPT_SET_REQ, (VOS_UINT8 *)&para, sizeof(para), I0_PS_PID_VERRC);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VSYNCSRCRPT_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetVsyncmode(VOS_UINT8 indexNum)
{
    AT_VRRC_SyncModePara para = {0};
    VOS_UINT32           result;

    if (g_atParaIndex > AT_VSYNCMODE_PARA_MAX_NUM || g_atParaList[AT_VSYNCMODE_PARA_MODE].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
    para.mode = (AT_VRRC_SyncModeUint8)g_atParaList[AT_VSYNCMODE_PARA_MODE].paraValue;
    result    = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        AT_VRRC_SYNC_MODE_SET_REQ, (VOS_UINT8 *)&para, sizeof(para), I0_PS_PID_VERRC);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VSYNCMODE_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 VRRC_SetGnssInfo(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32      rslt;
    AT_VRRC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VRRC_SetGnssInfo: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_GNSS_TEST_START_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_SetGnssInfo: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetGnssInfo(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VRRC_SetGnssInfo(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_GNSS_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_WARN_LOG("AT_QryGnssInfo AT_ERROR\n");
        return AT_ERROR;
    }
}

VOS_UINT32 VRRC_GetGnssInfo(MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId)
{
    /* 发送消息给VRRC模块 */
    VOS_UINT32      rslt;
    AT_VRRC_ReqMsg *msg = VOS_NULL_PTR;

    /* 申请消息 */
    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("VRRC_GetGnssInfo: ALLOC MSG FAIL.");
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH, 0x00,
                       sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_GNSS_INFO_GET_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);
    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_GetGnssInfo: SEND MSG FAIL.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_GnssInfo(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (VRRC_GetGnssInfo(g_atClientTab[indexNum].clientId, 0) == VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_GNSSINFO_QRY;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_WARN_LOG("AT_GnssInfo AT_ERROR\n");
        return AT_ERROR;
    }
}

VOS_UINT32 VRRC_SetRsuVPhyStatClr(VOS_UINT32 ulSenderPid, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                                  const AT_VRRC_VphyStatClrSetReq *setParams)
{
    /* 发送给VRRC模块的消息 */
    AT_VRRC_ReqMsg            *msg    = VOS_NULL_PTR;
    AT_VRRC_VphyStatClrSetReq *params = VOS_NULL_PTR;
    VOS_UINT32                 rslt;
    VOS_UINT32                 length;
    errno_t                    memResult;

    /* 申请消息内存空间 */
    length = sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH - sizeof(VOS_UINT32) + sizeof(AT_VRRC_VphyStatClrSetReq);

    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, length, 0x00, length);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_RSU_VPHYSTAT_CLR_SET_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    params    = (AT_VRRC_VphyStatClrSetReq *)msg->content;
    memResult = memcpy_s(params, sizeof(AT_VRRC_VphyStatClrSetReq), setParams, sizeof(AT_VRRC_VphyStatClrSetReq));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(AT_VRRC_VphyStatClrSetReq), sizeof(AT_VRRC_VphyStatClrSetReq));

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_SetRsuVPhyStatClr:SEND MSG FAIL");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetVPhyStatClr(VOS_UINT8 indexNum)
{
    VOS_UINT32                rst;
    AT_VRRC_VphyStatClrSetReq params;
    errno_t                   memResult;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != AT_VPHYSTATCLR_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    memResult = memset_s(&params, sizeof(params), 0, sizeof(AT_VRRC_VphyStatClrSetReq));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(params), sizeof(AT_VRRC_VphyStatClrSetReq));
    params.vphyStatClrFlg = (VOS_UINT8)g_atParaList[AT_VPHYSTATCLR_PARA_VPHY_STAT_CLR_FLG].paraValue;
    rst = VRRC_SetRsuVPhyStatClr(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &params);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VPHYSTATCLR_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 VRRC_SetTxPower(VOS_UINT32 ulSenderPid, MN_CLIENT_ID_T clientId, MN_OPERATION_ID_T opId,
                           const AT_VRRC_SetTxPowerReq *setParams)
{
    /* 发送给VRRC模块的消息 */
    AT_VRRC_ReqMsg           *msg    = VOS_NULL_PTR;
    AT_VRRC_SetTxPowerReq    *params = VOS_NULL_PTR;
    VOS_UINT32                rslt;
    VOS_UINT32                length;
    errno_t                   memResult;

    /* 申请消息内存空间 */
    length = sizeof(AT_VRRC_ReqMsg) - VOS_MSG_HEAD_LENGTH - sizeof(VOS_UINT32) + sizeof(AT_VRRC_SetTxPowerReq);

    msg = (AT_VRRC_ReqMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    (VOS_VOID)memset_s((VOS_UINT8 *)msg + VOS_MSG_HEAD_LENGTH, length, 0x00, length);

    msg->clientId      = clientId;
    msg->opId          = opId;
    msg->msgName       = AT_VRRC_SET_TX_POWER_REQ;
    msg->ulSenderPid   = WUEPS_PID_AT;
    msg->ulReceiverPid = AT_GetDestPid(clientId, I0_PS_PID_VERRC);

    params    = (AT_VRRC_SetTxPowerReq *)msg->content;
    memResult = memcpy_s(params, sizeof(AT_VRRC_SetTxPowerReq), setParams, sizeof(AT_VRRC_SetTxPowerReq));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(AT_VRRC_SetTxPowerReq), sizeof(AT_VRRC_SetTxPowerReq));

    rslt = TAF_TraceAndSendMsg(WUEPS_PID_AT, msg);

    if (rslt != VOS_OK) {
        AT_ERR_LOG("VRRC_SetRsuPhyr:SEND MSG FAIL");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_GetValidTxPowerParam(VOS_INT16 * dbmNum)
{
    VOS_UINT32 cellPower = 0;

    /* 功率长度大于7或参数为空，表示无效参数 */
    if ((g_atParaList[0].paraLen > AT_TX_POWER_PARA_LEN_MAX) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串参数为'-'，表示输入为负数 */
    if (g_atParaList[0].para[0] == '-') {
        if (atAuc2ul(&g_atParaList[0].para[1], g_atParaList[0].paraLen - 1, &cellPower) != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 由于功率值为负，所以转换下 */
        *dbmNum = -(VOS_INT16)cellPower;
    }else {
        if (atAuc2ul(&g_atParaList[0].para[0], g_atParaList[0].paraLen, &cellPower) != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        *dbmNum = (VOS_INT16)cellPower;
    }

    /* LTE-V输出功率范围为[-62dBm, 30dBm], 按照0.125dBm精度控制，dbmNum的范围为[-496, 240] */
    if ((*dbmNum > AT_SET_TX_POWER_MAX) || (*dbmNum < AT_SET_TX_POWER_MIN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_GetValidPowerPara(AT_VRRC_SetTxPowerReq * powerSetPara)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数量错误 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 从AT命令中获取输出功率 */
    if (AT_GetValidTxPowerParam(&(powerSetPara->setTxPower)) != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetVTxPower(VOS_UINT8 indexNum)
{
    VOS_UINT32                          rst;
    AT_VRRC_SetTxPowerReq               params;
    errno_t                             memResult;

    memResult = memset_s(&params, sizeof(params), 0, sizeof(AT_VRRC_SetTxPowerReq));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(params), sizeof(AT_VRRC_SetTxPowerReq));

    /* 取出AT命令中的数值，正数负数都有 */
    rst = AT_GetValidPowerPara(&params);
    if (rst != AT_OK) {
        AT_WARN_LOG("AT_SetVTxPower INCORRECT_PARAMETERS\n");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = VRRC_SetTxPower(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &params);
    if (rst != VOS_OK) {
        AT_WARN_LOG("AT_SetVTxPower AT_ERROR\n");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VTXPOWER_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

#endif

