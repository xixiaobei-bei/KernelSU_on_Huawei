/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include "securec.h"
#include "product_config.h"
#include "dms_debug.h"


/*
 * The protosol stack prints the file macro definition under the dot mode.
 */

#define THIS_FILE_ID PS_FILE_ID_DMS_FILE_NODE_C

#define DMS_SLICE_FILE_LEN		16
#define DMS_SLICE_FILE_NAME		"getslice"

#define DMS_MODEM_STATUS_FILE_LEN	3
#define DMS_MODEM_STATUS_FILE_NAME	"modemstatus"

unsigned char modem_status[DMS_MODEM_STATUS_FILE_LEN] = {0};

/*
 * dms_set_modem_status - Set modem status with modem_id to 1.
 * @modem_id: modem id
 *
 * Return 0 on success, negative on failure.
 */
int dms_set_modem_status(unsigned short modem_id)
{
	if (modem_id >= DMS_MODEM_STATUS_FILE_LEN)
		return -EFAULT;

	modem_status[modem_id] = 1;
	return 0;
}

/*
 * dms_reset_modem_status - Reset function of all modem status.
 */
void dms_reset_modem_status(void)
{
	(void)memset_s(modem_status, sizeof(modem_status), 0, sizeof(modem_status));
}

/*
 * dms_file_slice_read - Read function of virtual file.
 * @filp: file handle
 * @user_buf : the user space buffer to write to
 * @count: the size of the user space buffer
 * @ppos: the current position in the buffer
 *
 * On success, the number of bytes read is returned, or negative value is
 * returned on error.
 */
STATIC ssize_t dms_file_slice_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	char buf[DMS_SLICE_FILE_LEN + 1];
	u64 slice_val;
	u32 h32_val = 0;
	u32 l32_val = 0;
	ssize_t ret = 0;

	if (*ppos > 0)
		return 0;
	if (mdrv_timer_get_accuracy_timestamp(&h32_val, &l32_val)) {
		DMS_LOGE("get stamp error.");
		return -EFAULT;
	}

	slice_val = h32_val;
	slice_val = (slice_val << 32) | l32_val;

	(void)memset_s(buf, sizeof(buf), 0, sizeof(buf));
	scnprintf(buf, sizeof(buf), "%llx", slice_val);

	ret = simple_read_from_buffer(user_buf, count, ppos, buf, strlen(buf));
	if (ret < 0) {
		DMS_LOGE("input buf read error, return: %zd.", ret);
	}

	return ret;
}

static const struct file_operations dms_file_slice_ops = {
	.owner = THIS_MODULE,
	.read = dms_file_slice_read,
};

/*
 * dms_file_slice_init - Init function of virtual file.
 *
 * Return 0 on success, negative on failure.
 */
STATIC int dms_file_slice_init(void)
{
	struct proc_dir_entry *proc_entry = NULL;

	proc_entry = proc_create(DMS_SLICE_FILE_NAME, 0770,
				 NULL, &dms_file_slice_ops);
	if (proc_entry == NULL)
		return -ENOMEM;

	return 0;
}

/*
 * dms_file_slice_exit - Exit function of virtual file.
 */
STATIC void dms_file_slice_exit(void)
{
	remove_proc_entry(DMS_SLICE_FILE_NAME, NULL);
}

/*
 * dms_file_modem_status_read - Read function of virtual file.
 * @filp: file handle
 * @user_buf : the user space buffer to write to
 * @count: the size of the user space buffer
 * @ppos: the current position in the buffer
 *
 * On success, the number of bytes read is returned, or negative value is
 * returned on error.
 */
STATIC ssize_t dms_file_modem_status_read(struct file *file, char __user *user_buf,
					  size_t count, loff_t *ppos)
{
	ssize_t ret;

	if (*ppos > 0)
		return 0;

	DMS_LOGI("entry, status: %d, %d, %d.",
		     modem_status[0], modem_status[1], modem_status[2]);

	ret = simple_read_from_buffer(user_buf, count, ppos, modem_status, DMS_MODEM_STATUS_FILE_LEN);
	if (ret < 0) {
		DMS_LOGE("input buf read error, return: %zd.", ret);
	}

	return ret;
}

static const struct file_operations dms_file_modem_status_ops = {
	.owner = THIS_MODULE,
	.read  = dms_file_modem_status_read,
};

/*
 * dms_file_modem_status_init - Init function of modem status file.
 *
 * Return 0 on success, negative on failure.
 */
STATIC int dms_file_modem_status_init(void)
{
	struct proc_dir_entry *proc_entry = NULL;

	proc_entry = proc_create(DMS_MODEM_STATUS_FILE_NAME, 0770,
				 VOS_NULL_PTR, &dms_file_modem_status_ops);
	if (proc_entry == NULL)
		return -ENOMEM;

	return 0;
}

/*
 * dms_file_modem_status_exit - Exit function of modem status file.
 */
STATIC void dms_file_modem_status_exit(void)
{
	remove_proc_entry(DMS_MODEM_STATUS_FILE_NAME, NULL);
}


/*
 * dms_file_init - Init function of file.
 *
 * Return 0 on success, negative on failure.
 */
/*lint -save -e801*/
int __init dms_file_init(void)
{
	int ret = 0;

	ret = dms_file_slice_init();
	if (ret)
		goto err_silce;

	ret = dms_file_modem_status_init();
	if (ret)
		goto err_modem_status;

	return 0;

err_modem_status:
	dms_file_slice_exit();
err_silce:
	return ret;
}
/*lint -restore +e801*/

/*
 * dms_file_exit - Exit function of file.
 *
 * Return 0 on success, negative on failure.
 */
void __exit dms_file_exit(void)
{
	dms_file_slice_exit();
	dms_file_modem_status_exit();
}

/* This function is called on driver initialization and exit */
#ifndef CONFIG_HISI_BALONG_MODEM_MODULE
module_init(dms_file_init);
module_exit(dms_file_exit);
#endif /* CONFIG_HISI_BALONG_MODEM_MODULE */

