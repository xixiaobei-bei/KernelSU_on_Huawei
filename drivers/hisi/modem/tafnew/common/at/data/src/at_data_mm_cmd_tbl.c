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

#include "at_data_mm_cmd_tbl.h"
#include "at_data_mm_qry_cmd_proc.h"

#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_MM_CMD_TBL_C

static const AT_ParCmdElement g_atDataMmCmdTbl[] = {
#if (defined(FEATURE_PHONE_ENG_AT_CMD) && (FEATURE_PHONE_ENG_AT_CMD == FEATURE_ON))
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询PS域当前使用的加密算法
     * [说明]: 该命令用来查询NR、LTE、WCDMA或GSM的PS域当前使用的加密算法。
     *         注：此命令由FEATURE_PHONE_ENG_AT_CMD宏控制。
     * [语法]:
     *     [命令]: ^CIPHERQRY?
     *     [结果]: LTE、WCDMA或GSM下：
     *             <CR><LF>^CIPHERQRY: <EA><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *             NR下：
     *             <CR><LF>^CIPHERQRY: <EA>,<EA1><CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <EA>: 整型值，取值范围：0-21。
     *             0：没有启动加密(NR、GSM、WCDMA或LTE模式下使用)；
     *             1：GEA1(GSM模式下使用)；
     *             2：GEA2(GSM模式下使用)；
     *             3：GEA3(GSM模式下使用)；
     *             4：GEA4(GSM模式下使用)；
     *             5：GEA5(GSM模式下使用)；
     *             6：GEA6(GSM模式下使用)；
     *             7：GEA7(GSM模式下使用)；
     *             8：UEA1(WCDMA模式下使用)；
     *             9：UEA2(WCDMA模式下使用)；
     *             10：UEA3(WCDMA 模式下使用)；
     *             11：UEA4(WCDMA 模式下使用);
     *             12：UEA5(WCDMA 模式下使用)；
     *             13：UEA6(WCDMA 模式下使用);
     *             14：UEA7(WCDMA 模式下使用)；
     *             15：EEA1(LTE 模式下使用)；
     *             16：EEA2(LTE 模式下使用);
     *             17：EEA3(LTE 模式下使用)；
     *             18：EEA4(LTE 模式下使用);
     *             19：EEA5(LTE 模式下使用)；
     *             20：EEA6(LTE 模式下使用);
     *             21：EEA7(LTE 模式下使用)；
     *             22：NEA1(NR 模式下使用)；
     *             23：NEA2(NR 模式下使用)；
     *             24：NEA3(NR 模式下使用)；
     *             25：SPARE4(NR 模式下使用)；
     *             26：SPARE4 (NR 模式下使用)；
     *             27：SPARE4 (NR 模式下使用)；
     *             28：SPARE4 (NR 模式下使用)。
     *     <EA1>: 0：没有启动加密(NR 模式下使用)；
     *             22：NEA1(NR 模式下使用)；
     *             23：NEA2(NR 模式下使用)；
     *             24：NEA3(NR 模式下使用)；
     *             25：SPARE4(NR 模式下使用)；
     *             26：SPARE4 (NR 模式下使用)；
     *             27：SPARE4 (NR 模式下使用)；
     *             28：SPARE4 (NR 模式下使用)。
     * [示例]:
     *     · 查询PS域当前使用的加密算法为UEA1
     *       AT^CIPHERQRY?
     *       ^CIPHERQRY: 8
     *       OK
     *     · 查询NR下当前使用的加密算法：主小区组(MCG)：NEA1 副小区组(SCG)：NEA2
     *       AT^CIPHERQRY?
     *       ^CIPHERQRY: 22,23
     *       OK
     */
    { AT_CMD_CIPHERQRY,
      VOS_NULL_PTR, AT_NOT_SET_TIME, AT_QryCipherPara, AT_QRY_PARA_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_CME_INCORRECT_PARAMETERS, CMD_TBL_LIMITED_NULL,
      (VOS_UINT8 *)"^CIPHERQRY", VOS_NULL_PTR },
#endif
};

/* 注册MM组件数据业务AT命令表 */
VOS_UINT32 AT_RegisterDataMmCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDataMmCmdTbl, sizeof(g_atDataMmCmdTbl) / sizeof(g_atDataMmCmdTbl[0]));
}

