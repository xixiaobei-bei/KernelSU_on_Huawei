/*
 * direct_charger_para_parse.h
 *
 * parameter parse interface for direct charger
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

#ifndef _DIRECT_CHARGER_PARA_PARSE_H_
#define _DIRECT_CHARGER_PARA_PARSE_H_

#ifdef CONFIG_DIRECT_CHARGER
int direct_charge_parse_dts(struct device_node *np, void *p);
#else
static inline int direct_charge_parse_dts(struct device_node *np, void *p)
{
	return -1;
}
#endif /* CONFIG_DIRECT_CHARGER */

#ifdef CONFIG_SCHARGER_V300
extern bool is_hi6523_cv_limit(void);
#endif

#endif /* _DIRECT_CHARGER_PARA_PARSE_H_ */
