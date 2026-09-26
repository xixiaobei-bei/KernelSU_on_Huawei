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

#include "at_general_mm_cmd_tbl.h"
#include "at_general_mm_set_cmd_proc.h"
#include "at_general_mm_qry_cmd_proc.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_GENERAL_MM_CMD_TBL_C

static const AT_ParCmdElement g_atGeneralMmCmdTbl[] = {
    /*
     * [类别]: 协议AT-通用操作
     * [含义]: TE与MT握手
     * [说明]: 该命令用于TE与MT之间握手。TE启动及停止时会通过此命令发送一个消息给MT，MT在收到消息后响应TE。
     * [语法]:
     *     [命令]: ^HS=<id>,<action>
     *     [结果]: <CR><LF>^HS: <id>,<protocol>,<is_offline>,<product_class>,<product_id>,[<software_id>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^HS=?
     *     [结果]: <CR><LF>^HS: (list of supported <id>s),(list of supported <action>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <id>: 整数型，8位数字，由MT上报用于识别MT的标识。TE第一次下发时，将id设为0，而后则将id设为MT上报的值。
     *     <action>: 整数型，表示TE动作类型，取值0～1。
     *             0：TE处于Living状态（如果TE第一次发送则表示启动）；
     *             1：TE停止。
     *     <protocol>: 整数型，表示单板当前所处的通信协议状态，取值0～1。
     *             0：单板运行在APP；
     *             1：单板运行在BOOT。
     *     <is_offline>: 整数型，当前状态，取值0～2。
     *             0：online状态；
     *             1：offline状态；
     *             2：LPM（低功耗状态）。
     *     <product_class>: 整数型，产品类型。
     *             0：GW。
     *     <product_id>: 整数型，产品的ID，每一款产品华为都会分配一个唯一的ID。
     *     <software_id>: 整数型，保留。
     * [示例]:
     *     · 开机握手
     *       AT^HS=0,0
     *       ^HS: 12345678,0,1,0,55
     *       OK
     */
    { AT_CMD_HS,
      At_SetHSPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HS", (VOS_UINT8 *)"(0-99999999),(0,1)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 查询位置信息
     * [说明]: 该命令用来查询当前驻留网络的<PLMN>、<LAC>、<RAC>、<CELL ID>。
     * [语法]:
     *     [命令]: ^LOCINFO?
     *     [结果]: <CR><LF>^LOCINFO: <PLMN>,<LAC>,<RAC>,<CELL ID><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <PLMN>: PLMN信息，五个或六个字符，其中前三位是MCC，后两位或三位是MNC。
     *     <LAC>: 最大为3bytes的16进制数的字符串类型，位置码信息，例如：“0xC3”表示10进制的195。
     *             注：驻留LTE/NR下此参数表示TAC（位置码信息，格式同LAC）。
     *     <RAC>: 最大为2bytes的16进制数的字符串类型，路由区码信息，例如：“0xC3”表示10进制的195。
     *     <CELL ID>: 8bytes的16进制数的字符串类型，小区信息，类型同<LAC>。使用Half-Byte码（半字节）编码，例如0x100000005，表示高4字节是0x00000001，低4字节是0x00000005。
     * [示例]:
     *     · 查询UE当前位置信息
     *       AT^LOCINFO?
     *       ^LOCINFO:24005,0xC3,0xC3,0x5
     *       OK
     *       当前NR下，查询UE当前位置信息
     *          AT^LOCINFO?
     *          ^LOCINFO:24005,0xC3,0xC3,0x100000005
     *          OK
     */
    { AT_CMD_LOCINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryLocinfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^LOCINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 内置OPLMN列表命令
     * [说明]: 该命令用于设置和查询内置OPLMN列表，内置的OPLMN列表保存在NV项中，与SIM卡中的OPLMN列表互斥，且内置OPLMN列表的优先级高于SIM卡中的OPLMN列表。
     *         内置OPLMN特性使能受NV项中的特性开关和使能IMSI列表控制。此命令为电信需求，当前最大支持预置256条OPLMN信息，需要分6组设置，每组最大允许设置50个OPLMN。
     *         仅设置1组数据且将其length设置为0，pdu设置为空时，则表示需要清空内置的OPLMN数据。
     *         CL模式下该命令不支持。
     * [语法]:
     *     [命令]: ^EOPLMN=<version>,<index>,<length>,<pdu>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^EOPLMN?
     *     [结果]: <CR><LF>^EOPLMN: <version>,<length>,<pdu><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^EOPLMN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <version>: 字符串类型，允许配置的长度为0～7，第8位为字符串结束符。表示当前内置的OPLMN版本号。
     *     <index>: 整型值，表示设置的数据组号，取值范围为0~5。
     *             0：第1组
     *             1：第2组
     *             2：第3组
     *             3：第4组
     *             4：第5组
     *             5：第6组
     *     <length>: 整型值，表示PDU长度，取值范围为[0,500]。
     *     <pdu>: 字符串类型，长度为[0,500]。带接入技术的OPLMN，每个OPLMN由10个字节表示。
     * [示例]:
     *     · 设置内置OPLMN列表
     *       AT^EOPLMN="1.0",0,10,"64F0008080"
     *       OK
     *     · 查询内置OPLMN列表
     *       AT^EOPLMN?
     *       ^EOPLMN: "1.0",10,"64F0000808"
     *       OK
     *     · 测试内置OPLMN 列表
     *       AT^EOPLMN=?
     *       OK
     */
    { AT_CMD_EOPLMN,
      AT_SetUserCfgOPlmnPara, AT_SET_PARA_TIME, AT_QryUserCfgOPlmnPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^EOPLMN", (VOS_UINT8 *)"(@version),(0-5),(0-500),(@PDU_DATA)" },

    /*
     * [类别]: 协议AT-通用操作
     * [含义]: 设置当前UE支持的模式
     * [说明]: 该命令用于设置以及查询当前UE支持的模式。
     *         本命令遵从3GPP TS 27.007协议。
     * [语法]:
     *     [命令]: +WS46=<n>
     *     [结果]: 正常情况：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: +WS46?
     *     [结果]: <CR><LF><n><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: +WS46=?
     *     [结果]: <CR><LF> (list of supported <n>s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <+WS46=<n>：设置<n>>: <n>:integer type
     *             12: 3GPP System
     *             22,25,28,29,30,31：按照12进行处理。
     *     <+WS46=?：\n返回值<n>>: 12：GSM Digital Cellular Systems (GERAN only)
     *             22：UTRAN only
     *             25：3GPP Systems (GERAN, UTRAN and E-UTRAN)
     *             28：E-UTRAN only
     *             29：GERAN and UTRAN
     *             30：GERAN and E-UTRAN
     *             31：UTRAN and E-UTRAN
     * [示例]:
     *     · 设置命令
     *       AT+WS46=12
     *       OK
     *     · 查询命令
     *       AT+WS46?
     *       25
     *       OK
     *     · 测试命令
     *       AT+WS46=?
     *       (12,22,25,28,29,30,31)
     *       OK
     */
    { AT_CMD_WS46,
      AT_SetWs46Para, AT_SET_PARA_TIME, AT_QryWs46Para, AT_QRY_PARA_TIME, AT_TestWs46Para, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+WS46", (VOS_UINT8 *)"(12,22,25,28,29,30,31)" },

    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 指示RSSI变化
     * [说明]: 当RSSI 变化超过NdB（N可设置为1-5），并且与上次上报的时间间隔不小于M(M可设置为1-20s)时，MT主动上报指示给TE。
     *         该AT命令的主动上报受^CURC控制。
     * [语法]:
     *     [命令]: ^CERSSI=<n>,<m>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CERSSI?
     *     [结果]: 手机测试：
     *             <CR><LF>^CERSSI: <n>,<m>,<RSSI>,<RSCP>,<EC/IO>,
     *             <RSRP>,<RSRQ>,<RSSNR>,<RI>,<CQI0>,<CQI1>,<5GRSRP>,<5GRSRQ>,<5GRSSNR><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             MBB测试：
     *             <CR><LF>^CERSSI: <n>,<m>,<RSSI>,<RSCP>,<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>,<CQI0>,<CQI1>,<RXANTNUM >,
     *             <RSRPRX0>,<RSRPRX1>,<RSRPRX2>,<RSRPRX3>,<SINRRX0>,<SINRRX1>,<SINRRX2>,<SINRRX3>,<5GRSRP>,<5GRSRQ>,<5GRSSNR><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *             注意：
     *             1、在开机后还没有驻留时，使用该命令查询，MBB模式得到<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>[,<CQI0>][,<CQI1>],<rxANTNum>,<rsrpRx0>,<rsrpRx1>,<rsrpRx2>,<rsrpRx3>,<sinrRx0>,<sinrRx1>,<sinrRx2>,<sinrRx3>,<s5GRsrp>,<s5GRsrq>,<l5GSinr>参数为无效值：255, 0, 0, 0, 0, 32639, 32639, 0, 99, 99, 99, 99, 99, 99, 99, 99, 0, 0, 0
     *             2、ANDROID模式得到<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>[,<CQI0>][,<CQI1>],<s5GRsrp>,<s5GRsrq>,<l5GSinr>参数为无效值：255, 0, 0, 0, 0, 32639, 32639, 0, 0, 0
     *             3、只有在驻留状态下查询，该命令返回的信号强度才保证正确。
     *     [命令]: ^CERSSI=?
     *     [结果]: <CR><LF>^CERSSI: (list of supported <n>s), (list of supported<m>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: 1、NV_System_APP_Config配置为非ANDROID（MBB）模式: <CR><LF>^CERSSI: <RSSI>,<RSCP>,<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>,<CQI0>,<CQI1>,<RXANTNUM >,<RSRPRX0>,<RSRPRX1>,<RSRPRX2>,<RSRPRX3>,<SINRRX0>,<SINRRX1>,<SINRRX2>,<SINRRX3>,<5GRSRP>,<5GRSRQ>,<5GRSSNR><CR><LF>
     *             2、NV_System_APP_Config配置为ANDROID模式:
     *             <CR><LF>^CERSSI: <RSSI>,<RSCP>,<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>,<CQI0>,<CQI1>,<5GRSRP>,<5GRSRQ>,<5GRSSNR><CR><LF>
     * [参数]:
     *     <n>: 0：不主动上报扩展的信号质量,当<n>为0时，<m>参数无效，默认为0；
     *             1~5：当信号质量变化超过ndBm，主动上报扩展的信号质量。发生模式
     *             切换时，只有驻留小区后才进行主动上报。
     *     <m>: 0：上报扩展信号质量无时间限制；
     *             1-20：两次上报信号质量的最小间隔时间，单位：S。
     *             可选参数，不填写时默认为0。
     *     <RSSI>: UE驻留2G时服务小区的信号质量，驻留非2G小区时，设置为0。
     *     <RSCP>: UE驻留3G时服务小区的信号质量，驻留非3G小区时，设置为0。
     *     <EC/IO>: UE驻留3G FDD小区时的服务小区信噪比，驻留非3G FDD小区时，设置为255。
     *     <RSRP>: UE驻留4G时服务小区参考信号接收功率，无效值99，其余为负值。驻留非4G小区时，设置为0。
     *     <RSRQ>: UE驻留4G时服务小区参考信号接收质量，无效值99，其余为负值。驻留非4G小区时，设置为0。
     *     <RSSNR>: UE驻留4G时服务小区有效，驻留非4G小区时，设置为0。
     *             对应物理层的SINR，取值范围[-10,40]，无效值为99。
     *     <RI>: RI指示为1时，上报一个码字的CQI，保存在CQI0中，另外一个码字对应的CQI上报无效值（无效值定为0x7F7F），保存在CQI1中。
     *             RI指示为2时，上报两个码字的CQI，分别为CQI0和CQI1。
     *             UE驻留非4G时，RI指示为0，不带CQI0和CQI1参数。
     *             [CQI0]：UE驻留4G时服务小区信噪质量指示。
     *             [CQI1]：UE驻留4G时服务小区信噪质量指示。
     *     <5GRSRP>: UE 驻留5G时服务小区参考信号接收功率，范围：(-141,-44)，无效值99。驻留非5G小区时，设置为0。
     *     <5GRSRQ>: UE 驻留5G时服务小区参考信号接收质量，范围：(-40, -6) ，无效值99。驻留非5G小区时，设置为0。
     *     <5GRSSNR>: UE 驻留5G时服务小区有效，驻留非5G小区时，设置为0。对应物理层的SINR，取值范围[-10,40]，无效值为99。
     *     <RXANTNUM>: 天线的总数，最多为4个。
     *     <RSRPRX>: RSRPRX0：UE 驻留4G 时天线0的参考信号接收功率，驻留非4G 小区时，设置为99；
     *             RSRPRX1：UE驻留4G 时天线1的参考信号接收功率，驻留非4G 小区时，设置为99；
     *             RSRPRX2：UE驻留4G 时天线2的参考信号接收功率，驻留非4G 小区时，设置为99；
     *             RSRPRX3：UE驻留4G 时天线3的参考信号接收功率，驻留非4G 小区时，设置为99。
     *     <SINRRX>: SINRRX0：天线0的信噪比，UE驻留4G时服务小区有效，驻留非4G小区时，设置为99；
     *             SINRRX1：天线1的信噪比，UE驻留4G时服务小区有效，驻留非4G小区时，设置为99；
     *             SINRRX2：天线2的信噪比，UE驻留4G时服务小区有效，驻留非4G小区时，设置为99；
     *             SINRRX3：天线3的信噪比，UE驻留4G时服务小区有效，驻留非4G小区时，设置为99。
     * [示例]:
     *     · 驻留 2G，设置信号质量改变超过5dB 主动上报
     *       AT^CERSSI=5
     *       OK
     *       ?6?1 信号质量改变超过 5dB，主动上报
     *       ^CERSSI: -42,0,255,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · ?6?1 查询命令
     *       AT^CERSSI?
     *       ^CERSSI: 5,0,-42,0,255,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · 驻留 3G，设置信号质量改变超过3dB 并且间隔时间超过5s，主动上报
     *       AT^CERSSI=3,5
     *       OK
     *       ?6?1 信号质量改变超过 3dB 并且距离上次上报超过5s，主动上报
     *       ^CERSSI: 0,-68,-6,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · ?6?1 查询命令
     *       AT^CERSSI?
     *       ^CERSSI: 3,5,0,-68,-6,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · 驻留 4G，设置信号质量改变超过4dB 并且间隔时间超过5s，主动上报
     *       AT^CERSSI=4,5
     *       OK
     *       ?6?1 信号质量改变超过 4dB 并且间隔时间超过5s，主动上报
     *       ^CERSSI: 0,0,255,-61,-42,5,2,2,0,0,0,0
     *       OK
     *     · ?6?1 查询命令
     *       AT^CERSSI?
     *       ^CERSSI: 4,5,0,0,255,-61,-42,5,2,2,0,0,0,0
     *       OK
     *     · 驻留 5G，设置信号质量改变超过4dB 并且间隔时间超过5s，主动上报
     *       AT^CERSSI=4,5
     *       OK
     *       ?6?1 信号质量改变超过 4dB 并且间隔时间超过5s，主动上报
     *       ^CERSSI: 0,0,255,0,0,0,0,32639,32639,-61,-42,5
     *       OK
     *     · ?6?1 查询命令
     *       AT^CERSSI?
     *       ^CERSSI: 4,5,0,0,255,0,0,0,0,32639,32639,-61,-42,5
     *       OK
     *     · 执行测试命令
     *       AT^CERSSI=?
     *       ^CERSSI: (0,1,2,3,4,5),(0-20)
     *       OK
     */
    { AT_CMD_CERSSI,
      At_SetCerssiPara, AT_SET_PARA_TIME, At_QryCerssiPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CERSSI", (VOS_UINT8 *)"(0,1,2,3,4,5),(0-20)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 指示NSA模式下辅连接 RSSI 变化
     * [说明]: 在NSA情况下，当辅连接的RSSI 变化超过NdB（N 可设置为1-5），并且与上次上报的时间间隔不小于M(M可设置为1-20s)时，MT 主动上报指示给TE。
     *         本命令仅在支持NR的情况下才支持上报和查询，否则查询回复失败。
     * [语法]:
     *     [命令]: -
     *     [结果]: <CR><LF>^CSERSSI: <RSSI>,<RSCP>,<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>,<CQI0>,<CQI1>,<5GRSRP>,<5GRSRQ
     *             >,<5GRSSNR><CR><LF>
     *     [命令]: ^CSERSSI?
     *     [结果]: <CR><LF>^CSERSSI: <n>,<m>,<RSSI>,<RSCP>,<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>,<CQI0>,<CQI1>,<5GRSRP>,<5GRSRQ>,<5GRSSNR><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *             注意：在开机后还没有驻留时，使用该命令查询，得到的
     *             <RSSI>,<RSCP>,<EC/IO>,<RSRP>,<RSRQ>,<RSSNR>,<RI>
     *             , <CQI0>,<CQI1>,<5GRSRP>,<5GRSRQ>,<5GRSSNR>参数为无效值：0,0,255,0,0,0,0,32639,32639,0,0,0
     *             只有在驻留状态下查询，该命令返回的信号强度才保证正确
     * [参数]:
     *     <n>: 0：不主动上报扩展的信号质量,当<n>为0 时，<m>参数无效，默认为
     *             0；
     *             1~5：当信号质量变化超过ndBm，主动上报扩展的信号质量。发生模式
     *             切换时，只有驻留小区后才进行主动上报。
     *     <m>: 0：上报扩展信号质量无时间限制；
     *             1-20：两次上报信号质量的最小间隔时间，单位：S。
     *             可选参数，不填写时默认为0。
     *     <RSSI>: UE 驻留2G 时服务小区的信号质量，驻留非2G 小区时，设置为0。
     *     <RSCP>: UE 驻留3G 时服务小区的信号质量，驻留非3G 小区时，设置为0。
     *     <EC/IO>: UE 驻留3G FDD 小区时的服务小区信噪比，驻留非3G FDD 小区时，
     *             设置为255。
     *     <RSRP>: UE 驻留4G 时服务小区信噪比，无效值99，其余为负值。驻留非4G 小
     *             区时，设置为0。
     *     <RSRQ>: UE 驻留4G 时服务小区信噪比，无效值99，其余为负值。驻留非4G 小
     *             区时，设置为0。
     *     <RSSNR>: UE 驻留4G 时服务小区有效，驻留非4G 小区时，设置为0。
     *             对应物理层的SINR，无效值为99。
     *     <RI>: RI 指示为1 时，上报一个码字的CQI，保存在CQI0 中，另外一个码字
     *             对应的CQI 上报无效值（无效值定为0x7F7F），保存在CQI1 中。
     *             RI 指示为2 时，上报两个码字的CQI，分别为CQI0 和CQI1。
     *             UE 驻留非4G 时，RI 指示为0，CQI0和CQI1上报无效值（无效值为32639）。
     *             [CQI0]：UE 驻留4G 时服务小区信噪质量指示。
     *             [CQI1]：UE 驻留4G 时服务小区信噪质量指示。
     *     <5GRSRP>: UE 驻留5G时服务小区信噪比，无效值99，其余为负值。驻留非5G小区时，设置为0。
     *     <5GRSRQ>: UE 驻留5G时服务小区信噪比，无效值99，其余为负值。驻留非5G小区时，设置为0。
     *     <5GRSSNR>: UE 驻留5G时服务小区有效，驻留非5G小区时，设置为0。
     *             对应物理层的SINR，取值范围[-10,40]，无效值为99。
     * [示例]:
     *     · 设置主动上报门限为信号质量改变超过4dB 并且间隔时间超过5s
     *       AT^CERSSI=4,5
     *       AT^CSERSSI?
     *       ^CSERSSI: 4,5,0,0,255,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · 查询命令：驻留 3G
     *       AT^CSERSSI?
     *       ^CSERSSI: 4,5,0,0,255,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · 查询命令：驻留 4G，SA模式
     *       AT^CSERSSI?
     *       ^CSERSSI: 4,5,0,0,255,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · 查询命令：驻留 5G，SA模式
     *       AT^CSERSSI?
     *       ^CSERSSI: 4,5,0,0,255,0,0,0,0,32639,32639,0,0,0
     *       OK
     *     · 查询命令：驻留 4G，NSA模式
     *       AT^CSERSSI?
     *       ^CSERSSI: 4,5,0,0,255,0,0,0,0,32639,32639,-61,-42,5
     *       OK
     *       主动上报：驻留 4G，NSA模式
     *       ?6?1 NR信号质量改变超过 4dB 并且间隔时间超过5s，主动上报
     *       ^CSERSSI: 0,0,255,0,0,0,0,32639,32639,-61,-42,5
     *       OK
     */
    { AT_CMD_CSERSSI,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryCserssiPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CSERSSI", VOS_NULL_PTR },
#endif
};

/* 注册LMM通用AT命令表 */
VOS_UINT32 AT_RegisterGeneralMmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atGeneralMmCmdTbl, sizeof(g_atGeneralMmCmdTbl) / sizeof(g_atGeneralMmCmdTbl[0]));
}

