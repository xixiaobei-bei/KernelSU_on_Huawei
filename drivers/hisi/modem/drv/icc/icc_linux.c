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

#include <bsp_dump.h>
#include "icc_core.h"
#include <securec.h>
#include <linux/of_device.h>

#include <linux/version.h>
#include <linux/dma-mapping.h>
#include <linux/syscore_ops.h>
#undef THIS_MODU
#define THIS_MODU mod_icc
extern struct icc_control g_icc_ctrl;
extern u32 g_iccchannel_recvtimes[ICC_CHN_ID_MAX];
extern u32 g_iccchannel_sendtimes[ICC_CHN_ID_MAX];

void icc_restore_recv_channel_flag(struct icc_channel_fifo *channel_fifo)
{
    /* nothing to do */
    return;
}

s32 icc_channel_has_data(void)
{
    /*lint --e{53, 58, 56 } */
    s32 i;
    u32 read = 0;
    u32 write = 0;
    struct icc_channel_fifo *fifo = NULL;
    struct icc_channel_packet packet = { 0 };

    for (i = 0; i < (int)ICC_CHN_ID_MAX; i++) {
        if (g_icc_ctrl.channels[i] == NULL || i == ICC_CHN_APLUSB_IFC || i == ICC_CHN_APLUSB_PCIE) {
            continue;
        }
        fifo = g_icc_ctrl.channels[i]->fifo_recv;
        read = fifo->read;
        write = fifo->write;
        if (read != write) { /* has data to be handled */
            icc_print_error("%s[channel-%d] stop suspend flow\n", g_icc_ctrl.channels[i]->name, i);
            if (i == ICC_CHN_IFC || i == ICC_CHN_MCORE_ACORE || i == ICC_CHN_MCORE_CCORE) {
                while (read < write) {
                    if (fifo_get(fifo, ((u32)i << 0x10), (u8 *)&packet, sizeof(packet), &read) != sizeof(packet)) {
                        break;
                    }
                    icc_sleep_flag_set();
                    if (icc_channel_packet_dump(&packet) != ICC_OK) {
                        break;
                    }
                    read = (read + packet.len);
                    read = (read >= fifo->size) ? (read - fifo->size) : (read);
                }
            }
            __pm_stay_awake(&g_icc_ctrl.wake_lock);
            if (!g_icc_ctrl.channels[i]->mode.union_stru.no_task) {
                if (g_icc_ctrl.channels[i]->mode.union_stru.task_shared)
                    osl_sem_up(&g_icc_ctrl.shared_task_sem);
                else
                    osl_sem_up(&(g_icc_ctrl.channels[i]->private_task_sem)); /*lint !e661 */
            }
            return ICC_BUSY; /*lint !e454 */
        }
    }

    return ICC_OK;
}
/* =====================BEGIN===================== */

void icc_shared_sem_init(void)
{
    osl_sem_init(ICC_SEM_FULL, &g_icc_ctrl.shared_task_sem);
}

void icc_private_sem_init(osl_sem_id *private_sem)
{
    osl_sem_init(ICC_SEM_FULL, private_sem);
}

int icc_shared_task_init(void)
{
    return osl_task_init("icc_shared", ICC_TASK_SHARED_PRI, ICC_TASK_STK_SIZE, icc_task_shared_func,
        NULL,                                /*lint !e611 */
        (void *)&g_icc_ctrl.shared_task_id); /*lint !e611 */
}

void icc_system_error(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length)
{
    system_error(mod_id, arg1, arg2, data, length);
}

int icc_pm_notify(struct notifier_block *nb, unsigned long event, void *dummy)
{
    if (event == PM_SUSPEND_PREPARE) {
        return icc_channel_has_data();
    }

    return ICC_OK;
}

static s32 icc_pm_notify_init(void)
{
    errno_t err;
    err = memset_s(&g_icc_ctrl.pm_notify, sizeof(g_icc_ctrl.pm_notify), 0, sizeof(g_icc_ctrl.pm_notify));
    if (err != EOK) {
        return ICC_ERR;
    }
    g_icc_ctrl.pm_notify.notifier_call = icc_pm_notify;
    register_pm_notifier(&g_icc_ctrl.pm_notify);
    return ICC_OK;
}

void icc_wake_lock_init(struct wakeup_source *lock, const char *name)
{
    wakeup_source_init(lock, name);
}
void icc_wake_lock(struct wakeup_source *lock)
{
    __pm_stay_awake(lock); /*lint !e454 */
} /*lint !e454 */
void icc_wake_unlock(struct wakeup_source *lock)
{
    __pm_relax(lock); /*lint !e455 */
}

extern s32 bsp_reset_ccpu_status_get(void);

int icc_ccore_is_reseting(u32 cpuid)
{
    if ((bsp_reset_ccpu_status_get() == 0) && ((cpuid == ICC_CPU_MODEM) || (cpuid == ICC_CPU_NRCCPU))) {
        return 1;
    }

    return ICC_OK;
}

s32 bsp_icc_channel_reset(drv_reset_cb_moment_e stage, int usrdata)
{
    struct icc_channel *channel = NULL;
    u32 i;
    if (stage == MDRV_RESET_RESETTING) {
        for (i = ICC_CHN_ACORE_CCORE_MIN; i < ICC_CHN_ACORE_CCORE_MAX; i++) {
            channel = g_icc_ctrl.channels[i];
            if (channel == NULL) {
                continue;
            }
            channel->fifo_send->read = 0;
            channel->fifo_send->write = 0;
            channel->fifo_recv->read = 0;
            channel->fifo_recv->write = 0;
        }
        // 新增Acore 和NRcore 之间的通道处理
        for (i = ICC_CHN_IQI; i <= ICC_STATIC_CHN_ID_MAX; i++) {
            if ((i == ICC_CHN_APLUSB_IFC) || (i == ICC_CHN_APLUSB_PCIE) || (i == ICC_CHN_ACORE_HIFI)) {
                continue;
            }
            if (g_icc_ctrl.channels[i]) {
                // 新增的IQI 通道单独处理
                g_icc_ctrl.channels[i]->fifo_send->read = 0;
                g_icc_ctrl.channels[i]->fifo_send->write = 0;
                g_icc_ctrl.channels[i]->fifo_recv->read = 0;
                g_icc_ctrl.channels[i]->fifo_recv->write = 0;
            }
        }
    }
    return ICC_OK;
}

void icc_wakeup_flag_set(void)
{
    g_icc_ctrl.wake_up_flag = 1;
}

void icc_sleep_flag_set(void)
{
    g_icc_ctrl.sleep_flag = 1;
}

u32 bsp_icc_channel_status_get(u32 real_channel_id, u32 *channel_stat)
{
    struct icc_channel *channel = g_icc_ctrl.channels[real_channel_id];

    if (channel == NULL) {
        *channel_stat = ICC_CHN_CLOSED;
        return *channel_stat;
    }

    *channel_stat = channel->state;
    return *channel_stat;
}

int icc_pm_init(void)
{
    if (ICC_OK != icc_pm_notify_init()) {
        return ICC_ERR;
    }
    return ICC_OK;
}

EXPORT_SYMBOL(bsp_icc_channel_status_get);
EXPORT_SYMBOL(icc_channel_has_data);
