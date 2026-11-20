/*
 * ktz8864.h
 *
 * bias+backlight driver of KTZ8864
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
#ifndef __KTZ8864_H
#define __KTZ8864_H

#include <linux/module.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/i2c.h>

#define KTZ8864_NAME                     "ktz8864"
#define DTS_COMP_KTZ8864                 "ktz,ktz8864"
#define KTZ8864_HIDDEN_REG_SUPPORT       "ktz8864_hidden_reg_support"
#define KTZ8864_SUPPORT                  "ktz8864_support"
#define KTZ8864_I2C_BUS_ID               "ktz8864_i2c_bus_id"
#define KTZ8864_HW_ENABLE                "ktz8864_hw_enable"
#define KTZ8864_HW_EN_GPIO               "ktz8864_hw_en_gpio"
#define KTZ8864_HW_EN_DELAY              "bl_on_kernel_mdelay"
#define KTZ8864_PULL_BOOST_SUPPORT       "ktz8864_pull_boost_support"
#define KTZ8864_PULL_DOWN_BOOST          "ktz8864_pull_down_boost"
#define KTZ8864_ENABLE_VSP_VSP           "ktz8864_enable_vsp_vsn"
#define KTZ8864_PULL_UP_BOOST            "ktz8864_pull_up_boost"
#define KTZ8864_BL_LEVEL                 "bl_level"
#define GPIO_KTZ8864_EN_NAME             "ktz8864_hw_en"
#define KTZ8864_ONLY_BACKLIGHT "ktz8864_only_backlight"
#define KTZ8864_HW_EN_PULL_LOW "ktz8864_hw_en_pull_low"

#define GPIO_DIR_OUT                     1
#define GPIO_OUT_ONE                     1
#define GPIO_OUT_ZERO                    0
#define MAX_RATE_NUM                     9
/* base reg */
#define REG_REVISION                     0x01
#define REG_BL_CONFIG_1                  0x02
#define REG_BL_CONFIG_2                  0x03
#define REG_BL_BRIGHTNESS_LSB            0x04
#define REG_BL_BRIGHTNESS_MSB            0x05
#define REG_AUTO_FREQ_LOW                0x06
#define REG_AUTO_FREQ_HIGH               0x07
#define REG_BL_ENABLE                    0x08
#define REG_DISPLAY_BIAS_CONFIG_1        0x09
#define REG_DISPLAY_BIAS_CONFIG_2        0x0A
#define REG_DISPLAY_BIAS_CONFIG_3        0x0B
#define REG_LCM_BOOST_BIAS               0x0C
#define REG_VPOS_BIAS                    0x0D
#define REG_VNEG_BIAS                    0x0E
#define REG_FLAGS                        0x0F
#define REG_BL_OPTION_1                  0x10
#define REG_BL_OPTION_2                  0x11
#define REG_PWM_TO_DIGITAL_LSB           0x12
#define REG_PWM_TO_DIGITAL_MSB           0x13
#define REG_MAX                          0x14
#define REG_HIDDEN_ADDR                  0x6A
#define REG_SET_SECURITYBIT_ADDR         0x50
#define REG_SET_SECURITYBIT_VAL          0x08
#define REG_CLEAR_SECURITYBIT_VAL        0x00
#define KTZ8864_RW_REG_MAX 15

#define KTZ8864_VOL_45                   0x0A     /* 4.5V */
#define KTZ8864_VOL_46                   0x0C     /* 4.6V */
#define KTZ8864_VOL_47                   0x0E     /* 4.7V */
#define KTZ8864_VOL_48                   0x0f     /* 4.8V */
#define KTZ8864_VOL_49                   0x12     /* 4.9V */
#define KTZ8864_VOL_50                   0x14     /* 5.0V */
#define KTZ8864_VOL_51                   0x16     /* 5.1V */
#define KTZ8864_VOL_52                   0x18     /* 5.2V */
#define KTZ8864_VOL_53                   0x1A     /* 5.3V */
#define KTZ8864_VOL_54                   0x1C     /* 5.4V */
#define KTZ8864_VOL_55                   0x1E     /* 5.5V */
#define KTZ8864_VOL_56                   0x20     /* 5.6V */
#define KTZ8864_VOL_57                   0x22     /* 5.7V */
#define KTZ8864_VOL_575                  0x23     /* 5.75V */
#define KTZ8864_VOL_58                   0x24     /* 5.8V */
#define KTZ8864_VOL_59                   0x26     /* 5.9V */
#define KTZ8864_VOL_60                   0x28     /* 6.0V */
#define KTZ8864_VOL_61                   0x2A     /* 6.1V */
#define KTZ8864_VOL_62                   0x2C     /* 6.2V */

#define MIN_BL_RESUME_TIMMER             1
#define MAX_TRY_NUM                      3
#define MAX_BL_RESUME_TIMMER             400
#define PROTECT_BL_RESUME_TIMMER         28
#define BL_LVL_MAP_SIZE_KTZ8864          (2047)
#define LCD_BL_LINEAR_EXPONENTIAL_TABLE_NUM  2048
/* mask code */
#define MASK_BL_LSB                      0x07
#define MASK_LCM_EN                      0xE0
#define MASK_SOFTWARE_RESET              0x80
/* update bit val */
#define DEVICE_FAULT_OCCUR               0
#define DEVICE_RESET                     0x1
#define BL_MIN                           0
#define BL_MAX                           2047
#define MSB                              3
#define LSB                              0x07
#define FLAG_CHECK_NUM                   3
#define LOG_LEVEL_INFO                   8

#define BL_POWER_ON 0x0
#define BL_POWER_OFF 0x1
#define BL_CONFIG_LEVEL_POWER_ON 0x2
#define BL_DIMMING_SUPPORT 1

#define ktz8864_emerg(msg, ...)    \
	do { if (ktz8864_msg_level > 0)  \
		printk(KERN_EMERG "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_alert(msg, ...)    \
	do { if (ktz8864_msg_level > 1)  \
		printk(KERN_ALERT "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_crit(msg, ...)    \
	do { if (ktz8864_msg_level > 2)  \
		printk(KERN_CRIT "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_err(msg, ...)    \
	do { if (ktz8864_msg_level > 3)  \
		printk(KERN_ERR "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_warning(msg, ...)    \
	do { if (ktz8864_msg_level > 4)  \
		printk(KERN_WARNING "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_notice(msg, ...)    \
	do { if (ktz8864_msg_level > 5)  \
		printk(KERN_NOTICE "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_info(msg, ...)    \
	do { if (ktz8864_msg_level > 6)  \
		printk(KERN_INFO "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define ktz8864_debug(msg, ...)    \
	do { if (ktz8864_msg_level > 7)  \
		printk(KERN_DEBUG "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)

struct ktz8864_chip_data {
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

struct ktz8864_check_flag {
	u8 flag;
	int err_no;
};

struct ktz8864_backlight_information {
	int ktz8864_pull_boost_support;
	int ktz8864_pull_down_boost;
	int ktz8864_pull_up_boost;
	int ktz8864_enable_vsp_vsn;
	int ktz8864_hw_en_gpio;
	int ktz8864_hw_en;
	int bl_on_kernel_mdelay;
	int bl_only;
	int ktz8864_hw_en_pull_low;
	int bl_dimming_support;
	int bl_dimming_config;
	int bl_dimming_resume;
};

struct ktz8864_voltage {
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

#define BL_CONFIG_MODE_REG_NUM           3
#define BL_CONFIG_CURR_REG_NUM           2
#define BL_CONFIG_ENABLE_REG_NUM         1
#define BL_LOWER_POW_DELAY               6
#define BL_MAX_PREFLASHING_TIMER         800

/* bl_mode_config reg */
#define BL_MAX_CONFIG_REG_NUM            3

int ktz8864_set_backlight_reg(uint32_t bl_level);
ssize_t ktz8864_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val);

#endif /* __KTZ8864_H */

