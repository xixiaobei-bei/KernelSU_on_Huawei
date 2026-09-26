/*
 * lcd_kit_parse.h
 *
 * lcdkit parse function for lcdkit head file
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

#ifndef __LCD_KIT_PARSE_H_
#define __LCD_KIT_PARSE_H_

#define LCD_KIT_CMD_REQ_MAX                  4
#define LCD_KIT_CMD_REQ_RX                   0x0001
#define LCD_KIT_CMD_REQ_COMMIT               0x0002
#define LCD_KIT_CMD_CLK_CTRL                 0x0004
#define LCD_KIT_CMD_REQ_UNICAST              0x0008
#define LCD_KIT_CMD_REQ_DMA_TPG              0x0040
#define LCD_KIT_CMD_REQ_NO_MAX_PKT_SIZE      0x0008
#define LCD_KIT_CMD_REQ_LP_MODE              0x0010
#define LCD_KIT_CMD_REQ_HS_MODE              0x0020

int lcd_kit_parse_dcs_cmds(struct device_node *np, char *cmd_key,
	const char *link_key, struct lcd_kit_dsi_panel_cmds *pcmds);
int lcd_kit_parse_array_data(struct device_node *np,
	const char *name, struct lcd_kit_array_data *out);
int lcd_kit_parse_u32_array(const struct device_node *np, const char *name,
	uint32_t out[], int len);
int lcd_kit_parse_arrays_data(struct device_node *np,
	const char *name, struct lcd_kit_arrays_data *out, int num);
int lcd_kit_parse_dirty_info(const struct device_node *np, const char *prop_name,
	int *out);
int lcd_kit_parse_u64(const struct device_node *np, const char *prop_name,
	uint64_t *out, uint64_t def);
int lcd_kit_parse_u32(const struct device_node *np, const char *prop_name,
	unsigned int *out, unsigned int def);
int lcd_kit_parse_u8(const struct device_node *np, const char *prop_name,
	unsigned char *out, unsigned char def);
int lcd_kit_parse_string_array(const struct device_node *np, const char *prop_name,
	const char **out_strs, size_t sz);
int lcd_kit_parse_string(const struct device_node *np, const char *prop_name,
	const char **out_strs);
#endif
