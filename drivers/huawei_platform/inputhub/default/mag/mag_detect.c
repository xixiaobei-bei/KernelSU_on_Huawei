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
#include "mag_sysfs.h"

#define MAG_DEVICE_ID_0 0
#define MAG_DEVICE_ID_1 1
#define CURRENT_FAC_AKM_INIT  0
#define SOFTIRON_ARGS_NUM 9
#define MAG_SENSOR_DEFAULT_ID "2"

static char *mag_sensor_id = MAG_SENSOR_DEFAULT_ID;
static const char *str_soft_para = "softiron_parameter";
static const char *str_soft_para_far = "softiron_parameter_far";
static struct mag_platform_iron_data mag_iron_data = {
	.pdc_data_far = { 0 },
};

static struct compass_platform_data mag_data[MAG_DEV_COUNT_MAX] = {
	{
	.cfg = DEF_SENSOR_COM_SETTING,
	.poll_interval = 10, /* interval 10 ms */
	.outbit = 0,
	.axis_map_x = 0,
	.axis_map_y = 1,
	.axis_map_z = 2, /* index 2 */
	.angle = 0,
	.soft_filter = 0,
	.calibrate_method = 1,
	.charger_trigger = 0,
	},
};
static struct mag_device_info g_mag_dev_info[MAG_DEV_COUNT_MAX];

struct compass_platform_data *mag_get_platform_data(int32_t tag)
{
	if (tag == TAG_MAG)
		return (&(mag_data[MAG_DEVICE_ID_0]));
	else if (tag == TAG_MAG1)
		return (&(mag_data[MAG_DEVICE_ID_1]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct mag_device_info *mag_get_device_info(int32_t tag)
{
	if (tag == TAG_MAG)
		return (&(g_mag_dev_info[MAG_DEVICE_ID_0]));
	else if (tag == TAG_MAG1)
		return (&(g_mag_dev_info[MAG_DEVICE_ID_1]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

char *mag_get_sensors_id_string(void)
{
	return mag_sensor_id;
}

void mag_get_sensors_id_from_dts(struct device_node *dn)
{
	if (dn == NULL)
		return;
	if (of_property_read_string(dn,
		"mag_sensor_id", (const char **)&mag_sensor_id)) {
		mag_sensor_id = MAG_SENSOR_DEFAULT_ID;
		hwlog_err("mag_sensor_id err\n");
	}
	hwlog_info("mag_sensor_id:%s\n", mag_sensor_id);
}

static void read_mag_config1_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct compass_platform_data *pf_data =
		(struct compass_platform_data *)sm->spara;
	int temp = 0;

	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s:read %s mag poll_interval fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->poll_interval = (uint16_t)temp;

	if (of_property_read_u32(dn, "axis_map_x", &temp))
		hwlog_err("%s:read %s mag axis_map_x fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->axis_map_x = (uint8_t)temp;

	if (of_property_read_u32(dn, "axis_map_y", &temp))
		hwlog_err("%s:read %s mag axis_map_y fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->axis_map_y = (uint8_t)temp;

	if (of_property_read_u32(dn, "axis_map_z", &temp))
		hwlog_err("%s:read %s mag axis_map_z fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->axis_map_z = (uint8_t)temp;

	if (of_property_read_u32(dn, "negate_x", &temp))
		hwlog_err("%s:read %s mag negate_x fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->negate_x = (uint8_t)temp;

	if (of_property_read_u32(dn, "negate_y", &temp))
		hwlog_err("%s:read %s mag negate_y fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->negate_y = (uint8_t)temp;

	if (of_property_read_u32(dn, "negate_z", &temp))
		hwlog_err("%s:read %s mag negate_z fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->negate_z = (uint8_t)temp;

	if (of_property_read_u32(dn, "angle", &temp))
		hwlog_err("%s:read %s mag angle fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->angle = (uint16_t)temp;
}

static void read_mag_config2_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct compass_platform_data *pf_data =
		(struct compass_platform_data *)sm->spara;
	int temp = 0;
	int aux = 0;

	if (of_property_read_u32(dn, "outbit", &temp))
		hwlog_err("%s:read %s mag outbit fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->outbit = (uint8_t)temp;

	if (of_property_read_u32(dn, "softfilter", &temp))
		hwlog_err("%s:read %s mag softfilter fail; use default value:%d\n",
			__func__, sm->sensor_name_str, pf_data->soft_filter);
	else
		pf_data->soft_filter = (uint8_t)temp;

	if (of_property_read_u32(dn, "calibrate_method", &temp))
		hwlog_err("%s:read %s mag calibrate_method fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->calibrate_method = (uint8_t)temp;

	if (of_property_read_u32(dn, "charger_trigger", &temp))
		hwlog_err("%s:read %s mag charger_trigger fail; use default value:%d\n",
			__func__, sm->sensor_name_str, pf_data->charger_trigger);
	else
		pf_data->charger_trigger = (uint8_t)temp;

	if (of_property_read_u32(dn, "aux_prop", &aux))
		hwlog_err("%s:read %s aux_prop fail\n",
			__func__, sm->sensor_name_str);
	else
		hwlog_info("%s:read %s aux_prop %d\n",
			__func__, sm->sensor_name_str, aux);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s:read %s file_id fail\n", __func__, sm->sensor_name_str);
	else if (aux)
		read_aux_file_list(temp, sm->tag);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read %s mag sensor_list_info_id fail\n",
			__func__, sm->sensor_name_str);
	else
		add_sensor_list_info_id((uint16_t)temp);

	temp = of_get_named_gpio(dn, "gpio_reset", 0);
	if (temp < 0)
		hwlog_err("%s:read %s gpio_rst fail\n",
			__func__, sm->sensor_name_str);
	else
		pf_data->gpio_rst = (GPIO_NUM_TYPE)temp;
}

static void read_mag_info_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct mag_device_info *dev_info = NULL;
	int temp = 0;

	dev_info = mag_get_device_info(sm->tag);
	if (dev_info == NULL)
		return;

	if (of_property_read_u32(dn, "threshold_for_als_calibrate", &temp))
		hwlog_err("%s:read %s mag threshold_for_als_calibrate fail\n",
			__func__, sm->sensor_name_str);
	else
		dev_info->mag_threshold_for_als_calibrate = temp;

	if (of_property_read_u32(dn, "akm_cal_algo", &temp)) {
		hwlog_debug("%s:read %s mag akm_cal_algo fail\n",
			__func__, sm->sensor_name_str);
		dev_info->akm_cal_algo = 0;
	} else {
		dev_info->akm_cal_algo = ((temp == 1) ? 1 : 0);
		hwlog_debug("%s: mag %s akm_cal_algo=%d\n",
			__func__, sm->sensor_name_str, dev_info->akm_cal_algo);
	}

	if (of_property_read_u32(dn, "akm_need_charger_current", &temp)) {
		hwlog_debug("%s:read mag akm_need_charger_current fail\n",
			__func__);
		dev_info->akm_need_charger_current = 0;
	} else {
		dev_info->akm_need_charger_current = ((temp == 1) ? 1 : 0);
		hwlog_debug("%s: mag akm_need_charger_current=%d\n",
			__func__, dev_info->akm_need_charger_current);
	}

	if (of_property_read_u32(dn, "akm_current_x_fac", &temp)) {
		hwlog_debug("%s:read mag akm_current_x_fac fail\n", __func__);
		dev_info->akm_current_x_fac = CURRENT_FAC_AKM_INIT;
	} else {
		dev_info->akm_current_x_fac = temp;
		hwlog_debug("%s: mag akm_current_x_fac=%d\n",
			__func__, dev_info->akm_current_x_fac);
	}

	if (of_property_read_u32(dn, "akm_current_y_fac", &temp)) {
		hwlog_debug("%s:read mag akm_current_y_fac fail\n", __func__);
		dev_info->akm_current_y_fac = CURRENT_FAC_AKM_INIT;
	} else {
		dev_info->akm_current_y_fac = temp;
		hwlog_debug("%s: mag akm_current_y_fac=%d\n",
			__func__, dev_info->akm_current_y_fac);
	}

	if (of_property_read_u32(dn, "akm_current_z_fac", &temp)) {
		hwlog_debug("%s:read mag akm_current_z_fac fail\n", __func__);
		dev_info->akm_current_z_fac = CURRENT_FAC_AKM_INIT;
	} else {
		dev_info->akm_current_z_fac = temp;
		hwlog_debug("%s: mag akm_current_z_fac=%d\n",
			__func__, dev_info->akm_current_z_fac);
	}
}

void read_mag_data_from_dts(struct device_node *dn,
	struct sensor_detect_manager *sm)
{
	struct compass_platform_data *pf_data = NULL;
	struct mag_device_info *dev_info = NULL;
	int ret;

	if (dn == NULL || sm == NULL)
		return;
	pf_data = (struct compass_platform_data *)sm->spara;
	dev_info = mag_get_device_info(sm->tag);
	if (dev_info == NULL)
		return;

	read_chip_info(dn, sm->sensor_id);
	read_mag_config1_from_dts(dn, sm);
	read_mag_config2_from_dts(dn, sm);
	read_mag_info_from_dts(dn, sm);
	read_sensorlist_info(dn, MAG);

	/* below only for MAG */
	if (sm->sensor_id != MAG)
		return;
	ret = fill_extend_data_in_dts(dn, str_soft_para,
		pf_data->pdc_data, PDC_SIZE,
		EXTEND_DATA_TYPE_IN_DTS_BYTE);
	if (ret)
		hwlog_err("%s:fill_extend_data_in_dts fail\n",
			str_soft_para);
	ret = fill_extend_data_in_dts(dn, str_soft_para_far,
		mag_iron_data.pdc_data_far, PDC_SIZE,
		EXTEND_DATA_TYPE_IN_DTS_BYTE);
	if (ret) {
		dev_info->mag_folder_function = 0;
		hwlog_err("%s:fill_extend_data_in_dts fail\n",
			str_soft_para_far);
	} else {
		dev_info->mag_folder_function = 1;
	}
}

static int set_far_softiron(int32_t tag, const unsigned char *input, int len)
{
	struct write_info pkg_ap = { 0 };
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	/* trans to uint8,so multi 2 */
	int gyro_args_to_mcu[SOFTIRON_ARGS_NUM * 2] = {0};
	uint32_t size = sizeof(gyro_args_to_mcu) / sizeof(gyro_args_to_mcu[0]);
	int ret;
	int i;

	hwlog_info("%s len=%d size=%d\n", __func__, len, size);
	if (len != PDC_SIZE || len < size)
		return -1;

	for (i = 0; i < size; i++) {
		gyro_args_to_mcu[i] = (int)input[i];
		hwlog_info("gyro_args_to_mcu %d:%d\n", i, gyro_args_to_mcu[i]);
	}

	ret = memset_s(&cpkt, sizeof(cpkt), 0, sizeof(cpkt));
	if (ret != EOK) {
		hwlog_err("set_mag_far_softiron memset_s cpkt err\n");
		return -1;
	}

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = SUB_CMD_MAG_FAR_SOFTIRON;
	pkg_ap.wr_buf = &hd[1];
	/* trans to uint8,so multi 2 */
	pkg_ap.wr_len = sizeof(int) * SOFTIRON_ARGS_NUM * 2 + SUBCMD_LEN;
	ret = memcpy_s(cpkt.para, sizeof(cpkt.para), gyro_args_to_mcu,
		sizeof(int) * SOFTIRON_ARGS_NUM * 2); /* multi 2 to uint8 */
	if (ret != EOK) {
		hwlog_err("set_mag_far_softiron memcpy_s cpkt err\n");
		return -1;
	}

	ret = write_customize_cmd(&pkg_ap, NULL, false);
	if (ret != 0) {
		hwlog_err("set %s sensor_softiron failed, ret = %d in %s\n",
			obj_tag_str[tag], ret, __func__);
		return -1;
	}
	return 0;
}

int mag_set_far_softiron(int32_t tag)
{
	return set_far_softiron(tag, mag_iron_data.pdc_data_far,
		sizeof(mag_iron_data.pdc_data_far));
}

int mag1_data_from_mcu(const struct pkt_header *head)
{
	switch (((pkt_mag_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_CALIBRATE_DATA_REQ:
		return write_magsensor_calibrate_data_to_nv(
			((pkt_mag1_calibrate_data_req_t *)head)->calibrate_data,
			MAG_MAX_CALIB_NV_SIZE);
	default:
		hwlog_err("uncorrect subcmd 0x%x\n",
			((pkt_mag1_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}

int mag_data_from_mcu(const struct pkt_header *head)
{
	struct mag_device_info *dev_info = &(g_mag_dev_info[MAG_DEVICE_ID_0]);

	switch (((pkt_mag_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_CALIBRATE_DATA_REQ:
		if (dev_info->mag_folder_function == 1)
			return write_magsensor_calibrate_data_to_nv(
				((pkt_akm_mag_calibrate_data_req_t *)head)->calibrate_data,
				MAG_MAX_CALIB_NV_SIZE);
		else if (dev_info->akm_cal_algo == 1)
			return write_magsensor_calibrate_data_to_nv(
				((pkt_akm_mag_calibrate_data_req_t *)head)->calibrate_data,
				MAG_MAX_CALIB_NV_SIZE);
		else
			return write_magsensor_calibrate_data_to_nv(
				((pkt_mag_calibrate_data_req_t *)head)->calibrate_data,
				MAG_MAX_CALIB_NV_SIZE);
	default:
		hwlog_err("uncorrect subcmd 0x%x\n",
			((pkt_mag_calibrate_data_req_t *)head)->subcmd);
	}
	return 0;
}

void mag_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;
	int32_t i;

	if (len <= MAG1) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_mag_dev_info[MAG_DEVICE_ID_0].obj_tag = TAG_MAG;
	g_mag_dev_info[MAG_DEVICE_ID_1].obj_tag = TAG_MAG1;
	g_mag_dev_info[MAG_DEVICE_ID_0].detect_list_id = MAG;
	g_mag_dev_info[MAG_DEVICE_ID_1].detect_list_id = MAG1;
	for (i = 0; i < MAG_DEV_COUNT_MAX; i++) {
		g_mag_dev_info[i].mag_dev_index = i;
		p = sm + g_mag_dev_info[i].detect_list_id;
		p->spara = (const void *)&(mag_data[i]);
		p->cfg_data_length = sizeof(mag_data[i]);
		if (i == 0)
			continue;
		if (memcpy_s(&(mag_data[i]), sizeof(mag_data[i]),
			&(mag_data[0]), sizeof(mag_data[0])) != EOK)
			hwlog_info("%s: memcpy_s error i=%d\n", __func__, i);
	}
}

