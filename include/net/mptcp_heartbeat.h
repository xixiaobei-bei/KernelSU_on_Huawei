/*
 * mptcp_heartbeat.h
 *
 * heartbeat for operator mp feature
 *
 * Copyright (c) 2012-2018 Huawei Technologies Co., Ltd.
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

#ifndef _MPTCP_HEARTBEAT_H_
#define _MPTCP_HEARTBEAT_H_

/* include headers */
#include <linux/in.h>
#include <linux/net.h>

/* macro */
#define LINK_NUM (2) // the total links availiable, currently 2, wifi and cellular
#define IF_NAME_LIST {"wlan0", "rmnet0"}

#define MAX_PAYLOAD_SIZE (128)
#define MAX_RETRY_COUNT (10) // a maximum retry count is necessary

/* struct definition */
struct detection_para {
	int retry_count;
	int timeout;    // for each retry
	char* payload;
	int payload_len;    // do NOT exceed MAX_PAYLOAD_SIZE
	void (*cb_fun)(int type, int result);
};

enum link_status {
	LINK_OK,
	LINK_BROKEN,
};

/* argment of function "heartbeat_trigger" may be a combination of the below */
enum link_types {
	WIFI_DETECTION = (1<<0),
	LTE_DETECTION = (1<<1),
};

/* function declaration */
void heartbeat_init(struct sockaddr_in* link_addrs);
void heartbeat_deinit(void);
void heartbeat_trigger(unsigned int type, struct detection_para* para);

#endif /* _MPTCP_HEARTBEAT_H_ */
