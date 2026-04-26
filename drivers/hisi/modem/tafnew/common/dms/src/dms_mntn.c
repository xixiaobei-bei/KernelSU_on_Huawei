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

#include "dms_mntn.h"
#include "mn_comm_api.h"
#include "dms_debug.h"

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_MNTN_C

struct DMS_MntnExcLogInfo *g_dmsExcAddr = VOS_NULL_PTR;
struct DMS_MntnInfo g_dmsMntnInfo = {0};

/* 端口和stats type与stats id映射表，DMS_COM_ERR_ID_MAX表示该端口不存在stats type对应的stats id */
const VOS_UINT32 g_dmsMntnStatsTbl[DMS_MNTN_DEV_TYPE_NUM][DMS_STATS_TYPE_MAX] = {
    {
        /* APPCOM */
        DMS_APP_OPEN_ERR, DMS_APP_REG_READY_CB_ERR, DMS_APP_READY_CB, DMS_APP_REG_RD_CB_ERR,
        DMS_APP_RD_CB, DMS_APP_REG_WRT_CB_ERR, DMS_APP_WRT_CB, DMS_APP_USER_DATA_ERR,
        DMS_APP_WRT_ASYNC, DMS_APP_WRT_ASYNC_ERR, DMS_APP_RETURN_BUFF_ERR, DMS_APP_RD_BUFF_ERR
    }, {
        /* ACM */
        DMS_ACM_OPEN_ERR, DMS_ACM_REG_READY_CB_ERR, DMS_ACM_READY_CB, DMS_ACM_REG_RD_CB_ERR,
        DMS_ACM_RD_CB, DMS_ACM_REG_WRT_CB_ERR, DMS_ACM_WRT_CB, DMS_ACM_USER_DATA_ERR,
        DMS_ACM_WRT_ASYNC, DMS_ACM_WRT_ASYNC_ERR, DMS_ACM_RETURN_BUFF_ERR, DMS_ACM_RD_BUFF_ERR
    }, {
        /* NCM CTRL */
        DMS_NCM_CTRL_OPEN_ERR, DMS_NCM_CTRL_REG_READY_CB_ERR, DMS_NCM_CTRL_READY_CB, DMS_NCM_CTRL_REG_RD_CB_ERR,
        DMS_NCM_CTRL_RD_CB, DMS_NCM_CTRL_REG_WRT_CB_ERR, DMS_NCM_CTRL_WRT_CB, DMS_NCM_CTRL_USER_DATA_ERR,
        DMS_NCM_CTRL_WRT_ASYNC, DMS_NCM_CTRL_WRT_ASYNC_ERR, DMS_NCM_CTRL_RETURN_BUFF_ERR, DMS_NCM_CTRL_RD_DATA_NULL
    }, {
        /* UART */
        DMS_UART_OPEN_ERR, DMS_UART_REG_READY_CB_ERR, DMS_UART_READY_CB, DMS_UART_REG_RD_CB_ERR,
        DMS_UART_RD_CB, DMS_COM_ERR_ID_MAX, DMS_COM_ERR_ID_MAX, DMS_UART_USER_DATA_ERR,
        DMS_COM_ERR_ID_MAX, DMS_COM_ERR_ID_MAX, DMS_UART_RETURN_BUFF_ERR, DMS_UART_RD_BUFF_ERR
    }, {
        /* SOCK */
        DMS_SOCK_OPEN_ERR, DMS_SOCK_REG_READY_CB_ERR, DMS_SOCK_READY_CB, DMS_SOCK_REG_RD_CB_ERR,
        DMS_SOCK_RD_CB, DMS_SOCK_REG_WRT_CB_ERR, DMS_SOCK_WRT_CB, DMS_SOCK_USER_DATA_ERR,
        DMS_SOCK_WRT_ASYNC, DMS_SOCK_WRT_ASYNC_ERR, DMS_SOCK_RETURN_BUFF_ERR, DMS_SOCK_RD_DATA_NULL
    }, {
        /* MODEM */
        DMS_MDM_OPEN_ERR, DMS_MDM_REG_READY_CB_ERR, DMS_MDM_READY_CB, DMS_MDM_REG_RD_CB_ERR,
        DMS_MDM_RD_CB, DMS_MDM_REG_FREE_CB_ERR, DMS_MDM_FREE_CB, DMS_MDM_USER_DATA_ERR,
        DMS_MDM_WRT_ASYNC, DMS_MDM_WRT_ASYNC_ERR, DMS_MDM_RETURN_BUFF_ERR, DMS_MDM_RD_BUFF_ERR
    }, {
        /* HSUART */
        DMS_HSUART_OPEN_ERR, DMS_HSUART_REG_READY_CB_ERR, DMS_HSUART_READY_CB, DMS_HSUART_REG_RD_CB_ERR,
        DMS_HSUART_RD_CB, DMS_HSUART_REG_FREE_CB_ERR, DMS_HSUART_FREE_CB, DMS_HSUART_USER_DATA_ERR,
        DMS_HSUART_WRT_ASYNC, DMS_HSUART_WRT_ASYNC_ERR, DMS_HSUART_RETURN_BUFF_ERR, DMS_HSUART_RD_BUFF_ERR
    }
};

struct DMS_MntnInfo *DMS_MNTN_GetDmsMntnInfo(VOS_VOID)
{
    return &g_dmsMntnInfo;
}

STATIC struct DMS_MntnExcLogInfo *DMS_MNTN_GetDmsExcAddr(VOS_VOID)
{
    return g_dmsExcAddr;
}

STATIC VOS_VOID DMS_MNTN_SetDmsExcAddr(struct DMS_MntnExcLogInfo *dmsExcAddr, VOS_UINT32 bufSize)
{
    g_dmsExcAddr = dmsExcAddr;
}

STATIC VOS_UINT32 DMS_MNTN_GetDmsStatsId(VOS_UINT32 devType, DMS_MntnStatsTypeUint32 statsType)
{
    if (devType >= DMS_MNTN_DEV_TYPE_NUM || statsType >= DMS_STATS_TYPE_MAX) {
        DMS_LOGE("devType:%d statsType:%d invalid", devType, statsType);
        return DMS_COM_ERR_ID_MAX;
    }

    return g_dmsMntnStatsTbl[devType][statsType];
}

VOS_VOID DMS_MNTN_IncAppStatsInfo(DMS_PortIdUint16 portId, DMS_MntnAppStatsIdUint32 statsId)
{
    struct DMS_MntnInfo *statsInfo = VOS_NULL_PTR;

    if (statsId >= DMS_APP_ERR_ID_MAX || portId >= DMS_PORT_PCUI) {
        DMS_LOGE("portid:%d statsId:%d invalid", portId, statsId);
        return;
    }

    statsInfo = DMS_MNTN_GetDmsMntnInfo();
    statsInfo->appstatsInfo[portId][statsId]++;
}

VOS_VOID DMS_MNTN_IncComStatsInfo(DMS_MntnComPortStatsIdUint32 statsId)
{
    struct DMS_MntnInfo *statsInfo = VOS_NULL_PTR;

    if (statsId >= DMS_COM_ERR_ID_MAX) {
        DMS_LOGE("statsId %d invalid", statsId);
        return;
    }

    statsInfo = DMS_MNTN_GetDmsMntnInfo();
    statsInfo->comStatsInfo[statsId]++;
}

VOS_VOID DMS_MNTN_IncStatsInfo(DMS_PortIdUint16 portId, DMS_MntnStatsTypeUint32 statsType)
{
    struct DMS_MntnInfo *mntnInfo = VOS_NULL_PTR;
    VOS_UINT32          *statsInfo = VOS_NULL_PTR;
    VOS_UINT32           devType;
    VOS_UINT32           statsId;

    if (statsType >= DMS_STATS_TYPE_MAX || portId >= DMS_PORT_BUTT) {
        DMS_LOGE("portid:%d statsType:%d invalid", portId, statsType);
        return;
    }

    mntnInfo = DMS_MNTN_GetDmsMntnInfo();

    if (portId < DMS_APP_PORT_SIZE) { /* 都属于APPVCOM */
        devType = 0;
        statsInfo = mntnInfo->appstatsInfo[portId];
    } else {
        if (portId <= DMS_PORT_PCUI2) { /* PCUI/CTRL/PCUI2都属于ACM */
            devType = 1;
        } else {
            devType = portId - DMS_PORT_PCUI2 + 1;
        }
        statsInfo = mntnInfo->comStatsInfo;
    }

    statsId = DMS_MNTN_GetDmsStatsId(devType, statsType);
    if (statsId >= DMS_COM_ERR_ID_MAX) {
        DMS_PORT_LOGE(portId, "statsType:%d statsId:%d invalid", statsType, statsId);
        return;
    }

    statsInfo[statsId]++;
}

VOS_VOID DMS_MNTN_TraceReport(DMS_PortIdUint16 portId, DMS_IntraMsgIdUint32 msgId,
    VOS_UINT8 *data, VOS_UINT32 len)
{
    struct DMS_MntnMsg *mntnMsg = VOS_NULL_PTR;
    VOS_UINT32          msgLen;
    errno_t             memResult;

    if ((data == VOS_NULL_PTR) && (len != 0)) {
        DMS_ERROR_LOG("DMS_MNTN_TraceReport:para invalid.");
        return;
    }

    if (portId >= DMS_PORT_BUTT) {
        DMS_ERROR_LOG1("DMS_MNTN_TraceReport:portid invalid:%d.", portId);
        return;
    }

    /* 申请内存 */
    if (len == 0) {
        msgLen = sizeof(*mntnMsg);
    } else {
        msgLen = sizeof(*mntnMsg) + len;
    }

    mntnMsg = (struct DMS_MntnMsg *)PS_MEM_ALLOC(PS_PID_DMS, msgLen);

    if (mntnMsg == VOS_NULL_PTR) {
        DMS_ERROR_LOG("DMS_MNTN_TraceReport:Alloc mem fail.");
        return;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)mntnMsg, PS_PID_DMS, PS_PID_DMS, msgLen - VOS_MSG_HEAD_LENGTH);

    /* 填写消息内容 */
    mntnMsg->msgId  = msgId;
    mntnMsg->portId = portId;

    if ((data != VOS_NULL_PTR) && (len > 0)) {
        memResult = memcpy_s(mntnMsg->data, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }

    mdrv_diag_trace_report(mntnMsg);

    /*lint -save -e516 */
    PS_MEM_FREE(PS_PID_DMS, mntnMsg);
    /*lint -restore */
}

MODULE_EXPORTED VOS_VOID DMS_SendPortDebugNvCfg(VOS_UINT32 debugLevel, VOS_UINT32 portIdMask1, VOS_UINT32 portIdMask2)
{
    struct DMS_PortDebugCfg *debugCfg = VOS_NULL_PTR;

    debugCfg = DMS_PORT_GetDebugCfg();

    debugCfg->debugLevel = debugLevel;
    debugCfg->portIdMask = portIdMask2;
    debugCfg->portIdMask = (debugCfg->portIdMask << 32) | portIdMask1;
}

STATIC VOS_VOID DMS_MntnSaveExcLog(VOS_VOID)
{
    struct DMS_MntnExcLogInfo *dmsExcAddr = DMS_MNTN_GetDmsExcAddr();
    struct DMS_MntnInfo       *dmsMntnInfo = DMS_MNTN_GetDmsMntnInfo();
    errno_t                    memResult;

    /* 申请失败，直接退出 */
    if (dmsExcAddr == VOS_NULL_PTR) {
        DMS_ERROR_LOG("DMS_MntnSaveExcLog:No memory allocated");
        return;
    }

    (VOS_VOID)memset_s(dmsExcAddr, DMS_MNTN_SAVE_EXC_LOG_LENGTH, 0, DMS_MNTN_SAVE_EXC_LOG_LENGTH);

    dmsExcAddr->beginTag = DMS_MNTN_DUMP_BEGIN_TAG;
    dmsExcAddr->endTag   = DMS_MNTN_DUMP_END_TAG;

#if (FEATURE_VCOM_EXT == FEATURE_ON)
    dmsMntnInfo->vcomExtFlag = 1;
#else
    dmsMntnInfo->vcomExtFlag = 0;
#endif

    memResult = memcpy_s(&(dmsExcAddr->mntnInfo), sizeof(dmsExcAddr->mntnInfo), dmsMntnInfo, sizeof(*dmsMntnInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(dmsExcAddr->mntnInfo), sizeof(dmsExcAddr->mntnInfo));
}

VOS_VOID DMS_RegisterDumpCallBack(VOS_VOID)
{
    struct DMS_MntnExcLogInfo *dmsExcAddr = VOS_NULL_PTR;

    /* 调用底软接口进行申请内存 */
    /* 分配内存 */
    dmsExcAddr = (struct DMS_MntnExcLogInfo *)mdrv_om_register_field(DMS_DUMP_FIELD_ID, "DMS dump",
        DMS_MNTN_SAVE_EXC_LOG_LENGTH, 0);

    /* 申请失败打印异常信息，内存申请异常时仍然注册，回调函数中有地址申请失败保护 */
    if (dmsExcAddr == VOS_NULL_PTR) {
        DMS_ERROR_LOG("DMS_RegisterDumpCallBack:mdrv_om_register_field fail.");
    }

    DMS_MNTN_SetDmsExcAddr(dmsExcAddr, sizeof(*dmsExcAddr));

    (VOS_VOID)mdrv_om_register_callback("DMS_EXCLOG", (dump_hook)DMS_MntnSaveExcLog);
}

