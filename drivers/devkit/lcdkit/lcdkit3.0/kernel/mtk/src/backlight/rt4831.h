/*
 * rt4831.h
 *
 * bias+backlight driver of RT4831
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
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>


#ifndef __RT4831_H
#define __RT4831_H

#define RT4831_NAME "rt4831"
#define DTS_COMP_RT4831 "rt,rt4831"
#define RT4831_HIDDEN_REG_SUPPORT "rt4831_hidden_reg_support"
#define RT4831_HW_ENABLE "rt4831_hw_enable"
#define RT4831_HW_EN_GPIO "rt4831_hw_en_gpio"
#define RT4831_HW_EN_DELAY "bl_on_kernel_mdelay"
#define RT4831_PULL_BOOST_SUPPORT "rt4831_pull_boost_support"
#define RT4831_PULL_DOWN_BOOST "rt4831_pull_down_boost"
#define RT4831_ENABLE_VSP_VSP "rt4831_enable_vsp_vsn"
#define RT4831_PULL_UP_BOOST "rt4831_pull_up_boost"
#define RT4831_VTC_LINE_BOOST "rt4831_vtc_line_boost"
#define RT4831_VTC_END_BOOST "rt4831_vtc_end_boost"
#define RT4831_ONLY_BACKLIGHT "rt4831_only_backlight"
#define RT4831_HW_EN_PULL_LOW "rt4831_hw_en_pull_low"

#define GPIO_DIR_OUT 1
#define GPIO_OUT_ONE 1
#define GPIO_OUT_ZERO 0
#define MAX_RATE_NUM 9
/* base reg */
#define REG_REVISION 0x01
#define REG_BL_CONFIG_1 0x02
#define REG_BL_CONFIG_2 0x03
#define REG_BL_BRIGHTNESS_LSB 0x04
#define REG_BL_BRIGHTNESS_MSB 0x05
#define REG_AUTO_FREQ_LOW 0x06
#define REG_AUTO_FREQ_HIGH 0x07
#define REG_RT4831_REG_HIDDEN_F0 0xF0
#define REG_RT4831_REG_HIDDEN_B1 0xB1
#define REG_BL_ENABLE 0x08
#define REG_DISPLAY_BIAS_CONFIG_1 0x09
#define REG_DISPLAY_BIAS_CONFIG_2 0x0A
#define REG_DISPLAY_BIAS_CONFIG_3 0x0B
#define REG_LCM_BOOST_BIAS 0x0C
#define REG_VPOS_BIAS 0x0D
#define REG_VNEG_BIAS 0x0E
#define REG_FLAGS 0x0F
#define REG_BL_OPTION_1 0x10
#define REG_BL_OPTION_2 0x11
#define REG_PWM_TO_DIGITAL_LSB 0x12
#define REG_PWM_TO_DIGITAL_MSB 0x13
#define REG_MAX 0x14
#define REG_HIDDEN_ADDR 0x6A
#define REG_SET_SECURITYBIT_ADDR 0x50
#define REG_SET_SECURITYBIT_VAL 0x08
#define REG_CLEAR_SECURITYBIT_VAL 0x00

#define RT4831_VOL_400 (0x00)
#define RT4831_VOL_405 (0x01)
#define RT4831_VOL_410 (0x02)
#define RT4831_VOL_415 (0x03)
#define RT4831_VOL_420 (0x04)
#define RT4831_VOL_425 (0x05)
#define RT4831_VOL_430 (0x06)
#define RT4831_VOL_435 (0x07)
#define RT4831_VOL_440 (0x08)
#define RT4831_VOL_445 (0x09)
#define RT4831_VOL_450 (0x0A)
#define RT4831_VOL_455 (0x0B)
#define RT4831_VOL_460 (0x0C)
#define RT4831_VOL_465 (0x0D)
#define RT4831_VOL_470 (0x0E)
#define RT4831_VOL_475 (0x0F)
#define RT4831_VOL_480 (0x10)
#define RT4831_VOL_485 (0x11)
#define RT4831_VOL_490 (0x12)
#define RT4831_VOL_495 (0x13)
#define RT4831_VOL_500 (0x14)
#define RT4831_VOL_505 (0x15)
#define RT4831_VOL_510 (0x16)
#define RT4831_VOL_515 (0x17)
#define RT4831_VOL_520 (0x18)
#define RT4831_VOL_525 (0x19)
#define RT4831_VOL_530 (0x1A)
#define RT4831_VOL_535 (0x1B)
#define RT4831_VOL_540 (0x1C)
#define RT4831_VOL_545 (0x1D)
#define RT4831_VOL_550 (0x1E)
#define RT4831_VOL_555 (0x1F)
#define RT4831_VOL_560 (0x20)
#define RT4831_VOL_565 (0x21)
#define RT4831_VOL_570 (0x22)
#define RT4831_VOL_575 (0x23)
#define RT4831_VOL_580 (0x24)
#define RT4831_VOL_585 (0x25)
#define RT4831_VOL_590 (0x26)
#define RT4831_VOL_595 (0x27)
#define RT4831_VOL_600 (0x28)
#define RT4831_VOL_605 (0x29)
#define RT4831_VOL_640 (0x30)
#define RT4831_VOL_645 (0x31)
#define RT4831_VOL_650 (0x32)

/* mask code */
#define MASK_BL_LSB 0x07
#define MASK_LCM_EN 0xE0
#define MASK_SOFTWARE_RESET 0x80

/* update bit val */
#define DEVICE_FAULT_OCCUR 0
#define DEVICE_RESET 0x1

#define BL_MIN 0
#define BL_MAX 2047
#define MSB 3
#define LSB 0x07
#define MIN_BL_RESUME_TIMMER 1
#define MAX_BL_RESUME_TIMMER 400
#define PROTECT_BL_RESUME_TIMMER 28
#define RT4831_IC_DELAY 16

#define BL_POWER_ON 0x0
#define BL_POWER_OFF 0x1
#define BL_CONFIG_LEVEL_POWER_ON 0x2
#define BL_DIMMING_SUPPORT 1

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#define LOG_LEVEL_INFO 8

#define RT4831_EMERG(msg, ...) \
	do { \
		if (rt4831_msg_level > 0) \
			pr_emerg("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_ALERT(msg, ...) \
	do { \
		if (rt4831_msg_level > 1) \
			pr_alert("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_CRIT(msg, ...) \
	do { \
		if (rt4831_msg_level > 2) \
			pr_crit("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_ERR(msg, ...) \
	do { \
		if (rt4831_msg_level > 3) \
			pr_err("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_WARNING(msg, ...) \
	do { \
		if (rt4831_msg_level > 4) \
			pr_warn("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_NOTICE(msg, ...) \
	do { \
		if (rt4831_msg_level > 5) \
			pr_notice("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_INFO(msg, ...) \
	do { \
		if (rt4831_msg_level > 6) \
			pr_info("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)
#define RT4831_DEBUG(msg, ...) \
	do { \
		if (rt4831_msg_level > 7) \
			pr_devel("[rt4831]%s: " msg, __func__, ##__VA_ARGS__); \
	} while (0)

struct rt4831_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

#define FLAG_CHECK_NUM 3

enum {
	OVP_FAULT_FLAG = 0x02,
	OCP_FAULT_FLAG = 0x01,
	TSD_FAULT_FLAG = 0x40,
};

struct rt4831_check_flag {
	unsigned char flag;
	int err_no;
};

#define RT4831_RW_REG_MAX 17

struct rt4831_backlight_information {
	int rt4831_pull_boost_support;
	int rt4831_pull_down_boost;
	int rt4831_pull_up_boost;
	int rt4831_enable_vsp_vsn;
	int rt4831_vtc_line_boost;
	int rt4831_vtc_end_boost;
	int rt4831_hw_en_gpio;
	int rt4831_hw_en;
	int bl_on_kernel_mdelay;
	int bl_only;
	int rt4831_hw_en_pull_low;
	int bl_dimming_support;
	int bl_dimming_config;
	int bl_dimming_resume;
};

struct rt4831_voltage {
	unsigned int voltage;
	int value;
};

enum bl_enable {
	EN_2_SINK = 0x15,
	EN_4_SINK = 0x1F,
	BL_RESET = 0x80,
	BL_DISABLE = 0x00,
};

enum lcm_en {
	NORMAL_MODE = 0x80,
	BIAS_SUPPLY_OFF = 0x00,
};

enum {
	BL_OVP_25V = 0x40,
	BL_OVP_29V = 0x60,
};

enum {
	CURRENT_RAMP_0US = 0x85,
	CURRENT_RAMP_5MS = 0xAD,
};

enum {
	LSB_MIN = 0x00,
	LSB_10MA = 0x05,
	LSB_30MA = 0x07,
};

enum {
	MSB_MIN = 0x00,
	MSB_10MA = 0xD2,
	MSB_30MA = 0xFF,
};

enum bl_option_2 {
	BL_OCP_1 = 0x35, /* 1.2A */
	BL_OCP_2 = 0x36, /* 1.5A */
	BL_OCP_3 = 0x37, /* 1.8A */
};

enum {
	BIAS_BOOST_TIME_0 = 0x00, /* 156ns */
	BIAS_BOOST_TIME_1 = 0x10, /* 181ns */
	BIAS_BOOST_TIME_2 = 0x20, /* 206ns */
	BIAS_BOOST_TIME_3 = 0x30, /* 231ns */
};

enum resume_type {
	RESUME_IDLE = 0x0,
	RESUME_2_SINK = 0x1,
	RESUME_REMP_OVP_OCP = 0x2,
};

#define BL_CONFIG_MODE_REG_NUM                  3
#define BL_CONFIG_CURR_REG_NUM                  2
#define BL_CONFIG_ENABLE_REG_NUM                1
#define BL_LOWER_POW_DELAY                      6
#define BL_MAX_PREFLASHING_TIMER                800

/* bl_mode_config reg */
#define BL_MAX_CONFIG_REG_NUM                   3

int rt4831_set_backlight_reg(uint32_t bl_level);
ssize_t rt4831_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val);

#endif /* __LINUX_RT4831_H */

