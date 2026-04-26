/*
 * npu_pm.h
 *
 * header file of npu pm driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#ifndef __NPU_PM_H__
#define __NPU_PM_H__


int npu_pm_power_on(void);
int npu_pm_power_off(void);

/* return mv */
int hisi_npu_get_voltage(void);

#endif /* __NPU_PM_H__ */
