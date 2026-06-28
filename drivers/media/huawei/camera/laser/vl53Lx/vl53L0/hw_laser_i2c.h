/*
 *  stmvl53l0-i2c.h - Linux kernel modules for STM VL53L0 FlightSense TOF sensor
 *
 *  Copyright (C) 2016 STMicroelectronics Imaging Division
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef HW_LASER_I2C_H
#define HW_LASER_I2C_H
#include <linux/pwm.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/of_gpio.h>
#include <media/huawei/laser_cfg.h>
#include "laser_module.h"



#define VL53L0_CHIP_ID      0xC0
#define GP2AP01_CHIP_ID      0x3B
#define CHIP_NAME_LEN 32
#define POWER_SLEEP_TIME 2
struct i2c_data {
    struct i2c_client *client;
    struct regulator_bulk_data ldo_avdd;
    struct pinctrl	*pinctrl;
    struct pinctrl_state	*pins_default;
    struct pinctrl_state	*pins_idle;
    //char chip_name[CHIP_NAME_LEN];
    uint8_t power_up;
    int irq;
    unsigned int xsdn_gpio;
    unsigned int intr_gpio;
    struct i2d_data_flags_t {
        unsigned pwr_owned:1; /*!< set if pwren gpio is owned*/
        unsigned xsdn_owned:1; /*!< set if sxdn  gpio is owned*/
        unsigned intr_owned:1; /*!< set if intr  gpio is owned*/
        unsigned intr_started:1; /*!< set if irq is hanlde  */
    } io_flag;
    hw_laser_ctrl_t *ctrl;
};

#endif /* HW_LASER_I2C_H */
