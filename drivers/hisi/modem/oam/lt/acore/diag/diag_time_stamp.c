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

/*
 * 1 Include HeadFile
 */
#include "diag_time_stamp.h"
#include <msp.h>
#include <mdrv.h>
#include "diag_common.h"
#include "diag_cfg.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_debug.h"
#include "soc_socp_adapter.h"

#define THIS_FILE_ID MSP_FILE_ID_DIAG_TS_C

HTIMER g_HighTs;
DIAG_CMD_LOG_TIMESTAMP_REQ_STRU g_HighTsCtrl = { DIAG_CFG_SWT_CLOSE, DIAG_HIGH_TS_PUSH_TIMER_LEN };

/* DIAG_CMD_HIGH_TIMESTAPM_IND 推送时间戳高32位给工具 */
VOS_VOID diag_PushHighTs(VOS_VOID)
{
    DIAG_TIME_STAMP_T stTimeStamp;
    VOS_UINT32 pulHigh32bitValue;
    VOS_UINT32 pulLow32bitValue;
    VOS_INT32 ret;

    ret = mdrv_timer_get_accuracy_timestamp(&pulHigh32bitValue, &pulLow32bitValue);
    if (ret) {
        diag_error("get time stamp fail(0x%x)\n", ret);
        return;
    }

    stTimeStamp.stHighTs.ulLowTimeStamp = pulLow32bitValue;
    stTimeStamp.stHighTs.ulHighTimeStamp = pulHigh32bitValue;

    stTimeStamp.stTransInfo.ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_LTE, DIAG_MSG_TYPE_MSP);
    stTimeStamp.stTransInfo.ulPid = MSP_PID_DIAG_APP_AGENT;
    stTimeStamp.stTransInfo.ulReserve = 0;
    stTimeStamp.stTransInfo.ulMsgId = DIAG_CMD_HIGH_TIMESTAPM_IND;
    stTimeStamp.stTransInfo.ulLength = sizeof(stTimeStamp.stHighTs);

    stTimeStamp.stTransInfo.pData = (void *)(&stTimeStamp.stHighTs);

    ret = mdrv_diag_report_msg_trans((diag_trans_ind_s *)&stTimeStamp, DIAG_CMD_HIGH_TIMESTAPM_IND);
    if (ret) {
        /* 此处不能加打印，因为开机log情况下有可能上报不成功 */
    }

    return;
}

VOS_VOID diag_PowerOnLogHighTsTimer(VOS_VOID)
{
    VOS_UINT32 ret;
    VOS_UINT32 time;

    time = g_HighTsCtrl.ulPeriod * 0xEA60;

    /* 启动定时器定时上报时间戳高位给工具 */
    ret = VOS_StartRelTimer(&g_HighTs, MSP_PID_DIAG_APP_AGENT, time, DIAG_HIGH_TS_PUSH_TIMER_NAME,
        DIAG_HIGH_TS_PUSH_TIMER_PARA, VOS_RELTIMER_NOLOOP, VOS_TIMER_PRECISION_0);
    if (ret != ERR_MSP_SUCCESS) {
        diag_error("VOS_StartRelTimer fail\n");
        return;
    }
}

VOS_VOID diag_StartHighTsTimer(VOS_VOID)
{
    VOS_UINT32 ret;
    VOS_UINT32 time;

    if (g_HighTsCtrl.ulSwitch == DIAG_CFG_SWT_OPEN) {
        diag_crit("time stamp timer is actived\n");
        return;
    }

    g_HighTsCtrl.ulSwitch = DIAG_CFG_SWT_OPEN;

    time = g_HighTsCtrl.ulPeriod * 0xEA60;

    /* 启动定时器定时上报时间戳高位给工具 */
    ret = VOS_StartRelTimer(&g_HighTs, MSP_PID_DIAG_APP_AGENT, time, DIAG_HIGH_TS_PUSH_TIMER_NAME,
        DIAG_HIGH_TS_PUSH_TIMER_PARA, VOS_RELTIMER_LOOP, VOS_TIMER_PRECISION_5);
    if (ret != ERR_MSP_SUCCESS) {
        diag_error("VOS_StartRelTimer fail\n");
        return;
    }
}

VOS_VOID diag_StopHighTsTimer(VOS_VOID)
{
    /* 删除定时器 */
    if (g_HighTsCtrl.ulSwitch == DIAG_CFG_SWT_CLOSE) {
        diag_crit("high timestamp timer is deactive\n");
        return;
    }

    g_HighTsCtrl.ulSwitch = DIAG_CFG_SWT_CLOSE;

    (VOS_VOID)VOS_StopRelTimer(&g_HighTs);

    return;
}

/*
 * Function Name: diag_TransCfgProc
 * Description: deal witch trans msg cfg
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_HighTsCfgProc(VOS_UINT8 *pstReq)
{
    DIAG_CMD_LOG_CAT_TIMESTAMP_CNF_STRU stTimeCnf = { 0 };
    DIAG_CMD_LOG_TIMESTAMP_REQ_STRU *pstTimeReq = NULL;
    msp_diag_frame_info_s *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    VOS_UINT32 ret;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    mdrv_diag_ptr(EN_DIAG_PTR_CFG_HIGHTS, 1, pstDiagHead->ulCmdId, 0);

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_LOG_TIMESTAMP_REQ_STRU)) {
        diag_error("len error:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INALID_LEN_ERROR;
        goto DIAG_ERROR;
    }

    pstTimeReq = (DIAG_CMD_LOG_TIMESTAMP_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    g_HighTsCtrl.ulPeriod = pstTimeReq->ulPeriod;

    if (pstTimeReq->ulSwitch == DIAG_CFG_SWT_CLOSE) {
        diag_crit("switch to close\n");
        diag_StopHighTsTimer();
    } else {
        diag_crit("switch to open\n");
        diag_StartHighTsTimer();
    }

    ret = ERR_MSP_SUCCESS;

DIAG_ERROR:
    DIAG_MSG_COMMON_PROC(stDiagInfo, stTimeCnf, pstDiagHead);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stTimeCnf.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stTimeCnf, (VOS_UINT32)sizeof(stTimeCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_CFG_HIGHTS_CNF, 1, pstDiagHead->ulCmdId, ret);
    return (VOS_UINT32)ret;
}
