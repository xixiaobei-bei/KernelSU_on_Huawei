/*
 * ext_sensorhub_route.h
 *
 * head file for external sensorhub route
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

#ifndef EXT_SENSORHUB_ROUTE_H
#define EXT_SENSORHUB_ROUTE_H

#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include "ext_sensorhub_channel.h"
#include "ext_sensorhub_api.h"

#define MAX_IDLE_READ_LEN 100
#define MAX_SUBSCMDS_COUNT   255

/* must be same with ext sensorhub channel */
enum port {
	ROUTE_SENSOR_PORT = 0x01,
	ROUTE_EXT_SENSORHUB_PORT = 0x02,
	ROUTE_FLP_PORT = 0x03,
	ROUTE_BT_PORT = 0x04,
	ROUTE_VIBRATOR_PORT = 0x05,
	ROUTE_UPGRADE_PORT = 0x06,
	ROUTE_CHARGE_PORT  = 0x07,
	ROUTE_DEBUG_PORT   = 0x08,
	ROUTE_MNTN_PORT = 0x09,
	ROUTE_AUDIO_PORT = 0x0a,
	ROUTE_NFC_PORT = 0x0b,
	ROUTE_AT_PORT = 0x0c,
	ROUTE_AR_PORT = 0x0d,
	ROUTE_BT_A_PORT = 0x0e,
	ROUTE_MOTION_PORT = 0x0f,
};

struct wait_resp {
	unsigned char		   service_id;
	unsigned char		   command_id;
	unsigned char		   *data;
	int			   data_len;
};

struct subs_cmds {
	unsigned char		   *service_ids;
	unsigned char		   *command_ids;
	unsigned short		   cmd_cnt;
};

/*
 * Every opened channel get one route record, related to file struct
 */
struct ext_sensorhub_route {
	unsigned short port;
	/* if wait cmd if null, do not wait resp */
	struct wait_resp *wait_resp;
	/* every route(channel) subscribe some cmds
	 * use loop to match route
	 */
	struct subs_cmds *subs_cmds;
	data_callback callback;
	struct ext_sensorhub_buf_list *read_buf_head;

	int max_read_list_len;
	int read_count;
	/* mutext for read_count */
	struct mutex read_count_lock;
	/* mutext for write */
	struct mutex write_lock;
	/* mutext for read */
	struct mutex read_lock;
	/* mutext for subscribe cmds */
	struct mutex sub_cmds_lock;
	/* mutext for wait resp */
	struct mutex wait_lock;
	wait_queue_head_t resp_wait; /* block send command when need resp */
	wait_queue_head_t read_wait; /* to block read when no data in buffer */
};

enum {
	MAX_READ_LIST_LEN_SENSOR = 20,
	MAX_READ_LIST_LEN_EXT_SENSORHUB = 30,
	MAX_READ_LIST_LEN_FLP = 20,
	MAX_READ_LIST_LEN_BT = 20,
	MAX_READ_LIST_LEN_VIBRATOR = 5,
	MAX_READ_LIST_LEN_UPGRADE = 10,
	MAX_READ_LIST_LEN_CHARGE = 5,
	MAX_READ_LIST_LEN_DEBUG = 10,
	MAX_READ_LIST_LEN_MNTN = 5,
	MAX_READ_LIST_LEN_AUDIO = 5,
	MAX_READ_LIST_LEN_NFC = 5,
	MAX_READ_LIST_LEN_AT = 3,
	MAX_READ_LIST_LEN_AR = 5,
	MAX_READ_LIST_LEN_BT_A = 3,
	MAX_READ_LIST_LEN_MOTION = 3,
};

struct ext_sensorhub_buf_list {
	struct list_head	list;

	unsigned char		   service_id;
	unsigned char		   command_id;
	unsigned int		read_cnt;
	unsigned char		*buffer;
};

/* for all kernel channels data */
struct sensorhub_data {
	/* all route(channel) get idle buffer */
	struct sensorhub_buf_list *idle_buf_head;

	/* for route(channel) list */
	struct sensorhub_route	*route_tbl;
};

struct ext_sensorhub_event_tbl {
	/* mutext for subscribe events */
	struct mutex events_lock;

	/* record each event callbacks count */
	int cb_cnt[EXT_SENSORHUB_MAX_EVENT];
	/* each event maybe contains multiple callbacks */
	event_callback * callbacks[EXT_SENSORHUB_MAX_EVENT];
	/* record each event callbacks tid */
	unsigned char *cb_tid[EXT_SENSORHUB_MAX_EVENT];
};

struct commu_state {
	int commu_cnt;
	char *name[COMMU_DRIVER_COUNT];
	bool handshake[COMMU_DRIVER_COUNT];
	int test_sync_ret[COMMU_DRIVER_COUNT];
};

struct commu_state *get_commu_state(void);

int ext_sensorhub_route_init(void);

void ext_sensorhub_route_exit(void);

ssize_t ext_sensorhub_route_read(unsigned short port, char __user *buf,
				 size_t count, unsigned char *service_id,
				 unsigned char *command_id);

ssize_t ext_sensorhub_route_write(unsigned short port,
				  struct ioctl_send_data *send_data);

void ext_sensorhub_route_append(struct ext_sensorhub_buf_list *resp_buf);

long ext_sensorhub_route_config(unsigned short port,
				unsigned char __user *service_ids,
				unsigned char __user *command_ids,
				unsigned short cmd_cnt);

int ext_sensorhub_route_reg_cb(unsigned short port,
			       unsigned char *service_ids,
			       unsigned char *command_ids,
			       unsigned short cmd_cnt,
			       data_callback callback);

int ext_sensorhub_route_unreg_cb(unsigned short port);

ssize_t ext_sensorhub_route_read_kernel(unsigned short port, char *buf,
					size_t count,
					unsigned char *service_id,
					unsigned char *command_id);

ssize_t ext_sensorhub_route_write_kernel(unsigned short port,
					 struct command *cmd,
					 bool need_resp,
					 struct cmd_resp *resp);

int ext_sensorhub_event_reg_cb(enum ext_sensorhub_event event,
			       unsigned char tid, event_callback callback);

int ext_sensorhub_event_unreg_cb(enum ext_sensorhub_event event,
				 unsigned char tid, event_callback callback);

void notify_event(enum ext_sensorhub_event event, unsigned char tid);

void ext_sensorhub_lock_try_relax(void);

#endif /* EXT_SENSORHUB_ROUTE_H */
