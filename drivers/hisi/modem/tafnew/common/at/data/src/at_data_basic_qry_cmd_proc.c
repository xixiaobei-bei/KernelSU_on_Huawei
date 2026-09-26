/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "at_data_basic_qry_cmd_proc.h"
#include "securec.h"

#include "at_mdrv_interface.h"
#include "ppp_interface.h"
#include "taf_drv_agent.h"
#include "taf_tafm_remote.h"
#include "ttf_mem_at_interface.h"
#include "at_cmd_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "at_cmd_msg_proc.h"
#include "at_snd_msg.h"
#include "at_rnic_interface.h"
#include "at_input_proc.h"
#include "at_event_report.h"
#include "at_device_cmd.h"
#include "at_taf_agent_interface.h"
#include "app_vc_api.h"
#include "taf_oam_interface.h"
#include "taf_iface_api.h"
#include "at_lte_eventreport.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "at_lte_common.h"
#endif

#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "css_at_interface.h"

#include "at_msg_print.h"

#include "ps_common_def.h"
#include "ps_lib.h"
#include "product_config.h"
#include "taf_ccm_api.h"
#include "mn_comm_api.h"
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#endif
#include "throt_at_interface.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_BASIC_QRY_CMD_PROC_C

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_UINT32 AT_QryIprPara(VOS_UINT8 indexNum)
{
    AT_UartCtx *uartCtx = VOS_NULL_PTR;
    VOS_UINT16  length;

    /* 初始化 */
    uartCtx = AT_GetUartCtxAddr();
    length  = 0;

    /* 通道检查 */
    if ((AT_CheckHsUartUser(indexNum) != VOS_TRUE) &&
        (AT_IsUartUserSupportBaudRate(indexNum) != VOS_TRUE)) {
        return AT_OK;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %u", g_parseContext[indexNum].cmdElement->cmdName,
        uartCtx->phyConfig.baudRate);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_TestIprPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(
        AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress,
        "%s: (0,300,600,1200,2400,4800,9600,19200,38400,57600,115200,230400,460800,921600,2764800,4000000)",
        g_parseContext[indexNum].cmdElement->cmdName);
    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 AT_QryIcfPara(VOS_UINT8 indexNum)
{
    AT_UartCtx *uartCtx = VOS_NULL_PTR;
    VOS_UINT16  length;

    /* 初始化 */
    uartCtx = AT_GetUartCtxAddr();
    length  = 0;

    /* 通道检查 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        uartCtx->phyConfig.frame.format, uartCtx->phyConfig.frame.parity);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_TestIcfPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (1-6),(0-1)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 AT_QryIfcPara(VOS_UINT8 indexNum)
{
    AT_UartCtx *uartCtx = VOS_NULL_PTR;
    VOS_UINT16  length;

    /* 初始化 */
    uartCtx = AT_GetUartCtxAddr();
    length  = 0;

    /* 通道检查 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        uartCtx->flowCtrl.dceByDte, uartCtx->flowCtrl.dteByDce);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_TestIfcPara(VOS_UINT8 indexNum)
{
    VOS_UINT16 length;

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0,2),(0,2)", g_parseContext[indexNum].cmdElement->cmdName);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}
#endif


