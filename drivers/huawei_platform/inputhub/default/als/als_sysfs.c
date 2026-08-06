/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als sysfs source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "als_sysfs.h"

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

#include "als_tp_color.h"
#include "contexthub_route.h"

#define LAYA_PHONE_TYPE 37
#define ALS_PERIOD_MAX 100

enum {
	ALS_CALI_ENV_LIGHT = 1,
	ALS_CALI_ENV_DARK = 2,
	ALS_CALI_ENV_LEAKAGE = 7,
};

#ifdef SENSOR_DATA_ACQUISITION
#define ALS_CAL_NUM                  6
#define ALS_THRESHOLD_NUM            6

#define ALS_CALI_R_MSG               703013001
#define ALS_CALI_G_MSG               703013002
#define ALS_CALI_B_MSG               703013003
#define ALS_CALI_C_MSG               703013004
#define ALS_CALI_LUX_MSG             703013005
#define ALS_CALI_CCT_MSG             703013006
#define ALS_CALI_DARK_OFFSET_MSG     703013007

static char *als_test_cal[ALS_DEV_COUNT_MAX] = {
	"ALS", "ALS1", "ALS2"
};
static char *als_dark_cali_name[ALS_DEV_COUNT_MAX] = {
	"ALS_DARK_CALI_OFFSET",
	"ALS1_DARK_CALI_OFFSET",
	"ALS2_DARK_CALI_OFFSET"
};
static char *als_test_name[ALS_DEV_COUNT_MAX][ALS_CAL_NUM] = {
	{
	"ALS_CALI_R_MSG", "ALS_CALI_G_MSG", "ALS_CALI_B_MSG",
	"ALS_CALI_C_MSG", "ALS_CALI_LUX_MSG", "ALS_CALI_CCT_MSG"
	},
	{
	"ALS1_CALI_R_MSG", "ALS1_CALI_G_MSG", "ALS1_CALI_B_MSG",
	"ALS1_CALI_C_MSG", "ALS1_CALI_LUX_MSG", "ALS1_CALI_CCT_MSG"
	},
	{
	"ALS2_CALI_R_MSG", "ALS2_CALI_G_MSG", "ALS2_CALI_B_MSG",
	"ALS2_CALI_C_MSG", "ALS2_CALI_LUX_MSG", "ALS2_CALI_CCT_MSG"
	}
};
#endif

static int g_stop_auto_als;

int get_stop_auto_als(void)
{
	return g_stop_auto_als;
}

static ssize_t attr_als_calibrate_show_by_tag(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct als_device_info *dev_info = NULL;

	dev_info = als_get_device_info(tag);
	if (!dev_info)
		return -1;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)dev_info->als_calibration_res);
}

ssize_t attr_als_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return attr_als_calibrate_show_by_tag(TAG_ALS, dev, attr, buf);
}

ssize_t attr_als1_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return attr_als_calibrate_show_by_tag(TAG_ALS1, dev, attr, buf);
}

ssize_t attr_als2_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return attr_als_calibrate_show_by_tag(TAG_ALS2, dev, attr, buf);
}

static int write_als_offset_to_nv(int32_t tag, const char *temp, int32_t length)
{
	uint32_t addr = 0;
	uint32_t cp_len = length;
	struct als_device_info *dev_info = NULL;
	uint8_t nv_buf[ALS_CALIDATA_NV_TOTAL_SIZE];

	dev_info = als_get_device_info(tag);
	if (!dev_info || !temp || length <= 0) {
		hwlog_err("write_als_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (cp_len > ALS_CALIDATA_NV_ONE_SIZE)
		cp_len = ALS_CALIDATA_NV_ONE_SIZE;

	if (als_get_calidata_nv_addr(tag, &addr))
		return -1;
	if ((addr + ALS_CALIDATA_NV_ONE_SIZE) > sizeof(nv_buf))
		return -1;

	if (read_calibrate_data_from_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_TOTAL_SIZE, "LSENSOR"))
		return -1;
	if (memcpy_s(nv_buf, sizeof(nv_buf), user_info.nv_data,
		ALS_CALIDATA_NV_TOTAL_SIZE) != EOK)
		return -1;
	if (memcpy_s(&nv_buf[addr], ALS_CALIDATA_NV_ONE_SIZE, temp, cp_len) != EOK)
		return -1;
	if (write_calibrate_data_to_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_TOTAL_SIZE, "LSENSOR", nv_buf))
		return -1;

	if (memcpy_s((void *)dev_info->als_sensor_calibrate_data,
		sizeof(dev_info->als_sensor_calibrate_data), temp, cp_len) != EOK)
		return -1;

	return 0;
}

static int write_als_dark_noise_offset_to_nv(int32_t tag,
	uint16_t *dark_noise_offset)
{
	uint32_t addr = 0;
	struct als_device_info *dev_info = NULL;
	uint8_t nv_buf[ALS_CALIDATA_NV_TOTAL_SIZE];

	dev_info = als_get_device_info(tag);
	if (!dark_noise_offset || !dev_info) {
		hwlog_err("write_als_dark_noise_offset_to_nv fail, invalid para\n");
		return -1;
	}

	if (als_get_calidata_nv_addr(tag, &addr))
		return -1;
	if ((addr + ALS_CALIDATA_NV_SIZE + sizeof(uint16_t)) > sizeof(nv_buf))
		return -1;

	if (read_calibrate_data_from_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_TOTAL_SIZE, "LSENSOR"))
		return -1;
	if (memcpy_s(nv_buf, sizeof(nv_buf), user_info.nv_data,
		ALS_CALIDATA_NV_TOTAL_SIZE) != EOK)
		return -1;
	if (memcpy_s(&nv_buf[addr + ALS_CALIDATA_NV_SIZE],
		(ALS_CALIDATA_NV_ONE_SIZE - ALS_CALIDATA_NV_SIZE), dark_noise_offset,
		sizeof(uint16_t)) != EOK)
		return -1;
	if (memcpy_s(dev_info->als_sensor_calibrate_data,
		sizeof(dev_info->als_sensor_calibrate_data), &nv_buf[addr],
		ALS_CALIDATA_NV_SIZE + sizeof(uint16_t)) != EOK)
		return -1;
	msleep(10); /* sleep 10 ms */
	if (write_calibrate_data_to_nv(ALS_CALIDATA_NV_NUM,
		ALS_CALIDATA_NV_TOTAL_SIZE,
		"LSENSOR", nv_buf))
		return -1;

	return 0;
}

#ifdef SENSOR_DATA_ACQUISITION
static void als_dark_noise_offset_enq_notify_work(
	struct als_device_info *dev_info, const int item_id, uint16_t value,
	uint16_t min_threshold, uint16_t max_threshold)
{
	int ret;
	struct event offset_event = { 0 };

	if (dev_info->als_dev_index >= ALS_DEV_COUNT_MAX)
		return;
	hwlog_info("%s:als_dev_index=%d\n", __func__, dev_info->als_dev_index);

	offset_event.item_id = item_id;
	if (memcpy_s(offset_event.device_name,
		sizeof(offset_event.device_name),
		als_test_cal[dev_info->als_dev_index],
		strlen(als_test_cal[dev_info->als_dev_index]) + 1) != EOK)
		return;
	if (memcpy_s(offset_event.result,
		sizeof(offset_event.result),
		sensor_cal_result(dev_info->als_calibration_res),
		strlen(sensor_cal_result(dev_info->als_calibration_res)) +
		1) != EOK) {
		hwlog_err("%s memcpy_s result error\n", __func__);
		return;
	}
	if (memcpy_s(offset_event.test_name, sizeof(offset_event.test_name),
		als_dark_cali_name[dev_info->als_dev_index],
		strlen(als_dark_cali_name[dev_info->als_dev_index]) +
		1) != EOK)
		return;
	snprintf_s(offset_event.value, MAX_VAL_LEN, MAX_VAL_LEN - 1,
		"%d", value);
	snprintf_s(offset_event.min_threshold, MAX_VAL_LEN, MAX_VAL_LEN - 1,
		"%u", min_threshold);
	snprintf_s(offset_event.max_threshold, MAX_VAL_LEN, MAX_VAL_LEN - 1,
		"%u", max_threshold);

	ret = enq_msg_data_in_sensorhub_single(offset_event);
	if (ret > 0)
		hwlog_info("als_dark_noise_offset_enq_notify_work succ!!item_id=%d\n",
			offset_event.item_id);
	else
		hwlog_info("als_dark_noise_offset_enq_notify_work failed\n");
}
#endif

static int als_calibrate_save(int32_t tag, struct als_device_info *dev_info,
	const void *buf, int length)
{
	const uint16_t *poffset_data = (const uint16_t *)buf;
	int ret;

	if (!buf || length <= 0) {
		hwlog_err("%s invalid argument\n", __func__);
		dev_info->als_calibration_res = EXEC_FAIL;
		return -1;
	}

	hwlog_info("%s:als calibrate ok, %d  %d  %d %d  %d  %d\n", __func__,
		   *poffset_data, *(poffset_data + 1), *(poffset_data + 2),
		   *(poffset_data + 3), *(poffset_data + 4), *(poffset_data + 5));
	ret = write_als_offset_to_nv(tag, (char *)buf, length);
	if (ret) {
		hwlog_err("nv write fail\n");
		dev_info->als_calibration_res = NV_FAIL;
		return -1;
	}
	dev_info->als_calibration_res = SUC;
	return 0;
}

#ifdef SENSOR_DATA_ACQUISITION

#define APDS9253_DARK_NOISE_OFFSET_MAX 10
#define APDS9253_DARK_NOISE_OFFSET_MIN 0

/* not support als1 als2 */
static void als_calibrate_enq_notify_work(struct als_platform_data *pf_data,
	struct als_device_info *dev_info, unsigned long val,
	uint16_t *cali_data_u16)
{
	int32_t als_cali_data_int32[ALS_CAL_NUM];
	int32_t min_threshold_als[ALS_CAL_NUM];
	int32_t max_threshold_als[ALS_CAL_NUM];
	struct sensor_eng_cal_test als_test = { 0 };
	int i;

	if (dev_info->als_dev_index >= ALS_DEV_COUNT_MAX)
		return;
	hwlog_info("%s:als_dev_index=%d\n", __func__, dev_info->als_dev_index);

	if (val == ALS_CALI_ENV_DARK &&
		(pf_data->als_phone_type == LAYA_PHONE_TYPE ||
		dev_info->is_cali_supported)) {
		als_dark_noise_offset_enq_notify_work(dev_info,
			ALS_CALI_DARK_OFFSET_MSG, *cali_data_u16,
			APDS9253_DARK_NOISE_OFFSET_MIN,
			APDS9253_DARK_NOISE_OFFSET_MAX);
		return;
	}

	for (i = 0; i < ALS_CAL_NUM; i++) {
		als_cali_data_int32[i] = *(cali_data_u16 + i);
		min_threshold_als[i] = dev_info->min_thres;
		max_threshold_als[i] = dev_info->max_thres;
	}

	hwlog_info("als calibrate data for collect, %d %d %d %d %d %d\n",
		*cali_data_u16, *(cali_data_u16 + 1), *(cali_data_u16 + 2),
		*(cali_data_u16 + 3), *(cali_data_u16 + 4), *(cali_data_u16 + 5));

	als_test.cal_value = als_cali_data_int32;
	als_test.first_item = ALS_CALI_R_MSG;
	als_test.value_num = ALS_CAL_NUM;
	als_test.threshold_num = ALS_THRESHOLD_NUM;
	als_test.min_threshold = min_threshold_als;
	als_test.max_threshold = max_threshold_als;

	if (memcpy_s(als_test.name, sizeof(als_test.name),
		als_test_cal[dev_info->als_dev_index],
		strlen(als_test_cal[dev_info->als_dev_index]) + 1) != EOK)
		return;
	if (memcpy_s(als_test.result, sizeof(als_test.result),
		sensor_cal_result(dev_info->als_calibration_res),
		(strlen(sensor_cal_result(dev_info->als_calibration_res)) +
		1)) != EOK)
		return;
	for (i = 0; i < ALS_CAL_NUM; i++)
		als_test.test_name[i] =
			als_test_name[dev_info->als_dev_index][i];

	enq_notify_work_sensor(als_test);
}
#endif

static int als_calibrate_on_write(int32_t tag,
	struct als_platform_data *pf_data, struct als_device_info *dev_info,
	unsigned long val)
{
	enum ret_type rtype = RET_INIT;
	struct read_info pkg_mcu;

	/* send calibrate command, need set delay first */
	pkg_mcu = send_calibrate_cmd(tag, val, &rtype);
	dev_info->als_calibration_res = rtype;
	if (dev_info->als_calibration_res == COMMU_FAIL) {
		return -1;
	} else if (pkg_mcu.errno == 0) {
		uint8_t *cali_data = NULL;

		if (val == ALS_CALI_ENV_DARK &&
			pf_data->als_phone_type == LAYA_PHONE_TYPE)
			write_als_dark_noise_offset_to_nv(tag, (uint16_t *)pkg_mcu.data);
		else
			als_calibrate_save(tag, dev_info, pkg_mcu.data,
				pkg_mcu.data_length);

		cali_data = (uint8_t *)dev_info->als_sensor_calibrate_data;
		hwlog_info("als_sensor_calibrate_data %d %d %d %d %d %d %d\n",
			(((uint32_t)cali_data[1] << 8) | cali_data[0]),
			(((uint32_t)cali_data[3] << 8) | cali_data[2]),
			(((uint32_t)cali_data[5] << 8) | cali_data[4]),
			(((uint32_t)cali_data[7] << 8) | cali_data[6]),
			(((uint32_t)cali_data[9] << 8) | cali_data[8]),
			(((uint32_t)cali_data[11] << 8) | cali_data[10]),
			(((uint32_t)cali_data[13] << 8) | cali_data[12]));
	}

#ifdef SENSOR_DATA_ACQUISITION
	als_calibrate_enq_notify_work(pf_data, dev_info, val,
		(uint16_t *)pkg_mcu.data);
#endif

	return 0;
}


static int als_calibrate_on_enter(int32_t tag,
	struct als_device_info *dev_info)
{
	int ret;
	interval_param_t param = { 0 };

	/* if ALS is not opened, open first */
	if (sensor_status.opened[tag] == 0) {
		dev_info->als_opened_before_calibrate = 0;
		hwlog_info("send als open cmd(during calibrate)\n");
		ret = inputhub_sensor_enable(tag, true);
		if (ret) {
			dev_info->als_calibration_res = COMMU_FAIL;
			hwlog_err("send als open cmd(during calibrate)fail %d\n", ret);
			return -1;
		}
	} else {
		dev_info->als_opened_before_calibrate = 1;
	}

	if ((sensor_status.delay[tag] == 0) ||
	    (sensor_status.delay[tag] > ALS_PERIOD_MAX)) {
		hwlog_info("send als setdelay cmd(during calibrate)\n");
		param.period = ALS_PERIOD_MAX;
		ret = inputhub_sensor_setdelay(tag, &param);
		if (ret) {
			dev_info->als_calibration_res = COMMU_FAIL;
			hwlog_err("send als setdelay cmd(during calibrate)fail %d\n", ret);
			return -1;
		}
	}

	return 0;
}

static void als_calibrate_on_exit(int32_t tag,
	struct als_device_info *dev_info)
{
	int ret;

	if (dev_info->als_opened_before_calibrate == 0) {
		dev_info->als_opened_before_calibrate = 1;
		hwlog_info("send als close cmd(during calibrate)\n");
		ret = inputhub_sensor_enable(tag, false);
		if (ret) {
			dev_info->als_calibration_res = COMMU_FAIL;
			hwlog_err("send als close cmd(during calibrate)fail %d\n", ret);
			return;
		}
	}
}

ssize_t attr_als_calibrate_write_by_tag(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned long val = 0;
	int ret;
	struct als_platform_data *pf_data = NULL;
	struct als_device_info *dev_info = NULL;

	pf_data = als_get_platform_data(tag);
	dev_info = als_get_device_info(tag);
	if (!pf_data || !dev_info)
		return -1;

	if (als_flag_result_cali(dev_info) == 0) {
		hwlog_info("%s: no need calibrate chip_type=%d is_cali_supported=%d\n",
			__func__, dev_info->chip_type,
			dev_info->is_cali_supported);
		return count;
	}

	ret = get_tpcolor_from_nv(tag);
	if (ret)
		hwlog_err("cali write, get_tpcolor_from_nv fail %d\n", ret);
	else
		hwlog_info("cali write, get_tpcolor_from_nv success\n");

	if (strict_strtoul(buf, TO_DECIMALISM, &val))
		return -EINVAL;

	switch (val) {
	case ALS_CALI_ENV_LIGHT: /* als light calibrate */
		break;
	case ALS_CALI_ENV_DARK: /* als dark current calibrate */
		if (pf_data->als_phone_type == LAYA_PHONE_TYPE)
			break;
		return count;
	case ALS_CALI_ENV_LEAKAGE: /* als leakage light calibrate */
		if (pf_data->is_bllevel_supported)
			break;
		/* fall-through */
	default:
		return count; /* for other cmd, drop it. */
	}

	ret = als_calibrate_on_enter(tag, dev_info);
	if (ret)
		return count;

	msleep(350); /* sleep 350 ms */

	if (als_calibrate_on_write(tag, pf_data, dev_info, val))
		return count;
	als_calibrate_on_exit(tag, dev_info);
	return count;
}

ssize_t attr_als_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return attr_als_calibrate_write_by_tag(TAG_ALS, dev, attr, buf, count);
}

ssize_t attr_als1_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return attr_als_calibrate_write_by_tag(TAG_ALS1, dev, attr, buf, count);
}

ssize_t attr_als2_calibrate_write(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	return attr_als_calibrate_write_by_tag(TAG_ALS2, dev, attr, buf, count);
}

ssize_t attr_set_stop_als_auto_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	g_stop_auto_als = simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	hwlog_err("%s stop_auto_als %d\n", __func__, g_stop_auto_als);
	return size;
}

ssize_t attr_set_dt_als_sensor_stup(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct sensor_data event;
	unsigned long source;

	source = simple_strtoul(buf, NULL, TO_HEXADECIMAL);
	hwlog_err("%s buf %s, source %lu\n", __func__, buf, source);

	event.type = TAG_ALS;
	event.length = 4; /* 4 bytes */
	event.value[0] = (int)source;

	inputhub_route_write(ROUTE_SHB_PORT, (char *)&event, 8); /* 8 bytes */
	return size;
}

ssize_t attr_get_als_sensor_id(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	char *id_string = NULL;

	id_string = als_get_sensors_id_string();
	if (id_string == NULL)
		return -1;

	hwlog_info("%s is %s\n", __func__, id_string);
	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s\n",
		id_string);
}

ssize_t sensors_calibrate_show_als(int tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct als_device_info *dev_info = NULL;

	dev_info = als_get_device_info(tag);
	if (dev_info == NULL)
		return -1;

	return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
		(int32_t)(dev_info->als_calibration_res != SUC));
}

