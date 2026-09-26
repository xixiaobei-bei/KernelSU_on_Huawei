/*
 * rt5748b.c
 *
 * driver function of rt5748b
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
#define REG_CONTROL1 0x02
/* Read-only register identifies vendor and chip type */
#define REG_ID1     0x03
/* Read-only register identifies die revision */
#define REG_ID2     0x04
/* Indicates device status */
#define REG_MONITOR 0x05

/*
 * MONITOR
 * bit7: 1: Buck is enabled and soft-start is completed
 * bit6: 1: Signifies the VIN is less than the UVLO threshold
 * bit5: 1: Reserved
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

#define RT5748B_OUT_VOL_MIN 300000 /* 0.3V */
#define RT5748B_OUT_VOL_MAX 1300000 /* 1.3V */
#define RT5748B_VOL_STEP    5000 /* 5mV step */

static struct mutex buck_mut_rt5748b = __MUTEX_INITIALIZER(buck_mut_rt5748b);

static int rt5748b_get_dt_data(struct hw_buck_ctrl_t *buck_ctrl)
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

static int rt5748b_match(struct hw_buck_ctrl_t *buck_ctrl)
{
	cam_info("%s enter", __func__);
	return 0;
}

static u32 rt5748b_calc_voltage_level(u32 voltage)
{
	u32 level; /* NSELx */

	if (voltage > RT5748B_OUT_VOL_MAX)
		level = RT5748B_OUT_VOL_MAX / RT5748B_VOL_STEP;
	else if (voltage < RT5748B_OUT_VOL_MIN)
		level = 0;
	else
		level = (voltage - RT5748B_OUT_VOL_MIN) / RT5748B_VOL_STEP;
	return level;
}

int rt5748b_set_voltage(struct hw_buck_ctrl_t *buck_ctrl,
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
		reg_vsel_value = 0x0;
	} else {
		/* Vout = 0.300V+NSELx*5mV */
		reg_vsel_value = rt5748b_calc_voltage_level(voltage);
	}
	cam_info("%s enter, voltage:%u, reg_vsel_value:%u", __func__, voltage, reg_vsel_value);
	ret = i2c_func->i2c_write(i2c_client, reg_vsel, reg_vsel_value);
	if (ret < 0) {
		cam_err("%s: write reg_vsel failed, ret = %d", __func__, ret);
		return -1;
	}
	i2c_func->i2c_read(i2c_client, REG_MONITOR, &reg_value);

	cam_info("%s monitor reg =0x%x, ret:%d", __func__, reg_value, ret);
	return 0;
}

static int rt5748b_power_config(struct hw_buck_ctrl_t *buck_ctrl,
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

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "rt5748b");
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

	hw_camdrv_msleep(1);
	/* software enable buck */
	if (buck_ctrl->buck_info.pin_vsel == PIN_VSEL_LOW)
		/* low->vsel0 en */
		ret = rt5748b_set_voltage(buck_ctrl, REG_VSEL0, voltage, state);
	else
		/* low->vsel1 en */
		ret = rt5748b_set_voltage(buck_ctrl, REG_VSEL1, voltage, state);

	return ret;
}

static int rt5748b_init(struct hw_buck_ctrl_t *buck_ctrl)
{
	struct hw_buck_i2c_client *i2c_client = NULL;
	struct hw_buck_i2c_fn_t *i2c_func = NULL;
	u8 reg_value = 0;
	int ret = 0;

	cam_info("%s enter", __func__);

	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "rt5748b");
	if (ret < 0) {
		cam_err("failed to request gpio[%d]", buck_ctrl->buck_info.en_pin);
		return ret;
	}
	ret = gpio_direction_output(buck_ctrl->buck_info.en_pin, 1); /*pull high*/
	if (ret < 0) {
		cam_err("failed to set gpio high[%d]", buck_ctrl->buck_info.en_pin);
		gpio_free(buck_ctrl->buck_info.en_pin);
		return ret;
	}
	hw_camdrv_msleep(1);

	i2c_client = buck_ctrl->buck_i2c_client;
	i2c_func = buck_ctrl->buck_i2c_client->i2c_func_tbl;
	ret = i2c_func->i2c_read(i2c_client, REG_CONTROL1, &reg_value);
	if (ret < 0) {
		cam_err("%s: read CHIP ID failed, ret = %d", __func__, ret);
		goto EXIT;
	}
	cam_info("%s chip id=0x%x", __func__, reg_value);

	ret = i2c_func->i2c_read(i2c_client, REG_ID2, &reg_value);
	if (ret < 0) {
		cam_err("%s: read version failed, ret = %d", __func__, ret);
		goto EXIT;
	}

	ret = i2c_func->i2c_read(i2c_client, REG_MONITOR, &reg_value);
	if (ret < 0) {
		cam_err("%s: read monitor failed, ret = %d", __func__, ret);
		goto EXIT;
	}
	cam_info("%s monitor=0x%x", __func__, reg_value);

	ret = i2c_func->i2c_read(i2c_client, REG_VSEL0, &reg_value);
	if (ret < 0) {
		cam_err("%s: read reg_vsel0 failed, ret = %d", __func__, ret);
		goto EXIT;
	}
	cam_info("%s reg_vsel0=0x%x", __func__, reg_value);
	ret = i2c_func->i2c_read(i2c_client, REG_VSEL1, &reg_value);
	if (ret < 0) {
		cam_err("%s: read reg_vsel1 failed, ret = %d", __func__, ret);
		goto EXIT;
	}

EXIT:
	if (gpio_direction_output(buck_ctrl->buck_info.en_pin, 0))
		cam_err("%s: gpio set error",__func__);

	gpio_free(buck_ctrl->buck_info.en_pin);

	return ret;
}

static int rt5748b_exit(struct hw_buck_ctrl_t *buck_ctrl)
{
	int ret;

	cam_info("%s enter", __func__);
	if (!buck_ctrl) {
		cam_err("%s buck_ctrl is NULL", __func__);
		return -1;
	}

	ret = gpio_request(buck_ctrl->buck_info.en_pin, "rt5748b");
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


static struct hw_buck_i2c_client rt5748b_i2c_client;


static struct hw_buck_fn_t rt5748b_func_tbl = {
	.buck_power_config = rt5748b_power_config,
	.buck_get_dt_data = rt5748b_get_dt_data,
	.buck_match = rt5748b_match,
	.buck_init = rt5748b_init,
	.buck_exit = rt5748b_exit,
};

/* driver_data */
static struct hw_buck_ctrl_t rt5748b_ctrl = {
	.buck_i2c_client = &rt5748b_i2c_client,
	.func_tbl = &rt5748b_func_tbl,
	.buck_mutex = &buck_mut_rt5748b,
};

static const struct i2c_device_id rt5748b_id[] = {
	/* name, driver_data */
	{"rt5748b", (unsigned long)&rt5748b_ctrl },
	{}
};

static const struct of_device_id rt5748b_dt_match[] = {
	{.compatible = "huawei,rt5748b"},
	{}
};

MODULE_DEVICE_TABLE(of, rt5748b_dt_match);

static struct i2c_driver rt5748b_i2c_driver = {
	.probe    = hw_buck_i2c_probe,
	.remove   = hw_buck_remove,
	.id_table = rt5748b_id,
	.driver = {
		.name           = "rt5748b",
		.of_match_table = rt5748b_dt_match,
	},
};

static int __init rt5748b_module_init(void)
{
	cam_info("%s enter", __func__);
	return i2c_add_driver(&rt5748b_i2c_driver);
}

static void __exit rt5748b_module_exit(void)
{
	cam_info("%s enter", __func__);
	i2c_del_driver(&rt5748b_i2c_driver);
}

module_init(rt5748b_module_init);
module_exit(rt5748b_module_exit);

MODULE_DESCRIPTION("RT5748B BUCK");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
