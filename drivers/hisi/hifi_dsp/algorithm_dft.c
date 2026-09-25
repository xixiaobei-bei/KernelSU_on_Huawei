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

#include "hifi_lpp.h"
#include "hifi_om.h"
#include "algorithm_dft.h"


void smartpa_dft_report_process_error(const char *data)
{
	int ret;
	int i;
	char str[MSG_SIZE_128_BYTES];
	smartpa_dft_data_process_error *info = (smartpa_dft_data_process_error *)data;
	struct imonitor_eventobj *obj = NULL;
	SMARTPA_DFT_ASSERT((info != NULL));
	obj = imonitor_create_eventobj(SOC_SMARTPA_ERR_BASE_ID + SMARTPA_DFT_PROCESS_ERROR_ID);

	imonitor_set_param_integer_v2(obj, "EventLevel", info->err_level);
	imonitor_set_param_integer_v2(obj, "ErrCode", info->err_code);
	snprintf(str, MSG_SIZE_64_BYTES, "timeAtUl_%uchip_%s",
		info->raw_data.timestamp_at_upload / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
		info->raw_data.chip_name);
	imonitor_set_param_string_v2(obj, "EventModule", str);

	for (i = 0; i < info->data_set_index && i < DFT_PROCESS_ERROR_REPORT_MSG_DATA_SET_SIZE; i++) {
		snprintf(str, MSG_SIZE_128_BYTES, "time_%uerror_%dscene_%d",
			info->raw_data.data_set[i].timestamp / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
			info->raw_data.data_set[i].error_code,
			info->raw_data.data_set[i].scene_id);
		imonitor_set_param_integer_v2(obj, str, 0);
	}

	ret = imonitor_send_event(obj);
	logi("send event to imonitor, ret = %d\n", ret);

	imonitor_destroy_eventobj(obj);
}

void smartpa_dft_report_f0_error(const char *data)
{
	int ret;
	int i;
	char str[MSG_SIZE_128_BYTES];
	smartpa_dft_data_f0_error *info = (smartpa_dft_data_f0_error *)data;
	struct imonitor_eventobj *obj = NULL;
	SMARTPA_DFT_ASSERT((info != NULL));
	obj = imonitor_create_eventobj(SOC_SMARTPA_ERR_BASE_ID + SMARTPA_DFT_F0_ERROR_ID);

	imonitor_set_param_integer_v2(obj, "EventLevel", info->err_level);
	imonitor_set_param_integer_v2(obj, "ErrCode", info->err_code);
	snprintf(str, MSG_SIZE_64_BYTES, "timeAtUl_%uchip_%s,box%u",
		info->raw_data.timestamp_at_upload / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
		info->raw_data.chip_name, info->raw_data.box_vendor);
	imonitor_set_param_string_v2(obj, "EventModule", str);

	for (i = 0; i < info->data_set_index && i < DFT_F0_ERROR_REPORT_MSG_DATA_SET_SIZE; i++) {
		snprintf(str, MSG_SIZE_128_BYTES, "time_%uch_%uscene_%uf0_%u",
			info->raw_data.data_set[i].timestamp / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
			info->raw_data.data_set[i].channel_index,
			info->raw_data.data_set[i].scene_id,
			info->raw_data.data_set[i].f0_value);
		imonitor_set_param_integer_v2(obj, str, 0);
	}

	ret = imonitor_send_event(obj);
	logi("send event to imonitor, ret = %d\n", ret);

	imonitor_destroy_eventobj(obj);
}

void smartpa_dft_report_resistance_error(const char *data)
{
	int ret;
	int i;
	char str[MSG_SIZE_128_BYTES];
	smartpa_dft_data_resistance_error *info = (smartpa_dft_data_resistance_error *)data;
	struct imonitor_eventobj *obj = NULL;
	SMARTPA_DFT_ASSERT((info != NULL));
	obj = imonitor_create_eventobj(SOC_SMARTPA_ERR_BASE_ID +
					SMARTPA_DFT_RESISTANCE_ERROR_ID);

	imonitor_set_param_integer_v2(obj, "EventLevel", info->err_level);
	imonitor_set_param_integer_v2(obj, "ErrCode", info->err_code);
	snprintf(str, MSG_SIZE_64_BYTES, "timeAtUl_%uchip_%s,box%u",
		info->raw_data.timestamp_at_upload / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
		info->raw_data.chip_name, info->raw_data.box_vendor);
	imonitor_set_param_string_v2(obj, "EventModule", str);

	for (i = 0; i < info->data_set_index && i < DFT_RESISTANCE_ERROR_REPORT_MSG_DATA_SET_SIZE; i++) {
		snprintf(str, MSG_SIZE_128_BYTES, "time_%uch_%uscene_%uR_%ucalib_%u",
			info->raw_data.data_set[i].timestamp / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
			info->raw_data.data_set[i].channel_index,
			info->raw_data.data_set[i].scene_id,
			info->raw_data.data_set[i].resistance_value,
			info->raw_data.data_set[i].calibration_value);
		imonitor_set_param_integer_v2(obj, str, 0);
	}

	ret = imonitor_send_event(obj);
	logi("send event to imonitor, ret = %d\n", ret);

	imonitor_destroy_eventobj(obj);
}

void smartpa_dft_report_overheat_error(const char *data)
{
	int ret;
	char str[MSG_SIZE_128_BYTES];
	int i;
	smartpa_dft_data_overheat_error *info = (smartpa_dft_data_overheat_error *)data;
	struct imonitor_eventobj *obj = NULL;
	SMARTPA_DFT_ASSERT((info != NULL));
	obj = imonitor_create_eventobj(SOC_SMARTPA_ERR_BASE_ID + SMARTPA_DFT_OVERHEAT_ERROR_ID);

	snprintf(str, MSG_SIZE_64_BYTES, "timeAtUl_%uchip_%s,box%u",
		info->raw_data.timestamp_at_upload / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
		info->raw_data.chip_name, info->raw_data.box_vendor);
	imonitor_set_param_string_v2(obj, "EventModule", str);

	for (i = 0; i < info->data_set_index && i < DFT_OVERHEAT_ERROR_REPORT_MSG_DATA_SET_SIZE; i++) {
		snprintf(str, MSG_SIZE_128_BYTES, "time_%uch_%uscene_%utemp1_%d_temp2_%dgain1_0.%dgain2_0.%dcalib1_%dcalib2_%d",
			info->raw_data.data_set[i].timestamp / HIFI_TIMESTAMP_CYCLES_PER_SECOND,
			info->raw_data.data_set[i].channel_index,
			info->raw_data.data_set[i].scene_id,
			info->raw_data.data_set[i].temperature[0],
			info->raw_data.data_set[i].temperature[1],
			info->raw_data.data_set[i].current_gain[0],
			info->raw_data.data_set[i].current_gain[1],
			info->raw_data.data_set[i].calibration_value[0],
			info->raw_data.data_set[i].calibration_value[1]);
		imonitor_set_param_integer_v2(obj, str, 0);
	}

	ret = imonitor_send_event(obj);
	logi("send event to imonitor, ret = %d\n", ret);

	imonitor_destroy_eventobj(obj);
}


