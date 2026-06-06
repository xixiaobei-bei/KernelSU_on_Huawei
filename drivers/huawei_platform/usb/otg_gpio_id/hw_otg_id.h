/*
 * hw_otg_id.h
 *
 * gpio based for otgid driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HW_OTG_ID_
#define _HW_OTG_ID_

#ifdef CONFIG_DEC_USB
#include "dwc_otg_dec.h"
#include "dwc_otg_cil.h"
#endif /* CONFIG_DEC_USB */

#define SAMPLE_DOING     0
#define SAMPLE_DONE      1

#define GPIO_HIGH        1
#define GPIO_LOW         0

struct otg_gpio_id_dev {
	struct platform_device *pdev;
	struct notifier_block otg_nb;
	unsigned int otg_adc_channel;
	int gpio;
	int irq;
	unsigned int fpga_flag;
	unsigned int sampling_time_optimize;
	struct work_struct otg_intb_work;
	bool otg_irq_enabled;
};

#ifdef CONFIG_OTG_GPIO_ID
extern int hw_get_otg_id_gpio_value(int *gpio_value);

#else

static inline int hw_get_otg_id_gpio_value(int *gpio_value)
{
	return 0;
}
#endif /* CONFIG_OTG_GPIO_ID */

#endif /* _HW_OTG_ID_ */
