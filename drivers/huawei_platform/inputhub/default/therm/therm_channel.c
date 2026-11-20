/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: therm channel source file
 * Author: linjianpeng <linjianpeng1@huawei.com>
 * Create: 2020-05-25
 */

#include "therm_channel.h"

#include <linux/err.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/types.h>

#include <securec.h>

#include "contexthub_route.h"
#include "therm_sysfs.h"

void reset_therm_calibrate_data(void)
{
	if (strlen(get_sensor_chip_info_address(THERMOMETER)) == 0)
		return;
	hwlog_info("%s\n", __func__);
}

