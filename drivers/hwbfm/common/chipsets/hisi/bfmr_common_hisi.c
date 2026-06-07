/*
 * bfmr_common_hisi.c
 *
 * define common interface for BFMR (Boot Fail Monitor and Recovery) on HISI
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

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/hisi/kirin_partition.h>
#include <hisi_partition.h>
#include <chipset_common/bfmr/common/bfmr_common.h>

/*
 * function: int bfm_get_device_full_path(char *dev_name, char *path_buf,
 *                                             unsigned int path_buf_len)
 * brief: get full path of the "dev_name".
 *
 * param: dev_name [in] device name such as: boot/recovery/rrecord.
 *        path_buf [out] buffer will store the full path of "dev_name".
 *        path_buf_len [in] length of the path_buf.
 *
 * return: 0 - succeeded; -1 - failed.
 *
 * note:
 */
int bfmr_get_device_full_path(char *dev_name, char *path_buf,
		unsigned int path_buf_len)
{
	int ret;

	if (unlikely(!dev_name || !path_buf)) {
		BFMR_PRINT_INVALID_PARAMS("dev_name or path_buf\n");
		return -1;
	}

	ret = flash_find_ptn_s(dev_name, path_buf, path_buf_len);
	if (ret != 0) {
		BFMR_PRINT_ERR("find full path for: [%s] failed\n", dev_name);
		return -1;
	}

	return 0;
}


/*
 * function: unsigned int bfmr_get_bootup_time(void)
 * brief: get bootup time.
 *
 * param:
 *
 * return: bootup time(seconds).
 *
 * note:
 */
unsigned int bfmr_get_bootup_time(void)
{
	u64 ts;

#ifdef CONFIG_HISI_TIME
	ts = hisi_getcurtime();
#else
	ts = sched_clock();
#endif

	return (unsigned int)(ts / 1000 / 1000 / 1000);
}
