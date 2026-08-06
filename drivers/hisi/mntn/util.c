/*
 * modem platform misc utilities function
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/kmod.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/sysctl.h>
#include <linux/hisi/util.h>
#include <linux/uaccess.h> /* For copy_to_user */
#include <linux/pstore_ram.h>
#include <linux/delay.h>
#include <pr_log.h>
#define PR_LOG_TAG UTIL_TAG
#include <securec.h>
#include "blackbox/rdr_print.h"

static char himntn[HIMNTN_VALID_SIZE + 1] = { '\0' };

/*
 * Determine if the nv is open
 * input: feature:position of himntn
 */
int check_himntn(int feature)
{
	int ret = 0;

	if (feature >= HIMNTN_BOTTOM || feature < 0)
		goto out;

	if (himntn[feature] == '1')
		ret = 1;

out:
	return ret;
}

int get_himntn_value(int feature)
{
	int ret = 0;

	if (feature >= HIMNTN_BOTTOM || feature < 0)
		goto out;

	ret = himntn[feature];

out:
	return ret;
}

static int __init early_parse_himntn_cmdline(char *himntn_cmdline)
{
	int ret;

	if (!himntn_cmdline) {
		BB_PRINT_ERR("[%s]himntn_cmdline is null!\n", __func__);
		return -1;
	}

	(void)memset_s(himntn, HIMNTN_VALID_SIZE + 1, 0x0, HIMNTN_VALID_SIZE + 1);

	if (strlen(himntn_cmdline) > HIMNTN_VALID_SIZE) {
		BB_PRINT_ERR("error: invalid himn cmdline size!\n");
		return -1;
	}
	ret = memcpy_s(himntn, HIMNTN_VALID_SIZE, himntn_cmdline, strlen(himntn_cmdline));
	if (ret != EOK)
		return -1;

	return 0;
}
early_param("himntn", early_parse_himntn_cmdline);

#define PBUF_MAXSIZE 128
void mntn_print_to_ramconsole(const char *fmt, ...)
{
	char pbuf[PBUF_MAXSIZE] = { 0 };
	va_list ap;

	if (!fmt)
		return;

	va_start(ap, fmt);
	vsnprintf(pbuf, PBUF_MAXSIZE, fmt, ap);
	va_end(ap);
#ifdef CONFIG_PSTORE
	ramoops_console_write_buf(pbuf, strlen(pbuf));
#endif
}

/*
 * Description: transfer string to int
 */
u32 atoi(char *s)
{
	char *p = s;
	char c;
	u64 ret = 0;

	if (!s)
		return 0;
	c = *p;
	while (c != '\0') {
		if (c >= '0' && c <= '9') {
			/* change to Decimal */
			ret *= 10;
			ret += (u64)((unsigned char)c - '0');
			if (ret > U32_MAX)
				return 0;
		} else {
			break;
		}
		p++;
		c = *p;
	}
	return (u32)ret;
}

/*
 * Description: checksum32
 */
u32 checksum32(u32 *addr, u32 count)
{
	u64 sum = 0;
	u32 i;

	while (count > sizeof(u32) - 1) {
		/*  This is the inner loop */
		sum += *(addr++);
		count -= sizeof(u32);
	}

	if (count > 0) {
		u32 left = 0;

		i = 0;
		while (i <= count) {
			*((u8 *)&left + i) = *((u8 *)addr + i);
			i++;
		}

		sum += left;
	}

	while (sum>>32)
		sum = (sum & 0xffffffff) + (sum >> 32);

	return (~sum);
}

static struct proc_dir_entry *proc_dfx_entry = NULL;
static struct proc_dir_entry *proc_dfx_stats_entry = NULL;
static struct proc_dir_entry *proc_dfx_memory_entry = NULL;
static struct proc_dir_entry *proc_dfx_log_entry = NULL;
static struct proc_dir_entry *proc_dfx_pstore = NULL;
static int __init dfx_proc_fs_init(void)
{
	proc_dfx_entry = proc_mkdir("balong", NULL);
	if (!proc_dfx_entry)
		panic("cannot create dfx proc entry");

	proc_dfx_stats_entry = proc_mkdir("stats", proc_dfx_entry);
	if (!proc_dfx_stats_entry)
		panic("cannot create dfx sys proc entry");

	proc_dfx_memory_entry = proc_mkdir("memory", proc_dfx_entry);
	if (!proc_dfx_memory_entry)
		panic("cannot create dfx memory proc entry");

	proc_dfx_log_entry = proc_mkdir("log", proc_dfx_entry);
	if (!proc_dfx_log_entry)
		panic("cannot create dfx log proc entry");

	proc_dfx_pstore = proc_mkdir("pstore", proc_dfx_entry);
	if (!proc_dfx_pstore)
		panic("cannot create dfx pstore proc entry");
	return 0;
}

core_initcall(dfx_proc_fs_init);

static inline struct proc_dir_entry *dfx_create_proc_entry(const char *name, mode_t mode,
				struct proc_dir_entry *parent, const struct file_operations *proc_fops, void *data)
{
#ifdef CONFIG_PROC_FS
	return proc_create_data(name, mode, parent, proc_fops, data);
#endif

	return NULL;
}

static inline void dfx_remove_proc_entry(const char *name,
				struct proc_dir_entry *parent)
{
#ifdef CONFIG_PROC_FS
	remove_proc_entry(name, parent);
#endif
}

/* cppcheck-suppress */
#define CREATE_PROC_ENTRY_DECLARE(NAME, PARENT) \
struct proc_dir_entry *dfx_create_ ## NAME ## _proc_entry(const char *name, \
	mode_t mode, const struct file_operations *proc_fops, void *data) \
{ \
	return dfx_create_proc_entry(name, mode, PARENT, proc_fops, data); \
} \
EXPORT_SYMBOL(dfx_create_ ## NAME ## _proc_entry); \
\
void dfx_remove_ ## NAME ## _proc_entry(const char *name) \
{ \
	dfx_remove_proc_entry(name, PARENT); \
\
	return; \
} \
EXPORT_SYMBOL(dfx_remove_ ## NAME ## _proc_entry);

CREATE_PROC_ENTRY_DECLARE(stats, proc_dfx_stats_entry)
CREATE_PROC_ENTRY_DECLARE(memory, proc_dfx_memory_entry)
CREATE_PROC_ENTRY_DECLARE(log, proc_dfx_log_entry)
CREATE_PROC_ENTRY_DECLARE(pstore, proc_dfx_pstore)
