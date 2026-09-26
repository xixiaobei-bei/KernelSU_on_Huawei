/*
 * drv_mailbox_debug.h
 *
 * this file implement mailbox debug utils
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef __DRV_MAILBOX_DEBUG_H__
#define __DRV_MAILBOX_DEBUG_H__

#include <linux/types.h>
#include "drv_mailbox_platform.h"

#define MAILBOX_MAIL_TRANS_TIME_LIMIT 3000
#define MAILBOX_MAIL_DEAL_TIME_LIMIT 2000
#define MAILBOX_MAIL_SCHE_TIME_LIMIT 2000


#define MAILBOX_ERRO_ARRAY_NUM 5

#define MAILBOX_LOG_NONE (-1)
#define MAILBOX_LOG_CRITICAL 0
#define MAILBOX_LOG_ERROR 1
#define MAILBOX_LOG_WARNING 2
#define MAILBOX_LOG_INFO 3

#define MAILBOX_SHOW_ALL 0x001
#define MAILBOX_SHOW_CLEAR 0x010

#define MAILBOX_QUEUE_LEFT_INVALID 0xffffffff

#define MAILBOX_MAX_SLICE 0xffffffff

#if (MAILBOX_LOG_LEVEL == MAILBOX_LOG_NONE)
#define mb_logerro_p0(ErrorId) (uint32_t)(ErrorId)
#define mb_logerro_p1(ErrorId, Param) (uint32_t)(ErrorId)
#define mb_logerro_p2(ErrorId, Param1, Param2) (uint32_t)(ErrorId)
#else
#define mb_logerro_p0(ErrorId) \
	mailbox_log_erro((uint32_t)(ErrorId), 0, 0, \
		(uint32_t)_MAILBOX_LINE_, (char *)_MAILBOX_FILE_)

#define mb_logerro_p1(ErrorId, Param) \
	mailbox_log_erro((uint32_t)(ErrorId), \
		(uintptr_t)(Param), 0, \
		(uint32_t)_MAILBOX_LINE_, (char *)_MAILBOX_FILE_)

#define mb_logerro_p2(ErrorId, Param1, Param2) \
	mailbox_log_erro((uint32_t)(ErrorId), \
		(unsigned long)(Param1), (unsigned long)(Param2), \
		(uint32_t)_MAILBOX_LINE_, (char *)_MAILBOX_FILE_)
#endif

#define MB_F_RET_START 0x80000001
#define MB_F_GUT_INVALID_USER_MAIL_HANDLE 0x80000002
#define MB_F_GUT_INIT_CHANNEL_POOL_TOO_SMALL 0x80000003
#define MB_F_GUT_INIT_USER_POOL_TOO_SMALL 0x80000004
#define MB_F_GUT_MUTEX_LOCK_FAILED 0x80000005
#define MB_F_GUT_MUTEX_UNLOCK_FAILED 0x80000006
#define MB_F_GUT_MSG_CHECK_FAIL 0x80000007
#define MB_F_GUT_RECEIVE_MAIL 0x80000008
#define MB_F_GUT_READ_MAIL 0x80000009
#define MB_F_GUT_MEMORY_CONFIG 0x8000000a
#define MB_F_PORT_CONFIG 0x80000010
#define MB_F_RET_END 0x800000ff

#define MB_E_RET_START 0x80000100
#define MB_E_GUT_INVALID_CPU_LINK 0x80000101
#define MB_E_GUT_INVALID_SRC_CPU 0x80000102
#define MB_E_GUT_INVALID_TARGET_CPU 0x80000103
#define MB_E_GUT_INVALID_CHANNEL_ID 0x80000104
#define MB_E_GUT_INVALID_CARRIER_ID 0x80000105
#define MB_E_GUT_REQUEST_CHANNEL 0x80000106
#define MB_E_GUT_RELEASE_CHANNEL 0x80000107
#define MB_E_GUT_TIMESTAMP_CHECK_FAIL 0x80000108
#define MB_E_GUT_WRITE_EXCEED_MAX_SIZE 0x80000109
#define MB_E_GUT_ALREADY_INIT 0x8000010a
#define MB_E_GUT_SEND_MAIL_IN_INT_CONTEXT 0x8000010b
#define MB_E_GUT_USER_BUFFER_SIZE_TOO_SMALL 0x8000010c
#define MB_E_GUT_INPUT_PARAMETER 0x8000010d
#define MB_E_GUT_NOT_INIT 0x8000010e
#define MB_E_GUT_MAILBOX_SEQNUM_CHECK_FAIL 0x8000010f
#define MB_E_GUT_MAILBOX_RECEIVE_FULL 0x80000110
#define MB_E_GUT_INIT_PLATFORM 0x80000112
#define MB_E_GUT_MAILBOX_NULL_PARAM 0x80000113
#define MB_E_GUT_CREATE_BOX 0x80000114
#define MB_E_GUT_MUTEX_CREATE_FAILED 0x80000115
#define MB_E_GUT_READ_CALLBACK_NOT_FIND 0x80000116
#define MB_E_GUT_INVALID_USER_ID 0x80000117
#define MB_E_GUT_INIT_CHANNEL_POOL_TOO_LARGE 0x80000118
#define MB_E_GUT_INIT_USER_POOL_TOO_LARGE 0x80000119
#define MB_E_GUT_CALCULATE_SPACE 0x8000011a
#define MB_E_GUT_INIT_CORESHARE_MEM 0x8000011b

#define MB_E_VXWORKS_TASK_CREATE 0x80000140
#define MB_E_VXWORKS_CALLBACK_NOT_FIND 0x80000141
#define MB_E_VXWORKS_CALLBACK_ERRO 0x80000142
#define MB_E_VXWORKS_MAIL_TASK_NOT_FIND 0x80000143
#define MB_E_VXWORKS_MAIL_INT_NOT_FIND 0x80000144
#define MB_E_VXWORKS_CHANNEL_NOT_FIND 0x80000145
#define MB_E_VXWORKS_ALLOC_MEMORY 0x80000146

#define MB_E_MCU_CHANNEL_NOT_FIND 0x80000160
#define MB_E_MCU_ZOS_MSG_ALLOC_FAIL 0x80000161
#define MB_E_MCU_ZOS_MSG_SEND_FAIL 0x80000162
#define MB_E_MCU_ZOS_PID_NOT_FIND 0x80000163
#define MB_E_MCU_ZOS_CBFUNC_NULL 0x80000164

#define MB_E_LINUX_TASK_CREATE 0x80000180
#define MB_E_LINUX_CALLBACK_NOT_FIND 0x80000181
#define MB_E_LINUX_CALLBACK_ERRO 0x80000182
#define MB_E_LINUX_MAIL_TASK_NOT_FIND 0x80000183
#define MB_E_LINUX_MAIL_INT_NOT_FIND 0x80000184
#define MB_E_LINUX_CHANNEL_NOT_FIND 0x80000185
#define MB_E_LINUX_ALLOC_MEMORY 0x80000186

#define MB_E_RET_END 0x800001ff

#define MB_W_RET_START 0x80000200
#define MB_W_USER_CALLBACK_ALREADY_EXIST 0x80000201
#define MB_W_TRANSPORT_TIME_OUT 0x80000202
#define MB_W_RECEIVE_TIME_OUT 0x80000203
#define MB_W_SCHE_TIME_OUT 0x80000204
#define MB_W_RET_END 0x800002ff

#define MB_I_RET_START 0x80000300
#define MB_I_RECEIVE_FIRST_MAIL 0x80000301
#define MB_I_SEND_FIRST_MAIL 0x80000302
#define MB_I_RET_END 0x800003ff

#define MAILBOX_BOARDST_USER_PROTECT1 0x18273645

#define MAILBOX_MCU_TEST_BUFF_SIZE 256

struct mb_st_msg {
	uint32_t protect;
	uint32_t length;
	uint32_t back_code;
	uint32_t test_id;
};

enum mailbox_boardst_test_id {
	MAILBOX_BOARDST_ID_LOOP_SEND,
	MAILBOX_BOARDST_ID_LOOP_BACK,
	MAILBOX_BOARDST_ID_LOOP_FINISH
};

struct mb_log {
	uint32_t erro_num;
	unsigned long param1;
	unsigned long param2;
	uint32_t line;
	const char *file;
};

struct mailbox_track {
	uint32_t send_slice;
	uint32_t recv_slice;
	unsigned long addr;
	unsigned short use_id;
	unsigned short reserved;
};

struct mb_slice {
	uint32_t total;
	uint32_t start;
	uint32_t max;
	uint32_t code;
	uint32_t overflow;
};

struct mb_mntn {
	uint32_t peak_traffic_left;

	struct mb_slice trans;
	struct mb_slice deal;
	struct mb_slice sche;

	uint32_t track_prob;
	struct mailbox_track track_array[MAILBOX_RECORD_USEID_NUM];
	struct mb_buff *mbuff;
};

int mailbox_log_erro(uint32_t err_no, unsigned long param1,
	unsigned long param2, uint32_t line_no, const char *file_name);

void mailbox_record_send(struct mb_mntn *mntn, uint32_t mailcode,
	uint32_t time_stamp, unsigned long mail_addr);

void mailbox_record_transport(struct mb_mntn *mntn, uint32_t mailcode,
	uint32_t write_slice, uint32_t read_slice,
	unsigned long mail_addr);

void mailbox_record_receive(struct mb_mntn *mntn, uint32_t mailcode,
	uint32_t slice_start);

void mailbox_record_sche_send(void *priv);

void mailbox_record_sche_recv(void *priv);

void mailbox_usb_suspend(bool is_suspend);

#endif /* __DRV_MAILBOX_DEBUG_H__ */
