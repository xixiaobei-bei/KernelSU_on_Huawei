/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: handpress detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "handpress_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/hisi/hw_cmdline_parse.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"

#define HANDPRESS_DEFAULT_STATE "huawei,default-state"

static struct handpress_platform_data handpress_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.bootloader_type = 0,
	.poll_interval = 500, /* interval 500 ms */
};

void read_handpress_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;

	read_chip_info(dn, HANDPRESS);

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read poll_interval fail\n", __func__);
	else
		handpress_data.poll_interval = (uint16_t)temp;

	hwlog_info("get handpress dev from dts.sensor name=%d\n",
		handpress_data.poll_interval);

	if (of_property_read_u32(dn, "bootloader_type", &temp))
		hwlog_err("%s:read handpress file_id_fw fail\n", __func__);
	else
		handpress_data.bootloader_type = (uint8_t)temp;

	hwlog_info("bootloader_type:%d\n", handpress_data.bootloader_type);
	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read handpress file_id fail\n", __func__);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read ps sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);

	hwlog_info("get handpress dev from temp=%d\n", temp);
}

static int handpress_of_read(struct handpress_platform_data *handpress_data,
	int chips, struct device_node *temp)
{
	int ret;
	u32 id = 0;
	u32 points = 0;
	u32 reg = 0;

	if (chips >= CYPRESS_CHIPS) {
		hwlog_err("The number of chips overflow\n");
		return -1;
	}
	ret = of_property_read_u32(temp, "huawei,id", &id);
	if (ret) {
		hwlog_err("huawei,id ret:%d\n", ret);
		return -1;
	}
	handpress_data->id[chips] = id;
	ret = of_property_read_u32(temp, "huawei,points", &points);
	if (ret) {
		hwlog_err("huawei,points ret:%d\n", ret);
		return -1;
	}
	handpress_data->t_pionts[chips] = points;
	ret = of_property_read_u32(temp, "reg", &reg);
	if (ret) {
		hwlog_err("reg ret:%d\n", ret);
		return -1;
	}
	handpress_data->i2c_address[chips] = reg;
	return 0;
}

int handpress_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	unsigned int chip_check_result = 0;
	struct device_node *temp = NULL;
	int num_handpress = 0;
	int chips = 0;
	u32 bus_num = 0;
	char chip_value[12] = { 0xff, 0xff, 0x00 };

	ret = of_property_read_u32(dn, "bus_number", &bus_num);
	if (ret) {
		hwlog_err("get bus_num err ret:%d\n", ret);
		return ret;
	}
	while ((temp = of_get_next_child(dn, temp)))
		num_handpress++;

	if (!num_handpress)
		return -ECHILD;

	for_each_child_of_node(dn, temp) {
		const char *state = NULL;

		ret = of_property_read_string(temp,
			HANDPRESS_DEFAULT_STATE, &state);
		if (!ret) {
			if (strncmp(state, "on", sizeof("on")) == 0) {
				if (handpress_of_read(&handpress_data, chips, temp) != 0)
					break;

				ret = mcu_i2c_rw(TAG_I2C, (uint8_t)bus_num,
					handpress_data.i2c_address[chips],
					NULL, 0, (uint8_t *)chip_value,
					(uint32_t)(sizeof(chip_value) /
					sizeof(chip_value[0])));
				if (ret < 0) {
					hwlog_err("mcu_i2c_i3c_rw read ret:%d\n",
						ret);
					chip_check_result |= 1 << (handpress_data.id[chips]);
					continue;
				}
				hwlog_err("chip_value: %2x %2x %2x %2x\n",
					chip_value[0], chip_value[1],
					chip_value[2], chip_value[3]);
				chips++;
			}
		}
	}

	if (!chip_check_result)
		ret = 0;
	else
		ret = -1;

	hwlog_info("handpress detect result:%d\n", ret);
	return ret;
}

void handpress_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= HANDPRESS) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}

	p = sm + HANDPRESS;
	p->spara = (const void *)&handpress_data;
	p->cfg_data_length = sizeof(handpress_data);
}

