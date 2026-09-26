/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: big data channel source file
 * Author: DIVS_SENSORHUB
 * Create: 2012-05-29
 */

#include "big_data_channel.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <huawei_platform/log/hwlog_kernel.h>
#include <huawei_platform/log/imonitor.h>
#include <securec.h>

#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_ext_log.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "sensor_config.h"
#include "sensor_detect.h"
#include "sensor_sysfs.h"

#define NO_TAG (-1)

/* system load bigdata */
#define SYSLOAD_MAX_TASK_NAME 16
#define SYSLOAD_TASK_NAME_BUFF_LENGTH (2 * SYSLOAD_MAX_TASK_NAME)
#define SYSLOAD_PERIOD_UPLOAD_FILED_NUM 8
#define SYSLOAD_PERIOD_UPLOAD_TASK_NUM 5
#define SYSLOAD_IDX0 0
#define SYSLOAD_IDX1 1
#define SYSLOAD_IDX2 2
#define SYSLOAD_IDX3 3
#define SYSLOAD_IDX4 4
#define SYSLOAD_IDX5 5
#define SYSLOAD_IDX6 6
#define SYSLOAD_IDX7 7
#define SYSLOAD_OFFSET 1

struct sysload_peak {
	uint32_t max_time;
	uint32_t all_time;
	uint32_t rtc;
};

struct sysload_period {
	char task_name[SYSLOAD_MAX_TASK_NAME];
	struct sysload_peak peak;
	uint32_t run_time;
	uint32_t over_time;
	uint32_t over_cnt;
	uint32_t threshold;
};

struct sysload_trigger {
	char task_name[SYSLOAD_MAX_TASK_NAME];
	uint32_t max_time;
	uint32_t all_time;
};

const char *sysload_str[SYSLOAD_PERIOD_UPLOAD_FILED_NUM] = {
	"name_",
	"threshold_",
	"max_time_",
	"all_time_",
	"rtc_",
	"run_time_",
	"over_threshold_cnt_",
	"over_threshold_time_",
};

static char sysload_name_buf[SYSLOAD_TASK_NAME_BUFF_LENGTH];

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
	{"Apchipstate", INT_PARAM, NO_TAG}
};

static big_data_param_detail_t event_vib_resp_time[] = {
	{ "sh_vib_resp_time", INT_PARAM, NO_TAG }
};

static big_data_param_detail_t event_ps_sound_param[] = {
	{ "Num_Call1", INT_PARAM, NO_TAG },
	{ "Num_Call2", INT_PARAM, NO_TAG },
	{ "Num_Call3", INT_PARAM, NO_TAG },
	{ "Num_Call4", INT_PARAM, NO_TAG },
	{ "Num_Entering1", INT_PARAM, NO_TAG },
	{ "Num_Entering2", INT_PARAM, NO_TAG },
	{ "Num_Entering3", INT_PARAM, NO_TAG },
	{ "Num_Entering4", INT_PARAM, NO_TAG },
	{ "Num_Leaving1", INT_PARAM, NO_TAG },
	{ "Num_Leaving2", INT_PARAM, NO_TAG },
	{ "Num_Leaving3", INT_PARAM, NO_TAG },
	{ "Num_Leaving4", INT_PARAM, NO_TAG },
	{ "Num_Within1", INT_PARAM, NO_TAG },
	{ "Num_Within2", INT_PARAM, NO_TAG },
	{ "Num_Within3", INT_PARAM, NO_TAG },
	{ "Num_Within4", INT_PARAM, NO_TAG }
};

static big_data_param_detail_t event_aod_info_param[] = {
	{ "aod_play_time", INT_PARAM, NO_TAG },
	{ "swing_animation_count", INT_PARAM, NO_TAG }
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
	{ BIG_DATA_EVENT_PS_SOUND_INFO, 16, event_ps_sound_param },
	{ BIG_DATA_EVENT_AOD_INFO, 2, event_aod_info_param },
};

/* 3.(optional)map tag to str */
static const char *big_data_str_map[] = {
	[BIG_DATA_STR] = "BIG_DATA_STR",
};


static int iomcu_big_data_fetch(uint32_t event_id, void *data, uint32_t length)
{
	struct write_info pkg_ap;
	struct read_info pkg_mcu;
	int ret;

	memset(&pkg_ap, 0, sizeof(pkg_ap));
	memset(&pkg_mcu, 0, sizeof(pkg_mcu));

	pkg_ap.tag = TAG_BIG_DATA;
	pkg_ap.cmd = CMD_BIG_DATA_REQUEST_DATA;
	pkg_ap.wr_buf = &event_id;
	pkg_ap.wr_len = sizeof(event_id);

	if (get_iom3_state() != IOM3_ST_NORMAL) {
		hwlog_err("%s fail g_iom3_state=%d\n", __func__,
			get_iom3_state());
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

static int do_sysload_trigger_upload(struct imonitor_eventobj *obj, void *data)
{
	struct sysload_trigger *raw_data = (struct sysload_trigger *)data;
	char *task_name = raw_data->task_name;
	int ret = 0;

	ret += imonitor_set_param_string_v2(obj, "name", task_name);
	ret += imonitor_set_param_integer_v2(obj, "max_time",
					     raw_data->max_time);
	ret += imonitor_set_param_integer_v2(obj, "all_time",
					     raw_data->all_time);

	return ret;
}

static int do_sysload_period_upload(struct imonitor_eventobj *obj, void *data)
{
	struct sysload_period *raw_data = (struct sysload_period *)data;
	char *param_name = &sysload_name_buf[0];
	int ret = 0;
	int i;

	for (i = 0; i < SYSLOAD_PERIOD_UPLOAD_TASK_NUM; i++) {
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX0], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_string_v2(obj, param_name,
						    raw_data[i].task_name);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX1], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].threshold);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX2], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].peak.max_time);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX3], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].peak.all_time);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX4], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].peak.rtc);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX5], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].run_time);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX6], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].over_cnt);
		if (sprintf_s(param_name, sizeof(sysload_name_buf), "%s%d",
		    sysload_str[SYSLOAD_IDX7], i + SYSLOAD_OFFSET) < 0)
			return 1;
		ret += imonitor_set_param_integer_v2(obj, param_name,
						     raw_data[i].over_time);
	}
	return ret;
}

static int process_sysload_big_data(uint32_t event_id, void *data)
{
	struct imonitor_eventobj *obj = NULL;
	int ret = 0;

	if (!data) {
		hwlog_err("%s para error\n", __func__);
		return -1;
	}

	obj = imonitor_create_eventobj(event_id);
	if (!obj) {
		hwlog_err("%s imonitor_create_eventobj failed\n", __func__);
		return -1;
	}

	if (event_id == BIG_DATA_EVENT_SYSLOAD_TRIGGER)
		ret = do_sysload_trigger_upload(obj, data);
	else if (event_id == BIG_DATA_EVENT_SYSLOAD_PERIOD)
		ret = do_sysload_period_upload(obj, data);

	if (ret) {
		imonitor_destroy_eventobj(obj);
		hwlog_err("%s imonitor_set_para fail, ret %d\n", __func__, ret);
		return ret;
	}

	ret = imonitor_send_event(obj);
	if (ret < 0)
		hwlog_err("%s imonitor_send_event fail, ret %d\n", __func__,
			  ret);

	imonitor_destroy_eventobj(obj);
	hwlog_info("%s big data imonitor send event id: %d success\n", __func__,
		   event_id);

	return ret;
}

static int iomcu_big_data_process(const struct pkt_header *head)
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
	case BIG_DATA_EVENT_AOD_INFO:
		process_big_data(report_t->event_id, data);
		break;
	case BIG_DATA_EVENT_PS_SOUND_INFO:
		process_big_data(report_t->event_id, data);
		break;
	case BIG_DATA_EVENT_SYSLOAD_PERIOD:
	case BIG_DATA_EVENT_SYSLOAD_TRIGGER:
		process_sysload_big_data(report_t->event_id, data);
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
