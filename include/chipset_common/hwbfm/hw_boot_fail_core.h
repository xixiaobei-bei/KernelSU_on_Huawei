/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: hw_boot_file_core.h
 *
 * define boot file core
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

#ifndef HW_BOOT_FAIL_CORE_H
#define HW_BOOT_FAIL_CORE_H

#include <linux/types.h>

/*
 * Description: save dfx and bfi partition's log to file
 */
void save_bootfail_info_to_file(void);

#ifdef CONFIG_HUAWEI_BFM
bool bfmr_is_enabled(void);
#else
static inline bool bfmr_is_enabled(void)
{
	return false;
}
#endif

#endif
