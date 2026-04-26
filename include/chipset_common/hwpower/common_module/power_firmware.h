/*
 * power_firmware.h
 *
 * firmware for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_FIRMWARE_H_
#define _POWER_FIRMWARE_H_

#define POWER_FW_ITEM_NAME_MAX    32
#define POWER_FW_DIR_PERMS        0555
#define POWER_FW_FILE_PERMS       0660
#define POWER_FW_MAX_BLOCKS       8

struct power_fw_hdr {
	unsigned long version_id;
	unsigned long crc_id;
	unsigned long patch_id;
	unsigned long config_id;
	unsigned long bin_size;
};

typedef ssize_t (*power_fw_read)(void *, char *, size_t);
typedef ssize_t (*power_fw_write)(void *, const char *, size_t);

struct power_fw_attr {
	char name[POWER_FW_ITEM_NAME_MAX];
	void *dev_data;
	power_fw_read read;
	power_fw_write write;
	struct list_head list;
};

#ifdef CONFIG_HUAWEI_POWER_FIRMWARE
void power_fw_ops_register(char *name, void *dev_data,
	power_fw_read read, power_fw_write write);
#else
static inline void power_fw_ops_register(char *name, void *dev_data,
	power_fw_read read, power_fw_write write)
{
}
#endif /* CONFIG_HUAWEI_POWER_FIRMWARE */

#endif /* _POWER_FIRMWARE_H_ */
