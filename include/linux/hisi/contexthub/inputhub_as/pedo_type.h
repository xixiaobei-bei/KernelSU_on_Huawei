/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description:ar_type.h
 * Create:2020.03.06
 */
#ifndef _PEDO_TYPE_H_
#define _PEDO_TYPE_H_

#include <linux/types.h>
#include "protocol_as.h"

enum pedo_func_type {
	PEDO_STEP_DETECTOR = 0,
	PEDO_STEP_COUNTER = 1,
	PEDO_STEP_WAKEUP = 2,
};

enum pedo_oper_type {
	PEDO_ENABLE_SENSOR = 0,
	PEDO_DISABLE_SENSOR = 1,
};

#pragma pack(1)

struct pedo_config_data {
	u8 oper; // oper type, enable or disable
	u8 sensor; // step detector, step counter or step counter wakeup
	u8 non_wakeup;
	u8 rsvd;
	u32 interval; // only step counter wakeup use
	u32 latency; // latency after event happened
};

struct pedo_config_packet {
	int sub_cmd;
	struct pedo_config_data config;
};

struct pedo_report_data {
	u16 flag;
	u8 sensor; // step detector, step counter or step counter wakeup
	u8 rsvd;
	u32 steps; // total steps
	u64 timestamp;
};

struct pedo_report_pkt {
	struct pkt_header hd;
	struct pedo_report_data data;
};

#define RSVD_LEN 3
struct pedo_flush_data {
	u8 sensor;
	u8 rsvd[RSVD_LEN];
};

struct pedo_flush_pkt {
	u32 sub_cmd;
	struct pedo_flush_data data;
};

#pragma pack()

#endif /* _PEDO_TYPE_H_ */
