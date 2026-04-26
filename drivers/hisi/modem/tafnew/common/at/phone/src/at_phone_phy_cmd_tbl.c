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
#include "at_phone_phy_cmd_tbl.h"
#include "AtParse.h"
#include "at_parse_cmd.h"
#include "at_phone_phy_set_cmd_proc.h"
#include "at_device_cmd.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PHONE_PHY_CMD_TBL_C

static const AT_ParCmdElement g_atPhonePhyCmdTbl[] = {
    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: DCXO样本操作测试命令
     * [说明]: 对现有样本点进行全样本备份、恢复、删除、平移、时间偏移操作。
     *         该命令需在软关机状态下生效。
     *         只允许主Modem发送AT命令。
     * [语法]:
     *     [命令]: ^SAMPLE=<Type>,<TempRange>,<PpmOffset>,<TimeOffset>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <Type>: 整型值，样本类型：
     *             0：备份当前样本；
     *             1：恢复备份样本；
     *             2：删除本地样本；
     *             3：删除备份样本；
     *             4：平移样本（包括平移样本和模拟老化）；
     *     <TempRange>: 整型值，温区范围：
     *             1: 极低温;
     *             2: 低温;
     *             3: 低温、极低温;
     *             4: 常温;
     *             5: 常温、极低温;
     *             6: 常温、低温;
     *             7: 常温、低温、极低温;
     *             8: 高温;
     *             9: 高温、极低温;
     *             10: 高温、低温;
     *             11: 高温、低温、极低温;
     *             12: 高温、常温;
     *             13: 高温、常温、极低温;
     *             14: 高温、常温、低温;
     *             15: 高温、常温、低温、极低温;
     *             16: 极高温;
     *             17: 极高温、极低温;
     *             18: 极高温、低温;
     *             19: 极高温、低温、极低温;
     *             20: 极高温、常温;
     *             21: 极高温、常温、极低温;
     *             22: 极高温、常温、低温;
     *             23: 极高温、常温、低温、极低温;
     *             24: 极高温、高温;
     *             25: 极高温、高温、极低温;
     *             26: 极高温、高温、低温;
     *             27: 极高温、高温、低温、极低温;
     *             28: 极高温、高温、常温;
     *             29: 极高温、高温、常温、极低温;
     *             30: 极高温、高温、常温、低温;
     *             31: 极高温、高温、常温、低温、极低温;
     *     <PpmOffset>: 整型值，样本偏移量（单位0.1ppm）,默认为0。
     *     <TimeOffset>: 整型值，样本偏移时间量（单位：月），默认为0。该参数为0或负数
     * [示例]:
     *     · 全温区备份
     *       AT^SAMPLE=0
     *       OK
     *     · 删除备份样本
     *       AT^SAMPLE=3
     *       OK
     *     · 要对高温和极低温的样本拉偏并加老化
     *       AT^SAMPLE=4,9,3,-6
     *       OK
     *       代表对高温区和极低温区样本点进行6个月后老化并向上拉偏0.3ppm。
     *     · 删除当前样本
     *       AT^SAMPLE=2,0
     *       OK
     */
    { AT_CMD_SAMPLE,
      AT_SetSamplePara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, At_CmdTestProcOK, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (TAF_UINT8 *)"^SAMPLE", (VOS_UINT8 *)"(0-255),(0-255)" },
};

/* 注册phone phy AT命令表 */
VOS_UINT32 AT_RegisterPhonePhyCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atPhonePhyCmdTbl, sizeof(g_atPhonePhyCmdTbl) / sizeof(g_atPhonePhyCmdTbl[0]));
}

