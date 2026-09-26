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

#include "at_general_taf_qry_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_snd_msg.h"
#include "at_mta_interface.h"
#include "at_cmd_proc.h"
#include "at_taf_agent_interface.h"
#include "at_device_cmd.h"
#include "at_mdrv_interface.h"
#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_TAF_QRY_CMD_PROC_C
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_QryS3Para(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%03d", g_atS3);
    return AT_OK;
}
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_QryS4Para(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%03d", g_atS4);
    return AT_OK;
}
/*
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_QryS5Para(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%03d", g_atS5);
    return AT_OK;
}

/*
 * History:
 *  1.Date: 2009-05-27
 *    Modification: Created function
 */
TAF_UINT32 At_QryS6Para(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%03d", g_atS6);
    return AT_OK;
}
/*
 * History:
 *  1.Date: 2009-05-27
 *    Modification: Created function
 */
TAF_UINT32 At_QryS7Para(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%03d", g_atS7);
    return AT_OK;
}

TAF_UINT32 At_QryCscsPara(TAF_UINT8 indexNum)
{
    if (g_atCscsType == AT_CSCS_IRA_CODE) {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: \"IRA\"", g_parseContext[indexNum].cmdElement->cmdName);
    } else if (g_atCscsType == AT_CSCS_UCS2_CODE) {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: \"UCS2\"", g_parseContext[indexNum].cmdElement->cmdName);
    } else {
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: \"GSM\"", g_parseContext[indexNum].cmdElement->cmdName);
    }
    return AT_OK;
}

TAF_UINT32 At_QryCmeePara(TAF_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, g_atCmeeType);
    return AT_OK;
}

TAF_UINT32 At_QryCrcPara(TAF_UINT8 indexNum)
{
    AT_ModemSsCtx *ssCtx = VOS_NULL_PTR;

    ssCtx = AT_GetModemSsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, ssCtx->crcType);
    return AT_OK;
}

TAF_UINT32 At_QryCurcPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 result;

    AT_PR_LOGI("Rcv Msg");

    /* AT 给MTA 发送CURC查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_CURC_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("At_QryCurcPara: send Msg fail.");
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CURC_READ;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_QryU2DiagPara(TAF_UINT8 indexNum)
{
    AT_USB_EnumStatus usbEnumStatus;
    NV_PID_EnableType pidEnableType;

    pidEnableType.pidEnabled = VOS_FALSE;

    (VOS_VOID)memset_s(&usbEnumStatus, sizeof(usbEnumStatus), 0x00, sizeof(usbEnumStatus));

    /* 读取PID使能NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PID_ENABLE_TYPE, &pidEnableType, sizeof(NV_PID_EnableType)) != NV_OK) {
        return AT_ERROR;
    }

    /* NV读取成功，检查PID是否使能，若使能，返回ERROR */
    if (pidEnableType.pidEnabled != VOS_FALSE) {
        return AT_ERROR;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_USB_ENUM_STATUS, &usbEnumStatus, sizeof(AT_USB_EnumStatus)) != NV_OK) {
        return AT_ERROR;
    }

    if (usbEnumStatus.status == 0) {
        /* 若NV_ITEM_USB_ENUM_STATUS未设置，则调用底软的API来获取默认的USB设备形态 */
        usbEnumStatus.value = DRV_GET_U2DIAG_DEFVALUE();
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, usbEnumStatus.value);
    return AT_OK;
}

VOS_UINT32 At_QryPort(VOS_UINT8 indexNum)
{
    VOS_UINT32                i;
    VOS_UINT32                j;
    VOS_UINT16                length = 0;
    VOS_UINT8                 count = 0;
    VOS_UINT32                result;
    DRV_DYNAMIC_PID_TYPE_STRU dynamicPidType;
    NV_PID_EnableType         pidEnableType;

    pidEnableType.pidEnabled = VOS_FALSE;

    (VOS_VOID)memset_s(&dynamicPidType, sizeof(dynamicPidType), 0x00, sizeof(DRV_DYNAMIC_PID_TYPE_STRU));

    /* 读取PID使能NV项 */
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PID_ENABLE_TYPE, &pidEnableType, sizeof(NV_PID_EnableType)) != NV_OK) {
        AT_ERR_LOG("At_QryPort:Read NV fail!");
        return AT_ERROR;
    }

    /* NV读取成功，检查PID是否使能，若不使能，返回ERROR */
    if (pidEnableType.pidEnabled != VOS_TRUE) {
        AT_WARN_LOG("At_QryPort:The PID is not enabled!");
        return AT_ERROR;
    }

    /* 查询当前端口形态 */
    result = DRV_SET_PORT_QUIRY(&dynamicPidType);
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("At_QryPort:Qry the pot type fail!");
        return AT_ERROR;
    }

    /* ^SETPORT: */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

    /* OLD PORT */
    for (i = 0; i < AT_SETPORT_PARA_MAX_LEN; i++) {
        for (j = 0; j < AT_SETPORT_DEV_MAP_LEN; j++) {
            if (dynamicPidType.aucFirstPortStyle[i] == g_setPortParaMap[j].drvPara) {
                /* 大于1个参数的显示格式: */
                if (count > 0) {
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",");
                }

                /* 参数 */
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_setPortParaMap[j].atSetPara);

                count++;
                break;
            }
        }
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ";");

    count = 0;

    /* CUR PORT */
    for (i = 0; i < AT_SETPORT_PARA_MAX_LEN; i++) {
        for (j = 0; j < AT_SETPORT_DEV_LEN; j++) {
            if (dynamicPidType.aucRewindPortStyle[i] == g_setPortParaMap[j].drvPara) {
                /* 大于1个参数的显示格式: */
                if (count > 0) {
                    /* 回车换行 */
                    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + length, ",");
                }

                /* 参数 */
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_setPortParaMap[j].atSetPara);

                count++;

                break;
            }
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_GetPortInfoIndex(VOS_UINT8 portType, VOS_UINT8 *indexNum)
{
    VOS_UINT32 loop;

    for (loop = 0; loop < AT_SETPORT_DEV_LEN; loop++) {
        if (portType == g_setPortParaMap[loop].drvPara) {
            *indexNum = (VOS_UINT8)loop;
            return VOS_OK;
        }
    }

    return VOS_ERR;
}

VOS_UINT32 AT_TestSetPort(VOS_UINT8 indexNum)
{
    VOS_UINT32 i;
    VOS_UINT16 length;
    VOS_UINT32 ret;
    VOS_UINT32 availablePortTypeNum = 0;
    VOS_UINT8  availablePortType[AT_SETPORT_PARA_MAX_LEN];
    VOS_UINT8  portIndex;

    (VOS_VOID)memset_s(availablePortType, sizeof(availablePortType), 0x00, sizeof(availablePortType));

    ret = (VOS_UINT32)DRV_USB_GET_AVAILABLE_PORT_TYPE(availablePortType, &availablePortTypeNum,
                                                      AT_SETPORT_PARA_MAX_LEN);
    if (ret != VOS_OK) {
        return AT_ERROR;
    }

    length = 0;
    availablePortTypeNum = AT_MIN(availablePortTypeNum, AT_SETPORT_PARA_MAX_LEN);
    for (i = 0; i < availablePortTypeNum; i++) {
        ret = AT_GetPortInfoIndex(availablePortType[i], &portIndex);
        if (ret != VOS_OK) {
            return AT_ERROR;
        }

        /* ^SETPORT: */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        /* 参数 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %s", g_setPortParaMap[portIndex].atSetPara,
            g_setPortParaMap[portIndex].atDispPara);

        if (i < (availablePortTypeNum - 1)) {
            /* 回车换行 */
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 At_QryPcscInfo(TAF_UINT8 indexNum)
{
    VOS_UINT32 portState;

    /* PCSC 口的开关状态, 0 打开; 1 关闭 */
    if (AT_ExistSpecificPort(AT_DEV_PCSC) == VOS_TRUE) {
        portState = VOS_TRUE;
    } else {
        portState = VOS_FALSE;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, portState);
    return AT_OK;
}

/*
 * History:
 *  1.Date: 2009-07-13
 *    Modification: Created function
 */
TAF_UINT32 At_QryGetportmodePara(TAF_UINT8 indexNum)
{
    /* 根据需求直接返回ERROR */
    return AT_ERROR;
}

VOS_UINT32 AT_QryFclassPara(VOS_UINT8 indexNum)
{
    AT_WARN_LOG("AT_QryFclassPara: Not support this command!");
    (VOS_VOID)indexNum;
    return AT_ERROR;
}

VOS_UINT32 At_TestFclass(VOS_UINT8 indexNum)
{
    AT_WARN_LOG("At_TestFclass: Not support this command!");
    (VOS_VOID)indexNum;
    return AT_ERROR;
}

VOS_UINT32 AT_QryGciPara(VOS_UINT8 indexNum)
{
    AT_WARN_LOG("AT_QryGciPara: Not support this command!");
    (VOS_VOID)indexNum;
    return AT_ERROR;
}

VOS_UINT32 At_TestGci(VOS_UINT8 indexNum)
{
    AT_WARN_LOG("At_TestGci: Not support this command!");
    (VOS_VOID)indexNum;
    return AT_ERROR;
}

