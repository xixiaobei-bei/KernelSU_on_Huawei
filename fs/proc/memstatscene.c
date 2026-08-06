/*
 * memstatscene.c
 *
 * Copyright (C) Huawei Technologies Co., Ltd. 2021. All rights reserved.
 *
 */
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/atomic.h>
#include <linux/uaccess.h>

#define MAX_SCENE_IN_LEN 2
#define CHAR_ZERO '0'

enum memscene_type {
	MSS_NORMAL = 0,
	MSS_BG_LIGHT,
	MSS_APP_GOTOBG,
	MSS_BG_MIDDLE,
	MSS_CAMERA_GOTOBG,
	MSS_APP_COLDSTART,
	MSS_APP_HOTSTART,
	MSS_APP_GOTOFG,
	MSS_CAMERA_GOTOFG,
	MSS_PAUSE_GC,
	MSS_SCENE_COUNT
};

static char * const memscene_type_names[MSS_SCENE_COUNT] = {
	"normal",
	"bgGcLight",
	"appGotoBg",
	"bgGcMiddle",
	"cameraToBg",
	"appCold",
	"appHot",
	"appGotoFg",
	"cameraToFg",
	"pauseGC",
};

static atomic_t mem_stat_scene = ATOMIC_INIT(0);

static int memstatscene_proc_show(struct seq_file *m, void *v)
{
	int memstat = atomic_read(&mem_stat_scene);
	seq_printf(m, "%d%s\n", memstat, memscene_type_names[memstat]);
	return 0;
}

static int memstatscene_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, memstatscene_proc_show, NULL);
}

static bool check_memscene_type(int type)
{
	if (type >= MSS_NORMAL && type < MSS_SCENE_COUNT)
		return true;
	else
		return false;
}

static ssize_t memstatscene_proc_write(struct file *file, const char __user *buffer,
					size_t count, loff_t *ppos)
{
	char scene[MAX_SCENE_IN_LEN] = {0};
	int memstat_in = 0;
	if ((count > sizeof(scene)) || (count <= 0)) {
		pr_err("smartgc %s error count\n", __func__);
		return -EINVAL;
	}
	if (copy_from_user(scene, buffer, count)) {
		pr_err("smartgc %s error copy\n", __func__);
		return -EFAULT;
	}
	scene[MAX_SCENE_IN_LEN - 1] = '\0';

	memstat_in = scene[0] - CHAR_ZERO;
	if (!check_memscene_type(memstat_in)) {
		pr_err("smartgc %s invalid input:%c\n", __func__, scene[0]);
		return -EINVAL;
	}
	atomic_set(&mem_stat_scene, memstat_in);
	pr_info("smartgc %s set scene: %s\n", __func__, memscene_type_names[memstat_in]);

	return count;
}

static const struct file_operations memstat_proc_fops = {
	.open		= memstatscene_proc_open,
	.read		= seq_read,
	.write		= memstatscene_proc_write,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init proc_memstat_init(void)
{
	proc_create("memstatscene", 0664, NULL, &memstat_proc_fops);
	return 0;
}
fs_initcall(proc_memstat_init);
