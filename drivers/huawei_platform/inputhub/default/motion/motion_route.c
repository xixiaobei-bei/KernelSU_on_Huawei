/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: motion route source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "motion_route.h"

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <huawei_platform/inputhub/motionhub.h>
#include <log/hiview_hievent.h>
#include <securec.h>

#include "contexthub_route.h"

#define SENSOR_FALL_IMONITOR_ID 936005010

static void motion_fall_down_report(const struct motion_fall_down_data *info)
{
	int ret;
	unsigned int ret_u = 0;
	struct hiview_hievent *hi_event = NULL;

	if (!info) {
		hwlog_err("%s info NULL\n", __func__);
		return;
	}

	hi_event = hiview_hievent_create(SENSOR_FALL_IMONITOR_ID);
	if (!hi_event) {
		hwlog_err("%s create hievent fail\n", __func__);
		return;
	}

	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"fall_status", info->fall_status);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"impact_force", info->impact_force);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"fall_height", info->fall_height);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"still_time", info->still_time);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"confidence", info->confidence);
	ret_u |= (unsigned int)hiview_hievent_put_integral(hi_event,
		"suspected_cnt", info->suspected_cnt);
	if (ret_u != 0)
		hwlog_err("%s put integral fail ret_put=%d\n", __func__,
			(int)ret_u);

	ret = (unsigned int)hiview_hievent_report(hi_event);
	if (ret < 0)
		hwlog_err("%s report fail ret=%d\n", __func__, ret);

	hiview_hievent_destroy(hi_event);

	hwlog_info("%s fall_status=%d impact_force=%d fall_height=%d still_time=%d confidence=%d suspected_cnt=%d\n",
		__func__, info->fall_status, info->impact_force,
		info->fall_height, info->still_time,
		info->confidence, info->suspected_cnt);
}

int inputhub_process_motion_report(const struct pkt_header *head)
{
	char *motion_data = (char *)head + sizeof(pkt_common_data_t);

	if ((((int)motion_data[0]) == MOTIONHUB_TYPE_TAKE_OFF) ||
		(((int)motion_data[0]) == MOTIONHUB_TYPE_PICKUP) ||
		(((int)motion_data[0]) == MOTION_TYPE_REMOVE)) {
		inputhub_route_pm_wakeup_event(jiffies_to_msecs(HZ));
		hwlog_err("%s weaklock HZ motiontype=%d\n", __func__,
			motion_data[0]);
	}

	hwlog_info("%s : motiontype = %d motion_result = %d motion_status = %d\n",
		/* 2 is motion_status */
		__func__, motion_data[0], motion_data[1], motion_data[2]);

	return inputhub_route_write(ROUTE_MOTION_PORT, motion_data,
		head->length -
		(sizeof(pkt_common_data_t) - sizeof(struct pkt_header)));
}

bool is_motion_data_report(const struct pkt_header *head)
{
	/* all sensors report data with command CMD_PRIVATE */
	return (head->tag == TAG_MOTION) && (head->cmd == CMD_DATA_REQ);
}

void process_step_counter_report(const struct pkt_header *head)
{
	uint16_t cnt;

	inputhub_route_pm_wakeup_event(jiffies_to_msecs(HZ));

	cnt = ((struct pkt_step_counter_motion_req *)head)->data_hd.cnt;
	if (cnt == 1) {
		struct pkt_motion_data *mt = NULL;

		hwlog_info("Kernel get pedometer motion\n");
		mt = &(((struct pkt_step_counter_motion_req *)head)->mt);
		if (mt->motion_type == MOTION_TYPE_FALL_DOWN)
			motion_fall_down_report(
				(const struct motion_fall_down_data *)mt->data);
		return;
	}

	if (cnt == 0)
		hwlog_info("Kernel get pedometer event\n");
	else
		hwlog_info("Kernel get pedometer event cnt=%u\n", cnt);

	step_counter_data_process((pkt_step_counter_data_req_t *)head);
	report_sensor_event(head->tag,
		(int *)(&((pkt_step_counter_data_req_t *) head)->step_count),
		head->length);
}

