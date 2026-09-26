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
#include "at_custom_lnas_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_lnas_set_cmd_proc.h"
#include "at_custom_lnas_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_LNAS_CMD_TBL_C

static const AT_ParCmdElement g_atCustomLnasCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置和查询预置频点频段信息
     * [说明]: ^MCC上报之后，AP下发该MCC对应的预置频点频段信息。
     * [语法]:
     *     [命令]: ^MCCFREQ=<SEQ>,<VER>,<OPERATION>,<LENGTH>[[,<BS1>[,<BS2>[,<BS3>]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^MCCFREQ?
     *     [结果]: <CR><LF>^MCCFREQ: <VER><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^MCCFREQ=?
     *     [结果]: <CR><LF>^MCCFREQ: (list of supported <SEQ>s),(str),(list of supported <OPERATION>s),(list of supported <LENGTH>s),(str),(str),(str)<CR><LF><CR>
     *             <LF>OK<CR><LF>
     * [参数]:
     *     <SEQ>: 流水号：取值范围为1至255，最后一条为255，若只有一条，则为255
     *     <VER>: 云通讯匹配频点/频段版本号
     *     <OPERATION>: 0: 新增
     *             1: 刪除全部
     *             2: 刪除某MCC
     *             删除全部的时候，长度需要设置为0，且不能包含BS1、BS2、BS3。
     *     <LENGTH>: 码流参数（BS1，BS2和BS3）的总长度，长度范围0~1500，BS1和BS2,BS3可能有，也可能没有。如果LENGTH长度不等于码流参数（BS1，BS2和BS3）的总长度，直接返回失败。
     *     <BS1>: 码流参数1，十六进制字节码流，两个十六进制字符码表示一个字节;长度不超过500
     *     <BS2>: 码流参数2，同<BS1>
     *     <BS3>: 码流参数3，同<BS1>
     * [示例]:
     *     · 新增预置频点频段信息
     *       AT^MCCFREQ=255,"00.00.001",0,690,"64F004FF0005030001017C02000000020000080002015E0000000100000001FF030000FC030000260003040C9400000C94000004D2940000D294000004D4940000D494000027000504CE950000CE95000004009600000096000004049600000496000004649600006496000004909600009096000028000304269800002698000004EC980000EC980000045299000052990000FF1004010006028E2900008E29000002A7290000A729000002C0290000C029000002D9290000D929000002F2290000F2290000020B2A00000B2A000003000201AE0200007C020000047206000072060000080002017C0000005F000000020D0C00000D0C000029","000104949D0000949D0000FF3002010002044B0000004B000000046400000064000000030001042107000021070000FF1103010002044B0000004B00000004640000006400000003000104210700002107000029000104B4A00000B4A00000"
     *       OK
     *     · 删除所有预置频点频段信息
     *       AT^MCCFREQ=255,"00.00.001",1,0
     *       OK
     *     · 查询命令
     *       AT^MCCFREQ?
     *       ^MCCFREQ: 00.00.001
     *       OK
     *     · 测试命令
     *       AT^MCCFREQ=?
     *       ^MCCFREQ: (1-255),(str),(0-2),(0-1500),(str),(str),(str)
     *       OK
     */
    { AT_CMD_MCCFREQ,
      AT_SetMccFreqPara, AT_SET_PARA_TIME, AT_QryMccFreqPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^MCCFREQ", (TAF_UINT8 *)"(1-255),(str),(0-2),(0-1500),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置云通信相关信息
     * [说明]: AT转发AP下发给CSS模块的云通信相关数据信息。
     * [语法]:
     *     [命令]: ^CLOUDDATA=<LENGTH>[[,<BS1>[,<BS2>[,<BS3>]]]]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <LENGTH>: 码流参数（BS1，BS2和BS3）的总长度，长度范围0~1500， BS2,BS3可能有，也可能没有。参数BS1，BS2，BS3需保证后者有码流填充时，前者必须有码流填充。例如：不能出现BS1没有数据，而后面又填充了BS2、或者BS3等类似情况。如果LENGTH长度不等于码流参数（BS1，BS2和BS3）的总长度，直接返回失败。若长度为0，则表示没有码流，直接返回失败。
     *     <BS1>: 码流参数1，ASCII码流，单个参数长度不超过500。
     *     <BS2>: 码流参数2，同<BS1>
     *     <BS3>: 码流参数3，同<BS1>
     * [示例]:
     *     · 设置云通信相关信息
     *       AT^CLOUDDATA=112,"00000000060000000000000000FF30312E30322E3030330406000001000F0001020200041200230045000000560000002301000034020000"
     *       OK
     *     · 测试命令
     *       AT^CLOUDDATA=?
     *       ^CLOUDDATA: (0-1500),(str),(str),(str)
     *       OK
     */
    { AT_CMD_CLOUDDATA,
      AT_SetCloudDataPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CLOUDDATA", (TAF_UINT8 *)"(0-1500),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置和查询云通信受限小区信息
     * [说明]: ^REPORTBLOCKCELLMCC主动上报之后，AP下发该MCC对应的云通信受限小区信息。
     *         注意：该命令是否支持受NV（52008）控制。
     * [语法]:
     *     [命令]: ^BLOCKCELLLIST=<SEQ>,<VER>,<OPERATION>,<SYSMODE>,<LENGTH>[[,<BS1>[,<BS2>[,<BS3>]]]]
     *             <CR><LF>OK<CR><LF>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^BLOCKCELLLIST?
     *     [结果]: <CR><LF>^BLOCKCELLLIST: <VER>[,<MCC>,<MCC>…..]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *             ---备注:为了验证版本的是否合入了该功能,即使modem NV功能关闭,modem侧也会返回成功,版本号是00.00.000
     *     [命令]: ^BLOCKCELLLIST=?
     *     [结果]: <CR><LF>^BLOCKCELLLIST: (list of supported <SEQ>s),(str),(list of supported <OPERATION>s),(list of supported <SYSMODE>s),(list of supported  <LENGTH>s),(str),(str),(str)<CR><LF><CR <LF>OK<CR><LF>
     * [参数]:
     *     <SEQ>: 流水号：取值范围为1至255，最后一条为255;，若只有一条
     *             则为255
     *     <VER>: 云通讯匹配版本号
     *     <OPERATION>: 0: 新增
     *             1: 刪除全部。
     *     <SYSMODE>: 系统制式。
     *             0：GSM；
     *             1：WCDMA；
     *             2：LTE;
     *             现在默认仅适用GSM
     *     <LENGTH>: 码流参数（BS1，BS2和BS3）的总长度，长度范围0~1500，
     *             BS1和BS2,BS3可能有，也可能没有。如果LENGTH长度不等于码流参数（BS1，BS2和BS3）的总长度，直接返回失败。删除全部的时候，长度需要设置为0，且不能包含BS1、BS2、BS3。
     *     <BS1>: 码流参数1，十六进制字节码流，两个十六进制字符码表示一个字节;长度不超过500; 小端模式:比如int32的值0x01020304下发的码流是04030201
     *     <BS2>: 码流参数2，同<BS1>
     *     <BS3>: 码流参数3，同<BS1>
     *     <MCC>: MCC信息，可能一个，也可能多个（目前最多17个），多个时以”逗号”隔开
     * [示例]:
     *     · 新增云通信受限小区信息, 仅用一个BS1就可以存下
     *       AT^BLOCKCELLLIST=255,"00.00.001",0,0,34,"0203000000020F000102109E519B00250A"
     *       OK
     *     · 新增云通信受限小区信息, 需要两个或三个BS
     *       AT^BLOCKCELLLIST=255,"00.00.111",0,0,690,"0406000000010F002A6037AD00AE00AF0A6137AE00AF00B00A6237AF00B000B10A6337B000B100B20A6437B100B200B30A6537B200B300B40A6637B300B400B50A6737B400B500B60A6837B500B600B70A6937B600B700B80A6A37B700B800B90A6B37B800B900BA0A6C37B900BA00BB0A6D37BA00BB00BC0A6E37BB00BC00BD0A6F37BC00BD00BE0A7037BD00BE00BF0A7137BE00BF00C00A7237BF00C000C10A7337C000C100C20A7437C100C200C30A7537C200C300C40A7637C300C400C50A7737C400C500C60A7837C500C600C70A7937C600C700C80A7A37C700C800C90A7B37C800C900CA0A7C37C900CA00CB0A7D37CA00CB00CC0A7E","37CB00CC00CD0A7F37CC00CD00CE0A8037CD00CE00CF0A8137CE00CF00D00A8237CF00D000D10A8337D000D100D20A8437D100D200D30A8537D200D300D40A8637D300D400D50A8737D400D500D60A8837D500D600D70A8937D600D700D80A"
     *       OK
     *     · 删除全部云通信受限小区信息
     *       AT^BLOCKCELLLIST=255,"00.00.000",1,0,0
     *       OK
     *     · 查询云通信受限小区信息
     *       AT^BLOCKCELLLIST?
     *       ^BLOCKCELLLIST: "00.00.001",460,230
     *       OK
     *     · 测试命令
     *       AT^BLOCKCELLLIST=?
     *       ^BLOCKCELLLIST: (1-255),(str),(0-1),(0-2),(0-1500),(str),(str),(str)
     *       OK
     */
    { AT_CMD_BLOCKCELLLIST,
      AT_SetBlockCellListPara, AT_SET_PARA_TIME, AT_QryBlockCellListPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^BLOCKCELLLIST", (VOS_UINT8 *)"(1-255),(str),(0-1),(0-2),(0-1500),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: AP侧GPS定位功能开关配置
     * [说明]: 该命令用于将AP侧GPS定位开关状态通知到CP侧。
     * [语法]:
     *     [命令]: ^GPSLOCSET=<loc_permit>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^GPSLOCSET=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <loc_permit>: 整型值，AP侧GPS定位开关状态：
     *             0：关闭定位能力；
     *             1：开启定位能力（默认值）。
     * [示例]:
     *     · 开启定位能力
     *       AT^GPSLOCSET=1
     *       OK
     */
    { AT_CMD_GPSLOCSET,
      AT_SetGpsLocSetPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^GPSLOCSET", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_GAMEMODE,
      AT_SetGameModePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^GAMEMODE", (VOS_UINT8 *)"(0,1)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置云通讯布网策略
     * [说明]: 该命令用来设置默认布网策略以及对于特定PLMN的布网策略。
     *         本命令仅在支持NR能力的产品上可用。
     * [语法]:
     *     [命令]: ^NWDEPLOYMENT=
     *             <seq>,<version>,<BS_count>,<BS1>[,<BS2>[,<BS3>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <seq>: 无符号8位整型值，流水号，取值范围为1至255，最后一条为255，若只有一条，则为255；
     *     <version>: 字符串类型，云通讯布网策略版本号，固定格式为xx.xx.xxx；
     *     <BS_count>: 码流个数，取值范围为（1~3）；
     *     <BS1>: 码流参数1，字符串格式，长度不超过500；码流格式：<strategy>,<roamingFlag>,<PLMNNUM>[,<PLMNx>,……]；
     *     <BS2>: 码流参数2，字符串格式，长度不超过500，BS_count为2或3时，BS2才有效；BS2中不包含<strategy>,<roamingFlag>和<PLMNNUM>，是BS1中<PLMNx>的延续码流；
     *     <BS3>: 码流参数3，字符串格式，长度不超过500，BS_count为3时，BS3才有效；BS3中不包含<strategy>,<roamingFlag>和<PLMNNUM>，是BS2中<PLMNx>的延续码流；
     *     <strategy>: 布网策略类型，一个BS1只承载一个策略类型，一个策略类型由于PLMN个数超过AT命令可下发的总长度原因可以拆分成多条AT命令，策略类型取值如下：
     *             0：LTE_ONLY
     *             1：NSA_ONLY
     *             2：SA_PREFER
     *             3：NSA_PREFER
     *     <roamingFlag>: 表示布网策略是否用于漫游：
     *             0：该布网策略类型非漫游
     *             1：该布网策略类型用于漫游
     *     <PLMNNUM>: 布网策略类型所包含的PLMN个数，即有多少PLMN使用该布网策略。如果该值为0，则说明<strategy>设置的是默认的布网策略类型；
     *     <PLMNx>: 布网策略类型所包含的PLMN，仅在<PLMNNUM>不为0时才可以设置。
     * [示例]:
     *     · 只设置漫游默认策略为SA_PREFER:
     *       AT^NWDEPLOYMENT=255,"10.10.141",1,"2,1,0"
     *       OK
     *     · 只设置非漫游默认策略为SA_PREFER:
     *       AT^NWDEPLOYMENT=255,"10.10.141",1,"2,0,0"
     *       OK
     *     · 设置漫游默认策略为SA_PREFER，并设置漫游PLMN 46010为NSA_ONLY:
     *       AT^NWDEPLOYMENT=1,"10.10.666",1,"2,1,0"
     *       OK
     *       AT^NWDEPLOYMENT=255,"10.10.666",1,"1,1,1,46010"
     *       OK
     *     · 设置非漫游默认策略为SA_PREFER，并设置非漫游PLMN 46010为NSA_ONLY，非漫游PLMN 46020和46030为NSA_PREFER，非漫游PLMN 460510为LTE_ONLY:
     *       AT^NWDEPLOYMENT=1,"10.10.666",1,"2,0,0"
     *       OK
     *       AT^NWDEPLOYMENT=2,"10.10.666",1,"1,0,1,46010"
     *       OK
     *       AT^NWDEPLOYMENT=3,"10.10.666",1,"3,0,2,46020,46030"
     *       OK
     *       AT^NWDEPLOYMENT=255,"10.10.666",1,"0,0,1,460510"
     *       OK
     */
    { AT_CMD_NWDEPLOYMENT,
      AT_SetNwDeploymentPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NWDEPLOYMENT", (VOS_UINT8 *)"(1-255),(str),(1-3),(str),(str),(str)" },
#endif

#if (FEATURE_LTE == FEATURE_ON)
    { AT_CMD_ISMCOEX,
      AT_SetIsmCoexPara, AT_SET_PARA_TIME, AT_QryIsmCoexPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^ISMCOEX", (TAF_UINT8 *)"(@bw),(@bw),(@bw),(@bw),(@bw),(@bw)" },
#endif

#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_LTECAT,
    VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryLteCatPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8 *)"^LTECAT", VOS_NULL_PTR },
#endif

#if (FEATURE_APN_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置APN backoff定时器上报状态的功能开关
     * [说明]: 控制是否使能+CABTSRI命令主动上报。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、modem2暂不支持，当从modem2发送此命令时，直接返回error。
     *         2、此命令受FEATURE_APN_BASED_BO_TIMER_PARA_RPT宏控制。
     * [语法]:
     *     [命令]: +CABTSR=[<n>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CABTSR?
     *     [结果]: <CR><LF>+CABTSR: <n><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +CABTSR=?
     *     [结果]: <CR><LF>+CABTSR: (list of supported <n>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整数类型，标志是否使能+CABTSRI主动上报，默认为0，取值如下：
     *             0：关闭CABTSRI主动上报；
     *             1：开启CABTSRI主动上报。
     * [示例]:
     *     · 不带参数，默认不开启+CABTSRI主动上报功能
     *       AT+CABTSR=
     *       OK
     *     · 开启+CABTSRI主动上报功能
     *       AT+CABTSR=1
     *       OK
     */
    { AT_CMD_CABTSR,
      At_SetCabtsrPara, AT_SET_PARA_TIME, At_QryCabtsrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CABTSR", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 读取APN backoff定时器相关参数
     * [说明]: 用于查询THROT上报的APN backoff定时器相关信息。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、modem2暂不支持，当从modem2发送此命令时，直接返回error；
     *         2、此命令受FEATURE_APN_BASED_BO_TIMER_PARA_RPT宏控制。
     * [语法]:
     *     [命令]: +CABTRDP[=<apn>]
     *     [结果]: 执行设置成功时：
     *             当无任何参数上报时：
     *             <CR><LF>OK<CR><LF>
     *             当有参数时：
     *             <CR><LF>+CABTRDP: <apn>[,<residual_backoff_time>][,<re-attempt_rat_indicator>[,<re-attempt_eplmn_indicator>[,<NSLPI>[,<procedure>]]]][<CR><LF>+CABTRDP: <apn>[,<residual_backoff_time>][,<re-attempt_rat_indicator>[,<re-attempt_eplmn_indicator>[,<NSLPI>[,<procedure>]]]][...]]<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CABTRDP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <apn>: APN信息。字符串类型，最大长度为99。当回复上报的apn为空字符时，输出逗号。
     *     <residual_backoff_time>: 该参数表示backoff定时器剩余时长，单位是秒。整数类型。
     *             注：
     *             当定时器处于deactivated 状态时，此值省略，输出逗号；
     *             当定时器处于停止或者超时状态时，此值为0。
     *     <re-attempt_rat_indicator>: 异系统是否可以重新发起会话管理流程。整数类型，取值如下：
     *             0：异系统后，允许重新发起会话管理流程；
     *             1：异系统后，不允许重新发起会话管理流程。
     *     <re-attempt_eplmn_indicator>: eplmn内是否可以发起会话管理流程。整数类型，取值如下：
     *             0: 允许eplmn内是否可以发起会话管理流程;
     *             1: 不允许eplmn内是否可以发起会话管理流程。
     *     <NSLPI>: 指示信令的优先级。整数类型，取值如下：
     *             0：配置为信令低优先级;
     *             1：没有配置为信令低优先级。
     *     <procedure>: 指示受到backoff定时器影响的会话管理流程。整数类型，取值如下：
     *             0: backoff定时器适用所有流程；
     *             1: backoff定时器适用STAND ALONE PDN流程；
     *             2: backoff定时器适用BEARER ALLOC流程；
     *             3: backoff定时器适用BEARER MODIFY流程；
     *             4: backoff定时器适用PDP_ACTIVE流程；
     *             5: backoff定时器适用SECOND_PDP_ACTIVE流程；
     *             6: backoff定时器适用PDP_MODIFY流程；
     *             7: backoff定时器适用PDU_ESTABLISH流程；
     *             8: backoff定时器适用PDU_MODIFY流程；
     *             9: backoff定时器适用ATTACH和STAND ALONE的PDN流程。
     * [示例]:
     *     · APN上有backoff定时器在运行，存在多组APN bakeoff信息时
     *       AT+CABTRDP
     *       +CABTRDP: "abc",100,1,1,1,0
     *       +CABTRDP: "abcD",115,1,1,1,0
     *       OK
     *     · APN上有backoff定时器，定时器deactivated时
     *       AT+CABTRDP
     *       +CABTRDP: "abcD",,0,1,1,1
     *       OK
     *     · backoff定时器停止且仅有一组apn信息时
     *       AT+CABTRDP
     *       +CABTRDP: "abcd",0,,,,
     *       OK
     */
    { AT_CMD_CABTRDP,
      At_SetCabtrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CABTRDP", (VOS_UINT8 *)"(str)" },
#endif

#if (FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置基于S-NSSAI的back-off定时器参数上报状态的功能开关
     * [说明]: 控制是否使能+CSBTSRI命令主动上报。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、此命令受FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT宏控制
     *         2、modem2暂不支持，当从modem2发送此命令时，直接返回error
     * [语法]:
     *     [命令]: +CSBTSR=[<n>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CSBTSR?
     *     [结果]: <CR><LF>+CABTSR: <n><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +CSBTSR=?
     *     [结果]: <CR><LF>+CSBTSR: (list of supported <n>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整数类型，标志是否使能+CSBTSRI主动上报，默认为0，取值如下：
     *             0：关闭CSBTSRI主动上报；
     *             1：开启CSBTSRI主动上报。
     * [示例]:
     *     · 不带参数，默认不开启+CABTSRI主动上报功能
     *       AT+CSBTSR=
     *       OK
     *     · 开启+CSBTSRI主动上报功能
     *       AT+CSBTSR=1
     *       OK
     */
    { AT_CMD_CSBTSR,
      At_SetCsbtsrPara, AT_SET_PARA_TIME, At_QryCsbtsrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CSBTSR", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 读取APN backoff定时器相关参数
     * [说明]: 用于查询THROT上报的S-NSSAI backoff定时器相关信息。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、此命令受FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT和FEATURE_UE_MODE_NR宏控制；
     *         2、modem2暂不支持，当从modem2发送此命令时，直接返回error。
     * [语法]:
     *     [命令]: +CSBTRDP[=<S-NSSAI>]
     *     [结果]: 执行设置成功时：
     *             当无任何参数上报时：
     *             <CR><LF>OK<CR><LF>
     *             当有参数时：
     *             <CR><LF>+CSBTRDP: <S-NSSAI>[,<S-NSSAI_backoff_time>][,<re-attempt_rat_indicator>[,<re-attempt_eplmn_indicator>[,<procedure>]]][<CR><LF>+CSBTRDP: <S-NSSAI>[,<S-NSSAI_backoff_time>][,<re-attempt_rat_indicator>[,<re-attempt_eplmn_indicator>[,<procedure>]]][...]]<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CSBTRDP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <S-NSSAI>: 5GS的分片信息，字符串类型。
     *                切片格式如下所示：
     *                1、sst
     *                2、sst.sd
     *                3、sst;mappedSst
     *                4、sst.sd;mappedSst
     *                5、sst.sd;mappedSst.mappedSd
     *                注意：字符串中的数据为16进制，字符取值范围：0-9，A-F,a-f。
     *     <S-NSSAI_backoff_time>: 该参数表示S-NSSAI backoff定时器剩余时长，单位是秒。整数类型。
     *             注：
     *             当定时器处于deactivated 状态时，此值省略，输出逗号；
     *             当定时器处于停止或者超时状态时，此值为0。
     *     <re-attempt_rat_indicator>: 异系统是否可以重新发起会话管理流程。整数类型，取值如下：
     *             0：异系统后，允许重新发起会话管理流程；
     *             1：异系统后，不允许重新发起会话管理流程。
     *     <re-attempt_eplmn_indicator>: eplmn内是否可以发起会话管理流程。整数类型，取值如下：
     *             0: 允许eplmn内是否可以发起会话管理流程;
     *             1: 不允许eplmn内是否可以发起会话管理流程。
     *     <procedure>: 指示受到T3585定时器影响的会话管理流程。整数类型，取值如下：
     *             0: backoff定时器适用所有流程；
     *             1:定时器适用PDU_ESTABLISH流程；
     *             2:定时器适用PDP_MODIFY流程。
     * [示例]:
     *     · 查询到S-NSSAI backoff定时器在运行
     *       AT+CSBTRDP
     *       +CSBTRDP: ,160,0,1,0
     *       OK
     *     · 查询到有S-NSSAI backoff定时器处于deactivated状态
     *       AT+CSBTRDP
     *       +CSBTRDP: ,,0,1,0
     *       OK
     *     · 查询到S-NSSAI backoff定时器停止
     *       AT+CSBTRDP
     *       +CSBTRDP: ,0,,,
     *       OK
     */
    { AT_CMD_CSBTRDP,
      At_SetCsbtrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CSBTRDP", (VOS_UINT8 *)"(dnn)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置基于S-NSSAI和DNN的back-off定时器参数上报状态的功能开关
     * [说明]: 控制是否使能+CSDBTSRI命令主动上报。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、此命令受FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT宏控制
     *         2、modem2暂不支持，当从modem2发送此命令时，直接返回error
     * [语法]:
     *     [命令]: +CSDBTSR=[<n>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CSDBTSR?
     *     [结果]: <CR><LF>+CABTSR: <n><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +CSDBTSR=?
     *     [结果]: <CR><LF>+CSDBTSR: (list of supported <n>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整数类型，标志是否使能+CSDBTSRI主动上报，默认为0，取值如下：
     *             0：关闭CSDBTSRI主动上报；
     *             1：开启CSDBTSRI主动上报。
     * [示例]:
     *     · 不带参数，默认不开启+CABTSRI主动上报功能
     *       AT+CSDBTSR=
     *       OK
     *     · 开启+CSDBTSRI主动上报功能
     *       AT+CSDBTSR=1
     *       OK
     */
    { AT_CMD_CSDBTSR,
      At_SetCsdbtsrPara, AT_SET_PARA_TIME, At_QryCsdbtsrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CSDBTSR", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 读取APN backoff定时器相关参数
     * [说明]: 用于查询THROT上报的基于S-NSSAI和DNN的backoff定时器相关信息。
     *         本命令遵从3GPP TS 27.007协议。
     *         注：
     *         1、此命令受FEATURE_NSSAI_BASED_BO_TIMER_PARA_RPT和FEATURE_UE_MODE_NR宏控制；
     *         2、modem2暂不支持，当从modem2发送此命令时，直接返回error；
     *         3、本命令遵从3GPP TS 27.007协议，但只携带一个参数的情况不存在，当前协议描述有误，请以手册为准。at+CSDBTRDP=,等价于at+CSDBTRDP="",""
     * [语法]:
     *     [命令]: +CSDBTRDP[=<S-NSSAI>,<DNN>]
     *     [结果]: 执行设置成功时：
     *             当无任何参数上报时：
     *             <CR><LF>OK<CR><LF>
     *             当有参数时：
     *             <CR><LF>+CSDBTRDP: <S-NSSAI>,<DNN>[,<S-NSSAI_DNN_backoff_time>][,<re-attempt_rat_indicator>[,<re-attempt_eplmn_indicator>[,<procedure>]]][<CR><LF>+CSDBTRDP: <S-NSSAI>,<DNN>[,<S-NSSAI_DNN_backoff_time>][,<re-attempt_rat_indicator>[,<re-attempt_eplmn_indicator>[,<procedure>]]][...]]<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: +CSDBTRDP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <S-NSSAI>: 5GS的分片信息，字符串类型。
     *                切片格式如下所示：
     *                1、sst
     *                2、sst.sd
     *                3、sst;mappedSst
     *                4、sst.sd;mappedSst
     *                5、sst.sd;mappedSst.mappedSd
     *                注意：字符串中的数据为16进制，字符取值范围：0-9，A-F,a-f。
     *     <DNN>: DNN信息。字符串类型，最大长度为99。
     *     <S-NSSAI_DNN_backoff_time>>: 该参数表示基于S-NSSAI和DNN的backoff定时器剩余时长，单位是秒。整数类型。
     *             注：
     *             当定时器处于deactivated 状态时，此值省略，输出逗号；
     *             当定时器处于停止或者超时状态时，此值为0。
     *     <re-attempt_rat_indicator>: 异系统是否可以重新发起会话管理流程。整数类型，取值如下：
     *             0：异系统后，允许重新发起会话管理流程；
     *             1：异系统后，不允许重新发起会话管理流程。
     *     <re-attempt_eplmn_indicator>: eplmn内是否可以发起会话管理流程。整数类型，取值如下：
     *             0: 允许eplmn内是否可以发起会话管理流程;
     *             1: 不允许eplmn内是否可以发起会话管理流程。
     *     <procedure>: 指示受到backoff定时器影响的会话管理流程。整数类型，取值如下：
     *             0: backoff定时器适用所有流程；
     *             1:定时器适用PDU_ESTABLISH流程；
     *             2:定时器适用PDP_MODIFY流程。
     * [示例]:
     *     · 查询到基于S-NSSAI和DNN的backoff定时器在运行
     *       AT+CSDBTRDP
     *       +CSDBTRDP: "01.010101","huawei.com",32,0,1,0
     *       OK
     *     · 查询到基于S-NSSAI和DNN的backoff定时器处于deactivated状态
     *       AT+CSDBTRDP="","huawei.com"
     *       +CSDBTRDP: "01.010101","huawei.com",,0,1,0
     *       OK
     *     · 查询到基于S-NSSAI和DNN的backoff定时器停止
     *       AT+CSDBTRDP
     *       +CSDBTRDP: ,"huawei.com",0,,,
     *       OK
     */
    { AT_CMD_CSDBTRDP,
      At_SetCsdbtrdpPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"+CSDBTRDP", (VOS_UINT8 *)"(snssai),(dnn)" },
#endif

};

VOS_UINT32 AT_RegisterCustomLnasCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomLnasCmdTbl, sizeof(g_atCustomLnasCmdTbl) / sizeof(g_atCustomLnasCmdTbl[0]));
}

