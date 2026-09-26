/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019.All rights reserved..
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

#include "netfilter_ex.h"
#include "netfilter_ex_ctrl.h"
#include "linux/inet.h"
#include "ttf_comm.h"

#define THIS_FILE_ID PS_FILE_ID_ACPU_NFEX_C
#define OM_SOCK_PORT_NUM 3000 /* 与OM的宏SOCK_PORT_NUM保持一致 */

enum NfExtFlagOmDataEnum {
    NF_EXT_FLAG_OM_DATA     = 0, /* OM消息标志 */
    NF_EXT_FLAG_NOT_OM_DATA = 1, /* 非OM消息标志 */

    NF_EXT_FLAG_OM_DATA_BUTT
};
typedef VOS_INT NfExtFlagOmDataEnumU32;

/* 判断是否OM的数据,对于协议栈中的om数据不能抓包，否则抓包风暴 */
STATIC NfExtFlagOmDataEnumU32 NFEXT_IsOmData(struct sk_buff* skb)
{
    const struct iphdr*  ipHeader  = (struct iphdr*)skb_network_header(skb);
    struct tcphdr*       tcpHeader = VOS_NULL_PTR;
    __be32         srcIp;
    __be32         destIp;
    __be16         srcPort;
    __be16         destPort;
    __be32         omSocketIp;

    /* 如果不是TCP报文则直接返回 */
    if (ipHeader->protocol != IP_PROTOCOL_TCP) {
        return NF_EXT_FLAG_NOT_OM_DATA;
    }

    /* 传输层的数据在ip层之后 */
    tcpHeader = (struct tcphdr*)(skb_network_header(skb) + sizeof(struct iphdr));

    srcIp      = ipHeader->saddr;
    destIp     = ipHeader->daddr;
    srcPort    = ntohs(tcpHeader->source);
    destPort   = ntohs(tcpHeader->dest);
    omSocketIp = NFEXT_GetEntityOmIp();

    if (((srcIp == omSocketIp) && (srcPort == OM_SOCK_PORT_NUM)) ||
        ((destIp == omSocketIp) && (destPort == OM_SOCK_PORT_NUM))) {
        return NF_EXT_FLAG_OM_DATA;
    }

    return NF_EXT_FLAG_NOT_OM_DATA;
}

#define NF_EXT_MAX_IP_SIZE 1500

/* 将勾取网桥中转报文导出到HIDS */
STATIC VOS_VOID NFEXT_BrDataExport(struct sk_buff* skb, const struct net_device* device_in,
    const struct net_device* device_out, TTF_MntnMsgTypeUint16 msgType)
{
    VOS_UINT8* data = VOS_NULL_PTR;
    VOS_UINT16 hookDataLen;

    /* skb->data指向数据包的IP头部，不上移14个(MAC_HEADER_LENGTH)字节令pucData指向数据包的mac头部 */
    data        = skb->data;
    hookDataLen = ((skb->len > NF_EXT_MAX_IP_SIZE) ? NF_EXT_MAX_IP_SIZE : skb->len);

    IPSMNTN_BridgePktInfoCB(device_in->name, device_out->name, data, hookDataLen, msgType);
}

/*  将勾取ARP报文导出到HIDS */
STATIC VOS_VOID NFEXT_ArpDataExport(struct sk_buff* skb, const struct net_device* device, TTF_MntnMsgTypeUint16 msgType)
{
    const VOS_UINT8* data = VOS_NULL_PTR;
    VOS_UINT32       hookDataLen;

    data        = skb->data;
    hookDataLen = skb->len;

    IPSMNTN_CtrlPktInfoCB(device->name, sizeof(device->name), data, (VOS_UINT16)hookDataLen, msgType);
}

/* 将勾取IP报文导出到HIDS */
STATIC VOS_VOID NFEXT_IpDataExport(struct sk_buff* skb, const struct net_device* device, TTF_MntnMsgTypeUint16 msgType)
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return;
    }

    IPSMNTN_PktInfoCB(device->name, sizeof(device->name), skb, msgType);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 1, 0))
/* BRIGE钩子函数 */
unsigned int NFEXT_BrPreRoutingHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    /* 判断是否OM的数据 */
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, in, out, ID_IPS_TRACE_BRIDGE_PRE_ROUTING_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrPostRoutingHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, in, out, ID_IPS_TRACE_BRIDGE_POST_ROUTING_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrLocalInHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, in, out, ID_IPS_TRACE_BRIDGE_LOCAL_IN_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrLocalOutHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, in, out, ID_IPS_TRACE_BRIDGE_LOCAL_OUT_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrForwardHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_BrDataExport(skb, in, out, ID_IPS_TRACE_BRIDGE_DATA_INFO);
    return NF_ACCEPT;
}

/* ARP钩子函数 */
unsigned int NFEXT_ArpInHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_ArpDataExport(skb, in, ID_IPS_TRACE_RECV_ARP_PKT);
    return NF_ACCEPT;
}

/* ARP钩子函数 */
unsigned int NFEXT_ArpOutHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_ArpDataExport(skb, out, ID_IPS_TRACE_SEND_ARP_PKT);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4PreRoutingHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, in, ID_IPS_TRACE_INPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4PostRoutingHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, out, ID_IPS_TRACE_OUTPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4LocalInHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, in, ID_IPS_TRACE_IP4_LOCAL_IN_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4LocalOutHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, out, ID_IPS_TRACE_IP4_LOCAL_OUT_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4ForwardHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, in, ID_IPS_TRACE_IP4_FORWARD_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6PreRoutingHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, in, ID_IPS_TRACE_INPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6PostRoutingHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, out, ID_IPS_TRACE_OUTPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6LocalInHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, in, ID_IPS_TRACE_IP6_LOCAL_IN_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6LocalOutHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, out, ID_IPS_TRACE_IP6_LOCAL_OUT_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6ForwardHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NFEXT_IpDataExport(skb, in, ID_IPS_TRACE_IP6_FORWARD_INFO);
    return NF_ACCEPT;
}

/* 网桥forward流控点 */
unsigned int NFEXT_BrForwardFlowCtrlHook(unsigned int hookNum, struct sk_buff* skb, const struct net_device* in,
    const struct net_device* out, int (*okfn)(struct sk_buff*))
{
    NF_EXT_STATS_INC(1, NF_EXT_STATS_BR_FC_ENTER);
    /* 网桥转发统计 */
    NFEXT_TxBytesInc(skb->len);

    /* 当前在网桥forward流控状态，直接丢包 */
    if ((NFEXT_GetFcMask() & NF_EXT_BR_FORWARD_FLOW_CTRL_MASK) == NF_EXT_BR_FORWARD_FLOW_CTRL_MASK) {
        NF_EXT_STATS_INC(1, NF_EXT_STATS_BR_FC_DROP);
        return NF_DROP;
    }
    return NF_ACCEPT;
}

#else
/* BRIGE钩子函数 */
unsigned int NFEXT_BrPreRoutingHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    /* 判断是否OM的数据 */
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, state->in, state->out, ID_IPS_TRACE_BRIDGE_PRE_ROUTING_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrPostRoutingHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, state->in, state->out, ID_IPS_TRACE_BRIDGE_POST_ROUTING_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrLocalInHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, state->in, state->out, ID_IPS_TRACE_BRIDGE_LOCAL_IN_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrLocalOutHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    if (NFEXT_IsOmData(skb) == NF_EXT_FLAG_OM_DATA) {
        return NF_ACCEPT;
    }

    NFEXT_BrDataExport(skb, state->in, state->out, ID_IPS_TRACE_BRIDGE_LOCAL_OUT_INFO);
    return NF_ACCEPT;
}

/* BRIGE钩子函数 */
unsigned int NFEXT_BrForwardHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_BrDataExport(skb, state->in, state->out, ID_IPS_TRACE_BRIDGE_DATA_INFO);
    return NF_ACCEPT;
}

/* ARP钩子函数 */
unsigned int NFEXT_ArpInHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_ArpDataExport(skb, state->in, ID_IPS_TRACE_RECV_ARP_PKT);
    return NF_ACCEPT;
}

/* ARP钩子函数 */
unsigned int NFEXT_ArpOutHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_ArpDataExport(skb, state->out, ID_IPS_TRACE_SEND_ARP_PKT);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4PreRoutingHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->in, ID_IPS_TRACE_INPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4PostRoutingHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->out, ID_IPS_TRACE_OUTPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4LocalInHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->in, ID_IPS_TRACE_IP4_LOCAL_IN_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4LocalOutHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->out, ID_IPS_TRACE_IP4_LOCAL_OUT_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip4ForwardHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->in, ID_IPS_TRACE_IP4_FORWARD_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip6PreRoutingHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->in, ID_IPS_TRACE_INPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV4钩子函数 */
unsigned int NFEXT_Ip6PostRoutingHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->out, ID_IPS_TRACE_OUTPUT_DATA_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6LocalInHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->in, ID_IPS_TRACE_IP6_LOCAL_IN_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6LocalOutHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->out, ID_IPS_TRACE_IP6_LOCAL_OUT_INFO);
    return NF_ACCEPT;
}

/* IPV6钩子函数 */
unsigned int NFEXT_Ip6ForwardHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NFEXT_IpDataExport(skb, state->in, ID_IPS_TRACE_IP6_FORWARD_INFO);
    return NF_ACCEPT;
}

/* 网桥forward流控点 */
unsigned int NFEXT_BrForwardFlowCtrlHook(void* priv, struct sk_buff* skb, const struct nf_hook_state* state)
{
    NF_EXT_STATS_INC(1, NF_EXT_STATS_BR_FC_ENTER);
    /* 网桥转发统计 */
    NFEXT_TxBytesInc(skb->len);

    /* 当前在网桥forward流控状态，直接丢包 */
    if ((NFEXT_GetFcMask() & NF_EXT_BR_FORWARD_FLOW_CTRL_MASK) == NF_EXT_BR_FORWARD_FLOW_CTRL_MASK) {
        NF_EXT_STATS_INC(1, NF_EXT_STATS_BR_FC_DROP);
        return NF_DROP;
    }
    return NF_ACCEPT;
}

#endif
