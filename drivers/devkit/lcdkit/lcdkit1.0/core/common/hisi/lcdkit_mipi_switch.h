/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: mipi switch between AP and MCU
 * Author: lijiawen
 * Create: 2020-04-20
 */
#ifndef __LCDKIT_MIPI_SWITCH_H_
#define __LCDKIT_MIPI_SWITCH_H_
#include "hisi_fb.h"

void mipi_to_ap(struct hisi_fb_data_type *hisifd);

void mipi_to_mcu(struct hisi_fb_data_type *hisifd);

void mipi_switch_release(struct platform_device* pdev);

void mipi_switch_init(struct platform_device* pdev);

int check_display_on(struct hisi_fb_data_type *hisifd);

int i2c_switch_ap(void);

int get_i2c_switch_value(void);

void read_watch_reg_value(struct hisi_fb_data_type *hisifd);

int get_watch_recovery_state(void);

#ifdef CONFIG_EXT_INPUTHUB
int ext_reset_lcd_command(void);
#endif

#endif
