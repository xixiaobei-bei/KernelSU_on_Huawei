/*
 * modem_route.c
 *
 * Code for ap mo connect
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "protocol.h"
#include <linux/device.h>
#include "contexthub_route.h"

#define MAX_RECV_MO_LEN 1023
static struct work_struct register_icc_read_work;
struct hw_ap_cp_route_t icc_route;

/* parse modem send to hub info */
static void process_modem_ap_prot(u32 channel_id, int len)
{
	int rc;
	unsigned int ret;
	char *modem_prot = NULL;
	pkt_header_t *mo_to_hub_data = NULL;
	struct modem_pkt_header_t *mo_info = NULL;

	if (((len + sizeof(pkt_header_t)) > MAX_RECV_MO_LEN) || (len <= 0) ||
		!icc_route.icc_read) {
		hwlog_err("process modem info para err, len = %d\n", len);
		return;
	}

	modem_prot = kmalloc(len + sizeof(pkt_header_t), GFP_ATOMIC);
	if (!modem_prot)
		return;

	ret = icc_route.icc_read(channel_id,
		(unsigned char *)modem_prot + sizeof(pkt_header_t), len);
	if (ret < len) {
		kfree(modem_prot);
		hwlog_info("%s size error = %d\n", __func__, len);
		return;
	}

	mo_info = (struct modem_pkt_header_t *)(modem_prot + sizeof(pkt_header_t));
	hwlog_info("%s get len = %d, mo_len = %d, tag = %d, cmd = %d\n",
		__func__, len, mo_info->length,
		mo_info->tag, mo_info->cmd);

	mo_to_hub_data = (pkt_header_t *)modem_prot;
	mo_to_hub_data->tag = TAG_MODEM;
	mo_to_hub_data->cmd = CMD_CMN_CONFIG_REQ;
	mo_to_hub_data->resp = NO_RESP;
	mo_to_hub_data->length = len;
	rc = send_modem_cmd_to_hub(modem_prot, len + sizeof(pkt_header_t));
	if (rc < 0)
		hwlog_err("%s, send modem info err\n", __func__);

	kfree(modem_prot);
}

static void register_icc_read_work_handler(struct work_struct *wk)
{
	hwlog_info("%s\n", __func__);

	if (!icc_route.icc_open) {
		hwlog_err("%s null ptr\n", __func__);
		return;
	}

	icc_route.icc_open(icc_route.channel_id, process_modem_ap_prot);
	hwlog_info("%s success!", __func__);
}

void connect_pcie_icc_sensorhub_route(struct hw_ap_cp_route_t *ap_cp_route)
{
	hwlog_info("%s\n", __func__);

	if (!ap_cp_route) {
		hwlog_err("%s null ptr\n", __func__);
		return;
	}
	memset(&icc_route, 0, sizeof(struct hw_ap_cp_route_t));
	memcpy(&icc_route, ap_cp_route, sizeof(struct hw_ap_cp_route_t));
	hwlog_info("%s, icc channel id = %d\n", __func__, icc_route.channel_id);
	INIT_WORK(&register_icc_read_work, register_icc_read_work_handler);
	schedule_work(&register_icc_read_work);
}
EXPORT_SYMBOL_GPL(connect_pcie_icc_sensorhub_route);

MODULE_AUTHOR("MOHub <hujianglei@huawei.com>");
MODULE_DESCRIPTION("MOHub driver");
MODULE_LICENSE("GPL");

