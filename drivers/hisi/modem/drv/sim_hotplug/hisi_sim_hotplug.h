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
#include <bsp_icc.h>

#define SIM0                (0)
#define SIM1                (1)
#define SIM_CARD_OUT        (0)
#define SIM_CARD_IN         (1)



#define SIMHP_OK 0
#define SIMHP_PMU_IO_SET_FAIL 1
#define SIMHP_PMU_VCC_SET_FAIL 3
#define SIMHP_PMU_IO_ENABLE_FAIL 5
#define SIMHP_PMU_IO_DISABLE_FAIL 6
#define SIMHP_PMU_VCC_ENABLE_FAIL 7
#define SIMHP_PMU_VCC_DISABLE_FAIL 8
#define SIMHP_NULL_POINTER 9
#define SIMHP_INVALID_PARAM 10
#define SIMHP_INVALID_CHNL 10
#define SIMHP_NO_IO_PMU 11
#define SIMHP_NO_VCC_PMU 12
#define SIMHP_NO_SWITCH_PMU 13
#define SIMHP_MSG_RECV_ERR 14
#define SIMHP_NO_POWER_SCHEME 15


#ifdef CONFIG_EXTRA_MODEM_SIM
#define SIM0_CHANNEL_ID     ((ICC_CHN_APLUSB_IFC<< 16) | APLUSB_IFC_FUNC_SIM0)
#define SIM1_CHANNEL_ID     ((ICC_CHN_APLUSB_IFC<< 16) | APLUSB_IFC_FUNC_SIM1)
#else
#define SIM0_CHANNEL_ID     ((ICC_CHN_IFC<< 16) | IFC_RECV_FUNC_SIM0)
#define SIM1_CHANNEL_ID     ((ICC_CHN_IFC<< 16) | IFC_RECV_FUNC_SIM1)
#endif

#define SIMHP_FIRST_MINOR		0
#define SIMHP_DEVICES_NUMBER		1

#define SIMHP_NAME_BASE			"simhotplug"

#define SIMHOTPLUG_IOC_MAGIC		'k'

#define SIMHOTPLUG_IOC_INFORM_CARD_INOUT	_IOWR(SIMHOTPLUG_IOC_MAGIC, 1,int32_t)


typedef enum
{
    CARD_MSG_OUT       = 0,
    CARD_MSG_IN        = 1,
    CARD_MSG_LEAVE     = 3,
    CARD_MSG_SIM       = 5,
    CARD_MSG_SD        = 6,
    CARD_MSG_NO_CARD   = 7,
    CARD_MSG_SD2JTAG   = 8,
    CARD_MSG_PMU_ON   = 9,
    CARD_MSG_PMU_OFF   = 10,
    CARD_MSG_PMU_FAIL   = 11,
} SCI_ACORE_RESPONSE;

typedef struct
{
    u32 msgtype;  
    u32 msgvalue;
    u32 msgsn;
    u32 timestamp;
}SCI_MSG_CTRL;



#endif      /* __HISI_SIM_HOTPLUG_H */
