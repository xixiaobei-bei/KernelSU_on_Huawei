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

#include "ads_ndis_interface.h"
#include "cds_ads_interface.h"
#include "dsm_ads_pif.h"
#include "imsa_ads_interface.h"
#include "ads_ctx.h"
#include "ads_mntn.h"
#include "ads_public.h"
#include "acpu_reset.h"
#include "ads_msg_proc.h"
#include "ads_log.h"
#include "ads_iface_mgmt.h"
#include "ads_iface_i.h"
#include "ads_iface_debug.h"
#include "ads_intra_msg.h"
#include "mn_comm_api.h"
#include "mdrv_espe.h"
#include "securec.h"
#include "ads_msg_chk.h"
#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
#include "ads_iface_filter.h"
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_ADS_MSG_PROC_C

VOS_UINT8 ADS_TransDsmAddrType2Mask(VOS_UINT8 ipType)
{
    VOS_UINT8 addrFamilyMask = 0;

    switch (ipType) {
        case DSM_ADS_SESSION_TYPE_IPV4:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV4_ADDR);
            break;

        case DSM_ADS_SESSION_TYPE_IPV6:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV6_ADDR);
            break;

        case DSM_ADS_SESSION_TYPE_IPV4V6:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV4_ADDR);
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV6_ADDR);
            break;

        case DSM_ADS_SESSION_TYPE_RAW:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_RAW_ADDR);
            break;

        case DSM_ADS_SESSION_TYPE_ETHERNET:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_ETH_ADDR);
            break;

        default:
            break;
    }

    return addrFamilyMask;
}

VOS_UINT8 ADS_GetTransModefromDsmAddrType(VOS_UINT8 ipType)
{
    switch (ipType) {
        case DSM_ADS_SESSION_TYPE_IPV4:
        case DSM_ADS_SESSION_TYPE_IPV6:
        case DSM_ADS_SESSION_TYPE_IPV4V6:
            return ADS_TRANS_MODE_IP;

        case DSM_ADS_SESSION_TYPE_RAW:
            return ADS_TRANS_MODE_RAW;

        case DSM_ADS_SESSION_TYPE_ETHERNET:
            return ADS_TRANS_MODE_ETH;

        default:
            return ADS_TRANS_MODE_BUTT;
    }
}

ADS_NDIS_IpPacketTypeUint8 ADS_TransUlBid2IpPktType(CDS_ADS_UlIpfBidUint8 bid)
{
    ADS_NDIS_IpPacketTypeUint8 ipType;

    switch (bid) {
        case CDS_ADS_UL_IPF_BID_DHCPV4:
            ipType = ADS_NDIS_IP_PACKET_TYPE_DHCPV4;
            break;

        case CDS_ADS_UL_IPF_BID_DHCPV6:
            ipType = ADS_NDIS_IP_PACKET_TYPE_DHCPV6;
            break;

        case CDS_ADS_UL_IPF_BID_ICMPV6:
            ipType = ADS_NDIS_IP_PACKET_TYPE_ICMPV6;
            break;

        case CDS_ADS_UL_IPF_BID_LL_FE80:
            ipType = ADS_NDIS_IP_PACKET_TYPE_LINK_FE80;
            break;

        case CDS_ADS_UL_IPF_BID_LL_FF:
            ipType = ADS_NDIS_IP_PACKET_TYPE_LINK_FF;
            break;

        default:
            ipType = ADS_NDIS_IP_PACKET_TYPE_BUTT;
            break;
    }

    return ipType;
}

VOS_VOID ADS_SndEvent(VOS_UINT32 event)
{
    if (ADS_GetTaskReadyFlag() == VOS_TRUE) {
        (VOS_VOID)VOS_EventWrite(ADS_GetTaskId(), event);
    }
}

VOS_UINT32 ADS_SendMsg(VOS_VOID *msg)
{
    VOS_UINT32  ret;

    ret = PS_SEND_MSG(ACPU_PID_ADS_UL, msg);
    return ret;
}

VOS_VOID ADS_SndCdsDataInd(IMM_Zc *zcData, struct rx_cb_map_s *mapInfo)
{
    ADS_CDS_DataIndV2 *msg  = VOS_NULL_PTR;
    VOS_UINT8         *data = VOS_NULL_PTR;
    VOS_UINT32         datalen;

    if (zcData == VOS_NULL_PTR) {
        ADS_ERROR_LOG("ADS_SndCdsDataInd: pstZcData is NULL!\n");
        return;
    }

    datalen = IMM_ZcGetUsedLen(zcData);

    /* 申请OSA消息 */
    msg = (ADS_CDS_DataIndV2 *)ADS_ALLOC_MSG_WITH_HDR(sizeof(ADS_CDS_DataIndV2) + datalen);
    if (msg == VOS_NULL_PTR) {
        ADS_ERROR_LOG("ADS_SndCdsDataInd: Alloc MSG failed.\n");
        return;
    }

    /* 清空消息内容 */
    if (memset_s(ADS_GET_MSG_ENTITY(msg), ADS_GET_MSG_LENGTH(msg), 0x00, ADS_GET_MSG_LENGTH(msg)) != EOK) {
        ADS_ERROR_LOG("ADS_SndCdsDataInd: memset failed!");
    }

    /* 填写消息头 */
    ADS_CFG_CDS_MSG_HDR(msg, ID_ADS_CDS_DATA_IND_V2);

    /* 填写消息内容 */
    msg->ifaceId      = (VOS_UINT8)mapInfo->userfield0;
    msg->bid          = mapInfo->ipf_result.bits.bid_qosid;
    msg->pduSessionId = mapInfo->ipf_result.bits.pdu_session_id;
    msg->modemId      = mapInfo->ipf_result.bits.modem_id;
    msg->len          = (VOS_UINT16)datalen;
    data              = IMM_ZcGetDataPtr(zcData);

    if (memcpy_s(msg->data, datalen, data, datalen) != EOK) {
        ADS_ERROR_LOG("ADS_SndCdsDataInd: memcpy failed!");
    }

    /* 发送消息 */
    if (ADS_SendMsg(msg) != VOS_OK) {
        ADS_ERROR_LOG("ADS_SndCdsDataInd: Send message fail!\n");
    }
}

VOS_VOID ADS_SndDplConfigInd(VOS_BOOL bDplNeededFlg)
{
    ADS_DplConfigInd *dplCfgInd = VOS_NULL_PTR;

    /* 构造消息 */
    dplCfgInd = (ADS_DplConfigInd *)ADS_ALLOC_MSG_WITH_HDR(sizeof(ADS_DplConfigInd));
    if (dplCfgInd == VOS_NULL_PTR) {
        ADS_ERROR_LOG("ADS_SndDplNeededFlgChgInd: Alloc MSG failed.\n");
        return;
    }

    /* 清空消息内容 */
    if (memset_s(ADS_GET_MSG_ENTITY(dplCfgInd), ADS_GET_MSG_LENGTH(dplCfgInd), 0x00, ADS_GET_MSG_LENGTH(dplCfgInd)) !=
        EOK) {
        ADS_ERROR_LOG("ADS_SndDplNeededFlgChgInd: memset failed.\n");
    }

    /* 填写消息头 */
    ADS_CFG_MSG_HDR(dplCfgInd, ACPU_PID_ADS_UL, ID_ADS_DPL_CONFIG_IND);

    dplCfgInd->dplNeededFlg = bDplNeededFlg;

    /* 发消息 */
    (VOS_VOID)ADS_SendMsg(dplCfgInd);
}

STATIC VOS_UINT32 ADS_RcvCdsDataInd(MsgBlock *msg)
{
    ADS_NDIS_DataIndV2 *ndisDataInd = VOS_NULL_PTR;
    CDS_ADS_DataIndV2  *cdsDataInd  = VOS_NULL_PTR;
    IMM_Zc             *immZc       = VOS_NULL_PTR;
    VOS_CHAR           *pcData      = VOS_NULL_PTR;
    VOS_UINT32          result;

    cdsDataInd = (CDS_ADS_DataIndV2 *)msg;

    /* 申请消息  */
    ndisDataInd = (ADS_NDIS_DataIndV2 *)ADS_ALLOC_MSG_WITH_HDR(sizeof(ADS_NDIS_DataIndV2));
    if (ndisDataInd == VOS_NULL_PTR) {
        ADS_ERROR_LOG("ADS_RcvCdsDataInd: Alloc message failed!");
        return VOS_ERR;
    }

    if (memset_s(ADS_GET_MSG_ENTITY(ndisDataInd), ADS_GET_MSG_LENGTH(ndisDataInd), 0x00,
                 ADS_GET_MSG_LENGTH(ndisDataInd)) != EOK) {
        ADS_ERROR_LOG("ADS_RcvCdsDataInd: memset_s failed!");
    }

    /* 填写消息头 */
    ADS_CFG_NDIS_MSG_HDR(ndisDataInd, ID_ADS_NDIS_DATA_IND_V2);

    /* 填写消息内容 */
    ndisDataInd->ifaceId      = cdsDataInd->ifaceId;
    ndisDataInd->ipPacketType = ADS_TransUlBid2IpPktType(cdsDataInd->bid);

    immZc = (IMM_Zc *)IMM_ZcStaticAlloc((VOS_UINT32)cdsDataInd->len);
    if (immZc == VOS_NULL_PTR) {
        ADS_ERROR_LOG("ADS_RcvCdsDataInd: IMM_ZcStaticAlloc failed!");
        PS_FREE_MSG(ACPU_PID_ADS_UL, ndisDataInd);
        ndisDataInd = VOS_NULL_PTR;
        return VOS_ERR;
    }

    ndisDataInd->data = immZc;

    pcData = (VOS_CHAR *)IMM_ZcPut(immZc, cdsDataInd->len);

    if (memcpy_s(pcData, IMM_ZcGetUsedLen(immZc), cdsDataInd->data, cdsDataInd->len) != EOK) {
        ADS_ERROR_LOG("ADS_RcvCdsDataInd: memcpy failed!");
    }

    /* 调用VOS发送原语 */
    result = ADS_SendMsg(ndisDataInd);
    if (result != VOS_OK) {
        ADS_ERROR_LOG("ADS_RcvCdsDataInd: Send Msg failed!");
        IMM_ZcFreeAny(immZc); /* 由操作系统接管 */
        return VOS_ERR;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 ADS_FindSharedIfaceId(VOS_UINT8 pduSessionId)
{
#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
    ADS_IFACE_Info *adsIfaceInfo = VOS_NULL_PTR;
    ADS_CONTEXT    *adsCtx = ADS_GetCtx();
    VOS_UINT32      ifaceId;
    VOS_UINT32      addrType;

    for (ifaceId = 0; ifaceId < PS_IFACE_ID_BUTT; ifaceId++) {
        if (!ADS_BIT64_IS_SET(adsCtx->ifaceStatsBitMask, ifaceId)) {
            continue;
        }

        adsIfaceInfo = ADS_GetIfaceInfo((VOS_UINT8)ifaceId);
        for (addrType = 0; addrType < ADS_ADDR_BUTT; addrType++) {
            if (!ADS_BIT8_IS_SET(adsIfaceInfo->addrFamilyMask, addrType)) {
                continue;
            }

            /* 已激活网卡的sessionId与本次激活网卡的相同，则为共享场景 */
            if (adsIfaceInfo->pduSessionTbl[addrType] == pduSessionId) {
                return ifaceId;
            }
        }
    }
#endif

    return PS_IFACE_ID_BUTT;
}

STATIC VOS_UINT32 ADS_RcvDsmIfaceConfigInd(MsgBlock *msg)
{
    DSM_ADS_IfaceConfigInd *ifaceConfigInd = VOS_NULL_PTR;
    DSM_ADS_IfaceInfo      *ifaceInfo      = VOS_NULL_PTR;
    ADS_IFACE_Config        ifaceConfig;
    VOS_UINT32              i;

    ifaceConfigInd = (DSM_ADS_IfaceConfigInd *)msg;
    ifaceInfo      = &(ifaceConfigInd->ifaceInfo);

    if (ifaceInfo->ifaceId >= PS_IFACE_ID_BUTT) {
        ADS_ERROR_LOG("ADS_RcvDsmIfaceConfigInd: RmNetId is invaild!");
        return VOS_ERR;
    }

    /* ModemID错误 */
    if (ifaceInfo->modemId >= MODEM_ID_BUTT) {
        ADS_ERROR_LOG("ADS_RcvDsmIfaceConfigInd: ModemID is invaild!");
        return VOS_ERR;
    }

    for (i = 0; i < DSM_ADS_MAX_PDU_SESSION_NUM; i++) {
        /* Ip类型错误 */
        if (ifaceConfigInd->ifaceInfo.pduSessionInfo[i].sessType >= DSM_ADS_SESSION_TYPE_ENUM_BUTT) {
            ADS_ERROR_LOG1("ADS_RcvDsmIfaceConfigInd: SessType is invaild! Session Type is",
                           ifaceConfigInd->ifaceInfo.pduSessionInfo[i].sessType);
            continue;
        }

        (VOS_VOID)memset_s(&ifaceConfig, sizeof(ifaceConfig), 0x00, sizeof(ifaceConfig));

        ifaceConfig.ifaceId        = ifaceInfo->ifaceId;
        ifaceConfig.ifaceOpType    = ifaceInfo->opType;
        ifaceConfig.modemId        = ifaceInfo->modemId;
        ifaceConfig.addrFamilyMask = ADS_TransDsmAddrType2Mask(ifaceInfo->pduSessionInfo[i].sessType);
        ifaceConfig.pduSessionId   = ifaceInfo->pduSessionInfo[i].pduSessionId;
        ifaceConfig.transMode      = ADS_GetTransModefromDsmAddrType(ifaceInfo->pduSessionInfo[i].sessType);

        ifaceConfig.sharedIfaceId  = ADS_FindSharedIfaceId(ifaceInfo->pduSessionInfo[i].pduSessionId);


        /* 网卡接口配置 */
        ADS_IfaceConfig(&ifaceConfig);
    }

    return VOS_OK;
}

#if (FEATURE_SHARE_APN == FEATURE_ON)

STATIC VOS_UINT32 ADS_RcvDsmEspeTransferConfigInd(MsgBlock *msg)
{
    DSM_ADS_EspeTransferConfigInd *espeConfigInd = VOS_NULL_PTR;

    espeConfigInd = (DSM_ADS_EspeTransferConfigInd *)msg;

    switch (espeConfigInd->opType) {
        case DSM_ADS_ESPE_TRANSFER_OP_TYPE_DISABLE:
            mdrv_espe_transfer_pause();
            break;
        case DSM_ADS_ESPE_TRANSFER_OP_TYPE_ENABLE:
            mdrv_espe_transfer_restart();
            break;
        default:
            ADS_ERROR_LOG("ADS_RcvDsmEspeTransferConfigInd: opType error.");
            break;
    }

    return VOS_OK;
}
#endif

#if (FEATURE_ON == FEATURE_IMS)

VOS_UINT8 ADS_GetTransModefromImsaAddrType(VOS_UINT8 ipType)
{
    switch (ipType) {
        case IMSA_ADS_IP_TYPE_IPV4:
        case IMSA_ADS_IP_TYPE_IPV6:
        case IMSA_ADS_IP_TYPE_IPV4V6:

            return ADS_TRANS_MODE_IP;

        case IMSA_ADS_IP_TYPE_NON_IP:

            return ADS_TRANS_MODE_RAW;

        default:

            return ADS_TRANS_MODE_BUTT;
    }
}

VOS_UINT8 ADS_TransImsaAddrType2Mask(VOS_UINT8 ipType)
{
    VOS_UINT8 addrFamilyMask = 0;

    switch (ipType) {
        case IMSA_ADS_IP_TYPE_IPV4:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV4_ADDR);
            break;

        case IMSA_ADS_IP_TYPE_IPV6:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV6_ADDR);
            break;

        case IMSA_ADS_IP_TYPE_IPV4V6:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV4_ADDR);
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_IPV6_ADDR);
            break;

        case IMSA_ADS_IP_TYPE_NON_IP:
            addrFamilyMask = ADS_BIT8_SET(addrFamilyMask, ADS_RAW_ADDR);
            break;

        default:
            break;
    }

    return addrFamilyMask;
}

STATIC VOS_UINT32 ADS_RcvImsaIfaceConfigInd(MsgBlock *msg)
{
    IMSA_ADS_IfaceConfigInd *ifaceConfigInd = VOS_NULL_PTR;
    IMSA_ADS_IfaceInfo      *ifaceInfo      = VOS_NULL_PTR;
    ADS_IFACE_Config         ifaceConfig;
    VOS_UINT32               ifaceNum;
    VOS_UINT32               pduSessionNum;

    ifaceConfigInd = (IMSA_ADS_IfaceConfigInd *)msg;

    if (ifaceConfigInd->ifaceNum > IMSA_ADS_MAX_IFACE_NUM) {
        ADS_ERROR_LOG("ADS_RcvImsaIfaceConfigInd: IfaceNum is invaild!");
        return VOS_ERR;
    }

    for (ifaceNum = 0; ifaceNum < ifaceConfigInd->ifaceNum; ifaceNum++) {
        ifaceInfo = &(ifaceConfigInd->ifaceInfo[ifaceNum]);

        /* PDU Session 错误 */
        if (ifaceInfo->pduSessionNum > IMSA_ADS_MAX_PDU_SESSION_NUM) {
            ADS_ERROR_LOG("ADS_RcvImsaIfaceConfigInd: IfaceNum is invaild!");
            continue;
        }

        /* ModemID错误 */
        if (ifaceInfo->modemId >= MODEM_ID_BUTT) {
            ADS_ERROR_LOG("ADS_RcvImsaIfaceConfigInd: ModemID is invaild!");
            continue;
        }

        for (pduSessionNum = 0; pduSessionNum < ifaceInfo->pduSessionNum; pduSessionNum++) {
            /* Ip类型错误 */
            if (ifaceInfo->pduSession[pduSessionNum].ipType >= IMSA_ADS_IP_TYPE_ENUM_BUTT) {
                ADS_ERROR_LOG("ADS_RcvImsaIfaceConfigInd: IpType is invaild!");
                continue;
            }

            (VOS_VOID)memset_s(&ifaceConfig, sizeof(ifaceConfig), 0x00, sizeof(ifaceConfig));

            ifaceConfig.ifaceId        = ifaceInfo->rmNetId;
            ifaceConfig.ifaceOpType    = ifaceInfo->opType;
            ifaceConfig.modemId        = ifaceInfo->modemId;
            ifaceConfig.pduSessionId   = ifaceInfo->pduSession[pduSessionNum].pduSessionId;
            ifaceConfig.addrFamilyMask = ADS_TransImsaAddrType2Mask(ifaceInfo->pduSession[pduSessionNum].ipType);
            ifaceConfig.transMode      = ADS_GetTransModefromImsaAddrType(ifaceInfo->pduSession[pduSessionNum].ipType);
#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
            ifaceConfig.sharedIfaceId  = PS_IFACE_ID_BUTT;
#endif
            /* 配置网卡接口 */
            ADS_IfaceConfig(&ifaceConfig);
        }
    }

    return VOS_OK;
}

STATIC VOS_UINT32 ADS_RcvImsaMsg(MsgBlock *msg)
{
    MSG_Header *msgTemp;

    msgTemp = (MSG_Header *)msg;

    switch (msgTemp->msgName) {
        case ID_IMSA_ADS_IFACE_CONFIG_IND:
            ADS_RcvImsaIfaceConfigInd(msg);
            break;

        default:
            ADS_ERROR_LOG1("ADS_RcvDsmMsg: rcv error msg id %d\r\n", msgTemp->msgName);
            break;
    }

    return VOS_OK;
}

#endif /* FEATURE_IMS */

STATIC VOS_UINT32 ADS_RcvCcpuResetStartInd(MsgBlock *msg)
{
    struct ads_iface_config_s ifaceInfo;
    VOS_UINT32                tiIndex;
    VOS_UINT32                ifaceId;
    VOS_UINT32                i;

    ADS_LOGH("enter.");

    /* 停止所有启动的定时器 */
    for (tiIndex = 0; tiIndex < ADS_MAX_TIMER_NUM; tiIndex++) {
        ADS_StopTimer(tiIndex, ADS_TIMER_STOP_CAUSE_USER);
    }

    /* 重置网络接口上下文 */
    ADS_AllIfaceInfoInit();

    /* 网卡接口配置为DOWN */
    memset_s(&ifaceInfo, sizeof(ifaceInfo), 0x00, sizeof(ifaceInfo));
    for (ifaceId = 0; ifaceId < PS_IFACE_ID_BUTT; ifaceId++) {
        for (i = 0; i < ADS_ADDR_BUTT; i++) {
            ifaceInfo.addr_family = (VOS_UINT8)i;
            ads_iface_bring_down((VOS_UINT8)ifaceId, &ifaceInfo);
        }
    }

    /* 初始化特殊包队列 */
    IMM_ZcQueueHeadInit(ADS_GetDataQue());

#if ((FEATURE_SHARE_APN == FEATURE_ON) || (FEATURE_SHARE_PDP == FEATURE_ON))
    ads_iface_filter_ctx_reset();
#endif
    /* 释放信号量，使得调用API任务继续运行 */
    VOS_SmV(ADS_GetResetSem());

    ADS_LOGH("leave.");
    return VOS_OK;
}

VOS_UINT32 ADS_RcvDplConfigInd(MsgBlock *msg)
{
    ADS_DplConfigInd *dplCfgMsg = VOS_NULL_PTR;

    dplCfgMsg = (ADS_DplConfigInd *)msg;

    ads_iface_dpl_set_needed_flg((bool)dplCfgMsg->dplNeededFlg);

    return VOS_OK;
}

STATIC VOS_VOID ADS_RcvTiDsFlowStatsExpired(VOS_VOID)
{
    /* 维测信息勾包 */
    ADS_MntnReportAllStatsInfo();

    RNIC_MntnReportAllStatsInfo();

    /* 清除流量统计信息 */
    ads_iface_clear_dsflow_stats();

    /* 启动流量统计定时器 */
    ADS_StartTimer(TI_ADS_DSFLOW_STATS, TI_ADS_DSFLOW_STATS_LEN);
}

STATIC VOS_VOID ADS_ProcSpecialDataEvent(VOS_VOID)
{
    IMM_Zc *immZc = VOS_NULL_PTR;

    for (;;) {
        immZc = IMM_ZcDequeueHead(ADS_GetDataQue());
        if (immZc == VOS_NULL_PTR) {
            break;
        }

        ADS_ProcSpecialData(immZc);
    }
}

VOS_VOID ADS_EventProc(VOS_UINT32 event)
{
    if (event & ADS_EVENT_SPECIAL_DATA_IND) {
        ADS_ProcSpecialDataEvent();
    }
}

STATIC VOS_UINT32 ADS_RcvCdsMsg(MsgBlock *msg)
{
    MSG_Header *msgTemp;

    msgTemp = (MSG_Header *)msg;

    switch (msgTemp->msgName) {
        case ID_CDS_ADS_DATA_IND_V2:
            ADS_RcvCdsDataInd(msg);
            break;

        default:
            ADS_NORMAL_LOG1("ADS_RcvCdsMsg: rcv error msg id %d\r\n", msgTemp->msgName);
            break;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 ADS_RcvTimerMsg(MsgBlock *msg)
{
    REL_TimerMsgBlock *timerMsg;

    timerMsg = (REL_TimerMsgBlock *)msg;

    /* 停止该定时器 */
    ADS_StopTimer(timerMsg->name, ADS_TIMER_STOP_CAUSE_TIMEOUT);

    switch (timerMsg->name) {
        case TI_ADS_DSFLOW_STATS:
            ADS_RcvTiDsFlowStatsExpired();
            break;

        default:
            ADS_NORMAL_LOG1("ADS_RcvTimerMsg: rcv error msg id %d\r\n", timerMsg->name);
            break;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 ADS_RcvIntraMsg(MsgBlock *msg)
{
    MSG_Header *msgTemp;

    msgTemp = (MSG_Header *)msg;

    switch (msgTemp->msgName) {
        case ID_ADS_CCPU_RESET_START_IND:
            ADS_RcvCcpuResetStartInd(msg);
            break;

        case ID_ADS_CCPU_RESET_END_IND:
            /* do nothing */
            ADS_NORMAL_LOG("ADS_RcvIntraMsg: rcv ID_ADS_CCPU_RESET_END_IND");
            break;

        case ID_ADS_DPL_CONFIG_IND:
            ADS_RcvDplConfigInd(msg);
            break;

        default:
            ADS_NORMAL_LOG1("ADS_RcvIntraMsg: rcv error msg id %d\r\n", msgTemp->msgName);
            break;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 ADS_RcvDsmMsg(MsgBlock *msg)
{
    MSG_Header *msgTemp;

    msgTemp = (MSG_Header *)msg;

    switch (msgTemp->msgName) {
        case ID_DSM_ADS_IFACE_CONFIG_IND:
            ADS_RcvDsmIfaceConfigInd(msg);
            break;
#if (FEATURE_SHARE_APN == FEATURE_ON)
        case ID_DSM_ADS_ESPE_TRANSFER_CONFIG_IND:
            ADS_RcvDsmEspeTransferConfigInd(msg);
            break;
#endif
        default:
            ADS_ERROR_LOG1("ADS_RcvDsmMsg: rcv error msg id %d\r\n", msgTemp->msgName);
            break;
    }

    return VOS_OK;
}

VOS_VOID ADS_ProcMsg(MsgBlock *msg)
{
    if (msg == VOS_NULL_PTR) {
        return;
    }

    if (TAF_ChkAdsUlFidRcvMsgLen(msg) != VOS_TRUE) {
        ADS_ERROR_LOG("ADS_ProcMsg: message length is invalid!");
        return;
    }

    /* 消息的分发处理 */
    switch (VOS_GET_SENDER_ID(msg)) {
        /* 来自Timer的消息 */
        case VOS_PID_TIMER:
            ADS_RcvTimerMsg(msg);
            return;

        /* 来自CDS的消息 */
        case UEPS_PID_CDS:
            ADS_RcvCdsMsg(msg);
            return;

        /* 来自ADS UL的消息 */
        case ACPU_PID_ADS_UL:
            ADS_RcvIntraMsg(msg);
            return;

        /* 来自DSM的消息 */
        case I0_UEPS_PID_DSM:
        case I1_UEPS_PID_DSM:
        case I2_UEPS_PID_DSM:
            ADS_RcvDsmMsg(msg);
            return;

#if (FEATURE_ON == FEATURE_IMS)
        /* 来自IMSA的消息 */
        case I0_PS_PID_IMSA:
        case I1_PS_PID_IMSA:
            ADS_RcvImsaMsg(msg);
            return;
#endif

        default:
            ADS_ERROR_LOG1("ADS_ProcMsg: rcv error pid msg %d\r\n", VOS_GET_SENDER_ID(msg));
            return;
    }
}

VOS_VOID ADS_IfaceUpStub(VOS_UINT16 modemId, VOS_UINT8 ifaceId, VOS_UINT8 pduSessionId, DSM_ADS_SessionTypeUint8 sessType)
{
    DSM_ADS_IfaceConfigInd ifaceConfigInd;

    (VOS_VOID)memset_s(&ifaceConfigInd, sizeof(ifaceConfigInd), 0x00, sizeof(ifaceConfigInd));

    ifaceConfigInd.msgId                                    = ID_DSM_ADS_IFACE_CONFIG_IND;
    ifaceConfigInd.ifaceInfo.modemId                        = modemId;
    ifaceConfigInd.ifaceInfo.ifaceId                        = ifaceId;
    ifaceConfigInd.ifaceInfo.opType                         = DSM_ADS_IFACE_OP_TYPE_UP;
    ifaceConfigInd.ifaceInfo.pduSessionInfo[0].sessType     = sessType;
    ifaceConfigInd.ifaceInfo.pduSessionInfo[0].pduSessionId = pduSessionId;

    ADS_RcvDsmIfaceConfigInd((MsgBlock *)&ifaceConfigInd);
}

VOS_VOID ADS_IfaceDownStub(VOS_UINT16 modemId, VOS_UINT8 ifaceId, VOS_UINT8 pduSessionId, DSM_ADS_SessionTypeUint8 sessType)
{
    DSM_ADS_IfaceConfigInd ifaceConfigInd;

    (VOS_VOID)memset_s(&ifaceConfigInd, sizeof(ifaceConfigInd), 0x00, sizeof(ifaceConfigInd));

    ifaceConfigInd.msgId                                    = ID_DSM_ADS_IFACE_CONFIG_IND;
    ifaceConfigInd.ifaceInfo.modemId                        = modemId;
    ifaceConfigInd.ifaceInfo.ifaceId                        = ifaceId;
    ifaceConfigInd.ifaceInfo.opType                         = ADS_IFACE_OP_TYPE_DOWN;
    ifaceConfigInd.ifaceInfo.pduSessionInfo[0].sessType     = sessType;
    ifaceConfigInd.ifaceInfo.pduSessionInfo[0].pduSessionId = pduSessionId;

    ADS_RcvDsmIfaceConfigInd((MsgBlock *)&ifaceConfigInd);
}

