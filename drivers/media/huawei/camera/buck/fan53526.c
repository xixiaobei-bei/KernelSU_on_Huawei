/*
 * fan53526.c
 *
 * driver function of fan53526
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

/*
 * VSEL0,VSEL1
 * bit7: Software buck enable. When EN pin is LOW, the regulator is off.
 * When EN pin is HIGH, BUCK_EN bit takes precedent.
 * bit0~6: Sets VOUT value from 0.600 to 1.39375 V
 * CONTROL
 * bit7: 0 When the regulator is disabled, VOUT is not discharged.
 * When the regulator is disabled, VOUT discharges through an internal pulldown.
 * bit6~4: Sets the slew rate for positive voltage transitions
 * bit3: reserved
 * bit2: RESET Setting to 1 resets all registers to default values.
 * Always reads back 0.
 * bit1~0: mode In combination with the VSEL pin, these two bits set
 * the operation of the buck to be either in Auto-PFM/PWM Mode
 * during light load or Forced PWM mode.
 * ID1
 * bit7~5: VENDOR Signifies Fairchild as the IC vendor.
 * bit4: Reserved
 * bit3~0: DIE_ID DIE ID - FAN53525/6.
 * ID2
 * bit7~4: Reserved Always reads back 0000
 * bit3~0: DIE_REV FAN53526 Die Revision
 * MONITOR
 * bit7: 1: Buck is enabled and soft-start is completed
 * bit6: 1: Signifies the VIN is less than the UVLO threshold
 * bit5: 1: Signifies the VIN is greater than the OVP threshold
 * bit4: 1: Signifies a positive voltage transition is in progress
 * and the output voltage has not yet reached its new setpoint.
 * This bit is also set during IC soft-start
 * bit3: 1: Signifies a negative voltage transition is in progress
 * and the output voltage has not yet reached its new setpoint
 * bit2: 1: Indicates that a register reset was performed.
 * This bit is cleared after register 5 is read
 * bit1: 1: Signifies the thermal shutdown is active
 * bit0: 1: Buck enabled; 0: buck disabled
 */

#define BUCK_ENABLE 1 /* software enable buck */

#define PIN_VSEL_HIGH 1
#define PIN_VSEL_LOW  0

#define REG_ID1_TYPE 0x81

#define FAN53526_IN_VOL_MIN  2500000
#define FAN53526_IN_VOL_MAX  5500000
#define FAN53526_OUT_VOL_MIN 600000 /* 0.6V */
#define FAN53526_OUT_VOL_MAX 1393750 /* 1.39375V */
#define FAN53526_VOL_STEP    6250 /* 6.25mV step */

static struct mutex buck_mut_fan53526 = __MUTEX_INITIALIZER(buck_mut_fan53526);

static int fan53526_get_dt_data(struct hw_buck_ctrl_t *buck_ctrl)
{
	int rc;

	if (!buck_ctrl) {
		cam_err("%s buck ctrl is NULL", __func__);
		return -1;
	}
	rc = hw_buck_get_dt_data(buck_ctrl);
	if (rc < 0)
		return -1;

	return 0;
}

static int fan53526_match(struct hw_buck_ctrl_t *buck_ctrl)
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
	ret = i2c_func->i2c_read(buck_ctrl->buck_i2c_client,
		REG_ID1, &reg_value);
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

static u32 fan53526_calc_voltage_level(u32 voltage)
{
	u32 level; /* NSELx */

	if (voltage > FAN53526_OUT_VOL_MAX)
		level = FAN53526_OUT_VOL_MAX / FAN53526_VOL_STEP;
	else if (voltage < FAN53526_OUT_VOL_MIN)
		level = 0;
	else
		level = (voltage - FAN53526_OUT_VOL_MIN) / FAN53526_VOL_STEP;
	return level;
}

int fan53526_set_voltage(struct hw_buck_ctrl_t *buck_ctrl,
			u8 reg_vsel, u32 voltage, int state)
{
	int ret;
	u8 reg_vsel_value;
	u8 reg_value = 0;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;
	struct hw_buck_i2c_client *i2c_client = NULL;

	if (!buck_ctrl || !buck_ctrl->buck_i2c_client) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}
	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;
	if (!i2c_func) {
		cam_err("%s i2c_func is NULL", __func__);
		return -1;
	}
	if (state == POWER_OFF) {
		reg_vsel_value = 0x0; /* buck enable to set 0 */
	} else {
		/* Vout = 0.600V+NSELx*6.25mV */
		/* NSELx bit6~bit0 */
		reg_vsel_value = fan53526_calc_voltage_level(voltage);
		/* 0x80: bit7=1 */
		reg_vsel_value = 0x80 + reg_vsel_value;
	}
	cam_info("%s enter, voltage:%u, reg_vsel_value:%u",
		__func__, voltage, reg_vsel_value);
	ret = i2c_func->i2c_write(i2c_client, reg_vsel, reg_vsel_value);
	if (ret < 0) {
		cam_err("%s: write reg_vsel failed, ret = %d", __func__, ret);
		return -1;
	}

	i2c_func->i2c_read(i2c_client, REG_MONITOR, &reg_value);
	cam_info("%s monitor reg =0x%x, ret:%d",
		__func__, reg_value, ret);

	return 0;
}

static int fan53526_power_config(struct hw_buck_ctrl_t *buck_ctrl,
				u32 voltage,
				int state)
{
	int ret;

	cam_info("%s enter, voltage:%u state:%d", __func__, voltage, state);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	if (state != POWER_OFF && state != POWER_ON) {
		cam_err("%s state [%u] error", __func__, state);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "fan53526");
	if (ret < 0) {
		cam_err("failed to request gpio[%d]", buck_ctrl->buck_info.en_pin);
		return -1;
	}

	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, state);
	if (ret < 0)
		cam_err("failed to set gpio high[%d]", buck_ctrl->buck_info.en_pin);

	gpio_free(buck_ctrl->buck_info.en_pin);

	if (state == POWER_OFF)
		return 0;

	/* software enable buck */
	if (buck_ctrl->buck_info.pin_vsel == PIN_VSEL_LOW)
		/* low->vsel0 en */
		ret = fan53526_set_voltage(buck_ctrl,
			REG_VSEL0, voltage, state);
	else
		/* low->vsel1 en */
		ret = fan53526_set_voltage(buck_ctrl,
			REG_VSEL1, voltage, state);

	return ret;
}

static int fan53526_init(struct hw_buck_ctrl_t *buck_ctrl)
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

static int fan53526_exit(struct hw_buck_ctrl_t *buck_ctrl)
{
	int ret;
	cam_info("%s enter", __func__);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "fan53526");
	if (ret < 0) {
		cam_err("failed to request gpio[%d]", buck_ctrl->buck_info.en_pin);
		return -1;
	}
	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, 0); /*pull low*/
	if (ret < 0)
		cam_err("failed to set gpio high[%d]", buck_ctrl->buck_info.en_pin);

	gpio_free(buck_ctrl->buck_info.en_pin);
	return 0;
}

static struct hw_buck_i2c_client fan53526_i2c_client;

static struct hw_buck_fn_t fan53526_func_tbl = {
	.buck_power_config = fan53526_power_config,
	.buck_get_dt_data = fan53526_get_dt_data,
	.buck_match = fan53526_match,
	.buck_init = fan53526_init,
	.buck_exit = fan53526_exit,
};

/* driver_data */
static struct hw_buck_ctrl_t fan53526_ctrl = {
	.buck_i2c_client = &fan53526_i2c_client,
	.func_tbl = &fan53526_func_tbl,
	.buck_mutex = &buck_mut_fan53526,
};

static const struct i2c_device_id fan53526_id[] = {
	/* name, driver_data */
	{ "fan53526", (unsigned long)&fan53526_ctrl },
	{}
};

static const struct of_device_id fan53526_dt_match[] = {
	{.compatible = "huawei,fan53526"},
	{}
};

MODULE_DEVICE_TABLE(of, fan53526_dt_match);

static struct i2c_driver fan53526_i2c_driver = {
	.probe    = hw_buck_i2c_probe,
	.remove   = hw_buck_remove,
	.id_table = fan53526_id,
	.driver = {
		.name           = "fan53526",
		.of_match_table = fan53526_dt_match,
	},
};

static int __init fan53526_module_init(void)
{
	cam_info("%s enter", __func__);
	return i2c_add_driver(&fan53526_i2c_driver);
}

static void __exit fan53526_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&fan53526_i2c_driver);
}

module_init(fan53526_module_init);
module_exit(fan53526_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("FAN53526 BUCK");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
