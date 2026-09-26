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
#include "at_custom_as_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"
#include "at_lte_common.h"

#include "at_custom_as_set_cmd_proc.h"
#include "at_custom_as_qry_cmd_proc.h"



#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_AS_CMD_TBL_C

static const AT_ParCmdElement g_atCustomAsCmdTbl[] = {
#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 触发GPS Fine Time/SFN脉冲
     * [说明]: 该命令用于提供给GPS芯片获取高精度网络时间。
     *         命令为异步处理，返回OK后，触发对应物理层的脉冲上报，脉冲对应的网络时间则通过^FINETIMEINFO命令或者^SFN命令主动上报。
     * [语法]:
     *     [命令]: ^FINETIMEREQ=<type>[,<num>[,<rat>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             目前仅支持LTE模式，其余模式时命令返回ERROR。
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^FINETIMEREQ=?
     *     [结果]: <CR><LF>^FINETIMEREQ: (list of supported <type>s),(1-256), (list of supported <rat>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整数类型，触发脉冲类型：
     *             0：强制触发脉冲（虚焊检测使用）；
     *             1：触发GPS Fine Time（SYNC时钟）脉冲；
     *             2：触发SFN帧脉冲（仅限LTE模式）。
     *     <num>: 整数型，脉冲触发的个数，取值范围1~256，<type>为0强制触发脉冲或1 SYNC脉冲时本参数有效，参数不下发则默认值为触发2个SYNC脉冲。
     *     <rat>: 整数类型，提供Fine Time的接入技术，参数不下发默认为LTE：
     *             0：GSM（暂不支持）；
     *             1：WCDMA（暂不支持）；
     *             2：LTE；
     *             3：TDS-CDMA（暂不支持）；
     *             4：CDMA_1X（暂不支持）；
     *             5：HRPD（暂不支持）；
     *             6：NR。
     * [示例]:
     *     · 查询GPS Fine Time并触发普通脉冲
     *       AT^FINETIMEREQ=1
     *       OK
     *     · 虚焊检测触发脉冲
     *       AT^FINETIMEREQ=0,250,6
     *       OK
     *     · 执行测试命令
     *       AT^FINETIMEREQ=?
     *       ^FINETIMEREQ: (0-2),(1-256),(2,6)
     *       OK
     */
    { AT_CMD_FINE_TIME,
      AT_SetFineTimeReqPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^FINETIMEREQ", (TAF_UINT8 *)"(0-2),(1-256),(2,6)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询增强型小区ID信息
     * [说明]: 该命令用来提供所有支持RAT（GSM，WCDMA，LTE，NR）的增强的小区ID信息。增强型小区ID信息包括服务小区和相邻小区信息。GPS芯片将使用这些信息来支持SUPL1.0和2.0的位置标识。
     * [语法]:
     *     [命令]: +ECID=<version>
     *     [结果]: 执行成功时：
     *             <CR><LF>+ECID=<version>,<rat>,[<cell_description>] <CR><LF><CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <version>: 整型值，命令版本号，当前为0。
     *     <rat>: 字符串类型，当前的无线接入技术。
     *             “NONE:”：当前Modem处于飞行模式或没有小区ID信息；
     *             “GSM:”：当前接入技术为GSM；
     *             “WCDMA:”：当前接入技术为WCDMA；
     *             “LTE:”：当前接入技术为LTE;
     *             “NR:”：当前接入技术为NR。
     *     <cell_description>: 增强的小区ID信息。
     *             接入技术为GSM时：
     *             <MCC>,<MNC>,<LAC>,<CID>,<TA>,<NB_CELL> [,<BSIC>,<ARFCN>,<RX_level>]*NB_CELL
     *             接入技术为WCDMA时：
     *             <MCC>,<MNC>,<UCID>,<Primary_Scrambling>, <UARFCN_UL>,<UARFCN_DL>,<NB_FREQ> [,<UARFCN_UL>,<UARFCN_DL>,<UC_RSSI>,<CMR_NB> [,<Cell_ID>, <Primary_CPICH>,<CPICH_Ec_NO>, <CPICH_RSCP>,<Pathloss>]*CMR_NB]*NB_FREQ
     *             接入技术为LTE时：
     *             <MCC>,<MNC>,<TAC>,<LCID>,<L_NB_CELL> [,<Cell_Code>,<EARFCN>,<RSRP>,<RSRQ>,<L_TA>]*NB_CELL
     *             说明：LTE时小区列表中第一个小区为服务小区，其余的为邻区。<TA>参数只在服务小区使用。
     *             接入技术为NR时：
     *             <MCC>,<MNC>,<TAC>,<NCID>,<N_NB_CELL> [,<Cell_Code>,<ARFCN-NR>,<5GSCS>, <5GRSRP>, <5GRSRQ>,<NR_TA>]*NB_CELL
     *             NR时小区列表中第一个小区为服务小区，其余的为邻区。<TA>参数只在服务小区使用。
     *     <MCC>: 整型值，当前国家码。
     *     <MNC>: 整型值，当前网络码。
     *     <LAC>: 整型值，当前位置区码。
     *     <CID>: 整型值，当前小区ID。
     *     <TA>: 整型值，当前时间提前量，不支持返回-1。
     *     <NB_CELL>: 整型值，邻区个数。
     *     <BSIC>: 整型值，该邻区的小区基站码。
     *     <ARFCN>: 整型值，该邻区的频点。
     *     <RX_level>: 整型值，该邻区的测量电平。
     *     <UCID>: 整型值，UTRA当前小区ID。
     *     <Primary_Scrambling>: 整型值，主扰码，不支持返回-1。
     *     <UARFCN_UL>: 整型值，上行频点，不支持返回-1。
     *     <UARFCN_DL>: 整型值，下行频点，不支持返回-1。
     *     <NB_FREQ>: 整型值，频点个数。
     *     <UC_RSSI>: 整型值，接收信号强度。
     *     <CMR_NB>: 整型值，该频点下测量的邻区个数。
     *     <Cell_ID>: 整型值，邻区的小区ID，不支持返回-1。
     *     <Primary_CPICH>: 整型值，邻区的主扰码。
     *     <CPICH_Ec_NO>: 整型值，邻区的EC NO，不支持返回-1。
     *     <CPICH_RSCP>: 整型值，邻区的RSCP，不支持返回-127。
     *     <Pathloss>: 整型值，邻区的Pathloss，不支持返回-1。
     *     <TAC>: 整型值，小区跟踪码。
     *     <LCID>: 整型值，LTE当前小区ID。
     *     <L_NB_CELL>: 整型值，LTE小区个数。
     *     <Cell_Code>: 整型值，小区码，PhysCellId。
     *     <EARFCN>: 整型值，小区频点，不支持返回-1。
     *     <RSRP>: 整型值，参考信号接收功率。
     *     <RSRQ>: 整型值，参考信号接收质量。
     *     <L_TA>: 整型值，LTE的时间提前量，不支持返回-1。
     *     <NCID>: 整型值，NR当前小区ID，Cell Identity。
     *     <ARFCN-NR>: 整型值，NR小区SSB频点，不支持返回-1。
     *     <N_NB_CELL>: 整型值，NR小区个数。
     *     <5GSCS>: 整型值，NR副载波间隔信息：
     *             0：15kHz；
     *             1：30kHz；
     *             2：60kHz；
     *             3：120kHz；
     *             4：240khz。
     *     <5GRSRP>: 整型值，NR参考信号接收功率，取值范围0~127。
     *     <5GRSRQ>: 整型值，NR参考信号接收质量，取值范围0~127。
     *     <NR_TA>: 整型值，NR的时间提前量，不支持返回-1。
     * [示例]:
     *     · 当前无任何小区信息
     *       AT+ECID=0
     *       +ECID=0,NONE:,
     *       OK
     *     · 当前接入技术为GSM，存在1个服务小区及2个邻区
     *       AT+ECID=0
     *       +ECID=0,GSM:,310,410,6042,4626,3,2,1,653,39,19,234,36
     *       OK
     *     · 当前接入技术为WCDMA，存在1个服务小区、3个频点及30个邻区
     *       AT+ECID=0
     *       +ECID=0,WCDMA:,310,410,131353446,447,1314,1539,3,1314,1539,63,24,1,204,15,14,-1,2,183,-1,-127,-1,3,168,-1,-127,-1,4,104,7,10,-1,5,402,-1,-127,-1,6,186,-1,-127,-1,7,480,13,13,-1,8,420,-1,-127,-1,9,391,-1,-127,-1,10,374,14,14,-1,11,347,-1,-127,-1,12,167,-1,-127,-1,13,137,-1,-127,-1,14,99,-1,-127,-1,15,133,-1,-127,-1,16,199,-1,-127,-1,17,320,-1,-127,-1,18,470,-1,-127,-1,19,451,-1,-127,-1,20,120,-1,-127,-1,21,306,-1,-127,-1,22,411,-1,-127,-1,23,110,9,13,-1,24,144,-1,-127,-1,212,612,78,3,0,420,-1,-127,-1,1,402,-1,-127,-1,2,447,20,17,-1,1687,1912,83,3,3,447,18,14,-1,4,402,-1,-127,-1,5,420,-1,-127,-1
     *       OK
     *     · 当前接入技术为LTE，存在1个服务小区及7个邻区
     *       AT+ECID=0
     *       +ECID=0,LTE:,310,410,35653,169078288,8,265,12345,60,25,256,289,23456,42,0,-1,290,34567,48,19,-1,19,45678,37,9,-1,20,56789,38,10,-1,21,01234,39,11,1,22,02345,40,12,2,23,03456,41,13,3
     *       OK
     */
    { AT_CMD_ECID,
      AT_SetEcidPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+ECID", (VOS_UINT8 *)"(0)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询当前服务小区参数
     * [说明]: 该命令用于查询当前服务小区功能。
     *         <PSC>,<Cell_ID>,<LAC>,<RSCP>,<RXLEV>,<EC/N0>,<DRX>,<URA>这些参数，在第一次驻留小区、小区重选、小区切换、空闲态切换到连接态时，可能短时间内获取不到有效值，出现留空的情况。
     *         第一次驻留LTE小区时，短时间内可能无法获取此参数的有效值，出现留空的情况。
     *         GUL下，如果L的NV 53877配置NETSCAN的开关没开的话，不会上报L的服务小区信息。
     *         RSRP、RSRQ、SINR目前测量上报是基于SS的测量结果，CSI-RS测量结果上报暂不支持。
     * [语法]:
     *     [命令]: ^MONSC
     *     [结果]: <CR><LF>^MONSC: <RAT>[,<cell_paras>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             无服务状态时：
     *             <CR><LF> ERROR<CR><LF>
     * [参数]:
     *     <RAT>: 接入技术，字符串类型：
     *             GSM：驻留在GSM小区，后续有<cell_paras>参数；
     *             WCDMA：驻留在WCDMA小区，后续有<cell_paras>参数；
     *             TD_SCDMA：驻留在TD_SCDMA小区，后续有<cell_paras>参数；
     *             LTE：驻留在LTE小区，后续有<cell_paras>参数；
     *             NR：驻留在NR小区，后续有<cell_paras>参数 ；
     *             NONE：无网络，后续无<cell_paras>参数。
     *     <cell_paras>: 服务小区参数，根据接入技术不同，返回不同参数。返回已经获取到的参数，参数随网络情况变化而改变，暂时未获取到的参数留空。
     *     <GSM网络模式参数>:
     *     <MCC>: 移动国家码，整型值。
     *     <MNC>: 移动网络码，整型值。
     *     <BAND>: 频段，整型值，范围（0-3）：
     *             0：GSM 850；
     *             1：GSM 900；
     *             2：GSM 1800；
     *             3：GSM 1900。
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，整型值，取值范围：0~16383。
     *     <BSIC>: 小区基站码，十进制，取值范围：0~63。
     *     <Cell_ID>: 十六进制，SIB3消息中的CELL IDENTITY参数，取值范围：0~FFFF。
     *     <LAC>: 位置区码，十六进制，取值范围：0~FFFF。
     *     <RXLEV>: 测量值，整型值，单位dbm，取值范围：-120~-37。
     *             （在第一次驻留小区、小区重选、小区切换时，可能短时间无法获取有效值，出现留空的情况）
     *     <RxQuality>: 测量质量，传输态、专用态有效，整型值，取值范围：0~7。
     *     <TA>: 时间提前量，在语音或者数据连接态时才能获取有效值，其他情况留空，整型值，取值范围：0~63。
     *     <WCDMA网络模式参数>:
     *     <MCC>: 移动国家码，整型值
     *     <MNC>: 移动网络码，整型值
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，整型值，取值范围：0~16383。
     *     <PSC>: 主扰码，整型值，取值范围：0~511。
     *     <Cell_ID>: 十六进制，SIB3消息中的CELL IDENTITY参数，包含RNC ID和 CELL ID，取值范围：0~FFFFFFF。
     *     <LAC>: 位置区码，十六进制，取值范围：0~FFFF。
     *     <RSCP>: 接收信号码功率，Received Signal Code Power in dBm，整型值，取值范围：-120~-25。
     *     <RXLEV>: 测量值，Receiving signal strength in dbm，整型值，单位dbm，取值范围：-120~-25。
     *     <EC/N0>: 每调制比特功率和噪声频谱密度的比率，整型值
     *     <DRX>: Discontinuous reception cycle length，整型值，只返回CS DRX值，取值范围：6~9。
     *     <URA>: UTRAN Registration Area Identity，整型值，取值范围：0~65535。
     *     <LTE网络模式参数>:
     *     <MCC>: 移动国家码，整型值。
     *     <MNC>: 移动网络码，整型值。
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，整型值，取值范围：0~ 133121。
     *     <Cell_ID>: SIB1中的cell identity，十六进制，取值范围：0~0xFFFFFFF。
     *     <PCI>: 物理小区ID，整型值，十六进制，取值范围：0~0x1F7。
     *     <TAC>: 跟踪区域码，Tracking Area Code，十六进制，取值范围：0~0xFFFF。
     *     <RSRP>: 参考信号接收功率，Reference Signal Received Power，整型值，取值范围：-140~-44，单位：dBm。
     *     <RSRQ>: 参考信号接收质量，Reference Signal Received Quality，整型值，取值范围：-19.5~-3，单位：dB。
     *     <RSSI>: 信号强度，Receiving signal strength in dbm，整型值，取值范围：-90~-25，单位：dBm。
     *     <TD_SCDMA网络模式参数>:
     *     <MCC>: 移动国家码，整型值。
     *     <MNC>: 移动网络码，整型值。
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，（9400-9600）、（10050-10125）、（11500-12000）
     *     <SYNC_ID>: 下行导频码，整型值，取值范围：0~31。
     *     <SC>: 扰码，整型值，取值范围：0~127。
     *     <CELL_ID>: cell identity，十六进制，取值范围：0~FFFFFFF。
     *     <LAC>: 位置区码，十六进制，取值范围：0~FFFF。
     *     <RSCP>: Received Signal Code Power in dBm，接收信号码功率，取值范围：-116~0。
     *     <DRX>: Discontinuous reception cycle length，整型值，取值范围：6~9。
     *     <RAC>: 路由区编号，十六进制，取值范围：0~FFFF。
     *     <NR网络模式参数>:
     *     <MCC>: 移动国家码，整型值。
     *     <MNC>: 移动网络码，整型值。
     *     <ARFCN-NR>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，整型值，取值范围：0~ 2279166。
     *     <SCS>: SubcarrierSpacing information，整型值，范围（0-4）：
     *             0：15kHz；
     *             1：30kHz；
     *             2：60kHz；
     *             3：120kHz；
     *             4：240khz。
     *     <Cell_ID>: Cell identity，十六进制。取值范围：0~0xFFFFFFFFFFFFFFFF
     *     <PCI>: 物理小区ID，十六进制，取值范围：0~0x3EF。
     *     <TAC>: 跟踪区域码，Tracking Area Code，十六进制，取值范围：0~0xFFFFFF。
     *     <RSRP>: 参考信号接收功率，Reference Signal Received Power，整型值，取值范围：-156~-31，无效值：-1256(-157*8)，单位：dBm。
     *     <RSRQ>: 参考信号接收质量，Reference Signal Received Quality，整型值，取值范围：-43~20，无效值：-348 (-43.5*8)，单位：dB。
     *     <SINR>: 信号强度，Receiving signal strength in dbm，整型值，取值范围：-23~40，无效值：-188(-23.5*8)，单位：dBm。
     * [示例]:
     *     · 查询当前服务小区
     *       AT^MONSC
     *       ^MONSC: LTE,460,01,1650,A5C933,1F3,183D,-98,-5,-72
     *       OK
     *     · 查询当前NR服务小区
     *       AT^MONSC
     *       ^MONSC: NR,460,01,1650,1,AB12345678,1F3,1234AB,-80,-10,32
     *       OK
     */
    { AT_CMD_MONSC,
      At_SetNetMonSCellPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MONSC", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询当前邻区参数
     * [说明]: 该命令用于查询当前邻区参数功能。
     *         <BSIC>,<Cell_ID>,<LAC>这些参数，在网络环境不稳定时，可能在短时间内无法获取有效值，出现留空的情况。<LAC>,<Cell_ID>在服务小区是LTE小区时，获取不到有效值，留空。
     *         NR下可能上报LTE的邻区和NR的邻区，不会上报G或W的邻区。L下可能上报G、W、L、NR的邻区。
     *         RSRP、RSRQ、SINR目前测量上报是基于SS的测量结果，CSI-RS测量结果上报暂不支持。
     * [语法]:
     *     [命令]: ^MONNC
     *     [结果]: <CR><LF>^MONNC: <RAT>[,<cell_paras>] [<CR><LF>^MONNC: <RAT>[,<cell_paras>]][…]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <RAT>: 接入技术，字符串表示：
     *             GSM：邻区是GSM小区；
     *             WCDMA：邻区是WCDMA小区；
     *             TD_SCDMA：邻区是TD_SCDMA小区；
     *             LTE：邻区是LTE小区；
     *             NR：邻区是NR小区;
     *             NONE：无邻区，后续无<cell_paras>参数。
     *     <cell_paras>: 临区的参数，根据接入技术不同，返回不同参数。返回已经获取到的参数，参数随网络情况变化而改变，暂时未获取到的参数留空。
     *     <GSM网络模式参数>:
     *     <BAND>: 频段，整型值，范围（0-3）：
     *             0：GSM 850；
     *             1：GSM 900；
     *             2：GSM 1800；
     *             3：GSM 1900。
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，取值范围：0~1023。
     *     <BSIC>: 小区基站码，取值范围：0~63。
     *     <Cell_ID>: 十六进制，SIB3消息中的CELL IDENTITY参数，取值范围：0~FFFF。
     *             (服务小区是LTE时，无法获取此邻区参数)
     *     <LAC>: 位置区码，取值范围：0~FFFF。
     *             (服务小区是LTE时，无法获取此邻区参数)
     *     <RXLEV>: Receiving signal strength in dbm，取值范围：-120~-37。
     *     <WCDMA网络模式参数>:
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，取值范围：0~16383。
     *     <PSC>: 主扰码，取值范围：0~511。
     *     <RSCP>: Received Signal Code Power in dBm，接收信号码功率，取值范围：-120~-25。
     *     <EC/N0>: 每调制比特功率和噪声频谱密度的比率，取值范围：-25~0。
     *     <LTE网络模式参数>:
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，取值范围：0~133121。
     *     <PCI>: 物理小区ID，整型值，取值范围：0~503。
     *     <RSRP>: Reference Signal Received Power, 参考信号接收功率，整型值，取值范围：-140~-44，单位：dBm。
     *     <RSRQ>: Reference Signal Received Quality, 参考信号接收质量，整型值，取值范围：-19.5~-3，单位：dB。
     *     <RXLEV>: Receiving signal strength in dbm，整型值，单位：-90~-25，单位：dBm。
     *     <TD_SCDMA网络模式参数>:
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier, （9400-9600）、（10050-10125）、（11500-12000）
     *     <SC>: 扰码，整型值，取值范围：0~127。
     *     <SYNC_ID>: 下行导频码，整型值，取值范围：0~31。
     *     <RSCP>: Received Signal Code Power in dBm，接收信号码功率，整型值，取值范围：-116~0。
     *     <NR的网络模式参数>:
     *     <ARFCN-NR>: 频点，Absolute Radio Frequency Channel Number of the BCCH carrier，取值范围：0~ 2279166。
     *     <PCI>: 物理小区ID，十六进制，取值范围：0~0x3EF。
     *     <RSRP>: 参考信号接收功率，Reference Signal Received Power，整型值，取值范围：-156~-31，无效值：-1256(-157*8)，单位：dBm。
     *     <RSRQ>: 参考信号接收质量，Reference Signal Received Quality，整型值，取值范围：-43~20，无效值：-348 (-43.5*8)，单位：dB。
     *     <SINR>: 信号强度，Receiving signal strength in dbm，整型值，取值范围：-23~40，无效值：-188(-23.5*8)，单位：dBm。
     * [示例]:
     *     · 查询当前邻区
     *       AT^MONNC
     *       ^MONNC: GSM,3,128,9,0,0,-66
     *       ^MONNC: GSM,1,130,9,0,0,-69
     *       ^MONNC: WCDMA,10750,301,-78,-2
     *       ^MONNC: WCDMA,10562,351,-79,-3
     *       OK
     *     · 当前NR下，查询当前邻区
     *       AT^MONNC
     *       ^MONNC: LTE,10000,1F,-82,-11,-1
     *       ^MONNC: NR,10001,1F3,-80,-10,32
     *       OK
     */
    { AT_CMD_MONNC,
      At_SetNetMonNCellPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MONNC", VOS_NULL_PTR },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NSA情况下查询辅连接服务小区信息功能
     * [说明]: 该命令用于查询NSA情况下查询辅连接服务小区的小区信息。
     *         本命令不在NSA的情况下都会返回查询失败。
     *         本命令暂最大支持8CC场景。
     * [语法]:
     *     [命令]: ^MONSSC
     *     [结果]: <CR><LF>^MONSSC: <RAT>[,<cell_paras>]
     *             [<CR><LF>^MONSSC: <RAT>[,<cell_paras>]][,…]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             有查询相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <RAT>: 接入技术，字符串表示：
     *             NR：ENDC下返回NR服务小区；
     *             LTE：NEDC 下返回LTE服务小区；
     *             NONE：非ENDC 状态，后续无<cell_paras>参数。
     *     <cell_paras>: 邻区的参数，根据接入技术不同，返回不同参数。返回已经获取到的参数，参数随网络情况变化而改变，暂时未获取到的参数留空。
     *             NR 下的参数：<ARFCNNR>,<PCI>,<RSRP>,<RSRQ>,<SINR>,<MEASTYPE>
     *             LTE 下的参数（目前暂不支持，待将来开发再补充）
     *     <ARFCN>: 频点，Absolute Radio Frequency Channel Number of the BCCH
     *             carrier，取值范围：0~ 2279166。
     *     <PCI>: 物理小区ID，十六进制，取值范围：0~0x3EF。
     *     <RSRP>: 参考信号接收功率，Reference Signal Received Power，整型值，取值范围：-156~-31，无效值：-1256(-157*8)，单位：dBm。
     *     <RSRQ>: 参考信号接收质量，Reference Signal Received Quality，整型值，取值范围：-43~20，无效值：-348 (-43.5*8)，单位：dB。
     *     <SINR>: 信号强度，Receiving signal strength in dbm，整型值，取值范围：-23~40，无效值：-188(-23.5*8)，单位：dBm。
     *     <MEASTYPE>: <RSRP><RSRQ><SINR>取值的对应测量方法：0：SSB测量，1：CSI-RS测量，当RSRP/RSRQ/SINR填无效值时，此类型填写BUTT。
     * [示例]:
     *     · 查询辅连接为单CC场景
     *       AT^MONSSC
     *       ^MONSSC: NR,633988,1,-70,-20,-10,0
     *       OK
     *     · 查询辅连接为2CC场景
     *       AT^MONSSC
     *       ^MONSSC: NR,633988,1,-70,-20,-10,0
     *       ^MONSSC: NR,633400,2,-68,-10,1,0
     *       OK
     *     · 查询辅连接为2CC场景但是Scell当前还没有上报信号
     *       AT^MONSSC
     *       ^MONSSC: NR,633988,1,-70,-20,-10,1
     *       ^MONSSC: NR,633400,2,-1256,-348,-188,2
     *       OK
     *     · 查询的小区不支持辅连接
     *       AT^MONSSC
     *       ^MONSSC: NONE
     *       OK
     */
    { AT_CMD_MONSSC,
      At_SetNetMonSSCellPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^MONSSC", VOS_NULL_PTR },

    { AT_CMD_NRCELLBAND,
      At_SetNrCellBandPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRCELLBAND", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: ERRC的LTE控制能力配置
     * [说明]: 通过这个命令可以动态打开和关闭ERRC的某个类型的控制能力。
     * [语法]:
     *     [命令]: ^ERRCCAPCFG=<cap_type>,<para1>[,<para2>[,<para3>]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^ERRCCAPCFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cap_type>: 整型值，ERRC的LTE能力类型，取值范围0~255。
     *             目前实现0~5，未实现的为预留值。
     *             每个取值表示不同类型的LTE控制能力，比如:
     *             0：是否支持CA相关能力
     *             1：是否支持catg15,16,17,18能力
     *             2：CA下行最大支持MIMO层数
     *             3：最大支持的载波个数
     *             4：EN_DC状态下MIMO层数 & 载波个数
     *             5：释放NR链路，优先通过上报overheating方式释放，如果overheating不支持或网络不释放，再通过SCG failure方式释放
     *     <para1>: 整型值，设置对应类型LTE控制能力的的第一项参数，参数具体取值范围受平台实际规格限制。
     *             cap_type0：0表示关闭CA，1表示支持CA
     *             cap_type1：0表示关闭catg15,16,17,18能力，1表示支持catg15,16,17,18能力。
     *             cap_type2：0表示关闭，其它值可取 2,4,8 ，表示MIMO层数。
     *             cap_type3：0表示关闭，载波个数，范围为[1，芯片支持的最大CA载波个数],目前芯片支持的最大CA载波个数为5。
     *             cap_type4： MIMO层数的值，0表示关闭，其它值可取 2、4，如果没有携带具体参数默认配0。
     *             cap_type5：0表示enable EN-DC，1表示disable EN-DC。
     *     <para2>: 整型值，设置对应类型LTE控制能力的的第二项参数:
     *             说明：
     *             该参数是可选项，如果该控制能力不足2个子能力则可以不配置该参数
     *             cap_type4：载波个数，0表示关闭，范围为[1，芯片支持的最大CA载波个数]，目前芯片支持的最大CA载波个数为5，如果没有携带具体参数默认配0。
     *             cap_type5：0表示发送TAU信息，1表示不发送TAU信息。
     *     <para3>: 整型值，设置对应类型LTE控制能力的的第三项参数：
     *             说明：
     *             该参数是可选项，如果该控制能力不足3个子能力则可以不配置该参数
     * [示例]:
     *     · 打开ERRC类型为1的控制能力,该控制能力只有1个参数
     *       AT^ERRCCAPCFG=1,1
     *       OK
     *     · 关闭ERRC类型为1的控制能力，该控制能力只有1个参数
     *       AT^ERRCCAPCFG=1,0
     *       OK
     */
    { AT_CMD_ERRCCAPCFG,
      AT_SetErrcCapCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ERRCCAPCFG", (VOS_UINT8 *)"(0-255),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: ERRC的LTE控制能力查询
     * [说明]: 通过该AT命令查询ERRC某个类型控制能力配置情况。
     * [语法]:
     *     [命令]: ^ERRCCAPQRY=<cap_type>
     *     [结果]: 执行成功时：
     *             ^ERRCCAPQRY: <cap_type>[<para1>,<para2>，<para3>]
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^ERRCCAPQRY=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cap_type>: 整型值，ERRC的LTE能力类型，取值范围0~255。
     *             目前实现0~5，未实现的为预留值。
     *             每个取值表示不同类型的LTE控制能力，比如:
     *             0：是否支持CA相关能力
     *             1：是否支持catg15,16,17,18能力
     *             2：CA下行最大支持MIMO层数
     *             3：最大支持的Scell个数
     *             4：EN_DC状态下MIMO层数 & 载波个数
     *             5：通过SCG failure释放NR链路
     *     <para1>: 整型值，设置对应类型LTE控制能力的的第一项参数
     *     <para2>: 整型值，设置对应类型LTE控制能力的的第一项参数
     *     <para3>: 整型值，设置对应类型LTE控制能力的的第一项参数
     * [示例]:
     *     · 查询ERRC的类型为2的控制能力
     *       AT^ERRCCAPQRY=2
     *       ^ERRCCAPQRY:2,1,0,1
     *       OK
     */
    { AT_CMD_ERRCCAPQRY,
      AT_SetErrcCapQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ERRCCAPQRY", (VOS_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 疑似伪基站小区信息通知
     * [说明]: 该命令用于AP侧智能识别疑似伪小区后，将信息通知到modem。
     * [语法]:
     *     [命令]: ^PSEUCELL=<Type>,<Rat>,<PLMN>,<Lac>,<CellID>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <Type>: 通知类型：整型值
     *             0：通过短信识别的伪小区
     *             目前只支持0。
     *     <Rat>: 接入技术：整型值
     *             1：GSM
     *             3：WCDMA
     *             4：TD-SCDMA
     *             6：LTE
     *             目前只支持1:GSM。
     *     <PLMN>: 网络PLMN信息：
     *             字符串类型，运营商信息。
     *     <Lac>: 位置区码：
     *             2byte的16进制数的字符串类型，位置码信息，例如：“00C3”表示10进制的195。
     *     <CellID>: 小区ID：
     *             4byte的16进制数的字符串类型，小区信息。
     * [示例]:
     *     · 设置疑似伪基站小区信息
     *       AT^PSEUCELL=0,1,"46000","00C3","0000001A"
     *       OK
     *     · 查询疑似伪基站小区信息
     *       AT^PSEUCELL?
     *       ERROR
     *     · 测试命令
     *       AT^PSEUCELL=?
     *       ERROR
     */
    { AT_CMD_PSEUCELL,
      AT_SetPseucellInfoPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^PSEUCELL", (VOS_UINT8 *)"(0),(1,3,4,6),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 配置ECC状态主动上报
     * [说明]: 通过这个命令可以动态打开和关闭ECC上报功能。
     * [语法]:
     *     [命令]: ^ECCCFG=<enable>,<interval>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^ECCCFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，有源ECC使能：
     *             0：关闭上报，
     *             1：打开上报。
     *     <interval>: 整型值，周期上报时间间隔，单位毫秒，推荐值1500，不允许小于1000。
     * [示例]:
     *     · 打开ECC上报功能
     *       AT^ECCCFG=1,1500
     *       OK
     */
    { AT_CMD_ECCCFG,
      AT_SetEccCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ECCCFG", (VOS_UINT8 *)"(0-1),(1000-65535)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询伪基站相关参数
     * [说明]: 本命令用于AP侧查询伪基站是否支持或伪基站拦截次数。
     * [语法]:
     *     [命令]: ^PSEUDBTS=<RAT>,<TYPE>
     *     [结果]: 执行正确时：
     *             <CR><LF>^PSEUDBTS: <RSLT><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *             注：不支持查询命令和测试命令
     * [参数]:
     *     <RAT>: 1:GSM
     *             2:WCDMA（暂不支持）
     *             3:LTE
     *     <TYPE>: 1:查询伪基站是否支持
     *             2:查询伪基站拦截次数
     *     <RSLT>: TYPE为1时，对应的伪基站是否支持（返回值1表示支持，0表示不支持）
     *             0：不支持；
     *             1：支持；
     *             TYPE为2时，对应的伪基站拦截次数
     * [示例]:
     *     · AP查询2G伪基站拦截次数：
     *       AT^PSEUDBTS=1,2
     *       ^PSEUDBTS: 10
     *       OK
     */
    { AT_CMD_PSEUDBTS,
      AT_SetPseudBtsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^PSEUDBTS", (VOS_UINT8 *)"(1,3),(1,2)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置机器学习子分类器模型参数
     * [说明]: 本命令用于AP侧把机器学习子分类器模型的云数据参数下发给MODEM。
     * [语法]:
     *     [命令]: ^SUBCLFSPARAM=<SEQ>,<VER>,<Type>,<FUNCSET>,<PROBA>,<CLFSNUM>[[,<BS1>[,<BS2>[,
     *             <BS3>]]]]
     *     [结果]: 执行正确时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *             注：测试命令直接返回ERROR
     *     [命令]: ^SUBCLFSPARAM?
     *     [结果]: <CR><LF>^SUBCLFSPARAM:<VER><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <SEQ>: 流水号：取值范围为1至255，最后一条为255，若只有一条，则为255
     *     <VER>: 云通讯版本号，格式为xx.xx.xxx
     *     <Type>: 子分类器类型：
     *             0：机器学习GSM伪基站模型参数
     *             目前只支持0（其他值按失败处理）
     *     <FUNC SET>: 子分类器类型对应的功能设置：
     *             0：功能关闭
     *             1：功能打开
     *     <PROBA>: 机器学习概率门限参数。
     *             参数范围为0~99999的整数
     *             （例如：99999，表示99.999%的概率。机器学习算法使用时自行转换）
     *     <CLFS NUM>: 该条AT命令中CLFS参数组数。
     *             组数为0时，仅设置<FUNC SET>开关，不更新BS参数。
     *             如果CLFS NUM组数不等于模型参数（BS1，BS2和BS3）的总组数，直接返回失败。
     *     <BS1>: CLFS模型参数数据。
     *             BS1和BS2,BS3可能有，也可能没有。每个BS最多500字符。
     *             机器学习模型参数（Type0）格式：
     *             每组包含4个参数，前3个为整型数，最后1个数为“1”或“-1”，每个数字以逗号隔开。例如：
     *             1200,2,2782075,1,
     *             6399,3,774506,-1
     *     <BS2>: 格式同BS1，每组包含4个参数，以逗号隔开。
     *     <BS3>: 格式同BS1，每组包含4个参数，以逗号隔开。
     * [示例]:
     *     · AP下发机器学习模型参数命令：
     *       AT^SUBCLFSPARAM=1,"01.01.001",0,1,99999[g1],2,"1200,2,2782075,1,6399,3,774506,-1"
     *       OK
     *       AT^SUBCLFSPARAM=255,"01.01.001",0,1,99999[g2],1,"4800,0,1261034,1"
     *       OK
     *     · AP下发设置机器学习GSM伪基站功能关闭命令：
     *       AT^SUBCLFSPARAM=255,"01.01.001",0,0,99999[g3],0
     *       OK
     *     · 查询命令：
     *       AT^SUBCLFSPARAM?
     *       ^SUBCLFSPARAM: "01.01.001"
     *       OK
     */
    { AT_CMD_SUBCLFSPARAM,
      AT_SetSubClfsParamPara, AT_SET_PARA_TIME, AT_QrySubClfsParamPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^SUBCLFSPARAM", (VOS_UINT8 *)"(1-255),(str),(0),(0,1),(0-99999),(0-65535),(str),(str),(str)" },

#if (FEATURE_LTE == FEATURE_ON)
#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 用于通知LRRC主动恢复连接快速填加SCG
     * [说明]: 主动快速回5G（NSA）需求，在满足快速填加SCG场景条件下，AP下发主动链路恢复快速添加SCG的操作。5G下使用。
     * [语法]:
     *     [命令]: ^CONNECTRECOVERY
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     * [示例]:
     *     · AP下发主动链路恢复快速添加SCG请求
     *       AT^CONNECTRECOVERY
     *       OK
     */
    { AT_CMD_CONNECT_RECOVERY,
      AT_SetConnectRecovery, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8*)"^CONNECTRECOVERY", VOS_NULL_PTR },
#endif
#endif

    { AT_CMD_TFDPDT,
      At_SetTfDpdtPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^TFDPDT", (VOS_UINT8 *)DPDT_CMD_PARA_STRING },

#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置LTE的UE能力数据参数
     * [说明]: 本命令用于AP侧将云端或者版本预置的随PLMN定制LTE的UE能力数据参数下发给MODEM，和^LRRCUECAPINFONTF命令配合使用。
     *         注：AT命令总长度不超过1600。
     * [语法]:
     *     [命令]: ^LRRCUECAPPARAMSET=<seq>,<EndFlag>,<CapParamNum>,<para1>[,<para2>[,<para3>]]
     *     [结果]: 执行正确时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^LRRCUECAPPARAMSET=?
     *     [结果]: <CR><LF>^LRRCUECAPPARAMSET:(list of supported <Seq>s),(list of supported <EndFlag>s),(list of supported <CapParamNum>s),<para>,<para>,<para><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Seq>: 整型，流水号，取值范围：[1,65535]
     *     <EndFlag>: 整型，参数设置完毕标识：
     *             0：未完毕
     *             1：已完毕
     *     <CapParamNum>: 整型，该条AT命令中UE能力数据参数组数，取值范围：[1,3]
     *             如果CapParamNum组数不等于能力参数（Para1，Para2和Para3）的总组数，直接返回失败。
     *     <Para1>: UE能力数据参数码流1，由接收组件解析。长度不超过560，且长度必须为偶数。
     *     <Para2>: UE能力数据参数码流2，由接收组件解析。长度不超过560，且长度必须为偶数。
     *     <Para3>: UE能力数据参数码流3，由接收组件解析。长度不超过560，且长度必须为偶数。
     * [示例]:
     *     · AP下发UE能力数据参数命令：
     *       AT^LRRCUECAPPARAMSET=1,0,3,"1111111111","22222222","3333333333"
     *       OK
     *       AT^LRRCUECAPPARAMSET=2,1,2,"4444444444","55555555"
     *       OK
     *     · 测试本命令
     *       AT^LRRCUECAPPARAMSET=?
     *       (1-65535),(0-1),(1-3),(str),(str),(str)
     *       OK
     */
    { AT_CMD_LRRC_UE_CAP_SET,
      AT_SetLrrcUeCapPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^LRRCUECAPPARAMSET", (TAF_UINT8 *)"(1-65535),(0-1),(1-3),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 过热缓解设置AT命令
     * [说明]: 本接口设置命令用于UE设置过热情况下诉求基站进行热缓解的措施，对不同工作制式设置命令携带不同的参数。其目的为终端提供热缓解方法。终端UE发送热缓解请求，基站执行。只对当前主模设置。
     * [语法]:
     *     [命令]: ^OVERHEATINGCFG=<type>,<overheatingFlag>,<setParamFlag>[,<para1>,<para2>,<para3>,<para4>[,<para5>,<para6>>,<para7>,<para8>,<para9>,<para10>]]
     *     [结果]: 执行正确时：
     *             <CR><LF>OK<CR><LF>
     *             有错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^OVERHEATINGCFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整型值
     *             当前接入技术指示
     *             0：LTE
     *             1:  NR
     *     <overheatingFlag>: 整型值
     *             0：检测到过热，进行参数设置
     *             1：退出过热状态
     *     <setParamFlag>: 整型值
     *             设置的参数标志位。
     *             当type=0时:
     *             bit0 : currentReportCcs 是否有效
     *             bit1：currentReportCat 是否有效
     *             bit2-bit30：保留位，当前版本设置为0
     *             bit31：是否立即生效，仅对NR生效
     *             当type=1时:
     *             bit0 : currentReportCcs 是否有效
     *             bit1：currentReportMimoFr1是否有效
     *             bit2：currentReportBwFr1是否有效
     *             bit3：currentReportMimoFr2是否有效
     *             bit4：currentReportBwFr2是否有效
     *             bit5-bit30：保留位，当前版本设置为0
     *             bit31：是否立即生效，仅对NR生效
     *     <Para1>: 整型值，十进制数字
     *             Para1为过热缓解上报的DL CC数
     *             当type=1时，协议取值范围为[0,31]，实际取值为[0,本产品支持的DL CC数 - 1]
     *     <Para2>: 整型值，十进制数字
     *             Para2为过热缓解上报的UL CC数
     *             当type=1时，协议取值范围为[0,31]，实际取值为[0,本产品支持的UL CC数 - 1]
     *     <Para3>: 整型值，十进制数字
     *             当type=0时，Para3为过热缓解上报的DL Category
     *             当type=1时，Para3为过热缓解上报的FR1 DL MIMO
     *             AP将MIMO枚举转换成UU具体值， MIMO枚举值映射关系如下：
     *             0：layers是2；
     *             1：layers是4；
     *             2：layers是8；
     *     <Para4>: 整型值，十进制数字
     *             当type=0时，Para4为过热缓解上报的UL Category
     *             当type=1时，Para4为过热缓解上报的FR1 UL MIMO
     *             AP将MIMO枚举转换成UU具体值， MIMO枚举值映射关系如下：
     *             0：layers是1；
     *             1：layers是2；
     *             2：layers是4；
     *     <Para5>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para5为过热缓解上报的FR1 DL目标带宽BandWidthDL枚举值
     *     <Para6>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para6为过热缓解上报的FR1 UL目标带宽BandWidthUL枚举值
     *     <Para7>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para7为过热缓解上报的FR2 DL MIMO
     *             AP将MIMO枚举转换成UU具体值， MIMO枚举值映射关系如下：
     *             0：layers是2；
     *             1：layers是4；
     *             2：layers是8；
     *     <Para8>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para8为过热缓解上报的FR2 UL MIMO
     *             AP将MIMO枚举转换成UU具体值， MIMO枚举值映射关系如下：
     *             0：layers是1；
     *             1：layers是2；
     *             2：layers是4；
     *     <Para9>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para9为过热缓解上报的FR2 DL目标带宽BandWidthDL枚举值
     *     <Para10>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para10为过热缓解上报的FR2 UL目标带宽BandWidthUL枚举值
     * [示例]:
     *     · 上报LTE OVERHEATING限制参数，上下行CC数限制为2，上行cat能力限制为8
     *       AT^OVERHEATINGCFG=0,0,3,2,2,8,8
     *       OK
     */
    { AT_CMD_OVER_HEATING_SET,
      AT_SetOverHeatingCfgPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^OVERHEATINGCFG", (TAF_UINT8 *)"(0-1),(0-1),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 过热缓解查询AT命令
     * [说明]: 本接口设置命令用于查询当前注册小区是否支持过热缓解特性以及当前接入技术（LTE或NR）下上一次设置的过热缓解设置参数。只对当前主模查询。
     * [语法]:
     *     [命令]: ^OVERHEATINGQRY=<type>
     *     [结果]: 执行正确时：
     *             ^OVERHEATINGQRY: <OverHeatingSupportFlag> ,<lastReportParamFlag>,<para1>,<para2>，<para3>,<para4>,<para5>,<para6>,<para7>,<para8>,<para9>,<para10>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^OVERHEATINGQRY=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整型值, 十进制数字
     *             当前接入技术指示
     *             0：LTE
     *             1:  NR
     *     <OverHeatingSupportFlag>: 整型值, 十进制数字
     *             指示当前小区是否支持OVERHEATING
     *             0：不支持
     *             1：支持
     *     <lastReportParamFlag>: 整型值, 十进制数字
     *             最后一次上报的参数是否有效标志位。0无效，1有效
     *             整型值
     *             设置的参数标志位。
     *             当type=0时:
     *             bit0 : currentReportCcs 是否有效
     *             bit1：currentReportCat 是否有效
     *             bit2-bit30：保留位，当前版本设置为0
     *             bit31：是否立即生效，仅对NR生效
     *             当type=1时:
     *             bit0 : currentReportCcs 是否有效
     *             bit1：currentReportMimoFr1是否有效
     *             bit2：currentReportBwFr1是否有效
     *             bit3：currentReportMimoFr2是否有效
     *             bit4：currentReportBwFr2是否有效
     *             bit5-bit30：保留位，当前版本设置为0
     *             bit31：是否立即生效，仅对NR生效
     *     <Para1>: 整型值，十进制数字
     *             Para1为最近一次过热缓解上报的DL CC数
     *             当type=1时，协议取值范围为[0,31]，实际取值为[0,本产品支持的DL CC数 - 1]
     *     <Para2>: 整型值，十进制数字
     *             Para2为最近一次过热缓解上报的UL CC数
     *             当type=1时，协议取值范围为[0,31]，实际取值为[0,本产品支持的UL CC数 - 1]
     *     <Para3>: 整型值，十进制数字
     *             当type=0时，Para3为最近一次过热缓解上报的DL Category
     *             当type=1时，Para3为最近一次过热缓解上报的DL MIMO
     *     <Para4>: 整型值，十进制数字
     *             当type=0时，Para4为最近一次过热缓解上报的UL Category
     *             当type=1时，Para4为最近一次过热缓解上报的UL MIMO
     *     <Para5>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para5为最近一次过热缓解上报的DL目标带宽BandWidthDL枚举值
     *     <Para6>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para6为最近一次过热缓解上报的UL目标带宽BandWidthUL枚举值
     *     <Para7>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para7为过热缓解上报的FR2 DL MIMO
     *     <Para8>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para8为过热缓解上报的FR2 UL MIMO
     *     <Para9>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para9为过热缓解上报的FR2 DL目标带宽BandWidthDL枚举值
     *     <Para10>: 整型值，十进制数字
     *             当type=0时，无此参数
     *             当type=1时，Para10为过热缓解上报的FR2 UL目标带宽BandWidthUL枚举值
     * [示例]:
     *       AT^OVERHEATINGQRY=1
     *       ^OVERHEATINGQRY: 1,1,1,1,1,1,4,4,1,1,4,4
     *       OK
     */
    { AT_CMD_OVER_HEATING_QRY,
      AT_SetOverHeatingQryParam, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^OVERHEATINGQRY", (TAF_UINT8 *)"(0-1)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NRRC能力限制配置命令
     * [说明]: 通过这个命令可以动态打开和关闭NRRC的某个类型的控制能力。
     * [语法]:
     *     [命令]: ^NRRCCAPCFG=<mode>,<para1>,<para2>,<para3>,<para4>,<para5>,<para6>,<para7>,<para8>[,<para9>,<para10>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NRRCCAPCFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值。
     *             0：NR下的高温温保配置参数；对应后面8个para参数，本操作下电后无效，配置后需要软关软开后起效，如果AT命令携带参数（除去mode）不是8个则返回error；
     *             1：EN-DC下的高温温保配置参数；对应后面的10个para参数；本操作下电后无效，配置后需要软关软开后起效，如果AT命令携带参数（除去mode）不是10个则返回error；
     *             2：VONR能力参数，对应后面的para1参数；下电后仍然有效，配置后需要软关软开后起效，如果AT命令携带参数（除去mode）不是1个则返回error；
     *             3：NR CA能力开关，对应后面的para1参数；下电后仍然有效，配置后需要软关软开后起效，如果AT命令携带参数（除去mode）不是1个则返回error；
     *             4：SA模拟不活动定时器参数，对应后面的7个参数。配置即生效，软开软关不清理，上下电恢复NV默认值，如果AT命令携带参数（除去mode）不是7个则返回error；
     *             5：随网推送DSS能力开关参数，对应后面的2个参数；下电后仍然有效，配置后需要软关软开后起效，如果AT命令携带参数（除去mode）不是2个则返回error；
     *             其他：无含义，返回error。
     *     <para1>: 整型值，设置对应类型NR控制能力的的第一项参数
     *             <mode> 为 0和1 下的参数含义：
     *             NR下行最大支持CA载波个数(maxNumberCCsDL)。
     *             取值范围
     *             1，芯片支持的NR下行最大CA载波个数；
     *             0：取消限制，恢复默认参数。
     *             <mode>为 2 ，VONR的参数含义：
     *             0：表示关闭VONR；
     *             1表示支持FR1-cd .VoNR;
     *             2表示支持FR2-VoNR;
     *             3表示支持FR1-VoNR和FR2-VoNR
     *             <mode>为 3 ，NR CA的参数含义：
     *             0 表示关闭NR CA；
     *             1 表示开启NR CA。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             para1为saInactivityEnable开关，1表示识别到长时间无数据回到idle态，0表示不回到idle态。
     *             <mode>为5，DSS特性rateMatchingLTE-CRS能力开关，0表示不改变产品定制值，1表示将每个Band的rateMatchingLTE-CRS都改为不支持。
     *     <para2>: 整数值，NR下行最大支持MIMO数(maxMIMOsDL)。
     *             取值2、4、8。
     *             0：取消限制，恢复默认参数。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             para2为defaultPeriod，连续defaultPeriod * M次收到L2的noData消息，会识别为长时间无数据（M动态的取multipleTimes数组元素）
     *             <mode>为5，DSS特性additionalDMRS-DL-Alt能力开关，
     *             0表示不改变产品定制值，1表示将每个组合（包括NRCA、ENDC、NR-DC等每种组合）的additionalDMRS-DL-Alt都改为不支持。
     *     <para3>: 整数值，NR下行FR1最大聚合带宽(maxFR1BwDL)。
     *             取值[5，芯片支持的下行FR1最大聚合带宽]，UE限制NR下行FR1最大聚合带宽小于等于该限制值。
     *             0：取消限制，恢复默认参数。
     *             说明：
     *             1）限制CA中FR1最大聚合带宽不超过该门限。计算CA中FR1多CC的最大带宽总和，若总和超过此门限，则这个CA不上报；如果不超过，则可上报。
     *             2）对于FR1单载波（non-CA），如果带宽限制小于Band可取的最小带宽值，那么两者取大，单CC带宽取此Band可取的最小带宽值。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             para3是protectTime，释放保护时间，单位为秒。
     *     <para4>: 整数值，NR下行FR2最大聚合带宽(maxFR2BwDL)。
     *             取值[50，芯片支持的下行FR2最大聚合带宽]，UE限制NR下行FR2最大聚合带宽小于等于该限制值。
     *             0：取消限制，恢复默认参数。
     *             说明：
     *             1）限制CA中FR2最大聚合带宽不超过该门限。计算CA中FR2多CC的最大带宽总和，若总和超过此门限，则这个CA不上报；如果不超过，则可上报。
     *             2）对于FR2单载波（non-CA），如果带宽限制小于Band可取的最小带宽值，那么两者取大，单CC带宽取此Band可取的最小带宽值。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             para4是rauAfterRel，1表示回到idle后立马发起注册，0表示不发起注册停留在idle态。
     *     <para5>: 整型值，NR上行最大支持CA载波个数(maxNumberCCsUL)。
     *             取值范围[1，芯片支持的NR上行最大CA载波个数]。
     *             0：取消限制，恢复默认参数。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             Para5是MultipleTimesNum，表示multipleTimes数组的大小。
     *     <para6>: 整数值，NR上行最大支持MIMO数(maxMIMOsUL)。
     *             取值1、2、4。
     *             0：取消限制，恢复默认参数。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             Para6可表示为0xAABBCCDD，DD表示multipleTimes[0]，CC表示 multipleTimes[1]，BB表示 multipleTimes[2]，AA表示 multipleTimes[3]。
     *     <para7>: 整数值，NR上行FR1最大聚合带宽(maxFR1BwUL)。
     *             取值[5，芯片支持的上行FR1最大聚合带宽]，UE限制NR上行FR1最大聚合带宽小于等于该限制值。
     *             0：取消限制，恢复默认参数。
     *             说明：
     *             1）限制CA中FR1最大聚合带宽不超过该门限。计算CA中FR1多CC的最大带宽总和，若总和超过此门限，则这个CA不上报；如果不超过，则可上报。
     *             2）对于FR1单载波（non-CA），如果带宽限制小于Band可取的最小带宽值，那么两者取大，单CC带宽取此Band可取的最小带宽值。
     *             <mode>为4，SA模拟不活动定时器参数含义：
     *             Para7可表示为0xAABBCCDD，DD表示multipleTimes[4]，CC表示 multipleTimes[5]，BB表示 multipleTimes[6]，AA表示 multipleTimes[7]。
     *             注意，即使multipleTimes数组长度小于4，第7个参数不使用，也要在AT命令带上，不然会配置失败。
     *     <para8>: 整数值，NR上行FR2最大聚合带宽(maxFR2BwUL)。
     *             取值[50，芯片支持的上行FR2最大聚合带宽]，UE限制NR上行FR2最大聚合带宽小于等于该限制值。
     *             0：取消限制，恢复默认参数。
     *             说明：
     *             1）限制CA中FR2最大聚合带宽不超过该门限。计算CA中FR2多CC的最大带宽总和，若总和超过此门限，则这个CA不上报；如果不超过，则可上报。
     *             2）对于FR2单载波（non-CA），如果带宽限制小于Band可取的最小带宽值，那么两者取大，单CC带宽取此Band可取的最小带宽值。
     *     <para9>: 整数值，EN-DC场景下LTE最大支持CA载波个数(maxNumberCCsLTE)。
     *             取值范围[1，芯片支持的LTE最大CA载波个数]。
     *             0：取消限制，恢复默认参数。
     *             说明：该参数是可选项，如果mode=0不需要配置该参数，如果mode=1需要配置该参数。
     *     <para10>: 整数值，EN-DC场景下LTE最大支持MIMO数(maxMIMOsLTE)。
     *             取值2、4、8。
     *             0：取消限制，恢复默认参数。
     *             说明：该参数是可选项，如果mode=0不需要配置该参数，如果mode=1需要配置该参数。
     * [示例]:
     *     · 配置SA场景NR能力限制，限制参数值由用户指定
     *       AT^NRRCCAPCFG=0,1,2,20,50,1,1,20,50
     *       OK
     *     · 取消SA场景NR能力限制
     *       AT^NRRCCAPCFG=0,0,0,0,0,0,0,0,0
     *       OK
     *     · 配置EN-DC场景NR能力限制，限制参数值由用户指定(说明：需要同时下发EN-DC场景配置LTE能力限制AT命令)
     *       AT^NRRCCAPCFG=1,1,2,20,50,1,1,20,50,1,2
     *       OK
     *       AT^ERRCCAPCFG=4,2,1
     *       OK
     *     · 取消EN-DC场景NR能力限制 (说明：需要同时下发EN-DC场景取消LTE能力限制AT命令)
     *       AT^NRRCCAPCFG=1,0,0,0,0,0,0,0,0,0,0
     *       OK
     *       AT^ERRCCAPCFG=4,0,0
     *       OK
     *     · 配置开启NR CA
     *       AT^NRRCCAPCFG=3,1
     *       OK
     *       配置SA不活动定时器参数
     *          AT^NRRCCAPCFG = 4,1,3,180,1,3,0x00040201,0
     *          OK
     *       说明：
     *       第1个参数表示saInactivity是1
     *       第2个参数表示defaultPeriod是3
     *       第3个参数表示protectTime是180秒
     *       第4个参数表示表示rauAfterRel是1
     *       第5各参数表示multipleTimesNum是3，后面的multipleTimes数组长度为3
     *       第6个参数0x00040201：01是multipleTimes[0]；02是multipleTimes [1]；04是multipleTimes[2]；00multipleTimes[3]；
     *       第7个参数0x04030201：01是multipleTimes[4]；02是multipleTimes [5]；03是multipleTimes[6]；04是multipleTimes[7]。
     *       注意，即使multipleTimes数组长度为3，第7个参数不使用，也要在AT命令带上，不然会配置失败。
     */
    { AT_CMD_NRRCCAPCFG,
      AT_SetNrrcCapCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRRCCAPCFG", (VOS_UINT8 *)"(0-255),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NRRC能力限制查询命令
     * [说明]: 通过该AT命令查询NRRC某种模式下的能力配置情况。
     * [语法]:
     *     [命令]: ^NRRCCAPQRY=<mode>
     *     [结果]: 执行成功时：
     *             ^NRRCCAPQRY: < mode >[<para1>,<para2>，<para3>,<para4>,<para5>,<para6>,<para7>,<para8>,<para9>,<para10>]
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRRCCAPQRY=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值。
     *             0：NR
     *             1：EN-DC
     *             2：VONR能力
     *             3：NR CA能力
     *             5：DSS能力
     *             其他：无含义，返回error。
     *     <para1>: 整型值，设置对应类型为NR能力限制配置的第一项参数
     *     <para2>: 整型值，设置对应类型为NR能力限制配置的第二项参数
     *     <para3>: 整型值，设置对应类型为NR能力限制配置的第三项参数
     *     <para4>: 整型值，设置对应类型为NR能力限制配置的第四项参数
     *     <para5>: 整型值，设置对应类型为NR能力限制配置的第五项参数
     *     <para6>: 整型值，设置对应类型为NR能力限制配置的第六项参数
     *     <para7>: 整型值，设置对应类型为NR能力限制配置的第七项参数
     *     <para8>: 整型值，设置对应类型为NR能力限制配置的第八项参数
     *     <para9>: 整型值，设置对应类型为NR能力限制配置的第九项参数
     *     <para10>: 整型值，设置对应类型为NR能力限制配置的第十项参数
     * [示例]:
     *     · 查询NRRC EN-DC模式下的能力配置
     *       AT^NRRCCAPQRY=1
     *       ^NRRCCAPQRY: 1,1,2,20,50,1,1,20,50,1,2
     *       OK
     *     · 查询NRRC CA能力配置
     *       AT^NRRCCAPQRY=3
     *       ^NRRCCAPQRY: 3,0,0,0,0,0,0,0,0,0,0
     *       OK
     */
    { AT_CMD_NRRCCAPQRY,
      AT_SetNrrcCapQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRRCCAPQRY", (VOS_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 定制自主N2L门限优化配置命令
     * [说明]: 该命令用于定制设置自主N2L门限优化配置。参数取值请参考建议取值范围配置，接入层可能对不合理参数组合进行纠错处理。
     * [语法]:
     *     [命令]: ^AUTON2LOPTCFG=<SwitchEnable>,<ParaEnable>,<AutoOptIdleSwitch>,<AutoOptConnSwitch>,<RsrpNrLowThreshold>,<SinrNrLowThreshold>,<RsrpLteHighThreshold>,<SinrLteHighThreshold>,<RsrpL2NJdgDelta>,<SinrL2nJdgDelta>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <SwitchEnable>: 用于指示开关配置（第3项和第4项）是否生效
     *             0：不生效
     *             1：生效
     *     <ParaEnable>: 用于指示参数配置（第5-10项）是否生效
     *             0：不生效
     *             1：生效
     *     <AutoOptIdleSwitch>: Idle态N2L优化总开关
     *             0：关闭
     *             1：打开
     *     <AutoOptConnSwitch>: 连接态N2L优化总开关
     *             0：关闭
     *             1：打开
     *     <RsrpNrLowThreshold>: NR小区信号RSRP低于此门限则启动优化,单位dbm, 建议取值范围[-156,-31]
     *     <SinrNrLowThreshold>: NR小区信号SINR低于此门限则启动优化,单位dbm, 建议取值范围[-23,40]
     *     <RsrpLteHighThreshold>: LTE小区信号RSRP高于此门限则启动优化,单位dbm, 建议取值范围[-141,-44]
     *     <SinrLteHighThreshold>: LTE小区信号SINR高于此门限则启动优化,单位dbm, 建议取值范围[-23,40]
     *     <RsrpL2NJdgDelta>: L2N重选或者测报判断时, NR小区的RSRP需要额外高出此Delta值才会解除抑制, 单位dbm, 建议取值范围[0,10]
     *     <SinrL2nJdgDelta>: L2N重选或者测报判断时, NR小区的SINR需要额外高出此Delta值才会解除抑制, 单位dbm, 建议取值范围[0,10]
     * [示例]:
     *     · 设置开关配置生效
     *       AT^AUTON2LOPTCFG=1,0,0,1,-100,30,-60,20,5,8
     *       OK
     *     · 设置参数配置生效
     *       AT^AUTON2LOPTCFG=0,1,0,1,-70,10,-55,-3,2,4
     *       OK
     */
    { AT_CMD_AUTON2LOPTCFG,
      AT_SetAutoN2LOptCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^AUTON2LOPTCFG", (VOS_UINT8 *)"(0-1),(0-1),(0-1),(0-1),(@rsrpNrLowThresh),(@sinrNrLowThresh),(@rsrpLteHighThresh),(@sinrLteHighThresh),(@rsrpLnJdgDelta),(@sinrLnJdgDelta)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置NR的UE能力数据参数
     * [说明]: 本命令用于AP侧将云端或者版本预置的随PLMN定制NR的UE能力数据参数下发给MODEM，和^NRRCUECAPINFONTF命令配合使用。
     *         注：AT命令总长度不超过1600。
     * [语法]:
     *     [命令]: ^NRRCUECAPPARAMSET=<seq>,<EndFlag>,<CapParamNum>,<para1>[,<para2>[,<para3>]]
     *     [结果]: 执行正确时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NRRCUECAPPARAMSET=?
     *     [结果]: <CR><LF>^NRRCUECAPPARAMSET: (list of supported <Seq>s),(list of supported <EndFlag>s),(list of supported <CapParamNum>s),<para>,<para>,<para><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Seq>: 整型，流水号，取值范围：[1,65535]
     *     <EndFlag>: 整型，参数设置完毕标识：
     *             0：未完毕
     *             1：已完毕
     *     <CapParamNum>: 整型，该条AT命令中UE能力数据参数组数，取值范围：[1,3]
     *             如果CapParamNum组数不等于能力参数（Para1，Para2和Para3）的总组数，直接返回失败。
     *     <Para1>: UE能力数据参数码流1，由接收组件解析。长度不超过560，且长度必须为偶数。
     *     <Para2>: UE能力数据参数码流2，由接收组件解析。长度不超过560，且长度必须为偶数。
     *     <Para3>: UE能力数据参数码流3，由接收组件解析。长度不超过560，且长度必须为偶数。
     * [示例]:
     *     · AP下发UE能力数据参数命令：
     *       AT^NRRCUECAPPARAMSET=1,0,3,"1111111111","22222222","3333333333"
     *       OK
     *       AT^NRRCUECAPPARAMSET=2,1,2,"4444444444","55555555"
     *       OK
     *     · 测试本命令
     *       AT^NRRCUECAPPARAMSET=?
     *       (1-65535),(0-1),(1-3),(str),(str),(str)
     *       OK
     */
    { AT_CMD_NRRC_UE_CAP_SET,
      AT_SetNrrcUeCapPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^NRRCUECAPPARAMSET", (TAF_UINT8 *)"(1-65535),(0-1),(1-3),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询NR小区CA状态信息
     * [说明]: 用于查询NR小区上行、下行CA配置状态，最多能查到8个CA小区信息。
     *         此命令仅在主模为NR或LTE，且驻留网络时查询有效，SA或NSA网络下可查询到有效的NR CA信息。
     * [语法]:
     *     [命令]: ^NRCACELL?
     *     [结果]: 执行成功时：
     *             <CR><LF>^NRCACELL: <total_num>[,<cell_index>,<dl_cfg>,<nul_cfg>,<sul_cfg>[,<cell_index>,<dl_cfg>,<nul_cfg>,<sul_cfg> <[…]]]<CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRCACELL=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <total_num>: 整型值，本次上报的NR小区个数
     *     <cell_index>: 整型值，NR小区索引，0表示PCell，其余为PSCell或Scell。
     *     <dl_cfg>: 整型值，该小区下行是否被配置：
     *             0：未配置；
     *             1：已配置。
     *     <nul_cfg>: 整型值，该小区上行Nul是否被配置：
     *             0：未配置；
     *             1：已配置。
     *     <sul_flg>: 整型值，该小区上行Sul是否被配置：
     *             0：未配置；
     *             1：已配置。
     * [示例]:
     *     · UE驻留在NR网络时查询NR CA状态信息,CA小区个数不为0
     *       AT^NRCACELL?
     *       ^NRCACELL: 2,0,1,1,1,1,1,1,1
     *       OK
     *     · UE驻留在NR网络时查询NR CA状态信息,CA小区个数为0
     *       AT^NRCACELL?
     *       ^NRCACELL: 0
     *       OK
     *     · 执行测试命令
     *       AT^NRCACELL=?
     *       OK
     */
    { AT_CMD_NRCACELL,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryNrCaCellPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRCACELL", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 控制NR小区CA信息主动上报
     * [说明]: 该命令用于控制UE在驻留NR或LTE NSA网络时，CA信息变化后是否主动上报^NRCACELLRPT命令，主动通知AP当前模式下小区NR CA状态信息变化。设置成功立即生效，进退飞行模式仍然有效，下电后失效，重新上电默认关闭主动上报。
     * [语法]:
     *     [命令]: ^NRCACELLRPTCFG=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRCACELLRPTCFG?
     *     [结果]: 执行成功时：
     *             <CR><LF>^NRCACELLRPTCFG: <enable><CR><LF> <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRCACELLRPTCFG=?
     *     [结果]: <CR><LF>^NRCACELLRPTCFG:(0,1)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，主动上报^NRCACELLRPT命令的开关：
     *             0：关闭^NRCACELLRPT主动上报；
     *             1：打开^NRCACELLRPT主动上报，CA信息有变化就上报。CA信息变化包括包括从无到有，从有到无和从有到有。从有到有是否变化根据cellIdx判断。
     * [示例]:
     *     · 打开^NRCACELLRPT主动上报
     *       AT^NRCACELLRPTCFG=1
     *       OK
     *     · 查询^NRCACELLRPT主动上报开关状态
     *       AT^NRCACELLRPTCFG?
     *       ^NRCACELLRPTCFG: 1
     *       OK
     *     · 执行测试命令
     *       AT^NRCACELLRPTCFG=?
     *       ^NRCACELLRPTCFG: (0,1)
     *       OK
     */
    { AT_CMD_NRCACELLRPTCFG,
      AT_SetNrCaCellRptCfgPara, AT_SET_PARA_TIME, AT_QryNrCaCellRptCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRCACELLRPTCFG", (TAF_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR网络能力主动上报开关
     * [说明]: 通过该AT命令开关NR的某种网络能力变化的主动上报。
     * [语法]:
     *     [命令]: ^NRNWCAPRPTCFG=<type>,<switch>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRNWCAPRPTCFG=?
     *     [结果]: <CR><LF>^NRCACELLRPTCFG:(0
     *             -255),(0,1)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整型值，取值0～255。
     *             1：网络的SA能力主动上报控制
     *             其他：暂无特别含义，预留使用
     *     <switch>: 0：关闭主动上报；
     *             1：打开主动上报。
     * [示例]:
     *     · 关闭主动上报当前网络SA能力变化
     *       AT^NRNWCAPRPTCFG=1,0
     *       OK
     */
    { AT_CMD_NRNWCAPRPTCFG,
      AT_SetNrNwCapRptCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRNWCAPRPTCFG", (TAF_UINT8 *)"(0-255),(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR网络能力主动上报开关查询
     * [说明]: 通过该AT命令查询NR的某种网络能力主动上报情况。
     * [语法]:
     *     [命令]: ^NRNWCAPRPTQRY=<type>
     *     [结果]: 执行成功时：
     *             <CR><LF>^NRNWCAPRPTQRY: < type>,<switch><CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRNWCAPRPTQRY=?
     *     [结果]: <CR><LF>^NRNWCAPRPTQRY:(0-255)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整型值，取值0～255。
     *             1：网络SA能力变化主动上报开关
     *             其他：暂无特别含义，预留使用
     *     <switch>: 整型值，取值范围:
     *             0：主动上报为关闭状态;
     *             1：主动上报为开启状态。
     * [示例]:
     *     · 查询网络SA能力变化主动上报开关， 当前上报开关开启
     *       AT^NRNWCAPRPTQRY=1
     *       ^NRNWCAPRPTQRY: 1,1
     *       OK
     */
    { AT_CMD_NRNWCAPRPTQRY,
      AT_SetNrNwCapRptQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRNWCAPRPTQRY", (TAF_UINT8 *)"(0-255)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR网络能力查询
     * [说明]: 通过该AT命令查询NR的某种网络能力情况。
     * [语法]:
     *     [命令]: ^NRNWCAPQRY=<type>
     *     [结果]: 执行成功时：
     *             <CR><LF>^NRNWCAPQRY: <type>,<para1>,<para2>，<para3>,<para4>,<para5>,<para6>,<para7>,<para8>,<para9>,<para10><CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRNWCAPQRY=?
     *     [结果]: <CR><LF>^NRNWCAPQRY:(0-255)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <type>: 整型值，取值0～255。
     *             1：当前网络SA能力，对应查询结果<para1>有效。当前不支持。
     *             其他：暂无特别含义，预留使用
     *     <para1>: 整型值，对应NR网络能力的第一项参数
     *             <type>为1时，取值范围:
     *             0：网络不支持SA;
     *             1：网络支持SA。
     *     <para2>: 整型值，对应NR网络能力的第二项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para3>: 整型值，对应NR网络能力的第三项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para4>: 整型值，对应NR网络能力的第四项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para5>: 整型值，对应NR网络能力的第五项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para6>: 整型值，对应NR网络能力的第六项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para7>: 整型值，对应NR网络能力的第七项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para8>: 整型值，对应NR网络能力的第八项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para9>: 整型值，对应NR网络能力的的第九项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     *     <para10>: 整型值，对应NR网络能力的第十项参数
     *             默认为0，暂无特别含义，待后续支持更多NR网络能力时使用
     * [示例]:
     *     · 查询网络SA的能力，当前网络支持SA(仅作为参数示例，不代表真实支持情况)
     *       AT^NRNWCAPQRY=1
     *       ^NRNWCAPQRY: 1,1,0,0,0,0,0,0,0,0,0
     *       OK
     */
    { AT_CMD_NRNWCAPQRY,
      AT_SetNrNwCapQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR,
      AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRNWCAPQRY", (TAF_UINT8 *)"(0-255)" },
#endif
#endif

    { AT_CMD_RSRP,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryRsrp, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+RSRP", VOS_NULL_PTR },

    { AT_CMD_RSRQ,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryRsrq, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+RSRQ", VOS_NULL_PTR },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR低功耗设置命令
     * [说明]: 本命令用于设置NR模式下的接入层低功耗配置参数
     * [语法]:
     *     [命令]: ^NRPOWERSAVINGCFG=<cmd_type>[,<para1>,<para2>,<para3>,…………,<para23>]
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRPOWERSAVINGCFG=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cmd_type>: 整型值，取值0～1
     *             0：暂时预留，功能不生效。
     *             1：低功耗配置类型：RRC快速释放；仅在MODEM开机下配置有效，软开关和上下电配置会被清除，后面目前携带4个para参数；
     *             其他：无含义，返回error。
     *     <para1>: 整型值，设置对应低功耗配置的第一项参数
     *             <cmd_type> 为1 下的参数含义：
     *             快速释放特性是否生效的标志
     *             取值范围0~1
     *             0：快速释放特性关闭
     *             1：快速释放特性打开
     *     <para2>: 整型值，设置对应低功耗配置的第二项参数
     *             <cmd_type> 为1 下的参数含义：
     *             快速释放特性立即生效的标志
     *             取值范围0~1
     *             0：快速释放立即生效未使能
     *             1：快速释放立即生效使能，UE立即执行快速释放一次
     *     <para3>: 整型值，设置对应低功耗配置的第三项参数
     *             <cmd_type> 为1 下的参数含义：
     *             快速释放特性期望的UE状态配置
     *             取值范围0~2
     *             0：IDLE态
     *             1：INACTIVE态
     *             2：CONN态
     *             其他：无含义，返回error。
     *     <para4>: 整型值，设置对应低功耗配置的第四项参数
     *             <cmd_type> 为1 下的参数含义：
     *             快速释放特性检测无数据周期时长，单位秒
     *             取值范围0~4,294,967,295
     *             0：按照快速释放特性关闭处理，不再检测无数据
     *             其他：按照配置时长为周期检测无数据
     *     <para5>: 整型值十进制，暂无含义，预留
     *     <para6>: 整型值十进制，暂无含义，预留
     *     <para7>: 整型值十进制，暂无含义，预留
     *     <para8>: 整型值十进制，暂无含义，预留
     *     <para9>: 整型值十进制，暂无含义，预留
     *     <para10>: 整型值十进制，暂无含义，预留
     *     <para11>: 整型值十进制，暂无含义，预留
     *     <para12>: 整型值十进制，暂无含义，预留
     *     <para13>: 整型值十进制，暂无含义，预留
     *     <para14>: 整型值十进制，暂无含义，预留
     *     <para15>: 整型值十进制，暂无含义，预留
     *     <para16>: 整型值十进制，暂无含义，预留
     *     <para17>: 整型值十进制，暂无含义，预留
     *     <para18>: 整型值十进制，暂无含义，预留
     *     <para19>: 整型值十进制，暂无含义，预留
     *     <para20>: 整型值十进制，暂无含义，预留
     *     <para21>: 整型值十进制，暂无含义，预留
     *     <para22>: 整型值十进制，暂无含义，预留
     *     <para23>: 整型值十进制，暂无含义，预留
     * [示例]:
     *     · 配置低功耗特性快速释放类型
     *       AT^NRPOWERSAVINGCFG=1,1,0,0,2
     *       OK
     *       第1个参数1表示本次配置类型是快速释放特性
     *       第2个参数1表示快速释放特性使能
     *       第3个函数0表示不立即生效快速释放
     *       第4个参数0表示快速释放的期望状态是IDLE
     *       第5个参数2表示检查无数据的周期为2秒
     */
    { AT_CMD_NRPOWERSAVINGCFG,
      AT_SetNrPowerSavingCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRPOWERSAVINGCFG", (VOS_UINT8 *)"(0-65535),(0-4294967295),(0-4294967295),(0-4294967295),\
(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),\
(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),\
(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR低功耗查询命令
     * [说明]: 本命令用于查询NR模式下的接入层低功耗配置参数
     * [语法]:
     *     [命令]: ^NRPOWERSAVINGQRY=<cme_type>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>^NRPOWERSAVINGQRY：<cmd_type>,<para1>,<para2>,……<para23>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^NRPOWERSAVINGQRY=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <cme_type>: 取值范围0~65535
     *             0：暂时预留，不生效
     *             1：查询RRC快速释放参数
     *             ……
     *     <para1>: cmd_type对应查询的结果，如果该类型无此参数，默认为0
     *     <para2>: cmd_type对应查询的结果，如果该类型无此参数，默认为0
     *     <……>: ……
     *     <para23>: cmd_type对应查询的结果，如果该类型无此参数，默认为0
     * [示例]:
     *       AT^NRPOWERSAVINGQRY=0
     *       ^NRPOWERSAVINGQRY: 1,1,2,20,50,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
     *       OK
     */
    { AT_CMD_NRPOWERSAVINGQRY,
      AT_SetNrPowerSavingQryPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRPOWERSAVINGQRY", (VOS_UINT8 *)"(0-65535)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询小区SSB信息
     * [说明]: 该命令用来提供NR小区对应的SSB信息。SSB信息包括服务小区和相邻小区的SSB信息。上层将使用这些信息来支持位置的标识，仅在支持NR能力的模式下生效。
     *         本命令仅在支持NR能力的产品上可用。
     *         SA场景下，本命令仅在主模链接态，且网侧配置测量时生效。
     *         ENDC场景下，本命令仅在ENDC建立成功，且网侧配置测量时生效。
     * [语法]:
     *     [命令]: ^NRSSBID?
     *     [结果]: 执行成功时：
     *             <CR><LF>^NRSSBID: <ARFCN-NR>,<CID>,<PCI>,<RSRP>,
     *             <SINR>,<TA>,<Scell_description>,<N_NB_CELL>[,<Ncell_description>]
     *             <CR><LF><CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err code><CR><LF>
     * [参数]:
     *     <Scell_description>: 服务小区SSB信息（Beam测量结果），需要网侧配置且上报测量结果。
     *             <S_SSBID0>,<S_ID0_RSRP>,<S_SSBID1>,<S_ID1_RSRP>, <S_SSBID2>,<S_ID2_RSRP>,<S_SSBID3>,<S_ID3_RSRP>, <S_SSBID4>,<S_ID4_RSRP>,<S_SSBID5>,<S_ID5_RSRP>, <S_SSBID6>,<S_ID6_RSRP>,<S_SSBID7>,<S_ID7_RSRP>
     *     <Ncell_description>: 邻小区SSB信息（Beam测量结果），需要网络侧配置邻区且上报测量结果，上报时不关注网侧重配置中reportCfg中的受限或者可信名单小区。  [,<NB_PCI>,<NB_ARFCN>,<NB_RSRP>,<NB_SINR>, <NB_SSBID0>,<NB_ID0_RSRP>,<NB_SSBID1>, <NB_ID1_RSRP>,<NB_SSBID2>,<NB_ID2_RSRP>, <NB_SSBID3>,<NB_ID3_RSRP>]*N_NB_CELL
     *     <ARFCN-NR>: 无符号32位整型值，NR服务小区SSB频点，无效值：0xFFFFFFFF
     *     <CID>: 8个字节，16个字符，16进制，表示当前服务小区ID。使用Half-Byte码（半字节）编码，例如000000010000001A，表示高4字节是0x00000001，低4字节是0x0000001A。注：NSA下上报的是LTE小区的CID，SA下也不一定能查到，与查询时序相关。无效值：0xFFFFFFFFFFFFFFFF
     *     <PCI>: 无符号16位整型值，当前服务物理小区ID。有效值[0,1007]，无效值:0xFFFF
     *     <RSRP>: 有符号16位整型值，当前服务小区参考信号接收功率。
     *             无效值：0x7FFF
     *     <SINR>: 有符号16位整型值，当前服务小区信号干扰噪声比。
     *             无效值：0x7FFF
     *     <TA>: 有符号32位整型值，服务小区当前时间提前量，无效值：-1。
     *     <S_SSBIDx>: 无符号8位整型值，服务区SSB ID（ID从0到7共8个）。无效值填;0xFF
     *     <S_IDx_RSRP>: 有符号16位整型值，服务区SSBID对应的参考信号接收功率（ID从0到7共8个）。无效值：0x7FFF
     *     <N_NB_CELL>: 无符号8位整型值，连接态NR邻区个数（最多4个，按Beam能量排序的前4个小区）。如果没有填0,上层识别该参数为0时，不再读取后续的值。
     *     <NB_PCI>: 无符号16位整型值，邻区物理小区ID。无效值：0xFFFF
     *     <NB_ARFCN>: 无符号32位整型值，邻区小区SSB频点，无效值：0xFFFFFFFF
     *     <NB_RSRP>: 有符号16位整型值，邻区参考信号接收功率。无效值：0x7FFF
     *     <NB_SINR>: 有符号16位整型值，邻区参考信号接收功率。无效值：0x7FFF
     *     <NB_SSBIDx>: 无符号8位整型值，邻区SSB ID（ID值范围0到7，上报的SSBID为其中RSRP测量结果最大的前4个）。无效值：0xFF
     *     <NB_IDx_RSRP>: 有符号16位整型值，邻区SSBID（ID从0到7，数量共4个）对应的参考信号接收功率。无效值：0x7FFF
     * [示例]:
     *     · 在SA上查询NRSSBID:
     *       AT^NRSSBID?
     *       ^NRSSBID=430010,00000001000000d5,110,-83,-10,-1,1,-50,2,-60,255,32767,255,32767,255,32767,255,32767,255,32767,255,32767,2,142,430040,-50,-15,3,-43,2,-55,4,-56,255,32767,143,430036,-51,-16,4,-43,2,-55,1,-56,255,32767
     *       OK
     *     · 在NSA上查询NRSSBID:
     *       AT^NRSSBID?
     *       ^NRSSBID=430010,0000000000000087,110,-83,-10,-1,1,-50,2,-60,255,32767,255,32767,255,32767,255,32767,255,32767,255,32767,1,142,430040,-50,-15,3,-43,2,-55,4,-56,255,32767
     *       OK
     */
    { AT_CMD_NRSSBID,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryNrSsbIdPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRSSBID", VOS_NULL_PTR },
#endif

    { AT_CMD_NCELLMONITOR,
      AT_SetNCellMonitorPara, AT_SET_PARA_TIME, AT_QryNCellMonitorPara, AT_QRY_PARA_TIME, AT_TestNCellMonitorPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NCELLMONITOR", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_ANQUERY,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryAnQuery, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^ANQUERY", VOS_NULL_PTR },

#if (FEATURE_LTE == FEATURE_ON)
    { AT_CMD_LFROMCONNTOIDLE,
      At_SetLFromConnToIdlePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^LFROMCONNTOIDLE", VOS_NULL_PTR },
#endif

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: LTE CA辅小区信息查询命令
     * [说明]: 该命令用于查询LTE CA状态下SCELL信息，当前最大支持4个SCELL信息查询。
     * [语法]:
     *     [命令]: ^CASCELLINFO?
     *     [结果]: ^CASCELLINFO: <index>,<pci>,<rssi>,<rsrp>,<rsrq>,<band_class>,<ulfreqpoint>,<dlfreqpoint>,<ulfreq>,<dlfreq>,<ulbw>,<dlbw><CR><LF>[[^CASCELLINFO: <index>,<pci>,<rssi>,<rsrp>,<rsrq>,<band_class>,<ulfreqpoint>,<dlfreqpoint>,<ulfreq>,<dlfreq>,<ulbw>,<dlbw><CR><LF>][…]]<CR><LF>OK<CR><LF>
     *             有其他错误
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <index>: 整型值，指示当前SCELL小区索引值，取值范围1~4。
     *     <pci>: 整型值，指示当前SCELL小区PCI。
     *     <rssi>: 整型值，指示当前SCELL小区rssi值，
     *             取值如下：
     *             -120 ~ -25, 单位dBm。
     *     <rsrp>: 整型值，指示当前SCELL小区rsrp值，
     *             取值如下：
     *             -140 ~ -44, 单位dBm。
     *     <rsrq>: 整型值，指示当前SCELL小区rsrq值，
     *             取值如下：
     *             -19~ -3, 单位dB。
     *     <band_class>: 整型值，表示当前终端所处小区对应的频段。
     *     <ulfreqpoint>: 整型值，指示当前SCELL小区上行频点。
     *     <dlfreqpoint>: 整型值，指示当前SCELL小区下行频点。
     *     <ulfreq>: 整型值，指示当前SCELL小区上行频率，单位为(100kHz)。
     *     <dlfreq>: 整型值，指示当前SCELL小区下行频率，单位为(100kHz)。
     *     <ulbw>: 整型值，指示当前SCELL小区上行带宽，
     *             取值如下：
     *             0：表示1.4M带宽
     *             1：表示3M带宽
     *             2：表示5M带宽
     *             3：表示10M带宽
     *             4：表示15M带宽
     *             5：表示20M带宽
     *     <dlbw>: 整型值，指示当前SCELL小区下行带宽，取值如下：
     *             0：表示1.4M带宽
     *             1：表示3M带宽
     *             2：表示5M带宽
     *             3：表示10M带宽
     *             4：表示15M带宽
     *             5：表示20M带宽
     * [示例]:
     *     · 测试命令:
     *       AT^CASCELLINFO=?
     *       OK
     *     · CA配置时查询命令（2CC CA）
     *       AT^CASCELLINFO?
     *       ^CASCELLINFO: 1,417,-60,-80,-5,18,23925,5925,8225,8675,5,5
     *       OK
     *     · CA未配置时查询命令:
     *       AT^CASCELLINFO?
     *       ERROR
     *     · CA配置时查询命令（3CC CA）:
     *       AT^CASCELLINFO?
     *       ^CASCELLINFO: 1,417,-60,-80,-5,18,23925,5925,8225,8675,3,3
     *       ^CASCELLINFO: 2,418,-61,-81,-7,18,23925,5925,8225,8675,3,3
     *        OK
     */
    { AT_CMD_CASCELLINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCaScellInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CASCELLINFO", VOS_NULL_PTR },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /* 协议升级,将HSPA的设置范围由(0-3),更改为(0-7),其中4对应R8,5对应R9，其它数字用于以后的扩展 */
    { AT_CMD_HSPA,
      At_SetRRCVersion, AT_SET_PARA_TIME, At_QryRRCVersion, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HSPA", (VOS_UINT8 *)"(0-7)" },
#endif
};

/* 注册AS定制AT命令表 */
VOS_UINT32 AT_RegisterCustomAsCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomAsCmdTbl, sizeof(g_atCustomAsCmdTbl) / sizeof(g_atCustomAsCmdTbl[0]));
}

