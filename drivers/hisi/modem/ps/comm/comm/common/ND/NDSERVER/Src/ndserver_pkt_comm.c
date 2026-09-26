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

#include "ndserver_pkt_comm.h"
#include "securec.h"
#include "imm_interface.h"
#include "ppp_interface.h"
#include "ndis_interface.h"
#include "ndis_entity.h"
#include "ndis_arp_proc.h"
#include "ip_ipm_global.h"
#include "ip_comm.h"
#include "ip_nd_server.h"
#include "ndserver_entity.h"
#include "ndserver_rs_proc.h"
#include "ndserver_ns_proc.h"
#include "ndserver_na_proc.h"
#include "ndserver_om.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERPKTCOMM_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */

/*
 * Function Name: IP_NDSERVER_FormEtherHeaderMsg
 * Description: IPv6报头设置
 * Input: aucSrcMacAddr --- 源MAC地址
 *                   aucDstMacAddr --- 目的MAC地址
 * Output: pucData --- 报文缓冲指针
 * History:
 *    1.      2011-04-07  Draft Enact
 */
VOS_VOID IP_NDSERVER_FormEtherHeaderMsg(const ND_MacAddrInfo srcMacAddr, const ND_MacAddrInfo dstMacAddr,
                                        VOS_UINT8 *data)
{
    VOS_INT32 rlt;
    /* 目的MAC */
    rlt = memcpy_s(data, IP_MAC_ADDR_LEN, dstMacAddr.macAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    data += IP_MAC_ADDR_LEN;

    /* 源MAC */
    rlt = memcpy_s(data, IP_MAC_ADDR_LEN, srcMacAddr.macAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    data += IP_MAC_ADDR_LEN;

    /* 类型 */
    IP_SetUint16Data(data, (VOS_UINT16)IP_GMAC_PAYLOAD_TYPE_IPV6);

    return;
}
VOS_VOID IP_NDSERVER_InitAddrInfo(ND_Ipv6AddrInfo *srcIPAddr, ND_MacAddrInfo *srcMacAddr, ND_Ipv6AddrInfo *dstIPAddr,
                                  ND_MacAddrInfo  *dstMacAddr)
{
    (VOS_VOID)memset_s(srcIPAddr, sizeof(ND_Ipv6AddrInfo), IP_NULL, sizeof(ND_Ipv6AddrInfo));
    (VOS_VOID)memset_s(dstIPAddr, sizeof(ND_Ipv6AddrInfo), IP_NULL, sizeof(ND_Ipv6AddrInfo));
    (VOS_VOID)memset_s(srcMacAddr, sizeof(ND_MacAddrInfo), IP_NULL, sizeof(ND_MacAddrInfo));
    (VOS_VOID)memset_s(dstMacAddr, sizeof(ND_MacAddrInfo), IP_NULL, sizeof(ND_MacAddrInfo));
}
/*lint +e767*/

/* 统计信息 */


/*
 * 2 Declare the Global Variable
 */

/*
 * Function Name: IP_NDSERVER_EqualAdvertisedPrefix
 * Description: 判断是否符合已公告的全球前缀
 * Input: ulIndex  -------------- 处理消息实体索引
 *                   aucIPAddr ------------  IP地址指针
 * History:
 *    1.      2011-04-08  Draft Enact
 */
VOS_UINT32 IP_NDSERVER_EqualAdvertisedPrefix(VOS_UINT32 addrIndex, const VOS_UINT8 *iPAddr)
{
    ESM_IP_Ipv6NWPara *nwPara = IP_NULL_PTR;

    IP_ASSERT_RTN(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM, IP_FALSE);
    nwPara = IP_NDSERVER_ADDRINFO_GET_NWPARA(addrIndex);
    if (IP_MEM_CMP(iPAddr, nwPara->prefixList[0].prefix, ND_IP_IPV6_ADDR_LENGTH - ND_IP_IPV6_IFID_LENGTH)
        == IP_NULL) {
        return IP_TRUE;
    } else {
        return IP_FALSE;
    }
}


/*
 * Function Name: IP_NDSERVER_IsSelfIPAddr
 * Description: 判断是否是自己的IP地址
 * Input: ulIndex  -------------- 处理消息实体索引
 *                   aucIPAddr ------------  IP地址指针
 * History:
 *    1.      2011-04-08  Draft Enact
 */
VOS_UINT32 IP_NDSERVER_IsSelfIPAddr(VOS_UINT32 addrIndex, const VOS_UINT8 *iPAddr)
{
    VOS_UINT8                 linkLocalIPAddr[IP_IPV6_ADDR_LEN] = {IP_NULL};
    VOS_UINT8                 globalIPAddr[IP_IPV6_ADDR_LEN]    = {IP_NULL};
    VOS_UINT8                 macAddr[IP_MAC_ADDR_LEN];
    VOS_UINT8                *macIpAddr                         = &macAddr[0];
    VOS_UINT8                *ipAddr                            = IP_NULL_PTR;
    ESM_IP_Ipv6NWPara        *nwPara                            = IP_NULL_PTR;
    VOS_INT32                 rlt;
    IP_ASSERT_RTN(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM, IP_FALSE);

    /* 根据自身MAC地址生成link-local地址 */
    rlt = memcpy_s(macAddr, IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FALSE);
    ipAddr = &linkLocalIPAddr[0];
    IP_ProduceIfaceIdFromMacAddr(ipAddr, macIpAddr);
    IP_SetUint16Data(ipAddr, IP_IPV6_LINK_LOCAL_PREFIX);

    ipAddr = &globalIPAddr[0];
    IP_ProduceIfaceIdFromMacAddr(ipAddr, macIpAddr);
    nwPara = IP_NDSERVER_ADDRINFO_GET_NWPARA(addrIndex);
    rlt = memcpy_s(globalIPAddr, IP_IPV6_ADDR_LEN, nwPara->prefixList[0].prefix,
                   ND_IP_IPV6_ADDR_LENGTH - ND_IP_IPV6_IFID_LENGTH);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FALSE);

    /*lint -e960*/
    if ((IP_MEM_CMP(iPAddr, linkLocalIPAddr, IP_IPV6_ADDR_LEN) == IP_NULL) ||
        (IP_MEM_CMP(iPAddr, globalIPAddr, IP_IPV6_ADDR_LEN) == IP_NULL)) {
        /*lint +e960*/
        return IP_TRUE;
    } else {
        return IP_FALSE;
    }
}

/*
 * 功能描述: 发送数据到PPP
 * 修改历史:
 *  1.日    期: 2020年07月20日
 *    修改内容: 新生成函数
 */
STATIC VOS_UINT32 NdSer_SendPktToPPP(const VOS_UINT8 *buf, VOS_UINT32 len, VOS_UINT8 ifaceId)
{
    IMM_Zc           *immZc = VOS_NULL_PTR;
    VOS_UINT8        *pdata       = VOS_NULL_PTR;
    errno_t           rlt;
    immZc = IMM_ZcStaticAlloc(len);
    if (immZc == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendFrmToPPP, IMM_ZcStaticAlloc fail!");
        return PS_FAIL;
    }

    pdata = (VOS_UINT8 *)IMM_ZcPut(immZc, len);
    if (pdata == VOS_NULL_PTR) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_SendFrmToPPP, IMM_ZcPut fail!");
        return PS_FAIL;
    }

    /* IP头开始 */
    rlt = memcpy_s(pdata, len, buf + ETH_MAC_HEADER_LEN, len);
    if (rlt != EOK) {
        /*lint -e522*/
        IMM_ZcFree(immZc);
        /*lint +e522*/
        return PS_FAIL;
    }
    PPP_RcvNdData(ifaceId, immZc);
    return PS_SUCC;
}

/*
 * 功能描述: 发送数据到PC或者PPP
 */
VOS_UINT32 NdSer_SendPkt(const VOS_UINT8 *buf, VOS_UINT32 len, VOS_UINT8 exRabId)
{
    if (buf == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NdSer_SendPkt, Buf is NULL!");
        return PS_FAIL;
    }

    /* PPP网卡流程 */
    if (exRabId == PS_IFACE_ID_PPP0) {
        if (len <= ETH_MAC_HEADER_LEN) {
            return PS_FAIL;
        }
        return NdSer_SendPktToPPP(buf, len - ETH_MAC_HEADER_LEN, exRabId);
    }

    /* NDIS网卡流程 */
    if (Ndis_SendMacFrm(buf, len, exRabId) == PS_FAIL) {
        return PS_FAIL;
    }
    return PS_SUCC;
}

/*
 * Function Name: IP_NDSERVER_FormEchoReply
 * Description: 形成向PC回复的ECHO REPLY
 * Input: pucSrcData ----------- 源报文指针
 *                   pstDestData ---------- 目的转换结构指针
 *                   ulIcmpv6HeadOffset --- ICMPv6报头偏移量
 * History:
 *    1.      2011-12-09  Draft Enact
 */
STATIC VOS_UINT32 IP_NDSERVER_FormEchoReply(VOS_UINT32 addrIndex, const VOS_UINT8 *srcData, VOS_UINT8 *destData,
                                            VOS_UINT32 dataLen)
{
    const VOS_UINT8                     *ipMsg                       = srcData;
    VOS_UINT8                     *data                        = VOS_NULL_PTR;
    ND_Ipv6AddrInfo                srcIPAddr;
    ND_MacAddrInfo                 srcMacAddr;
    ND_Ipv6AddrInfo                dstIPAddr;
    ND_MacAddrInfo                 dstMacAddr;
    IP_NdserverTeAddrInfo         *teInfo                      = IP_NULL_PTR;
    VOS_INT32                      rlt;

    if (dataLen <= IP_IPV6_HEAD_LEN) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_FormEchoReply: ulDataLen is too short.", dataLen);
        return IP_FAIL;
    }
    IP_NDSERVER_InitAddrInfo(&srcIPAddr, &srcMacAddr, &dstIPAddr, &dstMacAddr);

    /* 根据自身MAC地址生成link-local地址 */
    rlt = memcpy_s(srcMacAddr.macAddr, IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    IP_ProduceIfaceIdFromMacAddr(srcIPAddr.ipv6Addr, srcMacAddr.macAddr);
    IP_SetUint16Data(srcIPAddr.ipv6Addr, IP_IPV6_LINK_LOCAL_PREFIX);

    /* 得到目的IPV6地址 */
    rlt = memcpy_s(dstIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);

    /* 得到目的MAC地址 */
    teInfo = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);
    rlt     = memcpy_s(dstMacAddr.macAddr, IP_MAC_ADDR_LEN, teInfo->teLinkLayerAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);

    /* 指向ICMP首部 */
    data = destData + IP_ETHERNET_HEAD_LEN + IP_IPV6_HEAD_LEN;

    /* 填写ICMP报文 */
    ipMsg += IP_IPV6_HEAD_LEN;
    rlt = memcpy_s(data, (IP_IPM_MTU - IP_ETHERNET_HEAD_LEN - IP_IPV6_HEAD_LEN), ipMsg,
                   (dataLen - IP_IPV6_HEAD_LEN));
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);

    /* ICMP type域 */
    *data = IP_ICMPV6_TYPE_ECHOREPLY;
    data++;

    /* ICMP code域 */
    *data = IP_IPV6_ND_VALID_CODE;
    data++;

    /* ICMP 校验和域 */
    *(VOS_UINT16 *)(VOS_VOID *)data = 0;

    /* 指向IPV6首部 */
    destData += IP_ETHERNET_HEAD_LEN;
    /* 填写IPV6头部 */
    IP_ND_FormIPv6HeaderMsg(srcIPAddr, dstIPAddr, (dataLen - IP_IPV6_HEAD_LEN), destData,
                            IP_HEAD_PROTOCOL_ICMPV6);

    /* 生成ICMPv6报头校验和 */
    if (IP_BuildIcmpv6Checksum(destData, IP_IPV6_HEAD_LEN) != IP_SUCC) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormEchoReply: Build ICMPv6 Checksum failed.");
        return IP_FAIL;
    }

    /* 设置以太报头 */
    destData -= IP_ETHERNET_HEAD_LEN;
    IP_NDSERVER_FormEtherHeaderMsg(srcMacAddr, dstMacAddr, destData);

    return IP_SUCC;
}

/*
 * Function Name: IP_NDSERVER_EchoRequestMsgProc
 * Description: 处理ECHO REQUEST消息
 * Input: ulIndex  -------------- 处理消息实体索引
 *                   pucSrcData ------------ IP数据报
 *                   ulIcmpv6HeadOffset ---- ICMPv6报文头偏移量
 * History:
 *    1.      2011-12-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_EchoRequestMsgProc(VOS_UINT32 addrIndex, const VOS_UINT8 *srcData, VOS_UINT32 dataLen)
{
    const VOS_UINT8                  *ipMsg                     = srcData;
    VOS_UINT8                  *sendBuff                  = VOS_NULL_PTR;
    IP_NdserverAddrInfo        *infoAddr                  = IP_NULL_PTR;
    VOS_UINT8                   pktAddr[IP_IPV6_ADDR_LEN];
    VOS_INT32                   rlt;
    VOS_UINT8                  *ipAddrData = &pktAddr[0];

    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_EchoRequestMsgProc is entered.");

    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    IP_NDSERVER_AddRcvEchoPktNum(addrIndex);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    if (infoAddr->validFlag != IP_TRUE) {
        IP_NDSERVER_AddErrEchoPktNum(addrIndex);
        IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_EchoRequestMsgProc: ND Server info flag is invalid!");
        return;
    }

    rlt = memcpy_s(pktAddr, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    if (IP_NDSERVER_IsSelfIPAddr(addrIndex, ipAddrData) != IP_TRUE) {
        IP_NDSERVER_AddErrEchoPktNum(addrIndex);
        return;
    }

    sendBuff = IP_NDSERVER_GET_SENDMSG_BUFFER();
    (VOS_VOID)memset_s(sendBuff, IP_IPM_MTU, IP_NULL, IP_IPM_MTU);

    if (IP_NDSERVER_FormEchoReply(addrIndex, ipMsg, sendBuff, dataLen) != IP_SUCC) {
        IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_EchoRequestMsgProc: IP_NDSERVER_FormEchoReply fail!");
        return;
    }

    /* 将ECHO REPLY发送到PC */
    if (NdSer_SendPkt(sendBuff, dataLen + IP_ETHERNET_HEAD_LEN, infoAddr->epsbId) == PS_FAIL) {
        IP_NDSERVER_AddTransPktFailNum(addrIndex);
        IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_EchoRequestMsgProc:Send ECHO REPLY failed.");
        return;
    }

    IP_NDSERVER_AddTransEchoPktNum(addrIndex);
    IP_NDSERVER_AddTransPktTotalNum(addrIndex);

    return;
}
STATIC VOS_VOID NdSer_NdProcPktByMsgType(const VOS_UINT8 *data, VOS_UINT32 icmpv6HeadOffset, VOS_UINT32 addrIndex,
                                         VOS_UINT32 dataLen)
{
    IP_Icmpv6TypeUint32         icmpv6MsgType;
    /* 取ICMPV6消息中的TYPE字段 */
    icmpv6MsgType = *(data + icmpv6HeadOffset);

    switch (icmpv6MsgType) {
        case IP_ICMPV6_TYPE_RS:
            IP_NDSERVER_RsMsgProc(addrIndex, data, icmpv6HeadOffset);
            break;
        case IP_ICMPV6_TYPE_NS:
            IP_NDSERVER_NsMsgProc(addrIndex, data, icmpv6HeadOffset);
            break;
        case IP_ICMPV6_TYPE_NA:
            IP_NDSERVER_NaMsgProc(addrIndex, data, icmpv6HeadOffset);
            break;
        case IP_ICMPV6_TYPE_ECHOREQUEST:
            IP_NDSERVER_EchoRequestMsgProc(addrIndex, data, dataLen);
            break;

        default:
            IP_NDSERVER_AddDiscPktNum(addrIndex);
            IPND_WARNING_LOG1(NDIS_NDSERVER_PID, "NdSer_NdAndEchoPktProc: Ignored IPV6 ND Msg:!", icmpv6MsgType);
            break;
    }
    return;
}
VOS_UINT32 NdSer_NdGetMsgInfo(const MsgBlock *msgBlock, VOS_UINT8 **data, VOS_UINT32 *dataLen, VOS_UINT8 *exRabId)
{
    ADS_NDIS_DataInd       *rcvMsg = (ADS_NDIS_DataInd *)(VOS_VOID *)msgBlock;

    *data = IMM_ZcGetDataPtr(rcvMsg->data);
    if (*data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "NdSer_NdAndEchoPktProc, IMM_ZcGetDataPtr fail!");
        return PS_FAIL;
    }
    *dataLen = IMM_ZcGetUsedLen(rcvMsg->data);
    *exRabId = NDIS_FORM_EXBID(rcvMsg->modemId, rcvMsg->rabId);
    return PS_SUCC;
}
/*
 * Function Name: NdSer_NdAndEchoPktProc
 */
VOS_VOID NdSer_NdAndEchoPktProc(const MsgBlock *msgBlock)
{
    VOS_UINT32                  icmpv6HeadOffset = IP_NULL;
    VOS_UINT32                  addrIndex;
    VOS_UINT8                  *data             = VOS_NULL_PTR;
    VOS_UINT32                  dataLen;
    VOS_UINT8                   exRabId;
    IP_NdserverAddrInfo *nDSerAddrInfoTmp = VOS_NULL_PTR;
    if (NdSer_NdGetMsgInfo(msgBlock, &data, &dataLen, &exRabId) != PS_SUCC) {
        return;
    }

#if (FEATURE_OFF == FEATURE_DATA_SERVICE_NEW_PLATFORM)
    if (ND_CheckEpsIdValid(exRabId) != IP_SUCC) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "NdSer_NdAndEchoPktProc, IMM_ZcGetDataPtr return NULL", exRabId);
        return;
    }
#endif
    addrIndex = IP_NDSERVER_GET_ADDR_INFO_INDEX(exRabId);
    if (addrIndex >= IP_NDSERVER_ADDRINFO_MAX_NUM) {
        /*
         * MT关机再开机后，ND SERVER收到网侧RA前，可能先收到TE的重复地址检测NS ，为了能够处理此类消息，
         * 此处临时申请一个ND Ser Addr Info
         */
        IPND_INFO_LOG1(NDIS_NDSERVER_PID, "NdSer_NdAndEchoPktProc: alloc new addr info for NS msg", exRabId);
        nDSerAddrInfoTmp = NdSer_AllocAddrInfo(&addrIndex);
        if (nDSerAddrInfoTmp == IP_NULL_PTR) {
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "NdSer_AllocAddrInfo: return NULL!");
            return;
        }
        nDSerAddrInfoTmp->validFlag = IP_FALSE;
    }

    IP_NDSERVER_AddRcvPktTotalNum(addrIndex);

    /* 判断消息是否合法的ND消息 */
    if (IP_IsValidNdMsg(data, dataLen, &icmpv6HeadOffset) != IP_TRUE) {
        IP_NDSERVER_AddDiscPktNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "NdSer_NdAndEchoPktProc: Invalid ND Msg!");
        return;
    }

    NdSer_NdProcPktByMsgType(data, icmpv6HeadOffset, addrIndex, dataLen);

    return;
}



