/*
 * vibrator_channel.h
 *
 * huawei x-ray vibrator driver
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

#ifndef _VIBRATOR_CHANNEL_H_
#define _VIBRATOR_CHANNEL_H_

extern int get_iomcu_power_state(void);
int write_vibrator_calib_value_to_nv(const char *temp, uint16_t length);

#endif /* _VIBRATOR_CHANNEL_H_ */
