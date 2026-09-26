/* Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HISI_FB_PANEL_DEBUG_H_
#define __HISI_FB_PANEL_DEBUG_H_

#include "hisi_fb.h"
#include <linux/debugfs.h>
#include <linux/ctype.h>
#include <linux/syscalls.h>
#include <linux/string.h>
#include <stdarg.h>

#define HISI_FB_FAIL (-1)
#define HISI_FB_OK (0)

#define HISI_FB_PANEL_CONFIG_TABLE_MAX_NUM (2 * PAGE_SIZE)
#define HISI_FB_PANEL_PARAM_FILE_PATH "/data/lcd_param_config.xml"
#define HISI_FB_PANEL_ITEM_NAME_MAX   100
#define HISI_FB_PANEL_DBG_BUFF_MAX    2048
#define DPU_MIPI_PHY_MAX_CFG_NUM      32
#define MIPI_PHY_TEST_START_OFFSET    0x10000

enum hisi_fb_panel_parse_status {
	PARSE_HEAD,
	RECEIVE_DATA,
	PARSE_FINAL,
	NOT_MATCH,
	INVALID_DATA,
};
enum hisi_fb_panel_cmds_type {
	HISI_FB_PANEL_DBG_PARAM_CONFIG = 0,
	HISI_FB_PANEL_DBG_NUM_MAX,
};

#define PSTR_LEN (100)
struct hisi_fb_panel_dbg_cmds {
	char type;
	char pstr[PSTR_LEN];
};

struct hisi_fb_panel_dbg_u8 {
	unsigned char *name;
	uint8_t *addr;
};

struct hisi_fb_panel_dbg_u32 {
	unsigned char *name;
	uint32_t *addr;
};

typedef int (*DBG_FUNC)(unsigned char *item, char *par);
struct hisi_fb_panel_dbg_func {
	unsigned char *name;
	DBG_FUNC func;
};
struct mipi_phy_tab_st {
	uint32_t addr;
	uint32_t value;
};

struct mipi_phy_tab_info {
	uint32_t config_count;
	struct mipi_phy_tab_st mipi_phy_tab[DPU_MIPI_PHY_MAX_CFG_NUM];
};

int hisi_fb_panel_debugfs_init(void);
void dpu_mipi_phy_debug_config(void);

#endif
