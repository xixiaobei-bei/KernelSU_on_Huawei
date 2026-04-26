/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#include "at_mt_msg_proc.h"
#include "securec.h"
#include "at_type_def.h"
#include "mn_client.h"
#include "at_cmd_proc.h"
#include "mn_comm_api.h"
#include "at_phy_interface.h"
#include "taf_phy_pid_def.h"

#include "at_device_as_rslt_proc.h"
#include "at_device_pam_rslt_proc.h"
#include "at_device_phy_rslt_proc.h"

/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_MT_MSG_PROC_C

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
#if (FEATURE_LTEV == FEATURE_ON)
#define AT_COMPONENTTYPE_RAT_LTEV 0x7
#endif
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
static const AT_PROC_BbicMsg g_atProcBbicMsgTab[] = {
    { ID_BBIC_CAL_AT_MT_TX_CNF, AT_ProcTxCnf },
    { ID_BBIC_CAL_AT_MT_RX_CNF, AT_ProcRxCnf },
    { ID_BBIC_AT_CAL_MSG_CNF, AT_ProcGsmTxCnf_ModulateWave },
    { ID_BBIC_AT_GSM_TX_PD_CNF, AT_ProcPowerDetCnf_ModulateWave },
    { ID_BBIC_CAL_AT_MT_TX_PD_IND, AT_ProcPowerDetCnf },
    { ID_BBIC_AT_MIPI_READ_CNF, AT_RcvBbicCalMipiRedCnf },
    { ID_BBIC_AT_MIPI_WRITE_CNF, AT_RcvBbicCalMipiWriteCnf },
    { ID_BBIC_AT_PLL_QRY_CNF, At_RcvBbicPllStatusCnf },
    { ID_BBIC_CAL_AT_MT_RX_RSSI_IND, At_RcvBbicRssiInd },
    { ID_BBIC_AT_DPDT_CNF, AT_RcvBbicCalSetDpdtCnf },
    { ID_BBIC_AT_TEMP_QRY_CNF, AT_RcvBbicCalQryFtemprptCnf },
    { ID_BBIC_AT_DCXO_CNF, At_RcvBbicCalDcxoCnf },
    { ID_BBIC_AT_TRX_TAS_CNF, AT_RcvBbicCalSetTrxTasCnf }
};

static const AT_PROC_CbtMsg g_atProcCbtMsgTab[] = {
    { ID_CCBT_AT_SET_WORK_MODE_CNF, AT_ProcSetWorkModeCnf },
};

static const AT_PROC_UecbtMsg g_atProcUeCbtMsgTab[] = {
    { ID_UECBT_AT_RFIC_MEM_TEST_CNF, At_ProcUeCbtRfIcMemTestCnf },
    { ID_UECBT_AT_RFIC_DIE_IE_QUERY_CNF, At_RcvUeCbtRfIcIdExQryCnf },

};

static const AT_PROC_DspIdleMsg g_atProcDspIdleMsgTab[] = {
    { ID_FESS_FLPM_NAS_SERDES_AGING_CNF, At_ProcFessFlpmNasSerdesAgingCnf },
    { ID_FESS_FLPM_NAS_AT_NOBIZ_CNF, At_ProcFessFlpmNasSerdesNobizCnf },
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { ID_FESS_FLPM_NAS_SERDES_SLT_CNF, AT_ProcFessFlpmNasSerdesSltCnf },
#endif
};

#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_VOID At_RfRssiIndProc(HPA_AT_RfRxRssiInd *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT8          indexNum;
    VOS_UINT16         length;
    VOS_INT32          rssi;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (msg->agcGain == AT_DSP_RF_AGC_STATE_ERROR) { /* 错误 */
        AT_WARN_LOG("AT_RfRssiIndProc err");
        rslt = AT_FRSSI_OTHER_ERR;
    } else {
        g_atSendDataBuff.bufLen = 0;

        /* 由于RSSI测量值单位0.125dBm，为了消除浮点数*1000. */
        rssi = (VOS_INT32)msg->rssi * AT_DSP_RSSI_VALUE_MUL_THOUSAND;

        /*
         * 读取的RSSI值，采用正值上报，精确到0.1dBm定制值信息。如果当前的RSSI
         * 值为-85.1dBm，返回值为851. 由于之前乘1000，所以精确到0.1dBm这里要除100
         */
        if (rssi < 0) {
            rssi = (-1 * rssi) / 100;
        } else {
            rssi = rssi / 100;
        }

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, rssi);

        g_atSendDataBuff.bufLen = length;
        rslt                    = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rslt);
}

VOS_VOID At_WTxCltIndProc(WPHY_AT_TxCltInd *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    /* 初始化全局变量 */
    atDevCmdCtrl = AT_GetDevCmdCtrl();
    (VOS_VOID)memset_s(&atDevCmdCtrl->cltInfo, sizeof(atDevCmdCtrl->cltInfo), 0x0, sizeof(AT_TX_CltInfo));

    /* 设置CLT信息有效标志 */
    atDevCmdCtrl->cltInfo.infoAvailableFlg = VOS_TRUE;

    /* 将接入层上报的信息记录下全局变量中 */
    atDevCmdCtrl->cltInfo.gammaReal = msg->gammaReal; /* 反射系数实部 */
    atDevCmdCtrl->cltInfo.gammaImag = msg->gammaImag; /* 反射系数虚部 */
    /* 驻波检测场景0反射系数幅度 */
    atDevCmdCtrl->cltInfo.gammaAmpUc0 = msg->gammaAmpUc0;
    /* 驻波检测场景1反射系数幅度 */
    atDevCmdCtrl->cltInfo.gammaAmpUc1 = msg->gammaAmpUc1;
    /* 驻波检测场景2反射系数幅度 */
    atDevCmdCtrl->cltInfo.gammaAmpUc2        = msg->gammaAmpUc2;
    atDevCmdCtrl->cltInfo.gammaAntCoarseTune = msg->gammaAntCoarseTune; /* 粗调格点位置 */
    atDevCmdCtrl->cltInfo.ulwFomcoarseTune   = msg->fomcoarseTune;      /* 粗调FOM值 */
    atDevCmdCtrl->cltInfo.cltAlgState        = msg->cltAlgState;        /* 闭环算法收敛状态 */
    atDevCmdCtrl->cltInfo.cltDetectCount     = msg->cltDetectCount;     /* 闭环收敛总步数 */
    atDevCmdCtrl->cltInfo.dac0               = msg->dac0;               /* DAC0 */
    atDevCmdCtrl->cltInfo.dac1               = msg->dac1;               /* DAC1 */
    atDevCmdCtrl->cltInfo.dac2               = msg->dac2;               /* DAC2 */
    atDevCmdCtrl->cltInfo.dac3               = msg->dac3;               /* DAC3 */
}
#else
VOS_VOID AT_MT_ClearMsgEntity(AT_MT_MsgHeader *msg)
{
    (VOS_VOID)memset_s(AT_MT_GET_MSG_ENTITY(msg), AT_MT_GET_MSG_LENGTH(msg), 0x00, AT_MT_GET_MSG_LENGTH(msg));
}

VOS_VOID AT_ProcCbtMsg(struct MsgCB *msg)
{
    VOS_UINT32 i;
    VOS_UINT32 msgCnt;
    VOS_UINT32 rst;
    VOS_UINT16 msgId;

    msgCnt = sizeof(g_atProcCbtMsgTab) / sizeof(AT_PROC_CbtMsg);

    msgId = ((AT_MT_MsgHeader *)msg)->usMsgId;

    /* g_atProcCbtMsgTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcCbtMsgTab[i].msgType == msgId) {
            rst = g_atProcCbtMsgTab[i].procMsgFunc(msg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcCbtMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcCbtMsg: Msg Id is invalid!");
    }
}

VOS_VOID AT_ProcUeCbtMsg(struct MsgCB *msg)
{
    VOS_UINT32 i;
    VOS_UINT32 msgCnt;
    VOS_UINT32 rst;
    VOS_UINT16 msgId;

    msgCnt = sizeof(g_atProcUeCbtMsgTab) / sizeof(AT_PROC_UecbtMsg);

    msgId = ((AT_MT_MsgHeader *)msg)->usMsgId;

    for (i = 0; i < msgCnt; i++) {
        if (g_atProcUeCbtMsgTab[i].msgType == msgId) {
            rst = g_atProcUeCbtMsgTab[i].procMsgFunc(msg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcUeCbtMsg: Msg Proc Err!");
            }

            return;
        }
    }

    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcUeCbtMsg: Msg Id is invalid!");
    }
}

VOS_VOID AT_ProcDspIdleMsg(struct MsgCB *msg)
{
    VOS_UINT32 i;
    VOS_UINT32 msgCnt;
    VOS_UINT32 rst;
    VOS_UINT16 msgId;

    msgCnt = sizeof(g_atProcDspIdleMsgTab) / sizeof(AT_PROC_DspIdleMsg);

    msgId = ((AT_MT_MsgHeader *)msg)->usMsgId;

    for (i = 0; i < msgCnt; i++) {
        if (g_atProcDspIdleMsgTab[i].msgType == msgId) {
            rst = g_atProcDspIdleMsgTab[i].procMsgFunc(msg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcDspIdleMsg: Msg Proc Err!");
            }

            return;
        }
    }

    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcDspIdleMsg: Msg Id is invalid!");
    }
}

VOS_VOID AT_ProcBbicMsg(struct MsgCB *msg)
{
    VOS_UINT32 i;
    VOS_UINT32 msgCnt;
    VOS_UINT32 rst;
    VOS_UINT16 msgId;

    msgCnt = sizeof(g_atProcBbicMsgTab) / sizeof(AT_PROC_BbicMsg);

    msgId = ((AT_MT_MsgHeader *)msg)->usMsgId;

    /* g_atProcBbicMsgTab查表，进行消息分发 */
    for (i = 0; i < msgCnt; i++) {
        if (g_atProcBbicMsgTab[i].msgType == msgId) {
            rst = g_atProcBbicMsgTab[i].procMsgFunc(msg);

            if (rst == VOS_ERR) {
                AT_ERR_LOG("AT_ProcBbicMsg: Msg Proc Err!");
            }

            return;
        }
    }

    /* 没有找到匹配的消息 */
    if (msgCnt == i) {
        AT_ERR_LOG("AT_ProcBbicMsg: Msg Id is invalid!");
    }
}
#endif

