/*
 * charge_pump.h
 *
 * charge pump driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _CHARGE_PUMP_H_
#define _CHARGE_PUMP_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#define CP_I2C_RETRY_CNT                3
#define CP_BP_RATIO                     1
#define CP_CP_RATIO                     2
#define CP_GET_OPS_RETRY_CNT            3
#define CP_OPS_BYPASS                   BIT(10)

#define CP_TYPE_MAIN                    BIT(0)
#define CP_TYPE_AUX                     BIT(1)
#define CP_TYPE_MULTI                   (CP_TYPE_MAIN | CP_TYPE_AUX)

enum charge_pump_device_id {
	CP_DEVICE_ID_BEGIN = 0,
	CP_DEVICE_ID_HL1506_MAIN = CP_DEVICE_ID_BEGIN,
	CP_DEVICE_ID_HL1506_AUX,
	CP_DEVICE_ID_SY6510,
	CP_DEVICE_ID_PCA9488,
	CP_DEVICE_ID_HL1512,
	CP_DEVICE_ID_SC8502,
	CP_DEVICE_ID_END,
};

struct charge_pump_device_data {
	unsigned int id;
	const char *name;
};

struct charge_pump_ops {
	void *dev_data;
	unsigned int cp_type;
	const char *chip_name;
	int (*dev_check)(void *);
	int (*post_probe)(void *);
	int (*chip_init)(void *); /* forward charge chip init */
	int (*rvs_chip_init)(void *); /* reverse charge chip init */
	int (*rvs_cp_chip_init)(void *); /* reverse charge cp chip init */
	int (*set_bp_mode)(void *); /* forward bypass mode */
	int (*set_cp_mode)(void *); /* forward 2:1 cp_mode */
	int (*set_rvs_bp_mode)(void *); /* reverse bypass mode */
	int (*set_rvs_cp_mode)(void *); /* reverse 2:1 cp_mode */
	bool (*is_bp_open)(void *);
	bool (*is_cp_open)(void *);
	int (*get_cp_ratio)(void *);
	int (*get_cp_vout)(void *);
	int (*set_rvs_bp2cp_mode)(void *);
};

struct charge_pump_dev {
	int gpio_main_en;
	int gpio_main_cp_en_val;
	int gpio_aux_en;
	int gpio_aux_cp_en_val;
	struct charge_pump_ops *p_ops;
	struct charge_pump_ops *t_ops[CP_DEVICE_ID_END];
};

#ifdef CONFIG_CHARGE_PUMP
extern int charge_pump_ops_register(struct charge_pump_ops *ops);
extern void charge_pump_chip_enable(unsigned int type, bool enable);
extern int charge_pump_chip_init(unsigned int type);
extern int charge_pump_reverse_chip_init(unsigned int type);
extern int charge_pump_reverse_cp_chip_init(unsigned int type);
extern int charge_pump_set_bp_mode(unsigned int type);
extern int charge_pump_set_cp_mode(unsigned int type);
extern int charge_pump_set_reverse_bp_mode(unsigned int type);
extern int charge_pump_set_reverse_cp_mode(unsigned int type);
extern bool charge_pump_is_bp_open(unsigned int type);
extern bool charge_pump_is_cp_open(unsigned int type);
extern int charge_pump_get_cp_ratio(unsigned int type);
extern int charge_pump_get_cp_vout(unsigned int type);
extern int charge_pump_set_reverse_bp2cp_mode(unsigned int type);
#else
static inline int charge_pump_ops_register(struct charge_pump_ops *ops)
{
	return -1;
}

static inline void charge_pump_chip_enable(unsigned int type, bool enable)
{
}

static inline int charge_pump_chip_init(unsigned int type)
{
	return -1;
}

static inline int charge_pump_reverse_chip_init(unsigned int type)
{
	return -1;
}

static inline int charge_pump_reverse_cp_chip_init(unsigned int type)
{
	return -1;
}

static inline int charge_pump_set_bp_mode(unsigned int type)
{
	return -1;
}

static inline int charge_pump_set_cp_mode(unsigned int type)
{
	return -1;
}

static inline int charge_pump_set_reverse_bp_mode(unsigned int type)
{
	return -1;
}

static inline int charge_pump_set_reverse_cp_mode(unsigned int type)
{
	return -1;
}

static inline bool charge_pump_is_bp_open(unsigned int type)
{
	return true;
}

static inline bool charge_pump_is_cp_open(unsigned int type)
{
	return false;
}

static inline int charge_pump_get_cp_ratio(unsigned int type)
{
	return CP_BP_RATIO;
}

static inline int charge_pump_get_cp_vout(unsigned int type)
{
	return -1;
}

static inline int charge_pump_set_reverse_bp2cp_mode(unsigned int type)
{
	return -1;
}
#endif /* CONFIG_CHARGE_PUMP */

#endif /* _CHARGE_PUMP_H_ */
