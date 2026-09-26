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
#include "at_sim_pam_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_sim_pam_set_cmd_proc.h"
#include "at_sim_pam_qry_cmd_proc.h"
#include "at_test_para_cmd.h"
#include "at_device_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SIM_PAM_CMD_TBL_C

static const AT_ParCmdElement g_atSimPamCmdTbl[] = {
    /* PHONEBOOK */
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取电话本
     * [说明]: 读取当前存储器中位置在<index1>与<index2>之间（闭区间）的电话本记录。
     *         如果只输入<index1>，则只返回在<index1>位置的电话本记录。
     *         测试命令返回当前电话本存储器支持的<number>、<text>的最大长度。
     *         此命令是对“AT+CPBR”命令的扩展。
     *         注意：建议index1-index2差值不大于200，否则会概率出现查询结果返回不全问题，原因是USB缓存为256条，如果一次查询过多，PC处理不过来，会在USB缓存队列中缓存，缓存队列满后则会返回发送失败，整个上报通道没有反压机制，只能丢弃消息，导致上报结果不全。
     * [语法]:
     *     [命令]: ^CPBR=<index1>[,<index2>]
     *     [结果]: <CR><LF> [^CPBR: <index1>,<number>,<type>,<text>,<coding>[<CR><LF>^CPBR: <index2>,<number>,<type>,<text>,<coding>]]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CPBR=?
     *     [结果]: <CR><LF>^CPBR: (list of supported <index>s),[<nlength>],[<tlength>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <index1>\n<index2>: 整型值，电话号码在存储器中的位置。
     *             <index1>、<index2>取值大于零并且小于等于+CPBS?命令返回的<total>。
     *             <index1>小于等于<index2>。
     *             说明
     *             当电话本中实际存储的电话本条数小于由<index1>和<index2>确定的条数时，按照实际存储的电话本记录数显示。
     *     <number>: 字符串类型，电话号码，格式由<type>决定。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <text>: 字符串类型，姓名。
     *     <nlength>: 整型值，电话号码的最大长度。
     *     <tlength>: 整型值，姓名的最大长度。
     *     <coding>: 编码方式，表示<text>字段的字符编码并指定语言。
     *             0：GSM 7 bit Default Alphabet（即以非压缩的7bit值上传）。
     *             1：RAW mode（即将<text>以原始数据格式上传），数据的头包含UNICODE编码方式。
     * [示例]:
     *     · 读取记录（AT＋CPBS设置为“SM”）
     *       AT^CPBR=1,5
     *       ^CPBR: 1,"12345678",129,"reinhardt",0
     *       ^CPBR: 2,"7654321",129,"sophia",0
     *       ^CPBR: 3,"111111111",129,"David",0
     *       ^CPBR: 4,"68644808",129,"80534E4E3AFF",1
     *       ^CPBR: 5,"+8613902934081",145," 82038363800031FF",1
     *       OK
     *     · 读取记录（AT+CPBS设置为“ON”）
     *       AT^CPBR=1
     *       ^CPBR: 1,"+8612345678",145,"reinhardt",0
     *       OK
     *     · 查询位置范围、号码和姓名的最大长度
     *       AT^CPBR=?
     *       ^CPBR: (1-65535),20,100
     *       OK
     */
    { AT_CMD_CPBR,
      At_SetCpbrPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CPBR", (TAF_UINT8 *)"(1-65535),(1-65535)" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 写入电话本
     * [说明]: 在当前存储器中添加、修改、删除一条电话本记录。
     *         如果命令参数中只含有index，则index相应位置的电话本条目将被删除。
     *         如果index字段被省略，但参数中含有number字段，则这条电话本条目将被写到第一个空位置。如果此时没有空位置，则上报：+CME ERROR: memory full。仅当所选电话本类型为“SM”和“ON”中的一种时可进行写操作，在其余类型下执行写操作会在AT命令中返回操作不允许的错误提示。
     *         测试命令返回当前存储器中<number>、<text>的最大长度。
     *         此命令是对“AT+CPBW”的扩展。
     * [语法]:
     *     [命令]: ^CPBW=[<index>][,<number>[,<type>[,<text>,<coding>]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CPBW=?
     *     [结果]: <CR><LF>^CPBW: (list of supported <index>s),[<nlength>],(list of supported <type>s),[<tlength>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <text>: 字符串类型，姓名。
     *     <index>: 整型值，电话号码在存储器中的位置。
     *     <number>: 电话号码，字符串类型（合法字符为：“0”～“9”，“#”，“*”，“+”，“(”，“)”，“-”）。
     *             “(”，“)”和“-”三个字符无论出现在任何位置，处理程序都忽略它们，不会作为非法字符报错。“+”只能出现在号码的起始位置。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <coding>: 整型值，表示<text>字段的字符编码方式并指定语言。
     *             0：“IRA”，ASCII码；
     *             1：“UCS2”，UNICODE码；
     *     <nlength>: 整型值，电话号码的最大长度。
     *     <tlength>: 整型值，姓名的最大长度。
     * [示例]:
     *     · 修改一条记录
     *       AT^CPBW=3,"12345678",129,"reinhardt",0
     *       OK
     *       AT^CPBW=1,”28780808”,129,”80534E4E3A”,1
     *       OK
     *     · 添加一条记录
     *       AT^CPBW=,"12345678",129,"reinhardt",0
     *       OK
     *     · 删除一条记录
     *       AT^CPBW=7
     *       OK
     *     · 查询存储器位置范围，号码最大长度，号码类型和姓名最大长度
     *       AT^CPBW=?
     *       ^CPBW: (1-65535),20,(0-255),100
     *       OK
     */
    { AT_CMD_CPBW,
      At_SetCpbwPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CPBW", (TAF_UINT8 *)"(1-65535),(number),(0-255),(text),(0,1)" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 写入电话本
     * [说明]: 此命令只适用于定制版本，只有在NV项配置后才能使用。基线版本不支持此命令。
     *         该命令用于更新扩展电话本和普通电话本，无论是那种电话本，单板都会把能够写入的信息写入到卡的指定文件中，对于不能写入的内容将丢弃，之后根据写入结果返回AT操作结果。
     *         该命令将电话本条目写入当前已经选中电话本内存中index指定的位置。
     *         如果命令参数中只含有index，则index位置的电话本条目将被删除。如果index字段被省略，但参数中含有num1~num4字段，则这条电话本条目将被写到第一个空位置。如果此时没有空位置，则上报：+CME ERROR: memory full。
     * [语法]:
     *     [命令]: ^SCPBW=[<index>][,<num1>[,<type>[,<num2>[,<type>[,<num3>[,<type>[,<num4>[,<type>[,<text>,<coding>[,email]]]]]]]]]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SCPBW=?
     *     [结果]: <CR><LF>^SCPBW: (list of supported <index>s), [<nlength>],(list of supported <type>s),[<tlength>], [<mlength>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <text>: 字符串类型，姓名。
     *     <index>: 整型值，电话号码在存储器中的位置。
     *     <num1>\n<num2>\n<num3>\n<num4>: 电话号码，字符串类型（合法字符为：“0”～“9”，“#”，“*”，“+”，“(”，“)”，“-”）。
     *             “(”，“)”和“-”三个字符无论出现在任何位置，处理程序都忽略它们，不会作为非法字符报错。“+”只能出现在号码的起始位置。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <coding>: 整型值，<text>字段的字符编码方式并指定语言。
     *             0：“IRA”，ASCII码；
     *             1：“UCS2”，UNICODE码；
     *     <email>: 字符串类型，表示email地址。该字段只在存储介质为USIM卡时有效
     *     <tlength>: 整型值，姓名的最大长度。
     *     <nlenght>: 整型值，号码的最大长度。
     *     <mlength>: 整型值，Email的最大字符数。
     * [示例]:
     *     · 写入一条记录
     *       AT^SCPBW=1,"28780808",129,"",129,"",129,"",129,"80534E4E3A",1,"HW@huawei.com"
     *       OK
     *       注意：数据解释1（该记录所在USIM卡中的位置）,“28780808”（电话号码），129（号码类型），“80534E4E3A”（姓名：华为），1（姓名编码格式UCS2），“HW@huawei.com”（email地址）。
     */
    { AT_CMD_SCPBW,
      At_SetScpbwPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^SCPBW", (TAF_UINT8 *)"(1-65535),(numa),(0-255),(numb),(0-255),(numc),(0-255),(numd),(0-255),(text),(0,1),(email)" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取电话本
     * [说明]: 此命令只适用于定制版本，只有在NV项配置后才能使用。基线版本不支持此命令。
     *         建议index1-index2差值不大于200，否则会概率出现查询结果返回不全问题，原因是USB缓存为256条，如果一次查询过多，PC处理不过来，会在USB缓存队列中缓存，缓存队列满后则会返回发送失败，整个上报通道没有反压机制，只能丢弃消息，导致上报结果不全。
     *         该命令返回当前已经选中的电话本内存中位置index1与index2之间的电话本条目。如果index1到index2之间所有的位置都没有电话本条目，将返回：+CME ERROR: not found。
     *         也可以只输入index1，此时只会返回index1位置的电话本记录，若index1处无记录，也会返回+CME ERROR: not found。
     *         MT将SIM卡中保存的数据透传给TE。
     *         test命令返回当前已经选中的电话本内存的位置范围，以及<number>、<text>和<email>的最大长度。
     *         本命令的字段要求依据中国移动对于USIM电话本vCard数据字段要求规定。
     * [语法]:
     *     [命令]: ^SCPBR=<index1>
     *             [,<index2>]
     *     [结果]: [<CR><LF>^SCPBR: <index1>,<num1>,<type>,<num2>,<type>, <num3>,<type>,<num4>,<type>,<text>,<coding>[,<email>][[...]
     *             <CR><LF>^SCPBR: <index2>,<num1>,<type>,<num2>,<type>, <num3>,<type>,<num4>,<type>,<text>],<coding>[,<email>]]<CR><LF>]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CSPBR=?
     *     [结果]: <CR><LF>^SCPBR: (list of supported <index>s),[<nlength>], [<tlength>],[<mlenth>]<CR><LF><CR><LF>OK<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <text>: 字符串类型，姓名。
     *     <index1>、\n<index2>: 整型值，电话号码在存储器中的位置。
     *     <num1>、\n<num2>、\n<num3>、\n<num4>: 电话号码，字符串类型（合法字符为：“0”～“9”，“#”，“*”，“+”，“(”，“)”，“-”）。
     *             “(”，“)”和“-”三个字符无论出现在任何位置，处理程序都忽略它们，不会作为非法字符报错。“+”只能出现在号码的起始位置。
     *     <type>: 号码地址类型，整型值。
     *             129：普通号码；
     *             145：国际号码（以“+”开头）；
     *             其他取值请参见2.4.11 主动上报来电号码：+CLIP中参数<type>的具体定义。
     *     <coding>: 整型值，编码方式，表示<text>字段的字符编码并指定语言，取值0～1。
     *             0：GSM 7 bit Default Alphabet（即以非压缩的7bit值上传）。
     *             1：RAW mode（即将<text>以原始数据格式上传），数据的头包含UNICODE编码方式。
     *     <email>: 字符串类型，表示email地址。目前，该字段只在存储介质为USIM卡时有效
     *     <tlength>: 整型值，姓名的最大长度。
     *     <nlenght>: 整型值，号码的最大长度。
     *     <mlength>: 整型值，Email的最大字符数。
     * [示例]:
     *     · 读取一条记录
     *       AT^SCPBR=1
     *       ^CPBR: 1,"13902934081",129,"",129,"",129,"",129,"80534E4E3A",1,"HW@huawei.com"
     *       OK
     *       注：数据解释 1（该记录所在存储器中的位置），“13902934081”（电话号码），129（号码类型），“80534E4E3A”（姓名：华为），1（姓名编码格式UCS2），“HW@huawei.com”（email 地址）。
     */
    { AT_CMD_SCPBR,
      At_SetScpbrPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, AT_TestCpbrPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^SCPBR", (TAF_UINT8 *)"(1-65535),(1-65535)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 控制(U)SIM状态变化上报
     * [说明]: 控制当(U)SIM状态发生变化时，MT是否主动上报给TE。
     * [语法]:
     *     [命令]: ^SIMST=[<n>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SIMST=?
     *     [结果]: <CR><LF>^SIMST: (list of supported <n>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整数型，是否主动上报(U)SIM状态，默认值1：
     *             0：禁止主动上报(U)SIM状态；
     *             1：启动主动上报(U)SIM状态。
     * [示例]:
     *     · 禁止主动上报(U)SIM状态
     *       AT^SIMST=0
     *       OK
     *     · 启动主动上报(U)SIM状态
     *       AT^SIMST=1
     *       OK
     */
    { AT_CMD_SIMST,
      At_SetSimstPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SIMST", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: SIM卡状态上报模式设置和查询
     * [说明]: SET命令用于控制主动上报^SIMSQ的显示：当<mode>=0时，无^SIMSQ主动上报；当<mode>=1并且SIM卡状态发生变化时，^SIMSQ: <mode>,<sim_status>上报。
     *         READ命令用于查询当前上报模式和SIM卡当前状态。
     *         TEST命令用于查询所支持上报模式范围。
     *         MBB形态使用，手机形态不支持
     * [语法]:
     *     [命令]: AT^SIMSQ=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             与MT相关的错误
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: AT^SIMSQ?
     *     [结果]: <CR><LF>^SIMSQ: <mode>,<sim_status><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: AT^SIMSQ=?
     *     [结果]: <CR><LF>^SIMSQ: (list of supported <mode>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: -
     *     [结果]: 当<mode>=1时，根据SIM卡状态变化SIMSQ主动上报：
     *             <CR><LF>^SIMSQ: <mode>,<sim_status><CR><LF>
     * [参数]:
     *     <mode>: 整数值，用于控制SIM卡主动上报模式，默认为0
     *             0   关闭SIM卡状态主动上报^SIMSQ
     *             1   使能SIM卡状态主动上报^SIMSQ: <mode>,<sim_status>
     *     <sim_status>: 整数值，SIM卡状态
     *             0     卡不在位 SIM not Inserted
     *             1  卡已插入 SIM Inserted
     *             2     卡被锁   SIM PIN/PUK locked
     *             3     SIMLOCK 锁定(暂不支持上报)
     *             10    卡文件正在初始化  SIM Initializing
     *             11    卡初始化完成 （可接入网络） SIM Initialized  (Network service available)
     *             12    卡初始化完成 （短信和电话本可以接入）SIM Ready  (PBM and SMS access)
     *             98    卡物理失效  （PUK锁死或者卡物理失效）
     *             99    卡移除  SIM removed
     *             100   卡错误（初始化过程中，卡失败）
     *             Note1:  <sim_status>中4-9，13-97均为预留状态
     *             Note2:  暂不考虑虚拟SIM卡
     * [示例]:
     *     · SIM卡PIN锁等待用户解锁时，查询SIM卡状态
     *       AT^SIMSQ?
     *       ^SIMSQ: 1,2
     *       OK
     *     · SIM卡PUK锁等待用户解锁时，查询SIM卡状态（未开启主动上报）
     *       AT^SIMSQ?
     *       ^SIMSQ: 0,2
     *       OK
     *       当模块上电后，检测到未插入SIM卡时
     *       ^SIMSQ: 1,0
     *       当模块检测到SIM卡已插入时
     *       ^SIMSQ: 1,1
     *       SIM卡被拔出时
     *       ^SIMSQ: 1,0
     *       SIM卡被锁等待用户解PIN时
     *       ^SIMSQ: 1,2
     *       SIM卡进入初始化过程时
     *       ^SIMSQ: 1,10
     */
    { AT_CMD_SIMSQ,
      AT_SetParaRspSimsqPara, AT_SET_PARA_TIME, AT_QryParaRspSimsqPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^SIMSQ", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取PNN
     * [说明]: 读取(U)SIM卡内PNN文件内容。
     * [语法]:
     *     [命令]: ^PNN=<pnn_type>[,<index>]
     *     [结果]: <CR><LF>^PNN: <full_name_seg>,<short_name_seg>[,<additional_information>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^PNN?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^PNN=?
     *     [结果]: <CR><LF>^PNN: (list of supported <pnn_type>s), (list of supported <record_range>s), (list of supported <record_len>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <pnn_type>: 整型值，PNN文件类型，取值0～1。
     *             0：读取SIM卡中PNN文件；
     *             1：读取USIM卡中PNN文件。
     *     <index>: 整型值，PNN的记录索引，即读PNN的第index条记录。
     *             （如果不带index参数，则读出PNN的记录列表；如果带index参数，则读出指定索引记录。）
     *     <record_range>: PNN的索引范围。
     *     <full_name_seg>: 长名字段，包括长名的标志符，长度和数据内容。
     *     <short_name_seg>: 短名字段，包括短名的标志符，长度和数据内容。
     *     <additional_information>: 附加消息，目前没有实际用途，只是用来显示SIM卡PNN文件实际内容。
     *     <record_len>: 记录的长度。
     * [示例]:
     *       无。
     */
    { AT_CMD_PNN,
      At_SetPNNPara, AT_SET_PARA_TIME, At_QryPNNPara, AT_QRY_PARA_TIME, AT_TestPnnPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^PNN", (TAF_UINT8 *)"(0,1),(0-255)" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取当前注册网络对应的PNN
     * [说明]: 读取当前注册网络对应的卡内PNN文件内容。没有注册网络时返回ERROR。
     *         执行命令实现PNN选择逻辑，按照协议返回需要显示的PNN。非漫游时，返回SIM卡PNN的第一条记录，漫游时，根据当前注册的漫游网络，从OPL找到与当前PLMN相同的PLMN，再根据OPL记录的PNN索引读PNN。
     *         测试命令返回PNN文件类型，需要注意的是当SIM/USIM卡里面没有opl文件或者没有pnn文件时将返回ERROR信息。
     *         该命令执行受EFust文件约束。
     * [语法]:
     *     [命令]: ^CPNN=<pnn_type>
     *     [结果]: <CR><LF>^CPNN: <full_name>,<full_name_coding>,<full_name_ci>, <short_name>,<short_name_coding>,<shor_name_ci><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CPNN?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^CPNN=?
     *     [结果]: <CR><LF>^CPNN: (list of supported <pnn_type>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <pnn_type>: 整型值，PNN文件类型，取值0～1。
     *             0：读取SIM卡中PNN文件；
     *             1：读取USIM卡中PNN文件。
     *     <full_name>: PLMN的长名，只包含PNN的full name字段的字符部分。
     *     <full_name_coding>: 0：长名用压缩GSM7位编码；
     *             1：长名用UCS2编码。
     *     <full_name_ci>: 长名的add ci值，取值0～1。
     *     <short_name>: PLMN的短名，只包含PNN的short name字段的字符部分。
     *     <short_name_coding>: 0：短名用压缩GSM7位编码；
     *             1：短名用UCS2编码。
     *     <short name_ci>: 短名的add ci值，取值0～1。
     * [示例]:
     *     · 查询当前注册网络的CPNN
     *       AT^CPNN=1
     *       ^CPNN: "D456F32D4EB3CB2022",0,0,"",0,0
     *       OK
     *     · 测试CPNN
     *       AT^CPNN=?
     *       ^CPNN: (0,1)
     *       OK
     */
    { AT_CMD_CPNN,
      At_SetCPNNPara, AT_SET_PARA_TIME, At_QryCPNNPara, AT_QRY_PARA_TIME, AT_TestCpnnPara, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^CPNN", (TAF_UINT8 *)"(0,1)" },
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取OPL
     * [说明]: 读取(U)SIM卡内OPL文件内容。
     *         需要注意的是测试命令，当SIM/USIM卡里面没有opl文件时将返回ERROR（文件不存在）的信息。当SIM/USIM卡里面存在opl文件，但是opl文件里面没有数据时，将直接返回OK。其他情况正常返回。
     *         该命令执行受EFust文件约束。
     * [语法]:
     *     [命令]: ^OPL=<opl_type>[,<index>]
     *     [结果]: <CR><LF>^OPL: <plmn id>,<lac_range>,<index_pnn><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^OPL?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^OPL=?
     *     [结果]: [<CR><LF>^OPL: <opl_type>, (list of supported
     *             <record_range>s),<record_len><CR><LF>]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <opl_type>: 整型值，OPL文件类型，取值0～1。
     *             0：读取SIM卡中OPL文件；
     *             1：读取USIM卡中OPL文件。
     *     <index>: 整型值，OPL的记录索引，即读OPL的第index条记录。
     *             （如果不带index参数，则读出OPL的记录列表；如果带index参数，则读出指定索引记录）
     *     <plmn id>: 整型值，PLMN ID。
     *     <lac_range>: 整型值，本地区域码范围。
     *     <index_pnn>: 整型值，指向PNN的第index_pnn条记录。
     *     <record_range>: 整型值，PNN的索引范围。
     *     <record_len>: 整型值，记录的长度。
     * [示例]:
     *     · 读取OPL文件第一条记录
     *       AT^OPL=1,1
     *       ^OPL: 23201,0x0-0xFFFE,1
     *       OK
     */
    { AT_CMD_OPL,
      At_SetOPLPara, AT_SET_PARA_TIME, At_QryOPLPara, AT_QRY_PARA_TIME, AT_TestOplPara, AT_SET_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (TAF_UINT8 *)"^OPL", (TAF_UINT8 *)"(0,1),(0-255)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 读取SPN
     * [说明]: 该命令用于TE通过ME查询当前SIM/USIM卡上存储的2G/3G的SPN文件。
     * [语法]:
     *     [命令]: ^SPN=<spn_type>
     *     [结果]: <CR><LF>^SPN: <disp_cond>,<coding>,<spn_name><CR><LF><CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^SPN=?
     *     [结果]: <CR><LF>^SPN: (list of supported <spn_type>)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <spn_type>: 整型值，SPN文件类型。
     *             0：GSM_SPN；
     *             1：USIM_SPN。
     *     <disp_cond>: 整型值，非漫游时的显示控制条件，包含两位：b1控制RPLMN显示条件，b2控制SPN显示条件（b2仅在E5形态上报）。
     *             b1=0：不显示；
     *             b1=1：显示；
     *             b2=0：显示；
     *             b2=1：不显示；
     *             99：该字段无效，且无须再读取spn_name字段。
     *     <coding>: 整型值，spn_name字段的字符编码方案并指定语言。
     *             0：GSM 7 bit Default Alphabet（即以非压缩的7bit值上传）；
     *             1：RAW mode（即将spn_name以原始数据格式上传）。
     *     <spn_name>: 字符串，SPN内容。
     *             当<coding>=0时，不超过16个字符。
     *             当<coding>=1时，SPN内容以“16进制文本值”进行描述，长度不超过32。单板上报时，统一按照去掉0x的16进制填写。
     * [示例]:
     *     · 查询当前SPN，7bit编码
     *       AT^SPN=1
     *       ^SPN: 1,0,"vodafone"
     *       OK
     *     · 查询当前SPN，UCS2编码
     *       AT^SPN=1
     *       ^SPN: 1,1,"804E2D56FD79FB52A8"
     *       OK
     */
    { AT_CMD_SPN,
      At_SetSPNPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^SPN", (VOS_UINT8 *)"(0-1)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 识别SIM/USIM卡模式
     * [说明]: 此命令查询当前插入的SIM卡的类型。
     *         CL模式下该命令不支持。
     * [语法]:
     *     [命令]: ^CARDMODE
     *     [结果]: <CR><LF>^CARDMODE: <sim_type><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <sim_type>: 整型值，SIM卡的类型，取值0～2。
     *             0：卡不存在；
     *             1：SIM卡；
     *             2：USIM卡。
     * [示例]:
     *     · 查询当前卡类型
     *       AT^CARDMODE
     *       ^CARDMODE: 2
     *       OK
     */
    { AT_CMD_CARDMODE,
      At_SetCardModePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CARDMODE", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: vSIM卡状态
     * [说明]: 该命令用于设置或查询SIM硬卡或vSIM虚拟卡的激活状态。
     *         不能同时激活SIM硬卡和vSIM虚拟卡。查询命令的返回结果中，参数2固定为1，参数4代表卡的激活状态，0：去激活；1：激活。
     * [语法]:
     *     [命令]: ^HVSST=<index>,<enable>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^HVSST?
     *     [结果]: ^HVSST: <index>,<enable>,<slot>,<state><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^HVSST=?
     *     [结果]: <CR><LF>^HVSST: (list of supported <index>s),(list of supported <enable>s) <CR><LF>OK<CR><LF>
     *             有MT相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <index>: vSIM卡/ SIM硬卡信息的索引值，整型值。
     *             1：SIM硬卡。
     *             11：vSIM虚拟卡。
     *     <enable>: 激活或去激活vSIM虚拟卡/SIM硬卡，整型值，取值范围0~1。
     *             0：去激活对应<index>的卡，Modem将不会使用对应的卡注册网络；
     *             1：激活对应<index>的卡，Modem将使用对应的卡注册网络。
     *     <slot>: 当前Modem对应的SIM卡槽，整型值。
     *             0：SIM卡槽0；
     *             1：SIM卡槽1；
     *             2：SIM卡槽2。
     *     <state>: 对应<index>的卡是否用于接入网络，整型值。
     *             0：未用于接入网络；
     *             1：用于接入网络。
     * [示例]:
     *     · 在当前Modem上激活vSIM虚拟卡
     *       AT^HVSST=11,1
     *       OK
     *     · 查询vSIM虚拟卡/SIM硬卡激活状态，返回vSIM虚拟卡已经启用，且Modem对应SIM卡槽0
     *       AT^HVSST?
     *       ^HVSST: 11,1,0,1
     *       OK
     *     · 测试命令
     *       AT^HVSST=?
     *       ^HVSST: (1,11),(0,1)
     *       OK
     */
    { AT_CMD_HVSST,
      At_SetHvsstPara, AT_SET_PARA_TIME, At_QryHvsstPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^HVSST", (VOS_UINT8 *)"(1,11),(0,1)" },

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 受限(U)SIM访问
     * [说明]: 该命令用于支持不同应用类型的卡文件的访问，IMS GBA功能使用。
     * [语法]:
     *     [命令]: ^CURSM=<app_type>,<command>,<fileid>,<record_number>[,<data>[,<pathid>]]
     *     [结果]: <CR><LF>^CURSM: <data><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CURSM=?
     *     [结果]: <CR><LF>^CURSM: (list of supported< app_type >s),(list of supported< command >s) (list of supported < fileid >s),(list of supported < record_number >s) (<data>),(<path>)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <app_type>: USIM应用类型
     *             0：USIM；
     *             1：ISIM。
     *     <command>: 整型值，卡的访问命令。
     *             0：读取文件；
     *             1：更新文件。
     *     <fileid>: 整型值，EF文件 ID。
     *     <record_number>: 整型值，记录号。
     *     <data>: 字符串类型，最大长度512字节。
     *     <pathid>: 暂时不使用。
     */
    { AT_CMD_CURSM,
      AT_SetUrsmPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CURSM", (VOS_UINT8 *)"(0,1),(0,1),(1-65535),(1-255),(data),(path)" },
#endif

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: CDMA卡能力查询
     * [说明]: 此命令仅在CL+G拼片产品形态下查询CDMA卡的能力时使用。其它情况下使用可能会得到错误的返回值。
     *         该命令用于CL+G拼片产品形态下查询CDMA卡的能力。对于UICC卡，可能有CSIM应用和/或USIM应用。对于ICC卡，可能有DF_CDMA和/或DF_GSM。通过此命令可以获取CDMA卡上究竟有哪些应用或DF。
     * [语法]:
     *     [命令]: ^CARDTYPE?
     *     [结果]: <CR><LF>^CARDTYPE: <CARDMODE>,<HAS_C>,<HAS_G><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <CARDMODE>: 卡类型标识。
     *             1：ICC卡；
     *             2：UICC卡。
     *     <HAS_C>: 标识卡上是否具有支持CDMA的能力，即UICC卡上存在CSIM APP时为1，ICC卡上存在DF_CDMA时为1，不支持CDMA时为0。
     *     <HAS_G>: 标示卡上是否有UMTS/GSM能力，即UICC卡上存在USIM APP时为1，ICC卡上存在DF_GSM时为1，不支持UMTS/GSM时为0。
     * [示例]:
     *     · 使用联通USIM卡时查询结果
     *       AT^CARDTYPE?
     *       ^CARDTYPE: 2, 0, 1
     *       OK
     *     · 使用电信4G卡(CSIM+USIM）卡时查询结果
     *       AT^CARDTYPE?
     *       ^CARDTYPE: 2, 1, 1
     *       OK
     *     · 使用移动ICC卡(DF_GSM)时查询结果
     *       AT^CARDTYPE?
     *       ^CARDTYPE: 1, 0, 1
     *       OK
     *     · 使用电信ICC现网卡(DF_CDMA+DF_GSM)时查询结果
     *       AT^CARDTYPE?
     *       ^CARDTYPE: 1, 1, 1
     *       OK
     *     · 使用电信ICC现网卡(只有DF_CDMA)时查询结果
     *       AT^CARDTYPE?
     *       ^CARDTYPE: 1, 1, 0
     *       OK
     */
    { AT_CMD_CARDTYPE,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryCardTypePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CARDTYPE", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: CDMA卡能力查询
     * [说明]: 该命令用于查询CDMA卡的能力。对于UICC卡，可能有CSIM应用和/或USIM应用。对于ICC卡，可能有DF_CDMA和/或DF_GSM。通过此命令可以获取CDMA卡上究竟有哪些应用或DF。
     * [语法]:
     *     [命令]: ^CARDTYPEEX?
     *     [结果]: <CR><LF>^CARDTYPE: <CARDMODE>,<HAS_C>,<HAS_G><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <CARDMODE>: 卡类型标识。
     *             1：ICC卡；
     *             2：UICC卡。
     *     <HAS_C>: 标识卡上是否具有支持CDMA的能力，即UICC卡上存在CSIM APP时为1，ICC卡上存在DF_CDMA时为1，不支持CDMA时为0。
     *     <HAS_G>: 标示卡上是否有UMTS/GSM能力，即UICC卡上存在USIM APP时为1，ICC卡上存在DF_GSM时为1，不支持UMTS/GSM时为0。
     * [示例]:
     *     · 使用联通USIM卡时查询结果
     *       AT^CARDTYPEEX?
     *       ^CARDTYPEEX: 2, 0, 1
     *       OK
     *     · 使用电信4G卡(CSIM+USIM）卡时查询结果
     *       AT^CARDTYPEEX?
     *       ^CARDTYPEEX: 2, 1, 1
     *       OK
     *     · 使用移动ICC卡(DF_GSM)时查询结果
     *       AT^CARDTYPEEX?
     *       ^CARDTYPEEX: 1, 0, 1
     *       OK
     *     · 使用电信ICC现网卡(DF_CDMA+DF_GSM)时查询结果
     *       AT^CARDTYPEEX?
     *       ^CARDTYPEEX: 1, 1, 1
     *       OK
     *     · 使用电信ICC现网卡(只有DF_CDMA)时查询结果
     *       AT^CARDTYPEEX?
     *       ^CARDTYPEEX: 1, 1, 0
     *       OK
     */
    { AT_CMD_CARDTYPEEX,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryCardTypeExPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CARDTYPEEX", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 设置6F7E卡文件信息
     * [说明]: 该命令用于AP通知设置和查询6f7e卡文件信息。
     * [语法]:
     *     [命令]: ^EFLOCIINFO=<tmsi>,<plmn>,<lac>,<location_update_status>,<rfu>
     *     [结果]: 设置正确：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<CR><LF>
     *     [命令]: ^EFLOCIINFO?
     *     [结果]: <CR><LF>^EFLOCIINFO: <tmsi>,<plmn><lac>,<location_update_status>,<rfu><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^EFLOCIINFO=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <tmsi>: 4个字节, TMSI信息。
     *     <plmn>: 字符串类型，PLMN信息。
     *     <lac>: 两个字节，LAC信息。
     *     <location_update_status>: 1个字节：
     *             0: updated
     *             1: not updated
     *             2: plmn not allowed
     *             3: location area not allowed
     *             4~255: reserved。
     *     <rfu>: 保留字段。
     * [示例]:
     *     · 清空6f7e卡文件信息：
     *       AT^EFLOCIINFO=0xffffffff,"ffffff",0x0000,1,0xff
     *       OK
     *     · 查询6f7e卡文件信息:
     *       AT^EFLOCIINFO?
     *       ^EFLOCIINFO: 0xffffffff,"ffffff",0x0000,1,0xff
     *       OK
     */
    { AT_CMD_EFLOCIINFO,
      At_SetEflociInfoPara, AT_SET_PARA_TIME, At_QryEflociInfoPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^EFLOCIINFO", (VOS_UINT8 *)"(0-4294967295),(PLMN),(0-65535),(0-255),(0-255)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 设置6F73卡文件信息
     * [说明]: 该命令用于AP通知设置和查询6f73卡文件信息。
     * [语法]:
     *     [命令]: ^EFPSLOCIINFO=<ptmsi>,<Ptmsi_signature><plmn>,<lac>,<rac><routing_area_update_status>
     *     [结果]: 设置正确：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<CR><LF>
     *     [命令]: ^EFPSLOCIINFO?
     *     [结果]: <CR><LF>^EFPSLOCIINFO: <Ptmsi>,<Ptmsi_signature>,<plmn>,<lac>,<rac><routing_area_update_status><CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^EFPSLOCIINFO=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <ptmsi>: 四个字节, PTMSI信息。
     *     <ptmsi_signature>: 三个字节，PTMSI SIGNATURE信息
     *     <plmn>: 字符串类型，PLMN信息。
     *     <lac>: 两个字节，LAC信息。
     *     <rac>: 一个字节，RAC信息
     *     <routing_area_update_status>: 1个字节：
     *             0: updated
     *             1: not updated
     *             2: plmn not allowed
     *             3: routing area not allowed
     *             4~255: reserved。
     * [示例]:
     *     · 清空6f73卡文件信息：
     *       AT^EFPSLOCIINFO=0xffffffff,0xffffff,"ffffff",0x0000,0xff,1
     *       OK
     *     · 查询6f73卡文件信息:
     *       AT^EFPSLOCIINFO?
     *       ^EFPSLOCIINFO: 0xffffffff,0xffffff,"ffffff",0x0000,0xff,1
     *       OK
     */
    { AT_CMD_EFPSLOCIINFO,
      At_SetPsEflociInfoPara, AT_SET_PARA_TIME, At_QryPsEflociInfoPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^EFPSLOCIINFO", (VOS_UINT8 *)"(0-4294967295),(0-16777215),(PLMN),(0-65535),(0-255),(0-255)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 私有打开逻辑通道
     * [说明]: 本命令用于打开逻辑通道。
     * [语法]:
     *     [命令]: ^CCHO=<dfname>
     *     [结果]: 执行成功时：
     *             <CR><LF>^CCHO: <sessionId>,<length>,<response><CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^CCHO=?
     *     [结果]: <CR><LF>OK CR><LF>
     * [参数]:
     *     <dfname>: 整形值，应用名称，长度1~16字节。
     *     <sessionId>: 整形值，逻辑通道号。这是AT命令中的标识符，用来发送APDU命令到UICC通道。
     *     <length>: 整形值，选择应用返回数据的长度
     *     <response>: 选择应用返回的数据
     * [示例]:
     *     · 打开逻辑通道
     *       AT^CCHO="A0000000871002FF86FF0389FFFFFFFF"
     *       ^CCHO: 123456,4,"9000"
     *     · 测试^CCHO
     *       AT^CCHO=?
     *       OK
     */
    { AT_CMD_PRIVATECCHO,
      At_SetPrivateCchoPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CCHO", (TAF_UINT8 *)"(dfname)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 私有打开逻辑通道
     * [说明]: 本命令用于打开逻辑通道。
     * [语法]:
     *     [命令]: ^CCHP=<dfname>,<P2>
     *     [结果]: 执行成功时：
     *             <CR><LF>^CCHP: <sessionId>,<length>,<response><CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^CCHP=?
     *     [结果]: <CR><LF>OK CR><LF>
     * [参数]:
     *     <dfname>: 整形值，应用名称，长度1~16字节。
     *     <P2>: 整型值，APDU 命令的P2参数。取值 0~255。目前支持：
     *             0x00:激活应用。
     *             0x40:终止应用。
     *             0x04:打开逻辑通道，需要带回  FCP。
     *             0x0C:打开逻辑通道，没有数据带回。
     *     <sessionId>: 整形值，逻辑通道号。这是AT命令中的标识符，用来发送APDU命令到UICC通道。
     *     <length>: 整形值，选择应用返回数据的长度
     *     <response>: 选择应用返回的数据
     * [示例]:
     *     · 打开逻辑通道
     *       AT^CCHP="A0000000871002FF86FF0389FFFFFFFF",4
     *       ^CCHP:450647875,102,"622F820278218410A0000000871002FF86FF0389FFFFFFFF8A01058B036F0629C60F9001A09501008301018301819000"
     *     · 测试^CCHP
     *       AT^CCHP=?
     *       OK
     */
    { AT_CMD_PRIVATECCHP,
      At_SetPrivateCchpPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CCHP", (TAF_UINT8 *)"(dfname),(0-255)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 设置卡返回BUSY的等待时间
     * [说明]: 本命令用于配置控制卡返回BUSY时的等待时间，即修改NV4012中的等待时间。
     * [语法]:
     *     [命令]: ^BWT=<newTime>
     *     [结果]: 执行设置成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^BWT=?
     *     [结果]: <CR><LF>^BWT: (list of supported <newTime>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <newTime>: 设置需要等待的时长，取值范围0~65535，单位为秒。
     * [示例]:
     *     · 设置卡返回BUSY的等待时间
     *       AT^BWT=5
     *       OK
     *     · 测试命令
     *       AT^BWT=?
     *       ^BWT: (0-65535)
     *       OK
     */
    { AT_CMD_BWT,
      At_SetBwtPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^BWT", (VOS_UINT8 *)"(0-65535)" },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: SIM卡初始化在位查询
     * [说明]: 本命令用于查询SIM卡初始化失败场景下是否有SIM卡在位。
     * [语法]:
     *     [命令]: ^CARDHISTORYATR
     *     [结果]: 执行成功时：
     *             <CR><LF>^CARDHISTORYATR: <ATR><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             执行错误时：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: ^CARDHISTORYATR=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <ATR>: SIM卡返回的ATR数据
     * [示例]:
     *     · SIM卡初始化失败场景下查询sim卡的ATR数据：
     *       AT^CARDHISTORYAT
     *       ^CARDHISTORYAT: "3B9F94801FC78031E073FE21135712291102010000C3"
     *       OK
     */
    { AT_CMD_CARDHISTORYATR,
      At_SetCardHistoryATRPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^CARDHISTORYATR", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 查询卡应用SESSION ID
     * [说明]: 此命令仅在UICC卡下时使用，在ICC卡下使用会得到错误的返回值。
     *         该命令用于查询UICC卡上应用的SESSSION ID，目前返回CSIM、USIM、ISIM应用对应的值，该命令用于在CRLA之前确认访问通道的Session ID的值。
     * [语法]:
     *     [命令]: ^CARDSESSION?
     *     [结果]: <CR><LF>^CARDSESSION: CSIM, <sessionid>, USIM, <sessionid>, ISIM, <sessionid>, <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR:<err><CR><LF>
     * [参数]:
     *     <sessionid>: 十进制整型值，0代表此应用不存在，其他值为正常。
     * [示例]:
     *     · 查询UICC卡应用
     *       AT^CARDSESSION?
     *       ^CARDSESSION: CSIM,123456,USIM,0,ISIM,234567
     *       OK
     */
    { AT_CMD_CARDSESSION,
      TAF_NULL_PTR, AT_NOT_SET_TIME, At_QryCardSession, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CARDSESSION", VOS_NULL_PTR },


    /*
     * [类别]: 协议AT-(U)SIM相关
     * [含义]: 受限逻辑通道访问
     * [说明]: 该命令用于在FEATURE_UE_UICC_MULTI_APP_SUPPORT宏打开的版本上，AP在卡的逻辑通道上面访问UICC卡的文件，ICC卡不支持多逻辑通道，但是使用改命令时候依然需要输入对应应用的Session ID。
     *         ICC卡，请参见“3GPP 51.011”的“9.2 Coding of the commands”对于命令的编码。
     *         UICC卡，请参见“ETSI TS 102.221”的“11 Commands”对于命令的编码。
     *         本命令遵从“3GPP TS 27.007”协议。
     * [语法]:
     *     [命令]: ^CRLA=<sessionid>,<command>[,<fileid>[,<P1>,<P2>,<P3>[,<data>[,<pathid>]]]]
     *     [结果]: <CR><LF>^CRLA: <sw1>,<sw2>[,<response>]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CRLA=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <sessionid>: 来自于AT^CARDSESSION的返回值，假如AT^CARDSESSION返回失败则此AT命令无法正常使用
     *     <command>: 整型值，SIM访问命令：
     *             162：查询记录文件有效记录信息；
     *             176：读取二进制文件；
     *             178：读取记录内容；
     *             192：获取响应；
     *             214：更新二进制文件；
     *             220：更新记录内容；
     *             242：状态查询。
     *     <fileid>: 整型值，EF文件ID，除状态查询以外的命令均必须下发该参数。
     *     <P1>,<P2>,<P3>: 整型值，除了状态查询命令，这些参数必须指定，参见“3GPP TS 51.011  9.2 Coding of the commands”。
     *     <data>: 以十六进制格式的信息字段。
     *     <pathid>: 字符串类型，包含卡上EF文件的路径（请参考ETSI TS 102 221）。如果有fileid字段，则pathid必填，并且路径从3F 00开始
     *     <sw1>,<sw2>: 整型值，命令执行后SIM卡返回的响应。
     *     <response>: 携带命令执行成功后上报的数据，对于二进制更新和记录更新命令，<response>不返回。
     * [示例]:
     *     · 卡状态查询
     *       AT^CRLA=123456,242
     *       ^CRLA: 90,0,"622D82027821840CA0000000871002FF49FF0589A503DA01038A0 1058B03 2F0603C60C90016083010183010C830181"
     *       OK
     *     · 读取IMSI
     *       AT^CRLA=123456,176,28423,0,0,9,,"3F007FFF"
     *       ^CRLA: 90,0,"08490620281190 8074"
     *       OK
     *     · 获取响应
     *       AT^CRLA=123456,192,20272,0,0,15,,"3F007F105F3A"
     *       ^CRLA: 144,0,"621E82054221004C0283024F30A5038001718A01058B036F060 5800200988800"
     *     · 查询记录文件有效记录信息
     *       AT^CRLA=71938592,162,0x6F3B,1,4,0x1C,"FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF","3F007FFF"
     *       ^CRLA: 144,0,"0102030405060708090A"
     *       OK
     */
    { AT_CMD_CRLA,
      At_SetCrlaPara, AT_SET_PARA_TIME, TAF_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (TAF_UINT8 *)"^CRLA", (TAF_UINT8 *)"(1-4294967295),(0-255),(1-65535),(0-255),(0-255),(0-255),(cmd),(pathid)" },
};

/* 注册sim pam AT命令表 */
VOS_UINT32 AT_RegisterSimPamCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atSimPamCmdTbl, sizeof(g_atSimPamCmdTbl) / sizeof(g_atSimPamCmdTbl[0]));
}

