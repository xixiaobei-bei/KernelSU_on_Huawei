/*
 * devfreq.h
 *
 * devfreq share
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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
#ifndef _HISI_DEVFREQ_H
#define _HISI_DEVFREQ_H
#include <linux/version.h>

int hisi_devfreq_free_freq_table(struct device *dev, unsigned long **table);
int hisi_devfreq_init_freq_table(struct device *dev, unsigned long **table);

#endif /* _DEVFREQ_H */
