/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: airpress sysfs source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "airpress_sysfs.h"

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>

#include "contexthub_route.h"

#define MAX_AIRPRESS_OFFSET 500
#define MIN_AIRPRESS_OFFSET (-500)

#ifdef SENSOR_DATA_ACQUISITION
#define AIRPRESS_CAL_NUM             17
#define AIR_THRESHOLD_NUM            1
#define AIRPRESS_TEST_CAL           "AIRPRESS"
#define airpress_result(x) (((x) >= MIN_AIRPRESS_OFFSET && \
	(x) <= MAX_AIRPRESS_OFFSET) ? "pass" : "fail")

#define  AIRPRESS_CALI_OFFSET_MSG         703017001
#define  AIRPRESS_TOUCH_CALI_SLOPE_MSG    703017002
#define  AIRPRESS_TOUCH_CALI_BASEP_MSG    703017003
#define  AIRPRESS_TOUCH_CALI_MAXP_MSG     703017004
#define  AIRPRESS_TOUCH_CALI_RAISEP_MSG   703017005
#define  AIRPRESS_TOUCH_CALI_MINP_MSG     703017006
#define  AIRPRESS_TOUCH_CALI_TEMP_MSG     703017007
#define  AIRPRESS_TOUCH_CALI_SPEED_MSG    703017008
#define  AIRPRESS_TOUCH_CALI_RFLAG_MSG    703017009
#define  AIRPRESS_TOUCH_TEST_SLOPE_MSG    703017010
#define  AIRPRESS_TOUCH_TEST_BASEP_MSG    703017011
#define  AIRPRESS_TOUCH_TEST_MAXP_MSG     703017012
#define  AIRPRESS_TOUCH_TEST_RAISEP_MSG   703017013
#define  AIRPRESS_TOUCH_TEST_MINP_MSG     703017014
#define  AIRPRESS_TOUCH_TEST_TEMP_MSG     703017015
#define  AIRPRESS_TOUCH_TEST_SPEED_MSG    703017016
#define  AIRPRESS_TOUCH_TEST_RFLAG_MSG    703017017
#endif

#ifdef SENSOR_DATA_ACQUISITION
static char *airpress_test_name[AIRPRESS_CAL_NUM] = {
	"AIRPRESS_CALI_OFFSET_MSG", "AIRPRESS_TOUCH_CALI_SLOPE_MSG",
	"AIRPRESS_TOUCH_CALI_BASEP_MSG", "AIRPRESS_TOUCH_CALI_MAXP_MSG",
	"AIRPRESS_TOUCH_CALI_RAISEP_MSG", "AIRPRESS_TOUCH_CALI_MINP_MSG",
	"AIRPRESS_TOUCH_CALI_TEMP_MSG", "AIRPRESS_TOUCH_CALI_SPEED_MSG",
	"AIRPRESS_TOUCH_CALI_RFLAG_MSG", "AIRPRESS_TOUCH_TEST_SLOPE_MSG",
	"AIRPRESS_TOUCH_TEST_BASEP_MSG", "AIRPRESS_TOUCH_TEST_MAXP_MSG",
	"AIRPRESS_TOUCH_TEST_RAISEP_MSG", "AIRPRESS_TOUCH_TEST_MINP_MSG",
	"AIRPRESS_TOUCH_TEST_TEMP_MSG", "AIRPRESS_TOUCH_TEST_SPEED_MSG",
	"AIRPRESS_TOUCH_TEST_RFLAG_MSG"
};
#endif

static int airpress_cali_flag;

struct read_info send_airpress_calibrate_cmd(uint8_t tag, unsigned long val,
	enum ret_type *rtype)
{
	return send_calibrate_cmd(tag, val, rtype);
}

ssize_t show_sensor_read_airpress_common(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	airpress_cali_flag = 1;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		get_airpress_data_value());
}

ssize_t show_sensor_read_airpress(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return show_sensor_read_airpress_common(dev, attr, buf);
}

ssize_t show_airpress_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct airpress_platform_data *pf_data = NULL;

	pf_data = airpress_get_platform_data(TAG_PRESSURE);
	if (pf_data == NULL)
		return -1;
	if (strlen(get_sensor_chip_info_address(AIRPRESS)) != 0)
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
			pf_data->offset);

	return -1;
}

static void airpress_extend_data_init(void)
{
	struct airpress_platform_data *pf_data = NULL;
	int i;

	pf_data = airpress_get_platform_data(TAG_PRESSURE);
	if (pf_data == NULL)
		return;

	/* airpress extend data length is 48 */
	for (i = 0; i < 48; i++)
		pf_data->airpress_extend_data[i] = i;
}

ssize_t store_airpress_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct airpress_platform_data *pf_data = NULL;
	long source;
	int ret;
	int temp;

	pf_data = airpress_get_platform_data(TAG_PRESSURE);
	if (pf_data == NULL)
		return -1;
	if (strlen(get_sensor_chip_info_address(AIRPRESS)) == 0) {
		hwlog_err("AIRPRESS not exits !!\n");
		return -1;
	}
	if (!airpress_cali_flag) {
		hwlog_warn("Takes effect only when the calibration data\n");
		return -1;
	}
	source = simple_strtol(buf, NULL, TO_DECIMALISM);
	if (source > MAX_AIRPRESS_OFFSET || source < MIN_AIRPRESS_OFFSET) {
		hwlog_err("Incorrect offset. source = %ld\n", source);
		return -1;
	}
	pf_data->offset += (int)source;

	airpress_extend_data_init();
	/* send to mcu */
	hwlog_info("%s\n", __func__);
	if (send_calibrate_data_to_mcu(TAG_PRESSURE, SUB_CMD_SET_OFFSET_REQ,
		&pf_data->offset, sizeof(pf_data->offset), false))
		return -1;
	hwlog_info("%s:%d to mcu success\n", __func__, pf_data->offset);
	if (write_calibrate_data_to_nv(AIRPRESS_CALIDATA_NV_NUM,
		sizeof(pf_data->offset), "AIRDATA",
		(char *)&pf_data->offset)) {
		hwlog_err("nve_direct_access write error %d\n", ret);
		return -1;
	}

	msleep(10); /* sleep 10 ms */
	if (read_calibrate_data_from_nv(AIRPRESS_CALIDATA_NV_NUM,
		AIRPRESS_CALIDATA_NV_SIZE, "AIRDATA")) {
		hwlog_err("nve direct access read error %d\n", ret);
		return -1;
	}
	if (memcpy_s(&temp, sizeof(temp),
		user_info.nv_data, sizeof(temp)) != EOK)
		return -1;
	if (temp != pf_data->offset) {
		hwlog_err("nv write fail, %d %d\n", temp, pf_data->offset);
		return -1;
	}
	airpress_cali_flag = 0;
	return size;
}

#ifdef SENSOR_DATA_ACQUISITION
void airpress_touch_data_acquisition(struct airpress_touch_calibrate_data *cd)
{
	struct airpress_platform_data *pf_data = NULL;
	int airpress_touch_data[AIRPRESS_CAL_NUM] = { 0 };
	int min_threshold_airpress[AIR_THRESHOLD_NUM] = { MIN_AIRPRESS_OFFSET };
	int max_threshold_airpress[AIR_THRESHOLD_NUM] = { MAX_AIRPRESS_OFFSET };
	struct sensor_eng_cal_test air_test = { 0 };
	int i;

	if (cd == NULL)
		return;
	pf_data = airpress_get_platform_data(TAG_PRESSURE);
	if (pf_data == NULL)
		return;

	airpress_touch_data[0] = pf_data->offset;
	airpress_touch_data[1] = cd->c_result.slope;
	airpress_touch_data[2] = cd->c_result.base_press;
	airpress_touch_data[3] = cd->c_result.max_press;
	airpress_touch_data[4] = cd->c_result.raise_press;
	airpress_touch_data[5] = cd->c_result.min_press;
	airpress_touch_data[6] = cd->c_result.temp;
	airpress_touch_data[7] = cd->c_result.speed;
	airpress_touch_data[8] = cd->c_result.result_flag;
	airpress_touch_data[9] = cd->t_result.slope;
	airpress_touch_data[10] = cd->t_result.base_press;
	airpress_touch_data[11] = cd->t_result.max_press;
	airpress_touch_data[12] = cd->t_result.raise_press;
	airpress_touch_data[13] = cd->t_result.min_press;
	airpress_touch_data[14] = cd->t_result.temp;
	airpress_touch_data[15] = cd->t_result.speed;
	airpress_touch_data[16] = cd->t_result.result_flag;

	air_test.cal_value = airpress_touch_data;
	air_test.value_num = AIRPRESS_CAL_NUM;
	air_test.threshold_num = AIR_THRESHOLD_NUM;
	air_test.first_item = AIRPRESS_CALI_OFFSET_MSG;
	air_test.min_threshold = min_threshold_airpress;
	air_test.max_threshold = max_threshold_airpress;
	if (memcpy_s(air_test.name, sizeof(air_test.name),
		AIRPRESS_TEST_CAL, sizeof(AIRPRESS_TEST_CAL)) != EOK)
		return;
	if (memcpy_s(air_test.result, sizeof(air_test.result),
		airpress_result(pf_data->offset),
		strlen(airpress_result(pf_data->offset)) + 1) != EOK)
		return;
	for (i = 0; i < AIRPRESS_CAL_NUM; i++)
		air_test.test_name[i] = airpress_test_name[i];
	enq_notify_work_sensor(air_test);
}
#endif

