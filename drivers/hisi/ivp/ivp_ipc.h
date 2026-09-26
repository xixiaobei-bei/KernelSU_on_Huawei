/*
 * ivp ipc communication
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

#ifndef _IVP_IPC_H_
#define _IVP_IPC_H_
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/atomic.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/platform_device.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/notifier.h>


struct ivp_ipc_packet {
	char *buff;
	size_t len;
	struct list_head list;
};

struct ivp_ipc_queue {
	struct list_head head;
	spinlock_t rw_lock;
	struct semaphore r_lock;
	atomic_t flush;
};

struct ivp_ipc_device {
	struct miscdevice device;
	struct notifier_block recv_nb;
	struct ivp_ipc_queue recv_queue;
	atomic_t accessible;
	unsigned int ivp_secmode;
	struct mutex ivp_ion_mutex;
	struct mutex ivp_ipc_read_mutex;
	rproc_id_t recv_ipc;
	rproc_id_t send_ipc;
	struct platform_device *ipc_pdev;
};
#endif /* _IVP_IPC_H_ */

