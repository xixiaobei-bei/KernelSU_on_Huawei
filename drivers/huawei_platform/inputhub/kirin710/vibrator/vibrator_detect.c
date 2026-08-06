/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: vibrator detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "vibrator_detect.h"

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

#define RET_FAIL             (-1)

struct vibrator_paltform_data vibrator_data = {
	.cfg = {
		.bus_type = TAG_I2C,
		.bus_num = 0,
		.disable_sample_thread = 0,
		{ .i2c_address = 0x5A },
	},
	.max_timeout_ms = 10000, /* max timeout ms 10000 */
	.reduce_timeout_ms = 0,
	.skip_lra_autocal = 1,
};

struct vibrator_paltform_data *get_vibrator_data(void)
{
	return &vibrator_data;
}

static void read_vibrator_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;
	int rc;

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read file_id fail\n", __func__);
	else
		read_dyn_file_list((uint16_t)temp);

	hwlog_err("%s: file id is %d\n", __func__, temp);

	rc = of_property_read_u32(dn, "ti,max-timeout-ms", &temp);
	/* configure minimum idle timeout */
	if (rc < 0)
		vibrator_data.max_timeout_ms = HUB_MAX_TIMEOUT;
	else
		vibrator_data.max_timeout_ms = (int)temp;

	hwlog_err("vibrator max_timeout_ms:%d\n", vibrator_data.max_timeout_ms);

	rc = of_property_read_u32(dn, "ti,reduce-timeout-ms", &temp);
	/* configure reduce timeout */
	if (rc < 0)
		vibrator_data.reduce_timeout_ms = 0;
	else
		vibrator_data.reduce_timeout_ms = (int)temp;

	hwlog_err("vibrator reduce timedout_ms:%d\n",
		vibrator_data.reduce_timeout_ms);

	rc = of_property_read_u32(dn, "lra_rated_voltage", &temp);
	if (rc < 0)
		vibrator_data.lra_rated_voltage = HUB_LRA_RATED_VOLTAGE;
	else
		vibrator_data.lra_rated_voltage = (char)temp;

	hwlog_err("vibrator lra_rated_voltage:0x%x\n",
		vibrator_data.lra_rated_voltage);
}

static void read_vibrator_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;
	int rc;

	rc = of_property_read_u32(dn, "lra_overdriver_voltage", &temp);
	if (rc < 0)
		vibrator_data.lra_overdriver_voltage = HUB_LRA_OVERDRIVE_CLAMP_VOLTAGE;
	else
		vibrator_data.lra_overdriver_voltage = (char)temp;

	hwlog_err("vibrator lra_overdriver_voltage:0x%x\n",
		vibrator_data.lra_overdriver_voltage);
	if (runmode_is_factory())
		vibrator_data.lra_overdriver_voltage = VIB_FAC_LRALVILTAGE;

	rc = of_property_read_u32(dn, "lra_rtp_strength", &temp);
	if (rc < 0)
		vibrator_data.lra_rtp_strength = HUB_REAL_TIME_PLAYBACK_STRENGTH;
	else
		vibrator_data.lra_rtp_strength = (char)temp;

	hwlog_err("vibrator lra_rtp_strength:0x%x\n",
		vibrator_data.lra_rtp_strength);

	rc = of_property_read_u32(dn, "support_amplitude_control", &temp);
	if (rc < 0)
		vibrator_data.support_amplitude_control = 0;
	else
		vibrator_data.support_amplitude_control = (char)temp;

	hwlog_err("vibrator support_amplitude_control:%d\n",
		vibrator_data.support_amplitude_control);

	vibrator_data.gpio_enable = of_get_named_gpio(dn, "gpio-enable", 0);
	hwlog_err("vibrator gpio_enable:%d\n", vibrator_data.gpio_enable);
	gpio_direction_output(vibrator_data.gpio_enable, 1);
	udelay(30); /* 30 us */
}

void read_vibrator_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	read_chip_info(dn, VIBRATOR);
	read_vibrator_config1_from_dts(dn, sm);
	read_vibrator_config2_from_dts(dn, sm);
}

void vibrator_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= VIBRATOR) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}

	p = sm + VIBRATOR;
	p->spara = (const void *)&vibrator_data;
	p->cfg_data_length = sizeof(vibrator_data);
}
