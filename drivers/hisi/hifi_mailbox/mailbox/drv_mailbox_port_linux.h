/*
 * drv_mailbox_port_linux.h
 *
 * mailbox implement on linux platform
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
 *
 */
#ifndef __DRV_MAILBOX_PORT_LINUX_H__
#define __DRV_MAILBOX_PORT_LINUX_H__

#include <linux/kernel.h>

/*
 * At present, We has not pre-mapped the reserved virtual address space.
 * Do the mapping in the init function.
 */
#define MEM_CORE_SHARE_PHY2VIRT(phy) (phy)
#define MEM_CORE_SHARE_VIRT2PHY(virt) (virt)

/* ID of this CPU */
#define MAILBOX_LOCAL_CPUID MAILBOX_CPUID_ACPU

/*
 * Define the total number of mailbox channels related to the C core platform,
 * and care about the sending and receiving channels
 */
#define MAILBOX_CHANNEL_NUM \
	(MAILBOX_CHANNEL_BUTT(ACPU, HIFI) + MAILBOX_CHANNEL_BUTT(HIFI, ACPU))

/*
 * Define the total number of emails related to the C core platform,
 * only concerned with the receiving channel
 */
#define MAILBOX_USER_NUM (MAILBOX_USER_BUTT(HIFI, ACPU, MSG))

/* Define whether the macro controls whether to print the number of lines */
#define _MAILBOX_LINE_ __LINE__

/*
 * Define macro control whether to print the file name,
 * do not print the file name to save code space
 */
#define _MAILBOX_FILE_ (void *)(0) /* __FILE__ */
#define MAILBOX_LOG_LEVEL MAILBOX_LOG_ERROR

/* Definition of configuration related to mailbox debugging */
/* Indicates that the om function is turned on on this CPU */
#ifndef MAILBOX_OPEN_MNTN
#define MAILBOX_OPEN_MNTN
#endif

/* The maximum number of transmission IDs that can be recorded */
#define MAILBOX_RECORD_USEID_NUM 64

#endif /* __DRV_MAILBOX_PORT_LINUX_H__ */
