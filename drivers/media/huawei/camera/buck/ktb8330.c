/*
 * ktb8330.c
 *
 * driver function of ktb8330
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

#include <linux/delay.h>
#include "hw_buck.h"
#include "../cam_log.h"
#include "../platform/sensor_commom.h"

/* register define */
/* Controls VOUT settings when VSEL pin = LOW */
#define REG_VSEL0   0x00
/* Controls VOUT settings when VSEL pin = HIGH */
#define REG_VSEL1   0x01
/* Determines whether VOUT output discharge is enabled and also
 * the slew rate of positive transitions
 */
#define REG_CONTROL 0x02
/* Read-only register identifies vendor and chip type */
#define REG_ID1     0x03
/* Read-only register identifies die revision */
#define REG_ID2     0x04
/* Indicates device status */
#define REG_MONITOR 0x05

#define PIN_VSEL_HIGH 1
#define PIN_VSEL_LOW  0

#define REG_ID1_TYPE 0xA0

#define KTB8330_RANG1_OUT_VOL_MIN 600000 /* 0.6V */
#define KTB8330_RANG1_OUT_VOL_MAX 1393750 /* 1.39375V */
#define KTB8330_RANG1_VOL_STEP    6250 /* 6.25mV step */
#define KTB8330_RANG2_OUT_VOL_MIN    1440000 /* 1.4V */
#define KTB8330_RANG2_OUT_VOL_MAX 3345000 /* 3.345V */
#define KTB8330_RANG2_VOL_STEP    15000 /* 15mV step */

#define BIT3_0_MASK    0xF7
#define BIT3_1_MASK    0x08

static struct mutex buck_mut_ktb8330 = __MUTEX_INITIALIZER(buck_mut_ktb8330);
static struct hw_buck_i2c_client ktb8330_i2c_client;

static int ktb8330_get_dt_data(struct hw_buck_ctrl_t *buck_ctrl)
{
	struct hw_buck_info *buck_info = NULL;
	int rc;

	if (!buck_ctrl) {
		cam_err("%s buck ctrl is NULL", __func__);
		return -1;
	}
	rc = hw_buck_get_dt_data(buck_ctrl);
	if (rc < 0)
		return -1;

	buck_info = &buck_ctrl->buck_info;
	buck_ctrl->buck_i2c_client->client->addr = buck_info->slave_address;

	return 0;
}

static int ktb8330_match(struct hw_buck_ctrl_t *buck_ctrl)
{
	int ret;
	u8 reg_value = 0;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client) {
		cam_err("%s buck_ctrl or buck_i2c_client is NULL", __func__);
		return -1;
	}

	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;
	if (!i2c_func) {
		cam_err("%s i2c_func is NULL", __func__);
		return -1;
	}
	ret = i2c_func->i2c_read(buck_ctrl->buck_i2c_client, REG_ID1, &reg_value);
	if (ret < 0) {
		cam_err("%s: get ID1 value failed, ret = %d", __func__, ret);
		return -1;
	}
	if (reg_value == REG_ID1_TYPE) {
		cam_info("%s get ID1 value succ, 0x%x", __func__, reg_value);
	} else {
		cam_err("%s get ID1 value failed, 0x%x", __func__, reg_value);
		return -1;
	}
	return 0;
}

static int ktb8330_calc_voltage_level(u32 *volt)
{
	u32 level;

	if (!volt) {
		cam_err("%s,volt is NULL", __func__);
		return -1;
	}

	if ((*volt >= KTB8330_RANG1_OUT_VOL_MIN) &&
		(*volt <= KTB8330_RANG1_OUT_VOL_MAX))
		level = (*volt - KTB8330_RANG1_OUT_VOL_MIN) / KTB8330_RANG1_VOL_STEP;
	else
		level = (*volt - KTB8330_RANG2_OUT_VOL_MIN) / KTB8330_RANG2_VOL_STEP;

	return level;
}

static int ktb8330_set_voltage_range(struct hw_buck_ctrl_t *buck_ctrl, u32 *volt)
{
	struct hw_buck_i2c_fn_t *i2c_func = NULL;
	struct hw_buck_i2c_client *i2c_client = NULL;
	u8 reg_value;

	if (!volt || !buck_ctrl) {
		cam_err("%s,input is NULL", __func__);
		return -1;
	}

	i2c_client = buck_ctrl->buck_i2c_client;
	if (!i2c_client) {
		cam_err("%s i2c_client is NULL", __func__);
		return -1;
	}
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;
	if (!i2c_func) {
		cam_err("%s i2c_func is NULL", __func__);
		return -1;
	}

	i2c_func->i2c_read(i2c_client, REG_CONTROL, &reg_value);

	if (*volt < KTB8330_RANG1_OUT_VOL_MIN) {
		*volt = KTB8330_RANG1_OUT_VOL_MIN;
		reg_value = reg_value & BIT3_0_MASK;
	} else if ((*volt >= KTB8330_RANG1_OUT_VOL_MIN) &&
		(*volt <= KTB8330_RANG1_OUT_VOL_MAX)) {
		reg_value = reg_value & BIT3_0_MASK;
	} else if ((*volt > KTB8330_RANG1_OUT_VOL_MAX) &&
		(*volt < KTB8330_RANG2_OUT_VOL_MIN)) {
		*volt = KTB8330_RANG2_OUT_VOL_MIN;
		reg_value = reg_value | BIT3_1_MASK;
	} else if ((*volt >= KTB8330_RANG2_OUT_VOL_MIN) &&
		(*volt <= KTB8330_RANG2_OUT_VOL_MAX)) {
		reg_value = reg_value | BIT3_1_MASK;
	} else {
		*volt = KTB8330_RANG2_OUT_VOL_MAX;
		reg_value = reg_value | BIT3_1_MASK;
	}

	if (i2c_func->i2c_write(i2c_client, REG_CONTROL, reg_value) < 0) {
		cam_err("%s,write reg0x02 failed", __func__);
		return -1;
	}

	return 0;
}

int ktb8330_set_voltage(struct hw_buck_ctrl_t *buck_ctrl,
	u8 reg_vsel, u32 voltage, int state)
{
	int ret;
	u8 reg_vsel_value;
	u8 reg_value;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;
	struct hw_buck_i2c_client *i2c_client = NULL;
	u32 *volt = &voltage;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	i2c_client = buck_ctrl->buck_i2c_client;
	if (!i2c_client) {
		cam_err("%s i2c_client is NULL", __func__);
		return -1;
	}
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;
	if (!i2c_func) {
		cam_err("%s i2c_func is NULL", __func__);
		return -1;
	}

	ret = ktb8330_set_voltage_range(buck_ctrl, volt);
	if (ret < 0) {
		cam_err("%s set range fail", __func__);
		return -1;
	}

	if (state == POWER_OFF) {
		reg_vsel_value = 0x0;
	} else {
		/* Vout = 0.600V+NSELx*6.25mV */
		/* NSELx bit6~bit0 */
		reg_vsel_value = ktb8330_calc_voltage_level(volt);
		/* 0x80: bit7=1 */
		reg_vsel_value = 0x80 + reg_vsel_value;
	}
	cam_info("%s enter, voltage:%u, reg_vsel_value:%u",
		__func__, *volt, reg_vsel_value);
	ret = i2c_func->i2c_write(i2c_client, reg_vsel, reg_vsel_value);
	if (ret < 0) {
		cam_err("%s write reg_vsel failed,ret = %d", __func__, ret);
		return -1;
	}

	i2c_func->i2c_read(i2c_client, REG_MONITOR, &reg_value);
	cam_info("%s monitor reg =0x%x", __func__, reg_value);

	return 0;
}

static int ktb8330_power_config(struct hw_buck_ctrl_t *buck_ctrl,
	u32 voltage, int state)
{
	int ret;

	cam_info("%s enter, voltage:%u state:%d", __func__, voltage, state);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	if (state != POWER_OFF && state != POWER_ON) {
		cam_err("%s state [%d] error", __func__, state);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "ktb8330");
	if (ret < 0) {
		cam_err("req gpio[%u] failed", buck_ctrl->buck_info.en_pin);
		return -1;
	}

	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, state);
	if (ret < 0)
		cam_err("set gpio[%u] failed", buck_ctrl->buck_info.en_pin);

	gpio_free(buck_ctrl->buck_info.en_pin);

	if (state == POWER_OFF)
		return 0;

	/* software enable buck */
	if (buck_ctrl->buck_info.pin_vsel == PIN_VSEL_LOW)
		/* low->vsel0 en */
		ret = ktb8330_set_voltage(buck_ctrl,
			REG_VSEL0, voltage, state);
	else
		/* low->vsel1 en */
		ret = ktb8330_set_voltage(buck_ctrl,
			REG_VSEL1, voltage, state);

	return ret;
}

static int ktb8330_init(struct hw_buck_ctrl_t *buck_ctrl)
{
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;
	u8 reg_value = 0;
	int ret;

	cam_info("%s enter", __func__);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;
	ret = i2c_func->i2c_read(i2c_client, REG_CONTROL, &reg_value);
	if (ret < 0) {
		cam_err("%s: read REG_CONTROL failed, ret = %d", __func__, ret);
		return -1;
	}
	cam_info("%s, REG_CONTROL:%u", __func__, reg_value);

	ret = i2c_func->i2c_read(i2c_client, REG_ID2, &reg_value);
	if (ret < 0) {
		cam_err("%s: read version failed, ret = %d", __func__, ret);
		return -1;
	}

	ret = i2c_func->i2c_read(i2c_client, REG_MONITOR, &reg_value);
	if (ret < 0) {
		cam_err("%s: read monitor failed, ret = %d", __func__, ret);
		return -1;
	}
	cam_info("%s, REG_MONITOR:%u", __func__, reg_value);

	ret = i2c_func->i2c_read(i2c_client, REG_VSEL0, &reg_value);
	if (ret < 0) {
		cam_err("%s: read reg_vsel0 failed, ret = %d", __func__, ret);
		return -1;
	}

	ret = i2c_func->i2c_read(i2c_client, REG_VSEL1, &reg_value);
	if (ret < 0) {
		cam_err("%s: read reg_vsel1 failed, ret = %d", __func__, ret);
		return -1;
	}

	return 0;
}

static int ktb8330_exit(struct hw_buck_ctrl_t *buck_ctrl)
{
	int ret;

	cam_info("%s enter", __func__);
	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "ktb8330");
	if (ret < 0) {
		cam_err("req gpio[%u] failed", buck_ctrl->buck_info.en_pin);
		return -1;
	}
	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, 0); /* pull low */
	if (ret < 0)
		cam_err("set gpio[%u] failed", buck_ctrl->buck_info.en_pin);

	gpio_free(buck_ctrl->buck_info.en_pin);
	return 0;
}

static struct hw_buck_fn_t ktb8330_func_tbl = {
	.buck_power_config = ktb8330_power_config,
	.buck_get_dt_data = ktb8330_get_dt_data,
	.buck_match = ktb8330_match,
	.buck_init = ktb8330_init,
	.buck_exit = ktb8330_exit,
};

/* driver_data */
static struct hw_buck_ctrl_t ktb8330_ctrl = {
	.buck_i2c_client = &ktb8330_i2c_client,
	.func_tbl = &ktb8330_func_tbl,
	.buck_mutex = &buck_mut_ktb8330,
};

static const struct i2c_device_id ktb8330_id[] = {
	/* name, driver_data */
	{ "ktb8330", (unsigned long)&ktb8330_ctrl },
};

static const struct of_device_id ktb8330_dt_match[] = {
	{ .compatible = "huawei,ktb8330" },
};

MODULE_DEVICE_TABLE(of, ktb8330_dt_match);

static struct i2c_driver ktb8330_i2c_driver = {
	.probe    = hw_buck_i2c_probe,
	.remove   = hw_buck_remove,
	.id_table = ktb8330_id,
	.driver = {
		.name           = "ktb8330",
		.of_match_table = ktb8330_dt_match,
	},
};

static int __init ktb8330_module_init(void)
{
	cam_info("%s enter", __func__);
	return i2c_add_driver(&ktb8330_i2c_driver);
}

static void __exit ktb8330_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&ktb8330_i2c_driver);
}

module_init(ktb8330_module_init);
module_exit(ktb8330_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("KTB8330 BUCK");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
