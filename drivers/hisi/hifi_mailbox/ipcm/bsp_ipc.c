/*
 * bsp_ipc.c
 *
 * ipc driver implements
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

#include "bsp_drv_ipc.h"

#include <linux/module.h>
#include <linux/io.h>
#include <linux/string.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/audio_log.h>

#include "soc_acpu_baseaddr_interface.h"

#define LOG_TAG "mailbox_ipc"

#define IPC_MSG_LEN 2

/* hifi system ipc */
#define SYS_IPC_BASE_ADDR_S (unsigned long)(SOC_ACPU_IPC_BASE_ADDR)
#define SYS_IPC_BASE_ADDR_NS (unsigned long)(SOC_ACPU_IPC_NS_BASE_ADDR)

#define SYS_IPC_CORE_LIT 0
#define SYS_IPC_CORE_BIG 1
#define SYS_IPC_CORE_HIFI 4
#define MAILBOX_SEND_NUM 18

#define MAILBOX_REC_NUM 2
#define INT_SEND_NUM 220
#define INT_REC_NUM 227

enum {
	/* ap, npm3, and iom3 core inside hifi are all corresponding 0 bit */
	HIFI_IPC_CORE_AP_LPM3_IOM3 = 0,
	HIFI_IPC_CORE_MODEM_A9 = 1,
	HIFI_IPC_CORE_MODEM_BBE = 2,
	HIFI_IPC_CORE_HIFI = 3,
	HIFI_IPC_CORE_IOM3 = 5,
	HIFI_IPC_CORE_AP = 6,
	HIFI_IPC_CORE_LPM3 = 8
};

enum ipc_mode {
	SEC_SYS_IPC = 0,
	UNSEC_SYS_IPC,
	HIFI_IPC
};

enum {
	IPC_CORE_IS_SEND = 0,
	IPC_CORE_IS_RECEIVE,
	IPC_CORE_IS_UNKNOEN
};

struct ipc_config {
	/* Use system ipc or audio ipc */
	enum ipc_mode mode;
	/* Ipc base address after mapping */
	void __iomem *base;
	/* Mailbox number */
	uint32_t mailbox_num;
	/* Interrupt number */
	uint32_t int_num;
	/* Source core number */
	uint32_t source;
	/* Target core number */
	uint32_t dest;
};

/* Each core should have two structures, one for each transceiver */
static struct ipc_config g_ipc_config[IPC_CORE_IS_UNKNOEN] = {
	/* This core is used as the send ipc side */
	{
		UNSEC_SYS_IPC,
		NULL,
		MAILBOX_SEND_NUM,
		INT_SEND_NUM,
		SYS_IPC_CORE_LIT,
		SYS_IPC_CORE_HIFI
	},

	/* This core is used as rec ipc side */
	{
		UNSEC_SYS_IPC,
		NULL,
		MAILBOX_REC_NUM,
		INT_REC_NUM,
		SYS_IPC_CORE_HIFI,
		SYS_IPC_CORE_BIG
	}
};

static bool g_initialized;
static struct notifier_block g_rx_nb;
static spinlock_t g_ipc_int_lock = __SPIN_LOCK_UNLOCKED("ipc");
static struct ipc_debug g_ipc_debug = {0};
static struct bsp_ipc_entry g_ipc_int_table[INTSRC_NUM];

static int ipc_int_handler(struct notifier_block *nb, unsigned long len,
	void *mbox_msg)
{
	uint32_t level;
	mbox_msg_t *msg = (mbox_msg_t *)mbox_msg;
	struct bsp_ipc_entry *table = g_ipc_int_table;

	level = msg[0];

	/* Call the registered interrupt handler function */
	if (level < INTSRC_NUM) {
		g_ipc_debug.int_handle_times[level]++;

		if (table[level].routine != NULL)
			table[level].routine(table[level].arg);
	}

	return 0;
}

static int audio_hifi_mbox_init(void)
{
	int ret;
	rproc_id_t rproc_id;

	g_rx_nb.next = NULL;
	g_rx_nb.notifier_call = ipc_int_handler;

	rproc_id = HISI_HIFI_ACPU_MBX_1;
	/* register the rx notify callback */
	ret = RPROC_MONITOR_REGISTER(rproc_id, &g_rx_nb);
	if (ret)
		AUDIO_LOGE("rproc monitor register failed");

	return ret;
}

static void audio_hifi_mbox_deinit(void)
{
	rproc_id_t rproc_id = HISI_HIFI_ACPU_MBX_1;
	RPROC_MONITOR_UNREGISTER(rproc_id, &g_rx_nb);
}

int drv_ipc_int_init(void)
{
	if (g_initialized == true)
		return 0;

	memset(g_ipc_int_table, 0, sizeof(g_ipc_int_table));

	audio_hifi_mbox_init();

	g_initialized = true;

	AUDIO_LOGI("succeed");
	return 0;
}

void drv_ipc_int_deinit(void)
{
	audio_hifi_mbox_deinit();
}

int ipc_int_connect(enum ipc_int_lev level, irq_handler routine, uint32_t para)
{
	unsigned long flag = 0;
	struct bsp_ipc_entry *table = g_ipc_int_table;

	spin_lock_irqsave(&g_ipc_int_lock, flag);
	table[level].routine = routine;
	table[level].arg = para;
	spin_unlock_irqrestore(&g_ipc_int_lock, flag);

	return 0;
}

int ipc_int_send(enum ipc_int_core dst_core, enum ipc_int_lev level)
{
	int ret;
	int role = IPC_CORE_IS_SEND;
	rproc_id_t rproc_id;
	uint32_t ipc_msg[IPC_MSG_LEN] = {0};
	uint32_t source = g_ipc_config[role].source;

	if (level >= INTSRC_NUM) {
		AUDIO_LOGE("invalid int level %d", level);
		return -1;
	}

	/* If communicating with hifi, level uses registers */
	if (dst_core == IPC_CORE_HiFi) {
		ipc_msg[0] = (source << 24) | (level << 8); /* fill first int */

		rproc_id = HISI_ACPU_HIFI_MBX_1;
		ret = RPROC_ASYNC_SEND(rproc_id, (mbox_msg_t *)ipc_msg,
			IPC_MSG_LEN);
		if (ret != 0) {
			AUDIO_LOGE("send error");
			return ret;
		}
	} else {
		AUDIO_LOGE("invalid dest: %d", dst_core);
		return -1;
	}

	g_ipc_debug.recv_int_core = dst_core;
	g_ipc_debug.int_send_times[level]++;

	return 0;
}

