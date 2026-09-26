/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: for lmkd to trigger in-kernel lowmem info dump
 * Author: Xia Qing <saberlily.xia@hisilicon.com> Sun Cai <suncai1@huawei.com>
 * Create: 2020-03-26
 */
#include <linux/atomic.h>
#include <linux/fs.h>
#include <linux/hisi/lowmem_killer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/oom.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

/* Worst case buffer size needed for holding an integer. */
#define PROC_NUMBUF 8

static int lmkd_oom_score_adj;
static atomic64_t lmkd_no_cma_cnt = ATOMIC64_INIT(0);

void lmkd_inc_no_cma_cnt(void)
{
	atomic64_inc(&lmkd_no_cma_cnt);
}

void lmkd_dec_no_cma_cnt(void)
{
	atomic64_dec(&lmkd_no_cma_cnt);
}

static int lmkd_dbg_trigger_proc_show(struct seq_file *m, void *v)
{
	struct zone *z = NULL;
	long media_zone_nr_free = 0;

	seq_printf(m, "lmkd_oom_score_adj:  %d\n", lmkd_oom_score_adj);
	seq_printf(m, "lmkd_no_cma_cnt:  %ld\n",
			atomic64_read(&lmkd_no_cma_cnt));

#ifdef CONFIG_ZONE_MEDIA
	for_each_populated_zone(z) {
		if (IS_MEIDA_ZONE_IDX(zone_idx(z)))
			media_zone_nr_free = zone_page_state(z, NR_FREE_PAGES);
	}
#endif
	seq_printf(m, "lmkd_media_zone_free:  %ld\n", media_zone_nr_free);

	return 0;
}

static int lmkd_dbg_trigger_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, lmkd_dbg_trigger_proc_show, NULL);
}

static ssize_t lmkd_dbg_trigger_write(struct file *file, const char __user *buf,
					size_t count, loff_t *ppos)
{
	char buffer[PROC_NUMBUF];
	int oom_score_adj;
	int err;

	pr_err("%s: show lowmem status\n", __func__);

	memset(buffer, 0, sizeof(buffer));
	if (count > sizeof(buffer) - 1)
		count = sizeof(buffer) - 1;
	if (copy_from_user(buffer, buf, count)) {
		err = -EFAULT;
		goto out;
	}

	err = kstrtoint(strstrip(buffer), 0, &oom_score_adj);
	if (err)
		goto out;

	if (oom_score_adj < OOM_SCORE_ADJ_MIN ||
	    oom_score_adj > OOM_SCORE_ADJ_MAX) {
		err = -EINVAL;
		goto out;
	}

	lmkd_oom_score_adj = oom_score_adj;
	hisi_lowmem_dbg(oom_score_adj);

out:
	return err < 0 ? err : count;
}

static const struct file_operations lmkd_dbg_trigger_proc_fops = {
	.open		= lmkd_dbg_trigger_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
	.write		= lmkd_dbg_trigger_write,
};

static int __init proc_lmkd_dbg_trigger_init(void)
{
	proc_create("lmkd_dbg_trigger", 0660, NULL,
		    &lmkd_dbg_trigger_proc_fops);
	return 0;
}

fs_initcall(proc_lmkd_dbg_trigger_init);
