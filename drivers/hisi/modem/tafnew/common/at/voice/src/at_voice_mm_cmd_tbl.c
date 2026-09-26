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
#include "at_voice_mm_cmd_tbl.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_voice_mm_set_cmd_proc.h"
#include "at_voice_mm_qry_cmd_proc.h"
#include "at_test_para_cmd.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_VOICE_MM_CMD_TBL_C

static const AT_ParCmdElement g_atVoiceMmCmdTbl[] = {
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 退出紧急呼回呼模式
     * [说明]: QCCB:QUIT CDMA CALL BACK，该命令用于退出紧急呼回呼模式，若当前处于回呼模式中，返回退出成功。
     * [语法]:
     *     [命令]: ^QCCB
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     * [参数]:
     * [示例]:
     *     · 退出CALLBACK
     *       AT^QCCB
     *       OK
     */
    { AT_CMD_QCCB,
      AT_SetQuitCallBack, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^QCCB", VOS_NULL_PTR },

#if (FEATURE_LTE == FEATURE_ON)
    /*
     * [类别]: 协议AT-电路域业务
     * [含义]: 设置UE的操作模式
     * [说明]: 本命令用于设置UE的操作模式。
     *         注意：该命令的设置和查询仅在驻留LTE上生效，其他情况均返回ERROR。
     * [语法]:
     *     [命令]: +CEMODE=<mode>
     *     [结果]: <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err_code><CR><LF>
     *     [命令]: +CEMODE?
     *     [结果]: <CR><LF>+ CEMODE: <mode><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <mode>: 整型值，UE操作模式。
     *             0：PS域服务，设定在数据中心
     *             1：CS+PS域服务，设定在语音中心
     *             2：CS+PS域服务，设定在数据中心
     *             3：PS域服务，设定在语音中心
     * [示例]:
     *     · 设置UE在PS域服务，数据业务
     *       AT+CEMODE=0
     *       OK
     *     · 查询UE操作模式
     *       AT+CEMODE?
     *       +CEMODE: 0
     *       OK
     */
    { AT_CMD_CEMODE,
      AT_SetCemodePara, AT_SET_PARA_TIME, At_QryCemode, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"+CEMODE", (VOS_UINT8 *)"(0-3)" },
#endif
};

/* 注册taf呼叫AT命令表 */
VOS_UINT32 AT_RegisterVoiceMmCmdTbl(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atVoiceMmCmdTbl, sizeof(g_atVoiceMmCmdTbl) / sizeof(g_atVoiceMmCmdTbl[0]));
}

