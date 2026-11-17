/*
 * irda_driver.h
 *
 * irda module registe interface
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
 */

#ifndef __IRDA_DRIVER_H
#define __IRDA_DRIVER_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>
#include <linux/mutex.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <huawei_platform/log/hw_log.h>

#define IRDA_DRIVER_COMPATIBLE_ID     "irda,config"
#define IRDA_CHIP_TYPE                "irda,chiptype"

#ifdef CONFIG_HUAWEI_DSM
/* use nfc client */
#define DSM_CLIENT_NAME                 "dsm_nfc"
#define DSM_IRDA_BUFFSIZE               256
#define DSM_IRDA_SEPARATED_ERROR_NO     923002018
#endif

/* struct for the chip type */
enum irda_chiptype {
	DEFAULT = 0,
	MAXIM_616, /* not use */
	HI11XX,
	HI64XX,
	OTHERS,
};

struct irda_irq_handler {
	int gpio;
	int irq;
	int irq_flag;
	int gpio_out_place;
};

/*
 * The function of register hisi irda chip, configure it through DTS.
 * return 0: successful registration or no DTS configuration.
 * return negative value: registration failure.
 */
int irda_chip_type_regist(void);
void irda_chip_type_unregist(void);
struct class *get_irda_class(void);

#endif
