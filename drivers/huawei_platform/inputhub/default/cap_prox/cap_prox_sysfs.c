/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: cap prox sysfs source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "cap_prox_sysfs.h"

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

#define CAP_PROX_THRESHOLD_NUM      2
#define CAP_PROX_CAL_NUM            2

#ifdef SENSOR_DATA_ACQUISITION
#define CAP_PROX_TEST_CAL           "SAR"
#define CAP_PROX_RESULT      "pass"

#define SAR_SENSOR_PH1_OFFSET_MSG    703016001
#define SAR_SENSOR_PH2_OFFSET_MSG    703016002
#define SAR_SENSOR_DIFF1_MSG         703016003
#define SAR_SENSOR_DIFF2_MSG         703016004

#define SAR_SENSOR1_PH1_OFFSET_MSG   703016011
#define SAR_SENSOR1_PH2_OFFSET_MSG   703016012
#define SAR_SENSOR1_DIFF1_MSG        703016013
#define SAR_SENSOR1_DIFF2_MSG        703016014
#endif

enum sar_calibrate_type {
	NEAR_CALIBRATE = 1,
	FAR_CALIBRATE,
	SAR_CALIBRATE_END = 3,
};

enum sar_sensor_type {
	HW_CAP_PROX_SENSOR = 0,
	HW_CAP_PROX1_SENSOR = 10,
};

#ifdef SENSOR_DATA_ACQUISITION
static char *cap_prox_offset_test_name[CAP_PROX_CAL_NUM] = {
	"SAR_SENSOR_PH1_OFFSET_MSG", "SAR_SENSOR_PH2_OFFSET_MSG"
};

static char *cap_prox_diff_test_name[CAP_PROX_CAL_NUM] = {
	"SAR_SENSOR_DIFF1_MSG", "SAR_SENSOR_DIFF2_MSG"
};

static char *cap_prox1_offset_test_name[CAP_PROX_CAL_NUM] = {
	"SAR_SENSOR1_PH1_OFFSET_MSG", "SAR_SENSOR1_PH2_OFFSET_MSG"
};

static char *cap_prox1_diff_test_name[CAP_PROX_CAL_NUM] = {
	"SAR_SENSOR1_DIFF1_MSG", "SAR_SENSOR1_DIFF2_MSG"
};
#endif

static enum ret_type return_cap_prox_calibration = RET_INIT;
static enum ret_type return_cap_prox1_calibration = RET_INIT;
static struct work_struct cap_prox_calibrate_work;
static struct work_struct cap_prox1_calibrate_work;
static int cap_prox_calibrate_len;
static uint8_t cap_prox_calibrate_data[CAP_PROX_CALIDATA_NV_SIZE];
static int cap_prox1_calibrate_len;
static uint8_t cap_prox1_calibrate_data[CAP_PROX1_CALIDATA_NV_SIZE];

#ifdef SENSOR_DATA_ACQUISITION
static void cap_prox_do_enq_work(int calibrate_index,
	int32_t *cap_prox_data, int32_t *min_threshold,
	int32_t *max_threshold, int sar_type)
{
	struct sensor_eng_cal_test air_test = { 0 };
	int i;

	if (!cap_prox_data || !min_threshold || !max_threshold) {
		hwlog_err("%s: input error\n", __func__);
		return;
	}
	air_test.cal_value = cap_prox_data;
	air_test.value_num = CAP_PROX_CAL_NUM;
	air_test.threshold_num = CAP_PROX_THRESHOLD_NUM;
	air_test.min_threshold = min_threshold;
	air_test.max_threshold = max_threshold;
	if (memcpy_s(air_test.name, sizeof(air_test.name),
		CAP_PROX_TEST_CAL, sizeof(CAP_PROX_TEST_CAL)) != EOK)
		return;
	if (memcpy_s(air_test.result, sizeof(air_test.result),
		CAP_PROX_RESULT, strlen(CAP_PROX_RESULT) + 1) != EOK)
		return;

	switch (calibrate_index) {
	case NEAR_CALIBRATE:
		air_test.first_item = SAR_SENSOR_DIFF1_MSG + sar_type;
		for (i = 0; i < CAP_PROX_CAL_NUM; i++) {
			if (sar_type == HW_CAP_PROX_SENSOR)
				air_test.test_name[i] =
					cap_prox_diff_test_name[i];
			else
				air_test.test_name[i] =
					cap_prox1_diff_test_name[i];
		}
		enq_notify_work_sensor(air_test);
		break;
	case FAR_CALIBRATE:
		air_test.first_item = SAR_SENSOR_PH1_OFFSET_MSG + sar_type;
		for (i = 0; i < CAP_PROX_CAL_NUM; i++) {
			if (sar_type == HW_CAP_PROX_SENSOR)
				air_test.test_name[i] =
					cap_prox_offset_test_name[i];
			else
				air_test.test_name[i] =
					cap_prox1_offset_test_name[i];
		}
		enq_notify_work_sensor(air_test);
		break;
	default:
		hwlog_err("%s: error calibrate_index=%d\n", __func__, calibrate_index);
		break;
	}
}
#endif

static int cap_prox1_calibrate_save(void *buf, int length)
{
	int ret = 0;
	u32 *pcaldata = NULL;

	if (!buf) {
		hwlog_err("%s invalid argument", __func__);
		return_cap_prox1_calibration = EXEC_FAIL;
		return -1;
	}
	pcaldata = (u32 *)buf;
	hwlog_err("%s:cap_prox1 calibrate ok, %08x  %08x  %08x\n", __func__,
		pcaldata[0], pcaldata[1], pcaldata[2]);

	if (read_calibrate_data_from_nv(CAP_PROX1_CALIDATA_NV_NUM,
		CAP_PROX1_CALIDATA_NV_SIZE, "CSENSOR1")) {
		return_cap_prox1_calibration = EXEC_FAIL;
		hwlog_err("nve_direct_access read error %d\n", ret);
		return -2; /* -2: read error */
	}

	if (write_calibrate_data_to_nv(CAP_PROX1_CALIDATA_NV_NUM, length,
		"CSENSOR1", buf)) {
		return_cap_prox1_calibration = EXEC_FAIL;
		hwlog_err("nve_direct_access write error %d\n", ret);
		return -3; /* -3: write error */
	}
	return_cap_prox1_calibration = SUC;
	return 0;
}

static void cap_prox1_calibrate_work_func(struct work_struct *work)
{
	int ret;

	hwlog_info("cap_prox1 calibrate work enter ++\n");
	ret = cap_prox1_calibrate_save(cap_prox1_calibrate_data,
		cap_prox1_calibrate_len);
	if (ret < 0)
		hwlog_err("nv write failed\n");
	hwlog_info("cap_prox1 calibrate work enter --\n");
}

static void cap_prox_calibrate_data_process(int calibrate_index,
	const char *data, uint16_t *diff_value, uint16_t *offset_value,
	uint16_t *calibrate_thred, int sar_type)
{
	int32_t min_threshold[CAP_PROX_THRESHOLD_NUM] = { 0 };
	int32_t max_threshold[CAP_PROX_THRESHOLD_NUM] = { 0 };
	int32_t cap_prox_data[CAP_PROX_CAL_NUM] = { 0 };

	if (!diff_value || !offset_value || !calibrate_thred || !data) {
		hwlog_err("%s: input error\n", __func__);
		return;
	}
	switch (calibrate_index) {
	case NEAR_CALIBRATE:
		if (memcpy_s(diff_value,
			sizeof(uint16_t) * SAR_USE_PH_NUM, data,
			sizeof(uint16_t) * SAR_USE_PH_NUM) != EOK)
			return;
		cap_prox_data[0] = diff_value[0];
		cap_prox_data[1] = diff_value[1];
		min_threshold[0] = calibrate_thred[DIFF_MIN_THREDHOLD];
		max_threshold[0] = calibrate_thred[DIFF_MAX_THREDHOLD];
		break;
	case FAR_CALIBRATE:
		if (memcpy_s(offset_value,
			sizeof(uint16_t) * SAR_USE_PH_NUM,
			data,
			sizeof(uint16_t) * SAR_USE_PH_NUM) != EOK)
			return;
		cap_prox_data[0] = offset_value[0];
		cap_prox_data[1] = offset_value[1];
		min_threshold[0] = calibrate_thred[OFFSET_MIN_THREDHOLD];
		max_threshold[0] = calibrate_thred[OFFSET_MAX_THREDHOLD];
		break;
	default:
		hwlog_err("%s: calibrate_index error\n", __func__);
		break;
	}
	hwlog_info("%s: offset1=%d offset2=%d diff1=%d diff2=%d\n", __func__,
		offset_value[0], offset_value[1],
		diff_value[0], diff_value[1]);

#ifdef SENSOR_DATA_ACQUISITION
	cap_prox_do_enq_work(calibrate_index, cap_prox_data,
		min_threshold, max_threshold, sar_type);
#endif
}

static int32_t cap_prox1_cali_process(unsigned long val)
{
	struct sar_platform_data *pf_data = NULL;
	struct cap_prox_device_info *dev_info = NULL;
	struct read_info pkg_mcu = { 0 };
	uint16_t *diff_value = NULL;
	uint16_t *offset_value = NULL;
	uint16_t *calibrate_thred = NULL;

	pf_data = cap_prox_get_platform_data(TAG_CAP_PROX1);
	dev_info = cap_prox_get_device_info(TAG_CAP_PROX1);
	if (!pf_data || !dev_info)
		return -1;
	return_cap_prox1_calibration = EXEC_FAIL;
	pkg_mcu = send_calibrate_cmd(TAG_CAP_PROX1, val,
		&return_cap_prox1_calibration);
	if (return_cap_prox1_calibration == COMMU_FAIL || pkg_mcu.errno != 0) {
		hwlog_err("%s: errno = %d\n", __func__, pkg_mcu.errno);
		return -1;
	}
	diff_value = dev_info->sar_calibrate_datas.diff;
	offset_value = dev_info->sar_calibrate_datas.offset;
	if (!strncmp(get_sensor_chip_info_address(CAP_PROX1),
		"huawei,semtech-sx9323", strlen("huawei,semtech-sx9323"))) {
		calibrate_thred = pf_data->sar_datas.semteck_data.calibrate_thred;
	} else if (!strncmp(get_sensor_chip_info_address(CAP_PROX1),
		"huawei,semtech-sx9335", strlen("huawei,semtech-sx9335"))) {
		calibrate_thred = pf_data->sar_datas.semteck_data.calibrate_thred;
	} else if (!strncmp(get_sensor_chip_info_address(CAP_PROX1),
		"huawei,abov-a96t3x6", strlen("huawei,abov-a96t3x6"))) {
		calibrate_thred = pf_data->sar_datas.abov_data.calibrate_thred;
	}
	cap_prox_calibrate_data_process((int)val, pkg_mcu.data, diff_value,
		offset_value, calibrate_thred, HW_CAP_PROX1_SENSOR);
	cap_prox1_calibrate_len = sizeof(dev_info->sar_calibrate_datas);
	if (cap_prox1_calibrate_len > sizeof(cap_prox1_calibrate_data))
		cap_prox1_calibrate_len = sizeof(cap_prox1_calibrate_data);
	if (memcpy_s(cap_prox1_calibrate_data,
		sizeof(cap_prox1_calibrate_data),
		&dev_info->sar_calibrate_datas,
		cap_prox1_calibrate_len) != EOK)
		return -1;
	return 0;
}

ssize_t attr_cap_prox1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;

	if (strlen(get_sensor_chip_info_address(CAP_PROX1)) == 0) {
		hwlog_err("%s: no sar sensor\n", __func__);
		return -EINVAL;
	}
	if (kstrtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	if (val >= SAR_CALIBRATE_END || val < NEAR_CALIBRATE) {
		hwlog_err("%s: val = %lu\n", __func__, val);
		return -EINVAL;
	}
	if (cap_prox1_cali_process(val))
		return count;
	INIT_WORK(&cap_prox1_calibrate_work, cap_prox1_calibrate_work_func);
	queue_work(system_power_efficient_wq, &cap_prox1_calibrate_work);
	return count;
}

ssize_t attr_cap_prox_calibrate_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int ret = 0;
	u32 *pcaldata = NULL;

	if (read_calibrate_data_from_nv(CAP_PROX_CALIDATA_NV_NUM,
		CAP_PROX_CALIDATA_NV_SIZE, "Csensor")) {
		hwlog_err("nve_direct_access read error %d\n", ret);
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			return_cap_prox_calibration);
	}

	pcaldata = (u32 *)(user_info.nv_data + 4); /* 4 bytes offset */
	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d:%08x %08x %08x\n",
		return_cap_prox_calibration,
		pcaldata[0], pcaldata[1], pcaldata[2]);
}

static int cap_prox_calibrate_save(void *buf, int length)
{
	int ret = 0;
	u32 *pcaldata = NULL;

	if (!buf) {
		hwlog_err("%s invalid argument", __func__);
		return_cap_prox_calibration = EXEC_FAIL;
		return -1;
	}
	pcaldata = (u32 *)buf;
	hwlog_err("%s:cap_prox calibrate ok, %08x  %08x  %08x\n",
		__func__, pcaldata[0], pcaldata[1], pcaldata[2]);

	if (read_calibrate_data_from_nv(CAP_PROX_CALIDATA_NV_NUM,
		CAP_PROX_CALIDATA_NV_SIZE, "Csensor")) {
		return_cap_prox_calibration = EXEC_FAIL;
		hwlog_err("nve_direct_access read error %d\n", ret);
		return -2; /* -2: read error */
	}
	if (write_calibrate_data_to_nv(CAP_PROX_CALIDATA_NV_NUM,
		length, "Csensor", buf)) {
		return_cap_prox_calibration = EXEC_FAIL;
		hwlog_err("nve_direct_access write error %d\n", ret);
		return -3; /* -3: write error */
	}
	return_cap_prox_calibration = SUC;
	return 0;
}

static void cap_prox_calibrate_work_func(struct work_struct *work)
{
	int ret;

	hwlog_info("cap_prox calibrate work enter ++\n");
	ret = cap_prox_calibrate_save(cap_prox_calibrate_data,
		cap_prox_calibrate_len);
	if (ret < 0)
		hwlog_err("nv write faild\n");
	hwlog_info("cap_prox calibrate work enter --\n");
}

static int32_t cap_prox_cali_process(unsigned long val)
{
	struct sar_platform_data *pf_data = NULL;
	struct cap_prox_device_info *dev_info = NULL;
	struct read_info pkg_mcu = { 0 };
	uint16_t *diff_value = NULL;
	uint16_t *offset_value = NULL;
	uint16_t *calibrate_thred = NULL;

	pf_data = cap_prox_get_platform_data(TAG_CAP_PROX);
	dev_info = cap_prox_get_device_info(TAG_CAP_PROX);
	if (!pf_data || !dev_info)
		return -1;
	return_cap_prox_calibration = EXEC_FAIL;
	pkg_mcu = send_calibrate_cmd(TAG_CAP_PROX, val,
		&return_cap_prox_calibration);
	if (return_cap_prox_calibration == COMMU_FAIL || pkg_mcu.errno != 0) {
		hwlog_err("%s: errno = %d\n", __func__, pkg_mcu.errno);
		return -1;
	}
	diff_value = dev_info->sar_calibrate_datas.diff;
	offset_value = dev_info->sar_calibrate_datas.offset;
	if (!strncmp(get_sensor_chip_info_address(CAP_PROX),
		"huawei,semtech-sx9323", strlen("huawei,semtech-sx9323"))) {
		calibrate_thred = pf_data->sar_datas.semteck_data.calibrate_thred;
	} else if (!strncmp(get_sensor_chip_info_address(CAP_PROX),
		"huawei,abov-a96t3x6", strlen("huawei,abov-a96t3x6"))) {
		calibrate_thred = pf_data->sar_datas.abov_data.calibrate_thred;
	} else if (!strncmp(get_sensor_chip_info_address(CAP_PROX),
		"huawei,awi-aw9610x", strlen("huawei,awi-aw9610x"))) {
		calibrate_thred = pf_data->sar_datas.awi_data.calibrate_thred;
	}
	cap_prox_calibrate_data_process((int)val, pkg_mcu.data, diff_value,
		offset_value, calibrate_thred, HW_CAP_PROX_SENSOR);
	cap_prox_calibrate_len = sizeof(dev_info->sar_calibrate_datas);
	if (cap_prox_calibrate_len > sizeof(cap_prox_calibrate_data))
		cap_prox_calibrate_len = sizeof(cap_prox_calibrate_data);
	if (memcpy_s(cap_prox_calibrate_data, sizeof(cap_prox_calibrate_data),
		&dev_info->sar_calibrate_datas, cap_prox_calibrate_len) != EOK)
		return -1;
	return 0;
}

ssize_t attr_cap_prox_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val;

	if (strlen(get_sensor_chip_info_address(CAP_PROX)) == 0) {
		hwlog_err("%s: no sar sensor\n", __func__);
		return -EINVAL;
	}
	if (kstrtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;
	if (val >= SAR_CALIBRATE_END || val < NEAR_CALIBRATE) {
		hwlog_err("%s: val = %lu\n", __func__, val);
		return -EINVAL;
	}
	if (cap_prox_cali_process(val))
		return count;
	INIT_WORK(&cap_prox_calibrate_work, cap_prox_calibrate_work_func);
	queue_work(system_power_efficient_wq, &cap_prox_calibrate_work);
	return count;
}

ssize_t show_cap_prox_calibrate_method(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct sar_platform_data *pf_data = NULL;

	pf_data = cap_prox_get_platform_data(TAG_CAP_PROX);
	if (pf_data == NULL)
		return -1;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%d\n",
		pf_data->calibrate_type);
}

ssize_t show_cap_prox_calibrate_orders(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *order_string = NULL;

	switch (tag) {
	case TAG_CAP_PROX:
	case TAG_CAP_PROX1:
		order_string = cap_prox_get_calibrate_order_string();
		if (order_string == NULL)
			return -1;
		return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
			order_string);
	default:
		hwlog_err("tag %d get cap_prox_calibrate_orders not implement in %s\n",
			tag, __func__);
		break;
	}

	return 0;
}

ssize_t attr_get_cap_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *id_string = NULL;

	id_string = cap_prox_get_sensors_id_string();
	if (id_string == NULL)
		return -1;

	hwlog_info("%s is %s\n", __func__, id_string);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
		id_string);
}

ssize_t show_sar_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct cap_prox_device_info *dev_info = NULL;

	dev_info = cap_prox_get_device_info(TAG_CAP_PROX);
	if (!dev_info)
		return -1;
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"offset1:%d offset2:%d diff1:%d diff2:%d\n",
		dev_info->sar_calibrate_datas.offset[CAP_PROX_PHASE0],
		dev_info->sar_calibrate_datas.offset[CAP_PROX_PHASE1],
		dev_info->sar_calibrate_datas.diff[CAP_PROX_PHASE0],
		dev_info->sar_calibrate_datas.diff[CAP_PROX_PHASE1]);
}

ssize_t sensors_calibrate_show_cap_prox(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (tag == TAG_CAP_PROX)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			(int32_t)(return_cap_prox_calibration != SUC));
	else if (tag == TAG_CAP_PROX1)
		return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
			(int32_t)(return_cap_prox1_calibration != SUC));

	return -1;
}

