/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include "at_input_proc.h"
#include "securec.h"

#include "ppp_interface.h"
#include "at_usb_com_itf.h"
#include "at_taf_agent_interface.h"
#include "taf_agent_interface.h"
#include "at_cmd_msg_proc.h"
#include "at_data_proc.h"
#include "at_mdrv_interface.h"
#include "at_mntn.h"
#include "acpu_reset.h"

#include "at_internal_msg.h"
#include "mn_comm_api.h"

#include "dms_debug.h"
#include "dms_file_node_i.h"
#include "product_config.h"
#include "dms_port_i.h"




#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)
/* adrv.h用于hifireset_regcbfunc函数申明，后续该接口删除后，该头文件可以删除 */
#include "adrv.h"
#endif


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_INPUTPROC_C
#define AT_MSG_CUSTOMIZED_ITEM_LEN 4

VOS_UINT32 g_atUsbDebugFlag = VOS_FALSE;

#if (FEATURE_AT_HSUART == FEATURE_ON)
/* AT帧结构与DRV 值之间的对应关系 */
static const AT_UartFormatParam g_atUartFormatTab[] = {
    /* auto detect (not support) */

    { AT_UART_FORMAT_8DATA_2STOP, AT_UART_DATA_LEN_8_BIT,
      AT_UART_STOP_LEN_2_BIT, AT_UART_PARITY_LEN_0_BIT },

    { AT_UART_FORMAT_8DATA_1PARITY_1STOP, AT_UART_DATA_LEN_8_BIT,
      AT_UART_STOP_LEN_1_BIT, AT_UART_PARITY_LEN_1_BIT },

    { AT_UART_FORMAT_8DATA_1STOP, AT_UART_DATA_LEN_8_BIT,
      AT_UART_STOP_LEN_1_BIT, AT_UART_PARITY_LEN_0_BIT },

    { AT_UART_FORMAT_7DATA_2STOP, AT_UART_DATA_LEN_7_BIT,
      AT_UART_STOP_LEN_2_BIT, AT_UART_PARITY_LEN_0_BIT },

    { AT_UART_FORMAT_7DATA_1PARITY_1STOP, AT_UART_DATA_LEN_7_BIT,
      AT_UART_STOP_LEN_1_BIT, AT_UART_PARITY_LEN_1_BIT },

    { AT_UART_FORMAT_7DATA_1STOP, AT_UART_DATA_LEN_7_BIT,
      AT_UART_STOP_LEN_1_BIT, AT_UART_PARITY_LEN_0_BIT }
};
#endif

MODULE_EXPORTED VOS_UINT32 AT_GetUsbDebugFlag(VOS_VOID)
{
    return g_atUsbDebugFlag;
}

AT_ClientManage* AT_GetClientManage(AT_ClientIdUint16 clientId)
{
    /* 内部接口，由调用方保证client id有效 */
    return &(g_atClientTab[clientId]);
}

VOS_INT32 AT_RcvCmdData(VOS_UINT16 clientId, VOS_UINT8 *data, VOS_UINT32 length)
{
    AT_ClientManage   *atClientManage = VOS_NULL_PTR;
    VOS_UINT32         ret;

    /* AT命令长度处理函数统一用的VOS_UINT16，超过支持范围的长度打印异常返回 */
    if (length > AT_RCV_DATA_MAX_LEN) {
        AT_WARN_LOG("AT_RcvCmdData: length is more than 0xffff!");
        return VOS_ERR;
    }

    if (clientId >= AT_CLIENT_ID_BUTT) {
        AT_WARN_LOG1("AT_RcvCmdData: client id more than AT_CLIENT_ID_BUTT!", clientId);
        return VOS_ERR;
    }

    atClientManage = AT_GetClientManage(clientId);

    if (atClientManage->used == AT_CLIENT_NULL) {
        AT_WARN_LOG("AT_RcvCmdData: APP client is unused!");
        return VOS_ERR;
    }

    if (AT_GetUsbDebugFlag() == VOS_TRUE) {
        PS_PRINTF_INFO("AT_RcvCmdData clientId = %d, length = %d, data = %s\n", clientId, length, data);
    }

    ret = At_CmdStreamPreProc((VOS_UINT8)clientId, data, (VOS_UINT16)length);

    if (ret == AT_SUCCESS) {
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

VOS_UINT32 AT_GetAtMsgStruMsgLength(VOS_UINT32 infoLength)
{
    VOS_UINT32 msgLength;

    if (infoLength > AT_MSG_CUSTOMIZED_ITEM_LEN) {
        msgLength = (sizeof(AT_Msg) - VOS_MSG_HEAD_LENGTH) + (infoLength - AT_MSG_CUSTOMIZED_ITEM_LEN);
    } else {
        msgLength = sizeof(AT_Msg) - VOS_MSG_HEAD_LENGTH;
    }

    return msgLength;
}

VOS_UINT8 AT_GetUserTypeFromIndex(VOS_UINT8 indexNum)
{
    VOS_UINT8 userType;

    if (indexNum < AT_MAX_CLIENT_NUM) {
        userType = g_atClientTab[indexNum].userType;
    } else {
        userType = AT_BUTT_USER;
    }

    return userType;
}

VOS_BOOL AT_IsClientAllowEcho(VOS_UINT8 clientId)
{
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    systemAppConfig = AT_GetSystemAppConfigAddr();

    /* UART定制特性开启，UART通道支持无需回显 */
#if (FEATURE_IOT_UART_CUST == FEATURE_OFF)
    if (clientId == AT_CLIENT_ID_UART) {
        return VOS_FALSE;
    }
#endif

    /* NDIS通道无需回显 */
    if (clientId == AT_CLIENT_ID_NDIS) {
        return VOS_FALSE;
    }

    /* HSUART通道无需回显 */
#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (clientId == AT_CLIENT_ID_HSUART) {
        return VOS_FALSE;
    }
#endif

    if (AT_CheckAppUser(clientId) == VOS_TRUE) {
        /* AGPS通道无需回显 */
#if (FEATURE_VCOM_EXT == FEATURE_ON)
        if ((clientId == AT_CLIENT_ID_APP9) || (clientId == AT_CLIENT_ID_APP12) || (clientId == AT_CLIENT_ID_APP24)) {
            return VOS_FALSE;
        }
#endif

        /* E5形态无需回显 */
        if (*systemAppConfig == SYSTEM_APP_WEBUI) {
            return VOS_FALSE;
        }
    }

    if (AT_GetModemMbbCustCtxAddrFromClientId(clientId)->vcomEchoFlag == VOS_TRUE) {
        if (clientId == AT_CLIENT_ID_APP) {
            return VOS_FALSE;
        }
#if (FEATURE_VCOM_EXT == FEATURE_ON)
        if ((clientId == AT_CLIENT_ID_APP2) || (clientId == AT_CLIENT_ID_APP4) || (clientId == AT_CLIENT_ID_APP5)) {
            return VOS_FALSE;
        }
#endif
    }

    return VOS_TRUE;
}

VOS_VOID AT_CmdStreamEcho(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT32 ret;
    VOS_UINT16 echoLen;

    /* 判断pData码流的结尾是否为<CR><LF>形式，代码中2为回车换行两个字符长度 */
    if ((len > 2) && (g_atS3 == data[len - 2]) && (g_atS4 == data[len - 1])) {
        /* 删去结尾的<LF>字符 */
        echoLen = len - 1;
    } else {
        echoLen = len;
    }

    if (AT_IsClientAllowEcho(indexNum) == VOS_TRUE) {
        ret = DMS_PORT_SendData(g_atClientTab[indexNum].portNo, data, echoLen);
    } else {
        AT_ERR_LOG1("AT_CmdStreamEcho:WARNING: not allow echo, ucIndex:", indexNum);
        return;
    }

    if (ret != VOS_OK) {
        AT_ERR_LOG1("AT_CmdStreamEcho:WARNING: Abnormal UserType,ucIndex:", indexNum);
    }
}

VOS_UINT32 At_CmdStreamPreProc(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT8 *head = TAF_NULL_PTR;
    errno_t    memResult;
    VOS_UINT16 count = 0;
    VOS_UINT16 total = 0;

    head = data;

    if (g_atUsbDebugFlag == VOS_TRUE) {
        PS_PRINTF_INFO("<At_CmdStreamPreProc> AtEType = %d, UserType = %d, g_atS3 = %d\n", g_atEType,
                       g_atClientTab[indexNum].userType, g_atS3);
    }

    /* 处理通道回显 */
    if (g_atEType == AT_E_ECHO_CMD) {
        AT_CmdStreamEcho(indexNum, data, len);
    }

    /*
     * MAC系统上的MP后台问题:AT+CMGS=**<CR><^z><Z>(或AT+CMGW=**<CR><^z><Z>)
     * 为了规避该问题，需要在接收到如上形式的码流后，
     * 需要将命令后的无效字符<^z><Z>删去
     */
    AT_DiscardInvalidCharForSms(data, &len);

    /* 解析到如下字符才将码流以消息方式发送到AT的消息队列中: <CR>/<ctrl-z>/<ESC> */
    while (count++ < len) {
        if (At_CheckSplitChar((*((data + count) - 1)))) {
            if (g_atDataBuff[indexNum].buffLen > 0) {
                if ((g_atDataBuff[indexNum].buffLen + count) >= AT_COM_BUFF_LEN) {
                    g_atDataBuff[indexNum].buffLen = 0;
                    AT_WARN_LOG("At_CmdStreamPreProc: buff is full at split char!");
                    return AT_FAILURE;
                }
                memResult = memcpy_s(&g_atDataBuff[indexNum].dataBuff[g_atDataBuff[indexNum].buffLen],
                                     AT_COM_BUFF_LEN - g_atDataBuff[indexNum].buffLen, head, count);
                TAF_MEM_CHK_RTN_VAL(memResult, AT_COM_BUFF_LEN - g_atDataBuff[indexNum].buffLen, count);
                At_SendCmdMsg(indexNum, g_atDataBuff[indexNum].dataBuff,
                              (TAF_UINT16)(g_atDataBuff[indexNum].buffLen + count), AT_NORMAL_TYPE_MSG);
                head                           = data + count;
                total                          = count;
                g_atDataBuff[indexNum].buffLen = 0;
            } else {
                At_SendCmdMsg(indexNum, head, count - total, AT_NORMAL_TYPE_MSG);
                head  = data + count;
                total = count;
            }
        }
    }

    if (total < len) {
        if ((g_atDataBuff[indexNum].buffLen + (len - total)) >= AT_COM_BUFF_LEN) {
            g_atDataBuff[indexNum].buffLen = 0;
            AT_WARN_LOG("At_CmdStreamPreProc: buff is full after split char!");
            return AT_FAILURE;
        }
        memResult = memcpy_s(&g_atDataBuff[indexNum].dataBuff[g_atDataBuff[indexNum].buffLen],
                             AT_COM_BUFF_LEN - g_atDataBuff[indexNum].buffLen, head, (TAF_UINT32)(len - total));
        TAF_MEM_CHK_RTN_VAL(memResult, AT_COM_BUFF_LEN - g_atDataBuff[indexNum].buffLen, (TAF_UINT32)(len - total));
        g_atDataBuff[indexNum].buffLen += (VOS_UINT16)((data - head) + len);
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ProcDtrDeassertedIndPppDataMode(AT_ClientIdUint16 indexNum)
{
    AT_ClientManage    *clientManage = VOS_NULL_PTR;

    clientManage = AT_GetClientManage(indexNum);

    /*
     * 1.判断(g_atClientTab[ucIndex].CmdCurrentOpt != AT_CMD_PS_DATA_CALL_END_SET)
     *  的目的:若正常断开，则之前已经执行了PDP DEACTIVE流程，此时若再拉低DTR，则
     *  无需再执行该异常流程
     * 2.该分支的处理场景:若出于流控状态下,用户发起PPP断开，此时PPP报文无法交互，
     *   只能在最后拉低DTR信号的时候，执行PPP断开操作
     */
    if ((clientManage->cmdCurrentOpt == AT_CMD_PS_DATA_CALL_END_SET) ||
        (clientManage->cmdCurrentOpt == AT_CMD_WAIT_PPP_PROTOCOL_REL_SET)) {
        return AT_SUCCESS;
    }

    /*
     * 若处于流控状态下，则PPP断开的协商报文是无法传到UE侧的，
     * UE只能在DTR信号拉低的时候,执行PPP拨号断开操作
     */
    if (DMS_PORT_IsCtsDeasserted(clientManage->portNo) == VOS_TRUE) {
        /* 通过拉高cts，停止流控 */
        DMS_PORT_AssertCts(clientManage->portNo);
    }

    AT_SendRelPppReq(clientManage->pppId, PPP_AT_CTRL_REL_PPP_REQ);

    /* 停止Modem口的AT定时器以及AT链路的当前操作指示 */
    AT_StopTimerCmdReady((VOS_UINT8)indexNum);

    /* EVENT - RCV Down DTR to Disconnect PPP in Abnormal procedure(PDP type:IP) ;index */
    AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_DOWN_DTR_RELEASE_PPP_IP_TYPE, (VOS_UINT8 *)&indexNum, sizeof(indexNum));

    if (TAF_PS_PPP_DIAL_DOWN(WUEPS_PID_AT, AT_PS_BuildExClientId(clientManage->clientId), 0,
                       clientManage->cid) == VOS_OK) {
        /* 开定时器 */
        if (At_StartTimer(AT_SET_PARA_TIME, (VOS_UINT8)indexNum) != AT_SUCCESS) {
            AT_ERR_LOG("AT_ProcDtrDeassertedIndPppDataMode:ERROR:Start Timer");
            return AT_FAILURE;
        }

        /* 设置当前操作类型 */
        clientManage->cmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
    } else {
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ProcDtrDeassertedIndRawDataMode(AT_ClientIdUint16 indexNum)
{
    AT_ClientManage    *clientManage = VOS_NULL_PTR;

    clientManage = AT_GetClientManage(indexNum);

    /* 若原先开启了流控，则需停止流控 */
    if (DMS_PORT_IsCtsDeasserted(clientManage->portNo) == VOS_TRUE) {
        /* 通过拉高cts，停止流控 */
        DMS_PORT_AssertCts(clientManage->portNo);
    }

    AT_SendRelPppReq(clientManage->pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

    /* 停止Modem口的AT定时器以及AT链路的当前操作指示 */
    AT_StopTimerCmdReady((VOS_UINT8)indexNum);

    /* EVENT - RCV Down DTR to Disconnect PPP in Abnormal procedure(PDP type:PPP) ;index */
    AT_EventReport(WUEPS_PID_AT, NAS_OM_EVENT_DTE_DOWN_DTR_RELEASE_PPP_PPP_TYPE, (VOS_UINT8*)&indexNum, sizeof(indexNum));

    if (TAF_PS_PPP_DIAL_DOWN(WUEPS_PID_AT, AT_PS_BuildExClientId(clientManage->clientId), 0,
                       clientManage->cid) == VOS_OK) {
        clientManage->cmdCurrentOpt = AT_CMD_PS_DATA_CALL_END_SET;
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/* 删除At_SetModemStatusForFC函数, 功能和At_SetModemStatus重复 */

VOS_UINT32 AT_ModemEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_MODEM;

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_MODEM, ID_AT_MNTN_RESULT_MODEM);
    /* 清空对应表项 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    /* 填写用户表项 */
    g_atClientTab[indexNum].clientId = AT_CLIENT_ID_MODEM;
    g_atClientTab[indexNum].portNo   = DMS_PORT_MODEM;
    g_atClientTab[indexNum].userType = AT_MODEM_USER;
    g_atClientTab[indexNum].used     = AT_CLIENT_USED;

    /* 以下可以不用填写，前面PS_MEMSET已经初始化，只为可靠起见 */
    g_atClientTab[indexNum].indMode         = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus   = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_MODEM;
    clientInfo.type       = DMS_CLIENT_TYPE_MODEM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_MODEM, &clientInfo);

    AT_NORM_LOG2("AT_ModemEst ucIndex and reg ret is ", indexNum, ret);

    return ret;
}

MODULE_EXPORTED VOS_VOID AT_SetUsbDebugFlag(VOS_UINT32 flag)
{
    g_atUsbDebugFlag = flag;
}

VOS_UINT32 At_UsbPcuiEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_PCUI;

    /* 清空对应表项 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_PCUI, ID_AT_MNTN_RESULT_PCUI);

    /* 填写用户表项 */
    g_atClientTab[indexNum].clientId = AT_CLIENT_ID_PCUI;
    g_atClientTab[indexNum].portNo   = DMS_PORT_PCUI;
    g_atClientTab[indexNum].userType = AT_USBCOM_USER;
    g_atClientTab[indexNum].used     = AT_CLIENT_USED;

    /* 以下可以不用填写，前面PS_MEMSET已经初始化，只为可靠起见 */
    g_atClientTab[indexNum].indMode         = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus   = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_PCUI;
    clientInfo.type       = DMS_CLIENT_TYPE_COM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_PCUI, &clientInfo);

    AT_WARN_LOG2("At_UsbPcuiEst ucIndex and reg ret is ", indexNum, ret);

    return VOS_OK;
}

VOS_UINT32 At_UsbCtrEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_CTRL;

    /* 清空对应表项 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_CTRL, ID_AT_MNTN_RESULT_CTRL);

    /* 填写用户表项 */
    g_atClientTab[indexNum].clientId = AT_CLIENT_ID_CTRL;
    g_atClientTab[indexNum].portNo   = DMS_PORT_CTRL;
    g_atClientTab[indexNum].userType = AT_CTR_USER;
    g_atClientTab[indexNum].used     = AT_CLIENT_USED;

    /* 以下可以不用填写，前面PS_MEMSET已经初始化，只为可靠起见 */
    g_atClientTab[indexNum].indMode         = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus   = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_CTRL;
    clientInfo.type       = DMS_CLIENT_TYPE_COM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_CTRL, &clientInfo);

    AT_WARN_LOG2("At_UsbCtrEst ucIndex and reg ret is ", indexNum, ret);

    return VOS_OK;
}

VOS_UINT32 At_UsbPcui2Est(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_PCUI2;

    /* 清空对应表项 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_PCUI2, ID_AT_MNTN_RESULT_PCUI2);

    /* 填写用户表项 */
    g_atClientTab[indexNum].clientId = AT_CLIENT_ID_PCUI2;
    g_atClientTab[indexNum].portNo   = DMS_PORT_PCUI2;
    g_atClientTab[indexNum].userType = AT_PCUI2_USER;
    g_atClientTab[indexNum].used     = AT_CLIENT_USED;

    /* 以下可以不用填写，前面PS_MEMSET已经初始化，只为可靠起见 */
    g_atClientTab[indexNum].indMode         = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt   = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus   = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_PCUI2;
    clientInfo.type       = DMS_CLIENT_TYPE_COM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_PCUI2, &clientInfo);

    AT_WARN_LOG2("At_UsbPcui2Est ucIndex and reg ret is ", indexNum, ret);

    return VOS_OK;
}

VOS_VOID AT_UartInitPort(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_UART;

    /* 清空对应表项 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_UART, ID_AT_MNTN_RESULT_UART);

    /* 填写用户表项 */
    g_atClientTab[indexNum].clientId = AT_CLIENT_ID_UART;
    g_atClientTab[indexNum].portNo   = DMS_PORT_UART;
    g_atClientTab[indexNum].userType = AT_UART_USER;
    g_atClientTab[indexNum].used     = AT_CLIENT_USED;

    /* 以下可以不用填写，前面PS_MEMSET已经初始化，只为可靠起见 */
    g_atClientTab[indexNum].indMode       = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_UART;
    clientInfo.type       = DMS_CLIENT_TYPE_COM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_UART, &clientInfo);

    AT_WARN_LOG2("AT_UART_InitPort ucIndex and reg ret is ", indexNum, ret);
}

VOS_UINT32 AT_IsUartUserSupportBaudRate(AT_ClientIdUint16 indexNum)
{
#if (FEATURE_UART_BAUDRATE_AUTO_ADAPTION == FEATURE_ON)
    if (g_atClientTab[indexNum].userType == AT_UART_USER) {
        return VOS_TRUE;
    }
#endif

    return VOS_FALSE;
}

VOS_UINT32 AT_UART_IsBaudRateValid(AT_UartBaudrateUint32 baudRate)
{
    VOS_UINT32 ret = VOS_FALSE;

    switch (baudRate) {
        case AT_UART_BAUDRATE_0:
        case AT_UART_BAUDRATE_300:
        case AT_UART_BAUDRATE_600:
        case AT_UART_BAUDRATE_1200:
        case AT_UART_BAUDRATE_2400:
        case AT_UART_BAUDRATE_4800:
        case AT_UART_BAUDRATE_9600:
        case AT_UART_BAUDRATE_19200:
        case AT_UART_BAUDRATE_38400:
        case AT_UART_BAUDRATE_57600:
        case AT_UART_BAUDRATE_115200:
        case AT_UART_BAUDRATE_230400:
        case AT_UART_BAUDRATE_460800:
        case AT_UART_BAUDRATE_921600:
        case AT_UART_BAUDRATE_2764800:
        case AT_UART_BAUDRATE_4000000:
            ret = VOS_TRUE;
            break;

        default:
            ret = VOS_FALSE;
            break;
    }

    return ret;
}

#if (FEATURE_AT_HSUART == FEATURE_ON)
VOS_VOID AT_UART_ConfigDefaultPara(VOS_VOID)
{
    AT_UartPhyCfg *phyCfg = VOS_NULL_PTR;
    phyCfg                = AT_GetUartPhyCfgInfo();

    /* 配置UART波特率 */
    AT_HSUART_ConfigBaudRate(AT_CLIENT_ID_UART, phyCfg->baudRate);
}

VOS_UINT32 AT_HSUART_IsBaudRateValid(AT_UartBaudrateUint32 baudRate)
{
    VOS_UINT32 ret = VOS_FALSE;

    /*
     * ARM   --- 0,300,600,1200,2400,4800,9600,19200,38400,57600,115200,
     * 230400,460800,921600,2764800,4000000
     */
    switch (baudRate) {
        case AT_UART_BAUDRATE_0:
        case AT_UART_BAUDRATE_300:
        case AT_UART_BAUDRATE_600:
        case AT_UART_BAUDRATE_1200:
        case AT_UART_BAUDRATE_2400:
        case AT_UART_BAUDRATE_4800:
        case AT_UART_BAUDRATE_9600:
        case AT_UART_BAUDRATE_19200:
        case AT_UART_BAUDRATE_38400:
        case AT_UART_BAUDRATE_57600:
        case AT_UART_BAUDRATE_115200:
        case AT_UART_BAUDRATE_230400:
        case AT_UART_BAUDRATE_460800:
        case AT_UART_BAUDRATE_921600:
        case AT_UART_BAUDRATE_2764800:
        case AT_UART_BAUDRATE_4000000:
            ret = VOS_TRUE;
            break;

        default:
            ret = VOS_FALSE;
            break;
    }

    return ret;
}

VOS_UINT32 AT_HSUART_IsFormatValid(AT_UartFormatUint8 format)
{
    VOS_UINT32 ret = VOS_FALSE;

    switch (format) {
        case AT_UART_FORMAT_8DATA_2STOP:
        case AT_UART_FORMAT_8DATA_1PARITY_1STOP:
        case AT_UART_FORMAT_8DATA_1STOP:
        case AT_UART_FORMAT_7DATA_2STOP:
        case AT_UART_FORMAT_7DATA_1PARITY_1STOP:
        case AT_UART_FORMAT_7DATA_1STOP:
            ret = VOS_TRUE;
            break;

        default:
            ret = VOS_FALSE;
            break;
    }

    return ret;
}

VOS_UINT32 AT_HSUART_IsParityValid(AT_UartParityUint8 parity)
{
    VOS_UINT32 ret = VOS_FALSE;

    /*
     * 注: 不同版本选用的UART IP不同, 校验规格差异如下
     * V3R3            --- ODD, EVEN, MARK, SPACE
     * V7R11(or later) --- ODD, EVEN
     */
    switch (parity) {
        case AT_UART_PARITY_ODD:
        case AT_UART_PARITY_EVEN:
            ret = VOS_TRUE;
            break;

        default:
            ret = VOS_FALSE;
            break;
    }

    return ret;
}

VOS_UINT32 AT_HSUART_ValidateFlowCtrlParam(AT_UartFcDceByDteUint8 fcDceByDte, AT_UartFcDteByDceUint8 fcDteByDce)
{
    /*
     * 注: 不同版本选用的UART IP不同, 流控规格差异如下
     * V3R3            --- 硬件流控支持上下行单独开启或关闭
     * V7R11(or later) --- 硬件流控支持上下行同时开启或关闭
     */
    if (fcDceByDte != fcDteByDce) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_HSUART_ValidateCharFrameParam(AT_UartFormatUint8 format, AT_UartParityUint8 parity)
{
    /* 检查格式类型是否支持 */
    if (AT_HSUART_IsFormatValid(format) == VOS_FALSE) {
        return VOS_FALSE;
    }

    /* 检查检验方式是否支持 */
    if (AT_HSUART_IsParityValid(parity) == VOS_FALSE) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

const AT_UartFormatParam* AT_HSUART_GetFormatParam(AT_UartFormatUint8 format)
{
    const AT_UartFormatParam   *formatTblPtr = VOS_NULL_PTR;
    const AT_UartFormatParam   *formatParam  = VOS_NULL_PTR;
    VOS_UINT32                  cnt;

    formatTblPtr = AT_UART_GET_FORMAT_TBL_PTR();

    for (cnt = 0; cnt < AT_UART_GET_FORMAT_TBL_SIZE(); cnt++) {
        if (format == formatTblPtr[cnt].format) {
            formatParam = &formatTblPtr[cnt];
        }
    }

    return formatParam;
}

VOS_UINT32 AT_HSUART_GetUdiValueByDataLen(AT_UartDataLenUint8 dataLen, VOS_UINT32 *udiValue)
{
    /* 指针非空检测 */
    if (udiValue == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    /* 映射底软UDI VALUE */
    switch (dataLen) {
        case AT_UART_DATA_LEN_5_BIT:
            *udiValue = DMS_WLEN_5_BITS;
            break;

        case AT_UART_DATA_LEN_6_BIT:
            *udiValue = DMS_WLEN_6_BITS;
            break;

        case AT_UART_DATA_LEN_7_BIT:
            *udiValue = DMS_WLEN_7_BITS;
            break;

        case AT_UART_DATA_LEN_8_BIT:
            *udiValue = DMS_WLEN_8_BITS;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_HSUART_GetUdiValueByStopLen(AT_UartStopLenUint8 stopLen, VOS_UINT32 *udiValue)
{
    /* 指针非空检测 */
    if (udiValue == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    /* 映射底软UDI VALUE */
    switch (stopLen) {
        case AT_UART_STOP_LEN_1_BIT:
            *udiValue = DMS_STP2_OFF;
            break;

        case AT_UART_STOP_LEN_2_BIT:
            *udiValue = DMS_STP2_ON;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_HSUART_GetUdiValueByParity(AT_UartParityUint8 parity, VOS_UINT32 *udiValue)
{
    /* 指针非空检测 */
    if (udiValue == VOS_NULL_PTR) {
        return VOS_ERR;
    }

    /* 映射底软UDI VALUE */
    switch (parity) {
        case AT_UART_PARITY_ODD:
            *udiValue = DMS_PARITY_CHECK_ODD;
            break;

        case AT_UART_PARITY_EVEN:
            *udiValue = DMS_PARITY_CHECK_EVEN;
            break;

        case AT_UART_PARITY_MARK:
            *udiValue = DMS_PARITY_CHECK_MARK;
            break;

        case AT_UART_PARITY_SPACE:
            *udiValue = DMS_PARITY_CHECK_SPACE;
            break;

        default:
            return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_HSUART_ConfigFlowCtrl(VOS_UINT8 indexNum, VOS_UINT32 rtsFlag, VOS_UINT32 ctsFlag)
{
    if (DMS_PORT_ConfigFlowCtrl(DMS_PORT_HSUART, rtsFlag, ctsFlag) != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_HSUART_ConfigCharFrame(VOS_UINT8 indexNum, AT_UartFormatUint8 format, AT_UartParityUint8 parity)
{
    const AT_UartFormatParam   *formatParam = VOS_NULL_PTR;
    VOS_UINT32                  udiDataLenth = DMS_WLEN_8_BITS;
    VOS_UINT32                  udiStpLenth = DMS_STP2_OFF;
    VOS_UINT32                  udiParity = DMS_PARITY_NO_CHECK;
    VOS_UINT32                  result;

    /* 参数初始化 */

    /* 将设置的帧格式和校验方法转换为DRV接口格式 */
    formatParam = AT_HSUART_GetFormatParam(format);
    if (formatParam == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Format is invalid!");
        return VOS_ERR;
    }

    result = AT_HSUART_GetUdiValueByDataLen(formatParam->dataBitLength, &udiDataLenth);
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Data bit length is invalid!");
        return VOS_ERR;
    }

    result = AT_HSUART_GetUdiValueByStopLen(formatParam->stopBitLength, &udiStpLenth);
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Stop bit length is invalid!");
        return VOS_ERR;
    }

    if (formatParam->parityBitLength == AT_UART_PARITY_LEN_1_BIT) {
        result = AT_HSUART_GetUdiValueByParity(parity, &udiParity);
        if (result != VOS_OK) {
            AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Parity bit length is invalid!");
            return VOS_ERR;
        }
    }

    /* 调用DRV函数设置UART帧格式参数（包括串口数据位长度/停止位长度/校验位） */
    if (DMS_PORT_SetCharFrame(DMS_PORT_HSUART, udiDataLenth, udiStpLenth, udiParity) != VOS_OK) {
        AT_ERR_LOG("AT_HSUART_ConfigCharFrame: Set CharFrame failed!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_HSUART_ConfigBaudRate(VOS_UINT8 indexNum, AT_UartBaudrateUint32 baudRate)
{
    /* 调用DRV函数设置串口的波特率 */
    DMS_PortIdUint16 portId = DMS_PORT_BUTT;

    if (AT_CheckHsUartUser(indexNum) == VOS_TRUE) {
        portId = DMS_PORT_HSUART;
    } else if (AT_IsUartUserSupportBaudRate(indexNum) == VOS_TRUE) {
        portId = DMS_PORT_UART;
    } else {
        AT_ERR_LOG1("AT_HSUART_ConfigBaudRate: indexNum invalid!", indexNum);
        return VOS_ERR;
    }

    if (DMS_PORT_SetBaud(portId, baudRate) != VOS_OK) {
        AT_ERR_LOG("AT_HSUART_ConfigBaudRate: Set Baud failed!");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID AT_HSUART_InitLink(VOS_UINT8 indexNum)
{
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));

    g_atClientTab[indexNum].clientId      = AT_CLIENT_ID_HSUART;
    g_atClientTab[indexNum].used          = AT_CLIENT_USED;
    g_atClientTab[indexNum].userType      = AT_HSUART_USER;
    g_atClientTab[indexNum].portNo        = DMS_PORT_HSUART;
    g_atClientTab[indexNum].indMode       = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 AT_HSUART_InitPort(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_HSUART;

    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    /* 配置HSUART端口消息映射 */
    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_HSUART, ID_AT_MNTN_RESULT_HSUART);

    /* 初始化UART链路 */
    AT_HSUART_InitLink(indexNum);

    clientInfo.id         = AT_CLIENT_ID_HSUART;
    clientInfo.type       = DMS_CLIENT_TYPE_HSUART;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_HSUART, &clientInfo);

    AT_WARN_LOG2("AT_HSUART_InitPort ucIndex and reg ret is ", indexNum, ret);

    return ret;
}

VOS_VOID AT_HSUART_ConfigDefaultPara(VOS_VOID)
{
    AT_UartPhyCfg *phyCfg = VOS_NULL_PTR;
    phyCfg                = AT_GetUartPhyCfgInfo();

    /* 配置UART波特率 */
    AT_HSUART_ConfigBaudRate(AT_CLIENT_ID_HSUART, phyCfg->baudRate);

    /* 配置UART帧格式 */
    AT_HSUART_ConfigCharFrame(AT_CLIENT_ID_HSUART, phyCfg->frame.format, phyCfg->frame.parity);
}
#endif

VOS_UINT32 AT_IsHsuartBaseUser(VOS_UINT8 indexNum)
{
    VOS_UINT32 rslt = VOS_FALSE;
#if (FEATURE_AT_HSUART == FEATURE_ON)
    DMS_PortIdUint16 portId = AT_GetDmsPortIdByClientId(indexNum);
    rslt = DMS_PORT_IsHsuartBasePort(portId);
#endif
    return rslt;
}

VOS_INT32 AT_AppComEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum;
    VOS_UINT8             loop;

    for (loop = 0; loop < DMS_APP_PORT_SIZE; loop++) {
        indexNum = AT_CLIENT_ID_APP + loop;

        /* 清空对应表项 */
        (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
        (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

        AT_ConfigTraceMsg(indexNum, (ID_AT_CMD_APP + loop), (ID_AT_MNTN_RESULT_APP + loop));

        g_atClientTab[indexNum].clientId = AT_CLIENT_ID_APP + (VOS_UINT16)loop;

        /* 填写用户表项 */
        g_atClientTab[indexNum].portNo        = DMS_PORT_APP + (VOS_UINT16)loop;
        g_atClientTab[indexNum].userType      = AT_APP_USER;
        g_atClientTab[indexNum].used          = AT_CLIENT_USED;
        g_atClientTab[indexNum].indMode       = AT_IND_MODE;
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;

        clientInfo.id         = indexNum;
        clientInfo.type       = DMS_CLIENT_TYPE_COM;
        clientInfo.RcvCmdData = AT_RcvCmdData;

        ret = DMS_PORT_RegClientInfo(g_atClientTab[indexNum].portNo, &clientInfo);
        AT_WARN_LOG2("AT_AppComEst ucIndex and reg ret is ", indexNum, ret);
    }
    return VOS_OK;
}

VOS_UINT32 AT_SockEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_SOCK;

    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_SOCK, ID_AT_MNTN_RESULT_SOCK);

    g_atClientTab[indexNum].clientId      = AT_CLIENT_ID_SOCK;
    g_atClientTab[indexNum].portNo        = DMS_PORT_SOCK;
    g_atClientTab[indexNum].userType      = AT_SOCK_USER;
    g_atClientTab[indexNum].used          = AT_CLIENT_USED;
    g_atClientTab[indexNum].indMode       = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_SOCK;
    clientInfo.type       = DMS_CLIENT_TYPE_COM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_SOCK, &clientInfo);

    AT_WARN_LOG2("AT_SockEst ucIndex and reg ret is ", indexNum, ret);

    return ret;
}

VOS_UINT32 AT_UsbNdisEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_NDIS;

    /* 清空对应表项 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));
    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    AT_ConfigTraceMsg(indexNum, ID_AT_CMD_NDIS, ID_AT_MNTN_RESULT_NDIS);

    /* 填写用户表项 */
    g_atClientTab[indexNum].clientId = AT_CLIENT_ID_NDIS;
    g_atClientTab[indexNum].portNo   = DMS_PORT_NCM_CTRL;
    g_atClientTab[indexNum].userType = AT_NDIS_USER;
    g_atClientTab[indexNum].used     = AT_CLIENT_USED;

    /* 以下可以不用填写，前面PS_MEMSET已经初始化，只为可靠起见 */
    g_atClientTab[indexNum].indMode       = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;

    clientInfo.id         = AT_CLIENT_ID_NDIS;
    clientInfo.type       = DMS_CLIENT_TYPE_COM;
    clientInfo.RcvCmdData = AT_RcvCmdData;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_NCM_CTRL, &clientInfo);

    AT_WARN_LOG2("AT_UsbNdisEst ucIndex and reg ret is ", indexNum, ret);

    return VOS_OK;
}

VOS_UINT32 AT_UsbNdisDataEst(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;

    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    /* NDIS data 通道不参与at命令处理，所以client id为0 */
    clientInfo.id    = 0;
    clientInfo.type  = DMS_CLIENT_TYPE_NDIS_DATA;

    ret = DMS_PORT_RegClientInfo(DMS_PORT_NCM_DATA, &clientInfo);
#if (FEATURE_MULTI_NCM == FEATURE_ON)
    DMS_PORT_RegClientInfo(DMS_PORT_NCM_DATA1, &clientInfo);
    DMS_PORT_RegClientInfo(DMS_PORT_NCM_DATA2, &clientInfo);
    DMS_PORT_RegClientInfo(DMS_PORT_NCM_DATA3, &clientInfo);
#endif

    AT_WARN_LOG1("AT_UsbNdisDataEst reg ret is ", ret);

    return ret;
}

VOS_UINT32 AT_CheckPcuiCtrlConcurrent(VOS_UINT8 indexOne, VOS_UINT8 indexTwo)
{
    VOS_UINT32 userFlg1;
    VOS_UINT32 userFlg2;

    userFlg1 = AT_CheckUserType(indexOne, AT_USBCOM_USER);
    userFlg1 |= AT_CheckUserType(indexOne, AT_CTR_USER);
    userFlg1 |= AT_CheckUserType(indexOne, AT_PCUI2_USER);

    if (userFlg1 != VOS_TRUE) {
        return VOS_FALSE;
    }

    userFlg2 = AT_CheckUserType(indexTwo, AT_USBCOM_USER);
    userFlg2 |= AT_CheckUserType(indexTwo, AT_CTR_USER);
    userFlg2 |= AT_CheckUserType(indexTwo, AT_PCUI2_USER);

    if (userFlg2 != VOS_TRUE) {
        return VOS_FALSE;
    }

    if (g_atClientTab[indexOne].userType != g_atClientTab[indexTwo].userType) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_IsConcurrentPorts(VOS_UINT8 indexOne, VOS_UINT8 indexTwo)
{
    VOS_UINT32 appUserFlg1;
    VOS_UINT32 appUserFlg2;

    /* 同一个通道不支持并发，由外层函数保证 */
    appUserFlg1 = AT_CheckAppUser(indexOne);
    appUserFlg2 = AT_CheckAppUser(indexTwo);

    /* 通道1是APP通道,通道2也是APP通道 */
    if (appUserFlg1 == VOS_TRUE) {
        if (appUserFlg2 == VOS_TRUE) {
            return VOS_TRUE;
        }
    }

    /* PCUI和CTRL口并发判断，仅供测试用 */
    if (AT_GetPcuiCtrlConcurrentFlag() == VOS_TRUE) {
        if (AT_CheckPcuiCtrlConcurrent(indexOne, indexTwo) == VOS_TRUE) {
            return VOS_TRUE;
        }
    }

    return VOS_FALSE;
}

VOS_UINT32 AT_CheckAppUser(VOS_UINT8 indexNum)
{
    if (g_atClientTab[indexNum].userType != AT_APP_USER) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_CheckUartUser(VOS_UINT8 indexNum)
{
    if (g_atClientTab[indexNum].userType == AT_UART_USER) {
        return VOS_TRUE;
    }
    return VOS_FALSE;
}

VOS_UINT32 AT_CheckHsUartUser(AT_ClientIdUint16 indexNum)
{
#if (FEATURE_AT_HSUART == FEATURE_ON)
    if (g_atClientTab[indexNum].userType == AT_HSUART_USER) {
        return VOS_TRUE;
    }
#endif

    return VOS_FALSE;
}

VOS_UINT32 AT_CheckModemUser(VOS_UINT8 indexNum)
{
    if (g_atClientTab[indexNum].userType != AT_MODEM_USER) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_CheckUserType(VOS_UINT8 indexNum, AT_USER_TYPE userType)
{
    if (userType != g_atClientTab[indexNum].userType) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

/*
 * 功能描述: 获取消息个数是否达到上限状态
 */
VOS_UINT32 AT_MsgNumInPoolNotFull(VOS_UINT32 msgType)
{
    AT_CmdMsgNumCtrl   *msgNumCtrlCtx = VOS_NULL_PTR;
    AT_Msg_QostypeUint8 qosType;

    msgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();
    qosType = (msgType == AT_ATNLPROXY_MSG) ? AT_MSG_QOSTYPE_ATNLPROXY : AT_MSG_QOSTYPE_NORMAL;
    return ((msgNumCtrlCtx->msgCount[qosType] <= AT_GetMaxCtrlNum(qosType)) ? VOS_TRUE : VOS_FALSE);
}

VOS_UINT32 AT_MsgNumAdd(AT_Msg *msg, TAF_UINT8 type)
{
    AT_CmdMsgNumCtrl   *msgNumCtrlCtx = VOS_NULL_PTR;
    VOS_ULONG           lockLevel;
    AT_Msg_QostypeUint8 qosType;

    msgNumCtrlCtx = AT_GetMsgNumCtrlCtxAddr();

    if ((type == AT_NORMAL_TYPE_MSG) || (type == AT_ATNLPROXY_MSG)) {
        qosType = (type == AT_ATNLPROXY_MSG) ? AT_MSG_QOSTYPE_ATNLPROXY : AT_MSG_QOSTYPE_NORMAL;
        if (msgNumCtrlCtx->msgCount[qosType] > AT_GetMaxCtrlNum(qosType)) {
            AT_WARN_LOG1("AT_MsgNumAdd : type %d msg num full!", qosType);
            /*lint -save -e516 */
            /* 释放分配的内存空间 */
            PS_FREE_MSG(WUEPS_PID_AT, msg);
            /*lint -restore */

            return VOS_FALSE;
        }

        lockLevel = 0;
        /*lint -e571*/
        VOS_SpinLockIntLock(&(msgNumCtrlCtx->spinLock), lockLevel);
        /*lint +e571*/

        msgNumCtrlCtx->msgCount[qosType]++;

        VOS_SpinUnlockIntUnlock(&(msgNumCtrlCtx->spinLock), lockLevel);
    }
    return VOS_TRUE;
}

/*
 * 功能描述: 检查发送命令消息参数合法性
 */
LOCAL TAF_UINT32 At_CheckCmdMsg(TAF_UINT8 *data, TAF_UINT16 len)
{
    if (data == VOS_NULL_PTR) {
        AT_WARN_LOG("At_CheckCmdMsg :pData is null ptr!");
        return AT_FAILURE;
    }

    if (len == 0) {
        AT_WARN_LOG("At_CheckCmdMsg ulLength = 0");
        return AT_FAILURE;
    }

    if (len > AT_COM_BUFF_LEN) {
        AT_WARN_LOG("At_CheckCmdMsg ulLength > AT_COM_BUFF_LEN");
        return AT_FAILURE;
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SendCmdMsg(TAF_UINT8 indexNum, TAF_UINT8 *data, TAF_UINT16 len, TAF_UINT8 type)
{
    AT_Msg           *msg           = TAF_NULL_PTR;
    VOS_UINT_PTR      tmpAddr;
    VOS_UINT32        length;
    errno_t           memResult;

    ModemIdUint16 modemId;

    if (At_CheckCmdMsg(data, len) == AT_FAILURE) {
        return AT_FAILURE;
    }

    /* 增加自定义的ITEM，共4个字节 */
    length = AT_GetAtMsgStruMsgLength(len);
    msg = (AT_Msg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);
    if (msg == TAF_NULL_PTR) {
        AT_ERR_LOG("At_SendCmdMsg:ERROR:Alloc Msg");
        return AT_FAILURE;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_AT, length);

    if (AT_MsgNumAdd(msg, type) == VOS_FALSE) {
        return AT_FAILURE;
    }

    if (type == AT_COMBIN_BLOCK_MSG) {
        msg->msgId = ID_AT_COMBIN_BLOCK_CMD;
    } else {
        msg->msgId = AT_GetCmdMsgID(indexNum);
    }

    msg->type     = type;     /* 类型 */
    msg->indexNum = indexNum; /* 索引 */
    msg->len      = len;      /* 长度 */

    modemId = MODEM_ID_0;
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        modemId = MODEM_ID_0;
    }

    msg->modemId = (VOS_UINT8)modemId;
    /* 版本信息 */
    msg->versionId    = 0xAA;
    msg->filterAtType = (VOS_UINT8)g_logPrivacyAtCmd;

    (VOS_VOID)memset_s(msg->value, sizeof(msg->value), 0x00, sizeof(msg->value));

    msg->userType = AT_GetUserTypeFromIndex(indexNum);

    /* 填写新消息内容 */
    tmpAddr   = (VOS_UINT_PTR)(msg->value);
    memResult = memcpy_s((VOS_UINT8 *)tmpAddr, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    /* 发送消息到AT_PID; */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != 0) {
        AT_ERR_LOG("At_SendCmdMsg:ERROR:VOS_SendMsg");

        /* 由于消息发送失败时，会触发整机复位，故此处不做ulMsgCount--操作 */

        return AT_FAILURE;
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_IsApPort(VOS_UINT8 indexNum)
{
    VOS_UINT32 vcomUserFlg;
    VOS_UINT8 *systemAppConfig = VOS_NULL_PTR;

    if (g_atDebugInfo.unCheckApPortFlg == 0) {
        /* 初始化 */
        systemAppConfig = AT_GetSystemAppConfigAddr();
        vcomUserFlg     = AT_CheckAppUser(indexNum);

        if (*systemAppConfig == SYSTEM_APP_ANDROID) {
            /* 如果是手机形态，需要判断HSIC端口，MUX端口，VCOM端口 */
            if (vcomUserFlg == VOS_FALSE) {
                return VOS_FALSE;
            }
#if (FEATURE_PHONE_SC == FEATURE_ON)
        } else if (*systemAppConfig == SYSTEM_APP_MP) {
            if (vcomUserFlg == VOS_FALSE) {
                return VOS_FALSE;
            }
#endif
        } else {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_OFF)

STATIC VOS_INT AT_ProcCCpuResetBefore(VOS_VOID)
{
    AT_Msg *msg = VOS_NULL_PTR;

    AT_PR_LOGI("enter, %u", VOS_GetSlice());

    /* 设置处于复位前的标志 */
    AT_SetResetFlag(VOS_TRUE);

    dms_reset_modem_status();

    /* 清除TAFAGENT所有的信号量 */
    TAF_AGENT_ClearAllSem();

    /* 构造消息 */
    /*lint -save -e516 */
    msg = (AT_Msg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(AT_Msg));
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        AT_PR_LOGE("alloc msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_AT, sizeof(AT_Msg) - VOS_MSG_HEAD_LENGTH);
    msg->type = ID_CCPU_AT_RESET_START_IND;

    msg->msgId = ID_AT_COMM_CCPU_RESET_START;

    /* 发消息 */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_PR_LOGE("send msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* 等待回复信号量初始为锁状态，等待消息处理完后信号量解锁。 */
    if (VOS_SmP(AT_GetResetSem(), AT_RESET_TIMEOUT_LEN) != VOS_OK) {
        AT_PR_LOGE("VOS_SmP fail, %u", VOS_GetSlice());
        AT_DBG_LOCK_BINARY_SEM_FAIL_NUM(1);

        return VOS_ERROR;
    }

    /* 记录复位前的次数 */
    AT_DBG_SAVE_CCPU_RESET_BEFORE_NUM(1);

    AT_PR_LOGI("succ, %u", VOS_GetSlice());

    return VOS_OK;
}

STATIC VOS_INT AT_ProcCCpuResetAfter(VOS_VOID)
{
    AT_Msg *msg = VOS_NULL_PTR;

    AT_PR_LOGI("enter, %u", VOS_GetSlice());

    /* 构造消息 */
    /*lint -save -e516 */
    msg = (AT_Msg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(AT_Msg));
    /*lint -restore */
    if (msg == VOS_NULL_PTR) {
        AT_PR_LOGE("alloc msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_AT, sizeof(AT_Msg) - VOS_MSG_HEAD_LENGTH);
    msg->type = ID_CCPU_AT_RESET_END_IND;

    msg->msgId = ID_AT_COMM_CCPU_RESET_END;

    /* 发消息 */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_PR_LOGE("send msg fail, %u", VOS_GetSlice());
        return VOS_ERROR;
    }

    /* 记录复位后的次数 */
    AT_DBG_SAVE_CCPU_RESET_AFTER_NUM(1);

    AT_PR_LOGI("succ, %u", VOS_GetSlice());

    return VOS_OK;
}

STATIC VOS_INT AT_CCpuResetCallback(drv_reset_cb_moment_e param, VOS_INT iUserData)
{
    /* 复位前 */
    if (param == MDRV_RESET_CB_BEFORE) {
        return AT_ProcCCpuResetBefore();
    }
    /* 复位后 */
    else if (param == MDRV_RESET_CB_AFTER) {
        return AT_ProcCCpuResetAfter();
    } else {
        return VOS_ERROR;
    }
}

STATIC VOS_INT AT_HifiResetCallback(enum DRV_RESET_CALLCBFUN_MOMENT param, VOS_INT iUserData)
{
    AT_Msg *msg = VOS_NULL_PTR;

    /* 参数为0表示复位前调用 */
    if (param == DRV_RESET_CALLCBFUN_RESET_BEFORE) {
        AT_PR_LOGI("before reset enter, %u", VOS_GetSlice());
        /* 构造消息 */
        /*lint -save -e516 */
        msg = (AT_Msg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(AT_Msg));
        /*lint -restore */
        if (msg == VOS_NULL_PTR) {
            AT_PR_LOGE("before reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        /* 填写消息头 */
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_AT, sizeof(AT_Msg) - VOS_MSG_HEAD_LENGTH);
        msg->type = ID_HIFI_AT_RESET_BEGIN_IND;

        msg->msgId = ID_AT_COMM_HIFI_RESET_START;

        /* 发消息 */
        if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
            AT_PR_LOGE("after reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        return VOS_OK;
    }
    /* 复位后 */
    else if (param == DRV_RESET_CALLCBFUN_RESET_AFTER) {
        AT_PR_LOGI("after reset enter, %u", VOS_GetSlice());
        /* 构造消息 */
        /*lint -save -e516 */
        msg = (AT_Msg *)TAF_AllocMsgWithHeaderLen(WUEPS_PID_AT, sizeof(AT_Msg));
        /*lint -restore */
        if (msg == VOS_NULL_PTR) {
            AT_PR_LOGE("after reset alloc msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }

        /* 填写消息头 */
        TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_AT, sizeof(AT_Msg) - VOS_MSG_HEAD_LENGTH);
        msg->type = ID_HIFI_AT_RESET_END_IND;

        msg->msgId = ID_AT_COMM_HIFI_RESET_END;

        /* 发消息 */
        if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
            AT_PR_LOGE("after reset send msg fail, %u", VOS_GetSlice());
            return VOS_ERROR;
        }
        return VOS_OK;
    } else {
        return VOS_ERROR;
    }
}

VOS_VOID AT_RegResetCallbackFunc(VOS_VOID)
{
    /* 给低软注册回调函数，用于C核单独复位的处理 */
    AT_RegisterSysbootResetNotify(NAS_AT_FUNC_PROC_NAME, AT_CCpuResetCallback, 0, ACPU_RESET_PRIORITY_AT);

    /* 给低软注册回调函数，用于HIFI单独复位的处理 */
    hifireset_regcbfunc(NAS_AT_FUNC_PROC_NAME, AT_HifiResetCallback, 0, ACPU_RESET_PRIORITY_AT);
}
#endif

VOS_VOID AT_ModemEnableCB(VOS_UINT8 indexNum, VOS_UINT32 enable)
{
    AT_ClientManage        *clientManage = VOS_NULL_PTR;
    DMS_PortDataModeUint8   dataMode;

    clientManage = AT_GetClientManage(indexNum);
    dataMode     = DMS_PORT_GetDataMode(clientManage->portNo);

    /*
     * 设备默认处于生效状态，有数据就通过读回调接收，
     * 　 设备失效时，根据当前状态，通知PPP，如处于数传态，
     *    则通知AT去激活PDP.
     */
    if (enable == PS_FALSE) {
        if (dataMode == DMS_PORT_DATA_PPP) {
            AT_SendRelPppReq(clientManage->pppId, PPP_AT_CTRL_REL_PPP_REQ);

            /* 若原先开启了流控，则需停止流控 */
            if (DMS_PORT_IsCtsDeasserted(clientManage->portNo) == VOS_TRUE) {
                /* 通过拉高cts，停止流控 */
                DMS_PORT_AssertCts(clientManage->portNo);
            }

            /* 断开拨号 */
            if (TAF_PS_PPP_DIAL_DOWN(WUEPS_PID_AT, AT_PS_BuildExClientId(clientManage->clientId), 0,
                               clientManage->cid) != VOS_OK) {
                AT_ERR_LOG("AT_ModemEnableCB: TAF_PS_CallEnd failed in <AT_PPP_DATA_MODE>!");
                return;
            }
        } else if (dataMode == DMS_PORT_DATA_PPP_RAW) {
            AT_SendRelPppReq(clientManage->pppId, PPP_AT_CTRL_REL_PPP_RAW_REQ);

            /* 若原先开启了流控，则需停止流控 */
            if (DMS_PORT_IsCtsDeasserted(clientManage->portNo) == VOS_TRUE) {
                /* 通过拉高cts，停止流控 */
                DMS_PORT_AssertCts(clientManage->portNo);
            }

            /* 断开拨号 */
            if (TAF_PS_PPP_DIAL_DOWN(WUEPS_PID_AT, AT_PS_BuildExClientId(clientManage->clientId), 0,
                               clientManage->cid) != VOS_OK) {
                AT_ERR_LOG("AT_ModemEnableCB: TAF_PS_CallEnd failed in <AT_IP_DATA_MODE>!");
                return;
            }
        } else {
            /* 空的else分支，避免PCLINT报错 */
        }

        /* 停止定时器 */
        AT_StopRelTimer(indexNum, &clientManage->hTimer);

        /*
         * 管脚信号修改后，At_ModemRelInd函数只可能在USB被拔出的时候调用，
         * 为了达到Modem口always-on的目的，此时需要将该AT链路的状态迁入
         * 到正常的命令状态:
         */
        DMS_PORT_ResumeCmdMode(clientManage->portNo);
        clientManage->indMode       = AT_IND_MODE;
        clientManage->cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
    }
}

#if (FEATURE_IOT_CMUX == FEATURE_ON)

VOS_VOID AT_CMUX_InitLink(VOS_UINT8 indexNum)
{
    VOS_UINT8 offset = 0;

    if (indexNum < AT_CLIENT_ID_CMUXAT) {
        return;
    }

    offset = indexNum - AT_CLIENT_ID_CMUXAT;
    (VOS_VOID)memset_s(&g_atClientTab[indexNum], sizeof(AT_ClientManage), 0x00, sizeof(AT_ClientManage));

    g_atClientTab[indexNum].clientId      = AT_CLIENT_ID_CMUXAT + offset;
    g_atClientTab[indexNum].used          = AT_CLIENT_USED;
    g_atClientTab[indexNum].userType      = AT_CMUXAT_USER + offset;
    g_atClientTab[indexNum].portNo        = DMS_PORT_CMUXAT + offset;
    g_atClientTab[indexNum].indMode       = AT_IND_MODE;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURRENT_OPT_BUTT;
    g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_READY;
    /*初始化的时候，usDataId/usCtrlId与usClientId相同*/
    g_atClientTab[indexNum].dataId = indexNum;
    g_atClientTab[indexNum].ctrlId = indexNum;
}

VOS_VOID AT_CMUX_InitPort(VOS_VOID)
{
    struct DMS_ClientInfo clientInfo;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = AT_CLIENT_ID_CMUXAT;
    VOS_UINT8             i = 0;

    (VOS_VOID)memset_s(&clientInfo, sizeof(struct DMS_ClientInfo), 0x00, sizeof(struct DMS_ClientInfo));

    /* 配置端口消息映射--TO DO */
    for (i = 0; i < MAX_CMUX_PORT_NUM; i++) {
        indexNum = AT_CLIENT_ID_CMUXAT + i;
        /* 配置CMUX端口消息映射 */
        AT_ConfigTraceMsg(indexNum, ID_AT_CMD_CMUXAT + i,
                                        ID_AT_MNTN_RESULT_CMUXAT + i);
        /* 初始化链路 */
        AT_CMUX_InitLink(indexNum);

        clientInfo.id         = AT_CLIENT_ID_CMUXAT + i;
        clientInfo.type       = DMS_CLIENT_TYPE_CMUX;
        clientInfo.RcvCmdData = AT_RcvCmdData;

        ret = DMS_PORT_RegClientInfo(DMS_PORT_CMUXAT + i, &clientInfo);

        AT_WARN_LOG2("AT_CMUX_InitPort ucIndex and reg ret is ", indexNum, ret);
    }
}

VOS_UINT32 AT_CheckCmuxUser(VOS_UINT8 indexNum)
{
    if (DMS_PORT_GetCmuxOpenFlg() == VOS_FALSE) {
        return VOS_FALSE;
    }

    if ((g_atClientTab[indexNum].userType == AT_CMUXAT_USER) ||
        (g_atClientTab[indexNum].userType == AT_CMUXMDM_USER)||
        (g_atClientTab[indexNum].userType == AT_CMUXEXT_USER)||
        (g_atClientTab[indexNum].userType == AT_CMUXGPS_USER)) {
        return VOS_TRUE;
    }

    return VOS_FALSE;
}
#endif

VOS_UINT32 AT_GetMaxCtrlNum(AT_Msg_QostypeUint8 qosType)
{
    return ((qosType == AT_MSG_QOSTYPE_NORMAL) ? AT_MAX_MSG_NUM : AT_MAX_MSG_NUM_ATNLPROXY);
}

