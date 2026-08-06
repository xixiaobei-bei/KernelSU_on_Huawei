/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: therm detect source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "therm_detect.h"

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

#define THERM_DEVICE_ID_0 0
#define THERM_SENSOR_DEFAULT_ID "65574"
#define THERM_DEFAULT_CALIBRATE_INDEX "1 2"

#ifdef SENSOR_DATA_ACQUISITION
static char *thermo_test_name[THERMO_CAL_NUM] = {
	"THERMO_VERIFY_TEMP_DIFF", "THERMO_LOW_TEMP_1", "THERMO_LOW_TEMP_2",
	"THERMO_LOW_TEMP_3", "THERMO_LOW_TEMP_4", "THERMO_LOW_TEMP_5",
	"THERMO_LOW_TEMP_6", "THERMO_LOW_TEMP_7", "THERMO_LOW_TEMP_8",
	"THERMO_LOW_TEMP_9", "THERMO_LOW_TEMP_10", "THERMO_HIGH_TEMP_1",
	"THERMO_HIGH_TEMP_2", "THERMO_HIGH_TEMP_3", "THERMO_HIGH_TEMP_4",
	"THERMO_HIGH_TEMP_5", "THERMO_HIGH_TEMP_6", "THERMO_HIGH_TEMP_7",
	"THERMO_HIGH_TEMP_8", "THERMO_HIGH_TEMP_9", "THERMO_HIGH_TEMP_10",
	"THERMO_CALI_HA", "THERMO_CALI_HB", "THERMO_VERIFY_TEMP", "THERMO_LOW_TA_MAX",
	"THERMO_LOW_TO_AVG", "THERMO_HIGH_TA_MAX", "THERMO_HIGH_TO_AVG",
};
#endif

static char *therm_sensor_id = THERM_SENSOR_DEFAULT_ID;
static char *therm_calibrate_index = THERM_DEFAULT_CALIBRATE_INDEX;
int g_thermo_min_threshold[THERMO_THRESHOLD_NUM] = { 0 };
int g_thermo_max_threshold[THERMO_THRESHOLD_NUM] = { 0 };
static struct therm_platform_data therm_data[THERM_DEV_COUNT_MAX] = {
	{
	.cfg = {
		.bus_type = TAG_I2C,
		.bus_num = 0,
		.disable_sample_thread = 0,
		{ .i2c_address = 0x3A },
	},
	.blackbody_count = 2, /* blackbody count 2 */
	.th_cfg = {
		.min_verify_diff_th = 0,
		.max_verify_diff_th = 0,
		.min_low_threshold = 0,
		.max_low_threshold = 0,
		.min_high_threshold = 0,
		.max_high_threshold = 0,
		.min_ha_threshold = 0,
		.max_ha_threshold = 0,
		.min_hb_threshold = 0,
		.max_hb_threshold = 0,
		.min_verify_threshold = 0,
		.max_verify_threshold = 0,
		.ta_max_threshold = 0,
		.to_avg_threshold = 0,
	},
	}
};
static struct therm_device_info g_therm_dev_info[THERM_DEV_COUNT_MAX];

struct therm_platform_data *therm_get_platform_data(int32_t tag)
{
	if (tag == TAG_THERMOMETER)
		return (&(therm_data[THERM_DEVICE_ID_0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

struct therm_device_info *therm_get_device_info(int32_t tag)
{
	if (tag == TAG_THERMOMETER)
		return (&(g_therm_dev_info[THERM_DEVICE_ID_0]));

	hwlog_info("%s error, please check tag %d\n", __func__, tag);
	return NULL;
}

char *therm_get_sensors_id_string(void)
{
	return therm_sensor_id;
}

void therm_get_sensors_id_from_dts(struct device_node *dn)
{
	if (dn == NULL)
		return;
	if (of_property_read_string(dn,
		"therm_sensor_id", (const char **)&therm_sensor_id)) {
		therm_sensor_id = THERM_SENSOR_DEFAULT_ID;
		hwlog_err("therm_sensor_id err\n");
	}
	hwlog_info("therm_sensor_id:%s\n", therm_sensor_id);
}

char *therm_get_calibrate_index_string(void)
{
	return therm_calibrate_index;
}

static void read_thermometer_distance_algo_para(struct device_node *dn)
{
	struct therm_platform_data *pf_data = &therm_data[THERM_DEVICE_ID_0];
	int temp = 0;

	if (of_property_read_u32(dn, "blackbody_count", &temp))
		hwlog_err("blackbody_count err\n");
	else
		pf_data->blackbody_count = (uint8_t)temp;
	hwlog_info("therm blackbody_count:%d\n", temp);

	if (of_property_read_u32(dn, "ar_mode", &temp))
		hwlog_err("ar_mode err\n");
	else
		pf_data->ar_mode = (uint8_t)temp;
	hwlog_info("therm ar_mode:%d\n", temp);

	if (of_property_read_u32(dn, "k1", &temp))
		hwlog_err("therm k1 err\n");
	else
		pf_data->k1 = temp;

	if (of_property_read_u32(dn, "k2", &temp))
		hwlog_err("therm k2 err\n");
	else
		pf_data->k2 = temp;

	if (of_property_read_u32(dn, "c1", &temp))
		hwlog_err("therm c1 err\n");
	else
		pf_data->c1 = temp;

	if (of_property_read_u32(dn, "c2", &temp))
		hwlog_err("therm c2 err\n");
	else
		pf_data->c2 = temp;

	if (of_property_read_u32(dn, "c3", &temp))
		hwlog_err("therm c3 err\n");
	else
		pf_data->c3 = temp;

	if (of_property_read_u32(dn, "c4", &temp))
		hwlog_err("therm c3 err\n");
	else
		pf_data->c4 = temp;

	if (of_property_read_u32(dn, "c5", &temp))
		hwlog_err("therm c3 err\n");
	else
		pf_data->c5 = temp;
}

void read_thermometer_data_from_dts(struct device_node *dn)
{
	int temp = 0;

	read_chip_info(dn, THERMOMETER);

	if (of_property_read_string(dn, "calibrate_index", (const char **)&therm_calibrate_index)) {
		therm_calibrate_index = THERM_DEFAULT_CALIBRATE_INDEX;
		hwlog_err("therm_calibrate_index err\n");
	}
	hwlog_info("therm_calibrate_index:%s\n", therm_calibrate_index);

	read_thermometer_distance_algo_para(dn);
	read_thermo_thre_from_dts1(dn);
	read_thermo_thre_from_dts2(dn);

	if (of_property_read_u32(dn, "file_id", &temp))
		hwlog_err("%s: read thermometer file_id fail\n", __func__);
	else
		read_dyn_file_list((uint16_t)temp);

	if (of_property_read_u32(dn, "sensor_list_info_id", &temp))
		hwlog_err("%s:read thermometer sensor_list_info_id fail\n", __func__);
	else
		add_sensor_list_info_id((uint16_t)temp);
}

void read_thermo_thre_from_dts1(struct device_node *dn)
{
	struct therm_platform_data *pf_data = &therm_data[THERM_DEVICE_ID_0];
	int temp = 0;

	if (of_property_read_u32(dn, "min_verify_diff_th", &temp))
		hwlog_err("thermo min_verify_diff_th err\n");
	else
		pf_data->th_cfg.min_verify_diff_th = temp;
	hwlog_info("therm min_verify_diff_th:%d\n", temp);

	if (of_property_read_u32(dn, "max_verify_diff_th", &temp))
		hwlog_err("thermo max_verify_diff_th err\n");
	else
		pf_data->th_cfg.max_verify_diff_th = temp;
	hwlog_info("therm max_verify_diff_th:%d\n", temp);

	if (of_property_read_u32(dn, "min_low_th", &temp))
		hwlog_err("thermo min_low_threshold err\n");
	else
		pf_data->th_cfg.min_low_threshold = temp;
	hwlog_info("thermo min_low_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "max_low_th", &temp))
		hwlog_err("thermo max_low_threshold err\n");
	else
		pf_data->th_cfg.max_low_threshold = temp;
	hwlog_info("thermo max_low_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "min_high_th", &temp))
		hwlog_err("thermo min_high_threshold err\n");
	else
		pf_data->th_cfg.min_high_threshold = temp;
	hwlog_info("thermo min_high_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "max_high_th", &temp))
		hwlog_err("thermo max_high_threshold err\n");
	else
		pf_data->th_cfg.max_high_threshold = temp;
	hwlog_info("thermo max_high_threshold:%d\n", temp);
}

void read_thermo_thre_from_dts2(struct device_node *dn)
{
	struct therm_platform_data *pf_data = &therm_data[THERM_DEVICE_ID_0];
	int temp = 0;

	if (of_property_read_u32(dn, "min_ha_th", &temp))
		hwlog_err("thermo min_ha_threshold err\n");
	else
		pf_data->th_cfg.min_ha_threshold = temp;
	hwlog_info("thermo min_ha_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "max_ha_th", &temp))
		hwlog_err("thermo max_ha_threshold err\n");
	else
		pf_data->th_cfg.max_ha_threshold = temp;
	hwlog_info("thermo max_ha_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "min_hb_th", &temp))
		hwlog_err("thermo min_hb_threshold err\n");
	else
		pf_data->th_cfg.min_hb_threshold = temp;
	hwlog_info("thermo min_hb_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "max_hb_th", &temp))
		hwlog_err("thermo max_hb_threshold err\n");
	else
		pf_data->th_cfg.max_hb_threshold = temp;
	hwlog_info("thermo max_hb_threshold:%d\n", temp);

	if (of_property_read_u32(dn, "min_verify_th", &temp))
		hwlog_err("thermo min_verify_threshold err\n");
	else
		pf_data->th_cfg.min_verify_threshold = temp;
	hwlog_info("thermo min_verify_threshold:%d\n", temp);

	if (of_property_read_u32(dn,"max_verify_th", &temp))
		hwlog_err("thermo max_verify_threshold err\n");
	else
		pf_data->th_cfg.max_verify_threshold = temp;
	hwlog_info("thermo max_verify_threshold:%d\n", temp);

	if (of_property_read_u32(dn,"ta_max_th", &temp))
		hwlog_err("thermo ta_max_threshold err\n");
	else
		pf_data->th_cfg.ta_max_threshold = temp;
	hwlog_info("thermo ta_max_threshold:%d\n", temp);

	if (of_property_read_u32(dn,"to_avg_th", &temp))
		hwlog_err("thermo to_avg_threshold err\n");
	else
		pf_data->th_cfg.to_avg_threshold = temp;
	hwlog_info("thermo to_avg_threshold:%d\n", temp);
}

#ifdef SENSOR_DATA_ACQUISITION
void thermometer_set_big_data_th(void)
{
	int i;
	struct therm_platform_data *pf_data = &therm_data[THERM_DEVICE_ID_0];

	g_thermo_min_threshold[0] = pf_data->th_cfg.min_verify_diff_th;
	g_thermo_max_threshold[0] = pf_data->th_cfg.max_verify_diff_th;
	for (i = 1; i < THERMO_TATO_LEN; i++) {
		g_thermo_min_threshold[i] = pf_data->th_cfg.min_low_threshold;
		g_thermo_min_threshold[i + THERMO_TATO_LEN] = pf_data->th_cfg.max_low_threshold;
		g_thermo_max_threshold[i] = pf_data->th_cfg.min_high_threshold;
		g_thermo_max_threshold[i + THERMO_TATO_LEN] = pf_data->th_cfg.max_high_threshold;
	}
	/* min_threshold[21] is min_ha_threshold */
	g_thermo_min_threshold[21] = pf_data->th_cfg.min_ha_threshold;
	/* max_threshold[21] is max_ha_threshold */
	g_thermo_max_threshold[21] = pf_data->th_cfg.max_ha_threshold;
	/* min_threshold[22] is min_hb_threshold */
	g_thermo_min_threshold[22] = pf_data->th_cfg.min_hb_threshold;
	/* max_threshold[22] is max_hb_threshold */
	g_thermo_max_threshold[22] = pf_data->th_cfg.max_hb_threshold;
	/* min_threshold[23] si min_verify_threshold */
	g_thermo_min_threshold[23] = pf_data->th_cfg.min_verify_threshold;
	/* max_threshold[23] is max_verify_threshold */
	g_thermo_max_threshold[23] = pf_data->th_cfg.max_verify_threshold;
	/* min_threshold[24] si ta_max_threshold */
	g_thermo_min_threshold[24] = pf_data->th_cfg.ta_max_threshold;
	/* max_threshold[24] is to_avg_threshold */
	g_thermo_max_threshold[24] = pf_data->th_cfg.to_avg_threshold;
	/* min_threshold[25] is min_low_threshold */
	g_thermo_min_threshold[25] = pf_data->th_cfg.min_low_threshold;
	/* max_threshold[25] is max_low_threshold */
	g_thermo_max_threshold[25] = pf_data->th_cfg.max_low_threshold;
	/* min_threshold[26] is ta_max_threshold */
	g_thermo_min_threshold[26] = pf_data->th_cfg.ta_max_threshold;
	/* max_threshold[26] is to_avg_threshold */
	g_thermo_max_threshold[26] = pf_data->th_cfg.to_avg_threshold;
	/* min_threshold[27] is min_high_threshold */
	g_thermo_min_threshold[27] = pf_data->th_cfg.min_high_threshold;
	/* min_threshold[27] is max_high_threshold */
	g_thermo_max_threshold[27] = pf_data->th_cfg.max_high_threshold;
}

void thermometer_big_data_from_mcu(const struct pkt_header *head,
	struct therm_device_info *dev_info)
{
	int i;
	int *self_cali_data = NULL;
	struct sensor_eng_cal_test therm_test;

	dev_info->therm_upload_data_flag = ((pkt_thermometer_data_req_t *)head)->return_data[1];
	hwlog_info("%s :therm_upload_data_flag %d\n", __func__, dev_info->therm_upload_data_flag);
	self_cali_data = ((pkt_thermometer_data_req_t *)head)->return_data;
	thermometer_set_big_data_th();
	if (memset_s(&therm_test, sizeof(therm_test), 0, sizeof(therm_test)) != EOK)
		hwlog_err("%s: memset_s therm_test err\n", __func__);
	hwlog_info("%s: is sport data collect\n", __func__);

	therm_test.cal_value = self_cali_data + THERMO_INDEX;
	therm_test.value_num = THERMO_CAL_NUM;
	therm_test.threshold_num = THERMO_THRESHOLD_NUM;
	therm_test.first_item = THERMO_VERIFY_TEMP_DIFF;
	therm_test.min_threshold = g_thermo_min_threshold;
	therm_test.max_threshold = g_thermo_max_threshold;
	if (memcpy_s(therm_test.name, sizeof(therm_test.name),
		THERMO_TEST_CAL, sizeof(THERMO_TEST_CAL)) != EOK)
		hwlog_err("%s: memcpy_s therm_tets.name err\n", __func__);
	if (memcpy_s(therm_test.result, sizeof(therm_test.result),
		sensor_cal_result(dev_info->therm_calibration_res),
		(strlen(sensor_cal_result(dev_info->therm_calibration_res)) + 1)) != EOK)
		hwlog_err("%s: memcpy_s therm_test.result err\n", __func__);
	for (i = 0; i < THERMO_CAL_NUM; i++)
		therm_test.test_name[i] = thermo_test_name[i];
	/* 2 meas high thermo test done */
	if (dev_info->therm_upload_data_flag == THERMO_INDEX) {
		for (i = 0; i < THERMO_BIG_DATA_LEN; i++)
			hwlog_info("%s: thermo get big data from mcu %d",
				__func__, *(self_cali_data + i));
		enq_notify_work_sensor(therm_test);
		hwlog_info("%s: thermo eng_notify_work_sensor success\n", __func__);
	}
	hwlog_info("%s: is sport data collect done\n", __func__);
}
#endif

int thermometer_data_from_mcu(const struct pkt_header *head)
{
	struct therm_device_info *dev_info =
		&g_therm_dev_info[THERM_DEVICE_ID_0];
	int value;
	char buf[SELFTEST_RESULT_MAXLEN];

	switch (((pkt_thermometer_data_req_t *)head)->subcmd) {
	case SUB_CMD_SELFCALI_REQ:
		value = ((pkt_thermometer_data_req_t *)head)->return_data[0];
		if (value == 0)
			dev_info->therm_calibration_res = SUC;
		else
			dev_info->therm_calibration_res = value;
		hwlog_info("therm calibrate cmd %d, value %d, result %d\n",
			((pkt_thermometer_data_req_t *)head)->subcmd,
			value, dev_info->therm_calibration_res);
		calibrate_processing = 0;
#ifdef SENSOR_DATA_ACQUISITION
		thermometer_big_data_from_mcu(head, dev_info);
#endif
		break;
	case SUB_CMD_SELFTEST_REQ:
		value = ((pkt_thermometer_data_req_t *)head)->return_data[0];
		hwlog_info("therm selftest cmd %d, result %d\n",
			((pkt_thermometer_data_req_t *)head)->subcmd, value);
		if (value == 0) {
			if (memcpy_s(sensor_status.selftest_result[TAG_THERMOMETER],
				SELFTEST_RESULT_MAXLEN, "0", 2) != EOK) /* 2: src size */
				hwlog_err("%s: memcpy_s error\n", __func__);
		} else {
			snprintf_s(buf, SELFTEST_RESULT_MAXLEN, SELFTEST_RESULT_MAXLEN - 1, "%d\n", value);
			if (memcpy_s(sensor_status.selftest_result[TAG_THERMOMETER],
				SELFTEST_RESULT_MAXLEN, buf, SELFTEST_RESULT_MAXLEN) != EOK)
				hwlog_err("%s: memcpy_s error\n", __func__);
		}
		break;
	default:
		hwlog_err("uncorrect subcmd 0x%x\n", ((pkt_thermometer_data_req_t *)head)->subcmd);
		break;
	}
	return 0;
}

void therm_detect_init(struct sensor_detect_manager *sm, uint32_t len)
{
	struct sensor_detect_manager *p = NULL;

	if (len <= THERMOMETER) {
		hwlog_err("%s:len=%d\n", __func__, len);
		return;
	}
	g_therm_dev_info[THERM_DEVICE_ID_0].obj_tag = TAG_THERMOMETER;
	g_therm_dev_info[THERM_DEVICE_ID_0].detect_list_id = THERMOMETER;
	g_therm_dev_info[THERM_DEVICE_ID_0].therm_dev_index = 0;
	g_therm_dev_info[THERM_DEVICE_ID_0].therm_upload_data_flag = 0;
	p = sm + THERMOMETER;
	p->spara = (const void *)&therm_data[THERM_DEVICE_ID_0];
	p->cfg_data_length = sizeof(therm_data[THERM_DEVICE_ID_0]);
}

