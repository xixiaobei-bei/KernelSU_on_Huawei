/*
 * power_nv.c
 *
 * nv(non-volatile) interface for power module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/common_module/power_nv.h>
#include <linux/device.h>
#include <linux/module.h>
#ifdef CONFIG_HISI_NVE
#include <linux/mtd/hisi_nve_interface.h>
#endif
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_nv
HWLOG_REGIST();

static struct power_nv_data_info g_power_nv_data[] = {
	{ POWER_NV_BLIMSW, 388, "BLIMSW" },
	{ POWER_NV_BBINFO, 389, "BBINFO" },
	{ POWER_NV_BATHEAT, 426, "BATHEAT" },
	{ POWER_NV_BATCAP, 418, "BATCAP" },
	{ POWER_NV_DEVCOLR, 330, "DEVCOLR" },
	{ POWER_NV_TERMVOL, 425, "TERM_VOL" },
	{ POWER_NV_CHGDIEID, 501, "CHGDIEID" },
	{ POWER_NV_SCCALCUR, 402, "SCCAL" },
};

static struct power_nv_data_info *power_nv_get_data(enum power_nv_type type)
{
	int i;
	struct power_nv_data_info *p_data = g_power_nv_data;
	int size = ARRAY_SIZE(g_power_nv_data);

	if ((type < POWER_NV_TYPE_BEGIN) || (type >= POWER_NV_TYPE_END)) {
		hwlog_err("nv_type %d check fail\n", type);
		return NULL;
	}

	for (i = 0; i < size; i++) {
		if (type == p_data[i].type)
			break;
	}

	if (i >= size) {
		hwlog_err("nv_type %d find fail\n", type);
		return NULL;
	}

	hwlog_info("nv [%d]=%d,%u,%s\n",
		i, p_data[i].type, p_data[i].id, p_data[i].name);
	return &p_data[i];
}

#ifdef CONFIG_HISI_NVE
static int power_nv_hisi_write(uint32_t nv_number, const char *nv_name,
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

static int power_nv_hisi_read(uint32_t nv_number, const char *nv_name,
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
#else
static int power_nv_hisi_write(uint32_t nv_number, const char *nv_name,
	const void *data, uint32_t data_len)
{
	return 0;
}

static int power_nv_hisi_read(uint32_t nv_number, const char *nv_name,
	void *data, uint32_t data_len)
{
	return 0;
}
#endif /* CONFIG_HISI_NVE */

int power_nv_write(enum power_nv_type type, const void *data, uint32_t data_len)
{
	struct power_nv_data_info *p_data = power_nv_get_data(type);

	if (!p_data)
		return -EINVAL;

	return power_nv_hisi_write(p_data->id, p_data->name, data, data_len);
}

int power_nv_read(enum power_nv_type type, void *data, uint32_t data_len)
{
	struct power_nv_data_info *p_data = power_nv_get_data(type);

	if (!p_data)
		return -EINVAL;

	return power_nv_hisi_read(p_data->id, p_data->name, data, data_len);
}
