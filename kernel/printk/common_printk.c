/*
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
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
#include "common_printk.h"

#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <securec.h>
#include <linux/version.h>
#ifdef CONFIG_HUAWEI_PRINTK_CTRL
#include <linux/hisi/hw_cmdline_parse.h> /* for runmode_is_factory */
#include <log/log_usertype.h>
#endif

#define SCBBPD_RX_STAT1 0x534
#define SCBBPD_RX_STAT2 0x538

#define HAS_INITED_TIME_ADDR 1
#define HAS_NOT_INITED_TIME_ADDR 0
#define TRUE 1
#define SECONDS_OF_MINUTE 60
#define BEGIN_YEAR 1900
#define REG_NUMS 3

#define FPGA_SECOND_TICK_CNT 19200000
#define SECOND_TICK_CNT 32764
#define LOG_STORE_CYCLE_COUNT 9

#define MAX_TIMESTAMP 15724800000000000 /* half a year in nano second */
#define STEP_LEN LOG_ALIGN
#define HEAD_LEN sizeof(struct printk_log)

static unsigned int scbbpdrxstat1;
static unsigned int scbbpdrxstat2;
static unsigned int fpga_flag;
static void __iomem *addr;
static int init_time_addr;

u64 hisi_getcurtime(void)
{
	u64 timervalue[4]; /* there are 4 time registers */
	u64 pcurtime;
	u64 ts;

	if (!init_time_addr)
		return 0;

	timervalue[0] = readl(addr + scbbpdrxstat1);
	timervalue[1] = readl(addr + scbbpdrxstat2);
	timervalue[2] = readl(addr + scbbpdrxstat1);
	timervalue[3] = readl(addr + scbbpdrxstat2);

	/* make up current time value */
	if (timervalue[2] < timervalue[0])
		pcurtime = (((timervalue[3] - 1) << 32) | timervalue[0]);
	else
		pcurtime = ((timervalue[1] << 32) | timervalue[0]);

	/* convert system tick count to ns */
	if (fpga_flag == FPGA_CONFIGED) {
		ts = do_div(pcurtime, FPGA_SECOND_TICK_CNT);
		
		ts = ts * 625;
		do_div(ts, 12);
		pcurtime = pcurtime * NS_INTERVAL + ts;
	} else {
		ts = do_div(pcurtime, SECOND_TICK_CNT);
		
		ts = ts * 1953125;
		do_div(ts, 64);
		pcurtime = pcurtime * NS_INTERVAL + ts;
	}

	return pcurtime;
}

size_t print_time(u64 ts, char *buf)
{
	int temp;
	int ret;
	char ts_buf[TS_BUF_SIZE] = {0};
	unsigned long rem_nsec;

	rem_nsec = do_div(ts, NS_INTERVAL);

	if (!printk_time)
		return 0;

	if (!buf) {
		ret = snprintf_s(ts_buf, TS_BUF_SIZE, TS_BUF_SIZE - 1,
			"[%5lu.000000s]", (unsigned long)ts
			);
		if (ret >= 0)
			return (unsigned int)ret;
		else
			return 0;
	}

	/* cppcheck-suppress */
	temp = sprintf_s(buf, TIME_LOG_SIZE, "[%5lu.%06lus]", (unsigned long)ts,
		rem_nsec / 1000); /* convert to us */
	if (temp >= 0)
		return (unsigned int)temp;
	else
		return 0;
}

static int __init uniformity_timer_init(void)
{
	struct device_node *np = NULL;
	int ret;

	np = of_find_compatible_node(NULL, NULL, "hisilicon,prktimer");
	if (!np) {
		pr_err("NOT FOUND device node 'prktimer'!\n");
		return -ENXIO;
	}

	if (addr) {
		init_time_addr = HAS_NOT_INITED_TIME_ADDR;
		early_iounmap(addr, SZ_4K);
		addr = NULL;
	}

	addr = of_iomap(np, 0);
	if (!addr) {
		pr_err("get prktimer resource fail, lisc addr init fail\n");
		return -ENXIO;
	}

	ret = of_property_read_u32(np, "fpga_flag", &fpga_flag);
	if (ret) {
		pr_err("failed to get fpga_flag resource\n");
		return -ENXIO;
	}

	if (fpga_flag == FPGA_CONFIGED) {
		scbbpdrxstat1 = FPGA_SCBBPD_RX_STAT1;
		scbbpdrxstat2 = FPGA_SCBBPD_RX_STAT2;
	} else {
		scbbpdrxstat1 = SCBBPD_RX_STAT1;
		scbbpdrxstat2 = SCBBPD_RX_STAT2;
	}
	init_time_addr = HAS_INITED_TIME_ADDR;

	return 0;
}

#ifdef CONFIG_HISI_APANIC
static void panic_print_msg(struct printk_log *msg)
{
	char *text = (char *)msg + sizeof(struct printk_log);
	size_t text_size = msg->text_len;
	char time_log[TIME_LOG_SIZE] = "";
	size_t tlen;
	char *ptime_log = NULL;

	ptime_log = time_log;

	do {
		const char *next = memchr(text, '\n', text_size);
		size_t text_len;

		if (next) {
			text_len = next - text;
			next++;
			text_size -= next - text;
		} else {
			text_len = text_size;
		}
		tlen = print_time(msg->ts_nsec, ptime_log);
		apanic_console_write(ptime_log, tlen);
		apanic_console_write(text, text_len);
		apanic_console_write("\n", 1);

		text = (char *)next;
	} while (text);
}
#endif

void pr_log_store_add_time(
	char *pr_char, u32 sizeof_pr_char, u16 *pr_len)
{
	struct tm tm_rtc;
	unsigned long cur_secs;
	static unsigned long prev_jffy;
	static unsigned int prejf_init_flag;
	static int number;
	int ret = 0;

	if (!prejf_init_flag) {
		prejf_init_flag = TRUE;
		prev_jffy = jiffies;
	}
	cur_secs = get_seconds();
	cur_secs -= sys_tz.tz_minuteswest * SECONDS_OF_MINUTE;
	time_to_tm(cur_secs, 0, &tm_rtc);
	if (time_after(jiffies, prev_jffy + 1 * HZ)) {
		prev_jffy = jiffies;
		ret = snprintf_s(
			pr_char, sizeof_pr_char, sizeof_pr_char - 1,
			"[%lu:%.2d:%.2d %.2d:%.2d:%.2d]",
			BEGIN_YEAR + tm_rtc.tm_year,
			tm_rtc.tm_mon + 1, tm_rtc.tm_mday, tm_rtc.tm_hour,
			tm_rtc.tm_min, tm_rtc.tm_sec);
		if (ret < 0) {
			pr_err("%s: snprintf_s failed\n", __func__);
			return;
		}
		*pr_len += ret;
	}

#if defined(CONFIG_HISI_PRINTK_EXTENSION)
	ret = snprintf_s(pr_char + *pr_len, sizeof_pr_char - *pr_len,
		sizeof_pr_char - *pr_len - 1,
		"[pid:%d,cpu%u,%s,%d]", current->pid,
		smp_processor_id(), in_irq() ? "in irq" : current->comm, number);
	number += 1;
	if (number > LOG_STORE_CYCLE_COUNT)
		number = 0;
#endif
	if (ret < 0) {
		pr_err("%s: snprintf_s failed\n", __func__);
		return;
	}
	*pr_len += ret;
}
pure_initcall(uniformity_timer_init);

int get_console_index(void)
{
	if ((preferred_console != -1) &&
		(preferred_console < MAX_CMDLINECONSOLES))
		return console_cmdline[preferred_console].index;

	return -1;
}

int get_console_name(char *name, int name_buf_len)
{
	int ret;
	u32 strcount;

	if (!name) {
		pr_err("%s:name is NULL\n", __func__);
		return -1;
	}

	if (name_buf_len <= 0) {
		pr_err("%s:name_buf_len is invalid\n", __func__);
		return -1;
	}

	if ((preferred_console != -1) &&
		(preferred_console < MAX_CMDLINECONSOLES)) {
		strcount = (u32)min(
			(int)(sizeof(console_cmdline[preferred_console].name)),
			name_buf_len - 1);
		ret = strncpy_s(name, name_buf_len,
			console_cmdline[preferred_console].name,
			strcount);
		if (ret) {
			pr_err("%s: strncpy_s failed\n", __func__);
			return -1;
		}
	}

	return 0;
}

#ifdef CONFIG_HUAWEI_PRINTK_CTRL
int printk_level = LOGLEVEL_DEBUG;
int sysctl_printk_level = LOGLEVEL_DEBUG;
/*
 * if loglevel > level, the log will not be saved to memory in no log load
 * log load and factory mode load will not be affected
 */
void printk_level_setup(int level)
{
	if (runmode_is_factory())
		return;

	pr_alert("%s: %d\n", __func__, level);
	raw_spin_lock(g_logbuf_level_lock_ex);
	if (level >= LOGLEVEL_EMERG && level <= LOGLEVEL_DEBUG)
		printk_level = level;
	raw_spin_unlock(g_logbuf_level_lock_ex);
}
#endif

static int __init early_printk_timer_setup(char *str)
{
	int i = 0;
	int ret;
	char *token = NULL;
	unsigned long long regs[REG_NUMS] = {};

	while ((token = strsep(&str, ","))) {
		token = strim(token);
		if (!*token)
			continue;

		if (i >= REG_NUMS)
			goto fmt_err;

		ret = kstrtoull(token, 0, &regs[i++]);
		if (ret < 0)
			goto fmt_err;
	}

	if (i != REG_NUMS)
		goto fmt_err;

	if (!regs[0])
		goto fmt_err;

	addr = early_ioremap(regs[0], SZ_4K);
	if (!addr) {
		pr_err("%s fail to ioremap\n", __func__);
		return -ENOMEM;
	}

	scbbpdrxstat1 = regs[1];
	scbbpdrxstat2 = regs[2];
	init_time_addr = HAS_INITED_TIME_ADDR;

	return 0;

fmt_err:
	pr_err("should be printktimer=<base>,<offset>,<offset>\n");
	return -EINVAL;
}

early_param("printktimer", early_printk_timer_setup);

static struct printk_log *get_record_by_idx(const char *buf, u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(buf + idx);

	if (msg->len)
		return msg;

	return (struct printk_log *)buf;
}

static u64 get_msg_ts(const char *buf, u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(buf + idx);

	return msg->ts_nsec;
}

static u32 get_next_record(const char *buf, u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(buf + idx);

	if (msg->len)
		return idx + msg->len;

	return 0;
}

static bool is_msg_valid(const char *buf, u32 idx)
{
	struct printk_log *msg = (struct printk_log *)(buf + idx);
	u32 pad_len;

	if (idx + msg->len > __LOG_BUF_LEN)
		return false;

	if (msg->ts_nsec == 0 || msg->ts_nsec > MAX_TIMESTAMP)
		return false;

	if (msg->len != msg_used_size(msg->text_len, msg->dict_len, &pad_len))
		return false;

	return true;
}

static bool search_log_buff(u32 idx, const char *src, u32 *head, u32 *tail)
{
	struct printk_log *head_msg = NULL;
	u64 ts;

	pr_err("%s:begin search head\n", __func__);
	while (true) {
		idx += STEP_LEN;
		if (idx >= __LOG_BUF_LEN - HEAD_LEN) {
			pr_err("%s:can not find head record,idx=%u\n",
			    __func__, idx);
			return false;
		}
		if (is_msg_valid(src, idx))
			break;
	}
	if (idx < HEAD_LEN) {
		pr_err("%s:reach the head,idx=%u\n", __func__, idx);
		return false;
	}
	head_msg = (struct printk_log *)(src + idx);
	ts = get_record_by_idx(src, *tail)->ts_nsec;
	if (head_msg->ts_nsec >= ts) {
		pr_err("%s:head record ts incorrect, head=%llu,tail=%llu\n",
		    __func__, head_msg->ts_nsec, ts);
		return false;
	}
	*head = (char *)head_msg - src;
	pr_err("%s:search head=%u success\n", __func__, *head);

	return true;
}

static bool find_head_index(u32 *head_idx, u32 *tail_idx, const char *src)
{
	u32 cur_idx = 0;
	const struct printk_log *msg = NULL;
	u64 tail_ts;

	while (true) {
		msg = get_record_by_idx(src, cur_idx);
		if (!is_msg_valid(src, cur_idx)) {
			if (!search_log_buff(cur_idx, src, head_idx, tail_idx))
				return false;
			return true;
		}

		if (!msg->len) {
			pr_err("%s:cannot find head until bottom, idx=%u\n",
			    __func__, cur_idx);
			return true;
		}
		tail_ts = get_msg_ts(src, *tail_idx);
		if (cur_idx != *tail_idx &&
		    msg->ts_nsec < tail_ts) {
			pr_err("%s:find head,h=%u,ts=%llu,t=%u,ts=%llu\n",
			    __func__, cur_idx, msg->ts_nsec,
			    *tail_idx, tail_ts);
			*head_idx = cur_idx;
			return true;
		}

		*tail_idx = cur_idx;
		cur_idx = get_next_record(src, cur_idx);
		if (cur_idx > __LOG_BUF_LEN - HEAD_LEN) {
			pr_err("%s:index %u too big\n", __func__, cur_idx);
			return false;
		}
	}
}

static bool check_all_record(u32 head_idx, u32 tail_idx, const char *src)
{
	u32 cur_idx = head_idx;
	char zero_record[HEAD_LEN] = {0};

	if (head_idx == tail_idx) {
		pr_err("%s:check failed,head=%u,tail=%u\n", __func__,
		    head_idx, tail_idx);
		return false;
	}

	while (cur_idx != tail_idx) {
		if (!is_msg_valid(src, cur_idx) &&
		    memcmp(src + cur_idx, zero_record, HEAD_LEN)) {
			pr_err("%s:record wrong,idx=%u\n", __func__, cur_idx);
			return false;
		}
		cur_idx = get_next_record(src, cur_idx);
	}

	return true;
}

int decode_log_buff(char *dst, size_t size, const char *src)
{
	u32 cur_idx;
	u32 head_idx = 0;
	u32 tail_idx = 0;
	size_t written = 0;
	const struct printk_log *msg = NULL;

	if (!find_head_index(&head_idx, &tail_idx, src))
		return -1;

	if (!check_all_record(head_idx, tail_idx, src))
		return -1;

	cur_idx = head_idx;
	while (cur_idx != tail_idx) {
		msg = get_record_by_idx(src, cur_idx);
		written += msg_print_text(msg, true, dst + written,
		    size - written);
		if (written > size) {
			pr_err("%s:out buf not enough,written=%lu\n", __func__,
			    written);
			return false;
		}
		cur_idx = get_next_record(src, cur_idx);
	}
	msg = get_record_by_idx(src, cur_idx);
	written += msg_print_text(msg, true, dst + written, size - written);
	if (written > size) {
		pr_err("%s:out buf is not enough,written=%lu\n", __func__,
		    written);
		return false;
	}

	return written;
}
