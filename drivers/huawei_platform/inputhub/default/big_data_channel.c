/*
 * big_data_channel.c
 *
 * big_data_channel source file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/slab.h>
#include "contexthub_route.h"
#include "sensor_config.h"
#include "contexthub_debug.h"
#include "contexthub_recovery.h"
#include "contexthub_pm.h"
#include "sensor_sysfs.h"
#include "contexthub_ext_log.h"
#include "sensor_detect.h"
#include <huawei_platform/log/hwlog_kernel.h>
#include <huawei_platform/log/imonitor.h>
#include "big_data_channel.h"

#define NO_TAG (-1)

/* 1.event parameter setting */
static big_data_param_detail_t event_motion_type_param[] = {
	{ "Pickup", INT_PARAM, MOTION_TYPE_PICKUP },
	{ "Flip", INT_PARAM, MOTION_TYPE_FLIP },
	{ "Proximity", INT_PARAM, MOTION_TYPE_PROXIMITY },
	{ "Shake", INT_PARAM, MOTION_TYPE_SHAKE },
	{ "TiltLr", INT_PARAM, MOTION_TYPE_TILT_LR },
	{ "Pocket", INT_PARAM, MOTION_TYPE_POCKET },
	{ "Rotation", INT_PARAM, MOTION_TYPE_ROTATION },
	{ "Activity", INT_PARAM, MOTION_TYPE_ACTIVITY },
	{ "TakeOff", INT_PARAM, MOTION_TYPE_TAKE_OFF },
	{ "HeadDown", INT_PARAM, MOTION_TYPE_HEAD_DOWN },
	{ "PutDown", INT_PARAM, MOTION_TYPE_PUT_DOWN },
	{ "Sidegrip", INT_PARAM, MOTION_TYPE_SIDEGRIP }
};

static big_data_param_detail_t event_ddr_info_param[] = {
	{ "Times", INT_PARAM, NO_TAG },
	{ "Duration", INT_PARAM, NO_TAG }
};

static big_data_param_detail_t event_tof_phonecall_param[] = {
	{ "Closest", INT_PARAM, NO_TAG },
	{ "Farthest", INT_PARAM, NO_TAG }
};

static big_data_param_detail_t event_phonecall_screen_param[] = {
	{"Times", INT_PARAM, NO_TAG}
};
static big_data_param_detail_t event_fold_temp_param[] = {
	{"Temperature", INT_PARAM, NO_TAG},
	{"Flag", INT_PARAM, NO_TAG}
};

static big_data_param_detail_t event_vib_resp_time[] = {
	{ "sh_vib_resp_time", INT_PARAM, NO_TAG }
};

/* 2.register event param here {EVENT_ID, param_num, param_detail_struct} */
static const big_data_event_detail_t big_data_event[] = {
	{ BIG_DATA_EVENT_MOTION_TYPE, 12, event_motion_type_param },
	{ BIG_DATA_EVENT_DDR_INFO, 2, event_ddr_info_param },
	{ BIG_DATA_EVENT_TOF_PHONECALL, 2, event_tof_phonecall_param },
	{ BIG_DATA_EVENT_PHONECALL_SCREEN_STATUS, 1,
		event_phonecall_screen_param },
	{ BIG_DATA_EVENT_VIB_RESP_TIME, 1, event_vib_resp_time },
	{ BIG_DATA_FOLD_TEMP, 2, event_fold_temp_param },
};

/* 3.(optional)map tag to str */
static const char *big_data_str_map[] = {
	[BIG_DATA_STR] = "BIG_DATA_STR",
};


static int iomcu_big_data_fetch(uint32_t event_id, void *data, uint32_t length)
{
	write_info_t pkg_ap;
	read_info_t pkg_mcu;
	int ret;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = TAG_BIG_DATA;
	pkg_ap.cmd = CMD_BIG_DATA_REQUEST_DATA;
	pkg_ap.wr_buf = &event_id;
	pkg_ap.wr_len = sizeof(event_id);

	if (g_iom3_state != IOM3_ST_NORMAL) {
		hwlog_err("%s fail g_iom3_state=%d\n", __func__, g_iom3_state);
		return -1;
	}

	ret = write_customize_cmd(&pkg_ap, &pkg_mcu, true);
	if (ret != 0) {
		hwlog_err("send big data fetch req type = %d fail\n", event_id);
		return -1;
	} else if (pkg_mcu.errno != 0) {
		hwlog_err("big data fetch req to mcu fail errno = %d\n",
			pkg_mcu.errno);
		return -1;
	} else {
		if (length < MAX_PKT_LENGTH)
			memcpy(data, pkg_mcu.data, length);
		else
			memcpy(data, pkg_mcu.data, MAX_PKT_LENGTH);
	}
	return 0;
}

int iomcu_dubai_log_fetch(uint32_t event_type, void *data, uint32_t length)
{
	int ret = -1;

	if (event_type > DUBAI_EVENT_END || event_type < DUBAI_EVENT_NULL) {
		hwlog_err("dubai log fetch event_type: %d illegal\n",
			event_type);
		return ret;
	}
	ret = iomcu_big_data_fetch(event_type, data, length);
	return ret;
}

static int process_big_data(uint32_t event_id, void *data)
{
	struct imonitor_eventobj *obj = NULL;
	int i;
	int ret = 0;
	int tag = 0;
	uint32_t *raw_data = (uint32_t *)data;
	big_data_event_detail_t event_detail;
	big_data_param_detail_t param_detail;

	memset(&event_detail, 0, sizeof(event_detail));
	memset(&param_detail, 0, sizeof(param_detail));

	if (!data) {
		hwlog_err("%s para error\n", __func__);
		return -1;
	}

	obj = imonitor_create_eventobj(event_id);

	if (!obj) {
		hwlog_err("%s imonitor_create_eventobj failed\n", __func__);
		return -1;
	}
	for (i = 0; i < sizeof(big_data_event) / sizeof(big_data_event[0]); ++i) {
		if (big_data_event[i].event_id == event_id) {
			memcpy(&event_detail, &big_data_event[i],
				sizeof(event_detail));
			break;
		}
	}

	for (i = 0; i < event_detail.param_num; ++i) {
		memcpy(&param_detail, &event_detail.param_data[i],
			sizeof(param_detail));
		tag = (param_detail.tag == NO_TAG) ? i : param_detail.tag;
		if (param_detail.param_type == INT_PARAM) {
			ret += imonitor_set_param_integer_v2(obj,
				param_detail.param_name, raw_data[tag]);
		} else if (param_detail.param_type == STR_PARAM) {
			if (big_data_str_map[raw_data[tag]])
				ret += imonitor_set_param_string_v2(obj,
					param_detail.param_name,
					big_data_str_map[raw_data[tag]]);
		}
	}

	if (ret) {
		imonitor_destroy_eventobj(obj);
		hwlog_err("%s imonitor_set_para fail, ret %d\n", __func__, ret);
		return ret;
	}

	ret = imonitor_send_event(obj);
	hwlog_info("big data imonitor send event id: %d success\n", event_id);
	if (ret < 0)
		hwlog_err("%s imonitor_send_event fail, ret %d\n",
			__func__, ret);

	imonitor_destroy_eventobj(obj);
	return ret;
}

static int iomcu_big_data_process(const pkt_header_t *head)
{
	uint32_t *data = NULL;
	pkt_big_data_report_t *report_t = NULL;

	if (!head)
		return -1;

	report_t = (pkt_big_data_report_t *)head;
	data = (uint32_t *)&report_t[1];
	switch (report_t->event_id) {
	case DUBAI_EVENT_AOD_PICKUP:
		hwlog_info("DUBAI_EVENT_AOD_PICKUP: %d\n", data[0]);
		HWDUBAI_LOGE("DUBAI_TAG_TP_AOD", "type=%d data=%d",
			DUBAI_EVENT_AOD_PICKUP, data[0]);
		break;
	case DUBAI_EVENT_AOD_PICKUP_NO_FINGERDOWN:
		hwlog_info("DUBAI_EVENT_AOD_PICKUP_NO_FINGERDOWN: %d\n",
			data[0]);
		HWDUBAI_LOGE("DUBAI_TAG_TP_AOD", "type=%d data=%d",
			DUBAI_EVENT_AOD_PICKUP_NO_FINGERDOWN, data[0]);
		break;
	case BIG_DATA_EVENT_MOTION_TYPE:
	case BIG_DATA_EVENT_DDR_INFO:
	case BIG_DATA_EVENT_TOF_PHONECALL:
	case BIG_DATA_EVENT_PHONECALL_SCREEN_STATUS:
	case BIG_DATA_EVENT_VIB_RESP_TIME:
	case BIG_DATA_FOLD_TEMP:
		process_big_data(report_t->event_id, data);
		break;
	default:
		hwlog_info("iomcu_big_data_process no matched event id: %d\n",
			report_t->event_id);
		break;
	}

	return 0;
}

static int iomcu_big_data_init(void)
{
	if (is_sensorhub_disabled())
		return -1;

	register_mcu_event_notifier(TAG_BIG_DATA,
		CMD_BIG_DATA_SEND_TO_AP_RESP, iomcu_big_data_process);
	hwlog_info("iomcu_big_data_init success\n");
	return 0;
}

late_initcall_sync(iomcu_big_data_init);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("SensorHub big data channel");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
