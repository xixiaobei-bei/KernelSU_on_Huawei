/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This module is to collect wifi chip parameters.
 * Author: tongxilin@huawei.com
 * Create: 2020-03-30
 */

#ifndef _WIFI_PARA_COLLEC_H
#define _WIFI_PARA_COLLEC_H

#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <net/cfg80211.h>

#include "netlink_handle.h"

#define MAX_RSSI (-1)
#define MIN_RSSI (-100)
#define MAX_NOISE 0
#define MIN_NOISE (-100)
#define MAX_SNR 50
#define MIN_SNR (-10)
#define MAX_UL_DELAY 60000
#define MIN_UL_DELAY 0
#define MAX_CHLOAD 1000
#define MIN_CHLOAD 0
#define MAX_WIFI_NUM 2
#define DEV_INDEX_OFFSET 2 /* wlan0:devid = 2, wlan1:devid = 3 */
#define INVALID_NUM1 0xFFFF
#define INVALID_NUM2 0
#define MAX_U32 0xFFFFFFFF


struct wifi_req_msg {
	u16 type;
	u16 len;
	u32 report_expires;
	u32 wifi_gateway[MAX_WIFI_NUM];
};

struct timer_msg {
	u16 type;
	u16 len;
	u32 sync;
};

struct wifi_res {
	s32 rx_bytes;
	s32 tx_bytes;
	s32 rssi;
	u32 phy_tx_rate;
	u32 phy_rx_rate;
	s32 rx_packets;
	s32 tx_packets;
	s32 tx_failed;
	s32 noise;
	s32 snr;
	s32 chload;
	s32 ul_delay;
	u32 arp_state;
};

struct wifi_para {
	u32 rx_bytes;
	u32 tx_bytes;
	u32 phy_tx_rate;
	u32 phy_rx_rate;
	u32 rx_packets;
	u32 tx_packets;
	u32 tx_failed;
	u32 arp_state;
	s32 rssi;
	s32 noise;
	s32 snr;
	s32 chload;
	s32 ul_delay;
};

/* Statistical list head of wifi stat */
struct wifi_stat {
	u16 dev_id;
	u16 flag;
	struct wifi_para info;
};

/* wifi paramter collect context struct */
struct wifi_ctx {
	u32 expires; // timer cycle by jiffies, set by upper.
	struct wifi_stat *past; // cpus private statistics context parameter
	notify_event *fn; // recv notification interface callback function.
	struct timer_list timer; // report timer for statistics
};

struct wifi_res_msg {
	u16 type; // Event enumeration values
	u16 len; // The length behind this field, the limit lower 2048
	struct wifi_stat wifi_res[0];
};

enum wifi_index {
	WLAN0_IDX,
	WLAN1_IDX,
};

msg_process *wifi_para_collec_init(notify_event *fn);
void wifi_para_collec_exit(void);

#endif
