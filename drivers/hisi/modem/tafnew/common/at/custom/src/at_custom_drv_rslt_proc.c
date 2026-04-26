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
#include "at_custom_drv_rslt_proc.h"
#include "securec.h"

#include "at_ctx.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_mta_interface.h"
#include "at_mdrv_interface.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_DRV_RSLT_PROC_C

VOS_UINT32 AT_RcvMtaNvLoadSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *rcvMsg = VOS_NULL_PTR;
    MTA_AT_SetNvloadCnf *setCnf = VOS_NULL_PTR;
    VOS_UINT32           result;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    setCnf   = (MTA_AT_SetNvloadCnf *)rcvMsg->content;
    result   = AT_OK;
    g_atSendDataBuff.bufLen = 0;

    if (setCnf->result == TAF_ERR_NO_ERROR) {
        result = AT_OK;
    } else {
        result = At_ChgTafErrorCode(indexNum, setCnf->result);
    }

    return result;
}

VOS_UINT32 AT_RcvDrvAgentFlashInfoQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                 ret;
    VOS_UINT8                  indexNum = 0;
    VOS_UINT16                 length;
    DRV_AGENT_FlashinfoQryCnf *event  = VOS_NULL_PTR;
    DRV_AGENT_Msg             *rcvMsg = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_FlashinfoQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentFlashInfoQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentFlashInfoQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待^FLASHINFO查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FLASHINFO_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_FLASHINFO_QRY_NO_ERROR) {
        /*
         * 设置错误码为AT_OK
         * 构造结构为<CR><LF>~~~~~~FLASH INFO~~~~~~:<CR><LF>
         *  <CR><LF>MMC BLOCK COUNT:<blockcount>,
         *       PAGE SIZE:<pagesize>,
         *       PAGE COUNT PER BLOCK:<blocksize><CR><LF>
         *  <CR><LF>OK<CR><LF>格式
         */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s%s", "~~~~~~FLASH INFO~~~~~~:", g_atCrLf);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "MMC BLOCK COUNT:%d, PAGE SIZE:%d, PAGE COUNT PER BLOCK:%d",
            event->flashInfo.blockCount, event->flashInfo.pageSize, event->flashInfo.pgCntPerBlk);

        ret = AT_OK;

    } else {
        /* 查询失败返回ERROR字符串 */
        length = 0;
        ret    = AT_ERROR;
    }

    /* 调用At_FormatResultData输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentAppdmverQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                ret;
    VOS_UINT8                 indexNum = 0;
    DRV_AGENT_AppdmverQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg            *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_AppdmverQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentAppdmverQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentAppdmverQryRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待APPDMVER查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_APPDMVER_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_APPDMVER_QRY_NO_ERROR) {
        /* 设置错误码为AT_OK           构造结构为^APPDMVER:<pdmver>格式 */
        ret = AT_OK;

        /* buf从外部接口返回的，增加结束符，防止缓冲区溢出 */
        event->pdmver[AT_MAX_PDM_VER_LEN] = '\0';
        g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:%s", g_parseContext[indexNum].cmdElement->cmdName, event->pdmver);

    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentMfreelocksizeQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                     ret;
    VOS_UINT8                      indexNum = 0;
    DRV_AGENT_MfreelocksizeQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg                 *rcvMsg   = VOS_NULL_PTR;
    VOS_UINT32                     aCoreMemfreeSize;

    /* 初始化 */
    rcvMsg           = (DRV_AGENT_Msg *)msg;
    event            = (DRV_AGENT_MfreelocksizeQryCnf *)rcvMsg->content;
    aCoreMemfreeSize = 0;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentMfreelocksizeQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentMfreelocksizeQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待MFREELOCKSIZE查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MFREELOCKSIZE_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置操作结果 :  */
    g_atSendDataBuff.bufLen = 0;
    if (event->result == DRV_AGENT_MFREELOCKSIZE_QRY_NO_ERROR) {
        /* 获取A核的剩余系统内存 */
        aCoreMemfreeSize = FREE_MEM_SIZE_GET();

        /* 由于底软返回的是KB，转为字节 */
        aCoreMemfreeSize *= AT_KB_TO_BYTES_NUM;

        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            event->maxFreeLockSize, aCoreMemfreeSize);

        /* 设置错误码为AT_OK */
        ret = AT_OK;
    } else {
        /* 设置失败返回ERROR字符串 */
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_ERROR;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentYjcxSetCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg        *rcvMsg     = VOS_NULL_PTR;
    DRV_AGENT_YjcxSetCnf *yjcxSetCnf = VOS_NULL_PTR;
    VOS_UINT32            result;
    VOS_UINT8             indexNum = 0;
    VOS_UINT16            dataLen;

    /* 初始化 */
    rcvMsg     = (DRV_AGENT_Msg *)msg;
    yjcxSetCnf = (DRV_AGENT_YjcxSetCnf *)(rcvMsg->content);
    result     = AT_OK;
    dataLen    = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(yjcxSetCnf->atAppCtrl.clientId, AT_CMD_YJCX_SET, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^YJCX设置命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (yjcxSetCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        dataLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        yjcxSetCnf->flashInfo[TAF_MAX_FLAFH_INFO_LEN] = '\0';
        dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s", yjcxSetCnf->flashInfo);
        g_atSendDataBuff.bufLen = dataLen;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentYjcxQryCnf(struct MsgCB *msg)
{
    DRV_AGENT_Msg        *rcvMsg     = VOS_NULL_PTR;
    DRV_AGENT_YjcxQryCnf *yjcxQryCnf = VOS_NULL_PTR;
    VOS_UINT32            result;
    VOS_UINT8             indexNum = 0;
    VOS_UINT16            dataLen;

    /* 初始化 */
    rcvMsg     = (DRV_AGENT_Msg *)msg;
    yjcxQryCnf = (DRV_AGENT_YjcxQryCnf *)(rcvMsg->content);
    result     = AT_OK;
    dataLen    = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(yjcxQryCnf->atAppCtrl.clientId, AT_CMD_YJCX_QUERY, &indexNum)
        != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 格式化AT^YJCX查询命令返回 */
    g_atSendDataBuff.bufLen = 0;
    if (yjcxQryCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        dataLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        yjcxQryCnf->gpioInfo[TAF_MAX_GPIO_INFO_LEN] = '\0';
        dataLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + dataLen, "%s", yjcxQryCnf->gpioInfo);

        g_atSendDataBuff.bufLen = dataLen;
    }

    /* 输出结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

