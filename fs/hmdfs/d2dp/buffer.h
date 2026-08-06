/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D TX/RX buffer interface
 */

#ifndef D2D_BUFFER_H
#define D2D_BUFFER_H

#include <linux/types.h>
#include <linux/net.h>

#include "sack.h"
#include "wrappers.h"

/* forward declarations */
struct tx_buffer;
struct rx_buffer;

/* This structures are used to communicate with D2D buffers */
struct d2d_data_packet {
	void   *data;
	size_t len;
	wrap_t seq_id;
	bool   resend;
};

/* TX operations */
struct tx_buffer *tx_buf_alloc(size_t bufsize, size_t elem_size);
void tx_buf_free(struct tx_buffer *txb);
bool tx_buf_has_free_space(const struct tx_buffer *txb);
size_t tx_buf_get_free_bytes(const struct tx_buffer *txb);
size_t tx_buf_append_iov(struct tx_buffer *txb, const struct kvec *iov,
			 size_t iovlen, size_t to_append);
bool tx_buf_get_last_seq_id(const struct tx_buffer *txb, wrap_t *res);
bool tx_buf_has_packets_to_send(const struct tx_buffer *txb);
void tx_buf_peek_packet(struct tx_buffer *txb, struct d2d_data_packet *packet);
int tx_buf_peek_return(struct tx_buffer *txb, struct d2d_data_packet *packet);
size_t tx_buf_process_ack(struct tx_buffer *txb, uint32_t ack_id,
			  const struct sack_pairs *pairs);
void tx_buf_mark_as_need_resent(struct tx_buffer *txb);
bool tx_buf_is_empty(struct tx_buffer *txb);
void tx_buf_set_flowcontrol_mode(struct tx_buffer *txb, bool mode);
size_t tx_buf_get_size(struct tx_buffer *txb);

/* RX operations */
struct rx_buffer *rx_buf_alloc(size_t bufsize, size_t elem_size);
void rx_buf_free(struct rx_buffer *rxb);
bool rx_buf_has_data(const struct rx_buffer *rxb);
size_t rx_buf_get_available_bytes(const struct rx_buffer *rxb);
size_t rx_buf_get(struct rx_buffer *rxb, void *data, size_t len);

/**
 * enum rx_buf_put_result - the result of `rx_buf_put` operation
 * @OK: the packet has been successfully inserted to RXB
 * @OVERFLOW: there are no free space in RXB to place the new packet
 * @CONSUMED: the reader has already consumed the data with this seq_id
 * @DUPLICATE: there is exactly the same packet in the RXB already
 * @BAD_PACKET: this packet is malformed or the RX buffer is probably corrupted
 */
enum rx_buf_put_result {
	RX_BUF_PUT_OK,
	RX_BUF_PUT_OVERFLOW,
	RX_BUF_PUT_CONSUMED,
	RX_BUF_PUT_DUPLICATE,
	RX_BUF_PUT_BAD_PACKET,
};

int rx_buf_put(struct rx_buffer *rxb, const struct d2d_data_packet *packet);
uint32_t rx_buf_get_ack(const struct rx_buffer *rxb);
void rx_buf_generate_sack_pairs(const struct rx_buffer *rxb,
				struct sack_pairs *s_pairs, size_t len);
size_t rx_buf_get_fill_percentage(const struct rx_buffer *rxb);
size_t rx_buf_get_size(const struct rx_buffer *rxb);

#endif /* D2D_BUFFER_H */
