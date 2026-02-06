

#include <hwnet/ipv4/wifi_delayst.h>
#include <linux/ktime.h>
#include <linux/timekeeping.h>
#include <net/tcp.h>
#include <linux/sysctl.h>
#include <trace/events/skb.h>
#include <linux/cryptohash.h>
#include <linux/inet.h>
#include <hwnet/chr/wbc_hw_hook.h>

#define PRINT_INTERVAL 1000
#define THRESHOLD_INIT 4
#define INTERVAL_THRESHOLD_FIRST 50
#define INTERVAL_THRESHOLD_SECOND 200
#define INTERVAL_THRESHOLD_THIRD 1000
#define INTERVAL_THRESHOLD_FORTH 10000
#define INTERVAL_TIME 1000000
#define FLAG_NAME_LENGTH 20
#define ADDR_LENGTH_MAX 128
#define ADDR_LENGTH_SMALL 96
#define DELAY_MIN_TIMES 7
#define DELAY_MAX_TIMES 100
#define DELAY_EXCEED_COUNT 10

u8 delayst_switch = 1;
/* for sysctl command,statistic configuration */
char tcp_delay_filter[DELAY_FILTER_NAME_MAX] = {0};

/* store statistics results delay_s */
delay_stat_t delay_s = {{{0}, {0}, {0}, {0}, {0}}, {0}, {0}};

delay_stat_t rcv_delay_s = {{{0}, {0}, {0}, {0}, {0}}, {0}, {0}};
dp_settings_t settings = {
	.dp_mode = MODE_STAT,
	.print_interval = PRINT_INTERVAL,
	.android_uid = UID_MATCH_ALL,
	.dp_switch = FLAG_AUTO,
};

struct static_key wifi_delay_statistic_key __read_mostly =
	STATIC_KEY_INIT_FALSE;

/* operatios for delayst_skb_cb for  skb->cb */
skbdirect_enum skbprobe_get_direct(struct sk_buff *pskb)
{
	return delayst_skb_cb(pskb)->pdirect;
}

void skbprobe_set_direct(struct sk_buff *pskb, skbdirect_enum direct)
{
	delayst_skb_cb(pskb)->pdirect = direct;
}

skbtype_enum skbprobe_get_proto(struct sk_buff *pskb)
{
	return delayst_skb_cb(pskb)->pproto;
}

void skbprobe_set_proto(struct sk_buff *pskb, skbtype_enum proto)
{
	delayst_skb_cb(pskb)->pproto = proto;
}

ktime_t skbprobe_get_skbtime(struct sk_buff *pskb, int time_index)
{
	return (delayst_skb_cb(pskb)->ptime[time_index]);
}

__u32 skbprobe_get_skbuid(struct sk_buff *pskb)
{
	return (delayst_skb_cb(pskb)->android_uid);
}

void skbprobe_copy_delayskbcb(delayskb_cb_t *delayskbcb, struct sk_buff *pskb)
{
	if (is_direct(pskb, TP_SKB_DIRECT_SND)) {
		delayskbcb->android_uid = skbprobe_get_skbuid(pskb);
		delayskbcb->ptime[TP_SKB_SEND] =
			skbprobe_get_skbtime(pskb, TP_SKB_SEND);
		delayskbcb->ptime[TP_SKB_IP] =
			skbprobe_get_skbtime(pskb, TP_SKB_IP);
		delayskbcb->ptime[TP_SKB_HMAC_XMIT] =
			skbprobe_get_skbtime(pskb, TP_SKB_HMAC_XMIT);
		delayskbcb->ptime[TP_SKB_HMAC_TX] =
			skbprobe_get_skbtime(pskb, TP_SKB_HMAC_TX);
		delayskbcb->ptime[TP_SKB_DMAC] =
			skbprobe_get_skbtime(pskb, TP_SKB_DMAC);
	} else {
		delayskbcb->android_uid = skbprobe_get_skbuid(pskb);
		delayskbcb->ptime[TP_SKB_DMAC] = skbprobe_get_skbtime(pskb,
			TP_SKB_DMAC);
		delayskbcb->ptime[TP_SKB_HMAC_UPLOAD] =
			skbprobe_get_skbtime(pskb, TP_SKB_HMAC_UPLOAD);
		delayskbcb->ptime[TP_SKB_IP] =
			skbprobe_get_skbtime(pskb, TP_SKB_IP);
		delayskbcb->ptime[TP_SKB_RECV] =
			skbprobe_get_skbtime(pskb, TP_SKB_RECV);
	}
}

/* 0: not match ,1:match */
int is_uid_match(struct sk_buff *skb)
{
	if ((delayst_skb_cb(skb)->android_uid) == 0)
		return 0;

	if ((settings.android_uid == UID_MATCH_ALL) ||
		(settings.android_uid == (delayst_skb_cb(skb)->android_uid)))
		return 1;
	else
		return 0;
}

void skbprobe_record_first(struct sk_buff *skb, u32 type)
{
	if (type > (TP_SKB_MAX_ENTRY - 1))
		return;
	memset(delayst_skb_cb(skb)->ptime, 0,
		sizeof(ktime_t) * TP_SKB_MAX_ENTRY);
	delayst_skb_cb(skb)->ptime[type] = ktime_get_real();
	delayst_skb_cb(skb)->pdirect = ((type == 0) ?
		(TP_SKB_DIRECT_SND) : (TP_SKB_DIRECT_RCV));
	delayst_skb_cb(skb)->android_uid = UID_MATCH_ALL;
}

void skbprobe_record_time(struct sk_buff *skb, int index)
{
	if (index < 0 || index >= TP_SKB_MAX_ENTRY)
		return;

	delayst_skb_cb(skb)->ptime[index] = ktime_get_real();
}

void skbprobe_record_proto(struct sk_buff *skb, __u8 n)
{
	skbprobe_set_proto(skb, n);
}

u32 skbprobe_get_latency(struct sk_buff *skb, int t1, int t2)
{
	if (t1 < 0 || t2 < 0 || t1 >= TP_SKB_MAX_ENTRY ||
		t2 >= TP_SKB_MAX_ENTRY)
		return 0;

	return (ktime_to_us(skbprobe_get_skbtime(skb, t2)) -
		ktime_to_us(skbprobe_get_skbtime(skb, t1)));
}

u32 skbprobe_get_latency_from_delayst(delayskb_cb_t *delayskb, int t1, int t2)
{
	if (t1 < 0 || t2 < 0 || t1 >= TP_SKB_MAX_ENTRY ||
		t2 >= TP_SKB_MAX_ENTRY)
		return 0;

	return (ktime_to_us(delayskb->ptime[t2]) -
		ktime_to_us(delayskb->ptime[t1]));
}

int delay_record_get_interval_index(uint32_t delay)
{
	const int threshold_values[THRESHOLD_INIT] = {INTERVAL_THRESHOLD_FIRST,
		INTERVAL_THRESHOLD_SECOND, INTERVAL_THRESHOLD_THIRD,
		INTERVAL_THRESHOLD_FORTH};
	int j;

	for (j = 0; j < THRESHOLD_INIT; ++j) {
		if (delay < threshold_values[j])
			break;
	}
	return j;
}

void delay_record_send(delay_stat_t *delay, struct sk_buff *skb)
{
	int i;
	uint32_t un_delay;

	for (i = 0; i < THRESHOLD_INIT; ++i) {
		un_delay = skbprobe_get_latency(skb, i, i + 1);
		delay->t_totaldelay[i + 1] += un_delay;
		delay->t_totalpkts[i + 1] += 1;
		delay->t_gap[i + 1][delay_record_get_interval_index(
			un_delay)]++;
	}
}

void delay_record_receive(delay_stat_t *delay, struct sk_buff *skb)
{
	int i;
	uint32_t un_delay;

	for (i = 3; i > 0; --i) { /* delay init 3 */
		un_delay = skbprobe_get_latency(skb, i, i - 1);
		delay->t_totaldelay[i - 1] += un_delay;
		delay->t_totalpkts[i - 1] += 1;
		delay->t_gap[i - 1][delay_record_get_interval_index(
			un_delay)]++;
	}
}

/* combine funtion for network layer */
void delay_record_ip_combine(struct sk_buff *skb, skbdirect_enum direct)
{
	struct iphdr *iph = NULL;
	int index = TP_SKB_HMAC_UPLOAD;

	delay_flow_ctl(skb);
	iph = ip_hdr(skb);
	if (iph == NULL)
		return;

	if (iph->saddr == in_aton("127.0.0.1") ||
		iph->daddr == in_aton("127.0.0.1")) {
		memset(delayst_skb_cb(skb)->ptime, 0,
			sizeof(ktime_t) * TP_SKB_MAX_ENTRY);
		return;
	}
	if (direct == TP_SKB_DIRECT_SND)
		index = TP_SKB_SEND;

	if (IS_NEED_RECORD_DELAY(skb, index))
		skbprobe_record_time(skb, TP_SKB_IP);
}

/*
 * determine whether we should record the packet or
 * not in receive direction,filter function
 */
void delay_record_rcv_combine(struct sk_buff *skb, struct sock *sk,
	skbtype_enum type)
{
	delayst_skb_cb(skb)->android_uid = sk->sk_uid.val;
	if (IS_NEED_RECORD_DELAY(skb, TP_SKB_IP) && is_uid_match(skb)) {
		skbprobe_record_time(skb, TP_SKB_RECV);
		skbprobe_record_proto(skb, type);
		delay_record_gap(skb);
	}
}

/*
 * determine whether we should record the packet or
 * not in send direction,filter function
 */
void delay_record_snd_combine(struct sk_buff *skb)
{
	if (IS_NEED_RECORD_DELAY(skb, TP_SKB_HMAC_TX) && is_uid_match(skb)) {
		skbprobe_record_time(skb, TP_SKB_DMAC);
		delay_record_gap(skb);
	}
}

void delay_record_gap(struct sk_buff *skb)
{
	if (is_direct(skb, TP_SKB_DIRECT_SND)) {
		delay_print_time_exception(skb, TP_SKB_SEND, TP_SKB_DMAC);
		delay_record_send(&delay_s, skb);
	} else {
		delay_print_time_exception(skb, TP_SKB_DMAC, TP_SKB_RECV);
		delay_record_receive(&rcv_delay_s, skb);
	}
	if (settings.dp_mode == MODE_STAT) {
		if (delayst_switch != 0)
			delay_record_print_combine(skb);
	} else {
		trace_skb_latency(skb);
	}
}

/* record first timestamp */
void delay_record_first_combine(struct sock *sk, struct sk_buff *skb,
	skbdirect_enum direct, skbtype_enum type)
{
	skbprobe_record_first(skb, TP_SKB_SEND);
	skbprobe_record_proto(skb, type);
	delayst_skb_cb(skb)->android_uid = sk->sk_uid.val;
}

/* print delay time */
void delay_record_print_combine(struct sk_buff *skb)
{
	if (!delayst_switch)
		return;

	if (is_direct(skb, TP_SKB_DIRECT_RCV)) {
		if (packet_is_enough_for_print(rcv_delay_s, TP_SKB_RECV)) {
			printk("SumPkts record : %u ,Rcv Delay [<50us, <200us, <1ms, <10ms, >=10ms]driver:[%u, %u, %u, %u, %u ]ip:[%u, %u, %u, %u, %u ]transport:[%u, %u, %u, %u, %u ]Average: driver  %u, ip %u, transport %u",
			rcv_delay_s.t_totalpkts[TP_SKB_RECV], GET_UPLOAD_ALL,
			GET_RCV_IP_ALL, GET_RECV_ALL, GET_AVG_UPLOAD,
			GET_RCV_AVG_IP, GET_AVG_RECV);
			memset(&rcv_delay_s, 0, sizeof(delay_stat_t));
		}
	} else {
		if (packet_is_enough_for_print(delay_s, TP_SKB_DMAC)) {
			printk("SumPkts record in driver: %u ,Send Delay [<50us, <200us, <1ms, <10ms, >=10ms]transport:[%u, %u, %u, %u, %u ]ip:[%u, %u, %u, %u, %u ]hmac:[%u, %u, %u, %u, %u ]driver:[%u, %u, %u, %u, %u ]Average: transport %u, ip %u,hmac %u,driver %u",
			delay_s.t_totalpkts[TP_SKB_DMAC],
			GET_SND_IP_ALL, GET_XMIT_ALL, GET_TX_ALL, GET_DMAC_ALL,
			GET_SND_AVG_IP, GET_AVG_XMIT, GET_AVG_TX, GET_AVG_DMAC);
			memset(&delay_s, 0, sizeof(delay_stat_t));
		}
	}
}

/* for flow control ,based on pps */
void delay_flow_ctl(struct sk_buff *skb)
{
	static u32 total_pkt;
	static u8 delay_cnt;
	static s64 t_begin;
	s64 kt;

	if (!IS_DELAY_SWITCH_AUTO)
		return;

	kt = ktime_to_us(ktime_get_real());
	if (t_begin == 0)
		t_begin = kt;

	total_pkt += 1;
	if ((kt - t_begin) > INTERVAL_TIME) {
		/* statistics time is over 1s */
		if ((total_pkt > DELAY_FLOW_THRESHOLD && delayst_switch) ||
			(total_pkt < DELAY_FLOW_THRESHOLD &&
			(!delayst_switch))) {
			/* overflow/lowflow */
			delay_cnt++;
			if (delay_cnt >= 3) { // at least 3 times
				/* overflow/lowflow for at least 3 times */
				delayst_switch = !delayst_switch;
				delay_cnt = 0;
			}
		} else {
			/* not overflow/lowflow, count again */
			delay_cnt = 0;
		}
		total_pkt = 0;
		t_begin = kt;
	}
}

static void print_exceed_tx(u8 delay_exceed_count_tx, delayskb_cb_t *tx_buff)
{
	struct delay_chr_report delay_excp_chr_report = {0};

	delay_excp_chr_report.data_direct = TP_SKB_DIRECT_SND;
	delay_excp_chr_report.exception_cnt = delay_exceed_count_tx;
	delay_excp_chr_report.driver_delay =
	skbprobe_get_latency_from_delayst(tx_buff, TP_SKB_HMAC_TX,
		TP_SKB_DMAC);
	delay_excp_chr_report.hmac_delay =
		skbprobe_get_latency_from_delayst(tx_buff, TP_SKB_HMAC_XMIT,
		TP_SKB_HMAC_TX);
	delay_excp_chr_report.ip_delay =
		skbprobe_get_latency_from_delayst(tx_buff, TP_SKB_IP,
		TP_SKB_HMAC_XMIT);
	delay_excp_chr_report.transport_delay =
		skbprobe_get_latency_from_delayst(tx_buff, TP_SKB_SEND,
		TP_SKB_IP);
	delay_excp_chr_report.android_uid = tx_buff->android_uid;
	wifi_kernel_delay_report(&delay_excp_chr_report);
	printk("time_delay_exception! TX: exception_count = %d, uid = %u, transport_delay %u, ip_delay %u,hmac_delay %u,driver_delay %u,send_time:%lld,ip_time:%lld",
		delay_exceed_count_tx, tx_buff->android_uid,
		delay_excp_chr_report.transport_delay,
		delay_excp_chr_report.ip_delay,
		delay_excp_chr_report.hmac_delay,
		delay_excp_chr_report.driver_delay,
		ktime_to_us(tx_buff->ptime[TP_SKB_SEND]),
		ktime_to_us(tx_buff->ptime[TP_SKB_IP]));
}

static void print_exceed_rx(u8 delay_exceed_count_rx, delayskb_cb_t *rx_buff)
{
	struct delay_chr_report delay_excp_chr_report = {0};

	delay_excp_chr_report.data_direct =
		TP_SKB_DIRECT_RCV;
	delay_excp_chr_report.exception_cnt =
		delay_exceed_count_rx;
	delay_excp_chr_report.driver_delay =
		skbprobe_get_latency_from_delayst(rx_buff, TP_SKB_DMAC,
		TP_SKB_HMAC_UPLOAD);
	delay_excp_chr_report.ip_delay =
		skbprobe_get_latency_from_delayst(rx_buff, TP_SKB_HMAC_UPLOAD,
		TP_SKB_IP);
	delay_excp_chr_report.transport_delay =
		skbprobe_get_latency_from_delayst(rx_buff, TP_SKB_IP,
		TP_SKB_RECV);
	delay_excp_chr_report.android_uid = rx_buff->android_uid;
	wifi_kernel_delay_report(&delay_excp_chr_report);
	printk("time_delay_exception! RX: exception_count = %d, uid = %u, driver_delay %u, ip_delay %u, transport_delay %u, DMAC_time:%lld,UPLOAD_time:%lld",
	delay_exceed_count_rx, rx_buff->android_uid,
	delay_excp_chr_report.driver_delay,
	delay_excp_chr_report.ip_delay,
	delay_excp_chr_report.transport_delay,
	ktime_to_us(rx_buff->ptime[TP_SKB_DMAC]),
	ktime_to_us(rx_buff->ptime[TP_SKB_HMAC_UPLOAD]));
}

/* for time delay exception */
void delay_print_time_exception(struct sk_buff *skb, int t1, int t2)
{
	static u8 delay_exceed_count_tx;
	static u8 delay_exceed_count_rx;
	u64 delay;
	static delayskb_cb_t tx_buff;
	static delayskb_cb_t rx_buff;

	delay = skbprobe_get_latency(skb, t1, t2);
	if (is_direct(skb, TP_SKB_DIRECT_SND)) {
		if ((delay > DELAY_MIN_TIMES * DELAY_NORMAL_TIME) &&
			(delay < DELAY_MAX_TIMES * DELAY_NORMAL_TIME)) {
		/* bigger than 7 times normal delay */
			delay_exceed_count_tx++;
			if (delay_exceed_count_tx >= DELAY_EXCEED_COUNT)
				skbprobe_copy_delayskbcb(&tx_buff, skb);
		} else {
			if (delay_exceed_count_tx >= DELAY_EXCEED_COUNT)
				print_exceed_tx(delay_exceed_count_tx,
					&tx_buff);

			delay_exceed_count_tx = 0;
		}
	} else {
		if ((delay > 7 * DELAY_NORMAL_TIME) && /* 7 times */
			(delay < 100 * DELAY_NORMAL_TIME)) { /* 100 times */
			delay_exceed_count_rx++;
			if (delay_exceed_count_rx >= 10) /* exceed count 10 */
				skbprobe_copy_delayskbcb(&rx_buff, skb);
		} else {
			if (delay_exceed_count_rx >= 10) /* exceed count 10 */
				print_exceed_rx(delay_exceed_count_rx,
					&rx_buff);

			delay_exceed_count_rx = 0;
		}
	}
}

static void solve_flag_state(const char *clone)
{
	if (strcmp(clone, "flag_on") == 0) {
		settings.dp_switch = FLAG_ON;
		static_key_enable(&wifi_delay_statistic_key);
		delayst_switch = 1;
	} else if (strcmp(clone, "flag_off") == 0) {
		settings.dp_switch = FLAG_OFF;
		static_key_disable(&wifi_delay_statistic_key);
		delayst_switch = 0;
	} else if (strcmp(clone, "flag_auto") == 0) {
		settings.dp_switch = FLAG_AUTO;
		delayst_switch = 1;
	}
}

static void solve_m_cmd(const char *clone)
{
	if (strcmp(clone, "mode_stat") == 0)
		settings.dp_mode = MODE_STAT;
	else if (strcmp(clone, "mode_trace") == 0)
		settings.dp_mode = MODE_TRACE;
}

static void solve_u_cmd(const char *sub_cmd)
{
	unsigned int uid = 0;
	int err;

	err = kstrtouint(sub_cmd, 10, &uid); /* cmd length 10 */
	if (!err) {
		settings.android_uid = uid;
		CLEAN_DELAY_RECORD;
	}
}

static void solve_p_cmd(const char *sub_cmd)
{
	unsigned int interval = 0;
	int err;

	/* cmd length 10 */
	err = kstrtouint(sub_cmd, 10, &interval);
	if (!err) {
		settings.print_interval = interval;
		CLEAN_DELAY_RECORD;
	}
}

int delay_set_wifi_command(const char *val)
{
	char *clone = NULL;
	char *sub_cmd = NULL;

	const char flag_names[][FLAG_NAME_LENGTH] = {"flag_off",
		"flag_on", "flag_auto"};
	const char mode_names[][FLAG_NAME_LENGTH] = {"mode_stat", "mode_trace"};

	if (val == NULL)
		return -EFAULT; /* Bad address */
	clone = kstrdup(val, GFP_USER);
	if (clone == NULL)
		return -EFAULT;

	switch (clone[0]) {
	case 'f':
		solve_flag_state(clone);
		break;
	case 'm':
		solve_m_cmd(clone);
		break;
	case 'u':
		sub_cmd = strchr((const char *)clone, '_');
		if (sub_cmd == NULL) {
			kfree(clone);
			return -EFAULT;
		}
		sub_cmd++;
		solve_u_cmd(sub_cmd);
		break;
	case 'p':
		sub_cmd = strchr((const char *)clone, '_');
		if (sub_cmd == NULL) {
			kfree(clone);
			return -EFAULT;
		}
		sub_cmd++;
		solve_p_cmd(sub_cmd);
		break;
	case 'd':
		CLEAN_DELAY_RECORD;
		break;
	case 'c':
		printk("delay_command configure:delay_switch = %s,dealy_mode = %s,print_interval = %u,android_uid = %u",
			flag_names[settings.dp_switch],
			mode_names[settings.dp_mode],
			settings.print_interval, settings.android_uid);
		break;
	default:
		break;
	}
	kfree(clone);
	return 0;
}

int proc_wifi_delay_command(struct ctl_table *ctl, int write,
	void __user *buffer, size_t *lenp, loff_t *ppos)
{
	char val[DELAY_FILTER_NAME_MAX];
	struct ctl_table tbl = {
		.data = val,
		.maxlen = DELAY_FILTER_NAME_MAX,
	};
	int err = 0;

	err = proc_dostring(&tbl, write, buffer, lenp, ppos);
	if (write && err == 0)
		err = delay_set_wifi_command(val);
	return err;
}

