/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "acc_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "acc_sysfs.h"
#include "contexthub_route.h"

#define ACC_DEVICE_ID_0 0
#define ACC_DEVICE_ID_1 1
#define ACC_SENSOR_DEFAULT_ID "1"

static char *acc_sensor_id = ACC_SENSOR_DEFAULT_ID;

static struct g_sensor_platform_data gsensor_data[ACC_DEV_COUNT_MAX] = {
	{
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 10, /* interval 10 ms */
	.axis_map_x = 1,
	.axis_map_y = 0,
	.axis_map_z = 2, /* index 2 */
	.negate_x = 0,
	.negate_y = 1,
	.negate_z = 0,
	.used_int_pin = 2, /* used int pin 2 */
	.gpio_int1 = 208, /* gpio int1 208 */
	.gpio_int2 = 0,
	.gpio_int2_sh = 0,
	.device_type = 0,
	.calibrate_style = 0,
	.calibrate_way = 0,
	.x_calibrate_thredhold = 250, /* 250 mg */
	.y_calibrate_thredhold = 250, /* 250 mg */
	.z_calibrate_thredhold = 320, /* 320 mg */
	.wakeup_duration = 0x60, /* default set up 3 duration */
	.gpio_int2_sh_func = 2, /* gpio int2 sh func 2 */
	.acc_data_thredhold = 0,
	},
};
static struct acc_device_info g_acc_dev_info[ACC_DEV_COUNT_MAX];
struct g_sensor_platform_data *acc_get_platform_data(int32_t tag)
{
	if (tag == TAG_ACCEL)
		return (&(gsensor_data[ACC_DEVICE_ID_0]));
	else if (tag == TAG_ACC1)
		return (&(gsensor_data[ACC_DEVICE_ID_1]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct acc_device_info *acc_get_device_info(int32_t tag)
{
	if (tag == TAG_ACCEL)
		return (&(g_acc_dev_info[ACC_DEVICE_ID_0]));
	else if (tag == TAG_ACC1)
		return (&(g_acc_dev_info[ACC_DEVICE_ID_1]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

char *acc_get_sensors_id_string(void)
{
	return acc_sensor_id;
}

void acc_get_sensors_id_from_dts(struct device_node *dn)
{
	if (dn == NULL)
		return;
	if (of_property_read_string(dn,
		"acc_sensor_id", (const char **)&acc_sensor_id)) {
		acc_sensor_id = ACC_SENSOR_DEFAULT_ID;
		hwlog_err("acc_sensor_id err.\n");
	}
	hwlog_info("acc_sensor_id:%s\n", acc_sensor_id);
}

static void read_acc_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct g_sensor_platform_data *pf_data =
		(struct g_sensor_platform_data *)sm->spara;
	int temp = 0;

	if (of_property_read_u32(dn, "used_int_pin", &temp))
		hwlog_err("%s:read %s used_int_pin fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->used_int_pin = (uint8_t)temp;

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int1 fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->gpio_int1 = (GPIO_NUM_TYPE)temp;

	temp = of_get_named_gpio(dn, "gpio_int2", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_int2 fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->gpio_int2 = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_int2_sh", &temp))
		hwlog_err("%s:read %s gpio_int2_sh fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->gpio_int2_sh = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_int2_sh_func", &temp))
		hwlog_err("%s:read %s gpio_int2_sh_func fail\n", __func__, sm->sensor_name_str);
	else
		pf_data->gpio_int2_sh_func = (uint8_t) temp;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s poll_interval fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->poll_interval = (uint16_t)temp;

	if (of_property_read_u32(dn, "calibrate_style", &temp))
		hwlog_err("%s:read %s calibrate_style fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->calibrate_style = (uint8_t)temp;

	if (of_property_read_u32(dn, "axis_map_x", &temp))
		hwlog_err("%s:read %s axis_map_x fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->axis_map_x = (uint8_t) temp;
}

static void read_acc_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct g_sensor_platform_data *pf_data =
		(struct g_sensor_platform_data *)sm->spara;
	int temp = 0;

	if (of_property_read_u32(dn, "axis_map_y", &temp))
		hwlog_err("%s:read %s axis_map_y fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->axis_map_y = (uint8_t)temp;

	if (of_property_read_u32(dn, "axis_map_z", &temp))
		hwlog_err("%s:read %s axis_map_z fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->axis_map_z = (uint8_t)temp;

	if (of_property_read_u32(dn, "negate_x", &temp))
		hwlog_err("%s:read %s negate_x fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->negate_x = (uint8_t)temp;

	if (of_property_read_u32(dn, "negate_y", &temp))
		hwlog_err("%s:read %s negate_y fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->negate_y = (uint8_t)temp;

	if (of_property_read_u32(dn, "negate_z", &temp))
		hwlog_err("%s:read %s negate_z fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->negate_z = (uint8_t)temp;

	if (of_property_read_u32(dn, "device_type", &temp))
		hwlog_err("%s:read %s device_type fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->device_type = (uint8_t)temp;

	if (of_property_read_u32(dn, "x_calibrate_thredhold", &temp))
		hwlog_err("%s:read %s x_calibrate_thredhold fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->x_calibrate_thredhold = (uint16_t)temp;

	if (of_property_read_u32(dn, "y_calibrate_thredhold", &temp))
		hwlog_err("%s:read %s y_calibrate_thredhold fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->y_calibrate_thredhold = (uint16_t)temp;
}

static void read_acc_config3_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct g_sensor_platform_data *pf_data =
		(struct g_sensor_platform_data *)sm->spara;
	int temp = 0;
	int aux = 0;

	if (of_property_read_u32(dn, "z_calibrate_thredhold", &temp))
		hwlog_err("%s:read %s z_calibrate_thredhold fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->z_calibrate_thredhold = (uint16_t)temp;

	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n",
			__func__, sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n",
			__func__, sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n",
			__func__, sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s sensor_list_info_id fail\n",
			__func__, sm->sensor_name_str);
	else
		add_sensor_list_info_id((uint16_t)temp);

	if (of_property_read_u32(dn, "calibrate_way", &temp))
		hwlog_err("%s:read %s calibrate_way fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->calibrate_way = (uint8_t)temp;

	if (of_property_read_u32(dn, "wakeup_duration", &temp))
		hwlog_err("%s:read %s wakeup_duration fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->wakeup_duration = (uint8_t) temp;

	if (of_property_read_string(dn, "sensor_in_board_status",
		(const char **)&sensor_in_board_status))
		hwlog_err("read sensor_in_board_status order err\n");

	hwlog_info("sensor_in_board_status order:%s\n", sensor_in_board_status);
}

static void read_acc_config4_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct g_sensor_platform_data *pf_data =
		(struct g_sensor_platform_data *)sm->spara;
	int temp = 0;
	const char *bus_type = NULL;

	if (of_property_read_string(dn, "bus_type", &bus_type)) {
		hwlog_warn("%s:acc %s bus_type not configured\n",
			__func__, sm->sensor_name_str);
	} else {
		if (get_combo_bus_tag(bus_type, (uint8_t *)&temp))
			hwlog_warn("%s:acc %s bus_type invalid\n",
				__func__, sm->sensor_name_str);
		else
			pf_data->cfg.bus_type = temp;
	}

	if (of_property_read_u32(dn, "bus_number", &temp))
		hwlog_warn("%s:acc %s bus_number not configured\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->cfg.bus_num = temp;

	if (pf_data->cfg.bus_type == TAG_I2C ||
		pf_data->cfg.bus_type == TAG_I3C) {
		if (of_property_read_u32(dn, "i2c_address", &temp))
			hwlog_warn("%s:acc %s i2c_address not configured\n",
				__func__, sm->sensor_name_str);
		else
			pf_data->cfg.i2c_address = temp;
	}

	if (of_property_read_u32(dn, "acc_data_thredhold", &temp))
		hwlog_err("%s:read %s acc_data_thredhold fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->acc_data_thredhold = (uint8_t)temp;
}

void read_acc_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	read_chip_info(dn, sm->sensor_id);
	read_acc_config1_from_dts(dn, sm);
	read_acc_config2_from_dts(dn, sm);
	read_acc_config3_from_dts(dn, sm);
	read_acc_config4_from_dts(dn, sm);
	read_sensorlist_info(dn, sm->sensor_id);
}

void acc_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;
	int32_t i;

	if (len <= ACC1) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_acc_dev_info[ACC_DEVICE_ID_0].obj_tag = TAG_ACCEL;
	g_acc_dev_info[ACC_DEVICE_ID_1].obj_tag = TAG_ACC1;
	g_acc_dev_info[ACC_DEVICE_ID_0].detect_list_id = ACC;
	g_acc_dev_info[ACC_DEVICE_ID_1].detect_list_id = ACC1;
	for (i = 0; i < ACC_DEV_COUNT_MAX; i++) {
		g_acc_dev_info[i].acc_dev_index = i;
		p = sm + g_acc_dev_info[i].detect_list_id;
		p->spara = (const void *)&(gsensor_data[i]);
		p->cfg_data_length = sizeof(gsensor_data[i]);
		if (i == 0)
			continue;
		if (memcpy_s(&(gsensor_data[i]), sizeof(gsensor_data[i]),
			&(gsensor_data[0]), sizeof(gsensor_data[0])) != EOK)
			hwlog_info("%s: memcpy_s error i=%d\n", __func__, i);
	}
}

