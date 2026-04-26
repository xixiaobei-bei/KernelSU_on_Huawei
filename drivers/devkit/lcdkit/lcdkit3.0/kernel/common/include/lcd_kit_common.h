/*
 * lcd_kit_common.h
 *
 * lcdkit common function for lcdkit head file
 *
 * Copyright (c) 2018-2021 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_PANEL_H_
#define __LCD_KIT_PANEL_H_
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/time.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/io.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/memblock.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/pinctrl/consumer.h>
#include <linux/file.h>
#include <linux/dma-buf.h>
#include <linux/genalloc.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include "lcd_kit_bias.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bl.h"

#if defined CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

extern int lcd_kit_msg_level;
#ifdef LV_GET_LCDBK_ON
extern u32 mipi_level;
#endif

/* log level */
#define MSG_LEVEL_ERROR   1
#define MSG_LEVEL_WARNING 2
#define MSG_LEVEL_INFO    3
#define MSG_LEVEL_DEBUG   4

#define LCD_KIT_ALPHA_DEFAULT 4095

#define LCD_KIT_ERR(msg, ...) do { \
	if (lcd_kit_msg_level >= MSG_LEVEL_ERROR) \
		printk(KERN_ERR "[LCD_KIT/E]%s: "msg, __func__, ## __VA_ARGS__); \
} while (0)
#define LCD_KIT_WARNING(msg, ...) do { \
	if (lcd_kit_msg_level >= MSG_LEVEL_WARNING) \
		printk(KERN_WARNING "[LCD_KIT/W]%s: "msg, __func__, ## __VA_ARGS__); \
} while (0)
#define LCD_KIT_INFO(msg, ...) do { \
	if (lcd_kit_msg_level >= MSG_LEVEL_INFO) \
		printk(KERN_ERR "[LCD_KIT/I]%s: "msg, __func__, ## __VA_ARGS__); \
} while (0)
#define LCD_KIT_DEBUG(msg, ...) do { \
	if (lcd_kit_msg_level >= MSG_LEVEL_DEBUG) \
		printk(KERN_INFO "[LCD_KIT/D]%s: "msg, __func__, ## __VA_ARGS__); \
} while (0)

#define LCD_KIT_CMD_NAME_MAX 100
#define MAX_REG_READ_COUNT   4
#define LCD_KIT_SN_CODE_LENGTH 54
#define MAX_I2C_DMD_COUNT  2

#define LCD_KIT_FAIL (-1)
#define LCD_KIT_OK   0
#define NOT_SUPPORT 0
/* check thead period */
#define CHECK_THREAD_TIME_PERIOD 5000
#define BITS(x) (1 << (x))
#define BL_MIN  0
#define BL_NIT  400
#define BL_REG_NOUSE_VALUE 128
#define LCD_KIT_ENABLE_ELVSSDIM_MASK  0x80
#define LCD_KIT_DISABLE_ELVSSDIM_MASK 0x7F
#define LCD_KIT_SHIFT_FOUR_BIT 4
#define LCD_KIT_HIGH_12BIT_CTL_HBM_SUPPORT 1
#define LCD_KIT_8BIT_CTL_HBM_SUPPORT 2
#define HBM_SET_MAX_LEVEL 5000
#define LCD_KIT_FPS_HIGH 90
#define MAX_MARGIN_DELAY 62
#define TP_PROXMITY_DISABLE 0
#define TP_PROXMITY_ENABLE  1
#define LCD_RESET_LOW      0
#define LCD_RESET_HIGH      1
#define POWER_ON 0
#define POWER_TS_SUSPEND 1
#define POWER_OFF 2

#define SEQ_NUM 3

/* pcd errflag */
#define PCD_ERRFLAG_FAIL        3
#define GPIO_PCD_ERRFLAG_NAME   "gpio_lcdkit_pcd_errflag"
#define GPIO_LOW_PCDERRFLAG     0
#define GPIO_HIGH_PCDERRFLAG    1

#define FPS_DFR_STATUS_IDLE 1
#define FPS_DFR_STATUS_DOING 0
#define HBM_STATUS_IDLE 1
#define HBM_STATUS_DOING 0

#define GPIO_CHECK_TIMES 3

struct lcd_kit_common_ops *lcd_kit_get_common_ops(void);
#define common_ops lcd_kit_get_common_ops()
struct lcd_kit_common_info *lcd_kit_get_common_info(void);
#define common_info lcd_kit_get_common_info()
struct lcd_kit_power_desc *lcd_kit_get_power_handle(void);
#define power_hdl lcd_kit_get_power_handle()
struct lcd_kit_power_seq *lcd_kit_get_power_seq(void);
#define power_seq lcd_kit_get_power_seq()
struct lcd_kit_common_lock *lcd_kit_get_common_lock(void);
#define COMMON_LOCK lcd_kit_get_common_lock()
struct lcd_kit_power_desc *lcd_kit_get_panel_power_handle(
	uint32_t panel_id);
struct lcd_kit_power_seq *lcd_kit_get_panel_power_seq(
	uint32_t panel_id);

/* parse dirty region info node */
#define OF_PROPERTY_READ_DIRTYREGION_INFO_RETURN(np, propname, ptr_out_value) do { \
	if (of_property_read_u32(np, propname, ptr_out_value)) { \
		LCD_KIT_INFO("of_property_read_u32: %s not find\n", propname); \
		*ptr_out_value = -1; \
	} \
	if (*ptr_out_value == 0xffff) { \
		*ptr_out_value = -1; \
	} \
} while (0)

#define OF_PROPERTY_READ_U64_RETURN(np, propname, ptr_out_value) do { \
	u32 temp = 0; \
	if (of_property_read_u32(np, propname, &temp)) { \
		LCD_KIT_INFO("of_property_read: %s not find\n", propname); \
		temp = 0; \
	} \
	*ptr_out_value = (u64)temp; \
} while (0)

/* parse dts node */
#define OF_PROPERTY_READ_U32_RETURN(np, propname, ptr_out_value) do { \
	if (of_property_read_u32(np, propname, ptr_out_value)) { \
		LCD_KIT_INFO("of_property_read_u32: %s not find\n", propname); \
		*ptr_out_value = 0; \
	} \
} while (0)

#define OF_PROPERTY_READ_S8_RETURN(np, propname, ptr_out_value) do { \
	if (of_property_read_u32(np, propname, ptr_out_value)) { \
		LCD_KIT_INFO("of_property_read_u32: %s not find\n", propname); \
		*ptr_out_value = 0; \
	} \
	if ((*ptr_out_value & 0xff00) == 0xff00) { \
		*ptr_out_value = 0 - (*ptr_out_value & 0xff); \
	} \
} while (0)


/* parse dts node */
#define OF_PROPERTY_READ_U8_RETURN(np, propname, ptr_out_value) do { \
	int temp = 0; \
	if (of_property_read_u32(np, propname, &temp)) { \
		LCD_KIT_INFO("of_property_read: %s not find\n", propname); \
		temp = 0; \
	} \
	*ptr_out_value = (char)temp; \
} while (0)

/* parse dts node */
#define OF_PROPERTY_READ_U32_DEFAULT(np, propname, ptr_out_value, default) do { \
	if (of_property_read_u32(np, propname, ptr_out_value)) { \
		LCD_KIT_INFO("of_property_read_u32: %s not find, use default: %d\n", propname, default); \
		*ptr_out_value = default; \
	} \
} while (0)

/* parse dts node */
#define OF_PROPERTY_READ_U8_DEFAULT(np, propname, ptr_out_value, default) do { \
	int temp = 0; \
	if (of_property_read_u32(np, propname, &temp)) { \
		LCD_KIT_INFO("of_property_read: %s not find, use default: %d\n", propname, default); \
		temp = default;  \
	} \
	*ptr_out_value = (char)temp; \
} while (0)

/* enum */
enum lcd_kit_mipi_ctrl_mode {
	LCD_KIT_DSI_LP_MODE,
	LCD_KIT_DSI_HS_MODE,
};

/* get blmaxnit */
enum {
	GET_BLMAXNIT_FROM_DDIC = 1,
};

enum lcd_kit_power_mode {
	NONE_MODE,
	REGULATOR_MODE,
	GPIO_MODE,
};

enum lcd_kit_power_type {
	LCD_KIT_VCI,
	LCD_KIT_IOVCC,
	LCD_KIT_VSP,
	LCD_KIT_VSN,
	LCD_KIT_RST,
	LCD_KIT_TP_RST,
	LCD_KIT_BL,
	LCD_KIT_VDD,
	LCD_KIT_AOD,
	LCD_KIT_POWERIC_DET_DBC,
	LCD_KIT_MIPI_SWITCH,
};

enum lcd_kit_event {
	EVENT_NONE,
	EVENT_VCI,
	EVENT_IOVCC,
	EVENT_VSP,
	EVENT_VSN,
	EVENT_RESET,
	EVENT_MIPI,
	EVENT_EARLY_TS,
	EVENT_LATER_TS,
	EVENT_VDD,
	EVENT_AOD,
	EVENT_BIAS,
	EVENT_AOD_MIPI,
	EVENT_MIPI_SWITCH,
	EVENT_AVDD_MIPI,
	EVENT_2ND_POWER_OFF_TS,
};

enum bl_order {
	BL_BIG_ENDIAN,
	BL_LITTLE_ENDIAN,
};

enum cabc_mode {
	CABC_OFF_MODE = 0,
	CABC_UI = 1,
	CABC_STILL = 2,
	CABC_MOVING = 3,
};

enum hbm_mode {
	HBM_OFF_MODE = 0,
	HBM_MAX_MODE = 1,
	HBM_MEDIUM_MODE = 2,
};

enum acl_mode {
	ACL_OFF_MODE = 0,
	ACL_HIGH_MODE = 1,
	ACL_MIDDLE_MODE = 2,
	ACL_LOW_MODE = 3,
};

enum vr_mode {
	VR_DISABLE = 0,
	VR_ENABLE = 1,
};

enum ce_mode {
	CE_OFF_MODE = 0,
	CE_SRGB = 1,
	CE_USER = 2,
	CE_VIVID = 3,
};

enum esd_state {
	ESD_RUNNING = 0,
	ESD_STOP = 1,
};

enum lcd_type {
	LCD_TYPE = 1,
	AMOLED_TYPE = 2,
};

enum esd_judge_type {
	ESD_UNEQUAL,
	ESD_EQUAL,
	ESD_BIT_VALID,
};

enum {
	LCD_KIT_WAIT_US = 0,
	LCD_KIT_WAIT_MS,
};

enum {
	POWER_MODE = 0,
	POWER_NUM,
	POWER_VOL,
};

enum {
	EVENT_NUM = 0,
	EVENT_DATA, // it means power on or power off
	EVENT_DELAY,
};

enum {
	LCD_KIT_BTB_CHECK_GPIO = 0,
	LCD_KIT_BTB_CHECK_IRQ,
};

/* dfr hbm combined cmd type */
enum {
	FPS_60P_NORMAL_DIM = 1,
	FPS_60P_HBM_NO_DIM = 2,
	FPS_60P_HBM_DIM = 3,
	FPS_90_NORMAL_DIM = 4,
	FPS_90_HBM_NO_DIM = 5,
	FPS_90_HBM_DIM = 6,
	FPS_90_NORMAL_NO_DIM = 7,
	FPS_SCENCE_MAX,
};

/*
 * struct definition
 */
struct lcd_kit_dsi_cmd_desc {
	char dtype; /* data type */
	char last; /* last in chain */
	char vc; /* virtual chan */
	char ack; /* ask ACK from peripheral */
	char wait; /* ms */
	char waittype;
	char dlen; /* 8 bits */
	char *payload;
};

struct lcd_kit_dsi_cmd_desc_header {
	char dtype; /* data type */
	char last; /* last in chain */
	char vc; /* virtual chan */
	char ack; /* ask ACK from peripheral */
	char wait; /* ms */
	char waittype;
	char dlen; /* 8 bits */
};

struct lcd_kit_dsi_panel_cmds {
	char *buf;
	int blen;
	struct lcd_kit_dsi_cmd_desc *cmds;
	int cmd_cnt;
	int link_state;
	u32 flags;
};

/* get blmaxnit */
struct lcd_kit_blmaxnit {
	u32 get_blmaxnit_type;
	u32 lcd_kit_brightness_ddic_info;
	struct lcd_kit_dsi_panel_cmds bl_maxnit_cmds;
};

struct lcd_kit_thp_proximity {
	unsigned int support;
	int work_status;
	int panel_power_state;
	int after_reset_delay_min;
	struct timeval lcd_reset_record_tv;
};

struct lcd_kit_array_data {
	uint32_t *buf;
	int cnt;
};

struct lcd_kit_arrays_data {
	struct lcd_kit_array_data *arry_data;
	int cnt;
};

struct region_rect {
	u32 x;
	u32 y;
	u32 w;
	u32 h;
};

struct pre_camera_position {
	u32 support;
	u32 end_y;
};

struct lcd_kit_sn {
	unsigned int support;
	unsigned int check_support;
	unsigned char sn_code_data[LCD_KIT_SN_CODE_LENGTH];
	struct lcd_kit_array_data sn_code_info;
};

struct lcd_kit_cabc {
	u32 support;
	u32 mode;
	/* cabc off command */
	struct lcd_kit_dsi_panel_cmds cabc_off_cmds;
	/* cabc ui command */
	struct lcd_kit_dsi_panel_cmds cabc_ui_cmds;
	/* cabc still command */
	struct lcd_kit_dsi_panel_cmds cabc_still_cmds;
	/* cabc moving command */
	struct lcd_kit_dsi_panel_cmds cabc_moving_cmds;
};

#define HBM_GAMMA_SIZE 24
#define HBM_GAMMA_NODE_SIZE (HBM_GAMMA_SIZE / 2)

struct hbm_gamma {
	uint32_t check_flag;
	uint8_t red_60_hz[HBM_GAMMA_SIZE];
	uint8_t green_60_hz[HBM_GAMMA_SIZE];
	uint8_t blue_60_hz[HBM_GAMMA_SIZE];
	uint8_t red_90_hz[HBM_GAMMA_SIZE];
	uint8_t green_90_hz[HBM_GAMMA_SIZE];
	uint8_t blue_90_hz[HBM_GAMMA_SIZE];
};

struct gamma_node_info {
	uint32_t red_60_hz[HBM_GAMMA_NODE_SIZE];
	uint32_t green_60_hz[HBM_GAMMA_NODE_SIZE];
	uint32_t blue_60_hz[HBM_GAMMA_NODE_SIZE];
	uint32_t red_90_hz[HBM_GAMMA_NODE_SIZE];
	uint32_t green_90_hz[HBM_GAMMA_NODE_SIZE];
	uint32_t blue_90_hz[HBM_GAMMA_NODE_SIZE];
	struct lcd_kit_array_data node_grayscale;
};

struct lcd_kit_hbm {
	u32 support;
	// for UD printfinger start
	uint8_t ori_elvss_val;
	u32 hbm_fp_support;
	u32 local_hbm_support;
	u32 hbm_fps_command_support;
	u32 hbm_level_bias;
	u32 hbm_level_skip_high;
	u32 hbm_level_skip_low;
	u32 hbm_level_base;
	u32 hbm_level_max;
	u32 hbm_level_current;
	u32 hbm_if_fp_is_using;
	u32 hbm_fp_elvss_support;
	u32 hbm_set_elvss_dim_lp;
	u32 hbm_fp_elvss_cmd_delay;
	u32 hbm_special_bit_ctrl;
	// for UD printfinger end
	struct lcd_kit_dsi_panel_cmds enter_cmds;
	struct lcd_kit_dsi_panel_cmds fp_enter_extern_cmds;
	struct lcd_kit_dsi_panel_cmds fp_exit_extern_cmds;
	struct lcd_kit_dsi_panel_cmds enter_no_dim_cmds;
	struct lcd_kit_dsi_panel_cmds fp_enter_cmds;
	struct lcd_kit_dsi_panel_cmds hbm_prepare_cmds;
	struct lcd_kit_dsi_panel_cmds prepare_cmds_fir;
	struct lcd_kit_dsi_panel_cmds prepare_cmds_sec;
	struct lcd_kit_dsi_panel_cmds prepare_cmds_thi;
	struct lcd_kit_dsi_panel_cmds prepare_cmds_fou;
	struct lcd_kit_dsi_panel_cmds hbm_cmds;
	struct lcd_kit_dsi_panel_cmds hbm_post_cmds;
	struct mutex hbm_lock;
	struct lcd_kit_dsi_panel_cmds exit_cmds;
	struct lcd_kit_dsi_panel_cmds exit_cmds_fir;
	struct lcd_kit_dsi_panel_cmds exit_cmds_sec;
	struct lcd_kit_dsi_panel_cmds exit_cmds_thi;
	struct lcd_kit_dsi_panel_cmds exit_cmds_thi_new;
	struct lcd_kit_dsi_panel_cmds exit_cmds_fou;
	struct lcd_kit_dsi_panel_cmds enter_dim_cmds;
	struct lcd_kit_dsi_panel_cmds exit_dim_cmds;
	struct lcd_kit_dsi_panel_cmds elvss_prepare_cmds;
	struct lcd_kit_dsi_panel_cmds elvss_read_cmds;
	struct lcd_kit_dsi_panel_cmds elvss_write_cmds;
	struct lcd_kit_dsi_panel_cmds elvss_post_cmds;
	struct lcd_kit_dsi_panel_cmds enter_alpha_cmds;
	struct lcd_kit_dsi_panel_cmds hbm_dbv_cmds;
	struct lcd_kit_dsi_panel_cmds hbm_em_configure_60hz_cmds;
	struct lcd_kit_dsi_panel_cmds hbm_em_configure_90hz_cmds;
	struct lcd_kit_dsi_panel_cmds exit_alpha_cmds;
	struct lcd_kit_dsi_panel_cmds enter_circle_cmds;
	struct lcd_kit_dsi_panel_cmds exit_circle_cmds;
	struct lcd_kit_dsi_panel_cmds enter_alphacircle_cmds;
	struct lcd_kit_dsi_panel_cmds exit_alphacircle_cmds;
	struct lcd_kit_dsi_panel_cmds circle_coordinate_cmds;
	struct lcd_kit_dsi_panel_cmds circle_size_small_cmds;
	struct lcd_kit_dsi_panel_cmds circle_size_mid_cmds;
	struct lcd_kit_dsi_panel_cmds circle_size_large_cmds;
	struct lcd_kit_dsi_panel_cmds circle_color_cmds;
	struct lcd_kit_array_data alpha_table;
	struct lcd_kit_dsi_panel_cmds hbm_60_hz_gamma_read_cmds;
	struct lcd_kit_dsi_panel_cmds hbm_90_hz_gamma_read_cmds;
	struct hbm_gamma hbm_gamma;
	struct lcd_kit_dsi_panel_cmds hbm_circle_color_setting_cmds;
	struct gamma_node_info gamma_info;
};

struct lcd_kit_ddic_alpha {
	u32 alpha_support;
	int alpha_with_enable_flag;
	struct lcd_kit_dsi_panel_cmds enter_alpha_cmds;
	struct lcd_kit_dsi_panel_cmds exit_alpha_cmds;
};

/* fps dfr status and hbm cmds */
struct lcd_kit_dfr_info {
	u32 fps_lock_command_support;
	wait_queue_head_t fps_wait;
	wait_queue_head_t hbm_wait;
	u32 fps_dfr_status;
	u32 hbm_status;
	struct lcd_kit_dsi_panel_cmds cmds[FPS_SCENCE_MAX];
};

struct lcd_kit_sysfs_lock {
	struct mutex model_lock;
	struct mutex type_lock;
	struct mutex panel_info_lock;
	struct mutex vol_enable_lock;
	struct mutex amoled_acl_lock;
	struct mutex amoled_vr_lock;
	struct mutex support_mode_lock;
	struct mutex gamma_dynamic_lock;
	struct mutex frame_count_lock;
	struct mutex frame_update_lock;
	struct mutex mipi_dsi_clk_lock;
	struct mutex fps_scence_lock;
	struct mutex fps_order_lock;
	struct mutex alpm_function_lock;
	struct mutex alpm_setting_lock;
	struct mutex ddic_alpha_lock;
	struct mutex func_switch_lock;
	struct mutex reg_read_lock;
	struct mutex ddic_oem_info_lock;
	struct mutex bl_mode_lock;
	struct mutex support_bl_mode_lock;
	struct mutex effect_bl_mode_lock;
	struct mutex ddic_lv_detect_lock;
	struct mutex hbm_mode_lock;
	struct mutex panel_sn_code_lock;
	struct mutex pre_camera_position_lock;
};

struct lcd_kit_common_lock {
	struct mutex mipi_lock;
	struct mutex hbm_lock;
	struct lcd_kit_sysfs_lock sysfs_lock;
};

struct lcd_kit_esd {
	u32 support;
	u32 fac_esd_support;
	u32 recovery_bl_support;
	u32 te_check_support;
	u32 status;
	u32 gpio_detect_support;
	u32 gpio_detect_num;
	u32 gpio_normal_value;
	u32 tp_esd_event;
	u32 tp_report_detect_times;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_array_data value;
	struct lcd_kit_dsi_panel_cmds ext_cmds;
	struct lcd_kit_array_data ext_value;
	struct lcd_kit_dsi_panel_cmds mipi_err_cmds;
	u32 mipi_err_cnt;
};

struct lcd_kit_dsi {
	u32 support;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_array_data value;
};

struct lcd_kit_esd_error_info {
	int esd_error_reg_num;
	int esd_reg_index[MAX_REG_READ_COUNT];
	int esd_expect_reg_val[MAX_REG_READ_COUNT];
	int esd_error_reg_val[MAX_REG_READ_COUNT];
};

struct lcd_kit_check_reg_dsm {
	u32 support;
	u32 support_dsm_report;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_array_data value;
};

struct lcd_kit_mipicheck {
	u32 support;
	u32 mipi_error_report_threshold;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_array_data value;
};

struct lcd_kit_mipierrors {
	u32 mipi_check_times;
	u32 mipi_error_times;
	u32 total_errors;
};

struct lcd_kit_dirty {
	u32 support;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_acl {
	u32 support;
	u32 mode;
	struct lcd_kit_dsi_panel_cmds acl_enable_cmds;
	struct lcd_kit_dsi_panel_cmds acl_off_cmds;
	struct lcd_kit_dsi_panel_cmds acl_low_cmds;
	struct lcd_kit_dsi_panel_cmds acl_middle_cmds;
	struct lcd_kit_dsi_panel_cmds acl_high_cmds;
};

struct lcd_kit_vr {
	u32 support;
	u32 mode;
	struct lcd_kit_dsi_panel_cmds enable_cmds;
	struct lcd_kit_dsi_panel_cmds disable_cmds;
};

struct lcd_kit_ce {
	u32 support;
	u32 mode;
	struct lcd_kit_dsi_panel_cmds off_cmds;
	struct lcd_kit_dsi_panel_cmds srgb_cmds;
	struct lcd_kit_dsi_panel_cmds user_cmds;
	struct lcd_kit_dsi_panel_cmds vivid_cmds;
};

struct lcd_kit_set_vss {
	u32 support;
	u32 power_off;
	u32 new_backlight;
	struct lcd_kit_dsi_panel_cmds cmds_fir;
	struct lcd_kit_dsi_panel_cmds cmds_sec;
	struct lcd_kit_dsi_panel_cmds cmds_thi;
};

struct lcd_kit_set_power {
	u32 support;
	u32 get_thermal;
	u32 oldthermal;
	u32 thermal1;
	u32 thermal2;
	struct lcd_kit_dsi_panel_cmds cmds_fir;
	struct lcd_kit_dsi_panel_cmds cmds_sec;
};

struct lcd_kit_elvdd_detect {
	u32 support;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_effect_on {
	u32 support;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_grayscale_optimize {
	u32 support;
	struct lcd_kit_dsi_panel_cmds cmds;
};

struct lcd_kit_screen_on_effect {
	u32 support;
	struct lcd_kit_dsi_panel_cmds prepare_cmds;
	struct lcd_kit_dsi_panel_cmds cmds;
	struct lcd_kit_dsi_panel_cmds exit_cmds;
};

struct lcd_kit_effect_color {
	u32 support;
	u32 mode;
};

struct lcd_kit_adapt_ops {
	int (*mipi_tx)(void *hld, struct lcd_kit_dsi_panel_cmds *cmds);
	int (*daul_mipi_diff_cmd_tx)(void *hld,
		struct lcd_kit_dsi_panel_cmds *dsi0_cmds,
		struct lcd_kit_dsi_panel_cmds *dsi1_cmds);
	int (*mipi_rx)(void *hld, u8 *out, int out_len,
		struct lcd_kit_dsi_panel_cmds *cmds);
	int (*gpio_enable)(u32 type);
	int (*gpio_disable)(u32 type);
	int (*gpio_disable_plugin)(u32 type);
	int (*gpio_enable_nolock)(u32 type);
	int (*gpio_disable_nolock)(u32 type);
	int (*regulator_enable)(u32 type);
	int (*regulator_disable)(u32 type);
	int (*regulator_disable_plugin)(u32 type);
	int (*buf_trans)(const char *inbuf, int inlen, char **outbuf,
		int *outlen);
	int (*lock)(void *hld);
	void (*release)(void *hld);
	void *(*get_pdata_hld)(void);
};

struct lcd_kit_backlight {
	u32 order;
	u32 bl_min;
	u32 bl_max;
	u32 need_prepare;
	struct lcd_kit_dsi_panel_cmds bl_cmd;
	struct lcd_kit_dsi_panel_cmds prepare;
};

struct lcd_kit_check_thread {
	int enable;
	int check_bl_support;
	struct hrtimer hrtimer;
	struct delayed_work check_work;
};

struct lcd_kit_common_ops {
	int (*panel_power_on)(void *hld);
	int (*panel_power_on_without_mipi)(void *hld);
	int (*panel_power_on_with_mipi)(void *hld);
	int (*panel_on_lp)(void *hld);
	int (*panel_on_hs)(void *hld);
	int (*panel_off_hs)(void *hld);
	int (*panel_off_lp)(void *hld);
	int (*panel_power_off)(void *hld);
	int (*panel_only_power_off)(void *hld);
	int (*get_panel_name)(char *buf);
	int (*get_panel_info)(char *buf);
	int (*get_cabc_mode)(char *buf);
	int (*set_cabc_mode)(void *hld, u32 mode);
	int (*get_ce_mode)(char *buf);
	int (*get_acl_mode)(char *buf);
	int (*set_acl_mode)(void *hld, u32 mode);
	int (*get_vr_mode)(char *buf);
	int (*set_vr_mode)(void *hld, u32 mode);
	int (*esd_handle)(void *hld);
	int (*dsi_handle)(void *hld);
	int (*dirty_region_handle)(void *hld, struct region_rect *dirty);
	int (*set_ce_mode)(void *hld, u32 mode);
	int (*hbm_set_handle)(void *hld, int last_hbm_level,
		int hbm_dimming, int hbm_level, int fps_status);
	int (*fp_hbm_enter_extern)(void *hld);
	int (*fp_hbm_exit_extern)(void *hld);
	int (*set_ic_dim_on)(void *hld, int fps_status);
	int (*set_effect_color_mode)(u32 mode);
	int (*get_effect_color_mode)(char *buf);
	int (*set_mipi_backlight)(void *hld, u32 bl_level);
	int (*common_init)(struct device_node *np);
	int (*get_bias_voltage)(int *vpos, int *vneg);
	void (*mipi_check)(void *pdata, char *panel_name, long display_on_record_time);
	void (*btb_check)(void);
	void (*btb_init)(void);
};

struct lcd_kit_panel_cmd_backup {
	u32 panel_on_support;
	u32 aod_support;
	u32 detect_gpio;
	u32 gpio_exp_val;
	u32 change_flag;
};

struct lcd_kit_common_info {
	/* power on check reg */
	struct lcd_kit_check_reg_dsm check_reg_on;
	/* power off check reg */
	struct lcd_kit_check_reg_dsm check_reg_off;
	/* mipi check commond */
	struct lcd_kit_mipicheck mipi_check;
	/* vss */
	struct lcd_kit_set_vss set_vss;
	/* power by thermal */
	struct lcd_kit_set_power set_power;
	/* elvdd detect */
	struct lcd_kit_elvdd_detect elvdd_detect;
	u32 panel_on_always_need_reset;
	/* effect */
	int bl_level_max;
	int bl_level_min;
	u32 ul_does_lcd_poweron_tp;
	u32 tp_gesture_sequence_flag;
	/* default max nit */
	u32 bl_max_nit;
	/* actual max nit */
	u32 actual_bl_max_nit;
	u32 bl_max_nit_min_value;
	/*  backlight delay xx ms */
	u32 bl_set_delay;
	struct lcd_kit_effect_color effect_color;
	/* cabc function */
	struct lcd_kit_cabc cabc;
	/* hbm function */
	struct lcd_kit_hbm hbm;
	/* ACL ctrl */
	struct lcd_kit_acl acl;
	/* vr mode ctrl */
	struct lcd_kit_vr vr;
	/* ce */
	struct lcd_kit_ce ce;
	/* effect on after panel on */
	struct lcd_kit_effect_on effect_on;
	/* optimize grayscale after panel on */
	struct lcd_kit_grayscale_optimize grayscale_optimize;
	/* effect after panel on in kernel */
	struct lcd_kit_screen_on_effect screen_on_effect;
	/* end */
	/* normal */
	/* panel name */
	char *panel_name;
	/* panel model */
	char *panel_model;
	/* panel information */
	char *module_info;
	/* panel type */
	u32 panel_type;
	/* lcd on command */
	struct lcd_kit_dsi_panel_cmds panel_on_cmds;
	/* lcd off command */
	struct lcd_kit_dsi_panel_cmds panel_off_cmds;
	/* esd check commond */
	struct lcd_kit_esd esd;
	/* dsi check commond */
	struct lcd_kit_dsi dsi;
	/* display region */
	struct lcd_kit_dirty dirty_region;
	/* backlight */
	struct lcd_kit_backlight backlight;
	/* check thread */
	struct lcd_kit_check_thread check_thread;
	/* get_blmaxnit */
	struct lcd_kit_blmaxnit blmaxnit;
	/* thp proximity */
	struct lcd_kit_thp_proximity thp_proximity;
	/* HBM */
	u32 hbm_mode;
	/* ALPM */
	u32 aod_no_need_init;
	bool new_doze_state;
	/* sn code */
	struct lcd_kit_sn sn_code;
	/* pre camera position */
	struct pre_camera_position p_cam_position;
	/* drf info */
	struct lcd_kit_dfr_info dfr_info;
	/* aod exit ap display on cmds */
	struct lcd_kit_dsi_panel_cmds aod_exit_dis_on_cmds;
	struct lcd_kit_ddic_alpha ddic_alpha;
	/* no bl skip support */
	int force_delta_bl_update_support;
	/* btb check info */
	unsigned int btb_support;
	unsigned int btb_check_type;
	unsigned int gpio_offset;
	struct lcd_kit_array_data lcd_btb_gpio;
	struct lcd_kit_panel_cmd_backup panel_cmd_backup;
	/* end */
};

struct lcd_kit_power_desc {
	struct lcd_kit_array_data lcd_vci;
	struct lcd_kit_array_data lcd_iovcc;
	struct lcd_kit_array_data lcd_vsp;
	struct lcd_kit_array_data lcd_vsn;
	struct lcd_kit_array_data lcd_rst;
	struct lcd_kit_array_data lcd_backlight;
	struct lcd_kit_array_data lcd_te0;
	struct lcd_kit_array_data tp_rst;
	struct lcd_kit_array_data lcd_vdd;
	struct lcd_kit_array_data lcd_aod;
	struct lcd_kit_array_data lcd_power_down_vsp;
	struct lcd_kit_array_data lcd_power_down_vsn;
	struct lcd_kit_array_data lcd_mipi_switch;
	struct lcd_kit_array_data lcd_elvdd_gpio;
};

struct lcd_kit_power_seq {
	struct lcd_kit_arrays_data power_on_seq;
	struct lcd_kit_arrays_data panel_on_lp_seq;
	struct lcd_kit_arrays_data panel_on_hs_seq;
	struct lcd_kit_arrays_data gesture_power_on_seq;
	struct lcd_kit_arrays_data panel_off_hs_seq;
	struct lcd_kit_arrays_data panel_off_lp_seq;
	struct lcd_kit_arrays_data power_off_seq;
	struct lcd_kit_arrays_data only_power_off_seq;
};

struct color_cmds_rgb {
	int red_payload[2];
	int green_payload[2];
	int blue_payload[2];
};

/* function declare */
int lcd_kit_adapt_register(struct lcd_kit_adapt_ops *ops);
struct lcd_kit_adapt_ops *lcd_kit_get_adapt_ops(void);
void lcd_kit_delay(int wait, int waittype, bool allow_sleep);
void lcd_backlight_i2c_dmd(void);
void lcd_bias_i2c_dmd(void);
#ifdef CONFIG_HUAWEI_DSM
int lcd_dsm_client_record(struct dsm_client *lcd_dclient, char *record_buf,
	int lcd_dsm_error_no, int rec_num_limit, int *cur_rec_time);
#endif
int lcd_kit_reset_power_ctrl(int enable);
int lcd_kit_get_pt_mode(void);
bool lcd_kit_get_thp_afe_status(struct timeval *record_tv);
int lcd_kit_judge_esd(unsigned char type, unsigned char read_val,
	unsigned char expect_val);
void lcd_hardware_reset(void);
int32_t lcd_kit_get_gpio_value(u32 gpio_num, const char *gpio_name);
int lcd_kit_local_hbm_set_color_cmds_value_by_index(const uint32_t fps, int gamma_node_index);
void lcd_kit_hbm_gamma_read(void *hld);
int lcd_kit_local_hbm_set_color_by_index(void *hld, uint32_t fps, uint32_t gamma_node_index);
int lcd_kit_local_hbm_set_color_by_grayscale(void *hld, uint32_t fps, int grayscale);
int lcd_kit_local_hbm_set_circle(void *hld, bool is_on);
int lcd_kit_local_hbm_set_dbv(void *hld, uint32_t dbv, uint32_t fps);
u32 lcd_kit_get_mipi_level(void);
int lcd_kit_local_hbm_alpha_circle_with_alpha_and_dbv(void *hld, uint32_t fps,
	bool is_on, uint32_t alpha, uint32_t dbv);
int lcd_kit_enter_ddic_alpha(void *hld, uint32_t alpha);
int lcd_kit_exit_ddic_alpha(void *hld, int alpha);
u32 lcd_kit_get_alpha_support(void);
u32 lcd_kit_get_force_delta_bl_update_support(void);
u32 lcd_kit_get_local_hbm_support(void);
#endif
