/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: This file is the interface file of other modules.
 * Author: linlixin2@huawei.com
 * Create: 2019-03-19
 */

#ifndef _NETLINK_HANDLE_H
#define _NETLINK_HANDLE_H

#include <linux/types.h>
#include <linux/version.h>

#define MAX_REQ_DATA_LEN 2048

struct seg_tlv_head {
	u8 type;
	u8 len;
};

/* Notification request issued by the upper layer is defined as: */
struct req_msg_head {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
};

/* Each module sends the message request is defined as: */
struct res_msg_head {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
};

/*
 * This enumeration type is the netlink command types issued by the JNI.
 * Mainly to maintain the channel with JNI.
 */
enum nl_cmd_type {
	NL_MSG_REG,
	NL_MSG_JNI_REQ,

	NB_MSG_REQ_BUTT
};

/*
 * This enumeration type is an enumeration of internal and external messages,
 * less than 0x8000 is an upper-level message,
 * and a kernel internal module message is greater than 0x8000.
 */
enum msg_rpt_type {
	APP_QOE_RPT = 0,
	PACKET_FILTER_DROP_RPT = 1,
	PACKET_FILTER_RECOVERY_RPT = 2,
	TCP_PKT_CONUT_RPT = 3,
	SLICE_IP_PARA_RPT = 4,
	ICMP_PING_REPORT = 5,
	PACKET_DELAY_RPT = 6,
	FASTGRAB_CHR_RPT = 7,
	STEADY_SPEED_STATS_RPT = 8,
	WIFI_PARA_RPT = 9,
	SPEED_TEST_CHR_RPT = 10,
	STREAM_DETECTION_RPT,
	INTER_MSG_BASE = 0x8000,
	WIFI_RPT_TIMER,
	NBMSG_RPT_BUTT
};

/* Message type enumeration issued by the upper layer */
enum req_msg_type {
	APP_QOE_SYNC_CMD = 0,
	UPDATE_APP_INFO_CMD,
	/* hw_packet_filter_bypass begin */
	ADD_FG_UID,
	DEL_FG_UID,
	BYPASS_FG_UID,
	NOPASS_FG_UID,
	/* hw_packet_filter_bypass end */
	TCP_PKT_COLLEC_CMD,
	BIND_UID_PROCESS_TO_NETWORK_CMD,
	DEL_UID_BIND_CMD,
	SLICE_RPT_CTRL_CMD,
	SOCK_DESTROY_HANDLER_CMD,
	ICMP_PING_DETECT_CMD,
	/* tcp_congestion_algo_controller ops */
	UPDATE_TCP_CA_CONFIG,
	UPDATE_UID_STATE,
	UPDATE_DS_STATE,
	BIND_PROCESS_TO_SLICE_CMD,
	DEL_SLICE_BIND_CMD,
	APP_ACCELARTER_CMD,
	/* keep value with SteadySpeedChr in booster apk */
	UPDATE_UID_LIST = 18,
	WIFI_PARA_COLLECT_START = 20,
	WIFI_PARA_COLLECT_STOP,
	WIFI_PARA_INTRA_MSG,
	SPEED_TEST_STATUS = 23,
	STREAM_DETECTION_START = 24,
	STREAM_DETECTION_STOP,
	UPDAT_INTERFACE_NAME = 26,
	WIFI_PARA_COLLECT_UPDATE = 27,
	CMD_NUM_MAX
};

/* module IDs defined for each module */
enum install_model {
	IP_PARA_COLLEC = 0,
	PACKET_FILTER_BYPASS,
	TCP_PARA_COLLEC,
	NETWORK_SLICE_MANAGEMENT,
	SOCK_DESTROY_HANDLER,
	ICMP_PING_DETECT,
	TCP_CONG_ALGO_CTRL,
	PACKET_TRACKER,
	APP_ACCELERATOR,
	STEADY_SPEED,
	WIFI_PARA_COLLEC,
	STREAM_DETECT,
	MODEL_NUM
};

/* mesage map entry index */
enum map_index {
	MAP_KEY_INDEX = 0,
	MAP_VALUE_INDEX = 1,
	MAP_ENTITY_NUM
};

typedef void notify_event(struct res_msg_head *msg);
typedef void msg_process(struct req_msg_head *req);

#endif /* _NETLINK_HANDLE_H */
