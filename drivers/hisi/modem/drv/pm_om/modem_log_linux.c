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
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/suspend.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/err.h>
#include <osl_generic.h>
#include <osl_thread.h>
#include <bsp_slice.h>
#include <bsp_ring_buffer.h>
#include "securec.h"
#include "modem_log_linux.h"

#undef THIS_MODU
#define THIS_MODU mod_pmom

static LIST_HEAD(g_modem_log_list);
struct modem_log g_modem_log;

/*lint --e{64,826,785,528}*/
/* 64&826:list_for_each_entry, 785 Too few initializers for struct define; 528: module_init */
static struct logger_log *get_modem_log_from_name(const char *name)
{
    struct logger_log *log = NULL;

    list_for_each_entry(log, &g_modem_log_list, logs) {
        if (!strncmp(log->misc.name, name, MODEM_LOG_DEV_NAME_LEN)) {
            return log;
        }
    }

    return NULL;
}

struct logger_log *get_modem_log_from_minor(int minor)
{
    struct logger_log *log = NULL;

    list_for_each_entry(log, &g_modem_log_list, logs)
    {
        if (log->misc.minor == minor) {
            return log;
        }
    }

    return NULL;
}

void modem_log_ring_buffer_get(struct log_usr_info *usr_info, struct ring_buffer *rb)
{
    int ret;
    if ((usr_info != NULL) && (usr_info->mem != NULL)) {
        rb->buf = usr_info->ring_buf;
        rb->read = usr_info->mem->read;
        rb->write = usr_info->mem->write;
        rb->size = usr_info->mem->size;
    } else {
        ret = memset_s((void *)rb, sizeof(struct ring_buffer), 0, sizeof(struct ring_buffer));
        if (ret) {
            modem_log_pr_err("rb memset ret = %d\n", ret);
        }
    }
}

static unsigned int modem_log_poll(struct file *file, poll_table *wait)
{
    struct logger_log *log = NULL;
    unsigned int ret = 0;
    u32 read = 0;
    u32 write = 0;

    modem_log_pr_debug("%s entry\n", __func__);
    if (!(file->f_mode & FMODE_READ)) {
        return ret;
    }

    log = file->private_data;
    if (unlikely(log == NULL)) { /*lint !e730: (Info -- Boolean argument to function)*/
        modem_log_pr_err("device poll fail\n");
        return -ENODEV;
    }
    if (log->usr_info != NULL && log->usr_info->mem != NULL) {
        read = log->usr_info->mem->read;
        write = log->usr_info->mem->write;
    }

    mutex_lock(&log->mutex);
    if (read != write) {
        ret = POLLIN | POLLRDNORM;
    }
    mutex_unlock(&log->mutex);

    if (!ret) {
        poll_wait(file, &log->wq, wait);
    }

    modem_log_pr_debug("%s exit\n", __func__);

    return ret;
}

static ssize_t modem_log_cpy_rb_to_user(struct logger_log *log, struct ring_buffer *rb, char __user *buf, size_t count)
{
    size_t len, left, cpy_len;
    ssize_t ret = 0;
    if (buf == NULL) {
        modem_log_pr_err("modem_log_cpy_rb_to_user buf is NULL\n");
        return -1;
    }
    mutex_lock(&log->mutex);
    len = bsp_ring_buffer_readable_size(rb);
    len = min(len, count);
    if (len == 0) {
        goto out;
    }

    left = min(len, (size_t)(rb->size - rb->read));
    cpy_len = copy_to_user(buf, (rb->buf + rb->read), left);
    if (cpy_len != 0) {
        modem_log_pr_err("copy_to_user err\n");
        ret = -1;
        goto out;
    }

    if ((len > left) && ((len - left) < count) && ((buf + left) != NULL)) {
        cpy_len = copy_to_user((buf + left), rb->buf, (len - left));
        if (cpy_len != 0) {
            modem_log_pr_err("copy_to_user left err\n");
            ret = -1;
            goto out;
        }
    }

    rb->read += (u32)len;
    rb->read %= (rb->size);

    ret = (ssize_t)len;
    log->usr_info->mem->read = rb->read;
out:
    mutex_unlock(&log->mutex);
    return ret;
}

/*
 * modem_log_read - our log's read() method
 * 1) O_NONBLOCK works
 * 2) If there are no log to read, blocks until log is written to
 *  3) Will set errno to EINVAL if read
 */
static ssize_t modem_log_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{ /*lint --e{715} suppress pos not referenced*/
    struct logger_log *log = (struct logger_log *)file->private_data;
    struct ring_buffer rb = { 0 };
    ssize_t ret = 1;
    DEFINE_WAIT(wait); /*lint !e446 (Warning -- side effect in initializer)*/

    if (unlikely(log == NULL)) { /*lint !e730: (Info -- Boolean argument to function)*/
        modem_log_pr_err("device read fail\n");
        return -ENODEV;
    }

    while (1) {
        mutex_lock(&log->mutex);

        prepare_to_wait(&log->wq, &wait, TASK_INTERRUPTIBLE);

        if (!log->usr_info->mem_is_ok) {
            goto skip_read;
        }

        modem_log_ring_buffer_get(log->usr_info, &rb);

        if (log->usr_info->read_func) {
            ret = log->usr_info->read_func(log->usr_info, buf, (u32)count);
            if (ret) {
                mutex_unlock(&log->mutex);
                modem_log_pr_err_once("[modem log]user(%s) read method invoked\n", log->usr_info->dev_name);
                break;
            }
        }

        ret = is_ring_buffer_empty(&rb);
    skip_read:
        mutex_unlock(&log->mutex);
        if (!ret) /* has data to be read, break out */
        {
            break;
        }
        if (file->f_flags & O_NONBLOCK) {
            modem_log_pr_err("try again, when read file flag is O_NONBLOCK\n");
            ret = -EAGAIN;
            break;
        }

        if (signal_pending(current)) {
            ret = -EINTR;
            break;
        }

        __pm_relax(&g_modem_log.wake_lock); /*lint !e455*/
        schedule();
    }

    finish_wait(&log->wq, &wait);
    if (ret) {
        return ret;
    }

    return modem_log_cpy_rb_to_user(log, &rb, buf, count);
}

/*
 * modem_log_open - our log's open() method, not support open twice by same application
 */
static int modem_log_open(struct inode *inode, struct file *file)
{
    struct logger_log *log = get_modem_log_from_minor(MINOR(inode->i_rdev));
    if (unlikely(log == NULL)) { /*lint !e730: (Info -- Boolean argument to function)*/
        modem_log_pr_err("device get fail\n");
        return -ENODEV;
    }

    file->private_data = log;

    mutex_lock(&log->mutex);
    if (log->usr_info->open_func && log->usr_info->open_func(log->usr_info)) {
        modem_log_pr_err("user(%s) open method error, exit\n", log->usr_info->dev_name);
        mutex_unlock(&log->mutex);
        return -EINVAL;
    }

    if (log->usr_info->mem != NULL) {
        log->usr_info->mem->app_is_active = 1; /* reader is ready  */
    }

    log->fopen_cnt++;

    mutex_unlock(&log->mutex);

    /* indicate that application exit exceptionally, need to clear ring buffer */
    if (log->fopen_cnt > 1) {
        modem_log_pr_err("file_open_cnt=%d\n", log->fopen_cnt);
        mutex_lock(&log->mutex);
        if (log->usr_info->mem != NULL) {
            log->usr_info->mem->read = log->usr_info->mem->write;
        }
        mutex_unlock(&log->mutex);
    }

    modem_log_pr_debug("%s entry\n", __func__);

    return 0;
}

/*
 * modem_log_release - our log's close() method
 */
static int modem_log_release(struct inode *inode, struct file *file)
{ /*lint --e{715} suppress inode not referenced*/
    struct logger_log *log = file->private_data;

    if (unlikely(log == NULL)) { /*lint !e730: (Info -- Boolean argument to function)*/
        modem_log_pr_err("device release fail\n");
        return -ENODEV;
    }

    mutex_lock(&log->mutex);
    if (log->usr_info->close_func && log->usr_info->close_func(log->usr_info)) {
        modem_log_pr_err("user(%s) release method error, exit\n", log->usr_info->dev_name);
    }
    if (log->usr_info->mem != NULL) {
        log->usr_info->mem->app_is_active = 0; /* reader cannot work  */
    }
    mutex_unlock(&log->mutex);

    __pm_relax(&g_modem_log.wake_lock); /*lint !e455*/

    modem_log_pr_debug("%s entry\n", __func__);
    return 0;
}

static const struct file_operations g_modem_log_fops = {
    .read = modem_log_read,
    .poll = modem_log_poll,
    .open = modem_log_open,
    .release = modem_log_release,
};

/*
 * modem_log_wakeup_all - wake up all waitquque
 */
void modem_log_wakeup_all(void)
{
    struct logger_log *log = NULL;
    list_for_each_entry(log, &g_modem_log_list, logs)
    {
        if (log->usr_info->mem != NULL && log->usr_info->mem->read != log->usr_info->mem->write) {
            __pm_wakeup_event(&g_modem_log.wake_lock, jiffies_to_msecs(HZ));
            if ((&log->wq) != NULL) {
                wake_up_interruptible(&log->wq);
            }
        }
    }
}
void bsp_modem_log_wakeup_one(struct log_usr_info *usr_info)
{
    struct logger_log *log = NULL;

    if (usr_info == NULL) {
        return;
    }
    log = get_modem_log_from_name(usr_info->dev_name);
    if (log == NULL) {
        return;
    }

    /* if reader is not ready, no need to wakeup waitqueue */
    if (usr_info->mem != NULL && usr_info->mem->app_is_active == 1 && (usr_info->mem->read != usr_info->mem->write)) {
        __pm_wakeup_event(&g_modem_log.wake_lock, jiffies_to_msecs(HZ));
        if ((&log->wq) != NULL) {
            wake_up_interruptible(&log->wq);
        }
    }
}
int bsp_modem_log_mem_init(void)
{
    struct logger_log *log = NULL;
    list_for_each_entry(log, &g_modem_log_list, logs)
    {
        if (log->fopen_cnt && log->usr_info != NULL && log->usr_info->mem == NULL) {
            if (log->usr_info->open_func && log->usr_info->open_func(log->usr_info)) {
                return -EINVAL;
            }
            if (log->usr_info->mem != NULL) {
                log->usr_info->mem->app_is_active = 1;
            }
        }
    }
    return 0;
}

/*
 * modem_log_notify - modem log pm notify
 */
s32 modem_log_notify(struct notifier_block *notify_block, unsigned long mode, void *unused)
{ /*lint --e{715} suppress notify_block&unused not referenced*/
    struct logger_log *log = NULL;

    modem_log_pr_debug("entry\n");

    switch (mode) {
        case PM_POST_HIBERNATION:
            list_for_each_entry(log, &g_modem_log_list, logs)
            {
                if ((log->usr_info->mem) && (log->usr_info->mem->read != log->usr_info->mem->write) &&
                    (log->usr_info->mem->app_is_active)) {
                    __pm_wakeup_event(&g_modem_log.wake_lock, jiffies_to_msecs(HZ));
                    if ((&log->wq) != NULL) {
                        wake_up_interruptible(&log->wq);
                    }
                }
            }
            break;
        default:
            break;
    }

    return 0;
}

/*
 * bsp_modem_log_register - tell modem log which user information is
 * @usr_info: information which modem log need to know
 * Returns 0 if success
 */
s32 bsp_modem_log_register(struct log_usr_info *usr_info)
{
    s32 ret = 0;
    struct logger_log *log = NULL;

    if (unlikely(usr_info == NULL)) { /*lint !e730: (Info -- Boolean argument to function)*/
        ret = (s32)MODEM_LOG_NO_USR_INFO;
        modem_log_pr_err("no user info to register\n");
        goto out;
    }

    log = kzalloc(sizeof(struct logger_log), GFP_KERNEL);
    if (log == NULL) {
        ret = (s32)MODEM_LOG_NO_MEM;
        goto out;
    }

    log->usr_info = usr_info;

    log->misc.minor = MISC_DYNAMIC_MINOR;
    log->misc.name = kstrdup(usr_info->dev_name, GFP_KERNEL);
    if (unlikely(log->misc.name == NULL)) { /*lint !e730: (Info -- Boolean argument to function)*/
        ret = (s32)MODEM_LOG_NO_MEM;
        goto out_free_log;
    }

    log->misc.fops = &g_modem_log_fops;
    log->misc.parent = NULL;

    init_waitqueue_head(&log->wq);
    mutex_init(&log->mutex);

    INIT_LIST_HEAD(&log->logs);
    list_add_tail(&log->logs, &g_modem_log_list);

    /* finally, initialize the misc device for this log */
    ret = misc_register(&log->misc);
    if (unlikely(ret)) { /*lint !e730: (Info -- Boolean argument to function)*/
        modem_log_pr_err("failed to register misc device for log '%s'!\n", log->misc.name); /*lint !e429*/
        goto out_rm_node;
    }

    modem_log_pr_debug("created log '%s'\n", log->misc.name);
    return 0; /*lint !e429*/
out_rm_node:
    list_del(&log->logs);
    kfree(log->misc.name);
    log->misc.name = NULL;
out_free_log:
    kfree(log);
    log = NULL;
out:
    return ret;
}

/*
 * bsp_modem_log_fwrite_trigger - trigger file write from ring buffer to log record file
 * @usr_info: information regitered to modem log
 */
void bsp_modem_log_fwrite_trigger(struct log_usr_info *usr_info)
{
    struct logger_log *log = NULL;

    if (usr_info == NULL) {
        modem_log_pr_err("no user info\n");
        return;
    }

    log = get_modem_log_from_name(usr_info->dev_name);
    if (log == NULL) {
        modem_log_pr_err("no device log\n");
        return;
    }

    /* if reader is not ready, no need to wakeup waitqueue */
    if (usr_info->mem && usr_info->mem->app_is_active) { /*lint !e456*/
        __pm_stay_awake(&g_modem_log.wake_lock);         /*lint !e454*/
        if ((&log->wq) != NULL) {
            wake_up_interruptible(&log->wq);
        }
    }
    modem_log_pr_debug("%s\n", __func__); /*lint !e456*/
} /*lint !e454*/

/*
 * modem_log_fwrite_trigger_force - wakeup acore and trigger file write from ring buffer to log record file
 */
void modem_log_fwrite_trigger_force(void)
{
    modem_log_wakeup_all();
}
EXPORT_SYMBOL(modem_log_fwrite_trigger_force); /*lint !e19 */

int modem_log_init(void)
{
    wakeup_source_init(&g_modem_log.wake_lock, "modem_log_wake");
    g_modem_log.pm_notify.notifier_call = modem_log_notify;
    register_pm_notifier(&g_modem_log.pm_notify);
    g_modem_log.init_flag = 1;
    modem_log_pr_err("init ok\n");

    return 0;
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(modem_log_init);
#endif
