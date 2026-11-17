/*
 * soundtrigger_socdsp_mailbox.h
 *
 * soundtrigger mailbox message process interface
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef __SOUNDTRIGGER_SOCDSP_MAILBOX_H__
#define __SOUNDTRIGGER_SOCDSP_MAILBOX_H__

enum wakeup_chn_msg_type {
	WAKEUP_CHN_MSG_START = 0xFD01,
	WAKEUP_CHN_MSG_START_ACK = 0xFD08,
	WAKEUP_CHN_MSG_STOP = 0xFD02,
	WAKEUP_CHN_MSG_STOP_ACK = 0xFD09,
	WAKEUP_CHN_MSG_PARAMETER_SET = 0xFD03,
	WAKEUP_CHN_MSG_PARAMETER_SET_ACK = 0xFD0A,
	WAKEUP_CHN_MSG_PARAMETER_GET = 0xFD04,
	WAKEUP_CHN_MSG_PARAMETER_GET_ACK = 0xFD0B,
	WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV = 0xFD05,
	WAKEUP_CHN_MSG_ELAPSED_RCV = 0xFD06,
	WAKEUP_CHN_MSG_WAKEUP_SET_MUTE = 0xFD30,
};

/* 0xFD1C--0xFD1F are used inside soc dsp, should not be mailbox message */
enum lp_wakeup_chn_msg_type {
	/* AP <--> SOC DSP */
	LP_WAKEUP_CHN_MSG_START = 0xFD11,
	LP_WAKEUP_CHN_MSG_STOP = 0xFD12,
	LP_WAKEUP_CHN_MSG_PARAMETER_SET = 0xFD13,
	LP_WAKEUP_CHN_MSG_PARAMETER_GET = 0xFD14,

	/* SOC DSP <--> AP */
	LP_WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV = 0xFD15,
	LP_WAKEUP_CHN_MSG_ELAPSED_RCV = 0xFD16,
	LP_WAKEUP_CHN_MSG_START_ACK = 0xFD17,
	LP_WAKEUP_CHN_MSG_STOP_ACK = 0xFD18,
	LP_WAKEUP_CHN_MSG_PARAMETER_SET_ACK = 0xFD19,
	LP_WAKEUP_CHN_MSG_PARAMETER_GET_ACK = 0xFD1A,
	LP_WAKEUP_CHN_MSG_MMI_MAD_INTR_ACK = 0xFD1B,
	LP_WAKEUP_CHN_MSG_ELAPSED_TIMEOUT_ACK = 0xFD10,
};

struct parameter_set {
	int key;
	union {
		int value;
		struct {
			uint32_t index;
			uint32_t length;
			char piece[0];
		} model;
	};
};

#define WAKEUP_CHN_COMMON   \
	uint16_t msg_type;   \
	uint16_t reserved;

struct soundtrigger_sync_msg {
	WAKEUP_CHN_COMMON
	int module_id;
	struct parameter_set set_param;
};

struct parameter_set_msg {
	WAKEUP_CHN_COMMON
	uint32_t module_id;
	struct parameter_set para;
};

struct parameter_get_msg {
	WAKEUP_CHN_COMMON
};

struct wakeup_start_msg {
	WAKEUP_CHN_COMMON
	uint32_t module_id;
};

struct wakeup_stop_msg {
	WAKEUP_CHN_COMMON
	uint32_t module_id;
};

struct wakeup_period_elapsed {
	uint32_t seq;
	uint32_t start;
	uint32_t len;
};

struct soundtrigger_rcv_msg {
	WAKEUP_CHN_COMMON
	union {
		int socdsp_handle;
		uint32_t fast_len;
		struct wakeup_period_elapsed elapsed;
	};
	char *elapsed_buffer;
};

struct msg_node {
	struct completion *msg_complete;
	uint16_t msg_type;
	uint16_t recv_msg_type;
	void (*recv_msg_proc)(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete);
	char msg_name[40];
};

int start_recognition_msg(uint16_t msg_type, int module_id, bool is_lp);
int stop_recognition_msg(uint16_t msg_type, int module_id, bool is_lp);
int get_handle_msg(uint16_t msg_type, int *socdsp_handle);
int parameter_set_msg(uint16_t msg_type, int module_id, const struct parameter_set *set_val);
int soundtrigger_mailbox_init(void);
void soundtrigger_mailbox_deinit(void);

#endif /* __SOUNDTRIGGER_SOCDSP_MAILBOX_H__ */