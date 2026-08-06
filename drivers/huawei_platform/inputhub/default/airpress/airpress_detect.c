/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: airpress detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "airpress_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"

#define RET_FAIL              (-1)

#define AIRPRESS_RECHECK_DELAY 15

static struct airpress_platform_data airpress_data[AIRPRESS_DEV_COUNT_MAX] = {
	{
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 1000, /* interval 1000 ms */
	},
};
static struct airpress_device_info g_airpress_dev_info[AIRPRESS_DEV_COUNT_MAX];

struct airpress_platform_data *airpress_get_platform_data(int32_t tag)
{
	if (tag == TAG_PRESSURE)
		return (&(airpress_data[0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct airpress_device_info *airpress_get_device_info(int32_t tag)
{
	if (tag == TAG_PRESSURE)
		return (&(g_airpress_dev_info[0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

void read_airpress_data_from_dts(struct device_node *dn)
{
	struct airpress_platform_data *pf_data = &(airpress_data[0]);
	int temp = 0;

	read_chip_info(dn, AIRPRESS);

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read poll_interval fail\n", __func__);
	else
		pf_data->poll_interval = (uint16_t)temp;

	if (of_property_read_u32(dn, "isSupportTouch", &temp)) {
		hwlog_err("%s:read isSupportTouch fail\n", __func__);
		pf_data->is_support_touch = 0;
	} else {
		pf_data->is_support_touch = temp;
	}

	if (of_property_read_u32(dn, "airpress_touch_calibrate_order", &temp)) {
		hwlog_err("%s:read airpress_touch_calibrate_order fail\n", __func__);
		pf_data->touch_fac_order = 0;
	} else {
		pf_data->touch_fac_order = (uint16_t)temp;
	}

	if (of_property_read_u32(dn, "touch_fac_wait_time", &temp)) {
		hwlog_err("%s:read touch_fac_wait_time fail\n", __func__);
		pf_data->touch_fac_wait_time = 0;
	} else {
		pf_data->touch_fac_wait_time = (uint16_t)temp;
	}

	if (of_property_read_u16_array(dn, "tp_touch_coordinate_threshold",
		pf_data->tp_touch_coordinate_threshold,
		TP_COORDINATE_THRESHOLD)) {
		hwlog_err("%s:read tp_touch_coordinate_threshold fail\n", __func__);
		for (temp = 0; temp < TP_COORDINATE_THRESHOLD; temp++)
			pf_data->tp_touch_coordinate_threshold[temp] = 0;
	}

	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read airpress reg fail\n", __func__);
	else
		pf_data->cfg.i2c_address = (uint8_t)temp;

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read airpress file_id fail\n", __func__);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read ps sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);

	read_sensorlist_info(dn, AIRPRESS);
}

static int airpress_sensor_check(struct device_node *dn, int index,
	struct sensor_combo_cfg *cfg)
{
	uint8_t register_add;
	int ret;
	char *chip_info = NULL;
	/* 4 is for may be return 4 bytes */
	uint8_t detected_device_id[4] = { 0 };

	ret = _device_detect(dn, index, cfg);
	if (ret)
		return ret;
	if (of_property_read_string(dn, "compatible",
		(const char **)&chip_info))
		return ret;
	if (strncmp(chip_info, "huawei,goer_spl07001",
		sizeof("huawei,goer_spl07001")))
		return ret;

	hwlog_info("%s recheck start\n", __func__);
	register_add = 0x00;
	if (mcu_i2c_rw(TAG_I2C, cfg->bus_num, cfg->i2c_address,
		(uint8_t *)&register_add, 1, detected_device_id, 1) < 0)
		return ret;
	hwlog_info("%s recheck 0x%x\n", __func__, detected_device_id[0]);
	/* not 0x50 not bosch */
	if (detected_device_id[0] != 0x50)
		return ret;
	msleep(AIRPRESS_RECHECK_DELAY);
	register_add = 0x0D;
	if (mcu_i2c_rw(TAG_I2C, cfg->bus_num, cfg->i2c_address,
		(uint8_t *)&register_add, 1, detected_device_id, 1) < 0)
		return ret;
	if (detected_device_id[0] == 0x10) /* 0x10 is goer */
		return ret;

	hwlog_info("%s give up 0x%x\n", __func__, detected_device_id[0]);
	return RET_FAIL;
}

int airpress_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };

	if ((!dn) || (index < 0) || (index >= SENSOR_MAX) || (!sm))
		return RET_FAIL;

	ret = airpress_sensor_check(dn, index, &cfg);
	if (!ret) {
		p = sm + index;
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}
	return ret;
}

void airpress_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= AIRPRESS) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_airpress_dev_info[0].obj_tag = TAG_PRESSURE;
	g_airpress_dev_info[0].detect_list_id = AIRPRESS;
	g_airpress_dev_info[0].airpress_dev_index = 0;
	p = sm + AIRPRESS;
	p->spara = (const void *)&(airpress_data[0]);
	p->cfg_data_length = sizeof(airpress_data[0]);
}

