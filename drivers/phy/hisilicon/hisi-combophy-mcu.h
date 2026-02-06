/*
 * hisi-combophy-mcu.h
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Create:2019-09-24
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __CHIP_COMBOPHY_MCU_H__
#define __CHIP_COMBOPHY_MCU_H__

extern int combophy_mcu_init(const void *firmware, size_t size);
extern void combophy_mcu_exit(void);
extern void combophy_mcu_register_debugfs(struct dentry *root);

#endif /* __CHIP_COMBOPHY_MCU_H__ */
