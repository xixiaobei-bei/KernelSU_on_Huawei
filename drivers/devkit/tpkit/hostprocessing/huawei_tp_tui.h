/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#ifndef _TP_TUI_H_
#define _TP_TUI_H_

#if defined(CONFIG_TEE_TUI)

#define THP_PROJECT_ID_LEN 10
#define TP_TUI_NEW_IRQ_SUPPORT 1
#define TP_TUI_NEW_IRQ_MASK 0x1
#define TP_TUI_DEV_NAME_SIZE 11
#define TP_TUI_RESERVED_ARRAY_SIZE 4
#define MAX_SUPPORT_TP_TUI_NUM 2
static unsigned char tui_enable;
struct thp_tui_data {
	char project_id[THP_PROJECT_ID_LEN + 1];
	unsigned char enable;
	u16 frame_data_addr;
	/* the size of thp_tui_data is 24 byte in tee os */
	unsigned char reserved[10];
};

struct ts_tui_data {
	char device_name[TP_TUI_DEV_NAME_SIZE];
	/* thp_info is reserved for thp struct in tee os */
	unsigned char thp_info[TP_TUI_RESERVED_ARRAY_SIZE];
	/* tui_special_feature_support's bit0: set new irq gpio support */
	unsigned short tui_special_feature_support;
	unsigned short tui_irq_gpio;
	unsigned short tui_irq_num;
	/* the size of ts_tui_data is 24 byte in tee os */
	unsigned char reserved[2];
};

struct multi_tp_tui_data {
	union {
		struct thp_tui_data thp_tui_data;
		struct ts_tui_data tpkit_tui_data;
	} tp_tui_normalized_data;
	int (*tui_drv_switch)(void *pdata, int secure);
	bool panel_in_tui;
};

extern struct multi_tp_tui_data tp_tui_data[MAX_SUPPORT_TP_TUI_NUM];
#endif

#endif /* _THP_H_ */

