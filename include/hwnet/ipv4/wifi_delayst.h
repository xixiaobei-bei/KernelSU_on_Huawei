

#ifndef __WIFI_DELAYST_H
#define __WIFI_DELAYST_H

#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <net/ipv6.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/tracepoint.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/jump_label.h>
#include <linux/version.h>

#define DELAY_FILTER_NAME_MAX 30
#define DELAY_NORMAL_TIME (30 * 1000) /* 30ms */
#define UID_MATCH_ALL 888888 /* match all uid */
#define MEMCPY_SKB_CB_LEN 96
#define MEMCPY_SKB_CB_MPTCP_LEN 128

/* flow control, for auto delay statistics,based on pps  50Mbps/1.5KB(1536B) */
#define DELAY_FLOW_THRESHOLD  (((50 * 1024 * 1024) / 8) / 1536)

/* macro definition for delay record point */
enum delay_record_point {
	TP_SKB_SEND = 0,
	TP_SKB_IP = 1,
	TP_SKB_HMAC_XMIT = 2,
	TP_SKB_HMAC_TX = 3,
	TP_SKB_DMAC = 4,

	TP_SKB_RECV = 0,
	TP_SKB_HMAC_UPLOAD = 2,
	TP_SKB_HMAC_RX = 3,

	TP_SKB_MAX_ENTRY = 5
};

#define T_DELAY 5 /* Delay Gaps */

typedef struct delay_stat {
	uint32_t t_gap[TP_SKB_MAX_ENTRY][T_DELAY];
	uint32_t t_totaldelay[TP_SKB_MAX_ENTRY];
	uint32_t t_totalpkts[TP_SKB_MAX_ENTRY];
} delay_stat_t;

#define FLAG_OFF 0
#define FLAG_ON 1
#define FLAG_AUTO 2

typedef enum {
	MODE_STAT = 0,
	MODE_TRACE,
} dp_mode_enum;

typedef struct dp_setting { /* delay_print settings for users */
	dp_mode_enum dp_mode;
	unsigned int print_interval;
	unsigned int android_uid;
	u8 dp_switch;
} dp_settings_t;

typedef enum {
	TP_SKB_TYPE_TCP = 0,
	TP_SKB_TYPE_UDP,
} skbtype_enum;

typedef enum {
	TP_SKB_DIRECT_SND = 0,
	TP_SKB_DIRECT_RCV,
} skbdirect_enum;

/*
 * total len
 * 4 + 4 + 5 * 8 = 48
 */
typedef struct delayskbcb {
	__u32 pdirect : 1; /* 0 for send, 1 for receive */
	__u32 pproto : 1; /* 0 for tcp, 1 for udp */
	__u32 reserved : 30;
	__u32 android_uid;
	ktime_t ptime[TP_SKB_MAX_ENTRY]; /* timestamp for skb */
} delayskb_cb_t;

#ifdef CONFIG_MPTCP
#define delayst_skb_cb(__skb) ((delayskb_cb_t *) &((__skb)->cb[80]))
#else
#define delayst_skb_cb(__skb) ((delayskb_cb_t *) &((__skb)->cb[48]))
#endif

#if (KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE)
#define IS_NEED_RECORD_DELAY(__skb, __index) \
	(0 != (skbprobe_get_skbtime(__skb, __index).tv64))
#else
#define IS_NEED_RECORD_DELAY(__skb, __index) \
	(skbprobe_get_skbtime(__skb, __index) != 0)
#endif

/* packet is enough for print */
#define packet_is_enough_for_print(__DIRECT, __index) \
	((__DIRECT).t_totalpkts[__index] >= settings.print_interval)

/* copy skb->cb to other fragment ,using in ip_copy_meta_data */
#ifdef CONFIG_MPTCP
#define memcpy_skb_cb(__to, __from) \
	(memcpy((__to)->cb, (__from)->cb, MEMCPY_SKB_CB_MPTCP_LEN))
#else
#define memcpy_skb_cb(__to, __from) \
	(memcpy((__to)->cb, (__from)->cb, MEMCPY_SKB_CB_LEN))
#endif

extern char tcp_delay_filter[DELAY_FILTER_NAME_MAX];
extern dp_settings_t settings;
extern delay_stat_t delay_s;
extern delay_stat_t rcv_delay_s;
extern u8 delayst_switch;
extern struct static_key wifi_delay_statistic_key;

#define is_direct(__skb, __direct) ((__direct) == skbprobe_get_direct(__skb))
#define IS_DELAY_SWITCH_AUTO (settings.dp_switch == FLAG_AUTO)
#define get_time_from_skb(__skb, __index) \
	(ktime_to_us(skbprobe_get_skbtime(__skb, __index)))
#define DELAY_STATISTIC_SWITCH_ON \
	(static_key_enabled(&wifi_delay_statistic_key))

#define CLEAN_DELAY_RECORD \
	do {                                                \
		memset(&rcv_delay_s, 0, sizeof(delay_stat_t)); \
		memset(&delay_s, 0, sizeof(delay_stat_t));   \
	} while (0)

/* get time gap from delay_stat_t */
#define get_rcv_upload(__index) (rcv_delay_s.t_gap[TP_SKB_HMAC_UPLOAD][__index])
#define get_rcv_ip(__index) (rcv_delay_s.t_gap[TP_SKB_IP][__index])
#define get_rcv_recv(__index) (rcv_delay_s.t_gap[TP_SKB_RECV][__index])

#define get_snd_ip(__index) (delay_s.t_gap[TP_SKB_IP][__index])
#define get_snd_xmit(__index) (delay_s.t_gap[TP_SKB_HMAC_XMIT][__index])
#define get_snd_tx(__index) (delay_s.t_gap[TP_SKB_HMAC_TX][__index])
#define get_snd_dmac(__index) (delay_s.t_gap[TP_SKB_DMAC][__index])

#define GET_AVG_UPLOAD (rcv_delay_s.t_totaldelay[TP_SKB_HMAC_UPLOAD] / \
	rcv_delay_s.t_totalpkts[TP_SKB_HMAC_UPLOAD])
#define GET_RCV_AVG_IP (rcv_delay_s.t_totaldelay[TP_SKB_IP] / \
	rcv_delay_s.t_totalpkts[TP_SKB_IP])
#define GET_AVG_RECV (rcv_delay_s.t_totaldelay[TP_SKB_RECV] / \
	rcv_delay_s.t_totalpkts[TP_SKB_RECV])

#define GET_SND_AVG_IP (delay_s.t_totaldelay[TP_SKB_IP] / \
	delay_s.t_totalpkts[TP_SKB_IP])
#define GET_AVG_XMIT (delay_s.t_totaldelay[TP_SKB_HMAC_XMIT] / \
	delay_s.t_totalpkts[TP_SKB_HMAC_XMIT])
#define GET_AVG_TX (delay_s.t_totaldelay[TP_SKB_HMAC_TX] / \
	delay_s.t_totalpkts[TP_SKB_HMAC_TX])
#define GET_AVG_DMAC (delay_s.t_totaldelay[TP_SKB_DMAC] / \
	delay_s.t_totalpkts[TP_SKB_DMAC])

#define GET_UPLOAD_ALL get_rcv_upload(0), get_rcv_upload(1), \
	get_rcv_upload(2), get_rcv_upload(3), get_rcv_upload(4)
#define GET_RCV_IP_ALL get_rcv_ip(0), get_rcv_ip(1), get_rcv_ip(2), \
	get_rcv_ip(3), get_rcv_ip(4)
#define GET_RECV_ALL get_rcv_recv(0), get_rcv_recv(1), get_rcv_recv(2), \
	get_rcv_recv(3), get_rcv_recv(4)
#define GET_SND_IP_ALL get_snd_ip(0), get_snd_ip(1), get_snd_ip(2), \
	get_snd_ip(3), get_snd_ip(4)
#define GET_XMIT_ALL get_snd_xmit(0), get_snd_xmit(1), get_snd_xmit(2), \
	get_snd_xmit(3), get_snd_xmit(4)
#define GET_TX_ALL get_snd_tx(0), get_snd_tx(1), get_snd_tx(2), \
	get_snd_tx(3), get_snd_tx(4)
#define GET_DMAC_ALL get_snd_dmac(0), get_snd_dmac(1), get_snd_dmac(2), \
	get_snd_dmac(3), get_snd_dmac(4)

#define tp_store_addr_ports_v4(__entry, inet, sk)                   \
	do {                                                            \
		struct sockaddr_in *v4 = (void *)(__entry)->saddr;        \
		                                                        \
		v4->sin_family = AF_INET;                               \
		v4->sin_port = (inet)->inet_sport;                        \
		v4->sin_addr.s_addr = (inet)->inet_saddr;                 \
		v4 = (void *)(__entry)->daddr;                            \
		v4->sin_family = AF_INET;                               \
		v4->sin_port = (inet)->inet_dport;                        \
		v4->sin_addr.s_addr = (inet)->inet_daddr;                 \
	} while (0)

#if IS_ENABLED(CONFIG_IPV6)

#define tp_store_addr_ports(__entry, inet, sk)                 \
	do {                                                       \
		if (sk->sk_family == AF_INET6) {                    \
			struct sockaddr_in6 *v6 = (void *)(__entry)->saddr; \
			                                               \
			v6->sin6_family = AF_INET6;                     \
			v6->sin6_port = inet->inet_sport;               \
			v6->sin6_addr = inet6_sk(sk)->saddr;            \
			v6 = (void *)__entry->daddr;                    \
			v6->sin6_family = AF_INET6;                     \
			v6->sin6_port = inet->inet_dport;               \
			v6->sin6_addr = sk->sk_v6_daddr;                \
		} else                                              \
			tp_store_addr_ports_v4(__entry, inet, sk);      \
	} while (0)

#else

#define tp_store_addr_ports(__entry, inet, sk) \
	tp_store_addr_ports_v4(__entry, inet, sk);

#endif

#define tp_skb_store_addr_v4(__entry, skb, sport, dport)           \
	do {                                                           \
		struct sockaddr_in *v4 = (void *)(__entry)->saddr;        \
		struct iphdr *iph = ip_hdr(skb);                        \
		                                                       \
		v4->sin_family = AF_INET;                               \
		v4->sin_port = sport;                                   \
		if (iph) v4->sin_addr.s_addr = iph->saddr;              \
		v4 = (void *)(__entry)->daddr;                            \
		v4->sin_family = AF_INET;                               \
		v4->sin_port = dport;                                   \
		if (iph) v4->sin_addr.s_addr = iph->daddr;              \
	} while (0)

#if IS_ENABLED(CONFIG_IPV6)

#define tp_skb_store_addr(__entry, skb, sport, dport)          \
	do {                                                       \
		if (skb->protocol == htons(ETH_P_IPV6)) {           \
			struct sockaddr_in6 *v6 = (void *)__entry->saddr; \
			struct ipv6hdr *hdr = ipv6_hdr(skb);            \
			                                               \
			v6->sin6_family = AF_INET6;                        \
			v6->sin6_port = sport;                             \
			if (hdr) v6->sin6_addr = hdr->saddr;               \
			v6 = (void *)__entry->daddr;                   \
			v6->sin6_family = AF_INET6;                    \
			v6->sin6_port = dport;                         \
			if (hdr) v6->sin6_addr = hdr->daddr;           \
		} else if (skb->protocol == htons(ETH_P_IP))       \
			tp_skb_store_addr_v4(__entry, skb, sport, dport); \
	} while (0)

#else

#define tp_skb_store_addr(__entry, skb, sport, dport) \
	tp_skb_store_addr_v4(__entry, skb, sport, dport);

#endif

skbdirect_enum skbprobe_get_direct(struct sk_buff *pskb);
void skbprobe_set_direct(struct sk_buff *pskb, skbdirect_enum direct);
skbtype_enum skbprobe_get_proto(struct sk_buff *pskb);
void skbprobe_set_proto(struct sk_buff *pskb, skbtype_enum proto);
ktime_t skbprobe_get_skbtime(struct sk_buff *pskb, int time_index);
__u32 skbprobe_get_skbuid(struct sk_buff *pskb);

void skbprobe_record_first(struct sk_buff *skb, u32 type);
void skbprobe_record_time(struct sk_buff *skb, int index);
void skbprobe_record_proto(struct sk_buff *skb, __u8 n);
u32 skbprobe_get_latency(struct sk_buff *skb, int t1, int t2);
u32 skbprobe_get_latency(struct sk_buff *skb, int t1, int t2);
int is_uid_match(struct sk_buff *skb);

void delay_flow_ctl(struct sk_buff *skb);
void delay_record_send(delay_stat_t *delay, struct sk_buff *skb);
void delay_record_receive(delay_stat_t *delay, struct sk_buff *skb);
void delay_record_ip_combine(struct sk_buff *skb, skbdirect_enum direct);
void delay_record_first_combine(struct sock *sk, struct sk_buff *skb,
	skbdirect_enum direct, skbtype_enum type);
void delay_record_gap(struct sk_buff *skb);
void delay_record_print_combine(struct sk_buff *skb);
void delay_record_rcv_combine(struct sk_buff *skb, struct sock *sk,
	skbtype_enum type);
void delay_record_snd_combine(struct sk_buff *skb);
void delay_print_time_exception(struct sk_buff *skb, int t1, int t2);
int proc_wifi_delay_command(struct ctl_table *ctl, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos);

#endif /* end __WIFI_DELAYST_H */
