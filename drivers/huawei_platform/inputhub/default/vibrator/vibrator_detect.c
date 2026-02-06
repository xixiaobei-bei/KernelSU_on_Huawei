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
#include "vibrator_channel.h"

static struct vibrator_paltform_data vibrator_data = {
	.cfg = {
		.bus_type = TAG_I2C,
		.bus_num = 0,
		.disable_sample_thread = 0,
		{ .i2c_address = 0x36 },
	},
	.max_timeout_ms = 10000, /* max timeout ms 10000 */
	.reduce_timeout_ms = 0,
	.skip_lra_autocal = 1,
	.cold_level_count = 0,
	.cold_level = { 0 },
	.battery_level = { 0 },
	.chip_type = VIBRATOR_CHIP_UNKNOWN,
	.gpio_irq = 180, /* gpio irq 180 */
	.gpio_reset = 202, /* gpio reset 202 */
	.rst_value = 0,
	.reg_addr_bytes = 1,
	.reg_value_bytes = 2, /* reg value bytes 2 */
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
		hwlog_err("%s:read read_vibrator_from_dts file_id fail\n",
			__func__);
	else
		read_dyn_file_list((uint16_t)temp);

	hwlog_err("read_vibrator_from_dts file id is %d\n", temp);

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
		vibrator_data.lra_overdriver_voltage =
			HUB_LRA_OVERDRIVE_CLAMP_VOLTAGE;
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

	rc = of_property_read_u32(dn, "cold_level_count", &temp);
	if (rc < 0) {
		hwlog_err("vibrator cold_level_count fail\n");
		vibrator_data.cold_level_count = 0;
	} else {
		hwlog_info("vibrator cold_level_count:%d\n", temp);
		vibrator_data.cold_level_count = (temp > SENSOR_COLD_LEVEL_COUNT) ?
			SENSOR_COLD_LEVEL_COUNT : temp;
	}
}

static void read_vibrator_config3_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;
	int rc;
	const char *ptr_cold = NULL;
	int i;

	for (i = 0; i < vibrator_data.cold_level_count; i++) {
		rc = of_property_read_string_index(dn, "cold_level", i, &ptr_cold);
		if (rc < 0) {
			hwlog_err("vibrator cold_level:%d error\n", i);
		} else {
			rc = kstrtoint(ptr_cold, TO_DECIMALISM, &temp);
			if (rc < 0)
				hwlog_err("cold_level kstrtoint:%d fail\n", i);
			else
				vibrator_data.cold_level[i] = temp;
		}
		rc = of_property_read_string_index(dn, "battery_level", i, &ptr_cold);
		if (rc < 0) {
			hwlog_err("vibrator battery_level:%d error\n", i);
		} else {
			rc = kstrtoint(ptr_cold, TO_DECIMALISM, &temp);
			if (rc < 0)
				hwlog_err("battery_level kstrtoint:%d fail\n", i);
			else
				vibrator_data.battery_level[i] = temp;
		}
		hwlog_info("vibrator %d cold_level:%d, battery_level:%d\n",
			i, vibrator_data.cold_level[i], vibrator_data.battery_level[i]);
	}

	if (of_property_read_u32(dn, "bus_number", &temp))
		hwlog_err("%s:read bus_number fail\n", __func__);
	else
		vibrator_data.cfg.bus_num = (uint8_t)temp;

	if (of_property_read_u32(dn, "i2c_address", &temp))
		hwlog_err("%s:read i2c_address fail\n", __func__);
	else
		vibrator_data.cfg.i2c_address = (uint32_t)temp;

	if (of_property_read_u32(dn, "reg", &temp))
		hwlog_err("%s:read reg fail\n", __func__);
	else
		vibrator_data.chip_type = (uint32_t)temp;
}

static void read_vibrator_config4_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;

	if (of_property_read_u32(dn, "gpio_irq", &temp))
		hwlog_err("%s:read gpio_irq fail\n", __func__);
	else
		vibrator_data.gpio_irq = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "gpio_reset", &temp))
		hwlog_err("%s:read gpio_reset fail\n", __func__);
	else
		vibrator_data.gpio_reset = (GPIO_NUM_TYPE)temp;

	if (of_property_read_u32(dn, "rst_value", &temp))
		hwlog_err("%s:read rst_value fail\n", __func__);
	else
		vibrator_data.rst_value = (uint8_t)((temp != 0) ? 1 : 0);

	if (of_property_read_u32(dn, "reg_addr_bytes", &temp))
		hwlog_err("%s:read reg_addr_bytes fail\n", __func__);
	else
		vibrator_data.reg_addr_bytes = (uint8_t)temp;

	if (of_property_read_u32(dn, "reg_value_bytes", &temp))
		hwlog_err("%s:read reg_value_bytes fail\n", __func__);
	else
		vibrator_data.reg_value_bytes = (uint8_t)temp;

	if (of_property_read_u32(dn, "ps_disable", &temp))
		hwlog_err("%s:read ps_disable fail\n", __func__);
	else
		vibrator_data.ps_disable = (uint8_t)temp;

	if (of_property_read_u32(dn, "dma_irq_gpio", &temp))
		hwlog_err("%s:read dma_irq_gpio fail\n", __func__);
	else
		vibrator_data.dma_irq_gpio = (uint16_t)temp;

	if (of_property_read_u32(dn, "dma_irq_gpio_addr", &temp))
		hwlog_err("%s:read dma_irq_gpio_addr fail\n", __func__);
	else
		vibrator_data.dma_irq_gpio_addr = (uint32_t)temp;

	if (of_property_read_u32(dn, "dma_channel", &temp))
		hwlog_err("%s:read dma_channel fail\n", __func__);
	else
		vibrator_data.dma_channel = (uint8_t)temp;
}

void read_vibrator_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	read_chip_info(dn, VIBRATOR);
	read_vibrator_config1_from_dts(dn, sm);
	read_vibrator_config2_from_dts(dn, sm);
	read_vibrator_config3_from_dts(dn, sm);
	read_vibrator_config4_from_dts(dn, sm);
}

static bool vibrator_sensor_status_is_ok(struct device_node *dn)
{
	int ret;
	char *status = NULL;
	const char *vib_status_ok = "ok";

	if (!dn)
		return false;

	ret = of_property_read_string(dn, "status", (const char **)&status);
	if (!ret) {
		/* compare the string */
		if (strncmp(status, vib_status_ok, strlen(vib_status_ok))) {
			hwlog_err("%s: status disable\n", __func__);
			return false;
		}
	} else {
		hwlog_err("%s:read status fail\n", __func__);
		return false;
	}

	return true;
}

int vibrator_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	char *sensor_vendor = NULL;
	int temp = 0;
	GPIO_NUM_TYPE gpio_reset = 0;
	int gpio_ori_val;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };

	if (!vibrator_sensor_status_is_ok(dn) || !sm)
		return -1;

	if (of_property_read_u32(dn, "gpio_reset", &temp)) {
		hwlog_err("%s:read gpio_reset fail\n", __func__);
	} else {
		gpio_reset = (GPIO_NUM_TYPE)temp;
		hwlog_info("%s gpio_reset = %d\n", __func__, gpio_reset);
	}
	ret = gpio_request(gpio_reset, "vibrator_rst_gpio");
	if (ret < 0)
		hwlog_err("%s, fail request enable pin = %d\n", __func__, ret);
	gpio_ori_val = gpio_get_value(gpio_reset);
	hwlog_info("%s gpio_ori_val: %d\n", __func__, gpio_ori_val);
	ret = of_property_read_string(dn, "compatible", (const char **)&sensor_vendor);
	if (!ret) {
		uint32_t rst_value = 0;

		if (of_property_read_u32(dn, "rst_value", &temp)) {
			hwlog_err("%s:read rst_value fail\n", __func__);
		} else {
			rst_value = (uint32_t)temp;
			hwlog_info("%s rst_value = %u\n", __func__, rst_value);
		}
		gpio_direction_output(gpio_reset, (~rst_value) & 0x1);
		msleep(VIB_RESET_GPIO_DELAY);
		gpio_set_value(gpio_reset, rst_value);
		msleep(VIB_RESET_GPIO_DELAY);
	}
	ret = _device_detect(dn, index, &cfg);
	if (!ret) {
		p = sm + index;
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			return -1;
	} else {
		hwlog_info("%s gpio_free gpio_ori_val: %d\n", __func__, gpio_ori_val);
		gpio_direction_output(gpio_reset, gpio_ori_val);
		gpio_free(gpio_reset);
	}
	return ret;
}

void resend_vibrator_parameters_to_mcu(void)
{
	send_parameter_to_mcu(VIBRATOR, SUB_CMD_SET_PARAMET_REQ);
}

int vibrator_data_from_mcu(const struct pkt_header *head)
{
	switch (((struct pkt_vibrator_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_CALIBRATE_DATA_REQ:
		return write_vibrator_calib_value_to_nv(
			((struct pkt_vibrator_calibrate_data_req_t *)head)->calibrate_data,
			MAX_VIB_CALIBRATE_DATA_LENGTH);
	default:
		hwlog_err("uncorrect subcmd 0x%x\n",
			((struct pkt_vibrator_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
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

