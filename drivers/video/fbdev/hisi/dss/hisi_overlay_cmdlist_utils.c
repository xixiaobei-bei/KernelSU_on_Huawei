/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#define CMDLIST_HEADER_LEN (SZ_1K)
#define CMDLIST_ITEM_LEN (SZ_8K)

struct list_item_param {
	uint8_t bw;
	uint8_t bs;
	uint32_t value;
	uint32_t new_addr;
	uint32_t old_addr;
	int condition;
};

struct dss_cmdlist_data *g_cmdlist_data;
uint32_t g_online_cmdlist_idxs;
uint32_t g_offline_cmdlist_idxs;

uint32_t g_cmdlist_chn_map[DSS_CMDLIST_MAX] = {
	DSS_CMDLIST_CHN_D2, DSS_CMDLIST_CHN_D3,
	DSS_CMDLIST_CHN_V0, DSS_CMDLIST_CHN_G0,
	DSS_CMDLIST_CHN_V1, DSS_CMDLIST_CHN_G1,
	DSS_CMDLIST_CHN_D0, DSS_CMDLIST_CHN_D1,
	DSS_CMDLIST_CHN_W0, DSS_CMDLIST_CHN_W1,
	DSS_CMDLIST_CHN_OV0, DSS_CMDLIST_CHN_OV1,
	DSS_CMDLIST_CHN_OV2, DSS_CMDLIST_CHN_OV3,
	DSS_CMDLIST_CHN_V2, DSS_CMDLIST_CHN_W2};

uint32_t g_cmdlist_eventlist_map[DSS_CMDLIST_MAX] = {
	DSS_EVENT_LIST_D2, DSS_EVENT_LIST_D3,
	DSS_EVENT_LIST_V0, DSS_EVENT_LIST_G0,
	DSS_EVENT_LIST_V1, DSS_EVENT_LIST_G1,
	DSS_EVENT_LIST_D0, DSS_EVENT_LIST_D1,
	DSS_EVENT_LIST_W0, DSS_EVENT_LIST_W1,
	DSS_EVENT_LIST_OV0, DSS_EVENT_LIST_OV1,
	DSS_EVENT_LIST_OV2, DSS_EVENT_LIST_OV3,
	DSS_EVENT_LIST_V2, DSS_EVENT_LIST_W2};

static uint32_t g_cmdlist_chnidx[DSS_CHN_MAX_DEFINE] = {
	DSS_CMDLIST_D2, DSS_CMDLIST_D3, DSS_CMDLIST_V0, DSS_CMDLIST_G0,
	DSS_CMDLIST_V1, DSS_CMDLIST_G1, DSS_CMDLIST_D0, DSS_CMDLIST_D1,
	DSS_CMDLIST_W0, DSS_CMDLIST_W1, DSS_CMDLIST_V2, DSS_CMDLIST_W2};

static uint32_t g_cmdlist_ovlidx[DSS_OVL_IDX_MAX] = {
	DSS_CMDLIST_OV0, DSS_CMDLIST_OV1,
	DSS_CMDLIST_OV2, DSS_CMDLIST_OV3};

int32_t hisi_get_cmdlist_idx_by_chnidx(uint32_t chnidx)
{
	if (unlikely(chnidx >= DSS_CHN_MAX_DEFINE)) {
		DPU_FB_ERR("chnidx %u is invalid!\n", chnidx);
		return -1;
	}

	return g_cmdlist_chnidx[chnidx];
}

int32_t hisi_get_cmdlist_idx_by_ovlidx(uint32_t ovlidx)
{
	if (unlikely(ovlidx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("ovlidx %d is invalid!\n", ovlidx);
		return -1;
	}

	return g_cmdlist_ovlidx[ovlidx];
}

uint32_t hisi_get_cmdlist_chnidx(uint32_t poolidx)
{
	if (unlikely(poolidx >= DSS_CMDLIST_MAX)) {
		DPU_FB_ERR("poolidx %d is invalid!\n", poolidx);
		return DSS_CMDLIST_CHN_MAX;
	}

	return g_cmdlist_chn_map[poolidx];
}

#if !defined(CONFIG_HISI_DSS_CMDLIST_LAST_USED) || defined(CONFIG_HISI_FB_V320) || defined(CONFIG_HISI_FB_V330)
static uint32_t hisi_get_cmdlist_chnidxs_by_poolidxs(uint32_t poolidxs)
{
	int i;
	uint32_t cmdlist_idxs_temp = poolidxs;
	uint32_t cmdlist_chnidxs = 0;
	uint32_t cmdlist_chnidx;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			cmdlist_chnidx = hisi_get_cmdlist_chnidx(i);
			cmdlist_chnidxs |= (1 << cmdlist_chnidx);
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	return cmdlist_chnidxs;
}
#endif

static int32_t hisi_get_eventlist_by_poolidx(uint32_t poolidx)
{
	if (unlikely(poolidx >= DSS_CMDLIST_MAX)) {
		DPU_FB_ERR("poolidx %d is invalid!\n", poolidx);
		return -1;
	}

	return g_cmdlist_eventlist_map[poolidx];
}

static int hisi_cmdlist_get_cmdlist_idxs_temp(dss_overlay_t *pov_req)
{
	uint32_t i;
	uint32_t k;
	uint32_t m;
	dss_layer_t *layer = NULL;
	dss_wb_layer_t *wb_layer = NULL;
	uint32_t cmdlist_idxs_temp = 0;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	bool no_ovl_idx = false;

	pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)pov_req->ov_block_infos_ptr;
	for (m = 0; m < pov_req->ov_block_nums; m++) {
		pov_h_block = &(pov_h_block_infos[m]);
		for (i = 0; i < pov_h_block->layer_nums; i++) {
			layer = &(pov_h_block->layer_infos[i]);

			if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
				continue;

			cmdlist_idxs_temp |=
				(1 << (uint32_t)hisi_get_cmdlist_idx_by_chnidx((uint32_t)(layer->chn_idx)));
		}
	}

	if (pov_req->wb_enable == 1) {
		for (k = 0; k < pov_req->wb_layer_nums; k++) {
			wb_layer = &(pov_req->wb_layer_infos[k]);

			if (wb_layer->chn_idx == DSS_WCHN_W2)
				no_ovl_idx = true;

			cmdlist_idxs_temp |=
				(1 << (uint32_t)hisi_get_cmdlist_idx_by_chnidx((uint32_t)(wb_layer->chn_idx)));
		}
	}

	if (no_ovl_idx == false)
		cmdlist_idxs_temp |= (1 << (uint32_t)hisi_get_cmdlist_idx_by_ovlidx(pov_req->ovl_idx));

	return cmdlist_idxs_temp;
}

/* get cmdlist indexs */
int hisi_cmdlist_get_cmdlist_idxs(dss_overlay_t *pov_req,
	uint32_t *cmdlist_pre_idxs, uint32_t *cmdlist_idxs)
{
	uint32_t cmdlist_idxs_temp;

	cmdlist_idxs_temp = hisi_cmdlist_get_cmdlist_idxs_temp(pov_req);
	if (cmdlist_idxs_temp & (~HISI_DSS_CMDLIST_IDXS_MAX)) {
		DPU_FB_ERR("cmdlist_idxs_temp(0x%x) is invalid!\n", cmdlist_idxs_temp);
		return -EINVAL;
	}

	if (cmdlist_idxs && cmdlist_pre_idxs) {
		*cmdlist_idxs = cmdlist_idxs_temp;
		*cmdlist_pre_idxs &= (~(*cmdlist_idxs));
	} else if (cmdlist_idxs) {
		*cmdlist_idxs = cmdlist_idxs_temp;
	} else if (cmdlist_pre_idxs) {
		*cmdlist_pre_idxs = cmdlist_idxs_temp;
	} else {
		DPU_FB_ERR("cmdlist_idxs && cmdlist_pre_idxs is NULL!\n");
		return -EINVAL;
	}

	if (g_debug_ovl_cmdlist)
		DPU_FB_INFO("cmdlist_pre_idxs(0x%x), cmdlist_idxs(0x%x).\n",
			(cmdlist_pre_idxs ? *cmdlist_pre_idxs : 0),
			(cmdlist_idxs ? *cmdlist_idxs : 0));

	return 0;
}

uint32_t hisi_cmdlist_get_cmdlist_need_start(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs)
{
	uint32_t cmdlist_idxs_temp;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return 0;
	}

	cmdlist_idxs_temp = g_offline_cmdlist_idxs;
	g_offline_cmdlist_idxs |= cmdlist_idxs;
	cmdlist_idxs_temp = (g_offline_cmdlist_idxs & (~cmdlist_idxs_temp));

	cmdlist_idxs_temp |= (cmdlist_idxs & g_online_cmdlist_idxs);
	g_online_cmdlist_idxs &= (~cmdlist_idxs_temp);

	if (g_debug_ovl_cmdlist)
		DPU_FB_INFO("g_online_cmdlist_idxs=0x%x, cmdlist_idxs_need_start=0x%x\n",
			g_online_cmdlist_idxs, cmdlist_idxs_temp);

	return cmdlist_idxs_temp;
}

/*
 * data0: addr0[17:0]
 * data1: addr0[17:0] + addr1[5:0]
 * data2: addr0[17:0] + addr2[5:0]
 *
 * cnt[1:0]:
 * 2'b00: reg0
 * 2'b01: reg0, reg1
 * 2'b10: reg0, reg1, reg2
 * 2'b11: ((inp32(addr0) & data1) | data2) -> addr0
 */
bool hisi_check_cmdlist_paremeters_validate(uint8_t bw, uint8_t bs)
{
	if ((bs > 32) || (bw > 32) || ((bw + bs) > 32)) {
		DPU_FB_ERR("Can't do this,which may cause overflow.\n");
		return false;
	}

	return true;
}

static void list_item_set_reg(struct dss_cmdlist_node *node, struct list_item_param item_param)
{
	int index;
	uint64_t mask = ((uint64_t)1 << item_param.bw) - 1;

	index = node->item_index;
	if (item_param.bw != 32) {  /* 4bytes */
		if (node->item_flag != 0)
			index++;

		node->list_item[index].reg_addr.bits.add0 = item_param.new_addr;
		node->list_item[index].data0 = item_param.value;
		node->list_item[index].data1 = (uint32_t) ((~(mask << item_param.bs)) & 0xFFFFFFFF);
		node->list_item[index].data2 = (uint32_t) (((mask & item_param.value) << item_param.bs) & 0xFFFFFFFF);
		node->list_item[index].reg_addr.bits.cnt = 3;  /* number of config registers */
		node->item_flag = 3;  /* number of config registers */
	} else {
		if (node->item_flag == 0) {
			node->list_item[index].reg_addr.bits.add0 = item_param.new_addr;
			node->list_item[index].data0 = item_param.value;
			node->list_item[index].reg_addr.bits.cnt = 0;
			node->item_flag = 1;
		} else if (node->item_flag == 1 && item_param.condition) {
			node->list_item[index].reg_addr.bits.add1 = item_param.new_addr - item_param.old_addr;
			node->list_item[index].data1 = item_param.value;
			node->list_item[index].reg_addr.bits.cnt = 1;
			node->item_flag = 2;
		} else if (node->item_flag == 2 && item_param.condition) {
			node->list_item[index].reg_addr.bits.add2 = item_param.new_addr - item_param.old_addr;
			node->list_item[index].data2 = item_param.value;
			node->list_item[index].reg_addr.bits.cnt = 2;
			node->item_flag = 3;
		} else {
			index++;
			node->list_item[index].reg_addr.bits.add0 = item_param.new_addr;
			node->list_item[index].data0 = item_param.value;
			node->list_item[index].reg_addr.bits.cnt = 0;
			node->item_flag = 1;
		}
	}

	dpu_check_and_no_retval((index >= MAX_ITEM_INDEX), ERR, "index=%d is too large(1k)!\n", index);

	node->item_index = index;
	node->list_header->total_items.bits.count = node->item_index + 1;
}

void hisi_cmdlist_set_reg(struct dpu_fb_data_type *dpufd, char __iomem *addr,
	uint32_t value, uint8_t bw, uint8_t bs)
{
	struct dss_cmdlist_node *node = NULL;
	int cmdlist_idx;
	int index;
	uint64_t temp_addr = 0;
	struct list_item_param item_param = {0};

	item_param.bw = bw;
	item_param.bs = bs;
	item_param.value = value;

	dpu_check_and_no_retval(!addr, ERR, "addr is NULL!\n");
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");
	if (!hisi_check_cmdlist_paremeters_validate(bw, bs))
		return;

	cmdlist_idx = dpufd->cmdlist_idx;
	dpu_check_and_no_retval((cmdlist_idx < 0) || (cmdlist_idx >= HISI_DSS_CMDLIST_MAX), ERR,
		"cmdlist_idx is invalid\n");

	node = list_entry(dpufd->cmdlist_data->cmdlist_head_temp[cmdlist_idx].prev,
		struct dss_cmdlist_node, list_node);
	dpu_check_and_no_retval(!node, ERR, "node is NULL!\n");
	dpu_check_and_no_retval((node->node_type == CMDLIST_NODE_NOP), ERR,
		"can't set register value to NOP node!\n");

	index = node->item_index;
	if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		temp_addr = (uint64_t)(uintptr_t)(addr);
		item_param.new_addr = (uint32_t)(temp_addr & 0xFFFFFFFF);
	} else {
		item_param.new_addr = (uint32_t)(addr - dpufd->dss_base + dpufd->dss_base_phy);
	}

	item_param.new_addr = (item_param.new_addr >> 2) & CMDLIST_ADDR_OFFSET;
	item_param.old_addr = node->list_item[index].reg_addr.ul32 & CMDLIST_ADDR_OFFSET;
	item_param.condition = (((item_param.new_addr - item_param.old_addr) < MAX_ITEM_OFFSET) &&
		(item_param.new_addr >= item_param.old_addr));

	list_item_set_reg(node, item_param);
}

/*
 * flush cache for cmdlist, make sure that
 * cmdlist has writen through to memory before config register
 */
void hisi_cmdlist_flush_cache(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs)
{
	uint32_t i;
	uint32_t cmdlist_idxs_temp;
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;
	struct list_head *cmdlist_heads = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			cmdlist_heads = &(dpufd->cmdlist_data->cmdlist_head_temp[i]);
			if (!cmdlist_heads) {
				DPU_FB_ERR("cmdlist_data is NULL!\n");
				continue;
			}

			list_for_each_entry_safe_reverse(node, _node_, cmdlist_heads, list_node) {
				if (node) {
					dma_sync_single_for_device(NULL, node->header_phys,
						node->header_len, DMA_TO_DEVICE);
					dma_sync_single_for_device(NULL, node->item_phys,
						node->item_len, DMA_TO_DEVICE);
				}
			}
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}
}

struct dss_cmdlist_node *hisi_cmdlist_node_alloc(struct dpu_fb_data_type *dpufd, size_t header_len,
	size_t item_len)
{
	struct dss_cmdlist_node *node = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return NULL;
	}

	if (!dpufd->cmdlist_pool) {
		DPU_FB_ERR("fb%d, cmdlist_pool is NULL!\n", dpufd->index);
		return NULL;
	}

	node = (struct dss_cmdlist_node *)kzalloc(sizeof(struct dss_cmdlist_node), GFP_KERNEL);
	if (!node) {
		DPU_FB_ERR("failed to alloc struct dss_cmdlist_node!\n");
		return NULL;
	}

	memset(node, 0, sizeof(struct dss_cmdlist_node));

	node->header_len = header_len;
	node->item_len = item_len;

	/* alloc buffer for header */
	node->list_header = (struct cmd_header *)(uintptr_t)gen_pool_alloc(dpufd->cmdlist_pool, node->header_len);
	if (!node->list_header) {
		DPU_FB_ERR("fb%d, header gen_pool_alloc failed!\n", dpufd->index);
		goto err_header_alloc;
	}

	node->header_phys = gen_pool_virt_to_phys(dpufd->cmdlist_pool, (unsigned long)(uintptr_t)node->list_header);

	/* alloc buffer for items */
	node->list_item = (struct cmd_item *)(uintptr_t)gen_pool_alloc(dpufd->cmdlist_pool, node->item_len);
	if (!node->list_item) {
		DPU_FB_ERR("fb%d, item gen_pool_alloc failed!\n", dpufd->index);
		goto err_item_alloc;
	}

	node->item_phys = gen_pool_virt_to_phys(dpufd->cmdlist_pool, (unsigned long)(uintptr_t)node->list_item);

	memset(node->list_header, 0, node->header_len);
	memset(node->list_item, 0, node->item_len);

	/* fill node info */
	node->item_flag = 0;
	node->item_index = 0;

	node->is_used = 0;
	node->node_type = CMDLIST_NODE_NONE;
	return node;

err_item_alloc:
	if (node->list_header)
		gen_pool_free(dpufd->cmdlist_pool, (unsigned long)(uintptr_t)node->list_header, node->header_len);
err_header_alloc:
	if (node) {
		kfree(node);
		node = NULL;
	}

	return node;
}

void hisi_cmdlist_node_free(struct dpu_fb_data_type *dpufd, struct dss_cmdlist_node *node)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	if (!node) {
		DPU_FB_ERR("node is NULL!\n");
		return;
	}

	if (dpufd->cmdlist_pool && node->list_header) {
		gen_pool_free(dpufd->cmdlist_pool, (unsigned long)(uintptr_t)node->list_header, node->header_len);
		node->list_header = NULL;
	}

	if (dpufd->cmdlist_pool && node->list_item) {
		gen_pool_free(dpufd->cmdlist_pool, (unsigned long)(uintptr_t)node->list_item, node->item_len);
		node->list_item = NULL;
	}

	kfree(node);
	node = NULL;
}

static struct dss_cmdlist_node *hisi_cmdlist_get_free_node(struct dss_cmdlist_node *node[], int *id)
{
	int i;

	for (i = 0; i < HISI_DSS_CMDLIST_NODE_MAX; i++) {
		if (node[i] && (node[i]->is_used == 0)) {
			node[i]->is_used = 1;
			*id = i + 1;
			return node[i];
		}
	}

	return NULL;
}

static bool hisi_cmdlist_addr_check(struct dpu_fb_data_type *dpufd, uint32_t *list_addr)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return false;
	}

	if ((*list_addr > (uint32_t)(dpufd->cmdlist_pool_phy_addr + dpufd->sum_cmdlist_pool_size)) ||
		(*list_addr < (uint32_t)dpufd->cmdlist_pool_phy_addr)) {
		DPU_FB_ERR("fb%d, cmdlist_addr is invalid, sum_cmdlist_pool_size=%zu.\n",
			dpufd->index, dpufd->sum_cmdlist_pool_size);
		*list_addr = dpufd->cmdlist_pool_phy_addr;
		return false;
	}

	return true;
}

int hisi_cmdlist_add_nop_node(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs,
	int pending, int reserved)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *first_node = NULL;
	uint32_t cmdlist_idxs_temp;
	int i;
	int id = 0;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			if (!dpufd->cmdlist_data) {
				DPU_FB_ERR("cmdlist_data is nullptr!\n");
				return -EINVAL;
			}
			node = hisi_cmdlist_get_free_node(dpufd->cmdlist_data->cmdlist_nodes_temp[i], &id);
			if (!node) {
				DPU_FB_ERR("failed to hisi_get_free_cmdlist_node!\n");
				return -EINVAL;
			}

			node->list_header->flag.bits.id = id;
			node->list_header->flag.bits.nop = 0x1;
			node->list_header->flag.bits.pending = pending ? 0x1 : 0x0;
			node->list_header->flag.bits.valid_flag = CMDLIST_NODE_VALID;
			node->list_header->flag.bits.last = 0;
			node->list_header->next_list = node->header_phys;
			hisi_cmdlist_addr_check(dpufd, &(node->list_header->next_list));

			node->is_used = 1;
			node->node_type = CMDLIST_NODE_NOP;
			node->reserved = reserved ? 0x1 : 0x0;

			/* add this nop to list */
			list_add_tail(&(node->list_node), &(dpufd->cmdlist_data->cmdlist_head_temp[i]));

			if (node->list_node.prev != &(dpufd->cmdlist_data->cmdlist_head_temp[i])) {
				struct dss_cmdlist_node *pre_node;

				first_node = list_first_entry(&(dpufd->cmdlist_data->cmdlist_head_temp[i]),
					struct dss_cmdlist_node, list_node);
				pre_node = list_entry(node->list_node.prev, struct dss_cmdlist_node, list_node);
				pre_node->list_header->next_list = first_node->header_phys;
				hisi_cmdlist_addr_check(dpufd, &(pre_node->list_header->next_list));

				if (node->list_header->flag.bits.pending == 0x1)
					pre_node->reserved = 0x0;

				pre_node->list_header->flag.bits.task_end = 0x1;
				node->list_header->flag.bits.task_end = 0x1;

				if (g_debug_ovl_cmdlist)
					DPU_FB_DEBUG("i = %d, next_list = 0x%x\n", i, (uint32_t)(node->header_phys));
			}
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

static void set_list_header_bits_event_list(struct dss_cmdlist_node *node, int remove,
	int i, int32_t event_list)
{
	if (i < DSS_CMDLIST_W0)
		node->list_header->flag.bits.event_list = remove ? 0x8 : (event_list);
	else if (i < DSS_CMDLIST_OV0)
		node->list_header->flag.bits.event_list = remove ? 0x8 : (event_list);
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
	else if (i == DSS_CMDLIST_V2)
		node->list_header->flag.bits.event_list = remove ? 0x8 : 0x16;
	else if (i == DSS_CMDLIST_W2)
		node->list_header->flag.bits.event_list = remove ? 0x8 : 0x20;
#endif
	else
		node->list_header->flag.bits.event_list = remove ? 0x8 : (event_list);
}

int hisi_cmdlist_add_new_node(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs,
	union cmd_flag flag, int remove)
{
	struct dss_cmdlist_node *node = NULL;
	uint32_t cmdlist_idxs_temp;
	int i;
	int id = 0;
	int32_t event_list;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			if (!dpufd->cmdlist_data) {
				DPU_FB_ERR("cmdlist_data is nullptr!\n");
				return -EINVAL;
			}
			node = hisi_cmdlist_get_free_node(dpufd->cmdlist_data->cmdlist_nodes_temp[i], &id);
			dpu_check_and_return(!node, -EINVAL, ERR, "failed to hisi_get_free_cmdnode!\n");

			/* fill the header and item info */
			node->list_header->flag.bits.id = id;
			node->list_header->flag.bits.pending = flag.bits.pending ? 0x1 : 0x0;

			event_list = hisi_get_eventlist_by_poolidx(i);

			set_list_header_bits_event_list(node, remove, i, event_list);

			node->list_header->flag.bits.task_end = flag.bits.task_end ? 0x1 : 0x0;
			node->list_header->flag.bits.last = flag.bits.last ? 0x1 : 0x0;

			node->list_header->flag.bits.valid_flag = CMDLIST_NODE_VALID;
			node->list_header->flag.bits.exec = 0x1;
			node->list_header->list_addr = node->item_phys;
			node->list_header->next_list = node->header_phys;

			hisi_cmdlist_addr_check(dpufd, &(node->list_header->list_addr));
			hisi_cmdlist_addr_check(dpufd, &(node->list_header->next_list));

			node->is_used = 1;
			node->node_type = CMDLIST_NODE_FRAME;
			node->item_flag = 0;
			node->reserved = 0;

			/* add this nop to list */
			list_add_tail(&(node->list_node), &(dpufd->cmdlist_data->cmdlist_head_temp[i]));

			if (node->list_node.prev != &(dpufd->cmdlist_data->cmdlist_head_temp[i])) {
				struct dss_cmdlist_node *pre_node;

				pre_node = list_entry(node->list_node.prev, struct dss_cmdlist_node, list_node);
				pre_node->list_header->next_list = node->header_phys;
				hisi_cmdlist_addr_check(dpufd, &(pre_node->list_header->next_list));
				pre_node->reserved = 0x0;
				if (g_debug_ovl_cmdlist)
					DPU_FB_DEBUG("i = %d, next_list = 0x%x\n",  i, (uint32_t)node->header_phys);
			}
		}

		cmdlist_idxs_temp  = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

int hisi_cmdlist_del_all_node(struct list_head *cmdlist_heads)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;

	if (!cmdlist_heads) {
		DPU_FB_ERR("cmdlist_heads is NULL\n");
		return -EINVAL;
	}

	list_for_each_entry_safe(node, _node_, cmdlist_heads, list_node) {
		if (node->reserved != 0x1) {
			list_del(&node->list_node);

			node->list_header->flag.bits.exec = 0;
			node->list_header->flag.bits.last = 1;
			node->list_header->flag.bits.nop = 0;
			node->list_header->flag.bits.interrupt = 0;
			node->list_header->flag.bits.pending = 0;
			node->list_header->flag.bits.id = 0;
			node->list_header->flag.bits.event_list = 0;
			node->list_header->flag.bits.qos = 0;
			node->list_header->flag.bits.task_end = 0;
			node->list_header->flag.bits.valid_flag = 0;
			node->list_header->total_items.ul32 = 0;

			memset(node->list_item, 0, CMDLIST_ITEM_LEN);

			node->item_index = 0;
			node->item_flag = 0;
			node->node_type = CMDLIST_NODE_NONE;
			node->is_used = 0;
		}
	}

	return 0;
}

int hisi_cmdlist_check_cmdlist_state(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs)
{
	char __iomem *cmdlist_base = NULL;
	uint32_t offset;
	uint32_t temp;
	uint32_t cmdlist_idxs_temp;
	int i;
	int delay_count = 0;
	bool is_timeout = true;
	int ret = 0;
	uint32_t cmdlist_chnidx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		cmdlist_base = dpufd->media_common_base + DSS_CMDLIST_OFFSET;
	else
		cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;


	offset = 0x40;
	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			cmdlist_chnidx = hisi_get_cmdlist_chnidx(i);
			while (1) {
				temp = inp32(cmdlist_base + CMDLIST_CH0_STATUS + cmdlist_chnidx * offset);
#ifdef CONFIG_HISI_DSS_CMDLIST_LAST_USED
				if (((temp & 0xF) == 0x0) || delay_count > 5000) {  /* 5ms */
#else
				if (((temp & 0xF) == 0x7) || delay_count > 5000) {  /* 5ms */
#endif
					is_timeout = (delay_count > 5000) ? true : false;  /* 5ms */
					delay_count = 0;
					break;
				} else {
					udelay(1);  /* 1us */
					++delay_count;
				}
			}

			if (is_timeout) {
#ifdef CONFIG_HISI_DSS_CMDLIST_LAST_USED
				DPU_FB_ERR("cmdlist_ch%d not in idle state,ints=0x%x !\n", cmdlist_chnidx, temp);
#else
				DPU_FB_ERR("cmdlist_ch%d not in pending state,ints=0x%x !\n", cmdlist_chnidx, temp);
#endif
				ret = -1;
			}
		}

		cmdlist_idxs_temp = (cmdlist_idxs_temp >> 1);
	}

	return ret;
}

/*
 * stop the pending state for one new frame
 * if the current cmdlist status is e_status_wait.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winteger-overflow"

static void hisi_cmdlist_chn_state_check(struct dpu_fb_data_type *dpufd,
	uint32_t cmdlist_idxs, uint32_t poolindex, const char __iomem *cmdlist_base)
{
	uint32_t temp;
	int delay_count = 0;
	bool is_timeout = true;
	uint32_t cmdlist_chnidx;

	cmdlist_chnidx = hisi_get_cmdlist_chnidx(poolindex);
	while (1) {
		temp = inp32(cmdlist_base + CMDLIST_CH0_STATUS + cmdlist_chnidx * 0x40);
#ifdef CONFIG_HISI_DSS_CMDLIST_LAST_USED
		if (((temp & 0xF) == 0x0) || delay_count > 500) {  /* 500us */
#else
		if (((temp & 0xF) == 0x7) || delay_count > 500) {  /* 500us */
#endif
			is_timeout = (delay_count > 500) ? true : false;  /* 500us */
			break;
		}

		udelay(1);  /* 1us */
		++delay_count;
	}

	if (is_timeout) {
#ifndef CONFIG_HISI_DSS_CMDLIST_LAST_USED
		DPU_FB_ERR("cmdlist_ch%d not in pending state,ints=0x%x !\n", cmdlist_chnidx, temp);
#else
		DPU_FB_ERR("cmdlist_ch%d not in idle state,ints=0x%x !\n", cmdlist_chnidx, temp);
#endif

		if (g_debug_ovl_cmdlist)
			hisi_cmdlist_dump_all_node(dpufd, NULL, cmdlist_idxs);

		if (g_debug_ovl_offline_composer_hold)
			mdelay(HISI_DSS_COMPOSER_HOLD_TIME);
	}
}

int hisi_cmdlist_exec(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs)
{
	char __iomem *cmdlist_base = NULL;
	uint32_t cmdlist_idxs_temp = 0;
	int i;
#ifndef CONFIG_HISI_DSS_CMDLIST_LAST_USED
	uint32_t cmdlist_chnidxs;
#endif

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");

	cmdlist_base = (dpufd->index == MEDIACOMMON_PANEL_IDX) ?
		(dpufd->media_common_base + DSS_CMDLIST_OFFSET) : (dpufd->dss_base + DSS_CMDLIST_OFFSET);

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1)
			hisi_cmdlist_chn_state_check(dpufd, cmdlist_idxs, i, cmdlist_base);

		cmdlist_idxs_temp = (cmdlist_idxs_temp >> 1);
	}

#ifndef CONFIG_HISI_DSS_CMDLIST_LAST_USED
	/* task continue */
	cmdlist_chnidxs = hisi_get_cmdlist_chnidxs_by_poolidxs(cmdlist_idxs);
	outp32(cmdlist_base + CMDLIST_TASK_CONTINUE, cmdlist_chnidxs);
	if (g_debug_set_reg_val)
		DPU_FB_INFO("writel: [0x%x] = 0x%x\n",
			(uint32_t)(cmdlist_base + CMDLIST_TASK_CONTINUE), cmdlist_chnidxs);
#endif

	return 0;
}

#pragma GCC diagnostic pop
/*
 * start cmdlist.
 * it will set cmdlist into pending state.
 */
extern uint32_t g_dss_module_ovl_base[DSS_MCTL_IDX_MAX][MODULE_OVL_MAX];

static uint32_t get_hisi_cmdlist_node_addr(struct dpu_fb_data_type *dpufd, int mctl_idx,
	uint32_t wb_compose_type, uint32_t cmdlist_index)
{
	struct dss_cmdlist_node *cmdlist_node = NULL;

	if (mctl_idx >= DSS_MCTL2) {
		if (!dpufd->cmdlist_data_tmp[wb_compose_type])
			return 0;

		cmdlist_node = list_first_entry(&(dpufd->cmdlist_data_tmp[wb_compose_type]->cmdlist_head_temp[cmdlist_index]),
			struct dss_cmdlist_node, list_node);
	} else {
		if (!dpufd->cmdlist_data)
			return 0;

		cmdlist_node = list_first_entry(&(dpufd->cmdlist_data->cmdlist_head_temp[cmdlist_index]),
			struct dss_cmdlist_node, list_node);
	}

	return cmdlist_node->header_phys;
}

static int config_hisi_cmdlist_init(struct dpu_fb_data_type *dpufd, int mctl_idx,
	uint32_t wb_compose_type, uint32_t *cmdlist_idxs_temp, uint32_t *temp)
{
	uint32_t i;
	uint32_t cmdlist_chnidx;
	int status_temp = 0;
	uint32_t ints_temp = 0;
	uint32_t list_addr = 0;
	uint32_t offset = 0;
	char __iomem *cmdlist_base = NULL;

	offset = 0x40; /* Indicates the offset value of a register */
	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((*cmdlist_idxs_temp & 0x1) == 0x1) {
			cmdlist_chnidx = hisi_get_cmdlist_chnidx(i);
			status_temp =  inp32(cmdlist_base + CMDLIST_CH0_STATUS + cmdlist_chnidx * offset);
			ints_temp = inp32(cmdlist_base + CMDLIST_CH0_INTS + cmdlist_chnidx * offset);

			list_addr = get_hisi_cmdlist_node_addr(dpufd, mctl_idx, wb_compose_type, i);
			if (g_debug_ovl_cmdlist)
				DPU_FB_INFO("list_addr:0x%x, i=%d, ints_temp=0x%x\n",
					list_addr, cmdlist_chnidx, ints_temp);

			if (hisi_cmdlist_addr_check(dpufd, &list_addr) == false)
				return -EINVAL;

			*temp |= (1 << cmdlist_chnidx);

			outp32(cmdlist_base + CMDLIST_ADDR_MASK_EN, BIT(cmdlist_chnidx));
			if (g_debug_set_reg_val)
				DPU_FB_INFO("writel: [%pK] = 0x%lx\n",
					cmdlist_base + CMDLIST_ADDR_MASK_EN, BIT(cmdlist_chnidx));

			if (mctl_idx <= DSS_MCTL1)
				set_reg(cmdlist_base + CMDLIST_CH0_CTRL + cmdlist_chnidx * offset, 0x1, 1, 6);
			else
				set_reg(cmdlist_base + CMDLIST_CH0_CTRL + cmdlist_chnidx * offset, 0x0, 1, 6);

			set_reg(cmdlist_base + CMDLIST_CH0_STAAD + cmdlist_chnidx * offset, list_addr, 32, 0);

			set_reg(cmdlist_base + CMDLIST_CH0_CTRL + cmdlist_chnidx * offset, 0x1, 1, 0);
			if ((mctl_idx <= DSS_MCTL1) && ((ints_temp & 0x2) == 0x2))
				set_reg(cmdlist_base + CMDLIST_SWRST, 0x1, 1, cmdlist_chnidx);

			if (mctl_idx >= DSS_MCTL2) {
				if ((((uint32_t)status_temp & 0xF) == 0x0) || ((ints_temp & 0x2) == 0x2))
					set_reg(cmdlist_base + CMDLIST_SWRST, 0x1, 1, cmdlist_chnidx);
				else
					DPU_FB_INFO("i=%d, status_temp=0x%x, ints_temp=0x%x\n",
						cmdlist_chnidx, status_temp, ints_temp);
			}
		}

		*cmdlist_idxs_temp = *cmdlist_idxs_temp >> 1;
	}
	return 0;
}

int hisi_cmdlist_config_start(struct dpu_fb_data_type *dpufd, int mctl_idx,
	uint32_t cmdlist_idxs, uint32_t wb_compose_type)
{
	char __iomem *mctl_base = NULL;
	char __iomem *cmdlist_base = NULL;
	uint32_t offset;
	uint32_t cmdlist_idxs_temp;
	uint32_t temp = 0;
	int i;
	uint32_t cmdlist_chnidx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");
	dpu_check_and_return(((mctl_idx < 0) || (mctl_idx >= DSS_MCTL_IDX_MAX)), -EINVAL, ERR,
		"mctl_idx=%d is invalid.", mctl_idx);

	mctl_base = dpufd->dss_base + g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	offset = 0x40;
	cmdlist_idxs_temp = cmdlist_idxs;

	dsb(sy);
	if (config_hisi_cmdlist_init(dpufd, mctl_idx, wb_compose_type, &cmdlist_idxs_temp, &temp) != 0)
		return -EINVAL;

	outp32(cmdlist_base + CMDLIST_ADDR_MASK_DIS, temp);
	if (g_debug_set_reg_val)
		DPU_FB_INFO("writel: [%pK] = 0x%x\n", cmdlist_base + CMDLIST_ADDR_MASK_DIS, temp);

	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			cmdlist_chnidx = hisi_get_cmdlist_chnidx(i);
			set_reg(cmdlist_base + CMDLIST_CH0_CTRL + cmdlist_chnidx * offset, mctl_idx, 3, 2);
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	if (mctl_idx >= DSS_MCTL2) {
		set_reg(mctl_base + MCTL_CTL_ST_SEL, 0x1, 1, 0);
		set_reg(mctl_base + MCTL_CTL_SW_ST, 0x1, 1, 0);
	}

	return 0;
}

static void set_hisi_cmdlist_config_mif(struct dpu_fb_data_type *dpufd,
	uint32_t *cmdlist_idxs_temp, uint8_t flag)
{
	int i;
	char __iomem *tmp_base = NULL;

	for (i = 0; i < 4; i++) {  /* mif_sub_ch_nums = 4 */
		if ((*cmdlist_idxs_temp & 0x1) == 0x1) {
			tmp_base = dpufd->dss_module.mif_ch_base[i];
			if (tmp_base)
				set_reg(tmp_base + MIF_CTRL0, flag, 1, 0);
		}

		*cmdlist_idxs_temp = *cmdlist_idxs_temp >> 1;
	}
}

void hisi_cmdlist_config_mif_reset(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, uint32_t cmdlist_idxs, int mctl_idx)
{
	char __iomem *dss_base = NULL;
	uint32_t cmdlist_idxs_temp;
	int delay_count = 0;
	bool is_timeout = true;
	int i;
	int j = 0;
	uint32_t tmp_reg = 0;
	int mif_nums_max = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL!\n");
	dpu_check_and_no_retval(!pov_req, ERR, "pov_req is NULL!\n");

	dss_base = dpufd->dss_base;

	mif_nums_max = (mctl_idx <= DSS_MCTL1) ? DSS_WCHN_W0 : DSS_CHN_MAX;

	if (mctl_idx != DSS_MCTL5) {
		/* check mif chn status & mif ctrl0: chn disable */
		cmdlist_idxs_temp = cmdlist_idxs;
		for (i = 0; i < mif_nums_max; i++) {
			if ((cmdlist_idxs_temp & 0x1) == 0x1) {
				is_timeout = false;

				while (1) {
					for (j = 1; j <= 4; j++)  /* mif_sub_ch_nums = 4 */
						tmp_reg |= (uint32_t)inp32(dss_base + DSS_MIF_OFFSET +
							MIF_STAT1 + 0x10 * (i * 4 + j));

					if (((tmp_reg & 0x1f) == 0x0) || delay_count > 500) {  /* 5ms */
						is_timeout = (delay_count > 500) ? true : false;
						delay_count = 0;
						break;
					} else {
						udelay(10);  /* 10us */
						++delay_count;
					}
				}

				if (is_timeout)
					DPU_FB_ERR("mif_ch%d MIF_STAT1=0x%x !\n", i, tmp_reg);
			}

			cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
		}

		cmdlist_idxs_temp = cmdlist_idxs;
		set_hisi_cmdlist_config_mif(dpufd, &cmdlist_idxs_temp, 0x0);
		mdelay(5);  /* 5ms */
		/* mif ctrl0: chn enable */
		set_hisi_cmdlist_config_mif(dpufd, &cmdlist_idxs_temp, 0x1);
	}
}

static bool dpufb_mctl_clear_ack_timeout(const char __iomem *mctl_base)
{
	uint32_t mctl_status;
	int delay_count = 0;
	bool is_timeout = false;

	while (1) {
		mctl_status = inp32(mctl_base + MCTL_CTL_STATUS);
		if (((mctl_status & 0x10) == 0) || (delay_count > 500)) {  /* 500us */
			is_timeout = (delay_count > 500) ? true : false;
			break;
		} else {
			udelay(1);  /* 1us */
			++delay_count;
		}
	}

	return is_timeout;
}

#if   defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501)
static char __iomem *get_ldi_base(struct dpu_fb_data_type *dpufd)
{
	char __iomem *ldi_base = NULL;

	ldi_base = dpufd->dss_base + DSS_LDI1_OFFSET;

	return ldi_base;
}
static void dpufb_mctl_sw_clr(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, uint32_t cmdlist_idxs)
{
	char __iomem *mctl_base = NULL;
	char __iomem *ldi_base = NULL;
	char __iomem *cmdlist_base = NULL;
	int mctl_idx;
	uint32_t tmp = 0;
	uint32_t i = 0;
	uint32_t isr_s1 = 0;
	uint32_t isr_s2 = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd or pov_req is NULL!\n");
	dpu_check_and_no_retval(!pov_req, ERR, "dpufd or pov_req is NULL!\n");

	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	/* set cmdlist chn soft reset */
	set_reg(cmdlist_base + CMDLIST_SWRST, cmdlist_idxs, 32, 0);

	mctl_idx = pov_req->ovl_idx;
	dpu_check_and_no_retval(((mctl_idx < 0) || (mctl_idx >= DSS_MCTL_IDX_MAX)), ERR,
		"mctl_idx=%d is invalid.", mctl_idx);

	mctl_base = dpufd->dss_module.mctl_base[mctl_idx];
	if (mctl_base) {
		set_reg(mctl_base + MCTL_CTL_CLEAR, 0x1, 1, 0);
		if (dpufb_mctl_clear_ack_timeout(mctl_base)) {
			DPU_FB_ERR("clear_ack_timeout, mctl_status=0x%x !\n", inp32(mctl_base + MCTL_CTL_STATUS));
			for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
				DPU_FB_ERR("chn%d: [DMA_BUF_DBG0]=0x%x [DMA_BUF_DBG1]=0x%x !\n", i,
					inp32(dpufd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG0),
					inp32(dpufd->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG1));
			}
		}
	}

	if ((dpufd->index == PRIMARY_PANEL_IDX) || (dpufd->index == EXTERNAL_PANEL_IDX))
		enable_ldi(dpufd);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_PDP_INTS);
		isr_s2 = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS);
		outp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS, isr_s2);
		outp32(dpufd->dss_base + GLB_CPU_PDP_INTS, isr_s1);

		tmp = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK);
		outp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK, tmp);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_SDP_INTS);
		ldi_base = get_ldi_base(dpufd);
		isr_s2 = inp32(ldi_base + LDI_CPU_ITF_INTS);
		outp32(ldi_base + LDI_CPU_ITF_INTS, isr_s2);
		outp32(dpufd->dss_base + GLB_CPU_SDP_INTS, isr_s1);

		tmp = inp32(ldi_base + LDI_CPU_ITF_INT_MSK);
		outp32(ldi_base + LDI_CPU_ITF_INT_MSK, tmp);
	}
}
#endif
void hisi_mctl_ctl_clear(struct dpu_fb_data_type *dpufd, int mctl_idx)
{
	char __iomem *tmp_base = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return;
	}

	if ((mctl_idx < 0) || (mctl_idx >= DSS_MCTL_IDX_MAX)) {
		DPU_FB_ERR("mctl_idx=%d is invalid!\n", mctl_idx);
		return;
	}

	tmp_base = dpufd->dss_module.mctl_base[mctl_idx];
	if (tmp_base) {
		set_reg(tmp_base + MCTL_CTL_CLEAR, 0x1, 1, 0);
	}
}

#ifdef SUPPORT_CLR_CMDLIST_BY_HARDWARE
void hisi_cmdlist_config_reset(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, uint32_t cmdlist_idxs)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL point!\n");
	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);
	dpu_check_and_no_retval(!pov_req, ERR, "pov_req is NULL point!\n");
	dpufb_mctl_sw_clr(dpufd, pov_req, cmdlist_idxs);
}
#endif


int hisi_cmdlist_config_stop(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_pre_idxs)
{
	dss_overlay_t *pov_req = NULL;
	char __iomem *cmdlist_base = NULL;
	uint32_t i;
	uint32_t tmp;
	uint32_t offset;
	uint32_t cmdlist_chnidx;
	int ret = 0;
	union cmd_flag flag = {
		.bits.pending = 0,
		.bits.task_end = 1,
		.bits.last = 1,
	};

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	pov_req = &(dpufd->ov_req);

	if ((pov_req->ovl_idx < 0) || pov_req->ovl_idx >= DSS_OVL_IDX_MAX) {
		DPU_FB_ERR("fb%d, invalid ovl_idx=%d!\n", dpufd->index, pov_req->ovl_idx);
		goto err_return;
	}

	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	offset = 0x40;
	/* remove prev chn cmdlist */
	ret = hisi_cmdlist_add_new_node(dpufd, cmdlist_pre_idxs, flag, 1);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, hisi_cmdlist_add_new_node err:%d\n", dpufd->index, ret);
		goto err_return;
	}

	for (i = 0; i < DSS_WCHN_W0; i++) {
		dpufd->cmdlist_idx = hisi_get_cmdlist_idx_by_chnidx(i);
		tmp = (0x1 << (uint32_t)(dpufd->cmdlist_idx));

		if ((cmdlist_pre_idxs & tmp) == tmp) {
			dpufd->set_reg(dpufd, dpufd->dss_module.mctl_base[pov_req->ovl_idx] +
				MCTL_CTL_MUTEX_RCH0 + i * 0x4, 0, 32, 0);
			cmdlist_chnidx = hisi_get_cmdlist_chnidx(dpufd->cmdlist_idx);
			dpufd->set_reg(dpufd, cmdlist_base + CMDLIST_CH0_CTRL + cmdlist_chnidx * offset, 0x6, 3, 2);
		}
	}

	if ((cmdlist_pre_idxs & (0x1 << DSS_CMDLIST_V2)) == (0x1 << DSS_CMDLIST_V2)) {
		dpufd->cmdlist_idx = DSS_CMDLIST_V2;
		dpufd->set_reg(dpufd, dpufd->dss_module.mctl_base[pov_req->ovl_idx] +
			MCTL_CTL_MUTEX_RCH8, 0, 32, 0);
		cmdlist_chnidx = hisi_get_cmdlist_chnidx(dpufd->cmdlist_idx);
		dpufd->set_reg(dpufd, cmdlist_base + CMDLIST_CH0_CTRL + cmdlist_chnidx * offset, 0x6, 3, 2);
	}

	return 0;

err_return:
	return ret;
}

void hisi_dss_cmdlist_qos_on(struct dpu_fb_data_type *dpufd)
{
	char __iomem *cmdlist_base = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	cmdlist_base = dpufd->dss_base + DSS_CMDLIST_OFFSET;
	set_reg(cmdlist_base + CMDLIST_CTRL, 0x3, 2, 4);
}

void hisi_dump_cmdlist_node_items(struct cmd_item *item, uint32_t count)
{
	uint32_t index;
	uint32_t addr;

	for (index = 0; index < count; index++) {
		addr = item[index].reg_addr.bits.add0;
		addr = addr & CMDLIST_ADDR_OFFSET;
		addr = addr << 2;
		DPU_FB_INFO("set addr:0x%x value:0x%x add1:0x%x value:0x%x add2:0x%x value:0x%x\n",
			addr, item[index].data0,
			item[index].reg_addr.bits.add1 << 2, item[index].data1,
			item[index].reg_addr.bits.add2 << 2, item[index].data2);
	}
}

static void hisi_dump_cmdlist_content(struct list_head *cmdlist_head, char *filename,
	int length_filename, uint32_t addr)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;

	(void)length_filename;
	if (!cmdlist_head) {
		DPU_FB_ERR("cmdlist_head is NULL\n");
		return;
	}
	if (!filename) {
		DPU_FB_ERR("filename is NULL\n");
		return;
	}

	if (g_dump_cmdlist_content == 0)
		return;

	DPU_FB_INFO("%s\n", filename);

	list_for_each_entry_safe(node, _node_, cmdlist_head, list_node) {
		if (node->header_phys == addr)
			dpufb_save_file(filename, (char *)(node->list_header), CMDLIST_HEADER_LEN);

		if (node->item_phys == addr)
			dpufb_save_file(filename, (char *)(node->list_item), CMDLIST_ITEM_LEN);
	}
}

static void hisi_dump_cmdlist_one_node(struct list_head *cmdlist_head, uint32_t cmdlist_idx)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;
	uint32_t count = 0;
	int i = 0;
	char filename[256] = {0};  /* the patch of dssdump.txt */

	if (!cmdlist_head) {
		DPU_FB_ERR("cmdlist_head is NULL\n");
		return;
	}

	list_for_each_entry_safe(node, _node_, cmdlist_head, list_node) {
		if (node->node_type == CMDLIST_NODE_NOP)
			DPU_FB_INFO("node type = NOP node\n");
		else if (node->node_type == CMDLIST_NODE_FRAME)
			DPU_FB_INFO("node type = Frame node\n");

		DPU_FB_INFO("\t qos  | flag | pending | tast_end | last  | event_list | list_addr | "
			"next_list  | count | id | is_used | reserved | cmdlist_idx\n");
		DPU_FB_INFO("\t------+---------+------------+------------+------------+------------\n");
		DPU_FB_INFO("\t 0x%2x | 0x%2x |0x%6x | 0x%5x | 0x%3x | 0x%8x | 0x%8x | 0x%8x | "
			"0x%3x | 0x%2x | 0x%2x | 0x%2x | 0x%2x\n",
			node->list_header->flag.bits.qos, node->list_header->flag.bits.valid_flag,
			node->list_header->flag.bits.pending, node->list_header->flag.bits.task_end,
			node->list_header->flag.bits.last, node->list_header->flag.bits.event_list,
			node->list_header->list_addr, node->list_header->next_list,
			node->list_header->total_items.bits.count, node->list_header->flag.bits.id,
			node->is_used, node->reserved, cmdlist_idx);

		if (i == 0) {
			snprintf(filename, 256, "/data/dssdump/list_start_0x%x.txt", (uint32_t)node->header_phys);
			hisi_dump_cmdlist_content(cmdlist_head, filename, ARRAY_SIZE(filename), node->header_phys);
		}

		count = node->list_header->total_items.bits.count;
		hisi_dump_cmdlist_node_items(node->list_item, count);

		i++;
	}
}

int hisi_cmdlist_dump_all_node(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	uint32_t cmdlist_idxs)
{
	int i;
	uint32_t cmdlist_idxs_temp;
	uint32_t wb_compose_type = 0;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (pov_req) {
		if (pov_req->wb_enable)
			wb_compose_type = pov_req->wb_compose_type;
	}

	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if (0x1 == (cmdlist_idxs_temp & 0x1)) {
			if (pov_req && pov_req->wb_enable) {
				if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
					if (dpufd->media_common_cmdlist_data)
						hisi_dump_cmdlist_one_node(
							&(dpufd->media_common_cmdlist_data->cmdlist_head_temp[i]), i);
				} else {
					if (dpufd->cmdlist_data_tmp[wb_compose_type])
						hisi_dump_cmdlist_one_node(
							&(dpufd->cmdlist_data_tmp[wb_compose_type]->cmdlist_head_temp[i]), i);
				}
			} else {
				if (dpufd->cmdlist_data)
					hisi_dump_cmdlist_one_node(&(dpufd->cmdlist_data->cmdlist_head_temp[i]), i);
			}
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

static void dpufb_write_file_for_mdfx(struct file *fd, const char *buf, uint32_t buf_len)
{
	loff_t pos = 0;

	if (!fd) {
		DPU_FB_ERR("fd is NULL\n");
		return;
	}
	if (!buf) {
		DPU_FB_ERR("buf is NULL\n");
		return;
	}

	(void)vfs_write(fd, (char __user *)buf, buf_len, &pos);
}

static void hisi_dump_cmdlist_content_for_mdfx(struct dss_cmdlist_node *node, struct file *fd)
{
	char tmp_str[32] = {0};
	char newline_str[] = "\n";
	int ret;

	if (!node) {
		DPU_FB_ERR("node is NULL\n");
		return;
	}

	if (!fd) {
		DPU_FB_ERR("fd is NULL\n");
		return;
	}

	ret = snprintf(tmp_str, sizeof(tmp_str), "header address: 0x%x:\n", (uint32_t)node->header_phys);
	if (ret > 0)
		dpufb_write_file_for_mdfx(fd, tmp_str, strlen(tmp_str));
	dpufb_write_file_for_mdfx(fd, (char *)(node->list_header), CMDLIST_HEADER_LEN);
	dpufb_write_file_for_mdfx(fd, newline_str, strlen(newline_str));
	ret = snprintf(tmp_str, sizeof(tmp_str), "item address: 0x%x:\n", (uint32_t)node->item_phys);
	if (ret > 0)
		dpufb_write_file_for_mdfx(fd, tmp_str, strlen(tmp_str));
	dpufb_write_file_for_mdfx(fd, (char *)(node->list_item), CMDLIST_ITEM_LEN);
	dpufb_write_file_for_mdfx(fd, newline_str, strlen(newline_str));
}

static void hisi_dump_cmdlist_one_node_for_mdfx(struct list_head *cmdlist_head, uint32_t cmdlist_idx, struct file *fd)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;
	uint32_t count;
	char tmp_str[32] = {0};
	int ret;

	if (!cmdlist_head) {
		DPU_FB_ERR("cmdlist_head is NULL\n");
		return;
	}

	if (!fd) {
		DPU_FB_ERR("fd is NULL\n");
		return;
	}

	ret = snprintf(tmp_str, sizeof(tmp_str), "\ncmdlist_idx: %u:\n", cmdlist_idx);
	if (ret > 0)
		dpufb_write_file_for_mdfx(fd, tmp_str, strlen(tmp_str));
	list_for_each_entry_safe(node, _node_, cmdlist_head, list_node) {
		if (node->node_type == CMDLIST_NODE_NOP)
			DPU_FB_INFO("node type = NOP node\n");
		else if (node->node_type == CMDLIST_NODE_FRAME)
			DPU_FB_INFO("node type = Frame node\n");

		DPU_FB_INFO("\t qos  | flag | pending | tast_end | last  | event_list | list_addr | "
			"next_list  | count | id | is_used | reserved | cmdlist_idx\n");
		DPU_FB_INFO("\t------+---------+------------+------------+------------+------------\n");
		DPU_FB_INFO("\t 0x%2x | 0x%2x |0x%6x | 0x%5x | 0x%3x | 0x%8x | 0x%8x | 0x%8x | "
			"0x%3x | 0x%2x | 0x%2x | 0x%2x | 0x%2x\n",
			node->list_header->flag.bits.qos, node->list_header->flag.bits.valid_flag,
			node->list_header->flag.bits.pending, node->list_header->flag.bits.task_end,
			node->list_header->flag.bits.last, node->list_header->flag.bits.event_list,
			node->list_header->list_addr, node->list_header->next_list,
			node->list_header->total_items.bits.count, node->list_header->flag.bits.id,
			node->is_used, node->reserved, cmdlist_idx);

		hisi_dump_cmdlist_content_for_mdfx(node, fd);

		count = node->list_header->total_items.bits.count;
		hisi_dump_cmdlist_node_items(node->list_item, count);
	}
}

int hisi_cmdlist_dump_all_node_for_mdfx(struct dpu_fb_data_type *dpufd, uint32_t cmdlist_idxs, struct file *fd)
{
	int i;
	uint32_t cmdlist_idxs_temp;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (!fd) {
		DPU_FB_ERR("fd is NULL\n");
		return -EINVAL;
	}

	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1)
			hisi_dump_cmdlist_one_node_for_mdfx(&(dpufd->cmdlist_data->cmdlist_head_temp[i]), i, fd);
		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

int hisi_cmdlist_del_node(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	uint32_t cmdlist_idxs)
{
	int i;
	uint32_t wb_compose_type = 0;
	uint32_t cmdlist_idxs_temp;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (pov_req) {
		if (pov_req->wb_enable)
			wb_compose_type = pov_req->wb_compose_type;
	}

	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			if (pov_req && pov_req->wb_enable) {
				if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
					if (dpufd->media_common_cmdlist_data)
						hisi_cmdlist_del_all_node(
							&(dpufd->media_common_cmdlist_data->cmdlist_head_temp[i]));
				} else {
					if (dpufd->cmdlist_data_tmp[wb_compose_type])
						hisi_cmdlist_del_all_node(
							&(dpufd->cmdlist_data_tmp[wb_compose_type]->cmdlist_head_temp[i]));
				}
			} else {
				if (dpufd->cmdlist_data)
					hisi_cmdlist_del_all_node(&(dpufd->cmdlist_data->cmdlist_head_temp[i]));
			}
		}

		cmdlist_idxs_temp = (cmdlist_idxs_temp >> 1);
	}

	return 0;
}

static void hisi_cmdlist_data_free(struct dpu_fb_data_type *dpufd, struct dss_cmdlist_data *cmdlist_data)
{
	int i;
	int j;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if (!cmdlist_data) {
		DPU_FB_ERR("cmdlist_data is NULL\n");
		return;
	}

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		for (j = 0; j < HISI_DSS_CMDLIST_NODE_MAX; j++) {
			hisi_cmdlist_node_free(dpufd, cmdlist_data->cmdlist_nodes_temp[i][j]);
			cmdlist_data->cmdlist_nodes_temp[i][j] = NULL;
		}
	}

	kfree(cmdlist_data);
	cmdlist_data = NULL;
}

static struct dss_cmdlist_data *hisi_cmdlist_data_alloc(struct dpu_fb_data_type *dpufd)
{
	int i;
	int j;
	struct dss_cmdlist_data *cmdlist_data = NULL;
	size_t header_len = roundup(CMDLIST_HEADER_LEN, PAGE_SIZE);
	size_t item_len = roundup(CMDLIST_ITEM_LEN, PAGE_SIZE);

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return NULL;
	}

	cmdlist_data = (struct dss_cmdlist_data *)kmalloc(sizeof(struct dss_cmdlist_data), GFP_ATOMIC);
	if (cmdlist_data) {
		memset(cmdlist_data, 0, sizeof(struct dss_cmdlist_data));
	} else {
		DPU_FB_ERR("failed to kmalloc cmdlist_data!\n");
		return NULL;
	}

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		INIT_LIST_HEAD(&(cmdlist_data->cmdlist_head_temp[i]));

		for (j = 0; j < HISI_DSS_CMDLIST_NODE_MAX; j++) {
			cmdlist_data->cmdlist_nodes_temp[i][j] = hisi_cmdlist_node_alloc(dpufd, header_len, item_len);
			if (cmdlist_data->cmdlist_nodes_temp[i][j] == NULL) {
				DPU_FB_ERR("failed to hisi_cmdlist_node_alloc!\n");
				hisi_cmdlist_data_free(dpufd, cmdlist_data);
				return NULL;
			}
		}
	}

	return cmdlist_data;
}

static struct dss_cmdlist_info *hisi_cmdlist_info_alloc(struct dpu_fb_data_type *dpufd)
{
	int i;
	struct dss_cmdlist_info *cmdlist_info = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return NULL;
	}

	cmdlist_info = (struct dss_cmdlist_info *)kmalloc(sizeof(struct dss_cmdlist_info), GFP_ATOMIC);
	if (cmdlist_info) {
		memset(cmdlist_info, 0, sizeof(struct dss_cmdlist_info));
	} else {
		DPU_FB_ERR("failed to kmalloc cmdlist_info!\n");
		return NULL;
	}

	sema_init(&(cmdlist_info->cmdlist_wb_common_sem), 1);

	for (i = 0; i < WB_TYPE_MAX; i++) {
		sema_init(&(cmdlist_info->cmdlist_wb_sem[i]), 1);
		init_waitqueue_head(&(cmdlist_info->cmdlist_wb_wq[i]));
		cmdlist_info->cmdlist_wb_done[i] = 0;
		cmdlist_info->cmdlist_wb_flag[i] = 0;
	}

	return cmdlist_info;
}

static struct dss_media_common_info *hisi_media_common_info_alloc(struct dpu_fb_data_type *dpufd)
{
	struct dss_media_common_info *mdc_info = NULL;

	if (!dpufd)
		DPU_FB_ERR("dpufd is null.\n");

	mdc_info = (struct dss_media_common_info *)kmalloc(sizeof(struct dss_media_common_info), GFP_ATOMIC);
	if (mdc_info) {
		memset(mdc_info, 0, sizeof(struct dss_media_common_info));
	} else {
		DPU_FB_ERR("failed to kmalloc copybit_info!\n");
		return NULL;
	}

	init_waitqueue_head(&(mdc_info->mdc_wq));
	mdc_info->mdc_done = 0;

	return mdc_info;
}

void hisi_cmdlist_data_get_online(struct dpu_fb_data_type *dpufd)
{
	int tmp;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	tmp = dpufd->frame_count % HISI_DSS_CMDLIST_DATA_MAX;
	dpufd->cmdlist_data = dpufd->cmdlist_data_tmp[tmp];
	hisi_cmdlist_del_node(dpufd, NULL, HISI_DSS_CMDLIST_IDXS_MAX);
}

void hisi_cmdlist_data_get_offline(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if (!pov_req) {
		DPU_FB_ERR("pov_req is NULL\n");
		return;
	}

	if (pov_req->wb_compose_type == DSS_WB_COMPOSE_COPYBIT)
		dpufd->cmdlist_data = dpufd->cmdlist_data_tmp[1];
	else
		dpufd->cmdlist_data = dpufd->cmdlist_data_tmp[0];

	if (!dpufd->cmdlist_data)
		DPU_FB_ERR("fb%d, cmdlist_data is NULL!\n", dpufd->index);
}

static void set_hisi_cmdlist_pool_size(struct dpu_fb_data_type *dpufd)
{
	int i;
	int j;
	int dss_cmdlist_max = HISI_DSS_CMDLIST_MAX;
	size_t one_cmdlist_pool_size = 0;


	for (i = 0; i < dss_cmdlist_max; i++) {
		for (j = 0; j < HISI_DSS_CMDLIST_NODE_MAX; j++)
			one_cmdlist_pool_size += (roundup(CMDLIST_HEADER_LEN, PAGE_SIZE) +
				roundup(CMDLIST_ITEM_LEN, PAGE_SIZE));
	}

	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		dpufd->sum_cmdlist_pool_size = 2 * one_cmdlist_pool_size;
	} else if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		dpufd->sum_cmdlist_pool_size = one_cmdlist_pool_size;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX && !dpufd->panel_info.fake_external) {
		dpufd->sum_cmdlist_pool_size = HISI_DSS_CMDLIST_DATA_MAX * one_cmdlist_pool_size;
	} else {
		if (dpufd->index == PRIMARY_PANEL_IDX) {
			for (i = 0; i < HISI_DSS_CMDLIST_DATA_MAX; i++)
				dpufd->sum_cmdlist_pool_size += one_cmdlist_pool_size;
		}
	}
}

static int hisi_cmdlist_pool_init(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	size_t tmp = 0;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");
	dpu_check_and_return(!dpufd->pdev, -EINVAL, ERR, "dpufd is NULL!\n");

	set_hisi_cmdlist_pool_size(dpufd);
	if (dpufd->sum_cmdlist_pool_size == 0)
		return 0;

	dpufd->sum_cmdlist_pool_size = roundup(dpufd->sum_cmdlist_pool_size, PAGE_SIZE);
	if (hisi_dss_alloc_cmdlist_buffer(dpufd)) {
		DPU_FB_ERR("dma alloc coherent cmdlist buffer failed!\n");
		return -ENOMEM;
	}

	DPU_FB_INFO("fb%d, sum_cmdlist_pool_size=%zu, tmp=%zu.\n", dpufd->index, dpufd->sum_cmdlist_pool_size, tmp);

	/* create cmdlist pool */
	dpufd->cmdlist_pool = gen_pool_create(PAGE_SHIFT, -1);
	if (!dpufd->cmdlist_pool) {
		DPU_FB_ERR("fb%d, cmdlist_pool gen_pool_create failed!", dpufd->index);
		ret = -ENOMEM;
		goto err_pool_create;
	}

	if (gen_pool_add_virt(dpufd->cmdlist_pool, (unsigned long)(uintptr_t)dpufd->cmdlist_pool_vir_addr,
			dpufd->cmdlist_pool_phy_addr, dpufd->sum_cmdlist_pool_size, -1) != 0) {
		DPU_FB_ERR("fb%d, cmdlist_pool gen_pool_add failed!\n", dpufd->index);
		goto err_pool_add;
	}

	return 0;

err_pool_add:
	if (dpufd->cmdlist_pool) {
		gen_pool_destroy(dpufd->cmdlist_pool);
		dpufd->cmdlist_pool = NULL;
	}
err_pool_create:
	hisi_dss_free_cmdlist_buffer(dpufd);
	return ret;
}

static void hisi_cmdlist_pool_deinit(struct dpu_fb_data_type *dpufd)
{
	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	if (dpufd->cmdlist_pool) {
		gen_pool_destroy(dpufd->cmdlist_pool);
		dpufd->cmdlist_pool = NULL;
	}

	hisi_dss_free_cmdlist_buffer(dpufd);
}

static void deinit_hisi_ov_block_rects(struct dpu_fb_data_type *dpufd)
{
	int i;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL!\n");
		return;
	}

	for (i = 0; i < HISI_DSS_CMDLIST_BLOCK_MAX; i++) {
		if (dpufd->ov_block_rects[i]) {
			kfree(dpufd->ov_block_rects[i]);
			dpufd->ov_block_rects[i] = NULL;
		}
	}
}

int hisi_cmdlist_init(struct dpu_fb_data_type *dpufd)
{
	int ret;
	int i;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	ret = hisi_cmdlist_pool_init(dpufd);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, hisi_cmdlist_pool_init failed!\n", dpufd->index);
		return -EINVAL;
	}

	for (i = 0; i < HISI_DSS_CMDLIST_BLOCK_MAX; i++) {
		dpufd->ov_block_rects[i] = (dss_rect_t *)kmalloc(sizeof(dss_rect_t), GFP_ATOMIC);
		if (!dpufd->ov_block_rects[i]) {
			DPU_FB_ERR("ov_block_rects[%d] failed to alloc!\n", i);
			goto err_deint;
		}
	}

	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		dpufd->cmdlist_data_tmp[0] = hisi_cmdlist_data_alloc(dpufd);
		dpufd->cmdlist_data_tmp[1] = hisi_cmdlist_data_alloc(dpufd);
		dpufd->cmdlist_info = hisi_cmdlist_info_alloc(dpufd);
		if (!dpufd->cmdlist_data_tmp[0] || !dpufd->cmdlist_data_tmp[1] || !dpufd->cmdlist_info)
			goto err_deint;
	} else if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		dpufd->media_common_cmdlist_data = hisi_cmdlist_data_alloc(dpufd);
		dpufd->media_common_info = hisi_media_common_info_alloc(dpufd);
		if (!dpufd->media_common_cmdlist_data || !dpufd->media_common_info)
			goto err_deint;
	} else {
		if (dpufd->index == PRIMARY_PANEL_IDX ||
			(dpufd->index == EXTERNAL_PANEL_IDX && !dpufd->panel_info.fake_external)) {
			for (i = 0; i < HISI_DSS_CMDLIST_DATA_MAX; i++) {
				dpufd->cmdlist_data_tmp[i] = hisi_cmdlist_data_alloc(dpufd);
				if (!dpufd->cmdlist_data_tmp[i])
					goto err_deint;
			}
		}
	}

	dpufd->cmdlist_data = dpufd->cmdlist_data_tmp[0];
	dpufd->cmdlist_idx = -1;

	return 0;

err_deint:
	hisi_cmdlist_deinit(dpufd);
	return -EINVAL;
}

int hisi_cmdlist_deinit(struct dpu_fb_data_type *dpufd)
{
	int i = 0;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");

	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		if (dpufd->cmdlist_info) {
			kfree(dpufd->cmdlist_info);
			dpufd->cmdlist_info = NULL;
		}

		if (dpufd->copybit_info) {
			kfree(dpufd->copybit_info);
			dpufd->copybit_info = NULL;
		}

		for (i = 0; i < HISI_DSS_CMDLIST_DATA_MAX; i++) {
			if (dpufd->cmdlist_data_tmp[i]) {
				hisi_cmdlist_data_free(dpufd, dpufd->cmdlist_data_tmp[i]);
				dpufd->cmdlist_data_tmp[i] = NULL;
			}
		}
	} else if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		if (dpufd->media_common_cmdlist_data) {
			kfree(dpufd->media_common_cmdlist_data);
			dpufd->media_common_cmdlist_data = NULL;
		}

		if (dpufd->media_common_info) {
			kfree(dpufd->media_common_info);
			dpufd->media_common_info = NULL;
		}
	} else {
		if (dpufd->index == PRIMARY_PANEL_IDX ||
			(dpufd->index == EXTERNAL_PANEL_IDX && !dpufd->panel_info.fake_external)) {
			for (i = 0; i < HISI_DSS_CMDLIST_DATA_MAX; i++) {
				if (dpufd->cmdlist_data_tmp[i]) {
					hisi_cmdlist_data_free(dpufd, dpufd->cmdlist_data_tmp[i]);
					dpufd->cmdlist_data_tmp[i] = NULL;
				}
			}
		}
	}

	deinit_hisi_ov_block_rects(dpufd);
	hisi_cmdlist_pool_deinit(dpufd);

	return 0;
}
#pragma GCC diagnostic pop

