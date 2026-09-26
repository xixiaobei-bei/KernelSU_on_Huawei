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
/*lint --e{715,838}*/
/* 715:Symbol  not referenced, 838 Previously assigned value to variable has not been used */
#include <mdrv_pm.h>
#include <bsp_pm_om.h>
#include <bsp_pm.h>
#include <osl_module.h>
#undef THIS_MODU
#define THIS_MODU mod_pmom
#ifndef CONFIG_PM_OM_BALONG

int bsp_pm_log(u32 magic, u32 data_len, const void *data)
{
    return -1;
}

int bsp_pm_info_stat_register(pm_info_cbfun pcbfun, struct pm_info_usr_data *usr_data)
{
    return -1;
}

int bsp_pm_log_type(u32 magic, u32 type, u32 data_len, const void *data)
{
    return -1;
}

void* bsp_pm_log_addr_get(void)
{
    return NULL;
}

void* bsp_pm_dump_get(u32 magic, u32 len)
{
    return NULL;
}

void bsp_pm_om_wakeup_stat(void)
{
}
void bsp_pm_om_wakeup_log(void)
{
}
s32 pm_wakeup_nrcore(enum debug_wake_type type)
{
    return 0;
}

int bsp_ring_buffer_in(struct ring_buffer *rb, void *data, u32 len, MEMCPY_FUNC memcpy_func)
{
    return 0;
}
int bsp_ring_buffer_out(struct ring_buffer *rb, void *data, u32 len, MEMCPY_FUNC memcpy_func)
{
    return 0;
}

s32 bsp_modem_log_register(struct log_usr_info *usr_info)
{
    char *dev_name = NULL;
    if (usr_info != NULL) {
        dev_name = usr_info->dev_name;
    }
    pmom_pr_err("is stub caller is %s\n", dev_name);
    return 0;
}
void bsp_modem_log_fwrite_trigger(struct log_usr_info *usr_info)
{
    char *dev_name = NULL;
    if (usr_info != NULL) {
        dev_name = usr_info->dev_name;
    }
    pmom_pr_err("is stub caller is %s\n", dev_name);
}
void modem_log_ring_buffer_get(struct log_usr_info *usr_info, struct ring_buffer *rb)
{
    char *dev_name = NULL;
    if (usr_info != NULL) {
        dev_name = usr_info->dev_name;
    }
    pmom_pr_err("is stub caller is %s\n", dev_name);
}
int bsp_pm_om_log_init(void)
{
    pmom_pr_err("is stub\n");
    return 0;
}
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_pm_om_log_init);
#endif
#endif /* CONFIG_PM_OM_BALONG */

int mdrv_pm_log(int mod_id, unsigned int data_len, const void *data)
{
    int ret;
    if (data == NULL) {
        pmom_pr_err("err: data = NULL\n");
        return -1;
    }
    if (mod_id >= PM_OM_MOD_ID_ENUM_MAX) {
        pmom_pr_err("err: invalid mod_id(%d)\n", mod_id);
        return -1;
    }

    ret = bsp_pm_log((u32)mod_id, data_len, data);
    return ret == PM_OM_OK ? 0 : -1;
}

int mdrv_pm_info_stat_register(pm_info_cbfun pcbfun, struct pm_info_usr_data *usr_data)
{
    int ret;

    if (usr_data == NULL || pcbfun == NULL) {
        pmom_pr_err("null pointer: usr_data, pcbfun\n");
        return -1;
    } else if (usr_data->mod_id >= PM_OM_MOD_ID_ENUM_MAX) {
        pmom_pr_err("err: invalid mod_id(%d)\n", usr_data->mod_id);
        return -1;
    } else {
        pmom_pr_err("err: invalid branch\n");
        return -1;
    }
    ret = bsp_pm_info_stat_register(pcbfun, usr_data);
    return ret == PM_OM_OK ? 0 : -1;
}
EXPORT_SYMBOL(mdrv_pm_info_stat_register); /*lint !e19 */
EXPORT_SYMBOL(mdrv_pm_log); /*lint !e19 */
