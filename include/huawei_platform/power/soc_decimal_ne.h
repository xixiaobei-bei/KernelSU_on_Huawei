/*
 * soc_decimal_ne.h
 *
 * notifier event of calculate soc with decimal
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

#ifndef _SOC_DECIMAL_NE_H_
#define _SOC_DECIMAL_NE_H_

/*
 * define notifier event for calculate soc with decimal
 * NE is simplified identifier with notifier event
 */
enum soc_decimal_ne_list {
	SOC_DECIMAL_NE_BEGIN = 0,
	SOC_DECIMAL_NE_DC = SOC_DECIMAL_NE_BEGIN,
	SOC_DECIMAL_NE_WL_DC,
	SOC_DECIMAL_NE_DEFAULT,
	SOC_DECIMAL_NE_END,
};

#ifdef CONFIG_HUAWEI_SOC_DECIMAL
void soc_decimal_event_notify(unsigned long event, void *data);
#else
static inline void soc_decimal_event_notify(unsigned long event, void *data)
{
}
#endif /* CONFIG_HUAWEI_SOC_DECIMAL */

#endif /* _SOC_DECIMAL_NE_H_ */
