/* SPDX-License-Identifier: GPL-2.0 */
#ifndef D2D_RX_BUFFER_H
#define D2D_RX_BUFFER_H

#include <linux/types.h>

#include "dynamic_list.h"

struct rx_buffer {
	struct dlist *dlist;
	u32 reader_pos;
	size_t node_read_offset;
	struct dlist_node *last_sequential_node;
	size_t available_bytes;
};

#endif /* D2D_RX_BUFFER_H */
