/*
 * ext_sensorhub_api.c
 *
 * code for external sensorhub channel kernel api
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "ext_sensorhub_api.h"
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <huawei_platform/log/hw_log.h>

#include "ext_sensorhub_route.h"
#include "ext_sensorhub_frame.h"

#define HWLOG_TAG ext_sensorhub_api
HWLOG_REGIST();

int send_command(enum ext_channel_id channel_id,
		 struct command *cmd, bool need_resp, struct cmd_resp *resp)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	if (!cmd || cmd->send_buffer_len < 0 ||
	    (cmd->send_buffer_len > 0 && !cmd->send_buffer))
		return -EINVAL;

	if (need_resp && (!resp || resp->receive_buffer_len <= 0 ||
			  !resp->receive_buffer))
		return -EINVAL;

	return ext_sensorhub_route_write_kernel(channel_id, cmd, need_resp,
		resp);
}

int register_data_callback(enum ext_channel_id channel_id,
			   struct subscribe_cmds *sub_cmds,
			   data_callback callback)
{
	int ret;
	int i;
	unsigned char *sids = NULL;
	unsigned char *cids = NULL;

	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	if (!sub_cmds || !sub_cmds->cmds || !callback)
		return -EINVAL;

	/* kfree in ext sensorhub route */
	sids = kmalloc(sub_cmds->cmd_cnt, GFP_KERNEL);
	cids = kmalloc(sub_cmds->cmd_cnt, GFP_KERNEL);
	if (!sids || !cids)
		return -ENOMEM;

	for (i = 0; i < sub_cmds->cmd_cnt; ++i) {
		sids[i] = sub_cmds->cmds[i].service_id;
		cids[i] = sub_cmds->cmds[i].command_id;
	}

	ret = ext_sensorhub_route_reg_cb(channel_id, sids,
					 cids, sub_cmds->cmd_cnt, callback);
	return ret;
}

int unregister_data_callback(enum ext_channel_id channel_id)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	return ext_sensorhub_route_unreg_cb(channel_id);
}

int subscribe(enum ext_channel_id channel_id, struct subscribe_cmds *sub_cmds)
{
	int ret;
	int i;
	unsigned char *sids = NULL;
	unsigned char *cids = NULL;

	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	if (sub_cmds && sub_cmds->cmd_cnt == 0)
		return ext_sensorhub_route_unreg_cb(channel_id);
	if (!sub_cmds || sub_cmds->cmd_cnt < 0 || !sub_cmds->cmds)
		return -EINVAL;
	if (sub_cmds->cmd_cnt > MAX_SUBSCMDS_COUNT) {
		hwlog_err("cmd count[%d] exceeds", sub_cmds->cmd_cnt);
		return -EINVAL;
	}

	sids = kmalloc(sub_cmds->cmd_cnt, GFP_KERNEL);
	cids = kmalloc(sub_cmds->cmd_cnt, GFP_KERNEL);
	if (!sids || !cids)
		return -ENOMEM;

	for (i = 0; i < sub_cmds->cmd_cnt; ++i) {
		sids[i] = sub_cmds->cmds[i].service_id;
		cids[i] = sub_cmds->cmds[i].command_id;
	}

	ret = ext_sensorhub_route_reg_cb(channel_id, sids, cids,
					 sub_cmds->cmd_cnt, NULL);
	return ret;
}

int read_data(enum ext_channel_id channel_id, struct received_data *data)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;
	if (!data || data->receive_buffer_len <= 0 ||
	    !data->receive_buffer)
		return -EINVAL;

	return ext_sensorhub_route_read_kernel(channel_id, data->receive_buffer,
		data->receive_buffer_len, &data->service_id, &data->command_id);
}

bool is_ext_sensorhub_disabled(void)
{
	int len = 0;
	struct device_node *status_node = NULL;
	const char *status = NULL;
	static int ret;
	static int once;

	if (once)
		return ret;

	status_node = of_find_compatible_node(NULL, NULL,
					      "huawei,ext_sensorhub_status");
	if (!status_node) {
		hwlog_err("%s, can not find node ext_sensorhub_status node",
			  __func__);
		return -1;
	}

	status = of_get_property(status_node, "status", &len);
	if (!status) {
		hwlog_err("%s, can't find property status\n", __func__);
		return -1;
	}

	if (strstr(status, "ok")) {
		hwlog_info("%s, ext_sensorhub enabled!\n", __func__);
		ret = 0;
	} else {
		hwlog_info("%s, ext_sensorhub disabled!\n", __func__);
		ret = -1;
	}
	once = 1;
	return ret;
}

int query_max_send_len(void)
{
	return max_send_data_len();
}

int register_event_callback(enum ext_sensorhub_event event, unsigned char tid,
			    event_callback callback)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;

	if (event <= EXT_SENSORHUB_MIN_EVENT ||
	    event >= EXT_SENSORHUB_MAX_EVENT || !callback)
		return -EINVAL;

	return ext_sensorhub_event_reg_cb(event, tid, callback);
}

int unregister_event_callback(enum ext_sensorhub_event event, unsigned char tid,
			      event_callback callback)
{
	if (is_ext_sensorhub_disabled())
		return -EINVAL;

	if (event <= EXT_SENSORHUB_MIN_EVENT ||
	    event >= EXT_SENSORHUB_MAX_EVENT || !callback)
		return -EINVAL;

	return ext_sensorhub_event_unreg_cb(event, tid, callback);
}

int notify_close_bt_uart(void)
{
	int ret;

	ret = ext_sensorhub_frame_close_uart();
	pr_info("%s try to close uart, ret:%d", __func__, ret);

	return ret;
}
