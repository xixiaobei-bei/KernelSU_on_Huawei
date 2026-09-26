/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020. All rights reserved.
 * Description:ar_type.h
 * Create:2020.03.06
 */
#ifndef _AR_TYPE_H_
#define _AR_TYPE_H_

// this enum need to not conflict with the old enumeration
typedef enum {
	AR_STATIONARY = 4,
	AR_MOTION = 38,
	AR_WALKING = 39,
	AR_RUNNING = 40,
	AR_WRIST_DOWN = 41,
	AR_OFFBODY = 42,
	AR_UNKNOWN = 63,
} ar_type;

#pragma pack(1)

struct ar_config_para {
	uint8_t activity; // cared ar event
	uint8_t event_type; // 1 is care enter, 2 is care exit , 3 is care all
	uint8_t non_wakeup;
	uint8_t rsvd;
	uint32_t report_latency;
};

struct ar_config_data {
	uint8_t core;
	uint8_t oper;
	uint16_t rsvd;
	struct ar_config_para para;
};

/*
 * packet for config ar activity
 */
struct ar_config_packet {
	int sub_cmd;
	struct ar_config_data config_data;
};

typedef enum {
	AR_ADD_EVENT = 0,
	AR_DEL_EVENT = 1,
} event_oper;

struct ar_report_state {
	uint8_t event_type;
	uint8_t activity;
	uint16_t rsvd;
	uint64_t timestamp;
	int32_t score; // confidence, not use now
};

/*
 * packet for ar report data
 */
struct ar_report_data {
	struct pkt_header hd;
	uint16_t event_flag; // now only use for flush
	uint16_t num;
	struct ar_report_state ar_data[];
};

struct ar_flush_data_ext {
	uint8_t core; // core_type_t
	uint8_t activity; // ar_type
	uint16_t rsvd;
};

/*
 * packet for flush
 */
struct ar_flush_data {
	int32_t subcmd;
	struct ar_flush_data_ext flush_data;
};

#pragma pack()

#endif /* _AR_TYPE_H_ */
