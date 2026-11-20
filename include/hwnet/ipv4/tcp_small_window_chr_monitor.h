

#ifndef _HWNET_IPV4_TCP_SMALL_WINDOW_CHR_MONITOR_H
#define _HWNET_IPV4_TCP_SMALL_WINDOW_CHR_MONITOR_H

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_PROC
#include <net/net_namespace.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/rtc.h>
#include <linux/kernel.h>
#endif

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_PROC
#define MAX_SMALL_WINDOW (256 * 1460)
#define MAX_SMALL_WIN_CNT (500)
#define MAX_REPORT_TIME (30 * 60 * HZ)
#define MAX_REPORT_CNT (10)
#define COUNT_BUF_LEN (8)

#define hw_chr_err(msg, ...) do { \
	printk(KERN_ERR "#### %s: "msg" ####\n", __func__, ## __VA_ARGS__); \
} while (0)

int tcp_small_win_init_proc(struct net *net);
#endif

extern unsigned long g_tcp_small_window;
extern unsigned long g_tcp_small_win_cnt;
extern unsigned long g_tcp_min_report_time;
extern unsigned long g_tcp_max_report_cnt;

#endif /* _HWNET_IPV4_TCP_SMALL_WINDOW_CHR_MONITOR_H */
