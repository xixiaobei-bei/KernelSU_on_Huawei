/** @file
 * Copyright (c) 2020-2020, Dkmdlicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef DKMD_TIMELINE_LISTENER_H
#define DKMD_TIMELINE_LISTENER_H

#include <linux/slab.h>
#include <linux/types.h>


/* each timeline maybe have a lot of listenerserver */
struct dkmd_timeline_listener {
	struct list_head list_node;

	uint64_t pt_value;
	void *listener_data;
	const struct dkmd_timeline_listener_ops *ops;
};

struct dkmd_timeline_listener_ops {
	const char* (*get_listener_name)(struct dkmd_timeline_listener *listener);
	int (*enable_signaling)(struct dkmd_timeline_listener *listener);
	int (*disable_signaling)(struct dkmd_timeline_listener *listener);
	bool (*is_signaled)(struct dkmd_timeline_listener *listener, uint32_t tl_val);
	int (*handle_signal)(struct dkmd_timeline_listener *listener);
	void (*release)(struct dkmd_timeline_listener *listener);
};


#endif /* DKMD_DISP_TIMELINE_LISTENER_H */
