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

#include <product_config.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <securec.h>
#include <bsp_dump.h>
#include <bsp_print.h>
#include <bsp_pm_om.h>
#include "pm_om_hac.h"
#include "pm_om_debug.h"

#ifdef CONFIG_MODULE_HAC_PM
#undef THIS_MODU
#define THIS_MODU mod_pmom

char *g_hac_wakelock_name[] = {
    "nr_ps", "nr_phy", "l2hac_ps", "debug", "rsv1",  "rsv2",  "rsv3",  "rsv4",  "rsv5",  "rsv6",  "rsv7",
    "rsv8",  "rsv9",   "rsv10",    "rsv11", "rsv12", "rsv13", "rsv14", "rsv15", "rsv16", "rsv17", "rsv18",
    "rsv19", "rsv20",  "rsv21",    "rsv22", "rsv23", "rsv24", "rsv25", "rsv26", "rsv27", "rsv28"
};
#define ERR_RET_SNPRINTF_S (-1)
char g_hac_print_buf[HAC_PRINT_BUF_SIZE];
struct hac_dump_info_s g_hac_dump_ctrl;
static void print_hac_pm_info(void)
{
    struct hac_dump *pm_base = NULL;
    u32 i;
    pm_base = (struct hac_dump *)g_hac_dump_ctrl.hac_pm_dump_base;
    if (pm_base == NULL) {
        return;
    }
    for (i = 0; i < HAC_CORE_NUM; i++) {
        pmom_pr_err(
            "[C SR]:hac cpu%d:pm in(0x%x),out(0x%x),"
            "wake slice:(begin:0x%x,end:0x%x),sleep slice(begin:0x%x,end:0x%x)\n",
            i, pm_base->sleep_cnt[i], pm_base->wakeup_cnt[i], pm_base->wakeup_slice[i][HAC_DUMP_BEGIN_SLICE_INDEX],
            pm_base->wakeup_slice[i][HAC_DUMP_END_SLICE_INDEX], pm_base->sleep_slice[i][HAC_DUMP_BEGIN_SLICE_INDEX],
            pm_base->sleep_slice[i][HAC_DUMP_END_SLICE_INDEX]);
    }
}

static void print_hac_wakelock_info(void)
{
    u32 cnt = 0;
    u32 i;
    int safe_len;
    struct hac_dump_info_s *hac_dump_ctrl = (struct hac_dump_info_s *)(uintptr_t)&g_hac_dump_ctrl;
    u32 check_sz = hac_dump_ctrl->hac_logbuf_max_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;
    u32 wakelock_cnt = sizeof(g_hac_wakelock_name) / sizeof(g_hac_wakelock_name[0]);
    struct hac_wakelock_dump_s *wakelock_base = (struct hac_wakelock_dump_s *)hac_dump_ctrl->hac_wakelock_dump_base;
    if ((hac_dump_ctrl->hac_logbuf_max_size == 0) || (wakelock_base == NULL)) {
        return;
    }
    if ((wakelock_base->latest_unlockid < HAC_WAKELOCK_NUMBER) &&
        (wakelock_base->latest_lockid < HAC_WAKELOCK_NUMBER)) {
        pmom_pr_err("[C SR]:hac wakelock info:latest locktime:0x%x,lock_name:%s,"
                    "latest unlocktime:0x%x,unlock_name:%s\n",
                    wakelock_base->latest_lock_time, g_hac_wakelock_name[wakelock_base->latest_lockid],
                    wakelock_base->latest_unlock_time, g_hac_wakelock_name[wakelock_base->latest_unlockid]);
    }
    safe_len = snprintf_s((char *)g_hac_print_buf + cnt, (size_t)HAC_PRINT_BUF_SIZE - cnt,
                          (size_t)(HAC_PRINT_BUF_SIZE - cnt - 1),
                          "[C SR]:hac wakelock(name,lockstate(1:lock,0:unlock),locktotaltime):");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    } else {
        cnt = cnt + (unsigned int)safe_len;
    }
    for (i = 0; i < wakelock_cnt; i++) {
        safe_len = snprintf_s((char *)g_hac_print_buf + cnt, (size_t)HAC_PRINT_BUF_SIZE - cnt,
                              (size_t)(HAC_PRINT_BUF_SIZE - cnt - 1), "(%s,%d,0x%x)", g_hac_wakelock_name[i],
                              ((1u << i) & wakelock_base->hac_wakelock_state), wakelock_base->total_time[i]);
        if (safe_len == ERR_RET_SNPRINTF_S) {
            pmom_pr_err("snprintf_s err\n");
            return;
        } else {
            cnt = cnt + (unsigned int)safe_len;
        }
        if (cnt >= check_sz) {
            pmom_pr_err("%s", g_hac_print_buf);
            cnt = 0;
        }
    }
    safe_len = snprintf_s((char *)g_hac_print_buf + cnt, (size_t)HAC_PRINT_BUF_SIZE - cnt,
                          (size_t)(HAC_PRINT_BUF_SIZE - cnt - 1), "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }
    pmom_pr_err("%s", g_hac_print_buf);
}

static void print_hac_dpm_info(void)
{
    int safe_len;
    ssize_t count = 0;
    struct hac_dpm_device_info *device_info = NULL;
    struct hac_dump_info_s *hac_dump_ctrl = (struct hac_dump_info_s *)(uintptr_t)&g_hac_dump_ctrl;
    u32 check_sz = hac_dump_ctrl->hac_logbuf_max_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;
    device_info = (struct hac_dpm_device_info *)(hac_dump_ctrl->hac_dpm_dump_base);
    if ((device_info == NULL) || (hac_dump_ctrl->hac_logbuf_max_size == 0)) {
        return;
    }
    pmom_pr_err("[C SR] hac dpm (fail_cnt, max suspend, resume):");

    while (*device_info->device_name != 0) {
        if (((uintptr_t)device_info + sizeof(struct dpm_device_info)) >=
            ((uintptr_t)hac_dump_ctrl->hac_dpm_dump_base + hac_dump_ctrl->hac_dpm_dump_size)) {
            return;
        }
        if (strlen(device_info->device_name) > HAC_DPM_DEBUG_CHAR_NUM) {
            pmom_pr_err("print hacdpm get an err name \n");
            break;
        }
        safe_len = snprintf_s(g_hac_print_buf + count, (size_t)(HAC_PRINT_BUF_SIZE - count),
                              (size_t)(HAC_PRINT_BUF_SIZE - count - 1), "%s(%d,%d,%d),", device_info->device_name,
                              device_info->fail_cnt, device_info->max_s, device_info->max_r);
        if (safe_len == ERR_RET_SNPRINTF_S) {
            pmom_pr_err("snprintf_s err\n");
            return;
        } else {
            count = count + (unsigned int)safe_len;
        }
        if (count >= check_sz) {
            pmom_pr_err("%s", g_hac_print_buf);
            count = 0;
        }
        device_info++;
    }
    safe_len = snprintf_s(g_hac_print_buf + count, (size_t)(HAC_PRINT_BUF_SIZE - count),
                          (size_t)(HAC_PRINT_BUF_SIZE - count - 1), "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }

    pmom_pr_err("%s", g_hac_print_buf);
}
void print_hac_lowpower_info(void)
{
    if (g_hac_dump_ctrl.hac_dump_base == NULL) {
        return;
    }
    print_hac_pm_info();
    print_hac_wakelock_info();
    print_hac_dpm_info();
}
void hacpm_suspend_prepare(void)
{
    struct hac_dump_info_s *hac_dump_ctrl = (struct hac_dump_info_s *)(uintptr_t)&g_hac_dump_ctrl;

    if (hac_dump_ctrl->hac_dump_base == NULL) {
        hac_dump_ctrl->hac_dump_base = (void *)bsp_dump_get_field_addr(DUMP_NRL2HAC_DRX);
        if (hac_dump_ctrl->hac_dump_base == NULL) {
            return;
        }
        hac_dump_ctrl->hac_dpm_dump_base =
            (void *)((uintptr_t)hac_dump_ctrl->hac_dump_base + hac_dump_ctrl->hac_dpm_dump_offset);
        hac_dump_ctrl->hac_pm_dump_base =
            (void *)((uintptr_t)hac_dump_ctrl->hac_dump_base + hac_dump_ctrl->hac_pm_dump_offset);
        hac_dump_ctrl->hac_wakelock_dump_base =
            (void *)((uintptr_t)hac_dump_ctrl->hac_dump_base + hac_dump_ctrl->hac_wakelock_dump_offset);
        hac_dump_ctrl->hac_logbuf_max_size = bsp_print_get_len_limit();
    }
}
void hac_dump_init(void)
{
    struct device_node *node = NULL;
    struct hac_dump_info_s *hac_dump_ctrl = (struct hac_dump_info_s *)(uintptr_t)&g_hac_dump_ctrl;
    u32 ret;
    node = of_find_compatible_node(NULL, NULL, "hisilicon,hac_pm_om");
    if (node == NULL) {
        pmom_pr_err("of_find_compatible_node hac_pm_om failed\n");
        return;
    }

    ret = of_property_read_u32_array(node, "hac_pmom_size", &hac_dump_ctrl->hac_pm_dump_size, 1);
    ret += of_property_read_u32_array(node, "hac_pmom_offset", &hac_dump_ctrl->hac_pm_dump_offset, 1);
    ret += of_property_read_u32_array(node, "hac_wakelock_size", &hac_dump_ctrl->hac_wakelock_dump_size, 1);
    ret += of_property_read_u32_array(node, "hac_wakelock_offset", &hac_dump_ctrl->hac_wakelock_dump_offset, 1);
    ret += of_property_read_u32_array(node, "hac_dpm_size", &hac_dump_ctrl->hac_dpm_dump_size, 1);
    ret += of_property_read_u32_array(node, "hac_dpm_offset", &hac_dump_ctrl->hac_dpm_dump_offset, 1);
    if (ret) {
        pmom_pr_err("hac_dump_ctrl get elm failed\n");
        return;
    }
}
#else
void hac_dump_init(void)
{
}
void hacpm_suspend_prepare(void)
{
}
void print_hac_lowpower_info(void)
{
}
#endif
