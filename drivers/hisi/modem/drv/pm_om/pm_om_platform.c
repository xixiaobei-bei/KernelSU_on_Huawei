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

#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <osl_sem.h>
#include <bsp_hardtimer.h>
#include <bsp_pm_om.h>
#include <bsp_ipc.h>
#include "pm_om_platform.h"
#include "pm_om_debug.h"
#include "modem_log_linux.h"
#include "securec.h"

#undef THIS_MODU
#define THIS_MODU mod_pmom

struct pm_om_platform g_pmom_platform;

u32 g_icc_channel_ready;
u32 g_icc_channel_pending;

#ifdef CONFIG_PM_OM_NR
static u32 g_icc_data_ready;
static u32 g_nricc_data_ready;
static spinlock_t g_data_spinlock;
u32 g_nricc_channel_ready;
u32 g_nricc_channel_pending;

u32 pm_om_is_nricc_channel_ready(void)
{
    return g_nricc_channel_ready;
}

void pm_om_set_nricc_channel_pending(u32 val)
{
    g_nricc_channel_pending = val;
}

static s32 pm_om_nricc_handler(pm_om_icc_data_type data)
{
    u32 channel_id;
    s32 ret;
    switch (data) {
        case MODEM_NR_DATA_OK:
            spin_lock(&g_data_spinlock);
            g_nricc_data_ready = 1;
            if (g_icc_data_ready) {
                modem_log_wakeup_all();
                g_icc_data_ready = 0;
                g_nricc_data_ready = 0;
            }
            spin_unlock(&g_data_spinlock);
            break;
        case MODEM_NRICC_CHN_READY:
            g_nricc_channel_ready = 1;
            if (g_nricc_channel_pending) {
                channel_id = PM_OM_NRICC_ACORE_CHN_ID;
                ret = bsp_icc_send(ICC_CPU_NRCCPU, channel_id, (u8 *)&data, (u32)sizeof(data));
                if (sizeof(data) != ret) {
                    pmom_pr_err("icc[0x%x] send fail: 0x%x\n", channel_id, ret);
                    return ret;
                }
                g_nricc_channel_pending = 0;
            }
            break;
        default:
            break;
    }
    return PM_OM_OK;
}
#endif

u32 pm_om_is_icc_channel_ready(void)
{
    return g_icc_channel_ready;
}

void pm_om_set_icc_channel_pending(u32 val)
{
    g_icc_channel_pending = val;
}

int pm_log_read(struct log_usr_info *usr_info, char *buf, u32 count)
{ /*lint --e{715} suppress buf&count not referenced*/
    struct ring_buffer rb;
    modem_log_ring_buffer_get(usr_info, &rb);

    /* 提前清空buffer */
    if (bsp_ring_buffer_writable_size(&rb) <= MAX_BUFFER_SIZE) {
        usr_info->mem->read = usr_info->mem->write;
    }
    return PM_OM_OK;
}

int pm_om_fwrite_trigger(void)
{
    if (g_pmom_platform.ctrl == NULL || !g_pmom_platform.ctrl->log.init_flag) {
        return PM_OM_ERR;
    }

    bsp_modem_log_fwrite_trigger(&(g_pmom_platform.log_info));
    return PM_OM_OK;
}

static s32 pm_om_icc_handler(u32 channel_id, u32 len, void *context)
{
    pm_om_icc_data_type data = 0;
    s32 ret;

    ret = bsp_icc_read(channel_id, (u8 *)&data, sizeof(data));
    if (sizeof(data) != (u32)ret) {
        pmom_pr_err("icc read fail: 0x%x\n", ret);
        return ret;
    }
    switch (data) {
        case MODEM_DATA_OK:
#ifdef CONFIG_PM_OM_NR
            spin_lock(&g_data_spinlock);
            g_icc_data_ready = 1;
            if (g_nricc_data_ready) {
#endif
                modem_log_wakeup_all();
#ifdef CONFIG_PM_OM_NR
                g_icc_data_ready = 0;
                g_nricc_data_ready = 0;
            }
            spin_unlock(&g_data_spinlock);
#endif
            break;
        case MODEM_ICC_CHN_READY:
            g_icc_channel_ready = 1;
            if (g_icc_channel_pending) {
                channel_id = PM_OM_ICC_ACORE_CHN_ID;
                ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8 *)&data, (u32)sizeof(data));
                if (sizeof(data) != ret) {
                    pmom_pr_err("icc[0x%x] send fail: 0x%x\n", channel_id, ret);
                    return ret;
                }
                g_icc_channel_pending = 0;
            }
            break;
        default:
            break;
    }
#ifdef CONFIG_PM_OM_NR
    return pm_om_nricc_handler(data);
#endif
    return PM_OM_OK;
}

static int pm_om_icc_init(void)
{
    u32 channel_id = PM_OM_ICC_ACORE_CHN_ID;
    if (bsp_icc_event_register(channel_id, pm_om_icc_handler, NULL, NULL, NULL)) {
        pmom_pr_err("icc chn =%d err\n", channel_id);
        return PM_OM_ERR;
    }
#ifdef CONFIG_PM_OM_NR
    channel_id = PM_OM_NRICC_ACORE_CHN_ID;
    if (bsp_icc_event_register(channel_id, pm_om_icc_handler, NULL, NULL, NULL)) {
        pmom_pr_err("icc chn =%d err\n", channel_id);
        return PM_OM_ERR;
    }
    spin_lock_init(&g_data_spinlock);
#endif
    return PM_OM_OK;
}

int pm_om_platform_init(void)
{
    int ret;
    char *dev_name = "pmom"; /*lint !e578: (Warning -- Declaration of symbol 'dev_name' hides symbol */
    struct pm_om_ctrl *ctrl = pm_om_ctrl_get();

    ret = memset_s((void *)&g_pmom_platform, sizeof(g_pmom_platform), 0, sizeof(g_pmom_platform));
    if (ret) {
        pmom_pr_err("g_pmom_platform memset ret = %d\n", ret);
        return PM_OM_ERR;
    }
    g_pmom_platform.log_info.dev_name = dev_name;
    g_pmom_platform.log_info.mem = &(ctrl->log.smem->mem_info);
    g_pmom_platform.log_info.ring_buf = ctrl->log.rb.buf;
    g_pmom_platform.log_info.read_func = (USR_READ_FUNC)pm_log_read;

    ret = bsp_modem_log_register(&g_pmom_platform.log_info);
    if (ret) {
        pmom_pr_err("register modem log error\n");
        return PM_OM_ERR;
    }
    g_pmom_platform.ctrl = ctrl;
    ctrl->platform = (void *)&g_pmom_platform;

    ret = pm_om_icc_init();
    if (ret) {
        pmom_pr_err("pm_om_icc_init error\n");
        return PM_OM_ERR;
    }

    return PM_OM_OK;
}
EXPORT_SYMBOL(pm_om_fwrite_trigger);
