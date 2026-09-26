/*
 * power_thermalzone.h
 *
 * thermal for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_THERMALZONE_H_
#define _POWER_THERMALZONE_H_

#define POWER_TZ_SENSORS                      10
#define POWER_TZ_STR_MAX_LEN                  16
#define POWER_TZ_GROUP_SIZE                   2

#define POWER_TZ_DEFAULT_TEMP                 25000
#define POWER_TZ_MIN_TEMP                     (-40000)
#define POWER_TZ_MAX_TEMP                     125000

/* in order to resolve divisor less than zero */
#define PRORATE_OF_INIT                       1000

/* get temp data through platform adc */
#define POWER_TZ_DEFAULT_OPS                  "platform"
#define POWER_TZ_ADC_RETRY_TIMES              3

enum power_tz_ntc_index {
	NTC_BEGIN = 0,
	NTC_PULLUP_100K_RT = NTC_BEGIN, /* Thermitor 07050125 */
	NTC_PULLUP_10K_VT = 1, /* Thermitor 07050124, pullup 10K */
	NTC_PULLUP_12K_VT = 2, /* Thermitor 07050124, pullup 12K */
	NTC_PULLUP_18K_VT = 3, /* Thermitor 07050124, pullup 18K */
	NTC_PULLUP_18K_RT = 4, /* Thermitor ERTJZEG103FA, pullup 18K */
	NTC_NO_CONVERSION = 5, /* No temperature conversion required */
	NTC_PULLUP_22K_VT = 6, /* Thermitor 100K, pullup 22K */
	NTC_PULLUP_10K_RT = 7, /* Thermitor 10K, pullup 22K */
	NTC_PULLUP_100K_VT = 8, /* Thermitor 100K, pullup 100K */
	NTC_END,
};

struct power_tz_sensor {
	char sensor_name[POWER_TZ_STR_MAX_LEN];
	char ops_name[POWER_TZ_STR_MAX_LEN];
	u32 adc_channel;
	u32 ntc_index;
	struct thermal_zone_device *tz_dev;
	void *dev_data;
	int (*get_raw_data)(int, long *, void *);
};

struct power_tz_info {
	struct platform_device *pdev;
	struct device *dev;
	int total_sensor;
	struct power_tz_sensor sensor[POWER_TZ_SENSORS];
};

struct power_tz_ops {
	void *dev_data;
	int (*get_raw_data)(int, long *, void *);
};

#ifdef CONFIG_HUAWEI_POWER_THERMALZONE
int power_tz_ops_register(struct power_tz_ops *ops, char *ops_name);
#else
static inline int power_tz_ops_register(struct power_tz_ops *ops, char *ops_name)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_POWER_THERMALZONE */

#endif /* _POWER_THERMALZONE_H_ */
