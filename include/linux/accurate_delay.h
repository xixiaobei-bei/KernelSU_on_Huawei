/*
 * accurate_delay.h
 *
 * hisi accurate delay
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2020. All rights reserved.
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

#ifndef _ACCURATE_DELAY_H_
#define _ACCURATE_DELAY_H_

#ifdef CONFIG_ACCURATE_DELAY
void accurate_delay_100us(unsigned long us_100);
#else
#include <asm-generic/delay.h>
static void accurate_delay_100us(unsigned long us_100)
{
	udelay(100*us_100);
}
#endif

#endif /* _ACCURATE_DELAY_H_ */
