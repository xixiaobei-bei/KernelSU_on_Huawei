/*
 * mntn_diaginfo_record.c
 *
 * record the diaginfo for mntn module
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
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <securec.h>
#include <linux/module.h>
#include <linux/kernel.h>
/* check cpu_up is OK, if not record the error status */
void cpu_up_diaginfo_record(unsigned int cpu, int status)
{
	/* cpu up fail */
	if (status)
		bbox_diaginfo_record(CPU_UP_FAIL, NULL, "cpu up failed, cpu: %u, fail reason: %d", cpu, status);
}
