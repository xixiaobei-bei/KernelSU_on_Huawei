/*
 * IOMMU API for ARM architected SMMUv3 implementations.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2015 ARM Limited
 *
 * Author: Will Deacon <will.deacon@arm.com>
 *
 * This driver is powered by bad coffee and bombay mix.
 */
#define pr_fmt(fmt) "svm:" fmt

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/mmu_notifier.h>
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include "hisi_smmuv3.h"
#include "hisi_svm.h"
#include "io-pgtable.h"
#include <linux/kthread.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>

#define CONFIG_HISI_SVM_STE_BYPASS
#ifdef CONFIG_HISI_SVM_STE_BYPASS
#define PTE_PROT              (1UL << 5)
#define PGD_PROT              (1UL << 63)
#define SMMU_PTE_USER          BIT(6)
#define SMMU_PGD_TYPE         (BIT(0) | BIT(1))
#define SMMU_PTE_AF            BIT(10)       /* Access Flag */
#define SMMU_PMD_DEFAULT      (BIT(0) | SMMU_PTE_AF)
typedef u64 smmu_pgd_t;
typedef u64 smmu_pmd_t;
typedef u64 smmu_pte_t;
#define SMMU_PGDIR_SHIFT	  (30)
#define SMMU_PTRS_PER_PGD     (4)
#define SMMU_PGDIR_SIZE		  (UL(1) << SMMU_PGDIR_SHIFT)
#define SMMU_PMDIR_SHIFT      (21)
#define SMMU_PTRS_PER_PMD     (512)
#define SMMU_PMDIR_SIZE        BIT(SMMU_PMDIR_SHIFT)
#define smmu_pgd_index(addr)   \
	(((addr) >> SMMU_PGDIR_SHIFT) & (SMMU_PTRS_PER_PGD - 1))
#define smmu_pmd_index(addr)   \
	(((addr) >> SMMU_PMDIR_SHIFT) & (SMMU_PTRS_PER_PMD - 1))
#endif

#define MSTR_END_ACK(end) SMMU_MSTR_END_ACK_##end

enum smmu_status {
	smmu_init = 0,
	smmu_enable,
	smmu_suspend,
	smmu_resume,
	smmu_disable,
};

enum smmu_bypass_status {
	bypass_disable = 0,
	bypass_enable,
};

enum hisi_svm_id {
	svm_sdma = 0,
	svm_ai,
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
	spinlock_t lock;
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

#define ARM_SMMU_FEAT_2_LVL_STRTAB (1 << 0)
#define ARM_SMMU_FEAT_2_LVL_CDTAB (1 << 1)
#define ARM_SMMU_FEAT_TT_LE (1 << 2)
#define ARM_SMMU_FEAT_TT_BE (1 << 3)
#define ARM_SMMU_FEAT_PRI (1 << 4)
#define ARM_SMMU_FEAT_ATS (1 << 5)
#define ARM_SMMU_FEAT_SEV (1 << 6)
#define ARM_SMMU_FEAT_MSI (1 << 7)
#define ARM_SMMU_FEAT_COHERENCY (1 << 8)
#define ARM_SMMU_FEAT_TRANS_S1 (1 << 9)
#define ARM_SMMU_FEAT_TRANS_S2 (1 << 10)
#define ARM_SMMU_FEAT_STALLS (1 << 11)
#define ARM_SMMU_FEAT_HYP (1 << 12)
	u32 features;

#define ARM_SMMU_OPT_SKIP_PREFETCH (1 << 0)
	u32 options;

#define ARM_SMMU_DEVICES (0)

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
	enum hisi_svm_id smmuid;
	int smmu_irq;
	int hisi_aicpu_irq;
	void __iomem  *asid_mem_base;
	void __iomem  *va_mem_base;
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
	struct hisi_smmu_group *smmu_grp;
	struct mutex init_mutex;
	struct io_pgtable_ops *pgtbl_ops;
	spinlock_t pgtbl_lock;
	enum arm_smmu_domain_flag flag;
	struct arm_smmu_s1_cfg s1_cfg;
	struct iommu_domain domain;
};

struct arm_smmu_option_prop {
	u32 opt;
	const char *prop;
};

struct hisi_svm_manager {
	struct list_head hisi_svm_head;
	struct dentry *root;
};

struct arm_smmu_cdtab_cfg {
	__le64 *cdtab;
	size_t sz;
	dma_addr_t cdtab_dma;
};

struct hisi_smmu_group {
	struct device *sgrp_dev;
	enum smmu_status status;
	unsigned long ias;
	unsigned long oas;
	unsigned int asid_bits;
	DECLARE_BITMAP(asid_map, ARM_SMMU_MAX_ASIDS);
	unsigned int ssid_bits;
	DECLARE_BITMAP(ssid_map, CTXDESC_CD_MAX_SSIDS);
	struct arm_smmu_cdtab_cfg cdtab_cfg;

	struct mutex sgrp_mtx;
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

enum hisi_svm_lvl {
	svm_error = 0,
	svm_warn = 1,
	svm_debug = 2,
	svm_info = 3,
	svm_trace = 4,
};

static int svm_debug_level = 3;
module_param_named(level, svm_debug_level, int, S_IRUGO);

#define hisi_svm_print(level, x...)                               \
	do {                                                          \
		if (svm_debug_level >= (level))                           \
			pr_err(x);                                            \
	} while (0)

struct hisi_svm *hisi_svm_bind;
struct hisi_smmu_group *hisi_smmu_group;
struct hisi_svm_manager hisi_svm_manager;
struct arm_smmu_device *hisi_mmu_dev;
static struct mutex hisi_svm_mutex;
static struct mutex hisi_svmtlb_mutex;
u64 pgfault_asid_addr_g = 0;
u64 pgfault_va_addr_g = 0;
static int hisi_smmu_poweron_flag;

struct bus_type ion_svm_bus = {
	.name = "svm",
};

static void hisi_smmu_group_add_device(
	struct hisi_smmu_group *gp, struct arm_smmu_device *smmu);
static void hisi_smmu_group_del_device(
	struct hisi_smmu_group *gp, struct arm_smmu_device *smmu);
static int arm_smmu_enable_cd(
	struct hisi_smmu_group *grp, struct arm_smmu_domain *dom);
static void hisi_evt_flag_set(struct arm_smmu_device *smmu);
static struct iommu_domain *arm_smmu_domain_alloc(unsigned int type);
static void arm_smmu_domain_free(struct iommu_domain *domain);
static int arm_smmu_attach_dev(
	struct iommu_domain *domain, struct device *dev);
static void arm_smmu_detach_dev(
	struct iommu_domain *domain, struct device *dev);
static int arm_smmu_device_disable(struct arm_smmu_device *smmu);
static void hisi_smmu_group_tlb_inv_context(void *cookie);
static void hisi_structure_init_flag_set(struct arm_smmu_device *smmu);
static void hisi_structure_init_flag_unset(struct arm_smmu_device *smmu);

static struct iommu_ops arm_smmu_ops = {
	.domain_alloc = arm_smmu_domain_alloc,
	.domain_free = arm_smmu_domain_free,
	.attach_dev = arm_smmu_attach_dev,
	.detach_dev = arm_smmu_detach_dev,
	.pgsize_bitmap = (unsigned long)-1, /* Restricted during dev attach */
};

static int invalid_tcu_cache(struct arm_smmu_device *smmu)
{
	u32 reg;
	u32 check_times = 0;

	writel_relaxed(1, smmu->base + CACHELINE_INV_ALL);
	do {
		reg = readl_relaxed(smmu->base + CACHELINE_INV_ALL);
		if (!(reg & 0x1))
			break;
		udelay(1);
		if (++check_times >= MAX_CHECK_TIMES) {
			hisi_svm_print(svm_error,
				"CACHELINE_INV_ALL failed !%s\n", __func__);
			return -EINVAL;
		}
	} while (1);

	return 0;
}

static struct arm_smmu_domain *to_smmu_domain(struct iommu_domain *dom)
{
	return container_of(dom, struct arm_smmu_domain, domain);
}

static int pte_flush_range(pmd_t *pmd, unsigned long addr, unsigned long end)
{
	pte_t *pte, *pte4k;

	pte = pte_offset_map(pmd, addr);//lint !e666

	pte4k = (pte_t *)round_down((u64)pte, PAGE_SIZE);
	__flush_dcache_area(pte4k, PAGE_SIZE);

	pte_unmap(pte);

	return 0;
}

static int pmd_flush_range(pud_t *pud, unsigned long addr, unsigned long end)
{
	pmd_t *pmd, *pmd4k;
	unsigned long next;

	pmd = pmd_offset(pud, addr);//lint !e666
	pmd4k = (pmd_t *)round_down((u64)pmd, PAGE_SIZE);

	do {
		next = pmd_addr_end(addr, end);
		pte_flush_range(pmd, addr, next);
		pmd++;
		addr = next;
	}while(addr != end);

	__flush_dcache_area(pmd4k, PAGE_SIZE);

	return 0;
}

static int pud_flush_range(pgd_t *pgd, unsigned long addr, unsigned long end)
{
	pud_t *pud;
#if CONFIG_PGTABLE_LEVELS > 3
	pud_t *pud4k;
#endif
	unsigned long next;

	pud = pud_offset(pgd, addr);
#if CONFIG_PGTABLE_LEVELS > 3
	pud4k = (pud_t *)round_down((u64)pud, PAGE_SIZE);
#endif

	do {
		next = pud_addr_end(addr, end);
		pmd_flush_range(pud, addr, next);
		pud++;
		addr = next;
	}while(addr != end);

#if CONFIG_PGTABLE_LEVELS > 3
	__flush_dcache_area(pud4k, PAGE_SIZE);
#endif
	return 0;
}

static void hisi_smmu_group_flush_tlb(void)
{
	struct arm_smmu_domain *smmu_domain;

	if (!hisi_svm_bind)
		return;

	smmu_domain = to_smmu_domain(hisi_svm_bind->dom);

	if (!smmu_domain) {
		hisi_svm_print(svm_error, "%s,smmu_domain is null\n", __func__);
		return;
	}
	hisi_smmu_group_tlb_inv_context(smmu_domain);

	hisi_svm_print(svm_trace, "out %s\n", __func__);
}

int hisi_svm_flush_cache(struct mm_struct *mm,
	unsigned long addr, size_t size)
{
	pgd_t *pgd;
	pgd_t *pgd4k;
	unsigned long next;
	unsigned long end = addr + PAGE_ALIGN(size);

	hisi_svm_print(svm_trace, "tlb-cache:map user addr[0x%8lx ~0x%8lx]\n", addr, end);

	pgd = pgd_offset(mm, addr);
	pgd4k = (pgd_t *)round_down((u64)pgd, PAGE_SIZE);

	do {
		next = pgd_addr_end(addr, end);
		pud_flush_range(pgd, addr, next);
		pgd++;
		addr = next;
	} while (addr != end);

	__flush_dcache_area(pgd4k, PAGE_SIZE);

	if (hisi_smmu_poweron_flag)
		hisi_smmu_group_flush_tlb();/*flush tlb when mmap*/

	return 0;
}


#ifdef CONFIG_HISI_DEBUG_FS
static int hisi_svm_instance_dbgfs_show(struct seq_file *s, void *private)
{
	unsigned int *desc = NULL;
	struct arm_smmu_domain *smmu_domain;
	struct hisi_svm *svm = s->private;
	struct arm_smmu_s1_cfg *s1_cfg;
	struct hisi_smmu_group *smmu_group;
	struct list_head *p, *n;
	struct arm_smmu_device *smmu = NULL;
	__le64 *step = NULL;
	struct arm_smmu_strtab_cfg *cfg;
	unsigned int *ste = NULL;

	if (!svm)
		return 0;

	if (!svm->dom)
		return 0;

	smmu_domain = to_smmu_domain(svm->dom);
	if (!smmu_domain)
		return 0;

	smmu_group = smmu_domain->smmu_grp;
	if (!smmu_group)
		return 0;

	s1_cfg = &smmu_domain->s1_cfg;
	if (!s1_cfg)
		return 0;

	desc = (unsigned int *)smmu_domain->s1_cfg.cdptr;
	if (!desc)
		return 0;

	seq_printf(s, "%16s\t%16s\t%16s\t%16s\t%16s\t%16s\t%16s\n", "name",
		"l2addr", "ssid", "asid", "ttbr", "tcr", "mair");

	seq_printf(s,
		"%16s\t%16lx\t%16x\t%16x\t%16llx\t%16llx\t%16llx\n",
		svm->name, svm->l2addr, smmu_domain->s1_cfg.cd.ssid,
		smmu_domain->s1_cfg.cd.asid, smmu_domain->s1_cfg.cd.ttbr,
		smmu_domain->s1_cfg.cd.tcr, smmu_domain->s1_cfg.cd.mair);

	seq_printf(s, "%16s\t%16x\t%16x\t%16x\t%16x\n",
		"context desc[0]", desc[0], desc[1], desc[2], desc[3]);
	seq_printf(s, "%16s\t%16x\t%16x\t%16x\t%16x\n",
		"context desc[1]", desc[4], desc[5], desc[6], desc[7]);

	list_for_each_safe(p, n, &smmu_group->smmu_list) {
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		cfg = &smmu->strtab_cfg;
		step = &cfg->strtab[0];/*all ste entry are the same */
		ste = (unsigned int *)step;
		seq_printf(s, "smmuid:%d\n", smmu->smmuid);
		seq_printf(s, "%16s\t%16x\t%16x\t%16x\t%16x\n",
			"ste[0]", ste[0], ste[1], ste[2], ste[3]);
		seq_printf(s, "%16s\t%16x\t%16x\t%16x\t%16x\n",
			"ste[1]", ste[4], ste[5], ste[6], ste[7]);
		seq_printf(s, "bypass sid:ai_w:%d,ai_r:%d,sdma_w:%d,sdma_r:%d\n",
			smmu->sid_bypass_wr_ai, smmu->sid_bypass_rd_ai,
			smmu->sid_bypass_wr_sdma,smmu->sid_bypass_rd_sdma);
	}
	return 0;
}

static int hisi_svm_instance_dbgfs_open(
	struct inode *inode, struct file *file)
{
	return single_open(
		file, hisi_svm_instance_dbgfs_show, inode->i_private);
}

static const struct file_operations hisi_svm_instance_dbgfs_fops = {
	.open = hisi_svm_instance_dbgfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

/* so the L2 buf's map the virt address and size must be 2M align */
void *hisi_svm_get_l2buf_pte(struct hisi_svm *svm, unsigned long addr)
{
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	struct mm_struct *mm;

	if (!svm || !svm->mm) {
		hisi_svm_print(svm_error, "%s,params is invalid\n", __func__);
		return NULL;
	}

	if (!IS_ALIGNED(addr, SZ_2M)) {
		hisi_svm_print(svm_error, "%s, addr is invalidate 0x%lx\n",
			__func__, addr);
		return NULL;
	}

	svm->l2addr = addr;
	mm = svm->mm;

	pgd = pgd_offset(mm, addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return NULL;

	pud = pud_offset(pgd, addr);
	if (pud_none(*pud) || pud_bad(*pud))
		return NULL;

	pmd = pmd_offset(pud, addr);//lint !e666

	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return NULL;

	pte = pte_offset_map(pmd, addr);//lint !e666
	if (pte_none(*pte)) {
		pte_unmap(pte);
		return NULL;
	}

	return pte;
}
EXPORT_SYMBOL(hisi_svm_get_l2buf_pte);

int hisi_svm_get_ssid(struct hisi_svm *svm, u16 *ssid, u64 *ttbr, u64 *tcr)
{
	struct arm_smmu_domain *smmu_domain;

	if (!svm || !ssid || !ttbr || !tcr)
		return -EINVAL;

	if (!svm->dom)
		return -EINVAL;
	smmu_domain = to_smmu_domain(svm->dom);
	if (!smmu_domain)
		return -EINVAL;

	*ssid = smmu_domain->s1_cfg.cd.ssid;
	*ttbr = smmu_domain->s1_cfg.cd.ttbr;
	*tcr = smmu_domain->s1_cfg.cd.tcr;
	return 0;
}
EXPORT_SYMBOL(hisi_svm_get_ssid);

#ifdef CONFIG_HISI_SVM_PTE_SHOW
static void *ioremap_page(phys_addr_t phys, int count)
{
	void *va;
	struct page *pages[2];
	pgprot_t pgprot;

	/* turn cached virt-addr to phy-addr,
	 * remap phy-addr to noncached virt-addr.
	 */
	pages[0] = phys_to_page(phys);
	pgprot = pgprot_writecombine(PAGE_KERNEL);/*noncached*/
	va = vmap(pages, count, VM_MAP, pgprot);
	if (!va)
		return NULL;
	return va;
}

static void unioremap_page(const void *v)
{
	vunmap(v);
}

/* if cache-dir != noncache-dir, no need to get next dir,
 * cache is not coherent.
 */
static int hisi_svm_check_pgd(pgd_t *pgd)
{
	phys_addr_t pgd_phy;
	void *pgd_virt;
	pgd_t *pgd_new;
	int count = 1; /*1 page*/

	if (!pgd)
		return -EINVAL;

	pgd_phy = virt_to_phys(pgd);
	pgd_virt = ioremap_page(pgd_phy, count);
	if (!pgd_virt) {
		pr_alert("ioremap_page failed!\n");
		return -EINVAL;
	}

	pgd_new = (pgd_t *)(pgd_virt + (pgd_phy & (PAGE_SIZE-1)));
	pr_cont("cache-pgd=%016llx,noncache-pgd=%016llx\n",
		pgd_val(*pgd), pgd_val(*pgd_new));

	if (pgd_none(*pgd) || pgd_bad(*pgd)
		|| pgd_none(*pgd_new) || pgd_bad(*pgd_new)) {
		pr_cont("pgd is bad!\n");
		goto unmap_pgd;
	}

	if (pgd_val(*pgd) != pgd_val(*pgd_new)) {
		pr_cont("pgd is not cache coherent\n");
		goto unmap_pgd;
	}
	unioremap_page(pgd_virt);
	return 0;

unmap_pgd:
	unioremap_page(pgd_virt);
	return -EINVAL;
}

static int hisi_svm_check_pud(pud_t *pud)
{
	phys_addr_t pud_phy;
	void *pud_virt;
	pud_t *pud_new;
	int count = 1; /*1 page*/

	if (!pud)
		return -EINVAL;

	pud_phy = virt_to_phys(pud);
	pud_virt = ioremap_page(pud_phy, count);
	if (!pud_virt)
		return -EINVAL;

	pud_new = (pud_t *)(pud_virt + (pud_phy & (PAGE_SIZE-1)));
	pr_cont("cache-pud=%016llx,noncache-pud=%016llx \n",
		pud_val(*pud), pud_val(*pud_new));

	if (pud_none(*pud) || pud_bad(*pud)
		|| pud_none(*pud_new) || pud_bad(*pud_new)) {
		pr_cont("pud is bad!\n");
		goto unmap_pud;
	}

	if (pud_val(*pud) != pud_val(*pud_new)) {
		pr_cont("pud is not cache coherent!\n");
		goto unmap_pud;
	}
	unioremap_page(pud_virt);
	return 0;

unmap_pud:
	unioremap_page(pud_virt);
	return -EINVAL;
}

static int hisi_svm_check_pmd(pmd_t *pmd)
{
	phys_addr_t pmd_phy;
	void *pmd_virt;
	pmd_t *pmd_new;
	int count = 1; /*1 page*/

	if (!pmd)
		return -EINVAL;

	pmd_phy = virt_to_phys(pmd);
	pmd_virt = ioremap_page(pmd_phy, count);
	if (!pmd_virt)
		return -EINVAL;

	pmd_new = (pmd_t *)(pmd_virt + (pmd_phy & (PAGE_SIZE-1)));
	pr_cont("cache-pmd=%016llx,noncache-pmd=%016llx\n",
		pmd_val(*pmd), pmd_val(*pmd_new));

	if (pmd_none(*pmd) || pmd_bad(*pmd)
		||pmd_none(*pmd_new) || pmd_bad(*pmd_new)) {
		pr_cont("pmd is bad!\n");
		goto unmap_pmd;
	}

	if (pmd_val(*pmd) != pmd_val(*pmd_new)) {
		pr_cont("pmd is not cache coherent!\n");
		goto unmap_pmd;
	}
	unioremap_page(pmd_virt);
	return 0;

unmap_pmd:
	unioremap_page(pmd_virt);
	return -EINVAL;
}

static int hisi_svm_check_pte(pte_t *pte)
{
	phys_addr_t pte_phy;
	void *pte_virt;
	pte_t *pte_new;
	int count = 1; /*1 page*/

	if (!pte)
		return -EINVAL;

	pte_phy = virt_to_phys(pte);
	pte_virt = ioremap_page(pte_phy, count);
	if (!pte_virt)
		return -EINVAL;

	pte_new = (pte_t *)(pte_virt + (pte_phy & (PAGE_SIZE-1)));
	pr_cont("cache-pte=%016llx,noncache-pte=%016llx\n",
		pte_val(*pte), pte_val(*pte_new));

	if (pte_val(*pte) != pte_val(*pte_new)) {
		pr_cont("pte is not cache coherent!\n");
		unioremap_page(pte_virt);
		return -EINVAL;
	}
	unioremap_page(pte_virt);
	return 0;
}

void hisi_svm_show_pte_noncache(struct hisi_svm *svm,
	unsigned long addr, size_t size)
{
	size_t step = SZ_4K;
	struct mm_struct *mm;
	int ret = 0;

	if (!svm || !svm->mm || !size)
		return;

	mm = svm->mm;
	pr_alert("non-cache pgd = %pK,addr:[0x%8lx]\n", mm->pgd, addr);

	do {
		pgd_t *pgd;
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		step = SZ_4K;
		pgd = pgd_offset(mm, addr);
		ret = hisi_svm_check_pgd(pgd);
		if (ret)
			return;

		pud = pud_offset(pgd, addr);
		ret = hisi_svm_check_pud(pud);
		if (ret)
			return;

		pmd = pmd_offset(pud, addr);//lint !e666
		ret = hisi_svm_check_pmd(pmd);
		if (ret)
			return;

		pte = pte_offset_map(pmd, addr);//lint !e666
		ret = hisi_svm_check_pte(pte);
		if (ret)
			return;

		pte_unmap(pte);
	} while (size >= step && (addr += step, size -= step));
}

void hisi_svm_show_pte(struct hisi_svm *svm,
	unsigned long addr, size_t size)
{
	size_t step = SZ_4K;
	struct mm_struct *mm;

	if (!svm || !svm->mm || !size)
		return;

	spin_lock(&svm->mm->page_table_lock);

	hisi_svm_show_pte_noncache(svm, addr, size);

	mm = svm->mm;
	pr_alert("cached pgd = %pK\n", mm->pgd);

	do {
		pgd_t *pgd;
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		step = SZ_4K;
		pgd = pgd_offset(mm, addr);
		pr_cont("[%08lx], *pgd=%016llx", addr, pgd_val(*pgd));
		if (pgd_none(*pgd) || pgd_bad(*pgd)) {
			step = SZ_1G;
			continue;
		}

		pud = pud_offset(pgd, addr);
		pr_cont(", *pud=%016llx", pud_val(*pud));
		if (pud_none(*pud) || pud_bad(*pud)) {
			step = SZ_1G;
			continue;
		}

		pmd = pmd_offset(pud, addr);//lint !e666
		pr_cont(", *pmd=%016llx", pmd_val(*pmd));
		if (pmd_none(*pmd) || pmd_bad(*pmd)) {
			step = SZ_2M;
			continue;
		}

		pte = pte_offset_map(pmd, addr);//lint !e666
		pr_cont(", *pte=%016llx", pte_val(*pte));
		pte_unmap(pte);
		pr_cont("\n");
	} while (size >= step && (addr += step, size -= step));

	spin_unlock(&svm->mm->page_table_lock);
}
EXPORT_SYMBOL(hisi_svm_show_pte);
#endif

int hisi_svm_flag_set(struct task_struct *task, u32 flag)
{
	struct mm_struct *mm;

	mm = get_task_mm(task);
	if (!mm) {
		hisi_svm_print(svm_error, "%s get_task_mm failed!\n", __func__);
		return -EINVAL;
	}
	if (flag)
		set_bit(MMF_SVM, &mm->flags);
	else
		clear_bit(MMF_SVM, &mm->flags);

	mmput(mm);
	hisi_svm_print(svm_info, "into %s,flag:%d\n", __func__, flag);
	return 0;
}
EXPORT_SYMBOL(hisi_svm_flag_set);

struct hisi_svm *hisi_svm_bind_task(
	struct device *dev, struct task_struct *task)
{
	struct hisi_svm *svm;
	struct mm_struct *mm;
	struct iommu_domain *dom;
#ifdef CONFIG_HISI_DEBUG_FS
	char debug_name[64];
#endif

	hisi_svm_print(svm_info, "into %s\n", __func__);
	if (!dev || !task) {
		hisi_svm_print(svm_error, "%s param invalid!\n", __func__);
		return NULL;
	}

	mutex_lock(&hisi_svm_mutex);
	if (hisi_svm_bind) {
		hisi_svm_print(svm_error, "%s: pls unbind last svm!\n", __func__);
		goto out;
	}

	svm = kzalloc(sizeof(*svm), GFP_KERNEL);
	if (!svm) {
		hisi_svm_print(svm_error, "%s kzalloc failed!\n", __func__);
		goto out;
	}

	dom = iommu_domain_alloc(&ion_svm_bus);
	if (!dom) {
		hisi_svm_print(
			svm_error, "%s iommu_domain_alloc failed!\n", __func__);
		goto out_free;
	}
	svm->dom = dom;

	if (iommu_attach_device(dom, dev)) {
		hisi_svm_print(
			svm_error, "%s iommu_attach_device failed!\n", __func__);
		goto dom_free;
	}
	svm->dev = dev;

	svm->name = task->comm;
	mm = get_task_mm(task);
	if (!mm) {
		hisi_svm_print(svm_error, "%s get_task_mm failed!\n", __func__);
		goto iommu_attach;
	}
	svm->mm = mm;
	/*
	 * Drop the reference to the mm_struct here. We rely on the
	 * mmu_notifier release call-back to inform us when the mm
	 * is going away.
	 */
	mmput(mm);
#ifdef CONFIG_HISI_DEBUG_FS
	snprintf(debug_name, 64, "svm_%s_%d", svm->name, task->pid);/*unsafe_function_ignore: snprintf*/
	debug_name[63] = '\0';

	svm->debug_root = debugfs_create_file(debug_name, 0640,
		hisi_svm_manager.root, svm, &hisi_svm_instance_dbgfs_fops);
	if (!svm->debug_root) {
		hisi_svm_print(
			svm_error, "%s debugfs_create_file failed!\n", __func__);
		goto iommu_attach;
	}
#endif
	hisi_svm_bind = svm;
	hisi_svm_print(svm_info, "out %s\n", __func__);
	mutex_unlock(&hisi_svm_mutex);
	return svm;

iommu_attach:
	iommu_detach_device(svm->dom, svm->dev);

dom_free:
	iommu_domain_free(dom);

out_free:
	kfree(svm);

out:
	mutex_unlock(&hisi_svm_mutex);
	hisi_svm_print(svm_error, "error out %s\n", __func__);
	return NULL;
}
EXPORT_SYMBOL(hisi_svm_bind_task);

void hisi_svm_unbind_task(struct hisi_svm *svm)
{
	if (!svm) {
		hisi_svm_print(svm_error, "%s:svm NULL\n", __func__);
		return;
	}

	mutex_lock(&hisi_svm_mutex);
	if (!hisi_svm_bind || (hisi_svm_bind != svm)) {
		hisi_svm_print(svm_error, "%s:svm invalid\n", __func__);
		mutex_unlock(&hisi_svm_mutex);
		return;
	}

	if (svm->debug_root)
		debugfs_remove_recursive(svm->debug_root);

	iommu_detach_device(svm->dom, svm->dev);
	iommu_domain_free(svm->dom);
	kfree(svm);
	hisi_svm_bind = NULL;
	mutex_unlock(&hisi_svm_mutex);
	hisi_svm_print(svm_info, "out %s\n", __func__);
}
EXPORT_SYMBOL(hisi_svm_unbind_task);

#ifdef CONFIG_HISI_DEBUG_FS
static int hisi_smmu_group_dbgfs_show(struct seq_file *m, void *private)
{
	struct hisi_smmu_group *smmu_group = hisi_smmu_group;

	if (!smmu_group)
		return -EINVAL;
	seq_printf(m, "group status:%d, ias:0x%lx, oas:0x%lx\n",
		smmu_group->status, smmu_group->ias, smmu_group->oas);
	seq_printf(m, "asid_bits:%d, ssid_bits:%d\n",
		smmu_group->asid_bits, smmu_group->ssid_bits);
#ifdef CONFIG_HISI_SVM_STE_BYPASS
	seq_printf(m, "bypass enable:%d\n", smmu_group->bypass_status);
#endif
	return 0;
}

static int hisi_smmu_group_dbgfs_open(
	struct inode *inode, struct file *file)
{
	return single_open(file, hisi_smmu_group_dbgfs_show, inode->i_private);
}

static const struct file_operations hisi_smmu_group_dbgfs_fops = {
	.open = hisi_smmu_group_dbgfs_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};
#endif

static void __init hisi_svm_manager_init(void)
{
	INIT_LIST_HEAD(&(hisi_svm_manager.hisi_svm_head));
	hisi_svm_manager.root = NULL;
}

#ifdef CONFIG_HISI_DEBUG_FS
static int __init hisi_svm_init_debugfs(void)
{
	struct dentry *entry, *file;
	struct dentry *root = debugfs_create_dir("svm", NULL);

	if (IS_ERR_OR_NULL(root)) {
		hisi_svm_print(svm_error,
			"debugfs_create_dir svm is error %s\n", __func__);
		return -ENXIO;
	}

	file = debugfs_create_file("hisi_smmu_group", S_IRUGO, root,
		hisi_smmu_group, &hisi_smmu_group_dbgfs_fops);
	if (IS_ERR_OR_NULL(file)) {
		hisi_svm_print(svm_error,
			"debugfs_create_dir hisi_smmu_group is error %s\n",
			__func__);
		return -ENXIO;
	}

	entry = debugfs_create_dir("hisi_svm", root);
	if (IS_ERR_OR_NULL(entry)) {
		hisi_svm_print(svm_error,
			"debugfs_create_dir hisi_svm is error %s\n", __func__);
		return -ENXIO;
	}
	hisi_svm_manager.root = entry;

	return 0;
}
#endif

static struct arm_smmu_option_prop arm_smmu_options[] = {
	{ARM_SMMU_OPT_SKIP_PREFETCH, "hisilicon,broken-prefetch-cmd"},
	{0, NULL},
};

static void parse_driver_options(struct arm_smmu_device *smmu)
{
	int i = 0;
	do {
		if (of_property_read_bool(
			    smmu->dev->of_node, arm_smmu_options[i].prop)) {
			smmu->options |= arm_smmu_options[i].opt;
			dev_notice(smmu->dev, "option %s\n",
				arm_smmu_options[i].prop);
		}
	} while (arm_smmu_options[++i].opt);
}

/* Low-level queue manipulation functions */
static bool queue_full(struct arm_smmu_queue *q)
{
	return Q_IDX(q, q->prod) == Q_IDX(q, q->cons) &&
	       Q_WRP(q, q->prod) != Q_WRP(q, q->cons);
}

static bool queue_empty(struct arm_smmu_queue *q)
{
	return Q_IDX(q, q->prod) == Q_IDX(q, q->cons) &&
	       Q_WRP(q, q->prod) == Q_WRP(q, q->cons);
}

static void queue_sync_cons(struct arm_smmu_queue *q)
{
	q->cons = readl_relaxed(q->cons_reg);
}

static void queue_inc_cons(struct arm_smmu_queue *q)
{
	u32 cons = (Q_WRP(q, q->cons) | Q_IDX(q, q->cons)) + 1;

	q->cons = Q_OVF(q, q->cons) | Q_WRP(q, cons) | Q_IDX(q, cons);
	writel(q->cons, q->cons_reg);
}

static int queue_sync_prod(struct arm_smmu_queue *q)
{
	int ret = 0;
	u32 prod = readl_relaxed(q->prod_reg);

	if (Q_OVF(q, prod) != Q_OVF(q, q->prod))
		ret = -EOVERFLOW;

	q->prod = prod;
	return ret;
}

static void queue_inc_prod(struct arm_smmu_queue *q)
{
	u32 prod = (Q_WRP(q, q->prod) | Q_IDX(q, q->prod)) + 1;

	q->prod = Q_OVF(q, q->prod) | Q_WRP(q, prod) | Q_IDX(q, prod);
	writel(q->prod, q->prod_reg);
}

/*
 * Wait for the SMMU to consume items. If drain is true, wait until the queue
 * is empty. Otherwise, wait until there is at least one free slot.
 */
static int queue_poll_cons(struct arm_smmu_queue *q, bool drain, bool wfe)
{
	ktime_t timeout = ktime_add_us(ktime_get(), ARM_SMMU_POLL_TIMEOUT_US);

	while (queue_sync_cons(q), (drain ? !queue_empty(q) : queue_full(q))) {
		if (ktime_compare(ktime_get(), timeout) > 0)
			return -ETIMEDOUT;

		if (wfe) {
			wfe();
		} else {
			cpu_relax();
			udelay(1);
		}
	}

	return 0;
}

static void queue_write(__le64 *dst, u64 *src, size_t n_dwords)
{
	u32 i;
	for (i = 0; i < n_dwords; ++i)
		*dst++ = cpu_to_le64(*src++);
}

static int queue_insert_raw(struct arm_smmu_queue *q, u64 *ent)
{
	if (queue_full(q))
		return -ENOSPC;
	queue_write(Q_ENT(q, q->prod), ent, q->ent_dwords);
	queue_inc_prod(q);
	return 0;
}

static void queue_read(__le64 *dst, u64 *src, size_t n_dwords)
{
	u32 i;

	for (i = 0; i < n_dwords; ++i)
		*dst++ = le64_to_cpu(*src++);
}

static int queue_remove_raw(struct arm_smmu_queue *q, u64 *ent)
{
	if (queue_empty(q))
		return -EAGAIN;

	queue_read(ent, Q_ENT(q, q->cons), q->ent_dwords);
	queue_inc_cons(q);
	return 0;
}

/* High-level queue accessors */
static int arm_smmu_cmdq_build_cmd(u64 *cmd, struct arm_smmu_cmdq_ent *ent)
{
	memset(cmd, 0, CMDQ_ENT_DWORDS << 3);/* unsafe_function_ignore: memset  */
	cmd[0] |= (ent->opcode & CMDQ_0_OP_MASK) << CMDQ_0_OP_SHIFT;

	switch (ent->opcode) {
	case CMDQ_OP_TLBI_EL2_ALL:
	case CMDQ_OP_TLBI_NSNH_ALL:
		break;
	case CMDQ_OP_PREFETCH_CFG:
		cmd[0] |= (u64)ent->prefetch.sid << CMDQ_PREFETCH_0_SID_SHIFT;
		cmd[1] |= ent->prefetch.size << CMDQ_PREFETCH_1_SIZE_SHIFT;
		cmd[1] |= ent->prefetch.addr & CMDQ_PREFETCH_1_ADDR_MASK;
		break;
	case CMDQ_OP_CFGI_STE:
		cmd[0] |= (u64)ent->cfgi.sid << CMDQ_CFGI_0_SID_SHIFT;
		cmd[1] |= ent->cfgi.leaf ? CMDQ_CFGI_1_LEAF : 0;
		break;
	case CMDQ_OP_CFGI_ALL:
		/* Cover the entire SID range */
		cmd[1] |= CMDQ_CFGI_1_RANGE_MASK << CMDQ_CFGI_1_RANGE_SHIFT;
		break;
	/*
	 * Cover the specal cd desc of sid,
	 * and in our code only use this case.
	 */
	case CMDQ_OP_CFGI_CD:
		cmd[0] |= (u64)ent->cfgi.ssid << CMDQ_CFGI_0_CD_SHIFT;
		cmd[0] |= (u64)ent->cfgi.sid << CMDQ_CFGI_0_SID_SHIFT;
		cmd[1] |= ent->cfgi.leaf ? CMDQ_CFGI_1_LEAF : 0;
		break;
	/* Cover the all cd descs of sid */
	case CMDQ_OP_CFGI_CD_ALL:
		cmd[0] |= (u64)ent->cfgi.sid << CMDQ_CFGI_0_SID_SHIFT;
		break;

	case CMDQ_OP_TLBI_NH_VA:
		cmd[0] |= (u64)ent->tlbi.asid << CMDQ_TLBI_0_ASID_SHIFT;
		cmd[1] |= ent->tlbi.leaf ? CMDQ_TLBI_1_LEAF : 0;
		cmd[1] |= ent->tlbi.addr & CMDQ_TLBI_1_VA_MASK;
		break;
	case CMDQ_OP_TLBI_S2_IPA:
		cmd[0] |= (u64)ent->tlbi.vmid << CMDQ_TLBI_0_VMID_SHIFT;
		cmd[1] |= ent->tlbi.leaf ? CMDQ_TLBI_1_LEAF : 0;
		cmd[1] |= ent->tlbi.addr & CMDQ_TLBI_1_IPA_MASK;
		break;
	case CMDQ_OP_TLBI_NH_ASID:
		cmd[0] |= (u64)ent->tlbi.asid << CMDQ_TLBI_0_ASID_SHIFT;
	/* Fallthrough */
	case CMDQ_OP_TLBI_S12_VMALL:
		cmd[0] |= (u64)ent->tlbi.vmid << CMDQ_TLBI_0_VMID_SHIFT;
		break;
	case CMDQ_OP_CMD_SYNC:
		cmd[0] |= CMDQ_SYNC_0_CS_SEV;
		break;
	default:
		return -ENOENT;
	}

	return 0;
}

static void arm_smmu_cmdq_skip_err(struct arm_smmu_device *smmu)
{
	static const char * const cerror_str[] = {
		[CMDQ_ERR_CERROR_NONE_IDX] = "No error",
		[CMDQ_ERR_CERROR_ILL_IDX] = "Illegal command",
		[CMDQ_ERR_CERROR_ABT_IDX] = "Abort on command fetch",
	};

	u32 i;
	u64 cmd[CMDQ_ENT_DWORDS];
	struct arm_smmu_queue *q = &smmu->cmdq.q;
	u32 cons = readl_relaxed(q->cons_reg);
	u32 idx = cons >> CMDQ_ERR_SHIFT & CMDQ_ERR_MASK;
	struct arm_smmu_cmdq_ent cmd_sync = {
		.opcode = CMDQ_OP_CMD_SYNC,
	};

	dev_err(smmu->dev, "CMDQ error (cons 0x%08x): %s\n", cons,
		cerror_str[idx]);

	switch (idx) {
	case CMDQ_ERR_CERROR_ILL_IDX:
		break;
	case CMDQ_ERR_CERROR_ABT_IDX:
		dev_err(smmu->dev, "retrying command fetch\n");
		return;
	case CMDQ_ERR_CERROR_NONE_IDX:
		return;
	}

	/*
	 * We may have concurrent producers, so we need to be careful
	 * not to touch any of the shadow cmdq state.
	 */
	queue_read(cmd, Q_ENT(q, cons), q->ent_dwords);
	dev_err(smmu->dev, "skipping command in error state:\n");
	for (i = 0; i < ARRAY_SIZE(cmd); ++i)
		dev_err(smmu->dev, "\t0x%016llx\n", (unsigned long long)cmd[i]);

	/* Convert the erroneous command into a CMD_SYNC */
	if (arm_smmu_cmdq_build_cmd(cmd, &cmd_sync)) {
		dev_err(smmu->dev, "failed to convert to CMD_SYNC\n");
		return;
	}

	queue_write(Q_ENT(q, cons), cmd, q->ent_dwords);
}

static void arm_smmu_cmdq_issue_cmd(
	struct arm_smmu_device *smmu, struct arm_smmu_cmdq_ent *ent)
{
	u64 cmd[CMDQ_ENT_DWORDS];
	unsigned long flags;
	bool wfe = !!(smmu->features & ARM_SMMU_FEAT_SEV);
	struct arm_smmu_queue *q = &smmu->cmdq.q;

	if (smmu->status != smmu_enable) {
		pr_err("%s,smmu is not enabled,id:%d \n",
			__func__, smmu->smmuid);
		return;
	}

	if (arm_smmu_cmdq_build_cmd(cmd, ent)) {
		dev_warn(smmu->dev, "ignoring unknown CMDQ opcode 0x%x\n",
			ent->opcode);
		return;
	}

	spin_lock_irqsave(&smmu->cmdq.lock, flags);
	while (queue_insert_raw(q, cmd) == -ENOSPC) {
		if (queue_poll_cons(q, false, wfe))
			dev_err_ratelimited(smmu->dev, "CMDQ timeout\n");
	}

	if (ent->opcode == CMDQ_OP_CMD_SYNC && queue_poll_cons(q, true, wfe))
		dev_err_ratelimited(smmu->dev, "CMD_SYNC timeout\n");
	spin_unlock_irqrestore(&smmu->cmdq.lock, flags);
}

/* Context descriptor manipulation functions */
static u64 arm_smmu_cpu_tcr_to_cd(u64 tcr)
{
	u64 val = 0;

	/* Repack the TCR. Just care about TTBR0 for now */
	val |= ARM_SMMU_TCR2CD(tcr, T0SZ);
	val |= ARM_SMMU_TCR2CD(tcr, TG0);
	val |= ARM_SMMU_TCR2CD(tcr, IRGN0);
	val |= ARM_SMMU_TCR2CD(tcr, ORGN0);
	val |= ARM_SMMU_TCR2CD(tcr, SH0);
	val |= ARM_SMMU_TCR2CD(tcr, EPD0);
	val |= ARM_SMMU_TCR2CD(tcr, EPD1);
	val |= ARM_SMMU_TCR2CD(tcr, IPS);
	val |= ARM_SMMU_TCR2CD(tcr, TBI0);

	return val;
}

static void arm_smmu_write_ctx_desc(
	struct hisi_smmu_group *sgrp, struct arm_smmu_s1_cfg *cfg)
{
	u64 val;

	/*
	 * We don't need to issue any invalidation here, as we'll invalidate
	 * the STE when installing the new entry anyway.
	 */

	val = arm_smmu_cpu_tcr_to_cd(cfg->cd.tcr) |
#ifdef __BIG_ENDIAN
	      CTXDESC_CD_0_ENDI |
#endif
	      CTXDESC_CD_0_S | CTXDESC_CD_0_R | CTXDESC_CD_0_A |
	      CTXDESC_CD_0_ASET_PRIVATE | CTXDESC_CD_0_AA64 |
	      (u64)cfg->cd.asid << CTXDESC_CD_0_ASID_SHIFT | CTXDESC_CD_0_V;

	cfg->cdptr[0] = cpu_to_le64(val);

	val = cfg->cd.ttbr & CTXDESC_CD_1_TTB0_MASK << CTXDESC_CD_1_TTB0_SHIFT;
	cfg->cdptr[1] = cpu_to_le64(val);

	cfg->cdptr[3] = cpu_to_le64(cfg->cd.mair << CTXDESC_CD_3_MAIR_SHIFT);
}

/* Stream table manipulation functions */
static void arm_smmu_write_strtab_l1_desc(
	__le64 *dst, struct arm_smmu_strtab_l1_desc *desc)
{
	u64 val = 0;

	val |= (desc->span & STRTAB_L1_DESC_SPAN_MASK)
	       << STRTAB_L1_DESC_SPAN_SHIFT;
	val |= desc->l2ptr_dma &
	       STRTAB_L1_DESC_L2PTR_MASK << STRTAB_L1_DESC_L2PTR_SHIFT;

	*dst = cpu_to_le64(val);
}

static void arm_smmu_write_strtab_ent(struct arm_smmu_device *smmu,
	u32 sid, __le64 *dst, struct arm_smmu_strtab_ent *ste)
{
	/*
	 * This is hideously complicated, but we only really care about
	 * three cases at the moment:
	 *
	 * 1. Invalid (all zero) -> Bypass  (init)
	 * 2. Bypass -> translation (attach)
	 * 3. Translation -> bypass (detach)
	 *
	 */
	u64 val = le64_to_cpu(dst[0]);
	val &= ~(STRTAB_STE_0_CFG_MASK << STRTAB_STE_0_CFG_SHIFT);
	if (ste->valid)
		val |= STRTAB_STE_0_V;
	else
		val &= ~STRTAB_STE_0_V;

	if (ste->bypass)
		val |= STRTAB_STE_0_CFG_BYPASS;
	else
		val |= STRTAB_STE_0_CFG_S1_TRANS;

	if (ste->cdtab_cfg)
		val |= ste->cdtab_cfg->cdtab_dma &
		       STRTAB_STE_0_S1CTXPTR_MASK
			       << STRTAB_STE_0_S1CTXPTR_SHIFT;
	/*
	 * number of CDs pointed to by
	 * S1ContextPtr,check cdmax
	 */
	val |= 6UL << STRTAB_STE_0_S1CDMAX_SHIFT;

	dst[0] = cpu_to_le64(val);
}

static void arm_smmu_init_bypass_stes(u64 *strtab, unsigned int nent)
{
	unsigned int i;
	struct arm_smmu_strtab_ent ste = {
		.valid = true,
		.bypass = true,
		.cdtab_cfg = NULL,
	};

	for (i = 0; i < nent; ++i) {
		arm_smmu_write_strtab_ent(NULL, 0, strtab, &ste);
		strtab += STRTAB_STE_DWORDS;
	}
}

/* IRQ and event handlers */
static irqreturn_t arm_smmu_evtq_handler(int irq, void *dev)
{
	irqreturn_t ret = IRQ_WAKE_THREAD;
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->evtq.q;

	/*
	 * Not much we can do on overflow, so scream and pretend we're
	 * trying harder.
	 */
	if (queue_sync_prod(q) == -EOVERFLOW)
		dev_err(smmu->dev, "EVTQ overflow detected -- events lost\n");
	else if (queue_empty(q))
		ret = IRQ_NONE;
	hisi_evt_flag_set(smmu);
	wake_up_interruptible(&smmu->wait_event_wq);
	return ret;
}

static irqreturn_t arm_smmu_cmdq_sync_handler(int irq, void *dev)
{
	/* We don't actually use CMD_SYNC interrupts for anything */
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_gerror_handler(int irq, void *dev)
{
	u32 gerror, gerrorn;
	struct arm_smmu_device *smmu = dev;

	gerror = readl_relaxed(smmu->base + ARM_SMMU_GERROR);
	gerrorn = readl_relaxed(smmu->base + ARM_SMMU_GERRORN);

	gerror ^= gerrorn;
	if (!(gerror & GERROR_ERR_MASK))
		return IRQ_NONE; /* No errors pending */

	dev_warn(smmu->dev, "unexpected global error reported (0x%08x), this "
			    "could be serious\n",
		gerror);

	if (gerror & GERROR_SFM_ERR) {
		dev_err(smmu->dev,
			"device has entered Service Failure Mode!\n");
		arm_smmu_device_disable(smmu);
	}

	if (gerror & GERROR_MSI_GERROR_ABT_ERR)
		dev_warn(smmu->dev, "GERROR MSI write aborted\n");

	if (gerror & GERROR_MSI_PRIQ_ABT_ERR)
		dev_warn(smmu->dev, "PRIQ MSI write aborted\n");

	if (gerror & GERROR_MSI_EVTQ_ABT_ERR)
		dev_warn(smmu->dev, "EVTQ MSI write aborted\n");

	if (gerror & GERROR_MSI_CMDQ_ABT_ERR) {
		dev_warn(smmu->dev, "CMDQ MSI write aborted\n");
		arm_smmu_cmdq_sync_handler(irq, smmu->dev);
	}

	if (gerror & GERROR_PRIQ_ABT_ERR)
		dev_err(smmu->dev,
			"PRIQ write aborted -- events may have been lost\n");

	if (gerror & GERROR_EVTQ_ABT_ERR)
		dev_err(smmu->dev,
			"EVTQ write aborted -- events may have been lost\n");

	if (gerror & GERROR_CMDQ_ERR)
		arm_smmu_cmdq_skip_err(smmu);

	writel(gerror, smmu->base + ARM_SMMU_GERRORN);
	return IRQ_HANDLED;
}

/* IO_PGTABLE API */
static void __arm_smmu_tlb_sync(struct arm_smmu_device *smmu)
{
	struct arm_smmu_cmdq_ent cmd;

	cmd.opcode = CMDQ_OP_CMD_SYNC;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
}

/* Register bits */
#define ARM_32_LPAE_TCR_EAE (1 << 31)
#define ARM_64_LPAE_S2_TCR_RES1 (1 << 31)

#define ARM_LPAE_TCR_EPD1 (1 << 23)

#define ARM_LPAE_TCR_TG0_4K (0 << 14)
#define ARM_LPAE_TCR_TG0_64K (1 << 14)
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

static int hisi_svm_lpae_alloc_pgtable_s1(
	struct io_pgtable_cfg *cfg, void *cookie)
{
	u64 reg = 0;
	struct mm_struct *mm;
	unsigned long asid = 0;
	/* TCR */

	switch (cfg->oas) {
	case HISI_SMMU_ADDR_SIZE_32:
		reg |= (ARM_LPAE_TCR_PS_32_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case HISI_SMMU_ADDR_SIZE_36:
		reg |= (ARM_LPAE_TCR_PS_36_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case HISI_SMMU_ADDR_SIZE_40:
		reg |= (ARM_LPAE_TCR_PS_40_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case HISI_SMMU_ADDR_SIZE_42:
		reg |= (ARM_LPAE_TCR_PS_42_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case HISI_SMMU_ADDR_SIZE_44:
		reg |= (ARM_LPAE_TCR_PS_44_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case HISI_SMMU_ADDR_SIZE_48:
		reg |= (ARM_LPAE_TCR_PS_48_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	default:
		goto out;
	}

	reg |= (64ULL - cfg->ias) << ARM_LPAE_TCR_T0SZ_SHIFT;

	/* Disable speculative walks through TTBR1 */
	reg |= ARM_LPAE_TCR_EPD1;
	cfg->arm_lpae_s1_cfg.tcr = reg;

	/* MAIRs */
	reg = (ARM_LPAE_MAIR_ATTR_NC
		      << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_NC)) |
	      (ARM_LPAE_MAIR_ATTR_WBRWA << ARM_LPAE_MAIR_ATTR_SHIFT(
		       ARM_LPAE_MAIR_ATTR_IDX_CACHE)) |
	      (ARM_LPAE_MAIR_ATTR_DEVICE
		      << ARM_LPAE_MAIR_ATTR_SHIFT(ARM_LPAE_MAIR_ATTR_IDX_DEV));

	cfg->arm_lpae_s1_cfg.mair[0] = reg;
	cfg->arm_lpae_s1_cfg.mair[1] = 0;

	/* Ensure the empty pgd is visible before any actual TTBR write */
	wmb();

	mm = get_task_mm(current);
	if(!mm)
		goto out;
	hisi_svm_print(svm_trace, "%s, pid: %d, process name: %s,pgd:%pK\n",
		__func__, current->pid, current->comm, mm->pgd);
	asid = ASID(mm);
	cfg->arm_lpae_s1_cfg.ttbr[0] = virt_to_phys(mm->pgd) | (asid << 48);
	cfg->arm_lpae_s1_cfg.ttbr[1] = 0;
	mmput(mm);

	hisi_svm_print(svm_trace, "out %s\n", __func__);
	return 0;
out:
	hisi_svm_print(svm_error, "err out %s\n", __func__);
	return -EINVAL;
}

static void hisi_smmu_group_tlb_sync(void *cookie)
{
	struct list_head *p, *n;
	struct arm_smmu_device *smmu;
	struct hisi_smmu_group *smmu_grp;
	struct arm_smmu_domain *smmu_domain = cookie;

	if (!smmu_domain)
		return;

	smmu_grp = smmu_domain->smmu_grp;
	if (!smmu_grp)
		return;

	list_for_each_safe(p, n, &smmu_grp->smmu_list) {
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		if (!smmu) {
			hisi_svm_print(
				svm_error, "%s smmu is null\n", __func__);
			return;
		}
		__arm_smmu_tlb_sync(smmu);
	}
}

static void hisi_smmu_group_tlb_inv_context(void *cookie)
{
	struct list_head *p, *n;
	struct arm_smmu_device *smmu;
	struct hisi_smmu_group *smmu_grp;
	struct arm_smmu_domain *smmu_domain = cookie;

	if (!smmu_domain)
		return;

	smmu_grp = smmu_domain->smmu_grp;
	if (!smmu_grp)
		return;

	mutex_lock(&hisi_svmtlb_mutex);
	list_for_each_safe(p, n, &smmu_grp->smmu_list) {
		struct arm_smmu_cmdq_ent cmd;
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		if (!smmu) {
			hisi_svm_print(
				svm_error, "%s smmu is null\n", __func__);
			mutex_unlock(&hisi_svmtlb_mutex);
			return;
		}

		cmd.opcode = CMDQ_OP_TLBI_NH_ASID;
		cmd.tlbi.asid = smmu_domain->s1_cfg.cd.asid;
		cmd.tlbi.vmid = 0;

		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
		__arm_smmu_tlb_sync(smmu);

		invalid_tcu_cache(smmu);
	}
	mutex_unlock(&hisi_svmtlb_mutex);
}

static void hisi_smmu_group_tlb_inv_range_nosync(
	unsigned long iova, size_t size, size_t granule,
	bool leaf, void *cookie)
{
	size_t sz = 0;
	struct list_head *p, *n;
	struct arm_smmu_device *smmu;
	struct hisi_smmu_group *smmu_grp;
	struct arm_smmu_domain *smmu_domain = cookie;

	if (!smmu_domain)
		return;

	smmu_grp = smmu_domain->smmu_grp;
	if (!smmu_grp)
		return;

	mutex_lock(&hisi_svmtlb_mutex);
	list_for_each_safe(p, n, &smmu_grp->smmu_list) {
		struct arm_smmu_cmdq_ent cmd;
		cmd.tlbi.leaf = leaf;
		cmd.tlbi.addr = iova;
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		if (!smmu) {
			hisi_svm_print(
				svm_error, "%s smmu is null\n", __func__);
			mutex_unlock(&hisi_svmtlb_mutex);
			return;
		}
		sz = ALIGN(size, PAGE_SIZE);
		cmd.opcode = CMDQ_OP_TLBI_NH_VA;
		cmd.tlbi.asid = smmu_domain->s1_cfg.cd.asid;
		while (sz -= PAGE_SIZE) {
			cmd.tlbi.addr += PAGE_SIZE;
			arm_smmu_cmdq_issue_cmd(smmu, &cmd);
			__arm_smmu_tlb_sync(smmu);
		}
		invalid_tcu_cache(smmu);
	}
	mutex_unlock(&hisi_svmtlb_mutex);
}

static struct iommu_gather_ops arm_smmu_gather_ops = {
	.tlb_flush_all = hisi_smmu_group_tlb_inv_context,
	.tlb_add_flush = hisi_smmu_group_tlb_inv_range_nosync,
	.tlb_sync = hisi_smmu_group_tlb_sync,
};
/*lint -e429*/
static struct iommu_domain *arm_smmu_domain_alloc(unsigned int type)
{
	struct arm_smmu_domain *smmu_domain;

	if (!hisi_smmu_group) {
		hisi_svm_print(
			svm_error, "hisi_smmu_group is null %s\n", __func__);
		return NULL;
	}

	if (type != IOMMU_DOMAIN_UNMANAGED) {
		hisi_svm_print(svm_error, "%s type is invalidate %u\n",
			__func__, type);
		return NULL;
	}
	/*
	 * Allocate the domain and initialise some of its data structures.
	 * We can't really do anything meaningful until we've added a
	 * master.
	 */
	smmu_domain = kzalloc(sizeof(*smmu_domain), GFP_KERNEL);
	if (!smmu_domain) {
		hisi_svm_print(
			svm_error, "%s alloc smmu_domain failed\n", __func__);
		return NULL;
	}

	/* Domain attach the smmu group */
	smmu_domain->smmu_grp = hisi_smmu_group;
	mutex_init(&smmu_domain->init_mutex);
	spin_lock_init(&smmu_domain->pgtbl_lock);

	return &smmu_domain->domain;
}
/*lint +e429*/

static int arm_smmu_bitmap_alloc(unsigned long *map, int span)
{
	int idx, size = 1 << span;

	do {
		idx = find_first_zero_bit(map, size);
		if (idx == size)
			return -ENOSPC;
	} while (test_and_set_bit(idx, map));

	return idx;
}

static void arm_smmu_bitmap_free(unsigned long *map, int idx)
{
	clear_bit(idx, map);
}

static void arm_smmu_domain_free(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain;

	if (unlikely(!domain))
		return;
	smmu_domain = to_smmu_domain(domain);
	if (unlikely(!smmu_domain))
		return;

	kfree(smmu_domain);
}

#ifdef CONFIG_HISI_SVM_STE_BYPASS
static void hisi_smmu_flush_pgtbl(void *addr, size_t size)
{
	__flush_dcache_area(addr, size);
}

static int hisi_bypass_smmu_pgtable_free(struct hisi_smmu_group *smmu_grp)
{
	if (!smmu_grp)
		return -EPERM;
	if (smmu_grp->pmd)
		kfree(smmu_grp->pmd);
	if (smmu_grp->pgd)
		kfree(smmu_grp->pgd);
	return 0;
}
static int hisi_bypass_pgtable_set(smmu_pgd_t *pgd,
	smmu_pmd_t *pmd, unsigned long va)
{
	smmu_pgd_t *l2pgd;
	smmu_pmd_t *l2pmd;
	unsigned long pgdprot = SMMU_PGD_TYPE | PGD_PROT;
	unsigned long pmdprot = SMMU_PMD_DEFAULT | PTE_PROT | SMMU_PTE_USER;
	if (!pgd || !pmd)
		return -EPERM;
	/*
	 * create 2MB granule pagetable to ensure performace
	 */
	l2pgd = pgd + smmu_pgd_index(va);
	*l2pgd = virt_to_phys(pmd) | pgdprot;
	l2pmd = pmd + smmu_pmd_index(va);
	*l2pmd = (unsigned long)va | pmdprot;
	hisi_svm_print(svm_trace,
		"l2pgd:0x%llx, l2pmd:0x%llx,pgd_off:0x%lx, pmd_off:0x%lx\n",
		*l2pgd, *l2pmd, smmu_pgd_index(va), smmu_pmd_index(va));
	return 0;
}
static int hisi_bypass_smmu_pgtbl_create(
	struct arm_smmu_domain *smmu_domain)
{
	int i = 0, ret = 0;
	smmu_pgd_t *pgd;
	smmu_pmd_t *pmd;
	struct hisi_smmu_group *smmu_group = smmu_domain->smmu_grp;

	pgd = (smmu_pgd_t *)kzalloc(SZ_4K, GFP_KERNEL);
	if (!pgd) {
		hisi_svm_print(svm_error, "%s alloc pgd failed\n", __func__);
		return -ENOMEM;
	}
	pmd = (smmu_pgd_t *)kzalloc(SZ_16K, GFP_KERNEL);
	if (!pmd) {
		hisi_svm_print(svm_error, "%s alloc pmd failed\n", __func__);
		goto fallback_pgd;
	}
	for (i = 0; i < L2BYPASS_ADDR_NUM; i++) {
		ret = hisi_bypass_pgtable_set(pgd, pmd, smmu_group->bypass_addr[i]);
		if (ret) {
			hisi_svm_print(svm_error,
				"%s set pgtbl[%d] addr:0x%x failed\n",
				__func__, i, smmu_group->bypass_addr[i]);
			goto fallback_pmd;
		}
	}
	hisi_smmu_flush_pgtbl(pgd, SZ_4K);
	hisi_smmu_flush_pgtbl(pmd, SZ_16K);
	smmu_group->pgd = pgd;
	smmu_group->pmd = pmd;

	return 0;

fallback_pmd:
	kfree(pmd);
fallback_pgd:
	kfree(pgd);
	return -ENOMEM;
}

static int hisi_bypass_cd_create(
	struct arm_smmu_domain *smmu_domain, struct io_pgtable_cfg *pgtbl_cfg)
{
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct hisi_smmu_group *smmu_group = smmu_domain->smmu_grp;

	cfg->cdptr =
		&smmu_group->cdtab_cfg_bypass.cdtab[(long)(cfg->cd.ssid) * (1 << 3)];
	cfg->cdptr_dma = smmu_group->cdtab_cfg_bypass.cdtab_dma +
			 (long)(cfg->cd.ssid) * (CTXDESC_CD_DWORDS << 3);
	cfg->cd.ttbr =
		virt_to_phys(smmu_group->pgd) | ((long)(cfg->cd.asid) << 48);
	hisi_svm_print(svm_trace, "%s,ssid:%d,ttbr:0x%llx\n",
		__func__, cfg->cd.ssid, cfg->cd.ttbr);
	arm_smmu_write_ctx_desc(smmu_group, cfg);

	return 0;
}

static int hisi_smmu_ste_bypass(
	struct arm_smmu_domain *smmu_domain, struct io_pgtable_cfg *pgtbl_cfg)
{
	int ret = 0;

	if (!smmu_domain || !pgtbl_cfg)
		return -EPERM;

	ret = hisi_bypass_smmu_pgtbl_create(smmu_domain);
	if (ret) {
		hisi_svm_print(svm_error,
			"%s hisi_bypass_pgtbl_create failed!!\n", __func__);
		return ret;
	}

	ret = hisi_bypass_cd_create(smmu_domain, pgtbl_cfg);
	if (ret) {
		hisi_svm_print(svm_error,
			"%s hisi_bypass_cd_create failed!!\n", __func__);
		return ret;
	}

	return 0;
}
#endif

static int arm_smmu_domain_finalise_s1(
	struct arm_smmu_domain *smmu_domain, struct io_pgtable_cfg *pgtbl_cfg)
{
	int ssid = 0;
	unsigned long asid = 0;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct hisi_smmu_group *smmu_group = smmu_domain->smmu_grp;
	struct mm_struct *mm = NULL;

	mm = get_task_mm(current);
	if (mm == NULL) {
		hisi_svm_print(svm_error, "%s get mm is null\n", __func__);
		return -EINVAL;
	}
	asid = ASID(mm);
	mmput(mm);

	ssid = arm_smmu_bitmap_alloc(
		smmu_group->ssid_map, smmu_group->ssid_bits);
	if (ssid < 0) {
		hisi_svm_print(svm_error, "%s,arm_smmu_bitmap_alloc failed!!\n",
			__func__);
		return -EINVAL;
	}
	cfg->cdptr = &smmu_group->cdtab_cfg.cdtab[(long)ssid * (1 << 3)];
	cfg->cdptr_dma = smmu_group->cdtab_cfg.cdtab_dma +
		(long)ssid * (CTXDESC_CD_DWORDS << 3);

	cfg->cd.ssid = (u16)ssid;
	cfg->cd.asid = (u16)asid;
	cfg->cd.ttbr = pgtbl_cfg->arm_lpae_s1_cfg.ttbr[0];
	cfg->cd.tcr = pgtbl_cfg->arm_lpae_s1_cfg.tcr;
	cfg->cd.mair = pgtbl_cfg->arm_lpae_s1_cfg.mair[0];
	hisi_svm_print(svm_trace, "%s,alloc ssid:%d, asid:0x%lx,ttbr:0x%llx\n", __func__,
		ssid, asid, cfg->cd.ttbr);
	/* only build the cd desc */
	arm_smmu_write_ctx_desc(smmu_group, cfg);

	return 0;
}


static int arm_smmu_domain_finalise(struct arm_smmu_domain *smmu_domain)
{
	int ret;
	struct io_pgtable_cfg pgtbl_cfg;
	struct io_pgtable_ops *pgtbl_ops = NULL;
	struct hisi_smmu_group *grp = smmu_domain->smmu_grp;

	pgtbl_cfg = (struct io_pgtable_cfg){
		.pgsize_bitmap = arm_smmu_ops.pgsize_bitmap,
		.ias = VA_BITS,
		.oas = grp->oas,
		.tlb = &arm_smmu_gather_ops,
	};

	ret = hisi_svm_lpae_alloc_pgtable_s1(&pgtbl_cfg, smmu_domain);
	if (ret) {
		hisi_svm_print(svm_error,
			"%s:hisi_svm_lpae_alloc_pgtable_s1 failed\n", __func__);
		return -ENOMEM;
	}

	/* When the page tbl ops is NULL
	 * the page size is not make sense.
	 */
	smmu_domain->pgtbl_ops = pgtbl_ops;
	arm_smmu_ops.pgsize_bitmap = pgtbl_cfg.pgsize_bitmap;

	ret = arm_smmu_domain_finalise_s1(smmu_domain, &pgtbl_cfg);
	if (ret)
		goto alloc_pgtable_err;

#ifdef CONFIG_HISI_SVM_STE_BYPASS
	if (grp->bypass_status == bypass_enable) {
		ret = hisi_smmu_ste_bypass(smmu_domain, &pgtbl_cfg);
		if (ret) {
			hisi_svm_print(svm_error, "error out %s:hisi_smmu_ste_bypass failed\n", __func__);
			goto alloc_pgtable_err;
		}
	}
#endif

	return ret;

alloc_pgtable_err:
	hisi_svm_print(svm_error, "error out %s\n", __func__);
	return ret;
}

static __le64 *arm_smmu_get_step_for_sid(
	struct arm_smmu_device *smmu, u32 sid)
{
	__le64 *step = NULL;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
		struct arm_smmu_strtab_l1_desc *l1_desc;
		int idx;

		/* Two-level walk */
		idx = (sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS;
		l1_desc = &cfg->l1_desc[idx];
		idx = (sid & ((1 << STRTAB_SPLIT) - 1)) * STRTAB_STE_DWORDS;
		step = &l1_desc->l2ptr[idx];
	} else {
		/* Simple linear lookup */
		step = &cfg->strtab[(long)sid * STRTAB_STE_DWORDS];
	}

	return step;
}

static int arm_smmu_enable_cd(
	struct hisi_smmu_group *grp, struct arm_smmu_domain *dom)
{
	u32 sid;
	struct list_head *p, *n;
	struct arm_smmu_device *smmu;

	/* maybe can prefetch */
	list_for_each_safe(p, n, &grp->smmu_list) {
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		if (!smmu || (smmu->status != smmu_enable)) {
			hisi_svm_print(
				svm_error, "%s smmu invalid,status:%d\n",
				__func__, smmu->status);
			return -EPERM;
		}

		invalid_tcu_cache(smmu);
		for (sid = 0; sid < smmu->strtab_cfg.num_l1_ents; sid++) {
			struct arm_smmu_cmdq_ent cmd;
			cmd.opcode = CMDQ_OP_CFGI_CD;
			cmd.cfgi.ssid = dom->s1_cfg.cd.ssid;
			cmd.cfgi.sid  = sid;
			cmd.cfgi.leaf = 1;
			arm_smmu_cmdq_issue_cmd(smmu, &cmd);
			cmd.opcode = CMDQ_OP_CMD_SYNC;
			arm_smmu_cmdq_issue_cmd(smmu, &cmd);
		}
	}
	return 0;
}

static void arm_smmu_disable_cd(
	struct hisi_smmu_group *grp, struct arm_smmu_domain *dom)
{
	u32 sid;
	struct list_head *p, *n;
	struct arm_smmu_device *smmu;

	/* maybe can prefetch */
	list_for_each_safe(p, n, &grp->smmu_list) {
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		if (!smmu) {
			hisi_svm_print(
				svm_error, "%s smmu is null\n", __func__);
			return;
		}
		invalid_tcu_cache(smmu);
		for (sid = 0; sid < smmu->strtab_cfg.num_l1_ents; sid++) {
			struct arm_smmu_cmdq_ent cmd;
			cmd.opcode = CMDQ_OP_CFGI_CD;
			cmd.cfgi.ssid = dom->s1_cfg.cd.ssid;
			cmd.cfgi.sid = sid;
			cmd.cfgi.leaf = 1;
			arm_smmu_cmdq_issue_cmd(smmu, &cmd);

			cmd.opcode = CMDQ_OP_CMD_SYNC;
			arm_smmu_cmdq_issue_cmd(smmu, &cmd);
		}
	}
}

static int arm_smmu_attach_dev(
	struct iommu_domain *domain, struct device *dev)
{
	int ret = 0;
	struct hisi_smmu_group *smmu_group = NULL;
	struct arm_smmu_domain *smmu_domain = NULL;

	if (!domain) {
		hisi_svm_print(svm_error, "%s domain is null\n", __func__);
		return -EINVAL;
	}
	smmu_domain = to_smmu_domain(domain);
	if (!smmu_domain) {
		hisi_svm_print(svm_error, "%s smmu_domain is null\n", __func__);
		return -EINVAL;
	}
	smmu_group = smmu_domain->smmu_grp;
	if (!smmu_group) {
		hisi_svm_print(svm_error, "%s smmu_group is null\n", __func__);
		return -EINVAL;
	}
	mutex_lock(&smmu_group->sgrp_mtx);

	ret = arm_smmu_domain_finalise(smmu_domain);
	if (ret)
		goto out_unlock;

	/* enable cd */
	ret = arm_smmu_enable_cd(smmu_group, smmu_domain);

out_unlock:
	mutex_unlock(&smmu_group->sgrp_mtx);
	return ret;
}

static void arm_smmu_domain_draft(struct arm_smmu_domain *smmu_domain)
{

	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct hisi_smmu_group *smmu_group = smmu_domain->smmu_grp;

	arm_smmu_bitmap_free(smmu_group->ssid_map, cfg->cd.ssid);
	/* clear cd decs */
	memset(cfg->cdptr, 0x0, CTXDESC_CD_DWORDS << 3);/* unsafe_function_ignore: memset  */
}

static void arm_smmu_detach_dev(
	struct iommu_domain *domain, struct device *dev)
{
	struct hisi_smmu_group *smmu_group;
	struct arm_smmu_domain *smmu_domain;

	if (unlikely(!domain)) {
		hisi_svm_print(svm_error, "domain is null %s\n", __func__);
		return;
	}

	smmu_domain = to_smmu_domain(domain);
	if (unlikely(!smmu_domain)) {
		hisi_svm_print(svm_error, "smmu_domain is null %s\n", __func__);
		return;
	}

	smmu_group = smmu_domain->smmu_grp;
	if (unlikely(!smmu_group)) {
		hisi_svm_print(svm_error, "smmu_group is null %s\n", __func__);
		return;
	}

	mutex_lock(&smmu_group->sgrp_mtx);
	arm_smmu_domain_draft(smmu_domain);
	/* Invalidate the ctx desc table */
	arm_smmu_disable_cd(smmu_group, smmu_domain);

#ifdef CONFIG_HISI_SVM_STE_BYPASS
	if (smmu_group->bypass_status == bypass_enable)
	hisi_bypass_smmu_pgtable_free(smmu_group);
#endif

	mutex_unlock(&smmu_group->sgrp_mtx);
}

/* Probing and initialisation functions */
static int arm_smmu_init_one_queue(struct arm_smmu_device *smmu,
	struct arm_smmu_queue *q, unsigned long prod_off,
	unsigned long cons_off, size_t dwords)
{
	size_t qsz = (((size_t)1 << q->max_n_shift) * dwords) << 3;

	q->base = dma_alloc_coherent(smmu->dev, qsz, &q->base_dma, GFP_KERNEL);
	if (!q->base) {
		dev_err(smmu->dev, "failed to allocate queue (0x%zx bytes)\n",
			qsz);
		return -ENOMEM;
	}

	q->prod_reg = smmu->base + prod_off;
	q->cons_reg = smmu->base + cons_off;
	q->ent_dwords = dwords;
	q->q_base = q->base_dma & (Q_BASE_ADDR_MASK << Q_BASE_ADDR_SHIFT);
	q->q_base |= (q->max_n_shift & Q_BASE_LOG2SIZE_MASK)
		     << Q_BASE_LOG2SIZE_SHIFT;

	q->prod = q->cons = 0;
	return 0;
}

static void arm_smmu_free_one_queue(
	struct arm_smmu_device *smmu, struct arm_smmu_queue *q)
{
	size_t qsz = (((size_t)1 << q->max_n_shift) * q->ent_dwords) << 3;

	dma_free_coherent(smmu->dev, qsz, q->base, q->base_dma);
}

static void arm_smmu_zeroed_one_queue(
	struct arm_smmu_device *smmu, struct arm_smmu_queue *q)
{
	size_t qsz = (((size_t)1 << q->max_n_shift) * q->ent_dwords) << 3;

	memset(q->base, 0, qsz);/* unsafe_function_ignore: memset  */
}

static void arm_smmu_free_queues(struct arm_smmu_device *smmu)
{
	arm_smmu_free_one_queue(smmu, &smmu->cmdq.q);
	arm_smmu_free_one_queue(smmu, &smmu->evtq.q);

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		arm_smmu_free_one_queue(smmu, &smmu->priq.q);
}

static void arm_smmu_zeroed_queues(struct arm_smmu_device *smmu)
{
	arm_smmu_zeroed_one_queue(smmu, &smmu->cmdq.q);
	arm_smmu_zeroed_one_queue(smmu, &smmu->evtq.q);

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		arm_smmu_zeroed_one_queue(smmu, &smmu->priq.q);
}

static int arm_smmu_init_queues(struct arm_smmu_device *smmu)
{
	int ret;

	/* cmdq */
	spin_lock_init(&smmu->cmdq.lock);
	ret = arm_smmu_init_one_queue(smmu, &smmu->cmdq.q, ARM_SMMU_CMDQ_PROD,
		ARM_SMMU_CMDQ_CONS, CMDQ_ENT_DWORDS);
	if (ret)
		goto out;

	/* evtq */
	ret = arm_smmu_init_one_queue(smmu, &smmu->evtq.q, ARM_SMMU_EVTQ_PROD,
		ARM_SMMU_EVTQ_CONS, EVTQ_ENT_DWORDS);
	if (ret)
		goto out_free_cmdq;

	/* priq */
	if (!(smmu->features & ARM_SMMU_FEAT_PRI))
		return 0;

	ret = arm_smmu_init_one_queue(smmu, &smmu->priq.q, ARM_SMMU_PRIQ_PROD,
		ARM_SMMU_PRIQ_CONS, PRIQ_ENT_DWORDS);
	if (ret)
		goto out_free_evtq;

	return 0;

out_free_evtq:
	arm_smmu_free_one_queue(smmu, &smmu->evtq.q);
out_free_cmdq:
	arm_smmu_free_one_queue(smmu, &smmu->cmdq.q);
out:
	return ret;
}

static void arm_smmu_free_l2_strtab(struct arm_smmu_device *smmu)
{
	u32 i;
	size_t size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	size = (size_t)1 << (STRTAB_SPLIT + ilog2(STRTAB_STE_DWORDS) + 3);
	for (i = 0; i < cfg->num_l1_ents; ++i) {
		struct arm_smmu_strtab_l1_desc *desc = &cfg->l1_desc[i];

		if (!desc->l2ptr)
			continue;

		dma_free_coherent(
			smmu->dev, size, desc->l2ptr, desc->l2ptr_dma);
	}
}

static void arm_smmu_zeroed_l2_strtab(struct arm_smmu_device *smmu)
{
	u32 i;
	size_t size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	size = (size_t)1 << (STRTAB_SPLIT + ilog2(STRTAB_STE_DWORDS) + 3);
	for (i = 0; i < cfg->num_l1_ents; ++i) {
		struct arm_smmu_strtab_l1_desc *desc = &cfg->l1_desc[i];

		if (!desc->l2ptr)
			continue;

		memset(desc->l2ptr, 0, size);/* unsafe_function_ignore: memset  */
	}
}

static bool arm_smmu_sid_in_range(struct arm_smmu_device *smmu, u32 sid)
{
	unsigned long limit = smmu->strtab_cfg.num_l1_ents;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		limit *= 1UL << STRTAB_SPLIT;

	return sid < limit;
}

static int arm_smmu_init_l2_strtab(struct arm_smmu_device *smmu, u32 sid)
{
	u32 i;
	size_t size;
	void *strtab;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	struct arm_smmu_strtab_l1_desc *desc = NULL;

	if (!arm_smmu_sid_in_range(smmu, sid))
		return -ERANGE;

	desc = &cfg->l1_desc[sid >> STRTAB_SPLIT];

	if (desc->l2ptr)
		return 0;

	size = (size_t)1 << (STRTAB_SPLIT + ilog2(STRTAB_STE_DWORDS) + 3);
	strtab = &cfg->strtab[(sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS];

	desc->span = STRTAB_SPLIT + 1;
	desc->l2ptr = dma_zalloc_coherent(
		smmu->dev, size, &desc->l2ptr_dma, GFP_KERNEL);
	if (!desc->l2ptr) {
		dev_err(smmu->dev,
			"failed to allocate l2 stream table for SID %u\n", sid);
		return -ENOMEM;
	}

	arm_smmu_init_bypass_stes(desc->l2ptr, 1 << STRTAB_SPLIT);

	for (i = 0; i < cfg->num_l1_ents; ++i) {
		arm_smmu_write_strtab_l1_desc(strtab, &cfg->l1_desc[i]);
		strtab += STRTAB_L1_DESC_DWORDS << 3;
	}

	return 0;
}

static int arm_smmu_init_l1_strtab(struct arm_smmu_device *smmu)
{
	int ret;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	size_t size = sizeof(*cfg->l1_desc) * cfg->num_l1_ents;

	cfg->l1_desc = devm_kzalloc(smmu->dev, size, GFP_KERNEL);
	if (!cfg->l1_desc)
		return -ENOMEM;

	ret = arm_smmu_init_l2_strtab(smmu, 0);

	return 0;
}

static int arm_smmu_init_strtab_2lvl(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size, l1size;
	int ret;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	/*
	 * If we can resolve everything with a single L2 table, then we
	 * just need a single L1 descriptor. Otherwise, calculate the L1
	 * size, capped to the SIDSIZE.
	 */
	if (smmu->sid_bits < STRTAB_SPLIT) {
		size = 0;
	} else {
		size = STRTAB_L1_SZ_SHIFT - (ilog2(STRTAB_L1_DESC_DWORDS) + 3);
		size = min(size, smmu->sid_bits - STRTAB_SPLIT);
	}
	cfg->num_l1_ents = 1 << size;

	size += STRTAB_SPLIT;
	if (size < smmu->sid_bits)
		dev_warn(smmu->dev,
			"2-level strtab only covers %u/%u bits of SID\n", size,
			smmu->sid_bits);

	l1size = cfg->num_l1_ents * (STRTAB_L1_DESC_DWORDS << 3);
	strtab = dma_zalloc_coherent(
		smmu->dev, l1size, &cfg->strtab_dma, GFP_KERNEL);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate l1 stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;

	/* Configure strtab_base_cfg for 2 levels */
	reg = STRTAB_BASE_CFG_FMT_2LVL;
	reg |= (size & STRTAB_BASE_CFG_LOG2SIZE_MASK)
	       << STRTAB_BASE_CFG_LOG2SIZE_SHIFT;
	reg |= (STRTAB_SPLIT & STRTAB_BASE_CFG_SPLIT_MASK)
	       << STRTAB_BASE_CFG_SPLIT_SHIFT;
	cfg->strtab_base_cfg = reg;

	ret = arm_smmu_init_l1_strtab(smmu);
	if (ret)
		dma_free_coherent(smmu->dev, l1size, strtab, cfg->strtab_dma);

	return ret;
}

static int arm_smmu_init_strtab_linear(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	/* desc size is 64 bytes and we only surpport 0~43 id */
	size = (1 << smmu->sid_bits) * (STRTAB_STE_DWORDS << 3);
	strtab = dma_zalloc_coherent(
		smmu->dev, size, &cfg->strtab_dma, GFP_KERNEL);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate linear stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;
	cfg->num_l1_ents = 1 << smmu->sid_bits;

	/* Configure strtab_base_cfg for a linear table covering all SIDs */
	reg = STRTAB_BASE_CFG_FMT_LINEAR;
	reg |= (smmu->sid_bits & STRTAB_BASE_CFG_LOG2SIZE_MASK)
	       << STRTAB_BASE_CFG_LOG2SIZE_SHIFT;
	cfg->strtab_base_cfg = reg;

	return 0;
}

static int arm_smmu_init_strtab(struct arm_smmu_device *smmu)
{
	u64 reg;
	int ret;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		ret = arm_smmu_init_strtab_2lvl(smmu);
	else
		ret = arm_smmu_init_strtab_linear(smmu);

	if (ret)
		return ret;

	/* Set the strtab base address */
	reg = smmu->strtab_cfg.strtab_dma &
	      (STRTAB_BASE_ADDR_MASK << STRTAB_BASE_ADDR_SHIFT);

	smmu->strtab_cfg.strtab_base = reg;

	return 0;
}

static void arm_smmu_free_strtab(struct arm_smmu_device *smmu)
{
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	u32 size = cfg->num_l1_ents;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
		arm_smmu_free_l2_strtab(smmu);
		size *= STRTAB_L1_DESC_DWORDS << 3;
	} else {
		size *= STRTAB_STE_DWORDS * 3;
	}

	dma_free_coherent(smmu->dev, size, cfg->strtab, cfg->strtab_dma);
}

static void arm_smmu_zeroed_strtab(struct arm_smmu_device *smmu)
{
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	u32 size = cfg->num_l1_ents;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
		arm_smmu_zeroed_l2_strtab(smmu);
		size *= STRTAB_L1_DESC_DWORDS << 3;
	} else {
		size *= STRTAB_STE_DWORDS * 3;
	}

	memset(cfg->strtab, 0, size);/* unsafe_function_ignore: memset  */
}

static int arm_smmu_init_structures(struct arm_smmu_device *smmu)
{
	int ret;

	ret = arm_smmu_init_queues(smmu);
	if (ret)
		return ret;

	ret = arm_smmu_init_strtab(smmu);
	if (ret)
		goto out_free_queues;

	return 0;

out_free_queues:
	arm_smmu_free_queues(smmu);
	hisi_svm_print(svm_error, "err out %s ret %d\n", __func__, ret);

	return ret;
}

static void arm_smmu_free_structures(struct arm_smmu_device *smmu)
{
	arm_smmu_free_strtab(smmu);
	arm_smmu_free_queues(smmu);
	hisi_structure_init_flag_unset(smmu);
}

static void arm_smmu_zeroed_structures(struct arm_smmu_device *smmu)
{
	arm_smmu_zeroed_strtab(smmu);
	arm_smmu_zeroed_queues(smmu);
}

static int arm_smmu_write_reg_sync(struct arm_smmu_device *smmu, u32 val,
	unsigned int reg_off, unsigned int ack_off)
{
	u32 reg = 0;

	writel_relaxed(val, smmu->base + reg_off);
	return readl_relaxed_poll_timeout(smmu->base + ack_off, reg, reg == val,
		1, ARM_SMMU_POLL_TIMEOUT_US);
}

int hisi_smmu_reg_set(struct arm_smmu_device *smmu, unsigned int req_off,
	unsigned int ack_off, unsigned int req_bit, unsigned int ack_bit)
{
	u32 reg = 0;
	u32 val = 0;

	val = readl_relaxed(smmu->base + req_off);
	val |= req_bit;
	writel_relaxed(val, smmu->base + req_off);
	return readl_relaxed_poll_timeout(smmu->base + ack_off, reg,
		reg & ack_bit, 1, ARM_SMMU_POLL_TIMEOUT_US);
}

int hisi_smmu_reg_unset(struct arm_smmu_device *smmu, unsigned int req_off,
	unsigned int ack_off, unsigned int req_bit, unsigned int ack_bit)
{
	u32 reg = 0;

	reg = readl_relaxed(smmu->base + req_off);
	reg &= ~req_bit;
	writel_relaxed(reg, smmu->base + req_off);
	return readl_relaxed_poll_timeout(smmu->base + ack_off, reg,
		!(reg & ack_bit), 1, ARM_SMMU_POLL_TIMEOUT_US);
}

static irqreturn_t arm_smmu_global_handler(int irq, void *data)
{
	u32 irq_status = 0, raw_irq_status = 0;
	u32 reg = (TCU_EVENT_Q_IRQ_CLR | TCU_CMD_SYNC_IRQ_CLR |
			TCU_GERROR_IRQ_CLR | TCU_EVENTTO_CLR);
	struct arm_smmu_device *smmu = (struct arm_smmu_device *)data;
	irq_status = readl_relaxed(smmu->base + SMMU_IRPT_STAT_NS);
	raw_irq_status = readl_relaxed(smmu->base + SMMU_IRPT_RAW_NS);
	hisi_svm_print(svm_info, "into %s,status:0x%x,raw_status:0x%x\n",
			__func__, irq_status, raw_irq_status);
	writel_relaxed(reg, smmu->base + SMMU_IRPT_CLR_NS);
	if (irq_status&TCU_EVENT_Q_IRQ) {
		arm_smmu_evtq_handler(irq, smmu);
	}
	if (irq_status&TCU_CMD_SYNC_IRQ) {
		arm_smmu_cmdq_sync_handler(irq, smmu);
	}
	if (irq_status&TCU_GERROR_IRQ) {
		arm_smmu_gerror_handler(irq, smmu);
	}
	return IRQ_HANDLED;
}

static void hisi_evt_flag_set(struct arm_smmu_device *smmu)
{
	if (smmu)
		atomic_set(&smmu->event_flag, 1);
}

static void hisi_evt_flag_unset(struct arm_smmu_device *smmu)
{
	if (smmu)
		atomic_set(&smmu->event_flag, 0);
}

static void hisi_structure_init_flag_set(struct arm_smmu_device *smmu)
{
	if (smmu)
		atomic_set(&smmu->structure_init_flag, 1);
}

static void hisi_structure_init_flag_unset(struct arm_smmu_device *smmu)
{
	if (smmu)
		atomic_set(&smmu->structure_init_flag, 0);
}

static int hisi_evt_irq_thread(void *data)
{
	u32 i;
	int ret = 0;
	struct arm_smmu_device *smmu = (struct arm_smmu_device *)data;
	struct arm_smmu_queue *q = &smmu->evtq.q;
	u64 evt[EVTQ_ENT_DWORDS];
	while (!kthread_should_stop()) {
		ret = wait_event_interruptible(smmu->wait_event_wq,
				atomic_read(&smmu->event_flag));
		if (ret) {
			hisi_svm_print(svm_error,
					"into %s, wait event intr err\n",
					__func__);
			continue;
		}
		mutex_lock(&hisi_svmtlb_mutex);

		if (smmu->status != smmu_enable) {
			pr_err("%s,smmu is not enabled,id:%d\n", __func__, smmu->smmuid);
			hisi_evt_flag_unset(smmu);
			mutex_unlock(&hisi_svmtlb_mutex);
			continue;
		}

		while (!queue_remove_raw(q, evt)) {
			u8 id = evt[0] >> EVTQ_0_ID_SHIFT & EVTQ_0_ID_MASK;
			dev_err(smmu->dev, "event 0x%02x received:\n", id);
			for (i = 0; i < ARRAY_SIZE(evt); ++i)
				dev_err(smmu->dev, "\t0x%016llx\n",
					(unsigned long long)evt[i]);
			if (EVTQ_TYPE_WITH_ADDR(id))
				hisi_svm_show_pte(hisi_svm_bind, evt[2], SZ_4K);
		}
		q->cons = Q_OVF(q, q->prod) | Q_WRP(q, q->cons) |
			Q_IDX(q, q->cons);
		hisi_evt_flag_unset(smmu);

		mutex_unlock(&hisi_svmtlb_mutex);
	}
	return 0;
}
static int hisi_evt_irq_setup(int irq, struct arm_smmu_device *smmu)
{
	long err = 0;
	struct task_struct *hisi_evt_task;
	hisi_evt_task = kthread_run(hisi_evt_irq_thread, smmu, "hisi_smmu_evt.%d", smmu->smmuid);
	if (IS_ERR(hisi_evt_task)) {
		err = PTR_ERR(hisi_evt_task);
		return err;
	}
	return 0;
}
static int hisi_smmu_setup_irqs(struct arm_smmu_device *smmu)
{
	int ret, irq;

	irq = smmu->smmu_irq;
	if (irq) {
		ret = devm_request_irq(smmu->dev, irq,
			arm_smmu_global_handler, 0,
			"arm-smmu-v3-global", smmu);
		if (ret) {
			dev_warn(smmu->dev,
				"failed to enable global irq\n");
			return ret;
		}
		ret = hisi_evt_irq_setup(irq, smmu);
		if (ret) {
			dev_warn(smmu->dev,
				"hisi_evt_irq_setup failed!\n");
			return ret;
		}
	}
	return 0;
}

static int arm_smmu_device_disable(struct arm_smmu_device *smmu)
{
	int ret;

	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret)
		dev_err(smmu->dev, "failed to clear cr0\n");

	return ret;
}

static int hisi_smmu_master_write(
	struct arm_smmu_device *smmu, u32 value, u32 offset)
{
	writel_relaxed(value, smmu->base + HISI_MASTER_0_BASE + offset);
	writel_relaxed(value, smmu->base + HISI_MASTER_1_BASE + offset);
	return 0;
}

static int hisi_smmu_master_init(struct arm_smmu_device *smmu)
{
	hisi_smmu_master_write(smmu, 0, SMMU_MSTR_GLB_BYPASS);
	hisi_smmu_master_write(smmu, HISI_VAL_MASK, SMMU_MSTR_SMRX_START_0);
	hisi_smmu_master_write(smmu, HISI_VAL_MASK, SMMU_MSTR_SMRX_START_1);
	hisi_smmu_master_write(smmu, HISI_VAL_MASK, SMMU_MSTR_SMRX_START_2);
	return 0;
}

static int hisi_smmu_intr_init(struct arm_smmu_device *smmu)
{
	u32 reg = (WDATA_BURST_CLR | WR_VA_ERR1_CLR
		| WR_VA_ERR0_CLR | RD_VA_ERR1_CLR | RD_VA_ERR0_CLR);
	hisi_smmu_master_write(smmu, reg, SMMU_MSTR_INTCLR);
	hisi_smmu_master_write(smmu, 0, SMMU_MSTR_INTMASK);
	writel_relaxed(IRQ_CTRL_EVTQ_IRQEN|IRQ_CTRL_GERROR_IRQEN,
		smmu->base + ARM_SMMU_IRQ_CTRL);
	writel_relaxed(HISI_VAL_MASK, smmu->base + SMMU_IRPT_CLR_NS);
	writel_relaxed(TCU_EVENT_TO_MASK, smmu->base + SMMU_IRPT_MASK_NS);
	return 0;
}

static void hisi_svm_dt_get_bypass_sid(struct arm_smmu_device *smmu)
{
	int ret = 0;
	u32 sid_bypass_wr_ai = 0;
	u32 sid_bypass_rd_ai = 0;
	u32 sid_bypass_wr_sdma = 0;
	u32 sid_bypass_rd_sdma = 0;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_bypass_wr_ai",
			&sid_bypass_wr_ai);
	if (ret) {
		smmu->sid_bypass_wr_ai = ARM_SMMU_MAX_SIDS;
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_bypass_wr_ai\n",
				__func__, smmu->smmuid);
	}
	else
		smmu->sid_bypass_wr_ai = sid_bypass_wr_ai;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_bypass_rd_ai",
			&sid_bypass_rd_ai);
	if (ret) {
		smmu->sid_bypass_rd_ai = ARM_SMMU_MAX_SIDS;
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_bypass_rd_ai\n",
				__func__, smmu->smmuid);
	}
	else
		smmu->sid_bypass_rd_ai = sid_bypass_rd_ai;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_bypass_wr_sdma",
			&sid_bypass_wr_sdma);
	if (ret) {
		smmu->sid_bypass_wr_sdma = ARM_SMMU_MAX_SIDS;
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_bypass_wr_sdma\n",
				__func__, smmu->smmuid);
	}
	else
		smmu->sid_bypass_wr_sdma = sid_bypass_wr_sdma;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_bypass_rd_sdma",
			&sid_bypass_rd_sdma);
	if (ret) {
		smmu->sid_bypass_rd_sdma = ARM_SMMU_MAX_SIDS;
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_bypass_rd_sdma\n",
				__func__, smmu->smmuid);
	}
	else
		smmu->sid_bypass_rd_sdma = sid_bypass_rd_sdma;

	hisi_svm_print(svm_trace, "%s:smmu%d bypass:wrai:%d,rdai:%d,wrsdma:%d,rdsdma:%d\n",
		__func__, smmu->smmuid, smmu->sid_bypass_wr_ai, smmu->sid_bypass_rd_ai,
		smmu->sid_bypass_wr_sdma, smmu->sid_bypass_rd_sdma);
}

static int hisi_svm_dt_get_mstr_end_val(struct arm_smmu_device *smmu)
{
	int ret = 0;
	u32 sid_mstr0_end0_val = 0;
	u32 sid_mstr0_end1_val = 0;
	u32 sid_mstr1_end0_val = 0;
	u32 sid_mstr1_end1_val = 0;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_mstr0_end0_val",
			&sid_mstr0_end0_val);
	if (ret) {
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_mstr0_end0_val\n",
				__func__, smmu->smmuid);
		return ret;
	}
	else
		smmu->sid_mstr0_end0_val = sid_mstr0_end0_val;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_mstr0_end1_val",
			&sid_mstr0_end1_val);
	if (ret) {
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_mstr0_end1_val\n",
				__func__, smmu->smmuid);
		return ret;
	}
	else
		smmu->sid_mstr0_end1_val = sid_mstr0_end1_val;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_mstr1_end0_val",
			&sid_mstr1_end0_val);
	if (ret) {
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_mstr1_end0_val\n",
				__func__, smmu->smmuid);
		return ret;
	}
	else
		smmu->sid_mstr1_end0_val = sid_mstr1_end0_val;

	ret = of_property_read_u32(smmu->dev->of_node, "sid_mstr1_end1_val",
			&sid_mstr1_end1_val);
	if (ret) {
		hisi_svm_print(svm_error, "%s ,smmu%d has no sid_mstr1_end1_val\n",
				__func__, smmu->smmuid);
		return ret;
	}
	else
		smmu->sid_mstr1_end1_val = sid_mstr1_end1_val;

	hisi_svm_print(svm_trace, "%s ,smmu%d m0e0:0x%x,m0e1:0x%x,m1e0:0x%x,m1e1:0x%x\n",
				__func__, smmu->smmuid, smmu->sid_mstr0_end0_val,
				smmu->sid_mstr0_end1_val,smmu->sid_mstr1_end0_val,
				smmu->sid_mstr1_end1_val);

	return 0;
}

static int arm_smmu_flush_cache(struct arm_smmu_device *smmu)
{
	struct arm_smmu_cmdq_ent cmd;

	/* Invalidate any cached configuration */
	cmd.opcode = CMDQ_OP_CFGI_ALL;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	cmd.opcode = CMDQ_OP_CMD_SYNC;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);

	/* Invalidate any stale TLB entries */
	if (smmu->features & ARM_SMMU_FEAT_HYP) {
		cmd.opcode = CMDQ_OP_TLBI_EL2_ALL;
		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	}

	cmd.opcode = CMDQ_OP_TLBI_NSNH_ALL;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	cmd.opcode = CMDQ_OP_CMD_SYNC;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);

	invalid_tcu_cache(smmu);

	return 0;
}

static int arm_smmu_device_reset(struct arm_smmu_device *smmu)
{
	int ret;
	u32 reg, enables;

	/* Clear CR0 and sync (disables SMMU and queue processing) */
	reg = readl_relaxed(smmu->base + ARM_SMMU_CR0);
	if (reg & CR0_SMMUEN)
		dev_warn(smmu->dev, "SMMU currently enabled! Resetting...\n");

	ret = arm_smmu_device_disable(smmu);
	if (ret)
		return ret;
	/* CR2 (random crap) */
	reg = CR2_PTM | CR2_RECINVSID;
	writel_relaxed(reg, smmu->base + ARM_SMMU_CR2);

	/* Stream table */
	writeq_relaxed(smmu->strtab_cfg.strtab_base,
		smmu->base + ARM_SMMU_STRTAB_BASE);
	writel_relaxed(smmu->strtab_cfg.strtab_base_cfg,
		smmu->base + ARM_SMMU_STRTAB_BASE_CFG);

	/* Command queue */
	writeq_relaxed(smmu->cmdq.q.q_base, smmu->base + ARM_SMMU_CMDQ_BASE);
	writel_relaxed(smmu->cmdq.q.prod, smmu->base + ARM_SMMU_CMDQ_PROD);
	writel_relaxed(smmu->cmdq.q.cons, smmu->base + ARM_SMMU_CMDQ_CONS);

	enables = CR0_CMDQEN;
	ret = arm_smmu_write_reg_sync(
		smmu, enables, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable command queue\n");
		return ret;
	}

	hisi_smmu_master_init(smmu);
	hisi_smmu_intr_init(smmu);
	/* Event queue */
	writeq_relaxed(smmu->evtq.q.q_base, smmu->base + ARM_SMMU_EVTQ_BASE);
	writel_relaxed(smmu->evtq.q.prod, smmu->base + ARM_SMMU_EVTQ_PROD);
	writel_relaxed(smmu->evtq.q.cons, smmu->base + ARM_SMMU_EVTQ_CONS);

	enables |= CR0_EVTQEN;
	ret = arm_smmu_write_reg_sync(
		smmu, enables, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable event queue\n");
		return ret;
	}

	/* Enable the SMMU interface */
	enables |= CR0_SMMUEN;
	ret = arm_smmu_write_reg_sync(
		smmu, enables, ARM_SMMU_CR0, ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable SMMU interface\n");
		return ret;
	}
	return 0;
}

static void arm_smmu_structures_dump(struct arm_smmu_device *smmu)
{
	dev_err(smmu->dev,
		"+++++++++++++++ dump smmu body start+++++++++++++++\n");

	dev_err(smmu->dev, "smmu ias      : 0x%08lx\n", smmu->ias);
	dev_err(smmu->dev, "smmu oas      : 0x%08lx\n", smmu->oas);
	dev_err(smmu->dev, "smmu feature  : 0x%08x\n", smmu->features);
	dev_err(smmu->dev, "smmu options  : 0x%08x\n", smmu->options);
	dev_err(smmu->dev, "smmu asid_bits: 0x%08x\n", smmu->asid_bits);
	dev_err(smmu->dev, "smmu vmid_bits: 0x%08x\n", smmu->vmid_bits);
	dev_err(smmu->dev, "smmu ssid_bits: 0x%08x\n", smmu->ssid_bits);
	dev_err(smmu->dev, "smmu sid_bits : 0x%08x\n", smmu->sid_bits);
	dev_err(smmu->dev, "smmu status   : 0x%08x\n", smmu->status);
	dev_err(smmu->dev, "smmu err irq  : 0x%08x\n", smmu->gerr_irq);

	dev_err(smmu->dev, "smmu cmdq irq        : 0x%08x\n", smmu->cmdq.q.irq);
	dev_err(smmu->dev, "smmu cmdq base_dma   : 0x%016llx\n",
		smmu->cmdq.q.base_dma);
	dev_err(smmu->dev, "smmu cmdq base       : %pK\n", smmu->cmdq.q.base);
	dev_err(smmu->dev, "smmu cmdq q_base     : 0x%016llx\n",
		smmu->cmdq.q.q_base);
	dev_err(smmu->dev, "smmu cmdq max_n_shift: 0x%x\n",
		smmu->cmdq.q.max_n_shift);
	dev_err(smmu->dev, "smmu cmdq cons       : 0x%x\n", smmu->cmdq.q.cons);
	dev_err(smmu->dev, "smmu cmdq prod       : 0x%x\n", smmu->cmdq.q.prod);
	dev_err(smmu->dev, "smmu cmdq *prod_reg  : 0x%08x\n",
		*smmu->cmdq.q.prod_reg);
	dev_err(smmu->dev, "smmu cmdq *cons_reg  : 0x%08x\n",
		*smmu->cmdq.q.cons_reg);
	dev_err(smmu->dev, "smmu cmdq ent_dwords : 0x%016lx\n",
		smmu->cmdq.q.ent_dwords);

	dev_err(smmu->dev, "smmu eventq irq        : 0x%08x\n",
		smmu->evtq.q.irq);
	dev_err(smmu->dev, "smmu eventq base_dma   : 0x%016llx\n",
		smmu->evtq.q.base_dma);
	dev_err(smmu->dev, "smmu eventq base       : %pK\n", smmu->evtq.q.base);
	dev_err(smmu->dev, "smmu eventq q_base     : 0x%016llx\n",
		smmu->evtq.q.q_base);
	dev_err(smmu->dev, "smmu eventq max_n_shift: 0x%x\n",
		smmu->evtq.q.max_n_shift);
	dev_err(smmu->dev, "smmu eventq cons       : 0x%x\n",
		smmu->evtq.q.cons);
	dev_err(smmu->dev, "smmu eventq prod       : 0x%x\n",
		smmu->evtq.q.prod);
	dev_err(smmu->dev, "smmu eventq *prod_reg  : 0x%08x\n",
		*smmu->evtq.q.prod_reg);
	dev_err(smmu->dev, "smmu eventq *cons_reg  : 0x%08x\n",
		*smmu->evtq.q.cons_reg);
	dev_err(smmu->dev, "smmu eventq ent_dwords : 0x%016lx\n",
		smmu->evtq.q.ent_dwords);

	dev_err(smmu->dev, "smmu str config dma : 0x%016llx\n",
		smmu->strtab_cfg.strtab_dma);
	dev_err(smmu->dev, "smmu str config vir : %pK\n",
		smmu->strtab_cfg.strtab);
	dev_err(smmu->dev, "smmu str config sid : 0x%08x\n",
		smmu->strtab_cfg.num_l1_ents);
	dev_err(smmu->dev, "smmu str config strbase : 0x%016llx\n",
		smmu->strtab_cfg.strtab_base);
	dev_err(smmu->dev, "smmu str config strbase_cfg: 0x%08x\n",
		smmu->strtab_cfg.strtab_base_cfg);

	dev_err(smmu->dev,
		"+++++++++++++++ dump smmu body end+++++++++++++++\n");
}

static int arm_smmu_device_probe(struct arm_smmu_device *smmu)
{
	u32 reg = 0;
	bool coherent;
	unsigned long pgsize_bitmap = 0;

	/* IDR0 */
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR0);
	/* 2-level L1 structures */
	if (IS_ENABLED(CONFIG_CAN_ST_2_LVL_STRTAB) &&
		((reg & IDR0_ST_LVL_MASK << IDR0_ST_LVL_SHIFT) ==
			IDR0_ST_LVL_2LVL))
		smmu->features |= ARM_SMMU_FEAT_2_LVL_STRTAB;

	/* 2-level CD structures */
	if (IS_ENABLED(CONFIG_CAN_CD_2_LVL_CDTAB) && (reg & IDR0_CD2L))
		smmu->features |= ARM_SMMU_FEAT_2_LVL_CDTAB;

	/*
	 * Translation table endianness.
	 * We currently require the same endianness as the CPU, but this
	 * could be changed later by adding a new IO_PGTABLE_QUIRK.
	 */
	switch (reg & IDR0_TTENDIAN_MASK << IDR0_TTENDIAN_SHIFT) {
	case IDR0_TTENDIAN_MIXED:
		smmu->features |= ARM_SMMU_FEAT_TT_LE | ARM_SMMU_FEAT_TT_BE;
		break;
#ifdef __BIG_ENDIAN
	case IDR0_TTENDIAN_BE:
		smmu->features |= ARM_SMMU_FEAT_TT_BE;
		break;
#else
	case IDR0_TTENDIAN_LE:
		smmu->features |= ARM_SMMU_FEAT_TT_LE;
		break;
#endif
	default:
		dev_err(smmu->dev, "unknown/unsupported TT endianness!\n");
		return -ENXIO;
	}

	if (reg & IDR0_SEV)
		smmu->features |= ARM_SMMU_FEAT_SEV;

	if (reg & IDR0_MSI)
		smmu->features |= ARM_SMMU_FEAT_MSI;

	if (reg & IDR0_HYP)
		smmu->features |= ARM_SMMU_FEAT_HYP;

	/*
	 * The dma-coherent property is used in preference to the ID
	 * register, but warn on mismatch.
	 */
	coherent = of_dma_is_coherent(smmu->dev->of_node);
	if (coherent)
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	if (!!(reg & IDR0_COHACC) != coherent)
		dev_warn(smmu->dev, "IDR0.COHACC overridden by dma-coherent "
				    "property (%s)\n",
			coherent ? "true" : "false");

	if (reg & IDR0_STALL_MODEL)
		smmu->features |= ARM_SMMU_FEAT_STALLS;

	if (reg & IDR0_S1P)
		smmu->features |= ARM_SMMU_FEAT_TRANS_S1;

	if (reg & IDR0_S2P)
		smmu->features |= ARM_SMMU_FEAT_TRANS_S2;

	if (!(reg & (IDR0_S1P | IDR0_S2P))) {
		dev_err(smmu->dev, "no translation support!\n");
		return -ENXIO;
	}

	/* We only support the AArch64 table format at present */
	switch (reg & IDR0_TTF_MASK << IDR0_TTF_SHIFT) {
	case IDR0_TTF_AARCH32_64:
		smmu->ias = HISI_SMMU_ADDR_SIZE_40;
	/* Fallthrough */
	case IDR0_TTF_AARCH64:
		break;
	default:
		dev_err(smmu->dev, "AArch64 table format not supported!\n");
		return -ENXIO;
	}

	/* ASID/VMID sizes */
	smmu->asid_bits = reg & IDR0_ASID16 ? HISI_SMMU_ID_SIZE_16 : HISI_SMMU_ID_SIZE_8;
	smmu->vmid_bits = reg & IDR0_VMID16 ? HISI_SMMU_ID_SIZE_16 : HISI_SMMU_ID_SIZE_8;

	/* IDR1 */
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR1);
	if (reg & (IDR1_TABLES_PRESET | IDR1_QUEUES_PRESET | IDR1_REL)) {
		dev_err(smmu->dev, "embedded implementation not supported\n");
		return -ENXIO;
	}
	/* Queue sizes, capped at 4k */
	smmu->cmdq.q.max_n_shift = min((u32)CMDQ_MAX_SZ_SHIFT,
		reg >> IDR1_CMDQ_SHIFT & IDR1_CMDQ_MASK);
	if (!smmu->cmdq.q.max_n_shift) {
		/* Odd alignment restrictions on the base, so ignore for now */
		dev_err(smmu->dev, "unit-length command queue not supported\n");
		return -ENXIO;
	}

	smmu->evtq.q.max_n_shift = min((u32)EVTQ_MAX_SZ_SHIFT,
		reg >> IDR1_EVTQ_SHIFT & IDR1_EVTQ_MASK);
	smmu->priq.q.max_n_shift = min((u32)PRIQ_MAX_SZ_SHIFT,
		reg >> IDR1_PRIQ_SHIFT & IDR1_PRIQ_MASK);

	/* SID/SSID sizes */
	smmu->ssid_bits = reg >> IDR1_SSID_SHIFT & IDR1_SSID_MASK;
	/*we only need 64 cd entries now*/
	if (smmu->sid_bits > HISI_SSID_MAX_BITS)
		smmu->sid_bits = HISI_SSID_MAX_BITS;

	smmu->sid_bits = reg >> IDR1_SID_SHIFT & IDR1_SID_MASK;
	/*we only need 64 ste entries now*/
	if (smmu->sid_bits > HISI_SID_MAX_BITS)
		smmu->sid_bits = HISI_SID_MAX_BITS;
	/* IDR5 */
	reg = readl_relaxed(smmu->base + ARM_SMMU_IDR5);
	/* Maximum number of outstanding stalls */
	smmu->evtq.max_stalls =
		reg >> IDR5_STALL_MAX_SHIFT & IDR5_STALL_MAX_MASK;

	/* Page sizes */
	if (reg & IDR5_GRAN64K)
		pgsize_bitmap |= SZ_64K | SZ_512M;
	if (reg & IDR5_GRAN16K)
		pgsize_bitmap |= SZ_16K | SZ_32M;
	if (reg & IDR5_GRAN4K)
		pgsize_bitmap |= SZ_4K | SZ_2M | SZ_1G;

	arm_smmu_ops.pgsize_bitmap &= pgsize_bitmap;

	/* Output address size */
	switch (reg & IDR5_OAS_MASK << IDR5_OAS_SHIFT) {
	case IDR5_OAS_32_BIT:
		smmu->oas = HISI_SMMU_ADDR_SIZE_32;
		break;
	case IDR5_OAS_36_BIT:
		smmu->oas = HISI_SMMU_ADDR_SIZE_36;
		break;
	case IDR5_OAS_40_BIT:
		smmu->oas = HISI_SMMU_ADDR_SIZE_40;
		break;
	case IDR5_OAS_42_BIT:
		smmu->oas = HISI_SMMU_ADDR_SIZE_42;
		break;
	case IDR5_OAS_44_BIT:
		smmu->oas = HISI_SMMU_ADDR_SIZE_44;
		break;
	default:
		dev_info(smmu->dev,
			"unknown output address size. Truncating to 48-bit\n");
	/* Fallthrough */
	case IDR5_OAS_48_BIT:
		smmu->oas = HISI_SMMU_ADDR_SIZE_48;
	}

	/* Set the DMA mask for our table walker */
	if (dma_set_mask_and_coherent(smmu->dev, DMA_BIT_MASK(smmu->oas)))
		dev_warn(
			smmu->dev, "failed to set DMA mask for table walker\n");

	smmu->ias = max(smmu->ias, smmu->oas);

	hisi_svm_print(svm_trace,
		"out %s ias %lu-bit, oas %lu-bit (features 0x%08x)\n", __func__,
		smmu->ias, smmu->oas, smmu->features);

	return 0;
}

static int arm_smmu_hw_set(struct arm_smmu_device *smmu)
{
	int ret = 0;

	if (!smmu) {
		hisi_svm_print(svm_error, "%s:invalid params!\n", __func__);
		return -ENOMEM;
	}
	ret = arm_smmu_device_probe(smmu);
	if (ret) {
		hisi_svm_print(svm_error,
			"%s:arm_smmu_device_probe failed (%d)\n", __func__,
			ret);
		return ret;
	}

	if (!atomic_read(&smmu->structure_init_flag)) {
		ret = arm_smmu_init_structures(smmu);
		if (ret) {
			hisi_svm_print(svm_error,
				"%s:arm_smmu_init_structures failed (%d)\n", __func__,
				ret);
			return ret;
		}
		hisi_structure_init_flag_set(smmu);
	}

	ret = arm_smmu_device_reset(smmu);
	if (ret) {
		hisi_svm_print(svm_error,
			"%s:arm_smmu_device_reset failed (%d)\n", __func__,
			ret);
		return ret;
	}
	arm_smmu_flush_cache(smmu);
	hisi_smmu_group_add_device(hisi_smmu_group, smmu);
	return ret;
}

/*lint -e429*/
static int arm_smmu_device_dt_probe(struct platform_device *pdev)
{
	int irq, ret;
	struct resource *res;
	struct arm_smmu_device *smmu;
	struct device *dev = &pdev->dev;
	u32 smmuid = 0;

	hisi_svm_print(svm_trace, "into %s\n", __func__);

	smmu = devm_kzalloc(dev, sizeof(*smmu), GFP_KERNEL);
	if (!smmu) {
		dev_err(dev, "%s:failed to alloc mem for smmu\n", __func__);
		return -ENOMEM;
	}
	smmu->dev = dev;
	smmu->dev_type = ARM_SMMU_DEVICES;

	/* Base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res || (res && (resource_size(res) + 1 < SZ_128K))) {
		dev_err(dev, "MMIO region too small (%pK)\n", res);
		return -EINVAL;
	}
	smmu->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(smmu->base)) {
		dev_err(dev, "%s:devm_ioremap_resource failed\n", __func__);
		return PTR_ERR(smmu->base);
	}
	/* Interrupt lines */
	irq = platform_get_irq_byname(pdev, "smmu");
	if (irq > 0)
		smmu->smmu_irq = irq;

	parse_driver_options(smmu);

	ret = of_property_read_u32(dev->of_node, "smmu-id", &smmuid);
	if (ret) {
		dev_err(smmu->dev, "%s:get smmuid failed\n", __func__);
		return ret;
	}
	smmu->smmuid = smmuid;
	hisi_svm_print(svm_trace, "%s:irq:%d,smmuid:%d\n",
		__func__, irq, smmuid);

	hisi_svm_dt_get_bypass_sid(smmu);

	ret = hisi_svm_dt_get_mstr_end_val(smmu);
	if (ret) {
		dev_err(smmu->dev, "hisi_svm_dt_get_mstr_end_val failed\n");
		return ret;
	}

	init_waitqueue_head(&(smmu->wait_event_wq));
	/* Record our private device structure */
	platform_set_drvdata(pdev, smmu);
	list_add(&smmu->smmu_node, &hisi_smmu_group->smmu_list);
	ret = hisi_smmu_setup_irqs(smmu);
	if (ret) {
		dev_err(smmu->dev, "hisi_smmu_setup_irqs failed\n");
		return ret;
	}
	hisi_svm_print(svm_trace, "out %s\n", __func__);
	return 0;
}
/*lint +e429*/

static int arm_smmu_device_remove(struct platform_device *pdev)
{
	struct arm_smmu_device *smmu = platform_get_drvdata(pdev);

	hisi_svm_print(svm_trace, "into %s\n", __func__);

	arm_smmu_device_disable(smmu);
	arm_smmu_free_structures(smmu);

	if (hisi_smmu_group)
		hisi_smmu_group_del_device(hisi_smmu_group, smmu);

	hisi_svm_print(svm_trace, "out %s\n", __func__);

	return 0;
}

static const struct of_device_id arm_smmu_of_match[] = {
	{
		.compatible = "arm,smmu-v3-ai",
	},
	{
		.compatible = "arm,smmu-v3-sdma",
	},
	{},
};
static struct platform_driver arm_smmu_driver = {
	.driver = {
			.name = "arm-smmu-v3",
			.of_match_table = of_match_ptr(arm_smmu_of_match),
	},
	.probe = arm_smmu_device_dt_probe,
	.remove = arm_smmu_device_remove,
};

/*
 * Because Hisilicon SVM with AI CPU
 * 1.realise the page fault function
 * 2.realise debug function of AI CPU
 */
static irqreturn_t hisi_mmu_gerror_handler(int irq, void *dev)
{
	u32 asid = 0;
	u64 vaddr = 0;

	if (pgfault_asid_addr_g && hisi_mmu_dev->asid_mem_base) {
		asid = *((u32 *)hisi_mmu_dev->asid_mem_base);
		hisi_svm_print(svm_info, "%s: page fault,asid:0x%x\n",
			__func__, asid);
	}

	if (pgfault_va_addr_g && hisi_mmu_dev->va_mem_base) {
		vaddr = *((u64 *)hisi_mmu_dev->va_mem_base);
		hisi_svm_print(svm_info, "%s: page fault,addr:0x%llx\n",
			__func__, vaddr);
		hisi_svm_show_pte(hisi_svm_bind, vaddr, SZ_4K);
	}

	return IRQ_HANDLED;
}

int hisi_aicpu_irq_offset_register(struct hisi_aicpu_irq_info info)
{
	pgfault_asid_addr_g = info.pgfault_asid_addr;
	pgfault_va_addr_g = info.pgfault_va_addr;
	hisi_svm_print(svm_info, "into %s,asid_addr:0x%llx,va_addr:0x%llx\n",
		__func__, pgfault_asid_addr_g, pgfault_va_addr_g);
	return 0;
}

static int hisi_aicpu_intr_addr_remap(void)
{
	void __iomem  *asid_mem_base;
	void __iomem  *va_mem_base;

	hisi_svm_print(svm_info, "into %s!\n", __func__);
	if (pgfault_asid_addr_g) {
		asid_mem_base = (void __iomem *)ioremap_wc(pgfault_asid_addr_g,
			sizeof(u32));
		if (!asid_mem_base) {
			hisi_svm_print(svm_error,
				"%s,failed to asid ioremap_wc!\n", __func__);
			return -EINVAL;
		}
		if (hisi_mmu_dev) {
			hisi_mmu_dev->asid_mem_base = asid_mem_base;
		}
	}

	if (pgfault_va_addr_g) {
		va_mem_base = (void __iomem *)ioremap_wc(pgfault_va_addr_g,
			sizeof(u64));
		if (!va_mem_base) {
			hisi_svm_print(svm_error,
				"%s,failed to va ioremap_wc!\n", __func__);
			return -EINVAL;
		}
		if (hisi_mmu_dev) {
			hisi_mmu_dev->va_mem_base = va_mem_base;
		}
	}
	hisi_svm_print(svm_info, "out %s!\n", __func__);
	return 0;
}

/*lint -e429*/
static int hisi_mmu_device_dt_probe(struct platform_device *pdev)
{
	int irq, ret;
	struct arm_smmu_device *hisi_mmu;
	struct device *dev = &pdev->dev;

	hisi_svm_print(svm_info, "into %s\n", __func__);

	hisi_mmu = devm_kzalloc(dev, sizeof(*hisi_mmu), GFP_KERNEL);
	if (!hisi_mmu) {
		dev_err(dev, "failed to allocate arm_smmu_device\n");
		return -ENOMEM;
	}
	hisi_mmu->dev = dev;
	hisi_mmu->dev_type = ARM_SMMU_DEVICES;

	/* Base fake register address
	 * Just for with AI CPU communication
	 */

	/* Interrupt lines */
	irq = platform_get_irq_byname(pdev, "gerror");
	if (irq > 0) {
		hisi_mmu->gerr_irq = irq;
		dev_err(hisi_mmu->dev, "hisi_mmu->gerr_irq %d\n",
				hisi_mmu->gerr_irq);
		ret = devm_request_irq(hisi_mmu->dev, irq,
			hisi_mmu_gerror_handler, 0,
			"hisi-mmu-v8-gerror", hisi_mmu);
		if (ret) {
			dev_err(hisi_mmu->dev,
					"failed to enable gerror irq\n");
			return -EINVAL;
		}
	}
	platform_set_drvdata(pdev, hisi_mmu);

	hisi_mmu->status = smmu_enable;
	hisi_mmu_dev = hisi_mmu;
	hisi_aicpu_intr_addr_remap();
	hisi_svm_print(svm_info, "out %s\n", __func__);
	return 0;
}
/*lint +e429*/

static int hisi_mmu_device_remove(struct platform_device *pdev)
{
	hisi_mmu_dev = NULL;
	hisi_svm_print(svm_info, "out %s\n", __func__);

	return 0;
}

static const struct of_device_id hisi_mmu_of_match[] = {
	{
		.compatible = "hisi,mmu-v8",
	},
	{},
};

static struct platform_driver hisi_mmu_driver = {
	.driver = {
			.name = "hisi-mmu-v8",
			.of_match_table = of_match_ptr(hisi_mmu_of_match),
	},
	.probe = hisi_mmu_device_dt_probe,
	.remove = hisi_mmu_device_remove,
};

void hisi_smmu_group_add_device(
	struct hisi_smmu_group *grp, struct arm_smmu_device *smmu)
{
	u32 sid;
	__le64 *step;

	if (!grp) {
		dev_err(smmu->dev, "gp is null smmu add to group is failed\n");
		return;
	}

	for (sid = 0; sid < smmu->strtab_cfg.num_l1_ents; sid++) {
		struct arm_smmu_strtab_ent ste = {
			.valid = true,
			.bypass = false,
			.cdtab_cfg = &grp->cdtab_cfg,
		};
		/*
		 * get bypass streamid from dtsi,
		 * when CONFIG_HISI_SVM_STE_BYPASS is open,
		 * smmu bypass use bypass pagetbl scheme.
		 */
		if ((sid == smmu->sid_bypass_wr_ai) ||
		    (sid == smmu->sid_bypass_rd_ai) ||
		    (sid == smmu->sid_bypass_wr_sdma) ||
		    (sid == smmu->sid_bypass_rd_sdma)) {
			if (hisi_smmu_group->bypass_status == bypass_enable)
				ste.cdtab_cfg = &grp->cdtab_cfg_bypass;
			else {
				hisi_smmu_master_write(smmu,
					(SSID_V_MASK_EN | MSTR_BYPASS), SMMU_MSTR_SMRX_0(sid));
				ste.bypass = true;
			}
			hisi_svm_print(svm_trace,
						"%s:set sid %d bypass \n", __func__, sid);
		}
		/* This func is diff with smmu supporting 2lvl strtab, but we
		 * only consider linear strtab now
		 */
		step = arm_smmu_get_step_for_sid(smmu, sid);

		arm_smmu_write_strtab_ent(smmu, sid, step, &ste);
	}
}

void hisi_smmu_group_del_device(
	struct hisi_smmu_group *gp, struct arm_smmu_device *smmu)
{
	hisi_svm_print(svm_info, "into %s\n", __func__);

	if (!gp || !smmu) {
		hisi_svm_print(svm_error, "%s,invalid param\n", __func__);
		return;
	}
	list_del(&smmu->smmu_node);

	hisi_svm_print(svm_info, "out %s\n", __func__);
}

void hisi_smmu_group_dump(void)
{
	struct list_head *p, *n;
	struct arm_smmu_device *smmu;
	struct hisi_smmu_group *grp = hisi_smmu_group;
	hisi_svm_print(svm_info, "======== %s start========\n",
		__func__);

	if (!grp)
		return;

	dev_err(grp->sgrp_dev, "ias       : %lu\n", grp->ias);
	dev_err(grp->sgrp_dev, "oas       : %lu\n", grp->oas);
	dev_err(grp->sgrp_dev, "status    : 0x%08x\n", grp->status);
	dev_err(grp->sgrp_dev, "asid bits : %d\n", grp->asid_bits);
	dev_err(grp->sgrp_dev, "ssid bits : %d\n", grp->ssid_bits);

	dev_err(grp->sgrp_dev, "cdtab     : %pK\n", grp->cdtab_cfg.cdtab);
	dev_err(grp->sgrp_dev, "cdtab_dma : 0x%016llx\n",
		grp->cdtab_cfg.cdtab_dma);
	dev_err(grp->sgrp_dev, "size      : 0x%016lx\n", grp->cdtab_cfg.sz);

	list_for_each_safe(p, n, &grp->smmu_list) {
		smmu = list_entry(p, struct arm_smmu_device, smmu_node);
		if (!smmu) {
			hisi_svm_print(
				svm_error, "%s smmu is null\n", __func__);
			return;
		}
		if (smmu->status == smmu_enable)
			arm_smmu_structures_dump(smmu);
	}

	hisi_svm_print(svm_info, "================= %s end===============\n",
		__func__);
}

int hisi_smmu_poweron(int smmuid)
{
	u32 reg = 0;
	int ret = 0;
	struct list_head *p, *n;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_device *tmp = NULL;

	hisi_svm_print(svm_info, "in %s,smmuid:%d\n", __func__, smmuid);
	if (smmuid != svm_ai && smmuid != svm_sdma) {
		hisi_svm_print(svm_error, "invalid params!%s\n", __func__);
		return -EINVAL;
	}

	list_for_each_safe(p, n, &hisi_smmu_group->smmu_list) {
		tmp = list_entry(p, struct arm_smmu_device, smmu_node);
		if (tmp && (tmp->smmuid == smmuid)
			&& (tmp->status != smmu_enable)) {
			smmu = tmp;
			break;
		}
	}

	if (!smmu) {
		hisi_svm_print(svm_error,
			"smmuid %d not found!%s\n",smmuid, __func__);
		return -EINVAL;
	}

	ret = hisi_smmu_reg_set(smmu, SMMU_LP_REQ, SMMU_LP_ACK,
			TCU_QREQN_CG, TCU_QACCEPTN_CG);
	if (ret) {
		hisi_svm_print(svm_error,
			"TCU_QACCEPTN_CG failed !%s\n", __func__);
		return -EINVAL;
	}

	ret = hisi_smmu_reg_set(smmu, SMMU_LP_REQ, SMMU_LP_ACK,
		TCU_QREQN_PD, TCU_QACCEPTN_PD);
	if (ret) {
		hisi_svm_print(svm_error,
			"TCU_QACCEPTN_PD failed !%s\n", __func__);
		return -EINVAL;
	}

	ret = hisi_smmu_reg_set(smmu, SMMU_TBU_CR, SMMU_TBU_CRACK,
		TBU_EN_REQ, TBU_EN_ACK);
	if (ret) {
		hisi_svm_print(svm_error,
				"TBU_EN_ACK failed !%s\n",
				__func__);
		return -EINVAL;
	}

	reg = readl_relaxed(smmu->base + SMMU_TBU_CRACK);
	if (!(reg & TBU_CONNECTED)) {
		hisi_svm_print(svm_error,
				"TBU_CONNECTED failed!%s\n",
				__func__);
		return -EINVAL;
	}

	ret = arm_smmu_hw_set(smmu);
	if (ret) {
		hisi_svm_print(svm_error,
			"arm_smmu_hw_set failed !%s\n", __func__);
		return -EINVAL;
	}

	smmu->status = smmu_enable;
	hisi_smmu_poweron_flag = 1;

	hisi_svm_print(svm_info, "out %s\n", __func__);
	return 0;
}

static int __smmu_master_end_check(const void *smmubase,
	unsigned long mstrbase, u32 end, u32 val)
{
	u32 reg = 0;
	u32 check_times = 0;

	do {
		reg = readl_relaxed(smmubase
		+ mstrbase + end);
		if (reg == val)
			break;
		udelay(1);
		if (++check_times >= MAX_CHECK_TIMES) {
			hisi_svm_print(svm_error,
				"%s:MSTR_END_CHECK failed,reg:0x%x\n", __func__, reg);
			return -EINVAL;
		}
	} while (1);

	return 0;
}

static int hisi_smmu_master_end_check(struct arm_smmu_device *smmu)
{
	if (__smmu_master_end_check(smmu->base,
		HISI_MASTER_0_BASE, MSTR_END_ACK(0), smmu->sid_mstr0_end0_val))
		return -EINVAL;

	if (__smmu_master_end_check(smmu->base,
		HISI_MASTER_0_BASE, MSTR_END_ACK(1), smmu->sid_mstr0_end1_val))
		return -EINVAL;

	if (__smmu_master_end_check(smmu->base,
		HISI_MASTER_1_BASE, MSTR_END_ACK(0), smmu->sid_mstr1_end0_val))
		return -EINVAL;

	if (__smmu_master_end_check(smmu->base,
		HISI_MASTER_1_BASE, MSTR_END_ACK(1), smmu->sid_mstr1_end1_val))
		return -EINVAL;

	return 0;
}

int hisi_smmu_poweroff(int smmuid)
{
	u32 reg = 0;
	u32 val = 0;
	int ret = 0;
	struct list_head *p, *n;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_device *tmp = NULL;

	hisi_svm_print(svm_info, "in %s,smmuid:%d\n", __func__, smmuid);
	if (smmuid != svm_ai && smmuid != svm_sdma) {
		hisi_svm_print(svm_error, "invalid params!%s\n", __func__);
		return -EINVAL;
	}

	list_for_each_safe(p, n, &hisi_smmu_group->smmu_list) {
		tmp = list_entry(p, struct arm_smmu_device, smmu_node);
		if (tmp && (tmp->smmuid == smmuid)
			&& (tmp->status != smmu_disable)) {
			smmu = tmp;
			break;
		}
	}

	if (!smmu) {
		hisi_svm_print(svm_error,
			"smmuid %d not found!%s\n",smmuid, __func__);
		return -EINVAL;
	}

	mutex_lock(&hisi_svmtlb_mutex);

	ret = hisi_smmu_master_end_check(smmu);
	if (ret) {
		hisi_svm_print(
			svm_error, "master_end_check failed !%s\n", __func__);
		goto free_source;
	}

	val = readl_relaxed(smmu->base + SMMU_TBU_CR);
	val &= ~TBU_EN_REQ;
	writel_relaxed(val, smmu->base + SMMU_TBU_CR);

	ret = readl_relaxed_poll_timeout(smmu->base + SMMU_TBU_CRACK,
		reg, reg & TBU_EN_ACK, 1, ARM_SMMU_POLL_TIMEOUT_US);
	if (ret) {
		hisi_svm_print(
			svm_error, "TBU_EN_ACK failed !%s\n", __func__);
		goto free_source;
	}

	reg = readl_relaxed(smmu->base + SMMU_TBU_CRACK);
	if (reg & TBU_CONNECTED) {
		hisi_svm_print(svm_error,
			"TBU is still connected !%s\n", __func__);
		ret = -EINVAL;
		goto free_source;
	}

	ret = hisi_smmu_reg_unset(smmu, SMMU_LP_REQ, SMMU_LP_ACK,
		TCU_QREQN_PD, TCU_QACCEPTN_PD);
	if (ret) {
		hisi_svm_print(svm_error,
			"TCU_QACCEPTN_PD failed !%s\n", __func__);
		goto free_source;
	}

	ret = hisi_smmu_reg_unset(smmu, SMMU_LP_REQ, SMMU_LP_ACK,
		TCU_QREQN_CG, TCU_QACCEPTN_CG);
	if (ret) {
		hisi_svm_print(svm_error,
			"TCU_QACCEPTN_CG failed !%s\n", __func__);
		goto free_source;
	}

free_source:
	arm_smmu_zeroed_structures(smmu);
	smmu->status = smmu_disable;
	hisi_smmu_poweron_flag = 0;

	mutex_unlock(&hisi_svmtlb_mutex);
	hisi_svm_print(svm_info, "out %s, smmuid:%d, ret:%d\n",
		__func__, smmuid, ret);

	return ret;
}

static void hisi_smmu_mutex_init(void)
{
	mutex_init(&hisi_svm_mutex);
	mutex_init(&hisi_svmtlb_mutex);
}

/*lint -e429*/
static int hisi_smmu_group_dt_probe(struct platform_device *pdev)
{
	int ret = 0;
	u64 size;
	struct hisi_smmu_group *grp = NULL;
	struct device *dev = &pdev->dev;

	hisi_svm_print(svm_info, "into %s,pdevname:%s\n", __func__, pdev->name);
	grp = devm_kzalloc(dev, sizeof(*grp), GFP_KERNEL);
	if (!grp) {
		dev_err(dev, "failed to allocate hisi_smmu_group\n");
		return -ENOMEM;
	}

	grp->sgrp_dev = dev;
	INIT_LIST_HEAD(&grp->smmu_list);

	grp->oas = HISI_SMMU_ADDR_SIZE_48;
	grp->ias = HISI_SMMU_ADDR_SIZE_48;

	grp->asid_bits = ilog2(IDR1_SID_MASK + 1);
	grp->ssid_bits = ilog2(IDR1_SSID_MASK + 1);

	/* Set the DMA mask for our table walker */
	if (dma_set_mask_and_coherent(grp->sgrp_dev,
				DMA_BIT_MASK(grp->oas)))
		dev_err(grp->sgrp_dev,
			"failed to set DMA mask for table walker\n");

	/*
	 * Allocation of the cd desc table.
	 * And all cd descs are invalid.
	 */
	size = (1UL << grp->ssid_bits) * (CTXDESC_CD_DWORDS << 3);
	grp->cdtab_cfg.cdtab = dma_zalloc_coherent(grp->sgrp_dev,
			(size_t)size, &grp->cdtab_cfg.cdtab_dma,
			GFP_KERNEL);
	if (!grp->cdtab_cfg.cdtab) {
		dev_err(grp->sgrp_dev,
			"failed to allocate cdtab (0x%llx bytes)\n",
			size);
		return -ENOMEM;
	}
	grp->cdtab_cfg.sz = size;
	/* The phy base always aligned 4K */
	WARN_ON(!IS_ALIGNED(grp->cdtab_cfg.cdtab_dma, PAGE_SIZE));

	/*
	 * Because cd descs are invalid so,
     * and ste is valid and bypass.
	 */
#ifdef CONFIG_HISI_SVM_STE_BYPASS
	grp->cdtab_cfg_bypass.cdtab = dma_zalloc_coherent(
		grp->sgrp_dev, size, &grp->cdtab_cfg_bypass.cdtab_dma, GFP_KERNEL);
	if (!grp->cdtab_cfg_bypass.cdtab) {
		dev_err(grp->sgrp_dev,
			"failed to allocate cdtab_cfg_bypass cdtab (0x%llx bytes)\n", size);
		goto err;
	}
	grp->cdtab_cfg_bypass.sz = size;
	WARN_ON(!IS_ALIGNED(grp->cdtab_cfg_bypass.cdtab_dma, PAGE_SIZE));
#endif
	mutex_init(&grp->sgrp_mtx);
	set_bit(0, grp->ssid_map);
	set_bit(0, grp->asid_map);

	hisi_svm_manager_init();

#ifdef CONFIG_HISI_DEBUG_FS
	ret = hisi_svm_init_debugfs();
	if (ret) {
		dev_err(grp->sgrp_dev, "%s,init debugfs fail!\n", __func__);
		goto bypass_err;
	}
#endif

	/* Record our private device structure */
	platform_set_drvdata(pdev, grp);
	bus_set_iommu(&ion_svm_bus, &arm_smmu_ops);
	grp->status = smmu_enable;
	hisi_smmu_mutex_init();
	hisi_smmu_group = grp;
	hisi_svm_print(svm_info, "out %s ok!\n", __func__);
	return 0;

#ifdef CONFIG_HISI_DEBUG_FS
bypass_err:
#ifdef CONFIG_HISI_SVM_STE_BYPASS
	dma_free_coherent(&pdev->dev, grp->cdtab_cfg_bypass.sz,
			grp->cdtab_cfg_bypass.cdtab, grp->cdtab_cfg_bypass.cdtab_dma);
#endif
#endif

err:
	dma_free_coherent(&pdev->dev, grp->cdtab_cfg.sz, grp->cdtab_cfg.cdtab,
		grp->cdtab_cfg.cdtab_dma);
	dev_err(dev, "err out %s\n", __func__);
	return ret;
}
/*lint +e429*/

/*
 * The function is called after smmu driver remove
 */
static int hisi_smmu_group_remove(struct platform_device *pdev)
{
	struct hisi_smmu_group *grp = NULL;
	hisi_svm_print(svm_info, "into %s\n", __func__);

	if (!pdev) {
		hisi_svm_print(svm_error, " %s pdev is null!!\n", __func__);
		return -EINVAL;
	}
	grp = platform_get_drvdata(pdev);
	if (!grp) {
		hisi_svm_print(svm_error, " %s grp is null!!\n", __func__);
		return -EINVAL;
	}

	/* Free the ctx desc table */
	if (grp->cdtab_cfg.cdtab)
		dma_free_coherent(&pdev->dev, grp->cdtab_cfg.sz,
			grp->cdtab_cfg.cdtab, grp->cdtab_cfg.cdtab_dma);
#ifdef CONFIG_HISI_SVM_STE_BYPASS
	if (grp->cdtab_cfg_bypass.cdtab)
		dma_free_coherent(&pdev->dev, grp->cdtab_cfg_bypass.sz,
			grp->cdtab_cfg_bypass.cdtab, grp->cdtab_cfg_bypass.cdtab_dma);
#endif
	hisi_smmu_group = NULL;

	hisi_svm_print(svm_info, "out %s\n", __func__);
	return 0;
}

static const struct of_device_id hisi_smmu_group_of_match[] = {
	{
		.compatible = "hisi,smmu-group",
	},
	{},
};

static struct platform_driver hisi_smmu_group_driver = {
	.driver = {
		.name = "hisi-smmu-group",
		.of_match_table = of_match_ptr(hisi_smmu_group_of_match),
	},
	.probe = hisi_smmu_group_dt_probe,
	.remove = hisi_smmu_group_remove,
};

#ifdef CONFIG_HISI_SVM_STE_BYPASS
static int hisi_smmu_bypass_dt_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	u32 bypass_addr[L2BYPASS_ADDR_NUM] = {0};

	hisi_svm_print(svm_info, "into %s,pdevname:%s\n", __func__, pdev->name);

	ret = of_property_read_u32_array(dev->of_node, "l2buff_addr", bypass_addr, L2BYPASS_ADDR_NUM);
	if (ret) {
		dev_err(dev, "get l2buff addr failed\n");
		return -ENOMEM;
	}
	hisi_svm_print(svm_info, "bypass addr:0x%x, 0x%x, 0x%x\n",
		bypass_addr[0], bypass_addr[1], bypass_addr[2]);
	if (hisi_smmu_group) {
		hisi_smmu_group->bypass_addr[0] = bypass_addr[0];
		hisi_smmu_group->bypass_addr[1] = bypass_addr[1];
		hisi_smmu_group->bypass_addr[2] = bypass_addr[2];
		hisi_smmu_group->bypass_status = bypass_enable;
	}

	hisi_svm_print(svm_info, "out %s\n", __func__);
	return ret;
}

static int hisi_smmu_bypass_remove(struct platform_device *pdev)
{
	if (hisi_smmu_group)
		hisi_smmu_group->bypass_status = bypass_disable;
	return 0;
}

static const struct of_device_id hisi_smmu_bypass_of_match[] = {
	{
		.compatible = "hisi,smmu-bypass",
	},
	{},
};

static struct platform_driver hisi_smmu_bypass_driver = {
	.driver = {
		.name = "hisi-smmu-bypass",
		.of_match_table = of_match_ptr(hisi_smmu_bypass_of_match),
	},
	.probe = hisi_smmu_bypass_dt_probe,
	.remove = hisi_smmu_bypass_remove,
};
#endif

static int __init hisi_smmu_group_init(void)
{
	int ret = 0;

	hisi_svm_print(svm_info, "into %s\n", __func__);

	ret = bus_register(&ion_svm_bus);
	if (ret) {
		hisi_svm_print(svm_error, "bus_register failed %s\n", __func__);
		goto out;
	}

	ret = platform_driver_register(&hisi_smmu_group_driver);
	if (ret) {
		hisi_svm_print(svm_error, "hisi_smmu_group_driver failed %s\n",
			__func__);
		goto bus_err;
	}

#ifdef CONFIG_HISI_SVM_STE_BYPASS
	ret = platform_driver_register(&hisi_smmu_bypass_driver);
	if (ret) {
		hisi_svm_print(svm_error, "hisi_smmu_bypass_driver failed %s\n",
			__func__);
		goto reg_err;
	}
#endif

	ret = platform_driver_register(&arm_smmu_driver);
	if (ret) {
		hisi_svm_print(
			svm_error, "arm_smmu_driver failed %s\n", __func__);
		goto bypass_err;
	}

	ret = platform_driver_register(&hisi_mmu_driver);
	if (ret) {
		hisi_svm_print(
			svm_error, "hisi_mmu_driver failed %s\n", __func__);
		goto arm_reg_err;
	}
	hisi_svm_print(svm_info, "out %s\n", __func__);
	return ret;

arm_reg_err:
	platform_driver_unregister(&arm_smmu_driver);

bypass_err:
#ifdef CONFIG_HISI_SVM_STE_BYPASS
	platform_driver_unregister(&hisi_smmu_bypass_driver);
#endif

reg_err:
	platform_driver_unregister(&hisi_smmu_group_driver);

bus_err:
	bus_unregister(&ion_svm_bus);

out:
	hisi_svm_print(svm_error, "error out %s ret %d\n", __func__, ret);

	return ret;
}

static void __exit hisi_smmu_group_exit(void)
{
	platform_driver_unregister(&hisi_mmu_driver);
	platform_driver_unregister(&arm_smmu_driver);
#ifdef CONFIG_HISI_SVM_STE_BYPASS
	platform_driver_unregister(&hisi_smmu_bypass_driver);
#endif
	platform_driver_unregister(&hisi_smmu_group_driver);
	bus_unregister(&ion_svm_bus);
}

late_initcall(hisi_smmu_group_init);
module_exit(hisi_smmu_group_exit);

MODULE_DESCRIPTION(
	"Hisilicon SVM Base on ARM architected SMMUv3 implementations");
MODULE_AUTHOR("YuDongbin <yudongbin@hisilicon.com>");
MODULE_LICENSE("GPL v2");
