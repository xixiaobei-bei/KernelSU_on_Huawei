

#include <linux/module.h>
#include <net/tcp.h>

/* resolution of rtt */
#define  MIBO_RTT_RESOL                1000
/* sample the min rtt each minute, 60 sec. */
#define  MIBO_RTT_ROLLOVER_CYCLE       60
#define  MIBO_RTT_SCALE                3

/* switch to print debug info */
#define  MIBO_DEBUG_CURRENT_RTT_CB     1
#define  MIBO_DEBUG_MIN_RTT_CB         1

/* NOTE: len is the actual value - 1 */
#define MIBO_CURRENT_RTT_CB_LEN       (4 + 1)
#define MIBO_MIN_RTT_CB_LEN           (10 + 1)

/* ms, the target queuing delay, 25~100 ms is recommended */
static int mibo_target_qdelay = 80;
/* multiplicative factor of the off_target gain */
static int mibo_gain_num = 1;
static int mibo_gain_den = 1;
/* extensions to play with slow start */
static int mibo_do_ss = 1;

module_param(mibo_target_qdelay, int, 0644);
MODULE_PARM_DESC(mibo_target_qdelay, "target queuing delay");
module_param(mibo_gain_num, int, 0644);
MODULE_PARM_DESC(mibo_gain_num, "multiplicative factor of the gain");
module_param(mibo_gain_den, int, 0644);
MODULE_PARM_DESC(mibo_gain_den, "multiplicative factor of the gain");
module_param(mibo_do_ss, int, 0644);
MODULE_PARM_DESC(mibo_do_ss, "do slow start: 0 no, 1 yes");

/*
 * Mibo states, MIBO_INITED indicate tcp_mibo_init has called and
 * init successfully. MIBO_CAN_SS indicate mibo can do slow start.
 */
enum tcp_mibo_state {
	MIBO_INITED = (1 << 0),
	MIBO_CAN_SS = (1 << 1),
};

struct mibo_rtt_circ_buf {
	u16 *buffer;
	u8 len;	  /* length of the buffer */
	u8 first;	/* index of first buffered value */
	u8 next;	 /* index of last buffered value */
	u8 min;	  /* index of the minimum value bufferred */
};

/* struct mibo */
struct mibo {
	/* base/min rtt last rollover timestamp */
	u32 last_rollover_stamp;
	/* count of offset, to increase/decrease cwnd */
	u32 snd_cwnd_cnt;
	/* record last ack received timestamp */
	u32 last_ack_stamp;
	/* record some flags of mibo, maybe u32 is too large, just reserved */
	u32 flag;
	/* circular buffer to save min rtt of each one minute */
	struct mibo_rtt_circ_buf min_rtt_cbuf;
	/* circular buffer to save each rtt sampled */
	struct mibo_rtt_circ_buf current_rtt_cbuf;
	/* the buffer of min_rtt_cbuf, the unit of rtt saved is ms */
	u16 min_rtt_buf[MIBO_MIN_RTT_CB_LEN];
	/* the buffer of current_rtt_cbuf, the unit of rtt saved is ms */
	u16 current_rtt_buf[MIBO_CURRENT_RTT_CB_LEN];
};

static void mibo_circ_buf_init(struct mibo_rtt_circ_buf *cbuf,
	u16 *buf, u16 buf_len)
{
	if (cbuf == NULL)
		return;
	if (buf == NULL)
		return;

	cbuf->buffer = buf;
	cbuf->len = buf_len;
	cbuf->first = 0;
	cbuf->next = 0;
	cbuf->min = 0;
}

static void tcp_mibo_release(struct sock *sk)
{
}

static void tcp_mibo_init(struct sock *sk)
{
	struct mibo *mibo = inet_csk_ca(sk);

	mibo_circ_buf_init(&mibo->min_rtt_cbuf, mibo->min_rtt_buf,
		MIBO_MIN_RTT_CB_LEN);
	mibo_circ_buf_init(&mibo->current_rtt_cbuf, mibo->current_rtt_buf,
		MIBO_CURRENT_RTT_CB_LEN);

	mibo->last_rollover_stamp = tcp_jiffies32;
	mibo->snd_cwnd_cnt = 0;
	mibo->last_ack_stamp = 0;
	mibo->flag = 0;

	if (mibo_do_ss)
		mibo->flag |= MIBO_CAN_SS;

	mibo->flag |= MIBO_INITED;
}

static u32 mibo_cbuf_get_min(struct mibo_rtt_circ_buf *cbuf)
{
	/* RFC6817 requires all history to be set to infinity on init */
	if (cbuf->first == cbuf->next)
		return 0xffffffff;
	return cbuf->buffer[cbuf->min];
}

static u32 mibo_get_current_rtt(struct mibo *mibo)
{
	return mibo_cbuf_get_min(&(mibo->current_rtt_cbuf));
}

static u32 mibo_get_min_rtt(struct mibo *mibo)
{
	return mibo_cbuf_get_min(&(mibo->min_rtt_cbuf));
}

#if MIBO_DEBUG_CURRENT_RTT_CB || MIBO_DEBUG_MIN_RTT_CB
static void mibo_print_rtt(struct mibo_rtt_circ_buf *cbuf, char *name)
{
	u16 curr = cbuf->first;

	pr_debug("%s: time %u ", name, tcp_jiffies32);

	while (curr != cbuf->next) {
		pr_debug("%u ", cbuf->buffer[curr]);
		curr = (curr + 1) % cbuf->len;
	}

	pr_debug("min %u, len %u, first %u, next %u\n", cbuf->buffer[cbuf->min],
		cbuf->len, cbuf->first, cbuf->next);
}
#endif

/* set the size of snd_cwnd */
static u32 tcp_mibo_ssthresh(struct sock *sk)
{
	return tcp_reno_ssthresh(sk);
}

static void mibo_update_cwnd(struct sock *sk, s64 offset)
{
	struct mibo *mibo = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	u32 max_cwnd = ((u32) (tp->snd_cwnd)) * mibo_target_qdelay;
	s64 cwnd = mibo->snd_cwnd_cnt + offset;

	/* calculate the new cwnd_cnt */
	if (cwnd >= 0) {
		/* if we have a positive number update the cwnd_count */
		mibo->snd_cwnd_cnt = cwnd;
		if (mibo->snd_cwnd_cnt >= max_cwnd) {
			/* increase the cwnd */
			if (tp->snd_cwnd < tp->snd_cwnd_clamp)
				tp->snd_cwnd++;
			mibo->snd_cwnd_cnt = 0;
		}
	} else {
		/* need to decrease the cwnd but we don't want to set it to 0 */
		if (tp->snd_cwnd > 1) {
			tp->snd_cwnd--;
			/* set the cwnd_cnt to max value - mibo_target_qdelay */
			mibo->snd_cwnd_cnt =
				(tp->snd_cwnd - 1) * mibo_target_qdelay;
		} else {
			tp->snd_cwnd_cnt = 0;
		}
	}
}

static void tcp_mibo_cong_avoid(struct sock *sk, u32 ack, u32 acked)
{
	struct mibo *mibo = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	s64 offset;
	s64 current_rtt;
	s64 min_rtt;
	s64 qdelay;

	/*
	 * This checks that we are not limited by the congestion window
	 * nor by the application, and is basically the same check that
	 * it is performed in RFC6817
	 */
	if (!tcp_is_cwnd_limited(sk))
		return;

	if (tp->snd_cwnd <= 1)
		mibo->flag |= MIBO_CAN_SS;

	if (mibo_do_ss > 0 &&
		tp->snd_cwnd <= tcp_mibo_ssthresh(sk) &&
		(mibo->flag & MIBO_CAN_SS)) {
		acked = tcp_slow_start(tp, acked);
		if (!acked)
			return;
	} else {
		mibo->flag &= ~MIBO_CAN_SS;
	}

	current_rtt = ((s64) mibo_get_current_rtt(mibo));
	min_rtt = ((s64) mibo_get_min_rtt(mibo));

	qdelay = current_rtt - min_rtt;
	offset = ((s64) mibo_target_qdelay) - qdelay;

	offset *= mibo_gain_num;
	do_div(offset, mibo_gain_den);

	/* Do not ramp more than TCP. */
	if (offset > mibo_target_qdelay)
		offset = mibo_target_qdelay;

	mibo_update_cwnd(sk, offset);
}

static void mibo_add_rtt(struct mibo_rtt_circ_buf *cbuf, u16 rtt)
{
	u8 i;

	if (cbuf == NULL)
		return;

	if (cbuf->next == cbuf->first) {
		/* buffer is empty */
		cbuf->buffer[cbuf->next] = rtt;
		cbuf->min = cbuf->next;
		cbuf->next++;
		return;
	}
	/* set the new rtt */
	cbuf->buffer[cbuf->next] = rtt;
	/* update the min */
	if (rtt < cbuf->buffer[cbuf->min])
		cbuf->min = cbuf->next;

	/* increment the next pointer */
	cbuf->next = (cbuf->next + 1) % cbuf->len;

	if (cbuf->next != cbuf->first)
		return;

	/* when next equals to first, discard the first element */
	if (cbuf->min == cbuf->first) {
		/* Discard the min, search a new one */
		cbuf->min = i = (cbuf->first + 1) % cbuf->len;
		while (i != cbuf->next) {
			if (cbuf->buffer[i] < cbuf->buffer[cbuf->min])
				cbuf->min = i;
			i = (i + 1) % cbuf->len;
		}
	}
	/* move the first */
	cbuf->first = (cbuf->first + 1) % cbuf->len;
}

static void mibo_update_current_rtt(struct mibo *mibo, u16 rtt)
{
	if (!(mibo->flag & MIBO_INITED)) {
#if MIBO_DEBUG_CURRENT_RTT_CB
		pr_info("[%s:%d]mibo buf is not inited", __func__, __LINE__);
#endif
		return;
	}
	mibo_add_rtt(&(mibo->current_rtt_cbuf), rtt);

#if MIBO_DEBUG_CURRENT_RTT_CB
	pr_debug(" added rtt:%u to current_rtt_cbuf\n", rtt);
	mibo_print_rtt(&(mibo->current_rtt_cbuf), "current_rtt_cbuf");
#endif
}

static void mibo_update_min_rtt(struct mibo *mibo, u16 rtt)
{
	u32 last;
	struct mibo_rtt_circ_buf *cbuf = &(mibo->min_rtt_cbuf);

	if (!(mibo->flag & MIBO_INITED)) {
#if MIBO_DEBUG_MIN_RTT_CB
		pr_info("[%s:%d]mibo buf is not inited", __func__, __LINE__);
#endif
		return;
	}
	if (mibo->min_rtt_cbuf.next == mibo->min_rtt_cbuf.first) {
		/* The circular buffer is empty, just add. */
		mibo_add_rtt(cbuf, rtt);
		return;
	}

	if (tcp_jiffies32 - mibo->last_rollover_stamp >
		(MIBO_RTT_ROLLOVER_CYCLE * HZ)) {
#if MIBO_DEBUG_MIN_RTT_CB
		pr_debug("time %u, new rollover\n", tcp_jiffies32);
#endif
		mibo->last_rollover_stamp = tcp_jiffies32;
		mibo_add_rtt(cbuf, rtt);
	} else {
		/* update the last value and the min if it is the case */
		last = (cbuf->next + cbuf->len - 1) % cbuf->len;
		if (rtt < cbuf->buffer[last]) {
			cbuf->buffer[last] = rtt;
			if (rtt < cbuf->buffer[cbuf->min])
				cbuf->min = last;
		}

	}
#if MIBO_DEBUG_MIN_RTT_CB
	pr_debug("added rtt:%u to min_rtt_cbuf\n", rtt);
	mibo_print_rtt(&(mibo->min_rtt_cbuf), "min_rtt_cbuf");
#endif
}

static void tcp_mibo_pkts_acked(struct sock *sk,
	const struct ack_sample *sample)
{
	struct mibo *mibo = inet_csk_ca(sk);
	struct tcp_sock *tp = tcp_sk(sk);
	u32 srtt_us = tp->srtt_us >> MIBO_RTT_SCALE;
	u16 srtt_ms = (u16)(srtt_us / MIBO_RTT_RESOL);
	u16 sample_rtt_ms = (u16) (sample->rtt_us / MIBO_RTT_RESOL);

	if (sample->rtt_us > 0) {
		mibo_update_current_rtt(mibo, sample_rtt_ms);
		mibo_update_min_rtt(mibo, srtt_ms);
	}

	if (mibo->last_ack_stamp == 0) {
		mibo->last_ack_stamp = tcp_jiffies32;
		return;
	}

	if (before(mibo->last_ack_stamp + srtt_us, tcp_jiffies32)) {
		/* Set the congestion window to 1 */
		tp->snd_cwnd = 1;
		pr_debug("[%s:%d] MIBO Set the congestion window to 1",
		__func__, __LINE__);
	}

	mibo->last_ack_stamp = tcp_jiffies32;
}

static u32 tcp_mibo_undo_cwnd(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);

	return max(tp->snd_cwnd, tp->prior_cwnd);
}

static struct tcp_congestion_ops tcp_mibo = {
	.init = tcp_mibo_init,
	.ssthresh = tcp_mibo_ssthresh,
	.cong_avoid = tcp_mibo_cong_avoid,
	.pkts_acked = tcp_mibo_pkts_acked,
	.release = tcp_mibo_release,
	.undo_cwnd  = tcp_mibo_undo_cwnd,

	.owner = THIS_MODULE,
	.name = "mibo"
};

static int __init tcp_mibo_register(void)
{
	BUILD_BUG_ON(sizeof(struct mibo) > ICSK_CA_PRIV_SIZE);
	return tcp_register_congestion_control(&tcp_mibo);
}

static void __exit tcp_mibo_unregister(void)
{
	tcp_unregister_congestion_control(&tcp_mibo);
}

module_init(tcp_mibo_register);
module_exit(tcp_mibo_unregister);

MODULE_AUTHOR("Chen Shengjun");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCP Mibo (Moderately increasing bandwidth occupancy)");
