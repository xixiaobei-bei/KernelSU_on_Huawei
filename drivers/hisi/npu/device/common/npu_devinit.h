/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_DEVINIT_H
#define __NPU_DEVINIT_H

#include <linux/fs.h>
#include <linux/module.h>

int devdrv_devinit(struct module *owner, u8 dev_id,
		   const struct file_operations *devdrv_fops);

void devdrv_devexit(u8 dev_id);

#endif /*__DEVDRV_MANAGER_H*/
