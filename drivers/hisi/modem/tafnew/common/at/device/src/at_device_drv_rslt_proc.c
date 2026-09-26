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
#include "at_device_drv_rslt_proc.h"
#include "securec.h"

#include "at_cmd_proc.h"
#include "at_cmd_msg_proc.h"

#include "at_mta_interface.h"
#include "at_device_comm.h"
#include "osm.h"
#include "at_lte_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_DRV_RSLT_PROC_C

VOS_UINT32 AT_RcvDrvAgentQryTbatvoltRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg            *rcvMsg         = VOS_NULL_PTR;
    DRV_AGENT_TbatvoltQryCnf *tbatvoltQryCnf = VOS_NULL_PTR;
    VOS_UINT8                 indexNum       = 0;
    VOS_UINT32                result;

    /* 初始化 */
    rcvMsg         = (DRV_AGENT_Msg *)msg;
    tbatvoltQryCnf = (DRV_AGENT_TbatvoltQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(tbatvoltQryCnf->atAppCtrl.clientId, AT_CMD_TBATVOLT_QRY, &indexNum) !=
        VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 电池电压查询结果判断 */
    if (tbatvoltQryCnf->fail != VOS_TRUE) {
        g_atSendDataBuff.bufLen =
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName,
                tbatvoltQryCnf->batVol);
        result = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_ERROR;
    }

    /* 调用AT_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

VOS_VOID At_ZeroReplaceBlankInString(VOS_UINT8 *data, VOS_UINT32 len)
{
    TAF_UINT32 chkLen = 0;
    TAF_UINT8 *write  = data;
    TAF_UINT8 *read   = data;

    /* 输入参数检查 */
    while (chkLen < len) {
        /*
         * 时间格式 May  5 2011 17:08:00
         * 转换成   May 05 2011 17:08:00
         */
        if (*read++ == ' ') {
            if (*read == ' ') {
                write++;
                *write = '0';
                read++;
                chkLen++;
            }
        }
        write++;
        chkLen++;
    }
}

VOS_UINT32 AT_RcvDrvAgentQryVersionRspParamCheck(VOS_UINT16 clientId, VOS_UINT8 *indexNum, ModemIdUint16 *modemId)
{
    VOS_UINT32     ret;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(clientId, indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentQryVersionRsp:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(*indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentQryVersionRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_VERSION_QRY/AT_CMD_RSFR_VERSION_QRY, ^RSFR命令也借用此接口 */
    if ((g_atClientTab[*indexNum].cmdCurrentOpt != AT_CMD_VERSION_QRY) &&
        (g_atClientTab[*indexNum].cmdCurrentOpt != AT_CMD_RSFR_VERSION_QRY)) {
        return VOS_ERR;
    }

    ret = AT_GetModemIdFromClient(*indexNum, modemId);

    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_RcvDrvAgentQryVersionRsp: Get modem id fail.");
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_VOID AT_PrintfVersionInfoWithNullInnerVersion(DRV_AGENT_VersionQryCnf *versionQryCnfInfo, VOS_CHAR *acTmpTime,
    VOS_UINT32 len, TAF_NVIM_CsVer csver, VOS_UINT16 *length)
{
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:BDT:%s%s", "^VERSION", acTmpTime, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTS:%s%s", "^VERSION",
        versionQryCnfInfo->softVersion.revisionId, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTS:%s%s", "^VERSION",
        versionQryCnfInfo->softVersion.revisionId, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTD:%s%s", "^VERSION", versionQryCnfInfo->isoVer.isoInfo,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTD:%s%s", "^VERSION", versionQryCnfInfo->isoVer.isoInfo,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTH:%s%s", "^VERSION", versionQryCnfInfo->interHwVer.hwVer,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTH:%s%s", "^VERSION", versionQryCnfInfo->fullHwVer.hwVer,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTU:%s%s", "^VERSION", versionQryCnfInfo->modelId.modelId,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTU:%s%s", "^VERSION",
        versionQryCnfInfo->interModelId.modelId, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:CFG:%d%s", "^VERSION", csver.csver, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:PRL:", "^VERSION");

    g_atSendDataBuff.bufLen = *length;
}

VOS_VOID AT_PrintfVersionInfoWithValidInnerVersion(DRV_AGENT_VersionQryCnf *versionQryCnfInfo, VOS_CHAR *acTmpTime,
    VOS_UINT32 len, TAF_NVIM_CsVer csver, VOS_UINT16 *length)
{
    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:BDT:%s%s", "^VERSION", acTmpTime, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTS:%s%s", "^VERSION",
        versionQryCnfInfo->softVersion.revisionId, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTS:%s", "^VERSION", g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTD:%s%s", "^VERSION", versionQryCnfInfo->isoVer.isoInfo,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTD:%s", "^VERSION", g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTH:%s%s", "^VERSION", versionQryCnfInfo->interHwVer.hwVer,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTH:%s", "^VERSION", g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:EXTU:%s%s", "^VERSION", versionQryCnfInfo->modelId.modelId,
        g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:INTU:%s", "^VERSION", g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:CFG:%d%s", "^VERSION", csver.csver, g_atCrLf);

    *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)(g_atSndCodeAddress + *length), "%s:PRL:", "^VERSION");

    g_atSendDataBuff.bufLen = *length;
}

VOS_VOID AT_SetRsfrVersionCnf(VOS_UINT8 indexNum, VOS_UINT8 *data, VOS_UINT32 len)
{
    VOS_UINT16 length;
    VOS_CHAR  *pcRsfrItem = NULL;

    pcRsfrItem = (VOS_CHAR *)PS_MEM_ALLOC(WUEPS_PID_AT, AT_RSFR_RSFW_MAX_LEN);
    if (pcRsfrItem == VOS_NULL_PTR) {
        return;
    }

    (VOS_VOID)memset_s(pcRsfrItem, AT_RSFR_RSFW_MAX_LEN, 0x00, AT_RSFR_RSFW_MAX_LEN);

    /* 把从SIMLOCK命令中读出来的结果字符串作为编码源 */
    AtBase64Encode(data, len, (VOS_UINT8 *)pcRsfrItem);

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^RSFR:\"VERSION\",\"111\",125,1,1,%s", g_atCrLf);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", pcRsfrItem);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, AT_OK);
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, pcRsfrItem);
    /*lint -restore */
}

VOS_UINT32 At_DelCtlAndBlankCharWithEndPadding(VOS_UINT8 *data, VOS_UINT16 *cmdLen)
{
    VOS_UINT16 origLen;

    origLen = *cmdLen;

    /* 扫描控制符 */
    if (At_ScanCtlChar(data, cmdLen) == AT_FAILURE) {
        return AT_FAILURE;
    }

    /* 扫描引号以外的空格符 */
    if (At_ScanBlankChar(data, cmdLen) == AT_FAILURE) {
        return AT_FAILURE;
    }

    (VOS_VOID)memset_s(data + (*cmdLen), origLen - (*cmdLen), 0x00, origLen - (*cmdLen));

    return AT_SUCCESS;
}

VOS_UINT32 AT_RcvDrvAgentQryVersionRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg           *rcvMsg            = (DRV_AGENT_Msg *)msg;
    DRV_AGENT_VersionQryCnf *versionQryCnfInfo = VOS_NULL_PTR;
    VOS_UINT8                indexNum          = 0;
    VOS_UINT32               len = 0;
    VOS_CHAR                 acTmpTime[AT_AGENT_DRV_VERSION_TIME_LEN] = {0};
    VOS_UINT16               length;
    VOS_BOOL                 bPhyNumIsNull;

    VOS_UINT32     opt;
    VOS_UINT32     ret;
    errno_t        memResult;
    TAF_NVIM_CsVer csver = {0};
    ModemIdUint16  modemId = MODEM_ID_0;

    AT_PR_LOGI("Rcv Msg");

    csver.csver = 0;

    ret = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CSVER, &(csver.csver), sizeof(csver.csver));

    AT_PR_LOGI("Call interface success!");
    if (ret != NV_OK) {
        return AT_ERROR;
    }

    /* 初始化消息，获取ucContent */
    versionQryCnfInfo = (DRV_AGENT_VersionQryCnf *)rcvMsg->content;

    if (AT_RcvDrvAgentQryVersionRspParamCheck(versionQryCnfInfo->atAppCtrl.clientId, &indexNum, &modemId) != VOS_OK) {
        return VOS_ERR;
    }

    opt = g_atClientTab[indexNum].cmdCurrentOpt;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 查询出错 */
    if (versionQryCnfInfo->result != DRV_AGENT_VERSION_QRY_NO_ERROR) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    /* 版本编译时间格式转换，将时间戳中连续两个空格的后一个空格用0替换 */
    len       = VOS_StrLen(versionQryCnfInfo->verTime);
    memResult = memcpy_s(acTmpTime, sizeof(acTmpTime), versionQryCnfInfo->verTime, len + 1);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acTmpTime), len + 1);
    At_ZeroReplaceBlankInString((VOS_UINT8 *)acTmpTime, len);

    /* 字符串预解析 */
    length = TAF_CDROM_VERSION_LEN;
    At_DelCtlAndBlankCharWithEndPadding(versionQryCnfInfo->isoVer.isoInfo, &length);

    /* 获取物理号是否为空的标识 */
    if (AT_PhyNumIsNull(modemId, AT_PHYNUM_TYPE_IMEI, &bPhyNumIsNull) != AT_OK) {
        AT_WARN_LOG("AT_RcvDrvAgentQryVersionRsp(): AT_PhyNumIsNull Error!");
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    length = 0;

    /*
     * 内部版本号在单板未写入物理号前，查询需如实显示；写入物理号后，如查询版本号前未
     * 输入解锁指令，内部版本号显示为空，如果已经输入解锁指令，内部版本号如实显示
     */
    if (g_atDataLocked == VOS_FALSE || (bPhyNumIsNull == VOS_TRUE)) {
        AT_PrintfVersionInfoWithNullInnerVersion(versionQryCnfInfo, acTmpTime, VOS_StrLen(acTmpTime), csver, &length);
    } else {
        AT_PrintfVersionInfoWithValidInnerVersion(versionQryCnfInfo, acTmpTime, VOS_StrLen(acTmpTime), csver, &length);
    }

    /* 如果是^RSFR命令发起的查询VERSION信息的请求,则由^RSFR命令去处理 */
    if (opt == AT_CMD_RSFR_VERSION_QRY) {
        AT_SetRsfrVersionCnf(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

        /* AT返回的字符串在AT_SetRsfrVersionCnf中处理 */
        return VOS_OK;
    }

    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentSetGpioplRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg          *rcvMsg             = VOS_NULL_PTR;
    DRV_AGENT_GpioplSetCnf *drvAgentGpioSetCnf = VOS_NULL_PTR;
    VOS_UINT8               indexNum           = 0;
    VOS_UINT32              result;

    rcvMsg             = (DRV_AGENT_Msg *)msg;
    drvAgentGpioSetCnf = (DRV_AGENT_GpioplSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(drvAgentGpioSetCnf->atAppCtrl.clientId, AT_CMD_GPIOPL_SET, &indexNum) !=
        VOS_OK) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (drvAgentGpioSetCnf->fail == VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    /* 调用AT_FormATResultDATa发送命令结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentQryGpioplRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg          *rcvMsg     = VOS_NULL_PTR;
    DRV_AGENT_GpioplQryCnf *gpioQryCnf = VOS_NULL_PTR;
    VOS_UINT16              length;
    VOS_UINT32              i;
    VOS_UINT8               indexNum = 0;
    VOS_UINT32              result;

    /* 初始化消息，获取ucContent */
    rcvMsg     = (DRV_AGENT_Msg *)msg;
    gpioQryCnf = (DRV_AGENT_GpioplQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(gpioQryCnf->atAppCtrl.clientId, AT_CMD_GPIOPL_QRY, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (gpioQryCnf->fail != VOS_TRUE) {
        /* 打印结果 */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

        for (i = 0; i < DRVAGENT_GPIOPL_MAX_LEN; i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)(g_atSndCodeAddress + length), "%02X", gpioQryCnf->gpiopl[i]);
        }

        g_atSendDataBuff.bufLen = length;
        result                  = AT_OK;
    } else {
        /* 调用AT_FormATResultDATa发送命令结果 */
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_ERROR;
    }
    At_FormatResultData(indexNum, result);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentHkAdcGetRsp(struct MsgCB *msg)
{
    VOS_UINT32             ret;
    VOS_UINT8              indexNum = 0;
    DRV_AGENT_HkadcGetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg         *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_HkadcGetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待HKADC电压查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_TBAT_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = 0;
    if (event->result == DRV_AGENT_HKADC_GET_NO_ERROR) {
        /* 设置错误码为AT_OK */
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s:1,%d", g_parseContext[indexNum].cmdElement->cmdName,
                event->tbatHkadc);

        ret = AT_OK;

    } else {
        /* 查询失败返回ERROR字符串 */
        ret = AT_ERROR;
    }

    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentQryTbatRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg        *rcvMsg      = VOS_NULL_PTR;
    DRV_AGENT_TbatQryCnf *tbatTypeCnf = VOS_NULL_PTR;
    VOS_UINT8             indexNum    = 0;

    /* 初始化消息 */
    rcvMsg      = (DRV_AGENT_Msg *)msg;
    tbatTypeCnf = (DRV_AGENT_TbatQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(tbatTypeCnf->atAppCtrl.clientId, AT_CMD_TBAT_QRY, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName,
            tbatTypeCnf->tbatType);

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentChipTempQryRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg            *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_ChiptempQryCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                 indexNum = 0;
    VOS_UINT32                result;
    VOS_UINT16                length;

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_ChiptempQryCnf *)rcvMsg->content;
    length = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_CHIPTEMP_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_NO_ERROR) {
        /* 输出查询结果 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d",
            g_parseContext[indexNum].cmdElement->cmdName, event->gpaTemp, event->wpaTemp, event->lpaTemp,
            event->simTemp, event->batTemp);

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentNvBackupStatQryRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg                *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_NvbackupstatQryCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                     indexNum = 0;
    VOS_UINT32                    result;
    VOS_UINT16                    length;

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NvbackupstatQryCnf *)rcvMsg->content;
    length = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_NVBACKUPSTAT_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_NO_ERROR) {
        if (event->nvBackupStat == NV_OK) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                VOS_TRUE);
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                VOS_FALSE);
        }

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentNandBadBlockQryRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_NandbbcQryCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                indexNum = 0;
    VOS_UINT32               result;
    VOS_UINT16               length;
    VOS_UINT32               uli;
    VOS_UINT32               maxBBNum;

    /* 初始化消息变量 */
    rcvMsg   = (DRV_AGENT_Msg *)msg;
    event    = (DRV_AGENT_NandbbcQryCnf *)rcvMsg->content;
    length   = 0;
    uli      = 0;
    maxBBNum = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_NANDBBC_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_NO_ERROR) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            event->badBlockNum);

        maxBBNum = (event->badBlockNum > DRV_AGENT_NAND_BADBLOCK_MAX_NUM) ? DRV_AGENT_NAND_BADBLOCK_MAX_NUM :
                                                                            event->badBlockNum;
        for (uli = 0; uli < maxBBNum; uli++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%s%s: %d", g_atCrLf,
                g_parseContext[indexNum].cmdElement->cmdName, event->badBlockIndex[uli]);
        }

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentNandDevInfoQryRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;
    DRV_AGENT_NandverQryCnf *event    = VOS_NULL_PTR;
    VOS_UINT8                indexNum = 0;
    VOS_UINT32               result;
    VOS_UINT16               length;

    /* 初始化消息变量 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NandverQryCnf *)rcvMsg->content;
    length = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(event->atAppCtrl.clientId, AT_CMD_NANDVER_READ, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (event->result == DRV_AGENT_NO_ERROR) {
        /* 输出查询结果 */

        /* buf从外部接口返回的，增加结束符，防止缓冲器溢出 */
        event->nandDevInfo.mufName[DRV_AGENT_NAND_MFU_NAME_MAX_LEN - 1] = '\0';
        event->nandDevInfo.devSpec[DRV_AGENT_NAND_DEV_SPEC_MAX_LEN - 1] = '\0';
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d,\"%s\",%d,\"%s\"",
            g_parseContext[indexNum].cmdElement->cmdName, event->nandDevInfo.mufId, event->nandDevInfo.mufName,
            event->nandDevInfo.devId, event->nandDevInfo.devSpec);

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;
    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvDrvAgentInfoRrsSetRsp(struct MsgCB *msg)
{
    VOS_UINT8                indexNum = 0;
    DRV_AGENT_InforrsSetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_InforrsSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }

    /* AT模块在等待INFORRU设置命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_INFORRS_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置操作结果 :  */
    if (event->rslt == NV_OK) {
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvMtaPmuDieSNQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg            *rcvMsg         = VOS_NULL_PTR;
    MTA_AT_PmuDieSnReqCnf *pmuDieSNReqCnf = VOS_NULL_PTR;
    VOS_INT32              i; /* 循环数 */
    VOS_UINT16             length;

    /* 初始化局部变量 */
    rcvMsg         = (AT_MTA_Msg *)msg;
    pmuDieSNReqCnf = (MTA_AT_PmuDieSnReqCnf *)rcvMsg->content;
    length         = 0;

    /* 格式化上报命令 */
    if (pmuDieSNReqCnf->result != MTA_AT_RESULT_NO_ERROR) {
        /* 命令结果 *AT_ERROR */
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    } else {
        /* 命令结果 *AT_OK */

        /* 最高位,高4 BIT置0 */
        pmuDieSNReqCnf->dieSn[MTA_PMU_MAX_DIE_SN_LEN - 1] = (pmuDieSNReqCnf->dieSn[MTA_PMU_MAX_DIE_SN_LEN - 1] & 0x0F);

        /* 格式化输出查询结果 */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: 0x", g_parseContext[indexNum].cmdElement->cmdName);

        for (i = (MTA_PMU_MAX_DIE_SN_LEN - 1); i >= 0; i--) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02x", pmuDieSNReqCnf->dieSn[i]);
        }

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }
}

VOS_UINT32 AT_RcvDrvAgentDloadInfoQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                 ret;
    VOS_UINT8                  indexNum = 0;
    DRV_AGENT_DloadinfoQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg             *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_DloadinfoQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentDloadInfoQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentDloadInfoQryRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待DLOADINFO查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DLOADINFO_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_DLOADINFO_QRY_NO_ERROR) {
        /*
         * 设置错误码为AT_OK
         *   构造结构为^DLOADINFO:<CR><LF>
         * <CR><LF>swver:<software version><CR><LF>
         * <CR><LF>isover:<iso version><CR><LF>
         * <CR><LF>product name:<product name><CR><LF>
         * <CR><LF>product name:<WebUiVer><CR><LF>
         * <CR><LF>dload type: <dload type><CR><LF>
         * <CR><LF>OK<CR><LF>格式
         */
        ret                     = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s", event->dlodInfo);

    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentHwnatQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                ret;
    VOS_UINT8                 indexNum = 0;
    DRV_AGENT_HwnatqryQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg            *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_HwnatqryQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentHwnatQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentHwnatQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待HWNAT查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_HWNATQRY_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_HWNATQRY_QRY_NO_ERROR) {
        /*
         * 设置错误码为AT_OK
         * 构造结构为^HWNATQRY: <cur_mode> 格式
         */
        ret                     = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, event->netMode);
    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentAuthorityIdQryRsp(struct MsgCB *msg)
{
    VOS_UINT32                   ret;
    VOS_UINT8                    indexNum = 0;
    DRV_AGENT_AuthorityidQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg               *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_AuthorityidQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentAuthorityIdQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentAuthorityIdQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待AUTHORITYID查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_AUTHORITYID_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 :  */
    if (event->result == DRV_AGENT_AUTHORITYID_QRY_NO_ERROR) {
        /*
         * 设置错误码为AT_OK 格式为<CR><LF><Authority ID>, <Authority Type><CR><LF>
         * <CR><LF>OK<CR><LF>
         */
        /* buf从外部接口返回的，增加结束符，防止缓冲区溢出 */
        event->authorityId[TAF_AUTH_ID_LEN] = '\0';
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s", event->authorityId);

        ret = AT_OK;
    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_RcvDrvAgentGodloadSetRsp(struct MsgCB *msg)
{
    VOS_UINT32               ret;
    VOS_UINT8                indexNum = 0;
    DRV_AGENT_GodloadSetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_GodloadSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentGodloadSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentGodloadSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待GODLOAD查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_GODLOAD_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置操作结果 :  */
    g_atSendDataBuff.bufLen = 0;
    if (event->result == DRV_AGENT_GODLOAD_SET_NO_ERROR) {
        /* 设置错误码为AT_OK */
        ret = AT_OK;
    } else {
        /* 设置失败返回ERROR字符串 */
        ret = AT_ERROR;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvDrvAgentInfoRbuSetRsp(struct MsgCB *msg)
{
    VOS_UINT8                indexNum = 0;
    DRV_AGENT_InforbuSetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_InforbuSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentInfoRbuSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentInfoRbuSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待INFORBU设置命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_INFORBU_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置操作结果 :  */
    if (event->rslt == NV_OK) {
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentNvBackupSetRsp(struct MsgCB *msg)
{
    VOS_UINT8                 indexNum = 0;
    DRV_AGENT_NvbackupSetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg            *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NvbackupSetCnf *)rcvMsg->content;

    AT_PR_LOGI("Rcv Msg");

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentNvBackupSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentNvBackupSetRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待NVBACKUP测试命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_NVBACKUP_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    if (event->rslt == NV_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%d", event->rslt);
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

#if ((FEATURE_VSIM == FEATURE_ON) && (FEATURE_VSIM_ICC_SEC_CHANNEL == FEATURE_OFF))
VOS_UINT32 AT_RcvDrvAgentHvpdhSetCnf(struct MsgCB *msg)
{
    VOS_UINT32          ret;
    VOS_UINT8           indexNum;
    DRV_AGENT_HvpdhCnf *event  = VOS_NULL_PTR;
    DRV_AGENT_Msg      *rcvMsg = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_HvpdhCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentHvpdhSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentHvpdhSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待HVPDH设置命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_HVPDH_SET) {
        AT_WARN_LOG("AT_RcvDrvAgentHvpdhSetCnf: WARNING:Not AT_CMD_HVPDH_SET!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_HVPDH_NO_ERROR) {
        ret = AT_OK;
    } else {
        /* 查询失败返回ERROR字符串 */
        ret = AT_ERROR;
    }

    g_atSendDataBuff.usBufLen = 0;

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}
#endif

VOS_UINT32 AT_RcvNvManufactureExtSetCnf(struct MsgCB *msg)
{
    VOS_UINT8                         indexNum = 0;
    DRV_AGENT_NvmanufactureextSetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg                    *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_NvmanufactureextSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvNvManufactureExtSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvNvManufactureExtSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待NvManufactureExt设置命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_NVMANUFACTUREEXT_SET) {
        AT_WARN_LOG("AT_RcvNvManufactureExtSetCnf: WARNING:Not AT_CMD_NVMANUFACTUREEXT_SET!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    if (event->rslt == NV_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%d", event->rslt);
        At_FormatResultData(indexNum, AT_OK);
    } else {
        At_FormatResultData(indexNum, AT_ERROR);
    }

    return VOS_OK;
}

VOS_UINT32 atSetTbatCnf(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG         *event = NULL;
    FTM_SetTbatCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetTbatCnf *)event->param1;
    HAL_SDMLOG("\n enter into atSetTbatCnf\n");
    CmdErrProc(clientId, cnf->errCode, 0, NULL);
    return AT_OK;
}

VOS_UINT32 atRdTbatCnf(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG        *event = NULL;
    FTM_RdTbatCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdTbatCnf *)event->param1;
    HAL_SDMLOG("\n enter into atRdTbatCnf\n");
    HAL_SDMLOG("\n pstCnf->ucType=%d,pstCnf->usValue=%d \n", (VOS_INT)cnf->type, (VOS_INT)cnf->value);

    g_atSendData.bufLen = 0;
    g_atSendData.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr, "^TBAT:%d,%d", cnf->type, cnf->value);

    CmdErrProc(clientId, cnf->errCode, g_atSendData.bufLen, g_atSndCodeAddr);

    return AT_OK;
}

