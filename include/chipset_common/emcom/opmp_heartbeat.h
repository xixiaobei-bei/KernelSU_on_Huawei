/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
* Description: opmp heartbeat head file
* Author: jiangwenhao jiangwenhao1@huawei.com
* Create: 2019-01-04
*/
#ifndef _OPMP_HEARTBEAT_H_
#define _OPMP_HEARTBEAT_H_

#include <linux/types.h>

/* heartbeat packet */
struct mutp_header_packet {
	uint8_t icheck : 1,
		spare : 4,
		version : 3;
	uint8_t type;
	uint16_t len;
	uint32_t tunnel_id;
};

/* misc values */
#define DEFAULT_PERIOD 300 // in seconds
#define DEFAULT_TIMEOUT 5
#define DEFAULT_RETRY_COUNT 3
#define TRAFFIC_FILE "/proc/net/xt_qtaguid/iface_stat_fmt"
#define HEARTBEAT_REQUEST_CODE 11
#define HEARTBEAT_RESPONSE_CODE 12
#define HEARTBEAT_ERROR_INDICATION 10
#define HEARTBEAT_PAYLOAD_LENGTH 8

struct heartbeat_init_info {
	int wifi_addr;
	int lte_addr;
	int port;
	int tunnel_id;
	int interval;
	int count;
	int period;
};

struct heartbeat_report {
	int type;
	int result;
};

void opmp_event_process(int32_t event, uint8_t *pdata, uint16_t len);

#endif // OPMP_HEARTBEAT_H_
