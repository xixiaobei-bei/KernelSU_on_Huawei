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

#include "at_cdma_set_cmd_proc.h"
#include "securec.h"
#include "at_set_para_cmd.h"
#include "AtParse.h"
#include "taf_drv_agent.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "taf_ccm_api.h"
#include "at_file_handle.h"
#include "at_msg_print.h"
#include "ppp_interface.h"
#include "nv_stru_cas.h"
#include "at_mta_interface.h"
#include "at_data_proc.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_CDMA_SET_CMD_PROC_C

#define AT_ECKMC_KEY 1
#define AT_CBURSTDTMF_PARA_NUM 4
#define AT_CBURSTDTMF_CALL_ID 0
#define AT_CBURSTDTMF_DTMF_KEY 1
#define AT_CBURSTDTMF_ON_LENGTH 2
#define AT_CBURSTDTMF_OFF_LENGTH 3

#define AT_CCONTDTMF_DTMF_KEY_VALID_LEN 1
#define AT_CCONTDTMF_DTMF_KEY_SWITCH_STOP 0
#define AT_CCONTDTMF_DTMF_KEY 2

#define AT_CSIDLIST_TRUST_LIST_NUM 0
#define AT_CSIDLIST_PARA_OFFSET_1 1
#define AT_CSIDLIST_PARA_OFFSET_2 2

#define AT_CFREQLOCK_SID 1
#define AT_CFREQLOCK_NID 2
#define AT_CFREQLOCK_CDMA_BAND_CLASS 3
#define AT_CFREQLOCK_CDMA_FREQ 4
#define AT_CFREQLOCK_CDMA_PN 5
#define AT_CFREQLOCK_EVDO_BAND_CLASS 6
#define AT_CFREQLOCK_EVDO_FREQ 7
#define AT_CFREQLOCK_EVDO_PN 8

#define AT_CDMACSQ_MODE 0
#define AT_CDMACSQ_TIME_INTERVAL 1
#define AT_CDMACSQ_RSSI_RPT_THRE 2
#define AT_CDMACSQ_ECIO_RPT_THRE 3
#define AT_CDMACSQ_MIN_PARA_NUM 4

#define AT_HDRCSQ_PARA_NUM 5
#define AT_HDRCSQ_MODE 0
#define AT_HDRCSQ_TIME_INTERVAL 1
#define AT_HDRCSQ_RSSI_RPT_THRE 2
#define AT_HDRCSQ_SNR_RPT_THRE 3
#define AT_HDRCSQ_ECIO_RPT_THRE 4

#define AT_ECKMC_VER 0
#define AT_ECKMC_PARA_NUM 2

#define AT_CSIDLIST_MAX_PARA_NUMBER 16

#define AT_CTOOSCOUNT_PARA_NUM 2
#define AT_CTOOSCOUNT_CL_COUNT 0
#define AT_CTOOSCOUNT_GUL_COUNT 1

#define AT_DOSIGMASK_DO_SIG_MASK 0
#define AT_DOSIGMASK_DO_SIG_MASK_VALID 10
#define AT_MEID_PARA_MEIDNUM 0
#define AT_DOSYSEVENT_PARA_NUM 1
#define AT_DOSYSEVENT_DO_SYS_EVENT 0
#define AT_DOSYSEVENT_DO_SYS_EVENT_VALID 10
#define AT_DOSIGMASK_PARA_NUM 1

#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
#define AT_CAGPSFORWARDDATA_PARA_MAX_NUM 5
#define AT_CAGPSFORWARDDATA_PARA_SERVER_MODE 0      /* CAGPSFORWARDDATA的第一个参数SERVER_MODE */
#define AT_CAGPSFORWARDDATA_PARA_TOTAL_SEGMENTS 1   /* CAGPSFORWARDDATA的第二个参数TOTAL_SEGMENTS */
#define AT_CAGPSFORWARDDATA_PARA_CURRENT_SEGMENTS 2 /* CAGPSFORWARDDATA的第三个参数CURRENT_SEGMENTS */
#define AT_CAGPSFORWARDDATA_PARA_DATA_LEN 3         /* CAGPSFORWARDDATA的第四个参数DATA_LEN */
#define AT_CAGPSFORWARDDATA_PARA_DATA 4             /* CAGPSFORWARDDATA的第五个参数DATA */

#define AT_CGPSCONTROLSTART_PARA_NUM 2
#define AT_CGPSCONTROLSTART_DATA_MAX_LEN 4
#define AT_CGPSCONTROLSTART_DATA_LEN 0
#define AT_CGPSCONTROLSTART_DATA 1

#define AT_CGPSCONTROLSTOP_PARA_NUM 2
#define AT_CGPSCONTROLSTOP_DATA_MAX_LEN 4
#define AT_CGPSCONTROLSTOP_DATA_LEN 0
#define AT_CGPSCONTROLSTOP_DATA 1

#define AT_CUPBINDSTATUS_PARA_MAX_NUM 2
#define AT_CUPBINDSTATUS_PARA_SERVERMODE 0 /* CUPBINDSTATUS的第一个参数SERVERMODE */
#define AT_CUPBINDSTATUS_PARA_BINDSTATUS 1 /* CUPBINDSTATUS的第二个参数BINDSTATUS */

#define AT_CAGPS_PARA_NUM 2

#define AT_CAGPSQRYTIME_PARA_NUM 2
#define AT_CAGPSQRYTIME_PARA_DATA 1 /* CAGPSQRYTIME的第二个参数DATA */
#endif

#if ((FEATURE_UE_MODE_CDMA == FEATURE_ON) && (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON) && \
     (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON))
#define ENCRYPT_VOICE_DATA_FILE_MAX_NUM 5
#endif

VOS_UINT32 AT_AsciiToHex(VOS_UINT8 *src, VOS_UINT8 *dst)
{
    if ((*src >= '0') && (*src <= '9')) { /* the number is 0-9 */
        *dst = (VOS_UINT8)(*src - '0');
    } else if ((*src >= 'a') && (*src <= 'f')) { /* the number is a-f */
        *dst = (VOS_UINT8)(*src - 'a') + 0x0a;
    } else if ((*src >= 'A') && (*src <= 'F')) { /* the number is A-F */
        *dst = (VOS_UINT8)(*src - 'A') + 0x0a;
    } else {
        return VOS_ERR;
    }

    return VOS_OK;
}

VOS_UINT32 AT_Hex2Ascii_Revers(VOS_UINT8 aucHex[], VOS_UINT32 length, VOS_UINT8 aucAscii[])
{
    VOS_INT32  loopSrc;
    VOS_UINT32 loopDest;
    VOS_UINT8  temp;

    loopSrc = (VOS_INT32)(length - 1);
    for (loopDest = 0; loopSrc >= 0; loopSrc--, loopDest++) {
        temp = (aucHex[loopSrc] >> 4) & 0x0F;
        if (temp < AT_DEC_MAX_NUM + 1) {
            /* 0~9 */
            aucAscii[loopDest] = temp + 0x30;
        }
        else {
            /* a~f */
            aucAscii[loopDest] = temp + 0x37;
        }

        loopDest++;
        temp = aucHex[loopSrc] & 0x0F;
        if (temp < AT_DEC_MAX_NUM + 1) {
            /* 0~9 */
            aucAscii[loopDest] = temp + 0x30;
        }
        else {
            /* a~f */
            aucAscii[loopDest] = temp + 0x37;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_AsciiToHexCode_Revers(VOS_UINT8 *src, VOS_UINT16 dataLen, VOS_UINT8 *dst)
{
    VOS_INT32  loop1;
    VOS_UINT32 loop2;
    VOS_UINT8  temp1;
    VOS_UINT8  temp2;
    VOS_UINT32 rslt;

    loop1 = (VOS_INT32)(dataLen - 1);
    for (loop2 = 0; loop1 >= 0; loop1--, loop2++) {
        rslt = AT_AsciiToHex(&(src[loop1]), &temp1);
        if (rslt == VOS_ERR) {
            return VOS_ERR;
        }

        loop1--;

        if (loop1 < 0) {
            AT_ERR_LOG("AT_AsciiToHexCode_Revers: sLoop1 is invalid.");

            return VOS_ERR;
        }

        rslt = AT_AsciiToHex(&(src[loop1]), &temp2);
        if (rslt == VOS_ERR) {
            return VOS_ERR;
        }

        dst[loop2] = (VOS_UINT8)((temp2 << 4) | temp1);
    }

    return VOS_OK;
}

VOS_VOID AT_ReadPlatFormPrint(ModemIdUint16 modemId, PLATAFORM_RatCapability *platRat)
{
    VOS_UINT32 i;

    AT_WARN_LOG1("Read PlatForm ModemId: ", modemId);
    AT_WARN_LOG1("Read PlatForm RatNum: ", platRat->ratNum);

    for (i = 0; (i < platRat->ratNum) && (i < PLATFORM_MAX_RAT_NUM); i++) {
        AT_WARN_LOG1("Read PlatForm RatType: ", platRat->ratList[i]);
    }
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_UINT32 AT_CheckCfshNumber(VOS_UINT8 *atPara, VOS_UINT16 len)
{
    VOS_UINT32 loop;

    /* 号码长度有效性判断:+号开头的国际号码，最大长度不能大于33；否则不能大于32 */
    if (atPara[0] == '+') {
        if (len > (TAF_CALL_MAX_FLASH_DIGIT_LEN + 1)) {
            return VOS_ERR;
        }

        atPara++;
        len--;
    } else {
        if (len > TAF_CALL_MAX_FLASH_DIGIT_LEN) {
            return VOS_ERR;
        }
    }

    /* 号码字符有效性判断(不包含国际号码的首字符'+') */
    for (loop = 0; loop < len; loop++) {
        if (((atPara[loop] >= '0') && (atPara[loop] <= '9')) || (atPara[loop] == '*') || (atPara[loop] == '#')) {
            continue;
        } else {
            return VOS_ERR;
        }
    }

    return VOS_OK;
}

VOS_UINT32 AT_SetCfshPara(VOS_UINT8 indexNum)
{
    errno_t            memResult;
    VOS_UINT32         rst;
    TAF_Ctrl           ctrl;
    TAF_CALL_FlashPara flashPara;
    ModemIdUint16      modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&flashPara, sizeof(flashPara), 0x00, sizeof(flashPara));

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 若携带了参数<number>，检查其有效性 */
    if (g_atParaIndex == 1) {
        if (AT_CheckCfshNumber(g_atParaList[0].para, g_atParaList[0].paraLen) != VOS_OK) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    } else {
        /* 这种AT命令AT^CFSH= 返回参数错误 */
        if (g_atParseCmd.cmdOptType == AT_CMD_OPT_SET_PARA_CMD) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    (VOS_VOID)memset_s(&flashPara, sizeof(flashPara), 0x00, sizeof(flashPara));

    flashPara.digitNum = (VOS_UINT8)g_atParaList[0].paraLen;
    if (g_atParaList[0].paraLen > 0) {
        memResult = memcpy_s(flashPara.digit, sizeof(flashPara.digit), g_atParaList[0].para, g_atParaList[0].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, sizeof(flashPara.digit), g_atParaList[0].paraLen);
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_FLASH_REQ消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, &flashPara, ID_TAF_CCM_SEND_FLASH_REQ, sizeof(flashPara), modemId);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFSH_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetCBurstDTMFPara(VOS_UINT8 indexNum)
{
    VOS_UINT32 rst;
    errno_t    memResult;
    VOS_UINT32 loop;
    VOS_UINT32 paraInvalidFlg;

    TAF_Ctrl               ctrl;
    TAF_CALL_BurstDtmfPara burstDTMFPara;
    ModemIdUint16          modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&burstDTMFPara, sizeof(burstDTMFPara), 0x00, sizeof(burstDTMFPara));

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    paraInvalidFlg = (g_atParaIndex != AT_CBURSTDTMF_PARA_NUM) ||
                     (g_atParaList[AT_CBURSTDTMF_CALL_ID].paraLen == 0) ||
                     (g_atParaList[AT_CBURSTDTMF_DTMF_KEY].paraLen == 0) ||
                     (g_atParaList[AT_CBURSTDTMF_ON_LENGTH].paraLen == 0) ||
                     (g_atParaList[AT_CBURSTDTMF_OFF_LENGTH].paraLen == 0);

    if (paraInvalidFlg == VOS_TRUE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key长度有效性判断 */
    if (g_atParaList[AT_CBURSTDTMF_DTMF_KEY].paraLen > TAF_CALL_MAX_BURST_DTMF_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* DTMF Key有效性判断 */
    for (loop = 0; loop < g_atParaList[AT_CBURSTDTMF_DTMF_KEY].paraLen; loop++) {
        if (((g_atParaList[AT_CBURSTDTMF_DTMF_KEY].para[loop] >= '0') &&
            (g_atParaList[AT_CBURSTDTMF_DTMF_KEY].para[loop] <= '9')) ||
            (g_atParaList[AT_CBURSTDTMF_DTMF_KEY].para[loop] == '*') ||
            (g_atParaList[AT_CBURSTDTMF_DTMF_KEY].para[loop] == '#')) {
            continue;
        } else {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    burstDTMFPara.callId   = (VOS_UINT8)g_atParaList[AT_CBURSTDTMF_CALL_ID].paraValue;
    burstDTMFPara.digitNum = (VOS_UINT8)g_atParaList[AT_CBURSTDTMF_DTMF_KEY].paraLen;

    memResult = memcpy_s(burstDTMFPara.digit, sizeof(burstDTMFPara.digit), g_atParaList[AT_CBURSTDTMF_DTMF_KEY].para,
                         g_atParaList[AT_CBURSTDTMF_DTMF_KEY].paraLen);
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(burstDTMFPara.digit), g_atParaList[AT_CBURSTDTMF_DTMF_KEY].paraLen);

    burstDTMFPara.onLength  = g_atParaList[AT_CBURSTDTMF_ON_LENGTH].paraValue;
    burstDTMFPara.offLength = g_atParaList[AT_CBURSTDTMF_OFF_LENGTH].paraValue;

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_BURST_DTMF_REQ消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, &burstDTMFPara, ID_TAF_CCM_SEND_BURST_DTMF_REQ, sizeof(burstDTMFPara), modemId);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CBURSTDTMF_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_CheckCContDtmfKeyPara(VOS_VOID)
{
    if (g_atParaList[AT_CCONTDTMF_DTMF_KEY].paraLen != AT_CCONTDTMF_DTMF_KEY_VALID_LEN) {
        return VOS_ERR;
    }

    if (((g_atParaList[AT_CCONTDTMF_DTMF_KEY].para[AT_CCONTDTMF_DTMF_KEY_SWITCH_STOP] >= '0') &&
         (g_atParaList[AT_CCONTDTMF_DTMF_KEY].para[AT_CCONTDTMF_DTMF_KEY_SWITCH_STOP] <= '9')) ||
         (g_atParaList[AT_CCONTDTMF_DTMF_KEY].para[AT_CCONTDTMF_DTMF_KEY_SWITCH_STOP] == '*') ||
         (g_atParaList[AT_CCONTDTMF_DTMF_KEY].para[AT_CCONTDTMF_DTMF_KEY_SWITCH_STOP] == '#')) {
        return VOS_OK;
    } else {
        return VOS_ERR;
    }
}

VOS_UINT32 AT_CheckCFreqLockEnablePara(VOS_VOID)
{
    VOS_UINT8 loop;

    for (loop = 1; loop < AT_CFREQLOCK_PARA_NUM_MAX; loop++) {
        if (g_atParaList[loop].paraLen == 0) {
            return VOS_FALSE;
        }
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_SetCContinuousDTMFPara(VOS_UINT8 indexNum)
{
    VOS_UINT32            rst;
    TAF_Ctrl              ctrl;
    TAF_CALL_ContDtmfPara contDTMFPara;
    ModemIdUint16         modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&contDTMFPara, sizeof(contDTMFPara), 0x00, sizeof(contDTMFPara));

    /* Check the validity of parameter */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Non set command!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*  Check the validity of <Call_ID> and <Switch> */
    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[1].paraLen == 0)) {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Call_ID> or <Switch>!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /*
     * If the <Switch> is Start and the number of parameter isn't equal to 3.
     * Or if the <Switch> is Stop and the number of parameter isn't equal to 2，both invalid
     */
    if (((g_atParaList[1].paraValue == TAF_CALL_CONT_DTMF_STOP) && (g_atParaIndex != AT_CCONTDTMF_PARA_NUM_MIN)) ||
        ((g_atParaList[1].paraValue == TAF_CALL_CONT_DTMF_START) && (g_atParaIndex != AT_CCONTDTMF_PARA_NUM_MAX))) {
        AT_WARN_LOG("AT_SetCContinuousDTMFPara: The number of parameters mismatch!");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* If the <Switch> is Start,the <Dtmf_Key> should be setted and check its validity */
    if (g_atParaList[1].paraValue == TAF_CALL_CONT_DTMF_START) {
        if (AT_CheckCContDtmfKeyPara() == VOS_ERR) {
            AT_WARN_LOG("AT_SetCContinuousDTMFPara: Invalid <Dtmf_Key>!");
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    contDTMFPara.callId     = (VOS_UINT8)g_atParaList[0].paraValue;
    contDTMFPara.dtmfSwitch = (VOS_UINT8)g_atParaList[1].paraValue;
    contDTMFPara.digit      = (VOS_UINT8)g_atParaList[AT_CCONTDTMF_DTMF_KEY].para[AT_CCONTDTMF_DTMF_KEY_SWITCH_STOP];

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_CONT_DTMF_REQ消息 */
    rst = TAF_CCM_CallCommonReq(&ctrl, &contDTMFPara, ID_TAF_CCM_SEND_CONT_DTMF_REQ, sizeof(contDTMFPara), modemId);

    if (rst == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCONTDTMF_SET;

        /* Return hang-up state */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

/* 移出CDMA编译开关 */
VOS_UINT32 AT_ConvertCSidListMcc(VOS_UINT8 *para, VOS_UINT16 len, VOS_UINT32 *mcc)
{
    VOS_UINT32 rslt = 0;

    /* 如果Mcc为空，返回失败 */
    if (len != AT_MCC_LENGTH) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt |= (VOS_UINT32)(*para - '0');
    rslt |= ((VOS_UINT32)(*(para + AT_CSIDLIST_PARA_OFFSET_1) - '0') << 8);
    rslt |= ((VOS_UINT32)(*(para + AT_CSIDLIST_PARA_OFFSET_2) - '0') << 16);

    *mcc = rslt;

    return AT_SUCCESS;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetCSidList(VOS_UINT8 indexNum)
{
    TAF_MMA_OperLockTrustSid trustSidList;
    VOS_UINT8                listNum;
    VOS_UINT32               rslt;
    VOS_UINT32               i;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 入参检查 */
    if (g_atParaList[AT_CSIDLIST_TRUST_LIST_NUM].paraLen != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为3N + 1,其中 1 表示 SID Trust名单数量 这个参数，3N为包含起始SID 终止SID 和国家码的三个一组N组数据的个数 */
    if ((g_atParaIndex - 1) % 3 != 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 AT命令最大参数个数为16个 即5组Trust名单 */
    if (g_atParaIndex > AT_CSIDLIST_MAX_PARA_NUMBER) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&trustSidList, sizeof(trustSidList), 0x00, sizeof(trustSidList));

    if (g_atParaList[AT_CSIDLIST_TRUST_LIST_NUM].paraValue != 0) {
        trustSidList.enable = VOS_TRUE;
    } else {
        trustSidList.enable = VOS_FALSE;
    }

    /* 计算Trust名单个数，1 表示 SID Trust名单数量 这个参数，一组Trust名单包含起始SID 终止SID 和国家码，所以除以3 */
    trustSidList.trustSysNum = ((VOS_UINT16)g_atParaIndex - 1) / 3;
    i                        = 0;
    /* 1表示第一组Trust名单中的起始SID所在位置索引 */
    /* 一组Trust名单包含起始SID终止SID和国家码3个数据， listNum += 3就跳转到下一组Trust名单进行判断 */
    for (listNum = 1; listNum < g_atParaIndex; listNum += 3) {
        /* startSid value must less then endSid value */
        /* listNum + 1表示每一组Trust名单中终止SID所在位置索引 */
        if (g_atParaList[listNum].paraValue > g_atParaList[listNum + 1].paraValue) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
        trustSidList.sysInfo[i].startSid = (VOS_UINT16)g_atParaList[listNum].paraValue;
        trustSidList.sysInfo[i].endSid   = (VOS_UINT16)g_atParaList[listNum + 1].paraValue;

        /* 转换 mcc */
        /* listNum + 2表示每一组Trust名单中国家码所在位置索引 */
        rslt = AT_ConvertCSidListMcc(g_atParaList[listNum + 2].para, g_atParaList[listNum + 2].paraLen,
                                     &trustSidList.sysInfo[i].mcc);

        i++;

        if (i >= TAF_MMA_MAX_TRUST_LOCK_SID_NUM) {
            break;
        }

        if (rslt != AT_SUCCESS) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 执行命令操作 */
    if (TAF_MMA_SetCSidList(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &trustSidList) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CSIDLIST_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCFreqLockPara(VOS_UINT8 indexNum)
{
    TAF_MMA_CfreqLockSetPara cFreqLockPara;
    VOS_UINT32               rst;

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果第一个参数是开启锁频,但参数个数不为9，或第一个参数是解除锁频，但参数个数不为1，都不合法 */
    if (((g_atParaList[0].paraValue == 1) && (g_atParaIndex != AT_CFREQLOCK_PARA_NUM_MAX)) ||
        ((g_atParaList[0].paraValue == 0) && (g_atParaIndex != AT_CFREQLOCK_PARA_NUM_MIN))) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果第一个参数是锁频，则1X/EVDO的锁频参数需同时设置，并需判断<cdma_pn>和<evdo_pn>的有效范围 */
    if (g_atParaList[0].paraValue == 1) {
        if (AT_CheckCFreqLockEnablePara() == VOS_FALSE) {
            return AT_CME_INCORRECT_PARAMETERS;
        }
    }

    /* 发送消息赋值 */
    (VOS_VOID)memset_s(&cFreqLockPara, sizeof(cFreqLockPara), 0x00, sizeof(TAF_MMA_CfreqLockSetPara));

    if ((VOS_UINT8)g_atParaList[0].paraValue == 1) {
        cFreqLockPara.freqLockMode  = TAF_MMA_CFREQ_LOCK_MODE_ON;
        cFreqLockPara.sid           = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_SID].paraValue;
        cFreqLockPara.nid           = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_NID].paraValue;
        cFreqLockPara.cdmaBandClass = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_CDMA_BAND_CLASS].paraValue;
        cFreqLockPara.cdmaFreq      = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_CDMA_FREQ].paraValue;
        cFreqLockPara.cdmaPn        = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_CDMA_PN].paraValue;
        cFreqLockPara.evdoBandClass = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_EVDO_BAND_CLASS].paraValue;
        cFreqLockPara.evdoFreq      = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_EVDO_FREQ].paraValue;
        cFreqLockPara.evdoPn        = (VOS_UINT16)g_atParaList[AT_CFREQLOCK_EVDO_PN].paraValue;
    } else {
        cFreqLockPara.freqLockMode = TAF_MMA_CFREQ_LOCK_MODE_OFF;
    }

    /* 发送TAF_MMA_CFREQ_LOCK_SET_REQ消息 */
    rst = TAF_MMA_ProcCFreqLockSetReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                      &cFreqLockPara);
    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CFREQLOCK_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_Set1xChanPara(VOS_UINT8 indexNum)
{
    TAF_MMA_CfreqLockSetPara cFreqLockPara;
    VOS_UINT32               rst;

    /* 参数有效性检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数合法性判断 */
    if ((g_atParaIndex != 1) || (g_atParaList[0].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 发送消息赋值 */
    (VOS_VOID)memset_s(&cFreqLockPara, sizeof(cFreqLockPara), 0x00, sizeof(TAF_MMA_CfreqLockSetPara));

    /* 复用锁频接口，除channel外，其他填0或者通配符 */
    cFreqLockPara.freqLockMode  = TAF_MMA_CFREQ_LOCK_MODE_ON;
    cFreqLockPara.sid           = 0x0000;
    cFreqLockPara.nid           = 0xFFFF;
    cFreqLockPara.cdmaBandClass = 0x0000;
    cFreqLockPara.cdmaFreq      = (VOS_UINT16)g_atParaList[0].paraValue;
    cFreqLockPara.cdmaPn        = 0xFFFF;
    cFreqLockPara.evdoBandClass = 0x0000;
    cFreqLockPara.evdoFreq      = 0x0000;
    cFreqLockPara.evdoPn        = 0xFFFF;

    /* 发送TAF_MMA_CFREQ_LOCK_SET_REQ消息 */
    rst = TAF_MMA_Proc1xChanSetReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                   &cFreqLockPara);
    if (rst == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_1XCHAN_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}
#endif

VOS_UINT32 AT_SetCdmaCsqPara(VOS_UINT8 indexNum)
{
    TAF_MMA_CdmacsqPara cdmaCsqPara;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex != AT_CDMACSQ_MIN_PARA_NUM) || (g_atParaList[AT_CDMACSQ_MODE].paraLen == 0) ||
        (g_atParaList[AT_CDMACSQ_TIME_INTERVAL].paraLen == 0) ||
        (g_atParaList[AT_CDMACSQ_RSSI_RPT_THRE].paraLen == 0) ||
        (g_atParaList[AT_CDMACSQ_ECIO_RPT_THRE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&cdmaCsqPara, sizeof(cdmaCsqPara), 0x00, sizeof(TAF_MMA_CdmacsqPara));
    cdmaCsqPara.mode         = (TAF_UINT8)g_atParaList[AT_CDMACSQ_MODE].paraValue;
    cdmaCsqPara.timeInterval = (TAF_UINT8)g_atParaList[AT_CDMACSQ_TIME_INTERVAL].paraValue;
    cdmaCsqPara.rssiRptThreshold = (TAF_UINT8)g_atParaList[AT_CDMACSQ_RSSI_RPT_THRE].paraValue;
    cdmaCsqPara.ecIoRptThreshold = (TAF_UINT8)g_atParaList[AT_CDMACSQ_ECIO_RPT_THRE].paraValue;

    if (TAF_MMA_ProcCdmaCsqSetReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &cdmaCsqPara) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CDMACSQ_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetHdrCsqPara(VOS_UINT8 indexNum)
{
    TAF_MMA_HdrCsqPara hdrCsqPara;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if ((g_atParaIndex != AT_HDRCSQ_PARA_NUM) || (g_atParaList[AT_HDRCSQ_MODE].paraLen == 0) ||
        (g_atParaList[AT_HDRCSQ_TIME_INTERVAL].paraLen == 0) ||
        (g_atParaList[AT_HDRCSQ_RSSI_RPT_THRE].paraLen == 0) ||
        (g_atParaList[AT_HDRCSQ_SNR_RPT_THRE].paraLen  == 0) ||
        (g_atParaList[AT_HDRCSQ_ECIO_RPT_THRE].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&hdrCsqPara, sizeof(hdrCsqPara), 0x00, sizeof(TAF_MMA_HdrCsqPara));
    hdrCsqPara.mode          = (TAF_UINT8)g_atParaList[AT_HDRCSQ_MODE].paraValue;
    hdrCsqPara.timeInterval  = (TAF_UINT8)g_atParaList[AT_HDRCSQ_TIME_INTERVAL].paraValue;
    hdrCsqPara.rssiThreshold = (TAF_UINT8)g_atParaList[AT_HDRCSQ_RSSI_RPT_THRE].paraValue;
    hdrCsqPara.snrThreshold  = (TAF_UINT8)g_atParaList[AT_HDRCSQ_SNR_RPT_THRE].paraValue;
    hdrCsqPara.ecioThreshold = (TAF_UINT8)g_atParaList[AT_HDRCSQ_ECIO_RPT_THRE].paraValue;

    if (TAF_MMA_ProcHdrCsqSetReq(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, &hdrCsqPara) == VOS_TRUE) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_HDR_CSQ_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetClocinfoEnable(VOS_UINT8 indexNum)
{
    AT_MTA_UnsolicitedRptSetReq atCmd;
    VOS_UINT32                  result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[0].paraValue > 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.reqType          = AT_MTA_SET_CLOCINFO_RPT_TYPE;
    atCmd.u.clocinfoRptFlg = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 给MTA发送^CLOCINFO设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetCSidEnable(VOS_UINT8 indexNum)
{

    AT_MTA_UnsolicitedRptSetReq atCmd;
    VOS_UINT32                  result;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[0].paraValue > 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));
    atCmd.reqType      = AT_MTA_SET_CSID_RPT_TYPE;
    atCmd.u.csidRptFlg = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 给MTA发送^CSID设置请求 */
    result = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_UNSOLICITED_RPT_SET_REQ,
                                    (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_UnsolicitedRptSetReq), I0_UEPS_PID_MTA);

    if (result != TAF_SUCCESS) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_UNSOLICITED_RPT_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT == FEATURE_ON)

VOS_UINT32 AT_SetEncryptCall(VOS_UINT8 indexNum)
{
    errno_t                  memResult;
    VOS_UINT8                callNum[AT_CMD_ECCALL_MAX_NUM_LENGTH + 1] = {0};
    VOS_UINT32               startIndex = 0;
    TAF_Ctrl                 ctrl = {0};
    TAF_CCM_EncryptVoicePara encrypVoicePara = {0};
    ModemIdUint16            modemId;
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)
    TAF_CHAR  acNewDocName[AT_TEST_ECC_FILE_NAME_MAX_LEN] = {0};
    VOS_INT32 length = 0;
    int       fd;

#endif

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为2 */
    if (g_atParaIndex != 2) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 入参长度检查 */
    if ((g_atParaList[0].paraLen != 1) || (g_atParaList[1].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 如果呼叫号码大于11位，则仅取最后的11位 */
    if (g_atParaList[1].paraLen > AT_CMD_ECCALL_MAX_NUM_LENGTH) {
        startIndex = (VOS_UINT32)g_atParaList[1].paraLen - AT_CMD_ECCALL_MAX_NUM_LENGTH;
    }
    memResult = memcpy_s(callNum, (VOS_UINT32)sizeof(callNum), &g_atParaList[1].para[startIndex],
                         AT_CMD_ECCALL_MAX_NUM_LENGTH);
    TAF_MEM_CHK_RTN_VAL(memResult, (VOS_UINT32)sizeof(callNum), AT_CMD_ECCALL_MAX_NUM_LENGTH);

    /* 检查并转换电话号码 */
    if (AT_AsciiNumberToBcd((VOS_CHAR *)callNum, encrypVoicePara.dialNumber.bcdNum,
                            &encrypVoicePara.dialNumber.numLen) != MN_ERR_NO_ERROR) {
        return AT_ERROR;
    }

    /* 执行命令操作 */

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    encrypVoicePara.eccVoiceType = g_atParaList[0].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SEND_FLASH_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &encrypVoicePara, ID_TAF_CCM_ENCRYPT_VOICE_REQ, sizeof(encrypVoicePara),
                              modemId) == VOS_OK) {
#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)

        /* 密话测试模式写文件path下标 */
        g_currEncVoiceDataWriteFileNum++;

        /* 最大写密话数据文件数为5,超过的话循环写文件1，同时需要删掉文件中的内容 */
        if (g_currEncVoiceDataWriteFileNum > ENCRYPT_VOICE_DATA_FILE_MAX_NUM) {
            g_currEncVoiceDataWriteFileNum = 1;
        }
        g_atCurrEncVoiceDataCount = 0;

        g_atCurrEncVoiceTestFileNum++;

        (VOS_VOID)memset_s(g_atCurrDocName, sizeof(g_atCurrDocName), 0x00, sizeof(g_atCurrDocName));

        length = snprintf_s(acNewDocName, (VOS_SIZE_T)sizeof(acNewDocName), (VOS_SIZE_T)(sizeof(acNewDocName) - 1),
                            MODEM_LOG_ROOT "/ECC_TEST/Encrypted_call_execute_time%d.txt", g_atCurrEncVoiceTestFileNum);
        if (length <= 0) {
            AT_WARN_LOG("AT_SetEncryptCall(): snprintf_s len <= 0");

            return AT_ERROR;
        }

        /* 清除待写入文件中的内容 */
        fd = at_file_open(g_currEncVoiceDataWriteFilePath[g_currEncVoiceDataWriteFileNum], "w");
        if (fd < 0) {
            AT_WARN_LOG("AT_SetEncryptCall: Clear file data failed!!!\n");
        } else {
            at_file_close(fd);
        }

        fd = at_file_open(acNewDocName, "w");
        if (fd < 0) {
            AT_WARN_LOG("AT_SetEncryptCall: Truncate file failed!!!\n");
        } else {
            at_file_close(fd);
        }
#endif
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCALL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }
    return AT_ERROR;
}

VOS_UINT32 AT_SetEncryptCallRemoteCtrl(VOS_UINT8 indexNum)
{
    TAF_Ctrl                     ctrl;
    TAF_CCM_RemoteCtrlAnswerInfo remoteCtrlAnswerInfo;
    ModemIdUint16                modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&remoteCtrlAnswerInfo, sizeof(remoteCtrlAnswerInfo), 0x00, sizeof(remoteCtrlAnswerInfo));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为2 */
    if (g_atParaIndex != 2) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 入参长度检查 */
    if ((g_atParaList[0].paraLen != 1) || (g_atParaList[1].paraLen != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    remoteCtrlAnswerInfo.remoteCtrlEvtType = g_atParaList[0].paraValue;
    remoteCtrlAnswerInfo.result            = g_atParaList[1].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_REMOTE_CTRL_ANSWER_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &remoteCtrlAnswerInfo, ID_TAF_CCM_REMOTE_CTRL_ANSWER_REQ,
                              sizeof(remoteCtrlAnswerInfo), modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCTRL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }
    return AT_ERROR;
}

VOS_UINT32 AT_SetEncryptCallCap(VOS_UINT8 indexNum)
{
    TAF_Ctrl                 ctrl;
    TAF_CCM_EccSrvCapCfgPara encryptCallCapPara;
    ModemIdUint16            modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&encryptCallCapPara, sizeof(encryptCallCapPara), 0x00, sizeof(encryptCallCapPara));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为2 */
    if (g_atParaIndex != 2) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 入参长度检查 */
    if ((g_atParaList[0].paraLen != 1) || (g_atParaList[1].paraLen != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 用户不允许enalbe密话业务能力，只允许disable密话能力 */
    if (g_atParaList[0].paraValue == TAF_CALL_ECC_SRV_CAP_ENABLE) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    /* 执行命令操作 */
    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    encryptCallCapPara.eccSrvCap    = g_atParaList[0].paraValue;
    encryptCallCapPara.eccSrvStatus = g_atParaList[1].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 给CCM发送ID_TAF_CCM_REMOTE_CTRL_ANSWER_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &encryptCallCapPara, ID_TAF_CCM_ECC_SRV_CAP_CFG_REQ, sizeof(encryptCallCapPara),
                              modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCAP_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

#if (FEATURE_CHINA_TELECOM_VOICE_ENCRYPT_TEST_MODE == FEATURE_ON)

VOS_UINT32 AT_SetEncryptCallKmc(VOS_UINT8 indexNum)
{
    TAF_Ctrl             ctrl;
    MN_CALL_AppEcKmcData kmcData;
    ModemIdUint16        modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&kmcData, sizeof(kmcData), 0x00, sizeof(kmcData));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为2 */
    if (g_atParaIndex != AT_ECKMC_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 入参长度检查 */
    if ((g_atParaList[AT_ECKMC_KEY].paraLen > AT_CMD_ECKMC_MAX_KMC_LENGTH) ||
        (g_atParaList[AT_ECKMC_KEY].paraLen == 0) ||
        (g_atParaList[AT_ECKMC_VER].paraLen == 0)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    kmcData.eccKmcVer = (VOS_UINT8)g_atParaList[AT_ECKMC_VER].paraValue;

    /* 要取KMC公钥长度的一半，如果长度为偶数，直接除2，否则除2后加1 */
    if (g_atParaList[AT_ECKMC_KEY].paraLen % 2 == 0) {
        kmcData.eccKmcLength = g_atParaList[AT_ECKMC_KEY].paraLen / 2;
    } else {
        kmcData.eccKmcLength = g_atParaList[AT_ECKMC_KEY].paraLen / 2 + 1;
    }

    /*lint -e516 */
    /*  把kmc data转换为hex保存 */
    if (At_AsciiString2HexSimple(kmcData.eccKmcData, g_atParaList[AT_ECKMC_KEY].para,
                                 g_atParaList[AT_ECKMC_KEY].paraLen) == AT_FAILURE) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SET_EC_KMC_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &kmcData, ID_TAF_CCM_SET_EC_KMC_REQ, sizeof(kmcData), modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECKMC_SET;

        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetEccTestMode(VOS_UINT8 indexNum)
{
    TAF_Ctrl                     ctrl;
    TAF_CALL_SetEcTestModeUint32 eccTestModeStatus;
    ModemIdUint16                modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数为1 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 入参长度检查 */
    if (g_atParaList[0].paraLen != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    g_atCurrEncVoiceTestFileNum = 0;

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    eccTestModeStatus = g_atParaList[0].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_SET_EC_TEST_MODE_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &eccTestModeStatus, ID_TAF_CCM_SET_EC_TEST_MODE_REQ, sizeof(eccTestModeStatus),
                              modemId) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_ECCTEST_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    }

    return AT_ERROR;
}

#endif
#endif

VOS_UINT32 AT_SetCtRoamInfo(VOS_UINT8 indexNum)
{
    /* 检查命令类型是否匹配 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查参数个数是否匹配 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (TAF_MMA_SetCtRoamInfo(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0,
                              (VOS_UINT8)g_atParaList[0].paraValue) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CTROAMINFO_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetCtOosCount(VOS_UINT8 indexNum)
{
    /* 检查命令类型是否匹配 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查参数个数是否匹配 */
    if (g_atParaIndex != AT_CTOOSCOUNT_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (TAF_MMA_SetCtOosCount(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0,
                              g_atParaList[AT_CTOOSCOUNT_CL_COUNT].paraValue,
                              g_atParaList[AT_CTOOSCOUNT_GUL_COUNT].paraValue) == VOS_TRUE) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CTOOSCOUNT_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetPrivacyModePreferred(VOS_UINT8 indexNum)
{
    TAF_Ctrl                    ctrl;
    TAF_CALL_PrivacyModeSetPara privacyMode;
    ModemIdUint16               modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&privacyMode, sizeof(privacyMode), 0x00, sizeof(privacyMode));

    /* 检查命令类型是否匹配 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查参数个数是否匹配 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查入参长度检查 */
    if (g_atParaList[0].paraLen != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 检查参数值范围 */
    if ((g_atParaList[0].paraValue != 0) && (g_atParaList[0].paraValue != 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    privacyMode.privacyMode = (TAF_CALL_PrivacyModeUint8)g_atParaList[0].paraValue;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送ID_TAF_CCM_PRIVACY_MODE_SET_REQ消息 */
    if (TAF_CCM_CallCommonReq(&ctrl, &privacyMode, ID_TAF_CCM_PRIVACY_MODE_SET_REQ, sizeof(privacyMode), modemId) ==
        VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CPMP_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetNoCardMode(VOS_UINT8 indexNum)
{
    AT_MTA_NoCardModeSetReq noCardModeCfg = {0};
    VOS_UINT32              rslt;

    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if ((g_atParaList[0].paraLen == 0) || (g_atParaList[0].paraValue > 1)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    (VOS_VOID)memset_s(&noCardModeCfg, sizeof(noCardModeCfg), 0x00, sizeof(noCardModeCfg));

    noCardModeCfg.enableFlag = g_atParaList[0].paraValue;

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_NO_CARD_MODE_SET_REQ,
                                  (VOS_UINT8 *)&noCardModeCfg, sizeof(noCardModeCfg), I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_NOCARDMODE_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("AT_SetNoCardMode: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCclprPara(VOS_UINT8 indexNum)
{
    VOS_UINT32           result;
    TAF_Ctrl             ctrl;
    TAF_CALL_QryClprPara qryClprPara;
    ModemIdUint16        modemId;

    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(ctrl));
    (VOS_VOID)memset_s(&qryClprPara, sizeof(qryClprPara), 0x00, sizeof(qryClprPara));

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = g_atClientTab[indexNum].opId;

    qryClprPara.callId          = (MN_CALL_ID_T)g_atParaList[0].paraValue;
    qryClprPara.qryClprModeType = TAF_CALL_QRY_CLPR_MODE_C;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    /* 发送跨核消息ID_TAF_CCM_QRY_CLPR_REQ到C核 */
    result = TAF_CCM_CallCommonReq(&ctrl, (void *)&qryClprPara, ID_TAF_CCM_QRY_CLPR_REQ, sizeof(qryClprPara), modemId);

    if (result != VOS_OK) {
        AT_WARN_LOG("AT_SetCclprPara: TAF_XCALL_SendCclpr fail.");
        return AT_ERROR;
    }

    /* 设置AT模块实体的状态为等待异步返回 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCLPR_SET;

    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetCcmgdPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 smsIndex;

    /* 参数不合法，返回错误 */
    if (g_atParaIndex > 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    smsIndex = (VOS_UINT8)g_atParaList[0].paraValue;


    /* 执行命令操作 */
    if (TAF_XSMS_DeleteSmsReq(g_atClientTab[indexNum].clientId, 0, smsIndex) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCMGD_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetCdmaMemStatusPara(VOS_UINT8 indexNum)
{
    TAF_XSMS_ApMemFullUint8 aPMemFullFlag = 0;

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
        aPMemFullFlag = TAF_XSMS_AP_MEM_FULL;
    } else {
        aPMemFullFlag = TAF_XSMS_AP_MEM_NOT_FULL;
    }

    /* 调用MN消息API发送设置请求给MSG模块 */
    if (TAF_XSMS_SetXsmsApMemFullReq(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId, aPMemFullFlag) !=
        AT_SUCCESS) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCSASM_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

/*
 * 功能描述: ^CCIMI
 * 修改历史:
 *  1.日    期: 2015年6月17日
 *    修改内容: 新生成函数
 */
TAF_UINT32 At_SetCCimiPara(TAF_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 执行命令操作 */
    if (SI_PIH_CCimiSetReq(g_atClientTab[indexNum].clientId, 0) == AT_SUCCESS) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CCIMI_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCrmPara(VOS_UINT8 indexNum)
{
    /* 参数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }
#if (FEATURE_PPP == FEATURE_ON)

    if (g_atParaList[0].paraValue == TAF_PS_CDATA_DIAL_MODE_RELAY) {
        PPP_SetRawDataByPassMode(VOS_TRUE);
    } else {
        PPP_SetRawDataByPassMode(VOS_FALSE);
    }
#endif

    if (TAF_PS_SetCdataDialModeInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId),
                                    g_atParaList[0].paraValue) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CRM_SET;
        return AT_WAIT_ASYNC_RETURN;
    }

    return AT_ERROR;
}

VOS_UINT32 AT_SetCtaPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 timeLen;

    /* 参数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    timeLen = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 执行命令操作 */
    if (TAF_PS_SetCtaInfo(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0, timeLen) !=
        VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CTA_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetCdmaDormantTimer(VOS_UINT8 indexNum)
{
    VOS_UINT8 dormantTimer;

    /* 参数检查 */
    if (g_atParaIndex != 1) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    dormantTimer = (VOS_UINT8)g_atParaList[0].paraValue;

    /* 执行命令操作 */
    if (TAF_PS_SetCdmaDormantTimer(WUEPS_PID_AT, AT_PS_BuildExClientId(g_atClientTab[indexNum].clientId), 0,
                                   dormantTimer) != VOS_OK) {
        return AT_ERROR;
    }

    /* 设置当前操作类型 */
    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_DORMTIMER_SET;

    /* 返回命令处理挂起状态 */
    return AT_WAIT_ASYNC_RETURN;
}

VOS_UINT32 AT_SetMeidPara(VOS_UINT8 indexNum)
{
    AT_MTA_MeidSetReq meIdReq;
    VOS_UINT32        rslt;

    memset_s(&meIdReq, sizeof(meIdReq), 0x00, sizeof(AT_MTA_MeidSetReq));

    /* 命令状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不为1或者字符串长度不为14 */
    if ((g_atParaIndex != 1) || (g_atParaList[AT_MEID_PARA_MEIDNUM].paraLen != 14)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_AsciiToHexCode_Revers(g_atParaList[AT_MEID_PARA_MEIDNUM].para,
                                    g_atParaList[AT_MEID_PARA_MEIDNUM].paraLen, meIdReq.meid);

    if (rslt != VOS_OK) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_MEID_SET_REQ, (VOS_UINT8 *)&meIdReq, sizeof(meIdReq), I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_MEID_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("AT_SetMeidPara: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetEvdoSysEvent(VOS_UINT8 indexNum)
{
    AT_MTA_EvdoSysEventSetReq sysEvent;
    VOS_UINT32                rslt;

    (VOS_VOID)memset_s(&sysEvent, sizeof(sysEvent), 0x00, sizeof(sysEvent));

    /* 命令状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不为1或者字符串长度大于10, 4294967295 = 0xffffffff */
    if ((g_atParaIndex != AT_DOSYSEVENT_PARA_NUM) || (g_atParaList[AT_DOSYSEVENT_DO_SYS_EVENT].paraLen >
        AT_DOSYSEVENT_DO_SYS_EVENT_VALID)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    sysEvent.doSysEvent = g_atParaList[AT_DOSYSEVENT_DO_SYS_EVENT].paraValue;

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_EVDO_SYS_EVENT_SET_REQ, (VOS_UINT8 *)&sysEvent, sizeof(sysEvent),
                                  I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EVDO_SYS_EVENT_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("AT_SetEvdoSysEvent: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetDoSigMask(VOS_UINT8 indexNum)
{
    AT_MTA_EvdoSigMaskSetReq sigMask;
    VOS_UINT32               rslt;

    (VOS_VOID)memset_s(&sigMask, sizeof(sigMask), 0x00, sizeof(sigMask));

    /* 命令状态检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不为1或者字符串长度大于10, 4294967295 = 0xffffffff */
    if ((g_atParaIndex != AT_DOSIGMASK_PARA_NUM) || (g_atParaList[AT_DOSIGMASK_DO_SIG_MASK].paraLen >
        AT_DOSIGMASK_DO_SIG_MASK_VALID)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    sigMask.doSigMask = g_atParaList[AT_DOSIGMASK_DO_SIG_MASK].paraValue;

    rslt = AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, g_atClientTab[indexNum].opId,
                                  ID_AT_MTA_EVDO_SIG_MASK_SET_REQ, (VOS_UINT8 *)&sigMask, sizeof(sigMask),
                                  I0_UEPS_PID_MTA);

    if (rslt == TAF_SUCCESS) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_EVDO_SIG_MASK_SET;

        /* 返回命令处理挂起状态 */
        return AT_WAIT_ASYNC_RETURN;
    } else {
        AT_WARN_LOG("AT_SetDoSigMask: AT_FillAndSndAppReqMsg fail.");
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCustomDial(VOS_UINT8 indexNum)
{
    TAF_CALL_CustomDialPara customDialPara;
    TAF_Ctrl                ctrl;
    ModemIdUint16           modemId;

    (VOS_VOID)memset_s(&customDialPara, sizeof(customDialPara), 0x00, sizeof(TAF_CALL_CustomDialPara));
    (VOS_VOID)memset_s(&ctrl, sizeof(ctrl), 0x00, sizeof(TAF_Ctrl));

    /* 将输入第一个参数转成呼叫号码 */
    if (AT_FillCalledNumPara(g_atParaList[0].para, g_atParaList[0].paraLen, &customDialPara.dialNumber) != VOS_OK) {
        AT_UpdateCallErrInfo(indexNum, TAF_CS_CAUSE_INVALID_PARAMETER, VOS_NULL_PTR);
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将输入第二个参数转成SO */
    customDialPara.so       = g_atParaList[1].paraValue;
    customDialPara.callType = MN_CALL_TYPE_VOICE;

    ctrl.moduleId = WUEPS_PID_AT;
    ctrl.clientId = g_atClientTab[indexNum].clientId;
    ctrl.opId     = 0;

    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        return AT_ERROR;
    }

    if (TAF_CCM_CallCommonReq(&ctrl, &customDialPara, ID_TAF_CCM_CUSTOM_DIAL_REQ, sizeof(customDialPara), modemId) ==
        VOS_OK) {
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CUSTOMDIAL_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        return AT_ERROR;
    }
}

/*
 * 功能描述: 从NV中获取平台能力
 */
LOCAL VOS_UINT32 AT_GetPlataFormRatCapability(PLATAFORM_RatCapability *modem0PlatRat,
    PLATAFORM_RatCapability *modem1PlatRat, PLATAFORM_RatCapability *modem2PlatRat)
{
    if (TAF_ACORE_NV_READ(MODEM_ID_0, NV_ITEM_PLATFORM_RAT_CAP, modem0PlatRat, sizeof(PLATAFORM_RatCapability)) !=
        NV_OK) {
        AT_ERR_LOG("AT_GetPlataFormRatCapability: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem0!");
        return AT_ERROR;
    }

    AT_NORM_LOG("Read PlatForm from Work success: ");
    AT_ReadPlatFormPrint(MODEM_ID_0, modem0PlatRat);
    /* 先打印便于分析，后边界保护 */
    modem0PlatRat->ratNum = AT_MIN(modem0PlatRat->ratNum, PLATFORM_MAX_RAT_NUM);

    if (TAF_ACORE_NV_READ(MODEM_ID_1, NV_ITEM_PLATFORM_RAT_CAP, modem1PlatRat, sizeof(PLATAFORM_RatCapability)) !=
        NV_OK) {
        AT_ERR_LOG("AT_GetPlataFormRatCapability: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem1!");
        return AT_ERROR;
    }

    AT_NORM_LOG("Read PlatForm from Work success: ");
    AT_ReadPlatFormPrint(MODEM_ID_1, modem1PlatRat);
    /* 先打印便于分析，后边界保护 */
    modem1PlatRat->ratNum = AT_MIN(modem1PlatRat->ratNum, PLATFORM_MAX_RAT_NUM);

#if (MULTI_MODEM_NUMBER == 3)
    if (TAF_ACORE_NV_READ(MODEM_ID_2, NV_ITEM_PLATFORM_RAT_CAP, modem2PlatRat, sizeof(PLATAFORM_RatCapability)) !=
        NV_OK) {
        AT_ERR_LOG("AT_GetPlataFormRatCapability: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem2!");
        return AT_ERROR;
    }

    AT_NORM_LOG("Read PlatForm from Work success: ");
    AT_ReadPlatFormPrint(MODEM_ID_2, modem2PlatRat);
    /* 先打印便于分析，后边界保护 */
    modem2PlatRat->ratNum = AT_MIN(modem2PlatRat->ratNum, PLATFORM_MAX_RAT_NUM);
#endif

    return AT_SUCCESS;
}

VOS_UINT32 AT_CheckSwitchCdmaModePara(PLATAFORM_RatCapability *modem0PlatRat, PLATAFORM_RatCapability *modem1PlatRat,
                                      PLATAFORM_RatCapability *modem2PlatRat, VOS_UINT32 modemId)
{
    VOS_UINT32 modem0Support1XFlg;
    VOS_UINT32 modem0SupportHrpdFlg;
    VOS_UINT32 modem1Support1XFlg;
    VOS_UINT32 modem1SupportHrpdFlg;
    VOS_UINT32 modem2Support1XFlg;
    VOS_UINT32 modem2SupportHrpdFlg;
    VOS_UINT32 modem0SupportCMode;
    VOS_UINT32 modem1SupportCMode;
    VOS_UINT32 modem2SupportCMode;

    modem0Support1XFlg   = AT_IsPlatformSupport1XMode(modem0PlatRat);
    modem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(modem0PlatRat);
    modem1Support1XFlg   = AT_IsPlatformSupport1XMode(modem1PlatRat);
    modem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(modem1PlatRat);
    modem2Support1XFlg   = AT_IsPlatformSupport1XMode(modem2PlatRat);
    modem2SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(modem2PlatRat);

    modem0SupportCMode = (modem0Support1XFlg || modem0SupportHrpdFlg);
    modem1SupportCMode = (modem1Support1XFlg || modem1SupportHrpdFlg);
    modem2SupportCMode = (modem2Support1XFlg || modem2SupportHrpdFlg);

    /* 如果modem0和modem1同时满足支持，则认为设置失败 */
    if ((modem0SupportCMode == VOS_TRUE) && (modem1SupportCMode == VOS_TRUE)) {
        AT_ERR_LOG("AT_CheckSwitchCdmaModePara: Modem0 and Modem1 Support CMode");
        return AT_ERROR;
    }

    /* 如果modem0和modem2同时满足支持，则认为设置失败 */
    if ((modem0SupportCMode == VOS_TRUE) && (modem2SupportCMode == VOS_TRUE)) {
        AT_ERR_LOG("AT_CheckSwitchCdmaModePara: Modem0 and Modem2 Support CMode");

        return AT_ERROR;
    }

    /* 如果modem1和modem2同时满足支持，则认为设置失败 */
    if ((modem1SupportCMode == VOS_TRUE) && (modem2SupportCMode == VOS_TRUE)) {
        AT_ERR_LOG("AT_CheckSwitchCdmaModePara: Modem2 and Modem1 Support CMode");
        return AT_ERROR;
    }

    /* 如果需要将CDMA modem切换的目标modem0，而modem0已经支持，则直接返回OK */
    if ((modemId == MODEM_ID_0) && (modem0SupportCMode)) {
        return AT_OK;
    }

    /* 如果需要将CDMA modem切换的目标modem1，而modem1已经支持，则直接返回OK */
    if ((modemId == MODEM_ID_1) && (modem1SupportCMode)) {
        return AT_OK;
    }

    /* 如果需要将CDMA modem切换的目标modem2，而modem2已经支持，则直接返回OK */
    if ((modemId == MODEM_ID_2) && (modem2SupportCMode)) {
        return AT_OK;
    }

    return AT_SUCCESS;
}

VOS_UINT32 AT_SetCdmaModemCapPara(PLATAFORM_RatCapability *sourceModemPlatform,
                                  PLATAFORM_RatCapability *destinationModemPlatform, ModemIdUint16 sourceModemId,
                                  ModemIdUint16 destinationModemId, VOS_UINT8 indexNum)
{
    AT_MTA_CdmaModemCapSetReq atCmd;
    errno_t                   memResult;

    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));

    AT_WARN_LOG2("AT_SetCdmaModemCapPara: Entry enSourceModemId and enDestinationModemId !", destinationModemId,
                 destinationModemId);

    atCmd.sourceModemId      = sourceModemId;
    atCmd.destinationModemId = destinationModemId;
    memResult = memcpy_s(&(atCmd.sourceModemPlatform), sizeof(atCmd.sourceModemPlatform), sourceModemPlatform,
                         sizeof(PLATAFORM_RatCapability));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmd.sourceModemPlatform), sizeof(PLATAFORM_RatCapability));

    memResult = memcpy_s(&(atCmd.destinationModemPlatform), sizeof(atCmd.destinationModemPlatform),
                         destinationModemPlatform, sizeof(PLATAFORM_RatCapability));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmd.destinationModemPlatform), sizeof(PLATAFORM_RatCapability));

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_CDMA_MODEM_CAP_SET_REQ,
                               (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_CdmaModemCapSetReq),
                               I0_UEPS_PID_MTA) == TAF_SUCCESS) {
        AT_ERR_LOG("AT_SetCdmaModemCapPara,AT_FillAndSndAppReqMsg return SUCCESS");
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CDMAMODEMSWITCH_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_ERR_LOG("AT_SetCdmaModemCapPara,AT_FillAndSndAppReqMsg return ERROR");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_IsPlatformValidRemovingXCap(PLATAFORM_RatCapability *modemPlatform)
{
    VOS_UINT32 modemSupport1XFlg;
    VOS_UINT32 modemSupportHrpdFlg;

    modemSupport1XFlg   = AT_IsPlatformSupport1XMode(modemPlatform);
    modemSupportHrpdFlg = AT_IsPlatformSupportHrpdMode(modemPlatform);

    /* 判断满足切换的条件:modem切换后，如果它的平台能力个数为0的话，则不切换 */
    if (((modemSupport1XFlg && modemSupportHrpdFlg)) && (modemPlatform->ratNum == 2)) { /* 接入技术的个数为2 */
        return VOS_FALSE;
    }

    if (((modemSupport1XFlg || modemSupportHrpdFlg)) && (modemPlatform->ratNum == 1)) { /* 接入技术的个数为1 */
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_RemoveSpecRatFromPlatform(PLATFORM_RatTypeUint16 ratType, PLATAFORM_RatCapability *platform)
{
    VOS_UINT16 i;
    VOS_UINT16 ratNum = 0;

    for (i = 0; i < AT_MIN(platform->ratNum, PLATFORM_MAX_RAT_NUM); i++) {
        if (ratType != platform->ratList[i]) {
            platform->ratList[ratNum] = platform->ratList[i];
            ratNum++;
        }
    }

    platform->ratNum = ratNum;

    return VOS_TRUE;
}

VOS_UINT32 AT_AddSpecRatIntoPlatform(PLATFORM_RatTypeUint16 ratType, PLATAFORM_RatCapability *platform)
{
    if (platform->ratNum >= PLATFORM_MAX_RAT_NUM) {
        return VOS_FALSE;
    }

    platform->ratList[platform->ratNum] = ratType;
    platform->ratNum++;

    return VOS_TRUE;
}

VOS_UINT32 AT_SwitchCdmaMode(VOS_UINT8 indexNum, VOS_UINT32 modemId, PLATAFORM_RatCapability *modem0Platform,
                             PLATAFORM_RatCapability *modem1Platform, PLATAFORM_RatCapability *modem2Platform)
{
    ModemIdUint16            sourceModemId;
    ModemIdUint16            destinationModemId;
    PLATAFORM_RatCapability *sourceModemPlatform      = VOS_NULL_PTR;
    PLATAFORM_RatCapability *destinationModemPlatform = VOS_NULL_PTR;
    VOS_UINT32               modem0Support1XFlg = AT_IsPlatformSupport1XMode(modem0Platform);
    VOS_UINT32               modem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(modem0Platform);
    VOS_UINT32               modem1Support1XFlg = AT_IsPlatformSupport1XMode(modem1Platform);
    VOS_UINT32               modem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(modem1Platform);
    VOS_UINT32               modem0SupportCMode;
    VOS_UINT32               modem1SupportCMode;
    VOS_UINT32               sourceSupport1XFlg;
    VOS_UINT32               sourceSupportHrpdFlg;

    modem0SupportCMode = (modem0Support1XFlg || modem0SupportHrpdFlg);
    modem1SupportCMode = (modem1Support1XFlg || modem1SupportHrpdFlg);

    if (modemId == 0) {
        destinationModemId       = MODEM_ID_0;
        destinationModemPlatform = modem0Platform;
    } else if (modemId == 1) {
        destinationModemId       = MODEM_ID_1;
        destinationModemPlatform = modem1Platform;
    } else {
        destinationModemId       = MODEM_ID_2;
        destinationModemPlatform = modem2Platform;
    }

    if (modem0SupportCMode == VOS_TRUE) {
        sourceModemId       = MODEM_ID_0;
        sourceModemPlatform = modem0Platform;
    } else if (modem1SupportCMode == VOS_TRUE) {
        sourceModemId       = MODEM_ID_1;
        sourceModemPlatform = modem1Platform;
    } else {
        sourceModemId       = MODEM_ID_2;
        sourceModemPlatform = modem2Platform;
    }

    sourceSupport1XFlg   = AT_IsPlatformSupport1XMode(sourceModemPlatform);
    sourceSupportHrpdFlg = AT_IsPlatformSupportHrpdMode(sourceModemPlatform);

    if (AT_IsPlatformValidRemovingXCap(sourceModemPlatform) == VOS_FALSE) {
        AT_ERR_LOG("AT_SwitchCdmaMode: AT_IsPlatformValidRemovingXCap return  FALSE !");
        return VOS_FALSE;
    }

    if (sourceSupport1XFlg == VOS_TRUE) {
        (VOS_VOID)AT_RemoveSpecRatFromPlatform(PLATFORM_RAT_1X, sourceModemPlatform);
    }

    if (sourceSupportHrpdFlg == VOS_TRUE) {
        (VOS_VOID)AT_RemoveSpecRatFromPlatform(PLATFORM_RAT_HRPD, sourceModemPlatform);
    }

    (VOS_VOID)AT_AddSpecRatIntoPlatform(PLATFORM_RAT_1X, destinationModemPlatform);
    (VOS_VOID)AT_AddSpecRatIntoPlatform(PLATFORM_RAT_HRPD, destinationModemPlatform);

    if (AT_SetCdmaModemCapPara(sourceModemPlatform, destinationModemPlatform, sourceModemId, destinationModemId,
                               indexNum) == AT_ERROR) {
        AT_ERR_LOG("AT_SwitchCdmaMode: AT_SetCdmaModemCapPara return Error!");

        return VOS_FALSE;
    } else {
        AT_ERR_LOG("AT_SwitchCdmaMode: AT_SetCdmaModemCapPara return Ok!");
    }

    return VOS_TRUE;
}

VOS_UINT32 At_SetCdmaModemSwitch(VOS_UINT8 indexNum)
{
#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
#if (MULTI_MODEM_NUMBER >= 2)
    PLATAFORM_RatCapability  modem0PlatRat = {0};
    PLATAFORM_RatCapability  modem1PlatRat = {0};
    PLATAFORM_RatCapability  modem2PlatRat = {0};
    VOS_UINT32               modemId;
    ModemIdUint16            modemIdTemp = MODEM_ID_BUTT;
    VOS_UINT8 isCdmaModemSwitchNotResetFlg;
    VOS_UINT32 checkParaRlt;
    VOS_UINT32 result;

    modemId = g_atParaList[0].paraValue;

    /* 改命令仅支持在Modem0上发起 */
    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemIdTemp) != VOS_OK) {
        AT_ERR_LOG("At_SetCdmaModemSwitch: Get modem id fail!");

        return AT_ERROR;
    }

    isCdmaModemSwitchNotResetFlg = AT_GetModemCdmaModemSwitchCtxAddrFromModemId(modemIdTemp)->enableFlg;

    if ((modemIdTemp != MODEM_ID_0) && (isCdmaModemSwitchNotResetFlg == VOS_TRUE)) {
        AT_ERR_LOG("At_SetCdmaModemSwitch: This Command Only Support On Modem0!");

        return AT_ERROR;
    }

    /* 从NV中获取平台能力 */
    if (AT_GetPlataFormRatCapability(&modem0PlatRat, &modem1PlatRat, &modem2PlatRat) != AT_SUCCESS) {
        AT_ERR_LOG("At_SetCdmaModemSwitch: GetPlataFormRatCapability error!");
        return AT_ERROR;
    }

#if (MULTI_MODEM_NUMBER == 2)
    /* 双modem时，设置为2的时候返回错误 */
    if (modemId == 2) {
        AT_ERR_LOG1("At_SetCdmaModemSwitch: ulModemId id Error! ulModemId: ", modemId);

        return AT_ERROR;
    }
#endif

    /*
     * CDMA支持切换需满足以下条件:
     * 1、CDMA能力只能在一个MODEM;
     * 2、目标modem上不存在CDMA，如果已经存在CDMA则直接返回OK
     * 3、进行C模切换后的modem平台支持接入技术个数不为0
     */
    checkParaRlt = AT_CheckSwitchCdmaModePara(&modem0PlatRat, &modem1PlatRat, &modem2PlatRat, modemId);
    if (checkParaRlt == AT_ERROR) {
        AT_ERR_LOG("At_SetCdmaModemSwitch: AT_CheckSwitchCdmaModePara return Error!");

        return AT_ERROR;
    } else if (checkParaRlt == AT_OK) {
        return AT_OK;
    } else {
        /* pclint */
    }

    result = AT_SwitchCdmaMode(indexNum, modemId, &modem0PlatRat, &modem1PlatRat, &modem2PlatRat);
    if (result == VOS_FALSE) {
        AT_ERR_LOG("At_SetCdmaModemSwitch: AT_SwitchCdmaMode return Error!");
        return AT_ERROR;
    } else {
        AT_ERR_LOG1("At_SetCdmaModemSwitch: AT_SwitchCdmaMode return Result : ", result);
        return AT_WAIT_ASYNC_RETURN;
    }

#else
    return AT_OK;
#endif
#else
    return AT_OK;
#endif
}

LOCAL VOS_UINT32 AT_CdmaModemIdSet(VOS_UINT32 modem0SupportCMode, VOS_UINT32 modem1SupportCMode,
                                   AT_MTA_CdmaCapResumeSetReq *atCmd)
{
    if ((modem0SupportCMode == VOS_TRUE) && (modem1SupportCMode == VOS_FALSE)) {
        atCmd->cdmaModemId = MODEM_ID_0;
        return AT_SUCCESS;
    } else if ((modem0SupportCMode == VOS_FALSE) && (modem1SupportCMode == VOS_TRUE)) {
        atCmd->cdmaModemId = MODEM_ID_1;
        return AT_SUCCESS;
    } else if ((modem0SupportCMode == VOS_FALSE) && (modem1SupportCMode == VOS_FALSE)) {
        /*  两个modem都没有cdma能力，不需要恢复 */
        AT_NORM_LOG("At_SetCdmaCapResume:Modem0 and modem1 not have cdma capa!");
        return AT_OK;
    } else { /* 当前不支持双cdma能力，如果配置，启动失败，物理层初始化失败 */
        AT_ERR_LOG("At_SetCdmaCapResume:Modem0 and modem1 both have cdma capa!");
        return AT_ERROR;
    }
}

/*
 * 功能描述: 从NV中获取当前平台能力
 */
LOCAL VOS_UINT32 At_GetNvPlatformRatCap(PLATAFORM_RatCapability *modem0PlatRat, PLATAFORM_RatCapability *modem1PlatRat)
{
    /* 读取nv出厂区配置 */
    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_0, NV_ITEM_PLATFORM_RAT_CAP, modem0PlatRat,
                                  sizeof(PLATAFORM_RatCapability)) != NV_OK) {
        AT_ERR_LOG("At_GetNvPlatformRatCap: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem0!");
        return VOS_FALSE;
    } else {
        AT_WARN_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, modem0PlatRat);
    }

    /* 读取nv出厂区配置 */
    if (TAF_ACORE_NV_READ_FACTORY(MODEM_ID_1, NV_ITEM_PLATFORM_RAT_CAP, modem1PlatRat,
                                  sizeof(PLATAFORM_RatCapability)) != NV_OK) {
        AT_ERR_LOG("At_GetNvPlatformRatCap: Read Nv Fail, NvId PlatForm_Rat_Cap,Modem1!");
        return VOS_FALSE;
    } else {
        AT_WARN_LOG("Read PlatForm from Factory success: ");
        AT_ReadPlatFormPrint(MODEM_ID_0, modem1PlatRat);
    }

    if ((modem0PlatRat->ratNum == 0) || (modem1PlatRat->ratNum == 0)) {
        AT_ERR_LOG("At_GetNvPlatformRatCap: Factory PlatForm RatNum is 0");
        return VOS_FALSE;
    }

    modem0PlatRat->ratNum = AT_MIN(modem0PlatRat->ratNum, PLATFORM_MAX_RAT_NUM);
    modem1PlatRat->ratNum = AT_MIN(modem1PlatRat->ratNum, PLATFORM_MAX_RAT_NUM);

    return VOS_TRUE;
}

VOS_UINT32 At_SetCdmaCapResume(VOS_UINT8 indexNum)
{
    PLATAFORM_RatCapability    modem0PlatRat;
    PLATAFORM_RatCapability    modem1PlatRat;
    AT_MTA_CdmaCapResumeSetReq atCmd;
    VOS_UINT32                 modem0Support1XFlg;
    VOS_UINT32                 modem0SupportHrpdFlg;
    VOS_UINT32                 modem1Support1XFlg;
    VOS_UINT32                 modem1SupportHrpdFlg;
    VOS_UINT32                 modem0SupportCMode;
    VOS_UINT32                 modem1SupportCMode;
    VOS_UINT32                 rslt;
    errno_t                    memResult;
    ModemIdUint16              modemId;

    (VOS_VOID)memset_s(&modem0PlatRat, sizeof(modem0PlatRat), 0x00, sizeof(modem0PlatRat));
    (VOS_VOID)memset_s(&modem1PlatRat, sizeof(modem1PlatRat), 0x00, sizeof(modem1PlatRat));
    (VOS_VOID)memset_s(&atCmd, sizeof(atCmd), 0x00, sizeof(atCmd));

    AT_WARN_LOG("At_SetCdmaCapResume Entry: ");

    /* 改命令仅支持在Modem0上发起 */
    if (AT_GetModemIdFromClient(g_atClientTab[indexNum].clientId, &modemId) != VOS_OK) {
        AT_ERR_LOG("At_SetCdmaCapResume: Get modem id fail!");

        return AT_ERROR;
    }

    if (modemId != MODEM_ID_0) {
        AT_ERR_LOG1("At_SetCdmaCapResume:  modem is not modem0!", modemId);
        return AT_ERROR;
    }

    if (At_GetNvPlatformRatCap(&modem0PlatRat, &modem1PlatRat) == VOS_FALSE) {
        return AT_ERROR;
    }

    memResult = memcpy_s(&(atCmd.modem0Platform), sizeof(atCmd.modem0Platform), &modem0PlatRat,
                         sizeof(PLATAFORM_RatCapability));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmd.modem0Platform), sizeof(PLATAFORM_RatCapability));

    memResult = memcpy_s(&(atCmd.modem1Platform), sizeof(atCmd.modem1Platform), &modem1PlatRat,
                         sizeof(PLATAFORM_RatCapability));
    TAF_MEM_CHK_RTN_VAL(memResult, sizeof(atCmd.modem1Platform), sizeof(PLATAFORM_RatCapability));

    /*
     * 检查cdma能力在哪个modem上，为了防止出现两个modem都存在cdma能力(会复位)，
     * 恢复时，先恢复无cdma能力的modem
     */
    modem0Support1XFlg   = AT_IsPlatformSupport1XMode(&modem0PlatRat);
    modem0SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem0PlatRat);
    modem1Support1XFlg   = AT_IsPlatformSupport1XMode(&modem1PlatRat);
    modem1SupportHrpdFlg = AT_IsPlatformSupportHrpdMode(&modem1PlatRat);

    modem0SupportCMode = (modem0Support1XFlg | modem0SupportHrpdFlg);
    modem1SupportCMode = (modem1Support1XFlg | modem1SupportHrpdFlg);

    rslt = AT_CdmaModemIdSet(modem0SupportCMode, modem1SupportCMode, &atCmd);
    if (rslt != AT_SUCCESS) {
        return rslt;
    }

    if (AT_FillAndSndAppReqMsg(g_atClientTab[indexNum].clientId, 0, ID_AT_MTA_CDMA_CAP_RESUME_SET_REQ,
                               (VOS_UINT8 *)&atCmd, sizeof(AT_MTA_CdmaCapResumeSetReq),
                               I0_UEPS_PID_MTA) == TAF_SUCCESS) {
        AT_ERR_LOG("At_SetCdmaCapResume, AT_FillAndSndAppReqMsg CDMA_CAP_RESUME_SET_REQ return SUCCESS");
        /* 设置当前操作类型 */
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CDMACAPRESUME_SET;
        return AT_WAIT_ASYNC_RETURN; /* 返回命令处理挂起状态 */
    } else {
        AT_ERR_LOG("At_SetCdmaCapResume,AT_FillAndSndAppReqMsg return ERROR");
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetClDbDomainStatusPara(VOS_UINT8 indexNum)
{
    VOS_UINT8 mode;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数个数不等于1，返回AT_CME_INCORRECT_PARAMETERS */
    if (g_atParaIndex != 1) {
        AT_WARN_LOG("AT_SetClDbDomainStatusPara: At Para Num Error.");
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 填写消息内容 */
    mode = (VOS_UINT8)g_atParaList[0].paraValue;

    if (TAF_MMA_SetClDbDomainStatus(WUEPS_PID_AT, g_atClientTab[indexNum].clientId, 0, mode) != VOS_TRUE) {
        return AT_ERROR;
    }

    g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CLDBDOMAINSTATUS_SET;

    return AT_WAIT_ASYNC_RETURN;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_SetCdmaAttDiversitySwitch(VOS_UINT8 indexNum)
{
    VOS_UINT32 ret;
    VOS_UINT8  nVWrLen;
    VOS_UINT8  diversitySwitch;

    if (AT_IsNVWRAllowedNvId(en_NV_Item_CPROC_1X_NVIM_DM_THRESHOLD) != VOS_TRUE) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    if (g_atParaList[0].paraLen == 0) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    diversitySwitch = (VOS_UINT8)g_atParaList[0].paraValue;
    nVWrLen         = sizeof(diversitySwitch);

    ret = TAF_ACORE_NV_WRITE_PART(MODEM_ID_0, en_NV_Item_CPROC_1X_NVIM_DM_THRESHOLD, 0, &diversitySwitch, nVWrLen);
    if (ret != ERR_MSP_SUCCESS) {
        return AT_ERROR;
    }

    return AT_OK;
}
#endif


#if ((FEATURE_AGPS == FEATURE_ON) && (FEATURE_XPDS == FEATURE_ON))
VOS_UINT32 AT_CagpsSndXpdsReq(VOS_UINT8 indexNum, AT_XPDS_MsgTypeUint32 msgType, VOS_UINT32 msgStructSize)
{
    errno_t      memResult;
    VOS_UINT32   dataLen;
    VOS_UINT32   msgLength;
    AT_XPDS_Msg *atXpdsMsg = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_CAGPS_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不正确 */
    if (g_atParaList[0].paraValue != g_atParaList[1].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[1].para, &g_atParaList[1].paraLen) == AT_FAILURE) {
        AT_ERR_LOG1("AT_CagpsSndXpdsReq: At_AsciiNum2HexString fail.", msgType);

        return AT_CME_INCORRECT_PARAMETERS;
    }

    dataLen = msgStructSize - VOS_MSG_HEAD_LENGTH - sizeof(VOS_UINT32) - sizeof(AT_APPCTRL);

    /* 消息内容长度是否正确 */
    if (dataLen != g_atParaList[1].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (msgStructSize <= VOS_MSG_HEAD_LENGTH) {
        AT_ERR_LOG("AT_CagpsSndXpdsReq: atXpdsMsg cannot malloc space !");
        return AT_ERROR;
    }
    msgLength = msgStructSize - VOS_MSG_HEAD_LENGTH;


    atXpdsMsg = (AT_XPDS_Msg *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (atXpdsMsg == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)atXpdsMsg, WUEPS_PID_AT, AT_GetDestPid(atXpdsMsg->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    atXpdsMsg->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    atXpdsMsg->appCtrl.opId     = g_atClientTab[indexNum].opId;

    if (dataLen > 0) {
        memResult = memcpy_s(((VOS_UINT8 *)atXpdsMsg) + VOS_MSG_HEAD_LENGTH + sizeof(VOS_UINT32) + sizeof(AT_APPCTRL),
                             dataLen, g_atParaList[1].para, g_atParaList[1].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, dataLen, g_atParaList[1].paraLen);
    }

    atXpdsMsg->msgId = msgType;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, atXpdsMsg) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCagpsCfgPosMode(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_CFG_POSITION_MODE_REQ, sizeof(AT_XPDS_GpsCfgPositionModeReq));

    if (result == AT_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CAGPSCFGPOSMODE_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return result;
}

VOS_UINT32 AT_SetCagpsStart(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_START_REQ, sizeof(AT_XPDS_GpsStartReq));

    if (result == AT_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CAGPSSTART_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return result;
}

VOS_UINT32 AT_SetCagpsStop(VOS_UINT8 indexNum)
{
    AT_XPDS_GpsStopReq *msgStopReq = VOS_NULL_PTR;
    VOS_UINT32          msgLength;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    msgLength = sizeof(AT_XPDS_GpsStopReq) - VOS_MSG_HEAD_LENGTH;

    msgStopReq = (AT_XPDS_GpsStopReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (msgStopReq == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgStopReq, WUEPS_PID_AT, AT_GetDestPid(msgStopReq->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    msgStopReq->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgStopReq->appCtrl.opId     = g_atClientTab[indexNum].opId;

    msgStopReq->msgId = ID_AT_XPDS_GPS_STOP_REQ;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgStopReq) == VOS_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CAGPSSTOP_SET;

        return AT_WAIT_ASYNC_RETURN;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCagpsCfgMpcAddr(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_CFG_MPC_ADDR_REQ, sizeof(AT_XPDS_GpsCfgMpcAddrReq));

    if (result == AT_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CAGPSCFGMPCADDR_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return result;
}

VOS_UINT32 AT_SetCagpsCfgPdeAddr(VOS_UINT8 indexNum)
{
    VOS_UINT32 result;

    result = AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_CFG_PDE_ADDR_REQ, sizeof(AT_XPDS_GpsCfgPdeAddrReq));

    if (result == AT_OK) {
        g_atClientTab[indexNum].cmdCurrentOpt = AT_CMD_CAGPSCFGPDEADDR_SET;

        return AT_WAIT_ASYNC_RETURN;
    }

    return result;
}

VOS_UINT32 AT_SetCagpsQryRefloc(VOS_UINT8 indexNum)
{
    VOS_UINT32                   msgLength;
    AT_XPDS_GpsQryReflocInfoReq *msgQryRefLoc = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_CMD_NO_PARA) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    msgLength = sizeof(AT_XPDS_GpsQryReflocInfoReq) - VOS_MSG_HEAD_LENGTH;

    msgQryRefLoc = (AT_XPDS_GpsQryReflocInfoReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (msgQryRefLoc == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgQryRefLoc, WUEPS_PID_AT,
                  AT_GetDestPid(msgQryRefLoc->appCtrl.clientId, I0_UEPS_PID_XPDS), msgLength);

    msgQryRefLoc->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgQryRefLoc->appCtrl.opId     = g_atClientTab[indexNum].opId;
    msgQryRefLoc->msgId            = ID_AT_XPDS_GPS_QRY_REFLOC_INFO_REQ;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgQryRefLoc) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCagpsQryTime(VOS_UINT8 indexNum)
{
    AT_XPDS_GpsQryTimeInfoReq *msgQryTime = VOS_NULL_PTR;
    VOS_UINT32                 msgLength;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_CAGPSQRYTIME_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CAGPSQRYTIME_PARA_DATA].para,
                              &g_atParaList[AT_CAGPSQRYTIME_PARA_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("AT_SetCagpsQryTime: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 消息内容长度是否正确 */
    if (sizeof(VOS_UINT32) != g_atParaList[AT_CAGPSQRYTIME_PARA_DATA].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 按照方案要求，1:reply time sync info；2:reply time sync info and GPS assist data */
    if ((g_atParaList[AT_CAGPSQRYTIME_PARA_DATA].para[0] == 0) ||
        (g_atParaList[AT_CAGPSQRYTIME_PARA_DATA].para[0] >= AT_XPDS_REPLY_SYNC_BUTT)) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    msgLength = sizeof(AT_XPDS_GpsQryTimeInfoReq) - VOS_MSG_HEAD_LENGTH;

    msgQryTime = (AT_XPDS_GpsQryTimeInfoReq *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (msgQryTime == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgQryTime, WUEPS_PID_AT, AT_GetDestPid(msgQryTime->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    msgQryTime->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgQryTime->appCtrl.opId     = g_atClientTab[indexNum].opId;

    msgQryTime->actionType = g_atParaList[AT_CAGPSQRYTIME_PARA_DATA].para[0];
    msgQryTime->msgId = ID_AT_XPDS_GPS_QRY_TIME_INFO_REQ;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgQryTime) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_SetCagpsPrmInfo(VOS_UINT8 indexNum)
{
    return AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_PRM_INFO_RSP, sizeof(AT_XPDS_GpsPrmInfoRsp));
}

VOS_UINT32 AT_SetCagpsReplyNiReq(VOS_UINT8 indexNum)
{
    return AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_REPLY_NI_REQ, sizeof(AT_XPDS_GpsReplyNiRsp));
}

VOS_UINT32 At_SetAgpsDataCallStatus(VOS_UINT8 indexNum)
{
    AT_XPDS_ApDataCallStatusInd *dataCallInd = VOS_NULL_PTR;
    VOS_UINT32                   msgLength;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > 1) {
        return AT_TOO_MANY_PARA;
    }

    if (g_atParaList[0].paraValue >= AT_XPDS_DATA_CALL_STATUS_BUTT) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    msgLength = sizeof(AT_XPDS_ApDataCallStatusInd) - VOS_MSG_HEAD_LENGTH;

    dataCallInd = (AT_XPDS_ApDataCallStatusInd *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (dataCallInd == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)dataCallInd, WUEPS_PID_AT, AT_GetDestPid(dataCallInd->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    dataCallInd->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    dataCallInd->appCtrl.opId     = At_GetOpId();

    dataCallInd->channelState = g_atParaList[0].paraValue;

    dataCallInd->msgId = ID_AT_XPDS_AP_DATA_CALL_STATUS_IND;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, dataCallInd) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetAgpsUpBindStatus(VOS_UINT8 indexNum)
{
    AT_XPDS_ApServerBindStatusInd *msgBindInd = VOS_NULL_PTR;
    VOS_UINT32                     msgLength;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CUPBINDSTATUS_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    if ((g_atParaList[AT_CUPBINDSTATUS_PARA_SERVERMODE].paraValue >= AT_XPDS_SERVER_MODE_BUTT) ||
        (g_atParaList[AT_CUPBINDSTATUS_PARA_BINDSTATUS].paraValue >= AT_XPDS_SERVER_BIND_STATUS_BUTT)) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    msgLength = sizeof(AT_XPDS_ApServerBindStatusInd) - VOS_MSG_HEAD_LENGTH;

    msgBindInd = (AT_XPDS_ApServerBindStatusInd *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (msgBindInd == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgBindInd, WUEPS_PID_AT, AT_GetDestPid(msgBindInd->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    msgBindInd->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgBindInd->appCtrl.opId     = At_GetOpId();

    msgBindInd->serverMode = g_atParaList[AT_CUPBINDSTATUS_PARA_SERVERMODE].paraValue;
    msgBindInd->bindStatus = g_atParaList[AT_CUPBINDSTATUS_PARA_BINDSTATUS].paraValue;
    msgBindInd->msgId      = ID_AT_XPDS_AP_SERVER_BIND_STATUS_IND;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgBindInd) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetAgpsForwardData(VOS_UINT8 indexNum)
{
    VOS_UINT32                dataLen;
    VOS_UINT32                msgLength;
    AT_XPDS_ApForwardDataInd *msgFwdDataInd = VOS_NULL_PTR;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_ERROR;
    }

    /* 参数过多 */
    if (g_atParaIndex > AT_CAGPSFORWARDDATA_PARA_MAX_NUM) {
        return AT_TOO_MANY_PARA;
    }

    if ((g_atParaList[AT_CAGPSFORWARDDATA_PARA_SERVER_MODE].paraValue >= AT_XPDS_SERVER_MODE_BUTT) ||
        (g_atParaList[AT_CAGPSFORWARDDATA_PARA_CURRENT_SEGMENTS].paraValue >
         g_atParaList[AT_CAGPSFORWARDDATA_PARA_TOTAL_SEGMENTS].paraValue)) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    /* 第4个参数值为奇数，返回错误 */
    if ((g_atParaList[AT_CAGPSFORWARDDATA_PARA_DATA_LEN].paraValue % AT_DOUBLE_LENGTH) == 1) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    /* the value of para4 and the len of para5 must be equal */
    if (g_atParaList[AT_CAGPSFORWARDDATA_PARA_DATA_LEN].paraValue !=
        g_atParaList[AT_CAGPSFORWARDDATA_PARA_DATA].paraLen) {
        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    dataLen = (g_atParaList[AT_CAGPSFORWARDDATA_PARA_DATA_LEN].paraValue / AT_ASCII_AND_HEX_CONVERSION_FACTOR);

    if (dataLen >= AT_DATA_DEFAULT_LENGTH) {
        msgLength = sizeof(AT_XPDS_ApForwardDataInd) + dataLen - VOS_MSG_HEAD_LENGTH - AT_DATA_DEFAULT_LENGTH;
    } else {
        msgLength = sizeof(AT_XPDS_ApForwardDataInd) - VOS_MSG_HEAD_LENGTH;
    }

    msgFwdDataInd = (AT_XPDS_ApForwardDataInd *)PS_ALLOC_MSG(WUEPS_PID_AT, msgLength);

    if (msgFwdDataInd == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgFwdDataInd, WUEPS_PID_AT,
                  AT_GetDestPid(msgFwdDataInd->appCtrl.clientId, I0_UEPS_PID_XPDS), msgLength);

    msgFwdDataInd->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgFwdDataInd->appCtrl.opId     = At_GetOpId();

    msgFwdDataInd->serverMode = g_atParaList[AT_CAGPSFORWARDDATA_PARA_SERVER_MODE].paraValue;
    msgFwdDataInd->totalNum   = g_atParaList[AT_CAGPSFORWARDDATA_PARA_TOTAL_SEGMENTS].paraValue;
    msgFwdDataInd->curNum     = g_atParaList[AT_CAGPSFORWARDDATA_PARA_CURRENT_SEGMENTS].paraValue;
    msgFwdDataInd->dataLen    = dataLen;

    if (At_AsciiString2HexSimple(msgFwdDataInd->data, g_atParaList[AT_CAGPSFORWARDDATA_PARA_DATA].para,
                                 (VOS_UINT16)g_atParaList[AT_CAGPSFORWARDDATA_PARA_DATA_LEN].paraValue) != AT_SUCCESS) {
        PS_FREE_MSG(WUEPS_PID_AT, msgFwdDataInd);  //lint !e516

        return AT_CME_OPERATION_NOT_ALLOWED;
    }

    msgFwdDataInd->msgId = ID_AT_XPDS_AP_FORWARD_DATA_IND;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgFwdDataInd) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCagpsPosInfo(VOS_UINT8 indexNum)
{
    if (AT_CagpsSndXpdsReq(indexNum, ID_AT_XPDS_GPS_POS_INFO_RSP, sizeof(AT_XPDS_GpsPosInfoRsp)) == AT_OK) {
        return AT_OK;
    }

    return AT_ERROR;
}

VOS_UINT32 At_SetCgpsControlStart(VOS_UINT8 indexNum)
{
    AT_XPDS_Msg *msgStr = VOS_NULL_PTR;
    VOS_UINT32   msgLength;
    errno_t      memResult;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_CGPSCONTROLSTART_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不正确 */
    if (g_atParaList[AT_CGPSCONTROLSTART_DATA_LEN].paraValue != g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CGPSCONTROLSTART_DATA].para,
                              &g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetgpsControlStart: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen >= AT_CGPSCONTROLSTART_DATA_MAX_LEN) {
        msgLength = sizeof(AT_XPDS_Msg) + g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen -
                    VOS_MSG_HEAD_LENGTH - AT_CGPSCONTROLSTART_DATA_MAX_LEN;
    } else {
        msgLength = sizeof(AT_XPDS_Msg) - VOS_MSG_HEAD_LENGTH;
    }

    msgStr = (AT_XPDS_Msg *)PS_ALLOC_MSG(UEPS_PID_XPDS, msgLength);

    if (msgStr == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgStr, WUEPS_PID_AT, AT_GetDestPid(msgStr->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    msgStr->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgStr->appCtrl.opId     = g_atClientTab[indexNum].opId;

    if (g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen > 0) {
        memResult = memcpy_s(msgStr->content, g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen,
                             g_atParaList[AT_CGPSCONTROLSTART_DATA].para,
                             g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen,
                            g_atParaList[AT_CGPSCONTROLSTART_DATA].paraLen);
    }

    msgStr->msgId = ID_AT_XPDS_UTS_TEST_START_REQ;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgStr) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 At_SetCgpsControlStop(VOS_UINT8 indexNum)
{
    AT_XPDS_Msg *msgStr = VOS_NULL_PTR;
    VOS_UINT32   msgLength;
    errno_t      memResult;

    /* 参数检查 */
    if (g_atParseCmd.cmdOptType != AT_CMD_OPT_SET_PARA_CMD) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 参数过多 */
    if (g_atParaIndex != AT_CGPSCONTROLSTOP_PARA_NUM) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 字符串长度不正确 */
    if (g_atParaList[AT_CGPSCONTROLSTOP_DATA_LEN].paraValue != g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen) {
        return AT_CME_INCORRECT_PARAMETERS;
    }

    /* 将字符串转换为16进制数组 */
    if (At_AsciiNum2HexString(g_atParaList[AT_CGPSCONTROLSTOP_DATA].para,
                              &g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen) == AT_FAILURE) {
        AT_ERR_LOG("At_SetCgpsControlStop: At_AsciiNum2HexString fail.");

        return AT_CME_INCORRECT_PARAMETERS;
    }

    if (g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen >= AT_CGPSCONTROLSTOP_DATA_MAX_LEN) {
        msgLength = sizeof(AT_XPDS_Msg) + g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen - VOS_MSG_HEAD_LENGTH -
                    AT_CGPSCONTROLSTOP_DATA_MAX_LEN;
    } else {
        msgLength = sizeof(AT_XPDS_Msg) - VOS_MSG_HEAD_LENGTH;
    }

    msgStr = (AT_XPDS_Msg *)PS_ALLOC_MSG(UEPS_PID_XPDS, msgLength);

    if (msgStr == VOS_NULL_PTR) {
        return AT_ERROR;
    }

    TAF_CfgMsgHdr((MsgBlock *)msgStr, UEPS_PID_XPDS, AT_GetDestPid(msgStr->appCtrl.clientId, I0_UEPS_PID_XPDS),
                  msgLength);

    msgStr->appCtrl.clientId = g_atClientTab[indexNum].clientId;
    msgStr->appCtrl.opId     = g_atClientTab[indexNum].opId;

    if (g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen > 0) {
        memResult = memcpy_s(msgStr->content, g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen,
                             g_atParaList[AT_CGPSCONTROLSTOP_DATA].para,
                             g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen);
        TAF_MEM_CHK_RTN_VAL(memResult, g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen,
                            g_atParaList[AT_CGPSCONTROLSTOP_DATA].paraLen);
    }

    msgStr->msgId = ID_AT_XPDS_UTS_TEST_STOP_REQ;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msgStr) == VOS_OK) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

#endif













#endif

