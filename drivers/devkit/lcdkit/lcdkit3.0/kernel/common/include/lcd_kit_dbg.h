/*
 * lcd_kit_dbg.h
 *
 * lcdkit debug function for lcdkit head file
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

#ifndef __LCD_KIT_DBG_H_
#define __LCD_KIT_DBG_H_

#include <linux/debugfs.h>
#include <linux/ctype.h>
#include <linux/syscalls.h>
#include "lcd_kit_common.h"

/* define macro */
#define LCD_KIT_DCS_STR              ("dcs_")
#define LCD_KIT_GEN_STR              ("gen_")
#define LCD_KIT_READ_STR             ("read_")
#define LCD_KIT_WRITE_STR            ("write_")

#define LCD_KIT_MAX_PARAM_NUM        25

#define LCD_KIT_OPER_READ           1
#define LCD_KIT_OPER_WRITE          2
#define LCD_KIT_MIPI_PATH_OPEN      1
#define LCD_KIT_MIPI_PATH_CLOSE     0
#define LCD_KIT_MIPI_DCS_COMMAND    (1<<0)
#define LCD_KIT_MIPI_GEN_COMMAND    4

#define LCD_KIT_HEX_BASE ((char)16)

/* dcs read/write */
#define DTYPE_DCS_WRITE     0x05 /* short write, 0 parameter */
#define DTYPE_DCS_WRITE1    0x15 /* short write, 1 parameter */
#define DTYPE_DCS_READ      0x06 /* read */
#define DTYPE_DCS_LWRITE    0x39 /* long write */

/* generic read/write */
#define DTYPE_GEN_WRITE     0x03 /* short write, 0 parameter */
#define DTYPE_GEN_WRITE1    0x13 /* short write, 1 parameter */
#define DTYPE_GEN_WRITE2    0x23 /* short write, 2 parameter */
#define DTYPE_GEN_LWRITE    0x29 /* long write */
#define DTYPE_GEN_READ      0x04 /* long read, 0 parameter */
#define DTYPE_GEN_READ1     0x14 /* long read, 1 parameter */
#define DTYPE_GEN_READ2     0x24 /* long read, 2 parameter */

/*
 * Message printing priorities:
 * LEVEL 0 KERN_EMERG (highest priority)
 * LEVEL 1 KERN_ALERT
 * LEVEL 2 KERN_CRIT
 * LEVEL 3 KERN_ERR
 * LEVEL 4 KERN_WARNING
 * LEVEL 5 KERN_NOTICE
 * LEVEL 6 KERN_INFO
 * LEVEL 7 KERN_DEBUG (Lowest priority)
 */

#ifndef TRUE
#define TRUE  1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define VCC_LCD_KIT_BIAS_NAME  "vcc_lcdbias"
#define VCC_LCD_KIT_VSN_NAME   "lcd_vsn"
#define VCC_LCD_KIT_VSP_NAME   "lcd_vsp"

#define LCD_KIT_VREG_VDD_NAME   "vdd"
#define LCD_KIT_VREG_LAB_NAME   "lab"
#define LCD_KIT_VREG_IBB_NAME   "ibb"
#define LCD_KIT_VREG_BIAS_NAME  "bias"
#define LCD_KIT_VREG_VDDIO_NAME "vddio"

#define LCD_KIT_INIT_TEST_PARAM      "/data/lcd_kit_init_param.txt"
#define LCD_KIT_CONFIG_TABLE_MAX_NUM (2 * PAGE_SIZE)
#define LCD_KIT_CMD_MAX_NUM          (PAGE_SIZE)
#define LCD_KIT_DCS_CMD_MAX_NUM (4 * PAGE_SIZE)

#define LCD_KIT_PARAM_FILE_PATH "/data/lcd_param_config.xml"
#define LCD_KIT_ITEM_NAME_MAX   100
#define LCD_KIT_DBG_BUFF_MAX    2048
/* cmdline buffer max */
#define CMDLINE_MAX 10
#define XML_FILE_MAX 100
#define XML_NAME_MAX 100

enum lcd_kit_parse_status {
	PARSE_HEAD,
	RECEIVE_DATA,
	PARSE_FINAL,
	NOT_MATCH,
	INVALID_DATA,
};
enum lcd_kit_cmds_type {
	LCD_KIT_DBG_LEVEL_SET = 0,
	LCD_KIT_DBG_INIT_CODE,
	LCD_KIT_DBG_PARAM_CONFIG,
	LCD_KIT_DBG_NUM_MAX,
};


struct lcd_kit_dsi_ctrl_hdr {
	char dtype; /* data type */
	char last;  /* last in chain */
	char vc;    /* virtual chan */
	char ack;   /* ask ACK from peripheral */
	char wait;  /* ms */
	char waittype;
	char dlen;  /* 8 bits */
};

#define PSTR_LEN 100
struct lcd_kit_dbg_cmds {
	char type;
	char pstr[PSTR_LEN];
};

typedef int (*DBG_FUNC)(char *PAR);
struct lcd_kit_dbg_func {
	unsigned char *name;
	DBG_FUNC func;
};

struct lcd_kit_debug {
	char *dbg_esd_cmds;
	char *dbg_on_cmds;
	char *dbg_off_cmds;
	char *dbg_effect_on_cmds;
	char *dbg_cabc_off_cmds;
	char *dbg_cabc_ui_cmds;
	char *dbg_cabc_still_cmds;
	char *dbg_cabc_moving_cmds;
	char *dbg_rgbw_mode1_cmds;
	char *dbg_rgbw_mode2_cmds;
	char *dbg_rgbw_mode3_cmds;
	char *dbg_rgbw_mode4_cmds;
	char *dbg_rgbw_backlight_cmds;
	char *dbg_rgbw_pixel_gainlimit_cmds;
	char *dbg_dirty_region_cmds;
	char *dbg_barcode_2d_cmds;
	char *dbg_brightness_color_cmds;
	char *dbg_cmds;
	unsigned int *dbg_power_on_array;
	unsigned int *dbg_lp_on_array;
	unsigned int *dbg_hs_on_array;
	unsigned int *dbg_hs_off_array;
	unsigned int *dbg_lp_off_array;
	unsigned int *dbg_power_off_array;
};

struct lcd_kit_dbg_ops {
	int (*fps_updt_support)(int val);
	int (*quickly_sleep_out_support)(int val);
	int (*blpwm_input_support)(int val);
	int (*dsi_upt_support)(int val);
	int (*rgbw_support)(int val);
	int (*gamma_support)(int val);
	int (*gmp_support)(int val);
	int (*hiace_support)(int val);
	int (*xcc_support)(int val);
	int (*arsr1psharpness_support)(int val);
	int (*prefixsharptwo_d_support)(int val);
	int (*video_idle_mode_support)(int val);
	int (*cmd_type)(int val);
	int (*pxl_clk)(int val);
	int (*pxl_clk_div)(int val);
	int (*vsync_ctrl_type)(int val);
	int (*hback_porch)(int val);
	int (*hfront_porch)(int val);
	int (*hpulse_width)(int val);
	int (*vback_porch)(int val);
	int (*vfront_porch)(int val);
	int (*vpulse_width)(int val);
	int (*mipi_burst_mode)(int val);
	int (*mipi_max_tx_esc_clk)(int val);
	int (*mipi_dsi_bit_clk)(int val);
	int (*mipi_dsi_bit_clk_a)(int val);
	int (*mipi_dsi_bit_clk_b)(int val);
	int (*mipi_dsi_bit_clk_c)(int val);
	int (*mipi_dsi_bit_clk_d)(int val);
	int (*mipi_dsi_bit_clk_e)(int val);
	int (*mipi_noncontinue_enable)(int val);
	int (*mipi_rg_vcm_adjust)(int val);
	int (*mipi_clk_post_adjust)(int val);
	int (*mipi_clk_pre_adjust)(int val);
	int (*mipi_clk_ths_prepare_adjust)(int val);
	int (*mipi_clk_tlpx_adjust)(int val);
	int (*mipi_clk_ths_trail_adjust)(int val);
	int (*mipi_clk_ths_exit_adjust)(int val);
	int (*mipi_clk_ths_zero_adjust)(int val);
	int (*mipi_lp11_flag)(int val);
	int (*mipi_phy_update)(int val);
	int (*rgbw_bl_max)(int val);
	int (*rgbw_set_mode1)(char *buf, int len);
	int (*rgbw_set_mode2)(char *buf, int len);
	int (*rgbw_set_mode3)(char *buf, int len);
	int (*rgbw_set_mode4)(char *buf, int len);
	int (*rgbw_backlight_cmd)(char *buf, int len);
	int (*rgbw_pixel_gainlimit_cmd)(char *buf, int len);
	int (*barcode_2d_cmd)(char *buf, int len);
	int (*brightness_color_cmd)(char *buf, int len);
	int (*set_voltage)(void);
	int (*dbg_mipi_rx)(uint8_t *out, int out_len,
		struct lcd_kit_dsi_panel_cmds *cmds);
	bool (*panel_power_on)(void);
	int (*esd_check_func)(void);
};

int lcd_kit_debugfs_init(void);
int lcd_kit_debug_register(struct lcd_kit_dbg_ops *ops);
int lcd_kit_debug_unregister(struct lcd_kit_dbg_ops *ops);
struct lcd_kit_dbg_ops *lcd_kit_get_debug_ops(void);
int lcd_kit_dbg_parse_cmd(struct lcd_kit_dsi_panel_cmds *pcmds, char *buf,
	int length);
bool lcd_kit_is_valid_char(char ch);
bool is_valid_char(char ch);
int lcd_parse_u32(const struct device_node *np, const char *item,
	unsigned int *out, unsigned int def);
int lcd_parse_u8(const struct device_node *np, const char *item,
	unsigned char *out, unsigned char def);
int lcd_parse_dcs(const struct device_node *np, const char *item,
	const char *item_state, struct lcd_kit_dsi_panel_cmds *pcmds);
int lcd_parse_arrays(const struct device_node *np, const char *item,
	struct lcd_kit_arrays_data *out, int size);
int lcd_parse_array(const struct device_node *np, const char *item,
	struct lcd_kit_array_data *out);
char *lcd_parse_string(const struct device_node *np, const char *item);
int lcd_parse_string_array(const struct device_node *np, const char *item,
	const char **out);
void lcd_parse_dirty_region(const struct device_node *np, const char *item,
	int *out);
bool is_valid_char(char ch);
int lcd_kit_str_to_del_invalid_ch(char *str);
int lcd_kit_parse_u32_digit(char *in, unsigned int *out, int len);
int lcd_kit_parse_u8_digit(char *in, char *out, int max);
int lcd_kit_dbg_parse_cmd(struct lcd_kit_dsi_panel_cmds *pcmds, char *buf,
	int length);
int lcd_kit_str_to_del_ch(char *str, char ch);
int lcd_kit_dbg_parse_array(char *in, unsigned int *array,
	struct lcd_kit_arrays_data *out, int len);
int is_dpd_mode(void);
int file_sys_is_ready(void);
void detect_fs_wq_handler(struct work_struct *work);
#endif
