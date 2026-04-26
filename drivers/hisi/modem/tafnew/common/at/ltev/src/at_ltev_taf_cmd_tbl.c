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
#include "at_ltev_taf_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_ltev_taf_set_cmd_proc.h"
#include "at_ltev_taf_qry_cmd_proc.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_TAF_CMD_TBL_C

#if (FEATURE_LTEV == FEATURE_ON)
static const AT_ParCmdElement g_atLtevTafCmdTbl[] = {

    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: LTE-V开关机
     * [说明]: 用于设置，查询，测试VMODE。
     *         LTE-V作为车载特性，只有车载产品使用，CPE、E5、CE、Phone等其它产品都不会使用。
     * [语法]:
     *     [命令]: ^VMODE=<rat>,<mode>
     *     [结果]: 执行正确是
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^VMODE?
     *     [结果]: <CR><LF>^VMODE: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     *     [命令]: ^VMODE=?
     *     [结果]: <CR><LF>^VMODE: (list of supported <rat>s) (list of supported <mode>s)CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <rat>: 接入制式：
     *             0：LTE；
     *             其他取值暂不支持。
     *     <mode>: 模式：
     *             0：关机；
     *             1：开机。
     * [示例]:
     *     · 设置命令
     *       AT^VMODE=0,1
     *       OK
     *     · 开机状态下，查询命令
     *       AT^VMODE?
     *       ^VMODE: 1
     *       OK
     *     · 测试命令
     *       AT^VMODE=?
     *       ^VMODE: (0),(0,1)
     *       OK
     */
    { AT_CMD_VMODE,
      At_SetVModePara, AT_SET_PARA_TIME, At_QryVModePara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_TEST_PARA_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^VMODE", (VOS_UINT8 *)"(0),(0,1)" },
};

/* 注册TAF LTEV AT命令表 */
VOS_UINT32 AT_RegisterLtevTafCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atLtevTafCmdTbl, sizeof(g_atLtevTafCmdTbl) / sizeof(g_atLtevTafCmdTbl[0]));
}
#endif

