/*
 * lcd_kit_utils.h
 *
 * lcdkit utils function for lcd driver head file
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_UTILS_H_
#define __LCD_KIT_UTILS_H_
#include <linux/kernel.h>
#include "lcd_kit_common.h"
#include "lcd_kit_panel.h"
#include "lcd_kit_sysfs.h"
#include "lcd_kit_adapt.h"
#include <linux/hisi/hw_cmdline_parse.h> // for runmode_is_factory

/* macro */
/* default panel */
#define LCD_KIT_DEFAULT_PANEL  "auo_otm1901a_5p2_1080p_video_default"

#define ARRAY_3_SIZE(array) (sizeof(array) / sizeof(array[0][0][0]))

/* lcd fps scence */
#define LCD_KIT_FPS_SCENCE_IDLE   BIT(0)
#define LCD_KIT_FPS_SCENCE_VIDEO  BIT(1)
#define LCD_KIT_FPS_SCENCE_GAME   BIT(2)
#define LCD_KIT_FPS_SCENCE_WEB    BIT(3)
#define LCD_KIT_FPS_SCENCE_EBOOK  BIT(4)
#define LCD_KIT_FPS_SCENCE_FORCE_30FPS          BIT(5)
#define LCD_KIT_FPS_SCENCE_FUNC_DEFAULT_ENABLE  BIT(6)
#define LCD_KIT_FPS_SCENCE_FUNC_DEFAULT_DISABLE BIT(7)
/* lcd fps value */
#define LCD_KIT_FPS_30 30
#define LCD_KIT_FPS_55 55
#define LCD_KIT_FPS_60 60
#define MAX_BUF 60
#define LCD_REG_LENGTH_MAX 200
#define LCD_DDIC_INFO_LEN       64
/* 2d barcode */
#define BARCODE_LENGTH   46
/* lcd panel version */
#define VERSION_VALUE_NUM_MAX 10
#define VERSION_NUM_MAX       20
/* gamma max len, store tpic */
#define GAMMA_MAX	146
#define GAMMA_HEAD_LEN	2
#define GAMMA_HEAD	0x47
#define GRAY_HEAD	0x46
#define GAMMA_LEN	0x0a

#define LCD_KIT_PCD_SIZE      3
#define LCD_KIT_ERRFLAG_SIZE  8
#define DMD_ERR_INFO_LEN      50
/* color uniform */
#define C_LMT_NUM   3
#define MXCC_ROW    3
#define MXCC_VOLUMN 3

#define CHROMA_ROW    4
#define CHROMA_VOLUMN 2
/* pcd errflag detect */
#define PCD_ERRFLAG_SUCCESS    0
#define PCD_FAIL               1
#define ERRFLAG_FAIL           2
/* max bl level */
#define MAX_BL_LEVEL	255

/* enum */
enum {
	RGBW_SET1_MODE = 1,
	RGBW_SET2_MODE = 2,
	RGBW_SET3_MODE = 3,
	RGBW_SET4_MODE = 4,
};

enum {
	RGBW_PANEL_ID_MIN = 0,
	JDI_NT36860C_PANEL_ID = 1,
	LG_NT36870_PANEL_ID = 2,
	SHARP_NT36870_PANEL_ID = 3,
	JDI_HX83112C_PANLE_ID = 4,
	SHARP_HX83112C_PANEL_ID = 5,
	JDI_TD4336_PANEL_ID = 6,
	SHARP_TD4336_PANEL_ID = 7,
	LG_NT36772A_PANEL_ID = 8,
	BOE_HX83112E_PANEL_ID = 9,
	JDI_TD4336_HMA_PANEL_ID = 10,
	SHARP_TD4336_HMA_PANEL_ID = 11,
	LG_NT36772A_HMA_PANEL_ID = 12,
	BOE_HX83112E_HMA_PANEL_ID = 13,
	JDI_TD4336_RT8555_HMA_PANEL_ID = 14,
	SHARP_TD4336_RT8555_HMA_PANEL_ID = 15,
	LG_NT36772A_RT8555_HMA_PANEL_ID = 16,
	RGBW_PANEL_ID_MAX,
};

enum {
	LCD_OFFLINE = 0,
	LCD_ONLINE = 1,
};

enum {
	PRIMARY_REGION = 0,
	SLAVE_REGION = 1,
	FOLD_REGION = 2,
	REGION_MAX = 3,
};

/* struct define */
struct lcd_kit_cascade_ic {
	u32 support;
	struct lcd_kit_dsi_panel_cmds region_a_cmds;
	struct lcd_kit_dsi_panel_cmds region_b_cmds;
	struct lcd_kit_dsi_panel_cmds region_ab_cmds;
	struct lcd_kit_dsi_panel_cmds region_ab_fold_cmds;
};

struct lcd_kit_gamma {
	u32 support;
	u32 addr;
	u32 length;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_brightness_color_uniform {
	u32 support;
	/* color consistency support */
	struct lcd_kit_dsi_panel_cmds brightness_color_cmds;
};

struct lcd_kit_panel_id {
	u32 modulesn;
	u32 equipid;
	u32 modulemanufactdate;
	u32 vendorid;
};

struct lcd_kit_color_uniform_params {
	u32 c_lmt[C_LMT_NUM];
	u32 mxcc_matrix[MXCC_ROW][MXCC_VOLUMN];
	u32 white_decay_luminace;
};

struct lcd_kit_color_measure_data {
	u32 chroma_coordinates[CHROMA_ROW][CHROMA_VOLUMN];
	u32 white_luminance;
};

struct lcd_kit_brightness_color_oeminfo {
	uint32_t id_flag;
	uint32_t tc_flag;
	struct lcd_kit_panel_id panel_id;
	struct lcd_kit_color_uniform_params color_params;
	struct lcd_kit_color_measure_data color_mdata;
};

struct lcd_kit_2d_barcode {
	u32 support;
	u32 block_num;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_oem_info {
	u32 support;
	/* 2d barcode */
	struct lcd_kit_2d_barcode barcode_2d;
	/* brightness and color uniform */
	struct lcd_kit_brightness_color_uniform brightness_color_uniform;
};

struct lcd_kit_project_id {
	u32 support;
	char *default_project_id;
	char id[LCD_DDIC_INFO_LEN];
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_panel_version {
	u32 support;
	u32 value_number;
	u32 version_number;
	char read_value[VERSION_VALUE_NUM_MAX];
	char lcd_version_name[VERSION_NUM_MAX][LCD_PANEL_VERSION_SIZE];
	struct lcd_kit_arrays_data value;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_dsi_panel_cmds enter_cmds;
	struct lcd_kit_dsi_panel_cmds exit_cmds;
};

struct lcd_kit_fps {
	u32 support;
	struct lcd_kit_dsi_panel_cmds dfr_enable_cmds;
	struct lcd_kit_dsi_panel_cmds dfr_disable_cmds;
	struct lcd_kit_dsi_panel_cmds fps_to_30_cmds;
	struct lcd_kit_dsi_panel_cmds fps_to_60_cmds;
	struct lcd_kit_array_data low_frame_porch;
	struct lcd_kit_array_data normal_frame_porch;
};

struct lcd_kit_rgbw {
	u32 support;
	u32 rgbw_bl_max;
	struct lcd_kit_dsi_panel_cmds mode1_cmds;
	struct lcd_kit_dsi_panel_cmds mode2_cmds;
	struct lcd_kit_dsi_panel_cmds mode3_cmds;
	struct lcd_kit_dsi_panel_cmds mode4_cmds;
	struct lcd_kit_dsi_panel_cmds backlight_cmds;
	struct lcd_kit_dsi_panel_cmds saturation_ctrl_cmds;
	struct lcd_kit_dsi_panel_cmds frame_gain_limit_cmds;
	struct lcd_kit_dsi_panel_cmds frame_gain_speed_cmds;
	struct lcd_kit_dsi_panel_cmds color_distor_allowance_cmds;
	struct lcd_kit_dsi_panel_cmds pixel_gain_limit_cmds;
	struct lcd_kit_dsi_panel_cmds pixel_gain_speed_cmds;
	struct lcd_kit_dsi_panel_cmds pwm_gain_cmds;
};

struct lcd_kit_alpm {
	u32 support;
	u32 state;
	struct lcd_kit_dsi_panel_cmds exit_cmds;
	struct lcd_kit_dsi_panel_cmds off_cmds;
	struct lcd_kit_dsi_panel_cmds low_light_cmds;
	struct lcd_kit_dsi_panel_cmds high_light_cmds;
};

struct lcd_kit_snd_disp {
	u32 support;
	struct lcd_kit_dsi_panel_cmds on_cmds;
	struct lcd_kit_dsi_panel_cmds off_cmds;
};

struct lcd_kit_quickly_sleep_out {
	u32 support;
	u32 interval;
	u32 panel_on_tag;
	struct timeval panel_on_record_tv;
};

struct lcd_kit_otp_gamma {
	u32 support;
	u8 gamma[GAMMA_MAX + 1];
	struct lcd_kit_dsi_panel_cmds elvss_cmds;
	struct lcd_kit_dsi_panel_cmds gamma_cmds;
	struct lcd_kit_dsi_panel_cmds gray_cmds;
};

struct lcd_kit_pcd_errflag {
	u32 pcd_support;
	u32 errflag_support;
	struct lcd_kit_dsi_panel_cmds start_pcd_check_cmds;
	struct lcd_kit_dsi_panel_cmds switch_page_cmds;
	struct lcd_kit_dsi_panel_cmds read_pcd_cmds;
	struct lcd_kit_array_data pcd_value;
	struct lcd_kit_dsi_panel_cmds read_errflag_cmds;
};

struct dbv_stat_desc {
	u32 support;
	u32 dbv[REGION_MAX];
	u32 pwon[REGION_MAX];
	struct timeval last_time[REGION_MAX];
	struct timeval pwon_last_time[REGION_MAX];
};

/* function declare */
extern int mipi_dsi_ulps_cfg(struct hisi_fb_data_type *hisifd, int enable);
struct hisi_fb_data_type *dev_get_hisifd(struct device *dev);
int lcd_kit_lread_reg(void *pdata, uint32_t *out,
	struct lcd_kit_dsi_cmd_desc *cmds, uint32_t len);
int lcd_kit_rgbw_set_mode(struct hisi_fb_data_type *hisifd, int mode);
int lcd_kit_rgbw_set_backlight(struct hisi_fb_data_type *hisifd,
	uint32_t bl_level);
int lcd_kit_rgbw_set_handle(struct hisi_fb_data_type *hisifd);
int lcd_kit_updt_fps(struct platform_device *pdev);
int lcd_kit_updt_fps_scence(struct platform_device *pdev, uint32_t scence);
int lcd_kit_get_bl_set_type(struct hisi_panel_info *pinfo);
int lcd_kit_rgbw_set_bl(struct hisi_fb_data_type *hisifd, uint32_t level);
int lcd_kit_blpwm_set_backlight(struct hisi_fb_data_type *hisifd, uint32_t level);
int lcd_kit_mipi_set_backlight(struct hisi_fb_data_type *hisifd, uint32_t level);
int lcd_kit_start_pcd_check(struct hisi_fb_data_type *hisifd);
int lcd_kit_check_pcd_errflag_check(struct hisi_fb_data_type *hisifd);
int lcd_kit_read_gamma(struct hisi_fb_data_type *hisifd, uint8_t *read_value,
	int len);
int lcd_kit_parse_switch_cmd(struct hisi_fb_data_type *hisifd,
	const char *command);
int lcd_kit_read_project_id(void);
int lcd_kit_panel_version_init(struct hisi_fb_data_type *hisifd);
int lcd_kit_alpm_setting(struct hisi_fb_data_type *hisifd, uint32_t mode);
int lcd_kit_utils_init(struct device_node *np, struct hisi_panel_info *pinfo);
int lcd_kit_dsi_fifo_is_full(const char __iomem *dsi_base);
int lcd_kit_dsi_fifo_is_empty(const char __iomem *dsi_base);
int lcd_kit_realtime_set_xcc(struct hisi_fb_data_type *hisifd, const char *buf,
	size_t count);
int lcd_kit_get_power_status(void);
bool lcd_kit_support(void);
void lcd_kit_effect_switch_ctrl(struct hisi_fb_data_type *hisifd, bool ctrl);
void lcd_kit_disp_on_check_delay(void);
void lcd_kit_disp_on_record_time(void);
void lcd_kit_read_power_status(struct hisi_fb_data_type *hisifd);
struct lcd_kit_brightness_color_oeminfo *lcd_kit_get_brightness_color_oeminfo(void);
void lcd_kit_set_mipi_link(struct hisi_fb_data_type *hisifd, int link_state);
void lcd_kit_set_mipi_link(struct hisi_fb_data_type *hisifd, int link_state);
void lcd_kit_set_mipi_clk(struct hisi_fb_data_type *hisifd, uint32_t clk);
int lcd_kit_get_value_from_dts(char *compatible, char *dts_name, u32 *value);
int lcd_kit_write_otp_gamma(u8 *buf);
int lcd_kit_set_otp_gamma(struct hisi_fb_data_type *hisifd);
int lcd_kit_set_otp_gray(struct hisi_fb_data_type *hisifd);
void lcd_frame_refresh(struct hisi_fb_data_type *hisifd);
void lcd_kit_recovery_display(struct hisi_fb_data_type *hisifd);
void lcd_hardware_reset(void);
void lcd_esd_enable(struct hisi_fb_data_type *hisifd, int enable);
bool lcd_is_power_on(uint32_t level);
bool lcd_is_dual_mipi(void);
#endif
