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
#include "at_ltev_event_report.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_EVENT_REPORT_C

#if (FEATURE_LTEV == FEATURE_ON)

#if (FEATURE_MBB_CUST == FEATURE_ON)
/* LOG SERVICE信息变量定义 */
AT_LogServiceLtevInfo g_logServiceLtevInfo = {0};

/* CBR拥塞分级范围 */
AT_LogServiceCbrRange g_congestionLvlRangeTbl[AT_LOG_SERVICE_LTEV_CONGESTION_LVL_NUM] = {
    { 0, 60 },
    { 60, 80 },
    { 80, 100 },
};

/*
 * 功能说明: 获取LOG SERVICE LTE INFO实体
 * 返回结果: 实体LTEV INFO指针
 */
AT_LogServiceLtevInfo *AT_GetLogServiceEntityLtevInfo(LogServiceVoid)
{
    return &g_logServiceLtevInfo;
}

/*
 * 功能说明: 获取当前CBR拥塞所处的等级
 * 输入参数: cbr: 当前信道忙率
 * 返回结果: 当前拥塞的等级
 */
LogServiceUChar AT_GetLogServiceCbrLevel(LogServiceUInt cbr)
{
    LogServiceUChar i;

    /* 查找对应的拥塞等级 */
    for (i = 0; i < AT_LOG_SERVICE_LTEV_CONGESTION_LVL_NUM; i++) {
        if ((cbr < g_congestionLvlRangeTbl[i].end) && (cbr >= g_congestionLvlRangeTbl[i].begin)) {
            break;
        }
    }

    return i;
}
#endif
VOS_UINT32 AT_RcvCbrRptInd(MN_AT_IndEvt *msg)
{
    VMAC_AT_CbrPara *para     = VOS_NULL_PTR;
    VOS_UINT8        indexNum = 0;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    AT_LogServiceLtevInfo *ltevInfo = VOS_NULL_PTR;
    LogServiceUChar cbrLvl;
#endif

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    para                    = (VMAC_AT_CbrPara *)msg->content;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^CBR: %d%s", g_atCrLf, para->psschCbr, g_atCrLf);
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);

#if (FEATURE_MBB_CUST == FEATURE_ON)
    ltevInfo = AT_GetLogServiceEntityLtevInfo();
    cbrLvl = AT_GetLogServiceCbrLevel(para->psschCbr);
    if (cbrLvl != ltevInfo->cbrCurrentLvl) {
        LogServiceParamInfo logParamInfo;
        (VOS_VOID)memset_s(&logParamInfo, sizeof(logParamInfo), 0x00, sizeof(logParamInfo));
        logParamInfo.paramLtevInfo.ltevMode = cbrLvl;
        LOG_SERVICE_EvtCollect(LOG_SERVICE_EVT_MOBILITY_LTEV_LEVEL_CHANGE, &logParamInfo);
        ltevInfo->cbrCurrentLvl = cbrLvl;
    }
#endif

    return VOS_OK;
}

VOS_UINT32 AT_RcvRssiRptInd(MN_AT_IndEvt *msg)
{
    VMAC_AT_Pc5RssiPara *para     = VOS_NULL_PTR;
    VOS_UINT8            indexNum = 0;

    para = (VMAC_AT_Pc5RssiPara *)msg->content;
    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^VRSSI: %d%s", g_atCrLf, para->rssi, g_atCrLf);
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    return VOS_OK;
}

VOS_UINT32 AT_RcvSyncSourceRptInd(MN_AT_IndEvt *msg)
{
    VRRC_AT_SyncSourceStatePara *para     = VOS_NULL_PTR;
    VOS_UINT8                    indexNum = 0;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    AT_LogServiceLtevInfo *ltevInfo = VOS_NULL_PTR;
#endif

    if (At_ClientIdToUserId(msg->clientId, &indexNum) == AT_FAILURE) {
        return VOS_ERR;
    }
    para                    = (VRRC_AT_SyncSourceStatePara *)msg->content;
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s^VSYNCSRC: %d,%d,%d,%d,%d%s", g_atCrLf, para->type, para->status,
        para->earfcn, para->slssId, para->subSlssId, g_atCrLf);
    At_SendResultData(indexNum, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
#if (FEATURE_MBB_CUST == FEATURE_ON)
    ltevInfo = AT_GetLogServiceEntityLtevInfo();
    if (para->type != ltevInfo->vsyncCurrentType) {
        LogServiceParamInfo logParamInfo;
        (VOS_VOID)memset_s(&logParamInfo, sizeof(logParamInfo), 0x00, sizeof(logParamInfo));
        logParamInfo.paramLtevInfo.vsyncOrigType = ltevInfo->vsyncCurrentType;
        logParamInfo.paramLtevInfo.vsyncCurrentType = para->type;
        ltevInfo->vsyncCurrentType = para->type;

        LOG_SERVICE_EvtCollect(LOG_SERVICE_EVT_MOBILITY_LTEV_TYPE_CHANGE, &logParamInfo);
    }

    if (para->status != ltevInfo->vsyncCurrentState) {
        LogServiceParamInfo logParamInfo;
        (VOS_VOID)memset_s(&logParamInfo, sizeof(logParamInfo), 0x00, sizeof(logParamInfo));
        ltevInfo->vsyncCurrentState = para->status;
        logParamInfo.paramLtevInfo.vsyncCurrentState = ltevInfo->vsyncCurrentState;

        LOG_SERVICE_EvtCollect(LOG_SERVICE_EVT_MOBILITY_LTEV_TYPE_CHANGE, &logParamInfo);
    }
#endif

    return VOS_OK;
}
#endif

