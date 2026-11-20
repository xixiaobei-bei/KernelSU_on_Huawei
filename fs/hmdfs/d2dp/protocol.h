/* SPDX-License-Identifier: GPL-2.0 */
/* D2D protocol main header */

#ifndef D2D_PROTOCOL_H
#define D2D_PROTOCOL_H

#include <linux/atomic.h>
#include <linux/completion.h>
#include <linux/if_ether.h>
#include <linux/kobject.h>
#include <linux/mutex.h>
#include <linux/net.h>
#include <linux/sched.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include "d2d.h"
#include "buffer.h"
#include "headers.h"
#include "timers.h"
#include "wrappers.h"

/**
 * struct d2d_options - structure for keeping D2DP customizable options
 * @timeout: RX timeout time
 *
 * These options are being get/set via d2d_get_opt/d2d_set_opt
 */
struct d2d_options {
	atomic_t timeout;
	struct mutex opt_mtx;
};

/**
 * struct net_frames - structure used to keep preallocated networking frames
 * @crypto_tx_mtx: protect TX keys against updating from the caller layer
 * @crypto_rx_mtx: protect RX keys against updating from the caller layer
 * @tx_frame: network frame for TX application data (plaintext)
 * @rx_frame: network frame for RX application data (plaintext)
 * @tx_encr_frame: network frame for TX application data (encrypted)
 * @rx_encr_frame: network frame for RX application data (encrypted)
 * @len: allocated size for frames
 *
 * When encryption is used, then @tx_encr_frame/@rx_encr_frame are used for
 * direct kernel_sengmsg/kernel_recvmsg, and @tx_frame/@rx_frame are used for
 * keeping temporary plaintext data. Otherwise, when encryption is not used,
 * @tx_frame/@rx_frame are used for both data and for send/recv operations.
 */
struct net_frames {
	/* to protect TX crypto operations from updating keys */
	struct mutex crypto_tx_mtx;
	/* to protect RX crypto operations from updating keys */
	struct mutex crypto_rx_mtx;
	void *tx_frame;
	void *rx_frame;
	void *tx_encr_frame;
	void *rx_encr_frame;
	size_t len;
};

struct ack_data {
	bool ack_ready;
	struct d2d_header ack_header;
};

struct tx_ack_state {
	wrap_t last_acked;
	bool last_limit_exceeded;
	atomic_t force_ack;
	struct ack_data acks[2];
	struct ack_data *ack_to_send;
	struct ack_data *ack_to_fill;
};

struct tx_handler {
	/*
	 * To protect TX handler against simultaneous usage:
	 * - TX transport thread
	 * - D2DP sending API
	 * - ACK/RTO timers
	 */
	spinlock_t lock;
	struct tx_buffer *buffer;
	struct wait_queue_head wq_free;
	struct wait_queue_head wq_data;
	struct d2d_header tx_header;
	struct tx_ack_state ack_state;
	atomic_t key_update;
	wrap_t   key_update_id;
};

struct rx_handler {
	/*
	 * To protect RX handler against simultaneous usage:
	 * - RX transport thread
	 * - D2DP receiving API
	 * - ACK timer
	 */
	spinlock_t lock;
	struct rx_buffer *buffer;
	struct wait_queue_head wq_data;
	struct d2d_header rx_header;
};

struct timers {
	struct workqueue_struct *timer_wq;
	struct d2d_timer ack_timer;
	struct d2d_timer rto_timer;
};

struct d2dp_thread_state {
	struct completion exit;
	struct task_struct *task;
};

/**
 * struct d2dp_transport_threads - transport threads bookkeeping
 *
 * This structure is used to carefully close the threads as they may exit at any
 * time. The pointers to transport tasks are protected by the lock and the
 * completions are used to await the threads.
 */
struct d2dp_transport_threads {
	/* this lock is used to synchronize [tr]x_thread exit */
	struct mutex exit_completion_lock;
	struct d2dp_thread_state tx;
	struct d2dp_thread_state rx;
};

/**
 * struct d2dp_stats - D2D Protocol statistics
 *
 * This structure is used mostly for profiling and tuning. The fields are
 * designed to be exposed via kobject interface.
 *
 * @tx_bytes: how many bytes are sent via D2DP
 * @tx_packets: how many packets are sent via D2DP
 * @rx_bytes: how many bytes are received via D2DP
 * @rx_packets: how many packets are received via D2DP
 * @rx_buf_bytes: how many bytes are stored in RX buffer now
 * @tx_buf_bytes: how many bytes are stored in TX buffer now
 * @tx_wait_switches: amount of TX thread switches performed
 * @tx_resend_bytes: amount of retransmission bytes TX thread sent
 * @rx_sacks_arrived: how many SACKs got
 * @rx_dups_arrived: how many duplicates got
 * @rx_overflow_arrived: how many packets dropped due to RX buffer overflow
 * @rto_timer_fired: how many times the RTO timer fired
 * @rx_drops_badhdr: packets dropped due to incorrect header
 * @rx_drops_decrypt: packets dropped due to failed decrypt
 * @rx_drops_pktid: packets dropped due to bad packet id (replay or reorder)
 * @rx_drops_trunc: packets dropped due to being longer than MTU and truncated
 * @rx_drops_empty: packets dropped due to being zero-length
 *
 * Note that most of the values are just performance counters, and are modified
 * using relaxed ordering. The exceptions are @tx_buf_bytes and @rx_buf_bytes,
 * which represent actual TX/RX buffer states, so they are modified under the
 * corresponding TX/RX buffer locks to achieve serialization.
 */
struct d2dp_stats {
	atomic64_t tx_bytes;
	atomic64_t tx_packets;
	atomic64_t rx_bytes;
	atomic64_t rx_packets;
	atomic_t tx_buf_bytes;
	atomic_t rx_buf_bytes;
	atomic_t tx_wait_switches;
	atomic_t tx_resend_bytes;
	atomic_t rx_sacks_arrived;
	atomic_t rx_dups_arrived;
	atomic_t rx_overflow_arrived;
	atomic_t rto_timer_fired;
	atomic_t rx_drops_badhdr;
	atomic_t rx_drops_decrypt;
	atomic_t rx_drops_pktid;
	atomic_t rx_drops_trunc;
	atomic_t rx_drops_empty;
};

/**
 * struct d2d_protocol - D2D Protocol instance
 *
 * This structure is used to keep all D2DP-related stuff. It owns all the data
 * except @sock and @security, which are immutably borrowed from the caller
 * layer.
 *
 * @kobj: kernel object base class
 * @sock: immutably borrowed UDP socket in connected state
 * @options: parameters which can be set/get using d2d_get_opt/d2d_set_opt
 * @flags: global flags for all the protocol
 * @tx_packet_id: global protocol's counter for sent packet
 * @rx_packet_id: global protocol's counter for received packets
 * @security: immutably borrowed security settings from upper layer
 * @frames: pre-allocated storage for networking frames
 * @tx: TX transport related stuff
 * @rx: RX transport related stuff
 * @timers: timer-related stuff (ACK, RTO, workqueue)
 * @transport: transport threads bookkeeping
 * @closing: atomic message to distinguish graceful protocol close
 * @exitcode: atomic one-shot field to track the first protocol error
 * @stats: the protocol statistics collection
 */
struct d2d_protocol {
	struct kobject kobj;
	struct socket *sock;
	struct d2d_options options;
	u32 flags;
	u64 tx_packet_id;
	u64 rx_packet_id;
	struct d2d_security *security;
	struct net_frames frames;
	struct tx_handler tx;
	struct rx_handler rx;
	struct timers timers;
	struct d2dp_transport_threads transport;
	atomic_t closing;
	atomic_t exitcode;
	struct d2dp_stats stats;
};

/**
 * enum send_wakeup_reason - internal wakeup reason for d2d_send* waitings
 *
 * The OK result means that the corresponding TX lock is acquired. Otherwise the
 * lock is not acquired.
 *
 * @OK: TX buffer is OK, we can continue sending activity
 * @PROTO_ERROR: the protocol failed with some internal error
 * @INVALID_SIZE: input argument for d2d_send* is too big
 * @ERESTARTSYS: wake up by some signal
 * @CLOSING: `d2d_protocol_close` is called
 */
enum send_wakeup_reason {
	D2D_SEND_WAKEUP_REASON_OK,
	D2D_SEND_WAKEUP_REASON_PROTO_ERROR,
	D2D_SEND_WAKEUP_REASON_INVALID_SIZE,
	D2D_SEND_WAKEUP_REASON_ERESTARTSYS,
	D2D_SEND_WAKEUP_REASON_CLOSING,
};

/**
 * enum recv_wakeup_reason - internal wakeup reason for d2d_recv* waitings
 *
 * The OK result means that the corresponding RX lock is acquired. Otherwise the
 * lock is not acquired.
 *
 * @OK: we have some packets for the receiver, can grab it
 * @PROTO_ERROR: the protocol failed with some internal error
 * @INVALID_SIZE: input argument for d2d_recv* is too big
 * @ERESTARTSYS: wake up by some signal
 * @TIMEOUT: no packets for a long time (when D2D_OPT_RCVTIMEO is set)
 * @CLOSING: `d2d_protocol_close` is called
 */
enum recv_wakeup_reason {
	D2D_RECV_WAKEUP_REASON_OK,
	D2D_RECV_WAKEUP_REASON_PROTO_ERROR,
	D2D_RECV_WAKEUP_REASON_INVALID_SIZE,
	D2D_RECV_WAKEUP_REASON_ERESTARTSYS,
	D2D_RECV_WAKEUP_REASON_TIMEOUT,
	D2D_RECV_WAKEUP_REASON_CLOSING,
};

void d2dp_wake_up_transport(struct d2d_protocol *proto);
void d2d_protocol_destroy(struct d2d_protocol *proto);

#endif /* D2D_PROTOCOL_H */
