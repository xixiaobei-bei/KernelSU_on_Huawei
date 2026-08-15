/*
 * zrhung_common.h
 *
 * This file is the common header file for zero hung
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ZRHUNG_COMMON_H_
#define __ZRHUNG_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HTRANS_ERROR(format, ...) \
	pr_err("[*%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)

#define HTRANS_INFO(format, ...) \
	pr_devel("[%s %d] " format, __func__, __LINE__, ##__VA_ARGS__)

#define MAGIC_NUM 0x9517
#define __HTRANSIO 0xAB
#define LOGGER_WRITE_HEVENT     _IO(__HTRANSIO, 1)
#define LOGGER_READ_HEVENT      _IO(__HTRANSIO, 2)
#define LOGGER_SET_HCFG         _IO(__HTRANSIO, 3)
#define LOGGER_GET_HCFG         _IO(__HTRANSIO, 4)
#define LOGGER_GET_HLASTWORD    _IO(__HTRANSIO, 5)
#define LOGGER_SET_HCFG_FLAG    _IO(__HTRANSIO, 6)
#define LOGGER_GET_HCFG_FLAG    _IO(__HTRANSIO, 7)
#define LOGGER_SET_FEATURE      _IO(__HTRANSIO, 8)
#define LOGGER_CMD_MAX          _IO(__HTRANSIO, 9)

#ifdef __cplusplus
}
#endif
#endif /* __ZRHUNG_COMMON_H_ */
