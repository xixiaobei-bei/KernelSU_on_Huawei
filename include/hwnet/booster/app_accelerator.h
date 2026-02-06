/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file is the internal header file for the
 *              app accelerator module.
 * Author: youpeigang@huawei.com
 * Create: 2020-01-10
 */

#ifndef APP_ACCELERATOR_H
#define APP_ACCELERATOR_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/netfilter.h>
#include <linux/timer.h>
#include <uapi/linux/netfilter.h>
#include "netlink_handle.h"
#include "ip_para_collec.h"

#define UID_INVALID (-1)
#define MAX_TCP_ACC_LEN 12

#define BITS_PER_SECONDE_DEFAULT (2048 * 1024 * 8)
#define STATICS_PACKET_NUM_INTERVAL 50
#define SPEED_TEST_UID 0
#define BYTE_RATE_CONV_BIT_RATE 8 /* Convert byte rate to bit rate */
#define CUBIC_TURBO_WIN_DEFAULT 60
#define WIN_GROW_FACTOR_DEFAULT 3

#define SPEED_TEST_STATUS_STOP 0
#define SPEED_TEST_STATUS_START 1
#define COUNT_NUM_DEFAULT 0
#define SPEED_TEST_CHR_REPORT_LEN 12


/* Notification request issued by the upper layer is defined as: */
struct app_acc_req_msg {
	/* Event enumeration values */
	u16 type;
	/* The length behind this field, the limit lower 8 */
	u16 len;
	u32 uid;
	u32 threshold;
	u32 packet_threshold;
	u32 cubic_trubo_init_win;
	u32 win_grow_fator_in_slow_start;
};

struct speed_test_status_msg {
	/* Event enumeration values */
	u16 type;
	/* The length behind this field, the limit lower 8 */
	u16 len;
	u32 uid;
	u32 status;
};


msg_process *app_acc_init(notify_event *fn);
void app_acc_process(struct req_msg_head *msg);
int app_acc_get_uid(void);
u32 app_acc_get_theshold(void);
u32 app_acc_cubic_trubo_init_win(void);
u32 app_acc_win_grow_fator_in_slow_start(void);
void app_acc_report(u32 disorder_packets, u32 total_packets,
	u32 dicard_bytes, u32 total_bytes);
bool app_acc_start_check(struct sock *sk);
void app_sock_acc_start_check(struct sock *sk, struct sk_buff *skb);
bool app_ul_acc_start_check(struct sock *sk);
bool speed_test_chr_start_check(struct sock *sk);
void speed_test_chr_max_seq_ack_num_increase(void);
void speed_test_chr_dsack_num_increase(void);
void update_sock_send_win(struct sock *sk);
u32 tcp_slow_start_cubic_turbo(struct sock *sk, u32 acked);
#endif
