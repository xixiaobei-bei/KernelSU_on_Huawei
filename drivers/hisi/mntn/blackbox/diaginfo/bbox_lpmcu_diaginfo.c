/*
 * process telemntn msg for bbox diaginfo.
 *
 * Copyright (c) 2018 Huawei Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/kthread.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/kthread.h>
#include <linux/hisi/util.h>
#include <bbox_diaginfo.h>
#include <bbox_diaginfo_id_def.h>
#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <libhwsecurec/securec.h>
#include <linux/hisi/hisi_log.h>
#include "../rdr_print.h"
#define HISI_LOG_TAG MNTN_BBOX_DIAGINFO

static struct semaphore bbox_telemntn_sem;
static int g_bbox_lpmcu_diaginfo_init = 0;
static DEFINE_SPINLOCK(lpmcu_ipc_lock);

/*
 * diaginfo IPC msg from LPMCU description
 * [diaginfo ID][data0][data1] ...
 */
#define MAX_IPC_NUM 		(5U)
#define MAX_PARAM_NUM 	(MAX_MAIL_SIZE - 2U)

static struct lpmcu_diaginfo diaginfo_buf[MAX_IPC_NUM];

#define __LPMCU_DIAGINFO_MAP(x,y,z) {x, y, z},
static struct lpmcu_diaginfo_def lpmcu_diaginfo_map[] = {
	#include <bbox_diaginfo_lpmcu_msg_map.h>
};

static u32 get_lpmcu_diaginfo_map_size(void)
{
	return (unsigned int)(sizeof(lpmcu_diaginfo_map) /
		sizeof(lpmcu_diaginfo_map[0]));
}

/************************************************************
Function:       bbox_lpmcu_diaginfo_save
Description:  process msg IPC_BBOX_DIAGINFO from LPMCU
Input:           msg buffer
Output:         N/A
Return:         N/A
************************************************************/
#define TEMPBUFF_LEN	(24)
int bbox_lpmcu_diaginfo_save(struct lpmcu_diaginfo *pmsg)
{
	int ret = 0;
	unsigned  int i, index, map_size;
	char* pdata = 0, temp[TEMPBUFF_LEN] = {0};

	pdata = kzalloc(MAX_DIAGINFO_LEN, GFP_ATOMIC);
	if (!pdata) {
		BB_PRINT_ERR("kzalloc failed in bbox_lpmcu_diaginfo_save\n");
		return BBOX_DIAGINFO_NO_MEM;
	}

	map_size = get_lpmcu_diaginfo_map_size();
	for (i =0; i < map_size; i ++) {
		if(lpmcu_diaginfo_map[i].err_id == pmsg->diaginfo[0]){
			break;
		}
	}

	if (i >= map_size) {
		BB_PRINT_ERR("undefined lpmcu msg[%d]\n", pmsg->diaginfo[0]);
		kfree(pdata);
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	index = i;
	if (lpmcu_diaginfo_map[index].param_num > MAX_PARAM_NUM) {
		pr_warning("lpmcu diaginfo ID[%d] parameters lost\n", pmsg->diaginfo[0]);
	}

	ret = snprintf_s(pdata, MAX_DIAGINFO_LEN, MAX_DIAGINFO_LEN-1, lpmcu_diaginfo_map[index].pmsg);
	if (!ret) {
		pr_debug("ID[%d] Msg is NULL\n", pmsg->diaginfo[0]);
	}

	for (i = 1;  i <= min(lpmcu_diaginfo_map[index].param_num, MAX_PARAM_NUM); i++){
		(void)snprintf_s(temp, TEMPBUFF_LEN, TEMPBUFF_LEN - 1, ",0x%x", pmsg->diaginfo[i]);
		ret = strncat_s(pdata, MAX_DIAGINFO_LEN, temp, strlen(pdata));
		if (EOK != ret) {
			pdata[MAX_DIAGINFO_LEN - 1] = '\0';
			pr_warning("lpmcu diaginfo ID[%d] msg error code: 0x%x\n", pmsg->diaginfo[0], ret);
			break;
		}
	}

	ret = bbox_diaginfo_register(pmsg->diaginfo[0], pdata, strlen(pdata) + 1, pmsg->ts);
	kfree(pdata);

	return ret;
}

/************************************************************
Function:       mntn_ipc_msg_nb
Description:   process msg IPC_BBOX_DIAGINFO from LPMCU
Input:           IPC message
Output:         N/A
Return:         N/A
************************************************************/
void mntn_ipc_msg_nb(unsigned int * msg)
{
	int ret;
	unsigned int i;
	u64 ts;
	unsigned long flags;

	/* check if telemntn module is inited */
	if(!g_bbox_lpmcu_diaginfo_init)
		return;

	ts = hisi_getcurtime();

	/* Store msg into local buffer */
	spin_lock_irqsave(&lpmcu_ipc_lock, flags);
	for (i = 0; i < MAX_IPC_NUM; i++) {
		if (0 == diaginfo_buf[i].ts) {
			diaginfo_buf[i].ts = ts;
			ret = memcpy_s(&diaginfo_buf[i].diaginfo[0], (MAX_MAIL_SIZE-1) * sizeof(int),
				&msg[1],  sizeof(int) * (MAX_MAIL_SIZE - 1));
			if (EOK != ret) {
				pr_debug("Memcpy ID[%d] Msg error\n", msg[1]);
			}
			break;
		}
	}
	spin_unlock_irqrestore(&lpmcu_ipc_lock, flags);

	/* buffer is full, ignore the msg */
	if (i >= MAX_IPC_NUM) {
		BB_PRINT_PN("LPMCU diaginfo ID( %d) is ignored \n", msg[1]);
		return ;
	}

	/* send sem to main thread */
	if(g_bbox_lpmcu_diaginfo_init)
		up(&bbox_telemntn_sem);
	return ;
}

/************************************************************
Function:      bbox_lpmcu_diaginfo_main_thread
Description:  the main thread, read diaginfo and save the diaginfo
		     into bbox buffer.
Input:           N/A
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
static int bbox_lpmcu_diaginfo_main_thread(void *arg)
{
	int len;
	unsigned int i;
	unsigned long flags;

	struct lpmcu_diaginfo pmsg;
	len = sizeof(struct lpmcu_diaginfo);

	while (!kthread_should_stop()) {
		if (down_interruptible(&bbox_telemntn_sem)) {
			BB_PRINT_PN("%s(), down sem fail\n", __func__);
			continue;
		}

repeat: 	/* check if ipc buffer has msg again */
		/* protect ipc_msg_buf by spinlock */
		spin_lock_irqsave(&lpmcu_ipc_lock, flags);
		for (i = 0; i < MAX_IPC_NUM; i++) {
			if (0 != diaginfo_buf[i].ts ) {
				(void)memcpy_s((void *)&pmsg, len, &diaginfo_buf[i], sizeof(struct lpmcu_diaginfo));
				diaginfo_buf[i].ts = 0;
				break;
			}
		}
		spin_unlock_irqrestore(&lpmcu_ipc_lock, flags);

		if (MAX_IPC_NUM <= i)
			continue;

		/* save lpmcu diaginfo into blackbox diaginfo list  */
		bbox_lpmcu_diaginfo_save(&pmsg);

		goto repeat;
	}

	return 0;
}

/************************************************************
Function:      bbox_lpmcu_diaginfo_init
Description:  init dianinfo process; create main thread
Input:           N/A
Output:         N/A
Return:         0:success ; other: fail
************************************************************/
int bbox_lpmcu_diaginfo_init(void)
{
	struct task_struct *diaginfo_thread = NULL;

	sema_init(&bbox_telemntn_sem, 0);

	diaginfo_thread = kthread_run(bbox_lpmcu_diaginfo_main_thread, NULL, "bbox_lpmcu_diaginfo");
	if (!diaginfo_thread) {
		BB_PRINT_ERR("create thread bbox_lpmcu_diaginfo faild.\n");
		return -1;
	}

	g_bbox_lpmcu_diaginfo_init = 1;
	return 0;
}
