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
#include "at_ltev_ttf_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_ltev_ttf_set_cmd_proc.h"
#include "at_ltev_ttf_qry_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_TTF_CMD_TBL_C

#if (FEATURE_LTEV == FEATURE_ON)
static const AT_ParCmdElement g_atLtevTtfCmdTbl[] = {
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 清除和查询V2X接收和发送的数据包情况
     * [说明]: 本命令用于清除和查询V2X接收和发送的数据包情况。
     * [语法]:
     *     [命令]: ^PTRRPT=<op>
     *     [结果]: 执行成功时：
     *             <CR><LF>^PTRRPT: <tx_bytes>,<rx_bytes>,<tx_packets>,<rx_packets>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^PTRRPT?
     *     [结果]: 执行成功时：
     *             <CR><LF>^PTRRPT: <tx_bytes>,<rx_bytes>,<tx_packets>,<rx_packets>
     *             <CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <tx_bytes>: PC5口发送的总字节数
     *     <rx_bytes>: PC5口接收的总字节数
     *     <tx_packets>: PC5口发送的总包数
     *     <rx_packets>: PC5口接收的总包数
     *     <op>: 1：收发数据包清零。
     * [示例]:
     *     · 清除命令
     *       AT^PTRRPT=1
     *       ^PHYR: 0,0,0,0
     *       OK
     *     · 查询命令
     *       AT^PTRRPT?
     *       ^PHYR: 1024,2048,900,800
     *       OK
     */
    { AT_CMD_PTRRPT,
      AT_SetPtrRpt, AT_SET_PARA_TIME, AT_QryPtrRpt, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^PTRRPT", (VOS_UINT8 *)"(1)" },

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 查询LTE-V PC5口RSSI均值命令
     * [说明]: 本命令用于LTE-V PC5口RSSI均值查询。
     * [语法]:
     *     [命令]: ^VRSSI?
     *     [结果]: 执行成功时：
     *             CR><LF>^VRSSI: <RSSI><CR>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <RSSI>: PC5口RSSI均值
     * [示例]:
     *     · 查询PC5 RSSI均值。
     *       AT^VRSSI?
     *       ^VRSSI: -80
     *       OK
     */
    { AT_CMD_VRSSI,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryVRssi, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VRSSI", VOS_NULL_PTR },

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 装备协议栈模拟发送PC5口数据包命令
     * [说明]: 本命令用于装备协议栈模拟发送PC5口数据包。
     *         注意：该命令在测试模式下才能使用。
     * [语法]:
     *     [命令]: ^SENDDATA=<SaRbStart>,<DaRbNum>,<Mcs>,<Period>,<SendFrmMask>,<Flag>,[<ResRsvPeriod>],[<FrameOffset>],[<PktPriority>],[<TimeGap>],[<RandomFlag>]
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <SaRbStart>: SA起始RB，取值范围：0~99，当前只支持SA/DA连续分配的方式，默认起始RB为0
     *     <DaRbNum>: DA RB数，取值范围：0~99，RB数分配需要符合（2/3/5非负整数倍）的准则
     *     <Mcs>: MCS取值范围：0~31，当前R14版本只支持1-17阶
     *     <Period>: 发包周期，取值范围：10-1000
     *     <SendFrmMask>: 发送子帧位图，取值范围：0~1023(0x3FF)（二进制10个1，代表10个子帧）
     *     <Flag>: 测试模式使能标志，0：不使能，1：使能固定时频发包，2：使能固定周期发包
     *     <ResRsvPeriod>: MAC资源预留周期，取值范围：0,20,50,100,200,300,400,500,600,700,800,900,1000
     *     <FrameOffset>: 数据发送帧偏移，取值范围：0-1024
     *     <PktPriority>: 报文发送优先级，取值范围：1-8
     *     <TimeGap>: harq重传间隔，0表示不支持重传，取值范围：0-15
     *     <RandomFlag>: 是否是发送随机数标记，取值：0-1
     * [示例]:
     *     · 设置命令，不设置可选参数。
     *       AT^SENDDATA=0,8,12,100,768,0
     *       OK
     *     · 设置命令，设置可选参数。
     *       AT^SENDDATA=0,8,12,100,768,0,100,0,2,0,0
     *       OK
     */
    { AT_CMD_SENDDATA,
      AT_SendData, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SENDDATA", (VOS_UINT8 *)"(0-99),(0-99),(0-17),(10-1000),(0-1023),(0,1,2),\
(0,20,50,100,200,300,400,500,600,700,800,900,1000),(0-1024),(1-8),(0-15),(0,1)" },
#endif
};

/* 注册TTF LTEV AT命令表 */
VOS_UINT32 AT_RegisterLtevTtfCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atLtevTtfCmdTbl, sizeof(g_atLtevTtfCmdTbl) / sizeof(g_atLtevTtfCmdTbl[0]));
}
#endif


