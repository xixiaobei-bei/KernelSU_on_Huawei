/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "at_ltev_msg_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_data_proc.h"
#include "at_ctx.h"

#include "at_ltev_as_rslt_proc.h"
#include "at_ltev_ttf_rslt_proc.h"
#include "at_ltev_event_report.h"


#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_MSG_PROC_C


#if (FEATURE_LTEV == FEATURE_ON)

/* AT模块处理来自VRRC消息函数对应表 */
const AT_VRRC_ProcMsgTbl g_atProcMsgFromVrrcTab[] = {
    /* 消息ID */ /* 消息处理函数 */
    { VRRC_AT_GNSS_TEST_START_CNF, 0, AT_RcvGnssTestStartCnf },
    { VRRC_AT_GNSS_INFO_QRY_CNF, 0, AT_RcvGnssInfoQryCnf },
    { VRRC_AT_RSU_VPHYSTAT_QRY_CNF, 0, AT_RcvRsuVphyStatQryCnf },
    { VRRC_AT_RSU_VPHYSTAT_CLR_SET_CNF, 0, AT_RcvRsuVphyStatClrSetCnf },
    { VRRC_AT_RSU_VSNRRSRP_QRY_CNF, 0, AT_RcvRsuVsnrRsrpQryCnf },
    { VRRC_AT_GNSS_INFO_GET_CNF, 0, AT_RcvGnssDebugInfoGetCnf },
    { VRRC_AT_RSU_RSSI_QRY_CNF, 0, AT_RcvRsuV2xRssiQryCnf },
    { VRRC_AT_SYNC_SOURCE_QRY_CNF, 0, AT_RcvSyncSourceQryCnf },
    { VRRC_AT_SYNC_SOURCE_RPT_SET_CNF, 0, AT_RcvSyncSourceRptSetCnf },
    { VRRC_AT_SYNC_SOURCE_RPT_QRY_CNF, 0, AT_RcvSyncSourceRptQryCnf },
    { VRRC_AT_SYNC_SOURCE_RPT_IND, 0, AT_RcvSyncSourceRptInd },
    { VRRC_AT_SYNC_MODE_QRY_CNF, 0, AT_RcvSyncModeQryCnf },
    { VRRC_AT_SYNC_MODE_SET_CNF, 0, AT_RcvSyncModeSetCnf },
    { VRRC_AT_SET_TX_POWER_CNF, 0, AT_RcvVrrcMsgSetTxPowerCnfProc },
};

/* AT 模块处理来自Vmac消息函数对应表 */
const AT_VMAC_ProcMsgTbl g_atProcMsgFromVmacTab[] = {
    /* 消息ID */ /* 消息处理函数 */
    { VMAC_MSG_RSU_VRSSI_QRY_CNF, 0, AT_RcvVrssiQryCnf },
    { VMAC_AT_RSSI_RPT_IND, 0, AT_RcvRssiRptInd },
    { VMAC_AT_CBR_QRY_CNF, 0, AT_RcvVmacCbrQryCnf },
    { VMAC_AT_CBR_RPT_IND, 0, AT_RcvCbrRptInd },
};

/* AT处理来自VTC消息函数对应表 */
const AT_VTC_ProcMsgTbl g_atProcMsgFromVtcTab[] = {
    { VTC_AT_TEST_MODE_ACTIVE_STATE_SET_CNF, AT_RcvTestModeActiveStateSetCnf },
    { VTC_AT_TEST_MODE_ACTIVE_STATE_QRY_CNF, AT_RcvTestModeActiveStateQryCnf },
    { VTC_AT_TEST_LOOP_MODE_E_CLOSE_STATE_SET_CNF, AT_RcvTestModeECloseStateSetCnf },
    { VTC_AT_TEST_LOOP_MODE_E_CLOSE_STATE_QRY_CNF, AT_RcvTestModeECloseStateQryCnf },
    { VTC_AT_SIDELINK_PACKET_COUNTER_QRY_CNF, AT_RcvSidelinkPacketCounterQryCnf },
    { VTC_AT_CBR_QRY_CNF, AT_RcvVtcCbrQryCnf },
    { VTC_AT_RESET_UTC_TIME_SET_CNF, AT_RcvResetUtcTimeSetCnf },
    { VTC_AT_SENDING_DATA_ACTION_SET_CNF, AT_RcvSendingDataActionSetCnf },
    { VTC_AT_SENDING_DATA_ACTION_QRY_CNF, AT_RcvSendingDataActionQryCnf },
    { VTC_AT_PC5_SEND_DATA_CNF, AT_RcvPc5PktSndCnf },
    { VTC_AT_LAYER_TWO_ID_SET_CNF, AT_RcvTestModeModifyL2IdSetCnf },
    { VTC_AT_LAYER_TWO_ID_QRY_CNF, AT_RcvTestModeModifyL2IdQryCnf },
};

VOS_VOID AT_ProcMsgFromVrrc(struct MsgCB *msg)
{
    MN_AT_IndEvt *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32    i;
    VOS_UINT32    msgCount;
    VOS_UINT16    msgName;
    VOS_UINT32    result;

    msgCount = AT_ARRAY_SIZE(g_atProcMsgFromVrrcTab);
    rcvMsg   = (MN_AT_IndEvt *)msg;
    msgName  = rcvMsg->msgName;

    for (i = 0; i < msgCount; i++) {
        if (g_atProcMsgFromVrrcTab[i].msgName == msgName) {
            result = g_atProcMsgFromVrrcTab[i].procMsgFunc(rcvMsg);
            if (result == VOS_ERR) {
                AT_ERR_LOG("AT_ProcMsgFromVrrc: Msg Proc Err!");
            }
            return;
        }
    }
    if (msgCount == i) {
        AT_ERR_LOG("AT_ProcMsgFromVrrc: Msg Id is invalid!");
    }
}

VOS_VOID AT_ProcMsgFromVmac(struct MsgCB *msg)
{
    MN_AT_IndEvt *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32    i;
    VOS_UINT32    msgCount;
    VOS_UINT16    msgName;
    VOS_UINT32    result;

    msgCount = AT_ARRAY_SIZE(g_atProcMsgFromVmacTab);
    rcvMsg   = (MN_AT_IndEvt *)msg;
    msgName  = rcvMsg->msgName;

    for (i = 0; i < msgCount; i++) {
        if (g_atProcMsgFromVmacTab[i].msgName == msgName) {
            result = g_atProcMsgFromVmacTab[i].procMsgFunc(rcvMsg);
            if (result == VOS_ERR) {
                AT_ERR_LOG("AT_ProcMsgFromVmac: Msg Proc Err!");
            }
            return;
        }
    }
    if (msgCount == i) {
        AT_ERR_LOG("AT_ProcMsgFromVmac: Msg Id is invalid!");
    }
}

VOS_VOID AT_ProcMsgFromVtc(struct MsgCB *msg)
{
    VTC_AT_MsgCnf       *rcvMsg = VOS_NULL_PTR;
    VOS_UINT32           i;
    VOS_UINT32           msgCount;
    AT_VTC_MsgTypeUint32 msgName;
    VOS_UINT32           result;

    msgCount = AT_ARRAY_SIZE(g_atProcMsgFromVtcTab);
    rcvMsg   = (VTC_AT_MsgCnf *)msg;
    msgName  = rcvMsg->msgName;

    for (i = 0; i < msgCount; i++) {
        if (g_atProcMsgFromVtcTab[i].msgName == msgName) {
            result = g_atProcMsgFromVtcTab[i].procMsgFunc(rcvMsg);
            if (result == VOS_ERR) {
                AT_ERR_LOG("AT_ProcMsgFromVtc: Msg Proc Err!");
            }
            return;
        }
    }
    if (msgCount == i) {
        AT_ERR_LOG("AT_ProcMsgFromVtc: Msg Id is invalid!");
    }
}

#endif

