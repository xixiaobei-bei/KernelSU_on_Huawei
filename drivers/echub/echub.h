/*
 * echub/echub.h
 *
 * echub interface & definition
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HUAWEI_ECHUB
#define _HUAWEI_ECHUB

/*
 * Message printing priorities: printk(KERN_EMERG []
 * LEVEL 0 KERN_EMERG (highest priority)
 * LEVEL 1 KERN_ALERT
 * LEVEL 2 KERN_CRIT
 * LEVEL 3 KERN_ERR
 * LEVEL 4 KERN_WARNING
 * LEVEL 5 KERN_NOTICE
 * LEVEL 6 KERN_INFO
 * LEVEL 7 KERN_DEBUG (Lowest priority)
 */

#define ECHUB_INFO
#ifndef ECHUB_INFO
#define echub_debug(fmt, args...)do {} while (0)
#define echub_info(fmt, args...) do {} while (0)
#define echub_warn(fmt, args...) do {} while (0)
#define echub_err(fmt, args...)  do {} while (0)
#else
#define echub_debug(fmt, args...)do { printk(KERN_DEBUG   "[hw_echub]" fmt, ## args); } while (0)
#define echub_info(fmt, args...) do { printk(KERN_INFO    "[hw_echub]" fmt, ## args); } while (0)
#define echub_warn(fmt, args...) do { printk(KERN_WARNING "[hw_echub]" fmt, ## args); } while (0)
#define echub_err(fmt, args...)  do { printk(KERN_ERR     "[hw_echub]" fmt, ## args); } while (0)
#endif


#endif
