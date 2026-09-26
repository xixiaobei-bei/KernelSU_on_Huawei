/*
 * head file for rotary crown driver
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

#ifndef __ROTARY_CROWN_H__
#define __ROTARY_CROWN_H__

#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/time.h>
#include <linux/cdev.h>
#include <linux/fb.h>
#include <linux/pm.h>
#include <linux/ctype.h>
#include <huawei_platform/log/hw_log.h>
#include <lcdkit_ext.h>

#ifndef NULL
#define NULL 0
#endif

// hw log define
#define HWLOG_TAG   ROTARY_CROWN

HWLOG_REGIST();

#define rc_debug(x...) _hwlog_debug(HWLOG_TAG, ##x)
#define rc_info(x...)  _hwlog_info(HWLOG_TAG, ##x)
#define rc_warn(x...)  _hwlog_warn(HWLOG_TAG, ##x)
#define rc_err(x...)   _hwlog_err(HWLOG_TAG, ##x)


#define NO_ERR 0

// i2c define
#define ROTARY_I2C_TYPE       "i2c_type"
#define ROTARY_I2C_MSG_LEN    2
#define ROTARY_I2C_MSG_INDEX0 0
#define ROTARY_I2C_MSG_INDEX1 1
#define I2C_WAIT_TIME         5 // 5ms wait period
#define I2C_RW_TRIES          3 // retry 3 times

// dts define
#define ROTARY_CROWN_DRV_NAME       "rotary_crown"
#define ROTARY_IRQ_GPIO             "irq_gpio"
#define ROTARY_CROWN_INPUT_DEV_NAME "rotary_crown"

// chip define
#define ROTARY_REG_ADDR_LEN         1
#define ROTARY_REG_VALUE_LEN        1
#define ROTARY_REG_WRITE_LEN        2
#define ROTARY_DELAY_1_MS           1
#define ROTARY_READ_DELTA_DELAY     10
#define ROTARY_READ_MOTION_TIMES    3
#define ROTARY_MOTION_VALID_MASK    0x80
#define ROTARY_HIGH_DELTA_SHIFT     4
#define ROTARY_HIGH_DELTA_SIGH_MASK 0x0800
#define ROTARY_HIGH_DELTA_SIGH      0xF000
#define ROTARY_HIGH_DELTA_MASK      0x0F00
#define ROTARY_STD_DELTA_VAL        532
#define ROTARY_STR_LEN              128

// register value
#define PA_DEFAULT_CHIP_ID1   0x31
#define PA_PAGE_BANK_0        0    // set PA_PAGE_BANK_CONTROL to bank 0
#define PA_CHIP_RESET         0x97 // software reset and power down mode
#define PA_CHIP_EXIT_RESET    0x17 // exit software reset
#define PA_CHIP_POWER_DOWN    0x1F // enter power down mode
#define PA_DISABLE_WR_PROTECT 0x5A // disable write protect
#define PA_ENABLE_WR_PROTECT  0x00 // enable write protect
#define PA_X_RES_VALUE        0x80 // x resolution
#define PA_Y_RES_VALUE        0x00 // y resolution
#define PA_USE_12_BIT_FMT     0x04 // usr 12-bit X/Y data format
#define PA_SET_POWER_SAVING   0x04 // set power saving
#define PA_PERF_CFG1_VAL      0x6D // trace performance config 1
#define PA_PERF_CFG2_VAL      0x00 // trace performance config 2
#define PA_PERF_CFG3_VAL      0x82 // trace performance config 3
#define PA_ENTER_SLEEP1_MODE  0xB2 // enter sleep 1 mode

// nv define
#define RC_RES_NV_DATA_SIZE 12
#define NV_READ_TIMEOUT 30
#define NV_NUMBER_VALUE 435
#define NV_READ_OPERATION 1
#define RC_NV_NAME_LEN 6
#define ROTARY_MIN_RES_X    ((PA_X_RES_VALUE * 9) / 10)
#define ROTARY_MAX_RES_X    ((PA_X_RES_VALUE * 11) / 10)

/* reg address map */
enum {
	PA_PRODUCT_ID1 = 0x00,
	PA_PRODUCT_ID2,
	PA_MOTION_STATUS,
	PA_DELTA_X_LOW,
	PA_DELTA_Y_LOW,
	PA_OPERATION_MODE,
	PA_DEVICE_CONTROL,
	PA_WRITE_PROTECT = 0x09,
	PA_SLEEP1_CONFIG,
	PA_SLEEP2_CONFIG,
	PA_RESOLUTION_X_CONFIG = 0x0D,
	PA_RESOLUTION_Y_CONFIG,
	PA_DELTA_XY_HIGH = 0x12,
	PA_FRAME_AVERAGE = 0x17,
	PA_DELTA_DATA_FORMAT = 0x19,
	PA_TRACE_PERF_CONFIG1 = 0x2B,
	PA_TRACE_PERF_CONFIG2 = 0x2D,
	PA_LOW_VOL_CONFIG = 0x4B,
	PA_TRACE_PERF_CONFIG3 = 0x7C,
	PA_PAGE_BANK_CONTROL = 0x7F,
	PA_PAGE_REG = 0xFF,
};

/* platform data that can be configured by extern */
struct rotary_crown_data {
	struct i2c_client *client;
	struct device *dev;
	struct input_dev *input_dev;
	struct work_struct work;
#if defined(CONFIG_FB)
	struct notifier_block fb_notif;
#elif defined(CONFIG_HAS_EARLYSUSPEND)
	struct early_suspend early_suspend;
#endif // CONFIG_HAS_EARLYSUSPEND
	uint16_t chip_id;
	uint8_t is_suspend;
	uint8_t irq_is_use;
	uint8_t is_one_byte;
	uint8_t is_init_ok;
	uint8_t sys_file_ok;
	uint8_t res_x_nv;
	int irq_gpio;
	int i2c_switch_gpio;
	int nv_status_count;
	struct pinctrl *rc_pinctrl;
	struct pinctrl_state *pinctrl_state_active;
	struct pinctrl_state *pinctrl_state_suspend;
};

#endif // __ROTARY_CROWN_H__
