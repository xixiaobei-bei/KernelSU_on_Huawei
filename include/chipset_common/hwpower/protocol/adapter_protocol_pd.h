/*
 * adapter_protocol_pd.h
 *
 * pd protocol driver
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

#ifndef _ADAPTER_PROTOCOL_PD_H_
#define _ADAPTER_PROTOCOL_PD_H_

#define PD_ADAPTER_5V             5000
#define PD_ADAPTER_9V             9000
#define PD_ADAPTER_12V            12000

struct pd_protocol_ops {
	const char *chip_name;
	void *dev_data;
	void (*hard_reset_master)(void *);
	void (*set_output_voltage)(int volt, void *);
};

struct pd_protocol_dev {
	struct device *dev;
	struct pd_protocol_ops *p_ops;
	int volt;
	int dev_id;
};

#ifdef CONFIG_ADAPTER_PROTOCOL_PD
int pd_protocol_ops_register(struct pd_protocol_ops *ops);
#else
static inline int pd_protocol_ops_register(struct pd_protocol_ops *ops)
{
	return -1;
}
#endif /* CONFIG_ADAPTER_PROTOCOL_PD */

#endif /* _ADAPTER_PROTOCOL_PD_H_ */
