/** @file
 * Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the tedpus of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _DKMD_UTILS_H_
#define _DKMD_UTILS_H_

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <linux/io.h>

#define CONFIG_DKMD_DEBUG_ENABLE (1)
#define CONFIG_DKMD_ASSERT_ENABLE (1)

#define HZ_1M   1000000
#define SIZE_1K 1024
#define DMA_STRIDE_ALIGN (128 / BITS_PER_BYTE)

extern uint32_t g_dkmd_log_level;

enum {
	DPU_LOG_LVL_ERR = 0,
	DPU_LOG_LVL_WARNING,
	DPU_LOG_LVL_INFO,
	DPU_LOG_LVL_DEBUG,
	DPU_LOG_LVL_MAX,
};

#undef pr_fmt
#define pr_fmt(fmt)  "dkmd: " fmt
#define dpu_dbg_nop(...)  ((void)#__VA_ARGS__)

#define dpu_pr_err(msg, ...)  do { \
		pr_err("[E][%s:%d]"msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#if CONFIG_DKMD_DEBUG_ENABLE
#define dpu_pr_warn(msg, ...)  do { \
		if (g_dkmd_log_level > DPU_LOG_LVL_WARNING) \
			pr_warning("[W][%s:%d]"msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define dpu_pr_info(msg, ...)  do { \
		if (g_dkmd_log_level > DPU_LOG_LVL_INFO) \
			pr_err("[I][%s:%d]"msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#define dpu_pr_debug(msg, ...)  do { \
		if (g_dkmd_log_level > DPU_LOG_LVL_DEBUG) \
			pr_info("[D][%s:%d]"msg, __func__, __LINE__, ## __VA_ARGS__); \
	} while (0)

#else

#define dpu_pr_warn(msg, ...)  dpu_dbg_nop(msg, __VA_ARGS__)
#define dpu_pr_info(msg, ...)  dpu_dbg_nop(msg, __VA_ARGS__)
#define dpu_pr_debug(msg, ...) dpu_dbg_nop(msg, __VA_ARGS__)

#endif

#if CONFIG_DKMD_ASSERT_ENABLE
#define dpu_assert_if_cond(cond)  do { \
		if (cond) { \
			pr_err("assertion failed! %s,%s:%d,%s\n", #cond,  __FILE__, __LINE__, __func__); \
			BUG(); \
		} \
	} while (0)
#else
#define dpu_assert_if_cond(ptr)
#endif

enum {
	WAIT_TYPE_US = 0,
	WAIT_TYPE_MS,
};

/* dtype for vcc */
enum {
	DTYPE_VCC_GET,
	DTYPE_VCC_PUT,
	DTYPE_VCC_ENABLE,
	DTYPE_VCC_DISABLE,
	DTYPE_VCC_SET_VOLTAGE,
};

/* pinctrl operation */
enum {
	DTYPE_PINCTRL_GET,
	DTYPE_PINCTRL_STATE_GET,
	DTYPE_PINCTRL_SET,
	DTYPE_PINCTRL_PUT,
};

/* pinctrl state */
enum {
	DTYPE_PINCTRL_STATE_DEFAULT,
	DTYPE_PINCTRL_STATE_IDLE,
};

/* dtype for gpio */
enum {
	DTYPE_GPIO_REQUEST,
	DTYPE_GPIO_FREE,
	DTYPE_GPIO_INPUT,
	DTYPE_GPIO_OUTPUT,
};

enum lcd_orientation {
	LCD_LANDSCAPE = 0,
	LCD_PORTRAIT,
};

enum lcd_format {
	LCD_RGB888 = 0,
	LCD_RGB101010,
	LCD_RGB565,
};

enum lcd_rgb_order {
	LCD_RGB = 0,
	LCD_BGR,
};

/* LCD init step */
enum LCD_INIT_STEP {
	LCD_INIT_NONE = 0,
	LCD_INIT_POWER_ON,
	LCD_INIT_MIPI_LP_SEND_SEQUENCE,
	LCD_INIT_MIPI_HS_SEND_SEQUENCE,
};

/* LCD uninit step */
enum LCD_UNINIT_STEP {
	LCD_UNINIT_NONE = 0,
	LCD_UNINIT_POWER_OFF,
	LCD_UNINIT_MIPI_LP_SEND_SEQUENCE,
	LCD_UNINIT_MIPI_HS_SEND_SEQUENCE,
};

enum IFBC_TYPE {
	IFBC_TYPE_NONE = 0,
	IFBC_TYPE_VESA3X_SINGLE,
	IFBC_TYPE_VESA3X_DUAL,

	IFBC_TYPE_MAX,
};

enum {
	BL_SET_BY_NONE = BIT(0),
	BL_SET_BY_PWM = BIT(1),
	BL_SET_BY_BLPWM = BIT(2),
	BL_SET_BY_MIPI = BIT(3),
	BL_SET_BY_SH_BLPWM = BIT(4)
};

enum {
	PANEL_NORMAL_TYPE = 0,
	PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE = 1,
};

struct gpio_desc {
	int dtype;
	int waittype;
	int wait;
	char *label;
	uint32_t *gpio;
	int value;
};

struct vcc_desc {
	int dtype;
	char *label;
	struct regulator **regulator;
	int min_uv;
	int max_uv;
	int waittype;
	int wait;
};

struct pinctrl_data {
	struct pinctrl *p;
	struct pinctrl_state *pinctrl_def;
	struct pinctrl_state *pinctrl_idle;
};

struct pinctrl_cmd_desc {
	int dtype;
	struct pinctrl_data *pctrl_data;
	int mode;
};

struct dsi_cmd_desc {
	uint32_t dtype;
	uint32_t vc;
	uint32_t wait;
	uint32_t waittype;
	uint32_t dlen;
	char *payload;
};

struct disp_rect {
	uint32_t x;
	uint32_t y;
	uint32_t w;
	uint32_t h;
};

#define outp32(addr, val) writel(val, addr)
#define inp32(addr)       readl(addr)

void set_reg(char __iomem *addr, uint32_t val, uint8_t bw, uint8_t bs);
uint32_t set_bits32(uint32_t old_val, uint32_t val, uint8_t bw, uint8_t bs);
int gpio_cmds_tx(struct gpio_desc *cmds, int cnt);
int vcc_cmds_tx(struct platform_device *pdev, struct vcc_desc *cmds, int cnt);
int pinctrl_cmds_tx(struct platform_device *pdev, struct pinctrl_cmd_desc *cmds, int cnt);
int mipi_dsi_cmds_tx(struct dsi_cmd_desc *cmds, int cnt, char __iomem *dsi_base);

#endif /* DKMD_UTILS_H */
