/*
 * omnivision TCM touchscreen driver
 *
 * Copyright (C) 2017-2018 omnivision Incorporated. All rights reserved.

 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND omnivision
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL omnivision BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF omnivision WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, omnivision'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/cdev.h>
#include <linux/gpio.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include "omnivision_tcm_core.h"

#define CHAR_DEVICE_NAME "tcm"

#define CONCURRENT true

#define DEVICE_IOC_MAGIC 's'
#define DEVICE_IOC_RESET _IO(DEVICE_IOC_MAGIC, 0) /* 0x00007300 */
#define DEVICE_IOC_IRQ _IOW(DEVICE_IOC_MAGIC, 1, int) /* 0x40047301 */
#define DEVICE_IOC_RAW _IOW(DEVICE_IOC_MAGIC, 2, int) /* 0x40047302 */
#define DEVICE_IOC_CONCURRENT _IOW(DEVICE_IOC_MAGIC, 3, int) /* 0x40047303 */

struct device_hcd {
	dev_t dev_num;
	bool raw_mode;
	bool concurrent;
	unsigned int ref_count;
	struct cdev char_dev;
	struct class *class;
	struct device *device;
	struct ovt_tcm_buffer out;
	struct ovt_tcm_buffer resp;
	struct ovt_tcm_buffer report;
	struct ovt_tcm_hcd *tcm_hcd;
};

DECLARE_COMPLETION(ovt_device_remove_complete);

static struct device_hcd *device_hcd;

static int rmidev_major_num;

static void device_capture_touch_report(unsigned int count)
{
	int retval;
	unsigned char id;
	unsigned int idx;
	unsigned int size;
	unsigned char *data = NULL;
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;
	static bool report;
	static unsigned int offset;
	static unsigned int remaining_size;

	if (count < 2)
		return;

	data = &device_hcd->resp.buf[0];

	if (data[0] != MESSAGE_MARKER)
		return;

	id = data[1];

	size = 0;

	LOCK_BUFFER(device_hcd->report);

	switch (id) {
	case REPORT_TOUCH:
		if (count >= 4) {
			remaining_size = le2_to_uint(&data[2]);
		} else {
			report = false;
			goto exit;
		}
		retval = ovt_tcm_alloc_mem(tcm_hcd,
				&device_hcd->report,
				remaining_size);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to allocate memory for device_hcd->report.buf");
			report = false;
			goto exit;
		}
		idx = 4;
		size = count - idx;
		offset = 0;
		report = true;
		break;
	case STATUS_CONTINUED_READ:
		if (report == false)
			goto exit;
		if (count >= 2) {
			idx = 2;
			size = count - idx;
		}
		break;
	default:
		goto exit;
	}

	if (size) {
		size = MIN(size, remaining_size);
		retval = secure_cpydata(&device_hcd->report.buf[offset],
				device_hcd->report.buf_size - offset,
				&data[idx],
				count - idx,
				size);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to copy touch report data");
			report = false;
			goto exit;
		} else {
			offset += size;
			remaining_size -= size;
			device_hcd->report.data_length += size;
		}
	}

	if (remaining_size)
		goto exit;

	LOCK_BUFFER(tcm_hcd->report.buffer);

	tcm_hcd->report.buffer.buf = device_hcd->report.buf;
	tcm_hcd->report.buffer.buf_size = device_hcd->report.buf_size;
	tcm_hcd->report.buffer.data_length = device_hcd->report.data_length;

	tcm_hcd->report_touch();

	UNLOCK_BUFFER(tcm_hcd->report.buffer);

	report = false;

exit:
	UNLOCK_BUFFER(device_hcd->report);

	return;
}

static int device_capture_touch_report_config(unsigned int count)
{
	int retval;
	unsigned int size;
	unsigned int buf_size;
	unsigned char *data = NULL;
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	if (device_hcd->raw_mode) {
		if (count < 3) {
			OVT_LOG_ERR(
					"Invalid write data");
			return -EINVAL;
		}

		size = le2_to_uint(&device_hcd->out.buf[1]);
		if (count - 3 < size) {
			OVT_LOG_ERR(
					"Incomplete write data");
			return -EINVAL;
		}

		if (!size)
			return 0;

		data = &device_hcd->out.buf[3];
		buf_size = device_hcd->out.buf_size - 3;
	} else {
		size = count - 1;

		if (!size)
			return 0;

		data = &device_hcd->out.buf[1];
		buf_size = device_hcd->out.buf_size - 1;
	}

	LOCK_BUFFER(tcm_hcd->config);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&tcm_hcd->config,
			size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory for tcm_hcd->config.buf");
		UNLOCK_BUFFER(tcm_hcd->config);
		return retval;
	}

	retval = secure_cpydata(tcm_hcd->config.buf,
			tcm_hcd->config.buf_size,
			data,
			buf_size,
			size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy touch report config data");
		UNLOCK_BUFFER(tcm_hcd->config);
		return retval;
	}

	tcm_hcd->config.data_length = size;

	UNLOCK_BUFFER(tcm_hcd->config);

	return 0;
}

static int irq_status = 1;

#ifdef HAVE_UNLOCKED_IOCTL
static long device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#else
static int device_ioctl(struct inode *inp, struct file *filp, unsigned int cmd,
		unsigned long arg)
#endif
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	mutex_lock(&tcm_hcd->extif_mutex);

	retval = 0;

	switch (cmd) {
	case DEVICE_IOC_RESET:
		// retval = tcm_hcd->reset_n_reinit(tcm_hcd, false, true);
		break;
	case DEVICE_IOC_IRQ:
		OVT_LOG_INFO("Try to enable irq %ld\n", arg);
		if (arg == 0) {
		  if (irq_status) {
			disable_irq(tcm_hcd->ovt_tcm_platform_data->irq_id);
			irq_status = 0;
		  }
		}
		else if (arg == 1) {
		  if (irq_status == 0) {
			irq_status = 1;
			enable_irq(tcm_hcd->ovt_tcm_platform_data->irq_id);
		  }
		}
		break;
	case DEVICE_IOC_RAW:
		if (arg == 0) {
			device_hcd->raw_mode = false;
#ifdef WATCHDOG_SW
			tcm_hcd->update_watchdog(tcm_hcd, true);
#endif
		} else if (arg == 1) {
			device_hcd->raw_mode = true;
#ifdef WATCHDOG_SW
			tcm_hcd->update_watchdog(tcm_hcd, false);
#endif
		}
		break;
	case DEVICE_IOC_CONCURRENT:
		if (arg == 0)
			device_hcd->concurrent = false;
		else if (arg == 1)
			device_hcd->concurrent = true;
		break;
	default:
		retval = -ENOTTY;
		break;
	}

	mutex_unlock(&tcm_hcd->extif_mutex);

	return retval;
}

static loff_t device_llseek(struct file *filp, loff_t off, int whence)
{
	return -EINVAL;
}

static ssize_t device_read(struct file *filp, char __user *buf,
		size_t count, loff_t *f_pos)
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	if (count == 0)
		return 0;

	mutex_lock(&tcm_hcd->extif_mutex);

	LOCK_BUFFER(device_hcd->resp);

	if (device_hcd->raw_mode) {
		retval = ovt_tcm_alloc_mem(tcm_hcd,
				&device_hcd->resp,
				count);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to allocate memory for device_hcd->resp.buf");
			UNLOCK_BUFFER(device_hcd->resp);
			goto exit;
		}

		retval = tcm_hcd->read_message(tcm_hcd,
				device_hcd->resp.buf,
				count);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to read message");
			UNLOCK_BUFFER(device_hcd->resp);
			goto exit;
		}
	} else {
		if (count != device_hcd->resp.data_length) {
			OVT_LOG_ERR(
					"Invalid length information");
			UNLOCK_BUFFER(device_hcd->resp);
			retval = -EINVAL;
			goto exit;
		}
	}

	if (copy_to_user(buf, device_hcd->resp.buf, count)) {
		OVT_LOG_ERR(
				"Failed to copy data to user space");
		UNLOCK_BUFFER(device_hcd->resp);
		retval = -EINVAL;
		goto exit;
	}

	if (!device_hcd->concurrent)
		goto skip_concurrent;

	// if (tcm_hcd->report_touch == NULL) {
	// 	OVT_LOG_ERR(
	// 			"Unable to report touch");
	// 	device_hcd->concurrent = false;
	// }

	if (device_hcd->raw_mode) {
		device_capture_touch_report(count);
	}
	

skip_concurrent:
	UNLOCK_BUFFER(device_hcd->resp);

	retval = count;

exit:
	mutex_unlock(&tcm_hcd->extif_mutex);

	return retval;
}

static ssize_t device_write(struct file *filp, const char __user *buf,
		size_t count, loff_t *f_pos)
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	if (count == 0)
		return 0;

	mutex_lock(&tcm_hcd->extif_mutex);

	LOCK_BUFFER(device_hcd->out);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&device_hcd->out,
			count == 1 ? count + 1 : count);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory for device_hcd->out.buf");
		UNLOCK_BUFFER(device_hcd->out);
		goto exit;
	}

	if (copy_from_user(device_hcd->out.buf, buf, count)) {
		OVT_LOG_ERR(
				"Failed to copy data from user space");
		UNLOCK_BUFFER(device_hcd->out);
		retval = -EINVAL;
		goto exit;
	}

	LOCK_BUFFER(device_hcd->resp);

	if (device_hcd->raw_mode) {
		retval = tcm_hcd->write_message(tcm_hcd,
				device_hcd->out.buf[0],
				&device_hcd->out.buf[1],
				count - 1,
				NULL,
				NULL,
				NULL,
				NULL,
				0);
	} else {
		mutex_lock(&tcm_hcd->reset_mutex);
		retval = tcm_hcd->write_message(tcm_hcd,
				device_hcd->out.buf[0],
				&device_hcd->out.buf[1],
				count - 1,
				&device_hcd->resp.buf,
				&device_hcd->resp.buf_size,
				&device_hcd->resp.data_length,
				NULL,
				0);
		mutex_unlock(&tcm_hcd->reset_mutex);
	}
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write command 0x%02x\n",
				device_hcd->out.buf[0]);
		UNLOCK_BUFFER(device_hcd->resp);
		UNLOCK_BUFFER(device_hcd->out);
		goto exit;
	}

	if (count && device_hcd->out.buf[0] == CMD_SET_TOUCH_REPORT_CONFIG) {
		retval = device_capture_touch_report_config(count);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to capture touch report config");
		}
	}

	UNLOCK_BUFFER(device_hcd->out);

	if (device_hcd->raw_mode)
		retval = count;
	else
		retval = device_hcd->resp.data_length;

	UNLOCK_BUFFER(device_hcd->resp);

exit:
	mutex_unlock(&tcm_hcd->extif_mutex);

	return retval;
}

static int device_open(struct inode *inp, struct file *filp)
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	mutex_lock(&tcm_hcd->extif_mutex);

	if (device_hcd->ref_count < 1) {
		device_hcd->ref_count++;
		retval = 0;
	} else {
		retval = -EACCES;
	}

	mutex_unlock(&tcm_hcd->extif_mutex);

	return retval;
}

static int device_release(struct inode *inp, struct file *filp)
{
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	mutex_lock(&tcm_hcd->extif_mutex);

	if (device_hcd->ref_count)
		device_hcd->ref_count--;

	mutex_unlock(&tcm_hcd->extif_mutex);

	return 0;
}

static char *device_devnode(struct device *dev, umode_t *mode)
{
	if (!mode)
		return NULL;

	*mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

	return kasprintf(GFP_KERNEL, "%s/%s", PLATFORM_DRIVER_NAME,
			dev_name(dev));
}

static int device_create_class(void)
{
	struct ovt_tcm_hcd *tcm_hcd = device_hcd->tcm_hcd;

	if (device_hcd->class != NULL)
		return 0;

	device_hcd->class = class_create(THIS_MODULE, PLATFORM_DRIVER_NAME);

	if (IS_ERR(device_hcd->class)) {
		OVT_LOG_ERR(
				"Failed to create class");
		return -ENODEV;
	}

	device_hcd->class->devnode = device_devnode;

	return 0;
}

static const struct file_operations device_fops = {
	.owner = THIS_MODULE,
#ifdef HAVE_UNLOCKED_IOCTL
	.unlocked_ioctl = device_ioctl,
#ifdef HAVE_COMPAT_IOCTL
	.compat_ioctl = device_ioctl,
#endif
#else
	.ioctl = device_ioctl,
#endif
	.llseek = device_llseek,
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
};

int ovt_tcm_device_init(struct ovt_tcm_hcd *tcm_hcd)
{
	int retval;
	dev_t dev_num;
	// const struct ovt_tcm_board_data *bdata = tcm_hcd->hw_if->bdata;

	device_hcd = kzalloc(sizeof(*device_hcd), GFP_KERNEL);
	if (!device_hcd) {
		OVT_LOG_ERR(
				"Failed to allocate memory for device_hcd");
		return -ENOMEM;
	}

	device_hcd->tcm_hcd = tcm_hcd;

	device_hcd->concurrent = CONCURRENT;

	INIT_BUFFER(device_hcd->out, false);
	INIT_BUFFER(device_hcd->resp, false);
	INIT_BUFFER(device_hcd->report, false);

	if (rmidev_major_num) {
		dev_num = (dev_t)MKDEV(rmidev_major_num, 0);
		retval = register_chrdev_region(dev_num, 1,
				PLATFORM_DRIVER_NAME);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to register char device");
			goto err_register_chrdev_region;
		}
	} else {
		retval = alloc_chrdev_region(&dev_num, 0, 1,
				PLATFORM_DRIVER_NAME);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to allocate char device");
			goto err_alloc_chrdev_region;
		}

		rmidev_major_num = MAJOR(dev_num);
	}

	device_hcd->dev_num = dev_num;

	cdev_init(&device_hcd->char_dev, &device_fops);

	retval = cdev_add(&device_hcd->char_dev, dev_num, 1);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to add char device");
		goto err_add_chardev;
	}

	retval = device_create_class();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to create class");
		goto err_create_class;
	}

	device_hcd->device = device_create(device_hcd->class, NULL,
			device_hcd->dev_num, NULL, CHAR_DEVICE_NAME"%d",
			MINOR(device_hcd->dev_num));
	if (IS_ERR(device_hcd->device)) {
		OVT_LOG_ERR(
				"Failed to create device");
		retval = -ENODEV;
		goto err_create_device;
	}

	return 0;

err_create_device:
	class_destroy(device_hcd->class);

err_create_class:
	cdev_del(&device_hcd->char_dev);

err_add_chardev:
	unregister_chrdev_region(dev_num, 1);

err_alloc_chrdev_region:
err_register_chrdev_region:
	RELEASE_BUFFER(device_hcd->report);
	RELEASE_BUFFER(device_hcd->resp);
	RELEASE_BUFFER(device_hcd->out);

	kfree(device_hcd);
	device_hcd = NULL;

	return retval;
}