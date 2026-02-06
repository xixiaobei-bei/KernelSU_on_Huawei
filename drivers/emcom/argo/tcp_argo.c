/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: An implementation of the TCP Argo Algorithm.
 * Author: Zhong Zhang, <zz.ustc@gmail.com>
 */
#include <net/tcp.h>
#include "securec.h"
#include "emcom/emcom_xengine.h"

#ifdef CONFIG_TCP_ARGO
int sysctl_tcp_argo __read_mostly = 1;
EXPORT_SYMBOL(sysctl_tcp_argo);

int sysctl_argo_log_mask __read_mostly;
EXPORT_SYMBOL(sysctl_argo_log_mask);

#define SHIFT_4X (2)
#define SHIFT_8X (3)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#define TCP_TIME_STAMP tcp_jiffies32
#endif

static bool argo_is_thin_stream(const struct tcp_sock *tp)
{
	long offs = tp->argo->high_sacked - tp->rcv_nxt;
	if (offs > (((long)tp->mss_cache) << SHIFT_8X))
		return false;

	return true;
}

static bool argo_is_timeout_retrans(const struct tcp_sock *tp)
{
	unsigned int rtt_ms;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	rtt_ms = jiffies_to_msecs(tp->rcv_rtt_est.rtt_us >> SHIFT_8X);
#else
	rtt_ms = jiffies_to_msecs(tp->rcv_rtt_est.rtt >> SHIFT_8X);
#endif
	if (rtt_ms < (ARGO_RTO_MIN >> 1)) {
		if (jiffies_to_msecs(TCP_TIME_STAMP - tp->argo->rcv_nxt_jiffies) > ARGO_RTO_MIN)
			return true;
	} else {
		if (tp->argo->retrans_tsval[ARRAY_SIZE(tp->argo->retrans_tsval) - 1])
			return true;
	}

	return false;
}

void argo_clear_hints(struct tcp_argo *p)
{
	(void)memset_s(&p->hints, sizeof(struct tcp_argo) - offsetof(struct tcp_argo, hints),
		0x00, sizeof(struct tcp_argo) - offsetof(struct tcp_argo, hints));
}
EXPORT_SYMBOL(argo_clear_hints); /*lint !e580*/

static bool argo_time_to_init(const struct tcp_sock *tp,
				     const struct sk_buff *skb)
{
	const char *delim = EMCOM_WLAN0_IFNAME;
	size_t len = strlen(delim);

	argo_logd("Argo time to init dest addr: %x, dest port: %u, "
		  "sysctl_tcp_argo: %u, dev name: %s,"
		  "argo: %p, tstamp: %x, "
		  "sack: %x, syn: %x",
		  ((const struct sock *)tp)->sk_daddr,
		  ((const struct sock *)tp)->sk_dport,
		  sysctl_tcp_argo, skb->dev->name,
		  tp->argo, tp->rx_opt.tstamp_ok,
		  tp->rx_opt.sack_ok, tcp_hdr(skb)->syn);

	if (!sysctl_tcp_argo)
		return false;

	if (strncmp(skb->dev->name, delim, len))
		return false;

	if (tp->argo)
		return false;

	if (!tp->rx_opt.tstamp_ok || !tp->rx_opt.sack_ok)
		return false;

	if (tcp_hdr(skb)->syn)
		return false;

	return true;
}

void argo_try_to_init(struct sock *sk, const struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (unlikely(argo_time_to_init(tp, skb))) {
		argo_logi("Argo init. dest addr: %x, dest port: %u",
			  sk->sk_daddr, sk->sk_dport);
		tp->argo = kzalloc(sizeof(struct tcp_argo), GFP_ATOMIC);
	}
}
EXPORT_SYMBOL(argo_try_to_init); /*lint !e580*/

void argo_deinit(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	if (tp->argo) {
		argo_logi("Argo deinit. dest addr: %x, dest port: %u",
			  sk->sk_daddr, sk->sk_dport);
		kfree(tp->argo);
	}
}
EXPORT_SYMBOL(argo_deinit); /*lint !e580*/

static void argo_check_ts(struct sock *sk)
{
	int i;
	int nums;
	struct tcp_sock *tp = tcp_sk(sk);

	if (!tp->argo->snd_high_tsval && !tp->argo->snd_high_tsecr) {
		/* Init */
		tp->argo->snd_high_tsval = tp->rx_opt.rcv_tsval;
		tp->argo->snd_high_tsecr = tp->rx_opt.rcv_tsecr;
		/* Check pingpong */
		tp->argo->high_snd_nxt = tp->snd_nxt;
	} else if (before(tp->rx_opt.rcv_tsval, tp->argo->snd_high_tsval) ||
		before(tp->rx_opt.rcv_tsecr, tp->argo->snd_high_tsecr)) {
		argo_logd("Arog disable because of disorder, "
		"dest addr: %x, dest port: %u",
		sk->sk_daddr, sk->sk_dport);
		/* Obviously, disorder occurred in the network. */
		tp->argo->disable_argo = true;
	} else {
		/* Update */
		tp->argo->snd_high_tsval = tp->rx_opt.rcv_tsval;
		tp->argo->snd_high_tsecr = tp->rx_opt.rcv_tsecr;
	}

	if (!tp->argo->retrans_tsval[0]) {
		tp->argo->retrans_tsval[0] = TCP_TIME_STAMP + tp->tsoffset;
		tp->argo->rcv_nxt_jiffies = TCP_TIME_STAMP;
	} else {
		nums = sizeof(tp->argo->retrans_tsval) >> SHIFT_4X;
		for (i = 1; i < nums; i++)
			if (!tp->argo->retrans_tsval[i] &&
			    !before(tp->rx_opt.rcv_tsecr,
				    tp->argo->retrans_tsval[i - 1]))
				tp->argo->retrans_tsval[i] = TCP_TIME_STAMP + tp->tsoffset;
	}
}

static void argo_check_seq(struct sock *sk, struct tcp_skb_cb *tcb)
{
	struct tcp_sock *tp = tcp_sk(sk);

	if (!tp->argo->high_sacked ||
	    after(tcb->end_seq, tp->argo->high_sacked)) {
		tp->argo->high_sacked = tcb->end_seq;
	} else if (tp->rcv_nxt == tcb->seq &&
		   !before(tp->rx_opt.rcv_tsecr,
		   tp->argo->retrans_tsval[0])) {
		if (!argo_is_thin_stream(tp)) {
			tp->argo->snd_high_seq = tp->argo->high_sacked;
			tp->argo->snd_retrans_stamp = tp->rx_opt.rcv_tsval;
		} else {
			argo_logd("Argo disable because of thin stream, "
				  "dest addr: %x, dest port: %u",
				  sk->sk_daddr, sk->sk_dport);
			tp->argo->disable_argo = true;
		}
	} else {
		argo_logd("Argo disable because of timestamp "
			  "echo reply check. "
			  "dest addr: %x, dest port: %u",
			  sk->sk_daddr, sk->sk_dport);
		tp->argo->disable_argo = true;
	}
}

void argo_calc_high_seq(struct sock *sk, struct sk_buff *skb)
{
	struct tcp_sock *tp = tcp_sk(sk);
	struct tcp_skb_cb *tcb = TCP_SKB_CB(skb);

	if (!tp->argo)
		return;

	argo_logd("Argo Cacl high seq. dest addr: %x, dest port: %u, "
		  "tcb seq: %u, tcb end_seq: %u, delay ack num: %u, "
		  "disable_argo: %u, snd high seq :%u, "
		  "high_sack :%u, rcv_nxt: %u, rcv_tsval: %u, "
		  "snd_high_tsval: %u, rcv_tsecr: %u, "
		  "snd_high_tsecr: %u, retrans_tsval[0]: %u",
		  sk->sk_daddr, sk->sk_dport,
		  tcb->seq, tcb->end_seq, tp->argo->delay_ack_nums,
		  tp->argo->disable_argo, tp->argo->snd_high_seq,
		  tp->argo->high_sacked, tp->rcv_nxt, tp->rx_opt.rcv_tsval,
		  tp->argo->snd_high_tsval, tp->rx_opt.rcv_tsecr,
		  tp->argo->snd_high_tsecr, tp->argo->retrans_tsval[0]);

	if (!after(tcb->end_seq, tcb->seq))
		return;

	if (tp->argo->delay_ack_nums) {
		argo_logd("Argo disable because from slow path to slow path. "
			  "dest addr: %x, dest port: %u, delay ack num: %u",
			  sk->sk_daddr, sk->sk_dport, tp->argo->delay_ack_nums);
		/* From slowpath to slowpath */
		tp->argo->disable_argo = true;
		tp->argo->delay_ack_nums = 0;
	}

	argo_check_ts(sk);

	if (!tp->argo->snd_high_seq && !tp->argo->disable_argo)
		argo_check_seq(sk, tcb);

	/* Check timeout retransmission */
	if (tp->rcv_nxt == tcb->seq) {
		if (argo_is_timeout_retrans(tp)) {
			argo_logd("Argo disable because of timeout retrans. "
				  "dest addr: %x, dest port: %u",
				  sk->sk_daddr, sk->sk_dport);
			tp->argo->disable_argo = true;
		}

		(void)memset_s(tp->argo->retrans_tsval, sizeof(tp->argo->retrans_tsval),
			0x00, sizeof(tp->argo->retrans_tsval));
		tp->argo->retrans_tsval[0] = TCP_TIME_STAMP + tp->tsoffset;
		tp->argo->rcv_nxt_jiffies = TCP_TIME_STAMP;
	}
}
EXPORT_SYMBOL(argo_calc_high_seq); /*lint !e580*/

void argo_calc_delay_ack_nums(struct sock *sk, u32 seq, u32 end_seq)
{
	struct tcp_sock *tp = tcp_sk(sk);
	int is_skb_queue_empty;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4,4,0)
	is_skb_queue_empty = RB_EMPTY_ROOT(&tp->out_of_order_queue);
#else
	is_skb_queue_empty = skb_queue_empty((struct sk_buff_head *)&tp->out_of_order_queue);
#endif

	if (!tp->argo)
		return;

	argo_logd("Argo Calc delay ack. dest addr: %x, dest port: %u, "
		  "tcb seq: %u, tcb end_seq: %u, snd_high_seq: %u, "
		  "tp rcv_nxt: %u, skb ofo queue: %u, "
		  "disable_argo: %u",
		  sk->sk_daddr, sk->sk_dport,
		  seq, end_seq, tp->argo->snd_high_seq,
		  tp->rcv_nxt, is_skb_queue_empty,
		  tp->argo->disable_argo);

	if (!after(end_seq, seq))
		return;

	if (tp->argo->snd_high_seq &&
	    !before(tp->rcv_nxt, tp->argo->snd_high_seq) &&
	    !is_skb_queue_empty) {
		argo_logd("Argo disable because of lost packet "
			  "between high_seq and snd_nxt, "
			  "dest addr: %x, dest port: %u",
			  sk->sk_daddr, sk->sk_dport);
		/* Packets lost between high_seq and snd_nxt on sender. */
		tp->argo->disable_argo = true;
	}

	if (tp->argo->snd_high_seq && !tp->argo->disable_argo &&
	    is_skb_queue_empty) {
		argo_logi("Argo start dealy ack, "
			  "dest addr: %x, dest port: %u",
			  sk->sk_daddr, sk->sk_dport);
		tp->argo->delay_ack_nums = 1;
	} else if (tp->argo->disable_argo &&
		   is_skb_queue_empty) {
		argo_logi("Argo clear hints because of disable argo, "
			  "dest addr: %x, dest port: %u",
			  sk->sk_daddr, sk->sk_dport);
		argo_clear_hints(tp->argo);
	}
}
EXPORT_SYMBOL(argo_calc_delay_ack_nums); /*lint !e580*/

int argo_delay_acks_in_fastpath(struct sock *sk, struct tcp_sock *tp)
{
	int err = 0;
	unsigned int reduce;

	if (tp->argo && sysctl_tcp_argo && !tp->rx_opt.num_sacks &&
	    tp->argo->delay_ack_nums == ARGO_DELACK_THRESH) {
		argo_logd("Argo fast path delay ack, "
			  "dest addr: %x, dest port: %u, tp argo: %pK, "
			  "sysctl_tcp_argo: %u, num_sacks: %u, "
			  "delay ack nums: %u, snd_nxt: %u, "
			  "argo high_snd_nxt: %u",
			  sk->sk_daddr, sk->sk_dport, tp->argo,
			  sysctl_tcp_argo, tp->rx_opt.num_sacks,
			  tp->argo->delay_ack_nums, tp->snd_nxt,
			  tp->argo->high_snd_nxt);

		if (tp->snd_nxt == tp->argo->high_snd_nxt) {
			argo_logi("Argo reduce timestamp, dest addr: %x, dest port: %u",
				  sk->sk_daddr, sk->sk_dport);
			reduce = tp->rx_opt.ts_recent - tp->argo->snd_retrans_stamp + 1;
			tp->rx_opt.ts_recent -= reduce;
			tcp_send_ack(sk);
			tp->rx_opt.ts_recent += reduce;
		} else {
			argo_logi("Argo does not reduce timestamp "
				  "because of pingpong, dest addr: %x, dest port: %u",
				  sk->sk_daddr, sk->sk_dport);
			tcp_send_ack(sk);
		}

		tp->argo->delay_ack_nums = 0;
	} else if (tp->argo && sysctl_tcp_argo && tp->argo->delay_ack_nums &&
		   tp->argo->delay_ack_nums < ARGO_DELACK_THRESH) {
			argo_logd("Argo fast path delay ack, "
				  "dest addr: %x, dest port: %u, tp argo: %pK, "
				  "sysctl_tcp_argo: %u, delay ack nums: %u",
				  sk->sk_daddr, sk->sk_dport, tp->argo,
				  sysctl_tcp_argo, tp->argo->delay_ack_nums);
		tp->argo->delay_ack_nums++;
	} else if (tp->argo && !sysctl_tcp_argo) {
		argo_logd("Argo fast path delay ack, "
			  "dest addr: %x, dest port: %u, tp argo: %pK, "
			  "sysctl_tcp_argo: %u",
			  sk->sk_daddr, sk->sk_dport, tp->argo,
			  sysctl_tcp_argo);

		tp->argo->delay_ack_nums = 0;
		err = -EPERM; /* Operation not permitted */
	} else {
		err = -EPERM; /* Operation not permitted */
	}

	return err;
}
EXPORT_SYMBOL(argo_delay_acks_in_fastpath); /*lint !e580*/

bool tcp_argo_send_ack_immediatly(struct tcp_sock *tp)
{
	if (!tp->argo || (tp->argo && !tp->argo->delay_ack_nums))
		return true;
	else
		return false;
}
EXPORT_SYMBOL(tcp_argo_send_ack_immediatly); /*lint !e580*/
#endif /* CONFIG_TCP_ARGO */
