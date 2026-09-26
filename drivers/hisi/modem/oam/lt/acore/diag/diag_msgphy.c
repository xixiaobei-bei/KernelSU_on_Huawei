/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

#include <securec.h>
#include "diag_msgphy.h"
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include "diag_msgmsp.h"
#include "diag_msgbbp.h"
#include "diag_debug.h"
#include "diag_common.h"
#include "diag_cfg.h"
#include "soc_socp_adapter.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_MSGDSP_C

VOS_UINT32 g_PhyPowerOnState = DIAG_PHY_POWER_ON_DOING;

VOS_UINT32 diag_GuDspTransProc(msp_diag_frame_info_s *pData)
{
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    DIAG_COMM_CNF_STRU stRttCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    diag_osa_msg_head_s *pstMsg = NULL;
    diag_osa_msg_head_s *pstVosMsg = NULL;
    errno_t err;

    if (pData->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(diag_osa_msg_head_s)) {
        diag_error("DataLen(0x%x) error\n", pData->ulMsgLen);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    pstMsg = (diag_osa_msg_head_s *)(pData->aucData + sizeof(msp_diag_data_head_s));

    if (VOS_CheckPidValidity(pstMsg->ulReceiverPid) != VOS_PID_AVAILABLE) {
        diag_error("pid:0x%x is invalid,msgid:0x%x\n", pstMsg->ulReceiverPid, pData->ulCmdId);
        ulRet = ERR_MSP_DIAG_ERRPID_CMD;
        goto DIAG_ERROR;
    }

    if (pstMsg->ulLength > DIAG_MEM_ALLOC_LEN_MAX) {
        diag_error("DataLen(0x%x) error,msgid:0x%x\n", pstMsg->ulLength, pData->ulCmdId);
        ulRet = ERR_MSP_INALID_LEN_ERROR;
        goto DIAG_ERROR;
    }

    pstVosMsg = (diag_osa_msg_head_s *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, pstMsg->ulLength);
    if (pstVosMsg == VOS_NULL) {
        diag_error("VOS_AllocMsg failed!\n");
        ulRet = ERR_MSP_MALLOC_FAILUE;
        goto DIAG_ERROR;
    }

    pstVosMsg->ulReceiverPid = pstMsg->ulReceiverPid;

    err = memcpy_s(&pstVosMsg->ulMsgId, pstMsg->ulLength, &pstMsg->ulMsgId, pstMsg->ulLength);
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstVosMsg);
    if (ulRet != VOS_OK) {
        ulRet = ERR_MSP_DIAG_SEND_MSG_FAIL;
        diag_error("VOS_SendMsg failed!\n");
    }

DIAG_ERROR:
    stRttCnf.ulRet = ulRet;
    DIAG_MSG_COMMON_PROC(stDiagInfo, stRttCnf, pData);

    stDiagInfo.ulMsgType = pData->stID.pri4b;

    /* 组包回复 */
    ulRet = DIAG_MsgReport(&stDiagInfo, &stRttCnf, sizeof(stRttCnf));

    return ulRet;
}

/*
 * Function Name: diag_DspMsgProc
 * Description: DSP处理消息处理包括连接断开
 * Input: None
 * Output: None
 * Return: None
 * History:
 */
VOS_UINT32 diag_DspMsgProc(msp_diag_frame_info_s *pData)
{
    VOS_UINT32 ulRet = ERR_MSP_SUCCESS;
    DIAG_COMM_CNF_STRU stRttCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    VOS_UINT32 ulLen;
    DIAG_PHY_MSG_A_TRANS_C_STRU *pstInfo = VOS_NULL;
    errno_t err;

    switch (pData->stID.mode4b) {
        /* GUDSP的命令在A核直接透传给GUDSP处理 */
        case DIAG_MODE_GSM:
        case DIAG_MODE_UMTS:
        case DIAG_MODE_1X:
        case DIAG_MODE_HRPD:
            return diag_GuDspTransProc(pData);

        default:
            break;
    }

    ulLen = sizeof(DIAG_PHY_MSG_A_TRANS_C_STRU) - VOS_MSG_HEAD_LENGTH + pData->ulMsgLen;
    if (ulLen > DIAG_MEM_ALLOC_LEN_MAX) {
        diag_error("Msg len valid,MsgId=0x%x, MsgLen=0x%x\n", pData->ulCmdId, pData->ulMsgLen);
        ulRet = ERR_MSP_NOT_ENOUGH_MEMORY;
        goto DIAG_ERROR;
    }

    pstInfo = (DIAG_PHY_MSG_A_TRANS_C_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, ulLen);
    if (pstInfo == VOS_NULL) {
        diag_error("VOS_AllocMsg failed!\n");
        ulRet = ERR_MSP_MALLOC_FAILUE;
        goto DIAG_ERROR;
    }

    pstInfo->ulSenderPid = MSP_PID_DIAG_APP_AGENT;

    if ((pData->stID.mode4b == DIAG_MODE_NR) || (pData->stID.mode4b == DIAG_MODE_COMM)) {
        pstInfo->ulReceiverPid = MSP_PID_DIAG_NRM_AGENT;
        pstInfo->ulMsgId = DIAG_MSG_HL1C_A_TRANS_NRM_REQ;
    } else {
        pstInfo->ulReceiverPid = MSP_PID_DIAG_AGENT;
        pstInfo->ulMsgId = DIAG_MSG_PHY_A_TRANS_C_REQ;
    }

    ulLen = sizeof(msp_diag_frame_info_s) + pData->ulMsgLen;
    err = memcpy_s(&pstInfo->stInfo, ulLen, pData, ulLen);
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }
    ulRet = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstInfo);
    if (ulRet) {
        diag_error("VOS_SendMsg failed!\n");
        ulRet = ERR_MSP_DIAG_SEND_MSG_FAIL;
        goto DIAG_ERROR;
    }

    return ulRet;
DIAG_ERROR:

    stRttCnf.ulRet = ulRet;
    DIAG_MSG_COMMON_PROC(stDiagInfo, stRttCnf, pData);

    stDiagInfo.ulMsgType = pData->stID.pri4b;
    stDiagInfo.ulMode = pData->stID.mode4b;

    /* 组包回复 */
    ulRet = DIAG_MsgReport(&stDiagInfo, &stRttCnf, sizeof(stRttCnf));
    return ulRet;
}

/*
 * Function Name: diag_DspMsgInit
 * Description: MSP dsp部分初始化
 * Input: None
 * Output: None
 * Return: None
 * History:
 */
VOS_VOID diag_DspMsgInit(VOS_VOID)
{
    /* 注册message消息回调 */
    DIAG_MsgProcReg(DIAG_MSG_TYPE_PHY, diag_DspMsgProc);
}

VOS_VOID diag_GuPhyMsgProc(MsgBlock *pMsgBlock)
{
    mdrv_diag_phy_state_msg_s *phy_msg;
    u32 phy_state = DIAG_PHY_POWER_ON_DOING;

    phy_msg = (mdrv_diag_phy_state_msg_s *)pMsgBlock;

    if (phy_msg->msg_id == ID_MSG_DIAG_CMD_PHY_READY_REQ) {
        phy_state = phy_msg->phy_state;
        diag_SetPhyPowerOnState(phy_state);
        diag_crit("set phy state 0x%x ok\n", g_PhyPowerOnState);
    }

    return;
}

VOS_VOID diag_SetPhyPowerOnState(VOS_UINT32 phy_state)
{
    g_PhyPowerOnState = phy_state;
    return;
}

VOS_UINT32 diag_GetPhyPowerOnState(VOS_VOID)
{
    return g_PhyPowerOnState;
}

