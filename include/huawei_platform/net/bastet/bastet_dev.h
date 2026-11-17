/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: Provide kernel device information for bastet.
 * Author: pengyu@huawei.com
 * Create: 2014-06-21
 */

#ifndef _BASTET_DEV_H
#define _BASTET_DEV_H

#include <linux/ioctl.h>
#include <linux/version.h>
#include "bastet_fastgrab.h"

#define BASTET_DEV_NAME "/dev/bastet"
#define IPV6_ADDR_LENGTH 16
#define BST_IOC_MAGIC 'k'
#define BST_IOC_SOCK_SYNC_START _IOW(BST_IOC_MAGIC, 1, \
	struct bst_set_sock_sync_delay)
#define BST_IOC_SOCK_SYNC_STOP _IOW(BST_IOC_MAGIC, 2, struct bst_sock_id)
#define BST_IOC_SOCK_SYNC_SET _IOW(BST_IOC_MAGIC, 3, \
	struct bst_set_sock_sync_prop)
#define BST_IOC_SOCK_COMM_GET _IOWR(BST_IOC_MAGIC, 4, \
	struct bst_get_sock_comm_prop)
#define BST_IOC_SOCK_CLOSE_SET _IOW(BST_IOC_MAGIC, 5, struct bst_sock_comm_prop)
#define BST_IOC_SOCK_STATE_GET _IOWR(BST_IOC_MAGIC, 6, \
	struct bst_get_bastet_sock_state)
#define BST_IOC_APPLY_LOCAL_PORT _IOR(BST_IOC_MAGIC, 7, uint16_t)
#define BST_IOC_RELEASE_LOCAL_PORT _IOW(BST_IOC_MAGIC, 8, uint16_t)
#define BST_IOC_SET_TRAFFIC_FLOW _IOW(BST_IOC_MAGIC, 9, \
	struct bst_traffic_flow_prop)
#define BST_IOC_GET_TIMESTAMP_INFO _IOR(BST_IOC_MAGIC, 10, \
	struct bst_timestamp_info)
#define BST_IOC_SET_NET_DEV_NAME _IOW(BST_IOC_MAGIC, 11, \
	struct bst_net_dev_name)
#ifdef CONFIG_HUAWEI_BASTET_COMM
#define BST_IOC_GET_MODEM_RAB_ID _IOR(BST_IOC_MAGIC, 12, \
	struct bst_modem_rab_id)
#endif
#define BST_IOC_SOCK_RECONN_SET _IOW(BST_IOC_MAGIC, 13, \
	struct bst_get_sock_comm_prop)
#define BST_IOC_SOCK_RECONN_FAIL _IOW(BST_IOC_MAGIC, 14, \
	struct bst_sock_comm_prop)
#define BST_IOC_SET_RECONN_PROXYID _IOW(BST_IOC_MAGIC, 15, \
	struct reconn_id)
#define BST_IOC_SYNC_PROP_START _IOW(BST_IOC_MAGIC, 16, \
	struct bst_set_sock_sync_prop)
#define BST_IOC_SYNC_PROP_STOP _IOW(BST_IOC_MAGIC, 17, \
	struct bst_sock_comm_prop)
#define BST_IOC_PROXY_AVAILABLE _IOW(BST_IOC_MAGIC, 18, int32_t)
#define BST_IOC_BUF_AVAILABLE _IOW(BST_IOC_MAGIC, 19, int32_t)
#define BST_IOC_PRIO_SEND_SUC _IOW(BST_IOC_MAGIC, 20, int32_t)
#define BST_IOC_SOCK_SYNC_PREPARE _IOW(BST_IOC_MAGIC, 21, \
	struct bst_set_sock_sync_delay)
#define BST_IOC_GET_FD_BY_ADDR _IOWR(BST_IOC_MAGIC, 22, struct addr_to_fd)
#define BST_IOC_GET_CMDLINE _IOWR(BST_IOC_MAGIC, 23, struct get_cmdline)
#define BST_IOC_SYNC_HOLD_TIME _IOW(BST_IOC_MAGIC, 24, \
	struct bst_set_sock_sync_delay)
#define BST_IOC_NET_DEV_RESET _IOW(BST_IOC_MAGIC, 25, int32_t)
#define BST_IOC_UID_PROP_MONITOR _IOW(BST_IOC_MAGIC, 26, \
	struct app_monitor_prop)
#define BST_IOC_SET_HRTAPP_ACTIVITY _IOW(BST_IOC_MAGIC, 27, int32_t)
#define BST_IOC_FAST_GRAB_INFO _IOW(BST_IOC_MAGIC, 28, struct fastgrab_info)
#define BST_IOC_SET_PROC_INFO _IOWR(BST_IOC_MAGIC, 29, struct set_process_info)
#define BST_IOC_SET_SPECIAL_UID _IOW(BST_IOC_MAGIC, 30, int32_t)
#define BST_IOC_NF_CONTROL _IOW(BST_IOC_MAGIC, 31, bool)
#define BST_IOC_GET_SOCK_NETDEV_NAME _IOWR(BST_IOC_MAGIC, 32, \
	struct get_netdev_name)
#define BST_IOC_SEND_HB_DATA _IOW(BST_IOC_MAGIC, 33, struct heartbeat_content)
#define BST_IOC_SET_HB_REPLY _IOW(BST_IOC_MAGIC, 34, struct heartbeat_content)
#define BST_IOC_FILTER_HB_REPLY _IOW(BST_IOC_MAGIC, 35, struct bst_sock_id)
#define BST_IOC_SET_FREEZER_STATE _IOW(BST_IOC_MAGIC, 36, struct freezer_state)
#define BST_IOC_SET_PRIO_CH_ENABLE _IOW(BST_IOC_MAGIC, 37, int32_t)
#define BST_IOC_SOCK_COMM_GET_IPV6 _IOWR(BST_IOC_MAGIC, 38, \
	struct bst_get_sock_comm_prop_ipv6)
#define BST_IOC_SOCK_SYNC_SET_IPV6 _IOW(BST_IOC_MAGIC, 39, \
	struct bst_set_sock_sync_prop_ipv6)
#define BST_IOC_SOCK_CLOSE_SET_IPV6 _IOW(BST_IOC_MAGIC, 40, \
	struct bst_sock_comm_prop_ipv6)
#define BST_IOC_SYNC_PROP_START_IPV6 _IOW(BST_IOC_MAGIC, 41, \
	struct bst_set_sock_sync_prop_ipv6)
#define BST_IOC_SYNC_PROP_STOP_IPV6 _IOW(BST_IOC_MAGIC, 42, \
	struct bst_sock_comm_prop_ipv6)

#define BST_IOC_SOCK_CMD_MIN 1
#define BST_IOC_SOCK_CMD_MAX 43

#define BST_IOC_SOCK_SYNC_START_ID 1
#define BST_IOC_SOCK_SYNC_STOP_ID 2
#define BST_IOC_SOCK_SYNC_SET_ID 3
#define BST_IOC_SOCK_COMM_GET_ID 4
#define BST_IOC_SOCK_CLOSE_SET_ID 5
#define BST_IOC_SOCK_STATE_GET_ID 6
#define BST_IOC_APPLY_LOCAL_PORT_ID 7
#define BST_IOC_RELEASE_LOCAL_PORT_ID 8
#define BST_IOC_SET_TRAFFIC_FLOW_ID 9
#define BST_IOC_GET_TIMESTAMP_INFO_ID 10
#define BST_IOC_SET_NET_DEV_NAME_ID 11
#define BST_IOC_GET_MODEM_RAB_ID_ID 12
#define BST_IOC_SOCK_RECONN_SET_ID 13
#define BST_IOC_SOCK_RECONN_FAIL_ID 14
#define BST_IOC_SET_RECONN_PROXYID_ID 15
#define BST_IOC_SYNC_PROP_START_ID 16
#define BST_IOC_SYNC_PROP_STOP_ID 17
#define BST_IOC_PROXY_AVAILABLE_ID 18
#define BST_IOC_BUF_AVAILABLE_ID 19
#define BST_IOC_PRIO_SEND_SUC_ID 20
#define BST_IOC_SOCK_SYNC_PREPARE_ID 21
#define BST_IOC_GET_FD_BY_ADDR_ID 22
#define BST_IOC_GET_CMDLINE_ID 23
#define BST_IOC_SYNC_HOLD_TIME_ID 24
#define BST_IOC_NET_DEV_RESET_ID 25
#define BST_IOC_UID_PROP_MONITOR_ID 26
#define BST_IOC_SET_HRTAPP_ACTIVITY_ID 27
#define BST_IOC_FAST_GRAB_INFO_ID 28
#define BST_IOC_SET_PROC_INFO_ID 29
#define BST_IOC_SET_SPECIAL_UID_ID 30
#define BST_IOC_NF_CONTROL_ID 31
#define BST_IOC_GET_SOCK_NETDEV_NAME_ID 32
#define BST_IOC_SEND_HB_DATA_ID 33
#define BST_IOC_SET_HB_REPLY_ID 34
#define BST_IOC_FILTER_HB_REPLY_ID 35
#define BST_IOC_SET_FREEZER_STATE_ID 36
#define BST_IOC_SET_PRIO_CH_ENABLE_ID 37
#define BST_IOC_SOCK_COMM_GET_IPV6_ID 38
#define BST_IOC_SOCK_SYNC_SET_IPV6_ID 39
#define BST_IOC_SOCK_CLOSE_SET_IPV6_ID 40
#define BST_IOC_SYNC_PROP_START_IPV6_ID 41
#define BST_IOC_SYNC_PROP_STOP_IPV6_ID 42
#define BST_IOC_GET_MODEM_RAB_ID_IPV6 43

enum bst_sock_state {
	BST_SOCK_NOT_USED = 0,
	BST_SOCK_INVALID,
	BST_SOCK_UPDATING,
	BST_SOCK_VALID,
	BST_SOCK_NOT_CREATED,
};

enum bst_ind_type {
	BST_IND_INVALID = 0,
	BST_IND_HISICOM,
	BST_IND_SOCK_SYNC_REQ,
	BST_IND_SOCK_SYNC_PROP,
	BST_IND_SOCK_CLOSED,
	BST_IND_MODEM_RESET = 5,
	BST_IND_NETFILTER_SYNC_UID,
	BST_IND_TRAFFIC_FLOW_REQ,
	BST_IND_PRIORITY_DATA,
	BST_IND_SOCK_DISCONNECT,
	BST_IND_SOCK_TIMEDOUT = 10,
	BST_IND_SOCK_EST,
	BST_IND_SOCK_RENEW_CLOSE,
	BST_IND_SOCK_NORMAL_CLOSE,
	BST_IND_SOCK_ERR_CLOSE,
	BST_IND_PRIO_SOCK_CLOSE = 15,
	BST_IND_SCREEN_STATE,
	BST_IND_UID_SOCK_PROP,
	BST_IND_TRIGGER_THAW,
	BST_IND_SOCK_STATE_CHANGED,
	BST_IND_PRIORITY_UID = 20,
	BST_IND_FG_KEY_MSG,
	BST_IND_FG_UID_SOCK_CHG,
	BST_IND_HB_REPLY_RECV,
	BST_IND_RRC_KEEP,
	BST_IND_RCVQUEUE_FULL = 25,
	BST_IND_SKSTATE_NOT_UPDATING,
	BST_IND_SOCK_SYNC_FAILED,
	BST_IND_GET_SK_FAILED,
	BST_IND_GET_BSK_FAILED,
	BST_IND_PENDING_SK_SET = 30,
	BST_IND_NOPENDING_SK_SET,
	BST_IND_SOCK_STATE_WAIT,
	BST_IND_SEND_DATA_NOTACK,
	BST_IND_RECV_DATA_INQUEUE,
	BST_IND_SOCK_SYNC_PROP_IPV6 = 35,
	BST_IND_SOCK_CLOSED_IPV6,
	BST_IND_SOCK_SYNC_REQ_IPV6,
	BST_IND_PRIORITY_DATA_IPV6,
	BST_IND_SOCK_DISCONNECT_IPV6,
	BST_IND_SOCK_TIMEDOUT_IPV6 = 40,
	BST_IND_SOCK_EST_IPV6,
	BST_IND_SOCK_NORMAL_CLOSE_IPV6,
	BST_IND_SOCK_ERR_CLOSE_IPV6,
	BST_IND_PRIO_SOCK_CLOSE_IPV6,
	BST_IND_UID_SOCK_PROP_IPV6 = 45,
	BST_IND_SN_INVALID,
	BST_IND_RECV_DATA_INQUEUE_IPV6,
	BST_IND_SEND_DATA_NOTACK_IPV6,
	BST_IND_SOCK_STATE_WAIT_IPV6,
	BST_IND_GET_BSK_FAILED_IPV6 = 50,
	BST_IND_PENDING_SK_SET_IPV6,
	BST_IND_SKSTATE_NOT_UPDATING_IPV6,
	BST_IND_SOCK_SYNC_FAILED_IPV6,
	BST_IND_NOPENDING_SK_SET_IPV6,
	BST_IND_RCVQUEUE_FULL_IPV6 = 55,
};

enum monitor_cmd {
	CMD_CHECK_UID_SOCK,
	CMD_ADD_UID,
	CMD_DEL_UID,
};

enum bastet_filter_cmd {
	CMD_ADD_PROC_INFO,
	CMD_DEL_PROC_INFO,
	CMD_DEL_ALL_INFO,
};

struct bst_device_ind {
	uint32_t cons;
	enum bst_ind_type type;
	uint32_t len;
	uint8_t value[0];
};

struct bst_sock_id {
	int fd;
	pid_t pid;
};

struct bst_sock_sync_prop {
	uint32_t seq;
	uint32_t rcv_nxt;
	uint32_t snd_wnd;
	uint32_t ts_recent;
	uint32_t mss;
	uint32_t tx;
	uint32_t rx;
	uint16_t snd_wscale;
	uint16_t rcv_wscale;
	uint32_t ts_current;
	uint32_t ts_recent_tick;
};

struct bst_sock_comm_prop {
	uint32_t local_ip;
	uint32_t remote_ip;
	uint16_t local_port;
	uint16_t remote_port;
};

struct bst_get_sock_comm_prop {
	struct bst_sock_id guide;
	struct bst_sock_comm_prop comm_prop;
};

struct bst_set_sock_sync_delay {
	struct bst_sock_id guide;
	uint32_t hold_time;
	int32_t proxy_id;
	bool is_wifi;
};

struct bst_close_sock_prop {
	struct bst_sock_comm_prop comm_prop;
	int32_t proxy_id;
};

struct bst_set_sock_sync_prop {
	struct bst_sock_comm_prop guide;
	struct bst_sock_sync_prop sync_prop;
};

struct bst_ind_sock_sync_prop {
	struct bst_sock_id guide;
	struct bst_sock_sync_prop sync_prop;
};

struct bst_ind_priority_prop {
	struct bst_sock_id guide;
	struct bst_sock_comm_prop comm_prop;
	struct bst_sock_sync_prop sync_prop;
	uint8_t priority;
	uint8_t sync_state;
	uint8_t seq_type;
	uint8_t pkt_type;
	uint32_t len;
	uint8_t data[0];
};

struct bst_get_bastet_sock_state {
	struct bst_sock_id guide;
	uint32_t sync_state;
};

struct bst_traffic_flow_prop {
	int fd;
	pid_t pid;
	uid_t uid;
	int protocol;
	uint32_t tx;
	uint32_t rx;
};

struct bst_traffic_flow_pkg {
	uint32_t cnt;
	uint8_t value[0];
};

struct bst_timestamp_info {
	uint32_t time_now;
	uint32_t time_freq;
};

struct bst_net_dev_name {
	char iface[IFNAMSIZ];
};

#ifdef CONFIG_HUAWEI_BASTET_COMM
struct bst_modem_rab_id {
	uint16_t modem_id;
	uint16_t rab_id;
};
#endif

struct addr_to_fd {
	int fd;
	pid_t pid;
	uint32_t remote_ip;
	uint16_t remote_port;
};

#define MAX_PID_NAME_LEN 100
struct get_cmdline {
	pid_t pid;
	char name[MAX_PID_NAME_LEN];
};

struct reconn_id {
	struct bst_sock_id guide;
	int32_t proxy_id;
	bool auto_connect;
};

struct app_monitor_prop {
	enum monitor_cmd cmd;
	int32_t uid;
	int32_t tid_count;
	int32_t tids[0];
};

struct bst_monitor_ind_prop {
	int32_t uid;
	struct bst_sock_id sock_id;
	struct bst_sock_comm_prop comm;
};

struct set_process_info {
	enum bastet_filter_cmd cmd;
	int32_t uid;
	int32_t pid;
};

/*
 * struct get_netdev_name - device info.
 * @guide:pid and fd to find sock
 * @netdev_name[IFNAMSIZ]:network device name
 *
 * used to record the network device information.
 * such as pid and fd, and device name.
 */
struct get_netdev_name {
	struct bst_sock_id guide;
	char netdev_name[IFNAMSIZ];
};

/*
 * struct heartbeat_content - heartbeat info structor.
 * @guide:pid and fd to find sock
 * @len:length of heartbeat content
 * @content[0]:heartbeat content
 *
 * used to record the heartbeat content information.
 */
struct heartbeat_content {
	struct bst_sock_id guide;
	uint32_t len;
	uint8_t content[0];
};

/*
 * struct freezer_state - freezer state information.
 * @guide:pid and fd to find sock
 * @frozen:process freeze state
 *
 * used to record the freezer state information.
 */
struct freezer_state {
	struct bst_sock_id guide;
	bool frozen;
};


struct bst_sock_comm_prop_ipv6 {
	uint8_t local_ip[IPV6_ADDR_LENGTH];
	uint8_t remote_ip[IPV6_ADDR_LENGTH];
	uint16_t local_port;
	uint16_t remote_port;
};

struct bst_get_sock_comm_prop_ipv6 {
	struct bst_sock_id guide;
	struct bst_sock_comm_prop_ipv6 comm_prop;
};

struct bst_close_sock_prop_ipv6 {
	struct bst_sock_comm_prop_ipv6 comm_prop;
	int32_t proxy_id;
};

struct bst_set_sock_sync_prop_ipv6 {
	struct bst_sock_comm_prop_ipv6 guide;
	struct bst_sock_sync_prop sync_prop;
};

struct bst_ind_priority_prop_ipv6 {
	struct bst_sock_id guide;
	struct bst_sock_comm_prop_ipv6 comm_prop;
	struct bst_sock_sync_prop sync_prop;
	uint8_t priority;
	uint8_t sync_state;
	uint8_t seq_type;
	uint8_t pkt_type;
	uint32_t len;
	uint8_t data[0];
};

struct bst_monitor_ind_prop_ipv6 {
	int32_t uid;
	struct bst_sock_id sock_id;
	struct bst_sock_comm_prop_ipv6 comm;
};

#define BST_MAX_WRITE_PAYLOAD 2048
#define BST_MAX_READ_PAYLOAD (BST_MAX_WRITE_PAYLOAD + \
	sizeof(struct bst_device_ind))

#endif /* _BASTET_DEV_H */
