/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
 * Description:  Register config for USB on HiSilicon platform
 * Author: Hilisicon
 * Create: 2017-06-23
 */

#ifndef __HISI_USB_REG_CFG_H__
#define __HISI_USB_REG_CFG_H__

#include <linux/regmap.h>
#include <linux/mfd/syscon.h>

enum reg_cfg_type {
	WRITE_ONLY,
	BIT_MASK,
	READ_WRITE,
	TEST_READ,
	REG_CFG_TYPE_MAX
};

struct chip_usb_reg_cfg {
	struct regmap *regmap;
	enum reg_cfg_type cfg_type;
	unsigned int offset;
	unsigned int value;
	unsigned int mask;
};

extern struct chip_usb_reg_cfg *of_get_chip_usb_reg_cfg(
		struct device_node *np,
		const char *prop_name);
extern int get_chip_usb_reg_cfg_array(struct device *dev, const char *prop_name,
		struct chip_usb_reg_cfg ***reg_cfgs, int *num_cfgs);
extern void of_remove_chip_usb_reg_cfg(struct chip_usb_reg_cfg *reg_cfg);
extern void free_chip_usb_reg_cfg_array(struct chip_usb_reg_cfg **reg_cfgs,
					int num_cfgs);
extern int chip_usb_reg_write(struct chip_usb_reg_cfg *reg_cfg);
extern int chip_usb_reg_test_cfg(struct chip_usb_reg_cfg *reg_cfg);
extern int chip_usb_reg_write_array(struct chip_usb_reg_cfg **reg_cfgs,
				    int num_cfgs);

#endif /* __HISI_USB_REG_CFG_H__ */
