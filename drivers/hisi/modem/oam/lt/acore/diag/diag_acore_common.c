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

#include "diag_acore_common.h"
#include <linux/debugfs.h>
#include <securec.h>
#include <vos.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <msp.h>
#include <omerrorlog.h>
#include <nv_stru_sys.h>
#include <soc_socp_adapter.h>
#include <eagleye.h>
#include "diag_common.h"
#include "diag_msgbsp.h"
#include "diag_msgbbp.h"
#include "diag_msg_easyrf.h"
#include "diag_msgphy.h"
#include "diag_msgps.h"
#include "diag_msghifi.h"
#include "diag_msgapplog.h"
#include "diag_api.h"
#include "diag_cfg.h"
#include "diag_debug.h"
#include "diag_connect.h"
#include "diag_msglrm.h"
#include "diag_msgnrm.h"
#include "diag_msgapplog.h"
#include "diag_time_stamp.h"
#include "diag_message.h"
#include "diag_msg_def.h"
#include "diag_time_stamp.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_ACORE_COMMON_C

drv_reset_cb_moment_e g_DiagResetingCcore = MDRV_RESET_CB_INVALID;
VOS_UINT32 g_ulDebugCfg = 0;
DIAG_DUMP_INFO_STRU g_stDumpInfo = { 0 };
struct wakeup_source g_diag_wakelock;
HTIMER g_TransCnfTimer = VOS_NULL;

VOS_INT diag_ResetCcoreCB(drv_reset_cb_moment_e enParam, int userdata)
{
    diag_trans_ind_s stResetMsg = {};
    VOS_UINT32 ret = ERR_MSP_SUCCESS;

    if (enParam == MDRV_RESET_CB_BEFORE) {
        diag_crit("Diag receive ccore reset Callback.\n");

        g_DiagResetingCcore = MDRV_RESET_CB_BEFORE;

        diag_SetPhyPowerOnState(DIAG_PHY_POWER_ON_DOING);

        if (!DIAG_IS_CONN_ON) {
            return ERR_MSP_SUCCESS;
        }

        stResetMsg.module = DRV_DIAG_GEN_MODULE_EX(DIAG_MODEM_0, DIAG_MODE_LTE, DIAG_MSG_TYPE_MSP);
        stResetMsg.pid = MSP_PID_DIAG_APP_AGENT;
        stResetMsg.msg_id = DIAG_CMD_MODEM_WILL_RESET;
        stResetMsg.reserved = 0;
        stResetMsg.length = sizeof(VOS_UINT32);
        stResetMsg.data = (void *)&ret;

        ret = mdrv_diag_report_reset_msg(&stResetMsg);
        if (ret) {
            diag_error("report reset msg fail, ret:0x%x\n", ret);
        }

        diag_crit("Diag report ccore reset to HIDP,and reset SOCP timer.\n");
        /* modem单独复位时，把中断超时时间恢复为默认值，让HIDP尽快收到复位消息 */
        mdrv_socp_set_ind_mode(SOCP_IND_MODE_DIRECT);
#ifdef DIAG_SEC_TOOLS
        diag_set_auth_state(DIAG_AUTH_TYPE_DEFAULT);
#endif
    } else if (enParam == MDRV_RESET_CB_AFTER) {
        g_DiagResetingCcore = MDRV_RESET_CB_AFTER;
    } else {
        diag_error("enParam(0x%x) error\n", enParam);
    }
    return ERR_MSP_SUCCESS;
}

VOS_UINT32 diag_AppAgentMsgProcInit(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 ret = ERR_MSP_SUCCESS;
    VOS_CHAR *resetName = "DIAG"; /* C核单独复位的名字 */
    VOS_INT resetLevel = 49;

    if (ip == VOS_IP_LOAD_CONFIG) {
        wakeup_source_init(&g_diag_wakelock, "g_diag_wakelock");
        ret = (VOS_UINT32)mdrv_sysboot_register_reset_notify(resetName, (pdrv_reset_cbfun)diag_ResetCcoreCB, 0,
            resetLevel);
        if (ret) {
            diag_error("diag register ccore reset fail\n");
        }

        diag_MspMsgInit();
        diag_BspMsgInit();
        diag_DspMsgInit();
        diag_EasyRfMsgInit();
        diag_BbpMsgInit();
        diag_PsMsgInit();
        diag_HifiMsgInit();
        diag_MessageInit();
        diag_AppLogMsgInit();
        diag_LRMMsgInit();
        diag_NrmMsgInit();
    } else if (ip == VOS_IP_RESTART) {
        diag_ConnReset();

        /* 都初始化结束后再设置开关 */
        diag_CfgResetAllSwt();
        /* 检测是否需要打开开机log */
        mdrv_scm_set_power_on_log();

        if (diag_IsPowerOnLogOpen() == VOS_TRUE) {
            /* 在打开开机log前，推送一个时间戳高位给工具 */
            diag_PushHighTs();
            diag_PowerOnLogHighTsTimer();
            g_ulDiagCfgInfo |= DIAG_CFG_POWERONLOG;
#ifdef DIAG_SYSTEM_5G
            /* 打开SOCP通道 */
            mdrv_socp_start(SOCP_CODER_SRC_ACPU_IND);
#endif
        }

        mdrv_scm_reg_ind_coder_dst_send_fuc();

        diag_crit("Diag PowerOnLog is %s.\n", (g_ulDiagCfgInfo & DIAG_CFG_POWERONLOG) ? "open" : "close");
    }

    return ret;
}

/*
 * Function Name: diag_DumpMsgInfo
 * Description: 保存A核最后收到的64条消息
 * Input:
 * 注意事项:
 * 不支持重入，由调用者保证不会重复进入
 */
VOS_VOID diag_DumpMsgInfo(VOS_UINT32 ulSenderPid, VOS_UINT32 ulMsgId, VOS_UINT32 ulSize)
{
    VOS_UINT32 ulPtr = g_stDumpInfo.ulMsgCur;

    if (g_stDumpInfo.pcMsgAddr) {
        *((VOS_UINT32 *)(&g_stDumpInfo.pcMsgAddr[ulPtr])) = ulSenderPid;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32 *)(&g_stDumpInfo.pcMsgAddr[ulPtr])) = ulMsgId;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32 *)(&g_stDumpInfo.pcMsgAddr[ulPtr])) = ulSize;
        ulPtr = ulPtr + sizeof(VOS_UINT32);
        *((VOS_UINT32 *)(&g_stDumpInfo.pcMsgAddr[ulPtr])) = mdrv_timer_get_normal_timestamp();

        g_stDumpInfo.ulMsgCur = (g_stDumpInfo.ulMsgCur + 0x10);
        if (g_stDumpInfo.ulMsgCur >= g_stDumpInfo.ulMsgLen) {
            g_stDumpInfo.ulMsgCur = 0;
        }
    }
}

VOS_VOID diag_ApAgentMsgProc(MsgBlock *pMsgBlock)
{
    DIAG_DATA_MSG_STRU *pMsgTmp = VOS_NULL;

    COVERITY_TAINTED_SET((VOS_VOID *)(pMsgBlock->value));

    pMsgTmp = (DIAG_DATA_MSG_STRU *)pMsgBlock;

    switch (pMsgTmp->ulMsgId) {
        case ID_MSG_DIAG_HSO_DISCONN_IND:
            (VOS_VOID)diag_SetChanDisconn(pMsgBlock);
            break;

        case ID_MSG_DIAG_CMD_CONNECT_REQ:
            (VOS_VOID)diag_ConnMgrProc(pMsgTmp->pContext);
            break;
        case ID_MSG_DIAG_CMD_DISCONNECT_REQ:
            (VOS_VOID)diag_DisConnMgrProc(pMsgTmp->pContext);
            break;

        default:
            break;
    }
}

VOS_VOID diag_TimerMsgProc(MsgBlock *pMsgBlock)
{
    REL_TimerMsgBlock *pTimer = NULL;

    pTimer = (REL_TimerMsgBlock *)pMsgBlock;

    if ((pTimer->name == DIAG_DEBUG_TIMER_NAME) && (pTimer->para == DIAG_DEBUG_TIMER_PARA)) {
        diag_ReportMntn();
    } else if ((pTimer->name == DIAG_HIGH_TS_PUSH_TIMER_NAME) && (pTimer->para == DIAG_HIGH_TS_PUSH_TIMER_PARA)) {
        /* 推送时间戳高位 */
        diag_PushHighTs();
    } else if ((pTimer->name == DIAG_FLOW_ACORE_TIMER_NAME) && (pTimer->para == DIAG_ACORE_FLOW_TIMER_PARA)) {
        /* 推送时间戳高位 */
        DIAG_ACoreFlowPress();
#if defined(DIAG_SEC_TOOLS) && defined(DIAG_SYSTEM_5G)
    } else if ((pTimer->name == DIAG_AUTH_TIMER_NAME) && (pTimer->para == DIAG_AUTH_TIMER_PARA)) {
        /* 鉴权定时器超时 */
        diag_ConnAuthTimerProc();
#endif
    } else if ((DIAG_TRANS_CNF_TIMER_NAME == pTimer->name) && (DIAG_TRANS_CNF_TIMER_PARA == pTimer->para)) {
        diag_TransTimeoutProc(pTimer);
    }

    return;
}
/*
 * Function Name: diag_AppAgentMsgProc
 * Description: DIAG APP AGENT接收到的消息处理入口
 * 注意事项:
 * 由于errorlog的消息不能识别发送PID，所以需要进入errorlog的处理函数中检查
 * 当已知消息被成功处理时，则不需要再进行errorlog的消息检查
 * 通过ulErrorLog的值判断是否进行errorlog的消息检查
 * 后续函数扩展时需要注意
 */
VOS_VOID diag_AppAgentMsgProc(MsgBlock *pMsgBlock)
{
    VOS_UINT32 ulErrorLog = ERR_MSP_CONTINUE; /* 见函数头中的注意事项的描述 */

    /* 入参判断 */
    if (pMsgBlock == NULL) {
        return;
    }

    /* 任务开始处理，不允许睡眠 */
    __pm_stay_awake(&g_diag_wakelock);

    diag_DumpMsgInfo(pMsgBlock->ulSenderPid, (VOS_UINT32)(*(VOS_UINT16 *)pMsgBlock->value), pMsgBlock->ulLength);

    /* 根据发送PID，执行不同处理 */
    switch (pMsgBlock->ulSenderPid) {
        /* 超时消息，按照超时包格式，打包回复 */
        case DOPRA_PID_TIMER:
            diag_TimerMsgProc(pMsgBlock);
            ulErrorLog = VOS_OK;
            break;

        case WUEPS_PID_REG:
        case CCPU_PID_PAM_OM:
            ulErrorLog = diag_TransCnfProc((VOS_UINT8 *)pMsgBlock, (pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH),
                DIAG_MSG_TYPE_BBP, &g_stPSTransHead);
            break;

        case MSP_PID_DIAG_APP_AGENT:
            diag_ApAgentMsgProc(pMsgBlock);
            ulErrorLog = VOS_OK;
            break;
        /*lint -save -e826*/
        case MSP_PID_DIAG_AGENT:
            diag_LrmAgentMsgProc(pMsgBlock);
            break;
        case MSP_PID_DIAG_NRM_AGENT:
            diag_NrmAgentMsgProc(pMsgBlock);
            break;
        /*lint -restore +e826*/
        case DSP_PID_APM:
            ulErrorLog = ERR_MSP_CONTINUE;
            break;

        case DSP_PID_STARTUP: {
            diag_crit("receive DSP_PID_STARTUP msg\n");
            diag_GuPhyMsgProc(pMsgBlock);
        } break;

        default:
            ulErrorLog = diag_TransCnfProc((VOS_UINT8 *)pMsgBlock, (pMsgBlock->ulLength + VOS_MSG_HEAD_LENGTH),
                DIAG_MSG_TYPE_PS, &g_stPSTransHead);
            break;
    }

    /* 任务开始结束，允许睡眠 */
    __pm_relax(&g_diag_wakelock);
    if (ulErrorLog != VOS_OK && ulErrorLog != ERR_MSP_CONTINUE) {
        diag_error("handle msg err,sender:%x, err:%x\n", pMsgBlock->ulSenderPid, ulErrorLog);
    }

    return;
}

/*
 * Function Name: diag_AddTransInfoToList
 * Description: 添加解码后的透传命令数据到链表中
 */
DIAG_TRANS_NODE_STRU *diag_AddTransInfoToList(VOS_UINT8 *pstReq, VOS_UINT32 ulRcvlen, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_NODE_STRU *pNewNode = NULL;
    VOS_UINT32 ulNodeSize;
    VOS_INT32 ret;

    ulNodeSize = sizeof(DIAG_TRANS_NODE_STRU) + ulRcvlen;
    if (ulNodeSize > DIAG_MEM_ALLOC_LEN_MAX) {
        diag_error("ERROR: ulNodeSize=0x%x,\n", ulNodeSize);
        return NULL;
    }

    /* 申请一个节点大小 */
    pNewNode = mdrv_diag_dymalloc(ulNodeSize);
    if (NULL == pNewNode) {
        diag_error("malloc fail\n");
        return NULL;
    }

    ret = memset_s(pNewNode, ulNodeSize, 0, ulNodeSize);
    if (ret) {
        diag_error("memset fail:%x\n", ret);
    }
    /* 将新来的命令缓存到节点中 */
    ret = memcpy_s(pNewNode->ucRcvData, ulRcvlen, pstReq, ulRcvlen);
    if (ret) {
        diag_error("memcpy fail:%x\n", ret);
    }

    /* 添加信号量保护 */
    (VOS_VOID)VOS_SmP(pstHead->TransSem, 0);

    if (list_empty(&pstHead->TransHead)) {
        ret = VOS_StartRelTimer(&g_TransCnfTimer, MSP_PID_DIAG_APP_AGENT, DIAG_TRANS_TIMEOUT_LEN,
            DIAG_TRANS_CNF_TIMER_NAME, DIAG_TRANS_CNF_TIMER_PARA, VOS_RELTIMER_LOOP, VOS_TIMER_NO_PRECISION);
        if (ret) {
            diag_error("VOS_StartRelTimer fail, ret=0x%x\n", ret);
        }
    }

    /* 插入节点到链表尾部 */
    list_add_tail(&pNewNode->DiagList, &pstHead->TransHead);


    pNewNode->ulMagicNum = DIAG_TRANS_MAGIC_NUM;
    pNewNode->StartTime = mdrv_timer_get_normal_timestamp();

    (VOS_VOID)VOS_SmV(pstHead->TransSem);

    return pNewNode;
}

VOS_UINT32 diag_TransReqProcEntry(msp_diag_frame_info_s *pstReq, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_MSG_STRU *pstSendReq = NULL;
    DIAG_TRANS_NODE_STRU *pNode = VOS_NULL;
    DIAG_OSA_MSG_STRU *pstMsg = NULL;
    VOS_UINT32 ret = ERR_MSP_FAILURE;
    VOS_UINT32 ulSize, ulCmdParasize;
#ifdef CONFIG_ARM64
    VOS_UINT_PTR ullAddr;
#endif

    if (pstReq->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_TRANS_MSG_STRU) - sizeof(pstSendReq->para)) {
        diag_FailedCmdCnf(pstReq, ERR_MSP_INALID_LEN_ERROR);
        return ERR_MSP_INALID_LEN_ERROR;
    }

    ulCmdParasize = pstReq->ulMsgLen - sizeof(msp_diag_data_head_s);

    /* 打包透传数据 */
    pstSendReq = (DIAG_TRANS_MSG_STRU *)(pstReq->aucData + sizeof(msp_diag_data_head_s));
    if (VOS_PID_AVAILABLE != VOS_CheckPidValidity(pstSendReq->ulReceiverPid)) {
        diag_FailedCmdCnf(pstReq, ERR_MSP_DIAG_ERRPID_CMD);
        return ERR_MSP_FAILURE;
    }

    ulSize = sizeof(msp_diag_frame_info_s) + pstReq->ulMsgLen;
    pNode = diag_AddTransInfoToList((VOS_UINT8 *)pstReq, ulSize, pstHead);
    if (pNode == VOS_NULL) {
        return ERR_MSP_FAILURE;
    }

    /* 写低32位 */
    pstSendReq->ulSN = (uintptr_t)pNode;
#ifdef CONFIG_ARM64
    ullAddr = (VOS_UINT_PTR)pNode;
    pstSendReq->usOriginalId = (VOS_UINT16)((ullAddr >> 0x20) & 0x0000FFFF);
    pstSendReq->usTerminalId = (VOS_UINT16)((ullAddr >> 0x30) & 0x0000FFFF);
#endif
    if (ulCmdParasize > DIAG_MEM_ALLOC_LEN_MAX) {
        diag_error("ERROR: ulMsgId=0x%x, ulCmdParasize = 0x%x.\n", pstReq->ulCmdId, ulCmdParasize);
        return ERR_MSP_FAILURE;
    }
    pstMsg = (DIAG_OSA_MSG_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, (ulCmdParasize - VOS_MSG_HEAD_LENGTH));
    if (pstMsg != NULL) {
        pstMsg->ulReceiverPid = pstSendReq->ulReceiverPid;
        ret = (VOS_UINT32)memcpy_s(&pstMsg->ulMsgId, ulCmdParasize - VOS_MSG_HEAD_LENGTH, &pstSendReq->msgId,
            (ulCmdParasize - VOS_MSG_HEAD_LENGTH));
        ret = VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstMsg);
        if (ret) {
            diag_error("VOS_SendMsg failed!\n");
            return ret;
        }
    }

    return ret;
}

/*
 * Function Name: diag_DelTransCmdNode
 * Description: 删除已经处理完的节点
 */
VOS_VOID diag_DelTransCmdNode(DIAG_TRANS_NODE_STRU *pTempNode)
{
    list_del(&pTempNode->DiagList);
    mdrv_diag_dymem_free(pTempNode);

    if (list_empty(&g_stPSTransHead.TransHead)) {
        if (g_TransCnfTimer != VOS_NULL) {
            (VOS_VOID)VOS_StopRelTimer(&g_TransCnfTimer);
        }
    }

    return;
}

/*
 * Function Name: diag_TransTimeoutProc
 * Description: 透传命令的超时处理
 */
VOS_VOID diag_TransTimeoutProc(REL_TimerMsgBlock *pTimer)
{
    DIAG_TRANS_NODE_STRU *pNode = VOS_NULL;
    msp_diag_frame_info_s *pFrame = VOS_NULL;
    VOS_UINT32 ulTimeStamp;
    VOS_UINT32 ulDetaTime;
    struct list_head *me = NULL;
    struct list_head *n = NULL;

    ulTimeStamp = mdrv_timer_get_normal_timestamp();

    (VOS_VOID)VOS_SmP(g_stPSTransHead.TransSem, 0);

    /* 在链表中查找每个子命令结点 */
    list_for_each_safe(me, n, &g_stPSTransHead.TransHead)
    {
        pNode = list_entry(me, DIAG_TRANS_NODE_STRU, DiagList);

        ulDetaTime = DIAG_TRANS_CNF_TIMEOUT(ulTimeStamp, pNode->StartTime);
        if (ulDetaTime > DIAG_TRANS_TIMEOUT_LEN) {
            pFrame = (msp_diag_frame_info_s *)pNode->ucRcvData;
            diag_info("trans timeout : cmdid 0x%x.\n", pFrame->ulCmdId);

            diag_DelTransCmdNode(pNode);
            continue;
        }

        break;
    }

    (VOS_VOID)VOS_SmV(g_stPSTransHead.TransSem);
    return;
}

DIAG_TRANS_NODE_STRU *diag_IsTransCnf(DIAG_TRANS_MSG_STRU *pstPsCnf, DIAG_TRANS_HEADER_STRU *pstHead)
{
    DIAG_TRANS_NODE_STRU *pNode = VOS_NULL;
    DIAG_TRANS_NODE_STRU *pTempNode = VOS_NULL;
    struct list_head *me = NULL;
#ifdef CONFIG_ARM64
    VOS_UINT_PTR ullAddr;
#endif

#ifndef CONFIG_ARM64
    pNode = (DIAG_TRANS_NODE_STRU *)(VOS_UINT_PTR)pstPsCnf->ulSN;
#else
    ullAddr = (VOS_UINT_PTR)pstPsCnf->usTerminalId;
    ullAddr = (ullAddr << 0x10) | pstPsCnf->usOriginalId;
    ullAddr = (ullAddr << 0x20) | pstPsCnf->ulSN;
    pNode = (DIAG_TRANS_NODE_STRU *)ullAddr;
#endif
    /* 添加信号量保护 */
    (VOS_VOID)VOS_SmP(pstHead->TransSem, 0);

    /* 在链表中查找每个子命令结点 */
    list_for_each(me, &pstHead->TransHead)
    {
        pTempNode = list_entry(me, DIAG_TRANS_NODE_STRU, DiagList);

        if (pTempNode == pNode) {
            (VOS_VOID)VOS_SmV(pstHead->TransSem);

            /* ulMagicNum非法表示节点已超时删除 */
            if (DIAG_TRANS_MAGIC_NUM != pNode->ulMagicNum) {
                return VOS_NULL;
            }

            return pNode;
        }
    }

    (VOS_VOID)VOS_SmV(pstHead->TransSem);

    return VOS_NULL;
}

/*
 * Function Name: diag_GetTransInfo
 * Description: 获取透传命令的信息，并删除节点
 */
VOS_VOID diag_GetTransInfo(MSP_DIAG_CNF_INFO_STRU *pstInfo, DIAG_TRANS_CNF_STRU *pstDiagCnf,
    DIAG_TRANS_MSG_STRU *pstPsCnf, DIAG_TRANS_NODE_STRU *pNode)
{
    msp_diag_frame_info_s *pFrame = VOS_NULL;
    msp_diag_data_head_s *pDiagData = VOS_NULL;
    DIAG_TRANS_MSG_STRU *pstOmMsg = VOS_NULL;

    /* 添加信号量保护 */
    (VOS_VOID)VOS_SmP(g_stPSTransHead.TransSem, 0);

    pFrame = (msp_diag_frame_info_s *)pNode->ucRcvData;
    pstInfo->ulTransId = pFrame->stService.ulMsgTransId;
    pstInfo->ulMsgId = pFrame->ulCmdId;
    pstInfo->ulMode = pFrame->stID.mode4b;
    pstInfo->ulSubType = pFrame->stID.sec5b;

    pDiagData = (msp_diag_data_head_s *)pFrame->aucData;
    pstDiagCnf->ulAuid = pDiagData->ulAuid;
    pstDiagCnf->ulSn = pDiagData->ulSn;

    pstOmMsg = (DIAG_TRANS_MSG_STRU *)pDiagData->ucData;

    pstPsCnf->usOriginalId = pstOmMsg->usOriginalId;
    pstPsCnf->usTerminalId = pstOmMsg->usTerminalId;
    pstPsCnf->ulSN = pstOmMsg->ulSN;

    diag_DelTransCmdNode(pNode);
    (VOS_VOID)VOS_SmV(g_stPSTransHead.TransSem);

    return;
}

VOS_UINT32 diag_TransCnfProc(VOS_UINT8 *pstCnf, VOS_UINT32 ulLen, unsigned int ulGroupId,
    DIAG_TRANS_HEADER_STRU *pstHead)
{
    VOS_UINT32 ret = 0;
    DIAG_TRANS_CNF_STRU *pstDiagCnf = VOS_NULL;
    DIAG_TRANS_MSG_STRU *pstPsCnf = VOS_NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    DIAG_TRANS_NODE_STRU *pNode = VOS_NULL;
    errno_t err = EOK;

    if (VOS_NULL_PTR == pstCnf) {
        return ERR_MSP_FAILURE;
    }
    pstPsCnf = (DIAG_TRANS_MSG_STRU *)pstCnf;

    mdrv_diag_ptr(EN_DIAG_PTR_TRANS_CNF_PROC, 1, ulGroupId, 0);

    if (pstPsCnf->ulLength < (sizeof(DIAG_TRANS_MSG_STRU) - VOS_MSG_HEAD_LENGTH - sizeof(pstPsCnf->para))) {
        /* 如果长度小于透传命令的回复长度肯定不是透传命令 */
        return ERR_MSP_CONTINUE;
    }

    pNode = diag_IsTransCnf(pstPsCnf, pstHead);
    if (VOS_NULL == pNode) {
        return ERR_MSP_CONTINUE;
    }

    diag_LNR(EN_DIAG_LNR_PS_TRANS, ((pstPsCnf->usOriginalId << 0x10) | pstPsCnf->usTerminalId), pstPsCnf->ulSN);

    if (DIAG_DEBUG_TRANS & g_ulDebugCfg) {
        diag_error("cmdid 0x%x, pid %d, msgid 0x%x.\n", stDiagInfo.ulMsgId, pstPsCnf->ulSenderPid, pstPsCnf->msgId);
    }

    if ((ulLen + sizeof(DIAG_TRANS_CNF_STRU)) > DIAG_MEM_ALLOC_LEN_MAX) {
        diag_error("ERROR: ulLen=0x%x.\n", ulLen);
        return ERR_MSP_NOT_ENOUGH_MEMORY;
    }
    pstDiagCnf = VOS_MemAlloc(MSP_PID_DIAG_APP_AGENT, DYNAMIC_MEM_PT, (ulLen + sizeof(DIAG_TRANS_CNF_STRU)));
    if (VOS_NULL_PTR == pstDiagCnf) {
        return ERR_MSP_FAILURE;
    }

    stDiagInfo.ulSSId = DIAG_SSID_APP_CPU;
    stDiagInfo.ulMsgType = ulGroupId;
    stDiagInfo.ulDirection = DIAG_MT_CNF;
    stDiagInfo.ulModemid = 0;

    diag_GetTransInfo(&stDiagInfo, pstDiagCnf, pstPsCnf, pNode);

    diag_LNR(EN_DIAG_LNR_PS_TRANS, (stDiagInfo.ulMsgId), VOS_GetSlice());

    err = memcpy_s(pstDiagCnf->aucPara, ulLen, pstPsCnf, ulLen);
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    ret = DIAG_MsgReport(&stDiagInfo, pstDiagCnf, (ulLen + sizeof(DIAG_TRANS_CNF_STRU)));

    VOS_MemFree(MSP_PID_DIAG_APP_AGENT, pstDiagCnf);

    return ret;
}



VOS_UINT32 MSP_AppDiagFidInit(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 ulRelVal;

    switch (ip) {
        case VOS_IP_LOAD_CONFIG:
            mdrv_ppm_reg_disconnect_cb(diag_DisconnectTLPort);

            ulRelVal = VOS_RegisterPIDInfo(MSP_PID_DIAG_APP_AGENT, (InitFunType)diag_AppAgentMsgProcInit,
                (MsgFunType)diag_AppAgentMsgProc);
            if (ulRelVal != VOS_OK) {
                return VOS_ERR;
            }

            ulRelVal = VOS_RegisterMsgTaskPrio(MSP_FID_DIAG_ACPU, VOS_PRIORITY_M2);
            if (ulRelVal != VOS_OK) {
                return VOS_ERR;
            }

            /* 申请8K的dump空间 */
            g_stDumpInfo.pcDumpAddr = (VOS_VOID *)mdrv_om_register_field(OM_AP_DIAG, "ap_diag", DIAG_DUMP_LEN, 0);

            if (g_stDumpInfo.pcDumpAddr != VOS_NULL) {
                g_stDumpInfo.pcMsgAddr = g_stDumpInfo.pcDumpAddr;
                g_stDumpInfo.ulMsgCur = 0;
                g_stDumpInfo.ulMsgLen = DIAG_DUMP_MSG_LEN;

                g_stDumpInfo.pcDFAddr = g_stDumpInfo.pcDumpAddr + DIAG_DUMP_MSG_LEN;
                g_stDumpInfo.ulDFCur = 0;
                g_stDumpInfo.ulDFLen = DIAG_DUMP_DF_LEN;
            }

            VOS_RegisterMsgGetHook((VOS_MsgHookFunc)mdrv_diag_layer_msg_report);
            return VOS_OK;

        default:
            break;
    }

    return VOS_OK;
}

VOS_VOID DIAG_DebugTransOn(VOS_UINT32 ulOn)
{
    if (ulOn == 0) {
        g_ulDebugCfg &= (~DIAG_DEBUG_TRANS);
    } else {
        g_ulDebugCfg |= DIAG_DEBUG_TRANS;
    }
}

/*
 * 函 数 名: diag_DisconnectTLPort
 * 功能描述: TL断开OM端口
 * 输入参数: 无
 * 输出参数: 无
 * 返 回 值: BSP_ERROR/BSP_OK
 * 修改历史:
 * 1.日    期: 2014年05月26日
 * 作    者: h59254
 * 修改内容: Creat Function
 */
VOS_UINT32 diag_DisconnectTLPort(void)
{
    DIAG_DATA_MSG_STRU *pstMsg;

    pstMsg =
        (DIAG_DATA_MSG_STRU *)VOS_AllocMsg(MSP_PID_DIAG_APP_AGENT, sizeof(DIAG_DATA_MSG_STRU) - VOS_MSG_HEAD_LENGTH);
    if (pstMsg == NULL) {
        return VOS_ERR;
    }

    pstMsg->ulReceiverPid = MSP_PID_DIAG_APP_AGENT;
    pstMsg->ulMsgId = ID_MSG_DIAG_HSO_DISCONN_IND;

    (void)VOS_SendMsg(MSP_PID_DIAG_APP_AGENT, pstMsg);

    return VOS_OK;
}
EXPORT_SYMBOL(DIAG_DebugTransOn);

