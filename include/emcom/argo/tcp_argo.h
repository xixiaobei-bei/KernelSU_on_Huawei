/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2018. All rights reserved.
 * Description: An implementation of the TCP Argo Algorithm.
 * Author: Zhong Zhang, <zz.ustc@gmail.com>
 */
#ifndef __TCP_ARGO_H__
#define __TCP_ARGO_H__

#include <huawei_platform/log/hw_log.h>

#ifdef CONFIG_TCP_ARGO
#define ARGO_RTO_MIN		(TCP_RTO_MIN * 1000 / HZ) /* ms */
#define ARGO_DELACK_THRESH	4
#undef HWLOG_TAG
#define HWLOG_TAG ARGO

HWLOG_REGIST();

enum {
	TCP_ARGO_LOG_DEBUG     = 1U << 0,
	TCP_ARGO_LOG_INFO      = 1U << 1,
	TCP_ARGO_LOG_ERROR     = 1U << 2,
};

extern int sysctl_argo_log_mask;

#define argo_logd(fmt, ...) \
	do { \
		if ((unsigned int)sysctl_argo_log_mask & TCP_ARGO_LOG_DEBUG) \
			hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

#define argo_logi(fmt, ...) \
	do { \
		if (((unsigned int)sysctl_argo_log_mask & TCP_ARGO_LOG_INFO) || \
		    ((unsigned int)sysctl_argo_log_mask & TCP_ARGO_LOG_DEBUG)) \
			hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

#define argo_loge(fmt, ...) \
	do { \
		if ((unsigned int)sysctl_argo_log_mask & TCP_ARGO_LOG_ERROR) \
			hwlog_info("%s "fmt"\n", __func__, ##__VA_ARGS__); \
	} while (0)

struct tcp_sock;
struct sock;
struct sk_buff;

struct tcp_argo {
	u32			abnormal_cnts; /* reserved */
	u32			snd_retrans_stamp;
	u32			high_snd_nxt;
	union {
		u32		hints;
		struct {
			u32	retrans_tsval[4];
			u32	rcv_nxt_jiffies; /* mstamp */
			u32	rcv_nxt_tsval;
			u32	rcv_nxt_tsecr;
			u32	snd_high_tsval;
			u32	snd_high_tsecr;
			u32	high_sacked;
			u32	snd_high_seq;
			u8	delay_ack_nums;
			bool	disable_argo;
		};
	};
};

extern int sysctl_tcp_argo;
extern void argo_clear_hints(struct tcp_argo *p);
extern void argo_try_to_init(struct sock *sk, const struct sk_buff *skb);
extern void argo_deinit(struct sock *sk);
extern void argo_calc_high_seq(struct sock *sk, struct sk_buff *skb);
extern void argo_calc_delay_ack_nums(struct sock *sk, u32 seq, u32 end_seq);
extern int argo_delay_acks_in_fastpath(struct sock *sk, struct tcp_sock *tp);
extern bool tcp_argo_send_ack_immediatly(struct tcp_sock *tp);

#endif /* CONFIG_TCP_ARGO */
#endif /* __TCP_ARGO_H__ */
