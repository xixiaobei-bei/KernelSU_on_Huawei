/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: analog headset switch driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#ifndef __ANA_HS_CORE_H__
#define __ANA_HS_CORE_H__

#include "huawei_platform/usb/hw_pd_dev.h"
#include "../ana_hs.h"
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

/* reg val_bits */
#define ANA_HS_REG_VALUE_B8     8
#define ANA_HS_REG_VALUE_B16    16
#define ANA_HS_REG_VALUE_B24    24
#define ANA_HS_REG_VALUE_B32    32

/* reg value mask by reg val_bits */
#define ANA_HS_REG_VALUE_M8     0xFF
#define ANA_HS_REG_VALUE_M16    0xFFFF
#define ANA_HS_REG_VALUE_M24    0xFFFFFF
#define ANA_HS_REG_VALUE_M32    0xFFFFFFFF

#define ana_hs_kfree_ops(_p) \
do { \
	if ((_p) != NULL) { \
		kfree(_p); \
		(_p) = NULL; \
	} \
} while (0)

#define ANA_HS_ENABLE    0
#define ANA_HS_DISABLE   1

enum {
	ANA_HS_GPIO        = 0,
	ANA_HS_I2C         = 1,
};

struct ana_hs_node {
	unsigned int key;
	unsigned int value;
	unsigned int mask;
	unsigned int delay;    // ms
};

struct ana_hs_sequence_data {
	unsigned int num;
	struct ana_hs_node *nodes;
};

struct ana_hs_regmap_cfg {
	/* write reg or update_bits */
	unsigned int value_mask;

	/* regmap config */
	int num_writeable;
	int num_unwriteable;
	int num_readable;
	int num_unreadable;
	int num_volatile;
	int num_unvolatile;
	int num_defaults;

	unsigned int *reg_writeable;
	unsigned int *reg_unwriteable;
	unsigned int *reg_readable;
	unsigned int *reg_unreadable;
	unsigned int *reg_volatile;
	unsigned int *reg_unvolatile;
	struct reg_default *reg_defaults;

	struct regmap_config cfg;

	struct regmap *regmap;
};

struct ana_hs_irq_handler {
	int gpio;
	int irq;
	int func_id;
	unsigned int irq_flag;
};

struct ana_hs_i2c_data {
	struct device *dev;
	struct i2c_client *i2c;
	/* reg map config */
	struct ana_hs_regmap_cfg *regmap_cfg;
	struct ana_hs_irq_handler *handler;
	int init_id;
	int remove_id;
};

struct ana_hs_bus_ctl_ops {
	int (*read_regs)(struct regmap *regmap,
		unsigned int reg_addr, unsigned int *value);
	int (*write_regs)(struct regmap *regmap,
		unsigned int reg_addr, unsigned int value);
	int (*update_bits)(struct regmap *regmap, unsigned int reg_addr,
		unsigned int mask, unsigned int value);
};

#ifdef CONFIG_ANA_HS_CORE
int ana_hs_get_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num);
int ana_hs_parse_sequence_data(struct ana_hs_sequence_data **data,
	struct device_node *node, const char *seq_str);
int ana_hs_register_bus_ops(struct ana_hs_bus_ctl_ops *ops, void *data);
int ana_hs_get_misc_init_flag(void);
void ana_hs_set_gpio_state(int enable);
int ana_hs_bus_read(unsigned int addr, unsigned int *value);
int ana_hs_bus_write(unsigned int addr, unsigned int value);
#else
static inline int ana_hs_get_u32_array(struct device_node *node,
	const char *propname, u32 **value, int *num)
{
	return 0;
}

static inline int ana_hs_parse_sequence_data(struct ana_hs_sequence_data **data,
	struct device_node *node, const char *seq_str)
{
	return 0;
}

static inline int ana_hs_register_bus_ops(struct ana_hs_bus_ctl_ops *ops, void *data)
{
	return 0;
}

static inline int ana_hs_get_misc_init_flag(void)
{
	return 0;
}

static inline void ana_hs_set_gpio_state(int enable)
{
}

static inline int ana_hs_bus_read(unsigned int addr, unsigned int *value)
{
	return 0;
}

static inline int ana_hs_bus_write(unsigned int addr, unsigned int value)
{
	return 0;
}
#endif
#endif /* __ANA_HS_H__ */
