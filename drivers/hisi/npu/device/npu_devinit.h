/*
 * npu_devinit.h
 *
 * about npu devinit
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
#ifndef __NPU_DEVINIT_H
#define __NPU_DEVINIT_H

#include <linux/fs.h>
#include <linux/module.h>

int devdrv_devinit(struct module *owner, u8 dev_id);

void devdrv_devexit(u8 dev_id);

#endif /* __DEVDRV_MANAGER_H */
