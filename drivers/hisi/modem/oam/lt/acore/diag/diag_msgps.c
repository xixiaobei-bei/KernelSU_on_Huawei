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
#include "diag_msgps.h"
#include <product_config.h>
#include <vos.h>
#include <mdrv.h>
#include <msp.h>
#include "diag_msgmsp.h"
#include "diag_debug.h"
#include "diag_acore_common.h"
#include "soc_socp_adapter.h"
#include "diag_connect.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_MSGPS_C

DIAG_TRANS_HEADER_STRU g_stPSTransHead = { { VOS_NULL, VOS_NULL }, 0 };
DIAG_MSGPS_CTRL g_DiagPsCtrl = {
    .ulChannelNum = 1,
#ifdef DIAG_SYSTEM_5G
    .ulChannelId = SOCP_CODER_SRC_ACPU_IND,
#else
    .ulChannelId = SOCP_CODER_SRC_LOM_IND1,
#endif
};

/*
 * Function Name: diag_PsTransProcEntry
 * Description: 该函数用于处理从PS透传命令
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_PsTransProcEntry(msp_diag_frame_info_s *pstReq)
{
    return diag_TransReqProcEntry(pstReq, &g_stPSTransHead);
}

VOS_UINT32 diag_PsConnMgr(VOS_UINT8 *pData)
{
    DIAG_CONN_MSG_SEND_T *pstRevMsg;

    pstRevMsg = (DIAG_CONN_MSG_SEND_T *)pData;

    if (pstRevMsg->ulMsgId == ID_MSG_DIAG_CMD_CONNECT_REQ) {
        return diag_PsConnect(pData);
    } else {
        return diag_PsDisconnect(pData);
    }
}

VOS_UINT32 diag_PsConnect(VOS_UINT8 *pData)
{
#ifdef DIAG_SYSTEM_5G
    mdrv_diag_cmd_reply_set_req_s stReplay = { 0 };
    mdrv_diag_connect_s stResult;
    DIAG_CONN_MSG_SEND_T *pstRevMsg;
    VOS_UINT32 ulCnfRst;
    VOS_UINT32 ret;

    pstRevMsg = (DIAG_CONN_MSG_SEND_T *)pData;

    stResult.ulChannelId = g_DiagPsCtrl.ulChannelId;

    /* if power on log */
    if (!DIAG_IS_POLOG_ON) {
        ret = mdrv_socp_stop(g_DiagPsCtrl.ulChannelId);
        if (ret) {
            diag_error("stop socp fail\n");
        }

        ret = mdrv_clear_socp_buff(g_DiagPsCtrl.ulChannelId);
        if (ret) {
            diag_error("clear socp src buff fail\n");
        }

        ret = mdrv_clear_socp_dst_buffer(SOCP_CODER_DST_OM_IND);
        if (ret) {
            diag_error("clear socp ind dst buff fail\n");
        }
    }

    /* 设置连接状态开关值 */
    ulCnfRst = diag_CfgSetGlobalBitValue(&g_ulDiagCfgInfo, DIAG_CFG_CONN_BIT, DIAG_CFG_SWT_OPEN);
    if (ulCnfRst) {
        stResult.ulResult = ulCnfRst;
        diag_error("Open DIAG_CFG_CONN_BIT failed.\n");
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_PS, pstRevMsg->ulSn, g_DiagPsCtrl.ulChannelNum, &stResult);
    }

    /* 使能socp编码源通道 */
    ret = mdrv_socp_start(g_DiagPsCtrl.ulChannelId);
    if (ret) {
        diag_error("start socp fail\n");
        stResult.ulResult = ERR_MSP_SCM_START_SOCP_FAIL;
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_PS, pstRevMsg->ulSn, g_DiagPsCtrl.ulChannelNum, &stResult);
    }

    ulCnfRst = diag_SendMsg(MSP_PID_DIAG_APP_AGENT, PS_PID_MM, ID_MSG_DIAG_CMD_REPLAY_TO_PS, (VOS_UINT8 *)&stReplay,
        (VOS_UINT32)sizeof(mdrv_diag_cmd_reply_set_req_s));
    if (ulCnfRst) {
        stResult.ulResult = ERR_MSP_DIAG_SEND_MSG_FAIL;
        return diag_ConnCnf(DIAG_CONN_ID_ACPU_PS, pstRevMsg->ulSn, g_DiagPsCtrl.ulChannelNum, &stResult);
    }

    /* 复位维测信息记录 */
    mdrv_diag_reset_mntn_info(DIAGLOG_SRC_MNTN);
    mdrv_diag_reset_mntn_info(DIAGLOG_DST_MNTN);

    /* 回复连接消息 */
    stResult.ulResult = ERR_MSP_SUCCESS;
    return diag_ConnCnf(DIAG_CONN_ID_ACPU_PS, pstRevMsg->ulSn, g_DiagPsCtrl.ulChannelNum, &stResult);
#else
    return ERR_MSP_SUCCESS;
#endif
}

VOS_VOID diag_ConnReset(VOS_VOID)
{
    /* 规避老的hids在建链时候下发disconnect命令，不能将开机log配置清除 */
    if (DIAG_IS_POLOG_ON) {
        g_ulDiagCfgInfo = DIAG_CFG_INIT | DIAG_CFG_POWERONLOG;
        diag_info("diag_ConnReset, keep init&poweronlog flag.\n");
    } else {
        g_ulDiagCfgInfo = DIAG_CFG_INIT;
    }

    return;
}

VOS_UINT32 diag_PsDisconnect(VOS_UINT8 *pData)
{
#ifdef DIAG_SYSTEM_5G
    mdrv_diag_connect_s stResult;
    DIAG_CONN_MSG_SEND_T *pstRevMsg;
    VOS_UINT32 ret;

    pstRevMsg = (DIAG_CONN_MSG_SEND_T *)pData;
    stResult.ulChannelId = g_DiagPsCtrl.ulChannelId;

    if (!DIAG_IS_POLOG_ON) {
        /* step1. 清空本地连接状态 */
        diag_ConnReset();

        /* step2. 清空上报配置,停止所有OAM消息上报 */
        diag_CfgResetAllSwt();

        /* step3. disable SOCP 编码输入通道 */
        ret = mdrv_socp_stop(g_DiagPsCtrl.ulChannelId);
        if (ret) {
            diag_error("start socp fail\n");
            stResult.ulResult = ERR_MSP_STOP_SOCP_FAIL;
            return diag_ConnCnf(DIAG_CONN_ID_ACPU_PS, pstRevMsg->ulSn, g_DiagPsCtrl.ulChannelNum, &stResult);
        }

        /* step4. 配置写指针为读指针，清空SOCP编码输入通道 */
        mdrv_clear_socp_buff(g_DiagPsCtrl.ulChannelId);

        /* step5. TRANS ID-Src Counter清零 */
        mdrv_diag_report_reset();

        /* 删除定时器 */
        diag_StopMntnTimer();
    }

    /* step6. 回复MSP DIAG DMS完成消息 */
    stResult.ulResult = ERR_MSP_SUCCESS;
    return diag_ConnCnf(DIAG_CONN_ID_ACPU_PS, pstRevMsg->ulSn, g_DiagPsCtrl.ulChannelNum, &stResult);
#else
    return ERR_MSP_SUCCESS;
#endif
}
/*
 * Function Name: diag_PsMsgInit
 * Description: MSP ps部分初始化
 */
VOS_VOID diag_PsMsgInit(VOS_VOID)
{
    VOS_UINT32 ulRet;

    /* 创建节点保护信号量 Diag Trans Ps */
    ulRet = VOS_SmBCreate("DTP", 1, VOS_SEMA4_FIFO, &g_stPSTransHead.TransSem);
    if (ulRet != VOS_OK) {
        diag_error("VOS_SmBCreate failed.\n");
    }

    /* 初始化请求链表 */
    blist_head_init(&g_stPSTransHead.TransHead);

    /* 注册message消息回调 */
    DIAG_MsgProcReg(DIAG_MSG_TYPE_PS, diag_PsTransProcEntry);

    ulRet = diag_ConnMgrSendFuncReg(DIAG_CONN_ID_ACPU_PS, g_DiagPsCtrl.ulChannelNum, &g_DiagPsCtrl.ulChannelId,
        diag_PsConnMgr);
    if (ulRet) {
        diag_error("acpu ps reg ConnMsg fail(0x%x)\n", ulRet);
    }
}

VOS_VOID DIAG_ShowTransList(VOS_VOID)
{
    struct list_head *me = NULL;

    /* 添加信号量保护 */
    (VOS_VOID)VOS_SmP(g_stPSTransHead.TransSem, 0);

    /* 在链表中查找每个子命令结点 */
    list_for_each(me, &g_stPSTransHead.TransHead)
    {
        diag_crit("header 0x%llx, 0x%llx.\n", me->next, me->prev);
    }
    (VOS_VOID)VOS_SmV(g_stPSTransHead.TransSem);

    return;
}

EXPORT_SYMBOL(DIAG_ShowTransList);

