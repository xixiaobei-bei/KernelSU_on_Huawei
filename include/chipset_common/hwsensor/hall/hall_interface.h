/*
 * hall_interface.h
 *
 * interface for huawei hall driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _HALL_INTERFACE_H_
#define _HALL_INTERFACE_H_

struct hall_interface_ops {
	void *dev_data;
	bool (*get_hall_status)(void);
};

struct hall_interface_dev {
	struct hall_interface_ops *p_ops;
};

#ifdef CONFIG_HUAWEI_HALL_INTERFACE
int hall_interface_ops_register(struct hall_interface_ops *ops);
bool hall_interface_get_hall_status(void);
#else
static inline int hall_interface_ops_register(struct hall_interface_ops *ops)
{
	return -1;
}

static inline bool hall_interface_get_hall_status(void)
{
	return false;
}
#endif /* CONFIG_HUAWEI_HALL_INTERFACE */

#endif /* _HALL_INTERFACE_H_ */
