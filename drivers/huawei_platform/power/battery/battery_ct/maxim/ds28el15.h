/*
 * ds28el15.h
 *
 * ds28el15 driver head file for battery checker
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

#ifndef _DS28EL15_H_
#define _DS28EL15_H_

#include <linux/random.h>
#include <linux/wakelock.h>

#include "maxim_onewire.h"
#include "../batt_aut_checker.h"

struct ds28el15_des {
	struct platform_device *pdev;
	struct maxim_ow_ic_des ic_des;
	struct attribute_group *attr_group;
	const struct attribute_group **attr_groups;
	struct wake_lock write_lock;
};

/* MAC types */
#define DS28EL15_CT_MAC_PAGE0                       MAC_RESOURCE_TPYE0
#define DS28EL15_CT_MAC_PAGE1                       MAC_RESOURCE_TPYE1
#define DS28EL15_CT_MAC_SIZE                        128

/* IC memory protection mode */
#define DS28EL15_INFO_BLOCK_NUM                     4
#define DS28EL15_READ_PROTECTION                    0x80
#define DS28EL15_WRT_PROT                           0x40
#define DS28EL15_EPROM_EMULATION_MODE               0x20
#define DS28EL15_AUT_PROT                           0x10
#define DS28EL15_PROT_MASK                          0xf0
#define DS28EL15_EMPTY_PROT                         0
#define DS28EL15_AUT_WRT_PROT                       (DS28EL15_WRT_PROT | \
						     DS28EL15_AUT_PROT)

/* ds28el15 return value */
#define DS28EL15_SUCCESS                            0
#define DS28EL15_FAIL                               1

/* set sram for get mac retry */
#define SET_SRAM_RETRY                              4
#define GET_USER_MEMORY_RETRY                       8
#define GET_PERSONALITY_RETRY                       8
#define GET_ROM_ID_RETRY                            8
#define GET_BLOCK_STATUS_RETRY                      8
#define SET_BLOCK_STATUS_RETRY                      8
#define GET_MAC_RETRY                               8

#define CURRENT_DS28EL15_TASK                       0
#define BYTES2BITS(x)                               ((x) << 3)

#define SN_LENGTH_BITS                              108
#define PRINTABLE_SN_SIZE                           17
#define SN_CHAR_PRINT_SIZE                          11
#define SN_HEX_PRINT_SIZE                           5

#define IS_ODD(a)                                   ((a) & 0x1)
#define IS_EVEN(a)                                  (!IS_ODD(a))
#define NOT_MUTI8(a)                                ((a) & 0x7)
#define IS_MUTI8(a)                                 (!NOT_MUTI8(a))

/* onewire communication error process */
#define DS28EL15_COMMUNICATION_INFO(x, y)                        \
do {                                                             \
	if (x)                                                   \
		hwlog_info(y" failed(%x) in %s\n", x, __func__); \
} while (0)

enum {
	SET_SRAM_INDEX = 0,
	GET_USER_MEMORY_INDEX,
	GET_BLOCK_STATUS_INDEX,
	SET_BLOCK_STATUS_INDEX,
	GET_PERSONALITY_INDEX,
	GET_ROM_ID_INDEX,
	GET_MAC_INDEX,
	__MAX_COM_ERR_NUM,
};

struct battery_constraint {
	unsigned char *id_mask;
	unsigned char *id_example;
	unsigned char *id_chk;
};

#endif /* _DS28EL15_H_ */
