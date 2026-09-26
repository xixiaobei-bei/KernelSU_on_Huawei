/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description: Supply a feature to track the specific tcp sockets and record
 *              the socket statistics to files in /proc/net/tcp_socket.
 * Author: zhangge
 * Create: 2020-07-17
 */

#ifndef _HW_TRACK_TCP_SOCKET
#define _HW_TRACK_TCP_SOCKET

#include <linux/types.h>
#include <net/sock.h>

#define MAX_TRACK_SOCKET_NUM	20
#define MAX_PROC_NAME_LEN	32
#define DELETE_EXPIRE_TIME	1000 // ms

#define ROLE_ALL	0 // default.
#define ROLE_PRODUCER	1 // means this socket is used to send data.
#define ROLE_CONSUMER	2 // means this socket is used to redv data.

#define START_TRACK_SOCKET	0x1
#define STOP_TRACK_SOCKET	0x2
#define START_RECORD_STATS	0x4

#define SUB_CMD_DEFAULT		0x0
#define SUB_CMD_RTT		(0x1 << 1)
#define SUB_CMD_RTT_MIN		(0x1 << 2)
#define SUB_CMD_RTT_MAX		(0x1 << 3)
#define SUB_CMD_SEND		(0x1 << 4)
#define SUB_CMD_RESEND		(0x1 << 5)
#define SUB_CMD_RECV		(0x1 << 6)
#define SUB_CMD_WINDOW		(0x1 << 7)
#define SUB_CMD_STATE		(0x1 << 8)
#define SUB_CMD_SYNC_SEND	(0x1 << 9)
#define SUB_CMD_SYNC_RECV	(0x1 << 10)
#define SUB_CMD_FIN_SEND	(0x1 << 11)

enum stats_cb_state {
	IDLE,
	WORKING,
	WAITING_TO_DELETE,
};

enum stats_cb_used_state {
	CB_IDLE,
	CB_USED_WITH_GLOBAL_TIMER,
	CB_USED_WITH_SELF_TIMER,
};

/**
 *  struct track_socket_value - command structure used to trigger socket
 *  tracking actions by users
 *
 *  @fd: socket file descriptor, the user MUST assign a value
 *  @app_name: app's name, user MUST assign a unique name
 *  @name_size: the length of app name, user MUST assign it
 *  @cmd_type: app MUST assign a value to trigger kernel actions
 *  @value: socket optional cmmand value
 *  @role: C/S role, and the default is ROLE_ALL. It is optional for app
 *  @sub_cmd_bits: ask kernel to record specific values according to those bits
 *  @level: level of record nums and frequences
 */
struct track_socket_value {
	s16		fd;
	char		app_name[TASK_COMM_LEN];
	u32		name_size;
	u32		cmd_type;
	union {
		u32	value;
		struct {
			u32	role		: 4;
			u32	sub_cmd_bits	: 16;
			u32	level		: 8;
			u32	pad		: 4;
		}	sub_cmds;
	}	cmd_value;
};

/**
 *  struct socket_info - information of the tracked socket used in kernel
 *  @sk: the socket's sock
 *  @fd: socket file descriptor
 *  @uid: the uid of this process
 *  @role: C/S role
 *  @app_name: app's name
 *  @local_port: the local port of the socket
 *  @remote_port: the remote port of the socket
 */
struct socket_info {
	struct sock	*sk;
	s32	fd;
	u32	uid;
	u32	role;
	char	app_name[TASK_COMM_LEN];
	u16	sub_cmd_bits;
	u16	local_port;
	u16	remote_port;
};

/**
 *  struct socket_period_stats - period socket statistics data.
 */
struct socket_period_stats {
	u32	rtt_min;
	u32	rtt_max;
	u32	rtt;
	u32	tx_packets;
	u32	re_tx_packets;
	u32	rx_packets;
	u32	in_resend_pkts;
};

/**
 *  struct socket_realtime_stats - the unrepeatable socket data.
 */
struct socket_realtime_stats {
	u32	socket_state;
	u32	max_send_seq;
	u32	sync_send_num;
	u32	sync_recv_num;
	u32	window_size;
};

/**
 *  struct single_socket_stats - the realtime socket data, used in netfilter
 */
struct single_socket_stats {
	struct socket_realtime_stats	realtime_stats;
	struct socket_period_stats		period_stats;
};

/**
 *  struct socket_stats_saved - history statistics, showed in
 *  proc/net/tcp_socket
 */
struct socket_stats_saved {
	struct socket_realtime_stats	realtime_stats;
	struct socket_period_stats	*first, *cur;
	struct socket_period_stats	*head, *end;
	struct socket_period_stats	period_stats[0];
};

/**
 *  struct socket_stats_cb - the collection of all data about the socket.
 *  @socket_info: the tracked socket's information
 *  @socket_stats: the realtime record of the socket traffic
 *  @sk_list: list of all sockets belonging to the same app
 *  @stats_saved: history statistics
 *  @jcycle: self record timer expiration time
 *  @timer: used to record history data
 *  @proc_node: the proc node address
 */
struct socket_stats_cb {
	spinlock_t	lock; /* spinlock for this stats_cb */
	struct socket_info	socket_info;
	struct single_socket_stats	socket_stats;
	struct list_head	sk_list;
	struct socket_stats_saved	*stats_saved;
	u32	jcycle;
	struct timer_list	timer;
	struct proc_list	*proc_node;
	u32	state;
};

/**
 *  struct proc_list - the proc node information.
 *  @proc_name: app_xxx, and "xxx" is the same with the app_name
 *              in track_socket_value
 *  @proc_dir_entry: record the proc node entry
 *  @list: link of all proc nodes
 *  @sk_head: link all socket stats CBs belonging to the same app
 */
struct proc_list {
	char proc_name[MAX_PROC_NAME_LEN];
	struct proc_dir_entry *proc_entry;
	struct list_head list;
	struct list_head sk_head;
};

/**
 *  struct tcp_socket_stats - the global entry of this feature
 */
struct tcp_socket_stats {
	spinlock_t	lock; /* spinlock for all stats_cbs */
	struct socket_stats_cb	*stats_cb;
	s8 used[MAX_TRACK_SOCKET_NUM];
	u32	cnt;
	u32	jcycle;
	struct timer_list	timer;
	struct proc_dir_entry *proc_parent;
	struct list_head proc_head;
	struct mutex proc_mutex; /* mutex for all proc nodes */
	struct timer_list	del_timer;
};

/**
 *  struct record_period_para - parameters used by the timer to
 *  record history statistics
 *
 *  @cycle: cycle time of the timer
 *  @cnt: the MAX group num of period statistics
 */
struct record_period_para {
	u32	cycle;
	u32	cnt;
};

/* Process user's command. */
int process_track_socket_cmd(struct sock *sk, const char __user *optval,
			     int optlen);

/* clear the socket's records when the socket is closed. */
void release_tcp_socket_tracking(struct sock *sk);

#endif /* _HW_TRACK_TCP_SOCKET */
