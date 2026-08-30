/*
 * record the diaginfo for mntn module
 *
 * Copyright (c) 2018 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <libhwsecurec/securec.h>

/* check cpu_up is OK, if not record the error status */
void cpu_up_diaginfo_record(unsigned int cpu, int status)
{
	/* cpu up fail */
	if(status){
		bbox_diaginfo_record(CPU_UP_FAIL, "cpu up failed, cpu: %d, fail reason: %d", cpu, status);
	}
}