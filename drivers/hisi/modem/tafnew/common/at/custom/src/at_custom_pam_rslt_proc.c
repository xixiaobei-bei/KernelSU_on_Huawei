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
#include "at_custom_pam_rslt_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PAM_RSLT_PROC_C

VOS_UINT32 At_ProcPihCardAtrQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_CARD_ATR_READ */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CARD_ATR_READ) {
        AT_WARN_LOG("At_ProcPihCardAtrQryCnf : CmdCurrentOpt is not AT_CMD_CARD_ATR_READ!");
        return AT_ERROR;
    }

    event->pihEvent.atrQryCnf.len = TAF_MIN(event->pihEvent.atrQryCnf.len, SI_ATR_MAX_LEN);

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s:\"", g_parseContext[indexNum].cmdElement->cmdName);

    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     event->pihEvent.atrQryCnf.command,
                                                     (VOS_UINT16)event->pihEvent.atrQryCnf.len);

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                                                   (TAF_CHAR *)g_atSndCodeAddress + (*length), "\"");

    return AT_OK;
}

#if (FEATURE_SECURITY_SHELL == FEATURE_ON)
VOS_UINT32 AT_RcvDrvAgentSetSpwordRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg          *rcvMsg       = VOS_NULL_PTR;
    DRV_AGENT_SpwordSetCnf *spwordSetCnf = VOS_NULL_PTR;
    VOS_UINT8               indexNum     = 0;

    /* 初始化消息 */
    rcvMsg       = (DRV_AGENT_Msg *)msg;
    spwordSetCnf = (DRV_AGENT_SpwordSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(spwordSetCnf->atAppCtrl.clientId, AT_CMD_SPWORD_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 密码校验通过，错误次数需要清零 */
    if (spwordSetCnf->result == VOS_OK) {
        g_spWordCtx.shellPwdCheckFlag = VOS_TRUE;
        g_spWordCtx.errTimes          = 0;

        /* 调用At_FormatResultData发送命令结果 */
        At_FormatResultData(indexNum, AT_OK);
    }
    /* 密码校验失败，错误次数加1 */
    else {
        g_spWordCtx.errTimes++;

        /* 调用At_FormatResultData发送命令结果 */
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}
#endif

VOS_UINT32 At_SetPassThroughCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PASSTHROUGH_SET) {
        AT_WARN_LOG("At_SetPassThroughCnf: CmdCurrentOpt is not AT_CMD_PASSTHROUGH_SET");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_QryPassThroughCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PASSTHROUGH_QRY) {
        AT_WARN_LOG("At_QryPassThroughCnf: CmdCurrentOpt is not AT_CMD_PASSTHROUGH_QRY");
        return AT_ERROR;
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        event->pihEvent.passThroughState);

    return AT_OK;
}

#if ((FEATURE_SINGLEMODEM_DUALSLOT == FEATURE_ON) && (MULTI_MODEM_NUMBER == 1))
/*
 * 功能描述: SI_PIH_EVENT_SINGLEMODEMDUALSLOT_SET_CNF的处理
 */
VOS_UINT32 At_SetSingleModemDualSlotCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SINGLEMODEMDUALSLOT_SET) {
        AT_WARN_LOG("At_SetSingleModemDualSlotCnf: CmdCurrentOpt is not AT_CMD_SINGLEMODEMDUALSLOT_SET");
        return AT_ERROR;
    }

    return AT_OK;
}

/*
 * 功能描述: SI_PIH_EVENT_SINGLEMODEMDUALSLOT_QUERY_CNF的处理
 */
VOS_UINT32 At_QrySingleModemDualSlotCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SINGLEMODEMDUALSLOT_QRY) {
        AT_WARN_LOG("At_QrySingleModemDualSlotCnf: CmdCurrentOpt is not AT_CMD_SINGLEMODEMDUALSLOT_QRY");
        return AT_ERROR;
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        event->pihEvent.singleModemSlotCfg);

    return AT_OK;
}
#endif

VOS_UINT32 At_ProcPihSciCfgSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_SCICHG_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SCICHG_SET) {
        AT_WARN_LOG("At_ProcPihSciCfgSetCnf : CmdCurrentOpt is not AT_CMD_SCICHG_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 At_SciCfgQueryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_SCICHG_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SCICHG_QRY) {
        AT_WARN_LOG("At_SciCfgQueryCnf : CmdCurrentOpt is not AT_CMD_SCICHG_QRY!");
        return AT_ERROR;
    }

    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        event->pihEvent.sciCfgCnf.card0Slot, event->pihEvent.sciCfgCnf.card1Slot);

#if (MULTI_MODEM_NUMBER == 3)
    (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + (*length), ",%d", event->pihEvent.sciCfgCnf.card2Slot);
#endif

    return AT_OK;
}

#if ((FEATURE_PHONE_SC == FEATURE_ON) && (FEATURE_PHONE_USIM == FEATURE_ON))

VOS_UINT32 At_PrintSilentPinInfo(TAF_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    VOS_UINT8 *pinIv = VOS_NULL_PTR;
    VOS_UINT32 i;

    /* 判断当前操作类型是否为AT_CMD_SILENTPIN_SET/AT_CMD_SILENTPININFO_SET */
    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SILENTPIN_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SILENTPININFO_SET)) {
        AT_WARN_LOG("At_PrintSilentPinInfo : CmdCurrentOpt is not AT_CMD_SILENTPIN_SET/AT_CMD_SILENTPININFO_SET!");
        return AT_ERROR;
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

    for (i = 0; i < DRV_AGENT_PIN_CRYPTO_DATA_LEN; i++) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "%02X", event->pihEvent.cryptoPin.cryptoPin[i]);
    }

    pinIv = (VOS_UINT8 *)event->pihEvent.cryptoPin.pinIv;

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), ",");

    for (i = 0; i < SI_CRYPTO_CBC_PIN_LEN; i++) {
        /* IV长度为16字节 */
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "%02X", (*(pinIv + i)));
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), ",");

    for (i = 0; i < DRV_AGENT_HMAC_DATA_LEN; i++) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), "%02X", event->pihEvent.cryptoPin.hmacValue[i]);
    }

    return AT_OK;
}
#endif

VOS_UINT32 At_ProcPihPrivateCglaSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_PRIVATECGLA_REQ */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_PRIVATECGLA_REQ) {
        AT_WARN_LOG("At_ProcPihPrivateCglaSetCnf : CmdCurrentOpt is not AT_CMD_PRIVATECGLA_REQ!");
        return AT_ERROR;
    }

    event->pihEvent.cglaHandleCnf.len = TAF_MIN(event->pihEvent.cglaHandleCnf.len, SI_PRIVATECGLA_APDU_MAX_LEN);

    /* 非最后一条打印上报IND，需要在行首添加回车换行 */
    if (event->pihEvent.cglaHandleCnf.lastDataFlag != VOS_TRUE) {
        (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "\r\n");
    }

    /* ^CGLA: <flag>,<length>,"[<command>]<SW1><SW2>" */
    (*length) +=
        (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s: %d,%d,\"", g_parseContext[indexNum].cmdElement->cmdName,
            event->pihEvent.cglaHandleCnf.lastDataFlag,
            (event->pihEvent.cglaHandleCnf.len + AT_DOUBLE_QUO_MARKS_LENGTH) * AT_DOUBLE_LENGTH);

    if (event->pihEvent.cglaHandleCnf.len != 0) {
        /* <command>, */
        (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                         (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                         event->pihEvent.cglaHandleCnf.command,
                                                         event->pihEvent.cglaHandleCnf.len);
    }

    /* <SW1> */
    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     &event->pihEvent.cglaHandleCnf.sw1, (VOS_UINT16)sizeof(TAF_UINT8));

    /* <SW2> */
    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     &event->pihEvent.cglaHandleCnf.sw2, (VOS_UINT16)sizeof(TAF_UINT8));
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "\"");

    return AT_OK;
}

VOS_UINT32 At_ProcPihCardVoltageQryCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_CARDVOLTAGE_QUERY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CARDVOLTAGE_QUERY) {
        AT_WARN_LOG("At_ProcPihCardVoltageQryCnf : CmdCurrentOpt is not AT_CMD_CARDVOLTAGE_QUERY!");
        return AT_ERROR;
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d, %x", g_parseContext[indexNum].cmdElement->cmdName,
        event->pihEvent.cardVoltageCnf.voltage, event->pihEvent.cardVoltageCnf.charaByte);

    return AT_OK;
}

VOS_UINT32 AT_RcvDrvAgentImsiChgQryRsp(struct MsgCB *msg)
{
    VOS_UINT8                indexNum = 0;
    DRV_AGENT_ImsichgQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_ImsichgQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentImsiChgQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentImsiChgQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待IMSICHG查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_IMSICHG_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置操作结果 :  */
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "^IMSICHG: %d,%d", event->dualIMSIEnable, event->curImsiSign);

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
#if (FEATURE_PHONE_USIM == FEATURE_OFF)
/*
 * 功能描述: ^NOCARD
 * 修改历史:
 *  1.日    期: 2020年5月21日
 *    修改内容: 新生成函数
 */
VOS_UINT32 AT_ProcNoCardSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    VOS_UINT32 result;
    AT_USIM_InfoCtx *usimInfoCtx = VOS_NULL_PTR;
    ModemIdUint16 modemId;

    if ((event == VOS_NULL_PTR) || (length == VOS_NULL_PTR) ||
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_NOCARD_SET)) {
        return AT_ERROR;
    }

    result = AT_GetModemIdFromClient(indexNum, &modemId);
    if ((result != VOS_OK) || (event->pihError != VOS_OK)) {
        return AT_ERROR;
    }

    /* 更新当前NOCARD的状态 */
    usimInfoCtx = AT_GetUsimInfoCtxFromModemId(modemId);
    usimInfoCtx->noCardMode = event->pihEvent.noCardMode;
    return AT_OK;
}
#endif
#endif

VOS_UINT32 At_ProcPihGenericAccessCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CSIM_SET) {
        AT_WARN_LOG("At_ProcPihGenericAccessCnf : CmdCurrentOpt is not AT_CMD_CSIM_SET!");
        return AT_ERROR;
    }

    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%s: ", g_parseContext[indexNum].cmdElement->cmdName);
    /* <length>, */
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + (*length), "%d,\"",
        (event->pihEvent.gAccessCnf.len + AT_DOUBLE_QUO_MARKS_LENGTH) * AT_DOUBLE_LENGTH);
    if (event->pihEvent.gAccessCnf.len != 0) {
        /* <command>, */
        (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                         (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                         event->pihEvent.gAccessCnf.command,
                                                         event->pihEvent.gAccessCnf.len);
    }
    /* SW1 */
    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     &event->pihEvent.gAccessCnf.sw1, sizeof(TAF_UINT8));
    /* SW1 */
    (*length) += (TAF_UINT16)At_HexAlpha2AsciiString(AT_CMD_MAX_LEN, (TAF_INT8 *)g_atSndCodeAddress,
                                                     (TAF_UINT8 *)g_atSndCodeAddress + (*length),
                                                     &event->pihEvent.gAccessCnf.sw2, sizeof(TAF_UINT8));
    (*length) += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                                                   (TAF_CHAR *)g_atSndCodeAddress + (*length), "\"");

    return AT_OK;
}

VOS_UINT32 AT_RcvDrvAgentMemInfoQryRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg           *rcvMsg            = VOS_NULL_PTR;
    VOS_UINT8                indexNum          = 0;
    DRV_AGENT_MeminfoQryRsp *cCpuMemInfoCnfMsg = VOS_NULL_PTR;
    AT_PID_MemInfoPara      *pidMemInfo        = VOS_NULL_PTR;
    VOS_UINT32 aCpuMemBufSize;
    VOS_UINT32 aCpuPidTotal;
    VOS_UINT16 atLength;
    VOS_UINT32 i;

    /* 初始化 */
    rcvMsg            = (DRV_AGENT_Msg *)msg;
    cCpuMemInfoCnfMsg = (DRV_AGENT_MeminfoQryRsp *)rcvMsg->content;

    /* 指向CCPU的每个PID的内存信息 */
    pidMemInfo = (AT_PID_MemInfoPara *)cCpuMemInfoCnfMsg->data;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(cCpuMemInfoCnfMsg->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_ERR_LOG("AT_RcvDrvAgentMemInfoQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* Client Id 为广播也返回ERROR */
    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_ERR_LOG("AT_RcvDrvAgentMemInfoQryRsp: AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* AT模块在等待MEMINFO查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MEMINFO_READ) {
        AT_ERR_LOG("AT_RcvDrvAgentMemInfoQryRsp: CmdCurrentOpt Error!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 对返回结果中的参数进行检查 */
    if ((cCpuMemInfoCnfMsg->result != VOS_OK) ||
        ((cCpuMemInfoCnfMsg->memQryType != AT_MEMQUERY_TTF) && (cCpuMemInfoCnfMsg->memQryType != AT_MEMQUERY_VOS))) {
        /* 调用At_FormatResultData返回ERROR字符串 */
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    /* 先打印命令名称 */
    atLength = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s:%s", g_parseContext[indexNum].cmdElement->cmdName, g_atCrLf);

    atLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + atLength, "C CPU Pid:%d%s", cCpuMemInfoCnfMsg->pidNum, g_atCrLf);

    /* 依次打印C CPU每个PID的内存信息 */
    for (i = 0; i < cCpuMemInfoCnfMsg->pidNum; i++) {
        atLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + atLength, "%d,%d,%d%s", pidMemInfo[i].pid, pidMemInfo[i].msgPeakSize,
            pidMemInfo[i].memPeakSize, g_atCrLf);
    }

    /* fusion版本VOS接口无法查询内存 */
    /*
     * 如果是查询VOS内存，则获取并打印A CPU的VOS内存使用情况.
     * 由于暂无A CPU的TTF内存查询接口，TTF类型查询只打印C CPU的TTF内存使用情况
     */
    if (cCpuMemInfoCnfMsg->memQryType == AT_MEMQUERY_VOS) {
        aCpuMemBufSize = AT_PID_MEM_INFO_LEN * sizeof(AT_PID_MemInfoPara);

        /* 分配内存以查询A CPU的VOS内存使用信息 */
        /*lint -save -e516 */
        pidMemInfo = (AT_PID_MemInfoPara *)PS_MEM_ALLOC(WUEPS_PID_AT, aCpuMemBufSize);
        /*lint -restore */
        if (pidMemInfo != VOS_NULL_PTR) {
            (VOS_VOID)memset_s(pidMemInfo, aCpuMemBufSize, 0x00, aCpuMemBufSize);

            aCpuPidTotal = 0;

            if (VOS_AnalyzePidMemory(pidMemInfo, aCpuMemBufSize, &aCpuPidTotal) != VOS_ERR) {
                /* 依次打印 A CPU的每个PID的内存使用情况 */
                atLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + atLength, "A CPU Pid:%d%s", aCpuPidTotal, g_atCrLf);

                for (i = 0; i < aCpuPidTotal; i++) {
                    atLength += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + atLength, "%d,%d,%d%s", pidMemInfo[i].pid,
                        pidMemInfo[i].msgPeakSize, pidMemInfo[i].memPeakSize, g_atCrLf);
                }
            }

            /* 释放内存 */
            /*lint -save -e516 */
            PS_MEM_FREE(WUEPS_PID_AT, pidMemInfo);
            pidMemInfo = VOS_NULL_PTR;
            /*lint -restore */
        }
    }
    g_atSendDataBuff.bufLen = atLength;

    /* 调用At_FormatResultData返回OK字符串 */
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSetApSimstRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg           *rcvMsg        = VOS_NULL_PTR;
    DRV_AGENT_ApSimstSetCnf *apSimStSetCnf = VOS_NULL_PTR;
    VOS_UINT8                indexNum      = 0;

    /* 初始化消息 */
    rcvMsg        = (DRV_AGENT_Msg *)msg;
    apSimStSetCnf = (DRV_AGENT_ApSimstSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(apSimStSetCnf->atAppCtrl.clientId, AT_CMD_APSIMST_SET, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^APSIMST设置命令返回 */
    if (apSimStSetCnf->result != VOS_OK) {
        /* 调用At_FormatResultData发送命令结果 */
        At_FormatResultData(indexNum, AT_ERROR);
    } else {
        /* 调用At_FormatResultData发送命令结果 */
        At_FormatResultData(indexNum, AT_OK);
    }

    return VOS_OK;
}

#if ((FEATURE_PHONE_USIM == FEATURE_OFF) && (MULTI_MODEM_NUMBER == 1))

VOS_UINT32 At_ProcPihCardAppAidSetCnf(VOS_UINT8 indexNum, SI_PIH_EventInfo *event, VOS_UINT16 *length)
{
    /* 判断当前操作类型是否为AT_CMD_CARDAPPAID_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CARDAPPAID_SET) {
        AT_WARN_LOG("At_ProcPihCardAppAidSetCnf : CmdCurrentOpt is not AT_CMD_CARDAPPAID_SET!");
        return AT_ERROR;
    }

    return AT_OK;
}
#endif

