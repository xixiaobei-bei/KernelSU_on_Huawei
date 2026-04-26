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
#include "at_custom_comm_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#if (FEATURE_MBB_CUST == FEATURE_ON)
#include "at_data_proc.h"
#endif


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_COMM_RSLT_PROC_C

#if (FEATURE_MBB_CUST == FEATURE_ON)
/* 记录拨号的历史拒绝码，分CID保存 */
LOCAL AtPsCallErrorCode g_psCallErrCode[TAF_MAX_CID + 1] = {{0}};
#endif

VOS_UINT32 AT_RcvMtaCommBoosterSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *rcvMsg                 = VOS_NULL_PTR;
    MTA_AT_CommBoosterSetCnf *mtaAtSetCommBoosterCnf = VOS_NULL_PTR;
    VOS_UINT32                result;

    /* 初始化 */
    rcvMsg                 = (AT_MTA_Msg *)msg;
    mtaAtSetCommBoosterCnf = (MTA_AT_CommBoosterSetCnf *)rcvMsg->content;
    result                 = AT_OK;
    g_atSendDataBuff.bufLen = 0;

    if ((mtaAtSetCommBoosterCnf->internalResultFlag == VOS_FALSE) &&
        (mtaAtSetCommBoosterCnf->externalResultFlag == VOS_FALSE)) {
        result = AT_OK;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, 0);
    } else if (mtaAtSetCommBoosterCnf->internalResultFlag == VOS_TRUE) {
        result = AT_ERROR;
    } else {
        result                  = AT_ERROR;
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            mtaAtSetCommBoosterCnf->result);
    }

    return result;
}

VOS_UINT32 AT_RcvMtaCommBoosterQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                 *rcvMsg                   = VOS_NULL_PTR;
    MTA_AT_CommBoosterQueryCnf *mtaAtQueryCommBoosterCnf = VOS_NULL_PTR;
    VOS_UINT32                  result;
    VOS_UINT16                  length;
    VOS_UINT32                  i;

    /* 初始化 */
    rcvMsg                   = (AT_MTA_Msg *)msg;
    mtaAtQueryCommBoosterCnf = (MTA_AT_CommBoosterQueryCnf *)rcvMsg->content;
    result                   = AT_OK;
    length                   = 0;
    g_atSendDataBuff.bufLen  = 0;

    if (mtaAtQueryCommBoosterCnf->result == VOS_FALSE) {
        result = AT_ERROR;
    } else {
        result = AT_OK;

        /* \r\n^COMMBOOSTER */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: \"", g_parseContext[indexNum].cmdElement->cmdName);

        if (mtaAtQueryCommBoosterCnf->len > MTA_AT_BOOSTER_QRY_CNF_MAX_NUM) {
            mtaAtQueryCommBoosterCnf->len = MTA_AT_BOOSTER_QRY_CNF_MAX_NUM;
        }

        for (i = 0; (i < mtaAtQueryCommBoosterCnf->len); i++) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%02X", mtaAtQueryCommBoosterCnf->date[i]);
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"");
        g_atSendDataBuff.bufLen = length;
    }

    return result;
}

VOS_UINT32 AT_RcvDrvAgentSwverSetCnf(struct MsgCB *msg)
{
    VOS_UINT32             ret;
    VOS_UINT8              indexNum = 0;
    DRV_AGENT_SwverSetCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg         *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_SwverSetCnf *)rcvMsg->content;

    AT_PR_LOGI("Rcv Msg");

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentSwverSetCnf: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentSwverSetCnf: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待SWVER查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SWVER_SET) {
        AT_WARN_LOG("AT_RcvDrvAgentSwverSetCnf: WARNING:Not AT_CMD_SWVER_SET!");
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    if (event->result == DRV_AGENT_NO_ERROR) {
        /*
         * 设置错误码为AT_OK           构造结构为<CR><LF>^SWVER: <SwVer>_(<VerTime>)<CR><LF>
         * <CR><LF>OK<CR><LF>格式
         */
        ret = AT_OK;

        /* buf从外部接口返回的，增加结束符，防止缓冲区溢出 */
        event->swverInfo.swVer[TAF_MAX_REVISION_ID_LEN]             = '\0';
        event->swverInfo.verTime[AT_AGENT_DRV_VERSION_TIME_LEN - 1] = '\0';
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s_(%s)", g_parseContext[indexNum].cmdElement->cmdName,
            event->swverInfo.swVer, event->swverInfo.verTime);

    } else {
        /* 查询失败返回ERROR字符串 */
        ret                     = AT_ERROR;
        g_atSendDataBuff.bufLen = 0;
    }

    /* 调用At_FormatResultData输出结果 */
    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 AT_RcvMtaChrAlarmRlatCfgSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg                   *rcvMsg         = VOS_NULL_PTR;
    MTA_AT_ChrAlarmRlatCfgSetCnf *chrAlarmSetCnf = VOS_NULL_PTR;
    VOS_UINT32                    result;
    VOS_UINT16                    length;

    /* 初始化 */
    rcvMsg         = (AT_MTA_Msg *)msg;
    chrAlarmSetCnf = (MTA_AT_ChrAlarmRlatCfgSetCnf *)rcvMsg->content;
    result         = AT_ERROR;

    /* 格式化命令返回 */
    g_atSendDataBuff.bufLen = 0;

    if (chrAlarmSetCnf->result == MTA_AT_RESULT_NO_ERROR) {
        if (chrAlarmSetCnf->alarmOp == AT_MTA_CHR_ALARM_RLAT_OP_READ) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                chrAlarmSetCnf->alarmDetail);

            g_atSendDataBuff.bufLen = length;
        }

        result = AT_OK;
    }

    return result;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: 设置指定IP类型的拒绝码
 * 输入参数: pdpType: IP类型
 *           cause: 拒绝原因值
 *           cid: 拨号被拒的CID
 */
LOCAL VOS_VOID AT_PS_Set3GppSmCause(TAF_PDP_TypeUint8 pdpType, VOS_UINT32 cause, VOS_UINT8 cid)
{
    if (cid > TAF_MAX_CID) {
        return;
    }

    switch (pdpType) {
        case TAF_PDP_IPV4: {
            g_psCallErrCode[cid].ipv4Cause = cause;
            break;
        }
        case TAF_PDP_IPV6: {
            g_psCallErrCode[cid].ipv6Cause = cause;
            break;
        }
        case TAF_PDP_IPV4V6: {
            g_psCallErrCode[cid].ipv4Cause = cause;
            g_psCallErrCode[cid].ipv6Cause = cause;
            break;
        }
        default: {
            break;
        }
    }
}

/*
 * 功能说明: 根据IP类型获取拒绝码
 * 输入参数: pdpType: IP类型
 *           cid: 指定的CID
 * 返回结果: 查询的CID的拒绝码
 */
LOCAL VOS_UINT32 AT_PS_Get3GppSmCause(TAF_PDP_TypeUint8 pdpType, VOS_UINT8 cid)
{
    VOS_UINT32 cause = 0;

    if (cid > TAF_MAX_CID) {
        return cause;
    }

    switch (pdpType) {
        case TAF_PDP_IPV4: {
            cause = g_psCallErrCode[cid].ipv4Cause;
            break;
        }
        case TAF_PDP_IPV6: {
            cause = g_psCallErrCode[cid].ipv6Cause;
            break;
        }
        default: {
            break;
        }
    }

    return cause;
}

/*
 * 功能说明: 清除拨号拒绝码
 * 输入参数: pdpType: IP类型
 *           cid: 指定的CID
 */
LOCAL VOS_VOID AT_PS_Clear3GppSmCause(TAF_PDP_TypeUint8 pdpType, VOS_UINT8 cid)
{
    if (cid > TAF_MAX_CID) {
        return;
    }

    switch (pdpType) {
        case TAF_PDP_IPV4: {
            g_psCallErrCode[cid].ipv4Cause = 0;
            break;
        }
        case TAF_PDP_IPV6: {
            g_psCallErrCode[cid].ipv6Cause = 0;
            break;
        }
        case TAF_PDP_IPV4V6: {
            g_psCallErrCode[cid].ipv4Cause = 0;
            g_psCallErrCode[cid].ipv6Cause = 0;
            break;
        }
        default: {
            break;
        }
    }
}

/*
 * 功能说明: 按照拨号结果处理保存的拨号拒绝原因值
 * 输入参数: ifaceStatus: IFACE状态指针，包含拨号结果参数
 *           smCause: 拨号拒绝原因
 */
VOS_VOID AT_PS_ProcSmCauseByPsResult(const TAF_IFACE_StatusInd *ifaceStatus, VOS_UINT32 smCause)
{
    /* 入参指针检查 */
    if (ifaceStatus == VOS_NULL_PTR) {
        return;
    }

    /* 如果是拨号成功，则清除之前记录的失败原因 */
    if (ifaceStatus->cause == TAF_PS_CAUSE_SUCCESS) {
        AT_PS_Clear3GppSmCause(ifaceStatus->pdpType, ifaceStatus->cid);
    } else {
        /* 如果是拨号失败，将原因值按照IP类型进行记录 */
        AT_PS_Set3GppSmCause(ifaceStatus->pdpType, smCause, ifaceStatus->cid);
    }
}

/*
 * 功能说明: 从CID BITMASK中获取有效的CID
 * 输入参数: cidMask: 结果中的CID BITMASK
 * 返回结果: CID BITMASK中有效的CID值
 */
VOS_UINT8 AT_PS_GetUsrCidFromCidMask(VOS_UINT32 cidMask)
{
    VOS_UINT32 usrCid;

    for (usrCid = 1; usrCid <= TAF_MAX_CID_NV; usrCid++) {
        /* 判断此CID是否有效 */
        if ((cidMask & (0x01UL << usrCid)) != 0) {
            break;
        }
    }

    return usrCid;
}

/*
 * 功能说明: 格式化拨号字符串结果
 * 输入参数: index: AT命令输入端口
 *           userCid: 查询的CID
 *           ipv4Status: V4的拨号状态
 *           ipv6Status: V6的拨号状态
 * 返回结果: 格式化后的结果字符串长度
 */
VOS_UINT16 AT_PS_FormatNdisStatPrint(VOS_UINT8 indexNum, VOS_UINT8 userCid, VOS_UINT32 ipv4Status,
    VOS_UINT32 ipv6Status)
{
    VOS_UINT32 cause;
    VOS_UINT16 length;

    /* 如果CID参数非法，直接返回0 */
    if (userCid > TAF_MAX_CID_NV) {
        return 0;
    }

    /* 先打印AT名称 */
    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress, (VOS_CHAR*)g_atSndCodeAddress,
        "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    switch (AT_GetIpv6Capability()) {
        case AT_IPV6_CAPABILITY_IPV4_ONLY: {
            /* 获取IPv4的拒绝原因值 */
            cause = AT_PS_Get3GppSmCause(TAF_PDP_IPV4, userCid);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,  (VOS_CHAR*)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,,\"%s\"", ipv4Status, cause, "IPV4");
            break;
        }
        case AT_IPV6_CAPABILITY_IPV6_ONLY: {
            /* 获取IPv6的拒绝原因值 */
            cause = AT_PS_Get3GppSmCause(TAF_PDP_IPV6, userCid);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,,\"%s\"", ipv6Status, cause, "IPV6");
            break;
        }
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_ONE_PDP:
        case AT_IPV6_CAPABILITY_IPV4V6_OVER_TWO_PDP: {
            /* 获取IPv4的拒绝原因值 */
            cause = AT_PS_Get3GppSmCause(TAF_PDP_IPV4, userCid);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,%d,,\"%s\"", ipv4Status, cause, "IPV4");
            /* 获取IPv6的拒绝原因值 */
            cause = AT_PS_Get3GppSmCause(TAF_PDP_IPV6, userCid);
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR*)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d,%d,,\"%s\"", ipv6Status, cause, "IPV6");
            break;
        }
        default: {
            break;
        }
    }

    return length;
}
#endif

