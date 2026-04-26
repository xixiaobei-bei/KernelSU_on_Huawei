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

#include "at_phone_ims_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_phone_ims_set_cmd_proc.h"
#include "at_phone_ims_qry_cmd_proc.h"
#include "at_device_cmd.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_IMS_CMD_TBL_C

#if (FEATURE_IMS == FEATURE_ON)
static const AT_ParCmdElement g_atPhoneImsCmdTbl[] = {
    /*
     * [类别]: 协议AT-电话管理
     * [含义]: 设置IMS的当前电池电量信息
     * [说明]: 该命令用于AP通知IMS域当前的电池电量信息。
     * [语法]:
     *     [命令]: ^BATTERYINFO=<Battery_Info>
     *     [结果]: 设置正确：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^BATTERYINFO?
     *     [结果]: 查询成功：
     *             <CR><LF>^BATTERYINFO: <Battery_Info><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^BATTERYINFO=?
     *     [结果]: <CR><LF>^ BATTERYINFO:  (list of supported <Battery_Info>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Battery_Info>: 默认填写为0。
     *             0: 正常;
     *             1: 低电;
     *             2: 电池耗尽
     * [示例]:
     *     · 设置电量信息为低电：
     *       AT^BATTERYINFO=1
     *       OK
     *     · 查询命令：
     *       AT^BATTERYINFO?
     *       ^BATTERYINFO: 1
     *       OK
     *     · 测试命令：
     *       AT^BATTERYINFO=?
     *       ^BATTERYINFO: (0-2)
     *       OK
     */
    { AT_CMD_BATTERYINFO,
      AT_SetBatteryInfoPara, AT_SET_PARA_TIME, AT_QryBatteryInfoPara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^BATTERYINFO", (VOS_UINT8 *)"(0-2)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置语音加密能力指示
     * [说明]: 通过这个命令通知IMS UE是否支持语音加密。
     * [语法]:
     *     [命令]: ^CALLENCRYPT=<state>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CALLENCRYPT=?
     *     [结果]: <CR><LF>^CALLENCRYPT: (list of supported <state>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <state>: 整型值，加密状态：
     *             0：不支持语音加密
     *             1：支持语音加密
     * [示例]:
     *     · 设置支持语音加密
     *       AT^CALLENCRYPT=1
     *       OK
     *     · 执行测试命令
     *       AT^CALLENCRYPT=?
     *       ^CALLENCRYPT: (0,1)
     *       OK
     */
    { AT_CMD_ENCRYPT,
      AT_SetCallEncryptPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CALLENCRYPT", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置IMS的PCSCF地址
     * [说明]: 该命令用于设置IMS域的PCSCF地址。
     * [语法]:
     *     [命令]: ^IMSPCSCF=<Source>,[<Ipv6Address1>],[<Ipv6Address2>],[<Ipv6Address3>],[<Ipv4Address4>],[<Ipv4Address5>],[<Ipv4Address6>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF> +CME ERROR:<err><CR><LF>
     *     [命令]: ^IMSPCSCF?
     *     [结果]: <CR><LF>^IMSPCSCF: <Source>,[<Ipv6Address1>],[<Ipv6Address2>],[<Ipv6Address3>],[<Ipv4Address4>],[<Ipv4Address5>],[<Ipv4Address6>]<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^IMSPCSCF=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Source>: 目前只支持0和1，
     *             0表示采用默认的P-CSCF配置
     *             1表示DM触发的P-CSCF配置，
     *             其他值用于后续扩展。
     *             当<Source>配置为0时，<Ipv6Address1>,<Ipv6Address2>,<Ipv6Address3>,<Ipv4Address1>,
     *             <Ipv4Address2>和<Ipv4Address3> 配置的都是默认的P-CSCF地址，这些配置没法被查询到，不会体现在查询命令的返回值中，因为查询命令只反映来自DM的P-CSCF地址配置，详见示例
     *     <Ipv6Address1>: 配置P-CSCF的第一个IPV6地址，字符串类型。
     *             携带端口号的格式举例：
     *             “[1234:5678:90ab:cdef:1234:5678:90ab:cdef]:8080”
     *             其中，中括号内的为IPV6地址（十六进制格式），中括号外“:”后的为端口号（十进制格式，端口号不能为0或者大于65535）。
     *             没有携带端口号的格式举例：
     *             “1234:5678:90ab:cdef:1234:5678:90ab:cdef”
     *             “”内的为IPV6地址（十六进制格式）
     *             此参数为可选项，不填写或者只填写””时表示需要清除之前保存的第一个IPV6地址
     *     <Ipv6Address2>: 配置P-CSCF的第二个IPV6地址，字符串类型。
     *             地址格式同<Ipv6Address1>。
     *             此参数为可选项，不填写或者只填写””时表示需要清除之前保存的第二个IPV6地址
     *     <Ipv6Address3>: 配置P-CSCF的第三个IPV6地址，字符串类型。
     *             地址格式同<Ipv6Address1>。
     *             此参数为可选项，不填写或者只填写””时表示需要清除之前保存的第三个IPV6地址
     *     <Ipv4Address1>: 配置P-CSCF的第一个IPV4地址，字符串类型。
     *             携带端口号的格式为：
     *             “123.456.789.123:8080”
     *             其中，“:”前的为IPV4地址，“:”后的为端口号，均为十进制格式，（端口号不能为0或者大于65535）
     *             此参数为可选项，不填写或者只填写””时表示需要清除之前保存的第一个IPV4地址
     *     <Ipv4Address2>: 配置P-CSCF的第二个IPV4地址，字符串类型。
     *             地址格式同<Ipv6Address1>。
     *             此参数为可选项，不填写或者只填写””时表示需要清除之前保存的第二个IPV4地址
     *     <Ipv4Address3>: 配置P-CSCF的第三个IPV4地址，字符串类型。
     *             地址格式同<Ipv6Address1>。
     *             此参数为可选项，不填写或者只填写””时表示需要清除之前保存的第三个IPV4地址
     * [示例]:
     *     · 设置第一个IPV4地址，删除其余二个IPV4地址及三个IPV6地址
     *       AT^IMSPCSCF?
     *       ^IMSPCSCF: 1,"1234:0:0:0:0:0:0:5678","[1234:0:0:0:0:0:0:5678]:80","1234:0:0:0:0:0:5678:90ab","1.1.1.1","11.11.11.11","111.111.111.111"
     *       OK
     *       AT^IMSPCSCF=1,,,,"10.40.102.100:80",,
     *       OK
     *       AT^IMSPCSCF?
     *       ^IMSPCSCF: 1,,,,"10.40.102.100:80",,
     *       OK
     *     · 设置第一个IPV6地址 ，删除其余二个IPV6地址及三个IPV4地址
     *       AT^IMSPCSCF=1,"[fe80:0:0:0:75a1:16a4:9252:a03a]:56","","","","",""
     *       OK
     *       AT^IMSPCSCF?
     *       ^IMSPCSCF: 1,"[fe80:0:0:0:75a1:16a4:9252:a03a]:56",,,,,
     *     · 清空配置的来自DM的P-CSCF地址
     *       AT^IMSPCSCF=1,,,,,,
     *       OK
     *       AT^IMSPCSCF?
     *       ^IMSPCSCF: 1,,,,,,
     *       OK
     *     · 配置默认的P-CSCF地址，查询命令不显示配置默认的P-CSCF地址
     *       AT^IMSPCSCF=0,"1234:0:0:0:0:0:0:5678","[1234:0:0:0:0:0:0:5678]:80","1234:0:0:0:0:0:5678:90ab","1.1.1.1","11.11.11.11","111.111.111.111"
     *       OK
     *       AT^IMSPCSCF?
     *       ^IMSPCSCF: 1,,,,,,
     *       OK
     */
    { AT_CMD_PCSCF,
      AT_SetImsPcscfPara, AT_SET_PARA_TIME, AT_QryImsPcscfPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSPCSCF", (VOS_UINT8 *)"(0-4294967295),(data),(data),(data),(data),(data),(data)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置DM的动态变化参数
     * [说明]: 该命令为LGU+专用命令，用于设置IMS域的控制参数。
     * [语法]:
     *     [命令]: ^DMDYN=[<AMR_WB_octet_aligned>],[<AMR_WB_bandwidth_efficient>],[<AMR_octet_aligned],[AMR_bandwidth_efficient>],[<AMR_WB_mode>],[<DTMF_WB>],[<DTMF_NB>],[<Speech_start>],[<Speech_end>],[<Video_start>],[<Video_end>],[<RegRetryBaseTime>],[<RegRetryMaxTime>],[<PhoneContext>],[<Public_user_identity>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     *     [命令]: ^DMDYN?
     *     [结果]: <CR><LF>^DMDYN: [<AMR_WB_octet_aligned>],[<AMR_WB_bandwidth_efficient>],[<AMR_octet_aligned>],[<AMR_bandwidth_efficient>],[<AMR_WB_mode>],[<DTMF_WB>],[<DTMF_NB>],[<Speech_start>],[<Speech_end>],[<Video_start>],[<Video_end>],[<RegRetryBaseTime>],[<RegRetryMaxTime>],[<PhoneContext>],[<Public_user_identity>]<CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DMDYN=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <AMR_WB_octet_aligned>: AMR-WB 字节对齐模式的payload type，取值范围：0x60~0x7F
     *             此参数为可选项，不填说明此参数没有设置
     *     <AMR_WB_bandwidth_efficient>: AMR-WB 宽带节省模式的payload type，取值范围：0x60~0x7F
     *             此参数为可选项，不填说明此参数没有设置。目前此参数为IMSA预留项，即IMSA不使用此参数配置，在下发设置命令设置此参数后，查询命令的返回值仍显示此项为未配置，详见示例
     *     <AMR_octet_aligned>: AMR字节对齐模式的payload type，取值范围：0x60~0x7F
     *             此参数为可选项，不填说明此参数没有设置
     *     <AMR_bandwidth_efficient>: AMR 宽带节省模式的payload type，取值范围：0x60~0x7F
     *             此参数为可选项，不填说明此参数没有设置。目前此参数为IMSA预留项，即IMSA不使用此参数配置，在下发设置命令设置此参数后，查询命令的返回值仍显示此项为未配置，详见示例
     *     <AMR_WB_mode>: AMR-WB速率，取值范围： 0~8
     *             此参数为可选项，不填说明此参数没有设置
     *     <DTMF_WB>: DTMF 16000时钟频率下的payload type，取值范围：0x60~0x7F
     *             此参数为可选项，不填说明此参数没有设置。目前此参数为IMSA预留项，即IMSA不使用此参数配置，在下发设置命令设置此参数后，查询命令的返回值仍显示此项为未配置，详见示例
     *     <DTMF_NB>: DTMF 8000时钟频率下的payload type，取值范围：0x60~0x7F
     *             此参数为可选项，不填说明此参数没有设置。目前此参数为IMSA预留项，即IMSA不使用此参数配置，在下发设置命令设置此参数后，查询命令的返回值仍显示此项为未配置，详见示例
     *     <Speech_start>: 语音RTP port最小值，只能配偶数, 0为无效值
     *             此参数为可选项，不填说明此参数没有设置
     *     <Speech_end>: 语音RTP port最大值，只能配偶数, 0为无效值
     *             此参数为可选项，不填说明此参数没有设置
     *     <Video_start>: 视频RTP port最小值，只能配偶数, 0为无效值
     *             此参数为可选项，不填说明此参数没有设置
     *     <Video_end>: 视频RTP port最大值，只能配偶数, 0为无效值
     *             此参数为可选项，不填说明此参数没有设置
     *     <RegRetryBaseTime>: IMSA选择下一次尝试P-CSCF地址，需要停止的时间，单位为秒。
     *             <RegRetryBaseTime>是一个基数，基于失败次数成指数增加
     *             此参数为可选项，不填说明此参数没有设置
     *     <RegRetryMaxTime>: <RegRetryBaseTime>可以增加到的最大时间，单位为秒。
     *             <RegRetryBaseTime>配置的值需要大于<RegRetryBaseTime>
     *             此参数为可选项，不填说明此参数没有设置
     *     <PhoneContext>: IMS使用参数，字符串数组，取值范围0-128bytes
     *             此参数为可选项，不填或者只填””说明此参数没有设置
     *     <Public_user_identity>: IMS使用参数，字符串数组，取值范围0-128bytes
     *             此参数为可选项，不填或者只填””说明此参数没有设置
     * [示例]:
     *       设置DM动态配置参数，通过查询命令查看设置前后参数配置变化
     *       AMR_WB_octet_aligned = 96;
     *       AMR_WB_bandwidth_efficient= 100;
     *       AMR_octet_aligned = 102
     *       AMR_bandwidth_efficient = 104,
     *       AMR_WB_mode = 8,
     *       DTMF_WB= 106,
     *       DTMF_NB= 108,
     *       Speech_start= 2
     *       Speech_end= 2
     *       Video_start不配置
     *       Video_end不配置
     *       RegRetryBaseTime= 5s
     *       RegRetryMaxTime= 30s
     *       PhoneContext不配置
     *     · Public_user_identity= ""
     *       AT^DMDYN?
     *       ^DMDYN: 96,,96,,1,,,4,6,8,10,30,1800,"test_PhoneContext","test_public_identity"
     *       OK
     *       AT^DMDYN=96,100,102,104,8,106,106,2,2,,,5,30,,""
     *       OK
     *       AT^DMDYN?
     *       ^DMDYN: 96,,102,,8,,,2,2,,,5,30,"test_PhoneContext","test_public_identity"
     *       OK
     */
    { AT_CMD_DMDYN,
      AT_SetDmDynPara, AT_SET_PARA_TIME, AT_QryDmDynPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^DMDYN", (VOS_UINT8 *)"(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(phoneContext),(public_user_id)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置和查询IMS的定时器信息
     * [说明]: 该命令用于设置和查询IMS域的定时器信息。
     * [语法]:
     *     [命令]: ^DMTIMER=[<Timer_T1>],[<Timer_T2>],[<Timer_T4>],[<Timer_TA>],[<Timer_TB>],[<Timer_TC>],[<Timer_TD>],[<Timer_TE>],[<Timer_TF>],[<Timer_TG>],[<Timer_TH>],[<Timer_TI>],[<Timer_TJ>],[<Timer_TK>]
     *     [结果]: 设置正确：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF> +CME ERROR:<err><CR><LF>
     *     [命令]: ^DMTIMER?
     *     [结果]: <CR><LF>^DMTIMER: [<Timer_T1>],[< Timer_T2>],[<  Timer_T4>],[< Timer_TA>],[< Timer_TB>],[< Timer_TC>],[<  Timer_TD>],[< Timer_TE>],[< Timer_TF>],[< Timer_TG>],[<  Timer_TH>],[< Timer_TI>],[< Timer_TJ>],[< Timer_TK>] <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DMTIMER=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Timer_T1>: IMS T1定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_T2>: IMS T2定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_T4>: IMS T4定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TA>: IMS A定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TB>: IMS B定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TC>: IMS C定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TD>: IMS D定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TE>: IMS E定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TF>: IMS F定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TG>: IMS G定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TH>: IMS H定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TI>: IMS I定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TJ>: IMS J定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     *     <Timer_TK>: IMS K定时器，单位为毫秒，取值范围：0~128000；此项可选，若不填写，说明此定时器没有设置时长。
     * [示例]:
     *     · 设置Timer_T1=2，Timer_TD=3的参数
     *       AT^DMTIMER=2,,,,,,3,,,,,,,
     *       OK
     *     · 查询定时器信息：
     *       AT^DMTIMER?
     *       ^DMTIMER: 2,,,,,,3,,,,,,,
     *       OK
     */
    { AT_CMD_DMTIMER,
      AT_SetImsTimerPara, AT_SET_PARA_TIME, AT_QryImsTimerPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^DMTIMER",
      (VOS_UINT8 *)"(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000),(0-128000)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置查询IMS域短信的公共服务标识信息
     * [说明]: 该命令用于设置和查询IMS域的公共服务标识（PSI）信息。
     * [语法]:
     *     [命令]: ^IMSPSI=<SMS_PSI>
     *     [结果]: 设置正确：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF> +CME ERROR:<err><CR><LF>
     *     [命令]: ^IMSPSI?
     *     [结果]: <CR><LF>^IMSPSI: <SMS_PSI ><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^IMSPSI=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <SMS_PSI>: 字符串类型，短信的公共服务标识信息，字符串长度范围0-128bytes
     * [示例]:
     *     · 设置IMS域的短信公共标识信息
     *       AT^IMSPSI="test_sms_psi"
     *       OK
     *     · 查询IMS域的短信公共标识信息
     *       AT^IMSPSI?
     *       ^IMSPSI: "test_sms_psi"
     *       OK
     */
    { AT_CMD_IMSPSI,
      AT_SetImsSmsPsiPara, AT_SET_PARA_TIME, AT_QryImsSmsPsiPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSPSI", (VOS_UINT8 *)"(data)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 查询IMS域的用户信息
     * [说明]: 该命令用于查询IMS域的用户信息。
     * [语法]:
     *     [命令]: ^DMUSER?
     *     [结果]: <CR><LF>^DMUSER: <Private_user_identity_IMPI>
     *             ,<Public_user_identity_IMPU>
     *             ,<Home_network_domain_name>
     *             , <Voice_Domain_Preference_EUTRAN>
     *             ,<SMS_over_IP_Networks_Indication>
     *             ,<Ipsec_enabled >
     *             <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^DMUSER=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Private_user_identity_IMPI>: IMS使用参数，字符串数组，取值范围0-128bytes
     *     <Public_user_identity_IMPU>: IMS使用参数，字符串数组，取值范围0-128bytes
     *     <Home_network_domain_name>: IMS使用参数，字符串数组，取值范围0-128bytes
     *     <Voice_Domain_Preference_EUTRAN>: 整型值，语音呼叫模式，默认值由具体制造商决定。
     *             0：CS Voice only
     *             1：IMS PS Voice only
     *             2：CS Voice preferred, IMS PS Voice as secondary
     *             3：IMS PS Voice preferred, CS Voice as secondary
     *     <SMS_over_IP_Networks_Indication>: 整型值，短信优选域。
     *             0：不使用 IMS发送短信；
     *             1：优先使用IMS发送短信。
     *     <Ipsec_Enabled>: 0：IPSEC包使能
     *             1：IPSEC包不使能
     * [示例]:
     *     · 查询IMS域的用户信息
     *       AT^DMUSER?
     *       ^DMUSER: "test_IMPI", "test_IMPU","test_domain",3,1,0
     *       OK
     */
    { AT_CMD_DMUSER,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryDmUserPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^DMUSER", (VOS_UINT8 *)"(char),(char),(char),(0-3),(0,1),(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 设置漫游下是否支持IMS特性
     * [说明]: 该命令用于设置漫游下是否支持IMS特性，用于上层将在漫游下是否支持IMS特性配置到modem，以及通过此命令查询modem当前在漫游下是否支持IMS特性。
     * [语法]:
     *     [命令]: ^ROAMIMSSERVICE=<InternationalIMSService>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^ROAMIMSSERVICE?
     *     [结果]: <CR><LF>^ROAMIMSSERVICE: < International IMS Service ><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^ROAMIMSSERVICE=?
     *     [结果]: <CR><LF>^ROAMIMSSERVICE: (list of supported < International IMS Service >s) <CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <International IMS Service>: 整数型，漫游下是否支持IMS特性配置，取值0,1。
     *             0：不支持；
     *             1：支持；
     * [示例]:
     *     · 设置漫游下是否支持IMS特性
     *       AT^ROAMIMSSERVICE=1
     *       OK
     *     · 查询漫游下是否支持IMS特性
     *       AT^ROAMIMSSERVICE?
     *       ^ROAMIMSSERVICE: 1
     *       OK
     *     · 执行测试命令
     *       AT^ROAMIMSSERVICE=?
     *       ^ROAMIMSSERVICE: (0,1)
     *       OK
     */
    { AT_CMD_ROAMIMSSERVICE,
      AT_SetRoamImsServicePara, AT_SET_PARA_TIME, AT_QryRoamImsServicePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ROAMIMSSERVICE", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-LTE相关
     * [含义]: 通知发起IMS注册请求
     * [说明]: 该命令用于通知UE发起IMS注册流程。
     * [语法]:
     *     [命令]: ^VOLTEREG
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     * [参数]:
     * [示例]:
     *     · 通知发起IMS注册请求
     *       AT^VOLTEREG
     *       OK
     */
    { AT_CMD_VOLTEREG,
      AT_SetVolteRegPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VOLTEREG", VOS_NULL_PTR },
};

/* 注册phone phy AT命令表 */
VOS_UINT32 AT_RegisterPhoneImsCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atPhoneImsCmdTbl, sizeof(g_atPhoneImsCmdTbl) / sizeof(g_atPhoneImsCmdTbl[0]));
}
#endif
