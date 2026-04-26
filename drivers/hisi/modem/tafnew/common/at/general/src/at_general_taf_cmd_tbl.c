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

#include "at_general_taf_cmd_tbl.h"
#include "at_general_taf_set_cmd_proc.h"
#include "at_general_taf_qry_cmd_proc.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_para_cmd.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atGeneralTafCmdTbl[] = {
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 使能/禁止主动上报
     * [说明]: 该命令在AP-Modem形态下用于控制AT命令的主动上报。每一个主动上报命令对应一个独立的Bit位，可按照需求独立控制每一个可控主动上报命令。可控命令默认全部主动上报。部分主动上报命令有自己单独的配置命令，需要同时配置。
     * [语法]:
     *     [命令]: ^CURC=<mode>[,<report_cfg>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CURC?
     *     [结果]: <CR><LF>^CURC: <mode>[,<report_flag>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CURC=?
     *     [结果]: <CR><LF>^CURC: (list of supported <mode>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，命令主动上报控制模式，取值0~2。一般默认为1，不同产品修改后默认值有可能不同。
     *             0：关闭表2-4中命令的主动上报；
     *             1：打开表2-4中命令的主动上报（默认值）；
     *             2：此模式需要参数<report_cfg>对表2-4中命令的主动上报进行配置。
     *     <report_cfg>: 16进制数字符串，命令主动上报标识。长度为8个字节，即每个Bit位对应一个主动上报AT命令，对应关系见表2-4。
     *             取值范围为0x0000000000000000~0xFFFFFFFFFFFFFFFF。每个Bit位的取值如下：
     *             0：关闭主动上报；
     *             1：打开主动上报。
     * [表]: 控制命令主动上报Bit位[63-0]与命令对应关系
     *       bit:   3,          2,          1,                                        0,             Oct0,
     *              ^SRVST,     ^REJINFO,   ^RSSI,                                    ^MODE,         Oct0,
     *       bit:   7,          6,          5,                                        4,             Oct0,
     *              ,           ^TIME,      ^SIMST,                                   ^PLMNSELEINFO, Oct0,
     *       bit:   11,         10,         9,                                        8,             Oct1,
     *              ^SMMEMFULL, ,           ,                                         ^ANLEVEL,      Oct1,
     *       bit:   15,         14,         13,                                       12,            Oct1,
     *              ,           ,           ,                                         ^ELEVATOR,     Oct1,
     *       bit:   19,         18,         17,                                       16,            Oct2,
     *              +CTZV,      ,           ,                                         ,              Oct2,
     *       bit:   23,         22,         21,                                       20,            Oct2,
     *              ,           ^DSFLOWRPT, ,                                         ,              Oct2,
     *       bit:   27,         26,         25,                                       24,            Oct3,
     *              ^CEND,      ^CONN,      ^CONF,                                    ^ORIG,         Oct3,
     *       bit:   31,         30,         29,                                       28,            Oct3,
     *              ,           ,           ^STIN、^CCIN、^CSIN、^CSMN、^CSTR、^SCEN, ,              Oct3,
     *       bit:   35,         34,         33,                                       32,            Oct4,
     *              ,           ,           ,                                         ,              Oct4,
     *       bit:   39,         38,         37,                                       36,            Oct4,
     *              ,           ,           ,                                         ,              Oct4,
     *       bit:   43,         42,         41,                                       40,            Oct5,
     *              ,           ,           ,                                         ,              Oct5,
     *       bit:   47,         46,         45,                                       44,            Oct5,
     *              ,           ,           ,                                         ,              Oct5,
     *       bit:   51,         50,         49,                                       48,            Oct6,
     *              ^ACINFO,    ^XLEMA,     ^LWURC,                                   ^CERSSI,       Oct6,
     *       bit:   55,         54,         53,                                       52,            Oct6,
     *              ,           ,           ^CALLSTATE,                               ^PLMN,         Oct6,
     *       bit:   59,         58,         57,                                       56,            Oct7,
     *              ,           ,           ,                                         ,              Oct7,
     *       bit:   63,         62,         61,                                       60,            Oct7,
     *              ,           ,           ,                                         ,              Oct7,
     * [示例]:
     *     · 关闭全部主动上报命令
     *       AT^CURC=0
     *       OK
     *     · 仅打开^RSSI的主动上报
     *       AT^CURC=2,0000000000000002
     *       OK
     *     · 查询当前主动上报命令状态
     *       AT^CURC?
     *       ^CURC: 1
     *       OK
     *       AT^CURC?
     *       ^CURC: 2,0x0000000000000002
     *       OK
     *     · 测试命令
     *       AT^CURC=?
     *       ^CURC: (0-2)
     *       OK
     */
    { AT_CMD_CURC,
      At_SetCurcPara, AT_NOT_SET_TIME, At_QryCurcPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CURC", (VOS_UINT8 *)"(0-2)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 控制设备形态切换
     * [说明]: 此命令仅在PID优化开关未使能时使用，该设置不适用手机。
     *         SET命令用于实现设备形态切换功能。手动重启单板后，命令设置才会生效。设置生效后单板将一直保持该设备形态，直到再次使用本命令将设备形态重新切换。
     *         READ命令用于查询当前的设备形态值。
     * [语法]:
     *     [命令]: ^U2DIAG=<val>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^U2DIAG?
     *     [结果]: <CR><LF>^U2DIAG: <val><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^U2DIAG=?
     *     [结果]: <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <val>: 整形值，表示PC侧出现modem、diag、PCUI、U盘（CDROM）、NDIS、PCSC、NEMA、SD、DVB等设备形态组合情况的取值。
     *             表2-5中描述了部分可能的设备形态取值，针对特定产品所支持的设备形态，请查阅产品的相关文档。
     *             说明：目前只有HILINK使用该命令，且val只能为118或者119，其它值按119处理。
     * [表]: <val>取值描述
     *       <val>, 设备形态描述,
     *       0,     Modem+DIAG+PCUI,
     *       1,     CDROM/Modem+PCUI+CDROM,
     *       2,     Modem+PCUI,
     *       5,     CDROM/DIAG+CDROM+PCUI+NDIS,
     *       6,     PCUI+NDIS,
     *       7,     PCUI+NDIS+DIAG,
     *       11,    CDROM/PCUI+CDROM+NDIS,
     *       12,    CDROM/Modem+DIAG+PCUI+CDROM,
     *       19,    CDROM/Modem+DIAG+PCUI+CDROM,
     *       20,    CDROM/Modem+NDIS+DIAG+PCUI+CDROM,
     *       25,    CDROM/Modem+PCUI,
     *       32,    CDROM/Modem+NDIS+PCUI+CDROM,
     *       36,    Modem+NDIS+DIAG+PCUI,
     *       41,    CDROM/Modem+NDIS+DIAG+PCUI,
     *       118,   SHELL+NDIS(调试模式),
     *       119,   NDIS(用户模式),
     *       257,   CDROM/Modem+PCUI+CDROM+SD,
     *       268,   CDROM/Modem+DIAG+PCUI+CDROM+SD,
     *       267,   CDROM/PCUI+CDROM+NDIS+SD,
     *       256,   Modem+DIAG+PCUI+SD,
     *       258,   Modem+PCUI+SD,
     *       262,   CDROM/PCUI+NDIS+SD,
     *       263,   CDROM/DIAG+PCUI+NDIS+SD,
     *       281,   CDROM/Modem+PCUI+SD,
     *       261,   CDROM/DIAG+CDROM+PCUI+NDIS+SD,
     *       276,   CDROM/Modem+NDIS+DIAG+PCUI+CDROM+SD,
     *       288,   CDROM/Modem+NDIS+PCUI+CDROM+SD,
     *       2316,  CDROM/Modem+OM+AT+CDROM+SD+ISHELL,
     *       4372,  CDROM/Modem+NDIS+DIAG+PCUI+CDROM+SD+BT,
     * [示例]:
     *     · 设置设备形态
     *       At^U2DIAG=118
     *       OK
     *     · 查询当前设备形态
     *       At^U2DIAG?
     *       ^U2DIAG: 118
     *       OK
     */
    { AT_CMD_U2DIAG,
      At_SetU2DiagPara, AT_SET_PARA_TIME, At_QryU2DiagPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_STICK | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^U2DIAG", (VOS_UINT8 *)"(0-4294967295)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设备形态切换
     * [说明]: 此命令仅在PID优化开关（NV:2601）使能时使用。
     *         对于E5形态，切换后的设备形态中不能带有MODEM设备形态(1)，切换后的第一个设备不能为MASS设备(A1或A2)。
     *         BalongV300R300对USB FIFO进行优化，最多只能支持8个interface（即8个USB设备），若FIFO大小无法支持切换出的设备个数，会导致USB枚举失败。
     *         该命令用于实现设备形态切换功能，设置后重启单板生效。
     *         READ命令用于查询当前生效的设备形态值。
     *         测试命令用于查询端口形态与具体字符（或子字符串）的对应关系。
     *         设置命令用字符串来表示一组端口形态，包含切换前和切换后的端口形态，它们之间以分号区分，端口形态与具体字符（或子字符串）对应关系如下：
     *         1：3G MODEM
     *         2：3G PCUI
     *         3：3G DIAG
     *         5：3G GPS
     *         10：4G MODEM
     *         12：4G PCUI
     *         13：4G DIAG
     *         14：4G GPS
     *         16：4G MODEM
     *         A：BLUE TOOTH
     *         A1：CDROM
     *         A2：SD
     *         A3：RNDIS
     *         对端口设置的约束如下：
     *         1）配置端口的字符或字符串非以上列举的或FF，均判断为非法，返回Error。
     *         2）切换前的端口配置第一位必须为A1(CDROM)，第二位如果有，仅允许有A2(SD)，其他端口出现在切换前的分号前时，一律判断为非法，返回Error。
     *         3）若需要设置为无切换前的端口形态，则切换前端口形态填写为FF。
     *         4）切换后的端口形态中必须有12(PCUI)，否则判断为非法端口形态，返回Error。
     *         5）若发现配置两个或多个相同的端口（如两个PCUI），则判为非法，返回Error。
     * [语法]:
     *     [命令]: ^SETPORT=<PORT_Type>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SETPORT?
     *     [结果]: <CR><LF>^SETPORT: <PORT_Type><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^SETPORT=?
     *     [结果]: <CR><LF>^SETPORT: A1: CDROM
     *             <CR><LF>^SETPORT: A2: SD
     *             <CR><LF>^SETPORT: A3: RNIDS
     *             <CR><LF>^SETPORT: A: BLUE TOOTH
     *             <CR><LF>^SETPORT: 1: 3G MODEM
     *             <CR><LF>^SETPORT: 2: 3G PCUI
     *             <CR><LF>^SETPORT: 3: 3G DIAG
     *             <CR><LF>^SETPORT: 5: 3G GPS
     *             <CR><LF>^SETPORT: 10: 4G MODEM
     *             <CR><LF>^SETPORT: 12: 4G PCUI
     *             <CR><LF>^SETPORT: 13: 4G DIAG
     *             <CR><LF>^SETPORT: 14: 4G GPS
     *             <CR><LF>^SETPORT: 16: NCM<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <PORT_Type>: 字符串类型，长度为0～50byte，描述切换前和切换后的端口形态。
     *             切换前和切换后的端口形态列表用分号分隔，各种端口形态之间用逗号分隔。
     * [示例]:
     *     · 当前端口形态为CDROM/Modem+PCUI，用户需要修改端口形态为CDROM+ SD /Modem+ DIAG +PCUI。
     *       AT^SETPORT="A1,A2;1,3,12"
     *       OK
     *     · 因为用户设置需要设备重启后生效，所以立刻查询得到的还是修改前的端口形态。
     *       AT^SETPORT?
     *       ^SETPORT: A1;1,12
     *       OK
     *     · 单板重启后切换前端口为CDROM+SD，切换后端口为Modem+ DIAG+PCUI。
     *       AT^SETPORT?
     *       ^SETPORT: A1,A2;1,3,12
     *       OK
     *     · 设置命令，无切换前端口形态，切换后端口为Modem+PCUI。
     *       AT^SETPORT="FF;1,12"
     *       OK
     *       查询当前端口形态
     *     · 切换前端口为CDROM+SD，切换后端口为Modem+ PCUI+DIAG
     *       AT^SETPORT?
     *       ^SETPORT: A1,A2;1,12,3
     *       OK
     *     · 测试命令
     *       AT^SETPORT=?
     *       ^SETPORT: A1: CDROM
     *       ^SETPORT: A2: SD
     *       ^SETPORT: A3: RNIDS
     *       ^SETPORT: A: BLUE TOOTH
     *       ^SETPORT: 1: 3G MODEM
     *       ^SETPORT: 2: 3G PCUI
     *       ^SETPORT: 3: 3G DIAG
     *       ^SETPORT: 5: 3G GPS
     *       ^SETPORT: 10: 4G MODEM
     *       ^SETPORT: 12: 4G PCUI
     *       ^SETPORT: 13: 4G DIAG
     *       ^SETPORT: 14: 4G GPS
     *       ^SETPORT: 16: NCM
     *       OK
     */
    { AT_CMD_PORT,
      At_SetPort, AT_SET_PARA_TIME, At_QryPort, AT_QRY_PARA_TIME, AT_TestSetPort, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SETPORT", (VOS_UINT8 *)"(Port)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 是否支持PCSC端口
     * [说明]: 本命令用于查询和设置当前设备是否支持PCSC端口。修改设置后，需要重启单板才能生效。
     *         该命令不支持参数范围查询。
     *         在使用该命令前，首先需要使用命令“AT^U2DIAG=288”将设备形态切到288。
     * [语法]:
     *     [命令]: ^PCSCINFO=<value>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^PCSCINFO?
     *     [结果]: <CR><LF>^PCSCINFO: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^PCSCINFO=?
     *     [结果]: <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <value>: 0：不支持PCSC端口；
     *             1：支持PCSC端口。
     * [示例]:
     *     · 设置支持PCSC端口
     *       AT^PCSCINFO=1
     *       OK
     *     · 查询
     *       AT^PCSCINFO?
     *       ^PCSCINFO: 1
     *       OK
     */
    { AT_CMD_PCSCINFO,
      At_SetPcscInfo, AT_NOT_SET_TIME, At_QryPcscInfo, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PCSCINFO", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 网关查询数据卡类型
     * [说明]: 对于现有的PID的数据卡，网关根据引用的文件进行查询，以获取数据卡的端口顺序信息。
     *         对于后面新增的PID，网关在切换到多端口以后，对每个端口依次下发本命令进行查询，如果有返回的端口，再下发本命令进行查询，以得到数据卡类型和端口的顺序。
     * [语法]:
     *     [命令]: ^GETPORTMODE
     *     [结果]: <CR><LF>^GETPORTMODE: <TYPE><芯片厂商>,<端口><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <TYPE>: 数据卡类型，返回字符串参数。
     *     <芯片厂商>: 数据卡芯片提供商。
     *     <端口>: 支持的端口。
     * [示例]:
     *     · 查询数据卡类型
     *       AT^GETPORTMODE
     *       ^GETPORTMODE: TYPE:WCDMA:huawei,NDIS:0,CDROM:1,DIAG:2,PCUI:3,GPS:4,PCVoice:5,SHELL:6
     *       Ok
     */
    { AT_CMD_GETPORTMODE,
      At_SetGetportmodePara, AT_NOT_SET_TIME, At_QryGetportmodePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_STICK,
      (VOS_UINT8 *)"^GETPORTMODE", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置PCUI、PCUI2及CTRL口的并发标志位
     * [说明]: 本命令用于设置PCUI、PCUI2及CTRL口的并发操作。
     * [语法]:
     *     [命令]: ^PORTCONCURRENT=<curflag>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^PORTCONCURRENT=?
     *     [结果]: <CR><LF>^PORTCONCURRENT: (list of supported < curflag>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <curflag>: 并行下发命令标志位，取值范围0~1。
     * [示例]:
     *     · 设置PCUI、PCUI2或CTRL口并行下发命令
     *       AT^PORTCONCURRENT=1
     *       OK
     *     · 测试命令
     *       AT^PORTCONCURRENT=?
     *       ^PORTCONCURRENT: (0-1)
     *       OK
     */
    { AT_CMD_PORTCONCURRENT,
      AT_SetPcuiCtrlConcurrentFlagForTest, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PORTCONCURRENT", (VOS_UINT8 *)"(0-1)" },
};

/* 注册TAF通用AT命令表 */
VOS_UINT32 AT_RegisterGeneralTafCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atGeneralTafCmdTbl, sizeof(g_atGeneralTafCmdTbl) / sizeof(g_atGeneralTafCmdTbl[0]));
}

