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
#include "at_sms_taf_set_cmd_proc.h"
#include "securec.h"
#include "nv_stru_gucnas.h"
#include "at_ctx.h"
#include "at_type_def.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_taf_agent_interface.h"
#include "at_input_proc.h"
#include "at_data_proc.h"
#include "at_check_func.h"
#include "at_mbb_cmd.h"
#include "at_msg_print.h"
#include "mnmsgcbencdec.h"
#include "taf_std_lib.h"
#include "mn_msg_api.h"
#include "at_sms_comm.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SMS_TAF_SET_CMD_PROC_C

#define AT_MN_MSG_RCVMSG_ACT_TRANSFER 2
#define AT_CNMI_PARA_MODE_TYPE 0
#define AT_CNMI_PARA_MT_TYPE 1
#define AT_CNMI_PARA_BM_TYPE 2
#define AT_CNMI_PARA_DS_TYPE 3
#define AT_CNMI_PARA_BFR_TYPE 4
#define AT_CNMI_MAX_PARA_NUM 5
#define AT_CMMS_MSG_LINK_CTRL 0
#define AT_CSCB_MIDS 1
#define AT_CSCB_DCSS 2
#define AT_CMGF_MSG_FORMAT 0
#define AT_CSCA_PARA_SCA 0
#define AT_CSCA_PARA_TOSCA 1
#define AT_CSMP_FO 0
#define AT_CSMP_PID 2
#define AT_CSMP_DCS 3
#define AT_MSG_DELETE_TYPE_NUM 5
#define AT_CSCA_PARA_NUM 2
#define AT_RELATIVE_VALID_PERIOD_MAX_VALUE 255
#define AT_CSMP_MAX_VP_VALUE 255
#define AT_CSMP_MAX_PARA_NUM 4
#define AT_CMGD_MAX_PARA_NUM 2
#define AT_CSCB_MIDS_MAX_VALID_CHARACTERS_NUM 6
#define AT_CMGS_TEXT_FORMAT_MAX_PARA_NUM 2
#define AT_ABSOLUTE_TIME_YEAR_VALID_LEN 2
#define AT_ABSOLUTE_TIME_MONTH_VALID_LEN 2
#define AT_ABSOLUTE_TIME_DAY_VALID_LEN 2
#define AT_ABSOLUTE_TIME_HOUR_VALID_LEN 2
#define AT_ABSOLUTE_TIME_MIN_VALID_LEN 2
#define AT_ABSOLUTE_TIME_SEC_VALID_LEN 2
#define AT_ABSOLUTE_TIME_TIMEZONE_VALID_LEN 2
#define AT_BCD_NUM_ARRAY_LEN 2
#define AT_ABSOLUTE_TIME_FORMAT_LEN 22
#define AT_ONE_HOUR_TIMEZONE_VALUE 4
#define AT_CMGW_MSG_TEXT_STAT 2
#define AT_CMGW_TPDU_LEN 0
#define AT_CMGW_MSG_PDU_STAT 1
#define AT_CMGW_MSG_OA_OR_DA 0
#define AT_CMGW_MSG_TOOA_OR_TODA 1
#define AT_CMGW_TEXT_FORMAT_MAX_PARA_NUM 3
#define AT_CMGW_PDU_FORMAT_MAX_PARA_NUM 2
#define AT_MN_MSG_SMS_TPDU_LEN 2
#define AT_CMGC_MSG_CMD_TYPE 1
#define AT_CMGC_MSG_FO 0
#define AT_CMGC_PDU_LEN 0
#define AT_CMGC_PID 2
#define AT_CMGC_MSG_NO 3
#define AT_CMGC_MSG_OA_OR_DA 4
#define AT_CMGC_MSG_TOOA_OR_TODA 5
#define AT_CMGC_MAX_PARA_NUM 6
#define AT_CMGL_MSG_STAT 0
#define AT_CMGR_MSG_INDEX 0
#define AT_CNMA_PDU_N_TYPE 0
#define AT_CNMA_TPDU_LEN 1
#define AT_CNMA_MAX_PARA_NUM 2
#define AT_CNMA_RP_ERROR 2
#define AT_CMSS_MSG_INDEX 0
#define AT_CMSS_MSG_DA 1
#define AT_CMSS_MSG_TODA 2
#define AT_CMSS_MAX_PARA_NUM 3
#define AT_CPMS_MEM_READOR_DELETE 0
#define AT_CPMS_MEM_SENDOR_WRITE 1
#define AT_CPMS_MEM_STORE 2
#define AT_CMST_MSG_INDEX 0
#define AT_CMST_MSG_DA 1
#define AT_CMST_MSG_TODA 2
#define AT_CMST_MSG_SENT_SM_TIME 3
#define AT_CMSTUB_STUB_TYPE 0
#define AT_CMSTUB_STUB_VALUE 1
#define AT_CMST_MAX_PARA_NUM 4
#define AT_CMMT_PARA_MAX_NUM 2
#define AT_CPMS_MAX_PARA_NUM 3
#define AT_AUC_TMP_LEN 4
#define AT_AUC_TMP_INDEX_2 2
#define AT_AUC_TMP_INDEX_3 3
#define AT_SET_SMS_TRUST_NUM_MAX_PARA_NUM 2

TAF_UINT32 At_SetCsdhPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 参数不为空 */
    if (g_atParaList[0].paraLen != 0) {
        smsCtx->csdhType = (AT_CSDH_TYPE)g_atParaList[0].paraValue;
    } else {
        smsCtx->csdhType = AT_CSDH_NOT_SHOW_TYPE;
    }
    return AT_OK;
}

TAF_UINT32 At_UpdateMsgRcvAct(TAF_UINT8 indexNum, AT_CSMS_MsgVersionUint8 csmsVersion, AT_CNMI_MT_TYPE cnmiMtType,
                              AT_CNMI_DS_TYPE cnmiDsType)
{
    MN_MSG_SetRcvmsgPathParm rcvPath;
    MN_MSG_RcvmsgActUint8    rcvSmAct[AT_CNMI_MT_TYPE_MAX] = {
        MN_MSG_RCVMSG_ACT_STORE,
        MN_MSG_RCVMSG_ACT_STORE,
        MN_MSG_RCVMSG_ACT_TRANSFER_AND_ACK,
        MN_MSG_RCVMSG_ACT_STORE
    };
    MN_MSG_RcvmsgActUint8 rcvStaRptAct[AT_CNMI_DS_TYPE_MAX] = {
        MN_MSG_RCVMSG_ACT_STORE,
        MN_MSG_RCVMSG_ACT_TRANSFER_AND_ACK,
        MN_MSG_RCVMSG_ACT_STORE
    };

    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;
    errno_t         memResult;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&rcvPath, sizeof(rcvPath), 0x00, sizeof(MN_MSG_SetRcvmsgPathParm));

    if ((cnmiMtType >= AT_CNMI_MT_TYPE_MAX) || (cnmiDsType >= AT_CNMI_DS_TYPE_MAX)) {
        return AT_ERROR;
    }

    memResult = memcpy_s(&rcvPath, sizeof(rcvPath), &(smsCtx->cpmsInfo.rcvPath), sizeof(rcvPath));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(rcvPath), sizeof(rcvPath));
    if (csmsVersion == AT_CSMS_MSG_VERSION_PHASE2_PLUS) {
        rcvSmAct[AT_MN_MSG_RCVMSG_ACT_TRANSFER]     = MN_MSG_RCVMSG_ACT_TRANSFER_ONLY;
        rcvStaRptAct[1] = MN_MSG_RCVMSG_ACT_TRANSFER_ONLY;
    }

    rcvPath.rcvSmAct       = rcvSmAct[cnmiMtType];
    rcvPath.rcvStaRptAct   = rcvStaRptAct[cnmiDsType];
    rcvPath.smsServVersion = csmsVersion;

    (VOS_VOID)memset_s(rcvPath.reserve1, sizeof(rcvPath.reserve1), 0x00, sizeof(rcvPath.reserve1));

    /* 将CNMI设置的MT type类型传输给TAF,在<MT>=3，收到CLASS3短信时候，按照CMT方式上报 */
    rcvPath.cnmiMtType = cnmiMtType;

    if (MN_MSG_SetRcvMsgPath(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &rcvPath) !=
        MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_VOID AT_SetCnmiTmpModeType(AT_ModemSmsCtx *smsCtx)
{
    if (g_atParaList[AT_CNMI_PARA_MODE_TYPE].paraLen != 0) {
        smsCtx->cnmiType.cnmiTmpModeType = (AT_CNMI_MODE_TYPE)g_atParaList[AT_CNMI_PARA_MODE_TYPE].paraValue;
    }
}

VOS_VOID AT_SetCnmiTmpMtType(AT_ModemSmsCtx *smsCtx)
{
    if (g_atParaList[AT_CNMI_PARA_MT_TYPE].paraLen != 0) {
        smsCtx->cnmiType.cnmiTmpMtType = (AT_CNMI_MT_TYPE)g_atParaList[AT_CNMI_PARA_MT_TYPE].paraValue;
    }
}

VOS_UINT32 AT_SetCnmiTmpBmType(AT_ModemSmsCtx *smsCtx)
{
    if (g_atParaList[AT_CNMI_PARA_BM_TYPE].paraLen != 0) {
        smsCtx->cnmiType.cnmiTmpBmType = (AT_CNMI_BM_TYPE)g_atParaList[AT_CNMI_PARA_BM_TYPE].paraValue;
        if ((smsCtx->cnmiType.cnmiTmpBmType != AT_CNMI_BM_CBM_TYPE) &&
            (smsCtx->cnmiType.cnmiTmpBmType != AT_CNMI_BM_NO_SEND_TYPE)) {
            return AT_CMS_OPERATION_NOT_SUPPORTED;
        }
    }

    return AT_OK;
}

VOS_VOID AT_SetCnmiTmpDsType(AT_ModemSmsCtx *smsCtx)
{
    if (g_atParaList[AT_CNMI_PARA_DS_TYPE].paraLen != 0) {
        smsCtx->cnmiType.cnmiTmpDsType = (AT_CNMI_DS_TYPE)g_atParaList[AT_CNMI_PARA_DS_TYPE].paraValue;
    }
}

VOS_VOID AT_SetCnmiTmpBfrType(AT_ModemSmsCtx *smsCtx)
{
    if (g_atParaList[AT_CNMI_PARA_BFR_TYPE].paraLen != 0) {
        smsCtx->cnmiType.cnmiTmpBfrType = (AT_CNMI_BFR_TYPE)g_atParaList[AT_CNMI_PARA_BFR_TYPE].paraValue;
    }
}

TAF_UINT32 At_SetCnmiPara(TAF_UINT8 indexNum)
{
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查和缺省参数配置 */
    /* 不带参数的设置命令，清空所有参数值为默认值 */
    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_CMD_NO_PARA) {
        smsCtx->cnmiType.cnmiTmpModeType = AT_CNMI_MODE_BUFFER_TYPE;
        smsCtx->cnmiType.cnmiTmpMtType   = AT_CNMI_MT_NO_SEND_TYPE;
        smsCtx->cnmiType.cnmiTmpBmType   = AT_CNMI_BM_NO_SEND_TYPE;
        smsCtx->cnmiType.cnmiTmpDsType   = AT_CNMI_DS_NO_SEND_TYPE;
        smsCtx->cnmiType.cnmiTmpBfrType  = AT_CNMI_BFR_SEND_TYPE;
    }

    if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) {
        /* 带参数的设置命令，参数个数不能为0且不能超过最大参数个数 */
        if ((g_atParaIndex == 0) || (g_atParaIndex > AT_CNMI_MAX_PARA_NUM)) {
            return AT_CMS_OPERATION_NOT_SUPPORTED;
        }

        /* 带参数的设置命令，用户未配置的参数不做修改 */
        smsCtx->cnmiType.cnmiTmpModeType = smsCtx->cnmiType.cnmiModeType;
        smsCtx->cnmiType.cnmiTmpMtType   = smsCtx->cnmiType.cnmiMtType;
        smsCtx->cnmiType.cnmiTmpBmType   = smsCtx->cnmiType.cnmiBmType;
        smsCtx->cnmiType.cnmiTmpDsType   = smsCtx->cnmiType.cnmiDsType;
        smsCtx->cnmiType.cnmiTmpBfrType  = smsCtx->cnmiType.cnmiBfrType;

        AT_SetCnmiTmpModeType(smsCtx);

        AT_SetCnmiTmpMtType(smsCtx);

        if (AT_SetCnmiTmpBmType(smsCtx) != AT_OK) {
            return AT_CMS_OPERATION_NOT_SUPPORTED;
        }

        AT_SetCnmiTmpDsType(smsCtx);

        AT_SetCnmiTmpBfrType(smsCtx);

        if ((smsCtx->cnmiType.cnmiTmpModeType == AT_CNMI_MODE_BUFFER_TYPE) ||
            (smsCtx->cnmiType.cnmiTmpModeType == AT_CNMI_MODE_SEND_OR_BUFFER_TYPE)) {
            if ((smsCtx->cnmiType.cnmiTmpMtType == AT_CNMI_MT_CMT_TYPE) ||
                (smsCtx->cnmiType.cnmiTmpMtType == AT_CNMI_MT_CLASS3_TYPE)) {
                return AT_CMS_OPERATION_NOT_SUPPORTED;
            }

            if (smsCtx->cnmiType.cnmiTmpDsType == AT_CNMI_DS_CDS_TYPE) {
                return AT_CMS_OPERATION_NOT_SUPPORTED;
            }
        }
    }

    g_atClientTab[indexNum].opId = At_GetOpId();
    ret                          = At_UpdateMsgRcvAct(indexNum, smsCtx->csmsMsgVersion, smsCtx->cnmiType.cnmiTmpMtType,
                                                      smsCtx->cnmiType.cnmiTmpDsType);
    if (ret != AT_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNMI_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

TAF_UINT32 At_SetCsmsPara(TAF_UINT8 indexNum)
{
    VOS_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查: 此处相对于旧代码有修改，待确认 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atClientTab[indexNum].opId = At_GetOpId();
    ret = At_UpdateMsgRcvAct(indexNum, (AT_CSMS_MsgVersionUint8)g_atParaList[0].paraValue, smsCtx->cnmiType.cnmiMtType,
                             smsCtx->cnmiType.cnmiDsType);

    if (ret != AT_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSMS_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

TAF_UINT32 At_SetCmmsPara(TAF_UINT8 indexNum)
{
    MN_MSG_LinkCtrlType setParam;
    TAF_UINT32          ret;
    MN_OPERATION_ID_T   opId = At_GetOpId();

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex == 0) {
        setParam.relayLinkCtrl = MN_MSG_LINK_CTRL_ENABLE;
    } else {
        setParam.relayLinkCtrl = (MN_MSG_LinkCtrlUint8)g_atParaList[AT_CMMS_MSG_LINK_CTRL].paraValue;
    }

    ret = MN_MSG_SetLinkCtrl(g_atClientTab[indexNum].clientId, opId, &setParam);
    if (ret != MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].opId          = opId;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMMS_SET;
    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_SetCmgfPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (((g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) &&
         (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA)) ||
        (g_atParaIndex > 1)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 参数为空 */
    if (g_atParaList[AT_CMGF_MSG_FORMAT].paraLen == 0) {
        g_atParaList[AT_CMGF_MSG_FORMAT].paraValue = AT_CMGF_MSG_FORMAT_PDU;
    }

    smsCtx->cmgfMsgFormat = (AT_CmgfMsgFormatUint8)g_atParaList[AT_CMGF_MSG_FORMAT].paraValue;
    return AT_OK;
}

TAF_UINT32 At_SetCgsmsPara(TAF_UINT8 indexNum)
{
    MN_MSG_SmsSendDomainType sendDomain;
    TAF_UINT32               ret;
    MN_OPERATION_ID_T        opId;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        g_atParaList[0].paraValue = MN_MSG_SEND_DOMAIN_CS;
    }

    opId = At_GetOpId();
    (VOS_VOID)memset_s(&sendDomain, sizeof(MN_MSG_SmsSendDomainType), 0x00, sizeof(MN_MSG_SmsSendDomainType));
    sendDomain.smsSendDomain = (MN_MSG_SendDomainUint8)g_atParaList[0].paraValue;

    ret = MN_MSG_SetSmsSendDomain(g_atClientTab[indexNum].clientId, opId, &sendDomain);
    if (ret != MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].opId          = opId;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CGSMS_SET;
    return AT_WAIT_ASYNC_RETURN;
}

TAF_UINT32 At_UnicodePrint2Ascii(TAF_UINT8 *dst, TAF_UINT8 *src, TAF_UINT16 srcLen)
{
    TAF_UINT8 *write = dst;
    TAF_UINT8 *read  = src;
    TAF_UINT8  tmp[AT_AUC_TMP_LEN];
    TAF_UINT16 chkLen   = 0;
    TAF_UINT8  indexNum = 0;

    /* 每4个字符可以转成一个ASCII码 */
    if ((srcLen % 4) != 0) {
        return AT_FAILURE;
    }

    /* 清空临时缓存 */
    (VOS_VOID)memset_s(tmp, sizeof(tmp), 0x00, AT_AUC_TMP_LEN);

    /* 字符均为'0'-'9','a'-'f','A'-'F' */
    while (chkLen++ < srcLen) {
        if ((*read >= '0') && (*read <= '9')) {
            tmp[indexNum] = *read - '0';
            indexNum++;
        } else if ((*read >= 'a') && (*read <= 'f')) {
            tmp[indexNum] = (*read - 'a') + 10; /* 十六进制格式转换 */
            indexNum++;
        } else if ((*read >= 'A') && (*read <= 'F')) {
            tmp[indexNum] = (*read - 'A') + 10; /* 十六进制格式转换 */
            indexNum++;
        } else {
            return AT_FAILURE;
        }

        /* 每四个字符得出一个字符 */
        if (indexNum == 4) {
            /* 必须是数字类型 */
            if ((tmp[0] != 0) || (tmp[1] != 0)) {
                return AT_FAILURE;
            }

            *write++ = (TAF_UINT8)(tmp[AT_AUC_TMP_INDEX_2] * 16) + tmp[AT_AUC_TMP_INDEX_3]; /* 写入一个字节 */
            if (At_CheckDialNum(*(write - 1)) == AT_FAILURE) {
                return AT_FAILURE;
            }
            /* 重新开始 */
            indexNum = 0;
        }

        /* 下一个字符 */
        read++;
    }

    return AT_SUCCESS;
}

/*
 * Description: 设置UNICODE或者ASCII类型的号码
 * History:
 *  1.Date: 2005-04-19
 *    Author:
 *    Modification: Created function
 */
TAF_UINT32 At_SetNumTypePara(TAF_UINT8 *dst, TAF_UINT32 dstLen, TAF_UINT8 *src, TAF_UINT16 srcLen)
{
    errno_t    memResult;
    TAF_UINT32 ret;
    TAF_UINT16 asciiStrLen;

    if (g_atCscsType == AT_CSCS_UCS2_CODE) {
        ret = At_UnicodePrint2Ascii(dst, src, srcLen);
        if (ret != AT_SUCCESS) {
            return ret;
        }
    } else {
        memResult = memcpy_s(dst, dstLen, src, srcLen);
        TAF_MEM_CHK_RTN_VAL(memResult, dstLen, srcLen);
    }

    asciiStrLen = (TAF_UINT16)VOS_StrLen((TAF_CHAR *)dst);
    if (At_CheckDialString(dst, asciiStrLen) == AT_SUCCESS) {
        return AT_SUCCESS;
    } else {
        return AT_FAILURE;
    }
}

/*
 * 功能描述: 检查Asc或者Bcd码地址有效性
 */
LOCAL VOS_UINT32 At_CheckAsciiOrBcdAddr(MN_MSG_AsciiAddr *asciiAddr, MN_MSG_BcdAddr *bcdAddr)
{
    if ((asciiAddr == TAF_NULL_PTR) && (bcdAddr == TAF_NULL_PTR)) {
        AT_WARN_LOG("At_CheckAsciiOrBcdAddr: output parameter is null. ");
        return AT_ERROR;
    }

    if (asciiAddr != TAF_NULL_PTR) {
        (VOS_VOID)memset_s(asciiAddr, sizeof(MN_MSG_AsciiAddr), 0x00, sizeof(MN_MSG_AsciiAddr));
    }

    if (bcdAddr != TAF_NULL_PTR) {
        (VOS_VOID)memset_s(bcdAddr, sizeof(MN_MSG_BcdAddr), 0x00, sizeof(MN_MSG_BcdAddr));
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ChkSmsNumType(MN_MSG_TonUint8 numType)
{
    return MN_ERR_NO_ERROR;
}

VOS_UINT32 AT_ChkSmsNumPlan(MN_MSG_NpiUint8 numPlan)
{
    return MN_ERR_NO_ERROR;
}

/*
 * 功能描述: 号码类型检查
 */
LOCAL VOS_UINT32 At_CheckSmsNumPlanAndType(MN_MSG_NpiUint8 numPlan, MN_MSG_TonUint8 numType)
{
    TAF_UINT32      ret;

    ret = AT_ChkSmsNumPlan(numPlan);
    if (ret != MN_ERR_NO_ERROR) {
        AT_ERR_LOG("At_CheckSmsNumPlanAndType: Numbering plan is invalid");
        return AT_ERROR;
    }
    ret = AT_ChkSmsNumType(numType);
    if (ret != MN_ERR_NO_ERROR) {
        AT_ERR_LOG("At_CheckSmsNumPlanAndType: Number type is invalid");
        return AT_ERROR;
    }

    return AT_SUCCESS;
}

/*
 * 功能描述: 获取Ascii地址信息
 */
LOCAL VOS_VOID At_GetAsciiAddrInfo(MN_MSG_TonUint8 numType, MN_MSG_NpiUint8 numPlan, TAF_UINT32 asciiAddrLen,
    const TAF_UINT8 *num, MN_MSG_AsciiAddr *asciiAddr)
{
    errno_t         memResult;

    if (asciiAddr != TAF_NULL_PTR) {
        asciiAddr->numType = numType;
        asciiAddr->numPlan = numPlan;
        asciiAddr->len     = asciiAddrLen;
        if (asciiAddr->len > 0) {
            memResult = memcpy_s(asciiAddr->asciiNum, sizeof(asciiAddr->asciiNum), num, asciiAddr->len);
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(asciiAddr->asciiNum), asciiAddr->len);
        }
    }
}

/*
 * 功能描述: 获取Bcd地址信息
 */
LOCAL VOS_UINT32 At_GetBcdAddrInfo(TAF_UINT8 addrType, const TAF_UINT8 *num, MN_MSG_BcdAddr *bcdAddr)
{
    TAF_UINT32      ret;

    if (bcdAddr != TAF_NULL_PTR) {
        bcdAddr->addrType = addrType;
        ret               = AT_AsciiNumberToBcd((TAF_CHAR *)num, bcdAddr->bcdNum, &bcdAddr->bcdLen);
        if (ret != MN_ERR_NO_ERROR) {
            return AT_ERROR;
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_GetAsciiOrBcdAddr(TAF_UINT8 *addr, TAF_UINT16 addrLen, TAF_UINT8 addrType, TAF_UINT16 numTypeLen,
                                MN_MSG_AsciiAddr *asciiAddr, MN_MSG_BcdAddr *bcdAddr)
{
    TAF_UINT8 asciiNum[MN_MAX_ASCII_ADDRESS_NUM + 2]; /* array  of ASCII Num */
    /* 指向实际号码（不包括+号）的指针 */
    TAF_UINT8      *num = VOS_NULL_PTR;
    TAF_UINT32      asciiAddrLen;
    MN_MSG_TonUint8 numType; /* type of number */
    MN_MSG_NpiUint8 numPlan; /* Numbering plan identification */

    if (At_CheckAsciiOrBcdAddr(asciiAddr, bcdAddr) == AT_ERROR) {
        AT_WARN_LOG("At_GetAsciiOrBcdAddr: output parameter is null. ");
        return AT_ERROR;
    }

    if (addrLen == 0) {
        return AT_OK;
    }

    if (At_CheckNumLen((MN_MAX_ASCII_ADDRESS_NUM + 1), addrLen) == AT_FAILURE) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    *(addr + addrLen) = '\0';

    /* MN_MAX_ASCII_ADDRESS_NUM + 2为aucAsciiNum的长度 */
    (VOS_VOID)memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));
    if (At_SetNumTypePara(asciiNum, sizeof(asciiNum), (TAF_UINT8 *)addr, addrLen) != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 设置<toda> */
    num = (TAF_UINT8 *)asciiNum;
    if ((At_GetCodeType(asciiNum[0])) == AT_MSG_INTERNAL_ISDN_ADDR_TYPE) {
        num = (TAF_UINT8 *)(asciiNum + 1);
        if ((numTypeLen != 0) && (addrType != AT_MSG_INTERNAL_ISDN_ADDR_TYPE)) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }

    if (numTypeLen == 0) {
        addrType = (TAF_UINT8)At_GetCodeType(asciiNum[0]);
    }

    asciiAddrLen = VOS_StrLen((TAF_CHAR *)num);
    if (asciiAddrLen > MN_MAX_ASCII_ADDRESS_NUM) {
        AT_NORM_LOG("At_GetAsciiOrBcdAddr: invalid address length.");
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    At_GetNumTypeFromAddrType(numType, addrType);
    At_GetNumPlanFromAddrType(numPlan, addrType);
    if (At_CheckSmsNumPlanAndType(numPlan, numType) == AT_ERROR) {
        return AT_ERROR;
    }

    At_GetAsciiAddrInfo(numType, numPlan, asciiAddrLen, num, asciiAddr);

    if (At_GetBcdAddrInfo(addrType, num, bcdAddr) == AT_ERROR) {
        return AT_ERROR;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCscaPara(TAF_UINT8 indexNum)
{
    errno_t                  memResult;
    TAF_UINT8                asciiNum[MN_MAX_ASCII_ADDRESS_NUM + 2]; /* array  of ASCII Num */
    TAF_UINT8                scaType;
    TAF_UINT32               ret;
    MN_MSG_WriteSrvParameter servParm;
    /* 指向实际号码（不包括+号）的指针 */
    TAF_UINT8        *num = VOS_NULL_PTR;
    MN_MSG_SrvParam   parmInUsim;
    MN_OPERATION_ID_T opId   = At_GetOpId();
    AT_ModemSmsCtx   *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&servParm, sizeof(servParm), 0x00, sizeof(MN_MSG_WriteSrvParameter));
    (VOS_VOID)memset_s(&parmInUsim, sizeof(parmInUsim), 0x00, sizeof(MN_MSG_SrvParam));

    /* 参数检查 */
    if ((g_atParaList[AT_CSCA_PARA_SCA].paraLen == 0) || (g_atParaIndex > AT_CSCA_PARA_NUM)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 如果<sca>号码过长，直接返回错误 */
    if (At_CheckNumLen((MN_MAX_ASCII_ADDRESS_NUM + 1), g_atParaList[AT_CSCA_PARA_SCA].paraLen) == AT_FAILURE) {
        return AT_ERROR;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(asciiNum, sizeof(asciiNum), 0x00, sizeof(asciiNum));
    /* 设置<sca> , 将UCS2码转换成ASCII码 */
    ret = At_SetNumTypePara(asciiNum, sizeof(asciiNum), g_atParaList[AT_CSCA_PARA_SCA].para,
                            g_atParaList[AT_CSCA_PARA_SCA].paraLen);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    scaType = (TAF_UINT8)At_GetCodeType(asciiNum[0]);
    if (scaType == AT_MSG_INTERNAL_ISDN_ADDR_TYPE) {
        num = &asciiNum[1];
    } else {
        num = asciiNum;
    }

    /* 设置<tosca> */
    if (g_atParaList[AT_CSCA_PARA_TOSCA].paraLen != 0) {
        if (scaType == AT_MSG_INTERNAL_ISDN_ADDR_TYPE) {
            if (scaType != ((TAF_UINT8)g_atParaList[AT_CSCA_PARA_TOSCA].paraValue)) {
                return AT_CMS_OPERATION_NOT_ALLOWED;
            }
        } else {
            scaType = (TAF_UINT8)g_atParaList[AT_CSCA_PARA_TOSCA].paraValue;
        }
    }

    /* 执行命令操作 */
    memResult = memcpy_s(&parmInUsim, sizeof(parmInUsim), &(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(parmInUsim));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(parmInUsim), sizeof(parmInUsim));

    parmInUsim.parmInd &= ~MN_MSG_SRV_PARM_MASK_SC_ADDR;
    parmInUsim.scAddr.addrType = scaType;
    ret = AT_AsciiNumberToBcd((TAF_CHAR *)num, parmInUsim.scAddr.bcdNum, &parmInUsim.scAddr.bcdLen);
    if (ret != MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }
    servParm.memStore  = MN_MSG_MEM_STORE_SIM;
    servParm.writeMode = MN_MSG_WRITE_MODE_REPLACE;

    servParm.index = (VOS_UINT32)smsCtx->cscaCsmpInfo.defaultSmspIndex;

    memResult = memcpy_s(&servParm.srvParm, sizeof(servParm.srvParm), &parmInUsim, sizeof(servParm.srvParm));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(servParm.srvParm), sizeof(servParm.srvParm));
    g_atClientTab[indexNum].opId = opId;
    if (MN_MSG_WriteSrvParam(g_atClientTab[indexNum].clientId, opId, &servParm) == MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSCA_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_ParseCsmpFo(VOS_UINT8 *fo)
{
    TAF_UINT32 ret;

    /* 检查<fo>,数字类型 */
    ret = At_CheckNumString(g_atParaList[0].para, g_atParaList[0].paraLen);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 注意: g_atParaList[0].ulParaValue此时尚未转换，检查其它命令的这种情况 */
    ret = At_Auc2ul(g_atParaList[0].para, g_atParaList[0].paraLen, &g_atParaList[0].paraValue);
    if (ret == AT_FAILURE) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 检查<fo>,一个字节 */
    if (g_atParaList[0].paraValue > 0xff) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (((0x03 & g_atParaList[0].paraValue) != 0x01) && ((0x03 & g_atParaList[0].paraValue) != 0x02)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    *fo = (TAF_UINT8)g_atParaList[0].paraValue;

    return AT_SUCCESS;
}

/*
 * 功能描述: AT命令AbsoluteTime中参数的检查
 */
LOCAL VOS_UINT32 AT_CheckAbsoluteTime(TAF_UINT8 *timeStr, TAF_UINT16 timeStrLen, MN_MSG_Timestamp *absoluteTime)
{
    /*
     * 6th of May 1994, 22:10:00 GMT+2 "94/05/06,22:10:00+08"
     * 注意:还要判断中间字符是否合法
     */
    if ((timeStr == TAF_NULL_PTR) || (absoluteTime == TAF_NULL_PTR)) {
        AT_WARN_LOG("At_GetAbsoluteTime: parameter is NULL.");
        return AT_ERROR;
    }

    /* 检查<vp>,字符串类型，格式必须符合"yy/mm/dd,hh:mm:ss(+/-)tz,dst", 否则返回错误 */
    if ((timeStrLen != AT_ABSOLUTE_TIME_FORMAT_LEN) ||
        (timeStr[0] != '"') ||(timeStr[timeStrLen - 1] != '"') || /* '"' */
        (timeStr[3] != '/') ||                                    /* '/' */
        (timeStr[6] != '/') ||                                    /* '/' */
        (timeStr[9] != ',') ||                                    /* ',' */
        (timeStr[12] != ':') ||                                   /* ':' */
        (timeStr[15] != ':')) {                                   /* ':' */
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    return AT_OK;
}

TAF_UINT32 At_AsciiNum2BcdNum(TAF_UINT8 *dst, TAF_UINT8 *src, TAF_UINT16 srcLen)
{
    TAF_UINT16 chkLen = 0;
    TAF_UINT8  tmp    = 0;
    TAF_UINT8  bcdCode;

    for (chkLen = 0; chkLen < srcLen; chkLen++) {
        tmp = chkLen % 2; /* 判断高低位 */

        if ((src[chkLen] >= 0x30) && (src[chkLen] <= 0x39)) { /* the number is 0-9 */
            bcdCode = src[chkLen] - 0x30;
        } else if (src[chkLen] == '*') {
            bcdCode = 0x0a;
        } else if (src[chkLen] == '#') {
            bcdCode = 0x0b;
        } else if ((src[chkLen] == 'a') || (src[chkLen] == 'b') || (src[chkLen] == 'c')) {
            bcdCode = (VOS_UINT8)((src[chkLen] - 'a') + 0x0c);
        } else {
            return AT_FAILURE;
        }

        tmp = chkLen % 2; /* 判断高低位 */
        if (tmp == 0) {
            dst[chkLen / 2] = bcdCode; /* 低位 */
        } else {
            dst[chkLen / 2] |= (TAF_UINT8)(bcdCode << 4); /* 高位 */
        }
    }

    if ((srcLen % 2) == 1) { /* 判断高低位 */
        dst[srcLen / 2] |= 0xf0; /* 高位 */
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_GetAbsoluteTime(TAF_UINT8 *timeStr, TAF_UINT16 timeStrLen, MN_MSG_Timestamp *absoluteTime)
{
    TAF_UINT32 ret;
    TAF_UINT8  bcdNum[AT_BCD_NUM_ARRAY_LEN];
    TAF_UINT32 tmp = 0;

    bcdNum[0] = 0;
    bcdNum[1] = 0;

    ret = AT_CheckAbsoluteTime(timeStr, timeStrLen, absoluteTime);
    if (ret != AT_OK) {
        return ret;
    }

    /* Year */
    ret = At_AsciiNum2BcdNum(bcdNum, &timeStr[1], AT_ABSOLUTE_TIME_YEAR_VALID_LEN);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    absoluteTime->year = AT_BCD_REVERSE(bcdNum[0]);

    /* Month */
    ret = At_AsciiNum2BcdNum(bcdNum, &timeStr[4], AT_ABSOLUTE_TIME_MONTH_VALID_LEN);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    absoluteTime->month = AT_BCD_REVERSE(bcdNum[0]);

    /* Day */
    ret = At_AsciiNum2BcdNum(bcdNum, &timeStr[7], AT_ABSOLUTE_TIME_DAY_VALID_LEN);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    absoluteTime->day = AT_BCD_REVERSE(bcdNum[0]);

    /* Hour */
    ret = At_AsciiNum2BcdNum(bcdNum, &timeStr[10], AT_ABSOLUTE_TIME_HOUR_VALID_LEN);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    absoluteTime->hour = AT_BCD_REVERSE(bcdNum[0]);

    /* Minute */
    ret = At_AsciiNum2BcdNum(bcdNum, &timeStr[13], AT_ABSOLUTE_TIME_MIN_VALID_LEN);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    absoluteTime->minute = AT_BCD_REVERSE(bcdNum[0]);

    /* Second */
    ret = At_AsciiNum2BcdNum(bcdNum, &timeStr[16], AT_ABSOLUTE_TIME_SEC_VALID_LEN);
    if (ret != AT_SUCCESS) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    absoluteTime->second = AT_BCD_REVERSE(bcdNum[0]);
   /* timezone */
    if (At_Auc2ul(&timeStr[19], AT_ABSOLUTE_TIME_TIMEZONE_VALID_LEN, &tmp) == AT_FAILURE) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (((tmp % AT_ONE_HOUR_TIMEZONE_VALUE) != 0) || (tmp > AT_MAX_TIMEZONE_VALUE)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* '+' 或者 '-' */
    switch (timeStr[18]) {
        case '+':
            absoluteTime->timezone = (TAF_INT8)tmp;
            break;

        case '-':
            absoluteTime->timezone = (TAF_INT8)((-1) * tmp);
            break;

        default:
            return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetAbsoluteValidPeriod(VOS_UINT8 indexNum, TAF_UINT8 *para, TAF_UINT16 paraLen,
                                     MN_MSG_ValidPeriod *validPeriod)
{
    errno_t         memResult;
    TAF_UINT32      ret;
    TAF_UINT8       dateInvalidType;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (paraLen == 0) {
        if (smsCtx->cscaCsmpInfo.vp.validPeriod == MN_MSG_VALID_PERIOD_ABSOLUTE) {
            memResult = memcpy_s(validPeriod, sizeof(MN_MSG_ValidPeriod), &(smsCtx->cscaCsmpInfo.vp),
                                 sizeof(MN_MSG_ValidPeriod));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(MN_MSG_ValidPeriod), sizeof(MN_MSG_ValidPeriod));
            return AT_SUCCESS;
        } else {
            return AT_ERROR;
        }
    } else {
        ret = At_GetAbsoluteTime(para, paraLen, &validPeriod->u.absoluteTime);
        if (ret != AT_SUCCESS) {
            return ret;
        }

        ret = MN_MSG_ChkDate(&validPeriod->u.absoluteTime, &dateInvalidType);
        if (ret != MN_ERR_NO_ERROR) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        return AT_SUCCESS;
    }
}

VOS_UINT32 AT_SetRelativeValidPeriod(VOS_UINT8 indexNum, TAF_UINT8 *para, TAF_UINT16 paraLen,
                                     MN_MSG_ValidPeriod *validPeriod)
{
    errno_t         memResult;
    TAF_UINT32      ret;
    TAF_UINT32      relativeValidPeriod;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (paraLen == 0) {
        if (smsCtx->cscaCsmpInfo.vp.validPeriod == MN_MSG_VALID_PERIOD_RELATIVE) {
            memResult = memcpy_s(validPeriod, sizeof(MN_MSG_ValidPeriod), &(smsCtx->cscaCsmpInfo.vp),
                                 sizeof(MN_MSG_ValidPeriod));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(MN_MSG_ValidPeriod), sizeof(MN_MSG_ValidPeriod));
            return AT_SUCCESS;
        } else if (smsCtx->cscaCsmpInfo.vp.validPeriod == MN_MSG_VALID_PERIOD_NONE) {
            validPeriod->u.otherTime = AT_CSMP_SUBMIT_VP_DEFAULT_VALUE;
            return AT_SUCCESS;
        } else {
            return AT_ERROR;
        }
    } else {
        /* 检查<vp>,数字类型 */
        ret = At_CheckNumString(para, paraLen);
        if (ret != AT_SUCCESS) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        /* 注意: g_atParaList[1].ulParaValue此时尚未转换，检查其它命令的这种情况 */
        ret = At_Auc2ul(para, paraLen, &relativeValidPeriod);
        if (ret == AT_FAILURE) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (relativeValidPeriod > AT_RELATIVE_VALID_PERIOD_MAX_VALUE) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        validPeriod->u.otherTime = (TAF_UINT8)relativeValidPeriod;
        return AT_SUCCESS;
    }
}

VOS_UINT32 At_ParseCsmpVp(VOS_UINT8 indexNum, MN_MSG_ValidPeriod *vp)
{
    errno_t         memResult;
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 获取当前配置的TP-VPF值，若用户不配置TP-VPF和TP-VP两项，则复制当前结构到临时结构并推出 */
    if (g_atParaList[0].paraLen != 0) {
        AT_GET_MSG_TP_VPF(vp->validPeriod, smsCtx->cscaCsmpInfo.tmpFo);
    } else if (g_atParaList[1].paraLen != 0) {
        vp->validPeriod = smsCtx->cscaCsmpInfo.vp.validPeriod;
    } else {
        memResult = memcpy_s(vp, sizeof(MN_MSG_ValidPeriod), &(smsCtx->cscaCsmpInfo.vp), sizeof(MN_MSG_ValidPeriod));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(MN_MSG_ValidPeriod), sizeof(MN_MSG_ValidPeriod));
        return AT_SUCCESS;
    }

    /* 短信有效期类型设置为无效，<VP>参数项必须为空 */
    if (vp->validPeriod == MN_MSG_VALID_PERIOD_NONE) {
        if (g_atParaList[1].paraLen != 0) {
            return AT_ERROR;
        }
        (VOS_VOID)memset_s(vp, sizeof(MN_MSG_ValidPeriod), 0x00, sizeof(MN_MSG_ValidPeriod));
        return AT_SUCCESS;
    }
    /* 短信有效期类型设置为相对有效期， */
    else if (vp->validPeriod == MN_MSG_VALID_PERIOD_RELATIVE) {
        ret = AT_SetRelativeValidPeriod(indexNum, g_atParaList[1].para, g_atParaList[1].paraLen, vp);
        return ret;
    } else if (vp->validPeriod == MN_MSG_VALID_PERIOD_ABSOLUTE) {
        ret = AT_SetAbsoluteValidPeriod(indexNum, g_atParaList[1].para, g_atParaList[1].paraLen, vp);
        return ret;
    } else {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
}

/*
 * Description: 解析csmp的pid参数
 * History:
 *  1.Date: 2020-08-11
 *    Author:
 *    Modification: Created function
 */
VOS_UINT32 At_ParseCsmpPid(MN_MSG_SrvParam *parmInUsim)
{
    if (g_atParaList[AT_CSMP_PID].paraLen != 0) {
        /* 注意: g_atParaList[2].ulParaValue此时尚未未转换，检查其它命令的这种情况 */
        if (At_Auc2ul(g_atParaList[AT_CSMP_PID].para, g_atParaList[AT_CSMP_PID].paraLen,
                      &g_atParaList[AT_CSMP_PID].paraValue) == AT_FAILURE) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (g_atParaList[AT_CSMP_PID].paraValue <= AT_CSMP_MAX_VP_VALUE) {
            parmInUsim->parmInd &= ~MN_MSG_SRV_PARM_MASK_PID;
            parmInUsim->pid = (TAF_UINT8)g_atParaList[AT_CSMP_PID].paraValue;
        } else {
            AT_NORM_LOG("At_SetCsmpPara: unsupport VP type.");
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }

    return AT_SUCCESS;
}
/*
 * Description: 解析csmp的dcs参数
 * History:
 *  1.Date: 2020-08-11
 *    Author:
 *    Modification: Created function
 */
VOS_UINT32 At_ParseCsmpDcs(MN_MSG_SrvParam *parmInUsim)
{
    if (g_atParaList[AT_CSMP_DCS].paraLen != 0) {
        /* 注意: g_atParaList[3].ulParaValue此时尚未未转换，检查其它命令的这种情况 */
        if (At_Auc2ul(g_atParaList[AT_CSMP_DCS].para, g_atParaList[AT_CSMP_DCS].paraLen,
                      &g_atParaList[AT_CSMP_DCS].paraValue) == AT_FAILURE) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (g_atParaList[AT_CSMP_DCS].paraValue <= AT_CSMP_MAX_VP_VALUE) {
            parmInUsim->parmInd &= ~MN_MSG_SRV_PARM_MASK_DCS;
            parmInUsim->dcs = (TAF_UINT8)g_atParaList[AT_CSMP_DCS].paraValue;
        } else {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }

    return AT_SUCCESS;
}

TAF_UINT32 At_SetCsmpPara(TAF_UINT8 indexNum)
{
    errno_t                  memResult;
    MN_MSG_WriteSrvParameter servParm;
    MN_MSG_SrvParam          parmInUsim;
    TAF_UINT32               ret;
    AT_ModemSmsCtx          *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&servParm, sizeof(servParm), 0x00, sizeof(MN_MSG_WriteSrvParameter));
    (VOS_VOID)memset_s(&parmInUsim, sizeof(parmInUsim), 0x00, sizeof(MN_MSG_SrvParam));

    /* 参数检查 */
    if ((g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) || (g_atParaIndex > AT_CSMP_MAX_PARA_NUM)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 设置<fo> */
    if (g_atParaList[AT_CSMP_FO].paraLen != 0) {
        ret = At_ParseCsmpFo(&(smsCtx->cscaCsmpInfo.tmpFo));
        if (ret != AT_SUCCESS) {
            return ret;
        }
    } else {
        smsCtx->cscaCsmpInfo.tmpFo = smsCtx->cscaCsmpInfo.fo;
    }

    memResult = memcpy_s(&parmInUsim, sizeof(parmInUsim), &(smsCtx->cscaCsmpInfo.parmInUsim), sizeof(parmInUsim));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(parmInUsim), sizeof(parmInUsim));

    /* 设置<vp> */
    ret = At_ParseCsmpVp(indexNum, &(smsCtx->cscaCsmpInfo.tmpVp));
    if (ret != AT_SUCCESS) {
        return ret;
    }

    /* 设置<pid> */
    ret = At_ParseCsmpPid(&parmInUsim);
    if (ret != AT_SUCCESS) {
        return ret;
    }

    /* 设置<dcs> */
    ret = At_ParseCsmpDcs(&parmInUsim);
    if (ret != AT_SUCCESS) {
        return ret;
    }

    /* 执行命令操作 */
    servParm.memStore  = MN_MSG_MEM_STORE_SIM;
    servParm.writeMode = MN_MSG_WRITE_MODE_REPLACE;

    servParm.index = (VOS_UINT32)smsCtx->cscaCsmpInfo.defaultSmspIndex;

    memResult = memcpy_s(&servParm.srvParm, sizeof(servParm.srvParm), &parmInUsim, sizeof(servParm.srvParm));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(servParm.srvParm), sizeof(servParm.srvParm));
    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_WriteSrvParam(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &servParm) ==
        MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSMP_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCmgdPara(TAF_UINT8 indexNum)
{
    MN_MSG_DeleteParam deleteInfo;
    MN_OPERATION_ID_T  opId            = At_GetOpId();
    TAF_UINT32         deleteTypeIndex = 0;
    TAF_UINT32         deleteTypes[AT_MSG_DELETE_TYPE_NUM]  = {
        AT_MSG_DELETE_SINGLE,
        AT_MSG_DELETE_READ,
        (AT_MSG_DELETE_READ + AT_MSG_DELETE_SENT),
        (AT_MSG_DELETE_READ + AT_MSG_DELETE_SENT + AT_MSG_DELETE_UNSENT),
        AT_MSG_DELETE_ALL
    };
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&deleteInfo, sizeof(deleteInfo), 0x00, sizeof(deleteInfo));

    /* 参数检查 */
    if (g_atParaIndex > AT_CMGD_MAX_PARA_NUM) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 设置<index> */
    deleteInfo.index = g_atParaList[0].paraValue;

    /* 设置删除存储器 */
    deleteInfo.memStore = smsCtx->cpmsInfo.memReadorDelete;

    /* 设置<delflag> */
    if (g_atParaList[1].paraLen != 0) {
        deleteTypeIndex = g_atParaList[1].paraValue;
    } else {
        deleteTypeIndex = 0;
    }

    g_atClientTab[indexNum].atSmsData.msgDeleteTypes = (TAF_UINT8)deleteTypes[deleteTypeIndex];
    if (g_atClientTab[indexNum].atSmsData.msgDeleteTypes == AT_MSG_DELETE_SINGLE) {
        if (g_atParaList[0].paraLen == 0) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }

    /* 执行命令操作 */
    ret = At_MsgDeleteCmdProc(indexNum, opId, deleteInfo, g_atClientTab[indexNum].atSmsData.msgDeleteTypes);
    if (ret != AT_OK) {
        return ret;
    }

    g_atClientTab[indexNum].opId          = opId;
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGD_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

#if ((FEATURE_GCBS == FEATURE_ON) || (FEATURE_WCBS == FEATURE_ON))
VOS_UINT32 AT_ProcParseCbAuc2ul(VOS_BOOL *bRange, TAF_UINT8 *temp, VOS_UINT32 *parmCount,
    TAF_CBA_CbmiRangeList *cbRange)
{
    VOS_UINT32 value;
    TAF_CBA_MsgidSrcFlgUint8 midSrcFlg;

    midSrcFlg = TAF_CBA_MSGID_SRC_FLG_NULL;

    if (*bRange) {
        if (At_Auc2ul(temp, (VOS_UINT16)VOS_StrLen((VOS_CHAR *)temp), &value) != AT_SUCCESS) {
            return VOS_ERR;
        }

        if (value > 0xFFFF) {
            return VOS_ERR;
        }

        if ((*parmCount) >= 1) {
            cbRange->cbmiRangeInfo[(*parmCount) - 1].msgIdTo = (VOS_UINT16)value;

            (*bRange) = VOS_FALSE;
        }

    } else {
        if (At_Auc2ul(temp, (VOS_UINT16)VOS_StrLen((VOS_CHAR *)temp), &value) != AT_SUCCESS) {
            return VOS_ERR;
        }

        if (value > 0xFFFF) {
            return VOS_ERR;
        }

        cbRange->cbmiRangeInfo[*parmCount].msgIdFrom = (VOS_UINT16)value;
        cbRange->cbmiRangeInfo[*parmCount].msgIdTo   = (VOS_UINT16)value;
        /* 用户操作的，需要刷新到卡和NV，所以文件标记为 NV + EFCBMI+ EFCBMIR */
        midSrcFlg = TAF_CBA_MSGID_SRC_FLG_NV;
        midSrcFlg |= TAF_CBA_MSGID_SRC_FLG_EFCBMI;
        midSrcFlg |= TAF_CBA_MSGID_SRC_FLG_EFCBMIR;

        cbRange->cbmiRangeInfo[*parmCount].msgIdSrcFlg = midSrcFlg;
        (cbRange->cbmirNum)++;
        (*parmCount)++;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetbRangeFlgParseCb(VOS_UINT8 *inPara, VOS_UINT32 iLoop, VOS_BOOL *bRange)
{
    if (inPara[iLoop] == '-') {
        *bRange = VOS_TRUE;
    } else if (inPara[iLoop] == '\0') {
        return VOS_OK;
    } else {
        *bRange = VOS_FALSE;
    }

    return VOS_ERR;
}

VOS_UINT32 AT_ParseCbParm(VOS_UINT8 *inPara, TAF_CBA_CbmiRangeList *cbRange)
{
    VOS_UINT32 i;
    VOS_UINT32 iLoop;
    VOS_UINT32 parmCount;
    TAF_UINT8  temp[TAF_CBA_MAX_CBMID_RANGE_NUM];
    VOS_BOOL   bRange = VOS_FALSE;

    cbRange->cbmirNum = 0;
    iLoop             = 0;
    parmCount         = 0;
    i                 = 0;

    while ((inPara[iLoop] != '\0') && (i < TAF_CBA_MAX_CBMID_RANGE_NUM)) {
        i = 0;
        while ((inPara[iLoop] != ',') && (inPara[iLoop] != '-') && (inPara[iLoop] != '\0')) {
            temp[i] = inPara[iLoop];
            i++;
            iLoop++;
            if (i >= AT_CSCB_MIDS_MAX_VALID_CHARACTERS_NUM) {
                return VOS_ERR;
            }
        }

        temp[i] = '\0';
        i++;

        if (VOS_StrLen((VOS_CHAR *)temp) == 0) {
            return VOS_ERR;
        }

        if (AT_ProcParseCbAuc2ul(&bRange, temp, &parmCount, cbRange) == VOS_ERR) {
            return VOS_ERR;
        }

        if (AT_SetbRangeFlgParseCb(inPara, iLoop, &bRange) == VOS_OK) {
            return VOS_OK;
        }

        iLoop++;

        if (parmCount >= AT_MAX_CBS_MSGID_NUM) {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_ProcCscbWithParm(TAF_UINT8 indexNum, TAF_CBA_CbmiRangeList *cbmirList)
{
    errno_t                memResult;
    VOS_UINT32             ret;
    TAF_CBA_CbmiRangeList *dcssInfo = VOS_NULL_PTR;
    VOS_UINT32             appRet;
    AT_ModemSmsCtx        *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(cbmirList, sizeof(TAF_CBA_CbmiRangeList), 0x00, sizeof(TAF_CBA_CbmiRangeList));

    ret = VOS_ERR;

    /* 表明 <mids> 有效 */
    if (g_atParaList[AT_CSCB_MIDS].paraLen > 0) {
        g_atParaList[AT_CSCB_MIDS].para[g_atParaList[AT_CSCB_MIDS].paraLen] = '\0';
        ret = AT_ParseCbParm(g_atParaList[AT_CSCB_MIDS].para, cbmirList);
    }

    /* 多个参数,MID必须存在,而且还必须合法,否则返回出错 */
    if (ret != VOS_OK) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 表明 <dcss> 有效 */
    if (g_atParaList[AT_CSCB_DCSS].paraLen > 0) {
        /*lint -save -e516 */
        dcssInfo = (TAF_CBA_CbmiRangeList *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_CBA_CbmiRangeList));
        /*lint -restore */
        if (dcssInfo == VOS_NULL) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        (VOS_VOID)memset_s(dcssInfo, sizeof(TAF_CBA_CbmiRangeList), 0x00, sizeof(TAF_CBA_CbmiRangeList));

        g_atParaList[AT_CSCB_DCSS].para[g_atParaList[AT_CSCB_DCSS].paraLen] = '\0';
        ret = AT_ParseCbParm(g_atParaList[AT_CSCB_DCSS].para, dcssInfo);
        if (ret == VOS_OK) {
            memResult = memcpy_s(&(smsCtx->cbsDcssInfo), sizeof(smsCtx->cbsDcssInfo), dcssInfo,
                                 sizeof(smsCtx->cbsDcssInfo));
            TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsCtx->cbsDcssInfo), sizeof(smsCtx->cbsDcssInfo));
        } else {
            /*lint -save -e516 */
            PS_MEM_FREE(WUEPS_PID_AT, dcssInfo);
            /*lint -restore */
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
        /*lint -save -e516 */
        PS_MEM_FREE(WUEPS_PID_AT, dcssInfo);
        /*lint -restore */
    }

    if (g_atParaList[0].paraValue == MN_MSG_CBMODE_ACCEPT) {
        appRet = MN_MSG_AddCbMids(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, cbmirList);
    } else {
        appRet = MN_MSG_DelCbMids(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, cbmirList);
    }

    ret = VOS_OK;
    /* 转换API调用结果 */
    if (appRet != MN_ERR_NO_ERROR) {
        ret = VOS_ERR;
    }

    if (ret != VOS_OK) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCscbPara(TAF_UINT8 indexNum)
{
    VOS_UINT32             ret;
    TAF_CBA_CbmiRangeList *cbmirList = VOS_NULL_PTR;
    VOS_UINT32             apiRet;
    TAF_CBA_MsgidSrcFlgUint8 midSrcFlg = TAF_CBA_MSGID_SRC_FLG_NULL;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 如果参数个数大于1,则第二个参数必须存在 */
    if ((g_atParaIndex > 1) && (g_atParaList[1].paraLen == 0)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }
    /*lint -save -e516 */
    cbmirList = (TAF_CBA_CbmiRangeList *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(TAF_CBA_CbmiRangeList));
    /*lint -restore */
    if (cbmirList == VOS_NULL_PTR) {
        AT_WARN_LOG("At_SetCscbPara: Fail to alloc memory.");
        return AT_CMS_ERROR_IN_MS;
    }
    (VOS_VOID)memset_s(cbmirList, sizeof(TAF_CBA_CbmiRangeList), 0x00, sizeof(TAF_CBA_CbmiRangeList));

    g_atClientTab[indexNum].opId = At_GetOpId();

    ret = AT_OK;
    if ((g_atParaIndex == 0) || ((g_atParaIndex == 1) && (g_atParaList[0].paraValue == MN_MSG_CBMODE_ACCEPT))) {
        /* 当前仅存在一个参数,表明是激活所有CBS消息 */
        cbmirList->cbmirNum                   = 1;
        cbmirList->cbmiRangeInfo[0].msgIdFrom = 0;
        cbmirList->cbmiRangeInfo[0].msgIdTo   = 0xFFFF;
        /* 用户操作的，需要刷新到卡和NV，所以文件标记为 NV + EFCBMI+ EFCBMIR */
        midSrcFlg = TAF_CBA_MSGID_SRC_FLG_NV;
        midSrcFlg |= TAF_CBA_MSGID_SRC_FLG_EFCBMI;
        midSrcFlg |= TAF_CBA_MSGID_SRC_FLG_EFCBMIR;

        cbmirList->cbmiRangeInfo[0].msgIdSrcFlg = midSrcFlg;

        apiRet = MN_MSG_AddCbMids(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, cbmirList);

        /* 转换API调用结果 */
        if (apiRet != MN_ERR_NO_ERROR) {
            ret = AT_CMS_OPERATION_NOT_ALLOWED;
        }
    } else if ((g_atParaIndex == 1) && (g_atParaList[0].paraValue == MN_MSG_CBMODE_REJECT)) {
        /* 当前仅存在一个参数,表明是去激活所有CBS消息 */
        apiRet = MN_MSG_EmptyCbMids(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId);

        /* 转换API调用结果 */
        if (apiRet != MN_ERR_NO_ERROR) {
            ret = AT_CMS_OPERATION_NOT_ALLOWED;
        }
    } else {
        /* 处理带有参数等情况 */
        ret = AT_ProcCscbWithParm(indexNum, cbmirList);
    }
    /*lint -save -e516 */
    PS_MEM_FREE(WUEPS_PID_AT, cbmirList);
    /*lint -restore */
    if (ret != AT_OK) {
        return ret;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSCB_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}
#endif

TAF_UINT32 At_SetCmgsPara(TAF_UINT8 indexNum)
{
    TAF_UINT8  smSFormat;
    TAF_UINT32 ret;

    smSFormat = AT_GetModemSmsCtxAddrFromClientId(indexNum)->cmgfMsgFormat;

    /* 参数检查，如果<da>号码或者<length>不存在，直接返回错误 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum].atSmsData, sizeof(g_atClientTab[indexNum].atSmsData), 0x00,
                       sizeof(g_atClientTab[indexNum].atSmsData));

    if (smSFormat == AT_CMGF_MSG_FORMAT_TEXT) {                 /* TEXT方式参数检查 */
        if (g_atParaIndex > AT_CMGS_TEXT_FORMAT_MAX_PARA_NUM) { /* 参数过多 */
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        /* 参数设置 */
        /* 设置<da> <toda> */
        ret = At_GetAsciiOrBcdAddr(g_atParaList[0].para, g_atParaList[0].paraLen, (TAF_UINT8)g_atParaList[1].paraValue,
                                   g_atParaList[1].paraLen, &g_atClientTab[indexNum].atSmsData.asciiAddr, TAF_NULL_PTR);
        if ((ret != AT_OK) || (g_atClientTab[indexNum].atSmsData.asciiAddr.len == 0)) {
            AT_NORM_LOG("At_SetCmgsPara: no da.");
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

    } else {                     /* PDU方式参数检查 */
        if (g_atParaIndex > 1) { /* 参数过多 */
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (g_atParaList[0].paraValue > AT_MSG_MAX_TPDU_LEN) {
            AT_NORM_LOG("At_SetCmgsPara: invalid tpdu data length.");
            return AT_ERROR;
        }

        /* 设置<length> */
        g_atClientTab[indexNum].atSmsData.pduLen = (TAF_UINT8)g_atParaList[0].paraValue;
    }

    At_SetCmdSubMode(indexNum, AT_SMS_MODE); /* 切换为短信模式 */

    /* 设置当前操作类型 */
    if (smSFormat == AT_CMGF_MSG_FORMAT_TEXT) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGS_TEXT_SET;
    } else {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGS_PDU_SET;
    }
    return AT_WAIT_SMS_INPUT; /* 返回提示短信内容输入状态 */
}

VOS_UINT32 AT_CheckCmgwFormatTextPara(VOS_VOID)
{
    /* 参数检查 */
    if ((g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) &&
        (g_atParaList[AT_CMGW_MSG_OA_OR_DA].paraLen == 0)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (g_atParaIndex > AT_CMGW_TEXT_FORMAT_MAX_PARA_NUM) { /* 参数过多 */
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckCmgwFormatPduPara(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParaList[AT_CMGW_TPDU_LEN].paraLen == 0) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (g_atParaIndex > AT_CMGW_PDU_FORMAT_MAX_PARA_NUM) { /* 参数过多 */
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    if (g_atParaList[AT_CMGW_TPDU_LEN].paraValue > AT_MSG_MAX_TPDU_LEN) {
        AT_NORM_LOG("At_SendPduMsgOrCmd: invalid tpdu data length.");
        return AT_ERROR;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckSmsState(TAF_UINT8 indexNum)
{
    if ((g_atClientTab[indexNum].atSmsData.smState != MN_MSG_STATUS_MT_NOT_READ) &&
        (g_atClientTab[indexNum].atSmsData.smState != MN_MSG_STATUS_MT_READ) &&
        (g_atClientTab[indexNum].atSmsData.smState != MN_MSG_STATUS_MO_NOT_SENT) &&
        (g_atClientTab[indexNum].atSmsData.smState != MN_MSG_STATUS_MO_SENT)) {
        AT_NORM_LOG("AT_CheckSmsState: invalid sm status.");
        return AT_ERROR;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCmgwPara(TAF_UINT8 indexNum)
{
    TAF_UINT8       smSFormat;
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smSFormat = smsCtx->cmgfMsgFormat;

    /* 初始化 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum].atSmsData, sizeof(g_atClientTab[indexNum].atSmsData), 0x00,
                       sizeof(g_atClientTab[indexNum].atSmsData));
    if (smSFormat == AT_CMGF_MSG_FORMAT_TEXT) { /* TEXT方式参数检查 */
        ret = AT_CheckCmgwFormatTextPara();
        if (ret != AT_OK) {
            return ret;
        }

        /* 设置<oa/da> */
        ret = At_GetAsciiOrBcdAddr(g_atParaList[AT_CMGW_MSG_OA_OR_DA].para, g_atParaList[AT_CMGW_MSG_OA_OR_DA].paraLen,
                                   (TAF_UINT8)g_atParaList[AT_CMGW_MSG_TOOA_OR_TODA].paraValue,
                                   g_atParaList[AT_CMGW_MSG_TOOA_OR_TODA].paraLen,
                                   &g_atClientTab[indexNum].atSmsData.asciiAddr,TAF_NULL_PTR);
        if (ret != AT_OK) {
            return ret;
        }

        /* 设置<stat> */
        if (g_atParaList[AT_CMGW_MSG_TEXT_STAT].paraLen != 0) {
            g_atClientTab[indexNum].atSmsData.smState = (TAF_UINT8)g_atParaList[AT_CMGW_MSG_TEXT_STAT].paraValue;
        } else {
            g_atClientTab[indexNum].atSmsData.smState = MN_MSG_STATUS_MO_NOT_SENT;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGW_TEXT_SET;
    } else { /* PDU方式参数检查 */
        ret = AT_CheckCmgwFormatPduPara();
        if (ret != AT_OK) {
            return ret;
        }

        g_atClientTab[indexNum].atSmsData.pduLen = (TAF_UINT8)g_atParaList[AT_CMGW_TPDU_LEN].paraValue;

        /* 设置<stat> */
        if (g_atParaList[AT_CMGW_MSG_PDU_STAT].paraLen != 0) {
            g_atClientTab[indexNum].atSmsData.smState = (TAF_UINT8)g_atParaList[AT_CMGW_MSG_PDU_STAT].paraValue;
        } else {
            g_atClientTab[indexNum].atSmsData.smState = MN_MSG_STATUS_MO_NOT_SENT;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGW_PDU_SET;
    }

    if (AT_CheckSmsState(indexNum) == AT_ERROR) {
        return AT_ERROR;
    }

    At_SetCmdSubMode(indexNum, AT_SMS_MODE); /* 切换为短信模式 */

    return AT_WAIT_SMS_INPUT; /* 返回提示短信内容输入状态 */
}

TAF_UINT32 At_SetCmgrPara(TAF_UINT8 indexNum)
{
    MN_MSG_ReadParm readParm;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParaList[AT_CMGR_MSG_INDEX].paraLen == 0) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&readParm, sizeof(readParm), 0x00, sizeof(readParm));

    /* 参数设置 */
    readParm.index = g_atParaList[AT_CMGR_MSG_INDEX].paraValue;

    /* 设置读取存储器 */
    readParm.memStore = smsCtx->cpmsInfo.memReadorDelete;

    if (smsCtx->paraCmsr == 0) {
        readParm.changeFlag = TAF_TRUE;
    } else {
        readParm.changeFlag = TAF_FALSE;
        if (g_atParaIndex > 1) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }

    /* 执行命令操作 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_Read(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &readParm) == MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGR_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCmglPara(TAF_UINT8 indexNum)
{
    MN_MSG_ListParm listParm;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if ((g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) && (g_atParaList[0].paraLen == 0)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* at命令只支持1个参数，增加检查 */
    if (g_atParaIndex > 1) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&listParm, sizeof(listParm), 0x00, sizeof(listParm));

    listParm.isFirstTimeReq = VOS_TRUE;

    /* 设置<stat> */
    if (g_atParaList[AT_CMGL_MSG_STAT].paraLen != 0) {
        listParm.status = (TAF_UINT8)g_atParaList[AT_CMGL_MSG_STAT].paraValue;
    } else {
        listParm.status = (TAF_UINT8)MN_MSG_STATUS_MT_NOT_READ;
    }

    /* 设置读取存储器 */
    listParm.memStore = smsCtx->cpmsInfo.memReadorDelete;

    if (smsCtx->paraCmsr == 0) {
        listParm.changeFlag = TAF_TRUE;
    } else {
        listParm.changeFlag = TAF_FALSE;
        if (g_atParaIndex > 1) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    }

    /* 执行命令操作 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_List(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &listParm) == MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCmgcPara(TAF_UINT8 indexNum)
{
    TAF_UINT8       smSFormat;
    TAF_UINT32      ret;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smSFormat = smsCtx->cmgfMsgFormat;

    /* 初始化 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum].atSmsData, sizeof(g_atClientTab[indexNum].atSmsData), 0x00,
                       sizeof(g_atClientTab[indexNum].atSmsData));

    if (smSFormat == AT_CMGF_MSG_FORMAT_TEXT) { /* TEXT方式 */
        /* 如果<ct>或<fo>不存在，直接返回错误 */
        if ((g_atParaList[AT_CMGC_MSG_CMD_TYPE].paraLen == 0) || (g_atParaList[AT_CMGC_MSG_FO].paraLen == 0)) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (g_atParaIndex > AT_CMGC_MAX_PARA_NUM) { /* 参数过多 */
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        /* 设置<fo>.SRR */
        g_atClientTab[indexNum].atSmsData.fo = (TAF_UINT8)g_atParaList[AT_CMGC_MSG_FO].paraValue;

        /* 设置<ct> */
        g_atClientTab[indexNum].atSmsData.commandType = (TAF_UINT8)g_atParaList[AT_CMGC_MSG_CMD_TYPE].paraValue;

        /* 设置<pid> */
        if (g_atParaList[AT_CMGC_PID].paraLen != 0) {
            g_atClientTab[indexNum].atSmsData.pid = (TAF_UINT8)g_atParaList[AT_CMGC_PID].paraValue;
        } else {
            g_atClientTab[indexNum].atSmsData.pid = 0;
        }

        /* 设置<mn> */
        if (g_atParaList[AT_CMGC_MSG_NO].paraLen == 0) {
            g_atClientTab[indexNum].atSmsData.messageNumber = 0;
        } else {
            g_atClientTab[indexNum].atSmsData.messageNumber = (TAF_UINT8)g_atParaList[AT_CMGC_MSG_NO].paraValue;
        }

        /* 设置<oa/da> */
        ret = At_GetAsciiOrBcdAddr(g_atParaList[AT_CMGC_MSG_OA_OR_DA].para, g_atParaList[AT_CMGC_MSG_OA_OR_DA].paraLen,
                                   (TAF_UINT8)g_atParaList[AT_CMGC_MSG_TOOA_OR_TODA].paraValue,
                                   g_atParaList[AT_CMGC_MSG_TOOA_OR_TODA].paraLen,
                                   &g_atClientTab[indexNum].atSmsData.asciiAddr, TAF_NULL_PTR);
        if (ret != AT_OK) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGC_TEXT_SET;
    } else { /* PDU方式 */
        /* 如果<length>不存在，直接返回错误 */
        if (g_atParaList[AT_CMGC_PDU_LEN].paraLen == 0) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (g_atParaIndex > 1) { /* 参数过多 */
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }

        if (g_atParaList[AT_CMGC_PDU_LEN].paraValue > AT_MSG_MAX_TPDU_LEN) {
            AT_NORM_LOG("At_SetCmgcPara: invalid tpdu data length.");
            return AT_ERROR;
        }

        g_atClientTab[indexNum].atSmsData.pduLen = (TAF_UINT8)g_atParaList[AT_CMGC_PDU_LEN].paraValue;
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGC_PDU_SET;
    }

    At_SetCmdSubMode(indexNum, AT_SMS_MODE); /* 切换为短信模式 */

    return AT_WAIT_SMS_INPUT; /* 返回提示短信内容输入状态 */
}

TAF_UINT32 At_SetCmssPara(TAF_UINT8 indexNum)
{
    MN_MSG_SendFrommemParm sendFromMemParm;
    TAF_UINT32             ret;
    AT_ModemSmsCtx        *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查，如果<index>不存在，直接返回错误 */
    if ((g_atParaList[AT_CMSS_MSG_INDEX].paraLen == 0) || (g_atParaIndex > AT_CMSS_MAX_PARA_NUM)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 设置<da> */
    ret = At_GetAsciiOrBcdAddr(g_atParaList[AT_CMSS_MSG_DA].para, g_atParaList[AT_CMSS_MSG_DA].paraLen,
                               (TAF_UINT8)g_atParaList[AT_CMSS_MSG_TODA].paraValue,
                               g_atParaList[AT_CMSS_MSG_TODA].paraLen, VOS_NULL_PTR, &sendFromMemParm.destAddr);
    if (ret != AT_OK) {
        return AT_ERROR;
    }

    /* 设置发送存储器 */
    sendFromMemParm.memStore = smsCtx->cpmsInfo.memSendorWrite;
    sendFromMemParm.index    = g_atParaList[AT_CMSS_MSG_INDEX].paraValue;

    /* 执行命令操作 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_SendFromMem(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &sendFromMemParm) ==
        MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt          = AT_CMD_CMSS_SET;
        g_atClientTab[indexNum].atSmsData.msgSentSmNum = 1;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ExecuteSendAckParm(TAF_BOOL bRpAck, TAF_UINT8 indexNum)
{
    MN_MSG_SendAckParm *ackParm = VOS_NULL_PTR;
    TAF_UINT32          ret;

    /* CMGF设置了文本格式, 或CNMA无参数，或CNMA参数中<length>为0 */
    ackParm = At_GetAckMsgMem();
    (VOS_VOID)memset_s(ackParm, sizeof(MN_MSG_SendAckParm), 0x00, sizeof(MN_MSG_SendAckParm));

    if (bRpAck == TAF_TRUE) {
        ackParm->rpAck              = TAF_TRUE;
        ackParm->tsRawData.tpduType = MN_MSG_TPDU_DELIVER_RPT_ACK;
    } else {
        ackParm->rpAck              = TAF_FALSE;
        ackParm->rpCause            = MN_MSG_RP_CAUSE_PROTOCOL_ERR_UNSPECIFIED;
        ackParm->tsRawData.tpduType = MN_MSG_TPDU_DELIVER_RPT_ERR;
        ackParm->tsRawData.len      = AT_MN_MSG_SMS_TPDU_LEN;
        ackParm->tsRawData.data[0]  = 0;
        ackParm->tsRawData.data[1]  = 0xff;
    }

    /* 执行命令操作 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_SendAck(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, ackParm) == MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNMA_TEXT_SET;
        ret                                   = AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        ret = AT_ERROR;
    }

    return ret;
}

TAF_UINT32 At_SetCnmaPara(TAF_UINT8 indexNum)
{
    TAF_BOOL            bRpAck  = TAF_TRUE;
    AT_ModemSmsCtx     *smsCtx  = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    if (g_atParaIndex > AT_CNMA_MAX_PARA_NUM) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&g_atClientTab[indexNum].atSmsData, sizeof(g_atClientTab[indexNum].atSmsData), 0x00,
                       sizeof(g_atClientTab[indexNum].atSmsData));
    if (smsCtx->cmgfMsgFormat == AT_CMGF_MSG_FORMAT_TEXT) { /* TEXT方式 */
        if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
            return AT_CMS_OPERATION_NOT_ALLOWED;
        }
    } else { /* PDU方式 */
        if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
            if (g_atParaList[AT_CNMA_PDU_N_TYPE].paraLen == 0) {
                return AT_CMS_OPERATION_NOT_ALLOWED;
            } else {
                /* 设置<n> */
                g_atClientTab[indexNum].atSmsData.cnmaType = (TAF_UINT8)g_atParaList[AT_CNMA_PDU_N_TYPE].paraValue;
            }

            /* 只有参数<n>,决定是否发送RP-ACK或RP-ERROR */
            if ((g_atParaList[AT_CNMA_TPDU_LEN].paraLen != 0) && (g_atParaList[AT_CNMA_TPDU_LEN].paraValue != 0)) {
                if (g_atClientTab[indexNum].atSmsData.cnmaType == 0) {
                    return AT_ERROR;
                }

                /* 设置<length> */
                if (g_atParaList[AT_CNMA_TPDU_LEN].paraValue > AT_MSG_MAX_TPDU_LEN) {
                    AT_NORM_LOG("At_SetCnmaPara: the length of PDU is not support.");
                    return AT_CMS_OPERATION_NOT_ALLOWED; /* 输入字串太长 */
                }

                g_atClientTab[indexNum].atSmsData.pduLen = (TAF_UINT8)g_atParaList[AT_CNMA_TPDU_LEN].paraValue;
                At_SetCmdSubMode(indexNum, AT_SMS_MODE);/* 切换为短信模式 */
                /* 设置当前操作类型 */
                g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CNMA_PDU_SET;
                return AT_WAIT_SMS_INPUT; /* 返回提示短信内容输入状态 */
            } else {
                if (g_atClientTab[indexNum].atSmsData.cnmaType == AT_CNMA_RP_ERROR) {
                    bRpAck = TAF_FALSE;
                }
            }
        }
    }

    return AT_ExecuteSendAckParm(bRpAck, indexNum);
}

TAF_UINT32 At_SetCmstPara(TAF_UINT8 indexNum)
{
    MN_MSG_SendFrommemParm sendFromMemParm;
    TAF_UINT32             ret;
    TAF_UINT32             loop;
    MN_OPERATION_ID_T      opId   = At_GetOpId();
    AT_ModemSmsCtx        *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查，如果<index>不存在，直接返回错误 */
    if ((g_atParaList[AT_CMST_MSG_INDEX].paraLen == 0) ||
        (g_atParaList[AT_CMST_MSG_SENT_SM_TIME].paraLen == 0) || (g_atParaIndex > AT_CMST_MAX_PARA_NUM) ||
        (g_atParaList[AT_CMST_MSG_SENT_SM_TIME].paraValue < 1)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 设置<da> */
    ret = At_GetAsciiOrBcdAddr(g_atParaList[AT_CMST_MSG_DA].para, g_atParaList[AT_CMST_MSG_DA].paraLen,
                               (TAF_UINT8)g_atParaList[AT_CMST_MSG_TODA].paraValue,
                               g_atParaList[AT_CMST_MSG_TODA].paraLen, VOS_NULL_PTR, &sendFromMemParm.destAddr);
    if (ret != AT_OK) {
        return AT_ERROR;
    }

    /* 设置发送存储器 */
    sendFromMemParm.memStore = smsCtx->cpmsInfo.memSendorWrite;
    sendFromMemParm.index    = g_atParaList[AT_CMST_MSG_INDEX].paraValue;

    /* 执行命令操作 */
    g_atClientTab[indexNum].atSmsData.msgSentSmNum = (TAF_UINT8)g_atParaList[AT_CMST_MSG_SENT_SM_TIME].paraValue;
    for (loop = 0; loop < g_atClientTab[indexNum].atSmsData.msgSentSmNum; loop++) {
        if (MN_MSG_SendFromMem(g_atClientTab[indexNum].clientId, opId, &sendFromMemParm) != AT_SUCCESS) {
            return AT_ERROR;
        }
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMST_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

TAF_UINT32 At_SetCmsrPara(TAF_UINT8 indexNum)
{
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 参数设置 */
    smsCtx->paraCmsr = (VOS_UINT8)g_atParaList[0].paraValue;

    return AT_OK;
}

TAF_UINT32 At_SetCmgiPara(TAF_UINT8 indexNum)
{
    MN_MSG_StatusTypeUint8 msgStatus;
    MN_MSG_ListParm        listPara;
    AT_ModemSmsCtx        *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&listPara, sizeof(MN_MSG_ListParm), 0x00, sizeof(MN_MSG_ListParm));

    /* 参数检查 */
    if (g_atParaIndex > 1) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 设置<stat> */
    if (g_atParaList[0].paraLen == 0) {
        return AT_ERROR;
    } else {
        msgStatus = (VOS_UINT8)g_atParaList[0].paraValue;
    }

    listPara.changeFlag = VOS_FALSE;
    listPara.memStore   = smsCtx->cpmsInfo.memReadorDelete;
    listPara.status     = msgStatus;
    (VOS_VOID)memset_s(listPara.reserve1, sizeof(listPara.reserve1), 0x00, sizeof(listPara.reserve1));

    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_Delete_Test(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &listPara) ==
        MN_ERR_NO_ERROR) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMGI_SET;
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCmmtPara(TAF_UINT8 indexNum)
{
    MN_MSG_ModifyStatusParm modifyParm;
    AT_ModemSmsCtx         *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    /* 参数过多 */
    if (g_atParaIndex > AT_CMMT_PARA_MAX_NUM) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 参数检查 */
    if ((g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) && (g_atParaList[0].paraLen == 0)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&modifyParm, sizeof(modifyParm), 0x00, sizeof(modifyParm));

    modifyParm.index = g_atParaList[0].paraValue;

    /* 设置<stat> */
    if (g_atParaList[1].paraValue == 0) {
        modifyParm.status = MN_MSG_STATUS_MT_NOT_READ;
    } else {
        modifyParm.status = MN_MSG_STATUS_MT_READ;
    }

    /* 设置读取存储器 */
    modifyParm.memStore = smsCtx->cpmsInfo.memReadorDelete;

    /* 执行命令操作 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    if (MN_MSG_ModifyStatus(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &modifyParm) ==
        MN_ERR_NO_ERROR) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMMT_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

TAF_UINT32 At_SetCpmsPara(TAF_UINT8 indexNum)
{
    MN_MSG_SetRcvmsgPathParm rcvPath;
    AT_ModemSmsCtx          *smsCtx = VOS_NULL_PTR;
    errno_t                  memResult;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&rcvPath, sizeof(rcvPath), 0x00, sizeof(MN_MSG_SetRcvmsgPathParm));
    /* 参数检查 */
    if ((g_atParaIndex > AT_CPMS_MAX_PARA_NUM) || (g_atParaIndex == 0) ||
        (g_atParaList[AT_CPMS_MEM_READOR_DELETE].paraLen == 0)) {
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    smsCtx->cpmsInfo.tmpMemReadorDelete = (MN_MSG_MemStoreUint8)(g_atParaList[AT_CPMS_MEM_READOR_DELETE].paraValue + 1);

    if (g_atParaList[AT_CPMS_MEM_SENDOR_WRITE].paraLen != 0) {
        smsCtx->cpmsInfo.tmpMemSendorWrite =
            (MN_MSG_MemStoreUint8 )(g_atParaList[AT_CPMS_MEM_SENDOR_WRITE].paraValue + 1);
    } else {
        smsCtx->cpmsInfo.tmpMemSendorWrite = smsCtx->cpmsInfo.memSendorWrite;
    }

    /* 执行命令操作 */
    g_atClientTab[indexNum].opId = At_GetOpId();
    memResult                    = memcpy_s(&rcvPath, sizeof(rcvPath), &(smsCtx->cpmsInfo.rcvPath), sizeof(rcvPath));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(rcvPath), sizeof(rcvPath));

    /* 将CNMI设置的MT type类型传输给TAF,在<MT>=3，收到CLASS3短信时候，按照CMT方式上报 */
    rcvPath.cnmiMtType = smsCtx->cnmiType.cnmiMtType;

    if (g_atParaList[AT_CPMS_MEM_STORE].paraLen != 0) {
        rcvPath.staRptMemStore = (MN_MSG_MemStoreUint8 )(g_atParaList[AT_CPMS_MEM_STORE].paraValue + 1);
        rcvPath.smMemStore     = (MN_MSG_MemStoreUint8 )(g_atParaList[AT_CPMS_MEM_STORE].paraValue + 1);
    }

    if ((smsCtx->cpmsInfo.tmpMemReadorDelete == MN_MSG_MEM_STORE_ME) ||
        (smsCtx->cpmsInfo.tmpMemSendorWrite == MN_MSG_MEM_STORE_ME) || (rcvPath.smMemStore == MN_MSG_MEM_STORE_ME)) {
#if (NAS_FEATURE_SMS_FLASH_SMSEXIST != FEATURE_ON)
        return AT_ERROR;
#else
        if (smsCtx->msgMeStorageStatus != MN_MSG_ME_STORAGE_ENABLE) {
            return AT_ERROR;
        }
#endif
    }

    if (MN_MSG_SetRcvMsgPath(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &rcvPath) !=
        MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].atSmsData.waitForCpmsSetRsp = TAF_TRUE;

    if (At_GetSmsStorage(indexNum, smsCtx->cpmsInfo.tmpMemReadorDelete, smsCtx->cpmsInfo.tmpMemSendorWrite,
                         rcvPath.smMemStore) != AT_OK) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPMS_SET;
    return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
}

TAF_UINT32 AT_StubSendAutoReplyMsg(VOS_UINT8 indexNum, MN_MSG_EventInfo *event, MN_MSG_TsDataInfo *tsDataInfo)
{
    MN_MSG_SendParm    sendMsg;
    MN_MSG_TsDataInfo *tsSubmitInfo = VOS_NULL_PTR;
    MN_MSG_Submit     *submit       = VOS_NULL_PTR;
    TAF_UINT32         ret;
    errno_t            memResult;

    /* 1. 为自动回复消息SUBMIT申请内存并情况 */
    tsSubmitInfo = (MN_MSG_TsDataInfo *)PS_MEM_ALLOC(WUEPS_PID_AT, sizeof(MN_MSG_TsDataInfo));
    if (tsSubmitInfo == VOS_NULL_PTR) {
        return AT_ERROR;
    }
    (VOS_VOID)memset_s(tsSubmitInfo, sizeof(MN_MSG_TsDataInfo), 0x00, sizeof(MN_MSG_TsDataInfo));

    /* 2. 为自动回复消息SUBMIT填写TPDU数据内容 */
    tsSubmitInfo->tpduType = MN_MSG_TPDU_SUBMIT;
    submit                 = (MN_MSG_Submit *)&tsSubmitInfo->u.submit;
    memResult              = memcpy_s(&submit->destAddr, sizeof(submit->destAddr), &tsDataInfo->u.deliver.origAddr,
                                      sizeof(tsDataInfo->u.deliver.origAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->destAddr), sizeof(tsDataInfo->u.deliver.origAddr));
    memResult = memcpy_s(&submit->dcs, sizeof(submit->dcs), &tsDataInfo->u.deliver.dcs,
            sizeof(tsDataInfo->u.deliver.dcs));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(submit->dcs), sizeof(tsDataInfo->u.deliver.dcs));
    submit->validPeriod.validPeriod = MN_MSG_VALID_PERIOD_NONE;

    /* 3. 为自动回复消息SUBMIT编码 */
    ret = MN_MSG_Encode(tsSubmitInfo, &sendMsg.msgInfo.tsRawData);
    if (ret != MN_ERR_NO_ERROR) {
        /*lint -save -e830 */
        PS_MEM_FREE(WUEPS_PID_AT, tsSubmitInfo);
        /*lint -restore */
        return AT_ERROR;
    }

    /* 4. 填写回复消息的短信中心, 存储设备，消息类型和发送域 */
    sendMsg.memStore                   = MN_MSG_MEM_STORE_NONE;
    sendMsg.clientType                 = MN_MSG_CLIENT_NORMAL;
    sendMsg.msgInfo.tsRawData.tpduType = MN_MSG_TPDU_SUBMIT;
    memResult = memcpy_s(&sendMsg.msgInfo.scAddr, sizeof(sendMsg.msgInfo.scAddr),
                         &event->u.deliverInfo.rcvMsgInfo.scAddr, sizeof(event->u.deliverInfo.rcvMsgInfo.scAddr));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(sendMsg.msgInfo.scAddr), sizeof(event->u.deliverInfo.rcvMsgInfo.scAddr));

    /* 5. 发送回复消息 */
    ret = MN_MSG_Send(indexNum, 0, &sendMsg);
    if (ret != MN_ERR_NO_ERROR) {
        ret = AT_ERROR;
    } else {
        ret = AT_OK;
    }
    PS_MEM_FREE(WUEPS_PID_AT, tsSubmitInfo);

    return ret;
}

TAF_VOID AT_StubTriggerAutoReply(VOS_UINT8 indexNum, TAF_UINT8 cfgValue)
{
    TAF_UINT8       loop;
    AT_ModemSmsCtx *smsCtx = VOS_NULL_PTR;

    smsCtx = AT_GetModemSmsCtxAddrFromClientId(indexNum);

    smsCtx->smsAutoReply = cfgValue;

    /* 若关闭自动回复功能，则清空相关动态内存 */
    if (smsCtx->smsAutoReply == 0) {
        for (loop = 0; loop < AT_SMSMT_BUFFER_MAX; loop++) {
            AT_StubClearSpecificAutoRelyMsg(indexNum, loop);
            smsCtx->smsMtBuffer[loop].used = TAF_FALSE;
        }

        return;
    }

    /* 若已启用自动回复功能，按顺序回复接收到的短信 */
    for (loop = 0; loop < AT_SMSMT_BUFFER_MAX; loop++) {
        if (smsCtx->smsMtBuffer[loop].used != TAF_TRUE) {
            AT_StubClearSpecificAutoRelyMsg(indexNum, loop);
            continue;
        }

        if ((smsCtx->smsMtBuffer[loop].event == VOS_NULL_PTR) ||
            (smsCtx->smsMtBuffer[loop].tsDataInfo == VOS_NULL_PTR)) {
            AT_StubClearSpecificAutoRelyMsg(indexNum, loop);
            smsCtx->smsMtBuffer[loop].used = TAF_FALSE;
            continue;
        }

        AT_StubSendAutoReplyMsg(indexNum, smsCtx->smsMtBuffer[loop].event, smsCtx->smsMtBuffer[loop].tsDataInfo);
        AT_StubClearSpecificAutoRelyMsg(indexNum, loop);
        smsCtx->smsMtBuffer[loop].used = TAF_FALSE;
        break;
    }
}

VOS_UINT32 At_SetCmStubPara(VOS_UINT8 indexNum)
{
    MN_MSG_StubMsg stubParam;
    VOS_UINT32     ret;

    /* 输入参数检查: <stubtype>类型是否有效，参数个数不允许超过2个 */
    if ((g_atParaIndex != 2) || (g_atParaList[AT_CMSTUB_STUB_TYPE].paraLen == 0) ||
        (g_atParaList[AT_CMSTUB_STUB_VALUE].paraLen == 0)) {
        AT_LOG1("At_SetCmStubPara: invalid parameter.", g_atParaIndex);
        AT_LOG1("At_SetCmStubPara: invalid parameter.", g_atParaList[AT_CMSTUB_STUB_TYPE].paraValue);
        return AT_CMS_OPERATION_NOT_ALLOWED;
    }

    /* A核能处理的桩直接处理并退出: 自动回复功能不需要通知C核 */
    if (g_atParaList[AT_CMSTUB_STUB_TYPE].paraValue == MN_MSG_STUB_TYPE_SMS_AUTO_REPLY) {
        AT_StubTriggerAutoReply(indexNum, (VOS_UINT8)g_atParaList[AT_CMSTUB_STUB_VALUE].paraValue);
        return AT_OK;
    }

#if (VOS_WIN32 == VOS_OS_VER)
    if (g_atParaList[AT_CMSTUB_STUB_TYPE].paraValue == MN_MSG_STUB_TYPE_CLASS0_TAILOR) {
        /* CLASS0类短信接收方式涉及A核和C核两部分，所以此处不能直接返回； */
        g_class0Tailor = (VOS_UINT8)g_atParaList[AT_CMSTUB_STUB_VALUE].paraValue;
    }
#endif

    /* 需要C核处理的桩: 发送消息给C核 */
    stubParam.stubType = (MN_MSG_StubTypeUint32 )g_atParaList[AT_CMSTUB_STUB_TYPE].paraValue;
    stubParam.value    = g_atParaList[AT_CMSTUB_STUB_VALUE].paraValue;

    g_atClientTab[indexNum].opId = At_GetOpId();
    ret = MN_MSG_ReqStub(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &stubParam);
    if (ret != MN_ERR_NO_ERROR) {
        return ret;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CMSTUB_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetMemStatusPara(VOS_UINT8 indexNum)
{
    MN_MSG_SetMemstatusParm memStatus;

    /* 初始化 */
    (VOS_VOID)memset_s(&memStatus, sizeof(memStatus), 0x00, sizeof(MN_MSG_SetMemstatusParm));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置参数 */
    if (g_atParaList[0].paraValue == 0) {
        memStatus.memFlag = MN_MSG_MEM_FULL_SET;
    } else {
        memStatus.memFlag = MN_MSG_MEM_FULL_UNSET;
    }

    /* 调用MN消息API发送设置请求给MSG模块 */
    if (MN_MSG_SetMemStatus(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, &memStatus) !=
        MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSASM_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetSmsDomainPara(VOS_UINT8 indexNum)
{
    AT_MTA_SmsDomainSetReq smsDomain;
    VOS_UINT32             rst;

    (VOS_VOID)memset_s(&smsDomain, sizeof(smsDomain), 0x00, sizeof(smsDomain));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数及长度检查 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    smsDomain.smsDomain = (AT_MTA_SmsDomainUint8)g_atParaList[0].paraValue;

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_SMS_DOMAIN_SET_REQ, (VOS_UINT8 *)&smsDomain, sizeof(smsDomain),
                                 I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetSmsDomainPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SMSDOMAIN_SET;

    return AT_WAIT_ASYNC_RETURN;
}

/* 短信\语音Trust名单号码合法性校验 */
VOS_UINT32 AT_CheckTrustNumPara(const VOS_UINT8 *data, VOS_UINT16 len)
{
    VOS_UINT16 count = 0;
    const VOS_UINT8 *temp = VOS_NULL_PTR;

    temp = data;
    /* 第一位是'+'时，允许的号码最大长度+1 */
    if (*temp == '+') {
        if (len > (AT_TRUSTLIST_NUM_PARA_MAX_LEN + 1)) {
            return AT_CME_DIAL_STRING_TOO_LONG;
        }
        temp++;
        count++;
    } else {
        if (len > AT_TRUSTLIST_NUM_PARA_MAX_LEN) {
            return AT_CME_DIAL_STRING_TOO_LONG;
        }
    }

    /* 检查是否输入合法字符 */
    for (; count < len; count++) {
        if (((*temp >= '0') && (*temp <= '9')) || (*temp == '*') || (*temp == '#')) {
            temp++;
        } else {
            return AT_CME_INVALID_CHARACTERS_IN_DIAL_STRING;
        }
    }
    return AT_SUCCESS;
}

/* 更新指定索引短信\语音Trust名单号码 */
VOS_UINT32 AT_TrustNumParaProc(TAF_NVIM_NumType *recordNumType)
{
    VOS_UINT32 result;

    /* 输入号码为空，表示删除指定索引Trust名单号码 */
    if (g_atParaList[1].paraLen == 0) {
        recordNumType->index = 0;
        (VOS_VOID)memset_s(recordNumType->number, sizeof(recordNumType->number), 0, sizeof(recordNumType->number));
    } else {
        /* 短信/语音Trust名单号码合法性检查 */
        result = AT_CheckTrustNumPara(g_atParaList[1].para, g_atParaList[1].paraLen);
        if (result != AT_SUCCESS) {
            return result;
        }
    }
    return AT_SUCCESS;
}

/* AT^TRUSTNUM设置命令处理函数 */
VOS_UINT32 AT_SetSmsTrustNumPara(VOS_UINT8 indexNum)
{
    TAF_NVIM_MtSmsNumTrustList smsTrustList;
    VOS_UINT32                 listIndex;
    VOS_UINT32                 result;
    errno_t                    memResult;

    /* 参数合法性检查 */
    if ((g_atParaIndex > AT_SET_SMS_TRUST_NUM_MAX_PARA_NUM) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 从NV中读取短信Trust名单列表 */
    (VOS_VOID)memset_s(&smsTrustList, sizeof(smsTrustList), 0, sizeof(smsTrustList));

    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_MT_SMS_NUM_TRUST_LIST, &smsTrustList, sizeof(smsTrustList)) != NV_OK) {
        AT_ERR_LOG("AT_SetSmsTrustNumPara: Read NV fail.");
        return AT_ERROR;
    }

    listIndex = g_atParaList[0].paraValue;
    /* 参数个数为1个时，表示读取对应索引的短信Trust名单号码 */
    if (g_atParaIndex == 1) {
        if (VOS_StrLen((VOS_CHAR *)smsTrustList.recordList[listIndex].number) != 0) {
            g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,\"%s\"", g_parseContext[indexNum].cmdElement->cmdName,
                listIndex, smsTrustList.recordList[listIndex].number);
            return AT_OK;
        } else {
            AT_INFO_LOG("The sms trustlist record of the specified index is empty.");
            return AT_OK;
        }
    } else {
        /* 参数个数为2个时,Trust名单号码合法性检查 */
        result = AT_TrustNumParaProc(&smsTrustList.recordList[listIndex]);
        if (result != AT_SUCCESS) {
            return result;
        }
        /* Trust名单号码重复性检查 */
        if (AT_CheckDeduplicationTrustNum(smsTrustList.recordList, TAF_TRUSTLIST_MAX_LIST_SIZE) == VOS_TRUE) {
            return AT_OK;
        }
        /* 更新指定索引的Trust名单号码 */
        smsTrustList.recordList[listIndex].index = (VOS_UINT8)g_atParaList[0].paraValue;
        (VOS_VOID)memset_s(smsTrustList.recordList[listIndex].number, sizeof(smsTrustList.recordList[listIndex].number),
                           0x00, sizeof(smsTrustList.recordList[listIndex].number));

        memResult = memcpy_s(smsTrustList.recordList[listIndex].number,
                             sizeof(smsTrustList.recordList[listIndex].number),
                             g_atParaList[1].para,
                             g_atParaList[1].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(smsTrustList.recordList[listIndex].number), g_atParaList[1].paraLen);
    }
    /* 更新短信Trust名单NV */
    result = TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_MT_SMS_NUM_TRUST_LIST, (VOS_UINT8 *)(&smsTrustList), sizeof(smsTrustList));
    if (result != NV_OK) {
        AT_ERR_LOG("AT_SetSmsTrustNumPara: Write NV fail.");
        return AT_ERROR;
    }
    return AT_OK;
}

#if (FEATURE_MBB_CUST == FEATURE_ON)
/*
 * 功能说明: WebSDK通知Modem侧应用本地短信数据库无短信空间或者有短信空间
 * 输入参数: index: AT下发通道
 * 返回结果: AT_WAIT_ASYNC_RETURN: 异步等待状态
 *           AT_ERROR: 执行失败
 */
VOS_UINT32 AT_SetHsmfPara(VOS_UINT8 indexNum)
{
    MN_MSG_SetMemstatusParm memStatus;

    /* 初始化 */
    (VOS_VOID)memset_s(&memStatus, sizeof(memStatus), 0x00, sizeof(memStatus));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 设置参数 */
    if (g_atParaList[0].paraValue == 0) {
        /* 对于HSMF，0代表未满 */
        memStatus.memFlag = MN_MSG_MEM_FULL_UNSET;
    } else {
        memStatus.memFlag = MN_MSG_MEM_FULL_SET;
    }

    /* 调用MN消息API发送设置请求给MSG模块 */
    if (MN_MSG_SetMemStatus(g_atClientTab[indexNum].clientId,
                            g_atClientTab[indexNum].opId,
                            &memStatus) != MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HSMF_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif
