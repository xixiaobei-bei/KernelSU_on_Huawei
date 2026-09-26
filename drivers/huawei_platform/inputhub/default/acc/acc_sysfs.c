/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: acc sysfs source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "acc_sysfs.h"

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

#define ACC_PERIOD_MAX 20

#ifdef SENSOR_DATA_ACQUISITION
#define ACC_CAL_NUM                  15
#define ACC_THRESHOLD_NUM            3

#define ACC_CALI_X_OFFSET_MSG        703015001
#define ACC_CALI_Y_OFFSET_MSG        703015002
#define ACC_CALI_Z_OFFSET_MSG        703015003
#define ACC_CALI_X_SEN_MSG           703015004
#define ACC_CALI_Y_SEN_MSG           703015005
#define ACC_CALI_Z_SEN_MSG           703015006
#define ACC_CALI_XIS_00_MSG          703015007
#define ACC_CALI_XIS_01_MSG          703015008
#define ACC_CALI_XIS_02_MSG          703015009
#define ACC_CALI_XIS_10_MSG          703015010
#define ACC_CALI_XIS_11_MSG          703015011
#define ACC_CALI_XIS_12_MSG          703015012
#define ACC_CALI_XIS_20_MSG          703015013
#define ACC_CALI_XIS_21_MSG          703015014
#define ACC_CALI_XIS_22_MSG          703015015

static char *acc_test_cal[ACC_DEV_COUNT_MAX] = {
	"ACC", "ACC1"
};
static char *acc_test_name[ACC_DEV_COUNT_MAX][ACC_CAL_NUM] = {
	{
	"ACC_CALI_X_OFFSET_MSG", "ACC_CALI_Y_OFFSET_MSG",
	"ACC_CALI_Z_OFFSET_MSG",
	"ACC_CALI_X_SEN_MSG", "ACC_CALI_Y_SEN_MSG", "ACC_CALI_Z_SEN_MSG",
	"ACC_CALI_XIS_00_MSG", "ACC_CALI_XIS_01_MSG", "ACC_CALI_XIS_02_MSG",
	"ACC_CALI_XIS_10_MSG", "ACC_CALI_XIS_11_MSG", "ACC_CALI_XIS_12_MSG",
	"ACC_CALI_XIS_20_MSG", "ACC_CALI_XIS_21_MSG", "ACC_CALI_XIS_22_MSG"
	},
	{
	"ACC1_CALI_X_OFFSET_MSG", "ACC1_CALI_Y_OFFSET_MSG",
	"ACC1_CALI_Z_OFFSET_MSG",
	"ACC1_CALI_X_SEN_MSG", "ACC1_CALI_Y_SEN_MSG", "ACC1_CALI_Z_SEN_MSG",
	"ACC1_CALI_XIS_00_MSG", "ACC1_CALI_XIS_01_MSG", "ACC1_CALI_XIS_02_MSG",
	"ACC1_CALI_XIS_10_MSG", "ACC1_CALI_XIS_11_MSG", "ACC1_CALI_XIS_12_MSG",
	"ACC1_CALI_XIS_20_MSG", "ACC1_CALI_XIS_21_MSG", "ACC1_CALI_XIS_22_MSG"
	}
};
#endif

static int32_t set_acc_calib_data[15];
static int32_t set_acc1_calib_data[ACC1_CALIDATA_LEN];

struct acc_offset_threshold acc_calib_threshold[CALIBRATE_DATA_LENGTH] = {
	{ -320, 320 },   /* x-offset unit:mg */
	{ -320, 320 },   /* y-offset unit:mg */
	{ -320, 320 },   /* z-offset unit:mg */
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
static void acc_calibrate_enq_notify_work(
	struct g_sensor_platform_data *pf_data,
	struct acc_device_info *dev_info, int32_t *acc_cali_data);
#endif

ssize_t attr_acc_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct acc_device_info *dev_info = NULL;

	dev_info = acc_get_device_info(TAG_ACCEL);
	if (dev_info == NULL)
		return -1;

	hwlog_info("acc_calibrate_show is old way,result=%d\n",
		(int32_t)dev_info->acc_calibration_res);
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)dev_info->acc_calibration_res);
}

int acc1_calibrate_save(const void *buf, int length)
{
	struct acc_device_info *dev_info = NULL;
	const int32_t *poffset_data = (const int32_t *)buf;
	int ret;
	int i;

	dev_info = acc_get_device_info(TAG_ACC1);
	if (dev_info == NULL)
		return -1;
	if (!buf || length <= 0) {
		hwlog_err("%s invalid argument", __func__);
		dev_info->acc_calibration_res = EXEC_FAIL;
		return -1;
	}
	hwlog_info("%s:acc1 calibrate ok, %d  %d  %d\n",
		__func__, *poffset_data, *(poffset_data + 1), *(poffset_data + 2));

	for (i = 0; i < ACC_OFFSET_CALIBRATE_LENGTH; i++) {
		if (*(poffset_data + i) < acc_calib_threshold[i].low_threshold ||
			*(poffset_data + i) > acc_calib_threshold[i].high_threshold) {
			hwlog_err("%s: acc1 calibrated_data is out of range. i = %d, num = %d\n",
				__FUNCTION__, i, *(poffset_data + i));
			dev_info->acc_calibration_res = NV_FAIL;
			return -1;
		}
	}

	ret = write_gsensor1_offset_to_nv((char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail\n");
		dev_info->acc_calibration_res = NV_FAIL;
		return -1;
	}
	dev_info->acc_calibration_res = SUC;
	return 0;
}

int acc_calibrate_save(const void *buf, int length)
{
	struct acc_device_info *dev_info = NULL;
	const int32_t *poffset_data = (const int32_t *)buf;
	int ret;
	int i;

	dev_info = acc_get_device_info(TAG_ACCEL);
	if (dev_info == NULL)
		return -1;
	if (!buf || length <= 0) {
		hwlog_err("%s invalid argument", __func__);
		dev_info->acc_calibration_res = EXEC_FAIL;
		return -1;
	}
	hwlog_info("%s:gsensor calibrate ok, %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
		__func__, *poffset_data, *(poffset_data + 1),
		*(poffset_data + 2), *(poffset_data + 3), *(poffset_data + 4),
		*(poffset_data + 5), *(poffset_data + 6), *(poffset_data + 7),
		*(poffset_data + 8), *(poffset_data + 9), *(poffset_data + 10),
		*(poffset_data + 11), *(poffset_data + 12),
		*(poffset_data + 13), *(poffset_data + 14));

	for (i = 0; i < ACC_OFFSET_CALIBRATE_LENGTH; i++) {
		if (*(poffset_data + i) < acc_calib_threshold[i].low_threshold ||
		    *(poffset_data + i) > acc_calib_threshold[i].high_threshold) {
			hwlog_err("%s: acc calibrated_data is out of range. i = %d, num = %d\n",
				__func__, i, *(poffset_data + i));
			dev_info->acc_calibration_res = NV_FAIL;
			return -1;
		}
	}
	ret = write_gsensor_offset_to_nv((char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail\n");
		dev_info->acc_calibration_res = NV_FAIL;
		return -1;
	}
	dev_info->acc_calibration_res = SUC;
	return 0;
}

static int acc1_calibrate_on_enter(struct acc_device_info *dev_info)
{
	int ret;
	interval_param_t param = { 0 };

	/* if assistant_acc is not opened, open first */
	if (sensor_status.opened[TAG_ACC1] == 0) {
		dev_info->acc_opened_before_calibrate = 0;
		hwlog_info("send acc1 open cmd(during calibrate) to mcu.\n");
		ret = inputhub_sensor_enable(TAG_ACC1, true);
		if (ret) {
			dev_info->acc_calibration_res = COMMU_FAIL;
			hwlog_err("send acc1 open cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	} else {
		dev_info->acc_opened_before_calibrate = 1;
	}
	/* period must <= 100 ms */
	if ((sensor_status.delay[TAG_ACC1] == 0) ||
	    (sensor_status.delay[TAG_ACCEL] > ACC_PERIOD_MAX)) {
		hwlog_info("send assistant_acc setdelay cmd(during calibrate) to mcu\n");
		param.period = ACC_PERIOD_MAX;
		ret = inputhub_sensor_setdelay(TAG_ACC1, &param);
		if (ret) {
			dev_info->acc_calibration_res = COMMU_FAIL;
			hwlog_err("send assistant_acc set delay cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	}

	return 0;
}

static void acc1_calibrate_on_exit(struct acc_device_info *dev_info)
{
	int ret;

	if (dev_info->acc_opened_before_calibrate == 0) {
		dev_info->acc_opened_before_calibrate = 1;
		hwlog_info("send acc1 close cmd(during calibrate) to mcu\n");
		ret = inputhub_sensor_enable(TAG_ACC1, false);
		if (ret) {
			dev_info->acc_calibration_res = COMMU_FAIL;
			hwlog_err("send acc1 close cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return;
		}
	}
}

ssize_t attr_acc1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct g_sensor_platform_data *pf_data = NULL;
	struct acc_device_info *dev_info = NULL;
	enum ret_type rtype = RET_INIT;
	unsigned long val = 0;
	struct read_info read_pkg;

	pf_data = acc_get_platform_data(TAG_ACC1);
	dev_info = acc_get_device_info(TAG_ACC1);
	if (pf_data == NULL || dev_info == NULL)
		return -1;
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (val != 1) {
		hwlog_err("send acc1 calibrate value is error value=%lu\n", val);
		return count;
	}

	if (acc1_calibrate_on_enter(dev_info))
		return count;

	msleep(300); /* sleep 300 ms */
	/* send calibrate command, need set delay first */
	read_pkg = send_calibrate_cmd(TAG_ACC1, val, &rtype);
	dev_info->acc_calibration_res = rtype;
	if (dev_info->acc_calibration_res == COMMU_FAIL)
		return count;
	else if (read_pkg.errno == 0)
		acc1_calibrate_save(read_pkg.data, read_pkg.data_length);

#ifdef SENSOR_DATA_ACQUISITION
	acc_calibrate_enq_notify_work(pf_data, dev_info,
		(int32_t *)read_pkg.data);
#endif

	acc1_calibrate_on_exit(dev_info);
	return count;
}

#ifdef SENSOR_DATA_ACQUISITION
static void acc_calibrate_enq_notify_work(
	struct g_sensor_platform_data *pf_data,
	struct acc_device_info *dev_info, int32_t *acc_cali_data)
{
	int32_t min_threshold_acc[ACC_THRESHOLD_NUM] = {
		-pf_data->x_calibrate_thredhold,
		-pf_data->y_calibrate_thredhold,
		-pf_data->z_calibrate_thredhold
	};
	int32_t max_threshold_acc[ACC_THRESHOLD_NUM] = {
		pf_data->x_calibrate_thredhold,
		pf_data->y_calibrate_thredhold,
		pf_data->z_calibrate_thredhold
	};
	struct sensor_eng_cal_test acc_test = { 0 };
	int32_t i;

	if (dev_info->acc_dev_index >= ACC_DEV_COUNT_MAX)
		return;
	hwlog_info("%s:acc_dev_index=%d\n", __func__, dev_info->acc_dev_index);

	acc_test.cal_value = acc_cali_data;
	acc_test.value_num = ACC_CAL_NUM;
	acc_test.threshold_num = ACC_THRESHOLD_NUM;
	acc_test.first_item = ACC_CALI_X_OFFSET_MSG;
	acc_test.min_threshold = min_threshold_acc;
	acc_test.max_threshold = max_threshold_acc;
	if (memcpy_s(acc_test.name, sizeof(acc_test.name),
		acc_test_cal[dev_info->acc_dev_index],
		strlen(acc_test_cal[dev_info->acc_dev_index]) + 1) != EOK)
		return;
	if (memcpy_s(acc_test.result, sizeof(acc_test.result),
		sensor_cal_result(dev_info->acc_calibration_res),
		(strlen(sensor_cal_result(dev_info->acc_calibration_res)) +
		1)) != EOK)
		return;
	for (i = 0; i < ACC_CAL_NUM; i++)
		acc_test.test_name[i] =
			acc_test_name[dev_info->acc_dev_index][i];

	enq_notify_work_sensor(acc_test);
}
#endif

static int acc_calibrate_on_enter(struct acc_device_info *dev_info)
{
	int ret;
	interval_param_t param = { 0 };

	/* if acc is not opened, open first */
	if (sensor_status.opened[TAG_ACCEL] == 0) {
		dev_info->acc_opened_before_calibrate = 0;
		hwlog_info("send acc open cmd(during calibrate) to mcu\n");
		ret = inputhub_sensor_enable(TAG_ACCEL, true);
		if (ret) {
			dev_info->acc_calibration_res = COMMU_FAIL;
			hwlog_err("send acc open cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	} else {
		dev_info->acc_opened_before_calibrate = 1;
	}
	/* period must <= 100 ms */
	if ((sensor_status.delay[TAG_ACCEL] == 0) ||
	    (sensor_status.delay[TAG_ACCEL] > ACC_PERIOD_MAX)) {
		hwlog_info("send acc setdelay cmd(during calibrate) to mcu\n");
		param.period = ACC_PERIOD_MAX;
		ret = inputhub_sensor_setdelay(TAG_ACCEL, &param);
		if (ret) {
			dev_info->acc_calibration_res = COMMU_FAIL;
			hwlog_err("send acc set delay cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return -1;
		}
	}

	return 0;
}

static void acc_calibrate_on_exit(struct acc_device_info *dev_info)
{
	int ret;

	if (dev_info->acc_opened_before_calibrate == 0) {
		dev_info->acc_opened_before_calibrate = 1;
		hwlog_info("send acc close cmd(during calibrate) to mcu\n");
		ret = inputhub_sensor_enable(TAG_ACCEL, false);
		if (ret) {
			dev_info->acc_calibration_res = COMMU_FAIL;
			hwlog_err("send acc close cmd(during calibrate) to mcu fail,ret=%d\n",
				ret);
			return;
		}
	}
}

ssize_t attr_acc_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct g_sensor_platform_data *pf_data = NULL;
	struct acc_device_info *dev_info = NULL;
	enum ret_type rtype = RET_INIT;
	unsigned long val = 0;
	int ret;
	struct read_info read_pkg;

	pf_data = acc_get_platform_data(TAG_ACCEL);
	dev_info = acc_get_device_info(TAG_ACCEL);
	if (pf_data == NULL || dev_info == NULL)
		return -1;
	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	if (val < 1 || val > 6) /* max 6 */
		return count;

	ret = acc_calibrate_on_enter(dev_info);
	if (ret)
		return count;

	msleep(300); /* sleep 300 ms */
	/* send calibrate command, need set delay first */
	read_pkg = send_calibrate_cmd(TAG_ACCEL, val, &rtype);
	dev_info->acc_calibration_res = rtype;
	if (dev_info->acc_calibration_res == COMMU_FAIL)
		return count;
	else if (read_pkg.errno == 0)
		acc_calibrate_save(read_pkg.data, read_pkg.data_length);

#ifdef SENSOR_DATA_ACQUISITION
	acc_calibrate_enq_notify_work(pf_data, dev_info,
		(int32_t *)read_pkg.data);
#endif

	acc_calibrate_on_exit(dev_info);
	return count;
}

ssize_t attr_get_acc_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *id_string = NULL;

	id_string = acc_get_sensors_id_string();
	if (id_string == NULL)
		return -1;

	hwlog_info("%s is %s\n", __func__, id_string);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
		id_string);
}

ssize_t show_acc_set_calidata(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct g_sensor_platform_data *pf_data = NULL;

	hwlog_info("%s: inter show acc set calidata node\n", __func__);
	pf_data = acc_get_platform_data(TAG_ACCEL);
	if (!pf_data)
		return -1;
	if (pf_data->calibrate_way == 0)
		return 0;

	if (strlen(get_sensor_chip_info_address(ACC)) != 0)
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
			"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			set_acc_calib_data[0], set_acc_calib_data[1],
			set_acc_calib_data[2], set_acc_calib_data[3],
			set_acc_calib_data[4], set_acc_calib_data[5],
			set_acc_calib_data[6], set_acc_calib_data[7],
			set_acc_calib_data[8], set_acc_calib_data[9],
			set_acc_calib_data[10], set_acc_calib_data[11],
			set_acc_calib_data[12], set_acc_calib_data[13],
			set_acc_calib_data[14]);

	return -1;
}

ssize_t store_acc_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct g_sensor_platform_data *pf_data = NULL;
	int32_t set_acc_sensor_offset[ACC_CALIBRATE_DATA_LENGTH] = { 0 };
	int i;

	hwlog_info("%s: inter acc set calidata node\n", __func__);
	pf_data = acc_get_platform_data(TAG_ACCEL);
	if (!pf_data)
		return -1;

	if (pf_data->calibrate_way == 0) {
		hwlog_info("%s: now is acc self_calibreate\n", __func__);
		return size;
	}
	if (strlen(get_sensor_chip_info_address(ACC)) == 0) {
		hwlog_err("ACC not exits !!\n");
		return -1;
	}
	if (size != sizeof(set_acc_calib_data)) {
		hwlog_err("%s:size %lu is not equal 15*4\n", __func__, size);
		return -1;
	}
	if (memcpy_s(set_acc_calib_data, sizeof(set_acc_calib_data),
		buf, size) != EOK)
		return -1;
	for (i = 0; i < CALIBRATE_DATA_LENGTH; i++) {
		if (set_acc_calib_data[i] < acc_calib_threshold[i].low_threshold ||
		    set_acc_calib_data[i] > acc_calib_threshold[i].high_threshold) {
			hwlog_err("%s: acc calibrated_data is out of range. i = %d, num = %d\n",
				__FUNCTION__, i, set_acc_calib_data[i]);
			return -1;
		}
	}
	acc_calibrate_save(set_acc_calib_data, sizeof(set_acc_calib_data));
	if (read_calibrate_data_from_nv(ACC_OFFSET_NV_NUM,
		ACC_OFFSET_NV_SIZE, "gsensor"))
		return -1;

	/* copy to gsensor_offset by pass */
	if (memcpy_s(set_acc_sensor_offset, sizeof(set_acc_sensor_offset),
		user_info.nv_data, sizeof(set_acc_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read acc_sensor offset: %d %d %d  sensitity:%d %d %d\n",
		set_acc_sensor_offset[0], set_acc_sensor_offset[1],
		set_acc_sensor_offset[2], set_acc_sensor_offset[3],
		set_acc_sensor_offset[4], set_acc_sensor_offset[5]);
	hwlog_info("nve_direct_access read acc_sensor xis_angle: %d %d %d  %d %d %d %d %d %d\n",
		set_acc_sensor_offset[6], set_acc_sensor_offset[7],
		set_acc_sensor_offset[8], set_acc_sensor_offset[9],
		set_acc_sensor_offset[10], set_acc_sensor_offset[11],
		set_acc_sensor_offset[12], set_acc_sensor_offset[13],
		set_acc_sensor_offset[14]);

	if (send_calibrate_data_to_mcu(TAG_ACCEL, SUB_CMD_SET_OFFSET_REQ,
		set_acc_sensor_offset, sizeof(set_acc_sensor_offset), false))
		return -1;
	return size;
}

ssize_t sensors_calibrate_show_acc(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct acc_device_info *dev_info = NULL;

	dev_info = acc_get_device_info(tag);
	if (dev_info == NULL)
		return -1;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)(dev_info->acc_calibration_res != SUC));
}

ssize_t show_acc1_set_calidata(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct g_sensor_platform_data *pf_data = NULL;

	hwlog_info("%s: inter show acc1 set calidata node\n", __func__);
	pf_data = acc_get_platform_data(TAG_ACC1);
	if (pf_data == NULL)
		return -1;

	if (pf_data->calibrate_way == 0)
		return 0;

	if (strlen(get_sensor_chip_info_address(ACC1)) != 0)
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
			"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
			/* offset:data[0]-data[2] sensitivity:data[3]-data[5] */
			set_acc1_calib_data[0], set_acc1_calib_data[1],
			set_acc1_calib_data[2], set_acc1_calib_data[3],
			set_acc1_calib_data[4], set_acc1_calib_data[5],
			/* six_angle:data[6] - data[14] */
			set_acc1_calib_data[6], set_acc1_calib_data[7],
			set_acc1_calib_data[8], set_acc1_calib_data[9],
			set_acc1_calib_data[10], set_acc1_calib_data[11],
			set_acc1_calib_data[12], set_acc1_calib_data[13],
			set_acc1_calib_data[14]);

	return -1;
}

ssize_t store_acc1_set_calidata(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct g_sensor_platform_data *pf_data = NULL;
	int32_t set_acc1_sensor_offset[ACC1_OFFSET_DATA_LENGTH] = { 0 };
	int i;

	hwlog_info("%s: inter acc1 set calidata node\n", __func__);
	pf_data = acc_get_platform_data(TAG_ACC1);
	if (pf_data == NULL)
		return -1;

	if (pf_data->calibrate_way == 0) {
		hwlog_info("%s: now is acc1 self_calibreate\n", __func__);
		return size;
	}
	if (strlen(get_sensor_chip_info_address(ACC1)) == 0) {
		hwlog_err("ACC1 not exits !!\n");
		return -1;
	}
	if (size != sizeof(set_acc1_calib_data)) {
		hwlog_err("%s:size %lu is not equal 15*4\n", __func__, size);
		return -1;
	}
	if (memcpy_s(set_acc1_calib_data, sizeof(set_acc1_calib_data),
		buf, size) != EOK)
		return -1;
	for (i = 0; i < CALIBRATE_DATA_LENGTH; i++) {
		if (set_acc1_calib_data[i] < acc_calib_threshold[i].low_threshold ||
		    set_acc1_calib_data[i] > acc_calib_threshold[i].high_threshold) {
			hwlog_err("%s: acc1 calibrated_data is out of range. i = %d, num = %d\n",
				__FUNCTION__, i, set_acc1_calib_data[i]);
			return -1;
		}
	}
	acc1_calibrate_save(set_acc1_calib_data, sizeof(set_acc1_calib_data));
	for (i = 0; i < CALIBRATE_DATA_LENGTH; i++)
		hwlog_info("set acc1 calibrate success data[%d] value is %d\n",
			i, set_acc1_calib_data[i]);

	if (read_calibrate_data_from_nv(ACC1_OFFSET_NV_NUM,
		ACC1_OFFSET_NV_SIZE, ACC1_NV_NAME))
		return -1;

	/* copy to gsensor_offset by pass */
	if (memcpy_s(set_acc1_sensor_offset, sizeof(set_acc1_sensor_offset),
		user_info.nv_data, sizeof(set_acc1_sensor_offset)) != EOK)
		return -1;
	hwlog_info("nve_direct_access read acc1_sensor offset: %d %d %d  sensitity:%d %d %d\n",
		set_acc1_sensor_offset[0], set_acc1_sensor_offset[1],
		set_acc1_sensor_offset[2], set_acc1_sensor_offset[3],
		set_acc1_sensor_offset[4], set_acc1_sensor_offset[5]);
	hwlog_info("nve_direct_access read acc1_sensor xis_angle: %d %d %d  %d %d %d %d %d %d\n",
		set_acc1_sensor_offset[6], set_acc1_sensor_offset[7],
		set_acc1_sensor_offset[8], set_acc1_sensor_offset[9],
		set_acc1_sensor_offset[10], set_acc1_sensor_offset[11],
		set_acc1_sensor_offset[12], set_acc1_sensor_offset[13],
		set_acc1_sensor_offset[14]);

	if (send_calibrate_data_to_mcu(TAG_ACC1, SUB_CMD_SET_OFFSET_REQ,
		set_acc1_sensor_offset, sizeof(set_acc1_sensor_offset), false))
		return -1;
	return size;
}

