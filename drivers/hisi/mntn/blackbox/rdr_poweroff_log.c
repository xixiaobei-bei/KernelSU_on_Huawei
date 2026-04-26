 /*
  * rdr_poweroff_log.c
  *
  * poweroff log driver for user-mode application
  *
  * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
  *
  * This software is licensed under the terms of the GNU General Public
  * License version 2, as published by the Free Software Foundation, and
  * may be copied, distributed, and modified under those terms.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  */
#include "rdr_poweroff_log.h"
#include <linux/proc_fs.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/hisi/util.h>
#include <linux/hisi/hisi_bootup_keypoint.h>
#include <linux/hisi/rdr_pub.h>
#include <securec.h>
#include "rdr_inner.h"
#include "rdr_print.h"

static ssize_t poweroff_reason_write(struct file *file,
	const char __user *userbuf, size_t bytes, loff_t *off)
{
	int ret;
	char reason[SHTDOWN_REASON_MAXLEN] = {0};
	char buf[HISTORY_LOG_SIZE] = {0};
	char date[DATATIME_MAXLEN] = {0};
	static int g_times = 0;
	mm_segment_t fs;

	if (g_times == 1) {
		BB_PRINT_ERR("[%s] already saved buf one times\n", __func__);
		return -EPERM;
	}
	if (userbuf == NULL || bytes == 0 || bytes >= SHTDOWN_REASON_MAXLEN) {
		BB_PRINT_ERR("[%s] the userbuf is NULL. Or the size is invalid, bytes=%lu\n", __func__, bytes);
		return -EPERM;
	}
	if (check_himntn(HIMNTN_GOBAL_RESETLOG) == 0)
		return -EPERM;
	if (copy_from_user(reason, userbuf, bytes) != 0) {
		BB_PRINT_ERR("[%s] copy shutdown reason fail\n", __func__);
		return -EPERM;
	}
	ret = snprintf_s(date, sizeof(date), sizeof(date) - 1, "%s-%08lld",
		rdr_get_timestamp(), rdr_get_tick());
	if (ret < 0) {
		BB_PRINT_ERR("[%s] copy date fail\n", __func__);
		return -EPERM;
	}
	ret = snprintf_s(buf, sizeof(buf), sizeof(buf) - 1,
		"system exception core [AP], reason [SHUTDOWN:%s], time [%s], sysreboot [true], bootup_keypoint [%u], category [SHUTDOWN]\n",
		reason, date, get_boot_keypoint());
	if (ret < 0) {
		BB_PRINT_ERR("[%s] copy shutdown log to buf fail\n", __func__);
		return -EPERM;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);//lint !e501
	if (rdr_wait_partition(PATH_MNTN_PARTITION, RDR_POWEROFF_TIME_OUT) != 0) {
		BB_PRINT_ERR("[%s] path %s is invalid\n", __func__, PATH_MNTN_PARTITION);
		set_fs(fs);
		return -EPERM;
	}
	if (rdr_save_buf(buf, strlen(buf)) != 0) {
		set_fs(fs);
		return -EPERM;
	}
	g_times = 1;
	set_fs(fs);
	return bytes;
}

static const struct file_operations poweroff_reason_fops = {
	.write = poweroff_reason_write,
};

static int poweroff_log_creat_procfs(void)
{
	struct proc_dir_entry *poweroff_reason = NULL;

	poweroff_reason = proc_create("poweroff_reason", POWEROFF_REASON_FILE_LIMIT, NULL,
		&poweroff_reason_fops);
	if (poweroff_reason == NULL) {
		BB_PRINT_ERR("[%s] poweroff_reason procfs create failed\n", __func__);
		return -1;
	}
	return 0;
}

int rdr_poweroff_log_init(void)
{
	if (poweroff_log_creat_procfs() < 0) {
		BB_PRINT_ERR("[%s]history log create procfs failed\n", __func__);
		return -1;
	}
	BB_PRINT_PN("rdr poweroff log init success\n");
	return 0;
}
