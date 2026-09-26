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

#include "ndserver_ra_proc.h"
#include "securec.h"
#include "ip_ipm_global.h"
#include "ip_comm.h"
#include "ndis_entity.h"
#include "ip_nd_server.h"
#include "ndserver_om.h"
#include "ndserver_entity.h"
#include "ndserver_pkt_comm.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERRAPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */

/*
 * Description: RA报文头固定部分设置
 * Input: ulIndex --- ND SERVER实体索引
 * Output: pucData --- 报文缓冲指针
 * History:
 *    1.      2011-04-07  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormRaHeaderMsg(VOS_UINT32 addrIndex, VOS_UINT8 *data)
{
    ESM_IP_Ipv6NWPara *nwPara = IP_NULL_PTR;

    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    nwPara = IP_NDSERVER_ADDRINFO_GET_NWPARA(addrIndex);

    /* 类型 */
    *data = IP_ICMPV6_TYPE_RA;
    data++;

    /* 代码 */
    *data = IP_IPV6_ND_VALID_CODE;
    data++;

    /* 2:跳过校验和 */
    data += 2;

    /* 当前跳限制 */
    *data = nwPara->curHopLimit;
    data++;

    /* 7 6:管理地址配置标志、其他有状态配置标志 */
    *data = (VOS_UINT8)(((g_ndServerMFlag & 0x1) << 7) | ((g_ndServerOFlag & 0x1) << 6));
    data++;

    /* 路由生存期 */
    IP_SetUint16Data(data, g_routerLifetime);
    data += 2; /* 路由生存周期需要占2个字节 */

    /* 可达时间 */
    IP_SetUint32Data(data, g_reachableTime);
    data += 4; /* 可达时间需要占4个字节 */

    /* 重发定时器 */
    IP_SetUint32Data(data, g_retransTimer);

    return;
}


/*
 * Function Name: IP_NDSERVER_FormRaOptMsg
 * Description: RA报文选项部分设置
 * Input: ulIndex --- ND SERVER实体索引
 *                   pucMacAddr- 源链路层地址
 * Output: pucData --- 报文缓冲指针
 *                   pulLen ---- 报文长度指针
 * History:
 *    1.      2011-04-07  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormRaOptMsg(VOS_UINT32 addrIndex, const ND_MacAddrInfo macAddr, VOS_UINT8 *data,
                                         VOS_UINT32 *len)
{
    ESM_IP_Ipv6NWPara        *nwPara = IP_NULL_PTR;
    VOS_UINT32                count;
    VOS_UINT32                tmpMtu;
    VOS_INT32                 rlt;
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    nwPara = IP_NDSERVER_ADDRINFO_GET_NWPARA(addrIndex);

    /* 类型 */
    *data = IP_ICMPV6_OPT_SRC_LINK_LAYER_ADDR;
    data++;

    /* 长度 */
    *data = 1;
    data++;

    /* 链路层地址 */
    rlt = memcpy_s(data, IP_MAC_ADDR_LEN, macAddr.macAddr, IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    data += IP_MAC_ADDR_LEN;

    (*len) += IP_ND_OPT_UNIT_LEN;

    if (nwPara->opMtu == IP_IPV6_OP_TRUE) {
        /* 类型 */
        *data = IP_ICMPV6_OPT_MTU;
        data++;

        /* 长度 */
        *data = 1;
        data++;

        /* 2:保留 */
        data += 2;

        /* MTU: 取NV中的MTU和RA中的MTU的最小值作为发给PC的MTU */
        tmpMtu = PS_MIN(g_nvMtu, nwPara->mtuValue);
        IP_SetUint32Data(data, tmpMtu);
        data += 4; /* MTU长度占4个字节 */

        (*len) += IP_ND_OPT_UNIT_LEN;
    }

    for (count = 0; count < nwPara->prefixNum; count++) {
        /* 类型 */
        *data = IP_ICMPV6_OPT_PREFIX_INFO;
        data++;

        /* 4:长度 */
        *data = 4;
        data++;

        /* 前缀长度 */
        *data = (VOS_UINT8)(nwPara->prefixList[count].prefixLen);
        data++;

        /* 链路上标志、自治标志 */
        *data = (VOS_UINT8)(((nwPara->prefixList[count].prefixLValue & 0x1) << 7) | /* 7:bit */
                            ((nwPara->prefixList[count].prefixAValue & 0x1) << 6)); /* 6:bit */
        data++;

        /* 有效生存期 */
        IP_SetUint32Data(data, nwPara->prefixList[count].validLifeTime);
        data += 4; /* 有效生存期占4个字节 */

        /* 选用生存期 */
        IP_SetUint32Data(data, nwPara->prefixList[count].preferredLifeTime);
        data += 4; /* 选用生存期占4个字节 */

        /* 4:保留字段 */
        data += 4;

        /* 前缀 */
        rlt = memcpy_s(data, IP_IPV6_ADDR_LEN, nwPara->prefixList[count].prefix, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
        data += IP_IPV6_ADDR_LEN;

        (*len) += (4 * IP_ND_OPT_UNIT_LEN); /* 4:len */
    }

    return;
}

/*
 * Function Name: IP_NDSERVER_FormRaMsg
 * Description: 生成路由公告消息
 * Input: ulIndex ------- 处理RA发包实体索引
 *                   pstNdMsgData -- 目的信息参数
 * Output: pucSendBuff --- 发送RA报文缓冲
 *                   pulSendLen ---- 发送报文长度
 * History:
 *    1.      2011-04-07  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_FormRaMsg(VOS_UINT32 addrIndex, const IP_ND_Msg *ndMsgData, VOS_UINT8 *sendBuff,
                                      VOS_UINT32 *sendLen)
{
    VOS_UINT32                  packetLen                    = IP_NULL;
    VOS_UINT8                  *data                        = sendBuff;
    ND_Ipv6AddrInfo             srcIPAddr;
    ND_MacAddrInfo              srcMacAddr;
    ND_Ipv6AddrInfo             dstIPAddr;
    ND_MacAddrInfo              dstMacAddr;
    IP_NdserverAddrInfo *infoAddr                    = IP_NULL_PTR;
    VOS_INT32                   rlt;
    /* 打印进入该函数 */
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormRaMsg is entered.");
    IP_NDSERVER_InitAddrInfo(&srcIPAddr, &srcMacAddr, &dstIPAddr, &dstMacAddr);
    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);

    data += IP_ETHERNET_HEAD_LEN + IP_IPV6_HEAD_LEN;

    /* 设置RA报头 */
    IP_NDSERVER_FormRaHeaderMsg(addrIndex, data);

    packetLen += IP_ICMPV6_RA_HEADER_LEN;

    /* 根据自身MAC地址生成link-local地址 */
    rlt = memcpy_s(srcMacAddr.macAddr, IP_MAC_ADDR_LEN, (VOS_VOID *)Ndis_GetMacAddr(), IP_MAC_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    IP_ProduceIfaceIdFromMacAddr(srcIPAddr.ipv6Addr, srcMacAddr.macAddr);
    IP_SetUint16Data(srcIPAddr.ipv6Addr, IP_IPV6_LINK_LOCAL_PREFIX);
    rlt = memcpy_s(infoAddr->ueLinkLocalAddr, IP_IPV6_ADDR_LEN, srcIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

    /* 设置RA报文选项 */
    IP_NDSERVER_FormRaOptMsg(addrIndex, srcMacAddr, (VOS_VOID *)(data + packetLen), &packetLen);

    /* 确定单播或组播方式 */
    if ((ndMsgData != VOS_NULL_PTR) && (IP_IPV6_IS_LINKLOCAL_ADDR(ndMsgData->srcIp)) &&
        (ndMsgData->ndMsgStru.rsInfo.opSrcLinkLayerAddr == 1)) {
        rlt = memcpy_s(dstIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN, ndMsgData->srcIp, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        rlt = memcpy_s(dstMacAddr.macAddr, IP_MAC_ADDR_LEN, ndMsgData->ndMsgStru.rsInfo.srcLinkLayerAddr,
                       IP_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    } else {
        rlt = memcpy_s(dstIPAddr.ipv6Addr, IP_IPV6_ADDR_LEN, g_ndAllNodesMulticaseAddr, IP_IPV6_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);

        rlt = memcpy_s(dstMacAddr.macAddr, IP_MAC_ADDR_LEN, g_ndAllNodesMacAddr, IP_MAC_ADDR_LEN);
        IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    }

    data -= IP_IPV6_HEAD_LEN;

    /* 设置IPv6报头 */
    IP_ND_FormIPv6HeaderMsg(srcIPAddr, dstIPAddr, packetLen, data, IP_HEAD_PROTOCOL_ICMPV6);

    /* 生成ICMPv6报头校验和 */
    if (IP_BuildIcmpv6Checksum(data, IP_IPV6_HEAD_LEN) != IP_SUCC) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_FormRaMsg: Build ICMPv6 Checksum failed.");
    }

    data -= IP_ETHERNET_HEAD_LEN;

    /* 设置以太报头 */
    IP_NDSERVER_FormEtherHeaderMsg(srcMacAddr, dstMacAddr, data);

    /* 返回报文长度 */
    *sendLen = packetLen + IP_IPV6_HEAD_LEN + IP_ETHERNET_HEAD_LEN;

    return;
}

/*
 * Function Name: IP_NDSERVER_SendRaMsg
 * Description: 发送路由公告消息
 * Input: ulIndex ------- 处理RA发包实体索引
 *                   pstNdMsgData -- 目的信息参数
 * History: VOS_UINT32
 *    1.      2011-04-07  Draft Enact
 */
VOS_UINT32 IP_NDSERVER_SendRaMsg(VOS_UINT32 addrIndex, const IP_ND_Msg *ndMsgData)
{
    VOS_UINT8 *sendBuff = VOS_NULL_PTR;
    VOS_UINT32 sendLen   = IP_NULL;
    VOS_UINT32 epsbId;

    IP_ASSERT_RTN(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM, PS_FAIL);
    epsbId = IP_NDSERVER_GET_EPSBID(addrIndex);

    sendBuff = IP_NDSERVER_GET_SENDMSG_BUFFER();
    (VOS_VOID)memset_s(sendBuff, IP_IPM_MTU, IP_NULL, IP_IPM_MTU);

    /* 调用形成RA消息函数 */
    IP_NDSERVER_FormRaMsg(addrIndex, ndMsgData, sendBuff, &sendLen);

    IP_NDSERVER_AddTransRaPktNum(addrIndex);
    IP_NDSERVER_AddTransPktTotalNum(addrIndex);

    /* 将RA消息发送到PC */
    return NdSer_SendPkt(sendBuff, sendLen, (VOS_UINT8)epsbId);
}



