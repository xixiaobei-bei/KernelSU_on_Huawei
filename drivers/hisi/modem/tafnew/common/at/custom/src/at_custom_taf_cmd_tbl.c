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
#include "at_custom_taf_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_taf_set_cmd_proc.h"
#include "at_custom_taf_qry_cmd_proc.h"
#include "at_custom_mm_qry_cmd_proc.h"
#include "at_custom_mm_set_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atCustomTafCmdTbl[] = {
#if (FEATURE_MULTI_MODEM == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 交换两个modem的相关信息
     * [说明]: 本命令用于交换两个指定modem的相关信息。
     *         注：当前只交换两个modem的NV 2613 (EquivalentPlmn)和NV 8451 (Taf_PdpPara_0_R7)值
     * [语法]:
     *     [命令]: ^EXCHANGEMODEMINFO=<MODEMID1>,<MODEMID2>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况返回值：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^EXCHANGEMODEMINFO=?
     *     [结果]: <CR><LF>^EXCHANGEMODEMINFO: (0-2),(0-2)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <MODEMID1>: 第一个modem ID。取值范围0~2
     *     <MODEMID2>: 第二个modem ID。取值范围0~2
     * [示例]:
     *     · 交换modem0和modem1的信息
     *       AT^EXCHANGEMODEMINFO=0,1
     *       OK
     *     · 测试命令
     *       AT^EXCHANGEMODEMINFO=?
     *       ^EXCHANGEMODEMINFO: (0-2),(0-2)
     *       OK
     */
    { AT_CMD_EXCHANGE_MODEM_INFO,
      AT_ExchangeModemInfo, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^EXCHANGEMODEMINFO", (TAF_UINT8 *)"(0-2),(0-2)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 配置MODEM的AT主动上报端口
     * [说明]: AP-Modem形态下，AP需要MODEM支持动态选择并配置主动上报的端口，可选择所有端口都上报，或只向某个或多个指定的端口上报。
     *         AP可以根据自己需求进行配置，打开指定端口的主动上报或关闭指定端口的主动上报。
     *         该命令只在与AP对接的AT通道收到有效，其他AT通道接收到均认为失败。
     *         每次开机都需要重新设置，关机后失效。
     * [语法]:
     *     [命令]: ^APRPTPORTSEL=<portsel1>,<portsel2>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^APRPTPORTSEL?
     *     [结果]: <CR><LF>^APRPTPORTSEL: <portsel1>, <portsel2><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^APRPTPORTSEL=?
     *     [结果]: <CR><LF>^APRPTPORTSEL: (0-FFFFFFFF), (0-FFFFFFFF)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <portselOne>: 整型值，上报端口选择。该字段包含4个字节，每一个bit位标识一个端口。
     *     <portselTwo>: 整型值，上报端口选择。该字段包含4个字节，每一个bit位标识一个端口。
     * [表]: portselOne的bit0-31对应端口
     *       ,     1Oct,  1Oct,  1Oct,  1Oct, 1Oct, 1Oct,  1Oct,    1Oct,
     *       Bit,  0,     1,     2,     3,    4,    5,     6,       7,
     *       端口, PCUI,  CTRL,  MODEM, NDIS, UART, SOCK,  APPSOCK, HSIC1,
     *       Bit,  8,     9,     10,    11,   12,   13,    14,      15,
     *       端口, HSIC2, HSIC3, HSIC4, MUX1, MUX2, MUX3,  MUX4,    MUX5,
     *       Bit,  16,    17,    18,    19,   20,   21,    22,      23,
     *       端口, MUX6,  MUX7,  MUX8,  APP,  APP1, APP2,  APP3,    APP4,
     *       Bit,  24,    25,    26,    27,   28,   29,    30,      31,
     *       取值, APP5,  APP6,  APP7,  APP8, APP9, APP10, APP11,   APP12,
     * [表]: portselTwo的bit0-31对应端口
     *       ,     1Oct,  1Oct,  1Oct,      1Oct, 1Oct, 1Oct, 1Oct, 1Oct,
     *       Bit,  0,     1,     2,         3,    4,    5,    6,    7,
     *       端口, APP13, APP14, HSICMODEM, 预留, 预留, 预留, 预留, 预留,
     *       Bit,  8,     9,     10,        11,   12,   13,   14,   15,
     *       端口, 预留,  预留,  预留,      预留, 预留, 预留, 预留, 预留,
     *       Bit,  16,    17,    18,        19,   20,   21,   22,   23,
     *       端口, 预留,  预留,  预留,      预留, 预留, 预留, 预留, 预留,
     *       Bit,  24,    25,    26,        27,   28,   29,   30,   31,
     *       取值, 预留,  预留,  预留,      预留, 预留, 预留, 预留, 预留,
     * [示例]:
     *     · 配置MODEM的AT主动上报端口为PCUI端口
     *       AT^APRPTPORTSEL=1,0
     *       OK
     *     · 配置HSIC1,2需要主动上报，其他端口都不主动上报
     *       AT^APRPTPORTSEL=180,0
     */
    { AT_CMD_APRPTPORTSEL,
      AT_SetApRptPortSelectPara, AT_NOT_SET_TIME, AT_QryApRptPortSelectPara, AT_NOT_SET_TIME, AT_TestApRptPortSelectPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^APRPTPORTSEL", (VOS_UINT8 *)"(@PortSelOne),(@PortSelTwo),(@PortSelThree)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置GPS是否支持清除辅助定位信息功能
     * [说明]: 该命令用于设置GPS芯片是否支持清除辅助定位信息功能，上电开机默认为GPS芯片不支持清除辅助定位信息功能。
     *         清除辅助定位信息功能主要是用于仪器测试GPS定位功能时，清除GPS芯片缓存的辅助定位信息，以免用例之间互相干扰。
     * [语法]:
     *     [命令]: ^XCPOSR=<support>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^XCPOSR?
     *     [结果]: <CR><LF>^XCPOSR: <support><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^XCPOSR=?
     *     [结果]: <CR><LF>
     *             ^XCPOSR: (list of supported <support>s)
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <support>: 整型值，是否支持清除辅助定位信息功能，取值范围0~1：
     *             0：不支持清除辅助定位信息功能；
     *             1：支持清除辅助定位信息功能。
     * [示例]:
     *     · 设置GPS芯片支持清除辅助定位信息功能
     *       AT^XCPOSR=1
     *       OK
     *     · 查询GPS芯片是否支持清除辅助定位信息功能
     *       AT^XCPOSR?
     *       ^XCPOSR: 1
     *       OK
     *     · 测试命令
     *       AT^XCPOSR=?
     *       ^XCPOSR: (0,1)
     *       OK
     */
    { AT_CMD_XCPOSR,
      AT_SetXcposrPara, AT_SET_PARA_TIME, AT_QryXcposrPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^XCPOSR", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 设置是否主动上报清除辅助定位信息指示
     * [说明]: 该命令用来设置Modem 是否主动上报清除辅助定位信息的指示给GPS芯片，上电开机默认为不允许主动上报。
     *         该命令主要用于在GPS芯片支持清除辅助定位信息功能时，控制Modem是否主动上报清除辅助定位信息的指示给GPS芯片。
     * [语法]:
     *     [命令]: ^XCPOSRRPT=<mode>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^XCPOSRRPT?
     *     [结果]: <CR><LF>^XCPOSRRPT: <mode><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: ^XCPOSRRPT=?
     *     [结果]: <CR><LF>^XCPOSRRPT: (list of supported <mode>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，是否启用清除辅助定位信息指示的主动上报功能，取值范围0~1，默认值为0：
     *             0：关闭主动上报功能；
     *             1：开启主动上报功能。
     * [示例]:
     *     · 设置启用清除辅助定位信息指示的主动上报功能：
     *       AT^XCPOSRRPT=1
     *       OK
     *     · 查询是否启用清除辅助定位信息指示的主动上报功能：
     *       AT^XCPOSRRPT?
     *       ^XCPOSRRPT: 1
     *       OK
     *     · 执行测试命令
     *       AT^XCPOSRRPT=?
     *       ^XCPOSRRPT: (0,1)
     *       OK
     */
    { AT_CMD_XCPOSRRPT,
      AT_SetXcposrRptPara, AT_SET_PARA_TIME, AT_QryXcposrRptPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^XCPOSRRPT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置GPS信息
     * [说明]: 设置GPS信息，包括高度、经度、纬度、速度和时间。
     *         此命令仅用于AP向Modem发送当前GPS信息，不更改Modem的相关状态。
     * [语法]:
     *     [命令]: ^CGPSINFO[=[<ALTITUDE>[,<LONGITUDE>[,<LATITUDE>[,<SPEED>[,<TIME>]]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CGPSINFO=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <ALTITUDE>: 字符串类型，高度信息。
     *     <LONGITUDE>: 字符串类型，经度信息。
     *     <LATITUDE>: 字符串类型，纬度信息。
     *     <SPEED>: 字符串类型，速度信息。
     *     <TIME>: 字符串类型，时间信息。
     * [示例]:
     *     · 设置GPS信息
     *       AT^CGPSINFO="0.0","121.6269374","31.2641758","0.0","201208161030"
     *       OK
     *     · 测试命令
     *       AT^CGPSINFO=?
     *       OK
     */
    { AT_CMD_CGPSINFO,
      AT_SetGpsInfoPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CGPSINFO", (VOS_UINT8 *)"(@altitude),(@longitude),(@latitude),(@speed),(@time)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置将缓存的LOG立即输出
     * [说明]: 该命令用于AP-Modem形态下通过LOG2.0机制，将可维可测信息保存在AP侧的SD卡上。
     *         命令使用限制：仅用于LOG2.0机制的可维可测数据延迟写入功能。
     * [语法]:
     *     [命令]: ^LOGSAVE
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：<CR><LF>+CME ERROR:<err><CR><LF>
     * [示例]:
     *     · 将Modem侧缓存的LOG立即输出到AP侧
     *       AT^LOGSAVE
     *       OK
     */
    { AT_CMD_LOGSAVE,
      AT_SetLogSavePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LOGSAVE", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询/主动上报MIPICLK筛选结果
     * [说明]: 查询当前MIPICLK的筛选结果，RF下行频率改变后主动上报MIPICLK筛选结果。
     * [语法]:
     *     [命令]: ^MIPICLK
     *     [结果]: <CR><LF>^MIPICLK <bitmap><CR><LF>
     *     [命令]: ^MIPICLK?
     *     [结果]: <CR><LF>^MIPICLK <bitmap><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR <err><CR><LF>
     *     [命令]: ^MIPICLK=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <bitmap>: 整型值，MIPICLK位图。
     * [示例]:
     *       RF下行频点改变后主动上报MIPICLK筛选结果
     *       ^MIPICLK: 12
     *     · 亮屏时主动查询最新的MIPICLK筛选结果
     *       AT^MIPICLK?
     *       ^MIPICLK: 12
     *       OK
     */
    { AT_CMD_MIPICLK,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryMipiClkValue, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^MIPICLK", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置、查询和主动上报器件最优频率类号
     * [说明]: 使能或禁止器件最优频率类号的主动上报功能；查询所有激活器件的最优频率类号的相关信息；当器件的最优频率类号发生变化时，主动上报变化，每次上报只上报一个变化的最优频率类号，若有多个频率类号发生变化则分多条指令上报。此命令只支持MODEM0，非MODEM0对应的AT通道下发的指令无效。
     *         注意点:
     *         若设置指令携带的DeviceID不为0且与NV配置的所有器件Device都不匹配，则返回ERROR；
     *         若激活器件数目为0，则查询结果不带具体内容，只返回OK。
     * [语法]:
     *     [命令]: ^BESTFREQ=<DeviceID>,<Mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>  ERROR<CR><LF>
     *     [命令]: ^BESTFREQ?
     *     [结果]: [<CR><LF>^BESTFREQ: <GroupID>, <DeviceID>, <CaseID>, <Mode>
     *             [<CR><LF>^BESTFREQ: <GroupID>, <DeviceID>, <CaseID >, <Mode>[…]]]
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^BESTFREQ=?
     *     [结果]: <CR><LF>^BESTFREQ: (0-255),(0-1)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: 器件最优频率类号发生变化时由Modem主动上报：
     *             [<CR><LF>^BESTFREQ: <GroupID>, <DeviceID>, <CaseID><CR><LF>]
     * [参数]:
     *     <Mode>: 整型值，禁止或使能^BESTFREQ的上报通知   0：禁止   1：使能
     *     <GroupID>: 整型值，标识器件组号，有效范围为[0~7]，分别对应干扰控制NV中的8个group组，每个group至多关联一个器件ID
     *     <DeviceID>: 整型值，标识器件ID，有效范围为[0~255]，1-255用于标识特定的器件，在一个特定型号的产品中器件ID是固定且独一无二的；0用于表示所有的器件（仅在^BESTFREQ=<DeviceID>,<Mode>指令中有效），如果设置的DeviceID不在8组器件中，则返回ERROR
     *     <CaseID>: 整型值，标识器件的频率类号，有效范围为[0~3],对应干扰控制NV中每个group中的4个Case类。一个Case类关联着器件一个预设的工作频率以及所产生的最多16个干扰频率
     * [示例]:
     *     · 使能全部group的主动上报
     *       AT^BESTFREQ=0,1
     *       OK
     *     · 禁止全部group的主动上报
     *       AT^BESTFREQ=0,0
     *       OK
     *     · 使能gourp1主动上报（假设group1相对应的DeviceID为134）
     *       AT^BESTFREQ=134,1
     *       OK
     *     · 禁止group2的主动上报（假设group2相对应的DeviceID为104）
     *       AT^BESTFREQ=104,0
     *       OK
     *     · 查询所有激活的器件的信息(有激活器件)
     *       AT^BESTFREQ?
     *       ^BESTFREQ: 3, 12, 2, 1
     *       ^BESTFREQ: 5, 23, 3, 1
     *       OK
     *     · 查询所有激活的器件的信息(无激活器件)
     *       AT^BESTFREQ?
     *       OK
     *       最优频率类号发生变化主动上报
     *       ^BESTFREQ: 3, 12, 2
     *       ^BESTFREQ: 5, 23, 3
     *     · 测试指令
     *       AT^BESTFREQ=?
     *       ^BESTFREQ: (0-255),(0-1)
     *       OK
     *     · 主动上报设置的DeviceID不在NV配置的8组器件DeviceID中，返回错误
     *       AT^BESTFREQ=121,0
     *       ERROR
     */
    { AT_CMD_BESTFREQ,
      AT_SetBestFreqPara, AT_SET_PARA_TIME, AT_QryBestFreqPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^BESTFREQ", (VOS_UINT8 *)"(0-255),(0-1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 手机运动状态
     * [说明]: Sensor Hub感知当前手机状态的相关操作。
     * [语法]:
     *     [命令]: ^SENSOR=<STATUS>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SENSOR=?
     *     [结果]: <CR><LF>^SENSOR: (list of supported <STATUS>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <STATUS>: 整数型，当前手机状态，取值0～4294967295，即0x0~0xffffffff，该值可以是如下bit位代表的各状态值的叠加值，高2个字节的是保留位，比如当前处理乘车和高铁状态，该值为0x401。
     *             0x00000000：UNKNOWN状态；
     *             0x00000001：车载状态；
     *             0x00000002：骑车状态；
     *             0x00000004：慢跑状态；
     *             0x00000008：快跑状态；
     *             0x00000010：静止状态；
     *             0x00000020：倾斜状态；
     *             0x00000040：END状态；
     *             0x00000080：公交状态；
     *             0x00000100：小车状态；
     *             0x00000200：地铁状态；
     *             0x00000400：高铁状态；
     *             0x00000800：公路交通状态；
     *             0x00001000：铁路交通状态；
     *             0x00002000：爬山状态；
     *             0x00004000：快走状态；
     *             0x00008000：停车状态。
     *             0x00010000：电梯状态；
     *             0x00020000：微动状态；
     *             0x00040000：车库状态。
     * [示例]:
     *     · 手机状态静止
     *       AT^SENSOR=16
     *       OK
     *     · 测试命令
     *       AT^SENSOR=?
     *       ^SENSOR: (0-4294967295)
     *       OK
     */
    { AT_CMD_SENSOR,
      At_SetSensorPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^SENSOR", (VOS_UINT8 *)"(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NV刷新命令
     * [说明]: 用于通知MODEM侧NV刷新。
     *         该命令只用于上电、换卡或双卡切换等场景下的NV随卡操作。
     * [语法]:
     *     [命令]: ^NVREFRESH
     *     [结果]: 执行正确时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     * [示例]:
     *       AT^NVREFRESH
     *       OK
     */
    { AT_CMD_NVREFRESH,
      AT_SetNvRefreshPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^NVREFRESH", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 网侧需要Reattach进行的Detach操作上报
     * [说明]: 该命令主要用于去注册时，如果DETACH REQUEST消息中携带的detach类型为"re-attach required"，则Modem需要把此事件上报给应用。
     * [语法]:
     *     [命令]: ^MTREATTACH=<report>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err ><CR><LF>
     *     [命令]: ^MTREATTACH=?
     *     [结果]: <CR><LF>^MTREATTACH: (list of supported <report>s) <CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: <CR><LF>^MTREATTACH<CR><LF>
     * [参数]:
     *     <report>: 整型值，控制是否主动上报，默认值为0：
     *             0：不主动上报；
     *             1： 主动上报。
     * [示例]:
     *     · 开启^MTREATTACH的主动上报
     *       AT^MTREATTACH=1
     *       ^MTREATTACH的主动上报
     *       ^MTREATTACH
     *     · 测试命令
     *       AT^MTREATTACH=?
     *       ^MTREATTACH: (0,1)
     *       OK
     */
    { AT_CMD_MTREATTACH,
      AT_SetMtReattachPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^MTREATTACH", (TAF_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 屏幕状态
     * [说明]: 手机屏幕状态的相关操作。
     * [语法]:
     *     [命令]: ^SCREEN=<STATUS>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SCREEN=?
     *     [结果]: <CR><LF>^SCREEN: (list of supported <STATUS>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <STATUS>: 整数型，当前手机屏幕状态，取值0～1。
     *             0：灭屏
     *             1：亮屏
     * [示例]:
     *     · 手机灭屏
     *       AT^SCREEN=0
     *       OK
     *     · 测试命令
     *       AT^SCREEN=?
     *       ^SCREEN: (0-1)
     *       OK
     */
    { AT_CMD_SCREEN,
      At_SetScreenPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^SCREEN", (VOS_UINT8 *)"(0-1)" },

    { AT_CMD_LOGENALBE,
      AT_SetLogEnablePara, AT_SET_PARA_TIME, AT_QryLogEnable, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^LOGENABLE", (TAF_UINT8 *)"(0,1)" },


    { AT_CMD_ACTPDPSTUB,
      AT_SetActPdpStubPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^ACTPDPSTUB", (TAF_UINT8 *)"(0-2),(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: Modem C核时间
     * [说明]: Modem C核的时间，包括日期、时间和时区的相关操作。
     * [语法]:
     *     [命令]: ^TIMESET=<date>,<time>,<zone>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TIMESET=?
     *     [结果]: <CR><LF>^TIMESET: (list of supported <STATUS>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <date>: 格式为yyyy/mm/dd，其中yyyy为4位，区间[1970~2050],mm为两位，区间为[01~12]，dd为两位[01~31]
     *     <time>: 格式为hh:mm:ss，其中hh为2位，区间为[0~24）；mm为两位区间为[0~59]；ss为两位，区间为[0~59]。
     *     <zone>: 整型值
     *             区间为[-12~12]
     * [示例]:
     *     · 设置MODEM时间失败
     *       AT^TIMESET="2012/12/31","12:32:59","33"
     *       +CME ERROR: Incorrect parameters
     *     · 设置MODEM时间成功
     *       AT^TIMESET="2012/12/31","12:32:59","8"
     *       OK
     *     · 测试命令
     *       AT^TIMESET=?
     *       ^TIMESET: (YTD),(time),(zone)
     *       OK
     */
    { AT_CMD_TIMESET,
      AT_SetModemTimePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^TIMESET", (VOS_UINT8 *)"(YTD),(time),(zone)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询Modem链接状态
     * [说明]: 本命令用于查询当前Modem是否处于连接态，以及设置Modem在链接状态发生变化时是否主动上报通知到应用。
     *         本命令仅提供给Modem与Wifi共天线方案使用，用于上层判断Wifi天线切换MIMO或SISO模式。
     *         当前链接状态变化涉及主要业务流程如下：CS域普通呼叫、CS域紧急呼叫、CS域短信业务、CS域补充业务、LAU流程、去注册流程、RAU流程、TAU流程、PS域数据业务信令流程、PS域短信业务等。
     *         该命令只在多Modem形态下有效。
     *         该命令查询的链接状态依据是NAS层业务起始状态，并不完全等同接入层的连接态。
     *         多Modem形态，主动上报开关状态共享，且打开上报时会立即触发一次主动上报。
     * [语法]:
     *     [命令]: ^CRRCONN=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^CRRCONN?
     *     [结果]: <CR><LF>^CRRCONN: <enable>,<status0>,<status1>,<status2>  <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^CRRCONN=?
     *     [结果]: <CR><LF>^CRRCONN: (list of supported <enable>s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，开启或关闭Modem链接状态变化主动上报：
     *             0：关闭主动上报；
     *             1：开启主动上报。
     *     <status0>: 整型值，Modem0的链接状态：
     *             0：无链接或即将退出连接态；
     *             1：有链接或即将进入连接态。
     *     <status1>: 整型值，Modem1的链接状态：
     *             0：无链接或即将退出连接态；
     *             1：有链接或即将进入连接态。
     *     <status2>: 整型值，Modem2的链接状态：
     *             0：无链接或即将退出连接态；
     *             1：有链接或即将进入连接态。
     * [示例]:
     *     · 开启Modem链接状态变化主动上报功能，此时Modem0处于连接态
     *       AT^CRRCONN=1
     *       OK
     *       ^CRRCONN: 1,0,0
     *     · 查询Modem链接状态，主动上报打开，Modem1处于连接态，Modem0、Modem2处于非连接态
     *       AT^CRRCONN?
     *       ^CRRCONN: 1,0,1,0
     *       OK
     *     · 执行测试命令
     *       AT^CRRCONN=?
     *       ^CRRCONN: (0,1)
     *       OK
     */
    { AT_CMD_CRRCONN,
      AT_SetCrrconnPara, AT_SET_PARA_TIME, AT_QryCrrconnPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CRRCONN", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: AP下发EPDU序列
     * [说明]: 该命令提供给GPS芯片将ePDU序列通过Modem发送给网络。受AT命令解析限制，采用分片的方式将ePDU序列码流拆分发送给Modem，再由Modem组装转换后发送给网络。
     *         设置命令用来传递来自GPS的EPDU序列码流信息。
     *         该命令无查询命令。
     *         使用^EPDU命令下发多条EPDU码流时，每相邻两条^EPDU命令之间不应间隔5S以上。
     * [语法]:
     *     [命令]: ^EPDU=<transaction_id>,<msg_type>,<common_info_valid_flg>,<end_flag>,<loc_source>,<loc_calc_err>,<id>,[<name>],<total>,<index>,[<data>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^EPDU=?
     *     [结果]: <CR><LF>^EPDU: (list of supported <transaction_id>),(list of supported <msg_type>),(list of supported <common_info_valid_flg>),(list of supported <end_flag>),(list of supported <loc_source>),(list of supported <loc_calc_err>),(list of supported <id>),(list of supported <name>),(list of supported <total>),(list of supported <index>),(list of supported <data>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <transaction_id>: ePDU码流对应的Transaction Id，整数类型，取值范围为0~65535。
     *     <msg_type>: ePDU码流的消息类型，整数类型，取值范围0~7。
     *             0：请求能力；
     *             1：提供能力；
     *             2：请求辅助数据；
     *             3：提供辅助数据；
     *             4：请求位置信息；
     *             5：提供位置信息；
     *             6：取消；
     *             7：错误。
     *     <common_info_valid_flg>: LPP CommonIEs（经纬度信息）信息是否有效标志位，整数类型，取值范围0~1。
     *             0：无效；
     *             1：有效。
     *     <end_flag>: 定位类型为MSB时ENDFLAG标志位，整数类型，取值范围0~1。
     *             0：ePDU流程结束；
     *             1：还有后续ePDU流程。
     *     <loc_source>: 定位类型为MSB时，定位技术，整数类型，取值范围0~10。
     *             0：未指定；
     *             1：AGNSS；
     *             2：OTDOA；
     *             3：EOTD；
     *             4：OTDOA Utran；
     *             5：ECID LTE；
     *             6：ECID GSM；
     *             7：ECID Utran；
     *             8：WLAN AP；
     *             9：SRN；
     *             10：Sensors。
     *     <loc_calc_err>: 定位类型为MSB时，计算位置信息时候填写，整数类型，bit_map形式，占16bit。0表示对应上述loc_source中位置信息计算成功；1表示计算失败（目前主要用于计算OTDOA的经纬度失败场景）。
     *             对应bit位如下：
     *             0：未指定；
     *             1：AGNSS；
     *             2：OTDOA；
     *             3：EOTD；
     *             4：OTDOA Utran；
     *             5：ECID LTE；
     *             6：ECID GSM；
     *             7：ECID Utran；
     *             8：WLAN AP；
     *             9：SRN；
     *             10：SENSORS。
     *     <id>: ePDU码流的标识，整数类型，取值范围1~256。
     *     <name>: ePDU码流的名称，可选参数，字符串类型，最多32个字符，超过直接丢弃。
     *     <total>: ePDU码流总分片个数，整数类型，取值范围1~9。
     *     <index>: ePDU码流分片索引，整数类型，取值范围1~9。
     *     <data>: ePDU分片码流，可选参数，通过Half-Byte（16进制）编码成的字符串，最大长度为500个字符，超过返回失败。
     * [示例]:
     *     · 下发ePDU序列
     *       AT^EPDU=1,2,0,0,8,0,121,"LPPE",1,1,0123456789ABCDEF0123456789ABCDEF
     *       OK
     *     · ePDU测试命令
     *       at^EPDU=?
     *       ^EPDU: (0-65535),(0-7),(0-1),(0-1),(0-10),(0-10),(1-256),(str),(1-9),(1-9),(@data)
     *       OK
     */
    { AT_CMD_EPDU,
      At_SetEpduPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^EPDU", (VOS_UINT8 *)"(0-65535),(0-7),(0-1),(0-1),(0-10),(0-10),(1-256),(str),(1-9),(1-9),(@data)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 上报VT流量
     * [说明]: 当视频承载处于激活状态时，该主动上报消息每隔2s发送一次(间隔时间NV可配)，上报的内容包括当前这次视频承载的连接时间、当前这次视频承载的发送速率、当前这次视频承载的接收速率、当前这次视频承载的发送流量、当前这次视频承载的接收流量、与网络侧协商后确定的PDP连接发送速率和与网络侧协商后确定的PDP连接接收速率。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^VTFLOWRPT: <curr_vt_time>,<curr_tx_flow>,<curr_rx_flow><CR><LF>
     *     [命令]: ^VTFLOWRPT=<oper>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^VTFLOWRPT=?
     *     [结果]: <CR><LF>^VTFLOWRPT: (list of supported <oper>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <curr_vt_time>: 0x0000_0000～0xFFFF_FFFF，8位16进制数，表示当前这次视频承载的连接时间，单位为秒。
     *     <curr_tx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示当前这次视频承载的发送流量，单位为字节。
     *     <curr_rx_flow>: 0x0000_0000_0000_0000～0xFFFF_FFFF_FFFF_FFFF，16位16进制数，表示当前这次视频承载的接收流量，单位为字节。
     *     <oper>: 整型值：视频承载流量上报开关。
     *             0：禁用视频承载自动流量上报；
     *             1：使能视频承载自动流量上报。
     * [示例]:
     *     · 流量上报
     *       AT^VTFLOWRPT=1
     *       OK
     *     · 测试命令
     *       AT^VTFLOWRPT=?
     *       ^VTFLOWRPT: (0,1)
     *       OK
     */
    { AT_CMD_VTFLOWRPT,
      AT_SetVTFlowRptPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VTFLOWRPT", (VOS_UINT8 *)"(0,1)" },

#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 数据业务开关通知
     * [说明]: 当AP侧数据业务开关状态变化时，通过该AT命令通知给Modem。
     * [语法]:
     *     [命令]: ^DATASWITCH=<state>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF> ERROR<CR><LF>
     *     [命令]: ^DATASWITCH?
     *     [结果]: <CR><LF>^DATASWITCH: <state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^DATASWITCH=?
     *     [结果]: <CR><LF>^DATASWITCH: (list of supported status) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值：数据业务开关状态。
     *             0：AP侧数据业务开关关闭；
     *             1：AP侧数据业务开关打开。
     * [示例]:
     *     · 数据业务开关打开通知
     *       AT^DATASWITCH=1
     *       OK
     *     · 数据业务开关关闭通知
     *       AT^DATASWITCH=0
     *       OK
     *     · 查询命令
     *       AT^DATASWITCH?
     *       ^DATASWITCH: 0
     *       OK
     *     · 测试命令
     *       AT^DATASWITCH=?
     *       ^DATASWITCH: (0,1)
     *       OK
     */
    { AT_CMD_DATASWITCH,
      AT_SetDataSwitchStatus, AT_SET_PARA_TIME, AT_QryDataSwitchStatus, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DATASWITCH", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 数据业务漫游开关通知
     * [说明]: 当AP侧数据业务漫游开关状态变化时，通过该AT命令通知给Modem。
     * [语法]:
     *     [命令]: ^DATAROAMSWITCH=<state>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF> ERROR<CR><LF>
     *     [命令]: ^DATAROAMSWITCH?
     *     [结果]: <CR><LF>^DATAROAMSWITCH: <state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^DATAROAMSWITCH=?
     *     [结果]: <CR><LF>^DATAROAMSWITCH: (list of supported status) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值：数据业务漫游开关状态。
     *             0：AP侧数据业务漫游开关关闭；
     *             1：AP侧数据业务漫游开关打开。
     * [示例]:
     *     · 数据业务漫游开关打开通知
     *       AT^DATAROAMSWITCH=1
     *       OK
     *     · 数据业务漫游开关关闭通知
     *       AT^DATAROAMSWITCH=0
     *       OK
     *     · 查询命令
     *       AT^DATAROAMSWITCH?
     *       ^DATAROAMSWITCH: 0
     *       OK
     *     · 测试命令
     *       AT^DATAROAMSWITCH=?
     *       ^DATAROAMSWITCH: (0,1)
     *       OK
     */
    { AT_CMD_DATAROAMSWITCH,
      AT_SetDataRoamSwitchStatus, AT_SET_PARA_TIME, AT_QryDataRoamSwitchStatus, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DATAROAMSWITCH", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询LTE默认承载注册信息
     * [说明]: 本命令用于获取LTE注册成功后默认承载的信息，目前仅在LTE携带空APN注册成功后，支持查询网络分配的APN以及IP TYPE信息
     *         服务无效和非LTE网络时，会返回ERROR
     * [语法]:
     *     [命令]: ^LTEATTACHINFO?
     *     [结果]: <CR><LF>^LTEATTACHINFO: <PDP_type>,<APN><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <PDP_type>: 整型值，分组数据协议类型
     *             1：IPV4
     *             2：IPV6
     *             3：IPV4V6
     *     <APN>: 字符串值，表示连接GGSN或外部网的接入点域名。若该值为空，则使用签约值
     * [示例]:
     *     · 获取LTE注册承载信息
     *       AT^LTEATTACHINFO?
     *       ^LTEATTACHINFO: 1,"Huawei.com"
     *       OK
     */
    { AT_CMD_LTEATTACHINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryLteAttachInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LTEATTACHINFO", VOS_NULL_PTR },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NSA下LTE-NR的DC双连接
     * [说明]: 该命令用于控制LTE下ENDC双连接状态变化上报，及查询LTE主模下，当前网络中NR网络情况。
     *         本命令仅在LTE主模查询结果有效。
     *         本命令仅在支持NR的情况下才支持上报和查询，否则查询回复失败。
     * [语法]:
     *     [命令]: ^LENDC=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LENDC?
     *     [结果]: <CR><LF>^LENDC: <enable>,<endc_available>, <endc_plmn_avail>, <endc_restricted>,<nr_pscell><CR><LF> <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LENDC=?
     *     [结果]: <CR><LF>^LENDC: (list of supported <enable>s) <CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: LTE主模下ENDC状态变化
     *     [结果]: <CR><LF>^LENDC: <endc_available>, <endc_plmn_avail>, <endc_restricted>,<nr_pscell><CR><LF>
     * [参数]:
     *     <enable>: 整型值，指示LTE下ENDC双连接状态变化是否主动上报：
     *             0：禁止主动上报；
     *             1：使能主动上报。
     *     <endc_available>: 整数值，当前小区是否支持ENDC模式，由LRRC中SIB2的upperLayerIndication-r15标记指示：
     *             0：不支持；
     *             1：支持。
     *     <endc_plmn_avail>: 整型值，PLMN LIST中是否有PLMN支持ENDC模式：
     *             0：没有PLMN支持ENDC模式；
     *             1：有PLMN支持ENDC模式。
     *     <endc_restricted>: 整型值，LNAS注册ACCEPT消息中ENDC能力：
     *             0：核心网没有限制ENDC能力；
     *             1：核心网限制ENDC能力。
     *     <nr_pscell>: 整型值，当前PSCell是否为NR，即是否进入ENDC双连接状态：
     *             0：非ENDC状态；
     *             1：ENDC已经建立状态；
     *             2：表示疑似ENDC锚点小区状态。
     * [示例]:
     * [示例]:
     *     · 使能主动上报
     *       AT^LENDC=1
     *       OK
     *     · LTE下查询ENDC双连接状态
     *       AT^LENDC?
     *       ^LENDC: 1,1,1,1,0
     *       OK
     *     · 执行测试命令
     *       AT^LENDC=?
     *       ^LENDC: (0,1)
     *       OK
     *       ENDC状态变化为当前小区非ENDC状态
     *       ^LENDC: 1,1,0,0
     *       ENDC状态变化为小区疑似ENDC锚点状态
     *       ^LENDC: 1,1,0,2
     */
    { AT_CMD_LENDC,
      AT_SetLendcPara, AT_SET_PARA_TIME, AT_QryLendcPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LENDC", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: UE policy section信息上报
     * [说明]: 设置类命令，表示设置主动上报命令^CPOLICYRPT是否启用。
     *         Modem收到网络侧下发的UE policy section信息后，使用^CPOLICYRPT主动上报给AP。
     *         注意：主动上报开关默认打开，Modem收到码流就会上报；与终端约束，保留主动上报控制命令，但暂不允许使用。
     * [语法]:
     *     [命令]: ^CPOLICYRPT=<enable>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: -
     *     [结果]: <CR><LF>^CPOLICYRPT: <total_length>,<section_num>,<protocol_ver>
     *             <CR><LF>
     * [参数]:
     *     <enable>: 整型值，是否允许主动上报
     *             0：不主动上报
     *             1：允许主动上报
     *     <total_length>: 整型值，UE POLICY SECTION信息中码流字节数
     *     <section_num>: 整型值，UE POLICY SECTION信息码流需要分段查询的个数
     *     <protocol_ver>: 整型值，URSP版本号
     *             1500: 24526-F00
     *             1510: 24526-F10
     *             1520: 24526-F20
     *             1530: 24526-F30
     *             注：该参数受NV6019的第二个参数控制
     * [示例]:
     *     · 设置主动上报命令
     *       AT^CPOLICYRPT=1
     *       OK
     *       主动上报信息
     *       ^CPOLICYRPT: 2100,2,1510
     */
    { AT_CMD_CPOLICYRPT,
      AT_SetCpolicyRptPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CPOLICYRPT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: UE policy section信息分段查询
     * [说明]: 根据携带的<index>参数，分段查询网络侧下发的码流信息（包括码流中的ANDSP信息），每个数据分段限制为1000个字节，即打印的字符串码流长度不超过2000。
     * [语法]:
     *     [命令]: ^CPOLICYCODE=<index>
     *     [结果]: <CR><LF>^CPOLICYCODE: <index >,<code><CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <index>: 整型值，分段标识
     *     <code>: 字符串，UE POLICY SECTION码流，长度小于等于2000字符
     *             注：上层在解析时，把2个字符拼为一个16进制的数值，例如：”1234”解析为”0x12,0x34”；
     * [示例]:
     *       AT^CPOLICYCODE=2
     *       ^CPOLICYCODE: 2,"0000011111222223333344444"
     *       OK
     */
    { AT_CMD_CPOLICYCODE,
      AT_GetCpolicyCodePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CPOLICYCODE", (VOS_UINT8 *)"(1-255)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置、查询LADN信息
     * [说明]: 设置类命令，如AT^CLADN=<n>，表示设置主动上报命令^CLADNU是否启用。
     *         查询类命令，如AT^CLADN?，表示查询当前位置区所有可以使用的LADN DNN个数及DNN列表，不可使用的LADN DNN个数及 DNN列表。
     * [语法]:
     *     [命令]: ^CLADN=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CLADN?
     *     [结果]: <CR><LF>^CLADNU: <n>,<allowed_ladn_dnn_num>,<allowed_dnn_list>,<nonallowed_ladn_dnn_num>, <nonallowed_dnn_list><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，是否允许主动上报
     *             0：禁止命令^CLADNU主动上报
     *             1：允许命令^CLADNU主动上报
     *     <allowed_ladn_num>: 整形值，当前位置区内可以正常使用的LADN DNN个数。
     *     <allowed_dnn_list>: 字符串类型，当前位置区内可以正常使用的LADN DNN；格式如下：“dnn1;dnn2;…dnn8”
     *     <nonallowed_ladn_dnn_num>: 整形值，当前位置区内不可以使用的LADN DNN个数。allowed_ladn_num与nonallowed_ladn_num的和小于等于8。
     *     <nonallowed_dnn_list>: 字符串类型，当前位置区内不可以使用的LADN DNN；格式如下：“dnn1;dnn2;…dnn8”
     * [示例]:
     *     · 设置主动上报命令
     *       AT^CLADN=1
     *       OK
     *       查询命令
     *     · 主动上报开启，可用/不可用DNN均存在
     *       AT^CLADN?
     *       ^CLADN: 1,1,"Ladn.dnn.01",2,"Ladn.dnn.02;Ladn.dnn.03"
     *       OK
     *     · LADN DNN均可用
     *       AT^CLADN?
     *       ^CLADN:
     *       1,8,"Ladn.dnn.01;Ladn.dnn.02;Ladn.dnn.03;Ladn.dnn.04;Ladn.dnn.05;Ladn.dnn.06;Ladn.dnn.07;Ladn.dnn.08",0,""
     *       OK
     *     · LADN DNN均不可用
     *       AT^CLADN?
     *       ^CLADN: 1,0,"",8,"Ladn.dnn.01;Ladn.dnn.02;Ladn.dnn.03; Ladn.dnn.04;Ladn.dnn.05;Ladn.dnn.06;Ladn.dnn.07;Ladn.dnn.08"
     *       OK
     */
    { AT_CMD_CLADN,
      AT_SetLadnRptPara, AT_SET_PARA_TIME, AT_QryLadnInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CLADN", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_C5GRSDQRY,
      AT_SetC5gRsdPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^C5GRSDQRY", (VOS_UINT8 *)C5GRSDQRY_CMD_PARA_STRING },
#endif

    { AT_CMD_CSQLVL,
      AT_SetCsqlvlPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"+CSQLVL", VOS_NULL_PTR },

    { AT_CMD_CSQLVLEXT,
      AT_SetCsqlvlExtPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CSQLVLEXT", (VOS_UINT8 *)"(0,20,40,60,80,99),(99)" },

    /* 统计从上电到pdp激活成功的启动时间 */
    { AT_CMD_LCSTARTTIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryLcStartTimePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^LCSTARTTIME", VOS_NULL_PTR },

    { AT_CMD_AUTHORITYVER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, atQryAuthorityVer, AT_UPGRADE_TIME_5S, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^AUTHORITYVER", VOS_NULL_PTR },

    { AT_CMD_AUTHORITYID,
      VOS_NULL_PTR, AT_NOT_SET_TIME, atQryAuthorityID, AT_UPGRADE_TIME_5S, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^AUTHORITYID", VOS_NULL_PTR },


    { AT_CMD_NVRSTSTTS,
      atSetNVRstSTTS, AT_UPGRADE_TIME_5S, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^NVRSTSTTS", VOS_NULL_PTR },

    /* XML命令 begin */
    { AT_CMD_APRPTSRVURL,
      AT_SetApRptSrvUrlPara, AT_NOT_SET_TIME, AT_QryApRptSrvUrlPara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APRPTSRVURL", (VOS_UINT8 *)"(@ApRptSrvUrl)" },

    { AT_CMD_APXMLINFOTYPE,
      AT_SetApXmlInfoTypePara, AT_NOT_SET_TIME, AT_QryApXmlInfoTypePara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APXMLINFOTYPE", (VOS_UINT8 *)"(@ApXmlInfoType)" },

    { AT_CMD_APXMLRPTFLAG,
      AT_SetApXmlRptFlagPara, AT_NOT_SET_TIME, AT_QryApXmlRptFlagPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APXMLRPTFLAG", (VOS_UINT8 *)"(0,1)" },
    /* XML命令 end  */

    { AT_CMD_VERTIME,
      AT_SetVertime, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VERTIME", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: RRC状态查询
     * [说明]: 该命令用于设置RRC状态和驻留状态上报，以及RRC状态和驻留状态查询。
     *         注：非LTE和NR下收到RRCSTAT查询命令时modem直接回复error。
     *         本命令仅NR主模会返回INACTIVE
     *         本命令仅LTE主模会返回camp_status参数
     *         DC场景返回当前主站接入制式的RRC状态
     *         GU不支持主动上报和查询
     * [语法]:
     *     [命令]: ^RRCSTAT=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^RRCSTAT?
     *     [结果]: 执行成功时：
     *             <CR><LF>^RRCSTAT: < enable > ,<rrc_status>[,<camp_status>]<CR><LF> <CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: -
     *     [结果]: <CR><LF>^RRCSTAT: <rrc_status>[,<camp_status>]<CR><LF>
     * [参数]:
     *     <enable>: 整型值，标识是否开启主动上报：
     *             0：关闭主动上报；
     *             1：开启主动上报；
     *             注意：RRCSTAT不受CURC主动上报约束。
     *     <rrc_status>: RRC连接状态：
     *             0：RRC状态为非连接态；
     *             1：RRC状态为连接态；
     *             2：RRC状态为INACTIVE状态；
     *             3：RRC状态无效。
     *     <camp_status>: 整型值，驻留状态：
     *             98：CAMPED状态
     *             99：NOT CAMPED状态
     * [示例]:
     *     · 设置RRCSTAT主动上报
     *       AT^RRCSTAT=1
     *       OK
     *       LTE主模下，RRCSTAT主动上报，RRC CONN态和CAMPED状态
     *       ^RRCSTAT: 1,98
     *       NR主模下，RRCSTAT主动上报，RRC CONN态
     *       ^RRCSTAT: 1
     *     · LTE主模下，查询当前RRC状态，当前允许主动上报，RRC IDLE非驻留态
     *       AT^RRCSTAT?
     *       ^RRCSTAT: 1,0,99
     *       OK
     *     · NR主模下，查询当前RRC状态，当前允许主动上报，RRC IDLE态
     *       AT^RRCSTAT?
     *       ^RRCSTAT: 1,0
     *       OK
     *     · 查询当前RRC状态，当前不允许上报，RRC INACTIVE态
     *       AT^RRCSTAT?
     *       ^RRCSTAT: 0,2
     *       OK
     */
    { AT_CMD_RRCSTAT,
      AT_SetRrcStatPara, AT_SET_PARA_TIME, AT_QryRrcStatPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RRCSTAT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: LTE/NR工作频率查询
     * [说明]: 通过该AT获取当前小区频率信息，NR支持多CC的频率信息上报，LTE仅支持主小区频率信息上报。
     *         本命令仅在LTE、NR主模生效。
     *         EN-DC场景，LTE/NR HFREQINFO都会上报。
     *         本命令ＮＲ支持多CC，LTE仅支持主小区。
     * [语法]:
     *     [命令]: ^HFREQINFO=<n>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^HFREQINFO?
     *     [结果]: 执行成功:
     *             <CR><LF>^HFREQINFO:<n>,<sysmode>,<band_class1>,<dl_fcn1>,<dl_freq1><dl_bw1>,<ul_fcn1>,<ul_freq1>,<ul_bw1>,[<band_class2>,<dl_fcn2>,<dl_freq2><dl_bw2>,<ul_fcn2>,<ul_freq2>,<ul_bw2>,[<band_class3>,<dl_fcn3>,<dl_freq3><dl_bw3>,<ul_fcn3>,<ul_freq3>,<ul_bw3>,[<band_class4>,<dl_fcn4>,<dl_freq4><dl_bw4>,<ul_fcn4>,<ul_freq4>,<ul_bw4>]]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^HFREQINFO=?
     *     [结果]: <CR><LF>^HFREQINFO: (list of supported <n>,<sysmode>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整形变量
     *             0    禁止^HFREQINFO的主动上报（默认值）
     *             1    使能^HFREQINFO的主动上报
     *     <sysmode>: 整形变量，表示当前终端所属服务模式：
     *             1    GSM（不支持）
     *             2    CDMA（不支持）
     *             3    WCDMA（不支持）
     *             4    TDSCDMA（不支持）
     *             6    LTE
     *             7    NR
     *             当前该AT仅支持LTE、NR模式
     *     <band_classN>: 整形变量，表示当前终端所处小区对应的频段
     *             LTE取值为1-44，具体值参考3GPP 36.101
     *             NR取值为1-86,257,258,260,261 具体值参考3GPP 38.101，N为载波数目，最大为4。
     *     <dl_fcnN>: 整形变量，下行主小区的频点，无效值取0，N为载波数目，最大为4
     *     <dl_freqN>: 整形变量，下行主小区的频率(100KHZ)，无效值取0，N为载波数目，最大为4。
     *     <dl_bwN>: 整形变量，下行主小区的系统带宽(KHZ)
     *             LTE取值：1400,3000,5000,10000,15000,20000
     *             NR取值：1400,30000,5000,10000,15000,20000,25000,30000,40000,50000,60000,80000,90000,100000,200000,400000，N为载波数目，最大为4。
     *     <ul_fcnN>: 整形变量，上行主小区的频点，无效值取0，N为载波数目，最大为4。
     *     <ul_freqN>: 整形变量，上行主小区的频率(100KHZ)，无效值取0，N为载波数目，最大为4。
     *     <ul_bwN>: 整形变量，上行主小区的系统带宽(KHZ)
     *             LTE取值：1400,3000,5000,10000,15000,20000
     *             NR取值：1400,30000,5000,10000,15000,20000,25000,30000,40000,50000,60000,80000,90000,100000,200000,400000，N为载波数目，最大为4。
     * [示例]:
     *     · WCDMA模式下，查询频率信息
     *       AT^HFREQINFO?
     *       ERROR
     *     · LTE模式下，查询频率信息
     *       AT^HFREQINFO?
     *       ^HFREQINFO: 0,6,18,5925,8675,20000,23925,8225,20000
     *       OK
     *     · NR模式下，查询频率信息
     *       AT^HFREQINFO?
     *       ^HFREQINFO: 0,7,77,640000,360000,100000,640000,36000,100000
     *       OK
     *     · DC模式下，查询频率信息
     *       AT^HFREQINFO?
     *       ^HFREQINFO: 0,6,18,5925,8675,20000,23925,8225,20000
     *       ^HFREQINFO: 0,7,18,5925,8675,20000,23925,8225,20000
     *       OK
     *     · DC模式下，主动上报打开，查询频率信息
     *       AT^HFREQINFO?
     *       ^HFREQINFO: 1,6,18,5925,8675,20000,23925,8225,20000
     *       ^HFREQINFO: 0,7,18,5925,8675,20000,23925,8225,20000
     *       OK
     *       主动上报LTE 小区频率信息
     *       ^HFREQINFO: 1,6,18,5925,8675,20000,23925,8225,20000
     */
    { AT_CMD_HFREQINFO,
      AT_SetHfreqInfoPara, AT_SET_PARA_TIME, AT_QryHfreqInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^HFREQINFO", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: HIFI复位指示
     * [说明]: 该命令用于AP指示MODEM HIFI复位开始、完成。
     *         本命令需要与呼叫相关的AT命令同一个通道下发
     * [语法]:
     *     [命令]: ^HIFIRESET=<mode>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <mode>: 整型值，HIFI复位指示控制，取值0~1。
     *             0：HIFI复位开始指示；
     *             1：HIFI复位完成指示。
     * [示例]:
     *     · HIFI复位开始指示
     *       AT^HIFIRESET=0
     *       OK
     *     · HIFI复位完成指示
     *       AT^HIFIRESET=1
     *       OK
     */
    { AT_CMD_HIFIRESET,
      At_SetHifiReset, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^HIFIRESET", (VOS_UINT8 *)"(0-1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: UE模式通知
     * [说明]: UE通知Modem当前所处模式，当前仅支持睡眠模式。
     * [语法]:
     *     [命令]: ^UEAIMODENTF=<mode_status>,<mode_flag>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <mode_status>: 整数值，标识UE当前所处模式
     *             0：睡眠模式（仅支持睡眠模式）
     *     <mode_flag>: 整数值，标识当前所处模式的状态
     *             0：退出当前所处模式；
     *             1：进入当前所处模式；
     * [示例]:
     *     · 退出睡眠模式
     *       AT^UEAIMODENTF=0,0
     *       OK
     *     · 进入睡眠模式
     *       AT^UEAIMODENTF=0,1
     *       OK
     */
    { AT_CMD_UEMODENTF,
      AT_SetUeModeStatus, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8*)"^UEAIMODENTF", (VOS_UINT8 *)"(0),(0-1)" },

#if (FEATURE_IMS == FEATURE_ON)
    { AT_CMD_IMSPDPCFG,
      AT_SetImsPdpCfg, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSPDPCFG", (VOS_UINT8 *)"(0-20),(0,1)" },
#endif

    { AT_CMD_SETPID,
      At_SetpidPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SETPID", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_RATCOMBINEPRIO,
      AT_SetRATCombinePara, AT_NOT_SET_TIME, AT_QryRATCombinePara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_E5_IS_LOCKED,
      (VOS_UINT8 *)"^RATCOMBINEPRIO", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电话管理
     * [含义]: MT短信和MT呼叫可信名单控制命令
     * [说明]: 用于设置/查询MT短信可信名单、MT语音可信名单和MT语音受限名单（暂不支持）。
     * [语法]:
     *     [命令]: ^TNUMCTL=<sms_white_enable>,<call_white_enable>,<call_black_enbale>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^TNUMCTL?
     *     [结果]: <CR><LF>^TNUMCTL: <sms_white_enable>,<call_white_enable>,<call_black_enbale><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^TNUMCTL=?
     *     [结果]: <CR><LF>^TNUMCTL: (list of supported <sms_white_enable>s),(list of supported <call_white_enable>s),(list of supported <call_black_enbale>s) <CR><LF><CR><LF>OK<CR><LR>
     * [参数]:
     *     <sms_white_enable>: 整型值，表示是否启用短信可信名单。
     *             0：Disable
     *             1：Enable
     *     <call_white_enable>: 整型值，表示是否启用呼叫可信名单。
     *             0：Disable
     *             1：Enable
     *     <call_black_enbale>: 整型值，是否启用呼叫受限名单。（暂不支持）
     *             0：Disable
     *             1：Enable
     * [示例]:
     *     · 启用MT短信可信名单
     *       AT^TNUMCTL=1,0,0
     *       OK
     *     · 查询MT短信可信名单是否使能：
     *       AT^TNUMCTL?
     *       ^TNUMCTL: 1,0,0
     *       OK
     *     · 查询命令支持的参数范围：
     *       AT^TNUMCTL=?^TNUMCTL:(0-1),(0-1),0
     *       OK
     */
    { AT_CMD_TNUMCTL,
    AT_SetTrustNumCtrlPara, AT_NOT_SET_TIME, AT_QryTrustNumCtrlPara, AT_NOT_SET_TIME, AT_TestTrustNumCtrlPara,
    AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
    AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
    (VOS_UINT8 *)"^TNUMCTL", (VOS_UINT8 *)"(0-1),(0-1),(0)" },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /* 为自动化测试需求增加 */
    { AT_CMD_CCC,
      AT_SetCccPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5,
      (VOS_UINT8 *)"^CCC", (VOS_UINT8 *)"(0,1),(1-7)" },

    { AT_CMD_CIMEI,
      At_SetImeiPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CIMEI", (VOS_UINT8 *)"(imei)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 通用(U)SIM访问ISDB
     * [说明]: 该命令用于通用(U)SIM卡访问ISDB功能。将输入的APDU透传到(U)SIM卡，并将(U)SIM卡回复的数据返回。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     *         考虑到AP安全需求，此命令仅限于HSIC AT通道，其余AT通道下发此命令Modem不做处理。
     * [语法]:
     *     [命令]: ^CISA=<length>,<command>
     *     [结果]: <CR><LF>^CISA: <length>,<response><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CISA=?
     *     [结果]: <CR><LF>^CISA: <length>,<cmd><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <length>: 整数型，参数<command>或<response>的字符长度，取值范围（1-520）。
     *     <command>: 下发到(U)SIM卡的APDU数据
     *             字符串类型，长度为10~520byte，使用Half-Byte码编码的二进制码流。
     *     <response>: (U)SIM卡对于下发的<command>的响应数据。
     *             字符串类型，长度为4~514byte，使用Half-Byte码编码的二进制码流。
     * [示例]:
     *     · 输入APDU
     *       AT^CISA=44,"90F2000010F0000000010001FF81FF10FFFFFFFF0201"
     *       ^CISA: 6,"019000"
     *       OK
     *     · 测试命令
     *       AT^CISA=?
     *       ^CISA: (1-520),(cmd)
     *       OK
     */
    { AT_CMD_CISA,
      VOS_NULL_PTR, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CISA", (TAF_UINT8 *)"(1-520),(cmd)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 使能LOG延迟输出
     * [说明]: 此命令仅在AP+Modem产品形态下通过LogTool在AP侧保存HiDS数据时使用。
     *         该命令用于AP-Modem形态下设置LogTool的延迟输出机制。延迟上报功能是为了能够在单板不接USB的情况，依然可以保存单板的可维可测数据，并且上报过程不对单板的功耗产生较大影响。因此该功能主要在Beta测试用户抓取Log功能使用，属于研发内部功能，不对外发布。功能默认关闭。
     *         功能打开和关闭此功能需要重启单板才能生效。
     *         打开功能之后，由于其参数配置超过HiDS工具的等待时间，因此无法连接HiDS工具。遇到HiDS无法连接工具时，需要先输入查询命令，判断当前功能的状态和参数设置。如果希望能够连接HiDS工具，需要按照以下操作进行尝试：
     *         AT^LOGPORT=0，AT命令返回成功后，请再次连接；
     *         如果在通过USB连接HiDS工具的情况下想通过LogTool抓取Log时，请先将手机的USB拔出后再启动LogTool。
     *         此命令中所使用的< TIMER_OUT >和<BUFFER_SIZE>参数是通过DTS在开机阶段获取的，通过此命令可以查询但不能修改参数的值。产品线可以根据产品的规格调整相关参数的值。以下描述中涉及的参数值均为Balong提供的默认配置。
     * [语法]:
     *     [命令]: ^LOGNVE[=[<ENABLE_FLAG>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^LOGNVE?
     *     [结果]: <CR><LF>^LOGNVE: <ENABLE_FLAG>,< TIMER_OUT >,<BUFFER_SIZE><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LOGNVE=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <ENABLE_FLAG>: LOG延迟写入使能标志。
     *             0：不使能；
     *             1：使能。
     *     <TIMER_OUT>: 延迟时间，以min为单位，取值范围10~20。
     *     <BUFFER_SIZE>: 延迟写入功能打开时使用的缓冲区大小，以byte为单位
     * [示例]:
     *     · 使能LOG延迟写入机制
     *       AT^LOGNVE=1
     *       OK
     *     · 查询命令，延迟功能已使能，超时时间为10分钟，缓存大小为50M
     *       AT^LOGNVE?
     *       ^LOGNVE: 1,10,52428800
     *       OK
     *     · 测试命令
     *       AT^LOGNVE=?
     *       OK
     */
    { AT_CMD_LOGNVE,
      AT_SetLogNvePara, AT_SET_PARA_TIME, AT_QryLogNvePara, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^LOGNVE", (VOS_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 清除LTE历史频点
     * [说明]: 用于清除LTE历史频点信息。
     * [语法]:
     *     [命令]: ^CHISFREQ=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CHISFREQ=?
     *     [结果]: <CR><LF>^CHISFREQ: List of supported <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，清除历史频点的类型。
     *             0：清除非CSG历史频点信息；
     *             1：清除CSG历史频点信息；
     *             2：清除所有（包括非CSG和CSG）历史频点信息
     * [示例]:
     *     · 测试命令
     *       AT^CHISFREQ=?
     *       ^CHISFREQ: (0,1,2)
     *        OK
     *     · 清除CSG历史频点信息
     *       AT^CHISFREQ=1
     *       OK
     */
    { AT_CMD_CHISFREQ,
      AT_SetHistoryFreqPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CHISFREQ", (VOS_UINT8 *)"(0,1,2)" },

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_LTE_MBMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 管理MBMS接收
     * [说明]: 该命令在LTE下才支持，用于管理MBMS接收相关功能命令，包括使能或去使能MBMS服务特性，设置MBMS服务状态，查询MBMS服务列表和测试命令请求，设置MBMS广播模式为单播或组播，查询SIB16网络时间，查询BSSI信号强度，查询网络信息，查询eMBMS功能状态功能。
     *         在关机状态下执行该命令相关功能将返回错误信息。MBMS服务特性默认是关闭的，每次软开关机后恢复为默认值，需要重新开启。
     *         只有MBB支持该命令，手机不支持。
     *         该命令相关功能（不包括设置MBMS广播模式为单播或组播，查询网络信息，查询eMBMS功能状态功能）受到设置MBMS服务特性使能或去使能配置影响，只有在MBMS服务特性使能的情况下才能执行相关功能，否则直接返回错误信息。
     * [语法]:
     *     [命令]: ^MBMSCMD=<cmd>[,<param1>[,<param2>]]
     *     [结果]: 设置执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置执行错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *             查询执行结果：
     *             <CR><LF>^MBMSCMD: <ResponseType><CR><LF><CR><LF>OK/+CME ERROR: <err><CR><LF>
     *     [命令]: ^MBMSCMD?
     *     [结果]: <CR><LF>^MBMSCMD: <AreaID>,<TMGI>[,<SessionID>] <CR><LF>^MBMSCMD: <AreaID>,<TMGI>[,<SessionID>] <CR><LF>
     *             …
     *             ^MBMSCMD: <AreaID>,<TMGI>[,<SessionID>] <CR><LF>
     *             <CR><LF>OK/+CME ERROR:<err><CR><LF>
     *     [命令]: ^MBMSCMD=?
     *     [结果]: <CR><LF>^MBMSCMD: List of supported <cmd><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <cmd>: 字符串类型，管理MBMS命令名称，目前仅支持下述命令名称：
     *             "MBMS_SERVICE_ENABLER"：使能或去使能MBMS服务；
     *             "ACTIVATE"：激活MBMS服务；
     *             "DEACTIVATE"：去激活MBMS服务；
     *             "DEACTIVATE_ALL"：去激活所有的MBMS服务；
     *             "MBMS_PREFERENCE"：设置MBMS广播模式为单播或组播；
     *             "SIB16_GET_NETWORK_TIME"：查询SIB16网络时间；
     *             "BSSI_SIGNAL_LEVEL"：查询BSSI信号强度；
     *             "NETWORK INFORMATION"：查询网络信息；
     *             "MODEM_STATUS"：查询eMBMS功能状态。
     *     <param1>: 使能或去使能MBMS服务时，表示MBMS服务特性操作，整数型，取值0，1：
     *             0：去使能；
     *             1：使能。
     *             激活/去激活MBMS服务时，表示ArealD，整数型。
     *             设置MBMS广播模式为单播或组播时，表示广播模式，整数型，取值0，1：
     *             0：单播；
     *             1：组播。
     *     <param2>: 激活/去激活MBMS服务时，表示TMGI信息，字符串，包含MBMS Service ID、PLMN ID。
     *     <ResponseType>: 查询SIB16网络时间时，表示<TimingInfo>：UTC时间（0-549755813887），整型值。
     *             查询BSSI信号强度时，表示<BSSI signal level>：BSSI信号强度，整数型，255(0xFF)表示BSSI信号强度无效。
     *             查询网络信息时，表示<cell_id>：小区ID，整数型。
     *             查询eMBMS功能状态时，表示<modem status>：整数型，取值0，1。
     *             0：eMBMS特性功能关闭；
     *             1：eMBMS特性功能开启。
     *     <AreaID>: Area ID，整数型。
     *     <TMGI>: TMGI，字符串，包含MBMS Service ID、PLMN ID。
     *     <SessionID>: SessionID，整数型。
     * [示例]:
     *     · 设置配置使能或去使能MBMS服务特性成功
     *       AT^MBMSCMD="MBMS_SERVICE_ENABLER",1
     *       OK
     *     · 设置所有MBMS服务状态为去激活成功
     *       AT^MBMSCMD="DEACTIVATE_ALL"
     *       OK
     *     · 查询MBMS服务列表
     *       AT^MBMSCMD?
     *       ^MBMSCMD: 1,10000146000,1
     *       ^MBMSCMD: 2,10000246001,2
     *       ^MBMSCMD: 3,10000346002,3
     *       OK
     *     · 测试命令
     *       AT^MBMSCMD=?
     *       ^MBMSCMD: ("MBMS_SERVICE_ENABLER","ACTIVATE","DEACTIVATE","DEACTIVATE_ALL","MBMS_PREFERENCE","SIB16_GET_NETWORK_TIME","BSSI_SIGNAL_LEVEL","NETWORK INFORMATION","MODEM_STATUS")
     *       OK
     *     · 设置MBMS广播模式为组播成功
     *       AT^MBMSCMD="MBMS_PREFERENCE",1
     *       OK
     *     · 查询SIB16网络时间
     *       AT^MBMSCMD="SIB16_GET_NETWORK_TIME"
     *       ^MBMSCMD: 124343
     *       OK
     *     · 查询BSSI信号强度
     *       AT^MBMSCMD="BSSI_SIGNAL_LEVEL"
     *       ^MBMSCMD: 65
     *       OK
     *     · 查询网络信息
     *       AT^MBMSCMD="NETWORKINFORMATION"
     *       ^MBMSCMD: 6
     *       OK
     *     · 查询eMBMS功能状态
     *       AT^MBMSCMD="MODEM_STATUS"
     *       ^MBMSCMD: 1
     *       OK
     */
    { AT_CMD_MBMSCMD,
      AT_SetMBMSCMDPara, AT_SET_PARA_TIME, AT_QryMBMSCmdPara, AT_QRY_PARA_TIME, At_TestMBMSCMDPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^MBMSCMD", (TAF_UINT8 *)MBMSCMD_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置MBMS主动上报
     * [说明]: 此命令在LTE下才支持，用于上层设置MBMS主动上报配置请求或者测试操作到modem。MBMS主动上报配置默认是关闭的，每次软开关机后恢复为默认值，需要重新开启。MBMS主动上报配置设置成功则直接返回OK，在关机状态下或者设置失败情况下返回错误信息。
     *         只有MBB支持，手机不支持。
     *         接入层主动上报MBMS服务事件将受到设置MBMS主动上报配置影响，只有在主动上报配置打开的情况下才能主动上报MBMS服务事件。
     * [语法]:
     *     [命令]: ^MBMSEV=<cmd>
     *     [结果]: 设置执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置执行错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^MBMSEV=?
     *     [结果]: <CR><LF>^MBMSEV: List of supported <cmd><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: (unsolicitedresultcode)
     *     [结果]: ^MBMSEV: <event>
     * [参数]:
     *     <cmd>: 主动上报配置，整数型，取值0，1。
     *             0：关闭；
     *             1：打开；
     *     <event>: 整数型，取值0-99。
     *             0：Service change event；
     *             1：No service (No LTE coverage)；
     *             2：Only unicast service available；
     *             3：LTE unicast and eMBMS service available；
     *             4-99：Reserved。
     * [示例]:
     *     · 设置MBMS主动上报成功
     *       AT^MBMSEV=1
     *       OK
     *     · 执行测试命令
     *       AT^MBMSEV=?
     *       ^MBMSEV: (0,1)
     *       OK
     */
    { AT_CMD_MBMSEV,
      AT_SetMBMSEVPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^MBMSEV", (TAF_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置Interest列表
     * [说明]: 只有MBB支持，手机不支持。
     *         此命令在LTE下才支持，用于上层设置Interest列表请求操作到modem。在设置成功状态下直接返回OK，在关机状态下或者设置失败状态下返回错误信息。
     * [语法]:
     *     [命令]: ^MBMSINTERESTLIST=<freq1>,<freq2>,
     *             <freq3>,<freq4>,<freq5>,<mbms_priority>
     *     [结果]: 设置执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置执行错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <freq1>,<freq2>,\n<freq3>,<freq4>,<freq5>: interest列表，整数型，32位的频点。默认值为0。
     *     <mbms_priority>: Mbms服务有限标识，整数型，取值0，1。
     *             0：单播有限；
     *             1：mbms优先；
     * [示例]:
     *     · 设置Interest列表成功且mbms优先
     *       AT^MBMSINTERESTLIST=0,0,0,0,0,1
     *       OK
     */
    { AT_CMD_MBMSINTERESTLIST,
      AT_SetMBMSInterestListPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^MBMSINTERESTLIST", (TAF_UINT8 *)"(@freqone),(@freqtwo),(@freqthree),(@freqfour),(@freqfive),(0,1)" },
#endif

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置低功耗
     * [说明]: 此命令在LTE下才支持，用于上层设置低功耗请求或者测试操作到modem。低功耗设置成功直接返回OK，设置失败或者关机状态下返回错误信息。
     * [语法]:
     *     [命令]: ^LTELOWPOWER=<low_power>
     *     [结果]: 设置执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             设置执行错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LTELOWPOWER=?
     *     [结果]: <CR><LF>^LTELOWPOWER: List of supported < low_power ><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <low_power>: 低功耗标识，整数型，取值0，1。
     *             0：Normal；
     *             1：Low Power Consumption；
     * [示例]:
     *     · 设置低功耗成功
     *       AT^LTELOWPOWER=1
     *       OK
     *     · 执行测试命令
     *       AT^LTELOWPOWER=?
     *       ^LTELOWPOWER: (0,1)
     *       OK
     */
    { AT_CMD_LTELOWPOWER,
      AT_SetLteLowPowerPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^LTELOWPOWER", (TAF_UINT8 *)"(0,1)" },
#endif

#if (FEATURE_LTE == FEATURE_ON)
    { AT_CMD_RSRPCFG,
      AT_SetRsrpCfgPara, AT_NOT_SET_TIME, AT_QryRsrpCfgPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RSRPCFG", (VOS_UINT8 *)"(0-3),(0-200)" },

    { AT_CMD_RSCPCFG,
      AT_SetRscpCfgPara, AT_NOT_SET_TIME, AT_QryRscpCfgPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RSCPCFG", (VOS_UINT8 *)"(0-3),(0-200)" },

    { AT_CMD_ECIOCFG,
      AT_SetEcioCfgPara, AT_NOT_SET_TIME, AT_QryEcioCfgPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ECIOCFG", (VOS_UINT8 *)"(0-3),(0-200)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: FR动态控制
     * [说明]: 该命令用于开启和关闭FAST RETURN功能。
     * [语法]:
     *     [命令]: ^FRSTATUS=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FRSTATUS=?
     *     [结果]: <CR><LF>^FRSTATUS: (list of supported < n >s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，FAST RETURN功能开关。
     *             0：关闭；
     *             1：开启。
     * [示例]:
     *     · 开启FAST RETURN
     *       AT^FRSTATUS=1
     *       OK
     *     · 测试FAST RETURN
     *       AT^FRSTATUS=?
     *       ^FRSTATUS: (0-1)
     *       OK
     */
    { AT_CMD_FRSTATUS,
      AT_SetFrStatus, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^FRSTATUS", (VOS_UINT8 *)"(0-1)" },
#endif
#if (FEATURE_MBB_CUST == FEATURE_ON)
    { AT_CMD_EONS,
      AT_SetEonsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, AT_TestEonsPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^EONS", (VOS_UINT8 *)"(1,2,3,4,5),(@plmn),(1-128)" },

    { AT_CMD_NWSCAN,
      AT_SetNwScanPara, AT_SYSCFG_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^NWSCAN", (VOS_UINT8 *)"(0-8),(@band),(0-65535)" },

    { AT_CMD_CELLROAM,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCellRoamPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CELLROAM", VOS_NULL_PTR },

    { AT_CMD_NWTIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryTimeQryPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
     (VOS_UINT8 *)"^NWTIME", VOS_NULL_PTR },

    { AT_CMD_HCSQ,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryHcsqPara, AT_QRY_PARA_TIME, AT_TestHcsqPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HCSQ", VOS_NULL_PTR },

    { AT_CMD_NETSELOPT,
      AT_SetNetSelOptPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NETSELOPT", (VOS_UINT8 *)"(1),(0-1)" },
#endif
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 查询SIMLock Manager版本号
     * [说明]: 用于查询单板软件所支持的SIMLock Manager版本号，Hi6910向终端申请的V2版本的算法。
     * [语法]:
     *     [命令]: ^AUTHVER?
     *     [结果]: <CR><LF>^AUTHVER: <value><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有单板相关错误时：
     *             <CR><LF> ERROR<CR><LF>
     * [参数]:
     *     <value>: SIMLock Manager版本号：
     *             0：表示接口不支持，无版本号；
     *             1：表示采用早期的版本进行密码获取。即1.0版本；
     *             2：表示2.0版本，采取本次优化之后的版本获取密码；
     *             其他：为保留值。
     */
    { AT_CMD_AUTHVER,
      TAF_NULL_PTR, AT_NOT_SET_TIME, AT_QryAuthverPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^AUTHVER", TAF_NULL_PTR },
};

/* 注册TAF装备AT命令表 */
VOS_UINT32 AT_RegisterCustomTafCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomTafCmdTbl, sizeof(g_atCustomTafCmdTbl) / sizeof(g_atCustomTafCmdTbl[0]));
}

