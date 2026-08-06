/*
 * hwioinfo.c
 *
 * iostats proc interface
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#include "internal.h"
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/vmstat.h>
#include <linux/bootdevice.h>
#include <linux/version.h>

void __attribute__((weak)) arch_report_meminfo(struct seq_file *m)
{
}

#define UI_IO_BLOCK_DOT 26
#define UNIT_50MS 50

u64 ui_thread_io_block_counts[UI_IO_BLOCK_DOT] = {0};

void sched_account_ui_thread_io_block_counts(int msecs)
{
	u64 ui_io_block_idx;

	ui_io_block_idx = msecs / UNIT_50MS;
	if (ui_io_block_idx >= UI_IO_BLOCK_DOT)
		ui_io_block_idx = UI_IO_BLOCK_DOT - 1;

	ui_thread_io_block_counts[ui_io_block_idx]++;
}

static void show_val(struct seq_file *m, const char *s, unsigned long num)
{
	char v[32];     // 32 keep consistent with block io info
	static const char blanks[7] = {' ', ' ', ' ', ' ', ' ', ' ', ' '};  // 7 keep consistent with block io info
	int len;

#if (KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE)
	len = num_to_str(v, sizeof(v), num, 0);
#else
	len = num_to_str(v, sizeof(v), num);
#endif

	seq_write(m, s, 16);  // 16 keep consistent with block io info

	if (len > 0) {
		if (len < 8)
			seq_write(m, blanks, 8 - len);     // 8 keep consistent with block io info

		seq_write(m, v, len);
	}
	seq_write(m, "\n", 1);
}

static int hwioinfo_proc_show(struct seq_file *m, void *v)
{
	int i;
	static const int MAX_LEN_FOR_IO_INFO = 128;
	char buf[MAX_LEN_FOR_IO_INFO];
	struct vm_event_state vm_stat_val;

#ifdef CONFIG_HUAWEI_QCOM_MMC
	show_val(m, "StorageManfid:	    ", get_bootdevice_manfid());
#endif

	for (i = 0; i < UI_IO_BLOCK_DOT; i++) {
		sprintf(buf, "UiIoDelaySeg%u:  ", (i + 1));
		show_val(m, buf, ui_thread_io_block_counts[i]);
	}

	/* read and clear */
	memset(ui_thread_io_block_counts, 0, sizeof(ui_thread_io_block_counts));

	all_vm_events((long *)&vm_stat_val);
	show_val(m, "vmstat_pswpin:  ", vm_stat_val.event[PSWPIN]);
	show_val(m, "vmstat_pswpout: ", vm_stat_val.event[PSWPOUT]);

	return 0;
}

static int hwioinfo_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, hwioinfo_proc_show, NULL);
}

static const struct file_operations hwioinfo_proc_fops = {
	.open       = hwioinfo_proc_open,
	.read       = seq_read,
	.llseek     = seq_lseek,
	.release    = single_release,
};

static int __init proc_hwioinfo_init(void)
{
	proc_create("hwioinfo", 0440, NULL, &hwioinfo_proc_fops);
	return 0;
}
fs_initcall(proc_hwioinfo_init);
