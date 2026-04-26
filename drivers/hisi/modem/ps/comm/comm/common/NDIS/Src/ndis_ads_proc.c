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

#include "ndis_ads_proc.h"
#include "vos.h"
#include "securec.h"
#include "imm_interface.h"
#include "ads_ndis_interface.h"
#include "ip_nd_server.h"
#include "ipv4_dhcp_server.h"
#include "ndserver_dhcpv6pkt_proc.h"
#include "ndserver_pkt_comm.h"
#include "ndis_interface.h"
#include "ndis_entity.h"
#include "ndis_om.h"
#include "ndis_msg_proc.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISADSPROC_C
/*lint +e767*/


/*
 * 2 Declare the Global Variable
 */

/*
 * 功能描述: DHCP处理
 * 修改历史:
 *  1.日    期: 2011年2月11日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_DHCPPkt_Proc(const MsgBlock *msgBlock)
{
    VOS_UINT8              *data       = VOS_NULL_PTR;
    NDIS_Entity            *ndisEntity = VOS_NULL_PTR;
    VOS_UINT8               exRabId;
    VOS_UINT32              pktMemLen;
    ADS_NDIS_DataInd       *rcvMsg = (ADS_NDIS_DataInd *)(VOS_VOID *)msgBlock;

    data = IMM_ZcGetDataPtr(rcvMsg->data);
    if (data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, IMM_ZcGetDataPtr fail!");
        return;
    }

    pktMemLen = IMM_ZcGetUsedLen(rcvMsg->data);
    exRabId = NDIS_FORM_EXBID(rcvMsg->modemId, rcvMsg->rabId);
    if (Ndis_ChkRabIdValid(exRabId) != PS_SUCC) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, Ndis_ChkRabIdValid fail!");
        return;
    }

    ndisEntity = NDIS_GetEntityByRabId(exRabId);
    if (ndisEntity == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, NDIS_GetEntityByRabId fail!");
        return;
    }

    if ((ndisEntity->rabType & NDIS_ENTITY_IPV4) != NDIS_ENTITY_IPV4) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_DHCPPkt_Proc, Rab not support IPV4!");
        return;
    }

    /* DHCP处理 */
    NDIS_STAT_UL_RECV_DHCPPKT(1);
    IPV4_DHCP_ProcDhcpPkt(data, exRabId, pktMemLen);

    return;
}

/*
 * 功能描述: NDIS接收ADS消息处理函数
 * 修改历史:
 *  1.日    期: 2011年12月15日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_AdsMsgProc(const MsgBlock *msgBlock)
{
    ADS_NDIS_DataInd *adsNdisMsg = (ADS_NDIS_DataInd *)(VOS_VOID *)msgBlock;

    if (adsNdisMsg->data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_AdsMsgProc recv NULL PTR!");
        return;
    }

    if (adsNdisMsg->msgId != ID_ADS_NDIS_DATA_IND) {
        /*lint -e522*/
        IMM_ZcFree(adsNdisMsg->data);
        /*lint +e522*/
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_AdsMsgProc, MsgId error!");
        return;
    }

    switch (adsNdisMsg->ipPacketType) {
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV4: /* DHCP包 */
            Ndis_DHCPPkt_Proc(msgBlock);
            break;
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV6: /* DHCPV6包 */
            NdSer_DhcpV6PktProc(msgBlock);
            break;
        case ADS_NDIS_IP_PACKET_TYPE_ICMPV6: /* ND和ECHO REQUEST包 */
            NdSer_NdAndEchoPktProc(msgBlock);
            break;

        default:
            NDIS_WARNING_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgProc:Other Msg!", adsNdisMsg->ipPacketType);
            break;
    }

    /* 处理完成后释放ImmZc */
    /*lint -e522*/
    IMM_ZcFree(adsNdisMsg->data);
    /*lint +e522*/

    return;
}

/*
 * 功能描述: ADS V2消息内容转成V1格式
 * 修改历史:
 *  1.日    期: 2018年08月16日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_AdsV2MsgTransToV1Msg(const ADS_NDIS_DataIndV2 *v2Msg, ADS_NDIS_DataInd *v1Msg)
{
    (VOS_VOID)memset_s(v1Msg, sizeof(ADS_NDIS_DataInd), 0, sizeof(ADS_NDIS_DataInd));

    v1Msg->msgId        = ID_ADS_NDIS_DATA_IND;
    v1Msg->modemId      = NDIS_GET_MODEMID_FROM_EXBID(v2Msg->ifaceId);
    v1Msg->rabId        = NDIS_GET_BID_FROM_EXBID(v2Msg->ifaceId);
    v1Msg->ipPacketType = v2Msg->ipPacketType;
    v1Msg->data         = v2Msg->data;

    return;
}

/*
 * 功能描述: NDIS接收ADS消息处理函数
 * 修改历史:
 *  1.日    期: 2011年12月15日
 *    修改内容: 新生成函数
 */
STATIC VOS_VOID Ndis_AdsMsgProcV2(const MsgBlock *msgBlock)
{
    ADS_NDIS_DataIndV2        *adsNdisMsg = (ADS_NDIS_DataIndV2 *)(VOS_VOID *)msgBlock;
    ADS_NDIS_DataInd           adsNdisV1Msg = {0};

    if (adsNdisMsg->data == VOS_NULL_PTR) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "Ndis_AdsMsgProcV2 recv NULL PTR!");
        return;
    }

    Ndis_AdsV2MsgTransToV1Msg(adsNdisMsg, &adsNdisV1Msg);

    switch (adsNdisV1Msg.ipPacketType) {
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV4: /* DHCP包 */
            Ndis_DHCPPkt_Proc((MsgBlock *)(&adsNdisV1Msg));
            break;
        case ADS_NDIS_IP_PACKET_TYPE_DHCPV6: /* DHCPV6包 */
            NdSer_DhcpV6PktProc((MsgBlock *)(&adsNdisV1Msg));
            break;
        case ADS_NDIS_IP_PACKET_TYPE_ICMPV6: /* ND和ECHO REQUEST包 */
            NdSer_NdAndEchoPktProc((MsgBlock *)(&adsNdisV1Msg));
            break;

        default:
            NDIS_WARNING_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgProcV2:Other Msg!", adsNdisV1Msg.ipPacketType);
            break;
    }

    /* 处理完成后释放ImmZc */
    /*lint -e522*/
    IMM_ZcFree(adsNdisMsg->data);
    /*lint +e522*/

    return;
}

/*
 * 功能描述: NDIS接收ADS消息处理函数
 * 修改历史:
 *  1.日    期: 2018年08月16日
 *    修改内容: 新生成函数
 */
VOS_VOID Ndis_AdsMsgDispatch(const MsgBlock *msgBlock)
{
    AT_NDIS_MsgIdUint32 msgId;
    NDIS_MsgTypeStruLen msgStruLen[] = {
        { ID_ADS_NDIS_DATA_IND, sizeof(ADS_NDIS_DataInd) },
        { ID_ADS_NDIS_DATA_IND_V2, sizeof(ADS_NDIS_DataIndV2) },
    };
    VOS_UINT32 msgIdNum = sizeof(msgStruLen) / sizeof(NDIS_MsgTypeStruLen);
    COVERITY_TAINTED_SET(msgBlock->value); /* 鹰眼插桩 */

    /* 长度异常保护 */
    if (sizeof(MSG_Header) - VOS_MSG_HEAD_LENGTH > msgBlock->ulLength) {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgDispatch: input msg length less than struc MSG_Header",
                        msgBlock->ulLength);
        return;
    }

    msgId = ((MSG_Header *)(VOS_VOID *)msgBlock)->msgName;
    if (NDIS_RecvMsgParaCheck(msgBlock->ulLength, msgStruLen, msgIdNum, msgId) == PS_FAIL) {
        NDIS_ERROR_LOG1(NDIS_TASK_PID, "Ndis_AdsMsgDispatch: input msg length less than struc MSG_Header",
                        msgBlock->ulLength);
        return;
    }
    switch (msgId) {
        case ID_ADS_NDIS_DATA_IND: /* 根据消息的不同处理AT不同的请求 */
            Ndis_AdsMsgProc(msgBlock);
            break;

        case ID_ADS_NDIS_DATA_IND_V2:
            Ndis_AdsMsgProcV2(msgBlock);
            break;

        default:
            NDIS_WARNING_LOG(NDIS_TASK_PID, "Ndis_AdsMsgDispatch:Error Msg!");
            break;
    }

    return;
}
/*lint +e40*/




