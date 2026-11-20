/* SPDX-License-Identifier: GPL-2.0 */
#undef TRACE_SYSTEM
#define TRACE_SYSTEM skb

#if !defined(_TRACE_SKB_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_SKB_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/tracepoint.h>

#ifdef CONFIG_WIFI_DELAY_STATISTIC
#include <hwnet/ipv4/wifi_delayst.h>
#endif
/*
 * Tracepoint for free an sk_buff:
 */
TRACE_EVENT(kfree_skb,

	TP_PROTO(struct sk_buff *skb, void *location),

	TP_ARGS(skb, location),

	TP_STRUCT__entry(
		__field(	void *,		skbaddr		)
		__field(	void *,		location	)
		__field(	unsigned short,	protocol	)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->location = location;
		__entry->protocol = ntohs(skb->protocol);
	),

	TP_printk("skbaddr=%p protocol=%u location=%p",
		__entry->skbaddr, __entry->protocol, __entry->location)
);

TRACE_EVENT(consume_skb,

	TP_PROTO(struct sk_buff *skb),

	TP_ARGS(skb),

	TP_STRUCT__entry(
		__field(	void *,	skbaddr	)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
	),

	TP_printk("skbaddr=%p", __entry->skbaddr)
);

TRACE_EVENT(skb_copy_datagram_iovec,

	TP_PROTO(const struct sk_buff *skb, int len),

	TP_ARGS(skb, len),

	TP_STRUCT__entry(
		__field(	const void *,		skbaddr		)
		__field(	int,			len		)
	),

	TP_fast_assign(
		__entry->skbaddr = skb;
		__entry->len = len;
	),

	TP_printk("skbaddr=%p len=%d", __entry->skbaddr, __entry->len)
);

#ifdef CONFIG_WIFI_DELAY_STATISTIC
TRACE_EVENT(skb_latency,
    TP_PROTO(struct sk_buff *skb),
    TP_ARGS(skb),
    TP_STRUCT__entry(
        __field(void *,	skb)
        __field(void *,	sk)
        __field(char *,  	direct)
        __field(char *,	proto)
        __field(u32,	    len)
        __field(u32,	    seq)

        __array(__u8, saddr, sizeof(struct sockaddr_in6))
        __array(__u8, daddr, sizeof(struct sockaddr_in6))
        __field(s64,	t1)
        __field(s64,	t2)
        __field(s64,	t3)
        __field(s64,	t4)
        __field(s64,	t5)

        __field(u32,	uid)
        __field(char *, interval1)
        __field(char *, interval2)
        __field(char *, interval3)
        __field(char *, interval4)
    ),

    TP_fast_assign(
         const struct inet_sock *inet;
         const struct sock *sk;
         const struct tcphdr *th;
         const struct udphdr *uh;

         __entry->skb  = skb;
         __entry->len   = skb->len;
         __entry->sk    = skb->sk;
         __entry->uid = delayst_skb_cb(skb)->android_uid;

         memset(__entry->saddr, 0, sizeof(struct sockaddr_in6));
         memset(__entry->daddr, 0, sizeof(struct sockaddr_in6));

         if (skb->sk){
		 inet = inet_sk(skb->sk);
		 sk = skb->sk;
		 tp_store_addr_ports(__entry, inet, skb->sk);
          } else if (skbprobe_get_proto(skb) == TP_SKB_TYPE_TCP){
		th = tcp_hdr(skb);
		if (th)
			tp_skb_store_addr(__entry, skb, th->source, th->dest);
         } else if (skbprobe_get_proto(skb) == TP_SKB_TYPE_UDP) {
		uh = udp_hdr(skb);
		if (uh)
			tp_skb_store_addr(__entry, skb, uh->source, uh->dest);
         } else {
             tp_skb_store_addr(__entry, skb, 0, 0);
         }

         switch (skbprobe_get_proto(skb)) {
             case TP_SKB_TYPE_TCP:
                 __entry->proto = "tcp";
                 break;
             case TP_SKB_TYPE_UDP:
                 __entry->proto = "udp";
                 break;
             default:
                __entry->proto = "X";
                break;
         }

         if (is_direct(skb, TP_SKB_DIRECT_RCV)) {
          __entry->t1 = get_time_from_skb(skb,TP_SKB_HMAC_RX);
	      __entry->t2 = get_time_from_skb(skb,TP_SKB_HMAC_UPLOAD);
	      __entry->t3 = get_time_from_skb(skb,TP_SKB_IP);
	      __entry->t4 = get_time_from_skb(skb,TP_SKB_RECV);
	      __entry->t5 = __entry->t4;
	      __entry->interval1 = "driver_delay";
	      __entry->interval2 = "ip_delay";
	      __entry->interval3 = "transport_delay";
	      __entry->interval4 = " ";
	      __entry->direct = "in";
	      __entry->seq = TCP_SKB_CB(skb)->end_seq;
         } else {
	      __entry->t1 = get_time_from_skb(skb,TP_SKB_SEND);
	      __entry->t2 = get_time_from_skb(skb,TP_SKB_IP);
	      __entry->t3 = get_time_from_skb(skb,TP_SKB_HMAC_XMIT);
	      __entry->t4 = get_time_from_skb(skb,TP_SKB_HMAC_TX);
	      __entry->t5 = get_time_from_skb(skb,TP_SKB_DMAC);
	      __entry->interval1 = "transport_delay";
	      __entry->interval2 = "ip_delay";
	      __entry->interval3 = "hmac_delay";
	      __entry->interval4 = "driver_delay";
	      __entry->direct = "out";
	      __entry->seq = TCP_SKB_CB(skb)->ack_seq;
         }
    ),
	TP_printk("uid=%u  len=%u seq=%u direct=%s proto=%s  IP:PORT=%pISpc ; %pISpc "
	    " %s=%lld, %s=%lld ,%s=%lld,%s=%lld ",
	    __entry->uid,__entry->len, __entry->seq,
	    __entry->direct, __entry->proto,
	    __entry->saddr, __entry->daddr,
	    __entry->interval1,  __entry->t2 - __entry->t1,
	    __entry->interval2,  __entry->t3 - __entry->t2,
	    __entry->interval3,  __entry->t4 - __entry->t3,
	    __entry->interval4,  __entry->t5 - __entry->t4)
);

#endif /*CONFIG_WIFI_DELAY_STATISTIC*/
#endif /* _TRACE_SKB_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
