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
#include "at_device_as_cmd_tbl.h"

#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_device_as_set_cmd_proc.h"
#include "at_device_as_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_DEVICE_AS_CMD_TBL_C

static const AT_ParCmdElement g_atDeviceAsCmdTbl[] = {
#if (FEATURE_MBB_CUST == FEATURE_OFF)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询产品特性
     * [说明]: 查询产品支持的特性，包括制式（频段）和功能（分集、电池、按键、屏、wifi、FM、GPS、蓝牙、LAN、NSF）等信息。
     * [语法]:
     *     [命令]: ^SFEATURE?
     *     [结果]: <CR><LF>^SFEATURE: <number><CR><LF>
     *             <CR><LF>^SFEATURE: <feature>[,<content>]<CR><LF>
     *             [….]
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <number>: 功能的总数。
     *     <feature>: 支持的功能名称，上报时直接上报名称，不使用双引号。
     *             LTE
     *             HSPA+
     *             HSDPA
     *             HSUPA
     *             DIVESITY
     *             UMTS
     *             EDGE
     *             GPRS
     *             GSM
     *             WIMAX
     *             WIFI
     *             GPS
     *             TD
     *             NR
     *     <content>: 功能对应的信息。
     *             LTE：支持的频段信息[5]chater 5，如B1、B2等，多频段中间用逗号隔开；
     *             HSPA+：支持的频段信息，如BC1、BC2、BC3，多频段中间用逗号隔开；
     *             HSDPA：支持的频段信息，如BC1、BC2、BC3，多频段中间用逗号隔开；
     *             HSUPA：支持的频段信息，如BC1、BC2、BC3，多频段中间用逗号隔开；
     *             DIVESITY：支持的分集的频段信息，频段采用Index下标方式，描述与主集描述保持一致，制式使用一个字符表示（如：“L”-LTE、“U”-UMTS、“W”-WiMAX、“F”-WiFi等），实现时字符不加引号；
     *             UMTS：支持的频段信息，如BC1、BC2、BC3，多频段中间用逗号隔开；
     *             EDGE：支持的频段信息，用850/900/1800/1900表示，多频段中间用逗号隔开；
     *             GPRS：支持的频段信息，用850/900/1800/1900表示，多频段中间用逗号隔开；
     *             GSM：支持的频段信息，用850/900/1800/1900表示，多频段中间用逗号隔开；
     *             WIMAX：支持的频段信息，用2300/2500/3500表示，多频段中间用逗号隔开；
     *             WIFI：支持的制式，用a/b/g/n表示，多制式中间用逗号隔开；
     *             GPS：GPS的分类，如GPSONE、GPS等；
     *             TD: 支持的频段信息，如2000， 2300，1900等；
     *             NR: 支持的频段信息；
     */
    { AT_CMD_FEATURE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryFeaturePara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SFEATURE", VOS_NULL_PTR },
#endif

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置HSDPA能力等级
     * [说明]: 设置HSDPA能力类型。若产品不支持直接返回ERROR。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^DPACAT=<rate>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DPACAT?
     *     [结果]: <CR><LF>^DPACAT: <rate> <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <rate>: 速率等级，长度为一个字节。
     *             0：支持速率为3.6M；
     *             1：支持速率为7.2M；
     *             2：支持速率为1.8M；
     *             3：支持速率为14.4M；
     *             4：支持速率为21M。
     * [示例]:
     *     · 设置HSDPA能力类型成功
     *       AT^DPACAT=0
     *       OK
     */
    { AT_CMD_DPACAT,
      At_SetDpaCat, AT_NOT_SET_TIME, At_QryDpaCat, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DPACAT", VOS_NULL_PTR },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置HSPA支持类型
     * [说明]: 设置HSPA支持类型的接口。若产品不支持直接返回ERROR。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^HSSPT=<rrc_ver>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^HSSPT?
     *     [结果]: <CR><LF>^HSSPT: <rrc_ver> <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <rrc_ver>: RRC版本，长度为一个字节。
     *             0：支持WCDMA，不支持HSDPA/HSUPA功能；
     *             1：支持HSDPA+WCDMA，不支持HSUPA功能；
     *             2：支持WCDMA+HSDPA+HSUPA功能；
     *             6：支持WCDMA+HSPA+HSPA plus。
     *     <err>: 0：数据保护未解除，操作失败；
     *             1：其它设置错误。
     * [示例]:
     *     · 设置HSPA类型成功
     *       AT^HSSPT=0
     *       OK
     */
    { AT_CMD_HSSPT,
      AT_SetHsspt, AT_NOT_SET_TIME, AT_QryHsspt, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HSSPT", (VOS_UINT8 *)"(0,1,2,6)" },
#endif

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 设置GPRS定时器
     * [说明]: 设置GPRS定时器。若产品不支持直接返回ERROR。
     * [语法]:
     *     [命令]: ^GTIMER=<tgprs>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^GTIMER?
     *     [结果]: <CR><LF>^GTIMER: <tgprs><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^GTIMER=?
     *     [结果]: <CR><LF>^GTIMER: (list of supported < tgprs >s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <tgprs>: GPRS的定时值，长度为四个字节，单位秒。
     * [示例]:
     *     · 设置GPRS定时器成功
     *       AT^GTIMER=1
     *       OK
     *     · 测试命令
     *       AT^GTIMER=?
     *       ^GTIMER: (0-429496728)
     *       ok
     */
    { AT_CMD_GTIMER,
      AT_SetGTimerPara, AT_NOT_SET_TIME, AT_QryGTimerPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GTIMER", (VOS_UINT8 *)"(0-429496728)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询NMR数据
     * [说明]: 该命令用来查询各制式下网络的NMR（Network Measurement Report）参数。
     *         CL模式下该命令不支持。
     * [语法]:
     *     [命令]: ^CNMR=<n>
     *     [结果]: <CR><LF>^CNMR: <indexn>,<index1>,<nmr data>
     *             <CR><LF>^CNMR: <indexn>,<index2>,<nmr data>
     *             ...
     *             ^CNMR: <indexn>,<indexn>,<nmr data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CNMR=?
     *     [结果]: <CR><LF>^CNMR: (list of supported <n>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 1：查询2G下的NMR数据；
     *             2：查询3G下的NMR数据；
     *             3：查询4G下的NMR数据。
     *     <indexn>: 指示查询结果中有多少条nmr数据。
     *             2G下取值范围为[0,15]
     *             3G下取值范围为[0,8]
     *             4G下取值范围为[0,1]
     *     <index1>-<indexn-1>: 指示当前上报的nmr数据的索引值。
     *             2G下为1上报的是服务小区NMR信息，为2上报的是第一个邻区NMR信息，为3是第二个邻区NMR信息，以此类推，最多上报14个邻区NMR信息。
     *             3G下为1上报的是第一个频点的NMR信息，为2上报的是第二个频点的NMR信息，以此类推，最多上报8个频点NMR信息。
     *             4G为1上报服务小区NMR信息。
     *     <NMR Data>: 指示index对应的nmr数据，不带引号的2byte的16进制数的字符串。
     *             各制式下NMR数据格式不同，2G格式见表3-1中的字节5-8；3G格式见表3-2中的字节5-796；4G格式见表3-3中的字节1-20。
     * [表]: 2G数据格式
     *       描述,                               长度,
     *       小区绝对频点号，取值范围[0,1023]。, 2,
     *       小区BSIC，取值范围[0,63]。,         1,
     *       小区测量电平，取值范围[0,63]。,     1,
     * [表]: 3G数据格式
     *       描述,                                                               长度,
     *       第1个bit位：第n个频点是否有frequencyInfo
     *       第2个bit位：第n个频点是否有utra-CarrierRSSI
     *       第3个bit位：第n个频点是否有cellMeasuredResultsList,                 1,
     *       预留。,                                                             3,
     *       第n个频点的requencyInfo是WCDMA模式还是TD_WCDMA。,                   4,
     *       第1个bit位：WCDMA模式的第n个频点的requencyInfo中上行频点是否存在。, 1,
     *       预留。,                                                             3,
     *       上行频点。,                                                         2,
     *       下行频点。,                                                         2,
     *       第n个频点的RSSI。,                                                  4,
     *       第n个频点下的小区个数。,                                            4,
     *       第1个bit位：第n个频点下的第一个小区是否有cellid。,                  1,
     *       预留。,                                                             3,
     *       第n个频点下的第一个小区的cellid。,                                  4,
     *       第n个频点下的小区是WCDMA还是TD-WCDMA小区。,                         4,
     *       第1个bit位：第n个频点的第一个小区是否有ecn0。
     *       第2个bit位：第n个频点是否有Rscp。
     *       第3个bit位：第n个频点是否有pathloss。,                              1,
     *       预留。,                                                             3,
     *       第n个频点的第一个小区的扰码primary CPICH-Info。,                    2,
     *       第n个频点的第一个小区的ecn0。,                                      1,
     *       第n个频点的第一个小区的Rscp。,                                      1,
     *       第n个频点的第一个小区的pathloss。,                                  1,
     *       预留。,                                                             3,
     *       第n个频点的第2个小区信息。,                                         24,
     *       …,                                                                 ,
     *       第n个频点的第32个小区信息。,                                        24,
     * [表]: 4G数据格式
     *       字节,  描述,         长度,
     *       1-8,   CellGlobalID, 8,
     *       9-12,  CELLID,       4,
     *       13-14, TAC,          2,
     *       15-16, physCellId,   2,
     *       17-18, RSRP,         2,
     *       19-20, RSRQ,         2,
     *       21-22, TA,           2,
     *       23-24, 补齐,         2,
     * [示例]:
     *     · 驻留2G，服务小区的绝对频点号为0x00b2（178）、BSIC为0x30（48）、测量电平为0x3f（63），邻区的绝对频点号为0x0041（65），BSIC为0x39（57），测量电平为0x3e（62）。NMR数据查询显示如下：
     *       AT^CNMR=1
     *       ^CNMR: 2,1,b200303f
     *       ^CNMR: 2,2,4100393e
     *       OK
     *     · 测试命令
     *       AT^CNMR=?
     *       ^CNMR: (1,2,3)
     *       OK
     */
    { AT_CMD_CNMR,
      At_SetCnmrPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CNMR", (VOS_UINT8 *)"(1,2,3)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 灭屏/亮屏指示重选
     * [说明]: 该命令用于AP侧通知L和W进行重选。
     * [语法]:
     *     [命令]: ^WLTHRESHOLDCFG=<OffOnFlg>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^WLTHRESHOLDCFG=?
     *     [结果]: <CR><LF>^WLTHRESHOLDCFG: (list of supported < OffOnFlg >s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <OffOnFlg>: 0：亮屏；
     *             1：灭屏。
     * [示例]:
     *     · 点亮屏幕
     *       AT^WLTHRESHOLDCFG=0
     *       OK
     *     · 测试命令
     *       AT^WLTHRESHOLDCFG=?
     *       ^WLTHRESHOLDCFG: (0,1)
     *       OK
     */
    { AT_CMD_WLTHRESHOLDCFG,
      At_SetWlthresholdcfgPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^WLTHRESHOLDCFG", (VOS_UINT8 *)"(0,1)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: TRX TAS设置命令
     * [说明]: 本命令用于设置TRX的TAS控制，目前只开发了NR模；
     *         注：这个命令在非信令下发^FTXON（打开TX）之后才能正常执行。
     * [语法]:
     *     [命令]: ^TRXTAS=<mode>,<rat>,<cmd>[,<tas_value>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <mode>: 0：信令模式
     *             1：非信令模式
     *     <rat>: 0：GSM
     *             1：WCDMA
     *             2：LTE
     *             3：TDSCDMA
     *             5：NR
     *             说明：
     *             目前信令和非信令下都只支持NR；
     *     <cmd>: 0：表示关算法 (非信令不支持)；
     *             1：表示配置参数，需要带<tas_value>参数；
     *             2：表示开算法，恢复手机原有的trx tas算法(非信令不支持)；
     *     <tas_value>: 整型值，透传的trxtas参数 ,使用UINT32的最后5bit:  2bit UE能力（5bit中的高位2bit）：0表示1T2R，1表示1T4R，2表示2T4R  1bit TX索引：0表示TX0，1表示TX1  2bit 天线port号：0、1、2、3。表示指定的TX索引切到哪个port。
     *     <err_code>: 错误码：
     *             1: 非信令下没有打开TX；
     *             2: 非信令下<cmd>参数只参设置为1，非信令下没有开关算法；
     *             3: <rat>参数错误；
     *             4: 非信令下，^TRXTAS与^FCHAN的RAT不匹配；
     *             50: 参数错误
     * [示例]:
     *     · 关闭算法:
     *       at^trxtas=0,5,0
     *       1T2R切到port0：at^trxtas=0,5,1,0
     *       1T2R切到port1：at^trxtas=0,5,1,1
     *       1T4R切到port0：at^trxtas=0,5,1,8
     *       1T4R切到port1：at^trxtas=0,5,1,9
     *       1T4R切到port2：at^trxtas=0,5,1,10
     *       1T4R切到port3：at^trxtas=0,5,1,11
     *       2T4R TX0切到port0：at^trxtas=0,5,1,16
     *       2T4R TX0切到port1：at^trxtas=0,5,1,17
     *       2T4R TX1切到port2：at^trxtas=0,5,1,22
     *       2T4R TX1切到port3：at^trxtas=0,5,1,23
     *     · 恢复算法:
     *       at^trxtas=0,5,2
     *       说明：
     *       1T2R，只有TX0，在port 0 1之间切
     *       1T4R，只有TX0，在port 0 1 2 3之间切
     *       2T4R，TX0在port 0 1之间切，TX1在port 2 3之间切
     *       NSA的时候使用的是TX0（和射频通路的TX0/1无关）
     */
    { AT_CMD_TRXTAS,
      At_SetTrxTasPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TRXTAS", (VOS_UINT8 *)"(0,1),(5),(0-2),(0-4294967295)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: TRX TAS查询命令
     * [说明]: 本命令用于TRX的TAS控制查询，目前只开发了NR的信令模式；
     *         注：非信令不支持查询；
     * [语法]:
     *     [命令]: ^TRXTASQRY=<mode>,<rat>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>^TRXTASQRY: <rat>,<tas_value>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <mode>: 0：信令模式
     *             1：非信令模式
     *     <rat>: 0：GSM
     *             1：WCDMA
     *             2：LTE
     *             3：TDSCDMA
     *             5：NR
     *     <tas_value>: 查询到的TRXTAS参数；
     *             2bit UE能力（6bit中的高位2bit）：0表示1T2R，1表示1T4R，2表示2T4R，3表示无效值  2bit TX0天线port号：0、1、2、3。表示TX0在哪个port  2bit TX1天线port号：0、1、2、3。表示TX1在哪个port
     * [示例]:
     *     · 查询NR下非信令模式下的TRXTAS值：
     *       AT^TRXTASQRY=1,5
     *       ^TRXTASQRY: 0
     *       OK
     */
    { AT_CMD_TRXTASQRY,
      At_QryTrxTasPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TRXTASQRY", (VOS_UINT8 *)"(0,1),(5)" },
#endif
};

/* 注册AS装备AT命令表 */
VOS_UINT32 AT_RegisterDeviceAsCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDeviceAsCmdTbl, sizeof(g_atDeviceAsCmdTbl) / sizeof(g_atDeviceAsCmdTbl[0]));
}

