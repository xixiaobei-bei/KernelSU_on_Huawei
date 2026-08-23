/* Copyright (c) 2008-2019, Huawei Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#ifndef __DP_FACTORY_H__
#define __DP_FACTORY_H__

// true: factory_mode enable
// false: factory_mode disable
bool dp_factory_mode_is_enable(void);

void dp_factory_link_cr_or_ch_eq_fail(bool is_cr);
bool dp_factory_is_4k_60fps(uint8_t rate, uint8_t lanes, uint16_t h_active, uint16_t v_active, uint8_t fps);
int dp_factory_get_test_result(char *buffer, int size);

#endif // __DP_FACTORY_H__

