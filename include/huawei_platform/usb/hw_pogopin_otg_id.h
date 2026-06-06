/*
 * hw_pogopin_otg_id.h
 *
 * huawei pogopin driver
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

#ifndef _HW_POGOPIN_OTG_ID_H_
#define _HW_POGOPIN_OTG_ID_H_

#ifndef TRUE
#define TRUE                        1
#endif
#ifndef FALSE
#define FALSE                       0
#endif

#define SAMPLE_DOING                0
#define SAMPLE_DONE                 1

#define VBUS_IS_CONNECTED           0
#define DISABLE_USB_IRQ             0
#define FAIL                        (-1)
#define SAMPLING_OPTIMIZE_FLAG      1
#define SAMPLING_INTERVAL           10
#define SMAPLING_OPTIMIZE           5
#define VBATT_AVR_MAX_COUNT         10
#define ADC_VOLTAGE_LIMIT           150
#define ADC_VOLTAGE_MAX             1250
#define ADC_VOLTAGE_NEGATIVE        2000
#define USB_CHARGER_INSERTED        1
#define USB_CHARGER_REMOVE          0
#define TIMEOUT_1000MS              1000
#define TIMEOUT_200MS               200
#define TIMEOUT_800MS               800
#define SLEEP_50MS                  50

#define OTG_INVALID_ADC             0

struct pogopin_otg_id_dev {
	struct platform_device *pdev;
	struct notifier_block otg_nb;
	struct notifier_block pogopin_nb;
	unsigned int otg_adc_channel;
	int gpio;
	int irq;
	unsigned int fpga_flag;
	unsigned int sampling_time_optimize;
	struct work_struct otg_intb_work;
	bool otg_irq_enabled;
};

#endif /* _HW_POGOPIN_OTG_ID_H_ */
