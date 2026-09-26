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
#include "at_test_as_cmd_tbl.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_as_set_cmd_proc.h"
#include "at_test_as_qry_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_AS_CMD_TBL_C

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
static const AT_ParCmdElement g_atTestAsCmdTbl[] = {
    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: 设置GSM
     * [说明]: 本功能只在GSM下有效。
     *         该命令用来指定开机频点、锁频点，查询GSM状态等特性。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^CGAS=0
     *     [结果]: <CR><LF><GAS STATUS><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CGAS=1
     *     [结果]: <CR><LF><cell1>:<Arfcn>,<Bsic>,<Rxlev>
     *             <CR><LF><cell2>:<Arfcn>,<Bsic>,<Rxlev>
     *             <CR><LF><cell3>:<Arfcn>,<Bsic>,<Rxlev>
     *             <CR><LF><cell4>:<Arfcn>,<Bsic>,<Rxlev>
     *             <CR><LF><cell5>:<Arfcn>,<Bsic>,<Rxlev>
     *             <CR><LF><cell6>:<Arfcn>,<Bsic>,<Rxlev>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CGAS=2
     *     [结果]: <CR><LF><Arfcn>,<Bsic>,<Rxlev><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^CGAS=5/6
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^CGAS=7,<Freq>
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^CGAS=8,<arfcn>,<band>,<bsic>,<barTimeLen>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF> ERROR<CR><LF>
     *     [命令]: ^CGAS=9,<arfcn>,<band>,<bsic>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF> ERROR <CR><LF>
     *     [命令]: ^CGAS=10
     *     [结果]: <CR><LF>OK<CR><LF>
     *     [命令]: ^CGAS=11,<enable>
     *     [结果]: <CR><LF> enable <CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况:
     *             <CR><LF> ERROR <CR><LF>
     * [参数]:
     *     <GAS STATUS>: 整型值，GSM内部状态。
     *             1：空闲态；
     *             2：专用态；
     *             3：传输态；
     *     <Arfcn>: 整型值，绝对频点号。
     *     <Bsic>: 十六进制值，BSIC号。
     *     <Rxlev>: 整型值，信号测量值。
     *     <Freq>: 十进制，带有频带标示的频点号。该数据为频带和频点之和。
     *             0x0000：850M频带，即GSM850；
     *             0x1000：900M频带，含EGSM900、PGSM900、RGSM900；
     *             0x2000：1800M频带，即GSM DCS；
     *             0x3000：1900M频带，即GSM PCS 1900。
     *     <band>: 整数值，频段值:
     *             0: GSM 850频段
     *             1: GSM 900 频段
     *             2: GSM 1800频段
     *             3: GSM 1900频段
     *     <barTimeLen>: 整数值，bar时长。
     *     <enable>: 整型值，取值0～1。
     *             0：在DSDS场景，另一个modem通知本modem退出游戏模式；
     *             1：在DSDS场景，另一个modem通知本modem进入游戏模式。
     *             其它值无效。
     * [示例]:
     *       查询GSM协议状态
     *     · 举例：GSM当前处于IDLE状态
     *       AT^CGAS=0
     *       1
     *       OK
     *       查询GSM邻区状态
     *     · 举例1：当前有两个邻区，频点分别为80、85，BSIC码为0x22，0x2A，测量电平值为-85，-90
     *       AT^CGAS=1
     *       1:80,0x22,-85
     *       2:85,0x2A,-90
     *       OK
     *     · 举例2：当前没有邻区
     *       AT^CGAS=1
     *       0
     *       OK
     *       若小区未同步上，其BSIC码显示为0xFF。
     *       查询GSM服务小区状态
     *     · 举例1：当前服务小区，频点分别为80，BSIC码为0x22，测量电平值为-85
     *       AT^CGAS=2
     *       80,0x22,-85
     *       OK
     *     · 锁定频点
     *       AT^CGAS=5
     *       OK
     *       该命令仅开机前设置有效，禁止主动重选/允许被动重选。
     *       举例1：当前为关机状态，输入AT^CGAS=5，开机驻留之后，禁止主动重选（该命令目前仅在开机之前使用有效）
     *     · 通知本modem进入游戏模式
     *       AT^CGAS=11,1
     *       1
     *       OK
     *     · 通知本modem退出游戏模式
     *       AT^CGAS=11,0
     *       0
     *       OK
     *       频点解锁
     *     · 举例1：当前已经锁定频点，主动重选被禁止，使用该命令解锁，以允许主动重选
     *       AT^CGAS=6
     *       OK
     *       指定开机频点
     *     · 举例1：指定MS开机优先驻留900M频段的114频点，用户输入的<Freq>应为频带0x1000与114频点的和4210。
     *       AT^CGAS=7,4210
     *       OK
     *     · 举例2：指定MS开机优先驻留1800M频段的521频点，用户输入的<Freq>应为频带0x2000与521频点的和8713。
     *       AT^CGAS=7,8713
     *       OK
     *       新增GSM FORBID小区
     *     · 举例1：新增GSM FORBID小区，小区的频点是80，频段是900频段，BSIC为0x22，小区加入后惩罚1000秒
     *       AT^CGAS=8,80,1,0x22,1000
     *       OK
     *       删除GSM FORBID小区
     *     · 举例1：删除GSM FORBID小区，小区的频点是80，频段是900频段，BSIC为0x22
     *       AT^CGAS=9,80,1,0x22
     *       OK
     *     · 删除G小区实体的历史系统消息，驻留G下时输入有效
     *       AT^CGAS=10
     *       OK
     */
    { AT_CMD_CGAS,
      At_SetCGasPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CGAS", (VOS_UINT8 *)"(0-65535),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295),(0-4294967295)" },

    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: 设置WAS参数
     * [说明]: 本功能只在WCDMA下有效。
     *         ^FREQLOCK/^WFREQLOCK/^CWAS不能同时使用。
     *         AT^CWAS=4/5和AT^CWAS=8/9不能同时使用。
     *         该命令用来查询，设置W相关参数。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^CWAS=<n>
     *     [结果]: <CR><LF>result<CR><LF><CR><LF>OK<CR><LF>
     *     [命令]: ^CWAS=<n>,<enable>,<freq>,<scrcode>
     *     [结果]: <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，取值0～9。
     *             0：打印当前小区的频点信息。出服务区或失步时，打印error。
     *             1：DCH状态下，打印所有活动集小区的扰码。
     *             非DCH状态下，打印当前驻留小区的扰码。
     *             2：当前协议栈所处的协议状态，能够检测当前是否处于失步状态，是否出服务区，result依次为0-DCH态、1-FACH态、2-PCH态、3-URA-PCH态、4-IDLE态，5-BUTT（除前面几种状态之外的状态：开机未驻留、丢网、未开机等）。
     *             3：Reserved，该功能目前不使用。
     *             4：锁频。
     *             5：锁频点和扰码。
     *             6：Reserved，该功能目前不使用。
     *             7：打印值依次为CS，PS，UTRAN的DrxCycLenCoeff，无效值为0。
     *             8：路测锁频功能，禁止异频、异系统重选、切换、重定向、CCO、强制异频切换到目标频点。
     *             9：路测锁频点锁扰码功能，禁止同频、异频、异系统重选、切换、重定向、CCO、强制同频、异频切换到目标小区。
     *             10：在DSDS场景，另一个modem通知本modem进入或者退出游戏模式；
     *             其他：无含义，返回error。
     *     <enable>: 整型值，取值0～1。
     *             0：解锁或者通知本modem退出游戏模式；
     *             1：加锁或者通知本modem进入游戏模式；
     *             其它值无效。
     *     <freq>: 整数值，表示当前锁定的频率值。
     *             enable=0时freq可以不填。
     *     <scrcode>: 整数值，表示当前锁定的扰码值。
     *             enable=0时scrcode可以不填。
     * [示例]:
     *     · 查询当前小区扰码
     *       AT^CWAS=1
     *       48
     *       OK
     *     · 查询当前RRC状态
     *       AT^CWAS=2
     *       4
     *       OK
     *       返回值4表示IDLE状态下
     *     · 通知本modem进入游戏模式
     *       AT^CWAS=10,1
     *       1
     *       OK
     *     · 通知本modem退出游戏模式
     *       AT^CWAS=10,0
     *       0
     *       OK
     */
    { AT_CMD_CWAS,
      At_SetCwasPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CWAS", (VOS_UINT8 *)"(0-255),(0-65535),(0-65535),(0-65535)" },

    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: 查询WCDMA模式DCH下最多10次测量报告
     * [说明]: 该命令用来查询DCH下最多10次测量报告。
     *         上报的事件为1系列，2系列，3系列，其他事件不上报，针对1系列事件，输出小区扰码。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     *         本功能只在WCDMA下有效。
     * [语法]:
     *     [命令]: ^MEANRPT
     *     [结果]: <CR><LF>cellinfo<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     * [表]: Event Number与协议中Event ID的对应关系
     *       Event Number, 编号含义（请参考协议25331的Event ID）,
     *       0x0,          1A,
     *       0x1,          1B,
     *       0x2,          1C,
     *       0x3,          1D,
     *       0x4,          1E,
     *       0x5,          1F,
     *       0x100,        2A,
     *       0x101,        2B,
     *       0x102,        2C,
     *       0x103,        2D,
     *       0x105,        2F,
     *       0x200,        3A,
     * [示例]:
     *     · 查询DCH下最多10次测量报告
     *       AT^MEANRPT
     *       2
     *       0x103,0
     *       0x103,0
     *       OK
     */
    { AT_CMD_MEANRPT,
      At_GetMeanRpt, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^MEANRPT", VOS_NULL_PTR },

#if (FEATURE_UE_MODE_NR == FEATURE_ON)
    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: NR的锁频锁小区功能
     * [说明]: 该命令用于查询、设置、取消NR的锁频功能。
     *         本命令仅在单板支持NR时才能使用（即当前单板支持NR能力）。
     *         ENDC场景不支持NR锁频参数。
     * [语法]:
     *     [命令]: ^NRFREQLOCK=<enable>[,<scstype>,<band>[,<arfcn>[,<cellid>]]]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             有MT 相关错误时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^NRFREQLOCK?
     *     [结果]: <CR><LF>^NRFREQLOCK: <enable> [,<scstype>,<band>,<freq>,<cellid>]<CR><LF> <CR><LF>OK<CR><LF>
     *     [命令]: ^NRFREQLOCK=?
     *     [结果]: <CR><LF>^NRFREQLOCK: (list of supported <enable>s),(list of supported <scstype>s),(list of supported <band>s),(list of supported <arfcn>s),(list of supported <cellid>s)<CR><LF> <CR><LF>OK<CR><LF>
     * [参数]:
     *     <enable>: 整型值，标识锁频功能是否启用：
     *             0：关闭锁频功能，下发关闭锁频功能时不需要再带其他参数；
     *             1：启用锁定频点功能；
     *             2：启用锁定小区功能；
     *             3：启用锁定Band功能。
     *             注意：如果当前已经处于enable状态，更改锁频类型，需要通过先下发0清除原有模式后，再次下发锁频锁小区请求。
     *     <scstype>: 频点的scstype信息，3种锁都必须设置该参数。
     *             整数值，有效值如下：
     *             0：SCS_TYPE_COMM_15；
     *             1：SCS_TYPE_COMM_30；
     *             2：SCS_TYPE_COMM_60；
     *             3：SCS_TYPE_COMM_120；
     *             4：SCS_TYPE_COMM_240。
     *     <band>: 整数值，锁频的频段，用来区分频点重复的频段；该Band需在UE能力支持的范围；3种锁都必须设置该参数，取值范围0～65535。
     *     <arfcn>: 整数值，当前锁定的频点，取值范围0～4294967295。
     *             当<enable>值为1或者2时必须设置该参数，值为3时不能设置该参数，该频点设置为小区的频点信息或者需要锁定的频点信息，并且处在Band支持的范围之内。
     *     <cellid>: 需要锁定的小区ID，<enable>值为2时必须设置该参数，值为1或3时不能设置该参数，整型值，取值范围0~1007；
     * [示例]:
     *     · 锁定NR小区Cell ID为374
     *       AT^NRFREQLOCK=2,1,77,633984,374
     *       OK
     *     · 锁定NR band为77
     *       AT^NRFREQLOCK=3,1,77
     *       OK
     *     · 锁定NR 频点为633984
     *       AT^NRFREQLOCK=1,1,77,633984
     *       OK
     *     · 关闭锁频功能
     *       AT^NRFREQLOCK=0
     *       OK
     *     · 执行测试命令
     *       AT^NRFREQLOCK=?
     *       ^NRFREQLOCK: (0-3),(0-4),(0-65535),(0-4294967295),(0-1007)
     *       OK
     */
    { AT_CMD_NRFREQLOCK,
      At_SetNrFreqLockPara, AT_SET_PARA_TIME, AT_QryNrFreqLockPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^NRFREQLOCK", (VOS_UINT8 *)"(0-3),(0-4),(0-65535),(0-4294967295),(0-1007)" },
#endif
};

/* 注册AS组件测试相关AT命令表 */
VOS_UINT32 AT_RegisterTestAsCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atTestAsCmdTbl, sizeof(g_atTestAsCmdTbl) / sizeof(g_atTestAsCmdTbl[0]));
}
#endif
