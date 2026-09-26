/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
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

#include "product_config.h"
#include "osl_types.h"
#include "bsp_dump.h"
#include "bsp_reset.h"
#include "bsp_slice.h"
#include "crdr_config.h"
#include "crdr_exc_handle.h"
#include "crdr_cp_core.h"
#include "crdr_mdmap_core.h"
#include "crdr_nr_core.h"

#undef THIS_MODU
#define THIS_MODU mod_dump

__init s32 bsp_crdr_init(void)
{
    s32 ret;

    ret = crdr_exception_handler_init();
    if (ret == BSP_ERROR) {
        crdr_error("fail to init exception handler\n");
        return BSP_ERROR;
    }

    ret = crdr_register_modem_exc_info();
    if (ret == BSP_ERROR) {
        return BSP_ERROR;
    }

    ret = crdr_mdmcp_init();
    if (ret != BSP_OK) {
        return BSP_ERROR;
    }

    ret = crdr_nr_init();
    if (ret != BSP_OK) {
        return BSP_ERROR;
    }

    crdr_ok("bsp_crdr_init ok");
    return BSP_OK;
}
__init s32 bsp_dump_init(void)
{
    return bsp_crdr_init();
}

s32 bsp_dump_mem_init(void)
{
    return 0;
}
s32 bsp_dump_resave_modem_log(void)
{
    return 0;
}
dump_handle bsp_dump_register_hook(const char *name, dump_hook func)
{
    return 0;
}

s32 bsp_dump_unregister_hook(dump_handle handle)
{
    return 0;
}

u8 *bsp_dump_register_field(u32 mod_id, const char *name, u32 length, u16 version_id)
{
    return NULL;
}

u8 *bsp_dump_get_field_addr(u32 field_id)
{
    return 0;
}

u8 *bsp_dump_get_field_phy_addr(u32 field_id)
{
    return 0;
}

void bsp_dump_save_regs(const void *reg_addr, u32 reg_size)
{
    return;
}
s32 bsp_dump_register_log_notifier(u32 modem_type, log_save_fun save_fun, const char *name)
{
    return 0;
}
void bsp_dump_log_notifer_callback(u32 modem_type, const char *path)
{
    return;
}
s32 bsp_dump_unregister_log_notifier(log_save_fun save_fun)
{
    return 0;
}
bool bsp_dump_is_secdump(void)
{
    return true;
}
dump_product_type_e bsp_dump_get_product_type(void)
{
    return DUMP_MBB;
}
dump_access_mdmddr_type_e bsp_dump_get_access_mdmddr_type(void)
{
    return DUMP_ACCESS_MDD_DDR_SEC;
}
int bsp_dump_save_file(const char *dir, const char *file_name, const void *addr, const void *phy_addr, u32 len)
{
    return 0;
}
int bsp_dump_sec_channel_free(drv_reset_cb_moment_e eparam, s32 usrdata)
{
    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_crdr_init);
#endif
