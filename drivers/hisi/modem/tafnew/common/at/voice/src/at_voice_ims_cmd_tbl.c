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
#include "at_voice_ims_cmd_tbl.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_voice_ims_set_cmd_proc.h"
#include "at_voice_ims_qry_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_IMS_CMD_TBL_C

static const AT_ParCmdElement g_atVoiceImsCmdTbl[] = {
#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 配置IMS注册域的优先级
     * [说明]: 配置IMS注册域的优先级：IMS域按照对应的优先级顺序发起注册流程。
     * [语法]:
     *     [命令]: ^IMSDOMAINCFG=<domain_cfg>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IMSDOMAINCFG?
     *     [结果]: <CR><LF>^IMSDOMAINCFG: <domain_cfg><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^IMSDOMAINCFG=?
     *     [结果]: <CR><LF>^IMSDOMAINCFG: (list of supported <domain_cfg>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <domain_cfg>: 整型值，IMS注册域的优先级：
     *             0：WIFI优先，注册顺序为WIFI->LTE/NR->GU；
     *             1：LTE/NR优先，注册顺序为LTE/NR->WIFI->GU；
     *             2：仅WIFI，仅在WIFI发起注册流程；
     *             3：WIFI低优先级，Modem无服务时选择WIFI发起注册流程。
     *             4：无效值，仅当AP没有调用^IMSDOMAINCFG命令设置IMS注册优先域时，查询^IMSDOMAINCFG，此时会返回该值；其他场景该值无效（即IMS注册优先域不能设置为4）；在AP没有调用^IMSDOMAINCFG命令设置IMS注册优先域时，IMS仅在Cellular上尝试注册；
     * [示例]:
     *     · 配置IMS注册域的优先级为WIFI 优先
     *       AT^IMSDOMAINCFG=0
     *       OK
     *     · 查询IMS注册域的优先级
     *       AT^IMSDOMAINCFG?
     *       ^IMSDOMAINCFG: 0
     *       OK
     *     · 测试IMS注册域的优先级
     *       AT^IMSDOMAINCFG=?
     *       ^IMSDOMAINCFG: (0-3)
     *       OK
     */
    { AT_CMD_IMSDOMAINCFG,
      AT_SetImsDomainCfgPara, AT_SET_PARA_TIME, AT_QryImsDomainCfgPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSDOMAINCFG", (VOS_UINT8 *)"(0-3)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询当前IMS注册域
     * [说明]: 查询当前IMS注册域。
     * [语法]:
     *     [命令]: ^IMSREGDOMAIN?
     *     [结果]: <CR><LF>^IMSREGDOMAIN: <domain><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <domain>: 整型值，IMS注册域
     *             0： LTE
     *             1： WIFI
     *             2： Utran
     *             3： Gsm
     *             4： NR
     *             255： UNKNOWN
     * [示例]:
     *     · 查询当前IMS注册域
     *       AT^IMSREGDOMAIN?
     *       ^IMSREGDOMAIN: 0
     *       OK
     */
    { AT_CMD_IMSREGDOMAIN,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryImsRegDomainPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSREGDOMAIN", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 转发MAPCON给IMSA的消息
     * [说明]: 转发MAPCON给IMSA的消息。
     * [语法]:
     *     [命令]: ^IMSCTRLMSG=<msg_id>,<msg_len>,<msg_context>[,<msg_context>,…]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^IMSCTRLMSG=?
     *     [结果]: <CR><LF>^IMSCTRLMSG: (list of supported <msg_id>s) (list of supported < msg_len >s)<msg><msg><msg><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <msg_id>: 对应接收模块
     *             0：IMSA
     *             1：MAPCONN
     *     <msg_len>: 消息内容<msg_context>的长度
     *     <msg_context>: 模块间接口消息码流
     * [示例]:
     *     · 转发MAPCON给IMSA的消息
     *       AT^IMSCTRLMSG=0,4,"12345678"
     *       OK
     *     · 测试本命令
     *       AT^IMSCTRLMSG=?
     *       ^IMSCTRLMSG: (0,1),(1-600),(msg),(msg),(msg)
     *       OK
     */
    { AT_CMD_IMSCTRLMSG,
      AT_SetImsctrlmsgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^IMSCTRLMSG", (VOS_UINT8 *)"(0,1),(1-600),(msg),(msg),(msg)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 查询IMS域的虚拟号码
     * [说明]: 该命令用于查询IMS域的IMPU信息以及虚拟号码。
     * [语法]:
     *     [命令]: ^VOLTEIMPU
     *     [结果]: <CR><LF>^VOLTEIMPU: < IMPU>[,<Virtual_Number >]<CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^VOLTEIMPU?
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^VOLTEIMPU=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <IMPU>: 字符串类型，IMPU信息，取值范围0-128bytes
     *     <Virtual_Number>: 字符串类型，网络分配的虚拟电话号码，最大长度为127 bytes
     * [示例]:
     *     · 网络没有分配虚拟电话号码
     *       AT^VOLTEIMPU
     *       ^VOLTEIMPU: sip:01022331621@lte-lguplus.co.kr
     *       OK
     *     · 网络分配虚拟电话号码01023151463
     *       AT^VOLTEIMPU
     *       ^VOLTEIMPU: sip:01022331621@lte-lguplus.co.kr,01023151463
     *       OK
     */
    { AT_CMD_VOLTEIMPU,
      AT_SetVolteimpuPara, AT_SET_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^VOLTEIMPU", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置和查询IMS的昵称信息
     * [说明]: 该命令用于设置和查询IMS域的昵称信息。
     * [语法]:
     *     [命令]: ^NICKNAME=<Nick_Name>
     *     [结果]: 设置正确：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NICKNAME?
     *     [结果]: 查询成功：
     *             <CR><LF>^NICKNAME: <Nick_Name><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NICKNAME=?
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Nick_Name>: UTF8编码格式，最大长度为256个字符
     * [示例]:
     *       设置昵称：中国
     *       “中”对应的UTF8编码为E4 B8 AD
     *     · “国”对应的UTF8编码为E5 9B BD
     *       AT^NICKNAME=E4B8ADE59BBD
     *       OK
     *     · 查询昵称结果为：中国
     *       AT^NICKNAME?
     *       ^NICKNAME: E4B8ADE59BBD
     *       OK
     *     · 测试命令
     *       AT^NICKNAME=?
     *       OK
     */
    { AT_CMD_NICKNAME,
      AT_SetNickNamePara, AT_SET_PARA_TIME, AT_QryNickNamePara, AT_QRY_PARA_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NICKNAME", (VOS_UINT8 *)"(@nickname)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 配置RTT功能开关
     * [说明]: 该命令用于将用户配置的RTT功能开关配置给IMS，用于RTT功能开启时自动拒接视频来电。
     * [语法]:
     *     [命令]: ^RTTCFG=<enable>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^RTTCFG=?
     *     [结果]: <CR><LF>^RTTCFG: (list of supported <enable>s)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，用户配置RTT功能开关：
     *             0：RTT功能关闭；
     *             1：RTT功能开启。
     * [示例]:
     *     · 配置打开RTT功能开关
     *       AT^RTTCFG=1
     *       OK
     *     · 测试命令
     *       AT^RTTCFG=?
     *       ^RTTCFG: (0,1)
     *       OK
     */
    { AT_CMD_RTTCFG,
      AT_SetRttCfgPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^RTTCFG", (VOS_UINT8 *)"(0,1)" },

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 切换RTT通话
     * [说明]: 该命令用于本端主动发起RTT通话切换请求或应答/拒绝远端发起的RTT通话切换请求。
     * [语法]:
     *     [命令]: ^RTTMODIFY=<call_id>,<operation>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^RTTMODIFY=?
     *     [结果]: <CR><LF>^RTTMODIFY: (list of supported <call_id>s),( list of supported <operation>s)<CR><LF>  <CR><LF>OK<CR><LF>
     * [参数]:
     *     <call_id>: 整型值，呼叫标识，取值范围1~7。
     *     <operation>: 整型值，RTT通话切换操作类型：
     *             0：Add Text流；
     *             1：Close Text流；
     *             2：同意Add Text流；
     *             3：拒绝Add Text流（暂不支持）。
     * [示例]:
     *     · 本端发起升级RTT通话请求
     *       AT^RTTMODIFY=1,0
     *       OK
     *     · 测试命令
     *       AT^RTTMODIFY=?
     *       ^RTTMODIFY: (1-7),(0-3)
     *       OK
     */
    { AT_CMD_RTTMODIFY,
      AT_SetRttModifyPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^RTTMODIFY", (VOS_UINT8 *)"(1-7),(0-3)" },
#endif

    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 配置IMS协议栈能力
     * [说明]: 该命令用于配置IMS协议栈IP侧能力开关。
     *         目前命令支持IPSEC和KeepAlive能力配置。
     * [语法]:
     *     [命令]: ^IMSIPCAPCFG=[<IPSec>],[<KeepAlive>]
     *     [结果]: 正确设置时：
     *             <CR><LF>OK<CR><LF>
     *             错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^IMSIPCAPCFG?
     *     [结果]: <CR><LF>^IMSIPCAPCFG:  <IPSec>,<KeepAlive>  <CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^IMSIPCAPCFG=?
     *     [结果]: <CR><LF>^IMSIPCAPCFG:  (list of supported <IPSec>s),(list of supported <KeepAlive>s)<CR><LF>  <CR><LF>OK<CR><LF>
     * [参数]:
     *     <IPSec>: 整型值，IPSEC能力配置：
     *             0：开启IPSEC能力；
     *             1：关闭IPSEC能力；
     *             如果参数空缺，将保持原来的能力配置。
     *     <KeepAlive>: 整型值，KeepAlive能力配置：
     *             0：关闭KeepAlive能力；
     *             1：开启KeepAlive能力；
     *             如果参数空缺，将保持原来的能力配置。
     * [示例]:
     *     · 配置IMS协议栈能力，开启IPSEC能力，关闭KeepAlive能力
     *       AT^IMSIPCAPCFG=0,0
     *       OK
     *     · 查询IMS协议栈能力配置
     *       AT^IMSIPCAPCFG?
     *       ^IMSIPCAPCFG: 0,0
     *       OK
     *     · 测试命令
     *       AT^IMSIPCAPCFG=?
     *       ^IMSIPCAPCFG: (0,1),(0,1)
     *       OK
     */
    { AT_CMD_IMSIPCAPCFG,
      AT_SetImsIpCapPara, AT_SET_PARA_TIME, AT_QryImsIpCapPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMSIPCAPCFG", (VOS_UINT8 *)"(0,1),(0,1)" },
};

/* 注册taf呼叫AT命令表 */
VOS_UINT32 AT_RegisterVoiceImsCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atVoiceImsCmdTbl, sizeof(g_atVoiceImsCmdTbl) / sizeof(g_atVoiceImsCmdTbl[0]));
}

