/*
 * power_pwm.h
 *
 * power pwm interface for power module
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _POWER_PWM_H_
#define _POWER_PWM_H_

#include <linux/pwm.h>

struct pwm_device *power_pwm_get_dev(struct device *dev, const char *con_id);
void power_pwm_put(struct pwm_device *dev);
int power_pwm_apply_state(struct pwm_device *dev, struct pwm_state *state);

#endif /* _POWER_PWM_H_ */
