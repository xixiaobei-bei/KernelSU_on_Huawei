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

#include "rnic_ctx.h"
#include "rnic_msg_proc.h"
#include "at_rnic_interface.h"
#include "v_typdef.h"
#include "rnic_timer_mgmt.h"
#include "mdrv.h"
#include "rnic_entity.h"
#include "rnic_log.h"
#include "PsTypeDef.h"
#include "ps_common_def.h"
#include "acpu_reset.h"
#include "imsa_rnic_interface.h"
#include "rnic_cds_interface.h"
#include "rnic_iface_cfg.h"
#include "rnic_iface_ondemand.h"
#include "rnic_mntn.h"
#include "rnic_dev_def.h"
#include "dsm_rnic_pif.h"
#include "securec.h"
#include "mn_comm_api.h"
#include "netmgr_vcom_i.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_RNIC_PROCMSG_C

#define NM_CTRL_SEND_MSG(data, len) nm_send_msg(data, len, true)

static const RNIC_PROC_Msg g_rnicMsgProcTab[] = {
    /* 发送消息PID */ /* 消息ID */ /* 消息处理函数 */
    /* ****************************    AT-> RNIC Begin  ****************************** */
    { WUEPS_PID_AT, ID_AT_RNIC_DIAL_MODE_REQ, RNIC_RcvAtDialModeReq },
    { WUEPS_PID_AT, ID_AT_RNIC_USB_TETHER_INFO_IND, RNIC_RcvAtUsbTetherInfoInd },
    { WUEPS_PID_AT, ID_AT_RNIC_RMNET_CFG_REQ, RNIC_RcvAtRmnetCfgReq },
    /* ****************************    AT-> RNIC End    ****************************** */

    /* ****************************    TIMER -> RNIC Begin  ****************************** */
    { VOS_PID_TIMER, TI_RNIC_DSFLOW_STATS, RNIC_RcvTiDsflowStatsExpired },
    { VOS_PID_TIMER, TI_RNIC_DEMAND_DIAL_DISCONNECT, RNIC_RcvTiDemandDialDisconnectExpired },
    /* ****************************    TIMER -> RNIC End  ****************************** */

    /* ****************************    RNIC -> RNIC Begin  ****************************** */
    { ACPU_PID_RNIC, ID_RNIC_CCPU_RESET_START_IND, RNIC_RcvCcpuResetStartInd },
    { ACPU_PID_RNIC, ID_RNIC_CCPU_RESET_END_IND, RNIC_RcvCcpuResetEndInd },
    { ACPU_PID_RNIC, ID_RNIC_NETDEV_READY_IND, RNIC_RcvNetdevReadyInd },
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
    { ACPU_PID_RNIC, ID_RNIC_IMS_DATA_PROC_IND, RNIC_ProcImsData },
    /* ****************************    RNIC -> RNIC End    ****************************** */

    /* ****************************    IMSA -> RNIC Begin  ****************************** */
    { PS_PID_IMSA, ID_IMSA_RNIC_PDN_ACT_IND, RNIC_RcvImsaPdnActInd },
    { PS_PID_IMSA, ID_IMSA_RNIC_PDN_DEACT_IND, RNIC_RcvImsaPdnDeactInd },
    { PS_PID_IMSA, ID_IMSA_RNIC_PDN_MODIFY_IND, RNIC_RcvImsaPdnModifyInd },
    { PS_PID_IMSA, ID_IMSA_RNIC_RESERVED_PORTS_CONFIG_IND, RNIC_RcvImsaReservedPortsCfgInd },
    { PS_PID_IMSA, ID_IMSA_RNIC_SOCKET_EXCEPTION_IND, RNIC_RcvImsaSocketExceptionInd },
    { PS_PID_IMSA, ID_IMSA_RNIC_SIP_PORTS_RANGE_IND, RNIC_RcvImsaSipPortRangeInd },
    /* ****************************    IMSA -> RNIC End  ****************************** */

    /* ****************************    CDS -> RNIC Begin ****************************** */
    { UEPS_PID_CDS, ID_CDS_RNIC_IMS_DATA_IND, RNIC_RcvCdsImsDataInd },
/* ****************************    CDS -> RNIC End   ****************************** */
#endif /* FEATURE_ON == FEATURE_IMS  && FEATURE_ON == FEATURE_ACPU_PHONE_MODE */

    /* ****************************    DSM -> RNIC Begin  ****************************** */
    { UEPS_PID_DSM, ID_DSM_RNIC_IFACE_CFG_IND, RNIC_RcvDsmPdnInfoCfgInd },
    { UEPS_PID_DSM, ID_DSM_RNIC_IFACE_REL_IND, RNIC_RcvDsmPdnInfoRelInd },
    { UEPS_PID_DSM, ID_DSM_RNIC_NAPI_CFG_IND,  RNIC_RcvDsmNapiCfgInd },
    { UEPS_PID_DSM, ID_DSM_RNIC_NAPI_LB_CFG_IND,  RNIC_RcvDsmNapiLbCfgInd },
    { UEPS_PID_DSM, ID_DSM_RNIC_RHC_CFG_IND,  RNIC_RcvDsmRhcCfgInd },
    /* ****************************    DSM -> RNIC End  ****************************** */
};

const RNIC_IfaceRmnetIdInfo g_rnicIfaceIdRmNetIdTab[] = {
    { PS_IFACE_ID_RMNET0, RNIC_DEV_ID_RMNET0 },   { PS_IFACE_ID_RMNET1, RNIC_DEV_ID_RMNET1 },
    { PS_IFACE_ID_RMNET2, RNIC_DEV_ID_RMNET2 },   { PS_IFACE_ID_RMNET3, RNIC_DEV_ID_RMNET3 },
    { PS_IFACE_ID_RMNET4, RNIC_DEV_ID_RMNET4 },   { PS_IFACE_ID_RMNET5, RNIC_DEV_ID_RMNET5 },
    { PS_IFACE_ID_RMNET6, RNIC_DEV_ID_RMNET6 },   { PS_IFACE_ID_RMNET7, RNIC_DEV_ID_RMNET7 },
    { PS_IFACE_ID_RMNET8, RNIC_DEV_ID_RMNET8 },   { PS_IFACE_ID_RMNET9, RNIC_DEV_ID_RMNET9 },
    { PS_IFACE_ID_RMNET10, RNIC_DEV_ID_RMNET10 }, { PS_IFACE_ID_RMNET11, RNIC_DEV_ID_RMNET11 },
};
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
const RNIC_IfaceRmnetIdInfo g_rnicImsIfaceIdRmNetIdTab[] = {
    { PS_IFACE_ID_RMNET_IMS00, RNIC_DEV_ID_RMNET_IMS00 },
    { PS_IFACE_ID_RMNET_IMS10, RNIC_DEV_ID_RMNET_IMS10 },
    { PS_IFACE_ID_RMNET_EMC0, RNIC_DEV_ID_RMNET_EMC0 },
    { PS_IFACE_ID_RMNET_EMC1, RNIC_DEV_ID_RMNET_EMC1 },
    { PS_IFACE_ID_RMNET_R_IMS00, RNIC_DEV_ID_RMNET_R_IMS00 },
    { PS_IFACE_ID_RMNET_R_IMS01, RNIC_DEV_ID_RMNET_R_IMS01 },
    { PS_IFACE_ID_RMNET_R_IMS10, RNIC_DEV_ID_RMNET_R_IMS10 },
    { PS_IFACE_ID_RMNET_R_IMS11, RNIC_DEV_ID_RMNET_R_IMS11 },
};
#endif

VOS_UINT8 RNIC_GetRmnetIdByIfaceId(const VOS_UINT8 ifaceId)
{
    VOS_UINT32 tableSize, i;

    /* 获取处理操作个数 */
    tableSize = sizeof(g_rnicIfaceIdRmNetIdTab) / sizeof(RNIC_IfaceRmnetIdInfo);

    /* g_rnicIfaceIdRmNetIdTab查表，获取RMNET ID */
    for (i = 0; i < tableSize; i++) {
        if (g_rnicIfaceIdRmNetIdTab[i].ifaceId == ifaceId) {
            return (VOS_UINT8)g_rnicIfaceIdRmNetIdTab[i].rmnetId;
        }
    }

    return RNIC_DEV_ID_BUTT;
}

VOS_UINT32 RNIC_RcvAtDialModeReq(MsgBlock *msg)
{
    AT_RNIC_DialModeReq *rcvInd = VOS_NULL_PTR;
    RNIC_AT_DialModeCnf *sndMsg = VOS_NULL_PTR;

    /* 内存分配 */
    sndMsg = (RNIC_AT_DialModeCnf *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_AT_DialModeCnf));
    if (sndMsg == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvAtDialModeReq: Malloc failed!");
        return VOS_ERR;
    }

    rcvInd = (AT_RNIC_DialModeReq *)msg;

    sndMsg->clientId        = rcvInd->clientId;
    sndMsg->dialMode        = RNIC_GET_PROC_DIAL_MODE();
    sndMsg->idleTime        = RNIC_GET_PROC_IDLE_TIME();
    sndMsg->eventReportFlag = RNIC_GET_PROC_EVENT_REPORT();

    /* 通过ID_RNIC_AT_DIAL_MODE_CNF消息发送给AT模块 */
    /* 填充消息 */
    RNIC_CFG_AT_MSG_HDR(sndMsg, ID_RNIC_AT_DIAL_MODE_CNF);

    /* 发送消息 */
    if (RNIC_SendMsg(sndMsg) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvAtDialModeReq: Send msg failed!");
        return VOS_ERR;
    }

    return VOS_OK;
}

STATIC VOS_VOID RNIC_UpdatePdnInfo(DSM_RNIC_IfaceCfgInd *rnicPdnCfgInd, VOS_UINT8 rmNetId)
{
    RNIC_PS_IfaceInfo *psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    VOS_UINT8 exRabId = RNIC_BUILD_EXRABID(rnicPdnCfgInd->modemId, rnicPdnCfgInd->rabId);

    psIfaceInfo->modemId = rnicPdnCfgInd->modemId;
    psIfaceInfo->ratType = RNIC_PS_RAT_TYPE_3GPP;

    if ((rnicPdnCfgInd->sessType & DSM_RNIC_SESSION_TYPE_IPV4) == DSM_RNIC_SESSION_TYPE_IPV4) {
        psIfaceInfo->opIpv4Act   = RNIC_BIT_OPT_TRUE;
        psIfaceInfo->ipv4RabId   = rnicPdnCfgInd->rabId;
        psIfaceInfo->ipv4ExRabId = exRabId;
        psIfaceInfo->ipv4Addr    = rnicPdnCfgInd->ipv4Addr;
    }

    if ((rnicPdnCfgInd->sessType & DSM_RNIC_SESSION_TYPE_IPV6) == DSM_RNIC_SESSION_TYPE_IPV6) {
        psIfaceInfo->opIpv6Act   = RNIC_BIT_OPT_TRUE;
        psIfaceInfo->ipv6RabId   = rnicPdnCfgInd->rabId;
        psIfaceInfo->ipv6ExRabId = exRabId;

        if (memcpy_s(psIfaceInfo->ipv6Addr, sizeof(psIfaceInfo->ipv6Addr), rnicPdnCfgInd->ipv6Addr,
                 RNICITF_MAX_IPV6_ADDR_LEN) != EOK) {
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_UpdatePdnInfo: memcpy_s fail!");
        }
    }

    if ((rnicPdnCfgInd->sessType & DSM_RNIC_SESSION_TYPE_ETHER) == DSM_RNIC_SESSION_TYPE_ETHER) {
        psIfaceInfo->opEthAct   = RNIC_BIT_OPT_TRUE;
        psIfaceInfo->ethRabId   = rnicPdnCfgInd->rabId;
        psIfaceInfo->ethExRabId = exRabId;
    }
}

STATIC VOS_VOID RNIC_ClearPdnInfo(DSM_RNIC_SessionTypeUint32 sessType, VOS_UINT8 rmNetId)
{
    RNIC_PS_IfaceInfo *psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);

    if ((sessType & DSM_RNIC_SESSION_TYPE_IPV4) == DSM_RNIC_SESSION_TYPE_IPV4) {
        /* 清空IPV4 PDP上下文信息 */
        RNIC_InitIpv4PdnInfo(psIfaceInfo);
    }

    if ((sessType & DSM_RNIC_SESSION_TYPE_IPV6) == DSM_RNIC_SESSION_TYPE_IPV6) {
        /* 清空IPV6 PDP上下文信息 */
        RNIC_InitIpv6PdnInfo(psIfaceInfo);
    }

    if ((sessType & DSM_RNIC_SESSION_TYPE_ETHER) == DSM_RNIC_SESSION_TYPE_ETHER) {
        /* 清空ETH PDP上下文信息 */
        RNIC_InitEthPdnInfo(psIfaceInfo);
    }

    /* PDP都未激活的时候 */
    if ((psIfaceInfo->opIpv4Act != RNIC_BIT_OPT_TRUE) && (psIfaceInfo->opIpv6Act != RNIC_BIT_OPT_TRUE) &&
        (psIfaceInfo->opEthAct != RNIC_BIT_OPT_TRUE)) {
        psIfaceInfo->modemId = MODEM_ID_BUTT;
        psIfaceInfo->ratType = RNIC_PS_RAT_TYPE_BUTT;
    }
}

STATIC VOS_UINT8 RNIC_GetAddrFamilyMask(DSM_RNIC_SessionTypeUint32 sessType)
{
    VOS_UINT8 addrFamilyMask = 0;

    if ((sessType & DSM_RNIC_SESSION_TYPE_IPV4) == DSM_RNIC_SESSION_TYPE_IPV4) {
        addrFamilyMask = RNIC_BIT8_SET(addrFamilyMask, RNIC_IPV4_ADDR);
    }

    if ((sessType & DSM_RNIC_SESSION_TYPE_IPV6) == DSM_RNIC_SESSION_TYPE_IPV6) {
        addrFamilyMask = RNIC_BIT8_SET(addrFamilyMask, RNIC_IPV6_ADDR);
    }

    if ((sessType & DSM_RNIC_SESSION_TYPE_ETHER) == DSM_RNIC_SESSION_TYPE_ETHER) {
        addrFamilyMask = RNIC_BIT8_SET(addrFamilyMask, RNIC_ETH_ADDR);
    }

    return addrFamilyMask;
}

VOS_UINT32 RNIC_RcvDsmPdnInfoCfgInd(MsgBlock *msg)
{
    DSM_RNIC_IfaceCfgInd *rnicPdnCfgInd = VOS_NULL_PTR;
    RNIC_IFACE_Ctx       *rnicIfaceCtx  = VOS_NULL_PTR;
    VOS_UINT8             addrFamilyMask;
    VOS_UINT8             rmNetId;

    rnicPdnCfgInd = (DSM_RNIC_IfaceCfgInd *)msg;

    /* 检查MODEMID */
    if (!RNIC_MODEM_ID_IS_VALID(rnicPdnCfgInd->modemId)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmPdnInfoCfgInd: ModemId is invalid.");
        return VOS_ERR;
    }

#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    /* 检查RABID */
    if (!RNIC_RAB_ID_IS_VALID(rnicPdnCfgInd->rabId)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmPdnInfoCfgInd: RabId is invalid.");
        return VOS_ERR;
    }
#endif

    rmNetId = RNIC_GetRmnetIdByIfaceId(rnicPdnCfgInd->ifaceId);

    /* 检查RmnetID */
    if (!RNIC_RMNET_IS_VALID(rmNetId)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmPdnInfoCfgInd: RmNetId is invalid.");
        return VOS_ERR;
    }

    /* 检查Session类型, 注意按位与操作下, ETHERNET是最后一个 */
    if (rnicPdnCfgInd->sessType >= DSM_RNIC_SESSION_TYPE_BUTT) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmPdnInfoCfgInd: IP type is invalid.");
        return VOS_ERR;
    }

    rnicIfaceCtx          = RNIC_GET_IFACE_CTX_ADR(rmNetId);
    rnicIfaceCtx->ifaceId = rnicPdnCfgInd->ifaceId;

    /* 更新PDP上下文信息 */
    RNIC_UpdatePdnInfo(rnicPdnCfgInd, rmNetId);
    addrFamilyMask = RNIC_GetAddrFamilyMask(rnicPdnCfgInd->sessType);

    RNIC_IFACE_PsIfaceUp(rmNetId, addrFamilyMask);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvDsmPdnInfoRelInd(MsgBlock *msg)
{
    DSM_RNIC_IfaceRelInd *rnicPdnRelInd = VOS_NULL_PTR;
    VOS_UINT8             addrFamilyMask;
    VOS_UINT8             rmNetId;

    rnicPdnRelInd = (DSM_RNIC_IfaceRelInd *)msg;
    rmNetId       = RNIC_GetRmnetIdByIfaceId(rnicPdnRelInd->ifaceId);

    /* 检查RmnetID */
    if (!RNIC_RMNET_IS_VALID(rmNetId)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmPdnInfoRelInd: RmNetId is invalid.");
        return VOS_ERR;
    }

    /* 清空PDP上下文地址 */
    RNIC_ClearPdnInfo(rnicPdnRelInd->sessType, rmNetId);
    addrFamilyMask = RNIC_GetAddrFamilyMask(rnicPdnRelInd->sessType);

    RNIC_IFACE_PsIfaceDown(rmNetId, addrFamilyMask);

    return VOS_OK;
}

STATIC VOS_VOID Rnic_SetNapiDefaultCfg(RNIC_NapiCfg *napiCfg)
{
    napiCfg->napiEnable = RNIC_FEATURE_OFF;
    napiCfg->groEnable = RNIC_FEATURE_OFF;
    napiCfg->napiWeightAdjMode = NAPI_WEIGHT_ADJ_STATIC_MODE;
    napiCfg->napiPollWeight = RNIC_NAPI_POLL_DEFAULT_WEIGHT;
    napiCfg->napiPollQueMaxLen = RNIC_POLL_QUEUE_DEFAULT_MAX_LEN;
}

VOS_UINT32 RNIC_RcvDsmNapiCfgInd(MsgBlock *msg)
{
    DSM_RNIC_NapiCfgInd *msgCfg = VOS_NULL_PTR;
    RNIC_NapiCfg         napiCfg;
    VOS_UINT32           rmNetId;
    errno_t              memResult;

    /* 防止driver 多次配置资源，只配置一次消息 */
    if (g_rnicCtx.rnicCfgIndNum++ >= RNIC_NV_CFG_MAX_NUM) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmNapiCfgInd: multiple configuration msg");
        return VOS_OK;
    }

    msgCfg = (DSM_RNIC_NapiCfgInd *)msg;

    memResult = memcpy_s(&napiCfg, sizeof(napiCfg), &(msgCfg->napiCfg), sizeof(msgCfg->napiCfg));
    if (memResult != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmNapiCfgInd: memcpy_s is failed.");
        Rnic_SetNapiDefaultCfg(&napiCfg);
    }

    for (rmNetId = 0; rmNetId <= RNIC_3GPP_NET_ID_MAX_NUM; rmNetId++) {
        if (g_rnicCtx.ipfMode == ADS_RD_THRD) {
            napiCfg.napiEnable = RNIC_FEATURE_OFF;
            napiCfg.groEnable  = RNIC_FEATURE_OFF;
        }
        RNIC_InitNapiCfg((VOS_UINT8)rmNetId,&napiCfg);

        if (g_rnicCtx.devReady == VOS_TRUE) {
            RNIC_IFACE_SetNapiCfg((VOS_UINT8)rmNetId);
        } else {
            RNIC_WARNING_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmNapiCfgInd: dev no ready.");
        }
    }

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvDsmNapiLbCfgInd(MsgBlock *msg)
{
    DSM_RNIC_NapiLbCfgInd *msgCfg = VOS_NULL_PTR;
    RNIC_NapiLbCfg         napiLbCfg;
    VOS_UINT32             rmNetId;
    errno_t                memResult;

    /* 防止driver 多次配置资源，只配置一次消息 */
    if (g_rnicCtx.rnicLbCfgIndNum++ >= RNIC_NV_CFG_MAX_NUM) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmNapiLbCfgInd: multiple configuration msg");
        return VOS_OK;
    }

    msgCfg = (DSM_RNIC_NapiLbCfgInd *)msg;

    memResult = memcpy_s(&napiLbCfg, sizeof(napiLbCfg), &(msgCfg->napiLbCfg), sizeof(msgCfg->napiLbCfg));
    if (memResult != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmNapiLbCfgInd: memcpy_s is failed.");
        napiLbCfg.napiLbEnable = RNIC_FEATURE_OFF;
    }

    for (rmNetId = 0; rmNetId <= RNIC_3GPP_NET_ID_MAX_NUM; rmNetId++) {
        RNIC_InitNapiLbCfg((VOS_UINT8)rmNetId,&napiLbCfg);

        if (g_rnicCtx.devReady == VOS_TRUE) {
            RNIC_IFACE_SetNapiLbCfg((VOS_UINT8)rmNetId);
        } else {
            RNIC_WARNING_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmNapiLbCfgInd: dev no ready.");
        }
    }

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvDsmRhcCfgInd(MsgBlock *msg)
{
    DSM_RNIC_RhcCfgInd *msgCfg = VOS_NULL_PTR;
    errno_t             memResult;

    /* 防止driver 多次配置资源，只配置一次消息 */
    if (g_rnicCtx.rhcCfgIndNum++ >= RNIC_NV_CFG_MAX_NUM) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmRhcCfgInd: multiple configuration msg");
        return VOS_OK;
    }

    msgCfg = (DSM_RNIC_RhcCfgInd *)msg;

    g_rnicCtx.rhcCfg.feature = msgCfg->rhcCfg.rhcFeature;
    g_rnicCtx.rhcCfg.qccEnable = msgCfg->rhcCfg.qccEnable;
    g_rnicCtx.rhcCfg.napiWtExpEnable = msgCfg->rhcCfg.napiWtExpEnable;
    g_rnicCtx.rhcCfg.ddrReqEnable =  msgCfg->rhcCfg.ddrReqEnable;
    g_rnicCtx.rhcCfg.qccTimeout = msgCfg->rhcCfg.qccTimeout;
    g_rnicCtx.rhcCfg.napiPollMax = msgCfg->rhcCfg.napiPollMax;
    g_rnicCtx.rhcCfg.rpsMaskEnable =  msgCfg->rhcCfg.rpsMaskEnable;
    g_rnicCtx.rhcCfg.ddrMidBd = msgCfg->rhcCfg.ddrMidBd;
    g_rnicCtx.rhcCfg.ddrHighBd = msgCfg->rhcCfg.ddrHighBd;

    memResult = memcpy_s(g_rnicCtx.rhcCfg.levelCfg, sizeof(g_rnicCtx.rhcCfg.levelCfg), msgCfg->rhcCfg.levelCfg,
        sizeof(msgCfg->rhcCfg.levelCfg));
    if (memResult != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmRhcCfgInd: memcpy_s is failed.");
        g_rnicCtx.rhcCfg.feature  = RNIC_FEATURE_OFF;
        g_rnicCtx.rhcCfg.qccEnable = RNIC_FEATURE_OFF;
        g_rnicCtx.rhcCfg.napiWtExpEnable = RNIC_FEATURE_OFF;
    }

    if (g_rnicCtx.devReady == VOS_TRUE) {
        RNIC_IFACE_SetRhcCfg();
    } else {
        RNIC_WARNING_LOG(ACPU_PID_RNIC, "RNIC_RcvDsmRhcCfgInd: dev no ready.");
    }

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvAtUsbTetherInfoInd(MsgBlock *msg)
{
#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
    AT_RNIC_UsbTetherInfoInd *rnicUsbTetherInd;
    RNIC_PS_IfaceTetherInfo   tetherInfo;

    rnicUsbTetherInd = (AT_RNIC_UsbTetherInfoInd *)msg;

    tetherInfo.tetherConnStat = rnicUsbTetherInd->tetherConnStat;
    if (memcpy_s(tetherInfo.rmnetName, sizeof(tetherInfo.rmnetName), rnicUsbTetherInd->rmnetName,
                 sizeof(rnicUsbTetherInd->rmnetName)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvAtUsbTetherInfoInd: memcpy_s is failed.");
    }

    /* 调用配置部分接口 */
    RNIC_IFACE_TetherInfo(&tetherInfo);
#endif

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvAtRmnetCfgReq(MsgBlock *msg)
{
    AT_RNIC_RmnetCfgReq *rmnetCfgReq = VOS_NULL_PTR;
    RNIC_AT_RmnetCfgCnf *rmnetCfgCnf = VOS_NULL_PTR;
    RNIC_PS_IfaceRmnetCfg rmnetCfg;
    VOS_INT ret;

    rmnetCfgReq = (AT_RNIC_RmnetCfgReq *)msg;

    rmnetCfg.featureMode = rmnetCfgReq->featureMode;
    if (memcpy_s(rmnetCfg.rmnetName, sizeof(rmnetCfg.rmnetName), rmnetCfgReq->rmnetName,
                 sizeof(rmnetCfgReq->rmnetName)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvAtRmnetCfgReq: memcpy_s is failed.");
    }

    rmnetCfg.weight = rmnetCfgReq->weight;

    ret = RNIC_IFACE_RmnetCfg(&rmnetCfg);

    rmnetCfgCnf = (RNIC_AT_RmnetCfgCnf *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_AT_RmnetCfgCnf));
    if (rmnetCfgCnf == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvAtRmnetCfgReq: Malloc failed!");
        return VOS_ERR;
    }

    rmnetCfgCnf->clientId = rmnetCfgReq->clientId;
    rmnetCfgCnf->result = ret;

    RNIC_CFG_AT_MSG_HDR(rmnetCfgCnf, ID_AT_RNIC_RMNET_CFG_CNF);

    /* 发送消息 */
    if (RNIC_SendMsg(rmnetCfgCnf) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvAtRmnetCfgReq: Send msg failed!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvTiDsflowStatsExpired(MsgBlock *msg)
{
    RNIC_DSFLOW_Stats totalDsFlowStats;
    VOS_UINT32        rmnetId;

    (VOS_VOID)memset_s(&totalDsFlowStats, sizeof(totalDsFlowStats), 0x00, sizeof(totalDsFlowStats));

    for (rmnetId = 0; rmnetId < RNIC_3GPP_NET_ID_MAX_NUM; rmnetId++) {
        if (RNIC_BIT32_IS_SET(g_rnicCtx.dsflowTimerMask, rmnetId)) {
            RNIC_IFACE_SetDsFlowStats((VOS_UINT8)rmnetId, &totalDsFlowStats);
#if (FEATURE_ON == FEATURE_RNIC_NAPI_GRO)
            RNIC_IFACE_AdjustNapiWeight((VOS_UINT8)rmnetId);
            RNIC_IFACE_AdjNapiLbLevel((VOS_UINT8)rmnetId);
            RNIC_CLEAN_IFACE_PERIOD_RECV_PKT(rmnetId);
#endif
        }
    }

    RNIC_IFACE_AdjRhcLevel(&totalDsFlowStats);
    RNIC_StartTimer(TI_RNIC_DSFLOW_STATS, TI_RNIC_DSFLOW_STATS_LEN);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvTiDemandDialDisconnectExpired(MsgBlock *msg)
{
    if (RNIC_IFACE_OndemandDisconnTimeoutProc(RNIC_DEV_ID_RMNET0) == VOS_OK) {
        RNIC_StartTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT, TI_RNIC_DEMAND_DIAL_DISCONNECT_LEN);
    }

    /* 上行发包数用于按需断开，按需断开定时器超时清零 */
    RNIC_CLEAN_IFACE_PERIOD_SEND_PKT(RNIC_DEV_ID_RMNET0);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvCcpuResetStartInd(MsgBlock *msg)
{
    VOS_UINT32 indexNum;
    RNIC_CTX *rnicCtx;

    rnicCtx = RNIC_GET_RNIC_CTX_ADR();

    RNIC_PR_LOGI("enter, %u", VOS_GetSlice());

    RNIC_StopAllTimer();

    for (indexNum = 0; indexNum < RNIC_NET_ID_MAX_NUM; indexNum++) {
        RNIC_IFACE_ResetPsIface(indexNum);
        RNIC_IFACE_ClearDsFlowStats(indexNum);
        RNIC_BIT32_CLR(g_rnicCtx.dsflowTimerMask, indexNum);
        RNIC_InitPdnInfo(&(rnicCtx->ifaceCtx[indexNum].psIfaceInfo));
    }

    RNIC_IFACE_ResetRhcLevel();
    RNIC_InitAllTimers();

#if (FEATURE_OFF == FEATURE_ACPU_PHONE_MODE)
    RNIC_SET_PROC_DIAL_MODE(RNIC_DIAL_MODE_MANUAL);
    RNIC_SET_PROC_IDLE_TIME(RNIC_DIAL_DEMA_IDLE_TIME);
    RNIC_SET_PROC_EVENT_REPORT(RNIC_FORBID_EVENT_REPORT);
#endif

    RNIC_CLEAR_TI_DIALDOWN_EXP_CONT();

    /* 释放信号量，使得调用API任务继续运行 */
    VOS_SmV(RNIC_GET_RESET_SEM());

    RNIC_PR_LOGI("leave, %u", VOS_GetSlice());
#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
    RNIC_SndNetManagerModemResetInd();
#endif
    return VOS_OK;
}

VOS_UINT32 RNIC_RcvCcpuResetEndInd(MsgBlock *msg)
{
    RNIC_NORMAL_LOG(ACPU_PID_RNIC, "RNIC_RcvCcpuResetEndInd: rcv ID_CCPU_RNIC_RESET_END_IND");
    return VOS_OK;
}

VOS_UINT32 RNIC_RcvNetdevReadyInd(MsgBlock *msg)
{
    g_rnicCtx.devReady = VOS_TRUE;

    /* 设置RNIC网卡特性 */
    RNIC_IFACE_SetFeatureCfg();

    /* 设置SPE配置 */

#if (defined(CONFIG_BALONG_ESPE))
    RNIC_IFACE_InitEspePort();
#endif

    return VOS_OK;
}

VOS_UINT32 RNIC_SendMsg(VOS_VOID *msg)
{
    VOS_UINT32  ret;

    ret = PS_SEND_MSG(ACPU_PID_RNIC, msg);
    return ret;
}

#if ((FEATURE_ACPU_PHONE_MODE == FEATURE_ON) && (FEATURE_IMS == FEATURE_ON))

STATIC VOS_UINT8 RNIC_GetImsEmcBearRmnetId(IMSA_RNIC_ImsRatTypeUint8 ratType, ModemIdUint16 modemId)
{
    /* 当前RNIC_RMNET_ID_EMC0只提供给lte使用，所以只需要判断接入技术为lte的时EmcInd标志 */
    if ((ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR) && (modemId == MODEM_ID_0)) {
        return RNIC_DEV_ID_RMNET_EMC0;
    }

    if ((ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI) && (modemId == MODEM_ID_0)) {
        return RNIC_DEV_ID_RMNET_R_IMS01;
    }

#if (MULTI_MODEM_NUMBER >= 2)
    if ((ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR) && (modemId == MODEM_ID_1)) {
        return RNIC_DEV_ID_RMNET_EMC1;
    }

    if ((ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI) && (modemId == MODEM_ID_1)) {
        return RNIC_DEV_ID_RMNET_R_IMS11;
    }
#endif

    return RNIC_DEV_ID_BUTT;
}

STATIC VOS_UINT8 RNIC_GetImsNormalBearRmnetId(IMSA_RNIC_ImsRatTypeUint8 ratType, ModemIdUint16 modemId)
{
    /* vowifi时为数据包转发网卡，volte时为vt网卡 */
    if ((modemId == MODEM_ID_0) && (ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR)) {
        return RNIC_DEV_ID_RMNET_IMS00;
    }

    if ((modemId == MODEM_ID_0) && (ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI)) {
        return RNIC_DEV_ID_RMNET_R_IMS00;
    }

#if (MULTI_MODEM_NUMBER >= 2)
    if ((modemId == MODEM_ID_1) && (ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR)) {
        return RNIC_DEV_ID_RMNET_IMS10;
    }

    if ((modemId == MODEM_ID_1) && (ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI)) {
        return RNIC_DEV_ID_RMNET_R_IMS10;
    }
#endif

    return RNIC_DEV_ID_BUTT;
}

STATIC VOS_UINT8 RNIC_GetImsRmnetId(IMSA_RNIC_ImsRatTypeUint8 ratType, ModemIdUint16 modemId,
                                    IMSA_RNIC_PdnEmcIndUint8 emcInd)
{
    RNIC_NORMAL_LOG3(ACPU_PID_RNIC, "RNIC_GetImsRmnetId: rat modemid emc is ", ratType, modemId, emcInd);

    if (emcInd == IMSA_RNIC_PDN_FOR_EMC) {
        return RNIC_GetImsEmcBearRmnetId(ratType, modemId);
    } else if (emcInd == IMSA_RNIC_PDN_NOT_FOR_EMC) {
        return RNIC_GetImsNormalBearRmnetId(ratType, modemId);
    } else {
        RNIC_NORMAL_LOG1(ACPU_PID_RNIC, "RNIC_GetImsRmnetId: enEmcInd abnormal, ", emcInd);
        return RNIC_DEV_ID_BUTT;
    }
}

STATIC PS_IFACE_IdUint8 RNIC_TransImsRmnetId2PsIfaceId(VOS_UINT8 rmnetId)
{
    VOS_UINT32 tableSize, i;
    PS_IFACE_IdUint8 psIfaceId = PS_IFACE_ID_BUTT;

    /* 获取处理操作个数 */
    tableSize = RNIC_ARRAY_SIZE(g_rnicImsIfaceIdRmNetIdTab);

    /* g_rnicIfaceIdRmNetIdTab查表，获取RMNET ID */
    for (i = 0; i < tableSize; i++) {
        if (g_rnicImsIfaceIdRmNetIdTab[i].rmnetId == rmnetId) {
            psIfaceId = (VOS_UINT8)g_rnicImsIfaceIdRmNetIdTab[i].ifaceId;
            break;
        }
    }

    return psIfaceId;
}

VOS_UINT32 RNIC_ProcImsaPdnActInd_Wifi(IMSA_RNIC_PdnInfoConfig *pdnInfo)
{
    RNIC_IFACE_Ctx    *ifaceCtx     = VOS_NULL_PTR;
    RNIC_PS_IfaceInfo *psIfaceInfo  = VOS_NULL_PTR;
    VOS_UINT8          ipFamilyMask = 0;
    VOS_UINT8          rmNetId;
    PS_IFACE_IdUint8   psIfaceId;

    /* 检查IP type */
    if ((pdnInfo->opIpv4PdnInfo == VOS_FALSE) && (pdnInfo->opIpv6PdnInfo == VOS_FALSE)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnActInd_Wifi: IP type is invalid.");
        return VOS_ERR;
    }

    /* 指定一张专门的网卡用于VT视频数据传输 */
    rmNetId = RNIC_GetImsRmnetId(IMSA_RNIC_IMS_RAT_TYPE_WIFI, pdnInfo->modemId, pdnInfo->emcInd);
    if (rmNetId == RNIC_DEV_ID_BUTT) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnActInd_Wifi: Get RmnetId failed.");
        return VOS_ERR;
    }

    psIfaceId = RNIC_TransImsRmnetId2PsIfaceId(rmNetId);

    /* 更新IFACE上下文信息 */
    ifaceCtx          = RNIC_GET_IFACE_CTX_ADR(rmNetId);
    ifaceCtx->ifaceId = psIfaceId;

    /* 更新PDP上下文信息 */
    psIfaceInfo          = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    psIfaceInfo->ratType = RNIC_PS_RAT_TYPE_IWLAN;
    psIfaceInfo->modemId = pdnInfo->modemId;

    /* IPV4激活 */
    if (pdnInfo->opIpv4PdnInfo == VOS_TRUE) {
        psIfaceInfo->opIpv4Act = RNIC_BIT_OPT_TRUE;
        ipFamilyMask           = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV4_ADDR);
    }

    /* IPV6激活 */
    if (pdnInfo->opIpv6PdnInfo == VOS_TRUE) {
        psIfaceInfo->opIpv6Act = RNIC_BIT_OPT_TRUE;
        ipFamilyMask           = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV6_ADDR);
    }

    RNIC_IFACE_PsIfaceUp(rmNetId, ipFamilyMask);

    return VOS_OK;
}

VOS_UINT32 RNIC_ProcImsaPdnActIndCellular(IMSA_RNIC_PdnInfoConfig *pdnInfo)
{
    RNIC_IFACE_Ctx    *ifaceCtx     = VOS_NULL_PTR;
    RNIC_PS_IfaceInfo *psIfaceInfo  = VOS_NULL_PTR;
    VOS_UINT8          ipFamilyMask = 0;
    VOS_UINT8          rmNetId;
    VOS_UINT8          exRabId;
    PS_IFACE_IdUint8   psIfaceId;

    /* 检查IP type */
    if ((pdnInfo->opIpv4PdnInfo == VOS_FALSE) && (pdnInfo->opIpv6PdnInfo == VOS_FALSE)) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnActIndCellular: IP type is invalid.");
        return VOS_ERR;
    }

#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    /* 检查RABID */
    if (!RNIC_RAB_ID_IS_VALID(pdnInfo->rabId)) {
        RNIC_INFO_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnActIndCellular: RabId is invalid.");
        return VOS_ERR;
    }
#endif

    /* 指定一张专门的网卡用于VT视频数据传输 */
    rmNetId = RNIC_GetImsRmnetId(IMSA_RNIC_IMS_RAT_TYPE_CELLULAR, pdnInfo->modemId, pdnInfo->emcInd);
    if (rmNetId == RNIC_DEV_ID_BUTT) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnActIndCellular: Get RmnetId failed.");
        return VOS_ERR;
    }

    psIfaceId = RNIC_TransImsRmnetId2PsIfaceId(rmNetId);

    /* 更新IFACE上下文信息 */
    ifaceCtx          = RNIC_GET_IFACE_CTX_ADR(rmNetId);
    ifaceCtx->ifaceId = psIfaceId;

    exRabId = RNIC_BUILD_EXRABID(pdnInfo->modemId, pdnInfo->rabId);

    /* 更新PDP上下文信息 */
    psIfaceInfo          = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    psIfaceInfo->ratType = RNIC_PS_RAT_TYPE_3GPP;
    psIfaceInfo->modemId = pdnInfo->modemId;

    /* IPV4激活 */
    if (pdnInfo->opIpv4PdnInfo == VOS_TRUE) {
        psIfaceInfo->opIpv4Act   = RNIC_BIT_OPT_TRUE;
        psIfaceInfo->ipv4RabId   = pdnInfo->rabId;
        psIfaceInfo->ipv4ExRabId = exRabId;
        ipFamilyMask             = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV4_ADDR);
    }

    /* IPV6激活 */
    if (pdnInfo->opIpv6PdnInfo == VOS_TRUE) {
        psIfaceInfo->opIpv6Act   = RNIC_BIT_OPT_TRUE;
        psIfaceInfo->ipv6RabId   = pdnInfo->rabId;
        psIfaceInfo->ipv6ExRabId = exRabId;
        ipFamilyMask             = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV6_ADDR);
    }

    RNIC_IFACE_PsIfaceUp(rmNetId, ipFamilyMask);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvImsaPdnActInd(MsgBlock *msg)
{
    IMSA_RNIC_PdnActInd     *rcvInd  = VOS_NULL_PTR;
    IMSA_RNIC_PdnInfoConfig *pdnInfo = VOS_NULL_PTR;
    VOS_UINT32               result;

    rcvInd  = (IMSA_RNIC_PdnActInd *)msg;
    pdnInfo = &(rcvInd->pdnInfo);

    if (pdnInfo->ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI) {
        result = RNIC_ProcImsaPdnActInd_Wifi(pdnInfo);
    } else if (pdnInfo->ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR) {
        result = RNIC_ProcImsaPdnActIndCellular(pdnInfo);
    } else {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvImsaPdnActInd rat type abnormal");
        result = VOS_ERR;
    }

    /*
     * 当前netmanager 只会up vt网卡，volte的RNIC_RMNET_ID_EMC0网卡也不在netmanager up，
     * vowifi的普通承载网卡和紧急承载网卡都是在mapcon处up和down的，所以此处代码逻辑暂不修改
     */
    if ((result == VOS_OK) && (pdnInfo->emcInd != IMSA_RNIC_PDN_FOR_EMC)) {
        RNIC_SndNetManagerPdpActInd(msg);
    }

    return result;
}

VOS_UINT32 RNIC_ProcImsaPdnDeactInd_Wifi(ModemIdUint16 modemId, IMSA_RNIC_PdnEmcIndUint8 emcInd)
{
    RNIC_PS_IfaceInfo *psIfaceInfo  = VOS_NULL_PTR;
    VOS_UINT8          ipFamilyMask = 0;
    VOS_UINT8          rmNetId;

    /* 指定一张专门的网卡用于VT视频数据传输 */
    rmNetId = RNIC_GetImsRmnetId(IMSA_RNIC_IMS_RAT_TYPE_WIFI, modemId, emcInd);
    if (rmNetId == RNIC_DEV_ID_BUTT) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnDeactInd_Wifi: ucRmNetId butt.");
        return VOS_ERR;
    }

    /* 获取PDP上下文地址 */
    psIfaceInfo          = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    psIfaceInfo->modemId = MODEM_ID_BUTT;
    psIfaceInfo->ratType = RNIC_PS_RAT_TYPE_BUTT;

    /* 如果是IPV4 PDP去激活 */
    if (psIfaceInfo->opIpv4Act == RNIC_BIT_OPT_TRUE) {
        /* 清空IPV4 PDP上下文信息 */
        RNIC_InitIpv4PdnInfo(psIfaceInfo);
        ipFamilyMask = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV4_ADDR);
    }

    if (psIfaceInfo->opIpv6Act == RNIC_BIT_OPT_TRUE) {
        /* 清空IPV6 PDP上下文信息 */
        RNIC_InitIpv6PdnInfo(psIfaceInfo);
        ipFamilyMask = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV6_ADDR);
    }

    RNIC_IFACE_PsIfaceDown(rmNetId, ipFamilyMask);

    return VOS_OK;
}

VOS_UINT32 RNIC_ProcImsaPdnDeactIndCellular(ModemIdUint16 modemId, IMSA_RNIC_PdnEmcIndUint8 emcInd)
{
    RNIC_PS_IfaceInfo *psIfaceInfo  = VOS_NULL_PTR;
    VOS_UINT8          ipFamilyMask = 0;
    VOS_UINT8          rmNetId;

    /* 指定一张专门的网卡用于VT视频数据传输 */
    rmNetId = RNIC_GetImsRmnetId(IMSA_RNIC_IMS_RAT_TYPE_CELLULAR, modemId, emcInd);
    if (rmNetId == RNIC_DEV_ID_BUTT) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcImsaPdnDeactIndCellular: ucRmNetId butt.");
        return VOS_ERR;
    }

    psIfaceInfo          = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    psIfaceInfo->modemId = MODEM_ID_BUTT;
    psIfaceInfo->ratType = RNIC_PS_RAT_TYPE_BUTT;

    /* 如果是IPV4 PDP去激活 */
    if (psIfaceInfo->opIpv4Act == RNIC_BIT_OPT_TRUE) {
        /* 清空IPV4 PDP上下文信息 */
        RNIC_InitIpv4PdnInfo(psIfaceInfo);
        ipFamilyMask = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV4_ADDR);
    }

    if (psIfaceInfo->opIpv6Act == RNIC_BIT_OPT_TRUE) {
        /* 清空IPV6 PDP上下文信息 */
        RNIC_InitIpv6PdnInfo(psIfaceInfo);
        ipFamilyMask = RNIC_BIT8_SET(ipFamilyMask, RNIC_IPV6_ADDR);
    }

    RNIC_IFACE_PsIfaceDown(rmNetId, ipFamilyMask);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvImsaPdnDeactInd(MsgBlock *msg)
{
    IMSA_RNIC_PdnDeactInd *rcvInd;
    ModemIdUint16          modemId;
    VOS_UINT32             result;

    rcvInd  = (IMSA_RNIC_PdnDeactInd *)msg;
    modemId = rcvInd->modemId;

    if (rcvInd->ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI) {
        result = RNIC_ProcImsaPdnDeactInd_Wifi(modemId, rcvInd->emcInd);
    } else if (rcvInd->ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR) {
        result = RNIC_ProcImsaPdnDeactIndCellular(modemId, rcvInd->emcInd);
    } else {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_RcvImsaPdnDeactInd rat type abnormal", rcvInd->ratType);
        result = VOS_ERR;
    }

    if ((result == VOS_OK) && (rcvInd->emcInd != IMSA_RNIC_PDN_FOR_EMC)) {
        RNIC_SndNetManagerPdpDeactInd(msg);
    }

    return result;
}

VOS_UINT32 RNIC_RcvImsaPdnModifyInd(MsgBlock *msg)
{
    IMSA_RNIC_PdnModifyInd  *rcvInd  = VOS_NULL_PTR;
    IMSA_RNIC_PdnInfoConfig *pdnInfo = VOS_NULL_PTR;
    VOS_UINT32               result;

    rcvInd  = (IMSA_RNIC_PdnModifyInd *)msg;
    pdnInfo = &(rcvInd->pdnInfo);

    /*
     * 当前RNIC_RMNET_ID_EMC0只提供给lte使用，且紧急呼当前不涉及切换,
     * 当modify消息中Emc标志存在时，认为异常
     */
    if (pdnInfo->emcInd == IMSA_RNIC_PDN_FOR_EMC) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvImsaPdnModifyInd: abnormal rcv emc modify ind.");
        return VOS_ERR;
    }

    if (pdnInfo->ratType == IMSA_RNIC_IMS_RAT_TYPE_CELLULAR) {
        RNIC_ProcImsaPdnDeactInd_Wifi(pdnInfo->modemId, IMSA_RNIC_PDN_NOT_FOR_EMC);
        RNIC_ProcImsaPdnActIndCellular(pdnInfo);
        result = VOS_OK;
    } else if (pdnInfo->ratType == IMSA_RNIC_IMS_RAT_TYPE_WIFI) {
        RNIC_ProcImsaPdnDeactIndCellular(pdnInfo->modemId, IMSA_RNIC_PDN_NOT_FOR_EMC);
        RNIC_ProcImsaPdnActInd_Wifi(pdnInfo);
        result = VOS_OK;
    } else {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_RcvImsaPdnModifyInd: rat type is ", pdnInfo->ratType);
        result = VOS_ERR;
    }

    if ((result == VOS_OK) && (pdnInfo->emcInd != IMSA_RNIC_PDN_FOR_EMC)) {
        RNIC_SndNetManagerPdpModifyInd(msg);
    }

    return result;
}

VOS_UINT32 RNIC_RcvImsaReservedPortsCfgInd(MsgBlock *msg)
{
    IMSA_RNIC_ReservedPortsConfigInd *rcvInd   = VOS_NULL_PTR;
    IMSA_RNIC_ImsPortInfo            *portInfo = VOS_NULL_PTR;

    rcvInd   = (IMSA_RNIC_ReservedPortsConfigInd *)msg;
    portInfo = &(rcvInd->imsPortInfo);

    if (portInfo->imsPortRangeNum > IMSA_RNIC_IMS_PORT_RANGE_GROUP_MAX_NUM) {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC,
                        "RNIC_RcvImsaReservedPortsCfgInd: ulImsPortRangeNum too large: ", portInfo->imsPortRangeNum);
        return VOS_ERR;
    }

    RNIC_SndNetManagerReservedPortCfgInd(msg);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvImsaSocketExceptionInd(MsgBlock *msg)
{
    RNIC_SndNetManagerSocketExceptInd(msg);
    return VOS_OK;
}

VOS_UINT32 RNIC_RcvImsaSipPortRangeInd(MsgBlock *msg)
{
    IMSA_RNIC_SipPortRangeInd *rcvInd = VOS_NULL_PTR;

    rcvInd = (IMSA_RNIC_SipPortRangeInd *)msg;

    if (rcvInd->sipPortRangeNum > IMSA_RNIC_SIP_PORT_RANGE_GROUP_MAX_NUM) {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC,
                        "RNIC_RcvImsaSipPortRangeInd: usSipPortRangeNum too large: ", rcvInd->sipPortRangeNum);
        return VOS_ERR;
    }

    RNIC_SndNetManagerSipPortRangeInd(msg);

    return VOS_OK;
}

VOS_UINT32 RNIC_ProcImsData(MsgBlock *msg)
{
    RNIC_IMS_DataProcInd *dataProcInd = VOS_NULL_PTR;
    RNIC_IFACE_Ctx       *ifaceCtx    = VOS_NULL_PTR;

    dataProcInd = (RNIC_IMS_DataProcInd *)msg;
    ifaceCtx    = RNIC_GET_IFACE_CTX_ADR(dataProcInd->netId);

    RNIC_SendVoWifiUlData(ifaceCtx);

    return VOS_OK;
}

VOS_UINT32 RNIC_RcvCdsImsDataInd(MsgBlock *msg)
{
    CDS_RNIC_ImsDataInd *imsDataInd = VOS_NULL_PTR;
    VOS_UINT32           ret;
    VOS_UINT8            rmNetId;

    imsDataInd = (CDS_RNIC_ImsDataInd *)msg;

    rmNetId = RNIC_GetImsRmnetId(IMSA_RNIC_IMS_RAT_TYPE_WIFI, imsDataInd->modemId,
                                 (IMSA_RNIC_PdnEmcIndUint8)imsDataInd->dataType);

    if (rmNetId >= RNIC_DEV_ID_BUTT) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RcvCdsImsDataInd: ucRmNetId butt.");
        return VOS_ERR;
    }

    ret = RNIC_RecvVoWifiDlData(rmNetId, imsDataInd);

    return ret;
}

VOS_VOID RNIC_FillNetManagerMsgPdnCfgInfo(NM_PDN_INFO_CONFIG_STRU *destPdnInfo, IMSA_RNIC_PdnInfoConfig *srcPdnInfo)
{
    /* 对stRnicNmMsg中参数进行赋值 */
    destPdnInfo->bitOpIpv4PdnInfo = srcPdnInfo->opIpv4PdnInfo;
    destPdnInfo->bitOpIpv6PdnInfo = srcPdnInfo->opIpv6PdnInfo;
    destPdnInfo->bitOpMtuInfo     = srcPdnInfo->opMtuInfo;

    destPdnInfo->enModemId = srcPdnInfo->modemId;
    destPdnInfo->enRatType = srcPdnInfo->ratType;
    destPdnInfo->ulMtuSize = srcPdnInfo->mtuSize;

    /* 对ipv4的pdn进行赋值 */
    if (memcpy_s(&(destPdnInfo->stIpv4PdnInfo), sizeof(NM_IPV4_PDN_INFO_STRU), &(srcPdnInfo->ipv4PdnInfo),
                 sizeof(IMSA_RNIC_Ipv4PdnInfo)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_FillNetManagerMsgPdnCfgInfo: memcpy failed!");
    }

    if (memcpy_s(&(destPdnInfo->stIpv6PdnInfo), sizeof(NM_IPV6_PDN_INFO_STRU), &(srcPdnInfo->ipv6PdnInfo),
                 sizeof(IMSA_RNIC_Ipv6PdnInfo)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_FillNetManagerMsgPdnCfgInfo: memcpy failed!");
    }
}

VOS_VOID RNIC_FillNetManagerMsgReservedPortCfgInfo(NM_RESERVED_PORTS_CONFIG_STRU *destReservedProtInfo,
                                                   IMSA_RNIC_ImsPortInfo         *srcReservedProtInfo)
{
    /* 对stRnicNmMsg中参数进行赋值 */
    destReservedProtInfo->ulImsPortRangeNum = srcReservedProtInfo->imsPortRangeNum;
    destReservedProtInfo->enModemId         = srcReservedProtInfo->modemId;

    if (memcpy_s(destReservedProtInfo->astImsPortRange,
                 (sizeof(NM_IMS_PORT_RANGE_STRU) * NM_IMS_PORT_RANGE_GROUP_MAX_NUM), srcReservedProtInfo->imsPortRange,
                 (sizeof(IMSA_RNIC_ImsPortRange) * IMSA_RNIC_IMS_PORT_RANGE_GROUP_MAX_NUM)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_FillNetManagerMsgReservedPortCfgInfo: memcpy failed!");
    }
}

VOS_VOID RNIC_FillNetManagerMsgSipPortRangeInfo(NM_SIP_PORTS_RANGE_STRU   *destSipProtRange,
                                                IMSA_RNIC_SipPortRangeInd *srcSipProtRange)
{
    /* 对stRnicNmMsg中参数进行赋值 */
    destSipProtRange->usSipPortRangeNum = srcSipProtRange->sipPortRangeNum;
    destSipProtRange->enModemId         = srcSipProtRange->modemId;

    if (memcpy_s(destSipProtRange->astSipPortRange, (sizeof(NM_IMS_PORT_RANGE_STRU) * NM_SIP_PORT_RANGE_GROUP_MAX_NUM),
                 srcSipProtRange->sipPortRange,
                 (sizeof(IMSA_RNIC_ImsPortRange) * IMSA_RNIC_SIP_PORT_RANGE_GROUP_MAX_NUM)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_FillNetManagerMsgSipPortRangeInfo: memcpy failed!");
    }
}

VOS_VOID RNIC_SndNetManagerPdpActInd(MsgBlock *msg)
{
    IMSA_RNIC_PdnActInd *rcvInd = VOS_NULL_PTR;
    NM_MSG_STRU          rnicNmMsg;

    (VOS_VOID)memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rcvInd             = (IMSA_RNIC_PdnActInd *)msg;
    rnicNmMsg.enMsgId  = ID_NM_PDN_ACT_IND;
    rnicNmMsg.ulMsgLen = sizeof(NM_PDN_INFO_CONFIG_STRU);

    RNIC_FillNetManagerMsgPdnCfgInfo(&(rnicNmMsg.stPdnCfgInfo), &(rcvInd->pdnInfo));

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}

VOS_VOID RNIC_SndNetManagerPdpDeactInd(MsgBlock *msg)
{
    IMSA_RNIC_PdnDeactInd *rcvInd;
    NM_MSG_STRU            rnicNmMsg;

    rcvInd = (IMSA_RNIC_PdnDeactInd *)msg;
    (VOS_VOID)memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rnicNmMsg.enMsgId  = ID_NM_PDN_DEACT_IND;
    rnicNmMsg.ulMsgLen = sizeof(NM_PDN_DEACT_IND_STRU);

    /* 对stRnicNmMsg中参数进行赋值 */
    rnicNmMsg.stPdnDeactInd.enModemId = rcvInd->modemId;
    rnicNmMsg.stPdnDeactInd.enRatType = rcvInd->ratType;

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}

VOS_VOID RNIC_SndNetManagerPdpModifyInd(MsgBlock *msg)
{
    IMSA_RNIC_PdnModifyInd *rcvInd = VOS_NULL_PTR;
    NM_MSG_STRU             rnicNmMsg;

    (VOS_VOID)memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rcvInd             = (IMSA_RNIC_PdnModifyInd *)msg;
    rnicNmMsg.enMsgId  = ID_NM_PDN_MODIFY_IND;
    rnicNmMsg.ulMsgLen = sizeof(NM_PDN_INFO_CONFIG_STRU);

    RNIC_FillNetManagerMsgPdnCfgInfo(&(rnicNmMsg.stPdnCfgInfo), &(rcvInd->pdnInfo));

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}

VOS_VOID RNIC_SndNetManagerModemResetInd(VOS_VOID)
{
    NM_MSG_STRU rnicNmMsg;

    (VOS_VOID)memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rnicNmMsg.enMsgId  = ID_NM_MODEM_RESET_IND;
    rnicNmMsg.ulMsgLen = 0;

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}

VOS_VOID RNIC_SndNetManagerReservedPortCfgInd(MsgBlock *msg)
{
    IMSA_RNIC_ReservedPortsConfigInd *rcvInd = VOS_NULL_PTR;
    NM_MSG_STRU                       rnicNmMsg;

    (VOS_VOID)memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rcvInd             = (IMSA_RNIC_ReservedPortsConfigInd *)msg;
    rnicNmMsg.enMsgId  = ID_NM_RESERVED_PORTS_CONFIG_IND;
    rnicNmMsg.ulMsgLen = sizeof(NM_RESERVED_PORTS_CONFIG_STRU);

    RNIC_FillNetManagerMsgReservedPortCfgInfo(&(rnicNmMsg.stPortsCfgInfo), &(rcvInd->imsPortInfo));

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}

VOS_VOID RNIC_SndNetManagerSocketExceptInd(MsgBlock *msg)
{
    IMSA_RNIC_SocketExceptionInd *rcvInd = VOS_NULL_PTR;
    NM_MSG_STRU                   rnicNmMsg;

    memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rcvInd             = (IMSA_RNIC_SocketExceptionInd *)msg;
    rnicNmMsg.enMsgId  = ID_NM_SOCKET_EXCEPTION_IND;
    rnicNmMsg.ulMsgLen = sizeof(NM_SOCKET_EXCEPTION_IND_STRU);

    /* 消息体赋值 */
    rnicNmMsg.stSocketExceptionInd.sSocketErrorNo = rcvInd->socketErrorNo;
    RNIC_FillNetManagerMsgPdnCfgInfo(&(rnicNmMsg.stSocketExceptionInd.stPdnInfo), &(rcvInd->pdnInfo));

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}

VOS_VOID RNIC_SndNetManagerSipPortRangeInd(MsgBlock *msg)
{
    IMSA_RNIC_SipPortRangeInd *rcvInd = VOS_NULL_PTR;
    NM_MSG_STRU                rnicNmMsg;

    memset_s(&rnicNmMsg, sizeof(rnicNmMsg), 0x00, sizeof(rnicNmMsg));

    rcvInd             = (IMSA_RNIC_SipPortRangeInd *)msg;
    rnicNmMsg.enMsgId  = ID_NM_SIP_PORT_RANGE_IND;
    rnicNmMsg.ulMsgLen = sizeof(NM_SIP_PORTS_RANGE_STRU);

    RNIC_FillNetManagerMsgSipPortRangeInfo(&(rnicNmMsg.stSipPortRangeInd), rcvInd);

    /* 调用虚拟设备提供的发送接口发送消息 */
    NM_CTRL_SEND_MSG(&rnicNmMsg, sizeof(NM_MSG_STRU));
}
#endif /* FEATURE_ON == FEATURE_IMS */

RNIC_PROC_MSG_FUNC RNIC_GetProcMsgFunc(VOS_UINT32 msgId, VOS_UINT32 senderPid)
{
    VOS_UINT32 i;
    VOS_UINT32 tableSize;

    /* 获取处理操作个数 */
    tableSize = sizeof(g_rnicMsgProcTab) / sizeof(RNIC_PROC_Msg);

    /* g_rnicMsgProcTab查表，进行消息处理的分发 */
    for (i = 0; i < tableSize; i++) {
        if ((g_rnicMsgProcTab[i].msgId == msgId) && (g_rnicMsgProcTab[i].senderPid == senderPid)) {
            return g_rnicMsgProcTab[i].rnicProcMsgFunc;
        }
    }

    RNIC_ERROR_LOG2(ACPU_PID_RNIC, "RNIC_GetProcMsgFunc:SendPid and MsgId is", senderPid, msgId);

    return VOS_NULL_PTR;
}

VOS_UINT32 RNIC_GetModem0Pid(VOS_UINT32 senderPid)
{
    VOS_UINT32 modem0Pid;

    switch (senderPid) {
        case I0_PS_PID_IMSA:
        case I1_PS_PID_IMSA:
            modem0Pid = PS_PID_IMSA;
            break;

        case I0_UEPS_PID_DSM:
        case I1_UEPS_PID_DSM:
        case I2_UEPS_PID_DSM:
            modem0Pid = UEPS_PID_DSM;
            break;

        default:
            modem0Pid = senderPid;
            break;
    }

    return modem0Pid;
}

VOS_VOID RNIC_ProcMsg(MsgBlock *msg)
{
    MSG_Header        *msgHeader   = VOS_NULL_PTR;
    REL_TimerMsgBlock *timerMsg    = VOS_NULL_PTR;
    RNIC_PROC_MSG_FUNC procMsgFunc = VOS_NULL_PTR;
    VOS_UINT32         rst;
    VOS_UINT32         msgName;
    VOS_UINT32         senderPid;

    if (msg == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_ProcMsg: Msg is invalid!");
        return;
    }

    msgHeader = (MSG_Header *)msg;

    senderPid = VOS_GET_SENDER_ID(msgHeader);

    /* 超时消息显式停止定时器 */
    if (senderPid == VOS_PID_TIMER) {
        timerMsg = (REL_TimerMsgBlock *)msg;
        msgName  = timerMsg->name;
        RNIC_StopTimer((VOS_UINT16)msgName);
    } else {
        msgName = msgHeader->msgName;
    }

    /* 将SenderPid 统一转为modem0 Pid */
    senderPid = RNIC_GetModem0Pid(senderPid);

    /* 得到消息处理函数 */
    procMsgFunc = RNIC_GetProcMsgFunc(msgName, senderPid);

    if (procMsgFunc != VOS_NULL_PTR) {
        rst = procMsgFunc(msg);
        if (rst == VOS_ERR) {
            RNIC_NORMAL_LOG(ACPU_PID_RNIC, "RNIC_ProcMsgEntry:proc func error!");
            return;
        }
    } else {
        RNIC_ERROR_LOG2(ACPU_PID_RNIC, "RNIC_ProcMsgEntry:SendPid and MsgId is", senderPid, msgName);
        return;
    }
}

VOS_UINT32 RNIC_SendDialInfoMsg(RNIC_IntraMsgIdUint32 msgId)
{
    RNIC_NotifyMsg    *dialInfo = VOS_NULL_PTR;
    RNIC_DialModePara *dialMode = VOS_NULL_PTR;

    /* 内存分配 */
    dialInfo = (RNIC_NotifyMsg *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_NotifyMsg));

    if (dialInfo == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendDialInfoMsg: Malloc failed!");
        return VOS_ERR;
    }

    dialMode = RNIC_GET_DIAL_MODE_ADR();

    /* 填充消息 */
    RNIC_CFG_INTRA_MSG_HDR(dialInfo, msgId);

    if (memcpy_s(&(dialInfo->dialInfo), sizeof(dialInfo->dialInfo), dialMode, sizeof(RNIC_DialModePara)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendDialInfoMsg: memcpy_s is failed.");
    }

    if (RNIC_SendMsg(dialInfo) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendDialInfoMsg: Send msg failed!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID RNIC_IfaceUpStub(VOS_UINT16 modemId, VOS_UINT8 ifaceId, VOS_UINT8 pduSessionId, VOS_UINT8 sessType)
{
    DSM_RNIC_IfaceCfgInd pdnInfoCfgInd;

    memset_s(&pdnInfoCfgInd, sizeof(pdnInfoCfgInd), 0x00, sizeof(pdnInfoCfgInd));

    pdnInfoCfgInd.sessType      = sessType;
    pdnInfoCfgInd.modemId       = modemId;
    pdnInfoCfgInd.pduSessionId  = pduSessionId;
    pdnInfoCfgInd.ifaceId       = ifaceId;

    RNIC_RcvDsmPdnInfoCfgInd((MsgBlock *)&pdnInfoCfgInd);
}

VOS_VOID RNIC_IfaceDownStub(VOS_UINT8 ifaceId, VOS_UINT8 sessType)
{
    DSM_RNIC_IfaceRelInd pdnInfoRelInd;

    memset_s(&pdnInfoRelInd, sizeof(pdnInfoRelInd), 0x00, sizeof(pdnInfoRelInd));

    pdnInfoRelInd.sessType = sessType;
    pdnInfoRelInd.ifaceId  = ifaceId;

    RNIC_RcvDsmPdnInfoRelInd((MsgBlock *)&pdnInfoRelInd);
}

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))

VOS_VOID RNIC_IWlanIfaceUpStub(VOS_UINT16 modemId)
{
    IMSA_RNIC_PdnInfoConfig pdnInfo;

    memset_s(&pdnInfo, sizeof(pdnInfo), 0x00, sizeof(pdnInfo));

    pdnInfo.opIpv4PdnInfo = VOS_TRUE;
    pdnInfo.modemId       = modemId;
    pdnInfo.pduSessionId  = 0;
    pdnInfo.ratType       = IMSA_RNIC_IMS_RAT_TYPE_WIFI;
    pdnInfo.emcInd        = IMSA_RNIC_PDN_NOT_FOR_EMC;

    RNIC_ProcImsaPdnActInd_Wifi(&pdnInfo);
}

VOS_VOID RNIC_IWlanIfaceDownStub(VOS_UINT16 modemId)
{
    RNIC_ProcImsaPdnDeactInd_Wifi(modemId, IMSA_RNIC_PDN_NOT_FOR_EMC);
}
#endif /* FEATURE_ON == FEATURE_IMS && FEATURE_ON == FEATURE_ACPU_PHONE_MODE */

