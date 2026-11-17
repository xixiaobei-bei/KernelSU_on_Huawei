/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This module is for app accelerator
 * Author: youpeigang@huawei.com
 * Create: 2020-01-10
 */

#include "app_accelerator.h"

#include <linux/net.h>
#include <net/sock.h>
#include <linux/list.h>
#include <linux/icmp.h>
#include <linux/icmpv6.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/tcp.h>

#define assign_short(p, val) (*(s16 *)(p) = (val))
#define assign_int(p, val) (*(s32 *)(p) = (val))
#define assign_uint(p, val) (*(u32 *)(p) = (val))
#define skip_byte(p, num) ((p) = (p) + (num))

int g_acc_uid = UID_INVALID;
/* default 2048 * 1024 * 8 bps */
u32 g_acc_threshold = BITS_PER_SECONDE_DEFAULT;
u32 g_acc_packet_threshold = STATICS_PACKET_NUM_INTERVAL;
u32 g_acc_cubic_trubo_init_win = CUBIC_TURBO_WIN_DEFAULT;
u32 g_acc_win_grow_fator_in_slow_start = WIN_GROW_FACTOR_DEFAULT;

int g_speed_test_uid = UID_INVALID;
u32 g_speed_test_status = SPEED_TEST_STATUS_STOP;
u32 g_speed_test_max_seq_ack_num = COUNT_NUM_DEFAULT;
u32 g_speed_test_dsack_num = COUNT_NUM_DEFAULT;
static notify_event *notifier = NULL;

msg_process *app_acc_init(notify_event *fn)
{
	if (!fn) {
		pr_info("app_acc_init null parameter");
		return NULL;
	}
	notifier = fn;
	pr_info("app_acc_init");
	g_acc_uid = UID_INVALID;
	g_speed_test_uid = UID_INVALID;
	return app_acc_process;
}

static void speed_test_chr_report()
{
	char event[SPEED_TEST_CHR_REPORT_LEN] = {0};
	char *p = event;

	// type
	assign_short(p, SPEED_TEST_CHR_RPT);
	skip_byte(p, sizeof(s16));

	// len = 2B type + 2B len + 4B ack_num + 4B dsack_num
	assign_short(p, SPEED_TEST_CHR_REPORT_LEN);
	skip_byte(p, sizeof(s16));

	// ack_num
	assign_uint(p, g_speed_test_max_seq_ack_num);
	skip_byte(p, sizeof(int));

	// dsack_num
	assign_uint(p, g_speed_test_dsack_num);
	skip_byte(p, sizeof(int));

	g_speed_test_max_seq_ack_num = COUNT_NUM_DEFAULT;
	g_speed_test_dsack_num = COUNT_NUM_DEFAULT;

	notifier((struct res_msg_head *)event);
}

void handle_speed_test_status(struct req_msg_head *msg)
{
	struct speed_test_status_msg *req = (struct speed_test_status_msg *)msg;

	if (msg->len < sizeof(struct speed_test_status_msg))
		return;

	g_speed_test_uid = req->uid;
	g_speed_test_status = req->status == 1 ? SPEED_TEST_STATUS_START : SPEED_TEST_STATUS_STOP;
	pr_info("[APP_ACC] %s: speed test staus(start 1 or stop 0) = %d\n",
		__func__, g_speed_test_status);

	if (g_speed_test_status == SPEED_TEST_STATUS_STOP) {
		pr_info("[APP_ACC] %s: speed test max seq ack num = %u, dsack num = %u\n",
			__func__, g_speed_test_max_seq_ack_num, g_speed_test_dsack_num);
		speed_test_chr_report();
	} else {
		g_speed_test_max_seq_ack_num = COUNT_NUM_DEFAULT;
		g_speed_test_dsack_num = COUNT_NUM_DEFAULT;
	}
}

void app_acc_process(struct req_msg_head *msg)
{
	if (!msg)
		return;

	pr_info("[APP_ACC] type :%d", msg->type);
	if (msg->type == APP_ACCELARTER_CMD) {
		struct app_acc_req_msg *req = (struct app_acc_req_msg *)msg;
		if (msg->len != sizeof(struct app_acc_req_msg))
			return;

		g_acc_uid = req->uid;
		if (req->threshold < BITS_PER_SECONDE_DEFAULT)
			g_acc_threshold = BITS_PER_SECONDE_DEFAULT;
		else
			g_acc_threshold = req->threshold;
		if (req->packet_threshold < STATICS_PACKET_NUM_INTERVAL)
			g_acc_packet_threshold = STATICS_PACKET_NUM_INTERVAL;
		else
			g_acc_packet_threshold = req->packet_threshold;
		if (req->cubic_trubo_init_win < CUBIC_TURBO_WIN_DEFAULT)
			g_acc_cubic_trubo_init_win = CUBIC_TURBO_WIN_DEFAULT;
		else
			g_acc_cubic_trubo_init_win = req->cubic_trubo_init_win;
		if (req->win_grow_fator_in_slow_start < WIN_GROW_FACTOR_DEFAULT)
			g_acc_win_grow_fator_in_slow_start = WIN_GROW_FACTOR_DEFAULT;
		else
			g_acc_win_grow_fator_in_slow_start = req->win_grow_fator_in_slow_start;

		pr_info("[APP_ACC] final threshold:%d packet threshold:%d"
			"cubic trubo init win:%u slow start win grow fator:%u",
			g_acc_threshold, g_acc_packet_threshold,
			g_acc_cubic_trubo_init_win, g_acc_win_grow_fator_in_slow_start);
	} else if (msg->type == SPEED_TEST_STATUS) {
		handle_speed_test_status(msg);
	} else {
		pr_err("unknown msg\n");
	}
}

int app_acc_get_uid()
{
	return g_acc_uid;
}

u32 app_acc_get_theshold()
{
	return g_acc_threshold;
}

u32 app_acc_cubic_trubo_init_win()
{
	return g_acc_cubic_trubo_init_win;
}

u32 app_acc_win_grow_fator_in_slow_start()
{
	return g_acc_win_grow_fator_in_slow_start;
}

bool app_acc_start_check(struct sock *sk)
{
	if (sk->start_acc_flag && (sk->sk_state != TCP_TIME_WAIT))
		return true; //lint !e1564
	else
		return false; //lint !e1564
}

void app_sock_acc_start_check(struct sock *sk, struct sk_buff *skb)
{
	unsigned long time_long;
	unsigned long bytes_speed;
	int uid;
	struct inet_sock *inet = NULL;

	if (!sk || (sk->sk_state == TCP_TIME_WAIT) || !skb)
		return;

	uid = (int)sk->sk_uid.val;
	if (app_acc_get_uid() != uid)
		return;

	if (sk->start_acc_flag)
		return;

	sk->packet_num++;
	if (sk->packet_num % STATICS_PACKET_NUM_INTERVAL)
		return;

	time_long = jiffies - sk->sk_born_stamp;
	/* zero indicates no time difference */
	if (time_long == 0)
		return;

	bytes_speed = (tcp_sk(sk)->bytes_received * HZ) / time_long;
	/* bps unit */
	bytes_speed = bytes_speed * BYTE_RATE_CONV_BIT_RATE;
	if (bytes_speed > app_acc_get_theshold()) {
		inet = inet_sk(sk);
		sk->start_acc_flag = true; //lint !e1564
		pr_info("[APP_ACC] start_acc_flag port:%d tcp.seq==%u",
			htons(inet->inet_sport), TCP_SKB_CB(skb)->seq);
	}
	pr_info("[APP_ACC] bytes_speed:%ld", bytes_speed);
}

bool app_ul_acc_start_check(struct sock *sk)
{
	int uid;

	if (!sk || !sk_fullsock(sk))
		return false;

	uid = (int)sk->sk_uid.val;
	if (app_acc_get_uid() == uid)
		return true;

	return false;
}

bool speed_test_chr_start_check(struct sock *sk)
{
	int uid;

	if (!sk || !sk_fullsock(sk))
		return false;

	uid = (int)sk->sk_uid.val;
	if ((g_speed_test_uid == uid)  && (g_speed_test_status == SPEED_TEST_STATUS_START))
		return true;

	return false;
}

void speed_test_chr_max_seq_ack_num_increase()
{
	g_speed_test_max_seq_ack_num++;
}

void speed_test_chr_dsack_num_increase()
{
	g_speed_test_dsack_num++;
}

void update_sock_send_win(struct sock *sk)
{
	struct tcp_sock *tp = tcp_sk(sk);
	if (app_ul_acc_start_check(sk))
		tp->snd_cwnd = app_acc_cubic_trubo_init_win();
}

u32 tcp_slow_start_cubic_turbo(struct sock *sk, u32 acked)
{
	struct tcp_sock *tp = tcp_sk(sk);
	u32 win_grow_factor = app_acc_win_grow_fator_in_slow_start();
	u32 cwnd = tp->snd_cwnd + acked * win_grow_factor;
	tp->snd_cwnd = min(cwnd, tp->snd_cwnd_clamp);

	acked = 0;
	return acked;
}
