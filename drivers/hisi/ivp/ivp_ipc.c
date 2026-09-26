/*
 * ivp ipc communication
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "ivp_ipc.h"
#include <linux/init.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/ion.h>
#include <linux/hisi/hisi_ion.h>
#include <linux/hisi-iommu.h>
#include "securec.h"
#include "ivp_sec.h"
#include "ivp.h"
#include "ivp_log.h"
#include "ivp_manager.h"
#include "ivp_platform.h"
/*lint -save -e785 -e64 -e715 -e838 -e747*/
/*lint -save -e712 -e737 -e64 -e30 -e438 -e713 -e713*/
/*lint -save -e529 -e838 -e438 -e774 -e826*/
/*lint -save -e775 -e730 -e730 -e528*/
/*lint -save -e753 -e1058*/

#define     CMD_SELECTALGO          0x10
#define     CMD_ALOGPARAM           0x11
#define     CMD_ALGOINIT            0x12
#define     CMD_ALGORUN             0x13
#define     CMD_ALGOEXIT            0x14
#define     CMD_MEMORYALLOC         0x40
#define     CMD_MEMORYFREE          0x41
#define     CMD_SHARE_MEM_ALLOC     0x42
#define     CMD_DUMPIMAGE           0x60
#define     CMD_LOADIMAGE           0x61
#define     CMD_RPC_INVOKE          0x68
#define     CMD_LOOPBACK            0x7F
#define     TMP_BUFF_MASK           0x7F
#define     IPC_RECV_LEN            4

static int ivp_ipc_open(struct inode *inode, struct file *file);
static ssize_t ivp_ipc_read(struct file *file, char __user *buff,
	size_t size, loff_t *off);
static ssize_t ivp_ipc_write(struct file *file, const char __user *buff,
	size_t size, loff_t *off);
static int ivp_ipc_release(struct inode *inode, struct file *file);
static long ivp_ipc_ioctl(struct file *fd, unsigned int cmd,
	unsigned long args);
static long ivp_ipc_ioctl32(struct file *fd, unsigned int cmd,
	unsigned long args);
static const struct file_operations ivp_ipc_fops = {
	.owner = THIS_MODULE,
	.open = ivp_ipc_open,
	.read = ivp_ipc_read,
	.write = ivp_ipc_write,
	.release = ivp_ipc_release,
	.unlocked_ioctl = ivp_ipc_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = ivp_ipc_ioctl32,
#endif
};

#ifdef CONFIG_OF
static const struct of_device_id ivp_ipc_of_descriptor[] = {
	{.compatible = "hisilicon,dev-ivp-ipc",},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_mdev_of_match);
#endif

/* IPC Pakcet Operations. include get, put, remove, replace */
static inline struct ivp_ipc_packet *ivp_ipc_alloc_packet(size_t len)
{
	struct ivp_ipc_packet *packet = NULL;

	packet = kzalloc(sizeof(struct ivp_ipc_packet), GFP_ATOMIC);
	if (!packet) {
		ivp_err("malloc packet fail");
		return NULL;
	}

	packet->buff = kzalloc(sizeof(char) * len, GFP_ATOMIC);
	if (!packet->buff) {
		ivp_err("malloc packet buf fail");
		kfree(packet);
		return NULL;
	}

	packet->len = len;

	return packet;
}

static inline void ivp_ipc_free_packet(struct ivp_ipc_packet *packet)
{
	if (!packet) {
		ivp_err("packet is NULL");
		return;
	}

	if (packet->buff) {
		kfree(packet->buff);
		packet->buff = NULL;
	}
	kfree(packet);
	packet = NULL;
}

static inline void ivp_ipc_init_queue(struct ivp_ipc_queue *queue)
{
	if (!queue) {
		ivp_err("queue is NULL");
		return;
	}
	spin_lock_init(&queue->rw_lock);
	sema_init(&queue->r_lock, 0);
	atomic_set(&queue->flush, 1);
	INIT_LIST_HEAD(&queue->head);
}

static struct ivp_ipc_packet *ivp_ipc_get_packet(struct ivp_ipc_queue *queue)
{
	struct ivp_ipc_packet *packet = NULL;

	ivp_dbg("get packet");
	spin_lock_irq(&queue->rw_lock);
	packet = list_first_entry_or_null(
		&queue->head, struct ivp_ipc_packet, list);
	spin_unlock_irq(&queue->rw_lock);

	return packet;
}

static int ivp_ipc_add_packet(
	struct ivp_ipc_queue *queue, const void *data,
	size_t len)
{
	struct ivp_ipc_packet *new_packet = NULL;
	int ret;

	len = (len > DEFAULT_MSG_SIZE) ? DEFAULT_MSG_SIZE : len;
	new_packet = ivp_ipc_alloc_packet(DEFAULT_MSG_SIZE);
	if (!new_packet) {
		ivp_err("new packet NULL");
		return -ENOMEM;
	}
	ret = memcpy_s(new_packet->buff, DEFAULT_MSG_SIZE, data, len);
	if (ret != EOK) {
		ivp_err("memcpy_s fail, ret [%d]", ret);
		ivp_ipc_free_packet(new_packet);
		return -EINVAL;
	}

	spin_lock_irq(&queue->rw_lock);
	list_add_tail(&new_packet->list, &queue->head);
	spin_unlock_irq(&queue->rw_lock);

	up(&queue->r_lock);

	return ret;
}

/* only remove one packet, the caller guarantees mutual exclusion */
static void ivp_ipc_remove_one_packet(
	struct ivp_ipc_queue *queue __attribute__((unused)),
	struct ivp_ipc_packet *packet)
{
	if (packet)
		list_del(&packet->list);

	ivp_ipc_free_packet(packet);
}

static void ivp_ipc_remove_packet(
	struct ivp_ipc_queue *queue, struct ivp_ipc_packet *packet)
{
	spin_lock_irq(&queue->rw_lock);
	ivp_ipc_remove_one_packet(queue, packet);
	spin_unlock_irq(&queue->rw_lock);
}

static void ivp_ipc_remove_all_packet(struct ivp_ipc_queue *queue)
{
	struct list_head *p = NULL;
	struct list_head *n = NULL;

	spin_lock_irq(&queue->rw_lock);
	list_for_each_safe(p, n, &queue->head) {
		struct ivp_ipc_packet *packet = list_entry(p,
			struct ivp_ipc_packet, list);

		ivp_ipc_remove_one_packet(queue, packet);
	}
	spin_unlock_irq(&queue->rw_lock);
}


static int ivp_ipc_open(struct inode *inode, struct file *fd)
{
	struct ivp_ipc_device *pdev = NULL;
	struct miscdevice *pmiscdev = NULL;
	int ret;

	if (!fd || !fd->private_data) {
		ivp_err("fd or pdev is NULL");
		return -EINVAL;
	}

	pmiscdev = (struct miscdevice *)fd->private_data;
	pdev = container_of(pmiscdev, struct ivp_ipc_device, device);

	if (pdev == NULL) {
		ivp_err("pdev is NULL");
		return -EINVAL;
	}

	if (!atomic_dec_and_test(&pdev->accessible)) {
		ivp_err("maybe ivp ipc dev has been opened!");
		atomic_inc(&pdev->accessible);
		return -EBUSY;
	}

	ret = nonseekable_open(inode, fd);
	if (ret != 0) {
		atomic_inc(&pdev->accessible);
		return ret;
	}
	fd->private_data = (void *)pdev;

	if ((unlikely(atomic_read(&pdev->recv_queue.flush))) != 1)
		ivp_warn("Flush was not set when first open! %u",
			atomic_read(&pdev->recv_queue.flush));

	atomic_set(&pdev->recv_queue.flush, 0);

	if (unlikely(!list_empty(&pdev->recv_queue.head))) {
		ivp_warn("queue is not Empty!");
		ivp_ipc_remove_all_packet(&pdev->recv_queue);
	}

	sema_init(&pdev->recv_queue.r_lock, 0);
	ivp_info("open node %s success", pdev->device.name);

	return ret;
}

/****************************************************************
 *  len:   msg len. data len is (len * sizeof(msg))
 ** *************************************************************/
static int ivp_ipc_recv_notifier(struct notifier_block *nb,
	unsigned long len, void *data)
{
	struct ivp_ipc_device *pdev = NULL;
	int ret;

	if (data == NULL || nb == NULL) {
		ivp_err("data or nb is NULL");
		return -EINVAL;
	}

	if (len == 0) {
		ivp_err("len equals to 0");
		return -EINVAL;
	}

	pdev = container_of(nb, struct ivp_ipc_device, recv_nb);
	if (!pdev) {
		ivp_err("pdev NULL");
		return -EINVAL;
	}

	if (atomic_read(&pdev->recv_queue.flush) == 1) {
		ivp_err("flushed.No longer receive msg");
		ret = -ECANCELED;
	} else {
		len *= IPC_RECV_LEN;
		ret = ivp_ipc_add_packet(&pdev->recv_queue, data, len);
	}

	return ret;
}

static ssize_t ivp_ipc_read(struct file *fd,
	char __user *buff,
	size_t size,
	loff_t *off)
{
	struct ivp_ipc_device *pdev = NULL;
	struct ivp_ipc_queue *queue = NULL;
	struct ivp_ipc_packet *packet = NULL;
	ssize_t ret = 0;

	if (!fd || !buff) {
		ivp_err("fd or buff is null!");
		return -EINVAL;
	}

	pdev = (struct ivp_ipc_device *)fd->private_data;
	queue = &pdev->recv_queue;

	if (size != DEFAULT_MSG_SIZE) {
		ivp_err("Size should be 32Byte.size:%lu", size);
		return -EINVAL;
	}

	/* Block until IVPCore send new Msg */
	if (down_interruptible(&queue->r_lock)) {
		ivp_info("interrupted.");
		return -ERESTARTSYS;
	}

	if (atomic_read(&queue->flush) == 1) {
		ivp_info("flushed.");
		return -ECANCELED;
	}

	mutex_lock(&pdev->ivp_ipc_read_mutex);
	packet = ivp_ipc_get_packet(queue);
	if (packet) {
		if (copy_to_user(buff, packet->buff, size)) {
			ivp_err("copy to user fail.");
			ret = -EFAULT;
			goto out;
		}

	} else {
		ivp_err("get packet NULL");
		mutex_unlock(&pdev->ivp_ipc_read_mutex);
		return -EINVAL;
	}
	ivp_info("send ipc cmd 0x%x,0x%x,0x%x,0x%x", packet->buff[0],
		packet->buff[1], packet->buff[2], packet->buff[3]);

	*off += size;
	ret = size;

out:
	ivp_ipc_remove_packet(queue, packet);
	mutex_unlock(&pdev->ivp_ipc_read_mutex);
	return ret;
}

static ssize_t ivp_ipc_write(struct file *fd, const char __user *buff,
	size_t size, loff_t *off)
{
	struct ivp_ipc_device *pdev = NULL;
	char *tmp_buff = NULL;
	ssize_t ret = 0;

	if (!fd || !buff) {
		ivp_err("fd or buff is null!");
		return -EINVAL;
	}

	pdev = (struct ivp_ipc_device *)fd->private_data;

	if (size != DEFAULT_MSG_SIZE) {
		ivp_err("size %lu not %d.", size, DEFAULT_MSG_SIZE);
		return -EINVAL;
	}

	tmp_buff = kzalloc((unsigned long)DEFAULT_MSG_SIZE, GFP_KERNEL);
	if (!tmp_buff) {
		ivp_err("malloc buf failed.");
		return -ENOMEM;
	}

	if (copy_from_user(tmp_buff, buff, size)) {
		ivp_err("copy from user fail.");
		ret = -EFAULT;
		goto out;
	}

/* trans ion fd to phyaddr */

	if (tmp_buff[0] == 0) {
		/* 3 is array element */
		ivp_info("receive ipc cmd 0x%x from node:%s",
			((unsigned char)tmp_buff[3] & TMP_BUFF_MASK), pdev->device.name);
	}

	ret = RPROC_ASYNC_SEND(pdev->send_ipc, (rproc_msg_t *)tmp_buff,
		size / sizeof(rproc_msg_len_t));
	if (ret != 0) {
		ivp_err("ipc send fail [%ld].", ret);
		goto out;
	}

	*off += size;
	ret = size;

out:
	kfree(tmp_buff);
	return ret;
}

static int ivp_ipc_release(struct inode *inode __attribute__((unused)),
	struct file *fd)
{
	struct ivp_ipc_device *pdev = NULL;

	if (!fd) {
		ivp_err("fd is NULL");
		return -EINVAL;
	}

	pdev = (struct ivp_ipc_device *)fd->private_data;

	if (atomic_read(&pdev->accessible) != 0) {
		ivp_err("maybe ivp dev not opened!");
		return -EINVAL;
	}

	ivp_info("enter");
	/* drop all packet */
	mutex_lock(&pdev->ivp_ipc_read_mutex);
	ivp_ipc_remove_all_packet(&pdev->recv_queue);
	mutex_unlock(&pdev->ivp_ipc_read_mutex);

	atomic_inc(&pdev->accessible);

	return EOK;
}

static int ivp_ipc_flush(struct ivp_ipc_device *pdev)
{
	struct ivp_ipc_queue *queue = NULL;
	if (!pdev) {
		ivp_err("invalid param pdev");
		return -EINVAL;
	}
	/* non block read.Make read return HAL. */
	queue = &pdev->recv_queue;
	if (!queue) {
		ivp_err("invalid param queue");
		return -EINVAL;
	}

	ivp_info("enter");
	atomic_set(&queue->flush, 1);
	up(&queue->r_lock);
	mutex_lock(&pdev->ivp_ipc_read_mutex);
	ivp_ipc_remove_all_packet(queue);
	mutex_unlock(&pdev->ivp_ipc_read_mutex);

	return EOK;
}

static long ivp_ipc_ioctl(struct file *fd, unsigned int cmd, unsigned long args)
{
	int ret = 0;
	struct ivp_ipc_device *pdev = NULL;
	if (!fd) {
		ivp_err("invalid input fd");
		return -EINVAL;
	}
	pdev = (struct ivp_ipc_device *)fd->private_data;

	ivp_info("cmd:%#x", cmd);
	ivp_dbg("IVP_IOCTL_IPC_FLUSH_ENABLE:%#lx", IVP_IOCTL_IPC_FLUSH_ENABLE);
	switch (cmd) {
	case IVP_IOCTL_IPC_FLUSH_ENABLE:
		ret = ivp_ipc_flush(pdev);
		break;

	case IVP_IOCTL_IPC_IVP_SECMODE:
		if (!args) {
			ivp_err("invalid input args");
			return -EINVAL;
		}
		if (copy_from_user(&pdev->ivp_secmode, (void *)(uintptr_t)args,
			sizeof(unsigned int)) != 0) {
			ivp_err("invalid input param size");
			return -EINVAL;
		}
		if (pdev->ivp_secmode != NOSEC_MODE &&
			pdev->ivp_secmode != SECURE_MODE) {
			ivp_err("invalid ivp sec mode");
			return -EINVAL;
		}
		break;

	default:
		ivp_err("invalid cmd, %#x", cmd);
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static long ivp_ipc_ioctl32(struct file *fd,
	unsigned int cmd,
	unsigned long args)
{
	void *user_ptr = compat_ptr(args);

	return ivp_ipc_ioctl(fd, cmd, (uintptr_t)user_ptr);
}

static inline void ivp_ipc_init_recv_nb(struct notifier_block *nb)
{
	nb->notifier_call = ivp_ipc_recv_notifier;
}

static int ivp_ipc_device_init(struct platform_device *platform_pdev,
		struct ivp_ipc_device *pdev)
{
	int ret;

	atomic_set(&pdev->accessible, 1);
	pdev->ipc_pdev = platform_pdev;
	ret = misc_register(&pdev->device);
	if (ret) {
		ivp_err("failed to register misc device");
		return ret;
	}
	mutex_init(&pdev->ivp_ipc_read_mutex);
	mutex_init(&pdev->ivp_ion_mutex);
	ret = RPROC_MONITOR_REGISTER(
		(unsigned char)pdev->recv_ipc, &pdev->recv_nb);
	if (ret) {
		ivp_err("failed to create receiving notifier block");
		goto err_out;
	}

	ivp_ipc_init_queue(&pdev->recv_queue);

	ivp_ipc_init_recv_nb(&pdev->recv_nb);

	platform_set_drvdata(platform_pdev, pdev);

	ivp_info("ipc device %s init success", pdev->device.name);
	return ret;

err_out:
	mutex_destroy(&pdev->ivp_ion_mutex);
	mutex_destroy(&pdev->ivp_ipc_read_mutex);
	misc_deregister(&pdev->device);
	ivp_err("ipc device %s init fail", pdev->device.name);
	return ret;
}

static int ivp_ipc_probe(struct platform_device *platform_pdev)
{
	struct ivp_ipc_device *pdev = NULL;
	int ret;
	enum ivp_core_id core_id;

	if (!platform_pdev) {
		ivp_err("platform_pdev is NULL");
		return -EINVAL;
	}

	core_id = ivp_determine_coreid(platform_pdev, IPC_CORE_TYPE);
	if (!ivp_check_coreid(core_id)) {
		ivp_err("get the invalid ivp_core id num: %d", core_id);
		return -EINVAL;
	}

	pdev = kzalloc(sizeof(struct ivp_ipc_device), GFP_KERNEL);
	if (!pdev) {
		ivp_err("ipc device kzalloc failed");
		return -ENOMEM;
	}
	pdev->device.minor = MISC_DYNAMIC_MINOR;
	pdev->device.fops = &ivp_ipc_fops;
	if (core_id == IVP_CORE0_ID) {
		pdev->device.name = "ivp-ipc";
		pdev->send_ipc = HISI_ACPU_IVP_MBX_1;
		pdev->recv_ipc = HISI_IVP_ACPU_MBX_1;
	} else {
		kfree(pdev);
		ivp_err("invalid core_id nums");
		return -EINVAL;
	}

	ret = ivp_ipc_device_init(platform_pdev, pdev);
	if (ret) {
		kfree(pdev);
		ivp_err("ivp%d_ipc_device_init failed %d", core_id, ret);
		return ret;
	}
	ivp_info("ipc device %s probe success", pdev->device.name);
	return ret;
}

static int ivp_ipc_remove(struct platform_device *plat_devp)
{
	struct ivp_ipc_device *pdev = NULL;

	if (!plat_devp) {
		ivp_err("platform_pdev is NULL");
		return -EINVAL;
	}

	pdev = (struct ivp_ipc_device *)platform_get_drvdata(plat_devp);
	if (!pdev) {
		ivp_err("get the drvdata failed!");
		return -ENOMEM;
	}

	RPROC_MONITOR_UNREGISTER(
		(unsigned char)pdev->recv_ipc, &pdev->recv_nb);
	mutex_destroy(&pdev->ivp_ion_mutex);
	mutex_destroy(&pdev->ivp_ipc_read_mutex);
	misc_deregister(&pdev->device);
	kfree(pdev);

	return EOK;
}

static struct platform_driver ivp_ipc_driver = {
	.probe = ivp_ipc_probe,
	.remove = ivp_ipc_remove,
	.driver = {
		.name = "ivp-ipc",
		.owner = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_match_ptr(ivp_ipc_of_descriptor),
#endif
	}, //lint -e785
}; //lint -e785

module_platform_driver(ivp_ipc_driver); //lint -e528 -e64
//lint -restore

