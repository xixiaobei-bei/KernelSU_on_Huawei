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
#include "at_test_ttf_cmd_tbl.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_ttf_qry_cmd_proc.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_TEST_TTF_CMD_TBL_C

#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
static const AT_ParCmdElement g_atTestTtfCmdTbl[] = {
    { AT_CMD_ACPUMEMINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryAcpuMemInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^ACPUMEMINFO", VOS_NULL_PTR },

    /*
     * [类别]: 协议AT-测试类私有
     * [含义]: TTF_NODE内存泄漏检查
     * [说明]: 该命令用于查询TTF_NODE内存使用情况，判断是否有内存泄漏。通过AT命令“AT^CCPUMEMINFO?”查询，返回结果全0为无泄漏，非0为有泄漏。
     *         备注：此命令下发时需要将当前所有的MODEM都通过A+CFUN=0关机后下发，否则查询结果不正确。
     *         此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^CCPUMEMINFO?
     *     [结果]: <CR><LF>^CCPUMEMINFO: <TTF_MEM Ret>,<TTF_NODE Ret>,<RRM_NODE Ret><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <TTF_MEM Ret>: TTF_MEM的检查结果，返回结果全0为无泄漏，非0为有泄漏。Bit位标识是那个内存池泄漏，0x2标识DL_PDU，0x4标识UL_DATA，0x10标识UL_IP_DATA
     *     <TTF_NODE Ret>: TTF_NODE的检查结果，返回结果全0为无泄漏，非0为有泄漏。
     *     <RRM_NODE Ret>: 不再使用，NODE合并，结果体现在TTF_NODE结果中。恒为0。
     * [示例]:
     *       执行查询命令
     *     · 正常结果：
     *       AT^CCPUMEMINFO?
     *       ^CCPUMEMINFO: 0,0,0
     *       OK
     *     · 异常结果：
     *       AT^CCPUMEMINFO?
     *       ^CCPUMEMINFO: 4,0,0
     *       OK
     */
    { AT_CMD_CCPUMEMINFO,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCcpuMemInfoPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^CCPUMEMINFO", VOS_NULL_PTR },
};

/* 注册TTF组件测试相关AT命令表 */
VOS_UINT32 AT_RegisterTestTtfCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atTestTtfCmdTbl, sizeof(g_atTestTtfCmdTbl) / sizeof(g_atTestTtfCmdTbl[0]));
}
#endif

