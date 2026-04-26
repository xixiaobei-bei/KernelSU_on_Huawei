/*
 * hkip_atkinfo_debugfs.c
 *
 * Huawei debugfs interface for HKIP upload atkinfo.
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
 */

#include "hkip_atkinfo.h"
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/debugfs.h>
#include <linux/uaccess.h>

static struct dentry *atkinfo_root;
extern unsigned int disable_upload;

#define MAX_BUF        16
#define MAX_SHOW_ITEMS 3
#define DEBUGFS_R_FOPS(name) \
	static const struct file_operations debugfs_##name##_fops = { \
		.owner = THIS_MODULE, \
		.open = attr_##name##_open, \
		.read = seq_read, \
		.llseek = seq_lseek, \
		.release = single_release, \
	}

#define DEBUGFS_RW_FOPS(name) \
	static const struct file_operations debugfs_##name##_fops = { \
		.owner = THIS_MODULE, \
		.open = attr_##name##_open, \
		.read = seq_read, \
		.llseek = seq_lseek, \
		.release = single_release, \
		.write = attr_##name##_write, \
	}

static inline const char *type_id_to_string(uint16_t type_id)
{
	switch (type_id) {
	case HHEE_EV_MMU_DISABLE:
		return "mmu";
	case HHEE_EV_MMU_REPLACE:
		return "ttbr1";
	case HHEE_EV_WXN_DISABLE:
		return "wxn";
	case HHEE_EV_OS_TEXT_OVERWRITE:
		return "kernel_text";
	default:
		return "wrong_type";
	}
}

static int attr_header_show(struct seq_file *m, void *private)
{
	struct hhee_event_header *hdr = m->private;

	if (!hdr)
		return -EINVAL;
	seq_printf(m, "MAGIC:0x%llx, writeoffset:%llu, size:%llu\n"
		   "capacity:%llu, buffer_offset:%llu, footer_offset:%llu\n",
		   hdr->magic, hdr->write_offset, hdr->buffer_size,
		   hdr->buffer_capacity, hdr->buffer_offset,
		   hdr->footer_offset);
	return 0;
}

static int attr_footer_show(struct seq_file *m, void *private)
{
	struct hhee_event_footer *footer = m->private;

	if (!footer)
		return -EINVAL;
	seq_printf(m, "readoffset:%llu\n", footer->read_offset);

	return 0;
}

static int attr_events_show(struct seq_file *m, void *private)
{
	struct hhee_event_header *hdr = m->private;
	struct hhee_event *et = NULL;
	uint64_t start, end;
	uint64_t count = 0;
	uint64_t i;
	uint32_t j;

	if (!hdr)
		return -EINVAL;

	et = hdr->events;

	start = (hdr->write_offset < hdr->buffer_capacity) ?
		0 : (hdr->write_offset - hdr->buffer_capacity);
	end = hdr->write_offset;

	for (i = start; i < end; i++, count++) {
		if (count >= MAX_SHOW_ITEMS) {
			pr_warn("the number of upload events is more than 3\n");
			break;
		}

		if (!hdr->buffer_capacity)
			return -EINVAL;

		j = i % hdr->buffer_capacity;

		seq_printf(m, "event[%d]:type=%s, pid=%u, Msg=\"%s\"\n",
			   j, type_id_to_string(et[j].type),
			   et[j].ctx_id, et[j].description);

		seq_printf(m, "\tlink=%llx virt_addr=0x%llx\n"
			   "rsvd1=%llx sysdrome=%u\n"
			   "\ttid_system=0x%llx tid_shared=0x%llx\n"
			   "tid_user=0x%llx\n",
			   et[j].link, et[j].virt_addr, et[j].rsvd1,
			   et[j].syndrome, et[j].tid_system,
			   et[j].tid_shared, et[j].tid_user);
	}

	return 0;
}

static int attr_cycle_times_show(struct seq_file *m, void *private)
{
	struct hkip_atkinfo *atkinfo = m->private;

	if (!atkinfo)
		return -EINVAL;

	seq_printf(m, "%u ms\n", atkinfo->cycle_time);

	return 0;
}

static ssize_t attr_cycle_times_write(struct file *file,
				      const char __user *ubuf,
				      size_t len, loff_t *offp)
{
	struct seq_file *m = file->private_data;
	struct hkip_atkinfo *atkinfo = NULL;
	int ret;

	if (!m)
		return -EINVAL;

	atkinfo = m->private;
	if (!atkinfo)
		return -EINVAL;

	ret = kstrtou32_from_user(ubuf, len, 0, &atkinfo->cycle_time);
	if (ret)
		return ret;

	mod_timer(&atkinfo->timer, jiffies + msecs_to_jiffies
		((const unsigned int)atkinfo->cycle_time));

	return len;
}

static int attr_logupload_switch_show(struct seq_file *m, void *private)
{
	if (disable_upload == 0)
		seq_puts(m, "on\n");
	else if (disable_upload == 1)
		seq_puts(m, "off\n");
	else
		seq_puts(m, "error\n");

	return 0;
}

static ssize_t attr_logupload_switch_write(struct file *file,
					   const char __user *ubuf,
					   size_t len, loff_t *offp)
{
	char buf[MAX_BUF] = {0};
	size_t cnt = len;

	if (cnt > sizeof(buf) - 1)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, cnt))
		return -EINVAL;

	while ((cnt > 0) && ((buf[cnt - 1] == ' ') || (buf[cnt - 1] == '\n')))
		cnt--;

	buf[cnt] = '\0';

	if (!strncmp(buf, "on", cnt))
		disable_upload = 0;
	else if (!strncmp(buf, "off", cnt))
		disable_upload = 1;
	else
		return -EINVAL;

	return len;
}

static int attr_attack_show(struct seq_file *m, void *private)
{
	seq_puts(m, "ttbr1\n");

	return 0;
}

static ssize_t attr_attack_write(struct file *file,
				 const char __user *ubuf,
				 size_t len, loff_t *offp)
{
	char buf[MAX_BUF] = {0};
	uint64_t value = ~0ULL;
	size_t cnt = len;

	if (cnt > sizeof(buf) - 1)
		return -EINVAL;

	if (copy_from_user(buf, ubuf, cnt))
		return -EINVAL;

	while ((cnt > 0) && ((buf[cnt - 1] == ' ') || (buf[cnt - 1] == '\n')))
		cnt--;

	buf[cnt] = '\0';

	pr_info("hkip_atkinfo : attack process %d:%s\n", current->pid,
		current->comm);

	if (!strncmp(buf, "ttbr1", cnt)) {
		write_sysreg(value, ttbr1_el1);
		msleep(50);
	} else {
		return -EINVAL;
	}

	return len;
}

static int attr_header_open(struct inode *inode, struct file *file)
{
	return single_open(file, attr_header_show, inode->i_private);
}

static int attr_footer_open(struct inode *inode, struct file *file)
{
	return single_open(file, attr_footer_show, inode->i_private);
}

static int attr_events_open(struct inode *inode, struct file *file)
{
	return single_open(file, attr_events_show, inode->i_private);
}

static int attr_attack_open(struct inode *inode, struct file *file)
{
	return single_open(file, attr_attack_show, inode->i_private);
}

static int attr_logupload_switch_open(struct inode *inode, struct file *file)
{
	return single_open(file, attr_logupload_switch_show, inode->i_private);
}

static int attr_cycle_times_open(struct inode *inode, struct file *file)
{
	return single_open(file, attr_cycle_times_show, inode->i_private);
}

DEBUGFS_R_FOPS(header);
DEBUGFS_R_FOPS(footer);
DEBUGFS_R_FOPS(events);
DEBUGFS_RW_FOPS(attack);
DEBUGFS_RW_FOPS(logupload_switch);
DEBUGFS_RW_FOPS(cycle_times);

int __init atkinfo_create_debugfs(struct hkip_atkinfo *atkinfo)
{
	struct dentry *debugfs_file = NULL;

	atkinfo_root = debugfs_create_dir("hkip_atkinfo", NULL);
	if (!atkinfo_root)
		return -ENXIO;

	debugfs_file = debugfs_create_file("header",
		0444, atkinfo_root, atkinfo->header, &debugfs_header_fops);
	if (!debugfs_file)
		goto end;

	debugfs_file = debugfs_create_file("footer",
		0444, atkinfo_root, atkinfo->footer, &debugfs_footer_fops);
	if (!debugfs_file)
		goto end;

	debugfs_file = debugfs_create_file("events",
		0444, atkinfo_root, atkinfo->header, &debugfs_events_fops);
	if (!debugfs_file)
		goto end;

	debugfs_file = debugfs_create_file("attack",
		0444, atkinfo_root, atkinfo, &debugfs_attack_fops);
	if (!debugfs_file)
		goto end;

	debugfs_file = debugfs_create_file("logupload_switch",
		0444, atkinfo_root, NULL, &debugfs_logupload_switch_fops);
	if (!debugfs_file)
		goto end;

	debugfs_file = debugfs_create_file("cycle_times",
		0444, atkinfo_root, atkinfo, &debugfs_cycle_times_fops);
	if (!debugfs_file)
		goto end;

	return 0;

end:
	debugfs_remove(atkinfo_root);
	atkinfo_root = NULL;
	return -ENODEV;
}
