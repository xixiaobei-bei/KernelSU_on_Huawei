/*
 * hw_typec_platform.h
 *
 * header file for typec driver
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

#ifndef _HW_TYPEC_PLATFORM_H_
#define _HW_TYPEC_PLATFORM_H_

#ifdef CONFIG_HUAWEI_TYPEC
#include <huawei_platform/usb/hw_typec_dev.h>

extern void typec_wake_lock(struct typec_device_info *di);
extern void typec_wake_unlock(struct typec_device_info *di);
extern int typec_current_notifier_register(struct notifier_block *nb);
extern int typec_detect_cc_orientation(void);
extern int typec_detect_current_mode(void);
extern int typec_detect_port_mode(void);

#else

static inline void typec_wake_lock(struct typec_device_info *di)
{
}

static inline void typec_wake_unlock(struct typec_device_info *di)
{
}

static inline int typec_current_notifier_register(struct notifier_block *nb)
{
	return -EINVAL;
}

static inline int typec_detect_cc_orientation(void)
{
	return -ENOMEM;
}

static inline int typec_detect_current_mode(void)
{
	return -ENOMEM;
}

static inline int typec_detect_port_mode(void)
{
	return -ENOMEM;
}
#endif /* CONFIG_HUAWEI_TYPEC */

#endif /* _HW_TYPEC_PLATFORM_H_ */
