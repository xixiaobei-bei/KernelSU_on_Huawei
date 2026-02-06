

#include "wbc_hw_hook.h"
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/ctype.h>
#include <linux/spinlock.h>
#include <linux/netlink.h>
#include <uapi/linux/netlink.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <net/sock.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
#include <net/tcp.h>
#include <hwnet/ipv4/tcp_small_window_chr_monitor.h>
#endif

#include "chr_netlink.h"
#ifdef CONFIG_HW_NETBOOSTER_MODULE
#include "net/netbooster/video_acceleration.h"
#endif

#ifndef DEBUG
#define DEBUG
#endif

/* This is to record the local in page information */
static struct http_stream g_http_para_in;
/* This is to record the local out page information */
static struct http_stream g_http_para_out;
/* The structure in order to record a different page stream with a hash index */
static struct http_stream *g_stream_list;
/* This structure stores the statistics of web pages */
static struct http_return g_rtn_stat[RNT_STAT_SIZE];
static struct http_return g_rtn_stat_wifi[RNT_STAT_SIZE];
/* Return the abnomal infomation */
static struct http_return g_rtn_abn[RNT_STAT_SIZE];
static struct rtt_from_stack g_stack_rtt[RNT_STAT_SIZE];
static unsigned int g_sleep_flag;

/* The HTTP keyword is used to filter tcp packets */
static char g_get_str[] = { 'G', 'E', 'T', 0, 0 };
static char g_http_str[] = { 'H', 'T', 'T', 'P', 0 };
static char g_post_str[] = { 'P', 'O', 'S', 'T', 0 };

/* These parameters are used to store the forbid time */
static unsigned long g_rpt_stamp;
static unsigned long g_abn_stamp_no_ack;
static unsigned long g_abn_stamp_rtt_large;
static unsigned long g_abn_stamp_web_fail;
static unsigned long g_abn_stamp_web_delay;
static unsigned long g_abn_stamp_web_no_ack;
static unsigned long g_abn_stamp_syn_no_ack;
static unsigned long g_abn_stamp_rat_tech_change;
/* These parameters are used to store the forbid time and count */
#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
static unsigned long g_rpt_sock_stamp;
static unsigned long g_upload_cnt;
#endif

static bool g_rtt_flag[RNT_STAT_SIZE];
static bool g_web_deley_flag[RNT_STAT_SIZE];

/* tcp protocol use this semaphone to inform chr netlink thread */
static struct semaphore g_web_stat_sync_sema;
static struct timer_list g_web_stat_timer;
static struct task_struct *g_web_stat_task;
static struct chr_key_val g_chr_key_val;

/* This parameters lock are used to lock the common parameters */
static spinlock_t g_web_stat_lock;
static spinlock_t g_web_para_in_lock;
static spinlock_t g_web_para_out_lock;

static unsigned long *g_abn_stamp_list_syn_no_ack;
static unsigned long *g_abn_stamp_list_web_no_ack;
static unsigned long *g_abn_stamp_list_web_fail;
static unsigned long *g_abn_stamp_list_web_delay;
static unsigned long *g_abn_stamp_list_tcp_rtt_large;
static int g_abn_stamp_list_syn_no_ack_idx;
static int g_abn_stamp_list_web_no_ack_idx;
static int g_abn_stamp_list_web_fail_idx;
static int g_abn_stamp_list_web_delay_idx;
static int g_abn_stamp_list_tcp_rtt_large_idx;

static int g_abn_stamp_syn_no_ack_num;
static int g_abn_stamp_tcp_rtt_large_num;
static int g_abn_stamp_web_delay_num;
static int g_abn_stamp_web_fail_num;
static int g_abn_stamp_web_no_ack_num;

static unsigned long abn_stamp_list_syn_no_ack_update(
	unsigned long time_stamp);
static unsigned long abn_stamp_list_web_no_ack_update(
	unsigned long time_stamp);
static unsigned long abn_stamp_list_web_fail_update(
	unsigned long time_stamp);
static unsigned long abn_stamp_list_web_delay_update(
	unsigned long time_stamp);
static unsigned long abn_stamp_list_tcp_rtt_large_update(
	unsigned long time_stamp);
static void abn_stamp_list_syn_no_ack_print_log(void);

static void save_app_syn_succ(u32 uid, u8 type);
static void save_app_web_no_ack(u32 uid, u8 type);
static void save_app_web_delay(u32 uid, int web_delay, u8 if_type);
static void save_app_web_fail(u32 uid, u8 if_type);
static void save_app_tcp_rtt(u32 uid, u32 tcp_rtt, u8 if_type);
static void save_rtt_stat_flag(u8 idx);
static void save_rtt_num(u8 idx);
static void set_foreground_app_uid(int uid);

static u32 g_s_report_app_uid_lst[CHR_MAX_REPORT_APP_COUNT] = {0};
static int g_data_reg_tech;
static int g_old_data_reg_tech;
int g_app_top_uid;

static u32 http_response_code(const char *pstr);
static void web_delay_rtt_flag_reset(void);
static u32 identify_network_protocol(struct sk_buff *skb,
	struct ipv6hdr **ip6h, struct iphdr **iph, struct tcphdr **tcph);
#ifdef CONFIG_HW_NETBOOSTER_MODULE
static void video_chr_stat_report(void);
#endif

u32 us_cvt_to_ms(u32 seq_rtt_us)
{
	return seq_rtt_us / 1000; /* 1ms equals to 1000us */
}

bool ipv6_address_equal(struct in6_addr src, struct in6_addr dst)
{
	if (src.s6_addr32[IPV6_ADD_P1] == dst.s6_addr32[IPV6_ADD_P1] &&
		src.s6_addr32[IPV6_ADD_P2] == dst.s6_addr32[IPV6_ADD_P2] &&
		src.s6_addr32[IPV6_ADD_P3] == dst.s6_addr32[IPV6_ADD_P3] &&
		src.s6_addr32[IPV6_ADD_P4] == dst.s6_addr32[IPV6_ADD_P4])
		return true;
	else
		return false;
}

static inline bool is_ipv6_packet(struct sock *sk)
{
	return ((sk->sk_family == AF_INET6 &&
		!((u32)ipv6_addr_type(&sk->sk_v6_rcv_saddr) &
		IPV6_ADDR_MAPPED)) ? true : false);
}

void notify_chr_thread_to_update_rtt(u32 seq_rtt_us, struct sock *sk,
	u8 data_net_flag)
{
	struct in6_addr dst_v6;
	bool ipv6_flag = false;
	u8 interface_type;

	if (seq_rtt_us <= 0)
		return;

	if (get_uid_from_sock(sk) != g_app_top_uid)
		return;

	if (!spin_trylock_bh(&g_web_stat_lock))
		return;
	if (sk == NULL)
		return;
	/* identify the network protocol of an IP packet */
	if (is_ipv6_packet(sk)) {
		ipv6_flag = true;
		dst_v6 = sk->sk_v6_daddr;
	} else {
		ipv6_flag = false;
	}

	/* identify the interface type */
	if (data_net_flag)
		interface_type = RMNET_INTERFACE;
	else
		interface_type = WLAN_INTERFACE;

	/* interface type 2 and 3 are reserved for ipv6 stats */
	interface_type += IPV6_INTERFACE * ipv6_flag;

	if (seq_rtt_us < SEQ_MAX_RTT) {
		g_stack_rtt[interface_type].tcp_rtt = us_cvt_to_ms(seq_rtt_us);
		g_stack_rtt[interface_type].is_valid = IS_USE;
		g_stack_rtt[interface_type].uid = get_uid_from_sock(sk);
		if (ipv6_flag == true)
			g_stack_rtt[interface_type].rtt_dst_v6_addr = dst_v6;
		else
			g_stack_rtt[interface_type].rtt_dst_addr = sk->sk_daddr;
	}
	spin_unlock_bh(&g_web_stat_lock);
	up(&g_web_stat_sync_sema);
}

void chr_update_buf_time(s64 time, u32 protocal)
{
	ktime_t kt;
	s64 buff;
	s64 cur_buf;
	unsigned long jif;
	long dif_jif;

	if (time == 0)
		return;

	jif = jiffies;
	switch (protocal) {
	case SOL_TCP:
		kt = ktime_get_real();
		dif_jif = (long)(jif - g_chr_key_val.tcp_last);
		cur_buf = ktime_to_ns(kt) - time;
		if (cur_buf < 0)
			cur_buf = 0;

		if (dif_jif > FILTER_TIME_LIMIT) {
			atomic_set(&g_chr_key_val.tcp_buf_time, cur_buf);
		} else {
			buff = atomic_read(&g_chr_key_val.tcp_buf_time);
			buff = buff - buff / ALPHA_FILTER_PARA +
				cur_buf / ALPHA_FILTER_PARA;
			atomic_set(&g_chr_key_val.tcp_buf_time, buff);
		}

		g_chr_key_val.tcp_last = jif;
		break;
	case SOL_UDP:
		kt = ktime_get_real();
		dif_jif = (long)(jif - g_chr_key_val.udp_last);
		cur_buf = ktime_to_ns(kt) - time;
		if (cur_buf < 0)
			cur_buf = 0;

		if (dif_jif > FILTER_TIME_LIMIT) {
			atomic_set(&g_chr_key_val.udp_buf_time, cur_buf);
		} else {
			buff = atomic_read(&g_chr_key_val.udp_buf_time);
			buff = buff - buff / ALPHA_FILTER_PARA +
				cur_buf / ALPHA_FILTER_PARA;
			atomic_set(&g_chr_key_val.udp_buf_time, buff);
		}

		g_chr_key_val.udp_last = jif;
		break;
	default:
		break;
	}
}

/*
 * This is the buffer time update function of the TCP/IP protocol stack,
 * which is passively obtained from the upper layer.
 * Return: buffer time
 */
static u32 report_buf(void)
{
	u16 tmp_buf;
	u32 buf_rtn;
	s64 buf64;
	unsigned long jif;
	long dif_jif;

	jif = jiffies;
	buf64 = atomic_read(&g_chr_key_val.udp_buf_time);
	tmp_buf = (u16) (buf64 / NS_CONVERT_TO_MS);
	if (buf64 > MAX_VALID_NS)
		tmp_buf = MAX_VALID_U16;

	dif_jif = (long)(jif - g_chr_key_val.udp_last);
	if (dif_jif > DIF_JIF_HIGH || dif_jif < DIF_JIF_LOW)
		tmp_buf = 0;

	buf_rtn = tmp_buf;
	buf64 = atomic_read(&g_chr_key_val.tcp_buf_time);
	tmp_buf = (u16) (buf64 / NS_CONVERT_TO_MS);
	if (buf64 > MAX_VALID_NS)
		tmp_buf = MAX_VALID_U16;

	dif_jif = (long)(jif - g_chr_key_val.tcp_last);
	if (dif_jif > DIF_JIF_HIGH || dif_jif < DIF_JIF_LOW)
		tmp_buf = 0;

	buf_rtn = tmp_buf + (buf_rtn << 16); /* binary left shift 16 bits */
	return buf_rtn;
}

u32 get_data_reg_type_chr_enum(int reg_type)
{
	switch (reg_type) {
	case RIL_RADIO_TECHNOLOGY_LTE:
	case RIL_RADIO_TECHNOLOGY_LTE_CA:
		return CHR_DATA_REG_TYPE_LTE;
	case RADIO_TECHNOLOGY_LTE_EN_DC:
		return CHR_DATA_REG_TYPE_ENDC;
	case RIL_RADIO_TECHNOLOGY_NR:
		return CHR_DATA_REG_TYPE_NR;
	default:
		break;
	}
	return CHR_DATA_REG_TYPE_LTE;
}

void web_stat_no_ack_report(u8 interface_type, u8 hash_cnt)
{
	int net_type;
	unsigned long abn_stamp;

	g_abn_stamp_web_no_ack_num++;
	abn_stamp = abn_stamp_list_web_no_ack_update(jiffies);
	if (time_after(jiffies, g_abn_stamp_web_no_ack) &&
		time_before(jiffies, abn_stamp + WEB_NO_ACK_REPORT_TIME)) {
		net_type = interface_type % IPV6_INTERFACE;
		if (interface_type < IPV6_INTERFACE) {
			g_rtn_abn[net_type].report_type = WEB_NO_ACK;
			g_rtn_abn[net_type].server_addr =
				g_stream_list[hash_cnt].dst_addr;
		} else {
			g_rtn_abn[net_type].report_type = WEB_NO_ACK_V6;
			g_rtn_abn[net_type].server_v6_addr =
				g_stream_list[hash_cnt].dst_v6_addr;
		}
		g_rtn_abn[net_type].uid = g_stream_list[hash_cnt].uid;
		g_rtn_abn[net_type].data_reg_tech =
			get_data_reg_type_chr_enum(g_data_reg_tech);
		g_rtn_abn[net_type].total_num = g_abn_stamp_web_no_ack_num;
		g_abn_stamp_web_no_ack_num = 0;
		g_rtn_abn[net_type].http_resp = 0xffffffff;
		spin_unlock_bh(&g_web_stat_lock);
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			g_rtn_abn);
		pr_info("chr: web no ack report s:%x-d:%x>:%x\n",
			g_stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].tcp_port);
		spin_lock_bh(&g_web_stat_lock);
		memset(&g_rtn_abn, 0, sizeof(g_rtn_abn));
		g_abn_stamp_web_no_ack = jiffies + FORBID_TIME;
	}
}

void syn_stat_no_ack_report(u8 interface_type, u32 hash_cnt)
{
	int net_type;
	unsigned long abn_stamp;

	g_abn_stamp_syn_no_ack_num++;
	abn_stamp = abn_stamp_list_syn_no_ack_update(jiffies);
	if (time_after(jiffies, g_abn_stamp_syn_no_ack) &&
		time_before(jiffies, abn_stamp + SYN_NO_ACK_REPORT_TIME)) {
		net_type = interface_type % IPV6_INTERFACE;
		if (interface_type < IPV6_INTERFACE) {
			g_rtn_abn[net_type].report_type = SYN_NO_ACK;
			g_rtn_abn[net_type].server_addr =
				g_stream_list[hash_cnt].dst_addr;
		} else {
			g_rtn_abn[net_type].report_type = SYN_NO_ACK_V6;
			g_rtn_abn[net_type].server_v6_addr =
				g_stream_list[hash_cnt].dst_v6_addr;
		}
		g_rtn_abn[net_type].uid = g_stream_list[hash_cnt].uid;
		g_rtn_abn[net_type].data_reg_tech =
			get_data_reg_type_chr_enum(g_data_reg_tech);
		g_rtn_abn[net_type].total_num = g_abn_stamp_syn_no_ack_num;
		g_abn_stamp_syn_no_ack_num = 0;
		spin_unlock_bh(&g_web_stat_lock);
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			g_rtn_abn);
		pr_info("chr: syn no ack report s:%x-d:%x>:%x\n",
			g_stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].tcp_port);
		abn_stamp_list_syn_no_ack_print_log();
		spin_lock_bh(&g_web_stat_lock);
		memset(&g_rtn_abn, 0, sizeof(g_rtn_abn));
		g_abn_stamp_syn_no_ack = jiffies + FORBID_TIME;
	}
}

void chr_no_ack_report(u32 hash_cnt, u8 if_type)
{
	u8 type;

	type = g_stream_list[hash_cnt].type;
	if (type == HTTP_GET && time_after(jiffies,
		g_stream_list[hash_cnt].get_time_stamp + EXPIRE_TIME)) {
		g_rtn_stat[if_type].total_num++;
		g_rtn_stat[if_type].no_ack_num++;
		save_app_web_no_ack(g_stream_list[hash_cnt].uid, if_type);
		web_stat_no_ack_report(if_type, hash_cnt);
		g_stream_list[hash_cnt].is_valid = IS_UNUSE;
	} else if (type == HTTP_SYN && time_after(jiffies,
		g_stream_list[hash_cnt].time_stamp + DELETE_TIME)) {
		syn_stat_no_ack_report(if_type, hash_cnt);
		g_stream_list[hash_cnt].is_valid = IS_UNUSE;
	}
}

/*
 * timer's expired process function.
 * In this function, the time-out data stream is discarded
 * and the statistics are reported periodically.
 * @arg1: data stream
 * Return: no
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
static void web_stat_timer(struct timer_list *data)
#else
static void web_stat_timer(unsigned long data)
#endif
{
	u32 hash_cnt;
	int hash_num = 0;
	u8 interface_type;

	spin_lock_bh(&g_web_stat_lock);
	for (hash_cnt = 0; hash_cnt < HASH_MAX; hash_cnt++) {
		if (g_stream_list[hash_cnt].is_valid != IS_USE)
			continue;

		/* interface type 2 and 3 are reserved for ipv6 stats */
		interface_type = g_stream_list[hash_cnt].interface +
			g_stream_list[hash_cnt].proto * IPV6_INTERFACE;

		/* web no ack report */
		chr_no_ack_report(hash_cnt, interface_type);
		hash_num++;
	}

	if (time_after(jiffies, g_rpt_stamp + REPORT_TIME)) {
		g_rpt_stamp = jiffies + REPORT_TIME;
		g_rtn_stat[RMNET_INTERFACE].report_type = WEB_STAT;
		g_rtn_stat[WLAN_INTERFACE].report_type = WEB_STAT;
		g_rtn_stat[RMNET_INTERFACE + IPV6_INTERFACE].report_type =
			WEB_STAT_V6;
		g_rtn_stat[WLAN_INTERFACE + IPV6_INTERFACE].report_type =
			WEB_STAT_V6;
		g_rtn_stat[RMNET_INTERFACE + IPV6_INTERFACE].data_reg_tech =
			get_data_reg_type_chr_enum(g_data_reg_tech);
		spin_unlock_bh(&g_web_stat_lock);
#ifdef CONFIG_HW_NETBOOSTER_MODULE
		video_chr_stat_report();
#endif
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			g_rtn_stat);
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			&g_rtn_stat[IPV6_INTERFACE]);
		spin_lock_bh(&g_web_stat_lock);
		memset(&g_rtn_stat, 0, sizeof(g_rtn_stat));
		web_delay_rtt_flag_reset();
	}

	/* Check if there are timeout entries and remove them */
	if (hash_num > 0) {
		g_sleep_flag = false;
		g_web_stat_timer.expires = jiffies + CHECK_TIME;
		spin_unlock_bh(&g_web_stat_lock);
		add_timer(&g_web_stat_timer);
		return;
	}
	g_sleep_flag = true;
	spin_unlock_bh(&g_web_stat_lock);
}

/*
 * Computes the hash value of the network tcp stream.
 * @arg1: dstination address
 * @arg2: source address
 * @arg3: port number
 * Return: hash value
 */
u8 hash3(u32 dst, u32 src, u32 port)
{
	u32 hash;

	hash = dst + src + port;
	hash = hash + hash / HASH_MAX + hash / HASH_MAX_16_BIT +
		hash / HASH_MAX_24_BIT;
	hash = hash % HASH_MAX;
	return (u8) hash;
}

/*
 * Computes the hash value of the network tcp stream for ipv6 packet.
 * @arg1: dstination ipv6 address
 * @arg2: source ipv6 address
 * @arg3: port number
 * Return: ipv6 hash value
 */
u8 hash3_v6(struct in6_addr dst, struct in6_addr src, u32 port)
{
	u32 hash;

	hash = dst.s6_addr32[IPV6_ADD_P1] + dst.s6_addr32[IPV6_ADD_P2] +
		dst.s6_addr32[IPV6_ADD_P3] + dst.s6_addr32[IPV6_ADD_P4] +
		src.s6_addr32[IPV6_ADD_P1] + src.s6_addr32[IPV6_ADD_P2] +
		src.s6_addr32[IPV6_ADD_P3] + src.s6_addr32[IPV6_ADD_P4] +
		port;
	hash = hash + hash / HASH_MAX + hash / HASH_MAX_16_BIT +
		hash / HASH_MAX_24_BIT;
	hash = hash % HASH_MAX;
	return (u8) hash;
}

bool stream_list_http_para_equal(struct http_stream http_para,
	struct http_stream stream_list, bool is_http_para_in)
{
	u8 proto;

	if (is_http_para_in)
		proto = stream_list.proto;
	else
		proto = http_para.proto;

	if (((proto != IPV6_NETWORK &&
		stream_list.src_addr == http_para.src_addr &&
		stream_list.dst_addr == http_para.dst_addr) ||

		(proto == IPV6_NETWORK &&
		ipv6_address_equal(stream_list.src_v6_addr,
			http_para.src_v6_addr) &&
		ipv6_address_equal(stream_list.dst_v6_addr,
			http_para.dst_v6_addr))) &&

		stream_list.tcp_port == http_para.tcp_port &&
		stream_list.interface == http_para.interface)
		return true;
	else
		return false;
}

void get_http_get_delay(u8 hash_cnt, u8 if_type)
{
	u32 http_get_delay = 0;

	if (g_stream_list[hash_cnt].ack_time_stamp != 0) {
		if (g_http_para_out.time_stamp >=
			g_stream_list[hash_cnt].ack_time_stamp)
			http_get_delay = (g_http_para_out.time_stamp -
				g_stream_list[hash_cnt].ack_time_stamp) * MULTIPLE;
		else
			http_get_delay = (MAX_JIFFIES -
				g_stream_list[hash_cnt].ack_time_stamp +
				g_http_para_out.time_stamp) * MULTIPLE;
	}
	g_rtn_stat[if_type].http_get_delay += http_get_delay;
	g_rtn_stat[if_type].http_send_get_num++;
}

u8 get_hash_cnt(struct http_stream http_para)
{
	u8 hash_cnt;

	if (http_para.proto == IPV6_NETWORK)
		hash_cnt = hash3_v6(http_para.dst_v6_addr,
			http_para.src_v6_addr, http_para.tcp_port);
	else
		hash_cnt = hash3(http_para.dst_addr,
			http_para.src_addr, http_para.tcp_port);
	return hash_cnt;
}

void http_para_out_http_get(u8 cnt)
{
	u8 nwk_type;
	u8 if_type;

	nwk_type = g_http_para_out.proto;
	if_type = g_http_para_out.interface + nwk_type * IPV6_INTERFACE;
	if (stream_list_http_para_equal(g_http_para_out, g_stream_list[cnt],
		false)) {
		g_stream_list[cnt].get_time_stamp = g_http_para_out.time_stamp;
		if (g_stream_list[cnt].interface == g_http_para_out.interface)
			get_http_get_delay(cnt, if_type);
		g_stream_list[cnt].type = HTTP_GET;
	}
}

void out_proc(void)
{
	u8 hash_cnt;

	spin_lock_bh(&g_web_para_out_lock);
	if (g_http_para_out.is_valid != IS_USE) {
		spin_unlock_bh(&g_web_para_out_lock);
		return;
	}

	hash_cnt = get_hash_cnt(g_http_para_out);
	if (g_stream_list[hash_cnt].is_valid == IS_UNUSE) {
		if (g_http_para_out.type == HTTP_SYN)
			memcpy(&g_stream_list[hash_cnt], &g_http_para_out,
				sizeof(g_http_para_out));
	} else if (g_stream_list[hash_cnt].type == HTTP_SYN &&
		g_http_para_out.type == HTTP_GET) {
		http_para_out_http_get(hash_cnt);
	}

	g_http_para_out.is_valid = IS_UNUSE;

	if (g_sleep_flag) {
		g_sleep_flag = false;
		g_web_stat_timer.expires = jiffies + CHECK_TIME;
		spin_unlock_bh(&g_web_para_out_lock);
		add_timer(&g_web_stat_timer);
		return;
	}
	spin_unlock_bh(&g_web_para_out_lock);
}

void wifi_disconnect_report(void)
{
	pr_info("%s: web_stat\n", __func__);
	spin_lock_bh(&g_web_stat_lock);
	g_rpt_stamp = jiffies + REPORT_TIME;
	g_rtn_stat[RMNET_INTERFACE].report_type = WEB_STAT;
	g_rtn_stat[WLAN_INTERFACE].report_type = WEB_STAT;
	g_rtn_stat[RMNET_INTERFACE + IPV6_INTERFACE].report_type = WEB_STAT_V6;
	g_rtn_stat[WLAN_INTERFACE + IPV6_INTERFACE].report_type = WEB_STAT_V6;
	spin_unlock_bh(&g_web_stat_lock);
	chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0, g_rtn_stat);
	chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
		&g_rtn_stat[IPV6_INTERFACE]);
	spin_lock_bh(&g_web_stat_lock);
	memset(&g_rtn_stat, 0, sizeof(g_rtn_stat));
	web_delay_rtt_flag_reset();
	spin_unlock_bh(&g_web_stat_lock);
}

void wifi_kernel_delay_report(struct delay_chr_report *delay_chr)
{
	pr_info("%s:\n", __func__);
	memset(&g_rtn_stat_wifi, 0, sizeof(g_rtn_stat_wifi));
	if (delay_chr == NULL)
		return;

	g_rtn_stat_wifi[WLAN_INTERFACE].report_type = WEB_STAT;
	g_rtn_stat_wifi[WLAN_INTERFACE].exception_cnt = delay_chr->exception_cnt;
	g_rtn_stat_wifi[WLAN_INTERFACE].data_direct = delay_chr->data_direct;
	g_rtn_stat_wifi[WLAN_INTERFACE].transport_delay =
		delay_chr->transport_delay;
	g_rtn_stat_wifi[WLAN_INTERFACE].ip_delay = delay_chr->ip_delay;
	g_rtn_stat_wifi[WLAN_INTERFACE].hmac_delay = delay_chr->hmac_delay;
	g_rtn_stat_wifi[WLAN_INTERFACE].driver_delay = delay_chr->driver_delay;
	g_rtn_stat_wifi[WLAN_INTERFACE].android_uid = delay_chr->android_uid;
	chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
		g_rtn_stat_wifi);
}

bool is_stream_list_unuse(u8 hash_cnt)
{
	if (g_stream_list[hash_cnt].is_valid == IS_UNUSE ||
		(g_stream_list[hash_cnt].type != HTTP_GET &&
			g_stream_list[hash_cnt].type != HTTP_SYN))
		return true;
	else
		return false;
}

u32 get_web_delay(u8 hash_cnt)
{
	u32 web_delay;

	if (g_http_para_in.time_stamp >= g_stream_list[hash_cnt].time_stamp)
		web_delay = (g_http_para_in.time_stamp -
			g_stream_list[hash_cnt].time_stamp) * MULTIPLE;
	else
		web_delay = (MAX_JIFFIES - g_stream_list[hash_cnt].time_stamp +
			g_http_para_in.time_stamp) * MULTIPLE;
	return web_delay;
}

void save_rtn_web_delay(u32 web_delay, u8 interface_type)
{
	if (g_web_deley_flag[interface_type]) {
		g_rtn_stat[interface_type].highest_web_delay = web_delay;
		g_rtn_stat[interface_type].lowest_web_delay = web_delay;
		g_rtn_stat[interface_type].last_web_delay = web_delay;
		g_web_deley_flag[interface_type] = false;
	}

	if (web_delay > g_rtn_stat[interface_type].highest_web_delay)
		g_rtn_stat[interface_type].highest_web_delay = web_delay;
	if (web_delay < g_rtn_stat[interface_type].lowest_web_delay)
		g_rtn_stat[interface_type].lowest_web_delay = web_delay;
	g_rtn_stat[interface_type].last_web_delay = web_delay;

	if (web_delay > DELAY_THRESHOLD_L1 && web_delay <= DELAY_THRESHOLD_L2)
		g_rtn_stat[interface_type].delay_num_l1++;
	else if (web_delay > DELAY_THRESHOLD_L2 &&
		 web_delay <= DELAY_THRESHOLD_L3)
		g_rtn_stat[interface_type].delay_num_l2++;
	else if (web_delay > DELAY_THRESHOLD_L3 &&
		 web_delay <= DELAY_THRESHOLD_L4)
		g_rtn_stat[interface_type].delay_num_l3++;
	else if (web_delay > DELAY_THRESHOLD_L4 &&
		 web_delay <= DELAY_THRESHOLD_L5)
		g_rtn_stat[interface_type].delay_num_l4++;
	else if (web_delay > DELAY_THRESHOLD_L5 &&
		 web_delay <= DELAY_THRESHOLD_L6)
		g_rtn_stat[interface_type].delay_num_l5++;
	else if (web_delay > DELAY_THRESHOLD_L6)
		g_rtn_stat[interface_type].delay_num_l6++;
}

void http_para_in_syn_succ(u8 interface_type, u8 hash_cnt)
{
	u32 handshake_delay;

	g_rtn_stat[interface_type].tcp_succ_num++;
	if (g_http_para_in.time_stamp >= g_stream_list[hash_cnt].time_stamp)
		handshake_delay = (g_http_para_in.time_stamp -
			g_stream_list[hash_cnt].time_stamp) * MULTIPLE;
	else
		handshake_delay = (MAX_JIFFIES -
			g_stream_list[hash_cnt].time_stamp +
			g_http_para_in.time_stamp) * MULTIPLE;

	g_rtn_stat[interface_type].tcp_handshake_delay += handshake_delay;
	g_stream_list[hash_cnt].ack_time_stamp = g_http_para_in.time_stamp;
	save_app_syn_succ(g_stream_list[hash_cnt].uid, interface_type);
}


void web_stat_succ_report(u8 if_type, u8 hash_cnt, u32 web_delay)
{
	int net_type;
	unsigned long abn_stamp;

	if (web_delay < WEB_DELAY_THRESHOLD)
		return;
	g_abn_stamp_web_delay_num++;
	abn_stamp = abn_stamp_list_web_delay_update(jiffies);
	if (time_after(jiffies, g_abn_stamp_web_delay) &&
		time_before(jiffies, abn_stamp + WEB_DELAY_REPORT_TIME)) {
		net_type = if_type % IPV6_INTERFACE;
		if (if_type < IPV6_INTERFACE) {
			g_rtn_abn[net_type].report_type = WEB_DELAY;
			g_rtn_abn[net_type].server_addr =
				g_stream_list[hash_cnt].dst_addr;
		} else {
			g_rtn_abn[net_type].report_type = WEB_DELAY_V6;
			g_rtn_abn[net_type].server_v6_addr =
				g_stream_list[hash_cnt].dst_v6_addr;
		}
		g_rtn_abn[net_type].web_delay = web_delay;
		g_rtn_abn[net_type].uid = g_stream_list[hash_cnt].uid;
		g_rtn_abn[net_type].total_num = g_abn_stamp_web_delay_num;
		g_abn_stamp_web_delay_num = 0;
		spin_unlock_bh(&g_web_para_in_lock);
		spin_unlock_bh(&g_web_stat_lock);
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			g_rtn_abn);
		pr_info("chr: web delay report s:%x-d:%x>:%d\n",
			g_stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].tcp_port);
		spin_lock_bh(&g_web_stat_lock);
		memset(&g_rtn_abn, 0, sizeof(g_rtn_abn));
		spin_lock_bh(&g_web_para_in_lock);
		g_abn_stamp_web_delay = jiffies + FORBID_TIME;
	}
}

void web_stat_fail_report(u8 if_type, u8 hash_cnt)
{
	int net_type;
	unsigned long abn_stamp;

	g_abn_stamp_web_fail_num++;
	abn_stamp = abn_stamp_list_web_fail_update(jiffies);
	if (time_after(jiffies, g_abn_stamp_web_fail) &&
		time_before(jiffies, abn_stamp + WEB_FAIL_REPORT_TIME)) {
		net_type = if_type % IPV6_INTERFACE;
		if (if_type < IPV6_INTERFACE) {
			g_rtn_abn[net_type].report_type = WEB_FAIL;
			g_rtn_abn[net_type].server_addr =
				g_stream_list[hash_cnt].dst_addr;
		} else {
			g_rtn_abn[net_type].report_type = WEB_FAIL_V6;
			g_rtn_abn[net_type].server_v6_addr =
				g_stream_list[hash_cnt].dst_v6_addr;
		}
		g_rtn_abn[net_type].uid = g_stream_list[hash_cnt].uid;
		g_rtn_abn[net_type].http_resp = g_http_para_in.resp_code;
		g_rtn_abn[net_type].total_num = g_abn_stamp_web_fail_num;
		g_abn_stamp_web_fail_num = 0;
		spin_unlock_bh(&g_web_para_in_lock);
		spin_unlock_bh(&g_web_stat_lock);
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			g_rtn_abn);
		pr_info("chr: web fail report s:%x-d:%x>:%d\n",
			g_stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
			g_stream_list[hash_cnt].tcp_port);
		spin_lock_bh(&g_web_stat_lock);
		memset(&g_rtn_abn, 0, sizeof(g_rtn_abn));
		spin_lock_bh(&g_web_para_in_lock);
		g_abn_stamp_web_fail = jiffies + FORBID_TIME;
	}
}

void abn_stamp_web_stat_report(u8 interface_type, u8 hash_cnt)
{
	u32 web_delay;

	switch (g_http_para_in.type) {
	case WEB_SUCC:
		g_rtn_stat[interface_type].total_num++;
		g_rtn_stat[interface_type].succ_num++;
		web_delay = get_web_delay(hash_cnt);
		g_rtn_stat[interface_type].web_delay += web_delay;
		save_rtn_web_delay(web_delay, interface_type);
		save_app_web_delay(g_stream_list[hash_cnt].uid, web_delay,
			interface_type);
		web_stat_succ_report(interface_type, hash_cnt, web_delay);
		g_stream_list[hash_cnt].is_valid = IS_UNUSE;
		break;

	case WEB_FAIL:
		g_rtn_stat[interface_type].total_num++;
		g_rtn_stat[interface_type].fail_num++;
		save_app_web_fail(g_stream_list[hash_cnt].uid, interface_type);
		web_stat_fail_report(interface_type, hash_cnt);
		g_stream_list[hash_cnt].is_valid = IS_UNUSE;
		break;

	case SYN_SUCC:
		http_para_in_syn_succ(interface_type, hash_cnt);
		break;

	default:
		g_stream_list[hash_cnt].is_valid = IS_UNUSE;
		break;
	}
}

void in_proc(void)
{
	u8 hash_cnt;
	u8 nwk_type;
	u8 if_type;

	try_upload_v6_info();
	spin_lock_bh(&g_web_para_in_lock);
	if (g_http_para_in.is_valid == IS_UNUSE) {
		spin_unlock_bh(&g_web_para_in_lock);
		return;
	}

	hash_cnt = get_hash_cnt(g_http_para_in);
	if (is_stream_list_unuse(hash_cnt)) {
		g_http_para_in.is_valid = IS_UNUSE;
		spin_unlock_bh(&g_web_para_in_lock);
		return;
	}

	nwk_type = g_http_para_in.proto;
	if_type = g_http_para_in.interface + nwk_type * IPV6_INTERFACE;
	if (stream_list_http_para_equal(g_http_para_in, g_stream_list[hash_cnt],
		true))
		abn_stamp_web_stat_report(if_type, hash_cnt);
	g_http_para_in.is_valid = IS_UNUSE;

	if (g_sleep_flag) {
		g_sleep_flag = false;
		g_web_stat_timer.expires = jiffies + CHECK_TIME;
		spin_unlock_bh(&g_web_para_in_lock);
		add_timer(&g_web_stat_timer);
		return;
	}
	spin_unlock_bh(&g_web_para_in_lock);
}

static void save_rtt_num(u8 idx)
{
	if (g_stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L5)
		g_rtn_stat[idx].rtt_num_l5++;
	else if (g_stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L4)
		g_rtn_stat[idx].rtt_num_l4++;
	else if (g_stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L3)
		g_rtn_stat[idx].rtt_num_l3++;
	else if (g_stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L2)
		g_rtn_stat[idx].rtt_num_l2++;
	else if (g_stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L1)
		g_rtn_stat[idx].rtt_num_l1++;
}

static void save_rtt_stat_flag(u8 idx)
{
	if (g_rtt_flag[idx]) {
		g_rtn_stat[idx].highest_tcp_rtt = g_stack_rtt[idx].tcp_rtt;
		g_rtn_stat[idx].lowest_tcp_rtt = g_stack_rtt[idx].tcp_rtt;
		g_rtn_stat[idx].last_tcp_rtt = g_stack_rtt[idx].tcp_rtt;
		g_rtt_flag[idx] = false;
	}
	if (g_stack_rtt[idx].tcp_rtt > g_rtn_stat[idx].highest_tcp_rtt)
		g_rtn_stat[idx].highest_tcp_rtt = g_stack_rtt[idx].tcp_rtt;
	if (g_stack_rtt[idx].tcp_rtt < g_rtn_stat[idx].lowest_tcp_rtt)
		g_rtn_stat[idx].lowest_tcp_rtt = g_stack_rtt[idx].tcp_rtt;
	g_rtn_stat[idx].last_tcp_rtt = g_stack_rtt[idx].tcp_rtt;
}

void save_rtn_abn(int idx)
{
	if (idx < IPV6_INTERFACE) {
		g_rtn_abn[idx % IPV6_INTERFACE].report_type = TCP_RTT_LARGE;
		g_rtn_abn[idx % IPV6_INTERFACE].rtt_abn_server_addr =
			g_stack_rtt[idx].rtt_dst_addr;
	} else {
		g_rtn_abn[idx % IPV6_INTERFACE].report_type = TCP_RTT_LARGE_V6;
		g_rtn_abn[idx % IPV6_INTERFACE].rtt_abn_srv_v6_addr =
			g_stack_rtt[idx].rtt_dst_v6_addr;
	}
	g_rtn_abn[idx % IPV6_INTERFACE].tcp_rtt = g_stack_rtt[idx].tcp_rtt;
	g_rtn_abn[idx % IPV6_INTERFACE].uid = g_stack_rtt[idx].uid;
}

static void rtt_large_abn_report(int index)
{
	unsigned long abn_stamp;

	if (g_stack_rtt[index].tcp_rtt < RTT_THRESHOLD)
		return;
	abn_stamp = abn_stamp_list_tcp_rtt_large_update(jiffies);
	g_abn_stamp_tcp_rtt_large_num++;
	if (time_after(jiffies, g_abn_stamp_rtt_large) &&
		time_before(jiffies, abn_stamp + TCP_RTT_LARGE_REPORT_TIME)) {
		g_abn_stamp_rtt_large = jiffies + FORBID_TIME;
		save_rtn_abn(index);
		g_rtn_abn[index % IPV6_INTERFACE].total_num =
			g_abn_stamp_tcp_rtt_large_num;
		g_abn_stamp_tcp_rtt_large_num = 0;
		spin_unlock_bh(&g_web_stat_lock);
		chr_notify_event(CHR_WEB_STAT_EVENT,
			g_user_space_pid, 0, g_rtn_abn);
		pr_info("chr: rtt large report\n");
		spin_lock_bh(&g_web_stat_lock);
		memset(&g_rtn_abn, 0, sizeof(g_rtn_abn));
	}
}

void rtt_proc(void)
{
	int idx;

	for (idx = 0; idx < RNT_STAT_SIZE; idx++) {
		if (g_stack_rtt[idx].is_valid == IS_USE) {
			g_rtn_stat[idx].tcp_total_num++;
			g_rtn_stat[idx].tcp_rtt += g_stack_rtt[idx].tcp_rtt;
			save_rtt_num(idx);
			save_app_tcp_rtt(g_stack_rtt[idx].uid,
				g_stack_rtt[idx].tcp_rtt, idx);
			save_rtt_stat_flag(idx);
			rtt_large_abn_report(idx);
			g_stack_rtt[idx].is_valid = IS_UNUSE;
			if (g_sleep_flag) {
				g_sleep_flag = false;
				g_web_stat_timer.expires = jiffies + CHECK_TIME;
				spin_unlock_bh(&g_web_stat_lock);
				add_timer(&g_web_stat_timer);
				spin_lock_bh(&g_web_stat_lock);
			}
		}
	}
}

void chr_rat_change_notify_event(void)
{
	bool is_need_notify;

	is_need_notify = is_notify_chr_event(g_old_data_reg_tech,
		g_data_reg_tech);
	if (!is_need_notify)
		return;

	g_rtn_stat[RMNET_INTERFACE].data_reg_tech =
		get_data_reg_type_chr_enum(g_old_data_reg_tech);
	g_rtn_stat[RMNET_INTERFACE].report_type = WEB_STAT;
	g_rtn_stat[WLAN_INTERFACE].report_type = WEB_STAT;
	spin_unlock_bh(&g_web_stat_lock);
	if (time_after(jiffies, g_abn_stamp_rat_tech_change)) {
		chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
			g_rtn_stat);
		g_abn_stamp_rat_tech_change =
			jiffies + RAT_TECH_CHANGE_NOTIFY_FORBIDDEN_TIME;
	}
	spin_lock_bh(&g_web_stat_lock);
	g_old_data_reg_tech = g_data_reg_tech;
	memset(&g_rtn_stat, 0, sizeof(g_rtn_stat));
}

static int chr_web_thread(void *data)
{
	while (1) {
		if (kthread_should_stop())
			break;

		down(&g_web_stat_sync_sema);
		spin_lock_bh(&g_web_stat_lock);
		chr_rat_change_notify_event();
		in_proc();
		out_proc();
		rtt_proc();
		spin_unlock_bh(&g_web_stat_lock);
	}
	return 0;
}

u8 http_response_type(const char *pstr)
{
	u8 type;

	type = UN_KNOW;
	if (pstr[HTTP_ACK_FROM_START] == HTTP_RESPONSE_NORMAL ||
		pstr[HTTP_ACK_FROM_START] == HTTP_RESPONSE_REDIRECT)
		type = WEB_SUCC;

	if (pstr[HTTP_ACK_FROM_START] == HTTP_RESPONSE_CLIENT_ERROR ||
		pstr[HTTP_ACK_FROM_START] == HTTP_RESPONSE_SERVER_ERROR)
		type = WEB_FAIL;

	return type;
}

u32 http_response_code(const char *pstr)
{
	u32 code = 0;
	int idx;
	char ch;

	if (pstr == NULL)
		return 0;

	for (idx = 0; idx < CODE_LEN; idx++) {
		ch = pstr[(int)(HTTP_ACK_FROM_START + idx)];
		if (ch >= '0' && ch <= '9')
			/* multiply 10 to add ch */
			code = code * 10 + (ch - '0');
		else
			return 0;
	}
	return code;
}

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
bool tcp_need_trigger_upload(unsigned long small_win_stamp)
{
	if (time_after(jiffies, g_rpt_sock_stamp) &&
		time_after(jiffies, small_win_stamp)) {
		if (g_upload_cnt >= g_tcp_max_report_cnt) {
			g_rpt_sock_stamp = jiffies + TCP_MAX_REPORT_TIME;
			g_upload_cnt = 0;
		} else {
			g_upload_cnt += 1;
		}
		return true;
	}
	return false;
}

void tcp_sock_win_report(struct tcphdr *th, struct sock *sk)
{
	struct tcp_sock *sock = NULL;
	struct http_return rtn_stat_sock[RNT_STAT_SIZE];

	if ((th == NULL) || (sk == NULL))
		return;

	memset(rtn_stat_sock, 0, sizeof(struct http_return) * RNT_STAT_SIZE);
	sock = tcp_sk(sk);
	rtn_stat_sock[WLAN_INTERFACE].report_type = WEB_STAT;
	rtn_stat_sock[WLAN_INTERFACE].sock_uid = get_uid_from_sock(sk);
	rtn_stat_sock[WLAN_INTERFACE].cur_win = th->window;
	rtn_stat_sock[WLAN_INTERFACE].win_cnt = sk->win_cnt;
	rtn_stat_sock[WLAN_INTERFACE].free_space = tcp_space(sk);
	rtn_stat_sock[WLAN_INTERFACE].mime_type = sk->mime_type;
	rtn_stat_sock[WLAN_INTERFACE].tcp_srtt = sock->srtt_us;
#ifdef CONFIG_HW_DPIMARK_MODULE
	rtn_stat_sock[WLAN_INTERFACE].sock_dura = jiffies - sk->sk_born_stamp;
#endif
	pr_info("chr_notify_event: %d, %d, %d, %d, %d, %d, %d\n",
		rtn_stat_sock[WLAN_INTERFACE].sock_uid,
		rtn_stat_sock[WLAN_INTERFACE].sock_dura,
		rtn_stat_sock[WLAN_INTERFACE].cur_win,
		rtn_stat_sock[WLAN_INTERFACE].win_cnt,
		rtn_stat_sock[WLAN_INTERFACE].free_space,
		rtn_stat_sock[WLAN_INTERFACE].mime_type,
		rtn_stat_sock[WLAN_INTERFACE].tcp_srtt);

	chr_notify_event(CHR_WEB_STAT_EVENT, g_user_space_pid, 0,
		rtn_stat_sock);

	if (time_after(jiffies, g_rpt_sock_stamp))
		g_rpt_sock_stamp = jiffies + g_tcp_min_report_time;
}

void tcp_win_monitor(struct sock *sk, struct tcphdr *th,
	char *p_http_str, s64 dlen)
{
	unsigned int cur_win;

	if (sk == NULL || th == NULL || p_http_str == NULL ||
		get_uid_from_sock(sk) == 0)
		return;

	cur_win = (th->window) << (tcp_sk(sk)->rx_opt.rcv_wscale);
	if (cur_win > 0 && cur_win <= g_tcp_small_window) {
		if (sk->win_cnt == 0)
			sk->small_win_stamp = jiffies +
				SMALL_WIN_STAMP_RATIO * g_tcp_min_report_time;
		++sk->win_cnt;
		if (sk->win_cnt > g_tcp_small_win_cnt && sk->win_flag &&
			tcp_need_trigger_upload(sk->small_win_stamp)) {
			sk->win_flag = false;
			tcp_sock_win_report(th, sk);
		}
	} else {
		if (sk->win_cnt > g_tcp_small_win_cnt &&
			tcp_need_trigger_upload(sk->small_win_stamp))
			tcp_sock_win_report(th, sk);

		sk->win_flag = true;
		sk->win_cnt = 0;
	}
}

#endif

bool is_valid_data_reg_tech(void)
{
	switch (g_data_reg_tech) {
	case RIL_RADIO_TECHNOLOGY_LTE:
	case RIL_RADIO_TECHNOLOGY_LTE_CA:
	case RADIO_TECHNOLOGY_LTE_EN_DC:
	case RIL_RADIO_TECHNOLOGY_NR:
		return true;
	default:
		break;
	}
	return false;
}

void set_http_para_out(bool is_ipv6_pkt, struct ipv6hdr *ip6h,
	struct iphdr *iph, struct tcphdr *tcph)
{
	if ((is_ipv6_pkt && ip6h == NULL) || (!is_ipv6_pkt && iph == NULL) ||
		tcph == NULL)
		return;

	g_http_para_out.tcp_port = tcph->source;
	if (is_ipv6_pkt) {
		g_http_para_out.proto = IPV6_NETWORK;
		g_http_para_out.src_v6_addr = ip6h->saddr;
		g_http_para_out.dst_v6_addr = ip6h->daddr;
	} else {
		g_http_para_out.proto = IPV4_NETWORK;
		g_http_para_out.src_addr = iph->saddr;
		g_http_para_out.dst_addr = iph->daddr;
	}
	g_http_para_out.time_stamp = jiffies;
	g_http_para_out.is_valid = IS_USE;
}

static s64 get_dlen(const char *p_http_str, bool is_ipv6_pkt, struct sk_buff *skb,
	struct ipv6hdr *ip6h, struct iphdr *iph)
{
	s64 dlen;

	if (is_ipv6_pkt) {
		if (ip6h == NULL)
			return -1;
		dlen = skb->len - (p_http_str - (char *)ip6h);
	} else {
		if (iph == NULL)
			return -1;
		dlen = skb->len - (p_http_str - (char *)iph);
	}
	return dlen;
}

u32 get_http_stream_iface(struct sk_buff *skb, bool is_http_para_in)
{
	struct tcp_sock *sock = NULL;

	if (strncmp(skb->dev->name, WEB_DS_NET, WEB_DS_NET_LEN)) {
		if (is_http_para_in)
			g_http_para_in.interface = WLAN_INTERFACE;
		else
			g_http_para_out.interface = WLAN_INTERFACE;
	} else {
		if (!is_valid_data_reg_tech())
			return -1;
		if (is_http_para_in) {
			g_http_para_in.interface = RMNET_INTERFACE;
		} else {
			g_http_para_out.interface = RMNET_INTERFACE;
			if (skb->sk->sk_state == TCP_ESTABLISHED) {
				sock = tcp_sk(skb->sk);
				sock->data_net_flag = true;
			}
		}
	}
	return 0;
}

static bool is_http_para_out(s64 dlen, const char *p_http_str)
{
	if (p_http_str == NULL)
		return false;
	if (dlen > WEB_DS_NET_LEN &&
		(strncmp(p_http_str, g_get_str, GET_STR_LEN) == 0 ||
			strncmp(p_http_str, g_post_str, POST_STR_LEN) == 0))
		return true;
	return false;
}

static u32 identify_network_protocol(struct sk_buff *skb,
	struct ipv6hdr **ip6h, struct iphdr **iph, struct tcphdr **tcph)
{
	if (is_ipv6_packet(skb->sk)) {
		*ip6h = ipv6_hdr(skb);
		if (*ip6h == NULL || (*ip6h)->nexthdr != IPPROTO_TCP)
			return -1;
	} else {
		*iph = ip_hdr(skb);
		if (*iph == NULL || (*iph)->protocol != IPPROTO_TCP)
			return -1;
	}

	*tcph = tcp_hdr(skb);
	if (*tcph == NULL || skb->data == NULL || (*tcph)->doff == 0)
		return -1;
	return 0;
}

static unsigned int hook_local_out(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;
	char *p_http_str = NULL;
	bool up_req = false;
	s64 dlen;
	bool is_ipv6_pkt = false;

	if (skb == NULL || skb->sk == NULL)
		return NF_ACCEPT;

	/* identify the network protocol of an IP packet */
	if (identify_network_protocol(skb, &ip6h, &iph, &tcph) == -1)
		return NF_ACCEPT;
	p_http_str = (char *)((u32 *)tcph + tcph->doff);
	is_ipv6_pkt = is_ipv6_packet(skb->sk);
	dlen = get_dlen(p_http_str, is_ipv6_pkt, skb, ip6h, iph);
	if (dlen < 0 || skb->dev == NULL)
		return NF_ACCEPT;
	if (get_http_stream_iface(skb, false) == -1 ||
		htons(tcph->dest) != HTTP_PORT ||
			!spin_trylock_bh(&g_web_para_out_lock))
		return NF_ACCEPT;
	if (get_uid_from_sock(skb->sk) != g_app_top_uid) {
		spin_unlock_bh(&g_web_para_out_lock);
		return NF_ACCEPT;
	}
	if (g_http_para_out.is_valid == IS_UNUSE) {
		/* This is an http ack syn packet processing */
		if (tcph->syn == 1 && tcph->ack == 0) {
			set_http_para_out(is_ipv6_pkt, ip6h, iph, tcph);
			g_http_para_out.type = HTTP_SYN;
			g_http_para_out.uid = get_uid_from_sock(skb->sk);
			up_req = true;
		} else if (is_http_para_out(dlen, p_http_str)) {
#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
			skb->sk->win_cnt = 0;
#endif
			set_http_para_out(is_ipv6_pkt, ip6h, iph, tcph);
			g_http_para_out.type = HTTP_GET;
			up_req = true;
		}
	}
	spin_unlock_bh(&g_web_para_out_lock);

	if (up_req)
		up(&g_web_stat_sync_sema);

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
	tcp_win_monitor(skb->sk, tcph, p_http_str, dlen);
#endif
	return NF_ACCEPT;
}

void set_http_para_in(bool is_ipv6_pkt, struct ipv6hdr *ip6h,
	struct iphdr *iph, struct tcphdr *tcph)
{
	if ((is_ipv6_pkt && ip6h == NULL) || (!is_ipv6_pkt && iph == NULL) ||
		tcph == NULL)
		return;

	g_http_para_in.tcp_port = tcph->dest;
	if (is_ipv6_pkt) {
		g_http_para_in.proto = IPV6_NETWORK;
		g_http_para_in.src_v6_addr = ip6h->daddr;
		g_http_para_in.dst_v6_addr = ip6h->saddr;
	} else {
		g_http_para_in.proto = IPV4_NETWORK;
		g_http_para_in.src_addr = iph->daddr;
		g_http_para_in.dst_addr = iph->saddr;
	}
	g_http_para_in.time_stamp = jiffies;
	g_http_para_in.is_valid = IS_USE;
}

int is_http_response(char *p_http_str, s64 dlen, struct tcphdr *tcph)
{
	if (dlen > HTTP_RSP_LEN &&
		strncmp(p_http_str, g_http_str, HTTP_STR_LEN) == 0 &&
		http_response_type(p_http_str) != UN_KNOW &&
		g_http_para_in.is_valid == IS_UNUSE)
		return 1;
	else if (g_http_para_in.is_valid == IS_UNUSE &&
		 tcph->syn == 1 && tcph->ack == 1)
		return -1;
	return 0;
}

static unsigned int hook_local_in(void *ops, struct sk_buff *skb,
	const struct nf_hook_state *state)
{
	s64 dlen;
	bool up_req = false;
	bool is_ipv6_pkt = false;
	char *p_http_str = NULL;
	struct iphdr *iph = NULL;
	struct tcphdr *tcph = NULL;
	struct ipv6hdr *ip6h = NULL;

	if (skb == NULL || skb->sk == NULL)
		return NF_ACCEPT;

	chr_router_discovery(skb);

	/* identify the network protocol of an IP packet */
	if (identify_network_protocol(skb, &ip6h, &iph, &tcph) == -1)
		return NF_ACCEPT;

	p_http_str = (char *)((u32 *)tcph + tcph->doff);
	is_ipv6_pkt = is_ipv6_packet(skb->sk);
	dlen = get_dlen(p_http_str, is_ipv6_pkt, skb, ip6h, iph);

	if (skb->dev == NULL)
		return NF_ACCEPT;

	if (get_http_stream_iface(skb, true) == -1 ||
		htons(tcph->source) != HTTP_PORT ||
			!spin_trylock_bh(&g_web_para_in_lock))
		return NF_ACCEPT;

	/* Determine whether the received packet is an HTTP response */
	if (is_http_response(p_http_str, dlen, tcph) == 1) {
		set_http_para_in(is_ipv6_pkt, ip6h, iph, tcph);
		g_http_para_in.type = http_response_type(p_http_str);
		g_http_para_in.resp_code = http_response_code(p_http_str);
		up_req = true;
	} else if (is_http_response(p_http_str, dlen, tcph) == -1) {
		set_http_para_in(is_ipv6_pkt, ip6h, iph, tcph);
		g_http_para_in.type = SYN_SUCC;
		up_req = true;
	}
	spin_unlock_bh(&g_web_para_in_lock);
	if (up_req)
		up(&g_web_stat_sync_sema);
	return NF_ACCEPT;
}

static struct nf_hook_ops g_net_hooks[] = {
	{
		.hook = hook_local_in,
#if (KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE)
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook_local_out,
#if (KERNEL_VERSION(4, 4, 0) > LINUX_VERSION_CODE)
		.owner = THIS_MODULE,
#endif
		.pf = PF_INET,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook_local_in,
		.pf = PF_INET6,
		.hooknum = NF_INET_LOCAL_IN,
		.priority = NF_IP_PRI_FILTER - 1,
	},
	{
		.hook = hook_local_out,
		.pf = PF_INET6,
		.hooknum = NF_INET_POST_ROUTING,
		.priority = NF_IP_PRI_FILTER - 1,
	}
};

static void web_delay_rtt_flag_reset(void)
{
	int flag_index;

	for (flag_index = 0; flag_index < RNT_STAT_SIZE; flag_index++) {
		g_rtt_flag[flag_index] = true;
		g_web_deley_flag[flag_index] = true;
	}
}

void variable_initialization(void)
{
	memset(&g_rtn_stat, 0, sizeof(g_rtn_stat));
	memset(&g_rtn_stat_wifi, 0, sizeof(g_rtn_stat_wifi));
	memset(&g_rtn_abn, 0, sizeof(g_rtn_abn));
	memset(&g_http_para_in, 0, sizeof(g_http_para_in));
	memset(&g_http_para_out, 0, sizeof(g_http_para_out));
	memset(&g_stack_rtt, 0, sizeof(g_stack_rtt));
}

void spin_lock_initialization(void)
{
	spin_lock_init(&g_web_stat_lock);
	spin_lock_init(&g_web_para_in_lock);
	spin_lock_init(&g_web_para_out_lock);
	ipv6_spin_lock_init();
	sema_init(&g_web_stat_sync_sema, 0);
}

void timestamp_initialization(void)
{
	g_abn_stamp_no_ack = jiffies;
	g_abn_stamp_rtt_large = jiffies;
	g_abn_stamp_web_fail = jiffies;
	g_abn_stamp_web_delay = jiffies;
	g_abn_stamp_syn_no_ack = jiffies;
	g_abn_stamp_rat_tech_change = jiffies;
	g_chr_key_val.tcp_last = jiffies;
	g_chr_key_val.udp_last = jiffies;
	atomic_set(&g_chr_key_val.tcp_buf_time, 0);
	atomic_set(&g_chr_key_val.udp_buf_time, 0);
}

void timer_initialization(void)
{

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	timer_setup(&g_web_stat_timer, web_stat_timer, 0);
#else
	init_timer(&g_web_stat_timer);
	g_web_stat_timer.data = 0;
	g_web_stat_timer.function = web_stat_timer;
#endif
	g_web_stat_timer.expires = jiffies + CHECK_TIME;
	add_timer(&g_web_stat_timer);
	g_sleep_flag = false;
}

void memset_abn_stamp_list(void)
{
	memset(g_abn_stamp_list_syn_no_ack, 0,
		sizeof(unsigned long) * SYN_NO_ACK_MAX);
	memset(g_abn_stamp_list_web_no_ack, 0,
		sizeof(unsigned long) * WEB_NO_ACK_MAX);
	memset(g_abn_stamp_list_web_fail, 0,
		sizeof(unsigned long) * WEB_FAIL_MAX);
	memset(g_abn_stamp_list_web_delay, 0,
		sizeof(unsigned long) * WEB_DELAY_MAX);
	memset(g_abn_stamp_list_tcp_rtt_large, 0,
		sizeof(unsigned long) * TCP_RTT_LARGE_MAX);
}

void free_abn_stamp_list(void)
{
	if (g_stream_list != NULL) {
		kfree(g_stream_list);
		g_stream_list = NULL;
	}
	if (g_abn_stamp_list_syn_no_ack != NULL) {
		kfree(g_abn_stamp_list_syn_no_ack);
		g_abn_stamp_list_syn_no_ack = NULL;
	}
	if (g_abn_stamp_list_web_no_ack != NULL) {
		kfree(g_abn_stamp_list_web_no_ack);
		g_abn_stamp_list_web_no_ack = NULL;
	}
	if (g_abn_stamp_list_web_fail != NULL) {
		kfree(g_abn_stamp_list_web_fail);
		g_abn_stamp_list_web_fail = NULL;
	}
	if (g_abn_stamp_list_web_delay != NULL) {
		kfree(g_abn_stamp_list_web_delay);
		g_abn_stamp_list_web_delay = NULL;
	}
	if (g_abn_stamp_list_tcp_rtt_large != NULL) {
		kfree(g_abn_stamp_list_tcp_rtt_large);
		g_abn_stamp_list_tcp_rtt_large = NULL;
	}
	pr_info("chr:web stat init fail");
}

int kmalloc_abn_stamp_list(void)
{
	g_abn_stamp_list_syn_no_ack =
		kmalloc(sizeof(unsigned long) * SYN_NO_ACK_MAX, GFP_KERNEL);
	if (g_abn_stamp_list_syn_no_ack == NULL)
		goto error;
	g_abn_stamp_list_web_no_ack =
		kmalloc(sizeof(unsigned long) * WEB_NO_ACK_MAX, GFP_KERNEL);
	if (g_abn_stamp_list_web_no_ack == NULL)
		goto error;
	g_abn_stamp_list_web_fail =
		kmalloc(sizeof(unsigned long) * WEB_FAIL_MAX, GFP_KERNEL);
	if (g_abn_stamp_list_web_fail == NULL)
		goto error;
	g_abn_stamp_list_web_delay =
		kmalloc(sizeof(unsigned long) * WEB_DELAY_MAX, GFP_KERNEL);
	if (g_abn_stamp_list_web_delay == NULL)
		goto error;
	g_abn_stamp_list_tcp_rtt_large =
		kmalloc(sizeof(unsigned long) * TCP_RTT_LARGE_MAX, GFP_KERNEL);
	if (g_abn_stamp_list_tcp_rtt_large == NULL)
		goto error;

	memset_abn_stamp_list();
	g_abn_stamp_list_syn_no_ack_idx = 0;
	g_abn_stamp_list_web_no_ack_idx = 0;
	g_abn_stamp_list_web_fail_idx = 0;
	g_abn_stamp_list_web_delay_idx = 0;
	g_abn_stamp_list_tcp_rtt_large_idx = 0;

	g_abn_stamp_syn_no_ack_num = 0;
	g_abn_stamp_tcp_rtt_large_num = 0;
	g_abn_stamp_web_delay_num = 0;
	g_abn_stamp_web_fail_num = 0;
	g_abn_stamp_web_no_ack_num = 0;
	g_app_top_uid = APP_UID_INVALID;
	return 0;

error:
	free_abn_stamp_list();
	return -1;
}

int registrate_hook_function(void)
{
	int ret;

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
	ret = nf_register_hooks(g_net_hooks, ARRAY_SIZE(g_net_hooks));
#else
	ret = nf_register_net_hooks(&init_net, g_net_hooks,
		ARRAY_SIZE(g_net_hooks));
#endif
	if (ret) {
		pr_info("chr:nf_init_in ret=%d", ret);
		return -1;
	}
	pr_info("chr:web stat init success\n");
	return 0;
}

int web_chr_init(void)
{
	int ret;

	/* Initialize the array */
	g_stream_list = kmalloc(sizeof(struct http_stream)*HASH_MAX,
		GFP_KERNEL);
	if (g_stream_list == NULL)
		return -1;
	memset(g_stream_list, 0, sizeof(struct http_stream)*HASH_MAX);

	/* Variable initialization */
	variable_initialization();
	/* spin lock initialization */
	spin_lock_initialization();
	/* flag initialization */
	web_delay_rtt_flag_reset();
	/* Timestamp initialization */
	timestamp_initialization();

	g_rpt_stamp = jiffies;
	/* kmalloc abn stamp list */
	ret = kmalloc_abn_stamp_list();
	if (ret == -1)
		return -1;

	/* Create a thread */
	g_web_stat_task = kthread_run(chr_web_thread, NULL, "chr_web_thread");

	/* Timer initialization */
	timer_initialization();

	/* Registration hook function */
	ret = registrate_hook_function();
	return ret;
}

void web_chr_exit(void)
{
	nf_unregister_net_hooks(&init_net, g_net_hooks,
		ARRAY_SIZE(g_net_hooks));
	kfree(g_stream_list);
	g_stream_list = NULL;
	kfree(g_abn_stamp_list_syn_no_ack);
	g_abn_stamp_list_syn_no_ack = NULL;
	kfree(g_abn_stamp_list_web_no_ack);
	g_abn_stamp_list_web_no_ack = NULL;
	kfree(g_abn_stamp_list_web_fail);
	g_abn_stamp_list_web_fail = NULL;
	kfree(g_abn_stamp_list_web_delay);
	g_abn_stamp_list_web_delay = NULL;
	kfree(g_abn_stamp_list_tcp_rtt_large);
	g_abn_stamp_list_tcp_rtt_large = NULL;
	pr_info("chr:web stat exit success\n");
}

uid_t get_uid_from_sock(struct sock *sk)
{
	if (sk == NULL)
		return 0;
	return sk->sk_uid.val;
}

unsigned long abn_stamp_list_syn_no_ack_update(
	unsigned long time_stamp)
{
	g_abn_stamp_list_syn_no_ack[g_abn_stamp_list_syn_no_ack_idx] =
		time_stamp;
	g_abn_stamp_list_syn_no_ack_idx =
		(g_abn_stamp_list_syn_no_ack_idx + 1) % SYN_NO_ACK_MAX;
	return g_abn_stamp_list_syn_no_ack[g_abn_stamp_list_syn_no_ack_idx];
}

void abn_stamp_list_syn_no_ack_print_log(void)
{
	int idx;

	for (idx = 0; idx < SYN_NO_ACK_MAX; idx++)
		pr_info("chr:g_abn_stamp_list_syn_no_ack[%d]=%ld\n", idx,
			g_abn_stamp_list_syn_no_ack[idx]);

	pr_info("chr:g_abn_stamp_list_syn_no_ack_idx=%d\n",
		g_abn_stamp_list_syn_no_ack_idx);
}

unsigned long abn_stamp_list_web_no_ack_update(
	unsigned long time_stamp)
{
	g_abn_stamp_list_web_no_ack[g_abn_stamp_list_web_no_ack_idx] =
		time_stamp;
	g_abn_stamp_list_web_no_ack_idx =
		(g_abn_stamp_list_web_no_ack_idx + 1) % WEB_NO_ACK_MAX;
	return g_abn_stamp_list_web_no_ack[g_abn_stamp_list_web_no_ack_idx];
}

unsigned long abn_stamp_list_web_fail_update(unsigned long time_stamp)
{
	g_abn_stamp_list_web_fail[g_abn_stamp_list_web_fail_idx] =
		time_stamp;
	g_abn_stamp_list_web_fail_idx =
		(g_abn_stamp_list_web_fail_idx + 1) % WEB_FAIL_MAX;
	return g_abn_stamp_list_web_fail[g_abn_stamp_list_web_fail_idx];
}

unsigned long abn_stamp_list_web_delay_update(
	unsigned long time_stamp)
{
	g_abn_stamp_list_web_delay[g_abn_stamp_list_web_delay_idx] = time_stamp;
	g_abn_stamp_list_web_delay_idx =
		(g_abn_stamp_list_web_delay_idx + 1) % WEB_DELAY_MAX;
	return g_abn_stamp_list_web_delay[g_abn_stamp_list_web_delay_idx];
}

unsigned long abn_stamp_list_tcp_rtt_large_update(
	unsigned long time_stamp)
{
	int idx;

	g_abn_stamp_list_tcp_rtt_large[g_abn_stamp_list_tcp_rtt_large_idx] =
		time_stamp;
	g_abn_stamp_list_tcp_rtt_large_idx =
		(g_abn_stamp_list_tcp_rtt_large_idx + 1) % TCP_RTT_LARGE_MAX;
	idx = g_abn_stamp_list_tcp_rtt_large_idx;
	return g_abn_stamp_list_tcp_rtt_large[idx];
}

bool is_notify_chr_event(int old_data_reg_tech, int new_data_reg_tech)
{
	int temp_new_data_reg_tech;
	int temp_old_data_reg_tech;

	temp_old_data_reg_tech = old_data_reg_tech;
	temp_new_data_reg_tech = new_data_reg_tech;

	if (temp_old_data_reg_tech == RIL_RADIO_TECHNOLOGY_LTE_CA)
		temp_old_data_reg_tech = RIL_RADIO_TECHNOLOGY_LTE;

	if (temp_new_data_reg_tech == RIL_RADIO_TECHNOLOGY_LTE_CA)
		temp_new_data_reg_tech = RIL_RADIO_TECHNOLOGY_LTE;

	if (temp_old_data_reg_tech == RIL_RADIO_TECHNOLOGY_LTE ||
		temp_old_data_reg_tech == RIL_RADIO_TECHNOLOGY_NR ||
			temp_old_data_reg_tech == RADIO_TECHNOLOGY_LTE_EN_DC) {
		if (temp_old_data_reg_tech != temp_new_data_reg_tech)
			return true;
	}
	return false;
}

int set_report_app_uid(int tag, u32 paras)
{
	if (tag >= 0 && tag < CHR_MAX_REPORT_APP_COUNT) {
		g_s_report_app_uid_lst[tag] = paras;
		return 0;
	}
	if (tag == DATA_REG_TECH_TAG) {
		g_old_data_reg_tech = g_data_reg_tech;
		g_data_reg_tech = paras;
		return 0;
	}
	if (tag == GET_AP_REPORT_TAG) {
		if (paras & 0x01) {
			chr_notify_event(CHR_SPEED_SLOW_EVENT, g_user_space_pid,
				report_buf(), NULL);
			return 0;
		}
	}
	if (tag == SET_FOREGROUND_APP_UID) {
		set_foreground_app_uid(paras);
		return 0;
	}
	pr_info("chr: set 'tag' invaild. tag=%d\n", tag);
	return -1;
}

void save_app_syn_succ(u32 uid, u8 type)
{
	int i;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == g_s_report_app_uid_lst[i]) {
			g_rtn_stat[type].report_app_stat_list[i].tcp_succ_num++;
			break;
		}
	}
}

void save_app_web_no_ack(u32 uid, u8 type)
{
	int i;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == g_s_report_app_uid_lst[i]) {
			g_rtn_stat[type].report_app_stat_list[i].total_num++;
			g_rtn_stat[type].report_app_stat_list[i].no_ack_num++;
			break;
		}
	}
}

void save_app_web_fail(u32 uid, u8 if_type)
{
	int i;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == g_s_report_app_uid_lst[i]) {
			g_rtn_stat[if_type].report_app_stat_list[i].total_num++;
			g_rtn_stat[if_type].report_app_stat_list[i].fail_num++;
			break;
		}
	}
}

static void save_app_web_delay(u32 uid, int web_delay, u8 if_type)
{
	int i;
	struct report_app_stat *app_stat = NULL;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == g_s_report_app_uid_lst[i]) {
			app_stat = &g_rtn_stat[if_type].report_app_stat_list[i];
			app_stat->total_num++;
			app_stat->succ_num++;
			app_stat->web_delay += web_delay;
			if (web_delay > DELAY_THRESHOLD_L1 &&
				web_delay <= DELAY_THRESHOLD_L2)
				app_stat->delay_num_l1++;
			else if (web_delay > DELAY_THRESHOLD_L2 &&
				web_delay <= DELAY_THRESHOLD_L3)
				app_stat->delay_num_l2++;
			else if (web_delay > DELAY_THRESHOLD_L3 &&
				web_delay <= DELAY_THRESHOLD_L4)
				app_stat->delay_num_l3++;
			else if (web_delay > DELAY_THRESHOLD_L4 &&
				web_delay <= DELAY_THRESHOLD_L5)
				app_stat->delay_num_l4++;
			else if (web_delay > DELAY_THRESHOLD_L5 &&
				web_delay <= DELAY_THRESHOLD_L6)
				app_stat->delay_num_l5++;
			else if (web_delay > DELAY_THRESHOLD_L6)
				app_stat->delay_num_l6++;
			break;
		}
	}
}

static void save_app_tcp_rtt(u32 uid, u32 tcp_rtt, u8 if_type)
{
	int i;
	struct report_app_stat *app_stat = NULL;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == g_s_report_app_uid_lst[i]) {
			app_stat = &g_rtn_stat[if_type].report_app_stat_list[i];
			app_stat->tcp_total_num++;
			app_stat->tcp_rtt += tcp_rtt;
			if (tcp_rtt > RTT_THRESHOLD_L1 &&
				tcp_rtt <= RTT_THRESHOLD_L2)
				app_stat->rtt_num_l1++;
			else if (tcp_rtt > RTT_THRESHOLD_L2 &&
				tcp_rtt <= RTT_THRESHOLD_L3)
				app_stat->rtt_num_l2++;
			else if (tcp_rtt > RTT_THRESHOLD_L3 &&
				tcp_rtt <= RTT_THRESHOLD_L4)
				app_stat->rtt_num_l3++;
			else if (tcp_rtt > RTT_THRESHOLD_L4 &&
				tcp_rtt <= RTT_THRESHOLD_L5)
				app_stat->rtt_num_l4++;
			else if (tcp_rtt > RTT_THRESHOLD_L5)
				app_stat->rtt_num_l5++;
			break;
		}
	}
}

void set_foreground_app_uid(int uid)
{
	if (uid <= MIN_APP_UID)
		return;

	spin_lock_bh(&g_web_para_out_lock);
	g_app_top_uid = uid;
	spin_unlock_bh(&g_web_para_out_lock);
}

#ifdef CONFIG_HW_NETBOOSTER_MODULE
static void video_chr_stat_report(void)
{
	struct video_chr_para video_chr = { 0 };

	chr_video_stat(&video_chr);
	g_rtn_stat[RMNET_INTERFACE].vod_avg_speed = video_chr.vod_avg_speed;
	g_rtn_stat[RMNET_INTERFACE].vod_freez_num = video_chr.vod_freez_num;
	g_rtn_stat[RMNET_INTERFACE].vod_time = video_chr.vod_time;
	g_rtn_stat[RMNET_INTERFACE].uvod_avg_speed = video_chr.uvod_avg_speed;
	g_rtn_stat[RMNET_INTERFACE].uvod_freez_num = video_chr.uvod_freez_num;
	g_rtn_stat[RMNET_INTERFACE].uvod_time = video_chr.uvod_time;
}
#endif
#undef DEBUG
