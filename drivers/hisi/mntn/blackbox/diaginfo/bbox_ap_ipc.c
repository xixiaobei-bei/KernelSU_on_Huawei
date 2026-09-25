/*
 * bbox_ap_ipc.c
 *
 * Bbox ipc for handling the ipc msg.
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
#include <linux/kthread.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/hisi/util.h>
#include <linux/hisi/ipc_msg.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#define HISI_LOG_TAG MNTN_BBOX_DIAGINFO
#include <linux/hisi/hisi_log.h>
#include "../rdr_print.h"

#define IPC_BBOX_DIAGINFO IPC_CMD(OBJ_LPM3, OBJ_MNTN_BBOX, CMD_NOTIFY, TYPE_MODE)

static struct notifier_block g_bbox_ipc_msg_nb;
static int g_bbox_ipc_init;

/*
 * Description:   IPC message notify function
 * Return:        0: OK; others: fail
 */
static int bbox_ipc_msg_notify(struct notifier_block *nb, unsigned long len, void *msg)
{
	unsigned int *_msg = (unsigned int *)msg;

	if (!nb || !msg) {
		BB_PRINT_ERR("%s():%d invalid pointer is NULL\n", __func__, __LINE__);
		return -1;
	}

	if (_msg[0] == IPC_BBOX_DIAGINFO) {
		/* Bbox diaginfo msg */
		mntn_ipc_msg_nb(_msg, MAX_MAIL_SIZE - 1);
		BB_PRINT_PN("receive ipc message\n");
	}

	return 0;
}

/*
 * Description:   init ipc module for Bbox IPC
 * Return:         0: OK; others: fail
 */
void bbox_ap_ipc_init(void)
{
	int err;

	/* make sure it's called only one time */
	if (g_bbox_ipc_init)
		return;

	g_bbox_ipc_msg_nb.next = NULL;
	g_bbox_ipc_msg_nb.notifier_call = bbox_ipc_msg_notify;
	err = RPROC_MONITOR_REGISTER(HISI_RPROC_LPM3_MBX0, &g_bbox_ipc_msg_nb);
	if (err) {
		BB_PRINT_ERR("Ipc register fail\n");
		return;
	}

	g_bbox_ipc_init = 1;
}
