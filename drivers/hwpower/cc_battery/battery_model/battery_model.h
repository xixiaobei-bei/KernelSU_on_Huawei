/*
 * battery_model.h
 *
 * huawei battery model information
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

#ifndef _BATTERY_MODEL_H_
#define _BATTERY_MODEL_H_

#include <linux/iio/consumer.h>
#include <chipset_common/hwpower/common_module/power_supply.h>
#include <chipset_common/hwpower/battery/battery_model_public.h>

#define BAT_MODEL_NAME_LEN          32
#define BAT_MODEL_BRAND_LEN         32
#define BAT_MODEL_TABLE_COLS        4
#define BAT_MODEL_TABLE_ROWS        8
#define BAT_MODEL_TYPE_LEN          4
#define BAT_MODEL_INVALID_CHANNEL   (-1)
#define BAT_MODEL_INVALID_INDEX     (-1)
#define BAT_MODEL_DEFAULT_INDEX     0
#define BAT_MODEL_DSM_CONTENT_SIZE  128
#define BAT_MODEL_INVALID_FCC       (-1)

struct bat_model_table {
	char name[BAT_MODEL_NAME_LEN];
	int id_vol_low;
	int id_vol_high;
	char sn_type[BAT_MODEL_TYPE_LEN];
};

struct bat_model_device {
	struct device *dev;
	struct bat_model_ops *ops;
	struct bat_model_table tables[BAT_MODEL_TABLE_ROWS];
	int table_size;
	int id_adc_channel;
	int id_index;
	int default_index;
	int fcc;
	int design_fcc;
	int vbat_max;
	int technology;
	unsigned int support_iio;
	bool is_iio_init;
	struct iio_channel *id_iio;
	char brand[BAT_MODEL_BRAND_LEN];
};

#ifdef CONFIG_HUAWEI_BATTERY_MODEL
int bat_model_get_vbat_max(void);
int bat_model_get_design_fcc(void);
int bat_model_get_technology(void);
int bat_model_is_removed(void);
const char *bat_model_get_brand(void);
#else
static inline int bat_model_get_vbat_max(void)
{
	return POWER_SUPPLY_DEFAULT_VOLTAGE_MAX;
}

static inline int bat_model_get_design_fcc(void)
{
	return POWER_SUPPLY_DEFAULT_CAPACITY_FCC;
}

static inline int bat_model_get_technology(void)
{
	return POWER_SUPPLY_DEFAULT_TECHNOLOGY;
}

static inline int bat_model_is_removed(void)
{
	return 0;
}

static inline const char *bat_model_get_brand(void)
{
	return POWER_SUPPLY_DEFAULT_BRAND;
}
#endif /* CONFIG_HUAWEI_BATTERY_MODEL */

#endif /* _BATTERY_MODEL_H_ */
