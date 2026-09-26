/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012-10-10
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

/*
 * 1 Include HeadFile
 */
#include "ndis_ul_proc.h"
#include "vos.h"
#include "imm_interface.h"
#include "ads_dev_i.h"
#include "ip_comm.h"
#include "ndis_interface.h"
#include "ipv4_dhcp_server.h"
#include "ndis_om.h"
#include "ip_nd_server.h"
#include "ndis_entity.h"
#include "ndis_debug.h"
#include "ndis_arp_proc.h"
#include "ndis_debug.h"
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_OFF)
#include "ads_device_interface.h"
#endif


#define THIS_MODU ps_ndis

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISULPROC_C
/*lint +e767*/


STATIC VOS_UINT32 Ndis_UlNcmFrmProcIpv4(IMM_Zc *immZc, VOS_UINT32 cacheLen, VOS_UINT32 dataLen)
{
    VOS_UINT8            *data     = VOS_NULL_PTR;
    ETH_Ipfixhdr         *ipFixHdr = VOS_NULL_PTR;
    VOS_UINT32            ipLen;
    data = IMM_ZcGetDataPtr(immZc);
    if (data == VOS_NULL_PTR) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        return IP_FAIL;
    }

    NDIS_SPE_MEM_UNMAP(immZc, cacheLen);

    /* 长度异常判断 */
    /*lint -e644*/
    if (dataLen < sizeof(ETH_Ipfixhdr)) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than size of ETH_Ipfixhdr!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return IP_FAIL;
    }
    /*lint +e644*/
    ipFixHdr = (ETH_Ipfixhdr *)((VOS_VOID *)data);
    ipLen     = IP_NTOHS(ipFixHdr->totalLen);
    if (ipLen < immZc->len) {
        immZc->tail -= (immZc->len - ipLen);
        immZc->len = ipLen;
    }

    NDIS_SPE_MEM_MAP(immZc, cacheLen);
    return IP_SUCC;
}

STATIC VOS_UINT32 Ndis_UlNcmFrmProcIpv6(VOS_UINT8 exRabId, IMM_Zc *immZc, ETHFRM_Ipv4Pkt *ipPacket)
{
    VOS_UINT32            dataLen;
    dataLen = IMM_ZcGetUsedLen(immZc);
    if ((dataLen - ETH_MAC_HEADER_LEN) > g_nvMtu) {
        NDIS_SPE_MEM_UNMAP(immZc, dataLen);
        IP_NDSERVER_ProcTooBigPkt(exRabId, ((VOS_UINT8 *)ipPacket + ETH_MAC_HEADER_LEN), g_nvMtu);
        NDIS_SPE_MEM_MAP(immZc, dataLen);

        /* 调用Imm_ZcFree释放ImmZc */
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        return IP_FAIL;
    }
    return IP_SUCC;
}

STATIC VOS_VOID Ndis_UlNcmFrmProcArp(VOS_UINT8 exRabId, NDIS_Mem *pktNode, ETHFRM_Ipv4Pkt *ipPacket)
{
    VOS_UINT32            dataLen;
    /* 长度异常判断 */
    dataLen = NDIS_MemGetUsedLen(pktNode);
    /*lint -e413*/
    if (dataLen < NDIS_ARP_FRAME_REV_OFFSET) {
        /*lint -e522*/
        NDIS_MemFree(pktNode);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than NDIS_ARP_FRAME_REV_OFFSET!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return;
    }

    /* ARP处理函数入参中增加RabId，后续以RabId作为Ndis实体遍历索引 */
    Ndis_ProcArpMsg((ETH_ArpFrame *)(VOS_VOID *)ipPacket, exRabId); /*lint !e527*/
    /* 处理完ARP后调用Imm_ZcFree释放ImmZc */
    /*lint -e522*/
    NDIS_MemFree(pktNode);
    /*lint +e522*/
    return;
}

STATIC VOS_UINT32 Ndis_UlNcmProcByFrameType(ETHFRM_Ipv4Pkt *ipPacket, VOS_UINT8 exRabId, NDIS_Mem *pktNode)
{
    VOS_UINT16            frameType = ipPacket->frameType;

    /* ARP处理 */
    if (frameType == ARP_PAYLOAD) {
        Ndis_UlNcmFrmProcArp(exRabId, pktNode, ipPacket);
        return PS_FAIL;
    }
    /* IPV6超长包处理 */
    if (frameType == IPV6_PAYLOAD) {
        if (Ndis_UlNcmFrmProcIpv6(exRabId, pktNode, ipPacket) == IP_FAIL) {
            return PS_FAIL;
        }
    }
    return PS_SUCC;
}

STATIC VOS_UINT32 Ndis_UlSendAds(VOS_UINT8 exRabId, NDIS_Mem *pktNode, VOS_UINT16 frameType)
{
#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    if (ADS_UL_SendPacket(pktNode, exRabId) != VOS_OK) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ADS_UL_SendPacket fail!");
        return PS_FAIL;
    }
#else
    IMM_ZcSetProtocol(pktNode, frameType);
    if (ads_iface_tx(exRabId, pktNode) != VOS_OK) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ads_iface_tx fail!");
        return PS_FAIL;
    }
#endif
    return PS_SUCC;
}

/*
 * 功能描述: 对上行IP类型NCM帧的处理
 */
VOS_VOID Ndis_UlIpNcmFrmProc(VOS_UINT8 exRabId, NDIS_Mem *pktNode, ETHFRM_Ipv4Pkt *ipPacket)
{
    VOS_UINT32            dataLen = 0;
    VOS_UINT32            ipv4Flag = 0;
    VOS_UINT32            cacheLen = 0;

    if (Ndis_UlNcmProcByFrameType(ipPacket, exRabId, pktNode) != PS_SUCC) {
        return;
    }

    if (ipPacket->frameType == IP_PAYLOAD) {
        ipv4Flag = 1;
        dataLen  = IMM_ZcGetUsedLen(pktNode);
        cacheLen = (dataLen < NDIS_SPE_CACHE_HDR_SIZE) ? dataLen : NDIS_SPE_CACHE_HDR_SIZE;
    }

    /* 经MAC层过滤后剩余的IP包发送，去掉MAC帧头后递交ADS */
    if (IMM_ZcRemoveMacHead(pktNode) != VOS_OK) {
        /*lint -e522*/
        IMM_ZcFree(pktNode);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcRemoveMacHead fail!");
        return;
    }

    /* 检查IPV4包长度和实际Skb长度，如果不一致，则修改Skb长度为实际IP包长度 */
    if (ipv4Flag == 1) {
        if (Ndis_UlNcmFrmProcIpv4(pktNode, cacheLen, dataLen) == IP_FAIL) {
            return;
        }
    }

    if (Ndis_UlSendAds(exRabId, pktNode, ipPacket->frameType) != PS_SUCC) {
        return;
    }

    NDIS_STAT_UL_SEND_ADSPKT(1);

    return;
}

/*
 * 功能描述: 对上行NCM帧的处理
 */
VOS_VOID Ndis_UlNcmFrmProc(VOS_UINT8 exRabId, NDIS_Mem *pktNode)
{
    ETHFRM_Ipv4Pkt       *ipPacket = VOS_NULL_PTR;
    VOS_UINT8            *data     = VOS_NULL_PTR;
    NDIS_Entity          *ndisEntity = VOS_NULL_PTR;

    data = NDIS_MemGetDataPtr(pktNode);
    if (data == VOS_NULL_PTR) {
        /*lint -e522*/
        NDIS_MemFree(pktNode);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcGetDataPtr fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return;
    }

    ipPacket = (ETHFRM_Ipv4Pkt *)(VOS_VOID *)data;

    /* 使用ExRabId获取NDIS实体 */
    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == IP_NULL_PTR) {
        /*lint -e522*/
        NDIS_MemFree(pktNode);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, NDIS_GetEntityByRabId fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return;
    }

    /* 5G LAN STICK模式直接透传数据 */
    if ((ndisEntity->rabType & NDIS_ENTITY_ETH) == NDIS_ENTITY_ETH) {
        if (Ndis_UlSendAds(exRabId, pktNode, ipPacket->frameType) != PS_SUCC) {
            return;
        }
        NDIS_STAT_UL_SEND_ADSPKT(1);

        return;
    }

    Ndis_UlIpNcmFrmProc(exRabId, pktNode, ipPacket);

    return;
}


/*
 * 功能描述: 根据SpePort查找RabId
 * 修改历史:
 *  1.日    期: 2011年12月9日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT8 Ndis_FindRabIdBySpePort(VOS_INT32 spePort, VOS_UINT16 frameType)
{
    VOS_UINT32        loop;
    VOS_UINT8         tmpRabType;
    NDIS_Entity      *ndisEntity = VOS_NULL_PTR;

    switch (frameType) {
        case ARP_PAYLOAD:
            tmpRabType = NDIS_ENTITY_IPV4; /* ARP包也经过SPE */
            break;
        case IP_PAYLOAD:
            tmpRabType = NDIS_ENTITY_IPV4;
            break;
        case IPV6_PAYLOAD:
            tmpRabType = NDIS_ENTITY_IPV6;
            break;
        default:
            NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_FindRabIdByHandle, FrameType Error!", frameType);
            return NDIS_INVALID_RABID;
    }
    for (loop = 0; loop < NAS_NDIS_MAX_ITEM; loop++) {
        ndisEntity = &g_ndisEntity[loop];

        if ((spePort == ndisEntity->spePort) &&
            (tmpRabType == (ndisEntity->rabType & tmpRabType))) { /* 数据包类型与承载类型一致 */
            return ndisEntity->rabId;
        }
    }

    return NDIS_INVALID_RABID;
}


/*
 * 功能描述: 根据Handle查找RabId
 * 修改历史:
 *  1.日    期: 2011年12月9日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT8 Ndis_FindRabIdByHandle(VOS_INT32 rabHandle, VOS_UINT16 frameType)
{
    VOS_UINT32        loop;
    VOS_UINT8         tmpRabType = NDIS_RAB_NULL;
    NDIS_Entity      *ndisEntity = VOS_NULL_PTR;

    if ((frameType == ARP_PAYLOAD) || (frameType == IP_PAYLOAD)) {
        tmpRabType = NDIS_ENTITY_IPV4;
    } else if (frameType == IPV6_PAYLOAD) {
        tmpRabType = NDIS_ENTITY_IPV6;
    } else {
    }

    for (loop = 0; loop < NAS_NDIS_MAX_ITEM; loop++) {
        ndisEntity = &g_ndisEntity[loop];

        /* 数据包类型与承载类型一致 */
        if ((rabHandle == ndisEntity->handle) &&
            ((tmpRabType == (ndisEntity->rabType & tmpRabType)) ||
            ((ndisEntity->rabType & NDIS_ENTITY_ETH) == NDIS_ENTITY_ETH))) {
            return ndisEntity->rabId;
        }
    }

    return NDIS_INVALID_RABID;
}

/*
 * 功能描述: App核间SPE通道
 * 修改历史:
 *  1.日    期: 2015年1月31日
 *    修改内容: 新生成函数
 */
VOS_UINT32 AppNdis_SpeReadCb(VOS_INT32 lSpePort, VOS_VOID *pktNode)
{
    IMM_Zc *immZc = (IMM_Zc *)pktNode; /* 目前ImmZc和sk_buff完全一致，直接强转 */

    VOS_UINT16            frameType;
    ETHFRM_Ipv4Pkt       *ipPacket = VOS_NULL_PTR;
    VOS_UINT8            *data     = VOS_NULL_PTR;
    VOS_UINT8             exRabId;
    VOS_UINT32            dataLen;

    if (immZc == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "AppNdis_UsbReadCb read NULL PTR!");
        return PS_FAIL;
    }

    data = IMM_ZcGetDataPtr(immZc);
    if (data == VOS_NULL_PTR) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcGetDataPtr fail!");
        return PS_FAIL;
    }

    /* 长度异常判断 */
    dataLen = IMM_ZcGetUsedLen(immZc);
    if (dataLen < ETH_MAC_HEADER_LEN) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than ETH_MAC_HEADER_LEN!");
        return PS_FAIL;
    }

    ipPacket = (ETHFRM_Ipv4Pkt *)(VOS_VOID *)data;
    frameType = ipPacket->frameType;

    /* 这里获取的是扩展RabId */
    exRabId = Ndis_FindRabIdBySpePort(lSpePort, frameType);
    if (Ndis_ChkRabIdValid(exRabId) != PS_SUCC) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, Ndis_ChkRabIdValid fail!");
        return PS_FAIL;
    }

    Ndis_LomTraceRcvUlData();
    Ndis_UlNcmFrmProc(exRabId, immZc);

    return PS_SUCC;
}

/*
 * 功能描述: App核间USB通道
 * 修改历史:
 *  1.日    期: 2011年1月31日
 *    修改内容: 新生成函数
 */
VOS_UINT32 AppNdis_UsbReadCb(VOS_INT32 rbIdHandle, VOS_VOID *pktNode)
{
    IMM_Zc *immZc = (IMM_Zc *)pktNode; /* 目前ImmZc和sk_buff完全一致，直接强转 */

    VOS_UINT8             exRabId;
    VOS_UINT16            frameType;
    ETHFRM_Ipv4Pkt       *ipPacket = VOS_NULL_PTR;
    VOS_UINT8            *data     = VOS_NULL_PTR;
    VOS_UINT32            dataLen;

    if (immZc == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "AppNdis_UsbReadCb read NULL PTR!");
        return PS_FAIL;
    }

    data = IMM_ZcGetDataPtr(immZc);
    if (data == VOS_NULL_PTR) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, IMM_ZcGetDataPtr fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return PS_FAIL;
    }

    /* 长度异常判断 */
    dataLen = IMM_ZcGetUsedLen(immZc);
    if (dataLen < ETH_MAC_HEADER_LEN) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, ulDataLen less than ETH_MAC_HEADER_LEN!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return PS_FAIL;
    }

    ipPacket = (ETHFRM_Ipv4Pkt *)(VOS_VOID *)data;
    frameType = ipPacket->frameType;

    /* 这里获取的是扩展RabId */
    exRabId = Ndis_FindRabIdByHandle(rbIdHandle, frameType);
    if (Ndis_ChkRabIdValid(exRabId) != PS_SUCC) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_UlNcmFrmProc, Ndis_ChkRabIdValid fail!");
        NDIS_STAT_UL_DISCARD_USBPKT(1);
        return PS_FAIL;
    }

    NDIS_STAT_UL_RECV_USBPKT_SUCC(1);

    Ndis_LomTraceRcvUlData();
    Ndis_UlNcmFrmProc(exRabId, immZc);

    return PS_SUCC;
}




