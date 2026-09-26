/**
 * @file cmdlist_priv.h
 * @brief Implementing interface function for cmdlist node
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DPU_CMDLIST_H__
#define __DPU_CMDLIST_H__

#include "chash.h"
#include "dkmd_cmdlist.h"

struct cmdlist_table {
	int scene_num;
	int current_scene;
	struct clist_head *flush_list;
	struct chash_table ht;
};

struct cmdlist_client {
	struct cnode list_node;
	struct cnode hash_node;
	int scene_id;
	int key;
	struct cmdlist_node *value;

	int already_appended;
};

/**
 * @brief init cmdlist device and cmdlist table
 *
 * @param scene_num
 * @return int
 */
int cmdlist_dev_open(int scene_num);

/**
 * @brief release cmdlist table and list
 *
 * @return int 0: success -1: fail
 */
int cmdlist_dev_close(void);

/**
 * @brief create cmdlist node by type, node_size and dst_addr
 *
 * @param user_client use this client type, node_size and dst_addr
 * @return int 0: success -1: fail
 */
int cmdlist_create_client(struct cmdlist_node_client *user_client);

/**
 * @brief append cmdlist node which recorded by user_client with append_next_id to this scene cmdlist node
 *
 * @param user_client use this client id and append_next_id
 * @return int 0: success -1: fail
 */
int cmdlist_append_next_client(struct cmdlist_node_client *user_client);

/**
 * @brief link this scene cmdlist node and link_next_id cmdlist node recorded by user_client
 *
 * @param user_client use this client id and link_next_id
 * @return int 0: success -1: fail
 */
int cmdlist_link_next_client(struct cmdlist_node_client *user_client);

/**
 * @brief signal this current scene cmdlist node
 *
 * @param user_client use this client id
 * @return int 0: success -1: fail
 */
int cmdlist_signal_client(struct cmdlist_node_client *user_client);

/**
 * @brief Dump this current scene cmdlist node
 *
 * @param user_client use this client id
 * @return int 0: success -1: fail
 */
int cmdlist_dump_user_client(struct cmdlist_node_client *user_client);

/**
 * @brief Dump all the same scene cmdlist nodes
 *
 * @param user_client use the scene_id
 * @return int 0: success -1: fail
 */
int cmdlist_dump_scene_client(struct cmdlist_node_client *user_client);

/**
 * @brief  Dump all cmdlist nodes
 *
 */
void cmdlist_dump_all_client(void);

#endif