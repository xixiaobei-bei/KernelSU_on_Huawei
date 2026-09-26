/*
 * wifi_audio_ptp.h
 *
 * wifi audio ptp time sync kernel module implementation
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

#ifndef _WIFI_AUDIO_PTP_H_
#define _WIFI_AUDIO_PTP_H_

#include <linux/netdevice.h>
#include <linux/hrtimer.h>
#include <linux/time.h>
#include <net/ip.h>
#include <linux/interrupt.h>
#include <linux/timekeeping.h>
#include "wifi_audio.h"

#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
#include <linux/io.h>
#endif
#include "wifi_audio_utils.h"

#define PTP_SET_TIME_OFFSET_THREHOLD_USEC 5000
#define SKB_DATA_LEN_MAX 512
#define SKB_HEAD_ROOM_LEN 32
#define SKB_TCP_SOURCE_PORT 65500
#define SKB_TCP_DEST_PORT 65501
#define SKB_UDP_SOURCE_PORT 3200
#define SKB_UDP_DEST_PORT 3200
#define MSG_TYPE_INDEX 0
#define MSG_SUB_TYPE_INDEX 1
#define MSG_PTP_COUNTER_INDEX 2
#define MSG_PTP_TV_SEC_INDEX 6
#define MSG_PTP_TV_USEC_INDEX 10
#define MSG_PTP_OFFSET_INDEX 6
#define MSG_PTP_DELAY_INDEX 14
#define MSG_PTP_SYNC (unsigned char)0x01
#define SYNC_REQ (unsigned char)0x00
#define DELAY_REQ (unsigned char)0x02
#define DELAY_RESP (unsigned char)0x03
#define SYNC_DONE (unsigned char)0x04
#define RESTART_TIME_SYNC (unsigned char)0x05
#define STOP_TIME_SYNC (unsigned char)0x06
#define SECOND_IN_HALF_HOUR 30
#define SECOND_IN_MINUTE 60
#define US_IN_SECOND 1000000
#define MS_N_SEOND 1000
#define PTP_TIME_QUEUE_LEN 60
#define PTP_TIME_STANDARD_CAL_START 10
#define DOUBLE_SIZE 2
#define PTP_TIME_STANDARD_THRES 1500
#define PTP_TIME_OFFSET_THRES 5
#define PTP_TIME_SET_INTERVAL 60
#define PTP_TIME_SET_USEC_THRES 999000
#define PTP_TIME_STANDARD_CAL_LEN ((PTP_TIME_QUEUE_LEN) - (PTP_TIME_STANDARD_CAL_START) * (DOUBLE_SIZE))
#define PTP_TIME_FIRST_SET_NUM_MAX 8
#define PTP_TIME_FIRST_SET_THRES 3
#define FIRST_SET_ARRAY_LEN 5
#define PTP_START_SYNC_AGAIN_TIME_NSEC (600 * 1000 * 1000)
#define PTP_TIME_SYNC_TIMEOUT_TIME_NSEC (300 * 1000 * 1000)
#define PTP_TIME_SYNC_TIMEOUT_TIME_SEC 1
#define STOP_DATA_SYNC_THRES (48 * 1000)
#define STOP_DATA_SYNC_TRIGGER 6
#define START_DATA_SYNC_TRIGGER (PTP_TIME_QUEUE_LEN)

#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
#define WRITE_TIME_REG_TIMEOUT_TIME_NSEC (300 * 1000)
#define WRITE_TIME_REG 0xF8A20410
#define WRITE_TIME_REG_LEN 8
#endif

#define WAUDIO_SKB_BUFFER_LEN 32
#define TIME_SYNC_TIMEOUT_COUNTER_MAX 10
#define TIME_SYNC_TIMEOUT_COUNTER_DBAC_MAX 20

#define MSG_LEN 128
#define UDP_MSG_LEN 28

enum time_sync_ver {
	TIME_SYNC_VER_0 = 0,
	TIME_SYNC_VER_1,
	TIME_SYNC_VER_INVALID,
};

enum ptp_role {
	PTP_MASTER = 0,
	PTP_SLAVE,
	PTP_ROLE_INVALID,
};

enum channel_id {
	CHANNEL_LEFT = 0,
	CHANNEL_RIGHT,
	CHANNEL_CENTER,
	CHANNEL_LS,
	CHANNEL_RS,
	CHANNEL_WOOFER,
	CHANNEL_ONE_SPEAKER,
	CHANNEL_MAX,
};

enum ptp_event_id {
	PTP_SEND_SYNC_REQUEST = 0,
	PTP_RECEIVE_SYNC_REQUEST,
	PTP_SEND_DELAY_REQUEST,
	PTP_RECEIVE_DELAY_REQUEST,
	PTP_SEND_DELAY_RESPONSE,
	PTP_RECEIVE_DELAY_RESPONSE,
	PTP_SEND_SYNC_DONE,
	PTP_RECEIVE_SYNC_DONE,
	PTP_SEND_RESTART_TIME_SYNC,
	PTP_RECEIVE_RESTART_TIME_SYNC,
	PTP_RECEIVE_STOP_TIME_SYNC,
	PTP_RECEIVE_SYNC_REQUEST_UDP,
	PTP_RECEIVE_DELAY_REQUEST_UDP,
	PTP_EVENT_ID_MAX,
};

enum ptp_status {
	PTP_NOT_ENABLED = 0,
	PTP_START,
	PTP_ABNORMAL,
	PTP_STOP,
	PTP_STATUS_INVALID,
};

enum ptp_time_sync_reason {
	TIME_SYNC_ABNORMAL_TIMEOUT,
	TIME_SYNC_STOP_BY_THE_OTHER_END,
	TIME_SYNC_REASON_INVALID,
};

enum data_sync_status {
	DATA_SYNC_STOP,
	DATA_SYNC_START,
	DATA_SYNC_INVALID,
};

enum net_dev_status {
	NET_DEV_NORMAL,
	NET_DEV_ABNORMAL,
	NET_DEV_STATUS_INVALID,
};

enum wifi_mode_type {
	WIFI_MODE_DEFAULT = 0, /* default config */
	WIFI_MODE_DBAC,
	WIFI_MODE_INVALID,
};

struct netlink_msg_config_info {
	unsigned int num;
	unsigned int mode;
};

struct netlink_msg_ptp_start_info {
	unsigned int channel;
	int role;
	unsigned int ip_dest;
	unsigned int ip_source;
	unsigned short cmd_port;
	unsigned short data_port;
	unsigned char mac_dest[ETH_ALEN];
	unsigned char mac_source[ETH_ALEN];
	char interface_name[IFNAMSIZ];
	int time_sync_ver;
};

struct netlink_msg_ptp_stop_info {
	unsigned int channel;
};

struct netlink_event_report_offset_delay {
	unsigned int channel;
	long long offset_usec;
	long long delay_usec;
};

struct netlink_event_report_time_sync {
	unsigned int channel;
	int status;
	int reason;
};

struct netlink_event_report_data_sync {
	unsigned int channel;
	int status;
};

struct netlink_event_report_sync_offset {
	unsigned int channel;
	long long offset_us;
};

struct netlink_event_report_net_dev {
	int status;
};

struct ptp_event_data {
	int id;
	__be32 ip_dest;
	__be32 ip_source;
	unsigned char msg[MSG_LEN];
	struct timespec64 stamp;
};

struct timeval_ptp {
	unsigned int tv_sec; /* seconds */
	unsigned int tv_usec; /* microseconds */
};

struct time_infor {
	unsigned int counter;
	struct timeval_ptp t1;
	struct timeval_ptp t2;
	struct timeval_ptp t3;
	struct timeval_ptp t4;
	long long offset;
	long long delay;
	int first_time_set_complete;
	long long offset_pre;
};

struct time_infor_udp {
	struct timespec64 t1;
	struct timespec64 t2;
	struct timespec64 t3;
	struct timespec64 t4;
};

struct ptp_dev {
	int status;
	int role;
	unsigned int channel;
	int time_sync_ver;
	int protocol;
	__be32 ip_dest;
	__be32 ip_source;
	unsigned short cmd_port;
	unsigned short data_port;
	unsigned char mac_dest[ETH_ALEN]; /* destination eth addr */
	unsigned char mac_source[ETH_ALEN]; /* source ether addr */
	char interface_name[IFNAMSIZ];
	struct net_device *net_dev;
	struct tasklet_struct dev_work;
	struct list_queue dev_event_queue;
	struct hrtimer start_ptp;
	struct work_struct start_ptp_work;
	struct time_infor cur_ptp_time;
	struct list_queue ptp_time_queue;
	struct hrtimer time_sync_timeout;
	struct work_struct time_sync_timeout_work;
	unsigned int time_sync_timeout_counter;
	int data_sync_stop_counter;
	int data_sync_start_counter;
	int data_sync_status;
	struct time_infor_udp time_udp;
	__be16 id_udp;
};

struct waudio_skb {
	struct sk_buff *skb;
	struct timespec64 stamp;
	int protocol;
};

struct wifi_audio_ptp {
	unsigned int num;
	unsigned int mode;
	unsigned int time_sync_timeout_max_counter;
	struct ptp_dev dev[CHANNEL_MAX];
	struct tasklet_struct event_handle_work;
	struct list_queue waudio_skb_buffer;
	struct list_queue waudio_skb_handle;
#ifdef HONGHU_WIFI_AUDIO_CAST_PCM_DATA
	struct hrtimer write_time_reg_timer;
	int write_time_reg_timer_started;
	void __iomem *time_regs;
#endif
};

int netlink_msg_ptp_handle(const struct netlink_data *msg_rev);
int wifi_audio_time_sync_init(void);
void wifi_audio_time_sync_uninit(void);
void ptp_clear_by_dev(const struct net_device *dev);
#endif /* _WIFI_AUDIO_PTP_H_ */

