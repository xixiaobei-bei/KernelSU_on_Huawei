/*
 * zrhung_ioctl.c
 *
 * Frozen screen solution interface provided to the upper layer
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <linux/aio.h>
#include <uapi/linux/uio.h>
#include <asm/ioctls.h>
#include <chipset_common/hwlogger/hw_logger.h>

#include "chipset_common/hwzrhung/zrhung.h"
#include "zrhung_config.h"
#include "zrhung_transtation.h"
#include "zrhung_common.h"

long zrhung_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = ZRHUNG_CMD_INVALID;

	if ((cmd < LOGGER_WRITE_HEVENT) || (cmd >= LOGGER_CMD_MAX))
		return ret;

	switch (cmd) {
	case LOGGER_WRITE_HEVENT:
		ret = htrans_write_event((void *)arg);
		break;
	case LOGGER_READ_HEVENT:
		ret = htrans_read_event((void *)arg);
		break;
	case LOGGER_GET_HLASTWORD:
		ret = htrans_read_lastword((void *)arg);
		break;
	case LOGGER_SET_HCFG:
		ret = hcfgk_set_cfg(file, (void *)arg);
		break;
	case LOGGER_GET_HCFG:
		ret = hcfgk_ioctl_get_cfg(file, (void *)arg);
		break;
	case LOGGER_SET_HCFG_FLAG:
		ret = hcfgk_set_cfg_flag(file, (void *)arg);
		break;
	case LOGGER_GET_HCFG_FLAG:
		ret = hcfgk_get_cfg_flag(file, (void *)arg);
		break;
	case LOGGER_SET_FEATURE:
		ret = hcfgk_set_feature(file, (void *)arg);
	default:
		break;
	}

	return ret;
}
EXPORT_SYMBOL(zrhung_ioctl);
