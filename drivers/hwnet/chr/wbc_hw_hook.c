
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
#include <linux/kernel.h>/* add for log */
#include <linux/ctype.h>/* add for tolower */
#include <linux/spinlock.h>/* add for spinlock */
#include <linux/netlink.h>/* add for thread */
#include <uapi/linux/netlink.h>/* add for netlink */
#include <linux/kthread.h>/* add for thread */
#include <linux/jiffies.h>/* add for jiffies */
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <net/sock.h>
#include <linux/skbuff.h>
#include <huawei_platform/log/hw_log.h>
#include <linux/version.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <linux/ipv6.h> /* add for ipv6 */
#include <net/ipv6.h> /* add for ipv6 */
#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
#include <net/tcp.h>
#include <hwnet/ipv4/tcp_small_window_chr_monitor.h>
#endif

#include "wbc_hw_hook.h"
#include "chr_netlink.h"
#ifndef CONFIG_CHR_MTK_NETLINK
#include "net/netbooster/video_acceleration.h"
#endif

#ifndef DEBUG
#define DEBUG
#endif

/*This is to record the local in page information*/
static struct http_stream http_para_in;
/*This is to record the local out page information*/
static struct http_stream http_para_out;
/*The structure in order to record a different page stream with a hash index*/
static struct http_stream *stream_list;
/*This structure stores the statistics of web pages*/
static struct http_return rtn_stat[RNT_STAT_SIZE];
/*Return the abnomal infomation*/
static struct http_return rtn_abn[RNT_STAT_SIZE];
static struct rtt_from_stack stack_rtt[RNT_STAT_SIZE];
static unsigned int sleep_flag;

/*The HTTP keyword is used to filter tcp packets*/
static char g_get_str[] = {'G', 'E', 'T', 0, 0};
static char g_http_str[] = {'H', 'T', 'T', 'P', 0};
static char g_post_str[] = {'P', 'O', 'S', 'T', 0};

/*These parameters are used to store the forbid time*/
static unsigned long rpt_stamp;
static unsigned long abn_stamp_no_ack;
static unsigned long abn_stamp_rtt_large;
static unsigned long abn_stamp_web_fail;
static unsigned long abn_stamp_web_delay;
static unsigned long abn_stamp_syn_no_ack;
static unsigned long abn_stamp_rat_tech_change;
/* These parameters are used to store the forbid time and count */
#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
static unsigned long g_rpt_sock_stamp;
static unsigned long g_upload_cnt;
#endif

static bool rtt_flag[RNT_STAT_SIZE];
static bool web_deley_flag[RNT_STAT_SIZE];

/*tcp protocol use this semaphone to inform chr netlink thread*/
static struct semaphore g_web_stat_sync_sema;
static struct timer_list g_web_stat_timer;
static struct task_struct *g_web_stat_task;
static struct chr_key_val g_chr_key_val;

/*This parameters lock are used to lock the common parameters*/
static spinlock_t g_web_stat_lock;
static spinlock_t g_web_para_in_lock;
static spinlock_t g_web_para_out_lock;

static unsigned long *abn_stamp_list_syn_no_ack;
static unsigned long *abn_stamp_list_web_no_ack;
static unsigned long *abn_stamp_list_web_fail;
static unsigned long *abn_stamp_list_web_delay;
static unsigned long *abn_stamp_list_tcp_rtt_large;
static int abn_stamp_list_syn_no_ack_idx;
static int abn_stamp_list_web_no_ack_idx;
static int abn_stamp_list_web_fail_idx;
static int abn_stamp_list_web_delay_idx;
static int abn_stamp_list_tcp_rtt_large_idx;

static unsigned long abnomal_stamp_list_syn_no_ack_update(
	unsigned long time_stamp);
static unsigned long abnomal_stamp_list_web_no_ack_update(
	unsigned long time_stamp);
static unsigned long abnomal_stamp_list_web_fail_update(
	unsigned long time_stamp);
static unsigned long abnomal_stamp_list_web_delay_update(
	unsigned long time_stamp);
static unsigned long abnomal_stamp_list_tcp_rtt_large_update(
	unsigned long time_stamp);
static void abnomal_stamp_list_syn_no_ack_print_log(void);

static void save_app_syn_succ(u32 uid, u8 interface_type);
static void save_app_web_no_ack(u32 uid, u8 interface_type);
static void save_app_web_delay(u32 uid, int web_delay, u8 interface_type);
static void save_app_web_fail(u32 uid, u8 interface_type);
static void save_app_tcp_rtt(u32 uid, u32 tcp_rtt, u8 interface_type);
static u32 s_report_app_uid_lst[CHR_MAX_REPORT_APP_COUNT] = {0};
static int data_reg_tech = 0;
static int old_data_reg_tech;
static uid_t get_uid_from_sock(struct sock *sk);
static uid_t get_des_addr_from_sock(struct sock *sk);
static u32 http_response_code(char *pstr);
static void web_delay_rtt_flag_reset(void);
#ifdef CONFIG_HW_NETBOOSTER_MODULE
static void video_chr_stat_report(void);
extern int chr_video_stat(struct video_chr_para *report);
#endif

/*us convert to ms*/
u32 us_cvt_to_ms(u32 seq_rtt_us)
{
	return seq_rtt_us/1000;
}

/* To compare two IPv6 addresses */
bool ipv6_address_equal(struct in6_addr src, struct in6_addr dst)
{
	if (src.s6_addr32[0] == dst.s6_addr32[0] &&
		src.s6_addr32[1] == dst.s6_addr32[1] &&
		src.s6_addr32[2] == dst.s6_addr32[2] &&
		src.s6_addr32[3] == dst.s6_addr32[3])
		return true;
	return false;
}

/*To notify thread to update rtt*/
void notify_chr_thread_to_update_rtt(u32 seq_rtt_us, struct sock *sk, u8 data_net_flag)
{
	struct in6_addr dst_v6;
	bool ipv6_flag; /* identify an ipv6 packet*/
	u8 interface_type;

	if (seq_rtt_us <= 0)
		return;
	if (!spin_trylock_bh(&g_web_stat_lock))
		return;

	if (sk == NULL)
		return;

	/* identify the network protocol of an IP packet */
	if (sk->sk_family == PF_INET6) {
		ipv6_flag = true;
		dst_v6 = sk->sk_v6_daddr;
	} else {
		ipv6_flag = false;
	}

	if(data_net_flag) {
		interface_type = RMNET_INTERFACE;
	}
	else {
		interface_type = WLAN_INTERFACE;
	}
	/* interface type 2 and 3 are reserved for ipv6 stats */
	interface_type += IPV6_INTERFACE * ipv6_flag;

	if (seq_rtt_us < MAX_RTT) {

		stack_rtt[interface_type].tcp_rtt = us_cvt_to_ms(seq_rtt_us);
		stack_rtt[interface_type].is_valid = IS_USE;
		stack_rtt[interface_type].uid = get_uid_from_sock(sk);
		if (ipv6_flag == true)
			stack_rtt[interface_type].rtt_dst_v6_addr = dst_v6;
		else
			stack_rtt[interface_type].rtt_dst_addr =
				get_des_addr_from_sock(sk);
	}

    spin_unlock_bh(&g_web_stat_lock);
    up(&g_web_stat_sync_sema);

}

/*Update protocol stack buffer information*/
void chr_update_buf_time(s64 time, u32 protocal)
{
	ktime_t kt;
	s64 buff;
	s64 curBuf;
	unsigned long jif;
	long difJif;

	if (time == 0)
		return;

	jif = jiffies;
	switch (protocal)
	{
	case SOL_TCP:
		kt = ktime_get_real();
		difJif = (long)(jif - g_chr_key_val.tcp_last);
		curBuf = ktime_to_ns(kt) - time;
if (curBuf < 0)
			curBuf = 0;

		if (difJif > FILTER_TIME_LIMIT) {
			atomic_set(&g_chr_key_val.tcp_buf_time, curBuf);
		} else {
			buff = atomic_read(&g_chr_key_val.tcp_buf_time);
			buff = buff - buff / ALPHA_FILTER_PARA + curBuf / ALPHA_FILTER_PARA;
			atomic_set(&g_chr_key_val.tcp_buf_time, buff);
		}

		g_chr_key_val.tcp_last = jif;
		break;
	case SOL_UDP:
		kt = ktime_get_real();
		difJif = (long)(jif - g_chr_key_val.udp_last);
		curBuf = ktime_to_ns(kt) - time;
		if (curBuf < 0)
			curBuf = 0;

		if (difJif > FILTER_TIME_LIMIT) {
			 atomic_set(&g_chr_key_val.udp_buf_time, curBuf);
		} else {
			buff = atomic_read(&g_chr_key_val.udp_buf_time);
			buff = buff - buff / ALPHA_FILTER_PARA + curBuf / ALPHA_FILTER_PARA;
			atomic_set(&g_chr_key_val.udp_buf_time, buff);
		}

		g_chr_key_val.udp_last = jif;
		break;
	default:
		break;
	}
}

/*This is the buffer time update function of the TCP/IP protocol stack,
* which is passively obtained from the upper layer.*/
static u32 reportBuf(void)
{
	u16 tmpBuf;
	u32 bufRtn = 0;
	s64 buf64;
	unsigned long jif;
	long difJif;

	jif = jiffies;

	buf64 = atomic_read(&g_chr_key_val.udp_buf_time);
	tmpBuf = (u16)(buf64 / NS_CONVERT_TO_MS);
	if (buf64 > MAX_VALID_NS)
		tmpBuf = MAX_VALID_U16;

	difJif = (long)(jif - g_chr_key_val.udp_last);
	if (difJif > 2*HZ || difJif < -2*HZ)
		tmpBuf = 0;

	bufRtn = tmpBuf;

	buf64 = atomic_read(&g_chr_key_val.tcp_buf_time);
	tmpBuf = (u16)(buf64 / NS_CONVERT_TO_MS);
	if (buf64 > MAX_VALID_NS)
		tmpBuf = MAX_VALID_U16;

	difJif = (long)(jif - g_chr_key_val.tcp_last);
	if (difJif > 2*HZ || difJif < -2*HZ)
		tmpBuf = 0;

	bufRtn = tmpBuf + (bufRtn << 16);

	return bufRtn;
}

u32 get_data_reg_type_chr_enum(int regType)
{
	switch (regType) {
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

/*timer's expired process function.
* In this function, the time-out data stream is discarded
* and the statistics are reported periodically.*/
static void web_stat_timer(unsigned long data)
{
	u32 hashcnt;
	int hashNum = 0;
	unsigned long abn_stamp;
	u8 interface_type;
	spin_lock_bh(&g_web_stat_lock);

	for (hashcnt = 0; hashcnt < HASH_MAX; hashcnt++) {

		if (stream_list[hashcnt].is_valid != IS_USE)
			continue;
		/* interface type 2 and 3 are reserved for ipv6 stats */
		interface_type = stream_list[hashcnt].interface +
			stream_list[hashcnt].proto * IPV6_INTERFACE;
		if (stream_list[hashcnt].type == HTTP_GET &&
		time_after(jiffies,
		stream_list[hashcnt].get_time_stamp + EXPIRE_TIME)) {
			rtn_stat[interface_type].total_num++;
			rtn_stat[interface_type].no_ack_num++;
			stream_list[hashcnt].is_valid = IS_UNUSE;

			save_app_web_no_ack(stream_list[hashcnt].uid, interface_type);

			abn_stamp =
				abnomal_stamp_list_web_no_ack_update(jiffies);
			if (time_after(jiffies, abn_stamp_no_ack) &&
				time_before(jiffies,
				abn_stamp + WEB_NO_ACK_REPORT_TIME)){
				int net_type = interface_type % IPV6_INTERFACE;

				if (interface_type < IPV6_INTERFACE)
					rtn_abn[net_type].report_type =
						WEB_NO_ACK;
				else
					rtn_abn[net_type].report_type =
						WEB_NO_ACK_V6;
				interface_type %= IPV6_INTERFACE;
				rtn_abn[interface_type].uid = stream_list[hashcnt].uid;
				rtn_abn[interface_type].http_resp = 0xffffffff;
				rtn_abn[interface_type].data_reg_tech =
					get_data_reg_type_chr_enum(data_reg_tech);
				spin_unlock_bh(&g_web_stat_lock);
				chr_notify_event(CHR_WEB_STAT_EVENT,
					g_user_space_pid, 0, rtn_abn);
				pr_info("chr: no ack report s:%x-d:%x>:%x\n",
				stream_list[hashcnt].src_addr & IPV4ADDR_MASK,
				stream_list[hashcnt].dst_addr & IPV4ADDR_MASK,
				stream_list[hashcnt].tcp_port);
				spin_lock_bh(&g_web_stat_lock);
				memset(&rtn_abn, 0, sizeof(rtn_abn));
				abn_stamp_no_ack =
					jiffies + FORBID_TIME;

			}
		}

		if (stream_list[hashcnt].type == HTTP_SYN &&
		time_after(jiffies,
		stream_list[hashcnt].time_stamp + DELETE_TIME)) {

			abn_stamp =
				abnomal_stamp_list_syn_no_ack_update(jiffies);
			if (time_after(jiffies, abn_stamp_syn_no_ack) &&
				time_before(jiffies,
				abn_stamp + SYN_NO_ACK_REPORT_TIME)) {
				int net_type = interface_type % IPV6_INTERFACE;

				if (interface_type < IPV6_INTERFACE)
					rtn_abn[net_type].report_type =
						SYN_NO_ACK;
				else
					rtn_abn[net_type].report_type =
						SYN_NO_ACK_V6;
				interface_type %= IPV6_INTERFACE;
				rtn_abn[interface_type].uid = stream_list[hashcnt].uid;
				rtn_abn[interface_type].data_reg_tech =
					get_data_reg_type_chr_enum(data_reg_tech);
				spin_unlock_bh(&g_web_stat_lock);
				chr_notify_event(CHR_WEB_STAT_EVENT,
					g_user_space_pid, 0, rtn_abn);
				pr_info("chr: syn no ack report s:%x-d:%x>:%x\n",
				stream_list[hashcnt].src_addr & IPV4ADDR_MASK,
				stream_list[hashcnt].dst_addr & IPV4ADDR_MASK,
				stream_list[hashcnt].tcp_port);
				abnomal_stamp_list_syn_no_ack_print_log();
				spin_lock_bh(&g_web_stat_lock);
				memset(&rtn_abn, 0, sizeof(rtn_abn));
				abn_stamp_syn_no_ack =
					jiffies + FORBID_TIME;

			}
			stream_list[hashcnt].is_valid = IS_UNUSE;
		}

		hashNum++;
	}

	if (time_after(jiffies, rpt_stamp + REPORT_TIME)) {

		rpt_stamp = jiffies + REPORT_TIME;
		rtn_stat[RMNET_INTERFACE].report_type = WEB_STAT;
		rtn_stat[WLAN_INTERFACE].report_type = WEB_STAT;
		/* rtn stat 2 and 3 are reserved for ipv6 stats */
		rtn_stat[RMNET_INTERFACE + IPV6_INTERFACE].report_type =
			WEB_STAT_V6;
		rtn_stat[WLAN_INTERFACE + IPV6_INTERFACE].report_type =
			WEB_STAT_V6;
		rtn_stat[RMNET_INTERFACE + IPV6_INTERFACE].data_reg_tech =
			get_data_reg_type_chr_enum(data_reg_tech);
		spin_unlock_bh(&g_web_stat_lock);
#ifdef CONFIG_HW_NETBOOSTER_MODULE
		video_chr_stat_report();
#endif
		chr_notify_event(CHR_WEB_STAT_EVENT,
			g_user_space_pid, 0, rtn_stat);
		/* notify the IPv6 event */
		chr_notify_event(CHR_WEB_STAT_EVENT,
			g_user_space_pid, 0, &rtn_stat[IPV6_INTERFACE]);
		spin_lock_bh(&g_web_stat_lock);
		memset(&rtn_stat, 0, sizeof(rtn_stat));
		web_delay_rtt_flag_reset();
	}

	/*Check if there are timeout entries and remove them*/
	if (hashNum > 0) {
		sleep_flag = false;
		g_web_stat_timer.expires = jiffies + CHECK_TIME;
		spin_unlock_bh(&g_web_stat_lock);
		add_timer(&g_web_stat_timer);
		return;
	}
	sleep_flag = true;

	spin_unlock_bh(&g_web_stat_lock);
}

/*Computes the hash value of the network tcp stream*/
u8 hash3(u32 dst, u32 src, u32 port)
{
	u32 hash;
	hash = dst + src + port;
	hash = hash + hash/256 + hash/65536 + hash/16777216;
	hash = hash%HASH_MAX;
	return (u8)hash;
}

/* Computes the hash value of the network tcp stream for ipv6 packet*/
u8 hash3_v6(struct in6_addr dst, struct in6_addr src, u32 port)
{
	u32 hash;

	hash = dst.s6_addr32[0] + dst.s6_addr32[1] + dst.s6_addr32[2] +
		dst.s6_addr32[3] + src.s6_addr32[0] + src.s6_addr32[1] +
		src.s6_addr32[2] + src.s6_addr32[3] + port;
	hash = hash + hash/HASH_MAX + hash/HASH_MAX_16BIT +
		hash/HASH_MAX_24BIT;
	hash = hash % HASH_MAX;
	return (u8)hash;
}

/*Local_out packet processing*/
void out_proc(void)
{
	u8 hash_cnt;
	u32 http_get_delay = 0;
	u8 nwk_type = http_para_out.proto;
	u8 interface_type = http_para_out.interface + nwk_type * IPV6_INTERFACE;

	spin_lock_bh(&g_web_para_out_lock);

	if (http_para_out.is_valid == IS_USE) {

		if (http_para_out.proto == IPV6_NETWORK)
			hash_cnt = hash3_v6(http_para_out.dst_v6_addr,
						http_para_out.src_v6_addr,
						http_para_out.tcp_port);
		else
			hash_cnt = hash3(http_para_out.dst_addr,
						http_para_out.src_addr,
						http_para_out.tcp_port);

		if (stream_list[hash_cnt].is_valid == IS_UNUSE) {

			if (http_para_out.type == HTTP_SYN) {

				memcpy(&stream_list[hash_cnt],
					&http_para_out, sizeof(http_para_out));

			}

		} else if (stream_list[hash_cnt].type == HTTP_SYN &&
			http_para_out.type == HTTP_GET) {
			if (((http_para_out.proto != IPV6_NETWORK &&
				stream_list[hash_cnt].src_addr ==
					http_para_out.src_addr &&
				stream_list[hash_cnt].dst_addr ==
					http_para_out.dst_addr) ||
				(http_para_out.proto == IPV6_NETWORK &&
				ipv6_address_equal(
					stream_list[hash_cnt].src_v6_addr,
					http_para_out.src_v6_addr) &&
				ipv6_address_equal(
					stream_list[hash_cnt].dst_v6_addr,
					http_para_out.dst_v6_addr))) &&
				stream_list[hash_cnt].tcp_port ==
					http_para_out.tcp_port &&
				stream_list[hash_cnt].interface ==
					http_para_out.interface) {
				stream_list[hash_cnt].get_time_stamp =
					http_para_out.time_stamp;
				if(stream_list[hash_cnt].interface == http_para_out.interface) {
					if (http_para_out.time_stamp >= stream_list[hash_cnt].ack_time_stamp && 0 != stream_list[hash_cnt].ack_time_stamp) {
						http_get_delay = (http_para_out.time_stamp - stream_list[hash_cnt].ack_time_stamp) * MULTIPLE;
					} else if (http_para_out.time_stamp < stream_list[hash_cnt].ack_time_stamp && 0 != stream_list[hash_cnt].ack_time_stamp) {
						http_get_delay = (MAX_JIFFIES - stream_list[hash_cnt].ack_time_stamp +http_para_out.time_stamp) * MULTIPLE;
					}
					rtn_stat[interface_type].http_get_delay += http_get_delay;
					rtn_stat[interface_type].http_send_get_num++;
				}
				stream_list[hash_cnt].type = HTTP_GET;

			}
		}

		http_para_out.is_valid = IS_UNUSE;

		if (sleep_flag) {
			sleep_flag = false;
			g_web_stat_timer.expires = jiffies + CHECK_TIME;
			spin_unlock_bh(&g_web_para_out_lock);
			add_timer(&g_web_stat_timer);
			return;
		}
	}

	spin_unlock_bh(&g_web_para_out_lock);
}

void wifi_disconnect_report(void)
{
	pr_info("wifi_disconnect_report web_stat\n");
	spin_lock_bh(&g_web_stat_lock);
	rpt_stamp = jiffies + REPORT_TIME;
	rtn_stat[RMNET_INTERFACE].report_type = WEB_STAT;
	rtn_stat[WLAN_INTERFACE].report_type = WEB_STAT;
	rtn_stat[RMNET_INTERFACE + IPV6_INTERFACE].report_type = WEB_STAT_V6;
	rtn_stat[WLAN_INTERFACE + IPV6_INTERFACE].report_type = WEB_STAT_V6;
	spin_unlock_bh(&g_web_stat_lock);
	chr_notify_event(CHR_WEB_STAT_EVENT,
		g_user_space_pid, 0, rtn_stat);
	chr_notify_event(CHR_WEB_STAT_EVENT,
		g_user_space_pid, 0, &rtn_stat[IPV6_INTERFACE]);
	spin_lock_bh(&g_web_stat_lock);
	memset(&rtn_stat, 0, sizeof(rtn_stat));
	web_delay_rtt_flag_reset();
	spin_unlock_bh(&g_web_stat_lock);
}

/*report abnormal event for kernel delay statistic*/
void wifi_kernel_delay_report(DELAY_CHR_REPROT_T *p_delay_chr)
{
	struct http_return rtn_stat_wifi[RNT_STAT_SIZE];
	pr_info("wifi_kernel_delay_report \n");
	memset(&rtn_stat_wifi, 0, sizeof(rtn_stat_wifi));
	if (NULL == p_delay_chr) {
            return;
	}
	rtn_stat_wifi[WLAN_INTERFACE].report_type = WEB_STAT;
	rtn_stat_wifi[WLAN_INTERFACE].exception_cnt = p_delay_chr->exception_cnt;
	rtn_stat_wifi[WLAN_INTERFACE].data_direct = p_delay_chr->data_direct;
	rtn_stat_wifi[WLAN_INTERFACE].transport_delay = p_delay_chr->transport_delay;
	rtn_stat_wifi[WLAN_INTERFACE].ip_delay = p_delay_chr->ip_delay;
	rtn_stat_wifi[WLAN_INTERFACE].hmac_delay = p_delay_chr->hmac_delay;
	rtn_stat_wifi[WLAN_INTERFACE].driver_delay = p_delay_chr->driver_delay;
	rtn_stat_wifi[WLAN_INTERFACE].android_uid = p_delay_chr->android_uid;
	chr_notify_event(CHR_WEB_STAT_EVENT,
		g_user_space_pid, 0, rtn_stat_wifi);
}

/*Local_in packet processing*/
void in_proc(void)
{
	u8 hash_cnt;
	u32 web_delay;
	u32 handshake_delay;
	unsigned long jiffies_tmp;
	unsigned long abn_stamp;
	u8 nwk_type = http_para_in.proto;
	u8 interface_type = http_para_in.interface + nwk_type * IPV6_INTERFACE;

	jiffies_tmp = jiffies;

	spin_lock_bh(&g_web_para_in_lock);
	if (http_para_in.is_valid == IS_UNUSE) {
		spin_unlock_bh(&g_web_para_in_lock);
		return;
	}

	if (http_para_in.proto == IPV6_NETWORK)
		hash_cnt = hash3_v6(http_para_in.dst_v6_addr,
			http_para_in.src_v6_addr, http_para_in.tcp_port);
	else
		hash_cnt = hash3(http_para_in.dst_addr,
			http_para_in.src_addr, http_para_in.tcp_port);

	if (stream_list[hash_cnt].is_valid == IS_UNUSE ||
			(stream_list[hash_cnt].type != HTTP_GET &&
			stream_list[hash_cnt].type != HTTP_SYN)) {

		http_para_in.is_valid = IS_UNUSE;
		spin_unlock_bh(&g_web_para_in_lock);
		return;

	}

	/*In all three cases, the tcp stream is removed from the table.
	Http get to http response time is too long,
	that is, no response to the page.
	Visit the web page successfully. Failed to access webpage*/
	if (((stream_list[hash_cnt].proto != IPV6_NETWORK &&
		stream_list[hash_cnt].src_addr == http_para_in.src_addr &&
		stream_list[hash_cnt].dst_addr == http_para_in.dst_addr) ||
		(stream_list[hash_cnt].proto == IPV6_NETWORK &&
		ipv6_address_equal(stream_list[hash_cnt].src_v6_addr,
			http_para_in.src_v6_addr) &&
		ipv6_address_equal(stream_list[hash_cnt].dst_v6_addr,
			http_para_in.dst_v6_addr))) &&
		stream_list[hash_cnt].tcp_port == http_para_in.tcp_port &&
		stream_list[hash_cnt].interface == http_para_in.interface) {
		switch (http_para_in.type) {
		case WEB_SUCC:
			rtn_stat[interface_type].total_num++;
			rtn_stat[interface_type].succ_num++;

			if (http_para_in.time_stamp >= stream_list[hash_cnt].time_stamp) {
				web_delay = (http_para_in.time_stamp - stream_list[hash_cnt].time_stamp) * MULTIPLE;
			} else {
				web_delay = (MAX_JIFFIES - stream_list[hash_cnt].time_stamp + http_para_in.time_stamp) * MULTIPLE;
			}
			rtn_stat[interface_type].web_delay += web_delay;

			if (web_deley_flag[interface_type])
			{
				rtn_stat[interface_type].highest_web_delay= web_delay;
				rtn_stat[interface_type].lowest_web_delay= web_delay;
				rtn_stat[interface_type].last_web_delay= web_delay;
				web_deley_flag[interface_type] = false;
			}
			/*recording the web_delays value*/
			if (web_delay > rtn_stat[interface_type].highest_web_delay)
				rtn_stat[interface_type].highest_web_delay = web_delay;
			if (web_delay< rtn_stat[interface_type].lowest_web_delay)
				rtn_stat[interface_type].lowest_web_delay = web_delay;
			rtn_stat[interface_type].last_web_delay = web_delay;
			if (web_delay > DELAY_THRESHOLD_L1 &&
					web_delay <= DELAY_THRESHOLD_L2)
				rtn_stat[interface_type].delay_num_L1++;

			else if (web_delay > DELAY_THRESHOLD_L2 &&
					web_delay <= DELAY_THRESHOLD_L3)
				rtn_stat[interface_type].delay_num_L2++;

			else if (web_delay > DELAY_THRESHOLD_L3 &&
					web_delay <= DELAY_THRESHOLD_L4)
				rtn_stat[interface_type].delay_num_L3++;

			else if (web_delay > DELAY_THRESHOLD_L4 &&
					web_delay <= DELAY_THRESHOLD_L5)
				rtn_stat[interface_type].delay_num_L4++;

			else if (web_delay > DELAY_THRESHOLD_L5 &&
					web_delay <= DELAY_THRESHOLD_L6)
				rtn_stat[interface_type].delay_num_L5++;

			else if (web_delay > DELAY_THRESHOLD_L6)
				rtn_stat[interface_type].delay_num_L6++;

			save_app_web_delay(stream_list[hash_cnt].uid,
				web_delay,interface_type);

			abn_stamp =
			abnomal_stamp_list_web_delay_update(jiffies_tmp);
			if (time_after(jiffies_tmp, abn_stamp_web_delay) &&
				web_delay > WEB_DELAY_THRESHOLD &&
				time_before(jiffies_tmp, abn_stamp +
				WEB_DELAY_REPORT_TIME)) {
				int net_type = interface_type % IPV6_INTERFACE;

				if (interface_type < IPV6_INTERFACE) {
					rtn_abn[net_type].report_type =
						WEB_DELAY;
					rtn_abn[net_type].server_addr =
						stream_list[hash_cnt].dst_addr;
				} else {
					rtn_abn[net_type].report_type =
						WEB_DELAY_V6;
					rtn_abn[net_type].server_v6_addr =
						stream_list[hash_cnt].dst_v6_addr;
				}
				rtn_abn[net_type].web_delay = web_delay;
				rtn_abn[net_type].uid =
					stream_list[hash_cnt].uid;
				spin_unlock_bh(&g_web_para_in_lock);
				spin_unlock_bh(&g_web_stat_lock);
				chr_notify_event(CHR_WEB_STAT_EVENT,
					g_user_space_pid, 0, rtn_abn);
				pr_info("chr: web delay report s:%x-d:%x>:%x\n",
				stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
				stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
				stream_list[hash_cnt].tcp_port);
				spin_lock_bh(&g_web_stat_lock);
				memset(&rtn_abn, 0, sizeof(rtn_abn));
				spin_lock_bh(&g_web_para_in_lock);
				abn_stamp_web_delay = jiffies_tmp + FORBID_TIME;
			}
			stream_list[hash_cnt].is_valid = IS_UNUSE;
			break;

		case WEB_FAIL:
			rtn_stat[interface_type].total_num++;
			rtn_stat[interface_type].fail_num++;
			save_app_web_fail(stream_list[hash_cnt].uid, interface_type);
			abn_stamp =
			abnomal_stamp_list_web_fail_update(jiffies_tmp);
			if (time_after(jiffies_tmp, abn_stamp_web_fail) &&
				time_before(jiffies_tmp, abn_stamp +
				WEB_FAIL_REPORT_TIME)) {
				int net_type = interface_type % IPV6_INTERFACE;

				if (interface_type < IPV6_INTERFACE) {
					rtn_abn[net_type].report_type =
						WEB_FAIL;
					rtn_abn[net_type].server_addr =
						stream_list[hash_cnt].dst_addr;
				} else {
					rtn_abn[net_type].report_type =
					WEB_FAIL_V6;
					rtn_abn[net_type].server_v6_addr =
					stream_list[hash_cnt].dst_v6_addr;
				}
				rtn_abn[net_type].uid =
					stream_list[hash_cnt].uid;
				rtn_abn[net_type].http_resp =
					http_para_in.resp_code;
				spin_unlock_bh(&g_web_para_in_lock);
				spin_unlock_bh(&g_web_stat_lock);
				chr_notify_event(CHR_WEB_STAT_EVENT,
					g_user_space_pid, 0, rtn_abn);
				pr_info("chr: web fail report s:%x-d:%x>:%d\n",
				stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
				stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
				stream_list[hash_cnt].tcp_port);
				spin_lock_bh(&g_web_stat_lock);
				memset(&rtn_abn, 0, sizeof(rtn_abn));
				spin_lock_bh(&g_web_para_in_lock);
				abn_stamp_web_fail = jiffies_tmp + FORBID_TIME;

			}
			stream_list[hash_cnt].is_valid = IS_UNUSE;
			break;

		case SYN_SUCC:
			rtn_stat[interface_type].tcp_succ_num++;
			if (http_para_in.time_stamp >= stream_list[hash_cnt].time_stamp) {
				handshake_delay = (http_para_in.time_stamp - stream_list[hash_cnt].time_stamp) * MULTIPLE;
			} else {
				handshake_delay = (MAX_JIFFIES - stream_list[hash_cnt].time_stamp + http_para_in.time_stamp) * MULTIPLE;
			}
			rtn_stat[interface_type].tcp_handshake_delay += handshake_delay;
			stream_list[hash_cnt].ack_time_stamp = http_para_in.time_stamp;
			save_app_syn_succ(stream_list[hash_cnt].uid, interface_type);
			break;

		default:
			stream_list[hash_cnt].is_valid = IS_UNUSE;
			break;

		}

	} else if (stream_list[hash_cnt].type == HTTP_GET &&
		time_after(jiffies_tmp, stream_list[hash_cnt].get_time_stamp +
		EXPIRE_TIME)) {

		rtn_stat[interface_type].total_num++;
		rtn_stat[interface_type].no_ack_num++;

		save_app_web_no_ack(stream_list[hash_cnt].uid, interface_type);

		abnomal_stamp_list_web_no_ack_update(jiffies_tmp);
		if (time_after(jiffies_tmp, abn_stamp_no_ack) &&
			time_before(jiffies_tmp, abn_stamp_list_web_no_ack[0] +
				WEB_NO_ACK_REPORT_TIME)) {
			int net_type = interface_type % IPV6_INTERFACE;

			if (interface_type < IPV6_INTERFACE) {
				rtn_abn[net_type].report_type = WEB_NO_ACK;
				rtn_abn[net_type].server_addr =
					stream_list[hash_cnt].dst_addr;
			} else {
				rtn_abn[net_type].report_type = WEB_NO_ACK_V6;
				rtn_abn[net_type].server_v6_addr =
					stream_list[hash_cnt].dst_v6_addr;
			}
			rtn_abn[net_type].uid = stream_list[hash_cnt].uid;
			rtn_abn[net_type].http_resp = 0xffffffff;
			spin_unlock_bh(&g_web_para_in_lock);
			spin_unlock_bh(&g_web_stat_lock);
			chr_notify_event(CHR_WEB_STAT_EVENT,
				g_user_space_pid, 0, rtn_abn);
			pr_info("chr: no ack report s:%x-d:%x>:%x\n",
			stream_list[hash_cnt].src_addr & IPV4ADDR_MASK,
			stream_list[hash_cnt].dst_addr & IPV4ADDR_MASK,
			stream_list[hash_cnt].tcp_port);
			spin_lock_bh(&g_web_stat_lock);
			memset(&rtn_abn, 0, sizeof(rtn_abn));
			spin_lock_bh(&g_web_para_in_lock);
			abn_stamp_no_ack = jiffies_tmp + FORBID_TIME;
		}

		stream_list[hash_cnt].is_valid = IS_UNUSE;
	}

	http_para_in.is_valid = IS_UNUSE;

	if (sleep_flag) {
		sleep_flag = false;
		g_web_stat_timer.expires = jiffies + CHECK_TIME;
		spin_unlock_bh(&g_web_para_in_lock);
		add_timer(&g_web_stat_timer);
		return;
	}

	spin_unlock_bh(&g_web_para_in_lock);
}

void rtt_proc(void)
{
	unsigned long abn_stamp;
	int idx = 0;
	for (idx = 0; idx < RNT_STAT_SIZE; idx++) {
		if (stack_rtt[idx].is_valid == IS_USE) {

			rtn_stat[idx].tcp_total_num++;
			rtn_stat[idx].tcp_rtt += stack_rtt[idx].tcp_rtt;

			if (stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L1 &&
					stack_rtt[idx].tcp_rtt <= RTT_THRESHOLD_L2)
				rtn_stat[idx].rtt_num_L1++;

			else if (stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L2 &&
					stack_rtt[idx].tcp_rtt <= RTT_THRESHOLD_L3)
				rtn_stat[idx].rtt_num_L2++;

			else if (stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L3 &&
					stack_rtt[idx].tcp_rtt <= RTT_THRESHOLD_L4)
				rtn_stat[idx].rtt_num_L3++;

			else if (stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L4 &&
					stack_rtt[idx].tcp_rtt <= RTT_THRESHOLD_L5)
				rtn_stat[idx].rtt_num_L4++;

			else if (stack_rtt[idx].tcp_rtt > RTT_THRESHOLD_L5)
				rtn_stat[idx].rtt_num_L5++;

			save_app_tcp_rtt(stack_rtt[idx].uid, stack_rtt[idx].tcp_rtt, idx);

			if (rtt_flag[idx]){
				rtn_stat[idx].highest_tcp_rtt = stack_rtt[idx].tcp_rtt;
				rtn_stat[idx].lowest_tcp_rtt = stack_rtt[idx].tcp_rtt;
				rtn_stat[idx].last_tcp_rtt = stack_rtt[idx].tcp_rtt;
				rtt_flag[idx] = false;
			}
			if (stack_rtt[idx].tcp_rtt > rtn_stat[idx].highest_tcp_rtt)
				rtn_stat[idx].highest_tcp_rtt = stack_rtt[idx].tcp_rtt;
			if (stack_rtt[idx].tcp_rtt < rtn_stat[idx].lowest_tcp_rtt)
				rtn_stat[idx].lowest_tcp_rtt = stack_rtt[idx].tcp_rtt;
			rtn_stat[idx].last_tcp_rtt = stack_rtt[idx].tcp_rtt;

			abn_stamp = abnomal_stamp_list_tcp_rtt_large_update(jiffies);
			if (stack_rtt[idx].tcp_rtt > RTT_THRESHOLD &&
				time_after(jiffies, abn_stamp_rtt_large) &&
				time_before(jiffies, abn_stamp +
				TCP_RTT_LARGE_REPORT_TIME)) {

				abn_stamp_rtt_large = jiffies + FORBID_TIME;
				if (idx < IPV6_INTERFACE) {
					rtn_abn[idx % 2].report_type =
						TCP_RTT_LARGE;
					rtn_abn[idx % 2].rtt_abn_server_addr =
						stack_rtt[idx].rtt_dst_addr;
				} else {
					rtn_abn[idx % 2].report_type =
						TCP_RTT_LARGE_V6;
					rtn_abn[idx % 2].rtt_abn_srv_v6_addr =
						stack_rtt[idx].rtt_dst_v6_addr;
				}
				rtn_abn[idx % 2].tcp_rtt =
					 stack_rtt[idx].tcp_rtt;
				rtn_abn[idx % 2].uid = stack_rtt[idx].uid;
				spin_unlock_bh(&g_web_stat_lock);
				chr_notify_event(CHR_WEB_STAT_EVENT,
					g_user_space_pid, 0, rtn_abn);
				pr_info("chr: rtt large report\n");
				spin_lock_bh(&g_web_stat_lock);
				memset(&rtn_abn, 0, sizeof(rtn_abn));
			}
			stack_rtt[idx].is_valid = IS_UNUSE;

			if (sleep_flag) {
				sleep_flag = false;
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
	bool is_need_notify = false;

	is_need_notify = is_notify_chr_event(old_data_reg_tech, data_reg_tech);
	if (!is_need_notify)
		return;

	rtn_stat[RMNET_INTERFACE].data_reg_tech =
		get_data_reg_type_chr_enum(old_data_reg_tech);
	rtn_stat[RMNET_INTERFACE].report_type = WEB_STAT;
	rtn_stat[WLAN_INTERFACE].report_type = WEB_STAT;
	spin_unlock_bh(&g_web_stat_lock);
	if (time_after(jiffies, abn_stamp_rat_tech_change)) {
		chr_notify_event(CHR_WEB_STAT_EVENT,
			g_user_space_pid, 0, rtn_stat);
		abn_stamp_rat_tech_change = jiffies + RAT_TECH_CHANGE_NOTIFY_FORBIDDEN_TIME;
	}
	spin_lock_bh(&g_web_stat_lock);
	old_data_reg_tech = data_reg_tech;
	memset(&rtn_stat, 0, sizeof(rtn_stat));
}

/*This is the main thread of web statistics*/
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

/*Calculates the return code for http*/
u8 http_response_type(char *pstr)
{
	u8 type;

	type = UN_KNOW;

	if (pstr[HTTP_ACK_FROM_START] == '2' ||
			pstr[HTTP_ACK_FROM_START] == '3') {
		type = WEB_SUCC;
	}

	if (pstr[HTTP_ACK_FROM_START] == '4' ||
			pstr[HTTP_ACK_FROM_START] == '5') {
		type = WEB_FAIL;
	}
	return type;
}

u32 http_response_code(char *pstr)
{
	u32 code = 0;
	int idx;
	char ch;

	if (pstr == NULL)
		return 0;

	for (idx = 0; idx < 3; idx++) {
		ch = pstr[(int)(HTTP_ACK_FROM_START + idx)];
		if ('0' <= ch && ch <= '9')
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

	chr_notify_event(CHR_WEB_STAT_EVENT,
		g_user_space_pid, 0, rtn_stat_sock);

	if (time_after(jiffies, g_rpt_sock_stamp))
		g_rpt_sock_stamp = jiffies + g_tcp_min_report_time;
}

void tcp_win_monitor(struct sock *sk, struct tcphdr *th,
	char *pHttpStr, int dlen)
{
	unsigned int cur_win = 0;

	if ((sk == NULL) || (th == NULL) ||
		(pHttpStr == NULL) || (get_uid_from_sock(sk) == 0))
		return;

	/* RFC1323 scaling reverse applied */
	cur_win = (th->window) << (tcp_sk(sk)->rx_opt.rcv_wscale);

	if ((cur_win > 0) && (cur_win <= g_tcp_small_window)) {
		if (sk->win_cnt == 0)
			sk->small_win_stamp = jiffies +
				SMALL_WIN_STAMP_RATIO * g_tcp_min_report_time;

		++sk->win_cnt;
		if ((sk->win_cnt > g_tcp_small_win_cnt) && (sk->win_flag) &&
			(tcp_need_trigger_upload(sk->small_win_stamp))) {
			sk->win_flag = false;
			tcp_sock_win_report(th, sk);
		}
	} else {
		if ((sk->win_cnt > g_tcp_small_win_cnt) &&
			(tcp_need_trigger_upload(sk->small_win_stamp)))
			tcp_sock_win_report(th, sk);

		sk->win_flag = true;
		sk->win_cnt = 0;
	}
}

#endif

bool is_valid_data_reg_tech(void)
{
	switch (data_reg_tech) {
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

/*Local out hook function*/
static unsigned int hook_local_out(void *ops, struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;
	struct tcp_sock *sock = NULL;
	char *pHttpStr = NULL;
	bool up_req = false;
	int dlen;
	bool is_ipv6_pkt = false;

	if (skb == NULL)
		return NF_ACCEPT;

	/* identify the network protocol of an IP packet */
	if (ntohs(skb->protocol) == ETH_P_IPV6) {
		ip6h = ipv6_hdr(skb);
		if (ip6h == NULL || ip6h->nexthdr != IPPROTO_TCP)
			return NF_ACCEPT;
		is_ipv6_pkt = true;
	} else {
		iph = ip_hdr(skb);
		if (iph == NULL || iph->protocol != IPPROTO_TCP)
			return NF_ACCEPT;
	}

	tcph = tcp_hdr(skb);
	if ((tcph == NULL) || (skb->data == NULL) || (tcph->doff == 0))
		return NF_ACCEPT;

	pHttpStr = (char *)((u32 *)tcph + tcph->doff);

	if (is_ipv6_pkt == true)
		dlen = skb->len - (pHttpStr - (char *)ip6h);
	else
		dlen = skb->len - (pHttpStr - (char *)iph);

	if (dlen < 0)
		return NF_ACCEPT;

	if ((skb->dev == NULL) || (skb->dev->name == NULL))
		return NF_ACCEPT;

	if (skb->sk == NULL)
		return NF_ACCEPT;

	if (strncmp(skb->dev->name, WEB_DS_NET, WEB_DS_NET_LEN)) {
		http_para_out.interface = WLAN_INTERFACE;
	} else {
			if (!is_valid_data_reg_tech())
				return NF_ACCEPT;
		http_para_out.interface = RMNET_INTERFACE;
		if (skb->sk->sk_state == TCP_ESTABLISHED) {
			sock = tcp_sk(skb->sk);
			sock->data_net_flag = true;
		}
	}

	if (htons(tcph->dest) != HTTP_PORT)
		return NF_ACCEPT;

	/*When the lock is not locked, the lock is triggered*/
	if (!spin_trylock_bh(&g_web_para_out_lock))
		return NF_ACCEPT;

	if (http_para_out.is_valid == IS_UNUSE) {

		/*This is an http ack syn packet processing*/
		if (tcph->syn == 1 && tcph->ack == 0) {
			http_para_out.tcp_port = tcph->source;
			if (is_ipv6_pkt == true) {
				http_para_out.proto = IPV6_NETWORK;
				http_para_out.src_v6_addr = ip6h->saddr;
				http_para_out.dst_v6_addr = ip6h->daddr;
			} else {
				http_para_out.proto = IPV4_NETWORK;
				http_para_out.src_addr = iph->saddr;
				http_para_out.dst_addr = iph->daddr;
			}
			http_para_out.type = HTTP_SYN;
			http_para_out.time_stamp = jiffies;
			http_para_out.is_valid = IS_USE;
			http_para_out.uid = get_uid_from_sock(skb->sk);
			up_req = true;

		} else if (dlen > WEB_DS_NET_LEN &&
			(strncmp(pHttpStr, g_get_str, STR_GET_LEN) == 0 ||
			strncmp(pHttpStr, g_post_str, STR_POST_LEN) == 0)) {
#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
			skb->sk->win_cnt = 0;
#endif
			http_para_out.tcp_port = tcph->source;
			if (is_ipv6_pkt == true) {
				http_para_out.proto = IPV6_NETWORK;
				http_para_out.src_v6_addr = ip6h->saddr;
				http_para_out.dst_v6_addr = ip6h->daddr;
			} else {
				http_para_out.proto = IPV4_NETWORK;
				http_para_out.src_addr = iph->saddr;
				http_para_out.dst_addr = iph->daddr;
			}
			http_para_out.type = HTTP_GET;
			http_para_out.time_stamp = jiffies;
			http_para_out.is_valid = IS_USE;
			up_req = true;
		}
	}

	spin_unlock_bh(&g_web_para_out_lock);

	if (up_req)
		up(&g_web_stat_sync_sema);

#ifdef CONFIG_HW_CHR_TCP_SMALL_WIN_MONITOR
	tcp_win_monitor(skb->sk, tcph, pHttpStr, dlen);
#endif

	return NF_ACCEPT;
}

/*Local in hook function*/
static unsigned int hook_local_in(void *ops, struct sk_buff *skb,
		const struct nf_hook_state *state)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	struct tcphdr *tcph = NULL;
	char *pHttpStr = NULL;
	bool up_req = false;
	u32 dlen;
	bool is_ipv6_pkt = false;

	if (skb == NULL)
		return NF_ACCEPT;

	/* identify the network protocol of an IP packet */
	if (ntohs(skb->protocol) == ETH_P_IPV6) {
		ip6h = ipv6_hdr(skb);
		if (ip6h == NULL || ip6h->nexthdr != IPPROTO_TCP)
			return NF_ACCEPT;
		is_ipv6_pkt = true;
	} else {
		iph = ip_hdr(skb);
		if (iph == NULL || iph->protocol != IPPROTO_TCP)
			return NF_ACCEPT;
	}
	tcph = tcp_hdr(skb);
	if (tcph == NULL || skb->data == NULL || tcph->doff == 0)
		return NF_ACCEPT;

	pHttpStr = (char *)((u32 *)tcph + tcph->doff);

	if (is_ipv6_pkt == true)
		dlen = skb->len - (pHttpStr - (char *)ip6h);
	else
		dlen = skb->len - (pHttpStr - (char *)iph);

	if (skb->dev == NULL || skb->dev->name == NULL)
		return NF_ACCEPT;

	if (strncmp(skb->dev->name, WEB_DS_NET, WEB_DS_NET_LEN)) {
		http_para_in.interface = WLAN_INTERFACE;
	} else {
			if (!is_valid_data_reg_tech())
				return NF_ACCEPT;
		http_para_in.interface = RMNET_INTERFACE;
	}

	if (htons(tcph->source) != HTTP_PORT)
		return NF_ACCEPT;

	if (!spin_trylock_bh(&g_web_para_in_lock))
		return NF_ACCEPT;

	/* Determine whether the received packet is an HTTP response */
	if (dlen > HTTP_RSP_LEN &&
		strncmp(pHttpStr, g_http_str, STR_HTTP_LEN) == 0 &&
		http_response_type(pHttpStr) != UN_KNOW &&
		http_para_in.is_valid == IS_UNUSE) {
		http_para_in.tcp_port = tcph->dest;
		if (is_ipv6_pkt == true) {
			http_para_in.proto = IPV6_NETWORK;
			http_para_in.src_v6_addr = ip6h->daddr;
			http_para_in.dst_v6_addr = ip6h->saddr;
		} else {
			http_para_in.proto = IPV4_NETWORK;
			http_para_in.src_addr = iph->daddr;
			http_para_in.dst_addr = iph->saddr;
		}
		http_para_in.time_stamp = jiffies;
		http_para_in.is_valid = IS_USE;
		http_para_in.type = http_response_type(pHttpStr);
		http_para_in.resp_code = http_response_code(pHttpStr);
		up_req = true;
	}
	/* Determine whether the received packet is an SYN ACK */
	else if (http_para_in.is_valid == IS_UNUSE &&
		tcph->syn == 1 && tcph->ack == 1) {
		http_para_in.tcp_port = tcph->dest;
		if (is_ipv6_pkt == true) {
			http_para_in.proto = IPV6_NETWORK;
			http_para_in.src_v6_addr = ip6h->daddr;
			http_para_in.dst_v6_addr = ip6h->saddr;
		} else {
			http_para_in.proto = IPV4_NETWORK;
			http_para_in.src_addr = iph->daddr;
			http_para_in.dst_addr = iph->saddr;
		}
		http_para_in.time_stamp = jiffies;
		http_para_in.is_valid = IS_USE;
		http_para_in.type = SYN_SUCC;
		up_req = true;
	}

	spin_unlock_bh(&g_web_para_in_lock);

	if (up_req)
		up(&g_web_stat_sync_sema);

	return NF_ACCEPT;
}

static struct nf_hook_ops net_hooks[] = {
	{
		.hook		= hook_local_in,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
		.owner		= THIS_MODULE,
#endif
		.pf			= PF_INET,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP_PRI_FILTER - 1,
	},
	{
		.hook		= hook_local_out,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
		.owner		= THIS_MODULE,
#endif
		.pf			= PF_INET,
		.hooknum	= NF_INET_POST_ROUTING,
		.priority	= NF_IP_PRI_FILTER - 1,
	},
	{
		.hook		= hook_local_in,
		.pf		= PF_INET6,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP_PRI_FILTER - 1,
	},
	{
		.hook		= hook_local_out,
		.pf		= PF_INET6,
		.hooknum	= NF_INET_POST_ROUTING,
		.priority	= NF_IP_PRI_FILTER - 1,
	}
};
static void web_delay_rtt_flag_reset(void)
{
	int flag_index;
	for (flag_index = 0; flag_index < RNT_STAT_SIZE; flag_index++) {
		rtt_flag[flag_index] = true;
		web_deley_flag[flag_index] = true;
	}
}
/*CHR Initialization function*/
int web_chr_init(void)
{
	int ret = 0;

       /*Initializes the array*/
	stream_list = kmalloc(sizeof(struct http_stream)*HASH_MAX, GFP_KERNEL);
	if (stream_list == NULL)
		return -1;

	memset(stream_list, 0, sizeof(struct http_stream)*HASH_MAX);

	/*Variable initialization*/
	memset(&rtn_stat, 0, sizeof(rtn_stat));
	memset(&rtn_abn, 0, sizeof(rtn_abn));
	memset(&http_para_in, 0, sizeof(http_para_in));
	memset(&http_para_out, 0, sizeof(http_para_out));
	memset(&stack_rtt, 0, sizeof(stack_rtt));

	/*spin lock initialization*/
	spin_lock_init(&g_web_stat_lock);
	spin_lock_init(&g_web_para_in_lock);
	spin_lock_init(&g_web_para_out_lock);
	sema_init(&g_web_stat_sync_sema, 0);
	/*flag initialization*/
	web_delay_rtt_flag_reset();
	/*Timestamp initialization*/
	abn_stamp_no_ack = jiffies;
	abn_stamp_rtt_large = jiffies;
	abn_stamp_web_fail = jiffies;
	abn_stamp_web_delay = jiffies;
	abn_stamp_syn_no_ack = jiffies;
	abn_stamp_rat_tech_change = jiffies;
	g_chr_key_val.tcp_last = jiffies;
	g_chr_key_val.udp_last = jiffies;
	atomic_set(&g_chr_key_val.tcp_buf_time, 0);
	atomic_set(&g_chr_key_val.udp_buf_time, 0);

	rpt_stamp = jiffies;

	abn_stamp_list_syn_no_ack =
		kmalloc(sizeof(unsigned long)*SYN_NO_ACK_MAX, GFP_KERNEL);
	if (abn_stamp_list_syn_no_ack == NULL)
		goto error;
	abn_stamp_list_web_no_ack =
		kmalloc(sizeof(unsigned long)*WEB_NO_ACK_MAX, GFP_KERNEL);
	if (abn_stamp_list_web_no_ack == NULL)
		goto error;
	abn_stamp_list_web_fail =
		kmalloc(sizeof(unsigned long)*WEB_FAIL_MAX, GFP_KERNEL);
	if (abn_stamp_list_web_fail == NULL)
		goto error;
	abn_stamp_list_web_delay =
		kmalloc(sizeof(unsigned long)*WEB_DELAY_MAX, GFP_KERNEL);
	if (abn_stamp_list_web_delay == NULL)
		goto error;
	abn_stamp_list_tcp_rtt_large =
		kmalloc(sizeof(unsigned long)*TCP_RTT_LARGE_MAX, GFP_KERNEL);
	if (abn_stamp_list_tcp_rtt_large == NULL)
		goto error;

	memset(abn_stamp_list_syn_no_ack, 0,
		sizeof(unsigned long)*SYN_NO_ACK_MAX);
	memset(abn_stamp_list_web_no_ack, 0,
		sizeof(unsigned long)*WEB_NO_ACK_MAX);
	memset(abn_stamp_list_web_fail, 0,
		sizeof(unsigned long)*WEB_FAIL_MAX);
	memset(abn_stamp_list_web_delay, 0,
		sizeof(unsigned long)*WEB_DELAY_MAX);
	memset(abn_stamp_list_tcp_rtt_large, 0,
		sizeof(unsigned long)*TCP_RTT_LARGE_MAX);

	abn_stamp_list_syn_no_ack_idx = 0;
	abn_stamp_list_web_no_ack_idx = 0;
	abn_stamp_list_web_fail_idx = 0;
	abn_stamp_list_web_delay_idx = 0;
	abn_stamp_list_tcp_rtt_large_idx = 0;

	/*Create a thread*/
	g_web_stat_task = kthread_run(chr_web_thread, NULL, "chr_web_thread");

	/*Timer initialization*/
	init_timer(&g_web_stat_timer);
	g_web_stat_timer.data = 0;
	g_web_stat_timer.function = web_stat_timer;
	g_web_stat_timer.expires = jiffies + CHECK_TIME;
	add_timer(&g_web_stat_timer);
	sleep_flag = false;

       /*Registration hook function*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	ret = nf_register_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	ret = nf_register_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
#endif
	if (ret) {
		pr_info("chr:nf_init_in ret=%d  ", ret);
		return -1;
	}
	pr_info("chr:web stat init success\n");

	return 0;
error:
	if (stream_list != NULL)
		kfree(stream_list);
	if (abn_stamp_list_syn_no_ack != NULL)
		kfree(abn_stamp_list_syn_no_ack);
	if (abn_stamp_list_web_no_ack != NULL)
		kfree(abn_stamp_list_web_no_ack);
	if (abn_stamp_list_web_fail != NULL)
		kfree(abn_stamp_list_web_fail);
	if (abn_stamp_list_web_delay != NULL)
		kfree(abn_stamp_list_web_delay);
	if (abn_stamp_list_tcp_rtt_large != NULL)
		kfree(abn_stamp_list_tcp_rtt_large);
	pr_info("chr:web stat init fail");
	return -1;
}

void web_chr_exit(void)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	nf_unregister_hooks(net_hooks, ARRAY_SIZE(net_hooks));
#else
	nf_unregister_net_hooks(&init_net, net_hooks, ARRAY_SIZE(net_hooks));
#endif
	kfree(stream_list);
	kfree(abn_stamp_list_syn_no_ack);
	kfree(abn_stamp_list_web_no_ack);
	kfree(abn_stamp_list_web_fail);
	kfree(abn_stamp_list_web_delay);
	kfree(abn_stamp_list_tcp_rtt_large);
	pr_info("chr:web stat exit success\n");
}

uid_t get_uid_from_sock(struct sock *sk)
{
	const struct file *filp = NULL;

	if (NULL == sk || NULL == sk->sk_socket)
		return 0;

	filp = sk->sk_socket->file;
	if (NULL == filp || NULL == filp->f_cred)
		return 0;

	return from_kuid(&init_user_ns, filp->f_cred->fsuid);
}

u32 get_des_addr_from_sock(struct sock *sk)
{
	if (NULL == sk)
		return 0;
	return sk->sk_daddr;
}
/* Append time_stamp to the end of the abn_stamp_list */
unsigned long abnomal_stamp_list_syn_no_ack_update(
unsigned long time_stamp)
{
	abn_stamp_list_syn_no_ack[abn_stamp_list_syn_no_ack_idx] = time_stamp;
	abn_stamp_list_syn_no_ack_idx =
		(abn_stamp_list_syn_no_ack_idx + 1) % SYN_NO_ACK_MAX;
	return abn_stamp_list_syn_no_ack[abn_stamp_list_syn_no_ack_idx];
}

void abnomal_stamp_list_syn_no_ack_print_log(void)
{
	int idx;

	for (idx = 0; idx < SYN_NO_ACK_MAX; idx++) {
		pr_info("chr:abn_stamp_list_syn_no_ack[%d]=%ld\n", idx,
			abn_stamp_list_syn_no_ack[idx]);
	}
	pr_info("chr:abn_stamp_list_syn_no_ack_idx=%d\n",
		abn_stamp_list_syn_no_ack_idx);
}

unsigned long abnomal_stamp_list_web_no_ack_update(
	unsigned long time_stamp)
{
	abn_stamp_list_web_no_ack[abn_stamp_list_web_no_ack_idx] = time_stamp;
	abn_stamp_list_web_no_ack_idx =
		(abn_stamp_list_web_no_ack_idx + 1) % WEB_NO_ACK_MAX;
	return abn_stamp_list_web_no_ack[abn_stamp_list_web_no_ack_idx];
}

unsigned long abnomal_stamp_list_web_fail_update(
	unsigned long time_stamp)
{
	abn_stamp_list_web_fail[abn_stamp_list_web_fail_idx] = time_stamp;
	abn_stamp_list_web_fail_idx =
		(abn_stamp_list_web_fail_idx + 1) % WEB_FAIL_MAX;
	return abn_stamp_list_web_fail[abn_stamp_list_web_fail_idx];
}

unsigned long abnomal_stamp_list_web_delay_update(
	unsigned long time_stamp)
{
	abn_stamp_list_web_delay[abn_stamp_list_web_delay_idx] = time_stamp;
	abn_stamp_list_web_delay_idx =
		(abn_stamp_list_web_delay_idx + 1) % WEB_DELAY_MAX;
	return abn_stamp_list_web_delay[abn_stamp_list_web_delay_idx];
}

unsigned long abnomal_stamp_list_tcp_rtt_large_update(
	unsigned long time_stamp)
{
	abn_stamp_list_tcp_rtt_large[abn_stamp_list_tcp_rtt_large_idx] =
		time_stamp;
	abn_stamp_list_tcp_rtt_large_idx =
		(abn_stamp_list_tcp_rtt_large_idx + 1) % TCP_RTT_LARGE_MAX;
	return abn_stamp_list_tcp_rtt_large[abn_stamp_list_tcp_rtt_large_idx];
}

unsigned int is_notify_chr_event(int old_data_reg_tech, u32 new_data_reg_tech)
{
	int temp_new_data_reg_tech = 0;
	int temp_old_data_reg_tech = 0;

	temp_old_data_reg_tech = old_data_reg_tech;
	temp_new_data_reg_tech = (int)new_data_reg_tech;

	if (temp_old_data_reg_tech == RIL_RADIO_TECHNOLOGY_LTE_CA)
		temp_old_data_reg_tech = RIL_RADIO_TECHNOLOGY_LTE;

	if (temp_new_data_reg_tech == RIL_RADIO_TECHNOLOGY_LTE_CA)
		temp_new_data_reg_tech = RIL_RADIO_TECHNOLOGY_LTE;

	if ((temp_old_data_reg_tech == RIL_RADIO_TECHNOLOGY_LTE) ||
		(temp_old_data_reg_tech == RIL_RADIO_TECHNOLOGY_NR) ||
		(temp_old_data_reg_tech == RADIO_TECHNOLOGY_LTE_EN_DC)) {
		if (temp_old_data_reg_tech != temp_new_data_reg_tech)
			return true;
	}
	return false;
}

int set_report_app_uid(int tag, u32 paras)
{
	if (tag >= 0 && tag < CHR_MAX_REPORT_APP_COUNT) {
		s_report_app_uid_lst[tag] = paras;
		return 0;
	}
	if (tag == DATA_REG_TECH_TAG) {
		old_data_reg_tech = data_reg_tech;
		data_reg_tech = paras;
		return 0;
	}
	if (tag == GET_AP_REPORT_TAG) {
		if (paras& 0x01)
			chr_notify_event(CHR_SPEED_SLOW_EVENT, g_user_space_pid,
				reportBuf(), NULL);
		return 0;
	}

	pr_info("chr:set_report_app_uid set 'tag' invaild. tag=%d\n", tag);
	return -1;
}

void save_app_syn_succ(u32 uid, u8 interface_type)
{
	int i = -1;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == s_report_app_uid_lst[i]) {
			rtn_stat[interface_type].report_app_stat_list[i].tcp_succ_num++;
			break;
		}
	}
}

void save_app_web_no_ack(u32 uid,u8 interface_type)
{
	int i = -1;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == s_report_app_uid_lst[i]) {
			rtn_stat[interface_type].report_app_stat_list[i].total_num++;
			rtn_stat[interface_type].report_app_stat_list[i].no_ack_num++;
			break;
		}
	}
}

void save_app_web_fail(u32 uid,u8 interface_type)
{
	int i = -1;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == s_report_app_uid_lst[i]) {
			rtn_stat[interface_type].report_app_stat_list[i].total_num++;
			rtn_stat[interface_type].report_app_stat_list[i].fail_num++;
			break;
		}
	}
}

static void save_app_web_delay(u32 uid, int web_delay,u8 interface_type)
{
	int i = -1;
	struct report_app_stat *app_stat = NULL;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == s_report_app_uid_lst[i]) {
			app_stat = &rtn_stat[interface_type].report_app_stat_list[i];
			app_stat->total_num++;
			app_stat->succ_num++;

			app_stat->web_delay += web_delay;

			if (web_delay > DELAY_THRESHOLD_L1 &&
					web_delay <= DELAY_THRESHOLD_L2)
				app_stat->delay_num_L1++;

			else if (web_delay > DELAY_THRESHOLD_L2 &&
					web_delay <= DELAY_THRESHOLD_L3)
				app_stat->delay_num_L2++;

			else if (web_delay > DELAY_THRESHOLD_L3 &&
					web_delay <= DELAY_THRESHOLD_L4)
				app_stat->delay_num_L3++;

			else if (web_delay > DELAY_THRESHOLD_L4 &&
					web_delay <= DELAY_THRESHOLD_L5)
				app_stat->delay_num_L4++;

			else if (web_delay > DELAY_THRESHOLD_L5 &&
					web_delay <= DELAY_THRESHOLD_L6)
				app_stat->delay_num_L5++;

			else if (web_delay > DELAY_THRESHOLD_L6)
				app_stat->delay_num_L6++;
			break;
		}
	}
}

static void save_app_tcp_rtt(u32 uid, u32 tcp_rtt,u8 interface_type)
{
	int i = -1;
	struct report_app_stat *app_stat = NULL;

	for (i = 0; i < CHR_MAX_REPORT_APP_COUNT; i++) {
		if (uid == s_report_app_uid_lst[i]) {
			app_stat = &rtn_stat[interface_type].report_app_stat_list[i];

			app_stat->tcp_total_num++;
			app_stat->tcp_rtt += tcp_rtt;

			if (tcp_rtt > RTT_THRESHOLD_L1 &&
					tcp_rtt <= RTT_THRESHOLD_L2)
				app_stat->rtt_num_L1++;

			else if (tcp_rtt > RTT_THRESHOLD_L2 &&
					tcp_rtt <= RTT_THRESHOLD_L3)
				app_stat->rtt_num_L2++;

			else if (tcp_rtt > RTT_THRESHOLD_L3 &&
					tcp_rtt <= RTT_THRESHOLD_L4)
				app_stat->rtt_num_L3++;

			else if (tcp_rtt > RTT_THRESHOLD_L4 &&
					tcp_rtt <= RTT_THRESHOLD_L5)
				app_stat->rtt_num_L4++;

			else if (tcp_rtt > RTT_THRESHOLD_L5)
				app_stat->rtt_num_L5++;

			break;
		}
	}
}

#ifdef CONFIG_HW_NETBOOSTER_MODULE
static void video_chr_stat_report(void)
{
	struct video_chr_para video_chr = {0};

	chr_video_stat(&video_chr);
	rtn_stat[RMNET_INTERFACE].vod_avg_speed = video_chr.vod_avg_speed;
	rtn_stat[RMNET_INTERFACE].vod_freez_num = video_chr.vod_freez_num;
	rtn_stat[RMNET_INTERFACE].vod_time = video_chr.vod_time;
	rtn_stat[RMNET_INTERFACE].uvod_avg_speed = video_chr.uvod_avg_speed;
	rtn_stat[RMNET_INTERFACE].uvod_freez_num = video_chr.uvod_freez_num;
	rtn_stat[RMNET_INTERFACE].uvod_time = video_chr.uvod_time;
	return;
}
#endif
#undef DEBUG
