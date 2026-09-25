/*
 * charge_pump.h
 *
 * charge pump driver
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#define CP_I2C_WR_MSG_LEN               1
#define CP_I2C_RD_MSG_LEN               2
#define CP_I2C_RETRY_CNT                3
#define CP_BYTE_LEN                     1
#define CP_WORD_LEN                     2
#define CP_BP_RATIO                     1
#define CP_CP_RATIO                     2
#define CP_GET_OPS_RETRY_CNT            3

enum charge_pump_device_id {
	CP_DEVICE_ID_BEGIN = 0,
	CP_DEVICE_ID_HL1506 = CP_DEVICE_ID_BEGIN,
	CP_DEVICE_ID_SY6510,
	CP_DEVICE_ID_PCA9488,
	CP_DEVICE_ID_HL1512,
	CP_DEVICE_ID_END,
};

struct charge_pump_device_data {
	unsigned int id;
	const char *name;
};

struct charge_pump_ops {
	const char *chip_name;
	int (*dev_check)(void);
	int (*post_probe)(void);
	int (*chip_init)(void);
	int (*reverse_chip_init)(void);
	int (*set_bp_mode)(void); /* bypass mode */
	int (*set_cp_mode)(void); /* charge pump mode */
	bool (*is_bp_open)(void);
	bool (*is_cp_open)(void);
	int (*get_cp_ratio)(void);
	int (*get_cp_vout)(void);
};

struct charge_pump_dev {
	struct charge_pump_ops *p_ops;
	struct charge_pump_ops *t_ops[CP_DEVICE_ID_END];
};

#ifdef CONFIG_CHARGE_PUMP
extern int charge_pump_ops_register(struct charge_pump_ops *ops);
extern int charge_pump_chip_init(void);
extern int charge_pump_reverse_chip_init(void);
extern int charge_pump_set_bp_mode(void);
extern int charge_pump_set_cp_mode(void);
extern bool charge_pump_is_bp_open(void);
extern bool charge_pump_is_cp_open(void);
extern int charge_pump_get_cp_ratio(void);
extern int charge_pump_get_cp_vout(void);
#else
static inline int charge_pump_ops_register(struct charge_pump_ops *ops)
{
	return -1;
}

static inline int charge_pump_chip_init(void)
{
	return -1;
}

static inline int charge_pump_reverse_chip_init(void)
{
	return -1;
}

static inline int charge_pump_set_bp_mode(void)
{
	return -1;
}

static inline int charge_pump_set_cp_mode(void)
{
	return -1;
}

static inline bool charge_pump_is_bp_open(void)
{
	return true;
}

static inline bool charge_pump_is_cp_open(void)
{
	return false;
}

static inline int charge_pump_get_cp_ratio(void)
{
	return CP_BP_RATIO;
}

static inline int charge_pump_get_cp_vout(void)
{
	return -1;
}
#endif /* CONFIG_CHARGE_PUMP */

#endif  /* _CHARGE_PUMP_H_ */
