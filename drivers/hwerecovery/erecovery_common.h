/*
 * erecovery_common.h
 *
 * This file is the common header file for erecovery
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ERECOVERY_COMMON_H_
#define __ERECOVERY_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ERECOVERY_ERROR(format, ...) \
	pr_err("[*%s %d]" format, __func__, __LINE__, ##__VA_ARGS__)

#define ERECOVERY_INFO(format, ...) \
	pr_devel("[%s %d]" format, __func__, __LINE__, ##__VA_ARGS__)

#define ERECOVERY_MAGIC_NUM 0x9527
#define ERECIO 0xAD
#define ERECOVERY_WRITE_EVENT _IO(ERECIO, 1)
#define ERECOVERY_READ_EVENT _IO(ERECIO, 2)

#ifdef __cplusplus
}
#endif
#endif
