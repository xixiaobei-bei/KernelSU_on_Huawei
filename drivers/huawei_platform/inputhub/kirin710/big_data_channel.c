/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: Sensor Hub Channel Bridge
 * Author: jincanran
 * Create: 2018-06-01
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <huawei_platform/log/hwlog_kernel.h>
#include <huawei_platform/log/imonitor.h>

#include "sensor_config.h"
#include "contexthub_debug.h"
#include "contexthub_ext_log.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"
#include "sensor_detect.h"
#include "sensor_sysfs.h"

#include "big_data_channel.h"

static big_data_param_detail_t event_motion_type_param[] = {
	{ "Pickup", INT_PARAM },
	{ "Flip", INT_PARAM },
	{ "Proximity", INT_PARAM },
	{ "Shake", INT_PARAM },
	{ "TiltLr", INT_PARAM },
	{ "Pocket", INT_PARAM },
	{ "Rotation", INT_PARAM },
	{ "Activity", INT_PARAM },
	{ "TakeOff", INT_PARAM },
	{ "HeadDown", INT_PARAM },
	{ "PutDown", INT_PARAM },
	{ "Sidegrip", INT_PARAM }
};

static big_data_param_detail_t event_ddr_info_param[] = {
	{ "Times", INT_PARAM },
	{ "Duration", INT_PARAM }
};

static big_data_param_detail_t event_tof_phonecall_param[] = {
	{ "Closest", INT_PARAM },
	{ "Farthest", INT_PARAM }
};

static big_data_event_detail_t big_data_event[] = {
	{ BIG_DATA_EVENT_MOTION_TYPE, 12, event_motion_type_param },
	{ BIG_DATA_EVENT_DDR_INFO, 2, event_ddr_info_param },
	{ BIG_DATA_EVENT_TOF_PHONECALL, 2, event_tof_phonecall_param },
};

static char *big_data_str_map[] = {
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
		hwlog_err("send big data fetch req type = %d fail\n",
			event_id);
		return -1;
	}

	if (pkg_mcu.errno != 0) {
		hwlog_err("big data fetch req to mcu fail errno = %d\n",
			pkg_mcu.errno);
		return -1;
	}

	if (length < MAX_PKT_LENGTH)
		memcpy(data, pkg_mcu.data, length);
	else
		memcpy(data, pkg_mcu.data, MAX_PKT_LENGTH);

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
	int i;
	int ret = 0;
	struct imonitor_eventobj *obj = NULL;
	uint32_t *raw_data = (uint32_t *)data;
	big_data_event_detail_t event_detail;
	big_data_param_detail_t param_detail;

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
		if (event_id == big_data_event[i].event_id) {
			memcpy(&event_detail, &big_data_event[i], sizeof(event_detail));
			break;
		}
	}

	for (i = 0; i < event_detail.param_num; ++i) {
		memcpy(&param_detail, &event_detail.param_data[i], sizeof(param_detail));
		if (param_detail.param_type == INT_PARAM) {
			ret += imonitor_set_param_integer_v2(obj, param_detail.param_name, raw_data[i]);
		} else if (param_detail.param_type == STR_PARAM) {
			if (big_data_str_map[raw_data[i]] != NULL)
				ret += imonitor_set_param_string_v2(obj, param_detail.param_name, big_data_str_map[raw_data[i]]);
		}
	}

	if (ret) {
		imonitor_destroy_eventobj(obj);
		hwlog_err("%s imonitor_set_param fail %d\n", __func__, ret);
		return ret;
	}

	ret = imonitor_send_event(obj);
	hwlog_info("big data imonitor send id:%d succ\n", (uint32_t)event_id);
	if (ret < 0)
		hwlog_err("%s imonitor_send_event failed\n", __func__);

	imonitor_destroy_eventobj(obj);
	return ret;
}

static int iomcu_big_data_process(const struct pkt_header *head)
{
	uint32_t *data = NULL;

	if (!head)
		return -1;

	pkt_big_data_report_t *report_t = (pkt_big_data_report_t *)head;

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
		process_big_data(report_t->event_id, data);
		break;

	default:
		hwlog_info("%s no matched event id:%d\n", __func__,
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

	hwlog_info("%s success\n", __func__);
	return 0;
}

late_initcall_sync(iomcu_big_data_init);

MODULE_AUTHOR("SensorHub <smartphone@huawei.com>");
MODULE_DESCRIPTION("SensorHub big data channel");
MODULE_LICENSE("GPL");
