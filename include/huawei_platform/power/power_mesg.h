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

#include <huawei_platform/power/power_mesg_srv.h>
#include <huawei_platform/power/power_mesg_type.h>
#include <huawei_platform/power/power_genl.h>

#ifdef CONFIG_HUAWEI_POWER_MESG_INTERFACE
int power_easy_send(struct power_mesg_node *node,
	unsigned char cmd, unsigned char version, void *data, unsigned int len);
int power_easy_node_register(struct power_mesg_node *node);
int power_mesg_create_attr(const struct device_attribute *attr);
#else
static inline int power_easy_send(struct power_mesg_node *node,
	unsigned char cmd, unsigned char version, void *data, unsigned int len)
{
	return -1;
}

static inline int power_easy_node_register(struct power_mesg_node *node)
{
	return -1;
}

int power_mesg_create_attr(const struct device_attribute *attr)
{
	return -1;
}
#endif /* CONFIG_HUAWEI_POWER_MESG_INTERFACE */

#endif /* _POWER_MESG_H_ */
