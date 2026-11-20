/*
 * Linux kernel modules for VI5300 FlightSense TOF sensor
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

#ifndef VI5300_H
#define VI5300_H

#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <media/huawei/laser_cfg.h>
#include "vi5300_def.h"

#define VI5300_CHIP_ADDR 0xD8

#define VI5300_REG_MCU_CFG 0x00
#define VI5300_REG_SYS_CFG 0x01
#define VI5300_REG_DEV_STAT 0x02
#define VI5300_REG_INTR_STAT 0x03
#define VI5300_REG_INTR_MASK 0x04
#define VI5300_REG_DEV_ADDR 0x06
#define VI5300_REG_PW_CTRL 0x07
#define VI5300_REG_SPCIAL_PURP 0x08
#define VI5300_REG_CMD 0x0A
#define VI5300_REG_SIZE 0x0B
#define VI5300_REG_SCRATCH_PAD_BASE 0x0C
#define VI5300_REG_DIGLDO_VREF 0x38
#define VI5300_REG_ANALDO_VREF 0x3A
#define VI5300_REG_PD_RESET 0x3B
#define VI5300_REG_RCO_VREF 0xE9
#define VI5300_REG_RCO_TEMP 0xEE
#define VI5300_REG_VCSELDRV_PULSE 0xF5


#define VI5300_WRITEFW_CMD 0x03
#define VI5300_USER_CFG_CMD 0x09
#define VI5300_XTALK_TRIM_CMD 0x0D
#define VI5300_START_RANG_CMD 0x0E

#define VI5300_XTALKR_SUBCMD 0x00
#define VI5300_XTALKW_SUBCMD 0x01
#define VI5300_OTPW_SUBCMD 0x02
#define VI5300_OTPR_SUBCMD 0x03
#define VI5300_XTALK_ADDR 0x00
#define VI5300_XTALK_SIZE 1
#define VI5300_MAX_WAIT_RETRY 5
#define DEFAULT_INTEGRAL_COUNTS 65535
#define DEFAULT_FRAME_COUNTS 30

#define VI5300_ERROR_NONE ((VI5300_Error) 0)
#define VI5300_ERROR_GPIO_ERROR ((VI5300_Error) -1)
#define VI5300_ERROR_INIT_ERROR ((VI5300_Error) -2)
#define VI5300_ERROR_XTALK_CALIB ((VI5300_Error) -3)
#define VI5300_ERROR_OFFSET_CALIB ((VI5300_Error) -4)
#define VI5300_ERROR_XTALK_CONFIG ((VI5300_Error) -5)
#define VI5300_ERROR_SINGLE_CMD ((VI5300_Error) -6)
#define VI5300_ERROR_CONTINUOUS_CMD ((VI5300_Error) -7)
#define VI5300_ERROR_GET_DATA ((VI5300_Error) -8)
#define VI5300_ERROR_STOP_CMD ((VI5300_Error) -9)
#define VI5300_ERROR_IRQ_STATE ((VI5300_Error) -10)
#define VI5300_ERROR_FW_FAILURE ((VI5300_Error) -11)
#define VI5300_ERROR_INTEGRAL_COUNT ((VI5300_Error) -12)
#define VI5300_ERROR_DELAY_COUNT ((VI5300_Error) -13)
#define VI5300_ERROR_FPS_ZERO ((VI5300_Error) -14)

struct VI5300_XTALK_Calib_Data {
	int8_t xtalk_cal;
	uint16_t xtalk_peak;
	int16_t xtalk_tof;
};

struct VI5300_OFFSET_Calib_Data {
	int16_t offset_cal;
};

struct vi5300_data {
	struct device *dev;
	const char *dev_name;
	hw_laser_ctrl_t *ctrl;
	struct VI5300_Measurement_Data RangData;
	struct VI5300_XTALK_Calib_Data XtalkData;
	struct VI5300_OFFSET_Calib_Data OffsetData;
	struct mutex update_lock;
	struct delayed_work dwork;
	struct kobject *range_kobj;
	struct i2c_client *client;
	struct miscdevice miscdev;
	int irq_gpio;
	int xshut_gpio;
	int irq;
	uint8_t firmware_status;
	uint8_t interrupt_status;
	uint8_t status;
	int8_t xtalk_config;
	int16_t offset_config;
	uint8_t xtalk_irq_received;
	uint8_t intr_status;
	uint32_t integral_counts;
	uint8_t frame_counts;
	struct mutex work_mutex;
 };

enum IRQ_STATUS {
	VI5300_IRQ_ENABLE = 0,
	VI5300_IRQ_DISABLE,
};

#define DEBUG
#define vi5300_infomsg(str, args...) \
	 pr_info("%s: " str, __func__, ##args)
#define vi5300_dbgmsg(str, args...) \
	 pr_debug("%s: " str, __func__, ##args)
#define vi5300_errmsg(str, args...) \
	 pr_err("%s: " str, __func__, ##args)

#endif
