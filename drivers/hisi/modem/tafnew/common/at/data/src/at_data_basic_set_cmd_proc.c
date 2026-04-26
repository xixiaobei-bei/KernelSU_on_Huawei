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

#include "at_data_basic_set_cmd_proc.h"

#include "securec.h"
#include "at_mdrv_interface.h"
#include "taf_type_def.h"
#include "at_cmd_proc.h"
#include "at_input_proc.h"
#include "at_check_func.h"
#include "at_parse_cmd.h"
#include "ppp_interface.h"
#include "at_msg_print.h"
#include "si_app_stk.h"
#include "si_app_pih.h"
#include "at_phy_interface.h"
#include "at_data_proc.h"
#include "taf_drv_agent.h"
#include "mn_comm_api.h"
#include "at_cmd_msg_proc.h"
#include "at_rabm_interface.h"
#include "at_snd_msg.h"
#include "at_device_cmd.h"
#include "at_rnic_interface.h"
#include "at_taf_agent_interface.h"
#include "taf_agent_interface.h"
#include "taf_app_xsms_interface.h"

#include "taf_std_lib.h"
#include "mnmsgcbencdec.h"

#if (FEATURE_LTE == FEATURE_ON)
#include "mdrv_nvim.h"
#include "at_common.h"
#include "gen_msg.h"
#include "at_lte_common.h"
#endif
#if (FEATURE_LTEV == FEATURE_ON)
#include "vnas_at_interface.h"
#endif

#include "nv_stru_gucnas.h"
#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_pam.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"

#include "mn_call_api.h"
#include "at_ctx.h"
#include "app_vc_api.h"
#include "at_imsa_interface.h"
#include "at_init.h"
#include "at_file_handle.h"

#include "gunas_errno.h"

#include "css_at_interface.h"
#include "taf_app_mma.h"
#include "taf_mtc_api.h"
#include "taf_ccm_api.h"

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
#if (VOS_OS_VER != VOS_LINUX)
#include "Linuxstub.h"
#endif
#if (FEATURE_DATA_SERVICE_NEW_PLATFORM == FEATURE_ON)
#include "ads_dev_i.h"
#else
#include "ads_device_interface.h"
#endif
#else
#include "cds_interface.h"
#endif

#if (VOS_OS_VER == VOS_WIN32)
#include "NasStub.h"
#include "UsimmApi.h"
#endif

#include "imm_interface.h"
#include "throt_at_interface.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_BASIC_SET_CMD_PROC_C

#define AT_ICF_PARA_NUM 2
#define AT_ICF_FORMAT 0
#define AT_ICF_PARITY 1
#define AT_IFC_PARA_NUM 2
#define AT_IFC_DCE_BY_DTE 0
#define AT_IFC_DTE_BY_DCE 1

VOS_UINT32 At_SetAmpCPara(VOS_UINT8 indexNum)
{
    DMS_PortIdUint16            portId;
    DMS_PortUartDcdModeUint8    dcdMode;
    DMS_PortModeUint8           portMode;

    portId   = AT_GetDmsPortIdByClientId((AT_ClientIdUint16)indexNum);
    portMode = DMS_PORT_GetMode(portId);

    /* 只支持UART端口下发 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen != 0) {
        if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        dcdMode = (DMS_PortUartDcdModeUint8)g_atParaList[0].paraValue;
    } else {
        /* AT&C 等效于 AT&C1 */
        dcdMode = DMS_PORT_UART_DEFAULT_DCD_MODE;
    }

    /* 保存DCD Line Mode至上下文 */
    DMS_PORT_SetDcdMode(dcdMode);

    /* DCD管脚操作 */
    if (DMS_PORT_IsDcdModeConnectOn() == VOS_FALSE) {
        DMS_PORT_AssertDcd(portId);
    } else {
        if (portMode == DMS_PORT_MODE_CMD) {
            DMS_PORT_DeassertDcd(portId);
        }
    }

    return AT_OK;
}

VOS_UINT32 At_SetAmpDPara(VOS_UINT8 indexNum)
{
    DMS_PortUartDtrModeUint8    dtrMode;

    /* 只支持UART端口下发 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen != 0) {
        dtrMode = (DMS_PortUartDtrModeUint8)g_atParaList[0].paraValue;
    } else {
        /* AT&D 等效于 AT&D2 */
        dtrMode = DMS_PORT_UART_DEFAULT_DTR_MODE;
    }

    /* 保存DTR Line Mode至上下文 */
    DMS_PORT_SetDtrMode(dtrMode);

    return AT_OK;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_UINT32 At_SetAmpSPara(VOS_UINT8 indexNum)
{
    DMS_PortUartDsrModeUint8    dsrMode;
    DMS_PortIdUint16            portId;

    portId   = AT_GetDmsPortIdByClientId((AT_ClientIdUint16)indexNum);

    /* 只支持UART端口下发 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen != 0) {
        dsrMode = (DMS_PortUartDsrModeUint8)g_atParaList[0].paraValue;
    } else {
        /* AT&S 等效于 AT&S0 */
        dsrMode = DMS_PORT_UART_DEFAULT_DSR_MODE;
    }

    /* 保存DSR Line Mode至上下文 */
    DMS_PORT_SetDsrMode(dsrMode);

    /* 设置DSR 管脚信号 */
    if (DMS_PORT_IsDsrModeConnectOn() == VOS_FALSE) {
        DMS_PORT_AssertDsr(portId);
    } else {
        DMS_PORT_DeassertDsr(portId);
    }

    return AT_OK;
}

VOS_VOID AT_UpdateUartCfgNV(VOS_VOID)
{
    AT_UartCtx    *uartCtx = VOS_NULL_PTR;
    TAF_NV_UartCfg uartNvCfg;

    /* 初始化 */
    (VOS_VOID)memset_s(&uartNvCfg, sizeof(uartNvCfg), 0x00, sizeof(TAF_NV_UartCfg));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_UART_CFG, &uartNvCfg, sizeof(TAF_NV_UartCfg)) != NV_OK) {
        AT_ERR_LOG("AT_UpdateUartCfgNV: Read NV fail!");
        return;
    }

    /* 获取UART参数 */
    uartCtx                = AT_GetUartCtxAddr();
    uartNvCfg.baudRate     = uartCtx->phyConfig.baudRate;
    uartNvCfg.frame.format = uartCtx->phyConfig.frame.format;
    uartNvCfg.frame.parity = uartCtx->phyConfig.frame.parity;

    /* 更新UART参数NV */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_UART_CFG, (VOS_UINT8 *)&uartNvCfg, sizeof(TAF_NV_UartCfg)) != NV_OK) {
        AT_ERR_LOG("AT_UpdateUartCfgNV: Write NV fail!");
    }
}

VOS_UINT32 AT_SetIprPara(VOS_UINT8 indexNum)
{
    AT_UartPhyCfg        *phyCfg = VOS_NULL_PTR;
    AT_UartBaudrateUint32 baudRate;

    phyCfg = AT_GetUartPhyCfgInfo();

    /* 只支持UART端口下发 */
    if ((AT_CheckHsUartUser(indexNum) != VOS_TRUE) &&
        (AT_IsUartUserSupportBaudRate(indexNum) != VOS_TRUE)) {
        return AT_OK;
    }

    /* 参数检查 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空, 使用等效值115200 */
    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_CMD_NO_PARA) {
        baudRate = AT_UART_DEFAULT_BAUDRATE;
    } else {
        if (g_atParaList[0].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        baudRate = g_atParaList[0].paraValue;
    }

    /* 波特率有效性检查 */
    if (AT_CheckHsUartUser(indexNum) == VOS_TRUE) {
        if (AT_HSUART_IsBaudRateValid(baudRate) != VOS_TRUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        if (AT_UART_IsBaudRateValid(baudRate) != VOS_TRUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 更新波特率参数 */
    phyCfg->baudRate = baudRate;

    /* 刷新UART配置NV */
    AT_UpdateUartCfgNV();

    /* 先上报OK */
    At_FormatResultData(indexNum, AT_OK);

#if (FEATURE_UART_BAUDRATE_AUTO_ADAPTION == FEATURE_ON)
    if (baudRate == AT_UART_BAUDRATE_0) {
        return AT_SUCCESS;
    }
#endif
    /* 配置波特率参数 */
    AT_HSUART_ConfigBaudRate(indexNum, baudRate);

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetIcfPara(VOS_UINT8 indexNum)
{
    AT_UartPhyCfg     *phyCfg = VOS_NULL_PTR;
    AT_UartFormatUint8 format;
    AT_UartParityUint8 parity;

    phyCfg = AT_GetUartPhyCfgInfo();

    /* 只支持UART端口下发 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    /* 参数检查 */
    if (g_atParaIndex > AT_ICF_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    /* 参数为空，使用等效值(3,3),即:8个数据位1个停止位无校验位 */
    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_CMD_NO_PARA) {
        format = AT_UART_DEFAULT_FORMAT;
        parity = AT_UART_DEFAULT_PARITY;
    } else {
        /* AT_CMD_OPT_SET_PARA_CMD */
        /* AT+ICF= */
        if (g_atParaIndex == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* AT+ICF=, */
        if (g_atParaList[AT_ICF_FORMAT].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        format = (VOS_UINT8)g_atParaList[AT_ICF_FORMAT].paraValue;

        if (g_atParaIndex > AT_ICF_PARITY) {
            if (g_atParaList[AT_ICF_PARITY].paraLen != 0) {
                /* AT+ICF=<format>,<parity> */
                parity = (VOS_UINT8)g_atParaList[AT_ICF_PARITY].paraValue;
            } else {
                /* AT+ICF=<format>, */
                return AT_CME_INCORRECT_PARAMETERS;
            }
        } else {
            /* AT+ICF=<format> */
            parity = AT_UART_DEFAULT_PARITY;
        }
    }

    /* 检查帧格式参数有效性 */
    if (AT_HSUART_ValidateCharFrameParam(format, parity) == VOS_FALSE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 更新帧格式参数 */
    phyCfg->frame.format = format;
    phyCfg->frame.parity = parity;

    /* 刷新UART配置NV */
    AT_UpdateUartCfgNV();

    /* 先上报OK */
    At_FormatResultData(indexNum, AT_OK);

    /* 配置帧格式参数 */
    AT_HSUART_ConfigCharFrame(indexNum, format, parity);

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetIfcPara(VOS_UINT8 indexNum)
{
    AT_UartFlowCtrl       *uartFlowCtrl = VOS_NULL_PTR;
    AT_UartFcDceByDteUint8 fcDceByDte;
    AT_UartFcDteByDceUint8 fcDteByDce;
    VOS_UINT32             rtsFlag;
    VOS_UINT32             ctsFlag;

    uartFlowCtrl = AT_GetUartFlowCtrlInfo();

    /* 只支持UART端口下发 */
    if (AT_CheckHsUartUser(indexNum) != VOS_TRUE) {
        return AT_OK;
    }

    /* 命令参数个数检查 */
    if (g_atParaIndex > AT_IFC_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * AT+IFC等效于AT+IFC=0,0
     * 注: 该处理为产品线要求, IUT-T-V.250中规定AT+IFC等效于AT+IFC=2,2
     */
    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_CMD_NO_PARA) {
        fcDceByDte = AT_UART_DEFAULT_FC_DCE_BY_DTE;
        fcDteByDce = AT_UART_DEFAULT_FC_DTE_BY_DCE;
    } else {
        /* AT+IFC= */
        if (g_atParaIndex == 0) {
            /* AT+IFC= */
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* AT+IFC=, */
        if (g_atParaList[AT_IFC_DCE_BY_DTE].paraLen == 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (g_atParaIndex > AT_IFC_DTE_BY_DCE) {
            if (g_atParaList[AT_IFC_DTE_BY_DCE].paraLen != 0) {
                /* AT+IFC=<DCE_by_DTE>,<DTE_by_DCE> */
                fcDteByDce = (VOS_UINT8)g_atParaList[AT_IFC_DTE_BY_DCE].paraValue;
            } else {
                /* AT+IFC=<DCE_by_DTE>, */
                return AT_CME_INCORRECT_PARAMETERS;
            }
        } else {
            /* AT+IFC=<DCE_by_DTE> */
            fcDteByDce = AT_UART_DEFAULT_FC_DTE_BY_DCE;
        }

        fcDceByDte = (VOS_UINT8)g_atParaList[AT_IFC_DCE_BY_DTE].paraValue;
    }

    /* 检查流控参数有效性 */
    if (AT_HSUART_ValidateFlowCtrlParam(fcDceByDte, fcDteByDce) == VOS_FALSE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 更新流控参数 */
    uartFlowCtrl->dceByDte = fcDceByDte;
    uartFlowCtrl->dteByDce = fcDteByDce;

    /* 转换硬件流控参数 */
    rtsFlag = (fcDceByDte == AT_UART_FC_DCE_BY_DTE_RTS) ? VOS_TRUE : VOS_FALSE;

    ctsFlag = (fcDteByDce == AT_UART_FC_DTE_BY_DCE_CTS) ? VOS_TRUE : VOS_FALSE;

    /* 配置硬件流控参数 */
    AT_HSUART_ConfigFlowCtrl(indexNum, rtsFlag, ctsFlag);

    return AT_OK;
}

VOS_UINT32 AT_SetOPara(VOS_UINT8 indexNum)
{
    DMS_PortIdUint16            portId;
    DMS_PortModeUint8           mode;

    portId   = g_atClientTab[indexNum].portNo;
    mode     = DMS_PORT_GetMode(portId);

    /* 如果当前通道模式为ONLINE-COMMAND模式, 直接恢复当前数据模式 */
    if (mode == DMS_PORT_MODE_ONLINE_CMD) {
        /* 将通道模式设置为DATA模式，其中data的子模式不做修改，直接继承 */
        DMS_PORT_SetMode(portId, DMS_PORT_MODE_ONLINE_DATA);

        /* 此处dataState先置为STOP，
         * 待connect上报后，数据状态才切换为start;
         * 切换为start的函数参见AT_DisplayResultData
         */
        DMS_PORT_SetDataState(portId, DMS_PORT_DATA_STATE_STOP);

        return AT_CONNECT;
    } else {
        return AT_NO_CARRIER;
    }
}

VOS_UINT32 AT_SetSwitchUart(VOS_UINT8 indexNum)
{
    VOS_UINT32 ulmode;

    if (indexNum != AT_CLIENT_ID_HSUART) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 1\n");
        return AT_ERROR;
    }

    /* 判断CK是否校验通过 */
    if (g_spWordCtx.shellPwdCheckFlag != VOS_TRUE) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 2\n");
        AT_WARN_LOG("AT_SetSwitchUart:WARNING: datalock check faild!");
        return AT_ERROR;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 3\n");
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 4\n");
        return AT_ERROR;
    }

    /* 切换到 A shell */
    if (g_atParaList[0].paraValue == AT_UART_MODE_ASHELL) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 5\n");
        ulmode = DMS_UART_A_SHELL;
    } else if (g_atParaList[0].paraValue == AT_UART_MODE_CSHELL) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 6\n");
        ulmode = DMS_UART_C_SHELL;
    } else {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 7\n");
        AT_WARN_LOG("AT_SetSwitchUart: unknown value!");
        return AT_ERROR;
    }

    if (DMS_PORT_SetACShell(DMS_PORT_HSUART, ulmode) != VOS_OK) {
        PS_PRINTF_WARNING("<AT_SetSwitchUart> 8\n");
        AT_WARN_LOG("AT_SetSwitchUart:DRV_UDI_IOCTL faild!");
        return AT_ERROR;
    }

    AT_INFO_LOG("AT_SetSwitchUart: success and leave!!");
    PS_PRINTF_INFO("<AT_SetSwitchUart> success and leave!!\n");

    return AT_OK;
}

#endif

TAF_UINT32 At_SetXPara(TAF_UINT8 indexNum)
{
    if (g_atParaList[0].paraLen != 0) {
        g_atXType = (TAF_UINT8)g_atParaList[0].paraValue;
    } else {
        g_atXType = 0;
    }
    return AT_OK;
}

