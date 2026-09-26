/*
 * Watch TP Stability Recovery
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include "watch_tp_recovery.h"
#include "huawei_ts_kit_api.h"
#include "securec.h"

#define TS_DETECT_QUEUE_SIZE 10
#define TS_RECOVERY_TIMEOUT 5000

static struct ts_recovery_data g_ts_recovery_data;
static void ts_recovery_work(struct work_struct *work);

int get_one_detect_cmd(struct ts_cmd_node *cmd)
{
	unsigned long flags = 0;
	int error;
	struct ts_cmd_queue *q = NULL;

	TS_LOG_INFO("%s begin\n", __func__);
	if (unlikely(!cmd)) {
		error = -EIO;
		TS_LOG_ERR("find null pointer\n");
		goto out;
	}

	q = &g_ts_recovery_data.queue;
	spin_lock_irqsave(&q->spin_lock, flags);
	/* memory barrier protect queue */
	smp_wmb();
	if (!q->cmd_count) {
		TS_LOG_DEBUG("queue is empty\n");
		spin_unlock_irqrestore(&q->spin_lock, flags);
		error = -EIO;
		goto out;
	}
	error = memcpy_s(cmd, sizeof(*cmd), &q->ring_buff[q->rd_index], sizeof(*cmd));
	if (error != EOK) {
		TS_LOG_ERR("%s memcpy cmd data fail\n", __func__);
		return -ENOMEM;
	}
	q->cmd_count--;
	q->rd_index++;
	q->rd_index %= q->queue_size;

	TS_LOG_DEBUG("%s rd_index=%d\n", __func__, q->rd_index);
	/* memory barrier protect flags */
	smp_mb();
	spin_unlock_irqrestore(&q->spin_lock, flags);
	TS_LOG_DEBUG("%s:%d from ring buff\n", __func__, cmd->command);
	error = NO_ERR;
	TS_LOG_INFO("%s end\n", __func__);
out:
	return error;
}

int put_one_detect_cmd(struct ts_cmd_node *cmd)
{
	int error;
	int ret;
	unsigned long flags = 0;
	struct ts_cmd_queue *q = NULL;
	struct ts_recovery_data *data = &g_ts_recovery_data;

	TS_LOG_INFO("%s begin\n", __func__);
	if (!cmd) {
		error = -EIO;
		TS_LOG_ERR("find null pointer\n");
		return error;
	}

	if (cmd->command != TS_CHIP_DETECT) {
		error = -EIO;
		TS_LOG_ERR("not detect command\n");
		return error;
	}
	q = &data->queue;

	spin_lock_irqsave(&q->spin_lock, flags);
	/* memory barrier protect queue */
	smp_wmb();
	if (q->cmd_count == q->queue_size) {
		spin_unlock_irqrestore(&q->spin_lock, flags);
		TS_LOG_ERR("queue is full\n");
		WARN_ON(1);
		error = -EIO;
		return error;
	}
	error = memcpy_s(&q->ring_buff[q->wr_index], sizeof(*cmd), cmd, sizeof(*cmd));
	if (error != EOK) {
		TS_LOG_ERR("%s memcpy cmd data fail\n", __func__);
		return -ENOMEM;
	}
	q->cmd_count++;
	q->wr_index++;
	q->wr_index %= q->queue_size;
	TS_LOG_DEBUG("%s wr_index=%d\n", __func__, q->wr_index);

	/* memory barrier protect flags */
	smp_mb();
	spin_unlock_irqrestore(&q->spin_lock, flags);
	TS_LOG_DEBUG("put one cmd :%d in ring buff\n", cmd->command);
	error = NO_ERR;
	/* memory barrier protect queue */
	smp_wmb();
	TS_LOG_INFO("%s end\n", __func__);
	return error;
}

static void ts_recovery_timer(unsigned long data)
{
	struct ts_recovery_data *cd = (struct ts_recovery_data *)data;
	TS_LOG_DEBUG("Timer triggered\n");
	if (!cd)
		return;
	if (!work_pending(&cd->recovery_work))
		schedule_work(&cd->recovery_work);
}

void ts_recovery_init(void)
{
	struct ts_recovery_data *data = &g_ts_recovery_data;

	TS_LOG_INFO("ts recovery init\n");
	data->queue.queue_size = TS_DETECT_QUEUE_SIZE;
	data->queue.rd_index = 0;
	data->queue.wr_index = 0;
	data->queue.cmd_count = 0;
	spin_lock_init(&data->queue.spin_lock);
	INIT_WORK(&(data->recovery_work), ts_recovery_work);
	setup_timer(&(data->recovery_timer), ts_recovery_timer,
		(unsigned long)(&g_ts_recovery_data));
	mod_timer(&data->recovery_timer, jiffies +
		msecs_to_jiffies(TS_RECOVERY_TIMEOUT));
	TS_LOG_INFO("%s end\n", __func__);
}

void release_memory()
{
	struct ts_cmd_node cmd;

	while (get_one_detect_cmd(&cmd) == NO_ERR) {
		if (cmd.cmd_param.pub_params.chip_data->ops == NULL)
			continue;
		if (cmd.cmd_param.pub_params.chip_data->ops->chip_clear_memory)
			cmd.cmd_param.pub_params.chip_data->ops->chip_clear_memory();
	}
}

static void ts_recovery_work(struct work_struct *work)
{
	int error;
	struct ts_cmd_node cmd;
	struct ts_cmd_node out_cmd;
	int cmd_num;

	TS_LOG_INFO("ts recovery work\n");
	cmd_num = g_ts_recovery_data.queue.cmd_count;

	while (cmd_num > 0) {
		get_one_detect_cmd(&cmd);
		cmd_num--;
		error = ts_chip_detect(&cmd, &out_cmd);
		if (!error) {
			ts_stop_recovery_timer();
			release_memory();
			return;
		} else {
			put_one_detect_cmd(&cmd);
		}
	}
	mod_timer(&g_ts_recovery_data.recovery_timer, jiffies +
		msecs_to_jiffies(TS_RECOVERY_TIMEOUT));
}

void ts_stop_recovery_timer()
{
	TS_LOG_INFO("stop recovery timer\n");
	del_timer(&g_ts_recovery_data.recovery_timer);
	cancel_work_sync(&g_ts_recovery_data.recovery_work);
}

