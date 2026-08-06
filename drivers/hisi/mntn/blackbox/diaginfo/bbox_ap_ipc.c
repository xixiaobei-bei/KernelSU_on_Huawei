/*
 * Bbox ipc for handling the ipc msg.
 *
 * Copyright (c) 2018 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

static struct notifier_block bbox_ipc_msg_nb;
static int g_bbox_ipc_init = 0;

#define IPC_BBOX_DIAGINFO IPC_CMD(OBJ_LPM3, OBJ_MNTN_BBOX, CMD_NOTIFY, TYPE_MODE)

/************************************************************
Function:       bbox_ipc_msg_notify
Description:   IPC message notify function
Input:           N/A
Output:         N/A
Return:         0: OK; others: fail
************************************************************/
static int bbox_ipc_msg_notify(struct notifier_block *nb, unsigned long len, void *msg)
{
	unsigned int *_msg = (unsigned int *)msg;

	if (IPC_BBOX_DIAGINFO == _msg[0]) {
		/* Bbox diaginfo msg */
		mntn_ipc_msg_nb(_msg);
		BB_PRINT_PN("receive ipc message \n");
	}

	return 0;
}

/************************************************************
Function:       bbox_ap_ipc_init
Description:   init ipc module for Bbox IPC
Input:           N/A
Output:         N/A
Return:         0: OK; others: fail
************************************************************/
void bbox_ap_ipc_init(void)
{
	int err;

	/*make sure it's called only one time */
	if (g_bbox_ipc_init)
		return;

	bbox_ipc_msg_nb.next = NULL;
	bbox_ipc_msg_nb.notifier_call = bbox_ipc_msg_notify;
	err = RPROC_MONITOR_REGISTER(HISI_RPROC_LPM3_MBX0, &bbox_ipc_msg_nb);
	if (err) {
		BB_PRINT_ERR("Ipc register fail \n");
		return;
	}

	g_bbox_ipc_init = 1;
}
