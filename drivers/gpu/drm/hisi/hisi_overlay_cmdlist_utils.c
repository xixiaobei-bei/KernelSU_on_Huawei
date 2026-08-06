/*
 * Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 */
/*lint -e679 -e701 -e712 -e734 -e737 -e826 -e838 -e574 -e648 -e570 -e587 -e559 -e578 -e548 -e529*/
#include <securec.h>

#include "hisi_drm_drv.h"
#include "hisi_overlay_cmdlist_utils.h"
#include "hisi_drm_dpe_utils.h"
#include "hisi_drm_dsi.h"
#include "hisi_panel.h"
#include "hisi_dss_iommu.h"
#include "hisi_mipi_dsi.h"
#include "hisi_drm_dss_global.h"
#include "hisi_defs.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

#define MAX_ITEM_OFFSET     (0x3F)
#define CMDLIST_ADDR_OFFSET (0x3FFFF)

#define CMDLIST_HEADER_LEN (SZ_1K)
#define CMDLIST_ITEM_LEN   (SZ_8K)
#define MAX_ITEM_INDEX     (SZ_1K)

#define HISI_DSS_COMPOSER_HOLD_TIME (1000UL * 3600 * 24 * 5)

struct dss_cmdlist_data *g_cmdlist_data;
uint32_t g_online_cmdlist_idxs;
uint32_t g_offline_cmdlist_idxs;
/* DEBUG */
int g_debug_ovl_cmdlist;
uint32_t g_dss_version_tag;
int g_debug_ovl_offline_composer_hold;
int g_debug_set_reg_val;
int g_ldi_data_gate_en = 1;
int g_dump_cmdlist_content;
int g_enable_ovl_cmdlist_online;

/*
 * data0: addr0[17:0]
 * data1: addr0[17:0] + addr1[5:0]
 * data2: addr0[17:0] + addr2[5:0]
 *
 * cnt[1:0]:
 * 2'b00:	reg0
 * 2'b01: reg0, reg1
 * 2'b10: reg0, reg1, reg2
 * 2'b11: ((inp32(addr0) & data1) | data2) -> addr0
 */
bool hisi_check_cmdlist_paremeters_validate(uint8_t bw, uint8_t bs)
{
	if ((bs > 32) || (bw > 32) || ((bw + bs) > 32)) {
		HISI_DRM_ERR("Can't do this,which may cause overflow");
		return false;
	}
	return true;
}

static void set_cmdlist_node(uint8_t bw, struct dss_cmdlist_node *node,
	int *index, uint8_t bs, uint32_t value)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
	uint64_t mask = ((uint64_t)1 << bw) - 1; //lint !e647

	uint32_t new_addr = 0;
	uint32_t old_addr;
	int condition;

	new_addr = (new_addr >> 2) & CMDLIST_ADDR_OFFSET;
	old_addr = node->list_item[*index].reg_addr.ul32 & CMDLIST_ADDR_OFFSET;
	condition = (((new_addr - old_addr) < MAX_ITEM_OFFSET) && (new_addr >= old_addr));
	if (bw != 32) {
		if (node->item_flag != 0)
			*index++;

		node->list_item[*index].reg_addr.bits.add0 = new_addr;
		node->list_item[*index].data0 = value;
		node->list_item[*index].data1 = (uint32_t) ((~(mask << bs)) & 0xFFFFFFFF);
		node->list_item[*index].data2 = (uint32_t) (((mask & value) << bs) & 0xFFFFFFFF);
		node->list_item[*index].reg_addr.bits.cnt = 3;
		node->item_flag = 3;
	} else {
		if (node->item_flag == 0) {
			node->list_item[*index].reg_addr.bits.add0 = new_addr;
			node->list_item[*index].data0 = value;
			node->list_item[*index].reg_addr.bits.cnt = 0;
			node->item_flag = 1;
		} else if (node->item_flag == 1 && condition) {
			node->list_item[*index].reg_addr.bits.add1 = new_addr - old_addr;
			node->list_item[*index].data1 = value;
			node->list_item[*index].reg_addr.bits.cnt = 1;
			node->item_flag = 2;
		} else if (node->item_flag == 2 && condition) {
			node->list_item[*index].reg_addr.bits.add2 = new_addr - old_addr;
			node->list_item[*index].data2 = value;
			node->list_item[*index].reg_addr.bits.cnt = 2;
			node->item_flag = 3;
		} else {
			(*index)++;
			node->list_item[*index].reg_addr.bits.add0 = new_addr;
			node->list_item[*index].data0 = value;
			node->list_item[*index].reg_addr.bits.cnt = 0;
			node->item_flag = 1;
		}
	}
}

void hisi_cmdlist_set_reg(struct hisi_drm_crtc *hisi_crtc, void __iomem *addr,
	uint32_t value, uint8_t bw, uint8_t bs)
{
	if (!hisi_check_cmdlist_paremeters_validate(bw, bs))
		return;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic pop
	struct dss_cmdlist_node *node = NULL;
	int cmdlist_idx = -1;
	int index = 0;
	struct hisi_cmdlist *cmdlist_info = NULL;

	if (addr == NULL) {
		HISI_DRM_ERR("addr is NULL");
		return;
	}
	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return;
	}
	cmdlist_info = &hisi_crtc->cmdlist_info;

	cmdlist_idx = cmdlist_info->cmdlist_idx;
	if ((cmdlist_idx < 0) || (cmdlist_idx >= HISI_DSS_CMDLIST_MAX)) {
		HISI_DRM_ERR("cmdlist_idx is invalid");
		return;
	}

	node = list_entry(cmdlist_info->cmdlist_data->cmdlist_head_temp[cmdlist_idx].prev,
		struct dss_cmdlist_node, list_node);
	if (node == NULL) {
		HISI_DRM_ERR("node is NULL");
		return;
	}

	if (node->node_type == CMDLIST_NODE_NOP) {
		HISI_DRM_ERR("can't set register value to NOP node!");
		return;
	}

	index = node->item_index;
	set_cmdlist_node(bw, node, &index, bs, value);

	if (index >= MAX_ITEM_INDEX) {
		HISI_DRM_ERR("index=%d is too large(1k)!\n", index);
		return;
	}

	node->item_index = index;
	node->list_header->total_items.bits.count = node->item_index + 1;
}

/*
 * flush cache for cmdlist, make sure that
 * cmdlist has writen through to memory before config register
 */
void hisi_cmdlist_flush_cache(struct hisi_cmdlist *cmdlist_info, uint32_t cmdlist_idxs)
{
	uint32_t i;

	uint32_t cmdlist_idxs_temp;
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;
	struct list_head *cmdlist_heads = NULL;

	if (cmdlist_info == NULL) {
		HISI_DRM_ERR("cmdlist_info is NULL!\n");
		return;
	}

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			cmdlist_heads = &(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]);
			if (cmdlist_heads == NULL) {
				HISI_DRM_ERR("cmdlist_data is NULL!\n");
				continue;
			}

			list_for_each_entry_safe_reverse(node, _node_, cmdlist_heads, list_node) {
				if (node != NULL) {
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
/*lint -e413*/

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

static bool hisi_cmdlist_addr_check(struct hisi_cmdlist *cmdlist_info, uint32_t *list_addr)
{
	if ((*list_addr > (uint32_t)(cmdlist_info->cmdlist_pool_phy_addr + cmdlist_info->sum_cmdlist_pool_size))
		|| (*list_addr < (uint32_t)cmdlist_info->cmdlist_pool_phy_addr)) {
		HISI_DRM_ERR("cmdlist_addr is invalid, sum_cmdlist_pool_size=%zu.\n",
			cmdlist_info->sum_cmdlist_pool_size);
		*list_addr = cmdlist_info->cmdlist_pool_phy_addr;
		return false;
	}

	return true;
}

int hisi_cmdlist_add_nop_node(struct hisi_cmdlist *cmdlist_info, uint32_t cmdlist_idxs, int pending, int reserved)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *first_node = NULL;
	uint32_t cmdlist_idxs_temp;
	int i;
	int id = 0;

	if (cmdlist_info == NULL) {
		HISI_DRM_ERR("cmdlist_info is NULL");
		return -EINVAL;
	}

	if (!cmdlist_info->cmdlist_data) {
		HISI_DRM_ERR("cmdlist_data is NULL");
		return -EINVAL;
	}

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			node = hisi_cmdlist_get_free_node(cmdlist_info->cmdlist_data->cmdlist_nodes_temp[i], &id);
			if (node == NULL) {
				HISI_DRM_ERR("failed to hisi_get_free_cmdlist_node!\n");
				return -EINVAL;
			}

			node->list_header->flag.bits.id = id;
			node->list_header->flag.bits.nop = 0x1;
			node->list_header->flag.bits.pending = pending ? 0x1 : 0x0;
			node->list_header->flag.bits.valid_flag = CMDLIST_NODE_VALID;
			node->list_header->flag.bits.last = 0;
			node->list_header->next_list = node->header_phys;
			hisi_cmdlist_addr_check(cmdlist_info, &(node->list_header->next_list));

			node->is_used = 1;
			node->node_type = CMDLIST_NODE_NOP;
			node->reserved = reserved ? 0x1 : 0x0;

			/* add this nop to list */
			list_add_tail(&(node->list_node), &(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]));

			if (node->list_node.prev != &(cmdlist_info->cmdlist_data->cmdlist_head_temp[i])) {
				struct dss_cmdlist_node *pre_node = NULL;

				first_node = list_first_entry(&(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]),
					struct dss_cmdlist_node, list_node);
				pre_node = list_entry(node->list_node.prev, struct dss_cmdlist_node, list_node);
				pre_node->list_header->next_list = first_node->header_phys;
				hisi_cmdlist_addr_check(cmdlist_info, &(pre_node->list_header->next_list));

				if (node->list_header->flag.bits.pending == 0x1)
					pre_node->reserved = 0x0;

				pre_node->list_header->flag.bits.task_end = 0x1;
				node->list_header->flag.bits.task_end = 0x1;

				if (g_debug_ovl_cmdlist)
					HISI_DRM_DEBUG("i = %d, next_list = 0x%x\n", i, (uint32_t)(node->header_phys));
			}
		}

		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

static void cmdlist_header_info_config(struct dss_cmdlist_node *node,
	int task_end, int last, struct hisi_cmdlist *cmdlist_info)
{
	node->list_header->flag.bits.task_end = task_end ? 0x1 : 0x0;
	node->list_header->flag.bits.last = last ? 0x1 : 0x0;

	node->list_header->flag.bits.valid_flag = CMDLIST_NODE_VALID;
	node->list_header->flag.bits.exec = 0x1;
	node->list_header->list_addr = node->item_phys;
	node->list_header->next_list = node->header_phys;

	hisi_cmdlist_addr_check(cmdlist_info, &(node->list_header->list_addr));
	hisi_cmdlist_addr_check(cmdlist_info, &(node->list_header->next_list));
}

int hisi_cmdlist_add_new_node(struct hisi_cmdlist *cmdlist_info,
	uint32_t cmdlist_idxs, int pending, int task_end, int remove, int last, uint32_t wb_type)
{
	struct dss_cmdlist_node *node = NULL;
	uint32_t cmdlist_idxs_temp;
	int i;
	int id = 0;

	if (cmdlist_info == NULL) {
		HISI_DRM_ERR("ctx is NULL");
		return -EINVAL;
	}

	cmdlist_idxs_temp = cmdlist_idxs;

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			node = hisi_cmdlist_get_free_node(cmdlist_info->cmdlist_data->cmdlist_nodes_temp[i], &id);
			if (node == NULL || node->list_header == NULL) {
				HISI_DRM_ERR("failed to hisi_get_free_cmdnode!\n");
				return -EINVAL;
			}

			/* fill the header and item info */
			node->list_header->flag.bits.id = id;
			node->list_header->flag.bits.pending = pending ? 0x1 : 0x0;

			if (i < DSS_CMDLIST_W0)
				node->list_header->flag.bits.event_list = remove ? 0x8 : (0xE + i);
			else if (i < DSS_CMDLIST_OV0)
				node->list_header->flag.bits.event_list = remove ? 0x8 : (0x16 + i);
			else if (i == DSS_CMDLIST_V2)
				node->list_header->flag.bits.event_list = remove ? 0x8 : 0x16;
			else if (i == DSS_CMDLIST_W2)
				node->list_header->flag.bits.event_list = remove ? 0x8 : 0x20;
			else
				node->list_header->flag.bits.event_list = remove ? 0x8 : (0xE + i);

			cmdlist_header_info_config(node, task_end, last, cmdlist_info);

			node->is_used = 1;
			node->node_type = CMDLIST_NODE_FRAME;
			node->item_flag = 0;
			node->reserved = 0;

			/* add this nop to list */
			list_add_tail(&(node->list_node), &(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]));

			if (node->list_node.prev != &(cmdlist_info->cmdlist_data->cmdlist_head_temp[i])) {
				struct dss_cmdlist_node *pre_node = NULL;

				pre_node = list_entry(node->list_node.prev, struct dss_cmdlist_node, list_node);
				pre_node->list_header->next_list = node->header_phys;

				hisi_cmdlist_addr_check(cmdlist_info, &(pre_node->list_header->next_list));
				pre_node->reserved = 0x0;
				if (g_debug_ovl_cmdlist)
					HISI_DRM_INFO("i = %d, next_list = 0x%x\n",  i, (uint32_t)node->header_phys);
			}
		}

		cmdlist_idxs_temp  = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

static int hisi_cmdlist_del_all_node(struct list_head *cmdlist_heads)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;
	int ret;

	if (cmdlist_heads == NULL) {
		HISI_DRM_ERR("cmdlist_heads is NULL");
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

			ret = memset_s(node->list_item, CMDLIST_ITEM_LEN, 0, CMDLIST_ITEM_LEN);
			drm_check_and_return(ret != EOK, -EINVAL, ERR, "memset failed!");

			node->item_index = 0;
			node->item_flag = 0;
			node->node_type = CMDLIST_NODE_NONE;
			node->is_used = 0;
		}
	}

	return 0;
}

/*
 * start cmdlist.
 * it will set cmdlist into pending state.
 */

static int config_init(struct hisi_drm_crtc *hisi_crtc, int mctl_idx,
	char __iomem **cmdlist_base, struct hisi_cmdlist **cmdlist_info,
	struct dss_hw_ctx **ctx)
{
	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("acrct is NULL");
		return -EINVAL;
	}
	*ctx = hisi_crtc->ctx;

	if (*ctx == NULL) {
		HISI_DRM_ERR("ctx is NULL");
		return -EINVAL;
	}

	if ((mctl_idx < 0) || (mctl_idx >= DSS_MCTL_IDX_MAX)) {
		HISI_DRM_ERR("mctl_idx=%d is invalid", mctl_idx);
		return -EINVAL;
	}

	*cmdlist_base = (*ctx)->dss_base + DSS_CMDLIST_OFFSET;
	*cmdlist_info = &hisi_crtc->cmdlist_info;
	return 0;
}

static void get_cmdlist_node(int mctl_idx, struct dss_cmdlist_node **cmdlist_node,
	struct hisi_cmdlist *cmdlist_info, uint32_t wb_compose_type, int i)
{
	if (mctl_idx >= DSS_MCTL2) {
		*cmdlist_node =
			list_first_entry(&(cmdlist_info->cmdlist_data_tmp[wb_compose_type]->cmdlist_head_temp[i]),
			struct dss_cmdlist_node,
			list_node);
	} else {
		*cmdlist_node = list_first_entry(&(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]),
			struct dss_cmdlist_node, list_node);
	}

}

static void set_reg_based_on_mctl1(int mctl_idx, char __iomem *cmdlist_base,
	int i, uint32_t offset, uint32_t list_addr, uint32_t ints_temp)
{
	if (mctl_idx <= DSS_MCTL1)
		set_reg(cmdlist_base + CMDLIST_CH0_CTRL + i * offset, 0x1, 1, 6);
	else
		set_reg(cmdlist_base + CMDLIST_CH0_CTRL + i * offset, 0x0, 1, 6);

	set_reg(cmdlist_base + CMDLIST_CH0_STAAD + i * offset, list_addr, 32, 0);

	set_reg(cmdlist_base + CMDLIST_CH0_CTRL + i * offset, 0x1, 1, 0);
	if ((mctl_idx <= DSS_MCTL1) && ((ints_temp & 0x2) == 0x2))
		set_reg(cmdlist_base + CMDLIST_SWRST, 0x1, 1, i);
}

static void set_reg_based_on_mctl2(int mctl_idx, int status_temp,
	uint32_t ints_temp, char __iomem *cmdlist_base, int i)
{
	if (mctl_idx >= DSS_MCTL2) {
		if ((((uint32_t)status_temp & 0xF) == 0x0) || ((ints_temp & 0x2) == 0x2))
			set_reg(cmdlist_base + CMDLIST_SWRST, 0x1, 1, i);
		else
			HISI_DRM_INFO("i=%d, status_temp=0x%x, ints_temp=0x%x\n", i, status_temp, ints_temp);
	}
}

static void set_reg_by_cmdlist_idx(uint32_t *cmdlist_idxs_temp,
	uint32_t cmdlist_idxs, char __iomem *cmdlist_base, uint32_t offset,
	int mctl_idx)
{
	int i;

	*cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if (((*cmdlist_idxs_temp) & 0x1) == 0x1)
			set_reg(cmdlist_base + CMDLIST_CH0_CTRL + i * offset, mctl_idx, 3, 2);

		*cmdlist_idxs_temp = (*cmdlist_idxs_temp) >> 1;
	}
}

static void set_mtrl_base(int mctl_idx, struct dss_hw_ctx *ctx)
{
	char __iomem *mctl_base = NULL;

	mctl_base = ctx->dss_base + g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
	if (mctl_idx >= DSS_MCTL2) {
		set_reg(mctl_base + MCTL_CTL_ST_SEL, 0x1, 1, 0);
		set_reg(mctl_base + MCTL_CTL_SW_ST, 0x1, 1, 0);
	}
}

struct config_info {
	uint32_t offset;
	uint32_t list_addr;
	uint32_t cmdlist_idxs_temp;
	int status_temp;
	uint32_t ints_temp;
	uint32_t temp;
};

int hisi_cmdlist_config_start(struct hisi_drm_crtc *hisi_crtc,
	int mctl_idx, uint32_t cmdlist_idxs, uint32_t wb_compose_type)
{
	char __iomem *cmdlist_base = NULL;
	struct dss_cmdlist_node *cmdlist_node = NULL;
	struct config_info config_info;
	struct hisi_cmdlist *cmdlist_info = NULL;
	struct dss_hw_ctx *ctx = NULL;
	int ret;
	uint32_t i;

	config_info.temp = 0;
	ret = config_init(hisi_crtc, mctl_idx, &cmdlist_base, &cmdlist_info, &ctx);
	if (ret)
		return ret;

	config_info.offset = 0x40;
	config_info.cmdlist_idxs_temp = cmdlist_idxs;
	dsb(sy);

	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((config_info.cmdlist_idxs_temp & 0x1) == 0x1) {
			config_info.status_temp =  inp32(cmdlist_base + CMDLIST_CH0_STATUS + i * config_info.offset);
			config_info.ints_temp = inp32(cmdlist_base + CMDLIST_CH0_INTS + i * config_info.offset);

			get_cmdlist_node(mctl_idx, &cmdlist_node, cmdlist_info, wb_compose_type, i);
			config_info.list_addr = cmdlist_node->header_phys;
			if (g_debug_ovl_cmdlist)
				HISI_DRM_INFO("list_addr:0x%x, i=%d, ints_temp=0x%x, status_temp=0x%x\n",
					config_info.list_addr, i, config_info.ints_temp, config_info.status_temp);

			if (hisi_cmdlist_addr_check(cmdlist_info, &(config_info.list_addr)) == false)
				return -EINVAL;

			config_info.temp |= (1 << i);

			outp32(cmdlist_base + CMDLIST_ADDR_MASK_EN, BIT(i));
			if (g_debug_set_reg_val)
				HISI_DRM_INFO("writel: [%pK] = 0x%lx\n",
					cmdlist_base + CMDLIST_ADDR_MASK_EN, BIT(i));

			set_reg_based_on_mctl1(mctl_idx, cmdlist_base, i, config_info.offset, config_info.list_addr, config_info.ints_temp);
			set_reg_based_on_mctl2(mctl_idx, config_info.status_temp, config_info.ints_temp, cmdlist_base, i);
		}

		config_info.cmdlist_idxs_temp = config_info.cmdlist_idxs_temp >> 1;
	}

	outp32(cmdlist_base + CMDLIST_ADDR_MASK_DIS, config_info.temp);
	if (g_debug_set_reg_val)
		HISI_DRM_INFO("writel: [%pK] = 0x%x\n",
			cmdlist_base + CMDLIST_ADDR_MASK_DIS, config_info.temp);
	set_reg_by_cmdlist_idx(&(config_info.cmdlist_idxs_temp), cmdlist_idxs, cmdlist_base, config_info.offset, mctl_idx);
	set_mtrl_base(mctl_idx, ctx);

	return 0;
}

/*lint -e679 -e527*/
static bool hisifb_mctl_clear_ack_timeout(const char __iomem *mctl_base)
{
	uint32_t mctl_status;
	int delay_count = 0;
	bool is_timeout = false;

	while (1) {
		mctl_status = inp32(mctl_base + MCTL_CTL_STATUS);
		if (((mctl_status & 0x10) == 0) || (delay_count > 500)) {
			is_timeout = (delay_count > 500) ? true : false;
			delay_count = 0;
			break;
		}
		udelay(1);
		++delay_count;
	}

	return is_timeout;
}

static void hisifb_mctl_sw_clr(struct hisi_drm_crtc *hisi_crtc, int mctl_idx, uint32_t cmdlist_idxs)
{
	uint32_t i;
	char __iomem *mctl_base = NULL;
	struct dss_hw_ctx *ctx = NULL;

	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("dsi is NULL!\n");
		return;
	}

	ctx = hisi_crtc->ctx;
	set_reg(ctx->dss_base + DSS_CMDLIST_OFFSET + CMDLIST_SWRST, cmdlist_idxs, 32, 0);

	if ((mctl_idx < 0) || (mctl_idx >= DSS_MCTL_IDX_MAX)) {
		HISI_DRM_ERR("mctl_idx=%d is invalid", mctl_idx);
		return;
	}

	mctl_base = hisi_crtc->dss_module.mctl_base[mctl_idx];

	if (mctl_base) {
		set_reg(mctl_base + MCTL_CTL_CLEAR, 0x1, 1, 0);
		if (hisifb_mctl_clear_ack_timeout(mctl_base)) {
			HISI_DRM_ERR("clear_ack_timeout, mctl_status=0x%x !\n", inp32(mctl_base + MCTL_CTL_STATUS));
			for (i = 0; i < DSS_CHN_MAX_DEFINE; i++)
				HISI_DRM_ERR("chn%d: [DMA_BUF_DBG0]=0x%x [DMA_BUF_DBG1]=0x%x !\n", i,
					inp32(ctx->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG0),
					inp32(ctx->dss_base + g_dss_module_base[i][MODULE_DMA] + DMA_BUF_DBG1));
		}
	}
}

void hisi_cmdlist_config_reset(struct hisi_drm_crtc *hisi_crtc, uint32_t cmdlist_idxs, int ovl_idx)
{
	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("ctx is NULL point!\n");
		return;
	}
	hisifb_mctl_sw_clr(hisi_crtc, ovl_idx, cmdlist_idxs);
}

int hisi_cmdlist_config_stop(struct hisi_drm_crtc *hisi_crtc, uint32_t cmdlist_pre_idxs, int ovl_idx)
{
	char __iomem *cmdlist_base = NULL;
	struct hisi_cmdlist *cmdlist_info = NULL;
	uint32_t i;
	uint32_t tmp;
	uint32_t offset;

	int ret;

	if (hisi_crtc == NULL) {
		HISI_DRM_ERR("hisi_crtc is NULL");
		return -EINVAL;
	}

	if ((ovl_idx < 0) ||
		ovl_idx >= DSS_OVL_IDX_MAX) {
		HISI_DRM_ERR("invalid ovl_idx=%d!", ovl_idx);
		goto err_return;
	}

	cmdlist_info = &(hisi_crtc->cmdlist_info);
	cmdlist_base = hisi_crtc->ctx->dss_base + DSS_CMDLIST_OFFSET;
	offset = 0x40;
	/* remove prev chn cmdlist */
	ret = hisi_cmdlist_add_new_node(cmdlist_info, cmdlist_pre_idxs, 0, 1, 1, 1, 0);
	if (ret != 0) {
		HISI_DRM_ERR("hisi_cmdlist_add_new_node err:%d\n", ret);
		goto err_return;
	}

	for (i = 0; i < DSS_WCHN_W0; i++) {
		tmp = (0x1 << i);
		cmdlist_info->cmdlist_idx = i;

		if ((cmdlist_pre_idxs & tmp) == tmp) {
			hisi_set_reg(hisi_crtc, hisi_crtc->dss_module.mctl_base[ovl_idx] +
				MCTL_CTL_MUTEX_RCH0 + i * 0x4, 0, 32, 0);
			hisi_set_reg(hisi_crtc, cmdlist_base + CMDLIST_CH0_CTRL + i * offset, 0x6, 3, 2);
		}
	}

	if ((cmdlist_pre_idxs & (0x1 << DSS_CMDLIST_V2)) == (0x1 << DSS_CMDLIST_V2)) {
		cmdlist_info->cmdlist_idx = DSS_CMDLIST_V2;
		hisi_set_reg(hisi_crtc, hisi_crtc->dss_module.mctl_base[ovl_idx] +
			MCTL_CTL_MUTEX_RCH8, 0, 32, 0);
		hisi_set_reg(hisi_crtc, cmdlist_base + CMDLIST_CH0_CTRL + DSS_CMDLIST_V2 * offset, 0x6, 3, 2);
	}

	return 0;

err_return:
	return ret;
}
/*lint +e679 +e527*/

void hisi_dss_cmdlist_qos_on(char __iomem *dss_base)
{
	char __iomem *cmdlist_base = NULL;

	if (dss_base == NULL) {
		HISI_DRM_ERR("dss_base is NULL");
		return;
	}

	cmdlist_base = dss_base + DSS_CMDLIST_OFFSET;
	set_reg(cmdlist_base + CMDLIST_CTRL, 0x3, 2, 4);
}

static void hisi_dump_cmdlist_node_items(struct cmd_item *item, uint32_t count)
{
	uint32_t index;
	uint32_t addr;

	for (index = 0; index < count; index++) {
		addr = item[index].reg_addr.bits.add0;
		addr = addr & CMDLIST_ADDR_OFFSET;
		addr = addr << 2;
		HISI_DRM_INFO("set addr:0x%x value:0x%x add1:0x%x value:0x%x add2:0x%x value:0x%x",
			addr, item[index].data0,
			item[index].reg_addr.bits.add1 << 2, item[index].data1,
			item[index].reg_addr.bits.add2 << 2, item[index].data2);
	}
}

static void hisi_dump_cmdlist_one_node(struct list_head *cmdlist_head, uint32_t cmdlist_idx)
{
	struct dss_cmdlist_node *node = NULL;
	struct dss_cmdlist_node *_node_ = NULL;
	uint32_t count = 0;
	int i = 0;

	if (cmdlist_head == NULL) {
		HISI_DRM_ERR("cmdlist_head is NULL");
		return;
	}

	list_for_each_entry_safe(node, _node_, cmdlist_head, list_node) {
		if (node->node_type == CMDLIST_NODE_NOP)
			HISI_DRM_INFO("node type = NOP node\n");
		else if (node->node_type == CMDLIST_NODE_FRAME)
			HISI_DRM_INFO("node type = Frame node\n");

		HISI_DRM_INFO("\t qos  | flag | pending | tast_end | last  | event_list | list_addr | next_list | count | id | is_used | reserved | cmdlist_idx | header_phys\n");
		HISI_DRM_INFO("\t------+---------+------------+------------+------------+------------\n");
		HISI_DRM_INFO("\t 0x%2x | 0x%2x |0x%6x | 0x%5x | 0x%3x | 0x%8x | 0x%8x | 0x%8x | 0x%3x | 0x%2x | 0x%2x | 0x%2x | 0x%2x | 0x%8x\n",
			node->list_header->flag.bits.qos, node->list_header->flag.bits.valid_flag,
			node->list_header->flag.bits.pending,
			node->list_header->flag.bits.task_end, node->list_header->flag.bits.last,
			node->list_header->flag.bits.event_list,
			node->list_header->list_addr, node->list_header->next_list,
			node->list_header->total_items.bits.count, node->list_header->flag.bits.id,
			node->is_used, node->reserved, cmdlist_idx, node->header_phys);

		count = node->list_header->total_items.bits.count;
		hisi_dump_cmdlist_node_items(node->list_item, count);

		i++;
	}
}

int hisi_cmdlist_dump_all_node(struct hisi_cmdlist *cmdlist_info, uint32_t cmdlist_idxs)
{
	int i;
	uint32_t cmdlist_idxs_temp;

	if (cmdlist_info == NULL) {
		HISI_DRM_ERR("cmdlist_info is NULL");
		return -EINVAL;
	}
	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if (0x1 == (cmdlist_idxs_temp & 0x1))
			hisi_dump_cmdlist_one_node(&(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]), i);
		cmdlist_idxs_temp = cmdlist_idxs_temp >> 1;
	}

	return 0;
}

int hisi_cmdlist_del_node(struct hisi_cmdlist *cmdlist_info, uint32_t cmdlist_idxs)
{
	int i = 0;
	uint32_t cmdlist_idxs_temp = 0;

	if (cmdlist_info == NULL) {
		HISI_DRM_ERR("hisifd is NULL");
		return -EINVAL;
	}

	cmdlist_idxs_temp = cmdlist_idxs;
	for (i = 0; i < HISI_DSS_CMDLIST_MAX; i++) {
		if ((cmdlist_idxs_temp & 0x1) == 0x1) {
			if (cmdlist_info->cmdlist_data != NULL)
				hisi_cmdlist_del_all_node(&(cmdlist_info->cmdlist_data->cmdlist_head_temp[i]));
		}
		cmdlist_idxs_temp = (cmdlist_idxs_temp >> 1);
	}

	return 0;
}

void hisi_cmdlist_data_get_online(struct hisi_cmdlist *cmdlist_info)
{
	if (cmdlist_info == NULL) {
		HISI_DRM_ERR("cmdlist_info is NULL");
		return;
	}

	cmdlist_info->cmdlist_data_idx = (cmdlist_info->cmdlist_data_idx + 1) % HISI_DSS_CMDLIST_DATA_MAX;
	cmdlist_info->cmdlist_data = cmdlist_info->cmdlist_data_tmp[cmdlist_info->cmdlist_data_idx];
	hisi_cmdlist_del_node(cmdlist_info, HISI_DSS_CMDLIST_IDXS_MAX);
	cmdlist_info->cmdlist_prev_idxs_mask = cmdlist_info->cmdlist_idxs_mask;
	cmdlist_info->cmdlist_idxs_mask = 0;
}

int get_cmdlist_idx(struct hisi_cmdlist *cmdlist_info, int chn_idx)
{
	int tmp;

	if (!cmdlist_info) {
		HISI_DRM_ERR("cmdlist_info is NULL");
		return -EINVAL;
	}

	if (chn_idx == DSS_RCHN_V2) {
		tmp = (0x1 << DSS_CMDLIST_V2);
		cmdlist_info->cmdlist_idx = DSS_CMDLIST_V2;
	} else if (chn_idx == DSS_WCHN_W2) {
		tmp = (0x1 << DSS_CMDLIST_W2);
		cmdlist_info->cmdlist_idx = DSS_CMDLIST_W2;
	} else {
		tmp = (0x1 << (uint32_t)chn_idx);
		cmdlist_info->cmdlist_idx = chn_idx;
	}
	return tmp;
}

int hisi_cmdlist_create_node(struct hisi_cmdlist *cmdlist_info, int cmdlist_idx)
{
	int ret;

	if (!cmdlist_info) {
		HISI_DRM_ERR("cmdlist_info is NULL");
		return -EINVAL;
	}

	if ((cmdlist_info->cmdlist_idxs_mask & cmdlist_idx) == 0) {
		ret = hisi_cmdlist_add_nop_node(cmdlist_info, cmdlist_idx, 0, 0);
		if (ret != 0) {
			HISI_DRM_ERR("hisi_cmdlist_add_nop_node err:%d", ret);
			return -1;
		}
	} else {
		HISI_DRM_INFO("%d already add nop node\n", cmdlist_idx);
	}

	ret = hisi_cmdlist_add_new_node(cmdlist_info, cmdlist_idx, 0, 0, 0, 0, 0);
	if (ret != 0) {
		HISI_DRM_ERR("hisi_cmdlist_add_new_node err:%d", ret);
		return -1;
	}
	cmdlist_info->cmdlist_idxs_mask |= cmdlist_idx;
	return 0;
}

#pragma GCC diagnostic pop
/*lint +e679 +e701 +e712 +e734 +e737 +e826 +e838 +e574 +e648 +e570 +e587 +e559 +e578 +e548 +e529*/
