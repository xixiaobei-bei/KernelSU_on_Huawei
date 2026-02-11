/*
 * power_nv.c
 *
 * nv(non-volatile) interface for power module
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/device.h>
#include <linux/mtd/hisi_nve_interface.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG power_nv
HWLOG_REGIST();

int power_nv_write(uint32_t nv_number, const char *nv_name,
	const void *data, uint32_t data_len)
{
	struct hisi_nve_info_user nv_info;

	if (!nv_name || !data) {
		hwlog_err("nv_name or data is null\n");
		return -EINVAL;
	}

	if (data_len > NVE_NV_DATA_SIZE) {
		hwlog_err("nv write data length %u is invalid\n", data_len);
		return -EINVAL;
	}

	memset(&nv_info, 0, sizeof(nv_info));
	nv_info.nv_operation = NV_WRITE;
	nv_info.nv_number = nv_number;
	strlcpy(nv_info.nv_name, nv_name, NV_NAME_LENGTH);
	nv_info.valid_size = data_len;
	memcpy(&nv_info.nv_data, data, data_len);

	if (hisi_nve_direct_access(&nv_info)) {
		hwlog_err("nv %s write fail\n", nv_name);
		return -EINVAL;
	}

	hwlog_info("nv %s,%u write succ\n", nv_name, data_len);
	return 0;
}

int power_nv_read(uint32_t nv_number, const char *nv_name,
	void *data, uint32_t data_len)
{
	struct hisi_nve_info_user nv_info;

	if (!nv_name || !data) {
		hwlog_err("nv_name or data is null\n");
		return -EINVAL;
	}

	if (data_len > NVE_NV_DATA_SIZE) {
		hwlog_err("nv read data length %u is invalid\n", data_len);
		return -EINVAL;
	}

	memset(&nv_info, 0, sizeof(nv_info));
	nv_info.nv_operation = NV_READ;
	nv_info.nv_number = nv_number;
	strlcpy(nv_info.nv_name, nv_name, NV_NAME_LENGTH);
	nv_info.valid_size = data_len;

	if (hisi_nve_direct_access(&nv_info)) {
		hwlog_err("nv %s read fail\n", nv_name);
		return -EINVAL;
	}
	memcpy(data, &nv_info.nv_data, data_len);

	hwlog_info("nv %s,%u read succ\n", nv_name, data_len);
	return 0;
}
