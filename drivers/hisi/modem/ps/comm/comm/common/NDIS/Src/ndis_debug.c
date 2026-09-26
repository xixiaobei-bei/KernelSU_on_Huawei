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

#include "ndis_debug.h"
#include "vos.h"
#include "ps_tag.h"
#include "ps_common_def.h"
#include "ndis_om.h"
#include "ndis_interface.h"
#include "ndis_entity.h"


#define THIS_MODU ps_ndis
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDISDEBUG_C
/*lint +e767*/

VOS_UINT32          g_ndisLomSwitch = 0;
VOS_UINT32 g_ulGUNdisOMSwitch = PS_FALSE;


/*
 * 2 Declare the Global Variable
 */

VOS_VOID Ndis_LomTraceRcvUlData(VOS_VOID)
{
    if (g_ndisLomSwitch == 1) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "User plane latency trace: NDIS rcv UL data");
    }
    return;
}

VOS_VOID Ndis_LomTraceRcvDlData(VOS_VOID)
{
    if (g_ndisLomSwitch == 1) {
        NDIS_ERROR_LOG(NDIS_TASK_PID, "User plane latency trace: NDIS rcv DL data");
    }
    return;
}

VOS_VOID NDIS_OpenLatency(VOS_VOID)
{
    g_ndisLomSwitch = 1;
    return;
}

VOS_VOID NDIS_CloseLatency(VOS_VOID)
{
    g_ndisLomSwitch = 0;
    return;
}
VOS_VOID   GU_NDIS_OM_SWITCH_ON(VOS_VOID)
{
    g_ulGUNdisOMSwitch = PS_TRUE;
    return;
}

VOS_VOID GU_NDIS_OM_SWITCH_OFF(VOS_VOID)
{
    g_ulGUNdisOMSwitch = PS_FALSE;
    return;
}


/*
 * 功能描述: 显示收发的业务数据信�?
 *  修改历史:
 *   1.�?    �?: 2009�?6�?16�?
 *     修改内容: 新生成函�?
 * 修改历史:
 *   2.�?    �?: 2013�?1�?22�?
 *     修改内容: DSDA
 */
VOS_VOID Ndis_ShowStat(VOS_VOID)
{
    PS_PRINTF_ERR("uplink discard packets num           %d\n", g_ndisStatStru.dicardUsbFrmNum);
    PS_PRINTF_ERR("uplink packets recevied from USB     %d\n", g_ndisStatStru.recvUsbPktSuccNum);
    PS_PRINTF_ERR("uplink packets sent to ADS           %d\n", g_ndisStatStru.sendPktToAdsSucNum);
    PS_PRINTF_ERR("downlink discarded ADS packets       %d\n", g_ndisStatStru.dicardAdsPktNum);
    PS_PRINTF_ERR("downlink received ADS succ           %d\n", g_ndisStatStru.recvAdsPktSuccNum);
    PS_PRINTF_ERR("downlink get IPV6 MAC fail           %d\n", g_ndisStatStru.getIpv6MacFailNum);
    PS_PRINTF_ERR("downlink packet diff with Rab        %d\n", g_ndisStatStru.dlPktDiffRabNum);
    PS_PRINTF_ERR("downlink add MAC head fail           %d\n", g_ndisStatStru.addMacHdrFailNum);
    PS_PRINTF_ERR("downlink send packet fail            %d\n", g_ndisStatStru.dlSendPktFailNum);
    PS_PRINTF_ERR("downlink send packet succ            %d\n", g_ndisStatStru.dlSendPktSuccNum);

    PS_PRINTF_ERR("\nrecv DHCP                          %d\n", g_ndisStatStru.recvDhcpPktNum);
    PS_PRINTF_ERR("recv ARP Request                     %d\n", g_ndisStatStru.recvArpReq);
    PS_PRINTF_ERR("recv ARP Reply                       %d\n", g_ndisStatStru.recvArpReply);
    PS_PRINTF_ERR("proc error ARP packets               %d\n", g_ndisStatStru.procArpError);
    PS_PRINTF_ERR("proc Not Ue Arp                      %d\n", g_ndisStatStru.procNotUeArp);
    PS_PRINTF_ERR("proc Free Arp                        %d\n", g_ndisStatStru.procFreeArp);
    PS_PRINTF_ERR("send ARP Req succ                    %d\n", g_ndisStatStru.sendArpReqSucc);
    PS_PRINTF_ERR("send ARP Req fail                    %d\n", g_ndisStatStru.sendArpReqFail);
    PS_PRINTF_ERR("send ARP Req No Reply                %d\n", g_ndisStatStru.arpReplyNotRecv);
    PS_PRINTF_ERR("send ARP Reply                       %d\n", g_ndisStatStru.sendArpReply);
    PS_PRINTF_ERR("send ARP or DHCP or ND fail          %d\n", g_ndisStatStru.sendArpDhcpNDFailNum);
    PS_PRINTF_ERR("MAC invalid pkt num                  %d\n", g_ndisStatStru.macInvalidPktNum);
    PS_PRINTF_ERR("dl en que pkt num                    %d\n", g_ndisStatStru.enquePktNum);
    PS_PRINTF_ERR("dl send que pkt num                  %d\n", g_ndisStatStru.sendQuePktNum);
    return;
}

/*
 * 功能描述: 显示有效的实体信�?
 * 修改历史:
 *  1.�?    �?: 2011�?3�?16�?
 *    修改内容: 新生成函�?
 */
VOS_VOID Ndis_ShowValidEntity(VOS_UINT16 modemId, VOS_UINT8 rabId)
{
    NDIS_Entity      *entity = VOS_NULL_PTR;
    VOS_UINT8         exRabId;

    exRabId = NDIS_FORM_EXBID(modemId, rabId);
    entity = NDIS_GetEntityByRabId(exRabId);
    if (entity == VOS_NULL_PTR) {
        PS_PRINTF_ERR("             no right NDIS entity    \n");
        return;
    }

    PS_PRINTF_ERR("                 ModemID:  %d\n", NDIS_GET_MODEMID_FROM_EXBID(entity->rabId));
    PS_PRINTF_ERR("                 EPS bearer ID:  %d\n", NDIS_GET_BID_FROM_EXBID(entity->rabId));
    PS_PRINTF_ERR("             ARP got flag:  %d\n", entity->ipV4Info.arpInitFlg);
    PS_PRINTF_ERR(" ARP recv reply flag:  %d\n", entity->ipV4Info.arpRepFlg);

    PS_PRINTF_ERR("\n======================================================\n");
}

/*
 * 功能描述: 显示�?有的实体信息
 * 修改历史:
 *  1.�?    �?: 2011�?4�?18�?
 *    修改内容: 新生成函�?
 *  2.�?    �?: 2013�?1�?16�?
 *    修改内容: DSDA
 */
VOS_VOID Ndis_ShowAllEntity(VOS_VOID)
{
    VOS_UINT32   loop;
    NDIS_Entity *entity = VOS_NULL_PTR;

    for (loop = 0; loop < NAS_NDIS_MAX_ITEM; loop++) {
        entity = &g_ndisEntityAddr[loop];
        if (entity->used == PS_FALSE) {
            PS_PRINTF_ERR("                 ModemID:  %d\n", NDIS_GET_MODEMID_FROM_EXBID(entity->rabId));
            PS_PRINTF_ERR("                 EPS bearer ID %d inactive\n", NDIS_GET_BID_FROM_EXBID(entity->rabId));
            continue;
        }

        PS_PRINTF_ERR("                 ModemID:  %d\n", NDIS_GET_MODEMID_FROM_EXBID(entity->rabId));
        PS_PRINTF_ERR("                 EPS bearer ID:  %d\n", NDIS_GET_BID_FROM_EXBID(entity->rabId));
        PS_PRINTF_ERR("             ARP got flag:  %d\n", entity->ipV4Info.arpInitFlg);
        PS_PRINTF_ERR(" ARP recv reply flag:  %d\n", entity->ipV4Info.arpRepFlg);

        PS_PRINTF_ERR("\n======================================================\n");
    }
}

/*
 * Function Name: IPV4_DHCP_ShowDebugInfo
 * Description: ��ӡDHCP Server������Ϣ
 * History:
 *    1.      2011-3-7  Draft Enact
 */
VOS_VOID IPV4_DHCP_ShowDebugInfo(VOS_VOID)
{
    PS_PRINTF_ERR("Recv Dhcp Packet              :    %d \r\n", g_dhcpStatStru.recvDhcpPkt);
    PS_PRINTF_ERR("Recv Dhcp Discover  Msg       :    %d \r\n", g_dhcpStatStru.recvDiscoverMsg);
    PS_PRINTF_ERR("Recv Dhcp Request   Msg       :    %d \r\n", g_dhcpStatStru.recvRequestMsg);
    PS_PRINTF_ERR("Recv Dhcp Selecting Msg       :    %d \r\n", g_dhcpStatStru.recvSelectingReqMsg);
    PS_PRINTF_ERR("Recv Dhcp Other     Msg       :    %d \r\n", g_dhcpStatStru.recvOtherReqMsg);
    PS_PRINTF_ERR("Recv Dhcp Release   Msg       :    %d \r\n", g_dhcpStatStru.recvReleaseMsg);
    PS_PRINTF_ERR("Recv Dhcp Other Typ Msg       :    %d \r\n", g_dhcpStatStru.recvOtherTypeMsg);
    PS_PRINTF_ERR("Proc Dhcp Packet    Error     :    %d \r\n", g_dhcpStatStru.procErr);
    PS_PRINTF_ERR("Send Dhcp Offer               :    %d \r\n", g_dhcpStatStru.sendOffer);
    PS_PRINTF_ERR("Send Dhcp Ack                 :    %d \r\n", g_dhcpStatStru.sendAck);
    PS_PRINTF_ERR("Send Dhcp NACK                :    %d \r\n", g_dhcpStatStru.sendNack);
    PS_PRINTF_ERR("Send Dhcp Mac Frame Succ      :    %d \r\n", g_dhcpStatStru.sendDhcpPktSucc);
    PS_PRINTF_ERR("Send Dhcp Mac Frame Fail      :    %d \r\n", g_dhcpStatStru.sendDhcpPktFail);

    return;
}



