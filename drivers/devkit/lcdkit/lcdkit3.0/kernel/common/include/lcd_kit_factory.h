/*
 * lcd_kit_factory.h
 *
 * lcdkit factory function for lcdkit head file
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_FACTORY_H_
#define __LCD_KIT_FACTORY_H_
#include "lcd_kit_common.h"

/* checksum */
#define TEST_PIC_0	0
#define TEST_PIC_1	1
#define TEST_PIC_2	2
#define CHECKSUM_CHECKCOUNT	5
#define LCD_KIT_CHECKSUM_SIZE	8
#define CHECKSUM_VALUE_SIZE	8
#define INDEX_START	2
#define INVALID_INDEX	0xFF
/* ldo check */
#define LDO_CHECK_COUNT	20
#define LDO_NUM_MAX	3
#define LDO_NAME_LEN_MAX	32
/* vertical line test picture index */
#define PIC1_INDEX	1
#define PIC2_INDEX	2
#define PIC3_INDEX	3
#define PIC4_INDEX	4
#define PIC5_INDEX	5
// pt test state
#define IN_POWER_TEST	2
// define ms time unit
#define MILLISEC_TIME	1000
/* horizontal line test times */
#define HOR_LINE_TEST_TIMES	3
/* pcd errflag */
#define PCD_ERRFLAG_FAIL	3
#define GPIO_LCDKIT_PCD_NAME	"gpio_lcdkit_pcd"
#define GPIO_LCDKIT_ERRFLAG_NAME	"gpio_lcdkit_errflag"
#define GPIO_LOW_PCDERRFLAG	0
#define GPIO_HIGH_PCDERRFLAG	1
/* factory sysfs attrs max num */
#define FAC_SYSFS_ATTRS_NUM	64

/* enum */
enum inversion_mode {
	COLUMN_MODE = 0,
	DOT_MODE,
};

enum scan_mode {
	FORWORD_MODE = 0,
	REVERT_MODE,
};

enum {
	LCD_KIT_CHECKSUM_END = 0,
	LCD_KIT_CHECKSUM_START = 1,
};

/* struct define */
struct lcd_kit_checksum {
	u32 support;
	u32 pic_index;
	u32 status;
	u32 check_count;
	u32 stress_test_support;
	u32 vdd;
	u32 rec_vdd;
	u32 mipi_clk;
	u32 rec_mipi_clk;
	struct lcd_kit_dsi_panel_cmds checksum_cmds;
	struct lcd_kit_dsi_panel_cmds enable_cmds;
	struct lcd_kit_dsi_panel_cmds disable_cmds;
	struct lcd_kit_arrays_data value;
	struct lcd_kit_arrays_data dsi1_value;
};

struct lcd_ldo {
	int lcd_ldo_num;
	char lcd_ldo_name[LDO_NUM_MAX][LDO_NAME_LEN_MAX];
	int lcd_ldo_current[LDO_NUM_MAX];
	int lcd_ldo_threshold[LDO_NUM_MAX];
	int lcd_ldo_channel[LDO_NUM_MAX];
};

struct lcd_kit_ldo_check {
	u32 support;
	u32 ldo_num;
	u32 ldo_channel[LDO_NUM_MAX];
	int ldo_current[LDO_NUM_MAX];
	int curr_threshold[LDO_NUM_MAX];
	char ldo_name[LDO_NUM_MAX][LDO_NAME_LEN_MAX];
};

struct lcd_kit_current_detect {
	u32 support;
	struct lcd_kit_dsi_panel_cmds detect_cmds;
	struct lcd_kit_array_data value;
};

struct lcd_kit_lv_detect {
	u32 support;
	struct lcd_kit_dsi_panel_cmds detect_cmds;
	struct lcd_kit_array_data value;
};

struct lcd_hor_line_desc {
	u32 support;
	u32 duration;
	struct lcd_kit_dsi_panel_cmds hl_cmds;
};

struct vertical_line_desc {
	u32 support;
	struct lcd_kit_dsi_panel_cmds vtc_cmds;
};

struct lcd_fact_ops {
	ssize_t (*inversion_mode_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*inversion_mode_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*scan_mode_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*scan_mode_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*check_reg_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*gram_check_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*gram_check_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*sleep_ctrl_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*sleep_ctrl_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*pcd_errflag_check_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*test_config_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*test_config_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*lv_detect_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*current_detect_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*ldo_check_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*general_test_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*vtc_line_test_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*vtc_line_test_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*hkadc_debug_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
	ssize_t (*hkadc_debug_store)(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
	ssize_t (*bl_self_test_show)(struct device *dev,
		struct device_attribute *attr, char *buf);
};

struct lcd_kit_inversion {
	u32 support;
	u32 mode;
	struct lcd_kit_dsi_panel_cmds dot_cmds;
	struct lcd_kit_dsi_panel_cmds column_cmds;
};

struct lcd_kit_scan {
	u32 support;
	u32 mode;
	struct lcd_kit_dsi_panel_cmds forword_cmds;
	struct lcd_kit_dsi_panel_cmds revert_cmds;
};

struct lcd_kit_checkreg {
	u32 support;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_array_data value;
};

struct lcd_kit_pt_test {
	u32 support;
	u32 panel_ulps_support;
	u32 mode;
};

struct lcd_kit_hkadc {
	u32 support;
	int value;
};

struct lcd_fact_desc {
	/* test config */
	char lcd_cmd_now[LCD_KIT_CMD_NAME_MAX];
	/* backlight open short test */
	u32 bl_open_short_support;
	/* pcd err flag test */
	u32 pcd_errflag_check_support;
	u32 gpio_pcd;
	u32 gpio_errflag;
	/* inversion test */
	struct lcd_kit_inversion inversion;
	/* lcd forword/revert scan test */
	struct lcd_kit_scan scan;
	/* running test check reg */
	struct lcd_kit_checkreg check_reg;
	/* PT current test */
	struct lcd_kit_pt_test pt;
	/* check sum test */
	struct lcd_kit_checksum checksum;
	/* adc sample vsp voltage */
	struct lcd_kit_hkadc hkadc;
	/* current detect */
	struct lcd_kit_current_detect current_det;
	/* lv detect */
	struct lcd_kit_lv_detect lv_det;
	/* ldo check */
	struct lcd_kit_ldo_check ldo_check;
	/* horizontal line test */
	struct lcd_hor_line_desc hor_line;
	/* vertical line test witch picture */
	struct vertical_line_desc vtc_line;
};

/* variable declare */
extern struct lcd_fact_desc g_fact_info;

/* function declare */
struct lcd_fact_ops *lcd_get_fact_ops(void);
int lcd_fact_ops_register(struct lcd_fact_ops *ops);
int lcd_fact_ops_unregister(struct lcd_fact_ops *ops);
int lcd_kit_inversion_get_mode(char *buf);
int lcd_kit_inversion_set_mode(void *hdl, u32 mode);
int lcd_kit_scan_get_mode(char *buf);
int lcd_kit_scan_set_mode(void *hdl,  u32 mode);
int lcd_kit_check_reg(void *hdl, char *buf);
int lcd_kit_is_enter_pt_mode(void);
int lcd_kit_get_pt_station_status(void);
int lcd_kit_get_sleep_mode(char *buf);
int lcd_kit_set_sleep_mode(u32 mode);
int lcd_kit_get_pt_ulps_support(struct platform_device *pdev);
/* extern interface */
void lcd_kit_fact_init(struct device_node *np);
int lcd_create_fact_sysfs(struct kobject *obj);
int lcd_kit_get_test_config(char *buf);
int lcd_kit_set_test_config(const char *buf);
int lcd_kit_is_enter_sleep_mode(void);
#endif
