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

#include "at_general_mm_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "at_set_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_MM_SET_CMD_PROC_C

#define AT_BUF_ARRAY_MAX_LEN 5
#define AT_HS_ID 0
#define AT_HS_ACTION 1
#define AT_HS_PARA_NUM 2
#define AT_EOPLMN_VERSION 0
#define AT_EOPLMN_INDEX 1
#define AT_EOPLMN_PDULEN 2
#define AT_EOPLMN_PDU 3
#define AT_CERSSI_PARA_MAX_NUM 2
#define AT_CERSSI_MODE 0
#define AT_C5GPNSSAI_PARA_NUM 2
#define AT_C5GNSSAI_PARA_NUM 2
#define AT_PLMN_ID_INDEX_0 0
#define AT_PLMN_ID_INDEX_1 1
#define AT_PLMN_ID_INDEX_2 2
#define AT_MCC_DIGIT_1 0
#define AT_MCC_DIGIT_2 1
#define AT_MCC_DIGIT_3 2
#define AT_MNC_DIGIT_1 3
#define AT_MNC_DIGIT_2 4
#define AT_C5GNSSAIRDP_NASSAI_TYPE 0
#define AT_C5GNSSAIRDP_PLMN_ID 1
#define AT_PLMN_LEN_SIX_MNC_DIGIT_2 5
#define AT_C5GNSSAIRDP_PARA_MAX_NUM 2
#define AT_C5GNSSAIRDP_PLMN_ID_VAILD_LEN1 5
#define AT_C5GNSSAIRDP_PLMN_ID_VAILD_LEN2 6

TAF_UINT32 At_SetHSPara(TAF_UINT8 indexNum)
{
    TAF_UINT8 buf[AT_BUF_ARRAY_MAX_LEN];
    errno_t   memResult;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > AT_HS_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_HS_ID].paraLen != 0) {
        memResult = memcpy_s(buf, sizeof(buf), &g_atParaList[AT_HS_ID].paraValue, sizeof(TAF_UINT32));
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(buf), sizeof(TAF_UINT32));
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_HS_ACTION].paraLen != 0) {
        buf[AT_BUF_ARRAY_MAX_LEN - 1] = (TAF_UINT8)g_atParaList[AT_HS_ACTION].paraValue;
    } else {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作，这些参数可以不用发送到MMA */
    if (TAF_MMA_QryHandShakeReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HS_READ;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEOPlmnParaCheck(VOS_VOID)
{
    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不为TAF_AT_EOPLMN_PARA_NUM，返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != TAF_AT_EOPLMN_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第1个参数检查,version长度不能大于TAF_MAX_USER_CFG_OPLMN_VERSION_LEN */
    if (g_atParaList[AT_EOPLMN_VERSION].paraLen >= TAF_MAX_USER_CFG_OPLMN_VERSION_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第2个参数检查，组Index值不能大于5 */
    if (g_atParaList[AT_EOPLMN_INDEX].paraValue > TAF_MAX_USER_CFG_OPLMN_GROUP_INDEX) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第3个参数检查，pduLen值不能大于500 */
    if (g_atParaList[AT_EOPLMN_PDULEN].paraValue > TAF_AT_PLMN_WITH_RAT_LEN * TAF_MAX_GROUP_CFG_OPLMN_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第3个参数检查，pduLen值必须为TAF_AT_PLMN_WITH_RAT_LEN的整数倍 */
    if ((g_atParaList[AT_EOPLMN_PDULEN].paraValue % TAF_AT_PLMN_WITH_RAT_LEN) != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 最大只允许设置256个，因此最后的第6组(index=5)最大允许用户设置6个OPLMN */
    if ((g_atParaList[AT_EOPLMN_PDULEN].paraValue > 6 * TAF_AT_PLMN_WITH_RAT_LEN) &&
        (g_atParaList[AT_EOPLMN_INDEX].paraValue == TAF_MAX_USER_CFG_OPLMN_GROUP_INDEX)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第4个参数检查，PDU的实际长度必须与指定的pdulen相同 */
    if (g_atParaList[AT_EOPLMN_PDU].paraLen != g_atParaList[AT_EOPLMN_PDULEN].paraValue) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第4个参数检查，PDU数据必须可以完成16进制字符串转换 */
    if (At_AsciiNum2HexString(g_atParaList[AT_EOPLMN_PDU].para,
                              &g_atParaList[AT_EOPLMN_PDU].paraLen) != AT_SUCCESS) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetUserCfgOPlmnPara(VOS_UINT8 indexNum)
{
    TAF_MMA_SetEoplmnList eOPlmnSetPara;
    VOS_UINT32            rst;
    errno_t               memResult;

    (VOS_VOID)memset_s(&eOPlmnSetPara, sizeof(eOPlmnSetPara), 0x00, sizeof(TAF_MMA_SetEoplmnList));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    rst = AT_SetEOPlmnParaCheck();
    if (rst != AT_SUCCESS) {
        return rst;
    }

    /* 将设置的AT参数封装成TAF_MMA_SetEoplmnList的形式 */
    memResult = memcpy_s(eOPlmnSetPara.version, sizeof(eOPlmnSetPara.version), g_atParaList[0].para,
                         TAF_MAX_USER_CFG_OPLMN_VERSION_LEN);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(eOPlmnSetPara.version), TAF_MAX_USER_CFG_OPLMN_VERSION_LEN);

    if (g_atParaList[AT_EOPLMN_PDU].paraLen > TAF_MAX_GROUP_CFG_OPLMN_DATA_LEN) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_EOPLMN_PDU].paraLen > 0) {
        memResult = memcpy_s(eOPlmnSetPara.oPlmnWithRat, sizeof(eOPlmnSetPara.oPlmnWithRat),
                             g_atParaList[AT_EOPLMN_PDU].para, g_atParaList[AT_EOPLMN_PDU].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(eOPlmnSetPara.oPlmnWithRat), g_atParaList[AT_EOPLMN_PDU].paraLen);
    }

    eOPlmnSetPara.indexNum   = (VOS_UINT8)g_atParaList[1].paraValue;
    eOPlmnSetPara.oPlmnCount = (VOS_UINT8)(g_atParaList[AT_EOPLMN_PDULEN].paraValue / TAF_AT_PLMN_WITH_RAT_LEN);

    /* 与MMA进行交互不成功返回失败，交互成功返回命令处理挂起状态 */
    rst = TAF_MMA_SetEOPlmnReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &eOPlmnSetPara);
    if (rst == VOS_TRUE) {
        /* 指示当前用户的命令操作类型为设置命令 */
        g_parseContext[indexNum].clientStatus = AT_FW_CLIENT_STATUS_PEND;
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EOPLMN_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_ConvertWs46RatOrderPara(VOS_UINT8 clientId, TAF_MMA_SysCfgPara *sysCfgSetPara)
{
    TAF_MMA_MultimodeRatCfg *ratOrder = VOS_NULL_PTR;
    ModemIdUint16            modemId = MODEM_ID_0;
    VOS_UINT32               rst;

    ratOrder = &(sysCfgSetPara->multiModeRatCfg);

    rst = AT_GetModemIdFromClient(clientId, &modemId);
    if (rst != VOS_OK) {
        AT_ERR_LOG1("AT_ConvertWs46RatOrderPara:Get ModemID From ClientID fail,ClientID:", clientId);
        return AT_ERROR;
    }

    AT_SetDefaultRatPrioList(modemId, ratOrder, VOS_TRUE, VOS_TRUE);

    sysCfgSetPara->userPrio = AT_GetSysCfgPrioRat(sysCfgSetPara);

    return AT_OK;
}

VOS_UINT32 AT_SetWs46Para(VOS_UINT8 indexNum)
{
    TAF_MMA_SysCfgPara sysCfgSetPara;
    VOS_UINT32         rst;
    AT_ModemNetCtx    *netCtx = VOS_NULL_PTR;

    (VOS_VOID)memset_s(&sysCfgSetPara, sizeof(sysCfgSetPara), 0x00, sizeof(sysCfgSetPara));

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    rst = AT_OK;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = AT_ConvertWs46RatOrderPara(indexNum, &sysCfgSetPara);

    if (rst != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* stSysCfgSetPara其他参数赋值 */
    if (netCtx->roamFeature == AT_ROAM_FEATURE_ON) {
        sysCfgSetPara.roam = TAF_MMA_ROAM_UNCHANGE;
    } else {
        sysCfgSetPara.roam = AT_ROAM_FEATURE_OFF_NOCHANGE;
    }

    sysCfgSetPara.srvDomain       = TAF_MMA_SERVICE_DOMAIN_NOCHANGE;
    sysCfgSetPara.guBand.bandLow  = TAF_PH_BAND_NO_CHANGE;
    sysCfgSetPara.guBand.bandHigh = 0;

    (VOS_VOID)memset_s(&(sysCfgSetPara.lBand), sizeof(sysCfgSetPara.lBand), 0x00, sizeof(sysCfgSetPara.lBand));
    sysCfgSetPara.lBand.bandInfo[0] = TAF_PH_BAND_NO_CHANGE;

    /* 执行命令操作 */
    if (TAF_MMA_SetSysCfgReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &sysCfgSetPara) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_WS46_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCerssiPara(VOS_UINT8 indexNum)
{
    TAF_START_InfoInd startInfoInd;
    AT_ModemNetCtx   *netCtx = VOS_NULL_PTR;

    netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);

    (VOS_VOID)memset_s(&startInfoInd, sizeof(startInfoInd), 0x00, sizeof(startInfoInd));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex > AT_CERSSI_PARA_MAX_NUM) || (g_atParaList[AT_CERSSI_MODE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex == 1) {
        netCtx->cerssiReportType         = 0;
        netCtx->cerssiMinTimerInterval   = 0;
        startInfoInd.minRptTimerInterval = 0;
    }

    if (g_atParaIndex == AT_CERSSI_PARA_MAX_NUM) {
        netCtx->cerssiMinTimerInterval   = (VOS_UINT8)g_atParaList[1].paraValue;
        startInfoInd.minRptTimerInterval = (VOS_UINT8)g_atParaList[1].paraValue;
    }

    if (g_atParaList[0].paraValue != 0) {
        netCtx->cerssiReportType   = (VOS_UINT8)g_atParaList[0].paraValue;
        startInfoInd.actionType    = TAF_START_EVENT_INFO_FOREVER;
        startInfoInd.signThreshold = (VOS_UINT8)g_atParaList[0].paraValue;
        startInfoInd.rrcMsgType    = TAF_EVENT_INFO_CELL_SIGN;
    } else {
        /*
         * 由于+creg/+cgreg/+csq/^rssi都是通过RRMM_AT_MSG_REQ或GRRMM_AT_MSG_REQ通知接入层主动上报，
         * 所以不能通知接入层停止，设置成默认rssi 5db改变上报，AT_GetModemNetCtxAddrFromClientId(ucIndex)->ucCerssiReportType全局变量控制^cerssi是否上报
         */
        startInfoInd.actionType          = TAF_START_EVENT_INFO_FOREVER;
        startInfoInd.rrcMsgType          = TAF_EVENT_INFO_CELL_SIGN;
        startInfoInd.signThreshold       = 0;
        netCtx->cerssiReportType         = AT_CERSSI_REPORT_TYPE_NOT_REPORT;
        netCtx->cerssiMinTimerInterval   = 0;
        startInfoInd.minRptTimerInterval = 0;
    }

    if (TAF_MMA_SetCerssiReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &startInfoInd) == VOS_TRUE) {
        /* Set at cmd type */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CERSSI_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }

    /* AT不需要发给L4A，由MMC发给LMM,再由LMM通知LRRC */
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
LOCAL VOS_UINT32 AT_DecodeMultiSNssaiString(VOS_UINT16 cfgSNssaiLen, VOS_UINT8 *sNssai, VOS_UINT16 sNssaiLen,
                                            PS_CONFIGURED_Nssai *sNssaiInfo)
{
    VOS_UINT8       temp[AT_SINGLE_S_NSSAI_MAX_STR_LEN] = {0};
    PS_S_NSSAI_STRU singleNssai;
    VOS_UINT32      i;
    VOS_UINT32      j;

    (VOS_VOID)memset_s(&singleNssai, sizeof(singleNssai), 0, sizeof(PS_S_NSSAI_STRU));

    /* 对用户配置的长度进行非法校验 */
    if (cfgSNssaiLen > AT_EVT_MULTI_S_NSSAI_LEN) {
        return VOS_ERR;
    }

    /* 如果切片长度为0，直接删除切片 */
    if (sNssaiLen == 0) {
        if (cfgSNssaiLen != 0) {
            return VOS_ERR;
        }

        sNssaiInfo->snssaiNum = 0;

        return VOS_OK;
    }

    /* 首尾的字符串不能是切片间隔符号: */
    if ((sNssai[0] == ':') || (sNssai[sNssaiLen - 1] == ':')) {
        return VOS_ERR;
    }

    j = 0;

    for (i = 0; i < sNssaiLen; i++) {
        if (sNssai[i] != ':') {
            /* 选出单个切片字符串 */
            temp[j] = sNssai[i];

            j++;

            if (j >= AT_SINGLE_S_NSSAI_MAX_STR_LEN) {
                return VOS_ERR;
            }

            if (i != (sNssaiLen - 1)) {
                continue;
            }
        }

        if (j == 0) {
            AT_WARN_LOG("AT_DecodeMultiSNssaiString: WARNING: Invalid S-NSSAI length.");
            return VOS_ERR;
        }

        if (AT_DecodeSNssai(temp, (VOS_UINT16)j, &singleNssai) != VOS_OK) {
            AT_WARN_LOG("AT_DecodeMultiSNssaiString: WARNING: decode single nssai err.");
            return VOS_ERR;
        }

        /* 超出最大规格 */
        if (sNssaiInfo->snssaiNum >= PS_MAX_CFG_S_NSSAI_NUM) {
            AT_WARN_LOG("AT_DecodeMultiSNssaiString: WARNING: too max.");
            return VOS_ERR;
        }

        /* 取出有效切片 */
        sNssaiInfo->snssai[sNssaiInfo->snssaiNum] = singleNssai;

        /* 切片个数累加 */
        sNssaiInfo->snssaiNum++;

        /* 重新将索引设置为0 */
        j = 0;
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetC5gPNssaiPara(VOS_UINT8 indexNum)
{
    AT_MTA_5GNssaiSetReq preferNssai;
    VOS_UINT32           rslt;

    /* +C5GPNSSAI=[<Preferred_NSSAI_3gpp_length>,[<Preferred_NSSAI_3gpp>]],[<Preferred_NSSAI_non3gpp_length>,[<Preferred_NSSAI_non3gpp>]]
     */
    /* 初始化 */
    (VOS_VOID)memset_s(&preferNssai, sizeof(preferNssai), 0x00, sizeof(AT_MTA_5GNssaiSetReq));

    preferNssai.sNssaiSetType = AT_MTA_NSSAI_SET_PREFER_NSSAI;

    if (g_atParaIndex == 0) {
        /* 1个参数也没有，删除Prefer切片信息 */
        preferNssai.st5gNssai.snssaiNum = 0;
    } else if (g_atParaIndex == 1) {
        if (g_atParaList[0].paraValue != 0) {
            /* 如果length字段值不为0，但只有length这1个字段，无切片信息，AT格式不合法 */
            return AT_CME_INCORRECT_PARAMETERS;
        }

        preferNssai.st5gNssai.snssaiNum = 0;
    } else if (g_atParaIndex == AT_C5GPNSSAI_PARA_NUM) {
        if (VOS_OK != AT_DecodeMultiSNssaiString((VOS_UINT16)g_atParaList[0].paraValue, g_atParaList[1].para,
                                                 g_atParaList[1].paraLen, &preferNssai.st5gNssai)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        /* 参数过多 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (preferNssai.st5gNssai.snssaiNum > PS_MAX_PREFER_S_NSSAI_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_5G_NSSAI_SET_REQ, (VOS_UINT8 *)&preferNssai,
                                  (VOS_SIZE_T)sizeof(preferNssai), I0_UEPS_PID_MTA);

    if (rslt != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetC5gPNssaiPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GPNSSAI_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetC5gNssaiPara(VOS_UINT8 indexNum)
{
    AT_MTA_5GNssaiSetReq defCfgNssai;
    VOS_UINT32           rslt;

    /* +C5GNSSAI=<default_configured_nssai_length>,<default_configured_nssai> */
    /* 初始化 */
    (VOS_VOID)memset_s(&defCfgNssai, sizeof(defCfgNssai), 0x00, sizeof(AT_MTA_5GNssaiSetReq));

    defCfgNssai.sNssaiSetType = AT_MTA_NSSAI_SET_DEF_CFG_NSSAI;

    if (g_atParaIndex == AT_C5GNSSAI_PARA_NUM) {
        if (VOS_OK != AT_DecodeMultiSNssaiString((VOS_UINT16)g_atParaList[0].paraValue, g_atParaList[1].para,
                                                 g_atParaList[1].paraLen, &defCfgNssai.st5gNssai)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        /* 参数个数不对 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_5G_NSSAI_SET_REQ, (VOS_UINT8 *)&defCfgNssai,
                                  (VOS_SIZE_T)sizeof(defCfgNssai), I0_UEPS_PID_MTA);

    if (rslt != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetC5gNssaiPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GDFTCFGNSSAI_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_ConvertPlmnStrToHex(VOS_UINT8 *para, VOS_UINT16 len, PS_NR_PlmnId *plmn)
{
    VOS_UINT32 temp = 0;

    /* check整个码流中是否有不符合字符，如果有的话直接按照解析失败处理 */
    if (At_String2Hex(para, len, &temp) == AT_FAILURE) {
        return AT_ERROR;
    }

    /* MCC digit1 */
    At_String2Hex(&para[AT_MCC_DIGIT_1], 1, &temp);

    plmn->plmnId[AT_PLMN_ID_INDEX_0] = (VOS_UINT8)temp;

    /* MCC digit2 */
    At_String2Hex(&para[AT_MCC_DIGIT_2], 1, &temp);

    plmn->plmnId[AT_PLMN_ID_INDEX_0] |= ((VOS_UINT8)(temp << 4));

    /* MCC digit3 */
    At_String2Hex(&para[AT_MCC_DIGIT_3], 1, &temp);

    plmn->plmnId[AT_PLMN_ID_INDEX_1] = (VOS_UINT8)temp;

    /* MNC digit1 */
    At_String2Hex(&para[AT_MNC_DIGIT_1], 1, &temp);

    plmn->plmnId[AT_PLMN_ID_INDEX_2] = (VOS_UINT8)temp;

    /* MNC digit2 */
    At_String2Hex(&para[AT_MNC_DIGIT_2], 1, &temp);

    plmn->plmnId[AT_PLMN_ID_INDEX_2] |= ((VOS_UINT8)(temp << 4));

    if (len == AT_PLMN_LEN_SIX) {
        /* MNC digit2 */
        At_String2Hex(&para[AT_PLMN_LEN_SIX_MNC_DIGIT_2], 1, &temp);

        plmn->plmnId[AT_PLMN_ID_INDEX_1] |= ((VOS_UINT8)(temp << 4));
    } else {
        plmn->plmnId[AT_PLMN_ID_INDEX_1] |= 0xf0;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetC5gNssaiRdpPara(VOS_UINT8 indexNum)
{
    AT_MTA_5GNssaiQryReq st5gNssaiQryReq;
    PS_NR_PlmnId         plmn;
    VOS_UINT32           rslt;

    /* +C5GNSSAIRDP[=<nssai_type>[,<plmn_id>]] */
    /* 初始化 */
    (VOS_VOID)memset_s(&st5gNssaiQryReq, sizeof(st5gNssaiQryReq), 0x00, sizeof(AT_MTA_5GNssaiQryReq));
    (VOS_VOID)memset_s(&plmn, sizeof(plmn), 0x00, sizeof(PS_NR_PlmnId));

    if (g_atParaIndex == 0) {
        st5gNssaiQryReq.sNssaiQryType = AT_MTA_NSSAI_QRY_DEF_CFG_AND_REJ_AND_CFG_AND_ALLOWED_NSSAI;
        st5gNssaiQryReq.opPlmn        = VOS_FALSE;
    } else if (g_atParaIndex == 1) {
        st5gNssaiQryReq.sNssaiQryType = g_atParaList[AT_C5GNSSAIRDP_NASSAI_TYPE].paraValue;
        st5gNssaiQryReq.opPlmn        = VOS_FALSE;
    } else if (g_atParaIndex == AT_C5GNSSAIRDP_PARA_MAX_NUM) {
        st5gNssaiQryReq.sNssaiQryType = g_atParaList[AT_C5GNSSAIRDP_NASSAI_TYPE].paraValue;

        /* PLMN为5个或者6个digit */
        if ((g_atParaList[AT_C5GNSSAIRDP_PLMN_ID].paraLen > AT_C5GNSSAIRDP_PLMN_ID_VAILD_LEN2) ||
            (g_atParaList[AT_C5GNSSAIRDP_PLMN_ID].paraLen < AT_C5GNSSAIRDP_PLMN_ID_VAILD_LEN1)) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (AT_ConvertPlmnStrToHex(g_atParaList[AT_C5GNSSAIRDP_PLMN_ID].para,
            g_atParaList[AT_C5GNSSAIRDP_PLMN_ID].paraLen, &plmn) != AT_OK) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        st5gNssaiQryReq.opPlmn = VOS_TRUE;
        st5gNssaiQryReq.plmn   = plmn;
    } else {
        /* 参数过多 */
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_5G_NSSAI_QRY_REQ, (VOS_UINT8 *)&st5gNssaiQryReq,
                                  (VOS_SIZE_T)sizeof(st5gNssaiQryReq), I0_UEPS_PID_MTA);

    if (rslt != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetC5gNssaiRdpPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_C5GNSSAIRDP_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

#endif




