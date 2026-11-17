/*
 * rt4539.h
 *
 * rt4539 driver for backlight
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef _BL_RT4539_H_
#define _BL_RT4539_H_

#include "hisi_fb.h"
#include "../hisi_fb_panel.h"
#if defined(CONFIG_LCDKIT_DRIVER)
#include "lcdkit_panel.h"
#endif
#include <linux/hisi/hw_cmdline_parse.h>

#define RT4539_NAME             "rt4539"
#define DTS_COMP_RT4539       "realtek,rt4539"
#define GPIO_RT4539_EN_NAME     "rt4539_hw_en"

#define RT4539_BL_MIN        (0)
#define RT4539_BL_MAX        (4095)
#define RT4539_RW_REG_MAX    (13)

/* rt4539 reg address */
#define RT4539_CONTROL_MODE_ADDR           (0x00)
#define RT4539_CURRENT_PROTECTION_ADDR     (0x01)
#define RT4539_CURRENT_SETTING_ADDR        (0x02)
#define RT4539_VOLTAGE_SETTING_ADDR        (0x03)
#define RT4539_BRIGHTNESS_MSB_ADDR         (0x04)
#define RT4539_BRIGHTNESS_LSB_ADDR         (0x05)
#define RT4539_SLOPE_TIME_CONTROL_ADDR     (0x06)
#define RT4539_SAMPLE_RATE_SETTING_ADDR    (0x07)
#define RT4539_MODE_DIVISION_ADDR          (0x08)
#define RT4539_CONTROL_CLK_PFM_FUNC_ADDR   (0x09)
#define RT4539_REG_CONFIG_50               (0x0A)
#define RT4539_BACKLIGHT_CONTROL           (0x0B)
#define RT4539_MTP_FUNCTION                (0xFF)

#define PARSE_FAILED        (0xffff)
#define DELAY_0_US          (0)
#define DELAY_5_MS          (5)
#define DELAY_10_US         (10)
#define DELAY_50_US         (50)
#define DEFAULT_MSG_LEVEL   (7)

#define I2C3_BUSNUM (3)
#define I2C4_BUSNUM (4)
#define RT4539_DISABLE_DELAY               (60)

#ifndef bit
#define bit(x)  (1 << (x))
#endif

#define rt4539_emerg(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 0) \
		dev_emerg(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_alert(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 1) \
		dev_alert(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_crit(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 2) \
		dev_crit(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_err(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 3) \
		dev_err(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_warning(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 4) \
		dev_warn(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_notice(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 5) \
		dev_notice(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_info(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 6) \
		_dev_info(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)
#define rt4539_debug(dev, msg, ...) \
	do { if (g_rt4539_msg_level > 7) \
		_dev_info(dev, "%s: "msg, __func__, ## __VA_ARGS__); } while (0)

struct rt4539_chip_data {
	struct device *dev;
	struct i2c_client *client;
	struct regmap *regmap;
	struct semaphore test_sem;
};
struct rt4539_info {
	/* whether support rt4539 or not */
	int rt4539_support;
	/* which i2c bus controller rt4539 mount */
	int rt4539_i2c_bus_id;
	/* rt4539 hw_en gpio */
	int rt4539_hw_en_gpio;
	uint32_t reg[RT4539_RW_REG_MAX];
	int bl_on_kernel_mdelay;
	int rt4539_level_lsb;
	int rt4539_level_msb;
	int bl_led_num;
	int bl_set_long_slope;
	int dual_ic;
};

ssize_t rt4539_set_backlight_init(uint32_t bl_value);
bool is_rt4539_used(void);

#endif /* _BL_RT4539_H */


