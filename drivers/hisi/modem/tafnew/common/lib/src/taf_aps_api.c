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

#include "ps_common_def.h"
#include "taf_ps_api.h"
#if (OSA_CPU_CCPU == VOS_OSA_CPU)
#include "taf_aps.h"
#include "taf_aps_mntn.h"
#endif
#include "securec.h"
#include "mn_comm_api.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_TAF_APS_API_C

/* Log Print Module Define */
#ifndef THIS_MODU
#define THIS_MODU ps_nas
#endif

VOS_UINT32 TAF_PS_SndMsg(VOS_UINT16 modemId, VOS_UINT32 msgId, const VOS_VOID *data, VOS_UINT32 length)
{
    VOS_UINT32  result = 0;
    TAF_PS_Msg *msg    = VOS_NULL_PTR;
    VOS_UINT32  pid;
    VOS_UINT32  sendPid;
    errno_t     memResult;

    switch (modemId) {
        case MODEM_ID_0:
            pid = I0_WUEPS_PID_TAF;
            break;

        case MODEM_ID_1:
            pid = I1_WUEPS_PID_TAF;
            break;

        case MODEM_ID_2:
            pid = I2_WUEPS_PID_TAF;
            break;

        default:
            PS_PRINTF_WARNING("<TAF_PS_SndMsg> ModemId is Error!\n");
            return VOS_ERR;
    }
    sendPid = pid;

#if (OSA_CPU_ACPU == VOS_OSA_CPU)
    sendPid = ACPU_PID_TAF;
#endif

    /* 构造消息 */
    msg = (TAF_PS_Msg *)TAF_AllocMsgWithHeaderLen(pid, sizeof(MSG_Header) + length);
    if (msg == VOS_NULL_PTR) {
        return VOS_ERR;
    }
    VOS_SET_SENDER_ID(msg, sendPid);

    VOS_SET_RECEIVER_ID(msg, pid);
    msg->header.msgName = msgId;

    /* 填写消息内容 */
    if (length > 0) {
        memResult = memcpy_s(msg->content, length, data, length);
        TAF_MEM_CHK_RTN_VAL(memResult, length, length);
    }

    /* 发送消息 */
    result = TAF_TraceAndSendMsg(pid, msg);
    if (result != VOS_OK) {
        return VOS_ERR;
    }

    return VOS_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
VOS_UINT32 TAF_PS_GetDsFlowInfo(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 usrCid, VOS_UINT8 opId)
#else
VOS_UINT32 TAF_PS_GetDsFlowInfo(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId)
#endif
{
    VOS_UINT32              result;
    TAF_PS_GetDsflowInfoReq getDsFlowInfoReq;

    /* 初始化 */
    result = VOS_OK;
    (VOS_VOID)memset_s(&getDsFlowInfoReq, sizeof(getDsFlowInfoReq), 0x00, sizeof(TAF_PS_GetDsflowInfoReq));

    /* 构造ID_MSG_TAF_PS_GET_DSFLOW_INFO_REQ消息 */
    getDsFlowInfoReq.ctrl.moduleId = moduleId;
    getDsFlowInfoReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getDsFlowInfoReq.ctrl.opId     = opId;
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /* 指定CID获取流量，CID需要传到C核 */
    getDsFlowInfoReq.usrCid = usrCid;
#endif
    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_DSFLOW_INFO_REQ,
                           &getDsFlowInfoReq, sizeof(TAF_PS_GetDsflowInfoReq));

    return result;
}

VOS_UINT32 TAF_PS_ClearDsFlowInfo(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId,
                                  TAF_DSFLOW_ClearConfig *clearConfigInfo)
{
    VOS_UINT32            result = VOS_OK;
    TAF_PS_ClearDsflowReq clearDsFlowReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&clearDsFlowReq, sizeof(clearDsFlowReq), 0x00, sizeof(TAF_PS_ClearDsflowReq));

    /* 构造ID_MSG_TAF_PS_CLEAR_DSFLOW_REQ消息 */
    clearDsFlowReq.ctrl.moduleId   = moduleId;
    clearDsFlowReq.ctrl.clientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    clearDsFlowReq.ctrl.opId       = opId;
    clearDsFlowReq.clearConfigInfo = *clearConfigInfo;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_CLEAR_DSFLOW_REQ,
                           &clearDsFlowReq, sizeof(TAF_PS_ClearDsflowReq));

    return result;
}

VOS_UINT32 TAF_PS_ConfigDsFlowRpt(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId,
                                  TAF_DSFLOW_ReportConfig *reportConfigInfo)
{
    VOS_UINT32                result = VOS_OK;
    TAF_PS_ConfigDsflowRptReq configDsFlowRptReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&configDsFlowRptReq, sizeof(configDsFlowRptReq), 0x00, sizeof(TAF_PS_ConfigDsflowRptReq));

    /* 构造ID_MSG_TAF_PS_CONFIG_DSFLOW_RPT_REQ消息 */
    configDsFlowRptReq.ctrl.moduleId    = moduleId;
    configDsFlowRptReq.ctrl.clientId    = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    configDsFlowRptReq.ctrl.opId        = opId;
    configDsFlowRptReq.reportConfigInfo = *reportConfigInfo;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_CONFIG_DSFLOW_RPT_REQ,
                           &configDsFlowRptReq, sizeof(TAF_PS_ConfigDsflowRptReq));

    return result;
}

VOS_UINT32 TAF_PS_ConfigVTFlowRpt(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId,
                                  TAF_VTFLOW_ReportConfig *reportConfigInfo)
{
    VOS_UINT32                result = VOS_OK;
    TAF_PS_ConfigVtflowRptReq configVTFlowRptReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&configVTFlowRptReq, sizeof(configVTFlowRptReq), 0x00, sizeof(TAF_PS_ConfigVtflowRptReq));

    /* 构造ID_MSG_TAF_PS_CONFIG_VTFLOW_RPT_REQ消息 */
    configVTFlowRptReq.ctrl.moduleId    = moduleId;
    configVTFlowRptReq.ctrl.clientId    = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    configVTFlowRptReq.ctrl.opId        = opId;
    configVTFlowRptReq.reportConfigInfo = *reportConfigInfo;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_CONFIG_VTFLOW_RPT_REQ,
                           &configVTFlowRptReq, sizeof(TAF_PS_ConfigVtflowRptReq));

    return result;
}

#if (FEATURE_UE_MODE_G == FEATURE_ON)

VOS_UINT32 TAF_PS_TrigGprsData(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId, TAF_GPRS_Data *gprsDataInfo)
{
    VOS_UINT32             result = VOS_OK;
    TAF_PS_TrigGprsDataReq trigGprsDataReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&trigGprsDataReq, sizeof(trigGprsDataReq), 0x00, sizeof(TAF_PS_TrigGprsDataReq));

    /* 构造ID_MSG_TAF_PS_TRIG_GPRS_DATA_REQ消息 */
    trigGprsDataReq.ctrl.moduleId = moduleId;
    trigGprsDataReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    trigGprsDataReq.ctrl.opId     = opId;
    trigGprsDataReq.gprsDataInfo  = *gprsDataInfo;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_TRIG_GPRS_DATA_REQ,
                           &trigGprsDataReq, sizeof(TAF_PS_TrigGprsDataReq));

    return result;
}
#endif

VOS_UINT32 TAF_PS_SetApDsFlowRptCfg(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId,
                                    TAF_APDSFLOW_RptCfg *rptCfg)
{
    TAF_PS_SetApdsflowRptCfgReq setRptCfgReq;
    VOS_UINT32                  result;
    errno_t                     memResult;

    (VOS_VOID)memset_s(&setRptCfgReq, sizeof(setRptCfgReq), 0x00, sizeof(TAF_PS_SetApdsflowRptCfgReq));

    /* 构造ID_MSG_TAF_PS_SET_APDSFLOW_RPT_CFG_REQ消息 */
    setRptCfgReq.ctrl.moduleId = moduleId;
    setRptCfgReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    setRptCfgReq.ctrl.opId     = opId;

    memResult = memcpy_s(&(setRptCfgReq.rptCfg), sizeof(setRptCfgReq.rptCfg), rptCfg, sizeof(TAF_APDSFLOW_RptCfg));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(setRptCfgReq.rptCfg), sizeof(TAF_APDSFLOW_RptCfg));

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_SET_APDSFLOW_RPT_CFG_REQ,
                           &setRptCfgReq, sizeof(TAF_PS_SetApdsflowRptCfgReq));

    return result;
}

VOS_UINT32 TAF_PS_GetApDsFlowRptCfg(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId)
{
    TAF_PS_GetApdsflowRptCfgReq getRptCfgReq;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&getRptCfgReq, sizeof(getRptCfgReq), 0x00, sizeof(TAF_PS_GetApdsflowRptCfgReq));

    /* 构造ID_MSG_TAF_PS_GET_APDSFLOW_RPT_CFG_REQ消息 */
    getRptCfgReq.ctrl.moduleId = moduleId;
    getRptCfgReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getRptCfgReq.ctrl.opId     = opId;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_APDSFLOW_RPT_CFG_REQ,
                           &getRptCfgReq, sizeof(TAF_PS_GetApdsflowRptCfgReq));

    return result;
}

VOS_UINT32 TAF_PS_SetDsFlowNvWriteCfg(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId,
                                      TAF_DSFLOW_NvWriteCfg *nvWriteCfg)
{
    TAF_PS_SetDsflowNvWriteCfgReq setWriteNvCfgReq;
    VOS_UINT32                    result;
    errno_t                       memResult;

    (VOS_VOID)memset_s(&setWriteNvCfgReq, sizeof(setWriteNvCfgReq), 0x00, sizeof(TAF_PS_SetDsflowNvWriteCfgReq));

    /* 构造ID_MSG_TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ消息 */
    setWriteNvCfgReq.ctrl.moduleId = moduleId;
    setWriteNvCfgReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    setWriteNvCfgReq.ctrl.opId     = opId;

    memResult = memcpy_s(&(setWriteNvCfgReq.nvWriteCfg), sizeof(setWriteNvCfgReq.nvWriteCfg), nvWriteCfg,
                         sizeof(TAF_DSFLOW_NvWriteCfg));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(setWriteNvCfgReq.nvWriteCfg), sizeof(TAF_DSFLOW_NvWriteCfg));

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_SET_DSFLOW_NV_WRITE_CFG_REQ,
                           &setWriteNvCfgReq, sizeof(TAF_PS_SetDsflowNvWriteCfgReq));

    return result;
}

VOS_UINT32 TAF_PS_GetDsFlowNvWriteCfg(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId)
{
    TAF_PS_GetDsflowNvWriteCfgReq getNvWriteCfgReq;
    VOS_UINT32                    result;

    (VOS_VOID)memset_s(&getNvWriteCfgReq, sizeof(getNvWriteCfgReq), 0x00, sizeof(TAF_PS_GetDsflowNvWriteCfgReq));

    /* 构造ID_MSG_TAF_PS_GET_DSFLOW_WRITE_NV_CFG_REQ消息 */
    getNvWriteCfgReq.ctrl.moduleId = moduleId;
    getNvWriteCfgReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getNvWriteCfgReq.ctrl.opId     = opId;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_DSFLOW_NV_WRITE_CFG_REQ,
                           &getNvWriteCfgReq, sizeof(TAF_PS_GetDsflowNvWriteCfgReq));

    return result;
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)

VOS_UINT32 TAF_PS_SetCdataDialModeInfo(VOS_UINT32 moduleId, VOS_UINT16 exClientId, TAF_PS_CdataDialModeUint32 dialMode)
{
    VOS_UINT32              result = VOS_OK;
    TAF_PS_CdataDialModeReq dialModeReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&dialModeReq, sizeof(dialModeReq), 0x00, sizeof(TAF_PS_CdataDialModeReq));

    /* 构造TAF_PS_CdataDialModeReq消息 */
    dialModeReq.ctrl.moduleId = moduleId;
    dialModeReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    dialModeReq.ctrl.opId     = 0;
    dialModeReq.dialMode      = dialMode;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_SET_CDMA_DIAL_MODE_REQ,
                           &dialModeReq, sizeof(TAF_PS_CdataDialModeReq));

    return result;
}

TAF_PS_CdataBearStatusUint8 TAF_PS_GetPppStatus(VOS_VOID)
{
    TAF_PS_CdataBearStatusUint8 cdataBearStatus = TAF_PS_CDATA_BEAR_STATUS_INACTIVE;

#if (OSA_CPU_CCPU == VOS_OSA_CPU)
    cdataBearStatus = TAF_APS_GetCdataBearStatus();
#endif

    return cdataBearStatus;
}

#endif

#if (FEATURE_ON == FEATURE_LTE)

VOS_UINT32 TAF_PS_GetLteCsInfo(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId

)
{
    VOS_UINT32      result = VOS_OK;
    TAF_PS_LtecsReq getLteCsInfoReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&getLteCsInfoReq, sizeof(getLteCsInfoReq), 0x00, sizeof(TAF_PS_LtecsReq));

    /* 构造ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    getLteCsInfoReq.ctrl.moduleId = moduleId;
    getLteCsInfoReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getLteCsInfoReq.ctrl.opId     = opId;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_LTE_CS_REQ,
                           &getLteCsInfoReq, sizeof(TAF_PS_LtecsReq));

    return result;
}

VOS_UINT32 TAF_PS_GetCemodeInfo(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId)
{
    VOS_UINT32       result = VOS_OK;
    TAF_PS_CemodeReq getCemodeInfoReq;

    /* 初始化 */
    (VOS_VOID)memset_s(&getCemodeInfoReq, sizeof(getCemodeInfoReq), 0x00, sizeof(TAF_PS_CemodeReq));

    /* 构造ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    getCemodeInfoReq.ctrl.moduleId = moduleId;
    getCemodeInfoReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getCemodeInfoReq.ctrl.opId     = opId;

    /* 发送消息 */
    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_CEMODE_REQ,
                           &getCemodeInfoReq, sizeof(TAF_PS_CemodeReq));

    return result;
}
#endif

VOS_UINT32 TAF_PS_Set_DataSwitchStatus(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId, VOS_UINT8 dataSwitch)
{
    TAF_PS_SetDataSwitchReq dataSwitchReq;
    VOS_UINT32              result;

    (VOS_VOID)memset_s(&dataSwitchReq, sizeof(dataSwitchReq), 0x00, sizeof(dataSwitchReq));

    dataSwitchReq.ctrl.moduleId = moduleId;
    dataSwitchReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    dataSwitchReq.ctrl.opId     = opId;
    dataSwitchReq.dataSwitch    = dataSwitch;

    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_SET_DATA_SWITCH_REQ,
                           &dataSwitchReq, sizeof(dataSwitchReq));

    return result;
}

VOS_UINT32 TAF_PS_Set_DataRoamSwitchStatus(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId,
                                           VOS_UINT8 dataRoamSwitch)
{
    TAF_PS_SetDataRoamSwitchReq dataRoamSwitchReq;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&dataRoamSwitchReq, sizeof(dataRoamSwitchReq), 0x00, sizeof(dataRoamSwitchReq));

    dataRoamSwitchReq.ctrl.moduleId  = moduleId;
    dataRoamSwitchReq.ctrl.clientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    dataRoamSwitchReq.ctrl.opId      = opId;
    dataRoamSwitchReq.dataRoamSwitch = dataRoamSwitch;

    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_SET_DATA_ROAM_SWITCH_REQ,
                           &dataRoamSwitchReq, sizeof(dataRoamSwitchReq));

    return result;
}

VOS_UINT32 TAF_PS_Get_DataSwitchStatus(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId)
{
    TAF_PS_GET_DATA_SWITCH_REQ_STRU getDataSwitchReq;
    VOS_UINT32                      result;

    (VOS_VOID)memset_s(&getDataSwitchReq, sizeof(getDataSwitchReq), 0x00, sizeof(getDataSwitchReq));

    getDataSwitchReq.ctrl.moduleId = moduleId;
    getDataSwitchReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getDataSwitchReq.ctrl.opId     = opId;

    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_DATA_SWITCH_REQ,
                           &getDataSwitchReq, sizeof(getDataSwitchReq));

    return result;
}

VOS_UINT32 TAF_PS_Get_DataRoamSwitchStatus(VOS_UINT32 moduleId, VOS_UINT16 exClientId, VOS_UINT8 opId)
{
    TAF_PS_GET_DATA_ROAM_SWITCH_REQ_STRU getDataRoamSwitchReq;
    VOS_UINT32                           result;

    (VOS_VOID)memset_s(&getDataRoamSwitchReq, sizeof(getDataRoamSwitchReq), 0x00, sizeof(getDataRoamSwitchReq));

    getDataRoamSwitchReq.ctrl.moduleId = moduleId;
    getDataRoamSwitchReq.ctrl.clientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(exClientId);
    getDataRoamSwitchReq.ctrl.opId     = opId;

    result = TAF_PS_SndMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(exClientId), ID_MSG_TAF_PS_GET_DATA_ROAM_SWITCH_REQ,
                           &getDataRoamSwitchReq, sizeof(getDataRoamSwitchReq));

    return result;
}

