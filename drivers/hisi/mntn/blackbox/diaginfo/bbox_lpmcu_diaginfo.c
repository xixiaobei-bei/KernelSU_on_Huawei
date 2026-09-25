/*
 * bbox_lpmcu_diaginfo.c
 *
 * process telemntn msg for bbox diaginfo.
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
#include <securec.h>
#include <linux/hisi/hisi_log.h>
#include "../rdr_print.h"

#define HISI_LOG_TAG MNTN_BBOX_DIAGINFO

static struct semaphore g_bbox_telemntn_sem;
static int g_bbox_lpmcu_diaginfo_init;
static DEFINE_SPINLOCK(lpmcu_ipc_lock);

/*
 * diaginfo IPC msg from LPMCU description
 * [diaginfo ID][data0][data1] ...
 */
#define MAX_IPC_NUM 5U
#define MAX_PARAM_NUM (MAX_MAIL_SIZE - 2U)

static struct lpmcu_diaginfo g_diaginfo_buf[MAX_IPC_NUM];

#define __LPMCU_DIAGINFO_MAP(x, y, z) { x, y, z },
static struct lpmcu_diaginfo_def g_lpmcu_diaginfo_map[] = {
	#include <bbox_diaginfo_lpmcu_msg_map.h>
};

static u32 get_lpmcu_diaginfo_map_size(void)
{
	return (unsigned int)ARRAY_SIZE(g_lpmcu_diaginfo_map);
}

/*
 * Description:  process msg IPC_BBOX_DIAGINFO from LPMCU
 * Input:        msg buffer
 */
#define TEMPBUFF_LEN 24
static int bbox_lpmcu_diaginfo_save(struct lpmcu_diaginfo *pmsg)
{
	int ret;
	unsigned int i, index, map_size;
	char *pdata = NULL;
	char temp[TEMPBUFF_LEN] = {0};

	if (!pmsg) {
		BB_PRINT_ERR("%s():%d invalid pmsg NULL\n", __func__, __LINE__);
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	pdata = kzalloc(MAX_DIAGINFO_LEN, GFP_ATOMIC);
	if (!pdata) {
		BB_PRINT_ERR("kzalloc failed in %s()\n", __func__);
		return BBOX_DIAGINFO_NO_MEM;
	}

	map_size = get_lpmcu_diaginfo_map_size();
	for (i = 0; i < map_size; i++) {
		if (g_lpmcu_diaginfo_map[i].err_id == pmsg->diaginfo[0])
			break;
	}

	if (i >= map_size) {
		BB_PRINT_ERR("undefined lpmcu msg[%d]\n", pmsg->diaginfo[0]);
		kfree(pdata);
		return BBOX_DIAGINFO_INVALIDPAR;
	}

	index = i;
	if (g_lpmcu_diaginfo_map[index].param_num > MAX_PARAM_NUM)
		pr_warn("lpmcu diaginfo ID[%d] parameters lost\n", pmsg->diaginfo[0]);

	ret = snprintf_s(pdata, MAX_DIAGINFO_LEN, MAX_DIAGINFO_LEN - 1, "%s", g_lpmcu_diaginfo_map[index].pmsg);
	if (ret < 0) {
		pr_debug("ID[%d] Msg is NULL\n", pmsg->diaginfo[0]);
		kfree(pdata);
		return BBOX_DIAGINFO_STR_MSG_ERR;
	}

	for (i = 1; i <= min(g_lpmcu_diaginfo_map[index].param_num, MAX_PARAM_NUM); i++) {
		ret = snprintf_s(temp, TEMPBUFF_LEN, TEMPBUFF_LEN - 1, ",0x%x", pmsg->diaginfo[i]);
		if (ret < 0) {
			BB_PRINT_ERR("%s():%d snprintf_s fail\n", __func__, __LINE__);
			kfree(pdata);
			return BBOX_DIAGINFO_STR_MSG_ERR;
		}
		ret = strncat_s(pdata, MAX_DIAGINFO_LEN, temp, strlen(pdata));
		if (ret != EOK) {
			pdata[MAX_DIAGINFO_LEN - 1] = '\0';
			pr_warn("lpmcu diaginfo ID[%d] msg error code: 0x%x\n", pmsg->diaginfo[0], ret);
			kfree(pdata);
			return BBOX_DIAGINFO_STR_MSG_ERR;
		}
	}

	ret = bbox_diaginfo_register(pmsg->diaginfo[0], NULL, pdata, strlen(pdata) + 1, pmsg->ts);
	kfree(pdata);

	return ret;
}

/*
 * Description:   process msg IPC_BBOX_DIAGINFO from LPMCU
 * Input:         IPC message
 */
void mntn_ipc_msg_nb(unsigned int *msg, u32 msg_len)
{
	int ret;
	unsigned int i;
	u64 ts;
	unsigned long flags;

	if (!msg) {
		BB_PRINT_ERR("%s():%d invalid msg NULL\n", __func__, __LINE__);
		return;
	}

	/* check if telemntn module is inited */
	if (!g_bbox_lpmcu_diaginfo_init)
		return;

	if (msg_len < (MAX_MAIL_SIZE - 1)) {
		BB_PRINT_ERR("[%s:%d]: invalid msg_len\n", __func__, __LINE__);
		return;
	}

	ts = hisi_getcurtime();

	/* Store msg into local buffer */
	spin_lock_irqsave(&lpmcu_ipc_lock, flags);
	for (i = 0; i < MAX_IPC_NUM; i++) {
		if (g_diaginfo_buf[i].ts == 0) {
			g_diaginfo_buf[i].ts = ts;
			ret = memcpy_s(&g_diaginfo_buf[i].diaginfo[0], (MAX_MAIL_SIZE - 1) * sizeof(int),
				&msg[1],  sizeof(int) * (MAX_MAIL_SIZE - 1));
			if (ret != EOK) {
				pr_debug("Memcpy ID[%u] Msg error\n", msg[1]);
				spin_unlock_irqrestore(&lpmcu_ipc_lock, flags);
				return;
			}
			break;
		}
	}

	spin_unlock_irqrestore(&lpmcu_ipc_lock, flags);

	/* buffer is full, ignore the msg */
	if (i >= MAX_IPC_NUM) {
		BB_PRINT_PN("LPMCU diaginfo ID: %u is ignored\n", msg[1]);
		return;
	}

	/* send sem to main thread */
	if (g_bbox_lpmcu_diaginfo_init)
		up(&g_bbox_telemntn_sem);
}

static int bbox_check_ipc_buffer(void)
{
	unsigned int i;
	unsigned long flags;
	struct lpmcu_diaginfo pmsg;
	int len;

	len = sizeof(pmsg);

repeat:
	/*
	 * check if ipc buffer has msg again
	 * protect ipc_msg_buf by spinlock
	 */
	spin_lock_irqsave(&lpmcu_ipc_lock, flags);
	for (i = 0; i < MAX_IPC_NUM; i++) {
		if (g_diaginfo_buf[i].ts != 0) {
			if (memcpy_s((void *)&pmsg, len,
				&g_diaginfo_buf[i], sizeof(pmsg)) != EOK) {
				BB_PRINT_ERR("Memcpy g_diaginfo_buf[%u] fail\n", i);
				spin_unlock_irqrestore(&lpmcu_ipc_lock, flags);
				return -1;
			}
			g_diaginfo_buf[i].ts = 0;
			break;
		}
	}

	spin_unlock_irqrestore(&lpmcu_ipc_lock, flags);

	if (i >= MAX_IPC_NUM)
		return -1;

	/* save lpmcu diaginfo into blackbox diaginfo list  */
	if (bbox_lpmcu_diaginfo_save(&pmsg) != 0)
		return -1;
	goto repeat;

	return 0;
}

/*
 * Description: the main thread, read diaginfo and save the diaginfo
 *              into bbox buffer.
 * Return:      0:success ; other: fail
 */
static int bbox_lpmcu_diaginfo_main_thread(void *arg)
{
	int ret;

	while (!kthread_should_stop()) {
		if (down_interruptible(&g_bbox_telemntn_sem)) {
			BB_PRINT_PN("%s(), down sem fail\n", __func__);
			continue;
		}
		ret = bbox_check_ipc_buffer();
		if (ret < 0)
			continue;
	}

	return 0;
}

/*
 * Description:  init dianinfo process; create main thread
 * Return:       0:success ; other: fail
 */
int bbox_lpmcu_diaginfo_init(void)
{
	struct task_struct *diaginfo_thread = NULL;

	sema_init(&g_bbox_telemntn_sem, 0);

	diaginfo_thread = kthread_run(bbox_lpmcu_diaginfo_main_thread, NULL, "bbox_lpmcu_diaginfo");
	if (!diaginfo_thread) {
		BB_PRINT_ERR("create thread bbox_lpmcu_diaginfo faild\n");
		return -1;
	}

	g_bbox_lpmcu_diaginfo_init = 1;
	return 0;
}
