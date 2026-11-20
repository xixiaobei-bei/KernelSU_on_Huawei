/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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


#ifndef __DRV_NV_ID_H__
#define __DRV_NV_ID_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*
 *  NV ID 的添加按从小到大排列
 */

typedef enum
{

    NV_ID_DRV_NV_DRV_VERSION_REPLACE_I     = 50018,     //NV_SW_VER_STRU
    NV_ID_DRV_SEC_BOOT_ENABLE_FLAG         = 50201,     //for sys list
    NV_ID_DRV_NV_PWC_SWITCH                = 0xd10b,    //ST_PWC_SWITCH_STRU
    NV_ID_DRV_NV_DFS_SWITCH                = 0xd10c,    //ST_PWC_DFS_STRU
    NV_ID_DRV_DUMP                         = 0xd111,    //NV_DUMP_STRU
    NV_ID_DRV_NV_VERSION_INIT              = 0xd115,    //PRODUCT_INFO_NV_STRU
    NV_ID_DRV_MODULE_SUPPORT               = 0xd118,    //DRV_MODULE_SUPPORT_STRU
    NV_ID_DRV_COUL_CALI                    = 0xd119,    //COUL_CALI_NV_TYPE
    NV_ID_DRV_UART_SHELL_FLAG              = 0Xd127,    //DRV_UART_SHELL_FLAG
    NV_ID_DRV_PM                           = 0Xd128,    //DRV_NV_PM_TYPE
    NV_ID_DRV_WDT_INIT_PARAM               = 0xd12d,    //DRV_WDT_INIT_PARA_STRU
    NV_ID_DRV_DIALUP_ACSHELL               = 0xd143,    //DRV_DIALUP_HSUART_STRU
    NV_ID_DRV_PM_OM                        = 0xd145,    //DRV_PM_OM_CFG_STRU
    NV_ID_DRV_WATCHPOINT                   = 0xd148,    //DRV_WATCHPOINT_CFG_STRU
    NV_ID_DRV_SELF_CTRL                    = 0xd149,    //NV_SELF_CTRL_STRU
    NV_ID_DRV_NV_DEBUG_CFG                 = 0xd14d,    //NV_DEBUG_CFG_STRU
    NV_ID_DRV_TEST_SUPPORT                 = 0Xd151,    //DRV_MODULE_TEST_STRU
    NV_ID_DRV_PMU                          = 0xd158,    //DRV_PMU_CFG_STRU
    NV_ID_DRV_CORESIGHT                    = 0xd168,    //DRV_CORESIGHT_CFG_STRU
    NV_ID_DRV_DIAG_PORT                    = 0xd16f,    //DIAG_CHANNLE_PORT_CFG_STRU
    NV_ID_DRV_IQI                          = 0xd17e,    //DRV_IQI_CFG_STRU

    NV_ID_DRV_ID_MAX                       = 0xd1ff

}NV_ID_DRV_ENUM;

#define en_NV_Item_SEC_BOOT_FLAG    NV_ID_DRV_SEC_BOOT_ENABLE_FLAG

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

