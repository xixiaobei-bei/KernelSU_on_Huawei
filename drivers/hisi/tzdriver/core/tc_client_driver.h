/*
 * tc_client_driver.h
 *
 * function declaration for proc open,close session and invoke
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
 */
#ifndef TC_CLIENT_DRIVER_H
#define TC_CLIENT_DRIVER_H

#include <linux/list.h>
#include "teek_ns_client.h"

struct tc_ns_dev_list *get_dev_list(void);
uint32_t tc_ns_get_uid(void);
struct tc_ns_dev_file *tc_find_dev_file(unsigned int dev_file_id);
int tc_ns_client_open(struct tc_ns_dev_file **dev_file, uint8_t kernel_api);
int tc_ns_client_close(struct tc_ns_dev_file *dev);
int is_agent_alive(unsigned int agent_id);

#ifdef CONFIG_ACPI
int get_acpi_tz_irq(void);
#endif

#endif
