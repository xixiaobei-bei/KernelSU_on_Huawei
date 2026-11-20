/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mag detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "mag_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"

#define MAG_DEVICE_ID_0 0
#define CURRENT_FAC_AKM_INIT  0
#define SOFTIRON_ARGS_NUM 9
#define MAG_SENSOR_DEFAULT_ID "2"


static struct mag_device_info g_mag_dev_info[MAG_DEV_COUNT_MAX];

struct mag_device_info *mag_get_device_info(int32_t tag)
{
	if (tag == TAG_MAG)
		return (&(g_mag_dev_info[MAG_DEVICE_ID_0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

void read_magn_charger_current_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	int currrent_x[CHARGER_TYPE_NUM] = {0};
	int currrent_y[CHARGER_TYPE_NUM] = {0};
	int currrent_z[CHARGER_TYPE_NUM] = {0};
	int i = 0;
	struct mag_device_info *dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return;

	if (of_property_read_u32(dn, "akm_cal_algo", &temp)) {
		hwlog_info("%s:read mag akm_cal_algo fail\n",
			__func__);
		dev_info->akm_cal_algo = 0;
	} else {
		dev_info->akm_cal_algo = ((temp == 1) ? 1 : 0);
		hwlog_info("%s: mag akm_cal_algo=%d\n",
			__func__, dev_info->akm_cal_algo);
	}

	if (of_property_read_u32(dn, "akm_need_charger_current", &temp)) {
		hwlog_info("%s:read mag akm_need_charger_current fail\n",
			__func__);
		dev_info->akm_need_charger_current = 0;
	} else {
		dev_info->akm_need_charger_current = ((temp == 1) ? 1 : 0);
		hwlog_info("%s: mag akm_need_charger_current=%d\n",
			__func__, dev_info->akm_need_charger_current);
	}

	if (of_property_read_u32_array(dn, "akm_current_x_fac", (unsigned int *)currrent_x, CHARGER_TYPE_NUM)) {
		hwlog_info("%s:read mag akm_current_x_fac fail\n", __func__);
		dev_info->akm_current_x_fac[0] = CURRENT_FAC_AKM_INIT;
	}
	for (i = 0; i < CHARGER_TYPE_NUM; i++)
		dev_info->akm_current_x_fac[i] = currrent_x[i];

	if (of_property_read_u32_array(dn, "akm_current_y_fac", (unsigned int *)currrent_y, CHARGER_TYPE_NUM)) {
		hwlog_info("%s:read mag akm_current_y_fac fail\n", __func__);
		dev_info->akm_current_y_fac[0] = CURRENT_FAC_AKM_INIT;
	}
	for (i = 0; i < CHARGER_TYPE_NUM; i++)
		dev_info->akm_current_y_fac[i] = currrent_y[i];

	if (of_property_read_u32_array(dn, "akm_current_z_fac", (unsigned int *)currrent_z, CHARGER_TYPE_NUM)) {
		hwlog_info("%s:read mag akm_current_z_fac fail\n", __func__);
		dev_info->akm_current_z_fac[0] = CURRENT_FAC_AKM_INIT;
	}
	for (i = 0; i < CHARGER_TYPE_NUM; i++)
		dev_info->akm_current_z_fac[i] = currrent_z[i];
}

void mag_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct mag_device_info *dev_info = mag_get_device_info(TAG_MAG);
	if (dev_info == NULL)
		return;

	dev_info->obj_tag = TAG_MAG;
	dev_info->detect_list_id = MAG;
	hwlog_info("%s: mag mag_detect_init succ!\n", __func__);
}