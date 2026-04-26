/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#ifndef _ATCHECKFUNC_H_
#define _ATCHECKFUNC_H_

#include "AtParse.h"
#include "ps_common_def.h"
#include "at_mntn.h"
#include "at_set_para_cmd.h"

#define AT_GNA_E5_MAC_ADDR_LEN_BETWEEN_COLONS 2 /* MAC地址中冒号之间的字符个数 */
#define AT_GNA_E5_MAC_ADDR_COLON_NUM 5          /* MAC地址中冒号的个数 */
#define AT_MAC_ADDR_LEN 32                      /* MAC地址长度 AT_GNA_E5_MAC_ADDR_LEN ->AT_MAC_ADDR_LEN */
#define AT_CDATA_DIAL_777_LEN 4

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* 模块名＋意义＋HEADER */

TAF_UINT32 At_CheckSemicolon(TAF_UINT8 Char);
TAF_UINT32 At_CheckDialNum(TAF_UINT8 Char);
TAF_UINT32 At_CheckDmChar(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharStar(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharWell(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharRightArrow(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharG(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharI(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharS(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharD(TAF_UINT8 Char);

TAF_UINT32 At_CheckCharPlus(TAF_UINT8 Char);
TAF_UINT32 At_CheckLeftBracket(TAF_UINT8 Char);
TAF_UINT32 At_CheckRightBracket(TAF_UINT8 Char);
TAF_UINT32 At_CheckDigit(TAF_UINT8 Char);
TAF_UINT32 At_CheckLetter(TAF_UINT8 Char);
TAF_UINT32 At_CheckUpLetter(TAF_UINT8 Char);
TAF_UINT32 At_CheckChar(TAF_UINT8 Char);
TAF_UINT32 At_CheckAlpha(TAF_UINT8 Char);
TAF_UINT32 At_CheckColon(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharSub(TAF_UINT8 Char);
TAF_UINT32 At_CheckQuot(TAF_UINT8 Char);
TAF_UINT32 At_CheckHorizontalLine(TAF_UINT8 Char);
TAF_UINT32 At_CheckEqu(TAF_UINT8 Char);
TAF_UINT32 At_CheckReq(TAF_UINT8 Char);
TAF_UINT32 At_CheckNoQuot(TAF_UINT8 Char);
TAF_UINT32 At_CheckNumString(TAF_UINT8 *data, TAF_UINT16 len);
TAF_UINT32 At_CheckMmiString(TAF_UINT8 *data, TAF_UINT16 len);
TAF_UINT32 At_CheckChar0x26(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharE(TAF_UINT8 Char);
TAF_UINT32 At_CheckChar1(TAF_UINT8 Char);
TAF_UINT32 At_CheckChar0(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharV(TAF_UINT8 Char);
TAF_UINT32 At_CheckCharF(TAF_UINT8 Char);

#if (FEATURE_IOT_ATNLPROXY == FEATURE_ON)
TAF_UINT32 AT_CheckSuccess(TAF_UINT8 Char);
TAF_UINT32 AT_CheckEscape(TAF_UINT8 Char);
#endif

TAF_UINT32 At_CheckRightArrowStr(TAF_UINT8 *data, TAF_UINT16 len);

VOS_UINT32 AT_CheckDateFormat(VOS_UINT8 *dateStr, VOS_UINT32 dateStrLen);

VOS_UINT32 AT_GetDate(VOS_UINT8 *dateStr, VOS_UINT32 dateStrLen, AT_DATE *date);

VOS_UINT32 AT_CheckDate(AT_DATE *date);

VOS_UINT32 At_CheckAndParsePara(AT_ParseParaType *atPara, VOS_UINT8 *begain, VOS_UINT8 *end);

VOS_UINT32 AT_CheckStrStartWith(VOS_UINT8 *lineStr,
                                VOS_UINT16 lineStrLen, VOS_UINT8 *pefixStr, VOS_UINT16 pefixStrLen
);

/* 比较、匹配基本命令名 */
VOS_UINT32 AT_CheckBasicCmdName(VOS_UINT8 Char);

/* 比较、匹配空格 */
VOS_UINT32 atCheckblank(VOS_UINT8 Char);

/* 比较、匹配逗号 */
VOS_UINT32 atCheckComma(VOS_UINT8 Char);

/* 比较、匹配冒号 */
VOS_UINT32 atCheckColon(VOS_UINT8 Char);

VOS_UINT32 atCheckChar0x2f(VOS_UINT8 Char);

/* 比较、匹配字符串类型 */
VOS_UINT32 At_CheckStringPara(AT_ParseParaType *para);

/* 比较、匹配数值类型的参数 */
VOS_UINT32 atCheckNumPara(AT_ParseParaType *para);

/* 比较、匹配字符范围类型的参数 */
VOS_UINT32 atCheckCharPara(AT_ParseParaType *para);

/* 比较、匹配数字 */
VOS_UINT32 atCheckNumString(VOS_UINT8 *data, VOS_UINT16 len);

/* 判断字符串中是否有"*"和"#" */
VOS_UINT32 atCheckMmiString(VOS_UINT8 *data, VOS_UINT16 len);

/* 判断字符串中是否有">" */
VOS_UINT32 atCheckRightArrowStr(VOS_UINT8 *data, VOS_UINT16 len);

/*
 * 功能描述: 比较、匹配无双引号包括字符串类型
 */
extern VOS_UINT32 atCheckNoQuotStringPara(AT_ParseParaType *para);

/*
 * 功能描述: 比较、匹配无双引号包括字母起始
 */
extern VOS_UINT32 atNoQuotLetter(VOS_UINT8 Char);

/*
 * 功能描述: 比较、匹配十六进制(0x或0X)
 */
extern VOS_UINT32 atCheckHex(VOS_UINT8 Char);

/*
 * 功能描述: 比较、匹配十六进制数字
 */
extern VOS_UINT32 atCheckHexNum(VOS_UINT8 Char);

extern VOS_UINT32 atfwParseSaveParam(VOS_UINT8 *stringBuf, VOS_UINT16 len);

extern VOS_UINT32 AT_SaveCdataDialParam(VOS_UINT8 *stringBuf, VOS_UINT16 len);

VOS_UINT8 At_GetDaysForEachMonth(VOS_UINT8 year, VOS_UINT8 month);
#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __AT_CHECK_FUNC_H__ */
