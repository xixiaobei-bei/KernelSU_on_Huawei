/*
 * nt50356.h
 *
 * bias+backlight driver of NT50356
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
#ifndef __NT50356_H
#define __NT50356_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

#define NT50356_NAME                        "nt50356"
#define DTS_COMP_NT50356                    "nt,nt50356"
#define NT50356_HIDDEN_REG_SUPPORT          "nt50356_hidden_reg_support"
#define NT50356_HW_ENABLE                   "nt50356_hw_enable"
#define NT50356_HW_EN_GPIO                  "nt50356_hw_en_gpio"
#define NT50356_HW_EN_DELAY                 "bl_on_kernel_mdelay"
#define NT50356_PULL_BOOST_SUPPORT          "nt50356_pull_boost_support"
#define NT50356_PULL_DOWN_BOOST             "nt50356_pull_down_boost"
#define NT50356_ENABLE_VSP_VSP              "nt50356_enable_vsp_vsn"
#define NT50356_PULL_UP_BOOST               "nt50356_pull_up_boost"
#define NT50356_VTC_LINE_BOOST              "nt50356_vtc_line_boost"
#define NT50356_ONLY_BACKLIGHT "nt50356_only_backlight"
#define NT50356_HW_EN_PULL_LOW "nt50356_hw_en_pull_low"
#define NT50356_VTC_END_BOOST               "nt50356_vtc_end_boost"

#define GPIO_DIR_OUT                        1
#define GPIO_OUT_ONE                        1
#define GPIO_OUT_ZERO                       0
#define MAX_RATE_NUM                        9

/* base reg */
#define REG_REVISION                        0x01
#define REG_BL_CONFIG_1                     0x02
#define REG_BL_CONFIG_2                     0x03
#define REG_BL_BRIGHTNESS_LSB               0x04
#define REG_BL_BRIGHTNESS_MSB               0x05
#define REG_AUTO_FREQ_LOW                   0x06
#define REG_AUTO_FREQ_HIGH                  0x07
#define REG_BL_ENABLE                       0x08
#define REG_DISPLAY_BIAS_CONFIG_1           0x09
#define REG_DISPLAY_BIAS_CONFIG_2           0x0A
#define REG_DISPLAY_BIAS_CONFIG_3           0x0B
#define REG_LCM_BOOST_BIAS                  0x0C
#define REG_VPOS_BIAS                       0x0D
#define REG_VNEG_BIAS                       0x0E
#define REG_FLAGS                           0x0F
#define REG_BL_OPTION_1                     0x10
#define REG_BL_OPTION_2                     0x11
#define REG_BL_CURRENT_CONFIG               0x20
#define REG_MAX                             0x21
#define REG_HIDDEN_ADDR                     0x6A
#define REG_SET_SECURITYBIT_ADDR            0x50
#define REG_SET_SECURITYBIT_VAL             0x08
#define REG_CLEAR_SECURITYBIT_VAL           0x00
/* add config reg for nt50356 TSD bug */
#define NT50356_REG_CONFIG_A9               0xA9
#define NT50356_REG_CONFIG_E8               0xE8
#define NT50356_REG_CONFIG_E9               0xE9

#define NT50356_VOL_400 (0x00)
#define NT50356_VOL_405 (0x01)
#define NT50356_VOL_410 (0x02)
#define NT50356_VOL_415 (0x03)
#define NT50356_VOL_420 (0x04)
#define NT50356_VOL_425 (0x05)
#define NT50356_VOL_430 (0x06)
#define NT50356_VOL_435 (0x07)
#define NT50356_VOL_440 (0x08)
#define NT50356_VOL_445 (0x09)
#define NT50356_VOL_450 (0x0A)
#define NT50356_VOL_455 (0x0B)
#define NT50356_VOL_460 (0x0C)
#define NT50356_VOL_465 (0x0D)
#define NT50356_VOL_470 (0x0E)
#define NT50356_VOL_475 (0x0F)
#define NT50356_VOL_480 (0x10)
#define NT50356_VOL_485 (0x11)
#define NT50356_VOL_490 (0x12)
#define NT50356_VOL_495 (0x13)
#define NT50356_VOL_500 (0x14)
#define NT50356_VOL_505 (0x15)
#define NT50356_VOL_510 (0x16)
#define NT50356_VOL_515 (0x17)
#define NT50356_VOL_520 (0x18)
#define NT50356_VOL_525 (0x19)
#define NT50356_VOL_530 (0x1A)
#define NT50356_VOL_535 (0x1B)
#define NT50356_VOL_540 (0x1C)
#define NT50356_VOL_545 (0x1D)
#define NT50356_VOL_550 (0x1E)
#define NT50356_VOL_555 (0x1F)
#define NT50356_VOL_560 (0x20)
#define NT50356_VOL_565 (0x21)
#define NT50356_VOL_570 (0x22)
#define NT50356_VOL_575 (0x23)
#define NT50356_VOL_580 (0x24)
#define NT50356_VOL_585 (0x25)
#define NT50356_VOL_590 (0x26)
#define NT50356_VOL_595 (0x27)
#define NT50356_VOL_600 (0x28)
#define NT50356_VOL_605 (0x29)
#define NT50356_VOL_640 (0x30)
#define NT50356_VOL_645 (0x31)
#define NT50356_VOL_650 (0x32)
#define NT50356_VOL_655 (0x33)
#define NT50356_VOL_660 (0x34)
#define NT50356_VOL_665 (0x35)
#define NT50356_VOL_670 (0x36)
#define NT50356_VOL_675 (0x37)
#define NT50356_VOL_680 (0x38)
#define NT50356_VOL_685 (0x39)
#define NT50356_VOL_690 (0x3A)
#define NT50356_VOL_695 (0x3B)
#define NT50356_VOL_700 (0x3C)


/* mask code */
#define MASK_BL_LSB                         0x07
#define MASK_LCM_EN                         0xE0
#define MASK_SOFTWARE_RESET                 0x80

/* update bit val */
#define DEVICE_FAULT_OCCUR                  0
#define DEVICE_RESET                        0x1

#define BL_MIN                              0
#define BL_MAX                              2047
#define MSB                                 3
#define LSB                                 0x07

#define MIN_BL_RESUME_TIMMER                1
#define MAX_BL_RESUME_TIMMER                400
#define PROTECT_BL_RESUME_TIMMER            28
#define NT50356_IC_DELAY                    16

#define LOG_LEVEL_INFO                      8
#define FLAG_CHECK_NUM                      3
#define NT50356_RW_REG_MAX 19
#define BL_CONFIG_MODE_REG_NUM              3
#define BL_CONFIG_CURR_REG_NUM              2
#define BL_CONFIG_ENABLE_REG_NUM            1
#define BL_LOWER_POW_DELAY                  6
#define BL_MAX_PREFLASHING_TIMER            800

#define BL_POWER_ON 0x0
#define BL_POWER_OFF 0x1
#define BL_CONFIG_LEVEL_POWER_ON 0x2
#define BL_DIMMING_SUPPORT 1

/* bl_mode_config reg */
#define BL_MAX_CONFIG_REG_NUM               3

#define nt50356_emerg(msg, ...) \
	do { if (nt50356_msg_level > 0) \
			pr_emerg("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_alert(msg, ...) \
	do { if (nt50356_msg_level > 1) \
			pr_alert("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_crit(msg, ...) \
	do { if (nt50356_msg_level > 2) \
			pr_crit("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_err(msg, ...) \
	do { if (nt50356_msg_level > 3) \
			pr_err("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_warning(msg, ...) \
	do { if (nt50356_msg_level > 4) \
			pr_warn("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_notice(msg, ...) \
	do { if (nt50356_msg_level > 5) \
			pr_notice("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_info(msg, ...) \
	do { if (nt50356_msg_level > 6) \
			pr_info("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define nt50356_debug(msg, ...) \
	do { if (nt50356_msg_level > 7) \
			pr_devel("[nt50356]%s: " msg, __func__, ##__VA_ARGS__); } while (0)

struct nt50356_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};

enum {
	OVP_FAULT_FLAG = 0x02,
	OCP_FAULT_FLAG = 0x01,
	TSD_FAULT_FLAG = 0x40,
};

struct nt50356_check_flag {
	u8 flag;
	int err_no;
};

struct nt50356_backlight_information {
	int nt50356_pull_boost_support;
	int nt50356_pull_down_boost;
	int nt50356_pull_up_boost;
	int nt50356_enable_vsp_vsn;
	int nt50356_vtc_line_boost;
	int nt50356_vtc_end_boost;
	int nt50356_hw_en_gpio;
	int nt50356_hw_en;
	int bl_on_kernel_mdelay;
	int bl_only;
	int nt50356_hw_en_pull_low;
	int bl_dimming_support;
	int bl_dimming_config;
	int bl_dimming_resume;
};

struct nt50356_voltage {
	u32 voltage;
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

int nt50356_set_backlight(uint32_t bl_level);
ssize_t nt50356_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val);

#endif /* __NT50356_H */

