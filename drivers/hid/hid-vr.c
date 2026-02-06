/*
 * hid-vr.c
 *
 * Driver for Huawei VR devices. Based on hidraw driver.
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/hidraw.h>
#include <linux/version.h>
#include <linux/sched/signal.h>
#include <securec.h>
#include "hid-ids.h"

#undef pr_fmt
#define pr_fmt(fmt) "[hw_vr]%s: " fmt, __func__

#ifdef HID_VR_DEBUG
#define d(format, arg...) pr_info(format, ##arg)
#else
#define d(format, arg...)
#endif

/* number of reports to buffer */
#define HW_HIDRAW_BUFFER_SIZE 64

#define HW_HIDRAW_MAX_DEVICES 8

#define HW_FIRST_MINOR 0

static struct class *hw_class;

static struct hidraw *hw_hidraw_table[HW_HIDRAW_MAX_DEVICES];
static DEFINE_MUTEX(minors_lock);

static int hw_major;
static struct cdev hw_cdev;

/* only used after mutex_lock(&list->read_mutex) */
static int hw_wait_for_add_data(struct hidraw_list *list,
					   struct file *file)
{
	int ret = 0;

	while (list->head == list->tail) {
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			break;
		}
		if (!list->hidraw->exist) {
			ret = -EIO;
			break;
		}
		if (file->f_flags & O_NONBLOCK) {
			ret = -EAGAIN;
			break;
		}

		/*
		 * allow O_NONBLOCK to work well
		 * from other threads
		 */
		mutex_unlock(&list->read_mutex);
		schedule();
		mutex_lock(&list->read_mutex);
		set_current_state(TASK_INTERRUPTIBLE);
	}

	return ret;
}

static ssize_t hw_hidraw_read(struct file *file,
				char __user *buffer,
				size_t count,
				loff_t *ppos)
{
	struct hidraw_list *list = NULL;
	int ret = 0;
	int len;
	DECLARE_WAITQUEUE(wait, current);

	if (!buffer)
		return -EINVAL;
	list = file->private_data;

	mutex_lock(&list->read_mutex);
	while (ret == 0) {
		if (list->head == list->tail) {
			add_wait_queue(&list->hidraw->wait, &wait);
			set_current_state(TASK_INTERRUPTIBLE);

			ret = hw_wait_for_add_data(list, file);

			set_current_state(TASK_RUNNING);
			remove_wait_queue(&list->hidraw->wait, &wait);
		}

		if (ret)
			goto out;

		len = list->buffer[list->tail].len > count ?
			count : list->buffer[list->tail].len;

		if (list->buffer[list->tail].value) {
			d("hidvr copy %d to user\n", len);
			if (copy_to_user(buffer,
					list->buffer[list->tail].value,
					len)) {
				ret = -EFAULT;
				goto out;
			}
			ret = len;
		}

		d("free list buffer 0x%pK\n", list->buffer[list->tail].value);
		kfree(list->buffer[list->tail].value);
		list->buffer[list->tail].value = NULL;
		list->tail = (list->tail + 1) % HW_HIDRAW_BUFFER_SIZE;
	}
out:
	mutex_unlock(&list->read_mutex);

	return ret;
}

static __u8 *hw_alloc_report_buf(struct hid_device *dev, size_t count)
{
	__u8 *buf = NULL;

	if (count > HID_MAX_BUFFER_SIZE) {
		hid_warn(dev, "hw - pid %d passed too large report\n",
			 task_pid_nr(current));
		return NULL;
	}

	if (count == 0) {
		hid_warn(dev, "hw - pid %d passed too short report\n",
			 task_pid_nr(current));
		return NULL;
	}
	buf = kzalloc(count * sizeof(__u8), GFP_KERNEL);
	if (!buf)
		pr_err("kzalloc buf error\n");

	return buf;
}

/*
 * The first byte is expected to be a report number.
 * This function is to be called with the minors_lock mutex held
 */
static ssize_t hw_hidraw_send_report(struct file *file,
					const char __user *buffer,
					size_t count,
					unsigned char report_type)
{
	unsigned int minor;
	struct hid_device *dev = NULL;
	__u8 *buf = NULL;
	int ret;

	if (!buffer)
		return -EINVAL;
	minor = iminor(file_inode(file));
	if (!hw_hidraw_table[minor]) {
		ret = -ENODEV;
		goto out;
	}
	dev = hw_hidraw_table[minor]->hid;

	buf = hw_alloc_report_buf(dev, count);
	if (!buf) {
		ret = -ENOMEM;
		goto out;
	}
	if (copy_from_user(buf, buffer, count)) {
		ret = -EFAULT;
		goto out_free;
	}

	if ((report_type == HID_OUTPUT_REPORT) &&
	    !(dev->quirks & HID_QUIRK_NO_OUTPUT_REPORTS_ON_INTR_EP)) {
		ret = hid_hw_output_report(dev, buf, count);
		/*
		 * compatibility with old implementation of USB-HID and I2C-HID:
		 * if the device does not support receiving output reports,
		 * on an interrupt endpoint, fallback to SET_REPORT HID command.
		 */
		if (ret != -ENOSYS)
			goto out_free;
	}

	ret = hid_hw_raw_request(dev, buf[0], buf, count, report_type,
				HID_REQ_SET_REPORT);

out_free:
	kfree(buf);
out:
	return ret;
}

/* the first byte is expected to be a report number */
static ssize_t hw_hidraw_write(struct file *file,
				const char __user *buffer,
				size_t count,
				loff_t *ppos)
{
	ssize_t ret;

	mutex_lock(&minors_lock);
	ret = hw_hidraw_send_report(file, buffer, count, HID_OUTPUT_REPORT);
	mutex_unlock(&minors_lock);
	return ret;
}

/*
 * This function performs a Get_Report transfer over the control endpoint
 * per section 7.2.1 of the HID specification, version 1.1.  The first byte
 * of buffer is the report number to request, or 0x0 if the defice does not
 * use numbered reports. The report_type parameter can be HID_FEATURE_REPORT
 * or HID_INPUT_REPORT.  This function is to be called with the minors_lock
 * mutex held.
 */
static ssize_t hw_hidraw_get_report(struct file *file,
					char __user *buffer,
					size_t count,
					unsigned char report_type)
{
	unsigned int minor;
	struct hid_device *dev = NULL;
	__u8 *buf = NULL;
	int ret;
	int len;
	unsigned char report_number;

	if (!buffer)
		return -EINVAL;
	minor = iminor(file_inode(file));
	if (!hw_hidraw_table[minor]) {
		ret = -ENODEV;
		goto out;
	}
	dev = hw_hidraw_table[minor]->hid;
	if (!dev->ll_driver->raw_request) {
		ret = -ENODEV;
		goto out;
	}

	buf = hw_alloc_report_buf(dev, count);
	if (!buf) {
		ret = -ENOMEM;
		goto out;
	}

	/*
	 * Read the first byte from the user. This is the report number,
	 * which is passed to hid_hw_raw_request().
	 */
	if (copy_from_user(&report_number, buffer, 1)) {
		ret = -EFAULT;
		goto out_free;
	}

	ret = hid_hw_raw_request(dev, report_number, buf, count, report_type,
				 HID_REQ_GET_REPORT);

	if (ret < 0)
		goto out_free;

	len = (ret < count) ? ret : count;

	if (copy_to_user(buffer, buf, len)) {
		ret = -EFAULT;
		goto out_free;
	}

	ret = len;

out_free:
	kfree(buf);
out:
	return ret;
}

static unsigned int hw_hidraw_poll(struct file *file, poll_table *wait)
{
	struct hidraw_list *list = file->private_data;

	poll_wait(file, &list->hidraw->wait, wait);
	if (list->head != list->tail)
		return POLLIN | POLLRDNORM;
	if (!list->hidraw->exist)
		return POLLERR | POLLHUP;
	return 0;
}

static int hw_hid_device_open(struct hidraw *dev)
{
	int err = 0;

	if (!dev->open++) {
		err = hid_hw_power(dev->hid, PM_HINT_FULLON);
		if (err < 0) {
			dev->open--;
			return err;
		}

		err = hid_hw_open(dev->hid);
		if (err < 0) {
			hid_hw_power(dev->hid, PM_HINT_NORMAL);
			dev->open--;
		}
	}

	return err;
}

static int hw_hidraw_open(struct inode *inode, struct file *file)
{
	struct hidraw *dev = NULL;
	struct hidraw_list *list = NULL;
	unsigned int minor = iminor(inode);
	int err;

	list = kzalloc(sizeof(*list), GFP_KERNEL);
	if (!list) {
		err = -ENOMEM;
		goto out;
	}

	mutex_lock(&minors_lock);
	if (!hw_hidraw_table[minor]) {
		err = -ENODEV;
		goto out_unlock;
	}

	d("hw: open %d:%s\n", current->pid, current->comm);

	dev = hw_hidraw_table[minor];

	list->hidraw = dev;
	mutex_init(&list->read_mutex);

	err = hw_hid_device_open(dev);
	if (err < 0)
		goto out_unlock;

	spin_lock_irq(&dev->list_lock);
	list_add_tail(&list->node, &dev->list);
	spin_unlock_irq(&dev->list_lock);

	file->private_data = list;

out_unlock:
	mutex_unlock(&minors_lock);
out:
	if (err < 0) {
		pr_err("dev open err %d", err);
		kfree(list);
	}
	return err;
}

static int hw_hidraw_fasync(int fd, struct file *file, int on)
{
	struct hidraw_list *list = file->private_data;

	return fasync_helper(fd, file, on, &list->fasync);
}

static void hw_free_list_buf(struct hidraw_list *list)
{
	int i;

	for (i = 0; i < HW_HIDRAW_BUFFER_SIZE; ++i) {
		if (list->buffer[i].value) {
			d("free list buffer 0x%pK\n", list->buffer[i].value);
			kfree(list->buffer[i].value);
		}
	}
}

static int hw_hidraw_release(struct inode *inode, struct file *file)
{
	unsigned long flags;
	struct hidraw_list *list = file->private_data;
	struct hidraw *dev = list->hidraw;

	d("HW: release %d:%s\n", current->pid, current->comm);

	mutex_lock(&minors_lock);

	spin_lock_irqsave(&dev->list_lock, flags);
	list_del(&list->node);
	spin_unlock_irqrestore(&dev->list_lock, flags);

	dev->open--;
	d("HW: release %d\n", dev->open);
	if (!dev->open) {
		if (dev->exist) {
			hid_hw_power(dev->hid, PM_HINT_NORMAL);
			hid_hw_close(dev->hid);
		} else {
			kfree(dev);
		}
	}

	hw_free_list_buf(list);
	kfree(list);

	mutex_unlock(&minors_lock);

	return 0;
}

int hw_report_event(struct hid_device *hid, u8 *data, int len)
{
	struct hidraw *dev = NULL;
	struct hidraw_list *list = NULL;
	int ret = 0;
	unsigned long flags;

	if (!hid || !data)
		return -EINVAL;
	dev = hid->hidvr;
	spin_lock_irqsave(&dev->list_lock, flags);
	list_for_each_entry(list, &dev->list, node) {
		int new_head = (list->head + 1) % HW_HIDRAW_BUFFER_SIZE;

		if (new_head == list->tail)
			continue;

		list->buffer[list->head].value = kmemdup(data, len, GFP_ATOMIC);
		if (!list->buffer[list->head].value) {
			ret = -ENOMEM;
			spin_unlock_irqrestore(&dev->list_lock, flags);
			break;
		}

		d("alloc list buffer 0x%pK\n", list->buffer[list->head].value);
		list->buffer[list->head].len = len;
		list->head = new_head;
		kill_fasync(&list->fasync, SIGIO, POLL_IN);
	}
	spin_unlock_irqrestore(&dev->list_lock, flags);

	wake_up_interruptible(&dev->wait);

	return ret;
}

static int hw_dev_add_hidraw_table(struct hidraw *dev)
{
	int minor;

	for (minor = 0; minor < HW_HIDRAW_MAX_DEVICES; minor++) {
		if (hw_hidraw_table[minor])
			continue;

		hw_hidraw_table[minor] = dev;
		break;
	}
	return minor;
}

int hw_connect(struct hid_device *hid)
{
	int minor;
	int result = 0;
	struct hidraw *dev = NULL;
	char *device_name = "hwvr";

	if (!hid)
		return -EINVAL;
	d("+\n");

	/* we accept any HID device, no matter the applications */
	dev = kzalloc(sizeof(struct hidraw), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	mutex_lock(&minors_lock);

	minor = hw_dev_add_hidraw_table(dev);
	if (minor == HW_HIDRAW_MAX_DEVICES) {
		mutex_unlock(&minors_lock);
		result = -EINVAL;
		kfree(dev);
		goto out;
	}

	dev->dev = device_create(hw_class, &hid->dev,
				MKDEV((unsigned int)hw_major, (unsigned int)minor),
				NULL, "%s%d", device_name, minor);

	d("devname: %s\n", dev_name(dev->dev));

	if (IS_ERR(dev->dev)) {
		hw_hidraw_table[minor] = NULL;
		mutex_unlock(&minors_lock);
		result = PTR_ERR(dev->dev);
		kfree(dev);
		goto out;
	}

	mutex_unlock(&minors_lock);
	init_waitqueue_head(&dev->wait);
	INIT_LIST_HEAD(&dev->list);
	spin_lock_init(&dev->list_lock);

	dev->hid = hid;
	dev->minor = minor;

	dev->exist = 1;
	hid->hidvr = dev;
	d("-\n");

out:
	return result;
}

void hw_disconnect(struct hid_device *hid)
{
	struct hidraw *hidraw = NULL;

	if (!hid)
		return;
	hidraw = hid->hidvr;

	d("+\n");
	mutex_lock(&minors_lock);

	hidraw->exist = 0;
	hw_hidraw_table[hidraw->minor] = NULL;

	device_destroy(hw_class, MKDEV((unsigned int)hw_major, hidraw->minor));

	if (hidraw->open) {
		hid_hw_close(hid);
		wake_up_interruptible(&hidraw->wait);
	} else {
		kfree(hidraw);
	}

	mutex_unlock(&minors_lock);
	d("-\n");
}

static long hw_default_ioctl_cmd(struct hidraw *dev, unsigned int cmd,
				void __user *user_arg, struct file *file)
{
	long ret;
	int len;
	struct hid_device *hid = dev->hid;

	if (_IOC_TYPE(cmd) != 'H')
		return -EINVAL;

	if (_IOC_NR(cmd) == _IOC_NR(HIDIOCSFEATURE(0))) {
		len = _IOC_SIZE(cmd);

		ret = hw_hidraw_send_report(file, user_arg,
				len, HID_FEATURE_REPORT);
		return ret;
	}
	if (_IOC_NR(cmd) == _IOC_NR(HIDIOCGFEATURE(0))) {
		len = _IOC_SIZE(cmd);

		ret = hw_hidraw_get_report(file, user_arg,
				len, HID_FEATURE_REPORT);
		return ret;
	}

	/* Begin Read-only ioctls. */
	if (_IOC_DIR(cmd) != _IOC_READ)
		return -EINVAL;

	if (_IOC_NR(cmd) == _IOC_NR(HIDIOCGRAWNAME(0))) {
		len = strlen(hid->name) + 1;

		if (len > _IOC_SIZE(cmd))
			len = _IOC_SIZE(cmd);
		ret = copy_to_user(user_arg, hid->name, len) ?
			-EFAULT : len;
		return ret;
	}

	if (_IOC_NR(cmd) == _IOC_NR(HIDIOCGRAWPHYS(0))) {
		len = strlen(hid->phys) + 1;

		if (len > _IOC_SIZE(cmd))
			len = _IOC_SIZE(cmd);
		ret = copy_to_user(user_arg, hid->phys, len) ?
			-EFAULT : len;
		return ret;
	}
	return -ENOTTY;
}

static long hw_hidraw_ioctl(struct file *file,
				unsigned int cmd,
				unsigned long arg)
{
	struct inode *inode = NULL;
	unsigned int minor;
	long ret = 0;
	struct hidraw *dev = NULL;
	void __user *user_arg = (void __user *)(uintptr_t)arg;
	__u32 len;
	struct hidraw_devinfo dinfo = {0};

	if (!user_arg || !file)
		return -EINVAL;
	inode = file->f_path.dentry->d_inode;
	minor = iminor(inode);
	mutex_lock(&minors_lock);
	dev = hw_hidraw_table[minor];
	if (!dev) {
		ret = -ENODEV;
		goto out;
	}
	switch (cmd) {
	case HIDIOCGRDESCSIZE:
		if (put_user(dev->hid->rsize, (int __user *)arg))
			ret = -EFAULT;
		break;
	case HIDIOCGRDESC:
		if (get_user(len, (int __user *)arg))
			ret = -EFAULT;
		else if (len > HID_MAX_DESCRIPTOR_SIZE - 1)
			ret = -EINVAL;
		else if (copy_to_user(user_arg + offsetof(
					struct hidraw_report_descriptor,
					value[0]),
					dev->hid->rdesc,
					min(dev->hid->rsize, len)))
			ret = -EFAULT;
		break;
	case HIDIOCGRAWINFO:
		dinfo.bustype = dev->hid->bus;
		dinfo.vendor = dev->hid->vendor;
		dinfo.product = dev->hid->product;
		if (copy_to_user(user_arg, &dinfo, sizeof(dinfo)))
			ret = -EFAULT;
		break;
	default:
		ret = hw_default_ioctl_cmd(dev, cmd, user_arg, file);
	}
out:
	mutex_unlock(&minors_lock);
	return ret;
}

static const struct file_operations hw_hid_ops = {
	.owner = THIS_MODULE,
	.read = hw_hidraw_read,
	.write = hw_hidraw_write,
	.poll = hw_hidraw_poll,
	.open = hw_hidraw_open,
	.release = hw_hidraw_release,
	.unlocked_ioctl = hw_hidraw_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = hw_hidraw_ioctl,
#endif
	.fasync = hw_hidraw_fasync,
	.llseek = noop_llseek,
};

static int hw_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int retval;

	d("+\n");
	retval = hid_parse(hdev);
	if (retval) {
		hid_err(hdev, "hw - parse failed\n");
		goto exit;
	}

	retval = hid_hw_start(hdev, HID_CONNECT_HIDINPUT | HID_CONNECT_HIDDEV |
				HID_CONNECT_FF);
	if (retval) {
		hid_err(hdev, "hw - hw start failed\n");
		goto exit;
	}

	retval = hw_connect(hdev);
	if (retval) {
		hid_err(hdev, "hw - Couldn't connect\n");
		goto exit_stop;
	}
	d("-\n");
	return 0;

exit_stop:
	hid_hw_stop(hdev);
exit:
	return retval;
}

static void hw_remove(struct hid_device *hdev)
{
	hw_disconnect(hdev);

	hid_hw_stop(hdev);
}

static int hw_raw_event(struct hid_device *hdev,
			struct hid_report *report,
			u8 *data,
			int size)
{
	struct hidraw *hidraw = hdev->hidvr;
	int retval = 0;

	if (hidraw && hidraw->exist) {
		retval = hw_report_event(hdev, data, size);
		if (retval < 0)
			pr_err("HW: raw event err %d\n", retval);
	}

	return retval;
}

static const struct hid_device_id hw_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_HUAWEI, USB_DEVICE_ID_HUAWEI_VR) },
	{ HID_USB_DEVICE(USB_VENDOR_ID_HUAWEI, USB_DEVICE_ID_HUAWEI_VR_1) },
	/* The VID&PID below adds for temporary using */
	{ HID_USB_DEVICE(0x12d1, 0x3a09) },
	{ }
};

MODULE_DEVICE_TABLE(hid, hw_devices);

static struct hid_driver hw_driver = {
		.name = "hwvr",
		.id_table = hw_devices,
		.probe = hw_probe,
		.remove = hw_remove,
		.raw_event = hw_raw_event
};

static int __init hw_init(void)
{
	int retval;
	dev_t dev_id;

	hw_class = class_create(THIS_MODULE, "hwvr");
	if (IS_ERR(hw_class))
		return PTR_ERR(hw_class);

	retval = hid_register_driver(&hw_driver);
	if (retval < 0) {
		pr_warn("Can't register driver\n");
		goto out_class;
	}

	retval = alloc_chrdev_region(&dev_id, HW_FIRST_MINOR,
			HW_HIDRAW_MAX_DEVICES, "hwvr");
	if (retval < 0) {
		pr_warn("Can't allocate chrdev region\n");
		goto out_register;
	}

	hw_major = MAJOR(dev_id);
	cdev_init(&hw_cdev, &hw_hid_ops);
	cdev_add(&hw_cdev, dev_id, HW_HIDRAW_MAX_DEVICES);

	return 0;

out_register:
	hid_unregister_driver(&hw_driver);

out_class:
	class_destroy(hw_class);

	return retval;
}

static void __exit hw_exit(void)
{
	dev_t dev_id = MKDEV((unsigned int)hw_major, 0);

	cdev_del(&hw_cdev);

	unregister_chrdev_region(dev_id, HW_HIDRAW_MAX_DEVICES);

	hid_unregister_driver(&hw_driver);

	class_destroy(hw_class);
}

module_init(hw_init);
module_exit(hw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei USB VR device driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd");
