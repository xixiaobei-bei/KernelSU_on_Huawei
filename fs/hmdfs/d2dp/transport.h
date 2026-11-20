/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Transport for D2D protocol
 */

#ifndef D2D_TRANSPORT_H
#define D2D_TRANSPORT_H

#include <linux/types.h>

/**
 * enum tx_wake_reasson - internal wakeup reason for TX transport thread
 *
 * @OK: TX thread has something to send (data packet or acknowledgement)
 * @PROTO_ERROR: the protocol failed with some internal error
 * @CLOSING: `d2d_protocol_close` is called, we have to shutdown now
 * @ERESTARTSYS: wake up by some signal
 */
enum tx_wake_reason {
	TX_WAKE_REASON_OK,
	TX_WAKE_REASON_PROTO_ERROR,
	TX_WAKE_REASON_CLOSING,
	TX_WAKE_REASON_ERESTARTSYS,
};

/**
 * enum rx_wake_reasson - internal wakeup reason for RX transport thread
 *
 * @OK: RX thread has a good data from the network
 * @EMPTY_PACKET: got zero-length packet, ignore it
 * @TRUNCATED_PACKET: got oversized packet, ignore it
 * @PROTO_ERROR: the protocol failed with some internal error
 * @CLOSING: `d2d_protocol_close` is called, we have to shutdown now
 * @ERESTARTSYS: wake up by some signal
 */
enum rx_wake_reason {
	RX_WAKE_REASON_OK,
	RX_WAKE_REASON_EMPTY_PACKET,
	RX_WAKE_REASON_TRUNCATED_PACKET,
	RX_WAKE_REASON_PROTO_ERROR,
	RX_WAKE_REASON_CLOSING,
	RX_WAKE_REASON_ERESTARTSYS,
};

bool ack_timer_action(void *proto);
bool rto_timer_action(void *proto);
int tx_transport_thread(void *proto);
int rx_transport_thread(void *proto);

#endif /* D2D_TRANSPORT_H */
