/*
 * power_delay.c
 *
 * delay interface for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <chipset_common/hwpower/common_module/power_delay.h>

/*
 * power_usleep - sleep for an approximate time
 * @delay: total time in usecs to sleep
 *
 * Notes that if delay > 20ms, please use power_msleep instead.
 */
void power_usleep(unsigned int delay)
{
	unsigned int delay_max;

	if (delay <= DT_USLEEP_2MS)
		delay_max = delay + DT_USLEEP_100US;
	else if (delay <= DT_USLEEP_5MS)
		delay_max = delay + DT_USLEEP_500US;
	else
		delay_max = delay + DT_USLEEP_1MS;

	usleep_range(delay, delay_max);
}

/*
 * power_msleep - sleep for an approximate time
 * @delay: total time in msecs to sleep
 * @interval: interval time in msecs to each sleep
 * @exit: function pointer, if true, stop sleep
 *
 * Notes that if delay <= 20ms, please use power_usleep instead.
 *
 * Returns true when sleep completely, otherwise false.
 */
bool power_msleep(unsigned int delay, unsigned int interval, bool (*exit)(void))
{
	int i;
	int cnt;

	if (!exit || !interval) {
		msleep(delay);
		return true;
	}

	if ((interval < DT_MSLEEP_20MS) || (delay < interval))
		return false;

	cnt = delay / interval;
	for (i = 0; i < cnt; i++) {
		msleep(interval);
		if (exit())
			return false;
	}

	return true;
}
