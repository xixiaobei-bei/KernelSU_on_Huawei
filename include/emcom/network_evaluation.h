/*
* Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
* Description: evaluate corresponding network by collected RTTs
* Author: dengyu davy.deng@huawei.com
* Create: 2018-02-13
*/

#ifndef _NETWORK_EVALUATION_H_
#define _NETWORK_EVALUATION_H_

#include <net/sock.h>
#include "evaluation_common.h"

/* misc values */
#define ADEQUATE_RTT 8

typedef struct {
	int8_t traffic_index;
	int8_t rtt_stability_index;
	int8_t reserved_1; // for alignment
	int8_t reserved_2;
} sub_indices_report;

void nweval_init(void);
void nweval_update_rtt(struct sock* sk, long rtt_us);
void nweval_start_rtt_report(int8_t network_type);
void nweval_stop_rtt_report(void);
void nweval_trigger_rtt_report(int32_t signal_strength);
void nweval_event_process(int32_t event, uint8_t *pdata, uint16_t len);
void nweval_on_dk_connected(void);

#endif // _NETWORK_EVALUATION_H_
