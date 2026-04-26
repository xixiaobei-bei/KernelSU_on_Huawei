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

#include "at_mntn.h"
#include "securec.h"
#include "at_input_proc.h"
#include "at_ctx.h"
#include "at_data_proc.h"
#include "taf_log_privacy_match.h"
#include "at_cmd_proc.h"
#include "taf_acore_log_privacy.h"
#include "mn_comm_api.h"
#include "gu_nas_log_filter.h"
#include "at_mdrv_interface.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_MNTN_C
#define AT_HSUART_PARITY_MAX_LEN 20
#define AT_HSUART_DCE_BY_DTE_MAX_LEN 20
#define AT_HSUART_DTE_BY_DCE_MAX_LEN 20
#define AT_CLIENT_STATE_MAX_LEN 20
#define AT_CLIENT_MODE_MAX_LEN 20

AT_DebugInfo g_atDebugInfo = {VOS_FALSE};

AT_MntnStats g_atStatsInfo;

AT_MntnMsgRecordInfo g_atMsgRecordInfo;

AT_MntnPortInfoLog g_atMntnPortInfo[AT_MNTN_MAX_PORT_NUM];

VOS_UINT8 *g_atExcAddr = VOS_NULL_PTR;

VOS_VOID AT_InitMntnCtx(VOS_VOID)
{
    (VOS_VOID)memset_s(&g_atMsgRecordInfo, sizeof(g_atMsgRecordInfo), 0x00, sizeof(g_atMsgRecordInfo));
    (VOS_VOID)memset_s(g_atMntnPortInfo, sizeof(g_atMntnPortInfo), 0x00, sizeof(g_atMntnPortInfo));
}

VOS_VOID AT_SetPcuiCtrlConcurrentFlag(VOS_UINT8 flag)
{
    g_atDebugInfo.pcuiCtrlConcurrentFlg = flag;
}

VOS_UINT8 AT_GetPcuiCtrlConcurrentFlag(VOS_VOID)
{
    return g_atDebugInfo.pcuiCtrlConcurrentFlg;
}

VOS_VOID AT_SetPcuiPsCallFlag(VOS_UINT8 flag, VOS_UINT8 indexNum)
{
    g_atDebugInfo.pcuiPsCallFlg = flag;
    g_atDebugInfo.pcuiUserId    = AT_CLIENT_ID_APP;
}

VOS_UINT8 AT_GetPcuiPsCallFlag(VOS_VOID)
{
    return g_atDebugInfo.pcuiPsCallFlg;
}

VOS_UINT8 AT_GetPcuiUsertId(VOS_VOID)
{
    return g_atDebugInfo.pcuiUserId;
}

VOS_VOID AT_SetCtrlPsCallFlag(VOS_UINT8 flag, VOS_UINT8 indexNum)
{
    g_atDebugInfo.ctrlPsCallFlg = flag;
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    g_atDebugInfo.ctrlUserId = AT_CLIENT_ID_APP5;
#else
    g_atDebugInfo.ctrlUserId  = AT_CLIENT_ID_APP;
#endif
}

VOS_UINT8 AT_GetCtrlPsCallFlag(VOS_VOID)
{
    return g_atDebugInfo.ctrlPsCallFlg;
}

VOS_UINT8 AT_GetCtrlUserId(VOS_VOID)
{
    return g_atDebugInfo.ctrlUserId;
}

VOS_VOID AT_SetPcui2PsCallFlag(VOS_UINT8 flag, VOS_UINT8 indexNum)
{
    g_atDebugInfo.pcui2PsCallFlg = flag;
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    g_atDebugInfo.pcui2UserId = AT_CLIENT_ID_APP20;
#else
    g_atDebugInfo.pcui2UserId = AT_CLIENT_ID_APP;
#endif /* #if FEATURE_ON == FEATURE_VCOM_EXT */
}

VOS_UINT8 AT_GetPcui2PsCallFlag(VOS_VOID)
{
    return g_atDebugInfo.pcui2PsCallFlg;
}

VOS_UINT8 AT_GetPcui2UserId(VOS_VOID)
{
    return g_atDebugInfo.pcui2UserId;
}

VOS_VOID AT_SetUnCheckApPortFlg(VOS_UINT8 flag)
{
    if ((flag == 0) || (flag == 1)) {
        g_atDebugInfo.unCheckApPortFlg = flag;
    }
}

VOS_VOID AT_MntnTraceEvent(struct MsgCB *msg)
{
    VOS_VOID *logPrivacyMsg = VOS_NULL_PTR;

    /* at模块内部维测消息, 增加脱敏处理 */
    logPrivacyMsg = msg;

    if (AT_GetPrivacyFilterEnableFlg() == VOS_TRUE) {
#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
        if (GUNAS_FilterAtToAtMsg((PS_MsgHeader *)msg) == VOS_NULL_PTR) {
            return;
        }

        /* cnas at命令脱敏过滤函数处理 */
        logPrivacyMsg = AT_PrivacyMatchAtCmd((struct MsgCB *)msg);
        if (logPrivacyMsg == VOS_NULL_PTR) {
            return;
        }
#endif
    }

    mdrv_diag_trace_report(logPrivacyMsg);

#if (OSA_CPU_ACPU == VOS_OSA_CPU || FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
    /* 如果脱敏处理函数申请了新的at命令字符串，释放掉 */
    if (logPrivacyMsg != msg) {
        VOS_MemFree(WUEPS_PID_AT, logPrivacyMsg);
    }
#endif
}

VOS_VOID AT_MntnTraceCmdResult(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT16 dataLen)
{
    AT_Msg             *msg = VOS_NULL_PTR;
    AT_InterMsgIdUint32 eventId;
    VOS_UINT32          length;
    errno_t             memResult;
    ModemIdUint16 modemId;

    length = AT_GetAtMsgStruMsgLength(dataLen);

    /* 申请消息内存 */
    msg = (AT_Msg *)PS_MEM_ALLOC(WUEPS_PID_AT, (length + VOS_MSG_HEAD_LENGTH));
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_MNTN_TraceCmdResult:ERROR:Alloc Mem Fail.");
        return;
    }
    (VOS_VOID)memset_s((VOS_CHAR *)msg + VOS_MSG_HEAD_LENGTH, length, 0x00, length);

    /* 填写消息头 */
    eventId = AT_GetResultMsgID(indexNum);
    AT_MNTN_CFG_MSG_HDR(msg, eventId, length);

    /* 填写消息内容 */
    msg->type     = 0x1;
    msg->indexNum = indexNum;
    msg->len      = dataLen;

    modemId = MODEM_ID_0;
    if (AT_GetModemIdFromClient(indexNum, &modemId) != VOS_OK) {
        modemId = MODEM_ID_0;
    }

    msg->modemId   = (VOS_UINT8)modemId;
    msg->versionId = 0xAA;

    msg->filterAtType = (VOS_UINT8)g_logPrivacyAtCmd;

    msg->userType = AT_GetUserTypeFromIndex(indexNum);

    if (dataLen > 0) {
        memResult = memcpy_s(msg->value, dataLen, data, dataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, dataLen, dataLen);
    }

    /* 填写消息内容 */
    AT_MntnTraceEvent((struct MsgCB *)msg);

    /* 释放消息内存 */
    /*lint -save -e830 */
    PS_MEM_FREE(WUEPS_PID_AT, msg);
    /*lint -restore */
}

VOS_VOID AT_MntnTraceRPTPORT(VOS_VOID)
{
    AT_MntnRptport *sndMsgCB = VOS_NULL_PTR;
    VOS_UINT8       loop;

    /*lint -save -e516 */
    sndMsgCB = (AT_MntnRptport *)PS_ALLOC_MSG(WUEPS_PID_AT, sizeof(AT_MntnRptport) - VOS_MSG_HEAD_LENGTH);
    /*lint -restore */
    if (sndMsgCB == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_MNTN_TraceRPTPORT:ERROR: Alloc Memory Fail.");
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)sndMsgCB, WUEPS_PID_AT, WUEPS_PID_AT, sizeof(AT_MntnRptport) - VOS_MSG_HEAD_LENGTH);

    sndMsgCB->msgId = ID_AT_MNTN_RPT_PORT;

    for (loop = 0; loop < AT_MAX_CLIENT_NUM; loop++) {
        sndMsgCB->atRptPort[loop].atClientTabIndex = loop;
        sndMsgCB->atRptPort[loop].modemId          = g_atClientCtx[loop].clientConfiguration.modemId;
        sndMsgCB->atRptPort[loop].reportFlg        = g_atClientCtx[loop].clientConfiguration.reportFlg;
    }

    AT_MntnTraceEvent((struct MsgCB *)sndMsgCB);
    /*lint -save -e516 */
    PS_FREE_MSG(WUEPS_PID_AT, sndMsgCB);
    /*lint -restore */
}

VOS_VOID AT_MntnTraceAtAtpCmdReq(VOS_UINT8 *data, VOS_UINT32 len)
{
    AT_Msg      *msg = VOS_NULL_PTR;
    VOS_UINT32   length;
    errno_t      memResult;

    length = AT_GetAtMsgStruMsgLength(len);
    msg = (AT_Msg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, length);
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_MntnTraceAtAtpCmdReq:ERROR:Alloc Msg");
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)msg, WUEPS_PID_AT, WUEPS_PID_AT, length);
    msg->msgId = ID_AT_MNTN_AT_ATP_CMD_REQ;
    msg->len = (VOS_UINT16)len;

    memResult = memcpy_s(msg->value, len, data, len);
    TAF_MEM_CHK_RTN_VAL(memResult, len, len);

    AT_MntnTraceEvent((struct MsgCB *)msg);
    /*lint -save -e516 */
    PS_FREE_MSG(WUEPS_PID_AT, msg);
    /*lint -restore */
}

VOS_VOID AT_RecordAtMsgInfo(VOS_UINT32 sendPid, VOS_UINT32 msgName, VOS_UINT32 sliceStart, VOS_UINT32 sliceEnd)
{
    VOS_UINT32 indexNum;

    if (g_atMsgRecordInfo.currentIndex >= AT_MNTN_MSG_RECORD_MAX_NUM) {
        g_atMsgRecordInfo.currentIndex = 0;
    }

    indexNum = g_atMsgRecordInfo.currentIndex;

    g_atMsgRecordInfo.atMntnMsgRecord[indexNum].sendPid    = sendPid;
    g_atMsgRecordInfo.atMntnMsgRecord[indexNum].msgName    = msgName;
    g_atMsgRecordInfo.atMntnMsgRecord[indexNum].sliceStart = sliceStart;
    g_atMsgRecordInfo.atMntnMsgRecord[indexNum].sliceEnd   = sliceEnd;

    g_atMsgRecordInfo.currentIndex++;
}

VOS_UINT16 AT_MntnTransferClientIdToMntnIndex(VOS_UINT16 clientId)
{
    VOS_UINT16 mntnIndex = AT_CLIENT_ID_BUTT;

    /* 当前仅对pcui ctrl口以及appvcom-appvcom34共37个端口抓取维测 */
    if (clientId <= AT_CLIENT_ID_CTRL) {
        mntnIndex = clientId;
    }

    if ((clientId >= AT_CLIENT_ID_APP) && (clientId < AT_MNTN_MAX_PORT_CLIENT_ID)) {
        /* mntn的端口是从pcui、ctrl、appvcom0、appvcom1...开始的,+2是为了加上pcui、ctrl的索引 */
        mntnIndex = clientId - AT_CLIENT_ID_APP + 2;
    }

    return mntnIndex;
}

VOS_VOID AT_MntnSaveCurCmdName(VOS_UINT16 indexNum)
{
    errno_t    memResult;
    VOS_UINT16 mntnIndex;

    mntnIndex = AT_MntnTransferClientIdToMntnIndex(indexNum);

    if (mntnIndex == AT_CLIENT_ID_BUTT) {
        return;
    }

    if ((g_atParseCmd.cmdName.cmdNameLen > 0) && (g_atParseCmd.cmdName.cmdNameLen <= AT_CMD_NAME_LEN)) {
        /* 最后保留一个字符串结束符 0 */
        memResult = memcpy_s(g_atMntnPortInfo[mntnIndex].cmdName, sizeof(g_atMntnPortInfo[mntnIndex].cmdName) - 1,
                             g_atParseCmd.cmdName.cmdName, sizeof(g_atMntnPortInfo[mntnIndex].cmdName) - 1);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(g_atMntnPortInfo[mntnIndex].cmdName) - 1,
                            sizeof(g_atMntnPortInfo[mntnIndex].cmdName) - 1);
    }
}

VOS_VOID AT_MntnSaveExcLog(VOS_VOID)
{
    AT_MntnSaveExcLogInfo *excLogBuffer = VOS_NULL_PTR;
    VOS_UINT32             atClientId;
    VOS_UINT32             i;
    errno_t                memResult;

    /* 申请失败，直接退出 */
    if (g_atExcAddr == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_MNTN_SaveExcLog:No memory allocated");
        return;
    }

    (VOS_VOID)memset_s(g_atExcAddr, AT_MNTN_SAVE_EXC_LOG_LENGTH, 0, AT_MNTN_SAVE_EXC_LOG_LENGTH);

    excLogBuffer           = (AT_MntnSaveExcLogInfo *)g_atExcAddr;
    excLogBuffer->beginTag = AT_MNTN_DUMP_BEGIN_TAG;
    excLogBuffer->endTag   = AT_MNTN_DUMP_END_TAG;

    memResult = memcpy_s(&(excLogBuffer->msgInfo), sizeof(excLogBuffer->msgInfo), &g_atMsgRecordInfo,
                         sizeof(g_atMsgRecordInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(excLogBuffer->msgInfo), sizeof(g_atMsgRecordInfo));

    memResult = memcpy_s(&excLogBuffer->portInfo[0], sizeof(excLogBuffer->portInfo), g_atMntnPortInfo,
                         sizeof(g_atMntnPortInfo));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(excLogBuffer->portInfo), sizeof(g_atMntnPortInfo));

    atClientId = AT_CLIENT_ID_PCUI;

    for (i = 0; i < AT_MNTN_MAX_PORT_NUM; i++) {
        if (i == 0) {
            atClientId = AT_CLIENT_ID_PCUI;
        }

        /* 仅统计pcui、ctrl以及appvcom0到appvcom34端口，所以当index为2时，需要把clientid置位app */
        if (i == 2) {
            atClientId = AT_CLIENT_ID_APP;
        }

        excLogBuffer->portInfo[i].clientStatus = g_parseContext[atClientId].clientStatus;
        atClientId++;
    }
}

VOS_VOID AT_RegisterDumpCallBack(VOS_VOID)
{
    /* 调用底软接口进行申请内存 */
    /* 分配内存 */
    g_atExcAddr = (VOS_UINT8 *)mdrv_om_register_field(AT_DUMP_FIELD_ID, "AT dump",
        AT_MNTN_SAVE_EXC_LOG_LENGTH, 0);

    /* 申请失败打印异常信息，内存申请异常时仍然注册，回调函数中有地址申请失败保护 */
    if (g_atExcAddr == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_RegisterDumpCallBack:No memory allocated");
    }

    (VOS_VOID)mdrv_om_register_callback("AT_EXCLOG", (dump_hook)AT_MntnSaveExcLog);
}

#if (FEATURE_AT_HSUART == FEATURE_ON)

VOS_VOID AT_ShowHsUartConfigInfo(VOS_VOID)
{
    AT_UartFlowCtrl    *flowCtrl                                      = VOS_NULL_PTR;
    AT_UartRiCfg       *uartRiCfgInfo                                 = VOS_NULL_PTR;
    AT_UartPhyCfg      *uartPhyCfgInfo                                = VOS_NULL_PTR;
    const AT_UartFormatParam *formatParam                                   = VOS_NULL_PTR;
    const VOS_CHAR      acParityStr[][AT_HSUART_PARITY_MAX_LEN]       = { "ODD", "EVEN", "MARK", "SPACE", "NONE" };
    const VOS_CHAR      acDceByDteStr[][AT_HSUART_DCE_BY_DTE_MAX_LEN] =
                            { "NONE", "XON OR XOFF REMOVE", "RTS", "XON_OR_XOFF_PASS" };
    const VOS_CHAR       acDteByDceStr[][AT_HSUART_DTE_BY_DCE_MAX_LEN] = { "NONE", "XON OR XOFF REMOVE", "CTS" };

    flowCtrl       = AT_GetUartFlowCtrlInfo();
    uartRiCfgInfo  = AT_GetUartRiCfgInfo();
    uartPhyCfgInfo = AT_GetUartPhyCfgInfo();
    formatParam    = AT_HSUART_GetFormatParam(uartPhyCfgInfo->frame.format);

    PS_PRINTF_INFO("[1] HSUART PHY                      \n");
    PS_PRINTF_INFO("BaudRate:                   %d\n", uartPhyCfgInfo->baudRate);
    PS_PRINTF_INFO("Format:                     %d\n", uartPhyCfgInfo->frame.format);
    PS_PRINTF_INFO("->Data Bit Length:          %d\n", formatParam->dataBitLength);
    PS_PRINTF_INFO("->Stop Bit Length:          %d\n", formatParam->stopBitLength);
    PS_PRINTF_INFO("->Parity Bit Length:        %d\n", formatParam->parityBitLength);
    PS_PRINTF_INFO("Parity Type:                %s\n", acParityStr[uartPhyCfgInfo->frame.parity]);

    PS_PRINTF_INFO("[2] HSUART LINE                     \n");

    PS_PRINTF_INFO("[3] HSUART FLOW CTRL                \n");
    PS_PRINTF_INFO("DCE BY DTE[%d]:             %s\n", flowCtrl->dceByDte, acDceByDteStr[flowCtrl->dceByDte]);
    PS_PRINTF_INFO("DTE BY DCE[%d]:             %s\n", flowCtrl->dteByDce, acDteByDceStr[flowCtrl->dteByDce]);

    PS_PRINTF_INFO("[4] HSUART RI                       \n");
    PS_PRINTF_INFO("SMS RI ON (ms):             %u\n", uartRiCfgInfo->smsRiOnInterval);
    PS_PRINTF_INFO("SMS RI OFF (ms):            %u\n", uartRiCfgInfo->smsRiOffInterval);
    PS_PRINTF_INFO("VOICE RI ON (ms):           %u\n", uartRiCfgInfo->voiceRiOnInterval);
    PS_PRINTF_INFO("VOICE RI OFF (ms):          %u\n", uartRiCfgInfo->voiceRiOffInterval);
    PS_PRINTF_INFO("VOICE RI Cycle Times:       %d\n", uartRiCfgInfo->voiceRiCycleTimes);
}
#endif

VOS_VOID AT_ShowAllClientState(VOS_VOID)
{
    VOS_UINT8           i;
    const VOS_CHAR      acStateStr[][AT_CLIENT_STATE_MAX_LEN] = { "READY", "PEND" };
    const VOS_CHAR      acModeStr[][AT_CLIENT_MODE_MAX_LEN]  = { "CMD", "DATA", "ONLINE_CMD" };
    DMS_PortModeUint8   mode;

    PS_PRINTF_INFO("The All Client State: \n");

    for (i = 0; i < AT_MAX_CLIENT_NUM; i++) {
        mode = DMS_PORT_GetMode(g_atClientTab[i].portNo);
        PS_PRINTF_INFO("Client[%d] State: %s    Mode: %s\n", i, acStateStr[g_parseContext[i].clientStatus],
                       acModeStr[mode]);
    }
}

VOS_VOID AT_ShowClientCtxInfo(VOS_VOID)
{
    VOS_UINT8                     i;
    const AT_ClientCfgMapTab     *cfgMapTbl = VOS_NULL_PTR;
    AT_ClientConfiguration       *clientCfg = VOS_NULL_PTR;

    PS_PRINTF_INFO("The All Used Client Config: \n");
    for (i = 0; i < AT_GET_CLIENT_CFG_TAB_LEN(); i++) {
        cfgMapTbl = AT_GetClientCfgMapTbl(i);
        clientCfg = AT_GetClientConfig(cfgMapTbl->clientId);
        PS_PRINTF_INFO("Client[%s] modem:%d, reportFlag:%d\n", cfgMapTbl->portName, clientCfg->modemId,
                       clientCfg->reportFlg);
    }
}

