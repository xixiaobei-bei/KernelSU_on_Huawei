/*
 * dpm_event.h
 *
 * dpm event
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _DPM_EVENT_H_
#define _DPM_EVENT_H_

#include <linux/mutex.h>
#include <linux/wait.h>
#include "inc/dpm_tcp_event.h"

#define DPM_TCP_EVENT_SIZE         4
#define DPM_TCP_EVENT_SEND_RETRY   3

struct pd_port;
struct tcpc_device;

struct dpm_event {
	struct pd_port *port;
	struct mutex tcp_event_lock;
	struct mutex tcp_sync_lock;
	wait_queue_head_t tcp_wait_queue;
	bool tcp_drop_once;
	bool tcp_wait_done;
	void *tcp_bk_data;
	uint8_t tcp_bk_size;
	uint8_t tcp_bk_ret;
	uint8_t tcp_wait_id;
	uint8_t tcp_event_1st_id;
	uint8_t tcp_event_2nd_id;
	uint8_t tcp_event_count;
	uint8_t tcp_event_head_index;
	struct dpm_tcp_event tcp_event_buf[DPM_TCP_EVENT_SIZE];
	struct dpm_tcp_event tcp_curr_event;
};

int dpm_event_init(struct pd_port *port);

#endif /* _DPM_EVENT_H_ */
