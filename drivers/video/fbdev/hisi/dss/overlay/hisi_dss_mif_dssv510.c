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

#include "hisi_overlay_utils.h"
#include "hisi_mmbuf_manager.h"

struct tag_org_invalid_sel_val {
	enum dss_mmu_tlb_tag_org tag_org;
	uint32_t invalid_sel_val;
};

static struct tag_org_invalid_sel_val tag_org_sel_val[] = {
	{ MMU_TLB_TAG_ORG_0x0, 1 }, { MMU_TLB_TAG_ORG_0x1, 1 },
	{ MMU_TLB_TAG_ORG_0x2, 2 }, { MMU_TLB_TAG_ORG_0x3, 2 },
	{ MMU_TLB_TAG_ORG_0x4, 0 }, { MMU_TLB_TAG_ORG_0x7, 0 },
	{ MMU_TLB_TAG_ORG_0x8, 3 }, { MMU_TLB_TAG_ORG_0x9, 3 },
	{ MMU_TLB_TAG_ORG_0xA, 3 }, { MMU_TLB_TAG_ORG_0xB, 3 },
	{ MMU_TLB_TAG_ORG_0xC, 0 }, { MMU_TLB_TAG_ORG_0xF, 0 },
	{ MMU_TLB_TAG_ORG_0x10, 1 }, { MMU_TLB_TAG_ORG_0x11, 1 },
	{ MMU_TLB_TAG_ORG_0x12, 2 }, { MMU_TLB_TAG_ORG_0x13, 2 },
	{ MMU_TLB_TAG_ORG_0x14, 0 }, { MMU_TLB_TAG_ORG_0x17, 0 },
	{ MMU_TLB_TAG_ORG_0x18, 3 }, { MMU_TLB_TAG_ORG_0x19, 3 },
	{ MMU_TLB_TAG_ORG_0x1A, 3 }, { MMU_TLB_TAG_ORG_0x1B, 3 },
	{ MMU_TLB_TAG_ORG_0x1C, 0 }, { MMU_TLB_TAG_ORG_0x1F, 0 }
};

uint32_t hisi_dss_mif_get_invalid_sel(dss_img_t *img,
	uint32_t transform, int v_scaling_factor,
	uint8_t is_tile, bool rdma_stretch_enable)
{
	uint32_t invalid_sel_val = 0;
	bool find_tag_org = false;
	uint32_t tlb_tag_org;
	uint32_t i;

	dpu_check_and_return(!img, 0, ERR, "img is NULL!\n");

	if ((transform == (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_H)) ||
		(transform == (HISI_FB_TRANSFORM_ROT_90 | HISI_FB_TRANSFORM_FLIP_V)))
		transform = HISI_FB_TRANSFORM_ROT_90;

	tlb_tag_org =  (transform & 0x7) | ((is_tile ? 1 : 0) << 3) | ((rdma_stretch_enable ? 1 : 0) << 4);

	for (i = 0; i < sizeof(tag_org_sel_val) / sizeof(struct tag_org_invalid_sel_val); i++) {
		if (tlb_tag_org == tag_org_sel_val[i].tag_org) {
			invalid_sel_val = tag_org_sel_val[i].invalid_sel_val;
			find_tag_org = true;
			break;
		}
	}

	if (!find_tag_org) {
		invalid_sel_val = 0;
		DPU_FB_ERR("not support this tlb_tag_org 0x%x!\n", tlb_tag_org);
	}

	return invalid_sel_val;
}

