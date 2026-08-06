/**
 * @file disp_cmdlist.c
 * @brief The ioctl the interface file for cmdlist node.
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
#include <linux/module.h>
#include <linux/syscalls.h>
#include "cmdlist_priv.h"
#include "cmdlist_node.h"
#include "dkmd_acquire_fence.h"

int g_cmdlist_next_id = 0;
static struct cmdlist_table *cmd_table = NULL;

static bool is_same_cmdlist_client(const struct cnode *a, const struct cnode *b)
{
	struct cmdlist_client *na;
	struct cmdlist_client *nb;

	na = cnode_entry(a, struct cmdlist_client, hash_node);
	nb = cnode_entry(b, struct cmdlist_client, hash_node);

	return na->key == nb->key;
}

static size_t get_cmdlist_scene_id(const struct cnode *node, size_t bkt_size)
{
	struct cmdlist_client *client = cnode_entry(node, struct cmdlist_client, hash_node);

	if (bkt_size != 0)
		client->scene_id = client->scene_id % bkt_size;

	return client->scene_id;
}

int cmdlist_dev_open(int scene_num)
{
	int i;

	if (cmd_table != NULL) {
		pr_info("cmd_table is already initialized!\n");
		return 0;
	}

	cmd_table = (struct cmdlist_table *)kzalloc(sizeof(struct cmdlist_table), GFP_KERNEL);
	if (!cmd_table) {
		return -1;
	}

	cmd_table->scene_num = scene_num;
	cmd_table->current_scene = 0;
	cmd_table->flush_list = (struct clist_head *)kzalloc(sizeof(struct clist_head) * cmd_table->scene_num, GFP_KERNEL);
	if (!cmd_table->flush_list) {
		kfree(cmd_table);
		cmd_table = NULL;
		return -1;
	}

	for (i = 0; i < cmd_table->scene_num; i++)
		init_clist_head(&cmd_table->flush_list[i]);

	if (chash_init(&cmd_table->ht, cmd_table->scene_num, is_same_cmdlist_client, get_cmdlist_scene_id) != 0) {
		kfree(cmd_table->flush_list);
		kfree(cmd_table);
		cmd_table = NULL;
		return -1;
	}

	return 0;
}

int cmdlist_dev_close(void)
{
	int i;
	struct cmdlist_client *tmp = NULL;
	struct cmdlist_client *client_node = NULL;

	if (!cmd_table) {
		pr_info("cmd_table is already release!\n");
		return 0;
	}

	for (i = 0; i < cmd_table->ht.bkt_size; i++) {
		clist_for_each_entry_safe(client_node, tmp, &(cmd_table->ht.bkts[i]), struct cmdlist_client, hash_node) {
			pr_info("%s --> free client <%d, %p>!\n", __func__, client_node->key, client_node->value);
			cmdlist_release_node(client_node->value);
			chash_remove(&client_node->hash_node);
		}
	}
	kfree(cmd_table->ht.bkts);
	kfree(cmd_table->flush_list);
	kfree(cmd_table);
	cmd_table = NULL;

	return 0;
}

static void cmdlist_chash_put(struct cmdlist_table *table, int key, struct cmdlist_node *node)
{
	struct cmdlist_client *client = NULL;

	client = (struct cmdlist_client *)kzalloc(sizeof(struct cmdlist_client), GFP_KERNEL);
	if (!client) {
		pr_err("kzalloc return err!\n");
		return;
	}

	client->key = key;
	client->value = node;
	client->scene_id = node->scene_id;
	client->already_appended = 0;

	chash_add(&table->ht, &client->hash_node);
	pr_info("%s --> add client <%d, %p>!\n", __func__, client->key, client->value);
}

int cmdlist_create_client(struct cmdlist_node_client *user_client)
{
	struct cmdlist_node *node = NULL;

	if (!user_client) {
		pr_err("input error, user_client is null pointer!\n");
		return -1;
	}

	if (!cmd_table) {
		pr_err("cmd_table is uninitialized!\n");
		return -1;
	}

	if (user_client->scene_id > cmd_table->scene_num) {
		pr_err("invalid scene_id:%d!\n", user_client->scene_id);
		return -1;
	}

	node = cmdlist_create_node(user_client->scene_id, user_client->type,
		user_client->node_size, user_client->dst_addr);
	if (!node) {
		pr_err("cmdlist create node failed!\n");
		return -1;
	}
	user_client->id = ++g_cmdlist_next_id;
	user_client->fence_fd = node->fence_fd;
	user_client->append_next_id = 0;
	user_client->link_next_id = 0;
	user_client->node_size = node->node_size;
	user_client->phyaddr = node->cmd_header_addr;
	user_client->viraddr = (uint64_t)(uintptr_t)node->list_cmd_header;

	node->fence_fd = -1;
	cmdlist_chash_put(cmd_table, user_client->id, node);

	return 0;
}

static struct cmdlist_client *cmdlist_get_current_client(struct cmdlist_node_client *user_client)
{
	struct cnode *node = NULL;
	struct cmdlist_client *client_node = NULL;
	struct cmdlist_client compare_client;

	if (!user_client) {
		pr_err("input error, user_client is null pointer!\n");
		return NULL;
	}

	if (!cmd_table) {
		pr_err("cmd_table is uninitialized!\n");
		return NULL;
	}

	if (user_client->scene_id > cmd_table->scene_num) {
		pr_err("invalid scene_id:%d!\n", user_client->scene_id);
		return NULL;
	}

	compare_client.key = user_client->id;
	compare_client.scene_id = user_client->scene_id;
	node = chash_find(&cmd_table->ht, &compare_client.hash_node);
	if (!node) {
		pr_err("input err user client_id[%d] and scene_id[%d]!\n",
			user_client->id, user_client->scene_id);
		return NULL;
	}
	client_node = cnode_entry(node, struct cmdlist_client, hash_node);

	return client_node;
}

int cmdlist_append_next_client(struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client_node = NULL;

	client_node = cmdlist_get_current_client(user_client);
	if (!client_node) {
		pr_err("Invalid current cmdlist client!\n");
		return -1;
	}

	if (clist_is_empty(&cmd_table->flush_list[user_client->scene_id])) {
		clist_add_tail(&cmd_table->flush_list[user_client->scene_id], &client_node->list_node);
		pr_info("scene_id[%d]: flush_list is empty, id = %d is header!\n",
			user_client->scene_id, user_client->id);
		client_node->already_appended = 1;
	}

	pr_info("scene_id[%d]: flush_list id[%d] -->...--> newId[%d]!\n",
		user_client->scene_id, user_client->id, user_client->append_next_id);

	user_client->id = user_client->append_next_id;
	client_node = cmdlist_get_current_client(user_client);
	if (!client_node) {
		pr_err("Invalid next cmdlist client!\n");
		return -1;
	}

	if (client_node->already_appended == 0) {
		clist_add_tail(&cmd_table->flush_list[user_client->scene_id], &client_node->list_node);
		client_node->already_appended = 1;
	}

	return 0;
}

int cmdlist_link_next_client(struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client_node = NULL;
	struct cmdlist_client *client_next_node = NULL;

	client_node = cmdlist_get_current_client(user_client);
	if (!client_node) {
		pr_err("Invalid current cmdlist client!\n");
		return -1;
	}

	user_client->id = user_client->link_next_id;
	client_next_node = cmdlist_get_current_client(user_client);
	if (!client_next_node) {
		pr_err("Invalid next cmdlist client!\n");
		return -1;
	}

	client_node->value->list_cmd_header->next_list = client_next_node->value->cmd_header_addr;

	pr_info("%s ------> client_node<%d, %d, %p> link to client_node<%d, %d, %p>!\n",
		__func__, user_client->id, client_node->key, client_node->value,
		user_client->link_next_id, client_next_node->key, client_next_node->value);

	return 0;
}

int cmdlist_signal_client(struct cmdlist_node_client *user_client)
{
	int ret = 0;
	struct cmdlist_client *client_node = NULL;

	client_node = cmdlist_get_current_client(user_client);
	if (!client_node) {
		pr_err("Invalid current cmdlist client!\n");
		return -1;
	}
	pr_info("%s --> get client<%d, %p> signal fence_fd = %d!\n",
		__func__, client_node->key, client_node->value, user_client->fence_fd);

	if (client_node->value) {
		pr_info("%s --> signal client cmdlist_node<%d, %p>!\n",
			__func__, client_node->key, client_node->value);
		ret = dkmd_acquire_fence_signal(user_client->fence_fd);
	}

	return ret;
}

int cmdlist_dump_user_client(struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *client_node = NULL;

	client_node = cmdlist_get_current_client(user_client);
	if (!client_node) {
		pr_err("Invalid current cmdlist client!\n");
		return -1;
	}

	pr_info("%s --> get client<%d, %p>!\n",
		__func__, client_node->key, client_node->value);

	cmdlist_dump_client(client_node->value);

	return 0;
}

int cmdlist_dump_scene_client(struct cmdlist_node_client *user_client)
{
	struct cmdlist_client *tmp = NULL;
	struct cmdlist_client *client_node = NULL;

	if (!user_client) {
		pr_err("input error, user_client is null pointer!\n");
		return -1;
	}

	if (!cmd_table) {
		pr_err("cmd_table is uninitialized!\n");
		return -1;
	}

	if (user_client->scene_id > cmd_table->scene_num) {
		pr_err("invalid scene_id[%d]!\n", user_client->scene_id);
		return -1;
	}

	pr_info("%s --> scene_id = %d, id = %d!\n", __func__, user_client->scene_id, user_client->id);

	clist_for_each_entry_safe(client_node, tmp,
		&(cmd_table->flush_list[user_client->scene_id]), struct cmdlist_client, list_node) {
		pr_info("%s --> get client<%d, %p>!\n",
			__func__, client_node->key, client_node->value);
		cmdlist_dump_client(client_node->value);
	}

	return 0;
}

void cmdlist_dump_all_client(void)
{
	int i;
	struct cnode *node = NULL;
	struct cmdlist_client *client_node = NULL;

	if (!cmd_table) {
		pr_err("cmd_table is uninitialized!\n");
		return ;
	}

	pr_info("%s --> scene_num = %d, current_scene = %d!\n",
		__func__, cmd_table->scene_num, cmd_table->current_scene);
	for (i = 0; i < cmd_table->ht.bkt_size; i++) {
		pr_info("%s --> dump bkt scene_id = %d!\n", __func__, i);
		clist_for_each(node, &(cmd_table->ht.bkts[i])) {
			client_node = cnode_entry(node, struct cmdlist_client, hash_node);
			pr_info("%s --> get client<%d, %p>!\n",
				__func__, client_node->key, client_node->value);
			cmdlist_dump_client(client_node->value);
		}

		pr_info("%s --> dump flush_list scene_id = %d!\n", __func__, i);
		clist_for_each(node, &(cmd_table->flush_list[i])) {
			client_node = cnode_entry(node, struct cmdlist_client, list_node);
			pr_info("%s --> get client<%d, %p>!\n",
				__func__, client_node->key, client_node->value);
			cmdlist_dump_client(client_node->value);
		}
	}
}

#define BUF_SYNC_TIMEOUT_MSEC (4 * MSEC_PER_SEC)

int cmdlist_scene_client_commit(int scene_id)
{
	struct cmdlist_client *tmp = NULL;
	struct cmdlist_client *client_node = NULL;

	if (!cmd_table) {
		pr_err("cmd_table is uninitialized!\n");
		return -1;
	}

	if (scene_id > cmd_table->scene_num) {
		pr_err("Invalid scene_id[%d]!\n", scene_id);
		return -1;
	}

	pr_info("%s --> scene_id = %d commit!\n", __func__, scene_id);

	clist_for_each_entry_safe(client_node, tmp,
		&(cmd_table->flush_list[scene_id]), struct cmdlist_client, list_node) {
		pr_info("%s --> get client<%d, %p>!\n",
			__func__, client_node->key, client_node->value);
		cmdlist_dump_client(client_node->value);
		if (client_node->value->fence_fd > 0)
			dkmd_acquire_fence_wait(client_node->value->fence_fd, BUF_SYNC_TIMEOUT_MSEC);
	}

	client_node = clist_first_entry(&(cmd_table->flush_list[scene_id]), struct cmdlist_client, list_node);
	if (client_node) {
		pr_info("%s --> get client<%d, %p>!\n",
			__func__, client_node->key, client_node->value);
		cmdlist_flush_scene(client_node->value);
	}

	return 0;
}
EXPORT_SYMBOL(cmdlist_scene_client_commit);

MODULE_LICENSE("GPL");