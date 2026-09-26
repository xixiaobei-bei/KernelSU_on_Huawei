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
#include <securec.h>
#include "diag_cfg.h"
#include <product_config.h>
#include <mdrv.h>
#include <mdrv_diag_system.h>
#include <nv_stru_drv.h>
#include <nv_stru_lps.h>
#include <soc_socp_adapter.h>
#include <msp.h>
#include "diag_common.h"
#include "diag_msgmsp.h"
#include "diag_msgphy.h"
#include "diag_debug.h"
#include "diag_msg_def.h"
#include "diag_cmdid_def.h"
#include "diag_message.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_CFG_C

/*
 * 2 Declare the Global Variable
 */
#define DIAG_CFG_PRINT_MODULE_NUM 44

VOS_UINT32 g_ulDiagCfgInfo = 0;
VOS_UINT8 g_ALayerSrcModuleCfg[VOS_CPU_ID_1_PID_BUTT - VOS_PID_CPU_ID_1_DOPRAEND] = {0};
VOS_UINT8 g_CLayerSrcModuleCfg[VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND] = {0};
VOS_UINT8 g_NrmLayerSrcModuleCfg[VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND] = {0};
VOS_UINT8 g_ALayerDstModuleCfg[VOS_CPU_ID_1_PID_BUTT - VOS_PID_CPU_ID_1_DOPRAEND] = {0};
VOS_UINT8 g_CLayerDstModuleCfg[VOS_CPU_ID_0_PID_BUTT - VOS_PID_CPU_ID_0_DOPRAEND] = {0};
VOS_UINT8 g_NrmLayerDstModuleCfg[VOS_CPU_ID_2_PID_BUTT - VOS_PID_CPU_ID_2_DOPRAEND] = {0};
VOS_UINT8 g_EventModuleCfg[DIAG_CFG_PID_NUM] = {0};
VOS_UINT8 g_PrintModuleCfg[DIAG_CFG_PID_NUM] = {0};
VOS_UINT32 g_PrintTotalCfg = DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE;
DIAG_CFG_LOG_CAT_CFG_STRU g_stMsgCfg = { 0 };
VOS_UINT32 g_ulTransCfg = DIAG_CFG_SWT_CLOSE;
VOS_UINT32 g_miniLogCfg = DIAG_CFG_SWT_CLOSE;


VOS_VOID diag_CfgResetAllSwt(VOS_VOID)
{
    errno_t err;
    /* 清空层间开关状态 */
    err = memset_s(g_ALayerSrcModuleCfg, (VOS_UINT32)sizeof(g_ALayerSrcModuleCfg), 0,
        (VOS_UINT32)sizeof(g_ALayerSrcModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memset_s(g_CLayerSrcModuleCfg, (VOS_UINT32)sizeof(g_CLayerSrcModuleCfg), 0,
        (VOS_UINT32)sizeof(g_CLayerSrcModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memset_s(g_NrmLayerSrcModuleCfg, (VOS_UINT32)sizeof(g_NrmLayerSrcModuleCfg), 0,
        (VOS_UINT32)sizeof(g_NrmLayerSrcModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memset_s(g_ALayerDstModuleCfg, (VOS_UINT32)sizeof(g_ALayerDstModuleCfg), 0,
        (VOS_UINT32)sizeof(g_ALayerDstModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memset_s(g_CLayerDstModuleCfg, (VOS_UINT32)sizeof(g_CLayerDstModuleCfg), 0,
        (VOS_UINT32)sizeof(g_CLayerDstModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }
    err = memset_s(g_NrmLayerDstModuleCfg, (VOS_UINT32)sizeof(g_NrmLayerDstModuleCfg), 0,
        (VOS_UINT32)sizeof(g_NrmLayerDstModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 为兼容原TL任务的EVENT开关机制，默认把所有EVENT子开关都设置为打开 */
    err = memset_s(g_EventModuleCfg, (VOS_UINT32)sizeof(g_EventModuleCfg), 0x1, (VOS_UINT32)sizeof(g_EventModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 清空打印开关状态 */
    err = memset_s(g_PrintModuleCfg, (VOS_UINT32)sizeof(g_PrintModuleCfg), 0, (VOS_UINT32)sizeof(g_PrintModuleCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    /* 清空打印总开关状态 */
    g_PrintTotalCfg = DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE;

    /* 清空消息过滤开关状态 */
    err = memset_s(&g_stMsgCfg, (VOS_UINT32)sizeof(g_stMsgCfg), 0, (VOS_UINT32)sizeof(g_stMsgCfg));
    if (err != EOK) {
        diag_error("memset fail:%x\n", err);
    }

    mdrv_diag_report_reset();

#ifdef DIAG_SYSTEM_5G
    g_ulTransCfg = DIAG_CFG_SWT_CLOSE;
#endif
    return;
}

VOS_UINT32 diag_CfgSetGlobalBitValue(VOS_UINT32 *pstDiagGlobal, ENUM_DIAG_CFG_BIT_U32 enBit,
    ENUM_DIAG_CFG_SWT_U8 enSwtich)
{
    /* 设置为open 1时，需要使用|才能置该bit 为1 */
    if (enSwtich == DIAG_CFG_SWT_OPEN) {
        *pstDiagGlobal |= ((VOS_UINT)1 << enBit);
    } else if (enSwtich == DIAG_CFG_SWT_CLOSE) {
        /* 设置为close 0时，需要使用&才能置该bit 为0 */
        *pstDiagGlobal &= ~((VOS_UINT)1 << enBit);
    } else {
        return ERR_MSP_INVALID_PARAMETER;
    }
    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_AirCfgProc
 * Description: 该函数用于处理CfgProcEntry传进来的空口开关命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_AirCfgProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ret;
    DIAG_CMD_LOG_CAT_AIR_REQ_STRU *pstAirSwtReq = NULL;
    DIAG_CMD_LOG_CAT_AIR_CNF_STRU stAirSwtCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    msp_diag_frame_info_s *pstDiagHead = VOS_NULL;
    ENUM_DIAG_CFG_SWT_U8 enLSwitch;
    VOS_UINT32 ulLen;
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);
    mdrv_diag_ptr(EN_DIAG_PTR_AIR_CFG_PROC, 1, pstDiagHead->ulCmdId, 0);

    /* 为兼容工具旧版本丢开机LOG问题，待工具发布一段时间后删除 */
    mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);

    if ((pstDiagHead->ulMsgLen < sizeof(DIAG_CMD_LOG_CAT_AIR_REQ_STRU) + sizeof(msp_diag_data_head_s)) ||
        (pstDiagHead->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("DataLen error, head->msglen:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    pstAirSwtReq = (DIAG_CMD_LOG_CAT_AIR_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    /* 设置LT空口开关值 */
    enLSwitch = DIAG_GET_CFG_SWT(pstAirSwtReq->ulSwitch);
    ret = diag_CfgSetGlobalBitValue(&g_ulDiagCfgInfo, DIAG_CFG_LT_AIR_BIT, enLSwitch);
    if (ret == ERR_MSP_SUCCESS) {
        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);
    }

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stAirSwtCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stAirSwtCnf.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stAirSwtCnf, (VOS_UINT32)sizeof(stAirSwtCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_AIR_CFG_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);

    /* 开机log上报特性中，在工具下发AirCfg配置时，才打开启动缓存的log上报 */
    if (DIAG_IS_POLOG_ON) {
        /* 收到空口开关恢复为正常的开关控制上报 */
        g_ulDiagCfgInfo &= (~DIAG_CFG_POWERONLOG);

#ifndef DIAG_SEC_TOOLS
        mdrv_socp_send_data_manager(SOCP_CODER_DST_OM_IND, SOCP_DEST_DSM_ENABLE);
#endif
    }
    return ret;
}


VOS_UINT32 diag_CfgSetLayerSwt(DIAG_CMD_LOG_CAT_LAYER_REQ_STRU *pstLayerReq, VOS_UINT32 ulCfgSize)
{
    VOS_UINT32 j;
    VOS_UINT32 ulOffset;
    ENUM_DIAG_CFG_SWT_U8 enSwitch;

    if ((ulCfgSize == 0) || ((ulCfgSize % sizeof(DIAG_CMD_LOG_CAT_LAYER_REQ_STRU) != 0))) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    /* 遍历某Category的开关配置项列表，查找对应的配置项进行设置 */
    for (j = 0; j < ulCfgSize / sizeof(DIAG_CMD_LOG_CAT_LAYER_REQ_STRU); j++) {
        enSwitch = DIAG_GET_CFG_SWT((pstLayerReq + j)->ulSwitch);

        if (DIAG_CFG_LAYER_MODULE_IS_ACORE((pstLayerReq + j)->ulModuleId)) {
            ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET((pstLayerReq + j)->ulModuleId);

            if ((pstLayerReq + j)->ulIsDestModule == DIAG_CMD_LAYER_MOD_SRC) {
                g_ALayerSrcModuleCfg[ulOffset] = (VOS_UINT8)enSwitch;
            } else {
                g_ALayerDstModuleCfg[ulOffset] = (VOS_UINT8)enSwitch;
            }
        } else if (DIAG_CFG_LAYER_MODULE_IS_CCORE((pstLayerReq + j)->ulModuleId)) {
            ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET((pstLayerReq + j)->ulModuleId);

            if ((pstLayerReq + j)->ulIsDestModule == DIAG_CMD_LAYER_MOD_SRC) {
                g_CLayerSrcModuleCfg[ulOffset] = (VOS_UINT8)enSwitch;
            } else {
                g_CLayerDstModuleCfg[ulOffset] = (VOS_UINT8)enSwitch;
            }
        } else if (DIAG_CFG_LAYER_MODULE_IS_NRM((pstLayerReq + j)->ulModuleId)) {
            ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET((pstLayerReq + j)->ulModuleId);

            if ((pstLayerReq + j)->ulIsDestModule == DIAG_CMD_LAYER_MOD_SRC) {
                g_NrmLayerSrcModuleCfg[ulOffset] = (VOS_UINT8)enSwitch;
            } else {
                g_NrmLayerDstModuleCfg[ulOffset] = (VOS_UINT8)enSwitch;
            }
        }
    }

    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_LayerCfgProc
 * Description: 该函数用于处理CfgProcEntry传进来的层间开关命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_LayerCfgProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ret;
    msp_diag_frame_info_s *pstDiagHead = NULL;
    DIAG_CMD_LOG_CAT_LAYER_REQ_STRU *pstLayerSwtReq = NULL;
    DIAG_CMD_LOG_CAT_LAYER_CNF_STRU stLayerSwtCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    VOS_UINT32 ulLen;
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    mdrv_diag_ptr(EN_DIAG_PTR_LAYER_CFG_PROC, 1, pstDiagHead->ulCmdId, 0);

    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) || (pstDiagHead->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("DataLen error, u32DataLen:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    pstLayerSwtReq = (DIAG_CMD_LOG_CAT_LAYER_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    /* 设置层间模块开关到全局变量中 */
    ret = diag_CfgSetLayerSwt(pstLayerSwtReq, (VOS_UINT32)(pstDiagHead->ulMsgLen - sizeof(msp_diag_data_head_s)));
    if (ret == ERR_MSP_SUCCESS) {
        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);
    }

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stLayerSwtCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stLayerSwtCnf.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stLayerSwtCnf, (VOS_UINT32)sizeof(stLayerSwtCnf));

    mdrv_diag_ptr(EN_DIAG_PTR_LAYER_CFG_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);

    return ret;
}


VOS_UINT32 diag_EventCfgProc(VOS_UINT8 *pstReq)
{
    DIAG_CMD_LOG_CAT_EVENT_REQ_STRU *pstEvtSwtReq = NULL;
    DIAG_CMD_LOG_CAT_EVENT_CNF_STRU stEvtSwtCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    msp_diag_frame_info_s *pstDiagHead = NULL;
    VOS_UINT32 ret;
    VOS_UINT32 i, pid;
    ENUM_DIAG_CFG_SWT_U8 enSwitch;
    VOS_UINT32 ulLen;
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;

    pstDiagHead = (msp_diag_frame_info_s *)pstReq;
    mdrv_diag_ptr(EN_DIAG_PTR_EVENT_CFG_PROC, 1, pstDiagHead->ulCmdId, 0);

    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_LOG_CAT_EVENT_REQ_STRU)) ||
        (pstDiagHead->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("DataLen error, u32DataLen:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    pstEvtSwtReq = (DIAG_CMD_LOG_CAT_EVENT_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_LOG_CAT_EVENT_REQ_STRU) +
        sizeof(VOS_UINT32) * pstEvtSwtReq->ulCount) ||
        (sizeof(VOS_UINT32) * pstEvtSwtReq->ulCount > pstDiagHead->ulMsgLen)) {
        diag_error("DataLen error,u32DataLen:0x%x,switch count:0x%x\n", pstDiagHead->ulMsgLen, pstEvtSwtReq->ulCount);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    enSwitch = (ENUM_DIAG_CFG_SWT_U8)pstEvtSwtReq->ulSwt;

    /* 打开和关闭都是配置固定的模块，只有总开关是打开时才配置到各模块 */
    if (enSwitch) {
        for (i = 0; i < pstEvtSwtReq->ulCount; i++) {
            pid = DIAG_EVENT_MODID(pstEvtSwtReq->aulModuleId[i]);
            if (DIAG_CFG_MODULE_IS_INVALID(pid)) {
                continue;
            }

            g_EventModuleCfg[pid - VOS_PID_DOPRAEND] = DIAG_EVENT_SWT(pstEvtSwtReq->aulModuleId[i]);
        }
    }

    ret = diag_CfgSetGlobalBitValue(&g_ulDiagCfgInfo, DIAG_CFG_EVENT_BIT, enSwitch);
    if (ret == ERR_MSP_SUCCESS) {
        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);
    }

DIAG_ERROR:
    DIAG_MSG_COMMON_PROC(stDiagInfo, stEvtSwtCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stEvtSwtCnf.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stEvtSwtCnf, sizeof(stEvtSwtCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_EVENT_CFG_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);
    return ret;
}


VOS_UINT32 diag_CfgSetMsgSwt(DIAG_CMD_LOG_CAT_CFG_REQ_STRU *pstCatCfgReq, VOS_UINT32 ulCfgSize)
{
    VOS_UINT32 i;
    VOS_UINT32 j;
    ENUM_DIAG_CFG_SWT_U8 enSwitch;
    VOS_UINT32 ulRst = ERR_MSP_INVALID_PARAMETER;
    DIAG_CFG_LOG_CAT_MSG_CFG_STRU *pstItemCfg = NULL;

    /* 参数检查 */
    if ((ulCfgSize == 0) || ((ulCfgSize % sizeof(DIAG_CMD_LOG_CAT_CFG_REQ_STRU)) != 0)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_DEBUG_MSG_ERR, ulCfgSize, 0, 0);
        return ERR_MSP_DIAG_CMD_SIZE_INVALID;
    }

    for (j = 0; j < ulCfgSize / sizeof(DIAG_CMD_LOG_CAT_CFG_REQ_STRU); j++) {
        /* 仅支持层间消息CATEGORY过滤 */
        if ((pstCatCfgReq + j)->ulCategory != DIAG_CMD_LOG_CATETORY_LAYER_ID) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_DEBUG_MSG_ERR, (pstCatCfgReq + j)->ulCategory, 0, 1);
            return ERR_MSP_NOT_SUPPORT;
        }
    }

    /* 遍历某Category的开关配置项列表，查找对应的配置项进行设置 */
    for (j = 0; j < ulCfgSize / sizeof(DIAG_CMD_LOG_CAT_CFG_REQ_STRU); j++) {
        enSwitch = DIAG_GET_CFG_SWT((pstCatCfgReq + j)->ulSwitch);

        for (i = 0; i < g_stMsgCfg.ulCfgCnt; i++) {
            pstItemCfg = (g_stMsgCfg.astMsgCfgList + i);

            if ((pstCatCfgReq + j)->ulId == pstItemCfg->ulId) {
                pstItemCfg->ulSwt = enSwitch;
                ulRst = ERR_MSP_SUCCESS;
                break;
            }
        }
        if (i >= g_stMsgCfg.ulCfgCnt) {
            /* 目前仅一次支持DIAG_CFG_CAT_CFG_NUM个消息过滤 */
            if ((g_stMsgCfg.ulCfgCnt < DIAG_CFG_CAT_CFG_NUM)) {
                pstItemCfg = g_stMsgCfg.astMsgCfgList + g_stMsgCfg.ulCfgCnt;

                pstItemCfg->ulId = (pstCatCfgReq + j)->ulId;
                pstItemCfg->ulSwt = enSwitch;

                g_stMsgCfg.ulCfgCnt++;
                ulRst = ERR_MSP_SUCCESS;
            } else {
                DIAG_DEBUG_SDM_FUN(EN_DIAG_DEBUG_MSG_ERR, g_stMsgCfg.ulCfgCnt, 0, 0x3);
                return ERR_MSP_INVALID_PARAMETER;
            }
        }
    }

    return ulRst;
}

/*
 * Function Name: diag_MsgCfgProc
 * Description: 该函数用于处理CfgProcEntry传进来的消息开关命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_MsgCfgProc(VOS_UINT8 *pstReq)
{
    DIAG_CMD_LOG_CAT_CFG_CNF_STRU stCatSwtCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    DIAG_CMD_LOG_CAT_CFG_REQ_STRU *pstCatCfgReq = NULL;
    VOS_UINT32 ret;
    msp_diag_frame_info_s *pstDiagHead = NULL;
    VOS_UINT32 ulLen;
    DIAG_MSG_TRANS_STRU *pstInfo = VOS_NULL;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);
    mdrv_diag_ptr(EN_DIAG_PTR_MSG_CFG_PROC, 1, pstDiagHead->ulCmdId, 0);

    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) || (pstDiagHead->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("DataLen error, u32DataLen:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    pstCatCfgReq = (DIAG_CMD_LOG_CAT_CFG_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    /* 设置消息过滤开关到全局变量中 */
    ret = diag_CfgSetMsgSwt(pstCatCfgReq, (VOS_UINT32)(pstDiagHead->ulMsgLen - sizeof(msp_diag_data_head_s)));
    if (ret == ERR_MSP_SUCCESS) {
        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);
    }

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCatSwtCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stCatSwtCnf.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stCatSwtCnf, (VOS_UINT32)sizeof(stCatSwtCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_MSG_CFG_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);
    return ret;
}


VOS_UINT32 diag_CfgSetPrintSwt(DIAG_CMD_LOG_CAT_PRINT_REQ_STRU *pstPrintReq, VOS_UINT32 ulCfgSize)
{
    VOS_UINT32 j;
    VOS_UINT8 ucLevelFilter;
    errno_t err;

    if ((ulCfgSize == 0) || ((ulCfgSize % sizeof(DIAG_CMD_LOG_CAT_PRINT_REQ_STRU)) != 0)) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    /* 工具的LEVEL值转换成MSP本地存储的LEVEL值 */
    /*
     * TOOL...............MSP   ...... PS
     * 0x40000000对应0x40  对应 1 (ERROR);
     * 0x20000000对应0x20  对应 2 (WARNING);
     * 0x10000000对应0x10  对应 3 (NORMAL);
     * 0x08000000对应0x08  对应 4 (INFO)
     */
    if (pstPrintReq->ulModuleId == DIAG_CFG_PRINT_TOTAL_MODULE) {
        /* 璁剧疆PRINT鏃堕鍏堥噸缃墍鏈夋ā鍧楄缃?? */
        err = memset_s(g_PrintModuleCfg, (VOS_UINT32)sizeof(g_PrintModuleCfg), 0, sizeof(g_PrintModuleCfg));
        if (err != EOK) {
            diag_error("memcpy fail:%x\n", err);
        }

        /* 设置打印总开关 */
        ucLevelFilter = DIAG_GET_PRINT_CFG_SWT(pstPrintReq->ulLevelFilter);
        g_PrintTotalCfg = ucLevelFilter;
    } else {
        /* 重置PRINT总开关0xFF模块 */
        g_PrintTotalCfg = DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE;

        /* 遍历某Category的开关配置项列表，查找对应的配置项进行设置 */
        for (j = 0; j < ulCfgSize / sizeof(DIAG_CMD_LOG_CAT_PRINT_REQ_STRU); j++) {
            if (DIAG_CFG_MODULE_IS_INVALID((VOS_INT32)((pstPrintReq + j)->ulModuleId))) {
                continue;
            }

            ucLevelFilter = DIAG_GET_PRINT_CFG_SWT((pstPrintReq + j)->ulLevelFilter);

            g_PrintModuleCfg[(pstPrintReq + j)->ulModuleId - VOS_PID_DOPRAEND] = ucLevelFilter;
        }
    }
    return ERR_MSP_SUCCESS;
}

/*
 * Function Name: diag_PrintCfgProc
 * Description: 该函数用于处理CfgProcEntry传进来的打印开关命令
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_PrintCfgProc(VOS_UINT8 *pstReq)
{
    VOS_UINT32 ret;
    msp_diag_frame_info_s *pstDiagHead = NULL;
    DIAG_CMD_LOG_CAT_PRINT_REQ_STRU *pstPrintSwtReq = NULL;
    DIAG_CMD_LOG_CAT_PRINT_CNF_STRU stPrintSwtCnf = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    VOS_UINT32 ulLen;
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    mdrv_diag_ptr(EN_DIAG_PTR_PRINT_CFG_PROC, 1, pstDiagHead->ulCmdId, 0);

    if ((pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s)) || (pstDiagHead->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("DataLen error, u32DataLen:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }

    pstPrintSwtReq = (DIAG_CMD_LOG_CAT_PRINT_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    /* 设置打印开关到全局变量中 */
    ret = diag_CfgSetPrintSwt(pstPrintSwtReq, (VOS_UINT32)(pstDiagHead->ulMsgLen - sizeof(msp_diag_data_head_s)));
    if (ret == ERR_MSP_SUCCESS) {
        DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
        DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);
    }

DIAG_ERROR:

    DIAG_MSG_COMMON_PROC(stDiagInfo, stPrintSwtCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    stPrintSwtCnf.ulRc = ret;

    ret = DIAG_MsgReport(&stDiagInfo, &stPrintSwtCnf, (VOS_UINT32)sizeof(stPrintSwtCnf));

    mdrv_diag_ptr(EN_DIAG_PTR_PRINT_CFG_FAIL_CNF, 1, pstDiagHead->ulCmdId, ret);

    return ret;
}


VOS_UINT64 diag_GetFrameTime(VOS_VOID)
{
    VOS_UINT32 ultimelow = 0;
    VOS_UINT32 ultimehigh = 0;
    VOS_UINT64 ulFrameCount;
    VOS_INT32 ret;

    ret = mdrv_timer_get_accuracy_timestamp(&ultimehigh, &ultimelow);
    if (ret != ERR_MSP_SUCCESS) {
        ulFrameCount = 0;
    } else {
        ulFrameCount = ((VOS_UINT64)ultimehigh << 0x20) | ((VOS_UINT64)ultimelow);
    }
    return ulFrameCount;
}

/*
 * Function Name: diag_GetTimeStampInitValue
 * Description: 该函数处理hidis获取单板中TL和Gu的时间戳初始值请求
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 * 2.c64416         2014-11-18  适配新的诊断架构
 */
VOS_UINT32 diag_GetTimeStampInitValue(VOS_UINT8 *pstReq)
{
    VOS_UINT ret = ERR_MSP_SUCCESS;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    DIAG_TIMESTAMP_CNF_STRU timestampCnf = { 0 };
    msp_diag_frame_info_s *pstDiagHead = NULL;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    mdrv_diag_ptr(EN_DIAG_PTR_GET_TIME_STAMP, 1, pstDiagHead->ulCmdId, 0);

    DIAG_MSG_COMMON_PROC(stDiagInfo, timestampCnf, pstDiagHead);

    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;

    timestampCnf.ulGuTimeStampInitValue = mdrv_timer_get_normal_timestamp();
    timestampCnf.ulTLTimeStampInitValue = diag_GetFrameTime();
    timestampCnf.ulErrcode = ret;

    /* 组包给FW回复 */
    ret = DIAG_MsgReport(&stDiagInfo, &timestampCnf, (VOS_UINT32)sizeof(timestampCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_GET_TIME_STAMP_CNF, 1, pstDiagHead->ulCmdId, ret);
    return (VOS_UINT32)ret;
}
#ifdef DIAG_SYSTEM_5G
/*
 * Function Name: diag_TransCfgProc
 * Description: deal witch trans msg cfg
 * Input: pstReq 待处理数据
 * Output: None
 * Return: VOS_UINT32
 */
VOS_UINT32 diag_TransCfgProc(VOS_UINT8 *pstReq)
{
    DIAG_CMD_LOG_CAT_TRANS_CNF_STRU stTransCnf = { 0 };
    DIAG_CMD_LOG_TRANS_REQ_STRU *pstTransReq = NULL;
    msp_diag_frame_info_s *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;
    VOS_UINT32 ret;
    VOS_UINT32 ulLen;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);
    mdrv_diag_ptr(EN_DIAG_PTR_CFG_TRANS, 1, pstDiagHead->ulCmdId, 0);

    if (pstDiagHead->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(DIAG_CMD_LOG_TRANS_REQ_STRU)) {
        diag_error("len error:0x%x\n", pstDiagHead->ulMsgLen);
        ret = ERR_MSP_INALID_LEN_ERROR;
        goto DIAG_ERROR;
    }

    pstTransReq = (DIAG_CMD_LOG_TRANS_REQ_STRU *)(pstReq + DIAG_MESSAGE_DATA_HEADER_LEN);

    g_ulTransCfg = pstTransReq->ulSwitch;

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);
    DIAG_MSG_SEND_CFG_TO_NRM(ulLen, pstDiagHead, pstInfo, ret);

    ret = ERR_MSP_SUCCESS;
DIAG_ERROR:
    DIAG_MSG_COMMON_PROC(stDiagInfo, stTransCnf, pstDiagHead);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stTransCnf.ulRc = ret;

    /* 组包给FW回复 */
    ret = DIAG_MsgReport(&stDiagInfo, &stTransCnf, (VOS_UINT32)sizeof(stTransCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_CFG_TRANS_CNF, 1, pstDiagHead->ulCmdId, ret);
    return (VOS_UINT32)ret;
}
VOS_UINT32 diag_UserPlaneCfgProc(VOS_UINT8 *pstReq)
{
    DIAG_CMD_LOG_CAT_USERPLANE_CNF_STRU stUserPlaneCnf = { 0 };
    msp_diag_frame_info_s *pstDiagHead = NULL;
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };
    DIAG_MSG_TRANS_STRU *pstInfo = NULL;
    VOS_UINT32 ret;
    VOS_UINT32 ulLen;

    pstDiagHead = (msp_diag_frame_info_s *)(pstReq);

    mdrv_diag_ptr(EN_DIAG_PTR_CFG_USERPLANE, 1, pstDiagHead->ulCmdId, 0);

    DIAG_MSG_ACORE_CFG_PROC(ulLen, pstDiagHead, pstInfo, ret);

    return ret;
DIAG_ERROR:
    DIAG_MSG_COMMON_PROC(stDiagInfo, stUserPlaneCnf, pstDiagHead);
    stDiagInfo.ulMsgType = DIAG_MSG_TYPE_MSP;
    stUserPlaneCnf.ulRc = ret;

    /* 组包给FW回复 */
    ret = DIAG_MsgReport(&stDiagInfo, &stUserPlaneCnf, (VOS_UINT32)sizeof(stUserPlaneCnf));
    mdrv_diag_ptr(EN_DIAG_PTR_CFG_USERPLANE_CNF, 1, pstDiagHead->ulCmdId, ret);
    return (VOS_UINT32)ret;
}
#endif

VOS_UINT32 diag_GetMiniLogCfg(VOS_VOID)
{
    return g_miniLogCfg;
}

VOS_UINT32 diag_MinilogCfgProc(VOS_UINT8 *req)
{
    VOS_UINT32 ret;
    VOS_UINT32 len;
    MSP_DIAG_CNF_INFO_STRU diag_info = { 0 };
    msp_diag_frame_info_s *diag_head = VOS_NULL_PTR;
    DIAG_MSG_TRANS_STRU *trans_info = VOS_NULL_PTR;
    diag_cmd_minilog_cnf_s minilog_cnf;
    diag_cmd_minilog_req_s *minilog_req = VOS_NULL_PTR;

    diag_info("Diag_test, acore receive comm log cfg\n");

    diag_head = (msp_diag_frame_info_s *)req;

    if ((diag_head->ulMsgLen < sizeof(msp_diag_data_head_s) + sizeof(diag_cmd_minilog_req_s)) ||
        (diag_head->ulMsgLen > DIAG_MEM_ALLOC_LEN_MAX)) {
        diag_error("DataLen error, u32DataLen: 0x%x\n", diag_head->ulMsgLen);
        ret = ERR_MSP_INVALID_PARAMETER;
        goto DIAG_ERROR;
    }
    minilog_req = (diag_cmd_minilog_req_s *)(req + DIAG_MESSAGE_DATA_HEADER_LEN);
    g_miniLogCfg = minilog_req->cfg;

    DIAG_MSG_ACORE_CFG_PROC(len, diag_head, trans_info, ret);
    DIAG_MSG_SEND_CFG_TO_NRM(len, diag_head, trans_info, ret);

    return ret;

DIAG_ERROR:
    DIAG_MSG_COMMON_PROC(diag_info, minilog_cnf, diag_head);
    diag_info.ulMsgType = DIAG_MSG_TYPE_MSP;
    minilog_cnf.rc = ret;

    ret = DIAG_MsgReport(&diag_info, &minilog_cnf, (VOS_UINT32)sizeof(minilog_cnf));
    return ret;
}

VOS_VOID diag_OpenAllSwitch(VOS_VOID)
{
    VOS_UINT32 ulIndex;
    errno_t err;

    /* set init&connect */
    g_ulDiagCfgInfo = g_ulDiagCfgInfo | DIAG_CFG_INIT | DIAG_CFG_CONN;

    /* open print */
    g_PrintTotalCfg = ((VOS_UINT32)1 << (0x7 - PS_LOG_LEVEL_ERROR)) | ((VOS_UINT32)1 << (0x7 - PS_LOG_LEVEL_WARNING)) |
        ((VOS_UINT32)1 << (0x7 - PS_LOG_LEVEL_NORMAL)) | ((VOS_UINT32)1 << (0x7 - PS_LOG_LEVEL_INFO));

    /* open event */
    g_ulDiagCfgInfo = g_ulDiagCfgInfo | DIAG_CFG_EVT;
    err = memset_s(g_EventModuleCfg, sizeof(g_EventModuleCfg), 1, sizeof(g_EventModuleCfg));
    if (err != EOK) {
        diag_error("memcpy fail:%x\n", err);
    }

    /* open air */
    g_ulDiagCfgInfo = g_ulDiagCfgInfo | DIAG_CFG_GU_AIR | DIAG_CFG_LT_AIR;

    /* open layer */
    for (ulIndex = 0; ulIndex < sizeof(g_ALayerSrcModuleCfg) / sizeof(g_ALayerSrcModuleCfg[0]); ulIndex++) {
        g_ALayerSrcModuleCfg[ulIndex] = DIAG_CFG_SWT_OPEN;
    }
    for (ulIndex = 0; ulIndex < sizeof(g_CLayerSrcModuleCfg) / sizeof(g_CLayerSrcModuleCfg[0]); ulIndex++) {
        g_CLayerSrcModuleCfg[ulIndex] = DIAG_CFG_SWT_OPEN;
    }
    for (ulIndex = 0; ulIndex < sizeof(g_NrmLayerSrcModuleCfg) / sizeof(g_NrmLayerSrcModuleCfg[0]); ulIndex++) {
        g_NrmLayerSrcModuleCfg[ulIndex] = DIAG_CFG_SWT_OPEN;
    }

    for (ulIndex = 0; ulIndex < sizeof(g_ALayerDstModuleCfg) / sizeof(g_ALayerDstModuleCfg[0]); ulIndex++) {
        g_ALayerDstModuleCfg[ulIndex] = DIAG_CFG_SWT_OPEN;
    }
    for (ulIndex = 0; ulIndex < sizeof(g_CLayerDstModuleCfg) / sizeof(g_CLayerDstModuleCfg[0]); ulIndex++) {
        g_CLayerDstModuleCfg[ulIndex] = DIAG_CFG_SWT_OPEN;
    }
    for (ulIndex = 0; ulIndex < sizeof(g_NrmLayerDstModuleCfg) / sizeof(g_NrmLayerDstModuleCfg[0]); ulIndex++) {
        g_NrmLayerDstModuleCfg[ulIndex] = DIAG_CFG_SWT_OPEN;
    }
}

VOS_UINT8 *diag_get_event_cfg(VOS_VOID)
{
    return g_EventModuleCfg;
}

