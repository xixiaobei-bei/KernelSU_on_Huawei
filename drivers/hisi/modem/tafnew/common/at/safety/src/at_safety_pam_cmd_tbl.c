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

#include "at_safety_pam_cmd_tbl.h"
#include "at_safety_pam_qry_cmd_proc.h"
#include "at_safety_pam_set_cmd_proc.h"
#include "AtParse.h"
#include "at_cmd_proc.h"
#include "at_test_para_cmd.h"



/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_SAFETY_PAM_CMD_TBL_C

static const AT_ParCmdElement g_atSafetyPamCmdTbl[] = {
#if (FEATURE_IMS == FEATURE_ON)
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: UICC鉴权
     * [说明]: 执行UICC鉴权，返回鉴权结果，IMS GBA功能使用。
     * [语法]:
     *     [命令]: ^UICCAUTH=<auth_type>,<rand>,<auth>
     *     [结果]: <CR><LF>^UICCAUTH: <result>[,<res>,<ck>,<ik>,<auts>]
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^UICCAUTH=?
     *     [结果]: <CR><LF>^UICCAUTH: (list of supported <auth_type>s),(rand),(auth)<CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <auth_type>: 鉴权的类型：
     *             0：USIM AKA AUTH
     *             1：ISIM AKA AUTH
     *             2：USIM GBA AUTH
     *             3：ISIM GBA AUTH
     *     <rand>: 字符串类型，最大长度512字节
     *     <auth>: 字符串类型，最大长度512字节
     *     <result>: 0：SUCCESS
     *             1：FAIL
     *             2：SYNC FAIL
     *             4：UNSUPPORTED
     *     <res>: 字符串类型，最大长度512字节
     *     <ck>: 字符串类型，最大长度512字节
     *     <ik>: 字符串类型，最大长度512字节
     *     <auts>: 字符串类型，最大长度512字节
     */
    { AT_CMD_UICCAUTH,
      AT_SetUiccAuthPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^UICCAUTH", (VOS_UINT8 *)"(0-3),(rand),(auth)" },

    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: KSNAF鉴权
     * [说明]: 用于KSNAF鉴权，IMS GBA功能使用。
     * [语法]:
     *     [命令]: ^KSNAFAUTH=<auth_type>,<naf_id>,<impi>
     *     [结果]: <CR><LF>^KSNAFAUTH: <result>,<ks_naf>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>+CME ERROR: <err><CR><LF>
     *     [命令]: ^KSNAFAUTH=?
     *     [结果]: <CR><LF>^KSNAFAUTH: <auth_type>,<naf_id>,<impi><CR><LF><CR><LF>OK<CR><LF>
     * [参数]:
     *     <auth_type>: 0：USIM NAF AUTH
     *             1：ISIM NAF AUTH
     *     <naf_id>: 字符串类型，最大长度512字节。
     *     <impi>: 字符串类型，最大长度512字节。
     *     <ks_naf>: 字符串类型，最大长度512字节。
     */
    { AT_CMD_KSNAFAUTH,
      AT_SetKsNafAuthPara, AT_SET_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_NO_LIMITED,
      (VOS_UINT8 *)"^KSNAFAUTH", (VOS_UINT8 *)"(0,1),(naf_id),(impi)" },
#endif

    /*
     * [类别]: 协议AT-安全管理
     * [含义]: 查询IMEI校验信息
     * [说明]: 当Modem启动时，进行IMEI安全校验检查，如果检查失败，IMEI号码默认全零，AP可以使用此命令进行查询校验结果。
     * [语法]:
     *     [命令]: ^IMEIVERIFY?
     *     [结果]: <CR><LF>^IMEIVERIFY: <result><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             错误情况：
     *             <CR><LF>ERROR<CR><LF>
     * [参数]:
     *     <result>: 整型值，IMEI安全校验结果
     *             0：校验失败
     *             1：校验通过
     * [示例]:
     *     · IMEI校验结果查询
     *       AT^IMEIVERIFY?
     *       ^IMEIVERIFY: 0
     *       OK
     */
    { AT_CMD_IMEIVERIFY,
      VOS_NULL_PTR, AT_NOT_SET_TIME, At_QryIMEIVerifyPara, AT_QRY_PARA_TIME, At_CmdTestProcERROR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_PIN_IS_LOCKED,
      (VOS_UINT8 *)"^IMEIVERIFY", VOS_NULL_PTR },
};

/* 注册PAM SAFETY AT命令表 */
VOS_UINT32 AT_RegisterSafetyPamCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atSafetyPamCmdTbl, sizeof(g_atSafetyPamCmdTbl) / sizeof(g_atSafetyPamCmdTbl[0]));
}

