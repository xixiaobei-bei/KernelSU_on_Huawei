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
#include "at_custom_rcm_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_rcm_set_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_RCM_CMD_TBL_C

#if (FEATURE_DCXO_HI1102_SAMPLE_SHARE == FEATURE_ON)
static const AT_ParCmdElement g_atCustomRcmCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: DCXO样本获取
     * [说明]: 该命令用于获取DCXO样本信息。
     *         注意：受宏控制，该命令目前仅在Orlando平台使用。
     *         该命令暂不支持。
     * [语法]:
     *     [命令]: ^DCXOQRY=<SampleType>
     *     [结果]: <CR><LF>^DCXOQRY: <SampleType ><Result > [[,<aTime>,<aMode>,<aTemp>,<aPpm>],[,<aTime>,<aMode>,<aTemp>,<aPpm>],>],[,<aTime>,<aMode>,<aTemp>,<aPpm>],……]
     *             [,<Frequency >,<Teperature]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <SampleType>: 整型值，DCXO样本类型。
     *             0：高温段样本数据；
     *             1：中温段样本数据；
     *             2：低温段样本数据；
     *             3：产线校准结果；
     *     <Result>: 整型值，DCXO样本查询结果。
     *             0：查询结果正确；
     *             1：查询失败；
     *             2：查询请求参数错误；
     *             3：定时器超时；
     *             4：未知错误；
     *     <aTime>: 整型值，DCXO样本时间。年月，比如2016.4表示为1604，小于1604表示该样本无效
     *     <aMode>: 整型值，DCXO样本模式。
     *     <aTemp>: 整型值，DCXO样本温度。现有温度单位为0.001度。
     *     <aPpm>: 整型值，DCXO样本相对频偏。定点化2^31/10^6
     *     <Frequency>: 普通晶体的初始频偏
     *     <Teperature>: 普通晶体的初始温度
     * [示例]:
     *     · 查询高温段DCXO样本
     *       AT^DCXOQRY=0
     *       ^DCXOQRY: 0,0,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139
     *       OK
     *     · 查询中温段DCXO样本
     *       AT^DCXOQRY=1
     *       ^DCXOQRY: 1,0,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139
     *       OK
     *     · 查询低温段DCXO样本
     *       AT^DCXOQRY=2
     *       ^DCXOQRY: 2,0,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139
     *       OK
     *     · 查询产线校准结果
     *       AT^DCXOQRY=3
     *       ^DCXOQRY: 3,0,100,101
     *       OK
     */
    { AT_CMD_DCXOQRY,
      AT_SetDcxoQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DCXOQRY", (TAF_UINT8 *)"(0-3)" },
};

/* 注册RCM定制AT命令表 */
VOS_UINT32 AT_RegisterCustomRcmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomRcmCmdTbl, sizeof(g_atCustomRcmCmdTbl) / sizeof(g_atCustomRcmCmdTbl[0]));
}
#endif

