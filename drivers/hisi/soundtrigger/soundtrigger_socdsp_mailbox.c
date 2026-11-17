/*
 * soundtrigger_socdsp_mailbox.c
 *
 * soundtrigger mailbox message process implement
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

#include <linux/proc_fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/errno.h>
#include <linux/hisi/audio_log.h>

#include "dsp_misc.h"
#include "drv_mailbox_msg.h"
#include "soundtrigger_event.h"
#include "soundtrigger_socdsp_pcm.h"
#include "soundtrigger_socdsp_mailbox.h"
#include "da_combine_algo_interface.h"

#define LOG_TAG "soundtrigger"

struct completion start_msg_complete;
struct completion stop_msg_complete;
struct completion parameter_set_msg_complete;
struct completion parameter_get_msg_complete;
struct completion lp_start_msg_complete;
struct completion lp_stop_msg_complete;
struct completion lp_parameter_set_msg_complete;
struct completion lp_parameter_get_msg_complete;
struct wakeup_source soundtrigger_rcv_wake_lock;

#define SOCDSP_WAKEUP_MSG_TIMEOUT (2 * HZ)
#define NORMAL_BUFFER_LEN 640 /* 16K*1ch*2byte*20ms */

static int g_socdsp_handle;
static int g_socdsp_lp_handle;
static char *g_model_buf;
static bool wakeup_is_start;
static bool wakeup_lp_is_start;

static void soundtrigger_hotword_detected_work(struct work_struct *wk)
{
	hw_soundtrigger_event_uevent(AUDIO_SOUNDTRIGGER_TRIGGER);
}

static void soundtrigger_set_mute_work(struct work_struct *wk)
{
	hw_soundtrigger_event_uevent(AUDIO_SOUNDTRIGGER_SETMUTE);
}

static DECLARE_WORK(hotword_detected_queue, soundtrigger_hotword_detected_work);
static DECLARE_WORK(set_mute_queue, soundtrigger_set_mute_work);

int soundtrigger_mailbox_send_data(const void *pmsg_body, uint32_t msg_len, uint32_t msg_priority)
{
	uint32_t ret;

	ret = mailbox_send_msg(MAILBOX_MAILCODE_ACPU_TO_HIFI_WAKEUP, pmsg_body, msg_len);
	if (ret != 0)
		AUDIO_LOGE("soundtrigger channel send mail failed, ret=%u", ret);

	return (int)ret;
}

static void msg_complete_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	if (msg_complete)
		complete(msg_complete);
}

static void wakeup_get_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	g_socdsp_handle = rcv_msg->socdsp_handle;
	if (msg_complete)
		complete(msg_complete);
}

static void wakeup_hotword_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_flag_init(NORMAL_WAKEUP_MODE);
	soundtrigger_socdsp_pcm_fastbuffer_filled(NORMAL_WAKEUP_MODE, rcv_msg->fast_len);
	schedule_work(&hotword_detected_queue);
}

static void wakeup_elapsed_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_elapsed(NORMAL_WAKEUP_MODE, rcv_msg->elapsed.start, rcv_msg->elapsed.len);
}

static void wakeup_set_mute_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	schedule_work(&set_mute_queue);
}

static void wakeup_lp_get_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	g_socdsp_lp_handle = rcv_msg->socdsp_handle;
	if (msg_complete)
		complete(msg_complete);
}

static void wakeup_lp_hotword_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_flag_init(LP_WAKEUP_MODE);
	soundtrigger_socdsp_pcm_fastbuffer_filled(LP_WAKEUP_MODE, rcv_msg->fast_len);
	schedule_work(&hotword_detected_queue);
}

static void wakeup_lp_elapsed_ack_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	soundtrigger_socdsp_pcm_elapsed(LP_WAKEUP_MODE, rcv_msg->elapsed.start, rcv_msg->elapsed.len);
}

static void wakeup_lp_mmi_mad_intr_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	hw_soundtrigger_event_input(0);
}

static void wakeup_lp_elapsed_timeout_proc(struct soundtrigger_rcv_msg *rcv_msg, struct completion *msg_complete)
{
	hw_soundtrigger_event_uevent(AUDIO_SOUNDTRIGGER_TIMEOUT);
}

static struct msg_node soundtrigger_msg_table[] = {
	{
		&start_msg_complete,
		WAKEUP_CHN_MSG_START,
		WAKEUP_CHN_MSG_START_ACK,
		msg_complete_proc,
		"wakeup start"
	},

	{
		&stop_msg_complete,
		WAKEUP_CHN_MSG_STOP,
		WAKEUP_CHN_MSG_STOP_ACK,
		msg_complete_proc,
		"wakeup stop"
	},

	{
		&parameter_set_msg_complete,
		WAKEUP_CHN_MSG_PARAMETER_SET,
		WAKEUP_CHN_MSG_PARAMETER_SET_ACK,
		msg_complete_proc,
		"wakeup set para"
	},

	{
		&parameter_get_msg_complete,
		WAKEUP_CHN_MSG_PARAMETER_GET,
		WAKEUP_CHN_MSG_PARAMETER_GET_ACK,
		wakeup_get_ack_proc,
		"wakeup get para"
	},

	{
		NULL,
		0,
		WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV,
		wakeup_hotword_ack_proc,
		"wakeup hot word"
	},

	{
		NULL,
		0,
		WAKEUP_CHN_MSG_ELAPSED_RCV,
		wakeup_elapsed_ack_proc,
		"wakeup elapsed rcv"
	},

	{
		NULL,
		0,
		WAKEUP_CHN_MSG_WAKEUP_SET_MUTE,
		wakeup_set_mute_proc,
		"wakeup set mute"
	},

	{
		&lp_start_msg_complete,
		LP_WAKEUP_CHN_MSG_START,
		LP_WAKEUP_CHN_MSG_START_ACK,
		msg_complete_proc,
		"lp wakeup start"
	},

	{
		&lp_stop_msg_complete,
		LP_WAKEUP_CHN_MSG_STOP,
		LP_WAKEUP_CHN_MSG_STOP_ACK,
		msg_complete_proc,
		"lp wakeup stop"
	},

	{
		&lp_parameter_set_msg_complete,
		LP_WAKEUP_CHN_MSG_PARAMETER_SET,
		LP_WAKEUP_CHN_MSG_PARAMETER_SET_ACK,
		msg_complete_proc,
		"lp wakeup set para"
	},

	{
		&lp_parameter_get_msg_complete,
		LP_WAKEUP_CHN_MSG_PARAMETER_GET,
		LP_WAKEUP_CHN_MSG_PARAMETER_GET_ACK,
		wakeup_lp_get_ack_proc,
		"lp wakeup get para"
	},

	{
		NULL,
		0,
		LP_WAKEUP_CHN_MSG_HOTWORD_DETECT_RCV,
		wakeup_lp_hotword_ack_proc,
		"lp wakeup hot word"
	},

	{
		NULL,
		0,
		LP_WAKEUP_CHN_MSG_ELAPSED_RCV,
		wakeup_lp_elapsed_ack_proc,
		"lp wakeup elapsed rcv"
	},

	{
		NULL,
		0,
		LP_WAKEUP_CHN_MSG_MMI_MAD_INTR_ACK,
		wakeup_lp_mmi_mad_intr_proc,
		"lp wakeup mmi mad intr"
	},

	{
		NULL,
		0,
		LP_WAKEUP_CHN_MSG_ELAPSED_TIMEOUT_ACK,
		wakeup_lp_elapsed_timeout_proc,
		"lp wakeup elapsed timeout"
	}
};

static struct msg_node *select_msg_node(uint16_t msg_type)
{
	uint32_t i;

	for (i = 0; i < ARRAY_SIZE(soundtrigger_msg_table); i++) {
		if (msg_type == soundtrigger_msg_table[i].msg_type ||
			msg_type == soundtrigger_msg_table[i].recv_msg_type)
			return &soundtrigger_msg_table[i];
	}

	AUDIO_LOGE("soundtrigger msg type error, msg:0x%x", msg_type);

	return NULL;
}

static void soundtrigger_mailbox_recv_proc(const void *usr_para,
	struct mb_queue *mail_handle, uint32_t mail_len)
{
	struct soundtrigger_rcv_msg rcv_msg = { 0 };
	uint32_t ret;
	uint32_t mail_size = mail_len;
	struct msg_node *msg_info = NULL;

	ret = mailbox_read_msg_data(mail_handle, (char*)&rcv_msg, &mail_size);
	if ((ret != 0) || (mail_size == 0) || (mail_size > sizeof(rcv_msg))) {
		AUDIO_LOGE("mailbox read error, read result:%u, read mail size:%u", ret, mail_size);
		return;
	}

	__pm_wakeup_event(&soundtrigger_rcv_wake_lock, 1000);
	msg_info = select_msg_node(rcv_msg.msg_type);
	if (msg_info != NULL) {
		AUDIO_LOGI("receive message: %s succ", msg_info->msg_name);
		msg_info->recv_msg_proc(&rcv_msg, msg_info->msg_complete);
	}

	return;
}

static bool is_wakeup_start(bool is_lp)
{
	if (is_lp)
		return wakeup_lp_is_start;
	else
		return wakeup_is_start;
}

static void set_wakeup_start(bool is_lp, bool value)
{
	if (is_lp)
		wakeup_lp_is_start = value;
	else
		wakeup_is_start = value;
}

int parameter_set_msg(uint16_t msg_type, int module_id, const struct parameter_set *set_val)
{
	int ret;
	unsigned long ret_val;
	struct parameter_set_msg set_msg = { 0 };
	struct msg_node *parameter_set_msg_node = select_msg_node(msg_type);

	if (parameter_set_msg_node == NULL || parameter_set_msg_node->msg_complete == NULL)
		return -EINVAL;

	init_completion(parameter_set_msg_node->msg_complete);

	AUDIO_LOGI("parameter_set_msg");

	set_msg.msg_type = msg_type;
	set_msg.module_id = module_id;
	memcpy(&set_msg.para, set_val, sizeof(*set_val));

	if (set_msg.para.key == MLIB_ST_PARA_MODEL) {
		if (set_val->model.length > AP_AUDIO_WAKEUP_MODEL_SIZE) {
			AUDIO_LOGE("model length exceed %u", set_val->model.length);
			return -EINVAL;
		}

		if (g_model_buf == NULL) {
			AUDIO_LOGE("g_model_buf is NULL");
			return -EPIPE;
		}

		memcpy(g_model_buf, set_val->model.piece, set_val->model.length);
	}

	ret = soundtrigger_mailbox_send_data(&set_msg, sizeof(set_msg), 0);
	if (ret) {
		AUDIO_LOGE("soundtrigger_mailbox_send_data error:%d", ret);
		return ret;
	}

	ret_val = wait_for_completion_timeout(parameter_set_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (ret_val == 0) {
		AUDIO_LOGE("msg send timeout");
		return -ETIME;
	}

	return ret;
}

/*
 * Attention: don't check is_wakeup_start in start_recognition_msg,
 * when wakeup started and a phone call in, wake up was stopped in DSP, AP know nothing about that
 * when call ended, a start message need to sent to DSP to restart wakeup.
 */
int start_recognition_msg(uint16_t msg_type, int module_id, bool is_lp)
{
	int ret = 0;
	unsigned long ret_val;
	struct wakeup_start_msg start_msg = { 0 };
	struct msg_node *start_msg_node = select_msg_node(msg_type);

	if (start_msg_node == NULL || start_msg_node->msg_complete == NULL)
		return -EINVAL;

	init_completion(start_msg_node->msg_complete);

	AUDIO_LOGI("start_recognition_msg");
	start_msg.msg_type = msg_type;
	start_msg.module_id = module_id;

	ret = soundtrigger_mailbox_send_data(&start_msg, sizeof(start_msg), 0);
	if (ret) {
		AUDIO_LOGE("soundtrigger_mailbox_send_data error:%d", ret);
		return ret;
	}

	ret_val = wait_for_completion_timeout(start_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (ret_val == 0) {
		AUDIO_LOGE("msg send timeout");
		return -ETIME;
	}

	set_wakeup_start(is_lp, true);

	return ret;
}

int stop_recognition_msg(uint16_t msg_type, int module_id, bool is_lp)
{
	int ret = 0;
	unsigned long ret_val;
	struct wakeup_stop_msg stop_msg = { 0 };
	struct msg_node *stop_msg_node = select_msg_node(msg_type);

	if (stop_msg_node == NULL || stop_msg_node->msg_complete == NULL)
		return -EINVAL;

	if (!is_wakeup_start(is_lp)) {
		AUDIO_LOGI("recognition not started, is lp: %d", is_lp);
		return ret;
	}

	init_completion(stop_msg_node->msg_complete);

	AUDIO_LOGI("stop_recognition_msg");
	stop_msg.msg_type = msg_type;
	stop_msg.module_id = module_id;

	ret = soundtrigger_mailbox_send_data(&stop_msg, sizeof(stop_msg), 0);
	if (ret) {
		AUDIO_LOGE("soundtrigger_mailbox_send_data error:%d", ret);
		return ret;
	}

	ret_val = wait_for_completion_timeout(stop_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (ret_val == 0) {
		AUDIO_LOGE("msg send timeout");
		return -ETIME;
	}

	set_wakeup_start(is_lp, false);

	return ret;
}

int get_handle_msg(uint16_t msg_type, int *socdsp_handle)
{
	int ret;
	unsigned long ret_val;
	struct parameter_get_msg get_msg = { 0 };
	int *handle = NULL;
	struct msg_node *parameter_get_msg_node = select_msg_node(msg_type);

	if (parameter_get_msg_node == NULL || parameter_get_msg_node->msg_complete == NULL)
		return -EINVAL;

	if (msg_type == WAKEUP_CHN_MSG_PARAMETER_GET)
		handle = &g_socdsp_handle;
	else if (msg_type == LP_WAKEUP_CHN_MSG_PARAMETER_GET)
		handle = &g_socdsp_lp_handle;
	else
		return -EINVAL;

	init_completion(parameter_get_msg_node->msg_complete);

	AUDIO_LOGI("get_handle_msg");
	get_msg.msg_type = msg_type;
	*handle = 0;
	ret = soundtrigger_mailbox_send_data(&get_msg, sizeof(get_msg), 0);
	if (ret) {
		AUDIO_LOGE("soundtrigger_mailbox_send_data error:%d", ret);
		return ret;
	}

	ret_val = wait_for_completion_timeout(parameter_get_msg_node->msg_complete, SOCDSP_WAKEUP_MSG_TIMEOUT);
	if (ret_val == 0) {
		AUDIO_LOGE("get handle msg send timeout");
		return -ETIME;
	} else {
		*socdsp_handle = *handle;
	}

	return ret;
}

int soundtrigger_mailbox_init(void)
{
	int ret;

	AUDIO_LOGI("soundtrigger mailbox init");

	ret = mailbox_reg_msg_cb(MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP, soundtrigger_mailbox_recv_proc, NULL);
	if (ret) {
		AUDIO_LOGE("register receive func error, ret:%d, mailcode:0x%x",
			ret, MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP);
		return -EIO;
	}

	wakeup_source_init(&soundtrigger_rcv_wake_lock, "soundtrigger_rcv_msg");

	g_model_buf = ioremap_wc(AP_AUDIO_WAKEUP_MODEL_ADDR, AP_AUDIO_WAKEUP_MODEL_SIZE);
	if (g_model_buf == NULL) {
		AUDIO_LOGE("model buffer ioremap err");
		wakeup_source_trash(&soundtrigger_rcv_wake_lock);
		mailbox_reg_msg_cb(MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP, NULL, NULL);
		return -ENOMEM;
	}

	return ret;
}

void soundtrigger_mailbox_deinit(void)
{
	AUDIO_LOGI("soundtrigger_mailbox_deinit");

	if (g_model_buf != NULL) {
		iounmap(g_model_buf);
		g_model_buf = NULL;
	}

	wakeup_source_trash(&soundtrigger_rcv_wake_lock);
	mailbox_reg_msg_cb(MAILBOX_MAILCODE_HIFI_TO_ACPU_WAKEUP, NULL, NULL);
}
