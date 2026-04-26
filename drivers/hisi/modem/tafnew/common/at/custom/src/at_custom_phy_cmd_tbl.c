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
#include "at_custom_phy_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"
#include "at_lte_common.h"

#include "at_custom_phy_set_cmd_proc.h"
#include "at_custom_phy_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_PHY_CMD_TBL_C

static const AT_ParCmdElement g_atCustomPhyCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 物理层配置通用命令
     * [说明]: 该命令用于支持物理层配置通用命令。
     * [语法]:
     *     [命令]: ^PHYCOMCFG=<cmd_type>,
     *             <rat>,
     *             <para1>,
     *             [<para2>,
     *             [<para3>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况时返回：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <cmd_type>: 整型值十进制 范围（0-65535）
     *             命令类型
     *     <rat>: 整型值十进制 范围（0-65535）
     *             接入模式，以bit为表示，如果需要设置多个模式，则将对应的bit位设置上
     *             1（00000001）：GSM实际输入十进制数 1
     *             2（00000010）：WCDMA实际输入十进制数 2
     *             4（00000100）：LTE实际输入十进制数 4
     *             8（00001000）：TDS实际输入十进制数 8
     *             16384（100000000000000）：NR实际输入十进制数16384
     *             64（01000000）：LTE或NR Band扩展（band65-band128）实际输入十进制数64
     *             128（10000000）：LTE或NR Band扩展（band129-band192）实际输入十进制数128
     *             256(100000000) ：NR Band扩展（band193-band256）实际输入十进制数256
     *             512（1000000000）：NR Band扩展（band257-band320）实际输入十进制数512
     *             如果要设置所有制式，则设置为255（11111111）
     *     <para1>: 整型值十进制 范围（0-4294967295）
     *             根据cmd_type取值不同表示不同含义
     *     <para2>: 整型值十进制 范围（0-4294967295）
     *             根据cmd_type取值不同表示不同含义
     *             如果不设置，自动设为0
     *     <para3>: 整型值十进制 范围（0-4294967295）
     *             根据cmd_type取值不同表示不同含义
     *             如果不设置，自动设为0
     * [表]: 命令功能取值说明
     *       <cmd_type>,                  其余设定值,
     *       0: 功能为锁定为上天线,       para1: 0：去锁定，1：TAS交叉，2：TAS直通.
     *                                    para2：  bit0-bit31对应3GPP band1 – band32
     *                                    para3: bit0-bit31对应3GPP band33 – band64
     *                                    para2, para3全为0表示所有band
     *                                    RAT LTE Band扩展置上后，para2, para3则表示band65 – band192
     *                                    对于锁定请求，每个模的TAS交叉或直通的Band信息以最后一次请求为准；
     *                                    Dsds2.0只在业务通道才执行锁定,
     *       1：设置HiTune支持的调谐类型, <rat>非0即可，<para1>为0表示不支持HiTune调谐，1表示支持开环，2表示支持完整闭环，3表示支持快速闭环(只供终端特定测试场景使用)，其他参数不涉及,
     *       2：HALL值循环配置,           <rat>非0即可，其他参数不涉及,
     *       3：温保限PA发射功率配置,     <rat>配置为255即可，<para1>的bit0-bit7 为GUCLT的功率回退档位信息，bit8-bit15 为NR的功率回退档位信息，bit16-bit31为保留字段，其他参数不涉及,
     * [示例]:
     *     · 对W模所有BAND执行天线锁交叉
     *       AT^PHYCOMCFG=0,2,1,0,0
     *       OK
     *     · 对W模所有BAND执行天线锁直通
     *       AT^PHYCOMCFG=0,2,2,0,0
     *       OK
     *     · 对W模所有BAND执行天线解锁
     *       AT^PHYCOMCFG=0,2,0,0,0
     *       OK
     *     · 设置HiTune支持开环
     *       AT^PHYCOMCFG=1,2,1,0,0
     *       OK
     *     · 设置HALL值循环配置
     *       AT^PHYCOMCFG=2,1,0,0,0
     *       OK
     *     · 设置温保功率回退档位
     *       AT^PHYCOMCFG=3,255,7,0,0
     *       OK
     */
    { AT_CMD_PHYCOMCFG,
      AT_SetPhyComCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      /* cmd_type, ratbitmap, para1, para2, para3 */
      (VOS_UINT8 *)"^PHYCOMCFG", (VOS_UINT8 *)"(0-65535),(0-65535),(0-4294967295),(0-4294967295),(0-4294967295)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 物理层配置通用命令
     * [说明]: 此命令仅用于NR模式下，用于给NRPHY进行参数配置。
     * [语法]:
     *     [命令]: ^NRPHYCOMCFG=<cmd_type>,<para1>
     *             [,<para2>,<para3>,
     *             <para4>,<para5>,
     *             <para6>,<para7>,
     *             <para8>,<para9>,
     *             <para10>,<para11>,
     *             <para12>,<para13>,
     *             <para14>,<para15>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行设置失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <cmdtype>: 整型值十进制 范围（0-65535）命令类型
     *     <para1>: 整型值十进制 范围（0-4294967295）根据cmd_type取值不同表示不同含义
     *     <para2>: 整型值十进制 范围（0-4294967295）根据cmd_type取值不同表示不同含义
     *     <para3>: 整型值十进制 范围（0-4294967295）根据cmd_type取值不同表示不同含义
     *     <…>: ….
     *     <para15>: 整型值十进制 范围（0-4294967295）根据cmd_type取值不同表示不同含义
     * [表]: 命令功能取值说明
     *       < cmd_type >,                其余设定值,
     *       0: MicroSleep 2.0,           para1：0：退出MicroSleep；1：进入MicroSleep,
     *       1：Local BWP,                para1：0：表示功能不生效；1：表示功能生效,
     *       2: 动态限制上行最大发射功率, para1：0：表示功能不生效；1：表示功能生效,
     * [示例]:
     *     · 进入 MicroSleep 2.0
     *       AT^NRPHYCOMCFG=0,1
     *       OK
     *     · 退出 MicroSleep 2.0
     *       AT^NRPHYCOMCFG=0,0
     *       OK
     *     · Local BWP功能生效
     *       AT^NRPHYCOMCFG=1,1
     *       OK
     *     · Local BWP功能不生效
     *       AT^NRPHYCOMCFG=1,0
     *       OK
     *     · 动态限制上行最大发射功率生效
     *       AT^NRPHYCOMCFG=2,1
     *       OK
     *     · 动态限制上行最大发射功率不生效
     *       AT^NRPHYCOMCFG=2,0
     *       OK
     */
    { AT_CMD_NRPHYCOMCFG,
      AT_SetNrphyComCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      /* cmd_type, para1, para2, para3, para4, para5, para6, para7, para8, para9, para10, para11, para12, para13, para14, para15 */
      (VOS_UINT8 *)"^NRPHYCOMCFG", (VOS_UINT8 *)"(0-65535),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询AFC时钟频偏信息
     * [说明]: 该命令用于查询Modem的AFC时钟的锁定状态及频偏信息，共时钟方案提供给GPS芯片来获取当前AFC时钟的频偏信息以便校准时钟并搜星。
     *         命令返回的频偏信息仅在时钟处于锁定状态时才有效。
     * [语法]:
     *     [命令]: ^AFCCLKINFO?
     *     [结果]: <CR><LF>^AFCCLKINFO: <status>[,<deviation>]， <sTemp>,<eTemp>,<a0_m>,<a0_e>,<a1_m >, <a1_e >, <a2_m >,<a2_e >,<a3_m >,<a3_e >,<rat>，<ModemId><CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^AFCCLKINFO=?
     *     [结果]: <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <status>: 整型值，AFC时钟锁定状态：
     *             0：非锁定；
     *             1：锁定。
     *     <deviation>: 整型值，时钟频偏信息，取值范围-30000~30000，单位ppb。
     *     <sTemp>: 整型值，温度范围的最高温度，取值范围-400~1100，单位0.1℃。
     *     <eTemp>: 整型值，温度范围的最低温度，取值范围-400~1100，单位0.1℃，最高温度高于最低温度。
     *     <a0_m>: 多项式系数a0的尾数(mantissa)，取值范围0x00000000~0xFFFFFFFF。
     *     <a0_e>: 多项式系数a0的指数(exponent)，取值范围0x0000~0xFFFF。
     *     <a1_m>: 多项式系数a1的尾数(mantissa)，取值范围0x00000000~0xFFFFFFFF。
     *     <a1_e>: 多项式系数a1的指数(exponent)，取值范围0x0000~0xFFFF。
     *     <a2_m>: 多项式系数a2的尾数(mantissa)，取值范围0x00000000~0xFFFFFFFF。
     *     <a2_e>: 多项式系数a2的指数(exponent)，取值范围0x0000~0xFFFF。
     *     <a3_m>: 多项式系数a3的尾数(mantissa)，取值范围0x00000000~0xFFFFFFFF。
     *     <a3_e>: 多项式系数a3的指数(exponent)，取值范围0x0000~0xFFFF。
     *     <rat>: 整数类型，锁定的制式：
     *             0：GSM；
     *             1：WCDMA；
     *             2：LTE；
     *             3：TDS-CDMA；
     *             4：CDMA_1X；
     *             5：HRPD；
     *             6：NR。
     *     <ModemId>: 整数类型，锁定的Modem Id：
     *             0：Modem 0；
     *             1：Modem 1；
     *             2：Modem 2。
     * [示例]:
     *     · 查询AFC时钟频偏信息
     *       AT^AFCCLKINFO?
     *       ^AFCCLKINFO: 1,-1754,562,-88,769032704,4253851137,485040176,2896011700,49182,49169,49153,16370,1,0
     *       OK
     *     · 执行测试命令
     *       AT^AFCCLKINFO=?
     *       ERROR
     */
    { AT_CMD_AFCCLKINFO,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryAfcClkInfo, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^AFCCLKINFO", VOS_NULL_PTR },


    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 开关Body SAR功能
     * [说明]: 该命令用于开启关闭Body SAR功能，并可获取当前Body SAR功能的状态。
     *         在Body SAR开关打开且当前制式的功率门限参数被配置的情况下，Body SAR功能才会生效。所以建议先使用AT^BODYSARWCDMA和AT^BODYSARGSM设置好各频段的最大发射功率门限参数值，再使用此命令来开启或关闭Body SAR功能。
     *         该命令暂不支持。
     * [语法]:
     *     [命令]: ^BODYSARON=<on>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^BODYSARON?
     *     [结果]: <CR><LF>^BODYSARON: <on><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^BODYSARON=?
     *     [结果]: <CR><LF>^BODYSARON: (0,1)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <on>: 整型值，Body SAR功能开关状态：
     *             0：关闭Body SAR功能；
     *             1：开启Body SAR功能。
     * [示例]:
     *     · 开启Body SAR功能
     *       AT^BODYSARON=1
     *       OK
     *     · 查询Body SAR功能状态
     *       AT^BODYSARON?
     *       ^BODYSARON: 1
     *       OK
     *     · 执行测试命令
     *       AT^BODYSARON=?
     *       ^BODYSARON: (0,1)
     *       OK
     */
    { AT_CMD_BODYSARON,
      AT_SetBodySarOnPara, AT_SET_PARA_TIME, AT_QryBodySarOnPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^BODYSARON", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设定WCDMA最大发射功率门限值
     * [说明]: 该命令用于设定查询WCDMA各频段的最大发射功率门限值。
     *         设置命令依据频段位域的值，对选中的频段进行设定。当不同频段的设定值不同时，可分多组参数进行不同门限值的设定。
     *         该命令暂不支持。
     * [语法]:
     *     [命令]: ^BODYSARWCDMA=<power>[,<band>[,<power,<band>]…]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^BODYSARWCDMA?
     *     [结果]: <CR><LF>^BODYSARWCDMA: (list of (<power>,<band>)s)
     *             <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^BODYSARWCDMA=?
     *     [结果]: <CR><LF>^BODYSARWCDMA: (17,24),<band> <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <power>: 整型值，对应频段的最大发射功率门限值，单位为dbm，取值范围17~24。
     *     <band>: 16进制数字串，频段位域。长度为4字节，每个Bit位对应一个频段，取值为表13-4中各参数值，或除0x3FFFFFFF以外的各参数叠加值。
     * [表]: WCDMA频段对应参数值表
     *       频段,                    参数值,
     *       WCDMA_I_2100,            00000001,
     *       WCDMA_II_1900,           00000002,
     *       WCDMA_III_1800,          00000004,
     *       WCDMA_IV_1700,           00000008,
     *       WCDMA_V_850,             00000010,
     *       WCDMA_VI_800,            00000020,
     *       WCDMA_VII_2600,          00000040,
     *       WCDMA_VIII_900,          00000080,
     *       WCDMA_IX_1700,           00000100,
     *       WCDMA_X（暂不支持）,     00000200,
     *       WCDMA_XI_1500,           00000400,
     *       WCDMA_XII（暂不支持）,   00000800,
     *       WCDMA_XIII（暂不支持）,  00001000,
     *       WCDMA_XIV（暂不支持）,   00002000,
     *       WCDMA_XV（暂不支持）,    00004000,
     *       WCDMA_XVI（暂不支持）,   00008000,
     *       WCDMA_XVII（暂不支持）,  00010000,
     *       WCDMA_XVIII（暂不支持）, 00020000,
     *       WCDMA_XIX_850,           00040000,
     *       所有支持的频段,          3FFFFFFF,
     * [示例]:
     *     · 设置WCDMA I的最大发射功率门限值为20，WCDMA II和III的最大发射功率门限值为18
     *       AT^BODYSARWCDMA=20,00000001,18,00000006
     *       OK
     *     · 查询当前所有支持的WCDMA频段最大发射功率门限值为19
     *       AT^BODYSARWCDMA?
     *       ^BODYSARWCDMA: (19,3FFFFFFF)
     *       OK
     *     · 查询当前支持的WCDMA频段I的最大发射功率门限值为19，频段II、III为20
     *       AT^BODYSARWCDMA?
     *       ^BODYSARWCDMA: (19,00000001),(20,00000006)
     *       OK
     *     · 执行测试命令，当前支持WCDMA频段I、II、III、IV
     *       AT^BODYSARWCDMA=?
     *       ^BODYSARWCDMA: (17,24),0000000F
     *       OK
     */
    { AT_CMD_BODYSARWCDMA,
      AT_SetBodySarWcdmaPara, AT_NOT_SET_TIME, AT_QryBodySarWcdmaPara, AT_NOT_SET_TIME, AT_TestBodySarWcdmaPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^BODYSARWCDMA", (VOS_UINT8 *)"(17-24),(@band),(17-24),(@band),(17-24),(@band),(17-24),(@band),(17-24),(@band),(17-24),(@band),(17-24),(@band),(17-24),(@band)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设定GSM最大发射功率门限值
     * [说明]: 该命令用于设定查询GSM各频段的最大发射功率门限值。
     *         设置命令依据频段位域的值，对选中的频段进行设定。当不同频段的设定值不同时，可分多组参数进行不同门限值的设定。
     *         该命令暂不支持。
     * [语法]:
     *     [命令]: ^BODYSARGSM=<power>[,<band>[,<power,<band>]…]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^BODYSARGSM?
     *     [结果]: <CR><LF>^BODYSARGSM: (list of (<power>,<band>)s)
     *             <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^BODYSARGSM=?
     *     [结果]: <CR><LF>^BODYSARGSM: (15,33),<band> <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <power>: 整型值，对应频段的最大发射功率门限值，单位为dbm，取值范围15~33。
     *     <band>: 16进制数字串，频段位域。长度为4字节，每个Bit位对应一个频段，取值为表2-23中各参数值，或除0x3FFFFFFF以外的各参数叠加值。
     * [表]: GSM频段对应参数值表
     *       频段,           参数值,
     *       GSM850(GPRS),   00000001,
     *       GSM900(GPRS),   00000002,
     *       GSM1800(GPRS),  00000004,
     *       GSM1900(GPRS),  00000008,
     *       GSM850(EDGE),   00010000,
     *       GSM900(EDGE),   00020000,
     *       GSM1800(EDGE),  00040000,
     *       GSM1900(EDGE),  00080000,
     *       所有支持的频段, 3FFFFFFF,
     * [示例]:
     *     · 设置GSM850的最大发射功率门限值为19，其余的最大发射功率门限值均为20
     *       AT^BODYSARGSM=19,00010001,20,000E000E
     *       OK
     *     · 查询当前所有支持的GSM频段最大发射功率门限值均为20
     *       AT^BODYSARGSM?
     *       ^BODYSARGSM: (20,3FFFFFFF)
     *       OK
     *     · 查询当前支持的GSM频段GSM850的最大发射功率门限值为18，频段GSM900为20，其余为19
     *       AT^BODYSARGSM?
     *       ^BODYSARGSM: (18,00010001),(20,00020002),(19,000A000A)
     *       OK
     *     · 执行测试命令，当前支持GSM850、GSM900、GSM1800和GSM1900频段
     *       AT^BODYSARGSM=?
     *       ^BODYSARGSM: (15,33),000F000F
     *       OK
     */
    { AT_CMD_BODYSARGSM,
      AT_SetBodySarGsmPara, AT_NOT_SET_TIME, AT_QryBodySarGsmPara, AT_NOT_SET_TIME, AT_TestBodySarGsmPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^BODYSARGSM", (VOS_UINT8 *)"(15-33),(@band),(15-33),(@band),(15-33),(@band),(15-33),(@band),(15-33),(@band),(15-33),(@band),(15-33),(@band),(15-33),(@band)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 控制GPS参考时钟状态主动上报
     * [说明]: 该命令用于AP-Modem形态下控制GPS参考时钟状态的主动上报及查询，以避免不确定的参考时钟状态的影响。
     *         命令使用场景：GPS芯片下发。
     *         命令使用限制：只限定于AP-Modem形态使用。
     *         命令使用通道：只能通过与AP对接的AT通道下发。
     * [语法]:
     *     [命令]: ^REFCLKFREQ=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^REFCLKFREQ?
     *     [结果]: 查询成功：
     *             <CR><LF>^REFCLKFREQ: <ver>,<freq>,<precision> <status><CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^REFCLKFREQ=?
     *     [结果]: <CR><LF>^REFCLKFREQ: (list of supported <status>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，GPS参考时钟状态上报控制：
     *             0：不主动上报；
     *             1：主动上报。
     *     <ver>: 整型值，本命令的版本ID，当前为0。
     *     <freq>: 整型值，GPS参考时钟的频率值，单位Hz。
     *     <precision>: 整型值，当前GPS参考时钟的精度，单位ppb。
     *     <status>: 整型值，GPS参考时钟频率锁定状态：
     *             0：未锁定；
     *             1：锁定。
     * [示例]:
     *     · 开启GPS参考时钟状态主动上报
     *       AT^REFCLKFREQ=1
     *       OK
     *     · 查询GPS参考时钟状态
     *       AT^REFCLKFREQ?
     *       ^REFCLKFREQ: 0,19200000,100,1
     *       OK
     *     · 执行测试命令
     *       AT^REFCLKFREQ=?
     *       ^REFCLKFREQ: (0,1)
     *       OK
     */
    { AT_CMD_REFCLKFREQ,
      AT_SetRefclkfreqPara, AT_SET_PARA_TIME, AT_QryRefclkfreqPara, AT_QRY_PARA_TIME, AT_TestRefclkfreqPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^REFCLKFREQ", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 配置MODEM切换主分集天线
     * [说明]: AP-Modem形态下，AP根据sensor算法判别左右手，通知modem进行切换分集天线。
     * [语法]:
     *     [命令]: ^HANDLEDECT=<handletype>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^HANDLEDECT?
     *     [结果]: <CR><LF>^HANDLEDECT: <handletype><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^HANDLEDECT=?
     *     [结果]: <CR><LF>^HANDLEDECT: (0-4)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <handletype>: 整型值，手持phone或pad的位置类型。
     *             0、no_handle；
     *             1、左手角握；
     *             2、右手角握；
     *             3、横屏上角握；
     *             4、横屏下角握。
     * [示例]:
     *     · 配置MODEM按照左手角握配置主分集天线
     *       AT^HANDLEDECT=1
     *       OK
     */
    { AT_CMD_HANDLEDECT,
      At_SetHandleDect, AT_SET_PARA_TIME, At_QryHandleDect, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HANDLEDECT", (VOS_UINT8 *)"(0-4)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 全网通产品设置RF ProfileId
     * [说明]: 只有在全网通功能使能的情况下才能设置RF ProfileId。
     * [语法]:
     *     [命令]: ^RATRFSWITCH=<RFProfileId>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^RATRFSWITCH?
     *     [结果]: <CR><LF>^RATRFSWITCH: <enable>,<RFProfileId><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR <err><CR><LF>
     *     [命令]: ^RATRFSWITCH=?
     *     [结果]: <CR><LF>^RATRFSWITCH: (list of supported <RFProfileId>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <RFProfileId>: RF使用的RFProfileId，整型值，取值范围为0~7。对应的Profile Id将保存到NV NV_TRI_MODE_FEM_PROFILE_ID中
     *             0：ProfileId 0;
     *             1：ProfileId 1;
     *             2：ProfileId 2;
     *     <enable>: 整型值，全网通功能是否使能
     *             0：全网通功能没有使能；
     *             1：全网通功能使能。
     * [示例]:
     *     · 查询RF ProfileId
     *       AT^RATRFSWITCH?
     *       RATRFSWITCH: 0,0
     *       OK
     *     · 设置RF ProfileId为1
     *       AT^RATRFSWITCH=1
     *       OK
     */
    { AT_CMD_RATRFSWITCH,
      At_SetRatRfSwitch, AT_NOT_SET_TIME, At_QryRatRfSwitch, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RATRFSWITCH", (VOS_UINT8 *)"(0-7)" },

    { AT_CMD_MCS,
      AT_SetMcsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^MCS", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: GUL发射功率查询
     * [说明]: 通过该AT获取当前modem发射功率，对于LTE来说是各信道（PRACH/PUCCH/PUSCH/SRS）的发射功率。对于GU来讲是stxpwr有效。
     *         本命令仅在GUL下有效，查询当前主模的发射功率，ENDC场景查询的是LTE的发射功率。
     * [语法]:
     *     [命令]: ^TXPOWER?
     *     [结果]: <CR><LF>^TXPOWER: <stxpwr>,<PPusch>,<PPucch>,<PSrs>,<PPrach><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <stxpwr>: 整型值，单位： 0.1dBm;如果是2G网络时， 取值范围为0到33dBm（实际上报值为0到330）；如果为3G网络时，取值范围为-50到24dBm（实际上报值为-510到240）,4G时，该参数值为999。
     *     <PPusch>: 整型值（<=23dbm），PUSCH发射功率，当为2G或者3G时，该值为999。
     *     <PPucch>: 整型值（<=23dbm），PUCCH发射功率，当为2G或者3G时，该值为999。
     *     <PSrs>: 整型值（<=23dbm），SRS发射功率，当为2G或者3G时，该值为999。
     *     <PPrach>: 整型值（<=23dbm），PRACH发射功率，当为2G或者3G时，该值为999。
     * [示例]:
     *     · 主动查询LTE的发射功率
     *       AT^TXPOWER?
     *       ^TXPOWER: 999,23,23,22,23
     *       OK
     *     · 主动查询G/W的发射功率
     *       AT^TXPOWER?
     *       ^TXPOWER: 240,999,999,999,999
     *       OK
     */
    { AT_CMD_TXPOWER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryTxPowerPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TXPOWER", VOS_NULL_PTR },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR发射功率查询
     * [说明]: 通过该AT获取当前modem发射功率，对于NR来说是各信道（PRACH/PUCCH/PUSCH/SRS）的发射功率。支持多CC的发射功率上报。
     *         本命令仅在NR/L下有效，查询NR的发射功率，ENDC场景查询的是NR的发射功率。
     * [语法]:
     *     [命令]: ^NTXPOWER?
     *     [结果]: 执行成功：
     *             <CR><LF>^NTXPOWER: <PPusch1>,<PPucch1>,<PSrs1>,<PPrach1>,<Freq1>,[<PPusch2>,<PPucch2>,<PSrs1>,<PPrach1>,<Freq2>,[<PPusch3>,<PPucch3>,<PSrs3>,<PPrach3>,<Freq3>,[<PPusch4>,<PPucch4>,<PSrs4>,<PPrach4>,<Freq4>]]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <PPuschN>: 整型值（<=26dbm），PUSCH发射功率，无效值999，N为载波数目，最大为4。
     *     <PPucchN>: 整型值（<=26dbm），PUCCH发射功率，无效值999，N为载波数目，最大为4。
     *     <PSrsN>: 整型值（<=26dbm），SRS发射功率，无效值999，N为载波数目，最大为4。
     *     <PPrachN>: 整型值（<=26dbm），PRACH发射功率，无效值999，N为载波数目，最大为4。
     *     <FreqN>: 整型变量，表示当前终端所处小区的频率，单位为(100kHz)，无效值取0，N为载波数目，最大为4。
     * [示例]:
     *     · 主动查询NR的发射功率 2CC
     *       AT^NTXPOWER?
     *       ^NTXPOWER: 23,3,23,22,8675,23,2,22,23,8677
     *       OK
     *     · 主动查询NR的发射功率单小区
     *       AT^NTXPOWER?
     *       ^NTXPOWER: 23,3,23,22,8675
     *       OK
     */
    { AT_CMD_NTXPOWER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNtxPowerPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NTXPOWER", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置/查询LTE降功耗命令
     * [说明]: 本接口用来支持对海思物理层实现的动态升/降CC和动态调整RI的降功耗功能，由上层应用基于实际使用场景使用。
     *         需要bitCloseCaAndMimoCtrl特性支持。
     *         注：^LTEPWRCTRL=1,1下发后，当前只支持降RI为0或者1。
     * [语法]:
     *     [命令]: ^LTEPWRCTRL=<Mode>,<Para>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LTEPWRCTRL=?
     *     [结果]: <CR><LF>^LTEPWRCTRL: (list of supported <Mode>s),(list of supported <Para>s)<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <Mode>: 整型值，十进制数字，取值0~2。
     *             0：调整CC数；
     *             1：调整RI值；
     *             2：调整BSR（暂不支持）
     *     <Para>: 整型值，十进制数字，取值-8~63。
     *             当Mode=0时：范围（-8~8），表示要降（负值）或者升（正值）的CC数；
     *             当Mode=1时：范围（0~1），1表示使能降RI功能；0表示关闭降RI功能；
     *             当Mode=2时：范围为0~63，表示支持的BSR的最大值，目前暂不支持；
     * [示例]:
     *     · SET命令
     *       AT^LTEPWRCTRL=1,1
     *       OK
     */
    { AT_CMD_LTEPWRCTRL,
      AT_SetLtePwrDissPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestLtePwrDissPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LTEPWRCTRL", (VOS_UINT8 *)"(0-2),(@para)" },

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR降RI命令
     * [说明]: 目前实现NR降RI功能。
     * [语法]:
     *     [命令]: ^NRPWRCTRL=<Mode>,<Para>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NRPWRCTRL=?
     *     [结果]: <CR><LF>^NRPWRCTRL: (list of supported <Mode>s),(list of supported <Para>s)<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <Mode>: 整型值，十进制数字，取值0~2。（目前只实现mode=1的功能）
     *             1：调整RI值；
     *     <Para>: 整型值，十进制数字。
     *             当Mode=1时：取值范围为Para为0,1
     *             1表示使能降RI功能
     *             0表示关闭降RI功能
     * [示例]:
     *     · 使能降RI功能
     *       AT^NRPWRCTRL=1,1
     *       OK
     */
    { AT_CMD_NRPWRCTRL,
      AT_SetNrPwrCtrlPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestNrPwrCtrlPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRPWRCTRL", (VOS_UINT8 *)"(0-2),(@para)" },
#endif
#endif

#if (FEATURE_UE_MODE_NR == FEATURE_OFF)
    { AT_CMD_PDMCTRL,
      AT_SetPdmCtrlPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^PDMCTRL", (TAF_UINT8 *)"(0-255),(0-65535),(0-65535),(0-65535)" }, /* value */
#endif

    { AT_CMD_TFDPDTQRY,
      At_SetQryTfDpdtPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TFDPDTQRY", (VOS_UINT8 *)DPDT_QRY_PARA_STRING },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 查询LTE网络频率信息
     * [说明]: 此命令查询当前LTE网络的频率信息，上层应用通过查询当前LTE的频率信息，用于规避与WIFI模块同频干扰。LTE与WIFI的频率干扰范围默认是(23700-24000)，可通过NV项动态调整。
     * [语法]:
     *     [命令]: ^LWCLASH?
     *     [结果]: <CR><LF>^LWCLASH: <state>,<ulfreq>,<ulbw>,<dlfreq>,<dlbw>,<band>,<ant_num>,<ant1_ulfreq>,<ant1_ulbw>,<ant1_dlfreq>,<ant1_dlbw>,<ant2_ulfreq>,<ant2_ulbw>,<ant2_dlfreq>,<ant2_dlbw>,<ant3_ulfreq>,<ant3_ulbw>,<ant3_dlfreq>,<ant3_dlbw>,<ant4_ulfreq>,<ant4_ulbw>,<ant4_dlfreq>,<ant4_dlbw>,<ant_dlmm><ant1_dlmm><ant2_dlmm><ant3_dlmm><ant4_dlmm>,<dl256QAMflag><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <state>: 当前LTE频率与WIFI频率冲突状态。
     *             0：不冲突，即LTE与WIFI不处于频率干扰范围；
     *             1：冲突，即LTE与WIFI处于频率干扰范围；
     *             2：无效值（当前LTE非接入状态，无固定的上行频率）。
     *     <ulfreq>: LTE上行频率，单位100kHz，取值范围：0~4294967295。
     *     <ulbw>: LTE上行带宽。
     *             0：1.4M；
     *             1：3M；
     *             2：5M；
     *             3：10M；
     *             4：15M；
     *             5：20M。
     *     <dlfreq>: LTE下行频率，单位100kHz，取值范围：0~4294967295。
     *     <dlbw>: LTE下行带宽。
     *             0：1.4M；
     *             1：3M；
     *             2：5M；
     *             3：10M；
     *             4：15M；
     *             5：20M。
     *     <band>: LTE模式的频段号，取值范围：1~70。
     *     <ant_num>: 天线的频点和频段个数，取值范围0~3。
     *     <dlmm>: UE MIMO层数
     *     <dl256QAMflag>: 是否支持下行256QAM。
     *             0：不支持；
     *             1：支持。
     */
    { AT_CMD_LWCLASH,
      VOS_NULL_PTR, AT_SET_PARA_TIME, AT_QryLwclashPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^LWCLASH", VOS_NULL_PTR },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 指示LPHY/NPHY进入低功耗模式
     * [说明]: 通过该AT命令指示LPHY或NR PHY进入低功耗模式。
     * [语法]:
     *     [命令]: ^LOWPWRMODE=<mode>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <mode>: 接入技术，取值0,1
     *             0：LTE
     *             1：NR
     * [示例]:
     *     · 指示LPHY进入低功耗模式
     *       AT^LOWPWRMODE=0
     *       OK
     */
    { AT_CMD_LOWPOWERMODE,
      At_SetLowPowerModePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LOWPWRMODE", (VOS_UINT8 *)LOWPWRMODE_CMD_PARA_STRING },
#endif
};

/* 注册PHY定制AT命令表 */
VOS_UINT32 AT_RegisterCustomPhyCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomPhyCmdTbl, sizeof(g_atCustomPhyCmdTbl) / sizeof(g_atCustomPhyCmdTbl[0]));
}

