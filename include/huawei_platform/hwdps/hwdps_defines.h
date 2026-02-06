/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function required for operations about
 *              some tools.
 * Create: 2020-06-16
 */

#ifndef _HWDPS_DEFINES_H
#define _HWDPS_DEFINES_H

#include <linux/types.h>

#define PHASE_1_KEY_LENGTH 32
#define USER_KEY_LENGTH 64

#define HWDPS_ENABLE_FLAG 0x00F0

enum {
	ERR_MSG_SUCCESS = 0,
	ERR_MSG_OUT_OF_MEMORY = 1,
	ERR_MSG_NULL_PTR = 2,
	ERR_MSG_BAD_PARAM = 3,
	ERR_MSG_KERNEL_PHASE1_KEY_NULL = 1006,
	ERR_MSG_KERNEL_PHASE1_KEY_NOTMATCH = 1007,
	ERR_MSG_LIST_NODE_EXIST = 2001,
	ERR_MSG_LIST_NODE_NOT_EXIST = 2002,
	ERR_MSG_GENERATE_FAIL = 2010,
	ERR_MSG_LIST_EMPTY = 2011,
};

typedef struct {
	u8 *data;
	u32 len;
} buffer_t;

typedef struct {
	u8 *data;
	u32 *len;
} out_buffer_t;

typedef struct {
	u8 **data;
	u32 *len;
} secondary_buffer_t;

typedef struct {
	pid_t pid;
	uid_t uid;
	uid_t task_uid;
} encrypt_id;

typedef struct {
	u8 **fek;
	u32 *fek_len;
	u8 **efek;
	u32 *efek_len;
} fek_efek_t;

#endif
