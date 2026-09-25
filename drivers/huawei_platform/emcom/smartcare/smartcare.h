/*
 * Copyright (c) 2018-2022 Huawei Technologies Co., Ltd.
 * Description: define macros and structs for smartcare.c
 * Author: liyouyong liyouyong@huawei.com
 * Create: 2018-09-10
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
#ifndef __SMARTCARE_H_INCLUDED__
#define __SMARTCARE_H_INCLUDED__

#include <linux/types.h>

void smartcare_event_process(int32_t event, uint8_t *pdata, uint16_t len);
void smartcare_init(void);
void smartcare_deinit(void);

#endif // __SMARTCARE_H_INCLUDED__
