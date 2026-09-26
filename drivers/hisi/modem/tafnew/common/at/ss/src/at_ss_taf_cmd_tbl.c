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
#include "at_ss_taf_cmd_tbl.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_ss_taf_set_cmd_proc.h"
#include "at_ss_taf_qry_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SS_TAF_CMD_TBL_C

static const AT_ParCmdElement g_atSsTafCmdTbl[] = {
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 呼叫相关补充业务
     * [说明]: 通过这个命令可以控制下列补充业务：
     *         呼叫保持和回复
     *         多方通话
     *         接听和挂断呼叫
     * [语法]:
     *     [命令]: ^CHLD=<n>,<call_type>
     *             [,<num>[,<rtt>]]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <n>: 整型值，操作码。
     *             0：挂断所有被保持的或者等待的呼叫，若同时存在被保持的呼叫和等待的呼叫，则只挂断正在等待的呼叫；
     *             1： 挂断正在通话的呼叫，并接听等待的呼叫或者恢复被保持的呼叫；
     *             2： 保持正在通话的呼叫，并接听等待的呼叫或者恢复被保持的呼叫；
     *             3： 建立多方呼叫；
     *             1x： 挂断第x个呼叫；
     *             2x： 保持除第x个呼叫以外的所有其他呼叫；
     *             4： 将保持的呼叫和当前的来电建立通话，主叫方结束呼叫，或将正在通话的呼叫和等待的呼叫建立通话，主叫方结束呼叫；
     *             5： 激活或接听网络发起的呼叫（CCBS）。
     *             6：增强型多方通话中挂断电话号码为num的用户
     *             7： 合并增强型多方通话与普通电话
     *             8： 接听来电或在一路通话被保持的状态接听另外一路等待的呼叫
     *     <call_type>: 整型值，呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     *     <num>: 字符串类型，需要挂断的电话号码
     *             说明：这个参数只有当<n>=6时，才能下发<num>,否则判为参数错误。
     *     <rtt>: 整型值，指示是否RTT模式接听通话，仅在<n>为2或8时有效：
     *             0：非RTT通话；
     *             1：RTT通话。
     * [示例]:
     *     · 接听语音电话
     *       AT^CHLD=8,0
     *       OK
     *     · 在增强型多方通话中挂断某个用户
     *       AT^CHLD=6,0,"135xxxxxxxx"
     *       OK
     *     · 合并增强型多方通话与普通通话
     *       AT^CHLD=7,0
     *       OK
     *     · 以RTT模式接听来电
     *       AT^CHLD=8,0,,1
     *       OK
     */
    { AT_CMD_CHLD_EX,
      At_SetChldExPara, AT_SET_CALL_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CHLD", (VOS_UINT8 *)"(0-8,11-19,21-29),(0-3),(@number),(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 查询主叫名称
     * [说明]: 来电时查询Unicode编码格式的主叫名称。
     * [语法]:
     *     [命令]: ^CNAP?
     *     [结果]: <CR><LF>^CNAP: <name>,<CNI_validity><CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: ^CNAP=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <name>: 字符串类型，Unicode编码格式的HEX码流，最大长度160字节（320个ASCII字符）。
     *     <CNI_validity>: 整型值，来电名称显示有效性：
     *             0：CNI有效；
     *             1：CNI被主叫方拒绝显示；
     *             2：由于发起网络的限制或网络问题造成CNI不可用。
     * [示例]:
     *     · 来电时查询USC-2编码类型主叫名称为“李雷abc123”
     *       AT^CNAP?
     *       ^CNAP: "674E96F7006100620063003100320033",0
     *       OK
     *     · X模来电时查询
     *       AT^CNAP?
     *       OK
     */
    { AT_CMD_CNAPEX,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCnapExPara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CNAP", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 多线路选择
     * [说明]: 该功能可用的前提是代码中打开多线路选择的宏开关。
     *         同一张SIM卡支持两个电话号码时，可用此命令设置拨出呼叫的号码。
     * [语法]:
     *     [命令]: ^ALS=<view>[,<line>]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^ALS?
     *     [结果]: <CR><LF>^ALS: <view>,<line><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^ALS=?
     *     [结果]: <CR><LF>^ALS: (list of supported <view>s),(list of supported <line>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <view>: 整型值，禁止或使能当前来电线路通知，默认值为0。
     *             0：禁止；
     *             1：使能。
     *     <line>: 整型值，当前主叫时的线路号，默认值为1。
     *             1：拨打电话时使用线路1的号码进行呼叫；
     *             2：拨打电话时使用线路2的号码进行呼叫。
     * [示例]:
     *     · 使能当前来电线路通知，且使用线路1进行呼叫
     *       AT^ALS=1,1
     *       OK
     *     · 查询多线路选择业务
     *       AT^ALS?
     *       ^ALS: 1,1
     *       OK
     *     · 测试ALS
     *       AT^ALS=?
     *       ^ALS: (0,1),(1,2)
     *       OK
     */
    { AT_CMD_ALS,
      AT_SetAlsPara, AT_SET_PARA_TIME, AT_QryAlsPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^ALS", (VOS_UINT8 *)"(0,1),(1,2)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 设置USSD传输模式
     * [说明]: 设置USSD方案为透传或者非透传方案，设置结果无需保存到NV中。单板初始化时先尝试读取NV，如果NV未激活则查找ussd_default_table表设置产品默认的USSD方案。
     *         ^USSDMODE?命令查询单板当前支持的USSD方案，返回全局变量ussd_mode当前值。
     * [语法]:
     *     [命令]: ^USSDMODE=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^USSDMODE?
     *     [结果]: <CR><LF>^USSDMODE: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^USSDMODE=?
     *     [结果]: <CR><LF>^USSDMODE: (list of supported <mode>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值。
     *             0：单板使用USSD非透传方案；
     *             1：单板使用USSD透传方案。
     * [示例]:
     *     · 设置当前模式为透传模式
     *       AT^USSDMODE=1
     *       OK
     */
    { AT_CMD_USSDMODE,
      At_SetUssdModePara, AT_NOT_SET_TIME, At_QryUssdModePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^USSDMODE", (VOS_UINT8 *)"(0-1)" },

#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 变更音视频通话类型
     * [说明]: 变更音视频通话类型。
     * [语法]:
     *     [命令]: ^CALLMODIFYINIT=<idx>,<curr_call_type>,<dest_call_type>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CALLMODIFYINIT=?
     *     [结果]: <CR><LF>^CALLMODIFYINIT: (list of supported <idx>s),(list of supported <curr_call_type>s),(list of supported <dest_call_type>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <idx>: 整型值，呼叫ID，取值范围1~7
     *     <curr_call_type>: 整型值，当前呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     *     <dest_call_type>: 整型值，目标呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     * [示例]:
     *     · 音频通话切换转成视频通话
     *       AT^CALLMODIFYINIT=1,0,3
     *       OK
     *     · 测试命令
     *       AT^CALLMODIFYINIT=?
     *       ^CALLMODIFYINIT: (1-7),(0-3),(0-3)
     *       OK
     */
    { AT_CMD_CALL_MODIFY_INIT,
      AT_SetCallModifyInitPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CALLMODIFYINIT", (VOS_UINT8 *)"(1-7),(0-3),(0-3)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: 接受对方音视频通话类型变更请求
     * [说明]: 接受对方音视频通话类型变更请求。
     * [语法]:
     *     [命令]: ^CALLMODIFYANS=<idx>,<curr_call_type>,<dest_call_type>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CALLMODIFYANS=?
     *     [结果]: <CR><LF>^CALLMODIFYANS: (list of supported <idx>s),(list of supported <curr_call_type>s),(list of supported <dest_call_type>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <idx>: 整型值，呼叫ID，取值范围1~7
     *     <curr_call_type>: 整型值，当前呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     *     <dest_call_type>: 整型值，目标呼叫类型。
     *             0： 语音呼叫
     *             1： 视频通话：单向发送视频，双向语音
     *             2： 视频通话：单向接收视频，双向语音
     *             3： 视频通话：双向视频，双向语音
     * [示例]:
     *     · 接受对方音视频通话类型变更请求
     *       AT^CALLMODIFYANS=1,0,3
     *       OK
     *     · 测试命令
     *       AT^CALLMODIFYANS=?
     *       ^CALLMODIFYANS: (1-7),(0-3),(0-3)
     *       OK
     */
    { AT_CMD_CALL_MODIFY_ANS,
      AT_SetCallModifyAnsPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^CALLMODIFYANS", (VOS_UINT8 *)"(1-7),(0-3),(0-3)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: IMS视频电话能力配置
     * [说明]: 该命令用于配置IMS视频电话相关能力。
     * [语法]:
     *     [命令]: ^IMSVTCAPCFG=<cap_type>,<value>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IMSVTCAPCFG?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^IMSVTCAPCFG=?
     *     [结果]: <CR><LF>^IMSVTCAPCFG: (list of supported <cap_type>s), (list of supported<value>s)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <cap_type>: 整型值，视频电话能力类型。
     *             0：视频电话动态开关(ChicagoC20不支持)（同时控制LTE和NR的开关）
     *             1：视频电话呼叫等待开关
     *     <value>: 整型值，能力开关。
     *             0：关
     *             1：开
     * [示例]:
     *     · 配置IMS视频电话支持呼叫等待
     *       AT^IMSVTCAPCFG=1,1
     *       OK
     *     · 配置IMS视频电话不支持呼叫等待
     *       AT^IMSVTCAPCFG=1,0
     *       OK
     *     · 配置打开IMS视频电话开关(Kirin960C20不支持)
     *       AT^IMSVTCAPCFG=0,1
     *       OK
     */
    { AT_CMD_IMSVTCAPCFG,
      AT_SetImsVtCapCfgPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSVTCAPCFG", (VOS_UINT8 *)"(0,1),(0,1)" },

    /*
     * [类别]: 协议AT-补充业务
     * [含义]: VOLTE语音到视频过程中取消视频
     * [说明]: 该命令用于VOLTE语音到视频过程中取消视频。
     * [语法]:
     *     [命令]: ^IMSVIDEOCALLCANCEL=<idx>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IMSVIDEOCALLCANCEL=?
     *     [结果]: <CR><LF>^IMSVIDEOCALLCANCEL: (1-7)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <idx>: 呼叫标识，与^clcc命令中idx保持一致  取值范围（1-7）
     * [示例]:
     *     · 测试命令
     *       AT^IMSVIDEOCALLCANCEL=?
     *       ^IMSVIDEOCALLCANCEL: (1-7)
     *       OK
     *     · VOLTE语音到视频过程中取消视频
     *       AT^IMSVIDEOCALLCANCEL=1
     *       OK
     */
    { AT_CMD_IMSVIDEOCALLCANCEL,
      AT_SetImsVideoCallCancelPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSVIDEOCALLCANCEL", (VOS_UINT8 *)"(1-7)" },
#endif

    { AT_CMD_CMMI,
      At_SetCmmiPara, AT_SET_SS_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE,
      (VOS_UINT8 *)"^CMMI", (VOS_UINT8 *)"(num)" },
};

/* 注册taf补充业务AT命令表 */
VOS_UINT32 AT_RegisterSsTafCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atSsTafCmdTbl, sizeof(g_atSsTafCmdTbl) / sizeof(g_atSsTafCmdTbl[0]));
}

