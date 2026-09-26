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

#include "at_data_drv_cmd_tbl.h"
#include "at_data_drv_set_cmd_proc.h"
#include "at_data_drv_qry_cmd_proc.h"
#include "securec.h"
#include "AtParse.h"
#include "at_cmd_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_DATA_DRV_CMD_TBL_C

static const AT_ParCmdElement g_atDataDrvCmdTbl[] = {
    /*
     * [类别]: 协议AT-分组域业务
     * [含义]: 查询Modem/NDIS支持形态
     * [说明]: 用于后台查询相应的拨号方式。此命令只能查询拨号方式，不能设置拨号方式。
     *         注意：^DIALMODE?命令由客户定制参数范围，海思默认返回0,0。
     * [语法]:
     *     [命令]: ^DIALMODE?
     *     [结果]: <CR><LF>^DIALMODE: <value>,<cdc_spec><CR><LF>
     *             <CR><LF>OK<CR><LF>
     *     [命令]: ^DIALMODE=?
     *     [结果]: <CR><LF>^DIALMODE: (list of supported <value>s), (list of supported <cdc_spec>s) <CR><LF>
     *             <CR><LF>OK<CR><LF>
     * [参数]:
     *     <value>: 整型值，拨号方式。
     *             0：使用Modem拨号；
     *             1：使用NDIS拨号；
     *             2：Modem拨号和NDIS拨号共存。
     *     <cdc_spec>: 整型值，CDC规范符合度。
     *             0：Modem和NDIS端口都不符合CDC规范；
     *             1：仅Modem端口符合CDC规范；
     *             2：仅NDIS端口符合CDC规范；
     *             3：Modem和NDIS端口都符合CDC规范；
     *             4-255：保留。
     * [示例]:
     *     · 查询命令
     *       AT^DIALMODE?
     *       ^DIALMODE: 0,0
     *       OK
     *     · 测试命令
     *       AT^DIALMODE=?
     *       ^DIALMODE: (0-2),(0-255)
     *       OK
     */
    { AT_CMD_DIALMODE,
      At_SetDialModePara, AT_NOT_SET_TIME, At_QryDialModePara, AT_NOT_SET_TIME, VOS_NULL_PTR, AT_NOT_SET_TIME,
      VOS_NULL_PTR, AT_NOT_SET_TIME,
      AT_ERROR, CMD_TBL_PIN_IS_LOCKED | CMD_TBL_CLAC_IS_INVISIBLE_STICK,
      (VOS_UINT8 *)"^DIALMODE", (VOS_UINT8 *)"(0-2),(0-255)" },
};

/* 注册基础数据业务AT命令表 */
VOS_UINT32 AT_RegisterDataDrvCmdTable(VOS_VOID)
{
    return AT_RegisterCmdTable(g_atDataDrvCmdTbl, sizeof(g_atDataDrvCmdTbl) / sizeof(g_atDataDrvCmdTbl[0]));
}

