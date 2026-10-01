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

/******************************************************************************
   1 头文件包含
******************************************************************************/

#include "RnicEntity.h"
#include "RnicMsgProc.h"
#include "RnicLog.h"
#include "RnicMntn.h"
#include "RnicIfaceOndemand.h"
#include "rnic_dev_i.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/

#define THIS_FILE_ID PS_FILE_ID_RNIC_ENTITY_C

/******************************************************************************
   2 外部函数变量声明
******************************************************************************/

/******************************************************************************
   3 私有定义
******************************************************************************/

/******************************************************************************
   4 全局变量定义
*****************************************************************************/

/******************************************************************************
   5 函数实现
******************************************************************************/


VOS_INT RNIC_V4DataTxProc(IMM_ZC_STRU *pstImmZc, VOS_UINT8 ucRmNetId)
{
    RNIC_PS_IFACE_INFO_STRU            *pstPsIfaceInfo;

    pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(ucRmNetId);

    return ADS_UL_SendPacketEx(pstImmZc, ADS_PKT_TYPE_IPV4, pstPsIfaceInfo->ucIpv4ExRabId);
}


VOS_INT RNIC_V6DataTxProc(IMM_ZC_STRU *pstImmZc, VOS_UINT8 ucRmNetId)
{
    RNIC_PS_IFACE_INFO_STRU            *pstPsIfaceInfo;

    pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(ucRmNetId);

    return ADS_UL_SendPacketEx(pstImmZc, ADS_PKT_TYPE_IPV6, pstPsIfaceInfo->ucIpv6ExRabId);
}


VOS_INT RNIC_DataRxProc(
    VOS_UINT8                           ucExRabid,
    IMM_ZC_STRU                        *pstImmZc,
    ADS_PKT_TYPE_ENUM_UINT8             enPktType,
    VOS_UINT32                          ulExParam
)
{
    VOS_UINT8                           ucRmNetId;

    ucRmNetId = RNIC_GET_RMNETID_FROM_EXPARAM(ulExParam);

    return rnic_rx_handle(ucRmNetId, pstImmZc, enPktType);
}


VOS_UINT32 RNIC_SendCdsImsDataReq(
    IMM_ZC_STRU                        *pstImmZc,
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx
)
{
    RNIC_CDS_IMS_DATA_REQ_STRU         *pstSndMsg = VOS_NULL_PTR;

    /* 内存分配 */
    pstSndMsg = (RNIC_CDS_IMS_DATA_REQ_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_RNIC,
                                    sizeof(RNIC_CDS_IMS_DATA_REQ_STRU) - 4 + pstImmZc->len);

    if (VOS_NULL_PTR == pstSndMsg)
    {
        return VOS_ERR;
    }

    /* 填充消息头 */
    pstSndMsg->ulSenderCpuId            = VOS_LOCAL_CPUID;
    pstSndMsg->ulSenderPid              = ACPU_PID_RNIC;
    pstSndMsg->ulReceiverCpuId          = VOS_LOCAL_CPUID;
    pstSndMsg->ulReceiverPid            = UEPS_PID_CDS;
    pstSndMsg->ulMsgId                  = ID_RNIC_CDS_IMS_DATA_REQ;

    /* 填充消息 */
    pstSndMsg->usModemId                = pstIfaceCtx->stPsIfaceInfo.enModemId;

    if (RNIC_RMNET_R_IS_EMC_BEAR(pstIfaceCtx->enRmNetId))
    {
        pstSndMsg->enDataType           = RNIC_CDS_WIFI_PDN_TYPE_EMC;
    }
    else
    {
        pstSndMsg->enDataType           = RNIC_CDS_WIFI_PDN_TYPE_NORMAL;
    }

    pstSndMsg->usDataLen                = (VOS_UINT16)pstImmZc->len;

    TAF_MEM_CPY_S(pstSndMsg->aucData, pstImmZc->len, pstImmZc->data, pstImmZc->len);

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstSndMsg))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID RNIC_SendVoWifiUlData(
    RNIC_IFACE_CTX_STRU                *pstIfaceCtx
)
{
    IMM_ZC_STRU                        *pstImmZc = VOS_NULL_PTR;

    for (;;)
    {
        pstImmZc = IMM_ZcDequeueHead(&(pstIfaceCtx->stPsIfaceInfo.stImsQue));
        if (VOS_NULL_PTR == pstImmZc)
        {
            break;
        }

        RNIC_SendCdsImsDataReq(pstImmZc, pstIfaceCtx);
        IMM_ZcFreeAny(pstImmZc);
    }

    return;
}


VOS_VOID RNIC_TrigImsDataProcEvent(RNIC_DEV_ID_ENUM_UINT8 enRmNetId)
{
    RNIC_IMS_DATA_PROC_IND_STRU         *pstSndMsg = VOS_NULL_PTR;

    /* 内存分配 */
    pstSndMsg = (RNIC_IMS_DATA_PROC_IND_STRU *)PS_ALLOC_MSG_WITH_HEADER_LEN(ACPU_PID_RNIC,
                                    sizeof(RNIC_IMS_DATA_PROC_IND_STRU));

    if (VOS_NULL_PTR == pstSndMsg)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_TrigImsDataProcEvent: Alloc msg failed!");
        return;
    }

    /* 填充消息头 */
    pstSndMsg->ulSenderCpuId            = VOS_LOCAL_CPUID;
    pstSndMsg->ulSenderPid              = ACPU_PID_RNIC;
    pstSndMsg->ulReceiverCpuId          = VOS_LOCAL_CPUID;
    pstSndMsg->ulReceiverPid            = ACPU_PID_RNIC;
    pstSndMsg->enMsgId                  = ID_RNIC_IMS_DATA_PROC_IND;
    pstSndMsg->enRmNetId                = enRmNetId;

    /* 发送消息 */
    if (VOS_OK != PS_SEND_MSG(ACPU_PID_RNIC, pstSndMsg))
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_TrigImsDataProcEvent: Send msg failed!");
    }

    return;
}


VOS_INT RNIC_VoWifiDataTxProc(
    IMM_ZC_STRU                        *pstImmZc,
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId
)
{
    RNIC_PS_IFACE_INFO_STRU            *pstPsIfaceInfo;
    VOS_UINT32                          ulNonEmpty;
    VOS_ULONG                           ulFlags;

    pstPsIfaceInfo = RNIC_GET_IFACE_PDN_INFO_ADR(enRmNetId);
    ulNonEmpty     = VOS_FALSE;

    /*lint -e571*/
    VOS_SpinLockIntLock(&(pstPsIfaceInfo->stImsQue.lock), ulFlags);
    /*lint +e571*/
    if (0 == IMM_ZcQueueLen(&(pstPsIfaceInfo->stImsQue)))
    {
        ulNonEmpty = VOS_TRUE;
    }

    __skb_queue_tail(&(pstPsIfaceInfo->stImsQue), pstImmZc);
    VOS_SpinUnlockIntUnlock(&(pstPsIfaceInfo->stImsQue.lock), ulFlags);

    if (VOS_TRUE == ulNonEmpty)
    {
        RNIC_TrigImsDataProcEvent(enRmNetId);
    }

    return VOS_OK;

}


VOS_UINT32 RNIC_RecvVoWifiDlData(
    RNIC_DEV_ID_ENUM_UINT8              enRmNetId,
    CDS_RNIC_IMS_DATA_IND_STRU         *pstImsDataInd
)
{
    IMM_ZC_STRU                        *pstImmZc      = VOS_NULL_PTR;
    VOS_UINT8                           ucIpType;
    VOS_UINT8                           ucIpFamily;

    /* 分配A核内存 */
    pstImmZc = IMM_ZcStaticAlloc(pstImsDataInd->usDataLen + IMM_MAC_HEADER_RES_LEN);
    if (VOS_NULL_PTR == pstImmZc)
    {
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RecvVoWifiDlData, IMM_ZcStaticAlloc fail!");
        return VOS_ERR;
    }

    IMM_ZcReserve(pstImmZc, IMM_MAC_HEADER_RES_LEN);

    /* 拷贝IP数据包 */
    TAF_MEM_CPY_S(pstImmZc->data, pstImsDataInd->usDataLen, pstImsDataInd->aucData, pstImsDataInd->usDataLen);

    IMM_ZcPut(pstImmZc, pstImsDataInd->usDataLen);

    /* 获取IP version */
    ucIpType = ((RNIC_IPFIXHDR_STRU *)(pstImsDataInd->aucData))->ucIpVer;
    if (RNIC_IPV4_VERSION == ucIpType)
    {
        ucIpFamily = RNIC_IPV4_ADDR;
    }
    else if (RNIC_IPV6_VERSION == ucIpType)
    {
        ucIpFamily = RNIC_IPV6_ADDR;
    }
    else
    {
        IMM_ZcFreeAny(pstImmZc);
        RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_RecvVoWifiDlData: IP version error.");
        return VOS_ERR;
    }

    rnic_rx_handle(enRmNetId, pstImmZc, ucIpFamily);

    return VOS_OK;
}


VOS_VOID RNIC_TxDropProc(VOS_UINT8 ucRmNetId)
{
    if ( (RNIC_DIAL_MODE_DEMAND_DISCONNECT == RNIC_GET_PROC_DIAL_MODE())
      && (RNIC_BIT_OPT_FALSE == RNIC_GET_IFACE_PDN_IPV4_ACT_STATE(ucRmNetId)) )
    {
        RNIC_IFACE_OndemandTxDataProc();
    }

    return;
}


unsigned int RNIC_StartFlowCtrl(unsigned char ucRmNetId)
{
    return VOS_OK;
}


unsigned int RNIC_StopFlowCtrl(unsigned char ucRmNetId)
{
    return VOS_OK;
}



