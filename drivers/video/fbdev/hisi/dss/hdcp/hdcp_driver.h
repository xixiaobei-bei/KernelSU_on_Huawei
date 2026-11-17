/*
 *               (C) COPYRIGHT 2014 - 2016 SYNOPSYS, INC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Copyright (c) 2017 Hisilicon Tech. Co., Ltd. Integrated into the Hisilicon display system.
 */

#ifndef HDCP_DRIVER_H
#define HDCP_DRIVER_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h>
#endif
#include "hdcp_common.h"

#define HISI_HDCP_IOCTL_MAGIC 'H'

#define HDCP_DRIVER_SUCCESS 0
#define HDCP_DRIVER_INVALID_PARAM (-1)
#define HDCP_DRIVER_NO_ACCESS (-2)
#define HDCP_DRIVER_IOCTL_FAILED (-3)

enum hdcp_soft_auth_state {
	AUTH_START = 0,
	AUTH_END = 1,
	IRQ_IN = 2,
	IRQ_OUT = 3,
	SOFT_AUTH_EN = 4,
};

struct hdcp_common_cmd_ioctl {
	uint8_t cmd;
	uint8_t soft_auth_enable;
	int returned_status;
};

/* ioctl cmd from 0x01 ~ 0x0c in host_lib_driver_linux_if.h */
#define HDCP_GENERAL_CMD \
	_IOW(HISI_HDCP_IOCTL_MAGIC, 0xd, struct hdcp_common_cmd_ioctl)

#endif

