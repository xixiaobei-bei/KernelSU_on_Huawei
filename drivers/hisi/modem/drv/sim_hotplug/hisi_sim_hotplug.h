/*
 * Header file for device driver SIM HOTPLUG
 *
 * Copyright (c) 2013 Linaro Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __HISI_SIM_HOTPLUG_H
#define __HISI_SIM_HOTPLUG_H

#include <linux/irqdomain.h>
#include "bsp_icc.h"

#define SIM0                (0)
#define SIM1                (1)
#define SIM_CARD_OUT        (0)
#define SIM_CARD_IN         (1)

#define SIM0_CHANNEL_ID     ((ICC_CHN_IFC<< 16) | IFC_RECV_FUNC_SIM0)
#define SIM1_CHANNEL_ID     ((ICC_CHN_IFC<< 16) | IFC_RECV_FUNC_SIM1)

#define SIMHP_FIRST_MINOR		0
#define SIMHP_DEVICES_NUMBER		1

#define SIMHP_NAME_BASE			"simhotplug"

#define SIMHOTPLUG_IOC_MAGIC		'k'

#define SIMHOTPLUG_IOC_INFORM_CARD_INOUT	_IOWR(SIMHOTPLUG_IOC_MAGIC, 1,int32_t)

#endif      /* __HISI_SIM_HOTPLUG_H */
