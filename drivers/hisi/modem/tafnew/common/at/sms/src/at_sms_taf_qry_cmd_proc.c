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
#include "at_sms_taf_qry_cmd_proc.h"
#include "securec.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_sms_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SMS_TAF_QRY_CMD_PROC_C

TAF_UINT32 At_QryCsdhPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, smsCtx->csdhType);

    return AT_OK;
}

TAF_UINT32 At_QryCnmiPara(TAF_UINT8 indexNum)
{
    TAF_UINT16      length = 0;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        smsCtx->cnmiType.cnmiModeType);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", smsCtx->cnmiType.cnmiMtType);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", smsCtx->cnmiType.cnmiBmType);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", smsCtx->cnmiType.cnmiDsType);
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d", smsCtx->cnmiType.cnmiBfrType);

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

TAF_UINT32 At_QryCmmsPara(TAF_UINT8 indexNum)
{
    TAF_UINT32 ret;

    g_atClientTab[indexNum].opId = At_GetOpId();
    ret                          = MN_MSG_GetLinkCtrl(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (ret == MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMMS_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryCmgfPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + g_atSendDataBuff.bufLen), "%s: %d",
        g_parseContext[indexNum].cmdElement->cmdName, smsCtx->cmgfMsgFormat);

    return AT_OK;
}

TAF_UINT32 At_QryCgsmsPara(TAF_UINT8 indexNum)
{
    TAF_UINT32 ret;

    g_atClientTab[indexNum].opId = At_GetOpId();

    ret = MN_MSG_GetSendDomain(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (ret == MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGSMS_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_QryCscaPara(VOS_UINT8 indexNum)
{
    VOS_UINT32           ret;
    MN_MSG_ReadCommParam readParam;

    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;
    smsCtx                 = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /*
     * 设置要查询的短信中心号码存储位置 :
     * AT模块获取短信中心号码等参数默认从SIM卡的EFSMSP文件的第一条记录中获取
     */

    readParam.index = (VOS_UINT32)smsCtx->cscaCsmpInfo.defaultSmspIndex;

    readParam.memStore = MN_MSG_MEM_STORE_SIM;
    (VOS_VOID)memset_s(readParam.reserve1, sizeof(readParam.reserve1), 0x00, sizeof(readParam.reserve1));

    /* 发消息到C核获取短信中心号码 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    ret = MN_MSG_ReadSrvParam(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &readParam);
    if (ret != MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSCA_READ;
    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_QryCsmsPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = 0;
    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)(g_atSndCodeAddress + g_atSendDataBuff.bufLen), "%s: %d,",
        g_parseContext[indexNum].cmdElement->cmdName, smsCtx->csmsMsgVersion);
    At_PrintCsmsInfo(indexNum);
    return AT_OK;
}

TAF_UINT32 At_QryCsmpPara(TAF_UINT8 indexNum)
{
    TAF_UINT16              length = 0;
    AT_ModemSmsCtx         *smsCtx = VOS_NULL_PTR;
    MN_MSG_ValidPeriodUint8 validPeriod; /* TP Validity Period Format */

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (smsCtx->cscaCsmpInfo.foUsed == TAF_TRUE) {
        /* <fo> */
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: %d,", g_parseContext[indexNum].cmdElement->cmdName,
            smsCtx->cscaCsmpInfo.fo);

        AT_GET_MSG_TP_VPF(validPeriod, smsCtx->cscaCsmpInfo.fo);

        /* <vp> */
        if (validPeriod == smsCtx->cscaCsmpInfo.vp.validPeriod) {
            length += At_MsgPrintVp(&(smsCtx->cscaCsmpInfo.vp), (g_atSndCodeAddress + length));
        }

    } else {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%s: ,", g_parseContext[indexNum].cmdElement->cmdName);
    }

    /* <pid>  <dcs> */
    length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress + length, ",%d,%d", smsCtx->cscaCsmpInfo.parmInUsim.pid,
        smsCtx->cscaCsmpInfo.parmInUsim.dcs);
    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

TAF_UINT32 At_QryCmgdPara(TAF_UINT8 indexNum)
{
    MN_MSG_ListParm listPara;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&listPara, sizeof(MN_MSG_ListParm), 0x00, sizeof(MN_MSG_ListParm));

    listPara.changeFlag = VOS_FALSE;
    listPara.memStore   = smsCtx->cpmsInfo.memReadorDelete;
    listPara.status     = MN_MSG_STATUS_NONE;

    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_Delete_Test(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &listPara) ==
        MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGD_TEST;
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_QryCmsrPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
        (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, smsCtx->paraCmsr);
    return AT_OK;
}

TAF_UINT32 At_QryCpmsPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (At_GetSmsStorage(indexNum, smsCtx->cpmsInfo.memReadorDelete, smsCtx->cpmsInfo.memSendorWrite,
                         smsCtx->cpmsInfo.rcvPath.smMemStore) != AT_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPMS_READ;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

VOS_UINT32 AT_QrySmsDomainPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

    /* AT 给MTA 发送查询请求消息 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_SMS_DOMAIN_QRY_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetSmsDomainPara: AT_QrySmsDomainPara fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SMSDOMAIN_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_QrySmsNasCapPara(TAF_UINT8 indexNum)
{
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_READ_CMD) {
        return AT_ERROR;
    }

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
    if (At_CheckCurrRatModeIsCL((VOS_UINT8)(g_atClientTab[indexNum].clientId)) == VOS_TRUE) {
        AT_WARN_LOG("AT_QrySmsNasCapPara: operation not allowed in CL mode.");
        return AT_CME_OPERATION_NOT_ALLOWED_IN_CL_MODE;
    }
#endif

    if (TAF_MMA_QrySmsNasCapReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId) !=
        VOS_TRUE) {
        AT_WARN_LOG("AT_QrySmsNasCapPara: send msg fail.");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SMSNASCAP_QRY;
    return AT_WAIT_ASYNC_RETURN;
}

/* AT^TRUSTNUM查询命令处理函数 */
VOS_UINT32 AT_QrySmsTrustNumPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_MtSmsNumTrustList smsTrustList;
    VOS_UINT32                 result;
    VOS_UINT32                 listIndex;
    VOS_UINT16                 length = 0;
    VOS_UINT8                  count  = 0;

    /* 从NV中读取短信Trust名单列表 */
    (VOS_VOID)memset_s(&smsTrustList, sizeof(smsTrustList), 0, sizeof(smsTrustList));

    result = TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MT_SMS_NUM_TRUST_LIST, &smsTrustList, sizeof(smsTrustList));
    if (result != NV_OK) {
        AT_ERR_LOG("AT_QrySmsTrustNumPara: Read NV fail.");
        return AT_ERROR;
    }

    /* 输出有效Trust名单号码 */
    for (listIndex = 0; listIndex < TAF_TRUSTLIST_MAX_LIST_SIZE; listIndex++) {
        if (VOS_StrLen((VOS_CHAR *)smsTrustList.recordList[listIndex].number) != 0) {
            if (count != 0) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);
            }
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,\"%s\"",
                g_parseContext[indexNum].cmdElement->cmdName,
                smsTrustList.recordList[listIndex].index, smsTrustList.recordList[listIndex].number);
            count++;
        }
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: WebSDK通过modem查询应用本地短信数据库是否还有空间存储短信
 * 输入参数: index: AT下发通道
 * 返回结果: AT_OK: 执行成功
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_QryHsmfPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_NvAppSmsFullFlag appMemStatus;

    /* 初始化 */
    (VOS_VOID)memset_s(&appMemStatus, sizeof(appMemStatus), 0x00, sizeof(appMemStatus));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_APP_SMS_FULL_FLG, &appMemStatus, sizeof(appMemStatus)) != NV_OK) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        appMemStatus.smsFullFlg);

    return AT_OK;
}
#endif

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
VOS_UINT32 At_QryCscbPara(VOS_UINT8 indexNum)
{
    TAF_UINT32 ret;

    g_atClientTab[indexNum].opId = At_GetOpId();

    ret = MN_MSG_GetAllCbMids(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);
    if (ret == MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSCB_READ;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestCscbPara(VOS_UINT8 indexNum)
{
    VOS_INT bufLen;

    bufLen = snprintf_s((VOS_CHAR *)g_atSndCodeAddress,
                        AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3,
                        (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3) - 1,
                        "%s: (0,1)", g_parseContext[indexNum].cmdElement->cmdName);
    if (bufLen < 0) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)bufLen;

    return AT_OK;
}
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能描述: +CMGS命令的打断处理
 *
 */
VOS_UINT32 AT_AbortCmgsPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    /* 特性是否打开 */
    if (AT_GetModemMbbCustCtxAddrFromClientId(indexNum)->smsAbortEnable == VOS_FALSE) {
        return AT_FAILURE;
    }

    /* +CMGS TEXT/PDU短信发送处理支持可中断 */
    if ((g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGS_TEXT_SET) ||
        (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_CMGS_PDU_SET)) {
        result = MSG_SendAppReq(MN_MSG_MSGTYPE_ABORT_RPDATA_DIRECT, g_atClientTab[indexNum].clientId,
            g_atClientTab[indexNum].opId, VOS_NULL_PTR);
        if (result == MN_ERR_NO_ERROR) {
            return AT_ABORT;
        }
    }

    return AT_FAILURE;
}
#endif

VOS_UINT32 AT_TestCnmaPara(VOS_UINT8 indexNum)
{
    VOS_INT         bufLen;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_PDU) { /* TEXT方式参数检查 */
        bufLen = snprintf_s((TAF_CHAR *)g_atSndCodeAddress, AT_SEND_CODE_ADDR_MAX_LEN, AT_SEND_CODE_ADDR_MAX_LEN - 1,
                            "%s: (0-2)", g_parseContext[indexNum].cmdElement->cmdName);
        if (bufLen < 0) {
            return AT_ERROR;
        }

        g_atSendDataBuff.bufLen = (VOS_UINT16)bufLen;
    } else {
        g_atSendDataBuff.bufLen = 0;
    }

    return AT_OK;
}

VOS_UINT32 AT_TestCpmsPara(VOS_UINT8 indexNum)
{
    VOS_INT         length;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (smsCtx->msgMeStorageStatus == MN_MSG_ME_STORAGE_ENABLE) {
        length = snprintf_s((VOS_CHAR *)g_atSndCodeAddress,
                            AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3,
                            (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3) - 1,
                            "%s: (\"SM\",\"ME\"),(\"SM\",\"ME\"),(\"SM\",\"ME\")",
                            g_parseContext[indexNum].cmdElement->cmdName);
    } else {
        length = snprintf_s((VOS_CHAR *)g_atSndCodeAddress,
                            AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3,
                            (AT_CMD_MAX_LEN + AT_CMD_PADDING_LEN - AT_SEND_DATA_BUFF_INDEX_3) - 1,
                            "%s: (\"SM\"),(\"SM\"),(\"SM\")", g_parseContext[indexNum].cmdElement->cmdName);
    }

    if (length < 0) {
        return AT_ERROR;
    }

    g_atSendDataBuff.bufLen = (VOS_UINT16)length;

    return AT_OK;
}

/* AT^TRUSTNUM测试命令处理函数 */
VOS_UINT32 AT_TestSmsTrustNumPara(VOS_UINT8 indexNum)
{
    g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: (0-19)", g_parseContext[indexNum].cmdElement->cmdName);
    return AT_OK;
}
