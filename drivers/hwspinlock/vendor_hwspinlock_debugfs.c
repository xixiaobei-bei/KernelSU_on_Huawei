/*
 * hardware spin lock, debug module
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/hwspinlock.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include "hwspinlock_internal.h"
#include <pr_log.h>
#include <linux/version.h>

#define PR_LOG_TAG HWSPINLOCK_DEBUGFS_TAG
/*
 * debugfs file node:
 * - dev/debugfs/hwspinlock/debug
 *
 * test cmd:
 * - "echo request 'ID'", request a unused hwspinlock;
 * - "echo free_lock 'ID'", free the requested hwspinlock;
 * - "echo trylock 'ID' 'timeout' 'unlock'", lock the request hwspinlock,
 * timeout is msec,unlock = 0 represents that don't unlock after
 * lock,otherwise unlock;
 * and nomally maybe unlock be setted 1;
 * but trylock timeout maybe cause the atomic question because of sys_write
 * is atomic
 * - "echo unlock 'ID'", unlock the hwspinlock;
 * - "cat debug", show current the resources of hwspinlock,but now have
 * requested all locks.
 */
struct hwspinlock_native {
	int id_in_group;
	void __iomem *address;
};
#ifdef CONFIG_HISI_DEBUG_FS
static struct dentry *hwspinlock_debug_dir;
static struct dentry *hwspinlock_fn;
#endif
static struct hwspinlock *hwlock;
static int locked;

#define RESOURCE_LOCK_STAT_OFFSET 0x8
#define DEBUG_FS_LEN 128
#define LOCK_NUM_MAX 64
#define DEBUG_FS_DEC 10
#define TRY_LOCK_CMD_NUM 3

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)) && defined(CONFIG_PRODUCT_ARMPC))
extern struct hwspinlock *hwspin_lock_lookup(unsigned int id);
#endif

static struct hwspinlock *debugfs_hwspinlock_request_specific(
	int id, int *b_id, int *n_locks)
{
	struct hwspinlock_native *hwlockinfo = NULL;
	struct hwspinlock *_hwlock = NULL;

	_hwlock = hwspin_lock_request_specific(id);
	if (!_hwlock) {
		pr_err("hwspinlock %d is already in use\n", id);
		return NULL;
	}

	*b_id = _hwlock->bank->base_id;
	*n_locks = _hwlock->bank->num_locks;
	hwlockinfo = (struct hwspinlock_native *)(_hwlock->priv);
	pr_info("[Debug] Cat Success! RESOURCE_LOCK ID_IN_Group=%d  ADDR=%pK\n",
		hwlockinfo->id_in_group, hwlockinfo->address);
	return _hwlock;
}

static int debugfs_hwspinlock_lock_free(struct hwspinlock *_hwlock)
{
	int ret;

	ret = hwspin_lock_free(_hwlock);
	if (ret)
		pr_err("[debug] hwspinlock free fail, ret = [%d]\n", ret);
	else
		pr_info("[debug] hwspinlock free %d success!\n",
			hwlock_to_id(_hwlock));

	return ret;
}

/*
 * hwspin_trylock()
 * Returns 0 if we successfully locked the hwspinlock, -EBUSY if
 * the hwspinlock was already taken, and -EINVAL if @hwlock is invalid.
 * hwspin_lock_timeout()
 * Returns 0 when the @hwlock was successfully taken, and an appropriate
 * error code otherwise (most notably an -ETIMEDOUT if the @hwlock is still
 * busy after @timeout msecs). The function will never sleep.
 */
static int debugfs_hwspinlock_trylock_timeout(int id, int timeout, int unlock)
{
	struct hwspinlock_native *hwlockinfo = NULL;
	struct hwspinlock_native *hwunlockinfo = NULL;
	int ret = 0;
	int val0, val1;

	if (!hwlock)
		return -1;
	if (!timeout) {
		ret = hwspin_trylock(hwlock);
		if (ret) {
			pr_err("%s: hwspinlock trylock failed!\n", __func__);
			return ret;
		}
	} else if (timeout > 0) {
		ret = hwspin_lock_timeout(hwlock, timeout);
		if (ret) {
			pr_err("%s: hwspinlock timeout!\n", __func__);
			return ret;
		}
	} else {
		pr_err("%s: timeout is invalid!\n", __func__);
	}

	if (!hwlock->priv)
		return -1;

	hwlockinfo = (struct hwspinlock_native *)(hwlock->priv);
	val0 = readl(hwlockinfo->address + RESOURCE_LOCK_STAT_OFFSET);
	locked = 1;
	if (unlock) {
		hwspin_unlock(hwlock);
		hwunlockinfo = (struct hwspinlock_native *)(hwlock->priv);
		val1 = readl(hwunlockinfo->address + RESOURCE_LOCK_STAT_OFFSET);
		locked = 0;
	} else {
		val1 = readl(hwlockinfo->address + RESOURCE_LOCK_STAT_OFFSET);
	}
	pr_info("[debug]the ResourceLock %d locked val= 0x%x,unlock val=0x%x\n",
		id, val0, val1);

	return ret;
}

static int debugfs_show(struct seq_file *s, void *data)
{
	int base_id, num_locks;
	int i;
	struct hwspinlock *_hwlock = NULL;

	base_id = 0;
	num_locks = LOCK_NUM_MAX;

	pr_info("[DEBUGFS] VENDOR RESOURCE_LOCK INFO ++++++++++++\n");
	/*
	 * num_lock default value is 64, if lock type supported ao_lock,
	 * it will change to 96, and will never changes
	 */
	for (i = 0; i < num_locks; i++) {
		_hwlock = debugfs_hwspinlock_request_specific(
			i, &base_id, &num_locks);
		if (_hwlock)
			debugfs_hwspinlock_lock_free(_hwlock);
	}

	pr_info("[DEBUGFS] base_id =%d num_locks=%d\n", base_id, num_locks);
	pr_info("[DEBUGFS] VENDOR RESOURCE_LOCK INFO ------------\n");

	return 0;
}

static int debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, debugfs_show, inode->i_private);
}

static ssize_t execution_request_cmd(char *cmd, char *_cmd)
{
	int id;

	cmd = cmd + strlen("request ");
	_cmd = cmd;
	while ((*_cmd != ' ') && (*_cmd != '\0'))
		_cmd++;
	*_cmd = '\0';

	if (kstrtos32(cmd, DEBUG_FS_DEC, &id)) {
		pr_err("Hwspinlock Debugfs cmd error\n");
		return -EINVAL;
	}

	hwlock = hwspin_lock_request_specific(id);
	if (!hwlock)
		pr_err("hwspinlock %d is already in use\n", id);
	else
		pr_info("[debug] Request hwspinlock %d success!", id);
	return 0;
}

static ssize_t execution_free_lock_cmd(char *cmd, char *_cmd)
{
	int id;
	struct hwspinlock *__hwlock = NULL;

	cmd = cmd + strlen("free_lock ");
	_cmd = cmd;
	while ((*_cmd != ' ') && (*_cmd != '\0'))
		_cmd++;
	*_cmd = '\0';
	if (kstrtos32(cmd, DEBUG_FS_DEC, &id)) {
		pr_err("Hwspinlock Debugfs cmd error\n");
		return -EINVAL;
	}

	if (!hwlock) {
		__hwlock = hwspin_lock_lookup(id);
		if (__hwlock) {
			debugfs_hwspinlock_lock_free(__hwlock);
		} else {
			pr_err("haven't requested hwspinlock %d\n", id);
			return 0;
		}
	} else if (hwlock_to_id(hwlock) == id) {
		debugfs_hwspinlock_lock_free(hwlock);
		hwlock = NULL;
	} else {
		pr_err("[debug] please freelock the correct lock!\n");
	}
	return 0;
}

static ssize_t execution_trylock_ops(int parameters[], int buf_len)
{
	int ret;

	if (!hwlock) {
		pr_err("haven't requested hwspinlock %d\n", parameters[0]);
		return -EINVAL;
	} else if (hwlock_to_id(hwlock) == parameters[0] &&
			parameters[1] >= 0) {
		/*
		 * parameters[0] lock_id, parameters[1] timeout,
		 * parameters[2] unlock
		 */
		ret = debugfs_hwspinlock_trylock_timeout(
			parameters[0],
			parameters[1],
			parameters[2]);
		if (!ret) {
			if (!parameters[1])
				pr_info("[debug] hwspin_trylock %d success!\n",
					parameters[0]);
			else
				pr_info(
				"[debug] hwspin_trylock_timeout %d success!\n",
					parameters[0]);
		}
	} else if (parameters[1] < 0) {
		pr_err("[debug] cmd err! timeout must > 0\n");
	} else {
		pr_err("[debug] please trylock the correct lock!\n");
	}

	return 0;
}

static ssize_t execution_trylock_cmd(char *cmd, char *_cmd)
{
	int para_flag = 0;
	int parameters[TRY_LOCK_CMD_NUM] = {0};
	char buf_temp[DEBUG_FS_LEN] = {0};
	char *ptr = buf_temp;

	cmd = cmd + strlen("trylock ");
	_cmd = cmd;
	while ((*_cmd != ' ') && (*_cmd != '\0')) {
		*ptr++ = *_cmd++;
		if (*_cmd == ' ') {
			*ptr = '\0';
			if (kstrtos32(buf_temp, DEBUG_FS_DEC,
				&parameters[para_flag])) {
				pr_err("[debug] cmd para error\n");
				return -EINVAL;
			}
			para_flag++;
			if (para_flag < TRY_LOCK_CMD_NUM) {
				ptr = buf_temp;
				cmd = _cmd + 1;
				_cmd = cmd;
			}
		}
	}

	if (para_flag == (TRY_LOCK_CMD_NUM - 1)) {
		*ptr = '\0';
		if (kstrtos32(buf_temp, DEBUG_FS_DEC,
			&parameters[para_flag])) {
			pr_err("[debug] cmd last para error\n");
			return -EINVAL;
		}
		para_flag++;
	}

	if (para_flag != TRY_LOCK_CMD_NUM)
		return -EINVAL;

	return execution_trylock_ops(parameters, TRY_LOCK_CMD_NUM);
}

static ssize_t execution_unlock_cmd(char *cmd, char *_cmd)
{
	int id;

	cmd = cmd + strlen("unlock ");
	_cmd = cmd;
	while ((*_cmd != ' ') && (*_cmd != '\0'))
		_cmd++;
	*_cmd = '\0';
	if (kstrtos32(cmd, DEBUG_FS_DEC, &id)) {
		pr_err("Hwspinlock Debugfs cmd error\n");
		return -EINVAL;
	}

	if (!hwlock) {
		pr_err("don't echo request hwspinlock %d\n", id);
		return 0;
	} else if (hwlock_to_id(hwlock) == id && locked == 1) {
		hwspin_unlock(hwlock);
		pr_info("[debug] hwspin_unlock id=%d success!\n", id);
	} else {
		pr_err("[debug] please free the correct request lock!\n");
	}

	return 0;
}

static ssize_t debugfs_write(struct file *filp, const char __user *ubuf,
	size_t cnt, loff_t *ppos)
{
	char buf[DEBUG_FS_LEN] = {0};
	char *cmd = NULL;
	char *_cmd = NULL;
	ssize_t result;

	if ((!ubuf) || (!cnt) || (cnt > ARRAY_SIZE(buf))) {
		pr_err("Input parameter is err!\n");
		return -EINVAL;
	}

	if (copy_from_user(buf, ubuf, cnt - 1)) {
		pr_err("[Hwspinlock Debugfs] can not copy from user\n");
		return -EINVAL;
	}

	buf[cnt - 1] = '\0';
	cmd = buf;
	_cmd = buf;
	pr_debug("[Hwspinlock Debugfs] [cmd: %s[cnt: %d]]\n", cmd, (int)cnt);

	if (!strncmp("request ", _cmd, strlen("request "))) {
		result = execution_request_cmd(cmd, _cmd);
	} else if (!strncmp("free_lock ", _cmd, strlen("free_lock "))) {
		result = execution_free_lock_cmd(cmd, _cmd);
	} else if (!strncmp("trylock ", _cmd, strlen("trylock "))) {
		result = execution_trylock_cmd(cmd, _cmd);
	} else if (!strncmp("unlock ", _cmd, strlen("unlock "))) {
		result = execution_unlock_cmd(cmd, _cmd);
	} else {
		pr_err("Hwspinlock Debugfs cmd error\n");
		result = -EINVAL;
	}

	return ((!result) ? cnt : result);
}

static const struct file_operations hwspinlock_debugfs_ops = {
	.open = debugfs_open,
	.read = seq_read,
	.write = debugfs_write,
	.llseek = seq_lseek,
	.release = single_release,
};

/*
 * Mode_t kernel file permission value
 * S_IRUGO = S_IRUSR | S_IRGRP | S_IROTH
 * S_IRUSR: User read 00400
 * S_IRGRP: User group read 00040
 * S_IROTH: Other reading 00004
 */
static int __init hwspinlock_debugfs_init(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	hwspinlock_debug_dir = debugfs_create_dir("hwspinlock", NULL);
	if (hwspinlock_debug_dir)
		hwspinlock_fn = debugfs_create_file("debug", 0444,
			hwspinlock_debug_dir, NULL, &hwspinlock_debugfs_ops);
#endif
	return 0;
}
module_init(hwspinlock_debugfs_init);

static void __exit hwspinlock_debugfs_exit(void)
{
#ifdef CONFIG_HISI_DEBUG_FS
	debugfs_remove(hwspinlock_fn);
	debugfs_remove(hwspinlock_debug_dir);
#endif
}
module_exit(hwspinlock_debugfs_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hwspinlock debugfs driver");
