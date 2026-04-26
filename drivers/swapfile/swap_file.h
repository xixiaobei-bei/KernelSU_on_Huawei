/*
 * swap_file.h
 *
 * Hisilicon devices test head file.
 *
 * Copyright (c) 2019-2020, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: pengrucheng@huawei.com
 * Create: 2020-04-08
 */
#ifndef _HISI_SWAP_FILE_H_
#define _HISI_SWAP_FILE_H_

#define SWAP_CMD_LEN 512
#define SWAPON_CMD "/sbin/swapon"
#define SWAPOFF_CMD "/sbin/swapoff"
#define SWAPFILE_NAME "/swap_file"

#define swap_file_err(fmt, ...) \
	pr_err("%s:"fmt"\n", __func__, ##__VA_ARGS__)

#define swap_file_info(fmt, ...) \
	pr_notice("%s:"fmt"\n", __func__, ##__VA_ARGS__)

#endif /* _HISI_SWAP_FILE_H_ */
