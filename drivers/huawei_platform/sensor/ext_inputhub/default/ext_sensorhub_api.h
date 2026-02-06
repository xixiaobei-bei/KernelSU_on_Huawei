/*
 * ext_sensorhub_api.h
 *
 * head file for external sensorhub channel kernel api
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

#ifndef EXT_SENSORHUB_API_H
#define EXT_SENSORHUB_API_H
#include "ext_sensorhub_frame.h"

/* send buffer cannot exceed this */
#define SEND_BUFFER_MAX_LEN MAX_SEND_BUFFER_LEN

enum ext_channel_id {
	SENSOR_CHANNEL = 0x01,
	EXT_SENSORHUB_CHANNEL = 0x02,
	FLP_CAHNNEL = 0x03,
	BT_CHANNEL = 0x04,
	VIBRATOR_CHANNEL = 0x05,
	UPGRADE_CHANNEL = 0x06,
	CHARGE_CHANNEL = 0x07,
	DEBUG_CHANNEL  = 0x08,
	MNTN_CHANNEL = 0x09,
	AUDIO_CHANNEL = 0x0a,
	NFC_CHANNEL = 0x0b,
	AT_CHANNEL = 0x0c,
	AR_CHANNEL = 0x0d,
	BT_A_CAHNNEL = 0x0e,
	MOTION_CHANNEL = 0x0f,
};

struct command {
	unsigned char service_id;
	unsigned char command_id;
	unsigned char *send_buffer;
	int send_buffer_len;
};

struct cmd_resp {
	unsigned char *receive_buffer;
	int receive_buffer_len;
};

struct sid_cid {
	unsigned char service_id;
	unsigned char command_id;
};

struct subscribe_cmds {
	struct sid_cid *cmds;
	int cmd_cnt;
};

typedef int (*data_callback)(unsigned char service_id,
	unsigned char command_id, unsigned char *data, int data_len);

struct received_data {
	unsigned char service_id;
	unsigned char command_id;
	unsigned char *receive_buffer;
	int receive_buffer_len;
};

enum ext_sensorhub_event {
	EXT_SENSORHUB_MIN_EVENT = 0x00,

	COMMU_HANDSHAKE = 0x01,

	EXT_SENSORHUB_MAX_EVENT,
};

/* Callback data maybe null. */
typedef int (*event_callback)(enum ext_sensorhub_event event, unsigned char tid,
	unsigned char *data, int data_len);

/*
 * register event callback for certain event and tid.
 * @param event The event concerned.
 * @param tid The event for which trans id. Now 0x20 for MCU, 0x30 for BT core.
 * @param callback The event callback.
 * @return ret >= 0 on success, < 0 on failure.
 */
int register_event_callback(enum ext_sensorhub_event event, unsigned char tid,
			    event_callback callback);

/*
 * unregister event callback for certain event and tid.
 * @param event The event concerned.
 * @param tid The event for which trans id. 0x20 for MCU, 0x30 for BT core.
 * @param callback The event callback.
 * @return ret >= 0 on success, < 0 on failure.
 */
int unregister_event_callback(enum ext_sensorhub_event event, unsigned char tid,
			      event_callback callback);

/*
 * send a command with data
 * @param channel_id The channel id the command send to.
 * @param cmd command to send
 * @param resp the command's resp. if need_resp is false resp can be null.
 * @param need_resp if the command need resp from ext sensorhub channel
 * @return ret >= 0 on success, the bytes length of resp, -1 on failure
 * writing to channel, -2 waitting resp timeout;
 */
int send_command(enum ext_channel_id channel_id,
		 struct command *cmd, bool need_resp,
		 struct cmd_resp *resp);

/*
 * regiter data callback for a channel
 * @param channel_id The channel id.
 * @param sub_cmds When get these cmds, do callback.
 * One channel can only record last subscribe_cmds.
 * Shared with subscribe func.
 * After registered sub_cmds, read_data func also be affected.
 * @param callback The data callback.
 * @return ret >= 0 on success, < 0 on failure.
 */
int register_data_callback(enum ext_channel_id channel_id,
			   struct subscribe_cmds *sub_cmds,
			   data_callback callback);

/*
 * unregister data callback and subscribed cmds from the channel.
 * @param channel_id The channel id.
 * @return ret >= 0 on success, < 0 on failure.
 */
int unregister_data_callback(enum ext_channel_id channel_id);

/*
 * subscribe commands for a channel id
 * @param channel_id The channel id.
 * @param sub_cmds When get these cmds, read_data will get the data.
 * One channel can only record last subscribe_cmds.
 * Shared with callback func.
 * After subscribe sub_cmds, callback func also be affected.
 * @return ret >= 0 on success, < 0 on failure.
 */
int subscribe(enum ext_channel_id channel_id,
	      struct subscribe_cmds *sub_cmds);

/*
 * read data from an ext sensorhub channel
 * @param channel_id The channel id the command send to.
 * @param data The data read from the channel.
 * @return ret >= 0 on success, the read bytes length,
 * < 0 on failure reading from channel;
 */
int read_data(enum ext_channel_id channel_id,
	      struct received_data *data);

/*
 * query max data len once send
 */
int query_max_send_len(void);

/*
 * check external sensorhub is disabled
 */
bool is_ext_sensorhub_disabled(void);

int notify_close_bt_uart(void);

#endif /* EXT_SENSORHUB_API_H */
