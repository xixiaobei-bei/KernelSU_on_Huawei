/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_boot.h"
#include "contexthub_debug.h"
#include "contexthub_route.h"

#define ALS_DEVICE_ID_0 0

struct als_platform_data als_data = {
	.cfg = DEF_SENSOR_COM_SETTING,
	.threshold_value = 1,
	.ga1 = 4166, /* 4166: parameter for light source 1 */
	.ga2 = 3000, /* 3000: parameter for light source 2 */
	.ga3 = 3750, /* 3750: parameter for light source 3 */
	.coe_b = 0,
	.coe_c = 0,
	.coe_d = 0,
	.gpio_int1 = 206, /* 206: gpio int pin */
	.atime = 0xdb,
	.again = 1,
	.poll_interval = 1000, /* interval 1000 ms */
	.init_time = 150, /* init need 150 ms */
	.als_phone_type = 0,
	.als_phone_version = 0,
	.als_gain_dynamic = 0,
	.is_bllevel_supported = 0,
	.als_always_open = 0,
};
static struct als_device_info g_als_dev_info[ALS_DEV_COUNT_MAX];

struct als_platform_data *als_get_platform_data(int32_t tag)
{
	if (tag == TAG_ALS)
		return (&als_data);

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct als_device_info *als_get_device_info(int32_t tag)
{
	if (tag == TAG_ALS)
		return (&(g_als_dev_info[ALS_DEVICE_ID_0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

static int als_rohm_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,rohm_bh1745",
		sizeof("huawei,rohm_bh1745")))
		dev_info->chip_type = ALS_CHIP_ROHM_RGB;
	else if (!strncmp(chip_info, "huawei,rohm_rpr531_als",
		sizeof("huawei,rohm_rpr531_als")))
		dev_info->chip_type = ALS_CHIP_RPR531;
	else if (!strncmp(chip_info, "huawei,B",
		sizeof("huawei,B")))
		dev_info->chip_type = ALS_CHIP_BH1726;
	else
		return -1;

	return 0;
}

static int als_avago_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,avago_apds9251",
		sizeof("huawei,avago_apds9251")))
		dev_info->chip_type = ALS_CHIP_AVAGO_RGB;
	else if (!strncmp(chip_info, "huawei,apds9922_als",
		sizeof("huawei,apds9922_als")))
		dev_info->chip_type = ALS_CHIP_APDS9922;
	else if (!strncmp(chip_info, "huawei,avago_apds9999",
		sizeof("huawei,avago_apds9999")))
		dev_info->chip_type = ALS_CHIP_APDS9999_RGB;
	else if (!strncmp(chip_info, "huawei,avago_apds9253",
		sizeof("huawei,avago_apds9253")))
		dev_info->chip_type = ALS_CHIP_APDS9253_RGB;
	else if (!strncmp(chip_info, "huawei,apds9308_als",
		sizeof("huawei,apds9308_als")))
		dev_info->chip_type = ALS_CHIP_APDS9308;
	else
		return -1;

	return 0;
}

static int als_ams_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,ams_tmd3725",
		sizeof("huawei,ams_tmd3725")))
		dev_info->chip_type = ALS_CHIP_AMS_TMD3725_RGB;
	else if (!strncmp(chip_info, "huawei,tmd2745_als",
		sizeof("huawei,tmd2745_als")))
		dev_info->chip_type = ALS_CHIP_TMD2745;
	else if (!strncmp(chip_info, "huawei,ams_tmd3702",
		sizeof("huawei,ams_tmd3702")))
		dev_info->chip_type = ALS_CHIP_AMS_TMD3702_RGB;
	else if (!strncmp(chip_info, "huawei,ams_tmd2702",
		sizeof("huawei,ams_tmd2702")))
		dev_info->chip_type = ALS_CHIP_AMS_TMD3702_RGB;
	else if (!strncmp(chip_info, "huawei,A",
		sizeof("huawei,A")))
		dev_info->chip_type = ALS_CHIP_TSL2591;
	else
		return -1;

	return 0;
}

static int als_liteon_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,liteon_ltr582",
		sizeof("huawei,liteon_ltr582")))
		dev_info->chip_type = ALS_CHIP_LITEON_LTR582;
	else if (!strncmp(chip_info, "huawei,ltr578_als",
		sizeof("huawei,ltr578_als")))
		dev_info->chip_type = ALS_CHIP_LTR578;
	else if (!strncmp(chip_info, "huawei,liteon_ltr2568",
		sizeof("huawei,liteon_ltr2568")))
		dev_info->chip_type = ALS_CHIP_LTR2568;
	else
		return -1;

	return 0;
}

static int als_sensortek_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,stk3321_als",
		sizeof("huawei,stk3321_als")))
		dev_info->chip_type = ALS_CHIP_STK3321;
	else if (!strncmp(chip_info, "huawei,stk3235_als",
		sizeof("huawei,stk3235_als")))
		dev_info->chip_type = ALS_CHIP_STK3235;
	else if (!strncmp(chip_info, "huawei,arrow_stk3338",
		sizeof("huawei,arrow_stk3338")))
		dev_info->chip_type = ALS_CHIP_STK3338;
	else
		return -1;

	return 0;
}

static int als_vishay_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,vishay_vcnl36658",
		sizeof("huawei,vishay_vcnl36658")))
		dev_info->chip_type = ALS_CHIP_VISHAY_VCNL36658;
	else if (!strncmp(chip_info, "huawei,vishay_vcnl36832",
		sizeof("huawei,vishay_vcnl36832")))
		dev_info->chip_type = ALS_CHIP_VISHAY_VCNL36832;
	else
		return -1;

	return 0;
}

static int als_silergy_detect(struct als_device_info *dev_info,
	const char *chip_info)
{
	if (!strncmp(chip_info, "huawei,sy3079_als",
		sizeof("huawei,sy3079_als")))
		dev_info->chip_type = ALS_CHIP_SY3079;
	else if (!strncmp(chip_info, "huawei,silergy_syh399",
		sizeof("huawei,silergy_syh399")))
		dev_info->chip_type = ALS_CHIP_SYH399;
	else
		return -1;

	return 0;
}

static int als_get_chip_type_by_compatible(struct als_device_info *dev_info,
	const char *chip_info)
{
	if ((!dev_info) || (!chip_info))
		return -1;

	if (!als_rohm_detect(dev_info, chip_info)) {
	} else if (!als_avago_detect(dev_info, chip_info)) {
	} else if (!als_ams_detect(dev_info, chip_info)) {
	} else if (!als_liteon_detect(dev_info, chip_info)) {
	} else if (!als_vishay_detect(dev_info, chip_info)) {
	} else if (!als_sensortek_detect(dev_info, chip_info)) {
	} else if (!als_silergy_detect(dev_info, chip_info)) {
	} else {
		return -1;
	}

	return 0;
}

static void read_als_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp;

	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s:read gpio_int1 fail\n", __func__);
	else
		als_data.gpio_int1 = (GPIO_NUM_TYPE)temp;

	if (!of_property_read_u32(dn, "poll_interval", &temp))
		als_data.poll_interval = (uint16_t)temp;

	if (!of_property_read_u32(dn, "reg", &temp))
		als_data.cfg.i2c_address = (uint8_t)temp;

	if (!of_property_read_u32(dn, "init_time", &temp))
		als_data.init_time = (uint16_t)temp;

	if (!of_property_read_u32(dn, "GA1", &temp))
		als_data.ga1 = temp;

	if (!of_property_read_u32(dn, "GA2", &temp))
		als_data.ga2 = temp;

	if (!of_property_read_u32(dn, "GA3", &temp))
		als_data.ga3 = temp;

	if (!of_property_read_u32(dn, "als_phone_type", &temp))
		als_data.als_phone_type = (uint8_t)temp;

	if (!of_property_read_u32(dn, "als_gain_dynamic", &temp))
		als_data.als_gain_dynamic = (uint8_t)temp;

	if (!of_property_read_u32(dn, "als_always_open", &temp))
		als_data.als_always_open = (uint8_t)temp;

	if (!of_property_read_u32(dn, "als_phone_version", &temp))
		als_data.als_phone_version = (uint8_t)temp;
	als_data.als_phone_tp_colour = get_phone_color();

	if (!of_property_read_u32(dn, "atime", &temp))
		als_data.atime = (uint8_t)temp;

	if (!of_property_read_u32(dn, "again", &temp))
		als_data.again = (uint8_t)temp;

	if (!of_property_read_u32(dn, "file_id", &temp))
		read_dyn_file_list((uint16_t)temp);
}

static void read_als_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	int temp = 0;
	int ret;

	if (of_property_read_u32(dn, "is_bllevel_supported", &temp))
		hwlog_err("%s:read als is_bllevle_supported failed\n", __func__);
	else
		als_data.is_bllevel_supported = (int)temp;

	ret = of_property_read_u32(dn, "phone_color_num", &temp);
	if (ret == 0) {
		if (temp < MAX_PHONE_COLOR_NUM) {
			ret = of_property_read_u32_array(dn, "sleeve_detect_threshhold",
				(uint32_t *)get_sleeve_detect_parameter(),
				temp * 2); /* sleeve_detect_pare have 2 members */
			if (ret)
				hwlog_err("%s: read sleeve_detect_threshhold failed!\n", __func__);
		}
	}

	if (!of_property_read_u32(dn, "sensor_list_info_id", &temp))
		add_sensor_list_info_id((uint16_t)temp);
}

void read_als_info_from_dts(struct als_device_info *dev_info,
	struct device_node *dn)
{
	int temp = 0;

	if (of_property_read_u32(dn, "tp_color_from_nv", &temp))
		hwlog_err("%s:read tp_color_from_nv fail\n", __func__);
	else
		dev_info->tp_color_from_nv_flag = (uint8_t)temp;

	if (!of_property_read_u32(dn, "is_cali_supported", &temp))
		dev_info->is_cali_supported = (int)temp;
}

void read_als_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;
	char *info = get_sensor_chip_info_address(ALS);

	pf_data = als_get_platform_data(TAG_ALS);
	dev_info = als_get_device_info(TAG_ALS);
	if (!pf_data || !dev_info) {
		hwlog_err("%s: pf_data or dev_info is NULL\n", __func__);
		return;
	}

	read_chip_info(dn, ALS);
	als_get_chip_type_by_compatible(dev_info, (const char *)info);

	read_als_config1_from_dts(dn, sm);
	read_als_config2_from_dts(dn, sm);
	read_als_info_from_dts(dev_info, dn);
	read_sensorlist_info(dn, ALS);
	select_als_para(pf_data, dev_info, dn);
}

void resend_als_parameters_to_mcu(void)
{
	send_parameter_to_mcu(ALS, SUB_CMD_SET_RESET_PARAM_REQ);
	hwlog_info("%s\n", __func__);
}

void als_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if ((!sm) || len <= ALS) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}

	p = sm + ALS;
	p->spara = (const void *)&als_data;
	p->cfg_data_length = sizeof(als_data);
}

