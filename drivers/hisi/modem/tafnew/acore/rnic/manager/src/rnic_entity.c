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

#include "rnic_entity.h"
#include "rnic_msg_proc.h"
#include "rnic_log.h"
#include "rnic_mntn.h"
#include "rnic_iface_ondemand.h"
#include "rnic_dev_i.h"
#include "mn_comm_api.h"
#include "mdrv_wan.h"
#include "securec.h"

#if (VOS_WIN32 == VOS_OS_VER)
#include <stdio.h>
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_RNIC_ENTITY_C

#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)

STATIC VOS_INT RNIC_IfaceDataTxProc(IMM_Zc *immZc, PS_IFACE_IdUint8 ifaceId, VOS_UINT32 flag)
{
    if (flag & WAN_MAA_OWN) {
        return ads_iface_tx_maa(ifaceId, immZc);
    }

    return ads_iface_tx(ifaceId, immZc);
}

VOS_INT RNIC_V4DataTxProc(IMM_Zc *immZc, VOS_UINT8 rmNetId, VOS_UINT32 flag)
{
    RNIC_IFACE_Ctx *ifaceCtx = RNIC_GET_IFACE_CTX_ADR(rmNetId);

    IMM_ZcSetProtocol(immZc, RNIC_NS_ETH_TYPE_IP);

    return RNIC_IfaceDataTxProc(immZc, ifaceCtx->ifaceId, flag);
}

VOS_INT RNIC_V6DataTxProc(IMM_Zc *immZc, VOS_UINT8 rmNetId, VOS_UINT32 flag)
{
    RNIC_IFACE_Ctx *ifaceCtx = RNIC_GET_IFACE_CTX_ADR(rmNetId);

    IMM_ZcSetProtocol(immZc, RNIC_NS_ETH_TYPE_IPV6);

    return RNIC_IfaceDataTxProc(immZc, ifaceCtx->ifaceId, flag);
}

VOS_INT RNIC_EthDataTxProc(IMM_Zc *immZc, VOS_UINT8 rmNetId, VOS_UINT32 flag)
{
    RNIC_IFACE_Ctx *ifaceCtx = RNIC_GET_IFACE_CTX_ADR(rmNetId);

    return RNIC_IfaceDataTxProc(immZc, ifaceCtx->ifaceId, flag);
}

VOS_INT RNIC_DataRxProc(VOS_ULONG usrData, IMM_Zc *immZc)
{
    return rnic_rx_handle((VOS_UINT8)usrData, immZc);
}

#else

VOS_INT RNIC_V4DataTxProc(IMM_Zc *immZc, VOS_UINT8 rmNetId, VOS_UINT32 flag)
{
    RNIC_PS_IfaceInfo *psIfaceInfo;

    psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);

    return ADS_UL_SendPacketEx(immZc, ADS_PKT_TYPE_IPV4, psIfaceInfo->ipv4ExRabId);
}

VOS_INT RNIC_V6DataTxProc(IMM_Zc *immZc, VOS_UINT8 rmNetId, VOS_UINT32 flag)
{
    RNIC_PS_IfaceInfo *psIfaceInfo;

    psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);

    return ADS_UL_SendPacketEx(immZc, ADS_PKT_TYPE_IPV6, psIfaceInfo->ipv6ExRabId);
}

VOS_INT RNIC_DataRxProc(VOS_UINT8 exRabid, IMM_Zc *immZc, ADS_PktTypeUint8 pktType, VOS_UINT32 exParam)
{
    IMM_ZcSetProtocol(immZc, ((ADS_PKT_TYPE_IPV4 == pktType) ? RNIC_NS_ETH_TYPE_IP : RNIC_NS_ETH_TYPE_IPV6));

    return rnic_rx_handle((VOS_UINT8)exParam, immZc);
}
#endif /* FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM */

#if ((FEATURE_ON == FEATURE_IMS) && (FEATURE_ON == FEATURE_ACPU_PHONE_MODE))
#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)

VOS_UINT32 RNIC_SendCdsImsDataReq(IMM_Zc *immZc, RNIC_IFACE_Ctx *ifaceCtx)
{
    RNIC_CDS_ImsDataReqV2 *msg  = VOS_NULL_PTR;
    VOS_UINT8             *data = VOS_NULL_PTR;
    VOS_UINT32             dataLen;

    dataLen = IMM_ZcGetUsedLen(immZc);

    /* 申请OSA消息 */
    msg = (RNIC_CDS_ImsDataReqV2 *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_CDS_ImsDataReqV2) + dataLen);
    if (msg == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendCdsImsDataReq: Alloc msg failed!");
        return VOS_ERR;
    }

    /* 清空消息内容 */
    if (memset_s(RNIC_GET_MSG_ENTITY(msg), RNIC_GET_MSG_LENGTH(msg), 0x00, RNIC_GET_MSG_LENGTH(msg)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendCdsImsDataReq, memset fail!");
    }

    /* 填充消息头 */
    RNIC_CFG_CDS_MSG_HDR(msg, ID_RNIC_CDS_IMS_DATA_REQ_V2);

    /* 填写消息内容 */
    msg->ifaceId = ifaceCtx->ifaceId;
    msg->len     = (VOS_UINT16)dataLen;

    data = IMM_ZcGetDataPtr(immZc);
    if (memcpy_s(msg->aucData, dataLen, data, dataLen) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendCdsImsDataReq, memcpy fail!");
    }

    /* 发送消息 */
    if (RNIC_SendMsg(msg) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendCdsImsDataReq: Send msg fail!");
        return VOS_ERR;
    }

    return VOS_OK;
}

#else

VOS_UINT32 RNIC_SendCdsImsDataReq(IMM_Zc *immZc, RNIC_IFACE_Ctx *ifaceCtx)
{
    RNIC_CDS_ImsDataReq *sndMsg = VOS_NULL_PTR;

    /* 内存分配 */
    sndMsg = (RNIC_CDS_ImsDataReq *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_CDS_ImsDataReq) - 4 + immZc->len);
    if (sndMsg == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    /* 清空消息内容 */
    if (memset_s(RNIC_GET_MSG_ENTITY(sndMsg), RNIC_GET_MSG_LENGTH(sndMsg), 0x00, RNIC_GET_MSG_LENGTH(sndMsg)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendCdsImsDataReq, memset fail!");
    }

    /* 填充消息头 */
    RNIC_CFG_CDS_MSG_HDR(sndMsg, ID_RNIC_CDS_IMS_DATA_REQ);

    /* 填充消息 */
    sndMsg->modemId = ifaceCtx->psIfaceInfo.modemId;

    if (RNIC_RMNET_R_IS_EMC_BEAR(ifaceCtx->rmNetId)) {
        sndMsg->dataType = RNIC_CDS_WIFI_PDN_TYPE_EMC;
    } else {
        sndMsg->dataType = RNIC_CDS_WIFI_PDN_TYPE_NORMAL;
    }

    sndMsg->dataLen = (VOS_UINT16)immZc->len;

    if (memcpy_s(sndMsg->data, sndMsg->dataLen, immZc->data, immZc->len) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_SendCdsImsDataReq, memcpy fail!");
    }

    /* 发送消息 */
    if (RNIC_SendMsg(sndMsg) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}
#endif /* FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM */

VOS_VOID RNIC_SendVoWifiUlData(RNIC_IFACE_Ctx *ifaceCtx)
{
    IMM_Zc *immZc = VOS_NULL_PTR;

    for (;;) {
        immZc = IMM_ZcDequeueHead(&(ifaceCtx->psIfaceInfo.imsQue));
        if (immZc == VOS_NULL_PTR) {
            break;
        }

        RNIC_SendCdsImsDataReq(immZc, ifaceCtx);
        IMM_ZcFreeAny(immZc); /* 由操作系统接管 */
    }
}

VOS_VOID RNIC_TrigImsDataProcEvent(RNIC_DEV_ID_ENUM_UINT8 rmNetId)
{
    RNIC_IMS_DataProcInd *sndMsg = VOS_NULL_PTR;

    /* 内存分配 */
    sndMsg = (RNIC_IMS_DataProcInd *)RNIC_ALLOC_MSG_WITH_HDR(sizeof(RNIC_IMS_DataProcInd));
    if (sndMsg == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_TrigImsDataProcEvent: Alloc msg failed!");
        return;
    }

    /* 清空消息内容 */
    if (memset_s(RNIC_GET_MSG_ENTITY(sndMsg), RNIC_GET_MSG_LENGTH(sndMsg), 0x00, RNIC_GET_MSG_LENGTH(sndMsg)) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_TrigImsDataProcEvent, memset fail!");
    }

    /* 填充消息头 */
    RNIC_CFG_INTRA_MSG_HDR(sndMsg, ID_RNIC_IMS_DATA_PROC_IND);
    sndMsg->netId = rmNetId;

    /* 发送消息 */
    if (RNIC_SendMsg(sndMsg) != VOS_OK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_TrigImsDataProcEvent: Send msg failed!");
    }
}

VOS_INT RNIC_VoWifiDataTxProc(IMM_Zc *immZc, RNIC_DEV_ID_ENUM_UINT8 rmNetId, VOS_UINT32 flag)
{
    RNIC_PS_IfaceInfo *psIfaceInfo;
    VOS_UINT32         nonEmpty;
    VOS_ULONG          flags;

    psIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(rmNetId);
    nonEmpty    = VOS_FALSE;

    /*lint -e571*/
    VOS_SpinLockIntLock(&(psIfaceInfo->imsQue.lock), flags);
    /*lint +e571*/
    if (IMM_ZcQueueLen(&(psIfaceInfo->imsQue)) == 0) {
        nonEmpty = VOS_TRUE;
    }

    __skb_queue_tail(&(psIfaceInfo->imsQue), immZc);
    VOS_SpinUnlockIntUnlock(&(psIfaceInfo->imsQue.lock), flags);

    if (nonEmpty == VOS_TRUE) {
        RNIC_TrigImsDataProcEvent(rmNetId);
    }

    return VOS_OK;
}

VOS_UINT32 RNIC_RecvVoWifiDlData(RNIC_DEV_ID_ENUM_UINT8 rmNetId, CDS_RNIC_ImsDataInd *imsDataInd)
{
    IMM_Zc   *immZc = VOS_NULL_PTR;
    VOS_UINT8 ipType;

    /* 分配A核内存 */
    immZc = IMM_ZcStaticAlloc(imsDataInd->dataLen + IMM_MAC_HEADER_RES_LEN);
    if (immZc == VOS_NULL_PTR) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RecvVoWifiDlData, IMM_ZcStaticAlloc fail!");
        return VOS_ERR;
    }

    IMM_ZcReserve(immZc, IMM_MAC_HEADER_RES_LEN);

    IMM_ZcPut(immZc, imsDataInd->dataLen);

    /* 拷贝IP数据包 */
    if (memcpy_s(immZc->data, IMM_ZcGetUsedLen(immZc), imsDataInd->data, imsDataInd->dataLen) != EOK) {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RecvVoWifiDlData, memcpy fail!");
    }

    /* 获取IP version */
    ipType = ((RNIC_IPFIXHDR *)(imsDataInd->data))->ipVer;
    if (ipType == RNIC_IPV4_VERSION) {
        IMM_ZcSetProtocol(immZc, RNIC_NS_ETH_TYPE_IP);
    } else if (ipType == RNIC_IPV6_VERSION) {
        IMM_ZcSetProtocol(immZc, RNIC_NS_ETH_TYPE_IPV6);
    } else {
        IMM_ZcFreeAny(immZc); /* 由操作系统接管 */
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RecvVoWifiDlData: IP version error.");
        return VOS_ERR;
    }

    rnic_rx_handle(rmNetId, immZc);

    return VOS_OK;
}
#endif /* FEATURE_ON == FEATURE_IMS && FEATURE_ON == FEATURE_ACPU_PHONE_MODE */

VOS_VOID RNIC_TxDropProc(VOS_UINT8 rmNetId)
{
    if ((RNIC_GET_PROC_DIAL_MODE() == RNIC_DIAL_MODE_DEMAND_DISCONNECT) &&
        (RNIC_GET_IFACE_PDN_IPV4_ACT_STATE(rmNetId) == RNIC_BIT_OPT_FALSE)) {
        RNIC_IFACE_OndemandTxDataProc();
    }
}

