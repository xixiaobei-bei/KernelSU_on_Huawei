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
#ifndef _CMD_LIST_UTILS_H_
#define _CMD_LIST_UTILS_H_

#include "drmP.h"

#include "hisi_overlay_utils_dssv510.h"

#define CONFIG_HISI_DSS_CMDLIST_LAST_USED

/* dssv510 */
#define HISI_DSS_CMDLIST_MAX (16)
/* 16 cmdlist, 16bit, 1111,1111,1111,1111=0xFFFF */
#define HISI_DSS_CMDLIST_IDXS_MAX (0xFFFF)

#define HISI_DSS_CMDLIST_DATA_MAX  (3)
#define HISI_DSS_CMDLIST_NODE_MAX  (32)
#define HISI_DSS_CMDLIST_BLOCK_MAX (32)

#define HISI_DSS_SUPPORT_DPP_MODULE_BIT(module) \
	(BIT(module) & HISI_DSS_DPP_MAX_SUPPORT_BIT)

enum dpp_module_idx {
	DPP_MODULE_POST_SCF = 0,
	DPP_MODULE_DBUF,
	DPP_MODULE_SBL,
	DPP_MODULE_ACM,
	DPP_MODULE_ACE,
	DPP_MODULE_LCP_IGM,
	DPP_MODULE_LCP_GMP,
	DPP_MODULE_LCP_XCC,
	DPP_MODULE_GAMA,
	DPP_MODULE_DITHER,
	DPP_MODULE_IFBC,
	DPP_MODULE_HIACE,
	DPP_MODULE_MAX
};

enum wb_type {
	WB_TYPE_WCH0,
	WB_TYPE_WCH1,
	WB_TYPE_WCH2,
	WB_TYPE_WCH0_WCH1,

	WB_TYPE_MAX,
};

enum dss_cmdlist_idx {
	DSS_CMDLIST_NONE = -1,
	DSS_CMDLIST_D2 = 0,
	DSS_CMDLIST_D3,
	DSS_CMDLIST_V0,
	DSS_CMDLIST_G0,
	DSS_CMDLIST_V1,
	DSS_CMDLIST_G1,
	DSS_CMDLIST_D0,
	DSS_CMDLIST_D1,

	DSS_CMDLIST_W0,
	DSS_CMDLIST_W1,

	DSS_CMDLIST_OV0,
	DSS_CMDLIST_OV1,
	DSS_CMDLIST_OV2,
	DSS_CMDLIST_OV3,

	DSS_CMDLIST_V2,
	DSS_CMDLIST_W2,
	DSS_CMDLIST_MAX,
};


union cmd_flag {
	struct {
		uint32_t exec:1;
		uint32_t last:1;
		uint32_t nop:1;
		uint32_t interrupt:1;
		uint32_t pending:1;
		uint32_t id:10;
		uint32_t event_list:6;
		uint32_t qos:1;
		uint32_t task_end:1;
		uint32_t reserved:1;
		uint32_t valid_flag:8; /* 0xA5 is valid */
	} bits;
	uint32_t ul32;
};

union total_items {
	struct {
		uint32_t count:14;
		uint32_t reserved:18;
	} bits;
	uint32_t ul32;
};

union reg_addr {
	struct {
		uint32_t add0:18;
		uint32_t add1:6;
		uint32_t add2:6;
		uint32_t cnt:2;
	} bits;
	uint32_t ul32;
};

struct cmd_item {
	union reg_addr reg_addr;
	uint32_t data0;
	uint32_t data1;
	uint32_t data2;
};

struct cmd_header {
	union cmd_flag flag;
	uint32_t next_list;
	union total_items total_items;
	uint32_t list_addr; // 128bit align
};

enum dss_cmdlist_node_valid {
	CMDLIST_NODE_INVALID = 0x0,
	CMDLIST_NODE_VALID = 0xA5,
};

enum dss_cmdlist_node_type {
	CMDLIST_NODE_NONE = 0x0,
	CMDLIST_NODE_NOP = 0x1,
	CMDLIST_NODE_FRAME = 0x2,
};

enum dss_cmdlist_status {
	E_STATUS_IDLE = 0x0,
	E_STATUS_WAIT = 0x1,
	E_STATUS_OTHER,
};

/*
 ** for normal node,all variable should be filled.
 ** for NOP node, just the list_header,header_buffer_handle,
 ** list_node, node_flag should be filled.
 ** node_type must be CMDLIST_NODE_NOP when it is NOP node.
 ** And item_buffer_handle in NOP node should be NULL.
 */
struct dss_cmdlist_node {
	struct list_head list_node;

	phys_addr_t header_phys;
	struct cmd_header *list_header;
	size_t header_len;

	phys_addr_t item_phys;
	struct cmd_item *list_item;
	size_t item_len;

	uint32_t item_index;
	int item_flag;
	uint32_t node_type;
	int is_used;
	int reserved;
};

struct dss_cmdlist_heads {
	struct list_head cmdlist_head;

	struct dss_cmdlist_node *cmdlist_nodes[HISI_DSS_CMDLIST_NODE_MAX];
};

struct dss_cmdlist_data {
	struct dss_cmdlist_heads *cmdlist_heads[HISI_DSS_CMDLIST_MAX];
	struct list_head cmdlist_head_temp[HISI_DSS_CMDLIST_MAX];
	struct dss_cmdlist_node
		*cmdlist_nodes_temp
		[HISI_DSS_CMDLIST_MAX][HISI_DSS_CMDLIST_NODE_MAX];
};

struct dss_cmdlist_info {
	/* for primary offline & copybit */
	struct semaphore cmdlist_wb_common_sem;
	struct semaphore cmdlist_wb_sem[WB_TYPE_MAX];
	wait_queue_head_t cmdlist_wb_wq[WB_TYPE_MAX];
	uint32_t cmdlist_wb_done[WB_TYPE_MAX];
	uint32_t cmdlist_wb_flag[WB_TYPE_MAX];
};

struct dss_copybit_info {
	struct semaphore copybit_sem;
	wait_queue_head_t copybit_wq;
	uint32_t copybit_flag;
	uint32_t copybit_done;
};

struct dss_media_common_info {
	wait_queue_head_t mdc_wq;
	uint32_t mdc_flag;
	uint32_t mdc_done;
};

struct dss_wb_info {
	uint32_t to_be_continued;
	uint32_t cmdlist_idxs;
	uint32_t wb_compose_type;
	uint32_t mctl_idx;
};

struct hisi_cmdlist {
	struct gen_pool *cmdlist_pool;
	void *cmdlist_pool_vir_addr;
	phys_addr_t cmdlist_pool_phy_addr;
	size_t sum_cmdlist_pool_size;

	struct dss_cmdlist_data *cmdlist_data_tmp[HISI_DSS_CMDLIST_DATA_MAX];
	struct dss_cmdlist_data *cmdlist_data;
	struct dss_cmdlist_info *cmdlist_info;
	int32_t cmdlist_idx;
	int32_t cmdlist_data_idx;
	uint32_t cmdlist_idxs_mask;
	uint32_t cmdlist_prev_idxs_mask;
};

extern struct dss_cmdlist_data *g_cmdlist_data;
extern int g_debug_ovl_cmdlist;
extern uint32_t g_dss_version_tag;
extern int g_enable_ovl_cmdlist_online;

/* hisi_drm_drv.h include this file, but here use hisi_drm_crtc/dss_hw_ctx */
struct hisi_drm_crtc;
struct dss_hw_ctx;
/******************************************************************************
 ** FUNCTIONS PROTOTYPES
 */
void hisi_cmdlist_set_reg(struct hisi_drm_crtc *hisi_crtc,
	void __iomem *addr, uint32_t value, uint8_t bw, uint8_t bs);
void hisi_cmdlist_flush_cache(struct hisi_cmdlist *cmdlist_info,
	uint32_t cmdlist_idxs);

void hisi_cmdlist_data_get_online(struct hisi_cmdlist *cmdlist_info);

int hisi_cmdlist_add_nop_node(struct hisi_cmdlist *cmdlist_info,
	uint32_t cmdlist_idxs, int pending, int reserved);
int hisi_cmdlist_add_new_node(struct hisi_cmdlist *cmdlist_info,
	uint32_t cmdlist_idxs, int pending,
	int task_end, int remove, int last, uint32_t wb_type);

int hisi_cmdlist_config_start(struct hisi_drm_crtc *acrct, int mctl_idx,
	uint32_t cmdlist_idxs, uint32_t wb_compose_type);

int hisi_cmdlist_config_stop(struct hisi_drm_crtc *hisi_crtc,
	uint32_t cmdlist_pre_idxs, int ovl_idx);
void hisi_cmdlist_config_reset(struct         hisi_drm_crtc *hisi_crtc,
	uint32_t cmdlist_idxs, int ovl_idx);

int hisi_cmdlist_del_node(struct hisi_cmdlist *cmdlist_info,
	uint32_t cmdlist_idxs);

void hisi_dss_cmdlist_qos_on(char __iomem *dss_base);
int hisi_cmdlist_dump_all_node(struct hisi_cmdlist *cmdlist_info,
	uint32_t cmdlist_idxs);

bool hisi_check_cmdlist_paremeters_validate(uint8_t bw, uint8_t bs);
int get_cmdlist_idx(struct hisi_cmdlist *cmdlist_info, int chn_idx);
int hisi_cmdlist_create_node(struct hisi_cmdlist *cmdlist_info,
	int cmdlist_idx);
#endif
