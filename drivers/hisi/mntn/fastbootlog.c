/*
 * fastbootlog.c
 *
 * hisi fastbootlog module
 *               record fastbootlog into filesystem when booting kernel
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/stat.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/hisi/util.h> /* For mach_call_usermoduleshell */
#include <linux/uaccess.h>   /* For copy_to_user */
#include <linux/vmalloc.h>   /* For vmalloc */
#include <global_ddr_map.h>
#include <linux/module.h>
#include <linux/hisi/hisi_log.h>
#include <securec.h>

#define HISI_LOG_TAG HISI_FASTBOOTLOG_TAG
#include "blackbox/rdr_print.h"

#define FASTBOOT_DUMP_LOG_ADDR HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_BASE
#define FASTBOOT_DUMP_LOG_SIZE HISI_SUB_RESERVED_FASTBOOT_LOG_PYHMEM_SIZE

#define FASTBOOT_MAGIC_1 0x5a5a5a5a
#define FASTBOOT_MAGIC_2 0x43474244

static char *s_last_fastbootlog_buff = NULL;
static size_t s_last_fastbootlog_size;
static char *s_fastbootlog_buff = NULL;
static size_t s_fastbootlog_size;

struct fastbootlog_head {
	unsigned int magic;
	unsigned int lastlog_start;
	unsigned int lastlog_offset;
	unsigned int log_start;
	unsigned int log_offset;
};

/* read dump file content */
static ssize_t last_fastbootlog_dump_file_read(struct file *file,
					char __user *userbuf,
					size_t bytes, loff_t *off)
{
	ssize_t copy;

	if (off == NULL)
		return 0;

	if (*off > (loff_t)s_last_fastbootlog_size) {
		BB_PRINT_ERR("%s: read offset error\n", __func__);
		return 0;
	}

	if (*off == s_last_fastbootlog_size) {
		/* end of file */
		return 0;
	}

	if (userbuf == NULL) {
		/* end of file */
		return 0;
	}

	copy = (ssize_t) min(bytes, (size_t) (s_last_fastbootlog_size - *off));

	if (copy_to_user(userbuf, s_last_fastbootlog_buff + *off, copy)) {
		BB_PRINT_ERR("%s: copy to user error\n", __func__);
		copy = -EFAULT;
		goto copy_err;
	}

	*off += copy;

copy_err:
	return copy;
}

static ssize_t fastbootlog_dump_file_read(struct file *file,
					char __user *userbuf, size_t bytes,
					loff_t *off)
{
	ssize_t copy;

	if (off == NULL)
		return 0;

	if (*off > (loff_t)s_fastbootlog_size) {
		BB_PRINT_ERR("%s: read offset error\n", __func__);
		return 0;
	}

	if (*off == s_fastbootlog_size) {
		/* end of file */
		return 0;
	}

	if (userbuf == NULL) {
		/* end of file */
		return 0;
	}

	copy = (ssize_t) min(bytes, (size_t) (s_fastbootlog_size - *off));

	if (copy_to_user(userbuf, s_fastbootlog_buff + *off, copy)) {
		BB_PRINT_ERR("%s: copy to user error\n", __func__);
		copy = -EFAULT;
		goto copy_err;
	}

	*off += copy;

copy_err:
	return copy;
}

static const struct file_operations last_fastbootlog_dump_file_fops = {
	.read = last_fastbootlog_dump_file_read,
};

static const struct file_operations fastbootlog_dump_file_fops = {
	.read = fastbootlog_dump_file_read,
};

static void bootloader_logger_dump(char *start, unsigned int size,
				const char *str)
{
	unsigned int i;
	char *p = start;

	if (p == NULL)
		return;

	pr_info("*****************************%s_bootloader_log begin*****************************\n", str);
	for (i = 0; i < size; i++) {
		if (start[i] == '\0')
			start[i] = ' ';
		if (start[i] == '\n') {
			start[i] = '\0';
			pr_info("%s_bootloader_log: %s\n", str, p);
			start[i] = '\n';
			p = &start[i + 1];
		}
	}
	pr_info("******************************%s_bootloader_log end******************************\n", str);
}

void hisi_dump_bootkmsg(void)
{
	if (s_last_fastbootlog_buff != NULL)
		bootloader_logger_dump(s_last_fastbootlog_buff,
				       s_last_fastbootlog_size, "last");

	if (s_fastbootlog_buff != NULL)
		bootloader_logger_dump(s_fastbootlog_buff, s_fastbootlog_size,
				       "current");
}

static void check_fastbootlog_head(struct fastbootlog_head *head,
				int *need_dump_whole)
{
	if (head->magic != FASTBOOT_MAGIC_1 && head->magic != FASTBOOT_MAGIC_2) {
		BB_PRINT_ERR("%s: fastbootlog magic number incorrect\n",
		       __func__);
		*need_dump_whole = 1;
		return;
	}

	if (head->lastlog_start >= FASTBOOT_DUMP_LOG_SIZE ||
		head->lastlog_start < sizeof(*head) ||
		head->lastlog_offset >= FASTBOOT_DUMP_LOG_SIZE ||
		head->lastlog_offset < sizeof(*head)) {
		BB_PRINT_ERR("%s: last fastbootlog offset incorrect\n",
		       __func__);
		*need_dump_whole = 1;
		return;
	}

	if (head->log_start >= FASTBOOT_DUMP_LOG_SIZE ||
		head->log_start < sizeof(*head) ||
		head->log_offset >= FASTBOOT_DUMP_LOG_SIZE ||
		head->log_offset < sizeof(*head)) {
		BB_PRINT_ERR("%s: fastbootlog offset incorrect\n",
		       __func__);
		*need_dump_whole = 1;
		return;
	}

	*need_dump_whole = 0;
}

static int dump_init_save_log(struct fastbootlog_head *head, const char *fastbootlog_buff)
{
	const char *log_start = NULL;
	unsigned int log_size;
	unsigned int tmp_len;
	int ret;

	log_start = fastbootlog_buff + head->log_start;
	if (head->log_offset < head->log_start) {
		tmp_len = FASTBOOT_DUMP_LOG_SIZE - head->log_start;
		log_size = tmp_len + head->log_offset - sizeof(*head);

		s_fastbootlog_buff = vmalloc(log_size);
		if (s_fastbootlog_buff == NULL) {
			BB_PRINT_ERR(
				"%s: fail to vmalloc %#x bytes s_fastbootlog_buff\n",
				__func__, log_size);
			return -1;
		}
		ret = memcpy_s(s_fastbootlog_buff, log_size, log_start, tmp_len);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			vfree(s_fastbootlog_buff);
			s_fastbootlog_buff = NULL;
			return -1;
		}
		log_start = fastbootlog_buff + sizeof(*head);
		ret = memcpy_s(s_fastbootlog_buff + tmp_len, log_size - tmp_len, log_start,
				log_size - tmp_len);
		if (ret != EOK) {
			BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
			vfree(s_fastbootlog_buff);
			s_fastbootlog_buff = NULL;
			return -1;
		}
		s_fastbootlog_size = log_size;
	} else {
		log_size = head->log_offset - head->log_start;
		if (log_size > 0) {
			s_fastbootlog_buff = vmalloc(log_size);
			if (s_fastbootlog_buff == NULL) {
				BB_PRINT_ERR(
					"%s: fail to vmalloc %#x bytes s_fastbootlog_buff\n",
					__func__, log_size);
				return -1;
			}
			ret = memcpy_s(s_fastbootlog_buff, log_size, log_start, log_size);
			if (ret != EOK) {
				BB_PRINT_ERR("%s():%d:memcpy_s fail!\n", __func__, __LINE__);
				vfree(s_fastbootlog_buff);
				s_fastbootlog_buff = NULL;
				return -1;
			}
			s_fastbootlog_size = log_size;
		}
	}

	return 0;
}

static int dump_init_save_lastlog(struct fastbootlog_head *head, const char *fastbootlog_buff)
{
	const char *lastlog_start = NULL;
	unsigned int lastlog_size;
	unsigned int tmp_len;

	lastlog_start = fastbootlog_buff + head->lastlog_start;
	if (head->lastlog_offset < head->lastlog_start) {
		tmp_len = FASTBOOT_DUMP_LOG_SIZE - head->lastlog_start;
		lastlog_size = tmp_len + head->lastlog_offset - sizeof(*head);

		s_last_fastbootlog_buff = vmalloc(lastlog_size);
		if (s_last_fastbootlog_buff == NULL) {
			BB_PRINT_ERR(
				"%s: fail to vmalloc %#x bytes s_last_fastbootlog_buff\n",
				__func__, lastlog_size);
			return -1;
		}

		if (memcpy_s(s_last_fastbootlog_buff, lastlog_size, lastlog_start, tmp_len) != EOK) {
			BB_PRINT_ERR("[%s:%d]: memcpy_s err\n]", __func__, __LINE__);
			vfree(s_last_fastbootlog_buff);
			s_last_fastbootlog_buff = NULL;
			return -1;
		}

		lastlog_start = fastbootlog_buff + sizeof(*head);

		if (memcpy_s(s_last_fastbootlog_buff + tmp_len, lastlog_size - tmp_len, lastlog_start,
			lastlog_size - tmp_len) != EOK) {
			BB_PRINT_ERR("[%s:%d]: memcpy_s err\n]", __func__, __LINE__);
			vfree(s_last_fastbootlog_buff);
			s_last_fastbootlog_buff = NULL;
			return -1;
		}

		s_last_fastbootlog_size = lastlog_size;
	} else {
		lastlog_size = head->lastlog_offset - head->lastlog_start;
		if (lastlog_size > 0) {
			s_last_fastbootlog_buff = vmalloc(lastlog_size);
			if (s_last_fastbootlog_buff == NULL) {
				BB_PRINT_ERR(
					"%s: fail to vmalloc %#x bytes s_last_fastbootlog_buff\n",
					__func__, lastlog_size);
				return -1;
			}
			if (memcpy_s(s_last_fastbootlog_buff, lastlog_size, lastlog_start, lastlog_size) != EOK) {
				BB_PRINT_ERR("[%s:%d]: memcpy_s err\n]", __func__, __LINE__);
				vfree(s_last_fastbootlog_buff);
				s_last_fastbootlog_buff = NULL;
				return -1;
			}

			s_last_fastbootlog_size = lastlog_size;
		}
	}

	return 0;
}

/*
 * Description :  fastbootlog init
 *                check emmc leaves log to record
 *                if there is log, launch work queue, and create /proc/balong/fastbootlog
 * Input:         NA
 * Output:        NA
 * Return:        OK:success
 */
static int __init fastbootlog_dump_init(void)
{
	char *fastbootlog_buff = NULL;
	struct fastbootlog_head *head = NULL;
	int use_ioremap = 0;
	int need_dump_whole = 0;
	int ret = 0;

	if (!check_himntn(HIMNTN_GOBAL_RESETLOG))
		return ret;

	if (pfn_valid(__phys_to_pfn(FASTBOOT_DUMP_LOG_ADDR))) {
		fastbootlog_buff = phys_to_virt(FASTBOOT_DUMP_LOG_ADDR);
	} else {
		use_ioremap = 1;
		fastbootlog_buff =
		    ioremap_wc(FASTBOOT_DUMP_LOG_ADDR, FASTBOOT_DUMP_LOG_SIZE);
	}
	if (fastbootlog_buff == NULL) {
		BB_PRINT_ERR(
		       "%s: fail to get the virtual address of fastbootlog\n", __func__);
		return -1;
	}

	head = (struct fastbootlog_head *)fastbootlog_buff;
	check_fastbootlog_head(head, &need_dump_whole);
	if (need_dump_whole) {
		head->lastlog_start = 0;
		head->lastlog_offset = 0;
		head->log_start = 0;
		head->log_offset = FASTBOOT_DUMP_LOG_SIZE;
	}

	ret = dump_init_save_lastlog(head, fastbootlog_buff);
	if (ret == -1)
		goto out;

	ret = dump_init_save_log(head, fastbootlog_buff);
	if (ret == -1)
		goto out;

out:
	if (use_ioremap && fastbootlog_buff != NULL)
		iounmap(fastbootlog_buff);

	if (s_last_fastbootlog_buff != NULL)
		balong_create_log_proc_entry("last_fastboot_log", S_IRUSR | S_IRGRP,
					     &last_fastbootlog_dump_file_fops, NULL);

	if (s_fastbootlog_buff != NULL)
		balong_create_log_proc_entry("fastboot_log", S_IRUSR | S_IRGRP,
					     &fastbootlog_dump_file_fops, NULL);

	return ret;
}

module_init(fastbootlog_dump_init);

/*
 * Description :  fastbootlog exit
 *                destroy the workqueue
 * Input:         NA
 * Output:        NA
 * Return:        NA
 */
static void __exit fastbootlog_dump_exit(void)
{
	balong_remove_log_proc_entry("last_fastboot_log");
	balong_remove_log_proc_entry("fastboot_log");

	if (s_last_fastbootlog_buff != NULL) {
		vfree(s_last_fastbootlog_buff);
		s_last_fastbootlog_buff = NULL;
	}
	s_last_fastbootlog_size = 0;

	if (s_fastbootlog_buff != NULL) {
		vfree(s_fastbootlog_buff);
		s_fastbootlog_buff = NULL;
	}
	s_fastbootlog_size = 0;
}

module_exit(fastbootlog_dump_exit);
