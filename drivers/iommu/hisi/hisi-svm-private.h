/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _HISI_SVM_PRIVATE_H
#define _HISI_SVM_PRIVATE_H

#include <linux/sched/mm.h>

#define CONFIG_HISI_SVM_STE_BYPASS
#ifdef CONFIG_HISI_SVM_STE_BYPASS
#define PTE_PROT BIT(5)
#define PGD_PROT BIT(63)
#define SMMU_PTE_USER BIT(6)
#define SMMU_PGD_TYPE (BIT(0) | BIT(1))
#define SMMU_PTE_AF BIT(10) /* Access Flag */
#define SMMU_PMD_DEFAULT (BIT(0) | SMMU_PTE_AF)
typedef u64 smmu_pgd_t;
typedef u64 smmu_pmd_t;
typedef u64 smmu_pte_t;
#define SMMU_PGDIR_SHIFT 30
#define SMMU_PTRS_PER_PGD 4
#define SMMU_PGDIR_SIZE (UL(1) << SMMU_PGDIR_SHIFT)
#define SMMU_PMDIR_SHIFT 21
#define SMMU_PTRS_PER_PMD 512
#define SMMU_PMDIR_SIZE BIT(SMMU_PMDIR_SHIFT)
#define smmu_pgd_index(addr)                                                   \
	(((addr) >> SMMU_PGDIR_SHIFT) & (SMMU_PTRS_PER_PGD - 1))
#define smmu_pmd_index(addr)                                                   \
	(((addr) >> SMMU_PMDIR_SHIFT) & (SMMU_PTRS_PER_PMD - 1))
#endif

#define MSTR_END_ACK(end) SMMU_MSTR_END_ACK_##end

/* Register bits */
#define ARM_32_LPAE_TCR_EAE BIT(31)
#define ARM_64_LPAE_S2_TCR_RES1 BIT(31)

#define ARM_LPAE_TCR_EPD1 BIT(23)

#define ARM_LPAE_TCR_TG0_4K (0 << 14)
#define ARM_LPAE_TCR_TG0_64K BIT(14)
#define ARM_LPAE_TCR_TG0_16K (2 << 14)

#define ARM_LPAE_TCR_SH0_SHIFT 12
#define ARM_LPAE_TCR_SH0_MASK 0x3
#define ARM_LPAE_TCR_SH_NS 0
#define ARM_LPAE_TCR_SH_OS 2
#define ARM_LPAE_TCR_SH_IS 3

#define ARM_LPAE_TCR_ORGN0_SHIFT 10
#define ARM_LPAE_TCR_IRGN0_SHIFT 8
#define ARM_LPAE_TCR_RGN_MASK 0x3
#define ARM_LPAE_TCR_RGN_NC 0
#define ARM_LPAE_TCR_RGN_WBWA 1
#define ARM_LPAE_TCR_RGN_WT 2
#define ARM_LPAE_TCR_RGN_WB 3

#define ARM_LPAE_TCR_SL0_SHIFT 6
#define ARM_LPAE_TCR_SL0_MASK 0x3

#define ARM_LPAE_TCR_T0SZ_SHIFT 0
#define ARM_LPAE_TCR_SZ_MASK 0xf

#define ARM_LPAE_TCR_PS_SHIFT 16
#define ARM_LPAE_TCR_PS_MASK 0x7

#define ARM_LPAE_TCR_IPS_SHIFT 32
#define ARM_LPAE_TCR_IPS_MASK 0x7

#define ARM_LPAE_TCR_PS_32_BIT 0x0ULL
#define ARM_LPAE_TCR_PS_36_BIT 0x1ULL
#define ARM_LPAE_TCR_PS_40_BIT 0x2ULL
#define ARM_LPAE_TCR_PS_42_BIT 0x3ULL
#define ARM_LPAE_TCR_PS_44_BIT 0x4ULL
#define ARM_LPAE_TCR_PS_48_BIT 0x5ULL

#define ARM_LPAE_MAIR_ATTR_SHIFT(n) ((n) << 3)
#define ARM_LPAE_MAIR_ATTR_MASK 0xff
#define ARM_LPAE_MAIR_ATTR_DEVICE 0x04
#define ARM_LPAE_MAIR_ATTR_NC 0x44
#define ARM_LPAE_MAIR_ATTR_WBRWA 0xff
#define ARM_LPAE_MAIR_ATTR_IDX_NC 0
#define ARM_LPAE_MAIR_ATTR_IDX_CACHE 1
#define ARM_LPAE_MAIR_ATTR_IDX_DEV 2
#define ARM_LPAE_TTBR_ASID_SHIFT 48

enum smmu_status {
	SMMU_INIT = 0,
	SMMU_ENABLE,
	SMMU_SUSPEND,
	SMMU_RESUME,
	SMMU_DISABLE,
};

enum smmu_bypass_status {
	BYPASS_DISABLE = 0,
	BYPASS_ENABLE,
};

enum mm_svm_id {
	SVM_SDMA = 0,
	SVM_AI,
	SVM_AI2,
};

enum arm_smmu_domain_stage {
	ARM_SMMU_DOMAIN_S1 = 0,
	ARM_SMMU_DOMAIN_S2,
	ARM_SMMU_DOMAIN_NESTED,
};

struct arm_smmu_cmdq_ent {
	/* Common fields */
	u8 opcode;
	bool substream_valid;

	/* Command-specific fields */
	union {
		struct {
			u32 sid;
			u8 size;
			u64 addr;
		} prefetch;

		struct {
			u32 ssid;
			u32 sid;
			union {
				bool leaf;
				u8 span;
			};
		} cfgi;

		struct {
			u16 asid;
			u16 vmid;
			bool leaf;
			u64 addr;
		} tlbi;
	};
};

struct arm_smmu_queue {
	int irq; /* Wired interrupt */
	__le64 *base;
	dma_addr_t base_dma;
	u64 q_base;
	size_t ent_dwords;
	u32 max_n_shift;
	u32 prod;
	u32 cons;
	u32 __iomem *prod_reg;
	u32 __iomem *cons_reg;
};

struct arm_smmu_cmdq {
	struct arm_smmu_queue q;
	spinlock_t lock; /* spinlock */
};

struct arm_smmu_evtq {
	struct arm_smmu_queue q;
	u32 max_stalls;
};

struct arm_smmu_priq {
	struct arm_smmu_queue q;
};

/* High-level stream table and context descriptor structures */
struct arm_smmu_strtab_l1_desc {
	u8 span;
	__le64 *l2ptr;
	dma_addr_t l2ptr_dma;
};

struct arm_smmu_s1_cfg {
	__le64 *cdptr;
	dma_addr_t cdptr_dma;

	struct arm_smmu_ctx_desc {
		u16 ssid;
		u16 asid;
		u64 ttbr;
		u64 tcr;
		u64 mair;
	} cd;
};

struct arm_smmu_s2_cfg {
	u16 vmid;
	u64 vttbr;
	u64 vtcr;
};

struct arm_smmu_strtab_ent {
	bool valid;
	bool bypass;
	struct arm_smmu_cdtab_cfg *cdtab_cfg;
};

struct arm_smmu_strtab_cfg {
	__le64 *strtab;
	dma_addr_t strtab_dma;
	struct arm_smmu_strtab_l1_desc *l1_desc;
	unsigned int num_l1_ents;
	u64 strtab_base;
	u32 strtab_base_cfg;
};

/* An SMMUv3 instance */
struct arm_smmu_device {
	struct device *dev;
	void __iomem *base;

#define ARM_SMMU_FEAT_2_LVL_STRTAB BIT(0)
#define ARM_SMMU_FEAT_2_LVL_CDTAB BIT(1)
#define ARM_SMMU_FEAT_TT_LE BIT(2)
#define ARM_SMMU_FEAT_TT_BE BIT(3)
#define ARM_SMMU_FEAT_PRI BIT(4)
#define ARM_SMMU_FEAT_ATS BIT(5)
#define ARM_SMMU_FEAT_SEV BIT(6)
#define ARM_SMMU_FEAT_MSI BIT(7)
#define ARM_SMMU_FEAT_COHERENCY BIT(8)
#define ARM_SMMU_FEAT_TRANS_S1 BIT(9)
#define ARM_SMMU_FEAT_TRANS_S2 BIT(10)
#define ARM_SMMU_FEAT_STALLS BIT(11)
#define ARM_SMMU_FEAT_HYP BIT(12)
	u32 features;

#define ARM_SMMU_OPT_SKIP_PREFETCH BIT(0)
	u32 options;

#define ARM_SMMU_DEVICES 0

	u32 dev_type;
	struct arm_smmu_cmdq cmdq;
	struct arm_smmu_evtq evtq;
	struct arm_smmu_priq priq;
	int gerr_irq;
	unsigned long ias; /* IPA */
	unsigned long oas; /* PA */
	unsigned int asid_bits;
	unsigned int vmid_bits;
	unsigned int ssid_bits;
	unsigned int sid_bits;
	enum smmu_status status;
	struct arm_smmu_strtab_cfg strtab_cfg;
	struct list_head smmu_node;
	enum mm_svm_id smmuid;
	int smmu_irq;
	int mm_aicpu_irq;
	void __iomem *asid_mem_base;
	void __iomem *va_mem_base;
	wait_queue_head_t wait_event_wq;
	atomic_t event_flag;
	atomic_t structure_init_flag;
	u32 sid_bypass_wr_ai;
	u32 sid_bypass_rd_ai;
	u32 sid_bypass_wr_sdma;
	u32 sid_bypass_rd_sdma;
	u32 sid_mstr0_end0_val;
	u32 sid_mstr0_end1_val;
	u32 sid_mstr1_end0_val;
	u32 sid_mstr1_end1_val;
};

enum arm_smmu_domain_flag {
	ARM_SMMU_DOMAIN_COMMON = 0,
	ARM_SMMU_DOMAIN_SVM_SHARED_PGD = 1,
	ARM_SMMU_DOMAIN_SVM_MIRROR_PGD = 2,
};

struct arm_smmu_domain {
	struct mm_smmu_group *smmu_grp;
	struct mutex init_mutex; /* mutex */
	struct io_pgtable_ops *pgtbl_ops;
	spinlock_t pgtbl_lock; /* spinlock */
	enum arm_smmu_domain_flag flag;
	struct arm_smmu_s1_cfg s1_cfg;
	struct iommu_domain domain;
};

struct arm_smmu_option_prop {
	u32 opt;
	const char *prop;
};

struct mm_svm_manager {
	struct list_head mm_svm_head;
	struct dentry *root;
};

struct arm_smmu_cdtab_cfg {
	__le64 *cdtab;
	size_t sz;
	dma_addr_t cdtab_dma;
};

struct mm_smmu_group {
	struct device *sgrp_dev;
	enum smmu_status status;
	unsigned long ias;
	unsigned long oas;
	unsigned int asid_bits;
	DECLARE_BITMAP(asid_map, ARM_SMMU_MAX_ASIDS);
	unsigned int ssid_bits;
	DECLARE_BITMAP(ssid_map, CTXDESC_CD_MAX_SSIDS);
	struct arm_smmu_cdtab_cfg cdtab_cfg;

	struct mutex sgrp_mtx; /* mutex */
	struct list_head smmu_list;
#ifdef CONFIG_HISI_SVM_STE_BYPASS
#define L2BYPASS_ADDR_NUM 3
	smmu_pgd_t *pgd;
	smmu_pmd_t *pmd;
	enum smmu_bypass_status bypass_status;
	struct arm_smmu_cdtab_cfg cdtab_cfg_bypass;
	u32 bypass_addr[L2BYPASS_ADDR_NUM];
#endif
};

enum mm_svm_lvl {
	SVM_ERROR = 0,
	SVM_WARN = 1,
	SVM_DEBUG = 2,
	SVM_INFO = 3,
	SVM_TRACE = 4,
};

int arm_smmu_svm_get_ssid(struct iommu_domain *domain, u16 *ssid, u64 *ttbr, u64 *tcr);
int arm_smmu_svm_tlb_inv_context(struct mm_struct *mm);
int arm_smmu_poweron(struct device *dev);
int arm_smmu_poweroff(struct device *dev);
int arm_smmu_evt_register_notify(struct device *dev, struct notifier_block *n);
int arm_smmu_evt_unregister_notify(struct device *dev, struct notifier_block *n);
void arm_smmu_tbu_status_print(struct device *dev);
int arm_smmu_get_reg_info(struct device *dev, char *buffer, int length);
#endif
