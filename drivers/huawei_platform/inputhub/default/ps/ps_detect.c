/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: ps detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */
#include "ps_detect.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>

#include "contexthub_route.h"
#include "huawei_thp_attr.h"
#include "ps_sysfs.h"

#define PS_DEVICE_ID_0 0

#define RET_FAIL              (-1)
#define RET_SUCC              0

enum {
	TP_REPLACE_PS = 1,
	SONIC_REPLACE_PS = 2,
};

static u32 tof_replace_ps_flag;
static u32 replace_ps_type;
static u32 replace_ps_by_ultrasonic = false;
static u32 is_need_ps_fun_flag;

static struct ps_platform_data ps_data[PS_DEV_COUNT_MAX] = {
	{
	.cfg = DEF_SENSOR_COM_SETTING,
	.min_proximity_value = 750, /* min proximity value 750 */
	.pwindows_value = 100, /* pwindows value 100 */
	.pwave_value = 60, /* pwave value 60 */
	.threshold_value = 70, /* threshold value 70 */
	.rdata_under_sun = 5500, /* rdata under sun 5500 */
	.ps_pulse_count = 5, /* ps pulse count 5 */
	.gpio_int1 = 206, /* gpio int1 206 */
	.persistent = 0x33,
	.ptime = 0xFF,
	.poll_interval = 250, /* poll interval 250 */
	.init_time = 100, /* init time 100 */
	.ps_oily_threshold = 2, /* ps oily threshold 2 */
	.use_oily_judge = 0,
	.ps_calib_20cm_threshold = 120, /* ps calib 20cm threshold 120 */
	.ps_calib_5cm_threshold = 200, /* ps calib 5cm threshold 200 */
	.ps_calib_3cm_threshold = 250, /* ps calib 3cm threshold 250 */
	.wtime = 100, /* 100 ms */
	.pulse_len = 8, /* 8 us */
	.pgain = 4, /* pgain 4 */
	.led_current = 102, /* 102 mA */
	.prox_avg = 2, /* ps average contrl 2 */
	.offset_max = 200, /* offset max 200 */
	.offset_min = 50, /* offset min 50 */
	.oily_max_near_pdata = 230, /* oily max near pdata 230 */
	.max_oily_add_pdata = 50, /* max oily add pdata 50 */
	.max_near_pdata_loop = 4, /* max near pdata loop 4 */
	.oily_count_size = 12, /* oily count size 12 */
	.ps_tp_threshold = 40, /* ps tp threshold 40 */
	.pwindows_screenon_value = 0,
	.pwave_screenon_value = 0,
	.threshold_screenon_value = 0,
	.digital_offset_max = 0,
	.is_always_on = 0,
	.position = 0,
	.is_check_ps_broke = 0,
	.need_far_near_cali = 0,
	.pwave_cali_threshold = 0,
	.pwindow_cali_threshold = 0,
	.cali_target = 31, /* cali target 31 */
	.max_oily_threshold = 400, /* max oily threshold 400 */
	.is_need_lcd_freq = 0,
	}
};
static struct ps_device_info g_ps_dev_info[PS_DEV_COUNT_MAX];

struct ps_platform_data *ps_get_platform_data(int32_t tag)
{
	if (tag == TAG_PS)
		return (&(ps_data[PS_DEVICE_ID_0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct ps_device_info *ps_get_device_info(int32_t tag)
{
	if (tag == TAG_PS)
		return (&(g_ps_dev_info[PS_DEVICE_ID_0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

ssize_t attr_ultra_ps_feature_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%u\n",
		replace_ps_by_ultrasonic);
}

static void read_ps_chip_info(void)
{
	struct ps_device_info *dev_info = &(g_ps_dev_info[PS_DEVICE_ID_0]);
	char *chip_info = get_sensor_chip_info_address(PS);

	if (!strncmp(chip_info, "huawei,txc-pa224",
		sizeof("huawei,txc-pa224"))) {
		dev_info->chip_type = PS_CHIP_PA224;
	} else if (!strncmp(chip_info, "huawei,ams-tmd2620",
		sizeof("huawei,ams-tmd2620"))) {
		dev_info->chip_type = PS_CHIP_TMD2620;
	} else if (!strncmp(chip_info, "huawei,avago-apds9110",
		sizeof("huawei,avago-apds9110"))) {
		dev_info->chip_type = PS_CHIP_APDS9110;
	} else if (!strncmp(chip_info, "huawei,ams-tmd3725",
		sizeof("huawei,ams-tmd3725"))) {
		dev_info->chip_type = PS_CHIP_TMD3725;
	} else if (!strncmp(chip_info, "huawei,ams_tcs3701",
		sizeof("huawei,ams_tcs3701"))) {
		dev_info->chip_type = PS_CHIP_TCS3701;
	} else if (!strncmp(chip_info, "huawei,liteon-ltr582",
		sizeof("huawei,liteon-ltr582"))) {
		dev_info->chip_type = PS_CHIP_LTR582;
	} else if (!strncmp(chip_info, "huawei,avago_apds9999",
		sizeof("huawei,avago_apds9999"))) {
		dev_info->chip_type = PS_CHIP_APDS9999;
	} else if (!strncmp(chip_info, "huawei,ams_tmd3702",
		sizeof("huawei,ams_tmd3702"))) {
		dev_info->chip_type = PS_CHIP_TMD3702;
	} else if (!strncmp(chip_info, "huawei,vishay-vcnl36658",
		sizeof("huawei,vishay-vcnl36658"))) {
		dev_info->chip_type = PS_CHIP_VCNL36658;
	} else if (!strncmp(chip_info, "huawei,avago_apds9253_006",
		sizeof("huawei,avago_apds9253_006"))) {
		dev_info->chip_type = PS_CHIP_APDS9253_006;
	} else if (!strncmp(chip_info, "huawei,sensortek_stk3638",
		sizeof("huawei,sensortek_stk3638"))) {
		dev_info->chip_type = PS_CHIP_STK3638;
	} else if (!strncmp(chip_info, "huawei,liteon_ltr2594",
		sizeof("huawei,liteon_ltr2594"))) {
		dev_info->chip_type = PS_CHIP_LTR2594;
	} else if (!strncmp(chip_info, "proximity,ps_s001002",
		sizeof("proximity,ps_s001002"))) {
		dev_info->chip_type = PS_CHIP_TCS3701;
	} else {
		return;
	}

	hwlog_debug("%s:%s ps i2c_address suc\n", __func__, chip_info);
}

static void read_ps_config1_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	struct ps_device_info *dev_info = &(g_ps_dev_info[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "ps_cali_supported", &temp))
		hwlog_err("%s: read ps is_cali_supported fail\n", __func__);
	else
		dev_info->ps_cali_supported = (uint8_t)temp;
	temp = of_get_named_gpio(dn, "gpio_int1", 0);
	if (temp < 0)
		hwlog_err("%s: read gpio_int1 fail\n", __func__);
	else
		pf_data->gpio_int1 = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "gpio_int1_sh", &temp))
		hwlog_err("%s: read gpio_int1_sh fail\n", __func__);
	else
		pf_data->gpio_int1_sh = (GPIO_NUM_TYPE)temp;
	if (of_property_read_u32(dn, "min_proximity_value", &temp))
		hwlog_err("%s: read mag min_proximity_value fail\n", __func__);
	else
		pf_data->min_proximity_value = temp;
	if (of_property_read_u32(dn, "pwindows_value", &temp))
		hwlog_err("%s: read pwindows_value fail\n", __func__);
	else
		pf_data->pwindows_value = temp;
	if (of_property_read_u32(dn, "pwave_value", &temp))
		hwlog_err("%s: read pwave_value fail\n", __func__);
	else
		pf_data->pwave_value = temp;
	if (of_property_read_u32(dn, "threshold_value", &temp))
		hwlog_err("%s: read threshold_value fail\n", __func__);
	else
		pf_data->threshold_value = temp;
	if (of_property_read_u32(dn, "pwindows_screenon_value", &temp))
		hwlog_err("%s: read pwindows_screenon_value fail\n", __func__);
	else
		pf_data->pwindows_screenon_value = temp;
}

static void read_ps_config2_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "pwave_screenon_value", &temp))
		hwlog_err("%s: read pwave_screenon_value fail\n", __func__);
	else
		pf_data->pwave_screenon_value = temp;
	if (of_property_read_u32(dn, "threshold_screenon_value", &temp))
		hwlog_err("%s: read threshold_screenon_value fail\n", __func__);
	else
		pf_data->threshold_screenon_value = temp;
	if (of_property_read_u32(dn, "rdata_under_sun", &temp))
		hwlog_err("%s: read rdata_under_sun fail\n", __func__);
	else
		pf_data->rdata_under_sun = temp;
	if (of_property_read_u32(dn, "ps_pulse_count", &temp))
		hwlog_err("%s: read ps_pulse_count fail\n", __func__);
	else
		pf_data->ps_pulse_count = (uint8_t)temp;
	if (of_property_read_u32(dn, "persistent", &temp))
		hwlog_err("%s: read persistent fail\n", __func__);
	else
		pf_data->persistent = (uint8_t)temp;
	if (of_property_read_u32(dn, "ptime", &temp))
		hwlog_err("%s: read ptime fail\n", __func__);
	else
		pf_data->ptime = (uint8_t)temp;
	if (of_property_read_u32(dn, "p_on", &temp))
		hwlog_err("%s: read p_on fail\n", __func__);
	else
		pf_data->p_on = (uint8_t)temp;
	if (of_property_read_u32(dn, "poll_interval", &temp))
		hwlog_err("%s: read poll_interval fail\n", __func__);
	else
		pf_data->poll_interval = (uint16_t)temp;
}

static void read_ps_config3_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "use_oily_judge", &temp))
		hwlog_err("%s: read use_oily_judge fail\n", __func__);
	else
		pf_data->use_oily_judge = (uint16_t)temp;
	if (of_property_read_u32(dn, "init_time", &temp))
		hwlog_err("%s: read init_time fail\n", __func__);
	else
		pf_data->init_time = (uint16_t)temp;
	if (of_property_read_u32(dn, "ps_oily_threshold", &temp))
		hwlog_err("%s: read ps_oily_threshold fail\n", __func__);
	else
		pf_data->ps_oily_threshold = (uint8_t)temp;
	if (of_property_read_u32(dn, "ps_calib_20cm_threshold", &temp))
		hwlog_err("%s: read ps_calib_20cm_threshold fail\n", __func__);
	else
		pf_data->ps_calib_20cm_threshold = (uint16_t)temp;
	if (of_property_read_u32(dn, "ps_calib_5cm_threshold", &temp))
		hwlog_err("%s: read ps_calib_5cm_threshold fail\n", __func__);
	else
		pf_data->ps_calib_5cm_threshold = (uint16_t)temp;
	if (of_property_read_u32(dn, "ps_calib_3cm_threshold", &temp))
		hwlog_err("%s: read ps_calib_3cm_threshold fail\n", __func__);
	else
		pf_data->ps_calib_3cm_threshold = (uint16_t)temp;
	if (of_property_read_u32(dn, "wtime", &temp))
		hwlog_err("%s: read wtime fail\n", __func__);
	else
		pf_data->wtime = (uint8_t)temp;
	if (of_property_read_u32(dn, "led_current", &temp))
		hwlog_err("%s: read led_current fail\n", __func__);
	else
		pf_data->led_current = (uint8_t)temp;
}

static void read_ps_config4_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "pulse_len", &temp))
		hwlog_err("%s: read pulse_len fail\n", __func__);
	else
		pf_data->pulse_len = (uint8_t)temp;
	if (of_property_read_u32(dn, "pgain", &temp))
		hwlog_err("%s: read pgain fail\n", __func__);
	else
		pf_data->pgain = (uint8_t)temp;
	if (of_property_read_u32(dn, "prox_avg", &temp))
		hwlog_err("%s: read prox_avg fail\n", __func__);
	else
		pf_data->prox_avg = (uint8_t)temp;
	if (of_property_read_u32(dn, "offset_max", &temp))
		hwlog_err("%s: read offset_max fail\n", __func__);
	else
		pf_data->offset_max = (uint8_t)temp;
	if (of_property_read_u32(dn, "digital_offset_max", &temp))
		hwlog_err("%s: read digital_offset_max fail\n", __func__);
	else
		pf_data->digital_offset_max = (uint8_t)temp;
	if (of_property_read_u32(dn, "offset_min", &temp))
		hwlog_err("%s: read offset_min fail\n", __func__);
	else
		pf_data->offset_min = (uint8_t)temp;
	if (of_property_read_u32(dn, "oily_max_near_pdata", &temp))
		hwlog_err("%s: read oily_max_near_pdata fail\n", __func__);
	else
		pf_data->oily_max_near_pdata = (uint16_t)temp;
	if (of_property_read_u32(dn, "max_oily_add_pdata", &temp))
		hwlog_err("%s: read max_oily_add_pdata fail\n", __func__);
	else
		pf_data->max_oily_add_pdata = (uint16_t)temp;
}

static void read_ps_config5_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	struct ps_device_info *dev_info = &(g_ps_dev_info[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "max_near_pdata_loop", &temp))
		hwlog_err("%s: read max_near_pdata_loop fail\n", __func__);
	else
		pf_data->max_near_pdata_loop = (uint8_t)temp;
	if (of_property_read_u32(dn, "need_reduce_interference", &temp))
		hwlog_err("%s: read need_reduce_interference fail\n", __func__);
	else
		pf_data->need_reduce_interference = (uint8_t)temp;
	if (of_property_read_u32(dn, "ps_cali_after_sale", &temp))
		hwlog_err("%s: read ps_cali_after_sale fail\n", __func__);
	else
		dev_info->ps_support_cali_after_sale = (uint8_t)temp;
	if (of_property_read_u32(dn, "oily_count_size", &temp))
		hwlog_err("%s: read oily_count_size fail\n", __func__);
	else
		pf_data->oily_count_size = (uint8_t)temp;
	if (of_property_read_u32(dn, "ps_tp_threshold", &temp))
		hwlog_err("%s: read uint16_t fail\n", __func__);
	else
		pf_data->ps_tp_threshold = (uint16_t)temp;
	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s: read ps file_id fail\n", __func__);
	else
		read_dyn_file_list((uint16_t)temp);
	if (of_property_read_u32(dn, "support_absolute_threshold", &temp))
		hwlog_err("%s: read ps support_absolute fail\n", __func__);
	else
		dev_info->ps_support_abs_threshold = temp;
	if (of_property_read_u32(dn, "is_always_on", &temp))
		hwlog_err("%s: read is_always_on fail\n", __func__);
	else
		pf_data->is_always_on = (uint8_t)temp;
}

static void read_ps_config6_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	struct ps_device_info *dev_info = &(g_ps_dev_info[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "position", &temp))
		hwlog_err("%s: read position fail\n", __func__);
	else
		pf_data->position = (uint8_t)temp;
	if (of_property_read_u32(dn, "is_check_ps_broke", &temp))
		hwlog_err("%s:read is_check_ps_broke fail\n", __func__);
	else
		pf_data->is_check_ps_broke = (uint8_t)temp;
	if (of_property_read_u32(dn, "need_far_near_cali", &temp))
		hwlog_err("%s: read need_far_near_cali fail\n", __func__);
	else
		pf_data->need_far_near_cali = (uint8_t)temp;
	if (of_property_read_u32(dn, "pwave_cali_threshold", &temp))
		hwlog_err("%s: read pwave_cali_threshold fail\n", __func__);
	else
		pf_data->pwave_cali_threshold = (uint16_t)temp;
	if (of_property_read_u32(dn, "pwindow_cali_threshold", &temp))
		hwlog_err("%s: read pwindow_cali_threshold fail\n", __func__);
	else
		pf_data->pwindow_cali_threshold = (uint16_t)temp;
	if (of_property_read_u32(dn, "cali_target", &temp))
		hwlog_err("%s: read cali_target fail\n", __func__);
	else
		pf_data->cali_target = (uint16_t)temp;
	if (of_property_read_u32(dn, "update_ps_cali", &temp))
		hwlog_err("%s: read rdata_under_sun fail\n", __func__);
	else
		dev_info->update_ps_cali = temp;
	if (of_property_read_u32(dn, "max_oily_threshold", &temp))
		hwlog_err("%s: read max_oily_threshold fail\n", __func__);
	else
		pf_data->max_oily_threshold = temp;
}

static int send_ps_para_to_mcu(int32_t tag, const unsigned char *input, int len)
{
	struct write_info pkg_ap = { 0 };
	pkt_parameter_req_t cpkt;
	struct pkt_header *hd = (struct pkt_header *)&cpkt;
	/* trans to uint8,so multi 2 */
	int ret;

	hwlog_info("%s len=%d\n", __func__, len);

	ret = memset_s(&cpkt, sizeof(cpkt), 0, sizeof(cpkt));
	if (ret != EOK) {
		hwlog_err("send_ps_config_to_mcu memset_s cpkt err\n");
		return RET_FAIL;
	}

	pkg_ap.tag = tag;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	cpkt.subcmd = SUB_CMD_PS_NEED_FLAG_REQ;
	pkg_ap.wr_buf = &hd[1];
	/* trans to uint8,so multi 2 */
	pkg_ap.wr_len = sizeof(int) * len + SUBCMD_LEN;
	ret = memcpy_s(cpkt.para, sizeof(cpkt.para), input, len);
	if (ret != EOK) {
		hwlog_err("send_ps_config_to_mcu memcpy_s cpkt err\n");
		return RET_FAIL;
	}

	ret = write_customize_cmd(&pkg_ap, NULL, false);
	if (ret != 0) {
		hwlog_err("send_ps_config_to_mcu failed ret = %d\n", ret);
		return RET_FAIL;
	}
	return RET_SUCC;
}

void ps_set_config_to_mcu(int32_t tag)
{
	int ret;
	unsigned char ps_para[sizeof(int)] = { 0 };

	if (!is_need_ps_fun_flag) {
		hwlog_info("this product need ps function\n");
		return;
	}
	ret = memcpy_s(ps_para, sizeof(int), &is_need_ps_fun_flag, sizeof(int));
	if (ret != EOK) {
		hwlog_err("ps_set_config_to_mcu memcpy_s para err\n");
		return;
	}
	ret = send_ps_para_to_mcu(tag, ps_para, sizeof(int));
	if (ret != EOK) {
		hwlog_err("ps_set_config_to_mcu failed\n");
		return;
	}
}

static void read_ps_config7_from_dts(struct device_node *dn)
{
	struct ps_platform_data *pf_data = &(ps_data[PS_DEVICE_ID_0]);
	int temp = 0;

	if (of_property_read_u32(dn, "is_need_lcd_freq", &temp))
		hwlog_err("%s: read is_need_lcd_freq fail\n", __func__);
	else
		pf_data->is_need_lcd_freq = temp;
}

void read_ps_data_from_dts(struct device_node *dn)
{
	int temp = 0;
	const char *ps_chip_info = "proximity-tp";
	const char *ps_vendor = "huawei";

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read ps sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);

	if (tof_replace_ps_flag != 0) {
		hwlog_info("tof_replace_ps_flag is true skip read ps dts %u\n",
			tof_replace_ps_flag);
		return;
	}
	if (replace_ps_type == TP_REPLACE_PS) {
		struct sensorlist_info *info = NULL;

		info = get_sensorlist_info_by_index(PS);
		if (strncpy_s(info->name, sizeof(info->name), ps_chip_info,
			(MAX_CHIP_INFO_LEN - 1)) != EOK)
			return;
		info->name[MAX_CHIP_INFO_LEN - 1] = '\0';
		if (strncpy_s(info->vendor, sizeof(info->vendor), ps_vendor,
			(MAX_CHIP_INFO_LEN - 1)) != EOK)
			return;
		info->vendor[MAX_CHIP_INFO_LEN - 1] = '\0';
		hwlog_info("tp replace ps, name is %s vendor is %s\n",
			info->name, info->vendor);
		return;
	} else if (replace_ps_type == SONIC_REPLACE_PS) {
		hwlog_info("%s:ultrasonic replace ps is true", __func__);
		return;
	}

	read_chip_info(dn, PS);
	read_ps_chip_info();
	read_ps_config1_from_dts(dn);
	read_ps_config2_from_dts(dn);
	read_ps_config3_from_dts(dn);
	read_ps_config4_from_dts(dn);
	read_ps_config5_from_dts(dn);
	read_ps_config6_from_dts(dn);
	read_ps_config7_from_dts(dn);
	read_sensorlist_info(dn, PS);
}

/* recieve ps offset data from mcu and write to nv */
int ps_data_from_mcu(const struct pkt_header *head)
{
	int ret = -1;

	switch (((pkt_ps_calibrate_data_req_t *)head)->subcmd) {
	case SUB_CMD_SET_WRITE_NV_ATTER_SALE:
		ret = write_ps_offset_to_nv((int *)(
			((pkt_ps_calibrate_data_req_t *)head)->calibrate_data),
			PS_CALIDATA_NV_SIZE);
		break;
	default:
		hwlog_err("uncorrect subcmd 0x%x\n",
			((pkt_ps_calibrate_data_req_t *)head)->subcmd);
		break;
	}
	return ret;
}

int ps_is_replace_by_ultrasonic(int ret, int index,
	struct sensor_combo_cfg *cfg)
{
	if (ret && (replace_ps_type == SONIC_REPLACE_PS)) {
		hwlog_info("%s is done\n", __func__);
		replace_ps_by_ultrasonic = true;
		ret = RET_SUCC;
	}

	return ret;
}

int ps_sensor_detect(struct device_node *dn,
	struct sensor_detect_manager *sm, int index)
{
	int ret;
	struct sensor_detect_manager *p = NULL;
	struct sensor_combo_cfg cfg = { 0 };
#ifdef CONFIG_HUAWEI_THP
	t_ap_sensor_ops_record *record = get_all_ap_sensor_operations();
#endif

	if ((!dn) || (index < 0) || (index >= SENSOR_MAX) || (!sm))
		return RET_FAIL;

	ret = of_property_read_u32(dn,
		"replace_by_tof", &tof_replace_ps_flag);
	if (ret)
		hwlog_info("no replace_by_tof config ret:%d\n", ret);

	if (tof_replace_ps_flag) {
		hwlog_info("get replace_by_tof flag %d, skip detect\n",
			tof_replace_ps_flag);
		return ret;
	}
	if (of_property_read_u32(dn, "is_need_ps_fun", &is_need_ps_fun_flag))
		hwlog_err("%s: read is_need_ps_fun fail\n", __func__);

	ret = of_property_read_u32(dn, "replace_by_who", &replace_ps_type);
	if (ret) {
		hwlog_info("get replace_by_who failed, use defalut value\n");
		replace_ps_type = 0;
	} else {
		hwlog_info("get replace_by_who successful, replace_ps_type =%d\n",
			replace_ps_type);
	}
	if (replace_ps_type == TP_REPLACE_PS) {
		hwlog_info("get replace_by_who flag %d, skip detect\n",
			replace_ps_type);
#ifdef CONFIG_HUAWEI_THP
		record[TAG_PS].work_on_ap = true;
		record[TAG_PS].ops.enable = thp_set_prox_switch_status;
		ret = is_tp_detected();
		return ret;
#endif
	}

	ret = _device_detect(dn, index, &cfg);
	ret = ps_is_replace_by_ultrasonic(ret, index, &cfg);
	if (!ret) {
		p = sm + index;
		if (memcpy_s((void *)p->spara, sizeof(struct sensor_combo_cfg),
			(void *)&cfg, sizeof(struct sensor_combo_cfg)) != EOK)
			ret = RET_FAIL;
	}

	return ret;
}

void ps_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= PS) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_ps_dev_info[PS_DEVICE_ID_0].obj_tag = TAG_PS;
	g_ps_dev_info[PS_DEVICE_ID_0].detect_list_id = PS;
	g_ps_dev_info[PS_DEVICE_ID_0].ps_dev_index = 0;
	p = sm + PS;
	p->spara = (const void *)&(ps_data[PS_DEVICE_ID_0]);
	p->cfg_data_length = sizeof(ps_data[PS_DEVICE_ID_0]);
}

