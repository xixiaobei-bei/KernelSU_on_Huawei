/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * Description: laser vi5300 driver
 *
 * Author: lvyali
 *
 * Create: 2021-01-28
 */

#ifndef _LASER_MODULE_H_
#define _LASER_MODULE_H_

#include <linux/module.h>
#include <linux/i2c.h>
#include <media/huawei/laser_cfg.h>

#define laser_err(fmt, ...) pr_err("[laser]ERROR: " fmt "\n", ##__VA_ARGS__)
#define laser_info(fmt, ...) pr_info("[laser]INFO: " fmt "\n", ##__VA_ARGS__)
#define laser_dbg(fmt, ...) pr_debug("[laser]DBG: " fmt "\n", ##__VA_ARGS__)

typedef struct _tag_laser_module_intf_t {
	char *laser_name;
	int (*data_init)(struct i2c_client *client,
		const struct i2c_device_id *id);
	int (*data_remove)(struct i2c_client *client);
	long (*laser_ioctl)(void *hw_data, unsigned int cmd, void *p);
} laser_module_intf_t;

#endif

