/*
 * power_mesg.h
 *
 * power netlink message head file
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

#ifndef _POWER_MESG_H_
#define _POWER_MESG_H_

#include <linux/device.h>
#include <linux/power/hisi/hisi_bci_battery.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/power/power_mesg_type.h>
#include <huawei_platform/power/power_genl.h>

int power_easy_send(power_mesg_node_t *node, unsigned char cmd,
		    unsigned char version, void *data, unsigned int len);
int power_easy_node_register(power_mesg_node_t *node);

#endif /* _POWER_MESG_H_ */
