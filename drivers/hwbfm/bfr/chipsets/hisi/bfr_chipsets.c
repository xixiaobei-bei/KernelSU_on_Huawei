/*
 * bfr_chipsets.c
 *
 * define chipsets' external public enum/macros/interface for BFR
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
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

#include <chipset_common/bfmr/bfr/chipsets/bfr_chipsets.h>
#include <chipset_common/bfmr/common/bfmr_common.h>
#include <linux/mm.h>


#define BFR_RRECORD_BACKUP_PART_NAME "reserved2"
#define BFR_KB 1024
#define BFR_MIN_MEM_SIZE_FOR_BOPD (3 * BFR_KB * BFR_KB)

static bool g_bopd_enabled;

bool bfr_safe_mode_has_been_enabled(void)
{
	return !g_bopd_enabled;
}

bool bfr_bopd_has_been_enabled(void)
{
	return g_bopd_enabled;
}

static int __init bfr_bopd_is_enable(char *p)
{
	if (p) {
		BFMR_PRINT_KEY_INFO("early_param bopd_mem_sp is %s\n", p);
		if (!strncmp(p, "on", strlen("on")))
			g_bopd_enabled = true;
	}
	return 0;
}

early_param("bopd_mem_sp", bfr_bopd_is_enable);

int bfr_get_full_path_of_rrecord_part(char **path_buf)
{
	int i;
	bool find_full_path_of_rrecord_part = false;
	char *rrecord_names[BFR_RRECORD_PART_MAX_COUNT] = {
			BFR_RRECORD_PART_NAME,
			BFR_RRECORD_BACKUP_PART_NAME };
	int count = ARRAY_SIZE(rrecord_names);

	if (unlikely(!path_buf)) {
		BFMR_PRINT_INVALID_PARAMS("path_buf.\n");
		return -1;
	}

	*path_buf = (char *)bfmr_malloc(BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (*path_buf == NULL) {
		BFMR_PRINT_ERR("bfmr_malloc failed!\n");
		return -1;
	}

	for (i = 0; i < count; i++) {
		int ret;

		memset((void *)*path_buf, 0, BFMR_DEV_FULL_PATH_MAX_LEN + 1);

		ret = bfmr_get_device_full_path(rrecord_names[i], *path_buf,
						BFMR_DEV_FULL_PATH_MAX_LEN);
		if (ret != 0) {
			BFMR_PRINT_ERR("get full path for device [%s]failed!\n",
					rrecord_names[i]);
			continue;
		}

		find_full_path_of_rrecord_part = true;
		break;
	}

	if (!find_full_path_of_rrecord_part)
		return -1;
	else
		return 0;
}

