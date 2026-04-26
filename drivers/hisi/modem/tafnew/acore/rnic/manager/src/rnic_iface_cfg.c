/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "ps_common_def.h"
#include "rnic_msg_proc.h"
#include "rnic_entity.h"
#include "rnic_mntn.h"
#include "rnic_iface_ondemand.h"
#include "rnic_log.h"
#include "bastet_rnic_interface.h"
#include "rnic_ondemand_i.h"
#include "rnic_policy_manage.h"
#include "securec.h"
#include "mn_comm_api.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_RNIC_IFACE_CFG_C

VOS_VOID RNIC_IFACE_SetNapiCfg(RNIC_DEV_ID_ENUM_UINT8 rmNetId)
{
    struct rnic_napi_config_s napiCfg;

    napiCfg.devid             = rmNetId;
    napiCfg.napi_enable       = RNIC_GET_NAPI_FEATURE(rmNetId);
    napiCfg.gro_enable        = RNIC_GET_GRO_FEATURE(rmNetId);
    napiCfg.napi_weight       = RNIC_GET_NAPI_WEIGHT(rmNetId);
    napiCfg.napi_queue_length = RNIC_GET_NAPI_POLL_QUE_MAX_LEN(rmNetId);

    rnic_set_napi_config(&napiCfg);
}

VOS_VOID RNIC_IFACE_SetNapiLbCfg(RNIC_DEV_ID_ENUM_UINT8 rmNetId)
{
    RNIC_NapiLbCfg          *rnicNapiLbCfg = RNIC_GET_NAPI_LB_CFG_ADR(rmNetId);
    struct rnic_lb_config_s  napiLbCfg;

    (VOS_VOID)memset_s(&napiLbCfg, sizeof(napiLbCfg), 0x00, sizeof(napiLbCfg));

    napiLbCfg.devid              = rmNetId;
    napiLbCfg.lb_enable          = rnicNapiLbCfg->napiLbEnable;
    napiLbCfg.cpu_bitmask        = RNIC_GET_NAPI_LB_CPUMASK(rmNetId);

    if (memcpy_s(napiLbCfg.level_cfg, sizeof(napiLbCfg.level_cfg),
                 rnicNapiLbCfg->napiLbLevelCfg,
                 sizeof(rnicNapiLbCfg->napiLbLevelCfg)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetNapiLbCfg: memcpy fail!");
    }

    rnic_set_lb_config(&napiLbCfg);
}

VOS_VOID RNIC_IFACE_SetNapiLbLevel(RNIC_DEV_ID_ENUM_UINT8 rmNetId, VOS_UINT8 lbLevel)
{
    if (lbLevel != RNIC_GET_NAPI_LB_CUR_LEVEL(rmNetId)) {
        if (!rnic_set_lb_level(rmNetId, lbLevel)) {
            RNIC_SET_NAPI_LB_CUR_LEVEL(rmNetId, lbLevel);
        }
    }
}

VOS_VOID RNIC_IFACE_SetNapiWeight(RNIC_DEV_ID_ENUM_UINT8 rmNetId, VOS_UINT8 weight)
{
    if (weight != RNIC_GET_NAPI_WEIGHT(rmNetId)) {
        rnic_set_napi_weight(rmNetId, weight);
        RNIC_SET_NAPI_WEIGHT(rmNetId, weight);
    }
}

VOS_VOID RNIC_IFACE_SetRhcCfg(VOS_VOID)
{
    RNIC_CTX                 *rnicCtx = VOS_NULL_PTR;
    struct rnic_rhc_config_s  rhcCfg;

    rnicCtx = RNIC_GET_RNIC_CTX_ADR();
    (VOS_VOID)memset_s(&rhcCfg, sizeof(struct rnic_rhc_config_s), 0x00,
                       sizeof(struct rnic_rhc_config_s));

    rhcCfg.rhc_feature        = rnicCtx->rhcCfg.feature;
    rhcCfg.qcc_enable         = rnicCtx->rhcCfg.qccEnable;
    rhcCfg.napi_wt_exp_enable = rnicCtx->rhcCfg.napiWtExpEnable;
    rhcCfg.ddr_req_enable     = rnicCtx->rhcCfg.ddrReqEnable;
    rhcCfg.rps_mask_enable    = rnicCtx->rhcCfg.rpsMaskEnable;
    rhcCfg.qcc_timeout        = rnicCtx->rhcCfg.qccTimeout;
    rhcCfg.napi_poll_max      = rnicCtx->rhcCfg.napiPollMax;
    rhcCfg.ddr_mid_bd         = rnicCtx->rhcCfg.ddrMidBd;
    rhcCfg.ddr_high_bd        = rnicCtx->rhcCfg.ddrHighBd;

    if (memcpy_s(rhcCfg.level_cfg, sizeof(rhcCfg.level_cfg),
                 rnicCtx->rhcCfg.levelCfg, sizeof(rnicCtx->rhcCfg.levelCfg)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetRhcCfg: memcpy_s fail!");
        return;
    }

    rnic_set_rhc_config(&rhcCfg);
}

VOS_VOID RNIC_IFACE_ResetNapiConfig(RNIC_DEV_ID_ENUM_UINT8 rmNetId)
{
    VOS_UINT8 minWeight = RNIC_GET_NAPI_WEIGHT_LEVEL1(rmNetId);

    if ((RNIC_GET_NAPI_FEATURE(rmNetId) == RNIC_FEATURE_ON) &&
        (RNIC_GET_NAPI_WEIGHT_ADJ_MODE(rmNetId) == NAPI_WEIGHT_ADJ_DYNAMIC_MODE)) {
        if (rnic_set_napi_weight(rmNetId, minWeight) != 0) {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_ResetNapiConfig: set rnic_set_napi_weight fail!");
        }
        RNIC_SET_NAPI_WEIGHT(rmNetId, minWeight);
    }

    if (RNIC_GET_NAPI_LB_FEATURE(rmNetId) == RNIC_FEATURE_ON) {
        if (rnic_set_lb_level(rmNetId, 0) != 0) {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_ResetNapiConfig: set rnic_set_lb_level fail!");
        }
        RNIC_SET_NAPI_LB_CUR_LEVEL(rmNetId, 0);
    }
}

VOS_VOID RNIC_IFACE_ResetRhcLevel(VOS_VOID)
{
    if (g_rnicCtx.rhcCfg.feature == RNIC_FEATURE_ON) {
        rnic_set_rhc_level(0);
        g_rnicCtx.rhcCfg.rhcLevel = 0;
    }
}

VOS_UINT8 RNIC_IFACE_CalcNapiWeight(VOS_UINT8 rmNetId)
{
    VOS_UINT8  weight = RNIC_NAPI_POLL_DEFAULT_WEIGHT;
    VOS_UINT32 dlNapiRecvPktNum;

    dlNapiRecvPktNum = RNIC_GET_IFACE_PERIOD_RECV_PKT(rmNetId);

    /*
     * 分为5档:2/4/8/32/64
     * 150M bps每秒收包数在12300左右，在[0,12300]这个范围内，weight值设置为2.
     * 300M bps每秒收包数在24600左右，在[12300,24600]这个范围内，weight值设置为4.
     * 450M bps每秒收包数在38400左右，在[24600,38400]这个范围内，weight值设置为8.
     * 600M bps每秒收包数在49200左右，在[38400,49200]这个范围内，weight值设置为32.
     * 大于600M，weight值统一设置为64.
     */

    if (dlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL1(rmNetId)) {
        weight = RNIC_GET_NAPI_WEIGHT_LEVEL1(rmNetId);
    } else if ((dlNapiRecvPktNum > RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL1(rmNetId)) &&
               (dlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL2(rmNetId))) {
        weight = RNIC_GET_NAPI_WEIGHT_LEVEL2(rmNetId);
    } else if ((dlNapiRecvPktNum > RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL2(rmNetId)) &&
               (dlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL3(rmNetId))) {
        weight = RNIC_GET_NAPI_WEIGHT_LEVEL3(rmNetId);
    } else if ((dlNapiRecvPktNum > RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL3(rmNetId)) &&
               (dlNapiRecvPktNum <= RNIC_GET_DL_PKT_NUM_PER_SEC_LEVEL4(rmNetId))) {
        weight = RNIC_GET_NAPI_WEIGHT_LEVEL4(rmNetId);
    } else {
        weight = RNIC_GET_NAPI_WEIGHT_LEVEL5(rmNetId);
    }

    return weight;
}

VOS_VOID RNIC_IFACE_AdjustNapiWeight(VOS_UINT8 rmNetId)
{
    VOS_UINT8 weight;

    /* NAPI Weight动态调整模式，才去计算weight值并调整。其他情况默认采用静态default weight值。 */
    if (RNIC_GET_NAPI_WEIGHT_ADJ_MODE(rmNetId) == NAPI_WEIGHT_ADJ_DYNAMIC_MODE) {
        weight = RNIC_IFACE_CalcNapiWeight(rmNetId);
        RNIC_IFACE_SetNapiWeight(rmNetId, weight);
    }
}

VOS_VOID RNIC_IFACE_AdjNapiLbLevel(VOS_UINT8 rmNetId)
{
    VOS_UINT32 lbLevel;
    VOS_UINT32 dlNapiRecvPktNum;

    /* NAPI负载均衡特性打开时，才根据流量调整CPU的权重 */
    if (RNIC_GET_NAPI_LB_FEATURE(rmNetId) == RNIC_FEATURE_ON) {
        dlNapiRecvPktNum = RNIC_GET_IFACE_PERIOD_RECV_PKT(rmNetId);

        for (lbLevel = 0; lbLevel < (RNIC_NVIM_NAPI_LB_MAX_LEVEL - 1); lbLevel++) {
            if ((dlNapiRecvPktNum >= RNIC_GET_NAPI_LB_LEVEL_PPS(rmNetId, lbLevel)) &&
                (dlNapiRecvPktNum < RNIC_GET_NAPI_LB_LEVEL_PPS(rmNetId, lbLevel + 1UL)))
                break;
        }

        RNIC_IFACE_SetNapiLbLevel(rmNetId, (VOS_UINT8)lbLevel);
    }
}

VOS_UINT8 RNIC_IFACE_GetLevelByPps(RNIC_DSFLOW_Stats *dsFlowStats)
{
    VOS_UINT32 matchLevel = 0;
    VOS_UINT32 totalPps, tcpPps, nonTcpPps, level, levelTcpPps;

    totalPps  = dsFlowStats->periodRecvPktNum;
    tcpPps    = dsFlowStats->periodRecvTcpPktNum;
    nonTcpPps = dsFlowStats->periodRecvNonTcpPktNum;

    for (level = 0; level < RNIC_NVIM_RHC_MAX_LEVEL; level++) {
        if (totalPps < g_rnicCtx.rhcCfg.levelCfg[level].totalPps) {
            break;
        } else {
            levelTcpPps = g_rnicCtx.rhcCfg.levelCfg[level].totalPps -
                          g_rnicCtx.rhcCfg.levelCfg[level].nonTcpPps;

            if ((nonTcpPps >= g_rnicCtx.rhcCfg.levelCfg[level].nonTcpPps) &&
                (tcpPps >= levelTcpPps)) {
                matchLevel = level;
            }
        }
    }

    return (VOS_UINT8)matchLevel;
}

VOS_VOID RNIC_IFACE_AdjRhcLevel(RNIC_DSFLOW_Stats *ptotalDsFlowStats)
{
    VOS_UINT8 rhcLevel = 0;

    if (g_rnicCtx.rhcCfg.feature == RNIC_FEATURE_OFF) {
        return;
    }

    rhcLevel = RNIC_IFACE_GetLevelByPps(ptotalDsFlowStats);

    if ((rhcLevel != g_rnicCtx.rhcCfg.rhcLevel) ||
        (g_rnicCtx.rhcCfg.levelCfg[rhcLevel].isolationDisable)) {
        rnic_set_rhc_level(rhcLevel);
        g_rnicCtx.rhcCfg.rhcLevel = rhcLevel;
    }
}

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)

VOS_VOID RNIC_IFACE_NapiSchedule(VOS_ULONG usrData)
{
    rnic_rx_complete((VOS_UINT8)usrData);
}

VOS_INT RNIC_IFACE_RegRxHandler(RNIC_IFACE_Ctx *ifaceCtx)
{
    struct ads_iface_rx_handler_s rxHandler;

    (VOS_VOID)memset_s(&rxHandler, sizeof(rxHandler), 0x00, sizeof(rxHandler));

    rxHandler.user_data      = ifaceCtx->rmNetId;
    rxHandler.rx_dma_support = true;
    rxHandler.rx_func        = RNIC_DataRxProc;
    rxHandler.rx_cmplt_func  = RNIC_IFACE_NapiSchedule;

    return ads_iface_register_rx_handler(ifaceCtx->ifaceId, &rxHandler);
}

STATIC rnic_ps_iface_data_tx_func RNIC_IFACE_GetDataTxFunc(RNIC_PS_RatTypeUint8 ratType, VOS_UINT8 addrType)
{
    rnic_ps_iface_data_tx_func data_tx_func = NULL;

    if (addrType == RNIC_ETH_ADDR) {
        data_tx_func = RNIC_EthDataTxProc;
    } else {
        data_tx_func = (addrType == RNIC_IPV4_ADDR) ? RNIC_V4DataTxProc : RNIC_V6DataTxProc;
#if ((FEATURE_IMS == FEATURE_ON) && (FEATURE_ACPU_PHONE_MODE == FEATURE_ON))
        if (ratType == RNIC_PS_RAT_TYPE_IWLAN) {
            data_tx_func = RNIC_VoWifiDataTxProc;
        }
#endif
    }

    return data_tx_func;
}

VOS_VOID RNIC_IFACE_SetNetDevUp(RNIC_IFACE_Ctx *ifaceCtx, VOS_UINT8 addrType)
{
    struct rnic_ps_iface_config_s psIfaceCfg;

    (VOS_VOID)memset_s(&psIfaceCfg, sizeof(psIfaceCfg), 0x00, sizeof(psIfaceCfg));

    psIfaceCfg.devid       = ifaceCtx->rmNetId;
    psIfaceCfg.addr_family = addrType;

    psIfaceCfg.info.iface_id = ifaceCtx->ifaceId;
    psIfaceCfg.info.pdu_session_id = (addrType == RNIC_IPV4_ADDR) ? ifaceCtx->psIfaceInfo.ipv4ExRabId :
                                                                    ifaceCtx->psIfaceInfo.ipv6ExRabId;
    psIfaceCfg.info.fc_head  = RNIC_GET_FC_HEAD_BY_MODEMID(ifaceCtx->psIfaceInfo.modemId);
    psIfaceCfg.info.modem_id = (VOS_UINT8)ifaceCtx->psIfaceInfo.modemId;

    if (addrType == RNIC_ETH_ADDR) {
        psIfaceCfg.info.pdu_session_id = ifaceCtx->psIfaceInfo.ethExRabId;
        psIfaceCfg.info.fc_head        = EIPF_ETH_ULFC;
    }

    psIfaceCfg.data_tx_func = RNIC_IFACE_GetDataTxFunc(ifaceCtx->psIfaceInfo.ratType, addrType);
    rnic_set_ps_iface_up(&psIfaceCfg);
}

VOS_VOID RNIC_IFACE_SetNetDevDown(RNIC_IFACE_Ctx *ifaceCtx, VOS_UINT8 addrType)
{
    struct rnic_ps_iface_config_s psIfaceCfg;

    memset_s(&psIfaceCfg, sizeof(psIfaceCfg), 0x00, sizeof(psIfaceCfg));

    psIfaceCfg.devid               = ifaceCtx->rmNetId;
    psIfaceCfg.addr_family         = addrType;
    psIfaceCfg.info.iface_id       = RNIC_INVALID_IFACE_ID;
    psIfaceCfg.info.pdu_session_id = RNIC_INVALID_PDU_SESSION_ID;
    psIfaceCfg.info.fc_head        = RNIC_INVALID_FC_HEAD;
    psIfaceCfg.info.modem_id       = MODEM_ID_BUTT;
    psIfaceCfg.data_tx_func        = VOS_NULL_PTR;

    rnic_set_ps_iface_down(&psIfaceCfg);

    RNIC_IFACE_ResetNapiConfig(ifaceCtx->rmNetId);
}
#else

VOS_VOID RNIC_IFACE_NapiSchedule(VOS_UINT32 usrData)
{
    rnic_rx_complete((VOS_UINT8)usrData);
}

VOS_INT RNIC_IFACE_RegRxHandler(RNIC_IFACE_Ctx *ifaceCtx, VOS_UINT8 ipFamilyMask)
{
    VOS_UINT8 exRabId = RNIC_RAB_ID_INVALID;

    if (RNIC_BIT8_IS_SET(ipFamilyMask, RNIC_IPV4_ADDR)) {
        exRabId = ifaceCtx->psIfaceInfo.ipv4ExRabId;
    }

    if (RNIC_BIT8_IS_SET(ipFamilyMask, RNIC_IPV6_ADDR)) {
        exRabId = ifaceCtx->psIfaceInfo.ipv6ExRabId;
    }

    if (ifaceCtx->psIfaceInfo.ratType == RNIC_PS_RAT_TYPE_3GPP) {
        /* 注册下行发送函数，ADS调用注册的函数发送下行数据 */
        (VOS_VOID)ADS_DL_RegDlDataCallback(exRabId, RNIC_DataRxProc, ifaceCtx->rmNetId);

#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
        /* NAPI特性打开才执行 */
        if (RNIC_GET_NAPI_FEATURE(ifaceCtx->rmNetId) == RNIC_FEATURE_ON) {
            /* 注册下行NAPI处理的回调函数 */
            (VOS_VOID)ADS_DL_RegNapiCallback(exRabId, RNIC_IFACE_NapiSchedule, ifaceCtx->rmNetId);
        }
#endif
    }

    return VOS_OK;
}

VOS_VOID RNIC_IFACE_SetNetDevUp(RNIC_IFACE_Ctx *ifaceCtx, VOS_UINT8 addrType)
{
    struct rnic_ps_iface_config_s psIfaceCfg;

    memset_s(&psIfaceCfg, sizeof(psIfaceCfg), 0x00, sizeof(psIfaceCfg));

    psIfaceCfg.devid       = ifaceCtx->rmNetId;
    psIfaceCfg.addr_family = addrType;

    /* 向网卡驱动挂载上行收包回调 */
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
    if (ifaceCtx->psIfaceInfo.ratType == RNIC_PS_RAT_TYPE_IWLAN) {
        psIfaceCfg.data_tx_func = RNIC_VoWifiDataTxProc;
    } else
#endif
    {
        psIfaceCfg.data_tx_func = (addrType == RNIC_IPV4_ADDR) ? RNIC_V4DataTxProc : RNIC_V6DataTxProc;
    }

    if (rnic_set_ps_iface_up(&psIfaceCfg) != 0) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetDevUp: set rnic_set_ps_iface_up fail!");
    }
}

VOS_VOID RNIC_IFACE_SetNetDevDown(RNIC_IFACE_Ctx *ifaceCtx, VOS_UINT8 addrType)
{
    struct rnic_ps_iface_config_s psIfaceCfg;

    memset_s(&psIfaceCfg, sizeof(psIfaceCfg), 0x00, sizeof(psIfaceCfg));

    psIfaceCfg.devid       = ifaceCtx->rmNetId;
    psIfaceCfg.addr_family = addrType;
    psIfaceCfg.data_tx_func = VOS_NULL_PTR;

    if (rnic_set_ps_iface_down(&psIfaceCfg) != 0) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetDevDown: set rnic_set_ps_iface_down fail!");
    }

    RNIC_IFACE_ResetNapiConfig(ifaceCtx->rmNetId);
}
#endif /* FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM */


#if (defined(CONFIG_BALONG_ESPE))

VOS_VOID RNIC_IFACE_InitEspePort(VOS_VOID)
{
    struct rnic_espe_port_config_s portConfig;

    portConfig.info.iface_id       = RNIC_INVALID_IFACE_ID;
    portConfig.info.pdu_session_id = RNIC_INVALID_PDU_SESSION_ID;
    portConfig.info.fc_head        = RNIC_INVALID_FC_HEAD;
    portConfig.info.modem_id       = MODEM_ID_BUTT;

    if (rnic_init_espe_port(&portConfig) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_InitEspePort: Config ESPE failed!");
    }
}
#endif

VOS_VOID RNIC_IFACE_SetFeatureCfg(VOS_VOID)
{
    VOS_UINT32 indexNum;

    for (indexNum = 0; indexNum <= RNIC_3GPP_NET_ID_MAX_NUM; indexNum++) {
        /* 设置NAPI特性 */
        RNIC_IFACE_SetNapiCfg((VOS_UINT8)indexNum);
        RNIC_IFACE_SetNapiLbCfg((VOS_UINT8)indexNum);

#if (FEATURE_OFF == FEATURE_ACPU_PHONE_MODE)
        /* 注册设备上行丢包回调 */
        RNIC_IFACE_RegTxDropCB((VOS_UINT8)indexNum);
#endif
    }

    RNIC_IFACE_SetRhcCfg();
}

VOS_VOID RNIC_IFACE_SetDsFlowStats(VOS_UINT8 rmNetId, RNIC_DSFLOW_Stats *totalDsFlowStats)
{
    RNIC_DSFLOW_Stats *mgmtDsflowStats;
    struct rnic_dsflow_stats_s devDsflowStats;

    mgmtDsflowStats = &(g_rnicCtx.ifaceCtx[rmNetId].dsFlowStats);
    memset_s(&devDsflowStats, sizeof(devDsflowStats), 0x00, sizeof(devDsflowStats));

    /* 调用驱动接口获取流量信息 */
    if (rnic_get_intvl_dsflow_stats(rmNetId, &devDsflowStats)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_SetDsFlowStats fail, get dsflow stats failed !");
        return;
    }

    mgmtDsflowStats->currentRecvRate  = devDsflowStats.rx_bytes;
    mgmtDsflowStats->currentSendRate  = devDsflowStats.tx_bytes;
    mgmtDsflowStats->periodRecvPktNum = devDsflowStats.rx_packets;
    /* 累加上行发送报文，按需断开定时器超时清零 */
    mgmtDsflowStats->periodSendPktNum += devDsflowStats.tx_packets;

    mgmtDsflowStats->periodRecvTcpPktNum    = devDsflowStats.rx_tcp_pkts;
    mgmtDsflowStats->periodRecvNonTcpPktNum = devDsflowStats.rx_non_tcp_pkts;

    totalDsFlowStats->currentRecvRate += devDsflowStats.rx_bytes;
    totalDsFlowStats->currentSendRate += devDsflowStats.tx_bytes;

    totalDsFlowStats->periodRecvPktNum += devDsflowStats.rx_packets;
    totalDsFlowStats->periodSendPktNum += devDsflowStats.tx_packets;
    totalDsFlowStats->periodRecvTcpPktNum += devDsflowStats.rx_tcp_pkts;
    totalDsFlowStats->periodRecvNonTcpPktNum += devDsflowStats.rx_non_tcp_pkts;

    RNIC_NORMAL_LOG4(ACPU_PID_RNIC, "RNIC_IFACE_SetDsFlowStats: <rx_bytes, tx_bytes, rx_packets, tx_packets> ",
                     devDsflowStats.rx_bytes, devDsflowStats.tx_bytes,
                     devDsflowStats.rx_packets, devDsflowStats.tx_packets);
}

VOS_VOID RNIC_IFACE_ClearDsFlowFlux(VOS_UINT8 rmNetId)
{
    rnic_clear_dsflow_stats(rmNetId);
}

VOS_VOID RNIC_IFACE_ClearDsFlowStats(VOS_UINT8 rmNetId)
{
    RNIC_IFACE_Ctx *ifaceCtx = VOS_NULL_PTR;

    ifaceCtx = RNIC_GET_IFACE_CTX_ADR(rmNetId);

    (VOS_VOID)memset_s(&ifaceCtx->dsFlowStats, sizeof(ifaceCtx->dsFlowStats),
                       0x00, sizeof(ifaceCtx->dsFlowStats));
}

VOS_VOID RNIC_IFACE_NetdevNotify(VOS_VOID)
{
    RNIC_DEV_READY_STRU *sndMsg;

    /* 内存分配 */
    sndMsg = (RNIC_DEV_READY_STRU *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_DEV_READY_STRU));

    if (sndMsg == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendNetdevReadyInd: Malloc failed!");
        return;
    }

    /* 填写消息 */
    RNIC_CFG_INTRA_MSG_HDR(sndMsg, ID_RNIC_NETDEV_READY_IND);

    /* 发送消息 */
    if (RNIC_SendMsg(sndMsg) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendNetdevReadyInd: Send msg failed!");
    }
}

VOS_VOID RNIC_IFACE_RegDevNotifyCB(VOS_VOID)
{
    struct rnic_deivce_notifier_s devNotifier;

    devNotifier.dev_notifier_func = RNIC_IFACE_NetdevNotify;

    rnic_register_device_notifier(&devNotifier);
}

VOS_VOID RNIC_IFACE_RegOnDemandOpsCB(VOS_VOID)
{
    struct rnic_ondemand_cb_ops_s ondemandOpsCB;

    ondemandOpsCB.dial_mode_cfg_func    = RNIC_IFACE_OndemandDialModeProc;
    ondemandOpsCB.idle_time_cfg_func    = RNIC_IFACE_OndemandIdleTimeProc;
    ondemandOpsCB.event_report_cfg_func = RNIC_IFACE_OndemandEventReportProc;

    rnic_register_ondemand_ops(&ondemandOpsCB);
}

VOS_VOID RNIC_IFACE_RegTxDropCB(VOS_UINT8 rmNetId)
{
    struct rnic_ps_iface_drop_notifier_s dropNotify;

    /* 网卡0上面上行报文触发按需拨号 */
    if (rmNetId == RNIC_DEV_ID_RMNET0) {
        dropNotify.devid              = rmNetId;
        dropNotify.drop_notifier_func = RNIC_TxDropProc;

        rnic_register_ps_iface_drop_notifier(&dropNotify);
    }
}

VOS_VOID RNIC_IFACE_ResetPsIface(VOS_UINT8 rmNetId)
{
    struct rnic_ps_iface_config_s psIfaceCfg;
    VOS_UINT32 addrType;

    (VOS_VOID)memset_s(&psIfaceCfg, sizeof(psIfaceCfg), 0x00, sizeof(psIfaceCfg));

    psIfaceCfg.devid        = rmNetId;
    psIfaceCfg.data_tx_func = VOS_NULL_PTR;
#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    psIfaceCfg.info.iface_id       = RNIC_INVALID_IFACE_ID;
    psIfaceCfg.info.pdu_session_id = RNIC_INVALID_PDU_SESSION_ID;
    psIfaceCfg.info.fc_head        = RNIC_INVALID_FC_HEAD;
    psIfaceCfg.info.modem_id       = MODEM_ID_BUTT;
#endif

    for (addrType = RNIC_IPV4_ADDR; addrType < RNIC_ADDR_BUTT; addrType++) {
        psIfaceCfg.addr_family = (VOS_UINT8)addrType;
        rnic_set_ps_iface_down(&psIfaceCfg);
    }

    RNIC_IFACE_ResetNapiConfig(rmNetId);
}

VOS_VOID RNIC_IFACE_PsIfaceUp(VOS_UINT8 rmNetId, VOS_UINT8 addrFamilyMask)
{
    RNIC_IFACE_Ctx    *ifaceCtx    = VOS_NULL_PTR;
    RNIC_PS_IfaceInfo *psIfaceInfo = VOS_NULL_PTR;
    VOS_UINT32         addrType;

    ifaceCtx    = RNIC_GET_IFACE_CTX_ADR(rmNetId);
    psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);

    RNIC_NORMAL_LOG3(ACPU_PID_RNIC, "RNIC_IFACE_PsIfaceUp: <ucRmNetId, ucIpFamilyMask, ucRatType>", rmNetId,
                     addrFamilyMask, psIfaceInfo->ratType);

    for (addrType = RNIC_IPV4_ADDR; addrType < RNIC_ADDR_BUTT; addrType++) {
        if (RNIC_BIT8_IS_SET(addrFamilyMask, addrType)) {
            RNIC_IFACE_SetNetDevUp(ifaceCtx, (VOS_UINT8)addrType);
        }
    }

    /* RMNET0为IPV4类型时才会有按需拨号, 停止按需拨号保护定时器 */
    if ((rmNetId == RNIC_DEV_ID_RMNET0) && RNIC_BIT8_IS_SET(addrFamilyMask, RNIC_IPV4_ADDR)) {
        RNIC_StopTimer(TI_RNIC_DEMAND_DIAL_PROTECT);
    }

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    RNIC_IFACE_RegRxHandler(ifaceCtx);
#else
    RNIC_IFACE_RegRxHandler(ifaceCtx, addrFamilyMask);
#endif

    if (rmNetId <= RNIC_3GPP_NET_ID_MAX_NUM) {
        RNIC_StartTimer(TI_RNIC_DSFLOW_STATS, TI_RNIC_DSFLOW_STATS_LEN);
    }

    RNIC_BIT32_SET(g_rnicCtx.dsflowTimerMask, rmNetId);
}

VOS_VOID RNIC_IFACE_PsIfaceDown(VOS_UINT8 rmNetId, VOS_UINT8 addrFamilyMask)
{
    RNIC_IFACE_Ctx *ifaceCtx = VOS_NULL_PTR;
    VOS_UINT32      addrType;

    ifaceCtx = RNIC_GET_IFACE_CTX_ADR(rmNetId);

    RNIC_NORMAL_LOG2(ACPU_PID_RNIC, "RNIC_IFACE_PsIfaceDown: <rmNetId, addrFamilyMask>", rmNetId, addrFamilyMask);

    for (addrType = RNIC_IPV4_ADDR; addrType < RNIC_ADDR_BUTT; addrType++) {
        if (RNIC_BIT8_IS_SET(addrFamilyMask, addrType)) {
            RNIC_IFACE_SetNetDevDown(ifaceCtx, (VOS_UINT8)addrType);
        }
    }

    /* RMNET0为IPV4类型时才会有按需拨号, 网卡down的时候停按需断开定时器 */
    if ((rmNetId == RNIC_DEV_ID_RMNET0) && RNIC_BIT8_IS_SET(addrFamilyMask, RNIC_IPV4_ADDR)) {
        RNIC_StopTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT);
        RNIC_CLEAR_TI_DIALDOWN_EXP_CONT();
    }

    if ((RNIC_GET_IFACE_PDN_IPV4_ACT_STATE(rmNetId) == RNIC_BIT_OPT_FALSE) &&
        (RNIC_GET_IFACE_PDN_IPV6_ACT_STATE(rmNetId) == RNIC_BIT_OPT_FALSE)) {
        RNIC_BIT32_CLR(g_rnicCtx.dsflowTimerMask, rmNetId);

        /* 如果数据网卡都down了，则停止流量统计定时器，清除流量统计 */
        if (!(g_rnicCtx.dsflowTimerMask & RNIC_3GPP_NET_MAX_NUM_MASK)) {
            RNIC_StopTimer(TI_RNIC_DSFLOW_STATS);
            RNIC_IFACE_ClearDsFlowStats(rmNetId);
            RNIC_IFACE_ResetRhcLevel();
        }
    }
}

VOS_VOID RNIC_IFACE_TetherInfo(RNIC_PS_IfaceTetherInfo *tetherInfo)
{
#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
    RNIC_DEV_ID_ENUM_UINT8 rmNetId;

    if (tetherInfo == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, pstTetherInfo is NULL!");
        return;
    }

    if (rnic_get_devid_by_name((char *)tetherInfo->rmnetName, &rmNetId) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, no such RmnetName!");
        return;
    }

    if (rmNetId >= RNIC_DEV_ID_BUTT) {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, invalid enRmNetId ", rmNetId);
        return;
    }

    /* 配置指定网卡的GRO功能 */
    if (tetherInfo->tetherConnStat == AT_RNIC_USB_TETHER_CONNECTED) {
        /* 先保存指定网卡的GRO开关 */
        RNIC_SET_TETHER_ORIG_GRO_FEATURE(RNIC_GET_GRO_FEATURE(rmNetId));

        RNIC_SET_GRO_FEATURE(rmNetId, RNIC_FEATURE_OFF);
    } else if (tetherInfo->tetherConnStat == AT_RNIC_USB_TETHER_DISCONNECT) {
        /* 恢复指定网卡的GRO开关 */
        RNIC_SET_GRO_FEATURE(rmNetId, RNIC_GET_TETHER_ORIG_GRO_FEATURE());

        /* 初始化Tethering信息，避免影响下次的设置 */
        RNIC_InitTetherInfo();
    } else {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_TetherInfo fail, enTetherConnStat invalid!");
        return;
    }

    RNIC_IFACE_SetNapiCfg(rmNetId);
#else
    (VOS_VOID)tetherInfo;
#endif
}

VOS_VOID RNIC_IFACE_SetAllNapiCfg(NAPI_WEIGHT_AdjModeUint8 mode, VOS_UINT8 weight)
{
    VOS_UINT32 indexNum;

    for (indexNum = 0; indexNum <= RNIC_3GPP_NET_ID_MAX_NUM; indexNum++) {
        RNIC_SET_NAPI_WEIGHT_ADJ_MODE(indexNum, mode);
        RNIC_SET_NAPI_WEIGHT(indexNum, weight);
        RNIC_IFACE_SetNapiCfg((RNIC_DEV_ID_ENUM_UINT8)indexNum);
    }
}

VOS_INT RNIC_IFACE_SetSingleNapiCfg(RNIC_PS_IfaceRmnetCfg *rmnetCfg, NAPI_WEIGHT_AdjModeUint8 mode, VOS_UINT8 weight)
{
    RNIC_DEV_ID_ENUM_UINT8 rmnetId;

    if (rnic_get_devid_by_name((char *)rmnetCfg->rmnetName, &rmnetId) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_RmnetCfg fail, no such RmnetName!");
        return VOS_ERROR;
    }

    if (rmnetId > RNIC_3GPP_NET_ID_MAX_NUM) {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_IFACE_RmnetCfg fail, invalid enRmNetId ", rmnetId);
        return VOS_ERROR;
    }

    RNIC_SET_NAPI_WEIGHT_ADJ_MODE(rmnetId, mode);
    RNIC_SET_NAPI_WEIGHT(rmnetId, weight);
    RNIC_IFACE_SetNapiCfg(rmnetId);

    return VOS_OK;
}

VOS_INT RNIC_IFACE_RmnetCfg(RNIC_PS_IfaceRmnetCfg *rmnetCfg)
{
    NAPI_WEIGHT_AdjModeUint8 mode;
    VOS_UINT8 weight;

    /* 配置的weight为0，则为动态调整模式，采用默认weight值 */
    mode = ((rmnetCfg->weight == 0) ? NAPI_WEIGHT_ADJ_DYNAMIC_MODE : NAPI_WEIGHT_ADJ_STATIC_MODE);
    weight = ((rmnetCfg->weight == 0) ? RNIC_NAPI_POLL_DEFAULT_WEIGHT : (rmnetCfg->weight));

    if (rmnetCfg->featureMode == AT_RNIC_ALL_RMNET) {
        RNIC_IFACE_SetAllNapiCfg(mode, weight);
    } else if (rmnetCfg->featureMode == AT_RNIC_SINGLE_RMNET) {
        return RNIC_IFACE_SetSingleNapiCfg(rmnetCfg, mode, weight);
    } else {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_RmnetCfg fail, featureMode invalid!");
        return VOS_ERROR;
    }

    return VOS_OK;
}

VOS_INT RNIC_BST_GetModemInfo(const char *netDevName, BST_RNIC_ModemInfo *modemInfo)
{
    RNIC_IFACE_Ctx        *ifaceCtx = VOS_NULL_PTR;
    RNIC_DEV_ID_ENUM_UINT8 rmNetId;

    if (netDevName == VOS_NULL_PTR) {
        return VOS_ERROR;
    }

    if (modemInfo == VOS_NULL_PTR) {
        return VOS_ERROR;
    }

    if (rnic_get_devid_by_name(netDevName, &rmNetId) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_BST_GetModemInfo fail, no such RmnetName!");
        return VOS_ERROR;
    }

    if (rmNetId >= RNIC_DEV_ID_BUTT) {
        return VOS_NULL_PTR;
    }

    ifaceCtx = RNIC_GET_IFACE_CTX_ADR(rmNetId);

    modemInfo->ipv4State = (ifaceCtx->psIfaceInfo.opIpv4Act == RNIC_BIT_OPT_FALSE) ? BST_RNIC_PDP_STATE_INACTIVE :
                                                                                     BST_RNIC_PDP_STATE_ACTIVE;
    modemInfo->ipv6State = (ifaceCtx->psIfaceInfo.opIpv6Act == RNIC_BIT_OPT_FALSE) ? BST_RNIC_PDP_STATE_INACTIVE :
                                                                                     BST_RNIC_PDP_STATE_ACTIVE;

    modemInfo->modemId = ifaceCtx->psIfaceInfo.modemId;
    modemInfo->ipv4RabId = ifaceCtx->psIfaceInfo.ipv4RabId;
    modemInfo->ipv6RabId = ifaceCtx->psIfaceInfo.ipv6RabId;

    return VOS_OK;
}

