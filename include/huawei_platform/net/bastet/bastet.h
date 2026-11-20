/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Bastet head file.
 * Author: zhuweichen@huawei.com
 * Create: 2014-09-01
 */

#ifndef _BASTET_H
#define _BASTET_H

#include <linux/netfilter/x_tables.h>

#include "bastet_dev.h"

struct bastet_reconn {
	bool enable;
	bool renew;
	bool flag;
	bool auto_connect;
	bool err_close;
	int old_fd;
	int proxy_id;
	struct socket *orig_sock;
	wait_queue_head_t wq;
	atomic_t handover;
};

/*
 * struct bastet_hbm - bastet heartbeat message structer.
 * @reply_offset:heartbeat reply has received offset length
 * @reply_len:heartbeat reply length
 * @reply_content:heartbeat reply content
 * @reply_filter_cnt:heartbeat reply start filter counter
 * @reply_matched_cnt:heartbeat reply matched counter
 * @frozen:process frozen state
 * @hbm_lock:heartbeat message lock of this unit.
 *
 * used to record the bastet heartbeat message information.
 */
struct bastet_hbm {
	int32_t reply_offset;
	uint32_t reply_len;
	uint8_t *reply_content;
	atomic_t reply_filter_cnt;
	atomic_t reply_matched_cnt;
	atomic_t frozen;
	spinlock_t hbm_lock;
};

struct bastet_sock {
	u8 bastet_sock_state;
	u8 bastet_timer_event;
	u8 user_ctrl;

	int fd;
	pid_t pid;
	int32_t proxy_id;

	unsigned long bastet_timeout;
	struct timer_list bastet_timer;
	unsigned long delay_sync_time_section;
	unsigned long last_sock_active_time_point;

	bool sync_retry;
	struct bst_sock_sync_prop *sync_p;

	struct sk_buff_head recv_queue;
	int recv_len;

	int need_repair;
	wait_queue_head_t wq;
	bool flag;
	bool is_wifi;
	struct bastet_hbm hbm;
};

void bastet_sock_seq_num_invalid(struct sock *sk);
bool bastet_sock_send_prepare(struct sock *sk);
bool bastet_sock_recv_prepare(struct sock *sk, struct sk_buff *skb);
void bastet_sock_release(struct sock *sk);
void bastet_delay_sock_sync_notify(struct sock *sk);
bool bastet_sock_repair_prepare_notify(struct sock *sk, int val);
unsigned char *bastet_save_uid_info(const char *table_name,
	struct xt_table_info *newinfo, int *buf_len);
void bastet_indicate_uid_info(unsigned char *buf, int buf_len);
void bastet_wait_traffic_flow(void);
int bastet_send_priority_msg(struct sock *sk, struct msghdr *msg, size_t size);
void bastet_reconn_config(struct sock *sk, int val);
void bastet_reconn_init(void);
void bastet_inet_release(struct sock *sk);
int bastet_block_recv(struct socket *sock, struct msghdr *msg,
	size_t size, int flags, int *addr_len, int err);
int bastet_check_reconn(struct socket *sock);
void bastet_check_partner(const struct sock *sk, int state);
int bastet_check_hb_reply(struct sock *sk, struct msghdr *msg, int err);
void bastet_clear_hb_reply(struct bastet_sock *bsk);
void bastet_mark_hb_reply(struct sock *sk, struct sk_buff *skb, int len);
#ifdef CONFIG_NETFILTER_XT_MATCH_QTAGUID
/* declare these functions to call the system update flow function */
extern void bastet_update_if_tag_stat(const char *ifname,
	uid_t uid, const struct sock *sk,
	enum ifs_tx_rx direction, int proto, int bytes);
extern int bastet_update_total_bytes(const char *dev_name,
	int proto, unsigned long tx_bytes, unsigned long rx_bytes);
#endif
#endif /* _BASTET_H */
