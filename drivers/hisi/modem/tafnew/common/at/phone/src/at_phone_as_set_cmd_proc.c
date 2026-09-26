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
#include "at_phone_as_set_cmd_proc.h"
#include "securec.h"
#include "taf_app_mma.h"
#include "at_ctx.h"
#include "at_cmd_proc.h"
#include "taf_ccm_api.h"
#include "at_data_proc.h"
#include "at_msg_print.h"
#include "at_phone_comm.h"
#include "at_mdrv_interface.h"
#include "gunas_errno.h"
#include "at_check_func.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_AS_SET_CMD_PROC_C

#define AT_BOOT_VAILD_PARA_NUM1 1
#define AT_BOOT_VAILD_PARA_NUM2 2
#define AT_FREQLOCK_PARA_NUM 6
#define AT_FREQLOCK_ENABLE_VAILD_VALUE 1
#define AT_FREQLOCK_FREQ_VAILD_LEN 2
#define AT_FREQLOCK_BAND_VAILD_LEN 2
#define AT_FREQLOCK_BAND 3

#define AT_GFREQLOCK_PARA_MIN_NUM 1
#define AT_GFREQLOCK_PARA_MAX_NUM 3
#define AT_GFREQLOCK_ENABLE_FLAG 0
#define AT_GFREQLOCK_FREQ 1
#define AT_GFREQLOCK_BAND 2
#define AT_FREQLOCK_ENABLE 0
#define AT_FREQLOCK_FREQ 1
#define AT_FREQLOCK_MODE 2
#define AT_FREQLOCK_PSC 4
#define AT_FREQLOCK_PCI 5
#define AT_FREQLOCK_GSMMAX 1023

TAF_UINT32 At_SetCSNR(TAF_UINT8 indexNum)
{
    /* 参数过多 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

TAF_UINT32 At_SetCellInfoPara(TAF_UINT8 indexNum)
{
    VOS_UINT32 atCmdCellInfo;
    VOS_UINT32 rst;

    /* 参数过多 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    atCmdCellInfo = g_atParaList[0].paraValue;

    /* 发送消息ID_AT_MTA_WRR_CELLINFO_QRY_REQ到C核AT AGENT处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_WRR_CELLINFO_QRY_REQ,
                                 (VOS_UINT8 *)&atCmdCellInfo, sizeof(VOS_UINT32), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CELLINFO_QUERY;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCellSearch(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;
    VOS_UINT8  cellSrh;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_ERROR;
    }

    cellSrh = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 发送消息 ID_AT_MTA_WRR_CELLSRH_SET_REQ 给 AT AGENT 处理，该消息带参数(VOS_UINT8)g_atParaList[0].ulParaValue */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, At_GetOpId(), ID_AT_MTA_WRR_CELLSRH_SET_REQ,
                                 &cellSrh, sizeof(cellSrh), I0_UEPS_PID_MTA);

    if (rst == AT_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_WAS_MNTN_SET_CELLSRH;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
TAF_UINT32 At_SetFreqLock(TAF_UINT8 indexNum)
{
    MTA_AT_WrrFreqlockCtrl freqLock;
    VOS_UINT32             rst;

    /* 参数个数不正确则返回参数错误 */
    if ((g_atParaIndex != AT_BOOT_VAILD_PARA_NUM1) && (g_atParaIndex != AT_BOOT_VAILD_PARA_NUM2)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * 如果第一个参数是锁频,但第二个参数频率值没有设置或
     * 设置的值无效则返回错误
     */
    if ((g_atParaList[0].paraValue == 1) && (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息赋值 */
    (VOS_VOID)memset_s(&freqLock, sizeof(freqLock), 0x00, sizeof(MTA_AT_WrrFreqlockCtrl));
    freqLock.freqLockEnable = (VOS_UINT8)g_atParaList[0].paraValue;
    freqLock.lockedFreq     = (VOS_UINT16)g_atParaList[1].paraValue;

    /* 发送消息 ID_AT_MTA_WRR_FREQLOCK_SET_REQ 给C核 AT 代理处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_WRR_FREQLOCK_SET_REQ,
                                 (VOS_UINT8 *)&freqLock, sizeof(MTA_AT_WrrFreqlockCtrl), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_FREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetGFreqLock(VOS_UINT8 indexNum)
{
    AT_MTA_SetGsmFreqlockReq gFreqLockInfo;
    VOS_UINT32               rst;

    /* 参数个数检查 */
    if ((g_atParaIndex != AT_GFREQLOCK_PARA_MIN_NUM) && (g_atParaIndex != AT_GFREQLOCK_PARA_MAX_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 初始化 */
    (VOS_VOID)memset_s(&gFreqLockInfo, sizeof(gFreqLockInfo), 0x00, sizeof(gFreqLockInfo));

    /* 参数有效性检查 */
    /* 第一个参数必须带 */
    if (g_atParaList[AT_GFREQLOCK_ENABLE_FLAG].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    gFreqLockInfo.enableFlag = (PS_BOOL_ENUM_UINT8)g_atParaList[0].paraValue;

    /* 若启动锁频，则必须带第二个参数和第三个参数 */
    if (gFreqLockInfo.enableFlag == PS_TRUE) {
        if ((g_atParaList[AT_GFREQLOCK_FREQ].paraLen == 0) || (g_atParaList[AT_GFREQLOCK_BAND].paraLen == 0)) {
            return AT_CME_INCORRECT_PARAMETERS;
        } else {
            gFreqLockInfo.freq = (VOS_UINT16)g_atParaList[AT_GFREQLOCK_FREQ].paraValue;
            gFreqLockInfo.band = (AT_MTA_GsmBandUint16)g_atParaList[AT_GFREQLOCK_BAND].paraValue;
        }
    }

    /* 发送消息给C核处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_GSM_FREQLOCK_REQ,
                                 (VOS_UINT8 *)&gFreqLockInfo, sizeof(AT_MTA_SetGsmFreqlockReq), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_GSM_FREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif

#if (FEATURE_PROBE_FREQLOCK == FEATURE_ON)

VOS_UINT32 AT_CheckM2MFreqLockParaNum(VOS_VOID)
{
    /* <enable>参数为0时，参数个数应该为1 */
    if (g_atParaList[0].paraValue == 0) {
        if (g_atParaIndex == 1) {
            return AT_SUCCESS;
        } else {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* <enable>参数为1时，设置命令参数个数应该为6 */
    if ((g_atParaList[AT_FREQLOCK_ENABLE].paraValue == AT_FREQLOCK_ENABLE_VAILD_VALUE) &&
        (g_atParaIndex != AT_FREQLOCK_PARA_NUM)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckM2MFreqLockParaMode(VOS_VOID)
{
    /* 转换<MODE>参数为无符号整型 */
    if (At_Auc2ul(g_atParaList[1].para, g_atParaList[1].paraLen, &g_atParaList[1].paraValue) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* <mode>参数为字符串类型，取值为"01"--"04"，长度为2位 */
    if ((g_atParaList[AT_FREQLOCK_FREQ].paraLen != AT_FREQLOCK_FREQ_VAILD_LEN) ||
        ((g_atParaList[AT_FREQLOCK_FREQ].paraValue < AT_MTA_M2M_FREQLOCK_MODE_GSM) ||
         (g_atParaList[AT_FREQLOCK_FREQ].paraValue > AT_MTA_M2M_FREQLOCK_MODE_LTE))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckM2MFreqLockParaBand(VOS_VOID)
{
    /* <Band>参数仅在GSM制式下有效 */
    if (g_atParaList[1].paraValue == AT_MTA_M2M_FREQLOCK_MODE_GSM) {
        /* <band>参数为字符串类型，取值为"00"--"03"，长度为2位 */
        if (g_atParaList[AT_FREQLOCK_BAND].paraLen != AT_FREQLOCK_BAND_VAILD_LEN) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* <band>参数为字符串类型，带双引号,字符串长度为4 */
        if (At_Auc2ul(g_atParaList[AT_FREQLOCK_BAND].para, g_atParaList[AT_FREQLOCK_BAND].paraLen,
                      &g_atParaList[AT_FREQLOCK_BAND].paraValue) == AT_FAILURE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        if (g_atParaList[AT_FREQLOCK_BAND].paraValue > AT_MTA_GSM_BAND_1900) {
            return AT_CME_INCORRECT_PARAMETERS;
        }

        /* GSM有效频点范围: 0-1023 */
        if (g_atParaList[AT_FREQLOCK_MODE].paraValue > AT_FREQLOCK_GSMMAX) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        /* 非GSM制式下，<Band>参数须为空 */
        if (g_atParaList[AT_FREQLOCK_BAND].paraLen != 0) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckM2MFreqLockParaPsc(VOS_VOID)
{
    /* <PSC>参数仅在WCDMA、TDS-CDMA制式下有效 */
    if ((g_atParaList[AT_FREQLOCK_PSC].paraLen != 0) &&
        ((g_atParaList[AT_FREQLOCK_FREQ].paraValue != AT_MTA_M2M_FREQLOCK_MODE_WCDMA) &&
         (g_atParaList[AT_FREQLOCK_FREQ].paraValue != AT_MTA_M2M_FREQLOCK_MODE_TDSCDMA))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckM2MFreqLockParaPci(VOS_VOID)
{
    /* <PCI>参数仅在LTE制式下有效 */
    if ((g_atParaList[AT_FREQLOCK_PCI].paraLen != 0) &&
        (g_atParaList[AT_FREQLOCK_FREQ].paraValue != AT_MTA_M2M_FREQLOCK_MODE_LTE)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    return AT_OK;
}

VOS_UINT32 AT_CheckM2MFreqLockPara(VOS_UINT8 clientId)
{
    VOS_UINT32             rst;

    rst = AT_CheckM2MFreqLockParaNum();
    if (rst != AT_OK) {
        return rst;
    }

    rst = AT_CheckM2MFreqLockParaMode();
    if (rst != AT_OK) {
        return rst;
    }

    rst = AT_CheckM2MFreqLockParaBand();
    if (rst != AT_OK) {
        return rst;
    }

    rst = AT_CheckM2MFreqLockParaPsc();
    if (rst != AT_OK) {
        return rst;
    }

    rst = AT_CheckM2MFreqLockParaPci();
    if (rst != AT_OK) {
        return rst;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_GetM2MGFreqLockPara(AT_MTA_SetM2MFreqlockReq *freqLockInfo)
{
    if (g_atParaList[AT_FREQLOCK_BAND].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    } else {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_GSM;
        /* <freq>频点参数 */
        freqLockInfo->gFreqPara.freq = (VOS_UINT16)g_atParaList[AT_FREQLOCK_MODE].paraValue;
        /* <band>频段参数 */
        freqLockInfo->gFreqPara.band = (VOS_UINT16)g_atParaList[AT_FREQLOCK_BAND].paraValue;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_GetM2MWFreqLockPara(AT_MTA_SetM2MFreqlockReq *freqLockInfo)
{
    if (g_atParaList[AT_FREQLOCK_PSC].paraLen == 0) {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_WCDMA;
        /* FREQ_ONLY类型 */
        freqLockInfo->wFreqPara.freqType = AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY;
        /* <freq>频点参数 */
        freqLockInfo->wFreqPara.freq = (VOS_UINT16)g_atParaList[AT_FREQLOCK_MODE].paraValue;
    } else {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_WCDMA;
        /* FREQ + PSC类型 */
        freqLockInfo->wFreqPara.freqType = AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_COMBINED;
        /* <freq>频点参数 */
        freqLockInfo->wFreqPara.freq = (VOS_UINT16)g_atParaList[AT_FREQLOCK_MODE].paraValue;
        /* 主扰码参数 */
        freqLockInfo->wFreqPara.psc = (VOS_UINT16)g_atParaList[AT_FREQLOCK_PSC].paraValue;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_GetM2MTdFreqLockPara(AT_MTA_SetM2MFreqlockReq *freqLockInfo)
{
    if (g_atParaList[AT_FREQLOCK_PSC].paraLen == 0) {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_TDSCDMA;
        /* FREQ_ONLY类型 */
        freqLockInfo->tdFreqPara.freqType = AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY;
        /* <freq>频点参数 */
        freqLockInfo->tdFreqPara.freq = (VOS_UINT16)g_atParaList[AT_FREQLOCK_MODE].paraValue;
    } else {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_TDSCDMA;
        /* FREQ + SC类型 */
        freqLockInfo->tdFreqPara.freqType = AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_COMBINED;
        /* <freq>频点参数 */
        freqLockInfo->tdFreqPara.freq = (VOS_UINT16)g_atParaList[AT_FREQLOCK_MODE].paraValue;
        /* 主扰码参数 */
        freqLockInfo->tdFreqPara.sc = (VOS_UINT16)g_atParaList[AT_FREQLOCK_PSC].paraValue;
    }
    return AT_SUCCESS;
}

VOS_UINT32 AT_GetM2MLFreqLockPara(AT_MTA_SetM2MFreqlockReq *freqLockInfo)
{
    if (g_atParaList[AT_FREQLOCK_PCI].paraLen == 0) {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_LTE;
        /* FREQ_ONLY类型 */
        freqLockInfo->lFreqPara.freqType = AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_ONLY;
        /* <freq>频点参数 */
        freqLockInfo->lFreqPara.freq = g_atParaList[AT_FREQLOCK_MODE].paraValue;
    } else {
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_ON;
        freqLockInfo->mode       = AT_MTA_M2M_FREQLOCK_MODE_LTE;
        /* FREQ + PCI类型 */
        freqLockInfo->lFreqPara.freqType = AT_MTA_M2M_FREQLOCK_TYPE_ENUM_FREQ_COMBINED;
        /* <freq>频点参数 */
        freqLockInfo->lFreqPara.freq = g_atParaList[AT_FREQLOCK_MODE].paraValue;
        /* 物理小区ID参数 */
        freqLockInfo->lFreqPara.pci = (VOS_UINT16)g_atParaList[AT_FREQLOCK_PCI].paraValue;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_GetM2MFreqLockPara(VOS_UINT8 clientId, AT_MTA_SetM2MFreqlockReq *freqLockInfo)
{
    if (g_atParaList[0].paraValue == 0) {
        /* 锁频功能关闭 */
        freqLockInfo->enableFlag = AT_MTA_M2M_FREQLOCK_FLAG_TYPE_OFF;
        return AT_SUCCESS;
    } else {
        /* GSM模式时，<band>参数为必选项，第5个参数无意义；WCDMA模式时，<band>参数无意义，根据<psc>是否为空，区分W锁频类型:锁频、锁频+扰码 */
        switch (g_atParaList[1].paraValue) {
            case AT_MTA_M2M_FREQLOCK_MODE_GSM:
                return AT_GetM2MGFreqLockPara(freqLockInfo);

            case AT_MTA_M2M_FREQLOCK_MODE_WCDMA:
                return AT_GetM2MWFreqLockPara(freqLockInfo);

            case AT_MTA_M2M_FREQLOCK_MODE_TDSCDMA:
                return AT_GetM2MTdFreqLockPara(freqLockInfo);

            case AT_MTA_M2M_FREQLOCK_MODE_LTE:
                return AT_GetM2MLFreqLockPara(freqLockInfo);

            default:
                return AT_CME_INCORRECT_PARAMETERS;
        }
    }
}

TAF_UINT32 At_SetM2MFreqLock(TAF_UINT8 indexNum)
{
    AT_MTA_SetM2MFreqlockReq freqLockInfo;

    VOS_UINT32 rst;

    /* 初始化 */
    (VOS_VOID)memset_s(&freqLockInfo, sizeof(freqLockInfo), 0x00, sizeof(AT_MTA_SetM2MFreqlockReq));

    /* 命令类型检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数有效性检查 */
    rst = AT_CheckM2MFreqLockPara(indexNum);

    if (rst != AT_SUCCESS) {
        return rst;
    }

    /* 获取用户锁频配置信息 */
    rst = AT_GetM2MFreqLockPara(indexNum, &freqLockInfo);

    if (rst != AT_SUCCESS) {
        return rst;
    }

    /* 发送消息 ID_AT_MTA_FREQLOCK_SET_REQ 给C核 AT 代理处理 */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_SET_M2M_FREQLOCK_REQ,
                                 (VOS_UINT8 *)&freqLockInfo, sizeof(AT_MTA_SetM2MFreqlockReq), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_M2M_FREQLOCK_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif /* MBB_WPG_FREQLOCK == FEATURE_ON */

#if (FEATURE_DSDS == FEATURE_ON)
VOS_UINT32 At_SetPsProtectModePara(VOS_UINT8 indexNum)
{
    VOS_UINT32             rst;
    AT_MTA_RrcProtectPsReq psProtectSetPara;

    /* 结构体重置 */
    (VOS_VOID)memset_s(&psProtectSetPara, sizeof(psProtectSetPara), 0x00, sizeof(AT_MTA_RrcProtectPsReq));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    /* 参数为空 */
    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    psProtectSetPara.psProtectFlg = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 向MTA发送消息通知PS PROTECT */
    rst = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, At_GetOpId(), ID_AT_MTA_RRC_PROTECT_PS_REQ,
                                 (VOS_UINT8 *)(&psProtectSetPara), sizeof(psProtectSetPara), I0_UEPS_PID_MTA);

    if (rst == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_PSPROTECTMODE_SET;
        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}
#endif


