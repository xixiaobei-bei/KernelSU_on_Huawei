/*
 * hisi_usb_helper.h
 *
 * Hisilicon usb interface defination
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _CHIP_USB_HELPER_H_
#define _CHIP_USB_HELPER_H_

#include <linux/usb/ch9.h>
#include <linux/clk.h>
#include <linux/of.h>
#include <linux/hisi/usb/hisi_usb.h>

#define CHIP_USB_GET_CLKS_MAX_NUM 10

const char *charger_type_string(enum hisi_charger_type type);
const char *chip_usb_state_string(enum usb_state state);
enum hisi_charger_type get_charger_type_from_str(const char *buf, size_t size);
enum usb_device_speed usb_speed_to_string(const char *maximum_speed, size_t len);
void __iomem *of_devm_ioremap(struct device *dev, const char *compatible);
int chip_usb_get_clks(struct device_node *node, struct clk ***clks, int *num_clks);
int chip_usb_put_clks(struct clk **clks, int num_clks);
int devm_chip_usb_get_clks(struct device *dev, struct clk ***clks, int *num_clks);
int chip_usb_init_clks(struct clk **clks, int num_clks);
void chip_usb_shutdown_clks(struct clk **clks, int num_clks);

#endif /* _CHIP_USB_HELPER_H_ */
