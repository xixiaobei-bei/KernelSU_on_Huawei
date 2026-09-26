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

#include "at_data_basic_cmd_tbl.h"
#include "at_data_basic_set_cmd_proc.h"
#include "at_data_basic_qry_cmd_proc.h"

#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_BASIC_CMD_TBL_C

#if (FEATURE_AT_HSUART == FEATURE_ON)
static const AT_ParCmdElement g_atDataBasicCmdTbl[] = {
    /*
     * [类别]: 装备AT-GUC装备
     * [含义]: 高速UART口切换成Shell口
     * [说明]: 该命令用于将高速UART口（HSUART口）切换为C shell口或者A shell口。在非HSUART端口输入该命令，返回ERROR。
     *         这个命令受 datalock锁定（使用^spword命令解锁），只有在解锁datalock后，才能使用这个命令，否则返回ERROR。
     *         命令设置以后，第一次非下电重启单板生效，第二次重启端口恢复为AT口。
     *         只用于MBB模块产品。
     * [语法]:
     *     [命令]: AT^UARTSWITCH=[value]
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <value>: 整型值。
     *             0：将当前HSUART口切换成A核shell口；
     *             1：将当前HSUART口切换成C核shell口。
     * [示例]:
     *     · 当前HSUART口切换成A核shell口
     *       AT^UARTSWITCH=0
     *       OK
     */
    { AT_CMD_SWITCH_UART,
      AT_SetSwitchUart, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^UARTSWITCH", (VOS_UINT8 *)"(0,1)" },
};

/* 注册基础数据业务AT命令表 */
VOS_UINT32 AT_RegisterDataBasicCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDataBasicCmdTbl, sizeof(g_atDataBasicCmdTbl) / sizeof(g_atDataBasicCmdTbl[0]));
}
#endif

