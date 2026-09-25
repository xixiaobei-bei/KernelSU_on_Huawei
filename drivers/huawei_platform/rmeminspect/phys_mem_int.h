/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: ddr inspect phys_mem_dev init
 * Author: zhouyubin
 * Create: 2019-05-30
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

#ifndef PHYS_MEM_INT_H
#define PHYS_MEM_INT_H

#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/semaphore.h>

#include <dsm/dsm_pub.h>

#include "phys_mem.h"

struct phys_mem_dev {
	struct phys_mem_dev *next; // next listitem
	struct semaphore sem; // Mutual exclusion
	struct cdev cdev;
};

#define SESSION_STATE_CLOSED		0
#define SESSION_STATE_OPEN		1
#define SESSION_STATE_CONFIGURING	2
#define SESSION_STATE_CONFIGURED	3
#define SESSION_STATE_MAPPED		4

#define SESSION_STATE_INVALID		5

/* Legal states */
#define SESSION_NUM_STATES		5

#define get_state(session) ((session)->status.state)
#define set_session_state(session, value) set_state((session), (value))

struct session_status {
	unsigned int state;
};

/* A session as described at the top of the file */
struct phys_mem_session {
	struct session_status status;
	unsigned long long session_id;
	int vmas; /* active mappings */
	struct phys_mem_dev *device;
	struct semaphore sem; /* Session Lock */
	/* The number of frame stati in status */
	unsigned long num_frame_stati;
	/* An array with num_status items */
	struct phys_mem_frame_status *frame_stati;
};

extern struct phys_mem_dev *g_phys_mem_devices;

extern struct dsm_client *ddr_dsm_client;

void free_page_stati(struct phys_mem_session *session);

/* The size of a number of frame-stati */
#define	session_frame_stati_size(num) \
	((num)*sizeof(struct phys_mem_frame_status))

/* Allocating and freeing memory for the frame-stati (session.frame_stati) */
#define	session_free_frame_stati(p) vfree(p)
#define	session_alloc_num_frame_stati(num) \
	vmalloc(session_frame_stati_size(num))


extern const struct file_operations phys_mem_fops;
extern struct kmem_cache *g_session_mem_cache;

static __attribute__((unused)) char *session_state_txt[] = {
	"SESSION_STATE_CLOSED",
	"SESSION_STATE_OPEN",
	"SESSION_STATE_CONFIGURING",
	"SESSION_STATE_CONFIGURED",
	"SESSION_STATE_MAPPED",
	"- INVALID -"
};

static inline void set_state(struct phys_mem_session *session,
				unsigned int new_state)
{
	if (new_state >= SESSION_NUM_STATES)
		new_state = SESSION_STATE_INVALID;
	session->status.state = new_state;
}

extern unsigned long g_reboot_flag_ddr;
extern unsigned long g_pre_isolate_ddr;
enum fault_type {
	NORMAL_REBOOT = 0,
	AP_S_PANIC,
	AP_S_AWDT,
	AP_S_DDRC_SEC,
	AP_S_DDR_UCE_WD,
	AP_S_DDR_FATAL_INTER,
	AP_S_PRESS6S,
	AP_S_COMBINATIONKEY,
	AP_S_F2FS,
	AP_S_BL31_PANIC,
	BR_PRESS_10S,
	BFM_S_NATIVE_BOOT_FAIL,
	BFM_S_BOOT_TIMEOUT,
	BFM_S_FW_BOOT_FAIL,
	BFM_S_NATIVE_DATA_FAIL,
	LPM3_S_LPMCURST,
	LPM3_S_EXCEPTION,
	NR_ERR_TYPES
};

extern unsigned int g_conti_nr_pfn;
extern unsigned int g_conti_order;

struct ddr_debugfs_files {
	struct dentry *debugfs_root;
	struct dentry *host_error;
};

#define MY_NAME	"meminspect"
#define MY_PREFIX	"module"

#endif /* PHYS_MEM_INT_H_ */
