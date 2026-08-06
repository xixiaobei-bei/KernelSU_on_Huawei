/*
 * batt_info_util.h
 *
 * battery information functions for battery checkers
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

#ifndef _BATT_INFO_UTIL_H_
#define _BATT_INFO_UTIL_H_

#include <huawei_platform/power/power_mesg_srv.h>

struct batt_checker_entry {
	struct list_head node;
	struct platform_device *pdev;
};

void add_to_batt_checkers_lists(struct batt_checker_entry *entry);
int send_batt_info_mesg(struct nl_dev_info *info, void *data, unsigned int len);

#endif /* _BATT_INFO_UTIL_H_ */
