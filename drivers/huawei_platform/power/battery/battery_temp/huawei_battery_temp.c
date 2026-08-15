/*
 * huawei_battery_temp.c
 *
 * battery temp driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/thermal.h>
#include <linux/hisi/hisi_adc.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/battery_voltage.h>
#include <huawei_platform/power/huawei_battery_temp.h>

#define HWLOG_TAG huawei_battery_temp
HWLOG_REGIST();

#define UTEMP_PER_MTEMP                      1000
#define ABNORMAL_BATT_TEMPERATURE_POWEROFF   67000
#define LOW_BATT_TEMP_CHECK_THRESHOLD        10000
#define DELTA_TEMP                           15000
#define DEFAULT_TEMP                         (-40000)
#define TEMP_LOW                             15000
#define TEMP_HIGH                            40000
#define BATT_TEMP_NTC_INVALID_THRE           10
#define BATT_TEMP_NTC_SAMPLES                3
#define COMP_PARA_ARRAYSIZE                  6
#define TEMP_SAMPLES                         3
#define RETRY_TIMES                          5
#define SENSOR_NAME_BATT_ID_0                "bat_0"
#define SENSOR_NAME_BATT_ID_1                "bat_1"
#define READ_TEMPERATURE_MS                  5000 /* 5 sec */
#define TEMPERATURE_CHANGE_LIMIT             1000 /* 1 degrees */
#define HUNDRED                              100
#define TEMPERATURE_UPDATE_STATUS            0
#define TEMPERATURE_INIT_STATUS              1

enum batt_temp_comp_para_info {
	BATT_TEMP_COMP_PARA_ICHG = 0,
	BATT_TEMP_COMP_PARA_VALUE,
	BATT_TEMP_COMP_PARA_TOTAL,
};

enum batt_temp_type {
	HW_BATT_TEMP_SYSFS_BATT_TEMP_0 = 0,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_1,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_0_COMP,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_1_COMP,
	HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED_COMP,
};

struct batt_temp_comp_para_data {
	int batt_temp_comp_ichg;
	int batt_temp_comp_value;
};

struct hw_batt_temp_info {
	struct device *dev;
	struct batt_temp_comp_para_data comp_para[COMP_PARA_ARRAYSIZE];
	struct delayed_work temp_work;
	int ntc_compensation_is;
	int batt_temp_low;
	int batt_temp_high;
	int batt_temp_0;
	int batt_temp_1;
	int batt_temp_mixed;
	int init_temp;
	int batt_temp_0_raw;
	int batt_temp_1_raw;
};

static struct hw_batt_temp_info *g_di;

#ifdef CONFIG_SYSFS
#define HW_BATT_TEMP_SYSFS_FIELD(_name, n, m, store) \
{ \
	.attr = __ATTR(_name, m, hw_batt_temp_sysfs_show, store), \
	.name = HW_BATT_TEMP_SYSFS_##n, \
}

#define HW_BATT_TEMP_SYSFS_FIELD_RO(_name, n) \
	HW_BATT_TEMP_SYSFS_FIELD(_name, n, 0444, NULL)

struct hw_batt_temp_sysfs_field_info {
	struct device_attribute attr;
	u8 name;
};

static ssize_t hw_batt_temp_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);

static struct hw_batt_temp_sysfs_field_info hw_batt_temp_sysfs_field_tbl[] = {
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_0,          BATT_TEMP_0),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_1,          BATT_TEMP_1),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_mixed,      BATT_TEMP_MIXED),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_0_comp,     BATT_TEMP_0_COMP),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_1_comp,     BATT_TEMP_1_COMP),
	HW_BATT_TEMP_SYSFS_FIELD_RO(batt_temp_mixed_comp, BATT_TEMP_MIXED_COMP),
};

static struct attribute *hw_batt_temp_sysfs_attrs
	[ARRAY_SIZE(hw_batt_temp_sysfs_field_tbl) + 1];

static const struct attribute_group hw_batt_temp_sysfs_attr_group = {
	.attrs = hw_batt_temp_sysfs_attrs,
};

static void hw_batt_temp_sysfs_init_attrs(void)
{
	int i;
	int limit = ARRAY_SIZE(hw_batt_temp_sysfs_field_tbl);

	for (i = 0; i < limit; i++)
		hw_batt_temp_sysfs_attrs[i] =
			&hw_batt_temp_sysfs_field_tbl[i].attr.attr;

	hw_batt_temp_sysfs_attrs[limit] = NULL;
}

static struct hw_batt_temp_sysfs_field_info *hw_batt_temp_sysfs_field_lookup(
	const char *name)
{
	int i;
	int limit = ARRAY_SIZE(hw_batt_temp_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		if (!strncmp(name,
			hw_batt_temp_sysfs_field_tbl[i].attr.attr.name,
			strlen(name)))
			break;
	}

	if (i >= limit)
		return NULL;

	return &hw_batt_temp_sysfs_field_tbl[i];
}

static int hw_batt_temp_sysfs_create_group(struct hw_batt_temp_info *di)
{
	hw_batt_temp_sysfs_init_attrs();
	return sysfs_create_group(&di->dev->kobj,
		&hw_batt_temp_sysfs_attr_group);
}

static void hw_batt_temp_sysfs_remove_group(struct hw_batt_temp_info *di)
{
	sysfs_remove_group(&di->dev->kobj, &hw_batt_temp_sysfs_attr_group);
}

#else

static int hw_batt_temp_sysfs_create_group(struct hw_batt_temp_info *di)
{
	return 0;
}

static void hw_batt_temp_sysfs_remove_group(struct hw_batt_temp_info *di)
{
}
#endif /* CONFIG_SYSFS */

static ssize_t hw_batt_temp_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hw_batt_temp_sysfs_field_info *info = NULL;
	struct hw_batt_temp_info *di = g_di;
	int batt_temp = 0;
	int len = 0;

	info = hw_batt_temp_sysfs_field_lookup(attr->attr.name);
	if (!info || !di) {
		hwlog_err("get sysfs entries failed\n");
		return -EINVAL;
	}

	switch (info->name) {
	case HW_BATT_TEMP_SYSFS_BATT_TEMP_0:
		huawei_battery_temp(BAT_TEMP_0, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_1:
		huawei_battery_temp(BAT_TEMP_1, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED:
		huawei_battery_temp(BAT_TEMP_MIXED, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_0_COMP:
		huawei_battery_temp_now(BAT_TEMP_0, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_1_COMP:
		huawei_battery_temp_now(BAT_TEMP_1, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	case HW_BATT_TEMP_SYSFS_BATT_TEMP_MIXED_COMP:
		huawei_battery_temp_now(BAT_TEMP_MIXED, &batt_temp);
		len = snprintf(buf, PAGE_SIZE, "%d\n", batt_temp);
		break;

	default:
		hwlog_err("invalid sysfs_name(%d)\n", info->name);
		break;
	}

	return len;
}

static int get_batt_temp_by_sensor_name(char *sensor_name)
{
	struct thermal_zone_device *tz = NULL;
	int temp_array[BATT_TEMP_NTC_SAMPLES] = {0};
	int temp_samples = TEMP_SAMPLES;
	int temp_invalid_flag = 0;
	int max, min, i;
	int temp;
	int ret;
	int sum = 0;

	tz = thermal_zone_get_zone_by_name(sensor_name);
	if (IS_ERR(tz)) {
		hwlog_err("get %s thermal zone fail\n", sensor_name);
		goto fail_get_temp;
	}

	while (temp_samples--) {
		temp_invalid_flag = 0;

		for (i = 0; i < BATT_TEMP_NTC_SAMPLES; ++i) {
			ret = thermal_zone_get_temp(tz, &temp_array[i]);
			temp_array[i] = temp_array[i];
			if (ret) {
				hwlog_err("get batt temp fail\n");
				temp_invalid_flag = 1;
				break;
			}

			hwlog_info("battID:%s,temp:%d\n",
				sensor_name, temp_array[i]);
		}

		if (temp_invalid_flag == 1)
			continue;

		/* check temperature value is valid */
		max = temp_array[0];
		min = temp_array[0];

		for (i = 0; i < BATT_TEMP_NTC_SAMPLES; ++i) {
			if (max < temp_array[i])
				max = temp_array[i];

			if (min > temp_array[i])
				min = temp_array[i];
		}

		if (max - min > BATT_TEMP_NTC_INVALID_THRE * UTEMP_PER_MTEMP) {
			hwlog_err("invalid temp temp_max:%d temp_min:%d\n",
				max, min);
			temp_invalid_flag = 1;
		}

		if (temp_invalid_flag == 0)
			break;

	}

	if (temp_invalid_flag == 1) {
		hwlog_err("get batt temp fail, use default temperature\n");
		goto fail_get_temp;
	}

	/* get average temperature */
	for (i = 0; i < BATT_TEMP_NTC_SAMPLES; ++i)
		sum += temp_array[i];

	temp = sum / BATT_TEMP_NTC_SAMPLES;
	hwlog_info("sensor_name:%s, temp is:%d\n", sensor_name, temp);
	return temp;

fail_get_temp:
	return DEFAULT_TEMP;
}

static int get_batt_temp_with_comp(int temp_without_compensation)
{
	struct hw_batt_temp_info *di = g_di;
	int temp_with_compensation = temp_without_compensation;
	int ichg;
	int i;

	if (!di) {
		hwlog_err("di is null\n");
		return temp_without_compensation;
	}

	if (!di->ntc_compensation_is) {
		hwlog_err("temp_without_compensation is 0\n");
		return temp_without_compensation;
	}

	ichg = hisi_battery_current();
	ichg = abs(ichg);

	for (i = 0; i < COMP_PARA_ARRAYSIZE; i++) {
		if (ichg >= di->comp_para[i].batt_temp_comp_ichg) {
			temp_with_compensation = temp_without_compensation -
				di->comp_para[i].batt_temp_comp_value;
			hwlog_info("ichg is %dma, batt_temp_comp_value is %d\n",
				ichg, di->comp_para[i].batt_temp_comp_value);
			break;
		}
	}

	hwlog_info("ichg is %dma, old temp is %d, temp with comp is %d\n",
		ichg, temp_without_compensation, temp_with_compensation);
	return temp_with_compensation;
}

static int get_batt_temp_stably(char *sensor_name)
{
	int retry_times = RETRY_TIMES;
	int cnt = 0;
	int temperature = 0;
	int delta;
	int batt_temp;

	batt_temp = get_batt_temp_by_sensor_name(sensor_name);

	while (cnt++ < retry_times) {
		temperature = get_batt_temp_with_comp(batt_temp);
		delta = abs(batt_temp - temperature);

		if ((delta > DELTA_TEMP) ||
			(temperature > ABNORMAL_BATT_TEMPERATURE_POWEROFF) ||
			(temperature <= LOW_BATT_TEMP_CHECK_THRESHOLD)) {
			continue;
		}

		hwlog_info("stably temp, old_temp:%d, cnt:%d, temp:%d\n",
			batt_temp, cnt, temperature);
		return temperature;
	}

	return temperature;
}

static int get_batt_temp_mixed(int bat0_temp, int bat1_temp)
{
	int temp_mixed;
	int batt_temp_high = g_di->batt_temp_high;
	int batt_temp_low = g_di->batt_temp_low;
	s64 temp_pro1;
	s64 temp_pro2;

	hwlog_info("bat0_temp is %d, bat1_temp is %d\n",
		bat0_temp, bat1_temp);
	temp_pro1 = (s64)(bat0_temp - batt_temp_low) *
		(s64)(bat1_temp - batt_temp_low);
	temp_pro2 = (s64)(bat0_temp - batt_temp_high) *
		(s64)(bat1_temp - batt_temp_high);

	if ((bat0_temp < batt_temp_low) && (bat1_temp > batt_temp_high)) {
		temp_mixed = (batt_temp_low - bat0_temp) >
			(bat1_temp - batt_temp_high) ? bat0_temp : bat1_temp;
	} else if ((bat0_temp > batt_temp_high) &&
		(bat1_temp < batt_temp_low)) {
		temp_mixed = (bat0_temp - batt_temp_high) >
			(batt_temp_low - bat1_temp) ? bat0_temp : bat1_temp;
	} else if (((bat0_temp < batt_temp_low) &&
		(bat1_temp < batt_temp_low)) || (temp_pro1 < 0)) {
		temp_mixed = bat0_temp < bat1_temp ? bat0_temp : bat1_temp;
	} else if (((bat0_temp > batt_temp_high) &&
		(bat1_temp > batt_temp_high)) || (temp_pro2 < 0)) {
		temp_mixed = bat0_temp > bat1_temp ? bat0_temp : bat1_temp;
	} else {
		/* temp 0 temp 1 between batt_temp_high and batt_temp_low */
		temp_mixed = (bat0_temp + bat1_temp) / 2; /* average value */
	}

	hwlog_info("mixed batt temp is %d\n", temp_mixed);
	return temp_mixed;
}

static int smooth_batt_temp(int temp_curr, int temp_pre)
{
	hwlog_info("temp_pre:%d, temp_curr:%d\n", temp_pre, temp_curr);

	if (temp_curr - temp_pre > TEMPERATURE_CHANGE_LIMIT)
		temp_curr = temp_pre + TEMPERATURE_CHANGE_LIMIT;
	else if (temp_pre - temp_curr > TEMPERATURE_CHANGE_LIMIT)
		temp_curr = temp_pre - TEMPERATURE_CHANGE_LIMIT;

	return temp_curr;
}

static void init_battery_temp(struct hw_batt_temp_info *di)
{
	int batt_temp_0 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_0);
	int batt_temp_1 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_1);
	int batt_temp_mixed = get_batt_temp_mixed(batt_temp_0, batt_temp_1);
	int batt_temp_0_raw =
		get_batt_temp_by_sensor_name(SENSOR_NAME_BATT_ID_0);
	int batt_temp_1_raw =
		get_batt_temp_by_sensor_name(SENSOR_NAME_BATT_ID_1);

	di->batt_temp_0 = batt_temp_0;
	di->batt_temp_1 = batt_temp_1;
	di->batt_temp_mixed = batt_temp_mixed;
	di->batt_temp_0_raw = batt_temp_0_raw;
	di->batt_temp_1_raw = batt_temp_1_raw;
	hwlog_info("init temp0:%d,temp1:%d,temp_mixed:%d, 0_raw:%d, 1_raw:%d\n",
		batt_temp_0, batt_temp_1, batt_temp_mixed, batt_temp_0_raw,
		batt_temp_1_raw);
}

static void update_battery_temp(struct hw_batt_temp_info *di)
{
	int batt_temp_0;
	int batt_temp_1;
	int batt_temp_mixed;
	int raw_0;
	int raw_1;
	s64 delta_comp, delta_raw;

	if (!di->init_temp) {
		init_battery_temp(di);
		di->init_temp = true;
		return;
	}

	batt_temp_0 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_0);
	batt_temp_1 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_1);
	raw_0 = get_batt_temp_by_sensor_name(SENSOR_NAME_BATT_ID_0);
	raw_1 = get_batt_temp_by_sensor_name(SENSOR_NAME_BATT_ID_1);
	hwlog_info("temp0:%d,temp1:%d,0_raw:%d,1_raw:%d\n",
		batt_temp_0, batt_temp_1, raw_0, raw_1);

	delta_comp = batt_temp_0 - di->batt_temp_0;
	delta_raw = raw_0 - di->batt_temp_0_raw;
	if (delta_comp * delta_raw < 0) {
		if (delta_raw < 0)
			batt_temp_0 = di->batt_temp_0;
	} else {
		if (abs(delta_comp) > abs(delta_raw))
			batt_temp_0 = di->batt_temp_0 + (int)delta_raw;
	}

	delta_comp = batt_temp_1 - di->batt_temp_1;
	delta_raw = raw_1 - di->batt_temp_1_raw;
	if (delta_comp * delta_raw < 0) {
		if (delta_raw < 0)
			batt_temp_1 = di->batt_temp_1;
	} else {
		if (abs(delta_comp) > abs(delta_raw))
			batt_temp_1 = di->batt_temp_1 + (int)delta_raw;
	}

	di->batt_temp_0 = smooth_batt_temp(batt_temp_0, di->batt_temp_0);
	di->batt_temp_1 = smooth_batt_temp(batt_temp_1, di->batt_temp_1);
	batt_temp_mixed = get_batt_temp_mixed(di->batt_temp_0, di->batt_temp_1);
	di->batt_temp_mixed = smooth_batt_temp(batt_temp_mixed,
		di->batt_temp_mixed);
	di->batt_temp_0_raw = raw_0;
	di->batt_temp_1_raw = raw_1;
}

static void read_temperature_work(struct work_struct *work)
{
	struct hw_batt_temp_info *di = container_of(work,
		struct hw_batt_temp_info, temp_work.work);

	update_battery_temp(di);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
}

int huawei_battery_temp(enum battery_temp_id id, int *temp)
{
	int batt_temp;
	struct hw_batt_temp_info *di = g_di;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -1;
	}

	if (!is_hisi_battery_exist())
		hwlog_err("battery not exist\n");

	if (hw_battery_get_series_num() == HW_ONE_BAT) {
		*temp = hisi_battery_temperature();
		hwlog_info("one batt, batt_temp is [%d]\n", *temp);
		return 0;
	}

	if (!di) {
		*temp = hisi_battery_temperature();
		hwlog_err("g_di is null, batt_temp = [%d]\n", *temp);
		return 0;
	}

	if (!di->init_temp) {
		init_battery_temp(di);
		di->init_temp = true;
	}

	switch (id) {
	case BAT_TEMP_0:
		batt_temp = di->batt_temp_0;
		break;

	case BAT_TEMP_1:
		batt_temp = di->batt_temp_1;
		break;

	case BAT_TEMP_MIXED:
		batt_temp = di->batt_temp_mixed;
		break;

	default:
		hwlog_err("invalid battery_temp_id: %d\n", id);
		return -1;
	}

	*temp = batt_temp / UTEMP_PER_MTEMP;
	hwlog_info("hw_batt_temp: temp is %d\n", *temp);
	return 0;
}

int huawei_battery_temp_now(enum battery_temp_id id, int *temp)
{
	int batt_temp;
	int batt_temp_0;
	int batt_temp_1;

	if (!temp) {
		hwlog_err("temp is null\n");
		return -1;
	}

	if (hw_battery_get_series_num() == HW_ONE_BAT) {
		*temp = hisi_battery_temperature_for_charger();
		hwlog_info("one batt, batt_temp is [%d]\n", *temp);
		return 0;
	}

	if (!g_di) {
		*temp = hisi_battery_temperature_for_charger();
		hwlog_err("g_di is null, batt_temp is [%d]\n", *temp);
		return 0;
	}

	switch (id) {
	case BAT_TEMP_0:
		batt_temp = get_batt_temp_stably(SENSOR_NAME_BATT_ID_0);
		break;

	case BAT_TEMP_1:
		batt_temp = get_batt_temp_stably(SENSOR_NAME_BATT_ID_1);
		break;

	case BAT_TEMP_MIXED:
		batt_temp_0 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_0);
		batt_temp_1 = get_batt_temp_stably(SENSOR_NAME_BATT_ID_1);
		batt_temp = get_batt_temp_mixed(batt_temp_0, batt_temp_1);
		break;

	default:
		hwlog_err("invalid battery_temp_id: %d\n", id);
		return -1;
	}

	*temp = batt_temp / UTEMP_PER_MTEMP;
	hwlog_info("hw_batt_comp_temp: temp is %d\n", *temp);
	return 0;
}

static void batt_temp_parse_dts(struct device_node *np,
	struct hw_batt_temp_info *di)
{
	const char *compensation_data_string = NULL;
	int ntc_compensation_is = 0;
	int array_len;
	int i;
	int idata = 0;
	int ret;
	int col;
	int row;

	ret = of_property_read_u32(np, "battery_temp_high",
		&di->batt_temp_high);
	if (ret) {
		hwlog_err("battery_temp_high dts read failed\n");
		di->batt_temp_high = TEMP_HIGH;
	}
	hwlog_info("batt_temp_high=%d\n", di->batt_temp_high);

	ret = of_property_read_u32(np, "battery_temp_low", &di->batt_temp_low);
	if (ret) {
		hwlog_err("batt_temp_low dts read failed\n");
		di->batt_temp_low = TEMP_LOW;
	}
	hwlog_info("batt_temp_low=%d\n", di->batt_temp_low);

	ret = of_property_read_u32(np, "ntc_compensation_is",
		&ntc_compensation_is);
	if (ret) {
		ntc_compensation_is = 0;
		hwlog_err("ntc_compensation_is dts read failed\n");
		return;
	}

	di->ntc_compensation_is = ntc_compensation_is;
	memset(di->comp_para, 0,
		COMP_PARA_ARRAYSIZE * sizeof(struct batt_temp_comp_para_data));

	array_len = of_property_count_strings(np, "ntc_temp_compensation_para");
	if ((array_len <= 0) || (array_len % BATT_TEMP_COMP_PARA_TOTAL != 0)) {
		hwlog_err("batt temp comp para is invaild\n");
		return;
	}

	if (array_len > COMP_PARA_ARRAYSIZE * BATT_TEMP_COMP_PARA_TOTAL) {
		array_len = COMP_PARA_ARRAYSIZE * BATT_TEMP_COMP_PARA_TOTAL;
		hwlog_err("para too long, use %d paras\n", array_len);
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"ntc_temp_compensation_para", i,
			&compensation_data_string);

		if (ret) {
			hwlog_err("ntc_temp_compensation_para dts read failed\n");
			di->ntc_compensation_is = 0;
			return;
		}

		/* 10: decimal base */
		if (kstrtoint(compensation_data_string, 10, &idata)) {
			hwlog_err("kstrtoint fail\n");
			di->ntc_compensation_is = 0;
			return;
		}

		col = i % BATT_TEMP_COMP_PARA_TOTAL;
		row = i / BATT_TEMP_COMP_PARA_TOTAL;
		switch (col) {
		case BATT_TEMP_COMP_PARA_ICHG:
			di->comp_para[row].batt_temp_comp_ichg = idata;
			break;

		case BATT_TEMP_COMP_PARA_VALUE:
			di->comp_para[row].batt_temp_comp_value = idata;
			break;

		default:
			hwlog_err("ntc_temp_compensation_para get failed\n");
			break;
		}

		hwlog_info("di->comp_para[%d][%d] is %d\n", row, col, idata);
	}

}

static int batt_temp_probe(struct platform_device *pdev)
{
	struct hw_batt_temp_info *di = NULL;
	struct device_node *np = NULL;
	struct class *power_class = NULL;
	int ret;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_di = di;
	di->dev = &pdev->dev;
	np = pdev->dev.of_node;
	batt_temp_parse_dts(np, di);
	platform_set_drvdata(pdev, di);

	ret = hw_batt_temp_sysfs_create_group(di);
	if (ret) {
		hwlog_err("sysfs group create failed\n");
		goto fail_free_mem;
	}

	power_class = hw_power_get_class();
	if (power_class) {
		if (!charge_dev)
			charge_dev = device_create(power_class, NULL, 0, NULL,
				"charger");
		if (IS_ERR(charge_dev)) {
			hwlog_err("sysfs device create failed\n");
			ret = PTR_ERR(charge_dev);
			goto fail_free_mem;
		}

		ret = sysfs_create_link(&charge_dev->kobj, &di->dev->kobj,
			"hw_batt_temp");
		if (ret) {
			hwlog_err("sysfs link create failed\n");
			hw_batt_temp_sysfs_remove_group(di);
			goto fail_free_mem;
		}
	}

	INIT_DELAYED_WORK(&di->temp_work, read_temperature_work);
	di->init_temp = false;
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
	hwlog_info("probe end\n");
	return 0;

fail_free_mem:
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_di = NULL;

	return ret;
}

static int batt_temp_remove(struct platform_device *pdev)
{
	struct hw_batt_temp_info *di = platform_get_drvdata(pdev);

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_di = NULL;

	hwlog_info("remove end\n");
	return 0;
}

#ifdef CONFIG_PM
static int batt_temp_resume(struct platform_device *pdev)
{
	struct hw_batt_temp_info *di = NULL;

	di = platform_get_drvdata(pdev);
	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	hwlog_info("batt_temp_resume begin\n");
	init_battery_temp(di);
	schedule_delayed_work(&di->temp_work,
		msecs_to_jiffies(READ_TEMPERATURE_MS));
	hwlog_info("batt_temp_resume end\n");
	return 0;
}

static int batt_temp_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct hw_batt_temp_info *di = platform_get_drvdata(pdev);

	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}

	hwlog_info("batt_temp_suspend begin\n");
	cancel_delayed_work_sync(&di->temp_work);
	hwlog_info("batt_temp_suspend end\n");
	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id batt_temp_match_table[] = {
	{
		.compatible = "huawei,battery_temp",
		.data = NULL,
	},
	{},
};

static struct platform_driver batt_temp_driver = {
	.probe = batt_temp_probe,
	.remove = batt_temp_remove,
#ifdef CONFIG_PM
	.resume = batt_temp_resume,
	.suspend = batt_temp_suspend,
#endif /* CONFIG_PM */
	.driver = {
		.name = "huawei,battery_temp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(batt_temp_match_table),
	},
};

int __init batt_temp_init(void)
{
	return platform_driver_register(&batt_temp_driver);
}

void __exit batt_temp_exit(void)
{
	platform_driver_unregister(&batt_temp_driver);
}

fs_initcall_sync(batt_temp_init);
module_exit(batt_temp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei battery temp module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
