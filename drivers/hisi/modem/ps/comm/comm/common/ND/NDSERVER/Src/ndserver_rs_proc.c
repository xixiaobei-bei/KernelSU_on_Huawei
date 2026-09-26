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

#include "ndserver_rs_proc.h"
#include "securec.h"
#include "ps_common_def.h"
#include "ip_ipm_global.h"
#include "ip_comm.h"
#include "ndserver_om.h"
#include "ip_nd_server.h"
#include "ndserver_ra_proc.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERRSPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */

STATIC VOS_UINT32 IP_NDSERVER_SetDestRsDataInfo(IP_ND_Msg *destData, VOS_UINT32 icmpv6HeadOffset,
                                                VOS_UINT32 payLoad, const VOS_UINT8 *ipMsg, VOS_INT32 remainLen)
{
    VOS_INT32          rlt;
    if (remainLen < 0) {
        IPND_ERROR_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeRsData: Invalid IPV6 PayLoad::!", payLoad,
                        icmpv6HeadOffset);
        return IP_FAIL;
    }

    rlt = memcpy_s(destData->srcIp, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_SRC_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    rlt = memcpy_s(destData->desIp, IP_IPV6_ADDR_LEN, ipMsg + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
    return IP_SUCC;
}
STATIC VOS_UINT32 IP_ProcRsOptByOptType(VOS_UINT32 optType, VOS_UINT32 optLen, IP_ND_MsgRs *rs, const VOS_UINT8 *ipMsg)
{
    VOS_INT32          rlt;
    switch (optType) {
        case IP_ICMPV6_OPT_SRC_LINK_LAYER_ADDR: {
            if (optLen == 1) {
                if (rs->opSrcLinkLayerAddr == 0) {
                    rlt = memcpy_s(rs->srcLinkLayerAddr, IP_MAC_ADDR_LEN, ipMsg + 2, IP_MAC_ADDR_LEN); /* 2:len */
                    IP_CHK_SEC_RETURN_VAL(rlt != EOK, IP_FAIL);
                    rs->opSrcLinkLayerAddr = 1;
                } else {
                    IPND_ERROR_LOG(NDIS_NDSERVER_PID,
                                   "IP_ProcRsOptByOptType: Redundant Source Link-Layer Addr!");
                }
            } else {
                IPND_ERROR_LOG1(NDIS_NDSERVER_PID,
                                "IP_ProcRsOptByOptType: Invalid Source Link-Layer Addr Length:!", optLen);
            }
        } break;
        default:
            break;
    }
    return IP_SUCC;
}
/*
 * Description: 对RS包进行格式转换
 */
STATIC VOS_UINT32 IP_NDSERVER_DecodeRsData(const VOS_UINT8 *srcData, IP_ND_Msg *destData, VOS_UINT32 icmpv6HeadOffset)
{
    VOS_UINT32         payLoad;
    VOS_INT32          remainLen;
    VOS_UINT32         optType;
    VOS_UINT32         optLen;
    const VOS_UINT8         *ipMsg = srcData;
    IP_ND_MsgRs       *rs    = &destData->ndMsgStru.rsInfo;
    /* 获取PAYLOAD */
    IP_GetUint16Data(payLoad, ipMsg + IP_IPV6_BASIC_HEAD_PAYLOAD_OFFSET);

    remainLen = (VOS_INT32)(payLoad + IP_IPV6_HEAD_LEN - (icmpv6HeadOffset + IP_ICMPV6_RS_HEADER_LEN));
    if (IP_NDSERVER_SetDestRsDataInfo(destData, icmpv6HeadOffset, payLoad, ipMsg, remainLen) == IP_FAIL) {
        return IP_FAIL;
    }

    /*lint -e679 -esym(679,*)*/
    ipMsg += icmpv6HeadOffset + IP_ICMPV6_RS_HEADER_LEN;
    /*lint -e679 +esym(679,*)*/

    while (remainLen >= IP_ND_OPT_UNIT_LEN) {
        optType = *ipMsg;
        optLen  = *(ipMsg + 1);

        if (optLen == 0) {
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeRsData: Invalid ND options length 0!");
            return IP_FAIL;
        }
        if (IP_ProcRsOptByOptType(optType, optLen, rs, ipMsg) == IP_FAIL) {
            return IP_FAIL;
        }

        remainLen -= (VOS_INT32)IP_GetNdOptionLen(optLen);
        /*lint -e679 -esym(679,*)*/
        ipMsg += IP_GetNdOptionLen(optLen);
        /*lint -e679 +esym(679,*)*/
    }

    if (remainLen != 0) {
        IPND_ERROR_LOG1(NDIS_NDSERVER_PID, "IP_NDSERVER_DecodeRsData: Payload not match Package:!", remainLen);
    }

    return IP_SUCC;
}

/*
 * Function Name: IP_NDSERVER_RsMsgProc
 * Description: 处理RS消息
 * Input: ulIndex  -------------- 处理消息实体索引
 *                   pucSrcData ------------ IP数据报
 *                   ulIcmpv6HeadOffset ---- ICMPv6报文头偏移量
 * History:
 *    1.      2011-04-06  Draft Enact
 */
VOS_VOID IP_NDSERVER_RsMsgProc(VOS_UINT32 addrIndex, const VOS_UINT8 *srcData, VOS_UINT32 icmpv6HeadOffset)
{
    const VOS_UINT8                  *ipMsg     = srcData;
    IP_ND_Msg                  *ndMsgData = VOS_NULL_PTR;
    IP_NdserverAddrInfo        *infoAddr  = IP_NULL_PTR;
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_RsMsgProc is entered.");

    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);
    IP_NDSERVER_AddRcvRsPktNum(addrIndex);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    if (infoAddr->validFlag != IP_TRUE) {
        IP_NDSERVER_AddErrRsPktNum(addrIndex);
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_RsMsgProc: ND Server info flag is invalid!");
        return;
    }

    ndMsgData = IP_NDSERVER_GET_NDMSGDATA_ADDR();
    (VOS_VOID)memset_s(ndMsgData, sizeof(IP_ND_Msg), IP_NULL, sizeof(IP_ND_Msg));

    if (IP_NDSERVER_DecodeRsData(ipMsg, ndMsgData, icmpv6HeadOffset) != IP_SUCC) {
        IP_NDSERVER_AddErrRsPktNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcessRsMsg: Invalid IPV6 RS Msg:!");
        return;
    }

    /* 发送RA消息到PC */
    if (IP_NDSERVER_SendRaMsg(addrIndex, ndMsgData) != PS_SUCC) {
        IP_NDSERVER_AddTransPktFailNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_RsMsgProc:Send RA Msg failed.");
    }

    return;
}



