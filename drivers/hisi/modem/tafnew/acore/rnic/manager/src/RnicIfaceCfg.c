/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/

#include "PsCommonDef.h"
#include "RnicMsgProc.h"
#include "RnicEntity.h"
#include "RnicIfaceOndemand.h"
#include "RnicLog.h"
#include "BastetRnicInterface.h"
#include "rnic_dev_i.h"
#include "rnic_ondemand_i.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID                 PS_FILE_ID_RNIC_IFACE_CFG_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_VOID RNIC_IFACE_SetNapiCfg(RNIC_DEV_ID_ENUM_UINT8 enRmNetId)
{
    struct rnic_napi_config_s           stNapiCfg;

    stNapiCfg.devid             = enRmNetId;
    stNapiCfg.napi_enable       = RNIC_GET_NAPI_FEATURE(enRmNetId);
    stNapiCfg.gro_enable        = RNIC_GET_GRO_FEATURE(enRmNetId);
    stNapiCfg.napi_weight       = RNIC_GET_NAPI_WEIGHT(enRmNetId);
    stNapiCfg.napi_queue_length = RNIC_GET_NAPI_POLL_QUE_MAX_LEN(enRmNetId);

    rnic_set_napi_config(&stNapiCfg);
    return;
}


VOS_VOID RNIC_IFACE_SetNapiWeight(
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId,
    VOS_UINT8                           ucWeight
)
{
    if (ucWeight != RNIC_GET_NAPI_WEIGHT(enRmNetId))
    {
        rnic_set_napi_weight(enRmNetId, ucWeight);
        RNIC_SET_NAPI_WEIGHT(enRmNetId, ucWeight);
    }

    return;
}


VOS_UINT8 RNIC_IFACE_CalcNapiWeight(VOS_UINT8 ucRmNetId)
{
    VOS_UINT8                           ucWeight;
    VOS_UINT32                          ulDlNapiRecvPktNum;

    ucWeight           = RNIC_NAPI_POLL_DEFAULT_WEIGHT;
    ulDlNapiRecvPktNum = RNIC_GET_IFACE_PERIOD_RECV_PKT(ucRmNetId);

    /* 分为4档:2/4/8/16
     * 150M bps每秒收包数在12300左右，小于这个收包数，weight值设置为2。
     * 300M bps每秒收包数在24600左右，在[12300,24600]这个范围内，weight值设置为4。
     * 450M bps每秒收包数在38400左右，在[24600,38400]这个范围内，weight值设置为8。
     * 600M bps每秒收包数在49200左右，大于38400，weight值统一设置为16。
     */

    if (ulDlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL1(ucRmNetId))
    {
        ucWeight = RNIC_GET_NAPI_WEIGHT_LEVEL1(ucRmNetId);
    }
    else if ((ulDlNapiRecvPktNum > RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL1(ucRmNetId))
          && (ulDlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL2(ucRmNetId)))
    {
        ucWeight = RNIC_GET_NAPI_WEIGHT_LEVEL2(ucRmNetId);
    }
    else if ((ulDlNapiRecvPktNum > RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL1(ucRmNetId))
          && (ulDlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL3(ucRmNetId)))
    {
        ucWeight = RNIC_GET_NAPI_WEIGHT_LEVEL3(ucRmNetId);
    }
    else
    {
        ucWeight = RNIC_GET_NAPI_WEIGHT_LEVEL4(ucRmNetId);
    }

    return ucWeight;
}

VOS_VOID RNIC_IFACE_AdjustNapiWeight(VOS_UINT8 ucRmNetId)
{
    VOS_UINT8                           ucWeight;

    /* NAPI Weight动态调整模式，才去计算weight值并调整。其他情况默认采用静态default weight值。 */
    if (NAPI_WEIGHT_ADJ_DYNAMIC_MODE == RNIC_GET_NAPI_WEIGHT_ADJ_MODE(ucRmNetId))
    {
        ucWeight = RNIC_IFACE_CalcNapiWeight(ucRmNetId);
        RNIC_IFACE_SetNapiWeight(ucRmNetId, ucWeight);
    }

    return;
}


VOS_VOID RNIC_IFACE_NapiSchedule(VOS_UINT32 ulUsrData)
{
    rnic_rx_complete((VOS_UINT8)ulUsrData);
    return;
}


VOS_INT RNIC_IFACE_RegRxHandler(
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx,
    VOS_UINT8                           ucIpFamilyMask
)
{
    VOS_UINT8                           ucExRabId = RNIC_RAB_ID_INVALID;

    if (RNIC_BIT8_IS_SET(ucIpFamilyMask, RNIC_IPV4_ADDR))
    {
        ucExRabId = pstIfaceCtx->stPsIfaceInfo.ucIpv4ExRabId;
    }

    if (RNIC_BIT8_IS_SET(ucIpFamilyMask, RNIC_IPV6_ADDR))
    {
        ucExRabId = pstIfaceCtx->stPsIfaceInfo.ucIpv6ExRabId;
    }

    if (RNIC_PS_RAT_TYPE_3GPP == pstIfaceCtx->stPsIfaceInfo.enRatType)
    {
        /* 注册下行发送函数，ADS调用注册的函数发送下行数据 */
        (VOS_VOID)ADS_DL_RegDlDataCallback(ucExRabId,
                                 RNIC_DataRxProc,
                                 pstIfaceCtx->enRmNetId);

        /* NAPI特性打开才执行 */
        if (RNIC_FEATURE_ON == RNIC_GET_NAPI_FEATURE(pstIfaceCtx->enRmNetId))
        {
            /* 注册下行NAPI处理的回调函数 */
            (VOS_VOID)ADS_DL_RegNapiCallback(ucExRabId,
                                   RNIC_IFACE_NapiSchedule,
                                   pstIfaceCtx->enRmNetId);
        }
    }

    return VOS_OK;
}


VOS_VOID RNIC_IFACE_SetNetDevUp(
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx,
    VOS_UINT8                           ucIpFamily
)
{
    struct rnic_ps_iface_config_s       stPsIfaceCfg;

    TAF_MEM_SET_S(&stPsIfaceCfg, sizeof(stPsIfaceCfg),
                  0x00, sizeof(struct rnic_ps_iface_config_s));

    stPsIfaceCfg.devid     = pstIfaceCtx->enRmNetId;
    stPsIfaceCfg.ip_family = ucIpFamily;

    /* 向网卡驱动挂载上行收包回调 */
    if (RNIC_PS_RAT_TYPE_IWLAN == pstIfaceCtx->stPsIfaceInfo.enRatType)
    {
        stPsIfaceCfg.data_tx_func = RNIC_VoWifiDataTxProc;
    }
    else
    {
        stPsIfaceCfg.data_tx_func = (RNIC_IPV4_ADDR == ucIpFamily) ?
                                    RNIC_V4DataTxProc : RNIC_V6DataTxProc;
    }

    if (0 != rnic_set_ps_iface_up(&stPsIfaceCfg))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetDevUp: set rnic_set_ps_iface_up fail!");
    }

    return;
}


VOS_VOID RNIC_IFACE_SetNetDevDown(
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx,
    VOS_UINT8                           ucIpFamily
)
{
    struct rnic_ps_iface_config_s       stPsIfaceCfg;

    TAF_MEM_SET_S(&stPsIfaceCfg, sizeof(stPsIfaceCfg),
                  0x00, sizeof(struct rnic_ps_iface_config_s));

    stPsIfaceCfg.devid        = pstIfaceCtx->enRmNetId;
    stPsIfaceCfg.ip_family    = ucIpFamily;
    stPsIfaceCfg.data_tx_func = VOS_NULL_PTR;

    if (0 != rnic_set_ps_iface_down(&stPsIfaceCfg))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC,
            "RNIC_IFACE_SetDevDown: set rnic_set_ps_iface_down fail!");
    }

    return;
}




VOS_VOID RNIC_IFACE_SetFeatureCfg(VOS_VOID)
{
    VOS_UINT32                          ulIndex;

    for (ulIndex = 0 ; ulIndex < RNIC_NET_ID_MAX_NUM ; ulIndex++)
    {
        /* 设置NAPI特性 */
        RNIC_IFACE_SetNapiCfg((VOS_UINT8)ulIndex);

    }

    return;
}

VOS_VOID RNIC_IFACE_SetDsFlowStats(VOS_UINT8 ucRmNetId)
{
    struct rnic_dsflow_stats_s         *pstDsflowStats = VOS_NULL_PTR;

    /* 调用驱动接口获取流量信息 */
    pstDsflowStats = rnic_get_dsflow_stats(ucRmNetId);

    if (VOS_NULL_PTR == pstDsflowStats)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetDsFlowStats fail, get dsflow stats failed !");
        return;
    }

    RNIC_SET_IFACE_CUR_RECV_RATE(ucRmNetId, pstDsflowStats->rx_bytes);
    RNIC_SET_IFACE_CUR_SEND_RATE(ucRmNetId, pstDsflowStats->tx_bytes);
    RNIC_SET_IFACE_PERIOD_RECV_PKT(ucRmNetId, pstDsflowStats->rx_packets);

    /* 累加上行发送报文，按需断开定时器超时清零 */
    RNIC_ADD_IFACE_PERIOD_SEND_PKT(ucRmNetId, pstDsflowStats->tx_packets);

    RNIC_NORMAL_LOG4(ACPU_PID_RNIC, "RNIC_IFACE_SetDsFlowStats: <rx_bytes, tx_bytes, rx_packets, tx_packets> ",
                    pstDsflowStats->rx_bytes,
                    pstDsflowStats->tx_bytes,
                    pstDsflowStats->rx_packets,
                    pstDsflowStats->tx_packets);

    return;
}


VOS_VOID RNIC_IFACE_ClearDsFlowFlux(VOS_UINT8 ucRmNetId)
{
    rnic_clear_dsflow_stats(ucRmNetId);
    return;
}


VOS_VOID RNIC_IFACE_ClearDsFlowStats(VOS_UINT8 ucRmNetId)
{
    RNIC_IFACE_CTX_STRU                 *pstIfaceCtx = VOS_NULL_PTR;

    pstIfaceCtx = RNIC_GET_IFACE_CTX_ADR(ucRmNetId);

    pstIfaceCtx->stDsFlowStats.ulCurrentRecvRate   = 0;
    pstIfaceCtx->stDsFlowStats.ulPeriodRecvPktNum  = 0;

    pstIfaceCtx->stDsFlowStats.ulCurrentSendRate   = 0;
    pstIfaceCtx->stDsFlowStats.ulPeriodSendPktNum  = 0;

    return;
}


VOS_VOID RNIC_IFACE_NetdevNotify(VOS_VOID)
{
    RNIC_DEV_READY_STRU                *pstSndMsg;

    /* 内存分配 */
    pstSndMsg = (RNIC_DEV_READY_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_RNIC, sizeof(RNIC_DEV_READY_STRU));

    if (VOS_NULL_PTR == pstSndMsg)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendNetdevReadyInd: Malloc failed!");
        return;
    }

    /* 填写消息 */
    RNIC_CFG_INTRA_MSG_HDR(pstSndMsg, ID_RNIC_NETDEV_READY_IND);

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstSndMsg))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendNetdevReadyInd: Send msg failed!");
    }

    return;
}


VOS_VOID RNIC_IFACE_RegDevNotifyCB(VOS_VOID)
{
    struct rnic_deivce_notifier_s       stDevNotifier;

    stDevNotifier.dev_notifier_func = RNIC_IFACE_NetdevNotify;

    rnic_register_device_notifier(&stDevNotifier);
    return;
}


VOS_VOID RNIC_IFACE_RegOnDemandOpsCB(VOS_VOID)
{
    struct rnic_ondemand_cb_ops_s       stOndemandOpsCB;

    stOndemandOpsCB.dial_mode_cfg_func    = RNIC_IFACE_OndemandDialModeProc;
    stOndemandOpsCB.idle_time_cfg_func    = RNIC_IFACE_OndemandIdleTimeProc;
    stOndemandOpsCB.event_report_cfg_func = RNIC_IFACE_OndemandEventReportProc;

    rnic_register_ondemand_ops(&stOndemandOpsCB);
    return;
}


VOS_VOID RNIC_IFACE_RegTxDropCB(VOS_UINT8 ucRmNetId)
{
    struct rnic_ps_iface_drop_notifier_s    stDropNotify;

    /* 网卡0上面上行报文触发按需拨号 */
    if (ucRmNetId == RNIC_DEV_ID_RMNET0)
    {
        stDropNotify.devid              = ucRmNetId;
        stDropNotify.drop_notifier_func = RNIC_TxDropProc;

        rnic_register_ps_iface_drop_notifier(&stDropNotify);
    }

    return;
}


VOS_VOID RNIC_IFACE_ResetPsIface(VOS_UINT8 ucRmNetId)
{
    struct rnic_ps_iface_config_s       stPsIfaceCfg;

    stPsIfaceCfg.devid        = ucRmNetId;
    stPsIfaceCfg.data_tx_func = VOS_NULL_PTR;

    /* Down V4 网卡 */
    stPsIfaceCfg.ip_family    = RNIC_IPV4_ADDR;
    rnic_set_ps_iface_down(&stPsIfaceCfg);

    /* Down V6 网卡 */
    stPsIfaceCfg.ip_family    = RNIC_IPV6_ADDR;
    rnic_set_ps_iface_down(&stPsIfaceCfg);

    return;
}


VOS_VOID RNIC_IFACE_PsIfaceUp(
    VOS_UINT8                           ucRmNetId,
    VOS_UINT8                           ucIpFamilyMask
)
{
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx    = VOS_NULL_PTR;
    RNIC_PS_IFACE_INFO_STRU            *pstPsIfaceInfo = VOS_NULL_PTR;
    RNIC_TIMER_ID_ENUM_UINT16           enDsFlowTimerId;

    pstIfaceCtx    = RNIC_GET_IFACE_CTX_ADR(ucRmNetId);
    pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(ucRmNetId);

    RNIC_NORMAL_LOG3(ACPU_PID_RNIC,
        "RNIC_IFACE_PsIfaceUp: <ucRmNetId, ucIpFamilyMask, ucRatType>",
        ucRmNetId, ucIpFamilyMask, pstPsIfaceInfo->enRatType);

    if (RNIC_BIT8_IS_SET(ucIpFamilyMask, RNIC_IPV4_ADDR))
    {
        RNIC_IFACE_SetNetDevUp(pstIfaceCtx, RNIC_IPV4_ADDR);

        /* 停止按需拨号保护定时器 */
        if (RNIC_DEV_ID_RMNET0 == ucRmNetId)
        {
            RNIC_StopTimer(TI_RNIC_DEMAND_DIAL_PROTECT);
        }
    }

    if (RNIC_BIT8_IS_SET(ucIpFamilyMask, RNIC_IPV6_ADDR))
    {
        RNIC_IFACE_SetNetDevUp(pstIfaceCtx, RNIC_IPV6_ADDR);
    }

    RNIC_IFACE_RegRxHandler(pstIfaceCtx, ucIpFamilyMask);

    enDsFlowTimerId = RNIC_GetDsflowTimerIdByNetId(ucRmNetId);
    RNIC_StartTimer(enDsFlowTimerId, TI_RNIC_DSFLOW_STATS_LEN);

    return;
}


VOS_VOID RNIC_IFACE_PsIfaceDown(
    VOS_UINT8                           ucRmNetId,
    VOS_UINT8                           ucIpFamilyMask
)
{
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx = VOS_NULL_PTR;
    RNIC_TIMER_ID_ENUM_UINT16           enDsFlowTimerId;

    pstIfaceCtx = RNIC_GET_IFACE_CTX_ADR(ucRmNetId);

    RNIC_NORMAL_LOG2(ACPU_PID_RNIC,
        "RNIC_IFACE_PsIfaceDown: <ucRmNetId, ucIpFamilyMask>",
        ucRmNetId, ucIpFamilyMask);

    if (RNIC_BIT8_IS_SET(ucIpFamilyMask, RNIC_IPV4_ADDR))
    {
        RNIC_IFACE_SetNetDevDown(pstIfaceCtx, RNIC_IPV4_ADDR);

        /* RMNET0为IPV4类型时才会有按需拨号, 网卡down的时候停按需断开定时器 */
        if (RNIC_DEV_ID_RMNET0 == ucRmNetId)
        {
            RNIC_StopTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT);
            RNIC_CLEAR_TI_DIALDOWN_EXP_CONT();
        }
    }

    if (RNIC_BIT8_IS_SET(ucIpFamilyMask, RNIC_IPV6_ADDR))
    {
        RNIC_IFACE_SetNetDevDown(pstIfaceCtx, RNIC_IPV6_ADDR);
    }

    if ((RNIC_BIT_OPT_FALSE == RNIC_GET_IFACE_PDN_IPV4_ACT_STATE(ucRmNetId))
     && (RNIC_BIT_OPT_FALSE == RNIC_GET_IFACE_PDN_IPV6_ACT_STATE(ucRmNetId)))
    {
        /* 停止流量统计定时器，清除流量统计 */
        enDsFlowTimerId = RNIC_GetDsflowTimerIdByNetId(ucRmNetId);
        RNIC_StopTimer(enDsFlowTimerId);
        RNIC_IFACE_ClearDsFlowStats(ucRmNetId);
    }

    return;
}


VOS_VOID RNIC_IFACE_TetherInfo(
    RNIC_PS_IFACE_TETHER_INFO_STRU     *pstTetherInfo
)
{
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId;

    if(VOS_NULL_PTR == pstTetherInfo)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, pstTetherInfo is NULL!");
        return;
    }

    if (VOS_OK != rnic_get_devid_by_name((char *)pstTetherInfo->aucRmnetName, &enRmNetId))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, no such RmnetName!");
        return;
    }

    if (enRmNetId >= RNIC_DEV_ID_BUTT)
    {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, invalid enRmNetId ", enRmNetId);
        return;
    }

    /* 配置指定网卡的GRO功能 */
    if (AT_RNIC_USB_TETHER_CONNECTED == pstTetherInfo->enTetherConnStat)
    {
        /* 先保存指定网卡的GRO开关 */
        RNIC_SET_TETHER_ORIG_GRO_FEATURE(RNIC_GET_GRO_FEATURE(enRmNetId));

        RNIC_SET_GRO_FEATURE(enRmNetId, RNIC_FEATURE_OFF);
    }
    else if (AT_RNIC_USB_TETHER_DISCONNECT == pstTetherInfo->enTetherConnStat)
    {
        /* 恢复指定网卡的GRO开关 */
        RNIC_SET_GRO_FEATURE(enRmNetId, RNIC_GET_TETHER_ORIG_GRO_FEATURE());

        /* 初始化Tethering信息，避免影响下次的设置 */
        RNIC_InitTetherInfo();
    }
    else
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, enTetherConnStat invalid!");
        return;
    }

    RNIC_IFACE_SetNapiCfg(enRmNetId);

    return;
}


VOS_INT RNIC_BST_GetModemInfo(
    char                               *pNetDevName,
    BST_RNIC_MODEM_INFO_STRU           *pstModemInfo
)
{
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx = VOS_NULL_PTR;
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId;

    if (VOS_NULL_PTR == pNetDevName)
    {
        return VOS_ERROR;
    }

    if (VOS_NULL_PTR == pstModemInfo)
    {
        return VOS_ERROR;
    }

    if (VOS_OK != rnic_get_devid_by_name(pNetDevName, &enRmNetId))
    {
         RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_BST_GetModemInfo fail, no such RmnetName!");
        return VOS_ERROR;
    }

    if (enRmNetId >= RNIC_DEV_ID_BUTT)
    {
        return VOS_NULL_PTR;
    }

    pstIfaceCtx = RNIC_GET_IFACE_CTX_ADR(enRmNetId);

    pstModemInfo->enIPv4State = (RNIC_BIT_OPT_FALSE == pstIfaceCtx->stPsIfaceInfo.bitOpIpv4Act) ?
                                BST_RNIC_PDP_STATE_INACTIVE : BST_RNIC_PDP_STATE_ACTIVE;
    pstModemInfo->usModemId   = pstIfaceCtx->stPsIfaceInfo.enModemId;
    pstModemInfo->ucRabId     = pstIfaceCtx->stPsIfaceInfo.ucIpv4RabId;

    return VOS_OK;
}



