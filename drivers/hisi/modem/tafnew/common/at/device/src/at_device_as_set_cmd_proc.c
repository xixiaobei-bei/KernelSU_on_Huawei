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
#include "at_device_as_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "at_ctx.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "nv_stru_lps.h"
#endif
#include "nv_stru_gas.h"

#include "at_mt_msg_proc.h"
#include "at_device_comm.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_AS_SET_CMD_PROC_C

#define AT_TRXTAS_RAT_MODE 1
#define AT_TRXTAS_MODE 0
#define AT_TRXTAS_CMD 2
#define AT_TRXTAS_DPDT_VALUE 3
#define AT_TRXTAS_PARA_NUM 3
#define AT_TRXTAS_CMD_SET 1

VOS_UINT32 At_WriteDpaCatToNV(VOS_UINT8 dpaRate)
{
    AT_NvimUeCapa uECapa;
    AT_DpacatPara dhpaCategory[AT_DPACAT_CATEGORY_TYPE_BUTT] = {
        /* 支持速率等级3.6M  */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_6, PS_FALSE, 0, PS_FALSE },
        /* 支持速率等级7.2M  */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_8, PS_FALSE, 0, PS_FALSE },
        /* 支持速率等级1.8M  */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_11, PS_FALSE, 0, PS_FALSE },
        /*  支持速率等级14.4M */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_10, PS_FALSE, 0, PS_FALSE },
        /*  支持速率等级21M */
        { PS_TRUE, AT_HSDSCH_PHY_CATEGORY_10, PS_TRUE, AT_HSDSCH_PHY_CATEGORY_14, PS_FALSE }
    };

    (VOS_VOID)memset_s(&uECapa, sizeof(uECapa), 0x00, sizeof(uECapa));

    if (dpaRate >= AT_DPACAT_CATEGORY_TYPE_BUTT) {
        AT_WARN_LOG("At_WriteDpaCatToNV: WARNING:Input DPA rate index invalid!");
        return VOS_ERR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, &uECapa, sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("At_WriteDpaCatToNV: en_NV_Item_WAS_RadioAccess_Capa NV Read Fail!");
        return VOS_ERR;
    }

    /* 是否支持enHSDSCHSupport的标志                */
    uECapa.hsdschSupport = dhpaCategory[dpaRate].hsdschSupport;

    /* 支持HS-DSCH物理层的类型标志                  */
    uECapa.hsdschPhyCategory    = dhpaCategory[dpaRate].hsdschPhyCategory;
    uECapa.macEhsSupport        = dhpaCategory[dpaRate].macEhsSupport;
    uECapa.hsdschPhyCategoryExt = dhpaCategory[dpaRate].hsdschPhyCategoryExt;

    /* 是否支持 Multi cell support,如果支持MultiCell,Ex2存在 */
    uECapa.multiCellSupport = dhpaCategory[dpaRate].multiCellSupport;

    /* 将新的能力写到内部的全局变量，并更新到NV项中 */
    uECapa.hspaStatus = NV_ITEM_ACTIVE; /* 此项成为激活项 */

    /* 已经设置好NV结构体中的对应值，将这些值写入NV */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, (VOS_UINT8 *)&uECapa,
                           sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("At_WriteDpaCatToNV: en_NV_Item_WAS_RadioAccess_Capa NV Write Fail!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_WriteRrcVerToNV(VOS_UINT8 srcWcdmaRRC)
{
    AT_NvimUeCapa uECapa;

    /* 从NV项中读取en_NV_Item_WAS_RadioAccess_Capa值 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, &uECapa, sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("AT_WriteRrcVerToNV():NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW NV Read Fail!");
        return VOS_ERR;
    }

    switch (srcWcdmaRRC) {
        case AT_RRC_VERSION_WCDMA:
            uECapa.edchSupport   = VOS_FALSE;
            uECapa.hsdschSupport = VOS_FALSE;
            break;

        case AT_RRC_VERSION_DPA:
            uECapa.edchSupport   = VOS_FALSE;
            uECapa.hsdschSupport = VOS_TRUE;
            break;

        case AT_RRC_VERSION_DPA_AND_UPA:
            uECapa.asRelIndicator = AT_PTL_VER_ENUM_R6;
            uECapa.edchSupport    = VOS_TRUE;
            uECapa.hsdschSupport  = VOS_TRUE;
            break;

        case AT_RRC_VERSION_HSPA_PLUNS:
            uECapa.asRelIndicator = AT_PTL_VER_ENUM_R7;
            uECapa.edchSupport    = VOS_TRUE;
            uECapa.hsdschSupport  = VOS_TRUE;
            break;

        default:
            break;
    }

    /* 将要设置的值写入Balong NV 结构体 */
    uECapa.hspaStatus = NV_ITEM_ACTIVE;

    /* 已经设置好NV结构体中的对应值，将这些值写入NV */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW, (VOS_UINT8 *)&uECapa,
                           sizeof(AT_NvimUeCapa)) != NV_OK) {
        AT_WARN_LOG("AT_WriteRrcVerToNV():NV_ITEM_WAS_RADIO_ACCESS_CAPA_NEW NV Write Fail!");
        return VOS_ERR;
    }

    return VOS_OK;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 At_SetDpaCat(VOS_UINT8 indexNum)
{
    /*
     * 0     支持速率为3.6M
     * *1     支持速率为7.2M
     * *2     支持速率为1.8M
     * *3     支持速率为14.4M
     * *4     支持速率为21M
     */
    VOS_UINT8 dpaRate;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数不为1 */
    if (g_atParaIndex != 1) {
        return AT_ERROR;
    }

    /* 参数内容只能为1个 */
    if (g_atParaList[0].paraLen != 1) {
        return AT_ERROR;
    }

    dpaRate = g_atParaList[0].para[0];
    /* ucDpaRate取值范围为'0'~'4' */
    if ((dpaRate >= '0') && (dpaRate <= '4')) {
        dpaRate = g_atParaList[0].para[0] - '0';
    } else {
        return AT_ERROR;
    }

    if (g_atDataLocked == VOS_TRUE) {
        return AT_ERROR;
    }

    /* 调用写NV接口函数: At_WriteDpaCatToNV,返回操作结果 */
    if (At_WriteDpaCatToNV(dpaRate) == VOS_OK) {
        return AT_OK;
    } else {
        AT_WARN_LOG("At_SetDpaCat:WARNING:WAS_MNTN_SetDpaCat failed!");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetHsspt(VOS_UINT8 indexNum)
{
    VOS_UINT8 rRCVer;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_DPAUPA_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_DPAUPA_ERROR;
    }

    /* 检查是否解除数据保护,未解除时返回出错信息:ErrCode:0 */
    if (g_atDataLocked == VOS_TRUE) {
        return AT_DATA_UNLOCK_ERROR;
    }

    rRCVer = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 调用写NV接口函数: AT_WriteRrcVerToNV,返回操作结果 */
    if (AT_WriteRrcVerToNV(rRCVer) == VOS_OK) {
        return AT_OK;
    } else {
        AT_WARN_LOG("At_SetHsspt:WARNING:Write NV failed!");
        return AT_DPAUPA_ERROR;
    }
}
#endif

VOS_UINT32 AT_SetGTimerPara(VOS_UINT8 indexNum)
{
    /* 参数个数错误，返回AT_DEVICE_OTHER_ERROR，上报CME ERROR:1 */
    if (g_atParaIndex != 1) {
        AT_NORM_LOG("AT_SetGTimerPara: the number of parameter is error.");
        return AT_ERROR;
    }

    /* 数据保护未解锁，返回AT_DATA_UNLOCK_ERROR，上报CME ERROR:0 */
    if (g_atDataLocked != VOS_FALSE) {
        AT_NORM_LOG("AT_SetGTimerPara: data locked.");
        return AT_ERROR;
    }

    if (NV_OK != TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_GPRS_ACTIVE_TIMER_LEN, (VOS_UINT8 *)&g_atParaList[0].paraValue,
                                    sizeof(g_atParaList[0].paraValue))) {
        AT_WARN_LOG("AT_SetGTimerPara: Fail to write NV_ITEM_GPRS_ACTIVE_TIMER_LEN.");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_SetCnmrPara(VOS_UINT8 indexNum)
{
    VOS_UINT32       rst;
    AT_MTA_QryNmrReq qryNmrReq;

    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度过长 */
    if (g_atParaList[0].paraLen != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 查询4g的nmr数据直接返回失败 */
    if (g_atParaList[0].paraValue == AT_CNMR_QRY_LTE_NMR_DATA) {
        return AT_ERROR;
    }

    /* 发送消息DRV_AGENT_AS_QRY_NMR_REQ给C核处理 */
    (VOS_VOID)memset_s(&qryNmrReq, sizeof(qryNmrReq), 0x00, sizeof(qryNmrReq));

    if (g_atParaList[0].paraValue == AT_CNMR_QRY_WAS_NMR_DATA) {
        qryNmrReq.ratType = 0;
    } else {
        qryNmrReq.ratType = 1;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, ID_AT_MTA_QRY_NMR_REQ,
                                 (VOS_UINT8 *)&qryNmrReq, sizeof(qryNmrReq), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNMR_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 At_SetLWThresholdCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32              ret;
    L4A_LW_ThreasholdCfgReq req            = {0};
    LPS_SWITCH_PARA_STRU    drxControlFlag = {0};

    req.ctrl.clientId = g_atClientTab[indexNum].clientId;
    req.ctrl.opId     = 0;
    req.ctrl.pid      = WUEPS_PID_AT;
    req.flag          = g_atParaList[0].paraValue;

    if (TAF_ACORE_NV_READ(MODEM_ID_0, EN_NV_ID_SWITCH_PARA, &drxControlFlag, sizeof(LPS_SWITCH_PARA_STRU)) != NV_OK) {
        return AT_ERROR;
    }

    if (((*((VOS_UINT32 *)(&(drxControlFlag.stPsFunFlag01)))) & LPS_NV_JP_DCOM_REL_OFFSET_BIT) == 0) {
        return AT_OK;
    }

    /* L模调用如下接口转发 */
    ret = atSendL4aDataMsg(g_atClientTab[indexNum].clientId, I0_MSP_L4_L4A_PID, ID_MSG_L4A_LW_THRESHOLD_REQ,
                           (VOS_UINT8 *)(&req), sizeof(L4A_LW_ThreasholdCfgReq));

    if (ret == ERR_MSP_SUCCESS) {
        return AT_OK;
    }
    return AT_ERROR;
}

VOS_UINT32 At_SetWlthresholdcfgPara(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_W == FEATURE_ON)
    VOS_UINT32                  result;
#endif
    AT_MTA_ReselOffsetCfgSetNtf reselOffsetCfgNtf;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&reselOffsetCfgNtf, sizeof(reselOffsetCfgNtf), 0x00, sizeof(reselOffsetCfgNtf));

    reselOffsetCfgNtf.offsetFlg = (VOS_UINT8)g_atParaList[0].paraValue;

#if (FEATURE_UE_MODE_W == FEATURE_ON)
    /* 发送消息AT_MTA_RESEL_OFFSET_CFG_SET_REQ_STRU */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_RESEL_OFFSET_CFG_SET_NTF, (VOS_UINT8 *)&reselOffsetCfgNtf,
                                    sizeof(reselOffsetCfgNtf), I0_UEPS_PID_MTA);
#endif

#if (FEATURE_LTE == FEATURE_ON)
    /* L使用参数 */
    At_SetLWThresholdCfgPara(indexNum);
#endif

#if (FEATURE_UE_MODE_W == FEATURE_ON)
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_SetWlthresholdcfgPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
#endif
    return AT_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_CheckTrxTasFtmPara(VOS_VOID)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();
    if (atMtInfoCtx->atInfo.txOnOff != AT_DSP_RF_SWITCH_ON) {
        return AT_TRXTAS_TX_NOT_SWITCH_ON;
    }

    /* 非信令下TRXTAS只有设置 */
    if (g_atParaList[AT_TRXTAS_CMD].paraValue != AT_TRXTAS_CMD_SET) {
        return AT_TRXTAS_CMD_PARA_ERR;
    }

    /* 记录测试接入技术模式信息 */
    if (At_CovertRatModeToBbicCal((AT_CmdRatmodeUint8)g_atParaList[AT_TRXTAS_RAT_MODE].paraValue,
                                  &atMtInfoCtx->bbicInfo.trxTasRatMode) == VOS_FALSE) {
        return AT_TRXTAS_RAT_ERR;
    }

    /* 判断^TRXTAS命令与^Fchan时传入的RAT是否一致 */
    if (atMtInfoCtx->bbicInfo.trxTasRatMode != atMtInfoCtx->bbicInfo.currtRatMode) {
        return AT_TRXTAS_FCHAN_RAT_NOT_MATCH;
    }

    return AT_SUCCESS;
}

VOS_UINT32 At_SetTrxTasPara(VOS_UINT8 indexNum)
{
    AT_MT_Info         *atMtInfoCtx = VOS_NULL_PTR;
    AT_MTA_SetTrxTasReq setTrxTasCmd;
    VOS_UINT32          rst;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_SetTrxTasPara Enter");

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (g_atParaIndex < AT_TRXTAS_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 若是设置命令，则需要下发第四个参数 */
    if ((g_atParaList[AT_TRXTAS_CMD].paraValue == 1) && (g_atParaList[AT_TRXTAS_DPDT_VALUE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 信令模式 */
    if (g_atParaList[AT_TRXTAS_MODE].paraValue == AT_MTA_CMD_SIGNALING_MODE) {
        /* AT发送至MTA的消息结构赋值 */
        (VOS_VOID)memset_s(&setTrxTasCmd, sizeof(setTrxTasCmd), 0x00, sizeof(setTrxTasCmd));
        setTrxTasCmd.ratMode     = (AT_MTA_CmdRatmodeUint8 )g_atParaList[AT_TRXTAS_RAT_MODE].paraValue;
        setTrxTasCmd.cmd         = (AT_MTA_TrxTasCmdUint8 )g_atParaList[AT_TRXTAS_CMD].paraValue;
        setTrxTasCmd.mode        = (AT_MTA_CmdSignalingUint8 )g_atParaList[AT_TRXTAS_MODE].paraValue;
        setTrxTasCmd.trxTasValue = g_atParaList[AT_TRXTAS_DPDT_VALUE].paraValue;

        /* 发送消息给C核处理 */
        rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_TRX_TAS_REQ,
                                     (VOS_UINT8 *)&setTrxTasCmd, sizeof(setTrxTasCmd), I0_UEPS_PID_MTA);
    }
    /* 非信令模式 */
    else {
        rst = At_CheckTrxTasFtmPara();
        if (rst != AT_SUCCESS) {
            return rst;
        }
        atMtInfoCtx->atInfo.indexNum = indexNum;
        rst = At_SndBbicCalSetTrxTasReq((VOS_UINT16)g_atParaList[AT_TRXTAS_DPDT_VALUE].paraValue);
    }

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TRX_TAS_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
#else
VOS_UINT32 At_SndBbicCalSetTrxTasReq(VOS_UINT16 trxTasValue)
{
    BBIC_CAL_SetTrxTasReqStru *msg         = VOS_NULL_PTR;
    AT_MT_Info                *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                 length;

    atMtInfoCtx = AT_GetMtInfoCtx();
    length      = sizeof(BBIC_CAL_SetTrxTasReqStru) - VOS_MSG_HEAD_LENGTH;
    msg         = (BBIC_CAL_SetTrxTasReqStru *)PS_ALLOC_MSG(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("At_SndBbicCalSetTrxTasReq: alloc msg fail!");
        return AT_FAILURE;
    }

    AT_MT_CLR_MSG_ENTITY(msg);
    AT_CFG_MT_MSG_HDR(msg, DSP_PID_BBA_CAL, ID_AT_BBIC_TRX_TAS_REQ);

    msg->data.band        = atMtInfoCtx->bbicInfo.dspBandFreq.dspBand;
    msg->data.ratMode     = atMtInfoCtx->bbicInfo.trxTasRatMode;
    msg->data.trxTasValue = trxTasValue;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_ERR_LOG("At_SndBbicCalSetTrxTasReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("At_SndBbicCalSetTrxTasReq Exit");

    return AT_SUCCESS;
}
#endif

