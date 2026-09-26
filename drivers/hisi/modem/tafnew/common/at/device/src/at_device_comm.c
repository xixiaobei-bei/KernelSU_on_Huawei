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
#include "at_device_comm.h"
#include "securec.h"
#include "AtParse.h"
#include "at_event_report.h"
#include "at_type_def.h"
#include "at_mt_msg_proc.h"
#include "at_lte_common.h"
#include "osm.h"
#include "at_input_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_COMM_C

#define AT_CRC32_TABLE_MAX_LEN 256
#define AT_CALC_CRC32_MAX_NUM 4

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
const AT_G_BandInfo g_atGBandInfoTable[] = {
    { 2, 3, 80000, { 512, 810 }}, /* GSM1900 */
    { 3, 2, 95000, { 512, 885 }}, /* GSM1800 */
    { 5, 0, 45000, { 128, 251 }}, /* GSM850 */
    { 8, 1, 45000, { 0, 1023 }},  /* GSM900 */
};

const VOS_UINT8 g_atGBandInfoTableLen = AT_ARRAY_SIZE(g_atGBandInfoTable);
#endif

VOS_UCHAR ToBase64Char(const VOS_UCHAR code6bit)
{
    VOS_UCHAR value;

    /* [ 0..25] => ['A'..'Z'] */
    if (code6bit < 26) {
        value = (VOS_UCHAR)(code6bit + 'A');
        return value;
    }

    /* [26..51] => ['a'..'z'] */
    else if (code6bit < 52) {
        value = (VOS_UCHAR)(code6bit + ('a' - 26));
        return value;
    }

    /* [52..61] => ['0'..'9'] */
    else if (code6bit < 62) {
        value = (VOS_UCHAR)(code6bit - 52 + '0');
        return value;
    }

    /* 62 => '+' */
    else if (code6bit == 62) {
        value = '+';
        return value;
    }

    /* 63 => */
    else {
        value = '/';
        return value;
    }
}

VOS_VOID AtBase64Encode(const VOS_UINT8 *pdata, const VOS_UINT32 dataSize, VOS_UINT8 *outPcode)
{
    const VOS_UCHAR *input     = (const unsigned char *)pdata;
    const VOS_UCHAR *inputEnd  = &input[dataSize];
    VOS_UCHAR       *output    = (unsigned char *)outPcode;
    VOS_UINT32       bordWidth = 0;
    /* input一次循环三个，output一次循环4个 */
    for (; input + AT_BORD_MAX_WIDTH < inputEnd; input += 3, output += 4) {
        output[AT_OUTPUT_INDEX_0] = ToBase64Char(input[AT_INPUT_INDEX_0] >> 2);
        output[AT_OUTPUT_INDEX_1] = ToBase64Char(((input[AT_INPUT_INDEX_0] << 4) | (input[AT_INPUT_INDEX_1] >> 4)) &
                                                 0x3F);
        output[AT_OUTPUT_INDEX_2] = ToBase64Char(((input[AT_INPUT_INDEX_1] << 2) | (input[AT_INPUT_INDEX_2] >> 6)) &
                                                 0x3F);
        output[AT_OUTPUT_INDEX_3] = ToBase64Char(input[AT_INPUT_INDEX_2] & 0x3F);
    }

    if (inputEnd > input) {
        bordWidth = (VOS_UINT32)(inputEnd - input);
        if (bordWidth == AT_BORD_MIN_WIDTH) {
            output[AT_OUTPUT_INDEX_0] = ToBase64Char(input[AT_INPUT_INDEX_0] >> 2);
            output[AT_OUTPUT_INDEX_1] = ToBase64Char((input[AT_INPUT_INDEX_0] << 4) & 0x3F);
            output[AT_OUTPUT_INDEX_2] = AT_BASE64_PADDING;
            output[AT_OUTPUT_INDEX_3] = AT_BASE64_PADDING;
        } else if (bordWidth == AT_BORD_MAX_WIDTH) {
            output[AT_OUTPUT_INDEX_0] = ToBase64Char(input[AT_INPUT_INDEX_0] >> 2);
            output[AT_OUTPUT_INDEX_1] =
                ToBase64Char(((input[AT_INPUT_INDEX_0] << 4) | (input[AT_INPUT_INDEX_1] >> 4)) & 0x3F);
            output[AT_OUTPUT_INDEX_2] = ToBase64Char((input[AT_INPUT_INDEX_1] << 2) & 0x3F);
            output[AT_OUTPUT_INDEX_3] = AT_BASE64_PADDING;
        } else {
        }
    }
}

VOS_UINT32 At_SimlockPlmnNumToAscii(const VOS_UINT8 *plmnRange, VOS_UINT8 plmnRangeLen, VOS_UINT8 *asciiStr)
{
    VOS_UINT8 loop;
    VOS_UINT8 len;
    VOS_UINT8 bcdCode;

    if ((plmnRange == VOS_NULL_PTR) || (asciiStr == VOS_NULL_PTR)) {
        AT_WARN_LOG("At_SimlockPlmnNumToAscii: Parameter of the function is null.");
        return AT_FAILURE;
    }

    /* 整理号码字符串，去除无效的0XFF数据 */
    while (plmnRangeLen > 1) {
        if (plmnRange[plmnRangeLen - 1] == 0xFF) {
            plmnRangeLen--;
        } else {
            break;
        }
    }

    /*
     * 判断pucPlmnRange所指向的字符串的最后一个字节的低位是否为1111，
     * 如果是，说明号码位数为奇数，否则为偶数
     */
    if ((plmnRange[plmnRangeLen - 1] & 0x0F) == 0x0F) {
        /* 号码位数为奇数 */
        len = (VOS_UINT8)((plmnRangeLen * 2) - 1);
    } else {
        /* 号码位数为偶数 */
        len = (VOS_UINT8)(plmnRangeLen * 2);
    }

    /* 解析号码 */
    for (loop = 0; loop < len; loop++) {
        /* 余2用于判断当前解码的是奇数位号码还是偶数位号码从0开始是偶数 */
        if ((loop % 2) == 0) {
            /* 如果是偶数位号码，则取高4位的值 */
            bcdCode = ((plmnRange[(loop / 2)] >> 4) & 0x0F);
        } else {
            /* 如果是奇数位号码，则取低4位的值 */
            bcdCode = (plmnRange[(loop / 2)] & 0x0F);
        }

        /* 将数字转换成Ascii码形式 */
        if (bcdCode <= AT_DEC_MAX_NUM) {
            asciiStr[loop] = (bcdCode + '0');
        } else {
            return AT_FAILURE;
        }
    }

    asciiStr[loop] = '\0'; /* 字符串末尾为0 */

    return AT_SUCCESS;
}

/* 计算字符串的CRC */
VOS_UINT32 Calc_CRC32(VOS_UINT8 *packet, VOS_UINT32 length)
{
    static VOS_UINT32 crc32Table[AT_CRC32_TABLE_MAX_LEN] = {
        0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005, 0x2608edb8,
        0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
        0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75, 0x6a1936c8, 0x6ed82b7f, 0x639b0da6,
        0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
        0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84,
        0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d, 0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
        0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a,
        0xec7dd02d, 0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
        0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca, 0x7897ab07,
        0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
        0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba, 0xaca5c697, 0xa864db20, 0xa527fdf9,
        0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
        0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b,
        0xf771768c, 0xfa325055, 0xfef34de2, 0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
        0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c,
        0x774bb0eb, 0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
        0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b, 0x0315d626,
        0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
        0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b, 0xd727bbb6, 0xd3e6a601, 0xdea580d8,
        0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
        0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a,
        0x8cf30bad, 0x81b02d74, 0x857130c3, 0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
        0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093,
        0x65c52d24, 0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
        0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654, 0xc5a92679,
        0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
        0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4, 0x89b8fd09, 0x8d79e0be, 0x803ac667,
        0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
        0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4,
    };

    VOS_UINT32 crc32 = 0;
    VOS_UINT32 i     = 0;

    for (i = 0; i < length; i++) {
        crc32 = ((crc32 << 8) | packet[i]) ^ (crc32Table[(crc32 >> 24) & 0xFF]);
    }

    for (i = 0; i < AT_CALC_CRC32_MAX_NUM; i++) {
        crc32 = ((crc32 << 8) | 0x00) ^ (crc32Table[(crc32 >> 24) & 0xFF]);
    }

    return crc32;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 AT_SetGlobalFchan(VOS_UINT8 rATMode)
{
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    atDevCmdCtrl                = AT_GetDevCmdCtrl();
    atDevCmdCtrl->deviceRatMode = rATMode;

    return VOS_OK;
}

#if (FEATURE_LTE == FEATURE_ON)
/*
 * 功能描述: 当前参数设置是否为LTE模
 */
VOS_UINT32 At_IsLteRatMode(VOS_UINT32 paraValue)
{
    if ((paraValue == AT_RAT_MODE_FDD_LTE) ||
#if (FEATURE_LTEV == FEATURE_ON)
        (paraValue == AT_RAT_MODE_LTEV) ||
#endif
        (paraValue == AT_RAT_MODE_TDD_LTE)) {

        return VOS_TRUE;
    }

    return VOS_FALSE;
}
#endif

#else

VOS_UINT32 At_CovertRatModeToBbicCal(AT_CmdRatmodeUint8 ratMode, RAT_MODE_ENUM_UINT16 *bbicMode)
{
    VOS_UINT32 result = VOS_TRUE;

    switch (ratMode) {
        case AT_CMD_RATMODE_GSM:
            *bbicMode = RAT_MODE_GSM;
            break;

        case AT_CMD_RATMODE_WCDMA:
            *bbicMode = RAT_MODE_WCDMA;
            break;

        case AT_CMD_RATMODE_LTE:
            *bbicMode = RAT_MODE_LTE;
            break;

        case AT_CMD_RATMODE_CDMA:
            *bbicMode = RAT_MODE_CDMA;
            break;

        case AT_CMD_RATMODE_NR:
            *bbicMode = RAT_MODE_NR;
            break;

        case AT_CMD_RATMODE_TD: /* 目前带NR的芯片不支持TD */
        default:
            *bbicMode = RAT_MODE_BUTT;
            result    = VOS_FALSE;
            break;
    }

    return result;
}

VOS_UINT32 AT_SndBbicCalSetDpdtReq(BBIC_DPDT_OPERTYPE_ENUM_UINT16 operType, VOS_UINT32 value, VOS_UINT32 workType)
{
    BBIC_CAL_DPDT_REQ_STRU *msg         = VOS_NULL_PTR;
    AT_MT_Info             *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32              length;

    atMtInfoCtx = AT_GetMtInfoCtx();
    /* 申请BBIC_CAL_DPDT_REQ_STRU消息 */
    length = sizeof(BBIC_CAL_DPDT_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    msg    = (BBIC_CAL_DPDT_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("At_SndBbicCalDpdtReq: alloc msg fail!");
        return AT_FAILURE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(msg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(msg, DSP_PID_BBA_CAL, ID_AT_BBIC_DPDT_REQ);

    msg->stPara.enRatMode        = atMtInfoCtx->bbicInfo.dpdtRatMode;
    msg->stPara.enOperType       = operType;
    msg->stPara.uwValue          = value;
    msg->stPara.uhwModemWorkType = (VOS_UINT16)workType;

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_ERR_LOG("At_SndBbicCalDpdtReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicCalSetDpdtReq Exit");

    return AT_SUCCESS;
}

VOS_VOID AT_SetUartTestState(AT_UartTestStateUint8 uartTestState)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx                            = AT_GetMtInfoCtx();
    atMtInfoCtx->cifTestInfo.uartTestState = uartTestState;
}

AT_UartTestStateUint8 AT_GetUartTestState(VOS_VOID)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();
    return atMtInfoCtx->cifTestInfo.uartTestState;
}

VOS_VOID AT_SetI2sTestState(AT_I2sTestStateUint8 i2sTestState)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx                           = AT_GetMtInfoCtx();
    atMtInfoCtx->cifTestInfo.i2sTestState = i2sTestState;
}

AT_I2sTestStateUint8 AT_GetI2sTestState(VOS_VOID)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();
    return atMtInfoCtx->cifTestInfo.i2sTestState;
}

VOS_UINT16 AT_GetGsmUlPathByBandNo(VOS_UINT16 bandNo)
{
    AT_MT_Info *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32  totalNum;
    VOS_UINT32  indexNum;
    VOS_UINT16  ulPath;

    atMtInfoCtx = AT_GetMtInfoCtx();
    ulPath      = 0;
    totalNum    = AT_GET_GSM_BAND_INFO_TAB_LEN();
    for (indexNum = 0; indexNum < totalNum; indexNum++) {
        if (g_atGBandInfoTable[indexNum].bandNo == atMtInfoCtx->bbicInfo.dspBandFreq.dspBand) {
            ulPath = g_atGBandInfoTable[indexNum].ulPath;
        }
    }

    return ulPath;
}

VOS_UINT32 At_SndGsmTxOnOffReq_ModulatedWave(VOS_UINT16 powerDetFlg)
{
    BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU *gsmTxReq    = VOS_NULL_PTR;
    AT_MT_Info                        *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                         totalNum;
    VOS_UINT32                         indexNum;
    MODU_TYPE_ENUM_UINT16              modType = MODU_TYPE_GMSK;

    atMtInfoCtx = AT_GetMtInfoCtx();
    /* 分配消息空间 */
    gsmTxReq = (BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT,
                                                                 sizeof(BBIC_CAL_RF_DEBUG_GSM_TX_REQ_STRU) -
                                                                 VOS_MSG_HEAD_LENGTH);
    if (gsmTxReq == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(gsmTxReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(gsmTxReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_RF_DEBUG_GSM_TX_REQ);

    gsmTxReq->stPara.uhwTxEnable                = atMtInfoCtx->atInfo.tempRxorTxOnOff;
    gsmTxReq->stPara.uhwMrxEanble               = powerDetFlg;
    gsmTxReq->stPara.stCommonInfo.enModemId     = MODEM_ID_0;
    gsmTxReq->stPara.stCommonInfo.enRatMode     = atMtInfoCtx->bbicInfo.currtRatMode;
    gsmTxReq->stPara.stCommonInfo.enBand        = atMtInfoCtx->bbicInfo.dspBandFreq.dspBand;
    gsmTxReq->stPara.stCommonInfo.enBandWith    = atMtInfoCtx->bbicInfo.bandWidth;
    gsmTxReq->stPara.stCommonInfo.uhwSignalType = BBIC_CAL_SIGNAL_TYPE_MODU;
    gsmTxReq->stPara.stCommonInfo.uhwUlPath     = AT_GetGsmUlPathByBandNo(atMtInfoCtx->bbicInfo.dspBandFreq.dspBand);
    gsmTxReq->stPara.stCommonInfo.enModType     = 0;

    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_GSM) {
        modType = MODU_TYPE_GMSK;
    }

    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_EDGE) {
        modType = MODU_TYPE_8PSK;
    }

    /* GSM TX 没有MIMO */
    gsmTxReq->stPara.stCommonInfo.uhwMimoType  = 0;
    gsmTxReq->stPara.stCommonInfo.uwTxFreqInfo = atMtInfoCtx->bbicInfo.dspBandFreq.ulFreq;

    gsmTxReq->stPara.uhwDataPattern = 1; /* 默认采用随机数 */
    gsmTxReq->stPara.enMrxEstMode   = MRX_ESTIMATE_POWER_MODE;
    gsmTxReq->stPara.uhwSlotCnt     = atMtInfoCtx->atInfo.gsmTxSlotType;

    totalNum = atMtInfoCtx->atInfo.gsmTxSlotType; /* 需要填写几个数组 */
    if (atMtInfoCtx->atInfo.gsmTxSlotType == AT_GSM_TX_8_SLOT) {
        totalNum                    = AT_GSM_TX_1_SLOT;
        gsmTxReq->stPara.uhwSlotCnt = AT_BBIC_CAL_MAX_GSM_SLOT; /* 8个slot */
    }

    for (indexNum = 0; indexNum < totalNum; indexNum++) {
        gsmTxReq->stPara.astSlotPara[indexNum].uhwModuType                          = modType;
        gsmTxReq->stPara.astSlotPara[indexNum].stTxPowerPara.enPowerCtrlMode        = POWER_CTRL_MODE_POWER;
        gsmTxReq->stPara.astSlotPara[indexNum].stTxPowerPara.stPaPara.ucTxPaMode    = atMtInfoCtx->bbicInfo.paLevel;
        gsmTxReq->stPara.astSlotPara[indexNum].stTxPowerPara.unPowerPara.shwTxPower = atMtInfoCtx->bbicInfo.fwavePower;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, gsmTxReq) != VOS_OK) {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndGsmTxOnOffReq_ModulatedWave Exit");

    return VOS_TRUE;
}

VOS_UINT32 At_SndTxOnOffReq(VOS_UINT16 powerDetFlg)
{
    AT_BBIC_CAL_MT_TX_REQ *txReq       = VOS_NULL_PTR;
    AT_MT_Info            *atMtInfoCtx = VOS_NULL_PTR;

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 分配消息空间 */
    txReq = (AT_BBIC_CAL_MT_TX_REQ *)PS_ALLOC_MSG(WUEPS_PID_AT, sizeof(AT_BBIC_CAL_MT_TX_REQ) - VOS_MSG_HEAD_LENGTH);
    if (txReq == VOS_NULL_PTR) {
        return VOS_FALSE;
    }

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(txReq);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(txReq, DSP_PID_BBA_CAL, ID_AT_BBIC_CAL_MT_TX_REQ);

    /* 填写消息内容 */
#if (FEATURE_LTEV == FEATURE_ON)
    if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_LTEV) {
        txReq->stComponentID.uwComponentType = AT_COMPONENTTYPE_RAT_LTEV;
    }
#endif
    txReq->stMtTxPara.ucTxEnable             = atMtInfoCtx->atInfo.tempRxorTxOnOff;
    txReq->stMtTxPara.ucTxPaMode             = atMtInfoCtx->bbicInfo.paLevel;
    txReq->stMtTxPara.enScsType              = atMtInfoCtx->bbicInfo.bbicScs;
    txReq->stMtTxPara.usMrxEanble            = powerDetFlg;
    txReq->stMtTxPara.enModemId              = MODEM_ID_0;
    txReq->stMtTxPara.enRatMode              = atMtInfoCtx->bbicInfo.currtRatMode;
    txReq->stMtTxPara.enBand                 = atMtInfoCtx->bbicInfo.dspBandFreq.dspBand;
    txReq->stMtTxPara.enBandWith             = atMtInfoCtx->bbicInfo.bandWidth;
    txReq->stMtTxPara.ulFreqInfo             = atMtInfoCtx->bbicInfo.dspBandFreq.ulFreq;
    txReq->stMtTxPara.enPowerCtrlMode        = POWER_CTRL_MODE_POWER;
    txReq->stMtTxPara.unPowerPara.shwTxPower = atMtInfoCtx->bbicInfo.fwavePower;
    txReq->stMtTxPara.rbCfgEnable            = atMtInfoCtx->atInfo.rbEnable;

    /* 单音还是调制 */
    if (atMtInfoCtx->bbicInfo.fwaveType == MODU_TYPE_BUTT) {
        txReq->stMtTxPara.ucIsSingleTone = VOS_TRUE;           /* 单音 */
        txReq->stMtTxPara.enModType      = MODU_TYPE_LTE_QPSK; /* 单音默认写QPSK */

        if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_GSM) {
            txReq->stMtTxPara.enModType = MODU_TYPE_GMSK;
        }

        if (atMtInfoCtx->atInfo.ratMode == AT_RAT_MODE_EDGE) {
            txReq->stMtTxPara.enModType = MODU_TYPE_8PSK;
        }
    } else {
        txReq->stMtTxPara.ucIsSingleTone = VOS_FALSE; /* 调制 */
        txReq->stMtTxPara.enModType      = atMtInfoCtx->bbicInfo.fwaveType;
    }

    /* MIMO类型 */
    if (atMtInfoCtx->bbicInfo.txAntType == AT_ANT_TYPE_MIMO) {
        txReq->stMtTxPara.usMimoType = atMtInfoCtx->bbicInfo.txMimoType;
        txReq->stMtTxPara.usAntMap   = atMtInfoCtx->bbicInfo.txMimoAntNum;
    } else {
        /* 默认是1 TX */
        txReq->stMtTxPara.usMimoType = AT_SET_BIT32(0);
        txReq->stMtTxPara.usAntMap   = AT_MIMO_ANT_NUM_1;
    }

    /* RB配置信息 */
    if (txReq->stMtTxPara.rbCfgEnable == VOS_TRUE) {
        txReq->stMtTxPara.rbNumCfg   = atMtInfoCtx->atInfo.rbNum;
        txReq->stMtTxPara.rbStartCfg = atMtInfoCtx->atInfo.rbStartCfg;
    }

    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, txReq) != VOS_OK) {
        return VOS_FALSE;
    }

    AT_PR_LOGH("At_SndTxOnOffReq Exit");

    return VOS_TRUE;
}

VOS_UINT32 AT_SndBbicPllStatusReq(VOS_VOID)
{
    BBIC_CAL_PLL_QRY_REQ_STRU *msg         = VOS_NULL_PTR;
    AT_MT_Info                *atMtInfoCtx = VOS_NULL_PTR;
    VOS_UINT32                 length;

    /* 申请BBIC_CAL_PLL_QRY_REQ_STRU消息 */
    length = sizeof(BBIC_CAL_PLL_QRY_REQ_STRU) - VOS_MSG_HEAD_LENGTH;
    msg    = (BBIC_CAL_PLL_QRY_REQ_STRU *)PS_ALLOC_MSG(WUEPS_PID_AT, length);

    if (msg == VOS_NULL_PTR) {
        AT_ERR_LOG("AT_SndBbicPllStatusReq: Alloc msg fail!");
        return AT_FAILURE;
    }

    atMtInfoCtx = AT_GetMtInfoCtx();

    /* 初始化 */
    AT_MT_CLR_MSG_ENTITY(msg);

    /* 填写消息头 */
    AT_CFG_MT_MSG_HDR(msg, DSP_PID_BBA_CAL, ID_AT_BBIC_PLL_QRY_REQ);

    msg->stPara.uhwBand = atMtInfoCtx->bbicInfo.dspBandFreq.dspBand;
    /* GSM  会用这个参数，但GSM不测调制模式 */
    msg->stPara.enModType = MODU_TYPE_BUTT;
    msg->stPara.enRatMode = atMtInfoCtx->bbicInfo.currtRatMode;

    /* 向对应PHY发送消息 */
    if (TAF_TraceAndSendMsg(WUEPS_PID_AT, msg) != VOS_OK) {
        AT_ERR_LOG("AT_SndBbicPllStatusReq: Send msg fail!");
        return AT_FAILURE;
    }

    AT_PR_LOGH("AT_SndBbicPllStatusReq Exit");

    return AT_SUCCESS;
}
#endif

/*
 * 功能描述  : 发送TMODE模式到TL C核
 */
VOS_UINT32 atSetTmodePara(VOS_UINT8 clientId, VOS_UINT32 tmode)
{
    SYM_SET_TmodeReq tmodeInfo;
    tmodeInfo.tmodeMode = (SYM_TMODE_ENUM)tmode;

    if (AT_SUCCESS == atSendFtmDataMsg(I0_MSP_SYS_FTM_PID, ID_MSG_SYM_SET_TMODE_REQ, clientId,
                                       (VOS_UINT8 *)(&tmodeInfo), sizeof(SYM_SET_TmodeReq))) {
        return AT_OK;
    } else {
        return AT_ERROR;
    }
}

VOS_UINT32 AT_TestHsicCmdPara(VOS_UINT8 indexNum)
{
    /* 通道检查 */
    if (AT_IsApPort(indexNum) == VOS_FALSE) {
        return AT_ERROR;
    }

    return AT_OK;
}

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
VOS_UINT32 At_ProcLteTxCltInfoReport(struct MsgCB *msgBlock)
{
    FTM_CltInfoInd    *txCltInfoInd = VOS_NULL_PTR;
    OS_MSG            *pstpOsMsg    = VOS_NULL_PTR;
    AT_DEVICE_CmdCtrl *atDevCmdCtrl = VOS_NULL_PTR;

    pstpOsMsg    = (OS_MSG *)(((MsgBlock *)msgBlock)->value);
    txCltInfoInd = (FTM_CltInfoInd *)pstpOsMsg->param1;
    atDevCmdCtrl = AT_GetDevCmdCtrl();

    /* 初始化全局变量 */
    (VOS_VOID)memset_s(&(atDevCmdCtrl->cltInfo), sizeof(atDevCmdCtrl->cltInfo), 0x0, sizeof(atDevCmdCtrl->cltInfo));

    /* 设置CLT信息有效标志 */
    atDevCmdCtrl->cltInfo.infoAvailableFlg = VOS_TRUE;

    /* 将接入层上报的信息记录下全局变量中 */
    /* 反射系数实部 */
    atDevCmdCtrl->cltInfo.gammaReal = txCltInfoInd->gammaReal;
    /* 反射系数虚部 */
    atDevCmdCtrl->cltInfo.gammaImag = txCltInfoInd->gammaImag;
    /* 驻波检测场景0反射系数幅度 */
    atDevCmdCtrl->cltInfo.gammaAmpUc0 = txCltInfoInd->gammaAmpUc0;
    /* 驻波检测场景1反射系数幅度 */
    atDevCmdCtrl->cltInfo.gammaAmpUc1 = txCltInfoInd->gammaAmpUc1;
    /* 驻波检测场景2反射系数幅度 */
    atDevCmdCtrl->cltInfo.gammaAmpUc2 = txCltInfoInd->gammaAmpUc2;
    /* 粗调格点位置 */
    atDevCmdCtrl->cltInfo.gammaAntCoarseTune = txCltInfoInd->gammaAntCoarseTune;
    /* 粗调FOM值 */
    atDevCmdCtrl->cltInfo.ulwFomcoarseTune = txCltInfoInd->ulwFomcoarseTune;
    /* 闭环算法收敛状态 */
    atDevCmdCtrl->cltInfo.cltAlgState = txCltInfoInd->cltAlgState;
    /* 闭环收敛总步数 */
    atDevCmdCtrl->cltInfo.cltDetectCount = txCltInfoInd->cltDetectCount;
    atDevCmdCtrl->cltInfo.dac0           = txCltInfoInd->dac0; /* DAC0 */
    atDevCmdCtrl->cltInfo.dac1           = txCltInfoInd->dac1; /* DAC1 */
    atDevCmdCtrl->cltInfo.dac2           = txCltInfoInd->dac2; /* DAC2 */
    atDevCmdCtrl->cltInfo.dac3           = txCltInfoInd->dac3; /* DAC3 */

    return VOS_TRUE;
}
#endif


