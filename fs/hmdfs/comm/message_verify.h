/* SPDX-License-Identifier: GPL-2.0 */
/*
 * message_verify.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: wangminmin4@huawei.com lousong@huawei.com
 * Create: 2020-07-08
 *
 */

#ifndef HMDFS_MESSAGE_VERIFY_H
#define HMDFS_MESSAGE_VERIFY_H

#include "protocol.h"

enum MESSAGE_LEN_JUDGE_TYPE {
	MESSAGE_LEN_JUDGE_RANGE = 0,
	MESSAGE_LEN_JUDGE_BIN = 1,
};

#define HMDFS_MESSAGE_MIN_INDEX		0
#define HMDFS_MESSAGE_MAX_INDEX		1
#define HMDFS_MESSAGE_LEN_JUDGE_INDEX	2
#define HMDFS_MESSAGE_MIN_MAX		3

void hmdfs_message_verify_init(void);
int hmdfs_message_verify(struct hmdfs_peer *con, struct hmdfs_head_cmd *head,
			 void *data);

#endif
