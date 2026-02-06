/*
 * governor_pm_qos.h
 *
 * governor_pm_qos head file for ddr
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __GOVERNOR_PM_QOS_H__
#define __GOVERNOR_PM_QOS_H__

/*
 * struct devfreq_pm_qos_data : void *data fed to struct devfreq and devfreq_add_device
 * bytes_per_sec_per_hz : Ratio to convert throughput request to devfreq frequency
 * pm_qos_class : pm_qos class to query for requested throughput
 */
struct devfreq_pm_qos_data {
	unsigned int bytes_per_sec_per_hz;
	unsigned int bd_utilization;
	int pm_qos_class;
	unsigned long freq;
};

#endif /* __GOVERNOR_PM_QOS_H__ */
