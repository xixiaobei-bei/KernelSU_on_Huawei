/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: motion detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "motion_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_boot.h"
#include "contexthub_pm.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"

#define RET_FAIL             (-1)
#define RET_SUCC             0
#define HORIZONTAL_PICKUP_PARA_LEN 16

struct motion_platform_data motion_data = {
	.motion_horizontal_pickup_flag = 0,
	.angle_gap = 45, /* default angle 45 */
};

void motion_get_fall_down_support_from_dts(void)
{
	struct device_node *sensorhub_node = NULL;
	uint32_t temp = 0;

	sensorhub_node = of_find_compatible_node(NULL, NULL,
		"huawei,sensorhub");
	if (!sensorhub_node) {
		hwlog_err("%s, can't find node sensorhub\n", __func__);
		return;
	}

	if (of_property_read_u32(sensorhub_node, "fall_down_support", &temp)) {
		hwlog_err("%s, can't find node fall_down_support\n", __func__);
		return;
	}

	hwlog_info("%s, fall_down_support=%u\n", __func__, temp);
	g_config_on_ddr->fall_down_support = temp;
}

void read_motion_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	u32 tmp = 0;

	if (!dn) {
		hwlog_err("%s! motion node is NULL\n", __func__);
		return;
	}

	read_chip_info(dn, MOTION);

	if (of_property_read_u32(dn, "motion_horizontal_pickup_flag", &tmp)) {
		hwlog_err("%s:read horizontal_pickup_flag from dts fail\n", __func__);
	} else {
		motion_data.motion_horizontal_pickup_flag = (uint8_t)tmp;
		hwlog_info("%s:read horizontal_pickup_flag from dts succ\n", __func__);
	}

	hwlog_info("%s: pickup_flag %d\n", __func__,
		(int)motion_data.motion_horizontal_pickup_flag);

	if (of_property_read_u32(dn, "angle_gap", &tmp)) {
		hwlog_err("%s:read angle_gap from dts fail\n", __func__);
	} else {
		motion_data.angle_gap = (uint8_t)tmp;
		hwlog_info("%s:read angle_gap from dts succ\n", __func__);
	}

	hwlog_info("%s: angle_gap %d\n", __func__, (int)motion_data.angle_gap);
}

int motion_set_cfg_data(void)
{
	int ret;
	uint8_t app_config[HORIZONTAL_PICKUP_PARA_LEN] = { 0 };

	hwlog_info("write motion cmd\n");
	app_config[0] = MOTION_TYPE_PICKUP;
	app_config[1] = SUB_CMD_MOTION_HORIZONTAL_PICKUP_REQ;
	if (motion_data.motion_horizontal_pickup_flag) {
		app_config[2] = motion_data.motion_horizontal_pickup_flag;
		app_config[3] = motion_data.angle_gap;

		if ((get_iom3_state() == IOM3_ST_RECOVERY) ||
			(get_iomcu_power_state() == ST_SLEEP))
			ret = send_app_config_cmd(TAG_MOTION, app_config, false);
		else
			ret = send_app_config_cmd(TAG_MOTION, app_config, true);

		if (ret) {
			hwlog_err("send motion %d cfg data to mcu fail,ret=%d\n",
				(int)app_config[0], ret);
			return RET_FAIL;
		}
		hwlog_info("write motion success. pickup_flag:%d,angle_gap:%d\n",
			(int)app_config[2], (int)app_config[3]);
	}
	return RET_SUCC;
}

void motion_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= MOTION) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}

	p = sm + MOTION;
	p->spara = (const void *)&motion_data;
	p->cfg_data_length = sizeof(motion_data);
}

