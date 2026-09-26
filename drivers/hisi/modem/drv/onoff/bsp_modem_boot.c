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

/*lint --e{528,537,715} */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <osl_spinlock.h>
#include <osl_sem.h>
#include "mdrv_errno.h"
#include <bsp_om_enum.h>
#include <bsp_reset.h>
#include <bsp_dump.h>
#include <bsp_onoff.h>
#include <gunas_errno.h>
#include <bsp_slice.h>
#include <linux/ctype.h>
#include "bsp_llt.h"
#include <securec.h>
#include "mdrv_vcom.h"
#include <bsp_icc.h>
#include <bsp_print.h>
#include <bsp_reset.h>
#include <hi_sysboot.h>
#include "bsp_modem_boot.h"
#define THIS_MODU mod_onoff
struct balong_power_plat_data {
    u32 modem_state;
};

#define MODEM_STATE_FILE_NAME "modemstatus"
#define MODEM_STATE_FILE_PERMISSION 0440

static struct balong_power_plat_data *g_balong_driver_plat_data = NULL;
static const char *g_modem_state_str[] = {
    "MODEM_STATE_OFF\n",
    "MODEM_STATE_READY\n",
    "MODEM_STATE_INIT_OK\n",
    "MODEM_STATE_INVALID\n",
};

/*
the following files need to be modified at the same time when modify MODEM_ID_MAX
*             : /ccore/drivers/onoff/bsp_onoff.c
*/
#define MODEM_ID_MAX 32
static struct balong_power_plat_data g_balong_modem_state[MODEM_ID_MAX];

/* To make modem poweroff called only once when there are two rilds. */
static int g_modem_power_off_flag = 0;
static int g_modem_power_on_flag = 0;
spinlock_t g_modem_power_spinlock;
osl_sem_id g_modem_status_sem;

void bsp_drv_set_modem_state(unsigned int modem_id, unsigned int state)
{
    return;
}

/*
 * 功能描述: 提供ttf设置modem状态的API
 */
int mdrv_set_modem_state(unsigned int state)
{
    if (g_balong_driver_plat_data == NULL) {
        bsp_err("Balong_power %s:%d not init.\n", __FUNCTION__, __LINE__);
        return -EINVAL;
    }
    if (state >= MODEM_INVALID) {
        bsp_err("Balong_power %s:%d invalid state 0x%x.\n", __FUNCTION__, __LINE__, state);
        return -EINVAL;
    }

    osl_sem_down(&g_modem_status_sem);
    g_balong_driver_plat_data->modem_state = state;
    osl_sem_up(&g_modem_status_sem);

    if (g_balong_driver_plat_data->modem_state == MODEM_READY) {
        bsp_err("Balong_power set state %d ,time slice %d\n", state, bsp_get_elapse_ms());
    } else {
        bsp_err("Balong_power set state %d\n", state);
    }

    return 0;
}

static ssize_t balong_power_get(struct device *dev, struct device_attribute *attr, char *buf)
{
    int ret;
    unsigned int state;

    if (g_balong_driver_plat_data == NULL) {
        bsp_err("Balong_power %s:%d not init.\n", __FUNCTION__, __LINE__);
        return 0;
    }
    if (g_balong_driver_plat_data->modem_state >= MODEM_INVALID) {
        bsp_err("Balong_power : %s:%d Invalid state 0x%x now is set.\n", __FUNCTION__, __LINE__,
            g_balong_driver_plat_data->modem_state);
        return 0;
    }

    osl_sem_down(&g_modem_status_sem);
    state = g_balong_driver_plat_data->modem_state;
    osl_sem_up(&g_modem_status_sem);

    ret = strncpy_s(buf, PAGE_SIZE, g_modem_state_str[state], strlen(g_modem_state_str[state]));
    if (ret != EOK) {
        bsp_err("Balong_power %s:strncpy_s fail.\n", __FUNCTION__);
        return -1;
    }

    return strlen(g_modem_state_str[state]);
}
EXPORT_SYMBOL_GPL(balong_power_get);

static void modem_reset(u32 mod_id)
{
    if (!is_in_llt()) {
        system_error(mod_id, 0, 0, NULL, 0);
    }
}

static void modem_reset_handle(long state)
{
    unsigned long lock_flag;

    spin_lock_irqsave(&g_modem_power_spinlock, lock_flag); /*lint !e550*/
    if ((g_modem_power_off_flag) && (!g_modem_power_on_flag) && (state != BALONG_MODEM_ON)) {
        bsp_err("modem has been power off,please power on,don't reset!\n");
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        return;
    }

    /* 切卡 */
    if (state == BALONG_MODEM_RESET) {
        bsp_err("modem reset %d\n", BALONG_MODEM_RESET);
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        modem_reset(DRV_ERRNO_RESET_SIM_SWITCH);
        return;
    } else if (state == BALONG_MODEM_OFF) {
        /* To make modem poweroff called only once when there are two rilds. */
        if (g_modem_power_off_flag) {
            bsp_err("Balong_power: modem power off has been called! \n");
            spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
            return;
        }
        bsp_modem_power_off();
        bsp_err("modem power off %d\n", BALONG_MODEM_OFF);
        g_modem_power_off_flag = 1;
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        return;
    } else if (state == BALONG_MODEM_ON) { /* 是否需要上电，根HIFI什么关系 */
        if ((g_modem_power_off_flag) && (!g_modem_power_on_flag)) {
            bsp_modem_power_on();
            g_modem_power_on_flag = 1;
            bsp_err("modem power on %d\n", BALONG_MODEM_ON);
        } else {
            bsp_err("modem now is power on!\n");
        }
    } else if (state == BALONG_MODEM_RILD_SYS_ERR) {
        bsp_err("modem reset using system_error by rild %d\n", BALONG_MODEM_RILD_SYS_ERR);
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        modem_reset(NAS_REBOOT_MOD_ID_RILD);
        return;
    } else if (state == BALONG_MODEM_3RD_SYS_ERR) {
        bsp_err("modem reset using system_error by 3rd modem %d\n", DRV_ERRNO_RESET_3RD_MODEM);
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        modem_reset(DRV_ERRNO_RESET_3RD_MODEM);
        return;
    } else {
        bsp_err("Balong_power : invalid code to balong power !!!!\n");
        spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
        return;
    }
    spin_unlock_irqrestore(&g_modem_power_spinlock, lock_flag);
    return;
}

ssize_t modem_reset_set(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    long state;

    if (count > 3U || count <= 1U) {
        bsp_err("buf len err: %d\n", (int)count);
        return -EINVAL;
    }
    if (count == 2U && (!isdigit(buf[0]))) {
        bsp_err("count = 2,buf err: %c\n", buf[0]);
        return -EINVAL;
    }
    if (count == 3U && (!isdigit(buf[0]) || (!isdigit(buf[0x1])))) {
        bsp_err("count = 3,buf err: %c%c\n", buf[0], buf[0x1]);
        return -EINVAL;
    }
    if (count == 2U) {
        bsp_info("Power set to %c\n", buf[0]);
    } else if (count == 3U) {
        bsp_info("Power set to %c%c\n", buf[0], buf[0x1]);
    }

    state = (count == 2U) ? (buf[0] - '0') : (((buf[0] - '0') * 10U) + (buf[0x1] - '0'));
    bsp_err("count = %lu\n", (unsigned long)count);

    /* 整机复位对rild为桩,应该整机复位 */
    if (!bsp_reset_is_connect_ril()) {
        bsp_err("<modem_reset_set>: modem reset not to be connected to ril\n");
        modem_reset(DRV_ERRNO_RESET_REBOOT_REQ);
        return (ssize_t)count;
    }

    modem_reset_handle(state);

    return (ssize_t)count;
}
EXPORT_SYMBOL_GPL(modem_reset_set);

ssize_t modem_state_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    return (ssize_t)count;
}
EXPORT_SYMBOL_GPL(modem_state_write);
/*lint -save -e* */
static DEVICE_ATTR(state, 0660, balong_power_get, modem_reset_set);
static DEVICE_ATTR(modem_state, 0660, balong_power_get, modem_state_write); /*lint -restore */


/*
 * Read function of virtual file.
 * @filp: file handle
 * @user_buf : the user space buffer to write to
 * @count: the size of the user space buffer
 * @ppos: the current position in the buffer
 *
 * On success, the number of bytes read is returned, or negative value is
 * returned on error.
 */
static ssize_t modem_status_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    unsigned int modem_id;
    unsigned char modem_state[MODEM_ID_MAX] = {0};
    ssize_t ret;

    if (*ppos > 0) {
        return 0;
    }

    if (count >= MODEM_ID_MAX) {
        bsp_err("count %d is larger than %d", (int)count, MODEM_ID_MAX);
        count = MODEM_ID_MAX;
    }

    osl_sem_down(&g_modem_status_sem);
    for (modem_id = 0; modem_id < MODEM_ID_MAX; modem_id++) {
        if (g_balong_modem_state[modem_id].modem_state != MODEM_READY) {
            modem_state[modem_id] = (unsigned char)MODEM_NOT_READY;
        } else {
            modem_state[modem_id] = (unsigned char)MODEM_READY;
        }
    }
    osl_sem_up(&g_modem_status_sem);

    ret = simple_read_from_buffer(user_buf, count, ppos, modem_state, count);
    if (ret < 0) {
        bsp_err("read state error, return:%d\n", (int)ret);
    }

    return ret;
}

static const struct file_operations g_modem_status_file_ops = {
    .owner = THIS_MODULE,
    .read = modem_status_read,
};

static int bsp_modem_status_file_init(void)
{
    return 0;
}

int onoff_modem_reset_cb(drv_reset_cb_moment_e param, int user_data)
{
    return 0;
}

static int bsp_power_probe(struct platform_device *pdev)
{
    u32 ret;

    g_balong_driver_plat_data = pdev->dev.platform_data;

    ret = (u32)device_create_file(&(pdev->dev), &dev_attr_state);
    ret |= (u32)device_create_file(&(pdev->dev), &dev_attr_modem_state);
    if (ret) {
        bsp_err("fail to create modem boot sysfs\n");
        return ret;
    }

    ret = bsp_modem_status_file_init();
    if (ret) {
        bsp_err("fail to create modem boot proc file\n");
    }

    ret = (u32)bsp_reset_cb_func_register("ONOFF", onoff_modem_reset_cb, 0, DRV_RESET_CB_PIOR_ALL);
    if (ret) {
        bsp_err(" register cp reset cb failed, ret = 0x%x\n", ret);
    }

    return (int)ret;
}

static struct balong_power_plat_data g_balong_power_plat_data = {
    .modem_state = MODEM_NOT_READY,
};

static struct platform_device g_balong_power_device = {
    .name = "balong_power",
    .id = -1,
    .dev = {
        .platform_data = &g_balong_power_plat_data,
    }, /* lint !e785 */
}; /* lint !e785 */

static struct platform_driver g_balong_power_drv = {
    .probe      = bsp_power_probe,
    .driver     = {
        .name     = "balong_power",
        .owner    = THIS_MODULE, /* lint !e64 */
        .probe_type = PROBE_FORCE_SYNCHRONOUS,
    }, /* lint !e785 */
}; /* lint !e785 */

int bsp_modem_boot_init(void)
{
    int ret;

    ret = platform_device_register(&g_balong_power_device);
    if (ret) {
        bsp_err("register his_modem boot device failed.\n");
        return ret;
    }
    spin_lock_init(&g_modem_power_spinlock);
    osl_sem_init(1, &g_modem_status_sem);

    ret = platform_driver_register(&g_balong_power_drv); /*lint !e64*/
    if (ret) {
        bsp_err("register his_modem boot driver failed.\n");
        platform_device_unregister(&g_balong_power_device);
    }

    return ret;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(bsp_modem_boot_init);
#endif
