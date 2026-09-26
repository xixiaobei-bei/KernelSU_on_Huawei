/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2021. All rights reserved.
 * Description: hhee msg header
 * Create : 2017/12/6
 */

#ifndef __HHEE_MSG_H__
#define __HHEE_MSG_H__

#define HHEE_MSG_SIZE		0x400 /* 1KB */
#define HHEE_MSG_MAX_SLOTS	15
#define HHEE_MSG_MAX_PAYLOAD	(HHEE_MSG_SIZE - sizeof(struct hhee_msg))

struct hhee_msg_state {
	uint32_t read_pos;
	uint32_t write_pos;
	uint64_t count;
	uint64_t lost;
	uint8_t enabled[HHEE_MSG_ID_MAX];
} __aligned(8);

struct hhee_msg {
	uint64_t seq;
	uint32_t id;
	uint32_t len;
	uint8_t payload[];
} __aligned(8);

#endif
