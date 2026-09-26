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
#include "at_custom_lnas_rslt_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "css_at_interface.h"
#include "at_event_report.h"
#include "at_custom_event_report.h"
#include "at_external_module_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_LNAS_RSLT_PROC_C

#if ((FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON) || (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON))
LOCAL VOS_UINT32 AT_RcvThrotCommonSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CnfMsg *cnfMsg = (THROT_AT_CnfMsg *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (cnfMsg->result == THROT_AT_RESULT_SUCC) {
        return AT_OK;
    }
    return AT_ERROR;
}
/* AT与CSS模块间消息处理函数指针 */
LOCAL AT_MsgProcEntry g_atThrotMsgTab[] = {
    /* 消息ID */ /* 消息处理函数 */
#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    { ID_THROT_AT_CABTSRI_CONFIGURATION_SET_CNF,     AT_CMD_CABTSR_SET,        AT_RcvThrotCommonSetCnf },
    { ID_THROT_AT_CABTSRI_CONFIGURATION_QUERY_CNF,   AT_CMD_CABTSR_QRY,        AT_RcvThortCabtsrQueryCnf },
    { ID_THROT_AT_CABTRDP_QUERY_CNF,                 AT_CMD_CABTRDP_SET,       AT_RcvThortCabtrdpSetCnf },
    { ID_THROT_AT_SINGLE_APN_BACKOFF_TIMER_PARA_NTF, AT_CMD_OPT_NO_NEED_CHECK, AT_RcvThortSingleApnBackoffTimeParaNtf },
#endif
#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    { ID_THROT_AT_CSBTSRI_CONFIGURATION_SET_CNF,            AT_CMD_CSBTSR_SET,  AT_RcvThrotCommonSetCnf },
    { ID_THROT_AT_CSBTSRI_CONFIGURATION_QUERY_CNF,          AT_CMD_CSBTSR_QRY,  AT_RcvThortCsbtsrQueryCnf },
    { ID_THROT_AT_CSBTRDP_QUERY_CNF,                        AT_CMD_CSBTRDP_SET, AT_RcvThortCsbtrdpSetCnf },
    { ID_THROT_AT_SINGLE_SNSSAI_BACKOFF_TIMER_PARA_NTF,     AT_CMD_OPT_NO_NEED_CHECK,
        AT_RcvThortSingleSnssaiBackoffTimerParaNtf },
    { ID_THROT_AT_CSDBTSRI_CONFIGURATION_SET_CNF,           AT_CMD_CSDBTSR_SET,  AT_RcvThrotCommonSetCnf },
    { ID_THROT_AT_CSDBTSRI_CONFIGURATION_QUERY_CNF,         AT_CMD_CSDBTSR_QRY,  AT_RcvThortCsdbtsrQueryCnf },
    { ID_THROT_AT_CSDBTRDP_QUERY_CNF,                       AT_CMD_CSDBTRDP_SET, AT_RcvThortCsdbtrdpSetCnf },
    { ID_THROT_AT_SINGLE_SNSSAI_DNN_BACKOFF_TIMER_PARA_NTF, AT_CMD_OPT_NO_NEED_CHECK,
        AT_RcvThortSingleSnssaiDnnBackoffTimerParaNtf },
#endif
};

LOCAL CONST AT_MsgProcEntryTblInfo g_atProcThrotMsgTblInfo = {
    .tbl      = g_atThrotMsgTab,
    .entryNum = AT_ARRAY_SIZE(g_atThrotMsgTab),
    .entryLen = sizeof(g_atThrotMsgTab[0])
};

LOCAL VOS_UINT32 AT_GetThrotUserId(CONST VOS_VOID *msg, VOS_UINT8 *indexNum)
{
    AT_THROT_Msg *throtMsg = (AT_THROT_Msg *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(throtMsg->clientId, indexNum) != AT_SUCCESS) {
        AT_WARN_LOG("AT_GetThrotUserId: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }
    return VOS_OK;
}

LOCAL CONST AT_ModuleMsgProcInfo g_atThrotMsgProcInfoTbl = {
    .moduleId   = AT_PROC_MODULE_THROT,
    .tblInfo    = &g_atProcThrotMsgTblInfo,
    .GetMsgId   = VOS_NULL_PTR,
    .GetUserId  = AT_GetThrotUserId,
};

VOS_VOID AT_InitThrotMsgProcTbl(VOS_VOID)
{
    VOS_UINT32 ret = AT_RegModuleMsgProcInfo(&g_atThrotMsgProcInfoTbl);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_InitThrotMsgProcTbl: register fail");
    }
}

/* 处理来自THROT模块的消息 */
VOS_VOID AT_ProcThrotMsg(struct MsgCB *msg)
{
    VOS_UINT32 ret = AT_ProcExternalModuleMsg(msg, AT_PROC_MODULE_THROT);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_ProcThrotMsg: fail");
    }
}
#endif

VOS_UINT32 AT_RcvCssMccInfoSetCnf(struct MsgCB *msg)
{
    /* 定义局部变量 */
    CSS_AT_MccInfoSetCnf *mccInfoSetCnf = VOS_NULL_PTR;
    VOS_UINT8             indexNum = 0;
    VOS_UINT32            result = AT_ERROR;
    VOS_UINT32            retVal = VOS_ERR;

    /* 初始化消息变量 */
    mccInfoSetCnf = (CSS_AT_MccInfoSetCnf *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(mccInfoSetCnf->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssMccInfoSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvCssMccInfoSetCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_MCCFREQ_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MCCFREQ_SET) {
        AT_WARN_LOG("AT_RcvCssMccInfoSetCnf: WARNING:Not AT_CMD_MCCFREQ_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (mccInfoSetCnf->result == VOS_OK) {
        result = AT_OK;
        retVal = VOS_OK;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);

    return retVal;
}

VOS_UINT32 AT_RcvCssMccVersionQryCnf(struct MsgCB *msg)
{
    /* 定义局部变量 */
    CSS_AT_MccVersionInfoCnf *mccVersionInfoCnf                   = VOS_NULL_PTR;
    VOS_UINT8                 versionId[MCC_INFO_VERSION_LEN + 1] = {0};
    VOS_UINT32                result;
    errno_t                   memResult;
    VOS_UINT8                 indexNum = 0;

    /* 初始化消息变量 */
    mccVersionInfoCnf = (CSS_AT_MccVersionInfoCnf *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(mccVersionInfoCnf->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssMccVersionQryCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvCssMccVersionQryCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_MCCFREQ_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MCCFREQ_QRY) {
        AT_WARN_LOG("AT_RcvCssMccVersionQryCnf: WARNING:Not AT_CMD_MCCFREQ_QRY!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 构造Version String */
    memResult = memcpy_s(versionId, sizeof(versionId), mccVersionInfoCnf->versionId, MCC_INFO_VERSION_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(versionId), MCC_INFO_VERSION_LEN);

    /* 判断查询操作是否成功 */
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName,
            (VOS_CHAR *)versionId);

    result = AT_OK;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvCssCloudDataSetCnf(struct MsgCB *msg)
{
    /* 定义局部变量 */
    CSS_AT_CloudDataSetCnf *CloudInfoSetCnf = VOS_NULL_PTR;
    VOS_UINT32              result          = AT_ERROR;
    VOS_UINT32              retVal          = VOS_ERR;
    VOS_UINT8               indexNum        = 0;

    /* 初始化消息变量 */
    CloudInfoSetCnf = (CSS_AT_CloudDataSetCnf *)msg;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(CloudInfoSetCnf->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssCloudDataSetCnf: WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvCssCloudDataSetCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CLOUDDATA_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CLOUDDATA_SET) {
        AT_WARN_LOG("AT_RcvCssCloudDataSetCnf: WARNING:Not AT_CMD_CLOUDDATA_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 判断查询操作是否成功 */
    if (CloudInfoSetCnf->result == VOS_OK) {
        result = AT_OK;
        retVal = VOS_OK;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);

    return retVal;
}

VOS_UINT32 AT_RcvCssBlockCellListSetCnf(struct MsgCB *msg)
{
    /* 定义局部变量 */
    CSS_AT_BlockCellListSetCnf *cssSetCnf = VOS_NULL_PTR;
    VOS_UINT32                  result;
    VOS_UINT8                   indexNum;

    /* 初始化消息变量 */
    cssSetCnf = (CSS_AT_BlockCellListSetCnf *)msg;

    indexNum = 0;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(cssSetCnf->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssBlockCellListSetCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvCssBlockCellListSetCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CLOUDBLOCKLIST_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_BLOCKCELLLIST_SET) {
        AT_WARN_LOG("AT_RcvCssBlockCellListSetCnf: WARNING:Not AT_CMD_BLOCKCELLLIST_SET!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    result = AT_ERROR;

    if (cssSetCnf->result == VOS_OK) {
        result = AT_OK;
    }

    g_atSendDataBuff.bufLen = 0;

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, result);

    return VOS_OK;
}

VOS_UINT32 AT_RcvCssBlockCellListQryCnf(struct MsgCB *msg)
{
    CSS_AT_BlockCellListQueryCnf *cssQryCnf = VOS_NULL_PTR;
    VOS_UINT8                     versionId[AT_CSS_BLOCK_CELL_LIST_VERSION_LEN + 1];
    VOS_UINT32                    mcc;
    VOS_UINT32                    loop;
    errno_t                       memResult;
    VOS_UINT16                    length;
    VOS_UINT8                     indexNum;

    /* 初始化消息变量 */
    cssQryCnf = (CSS_AT_BlockCellListQueryCnf *)msg;

    indexNum = 0;

    /* 通过ClientId获取ucIndex */
    if (At_ClientIdToUserId(cssQryCnf->clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssBlockCellListQryCnf: WARNING:AT INDEX NOT FOUND!");

        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvCssBlockCellListQryCnf: WARNING:AT_BROADCAST_INDEX!");

        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_CLOUDBLOCKLIST_QRY */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_BLOCKCELLLIST_QRY) {
        AT_WARN_LOG("AT_RcvCssBlockCellListQryCnf: WARNING:Not AT_CMD_BLOCKCELLLIST_QRY!");

        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* MCC个数不对 */
    if (cssQryCnf->mccInfo.mccNum > AT_CSS_MAX_MCC_ID_NUM) {
        AT_WARN_LOG("AT_RcvCssBlockCellListQryCnf: WARNING:INVALID MCC NUM!");

        At_FormatResultData(indexNum, AT_ERROR);

        return VOS_ERR;
    }

    /* 构造上报给Ril的Version */
    (VOS_VOID)memset_s(versionId, sizeof(versionId), 0, sizeof(versionId));
    memResult = memcpy_s(versionId, sizeof(versionId), cssQryCnf->versionId, sizeof(cssQryCnf->versionId));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(versionId), sizeof(cssQryCnf->versionId));

    length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: \"%s\"", g_parseContext[indexNum].cmdElement->cmdName, versionId);

    /* 构造上报给Ril的MCC字符串 */
    for (loop = 0; loop < cssQryCnf->mccInfo.mccNum; loop++) {
        mcc = 0;

        AT_ConvertNasMccToBcdType(cssQryCnf->mccInfo.mcc[loop], &mcc);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, ",%x%x%x", (mcc & 0x0f00) >> 8, (mcc & 0xf0) >> 4, (mcc & 0x0f));
    }

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_RcvCssNwDeploymentSetCnf(struct MsgCB *msg)
{
    CSS_AT_PlmnDeploymentInfoSetCnf *plmnDeploymentInfoSetCnf = VOS_NULL_PTR;
    VOS_UINT32 result;
    VOS_UINT8 atIndex;
    VOS_UINT32 msgLength;

    plmnDeploymentInfoSetCnf = (CSS_AT_PlmnDeploymentInfoSetCnf *)msg;
    atIndex = 0;

    msgLength = sizeof(CSS_AT_PlmnDeploymentInfoSetCnf) - VOS_MSG_HEAD_LENGTH;

    if (msgLength != VOS_GET_MSG_LEN(plmnDeploymentInfoSetCnf)) {
        AT_ERR_LOG("AT_RcvCssNwDeploymentSetCnf: ERROR: MSG LENGTH ERROR!");
        return VOS_ERR;
    }

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(plmnDeploymentInfoSetCnf->clientId, &atIndex) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvCssNwDeploymentSetCnf: WARNING: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    /* 广播消息不处理 */
    if (AT_IsBroadcastClientIndex(atIndex)) {
        AT_WARN_LOG("AT_RcvCssNwDeploymentSetCnf: WARNING:AT_BROADCAST_INDEX!");
        return VOS_ERR;
    }

    if (g_atClientTab[atIndex].cmdCurrentOpt != AT_CMD_NWDEPLOYMENT_SET) {
        AT_WARN_LOG("AT_RcvCssNwDeploymentSetCnf: WARNING: CmdCurrentOpt != AT_CMD_NWDEPLOYMENT_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(atIndex);

    g_atSendDataBuff.bufLen = 0;

    if (plmnDeploymentInfoSetCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        result = AT_OK;
    }

    /* 输出结果 */
    At_FormatResultData(atIndex, result);

    return VOS_OK;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaIsmCoexQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg               *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_LteWifiCoexQryCnf *cnf    = (MTA_AT_LteWifiCoexQryCnf *)mtaMsg->content;
    VOS_UINT32                i;

    g_atSendDataBuff.bufLen = 0;

    g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s:", g_parseContext[indexNum].cmdElement->cmdName);

    for (i = 0; i < AT_MTA_ISMCOEX_BANDWIDTH_NUM; i++) {
        g_atSendDataBuff.bufLen +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, " %d %d %d %d %d %d,", cnf->coexPara[i].cfg,
                cnf->coexPara[i].txBegin, cnf->coexPara[i].txEnd, cnf->coexPara[i].txPower, cnf->coexPara[i].rxBegin,
                cnf->coexPara[i].rxEnd);
    }

    g_atSendDataBuff.bufLen--;

    return AT_OK;
}
#endif

/* AT处理THROT模块查询Apn参数结果的函数 */
VOS_UINT16 AT_FormatApnPara(NAS_THROT_ApnInfo apnInfo, VOS_UINT16 length)
{
    errno_t   memResult;
    VOS_UINT8 apnStr[NAS_THROT_MAX_APN_LEN + 1];

    (VOS_VOID)memset_s(apnStr, sizeof(apnStr), 0x00, sizeof(apnStr));

    /* 有apn信息，但是apn信息为空 */
    if ((apnInfo.apnLen == 1) && (apnInfo.apnName[0] == 0)) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    } else {
        /* 有apn信息，而且APN信息不为空 */
        memResult = memcpy_s(apnStr, sizeof(apnStr), apnInfo.apnName, apnInfo.apnLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(apnStr), apnInfo.apnLen);
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\",", apnStr);
    }
    return length;
}

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)

/* AT处理THROT模块查询CABTSRI功能的开启状态结果的函数 */
VOS_UINT32 AT_RcvThortCabtsrQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CabtsriQueryCnf *cabtsriQueryCnf = (THROT_AT_CabtsriQueryCnf *)msg;

    /* 查询操作失败g_atSendDataBuff.bufLen为0 */
    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (cabtsriQueryCnf->result == THROT_AT_RESULT_SUCC) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            cabtsriQueryCnf->isEnableCabtsri);
        return AT_OK;
    }

    return AT_ERROR;

}

/* 检查上报的定时器<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>,<NSLPI>,<procedure>参数取值是否正常 */
VOS_UINT32 AT_CheckbackOffPara(const THROT_AT_BackOffTimerPara *backOffPara)
{
    /* <re-attempt_rat_indicator> */
    if (backOffPara->opReAttemptRatIndicator == VOS_TRUE) {
        if (backOffPara->reAttemptRatInd > 1) {
            AT_WARN_LOG("AT_CheckbackOffPara: reAttemptRatInd value is invalid");
            return VOS_ERR;
        }
    }

    /* <re-attempt_eplmn_indicator> */
    if (backOffPara->opReAttemptEplmnIndicator == VOS_TRUE) {
        if (backOffPara->reAttemptEplmnInd > 1) {
            AT_WARN_LOG("AT_CheckbackOffPara: reAttemptEplmnInd value is invalid");
            return VOS_ERR;
        }
    }

    /* <NSLPI> */
    if (backOffPara->opNslpi == VOS_TRUE) {
        if (backOffPara->nslpi > 1) {
            AT_WARN_LOG("AT_CheckbackOffPara: nslpi value is invalid");
            return VOS_ERR;
        }
    }

    /* <procedure> */
    if (backOffPara->opProcedure == VOS_TRUE) {
        if (backOffPara->sessionProcedure > THROT_AT_SESSION_PDU_ESTABLISH_MODIFY_PROC) {
            AT_WARN_LOG("AT_CheckbackOffPara: sessionProcedure value is invalid");
            return VOS_ERR;
        }
    }
    return VOS_OK;
}

/* AT处理THROT模块查询定时器<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>,<NSLPI>,<procedure>参数的结果函数 */
VOS_UINT16 AT_ParseBackoffTimerOtherPara(const THROT_AT_BackOffTimerPara *backOffPara, VOS_UINT16 length)
{
    /* <re-attempt_rat_indicator> */
    if (backOffPara->opReAttemptRatIndicator == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara->reAttemptRatInd);
    } else {
         length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
             (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* <re-attempt_eplmn_indicator> */
    if (backOffPara->opReAttemptEplmnIndicator == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara->reAttemptEplmnInd);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* <NSLPI> */
    if (backOffPara->opNslpi == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara->nslpi);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* <procedure> */
    if (backOffPara->opProcedure == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", backOffPara->sessionProcedure);
    }
    return length;
}

/* AT处理THROT模块查询定时器backOffTimerLen参数的结果函数 */
LOCAL VOS_UINT16 AT_ParseCabtrdpbackOffTimerPara(const THROT_AT_BackOffTimerPara *backOffPara, VOS_UINT16 length)
{
    /* <residual_backoff_time> */
    if (backOffPara->opBackOffTimerLen == VOS_TRUE) {
        if (backOffPara->isTimerDeactive == VOS_FALSE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara->backOffTimerLen);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", 0);
    }
    length = AT_ParseBackoffTimerOtherPara(backOffPara, length);

    return length;
}

/* 填充at下发查询命令后，throt模块回复的结果 */
LOCAL VOS_UINT16 AT_FillCabtrdpCnfResult(const THROT_AT_CabtrdpQueryCnf *cabtrdpSetCnf, VOS_UINT8 indexNum)
{
    VOS_UINT32 i;
    VOS_UINT16 length = 0;

    /* backOffParaList信息 */
    for (i = 0; i < cabtrdpSetCnf->backOffParaNum; i++) {
        if (i != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
        /* +CABTRDP:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <apn> */
        length = AT_FormatApnPara(cabtrdpSetCnf->backOffParaList[i].apnInfo, length);

        length = AT_ParseCabtrdpbackOffTimerPara(&(cabtrdpSetCnf->backOffParaList[i].backOffPara), length);
    }
    return length;
}

/* AT处理THROT模块查询apn backoff定时器信息的函数 */
VOS_UINT32 AT_RcvThortCabtrdpSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CabtrdpQueryCnf *cabtrdpSetCnf = (THROT_AT_CabtrdpQueryCnf *)msg;
    VOS_UINT32                i;
    VOS_UINT8                 apnLen;

    /* 参数个数不正确，直接返回错误 */
    if (cabtrdpSetCnf->backOffParaNum > MAX_CABTRDP_PARA_NUM) {
        AT_WARN_LOG("AT_RcvThortCabtrdpSetCnf: cabtrdp number is invalid!");
        return AT_ERROR;
    }

    for (i = 0; i < cabtrdpSetCnf->backOffParaNum; i++) {
        /* apnlen为0或者大于最大长度,直接返回错误 */
        apnLen = cabtrdpSetCnf->backOffParaList[i].apnInfo.apnLen;
        if ((apnLen == 0) || (apnLen > NAS_THROT_AT_MAX_APN_LEN)) {
            AT_WARN_LOG("AT_RcvThortCabtrdpSetCnf: apn number is invalid");
            return AT_ERROR;
        }
        /* backOff参数不正确，直接返回错误 */
        if (AT_CheckbackOffPara(&(cabtrdpSetCnf->backOffParaList[i].backOffPara)) != VOS_OK) {
            AT_ERR_LOG("AT_RcvThortCabtrdpSetCnf:AT_CheckbackOffPara Check backOff para error!.");
            return AT_ERROR;
        }
    }

    /* 查询操作失败g_atSendDataBuff.bufLen为0 */
    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (cabtrdpSetCnf->result == THROT_AT_RESULT_SUCC) {
        /* 填充回复上报的信息 */
        g_atSendDataBuff.bufLen = AT_FillCabtrdpCnfResult(cabtrdpSetCnf, indexNum);

        /* backOffParaNum = 0时，无APN，直接返回ok */
        return  AT_OK;
    }
    return AT_ERROR;
}
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
/* AT处理THROT模块查询CSBTSRI功能的开启状态结果的函数 */
VOS_UINT32 AT_RcvThortCsbtsrQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CsbtsriQueryCnf *csbtsriQueryCnf = (THROT_AT_CsbtsriQueryCnf *)msg;

    /* 查询操作失败g_atSendDataBuff.bufLen为0 */
    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (csbtsriQueryCnf->result == THROT_AT_RESULT_SUCC) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            csbtsriQueryCnf->isEnableCsbtsri);
        return AT_OK;
    }
    return AT_ERROR;
}

/* 检查上报的定时器<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>,<procedure>参数取值是否正常 */
VOS_UINT32 AT_CheckSnssaiDnnbackOffPara(const THROT_AT_SnssaiBasedBackOffTimerPara *backOffPara)
{
    /* <re-attempt_rat_indicator> */
    if (backOffPara->opReAttemptRatIndicator == VOS_TRUE) {
        if (backOffPara->reAttemptRatInd > 1) {
            return VOS_ERR;
        }
    }

    /* <re-attempt_eplmn_indicator> */
    if (backOffPara->opReAttemptEplmnIndicator == VOS_TRUE) {
        if (backOffPara->reAttemptEplmnInd > 1) {
            return VOS_ERR;
        }
    }

    /* <procedure> */
    if (backOffPara->opProcedure == VOS_TRUE) {
        if (backOffPara->sessionProcedure >= THROT_AT_SNSSAI_SESSION_BUTT) {
            return VOS_ERR;
        }
    }
    return VOS_OK;
}
LOCAL VOS_VOID AT_ConvertSnssaiToString(const NAS_THROT_Snssai *snssai, VOS_UINT16 *length)
{
    /*
     * sst                          only slice/service type (SST) is present
     * sst;mappedSst                SST and mapped configured SST are present
     * sst.sd                       SST and slice differentiator (SD) are present
     * sst.sd;mappedSst             SST, SD and mapped configured SST are present
     * sst.sd;mappedSst.mappedSd    SST, SD, mapped configured SST and mapped configured SD are present
     */

    if ((snssai->opSd == VOS_TRUE) && (snssai->opMappedSst == VOS_TRUE) && (snssai->opMappedSd == VOS_TRUE)) {
        /* sst.sd;mappedSst.mappedSd */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%02x.%06x;%02x.%06x\",", snssai->sst, snssai->sd,
            snssai->mappedSst, snssai->mappedSd);
    } else if ((snssai->opSd == VOS_TRUE) && (snssai->opMappedSst == VOS_TRUE)) {
        /* sst.sd;mappedSst */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%02x.%06x;%02x\",", snssai->sst, snssai->sd,
            snssai->mappedSst);
    } else if (snssai->opSd == VOS_TRUE) {
        /* sst.sd */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%02x.%06x\",", snssai->sst, snssai->sd);
    } else if (snssai->opMappedSst == VOS_TRUE) {
        /* sst;mappedSst */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%02x;%02x\",", snssai->sst, snssai->mappedSst);
    } else {
        /* sst */
        *length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + *length, "\"%02x\",", snssai->sst);
    }
}

/* AT处理THROT模块查询S-NSSAI参数结果的函数 */
VOS_VOID AT_ParseSnssaiPara(const NAS_THROT_SnssaiInfo *snssaiInfo, VOS_UINT16 *length)
{
    /* 有S-NSSAI信息，但是S-NSSAI信息为空 */
    if (snssaiInfo->isEmptyNssai == VOS_TRUE) {
        (*length) += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + (*length), ",");
    } else {
        /* 有S-NSSAI信息，而且S-NSSAI信息不为空 */
        AT_ConvertSnssaiToString(&(snssaiInfo->nssai), length);
    }
}

/* AT处理THROT模块查询定时器<re-attempt_rat_indicator>,<re-attempt_eplmn_indicator>,<procedure>参数的结果函数 */
VOS_UINT16 AT_ParseSnssaiDnnBackoffTimerOtherPara(const THROT_AT_SnssaiBasedBackOffTimerPara *backOffPara,
    VOS_UINT16 length)
{
    /* <re-attempt_rat_indicator> */
    if (backOffPara->opReAttemptRatIndicator == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara->reAttemptRatInd);
    } else {
         length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
             (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* <re-attempt_eplmn_indicator> */
    if (backOffPara->opReAttemptEplmnIndicator == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara->reAttemptEplmnInd);
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* <procedure> */
    if (backOffPara->opProcedure == VOS_TRUE) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d", backOffPara->sessionProcedure);
    }
    return length;
}

/* AT处理THROT模块查询定时器backOffTimerLen参数的结果函数 */
LOCAL VOS_UINT16 AT_ParseSnssaiDnnbackOffTimerPara(THROT_AT_SnssaiBasedBackOffTimerPara backOffPara, VOS_UINT16 length)
{
    /* 定时器剩余时长 */
    if (backOffPara.opBackOffTimerLen == VOS_TRUE) {
        if (backOffPara.isTimerDeactive == VOS_FALSE) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", backOffPara.backOffTimerLen);
        } else {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",");
        }
    } else {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", 0);
    }
    length = AT_ParseSnssaiDnnBackoffTimerOtherPara(&backOffPara, length);

    return length;
}

/* 填充at下发at+Csbtrdp?查询命令后，throt模块回复的结果 */
LOCAL VOS_UINT16 AT_FillCsbtrdpCnfResult(const THROT_AT_CsbtrdpQueryCnf *csbtrdpSetCnf, VOS_UINT8 indexNum)
{
    THROT_AT_CsbtrdpBackOffTimerPara backOffParaList;
    VOS_UINT32                       i;
    VOS_UINT16                       length = 0;

    /* backOffParaList信息 */
    for (i = 0; i < csbtrdpSetCnf->backOffParaNum; i++) {
        backOffParaList = csbtrdpSetCnf->backOffParaList[i];
        if (i != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
        /* +CSBTRDP:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <S-NSSAI> */
        AT_ParseSnssaiPara(&(backOffParaList.snssaiInfo), &length);

        length = AT_ParseSnssaiDnnbackOffTimerPara(backOffParaList.backOffPara, length);
    }
    return length;
}

/* AT处理THROT模块查询基于S-NSSAI的back-off定时器信息的函数 */
VOS_UINT32 AT_RcvThortCsbtrdpSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CsbtrdpQueryCnf *csbtrdpSetCnf = (THROT_AT_CsbtrdpQueryCnf *)msg;
    VOS_UINT32                i;

    /* 参数个数不正确，直接返回错误 */
    if (csbtrdpSetCnf->backOffParaNum > MAX_CSBTRDP_PARA_NUM) {
        AT_ERR_LOG("AT_RcvThortCsbtrdpSetCnf:backOffParaNum error!.");
        return AT_ERROR;
    }

    for (i = 0; i < csbtrdpSetCnf->backOffParaNum; i++) {
        /* backOff参数不正确，直接返回错误 */
        if (AT_CheckSnssaiDnnbackOffPara(&(csbtrdpSetCnf->backOffParaList[i].backOffPara)) != VOS_OK) {
            AT_ERR_LOG("AT_RcvThortCsbtrdpSetCnf:AT_CheckSnssaiDnnbackOffPara error!.");
            return AT_ERROR;
        }
    }

    /* 查询操作失败g_atSendDataBuff.bufLen为0 */
    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (csbtrdpSetCnf->result == THROT_AT_RESULT_SUCC) {
        /* 填充回复上报的信息 */
        g_atSendDataBuff.bufLen = AT_FillCsbtrdpCnfResult(csbtrdpSetCnf, indexNum);

        /* backOffParaListNum = 0时，无APN，直接返回ok */
        return AT_OK;
    }
    return AT_ERROR;
}

/* AT处理THROT模块查询CSDBTSRI功能的开启状态结果的函数 */
VOS_UINT32 AT_RcvThortCsdbtsrQueryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CsdbtsriQueryCnf *csdbtsriQueryCnf = (THROT_AT_CsdbtsriQueryCnf *)msg;

    /* 查询操作失败g_atSendDataBuff.bufLen为0 */
    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (csdbtsriQueryCnf->result == THROT_AT_RESULT_SUCC) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            csdbtsriQueryCnf->isEnableCsdbtsri);
        return AT_OK;
    }
    return AT_ERROR;
}

/* 填充at下发at+Csdbtrdp?查询命令后，throt模块回复的结果 */
LOCAL VOS_UINT16 AT_FillCsdbtrdpCnfResult(const THROT_AT_CsdbtrdpQueryCnf *csdbtrdpSetCnf, VOS_UINT8 indexNum)
{
    THROT_AT_CsdbtrdpBackOffTimerPara backOffParaList;
    VOS_UINT32                        i;
    VOS_UINT16                        length = 0;

    /* backOffParaList信息 */
    for (i = 0; i < csdbtrdpSetCnf->backOffParaNum; i++) {
        backOffParaList = csdbtrdpSetCnf->backOffParaList[i];
        if (i != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
        }
        /* +CSDBTRDP:  */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

        /* <S-NSSAI> */
        AT_ParseSnssaiPara(&(backOffParaList.snssaiInfo), &length);

        /* <DNN> */
        length = AT_FormatApnPara(backOffParaList.apnInfo, length);

        length = AT_ParseSnssaiDnnbackOffTimerPara(backOffParaList.backOffPara, length);
    }
    return length;
}

/* AT处理THROT模块查询基于S-NSSAI和DNN的back-off定时器信息的函数 */
VOS_UINT32 AT_RcvThortCsdbtrdpSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    THROT_AT_CsdbtrdpQueryCnf *csdbtrdpSetCnf = (THROT_AT_CsdbtrdpQueryCnf *)msg;
    VOS_UINT32                 i;
    VOS_UINT8                  dnnLen;

    /* 参数个数不正确，直接返回错误 */
    if (csdbtrdpSetCnf->backOffParaNum > MAX_CSDBTRDP_PARA_NUM) {
        return AT_ERROR;
    }
    for (i = 0; i < csdbtrdpSetCnf->backOffParaNum; i++) {
        /* DNN长度为0或者大于最大长度,直接返回错误 */
        dnnLen = csdbtrdpSetCnf->backOffParaList[i].apnInfo.apnLen;
        if ((dnnLen == 0) || (dnnLen > NAS_THROT_AT_MAX_APN_LEN)) {
            AT_ERR_LOG("AT_RcvThortCsdbtrdpSetCnf:dnnLen error!.");
            return AT_ERROR;
        }
        /* backOff参数不正确，直接返回错误 */
        if (AT_CheckSnssaiDnnbackOffPara(&(csdbtrdpSetCnf->backOffParaList[i].backOffPara)) != VOS_OK) {
            AT_ERR_LOG("AT_RcvThortCsdbtrdpSetCnf:AT_CheckSnssaiDnnbackOffPara error!.");
            return AT_ERROR;
        }
    }

    /* 查询操作失败g_atSendDataBuff.bufLen为0 */
    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (csdbtrdpSetCnf->result == THROT_AT_RESULT_SUCC) {
        /* 填充回复上报的信息 */
        g_atSendDataBuff.bufLen = AT_FillCsdbtrdpCnfResult(csdbtrdpSetCnf, indexNum);

        /* backOffParaListNum = 0时，无APN，直接返回ok */
        return AT_OK;
    }

    return AT_ERROR;
}
#endif

