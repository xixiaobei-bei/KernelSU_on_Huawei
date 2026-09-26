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
#include "at_voice_ims_rslt_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "taf_type_def.h"
#include "at_data_proc.h"
#include "at_device_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_IMS_RSLT_PROC_C

#if (FEATURE_IMS == FEATURE_ON)
VOS_UINT32 AT_RcvMmaImsDomainCfgSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ImsDomainCfgSetCnf *domainCfgSetCnf = (TAF_MMA_ImsDomainCfgSetCnf *)msg;

    g_atSendDataBuff.bufLen = 0;

    if (domainCfgSetCnf->imsDomainCfgResult == TAF_MMA_IMS_DOMAIN_CFG_RESULT_SUCCESS) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_RcvImsaImsRegDomainQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 局部变量 */
    IMSA_AT_ImsRegDomainQryCnf *imsRegDomainCnf = (IMSA_AT_ImsRegDomainQryCnf *)msg;

    /* 无效值修改为255后，删除返回值是否合法判断 */
    g_atSendDataBuff.bufLen +=
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            imsRegDomainCnf->imsRegDomain);

    return AT_OK;
}

VOS_UINT32 AT_RcvImsaImsCtrlMsg(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsCtrlMsg              *imsCtrlMsgInd = (IMSA_AT_ImsCtrlMsg *)msg;
    AT_IMS_CtrlMsgReceiveModuleUint8 module = AT_IMS_CTRL_MSG_RECEIVE_MODULE_IMSA;

    AT_PS_ReportImsCtrlMsgu(indexNum, module, imsCtrlMsgInd->wifiMsgLen, imsCtrlMsgInd->wifiMsg);

    return VOS_OK;
}

VOS_UINT32 AT_RcvImsaImpuSetCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_VolteimpuQryCnf *impuCnf = (IMSA_AT_VolteimpuQryCnf *)msg;
    VOS_CHAR                 acString[AT_IMSA_IMPU_MAX_LENGTH + 1];
    VOS_UINT32               result;
    errno_t                  memResult;
    VOS_UINT16               length = 0;

    (VOS_VOID)memset_s(acString, sizeof(acString), 0x00, sizeof(acString));
    if (impuCnf->impuLen > 0) {
        memResult = memcpy_s(acString, sizeof(acString), impuCnf->impu,
                             TAF_MIN(AT_IMSA_IMPU_MAX_LENGTH, impuCnf->impuLen));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(acString), TAF_MIN(AT_IMSA_IMPU_MAX_LENGTH, impuCnf->impuLen));
    }

    /* 判断查询操作是否成功 */
    if (impuCnf->result == VOS_OK) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %s", g_parseContext[indexNum].cmdElement->cmdName, acString);

        if ((impuCnf->impuLenVirtual != 0) && (impuCnf->impuLenVirtual < AT_IMSA_IMPU_MAX_LENGTH)) {
            impuCnf->impuVirtual[impuCnf->impuLenVirtual] = '\0';
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%s", impuCnf->impuVirtual);
        }

        g_atSendDataBuff.bufLen = length;
        result                  = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        result                  = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvImsaNickNameQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_NicknameQryCnf *nickNameQryCnf = (IMSA_AT_NicknameQryCnf *)msg;
    VOS_UINT32              result;
    VOS_UINT16              length = 0;
    VOS_UINT8               i = 0;

    /* 判断查询操作是否成功 */
    if ((nickNameQryCnf->result == VOS_OK) && (nickNameQryCnf->nickName.nickNameLen < MN_CALL_DISPLAY_NAME_STRING_SZ)) {
        if (nickNameQryCnf->nickName.nickNameLen != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

            for (i = 0; i < nickNameQryCnf->nickName.nickNameLen; i++) {
                length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress + length, "%X", (VOS_UINT8)nickNameQryCnf->nickName.nickName[i]);
            }
        } else {
            AT_WARN_LOG("AT_RcvImsaNickNameQryCnf: WARNING: ucNickNameLen is 0!");
        }

        result = AT_OK;
    } else {
        result = AT_ERROR;
    }

    g_atSendDataBuff.bufLen = length;

    return result;
}

VOS_UINT32 AT_RcvMmaImsDomainCfgQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_ImsDomainCfgQryCnf *domainCfgQryCnf = (TAF_MMA_ImsDomainCfgQryCnf *)msg;

    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            domainCfgQryCnf->imsDomainCfgType);

    return AT_OK;
}
#endif

VOS_UINT32 AT_RcvImsaImsIpCapQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    IMSA_AT_ImsIpCapQryCnf *imsIpCapQryCnf = (IMSA_AT_ImsIpCapQryCnf *)msg;

    /* 判断查询操作是否成功 */
    if (imsIpCapQryCnf->result == VOS_OK) {
        g_atSendDataBuff.bufLen = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
            imsIpCapQryCnf->ipsecFlag, imsIpCapQryCnf->keepAliveFlag);

        return AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}
