/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019.All rights reserved.
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

#include "ips_mntn_mini.h"
#include "ips_mntn.h"
#include "ips_traffic_statistic.h"
#include "securec.h"
#include "ttf_util.h"
#include "ttf_comm.h"
#include "gucttf_tag.h"
#include "netfilter_ex_ctrl.h"
#include "ps_trace_msg.h"
#include "netfilter_buffer.h"


#define THIS_FILE_ID PS_FILE_ID_ACPU_IPS_MNTN_MINI_C
#define THIS_MODU mod_nfext

STATIC IPS_MntnTraceCfgNotifyFunc g_treceCfgNotifyCb = VOS_NULL_PTR;


/* 向OM发送配置确认消息，V_SendMsg中释放透传内容指针 */
VOS_VOID IPSMNTN_SndCfgCnf2Om(VOS_UINT32 contentLen, const VOS_VOID* transMsg)
{
    /* 分配并填写消息 */
    MsgBlock* sendMsg = VOS_AllocMsg(ACPU_PID_NFEXT, contentLen - VOS_MSG_HEAD_LENGTH);

    if (sendMsg == VOS_NULL_PTR) {
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_ERROR, "Call VOS_AllocMsg fail!\n");
        return;
    }

    (VOS_VOID)memcpy_s(sendMsg, contentLen, transMsg, contentLen);

    (VOS_VOID)PS_TRACE_AND_SND_MSG(ACPU_PID_NFEXT, sendMsg);
}

/* 网桥中转钩包配置参数检查 */
STATIC PS_BoolUint8 IPSMNTN_BridgeTraceCfgChkParam(const IPS_MntnTraceCfgReq* msg)
{
    if (msg->bridgeArpTraceCfg.choice > IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN) {
        return PS_FALSE;
    }
    return PS_TRUE;
}

/* TCP/IP协议栈报文钩包配置参数检查 */
STATIC PS_BoolUint8 IPSMNTN_TraceCfgChkParam(const IPS_MntnTraceCfgReq* msg)
{
    if ((msg->preRoutingTraceCfg.choice > IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN) ||
        (msg->postRoutingTraceCfg.choice > IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN) ||
        (msg->localTraceCfg.choice > IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN) ||
        (msg->bridgeArpTraceCfg.choice > IPS_MNTN_TRACE_WHOLE_DATA_LEN_CHOSEN)) {
        return PS_FALSE;
    }

    if (msg->adsIpConfig > WTTF_TRACE_MSG_YES) {
        return PS_FALSE;
    }
    return PS_TRUE;
}

/* 钩包配置参数检查 */
STATIC PS_BoolUint8 IPSMNTN_TraceAdvancedCfgChkParam(const IPS_MntnTraceCfgReq* msg)
{
    /* TCP/IP协议栈报文钩包配置参数检查和网桥中转报文钩包配置参数检查 */
    if ((IPSMNTN_TraceCfgChkParam(msg) == PS_FALSE) || (IPSMNTN_BridgeTraceCfgChkParam(msg) == PS_FALSE)) {
        return PS_FALSE;
    }

    return PS_TRUE;
}

STATIC VOS_VOID IPSMNTN_TraceCfgNotify(WTTF_TraceMsgSimpleAttribUint8 adsIpConfig)
{
    VOS_BOOL traceEnable = (adsIpConfig == WTTF_TRACE_MSG_YES) ? VOS_TRUE : VOS_FALSE;

    if (g_treceCfgNotifyCb != VOS_NULL_PTR) {
        g_treceCfgNotifyCb(traceEnable);
    }
}

/* 保存钩包配置 */
STATIC VOS_VOID IPSMNTN_TraceAdvancedCfgReq(const MsgBlock* msg)
{
    PS_BoolUint8                     result         = PS_FALSE;
    const OM_IpsAdvancedTraceCfgReq* cfgReq         = (OM_IpsAdvancedTraceCfgReq*)msg;
    IPS_OmAdvancedTraceCfgCnf        ipsTraceCfgCnf = {{0}};

    /* 检测配置参数是否合法 */
    result = IPSMNTN_TraceAdvancedCfgChkParam(&(cfgReq->ipsAdvanceCfgReq));

    /* Fill DIAG trans msg header */
    TTF_SET_MSG_SENDER_ID(&(ipsTraceCfgCnf.diagHdr), ACPU_PID_NFEXT);
    TTF_SET_MSG_RECEIVER_ID(&(ipsTraceCfgCnf.diagHdr), MSP_PID_DIAG_APP_AGENT); /* 把应答消息发送给DIAG，由DIAG把透传命令的处理结果发送给HIDS工具 */
    TTF_SET_MSG_LEN(&(ipsTraceCfgCnf.diagHdr), sizeof(IPS_OmAdvancedTraceCfgCnf) - VOS_MSG_HEAD_LENGTH);
    ipsTraceCfgCnf.diagHdr.msgId = ID_IPS_OM_ADVANCED_TRACE_CONFIG_CNF;

    /* DIAG透传命令中的特定信息 */
    ipsTraceCfgCnf.diagHdr.originalId = cfgReq->diagHdr.originalId;
    ipsTraceCfgCnf.diagHdr.terminalId = cfgReq->diagHdr.terminalId;
    ipsTraceCfgCnf.diagHdr.timeStamp  = cfgReq->diagHdr.timeStamp;
    ipsTraceCfgCnf.diagHdr.sn         = cfgReq->diagHdr.sn;

    if (result == PS_FALSE) {
        /* 如果参数检测不合法，向OM回复配置失败 */
        ipsTraceCfgCnf.ipsAdvanceCfgCnf.result = PS_FAIL;
        IPSMNTN_SndCfgCnf2Om(sizeof(IPS_OmAdvancedTraceCfgCnf), &ipsTraceCfgCnf);
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_ERROR, "IPSMNTN_TraceAdvancedCfgReq check fail!\n");
        return;
    }

#if (FEATURE_NFEXT == FEATURE_ON)
    if (NFEXT_ConfigEffective(&(cfgReq->ipsAdvanceCfgReq)) == PS_FALSE) {
        /* 如果注册钩子函数失败，向OM回复配置失败 */
        ipsTraceCfgCnf.ipsAdvanceCfgCnf.result = PS_FAIL;
        IPSMNTN_SndCfgCnf2Om(sizeof(IPS_OmAdvancedTraceCfgCnf), &ipsTraceCfgCnf);
        TTF_LOG(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_ERROR, "IPS_MNTN_SetCallbackFunc fail!\n");
        return;
    }

    IPSMNTN_SetIpsTraceMsgCfg(&(cfgReq->ipsAdvanceCfgReq));
#endif

    IPSMNTN_TraceCfgNotify(cfgReq->ipsAdvanceCfgReq.adsIpConfig);

    /* 向OM回复配置成功 */
    ipsTraceCfgCnf.ipsAdvanceCfgCnf.result = PS_SUCC;

    IPSMNTN_SndCfgCnf2Om(sizeof(IPS_OmAdvancedTraceCfgCnf), &ipsTraceCfgCnf);
}

VOS_VOID IPSMNTN_RegTraceCfgNotify(IPS_MntnTraceCfgNotifyFunc notifyFunc)
{
    g_treceCfgNotifyCb = notifyFunc;
}

/* NFExt接收到来自OM模块的消息处理 */
STATIC VOS_VOID NFEXT_RcvOmMsg(const MsgBlock* msg)
{
    VOS_UINT16 msgId;

    if (msg == VOS_NULL_PTR) {
        return;
    }

    msgId = (VOS_UINT16)(*((VOS_UINT32*)((VOS_UINT8*)(msg) + VOS_MSG_HEAD_LENGTH)));

    switch (msgId) {
        case ID_OM_IPS_ADVANCED_TRACE_CONFIG_REQ:
            IPSMNTN_TraceAdvancedCfgReq(msg);
            break;
#if (FEATURE_NFEXT == FEATURE_ON)
        case ID_OM_IPS_MNTN_INFO_CONFIG_REQ:
            NFEXT_RcvNfExtInfoCfgReq(msg);
            break;
        case ID_OM_IPS_MNTN_TRAFFIC_CTRL_REQ:
            IPSMNTN_TrafficCtrlReq(msg);
            break;
#endif
        default:
            TTF_LOG1(ACPU_PID_NFEXT, DIAG_MODE_COMM, PS_PRINT_WARNING, "NFExt_RcvConfig:Receive Unkown Type Message!\n",
                msgId);
            break;
    }
}

/* NFExt可维可测控制消息处理函数 */
VOS_VOID NFEXT_MsgProc(MsgBlock* msg)
{
    if (msg == VOS_NULL_PTR) {
        TTF_PRINT_WARNING("NFEXT_MsgProc: Message is NULL!\n");
        return;
    }

    switch (TTF_GET_MSG_SENDER_ID(msg)) {
        case MSP_PID_DIAG_APP_AGENT: /* 来自OM的透传消息处理 */
            NFEXT_RcvOmMsg(msg);
            break;
        default:
            break;
    }
}

/* NFExt_Pid初始化 */
STATIC VOS_UINT32 NFEXT_PidInit(enum VOS_InitPhaseDefine ip)
{
    if (ip == VOS_IP_LOAD_CONFIG) {
#if (FEATURE_NFEXT == FEATURE_ON)
        if (NFEXT_CreateRingBuffer() != VOS_OK) {
            return VOS_ERR;
        }
#endif
    }

    return VOS_OK;
}

/* NFExt_Fid初始化 */
VOS_UINT32 NFEXT_FidInit(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 result = VOS_ERR;

    if (ip == VOS_IP_LOAD_CONFIG) {
#if (FEATURE_NFEXT == FEATURE_ON)
        /* 先完成模块初始化 */
        if (NFEXT_Init() != 0) {
            TTF_PRINT_ERR("[init]<NFEXT_FidInit>: NFEXT_Init FAIL!\n");
            return VOS_ERR;
        }
#endif

        /* 可维可测模块注册PID */
        result = VOS_RegisterPIDInfo(ACPU_PID_NFEXT, (InitFunType)NFEXT_PidInit, (MsgFunType)NFEXT_MsgProc);
        if (result != VOS_OK) {
            TTF_PRINT_ERR("[init]<NFEXT_FidInit>: VOS_RegisterPIDInfo FAIL!\n");
            return VOS_ERR;
        }

#if (FEATURE_NFEXT == FEATURE_ON)
        result = VOS_RegisterMsgTaskEntry(ACPU_FID_NFEXT, NFEXT_FidTask);
        if (result != VOS_OK) {
            TTF_PRINT_ERR("[init]<NFEXT_FidInit>: VOS_RegisterMsgTaskEntry fail!\n");
            return VOS_ERR;
        }
#endif
        result = VOS_RegisterMsgTaskPrio(ACPU_FID_NFEXT, VOS_PRIORITY_M4);
        if (result != VOS_OK) {
            TTF_PRINT_ERR("[init]<NFEXT_FidInit>: VOS_RegisterTaskPrio Failed!\n");
            return VOS_ERR;
        }
    }

    return VOS_OK;
}
