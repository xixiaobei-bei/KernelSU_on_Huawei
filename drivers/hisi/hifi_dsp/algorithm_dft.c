/*
 * Audio Algorithm DFT Module.
 *
 * Copyright (c) 2013 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <asm/io.h>

#include "huawei_platform/log/imonitor.h"
#include "huawei_platform/log/imonitor_keys.h"

#include "dsp_om.h"
#include "dsp_misc.h"
#include "algorithm_dft.h"


static void convert_data_to_hex_string(char *dst_str, const uint32_t str_len,
	const uint16_t *data, const uint32_t data_len)
{
	uint32_t i;
	uint32_t msg_len = str_len / BITS_TO_PRINT_EACH_NUMBER;

	SMARTPA_DFT_ASSERT(dst_str != NULL);
	SMARTPA_DFT_ASSERT(data != NULL);

	SMARTPA_DFT_ASSERT(data_len <= msg_len);

	for (i = 0; i < msg_len; ++i)
		snprintf(dst_str + BITS_TO_PRINT_EACH_NUMBER * i, BITS_TO_PRINT_EACH_NUMBER + 1, "%04x", data[i]);
	dst_str[str_len - 1] = '\0';
}

static void smartpa_dft_report_process_error(const struct smartpa_dft_process_error *msg)
{
	char str[MSG_SIZE_65_BYTES];
	struct imonitor_eventobj *obj = NULL;
	SMARTPA_DFT_ASSERT(msg != NULL);
	obj = imonitor_create_eventobj(msg->dft_id);

	imonitor_set_param_integer_v2(obj, "EventLevel", msg->err_code);

	memset(str, 0, sizeof(str));
	convert_data_to_hex_string(str, sizeof(str),
		msg->event_module, sizeof(msg->event_module) / sizeof(msg->event_module[0]));
	imonitor_set_param_string_v2(obj, "EventModule", str);

	imonitor_send_event(obj);
	imonitor_destroy_eventobj(obj);
}

static void smartpa_dft_report_abnormal_gain(const struct smartpa_dft_gain_protection *msg)
{
	char str[MSG_SIZE_65_BYTES];
	struct imonitor_eventobj *obj = NULL;
	SMARTPA_DFT_ASSERT(msg != NULL);
	obj = imonitor_create_eventobj(msg->dft_id);

	imonitor_set_param_integer_v2(obj, "EventLevel", msg->err_code);

	memset(str, 0, sizeof(str));
	convert_data_to_hex_string(str, sizeof(str),
		msg->event_module, sizeof(msg->event_module) / sizeof(msg->event_module[0]));
	imonitor_set_param_string_v2(obj, "EventModule", str);

	memset(str, 0, sizeof(str));
	convert_data_to_hex_string(str, sizeof(str),
		msg->err_pos, sizeof(msg->err_pos) / sizeof(msg->err_pos[0]));
	imonitor_set_param_string_v2(obj, "errPosTag", str);

	imonitor_send_event(obj);
	imonitor_destroy_eventobj(obj);
}

void hifi_om_work_smartpa_dft_report(const enum smartpa_dft_id *data)
{
	if (data == NULL)
		return;

	switch (*data) {
	case DFT_ID_PROCESS_ERROR:
		smartpa_dft_report_process_error((const struct smartpa_dft_process_error*)data);
		break;
	case DFT_ID_FRES_ERROR:
	case DFT_ID_RDC_ERROR:
	case DFT_ID_TEMP_ERROR:
		smartpa_dft_report_abnormal_gain((const struct smartpa_dft_gain_protection*)data);
		break;
	default:
		break;
	}
}

void hifi_om_work_audio_db(const enum smartpa_dft_id *data)
{
	if (data == NULL)
		return;

	switch (*data) {
	case DFT_ID_AUDIO_DB:
		smartpa_dft_report_process_error((const struct smartpa_dft_process_error*)data);
		break;
	default:
		break;
	}
}


