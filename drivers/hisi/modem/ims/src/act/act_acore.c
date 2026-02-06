/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include "securec.h"
#include "mdrv_icc.h"

#define BUFFERSIZE    256
#define RCSSRVBUFSIZE 3796
#define DEVICE_MAJOR  250
#define DEVICE_MINOR  0

#define VT_AP_CP_CHID          MDRV_ICC_VT_VOIP
#define ACT_DEV_NUM            2
#define ACT_DEV_MAX_INSTANCE   2
#define ACT_DEV_VT_INSTANCE    0
#define ACT_DEV_RCS_INSTANCE   1
#define ACT_UNUSED(_expr) \
    do {                  \
       (void)(_expr);     \
    } while (0)

struct act_cdev_data {
    struct list_head msg_list;
    unsigned int len;
    unsigned char data[];
};

struct act_instance {
    bool has_data;
    unsigned int len;
    unsigned int channel_id;
    unsigned char *data;
    wait_queue_head_t inq;
    spinlock_t list_lock;
    struct list_head msg_list;
    struct act_cdev *act_cdev;
};

struct act_cdev {
    struct cdev cdev;
    struct act_instance instance[ACT_DEV_MAX_INSTANCE];
};

struct act_cdev *act_cdevp = NULL; 
static struct class *act_class = NULL; 
static unsigned int act_major = 0;

/*lint -e429*/
unsigned int act_rcs_srv_msg_proc(unsigned int channel_id, int len)
{
    struct act_cdev_data *data = NULL;
    unsigned long flags = 0;
    int ret;

    printk(KERN_INFO "Enter act_rcs_srv_msg_proc len %u.\n", len);

    if (len > RCSSRVBUFSIZE || len == 0) {
        printk(KERN_ERR "length %d received exceed 3796.\n", len);
        return 0xFFFFFFFF;
    }

    data = (struct act_cdev_data *)kmalloc(sizeof(struct act_cdev_data) + len,
            GFP_KERNEL);
    if (data == NULL) {
        printk(KERN_ERR "act_rcs_srv_msg_proc: kmalloc data failed.\n");
        return -ENOMEM;
    }

    (void)memset_s(data, sizeof(struct act_cdev_data) + len, 0,
            sizeof(struct act_cdev_data) + len);
    ret = mdrv_icc_read(channel_id, data->data, len);
    if (ret == len) {
        data->len = len;
    } else {
        printk(KERN_ERR "rcs_msg_proc: mdrv_icc_read error ret: 0x%x.\n", ret);
        kfree(data);
        data = NULL;
        return 0xFFFFFFFF;
    }

    if (act_cdevp == NULL) {
        kfree(data);
        data = NULL;
        return 0xFFFFFFFF;
    }

    spin_lock_irqsave(&(act_cdevp->instance[1].list_lock), flags);

    list_add_tail(&(data->msg_list), &(act_cdevp->instance[1].msg_list)); 

    spin_unlock_irqrestore(&(act_cdevp->instance[1].list_lock), flags);
    act_cdevp->instance[1].has_data = true;
    wake_up(&(act_cdevp->instance[1].inq));  /* wake up the read process */

    printk(KERN_INFO "End act_rcs_srv_msg_proc result %d.\n", ret);

    return ret;
}
/*lint +e429*/

/*lint -e429*/
unsigned int act_msg_proc(unsigned int channel_id, int len)
{
    struct act_cdev_data *data = NULL;
    unsigned long flags = 0;
    int ret;

    printk(KERN_INFO "Enter act_msg_proc len %d.\n", len);

    if (len > BUFFERSIZE) {
        printk(KERN_ERR "the length %d received exceed 256.\n", len);
        return 0xFFFFFFFF;
    }

    data = (struct act_cdev_data *)kmalloc(sizeof(struct act_cdev_data) + len,
            GFP_KERNEL);
    if (data == NULL) {
        printk(KERN_ERR "act_msg_proc: kmalloc data failed.\n");
        return -ENOMEM;
    }

    (void)memset_s(data, sizeof(struct act_cdev_data) + len, 0,
            sizeof(struct act_cdev_data) + len);
    ret = mdrv_icc_read(channel_id, data->data, len);
    if (ret == len) {
        data->len = len;
    } else {
        printk(KERN_ERR "act_msg_proc: mdrv_icc_read error ret: 0x%x.\n", ret);
        kfree(data);
        data = NULL;
        return 0xFFFFFFFF;
    }

    if (act_cdevp == NULL) {
        kfree(data);
        data = NULL;
        return 0xFFFFFFFF;
    }

    spin_lock_irqsave(&(act_cdevp->instance[0].list_lock), flags);

    list_add_tail(&(data->msg_list), &(act_cdevp->instance[0].msg_list));

    spin_unlock_irqrestore(&(act_cdevp->instance[0].list_lock), flags);

    act_cdevp->instance[0].has_data = true;
    wake_up(&(act_cdevp->instance[0].inq));  /* wake up the read process */

    printk(KERN_INFO "End act_msg_proc result %d.\n", ret);

    return ret;
}
/*lint +e429*/

static int act_open(struct inode *node, struct file *filp)
{
    struct act_cdev *act_cdev = NULL;
    struct act_instance *instance = NULL;

    act_cdev = container_of(node->i_cdev, struct act_cdev, cdev);
    instance = &(act_cdev->instance[MINOR(node->i_rdev)]);
    filp->private_data = instance;
    if (MINOR(node->i_rdev) == ACT_DEV_VT_INSTANCE)
        instance->channel_id = MDRV_ICC_VT_VOIP;

    if (MINOR(node->i_rdev) == ACT_DEV_RCS_INSTANCE)
        instance->channel_id = MDRV_ICC_RCS_SERV;
    printk(KERN_ERR "info act_open: minor %d \n", MINOR(node->i_rdev));

    return 0;
}

static int act_release(struct inode *node, struct file *filp)
{
    ACT_UNUSED(node);
    ACT_UNUSED(filp);

    return 0;
}

/*lint -e571*/
static ssize_t act_read(struct file *filp, char __user *buf,
                            size_t size, loff_t *ppos)
{
    int ret = 0;
    unsigned long flags = 0;
    struct act_cdev_data *data = NULL;
    struct act_instance *instance = NULL;

    if (filp == NULL || buf == NULL)
        return -EFAULT;

    printk(KERN_INFO "Enter act_read to bufSize %u.\n", (unsigned int)size);

    instance = (struct act_instance *)filp->private_data;

    while (!instance->has_data) {
        if (filp->f_flags & O_NONBLOCK) {
            printk(KERN_ERR "act_read: NONBLOCK\n");
            return (size_t)(- EAGAIN);
        }

        wait_event_interruptible(instance->inq, instance->has_data);
    }

    printk(KERN_INFO "act_read start read Data %d.\n", instance->len);

    spin_lock_irqsave(&(instance->list_lock), flags);

    if (!list_empty(&(instance->msg_list))) {
        data = list_first_entry(&(instance->msg_list), struct act_cdev_data,
                    msg_list);
        if (data == NULL) {
            printk(KERN_INFO "act_read data is NULL.\n");
            spin_unlock_irqrestore(&(instance->list_lock), flags);
            return -EFAULT;
        }

        /* read data to user space */
        if (0 < data->len && size >= data->len) {
            if (copy_to_user(buf, (void *)(data->data), data->len)) {
                ret = -EFAULT;
                printk(KERN_ERR "act_read error\n");
            } else {
                ret = (int)(data->len);
                printk(KERN_INFO "act_read has read %d bytes.\n", ret);
            }
        }

        list_del((struct list_head *)data);
        kfree(data);
        data = NULL;
    } else {
        printk(KERN_ERR "act_read msg_list is empty! \n");
    }

    if (list_empty(&(instance->msg_list))) {
        instance->has_data = false;  /* has no data to read */
    } else {
        instance->has_data = true;  /* has data to read */
        printk(KERN_ERR "act_read msg_list is not empty! \n");
    }

    spin_unlock_irqrestore(&(instance->list_lock), flags);

    return (ssize_t)ret;
}
/*lint +e571*/

static ssize_t act_write(struct file *filp, const char __user *buf,
                            size_t size, loff_t *ppos)
{
    ssize_t len = size;
    struct act_instance *instance = NULL;
    size_t data_size;

    if ((filp == NULL) || (buf == NULL))
        return -EFAULT;

    instance = (struct act_instance *)(filp->private_data);
    if ((instance == NULL) || (instance->data == NULL)) {
        printk(KERN_ERR "act_write: instance OR buf is null\n");
        return -EFAULT;
    }

    if (instance->channel_id == VT_AP_CP_CHID) {
        data_size = BUFFERSIZE;
    } else if (instance->channel_id == MDRV_ICC_RCS_SERV){
        data_size = RCSSRVBUFSIZE;
    } else {
        return -E2BIG;
    }

    if (size > data_size) {
        printk(KERN_ERR "act_write input size long than buffer, channel:%u, size:%u.", 
                instance->channel_id, (unsigned int)size);
        return -E2BIG;
    }

    (void)memset_s(instance->data, data_size, 0, data_size);

    if (copy_from_user(instance->data, buf, size)) {
        printk(KERN_ERR "act_write failed.");
        return -EFAULT;
    }

    len = mdrv_icc_write(instance->channel_id, instance->data, (int)size);

    printk(KERN_INFO "act_write: send data %u bytes, iccreturn %u, "
            "channelId %u\n", (unsigned int)size, (unsigned int)len,
            instance->channel_id);
    return len;
}

static unsigned int act_poll(struct file *filp, poll_table *wait)
{
    struct act_instance *act_instance = NULL;
    unsigned int mask = 0;

    printk(KERN_INFO "Enter act_poll.\n");

    act_instance = filp->private_data;

    /* put the queue into poll_table */
    poll_wait(filp, &(act_instance->inq), wait);

    if (act_instance->has_data) {
        mask |= POLLIN | POLLRDNORM;
        printk(KERN_INFO "notify read  process");
    }

    return mask;
}

static const struct file_operations act_cdev_fops = {
    .owner = THIS_MODULE,
    .read = act_read,
    .write = act_write,
    .open = act_open,
    .release = act_release,
    .poll = act_poll,
};

static void act_cdev_setup(struct act_cdev *dev, unsigned int index)
{
    int err;
    dev_t devno = MKDEV(act_major, index);

    printk(KERN_INFO "Enter act_cdev_setup.");

    cdev_init(&dev->cdev, &act_cdev_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &act_cdev_fops;

    err = cdev_add(&dev->cdev, devno, ACT_DEV_NUM);
    if (err != 0) {
        printk(KERN_NOTICE "Error %d adding ACT Device Failed", err);
        return;
    }
}

static void act_instance_init(struct act_instance *instance)
{
    instance->has_data = false;
    init_waitqueue_head(&(instance->inq));
    INIT_LIST_HEAD(&(instance->msg_list));
    spin_lock_init(&(instance->list_lock));
}

int act_cdev_init_instance(int instance_id)
{
    ICC_CHAN_ATTR_S chanattr;
    struct act_instance *instance = NULL;
    icc_read_cb read_fun = NULL;
    unsigned int size;
    unsigned int chanid;
    int ret;

    if (instance_id == ACT_DEV_VT_INSTANCE) {
        size = BUFFERSIZE;
        chanid = MDRV_ICC_VT_VOIP;
        read_fun = (icc_read_cb)act_msg_proc;
    } else if (instance_id == ACT_DEV_RCS_INSTANCE) {
        size = RCSSRVBUFSIZE;
        chanid = MDRV_ICC_RCS_SERV;
        read_fun = (icc_read_cb)act_rcs_srv_msg_proc;
    } else {
        return -E2BIG;
    }

    instance = &(act_cdevp->instance[instance_id]);
    instance->act_cdev = act_cdevp;

    instance->data = kmalloc(size, GFP_KERNEL);
    if (instance->data == NULL) {
        return -ENOMEM;
    }
    act_instance_init(instance);

    (void)memset_s(&chanattr, sizeof(chanattr), 0, sizeof(chanattr));
    chanattr.read_cb = read_fun;
    ret = mdrv_icc_open(chanid, &chanattr);
    if (ret != 0) {
        printk(KERN_ERR "act_cdev_init_vt: failed  code 0x%x\n", ret);
        return ret;
    }
    return 0;
}

void act_free_res(void)
{
    if (act_cdevp == NULL) 
        return;

    if (act_cdevp->instance[0].data != NULL) {
        kfree(act_cdevp->instance[0].data);
        act_cdevp->instance[0].data = NULL;
    }

    if (act_cdevp->instance[1].data != NULL) {
        kfree(act_cdevp->instance[1].data);
        act_cdevp->instance[1].data = NULL;
    }

    kfree(act_cdevp);
    act_cdevp = NULL;
}

int __init act_cdev_init(void)
{
    int ret;
    dev_t devno = MKDEV(DEVICE_MAJOR, 0);
    unsigned int size;

    printk(KERN_INFO "Enter act_cdev_init.");

    if (act_major) {
        ret = register_chrdev_region(devno, 2, "act");
    } else {
        ret = alloc_chrdev_region(&devno, 0, 2, "act");
        act_major = MAJOR(devno);
    }

    if (ret < 0) {
        printk(KERN_ERR "Error %d Init ACT device failed.", ret);
        return ret;
    }

    size = sizeof(struct act_cdev);
    act_cdevp = (struct act_cdev *)kmalloc(size, GFP_KERNEL);
    if (act_cdevp == NULL) {
        printk(KERN_ERR "act_cdev_init: kmalloc failed.\n");
        unregister_chrdev_region(devno, 2);
        return -ENOMEM;
    }

    (void)memset_s(act_cdevp, size, 0, size);
    act_cdev_setup(act_cdevp, 0);

    act_class = class_create(THIS_MODULE, "act");
    if (IS_ERR(act_class)) {
        printk(KERN_ERR "Act_Init: failed to create class.\n");
        ret = -EFAULT;
    } else {
        /* MINOR 0 is volte device, MINOR 1 is rcs device */
        device_create(act_class, NULL, MKDEV(act_major, 0), NULL, "act");
        device_create(act_class, NULL, MKDEV(act_major, 1), NULL, "actr");
    }

    ret = act_cdev_init_instance(ACT_DEV_VT_INSTANCE);
    if (ret != 0) {
        act_free_res();
        return ret;
    }

    ret = act_cdev_init_instance(ACT_DEV_RCS_INSTANCE);
    if (ret != 0){
        act_free_res();
        return ret;
    }
    return 0;
}

void __exit act_cdev_exit(void)
{
    dev_t devno = MKDEV(act_major, 0);

    printk(KERN_INFO "Enter act_cdev_exit.");

    if (act_cdevp != NULL)
        cdev_del(&(act_cdevp->cdev));

    device_destroy(act_class, devno);
    class_destroy(act_class);

    if (act_cdevp != NULL) {
        kfree(act_cdevp);
        act_cdevp = NULL;
    }
 
    unregister_chrdev_region(devno, 1);
}

#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
MODULE_AUTHOR("HUAWEI");
MODULE_LICENSE("Dual BSD/GPL");

module_init(act_cdev_init);
module_exit(act_cdev_exit);
#endif
