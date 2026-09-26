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
#include "at_custom_hifi_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_custom_hifi_set_cmd_proc.h"
#include "at_custom_hifi_qry_cmd_proc.h"


#define THIS_FILE_ID PS_FILE_ID_AT_CUSTOM_HIFI_CMD_TBL_C

static const AT_ParCmdElement g_atCustomHifiCmdTbl[] = {
    /*
     * [类别]: 协议AT-与AP对接
     * [含义]: 设置双通语音时HIFI切换
     * [说明]: 该命令用于两路电话并发过程中，控制语音通路的切换。
     *         前台模式，即上下行语音输入输出正常（后续文中退出后台模式和设置成前台模式或激活某路电话意义相同）。
     *         后台模式，即上行进入静音状态，下行的语音输出通路断开（后续文中设置后台模式和去激活某路电话表示的是同样的意思）。
     *         命令使用场景：双卡通话切换时AP侧下发。
     * [语法]:
     *     [命令]: ^CBG=<n>
     *     [结果]: 设置成功：
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^CBG?
     *     [结果]: 查询成功：
     *             <CR><LF>^CBG: <n><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^CBG=?
     *     [结果]: <CR><LF>^CBG: (0,1)<CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <n>: 整型值，语音前后台开关。
     *             0：前台模式；
     *             1：后台模式。
     * [示例]:
     *     · 设置前台模式
     *       AT^CBG=0
     *       OK
     *     · 查询前后台模式
     *       AT^CBG?
     *       ^CBG: 0
     *       OK
     *     · 测试命令
     *       AT^CBG=?
     *       ^CBG: (0,1)
     *       OK
     */
    { AT_CMD_CBG,
      AT_SetCbgPara, AT_SET_VC_PARA_TIME, AT_QryCbgPara, AT_QRY_VC_PARA_TIME, AT_TestCbgPara, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CBG", (VOS_UINT8 *)"(0,1)" },
};

/* 注册HIFI定制AT命令表 */
VOS_UINT32 AT_RegisterCustomHifiCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atCustomHifiCmdTbl, sizeof(g_atCustomHifiCmdTbl) / sizeof(g_atCustomHifiCmdTbl[0]));
}

