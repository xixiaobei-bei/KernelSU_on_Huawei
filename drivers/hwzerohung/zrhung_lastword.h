/*
 * zrhung_lastword.h
 *
 * This file is the header file for zero hung last word
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

#ifndef __ZRHUNG_LASTWORD_H_
#define __ZRHUNG_LASTWORD_H_

#ifdef __cplusplus
extern "C" {
#endif

int hlastword_init(void);
int hlastword_read(void *buf, uint32_t len);
int hlastword_write(void *buf, uint32_t len);

#ifdef __cplusplus
}
#endif
#endif /* __ZRHUNG_LASTWORD_H_ */
