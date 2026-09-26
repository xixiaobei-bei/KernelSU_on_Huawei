/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2018-2020 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include "huawei_thp_log.h"
#include "huawei_thp.h"

static struct thp_log_save g_thplog_save = {0};
static char g_thplog_dir[SH_THP_PATH_MAXLEN] = SH_THP_LOG_DIR;
static char *g_raw_data_buff = NULL;
static char *g_dif_data_buff = NULL;

static void thp_wait_fs(const char *path)
{
	int fd;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS); /* lint !e501 */
	do {
		fd = sys_access(path, 0);
		if (fd) {
			msleep(THP_FS_WAIT_TIME);
			thp_log_info("%s wait ...\n", __func__);
		}
	} while (fd);
	set_fs(old_fs);
}

static void thp_log_to_file(const char *content, unsigned int len)
{
	int ret;
	static mm_segment_t oldfs;
	struct file *fp = NULL;
	char path[SH_THP_PATH_MAXLEN] = SH_THP_LOG_FILE;

	oldfs = get_fs();
	set_fs(KERNEL_DS); /* lint !e501 */
	if (sys_access(g_thplog_dir, 0)) {
		thp_log_err("[%s]failed to access dir:%s\n", __func__,
			g_thplog_dir);
		set_fs(oldfs);
		return;
	}
	fp = filp_open(path, O_CREAT | O_RDWR | O_APPEND,
		SH_THP_FILE_AUTHORITY);
	if (IS_ERR(fp)) {
		thp_log_err("[%s]filp_open failed:%s\n", __func__, path);
	} else {
		vfs_llseek(fp, 0L, SEEK_END);
		ret = vfs_write(fp, content, len, &(fp->f_pos));
		if (ret != len)
			thp_log_err("vfs_write %s error\n", path);
		filp_close(fp, NULL);
	}

	set_fs(oldfs);
}

static int thp_write_raw_data(void *buff)
{
	unsigned short i;
	unsigned short j;
	unsigned short num_col;
	unsigned short num_row;
	unsigned short offset = 0;
	unsigned short pos;
	unsigned short *raw_data = NULL;
	struct pkt_thplog_report_data_t *touchpanel_log_data =
		(struct pkt_thplog_report_data_t *)buff;

	raw_data = (unsigned short *)touchpanel_log_data->raw_data.data;
	num_col = touchpanel_log_data->raw_data.col;
	num_row = touchpanel_log_data->raw_data.row;

	if (num_col * num_row * sizeof(unsigned short) >
			THP_LOG_RAW_DATA_LENGTH_MAX) {
		thp_log_err("%s col or row is too large\n", __func__);
		return -1;
	}

	memset(g_raw_data_buff, 0, THP_LOG_RAW_DATA_LENGTH_MAX);
	offset += snprintf(g_raw_data_buff + offset,
		THP_LOG_RAW_DATA_LENGTH_MAX, "$$raw$");
	offset += snprintf(g_raw_data_buff + offset,
		THP_LOG_RAW_DATA_LENGTH_MAX, "%s\n",
		touchpanel_log_data->frame_head);
	for (i = 0; i < num_row; i++) {
		for (j = 0; j < num_col; j++) {
			pos = i * num_col + j;
			offset += snprintf(g_raw_data_buff + offset,
				THP_LOG_RAW_DATA_LENGTH_MAX, "%d\t",
				raw_data[pos]);
		}
		if (j == num_col)
			offset += snprintf(g_raw_data_buff + offset,
				THP_LOG_RAW_DATA_LENGTH_MAX, "\n");
	}
	if (offset > THP_LOG_AVAILABLE_DATA_LENGTH)
		thp_log_to_file(g_raw_data_buff, offset);
	return 0;
}

static int thp_write_dif_data(void *buff)
{
	unsigned short i;
	unsigned short j;
	unsigned short num_col;
	unsigned short num_row;
	unsigned short offset = 0;
	unsigned short pos;
	short *dif_data = NULL;
	struct pkt_thplog_report_data_t *touchpanel_log_data =
		(struct pkt_thplog_report_data_t *)buff;

	dif_data = (unsigned short *)touchpanel_log_data->dif_data.data;
	num_col = touchpanel_log_data->dif_data.col;
	num_row = touchpanel_log_data->dif_data.row;

	if (num_col * num_row * sizeof(short) >
			THP_LOG_DIF_DATA_LENGTH_MAX) {
		thp_log_err("%s col or row is too large\n", __func__);
		return -1;
	}

	memset(g_dif_data_buff, 0, THP_LOG_DIF_DATA_LENGTH_MAX);
	offset += snprintf(g_dif_data_buff + offset,
		THP_LOG_DIF_DATA_LENGTH_MAX, "$$dif$");
	offset += snprintf(g_dif_data_buff + offset,
		THP_LOG_DIF_DATA_LENGTH_MAX, "%s\n",
		touchpanel_log_data->frame_head);
	for (i = 0; i < num_row; i++) {
		for (j = 0; j < num_col; j++) {
			pos = i * num_col + j;
			offset += snprintf(g_dif_data_buff + offset,
				THP_LOG_DIF_DATA_LENGTH_MAX, "%4d\t",
				dif_data[pos]);
		}
		if (j == num_col)
			offset += snprintf(g_dif_data_buff + offset,
				THP_LOG_DIF_DATA_LENGTH_MAX, "\n");
	}
	if (offset > THP_LOG_AVAILABLE_DATA_LENGTH)
		thp_log_to_file(g_dif_data_buff, offset);
	return 0;
}

static void thp_write_logbuf2file(void *buff)
{
	unsigned short offset;
	char log_head[THP_LOG_AVAILABLE_DATA_LENGTH] = {0};
	struct pkt_thplog_report_data_t *touchpanel_log_data =
		(struct pkt_thplog_report_data_t *)buff;

	if (touchpanel_log_data == NULL) {
		thp_log_err("[%s], touchpanel_log_data is null\n", __func__);
		return;
	}

	/* dae */
	thp_log_to_file(touchpanel_log_data->dae_log,
		touchpanel_log_data->dae_logbuff_len);

	/* afe */
	thp_log_to_file(touchpanel_log_data->afe_log,
		touchpanel_log_data->afe_logbuff_len);

	/* raw */
	if (thp_write_raw_data((void *)touchpanel_log_data))
		thp_log_err("[%s]raw data format is wrong\n", __func__);

	/* diff */
	if (thp_write_dif_data((void *)touchpanel_log_data))
		thp_log_err("[%s]dif data format is wrong\n", __func__);

	/* tsa */
	offset = 0;
	if (touchpanel_log_data->tsa_logbuff_len >
		THP_LOG_AVAILABLE_DATA_LENGTH) {
		offset += snprintf(log_head, sizeof(log_head), "$$tsa$");
		thp_log_to_file(log_head, offset);
		thp_log_to_file(touchpanel_log_data->tsa_log,
			touchpanel_log_data->tsa_logbuff_len);
	}
}

static void thp_log_work_fn(struct work_struct *work)
{
	thp_log_info("%s save thp log start\n", __func__);
	thp_wait_fs("/data/lost+found");
	down(&g_thplog_save.sem);
	thp_write_logbuf2file((void *)(g_thplog_save.data));
	up(&g_thplog_save.sem);
}

static DECLARE_WORK(thp_log_work, thp_log_work_fn);
int thp_log_init(void)
{
	int ret = 0;

	thp_log_info("%s thp log is enabled\n", __func__);
	sema_init(&g_thplog_save.sem, 1);
	return ret;
}

int thp_log_save(const char *head)
{
	int ret = 0;
	struct pkt_thplog_report_data_t *touchpanel_log_data =
		(struct pkt_thplog_report_data_t *)(head);

	if (down_trylock(&g_thplog_save.sem)) {
		thp_log_err("[%s]work is busy\n", __func__);
		return -1;
	}
	if (g_thplog_save.data == NULL)
		g_thplog_save.data = (struct pkt_thplog_report_data_t *)
			kzalloc(sizeof(struct pkt_thplog_report_data_t),
				GFP_ATOMIC);
	if (g_raw_data_buff == NULL)
		g_raw_data_buff = (char *)
			kzalloc((size_t)THP_LOG_RAW_DATA_LENGTH_MAX,
				GFP_ATOMIC);
	if (g_dif_data_buff == NULL)
		g_dif_data_buff = (char *)
			kzalloc((size_t)THP_LOG_DIF_DATA_LENGTH_MAX,
				GFP_ATOMIC);
	if ((g_thplog_save.data == NULL) ||
		(g_raw_data_buff == NULL) ||
		(g_dif_data_buff == NULL)) {
		kfree(g_thplog_save.data);
		kfree(g_raw_data_buff);
		kfree(g_dif_data_buff);
		g_thplog_save.data = NULL;
		g_raw_data_buff = NULL;
		g_dif_data_buff = NULL;
		thp_log_err("[%s]buff malloc failed\n", __func__);
		up(&g_thplog_save.sem);
		return -1;
	}
	memcpy((void *)(g_thplog_save.data), (void *)touchpanel_log_data,
		sizeof(struct pkt_thplog_report_data_t));
	up(&g_thplog_save.sem);
	schedule_work(&thp_log_work);
	return ret;
}

