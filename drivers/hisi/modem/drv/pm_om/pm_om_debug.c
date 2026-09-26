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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/suspend.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif
#include <bsp_slice.h>
#include <bsp_pm.h>
#include <bsp_icc.h>
#include <bsp_dump.h>
#include <bsp_pm_om.h>
#include <bsp_m3pm_log.h>
#include "pm_om_platform.h"
#include "pm_om_debug.h"
#include "pm_om_hac.h"
#include "pm_om_pressure.h"
#include "product_config.h"
#include "securec.h"

#undef THIS_MODU
#define THIS_MODU mod_pmom

struct pm_om_debug g_pmom_debug;
char g_print_buf[PRINT_BUF_SIZE];

char *g_wakelock_name[] = {
    "TLPHY",      "PS_G0", "PS_W0", "PS_G1",  "PS_W1", "FTM",    "FTM_1", "NAS",    "NAS_1",   "OAM",  "SCI0",
    "SCI1",       "TLPS",  "TLPS1", "DSFLOW", "PM",    "LR_EASYRF",  "TDS",   "SI", /* pam */
    "USIM",                                                                     /* oam */
    "DSPPOWERON", /* v8r1 ccore 提供给GUTL DSP作为c核上电初始化投票用 */
    "RESET",      "PS_G2", "FTM_2", "NAS_2",  "1X",    "SENSOR", "MSP",   "VOWIFI", "DSFLOW1", "LTEV", "TTF"
};

void bsp_pm_om_wakeup_log(void)
{
    int ret;
    if (g_pmom_debug.stat.wrs_flag == 1) {
        ret = bsp_pm_log(PM_OM_PMLG, 0, NULL);
        if (ret != 0) {
            pmom_print("pm_om_wakeup_log send err\n");
        }
        g_pmom_debug.stat.wrs_flag = 0;
    }
}

void bsp_pm_om_wakeup_stat(void)
{
    u32 cur_slice = bsp_get_slice_value();
    u32 delta_slice = 0;

    g_pmom_debug.stat.wakeup_cnt++;

    delta_slice = delta_slice - g_pmom_debug.stat.waket_prev;
    g_pmom_debug.stat.waket_prev = cur_slice;
    if (delta_slice > g_pmom_debug.stat.waket_max) {
        g_pmom_debug.stat.waket_max = delta_slice;
    }
    if (delta_slice < g_pmom_debug.stat.waket_min) {
        g_pmom_debug.stat.waket_min = delta_slice;
    }

    g_pmom_debug.stat.wrs_flag = 1;
    pmom_print("[C SR]pm om wakeup\n");
}

u32 pm_om_log_time_rec(u32 time_log_start)
{
    u32 ret = 0;
    u32 delta_slice = get_timer_slice_delta(time_log_start, bsp_get_slice_value());
    if (g_pmom_debug.stat.logt_print_sw && delta_slice > g_pmom_debug.stat.logt_max) {
        g_pmom_debug.stat.logt_max = delta_slice;
        ret = delta_slice;
    }
    return ret;
}

void pm_om_log_show(void)
{
    struct pm_om_ctrl *ctrl = pm_om_ctrl_get();
    pmom_print("**********************************************\n");
    pmom_print("init_flag   : 0x%x\n", ctrl->log.init_flag);
    pmom_print("buf_is_full : 0x%x\n", ctrl->log.buf_is_full);
    pmom_print("threshold   : 0x%x\n", ctrl->log.threshold);
    pmom_print("**********************************************\n");
}

void pm_om_debug_show(void)
{
    pmom_print("**********************************************\n");
    pmom_print("logt_print_sw: 0x%x\n", g_pmom_debug.stat.logt_print_sw);
    pmom_print("logt_max\t: 0x%x\n", g_pmom_debug.stat.logt_max);
    pmom_print("**********************************************\n");
}

u32 pm_om_feature_on(void)
{
    struct pm_om_ctrl *ctrl = pm_om_ctrl_get();
    struct pm_om_platform *linux_plat = NULL;
    if (ctrl == NULL || ctrl->platform == NULL) {
        return 1;
    }

    linux_plat = (struct pm_om_platform *)ctrl->platform;
    ctrl->log.init_flag = PM_OM_INIT_MAGIC;
    ctrl->log.smem->nv_cfg.mod_sw_bitmap = PM_OM_ALL_MOD_ON;
    linux_plat->log_info.mem_is_ok = 1;

    return 0;
}

static inline int print_check_ret(int ret, unsigned int *count, unsigned int check_sz)
{
    if (ret == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return ERR_RET_SNPRINTF_S;
    } else {
        *count = *count + (unsigned int)ret;
    }
    if (*count >= check_sz) {
        pmom_pr_err("%s\n", g_print_buf);
        *count = 0;
    }
    return 0;
}

static inline void print_dpm_device_info(void)
{
    int safe_len;
    unsigned int count = 0;
    struct dpm_device_info *device_info = NULL;
    struct pm_om_debug *pmom_debug = &g_pmom_debug;

    unsigned int check_sz = pmom_debug->max_log_buffer_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;
    if (pmom_debug->max_log_buffer_size == 0) {
        return;
    }
    device_info = (struct dpm_device_info *)(uintptr_t)(pmom_debug->cdrx_dump_addr + CDRX_DUMP_DPM_INFOS_OFFSET);

    safe_len = snprintf_s(g_print_buf, (size_t)PRINT_BUF_SIZE, (size_t)(PRINT_BUF_SIZE - 1),
                          "[C SR]dpm (fail_cnt, max suspend, resume):");
    if (print_check_ret(safe_len, &count, check_sz) == ERR_RET_SNPRINTF_S) {
        return;
    }

    while (*device_info->device_name != 0) {
        if (((uintptr_t)device_info + sizeof(struct dpm_device_info)) >=
            ((uintptr_t)pmom_debug->cdrx_dump_addr + CDRX_DUMP_DPM_INFOS_END)) {
            return;
        }
        if (strlen(device_info->device_name) > DPM_DEBUG_CHAR_NUM) {
            pmom_pr_err("print cdpm get an err name \n");
            break;
        }
        safe_len = snprintf_s(g_print_buf + count, (size_t)(PRINT_BUF_SIZE - count),
                              (size_t)(PRINT_BUF_SIZE - count - 1), "%s(%d,%d,%d),", device_info->device_name,
                              device_info->fail_cnt, device_info->max_s, device_info->max_r);
        if (print_check_ret(safe_len, &count, check_sz) == ERR_RET_SNPRINTF_S) {
            return;
        }

        device_info++;
    }
    safe_len = snprintf_s(g_print_buf + count, (size_t)(PRINT_BUF_SIZE - count), (size_t)(PRINT_BUF_SIZE - count - 1),
                          "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }
    pmom_pr_err("%s", g_print_buf);
}
static inline void print_ccpu_wakeup_irq_info(void)
{
    int safe_len;
    u32 ret, i;
    u32 cnt = 0;
    struct pm_om_debug *pmom_debug = &g_pmom_debug;
    unsigned int check_sz = pmom_debug->max_log_buffer_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;
    struct pm_wakeup_irq_info *wakeirq_debug_addr = NULL;
    if (pmom_debug->max_log_buffer_size == 0) {
        return;
    }
    wakeirq_debug_addr = (struct pm_wakeup_irq_info *)(uintptr_t)(pmom_debug->cdrx_dump_corepm_addr + WAKEUP_IRQ_DEBUG);
    ret = (u32)readl(pmom_debug->cdrx_dump_corepm_addr + WAKEUP_INT_NUM);
    if (ret > CCPU_WAKEUP_IRQ_NUM_MAX) {
        pmom_pr_err("WAKEUP_INT_NUM %u is too large!\n", ret);
        return;
    }
    safe_len = snprintf_s((char *)g_print_buf, (size_t)PRINT_BUF_SIZE, (size_t)(PRINT_BUF_SIZE - 1),
                          "[C SR]pm wake cnt:");
    if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
        return;
    }
    for (i = 0; i < ret; i++) {
        if (strlen((wakeirq_debug_addr + i)->name) > DPM_DEBUG_CHAR_NUM) {
            pmom_pr_err("print cwakelock get an err name \n");
            break;
        }
        safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                              (size_t)(PRINT_BUF_SIZE - cnt - 1), "%s(%d),", (wakeirq_debug_addr + i)->name,
                              (wakeirq_debug_addr + i)->wake_cnt);
        if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
            return;
        }
    }
    safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                          (size_t)(PRINT_BUF_SIZE - cnt - 1), "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }
    pmom_pr_err("%s", g_print_buf);
}
static inline void print_ccpu_wakelock_info(void)
{
    int safe_len;
    u32 i, ret, temp;
    u32 cnt = 0;
    struct pm_om_debug *pmom_debug = &g_pmom_debug;
    unsigned int check_sz = pmom_debug->max_log_buffer_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;
    if (pmom_debug->max_log_buffer_size == 0) {
        return;
    }

    safe_len = snprintf_s((char *)g_print_buf, (size_t)PRINT_BUF_SIZE, (size_t)(PRINT_BUF_SIZE - 1),
                          "[C SR]wakelock(locked,total time):");
    if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
        return;
    }
    ret = (u32)readl(pmom_debug->cdrx_dump_addr + CDRX_DUMP_WAKE_OFFSET);
    temp = sizeof(g_wakelock_name) / sizeof(g_wakelock_name[0]);
    for (i = 0; i < temp; i++) {
        safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                              (size_t)(PRINT_BUF_SIZE - cnt - 1), "%s(%d,0x%x),", g_wakelock_name[i], (ret >> i) & 0x1,
                              readl(pmom_debug->cdrx_dump_addr + CDRX_DUMP_WAKELOCK_TIME_OFFSET + 0x4 * i));
        if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
            return;
        }
    }
    safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                          (size_t)(PRINT_BUF_SIZE - cnt - 1),
                          "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }
    pmom_pr_err("%s", g_print_buf);
}

static inline void print_ccpu_pm_info(void)
{ /*lint --e{826} suppress pointer-to-pointer conversion*/
    pmom_pr_err("[C SR]pm:in cnt:(%d),out cnt:(%d),dpm fail cnt:(%d),gic fail cnt:(%d),sleep total(0x%x),"
                "wake total(0x%x),last waked by %s\n",
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + PM_ENTER_COUNT)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + PM_OUT_COUNT)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + PM_DPM_FAIL_COUNT)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + PM_GIC_SUS_FAIL_TIME)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + PM_SYS_SLEEP_TOTAL_TIME)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + PM_SYS_WORK_TOTAL_TIME)),
                (char *)((g_pmom_debug.cdrx_dump_corepm_addr + WAKEUP_LATEST_INTER_NAME)));
    pmom_pr_err("[C SR]pm slice(start,end):sleep (0x%x, 0x%x), wake (0x%x, 0x%x)\n",
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + STAMP_PM_SUSPEND_START)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + STAMP_AFTER_SEND_IPC)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + STAMP_PWRUP_CODE_BEGIN)),
                *((u32 *)(g_pmom_debug.cdrx_dump_corepm_addr + STAMP_AFTER_DPM_RESUME)));
    print_ccpu_wakeup_irq_info();
}

static inline void print_ccpu_ipc_info(void)
{
    int safe_len;
    u32 i;
    u32 cnt = 0;
    unsigned int check_sz = g_pmom_debug.max_log_buffer_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;

    safe_len = snprintf_s((char *)g_print_buf, (size_t)PRINT_BUF_SIZE, (size_t)(PRINT_BUF_SIZE - 1),
                          "[C SR]ipcm(int count):");
    if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
        return;
    }

    for (i = 0; i < IPC_INT_BUTTOM; i++) {
        safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                              (size_t)(PRINT_BUF_SIZE - cnt - 1),
                              "%d,", readl(g_pmom_debug.cdrx_dump_addr + CDRX_DUMP_IPC_OFFSET + 0x4 * i));
        if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
            return;
        }
    }

    safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt), (size_t)(PRINT_BUF_SIZE - cnt - 1),
                          "\n[C SR]ipclock(stopsuspend count):");
    if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
        return;
    }

    for (i = 0; i < IPC_SEM_BUTTOM; i++) {
        safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                              (size_t)(PRINT_BUF_SIZE - cnt - 1), "%d,",
                              readl(g_pmom_debug.cdrx_dump_addr + CDRX_DUMP_IPC_OFFSET +
                                    (0x4 * IPC_INT_BUTTOM) + 0x4 * i));
        if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
            return;
        }
    }

    safe_len = snprintf_s((char *)g_print_buf + cnt,
                          (size_t)(PRINT_BUF_SIZE - cnt), (size_t)(PRINT_BUF_SIZE - cnt - 1), "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }

    pmom_pr_err("%s", g_print_buf);
}

static inline void print_ccpu_icc_info(void)
{
    int safe_len;
    u32 i;
    u32 cnt = 0;
    unsigned int check_sz = g_pmom_debug.max_log_buffer_size - PM_OM_PRINT_HRESHOLD_CHECK_SIZE;

    safe_len = snprintf_s((char *)g_print_buf, (size_t)PRINT_BUF_SIZE, (size_t)(PRINT_BUF_SIZE - 1),
                          "[C SR]icc(channel count):");
    if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
        return;
    }
    for (i = 0; i < ICC_CHN_ID_MAX; i++) {
        safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt),
                              (size_t)(PRINT_BUF_SIZE - cnt - 1),
                              "%d,", readl(g_pmom_debug.cdrx_dump_addr + CDRX_DUMP_ICC_OFFSET + 0x4 * i));
        if (print_check_ret(safe_len, &cnt, check_sz) == ERR_RET_SNPRINTF_S) {
            return;
        }
    }

    safe_len = snprintf_s((char *)g_print_buf + cnt, (size_t)(PRINT_BUF_SIZE - cnt), (size_t)(PRINT_BUF_SIZE - cnt - 1),
                          "\n");
    if (safe_len == ERR_RET_SNPRINTF_S) {
        pmom_pr_err("snprintf_s err\n");
        return;
    }

    pmom_pr_err("%s", g_print_buf);
}


void print_ccpu_lowpower_info(void)
{
    if (g_pmom_debug.cdrx_dump_addr != NULL) {
        pmom_pr_err("[C SR]current slice:0x%x\n", bsp_get_slice_value());
        print_ccpu_wakelock_info();
        print_ccpu_pm_info();
        print_dpm_device_info();
        print_ccpu_ipc_info();
        print_ccpu_icc_info();
    }
}
void lrpm_suspend_prepare(void)
{
    char *dump_base = NULL;
    if (g_pmom_debug.cdrx_dump_addr == NULL) {
        /* 获取DUMP 低功耗共享内存地址 */
        dump_base = (char *)bsp_dump_get_field_addr(DUMP_LRCCPU_DRX);
        if (dump_base == NULL) {
            pmom_pr_debug("*******get cdrx dump buffer failed!*******\n");
            return;
        }
        g_pmom_debug.cdrx_dump_addr = (char *)dump_base;
        g_pmom_debug.cdrx_dump_corepm_addr = (char *)(dump_base + CDRX_DUMP_PM_OFFSET);
    }
}
int cp_pm_notify(struct notifier_block *nb, unsigned long event, void *dummy)
{ /*lint --e{715} suppress 'nb' & 'dummy' not referenced*/
    if (event == PM_SUSPEND_PREPARE) {
        if (g_pmom_debug.max_log_buffer_size == 0) {
            g_pmom_debug.max_log_buffer_size = bsp_print_get_len_limit();
        }
        lrpm_suspend_prepare();
        nrpm_suspend_prepare();
        hacpm_suspend_prepare();
    }
    if (event == PM_POST_SUSPEND) {
        /* 此处用于增加CCPU的打印信息函数 */
        print_ccpu_lowpower_info();
        print_m3_lopower_info();
        print_nrccpu_lowpower_info();
        print_hac_lowpower_info();
    }
    return 0;
}

/* pmom在mbb/phone共代码, 都需要实现. pm_wakeup_ccore在phone需要在pmom模块实现,mbb上在pm模块实现 */
s32 pm_wakeup_ccore(enum debug_wake_type type)
{
    u32 channel_id = (ICC_CHN_IFC << SHITF_SHORT) | IFC_RECV_FUNC_WAKEUP;
    s32 ret;
    struct debug_pm_s pm_msg;
    u32 msglen = sizeof(pm_msg);
    pm_msg.wake_type = type;

    ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8 *)(&pm_msg), msglen);
    if ((u32)ret != msglen) {
        return -1;
    }
    return 0;
}

s32 pm_wakeup_nrcore(enum debug_wake_type type)
{
    u32 channel_id = (ICC_CHN_NRIFC << SHITF_SHORT) | NRIFC_RECV_FUNC_WAKEUP_DEBUG;
    s32 ret;
    struct debug_pm_s pm_msg;
    u32 msglen = sizeof(pm_msg);
    pm_msg.wake_type = type;

    ret = bsp_icc_send(ICC_CPU_NRCCPU, channel_id, (u8 *)(&pm_msg), msglen);
    if ((u32)ret != msglen) {
        return -1;
    }
    return 0;
}

void debug_pm_wake_lock(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    __pm_stay_awake(g_pmom_debug.wakelock_debug);
#else
    __pm_stay_awake(&g_pmom_debug.wakelock_debug);
#endif
}

void debug_pm_wake_unlock(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    __pm_relax(g_pmom_debug.wakelock_debug);
#else
    __pm_relax(&g_pmom_debug.wakelock_debug);
#endif
}

static s32 pm_wakeup_icc_msg(u32 id, u32 len, void *context)
{ /*lint --e{715} suppress context not referenced*/
    s32 ret;
    struct debug_pm_s read_data = {0};

    /* 不可以通过icc发送非法长度的消息过来(包括长度为0) */
    if (len != (u32)sizeof(read_data)) {
        pmom_pr_err("icc packet len(%d) != sizeof(struct debug_pm_s)(%d)\n", len, (u32)sizeof(read_data));
        return -1;
    }

    ret = bsp_icc_read(id, (u8 *)&read_data, len);
    if (ret != sizeof(read_data)) {
        pmom_pr_err("readed len(%d) != icc packet len(%u)", ret, len);
        return -1;
    }
    if (PM_WAKEUP == read_data.wake_type) {
        debug_pm_wake_lock();
    } else if (PM_WAKEUP_THEN_SLEEP == read_data.wake_type) {
        debug_pm_wake_unlock();
    }
    return 0;
}

void pm_wakeup_init(void)
{
    int ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
    g_pmom_debug.wakelock_debug = wakeup_source_register(NULL, "cp_pm_wakeup");
    if (g_pmom_debug.wakelock_debug == NULL) {
        pmom_pr_err("wakeup_source_register cp_pm_wakeup fail!\n");
        return;
    }
#else
    wakeup_source_init(&g_pmom_debug.wakelock_debug, "cp_pm_wakeup");
#endif

    g_pmom_debug.pm.notifier_call = cp_pm_notify;
    register_pm_notifier(&g_pmom_debug.pm);
    g_pmom_debug.cdrx_dump_addr = NULL;

    /* 即使注册icc失败(有错误打印),只影响调测,不影响功能 */
    ret = bsp_icc_event_register((ICC_CHN_IFC << SHITF_SHORT) | IFC_RECV_FUNC_WAKEUP,
                                 (read_cb_func)pm_wakeup_icc_msg, NULL, NULL, NULL);
    if (ret) {
        pmom_pr_err("bsp_icc_event_register IFC_RECV_FUNC_WAKEUP fail! %d.\n", ret);
        return;
    }
    ret = bsp_icc_event_register((ICC_CHN_MCORE_ACORE << SHITF_SHORT) | MCORE_ACORE_FUNC_WAKEUP,
                                 (read_cb_func)pm_wakeup_icc_msg, NULL, NULL, NULL);
    if (ret) {
        pmom_pr_err("bsp_icc_event_register MCORE_ACORE_FUNC_WAKEUP fail! %d.\n", ret);
        return;
    }
}

void pm_om_help(void)
{
    pmom_print("***********************\n");
    pmom_print("pm_om_log_show\n");
    pmom_print("pm_om_debug_show\n");
    pmom_print("pm_om_dbg_on\n");
    pmom_print("pm_om_dbg_off\n");
    pmom_print("pm_om_wake_interval_set slice\n");
    pmom_print("pm_om_feature_on\n");
    pmom_print("pm_wakeup_ccore 0|1\n");
    pmom_print("debug_pm_wake_lock\n");
    pmom_print("debug_pm_wake_unlock\n");
    pmom_print("***********************\n");
}

/* supress pclint of list_for_each_entry */
/*lint -save -e144 -e413 -e613 -e826 -e838*/
/* 通知其他核开启log功能 */
void pm_om_notify_other_core(pm_om_icc_data_type data)
{
    s32 ret;
    u32 channel_id;
    struct pm_info_list *pm_info = NULL;
    struct pm_om_ctrl *ctrl = NULL;
    if (g_pmom_debug.ctrl == NULL) {
        return;
    }
    ctrl = g_pmom_debug.ctrl;
    list_for_each_entry(pm_info, &(ctrl->pm_info.list), entry) {
        if ((pm_info != NULL) && (pm_info->cb_func != NULL)) {
            (void)pm_info->cb_func(&pm_info->usr_data);
            pmom_pr_err("mod[%d] is invoked\n", pm_info->usr_data.mod_id);
        }
    }
    if (pm_om_is_icc_channel_ready() == 1) {
        channel_id = PM_OM_ICC_ACORE_CHN_ID;
        ret = bsp_icc_send(ICC_CPU_MODEM, channel_id, (u8 *)&data, (u32)sizeof(data));
        if ((s32)sizeof(data) != ret) {
            goto icc_send_fail;
        }
    } else {
        pm_om_set_icc_channel_pending(1);
    }
#ifdef CONFIG_PM_OM_NR
    if (pm_om_is_nricc_channel_ready() == 1) {
        channel_id = PM_OM_NRICC_ACORE_CHN_ID;
        ret = bsp_icc_send(ICC_CPU_NRCCPU, channel_id, (u8 *)&data, (u32)sizeof(data));
        if ((s32)sizeof(data) != ret) {
            goto icc_send_fail;
        }
    } else {
        pm_om_set_nricc_channel_pending(1);
    }
#endif
    return;

icc_send_fail:
    pmom_pr_err("icc[0x%x] send fail: 0x%x\n", channel_id, ret);
    return;
}

/*lint -restore +e144 +e413 +e613*/
static ssize_t pm_stat_info_get(struct device *dev, struct device_attribute *attr, char *buf)
{ /*lint --e{715} suppress 'dev' & 'attr' not referenced*/
    int len;
    u64 sw_value;
    u32 tmp_low;
    u32 tmp_high;
    struct pm_om_ctrl *ctrl = g_pmom_debug.ctrl;

    sw_value = ctrl->log.smem->nv_cfg.mod_sw_bitmap;
    tmp_low = (u32)(sw_value & DEFAULT_VALUE);
    tmp_high = (u32)((sw_value >> PMOM_SHOW_BUFFER_LEN) & DEFAULT_VALUE);

    len = snprintf_s(buf, (size_t)PMOM_SHOW_BUFFER_LEN, (size_t)PMOM_SHOW_BUFFER_LEN - 1,
                     "0x%08x%08x\n", tmp_high, tmp_low);
    return len;
}

static ssize_t pm_check_and_get_buf(char p[], size_t dest_max, const char *buf, size_t count)
{
    int ret;
    if (count >= dest_max || buf == NULL) {
        pmom_pr_err("input buf larger than maximum value(200) or buf is NULL\n");
        return -EINVAL;
    }

    ret = memset_s(p, dest_max, 0, dest_max);
    if (ret != EOK) {
        pmom_pr_err("p memset ret = %d\n", ret);
        return -1;
    }
    ret = memcpy_s(p, dest_max, buf, count);
    if (ret != EOK) {
        pmom_pr_err("p memcpy ret = %d\n", ret);
        return -1;
    }
    return 0;
}

static ssize_t pm_stat_info_on_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{ /*lint --e{715} suppress 'dev' & 'attr' not referenced*/
    char *endp = NULL;
    struct pm_om_ctrl *ctrl = g_pmom_debug.ctrl;
    struct pm_om_platform *linux_plat = (struct pm_om_platform *)ctrl->platform;
    char *temp = NULL;
    char *pointer = NULL;
    ssize_t mod_id;
    char p[PM_OM_LOG_THRE_BUF_SIZE];
    int i;
    ssize_t ret = pm_check_and_get_buf(p, PM_OM_LOG_THRE_BUF_SIZE, buf, count);
    if (ret != 0) {
        return ret;
    }
    pointer = p;

    for (i = 0; i < PMOM_MOD_ONOFF_BIT_MAX && ((temp = strsep(&pointer, " ")) != NULL); i++) {
        if (*temp == '\0') {
            continue;
        }
        mod_id = simple_strtol(temp, &endp, PMOM_MOD_ID_N_BASE);
        if (temp == endp) {
            pmom_pr_err("input includes Non-number parameters.\n");
            return -EINVAL;
        }
        if ((mod_id > (ssize_t)PM_MOD_BEGIN && mod_id < (ssize_t)PM_MOD_END) || (mod_id == (ssize_t)PM_OM_MDRV)) {
            ctrl->log.init_flag = PM_OM_INIT_MAGIC;
            linux_plat->log_info.mem_is_ok = 1;
            pm_om_log_mod_on(&(ctrl->log.smem->nv_cfg.mod_sw_bitmap), (u32)mod_id);
        } else {
            pmom_pr_err("input number is invalid\n");
            return -EINVAL;
        }
    }

    pm_om_notify_other_core(1);
    return (ssize_t)count;
}

static ssize_t pm_stat_info_off_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{ /*lint --e{715} suppress 'dev' & 'attr' not referenced*/
    char *endp = NULL;
    struct pm_om_ctrl *ctrl = g_pmom_debug.ctrl;
    char p[PM_OM_LOG_THRE_BUF_SIZE];
    char *tempp = NULL;
    ssize_t mod_id, ret;
    int i;
    char *pointer = NULL;

    ret = pm_check_and_get_buf(p, PM_OM_LOG_THRE_BUF_SIZE, buf, count);
    if (ret != 0) {
        return ret;
    }
    pointer = p;

    for (i = 0; i < PMOM_MOD_ONOFF_BIT_MAX && ((tempp = strsep(&pointer, " ")) != NULL); i++) {
        if (*tempp == '\0') {
            continue;
        }
        mod_id = simple_strtol(tempp, &endp, PMOM_MOD_ID_N_BASE);
        if (tempp == endp) {
            pmom_pr_err("input includes Non-number parameters.\n");
            return -EINVAL;
        }
        if ((mod_id > (ssize_t)PM_MOD_BEGIN && mod_id < (ssize_t)PM_MOD_END) || (mod_id == (ssize_t)PM_OM_MDRV)) {
            ctrl->log.init_flag = 0;
            pm_om_log_mod_off(&(ctrl->log.smem->nv_cfg.mod_sw_bitmap), (u32)mod_id);
        } else {
            pmom_pr_err("input number is invalid\n");
            return -EINVAL;
        }
    }

    pm_om_notify_other_core(1);
    return (ssize_t)count;
}

/*lint -save -e846 -e514 -e778 -e866 -e84 suppress DEVICE_ATTR warning*/
static DEVICE_ATTR(mod_on, 0600, pm_stat_info_get, pm_stat_info_on_set);
static DEVICE_ATTR(mod_off, 0600, pm_stat_info_get, pm_stat_info_off_set);
/*lint -restore +e846 +e514 +e778 +e866 +e84*/
static struct attribute *g_pm_stat_info_on_attributes[] = { &dev_attr_mod_on.attr, NULL };
/* lint -save -e785 */ /* suppress too few initializers for struct */
static const struct attribute_group g_pm_stat_info_on_group = {
    .attrs = g_pm_stat_info_on_attributes,
};

static struct attribute *g_pm_stat_info_off_attributes[] = { &dev_attr_mod_off.attr, NULL };

static const struct attribute_group g_pm_stat_info_off_group = {
    .attrs = g_pm_stat_info_off_attributes,
};

static int pm_info_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct pm_om_ctrl *ctrl = pm_om_ctrl_get();
    if (ctrl->log.smem->nv_cfg.mod_sw_bitmap) {
        ret += device_create_file(&(pdev->dev), &dev_attr_mod_on);
        ret += device_create_file(&(pdev->dev), &dev_attr_mod_off);
    }
    if (ret) {
        printk("fail to creat balong modem pm info sysfs\n");
        return ret;
    }

    return ret;
}

static struct platform_device g_balong_modem_pm_info_device = {
    .name = "balong_modem_pm_info",
    .id = 0,
    .dev = {
        .init_name = "balong_modem_pm_info",
    },
};

static struct platform_driver g_balong_modem_pm_info_drv = {
    .probe      = pm_info_probe,
    .driver     = {
        .name     = "balong_modem_pm_info",
        .owner    = THIS_MODULE,
    },
}; /* lint -restore +e785 */

int balong_modem_pm_info_init(void)
{
    int ret;

    ret = platform_device_register(&g_balong_modem_pm_info_device);
    if (ret) {
        printk("register balong modem info device fail\n");
        return ret;
    }

    ret = platform_driver_register(&g_balong_modem_pm_info_drv);
    if (ret) {
        printk("register balong modem info driver fail\n");
        platform_device_unregister(&g_balong_modem_pm_info_device);
    }

    return ret;
}

void balong_modem_pm_info_exit(void)
{
    platform_driver_unregister(&g_balong_modem_pm_info_drv);
    platform_device_unregister(&g_balong_modem_pm_info_device);
}

int pm_om_debug_init(void)
{
    int ret;
    unsigned long flags;
    struct pm_om_ctrl *ctrl = pm_om_ctrl_get();
    struct pm_om_platform *platform = NULL;
    platform = (struct pm_om_platform *)ctrl->platform;
    if (ctrl->log.smem->nv_cfg.mod_sw_bitmap) {
        if (PM_OM_PROT_MAGIC1 == ctrl->log.smem->mem_info.magic) {
            pm_om_spin_lock(&ctrl->log.lock, flags); /*lint !e550: (Warning -- Symbol '__dummy' not accessed)*/
            ctrl->log.smem->mem_info.magic = PM_OM_PROT_MAGIC2;
            pm_om_spin_unlock(&ctrl->log.lock, flags);
            if (platform != NULL) {
                platform->log_info.mem_is_ok = 1;
            }
            ctrl->log.init_flag = PM_OM_INIT_MAGIC;
        } else if (PM_OM_PROT_MAGIC2 == ctrl->log.smem->mem_info.magic) {
            ctrl->log.init_flag = PM_OM_INIT_MAGIC;
            pm_om_spin_lock(&ctrl->log.lock, flags); /*lint !e550: (Warning -- Symbol '__dummy' not accessed)*/
            if (platform != NULL) {
                platform->log_info.mem_is_ok = 1;
            }
            pm_om_spin_unlock(&ctrl->log.lock, flags);
        }
    }
    ret = memset_s((void *)&g_pmom_debug, sizeof(g_pmom_debug), 0, sizeof(g_pmom_debug));
    if (ret) {
        pmom_pr_err("g_pmom_debug memset ret = %d\n", ret);
    }
    g_pmom_debug.stat.waket_prev = bsp_get_slice_value();
    g_pmom_debug.stat.waket_min = DEFAULT_VALUE;
    g_pmom_debug.stat.logt_print_sw = ctrl->log.smem->nv_cfg.reserved;
    ret = balong_modem_pm_info_init();
    if (ret) {
        printk("balong modem info driver init fail\n");
    }
    g_pmom_debug.ctrl = ctrl;
    ctrl->debug = (void *)&g_pmom_debug;
    pm_wakeup_init();
    nr_wakeup_irq_debug_init();
    hac_dump_init();
    return 0;
}

EXPORT_SYMBOL(pm_om_notify_other_core);
EXPORT_SYMBOL(pm_wakeup_ccore);
