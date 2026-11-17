/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Himos aweme detect head file.
 * Author: fanxiaoyu3@huawei.com
 * Create: 2018-07-19
 */

#ifndef _HW_HIMOS_AWEME_DETECT_H
#define _HW_HIMOS_AWEME_DETECT_H

#include <linux/types.h>
#include <linux/timer.h>
#include <linux/genetlink.h>

#include <net/genetlink.h>

#include "hw_himos_stats_report.h"
#include "hw_himos_stats_common.h"

#define HIMOS_NUM_9 9
#define HIMOS_NUM_10 10
#define HIMOS_NUM_15 15
#define HIMOS_NUM_16 16
#define HIMOS_MSG_LEN_LIMIT 8

enum key_shift_enum {
	KEY_SHIFT_0 = 0,
	KEY_SHIFT_8 = 8,
	KEY_SHIFT_16 = 16,
	KEY_SHIFT_24 = 24,

	KEY_SHIFT_BUFF
};

enum local_para_enum {
	LOC_PARA_0 = 0,
	LOC_PARA_1 = 1,
	LOC_PARA_2 = 2,
	LOC_PARA_3 = 3,
	LOC_PARA_4 = 4,
	LOC_PARA_5 = 5,
	LOC_PARA_6 = 6,
	LOC_PARA_7 = 7,

	LOC_PARA_BUFF
};

struct himos_sport_key_dict {
	unsigned short sport;
	char key[STALL_KEY_MAX];
};

struct himos_aweme_detect_info {
	/* this is must be the first member */
	struct himos_stats_common comm;

	__u32 interval;
	int keepalive;

	int detecting, cur_detect_index;
	struct stall_info stalls[AWEME_STALL_WINDOW];
	int cur_index, valid_num;

	unsigned short local_sport;
	unsigned int local_inbytes;

	struct himos_aweme_search_key keys;

	int cur_dict_index;
	int dict_num;
	struct himos_sport_key_dict sport_key_dict[AWEME_STALL_WINDOW];
};

int himos_start_aweme_detect(struct genl_info *info);
int himos_stop_aweme_detect(const struct genl_info *info);
int himos_keepalive_aweme_detect(const struct genl_info *info);
int himos_update_aweme_stall_info(struct sk_buff *skb, struct genl_info *info);
void himos_aweme_tcp_stats(struct sock *sk, struct msghdr *old,
	struct msghdr *new, int inbytes, int outbytes);

int himos_aweme_init(void) __init;

#endif
