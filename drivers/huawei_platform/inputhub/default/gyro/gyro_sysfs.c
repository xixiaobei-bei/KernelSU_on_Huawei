/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: gyro sysfs source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "gyro_sysfs.h"

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

#ifdef SENSOR_DATA_ACQUISITION
#define GYRO_CAL_NUM                 15
#define GYRO_THRESHOLD_NUM           3

#define GYRO_CALI_X_OFFSET_MSG       703014001
#define GYRO_CALI_Y_OFFSET_MSG       703014002
#define GYRO_CALI_Z_OFFSET_MSG       703014003
#define GYRO_CALI_X_SEN_MSG          703014004
#define GYRO_CALI_Y_SEN_MSG          703014005
#define GYRO_CALI_Z_SEN_MSG          703014006
#define GYRO_CALI_XIS_00_MSG         703014007
#define GYRO_CALI_XIS_01_MSG         703014008
#define GYRO_CALI_XIS_02_MSG         703014009
#define GYRO_CALI_XIS_10_MSG         703014010
#define GYRO_CALI_XIS_11_MSG         703014011
#define GYRO_CALI_XIS_12_MSG         703014012
#define GYRO_CALI_XIS_20_MSG         703014013
#define GYRO_CALI_XIS_21_MSG         703014014
#define GYRO_CALI_XIS_22_MSG         703014015

static char *gyro_test_cal[GYRO_DEV_COUNT_MAX] = {
	"GYRO", "GYRO1"
};
static char *gyro_test_name[GYRO_DEV_COUNT_MAX][GYRO_CAL_NUM] = {
	{
	"GYRO_CALI_X_OFFSET_MSG", "GYRO_CALI_Y_OFFSET_MSG",
	"GYRO_CALI_Z_OFFSET_MSG",
	"GYRO_CALI_X_SEN_MSG", "GYRO_CALI_Y_SEN_MSG", "GYRO_CALI_Z_SEN_MSG",
	"GYRO_CALI_XIS_00_MSG", "GYRO_CALI_XIS_01_MSG",
	"GYRO_CALI_XIS_02_MSG", "GYRO_CALI_XIS_10_MSG",
	"GYRO_CALI_XIS_11_MSG", "GYRO_CALI_XIS_12_MSG",
	"GYRO_CALI_XIS_20_MSG", "GYRO_CALI_XIS_21_MSG",
	"GYRO_CALI_XIS_22_MSG"
	},
	{
	"GYRO1_CALI_X_OFFSET_MSG", "GYRO1_CALI_Y_OFFSET_MSG",
	"GYRO1_CALI_Z_OFFSET_MSG",
	"GYRO1_CALI_X_SEN_MSG", "GYRO1_CALI_Y_SEN_MSG", "GYRO1_CALI_Z_SEN_MSG",
	"GYRO1_CALI_XIS_00_MSG", "GYRO1_CALI_XIS_01_MSG",
	"GYRO1_CALI_XIS_02_MSG", "GYRO1_CALI_XIS_10_MSG",
	"GYRO1_CALI_XIS_11_MSG", "GYRO1_CALI_XIS_12_MSG",
	"GYRO1_CALI_XIS_20_MSG", "GYRO1_CALI_XIS_21_MSG",
	"GYRO1_CALI_XIS_22_MSG"
	}
};
#endif

#define GYRO_RANGE_1000DPS 1000
#define GYRO_RANGE_FROM_2000DPS_TO_1000DPS 2
#define GYRO_PERIOD_MAX 10

static int32_t set_gyro_calib_data[15];
static int32_t set_gyro1_calib_data[GYRO1_CALIDATA_LEN];
static struct gyro_offset_threshold gyro_calib_threshold[CALIBRATE_DATA_LENGTH] = {
	{ -572, 572 },   /* x-offset */
	{ -572, 572 },   /* y-offset */
	{ -572, 572 },   /* z-offset */
	{ 6500, 13500 }, /* x-sensitivity */
	{ 6500, 13500 }, /* y-sensitivity */
	{ 6500, 13500 }, /* z-sensitivity */
	/* The following is the interaxial interference */
	{ 6500, 13500 },
	{ -2000, 2000 },
	{ -2000, 2000 },
	{ -2000, 2000 },
	{ 6500, 13500 },
	{ -2000, 2000 },
	{ -2000, 2000 },
	{ -2000, 2000 },
	{ 6500, 13500 },
};

#ifdef SENSOR_DATA_ACQUISITION
static void gyro_calibrate_enq_notify_work(struct gyro_platform_data *pf_data,
	struct gyro_device_info *dev_info);
#endif

ssize_t attr_gyro_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gyro_device_info *dev_info = NULL;

	dev_info = gyro_get_device_info(TAG_GYRO);
	if (dev_info == NULL)
		return -1;

	hwlog_info("gyro_calibrate_show is old way,result=%d\n",
		(int32_t)dev_info->gyro_calibration_res);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)dev_info->gyro_calibration_res);
}

static int gyro1_calibrate_save(const void *buf, int length)
{
	struct gyro_device_info *dev_info = NULL;
	const int32_t *poffset_data = (const int32_t *)buf;
	int ret;
	int i;

	dev_info = gyro_get_device_info(TAG_GYRO1);
	if (dev_info == NULL)
		return -1;
	if (!buf || length <= 0) {
		hwlog_err("%s invalid argument", __func__);
		dev_info->gyro_calibration_res = EXEC_FAIL;
		return -1;
	}
	hwlog_info("%s:gyro1 calibrate ok, %d %d %d\n", __func__,
		*poffset_data, *(poffset_data + 1), *(poffset_data + 2));

	for (i = 0; i < GYRO_OFFSET_CALIBRATE_LENGTH; i++) {
		if (*(poffset_data + i) < gyro_calib_threshold[i].low_threshold ||
			*(poffset_data + i) > gyro_calib_threshold[i].high_threshold) {
			hwlog_err("%s: gyro1 calibrated_data is out of range. i = %d, num = %d\n",
				__func__, i, *(poffset_data + i));
			dev_info->gyro_calibration_res = NV_FAIL;
			return -1;
		}
	}
	ret = write_gyro1_sensor_offset_to_nv((char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail\n");
		dev_info->gyro_calibration_res = NV_FAIL;
		return -1;
	}
	dev_info->gyro_calibration_res = SUC;
	return 0;
}

static int gyro_calibrate_save(const void *buf, int length)
{
	struct gyro_platform_data *pf_data = NULL;
	struct gyro_device_info *dev_info = NULL;
	const int32_t *poffset_data = (const int32_t *)buf;
	int ret;
	int gyro_range_factor = 1;
	int i;

	pf_data = gyro_get_platform_data(TAG_GYRO);
	dev_info = gyro_get_device_info(TAG_GYRO);
	if (pf_data == NULL || dev_info == NULL)
		return -1;
	if (!buf || length <= 0) {
		hwlog_err("%s invalid argument", __func__);
		dev_info->gyro_calibration_res = EXEC_FAIL;
		return -1;
	}
	hwlog_info("%s:gyro_sensor calibrate ok, %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		__func__, *poffset_data, *(poffset_data + 1),
		*(poffset_data + 2), *(poffset_data + 3), *(poffset_data + 4),
		*(poffset_data + 5), *(poffset_data + 6), *(poffset_data + 7),
		*(poffset_data + 8), *(poffset_data + 9), *(poffset_data + 10),
		*(poffset_data + 11), *(poffset_data + 12),
		*(poffset_data + 13), *(poffset_data + 14));

	if (pf_data->gyro_range == GYRO_RANGE_1000DPS)
		gyro_range_factor = GYRO_RANGE_FROM_2000DPS_TO_1000DPS;

	for (i = 0; i < GYRO_OFFSET_CALIBRATE_LENGTH; i++) {
		if ((*(poffset_data + i) <
			gyro_calib_threshold[i].low_threshold * gyro_range_factor) ||
			(*(poffset_data + i) >
			gyro_calib_threshold[i].high_threshold * gyro_range_factor)) {
			hwlog_err("%s: gyro calibrated_data is out of range. i = %d, num = %d\n",
				__func__, i, *(poffset_data + i));
			dev_info->gyro_calibration_res = NV_FAIL;
			return -1;
		}
	}

	ret = write_gyro_sensor_offset_to_nv((char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail.\n");
		dev_info->gyro_calibration_res = NV_FAIL;
		return -1;
	}
	dev_info->gyro_calibration_res = SUC;
	return 0;
}

static int gyro1_calibrate_on_write(struct gyro_platform_data *pf_data,
	struct gyro_device_info *dev_info, unsigned long val)
{
	enum ret_type rtype = RET_INIT;
	struct read_info read_pkg;
	const int32_t *gyro_cali_data = NULL;
	int32_t i;

	/* send calibrate command, need set delay first */
	read_pkg = send_calibrate_cmd(TAG_GYRO1, val, &rtype);
	dev_info->gyro_calibration_res = rtype;
	if (dev_info->gyro_calibration_res == COMMU_FAIL) {
		return -1;
	} else if (read_pkg.errno == 0) {
		gyro_cali_data = (const int32_t *)read_pkg.data;
		for (i = 0; i < GYRO_STATIC_CALI_LENGTH; i++)
			dev_info->gyro_calib_data[i] = *(gyro_cali_data + i);
		gyro1_calibrate_save(dev_info->gyro_calib_data,
			sizeof(dev_info->gyro_calib_data));
		hwlog_info("gyro calibrate success, val=%lu data=%d %d %d,len=%d\n",
			val, dev_info->gyro_calib_data[0],
			dev_info->gyro_calib_data[1],
			dev_info->gyro_calib_data[2], read_pkg.data_length);
	}

#ifdef SENSOR_DATA_ACQUISITION
	if (val == 1 || val == GYRO_DYN_CALIBRATE_END_ORDER)
		gyro_calibrate_enq_notify_work(pf_data, dev_info);
#endif

	return 0;
}

static int gyro1_calibrate_on_enter(struct gyro_device_info *dev_info)
{
	int ret;
	interval_param_t param = { 0 };

	/* if gyro1 is not opened, open first */
	if (sensor_status.opened[TAG_GYRO1] == 0) {
		dev_info->gyro_opened_before_calibrate = 0;
		hwlog_info("send gyro1 open cmd(during calibrate) to mcu.\n");
		ret = inputhub_sensor_enable(TAG_GYRO1, true);
		if (ret) {
			dev_info->gyro_calibration_res = COMMU_FAIL;
			hwlog_err("send gyro open cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	} else {
		dev_info->gyro_opened_before_calibrate = 1;
	}

	if ((sensor_status.delay[TAG_GYRO1] == 0) ||
		(sensor_status.delay[TAG_GYRO1] > GYRO_PERIOD_MAX)) {
		hwlog_info("send gyro1 setdelay cmd(during calibrate) to mcu\n");
		param.period = GYRO_PERIOD_MAX;
		ret = inputhub_sensor_setdelay(TAG_GYRO1, &param);
		if (ret) {
			dev_info->gyro_calibration_res = COMMU_FAIL;
			hwlog_err("send gyro1 set delay cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	}

	return 0;
}

static void gyro1_calibrate_on_exit(struct gyro_device_info *dev_info)
{
	int ret;

	if (dev_info->gyro_opened_before_calibrate == 0) {
		dev_info->gyro_opened_before_calibrate = 1;
		hwlog_info("send gyro close cmd(during calibrate) to mcu.\n");
		ret = inputhub_sensor_enable(TAG_GYRO1, false);
		if (ret) {
			dev_info->gyro_calibration_res = COMMU_FAIL;
			hwlog_err("send gyro close cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return;
		}
	}
}

ssize_t attr_gyro1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gyro_platform_data *pf_data = NULL;
	struct gyro_device_info *dev_info = NULL;
	unsigned long val = 0;

	pf_data = gyro_get_platform_data(TAG_GYRO1);
	dev_info = gyro_get_device_info(TAG_GYRO1);
	if (pf_data == NULL || dev_info == NULL)
		return -1;
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (val != 1) {
		hwlog_err("set gyro1 calibrate val invalid,val=%lu\n", val);
		return count;
	}

	if (gyro1_calibrate_on_enter(dev_info))
		return count;

	msleep(300); /* sleep 300 ms */

	if (gyro1_calibrate_on_write(pf_data, dev_info, val))
		return count;
	gyro1_calibrate_on_exit(dev_info);
	return count;
}

#ifdef SENSOR_DATA_ACQUISITION
static void gyro_calibrate_enq_notify_work(struct gyro_platform_data *pf_data,
	struct gyro_device_info *dev_info)
{
	int32_t min_threshold_gyro[GYRO_THRESHOLD_NUM] = {
		-pf_data->calibrate_thredhold,
		-pf_data->calibrate_thredhold,
		-pf_data->calibrate_thredhold
	}; /* -40dps */
	int32_t max_threshold_gyro[GYRO_THRESHOLD_NUM] = {
		pf_data->calibrate_thredhold,
		pf_data->calibrate_thredhold,
		pf_data->calibrate_thredhold
	}; /* 40dps */
	struct sensor_eng_cal_test gyro_test = { 0 };
	int32_t i;

	if (dev_info->gyro_dev_index >= GYRO_DEV_COUNT_MAX)
		return;
	hwlog_info("%s:gyro_dev_index=%d\n", __func__,
		dev_info->gyro_dev_index);

	gyro_test.cal_value = dev_info->gyro_calib_data;
	gyro_test.first_item = GYRO_CALI_X_OFFSET_MSG;
	gyro_test.value_num = GYRO_CAL_NUM;
	gyro_test.threshold_num = GYRO_THRESHOLD_NUM;
	gyro_test.min_threshold = min_threshold_gyro;
	gyro_test.max_threshold = max_threshold_gyro;
	if (memcpy_s(gyro_test.name, sizeof(gyro_test.name),
		gyro_test_cal[dev_info->gyro_dev_index],
		strlen(gyro_test_cal[dev_info->gyro_dev_index]) + 1) != EOK)
		return;
	if (memcpy_s(gyro_test.result, sizeof(gyro_test.result),
		sensor_cal_result(dev_info->gyro_calibration_res),
		(strlen(sensor_cal_result(dev_info->gyro_calibration_res)) +
		1)) != EOK)
		return;
	for (i = 0; i < GYRO_CAL_NUM; i++)
		gyro_test.test_name[i] =
			gyro_test_name[dev_info->gyro_dev_index][i];

	enq_notify_work_sensor(gyro_test);
}
#endif

static int gyro_calibrate_on_write(struct gyro_platform_data *pf_data,
	struct gyro_device_info *dev_info, unsigned long val)
{
	enum ret_type rtype = RET_INIT;
	struct read_info read_pkg;
	int32_t *gyro_cali_data = NULL;
	int32_t i;

	/* send calibrate command, need set delay first */
	read_pkg = send_calibrate_cmd(TAG_GYRO, val, &rtype);
	dev_info->gyro_calibration_res = rtype;
	if (dev_info->gyro_calibration_res == COMMU_FAIL) {
		return -1;
	} else if (read_pkg.errno == 0) {
		gyro_cali_data = (int32_t *)read_pkg.data;
		if (val == 1) {
			for (i = 0; i < GYRO_STATIC_CALI_LENGTH; i++)
				dev_info->gyro_calib_data[i] =
					*(gyro_cali_data + i);
			gyro_calibrate_save(dev_info->gyro_calib_data,
				sizeof(dev_info->gyro_calib_data));
		} else if (val == GYRO_DYN_CALIBRATE_END_ORDER) {
			i = GYRO_STATIC_CALI_LENGTH;
			for (; i < CALIBRATE_DATA_LENGTH; i++)
				dev_info->gyro_calib_data[i] =
					*(gyro_cali_data + i);
			gyro_calibrate_save(dev_info->gyro_calib_data,
				sizeof(dev_info->gyro_calib_data));
		} else {
			dev_info->gyro_calibration_res = SUC;
		}
		hwlog_info("gyro calibrate success, val=%lu data=%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d, len=%d\n",
			val, dev_info->gyro_calib_data[0],
			dev_info->gyro_calib_data[1],
			dev_info->gyro_calib_data[2],
			dev_info->gyro_calib_data[3],
			dev_info->gyro_calib_data[4],
			dev_info->gyro_calib_data[5],
			dev_info->gyro_calib_data[6],
			dev_info->gyro_calib_data[7],
			dev_info->gyro_calib_data[8],
			dev_info->gyro_calib_data[9],
			dev_info->gyro_calib_data[10],
			dev_info->gyro_calib_data[11],
			dev_info->gyro_calib_data[12],
			dev_info->gyro_calib_data[13],
			dev_info->gyro_calib_data[14], read_pkg.data_length);
	}

#ifdef SENSOR_DATA_ACQUISITION
	if (val == 1 || val == GYRO_DYN_CALIBRATE_END_ORDER)
		gyro_calibrate_enq_notify_work(pf_data, dev_info);
#endif

	return 0;
}

static int gyro_calibrate_on_enter(struct gyro_device_info *dev_info)
{
	int ret;
	interval_param_t param = { 0 };

	/* if gyro is not opened, open first */
	if (sensor_status.opened[TAG_GYRO] == 0) {
		dev_info->gyro_opened_before_calibrate = 0;
		hwlog_info("send gyro open cmd(during calibrate) to mcu\n");
		ret = inputhub_sensor_enable(TAG_GYRO, true);
		if (ret) {
			dev_info->gyro_calibration_res = COMMU_FAIL;
			hwlog_err("send gyro open cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	} else {
		dev_info->gyro_opened_before_calibrate = 1;
	}

	if ((sensor_status.delay[TAG_GYRO] == 0) ||
		(sensor_status.delay[TAG_GYRO] > GYRO_PERIOD_MAX)) {
		hwlog_info("send gyro setdelay cmd(during calibrate) to mcu\n");
		param.period = GYRO_PERIOD_MAX;
		ret = inputhub_sensor_setdelay(TAG_GYRO, &param);
		if (ret) {
			dev_info->gyro_calibration_res = COMMU_FAIL;
			hwlog_err("send gyro set delay cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	}

	return 0;
}

static void gyro_calibrate_on_exit(struct gyro_device_info *dev_info)
{
	int ret;

	if (dev_info->gyro_opened_before_calibrate == 0) {
		dev_info->gyro_opened_before_calibrate = 1;
		hwlog_info("send gyro close cmd(during calibrate) to mcu\n");
		ret = inputhub_sensor_enable(TAG_GYRO, false);
		if (ret) {
			dev_info->gyro_calibration_res = COMMU_FAIL;
			hwlog_err("send gyro close cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return;
		}
	}
}

ssize_t attr_gyro_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct gyro_platform_data *pf_data = NULL;
	struct gyro_device_info *dev_info = NULL;
	unsigned long val = 0;

	pf_data = gyro_get_platform_data(TAG_GYRO);
	dev_info = gyro_get_device_info(TAG_GYRO);
	if (pf_data == NULL || dev_info == NULL)
		return -1;
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if ((val < 1) || (val > 7)) { /* gyro calibrate val max 7 */
		hwlog_err("set gyro calibrate val invalid,val=%lu\n", val);
		return count;
	}

	if (gyro_calibrate_on_enter(dev_info))
		return count;

	msleep(300);
	if (gyro_calibrate_on_write(pf_data, dev_info, val))
		return count;
	gyro_calibrate_on_exit(dev_info);
	return count;
}

ssize_t attr_get_gyro_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *id_string = NULL;

	id_string = gyro_get_sensors_id_string();
	if (id_string == NULL)
		return -1;

	hwlog_info("%s is %s\n", __func__, id_string);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
		id_string);
}

ssize_t show_gyro_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gyro_platform_data *pf_data = NULL;

	hwlog_info("%s:inter show gyro set calidata node\n", __func__);
	pf_data = gyro_get_platform_data(TAG_GYRO);
	if (pf_data == NULL)
		return -1;
	if (pf_data->calibrate_way == 0)
		return 0;

	if (strlen(get_sensor_chip_info_address(GYRO)) != 0)
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
			"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			set_gyro_calib_data[0], set_gyro_calib_data[1],
			set_gyro_calib_data[2], set_gyro_calib_data[3],
			set_gyro_calib_data[4], set_gyro_calib_data[5],
			set_gyro_calib_data[6], set_gyro_calib_data[7],
			set_gyro_calib_data[8], set_gyro_calib_data[9],
			set_gyro_calib_data[10], set_gyro_calib_data[11],
			set_gyro_calib_data[12], set_gyro_calib_data[13],
			set_gyro_calib_data[14]);
	else
		return -1;
}

ssize_t store_gyro_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct gyro_platform_data *pf_data = NULL;
	int set_gyro_sensor_offset[GYRO_CALIBRATE_DATA_LENGTH] = { 0 };
	int i;

	hwlog_info("%s:inter gyro set calidata node\n", __func__);
	pf_data = gyro_get_platform_data(TAG_GYRO);
	if (pf_data == NULL)
		return -1;
	if (pf_data->calibrate_way == 0) {
		hwlog_info("%s: now is gyro self_calibreate\n", __func__);
		return size;
	}
	if (strlen(get_sensor_chip_info_address(GYRO)) == 0) {
		hwlog_err("GYRO not exits !!\n");
		return -1;
	}
	if (size != sizeof(set_gyro_calib_data)) {
		hwlog_err("%s:size %lu is not equal 15*4\n", __func__, size);
		return -1;
	}
	if (memcpy_s(set_gyro_calib_data, sizeof(set_gyro_calib_data),
		buf, size) != EOK)
		return -1;
	for (i = 0; i < CALIBRATE_DATA_LENGTH; i++) {
		if (set_gyro_calib_data[i] < gyro_calib_threshold[i].low_threshold ||
		    set_gyro_calib_data[i] > gyro_calib_threshold[i].high_threshold) {
			hwlog_err("%s: gyro calibrated_data is out of range. i = %d, num = %d\n",
				__func__, i, set_gyro_calib_data[i]);
			return -1;
		}
	}
	gyro_calibrate_save(set_gyro_calib_data, sizeof(set_gyro_calib_data));
	if (read_calibrate_data_from_nv(GYRO_CALIDATA_NV_NUM,
		GYRO_CALIDATA_NV_SIZE, "GYRO"))
		return -1;

	/* copy to gyro_offset by pass */
	if (memcpy_s(set_gyro_sensor_offset, sizeof(set_gyro_sensor_offset),
		user_info.nv_data, sizeof(set_gyro_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro_sensor offset: %d %d %d  sensitity:%d %d %d\n",
		set_gyro_sensor_offset[0], set_gyro_sensor_offset[1],
		set_gyro_sensor_offset[2], set_gyro_sensor_offset[3],
		set_gyro_sensor_offset[4], set_gyro_sensor_offset[5]);
	hwlog_info("nve_direct_access read gyro_sensor xis_angle: %d %d %d  %d %d %d %d %d %d\n",
		set_gyro_sensor_offset[6], set_gyro_sensor_offset[7],
		set_gyro_sensor_offset[8], set_gyro_sensor_offset[9],
		set_gyro_sensor_offset[10], set_gyro_sensor_offset[11],
		set_gyro_sensor_offset[12], set_gyro_sensor_offset[13],
		set_gyro_sensor_offset[14]);
	if (send_calibrate_data_to_mcu(TAG_GYRO, SUB_CMD_SET_OFFSET_REQ,
		set_gyro_sensor_offset, GYRO_CALIDATA_NV_SIZE, false))
		return -1;
	return size;
}

ssize_t sensors_calibrate_show_gyro(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gyro_device_info *dev_info = NULL;

	dev_info = gyro_get_device_info(tag);
	if (dev_info == NULL)
		return -1;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)(dev_info->gyro_calibration_res != SUC));
}

ssize_t show_gyro1_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct gyro_platform_data *pf_data = NULL;

	hwlog_info("%s: inter show gyro1 set calidata node\n", __func__);
	pf_data = gyro_get_platform_data(TAG_GYRO1);
	if (pf_data == NULL)
		return -1;
	if (pf_data->calibrate_way == 0)
		return 0;

	if (strlen(get_sensor_chip_info_address(GYRO1)) != 0)
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
			"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			/* offset: data[0]-data[2] sensitivity: data[3]-data[5] */
			set_gyro1_calib_data[0], set_gyro1_calib_data[1],
			set_gyro1_calib_data[2], set_gyro1_calib_data[3],
			set_gyro1_calib_data[4], set_gyro1_calib_data[5],
			/* six_angle: data[6]-data[14] */
			set_gyro1_calib_data[6], set_gyro1_calib_data[7],
			set_gyro1_calib_data[8], set_gyro1_calib_data[9],
			set_gyro1_calib_data[10], set_gyro1_calib_data[11],
			set_gyro1_calib_data[12], set_gyro1_calib_data[13],
			set_gyro1_calib_data[14]);
	else
		return -1;
}

ssize_t store_gyro1_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct gyro_platform_data *pf_data = NULL;
	int set_gyro1_sensor_offset[GYRO1_CALIBRATE_DATA_LENGTH] = { 0 };
	int i;

	hwlog_info("%s: inter gyro1 set calidata node\n", __func__);
	pf_data = gyro_get_platform_data(TAG_GYRO1);
	if (pf_data == NULL)
		return -1;
	if (pf_data->calibrate_way == 0) {
		hwlog_info("%s: now is gyro1 self_calibreate\n", __func__);
		return size;
	}
	if (strlen(get_sensor_chip_info_address(GYRO1)) == 0) {
		hwlog_err("GYRO1 not exits !!\n");
		return -1;
	}
	if (size != sizeof(set_gyro1_calib_data)) {
		hwlog_err("%s:size %lu is not equal 15*4\n", __func__, size);
		return -1;
	}
	if (memcpy_s(set_gyro1_calib_data, sizeof(set_gyro1_calib_data),
		buf, size) != EOK)
		return -1;
	for (i = 0; i < CALIBRATE_DATA_LENGTH; i++) {
		if (set_gyro1_calib_data[i] < gyro_calib_threshold[i].low_threshold ||
		    set_gyro1_calib_data[i] > gyro_calib_threshold[i].high_threshold) {
			hwlog_err("%s: gyro calibrated_data is out of range. i = %d, num = %d\n",
				__func__, i, set_gyro1_calib_data[i]);
			return -1;
		}
	}
	gyro1_calibrate_save(set_gyro1_calib_data, sizeof(set_gyro1_calib_data));
	if (read_calibrate_data_from_nv(GYRO1_OFFSET_NV_NUM,
		GYRO1_OFFSET_NV_SIZE, GYRO1_NV_NAME))
		return -1;

	/* copy to gyro_offset by pass */
	if (memcpy_s(set_gyro1_sensor_offset, sizeof(set_gyro1_sensor_offset),
		user_info.nv_data, sizeof(set_gyro1_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read gyro1_sensor offset: %d %d %d  sensitity:%d %d %d\n",
		set_gyro1_sensor_offset[0], set_gyro1_sensor_offset[1],
		set_gyro1_sensor_offset[2], set_gyro1_sensor_offset[3],
		set_gyro1_sensor_offset[4], set_gyro1_sensor_offset[5]);
	hwlog_info("nve_direct_access read gyro1_sensor xis_angle: %d %d %d  %d %d %d %d %d %d\n",
		set_gyro1_sensor_offset[6], set_gyro1_sensor_offset[7],
		set_gyro1_sensor_offset[8], set_gyro1_sensor_offset[9],
		set_gyro1_sensor_offset[10], set_gyro1_sensor_offset[11],
		set_gyro1_sensor_offset[12], set_gyro1_sensor_offset[13],
		set_gyro1_sensor_offset[14]);
	if (send_calibrate_data_to_mcu(TAG_GYRO1, SUB_CMD_SET_OFFSET_REQ,
		set_gyro1_sensor_offset, sizeof(set_gyro1_sensor_offset), false))
		return -1;
	return size;
}
