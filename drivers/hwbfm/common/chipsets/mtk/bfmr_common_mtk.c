/*
 * file: bfmr_common_mtk.c
 *
 * define the common interface for BFMR (Boot Fail Monitor and Recovery) on MTK
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

#include <linux/kernel.h>
#include <chipset_common/bfmr/common/bfmr_common.h>
#include <linux/rtc.h>
#include <linux/sched/clock.h>

/*
 * @function: int bfm_get_device_full_path(char *dev_name,
 *            char *path_buf, unsigned int path_buf_len)
 * @brief: get full path of the "dev_name".
 * @param: dev_name [in] device name such as: boot/recovery/rrecord.
 * @param: path_buf [out] buffer will store the full path of "dev_name".
 * @param: path_buf_len [in] length of the path_buf.
 * @return: 0 - succeeded; -1 - failed.
 */
int bfmr_get_device_full_path(char *dev_name, char *path_buf,
	unsigned int path_buf_len)
{
	if (unlikely((dev_name == NULL) || (path_buf == NULL))) {
		BFMR_PRINT_INVALID_PARAMS("dev_name: %p, path_buf: %p\n",
			dev_name, path_buf);
		return -1;
	}

	snprintf(path_buf, path_buf_len - 1,
		"/dev/block/bootdevice/by-name/%s", dev_name);

	return 0;
}

/*
 * @function: unsigned int bfmr_get_bootup_time(void)
 * @brief: get bootup time.
 * @return: bootup time(seconds).
 */
unsigned int bfmr_get_bootup_time(void)
{
	u64 ts;

	ts = sched_clock();
	/* Time unit conversion */
	return (unsigned int)(ts / 1000 / 1000 / 1000);
}

