/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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

/*
 * 1 头文件包含
 */
#include "scm_rate_ctrl.h"
#include <linux/module.h>
#include <mdrv.h>
#include <acore_nv_stru_drv.h>
#include <acore_nv_id_drv.h>
#include <osl_types.h>
#include <bsp_nvim.h>
#include <bsp_socp.h>
#include "diag_system_debug.h"

DRV_DIAG_RATE_STRU g_diag_rate_ctrl;
u32 scm_rate_ctrl_init(void)
{
#ifdef DIAG_SYSTEM_5G
    u32 ret;

    ret = bsp_nvm_read(NV_ID_DRV_DIAG_RATE, (u8 *)&g_diag_rate_ctrl, sizeof(g_diag_rate_ctrl));
    if (ret) {
        diag_error("read rate ctrl nv fail, ret:0x%x\n", ret);
        return ERR_MSP_NV_ERROR_READ;
    }
    ret = bsp_socp_set_rate_ctrl(&g_diag_rate_ctrl);
    if (ret) {
        diag_error("set socp rate fail, ret:0x%x\n", ret);
        return ERR_MSP_NV_ERROR_READ;
    }

    diag_crit("set rate ctrl success\n");
#endif

    return ERR_MSP_SUCCESS;
}

u32 scm_set_rate_limit(enum SCM_RATE_PORT_TYPE port_type)
{
#ifdef DIAG_SYSTEM_5G
    if (port_type >= SCM_RATE_PORT_BUTT) {
        diag_error("set socp rate port type invalid :0x%x\n", port_type);
        return ERR_MSP_NV_ERROR_READ;
    }

    if (g_diag_rate_ctrl.chan_enable == 0) {
        diag_error("socp rate is disable\n");
        return ERR_MSP_NV_ERROR_READ;
    }

    if (port_type == SCM_RATE_PORT_WIFI || port_type == SCM_RATE_PORT_VCOM) {
        bsp_socp_set_rate_threshold(SCM_RATE_VCOM_WIFI_LIMIT);
    } else {
        bsp_socp_set_rate_threshold(g_diag_rate_ctrl.rate_limits);
    }

    diag_crit("set rate ctrl port type:%d\n", port_type);
#endif
    return BSP_OK;
}

