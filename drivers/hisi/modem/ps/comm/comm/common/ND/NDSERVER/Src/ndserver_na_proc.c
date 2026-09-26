/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
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

#include "ndserver_na_proc.h"
#include "securec.h"
#include "ps_common_def.h"
#include "ads_device_interface.h"
#include "ip_ipm_global.h"
#include "ip_comm.h"
#include "ndis_entity.h"
#include "ndis_dl_proc.h"
#include "ndserver_om.h"
#include "ndserver_pkt_comm.h"
#include "ip_nd_server.h"
#include "ndserver_timer_proc.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERNAPROC_C
/*lint +e767*/

/*
 * Function Name: IP_NDSERVER_FormNaHeaderMsg
 * Description: NA报文头固定部分设置
 * Input: pucTargetIPAddr --- 目的IP地址
 *                   ucSolicitFlag   --- 请求标志
 * Output: pucData --- 报文缓冲指针
 * History:
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormNaHeaderMsg(const ND_Ipv6AddrInfo targetIPAddr, VOS_UINT8 solicitFlag, VOS_UINT8 *data)
{
    VOS_INT32 rlt;
    /* 类型 */
    *data = IP_ICMPV6_TYPE_NA;
    data++;

    /* 代码 */
    *data = IP_IPV6_ND_VALID_CODE;
    data++;

    /* 2:跳过校验和 */
    data += 2;

    /* 6:路由器标志、请求标志、覆盖标志 */
    *data = (VOS_UINT8)(0xa0 | ((solicitFlag & 0x1) << 6));
    data++;

    /* 3:保留 */
    data += 3;

    /* 目标地址 */
    rlt = memcpy_s(data, IP_IPV6_ADDR_LEN, targetIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    return;
}

/*
 * Description: NA报文选项部分设置
 * Input: pucMacAddr- 源链路层地址
 * Output: pucData --- 报文缓冲指针
 *                   pulLen ---- 报文长度指针
 * History:
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormNaOptMsg(const ND_MacAddrInfo macAddr, VOS_UINT8 *data, VOS_UINT32 *len)
{
    VOS_INT32 rlt;
    /* 类型 */
    *data = IP_ICMPV6_OPT_TGT_LINK_LAYER_ADDR;
    data++;

    /* 长度 */
    *data = 1;
    data++;

    /* 链路层地址 */
    rlt = memcpy_s(data, IP_MAC_ADDR_LEN, macAddr.macAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    (*len) += IP_ND_OPT_UNIT_LEN;

    return;
}

/*
 * Function Name: IP_NDSERVER_FormNaMsg
 * Description: 生成邻居公告消息
 * Input: ulIndex ------- 处理NA发包实体索引
 *                   pstNdMsgData -- 目的信息参数
 * Output: pucSendBuff --- 发送NA报文缓冲
 *                   pulSendLen ---- 发送报文长度
 * History:
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormNaMsg(const IP_ND_Msg *ndMsgData, VOS_UINT8 *sendBuff, VOS_UINT32 *sendLen)
{
    VOS_UINT32 packetLen                   = IP_NULL;
    VOS_UINT8 *data                        = sendBuff;
    ND_Ipv6AddrInfo  srcIPAddr;
    ND_MacAddrInfo   srcMacAddr;
    ND_Ipv6AddrInfo  dstIPAddr;
    ND_MacAddrInfo   dstMacAddr;
    VOS_UINT8  solicitFlag                  = IP_NULL;
    VOS_INT32  rlt;
    /* 打印进入该函数 */
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormNaMsg is entered.");
    IP_NDSERVER_InitAddrInfo(&srcIPAddr, &srcMacAddr, &dstIPAddr, &dstMacAddr);

    /* 根据自身MAC地址作为源MAC */
    rlt = memcpy_s(srcMacAddr.macAddr, IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    if (ndMsgData == VOS_NULL_PTR) {
        /* 根据自身MAC地址生成link-local地址 */
        IP_ProduceIfaceIdFromMacAddr(srcIPAddr.ipv6Addr, srcMacAddr.macAddr);
        IP_SetUint16Data(srcIPAddr.ipv6Addr, IP_IPV6_LINK_LOCAL_PREFIX);
    } else {
        /* 使用目标地址作为源IP */
        rlt = memcpy_s(srcIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN, ndMsgData->ndMsgStru.nsInfo.targetAddr, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    }

    /* 确定单播或组播方式 */
    if ((ndMsgData != VOS_NULL_PTR) && (!IP_IPV6_EQUAL_ALL_ZERO(ndMsgData->srcIp)) &&
        (!IP_IPV6_IS_MULTICAST_ADDR(ndMsgData->srcIp)) &&
        (ndMsgData->ndMsgStru.nsInfo.opSrcLinkLayerAddr == 1)) {
        rlt = memcpy_s(dstIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN, ndMsgData->srcIp, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        rlt = memcpy_s(dstMacAddr.macAddr, IP_MAC_ADDR_LEN, ndMsgData->ndMsgStru.nsInfo.srcLinkLayerAddr,
                       IP_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        solicitFlag = 1;
    } else {
        rlt = memcpy_s(dstIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN, g_ndAllNodesMulticaseAddr, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        rlt = memcpy_s(dstMacAddr.macAddr, IP_MAC_ADDR_LEN, g_ndAllNodesMacAddr, IP_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    }

    data += IP_ETHERNET_HEAD_LEN + IP_IPV6_HEAD_LEN;

    /* 设置NA报头 */
    IP_NDSERVER_FormNaHeaderMsg(srcIPAddr, solicitFlag, data);

    packetLen += IP_ICMPV6_NA_HEADER_LEN;

    /* 设置NA可选项 */
    IP_NDSERVER_FormNaOptMsg(srcMacAddr, (VOS_VOID *)(data + packetLen), &packetLen);

    data -= IP_IPV6_HEAD_LEN;

    /* 设置IPv6报头 */
    IP_ND_FormIPv6HeaderMsg(srcIPAddr, dstIPAddr, packetLen, data, IP_HEAD_PROTOCOL_ICMPV6);

    /* 生成ICMPv6校验和 */
    if (IP_BuildIcmpv6Checksum(data, IP_IPV6_HEAD_LEN) != IP_SUCC) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormNaMsg: Build ICMPv6 Checksum failed.");
    }

    data -= IP_ETHERNET_HEAD_LEN;

    /* 设置以太报头 */
    IP_NDSERVER_FormEtherHeaderMsg(srcMacAddr, dstMacAddr, data);

    /* 返回报文长度 */
    *sendLen = packetLen + IP_IPV6_HEAD_LEN + IP_ETHERNET_HEAD_LEN;

    return;
}

/*
 * Function Name: IP_NDSERVER_SendNaMsg
 * Description: 发送邻居公告消息
 * Input: ulIndex ------- 处理NA发包实体索引
 *                   pstNdMsgData -- 目的信息参数
 * History:
 *    1.      2011-04-07  Draft Enact
 */
VOS_UINT32 IP_NDSERVER_SendNaMsg(VOS_UINT32 addrIndex, const IP_ND_Msg *ndMsgData)
{
    VOS_UINT8 *sendBuff = VOS_NULL_PTR;
    VOS_UINT32 sendLen   = IP_NULL;
    VOS_UINT32 epsbId;
    IP_ASSERT_RTN(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM, PS_FAIL);
    epsbId = IP_NDSERVER_GET_EPSBID(addrIndex);

    sendBuff = IP_NDSERVER_GET_SENDMSG_BUFFER();
    (VOS_VOID)memset_s(sendBuff, IP_IPM_MTU, IP_NULL, IP_IPM_MTU);

    /* 调用形成NA消息函数 */
    IP_NDSERVER_FormNaMsg(ndMsgData, sendBuff, &sendLen);

    IP_NDSERVER_AddTransNaPktNum(addrIndex);
    IP_NDSERVER_AddTransPktTotalNum(addrIndex);

    return NdSer_SendPkt(sendBuff, sendLen, (VOS_UINT8)epsbId);
}

STATIC VOS_UINT32 IP_NDSERVER_FillDestDataInfo(VOS_INT32 *remainLen, const VOS_UINT8 *ipMsg,
                                               VOS_UINT32 icmpv6HeadOffset, IP_ND_Msg *destData, IP_ND_MsgNa *na)
{
    VOS_UINT32         payLoad;
    VOS_INT32          rlt;

    /* 获取PAYLOAD */
    IP_GetUint16Data(payLoad, ipMsg + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET);
    *remainLen = (VOS_INT32)(payLoad + IP_IPV6_HEAD_LEN - (icmpv6HeadOffset + IP_ICMPV6_NA_HEADER_LEN));

    if ((*remainLen) < 0) {
        IPND_ERROR_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeNaData: Invalid IPV6 PayLoad::!", payLoad,
                        icmpv6HeadOffset);
        return IP_FAIL;
    }

    rlt = memcpy_s(destData->srcIp, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    rlt = memcpy_s(destData->desIp, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);

    na->naRValue = ((*(ipMsg + icmpv6HeadOffset + IP_ICMPV6_NA_FLAG_OFFSET)) >> 7) & 0x01; /* 7:bit */
    na->naSValue = ((*(ipMsg + icmpv6HeadOffset + IP_ICMPV6_NA_FLAG_OFFSET)) >> 6) & 0x01; /* 6:bit */
    na->naOValue = ((*(ipMsg + icmpv6HeadOffset + IP_ICMPV6_NA_FLAG_OFFSET)) >> 5) & 0x01; /* 5:bit */

    rlt = memcpy_s(na->targetAddr, IP_IPV6_ADDR_LEN,
                   ipMsg + icmpv6HeadOffset + IP_ICMPV6_TARGET_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    return IP_SUCC;
}
STATIC VOS_UINT32 IP_ProcNaOptByOptType(VOS_UINT32 optType, VOS_UINT32 optLen, IP_ND_MsgNa *na, const VOS_UINT8 *ipMsg)
{
    VOS_INT32          rlt;
    switch (optType) {
        case IP_ICMPV6_OPT_TGT_LINK_LAYER_ADDR: {
            if (optLen == 1) {
                if (na->opTargetLinkLayerAddr == 0) {
                    rlt = memcpy_s(na->targetLinkLayerAddr, IP_MAC_ADDR_LEN, ipMsg + 2, IP_MAC_ADDR_LEN); /* 2:len */
                    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
                    na->opTargetLinkLayerAddr = 1;
                } else {
                    IPND_ERROR_LOG(NDIS_NDSERVER_PID,
                                   "IP_GetNaTargetLinkLayerAddr: Redundant Target Link-Layer Addr!");
                }
            } else {
                IPND_ERROR_LOG1(NDIS_NDSERVER_PID,
                                "IP_GetNaTargetLinkLayerAddr: Invalid Target Link-Layer Addr Length:!", optLen);
            }
        } break;
        default:
            break;
    }
    return IP_SUCC;
}
/*
 * Description: 对NA包进行格式转换
 */
STATIC VOS_UINT32 IP_NDSERVER_DecodeNaData(const VOS_UINT8 *srcData, IP_ND_Msg *destData, VOS_UINT32 icmpv6HeadOffset)
{
    VOS_INT32          remainLen = 0;
    VOS_UINT32         optType;
    VOS_UINT32         optLen;
    const VOS_UINT8         *ipMsg = srcData;
    IP_ND_MsgNa       *na    = &destData->ndMsgStru.naInfo;

    if (IP_NDSERVER_FillDestDataInfo(&remainLen, ipMsg, icmpv6HeadOffset, destData, na) == IP_FAIL) {
        return IP_FAIL;
    }

    /*lint -e679 -esym(679,*)*/
    ipMsg += icmpv6HeadOffset + IP_ICMPV6_NA_HEADER_LEN;
    /*lint -e679 +esym(679,*)*/

    while (remainLen >= IP_ND_OPT_UNIT_LEN) {
        optType = *ipMsg;
        optLen  = *(ipMsg + 1);

        if (optLen == 0) {
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeNaData: Invalid ND options length 0!");
            return IP_FAIL;
        }

        if (IP_ProcNaOptByOptType(optType, optLen, na, ipMsg) == IP_FAIL) {
            return IP_FAIL;
        }

        remainLen -= (VOS_INT32)IP_GetNdOptionLen(optLen);
        /*lint -e679 -esym(679,*)*/
        ipMsg += IP_GetNdOptionLen(optLen);
        /*lint -e679 +esym(679,*)*/
    }

    if (remainLen != 0) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeNaData: Payload not match Package:!", remainLen);
    }

    return IP_SUCC;
}

/*
 * Function Name: IP_NDSERVER_EqualSavedTeAddr
 * Description: 判断是否是已保存的TE地址
 * Input: ulIndex  -------------  ND SERVER实体索引
 *                   aucIPAddr ------------  IP地址指针
 *                   aucMACAddr -----------  MAC地址指针
 * History:
 *    1.      2011-04-09  Draft Enact
 */
STATIC VOS_UINT32 IP_NDSERVER_EqualSavedTeAddr(VOS_UINT32 addrIndex, const VOS_UINT8 *iPAddr, const VOS_UINT8 *mACAddr)
{
    IP_NdserverTeAddrInfo *teInfo = IP_NULL_PTR;

    IP_ASSERT_RTN(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM, IP_FALSE);
    teInfo = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);
    /*lint -e960*/
    if ((teInfo->teAddrState == IP_NDSERVER_TE_ADDR_REACHABLE) &&
        (IP_MEM_CMP(teInfo->teGlobalAddr, iPAddr, IP_IPV6_ADDR_LEN) == IP_NULL) &&
        (IP_MEM_CMP(teInfo->teLinkLayerAddr, mACAddr, IP_MAC_ADDR_LEN) == IP_NULL)) {
        /*lint +e960*/
        return IP_TRUE;
    } else {
        return IP_FALSE;
    }
}
/*
 * Function Name: IP_NDSERVER_SendDlPkt
 * Description: 发送下行缓存的IPV6数据包
 * Input: ulIndex  -------------  ND SERVER实体索引
 * History:
 *    1.      2011-12-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_SendDlPkt(VOS_UINT32 addrIndex)
{
    IMM_Zc                 *immZc  = VOS_NULL_PTR;
    VOS_UINT8               rabId;
    ADS_PktTypeUint8        pktType;
    VOS_UINT16              app;
    VOS_INT32               lockKey;

    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);

    while (LUP_IsQueEmpty(IP_NDSERVER_ADDRINFO_GET_DLPKTQUE(addrIndex)) != PS_TRUE) {
        lockKey = VOS_SplIMP();
        if (LUP_DeQue(IP_NDSERVER_ADDRINFO_GET_DLPKTQUE(addrIndex), (VOS_VOID **)(&immZc)) != PS_SUCC) {
            VOS_Splx(lockKey);
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_SendDlPkt, LUP_DeQue return fail!");
            return;
        }
        VOS_Splx(lockKey);

        /* 从 ImmZc中取出RabId和enPdpType */
        /*lint -e522*/
        app     = IMM_ZcGetUserApp(immZc);
        rabId   = (VOS_UINT8)(app & 0xFF); /* 保存的值实际是 扩展RabId */
        pktType = (VOS_UINT8)(app >> 8); /* 8:bit */
        if (Ndis_DlSendNcm(rabId, pktType, immZc) != PS_SUCC) {
            IMM_ZcFree(immZc);
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_SendDlPkt, Ndis_DlSendNcm fail!");
        }
        /*lint +e522*/
        IP_NDSERVER_AddSendQuePktNum(addrIndex);
    }

    return;
}

/*
 * Function Name: IP_NDSERVER_UpdateSpeMac
 * Description: 配置硬件MAC地址
 */
STATIC VOS_VOID IP_NDSERVER_UpdateSpeMac(VOS_UINT32 addrIndex,  const VOS_UINT8 *macAddr)
{
    IP_NdserverAddrInfo     *infoAddr   = VOS_NULL_PTR;
    VOS_INT32                rlt;

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    if (infoAddr->validFlag != IP_TRUE) {
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_UpdateSpeMac: ND Server info flag is invalid!");
        return;
    }

    rlt = memcpy_s(g_speMacHeader.srcAddr, ETH_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(),
                   ETH_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    rlt = memcpy_s(g_speMacHeader.dstAddr, ETH_MAC_ADDR_LEN, macAddr, ETH_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);


#if (FEATURE_ON == FEATURE_DATA_SERVICE_NEW_PLATFORM)
#if (defined(CONFIG_BALONG_ESPE))
        mdrv_wan_set_ipfmac((VOS_VOID *)&g_speMacHeader);
#endif
#endif

    IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_UpdateSpeMac OK");
    return;
}


/*
 * Function Name: IP_NDSERVER_UpdateTeAddrInfo
 * Description: 收到重复地址检测地址
 * Input: ulIndex  -------------  ND SERVER实体索引
 *                   aucGlobalIPAddr ------  全球IP地址指针
 *                   aucMACAddr -----------  MAC地址指针
 *                   aucLinkLocalIPAddr ---  链路本地IP地址指针
 * History:
 *    1.      2011-04-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_UpdateTeAddrInfo(VOS_UINT32 addrIndex, const VOS_UINT8 *globalIPAddr,
                                             const VOS_UINT8 *macAddr, const VOS_UINT8 *linkLocalIPAddr)
{
    IP_NdserverTeAddrInfo *teInfo = IP_NULL_PTR;
    VOS_INT32                      rlt;
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    teInfo = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);

    IPND_WARNING_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_UpdateTeAddrInfo: Old Te Addr:", teInfo->teAddrState);
    rlt = memcpy_s(teInfo->teGlobalAddr, IP_IPV6_ADDR_LEN, globalIPAddr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    rlt = memcpy_s(teInfo->teLinkLayerAddr, IP_MAC_ADDR_LEN, macAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* 更新完整MAC帧头中的目的MAC地址 */
    rlt = memcpy_s(IP_NDSERVER_ADDRINFO_GET_MACFRAME(addrIndex), IP_ETH_MAC_HEADER_LEN, macAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    if (IP_IPV6_IS_LINKLOCAL_ADDR(linkLocalIPAddr)) {
        rlt = memcpy_s(teInfo->teLinkLocalAddr, IP_IPV6_ADDR_LEN, linkLocalIPAddr, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    } else {
        (VOS_VOID)memset_s(teInfo->teLinkLocalAddr, IP_IPV6_ADDR_LEN, IP_NULL, IP_IPV6_ADDR_LEN);
    }

    teInfo->teAddrState = IP_NDSERVER_TE_ADDR_REACHABLE;

    /* 配置硬件MAC地址 */
    IP_NDSERVER_UpdateSpeMac(addrIndex, macAddr);

    IPND_WARNING_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_UpdateTeAddrInfo: New Te Addr:", teInfo->teAddrState);
    /* 发送下行IP缓存队列中的数据包 */
    IP_NDSERVER_SendDlPkt(addrIndex);
    return;
}

/*
 * Function Name: IP_NDSERVER_NaMsgProc
 * Description: 处理NA消息
 * Input: ulIndex  -------------- 处理消息实体索引
 *                   pucSrcData ------------ IP数据报
 *                   ulIcmpv6HeadOffset ---- ICMPv6报文头偏移量
 * History:
 *    1.      2011-04-08  Draft Enact
 */
VOS_VOID IP_NDSERVER_NaMsgProc(VOS_UINT32 addrIndex, const VOS_UINT8 *srcData, VOS_UINT32 icmpv6HeadOffset)
{
    const VOS_UINT8                  *ipMsg     = srcData;
    IP_ND_Msg                  *ndMsgData = VOS_NULL_PTR;
    IP_NdserverAddrInfo        *infoAddr  = IP_NULL_PTR;
    VOS_UINT8                  *ipAddrData = IP_NULL_PTR;

    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NaMsgProc is entered.");
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    IP_NDSERVER_AddRcvNaPktNum(addrIndex);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    if (infoAddr->validFlag != IP_TRUE) {
        IP_NDSERVER_AddErrNaPktNum(addrIndex);
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NaMsgProc: ND Server info flag is invalid!");
        return;
    }

    ndMsgData = IP_NDSERVER_GET_NDMSGDATA_ADDR();
    (VOS_VOID)memset_s(ndMsgData, sizeof(IP_ND_Msg), IP_NULL, sizeof(IP_ND_Msg));

    if (IP_NDSERVER_DecodeNaData(ipMsg, ndMsgData, icmpv6HeadOffset) != IP_SUCC) {
        IP_NDSERVER_AddErrNaPktNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NaMsgProc: Invalid IPV6 NS Msg:!");
        return;
    }

    if (ndMsgData->ndMsgStru.naInfo.opTargetLinkLayerAddr == 0) {
        IP_NDSERVER_AddErrNaPktNum(addrIndex);
        IPND_WARNING_LOG(NDIS_NDSERVER_PID,
                         "IP_NDSERVER_NaMsgProc:Packet don't contain Target Linklayer Addr, discard!");
        return;
    }

    ipAddrData = &ndMsgData->desIp[0];
    if ((IP_NDSERVER_IsSelfIPAddr(addrIndex, ipAddrData) != IP_TRUE) &&
        (!IP_IPV6_IS_MULTICAST_ADDR(ndMsgData->desIp))) {
        IP_NDSERVER_AddErrNaPktNum(addrIndex);
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NaMsgProc:Destination IP don't contain self Addr, discard!");
        return;
    }

    if (IP_NDSERVER_EqualAdvertisedPrefix(addrIndex, ndMsgData->ndMsgStru.naInfo.targetAddr) != IP_TRUE) {
        IP_NDSERVER_AddErrNaPktNum(addrIndex);
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NaMsgProc:Target IP don't equal Advertised Prefix, discard!");
        return;
    }

    if (IP_NDSERVER_EqualSavedTeAddr(addrIndex, ndMsgData->ndMsgStru.naInfo.targetAddr,
                                     ndMsgData->ndMsgStru.naInfo.targetLinkLayerAddr) != IP_TRUE) {
        IP_NDSERVER_UpdateTeAddrInfo(addrIndex, ndMsgData->ndMsgStru.naInfo.targetAddr,
                                     ndMsgData->ndMsgStru.naInfo.targetLinkLayerAddr, ndMsgData->srcIp);
    }

    IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_PERIODIC_NS);

    return;
}



