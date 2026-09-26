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
#include "at_custom_mm_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_mm_set_cmd_proc.h"
#include "at_custom_mm_qry_cmd_proc.h"



#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_MM_CMD_TBL_C

static const AT_ParCmdElement g_atCustomMmCmdTbl[] = {
#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: RCS动态开关命令
     * [说明]: 该命令用于设置和查询RCS的动态开关。
     * [语法]:
     *     [命令]: ^RCSSWITCH=<status>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^RCSSWITCH?
     *     [结果]: <CR><LF>^RCSSWITCH: <status>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <status>: 整型值，RCS开关状态：
     *             0： off；
     *             1： on。
     * [示例]:
     *     · 打开RCS动态开关成功：
     *       AT^RCSSWITCH=1
     *       OK
     *     · 查询RCS动态开关状态成功：
     *       AT^RCSSWITCH?
     *       ^RCSSWITCH: 1
     *       OK
     */
    { AT_CMD_RCSSWITCH,
      AT_SetRcsSwitch, AT_SET_PARA_TIME, AT_QryRcsSwitch, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^RCSSWITCH", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询当前IMSI号和HPLMN
     * [说明]: 查询当前IMSI号和HPLMN。
     * [语法]:
     *     [命令]: ^HPLMN?
     *     [结果]: <CR><LF> ^HPLMN: <IMSI>,<MNC Length>,<EHPLMN NUM>,<EHPLMN><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况返回值：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <IMSI>: IMSI号
     *     <MNC Length>: MNC长度，从卡文件中获取
     *     <EHPLMN NUM>: EHPLMN个数
     *     <EHPLMN>: EHPLMN列表
     * [示例]:
     *     · 查询命令
     *       AT^HPLMN?
     *       ^HPLMN: 46002123456789,2,2,46000,46007
     *       OK
     */
    { AT_CMD_HPLMN,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryHplmnPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^HPLMN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置和查询DPLMN列表
     * [说明]: 配置卡对应的全球预置DPLMN列表。
     *         查询之前需要先设置云通讯DPLMN版本号NV(2360)为有效值。
     * [语法]:
     *     [命令]: ^DPLMNLIST=<SEQ>,<VER>,<EHPLMNNUM>,<EHPLMN>,<BSCOUNT>,<BS1>,[<BS2>,<BS3>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况返回值：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^DPLMNLIST?
     *     [结果]: <CR><LF>^DPLMNLIST: <VER><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^DPLMNLIST=?
     *     [结果]: <CR><LF>^DPLMNLIST: (list of supported <SEQ>s),(str),(list of supported <EHPLMN NUM>s),(str),(list of supported <BS COUNT>s),(str),(str),(str)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <SEQ>: 流水号：取值范围为1至255，最后一条为255，若只有一条，则为255
     *     <VER>: 云通讯DPLMN版本号
     *     <EHPLMN NUM>: EHPLMN个数，取值范围1~16。
     *     <EHPLMN>: EHPLMN列表
     *     <BS COUNT>: <BS>个数，取值为1-3
     *     <BS1>: <DPLMN NUM>,<DPLMN>,<RAT>[,<DPLMN>,<RAT>,……]
     *             其中
     *             <DPLMN NUM>：该条AT命令里DPLMN总个数
     *             DPLMN和RAT之间以“逗号”隔开，RAT和下个DPLMN(如果有)之间也以“逗号”隔开
     *             <RAT>：1: GSM (Bit1)
     *             2: WCDMA (Bit2)
     *             3: GW (Bit1,Bit2)
     *             4: LTE (Bit3)
     *             5: GL (Bit1,Bit3)
     *             6: WL (Bit2,Bit3)
     *             7: GWL(Bit1,Bit2,Bit3)
     *             8: NR(Bit4)
     *             9: G+NR(Bit1,Bit4)
     *             A:W+NR(Bit2,Bit4)
     *             B:G+W+NR(Bit1,Bit2,Bit4)
     *             C:L+NR(Bit3,Bit4)
     *             D:G+L+NR(Bit1,Bit3,Bit4)
     *             E:W+L+NR(Bit2,Bit3,Bit4)
     *             F:G+W+L+NR(Bit1,Bit2,Bit3,Bit4)
     *             注：RAT值按照16进制来填写
     *     <BS2>: <DPLMN>,<RAT>,<DPLMN>,<RAT>,……
     *             BS COUNT为2或3时，BS2才有效；
     *             DPLMN和RAT之间以“逗号”隔开，RAT和下个DPLMN(如果有)之间也以“逗号”隔开。
     *     <BS3>: <DPLMN>,<RAT>,<DPLMN>,<RAT>,……
     *             BS COUNT为3时，BS3才有效；
     *             DPLMN和RAT之间以“逗号”隔开，RAT和下个DPLMN(如果有)之间也以“逗号”隔开。
     * [示例]:
     *     · 配置卡对应的全球预置DPLMN列表
     *       AT^DPLMNLIST=01,"01.01.001",2,"46000,46007",1,"1,310410,2"
     *       OK
     *     · 查询命令
     *       AT^DPLMNLIST?
     *       ^DPLMNLIST: 01.01.001
     *       OK
     *     · 支持NR时，配置卡对应的全球预置DPLMN列表（配置支持GULNR）
     *       AT^DPLMNLIST=01,"01.01.001",2,"46000,46007",1,"1,310410,F"
     *       OK
     */
    { AT_CMD_DPLMNLIST,
      AT_SetDplmnListPara, AT_SET_PARA_TIME, AT_QryDplmnListPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^DPLMNLIST", (VOS_UINT8 *)"(1-255),(str),(1-16),(str),(1-3),(str),(str),(str)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置点火状态
     * [说明]: FRAT特性时的点火状态的相关操作。
     * [语法]:
     *     [命令]: ^FRATIGNITION=<STATUS>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^FRATIGNITION?
     *     [结果]: <CR><LF>^ FRATIGNITION: < STATUS > <CR><LF>
     *             <CR><LF> OK<CR><LF>
     *     [命令]: ^FRATIGNITION=?
     *     [结果]: <CR><LF>^ FRATIGNITION: (list of supported <STATUS>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <STATUS>: 整数型，点火状态，取值0～1。
     *             0：关
     *             1：开
     * [示例]:
     *     · 开启点火状态
     *       AT^FRATIGNITION=1
     *       OK
     *     · 查询点火状态
     *       AT^FRATIGNITION?
     *       ^FRATIGNITION: 1
     *       OK
     */
    { AT_CMD_FRATIGNITION,
      AT_SetFratIgnitionPara, AT_SET_PARA_TIME, AT_QryFratIgnitionInfo, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^FRATIGNITION", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 边境信息配置
     * [说明]: 配置卡对应的预置边境信息列表。
     * [语法]:
     *     [命令]: ^BORDERINFO=<OPERATRION>
     *             [,<SEQ>,<VER>,<EHPLMNNUM>,<EHPLMNLIST>,<COUNT>,<BS1>[,<BS2>[,<BS3＞]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR: <err><CR><LF>
     *     [命令]: ^BORDERINFO?
     *     [结果]: <CR><LF>^BORDERINFO: <VER><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^BORDERINFO=?
     *     [结果]: <CR><LF>^BORDERINFO: (list of supported < OPERATRION>s), (list of supported <SEQ>s),
     *             (str), (list of supported <EHPLMN NUM>s), (str), (list of supported <BS COUNT>s),(str),(str),
     *             (str)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <OPERATION>: 0: 新增
     *             1: 删除，删除所有的边境信息，不是删除某一条边境信息。
     *             删除时，除OPERATION外，可以不带其他的参数。
     *     <SEQ>: 序列号: 01,02…FF
     *             若为最后一条或者只有一条，则序列号为FF
     *     <VER>: 边境区域信息版本号，用9个字节表示，比如"01.00.001"
     *     <EHPLMN NUM>: EHPLMN 个数
     *     <EHPLMN LIST>: EHPLMN 列表
     *     <COUNT>: 后续有<COUNT>个BS参数, <COUNT>取值范围为{1,2,3}
     *     <BS1>: <BS1>参数的格式为<NumBorderPlmn>,<BorderArea>,<BorderArea>,……
     *             由于AT命令每个参数的长度有限制，所以<BS1>中包含<NumBorderPlmn>和M个<BorderArea>,如果<BS1>中下发不完所有的边境信息，即NumBorderPlmn大于M，其他的<BorderArea>存在<BS2>（和<BS3>，如果<BS2>中还下发不完所有的边境信息)中，<BS2>和<BS3>中不带<NumBorderPlmn>。
     *             其中
     *             <NumBorderPlmn>:此条AT命令下发的边境区域中包含的PLMN的个数
     *             <BorderArea>:描述边境区域属于每个plmn的LAI/TAI的结构，格式为：{PLMN, Type,nLAC, LAC List}
     *             各参数意义如下：
     *             PLMN: 与HPLMN存在边境关系的VPLMN
     *             Type:指示接下来的是GU LAC还是LTE的TAC，其中，0表示LAC，1表示TAC
     *             nLAC:此边境区域该PLMN下的LAC/TAC个数
     *             LAC List:即由nLAC个LAC/TAC组成的LIST，每个LAC/TAC都是用十六进制表示的。
     *     <BS2>: 如果有<BS2>，<BS2>的格式为<BorderArea>,<BorderArea>,……
     *     <BS3>: 如果有<BS3>，<BS3>的格式为<BorderArea>,<BorderArea>,……
     * [示例]:
     *     · 配置卡对应的预置边境信息列表
     *       AT^BORDERINFO=0,255,"01.00.001",2,"46000,46002",1,"2,45400,0,2,47F,2F27,45401,1,1,104"
     *       OK
     *     · 查询命令
     *       AT^BORDERINFO?
     *       ^BORDERINFO: 01.01.001
     *       OK
     */
    { AT_CMD_BORDERINFO,
      AT_SetBorderInfoPara, AT_SET_PARA_TIME, AT_QryBorderInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^BORDERINFO", (TAF_UINT8 *)"(0,1),(1-255),(str),(1-16),(str),(1-3),(str),(str),(str)" },

    /* 根据干扰预警需求新增AT命令^EMRSSIRPT */
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: RSSI预警上报使能开关
     * [说明]: 该命令用于使能或者禁止^EMRSSIRPT主动上报，控制服务小区RSSI变化超过阈值时，是否上报^EMRSSIRPT。
     * [语法]:
     *     [命令]: ^EMRSSIRPT=<n>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^EMRSSIRPT?
     *     [结果]: <CR><LF>^EMRSSIRPT: <n><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^EMRSSIRPT=?
     *     [结果]: <CR><LF>^EMRSSIRPT: (list of supported <n>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，^EMRSSIRPT上报使能开关。
     *             0：禁止^EMRSSIRPT的主动上报（默认值）
     *             1：使能^EMRSSIRPT的主动上报
     * [示例]:
     *     · 使能^EMRSSIRPT上报功能
     *       AT^EMRSSIRPT=1
     *       OK
     */
    { AT_CMD_EMRSSIRPT,
      AT_SetEmRssiRptPara, AT_SET_PARA_TIME, AT_QryEmRssiRptPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^EMRSSIRPT", (VOS_UINT8 *)"(0-1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: AP下发搜网请求
     * [说明]: 该命令用于AP下发搜网命令。
     *         本命令目前仅支持AP下发指定历史搜LTE或NR的PLMN。
     * [语法]:
     *     [命令]: ^PLMNSRCH=<isbgtype>,<plmn>,<rat>,<srchtype>[,<rsrpthreshold>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <isbgtype>: 整型值，搜网类型是否为BG搜：
     *             0：指定搜；
     *             1：BG搜（暂不支持）。
     *     <plmn>: 字符串类型，要搜索的数字格式的PLMN ID。
     *     <rat>: 整型值，无线接入技术：
     *             0：GSM/GPRS；
     *             2：WCDMA；
     *             7：LTE；
     *             12：NR。
     *     <srchtype>: 整型值，扩展的搜网类型：
     *             0：Full Band搜；
     *             1：History搜；
     *             2：Pref Band搜（暂不支持）。
     *     <rsrpthreshold>: 整型值，信号强度最低阈值，可选参数，仅LTE和NR支持。
     *             参数<rat>为LTE制式时，取值范围44~141；
     *             参数<rat>为NR制式时，取值范围31~156。
     *             说明：当添加该参数时，最终驻留小区信号强度必须高于指定阈值；如果对指定制式和PLMN(当NV2356中detectAndUserHisSrchActiveFlg为打开状态，并且指定PLMN为HPLMN时，会判断指定PLMN以及EHPLMN)中不存在信号强度高于该阈值的小区，返回失败。
     * [示例]:
     *     · 不携带信号强度阈值的指定Full Band搜LTE制式Plmn 46085：
     *       AT^PLMNSRCH=0,46085,7,0
     *       OK
     *     · 携带信号强度阈值的指定历史搜LTE制式Plmn 46085：
     *       AT^PLMNSRCH=0,46085,7,1,100
     *       OK
     *     · 不携带信号强度阈值的指定历史搜NR制式Plmn 45403：
     *       AT^PLMNSRCH=0,45403,12,1
     *       OK
     *     · 携带信号强度阈值的指定历史搜NR制式Plmn 46085：
     *       AT^PLMNSRCH=0,46085,12,1,120
     *       OK
     *     · 携带信号强度阈值非指定范围：
     *       AT^PLMNSRCH=0,46085,7,1,142
     *     · +CME ERROR: Incorrect parameters
     *       AT^PLMNSRCH=0,46085,7,1,10
     *       +CME ERROR: Incorrect parameters
     */
    { AT_CMD_PLMNSRCH,
      AT_SetPlmnSrchPara, AT_COPS_SPEC_SRCH_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PLMNSRCH", (VOS_UINT8 *)PLMNSRCH_CMD_PARA_STRING },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 搜网信息主动上报
     * [说明]: 该命令用于AP-Modem形态下Modem主动上报一轮搜网开始和搜网结束的信息，并在搜网结束时携带搜网结果。该AT命令的主动上报仅受^CURC控制。
     *         上报的内容为搜网开始时，消息中的搜网结果字段为BUTT值。该AT的主动上报只受AT^CURC控制，使用^CURC第一个字节的倒数第五位作为控制位。
     * [语法]:
     *     [命令]: 一轮搜网信息的主动上报（搜网开始和搜网结束）
     *     [结果]: ^PLMNSELEINFO: <PLMN SELE FLAG>,<PLMN SELE RLST>
     * [参数]:
     *     <PLMN SELE FLAG>: 搜网开始/搜网结束的标志位。
     *             0：搜网开始；
     *             1：搜网结束；
     *             2：搜网被中断。
     *     <PLMN SELE RLST>: 搜网结束后的服务状态：
     *             0：正常服务
     *             1：限制服务
     *             2：无服务
     *             3：搜网被中断
     *             4：无效值（上报搜网开始时携带的值）
     * [示例]:
     *       搜网开始
     *       ^PLMNSELEINFO: 0,4
     *       搜网结束，搜网结果为正常服务
     *       ^PLMNSELEINFO: 1,0
     */
    { AT_CMD_PLMNSELEINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PLMNSELEINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 信号强度探测命令及结果主动上报
     * [说明]: 该命令用于AP下发的信号强度探测命令，目前仅支持在GU或HRPD正常服务状态下，探测指定PLMN在LTE制式下的最大信号强度(当NV2356中ucBack2LActiveFlg为打开状态，并且指定PLMN为HPLMN时，NAS会默认探测指定PLMN以及EHPLMN)。
     * [语法]:
     *     [命令]: ^DETECTPLMN=<plmn>,<rat>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^DETECTPLMN=?
     *     [结果]: <CR><LF>^DETECTPLMN: (list of supported <plmn>s),(list of supported <rat>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <plmn>: 数字类型，要搜索的PLMN信息。
     *     <rat>: 整型值，无线接入技术(当前只支持LTE)。
     *             0：GSM/GPRS；
     *             2：WCDMA；
     *             7：LTE;
     *             12：NR。
     * [示例]:
     *     · AP下发探测请求：
     *       AT^DETECTPLMN=46085,7
     *       OK
     *       ^DETECTPLMN: 46085,7,-100
     *     · AP下发探测请求不合法，直接返回失败，不再触发探测过程：
     *       AT^DETECTPLMN=46085,7,1
     *       +CME ERR:50
     */
    { AT_CMD_DETECTPLMN,
      At_SetDetectPlmnRsrp, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DETECTPLMN", (VOS_UINT8 *)"(@oper),(0,2,7,12)" },

#if (FEATURE_DSDS == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: DSDS状态设置是否可以上报
     * [说明]: 该命令用于AP给modem设置是否可以主动上报DSDS的状态。
     * [语法]:
     *     [命令]: ^DSDSSTATE=<enable>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     *     <enable>: 整型值，开启或关闭DSDS状态变化主动上报：
     *             0：关闭主动上报；
     *             1：开启主动上报。
     * [示例]:
     *     · 开启DSDS状态变化主动上报功能，此时MODEM处于DSDS3.0，然后MODEM做一次DSDS状态主动上报。
     *       AT^DSDSSTATE=1
     *       OK
     *       ^DSDSSTATE: 1
     *     · 执行测试命令
     *       AT^DSDSSTATE=?
     *       ^DSDSSTATE: (0,1)
     *       OK
     */
    { AT_CMD_DSDSSTATE,
      AT_SetDsdsStatePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^DSDSSTATE", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 副卡PS业务状态设置及查询
     * [说明]: 本命令用于AP通知Modem副卡当前的PS业务的开始和结束，同时指示在PS业务结束时是否需要发起PS DETACH。
     *         注意：
     *         1、该命令的设置和查询命令只针对Modem1生效；
     *         2、state=1时，ps_detach  必须为0；
     *         3、该命令CL模式不生效；
     *         4、PS DETACH操作只在G或W制式下生效；
     * [语法]:
     *     [命令]: ^PSSCENE=<state>,<ps_detach>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^PSSCENE?
     *     [结果]: 执行查询成功时：
     *             <CR><LF>^PSSCENE: (list of supported <state>), (list of supported <ps_detach>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值，设置副卡PS业务状态。取值范围 0~1。
     *             0：副卡结束PS业务；
     *             1：副卡开始PS业务。
     *     <ps_detach>: 整型值，副卡是否需要执行PS DETACH。
     *             0：不需要DETACH PS域；
     *             1：需要DETACH PS域。
     * [示例]:
     *     · 指示副卡PS业务开始
     *       AT^PSSCENE=1,0
     *       OK
     *     · 指示副卡PS业务结束并发起PS域DETACH
     *       AT^PSSCENE=0,1
     *       OK
     *     · 查询副卡PS业务状态
     *       AT^PSSCENE?
     *       ^PSSCENE：1
     *       OK
     *     · 执行测试命令
     *       AT^PSSCENE=?
     *       ^PSSCENE: (0,1),(0,1)
     *       OK
     */
    { AT_CMD_PSSCENE,
      At_SetPsScenePara, AT_SET_PARA_TIME, AT_QryPsScenePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^PSSCENE", (VOS_UINT8 *)"(0,1),(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 防嗅探短信攻击开启关闭
     * [说明]: 该命令用于下发防嗅探短信攻击开启和关闭。
     *         注：该命令需要与CGCATT命令配套下发；如果返回失败，上层需要重新评估下发该指示；
     * [语法]:
     *     [命令]: ^SMSANTIATTACK=<state>
     *     [结果]: 返回值
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <state>: 整型值，下发的防嗅探攻击状态；
     *             0：防攻击关闭；
     *             1：防攻击开启；
     * [示例]:
     *       AT^SMSANTIATTACK=1
     *       OK
     */
    { AT_CMD_SMSANTIATTACK,
      AT_SetSmsAntiAttackState, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SMSANTIATTACK", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询是否支持防嗅探短信攻击
     * [说明]: 该命令用于查询modem当前是否支持防嗅探短信攻击。
     * [语法]:
     *     [命令]: ^SMSANTIATTACKCAP=<smstype>
     *     [结果]: <CR><LF>^SMSANTIATTACKCAP: <result>,<cause><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <sms type>: 整型值，短信类型；
     *             0：验证码短信；
     *             1：静默短信；（为后续静默短信扩展保留内容）
     *     <result>: 整型值，是否支持防嗅探攻击；
     *             0：支持；
     *             1：不支持；
     *     <caule>: 整型值，不支持原因。
     *             0：无原因，result=0时携带；
     *             1：已开启防嗅探攻击功能；
     *             2：嗅探攻击特性NV关闭；
     *             3：短信能力NV9054配置为不支持；
     *             4：当前驻留在非NV指定的网络；
     *             5：当前存在CS或IMS呼叫；
     *             6：当前SENSOR状态不满足NV配置；
     *             7：当前防攻击不支持静默短信
     * [示例]:
     *       AT^SMSANTIATTACKCAP=0
     *       ^SMSANTIATTACKCAP: 0,0
     *       OK
     *       不支持的情况：
     *       ^SMSANTIATTACKCAP: 1,2
     *       OK
     */
    { AT_CMD_SMSANTIATTACKCAP,
      AT_SetSmsAntiAttackCapQrypara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^SMSANTIATTACKCAP", (VOS_UINT8 *)"(0,1)" },

    { AT_CMD_SCID,
      At_SetScidPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SCID", VOS_NULL_PTR },

    { AT_CMD_CACDC,
      AT_SetCacdcPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CACDC", (VOS_UINT8 *)"(@osid),(@osappid),(0,1)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置动态OOS搜网策略
     * [说明]: 该AT命令用于设置特定场景下的OOS搜网策略。
     * [语法]:
     *     [命令]: ^OOSSRCHSTGY=<enable>,<data>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <enable>: 整型值，OOS搜网动态策略设置状态。取值范围：0- 1
     *             1：设置OOS搜网策略；
     *             0：清除所有搜网策略。
     *     <data>: 十六进制字符串，具体需要设置的搜网策略，enable等于1时有效。
     *     <RecordNum>: 十六进制整型值，需要设置的搜网策略条数。
     *             占用字节大小：一字节
     *             取值范围：1~10
     *     <Record[]>: 数组类型，每条记录为一个特性场景的具体搜网策略：
     *             每条策略记录占用15字节；
     *     <scene>: 十六进制整型值，需要动态配置的搜网策略的场景：
     *             0：微动场景
     *             1：数据业务卡场景
     *             2：非数据业务卡场景
     *             3：车载OOS档位1场景
     *             4：车载OOS档位2场景
     *             5：车载OOS档位3场景
     *             6：车载OOS档位4场景
     *             7：车载OOS档位5场景
     *             占用字节大小：一字节
     *     <TotalTimerLen1>: 十六进制整形值，一阶段搜网总时长，单位：秒
     *             占用字节大小：两字节
     *     <SleepTimerLen1>: 十六进制整形值，一阶段搜网间隔定时器时长，单位：秒
     *             占用字节大小：两字节
     *     <HistoryNum1>: 十六进制整形值，一阶段第几次history之后会变成pref band搜或full band搜
     *             占用字节大小：一字节
     *     <PrefbandNum1>: 十六进制整形值，一阶段第几次pref band之后会变成full band搜。
     *             占用字节大小：一字节
     *     <FullbandNum1>: 十六进制整形值，一阶段第几次full band搜后此阶段结束，进入下一阶段
     *             占用字节大小：一字节
     *     <TotalTimerLen2>: 十六进制整形值，二阶段搜网总时长，单位：秒
     *             占用字节大小：两字节
     *     <SleepTimerLen2>: 十六进制整形值，二阶段搜网间隔定时器时长，单位：秒
     *             占用字节大小：两字节
     *     <HistoryNum2>: 十六进制整形值，二阶段第几次history之后会变成pref band搜或full band搜
     *             占用字节大小：一字节
     *     <PrefbandNum2>: 十六进制整形值，二阶段第几次pref band之后会变成full band搜。
     *             占用字节大小：一字节
     *     <FullbandNum2>: 十六进制整形值，二阶段第几次full band搜后此阶段结束，进入下一阶段
     *             占用字节大小：一字节
     * [示例]:
     *     · 设置OOS搜网策略成功
     *       AT^OOSSRCHSTGY=1,"01010064000A02010001F4000F030302"
     *       OK
     *     · 清除OOS搜网策略成功
     *       AT^OOSSRCHSTGY=0
     *       OK
     */
    { AT_CMD_OOSSRCHSTGY,
      AT_SetOosSrchStgyPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^OOSSRCHSTGY", (VOS_UINT8 *)"(0,1),(@data)" },
#if (FEATURE_MBB_CUST == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 查询和设置搜网策略档位
     * [说明]: 命令用于查询和设置OOS搜网场景对应的5个定制档位：车载OOS档位1、车载OOS档位2、车载OOS档位3、
               车载OOS档位4、车载OOS档位5。5个档位的策略配置参考AT命令OOSSRCHSTRTG。该AT命令为异步命令。受MBB CUST宏控制。
               注意：在设置搜网场景对应的5个定制档位前，必须先通过OOSSRCHSTRTG命令下发该档位的搜网策略。
               TEST命令不支持。
     * [语法]:
     *     [命令]: ^CAROOSGEARS=<gears>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CAROOSGEARS?
     *     [结果]: 执行成功时：
     *             <CR><LF>^CAROOSGEARS:<gears><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <gears>: 十六进制字符串，具体需要启动的档位策略取值：1、2、3、4、5； 代表当前5个档位。
     *             注意：查询前如果未设置档位返回0。
     * [示例]:
     *     · 未设置档位，查询搜网档位值返回0
     *       AT^CAROOSGEARS?
     *       CAROOSGEARS: 0
     *       OK
     *     · 设置档位1，并立即生效
     *       AT^CAROOSGEARS=1
     *       OK
     *     · 再次查询搜网档位值:
     *       AT^CAROOSGEARS?
     *       CAROOSGEARS: 1
     *       OK
     */
    { AT_CMD_CAROOSGEARS,
      AT_SetCarOosGearsPara, AT_SET_PARA_TIME, AT_QryCarOosGearsPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CAROOSGEARS", (VOS_UINT8 *)"(1-5)" },
#endif
    /* SVLTE项目AP需要在无卡时候设置CAATT命令 */
    { AT_CMD_CAATT,
      At_SetAutoAttach, AT_SET_PARA_TIME, At_QryAutoAttach, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE_E5 | CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CAATT", (VOS_UINT8 *)"(0,1)" },

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: IMS业务能力开关
     * [说明]: 通过这个命令可以动态打开和关闭IMS功能对LTE、Utran、Gsm接入技术的支持能力。
     *         注：打开和关闭NR的IMS能力参照^NRIMSSWITCH命令。
     * [语法]:
     *     [命令]: ^IMSSWITCH=<lte_enable>[,[<utran_enable>],[<gsm_enable>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             当前存在IMS业务时命令返回失败；
     *             当前设置了语音优选模式为PS_ONLY，下发关闭LTE域上的IMS功能时返回失败；
     *             切换IMS协议栈对LTE、Utran、Gsm接入技术支持能力失败。
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^IMSSWITCH?
     *     [结果]: <CR><LF>^IMSSWITCH:  <lte_enable>,<utran_enable>,<gsm_enable><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^IMSSWITCH=?
     *     [结果]: <CR><LF>^IMSSWITCH: (list of supported <lte_enable>s),(list of supported <utran_enable>s), (list of supported <gsm_enable>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <lte_enable>: 整型值，打开或关闭LTE网络下的IMS域能力：
     *             0：关闭；
     *             1：打开。
     *     <utran_enable>: 整型值，打开或关闭Utran网络下的IMS域能力，默认值为0：
     *             0：关闭；
     *             1：打开。
     *     <gsm_enable>: 整型值，打开或关闭Gsm网络下的IMS域能力，默认值为0：
     *             0：关闭；
     *             1：打开。
     * [示例]:
     *     · 打开LTE接入技术的IMS能力，关闭Utran、Gsm接入技术的IMS能力
     *       AT^IMSSWITCH=1,0,0
     *       OK
     */
    { AT_CMD_IMSSWITCH,
      AT_SetImsSwitchPara, AT_SET_PARA_TIME, AT_QryImsSwitchPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSSWITCH", (VOS_UINT8 *)"(0,1),(0,1),(0,1)" },

    { AT_CMD_IMSSWITCH,
      AT_SetImsSwitchPara, AT_SET_PARA_TIME, AT_QryImsSwitchPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^LTEIMSSWITCH", (VOS_UINT8 *)"(0,1),(0,1),(0,1)" },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: NR IMS业务能力开关
     * [说明]: 通过这个命令可以通过开关NR IMS能力来禁用和恢复NR制式的能力。
     *         注：1）关闭NR能力的操作必须在NR主摸（SA）下并且当前无NR上的IMS业务的时候才能执行；
     *         2）打开NR能力的操作需要MODEM处于开机状态；
     *         3）本命令使用后不会更改IMSA相关的NV能力配置，即MODEM上下电或者飞行开机后就恢复成原始NV能力。
     * [语法]:
     *     [命令]: ^NRIMSSWITCH=<nr_enable>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^NRIMSSWITCH?
     *     [结果]: <CR><LF>^NRIMSSWITCH: <nr_enable><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^NRIMSSWITCH=?
     *     [结果]: <CR><LF>^NRIMSSWITCH: (list of supported <nr_enable>s) <CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <nr_enable>: 整型值，打开或关闭NR网络下的IMS域能力：
     *             0：关闭；
     *             1：打开。
     * [示例]:
     *     · 打开NR接入技术的IMS能力
     *       AT^NRIMSSWITCH=1
     *       OK
     *     · 关闭NR接入技术的IMS能力
     *       AT^NRIMSSWITCH=0
     *       OK
     */
    { AT_CMD_NRIMSSWITCH,
      AT_SetNrImsSwitchPara, AT_SET_PARA_TIME, AT_QryNrImsSwitchPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^NRIMSSWITCH", (VOS_UINT8 *)"(0,1)" },
#endif

#endif

    { AT_CMD_APPWRONREG,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryAppWronReg, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APPWRONREG", VOS_NULL_PTR },


    { AT_CMD_APHPLMN,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryApHplmn, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^APHPLMN", VOS_NULL_PTR },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    { AT_CMD_USIM_STUB,
      At_SetUsimStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^USIMSTUB", VOS_NULL_PTR },

    { AT_CMD_REFRESH_STUB,
      At_SetRefreshStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^REFRESHSTUB", VOS_NULL_PTR },

    { AT_CMD_AUTO_RESEL_STUB,
      At_SetAutoReselStub, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^AUTORESELSTUB", (TAF_UINT8 *)"(0,1)" },

    { AT_CMD_CMM,
      At_SetCmmPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CMM", (VOS_UINT8 *)"(0-12),(0-255)" },

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置是否允许LTE漫游
     * [说明]: 该命令用于设置和查询是否允许LTE漫游。设置时会修改LTE漫游定制NV项，且仅修改该NV项的允许漫游标志位，不影响该NV项的其它成员的值，在下一次开机的时候才能生效。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^CLTEROAMALLOW=<roam_allow>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：<CR><LF> +CME ERROR: <err> <CR><LF>
     *     [命令]: ^CLTEROAMALLOW?
     *     [结果]: <CR><LF>^CLTEROAMALLOW :<roam_allow><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CLTEROAMALLOW=?
     *     [结果]: <CR><LF>^CLTEROAMALLOW: (list of supported <roam_allow>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <roam_allow>: 是否允许LTE漫游。
     *             0：不允许LTE漫游(此时能否驻留LTE还依赖LTE漫游定制NV项中的国家码列表)；
     *             1：允许LTE漫游。
     * [示例]:
     *     · 设置允许漫游
     *       AT^CLTEROAMALLOW=1
     *       OK
     *     · 查询命令
     *       AT^CLTEROAMALLOW?
     *       ^CLTEROAMALLOW: 1
     *       OK
     *     · 测试命令
     *       AT^CLTEROAMALLOW=?
     *       ^CLTEROAMALLOW: (0,1)
     *       OK
     */
    { AT_CMD_CLTEROAMALLOW,
      AT_SetCLteRoamAllowPara, AT_SET_PARA_TIME, AT_QryCLteRoamAllowPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CLTEROAMALLOW", (VOS_UINT8 *)"(0,1)" },
#endif

    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 服务小区上行频率上报设置
     * [说明]: 该命令用于设置当LTE/WCDMA/HRPD频率变化时主动上报服务小区上行频率信息的使能。
     *         当前系统不是L/W/HRPD时，查询命令返回^ULFREQRPT: 0,0,0,0。
     * [语法]:
     *     [命令]: ^ULFREQRPT=<mode>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^ULFREQRPT?
     *     [结果]: <CR><LF>^ulfreqrpt: <mode>,<rat>,<ulfreq>,<ulbw> <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，是否使能主动上报：
     *             0：关闭；
     *             1：使能；
     *             注：开机默认为0。
     *     <rat>: 整型值，接入技术：
     *             0：无服务
     *             1：WCDMA
     *             2：HRPD
     *             3：LTE
     *     <ulfreq>: 整型值，服务小区上行频率。单位100kHz。
     *     <ulbw>: 整型值，服务小区上行带宽。单位1kHz。
     * [示例]:
     *     · 设置服务小区频率上报使能
     *       AT^ULFREQRPT=1
     *       OK
     *       ^ULFREQRPT: 1,19200,5000
     *     · 执行查询命令
     *       AT^ULFREQRPT?
     *       ^ULFREQRPT: 1,1,19200,5000
     *       OK
     */
    { AT_CMD_ULFREQ,
      AT_SetUlFreqRptPara, AT_SET_PARA_TIME, AT_QryUlFreqPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^ULFREQRPT", (VOS_UINT8 *)"(0-1)" },
};

/* 注册MM定制AT命令表 */
VOS_UINT32 AT_RegisterCustomMmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomMmCmdTbl, sizeof(g_atCustomMmCmdTbl) / sizeof(g_atCustomMmCmdTbl[0]));
}

