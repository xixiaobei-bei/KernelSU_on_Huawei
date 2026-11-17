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
#include <linux/i2c.h>
#include <linux/slab.h>
#if defined(CONFIG_HUAWEI_DSM)
#include <dsm/dsm_pub.h>
extern struct dsm_client *lcd_dclient;
#endif

#ifndef __LINUX_KTZ8864_H
#define __LINUX_KTZ8864_H

#define KTZ8864_NAME                     "ktz8864"
#define DTS_COMP_KTZ8864                 "ktz,ktz8864"
#define KTZ8864_HIDDEN_REG_SUPPORT       "ktz8864_hidden_reg_support"
#define KTZ8864_ONLY_BIAS                "only_bias"
#define KTZ8864_RUNNING_RESUME_BL_TIMMER "ktz8864_running_resume_bl_timmer"
#define KTZ8864_UPDATE_RESUME_BL_TIMMER  "ktz8864_update_resume_bl_timmer"
#define KTZ8864_PULL_BOOST_SUPPORT       "ktz8864_pull_boost_support"
#define KTZ8864_PULL_DOWN_BOOST          "ktz8864_pull_down_boost"
#define KTZ8864_ENABLE_VSP_VSP           "ktz8864_enable_vsp_vsn"
#define KTZ8864_PULL_UP_BOOST            "ktz8864_pull_up_boost"
#define KTZ8864_VTC_LINE_BOOST "ktz8864_vtc_line_boost"
#define BRTGHTNESS_LSB_MAX               0x07
#define BRTGHTNESS_MSB_MAX               0xFF

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
#define REG_RAMP_TIME                    0x14
#define REG_LED_CURRENT                  0x15
#define REG_MAX                          0x16
#define REG_HIDDEN_ADDR                  0x6A
#define REG_SET_SECURITYBIT_ADDR         0x50
#define REG_SET_SECURITYBIT_VAL          0x08
#define REG_CLEAR_SECURITYBIT_VAL        0x00

#define KTZ8864_VOL_45     0x0A     /* 4.5V */
#define KTZ8864_VOL_46     0x0C     /* 4.6V */
#define KTZ8864_VOL_47     0x0E     /* 4.7V */
#define KTZ8864_VOL_48     0x0f     /* 4.8V */
#define KTZ8864_VOL_49     0x12     /* 4.9V */
#define KTZ8864_VOL_50     0x14     /* 5.0V */
#define KTZ8864_VOL_51     0x16     /* 5.1V */
#define KTZ8864_VOL_52     0x18     /* 5.2V */
#define KTZ8864_VOL_53     0x1A     /* 5.3V */
#define KTZ8864_VOL_54     0x1C     /* 5.4V */
#define KTZ8864_VOL_55     0x1E     /* 5.5V */
#define KTZ8864_VOL_56     0x20     /* 5.6V */
#define KTZ8864_VOL_57     0x22     /* 5.7V */
#define KTZ8864_VOL_575    0x23     /* 5.75V */
#define KTZ8864_VOL_58     0x24     /* 5.8V */
#define KTZ8864_VOL_59     0x26     /* 5.9V */
#define KTZ8864_VOL_60     0x28     /* 6.0V */
#define KTZ8864_VOL_61     0x2A     /* 6.1V */
#define KTZ8864_VOL_62     0x2C     /* 6.2V */

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

#ifndef BIT
#define BIT(x) (1 << (x))
#endif

#define LOG_LEVEL_INFO                   8

#define KTZ8864_EMERG(msg, ...)    \
	do { if (ktz8864_msg_level > 0)  \
		printk(KERN_EMERG "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_ALERT(msg, ...)    \
	do { if (ktz8864_msg_level > 1)  \
		printk(KERN_ALERT "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_CRIT(msg, ...)    \
	do { if (ktz8864_msg_level > 2)  \
		printk(KERN_CRIT "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_ERR(msg, ...)    \
	do { if (ktz8864_msg_level > 3)  \
		printk(KERN_ERR "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_WARNING(msg, ...)    \
	do { if (ktz8864_msg_level > 4)  \
		printk(KERN_WARNING "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_NOTICE(msg, ...)    \
	do { if (ktz8864_msg_level > 5)  \
		printk(KERN_NOTICE "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_INFO(msg, ...)    \
	do { if (ktz8864_msg_level > 6)  \
		printk(KERN_INFO "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)
#define KTZ8864_DEBUG(msg, ...)    \
	do { if (ktz8864_msg_level > 7)  \
		printk(KERN_DEBUG "[ktz8864]%s: " msg, __func__, ##__VA_ARGS__); } while (0)

struct ktz8864_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
	struct work_struct bl_resume_worker;
	struct workqueue_struct *bl_resume_wq;
	struct hrtimer bl_resume_hrtimer;
};

enum ktz8864_dual_ic {
	DUAL_IC_NONE = 0,
	DUAL_IC_I3C,
	DUAL_IC_I2C,
};

#if defined(CONFIG_HUAWEI_DSM)
#define FLAG_CHECK_NUM               3

enum {
	OVP_FAULT_FLAG = 0x02,
	OCP_FAULT_FLAG = 0x01,
	TSD_FAULT_FLAG = 0x40,
};

struct ktz8864_check_flag {
	u8 flag;
	int err_no;
};

static struct ktz8864_check_flag err_table[] = {
	{OVP_FAULT_FLAG, DSM_LCD_OVP_ERROR_NO},
	{OCP_FAULT_FLAG, DSM_LCD_BACKLIGHT_OCP_ERROR_NO},
	{TSD_FAULT_FLAG, DSM_LCD_BACKLIGHT_TSD_ERROR_NO},
};
#endif

#define KTZ8864_RW_REG_MAX               15
#define INVALID_REG_VAL                  0xFFFF

struct ktz8864_backlight_information {
	int ktz8864_pull_boost_support;
	int ktz8864_pull_down_boost;
	int ktz8864_pull_up_boost;
	int ktz8864_enable_vsp_vsn;
	int ktz8864_vtc_line_boost;
	/* which i2c bus controller the second ktz8864 mount */
	int ktz8864_2_i2c_bus_id;
	/* Dual ktz8864 ic support */
	int dual_ic;
	/* ktz8864 bias switch */
	int ktz8864_2_dis_bias_cfg_1;
};

int ktz8864_reg[KTZ8864_RW_REG_MAX] = { 0 };


static char *ktz8864_dts_string[KTZ8864_RW_REG_MAX] = {
	"ktz8864_bl_config_1",
	"ktz8864_bl_config_2",
	"ktz8864_auto_freq_low",
	"ktz8864_auto_freq_high",
	"ktz8864_display_bias_config_2",
	"ktz8864_display_bias_config_3",
	"ktz8864_lcm_boost_bias",
	"ktz8864_vpos_bias",
	"ktz8864_vneg_bias",
	"ktz8864_bl_option_1",
	"ktz8864_bl_option_2",
	"ktz8864_display_bias_config_1",
	"ktz8864_ramp_time",
	"ktz8864_led_current",
	"ktz8864_bl_en",
};
struct ktz8864_voltage {
	u32 voltage;
	int value;
};

static struct ktz8864_voltage voltage_table[] = {
	{ 4500000, KTZ8864_VOL_45 },
	{ 4600000, KTZ8864_VOL_46 },
	{ 4700000, KTZ8864_VOL_47 },
	{ 4800000, KTZ8864_VOL_48 },
	{ 4900000, KTZ8864_VOL_49 },
	{ 5000000, KTZ8864_VOL_50 },
	{ 5100000, KTZ8864_VOL_51 },
	{ 5200000, KTZ8864_VOL_52 },
	{ 5300000, KTZ8864_VOL_53 },
	{ 5400000, KTZ8864_VOL_54 },
	{ 5500000, KTZ8864_VOL_55 },
	{ 5600000, KTZ8864_VOL_56 },
	{ 5700000, KTZ8864_VOL_57 },
	{ 5750000, KTZ8864_VOL_575 },
	{ 5800000, KTZ8864_VOL_58 },
	{ 5900000, KTZ8864_VOL_59 },
	{ 6000000, KTZ8864_VOL_60 },
	{ 6100000, KTZ8864_VOL_61 },
	{ 6200000, KTZ8864_VOL_62 },
};
static unsigned int ktz8864_reg_addr[KTZ8864_RW_REG_MAX] = {
	REG_BL_CONFIG_1,
	REG_BL_CONFIG_2,
	REG_AUTO_FREQ_LOW,
	REG_AUTO_FREQ_HIGH,
	REG_DISPLAY_BIAS_CONFIG_2,
	REG_DISPLAY_BIAS_CONFIG_3,
	REG_LCM_BOOST_BIAS,
	REG_VPOS_BIAS,
	REG_VNEG_BIAS,
	REG_BL_OPTION_1,
	REG_BL_OPTION_2,
	REG_DISPLAY_BIAS_CONFIG_1,
	REG_RAMP_TIME,
	REG_LED_CURRENT,
	REG_BL_ENABLE,
};

static unsigned int g_reg_val[KTZ8864_RW_REG_MAX] = { 0 };

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

struct bl_config_reg {
	unsigned int reg_element_num;
	unsigned int reg_addr[BL_MAX_CONFIG_REG_NUM];
	unsigned int normal_reg_var[BL_MAX_CONFIG_REG_NUM];
	unsigned int enhance_reg_var[BL_MAX_CONFIG_REG_NUM];
};

struct backlight_work_mode_reg_info {
	struct bl_config_reg bl_mode_config_reg;
	struct bl_config_reg bl_current_config_reg;
	struct bl_config_reg bl_enable_config_reg;
};

int ktz8864_set_backlight_reg(uint32_t bl_level);
ssize_t ktz8864_set_reg(u8 bl_reg, u8 bl_mask, u8 bl_val);
bool is_ktz8864_used(void);
#endif /* __LINUX_KTZ8864_H */

