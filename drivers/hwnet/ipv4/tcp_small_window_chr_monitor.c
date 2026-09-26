

#include "tcp_small_window_chr_monitor.h"
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/compiler.h>
#include <net/sock.h>
#include <net/netlink.h>
#include <net/tcp.h>

#define TCP_NUM_BASE 10
#define TCP_SMALL_WINDOW 1460
#define TCP_SMALL_WINDOW_NUM 2

MODULE_LICENSE("GPL");

unsigned long g_tcp_small_window = TCP_SMALL_WINDOW_NUM * TCP_SMALL_WINDOW;
unsigned long g_tcp_small_win_cnt = 50;
unsigned long g_tcp_min_report_time = HZ;
unsigned long g_tcp_max_report_cnt = 5;

static int __init tcp_small_window_module_init(void)
{
	return 0;
}

static void __exit tcp_small_window_module_exit(void)
{
}

module_init(tcp_small_window_module_init);
module_exit(tcp_small_window_module_exit);

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_PROC
static int tcp_small_window_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "tcp_small_window = %d", g_tcp_small_window);
	seq_puts(seq, "\n");
	return 0;
}

static ssize_t tcp_small_window_write(struct file *file,
	const char __user *buffer, size_t count, loff_t *data)
{
	unsigned long small_window;

	if (kstrtoul_from_user(buffer, count, 10, &small_window)) {
		hw_chr_err("unvalid kstrtoul_from_user");
		return -EINVAL;
	}

	if ((small_window) > 0 && (small_window < MAX_SMALL_WINDOW)) {
		g_tcp_small_window = small_window;
		hw_chr_err("tcp_small_window_write g_tcp_small_window = %d",
			g_tcp_small_window);
	} else {
		hw_chr_err("unvalid small_window");
		return -EFAULT;
	}

	return count;
}

static int tcp_small_window_open(struct inode *inode, struct file *file)
{
	return single_open_net(inode, file, tcp_small_window_show);
}

static const struct file_operations g_tcp_small_window_seq_fops = {
	.owner = THIS_MODULE,
	.open = tcp_small_window_open,
	.read = seq_read,
	.write = tcp_small_window_write,
	.llseek = seq_lseek,
	.release = single_release_net,
};

static int tcp_small_win_cnt_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "tcp_small_win_cnt = %d", g_tcp_small_win_cnt);
	seq_puts(seq, "\n");
	return 0;
}

static ssize_t tcp_small_win_cnt_write(struct file *file,
	const char __user *buffer, size_t count, loff_t *data)
{
	unsigned long small_win_cnt;

	if (kstrtoul_from_user(buffer, count, TCP_NUM_BASE, &small_win_cnt)) {
		hw_chr_err("unvalid kstrtoul_from_user");
		return -EINVAL;
	}

	if ((small_win_cnt > 0) && (small_win_cnt <= MAX_SMALL_WIN_CNT)) {
		g_tcp_small_win_cnt = small_win_cnt;
		hw_chr_err("tcp_small_win_cnt_write g_tcp_small_win_cnt = %d",
			g_tcp_small_win_cnt);
	} else {
		hw_chr_err("unvalid small_win_cnt");
		return -EFAULT;
	}

	return count;
}

static int tcp_small_win_cnt_open(struct inode *inode, struct file *file)
{
	return single_open_net(inode, file, tcp_small_win_cnt_show);
}

static const struct file_operations g_tcp_small_win_cnt_seq_fops = {
	.owner = THIS_MODULE,
	.open = tcp_small_win_cnt_open,
	.read = seq_read,
	.write = tcp_small_win_cnt_write,
	.llseek = seq_lseek,
	.release = single_release_net,
};

static int tcp_min_report_time_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "tcp_small_window = %d", g_tcp_min_report_time);
	seq_puts(seq, "\n");
	return 0;
}

static ssize_t tcp_min_report_time_write(struct file *file,
	const char __user *buffer, size_t count, loff_t *data)
{
	unsigned long report_time;

	if (kstrtoul_from_user(buffer, count, 10, &report_time)) {
		hw_chr_err("unvalid kstrtoul_from_user");
		return -EINVAL;
	}

	if ((report_time > 0) && (report_time <= MAX_REPORT_TIME)) {
		g_tcp_min_report_time = report_time;
		hw_chr_err("tcp_min_report_time_write tcp_min_report_time = %d",
			g_tcp_min_report_time);
	} else {
		hw_chr_err("unvalid report_time");
		return -EFAULT;
	}

	return count;
}

static int tcp_min_report_time_open(struct inode *inode, struct file *file)
{
	return single_open_net(inode, file, tcp_min_report_time_show);
}

static const struct file_operations g_tcp_min_report_time_seq_fops = {
	.owner = THIS_MODULE,
	.open = tcp_min_report_time_open,
	.read = seq_read,
	.write = tcp_min_report_time_write,
	.llseek = seq_lseek,
	.release = single_release_net,
};

static int tcp_max_report_cnt_show(struct seq_file *seq, void *v)
{
	seq_printf(seq, "tcp_max_report_cnt = %d", g_tcp_max_report_cnt);
	seq_puts(seq, "\n");
	return 0;
}

static ssize_t tcp_max_report_cnt_write(struct file *file,
	const char __user *buffer, size_t count, loff_t *data)
{
	unsigned long report_cnt;

	if (kstrtoul_from_user(buffer, count, 10, &report_cnt)) {
		hw_chr_err("unvalid kstrtoul_from_user");
		return -EINVAL;
	}

	if ((report_cnt > 0) && (report_cnt <= MAX_REPORT_CNT)) {
		g_tcp_max_report_cnt = report_cnt;
		hw_chr_err("tcp_max_report_cnt_write tcp_max_report_cnt = %d",
			g_tcp_max_report_cnt);
	} else {
		hw_chr_err("unvalid report_cnt");
		return -EFAULT;
	}

	return count;
}

static int tcp_max_report_cnt_open(struct inode *inode, struct file *file)
{
	return single_open_net(inode, file, tcp_max_report_cnt_show);
}

static const struct file_operations g_tcp_max_report_cnt_seq_fops = {
	.owner = THIS_MODULE,
	.open = tcp_max_report_cnt_open,
	.read = seq_read,
	.write = tcp_max_report_cnt_write,
	.llseek = seq_lseek,
	.release = single_release_net,
};

int tcp_small_win_init_proc(struct net *net)
{
	if (!proc_create("chr_small_window",
		(S_IRUSR | S_IRGRP) | (S_IWUSR | S_IWGRP),
		net->proc_net, &g_tcp_small_window_seq_fops)) {
		return -ENOMEM;
	}
	if (!proc_create("chr_small_win_cnt",
		(S_IRUSR | S_IRGRP) | (S_IWUSR | S_IWGRP),
		net->proc_net, &g_tcp_small_win_cnt_seq_fops)) {
		return -ENOMEM;
	}
	if (!proc_create("chr_min_report_time",
		(S_IRUSR | S_IRGRP) | (S_IWUSR | S_IWGRP),
		net->proc_net, &g_tcp_min_report_time_seq_fops)) {
		return -ENOMEM;
	}
	if (!proc_create("chr_max_report_cnt",
		(S_IRUSR | S_IRGRP) | (S_IWUSR | S_IWGRP),
		net->proc_net, &g_tcp_max_report_cnt_seq_fops)) {
		return -ENOMEM;
	}

	return 0;
}

#endif

