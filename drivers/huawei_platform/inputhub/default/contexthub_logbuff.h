/*
 * drivers/inputhub/contexthub_logbuff.h
 *
 * sensors sysfs header
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

#ifndef __LINUX_SENSORHUB_LOGBUFF_H__
#define __LINUX_SENSORHUB_LOGBUFF_H__

#include "contexthub_boot.h"

typedef struct {
	pkt_header_t hd;
	uint32_t index;
} log_buff_req_t;

extern struct CONFIG_ON_DDR *pConfigOnDDr;
extern int set_log_level(int tag, int argv[], int argc);

#endif
