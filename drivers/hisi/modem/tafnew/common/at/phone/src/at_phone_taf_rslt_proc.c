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
#include "at_phone_taf_rslt_proc.h"
#include "securec.h"
#include "at_cmd_proc.h"
#include "at_phone_comm.h"
#include "at_event_report.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_TAF_RSLT_PROC_C
#define AT_CMD_TIMEQRY_YEAR_DEFAULT_VALUE 90
#define AT_CMD_TIMEQRY_MONTH_DEFAULT_VALUE 1
#define AT_CMD_TIMEQRY_DAY_DEFAULT_VALUE 6
#define AT_CMD_TIMEQRY_HOUR_DEFAULT_VALUE 8

static const MCC_ZONE_INFO_STRU g_mccZoneInfoTbl[] = {
    { 0x202, +30 * 4 / 10, 0 },  /* Greece */
    { 0x204, +10 * 4 / 10, 0 },  /* Greece */
    { 0x206, +10 * 4 / 10, 0 },  /* Holland */
    { 0x208, +10 * 4 / 10, 0 },  /* France */
    { 0x212, +10 * 4 / 10, 0 },  /* Monaco */
    { 0x213, +10 * 4 / 10, 0 },  /* Andorra */
    { 0x214, +10 * 4 / 10, 0 },  /* Spain */
    { 0x216, +10 * 4 / 10, 0 },  /* Hungary */
    { 0x218, +10 * 4 / 10, 0 },  /* Bosnia and Herzegovina, the Republic of */
    { 0x219, +10 * 4 / 10, 0 },  /* Croatia */
    { 0x220, +10 * 4 / 10, 0 },  /* Montenegro / Monte Nige Lu / Serbia */
    { 0x222, +10 * 4 / 10, 0 },  /* Italy */
    { 0x226, +20 * 4 / 10, 0 },  /* Romania */
    { 0x228, +10 * 4 / 10, 0 },  /* Switzerland */
    { 0x230, +10 * 4 / 10, 0 },  /* Czech */
    { 0x231, +10 * 4 / 10, 0 },  /* Slovakia */
    { 0x232, +10 * 4 / 10, 0 },  /* Austria */
    { 0x234, +10 * 4 / 10, 0 },  /* Guernsey */
    { 0x238, +10 * 4 / 10, 0 },  /* Denmark */
    { 0x240, +10 * 4 / 10, 0 },  /* Sweden */
    { 0x242, +10 * 4 / 10, 0 },  /* Norway */
    { 0x244, +20 * 4 / 10, 0 },  /* Finland */
    { 0x246, +20 * 4 / 10, 0 },  /* Lithuania */
    { 0x247, +20 * 4 / 10, 0 },  /* Latvia */
    { 0x248, +20 * 4 / 10, 0 },  /* Estonia */
    { 0x250, +80 * 4 / 10, 0 },  /* Russia GMT+3 ~ GMT+12 */
    { 0x255, +20 * 4 / 10, 0 },  /* Ukraine */
    { 0x257, +20 * 4 / 10, 0 },  /* Belarus */
    { 0x259, +20 * 4 / 10, 0 },  /* Moldova */
    { 0x260, +10 * 4 / 10, 0 },  /* Poland */
    { 0x262, +10 * 4 / 10, 0 },  /* Germany */
    { 0x266, +10 * 4 / 10, 0 },  /* Gibraltar */
    { 0x268, +0 * 4 / 10, 0 },   /* Portugal */
    { 0x270, +10 * 4 / 10, 0 },  /* Luxembourg */
    { 0x272, +0 * 4 / 10, 0 },   /* Ireland  GMT+0 ~ GMT+1 */
    { 0x274, +0 * 4 / 10, 0 },   /* Iceland  */
    { 0x276, +10 * 4 / 10, 0 },  /* Albania */
    { 0x278, +10 * 4 / 10, 0 },  /* Malta */
    { 0x280, +20 * 4 / 10, 0 },  /* Cyprus */
    { 0x282, +40 * 4 / 10, 0 },  /* Georgia */
    { 0x283, +40 * 4 / 10, 0 },  /* Armenia */
    { 0x284, +20 * 4 / 10, 0 },  /* Bulgaria */
    { 0x286, +20 * 4 / 10, 0 },  /* Turkey */
    { 0x288, +0 * 4 / 10, 0 },   /* Faroe Islands */
    { 0x290, -20 * 4 / 10, 0 },  /* Greenland GMT+0 ~ GMT-4 */
    { 0x292, +10 * 4 / 10, 0 },  /* San Marino */
    { 0x293, +10 * 4 / 10, 0 },  /* Slovenia */
    { 0x294, +10 * 4 / 10, 0 },  /* Macedonia */
    { 0x295, +10 * 4 / 10, 0 },  /* Liechtenstein */
    { 0x302, -40 * 4 / 10, 0 },  /* Canada */
    { 0x308, -30 * 4 / 10, 0 },  /* Saint-Pierre and Miquelon */
    { 0x310, -80 * 4 / 10, 0 },  /* America */
    { 0x311, -80 * 4 / 10, 0 },  /* America */
    { 0x330, -40 * 4 / 10, 0 },  /* Puerto Rico */
    { 0x334, -70 * 4 / 10, 0 },  /* Mexico */
    { 0x338, -50 * 4 / 10, 0 },  /* Jamaica */
    { 0x340, -40 * 4 / 10, 0 },  /* Martinique */
    { 0x342, -40 * 4 / 10, 0 },  /* Barbados */
    { 0x344, -40 * 4 / 10, 0 },  /* Antigua and Barbuda */
    { 0x346, -50 * 4 / 10, 0 },  /* Cayman Islands */
    { 0x348, -40 * 4 / 10, 0 },  /* The British Virgin Islands */
    { 0x350, -30 * 4 / 10, 0 },  /* Bermuda */
    { 0x352, -40 * 4 / 10, 0 },  /* Grenada */
    { 0x354, -40 * 4 / 10, 0 },  /* Montserrat */
    { 0x356, -40 * 4 / 10, 0 },  /* Saint Kitts and Nevis */
    { 0x358, -40 * 4 / 10, 0 },  /* St. Lucia */
    { 0x360, -40 * 4 / 10, 0 },  /* Saint Vincent and the Grenadines */
    { 0x362, -40 * 4 / 10, 0 },  /* Netherlands Antilles */
    { 0x363, -40 * 4 / 10, 0 },  /* Aruba */
    { 0x364, -50 * 4 / 10, 0 },  /* Bahamas */
    { 0x365, -40 * 4 / 10, 0 },  /* Anguilla */
    { 0x366, -50 * 4 / 10, 0 },  /* Dominican */
    { 0x368, -50 * 4 / 10, 0 },  /* Cuba */
    { 0x370, -50 * 4 / 10, 0 },  /* Dominican Republic */
    { 0x372, -50 * 4 / 10, 0 },  /* Haiti */
    { 0x374, -40 * 4 / 10, 0 },  /* The Republic of Trinidad and Tobago */
    { 0x376, -50 * 4 / 10, 0 },  /* Turks and Caicos Islands */
    { 0x400, +40 * 4 / 10, 0 },  /* Republic of Azerbaijan */
    { 0x401, +40 * 4 / 10, 0 },  /* Kazakhstan */
    { 0x402, +60 * 4 / 10, 0 },  /* Bhutan */
    { 0x404, +55 * 4 / 10, 0 },  /* India */
    { 0x405, +55 * 4 / 10, 0 },  /* India */
    { 0x410, +50 * 4 / 10, 0 },  /* Pakistan */
    { 0x412, +45 * 4 / 10, 0 },  /* Afghanistan */
    { 0x413, +55 * 4 / 10, 0 },  /* Sri Lanka */
    { 0x414, +65 * 4 / 10, 0 },  /* Myanmar */
    { 0x415, +30 * 4 / 10, 0 },  /* Lebanon */
    { 0x416, +20 * 4 / 10, 0 },  /* Jordan */
    { 0x417, +20 * 4 / 10, 0 },  /* Syria */
    { 0x418, +30 * 4 / 10, 0 },  /* Iraq */
    { 0x419, +30 * 4 / 10, 0 },  /* Kuwait */
    { 0x420, +30 * 4 / 10, 0 },  /* Saudi Arabia */
    { 0x421, +30 * 4 / 10, 0 },  /* Yemen */
    { 0x422, +40 * 4 / 10, 0 },  /* Oman */
    { 0x424, +40 * 4 / 10, 0 },  /* United Arab Emirates */
    { 0x425, +20 * 4 / 10, 0 },  /* Israel */
    { 0x426, +30 * 4 / 10, 0 },  /* Bahrain  */
    { 0x427, +30 * 4 / 10, 0 },  /* Qatar */
    { 0x428, +80 * 4 / 10, 0 },  /* Mongolia */
    { 0x429, +58 * 4 / 10, 0 },  /* Nepal */
    { 0x432, +35 * 4 / 10, 0 },  /* Iran */
    { 0x434, +50 * 4 / 10, 0 },  /* Uzbekistan */
    { 0x436, +50 * 4 / 10, 0 },  /* Tajikistan */
    { 0x437, +60 * 4 / 10, 0 },  /* Kyrgyzstan */
    { 0x438, +50 * 4 / 10, 0 },  /* Turkmenistan */
    { 0x440, +90 * 4 / 10, 0 },  /* Japan */
    { 0x450, +90 * 4 / 10, 0 },  /* South Korea */
    { 0x452, +70 * 4 / 10, 0 },  /* Vietnam */
    { 0x454, +80 * 4 / 10, 0 },  /* Hong Kong */
    { 0x455, +80 * 4 / 10, 0 },  /* Macau */
    { 0x456, +70 * 4 / 10, 0 },  /* Cambodia */
    { 0x457, +70 * 4 / 10, 0 },  /* Laos */
    { 0x460, +80 * 4 / 10, 0 },  /* China */
    { 0x466, +80 * 4 / 10, 0 },  /* Taiwan */
    { 0x467, +90 * 4 / 10, 0 },  /* North Korea */
    { 0x470, +60 * 4 / 10, 0 },  /* Bangladesh */
    { 0x472, +50 * 4 / 10, 0 },  /* Maldives */
    { 0x502, +80 * 4 / 10, 0 },  /*   */
    { 0x505, +90 * 4 / 10, 0 },  /*   */
    { 0x510, +80 * 4 / 10, 0 },  /*   */
    { 0x514, +90 * 4 / 10, 0 },  /*   */
    { 0x515, +80 * 4 / 10, 0 },  /*   */
    { 0x520, +70 * 4 / 10, 0 },  /*   */
    { 0x525, +80 * 4 / 10, 0 },  /*   */
    { 0x528, +80 * 4 / 10, 0 },  /*   */
    { 0x530, +120 * 4 / 10, 0 }, /*   */
    { 0x537, +100 * 4 / 10, 0 }, /*   */
    { 0x539, +130 * 4 / 10, 0 }, /*   */
    { 0x540, +110 * 4 / 10, 0 }, /*   */
    { 0x541, +110 * 4 / 10, 0 }, /*   */
    { 0x542, +120 * 4 / 10, 0 }, /*   */
    { 0x544, -110 * 4 / 10, 0 }, /* American Samoa */
    { 0x545, +130 * 4 / 10, 0 }, /* GMT +13 - +15 */
    { 0x546, +110 * 4 / 10, 0 }, /*   */
    { 0x547, -100 * 4 / 10, 0 }, /*   */
    { 0x548, -100 * 4 / 10, 0 }, /*   */
    { 0x549, -110 * 4 / 10, 0 }, /*   */
    { 0x550, +100 * 4 / 10, 0 }, /*   */
    { 0x552, +90 * 4 / 10, 0 },  /*   */
    { 0x602, +20 * 4 / 10, 0 },  /*   */
    { 0x603, +10 * 4 / 10, 0 },  /*   */
    { 0x604, +0 * 4 / 10, 0 },   /*   */
    { 0x605, +10 * 4 / 10, 0 },  /*   */
    { 0x606, +20 * 4 / 10, 0 },  /*   */
    { 0x607, +0 * 4 / 10, 0 },   /*   */
    { 0x608, +0 * 4 / 10, 0 },   /*   */
    { 0x609, +0 * 4 / 10, 0 },   /*   */
    { 0x610, +0 * 4 / 10, 0 },   /*   */
    { 0x611, +0 * 4 / 10, 0 },   /*   */
    { 0x612, +0 * 4 / 10, 0 },   /*   */
    { 0x613, +0 * 4 / 10, 0 },   /*   */
    { 0x614, +10 * 4 / 10, 0 },  /*   */
    { 0x615, +0 * 4 / 10, 0 },   /*   */
    { 0x616, +10 * 4 / 10, 0 },  /*   */
    { 0x617, +40 * 4 / 10, 0 },  /*   */
    { 0x618, +0 * 4 / 10, 0 },   /*   */
    { 0x619, +0 * 4 / 10, 0 },   /*   */
    { 0x620, +0 * 4 / 10, 0 },   /*   */
    { 0x621, +10 * 4 / 10, 0 },  /*   */
    { 0x622, +10 * 4 / 10, 0 },  /*   */
    { 0x623, +10 * 4 / 10, 0 },  /*   */
    { 0x624, +10 * 4 / 10, 0 },  /*   */
    { 0x625, +10 * 4 / 10, 0 },  /*   */
    { 0x626, +0 * 4 / 10, 0 },   /*   */
    { 0x627, +10 * 4 / 10, 0 },  /*   */
    { 0x628, +10 * 4 / 10, 0 },  /*   */
    { 0x629, +10 * 4 / 10, 0 },  /*   */
    { 0x630, +20 * 4 / 10, 0 },  /*   */
    { 0x631, +10 * 4 / 10, 0 },  /*   */
    { 0x632, +0 * 4 / 10, 0 },   /*   */
    { 0x633, +40 * 4 / 10, 0 },  /*   */
    { 0x634, +30 * 4 / 10, 0 },  /*   */
    { 0x635, +20 * 4 / 10, 0 },  /*   */
    { 0x636, +30 * 4 / 10, 0 },  /* Ethiopia */
    { 0x637, +30 * 4 / 10, 0 },  /* Somalia */
    { 0x638, +30 * 4 / 10, 0 },  /* Djibouti */
    { 0x639, +30 * 4 / 10, 0 },  /* Kenya */
    { 0x640, +30 * 4 / 10, 0 },  /* Tanzania */
    { 0x641, +30 * 4 / 10, 0 },  /* Uganda */
    { 0x642, +20 * 4 / 10, 0 },  /* Burundi */
    { 0x643, +20 * 4 / 10, 0 },  /* Mozambique */
    { 0x645, +20 * 4 / 10, 0 },  /* Zambia */
    { 0x646, +30 * 4 / 10, 0 },  /* Madagascar */
    { 0x647, +0 * 4 / 10, 0 },   /* not sure */
    { 0x648, +20 * 4 / 10, 0 },  /* Zimbabwe */
    { 0x649, +10 * 4 / 10, 0 },  /* Namibia */
    { 0x650, +20 * 4 / 10, 0 },  /* Malawi */
    { 0x651, +20 * 4 / 10, 0 },  /* Lesotho */
    { 0x652, +20 * 4 / 10, 0 },  /* Botswana */
    { 0x653, +20 * 4 / 10, 0 },  /* Swaziland */
    { 0x654, +20 * 4 / 10, 0 },  /* Comoros */
    { 0x655, +20 * 4 / 10, 0 },  /* South Africa */
    { 0x659, +30 * 4 / 10, 0 },  /* sudan */
    { 0x702, -60 * 4 / 10, 0 },  /* Belize */
    { 0x704, -60 * 4 / 10, 0 },  /* Guatemala */
    { 0x706, -60 * 4 / 10, 0 },  /* Salvador */
    { 0x708, -60 * 4 / 10, 0 },  /* Honduras */
    { 0x710, -60 * 4 / 10, 0 },  /* Nicaragua */
    { 0x712, -60 * 4 / 10, 0 },  /* Costa Rica */
    { 0x714, -50 * 4 / 10, 0 },  /* Panama */
    { 0x716, -50 * 4 / 10, 0 },  /* Peru */
    { 0x722, -30 * 4 / 10, 0 },  /* Argentina */
    { 0x724, -30 * 4 / 10, 0 },  /* Brazil */
    { 0x730, -50 * 4 / 10, 0 },  /* Chile */
    { 0x732, -50 * 4 / 10, 0 },  /* Colombia */
    { 0x734, -45 * 4 / 10, 0 },  /* Venezuela */
    { 0x736, -40 * 4 / 10, 0 },  /* Bolivia */
    { 0x738, -30 * 4 / 10, 0 },  /* Guyana */
    { 0x740, -50 * 4 / 10, 0 },  /* Ecuador */
    { 0x744, -40 * 4 / 10, 0 },  /* Paraguay */
    { 0x746, -30 * 4 / 10, 0 },  /* Suriname */
    { 0x748, -30 * 4 / 10, 0 },  /* Uruguay */
};

VOS_UINT32 At_QryParaRspCregProc(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RegStateQryCnf *cnfMsg = (TAF_MMA_RegStateQryCnf *)msg;
    TAF_PH_RegState *creg = &(cnfMsg->regInfo);
    AT_ModemNetCtx *netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        (VOS_UINT32)netCtx->cregType);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", creg->regState);

    if ((netCtx->cregType == AT_CREG_RESULT_CODE_ENTIRE_TYPE) &&
        ((creg->regState == TAF_PH_REG_REGISTERED_HOME_NETWORK) || (creg->regState == TAF_PH_REG_REGISTERED_ROAM))) {
        length += (VOS_UINT16)At_PhReadCreg(creg, g_atSndCodeAddress + length);
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 At_QryParaRspCgregProc(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RegStateQryCnf *cnfMsg = (TAF_MMA_RegStateQryCnf *)msg;
    TAF_PH_RegState *cgreg = &(cnfMsg->regInfo);
    AT_ModemNetCtx *netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        (VOS_UINT32)netCtx->cgregType);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cgreg->psRegState);

    if ((netCtx->cgregType == AT_CGREG_RESULT_CODE_ENTIRE_TYPE) &&
        ((cgreg->psRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
         (cgreg->psRegState == TAF_PH_REG_REGISTERED_ROAM)) &&
        (cgreg->act != TAF_PH_ACCESS_TECH_NR_5GC)) {
        length += (VOS_UINT16)At_PhReadCreg(cgreg, g_atSndCodeAddress + length);
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

#if (FEATURE_LTE == FEATURE_ON)

VOS_UINT32 AT_QryParaRspCeregProc(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RegStateQryCnf *cnfMsg = (TAF_MMA_RegStateQryCnf *)msg;
    TAF_PH_RegState *cereg = &(cnfMsg->regInfo);
    AT_ModemNetCtx *netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        (VOS_UINT32)netCtx->ceregType);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", cereg->psRegState);

    /* GU下查询只上报stat */
    if ((netCtx->ceregType == AT_CEREG_RESULT_CODE_ENTIRE_TYPE) && (cereg->act == TAF_PH_ACCESS_TECH_E_UTRAN) &&
        ((cereg->psRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
         (cereg->psRegState == TAF_PH_REG_REGISTERED_ROAM))) {
        length += (VOS_UINT16)At_PhReadCreg(cereg, g_atSndCodeAddress + length);
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

#endif

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
VOS_UINT32 AT_QryParaRspC5gregProc(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_RegStateQryCnf *cnfMsg = (TAF_MMA_RegStateQryCnf *)msg;
    TAF_PH_RegState *c5greg = &(cnfMsg->regInfo);
    AT_ModemNetCtx *netCtx = AT_GetModemNetCtxAddrFromClientId(indexNum);
    VOS_CHAR acStrAllowedNssai[AT_EVT_MULTI_S_NSSAI_LEN];
    VOS_UINT32 lengthTemp;
    VOS_UINT16 length = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d", g_parseContext[indexNum].cmdElement->cmdName,
        (VOS_UINT32)netCtx->c5gregType);

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, ",%d", c5greg->psRegState);

    if ((netCtx->c5gregType == AT_C5GREG_RESULT_CODE_ENTIRE_TYPE) && (c5greg->act == TAF_PH_ACCESS_TECH_NR_5GC) &&
        ((c5greg->psRegState == TAF_PH_REG_REGISTERED_HOME_NETWORK) ||
         (c5greg->psRegState == TAF_PH_REG_REGISTERED_ROAM))) {
        length += (VOS_UINT16)At_PhReadC5greg(c5greg, g_atSndCodeAddress + length);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");

        /* 组装切片消息格式 */
        lengthTemp = 0;
        memset_s(acStrAllowedNssai, sizeof(acStrAllowedNssai), 0, sizeof(acStrAllowedNssai));

        AT_ConvertMultiSNssaiToString(AT_MIN(c5greg->allowedNssai.snssaiNum, PS_MAX_ALLOWED_S_NSSAI_NUM),
                                      &c5greg->allowedNssai.snssai[0], acStrAllowedNssai, sizeof(acStrAllowedNssai),
                                      &lengthTemp);

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%d,", lengthTemp);

        if (lengthTemp != 0) {
            length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress + length, "\"%s\"", acStrAllowedNssai);
        }
    }

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}
#endif

VOS_BOOL AT_IsPlmnValid(TAF_MMA_LastCampPlmnQryCnf *plmnCnf)
{
    /* 无效的PLMN */
    if ((plmnCnf->plmnId.mcc == 0) && (plmnCnf->plmnId.mnc == 0)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

VOS_UINT32 AT_GetTimeZoneByMcc(VOS_UINT16 plmnMcc, VOS_INT8 *pcMccZoneInfo)
{
    VOS_UINT32 totalMccNum;
    VOS_UINT32 i   = 0;
    VOS_UINT32 ret = VOS_FALSE;

    totalMccNum = AT_ARRAY_SIZE(g_mccZoneInfoTbl);

    for (i = 0; i < totalMccNum; i++) {
        if (plmnMcc == g_mccZoneInfoTbl[i].mcc) {
            *pcMccZoneInfo = g_mccZoneInfoTbl[i].zone;

            ret = VOS_TRUE;

            break;
        }
    }

    return ret;
}

VOS_UINT32 AT_PlmnTimeZoneProc(TAF_UINT32 mcc, NAS_MM_InfoInd *mmTimeInfo)
{
    VOS_UINT32 result = VOS_ERR;
    VOS_UINT16 mccTemp;
    VOS_INT8   timeZone = 0;

    /* Get MCC */
    mccTemp = (VOS_UINT16)mcc;

    /* ^TIMEQRY在没收到EMM INFO的时候根据MCC计算时区 */
    if (AT_GetTimeZoneByMcc(mccTemp, &timeZone) == VOS_TRUE) {
        /* 能走到这个分支说明之前A核没有拿到过MM INFO时间，因此填充一个默认值 */
        mmTimeInfo->ieFlg                                  = NAS_MM_INFO_IE_UTLTZ;
        mmTimeInfo->universalTimeandLocalTimeZone.year     = AT_CMD_TIMEQRY_YEAR_DEFAULT_VALUE;
        mmTimeInfo->universalTimeandLocalTimeZone.month    = AT_CMD_TIMEQRY_MONTH_DEFAULT_VALUE;
        mmTimeInfo->universalTimeandLocalTimeZone.day      = AT_CMD_TIMEQRY_DAY_DEFAULT_VALUE;
        mmTimeInfo->universalTimeandLocalTimeZone.hour     = AT_CMD_TIMEQRY_HOUR_DEFAULT_VALUE;
        mmTimeInfo->universalTimeandLocalTimeZone.timeZone = timeZone;
        mmTimeInfo->localTimeZone                          = timeZone;

        result = VOS_OK;
    } else {
        result = VOS_ERR;
    }

    return result;
}

VOS_UINT32 AT_RcvMmaPlmnQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_LastCampPlmnQryCnf *plmnCnf = (TAF_MMA_LastCampPlmnQryCnf *)msg;
    NAS_MM_InfoInd              mmTimeInfo;
    VOS_UINT16                  length = 0;

    (VOS_VOID)memset_s(&mmTimeInfo, sizeof(NAS_MM_InfoInd), 0x00, sizeof(NAS_MM_InfoInd));

    /* 无效的PLMN，直接上报OK */
    if (AT_IsPlmnValid(plmnCnf) != VOS_TRUE) {
        return AT_OK;
    }

    if (g_atClientTab[indexNum].cmdCurrentOpt == AT_CMD_PLMN_QRY) {
        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s^PLMN: ", g_atCrLf);

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
            (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x%x,", (plmnCnf->plmnId.mcc & 0x0f00) >> 8,
            (plmnCnf->plmnId.mcc & 0xf0) >> 4, (plmnCnf->plmnId.mcc & 0x0f));

        if ((plmnCnf->plmnId.mnc & 0x0f00) == 0x0f00) {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x", (plmnCnf->plmnId.mnc & 0xf0) >> 4,
                (plmnCnf->plmnId.mnc & 0x0f));
        } else {
            length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (TAF_CHAR *)g_atSndCodeAddress,
                (TAF_CHAR *)g_atSndCodeAddress + length, "%x%x%x", (plmnCnf->plmnId.mnc & 0x0f00) >> 8,
                (plmnCnf->plmnId.mnc & 0xf0) >> 4, (plmnCnf->plmnId.mnc & 0x0f));
        }

        length += (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%s", g_atCrLf);

    } else {
        if (AT_PlmnTimeZoneProc(plmnCnf->plmnId.mcc, &mmTimeInfo) == VOS_OK) {
            length = AT_PrintNwTimeInfo(&mmTimeInfo, (VOS_CHAR *)g_parseContext[indexNum].cmdElement->cmdName, 0);
        } else {
            return AT_ERROR;
        }
    }

    g_atSendDataBuff.bufLen = length;
    return AT_OK;
}

VOS_UINT32 AT_RcvTafCcmQryXlemaCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_CCM_QryXlemaCnf *qryXlemaCnf = VOS_NULL_PTR;
    VOS_UINT32           i;
    VOS_UINT16           length;

    qryXlemaCnf = (TAF_CCM_QryXlemaCnf *)msg;

    /* 向APP逐条上报紧急呼号码 */
    qryXlemaCnf->qryXlemaPara.eccNumCount = AT_MIN(qryXlemaCnf->qryXlemaPara.eccNumCount, MN_CALL_MAX_EMC_NUM);
    for (i = 0; i < qryXlemaCnf->qryXlemaPara.eccNumCount; i++) {

        length = AT_FormatOneXlemaPara(&(qryXlemaCnf->qryXlemaPara.customEccNumList[i]),
            qryXlemaCnf->qryXlemaPara.eccNumCount, i);

        At_SendResultData(indexNum, g_atSndCodeAddress, length);
    }

    return AT_OK;
}

VOS_UINT32 AT_RcvMmaPacspQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    TAF_MMA_PacspQryCnf *pacspQryCnf = (TAF_MMA_PacspQryCnf *)msg;

    g_atSendDataBuff.bufLen = (TAF_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress, "+PACSP%d", pacspQryCnf->plmnMode);

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaQryJamDetectCfgCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *mtaMsg = VOS_NULL_PTR;
    MTA_AT_QryJamDetectCnf *qryCnf = VOS_NULL_PTR;
    VOS_UINT16              length = 0;

    mtaMsg = (AT_MTA_Msg *)msg;
    qryCnf = (MTA_AT_QryJamDetectCnf *)mtaMsg->content;

    /* GSM制式，干扰检测配置参数 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d%s",
        g_parseContext[indexNum].cmdElement->cmdName, AT_MTA_CMD_RATMODE_GSM, qryCnf->gsmPara.threshold,
        qryCnf->gsmPara.freqNum, g_atCrLf);

    /* WCDMA制式，干扰检测配置参数 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d%s",
        g_parseContext[indexNum].cmdElement->cmdName, AT_MTA_CMD_RATMODE_WCDMA,
        qryCnf->wcdmaPara.rssiSrhThreshold, qryCnf->wcdmaPara.rssiSrhFreqPercent,
        qryCnf->wcdmaPara.pschSrhThreshold, qryCnf->wcdmaPara.pschSrhFreqPercent, g_atCrLf);

    /* LTE制式，干扰检测配置参数 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d,%d,%d,%d",
        g_parseContext[indexNum].cmdElement->cmdName, AT_MTA_CMD_RATMODE_LTE,
        qryCnf->ltePara.rssiThresh + AT_JAM_DETECT_LTE_RSSI_THRESHOLD_MAX, qryCnf->ltePara.rssiPercent,
        qryCnf->ltePara.pssratioThresh, qryCnf->ltePara.pssratioPercent);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_UINT32 AT_RcvMtaQryJamDetectSwitchCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg             *mtaMsg = VOS_NULL_PTR;
    MTA_AT_QryJamDetectCnf *qryCnf = VOS_NULL_PTR;
    VOS_UINT16              length = 0;

    mtaMsg = (AT_MTA_Msg *)msg;
    qryCnf = (MTA_AT_QryJamDetectCnf *)mtaMsg->content;

    /* GSM制式，干扰检测开关状态 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d%s", g_parseContext[indexNum].cmdElement->cmdName,
        qryCnf->gsmJamMode, AT_MTA_CMD_RATMODE_GSM, g_atCrLf);

    /* WCDMA制式，干扰检测开关状态 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d%s", g_parseContext[indexNum].cmdElement->cmdName,
        qryCnf->wcdmaJamMode, AT_MTA_CMD_RATMODE_WCDMA, g_atCrLf);

    /* LTE制式，干扰检测开关状态 */
    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: %d,%d", g_parseContext[indexNum].cmdElement->cmdName,
        qryCnf->lteJamMode, AT_MTA_CMD_RATMODE_LTE);

    g_atSendDataBuff.bufLen = length;

    return AT_OK;
}

VOS_VOID At_RcvVcMsgSetTTYModeCnfProc(MN_AT_IndEvt *data)
{
    VOS_UINT32 ret;
    VOS_UINT8 *result   = VOS_NULL_PTR;
    VOS_UINT8  indexNum = 0;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_TTYMODE_SET, &indexNum) != VOS_OK) {
        return;
    }

    result = data->content;
    if (*result == VOS_OK) {
        ret = AT_OK;
    } else {
        ret = AT_ERROR;
    }

    /* 输出设置结果 */
    g_atSendDataBuff.bufLen = 0;

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_VOID At_RcvVcMsgQryTTYModeCnfProc(MN_AT_IndEvt *data)
{
    APP_VC_QryTtymodeCnf *tTYMode = VOS_NULL_PTR;
    VOS_UINT32            ret;
    VOS_UINT8             indexNum = 0;

    /* 初始化 */
    tTYMode = (APP_VC_QryTtymodeCnf *)data->content;

    /* 通过clientid获取index */
    if (AT_GetIndexNumByClientId(data->clientId, AT_CMD_TTYMODE_READ, &indexNum) != VOS_OK) {
        return;
    }

    /* 查询的TTY MODE */
    if (tTYMode->qryRslt == VOS_OK) {
        g_atSendDataBuff.bufLen =
            (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
                (VOS_CHAR *)g_atSndCodeAddress, "%s: %u", g_parseContext[indexNum].cmdElement->cmdName,
                tTYMode->ttyMode);
        ret = AT_OK;
    } else {
        g_atSendDataBuff.bufLen = 0;
        ret                     = AT_ERROR;
    }

    /* 复位AT状态 */
    AT_StopTimerCmdReady(indexNum);
    At_FormatResultData(indexNum, ret);
}

VOS_UINT32 AT_RcvMtaCclkQryCnf(VOS_VOID *msg, VOS_UINT8 indexNum)
{
    AT_MTA_Msg        *rcvMsg = VOS_NULL_PTR;
    MTA_AT_CclkQryCnf *qryCnf = VOS_NULL_PTR;
    AT_ModemNetCtx    *netCtx = VOS_NULL_PTR;
    VOS_UINT32         result;
    ModemIdUint16      modemId;
    TIME_ZONE_Time     time;

    /* 初始化 */
    (VOS_VOID)memset_s(&time, sizeof(time), 0x00, sizeof(time));
    rcvMsg   = (AT_MTA_Msg *)msg;
    qryCnf   = (MTA_AT_CclkQryCnf *)rcvMsg->content;
    result   = AT_OK;

    result = AT_GetModemIdFromClient(0, &modemId);
    if (result != VOS_OK) {
        AT_ERR_LOG("AT_RcvMtaCclkQryCnf: Get modem id fail.");
        return VOS_ERR;
    }

    netCtx = AT_GetModemNetCtxAddrFromModemId(modemId);

    g_atSendDataBuff.bufLen = 0;

    /* 判断查询操作是否成功 */
    if (qryCnf->result == MTA_AT_RESULT_NO_ERROR) {
        result = AT_OK;
        time   = qryCnf->time;

        netCtx->timeInfo.universalTimeandLocalTimeZone.year     = qryCnf->time.year;
        netCtx->timeInfo.universalTimeandLocalTimeZone.month    = qryCnf->time.month;
        netCtx->timeInfo.universalTimeandLocalTimeZone.day      = qryCnf->time.day;
        netCtx->timeInfo.universalTimeandLocalTimeZone.hour     = qryCnf->time.hour;
        netCtx->timeInfo.universalTimeandLocalTimeZone.minute   = qryCnf->time.minute;
        netCtx->timeInfo.universalTimeandLocalTimeZone.second   = qryCnf->time.second;
        netCtx->timeInfo.universalTimeandLocalTimeZone.timeZone = qryCnf->time.timeZone;
        netCtx->timeInfo.ieFlg |= NAS_MM_INFO_IE_UTLTZ;

        AT_PrintCclkTime(indexNum, &time, modemId);
    } else {
        result = AT_ERROR;
    }

    return result;
}

VOS_UINT32 AT_RcvDrvAgentCpuloadQryRsp(struct MsgCB *msg)
{
    VOS_UINT32               i;
    VOS_UINT16               length = 0;
    VOS_UINT8                indexNum = 0;
    DRV_AGENT_CpuloadQryCnf *event    = VOS_NULL_PTR;
    DRV_AGENT_Msg           *rcvMsg   = VOS_NULL_PTR;

    /* 初始化 */
    rcvMsg = (DRV_AGENT_Msg *)msg;
    event  = (DRV_AGENT_CpuloadQryCnf *)rcvMsg->content;

    /* 通过clientid获取index */
    if (At_ClientIdToUserId(event->atAppCtrl.clientId, &indexNum) == AT_FAILURE) {
        AT_WARN_LOG("AT_RcvDrvAgentCpuloadQryRsp: AT INDEX NOT FOUND!");
        return VOS_ERR;
    }

    if (AT_IsBroadcastClientIndex(indexNum)) {
        AT_WARN_LOG("AT_RcvDrvAgentCpuloadQryRsp: AT_BROADCAST_INDEX.");
        return VOS_ERR;
    }

    /* AT模块在等待CPULOAD查询命令的结果事件上报 */
    if (g_atClientTab[indexNum].cmdCurrentOpt != AT_CMD_CPULOAD_READ) {
        return VOS_ERR;
    }

    /* 使用AT_STOP_TIMER_CMD_READY恢复AT命令实体状态为READY状态 */
    AT_StopTimerCmdReady(indexNum);

    /* 输出设置操作结果 :  */
    g_atSendDataBuff.bufLen = 0;

    length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
        (VOS_CHAR *)g_atSndCodeAddress + length, "%s: ", g_parseContext[indexNum].cmdElement->cmdName);

    event->curCpuLoadCnt = TAF_MIN(event->curCpuLoadCnt, TAF_MAX_CPU_CNT);

    for (i = 0; i < event->curCpuLoadCnt; i++) {
        /* 100用来计算每个核的负载占用百分比的整数部分和小数部分 */
        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, "%u.%02u", event->curCpuLoad[i] / 100,
            event->curCpuLoad[i] % 100);

        if ((event->curCpuLoadCnt - 1) == i) {
            break;
        }

        length += (VOS_UINT16)AT_FormatReportString(AT_CMD_MAX_LEN, (VOS_CHAR *)g_atSndCodeAddress,
            (VOS_CHAR *)g_atSndCodeAddress + length, ",");
    }

    /* 调用At_FormatResultData输出结果 */
    g_atSendDataBuff.bufLen = length;
    At_FormatResultData(indexNum, AT_OK);
    return VOS_OK;
}

