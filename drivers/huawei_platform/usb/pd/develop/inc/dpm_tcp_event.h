/*
 * dpm_tcp_event.h
 *
 * dpm tcp event
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

#ifndef _DPM_TCP_EVENT_H_
#define _DPM_TCP_EVENT_H_

#include "tcpm_pd.h"

#define PD_GMIDB_TARGET_PORT   0
#define PD_GMIDB_TARGET_BATTRY 1

struct dpm_tcp_event;
struct dpm_event;
struct tcpc_device;

typedef void (*dpm_tcp_event_cb)(struct tcpc_device *tcpc,
	struct dpm_tcp_event *event);

struct dpm_tcp_event_cb_data {
	dpm_tcp_event_cb event_cb;
};

struct dpm_tcp_pd_request {
	int mv;
	int ma;
};

struct dpm_tcp_pd_request_ex {
	uint8_t pos;
	union {
		uint32_t max;
		uint32_t max_uw;
		uint32_t max_ma;
	};
	union {
		uint32_t oper;
		uint32_t oper_uw;
		uint32_t oper_ma;
	};
};

struct dpm_tcp_dp_data {
	uint32_t val;
	uint32_t mask;
};

struct dpm_tcp_cust_vdm_data {
	bool wait_resp;
	uint8_t cnt;
	uint32_t vdos[PD_DATA_OBJ_SIZE];
};

struct dpm_tcp_svdm_data {
	uint16_t svid;
	uint8_t ops;
};

struct dpm_tcp_battery_capabilities {
	uint8_t bat_cap_ref;
};

struct dpm_tcp_battery_status {
	uint8_t bat_status_ref;
};

struct dpm_tcp_manufacturer_info {
	uint8_t info_target;
	uint8_t info_ref;
};

struct dpm_tcp_event {
	uint8_t event_id;
	uint8_t result;
	dpm_tcp_event_cb event_cb;
	union {
		struct dpm_tcp_pd_request pd_req;
		struct dpm_tcp_pd_request_ex pd_req_ex;
		struct dpm_tcp_dp_data dp_data;
		struct dpm_tcp_cust_vdm_data vdm_data;
		struct dpm_tcp_svdm_data svdm_data;
		struct dpm_tcp_battery_capabilities gbcdb;
		struct dpm_tcp_battery_status gbsdb;
		struct dpm_tcp_manufacturer_info gmidb;
		uint32_t index;
		uint32_t data_object[PD_DATA_OBJ_SIZE];
	} tcp_data;
};

uint8_t dpm_pop_tcp_event(struct dpm_event *devent);
struct dpm_tcp_event *dpm_get_curr_tcp_event(struct dpm_event *devent);

bool dpm_put_tcp_event(struct dpm_event *devent,
	const struct dpm_tcp_event *tcp_event);
bool dpm_tcp_event_empty(struct dpm_event *devent);

int dpm_put_event(struct dpm_event *devent, struct dpm_tcp_event *event);
int dpm_send_sync_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *event, uint32_t tout_ms,
	uint8_t *data, uint8_t size);
int dpm_send_async_tcp_event(struct dpm_event *devent,
	struct dpm_tcp_event *event,
	const struct dpm_tcp_event_cb_data *cb_data);

void dpm_clear_tcp_event(struct dpm_event *devent, uint8_t reason);
void dpm_reply_tcp_event_2nd(struct dpm_event *devent, int ret);
void dpm_reply_tcp_event_1st(struct dpm_event *devent, int ret);

#endif /* _DPM_TCP_EVENT_H_ */
