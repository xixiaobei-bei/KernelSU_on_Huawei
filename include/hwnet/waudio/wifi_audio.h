/*
 * wifi_audio.h
 *
 * wifi audio kernel module implementation
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

#ifndef _WIFI_AUDIO_H_
#define _WIFI_AUDIO_H_

#define WIFI_AUDIO_MODULE "wifi_audio_module"
/* netlink user initial value */
#define USER_ID_MAX 0xFFFFFFFF
#define NETLINK_MSG_DATA_LEN_MAX 128

enum netlink_msg_id {
	NETLINK_MSG_SET_USER_ID = 0,
	NETLINK_MSG_SET_CONFIG_INFOR,
	NETLINK_MSG_START_TIMESYNC,
	NETLINK_MSG_STOP_TIMESYNC,
	NETLINK_EVENT_REPORT_OFFSET_DELAY,
	NETLINK_EVENT_REPORT_TIME_SYNC,
	NETLINK_EVENT_REPORT_DATA_SYNC,
	NETLINK_EVENT_REPORT_SYNC_OFFSET,
	NETLINK_EVENT_REPORT_NET_DEV,
	NETLINK_MSG_ID_INVALID,
};

struct netlink_data {
	int id;
	unsigned int msg_len;
	char msg_data[NETLINK_MSG_DATA_LEN_MAX];
};

int netlink_event_report(const struct netlink_data *event);
#endif /* _WIFI_AUDIO_H_ */
