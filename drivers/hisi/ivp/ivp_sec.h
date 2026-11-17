/*
 * This file is header file for ivp_sec.c lists variable and functions
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

#ifndef _IVP_SEC_H_
#define _IVP_SEC_H_
#include <linux/version.h>
#include "ivp_ipc.h"
#include "ivp_platform.h"

#define SIZE_1M_ALIGN          BIT(20)
#define DEFAULT_MSG_SIZE       32

/* head + fdnum + sharefd = 3, now 5 */
#define MAX_FD_NUM (DEFAULT_MSG_SIZE / sizeof(unsigned int) - 3)

int ivp_get_secbuff(
	struct device *dev,
	int sec_buf_fd,
	unsigned long *sec_buf_phy_addr);

int ivp_trans_sharefd_to_phyaddr(struct ivp_ipc_device *pdev,
	unsigned int *buff, unsigned int size __attribute__((unused)));
int ivp_create_secimage_thread(struct ivp_device *ivp_devp);
int ivp_destroy_secimage_thread(struct ivp_device *ivp_devp);
int ivp_sec_load(struct ivp_device *ivp_devp);
int ivp_sec_poweron_remap(struct ivp_device *ivp_devp);

#endif /* _IVP_SEC_H_ */
