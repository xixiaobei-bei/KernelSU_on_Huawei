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
#include "at_ltev_mm_cmd_tbl.h"
#include "AtParse.h"

#include "at_cmd_proc.h"
#include "at_parse_core.h"
#include "at_test_para_cmd.h"

#include "at_ltev_mm_set_cmd_proc.h"
#include "at_ltev_mm_qry_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_LTEV_MM_CMD_TBL_C

#if (FEATURE_LTEV == FEATURE_ON)
static const AT_ParCmdElement g_atLtevMmCmdTbl[] = {
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-LTE-V相关
     * [含义]: 设置和查询协议栈SrourceID命令
     * [说明]: 本命令用于设置和查询协议栈SrourceId。
     * [语法]:
     *     [命令]: ^SRCID=<Effective>,<SrcID>
     *     [结果]: 执行成功时：
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^SRCID?
     *     [结果]: 执行成功时：
     *             CR><LF>^SRCID: <SrcID>,<SrcIDOrign><CR>
     *             <CR><LF>OK<CR><LF>
     *             执行失败时：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     *     <Effective>: 0:失效，1：生效
     *     <SrcID>: SourceID值，0-0xffffff, 0代表无效值，不能设置。
     *     <SrcIDOrign>: SourceID来源：
     *             0代表source id来源于app
     *             1代表source id来源于at
     *             2代表source id来源于vnas
     * [示例]:
     *     · 设置SourceId。
     *       AT^SRCID=1,1
     *       OK
     *     · 查询SourceId。
     *       AT^SRCID?
     *       ^SRCID: 1,1
     *       OK
     */
    { AT_CMD_SRCID,
      AT_SetSourceId, AT_SET_PARA_TIME, AT_QrySourceId, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^SRCID", (VOS_UINT8 *)"(0-1),(1-16777215)" },
#endif
};

/* 注册MM LTEV AT命令表 */
VOS_UINT32 AT_RegisterLtevMmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atLtevMmCmdTbl, sizeof(g_atLtevMmCmdTbl) / sizeof(g_atLtevMmCmdTbl[0]));
}
#endif

