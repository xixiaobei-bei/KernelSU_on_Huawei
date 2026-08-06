/*
 * zrhung_config.h
 *
 * This file is header file for zero hung config
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ZRHUNG_CONFIG_H_
#define __ZRHUNG_CONFIG_H_

#include <linux/fs.h>

int hcfgk_set_cfg(struct file *file, const void *arg);
int hcfgk_ioctl_get_cfg(struct file *file, void *arg);
int hcfgk_set_cfg_flag(struct file *file, const  void *arg);
int hcfgk_get_cfg_flag(struct file *file, void *arg);
int hcfgk_set_feature(struct file *file, const void *arg);

#endif /* __ZRHUNG_CONFIG_H_ */
