/*
 * memcheck_ioctl.c
 *
 * implement the ioctl for user space to get memory usage information,
 * and also provider control command
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#include "memcheck_common.h"
#include "memcheck_interface.h"
#include <linux/uaccess.h>
#include <chipset_common/hwmemcheck/memcheck.h>

static int get_count(u16 type)
{
	int i;
	int cnt;

	for (i = 0, cnt = 0; i < NUM_KERN_MAX; i++) {
		if (TEST_BIT(type, (unsigned int)(i + IDX_KERN_START)))
			cnt++;
	}
	return cnt;
}

static int process_get_memstat(void *arg)
{
	struct memstat_all memstat;
	unsigned short result;

	if (copy_from_user(&memstat, arg, sizeof(memstat))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (memstat.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	if (((memstat.type & MTYPE_USER) && (memstat.type & MTYPE_KERNEL)) ||
	    (!(memstat.type & MTYPE_ALL))) {
		memcheck_err("invalid memtype %d\n", memstat.type);
		return -EINVAL;
	}
	if ((memstat.type & MTYPE_USER) && (memstat.id <= 0)) {
		memcheck_err("invalid pid %d\n", memstat.id);
		return -EINVAL;
	}
	if ((memstat.type & MTYPE_KERNEL) && (get_count(memstat.type) > 1)) {
		memcheck_err("invalid memtype %d\n", memstat.type);
		return -EINVAL;
	}

	result = memcheck_get_memstat(&memstat);
	if ((result & MTYPE_ALL) == MTYPE_NONE) {
		memcheck_err("get memstat infor failed\n");
		return -EFAULT;
	}

	/* the types successfully read will return to user space */
	memstat.type = result;

	if (copy_to_user((void *)arg, &memstat, sizeof(memstat))) {
		memcheck_err("copy_to_user failed\n");
		return -EFAULT;
	}

	return 0;
}

static int process_do_command(const void *arg)
{
	struct track_cmd cmd;

	if (copy_from_user(&cmd, arg, sizeof(cmd))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (cmd.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL COMMAND id=%u,type=%u,timestamp=%llu,cmd=%d\n",
		      cmd.id, cmd.type, cmd.timestamp, cmd.cmd);
	if (((cmd.type & MTYPE_USER) && (cmd.type & MTYPE_KERNEL)) ||
	    (!(cmd.type & MTYPE_ALL))) {
		memcheck_err("invalid memtype %d\n", cmd.type);
		return -EINVAL;
	}
	if (cmd.type & (MTYPE_USER_VSS | MTYPE_USER_ION | MTYPE_KERN_ION |
			MTYPE_KERN_CMA | MTYPE_KERN_ZSMALLOC | MTYPE_KERN_SKB |
			MTYPE_KERN_VMALLOC)) {
		memcheck_info("memtype %d is not supported\n", cmd.type);
		return 0;
	}
	if ((cmd.type & MTYPE_USER) && (cmd.id <= 0)) {
		memcheck_err("invalid pid %d\n", cmd.id);
		return -EINVAL;
	}
	if ((cmd.type & MTYPE_KERNEL) && (get_count(cmd.type) > 1)) {
		memcheck_err("invalid memtype %d\n", cmd.type);
		return -EINVAL;
	}
	if ((cmd.cmd <= MEMCMD_NONE) || (cmd.cmd >= MEMCMD_MAX)) {
		memcheck_err("invalid cmd %d\n", cmd.cmd);
		return -EINVAL;
	}
	if ((cmd.type & MTYPE_USER) && (cmd.cmd == MEMCMD_ENABLE) &&
	    (!cmd.timestamp)) {
		memcheck_err("invalid timestamp for MEMCMD_ENABLE\n");
		return -EINVAL;
	}

	return memcheck_do_command(&cmd);
}

static int process_detail_read(void *arg)
{
	struct detail_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL DETAIL type=%u,size=%llu\n", info.type, info.size);
	if (get_count(info.type) > 1) {
		memcheck_err("too many memtype %d\n", info.type);
		return -EINVAL;
	}
	if ((!info.size) || (info.size > MEMCHECK_DETAILINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}
	if (info.type != MTYPE_KERN_SLUB && info.type != MTYPE_KERN_ION &&
	    info.type != MTYPE_USER_ION && info.type != MTYPE_KERN_VMALLOC) {
		memcheck_err("do not support type %d\n", info.type);
		return -EINVAL;
	}

	return memcheck_detail_read(arg, &info);
}

static int process_stack_save(const void *arg)
{
	struct stack_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL STACK SAVE type=%u,size=%llu\n", info.type,
		      info.size);
	if ((!info.size) || (info.size > MEMCHECK_STACKINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}

	return memcheck_stack_write(arg, &info);
}

static int process_stack_read(void *arg)
{
	struct stack_info info;

	if (copy_from_user(&info, (const void *)arg, sizeof(info))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}
	if (info.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	memcheck_info("IOCTL STACK READ type=%u,size=%llu\n", info.type,
		      info.size);
	if ((!info.size) || (info.size > MEMCHECK_STACKINFO_MAXSIZE)) {
		memcheck_err("wrong size=%zu\n", info.size);
		return -EINVAL;
	}
	if (!(info.type & (MTYPE_USER_PSS | MTYPE_KERN_SLUB | MTYPE_KERN_LSLUB |
			   MTYPE_KERN_VMALLOC | MTYPE_KERN_BUDDY))) {
		memcheck_err("invalid memtype %d\n", info.type);
		return -EINVAL;
	}
	return memcheck_stack_read(arg, &info);
}

static int process_lmk_oom_write(const void *arg)
{
	struct lmk_oom_write wr;

	if (copy_from_user(&wr, (const void *)arg, sizeof(wr))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (wr.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	if (wr.data.pid <= 0) {
		memcheck_err("invalid pid %d\n", wr.data.pid);
		return -EINVAL;
	}
	if ((wr.data.ktype <= KILLTYPE_NONE) ||
	    (wr.data.ktype >= KILLTYPE_MAX)) {
		memcheck_err("invalid kill type %d\n", wr.data.ktype);
		return -EINVAL;
	}
	if (!wr.data.timestamp) {
		memcheck_err("invalid timestamp\n");
		return -EINVAL;
	}
	wr.data.name[MEMCHECK_PATH_MAX] = 0;

	return memcheck_lmk_oom_write(&wr);
}

static int process_lmk_oom_read(void *arg)
{
	struct lmk_oom_read rd;

	if (copy_from_user(&rd, arg, sizeof(rd))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (rd.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	if ((!rd.num) || (rd.num > MEMCHECK_OOM_LMK_MAXNUM)) {
		memcheck_err("wrong num=%zu\n", rd.num);
		return -EINVAL;
	}

	return memcheck_lmk_oom_read(arg, &rd);
}

static int process_get_task_type(void *arg)
{
	struct task_type_read rd;

	if (copy_from_user(&rd, arg, sizeof(rd))) {
		memcheck_err("copy_from_user failed\n");
		return -EFAULT;
	}

	if (rd.magic != MEMCHECK_MAGIC) {
		memcheck_err("bad maginc number\n");
		return -EINVAL;
	}
	if ((!rd.num) || (rd.num > MEMCHECK_TASK_MAXNUM)) {
		memcheck_err("wrong num=%zu\n", rd.num);
		return -EINVAL;
	}

	return memcheck_get_task_type(arg, &rd);
}

long memcheck_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = -EINVAL;

	if ((cmd < LOGGER_MEMCHECK_MIN) || (cmd > LOGGER_MEMCHECK_MAX))
		return MEMCHECK_CMD_INVALID;
	if ((!arg) && (cmd != LOGGER_MEMCHECK_FREE_MEM))
		return -EINVAL;

	switch (cmd) {
	case LOGGER_MEMCHECK_GET_MEMSTAT:
		ret = process_get_memstat((void *)arg);
		break;

	case LOGGER_MEMCHECK_COMMAND:
		ret = process_do_command((void *)arg);
		break;

	case LOGGER_MEMCHECK_DETAIL_READ:
		ret = process_detail_read((void *)arg);
		break;

	case LOGGER_MEMCHECK_STACK_READ:
		ret = process_stack_read((void *)arg);
		break;

	case LOGGER_MEMCHECK_STACK_SAVE:
		ret = process_stack_save((void *)arg);
		break;

	case LOGGER_MEMCHECK_LMK_OOM_SAVE:
		ret = process_lmk_oom_write((void *)arg);
		break;

	case LOGGER_MEMCHECK_LMK_OOM_READ:
		ret = process_lmk_oom_read((void *)arg);
		break;

	case LOGGER_MEMCHECK_FREE_MEM:
		memcheck_err("free track memory for commercial release\n");
		ret = buddy_track_unmap();
		break;

	case LOGGER_MEMCHECK_GET_TASK_TYPE:
		ret = process_get_task_type((void *)arg);
		break;

	default:
		break;
	}

	return ret;
}
EXPORT_SYMBOL(memcheck_ioctl);
