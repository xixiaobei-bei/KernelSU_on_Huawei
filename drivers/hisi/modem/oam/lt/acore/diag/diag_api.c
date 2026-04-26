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
#include "diag_api.h"
#include <vos.h>
#include <mdrv.h>
#include <msp.h>
#include <nv_stru_lps.h>
#include <nv_id_tlas.h>
#include <nv_id_drv.h>
#include <nv_stru_drv.h>
#include <soc_socp_adapter.h>
#include "diag_common.h"
#include "diag_cfg.h"
#include "diag_debug.h"
#include "diag_message.h"


#define THIS_FILE_ID MSP_FILE_ID_DIAG_API_C
/*
 * 2 Declare the Global Variable
 */
VOS_UINT32 g_ulDiagProfile = DIAGLOG_POWER_LOG_PROFILE_OFF;

mdrv_diag_layer_msg_match_func g_pLayerMatchFunc = VOS_NULL;
mdrv_diag_layer_msg_notify_func g_pLayerMatchNotifyFunc = VOS_NULL;


unsigned int mdrv_diag_get_conn_state(void)
{
    return (VOS_UINT32)((DIAG_IS_CONN_ON) ? 1 : 0);
}

/*
 * Function Name: mdrv_diag_log_port_switch
 * Description: 端口切换函数(提供给NAS，在at^logport中调用)
 * Input: VOS_UINT32 ulPhyPort: 待切换的物理端口类型(USB or VCOM)
 * VOS_BOOL ulEffect: 是否立即生效
 * Return: VOS_OK:success
 * VOS_ERR: error
 * ERR_MSP_AT_CHANNEL_BUSY:diag建链状态下不允许从USB切换到VCOM
 */
unsigned int mdrv_diag_log_port_switch(unsigned int phy_port, unsigned int effect)
{
    VOS_UINT32 enLogPort = 0; // 切换之前的logport
    VOS_UINT32 DiagStatus;    // 工具连接状态
    VOS_UINT32 ulRet;

    (VOS_VOID)mdrv_ppm_query_log_port(&enLogPort); /* 获取切换之前的端口类型 */

    DiagStatus = mdrv_diag_get_conn_state(); /* 获取diag的连接状态 */
    /* diag连接状态下，不允许从USB切换到VCOM */
    if ((DiagStatus == 1) && (enLogPort == DIAG_CPM_OM_PORT_TYPE_USB) && (phy_port == DIAG_CPM_OM_PORT_TYPE_VCOM)) {
        diag_crit("diag connected, USB does not allowed to change to vcom\n");
        return ERR_MSP_AT_CHANNEL_BUSY;
    } else {
        ulRet = mdrv_ppm_log_port_switch(phy_port, effect);
    }
    return ulRet;
}


VOS_UINT32 diag_GetLayerMsgCfg(VOS_UINT32 ulCatId, VOS_UINT32 ulMsgId)
{
    DIAG_CFG_LOG_CAT_MSG_CFG_STRU *pstItemCfg = NULL;
    VOS_UINT32 i;

    for (i = 0; i < g_stMsgCfg.ulCfgCnt; i++) {
        pstItemCfg = (g_stMsgCfg.astMsgCfgList + i);

        if (ulMsgId == pstItemCfg->ulId) {
            if (pstItemCfg->ulSwt == DIAG_CFG_SWT_CLOSE) {
                return ERR_MSP_CFG_LOG_NOT_ALLOW;
            } else if (pstItemCfg->ulSwt == DIAG_CFG_SWT_OPEN) {
                return ERR_MSP_SUCCESS;
            } else {
                return ERR_MSP_DIAG_MSG_CFG_NOT_SET;
            }
        }
    }

    return ERR_MSP_DIAG_MSG_CFG_NOT_SET;
}

VOS_UINT32 diag_GetLayerSrcCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 ulOffset;

    if (ulModuleId == DOPRA_PID_TIMER) {
        return ERR_MSP_SUCCESS;
    }

    if (DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);
        if (g_ALayerSrcModuleCfg[ulOffset] == DIAG_CFG_SWT_OPEN) {
            return ERR_MSP_SUCCESS;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);
        if (g_CLayerSrcModuleCfg[ulOffset] == DIAG_CFG_SWT_OPEN) {
            return ERR_MSP_SUCCESS;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId);
        if (g_NrmLayerSrcModuleCfg[ulOffset] == DIAG_CFG_SWT_OPEN) {
            return ERR_MSP_SUCCESS;
        }
    }

    return ERR_MSP_CFG_LOG_NOT_ALLOW;
}

VOS_UINT32 diag_GetLayerDstCfg(VOS_UINT32 ulModuleId)
{
    VOS_UINT32 ulOffset;

    if (DIAG_CFG_LAYER_MODULE_IS_ACORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_ACORE_OFFSET(ulModuleId);
        if (g_ALayerDstModuleCfg[ulOffset] == DIAG_CFG_SWT_OPEN) {
            return ERR_MSP_SUCCESS;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_CCORE(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_CCORE_OFFSET(ulModuleId);
        if (g_CLayerDstModuleCfg[ulOffset] == DIAG_CFG_SWT_OPEN) {
            return ERR_MSP_SUCCESS;
        }
    } else if (DIAG_CFG_LAYER_MODULE_IS_NRM(ulModuleId)) {
        ulOffset = DIAG_CFG_LAYER_MODULE_NRM_OFFSET(ulModuleId);
        if (g_NrmLayerDstModuleCfg[ulOffset] == DIAG_CFG_SWT_OPEN) {
            return ERR_MSP_SUCCESS;
        }
    }

    return ERR_MSP_CFG_LOG_NOT_ALLOW;
}


VOS_UINT32 diag_GetLayerCfg(VOS_UINT32 ulSrcModuleId, VOS_UINT32 ulDstModuleId, VOS_UINT32 ulMsgId)
{
    VOS_UINT32 ret;
    VOS_UINT32 ret2;
    VOS_UINT32 ulMsgCfg;

    /* 先判断消息ID过滤功能是否打开，如果有，则根据消息ID过滤 */
    ulMsgCfg = diag_GetLayerMsgCfg(DIAG_CMD_LOG_CATETORY_LAYER_ID, ulMsgId);
    if (ulMsgCfg == ERR_MSP_DIAG_MSG_CFG_NOT_SET) {
        ret = diag_GetLayerSrcCfg(ulSrcModuleId);
        ret2 = diag_GetLayerDstCfg(ulDstModuleId);
        if ((ret != ERR_MSP_SUCCESS) && (ret2 != ERR_MSP_SUCCESS)) {
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        } else {
            return ERR_MSP_SUCCESS;
        }
    } else if (ulMsgCfg == ERR_MSP_SUCCESS) {
        return ERR_MSP_SUCCESS;
    } else {
        return ERR_MSP_CFG_LOG_NOT_ALLOW;
    }
}


VOS_UINT32 diag_GetPrintCfg(VOS_UINT32 ulModuleId, VOS_UINT32 ulLevel)
{
    VOS_UINT32 ulLevelFilter;
    VOS_UINT32 ulTmp;

    if ((ulLevel == PS_LOG_LEVEL_OFF) || (ulLevel >= PS_LOG_LEVEL_BUTT)) {
        diag_LNR(EN_DIAG_LNR_LOG_LOST, ulLevel, 0xAAAAAAAA);
        return ERR_MSP_CFG_LOG_NOT_ALLOW;
    }

    /* 将协议栈的LEVEL值转换成MSP与HSO之间的LEVEL值 */
    /*
     * TOOL...............MSP   ...... PS
     * 0x40000000对应0x40  对应 1 (ERROR);
     * 0x20000000对应0x20  对应 2 (WARNING);
     * 0x10000000对应0x10  对应 3 (NORMAL);
     * 0x08000000对应0x08  对应 4 (INFO)
     */
    ulLevelFilter = ((VOS_UINT32)1 << (0x7 - ulLevel));

    ulTmp = (ulLevelFilter << 0x10) | g_PrintTotalCfg;

    /* 打印总开关模块打开情况 */
    if (g_PrintTotalCfg != DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE) {
        if (ulLevelFilter & g_PrintTotalCfg) {
            return ERR_MSP_SUCCESS;
        } else {
            diag_LNR(EN_DIAG_LNR_LOG_LOST, ulTmp, ulModuleId);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
    } else {
        /* 获取模块打印开关状态 */
        if (DIAG_CFG_MODULE_IS_INVALID((VOS_INT32)ulModuleId)) {
            diag_LNR(EN_DIAG_LNR_LOG_LOST, ulModuleId, 0xBBBBBBBB);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }

        ulTmp = (ulLevelFilter << 0x10) | g_PrintModuleCfg[ulModuleId - VOS_PID_DOPRAEND];

        if (ulLevelFilter & g_PrintModuleCfg[ulModuleId - VOS_PID_DOPRAEND]) {
            return ERR_MSP_SUCCESS;
        } else {
            diag_LNR(EN_DIAG_LNR_LOG_LOST, ulTmp, ulModuleId);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
    }
}
#ifdef DIAG_SYSTEM_5G
VOS_UINT32 diag_GetPrintPowerOnCfg(VOS_UINT32 ulLevel)
{
    if (((g_ulDiagProfile == DIAGLOG_POWER_LOG_PROFILE_SIMPLE) && (ulLevel <= PS_LOG_LEVEL_ERROR)) ||
        ((g_ulDiagProfile == DIAGLOG_POWER_LOG_PROFILE_NORAML) && (ulLevel <= PS_LOG_LEVEL_NORMAL)) ||
        ((g_ulDiagProfile == DIAGLOG_POWER_LOG_PROFILE_WHOLE) && (ulLevel <= PS_LOG_LEVEL_INFO))) {
        return ERR_MSP_SUCCESS;
    } else {
        return ERR_MSP_CFG_LOG_NOT_ALLOW;
    }
}

#endif

/*
 * 函 数 名: mdrv_diag_log_report
 * 功能描述: 打印上报接口
 * 输入参数: ulModuleId( 31-24:modemid,23-16:modeid,15-12:level )
 * pcFileName(上报时会把文件路径删除，只保留文件名)
 * ulLineNum(行号)
 * pszFmt(可变参数)
 * 注意事项: 由于此接口会被协议栈频繁调用，为提高处理效率，本接口内部会使用1K的局部变量保存上报的字符串信息，
 * 从而此接口对协议栈有两点限制，一是调用此接口的任务栈中的内存要至少为此接口预留1K空间；
 * 二是调用此接口输出的log不要超过1K（超出部分会自动丢弃）
 */
VOS_UINT32 mdrv_diag_log_report(VOS_UINT32 ulModuleId, VOS_UINT32 ulPid, VOS_CHAR *cFileName, VOS_UINT32 ulLineNum,
    VOS_CHAR *pszFmt, ...)
{
    VOS_UINT32 ret;
    VOS_UINT32 module;
    VOS_UINT32 ulLevel;
    va_list arg;

    /* 获取数组下标 */
    module = ulPid;

    /* 获取模块开关状态 */
    ulLevel = DIAG_GET_PRINTF_LEVEL(ulModuleId);

    if (!DIAG_IS_POLOG_ON) {
        /* 检查DIAG是否初始化且HSO是否连接上 */
        if (!DIAG_IS_CONN_ON) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ERR_MSP_NO_INITILIZATION, ulModuleId, 1);
            return ERR_MSP_NO_INITILIZATION;
        }

        ret = diag_GetPrintCfg(module, ulLevel);
        if (ret != ERR_MSP_SUCCESS) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ERR_MSP_UNAVAILABLE, ulModuleId, 0x2);
            return ERR_MSP_UNAVAILABLE;
        }
    } else {
#ifdef DIAG_SYSTEM_5G
        ret = diag_GetPrintPowerOnCfg(ulLevel);
        if (ret != ERR_MSP_SUCCESS) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ret, ulModuleId, 0x5);
            return ret;
        }
#endif
    }

    va_start(arg, pszFmt);

    ret = mdrv_diag_report_log(ulModuleId, module, cFileName, ulLineNum, pszFmt, arg);
    if (ret) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_ERR, ret, 0, 0x3);
    } else {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_PRINTFV_OK, 0, 0, 0x4);
    }

    va_end(arg);

    return ret;
}

/*
 * 函 数 名: mdrv_diag_trans_report_ex
 * 功能描述: 结构化数据上报扩展接口(比DIAG_TransReport多传入了DIAG_MESSAGE_TYPE)
 * 输入参数: mdrv_diag_trans_ind_s->module( 31-24:modemid,23-16:modeid,15-12:level,11-8:groupid )
 * mdrv_diag_trans_ind_s->ulMsgId(透传命令ID)
 * mdrv_diag_trans_ind_s->ulLength(透传信息的长度)
 * mdrv_diag_trans_ind_s->pData(透传信息)
 */
VOS_UINT32 mdrv_diag_trans_report_ex(mdrv_diag_trans_ind_s *pstData)
{
    VOS_UINT32 ret;

    if (!DIAG_IS_POLOG_ON) {
        if (!DIAG_IS_CONN_ON) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 0);
            return ERR_MSP_NO_INITILIZATION;
        }
#ifdef DIAG_SYSTEM_5G
        if (g_ulTransCfg == DIAG_CFG_SWT_CLOSE) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 1);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
#endif
    } else {
#ifdef DIAG_SYSTEM_5G
        if (g_ulDiagProfile <= DIAGLOG_POWER_LOG_PROFILE_SIMPLE) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 0x2);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
#endif
    }

    /* 检查参数合法性 */
    if ((pstData == VOS_NULL_PTR) || (pstData->pData == NULL) || (pstData->ulLength == 0)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 0x3);
        return ERR_MSP_INVALID_PARAMETER;
    }

    diag_LNR(EN_DIAG_LNR_TRANS_IND, pstData->ulMsgId, VOS_GetSlice());

    ret = mdrv_diag_report_trans((diag_trans_ind_s *)pstData);
    if (ret) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ret, 0, 0x4);
    } else {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_OK, pstData->ulMsgId, VOS_GetSlice(), 0x5);
    }

    return ret;
}


VOS_UINT32 mdrv_diag_trans_report(mdrv_diag_trans_ind_s *pstData)
{
    /* 检查DIAG是否初始化且HSO是否连接上 */
    if ((!DIAG_IS_CONN_ON) && (!DIAG_IS_POLOG_ON)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
        return ERR_MSP_NO_INITILIZATION;
    }

    /* 检查参数合法性 */
    if ((pstData == VOS_NULL_PTR) || (pstData->pData == NULL) || (pstData->ulLength == 0)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRANS_ERR, ERR_MSP_NO_INITILIZATION, 0, 0x2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    pstData->ulModule = MDRV_DIAG_GEN_MODULE_EX(DIAG_GET_MODEM_ID(pstData->ulModule),
        DIAG_GET_MODE_ID(pstData->ulModule), DIAG_MSG_TYPE_PS);

    return mdrv_diag_trans_report_ex(pstData);
}


VOS_UINT32 mdrv_diag_event_report(mdrv_diag_event_ind_s *pstEvent)
{
    VOS_UINT32 ret;
    VOS_UINT8 *event_cfg = diag_get_event_cfg();

    if (pstEvent == VOS_NULL_PTR) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ERR_MSP_INVALID_PARAMETER, 0, 0x2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (!DIAG_IS_POLOG_ON) {
        /* 检查是否允许事件上报 */
        if (!DIAG_IS_EVENT_ON) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
            return ERR_MSP_NO_INITILIZATION;
        }

        /* 融合GU的event机制后，需要判断子开关 */
        if ((DIAG_CFG_MODULE_IS_VALID(pstEvent->ulPid)) && (event_cfg[pstEvent->ulPid - VOS_PID_DOPRAEND] == 0)) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ERR_MSP_INVALID_PARAMETER, 0, 0x3);
            return ERR_MSP_INVALID_PARAMETER;
        }
    }

    ret = mdrv_diag_report_event((diag_event_ind_s *)pstEvent);
    if (ret) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_ERR, ret, 0, 0x4);
    } else {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_EVENT_OK, 0, 0, 0x5);
    }
    return ret;
}

/*
 * 函 数 名: mdrv_diag_allow_air_msg_report
 * 功能描述: 提供给协议栈,用于检测当前是否允许上报AirMsg
 * 输入参数: VOS_VOID
 * 返回值: ERR_MSP_SUCCESS = 0 当前允许上报空口消息
 * ERR_MSP_CFG_LOG_NOT_ALLOW = 203 当前不允许上报空口消息
 */
unsigned int mdrv_diag_allow_air_msg_report(void)
{
    /* 检查是否允许LT 空口上报 */
    if ((!DIAG_IS_LT_AIR_ON) && (!DIAG_IS_POLOG_ON)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
        return ERR_MSP_CFG_LOG_NOT_ALLOW;
    }
    return ERR_MSP_SUCCESS;
}


VOS_UINT32 mdrv_diag_air_report(mdrv_diag_air_ind_s *pstAir)
{
    VOS_UINT32 ret;

    /* 检查是否允许LT 空口上报 */
    if ((!DIAG_IS_LT_AIR_ON) && (!DIAG_IS_POLOG_ON)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
        return ERR_MSP_NO_INITILIZATION;
    }

    /* 检查参数合法性 */
    if ((pstAir == VOS_NULL) || (pstAir->pData == VOS_NULL) || (pstAir->ulLength == 0)) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ERR_MSP_INVALID_PARAMETER, 0, 0x2);
        return ERR_MSP_INVALID_PARAMETER;
    }

    ret = mdrv_diag_report_air((diag_air_ind_s *)pstAir);
    if (ret) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_ERR, ret, 0, 0x3);
    } else {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_AIR_OK, 0, 0, 0x4);
    }

    return ret;
}


VOS_VOID mdrv_diag_trace_report(VOS_VOID *pMsg)
{
    diag_osa_msg_head_s *pDiagMsg = (diag_osa_msg_head_s *)pMsg;
    VOS_UINT32 ret, ulSrcModule, ulDstModule, ulMsgId;

    if (pDiagMsg == VOS_NULL) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_INVALID_PARAMETER, 0, 1);
        return;
    }

    if (pDiagMsg->ulLength < LAYER_MSG_MIN_LEN) {
        return;
    }

    ulSrcModule = pDiagMsg->ulSenderPid;
    ulDstModule = pDiagMsg->ulReceiverPid;
    ulMsgId = pDiagMsg->ulMsgId;

    if (!DIAG_IS_POLOG_ON) {
        /* 检查DIAG是否初始化且HSO是否连接上 */
        if (!DIAG_IS_CONN_ON) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_NO_INITILIZATION, 0, 1);
            return;
        }

        /* 检查是否允许层间消息上报 */
        ret = diag_GetLayerCfg(ulSrcModule, ulDstModule, ulMsgId);
        if (ret) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 1);
            return;
        }
    } else {
#ifdef DIAG_SYSTEM_5G
        if (g_ulDiagProfile <= DIAGLOG_POWER_LOG_PROFILE_SIMPLE) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 0x6);
            return;
        }
#endif
    }

    ret = mdrv_diag_report_trace(pMsg, DIAG_MODEM_0);
    if (ret) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_ERR, ret, 0, 0x3);
    } else {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_CBT_API_TRACE_OK, 0, 0, 0x5);
    }
    return;
}


VOS_UINT32 mdrv_diag_layer_msg_report(VOS_VOID *pMsg)
{
    diag_osa_msg_head_s *pDiagMsg = (diag_osa_msg_head_s *)pMsg;
    diag_osa_msg_head_s *pNewMsg = NULL;
    VOS_UINT32 ret, ulSrcModule, ulDstModule, ulMsgId;

    if (pDiagMsg == VOS_NULL) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ERR_MSP_INVALID_PARAMETER, 0, 1);
        return ERR_MSP_INVALID_PARAMETER;
    }

    if (pDiagMsg->ulLength < LAYER_MSG_MIN_LEN) {
        return ERR_MSP_INVALID_PARAMETER;
    }

    ulSrcModule = pDiagMsg->ulSenderPid;
    ulDstModule = pDiagMsg->ulReceiverPid;
    ulMsgId = pDiagMsg->ulMsgId;

    if (!DIAG_IS_POLOG_ON) {
        /* 检查DIAG是否初始化且HSO是否连接上 */
        if (!DIAG_IS_CONN_ON) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ERR_MSP_NO_INITILIZATION, 0, 0x2);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }

        /* 检查是否允许层间消息上报 */
        ret = diag_GetLayerCfg(ulSrcModule, ulDstModule, ulMsgId);
        if (ret) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ERR_MSP_CFG_LOG_NOT_ALLOW, 0, 0x3);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
    }

    pNewMsg = pDiagMsg;

    /* 对外提供的消息匹配接口，可能对消息进行替换 */
    /* 返回值为过滤完成后的新消息指针，如果该指针和原指针不一致需要调用notify接口对返回指针进行释放 */
    if (g_pLayerMatchFunc != VOS_NULL) {
        pNewMsg = g_pLayerMatchFunc(pDiagMsg);
        if (pNewMsg == VOS_NULL) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_MATCH, ulSrcModule, ulDstModule, ulMsgId);
            return ERR_MSP_CFG_LOG_NOT_ALLOW;
        }
    }

    ret = mdrv_diag_report_trace(pNewMsg, DIAG_MODEM_0);
    if (ret) {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_ERR, ret, 0, 0x4);
    } else {
        DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_OK, 0, 0, 0x5);
    }

    if ((pNewMsg != pDiagMsg) && (g_pLayerMatchNotifyFunc != NULL)) {
        ret = g_pLayerMatchNotifyFunc(ulSrcModule, pNewMsg);
        if (ret) {
            DIAG_DEBUG_SDM_FUN(EN_DIAG_API_MSG_LAYER_NOTIFY, 0, 0, 0x6);
        }
    }
    return ERR_MSP_SUCCESS;
}


unsigned int mdrv_diag_layer_msg_match_func_reg(mdrv_diag_layer_msg_match_func pfunc)
{
    if (g_pLayerMatchFunc == VOS_NULL) {
        g_pLayerMatchFunc = pfunc;
        return ERR_MSP_SUCCESS;
    } else {
        return ERR_MSP_FAILURE;
    }
}
/*
 * Function Name: mdrv_diag_layer_msg_notify_func_reg
 * Description: 用于注册层间消息匹配后的notify接口(此接口不支持重复注册，多次注册返还失败)
 * 用于通知logfilter进行资源释放等操作
 * Input:
 * Output: None
 * Return: 返回值为注册结果: 0-注册成功；其他-注册失败
 */
unsigned int mdrv_diag_layer_msg_notify_func_reg(mdrv_diag_layer_msg_notify_func pfunc)
{
    if (g_pLayerMatchNotifyFunc == VOS_NULL) {
        g_pLayerMatchNotifyFunc = pfunc;
        return ERR_MSP_SUCCESS;
    } else {
        return ERR_MSP_FAILURE;
    }
}

/* 为MBB提供的errorlog功能，应推动MBB与phone拉通 */
VOS_UINT32 mdrv_diag_error_log(VOS_CHAR *cFileName, VOS_UINT32 ulFileId, VOS_UINT32 ulLine, VOS_UINT32 ulErrNo,
    VOS_VOID *pBuf, VOS_UINT32 ulLen)
{
    return 0;
}


VOS_UINT32 diag_SendMsg(VOS_UINT32 ulSenderId, VOS_UINT32 ulRecverId, VOS_UINT32 ulMsgId, VOS_UINT8 *pDta,
    VOS_UINT32 dtaSize)
{
    VOS_UINT32 ret = ERR_MSP_UNKNOWN;
    DIAG_DATA_MSG_STRU *pDataMsg = NULL;
    errno_t err;

    pDataMsg =
        (DIAG_DATA_MSG_STRU *)VOS_AllocMsg(ulSenderId, (sizeof(DIAG_DATA_MSG_STRU) + dtaSize - VOS_MSG_HEAD_LENGTH));
    if (pDataMsg != NULL) {
        pDataMsg->ulReceiverPid = ulRecverId;
        pDataMsg->ulSenderPid = ulSenderId;
        pDataMsg->ulLength = sizeof(DIAG_DATA_MSG_STRU) + dtaSize - 20;

        pDataMsg->ulMsgId = ulMsgId;
        pDataMsg->ulLen = dtaSize;
        if (dtaSize != 0) {
            err = memcpy_s(pDataMsg->pContext, pDataMsg->ulLen, pDta, dtaSize);
            if (err != EOK) {
                diag_error("memcpy fail:%x\n", err);
            }
        }

        ret = VOS_SendMsg(ulSenderId, pDataMsg);
        if (ret != VOS_OK) {
            diag_error("ulRecverId=0x%x,ulMsgId=0x%x,dtaSize=0x%x,ret=0x%x\n", ulRecverId, ulMsgId, dtaSize, ret);
        } else {
            ret = ERR_MSP_SUCCESS;
        }
    }

    return ret;
}

unsigned int mdrv_diag_get_log_level(unsigned int pid)
{
    VOS_UINT8 level;

    /* 判断模块ID是否在CCPU支持的PS范围内 */
    if ((pid >= VOS_PID_DOPRAEND) && (pid < VOS_PID_BUTT)) {
        if (g_PrintTotalCfg == DIAG_CFG_PRINT_TOTAL_MODULE_SWT_NOT_USE) {
            level = g_PrintModuleCfg[pid - VOS_PID_DOPRAEND];
        } else {
            level = (VOS_UINT8)g_PrintTotalCfg;
        }

        /* level中存储的值(0|ERROR|WARNING|NORMAL|INFO|0|0|0) bit 6-3 分别表示ERROR-INFO */
        if (level & 0x08) {
            return PS_LOG_LEVEL_INFO;
        } else if (level & 0x10) {
            return PS_LOG_LEVEL_NORMAL;
        } else if (level & 0x20) {
            return PS_LOG_LEVEL_WARNING;
        } else if (level & 0x40) {
            return PS_LOG_LEVEL_ERROR;
        } else {
            return PS_LOG_LEVEL_OFF;
        }
    }

    return PS_LOG_LEVEL_OFF;
}

/*
 * Function Name: diag_FailedCmdCnf
 * Description: MSP处理失败的诊断命令应答处理
 * Input: pData     诊断命令请求的内容
 * ulErrcode 给工具返回的错误码
 * Output: None
 * Return: VOS_UINT32
 * History:
 * 1.c64416      2016-02-15  Draft Enact
 */
VOS_UINT32 diag_FailedCmdCnf(msp_diag_frame_info_s *pData, VOS_UINT32 ulErrcode)
{
    DIAG_COMM_CNF_STRU stCnfCmd = { 0 };
    MSP_DIAG_CNF_INFO_STRU stDiagInfo = { 0 };

    mdrv_diag_ptr(EN_DIAG_PTR_FAIL_CMD_CNF, 1, pData->ulCmdId, ulErrcode);

    DIAG_MSG_COMMON_PROC(stDiagInfo, stCnfCmd, pData);

    stDiagInfo.ulMsgType = pData->stID.pri4b;

    stCnfCmd.ulRet = ulErrcode;

    return DIAG_MsgReport(&stDiagInfo, &stCnfCmd, sizeof(stCnfCmd));
}

/*
 * Function Name: diag_IsPowerOnLogOpen
 * Description: 获取协议栈的开机log上报功能是否打开
 * Input: None
 * Output: None
 * Return: VOS_TRUE:yes ; VOS_FALSE:no
 */
VOS_BOOL diag_IsPowerOnLogOpen(VOS_VOID)
{
#ifdef DIAG_SYSTEM_5G
    DRV_NV_POWER_ON_LOG_SWITCH_STRU stNVPowerOnLog = {};
    VOS_UINT32 ulRet;
    VOS_UINT32 cBufUsable;

    /* read buff use able */
    cBufUsable = mdrv_diag_shared_mem_read(POWER_ON_LOG_BUFF);
    if ((cBufUsable == ERR_MSP_FAILURE) || (!cBufUsable)) {
        diag_crit("open log buff is invalid, ret:0x%x\n", cBufUsable);
        g_ulDiagProfile = DIAGLOG_POWER_LOG_PROFILE_OFF;
        return VOS_FALSE;
    }

    /* read power on log swtich */
    ulRet = mdrv_nv_read(NV_ID_DRV_DIAG_POWER_LOG, &stNVPowerOnLog, sizeof(stNVPowerOnLog));
    if (ulRet) {
        diag_error("read power log nv fail, ret:0x%x\n", ulRet);
        g_ulDiagProfile = DIAGLOG_POWER_LOG_PROFILE_OFF;
        return VOS_FALSE;
    }

    if (!stNVPowerOnLog.cMasterSwitch) {
        diag_error("power on log is closed\n");
        g_ulDiagProfile = DIAGLOG_POWER_LOG_PROFILE_OFF;
        return VOS_FALSE;
    }

    /* set profile */
    g_ulDiagProfile = stNVPowerOnLog.cswACPUDiag;
    return VOS_TRUE;
#else
    VOS_UINT32 ulRet;
    VOS_INT32 stPowerOnLogA;
    NV_POWER_ON_LOG_SWITCH_STRU stNVPowerOnLog = {};

    stPowerOnLogA = mdrv_diag_shared_mem_read(POWER_ON_LOG_A);
    if (stPowerOnLogA) {
        ulRet = mdrv_nv_read(EN_NV_ID_POWER_ON_LOG_SWITCH, &stNVPowerOnLog, sizeof(stNVPowerOnLog));
        if ((ulRet == ERR_MSP_SUCCESS) && (stNVPowerOnLog.cPowerOnlogC)) {
            return VOS_TRUE;
        }
        return VOS_FALSE;
    }

    return VOS_FALSE;
#endif
}

EXPORT_SYMBOL(mdrv_diag_log_port_switch);

