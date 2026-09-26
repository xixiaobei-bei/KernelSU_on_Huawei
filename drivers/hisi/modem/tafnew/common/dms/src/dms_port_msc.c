/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "vos_pid_def.h"
#include "dms_debug.h"
#include "dms_port_i.h"
#include "dms_port.h"
#include "dms_task.h"
#include "dms_dev_i.h"
#include "dms_port_cmux.h"
#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_PORT_MSC_C

VOS_UINT32 DMS_PORT_StartFlowCtrl(VOS_UINT32 param1, VOS_UINT32 param2)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;
    DMS_PortIdUint16 portId;

    portId = (DMS_PortIdUint16)param1;
    DMS_PORT_LOGI(portId, "enter.");
    if (portId >= DMS_PORT_BUTT) {
        return VOS_ERR;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);

    if (modeInfo->mode == DMS_PORT_MODE_ONLINE_DATA) {
        DMS_PORT_DeassertCts(portId);
    }

    return VOS_OK;
}

VOS_UINT32 DMS_PORT_StopFlowCtrl(VOS_UINT32 param1, VOS_UINT32 param2)
{
    struct DMS_Port_ModeInfo *modeInfo = VOS_NULL_PTR;
    DMS_PortIdUint16 portId;

    portId = (DMS_PortIdUint16)param1;
    DMS_PORT_LOGI(portId, "enter.");
    if (portId >= DMS_PORT_BUTT) {
        return VOS_ERR;
    }

    modeInfo = DMS_PORT_GetModeInfo(portId);

    if (modeInfo->mode == DMS_PORT_MODE_ONLINE_DATA) {
        DMS_PORT_AssertCts(portId);
    }

    return VOS_OK;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

STATIC VOS_VOID DMS_PORT_ProcHsuartDtrAsserted(DMS_PortIdUint16 portId)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;
    lineCtrl = DMS_PORT_GetUartLineCtrl();

    DMS_LOGI("enter.");

    /* 判断&S[<value>] */
    if (lineCtrl->dsrMode == DMS_PORT_UART_DSR_MODE_ALWAYS_ON) {
        DMS_PORT_AssertDsr(portId);
    }

    /* 判断&C[<value>] */
    if (lineCtrl->dcdMode == DMS_PORT_UART_DCD_MODE_ALWAYS_ON) {
        DMS_PORT_AssertDcd(portId);
    }

    /*停止流控*/
    DMS_PORT_AssertCts(portId);
}

STATIC VOS_VOID DMS_PORT_ProcHsuartDtrDeassert(DMS_PortIdUint16 portId)
{
    struct DMS_PORT_UartLineCtrl   *lineCtrl   = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    lineCtrl   = DMS_PORT_GetUartLineCtrl();

    if (lineCtrl->dtrMode == DMS_PORT_UART_DTR_MODE_SWITCH_CMD_MODE) {
        /* dtr为switchmode时，处理逻辑和esc seq的处理逻辑一致 */
        DMS_SendIntraEvent(ID_DMS_EVENT_ESCAPE_SEQUENCE, portId, VOS_NULL_PTR, 0);
        return;
    }

    if (lineCtrl->dtrMode == DMS_PORT_UART_DTR_MODE_HANGUP_CALL) {
        DMS_SendIntraEvent(ID_DMS_EVENT_DTR_DEASSERT, portId, VOS_NULL_PTR, 0);
        return;
    }

    /* 其他模式不处理 */
    DMS_PORT_LOGE(DMS_PORT_HSUART, "invalid dtr mode:%d.", lineCtrl->dtrMode);
}
#endif

STATIC VOS_VOID DMS_PORT_ProcModemDtrAsserted(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    /* 拉高DSR信号 */
    DMS_PORT_AssertDsr(portId);
    /* 停止流控 */
    DMS_PORT_AssertCts(portId);
}

VOS_VOID DMS_PORT_ProcModemDtrDeasserted(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    /*
     * DSR信号在上电后一直保持拉高状态，即使收到DTR也不拉低DSR；
     * 同时，PC在正常流程中一般不会拉低DTR信号，在异常流程中会将之拉低，
     * 所以UE在收到DTR拉低 的时候，需要将DCD拉低
     */
    DMS_PORT_DeassertDcd(portId);

    DMS_SendIntraEvent(ID_DMS_EVENT_DTR_DEASSERT, portId, VOS_NULL_PTR, 0);
}

VOS_VOID DMS_PORT_ProcDtrAsserted(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    if (portId == DMS_PORT_MODEM) {
        DMS_PORT_ProcModemDtrAsserted(portId);
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (portId == DMS_PORT_HSUART) {
        DMS_PORT_ProcHsuartDtrAsserted(portId);
    }
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if ((DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) && (DMS_PORT_IsCmuxPort(portId) == VOS_TRUE)) {
        DMS_PORT_ProcHsuartDtrAsserted(portId);
    }
#endif
}

VOS_VOID DMS_PORT_ProcDtrDeasserted(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    if (portId == DMS_PORT_MODEM) {
        DMS_PORT_ProcModemDtrDeasserted(portId);
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (portId == DMS_PORT_HSUART) {
        DMS_PORT_ProcHsuartDtrDeassert(portId);
    }
#endif

#if (FEATURE_IOT_CMUX == FEATURE_ON)
    if (DMS_PORT_IsCmuxPort(portId) == VOS_TRUE) {
        if (DMS_PORT_GetCmuxPhyPort() == DMS_PORT_HSUART) {
            /* 高速串口处理 */
            DMS_PORT_ProcHsuartDtrDeassert(portId);
        } else {
            /* 低速串口处理 */
            DMS_PORT_ProcModemDtrDeasserted(portId);
        }
    }
#endif
}

VOS_VOID  DMS_PORT_RcvMsc(DMS_PortIdUint16 portId, struct DMS_MscInfo *msc)
{
    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_INPUT_MSC, (VOS_UINT8 *)msc, sizeof(*msc));

    DMS_SendDmsMscInd(portId, msc);
}

VOS_VOID DMS_PORT_AssertDcd(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.dcdFlag = VOS_TRUE;
    msc.dcd = DMS_IO_LEVEL_HIGH;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus |= DMS_IO_CTRL_DCD;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8 *)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_DeassertDcd(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.dcdFlag = VOS_TRUE;
    msc.dcd = DMS_IO_LEVEL_LOW;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus &= ~DMS_IO_CTRL_DCD;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_AssertDsr(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.dsrFlag = VOS_TRUE;
    msc.dsr = DMS_IO_LEVEL_HIGH;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus |= DMS_IO_CTRL_DSR;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_DeassertDsr(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.dsrFlag = VOS_TRUE;
    msc.dsr = DMS_IO_LEVEL_LOW;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus &= ~DMS_IO_CTRL_DSR;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_AssertCts(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_PORT_LOGI(portId, "enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_STOP_FLOW_CTRL, VOS_NULL_PTR, 0);

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.ctsFlag = VOS_TRUE;
    msc.cts = DMS_IO_LEVEL_HIGH;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus |= DMS_IO_CTRL_CTS;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_DeassertCts(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_PORT_LOGI(portId, "enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_START_FLOW_CTRL, VOS_NULL_PTR, 0);

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.ctsFlag = VOS_TRUE;
    msc.cts = DMS_IO_LEVEL_LOW;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus &= ~DMS_IO_CTRL_CTS;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_AssertRi(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.riFlag = VOS_TRUE;
    msc.ri = DMS_IO_LEVEL_HIGH;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus |= DMS_IO_CTRL_RI;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_DeassertRi(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo     *portInfo = VOS_NULL_PTR;
    struct DMS_MscInfo       msc;
    VOS_UINT32               ret;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return;
    }

    (VOS_VOID)memset_s(&msc, sizeof(msc), 0x00, sizeof(msc));

    msc.riFlag = VOS_TRUE;
    msc.ri = DMS_IO_LEVEL_LOW;

    portInfo = DMS_PORT_GetPortInfo(portId);

    /* 写入管脚信号参数 */
    ret = DMS_DEV_SendMscCmd(portInfo->devInfo, &msc);

    if (ret == VOS_OK) {
        portInfo->mscStatus &= ~DMS_IO_CTRL_RI;

        DMS_MNTN_TraceReport(portId, ID_DMS_MNTN_OUTPUT_MSC, (VOS_UINT8*)&msc, sizeof(msc));
    }
}

VOS_VOID DMS_PORT_SetDcdMode(DMS_PortUartDcdModeUint8 dcdMode)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    lineCtrl->dcdMode = dcdMode;
}

VOS_VOID DMS_PORT_SetDsrMode(DMS_PortUartDsrModeUint8 dsrMode)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    lineCtrl->dsrMode = dsrMode;
}

VOS_VOID DMS_PORT_SetDtrMode(DMS_PortUartDtrModeUint8 dtrMode)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    lineCtrl->dtrMode = dtrMode;
}

VOS_BOOL DMS_PORT_IsCtsDeasserted(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_FALSE;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    if ((portInfo->mscStatus & DMS_IO_CTRL_CTS) == 0) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_BOOL DMS_PORT_IsRiAsserted(DMS_PortIdUint16 portId)
{
    struct DMS_PortInfo *portInfo = VOS_NULL_PTR;

    DMS_LOGI("enter.");

    if (DMS_PORT_IsPortInited(portId) == VOS_FALSE) {
        DMS_LOGE("port not inited.");
        return VOS_FALSE;
    }

    portInfo = DMS_PORT_GetPortInfo(portId);

    if ((portInfo->mscStatus & DMS_IO_CTRL_RI) == 0) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_BOOL DMS_PORT_IsDcdModeConnectOn(VOS_VOID)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;
    VOS_BOOL ret;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    if (lineCtrl->dcdMode == DMS_PORT_UART_DCD_MODE_CONNECT_ON) {
        ret = VOS_TRUE;
    } else {
        ret = VOS_FALSE;
    }

    return ret;
}

VOS_BOOL DMS_PORT_IsDsrModeConnectOn(VOS_VOID)
{
    struct DMS_PORT_UartLineCtrl *lineCtrl = VOS_NULL_PTR;
    VOS_BOOL ret;

    lineCtrl = DMS_PORT_GetUartLineCtrl();

    if (lineCtrl->dsrMode == DMS_PORT_UART_DSR_MODE_CONNECT_ON) {
        ret = VOS_TRUE;
    } else {
        ret = VOS_FALSE;
    }

    return ret;
}

VOS_VOID DMS_PORT_ProcNoCarrierMsc(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    if (portId == DMS_PORT_MODEM) {
        /* 拉低DCD信号 */
        DMS_PORT_DeassertDcd(portId);
        VOS_TaskDelay(30);
        return;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (DMS_PORT_IsHsuartBasePort(portId) == VOS_TRUE) {
        /* 判断&S并处理DSR管脚 */
        if (DMS_PORT_IsDsrModeConnectOn() == VOS_TRUE) {
            DMS_PORT_DeassertDsr(portId);
        }

        /* 判断&C并处理DCD管脚 */
        if (DMS_PORT_IsDcdModeConnectOn() == VOS_TRUE) {
            DMS_PORT_DeassertDcd(portId);
        }

        return;
    }
#endif
}

VOS_VOID DMS_PORT_ProcConnectMsc(DMS_PortIdUint16 portId)
{
    DMS_LOGI("enter.");

    if (portId == DMS_PORT_MODEM) {
        /* 拉高DCD信号 */
        DMS_PORT_AssertDcd(portId);
        return;
    }

#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (DMS_PORT_IsHsuartBasePort(portId) == VOS_TRUE) {
        /* 判断&S并处理DSR管脚 */
        if (DMS_PORT_IsDsrModeConnectOn() == VOS_TRUE) {
            DMS_PORT_AssertDsr(portId);
        }

        /* 判断&C并处理DCD管脚 */
        if (DMS_PORT_IsDcdModeConnectOn() == VOS_TRUE) {
            DMS_PORT_AssertDcd(portId);
        }

        return;
    }
#endif
}

