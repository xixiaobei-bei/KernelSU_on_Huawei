/*
 * dual_loadswitch.h
 *
 * dual loadswitch driver
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
 *
 */

#ifndef _DUAL_LOADSWITCH_H_
#define _DUAL_LOADSWITCH_H_

#include <huawei_platform/power/direct_charger/direct_charger.h>

struct dual_loadswitch_info {
	struct platform_device *pdev;
	struct device *dev;
	int exit_order_inversion;
};

int loadswitch_main_ops_register(struct dc_ic_ops *ops);
int loadswitch_aux_ops_register(struct dc_ic_ops *ops);

#endif /* _DUAL_LOADSWITCH_H_ */
