/*
 * omnivision TCM touchscreen driver
 *
 * Copyright (C) 2017-2018 omnivision Incorporated. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS," AND omnivision
 * EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE,
 * AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS.
 * IN NO EVENT SHALL omnivision BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, PUNITIVE, OR CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED
 * AND BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF omnivision WAS ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF COMPETENT JURISDICTION DOES
 * NOT PERMIT THE DISCLAIMER OF DIRECT DAMAGES OR ANY OTHER DAMAGES, omnivision'
 * TOTAL CUMULATIVE LIABILITY TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S.
 * DOLLARS.
 */

#include <linux/gpio.h>
#include <linux/crc32.h>
#include <linux/firmware.h>
#include "omnivision_tcm_core.h"


#define ENABLE_SYS_ZEROFLASH true

#define FW_IMAGE_NAME "ts/hdl_firmware"

#define BOOT_CONFIG_ID "BOOT_CONFIG"

#define F35_APP_CODE_ID "F35_APP_CODE"

#define ROMBOOT_APP_CODE_ID "ROMBOOT_APP_CODE"

#define RESERVED_BYTES 14

#define APP_CONFIG_ID "APP_CONFIG"

#define DISP_CONFIG_ID "DISPLAY"

#define OPEN_SHORT_ID "OPENSHORT"

#define SYSFS_DIR_NAME "zeroflash"

#define IMAGE_FILE_MAGIC_VALUE 0x4818472b

#define FLASH_AREA_MAGIC_VALUE 0x7c05e516

#define PDT_START_ADDR 0x00e9

#define PDT_END_ADDR 0x00ee

#define UBL_FN_NUMBER 0x35

#define F35_CTRL3_OFFSET 18

#define F35_CTRL7_OFFSET 22

#define F35_WRITE_FW_TO_PMEM_COMMAND 4

#define TP_RESET_TO_HDL_DELAY_MS 11

#define DOWNLOAD_RETRY_COUNT 10

enum f35_error_code {
	SUCCESS = 0,
	UNKNOWN_FLASH_PRESENT,
	MAGIC_NUMBER_NOT_PRESENT,
	INVALID_BLOCK_NUMBER,
	BLOCK_NOT_ERASED,
	NO_FLASH_PRESENT,
	CHECKSUM_FAILURE,
	WRITE_FAILURE,
	INVALID_COMMAND,
	IN_DEBUG_MODE,
	INVALID_HEADER,
	REQUESTING_FIRMWARE,
	INVALID_CONFIGURATION,
	DISABLE_BLOCK_PROTECT_FAILURE,
};

enum config_download {
	HDL_INVALID = 0,
	HDL_TOUCH_CONFIG,
	HDL_DISPLAY_CONFIG,
	HDL_OPEN_SHORT_CONFIG,
};

struct area_descriptor {
	unsigned char magic_value[4];
	unsigned char id_string[16];
	unsigned char flags[4];
	unsigned char flash_addr_words[4];
	unsigned char length[4];
	unsigned char checksum[4];
};

struct block_data {
	const unsigned char *data;
	unsigned int size;
	unsigned int flash_addr;
};

struct image_info {
	unsigned int packrat_number;
	struct block_data boot_config;
	struct block_data app_firmware;
	struct block_data app_config;
	struct block_data disp_config;
	struct block_data open_short_config;
};

struct image_header {
	unsigned char magic_value[4];
	unsigned char num_of_areas[4];
};

struct rmi_f35_query {
	unsigned char version : 4;
	unsigned char has_debug_mode : 1;
	unsigned char has_data5 : 1;
	unsigned char has_query1 : 1;
	unsigned char has_query2 : 1;
	unsigned char chunk_size;
	unsigned char has_ctrl7 : 1;
	unsigned char has_host_download : 1;
	unsigned char has_spi_master : 1;
	unsigned char advanced_recovery_mode : 1;
	unsigned char reserved : 4;
} __packed;

struct rmi_f35_data {
	unsigned char error_code : 5;
	unsigned char recovery_mode_forced : 1;
	unsigned char nvm_programmed : 1;
	unsigned char in_recovery : 1;
} __packed;

struct rmi_pdt_entry {
	unsigned char query_base_addr;
	unsigned char command_base_addr;
	unsigned char control_base_addr;
	unsigned char data_base_addr;
	unsigned char intr_src_count : 3;
	unsigned char reserved_1 : 2;
	unsigned char fn_version : 2;
	unsigned char reserved_2 : 1;
	unsigned char fn_number;
} __packed;

struct rmi_addr {
	unsigned short query_base;
	unsigned short command_base;
	unsigned short control_base;
	unsigned short data_base;
};

struct firmware_status {
	unsigned short invalid_static_config : 1;
	unsigned short need_disp_config : 1;
	unsigned short need_app_config : 1;
	unsigned short hdl_version : 4;
	unsigned short need_open_short_config : 1;
	unsigned short reserved : 8;
} __packed;

struct zeroflash_hcd {
	bool has_hdl;
	bool f35_ready;
	bool has_open_short_config;
	const unsigned char *image;
	unsigned char *buf;
	const struct firmware *fw_entry;
	struct work_struct config_work;
	struct workqueue_struct *workqueue;
	struct kobject *sysfs_dir;
	struct rmi_addr f35_addr;
	struct image_info image_info;
	struct firmware_status fw_status;
	struct ovt_tcm_buffer out;
	struct ovt_tcm_buffer resp;
	struct ovt_tcm_hcd *tcm_hcd;
};

static struct zeroflash_hcd *zeroflash_hcd;

static int zeroflash_check_uboot(void)
{
	int retval;
	unsigned char fn_number;
	unsigned int retry = 3;
	struct rmi_f35_data data;
	struct rmi_f35_query query;
	struct rmi_pdt_entry p_entry;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;

re_check:
	retval = ovt_tcm_rmi_read(tcm_hcd,
			PDT_END_ADDR,
			&fn_number,
			sizeof(fn_number));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to read RMI function number");
		return retval;
	}

	LOGD(tcm_hcd->pdev->dev.parent,
			"Found F$%02x\n",
			fn_number);

	if (fn_number != UBL_FN_NUMBER) {
		if (retry--) {
			ovt_tcm_simple_hw_reset(tcm_hcd);
			goto re_check;
		}
		OVT_LOG_ERR(
				"Failed to find F$35");
		return -ENODEV;
	}

/* 	if (zeroflash_hcd->f35_ready)
		return 0; */

	retval = ovt_tcm_rmi_read(tcm_hcd,
			PDT_START_ADDR,
			(unsigned char *)&p_entry,
			sizeof(p_entry));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to read PDT entry");
		return retval;
	}

	zeroflash_hcd->f35_addr.query_base = p_entry.query_base_addr;
	zeroflash_hcd->f35_addr.command_base = p_entry.command_base_addr;
	zeroflash_hcd->f35_addr.control_base = p_entry.control_base_addr;
	zeroflash_hcd->f35_addr.data_base = p_entry.data_base_addr;

	retval = ovt_tcm_rmi_read(tcm_hcd,
			zeroflash_hcd->f35_addr.query_base,
			(unsigned char *)&query,
			sizeof(query));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to read F$35 query");
		return retval;
	}

	zeroflash_hcd->f35_ready = true;

	if (query.has_query2 && query.has_ctrl7 && query.has_host_download) {
		zeroflash_hcd->has_hdl = true;
	} else {
		OVT_LOG_ERR(
				"Host download not supported");
		zeroflash_hcd->has_hdl = false;
		return -ENODEV;
	}

	retval = ovt_tcm_rmi_read(tcm_hcd,
			zeroflash_hcd->f35_addr.data_base,
			(unsigned char *)&data,
			sizeof(data));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to read F$35 data");
		goto exit;
	}

	if (data.error_code != REQUESTING_FIRMWARE) {
		OVT_LOG_ERR(
				"Microbootloader error code = 0x%02x\n",
				data.error_code);
		ovt_tcm_simple_hw_reset(tcm_hcd);
		if (retry--) {
			goto re_check;
		}
		retval = -EINVAL;
		goto exit;
	}
exit:
	return retval;
}

static int zeroflash_parse_fw_image(void)
{
	unsigned int idx;
	unsigned int addr;
	unsigned int offset;
	unsigned int length;
	unsigned int checksum;
	unsigned int flash_addr;
	unsigned int magic_value;
	unsigned int num_of_areas;
	struct image_header *header;
	struct image_info *image_info;
	struct area_descriptor *descriptor = NULL;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
	const unsigned char *image;
	const unsigned char *content = NULL;

	image = zeroflash_hcd->image;
	image_info = &zeroflash_hcd->image_info;
	header = (struct image_header *)image;

	magic_value = le4_to_uint(header->magic_value);
	if (magic_value != IMAGE_FILE_MAGIC_VALUE) {
		OVT_LOG_ERR(
				"Invalid image file magic value");
		return -EINVAL;
	}

	memset(image_info, 0x00, sizeof(*image_info));

	offset = sizeof(*header);
	num_of_areas = le4_to_uint(header->num_of_areas);

	for (idx = 0; idx < num_of_areas; idx++) {
		addr = le4_to_uint(image + offset);
		descriptor = (struct area_descriptor *)(image + addr);
		offset += 4;

		magic_value = le4_to_uint(descriptor->magic_value);
		if (magic_value != FLASH_AREA_MAGIC_VALUE)
			continue;

		length = le4_to_uint(descriptor->length);
		content = (unsigned char *)descriptor + sizeof(*descriptor);
		flash_addr = le4_to_uint(descriptor->flash_addr_words) * 2;
		checksum = le4_to_uint(descriptor->checksum);

		if (0 == strncmp((char *)descriptor->id_string,
				BOOT_CONFIG_ID, strlen(BOOT_CONFIG_ID))) {
			if (checksum != (crc32(~0, content, length) ^ ~0)) {
				OVT_LOG_ERR(
						"Boot config checksum error");
				return -EINVAL;
			}
			image_info->boot_config.size = length;
			image_info->boot_config.data = content;
			image_info->boot_config.flash_addr = flash_addr;
			LOGD(tcm_hcd->pdev->dev.parent,
					"Boot config size = %d\n",
					length);
			LOGD(tcm_hcd->pdev->dev.parent,
					"Boot config flash address = 0x%08x\n",
					flash_addr);
		} else if ((0 == strncmp((char *)descriptor->id_string,
				F35_APP_CODE_ID, strlen(F35_APP_CODE_ID)))) {
			if (tcm_hcd->sensor_type != TYPE_F35) {
				OVT_LOG_ERR(
						"Improper descriptor, F35_APP_CODE_ID");
				return -EINVAL;
			}

			if (checksum != (crc32(~0, content, length) ^ ~0)) {
				OVT_LOG_ERR(
						"HDL_F35 firmware checksum error");
				return -EINVAL;
			}
			image_info->app_firmware.size = length;
			image_info->app_firmware.data = content;
			image_info->app_firmware.flash_addr = flash_addr;
			LOGD(tcm_hcd->pdev->dev.parent,
					"HDL_F35 firmware size = %d\n",
					length);
			LOGD(tcm_hcd->pdev->dev.parent,
					"HDL_F35 firmware flash address = 0x%08x\n",
					flash_addr);
		} else if ((0 == strncmp((char *)descriptor->id_string,
				ROMBOOT_APP_CODE_ID,
				strlen(ROMBOOT_APP_CODE_ID)))) {
			if (tcm_hcd->sensor_type != TYPE_ROMBOOT) {
				OVT_LOG_ERR(
						"Improper descriptor, ROMBOOT_APP_CODE_ID");
				return -EINVAL;
			}

			if (checksum != (crc32(~0, content, length) ^ ~0)) {
				OVT_LOG_ERR(
						"HDL_ROMBoot firmware checksum error");
				return -EINVAL;
			}
			image_info->app_firmware.size = length;
			image_info->app_firmware.data = content;
			image_info->app_firmware.flash_addr = flash_addr;
			LOGD(tcm_hcd->pdev->dev.parent,
					"HDL_ROMBoot firmware size = %d\n",
					length);
			LOGD(tcm_hcd->pdev->dev.parent,
					"HDL_ROMBoot firmware flash address = 0x%08x\n",
					flash_addr);
		} else if (0 == strncmp((char *)descriptor->id_string,
				APP_CONFIG_ID, strlen(APP_CONFIG_ID))) {
			if (checksum != (crc32(~0, content, length) ^ ~0)) {
				OVT_LOG_ERR(
						"Application config checksum error");
				return -EINVAL;
			}
			image_info->app_config.size = length;
			image_info->app_config.data = content;
			image_info->app_config.flash_addr = flash_addr;
			image_info->packrat_number = le4_to_uint(&content[14]);
			LOGD(tcm_hcd->pdev->dev.parent,
					"Application config size = %d\n",
					length);
			LOGD(tcm_hcd->pdev->dev.parent,
					"Application config flash address = 0x%08x\n",
					flash_addr);
		} else if (0 == strncmp((char *)descriptor->id_string,
				DISP_CONFIG_ID, strlen(DISP_CONFIG_ID))) {
			if (checksum != (crc32(~0, content, length) ^ ~0)) {
				OVT_LOG_ERR(
						"Display config checksum error");
				return -EINVAL;
			}
			image_info->disp_config.size = length;
			image_info->disp_config.data = content;
			image_info->disp_config.flash_addr = flash_addr;
			LOGD(tcm_hcd->pdev->dev.parent,
					"Display config size = %d\n",
					length);
			LOGD(tcm_hcd->pdev->dev.parent,
					"Display config flash address = 0x%08x\n",
					flash_addr);
		} else if (0 == strncmp((char *)descriptor->id_string,
				OPEN_SHORT_ID, strlen(OPEN_SHORT_ID))) {
			if (checksum != (crc32(~0, content, length) ^ ~0)) {
				OVT_LOG_ERR(
						"open_short config checksum error");
				return -EINVAL;
			}
			zeroflash_hcd->has_open_short_config = true;
			image_info->open_short_config.size = length;
			image_info->open_short_config.data = content;
			image_info->open_short_config.flash_addr = flash_addr;
			OVT_LOG_INFO(
					"open_short config size = %d\n",
					length);
		}
	}

	return 0;
}

static int zeroflash_get_fw_image(void)
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
	char fw_name[MAX_STR_LEN * 4] = {0};

	if (zeroflash_hcd->fw_entry != NULL) {
		release_firmware(zeroflash_hcd->fw_entry);
		zeroflash_hcd->fw_entry = NULL;
		zeroflash_hcd->image = NULL;
	} else {
		msleep(10000); // power on case, sleep 10s to get the file system img
	}
	OVT_LOG_INFO("zeroflash_get_fw_image");

	if (zeroflash_hcd->image == NULL) {
		if (tcm_hcd->hw_if->bdata->project_id) {
			snprintf(fw_name, (MAX_STR_LEN * 4), "%s_%s.img", FW_IMAGE_NAME,
			tcm_hcd->hw_if->bdata->project_id);
		} else {
			snprintf(fw_name, (MAX_STR_LEN * 4), "%s.img", FW_IMAGE_NAME);
		}
		OVT_LOG_INFO("fw_name is %s", fw_name);
		retval = request_firmware(&zeroflash_hcd->fw_entry,
				fw_name,
				tcm_hcd->pdev->dev.parent);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to request %s\n",
					fw_name);
			return retval;
		}
	}

	OVT_LOG_INFO(
			"Firmware image size = %d\n",
			(unsigned int)zeroflash_hcd->fw_entry->size);

	zeroflash_hcd->image = zeroflash_hcd->fw_entry->data;

	retval = zeroflash_parse_fw_image();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to parse firmware image");
		release_firmware(zeroflash_hcd->fw_entry);
		zeroflash_hcd->fw_entry = NULL;
		zeroflash_hcd->image = NULL;
		return retval;
	}

	return 0;
}

static int zeroflash_download_open_short_config(void)
{
	int retval;
	unsigned char response_code;
	struct image_info *image_info = NULL;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
	static unsigned int retry_count;

	OVT_LOG_INFO(
			"Downloading open_short config");

	image_info = &zeroflash_hcd->image_info;

	if (image_info->open_short_config.size == 0) {
		OVT_LOG_ERR(
				"No open_short config in image file");
		return -EINVAL;
	}

	LOCK_BUFFER(zeroflash_hcd->out);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&zeroflash_hcd->out,
			image_info->open_short_config.size + 2);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory for open_short config");
		goto unlock_out;
	}

	switch (zeroflash_hcd->fw_status.hdl_version) {
	case 0:
	case 1:
		zeroflash_hcd->out.buf[0] = 1;
		break;
	case 2:
		zeroflash_hcd->out.buf[0] = 2;
		break;
	default:
		retval = -EINVAL;
		OVT_LOG_ERR(
				"Invalid HDL version %d\n",
				zeroflash_hcd->fw_status.hdl_version);
		goto unlock_out;
	}

	zeroflash_hcd->out.buf[1] = HDL_OPEN_SHORT_CONFIG;

	retval = secure_cpydata(&zeroflash_hcd->out.buf[2],
			zeroflash_hcd->out.buf_size - 2,
			image_info->open_short_config.data,
			image_info->open_short_config.size,
			image_info->open_short_config.size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy open_short config data");
		goto unlock_out;
	}

	zeroflash_hcd->out.data_length = image_info->open_short_config.size + 2;

	LOCK_BUFFER(zeroflash_hcd->resp);

	retval = tcm_hcd->write_message(tcm_hcd,
			CMD_DOWNLOAD_CONFIG,
			zeroflash_hcd->out.buf,
			zeroflash_hcd->out.data_length,
			&zeroflash_hcd->resp.buf,
			&zeroflash_hcd->resp.buf_size,
			&zeroflash_hcd->resp.data_length,
			&response_code,
			20);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_DOWNLOAD_CONFIG));
		if (response_code != STATUS_ERROR)
			goto unlock_resp;
		retry_count++;
		if (DOWNLOAD_RETRY_COUNT && retry_count > DOWNLOAD_RETRY_COUNT)
			goto unlock_resp;
	} else {
		retry_count = 0;
	}

	retval = secure_cpydata((unsigned char *)&zeroflash_hcd->fw_status,
			sizeof(zeroflash_hcd->fw_status),
			zeroflash_hcd->resp.buf,
			zeroflash_hcd->resp.buf_size,
			sizeof(zeroflash_hcd->fw_status));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy firmware status");
		goto unlock_resp;
	}

	OVT_LOG_INFO(
			"open_short config downloaded");

	retval = 0;

unlock_resp:
	UNLOCK_BUFFER(zeroflash_hcd->resp);

unlock_out:
	UNLOCK_BUFFER(zeroflash_hcd->out);

	return retval;
}

static int zeroflash_download_disp_config(void)
{
	int retval;
	unsigned char response_code;
	struct image_info *image_info = NULL;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
	static unsigned int retry_count;

	OVT_LOG_INFO(
			"Downloading display config");

	image_info = &zeroflash_hcd->image_info;

	if (image_info->disp_config.size == 0) {
		OVT_LOG_ERR(
				"No display config in image file");
		return -EINVAL;
	}

	LOCK_BUFFER(zeroflash_hcd->out);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&zeroflash_hcd->out,
			image_info->disp_config.size + 2);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory for display config");
		goto unlock_out;
	}

	switch (zeroflash_hcd->fw_status.hdl_version) {
	case 0:
	case 1:
		zeroflash_hcd->out.buf[0] = 1;
		break;
	case 2:
		zeroflash_hcd->out.buf[0] = 2;
		break;
	default:
		retval = -EINVAL;
		OVT_LOG_ERR(
				"Invalid HDL version %d\n",
				zeroflash_hcd->fw_status.hdl_version);
		goto unlock_out;
	}

	zeroflash_hcd->out.buf[1] = HDL_DISPLAY_CONFIG;

	retval = secure_cpydata(&zeroflash_hcd->out.buf[2],
			zeroflash_hcd->out.buf_size - 2,
			image_info->disp_config.data,
			image_info->disp_config.size,
			image_info->disp_config.size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy display config data");
		goto unlock_out;
	}

	zeroflash_hcd->out.data_length = image_info->disp_config.size + 2;

	LOCK_BUFFER(zeroflash_hcd->resp);

	retval = tcm_hcd->write_message(tcm_hcd,
			CMD_DOWNLOAD_CONFIG,
			zeroflash_hcd->out.buf,
			zeroflash_hcd->out.data_length,
			&zeroflash_hcd->resp.buf,
			&zeroflash_hcd->resp.buf_size,
			&zeroflash_hcd->resp.data_length,
			&response_code,
			20);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_DOWNLOAD_CONFIG));
		if (response_code != STATUS_ERROR)
			goto unlock_resp;
		retry_count++;
		if (DOWNLOAD_RETRY_COUNT && retry_count > DOWNLOAD_RETRY_COUNT)
			goto unlock_resp;
	} else {
		retry_count = 0;
	}

	retval = secure_cpydata((unsigned char *)&zeroflash_hcd->fw_status,
			sizeof(zeroflash_hcd->fw_status),
			zeroflash_hcd->resp.buf,
			zeroflash_hcd->resp.buf_size,
			sizeof(zeroflash_hcd->fw_status));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy firmware status");
		goto unlock_resp;
	}

	OVT_LOG_INFO(
			"Display config downloaded");

	retval = 0;

unlock_resp:
	UNLOCK_BUFFER(zeroflash_hcd->resp);

unlock_out:
	UNLOCK_BUFFER(zeroflash_hcd->out);

	return retval;
}

static int zeroflash_download_app_config(void)
{
	int retval;
	unsigned char padding;
	unsigned char response_code;
	struct image_info *image_info = NULL;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
	static unsigned int retry_count;

	OVT_LOG_INFO(
			"Downloading application config");

	image_info = &zeroflash_hcd->image_info;

	if (image_info->app_config.size == 0) {
		OVT_LOG_ERR(
				"No application config in image file");
		return -EINVAL;
	}

	padding = image_info->app_config.size % 8;
	if (padding)
		padding = 8 - padding;

	LOCK_BUFFER(zeroflash_hcd->out);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&zeroflash_hcd->out,
			image_info->app_config.size + 2 + padding);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory for application config");
		goto unlock_out;
	}

	switch (zeroflash_hcd->fw_status.hdl_version) {
	case 0:
	case 1:
		zeroflash_hcd->out.buf[0] = 1;
		break;
	case 2:
		zeroflash_hcd->out.buf[0] = 2;
		break;
	default:
		retval = -EINVAL;
		OVT_LOG_ERR(
				"Invalid HDL version %d\n",
				zeroflash_hcd->fw_status.hdl_version);
		goto unlock_out;
	}

	zeroflash_hcd->out.buf[1] = HDL_TOUCH_CONFIG;

	retval = secure_cpydata(&zeroflash_hcd->out.buf[2],
			zeroflash_hcd->out.buf_size - 2,
			image_info->app_config.data,
			image_info->app_config.size,
			image_info->app_config.size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy application config data");
		goto unlock_out;
	}

	zeroflash_hcd->out.data_length = image_info->app_config.size + 2;
	zeroflash_hcd->out.data_length += padding;

	LOCK_BUFFER(zeroflash_hcd->resp);

	retval = tcm_hcd->write_message(tcm_hcd,
			CMD_DOWNLOAD_CONFIG,
			zeroflash_hcd->out.buf,
			zeroflash_hcd->out.data_length,
			&zeroflash_hcd->resp.buf,
			&zeroflash_hcd->resp.buf_size,
			&zeroflash_hcd->resp.data_length,
			&response_code,
			20);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write command %s\n",
				STR(CMD_DOWNLOAD_CONFIG));
		if (response_code != STATUS_ERROR)
			goto unlock_resp;
		retry_count++;
		if (DOWNLOAD_RETRY_COUNT && retry_count > DOWNLOAD_RETRY_COUNT)
			goto unlock_resp;
	} else {
		retry_count = 0;
	}

	retval = secure_cpydata((unsigned char *)&zeroflash_hcd->fw_status,
			sizeof(zeroflash_hcd->fw_status),
			zeroflash_hcd->resp.buf,
			zeroflash_hcd->resp.buf_size,
			sizeof(zeroflash_hcd->fw_status));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy firmware status");
		goto unlock_resp;
	}

	OVT_LOG_INFO(
			"Application config downloaded");

	retval = 0;

unlock_resp:
	UNLOCK_BUFFER(zeroflash_hcd->resp);

unlock_out:
	UNLOCK_BUFFER(zeroflash_hcd->out);

	return retval;
}

int zeroflash_download_config_directly(void)
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;

	retval = zeroflash_get_fw_image();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to get firmware image");
		return retval;
	}

	OVT_LOG_INFO(
			"Start of config download");

	if (zeroflash_hcd->fw_status.need_app_config) {
		retval = zeroflash_download_app_config();
		if (retval < 0) {
			atomic_set(&tcm_hcd->host_downloading, 0);
			OVT_LOG_ERR(
					"Failed to download application config, abort");
			return retval;
		}
		goto exit;
	}

	if (zeroflash_hcd->fw_status.need_disp_config) {
		retval = zeroflash_download_disp_config();
		if (retval < 0) {
			atomic_set(&tcm_hcd->host_downloading, 0);
			OVT_LOG_ERR(
					"Failed to download display config, abort");
			return retval;
		}
		goto exit;
	}

	if (zeroflash_hcd->fw_status.need_open_short_config &&
			zeroflash_hcd->has_open_short_config) {
		retval = zeroflash_download_open_short_config();
		if (retval < 0) {
			atomic_set(&tcm_hcd->host_downloading, 0);
			OVT_LOG_ERR(
					"Failed to download open_short config, abort");
			return retval;
		}
		goto exit;
	}

exit:
	OVT_LOG_INFO(
			"End of config download");

	if (tcm_hcd->ovt_tcm_driver_removing == 1)
		return retval;

	return retval;
}

static int zeroflash_download_app_fw(void)
{
	int retval;
	unsigned char command;
	struct image_info *image_info = NULL;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
#if TP_RESET_TO_HDL_DELAY_MS
	const struct ovt_tcm_board_data *bdata = tcm_hcd->hw_if->bdata;
#endif

	OVT_LOG_INFO(
			"Downloading application firmware");

	image_info = &zeroflash_hcd->image_info;

	if (image_info->app_firmware.size == 0) {
		OVT_LOG_ERR(
				"No application firmware in image file");
		return -EINVAL;
	}

	LOCK_BUFFER(zeroflash_hcd->out);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&zeroflash_hcd->out,
			image_info->app_firmware.size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to allocate memory for application firmware");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		return retval;
	}

	retval = secure_cpydata(zeroflash_hcd->out.buf,
			zeroflash_hcd->out.buf_size,
			image_info->app_firmware.data,
			image_info->app_firmware.size,
			image_info->app_firmware.size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy application firmware data");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		return retval;
	}

	zeroflash_hcd->out.data_length = image_info->app_firmware.size;

	command = F35_WRITE_FW_TO_PMEM_COMMAND;

#if TP_RESET_TO_HDL_DELAY_MS
	if (bdata->tpio_reset_gpio >= 0) {
		gpio_set_value(bdata->tpio_reset_gpio, bdata->reset_on_state);
		msleep(bdata->reset_active_ms);
		gpio_set_value(bdata->tpio_reset_gpio, !bdata->reset_on_state);
		mdelay(TP_RESET_TO_HDL_DELAY_MS);
	}
#endif

	retval = ovt_tcm_rmi_write(tcm_hcd,
			zeroflash_hcd->f35_addr.control_base + F35_CTRL3_OFFSET,
			&command,
			sizeof(command));
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write F$35 command");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		return retval;
	}

	retval = ovt_tcm_rmi_write(tcm_hcd,
			zeroflash_hcd->f35_addr.control_base + F35_CTRL7_OFFSET,
			zeroflash_hcd->out.buf,
			zeroflash_hcd->out.data_length);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write application firmware data");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		return retval;
	}

	UNLOCK_BUFFER(zeroflash_hcd->out);

	OVT_LOG_INFO(
			"Application firmware downloaded and fw should return identify report next");

	return 0;
}


int zeroflash_do_f35_firmware_download(void)
{
	int retval;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;
	unsigned int retry_count = 3;

retry_download:
	OVT_LOG_INFO(
			"Prepare F35 firmware download");

	retval = zeroflash_check_uboot();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to find valid uboot");
		goto exit;
	}

	atomic_set(&tcm_hcd->host_downloading, 1);

	retval = zeroflash_get_fw_image();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to get firmware image");
		goto exit;
	}

	OVT_LOG_INFO(
			"Start of firmware download");

	/* perform firmware downloading */
	retval = zeroflash_download_app_fw();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to download application firmware, so reset tp ");
		goto exit;
	}
	msleep(20);
	{
		retval = tcm_hcd->read_message(tcm_hcd, NULL, 0);
		if (retval < 0) {
			OVT_LOG_INFO(
				"fail of firmware download");
			if (retry_count--) {
				ovt_tcm_simple_hw_reset(tcm_hcd);
				goto retry_download;
			}
			goto exit;
		} else {
			if (tcm_hcd->in.buf[1] == REPORT_IDENTIFY) {
				OVT_LOG_INFO(
					"success of firmware download");
			}
			retval = 0;
		}
	}
exit:
	return retval;
}

int zeroflash_do_romboot_firmware_download(void)
{
	int retval = 0;
	unsigned char *resp_buf = NULL;
	unsigned int resp_buf_size;
	unsigned int resp_length;
	unsigned int data_size_blocks;
	unsigned int image_size;
	struct ovt_tcm_hcd *tcm_hcd = zeroflash_hcd->tcm_hcd;

	OVT_LOG_INFO(
			"Prepare ROMBOOT firmware download");

	atomic_set(&tcm_hcd->host_downloading, 1);
	resp_buf = NULL;
	resp_buf_size = 0;

	if (!tcm_hcd->irq_enabled) {
		retval = tcm_hcd->enable_irq(tcm_hcd, true, NULL);
		if (retval < 0) {
			OVT_LOG_ERR(
					"Failed to enable interrupt");
		}
	}
	pm_stay_awake(&tcm_hcd->pdev->dev);
	OVT_LOG_INFO("tcm mode = %d\n", tcm_hcd->id_info.mode);
	if (tcm_hcd->id_info.mode != MODE_ROMBOOTLOADER) {
		OVT_LOG_ERR(
				"Not in romboot mode");
		atomic_set(&tcm_hcd->host_downloading, 0);
		goto exit;
	}
	OVT_LOG_INFO("about to request firmware");
	retval = zeroflash_get_fw_image();
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to request romboot.img");
		goto exit;
	}

	image_size = (unsigned int)zeroflash_hcd->image_info.app_firmware.size;

	OVT_LOG_INFO(
			"image_size = %d\n",
			image_size);

	data_size_blocks = image_size / 16;

	LOCK_BUFFER(zeroflash_hcd->out);

	retval = ovt_tcm_alloc_mem(tcm_hcd,
			&zeroflash_hcd->out,
			image_size + RESERVED_BYTES);
	if (retval < 0) {
		LOGE(tcm_hcd->pdev->dev.parent,
				"Failed to allocate memory for application firmware\n");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		return retval;
	}

	zeroflash_hcd->out.buf[0] = zeroflash_hcd->image_info.app_firmware.size >> 16;

	retval = secure_cpydata(&zeroflash_hcd->out.buf[RESERVED_BYTES],
			zeroflash_hcd->image_info.app_firmware.size,
			zeroflash_hcd->image_info.app_firmware.data,
			zeroflash_hcd->image_info.app_firmware.size,
			zeroflash_hcd->image_info.app_firmware.size);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to copy payload");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		goto exit;
	}

	LOGD(tcm_hcd->pdev->dev.parent,
			"data_size_blocks: %d\n",
			data_size_blocks);

	retval = tcm_hcd->write_message(tcm_hcd,
			CMD_ROMBOOT_DOWNLOAD,
			zeroflash_hcd->out.buf,
			image_size + RESERVED_BYTES,
			&resp_buf,
			&resp_buf_size,
			&resp_length,
			NULL,
			20);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to write command ROMBOOT DOWNLOAD");
		UNLOCK_BUFFER(zeroflash_hcd->out);
		goto exit;
	}
	UNLOCK_BUFFER(zeroflash_hcd->out);

	retval = tcm_hcd->switch_mode(tcm_hcd, FW_MODE_BOOTLOADER);
	if (retval < 0) {
		OVT_LOG_ERR(
				"Failed to switch to bootloader");
		goto exit;
	}

exit:

	pm_relax(&tcm_hcd->pdev->dev);

	kfree(resp_buf);

	return retval;
}

int zeroflash_do_config_download_entry(void)
{
	int retval = 0;
	struct ovt_tcm_hcd *tcm_hcd;
	tcm_hcd = g_tcm_hcd;
	retval = tcm_hcd->read_message(tcm_hcd, NULL, 0);
	if (retval >= 0) {
		if (tcm_hcd->in.buf[1] == REPORT_STATUS) {
			struct firmware_status *fw_status = NULL;

			OVT_LOG_INFO(
				"read the status report");
			fw_status = &(zeroflash_hcd->fw_status);

			retval = secure_cpydata((unsigned char *)fw_status,
					sizeof(zeroflash_hcd->fw_status),
					tcm_hcd->report.buffer.buf,
					tcm_hcd->report.buffer.buf_size,
					sizeof(zeroflash_hcd->fw_status));
			if (retval < 0) {
				OVT_LOG_ERR(
						"Failed to copy firmware status");
				return retval;
			}
down_load_config:
			if (!fw_status->need_app_config && !fw_status->need_disp_config) {
				return 0;
			} else {
				retval = zeroflash_download_config_directly();
				if (retval < 0) {
					return retval;
				}
				goto down_load_config;
			}
		} else {
			if (tcm_hcd->in.buf[1] == STATUS_IDLE) {
				return 0;
			} else {
				return -EINVAL;
			}
		}
	} else {
		return retval;
	}
}
int ovt_zeroflash_init(struct ovt_tcm_hcd *tcm_hcd)
{
	int retval = 0;
	int idx;

	zeroflash_hcd = NULL;

	zeroflash_hcd = kzalloc(sizeof(*zeroflash_hcd), GFP_KERNEL);
	if (!zeroflash_hcd) {
		OVT_LOG_ERR(
				"Failed to allocate memory for zeroflash_hcd");
		return -ENOMEM;
	}

	zeroflash_hcd->tcm_hcd = tcm_hcd;
	zeroflash_hcd->image = NULL;
	zeroflash_hcd->has_hdl = false;
	zeroflash_hcd->f35_ready = false;
	zeroflash_hcd->has_open_short_config = false;

	INIT_BUFFER(zeroflash_hcd->out, false);
	INIT_BUFFER(zeroflash_hcd->resp, false);

	return retval;
}

int zeroflash_do_hostdownload(struct ovt_tcm_hcd *tcm_hcd)
{
	// update the firmware here power on update firmware here
	// irq shutdown by ts kit
	int retval = 0;
	OVT_LOG_ERR("zeroflash_do_hostdownload");
	if (tcm_hcd->use_dma_download_firmware)
		tcm_hcd->ovt_tcm_platform_data->spidev0_chip_info.com_mode = DMA_MODE;
	// do hostdownload here and init
	switch (tcm_hcd->sensor_type) {
	case TYPE_F35:
		retval = zeroflash_do_f35_firmware_download();
		break;
	case TYPE_ROMBOOT:
		retval = zeroflash_do_romboot_firmware_download();
		break;
	case TYPE_FLASH:
		retval = 0;
		break;
	default:
		retval = -EINVAL;
		break;
	}
	if (retval < 0)
		goto exit;

	// download config
	retval = zeroflash_do_config_download_entry();
exit:
	if (tcm_hcd->use_dma_download_firmware)
		tcm_hcd->ovt_tcm_platform_data->spidev0_chip_info.com_mode = POLLING_MODE;
	return retval;
}

static int zeroflash_remove(struct ovt_tcm_hcd *tcm_hcd)
{
	int idx;

	if (!zeroflash_hcd)
		goto exit;

	if (zeroflash_hcd->fw_entry)
		release_firmware(zeroflash_hcd->fw_entry);

	RELEASE_BUFFER(zeroflash_hcd->resp);
	RELEASE_BUFFER(zeroflash_hcd->out);

	kfree(zeroflash_hcd);
	zeroflash_hcd = NULL;

exit:
	return 0;
}
