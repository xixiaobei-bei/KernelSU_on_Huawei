/*
 * file_operation.c
 *
 * API for userspace ioctl
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/printk.h>
#include <securec.h>

#include "phys_mem_int.h"
#include "page_claiming.h"
#include "utils.h"
#include "mmap_phys.h"

/* Open and close phys_mem device */
atomic64_t session_counter = ATOMIC_INIT(0);

/* filp open */
static int phys_mem_open(struct inode *inode, struct file *filp)
{
	struct phys_mem_dev *dev = NULL;
	struct phys_mem_session *session = NULL;
	int num;

	if ((inode == NULL) || (filp == NULL))
		return -EINVAL;

	num = MINOR(inode->i_rdev); /* get minor number of dev */

	if (num >= MEMDEV_NR_DEVS)
		return -ENODEV;

	dev = container_of(inode->i_cdev, struct phys_mem_dev, cdev);

	session = kmem_cache_alloc(g_session_mem_cache, GFP_KERNEL);
	if (session == NULL)
		return -ENOMEM;

	session->session_id = atomic64_add_return(1, &session_counter);
	session->device = dev;
	sema_init(&session->sem, 1);
	session->vmas = 0;
	session->num_frame_stati = 0;
	session->frame_stati = NULL;
	session->status.state = SESSION_STATE_INVALID;

	SET_STATE(session, SESSION_STATE_OPEN);

	/* Memory session info to private data */
	filp->private_data = session;

	return 0;
}

/* filp release */
static int phys_mem_release(struct inode *inode, struct file *filp)
{
	struct phys_mem_session *session = NULL;

	if ((inode == NULL) || (filp == NULL))
		return -EINVAL;

	session = (struct phys_mem_session *)filp->private_data;

	if (down_interruptible(&session->sem))
		return -ERESTARTSYS;

	/* Several steps for cleaning depending on the state of the session */
	while (GET_STATE(session) != SESSION_STATE_CLOSED) {
		switch (GET_STATE(session)) {
		case SESSION_STATE_OPEN:
			session->device = NULL;
			session->vmas = 0;
			SET_STATE(session, SESSION_STATE_CLOSED);
			break;

		case SESSION_STATE_CONFIGURING:
			up(&session->sem);
			return -ERESTARTSYS;

		case SESSION_STATE_CONFIGURED:
			/* Free all claimed pages */
			free_page_stati(session);
			SET_STATE(session, SESSION_STATE_OPEN);
			break;

		case SESSION_STATE_MAPPED:
			if (session->vmas) {
				pr_info("Session %llu:", session->session_id);
				pr_info("still holds %d\n", session->vmas);
			}
			SET_STATE(session, SESSION_STATE_CONFIGURED);
			break;

		default:
			pr_err("Err session state\n");
			break;
		}
	}

	up(&session->sem);
	kmem_cache_free(g_session_mem_cache, session);

	return 0;
}

/*
 * Data management:
 * Read: Read session->frame_stati, Assumes that the session is in
 * SESSION_STATE_CONFIGURED or SESSION_STATE_MAPPED
 */
static ssize_t file_read_configured(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	ssize_t retval = 0;
	ssize_t max_size;
	struct phys_mem_session *session = NULL;

	if ((buf == NULL) || (filp == NULL) || (f_pos == NULL))
		return -EINVAL;

	session = (struct phys_mem_session *)filp->private_data;

	if (down_interruptible(&session->sem))
		return -ERESTARTSYS;

	if ((GET_STATE(session) != SESSION_STATE_CONFIGURED) &&
		(GET_STATE(session) != SESSION_STATE_MAPPED)) {
		retval = -EIO;
		pr_err("Session %llu: The session cannot be read in state %i",
			session->session_id, GET_STATE(session));
		up(&session->sem);
		return retval;
	}

	max_size = SESSION_FRAME_STATI_SIZE(session->num_frame_stati);

	if (*f_pos > max_size) {
		up(&session->sem);
		return retval;
	}

	if (*f_pos + count > max_size)
		count = max_size - *f_pos;

	if (copy_to_user(buf, ((void *)((unsigned long)session->frame_stati) +
		*f_pos), count)) {
		retval = -EFAULT;
		up(&session->sem);
		return retval;
	}

	up(&session->sem);

	*f_pos += count;
	return count;
}

static long file_ioctl_req_page(struct phys_mem_session *session,
	unsigned long arg)
{
	int ret;
	struct phys_mem_request request;

	if (session == NULL)
		return -EINVAL;

	if (memset_s((void *)&request,
		sizeof(request), 0, sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request, (struct phys_mem_request __user *)arg,
		sizeof(request))) {
		pr_err("Session %llu: file_ioctl_open: copy_from_user failed.\n",
			  session->session_id);
		ret = -EFAULT;
	} else {
		if (request.protocol_version != IOCTL_REQUEST_VERSION)
			ret = -EINVAL;
		else
			ret = handle_request_pages(session, &request);
	}

	return ret;
}

static long file_ioctl_mark_page(struct phys_mem_session *session,
	unsigned long arg)
{
	int ret;
	struct mark_page_poison request;

	if (session == NULL)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0,
		sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request,
		(struct mark_page_poison __user *)arg, sizeof(request))) {
		pr_err("Session %llu: file_ioctl_open: copy_from_user failed.\n",
			session->session_id);
		ret = -EFAULT;
	} else {
		if (request.protocol_version != IOCTL_REQUEST_VERSION)
			ret = -EINVAL;
		else
			ret = handle_mark_page_poison(session, &request);
	}
	return ret;
}

static long file_ioctl_get_pa_addr(const struct phys_mem_session *session,
	unsigned long arg)
{
	int ret;
	struct get_addr_info request;

	if (session == NULL)
		return -EINVAL;

	if (memset_s((void *)&request,
		sizeof(request), 0,	sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request,
		(struct get_addr_info __user *)arg, sizeof(request))) {
		pr_err("Session %llu: file_ioctl_open: copy_from_user failed.\n",
			  session->session_id);
		ret = -EFAULT;
	} else {
		if (request.protocol_version != IOCTL_REQUEST_VERSION)
			ret = -EINVAL;
		else {
			unsigned long paddr = virt_to_phy_u(request.address);

			request.address = paddr;
			ret = copy_to_user((struct get_addr_info __user *)arg,
				&request, sizeof(request));
		}
	}
	return ret;
}

static long file_ioctl_access_nve(const struct phys_mem_session *session,
	unsigned long arg)
{
	int ret;
	struct user_nve_info request;

	if (session == NULL)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0,
		sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request, (struct user_nve_info __user *)arg,
		sizeof(request))) {
		pr_err("Session %llu: file_ioctl_open: copy_from_user failed.\n",
			session->session_id);
		ret = -EFAULT;
	} else {
		if (request.protocol_version
			!= IOCTL_REQUEST_VERSION)
			ret = -EINVAL;
		else {
			ret = write_hisi_nve_ddrfault(&request);
			ret = copy_to_user((struct user_nve_info __user *)arg,
				&request, sizeof(request));
		}
	}
	return ret;
}

static long file_ioctl_reason(const struct phys_mem_session *session,
	unsigned long arg)
{
	int ret;
	struct reboot_reason request;

	if (session == NULL)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0,
		sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request, (struct reboot_reason __user *)arg,
		sizeof(request))) {
		pr_err("Session %llu: file_ioctl_open: copy_from_user failed.\n",
			session->session_id);
		ret = -EFAULT;
	} else {
		if (request.protocol_version != IOCTL_REQUEST_VERSION) {
			ret = -EINVAL;
		} else {
			g_pre_isolate_ddr = request.wp_reboot_reason;
			request.wp_reboot_reason = g_reboot_flag_ddr;
			ret = copy_to_user((struct reboot_reason __user *)arg,
				&request, sizeof(request));
		}
	}
	return ret;
}

static long file_ioctl_bbox_record(const struct phys_mem_session *session,
	unsigned long arg)
{
	int ret = 0;
	struct user_nve_info request;

	if (session == NULL)
		return -EINVAL;

	if (memset_s((void *)&request, sizeof(request), 0,
		sizeof(request)) != EOK) {
		pr_err("%s memset_s failed\n", __func__);
		return -EFAULT;
	}

	if (copy_from_user(&request, (struct user_nve_info __user *)arg,
		sizeof(request))) {
		pr_err("Session %llu: file_ioctl_open: copy_from_user failed.\n",
			session->session_id);
		ret = -EFAULT;
	} else {
		if (request.protocol_version != IOCTL_REQUEST_VERSION)
			ret = -EINVAL;
	}
	return ret;
}

/* The ioctl() implementation */
static long file_ioctl_open(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	int err;
	int ret;
	struct phys_mem_session *session = NULL;

	if (filp == NULL)
		return -EINVAL;

	session = (struct phys_mem_session *)filp->private_data;

	/* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
	if (_IOC_TYPE(cmd) != PHYS_MEM_IOC_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > PHYS_MEM_IOC_MAXNR)
		return -ENOTTY;

	if (_IOC_DIR(cmd) & _IOC_READ) {
		err = !access_ok(VERIFY_WRITE, (void __user *)arg,
			 _IOC_SIZE(cmd));
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		err = !access_ok(VERIFY_READ, (void __user *)arg,
			 _IOC_SIZE(cmd));
	}
	if (err)
		return -EFAULT;

	switch (cmd) {
	case PHYS_MEM_IOC_REQUEST_PAGES: {
		ret = file_ioctl_req_page(session, arg);
		break;
	}

	case PHYS_MEM_IOC_MARK_FRAME_BAD: {
		ret = file_ioctl_mark_page(session, arg);
		break;
	}

	case PHYS_MEM_IOC_VA_GET_PADDR: {
		ret = file_ioctl_get_pa_addr(session, arg);
		break;
	}

	case PHYS_MEM_IOC_ACCESS_NVE: {
		ret = file_ioctl_access_nve(session, arg);
		break;
	}

	case PHYS_MEM_IOC_REBOOT_REASON: {
		ret = file_ioctl_reason(session, arg);
		break;
	}

	case PHYS_MEM_IOC_BBOX_RECORD: {
		ret = file_ioctl_bbox_record(session, arg);
		break;
	}

	default:
		/* redundant, as cmd was checked against MAXNR */
		pr_info("Session %llu: default %d\n", session->session_id, cmd);
		return -ENOTTY;
	}

	return ret;
}

/* seek the read-buffer */
static loff_t file_llseek_configured(struct file *filp, loff_t off, int whence)
{
	struct phys_mem_session *session = NULL;
	size_t max_size;
	long newpos = 0;
	long error = 0;

	if (filp == NULL)
		return -EINVAL;

	session = (struct phys_mem_session *)filp->private_data;

	if (down_interruptible(&session->sem))
		return -ERESTARTSYS;

	if ((GET_STATE(session) != SESSION_STATE_CONFIGURED) &&
		(GET_STATE(session) != SESSION_STATE_MAPPED)) {
		error = -EIO;
		pr_err("Session %llu: The session cannot be llseek in state %i",
			session->session_id, GET_STATE(session));
		up(&session->sem);
		return error;
	}

	max_size = SESSION_FRAME_STATI_SIZE(session->num_frame_stati);

	switch (whence) {
	case 0: /* SEEK_SET */
		newpos = off;
		break;
	case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;
	case 2: /* SEEK_END */
		newpos = max_size + off;
		break;
	default: /* can't happen */
		error = -EINVAL;
		up(&session->sem);
		return error;
	}

	if (newpos < 0) {
		error = -EINVAL;
		up(&session->sem);
		return error;
	}

	filp->f_pos = newpos;

	up(&session->sem);
	return newpos;
}

/*
 * The file operations indexed by the session state. Dispatcher functions
 * extract the sessionstate from the file* and call the matching implementation
 * fops_by_session_state[status].op
 *
 */
const struct file_operations fops_by_session_state[] = {
	{
		/* CLOSED */
		.llseek = NULL,
		.read = NULL,
		.unlocked_ioctl = NULL,
		.mmap = NULL,
	},
	{
		/* OPEN */
		.llseek = NULL,
		.read = NULL,
		.unlocked_ioctl = file_ioctl_open,
		.mmap = NULL,
	},
	{
		/* CONFIGURING */
		.llseek = NULL,
		.read = NULL,
		.unlocked_ioctl = NULL,
		.mmap = NULL,
	},
	{
		/* CONFIGURED */
		.llseek = file_llseek_configured,
		.read = file_read_configured,
		.unlocked_ioctl = file_ioctl_open,
		.mmap = file_mmap_configured,
	},
	{
		/* MAPPED */
		.llseek = file_llseek_configured,
		.read = file_read_configured,
		.unlocked_ioctl = NULL,
		.mmap = file_mmap_configured,
	},
};

static loff_t dispatch_llseek(struct file *filp, loff_t off, int whence)
{
	struct phys_mem_session *session = NULL;
	loff_t (*fn)(struct file *, loff_t, int);

	if (filp == NULL)
		return -EINVAL;

	session = filp->private_data;

	if (session->status.state >= SESSION_NUM_STATES) {
		pr_err("Seeking with an invalid session state of %i!\n",
			session->status.state);
		return -EIO;
	}

	fn = fops_by_session_state[session->status.state].llseek;

	if (fn)
		return fn(filp, off, whence);

	pr_err("Session %llu:  llseek not supported in state %i\n",
		session->session_id, session->status.state);
	return -EIO;
}

static ssize_t dispatch_read(struct file *filp, char __user *buf,
	size_t count, loff_t *f_pos)
{
	struct phys_mem_session *session = NULL;
	ssize_t (*fn)(struct file *, char __user *, size_t, loff_t *);

	if ((buf == NULL) || (filp == NULL) || (f_pos == NULL))
		return -EINVAL;

	session = filp->private_data;

	if (session->status.state >= SESSION_NUM_STATES) {
		pr_err("Reading with an invalid session state of %i!\n",
			session->status.state);
		return -EIO;
	}

	fn = fops_by_session_state[session->status.state].read;

	if (fn)
		return fn(filp, buf, count, f_pos);

	pr_err("Session %llu:  read not supported in state %i\n",
		session->session_id, session->status.state);
	return -EIO;
}

static long dispatch_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	struct phys_mem_session *session = NULL;
	long (*fn)(struct file *, unsigned int, unsigned long);

	if (filp == NULL)
		return -EINVAL;

	session = filp->private_data;

	if (session->status.state >= SESSION_NUM_STATES) {
		pr_err("IOCTL with an invalid session state of %i!\n",
			session->status.state);
		return -EIO;
	}

	fn = fops_by_session_state[session->status.state].unlocked_ioctl;

	if (fn)
		return fn(filp, cmd, arg);

	pr_err("Session %llu:  ioctl not supported in state %i\n",
		session->session_id, session->status.state);
	return -EIO;
}

static int dispatch_mmap(struct file *filp, struct vm_area_struct *vma)
{
	/* get session info from filp's private data */
	struct phys_mem_session *session = NULL;
	int (*fn)(struct file *, struct vm_area_struct *);

	if ((filp == NULL) || (vma == NULL))
		return -EINVAL;

	session = filp->private_data;

	if (session->status.state >= SESSION_NUM_STATES) {
		pr_err("mmap with an invalid session state of %i!\n",
			session->status.state);
		return -EIO;
	}

	fn = fops_by_session_state[session->status.state].mmap;

	if (fn)
		return fn(filp, vma);

	pr_err("Session %llu:  mmap not supported in state %i\n",
		session->session_id, session->status.state);
	return -EIO;
}

const struct file_operations phys_mem_fops = {
	.owner		= THIS_MODULE,
	.llseek		= dispatch_llseek,
	.read		= dispatch_read,
	.unlocked_ioctl = dispatch_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = dispatch_ioctl,
#endif
	.open		= phys_mem_open,
	.release	= phys_mem_release,
	.mmap		= dispatch_mmap,
};
