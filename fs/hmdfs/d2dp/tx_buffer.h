/* SPDX-License-Identifier: GPL-2.0 */
/*
 * D2D TX buffer internal structures
 */

#include <linux/types.h>

#include "dynamic_list.h"
#include "wrappers.h"

enum txbuf_node_state {
	D2D_NODE_STATE_EMPTY,
	D2D_NODE_STATE_UNSENT,
	D2D_NODE_STATE_SENT,
	D2D_NODE_STATE_SENDING,
	D2D_NODE_STATE_ACKNOWLEDGED,
	D2D_NODE_STATE_NEED_RESEND,
};

struct tx_buffer {
	struct dlist *dlist;
	struct dlist_node *first_to_send;
	wrap_t tx_seq_id;
	bool flowcontrol_mode;
};
