/*
* Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
* foss@huawei.com
*
* If distributed as part of the Linux kernel, the following license terms
* apply:
*
* * This program is free software; you can redistribute it and/or modify
* * it under the terms of the GNU General Public License version 2 and
* * only version 2 as published by the Free Software Foundation.
* *
* * This program is distributed in the hope that it will be useful,
* * but WITHOUT ANY WARRANTY; without even the implied warranty of
* * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* * GNU General Public License for more details.
* *
* * You should have received a copy of the GNU General Public License
* * along with this program; if not, write to the Free Software
* * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
*
* Otherwise, the following license terms apply:
*
* * Redistribution and use in source and binary forms, with or without
* * modification, are permitted provided that the following conditions
* * are met:
* * 1) Redistributions of source code must retain the above copyright
* *    notice, this list of conditions and the following disclaimer.
* * 2) Redistributions in binary form must reproduce the above copyright
* *    notice, this list of conditions and the following disclaimer in the
* *    documentation and/or other materials provided with the distribution.
* * 3) Neither the name of Huawei nor the names of its contributors may
* *    be used to endorse or promote products derived from this software
* *    without specific prior written permission.
*
* * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/

/*****************************************************************************
 1. Other files included
*****************************************************************************/

#include <net/sock.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/wakelock.h>
#include <securec.h>
#include "product_config.h"
#include <linux/mtd/hisi_nve_interface.h>
#include "mdrv.h"
#include "diag_vcom_handler.h"
#include "diag_vcom_debug.h"





/*****************************************************************************
 2. Global defintions
*****************************************************************************/

static struct netlink_kernel_cfg diag_vcom_netlink_ctrl_config =
{
	.input = diag_vcom_ctrl_msg_handler,
};

static struct netlink_kernel_cfg diag_vcom_netlink_data_config =
{
	.input = diag_vcom_data_msg_handler,
};

static struct diag_vcom_entity_s diag_vcom_entitys[DIAG_VCOM_CHAN_MAX] = {
	{
		.channel	= DIAG_VCOM_CHAN_CTRL,
	},
	{
		.channel	= DIAG_VCOM_CHAN_DATA,
	},
};



/*****************************************************************************
 3. Function declarations
*****************************************************************************/

extern uint32_t PPM_LogPortSwitch(uint32_t ulPhyPort, uint32_t ulEffect);
extern uint32_t PPM_QueryLogPort(uint32_t *pulLogPort);


/*****************************************************************************
 4. Function defintions
*****************************************************************************/

/*****************************************************************************
 Prototype	  : diag_vcom_channel_init
 Description  : Register netlink protocol handler and open socket.
 Input		  : void
 Output 	  : None
 Return Value :
*****************************************************************************/
int diag_vcom_channel_init(void)
{
	struct sock *sk1, *sk2;
	struct diag_vcom_entity_s *entity;

	sk1 = netlink_kernel_create(&init_net, NETLINK_HW_LOGCAT_EXT,
							&diag_vcom_netlink_ctrl_config);
	if (!sk1) {
		DIAG_VCOM_LOGE("Cannot create diag ctrl socket");
		return -ENOMEM;
	}

	sk2 = netlink_kernel_create(&init_net, NETLINK_HW_LOGCAT,
							&diag_vcom_netlink_data_config);
	if (!sk2) {
		DIAG_VCOM_LOGE("Cannot to create diag data socket");
		netlink_kernel_release(sk1);
		return -ENOMEM;
	}

	entity = DIAG_VCOM_GET_ENTTIY(DIAG_VCOM_CHAN_CTRL);
	entity->sk = sk1;
	wake_lock_init(&entity->lock, WAKE_LOCK_SUSPEND, "diag_ctrl_wakelock");

	entity = DIAG_VCOM_GET_ENTTIY(DIAG_VCOM_CHAN_DATA);
	entity->sk = sk2;
	wake_lock_init(&entity->lock, WAKE_LOCK_SUSPEND, "diag_data_wakelock");

	return 0;
}

/*****************************************************************************
 Prototype	  : diag_vcom_channel_exit
 Description  : Release netlink resources.
 Input		  : void
 Output 	  : None
 Return Value :
*****************************************************************************/
void diag_vcom_channel_exit(void)
{
	struct diag_vcom_entity_s *entity;

	entity = DIAG_VCOM_GET_ENTTIY(DIAG_VCOM_CHAN_CTRL);
	wake_lock_destroy(&entity->lock);
	netlink_kernel_release(entity->sk);

	entity = DIAG_VCOM_GET_ENTTIY(DIAG_VCOM_CHAN_DATA);
	wake_lock_destroy(&entity->lock);
	netlink_kernel_release(entity->sk);
}

/*****************************************************************************
 Prototype	  : diag_vcom_register_ops
 Description  : Register channel operations.
 Input		  : uint32_t channel
				struct diag_vcom_cb_ops_s *ops
 Output 	  : None
 Return Value :
*****************************************************************************/
int diag_vcom_register_ops(uint32_t channel,
							struct diag_vcom_cb_ops_s *ops)
{
	struct diag_vcom_entity_s *entity;

	if (channel >= DIAG_VCOM_CHAN_MAX) {
		DIAG_VCOM_LOGE("Register channel[%d] ops failed", channel);
		return DIAG_VCOM_ERROR;
	}

	entity = DIAG_VCOM_GET_ENTTIY(channel);
	entity->cb_ops = *ops;
	entity->ready = true;

	DIAG_VCOM_LOGH("Register channel[%d] ops successfully", channel);
	return DIAG_VCOM_OK;
}

/*****************************************************************************
 Prototype	  : diag_vcom_send_resp_msg
 Description  : Send netlink response message.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
				uint32_t msg_len
 Output 	  : None
 Return Value :
*****************************************************************************/
static int diag_vcom_send_resp_msg(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg, uint32_t msg_len)
{
	struct sk_buff *resp_skb;
	struct nlmsghdr *resp_nlh;
	struct diag_vcom_msg_s *resp_msg;
	size_t payload;
	int ret;

	DIAG_VCOM_LOGI("pid=%d, msg_type=%d, op=%d, code[len]=%d",
		entity->resp_pid,
		msg->message_type,
		msg->op,
		msg->return_code);

	payload = NLMSG_ALIGN(msg_len);

	resp_skb = nlmsg_new(payload, GFP_KERNEL);
	if (!resp_skb) {
		DIAG_VCOM_LOGE("Alloc netlink message failed");
		entity->stats.alloc_errors++;
		return DIAG_VCOM_ERROR;
	}

	resp_nlh = nlmsg_put(resp_skb, 0, 0,
					NLMSG_DONE, (int)payload, 0);
	if (!resp_nlh) {
		DIAG_VCOM_LOGE("Put netlink message failed");
		kfree_skb(resp_skb);
		entity->stats.put_errors++;
		return DIAG_VCOM_ERROR;
	}

	resp_msg = (struct diag_vcom_msg_s *)nlmsg_data(resp_nlh);
	memcpy_s(resp_msg, msg_len, msg, msg_len);

	ret = nlmsg_unicast(entity->sk, resp_skb, entity->resp_pid);
	if (ret < 0) {
		DIAG_VCOM_LOGE("Send netlink message failed, pid=%d", entity->resp_pid);
		entity->stats.unicast_errors++;
		return DIAG_VCOM_ERROR;
	}

	entity->stats.unicast_message++;
	return DIAG_VCOM_OK;
}

/*****************************************************************************
 Prototype	  : diag_vcom_send_om_cmd_msg
 Description  : Send netlink om command response.
 Input		  : struct diag_vcom_entity_s *entity
				uint8_t *data
				uint32_t len
 Output 	  : None
 Return Value :
*****************************************************************************/
static int diag_vcom_send_om_cmd_msg(struct diag_vcom_entity_s *entity,
							uint8_t *data, uint32_t len)
{
	struct sk_buff *resp_skb;
	struct nlmsghdr *resp_nlh;
	struct diag_vcom_msg_s *resp_msg;
	size_t payload;
	int ret;

	DIAG_VCOM_LOGI("pid=%d, len=%d", entity->portid, len);

	payload = NLMSG_ALIGN(sizeof(struct diag_vcom_msg_s) + len);

	resp_skb = nlmsg_new(payload, GFP_KERNEL);
	if (!resp_skb) {
		DIAG_VCOM_LOGE("Alloc netlink message failed");
		entity->stats.alloc_errors++;
		return DIAG_VCOM_ERROR;
	}

	resp_nlh = nlmsg_put(resp_skb, 0, 0,
					NLMSG_DONE, (int)payload, 0);
	if (!resp_nlh) {
		DIAG_VCOM_LOGE("Put netlink message failed");
		kfree_skb(resp_skb);
		entity->stats.put_errors++;
		return DIAG_VCOM_ERROR;
	}

	resp_msg = (struct diag_vcom_msg_s *)nlmsg_data(resp_nlh);
	resp_msg->message_type = DIAG_VCOM_REQUEST_SEND_OM_COMMAND;
	resp_msg->op = DIAG_VCOM_MSG_RETURN_DATA;
	resp_msg->args_length = DIAG_VCOM_MSG_ARGS_SIZE(om_command) + len;

	resp_msg->om_command.length = len;
	memcpy_s(resp_msg->om_command.data, resp_msg->om_command.length, data, len);

	ret = nlmsg_unicast(entity->sk, resp_skb, entity->portid);
	if (ret < 0) {
		DIAG_VCOM_LOGE("Send netlink message failed, ret=%d", ret);
		entity->stats.unicast_errors++;
		return DIAG_VCOM_ERROR;
	}

	entity->stats.unicast_message++;
	return DIAG_VCOM_OK;
}

/*****************************************************************************
 Prototype	  : diag_vcom_send_om_data_msg
 Description  : Send netlink om data message.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_om_data_desc_s *desc
 Output 	  : None
 Return Value :
*****************************************************************************/
static int diag_vcom_send_om_data_msg(struct diag_vcom_entity_s *entity,
							struct diag_vcom_om_data_desc_s *desc)
{
	struct sk_buff *unsol_skb;
	struct nlmsghdr *unsol_nlh;
	struct diag_vcom_unsol_msg_s *unsol_msg;
	size_t payload;
	int ret;

	DIAG_VCOM_LOGD("pid=%d, len=%d, mode=%d", entity->portid, desc->len, desc->mode);

	payload = NLMSG_ALIGN(sizeof(struct diag_vcom_unsol_msg_s) + desc->len);

	unsol_skb = nlmsg_new(payload, GFP_KERNEL);
	if (!unsol_skb) {
		DIAG_VCOM_LOGE("Alloc netlink message failed");
		entity->stats.alloc_errors++;
		return DIAG_VCOM_ERROR;
	}

	unsol_nlh = nlmsg_put(unsol_skb, 0, 0,
					NLMSG_DONE, (int)payload, 0);
	if (!unsol_nlh) {
		DIAG_VCOM_LOGE("Put netlink message failed");
		kfree_skb(unsol_skb);
		entity->stats.put_errors++;
		return DIAG_VCOM_ERROR;
	}

	unsol_msg = (struct diag_vcom_unsol_msg_s *)nlmsg_data(unsol_nlh);
	unsol_msg->message_type = DIAG_VCOM_UNSOL_REPORT_OM_DATA;
	unsol_msg->op = DIAG_VCOM_MSG_RETURN_DATA;
	unsol_msg->args_length = DIAG_VCOM_UNSOL_MSG_ARGS_SIZE(om_data) + desc->len;

	unsol_msg->om_data.msg_seq = desc->msg_seq;
	unsol_msg->om_data.blk_num = desc->blk_num;
	unsol_msg->om_data.blk_seq = desc->blk_seq;
	unsol_msg->om_data.mode = desc->mode;
	unsol_msg->om_data.length = desc->len;
	memcpy_s(unsol_msg->om_data.data, unsol_msg->om_data.length, desc->data, desc->len);

	ret = netlink_unicast(entity->sk, unsol_skb, entity->portid, 0);
	if (ret < 0) {
		DIAG_VCOM_LOGD("Send netlink message failed, ret=%d", ret);
		entity->stats.unicast_errors++;
		return DIAG_VCOM_ERROR;
	}

	entity->stats.unicast_message++;
	return DIAG_VCOM_OK;
}

/*****************************************************************************
 Prototype	  : diag_vcom_fragment_om_data
 Description  : Fragment om data.
 Input		  : struct diag_vcom_entity_s *entity
				uint8_t *data
				uint32_t len
				uint8_t mode
 Output 	  : None
 Return Value :
*****************************************************************************/
static int diag_vcom_fragment_om_data(struct diag_vcom_entity_s *entity,
							uint8_t *data, uint32_t len, uint8_t mode)
{
	struct diag_vcom_om_data_desc_s om_data_desc;
	uint32_t blk_num;
	uint32_t tail_blk_size;
	uint32_t i;

	DIAG_VCOM_LOGD("len=%d, mode=%d", len, mode);

	blk_num = len / DIAG_VCOM_DATA_SIZE;
	tail_blk_size = len % DIAG_VCOM_DATA_SIZE;

	memset_s(&om_data_desc, sizeof(om_data_desc), 0, sizeof(om_data_desc));
	om_data_desc.blk_num = blk_num + ((tail_blk_size) ? 1 : 0);
	om_data_desc.mode = mode;
	om_data_desc.data = data;
	om_data_desc.len = DIAG_VCOM_DATA_SIZE;

	wake_lock(&entity->lock);

	for (i = 0; i < blk_num; i++) {
		diag_vcom_send_om_data_msg(entity, &om_data_desc);
		om_data_desc.data += DIAG_VCOM_DATA_SIZE;
		om_data_desc.blk_seq++;
	}

	if (tail_blk_size) {
		om_data_desc.len = tail_blk_size;
		diag_vcom_send_om_data_msg(entity, &om_data_desc);
	}

	wake_unlock(&entity->lock);
	return DIAG_VCOM_OK;
}

/*****************************************************************************
 Prototype	  : diag_vcom_report_om_data
 Description  : Report om data through netlink message.
 Input		  : uint32_t channel
				uint8_t *data
				uint32_t len
				uint8_t mode
 Output 	  : None
 Return Value :
*****************************************************************************/
int diag_vcom_report_om_data(uint32_t channel, uint8_t *data,
							uint32_t len, uint8_t mode)
{
	struct diag_vcom_entity_s *entity;

	DIAG_VCOM_LOGD("channel=%d, len=%d, mode=%d", channel, len, mode);

	if (unlikely(channel >= DIAG_VCOM_CHAN_MAX)) {
		DIAG_VCOM_LOGE("Channel[%d] is invalid", channel);
		return DIAG_VCOM_ERROR;
	}

	entity = DIAG_VCOM_GET_ENTTIY(channel);

	if (unlikely(!data || !len)) {
		DIAG_VCOM_LOGE("Data or len is invalid");
		entity->stats.rx_om_errors++;
		return DIAG_VCOM_ERROR;
	}

	if (!entity->portid) {
		DIAG_VCOM_LOGD("Channel[%d] is not opened", channel);
		entity->stats.rx_om_dropped++;
		return DIAG_VCOM_ERROR;
	}

	entity->stats.rx_om_packets++;

	if (channel == DIAG_VCOM_CHAN_CTRL)
		return diag_vcom_send_om_cmd_msg(entity, data, len);
	else
		return diag_vcom_fragment_om_data(entity, data, len, mode);
}

/*****************************************************************************
 Prototype	  : diag_vcom_associate_port
 Description  : Associate netlink channel.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_associate_port(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_entity_s *ent;
	struct diag_vcom_msg_s resp_msg;
	uint32_t i;

	DIAG_VCOM_LOGI("pid=%d,type=0x%x", msg->port.pid,msg->message_type);

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	if (!msg->port.pid) {
		resp_msg.return_code = DIAG_VCOM_E_INCORRECT_PARAMETERS;
	} else if (!entity->ready) {
		resp_msg.return_code = DIAG_VCOM_E_NOT_READY;
	} else {
		for (i = 0; i < DIAG_VCOM_CHAN_MAX; i++) {
			ent = DIAG_VCOM_GET_ENTTIY(i);
			ent->portid = msg->port.pid;

			if (ent->cb_ops.event_cb)
				ent->cb_ops.event_cb(ent->channel, DIAG_VCOM_EVT_CHAN_OPEN);
		}

		resp_msg.return_code = DIAG_VCOM_E_SUCCESS;
	}

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_unassociate_port
 Description  : Unassociate netlink channel.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_unassociate_port(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_entity_s *ent;
	struct diag_vcom_msg_s resp_msg;
	uint32_t i;

	DIAG_VCOM_LOGI("pid=%d.type=0x%x", msg->port.pid,msg->message_type);

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	for (i = 0; i < DIAG_VCOM_CHAN_MAX; i++) {
		ent = DIAG_VCOM_GET_ENTTIY(i);
		ent->portid = 0;

		if (ent->cb_ops.event_cb)
			ent->cb_ops.event_cb(ent->channel, DIAG_VCOM_EVT_CHAN_CLOSE);
	}

	resp_msg.return_code = DIAG_VCOM_E_SUCCESS;
	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_set_log_port_config
 Description  : Set log port.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_set_log_port_config(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	uint32_t log_port;

	DIAG_VCOM_LOGI("port=%d,type=0x%x", msg->log_port_config.port,msg->message_type);

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	log_port = (msg->log_port_config.port == DIAG_VCOM_LOG_PORT_USB) ?
					DIAG_VCOM_LOG_PORT_USB : DIAG_VCOM_LOG_PORT_VCOM;

	if (PPM_LogPortSwitch(log_port, true) != 0)
		resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
	else
		resp_msg.return_code = DIAG_VCOM_E_SUCCESS;

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_get_log_port_config
 Description  : Get current log port.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_get_log_port_config(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	uint32_t log_port = DIAG_VCOM_LOG_PORT_USB;

    DIAG_VCOM_LOGI("port=%d,type=0x%x", msg->log_port_config.port,msg->message_type);
    
	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	if (PPM_QueryLogPort(&log_port) != 0) {
		DIAG_VCOM_LOGE("Query log port failed");
		resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
	} else {
		resp_msg.op = DIAG_VCOM_MSG_RETURN_DATA;
		resp_msg.args_length = DIAG_VCOM_MSG_ARGS_SIZE(log_port_config);
		resp_msg.log_port_config.port = (log_port == DIAG_VCOM_LOG_PORT_USB) ?
							DIAG_VCOM_LOG_PORT_USB : DIAG_VCOM_LOG_PORT_VCOM;
	}

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_set_log_config
 Description  : Set log mode.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_set_log_config(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	uint32_t log_port = DIAG_VCOM_LOG_PORT_USB;
	SOCP_IND_MODE_ENUM mode;

	DIAG_VCOM_LOGH("mode=%d,type=0x%x", msg->log_config.mode,msg->message_type);

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;
	resp_msg.return_code = DIAG_VCOM_E_SUCCESS;

	if (PPM_QueryLogPort(&log_port) != 0) {
		DIAG_VCOM_LOGE("Query log port failed");
		resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
	} else {
		if (log_port == DIAG_VCOM_LOG_PORT_USB) {
			DIAG_VCOM_LOGE("Current log port is USB");
			resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
		} else {
			switch(msg->log_config.mode) {
			case DIAG_VCOM_LOG_MODE_DIRECT:
			case DIAG_VCOM_LOG_MODE_DELAY:
			case DIAG_VCOM_LOG_MODE_CYCLE:
				mode = (SOCP_IND_MODE_ENUM)msg->log_config.mode;
				if (mdrv_report_ind_mode_ajust(mode) != 0) {
					DIAG_VCOM_LOGE("Set ind mode failed");
					resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
				}
				break;

			default:
				resp_msg.return_code = DIAG_VCOM_E_INCORRECT_PARAMETERS;
				break;
			}
		}
	}

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_get_log_config
 Description  : Get log mode.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_get_log_config(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	uint32_t log_port = DIAG_VCOM_LOG_PORT_USB;
	uint32_t mode = 0;
	int ret;

    DIAG_VCOM_LOGI("mode=%d,type=0x%x", msg->log_config.mode,msg->message_type);
    
	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	if (PPM_QueryLogPort(&log_port) != 0) {
		DIAG_VCOM_LOGE("Query log port failed");
		resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
	} else {
		DIAG_VCOM_LOGH("Current log port=%d", log_port);

		if (log_port == DIAG_VCOM_LOG_PORT_USB) {
			ret = mdrv_socp_get_log_ind_mode(&mode);
		} else {
			DIAG_VCOM_LOGI("deflate feature off");
			ret = mdrv_socp_get_log_ind_mode(&mode);
		}

		if (ret) {
			DIAG_VCOM_LOGE("Get log ind mode failed");
			resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
		} else {
			resp_msg.op = DIAG_VCOM_MSG_RETURN_DATA;
			resp_msg.args_length = DIAG_VCOM_MSG_ARGS_SIZE(log_config);
			resp_msg.log_config.mode = mode;
		}
	}

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_set_log_nve_config
 Description  : Set log nve config.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_set_log_nve_config(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	struct hisi_nve_info_user nve_config;

	DIAG_VCOM_LOGI("NVE feature on, enable=%d", msg->log_nve_config.enable);

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	memset_s(&nve_config, sizeof(nve_config), 0, sizeof(nve_config));
	nve_config.nv_number = 285;
	nve_config.valid_size = 4;
	nve_config.nv_operation = NV_READ;

	if (hisi_nve_direct_access(&nve_config) != 0) {
		DIAG_VCOM_LOGE("Read NVE failed");
		resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
	} else {
		nve_config.nv_operation = NV_WRITE;
		nve_config.nv_data[0] = (msg->log_nve_config.enable == true) ? 0x31 : 0x30;

		if (hisi_nve_direct_access(&nve_config) != 0) {
			DIAG_VCOM_LOGE("Write NVE failed");
			resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
		} else {
			resp_msg.return_code = DIAG_VCOM_E_SUCCESS;
		}
	}

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}


/*****************************************************************************
 Prototype	  : diag_vcom_get_log_nve_config
 Description  : Get log nve config.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_get_log_nve_config(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	SOCP_ENC_DST_BUF_LOG_CFG_STRU log_cfg;

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	memset_s(&log_cfg, sizeof(log_cfg), 0, sizeof(log_cfg));

	if (mdrv_socp_get_sd_logcfg(&log_cfg) != 0) {
		resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
	} else {
		resp_msg.op = DIAG_VCOM_MSG_RETURN_DATA;
		resp_msg.args_length = DIAG_VCOM_MSG_ARGS_SIZE(log_nve_config);
		resp_msg.log_nve_config.enable = (log_cfg.logOnFlag == SOCP_DST_CHAN_DELAY) ?
											true : false;
	}

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_send_om_cmd_request
 Description  : Send om command to diag.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_send_om_cmd_request(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;
	uint32_t ret, real_len;

	DIAG_VCOM_LOGI("length=%d,type=0x%x", msg->om_command.length,msg->message_type);

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;

	real_len = msg->args_length - DIAG_VCOM_MSG_ARGS_SIZE(om_command);
	if (real_len < msg->om_command.length) {
		resp_msg.return_code = DIAG_VCOM_E_INCORRECT_PARAMETERS;
		diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
		return;
	}

	if (entity->cb_ops.data_rx_cb) {
		ret = entity->cb_ops.data_rx_cb(entity->channel,
							msg->om_command.data,
							msg->om_command.length);
		if (ret) {
			entity->stats.tx_om_errors++;
			resp_msg.return_code = DIAG_VCOM_E_GENERIC_FAILURE;
			diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
		} else {
			entity->stats.tx_om_packets++;
		}
	}
}

/*****************************************************************************
 Prototype	  : diag_vcom_process_unknown_msg
 Description  : Process unknown message.
 Input		  : struct diag_vcom_entity_s *entity
				struct diag_vcom_msg_s *msg
 Output 	  : None
 Return Value :
*****************************************************************************/
static void diag_vcom_process_unknown_msg(struct diag_vcom_entity_s *entity,
							struct diag_vcom_msg_s *msg)
{
	struct diag_vcom_msg_s resp_msg;

	DIAG_VCOM_LOGH("Unknow message");

	memset_s(&resp_msg, sizeof(resp_msg), 0, sizeof(resp_msg));
	resp_msg.message_type = msg->message_type;
	resp_msg.op = DIAG_VCOM_MSG_RETURN_CODE;
	resp_msg.return_code = DIAG_VCOM_E_UNKNOWN_MESSAGE;

	diag_vcom_send_resp_msg(entity, &resp_msg, sizeof(resp_msg));
}

/*****************************************************************************
 Prototype	  : diag_vcom_validate_msg
 Description  : Validate request message.
 Input		  : struct sk_buff *skb
 Output 	  : None
 Return Value :
*****************************************************************************/
static int diag_vcom_validate_msg(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	struct diag_vcom_msg_s *msg;
	uint32_t real_len;

	nlh = nlmsg_hdr(skb);
	if (!NLMSG_OK(nlh, skb->len)) {
		DIAG_VCOM_LOGE("Skb len %d is invalid", skb->len);
		return DIAG_VCOM_ERROR;
	}

	if (nlmsg_len(nlh) < (int)sizeof(struct diag_vcom_msg_s)) {
		DIAG_VCOM_LOGE("Netlink message len %d is invalid", nlh->nlmsg_len);
		return DIAG_VCOM_ERROR;
	}

	msg = (struct diag_vcom_msg_s *)nlmsg_data(nlh);
	if (msg->op != DIAG_VCOM_MSG_COMMAND) {
		DIAG_VCOM_LOGE("Message[%d] op %d is invalid",
			msg->message_type, msg->op);
		return DIAG_VCOM_ERROR;
	}

	real_len = (uint32_t)(nlmsg_len(nlh) - DIAG_VCOM_MSG_OFFSETOF(args_start));
	if (real_len < msg->args_length) {
		DIAG_VCOM_LOGE("Message[%d] args len %d is invalid",
			msg->message_type, msg->args_length);
		return DIAG_VCOM_ERROR;
	}

	return DIAG_VCOM_OK;
}

/*****************************************************************************
 Prototype	  : diag_vcom_ctrl_msg_handler
 Description  : Netlink message handle function.
 Input		  : struct sk_buff *skb
 Output 	  : None
 Return Value :
*****************************************************************************/
void diag_vcom_ctrl_msg_handler(struct sk_buff *skb)
{
	struct diag_vcom_entity_s *entity = DIAG_VCOM_GET_ENTTIY(DIAG_VCOM_CHAN_CTRL);
	struct nlmsghdr *nlh;
	struct diag_vcom_msg_s *msg;

	if (diag_vcom_validate_msg(skb) != DIAG_VCOM_OK) {
		DIAG_VCOM_LOGE("Message is invalid.");
		entity->stats.input_errors++;
		return;
	}

	entity->stats.input_message++;

	nlh = nlmsg_hdr(skb);
	msg = (struct diag_vcom_msg_s *)nlmsg_data(nlh);

	DIAG_VCOM_LOGI("Message[%d] op=%d, arg_len=%d, nlmsg_pid=%d, nlmsg_len=%d",
		msg->message_type,
		msg->op,
		msg->args_length,
		nlh->nlmsg_pid,
		nlh->nlmsg_len);

	entity->resp_pid = nlh->nlmsg_pid;

	switch (msg->message_type) {
	case DIAG_VCOM_REQUEST_ASSOCIATE_PORT:
		diag_vcom_associate_port(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_UNASSOCIATE_PORT:
		diag_vcom_unassociate_port(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_SET_LOG_PORT_CONFIG:
		diag_vcom_set_log_port_config(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_GET_LOG_PORT_CONFIG:
		diag_vcom_get_log_port_config(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_SET_LOG_CONFIG:
		diag_vcom_set_log_config(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_GET_LOG_CONFIG:
		diag_vcom_get_log_config(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_SET_LOG_NVE_CONFIG:
		diag_vcom_set_log_nve_config(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_GET_LOG_NVE_CONFIG:
		diag_vcom_get_log_nve_config(entity, msg);
		break;

	case DIAG_VCOM_REQUEST_SEND_OM_COMMAND:
		diag_vcom_send_om_cmd_request(entity, msg);
		break;

	default:
		diag_vcom_process_unknown_msg(entity, msg);
		break;
	}

	DIAG_VCOM_LOGH("Message[%d] is processed.", msg->message_type);
}

/*****************************************************************************
 Prototype	  : diag_vcom_data_msg_handler
 Description  : Netlink message handle function.
 Input		  : struct sk_buff *skb
 Output 	  : None
 Return Value :
*****************************************************************************/
void diag_vcom_data_msg_handler(struct sk_buff *skb)
{
	struct diag_vcom_entity_s *entity = DIAG_VCOM_GET_ENTTIY(DIAG_VCOM_CHAN_DATA);
	struct nlmsghdr *nlh;
	struct diag_vcom_msg_s *msg;

	if (diag_vcom_validate_msg(skb) != DIAG_VCOM_OK) {
		DIAG_VCOM_LOGE("Drop invalid message");
		entity->stats.input_errors++;
		return;
	}

	entity->stats.input_message++;

	nlh = nlmsg_hdr(skb);
	msg = (struct diag_vcom_msg_s *)nlmsg_data(nlh);

	DIAG_VCOM_LOGI("Message: pid=%d, len=%d, msg_type=%d, op=%d, arg_len=%d",
		nlh->nlmsg_pid,
		nlh->nlmsg_len,
		msg->message_type,
		msg->op,
		msg->args_length);

	entity->resp_pid = nlh->nlmsg_pid;

	diag_vcom_process_unknown_msg(entity, msg);

	DIAG_VCOM_LOGH("Message[%d] is processed.", msg->message_type);
}

/*****************************************************************************
 Prototype	  : diag_vcom_show_stats
 Description  : Show diag vcom statistics.
 Input		  : void
 Output 	  : None
 Return Value :
*****************************************************************************/
void diag_vcom_show_stats(void)
{
	struct diag_vcom_stats_s *stats;
	uint32_t i;

	for (i = 0; i < DIAG_VCOM_CHAN_MAX; i++) {
		stats = &diag_vcom_entitys[i].stats;
		pr_err("[CH:%d] input_message:             %10u\n", i, stats->input_message);
		pr_err("[CH:%d] input_errors:              %10u\n", i, stats->input_errors);
		pr_err("[CH:%d] tx_om_packets:             %10u\n", i, stats->tx_om_packets);
		pr_err("[CH:%d] tx_om_errors:              %10u\n", i, stats->tx_om_errors);
		pr_err("[CH:%d] rx_om_packets:             %10u\n", i, stats->rx_om_packets);
		pr_err("[CH:%d] rx_om_errors:              %10u\n", i, stats->rx_om_errors);
		pr_err("[CH:%d] rx_om_dropped:             %10u\n", i, stats->rx_om_dropped);
		pr_err("[CH:%d] alloc_errors:              %10u\n", i, stats->alloc_errors);
		pr_err("[CH:%d] put_errors:                %10u\n", i, stats->put_errors);
		pr_err("[CH:%d] unicast_errors:            %10u\n", i, stats->unicast_errors);
		pr_err("[CH:%d] unicast_message:           %10u\n", i, stats->unicast_message);
	}
}
EXPORT_SYMBOL(diag_vcom_show_stats);