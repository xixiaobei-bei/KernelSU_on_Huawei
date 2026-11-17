/*
 * power_pwm.c
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

#include <chipset_common/hwpower/common_module/power_pwm.h>
#include <chipset_common/hwpower/common_module/power_printk.h>

#define HWLOG_TAG power_pwm
HWLOG_REGIST();

struct pwm_device *power_pwm_get_dev(struct device *dev, const char *con_id)
{
	struct pwm_device *pwm_dev = NULL;

	pwm_dev = pwm_get(dev, con_id);
	if (IS_ERR(pwm_dev)) {
		hwlog_err("pwm_get_dev: failed\n");
		return NULL;
	}

	hwlog_info("pwm_get_dev: label=%s\n", pwm_dev->label);
	return pwm_dev;
}

void power_pwm_put(struct pwm_device *dev)
{
	pwm_put(dev);
}

int power_pwm_apply_state(struct pwm_device *dev, struct pwm_state *state)
{
	if (!dev || !dev->chip || !dev->chip->ops || !dev->chip->ops->apply) {
		hwlog_err("pwm_apply_state: failed\n");
		return -ENOTSUPP;
	}

	hwlog_info("pwm_apply_state: label=%s period=%u duty_cycle=%u polarity=%u enabled=%u\n",
		dev->label, state->period, state->duty_cycle, state->polarity, state->enabled);
	return dev->chip->ops->apply(dev->chip, dev, state);
}
