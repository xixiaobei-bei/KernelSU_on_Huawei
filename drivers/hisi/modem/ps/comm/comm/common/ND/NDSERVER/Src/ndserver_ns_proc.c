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

#include "ndserver_ns_proc.h"
#include "securec.h"
#include "ip_comm.h"
#include "ip_ipm_global.h"
#include "ps_common_def.h"
#include "ndis_entity.h"
#include "ndserver_om.h"
#include "ip_nd_server.h"
#include "ndserver_pkt_comm.h"
#include "ndserver_timer_proc.h"
#include "ndserver_na_proc.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERNSPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */


/*
 * Function Name: IP_NDSERVER_FormNsHeaderMsg
 * Description: NS报文头固定部分设置
 * Input: ulIndex --- ND SERVER实体索引
 * Output: pucData --- 报文缓冲指针
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormNsHeaderMsg(VOS_UINT32 addrIndex, VOS_UINT8 *data, const VOS_UINT8 *dstAddr)
{
    VOS_INT32 rlt;
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);

    /* 类型 */
    *data = IP_ICMPV6_TYPE_NS;
    data++;

    /* 代码 */
    *data = IP_IPV6_ND_VALID_CODE;
    data++;

    /* 2:跳过校验和 */
    data += 2;

    /* 4:保留 */
    data += 4;

    /* 目标地址 */
    rlt = memcpy_s(data, IP_IPV6_ADDR_LEN, dstAddr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    return;
}

/*
 * Function Name: IP_NDSERVER_FormNsOptMsg
 * Description: NS报文选项部分设置
 * Input: pucMacAddr- 源链路层地址
 * Output: pucData --- 报文缓冲指针
 *                   pulLen ---- 报文长度指针
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormNsOptMsg(const ND_MacAddrInfo macAddr, VOS_UINT8 *data, VOS_UINT32 *len)
{
    VOS_INT32 rlt;
    /* 类型 */
    *data = IP_ICMPV6_OPT_SRC_LINK_LAYER_ADDR;
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
 * Function Name: IP_NDSERVER_FormNsMsg
 * Description: 生成地址解析邻居请求消息
 * Input: ulIndex ------- 处理NS发包实体索引
 * Output: pucSendBuff --- 发送NS报文缓冲
 *                   pulSendLen ---- 发送报文长度
 * History: VOS_VOID
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormNsMsg(VOS_UINT32 addrIndex, VOS_UINT8 *sendBuff, VOS_UINT32 *sendLen,
                                      const VOS_UINT8 *dstAddr)
{
    VOS_UINT32 packetLen                    = IP_NULL;
    VOS_UINT8 *data                        = sendBuff;
    ND_Ipv6AddrInfo  srcIPAddr;
    ND_MacAddrInfo   srcMacAddr;
    ND_Ipv6AddrInfo  dstIPAddr;
    ND_MacAddrInfo   dstMacAddr;
    VOS_INT32  rlt;
    /* 打印进入该函数 */
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormNsMsg is entered.");

    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    IP_NDSERVER_InitAddrInfo(&srcIPAddr, &srcMacAddr, &dstIPAddr, &dstMacAddr);

    /* 根据自身MAC地址生成link-local地址 */
    rlt = memcpy_s(srcMacAddr.macAddr, IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    IP_ProduceIfaceIdFromMacAddr(srcIPAddr.ipv6Addr, srcMacAddr.macAddr);
    IP_SetUint16Data(srcIPAddr.ipv6Addr, IP_IPV6_LINK_LOCAL_PREFIX);

    /* 根据目的IP生成请求节点组播地址 */
    IP_ProduceSolicitedNodeMulticastIPAddr(dstIPAddr.ipv6Addr, dstAddr);
    IP_ProduceSolicitedNodeMulticastMacAddr(dstMacAddr.macAddr, dstAddr);

    data += IP_ETHERNET_HEAD_LEN + IP_IPV6_HEAD_LEN;

    /* 设置NS报头 */
    IP_NDSERVER_FormNsHeaderMsg(addrIndex, data, dstAddr);

    packetLen += IP_ICMPV6_NS_HEADER_LEN;

    /* 设置NS可选项 */
    IP_NDSERVER_FormNsOptMsg(srcMacAddr, (VOS_VOID *)(data + packetLen), &packetLen);

    data -= IP_IPV6_HEAD_LEN;

    /* 设置IPv6报头 */
    IP_ND_FormIPv6HeaderMsg(srcIPAddr, dstIPAddr, packetLen, data, IP_HEAD_PROTOCOL_ICMPV6);

    /* 生成ICMPv6校验和 */
    if (IP_BuildIcmpv6Checksum(data, IP_IPV6_HEAD_LEN) != IP_SUCC) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormNsMsg: Build ICMPv6 Checksum failed.");
    }

    data -= IP_ETHERNET_HEAD_LEN;

    /* 设置以太报头 */
    IP_NDSERVER_FormEtherHeaderMsg(srcMacAddr, dstMacAddr, data);

    /* 返回报文长度 */
    *sendLen = packetLen + IP_IPV6_HEAD_LEN + IP_ETHERNET_HEAD_LEN;

    return;
}

/*
 * Function Name: IP_NDSERVER_SendNsMsg
 * Description: 发送地址解析邻居请求消息
 * Input: ulIndex ------- 处理NS发包实体索引
 * History:
 *    1.      2011-04-07  Draft Enact
 */
VOS_UINT32 IP_NDSERVER_SendNsMsg(VOS_UINT32 addrIndex, const VOS_UINT8 *dstAddr)
{
    VOS_UINT8 *sendBuff = VOS_NULL_PTR;
    VOS_UINT32 sendLen   = IP_NULL;
    VOS_UINT32 epsbId;
    IP_ASSERT_RTN(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM, PS_FAIL);
    epsbId = IP_NDSERVER_GET_EPSBID(addrIndex);

    sendBuff = IP_NDSERVER_GET_SENDMSG_BUFFER();
    (VOS_VOID)memset_s(sendBuff, IP_IPM_MTU, IP_NULL, IP_IPM_MTU);

    /* 调用形成NS消息函数 */
    IP_NDSERVER_FormNsMsg(addrIndex, sendBuff, &sendLen, dstAddr);

    IP_NDSERVER_AddTransNsPktNum(addrIndex);
    IP_NDSERVER_AddTransPktTotalNum(addrIndex);

    return NdSer_SendPkt(sendBuff, sendLen, (VOS_UINT8)epsbId);
}
STATIC VOS_UINT32 IP_NDSERVER_SetDestNsDataInfo(IP_ND_Msg *destData, VOS_UINT32 icmpv6HeadOffset, VOS_UINT32 payLoad,
                                                const VOS_UINT8 *ipMsg, VOS_INT32 remainLen)
{
    VOS_INT32          rlt;
    IP_ND_MsgNs       *ns    = &destData->ndMsgStru.nsInfo;
    if (remainLen < 0) {
        IPND_ERROR_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeNsData: Invalid IPV6 PayLoad::!", payLoad,
                        icmpv6HeadOffset);
        return IP_FAIL;
    }

    rlt = memcpy_s(destData->srcIp, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    rlt = memcpy_s(destData->desIp, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    rlt = memcpy_s(ns->targetAddr, IP_IPV6_ADDR_LEN,
                   ipMsg + icmpv6HeadOffset + IP_ICMPV6_TARGET_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    return IP_SUCC;
}
STATIC VOS_UINT32 IP_ProcNsOptByOptType(VOS_UINT32 optType, VOS_UINT32 optLen, IP_ND_MsgNs *ns, const VOS_UINT8 *ipMsg)
{
    VOS_INT32          rlt;
    switch (optType) {
        case IP_ICMPV6_OPT_SRC_LINK_LAYER_ADDR: {
            if (optLen == 1) {
                if (ns->opSrcLinkLayerAddr == 0) {
                    rlt = memcpy_s(ns->srcLinkLayerAddr, IP_MAC_ADDR_LEN, ipMsg + 2, IP_MAC_ADDR_LEN); /* 2:len */
                    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
                    ns->opSrcLinkLayerAddr = 1;
                } else {
                    IPND_ERROR_LOG(NDIS_NDSERVER_PID,
                                   "IP_ProcNsOptByOptType: Redundant Source Link-Layer Addr!");
                }
            } else {
                IPND_ERROR_LOG1(NDIS_NDSERVER_PID,
                                "IP_ProcNsOptByOptType: Invalid Source Link-Layer Addr Length:!", optLen);
            }
        } break;
        default:
          break;
    }
    return IP_SUCC;
}
/*
 * Description: 对NS包进行格式转换
 */
STATIC VOS_UINT32 IP_NDSERVER_DecodeNsData(const VOS_UINT8 *srcData, IP_ND_Msg *destData, VOS_UINT32 icmpv6HeadOffset)
{
    VOS_UINT32         payLoad;
    VOS_INT32          remainLen;
    VOS_UINT32         optType;
    VOS_UINT32         optLen;
    const VOS_UINT8         *ipMsg = srcData;
    IP_ND_MsgNs       *ns    = &destData->ndMsgStru.nsInfo;

    /* 获取PAYLOAD */
    IP_GetUint16Data(payLoad, ipMsg + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET);

    remainLen = (VOS_INT32)(payLoad + IP_IPV6_HEAD_LEN - (icmpv6HeadOffset + IP_ICMPV6_NS_HEADER_LEN));
    if (IP_NDSERVER_SetDestNsDataInfo(destData, icmpv6HeadOffset, payLoad, ipMsg, remainLen) == IP_FAIL) {
        return IP_FAIL;
    }

    /*lint -e679 -esym(679,*)*/
    ipMsg += icmpv6HeadOffset + IP_ICMPV6_NS_HEADER_LEN;
    /*lint -e679 +esym(679,*)*/

    while (remainLen >= IP_ND_OPT_UNIT_LEN) {
        optType = *ipMsg;
        optLen  = *(ipMsg + 1);

        if (optLen == 0) {
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeNsData: Invalid ND options length 0!");
            return IP_FAIL;
        }
        if (IP_ProcNsOptByOptType(optType, optLen, ns, ipMsg) == IP_FAIL) {
            return IP_FAIL;
        }

        remainLen -= (VOS_INT32)IP_GetNdOptionLen(optLen);
        /*lint -e679 -esym(679,*)*/
        ipMsg += IP_GetNdOptionLen(optLen);
        /*lint -e679 +esym(679,*)*/
    }

    if (remainLen != 0) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeNsData: Payload not match Package:!", remainLen);
    }

    return IP_SUCC;
}

/*
 * Function Name: IP_NDSERVER_RcvTeDetectionAddr
 * Description: 收到重复地址检测地址
 * Input: ulIndex  -------------- ND SERVER实体索引
 *                   aucIPAddr ------------  IP地址指针
 * History:
 *    1.      2011-04-08  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_RcvTeDetectionAddr(VOS_UINT32 addrIndex, const VOS_UINT8 *iPAddr)
{
    IP_NdserverTeAddrInfo *teInfo = IP_NULL_PTR;
    VOS_INT32                      rlt;
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    teInfo = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);
    /*lint -e960*/
    if ((teInfo->teAddrState == IP_NDSERVER_TE_ADDR_REACHABLE) &&
        (IP_MEM_CMP(teInfo->teGlobalAddr, iPAddr, IP_IPV6_ADDR_LEN) == IP_NULL)) {
        /*lint +e960*/
        return;
    }

    IPND_WARNING_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_RcvTeDetectionAddr: Old Te Addr:", teInfo->teAddrState);
    rlt = memcpy_s(teInfo->teGlobalAddr, IP_IPV6_ADDR_LEN, iPAddr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    teInfo->teAddrState = IP_NDSERVER_TE_ADDR_INCOMPLETE;

    IPND_WARNING_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_RcvTeDetectionAddr: New Te Addr:", teInfo->teAddrState);

    return;
}
STATIC VOS_VOID IP_NDSERVER_NotSelfIPAddr(VOS_UINT32 addrIndex, const IP_ND_Msg *ndMsgData)
{
    /* 判断是否是重复地址检测NS */
    if ((IP_IPV6_EQUAL_ALL_ZERO(ndMsgData->srcIp)) &&
        (ndMsgData->ndMsgStru.nsInfo.opSrcLinkLayerAddr == 0)) {
        if (IP_NDSERVER_EqualAdvertisedPrefix(addrIndex, ndMsgData->ndMsgStru.nsInfo.targetAddr) == IP_TRUE) {
            IP_NDSERVER_RcvTeDetectionAddr(addrIndex, ndMsgData->ndMsgStru.nsInfo.targetAddr);
            /* 启动收到重复地址检测后的等待定时器 */
            IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_FIRST_NS);

            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc:Receive duplicate addr detection packet.");
            return;
        } else {
            IP_NDSERVER_AddErrNsPktNum(addrIndex);
            IPND_WARNING_LOG(NDIS_NDSERVER_PID,
                             "IP_NDSERVER_NsMsgProc:Duplicate addr detect IP don't equal Advertised Prefix, discard!");
            return;
        }
    } else if ((!IP_IPV6_EQUAL_ALL_ZERO(ndMsgData->srcIp)) &&
               (ndMsgData->ndMsgStru.nsInfo.opSrcLinkLayerAddr != 0)) {
        if (IP_NDSERVER_SendNaMsg(addrIndex, ndMsgData) != PS_SUCC) {
            IP_NDSERVER_AddTransPktFailNum(addrIndex);
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc:Send NA Msg failed.");
        }
    } else {
        IP_NDSERVER_AddErrNsPktNum(addrIndex);
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc:Target IP is not self addr, discard!");
        return;
    }
    return;
}


/*
 * Function Name: IP_NDSERVER_NsMsgProc
 * Description: 处理NS消息
 * Input: ulIndex  -------------- 处理消息实体索引
 *                   pucSrcData ------------ IP数据报
 *                   ulIcmpv6HeadOffset ---- ICMPv6报文头偏移量
 * History:
 *    1.      2011-04-08  Draft Enact
 */
VOS_VOID IP_NDSERVER_NsMsgProc(VOS_UINT32 addrIndex, const VOS_UINT8 *srcData, VOS_UINT32 icmpv6HeadOffset)
{
    const VOS_UINT8                  *ipMsg     = srcData;
    IP_ND_Msg                  *ndMsgData = VOS_NULL_PTR;
    IP_NdserverAddrInfo        *infoAddr  = IP_NULL_PTR;
    VOS_UINT8                  *ipAddrData = IP_NULL_PTR;

    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc is entered.");
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    IP_NDSERVER_AddRcvNsPktNum(addrIndex);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);

    ndMsgData = IP_NDSERVER_GET_NDMSGDATA_ADDR();
    (VOS_VOID)memset_s(ndMsgData, sizeof(IP_ND_Msg), IP_NULL, sizeof(IP_ND_Msg));

    if (IP_NDSERVER_DecodeNsData(ipMsg, ndMsgData, icmpv6HeadOffset) != IP_SUCC) {
        IP_NDSERVER_AddErrNsPktNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc: Invalid IPV6 NS Msg:!");
        return;
    }

    if (infoAddr->validFlag != IP_TRUE) {
        /* MT关机再开机后，ND SERVER收到网侧RA前，可能先收到TE的重复地址检测NS */
        if ((IP_IPV6_EQUAL_ALL_ZERO(ndMsgData->srcIp)) &&
            (ndMsgData->ndMsgStru.nsInfo.opSrcLinkLayerAddr == 0)) {
            IP_NDSERVER_SaveTeDetectIp(ndMsgData->ndMsgStru.nsInfo.targetAddr);
            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc: receive DAD NS packet before RA.");
        } else {
            IP_NDSERVER_AddErrNsPktNum(addrIndex);
            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc: ND Server info flag is invalid!");
        }
        return;
    }

    ipAddrData = &ndMsgData->ndMsgStru.nsInfo.targetAddr[0];
    if (IP_NDSERVER_IsSelfIPAddr(addrIndex, ipAddrData) == IP_TRUE) {
        /* 发送NA消息到PC */
        if (IP_NDSERVER_SendNaMsg(addrIndex, ndMsgData) != PS_SUCC) {
            IP_NDSERVER_AddTransPktFailNum(addrIndex);
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_NsMsgProc:Send NA Msg failed.");
        }
    } else {
        IP_NDSERVER_NotSelfIPAddr(addrIndex, ndMsgData);
    }

    return;
}



