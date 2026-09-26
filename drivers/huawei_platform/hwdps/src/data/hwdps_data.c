/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file contains the function definations required for
 *              data list.
 * Create: 2020-06-16
 */

#include "inc/data/hwdps_data.h"
#include <linux/rwsem.h>
#include "inc/data/hwdps_packages.h"

static struct rw_semaphore g_data_lock;

void hwdps_data_init(void)
{
	init_rwsem(&g_data_lock);
	down_write(&g_data_lock);
	up_write(&g_data_lock);
}

void hwdps_data_deinit(void)
{
	down_write(&g_data_lock);
	hwdps_packages_delete_all();
	up_write(&g_data_lock);
}

void hwdps_data_read_lock(void)
{
	down_read(&g_data_lock);
}

void hwdps_data_read_unlock(void)
{
	up_read(&g_data_lock);
}

void hwdps_data_write_lock(void)
{
	down_write(&g_data_lock);
}

void hwdps_data_write_unlock(void)
{
	up_write(&g_data_lock);
}
