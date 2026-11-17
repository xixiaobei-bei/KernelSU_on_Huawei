/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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
#ifndef _HISI_DSS_SMMU_BASE_H_
#define _HISI_DSS_SMMU_BASE_H_

#define SMMU_SID_NUM 64
#define DSS_SMMU_OUTSTANDING_VAL 0xf

typedef union {
	unsigned int value;
	struct {
		unsigned int ch_sid          : 8;  /* bit[7:0]  */
		unsigned int reserved_0      : 4;  /* bit[11:8] */
		unsigned int ch_ssid         : 8;  /* bit[19:12]  */
		unsigned int ch_ssidv        : 1;  /* bit[20] */
		unsigned int reserved_1      : 3;  /* bit[23:21] */
		unsigned int ch_sc_hint      : 4;  /* bit[27:24] */
		unsigned int ch_sc_hint_en   : 1;  /* bit[28] */
		unsigned int ch_ptl_as_ful   : 1;  /* bit[29] */
		unsigned int reserved_2      : 2;  /* bit[31:30] */
	} reg;
} soc_dss_mmu_id_attr;

typedef struct dss_smmu {
	unsigned int smmu_smrx_ns[SMMU_SID_NUM]; /* for smmu v2 */
	unsigned char smmu_smrx_ns_used[DSS_CHN_MAX_DEFINE];

	soc_dss_mmu_id_attr ch_mmu_attr[DSS_CHN_MAX_DEFINE]; /* for smmu v3 */
} dss_smmu_t;

void hisi_dss_smmu_init(char __iomem *smmu_base, dss_smmu_t *s_smmu);

#ifdef CONFIG_DSS_SMMU_V3
void hisi_dss_smmuv3_on(struct dpu_fb_data_type *dpufd);
int hisi_dss_smmuv3_off(struct dpu_fb_data_type *dpufd);
int hisi_mdc_smmu_off(struct dpu_fb_data_type *dpufd);
void hisi_dss_mmu_lock_clear(struct dpu_fb_data_type *dpufd);
void hisi_dss_mmu_tlb_flush(struct dpu_fb_data_type *dpufd, uint32_t frame_no);
void dpu_smmuv3_invalidate_tlb(struct dpu_fb_data_type *dpufd);
#else
void hisi_dss_smmu_on(struct dpu_fb_data_type *dpufd);
#endif
void hisi_mdc_smmu_on(struct dpu_fb_data_type *dpufd);

int hisi_dss_smmu_ch_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer);

void hisi_dss_smmu_ch_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *smmu_base, dss_smmu_t *s_smmu, int chn_idx);

#endif
