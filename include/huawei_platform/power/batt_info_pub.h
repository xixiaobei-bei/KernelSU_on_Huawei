/*
 * batt_info_pub.h
 *
 * battery information public interface head file
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

#ifndef _BATT_INFO_PUB_H_
#define _BATT_INFO_PUB_H_

#define BATTERY_TYPE_BUFF_SIZE  6
#define BATTERY_TYPE_SIZE       2

#ifdef CONFIG_HUAWEI_BATTERY_INFORMATION
int get_battery_type(unsigned char name[BATTERY_TYPE_BUFF_SIZE]);
#else
inline int get_battery_type(unsigned char name[BATTERY_TYPE_BUFF_SIZE])
{
	return -1;
}
#endif /* CONFIG_HUAWEI_BATTERY_INFORMATION */

#endif /* _BATT_INFO_PUB_H_ */
