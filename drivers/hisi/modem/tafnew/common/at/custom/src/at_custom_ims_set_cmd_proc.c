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
#include "at_custom_ims_set_cmd_proc.h"
#include "securec.h"

#include "AtParse.h"
#include "at_cmd_proc.h"

#include "taf_ccm_api.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_IMS_SET_CMD_PROC_C

#define AT_USERAGENTCFG_PARA_NUM 6
#define AT_USERAGENTCFG_PARA1 0
#define AT_USERAGENTCFG_PARA2 1
#define AT_USERAGENTCFG_PARA3 2
#define AT_USERAGENTCFG_PARA4 3
#define AT_USERAGENTCFG_PARA5 4
#define AT_USERAGENTCFG_PARA6 5
#define AT_IMSSMSCFG_PARA_NUM 4
#define AT_IMSSMSCFG_UTRAN_ENABLE 2
#define AT_IMSSMSCFG_GSM_ENABLE 3
#define AT_DMRCSCFG_PARA_NUM 2
#define AT_DMRCSCFG_FEATURETAG 0
#define AT_DMRCSCFG_DEVCFG 1

#define AT_FUSIONCALL_RAW_PARA_NUM 3
#define AT_FUSIONCALL_RAW_MSG_ID_INDEX 0
#define AT_FUSIONCALL_RAW_MSG_LEN_INDEX 1
#define AT_FUSIONCALL_RAW_MSG_DATA_INDEX 2
#define AT_ECLIMSCFG_MAX_PARA_NUM 2
#define AT_ECLIMSCFG_MODE_INDEX 0
#define AT_ECLIMSCFG_CONTENT_INDEX 1

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_SetImsSmsCfgPara(VOS_UINT8 indexNum)
{
    VOS_UINT32        rst;
    TAF_MMA_ImsSmsCfg imsSmsCfg;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数数量错误 */
    if (g_atParaIndex != AT_IMSSMSCFG_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0) ||
        (g_atParaList[AT_IMSSMSCFG_UTRAN_ENABLE].paraLen == 0) ||
        (g_atParaList[AT_IMSSMSCFG_GSM_ENABLE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&imsSmsCfg, sizeof(imsSmsCfg), 0, sizeof(imsSmsCfg));

    imsSmsCfg.wifiEnable  = (g_atParaList[0].paraValue == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
    imsSmsCfg.lteEnable   = (g_atParaList[1].paraValue == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
    imsSmsCfg.utranEnable = (g_atParaList[AT_IMSSMSCFG_UTRAN_ENABLE].paraValue == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;
    imsSmsCfg.gsmEnable   = (g_atParaList[AT_IMSSMSCFG_GSM_ENABLE].paraValue == VOS_TRUE) ? VOS_TRUE : VOS_FALSE;

    /* 执行命令操作 */
    rst = TAF_MMA_SetImsSmsCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &imsSmsCfg);

    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSSMSCFG_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetImsRegErrRpt(TAF_UINT8 indexNum)
{
    VOS_UINT32 rst;
    VOS_UINT8  reportFlag;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 当前只能使能或不使能ims注册错误原因值上报，故使用VOS_TRUE和VOS_FALSE标示 */
    reportFlag = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 给IMSA发送^IMSREGERRRPT设置请求 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_IMSA_REGERR_REPORT_SET_REQ, &(reportFlag), (VOS_UINT32)sizeof(reportFlag),
                                 PS_PID_IMSA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMSREGERRRPT_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetImsSrvStatRptCfgPara(TAF_UINT8 indexNum)
{
    AT_IMSA_ImsSrvStatReportSetReq imsSrvStatRptSetReq;
    VOS_UINT32                     result;

    (VOS_VOID)memset_s(&imsSrvStatRptSetReq, sizeof(imsSrvStatRptSetReq), 0x00, sizeof(imsSrvStatRptSetReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetImsSrvStatRpt: NOT AT_CMD_OPT_SET_PARA_CMD!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不正确 */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetImsSrvStatRpt: para num is not equal 1!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetImsSrvStatRpt: para len is error!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    imsSrvStatRptSetReq.imsSrvStatRpt = (AT_IMSA_ImsSrvStatReportUint32)g_atParaList[0].paraValue;

    /* 给IMSA发送^IMSSRVSTATRPT设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_IMS_SRV_STAT_RPT_SET_REQ,
                                    (VOS_UINT8 *)&(imsSrvStatRptSetReq.imsSrvStatRpt),
                                    (VOS_UINT32)sizeof(AT_IMSA_ImsSrvStatReportUint32), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetImsSrvStatRpt: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_IMS_SRV_STAT_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetEmcAIdPara(VOS_UINT8 indexNum)
{
    AT_IMSA_EmergencyAidInfo emcAIdInfo;
    VOS_UINT32               result;
    errno_t                  memResult;

    (VOS_VOID)memset_s(&emcAIdInfo, sizeof(emcAIdInfo), 0x00, sizeof(emcAIdInfo));

    /* 参数类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数错误 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数检查 */
    if (g_atParaList[0].paraLen > AT_IMSA_MAX_EMERGENCY_AID_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    emcAIdInfo.addressIdLen = g_atParaList[0].paraLen;

    if (emcAIdInfo.addressIdLen != 0) {
        memResult = memcpy_s(emcAIdInfo.addressId, sizeof(emcAIdInfo.addressId), g_atParaList[0].para,
                             g_atParaList[0].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(emcAIdInfo.addressId), g_atParaList[0].paraLen);
    }

    /* 给IMSA发送^WIEMCAID设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_EMERGENCY_AID_SET_REQ,
                                    (VOS_UINT8 *)&emcAIdInfo, (VOS_UINT32)sizeof(AT_IMSA_EmergencyAidInfo),
                                    PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetImsSrvStatRpt: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_WIEMCAID_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetDmRcsCfgPara(VOS_UINT8 indexNum)
{
    AT_IMSA_DmRcsCfgSetPara dmRcsCfgPara;
    VOS_UINT32              result;

    /* 清空 */
    (VOS_VOID)memset_s(&dmRcsCfgPara, sizeof(AT_IMSA_DmRcsCfgSetPara), 0x00, sizeof(AT_IMSA_DmRcsCfgSetPara));

    /* 不是设置命令则出错 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_SetDmRcsCfgPara():WARNING:ucCmdOptType is not AT_CMD_OPT_SET_PARA_CMD");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 判断参数是否有效 */
    if ((g_atParaIndex != AT_DMRCSCFG_PARA_NUM) || (g_atParaList[AT_DMRCSCFG_FEATURETAG].paraLen == 0) ||
        (g_atParaList[AT_DMRCSCFG_DEVCFG].paraLen == 0)) {
        AT_ERR_LOG("AT_SetDmRcsCfgPara: incorrect parameter, return ERROR");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    dmRcsCfgPara.featureTag = g_atParaList[AT_DMRCSCFG_FEATURETAG].paraValue;
    dmRcsCfgPara.devCfg     = g_atParaList[AT_DMRCSCFG_DEVCFG].paraValue;

    /* 给IMSA发送设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_DM_RCS_CFG_SET_REQ, (VOS_UINT8 *)&dmRcsCfgPara,
                                    (VOS_UINT32)sizeof(AT_IMSA_DmRcsCfgSetPara), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetDmRcsCfgPara():WARNING:AT_FillAndSndAppReqMsg fail");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DMRCSCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_VOID AT_ProcUserAgentPara(AT_IMSA_UserAgentCfg *userAgentCfg)
{
    errno_t memResult;

    /* 给参数赋值 */
    if (g_atParaList[0].paraLen != 0) {
        memResult = memcpy_s(userAgentCfg->para1, sizeof(userAgentCfg->para1), g_atParaList[0].para,
                             g_atParaList[0].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(userAgentCfg->para1), g_atParaList[0].paraLen);
    }

    if (g_atParaList[1].paraLen != 0) {
        memResult = memcpy_s(userAgentCfg->para2, sizeof(userAgentCfg->para2), g_atParaList[1].para,
                             g_atParaList[1].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(userAgentCfg->para2), g_atParaList[1].paraLen);
    }

    if (g_atParaList[AT_USERAGENTCFG_PARA3].paraLen != 0) {
        memResult = memcpy_s(userAgentCfg->para3, sizeof(userAgentCfg->para3),
                             g_atParaList[AT_USERAGENTCFG_PARA3].para,
                             g_atParaList[AT_USERAGENTCFG_PARA3].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(userAgentCfg->para3), g_atParaList[AT_USERAGENTCFG_PARA3].paraLen);
    }

    if (g_atParaList[AT_USERAGENTCFG_PARA4].paraLen != 0) {
        memResult = memcpy_s(userAgentCfg->para4, sizeof(userAgentCfg->para4),
                             g_atParaList[AT_USERAGENTCFG_PARA4].para,
                             g_atParaList[AT_USERAGENTCFG_PARA4].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(userAgentCfg->para4), g_atParaList[AT_USERAGENTCFG_PARA4].paraLen);
    }

    if (g_atParaList[AT_USERAGENTCFG_PARA5].paraLen != 0) {
        memResult = memcpy_s(userAgentCfg->para5, sizeof(userAgentCfg->para5),
                             g_atParaList[AT_USERAGENTCFG_PARA5].para,
                             g_atParaList[AT_USERAGENTCFG_PARA5].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(userAgentCfg->para5), g_atParaList[AT_USERAGENTCFG_PARA5].paraLen);
    }

    if (g_atParaList[AT_USERAGENTCFG_PARA6].paraLen != 0) {
        memResult = memcpy_s(userAgentCfg->para6, sizeof(userAgentCfg->para6),
                             g_atParaList[AT_USERAGENTCFG_PARA6].para,
                             g_atParaList[AT_USERAGENTCFG_PARA6].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(userAgentCfg->para6), g_atParaList[AT_USERAGENTCFG_PARA6].paraLen);
    }
}

VOS_UINT32 AT_SetUserAgentCfgPara(VOS_UINT8 indexNum)
{
    AT_IMSA_UserAgentCfg userAgentCfg;
    VOS_UINT32           result;

    (VOS_VOID)memset_s(&userAgentCfg, sizeof(userAgentCfg), 0x00, sizeof(userAgentCfg));

    /* 参数类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数错误 */
    if (g_atParaIndex != AT_USERAGENTCFG_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度错误 */
    if ((g_atParaList[AT_USERAGENTCFG_PARA1].paraLen > AT_IMSA_USER_AGENT_STR_LEN) ||
        (g_atParaList[AT_USERAGENTCFG_PARA2].paraLen > AT_IMSA_USER_AGENT_STR_LEN) ||
        (g_atParaList[AT_USERAGENTCFG_PARA3].paraLen > AT_IMSA_USER_AGENT_STR_LEN) ||
        (g_atParaList[AT_USERAGENTCFG_PARA4].paraLen > AT_IMSA_USER_AGENT_STR_LEN) ||
        (g_atParaList[AT_USERAGENTCFG_PARA5].paraLen > AT_IMSA_USER_AGENT_STR_LEN) ||
        (g_atParaList[AT_USERAGENTCFG_PARA6].paraLen > AT_IMSA_USER_AGENT_STR_LEN)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给参数赋值 */
    AT_ProcUserAgentPara(&userAgentCfg);

    /* 给IMSA发送^USERAGENTCFG设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_USER_AGENT_CFG_SET_REQ, (VOS_UINT8 *)&userAgentCfg,
                                    (VOS_UINT32)sizeof(AT_IMSA_UserAgentCfg), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetUserAgentCfgPara: AT_FillAndSndAppReqMsg is error!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_USERAGENTCFG_SET;

    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_SetSipTransPort(TAF_UINT8 indexNum)
{
    AT_IMSA_TransportTypeSetReq sipTransPort;
    VOS_UINT32                  result;

    (VOS_VOID)memset_s(&sipTransPort, sizeof(sipTransPort), 0x00, sizeof(sipTransPort));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    sipTransPort.tcpThreshold     = g_atParaList[0].paraValue;
    sipTransPort.appCtrl.clientId = g_atClientTab[indexNum].clientId;
    sipTransPort.appCtrl.opId     = g_atClientTab[indexNum].opId;

    /* 给IMSA发送+SIPPORT设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_IMSA_TRANSPORT_TYPE_SET_REQ, (VOS_UINT8 *)&(sipTransPort.tcpThreshold),
                                    sizeof(sipTransPort.tcpThreshold), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SIPPORT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SndImsaCallRawDataMsg(VOS_UINT16 clientId, VOS_UINT8 opId,
    VOS_UINT8 dstId, VOS_UINT32 dataLen, VOS_UINT8 *data)
{
    AT_IMSA_FusionCallCtrlMsg *sndMsg = VOS_NULL_PTR;
    VOS_UINT32          msgLen;
    VOS_UINT32          ret;
    errno_t             memResult;
    ModemIdUint16       modemId = MODEM_ID_0;

    /* 获取client id对应的Modem Id */
    ret = AT_GetModemIdFromClient(clientId, &modemId);

    if (ret == VOS_ERR) {
        AT_ERR_LOG("AT_SndImsaCallRawDataMsg:AT_GetModemIdFromClient is error");
        return VOS_ERR;
    }

#if (FEATURE_MODEM1_SUPPORT_LTE == FEATURE_ON)
    if (modemId == MODEM_ID_2)
#else
    if ((modemId == MODEM_ID_1) || (modemId == MODEM_ID_2))
#endif
    {
        AT_ERR_LOG("AT_SndImsaCallRawDataMsg: enModemId is not support ims");
        return VOS_ERR;
    }

    msgLen = sizeof(AT_IMSA_FusionCallCtrlMsg) - AT_DATA_DEFAULT_LENGTH + dataLen;

    /* 申请内存  */
    sndMsg = (AT_IMSA_FusionCallCtrlMsg *)TAF_AllocMsgWithoutHeaderLen(WUEPS_PID_AT, msgLen - VOS_MSG_HEAD_LENGTH);

    if (sndMsg == VOS_NULL_PTR) {
        /* 内存申请失败 */
        AT_ERR_LOG("AT_SndImsaCallRawDataMsg:ERROR: Memory Alloc Error for pstMsg");
        return VOS_ERR;
    }

    /* 填写相关参数 */
    TAF_CfgMsgHdr((MsgBlock *)sndMsg, WUEPS_PID_AT, AT_GetDestPid(clientId, PS_PID_IMSA), msgLen - VOS_MSG_HEAD_LENGTH);

    sndMsg->msgId            = ID_AT_IMSA_FUSIONCALL_CTRL_MSG;
    sndMsg->appCtrl.clientId = clientId;
    sndMsg->appCtrl.opId     = opId;
    sndMsg->dstId            = dstId;
    sndMsg->msgLen           = dataLen;

    if (dataLen > 0) {
        memResult = memcpy_s(sndMsg->msgData, dataLen, data, dataLen);
        TAF_MEM_CHK_RTN_VAL(memResult, dataLen, dataLen);
    }

    /* 调用VOS发送原语 */
    return TAF_TraceAndSendMsg(WUEPS_PID_AT, sndMsg);
}

LOCAL VOS_UINT32 AT_CheckFusionCallRawPara(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParaIndex < AT_FUSIONCALL_RAW_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[AT_FUSIONCALL_RAW_MSG_ID_INDEX].paraLen == 0) ||
        (g_atParaList[AT_FUSIONCALL_RAW_MSG_LEN_INDEX].paraLen == 0) ||
        (g_atParaList[AT_FUSIONCALL_RAW_MSG_DATA_INDEX].paraLen == 0)){
        return AT_CME_INCORRECT_PARAMETERS;
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_SetFusionCallRawPara(VOS_UINT8 indexNum)
{
    VOS_UINT8                *tmpData = VOS_NULL_PTR;
    VOS_UINT32                result;
    VOS_UINT16                msgStrLen = 0;
    VOS_UINT16                length = 0;

    /* 参数检查 */
    result = AT_CheckFusionCallRawPara();
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetFusionCallRawPara:check para error.");

        return result;
    }

    msgStrLen = g_atParaList[AT_FUSIONCALL_RAW_MSG_DATA_INDEX].paraLen;

    tmpData = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, msgStrLen + 1);

    if (tmpData == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SetFusionCallRawPara: Memory malloc failed!");
        return AT_ERROR;
    }

    (VOS_VOID)memset_s(tmpData, msgStrLen + 1, 0x00, msgStrLen + 1);
    length += (VOS_UINT16)AT_FormatReportString(msgStrLen + 1, (VOS_CHAR *)tmpData, (VOS_CHAR *)tmpData, "%s",
                                                g_atParaList[AT_FUSIONCALL_RAW_MSG_DATA_INDEX].para);

    /* 将字符串参数转换为码流 */
    result = At_AsciiNum2HexString(tmpData, &length);

    if ((result != AT_SUCCESS) || (g_atParaList[AT_FUSIONCALL_RAW_MSG_LEN_INDEX].paraValue != length)) {
        PS_MEM_FREE(WUEPS_PID_AT, tmpData);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 转发FUSIONCALL给IMSA的消息 */
    result = AT_SndImsaCallRawDataMsg(g_atClientTab[indexNum].clientId, 0,
                                      (VOS_UINT8)g_atParaList[AT_FUSIONCALL_RAW_MSG_ID_INDEX].paraValue,
                                      g_atParaList[AT_FUSIONCALL_RAW_MSG_LEN_INDEX].paraValue,
                                      tmpData);

    PS_MEM_FREE(WUEPS_PID_AT, tmpData);
    if (result != VOS_OK) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetVolteImpiPara(TAF_UINT8 indexNum)
{
    AT_IMSA_VolteimpiQryReq impi;
    VOS_UINT32              result;

    (VOS_VOID)memset_s(&impi, sizeof(impi), 0x00, sizeof(impi));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送^VOLTEIMPI设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_VOLTEIMPI_QRY_REQ, impi.content,
                                    sizeof(impi.content), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VOLTEIMPI_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetVolteDomainPara(TAF_UINT8 indexNum)
{
    AT_IMSA_VoltedomainQryReq domain;
    VOS_UINT32                result;

    (VOS_VOID)memset_s(&domain, sizeof(domain), 0x00, sizeof(domain));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 给IMSA发送^VOLTEDOMAIN设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_IMSA_VOLTEDOMAIN_QRY_REQ, domain.content,
                                    sizeof(domain.content), PS_PID_IMSA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_VOLTEDOMAIN_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetImsUrsp(VOS_UINT8 clientId)
{
    AT_ModemUePolicyCtx  *uePolicyCtx = VOS_NULL_PTR;
    AT_IMSA_ImsUrspSetReq urspSetReq;
    VOS_UINT32            rst;

    /* 参数个数不合法 */
    if (g_atParaIndex != 2) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    uePolicyCtx = AT_GetModemUePolicyCtxAddrFromClientId(clientId);

    (VOS_VOID)memset_s(uePolicyCtx, sizeof(AT_ModemUePolicyCtx), 0x00, sizeof(AT_ModemUePolicyCtx));
    (VOS_VOID)memset_s(&urspSetReq, sizeof(AT_IMSA_ImsUrspSetReq), 0x00, sizeof(AT_IMSA_ImsUrspSetReq));

    uePolicyCtx->imsUrspVer = (VOS_UINT8)g_atParaList[0].paraValue;

    if (g_atParaList[1].paraValue != 0) {
        if ((g_atParaList[1].paraValue % 2) != 0) {
            AT_ERR_LOG("AT_SetImsUrsp: length is invalid!");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 为准备输入的IMS URSP码流申请内存 */
        uePolicyCtx->imsUrspHead = (VOS_UINT8 *)PS_MEM_ALLOC(WUEPS_PID_AT, g_atParaList[1].paraValue);
        if (uePolicyCtx->imsUrspHead == VOS_NULL_PTR) {
            AT_ERR_LOG("AT_SetImsUrsp: Memory malloc failed!");
            return AT_ERROR;
        }

        uePolicyCtx->imsUrspCur = uePolicyCtx->imsUrspHead;
        uePolicyCtx->imsUrspLen = g_atParaList[1].paraValue;
        (VOS_VOID)memset_s(uePolicyCtx->imsUrspHead, g_atParaList[1].paraValue, 0x00, g_atParaList[1].paraValue);

        /* 切换为UE POLICY码流输入模式 */
        At_SetCmdSubMode(clientId, AT_IMS_URSP_MODE);

        return AT_WAIT_IMSA_URSP_INPUT;
    }

    urspSetReq.version = (VOS_UINT16)uePolicyCtx->imsUrspVer;

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[clientId].clientId, g_atClientTab[clientId].opId,
              ID_AT_IMSA_IMS_URSP_SET_REQ, (VOS_UINT8 *)(&urspSetReq.version),
              (VOS_UINT32)(sizeof(urspSetReq.version) + sizeof(urspSetReq.length) + urspSetReq.length), PS_PID_IMSA);
    if (rst != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetImsUrsp: AT_FillAndSndAppReqMsg FAIL");
        return AT_ERROR;
    }

    g_atClientTab[clientId].cmdCurrentOpt = AT_CMD_IMSURSP_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}
#endif

#if (FEATURE_ECALL == FEATURE_ON)
VOS_UINT32 AT_CheckEclImsCfgPara(VOS_VOID)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_ERR_LOG("AT_CheckEclImsCfgPara: cmdOptType Error!");
        return AT_ERROR;
    }

    /* 参数个数不能超过2，且第一个参数不能为空 */
    if ((g_atParaIndex > AT_ECLIMSCFG_MAX_PARA_NUM) || (g_atParaList[AT_ECLIMSCFG_MODE_INDEX].paraLen == 0)) {
        AT_ERR_LOG("AT_CheckEclImsCfgPara: num Error!");
        return AT_ERROR;
    }

    /* ecallContentTypeMode 的值只支持设置为0-2 即cs_prefer 和 ps_prefer_custom 和 ps_perfer_standard */
    if (g_atParaList[AT_ECLIMSCFG_MODE_INDEX].paraValue >= AT_IMSA_ECALL_CONTENT_TYPE_MODE_BUTT) {
        AT_ERR_LOG("AT_CheckEclImsCfgPara: value Error!");
        return AT_ERROR;
    }

    /* 参数1 ecallContentTypeMode为1(ps_prefer_custom)时，参数2 content type 不能为空 */
    if ((g_atParaList[AT_ECLIMSCFG_MODE_INDEX].paraValue == AT_IMSA_ECALL_CONTENT_TYPE_MODE_PS_PREFER_CUSTOM) &&
        (g_atParaList[AT_ECLIMSCFG_CONTENT_INDEX].paraLen == 0)) {
        AT_ERR_LOG("AT_SetEclImsCfgPara: value Error!");
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ConvertContentTypeModeToEcallDomainMode(VOS_UINT8 mode)
{
    AT_NORM_LOG1("AT_ConvertContentTypeModeToEcallDomainMode: ContentTypeMode", mode);
    if (mode == AT_IMSA_ECALL_CONTENT_TYPE_MODE_CS_PREFER) {
        return TAF_CALL_ECALL_DOMAIN_CS_PREFER;
    } else if (mode == AT_IMSA_ECALL_CONTENT_TYPE_MODE_PS_PREFER_CUSTOM) {
        return TAF_CALL_ECALL_DOMAIN_PS_PREFER;
    } else {
        return TAF_CALL_ECALL_DOMAIN_PS_PREFER;
    }
}

VOS_UINT32 AT_UpdateEcallDomainMode(VOS_UINT8 mode, ModemIdUint16 modemId, VOS_UINT8 indexNum)
{
    TAF_Ctrl                            ctrl;
    TAF_NVIM_CustomEcallCfg             ecallCfg;
    VOS_UINT32                          result;
    VOS_UINT32                          ecallDomainMode;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&ecallCfg, sizeof(ecallCfg), 0x00, sizeof(ecallCfg));

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    /* 先读取NV的值 */
    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_CUSTOM_ECALL_CFG, &ecallCfg, sizeof(TAF_NVIM_CustomEcallCfg));
    /* NV读取失败或NV未激活时，返回error */
    if (result != NV_OK) {
        AT_ERR_LOG("AT_UpdateEcallDomainMode: NV read!");
        return AT_ERROR;
    }

    ecallDomainMode = AT_ConvertContentTypeModeToEcallDomainMode(mode);
    /* 更新NV和全局变量 */
    if (ecallCfg.ecallDomainMode != ecallDomainMode) {
        ecallCfg.ecallDomainMode = (VOS_UINT8)ecallDomainMode;
        /* 写入NV的值 */
        result = TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_CUSTOM_ECALL_CFG, (VOS_UINT8 *)&ecallCfg,
            sizeof(TAF_NVIM_CustomEcallCfg));

        if (result != NV_OK) {
            AT_ERR_LOG("AT_UpdateEcallDomainMode: NV write!");
            return AT_ERROR;
        }

        /* 发消息到C核刷新ecall domain mode */
        result = TAF_CCM_CallCommonReq(&ctrl, &ecallDomainMode, ID_TAF_CCM_SET_ECALL_DOMAIN_MODE_REQ,
                                    sizeof(ecallDomainMode), modemId);
        if (result != TAF_SUCCESS) {
            AT_ERR_LOG("AT_UpdateEcallDomainMode: TAF_CCM_CallCommonReq FAIL");
            return AT_ERROR;
        }
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_FillEcallContentType(AT_IMSA_EcallContentTypeSetReq *ecallContentType)
{
    errno_t                             memValue;

    if (ecallContentType->mode == AT_IMSA_ECALL_CONTENT_TYPE_MODE_PS_PREFER_CUSTOM) {
        if ((g_atParaList[AT_ECLIMSCFG_CONTENT_INDEX].paraLen > AT_IMSA_CONTENT_TYPE_MAX_LEN) ||
            (g_atParaList[AT_ECLIMSCFG_CONTENT_INDEX].paraLen == 0)) {
            AT_ERR_LOG("AT_SetEclImsCfgPara: value Error!");
            return AT_CME_INCORRECT_PARAMETERS;
        }

        memValue = memcpy_s(ecallContentType->context, sizeof(ecallContentType->context),
                            g_atParaList[AT_ECLIMSCFG_CONTENT_INDEX].para,
                            g_atParaList[AT_ECLIMSCFG_CONTENT_INDEX].paraLen);
        TAF_MEM_CHK_RTN_VAL(memValue, sizeof(ecallContentType->context),
                            g_atParaList[AT_ECLIMSCFG_CONTENT_INDEX].paraLen);
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_SetEclImsCfgPara(VOS_UINT8 indexNum)
{
    AT_IMSA_EcallContentTypeSetReq      ecallContentType = {0};
    VOS_UINT32                          rst;
    ModemIdUint16                       modemId;

    /* 参数检查 */
    if (AT_CheckEclImsCfgPara() != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 新设置的值和当前值不一致时，刷新NV及全局变量 */
    ecallContentType.mode = (VOS_UINT8)g_atParaList[AT_ECLIMSCFG_MODE_INDEX].paraValue;
    if (AT_UpdateEcallDomainMode(ecallContentType.mode, modemId, indexNum) != AT_SUCCESS) {
        return AT_ERROR;
    }

    /* 填充 Ecall contentType */
    if (AT_FillEcallContentType(&ecallContentType) != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* ecallDomainMode 为CS_PREFER时，不需要通知IMSA */
    if (ecallContentType.mode == AT_IMSA_ECALL_CONTENT_TYPE_MODE_CS_PREFER) {
        return AT_OK;
    }
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
              ID_AT_IMSA_ECALL_ECONTENT_TYPE_SET_REQ, (VOS_UINT8 *)ecallContentType.context,
             (VOS_UINT32)(sizeof(ecallContentType.context) + sizeof(ecallContentType.mode) +
              sizeof(ecallContentType.reserved)),
              PS_PID_IMSA);
    if (rst != TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetEclImsCfgPara: AT_FillAndSndAppReqMsg FAIL");
        return AT_ERROR;
    }
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECLIMSCFG_SET;
    return AT_WAIT_ASYNC_RETURN;
}
#endif

