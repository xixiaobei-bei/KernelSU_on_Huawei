/**
 * @file cmdlist_node.c
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
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/dma-fence.h>

#include "dkmd_acquire_fence.h"
#include "cmdlist_node.h"
#include "cmdlist_dev.h"

static int cmdlist_alloc_items(struct cmdlist_node *node, uint32_t size, uint32_t dst_addr)
{
	node->list_cmd_header = (struct cmd_header *)(uintptr_t)cmdlist_mem_alloc(size,
		&node->cmd_header_addr, (uint32_t *)&node->node_size);
	if (!node->list_cmd_header) {
		pr_err("cmd header alloc failed!\n");
		return -1;
	}

	if ((node->node_size != 0) && (size > node->node_size)) {
		pr_err("return invalid size: 0x%x, node_size: 0x%x!\n", size, node->node_size);
		cmdlist_mem_free(node->list_cmd_header, node->node_size);
		return -1;
	}

	node->list_cmd_header->cmd_flag.bits.cmd_mode = node->cmd_mode;
	node->list_cmd_header->cmd_flag.bits.id = 0;
	node->list_cmd_header->cmd_flag.bits.pending = 0;
	node->list_cmd_header->cmd_flag.bits.last = 0;
	node->list_cmd_header->cmd_flag.bits.nop = 0;
	node->list_cmd_header->cmd_flag.bits.task_end = 0;
	node->list_cmd_header->cmd_flag.bits.valid_flag = 0xA5;
	node->list_cmd_header->cmd_flag.bits.exec = 0x0;
	node->list_cmd_header->cmd_flag.bits.event_list = 0;

	node->list_cmd_item = (struct cmd_item *)(node->list_cmd_header + 1);
	node->cmd_item_addr = node->cmd_header_addr + sizeof(struct cmd_header);

	pr_info("alloc node header_addr[0x%x].list_cmd_header[%p].node_size[%zu]\n",
		node->cmd_header_addr, node->list_cmd_header, node->node_size);

	node->list_cmd_header->list_addr = node->cmd_item_addr;
	node->payload_size = (size >> 4);
	node->list_cmd_header->total_items.bits.dma_dst_addr = (dst_addr >> 2);
	node->list_cmd_header->total_items.bits.items_count = node->payload_size;

	return 0;
}

/**
 * @brief Create special cmdlist node
 *
 * @param scene_id dpu work scene mode id(0..3: online mode, 4..6: offline mode etc.)
 * @param mode cmdlist node work mode(0: config mode, 1: transport mode, 2: nop mode)
 * @param size cmdlist node buffer size
 * @param dst_addr cmdlist node config dst addr
 * @return struct cmdlist_node* node
 */
struct cmdlist_node *cmdlist_create_node(uint32_t scene_id, uint32_t mode, uint32_t size, uint32_t dst_addr)
{
	struct cmdlist_node *node = NULL;

	node = (struct cmdlist_node *)kzalloc(sizeof(*node), GFP_KERNEL);
	if (!node) {
		pr_err("cmdlist create node failed!\n");
		return NULL;
	}
	node->cmd_mode = mode;
	node->scene_id = scene_id;
	node->fence_fd = -1;
	spin_lock_init(&node->lock);

	if (node->cmd_mode == NOP_MODE) {
		node->node_size = 0;
		node->list_cmd_header = NULL;
		node->cmd_header_addr = 0;
		node->payload_size = 0;
	} else {
		if (cmdlist_alloc_items(node, size, dst_addr) != 0) {
			pr_err("cmdlist alloc items failed!\n");
			kfree(node);
			return NULL;
		}
		node->fence_fd = dkmd_acquire_fence_create_fd(&node->base, &node->lock, scene_id);
	}

	pr_info("[%s] --> return node[%p].header_addr[0x%x].items_addr[0x%x].scene[%d].mode[%d] fence_fd=%d!\n",
		__func__, node, node->cmd_header_addr, node->cmd_item_addr, node->scene_id, node->cmd_mode, node->fence_fd);

	return node;
}

/**
 * @brief Release and free cmdlist node
 *
 * @param node special cmdlist node
 * @return int ret 0: success -1: fail
 */
int cmdlist_release_node(struct cmdlist_node *node)
{
	if (!node) {
		pr_err("cmdlist node is null!\n");
		return -1;
	}

	if (node->list_cmd_header != NULL)
		cmdlist_mem_free(node->list_cmd_header, node->node_size);

	if (node->fence_fd > 0) {
		sys_close(node->fence_fd);
		node->fence_fd = -1;
	}

	kfree(node);
	node = NULL;

	return 0;
}

/**
 * @brief set cmdlist items append on special cmdlist node
 *        This function will get the last position node list pointer,
 *        if the newest node's payload_size oversize node_size (transport mode cmdlist node),
 *        need create new cmdlist node list and append cmd_items on the newest cmdlist.
 *
 * @param node special cmdlist node pointer
 * @param addr base on dpu memory map, such as cmdlist_offset is 0x12000
 * @param value actual value to be configured
 * @return int ret 0: success -1: fail
 */
int cmdlist_set_reg(struct cmdlist_node *node, uint32_t addr, uint32_t value)
{
	uint32_t prev_item_base_addr = 0, new_addr = 0;
	struct cmdlist_node *new_node = NULL;
	struct cmd_item *current_cmd_item = NULL, *new_cmd_item = NULL;

	new_node = node;
	if (!new_node) {
		pr_err("new_node is null!\n");
		return -1;
	}

	new_addr = addr >> 2;
	new_node->list_cmd_header->total_items.bits.items_count = new_node->payload_size + 1;
	current_cmd_item = (struct cmd_item *)(new_node->list_cmd_item + new_node->payload_size);

	prev_item_base_addr = current_cmd_item->cmd_item.cmd_mode_0.addr0;
	if (prev_item_base_addr == 0) {
		current_cmd_item->cmd_item.cmd_mode_0.addr0 = new_addr;
		current_cmd_item->cmd_item.cmd_mode_0.data0 = value;
		current_cmd_item->cmd_item.cmd_mode_0.cnt = 0;
		return 0;
	}

	/* 0x3F: bit[5:0] */
	if ((new_addr > prev_item_base_addr) && (new_addr - prev_item_base_addr < 0x3F)) {
		if (current_cmd_item->cmd_item.cmd_mode_0.addr1 == 0) {
			current_cmd_item->cmd_item.cmd_mode_0.addr1 = new_addr - prev_item_base_addr;
			current_cmd_item->cmd_item.cmd_mode_0.data1 = value;
			current_cmd_item->cmd_item.cmd_mode_0.cnt = 1;
			return 0;
		}
		if (current_cmd_item->cmd_item.cmd_mode_0.addr2 == 0) {
			current_cmd_item->cmd_item.cmd_mode_0.addr2 = new_addr - prev_item_base_addr;
			current_cmd_item->cmd_item.cmd_mode_0.data2 = value;
			current_cmd_item->cmd_item.cmd_mode_0.cnt = 2;
			return 0;
		}
	}

	/* need get new cmd item */
	new_node->payload_size++;
	new_cmd_item = (struct cmd_item *)(new_node->list_cmd_item + new_node->payload_size);
	new_cmd_item->cmd_item.cmd_mode_0.addr0 = new_addr;
	new_cmd_item->cmd_item.cmd_mode_0.data0 = value;
	new_cmd_item->cmd_item.cmd_mode_0.cnt = 0;

	/* record items num */
	new_node->list_cmd_header->total_items.bits.items_count = new_node->payload_size + 1;

	return 0;
}

/**
 * @brief dump and save cmdlist node
 *
 * @param node special cmdlist node pointer
 */
void cmdlist_dump_client(struct cmdlist_node *node)
{
	uint32_t i = 0;
	struct cmd_item *temp_item = NULL;
	uint32_t *temp_dm = NULL;
	uint32_t items_count = 0;

	if (!node || (node->node_size == 0))
		return;

	pr_info("cmdlist_node: %p! fence_fd: %d\n", node, node->fence_fd);
	pr_info("   cmd_flag    |   next_list   |   total_items   |   list_addr  \n");
	pr_info(" --------------+---------------+-----------------+--------------\n");
	pr_info("   0x%8x  |   0x%8x  |   0x%8x    |   0x%8x \n",
		node->list_cmd_header->cmd_flag.ul32, node->list_cmd_header->next_list,
		node->list_cmd_header->total_items.ul32, node->list_cmd_header->list_addr);

	items_count = node->list_cmd_header->total_items.bits.items_count;
	if (node->cmd_mode == TRANSPORT_MODE) {
		temp_dm = (uint32_t *)node->list_cmd_item;
		items_count = (node->payload_size << 2);
		pr_info("dm-->items_count: %d\n", items_count/4);

		for ( i = 0; i <= items_count; i += 4)
			pr_info("addr_offset:0x%x value:0x%08x 0x%08x 0x%08x 0x%08x \n",
			(node->list_cmd_header->total_items.bits.dma_dst_addr << 2) + i * 4,
			temp_dm[i], temp_dm[i + 1], temp_dm[i + 2], temp_dm[i + 3]);
	} else {
		pr_info("items_count: %d \n", items_count);
		for (i = 0; i < items_count; i++) {
			temp_item = &(node->list_cmd_item[i]);
			pr_info("set addr:0x%08x value:0x%08x addr1:0x%02x value:0x%08x addr2:0x%02x value:0x%08x\n",
				(temp_item->cmd_item.cmd_mode_0.addr0 << 2), temp_item->cmd_item.cmd_mode_0.data0,
				temp_item->cmd_item.cmd_mode_0.addr1, temp_item->cmd_item.cmd_mode_0.data1,
				temp_item->cmd_item.cmd_mode_0.addr2, temp_item->cmd_item.cmd_mode_0.data2);
		}
	}
}

/**
 * @brief Make effective cmdlist hardware
 *
 * @param node special cmdlist node pointer
 */
void cmdlist_flush_scene(struct cmdlist_node *node)
{




}