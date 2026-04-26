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
#include "at_custom_as_set_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_ctx.h"
#include "nv_stru_lps.h"
#include "at_lte_common.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_AS_SET_CMD_PROC_C

#define AT_FINETIMEREQ_MAX_PARA_NUM 3
#define AT_FINETIMEREQ_MIN_PARA_NUM 1
#define AT_FINETIMEREQ_RAT 2
#define AT_FINE_TIME_DEFAULT_NUM 2
#define AT_ERRCCAPCFG_MAX_PARA_NUM 4
#define AT_ERRCCAPCFG_MIN_PARA_NUM 2
#define AT_ERRCCAPCFG_CONTROL_CAPA_3_PARA_NUM 3 /* ERRCCAPCFG命令控制能力为三个时的参数个数 */
#define AT_ERRCCAPCFG_PARA2 2
#define AT_ERRCCAPCFG_PARA3 3
#define AT_PSEUCELL_PLMN 2
#define AT_PSEUCELL_LAC 3
#define AT_PSEUCELL_CELLID 4
#define AT_PSEUCELL_PARA_NUM 5
#define AT_PSEUCELL_NOTIFY_TYPE 0
#define AT_PSEUCELL_RAT 1
#define AT_SUBCLFSPARAM_MAX_PARA_NUM_FUNC_OFF 6
#define AT_SUBCLFSPARAM_MAX_PARA_NUM 9
#define AT_SUBCLFSPARAM_MIN_PARA_NUM 6
#define AT_SUBCLFSPARAM_PARA_VERSIONID_MAX_LEN 10
#define AT_SUBCLFSPARAM_SEQ 0
#define AT_SUBCLFSPARAM_VERSIONID 1
#define AT_SUBCLFSPARAM_TYPE 2
#define AT_SUBCLFSPARAM_ACTIVEFLG 3
#define AT_SUBCLFSPARAM_PROBARATE 4
#define AT_SUBCLFSPARAM_CLFSGROUPNUM 5
#define AT_MTA_PARA_INFO_CLFS_GROUP_NUM 6 /* 结构体AT_MTA_ParaInfo的第一个成员clfsGroupNum*/
#define AT_MTA_PARA_INFO_DATA_LENGTH 7 /* 结构体AT_MTA_ParaInfo的第二个成员dataLength*/
#define AT_MTA_PARA_INFO_CLFS_DATA 8 /* 结构体AT_MTA_ParaInfo的第三个成员clfsData*/
#define AT_MTA_PARA_INFO_CLFS_DATA_LEN 4
#define AT_ECCCFG_PARA_NUM 2
#define AT_PSEUDBTS_PARA_NUM 2
#define AT_TFDPDT_PARA_NUM 2
#define AT_TFDPDT_RAT_MODE 0
#define AT_TFDPDT_DPDT_VALUE 1
#define AT_LRRCUECAPPARAMSET_PARA1_INDEX 3
#define AT_NRRCUECAPPARAMSET_PARA1_INDEX 3

#define AT_NRCACELLRPTCFG_PARA_NUM 1
#define AT_NRNWCAPRPTCFG_PARA_NUM 2
#define AT_NRNWCAPQRY_PARA_NUM 1
#define AT_NRNWCAPRPTQRY_PARA_NUM 1

#define AT_UE_CAP_MAX_PARA_NUM 6
#define AT_UE_CAP_MIN_PARA_NUM 4
#define AT_UE_CAP_PARA1_INDEX 3
#define AT_UE_CAP_PARA_REQ 0
#define AT_UE_CAP_PARA_ENDFLAG 1
#define AT_UE_CAP_CAPPARAMNUM 2
#define AT_UE_CAP_PARA1 3
#define AT_UE_CAP_PARA_ENDFLAG_VALID_VALUE 1
#define AT_OVERHEATINGCFG_TYPE 0
#define AT_OVERHEATINGCFG_FLAG 1
#define AT_OVERHEATINGCFG_SETPARAMFLAG 2
#define AT_OVERHEATINGCFG_PARA1 3
#define AT_OVERHEATINGCFG_PARA2 4
#define AT_OVERHEATINGCFG_PARA3 5
#define AT_OVERHEATINGCFG_PARA4 6
#define AT_OVERHEATINGCFG_PARA5 7
#define AT_OVERHEATINGCFG_PARA6 8
#define AT_OVERHEATINGCFG_PARA7 9
#define AT_OVERHEATINGCFG_PARA8 10
#define AT_OVERHEATINGCFG_PARA9 11
#define AT_OVERHEATINGCFG_PARA10 12

#define AT_NRRCCAPCFG_MIN_PARA_NUM 1
#define AT_NRRCCAPCFG_NRRCCFGMODE 0

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
#define AT_NRPOWERSAVINGCFG_MIN_PARA_NUM 1
#define AT_NRPOWERSAVINGCFG_MAX_PARA_NUM 24
#define AT_NRPOWERSAVINGCFG_CMDTYPE 0
#define AT_AUTON2LOPTCFG_MAX_PARA_NUM 10
#define AT_AUTON2LOPTCFG_SWITCH_ENABLE 0
#define AT_AUTON2LOPTCFG_PARA_ENABLE 1
#define AT_AUTON2LOPTCFG_IDLE_SWITCH 2
#define AT_AUTON2LOPTCFG_CONN_SWITCH 3
#define AT_AUTON2LOPTCFG_RSRP_NR_LOW 4
#define AT_AUTON2LOPTCFG_SINR_NR_LOW 5
#define AT_AUTON2LOPTCFG_RSRP_LTE_HIGH 6
#define AT_AUTON2LOPTCFG_SINR_LTE_HIGH 7
#define AT_AUTON2LOPTCFG_RSRP_L2N_JDG 8
#define AT_AUTON2LOPTCFG_SINR_L2N_JDG 9
#define AT_INT16_MAX_VALUE 0x7FFF
#define AT_INT16_ABSOLUTE_MIN_VALUE 0x8000

#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_SetFineTimeReqPara(VOS_UINT8 indexNum)
{
    AT_MTA_FineTimeSetReq fineTimeType;
    VOS_UINT32            rst;

    (VOS_VOID)memset_s(&fineTimeType, sizeof(fineTimeType), 0x00, sizeof(fineTimeType));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if ((g_atParaIndex < AT_FINETIMEREQ_MIN_PARA_NUM) || (g_atParaIndex > AT_FINETIMEREQ_MAX_PARA_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    fineTimeType.fineTimeType = (VOS_UINT8)g_atParaList[0].paraValue;

    if (g_atParaList[1].paraLen == 0) {
        /* <num>参数不下发取默认值 */
        fineTimeType.num = AT_FINE_TIME_DEFAULT_NUM;
    } else {
        fineTimeType.num = (VOS_UINT16)g_atParaList[1].paraValue;
    }

    if (g_atParaList[AT_FINETIMEREQ_RAT].paraLen == 0) {
        /* <rat>参数不下发取默认值LTE，即当前GPS芯片为老平台 */
        fineTimeType.ratMode = MTA_AT_FINE_TIME_RAT_LTE;
    } else {
        fineTimeType.ratMode = (VOS_UINT8)g_atParaList[AT_FINETIMEREQ_RAT].paraValue;
    }

    if (fineTimeType.fineTimeType == AT_MTA_FINETIME_PARA_0) {
        fineTimeType.forceFlag = VOS_TRUE;
    } else {
        fineTimeType.forceFlag = VOS_FALSE;
    }

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_FINE_TIME_SET_REQ, (VOS_UINT8 *)&fineTimeType, sizeof(fineTimeType),
                                 I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetFineTimeReqPara: AT_SetFineTimeReqPara fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FINE_TIME_SET;

    return AT_WAIT_ASYNC_RETURN;
}
#endif

VOS_UINT32 AT_SetEcidPara(VOS_UINT8 indexNum)
{
    VOS_UINT32        result;
    AT_MTA_EcidSetReq ecidSetReq;

    /* 参数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&ecidSetReq, sizeof(ecidSetReq), 0x00, sizeof(ecidSetReq));

    ecidSetReq.version = g_atParaList[0].paraValue;

    /* 发送跨核消息到C核, 设置GPS参考时钟状态是否主动上报 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_ECID_SET_REQ, (VOS_UINT8 *)&ecidSetReq, sizeof(ecidSetReq),
                                    I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetEcidPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECID_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SetNetMonSCellPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_SET_NETMON_SCELL_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MONSC_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetNetMonNCellPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_SET_NETMON_NCELL_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MONNC_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 At_SetNetMonSSCellPara(VOS_UINT8 atIndex)
{
    VOS_UINT32 rslt;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[atIndex].clientId, g_atClientTab[atIndex].opId,
                                  ID_AT_MTA_SET_NETMON_SSCELL_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        g_atClientTab[atIndex].cmdCurrentOpt = AT_CMD_MONSSC_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetNrCellBandPara(VOS_UINT8 atIndex)
{
    VOS_UINT32 rslt;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[atIndex].clientId, g_atClientTab[atIndex].opId,
                                  ID_AT_MTA_SET_NR_CELL_BAND_REQ, VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        g_atClientTab[atIndex].cmdCurrentOpt = AT_CMD_NRCELLBAND_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#endif

#if (FEATURE_LTE == FEATURE_ON)
LOCAL VOS_UINT32 AT_CheckErrcCapCfgPara(VOS_VOID)
{
    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为2-4个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if ((g_atParaIndex < AT_ERRCCAPCFG_MIN_PARA_NUM) || (g_atParaIndex > AT_ERRCCAPCFG_MAX_PARA_NUM)) {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第1个参数长度不为0，否则返回AT_CME_INCORR ECT_PARAMETERS */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetErrcCapCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_ErrccapCfgReq setErrcCapCfgReq;
    VOS_UINT32           result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setErrcCapCfgReq, sizeof(setErrcCapCfgReq), 0x00, sizeof(AT_MTA_ErrccapCfgReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    result = AT_CheckErrcCapCfgPara();
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充结构体 */
    setErrcCapCfgReq.errcCapType = g_atParaList[0].paraValue;
    setErrcCapCfgReq.para1       = g_atParaList[1].paraValue;

    if (g_atParaIndex == AT_ERRCCAPCFG_CONTROL_CAPA_3_PARA_NUM) {
        setErrcCapCfgReq.para2 = g_atParaList[AT_ERRCCAPCFG_PARA2].paraValue;
    }

    if (g_atParaIndex == AT_ERRCCAPCFG_MAX_PARA_NUM) {
        setErrcCapCfgReq.para2 = g_atParaList[AT_ERRCCAPCFG_PARA2].paraValue;
        setErrcCapCfgReq.para3 = g_atParaList[AT_ERRCCAPCFG_PARA3].paraValue;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_ERRCCAP_CFG_SET_REQ, (VOS_UINT8 *)&setErrcCapCfgReq,
                                    sizeof(AT_MTA_ErrccapCfgReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ERRCCAPCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetErrcCapQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_ErrccapQryReq setErrcCapQryReq;
    VOS_UINT32           result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setErrcCapQryReq, sizeof(setErrcCapQryReq), 0x00, sizeof(setErrcCapQryReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetErrcCapQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为1个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetErrcCapQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数长度不能为0，否则返回AT_CME_INCORR ECT_PARAMETERS */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetErrcCapQryPara: Length is 0.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体 */
    setErrcCapQryReq.errcCapType = g_atParaList[0].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_ERRCCAP_QRY_SET_REQ, (VOS_UINT8 *)&setErrcCapQryReq,
                                    sizeof(AT_MTA_ErrccapQryReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ERRCCAPQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif

VOS_UINT32 AT_CheckPseucellInfoParaLen(VOS_VOID)
{
    /* 如果notify type长度为0，返回错误 */
    if (g_atParaList[AT_PSEUCELL_NOTIFY_TYPE].paraLen == 0) {
        AT_WARN_LOG("AT_CheckPseucellInfoParaLen : Pseucell Notify Type Len is error.");

        return AT_ERROR;
    }

    /* 如果sys mode长度为0，返回错误 */
    if (g_atParaList[AT_PSEUCELL_RAT].paraLen == 0) {
        AT_WARN_LOG("AT_CheckPseucellInfoParaLen : sys mode Len is error.");

        return AT_ERROR;
    }

    /* PLMN参数长度不等于5也不等于6，返回错误 */
    if ((g_atParaList[AT_PSEUCELL_PLMN].paraLen != AT_MTA_PSEUCELL_PLMN_LEN_FIVE) &&
        (g_atParaList[AT_PSEUCELL_PLMN].paraLen != AT_MTA_PSEUCELL_PLMN_LEN_SIX)) {
        AT_WARN_LOG("AT_CheckPseucellInfoParaLen : Plmn Len is error.");

        return AT_ERROR;
    }

    /*  LAC参数长度不等于4，返回错误 */
    if (g_atParaList[AT_PSEUCELL_LAC].paraLen != AT_MTA_PSEUCELL_LAC_LEN) {
        AT_WARN_LOG("AT_CheckPseucellInfoParaLen : Lac Len is error.");

        return AT_ERROR;
    }

    /*  CellId参数长度大于8，或者长度为0，返回错误 */
    if (g_atParaList[AT_PSEUCELL_CELLID].paraLen != AT_MTA_PSEUCELL_CELLID_LEN) {
        AT_WARN_LOG("AT_CheckPseucellInfoParaLen : CellId Len is error.");

        return AT_ERROR;
    }

    return AT_OK;
}

VOS_VOID AT_ConvertMncToNasType(VOS_UINT32 mnc, VOS_UINT32 mncLen, VOS_UINT32 *nasMnc)
{
    VOS_UINT32 tmp[AT_MNC_LENGTH];
    VOS_UINT32 loop;

    *nasMnc = 0;

    for (loop = 0; loop < AT_MNC_LENGTH; loop++) {
        tmp[loop] = mnc & 0x0f;
        mnc >>= 4;
    }

    /* tmp[0],tmp[1],tmp[2]中的0 1 2分别表示tmp[AT_MNC_LENGTH]中的第0个元素，第1个元素，和第2个元素 */
    if (tmp[2] == 0x0000000F) {
        *nasMnc = 0x000F0000 | ((VOS_UINT32)tmp[0] << 8) | ((VOS_UINT32)tmp[1]);
    } else {
        *nasMnc = tmp[2] | ((VOS_UINT32)tmp[1] << 8) | ((VOS_UINT32)tmp[0] << 16);
    }
}

VOS_UINT32 AT_ParsePseucellInfoMccMnc(VOS_UINT8 *para, VOS_UINT16 paraLen, VOS_UINT32 *mcc, VOS_UINT32 *mnc)
{
    errno_t    memResult;
    VOS_UINT32 tempMcc = 0;
    VOS_UINT32 tempMnc = 0;
    VOS_UINT32 mncLen;
    VOS_UINT8  plmnStr[AT_MTA_PSEUCELL_PLMN_LEN_SIX];

    mncLen = (VOS_UINT32)paraLen - AT_MTA_PLMN_MCC_LEN;

    (VOS_VOID)memset_s(plmnStr, sizeof(plmnStr), 0, sizeof(plmnStr));

    if ((paraLen > 0) && (paraLen <= AT_MTA_PSEUCELL_PLMN_LEN_SIX)) {
        memResult = memcpy_s(plmnStr, AT_MTA_PSEUCELL_PLMN_LEN_SIX, para, paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_MTA_PSEUCELL_PLMN_LEN_SIX, paraLen);
    }

    if (AT_String2Hex(plmnStr, AT_MTA_PLMN_MCC_LEN, &tempMcc) == VOS_ERR) {
        AT_WARN_LOG("AT_ParsePseucellInfoMccMnc : Mcc is error.");

        return AT_ERROR;
    }

    if (AT_String2Hex(plmnStr + AT_MTA_PLMN_MCC_LEN, (VOS_UINT16)mncLen, &tempMnc) == VOS_ERR) {
        AT_WARN_LOG("AT_ParsePseucellInfoMccMnc : Mnc is error.");

        return AT_ERROR;
    }

    /* 将MCC转化为NAS类型 */
    AT_ConvertMccToNasType(tempMcc, mcc);

    /* 将MNC转化为NAS类型 */
    if (mncLen == AT_MTA_PLMN_MNC_LEN_TWO) {
        tempMnc |= 0x00000F00;
    } else {
        tempMnc &= 0x00000FFF;
    }

    AT_ConvertMncToNasType(tempMnc, mncLen, mnc);

    return AT_SUCCESS;
}

VOS_UINT32 AT_ParsePseucellInfoLac(VOS_UINT8 *para, VOS_UINT16 paraLen, VOS_UINT32 *lac)
{
    VOS_UINT8 lacStr[AT_MTA_PSEUCELL_LAC_LEN];
    errno_t   memResult;

    (VOS_VOID)memset_s(lacStr, sizeof(lacStr), 0, sizeof(lacStr));

    if ((paraLen > 0) && (paraLen <= AT_MTA_PSEUCELL_LAC_LEN)) {
        memResult = memcpy_s(lacStr, AT_MTA_PSEUCELL_LAC_LEN, para, paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_MTA_PSEUCELL_LAC_LEN, paraLen);
    }

    if (AT_String2Hex(lacStr, paraLen, lac) == VOS_ERR) {
        AT_WARN_LOG("AT_ParsePseucellInfoLac : Lac is error.");

        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_ParsePseucellInfoCellId(VOS_UINT8 *para, VOS_UINT16 paraLen, VOS_UINT32 *cellId)
{
    VOS_UINT8 cellIdStr[AT_MTA_PSEUCELL_CELLID_LEN];
    errno_t   memResult;

    (VOS_VOID)memset_s(cellIdStr, sizeof(cellIdStr), 0, sizeof(cellIdStr));

    if ((paraLen > 0) && (paraLen <= AT_MTA_PSEUCELL_CELLID_LEN)) {
        memResult = memcpy_s(cellIdStr, AT_MTA_PSEUCELL_CELLID_LEN, para, paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, AT_MTA_PSEUCELL_CELLID_LEN, paraLen);
    }

    paraLen = (VOS_UINT16)AT_MIN(paraLen, AT_MTA_PSEUCELL_CELLID_LEN);

    if (AT_String2Hex(cellIdStr, paraLen, cellId) == VOS_ERR) {
        AT_WARN_LOG("AT_ParsePseucellInfoCellId : CellId is error.");

        return AT_ERROR;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetPseucellInfoPara(VOS_UINT8 indexNum)
{
    AT_MTA_PseucellInfoSetReq pseucellInfoSetReq;
    VOS_UINT32                result = 0;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : Current Option is not AT_CMD_OPT_SET_PARA_CMD.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多或者过少 */
    if (g_atParaIndex != AT_PSEUCELL_PARA_NUM) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : Para Num is error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查输入参数长度是否正确 */
    if (AT_CheckPseucellInfoParaLen() == AT_ERROR) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : para len is error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 不是GSM模式，回应参数错误 */
    if (g_atParaList[AT_PSEUCELL_RAT].paraValue != AT_MTA_SYS_MODE_GSM) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : Not GSM.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&pseucellInfoSetReq, sizeof(pseucellInfoSetReq), 0x00, sizeof(pseucellInfoSetReq));

    /* 解析MCC和MNC */
    if (AT_ParsePseucellInfoMccMnc(g_atParaList[AT_PSEUCELL_PLMN].para,
                                   g_atParaList[AT_PSEUCELL_PLMN].paraLen,
                                   &pseucellInfoSetReq.mcc,
                                   &pseucellInfoSetReq.mnc) == AT_ERROR) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : parse mcc mnc error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 解析LAC */
    if (AT_ParsePseucellInfoLac(g_atParaList[AT_PSEUCELL_LAC].para,
                                g_atParaList[AT_PSEUCELL_LAC].paraLen, &pseucellInfoSetReq.lac) == AT_ERROR) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : parse lac error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 解析CellId */
    if (AT_ParsePseucellInfoCellId(g_atParaList[AT_PSEUCELL_CELLID].para, g_atParaList[AT_PSEUCELL_CELLID].paraLen,
                                   &pseucellInfoSetReq.cellId) == AT_ERROR) {
        AT_WARN_LOG("AT_SetPseucellInfoPara : parse cellid error.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    pseucellInfoSetReq.pseucellNotifyType = (VOS_UINT8)g_atParaList[AT_PSEUCELL_NOTIFY_TYPE].paraValue;
    pseucellInfoSetReq.sysmode            = g_atParaList[AT_PSEUCELL_RAT].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_PSEUCELL_INFO_SET_REQ, (VOS_UINT8 *)&pseucellInfoSetReq,
                                    (VOS_SIZE_T)sizeof(pseucellInfoSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetPseucellInfoPara: AT_FillAndSndAppReqMsg fail.");

        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PSEUCELL_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetEccCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetEccCfgReq setEccCfgReq;
    VOS_UINT32          result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setEccCfgReq, sizeof(setEccCfgReq), 0x00, sizeof(setEccCfgReq));

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为2个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != AT_ECCCFG_PARA_NUM) {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第1个参数长度不为0，否则返回AT_CME_INCORR ECT_PARAMETERS */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        AT_WARN_LOG("AT_CheckErrcCapCfgPara: Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体 */
    setEccCfgReq.eccEnable = (PS_BOOL_ENUM_UINT8)g_atParaList[0].paraValue;
    setEccCfgReq.rptPeriod = g_atParaList[1].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_ECC_CFG_SET_REQ, (VOS_UINT8 *)&setEccCfgReq, sizeof(AT_MTA_SetEccCfgReq),
                                    I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetPseudBtsPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetPseudbtsReq atCmd;
    VOS_UINT32            ret;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不等于2个，返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != AT_PSEUDBTS_PARA_NUM) {
        AT_WARN_LOG("AT_SetPseudBtsPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第1个参数长度为0，返回AT_CME_INCORR ECT_PARAMETERS */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        AT_WARN_LOG("AT_SetPseudBtsPara: para Length = 0");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    atCmd.pseudRat        = (VOS_UINT8)g_atParaList[0].paraValue;
    atCmd.pseudBtsQryType = (VOS_UINT8)g_atParaList[1].paraValue;

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_PSEUDBTS_SET_REQ, (VOS_UINT8 *)&atCmd,
                                 sizeof(AT_MTA_SetPseudbtsReq), I0_UEPS_PID_MTA);

    if (ret != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetPseudBtsPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PSEUDBTS_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_CheckSubClfsParamPara(VOS_VOID)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数大于9个、小于6个，返回AT_CME_INCORRECT_PARAMETERS */
    if ((g_atParaIndex < AT_SUBCLFSPARAM_MIN_PARA_NUM) || (g_atParaIndex > AT_SUBCLFSPARAM_MAX_PARA_NUM)) {
        AT_WARN_LOG("AT_CheckSubClfsParamPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_SUBCLFSPARAM_VERSIONID].paraLen > AT_SUBCLFSPARAM_PARA_VERSIONID_MAX_LEN) {
        AT_WARN_LOG("AT_CheckSubClfsParamPara: At Para1 Length Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果为功能关闭命令，则参数最多为6个 */
    if ((g_atParaList[AT_SUBCLFSPARAM_ACTIVEFLG].paraValue == 0) &&
        (g_atParaIndex > AT_SUBCLFSPARAM_MAX_PARA_NUM_FUNC_OFF)) {
        AT_WARN_LOG("AT_CheckSubClfsParamPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetSubClfsParamPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetSubclfsparamReq *msg = VOS_NULL_PTR;
    VOS_UINT_PTR               clfsDataAddr;
    VOS_UINT32                 bufLen;
    VOS_UINT32                 bsLen;
    VOS_UINT32                 loop;
    VOS_UINT32                 deltaLen;
    VOS_UINT32                 ret;
    errno_t                    memResult;

    /* 参数检查 */
    ret = AT_CheckSubClfsParamPara();
    if (ret != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetSubClfsParamPara: AT_CheckSubClfsParamPara Failed");
        return ret;
    }

    bsLen  = (VOS_UINT32)g_atParaList[AT_MTA_PARA_INFO_CLFS_GROUP_NUM].paraLen +
             (VOS_UINT32)g_atParaList[AT_MTA_PARA_INFO_DATA_LENGTH].paraLen +
             (VOS_UINT32)g_atParaList[AT_MTA_PARA_INFO_CLFS_DATA].paraLen;
    bufLen = sizeof(AT_MTA_SetSubclfsparamReq);

    if (bsLen > AT_MTA_PARA_INFO_CLFS_DATA_LEN) {
        bufLen += bsLen - AT_MTA_PARA_INFO_CLFS_DATA_LEN;
    }

    /*lint -save -e830*/
    msg = (AT_MTA_SetSubclfsparamReq *)PS_MEM_ALLOC(WUEPS_PID_AT, bufLen);
    /*lint -restore */

    /* 内存申请失败，返回AT_ERROR */
    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SetSubClfsParamPara:memory alloc fail.");
        return AT_ERROR;
    }

    (VOS_VOID)memset_s((VOS_INT8 *)msg, bufLen, 0x00, bufLen);

    /* 填写消息内容 */
    msg->seq                   = (VOS_UINT8)g_atParaList[AT_SUBCLFSPARAM_SEQ].paraValue;
    msg->type                  = (VOS_UINT8)g_atParaList[AT_SUBCLFSPARAM_TYPE].paraValue;
    msg->activeFlg             = (VOS_UINT8)g_atParaList[AT_SUBCLFSPARAM_ACTIVEFLG].paraValue;
    msg->probaRate             = g_atParaList[AT_SUBCLFSPARAM_PROBARATE].paraValue;
    msg->paraInfo.clfsGroupNum = (VOS_UINT16)g_atParaList[AT_SUBCLFSPARAM_CLFSGROUPNUM].paraValue;
    msg->paraInfo.dataLength   = (VOS_UINT16)bsLen;

    if (g_atParaList[AT_SUBCLFSPARAM_VERSIONID].paraLen > 0) {
        memResult = memcpy_s(msg->versionId, sizeof(msg->versionId), g_atParaList[AT_SUBCLFSPARAM_VERSIONID].para,
                             g_atParaList[AT_SUBCLFSPARAM_VERSIONID].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(msg->versionId), g_atParaList[AT_SUBCLFSPARAM_VERSIONID].paraLen);
    }

    deltaLen = 0;
    clfsDataAddr = (VOS_UINT_PTR)msg->paraInfo.clfsData;
    for (loop = AT_SUBCLFSPARAM_MIN_PARA_NUM; loop < AT_SUBCLFSPARAM_MAX_PARA_NUM; loop++) {
        if (g_atParaList[loop].paraLen > 0) {
            memResult = memcpy_s((VOS_UINT8 *)(clfsDataAddr + deltaLen), (bsLen - deltaLen), g_atParaList[loop].para,
                                 g_atParaList[loop].paraLen);
            TAF_MEM_CHK_RTN_VAL(memResult, (bsLen - deltaLen), g_atParaList[loop].paraLen);

            deltaLen = deltaLen + g_atParaList[loop].paraLen;
        } else {
            break;
        }
    }

    ret = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                 ID_AT_MTA_SUBCLFSPARAM_SET_REQ, (VOS_UINT8 *)msg, bufLen, I0_UEPS_PID_MTA);

    /*lint -save -e516*/
    PS_MEM_FREE(WUEPS_PID_AT, msg);
    /*lint -restore */

    if (ret != AT_SUCCESS) {
        AT_WARN_LOG("AT_SetPseudBtsPara: AT_FillAndSndAppReqMsg Failed!");
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_SUBCLFSPARAM_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetConnectRecovery(VOS_UINT8 indexNum)
{
    VOS_UINT32                          result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_LTE_CONN_RECOVERY_NTF,
                                    VOS_NULL_PTR, 0, I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetConnectRecovery: snd Fail");
        return AT_ERROR;
    }

    /* 与ap、lrrc接口约束，不需要等待结果应答，lrrc会启动保护定时器，保护ap频繁下发场景 */
    return AT_OK;
}
#endif
#endif

VOS_UINT32 At_SetTfDpdtPara(VOS_UINT8 indexNum)
{
    AT_MTA_SetDpdtValueReq atCmd;
    VOS_UINT32             rst;

    /* 参数检查 */
    if (g_atParaIndex != AT_TFDPDT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* AT发送至MTA的消息结构赋值 */
    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(AT_MTA_SetDpdtValueReq));
    atCmd.ratMode   = (AT_MTA_CmdRatmodeUint8)g_atParaList[AT_TFDPDT_RAT_MODE].paraValue;
    atCmd.dpdtValue = g_atParaList[AT_TFDPDT_DPDT_VALUE].paraValue;

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_DPDT_VALUE_REQ, (VOS_UINT8 *)&atCmd,
                                 sizeof(AT_MTA_SetDpdtValueReq), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_TFDPDT_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_CheckUeCapPara(VOS_VOID)
{
    VOS_UINT32 i;
    /* 参数类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if ((g_atParaIndex < AT_UE_CAP_MIN_PARA_NUM) || (g_atParaIndex > AT_UE_CAP_MAX_PARA_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数组数检查,命令总的参数个数减去后面三个参数 */
    if ((g_atParaList[AT_UE_CAP_CAPPARAMNUM].paraValue + 3) != g_atParaIndex) {
        AT_WARN_LOG("AT_CheckUeCapPara: Parameters Group Number Wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 必带参数检查 */
    if (g_atParaList[AT_UE_CAP_PARA1].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度不为偶数返回错误 */
    for (i = AT_UE_CAP_PARA1; i < g_atParaIndex; i++) {
        if ((g_atParaList[i].paraLen % 2) != 0) {
            AT_WARN_LOG("AT_CheckUeCapPara: Parameters Length Is Not Even.");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_FillUeCapPara(AT_MTA_UeCapParamSet *setUeCap, VOS_UINT32 paraLen)
{
    VOS_UINT32 loop;
    VOS_UINT32 moveLen;
    VOS_UINT32 result;

    setUeCap->seq = g_atParaList[AT_UE_CAP_PARA_REQ].paraValue;

    if (g_atParaList[AT_UE_CAP_PARA_ENDFLAG].paraValue == AT_UE_CAP_PARA_ENDFLAG_VALID_VALUE) {
        setUeCap->endFlag = VOS_TRUE;
    } else {
        setUeCap->endFlag = VOS_FALSE;
    }

    moveLen = 0;
    for (loop = AT_UE_CAP_PARA1_INDEX; loop < (VOS_UINT32)TAF_MIN(g_atParaIndex, AT_MAX_PARA_NUMBER); loop++) {
        if (g_atParaList[loop].paraLen > 0) {
            result = At_AsciiString2HexSimple(setUeCap->msg + moveLen, g_atParaList[loop].para,
                                              g_atParaList[loop].paraLen);
            if (result != AT_SUCCESS) {
                return AT_CME_INCORRECT_PARAMETERS;
            }
            moveLen += (VOS_UINT32)g_atParaList[loop].paraLen / 2;
        }
    }

    setUeCap->msgLen = paraLen;

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetLrrcUeCapPara(VOS_UINT8 indexNum)
{
    AT_MTA_UeCapParamSet *setUeCap = VOS_NULL_PTR;
    VOS_UINT32            msgLen;
    VOS_UINT32            paraLen = 0;
    VOS_UINT32            result;
    VOS_UINT32            i;

    /* 参数检查 */
    result = AT_CheckUeCapPara();
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetLrrcUeCapPara: AT_CheckUeCapPara Failed");
        return result;
    }

    for (i = AT_LRRCUECAPPARAMSET_PARA1_INDEX; i < (VOS_UINT32)TAF_MIN(g_atParaIndex, AT_MAX_PARA_NUMBER); i++) {
        paraLen +=  (VOS_UINT32)g_atParaList[i].paraLen / 2;
    }

    msgLen   = sizeof(AT_MTA_UeCapParamSet) + paraLen;
    setUeCap = (AT_MTA_UeCapParamSet *)PS_MEM_ALLOC(WUEPS_PID_AT, msgLen);
    /* 内存申请失败，返回AT_ERROR */
    if (setUeCap == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SetLrrcUeCapPara:memory alloc fail.");
        return AT_ERROR;
    }

    (VOS_VOID)memset_s((VOS_INT8 *)setUeCap, msgLen, 0x00, msgLen);

    /* 填写消息内容 */
    setUeCap->ratMode = AT_MTA_RAT_MODE_LTE;
    result            = AT_FillUeCapPara(setUeCap, paraLen);
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetLrrcUeCapPara: AT_FillUeCapPara Failed");
        PS_MEM_FREE(WUEPS_PID_AT, setUeCap);
        return result;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_LRRC_UE_CAP_PARAM_SET_REQ, (VOS_UINT8 *)setUeCap,
                                    msgLen, I0_UEPS_PID_MTA);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetLrrcUeCapPara: AT_FillAndSndAppReqMsg failed.");
        PS_MEM_FREE(WUEPS_PID_AT, setUeCap);
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_LRRC_UE_CAP_SET;
    PS_MEM_FREE(WUEPS_PID_AT, setUeCap);

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_CheckOverHeatingPara(VOS_VOID)
{
    /* 类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* SA必带参数检查:3个;ENDC必带参数4个，后续配合LRRC开发 */
    if ((g_atParaList[AT_OVERHEATINGCFG_TYPE].paraLen == 0) || (g_atParaList[AT_OVERHEATINGCFG_FLAG].paraLen == 0)
        || (g_atParaList[AT_OVERHEATINGCFG_SETPARAMFLAG].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 目前不支持ENDC，类型是ENDC直接回复失败 */
    if ((g_atParaList[AT_OVERHEATINGCFG_TYPE].paraValue != 0) && (g_atParaList[AT_OVERHEATINGCFG_TYPE].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_VOID AT_FillOverHeatingReqParam(AT_MTA_OverHeatingSetReq *overHeatingReq)
{
    overHeatingReq->overHeatingType = g_atParaList[AT_OVERHEATINGCFG_TYPE].paraValue == 1 ? MTA_AT_RAT_MODE_NR :
                                                                                            MTA_AT_RAT_MODE_LTE;
    overHeatingReq->overHeatingFlag = g_atParaList[AT_OVERHEATINGCFG_FLAG].paraValue == 1 ? AT_MTA_OVERHEATING_QUIT :
                                                                                            AT_MTA_OVERHEATING_SET;

    if (overHeatingReq->overHeatingType == MTA_AT_RAT_MODE_LTE) {
        overHeatingReq->lteSetParamFlag = g_atParaList[AT_OVERHEATINGCFG_SETPARAMFLAG].paraValue;

        if (overHeatingReq->overHeatingFlag == AT_MTA_OVERHEATING_SET) {
            overHeatingReq->param1  = g_atParaList[AT_OVERHEATINGCFG_PARA1].paraValue;
            overHeatingReq->param2  = g_atParaList[AT_OVERHEATINGCFG_PARA2].paraValue;
            overHeatingReq->param3  = g_atParaList[AT_OVERHEATINGCFG_PARA3].paraValue;
            overHeatingReq->param4  = g_atParaList[AT_OVERHEATINGCFG_PARA4].paraValue;
        }
    } else {
        overHeatingReq->nrSetParamFlag = g_atParaList[AT_OVERHEATINGCFG_SETPARAMFLAG].paraValue;

        if (overHeatingReq->overHeatingFlag == AT_MTA_OVERHEATING_SET) {
            overHeatingReq->param5  = g_atParaList[AT_OVERHEATINGCFG_PARA1].paraValue;
            overHeatingReq->param6  = g_atParaList[AT_OVERHEATINGCFG_PARA2].paraValue;
            overHeatingReq->param7  = g_atParaList[AT_OVERHEATINGCFG_PARA3].paraValue;
            overHeatingReq->param8  = g_atParaList[AT_OVERHEATINGCFG_PARA4].paraValue;
            overHeatingReq->param9  = g_atParaList[AT_OVERHEATINGCFG_PARA5].paraValue;
            overHeatingReq->param10 = g_atParaList[AT_OVERHEATINGCFG_PARA6].paraValue;
            overHeatingReq->param11 = g_atParaList[AT_OVERHEATINGCFG_PARA7].paraValue;
            overHeatingReq->param12 = g_atParaList[AT_OVERHEATINGCFG_PARA8].paraValue;
            overHeatingReq->param13 = g_atParaList[AT_OVERHEATINGCFG_PARA9].paraValue;
            overHeatingReq->param14 = g_atParaList[AT_OVERHEATINGCFG_PARA10].paraValue;
        }
    }
}

VOS_BOOL AT_CheckModem0(VOS_UINT8 indexNum)
{
    MODEM_ID_ENUM_UINT16 modemId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        AT_ERR_LOG("Get modem id fail!");
        return VOS_FALSE;
    }

    if (modemId != MODEM_ID_0) {
        AT_ERR_LOG1("Modem is not modem0!", modemId);
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_SetOverHeatingCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_OverHeatingSetReq overHeatingReq;
    VOS_UINT32               result;

    (VOS_VOID)memset_s(&overHeatingReq, sizeof(overHeatingReq), 0x00, sizeof(overHeatingReq));

    /* 参数检查 */
    result = AT_CheckOverHeatingPara();
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetOverHeatingCfgPara: AT_CheckOverHeatingPara Failed");
        return result;
    }

    AT_FillOverHeatingReqParam(&overHeatingReq);

#if (CFG_FEATURE_MODEM1_SUPPORT_NR == FEATURE_OFF)
    if (overHeatingReq.overHeatingType == MTA_AT_RAT_MODE_NR) {
        /* 仅支持在Modem0上发起 */
        result = AT_CheckModem0(indexNum);
        if (result != VOS_TRUE) {
            AT_ERR_LOG("AT_SetOverHeatingCfgPara: AT_CheckModem0 Failed");
            return AT_ERROR;
        }
    }
#endif

    /* 发送跨核消息到C核 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_OVERHEATING_CFG_SET_REQ, (VOS_UINT8 *)&overHeatingReq,
                                    sizeof(overHeatingReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetOverHeatingCfgPara: AT_FillAndSndAppReqMsg failed.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_OVER_HEATING_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetOverHeatingQryParam(VOS_UINT8 indexNum)
{
    AT_MTA_OverHeatingQryReq overHeatingQry;
    VOS_UINT32               result;

    (VOS_VOID)memset_s(&overHeatingQry, sizeof(overHeatingQry), 0x00, sizeof(overHeatingQry));

    /* 类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 必带参数检查 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    overHeatingQry.overHeatingType = g_atParaList[0].paraValue == 1 ? MTA_AT_RAT_MODE_NR : MTA_AT_RAT_MODE_LTE;

    /* 发送跨核消息到C核 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_OVERHEATING_CFG_QRY_REQ, (VOS_UINT8 *)&overHeatingQry,
                                    sizeof(overHeatingQry), I0_UEPS_PID_MTA);
    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_QryOverHeating: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_OVER_HEATING_QRY;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_SetNrCaCellRptCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrCaCellInfoRptCfgSetReq caCellType;
    VOS_UINT32                      rst;

    (VOS_VOID)memset_s(&caCellType, sizeof(caCellType), 0x00, sizeof(caCellType));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数及长度检查 */
    if ((g_atParaIndex != AT_NRCACELLRPTCFG_PARA_NUM) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    caCellType.caCellRptFlg = g_atParaList[0].paraValue;

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NR_CA_CELL_INFO_RPT_CFG_SET_REQ, (VOS_UINT8 *)&caCellType, sizeof(caCellType), I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetNrCaCellRptCfgPara: AT_SetNrcaCellRptCfgPara fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRCACELLRPTCFG_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetNrNwCapRptCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrNwCapInfoRptCfgSetReq rptCfg;
    VOS_UINT32                   rst;

    (VOS_VOID)memset_s(&rptCfg, sizeof(rptCfg), 0x00, sizeof(rptCfg));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数个数及长度检查 */
    if ((g_atParaIndex != AT_NRNWCAPRPTCFG_PARA_NUM) || (g_atParaList[0].paraLen == 0) ||
        (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rptCfg.type = g_atParaList[0].paraValue;
    rptCfg.rptFlg = g_atParaList[1].paraValue;

    /* 发送跨核消息到C核 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NR_NW_CAP_INFO_RPT_CFG_SET_REQ, (VOS_UINT8 *)&rptCfg, sizeof(rptCfg), I0_UEPS_PID_MTA);

    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetNrNwCapRptCfgPara: AT_SetNrNwCapRptCfgPara fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRNWCAPRPTCFG_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetNrNwCapRptQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrNwCapInfoRptCfgQryReq rptCfgQry;
    VOS_UINT32 rst;

    (VOS_VOID)memset_s(&rptCfgQry, sizeof(rptCfgQry), 0x00, sizeof(rptCfgQry));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数个数及长度检查 */
    if ((g_atParaIndex != AT_NRNWCAPRPTQRY_PARA_NUM) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体，发送消息到MTA */
    rptCfgQry.type = g_atParaList[0].paraValue;
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NR_NW_CAP_INFO_RPT_CFG_QRY_REQ, (VOS_UINT8 *)&rptCfgQry, sizeof(rptCfgQry), I0_UEPS_PID_MTA);
    if (rst != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetNrNwCapRptQryPara: AT_SetNrNwCapRptQryPara fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRNWCAPRPTQRY_SET;
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetNrNwCapQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrNwCapInfoQryReq qryReq;
    VOS_UINT32 result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&qryReq, sizeof(AT_MTA_NrNwCapInfoQryReq), 0x00, sizeof(AT_MTA_NrNwCapInfoQryReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNrNwCapQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为1个,且第1个参数长度不应为0，否则返回AT_CME_INCORRECT_PARAMETERS */
    if ((g_atParaIndex != AT_NRNWCAPQRY_PARA_NUM) || (g_atParaList[0].paraLen == 0)) {
        AT_WARN_LOG("AT_SetNrNwCapQryPara: At Cmd para num or len error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体，发送消息到MTA */
    qryReq.type = g_atParaList[0].paraValue;
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NR_NW_CAP_INFO_QRY_REQ, (VOS_UINT8 *)&qryReq, sizeof(AT_MTA_NrNwCapInfoQryReq), I0_UEPS_PID_MTA);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRNWCAPQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
LOCAL VOS_UINT32 AT_CheckNrrcCapCfgPara(VOS_VOID)
{
    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 不携带参数，返回AT_CME_INCORR ECT_PARAMETERS */
    if (g_atParaList[AT_NRRCCAPCFG_NRRCCFGMODE].paraLen == 0) {
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara: First Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

     /* 判断参数数量 */
    if ((g_atParaIndex < AT_NRRCCAPCFG_MIN_PARA_NUM)) {
        /* 参数数量错误 */
        AT_WARN_LOG("AT_CheckNrrcCapCfgPara : Current Number wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetNrrcCapCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrrccapCfgSetReq setNrrcCapCfgReq;
    VOS_UINT32              result;
    VOS_UINT32              i;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setNrrcCapCfgReq, sizeof(setNrrcCapCfgReq), 0x00, sizeof(setNrrcCapCfgReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    result = AT_CheckNrrcCapCfgPara();
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充结构体 */
    setNrrcCapCfgReq.nrrcCfgNetMode = g_atParaList[0].paraValue;
    setNrrcCapCfgReq.paraNum = (VOS_UINT32)(g_atParaIndex - 1);
    for (i = 0; i < AT_MTA_NRRCCAP_PARA_MAX_NUM; i++) {
        setNrrcCapCfgReq.para[i] = (g_atParaList + i + 1)->paraValue;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NRRCCAP_CFG_SET_REQ, (VOS_UINT8 *)&setNrrcCapCfgReq,
                                    sizeof(AT_MTA_NrrccapCfgSetReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRRCCAPCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

LOCAL VOS_UINT32 AT_CheckAutoN2LOptParaLen(VOS_VOID)
{
    if ((g_atParaList[AT_AUTON2LOPTCFG_SWITCH_ENABLE].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_PARA_ENABLE].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_IDLE_SWITCH].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_CONN_SWITCH].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_RSRP_NR_LOW].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_SINR_NR_LOW].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_RSRP_LTE_HIGH].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_SINR_LTE_HIGH].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_RSRP_L2N_JDG].paraLen == 0) ||
        (g_atParaList[AT_AUTON2LOPTCFG_SINR_L2N_JDG].paraLen == 0) ) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

LOCAL VOS_UINT32 AT_GetValidStringPara(AT_ParseParaType *atParaList, VOS_INT16 *paraValue)
{
    VOS_UINT32 transValue = 0;

    /* 字符串参数为'-'，表示输入为负数 */
    if (atParaList->para[0] == '-') {
        if (atAuc2ul(&atParaList->para[1], atParaList->paraLen - 1, &transValue) != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (transValue > AT_INT16_ABSOLUTE_MIN_VALUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* 由于值为负，所以转换下 */
        *paraValue = (VOS_INT16)((-1) * (VOS_INT16)transValue);
    } else {
        if (atAuc2ul(&atParaList->para[0], atParaList->paraLen, &transValue) != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (transValue > AT_INT16_MAX_VALUE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        *paraValue = (VOS_INT16)transValue;
    }

    return AT_OK;
}

LOCAL VOS_UINT32 AT_GetValidAutoN2LOptPara(AT_MTA_AutoN2lOptCfgSetReq *setAutoN2LOptCfgReq)
{
    if (AT_GetValidStringPara(&g_atParaList[AT_AUTON2LOPTCFG_RSRP_NR_LOW], &(setAutoN2LOptCfgReq->rsrpNrLowThresh)) !=
        AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetValidStringPara(&g_atParaList[AT_AUTON2LOPTCFG_SINR_NR_LOW], &(setAutoN2LOptCfgReq->sinrNrLowThresh)) !=
        AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetValidStringPara(&g_atParaList[AT_AUTON2LOPTCFG_RSRP_LTE_HIGH],
        &(setAutoN2LOptCfgReq->rsrpLteHighThresh)) != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetValidStringPara(&g_atParaList[AT_AUTON2LOPTCFG_SINR_LTE_HIGH],
        &(setAutoN2LOptCfgReq->sinrLteHighThresh)) != AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetValidStringPara(&g_atParaList[AT_AUTON2LOPTCFG_RSRP_L2N_JDG], &(setAutoN2LOptCfgReq->rsrpL2nJdgDelta)) !=
        AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (AT_GetValidStringPara(&g_atParaList[AT_AUTON2LOPTCFG_SINR_L2N_JDG], &(setAutoN2LOptCfgReq->sinrL2nJdgDelta)) !=
        AT_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_SetAutoN2LOptCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_AutoN2lOptCfgSetReq setAutoN2LOptCfgReq;
    VOS_UINT32                 result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setAutoN2LOptCfgReq, sizeof(setAutoN2LOptCfgReq), 0x00, sizeof(setAutoN2LOptCfgReq));

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckAutoN2LOptCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 判断参数数量 */
    if ((g_atParaIndex != AT_AUTON2LOPTCFG_MAX_PARA_NUM)) {
        /* 参数数量错误 */
        AT_WARN_LOG("AT_CheckAutoN2LOptCfgPara : Current Number wrong.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数长度检查,不合法直接返回失败 */
    result = AT_CheckAutoN2LOptParaLen();
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充结构体 */
    result = AT_GetValidAutoN2LOptPara(&setAutoN2LOptCfgReq);
    if (result != AT_OK) {
        AT_WARN_LOG("AT_AutoN2LOptCfgParaCheck: Check para fail.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    setAutoN2LOptCfgReq.switchChgFlg      = (VOS_UINT8)g_atParaList[AT_AUTON2LOPTCFG_SWITCH_ENABLE].paraValue;
    setAutoN2LOptCfgReq.paraChgFlg        = (VOS_UINT8)g_atParaList[AT_AUTON2LOPTCFG_PARA_ENABLE].paraValue;
    setAutoN2LOptCfgReq.autoOptIdleSwitch = (VOS_UINT8)g_atParaList[AT_AUTON2LOPTCFG_IDLE_SWITCH].paraValue;
    setAutoN2LOptCfgReq.autoOptConnSwitch = (VOS_UINT8)g_atParaList[AT_AUTON2LOPTCFG_CONN_SWITCH].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_AUTON2LOPT_CFG_SET_REQ, (VOS_UINT8 *)&setAutoN2LOptCfgReq, sizeof(AT_MTA_AutoN2lOptCfgSetReq),
        I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_AUTON2LOPTCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetNrrcCapQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrrccapQryReq setNrrcCapQryReq;
    VOS_UINT32           result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setNrrcCapQryReq, sizeof(setNrrcCapQryReq), 0x00, sizeof(setNrrcCapQryReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNrrcCapQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为1个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetNrrcCapQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数长度不能为0，否则返回AT_CME_INCORR ECT_PARAMETERS */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetNrrcCapQryPara: First Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体 */
    setNrrcCapQryReq.nrrcCfgNetMode = g_atParaList[0].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NRRCCAP_QRY_REQ, (VOS_UINT8 *)&setNrrcCapQryReq,
                                    sizeof(AT_MTA_NrrccapQryReq), I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRRCCAPQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetNrrcUeCapPara(VOS_UINT8 indexNum)
{
    AT_MTA_UeCapParamSet *setUeCap = VOS_NULL_PTR;
    VOS_UINT32            paraLen  = 0;
    VOS_UINT32            msgLen;
    VOS_UINT32            result;
    VOS_UINT32            i;

    /* 参数检查 */
    result = AT_CheckUeCapPara();
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetNrrcUeCapPara: AT_CheckUeCapPara Failed");
        return result;
    }

#if (CFG_FEATURE_MODEM1_SUPPORT_NR == FEATURE_OFF)
    /* 仅支持在Modem0上发起 */
    result = AT_CheckModem0(indexNum);
    if (result != VOS_TRUE) {
        AT_ERR_LOG("AT_SetNrrcUeCapPara: AT_CheckModem0 Failed");
        return AT_ERROR;
    }
#endif
    for (i = AT_NRRCUECAPPARAMSET_PARA1_INDEX; i < (VOS_UINT32)TAF_MIN(g_atParaIndex, AT_MAX_PARA_NUMBER); i++) {
        paraLen += (VOS_UINT32)g_atParaList[i].paraLen / 2;
    }

    msgLen   = sizeof(AT_MTA_UeCapParamSet) + paraLen;
    setUeCap = (AT_MTA_UeCapParamSet *)PS_MEM_ALLOC(WUEPS_PID_AT, msgLen);

    /* 内存申请失败，返回AT_ERROR */
    if (setUeCap == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SetNrrcUeCapPara:memory alloc fail.");
        return AT_ERROR;
    }

    (VOS_VOID)memset_s((VOS_INT8 *)setUeCap, msgLen, 0x00, msgLen);

    /* 填写消息内容 */
    setUeCap->ratMode = AT_MTA_RAT_MODE_NR;
    result            = AT_FillUeCapPara(setUeCap, paraLen);
    if (result != AT_SUCCESS) {
        AT_ERR_LOG("AT_SetNrrcUeCapPara: AT_FillUeCapPara Failed");
        PS_MEM_FREE(WUEPS_PID_AT, setUeCap);
        return result;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                    ID_AT_MTA_NRRC_UE_CAP_PARAM_SET_REQ, (VOS_UINT8 *)setUeCap, msgLen,
                                    I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        AT_WARN_LOG("AT_SetNrrcUeCapPara: AT_FillAndSndAppReqMsg failed.");
        PS_MEM_FREE(WUEPS_PID_AT, setUeCap);
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRRC_UE_CAP_SET;
    PS_MEM_FREE(WUEPS_PID_AT, setUeCap);

    return AT_WAIT_ASYNC_RETURN;
}

LOCAL VOS_UINT32 AT_CheckNrPowerSavingCfgPara(VOS_VOID)
{
    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_CheckNrPowerSavingCfgPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 不携带参数，返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaList[AT_NRPOWERSAVINGCFG_CMDTYPE].paraLen == 0) {
        AT_WARN_LOG("AT_CheckNrPowerSavingCfgPara: First Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

     /* 判断参数数量 */
    if (g_atParaIndex < AT_NRPOWERSAVINGCFG_MIN_PARA_NUM || g_atParaIndex > AT_NRPOWERSAVINGCFG_MAX_PARA_NUM) {
        /* 参数数量错误 */
        AT_WARN_LOG("AT_CheckNrPowerSavingCfgPara : Wrong Number of Parameters.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetNrPowerSavingCfgPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrPowerSavingCfgSetReq setNrPowerSavingCfgReq;
    VOS_UINT32 result;
    VOS_UINT32 i;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setNrPowerSavingCfgReq, sizeof(setNrPowerSavingCfgReq), 0x00, sizeof(setNrPowerSavingCfgReq));

    /* 参数个数和合法性检查,不合法直接返回失败 */
    result = AT_CheckNrPowerSavingCfgPara();
    if (result != AT_SUCCESS) {
        return result;
    }

    /* 填充结构体 */
    setNrPowerSavingCfgReq.type = g_atParaList[0].paraValue;
    setNrPowerSavingCfgReq.paraNum = (VOS_UINT32)(g_atParaIndex - 1);
    for (i = 0; i < setNrPowerSavingCfgReq.paraNum; i++) {
        setNrPowerSavingCfgReq.para[i] = g_atParaList[i + 1].paraValue;
    }

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NRPOWERSAVING_CFG_SET_REQ, (VOS_UINT8 *)&setNrPowerSavingCfgReq, sizeof(setNrPowerSavingCfgReq),
        I0_UEPS_PID_MTA);
    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRPOWERSAVINGCFG_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetNrPowerSavingQryPara(VOS_UINT8 indexNum)
{
    AT_MTA_NrPowerSavingQryReq setNrPowerSavingQryReq;
    VOS_UINT32                 result;

    /* 局部变量初始化 */
    (VOS_VOID)memset_s(&setNrPowerSavingQryReq, sizeof(setNrPowerSavingQryReq), 0x00, sizeof(setNrPowerSavingQryReq));

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetNrPowerSavingQryPara: At Cmd Opt Set Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数应为1个，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetNrPowerSavingQryPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 第一个参数长度不能为0，否则返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaList[0].paraLen == 0) {
        AT_WARN_LOG("AT_SetNrPowerSavingQryPara: First Para Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填充结构体 */
    setNrPowerSavingQryReq.type = g_atParaList[0].paraValue;

    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
        ID_AT_MTA_NRPOWERSAVING_QRY_REQ, (VOS_UINT8 *)&setNrPowerSavingQryReq, sizeof(setNrPowerSavingQryReq),
        I0_UEPS_PID_MTA);

    if (result == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NRPOWERSAVINGQRY_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif

VOS_UINT32 AT_SetNCellMonitorPara(VOS_UINT8 indexNum)
{
    AT_MTA_NcellMonitorSetReq atCmd;
    VOS_UINT32                result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.switchFlag = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 给MTA发送^NCELLMONITOR设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_NCELL_MONITOR_SET_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_NcellMonitorSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NCELL_MONITOR_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 At_SetLFromConnToIdlePara(VOS_UINT8 indexNum)
{
    VOS_UINT32                   ret;
    L4A_SET_LTE_TO_IDLE_REQ_STRU req            = {0};
    LPS_SWITCH_PARA_STRU         drxControlFlag = {0};

    req.ctrl.clientId = g_atClientTab[indexNum].clientId;
    req.ctrl.opId     = 0;
    req.ctrl.pid      = WUEPS_PID_AT;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (TAF_ACORE_NV_READ(MODEM_ID_0, EN_NV_ID_SWITCH_PARA, &drxControlFlag, sizeof(LPS_SWITCH_PARA_STRU)) != NV_OK) {
        return AT_ERROR;
    }

    if (((*((VOS_UINT32 *)(&(drxControlFlag.stPsFunFlag01)))) & LPS_NV_JP_DCOM_CON_TO_IDLE_BIT) == 0) {
        return AT_OK;
    }

    /* L模调用如下接口转发 */
    ret = atSendL4aDataMsg(g_atClientTab[indexNum].clientId, I0_MSP_L4_L4A_PID, ID_MSG_L4A_LTE_TO_IDLE_REQ,
                           (VOS_UINT8 *)(&req), sizeof(L4A_SET_LTE_TO_IDLE_REQ_STRU));

    if (ret == ERR_MSP_SUCCESS) {
        return AT_OK;
    }
    return AT_ERROR;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
TAF_UINT32 At_SetRRCVersion(TAF_UINT8 indexNum)
{
    VOS_UINT8  rRCVersion;
    VOS_UINT32 rst;

    /* 参数过多返回参数错误 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rRCVersion = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 发送消息DRV_AGENT_RRC_VERSION_SET_REQ给AT代理处理 */

    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_WRR_RRC_VERSION_SET_REQ, &rRCVersion,
                                 sizeof(VOS_UINT8), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_RRC_VERSION_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

