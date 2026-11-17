/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Description: provide interface for config smmu.
 * Author: lixiuhua
 * Create: 2012-12-22
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __SMMU_CFG_H__
#define __SMMU_CFG_H__

#include <linux/iommu.h>
#include <asm/io.h>
#include "hjpeg_common.h"

#define CONFIG_SMMU_RWERRADDR 1

extern int hjpeg_smmu_config(hjpeg_hw_ctl_t *hw_ctl, void *fama_pgd);
extern int hjpeg_smmu_deconfig(hjpeg_hw_ctl_t *hw_ctl, void *fama_pgd);
extern int do_cfg_smmuv3_tbuclose(hjpeg_hw_ctl_t *hw_ctl);
extern void dump_smmu_reg(hjpeg_hw_ctl_t *hw_ctl);
extern int is_hjpeg_qos_update(void);

#define SMMU_RW_ERR_ADDR_SIZE 128

#define SMMU_SMRX_NS(n) (0x0020 + n * 0x4)
#define SMMU_SMRX_S(n)  (0x0500 + n * 0x4)
#define SMMU_SMRX_P(n)  (0x10000 + n * 0x4)

#define SMMU_GLOBAL_BYPASS          0x00

#define SMMU_SCRS                   0x710

#define SMMU_INTMASK_NS             0x10
#define SMMU_INTCLR_NS              0x1C

#define SMMU_CB_TTBR0               0x204
#define SMMU_CB_TTBCR               0x20C
#define SMMU_FAMA_CTRL0_NS          0x220
#define SMMU_FAMA_CTRL1_NS          0x224
#define SMMU_ADDR_MSB               0x300
#define SMMU_ERR_RDADDR             0x304
#define SMMU_ERR_WRADDR             0x308

#define FAMA_CTRL0_NS_fama_bps_msb_ns_LEN       6
#define FAMA_CTRL0_NS_fama_bps_msb_ns_OFFSET    8
#define FAMA_CTRL0_NS_fama_chn_sel_ns_LEN       1
#define FAMA_CTRL0_NS_fama_chn_sel_ns_OFFSET    7
#define FAMA_CTRL0_NS_fama_sdes_msb_ns_LEN      7
#define FAMA_CTRL0_NS_fama_sdes_msb_ns_OFFSET   0

#define FAMA_CTRL1_NS_fama_ptw_msb_ns_LEN       7
#define FAMA_CTRL1_NS_fama_ptw_msb_ns_OFFSET    0

/* Define the union u_smmu_smmu_scr */
typedef union {
	/* Define the struct bits */
	struct {
		uint32_t glb_bypass : 1; /* [0] */
		uint32_t rqos_en : 1; /* [1] */
		uint32_t wqos_en : 1; /* [2] */
		uint32_t reserved_0 : 1; /* [3] */
		uint32_t cache_l1_en : 1; /* [4] */
		uint32_t cache_l2_en : 1; /* [5] */
		uint32_t rqos : 4; /* [9..6] */
		uint32_t wqos : 4; /* [13..10] */
		uint32_t reserved_1 : 1; /* [14] */
		uint32_t smr_rd_shadow : 1; /* [15] */
		uint32_t ptw_pf : 4; /* [19..16] */
		uint32_t ptw_mid : 6; /* [25..20] */
		uint32_t reserved_2 : 6; /* [31..26] */
	} bits;

	/* Define an unsigned member */
	uint32_t reg32;
} u_smmu_smmu_scr;

/* Define the union u_smmu_smmu_addr_msb */
typedef union {
	/* Define the struct bits */
	struct {
		uint32_t msb_errrd : 7; /* [6..0] */
		uint32_t msb_errwr : 7; /* [13..7] */
		uint32_t msb_ova : 7; /* [20..14] */
	} bits;

	/* Define an unsigned member */
	uint32_t reg32;
} u_smmu_smmu_addr_msb;

/******************* SMMU V3 TBU begin*****************************/
#define SMMUV3_SUCCESS                  0
#define SMMUV3_FAIL                     (-1)

#define SMMUV3_TBU_SCR_OFFSET           0x1000
#define SMMUV3_TBU_CR_OFFSET            0x0000
#define SMMUV3_TBU_CRACK_OFFSET         0x0004
#define SMMUV3_TBU_IRPT_CLR_NS_OFFSET   0x001C
#define SMMUV3_TBU_IRPT_MASK_NS_OFFSET  0x0010
#define SMMUV3_TBU_IRPT_CLR_S_OFFSET    0x101C
#define SMMUV3_TBU_IRPT_MASK_S_OFFSET   0x1010

#define TBU_EN_ACK_TIMECOST(start, end) \
	(1000000UL * ((end.tv_sec) - (start.tv_sec)) + \
	(end.tv_usec) - (start.tv_usec))
#define TBU_EN_ACK_TIMEOUT                    1000 /* us */

#define IPP_MAX_TOK_TRANS_NUM                 16
#define IPP_PREFSLOT_FULL_LEVEL_NON_BYPASS    16
#define IPP_FETCHSLOT_FULL_LEVEL_NON_BYPASS   16

#define IPP_FETCHSLOT_FULL_LEVEL_BYPASS       24

#define SMMUV3_IRQ_CTRL_OFFSET          0x50
#define SMMUV3_S_IRQ_CTRL_OFFSET        0x8050
#define SMMUV3_CR0_OFFSET               0x20
#define SMMUV3_CR0ACK_OFFSET            0x24
#define SMMUV3_SWID_CFG_OFFSET          0x100

#define SECADAPT_SWID_CFG_NS_OFFSET     0x000
#define SECADAPT_SWID_CFG_NS_PER_OFFSET 0x4

#define SID_SHIFT                       0x0
#define SSIDV_NS_SHIFT                  0x24

#define SMMUBYPASS_SID                  63
#define SMMUBYPASS_SSIDV_NS             0

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t ns_uarch        : 1; /* [0..0] */
		uint32_t hazard_dis      : 1; /* [1..1] */
		uint32_t ntlb_hitmap_dis : 1; /* [2..2] */
		uint32_t tbu_bypass      : 1; /* [3..3] */
		uint32_t dummy_unlock_en : 1; /* [4..4] */
		uint32_t tlb_inv_sel     : 1; /* [5..5] */
		uint32_t reserved        : 26; /* [31..6] */
	} bits;

	/* Define an unsigend memeber */
	uint32_t reg32;
} u_smmuv3_smmu_tbu_scr;

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t tbu_en_req           : 1; /* [0..0] */
		uint32_t clk_gt_ctrl          : 2; /* [2..1] */
		uint32_t pref_qos_level_en    : 1; /* [3..3] */
		uint32_t pref_qos_level       : 4; /* [7..4] */
		uint32_t max_tok_trans        : 8; /* [15..8] */
		uint32_t fetchslot_full_level : 6; /* [21..16] */
		uint32_t reserved             : 2; /* [23..22] */
		uint32_t prefslot_full_level  : 6; /* [29..24] */
		uint32_t trans_hazard_size    : 2; /* [31..30] */
	} bits;

	/* Define an unsigend memeber */
	uint32_t reg32;
} u_smmuv3_smmu_tbu_cr;

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t tbu_en_ack         : 1; /* [0..0] */
		uint32_t tbu_connected      : 1; /* [1..1] */
		uint32_t reseved            : 6; /* [7..2] */
		uint32_t tok_trans_gnt      : 8; /* [15..8] */
		uint32_t reserved2          : 16; /* [31..16] */
	} bits;

	/* Define an unsigend memeber */
	uint32_t reg32;
} u_smmuv3_smmu_tbu_crack;

typedef union {
	/* Define the struct bits */
	struct {
		uint32_t pref_num           : 8; /* [7..0] */
		uint32_t reserved           : 16; /* [8..23] */
		uint32_t pref_jump          : 4; /* [27..24] */
		uint32_t syscache_hint_sel  : 2; /* [29..28] */
		uint32_t pref_lock_mask     : 1; /* [30] */
		uint32_t pref_en            : 1; /* [31] */
	} bits;

	/* Define an unsigend memeber */
	uint32_t reg32;
} u_smmuv3_smmu_tbu_swid_cfg;

typedef union {
	unsigned int value;
	struct {
		/*
		 * unsigned int  user_def_ns : 16;
		 * bit[0-15] : ¡¤non-sec config
		 * [15] reserved;
		 * [14] partial_as_full;
		 * [13:12] reserved;
		 * [11:8] axhint;used when smmu is bypassed;
		 * [7:0] sid;
		 */
		unsigned int sid         : 8; /* bit[7:0] sid */
		unsigned int user_def_ns : 8; /* bit[15-8] */

		/* bit[16-23]: non-sec ssid config */
		unsigned int ssid_ns     : 8;
		/* bit[24]   : non-sec ssidv config */
		unsigned int ssidv_ns    : 1;
		unsigned int reserved    : 7; /* bit[25-31]: reserved */
	} reg;
} u_sec_adpt_swid_cfg_ns_union;

#endif /* __SMMU_CFG_H__ */
