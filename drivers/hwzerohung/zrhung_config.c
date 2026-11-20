/*
 * zrhung_config.c
 *
 * kernel configuration implementaion of zerohung
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "chipset_common/hwzrhung/zrhung.h"

#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/security.h>
#include <asm/current.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/syscalls.h>
#include <linux/mman.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <asm/cacheflush.h>
#include <linux/moduleloader.h>
#include <zrhung_common.h>

#define SCONTEXT "u:r:logserver:s0"
#define HCFG_VAL_SIZE_MAX ((ZRHUNG_CFG_VAL_LEN_MAX + 1) * ZRHUNG_CFG_ENTRY_NUM)
#define MAX_LOG_SIZE (4 * 1024) // 4KB
static const uint32_t MAX_STACK_DEPTH = 64;
static const uint32_t HCFG_FEATURE_VERSION = 1;
#define NOT_SUPPORT (-2)
#define NO_CONFIG (-1)
#define NOT_READY 1

struct hcfg_entry {
	uint32_t offset;
	uint32_t valid : 1;
};

struct hcfg_table_version {
	uint64_t len;
	struct hcfg_entry entry[ZRHUNG_CFG_ENTRY_NUM];
	char data[0];
};

struct hcfg_table {
	uint64_t len;
	struct hcfg_entry *entry;
	char *data;
};

struct hcfg_val {
	uint64_t wp;
	char data[ZRHUNG_CFG_VAL_LEN_MAX];
};

struct hcfg_ctx {
	struct hcfg_table table;
	unsigned long mem_size;
	int flag;
	void *user_table;
	unsigned long cfg_feature;
	unsigned long entry_num;
};

struct hstack_val {
	uint32_t magic;
	pid_t tid;
	char hstack_log_buff[MAX_LOG_SIZE];
};

static DEFINE_SPINLOCK(lock);
static struct hcfg_ctx ctx;
static pid_t hiview_pid;

int zrhung_is_id_valid(short wp_id)
{
	if ((wp_id <= ZRHUNG_WP_NONE) ||
	    ((wp_id >= ZRHUNG_WP_MAX) && (wp_id <= APPEYE_NONE)) ||
	    ((wp_id >= APPEYE_MAX) && (wp_id <= ZRHUNG_EVENT_NONE)) ||
	    (wp_id == XCOLLIE_FWK_SERVICE) || (wp_id >= ZRHUNG_XCOLLIE_MAX))
		return -EINVAL;
	return 0;
}

int hcfgk_set_cfg(struct file *file, const void __user *arg)
{
	int ret;
	uint64_t len = 0;
	uint64_t mem_size;
	void *user_table = NULL;
	void *tmp = NULL;
	uint64_t table_len = 0;
	uint64_t entry_num = 0;
	struct hcfg_table_version *t = NULL;

	if (!arg)
		return -EINVAL;

	hiview_pid = current->tgid;

	ret = copy_from_user(&len, arg, sizeof(len));
	if (ret) {
		pr_err("copy hung config table from user failed\n");
		return ret;
	}
	if (len > HCFG_VAL_SIZE_MAX || len <= 0)
		return -EINVAL;

	spin_lock(&lock);

	if (ctx.cfg_feature != HCFG_FEATURE_VERSION) {
		spin_unlock(&lock);
		pr_err("cfg_feature is invalid\n");
		return -EINVAL;
	}
	table_len = sizeof(struct hcfg_table_version);
	entry_num = ZRHUNG_CFG_ENTRY_NUM;

	spin_unlock(&lock);

	mem_size = PAGE_ALIGN(table_len + len);

	user_table = vmalloc(mem_size);
	if (!user_table)
		return -ENOMEM;

	memset(user_table, 0, mem_size);

	ret = copy_from_user(user_table, arg, table_len + len);
	if (ret) {
		pr_err("copy hung config table from user failed\n");
		vfree(user_table);
		return ret;
	}

	spin_lock(&lock);

	tmp = ctx.user_table;
	ctx.user_table = user_table;
	user_table = tmp;
	ctx.mem_size = mem_size;
	ctx.entry_num = entry_num;

	/* init table entry */
	if (ctx.cfg_feature != HCFG_FEATURE_VERSION) {
		spin_unlock(&lock);
		pr_err("cfg_feature is invalid\n");
		vfree(user_table);
		return -EINVAL;
	}

	t = ctx.user_table;
	ctx.table.entry = t->entry;
	ctx.table.data = t->data;

	/* make sure last byte in data is 0 terminated */
	ctx.table.len = len;
	ctx.table.data[len - 1] = '\0';

	spin_unlock(&lock);

	if (user_table != NULL)
		vfree(user_table);

	return ret;
}

int zrhung_get_config(enum zrhung_wp_id wp, char *data, uint32_t maxlen)
{
	int ret = NOT_READY;
	long long len;
	short entry_id;

	entry_id = ZRHUNG_WP_TO_ENTRY((unsigned int)wp);

	spin_lock(&lock);

	if (!ctx.table.entry || !ctx.table.data ||
	    ((zrhung_is_id_valid(wp) == 0) && (ctx.flag == 0)))
		goto out;

	if (!data || (entry_id >= ctx.entry_num) || (maxlen == 0)) {
		ret = -EINVAL;
		goto out;
	}

	if (!ctx.table.entry[entry_id].valid) {
		ret = NO_CONFIG;
		goto out;
	}

	if (ctx.table.entry[entry_id].offset >= ctx.table.len) {
		ret = -EINVAL;
		goto out;
	}

	len = ctx.table.len - ctx.table.entry[entry_id].offset;
	if (len <= 0) {
		ret = -EINVAL;
		goto out;
	}

	strncpy(data, ctx.table.data + ctx.table.entry[entry_id].offset,
		len > (maxlen - 1) ? (maxlen - 1) : len);
	data[maxlen - 1] = '\0';
	ret = 0;

out:
	spin_unlock(&lock);
	return ret;
}
EXPORT_SYMBOL(zrhung_get_config);

int hcfgk_ioctl_get_cfg(struct file *file, void __user *arg)
{
	struct hcfg_val val;
	int ret;

	if (!arg)
		return -EINVAL;

	memset(&val, 0, sizeof(val));
	if (copy_from_user(&val.wp, arg, sizeof(val.wp))) {
		pr_err("Get WP id from user failed\n");
		return NOT_SUPPORT;
	}

	ret = zrhung_get_config(val.wp, val.data, sizeof(val.data));
	if (!ret && copy_to_user(arg, &val, sizeof(val))) {
		pr_err("Failed to copy hung config val to user\n");
		ret = -EFAULT;
	}

	return ret;
}

int hcfgk_set_cfg_flag(struct file *file, const void __user *arg)
{
	int flag;

	if (!arg)
		return -EINVAL;

	if (copy_from_user(&flag, arg, sizeof(flag))) {
		pr_err("Copy Hung config flag from user failed\n");
		return -EFAULT;
	}

	pr_devel("set hcfg flag: %d\n", ctx.flag);

	spin_lock(&lock);

	if (flag > 0)
		ctx.flag = flag;

	spin_unlock(&lock);

	return 0;
}

int hcfgk_get_cfg_flag(struct file *file, void __user *arg)
{
	int flag;

	if (!arg)
		return -EINVAL;

	spin_lock(&lock);
	flag = ctx.flag;
	spin_unlock(&lock);
	pr_devel("get hcfg flag: %d\n", flag);

	if (copy_to_user(arg, &flag, sizeof(flag))) {
		pr_err("Failed to copy hung config flag to user\n");
		return -EFAULT;
	}

	return 0;
}

int hcfgk_set_feature(struct file *file, const void __user *arg)
{
	uint32_t feature;

	if (copy_from_user(&feature, arg, sizeof(feature))) {
		pr_err("Copy Hung config flag from user failed\n");
		return -EFAULT;
	}

	spin_lock(&lock);

	ctx.cfg_feature = feature;

	spin_unlock(&lock);

	return 0;
}

static int xcollie_save_stack(struct task_struct *task, char *out, uint32_t len)
{
	struct stack_trace trace;
	char *buf = NULL;
	uintptr_t *entries = NULL;
	size_t used = 0;
	int tmp = 0;
	int ret = -EFAULT;
	int i;

	if (len <= 0 || len > MAX_LOG_SIZE)
		return ret;

	entries = kmalloc_array(MAX_STACK_DEPTH, sizeof(*entries), GFP_KERNEL);
	if (!entries)
		return ret;

	trace.nr_entries = 0;
	trace.max_entries = MAX_STACK_DEPTH;
	trace.entries = entries;
	trace.skip = 0;

	buf = vzalloc(MAX_LOG_SIZE);
	if (!buf)
		goto err_entry;

	ret = mutex_lock_killable(&task->signal->cred_guard_mutex);

	if (ret)
		goto err_buf;
	save_stack_trace_tsk(task, &trace);
	for (i = 0; i < trace.nr_entries; i++) {
		tmp = snprintf(buf + used, MAX_LOG_SIZE -
			used, "xcollie: #%02d: %pB\n", i, (void *)entries[i]);
		if (tmp < 0) {
			ret = -EFAULT;
			goto err_mutex;
		}
		used += min((size_t)tmp, MAX_LOG_SIZE - used - 1);
		if (used >= (MAX_LOG_SIZE - 1))
			break;
	}
	memcpy(out, buf, MAX_LOG_SIZE);
	ret = 0;

err_mutex:
	mutex_unlock(&task->signal->cred_guard_mutex);
err_buf:
	vfree(buf);
err_entry:
	kfree(entries);
	return ret;
}

static int xcollie_obtain_stack(struct task_struct *task, int target_tgid,
			 struct hstack_val *val)
{
	int ret;

	if ((!current->tgid || (current->tgid != target_tgid)) &&
	    (!hiview_pid || (current->tgid != hiview_pid))) {
		pr_err("can not print stack of other process\n");
		return -EPERM;
	}

	ret = xcollie_save_stack(task, val->hstack_log_buff,
		sizeof(val->hstack_log_buff));
	if (ret) {
		pr_err("failed to get stack\n");
		return -EFAULT;
	}

	return 0;
}

int xcollie_get_stack(struct file *file, void __user *arg)
{
	struct hstack_val *val = NULL;
	struct task_struct *task = NULL;
	int ret = -EFAULT;
	int target_tgid = 0;

	if (!arg)
		return -EINVAL;

	val = vzalloc(sizeof(*val));
	if (!val)
		return -ENOMEM;

	if (copy_from_user(val, arg, sizeof(*val))) {
		pr_err("copy tid from user failed\n");
		goto err_val;
	}
	if ((val->magic != MAGIC_NUM) || (val->tid <= 0)) {
		pr_err("magic num or pid error\n");
		goto err_val;
	}

	rcu_read_lock();
	task = find_task_by_vpid(val->tid);
	if (task) {
		get_task_struct(task);
		target_tgid = task->tgid;
	}
	rcu_read_unlock();
	if (!task) {
		pr_err("task is null\n");
		goto err_val;
	}

	ret = xcollie_obtain_stack(task, target_tgid, val);
	if (ret)
		goto err_task;

	if (copy_to_user(arg, val, sizeof(*val))) {
		pr_err("Failed to copy kernel stack to user\n");
		goto err_task;
	}
	ret = 0;

err_task:
	put_task_struct(task);
err_val:
	vfree(val);
	return ret;
}

