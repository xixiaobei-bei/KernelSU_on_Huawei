/*
 * Linux kernel modules for VI5300 FlightSense TOF sensor
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include<linux/string.h>
#include <linux/kernel.h>
#include <linux/firmware.h>

#include "vi5300_firmware.h"

uint8_t Firmware[FIRMWARE_NUM];

uint32_t LoadFirmware(VI5300_DEV dev)
{
	const struct firmware *vi5300_firmware = NULL;
	const char *fw_name = "VI5300_Firmware.bin";
	uint32_t data_size;
	int err;

	err = request_firmware(&vi5300_firmware, fw_name, dev->dev);
	if (err || !vi5300_firmware)
	{
		vi5300_errmsg("Firmware request failed!\n");
		return err;
	} else {
		vi5300_infomsg("Firmware request succeeded!\n");
	}
	data_size = (uint32_t)vi5300_firmware->size;
	vi5300_dbgmsg("vi5300_firmware size:%d\n", data_size);

	if (memcpy_s(Firmware, FIRMWARE_NUM, vi5300_firmware->data, vi5300_firmware->size) != EOK)
		vi5300_errmsg("memcpy_s failed");
	release_firmware(vi5300_firmware);
	return data_size;
}
