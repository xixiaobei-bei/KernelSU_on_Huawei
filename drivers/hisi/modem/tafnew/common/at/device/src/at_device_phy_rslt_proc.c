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
#include "at_device_phy_rslt_proc.h"
#include "securec.h"

#include "at_ctx.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_lte_common.h"
#include "gen_msg.h"
#include "osm.h"
#include "at_device_comm.h"

#include "nv_stru_gucnas.h"
#include "nv_stru_sys.h"
#include "nv_stru_gas.h"
#include "nv_stru_pam.h"
#include "nv_stru_msp_interface.h"
#include "nv_stru_msp.h"
#include "at_external_module_msg_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_PHY_RSLT_PROC_C

#define AT_RFIC_NUM_2 2
#define AT_RFIC_NUM_3 3

#define AT_CAL_MIPIDEV_BYTE_CNT_VALUE_2 2
#define AT_CAL_MIPIDEV_BYTE_CNT_VALUE_3 3

VOS_UINT32 AT_RcvDrvAgentQryProdtypeRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg            *rcvMsg      = VOS_NULL_PTR;
    DRV_AGENT_ProdtypeQryCnf *prodTypeCnf = VOS_NULL_PTR;
    VOS_UINT8                 indexNum    = 0;

    /* 初始化消息 */
    rcvMsg      = (DRV_AGENT_Msg *)msg;
    prodTypeCnf = (DRV_AGENT_ProdtypeQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(prodTypeCnf->atAppCtrl.clientId, AT_CMD_PRODTYPE_QRY, &indexNum) != VOS_OK) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    g_atSendDataBuff.bufLen =
        (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            prodTypeCnf->prodType);

    /* 调用AT_FormATResultDATa发送命令结果 */
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT16 AT_RficDieIDOut(VOS_UINT8 *msg, VOS_UINT32 rficNum, VOS_UINT16 length, VOS_UINT8 indexNum)
{
    VOS_UINT32 i; /* 循环用 */
    VOS_UINT16 lengthtemp = length;

    lengthtemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthtemp, "%s: %d,\"", g_parseContext[indexNum].cmdElement->cmdName,
        rficNum);

    /* RFIC ID 使用低八位数据, 所以循环值一次加2 */
    for (i = 0; i < (MTA_AT_MAX_DIE_ID_LEN * AT_DOUBLE_LENGTH); i += 2) {
        lengthtemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthtemp, "%02x", msg[i]);
    }

    lengthtemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthtemp, "\"%s", g_atCrLf);
    return lengthtemp;
}

VOS_UINT32 AT_RcvMtaRficDieIDQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg          = VOS_NULL_PTR;
    MTA_AT_RficDieIdReqCnf *rficDieIDReqCnf = VOS_NULL_PTR;
    VOS_UINT16              length;

    /* 初始化局部变量 */
    rcvMsg          = (AT_MTA_Msg *)msg;
    rficDieIDReqCnf = (MTA_AT_RficDieIdReqCnf *)rcvMsg->content;
    length          = 0;

    /* 格式化上报命令 */
    if (rficDieIDReqCnf->result != MTA_AT_RESULT_NO_ERROR) {
        /* 命令结果 *AT_ERROR */
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    } else {
        /* 命令结果 *AT_OK */
        if (rficDieIDReqCnf->rfic0DieIdValid == VOS_TRUE) {
            length = AT_RficDieIDOut((VOS_UINT8 *)rficDieIDReqCnf->rfic0DieId, 0, length, indexNum);
        }

        if (rficDieIDReqCnf->rfic1DieIdValid == VOS_TRUE) {
            length = AT_RficDieIDOut((VOS_UINT8 *)rficDieIDReqCnf->rfic1DieId, 1, length, indexNum);
        }

        if (rficDieIDReqCnf->rfic2DieIdValid == VOS_TRUE) {
            length = AT_RficDieIDOut((VOS_UINT8 *)rficDieIDReqCnf->rfic2DieId, AT_RFIC_NUM_2, length, indexNum);
        }
        if (rficDieIDReqCnf->rfic3DieIdValid == VOS_TRUE) {
            length = AT_RficDieIDOut((VOS_UINT8 *)rficDieIDReqCnf->rfic3DieId, AT_RFIC_NUM_3, length, indexNum);
        }

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    }
}

VOS_UINT16 AT_RffeDieIDOut(VOS_UINT8 *msg, VOS_UINT32 rffeNum, VOS_UINT16 length, VOS_UINT8 indexNum)
{
    VOS_UINT32 i; /* 循环用 */
    VOS_UINT16 lengthtemp = length;

    lengthtemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthtemp, "%s: %d,\"", g_parseContext[indexNum].cmdElement->cmdName,
        rffeNum);

    /* RFFE ID */
    for (i = 0; i < MTA_AT_MAX_RFFE_DIE_ID_LEN; i++) {
        lengthtemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + lengthtemp, "%02x", msg[i]);
    }

    lengthtemp += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + lengthtemp, "\"%s", g_atCrLf);
    return lengthtemp;
}

VOS_UINT32 AT_RcvMtaRffeDieIDQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *rcvMsg          = VOS_NULL_PTR;
    MTA_AT_RffeDieIdReqCnf *rffeDieIDReqCnf = VOS_NULL_PTR;
    VOS_UINT16              dataLen;
    VOS_UINT32              i;

    /* 初始化局部变量 */
    rcvMsg          = (AT_MTA_Msg *)msg;
    rffeDieIDReqCnf = (MTA_AT_RffeDieIdReqCnf *)rcvMsg->content;
    dataLen         = 0;
    i               = 0;

    /* 格式化上报命令 */
    if (rffeDieIDReqCnf->result != MTA_AT_RESULT_NO_ERROR) {
        /* 命令结果 *AT_ERROR */
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    } else {
        /* 命令结果 *AT_OK */
        for (i = 0; i < MTA_AT_MAX_RFFE_DIE_ID_COUNT; i++) {
            if (rffeDieIDReqCnf->dieIdData[i].dataValid == MTA_AT_RFFE_DIE_ID_VALID) {
                dataLen = AT_RffeDieIDOut((VOS_UINT8 *)rffeDieIDReqCnf->dieIdData[i].rffeDieId, i, dataLen, indexNum);
            }
        }

        g_atSendDataBuff.bufLen = dataLen;
        return AT_OK;
    }
}

/*
 * 功能描述: 通用AT命令查询返回处理函数
 */
VOS_UINT32 atQryLTCommCmdParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG             *event = NULL;
    FTM_RdLtcommcmdCnf *cnf   = NULL;
    VOS_UINT32          crc;
    VOS_UINT16          crcL;
    VOS_UINT16          crcH;
    VOS_UINT16          length = 0;
    VOS_UINT32          i      = 0;

    if (msgBlock == VOS_NULL_PTR) {
        PS_PRINTF_WARNING("<%s> pMsgBlock null\n", __FUNCTION__);
        return ERR_MSP_INVALID_PARAMETER;
    }

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdLtcommcmdCnf *)event->param1;

    /* 计算CRC */
    crc  = Calc_CRC32((VOS_UINT8 *)cnf->data, cnf->dataLen);
    crcL = (unsigned short)(crc & 0x0000FFFF);
    crcH = (unsigned short)(crc >> 16);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^LTCOMMCMD:%d,%d,%d,", cnf->dataLen, crcL, crcH);

    for (i = 0; i < cnf->dataLen; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%02X", (VOS_UINT8)cnf->data[i]);
    }

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

/*
 * 功能描述: 通用命令返回处理函数
 */
VOS_UINT32 atSetLTCommCmdParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG              *event = NULL;
    FTM_SetLtcommcmdCnf *cnf   = NULL;
    VOS_UINT32           crc;
    VOS_UINT16           crcL;
    VOS_UINT16           crcH;
    VOS_UINT16           length = 0;
    VOS_UINT32           i      = 0;

    if (msgBlock == VOS_NULL_PTR) {
        PS_PRINTF_WARNING("<%s> pMsgBlock null\n", __FUNCTION__);
        return ERR_MSP_INVALID_PARAMETER;
    }
    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetLtcommcmdCnf *)event->param1;

    /* 计算CRC */
    crc  = Calc_CRC32((VOS_UINT8 *)cnf->data, cnf->dataLen);
    crcL = (VOS_UINT16)(crc & 0x0000FFFF);
    crcH = (VOS_UINT16)(crc >> 16);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^LTCOMMCMD:%d,%d,%d,", cnf->dataLen, crcL, crcH);

    for (i = 0; i < cnf->dataLen; i++) {
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
            (VOS_CHAR *)g_atSndCodeAddr + length, "%02X", (VOS_UINT8)cnf->data[i]);
    }

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetFWAVEParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG          *event = NULL;
    FTM_SetFwaveCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetFwaveCnf *)event->param1;

    CmdErrProc(clientId, cnf->errCode, 0, NULL);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetFCHANParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG          *event = NULL;
    FTM_SetFchanCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetFchanCnf *)event->param1;


    if (cnf->errCode == ERR_MSP_SUCCESS) {
#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
        (VOS_VOID)AT_SetGlobalFchan((VOS_UINT8)(cnf->fchanMode));
#endif
    }

    CmdErrProc(clientId, cnf->errCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atQryFCHANParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    FTM_RdFchanCnf *cnf    = NULL;
    OS_MSG         *event  = NULL;
    VOS_UINT16      length = 0;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdFchanCnf *)event->param1;

    if (cnf == NULL) {
        return ERR_MSP_FAILURE;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FCHAN:%d,%d,%u,%u", cnf->fchanMode, cnf->band, cnf->ulChannel,
        cnf->dlChannel);

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetTselrfParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG           *event = NULL;
    FTM_SetTselrfCnf *cnf   = NULL;

    HAL_SDMLOG("\n enter atSetTselrfParaCnfProc !!!\n");

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetTselrfCnf *)event->param1;

    CmdErrProc(clientId, cnf->errCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetFTXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG         *event = NULL;
    FTM_SetTxonCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetTxonCnf *)event->param1;

    CmdErrProc(clientId, cnf->errCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atQryFTXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    FTM_RdTxonCnf *cnf   = NULL;
    OS_MSG        *event = NULL;
    VOS_UINT16     length;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdTxonCnf *)event->param1;

    if (cnf == NULL) {
        return ERR_MSP_FAILURE;
    }

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FTXON:%d", cnf->swtich);

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 At_SaveRxDivPara(VOS_UINT16 setDivBands, VOS_UINT8 rxDivCfg)
{
    TAF_AT_NvimRxdivConfig rxdivConfig;

    (VOS_VOID)memset_s(&rxdivConfig, sizeof(rxdivConfig), 0x00, sizeof(TAF_AT_NvimRxdivConfig));

    rxdivConfig.vaild = rxDivCfg;

    /*
     * NV项en_NV_Item_ANTENNA_CONFIG在V3R2B060后改成en_NV_Item_W_RF_DIV_BAND，
     * 两个NV项完全一样。
     */
    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, en_NV_Item_W_RF_DIV_BAND, (VOS_UINT8 *)&setDivBands, sizeof(VOS_UINT16)) !=
        NV_OK) {
        TAF_LOG(WUEPS_PID_AT, 0, PS_LOG_LEVEL_ERROR, "TAF_ACORE_NV_WRITE en_NV_Item_W_RF_DIV_BAND fail!\n");
        return AT_CME_RX_DIV_OTHER_ERR;
    }

    if (TAF_ACORE_NV_WRITE(MODEM_ID_0, NV_ITEM_RXDIV_CONFIG, (VOS_UINT8 *)&rxdivConfig,
                           sizeof(TAF_AT_NvimRxdivConfig)) != NV_OK) {
        TAF_LOG(WUEPS_PID_AT, 0, PS_LOG_LEVEL_ERROR, "TAF_ACORE_NV_WRITE en_NV_RXDIV_CONFIG fail!\n");
        return AT_CME_RX_DIV_OTHER_ERR;
    }

    return AT_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_VOID At_RfCfgCnfReturnSuccProc(VOS_UINT8 indexNum)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_SET_FTXON:
            atDevCmdCtrl->txOnOff = atDevCmdCtrl->tempRxorTxOnOff;

            /* 如果是打开发射机操作，需要记录最近一次执行的是打开发射机还是打开接收机操作 */
            if (atDevCmdCtrl->txOnOff == AT_DSP_RF_SWITCH_ON) {
                atDevCmdCtrl->rxonOrTxon = AT_TXON_OPEN;
            }
            break;

        case AT_CMD_SET_FRXON:
            atDevCmdCtrl->rxOnOff = atDevCmdCtrl->tempRxorTxOnOff;

            /* 如果是打开接收机操作，需要记录最近一次执行的是打开发射机还是打开接收机操作 */
            if (atDevCmdCtrl->rxOnOff == AT_DSP_RF_SWITCH_ON) {
                atDevCmdCtrl->rxonOrTxon = AT_RXON_OPEN;
            }
            break;

        case AT_CMD_QUERY_RSSI:
            break;

        case AT_CMD_SET_RXDIV:
            if ((At_SaveRxDivPara(atDevCmdCtrl->origBand, 1) == AT_OK) &&
                (atDevCmdCtrl->currentTMode == AT_TMODE_FTM)) {
                atDevCmdCtrl->priOrDiv = AT_RX_DIV_ON;
                atDevCmdCtrl->rxDiv    = atDevCmdCtrl->origBand;
            }
            break;

        case AT_CMD_SET_RXPRI:
            atDevCmdCtrl->priOrDiv = AT_RX_PRI_ON;
            atDevCmdCtrl->rxPri    = atDevCmdCtrl->origBand;
            break;

        default:
            break;
    }
}

VOS_UINT32 At_RfCfgCnfReturnErrProc(VOS_UINT8 indexNum)
{
    VOS_UINT32 rslt;

    switch (g_atClientTab[indexNum].cmdCurrentOpt) {
        case AT_CMD_SET_FTXON:
            rslt = AT_FTXON_SET_FAIL;
            break;

        case AT_CMD_SET_FRXON:
            rslt = AT_FRXON_SET_FAIL;
            break;

        case AT_CMD_QUERY_RSSI:
            rslt = AT_FRSSI_OTHER_ERR;
            break;

        /* 设置主集和分集时收到DSP回复出错情况下返回的错误码相同 */
        case AT_CMD_SET_RXDIV:
        case AT_CMD_SET_RXPRI:
            rslt = AT_CME_RX_DIV_OTHER_ERR;
            break;

        default:
            rslt = AT_ERROR;
            break;
    }

    return rslt;
}

VOS_VOID At_HpaRfCfgCnfProc(HPA_AT_RfCfgCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT8          indexNum;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (msg->errFlg == AT_HPA_RSLT_FAIL) {
        AT_INFO_LOG("At_HpaRfCfgCnfProc: set rfcfg err");
        rslt = At_RfCfgCnfReturnErrProc(indexNum);
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, rslt);
    } else {
        rslt = AT_OK;
        At_RfCfgCnfReturnSuccProc(indexNum);

        /*
         * ^FRSSI?在GDSP LOAD情况会收到ID_HPA_AT_RF_CFG_CNF消息,不用上报,等收到
         * ID_HPA_AT_RF_RX_RSSI_IND消息时再上报
         */
        if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_QUERY_RSSI) {
            AT_StopTimerCmdReady(indexNum);
            At_FormatResultData(indexNum, rslt);
        }
    }
}

#if (FEATURE_UE_MODE_CDMA == FEATURE_ON)
VOS_VOID At_CHpaRfCfgCnfProc(CHPA_AT_RfCfgCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT8          indexNum;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (msg->errFlg == AT_HPA_RSLT_FAIL) {
        AT_INFO_LOG("At_CHpaRfCfgCnfProc: set rfcfg err");
        rslt = At_RfCfgCnfReturnErrProc(indexNum);
        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, rslt);
    } else {
        rslt = AT_OK;
        At_RfCfgCnfReturnSuccProc(indexNum);

        AT_StopTimerCmdReady(indexNum);
        At_FormatResultData(indexNum, rslt);
    }
}
#endif

VOS_UINT32 AT_SetFchanRspErr(DRV_AGENT_FchanSetErrorUint32 result)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    if (result == DRV_AGENT_FCHAN_BAND_NOT_MATCH) {
        return AT_FCHAN_BAND_NOT_MATCH;
    }

    if (result == DRV_AGENT_FCHAN_BAND_CHANNEL_NOT_MATCH) {
        return AT_FCHAN_BAND_CHANNEL_NOT_MATCH;
    }

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    if (result == DRV_AGENT_FCHAN_OTHER_ERR) {
        atDevCmdCtrl->dspLoadFlag = VOS_FALSE;
        AT_WARN_LOG("AT_SetFChanPara: DSP Load fail!");
        return AT_FCHAN_OTHER_ERR;
    }

    return DRV_AGENT_FCHAN_SET_NO_ERROR;
}

VOS_UINT32 AT_RcvDrvAgentSetFchanRsp(struct MsgCB *msg)
{
    DRV_AGENT_Msg         *rcvMsg       = VOS_NULL_PTR;
    DRV_AGENT_FchanSetCnf *fchanSetCnf  = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl     *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT8              indexNum     = 0;
    VOS_UINT32             error;

    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 初始化 */
    rcvMsg      = (DRV_AGENT_Msg *)msg;
    fchanSetCnf = (DRV_AGENT_FchanSetCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(fchanSetCnf->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentSetFchanRsp:WARNING:AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentSetFchanRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_FCHAN_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FCHAN_SET) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 设置有错误的处理 */
    error = AT_SetFchanRspErr(fchanSetCnf->result);
    if (error != DRV_AGENT_FCHAN_SET_NO_ERROR) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, error);
        return VOS_OK;
    }

    /* 设置无错误的处理 */
    atDevCmdCtrl->dspLoadFlag   = VOS_TRUE;
    atDevCmdCtrl->deviceRatMode = (VOS_UINT8)fchanSetCnf->fchanSetReq.deviceRatMode;
    atDevCmdCtrl->deviceAtBand  = (VOS_UINT8)fchanSetCnf->fchanSetReq.deviceAtBand;
    atDevCmdCtrl->dspBandArfcn  = fchanSetCnf->fchanSetReq.dspBandArfcn;
    atDevCmdCtrl->fdac          = 0; /* FDAC清零，防止G/W范围错误 */

    /* 调用AT_FormATResultDATa发送命令结果 */
    g_atSendDataBuff.bufLen = 0;
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

VOS_UINT32 AT_RcvDrvAgentTseLrfSetRsp(struct MsgCB *msg)
{
    DRV_AGENT_TselrfSetCnf *event        = VOS_NULL_PTR;
    DRV_AGENT_Msg          *rcvMsg       = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl      *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32              ret;
    VOS_UINT8               indexNum = 0;

    /* 初始化 */
    rcvMsg       = (DRV_AGENT_Msg *)msg;
    event        = (DRV_AGENT_TselrfSetCnf *)rcvMsg->content;
    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentTseLrfSetRsp : AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待TSELRF设置命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_TSELRF_SET) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出查询结果 */
    g_atSendDataBuff.bufLen = 0;
    if (event->result == DRV_AGENT_TSELRF_SET_NO_ERROR) {
        /* 设置错误码为AT_OK */
        ret                         = AT_OK;
        atDevCmdCtrl->dspLoadFlag   = VOS_TRUE;
        atDevCmdCtrl->deviceRatMode = event->deviceRatMode;
        atDevCmdCtrl->fdac          = 0;
    } else {
        /* 查询失败返回ERROR字符串 */
        ret = AT_ERROR;
    }

    At_FormatResultData(indexNum, ret);
    return VOS_OK;
}

VOS_UINT32 At_MipiRdCnfProc(HPA_AT_MipiRdCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT32         data;
    VOS_UINT16         length = 0;
    VOS_UINT8          indexNum;

    /* 初始化本地变量 */
    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MIPI_RD) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (msg->result == AT_HPA_RSLT_FAIL) {
        AT_INFO_LOG("At_MipiRdCnfProc: read mipi err");
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
        data = msg->value;

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, data);
    }
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, rslt);
    return VOS_OK;
}

VOS_UINT32 At_MipiWrCnfProc(HPA_AT_MipiWrCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT8          indexNum;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MIPI_WR) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (msg->errFlg == AT_HPA_RSLT_FAIL) {
        AT_INFO_LOG("At_MipiWrCnfProc: set MipiCfg err");
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
    }

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 At_SsiWrCnfProc(HPA_AT_SsiWrCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT32         rslt;
    VOS_UINT8          indexNum;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SSI_WR) {
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (msg->errFlg == AT_HPA_RSLT_FAIL) {
        AT_INFO_LOG("At_SsiWrCnfProc: set rfcfg err");
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
    }

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_VOID At_RfFpowdetTCnfProc(PHY_AT_PowerDetCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT8          indexNum;
    VOS_UINT16         length;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPOWDET_QRY) {
        AT_WARN_LOG("At_RfFPOWDETCnfProc: CmdCurrentOpt is not AT_CMD_FPOWDET_QRY!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 应物理层要求，如果返回值为0x7FFF则为无效值，项查询者返回ERROR */
    if (msg->powerDet == 0x7FFF) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
    } else {
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, msg->powerDet);

        g_atSendDataBuff.bufLen = length;

        At_FormatResultData(indexNum, AT_OK);
    }
}

VOS_UINT32 AT_RcvMtaPowerDetQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    /* 定义局部变量 */
    AT_MTA_Msg            *mtaMsg         = VOS_NULL_PTR;
    MTA_AT_PowerDetQryCnf *powerDetQryCnf = VOS_NULL_PTR;
    PHY_AT_PowerDetCnf     powerNetMsg;

    /* 初始化消息变量 */
    mtaMsg         = (AT_MTA_Msg *)msg;
    powerDetQryCnf = (MTA_AT_PowerDetQryCnf *)mtaMsg->content;

    (VOS_VOID)memset_s(&powerNetMsg, sizeof(powerNetMsg), 0x00, sizeof(PHY_AT_PowerDetCnf));

    if (powerDetQryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        powerNetMsg.powerDet = powerDetQryCnf->powerDet;
    } else {
        powerNetMsg.powerDet = 0x7FFF;
    }

    At_RfFpowdetTCnfProc(&powerNetMsg);

    return AT_CMD_NO_NEED_FORMAT_RSLT;
}

VOS_VOID At_RfPllStatusCnfProc(PHY_AT_RfPllStatusCnf *msg)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;
    VOS_UINT8          indexNum;
    VOS_UINT16         length;

    atDevCmdCtrl = AT_GetDevCmdCtrl();
    /* 获取本地保存的用户索引 */
    indexNum = atDevCmdCtrl->index;

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPLLSTATUS_QRY) {
        AT_WARN_LOG("At_RfPllStatusCnfProc: CmdCurrentOpt is not AT_CMD_FPLLSTATUS_QRY!");
        return;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName, msg->txStatus,
        msg->rxStatus);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, AT_OK);
}

#else
VOS_UINT32 AT_ProcSetWorkModeCnf(struct MsgCB *msg)
{
    AT_CCBT_LOAD_PHY_CNF_STRU *loadCnf     = VOS_NULL_PTR;
    AT_MT_Info                *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT8                  indexNum;

    loadCnf     = (AT_CCBT_LOAD_PHY_CNF_STRU *)msg;
    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;

    AT_PR_LOGH("AT_ProcSetWorkModeCnf Enter");

    /* 下标保护 */
    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcSetWorkModeCnf: ulIndex err!");
        return VOS_ERR;
    }

    /* 判断当前操作类型是否为AT_CMD_FCHAN_SET */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FCHAN_SET) {
        AT_ERR_LOG("AT_ProcSetWorkModeCnf: Not AT_CMD_FCHAN_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 非0表示错误 */
    if (loadCnf->ulErrorCode != MT_OK) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_FCHAN_LOAD_DSP_ERR);
        return VOS_OK;
    }

    g_atSendDataBuff.bufLen         = 0;
    atMtInfoCtx->atInfo.dspLoadFlag = VOS_TRUE;
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_RcvBbicCalSetDpdtCnf(struct MsgCB *msg)
{
    BBIC_CAL_DPDT_IND_STRU *dpdtCnf     = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32              rslt = AT_ERROR;
    VOS_UINT8               indexNum = 0;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 获取本地保存的用户索引 */
    indexNum = atMtInfoCtx->atInfo.indexNum;
    dpdtCnf  = (BBIC_CAL_DPDT_IND_STRU *)msg;

    AT_PR_LOGH("AT_RcvBbicCalSetDpdtCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_RcvBbicCalSetDpdtCnf: ulIndex err !");
        return VOS_ERR;
    }

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DPDT_SET) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DPDTQRY_SET)) {
        AT_ERR_LOG("AT_RcvBbicCalSetDpdtCnf: CmdCurrentOpt is not AT_CMD_DPDT_SET or AT_CMD_DPDTQRY_SET!");
        return VOS_ERR;
    }

    AT_StopTimerCmdReady(indexNum);

    if (dpdtCnf->stPara.uwErrorCode != MT_OK) {
        AT_ERR_LOG1("AT_RcvBbicCalSetDpdtCnf: set dpdt error, ErrorCode is ", dpdtCnf->stPara.uwErrorCode);
        rslt = AT_ERROR;
    } else {
        /* 当操作类型状态为Get时，上报查询Dpdt结果 */
        if (dpdtCnf->stPara.unOperType == BBIC_DPDT_OPERTYPE_GET) {
            rslt = AT_OK;
            g_atSendDataBuff.bufLen =
                (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                    (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                    dpdtCnf->stPara.uwValue);
        }
        /* 当操作类型状态为Set时，返回设置成功 */
        else {
            rslt                    = AT_OK;
            g_atSendDataBuff.bufLen = 0;
        }
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 At_RcvBbicCalDcxoCnf(struct MsgCB *msg)
{
    BBIC_CAL_DCXO_IND_STRU *dcxoInd     = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32              rslt;
    VOS_UINT8               indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();
    /* 获取本地保存的用户索引 */
    indexNum = atMtInfoCtx->atInfo.indexNum;
    dcxoInd  = (BBIC_CAL_DCXO_IND_STRU *)msg;
    rslt     = AT_OK;

    AT_PR_LOGH("At_RcvBbicCalDcxoCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("At_RcvBbicCalDcxoCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_DCXOTEMPCOMP_SET) {
        AT_ERR_LOG("At_RcvBbicCalDcxoCnf: CmdCurrentOpt is not AT_CMD_DCXOTEMPCOMP_SET!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (dcxoInd->ulErrorCode != MT_OK) {
        rslt = AT_ERROR;
        AT_ERR_LOG1("At_RcvBbicCalDcxoCnf: ErrorCode is ", dcxoInd->ulErrorCode);
        g_atSendDataBuff.bufLen = 0;
    } else {
        rslt                    = AT_OK;
        g_atSendDataBuff.bufLen = 0;
        atMtInfoCtx->atInfo.dcxoTempCompEnableFlg =
            (AT_DCXOTEMPCOMP_EnableUint8)atMtInfoCtx->bbicInfo.dcxoTempCompEnableFlg;
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 AT_RcvBbicCalQryFtemprptCnf(struct MsgCB *msg)
{
    BBIC_CAL_TEMP_QRY_IND_STRU *ftemprptCnf = VOS_NULL_PTR;
    AT_MT_Info                 *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                  rslt = AT_ERROR;
    VOS_UINT8                   indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 获取本地保存的用户索引 */
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    ftemprptCnf = (BBIC_CAL_TEMP_QRY_IND_STRU *)msg;

    AT_PR_LOGH("AT_RcvBbicCalQryFtemprptCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_RcvBbicCalQryFtemprptCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FTEMPRPT_QRY) {
        AT_ERR_LOG("AT_RcvBbicCalQryFtemprptCnf: CmdCurrentOpt is not AT_CMD_OPT_READ_CMD!");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (ftemprptCnf->stPara.uwErrorCode != MT_OK) {
        rslt = AT_ERROR;
        AT_ERR_LOG1("AT_RcvBbicCalQryFtemprptCnf: qry Ftemprpt error, ErrorCode is ", ftemprptCnf->stPara.uwErrorCode);
        g_atSendDataBuff.bufLen = 0;
    } else {
        rslt = AT_OK;
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                ftemprptCnf->stPara.enChannelType, ftemprptCnf->stPara.hwChannelNum, ftemprptCnf->stPara.wTemperature);
    }

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 AT_ProcRxCnf(struct MsgCB *msg)
{
    BBIC_CAL_AT_MT_TRX_CNF *cnf         = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT8               indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    cnf         = (BBIC_CAL_AT_MT_TRX_CNF *)msg;

    AT_PR_LOGH("AT_ProcRxCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcRxCnf: ulIndex err!");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SET_FRXON) {
        AT_ERR_LOG("AT_ProcRxCnf: Not Set FRXON QRY !");
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (cnf->ulErrCode == MT_OK) {
        atMtInfoCtx->atInfo.rxOnOff = atMtInfoCtx->atInfo.tempRxorTxOnOff;
        g_atSendDataBuff.bufLen     = 0;
        At_FormatResultData(indexNum, AT_OK);
    } else {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_FRXON_SET_FAIL);
    }

    return VOS_OK;
}

VOS_UINT32 AT_RcvBbicCalMipiRedCnf(struct MsgCB *msg)
{
    BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU *redCnf      = VOS_NULL_PTR;
    AT_MT_Info                           *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                            rslt = AT_OK;
    VOS_UINT32                            byteCnt;
    VOS_UINT16                            length = 0;
    VOS_UINT8                             indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 获取本地保存的用户索引 */
    indexNum = atMtInfoCtx->atInfo.indexNum;
    redCnf   = (BBIC_CAL_RF_DEBUG_READ_MIPI_IND_STRU *)msg;

    AT_PR_LOGH("AT_RcvBbicCalMipiRedCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_RcvBbicCalMipiRedCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MIPIOPERATE_SET) {
        AT_ERR_LOG("AT_RcvBbicCalMipiRedCnf: CmdCurrentOpt is not AT_CMD_MIPIOPERATE_SET!");
        return VOS_ERR;
    }

    if (redCnf->uwResult == MT_OK) {
        rslt    = AT_OK;
        byteCnt = redCnf->stPara.astData[0].stCmd.bitByteCnt;

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
            redCnf->stPara.astData[0].stData.bitByte0);

        if (byteCnt > 1) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN - length, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", redCnf->stPara.astData[0].stData.bitByte1);
        }

        if (byteCnt > AT_CAL_MIPIDEV_BYTE_CNT_VALUE_2) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN - length, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", redCnf->stPara.astData[0].stData.bitByte2);
        }

        if (byteCnt > AT_CAL_MIPIDEV_BYTE_CNT_VALUE_3) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN - length, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", redCnf->stPara.astData[0].stData.bitByte3);
        }

        g_atSendDataBuff.bufLen = length;
    } else {
        AT_INFO_LOG("AT_RcvBbicCalMipiRedCnfMsg: read mipi err");
        rslt = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 AT_RcvBbicCalMipiWriteCnf(struct MsgCB *msg)
{
    BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU *writeCnf    = VOS_NULL_PTR;
    AT_MT_Info                            *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                             rslt = AT_OK;
    VOS_UINT8                              indexNum;

    /* 获取本地保存的用户索引 */
    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    writeCnf    = (BBIC_CAL_RF_DEBUG_WRITE_MIPI_IND_STRU *)msg;

    AT_PR_LOGH("AT_RcvBbicCalMipiWriteCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_RcvBbicCalMipiWriteCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_MIPIOPERATE_SET) {
        AT_ERR_LOG("AT_RcvBbicCalMipiWriteCnf: CmdCurrentOpt is not AT_CMD_MIPIOPERATE_SET!");
        return VOS_ERR;
    }

    if (writeCnf->uwResult == MT_OK) {
        rslt = AT_OK;
    } else {
        rslt = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 AT_ProcTxCnf(struct MsgCB *msg)
{
    BBIC_CAL_AT_MT_TRX_CNF *cnf         = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT8               indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    cnf         = (BBIC_CAL_AT_MT_TRX_CNF *)msg;

    AT_PR_LOGH("AT_ProcTxCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcTxCnf: ulIndex err!");
        return VOS_ERR;
    }

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SET_FTXON) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPOWDET_QRY)) {
        AT_ERR_LOG("AT_ProcTxCnf: Not Set FTXON,FPOWDET QRY !");
        return VOS_ERR;
    }

    /* Power Det是借助TXON的消息发送的，如果是查询power det，除了回这条消息，还要回复 ID_BBIC_TOOL_CAL_RF_DEBUG_TX_RESULT_IND */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_FPOWDET_QRY) {
        if (cnf->ulErrCode != MT_OK) {
            /* 复位AT状态 */
            AT_StopTimerCmdReady(indexNum);
            g_atSendDataBuff.bufLen = 0;
            At_FormatResultData(indexNum, AT_ERROR);
        }

        return VOS_OK;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (cnf->ulErrCode == MT_OK) {
        atMtInfoCtx->atInfo.txOnOff = atMtInfoCtx->atInfo.tempRxorTxOnOff;
        g_atSendDataBuff.bufLen     = 0;
        At_FormatResultData(indexNum, AT_OK);
    } else {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_FTXON_SET_FAIL);
    }

    return VOS_OK;
}

VOS_UINT32 AT_ProcPowerDetCnf(struct MsgCB *msg)
{
    BBIC_CAL_AT_MT_TX_PD_IND *pdResultInd = VOS_NULL_PTR;
    AT_MT_Info               *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT16                length;
    VOS_UINT8                 indexNum;
    VOS_INT32                 powerValue;

    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    length      = 0;
    pdResultInd = (BBIC_CAL_AT_MT_TX_PD_IND *)msg;

    AT_PR_LOGH("AT_ProcPowerDetCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcPowerDetCnf: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPOWDET_QRY) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (pdResultInd->mtTxIndPara.errorCode != MT_OK) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    if ((atMtInfoCtx->bbicInfo.currtRatMode == RAT_MODE_LTE) || (atMtInfoCtx->bbicInfo.currtRatMode == RAT_MODE_NR)) {
        /* LTE和NR DSP上报精度为0.125 */
        powerValue = (VOS_INT32)pdResultInd->mtTxIndPara.antPower * 10 / 8; /* 上报精度0.1dB */
    } else {
        /* GUC上报精度为0.1 */
        powerValue = pdResultInd->mtTxIndPara.antPower;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^FPOWDET:%d", powerValue);

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_ProcPowerDetCnf_ModulateWave(struct MsgCB *msg)
{
    BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU *pdResultInd = VOS_NULL_PTR;
    AT_MT_Info                         *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                          totalNum;
    VOS_UINT32                          j;
    VOS_UINT16                          length;
    VOS_UINT8                           indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    length      = 0;
    pdResultInd = (BBIC_CAL_RF_DEBUG_GTX_MRX_IND_STRU *)msg;

    AT_PR_LOGH("AT_ProcPowerDetCnf_ModulateWave Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcPowerDetCnf_ModulateWave: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPOWDET_QRY) {
        return VOS_ERR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (pdResultInd->stPara.uwErrorCode != MT_OK) {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_ERROR);
        return VOS_OK;
    }

    totalNum = atMtInfoCtx->atInfo.gsmTxSlotType; /* 需要读取几个数组 */
    if (atMtInfoCtx->atInfo.gsmTxSlotType == AT_GSM_TX_8_SLOT) {
        totalNum = BBIC_CAL_GSM_TX_SLOT_NUM;
    }

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "^FPOWDET:");

    for (j = 0; j < totalNum; j++) {
        /* GUC上报精度为0.1 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", pdResultInd->stPara.astSlotValue[j].antPower);
    }

    /* 将最后一个逗号去掉 */
    length = length - 1;

    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);

    return VOS_OK;
}

VOS_UINT32 AT_ProcGsmTxCnf_ModulateWave(struct MsgCB *msg)
{
    BBIC_CAL_MSG_CNF_STRU *cnf         = VOS_NULL_PTR;
    AT_MT_Info            *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT8              indexNum;

    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    cnf         = (BBIC_CAL_MSG_CNF_STRU *)msg;

    AT_PR_LOGH("AT_ProcGsmTxCnf_ModulateWave Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcGsmTxCnf_ModulateWave: ulIndex err!");
        return VOS_ERR;
    }

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SET_FTXON) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPOWDET_QRY)) {
        AT_ERR_LOG("AT_ProcGsmTxCnf_ModulateWave: Not Set FTXON,FPOWDET QRY !");
        return VOS_ERR;
    }

    /* Power Det是借助TXON的消息发送的，如果是查询power det，除了回这条消息，还要回复 ID_BBIC_TOOL_CAL_RF_DEBUG_TX_RESULT_IND */
    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_FPOWDET_QRY) {
        if (cnf->stPara.uwErrorCode != MT_OK) {
            /* 复位AT状态 */
            AT_StopTimerCmdReady(indexNum);
            g_atSendDataBuff.bufLen = 0;
            At_FormatResultData(indexNum, AT_ERROR);
            return VOS_OK;
        }

        return VOS_OK;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    if (cnf->stPara.uwErrorCode == MT_OK) {
        atMtInfoCtx->atInfo.txOnOff = atMtInfoCtx->atInfo.tempRxorTxOnOff;
        g_atSendDataBuff.bufLen     = 0;
        At_FormatResultData(indexNum, AT_OK);
    } else {
        g_atSendDataBuff.bufLen = 0;
        At_FormatResultData(indexNum, AT_FTXON_SET_FAIL);
    }

    return VOS_OK;
}

VOS_UINT32 At_RcvBbicRssiInd(struct MsgCB *msg)
{
    BBIC_CAL_AT_MT_RX_RSSI_IND *rssiIndMsg  = VOS_NULL_PTR;
    AT_MT_Info                 *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                  rslt = AT_OK;
    VOS_INT32                   rssi = 0;
    VOS_UINT16                  length = 0;
    VOS_UINT8                   indexNum;

    /* 获取本地保存的用户索引 */
    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    rssiIndMsg  = (BBIC_CAL_AT_MT_RX_RSSI_IND *)msg;

    AT_PR_LOGH("At_RcvBbicRssiInd Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("At_RcvBbicRssiInd: ulIndex err !");
        return VOS_ERR;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_QUERY_RSSI) {
        AT_ERR_LOG("At_RcvBbicRssiInd: CmdCurrentOpt Not Query Rssi !");
        return VOS_ERR;
    }

    /* MIMO场景，1，3，5，7取ashwRssi[0]， 2，4，6，8取ashwRssi[1]，余2是为了区分奇偶 */
    if ((atMtInfoCtx->bbicInfo.rxAntType == AT_ANT_TYPE_PRI) ||
        ((atMtInfoCtx->atInfo.rxMimoAntNum % 2 == 1) && (atMtInfoCtx->bbicInfo.rxAntType == AT_ANT_TYPE_MIMO))) {
        /* 由于RSSI测量值单位0.125dBm，为了消除浮点数*1000. */
        rssi = (VOS_INT32)rssiIndMsg->mtRxIndPara.rssi[0] * AT_DSP_RSSI_VALUE_MUL_THOUSAND;
    } else {
        /* 由于RSSI测量值单位0.125dBm，为了消除浮点数*1000. */
        rssi = (VOS_INT32)rssiIndMsg->mtRxIndPara.rssi[1] * AT_DSP_RSSI_VALUE_MUL_THOUSAND;
    }

    if (rssiIndMsg->mtRxIndPara.errorCode != MT_OK) {
        AT_ERR_LOG("At_RcvBbicRssiIndProc err");
        rslt = AT_ERROR;
    } else {
        /*
         * 读取的RSSI值，采用正值上报，精确到0.01dBm定制值信息。如果当前的RSSI
         * 值为-85.1dBm，返回值为8510. 由于之前乘1000，所以精确到0.01dBm这里要除10
         */
        if (rssi < 0) {
            rssi = (-1 * rssi) / 100;
        } else {
            rssi = rssi / 100;
        }

        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName, rssi);

        g_atSendDataBuff.bufLen = length;
        rslt                    = AT_OK;
    }

    AT_StopTimerCmdReady(indexNum);

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}

VOS_UINT32 At_ProcUeCbtRfIcMemTestCnf(struct MsgCB *msg)
{
    UECBT_AT_RficMemTestCnf *rficTestResult = VOS_NULL_PTR;
    AT_MT_Info              *atMtInfoCtx    = VOS_NULL_PTR;

    AT_PR_LOGH("At_ProcUeCbtRfIcMemTestCnf Enter");

    atMtInfoCtx    = AT_GetMtInfoCtx();
    rficTestResult = (UECBT_AT_RficMemTestCnf *)msg;

    if (rficTestResult->errorCode == MT_OK) {
        atMtInfoCtx->atInfo.rficTestResult = AT_RFIC_MEM_TEST_PASS;
    } else {
        atMtInfoCtx->atInfo.rficTestResult = rficTestResult->errorCode;
    }

    return VOS_OK;
}

VOS_UINT32 At_ProcFessFlpmNasSerdesAgingCnf(struct MsgCB *msg)
{
    FESS_FLPM_NasAtCnf *serdesTestResult = VOS_NULL_PTR;
    AT_MT_Info         *atMtInfoCtx      = VOS_NULL_PTR;

    AT_PR_LOGH("At_ProcFessFlpmNasSerdesAgingCnf Enter");

    atMtInfoCtx      = AT_GetMtInfoCtx();
    serdesTestResult = (FESS_FLPM_NasAtCnf *)msg;

    if (serdesTestResult->result == MT_OK) {
        atMtInfoCtx->rserTestResult = (VOS_INT32)AT_SERDES_TEST_PASS;
    } else {
        atMtInfoCtx->rserTestResult = serdesTestResult->result;
    }

    return VOS_OK;
}

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
VOS_UINT32 AT_ProcFessFlpmNasSerdesSltCnf(struct MsgCB *msg)
{
    FESS_FLPM_NasAtCnf *sltTestResult = VOS_NULL_PTR;
    AT_MT_Info         *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32          rslt = AT_OK;
    VOS_UINT8           indexNum;

    /* 获取本地保存的用户索引 */
    atMtInfoCtx   = AT_GetMtInfoCtx();
    indexNum      = atMtInfoCtx->atInfo.indexNum;
    sltTestResult = (FESS_FLPM_NasAtCnf *)msg;

    AT_PR_LOGH("AT_ProcFessFlpmNasSerdesSltCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcFessFlpmNasSerdesSltCnf: ulIndex err !");
        return VOS_ERR;
    }
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_SLT_TEST_SET) {
        return VOS_ERR;
    }
    if (sltTestResult->result == MT_OK) {
        rslt = AT_OK;
    } else {
        rslt = AT_ERROR;
    }

    AT_StopTimerCmdReady(indexNum);

    /* 调用At_FormatResultData发送命令结果 */
    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}
#endif

VOS_UINT32 At_ProcFessFlpmNasSerdesNobizCnf(struct MsgCB *rcvMsg)
{
    FESS_FLPM_NasAtCnf *serdesTestResult = VOS_NULL_PTR;
    AT_MT_Info           *atMtInfoCtx      = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    AT_PR_LOGH("At_ProcFessFlpmNasSerdesNobizCnf Enter");

    serdesTestResult = (FESS_FLPM_NasAtCnf *)rcvMsg;
    atMtInfoCtx->serdesTestAsyncInfo.result = serdesTestResult->result;

    /* 主动上报 */
    if (atMtInfoCtx->serdesTestAsyncInfo.rptFlg == 1) {
        g_atSendDataBuff.bufLen = 0;
        g_atSendDataBuff.bufLen += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s%s: %d%s", g_atCrLf, "^SERDESTESTASYNC",
            atMtInfoCtx->serdesTestAsyncInfo.result, g_atCrLf);

        At_SendResultData(AT_BROADCAST_CLIENT_INDEX_MODEM_0, g_atSndCodeAddress, g_atSendDataBuff.bufLen);
    }

    return VOS_OK;
}

VOS_UINT32 At_RcvBbicPllStatusCnf(struct MsgCB *msg)
{
    BBIC_CAL_PLL_QRY_IND_STRU *qryCnf      = VOS_NULL_PTR;
    AT_MT_Info                *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                 rslt = AT_OK;
    VOS_UINT32                 pllStatus;
    VOS_UINT16                 length = 0;
    VOS_UINT8                  indexNum;

    /* 获取本地保存的用户索引 */
    atMtInfoCtx = AT_GetMtInfoCtx();
    indexNum    = atMtInfoCtx->atInfo.indexNum;
    qryCnf      = (BBIC_CAL_PLL_QRY_IND_STRU *)msg;

    AT_PR_LOGH("At_RcvBbicPllStatusCnf Enter");

    if (indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("At_RcvBbicPllStatusCnf: ulIndex err !");
        return VOS_ERR;
    }

    if ((g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPLLSTATUS_QRY) &&
        (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_FPLLSTATUS_SET)) {
        AT_ERR_LOG("At_RcvBbicPllStatusCnf: CmdCurrentOpt err !");
        return VOS_ERR;
    }

    if (qryCnf->stPara.uwErrorCode != MT_OK) {
        rslt = AT_ERROR;
    } else {
        rslt = AT_OK;
        if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_FPLLSTATUS_QRY) {
            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                qryCnf->stPara.uwTxPllStatus, qryCnf->stPara.uwRxPllStatus);
        } else {
            pllStatus = (atMtInfoCtx->atInfo.antType == AT_MT_ANT_TYPE_TX) ? (qryCnf->stPara.uwTxPllStatus) :
                                                                             qryCnf->stPara.uwRxPllStatus;

            length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName, pllStatus);
        }
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);

    g_atSendDataBuff.bufLen = length;

    At_FormatResultData(indexNum, rslt);

    return VOS_OK;
}
#endif

#if (FEATURE_LTE == FEATURE_ON)
VOS_UINT32 AT_RcvMtaLteCaCellExQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg           *mtaMsg = (AT_MTA_Msg *)msg;
    MTA_AT_CaCellInfoCnf *cnf    = (MTA_AT_CaCellInfoCnf *)mtaMsg->content;
    VOS_UINT32            result = AT_OK;
    VOS_UINT32            i;
    g_atSendDataBuff.bufLen = 0;

    if (cnf->result != MTA_AT_RESULT_NO_ERROR) {
        result = AT_ERROR;
    } else {
        cnf->totalCellNum = (cnf->totalCellNum < MTA_AT_CA_MAX_CELL_NUM) ? cnf->totalCellNum : MTA_AT_CA_MAX_CELL_NUM;

        for (i = 0; i < cnf->totalCellNum; i++) {
            g_atSendDataBuff.bufLen += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN,
                (VOS_CHAR *)g_atSndCodeAddress, (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen,
                "%s: %d,%d,%d,%d,%d,%d,%d,%d", g_parseContext[indexNum].cmdElement->cmdName,
                cnf->cellInfo[i].cellIndex, cnf->cellInfo[i].ulConfigured, cnf->cellInfo[i].dlConfigured,
                cnf->cellInfo[i].actived, cnf->cellInfo[i].laaScellFlg, cnf->cellInfo[i].bandInd,
                cnf->cellInfo[i].bandWidth, cnf->cellInfo[i].earfcn);
            if (i != cnf->totalCellNum - 1) {
                g_atSendDataBuff.bufLen +=
                    (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                        (VOS_CHAR *)g_atSndCodeAddress + g_atSendDataBuff.bufLen, "%s", g_atCrLf);
            }
        }
    }

    return result;
}

VOS_UINT32 AT_RcvMtaCACellQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg          *rcvMsg = VOS_NULL_PTR;
    MTA_AT_CaCellQryCnf *mtaCnf = VOS_NULL_PTR;
    VOS_UINT32           result;

    /* 初始化 */
    rcvMsg   = (AT_MTA_Msg *)msg;
    mtaCnf   = (MTA_AT_CaCellQryCnf *)rcvMsg->content;
    result   = AT_OK;
    g_atSendDataBuff.bufLen = 0;

    if (mtaCnf->result != VOS_OK) {
        result = AT_ERROR;
    } else {
        g_atSendDataBuff.bufLen +=
            (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
                mtaCnf->blEnableType);
    }

    return result;
}
#endif

/*
 * 功能描述: ^LCACELL命令应答处理
 * 修改历史:
 */
VOS_UINT32 atLcacellCnfProc(struct MsgCB *msgBlock)
{
    L4A_READ_LcacellCnf *lcacell = NULL;
    VOS_UINT16           length  = 0;
    VOS_UINT32           statCnt = 0;

    lcacell = (L4A_READ_LcacellCnf *)msgBlock;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^LCACELL: ");

    for (statCnt = 0; statCnt < CA_MAX_CELL_NUM; statCnt++) {
        length +=
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
                (VOS_CHAR *)g_atSndCodeAddr + length, "\"%d %d %d %d\",", statCnt,
                lcacell->lcacellInfo[statCnt].ulConfigured, lcacell->lcacellInfo[statCnt].dlConfigured,
                lcacell->lcacellInfo[statCnt].actived);
    }

    length--;

    CmdErrProc((VOS_UINT8)(lcacell->clientId), lcacell->errorCode, length, g_atSndCodeAddr);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetRadverCnfProc(struct MsgCB *msgBlock)
{
    L4A_SET_RadverCnf *cnf = NULL;

    cnf = (L4A_SET_RadverCnf *)msgBlock;

    CmdErrProc((VOS_UINT8)(cnf->clientId), cnf->errorCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 AT_RcvMtaSetMipiReadCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg         *rcvMsg         = VOS_NULL_PTR;
    MTA_AT_MipiReadCnf *setMipiReadCnf = VOS_NULL_PTR;
    VOS_UINT16          length;

    /* 初始化 */
    rcvMsg         = (AT_MTA_Msg *)msg;
    setMipiReadCnf = (MTA_AT_MipiReadCnf *)rcvMsg->content;

    /* 判断回复消息中的错误码 */
    if (setMipiReadCnf->result == MTA_AT_RESULT_NO_ERROR) {
        /* 命令结果 *AT_OK */
        length = (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress, "%s:%d", g_parseContext[indexNum].cmdElement->cmdName,
            setMipiReadCnf->value);

        g_atSendDataBuff.bufLen = length;
        return AT_OK;
    } else {
        /* 命令结果 *AT_ERROR */
        g_atSendDataBuff.bufLen = 0;
        return AT_ERROR;
    }
}

VOS_UINT32 atQryFRSSIParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG        *event = NULL;
    FTM_FRSSI_Cnf *cnf   = NULL;
    VOS_UINT16     length;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_FRSSI_Cnf *)event->param1;

    if (cnf == NULL) {
        return ERR_MSP_FAILURE;
    }

    length = 0;

    /* 适配V7R5版本4RX接收，GU只报一个值，其他报0，L根据FTM上报结果，支持4RX接收上报4个值，不支持时上报1个值 */
#if (FEATURE_LTE_4RX == FEATURE_ON)
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FRSSI:%d,%d,%d,%d", cnf->lValue1, cnf->lValue2, cnf->lValue3,
        cnf->lValue4);
#elif (FEATURE_LTE_8RX == FEATURE_ON)
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FRSSI:%d", cnf->value1);
#else
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FRSSI:%d", cnf->value1);
#endif

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atQryFPllStatusParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    FTM_RdFpllstatusCnf *cnf   = NULL;
    OS_MSG              *event = NULL;
    VOS_UINT16           length;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdFpllstatusCnf *)event->param1;

    if (cnf == NULL) {
        return ERR_MSP_FAILURE;
    }

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FPLLSTATUS: %d,%d", cnf->txStatus, cnf->rxStatus);

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atSetFRXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    OS_MSG         *event = NULL;
    FTM_SetRxonCnf *cnf   = NULL;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_SetRxonCnf *)event->param1;

    CmdErrProc(clientId, cnf->errCode, 0, NULL);

    return AT_FW_CLIENT_STATUS_READY;
}

VOS_UINT32 atQryFRXONParaCnfProc(VOS_UINT8 clientId, struct MsgCB *msgBlock)
{
    FTM_RdRxonCnf *cnf   = NULL;
    OS_MSG        *event = NULL;
    VOS_UINT16     length;

    event = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    cnf   = (FTM_RdRxonCnf *)event->param1;

    if (cnf == NULL) {
        return ERR_MSP_FAILURE;
    }

    length = 0;
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddr,
        (VOS_CHAR *)g_atSndCodeAddr + length, "^FRXON:%d", cnf->rxSwt);

    CmdErrProc(clientId, cnf->errCode, length, g_atSndCodeAddr);
    return AT_FW_CLIENT_STATUS_READY;
}

