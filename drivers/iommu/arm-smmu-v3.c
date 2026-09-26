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
#define pr_fmt(fmt) "smmuv3:" fmt

#include <linux/acpi.h>
#include <linux/acpi_iort.h>
#include <linux/delay.h>
#include <linux/dma-iommu.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/iommu.h>
#include <linux/iopoll.h>
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_iommu.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/amba/bus.h>
#include <linux/sched/mm.h>
#include <linux/hisi/hisi_iommu_dma.h>
#include <asm/pgtable.h>
#include <asm/cacheflush.h>
#include <mntn_subtype_exception.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/bitops.h>
#include <uapi/linux/sched/types.h>
#ifdef CONFIG_MM_TCU
#include <linux/hisi/hisi_svm.h>
#endif

#include "io-pgtable.h"
#include "hisi/hisi_smmu.h"
#include "soc_media1_crg_interface.h"
#include "soc_media2_crg_interface.h"
#include "linux/hisi/rdr_pub.h"

#if defined(CONFIG_HISI_DDR_SUBREASON) && defined(CONFIG_HISI_DDR_MONOCEROS)
#include "../hisi/ddrc/hisi_ddr_subreason/hisi_ddr_subreason_monoceros.h"
#endif

#define CONFIG_IOVA_POOL 1
#define HISI_SMMUV3_SUPPORT
/* MMIO registers */
#define ARM_SMMU_IDR0			0x0
#define IDR0_ST_LVL_SHIFT		27
#define IDR0_ST_LVL_MASK		0x3
#define IDR0_ST_LVL_2LVL		(1 << IDR0_ST_LVL_SHIFT)
#define IDR0_STALL_MODEL_SHIFT		24
#define IDR0_STALL_MODEL_MASK		0x3
#define IDR0_STALL_MODEL_STALL		(0 << IDR0_STALL_MODEL_SHIFT)
#define IDR0_STALL_MODEL_FORCE		(2 << IDR0_STALL_MODEL_SHIFT)
#define IDR0_TTENDIAN_SHIFT		21
#define IDR0_TTENDIAN_MASK		0x3
#define IDR0_TTENDIAN_LE		(2 << IDR0_TTENDIAN_SHIFT)
#define IDR0_TTENDIAN_BE		(3 << IDR0_TTENDIAN_SHIFT)
#define IDR0_TTENDIAN_MIXED		(0 << IDR0_TTENDIAN_SHIFT)
#define IDR0_CD2L			(1 << 19)
#define IDR0_VMID16			(1 << 18)
#define IDR0_PRI			(1 << 16)
#define IDR0_SEV			(1 << 14)
#define IDR0_MSI			(1 << 13)
#define IDR0_ASID16			(1 << 12)
#define IDR0_ATS			(1 << 10)
#define IDR0_HYP			(1 << 9)
#define IDR0_COHACC			(1 << 4)
#define IDR0_TTF_SHIFT			2
#define IDR0_TTF_MASK			0x3
#define IDR0_TTF_AARCH64		(2 << IDR0_TTF_SHIFT)
#define IDR0_TTF_AARCH32_64		(3 << IDR0_TTF_SHIFT)
#define IDR0_S1P			(1 << 1)
#define IDR0_S2P			(1 << 0)

#define ARM_SMMU_IDR1			0x4
#define IDR1_TABLES_PRESET		(1 << 30)
#define IDR1_QUEUES_PRESET		(1 << 29)
#define IDR1_REL			(1 << 28)
#define IDR1_CMDQ_SHIFT			21
#define IDR1_CMDQ_MASK			0x1f
#define IDR1_EVTQ_SHIFT			16
#define IDR1_EVTQ_MASK			0x1f
#define IDR1_PRIQ_SHIFT			11
#define IDR1_PRIQ_MASK			0x1f
#define IDR1_SSID_SHIFT			6
#define IDR1_SSID_MASK			0x1f
#define IDR1_SID_SHIFT			0
#define IDR1_SID_MASK			0x3f

#define ARM_SMMU_IDR5			0x14
#define IDR5_STALL_MAX_SHIFT		16
#define IDR5_STALL_MAX_MASK		0xffff
#define IDR5_GRAN64K			(1 << 6)
#define IDR5_GRAN16K			(1 << 5)
#define IDR5_GRAN4K			(1 << 4)
#define IDR5_OAS_SHIFT			0
#define IDR5_OAS_MASK			0x7
#define IDR5_OAS_32_BIT			(0 << IDR5_OAS_SHIFT)
#define IDR5_OAS_36_BIT			(1 << IDR5_OAS_SHIFT)
#define IDR5_OAS_40_BIT			(2 << IDR5_OAS_SHIFT)
#define IDR5_OAS_42_BIT			(3 << IDR5_OAS_SHIFT)
#define IDR5_OAS_44_BIT			(4 << IDR5_OAS_SHIFT)
#define IDR5_OAS_48_BIT			(5 << IDR5_OAS_SHIFT)

#define ARM_SMMU_CR0			0x20
#define CR0_CMDQEN			(1 << 3)
#define CR0_EVTQEN			(1 << 2)
#define CR0_PRIQEN			(1 << 1)
#define CR0_SMMUEN			(1 << 0)

#define ARM_SMMU_CR0ACK			0x24

#define ARM_SMMU_CR1			0x28
#define CR1_SH_NSH			0
#define CR1_SH_OSH			2
#define CR1_SH_ISH			3
#define CR1_CACHE_NC			0
#define CR1_CACHE_WB			1
#define CR1_CACHE_WT			2
#define CR1_TABLE_SH_SHIFT		10
#define CR1_TABLE_OC_SHIFT		8
#define CR1_TABLE_IC_SHIFT		6
#define CR1_QUEUE_SH_SHIFT		4
#define CR1_QUEUE_OC_SHIFT		2
#define CR1_QUEUE_IC_SHIFT		0

#define ARM_SMMU_CR2			0x2c
#define CR2_PTM				(1 << 2)
#define CR2_RECINVSID			(1 << 1)
#define CR2_E2H				(1 << 0)

#define ARM_SMMU_GBPA			0x44
#define GBPA_ABORT			(1 << 20)
#define GBPA_UPDATE			(1 << 31)

#define ARM_SMMU_IRQ_CTRL		0x50
#define IRQ_CTRL_EVTQ_IRQEN		(1 << 2)
#define IRQ_CTRL_PRIQ_IRQEN		(1 << 1)
#define IRQ_CTRL_GERROR_IRQEN		(1 << 0)

#define ARM_SMMU_IRQ_CTRLACK		0x54

#define ARM_SMMU_GERROR			0x60
#define GERROR_SFM_ERR			(1 << 8)
#define GERROR_MSI_GERROR_ABT_ERR	(1 << 7)
#define GERROR_MSI_PRIQ_ABT_ERR		(1 << 6)
#define GERROR_MSI_EVTQ_ABT_ERR		(1 << 5)
#define GERROR_MSI_CMDQ_ABT_ERR		(1 << 4)
#define GERROR_PRIQ_ABT_ERR		(1 << 3)
#define GERROR_EVTQ_ABT_ERR		(1 << 2)
#define GERROR_CMDQ_ERR			(1 << 0)
#define GERROR_ERR_MASK			0xfd

#define ARM_SMMU_GERRORN		0x64

#define ARM_SMMU_GERROR_IRQ_CFG0	0x68
#define ARM_SMMU_GERROR_IRQ_CFG1	0x70
#define ARM_SMMU_GERROR_IRQ_CFG2	0x74

#define ARM_SMMU_STRTAB_BASE		0x80
#define STRTAB_BASE_RA			(1UL << 62)
#define STRTAB_BASE_ADDR_SHIFT		6
#define STRTAB_BASE_ADDR_MASK		0x3ffffffffffUL

#define ARM_SMMU_STRTAB_BASE_CFG	0x88
#define STRTAB_BASE_CFG_LOG2SIZE_SHIFT	0
#define STRTAB_BASE_CFG_LOG2SIZE_MASK	0x3f
#define STRTAB_BASE_CFG_SPLIT_SHIFT	6
#define STRTAB_BASE_CFG_SPLIT_MASK	0x1f
#define STRTAB_BASE_CFG_FMT_SHIFT	16
#define STRTAB_BASE_CFG_FMT_MASK	0x3
#define STRTAB_BASE_CFG_FMT_LINEAR	(0 << STRTAB_BASE_CFG_FMT_SHIFT)
#define STRTAB_BASE_CFG_FMT_2LVL	(1 << STRTAB_BASE_CFG_FMT_SHIFT)

#define ARM_SMMU_CMDQ_BASE		0x90
#define ARM_SMMU_CMDQ_PROD		0x98
#define ARM_SMMU_CMDQ_CONS		0x9c

#define ARM_SMMU_EVTQ_BASE		0xa0
#define ARM_SMMU_EVTQ_PROD		0x100a8
#define ARM_SMMU_EVTQ_CONS		0x100ac
#define ARM_SMMU_EVTQ_IRQ_CFG0		0xb0
#define ARM_SMMU_EVTQ_IRQ_CFG1		0xb8
#define ARM_SMMU_EVTQ_IRQ_CFG2		0xbc

#define ARM_SMMU_PRIQ_BASE		0xc0
#define ARM_SMMU_PRIQ_PROD		0x100c8
#define ARM_SMMU_PRIQ_CONS		0x100cc
#define ARM_SMMU_PRIQ_IRQ_CFG0		0xd0
#define ARM_SMMU_PRIQ_IRQ_CFG1		0xd8
#define ARM_SMMU_PRIQ_IRQ_CFG2		0xdc

/* Common MSI config fields */
#define MSI_CFG0_ADDR_SHIFT		2
#define MSI_CFG0_ADDR_MASK		0x3fffffffffffUL
#define MSI_CFG2_SH_SHIFT		4
#define MSI_CFG2_SH_NSH			(0UL << MSI_CFG2_SH_SHIFT)
#define MSI_CFG2_SH_OSH			(2UL << MSI_CFG2_SH_SHIFT)
#define MSI_CFG2_SH_ISH			(3UL << MSI_CFG2_SH_SHIFT)
#define MSI_CFG2_MEMATTR_SHIFT		0
#define MSI_CFG2_MEMATTR_DEVICE_NG_NRE	(0x1 << MSI_CFG2_MEMATTR_SHIFT)

#define Q_IDX(q, p)			((p) & ((1 << (q)->max_n_shift) - 1))
#define Q_WRP(q, p)			((p) & (1 << (q)->max_n_shift))
#define Q_OVERFLOW_FLAG			(1 << 31)
#define Q_OVF(q, p)			((p) & Q_OVERFLOW_FLAG)
#define Q_ENT(q, p)			((q)->base +			\
					 Q_IDX(q, p) * (q)->ent_dwords)

#define Q_BASE_RWA			(1UL << 62)
#define Q_BASE_ADDR_SHIFT		5
#define Q_BASE_ADDR_MASK		0xfffffffffffUL
#define Q_BASE_LOG2SIZE_SHIFT		0
#define Q_BASE_LOG2SIZE_MASK		0x1fUL

/*
 * Stream table.
 *
 * Linear: Enough to cover 1 << IDR1.SIDSIZE entries
 * 2lvl: 128k L1 entries,
 *       256 lazy entries per table (each table covers a PCI bus)
 */
#define STRTAB_L1_SZ_SHIFT		20
#define STRTAB_SPLIT			8

#define STRTAB_L1_DESC_DWORDS		1
#define STRTAB_L1_DESC_SPAN_SHIFT	0
#define STRTAB_L1_DESC_SPAN_MASK	0x1fUL
#define STRTAB_L1_DESC_L2PTR_SHIFT	6
#define STRTAB_L1_DESC_L2PTR_MASK	0x3ffffffffffUL

#define STRTAB_STE_DWORDS		8
#define STRTAB_STE_0_V			(1UL << 0)
#define STRTAB_STE_0_CFG_SHIFT		1
#define STRTAB_STE_0_CFG_MASK		0x7UL
#define STRTAB_STE_0_CFG_ABORT		(0UL << STRTAB_STE_0_CFG_SHIFT)
#define STRTAB_STE_0_CFG_BYPASS		(4UL << STRTAB_STE_0_CFG_SHIFT)
#define STRTAB_STE_0_CFG_S1_TRANS	(5UL << STRTAB_STE_0_CFG_SHIFT)
#define STRTAB_STE_0_CFG_S2_TRANS	(6UL << STRTAB_STE_0_CFG_SHIFT)

#define STRTAB_STE_0_S1FMT_SHIFT	4
#define STRTAB_STE_0_S1FMT_LINEAR	(0UL << STRTAB_STE_0_S1FMT_SHIFT)
#define STRTAB_STE_0_S1CTXPTR_SHIFT	6
#define STRTAB_STE_0_S1CTXPTR_MASK	0x3ffffffffffUL
#define STRTAB_STE_0_S1CDMAX_SHIFT	59
#define STRTAB_STE_0_S1CDMAX_MASK	0x1fUL

#define STRTAB_STE_1_S1C_CACHE_NC	0UL
#define STRTAB_STE_1_S1C_CACHE_WBRA	1UL
#define STRTAB_STE_1_S1C_CACHE_WT	2UL
#define STRTAB_STE_1_S1C_CACHE_WB	3UL
#define STRTAB_STE_1_S1C_SH_NSH		0UL
#define STRTAB_STE_1_S1C_SH_OSH		2UL
#define STRTAB_STE_1_S1C_SH_ISH		3UL
#define STRTAB_STE_1_S1CIR_SHIFT	2
#define STRTAB_STE_1_S1COR_SHIFT	4
#define STRTAB_STE_1_S1CSH_SHIFT	6

#define STRTAB_STE_1_S1STALLD		(1UL << 27)

#define STRTAB_STE_1_EATS_ABT		0UL
#define STRTAB_STE_1_EATS_TRANS		1UL
#define STRTAB_STE_1_EATS_S1CHK		2UL
#define STRTAB_STE_1_EATS_SHIFT		28

#define STRTAB_STE_1_STRW_NSEL1		0UL
#define STRTAB_STE_1_STRW_EL2		2UL
#define STRTAB_STE_1_STRW_SHIFT		30

#define STRTAB_STE_1_SHCFG_INCOMING	1UL
#define STRTAB_STE_1_SHCFG_SHIFT	44
#define STRTAB_STE_1_SHCFG_OFFSET	1

#define STRTAB_STE_2_S2VMID_SHIFT	0
#define STRTAB_STE_2_S2VMID_MASK	0xffffUL
#define STRTAB_STE_2_S2VMID_OFFSET	2
#define STRTAB_STE_2_VTCR_SHIFT		32
#define STRTAB_STE_2_VTCR_MASK		0x7ffffUL
#define STRTAB_STE_2_S2AA64		(1UL << 51)
#define STRTAB_STE_2_S2ENDI		(1UL << 52)
#define STRTAB_STE_2_S2PTW		(1UL << 54)
#define STRTAB_STE_2_S2R		(1UL << 58)

#define STRTAB_STE_3_S2TTB_SHIFT	4
#define STRTAB_STE_3_S2TTB_MASK		0xfffffffffffUL
#define STRTAB_STE_3_S2TTB_OFFSET		3

#define STRTAB_SURPPOT_SPLIT		6

/* Context descriptor (stage-1 only) */
#define CTXDESC_CD_DWORDS		8
#define CTXDESC_CD_MAX_SSIDS		(1 << 4)
#define CTXDESC_CD_0_TCR_T0SZ_SHIFT	0
#define ARM64_TCR_T0SZ_SHIFT		0
#define ARM64_TCR_T0SZ_MASK		0x1fUL
#define CTXDESC_CD_0_TCR_TG0_SHIFT	6
#define ARM64_TCR_TG0_SHIFT		14
#define ARM64_TCR_TG0_MASK		0x3UL
#define CTXDESC_CD_0_TCR_IRGN0_SHIFT	8
#define ARM64_TCR_IRGN0_SHIFT		8
#define ARM64_TCR_IRGN0_MASK		0x3UL
#define CTXDESC_CD_0_TCR_ORGN0_SHIFT	10
#define ARM64_TCR_ORGN0_SHIFT		10
#define ARM64_TCR_ORGN0_MASK		0x3UL
#define CTXDESC_CD_0_TCR_SH0_SHIFT	12
#define ARM64_TCR_SH0_SHIFT		12
#define ARM64_TCR_SH0_MASK		0x3UL
#define CTXDESC_CD_0_TCR_EPD0_SHIFT	14
#define ARM64_TCR_EPD0_SHIFT		7
#define ARM64_TCR_EPD0_MASK		0x1UL
#define CTXDESC_CD_0_TCR_EPD1_SHIFT	30
#define ARM64_TCR_EPD1_SHIFT		23
#define ARM64_TCR_EPD1_MASK		0x1UL

#define CTXDESC_CD_0_ENDI		(1UL << 15)
#define CTXDESC_CD_0_V			(1UL << 31)

#define CTXDESC_CD_0_TCR_IPS_SHIFT	32
#define ARM64_TCR_IPS_SHIFT		32
#define ARM64_TCR_IPS_MASK		0x7UL
#define CTXDESC_CD_0_TCR_TBI0_SHIFT	38
#define ARM64_TCR_TBI0_SHIFT		37
#define ARM64_TCR_TBI0_MASK		0x1UL

#define CTXDESC_CD_0_AA64		(1UL << 41)
#define CTXDESC_CD_0_R			(1UL << 45)
#define CTXDESC_CD_0_A			(1UL << 46)
#define CTXDESC_CD_0_ASET_SHIFT		47
#define CTXDESC_CD_0_ASET_SHARED	(0UL << CTXDESC_CD_0_ASET_SHIFT)
#define CTXDESC_CD_0_ASET_PRIVATE	(1UL << CTXDESC_CD_0_ASET_SHIFT)
#define CTXDESC_CD_0_ASID_SHIFT		48
#define CTXDESC_CD_0_ASID_MASK		0xffffUL

#define CTXDESC_CD_1_TTB0_SHIFT		4
#define CTXDESC_CD_1_TTB0_MASK		0xfffffffffffUL
#define CTXDESC_CD_1_TTB0_OFFSET		1

#define CTXDESC_CD_3_MAIR_SHIFT		0
#define CTXDESC_CD_3_MAIR_OFFSET		3

/* Convert between AArch64 (CPU) TCR format and SMMU CD format */
#define ARM_SMMU_TCR2CD(tcr, fld)					\
	((((tcr) >> ARM64_TCR_##fld##_SHIFT) & ARM64_TCR_##fld##_MASK)	\
	 << CTXDESC_CD_0_TCR_##fld##_SHIFT)

/* Command queue */
#define CMDQ_ENT_DWORDS			2
#define CMDQ_MAX_SZ_SHIFT		8

#define CMDQ_ERR_SHIFT			24
#define CMDQ_ERR_MASK			0x7f
#define CMDQ_ERR_CERROR_NONE_IDX	0
#define CMDQ_ERR_CERROR_ILL_IDX		1
#define CMDQ_ERR_CERROR_ABT_IDX		2

#define CMDQ_0_OP_SHIFT			0
#define CMDQ_0_OP_MASK			0xffUL
#define CMDQ_0_SSV			(1UL << 11)

#define CMDQ_PREFETCH_0_SID_SHIFT	32
#define CMDQ_PREFETCH_1_SIZE_SHIFT	0
#define CMDQ_PREFETCH_1_ADDR_MASK	~0xfffUL

#define CMDQ_CFGI_0_CD_SHIFT		12
#define CMDQ_CFGI_0_CD_MASK		0xfffff000UL
#define CMDQ_CFGI_0_SID_SHIFT		32
#define CMDQ_CFGI_0_SID_MASK		0xffffffffUL
#define CMDQ_CFGI_1_LEAF		(1UL << 0)
#define CMDQ_CFGI_1_RANGE_SHIFT		0
#define CMDQ_CFGI_1_RANGE_MASK		0x1fUL

#define CMDQ_TLBI_0_VMID_SHIFT		32
#define CMDQ_TLBI_0_ASID_SHIFT		48
#define CMDQ_TLBI_1_LEAF		(1UL << 0)
#define CMDQ_TLBI_1_VA_MASK		~0xfffUL
#define CMDQ_TLBI_1_IPA_MASK		0xfffffffff000UL

#define CMDQ_PRI_0_SSID_SHIFT		12
#define CMDQ_PRI_0_SSID_MASK		0xfffffUL
#define CMDQ_PRI_0_SID_SHIFT		32
#define CMDQ_PRI_0_SID_MASK		0xffffffffUL
#define CMDQ_PRI_1_GRPID_SHIFT		0
#define CMDQ_PRI_1_GRPID_MASK		0x1ffUL
#define CMDQ_PRI_1_RESP_SHIFT		12
#define CMDQ_PRI_1_RESP_DENY		(0UL << CMDQ_PRI_1_RESP_SHIFT)
#define CMDQ_PRI_1_RESP_FAIL		(1UL << CMDQ_PRI_1_RESP_SHIFT)
#define CMDQ_PRI_1_RESP_SUCC		(2UL << CMDQ_PRI_1_RESP_SHIFT)

#define CMDQ_SYNC_0_CS_SHIFT		12
#define CMDQ_SYNC_0_CS_NONE		(0UL << CMDQ_SYNC_0_CS_SHIFT)
#define CMDQ_SYNC_0_CS_SEV		(2UL << CMDQ_SYNC_0_CS_SHIFT)

/* Event queue */
#define EVTQ_ENT_DWORDS			4
#define EVTQ_MAX_SZ_SHIFT		7

#define EVTQ_0_ID_SHIFT			0
#define EVTQ_0_ID_MASK			0xffUL

#define EVTQ_0_STREAM_ID_SHIFT	32
#define EVTQ_0_STREAM_ID_MASK		0xffffffffUL

#define EVTQ_0_SSID_SHIFT  12
#define EVTQ_0_SSID_MASK   0xffff

#define EVTQ_TYPE_STREAM_DISABLE           0x06
#define EVTQ_TYPE_TRANSL_FORBIDDEN         0x07
#define EVTQ_TYPE_WALK_EABT                0x0b
#define EVTQ_TYPE_TRANSLATION              0x10
#define EVTQ_TYPE_ADDR_SIZE                0x11
#define EVTQ_TYPE_ACCESS                   0x12
#define EVTQ_TYPE_PERMISSION               0x13

#define EVTQ_TYPE_WITH_ADDR(id)     ((EVTQ_TYPE_TRANSLATION == (id))   \
				|| (EVTQ_TYPE_WALK_EABT == (id))   \
				|| (EVTQ_TYPE_TRANSL_FORBIDDEN == (id)) \
				|| (EVTQ_TYPE_ADDR_SIZE == (id))   \
				|| (EVTQ_TYPE_ACCESS == (id))      \
				|| (EVTQ_TYPE_PERMISSION == (id)))

#define EVTQ_TYPE_WITHOUT_SSID(id)  ((EVTQ_TYPE_STREAM_DISABLE == (id))   \
				|| (EVTQ_TYPE_TRANSL_FORBIDDEN == (id)))
/* PRI queue */
#define PRIQ_ENT_DWORDS			2
#define PRIQ_MAX_SZ_SHIFT		8

#define PRIQ_0_SID_SHIFT		32
#define PRIQ_0_SID_MASK			0xffffffffUL
#define PRIQ_0_SSID_SHIFT		32
#define PRIQ_0_SSID_MASK		0xfffffUL
#define PRIQ_0_PERM_PRIV		(1UL << 58)
#define PRIQ_0_PERM_EXEC		(1UL << 59)
#define PRIQ_0_PERM_READ		(1UL << 60)
#define PRIQ_0_PERM_WRITE		(1UL << 61)
#define PRIQ_0_PRG_LAST			(1UL << 62)
#define PRIQ_0_SSID_V			(1UL << 63)

#define PRIQ_1_PRG_IDX_SHIFT		0
#define PRIQ_1_PRG_IDX_MASK		0x1ffUL
#define PRIQ_1_ADDR_SHIFT		12
#define PRIQ_1_ADDR_MASK		0xfffffffffffffUL

/* High-level queue structures */
#define ARM_SMMU_POLL_TIMEOUT_US	100000 /* 100ms */
#define ARM_SMMU_CMDQ_DRAIN_TIMEOUT_US	1000000 /* 1s! */

#define MSI_IOVA_BASE			0x8000000
#define MSI_IOVA_LENGTH			0x100000

/* Until ACPICA headers cover IORT rev. C */
#ifndef ACPI_IORT_SMMU_HISILICON_HI161X
#define ACPI_IORT_SMMU_HISILICON_HI161X		0x1
#endif

#ifndef ACPI_IORT_SMMU_V3_CAVIUM_CN99XX
#define ACPI_IORT_SMMU_V3_CAVIUM_CN99XX		0x2
#endif

#ifdef HISI_SMMUV3_SUPPORT
/* TCU CACHE */
#define MM_TCU_CACHE                     0x31000
#define TCU_CACHE_BYPASS                  (MM_TCU_CACHE + 0x0)
#define TCU_CACHELINE_INV_ALL             (MM_TCU_CACHE + 0x4)
/* TCU CTRL */
#define MM_TOP_CTL_BASE                  0x30000
#define SMMU_LP_REQ                       (MM_TOP_CTL_BASE + 0)
#define TCU_QREQN_CG                      (1 << 0)
#define TCU_QREQN_PD                      (1 << 1)
#define SMMU_LP_ACK                       (MM_TOP_CTL_BASE + 0x4)
#define TCU_QACCEPTN_CG                   (1 << 0)
#define TCU_QACCEPTN_PD                   (1 << 4)
#define PCTRL_PERI_STAT10_REG             0xfec3e0bc
#define STREAM_ID_BYPASS                   63
#define ARM_SMMU_IRPT_MASK_NS	(MM_TOP_CTL_BASE + 0x70)
#define ARM_SMMU_IRPT_RAW_NS	(MM_TOP_CTL_BASE + 0x74)
#define TCU_EVENT_TO_MASK		0x38
#define TCU_EVENT_MASK_ALL		0xFFFFFFFF
#define TCU_IRPT_CLR_NS_VAL_MASK	0xffffffff
#define ARM_SMMU_IRPT_CLR_NS		(MM_TOP_CTL_BASE + 0x7c)
#define TCU_EVENT_Q_IRQ_CLR		BIT(0)
#define TCU_CMD_SYNC_IRQ_CLR		BIT(1)
#define TCU_GERROR_IRQ_CLR		BIT(2)
#define TCU_EVENTTO_CLR		BIT(5)
#define INVALID_TCU_PW_CFG	0xFFFFFFFF
#define ARM_SMMU_STRTAB_BASE1		0x84
#define ARM_SMMU_CMDQ_BASE1		0x94
#define ARM_SMMU_EVTQ_BASE1		0xA4
#endif
#define IOVA_DEFAULT_ADDS		0x100000
#define IOVA_DEFAULT_SIZE		0xc0000000
#define DELAY_COEF                         2
#define CMDQ_MAX_TIMEOUT_TIMES	3
#define IOMMU_CELLS_VALUE		2
#define DWORD_BYTES_NUM		3
#define ARM_SMMU_ADDR_SIZE_32	32
#define ARM_SMMU_ADDR_SIZE_36	36
#define ARM_SMMU_ADDR_SIZE_40	40
#define ARM_SMMU_ADDR_SIZE_42	42
#define ARM_SMMU_ADDR_SIZE_44	44
#define ARM_SMMU_ADDR_SIZE_48	48
#define ARM_SMMU_ID_SIZE_8		8
#define ARM_SMMU_ID_SIZE_16		16
#define MAX_CHECK_TIMES	1000

#define ARM_LPAE_TCR_EPD1 BIT(23)
#define ARM_LPAE_TCR_T0SZ_SHIFT 0
#define ARM_LPAE_TCR_IPS_SHIFT 32
#define ARM_LPAE_TCR_IPS_MASK 0x7

#define ARM_LPAE_TCR_PS_32_BIT 0x0ULL
#define ARM_LPAE_TCR_PS_36_BIT 0x1ULL
#define ARM_LPAE_TCR_PS_40_BIT 0x2ULL
#define ARM_LPAE_TCR_PS_42_BIT 0x3ULL
#define ARM_LPAE_TCR_PS_44_BIT 0x4ULL
#define ARM_LPAE_TCR_PS_48_BIT 0x5ULL

#define ARM_LPAE_MAIR_ATTR_DEVICE 0x04
#define ARM_LPAE_MAIR_ATTR_NC 0x44
#define ARM_LPAE_MAIR_ATTR_WBRWA 0xff
#define ARM_LPAE_MAIR_ATTR_IDX_NC 0
#define ARM_LPAE_MAIR_ATTR_IDX_CACHE 1
#define ARM_LPAE_MAIR_ATTR_IDX_DEV 2
#define arm_lpae_mair_attr_shift(n) ((n) << 3)
#define TCU_CACHE_SOFT_INV  0x3
#define ARM_SMMU_SVM_S1CFG_VMID 0
#define LAZY_FREE_WATERLINE	10
#define WATCHDOG_CPU	0
#define WIFI_INTX_CPU	4
#define LAZY_FREE_SCHED_PRI	97

#ifdef CONFIG_MM_TCU
#define CD_TTBR_DATA_MASK       ((1UL << CTXDESC_CD_0_ASID_SHIFT) - 1)
#endif

static bool disable_bypass;
module_param_named(disable_bypass, disable_bypass, bool, 0444);
MODULE_PARM_DESC(disable_bypass,
	"Disable bypass streams such that incoming transactions from devices that are not attached to an iommu domain will report an abort back to the device and will not be allowed to pass through the SMMU.");

enum pri_resp {
	PRI_RESP_DENY,
	PRI_RESP_FAIL,
	PRI_RESP_SUCC,
};

enum arm_smmu_msi_index {
	EVTQ_MSI_INDEX,
	GERROR_MSI_INDEX,
	PRIQ_MSI_INDEX,
	ARM_SMMU_MAX_MSIS,
};

enum tlb_flush_ops {
	ASID_FLUSH, /* arm smmu driver flush tlb by asid */
	SSID_FLUSH, /* arm smmu driver flush tlb by ssid */
	DEV_FLUSH,  /* master dev driver flush tlb by ssid */
};

#ifdef HISI_SMMUV3_SUPPORT
/* smmuid must match with dtsi smmuid definition */
enum {
	SMMU_MEDIA1,
	SMMU_MEDIA2,
	SMMU_NPU,
	SMMU_HSDT0,
	SMMU_HSDT1,
};
#endif

struct arm_smmu_cmdq_ent {
	/* Common fields */
	u8				opcode;
	bool				substream_valid;

	/* Command-specific fields */
	union {
#define CMDQ_OP_PREFETCH_CFG	0x1
		struct {
			u32			sid;
			u8			size;
			u64			addr;
		} prefetch;

#define CMDQ_OP_CFGI_STE	0x3
#define CMDQ_OP_CFGI_ALL	0x4
#define CMDQ_OP_CFGI_CD		0x5
		struct {
			u32			sid;
			u32			ssid;
			union {
				bool		leaf;
				u8		span;
			};
		} cfgi;

#define CMDQ_OP_TLBI_NH_ASID	0x11
#define CMDQ_OP_TLBI_NH_VA	0x12
#define CMDQ_OP_TLBI_EL2_ALL	0x20
#define CMDQ_OP_TLBI_S12_VMALL	0x28
#define CMDQ_OP_TLBI_S2_IPA	0x2a
#define CMDQ_OP_TLBI_NSNH_ALL	0x30
		struct {
			u16			asid;
			u16			vmid;
			bool			leaf;
			u64			addr;
		} tlbi;

#define CMDQ_OP_PRI_RESP	0x41
		struct {
			u32			sid;
			u32			ssid;
			u16			grpid;
			enum pri_resp		resp;
		} pri;

#define CMDQ_OP_CMD_SYNC	0x46
	};
};

struct arm_smmu_queue {
	int				irq; /* Wired interrupt */

	__le64				*base;
	dma_addr_t			base_dma;
	u64				q_base;

	size_t				ent_dwords;
	u32				max_n_shift;
	u32				prod;
	u32				cons;

	u32 __iomem			*prod_reg;
	u32 __iomem			*cons_reg;
};

struct arm_smmu_cmdq {
	struct arm_smmu_queue		q;
	spinlock_t			lock;
};

struct arm_smmu_evtq {
	struct arm_smmu_queue		q;
	u32				max_stalls;
};

struct arm_smmu_priq {
	struct arm_smmu_queue		q;
};

/* High-level stream table and context descriptor structures */
struct arm_smmu_strtab_l1_desc {
	u8				span;

	__le64				*l2ptr;
	dma_addr_t			l2ptr_dma;
};

struct arm_smmu_s1_cfg {
	__le64				*cdptr;
	dma_addr_t			cdptr_dma;
	u8				tlb_flush;
	u16				vmid;
	struct arm_smmu_ctx_desc {
		u16	ssid;
		u16	asid;
		u32	ssid_nums;
		u64	ttbr;
		u64	tcr;
		u64	mair;
	}				cd;
};

struct arm_smmu_s2_cfg {
	u16				vmid;
	u64				vttbr;
	u64				vtcr;
};

struct arm_smmu_strtab_ent {
	/*
	 * An STE is "assigned" if the master emitting the corresponding SID
	 * is attached to a domain. The behaviour of an unassigned STE is
	 * determined by the disable_bypass parameter, whereas an assigned
	 * STE behaves according to s1_cfg/s2_cfg, which themselves are
	 * configured according to the domain type.
	 */
	bool				assigned;
	struct arm_smmu_s1_cfg		*s1_cfg;
	struct arm_smmu_s2_cfg		*s2_cfg;
};

struct arm_smmu_strtab_cfg {
	__le64				*strtab;
	dma_addr_t			strtab_dma;
	struct arm_smmu_strtab_l1_desc	*l1_desc;
	unsigned int			num_l1_ents;

	u64				strtab_base;
	u32				strtab_base_cfg;
};

struct arm_smmu_iommu_group {
	u32				sid;
	struct list_head			list;
	struct iommu_group		*group;
	struct blocking_notifier_head	notifier;
};

struct arm_smmu_tcu_pw {
	u32	lp_req_reg;
	u32	lp_ack_reg;
	u32	clk_gate_req;
	u32	clk_gate_ack;
	u32	pw_stat_req;
	u32	pw_stat_ack;
};

/* An SMMUv3 instance */
struct arm_smmu_device {
	struct device			*dev;
	void __iomem			*base;

#define ARM_SMMU_FEAT_2_LVL_STRTAB	(1 << 0)
#define ARM_SMMU_FEAT_2_LVL_CDTAB	(1 << 1)
#define ARM_SMMU_FEAT_TT_LE		(1 << 2)
#define ARM_SMMU_FEAT_TT_BE		(1 << 3)
#define ARM_SMMU_FEAT_PRI		(1 << 4)
#define ARM_SMMU_FEAT_ATS		(1 << 5)
#define ARM_SMMU_FEAT_SEV		(1 << 6)
#define ARM_SMMU_FEAT_MSI		(1 << 7)
#define ARM_SMMU_FEAT_COHERENCY		(1 << 8)
#define ARM_SMMU_FEAT_TRANS_S1		(1 << 9)
#define ARM_SMMU_FEAT_TRANS_S2		(1 << 10)
#define ARM_SMMU_FEAT_STALLS		(1 << 11)
#define ARM_SMMU_FEAT_HYP		(1 << 12)
#define ARM_SMMU_FEAT_TCU_CACHE		(1 << 20)
	u32				features;

#define ARM_SMMU_OPT_SKIP_PREFETCH	(1 << 0)
#define ARM_SMMU_OPT_PAGE0_REGS_ONLY	(1 << 1)
	u32				options;

	struct arm_smmu_tcu_pw		pw_cfg;
	struct arm_smmu_cmdq		cmdq;
	struct arm_smmu_evtq		evtq;
	struct arm_smmu_priq		priq;

	int				gerr_irq;
	int				combined_irq;

	unsigned long			ias; /* IPA */
	unsigned long			oas; /* PA */
	unsigned long			pgsize_bitmap;

#define ARM_SMMU_MAX_ASIDS		(1 << 16)
	unsigned int			asid_bits;
	DECLARE_BITMAP(asid_map, ARM_SMMU_MAX_ASIDS);

#define ARM_SMMU_MAX_VMIDS		(1 << 16)
	unsigned int			vmid_bits;
	DECLARE_BITMAP(vmid_map, ARM_SMMU_MAX_VMIDS);

	unsigned int			ssid_bits;
	unsigned int			sid_bits;

	struct arm_smmu_strtab_cfg	strtab_cfg;

	/* IOMMU core code handle */
	struct iommu_device		iommu;
	int				status;
	int                             smmuid;

	spinlock_t			iommu_groups_lock;
	struct list_head		iommu_groups;
	bool				bypass;
	spinlock_t			tcu_lock;
	struct notifier_block		tcu_pm_nb;
	unsigned long                   cnt;
	void                            *crg_vbase;
	u64                             evt[EVTQ_ENT_DWORDS];
};

/* SMMU private data for each master */
struct arm_smmu_master_data {
	struct arm_smmu_device		*smmu;
	struct arm_smmu_domain		*smmu_domain;
	struct arm_smmu_strtab_ent	ste;
	struct iommu_domain_data	iova;
	u8				tlb_flush;
};

/* SMMU private data for an IOMMU domain */
enum arm_smmu_domain_stage {
	ARM_SMMU_DOMAIN_S1 = 0,
	ARM_SMMU_DOMAIN_S2,
	ARM_SMMU_DOMAIN_NESTED,
	ARM_SMMU_DOMAIN_BYPASS,
};

enum arm_smmu_status {
	ARM_SMMU_DISABLE,
	ARM_SMMU_ENABLE,
};

enum arm_smmu_domain_type {
	ARM_SMMU_DOMAIN_DMA,
	ARM_SMMU_DOMAIN_SVM,
};

struct arm_smmu_domain {
	struct arm_smmu_device		*smmu;
	struct mutex			init_mutex; /* Protects smmu pointer */

	struct io_pgtable_ops		*pgtbl_ops;

	enum arm_smmu_domain_stage	stage;
	union {
		struct arm_smmu_s1_cfg	s1_cfg;
		struct arm_smmu_s2_cfg	s2_cfg;
	};

	struct iommu_domain		domain;
	struct device			*dev; /* svm domain use */
	u32				type; /* domain type:dma or svm */
	u8				tlb_flush;

	DECLARE_BITMAP(ssid_map, CTXDESC_CD_MAX_SSIDS); /* svm domain use */
};

struct arm_v3_domain {
	struct arm_smmu_domain smmu_domain;
	struct list_head list;
};

struct arm_smmu_option_prop {
	u32 opt;
	const char *prop;
};

struct finalise_stage_ops {
	int (*finalise_stage_fn)(struct arm_smmu_domain *smmu_domain,
				struct io_pgtable_cfg *pgtbl_cfg);
};

static struct arm_smmu_option_prop arm_smmu_options[] = {
	{ ARM_SMMU_OPT_SKIP_PREFETCH, "hisilicon,broken-prefetch-cmd" },
	{ ARM_SMMU_OPT_PAGE0_REGS_ONLY, "cavium,cn9900-broken-page1-regspace"},
	{ 0, NULL},
};

#define REG_NAME_LEN  30
struct reg_info {
	char name[REG_NAME_LEN];
	u32 offset;
};

static int ste_force_linear = 1;
static struct arm_smmu_device *smmu_media1;
static struct arm_smmu_device *smmu_media2;

static bool arm_smmu_sid_in_range(struct arm_smmu_device *smmu, u32 sid);
static void arm_smmu_sync_cd(struct arm_smmu_domain *dom);
static __le64 *arm_smmu_get_step_for_sid(struct arm_smmu_device *smmu, u32 sid);
static int arm_smmu_evt_notify(struct arm_smmu_device *smmu, u64 *evt, u8 len);
static struct arm_smmu_device *arm_smmu_get_dev_smmu(struct device *dev);

#ifdef HISI_SMMUV3_SUPPORT
static void arm_smmu_invalid_tcu_cache(struct arm_smmu_device *smmu);
static bool arm_smmu_get_tcu_pw_cfg(struct arm_smmu_device *smmu, struct device *dev);
static int hisi_tcu_cache_bypass(struct arm_smmu_device *smmu)
{
	if (!(smmu->features & ARM_SMMU_FEAT_TCU_CACHE))
		return 0;

	/* only npu system dont bypass tcu cache */
	if (smmu->smmuid == SMMU_NPU)
		return 0;

	writel_relaxed(1, smmu->base + TCU_CACHE_BYPASS);
	return 0;
}
#endif

static inline void __iomem *arm_smmu_page1_fixup(unsigned long offset,
						 struct arm_smmu_device *smmu)
{
	if ((offset > SZ_64K) &&
	    (smmu->options & ARM_SMMU_OPT_PAGE0_REGS_ONLY))
		offset -= SZ_64K;

	return smmu->base + offset;
}

static struct arm_smmu_domain *to_smmu_domain(struct iommu_domain *dom)
{
	return container_of(dom, struct arm_smmu_domain, domain);
}

static void parse_driver_options(struct arm_smmu_device *smmu)
{
	int i = 0;

	do {
		if (of_property_read_bool(smmu->dev->of_node,
						arm_smmu_options[i].prop)) {
			smmu->options |= arm_smmu_options[i].opt;
			dev_notice(smmu->dev, "option %s\n",
				arm_smmu_options[i].prop);
		}
	} while (arm_smmu_options[++i].opt);
}

/* Low-level queue manipulation functions */

static void queue_sync_cons(struct arm_smmu_queue *q)
{
	q->cons = readl_relaxed(q->cons_reg);
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

static void queue_inc_cons(struct arm_smmu_queue *q)
{
	u32 cons = (Q_WRP(q, q->cons) | Q_IDX(q, q->cons)) + 1;

	q->cons = Q_OVF(q, q->cons) | Q_WRP(q, cons) | Q_IDX(q, cons);

	/*
	 * Ensure that all CPU accesses (reads and writes) to the queue
	 * are complete before we update the cons pointer.
	 */
	mb();
	writel_relaxed(q->cons, q->cons_reg);
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
	ktime_t timeout;
	unsigned int delay = 1;

	/* Wait longer if it's queue drain */
	timeout = ktime_add_us(ktime_get(), drain ?
					    ARM_SMMU_CMDQ_DRAIN_TIMEOUT_US :
					    ARM_SMMU_POLL_TIMEOUT_US);

	while (queue_sync_cons(q), (drain ? !queue_empty(q) : queue_full(q))) {
		if (ktime_compare(ktime_get(), timeout) > 0)
			return -ETIMEDOUT;

		if (wfe) {
			wfe();
		} else {
			cpu_relax();
			udelay(delay);
			delay *= DELAY_COEF;
		}
	}

	return 0;
}

static void queue_write(__le64 *dst, u64 *src, size_t n_dwords,
				unsigned long src_len)
{
	u32 i;
	unsigned long lmin = (n_dwords < src_len) ? n_dwords : src_len;

	for (i = 0; i < lmin; ++i)
		*dst++ = cpu_to_le64(*src++);
}

static int queue_insert_raw(struct arm_smmu_queue *q, u64 *ent,
				unsigned long ent_buf_len)
{
	if (queue_full(q))
		return -ENOSPC;

	queue_write(Q_ENT(q, q->prod), ent, q->ent_dwords, ent_buf_len);
	queue_inc_prod(q);
	return 0;
}

static void queue_read(__le64 *dst, u64 *src, size_t n_dwords,
				unsigned long dst_len)
{
	u32 i;
	unsigned long lmin = (n_dwords < dst_len) ? n_dwords : dst_len;

	for (i = 0; i < lmin; ++i)
		*dst++ = le64_to_cpu(*src++);
}

static int queue_remove_raw(struct arm_smmu_queue *q, u64 *ent,
				unsigned long ent_buf_len)
{
	if (queue_empty(q))
		return -EAGAIN;

	queue_read(ent, Q_ENT(q, q->cons), q->ent_dwords, ent_buf_len);
	queue_inc_cons(q);
	return 0;
}

static int arm_smmu_cmdq_op_pri_resp(u64 *cmd, struct arm_smmu_cmdq_ent *ent)
{
	cmd[0] |= ent->substream_valid ? CMDQ_0_SSV : 0;
	cmd[0] |= ent->pri.ssid << CMDQ_PRI_0_SSID_SHIFT;
	cmd[0] |= (u64)ent->pri.sid << CMDQ_PRI_0_SID_SHIFT;
	cmd[1] |= ent->pri.grpid << CMDQ_PRI_1_GRPID_SHIFT;

	switch (ent->pri.resp) {
	case PRI_RESP_DENY:
		cmd[1] |= CMDQ_PRI_1_RESP_DENY;
		break;
	case PRI_RESP_FAIL:
		cmd[1] |= CMDQ_PRI_1_RESP_FAIL;
		break;
	case PRI_RESP_SUCC:
		cmd[1] |= CMDQ_PRI_1_RESP_SUCC;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void arm_smmu_cmdq_op_cfgi(u8 opcode, u64 *cmd,
				struct arm_smmu_cmdq_ent *ent)
{
	switch (opcode) {
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
	default:
		pr_err("%s, opcode %u error\n", __func__, opcode);
		break;
	}
}

/* High-level queue accessors */
static int arm_smmu_cmdq_build_cmd(u64 *cmd, struct arm_smmu_cmdq_ent *ent,
				unsigned long cmd_buf_len)
{
	int ret;

	memset(cmd, 0, CMDQ_ENT_DWORDS << DWORD_BYTES_NUM);
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
	case CMDQ_OP_CFGI_ALL:
	case CMDQ_OP_CFGI_CD:
		arm_smmu_cmdq_op_cfgi(ent->opcode, cmd, ent);
		break;
	case CMDQ_OP_TLBI_NH_VA:
		cmd[0] |= (u64)ent->tlbi.asid << CMDQ_TLBI_0_ASID_SHIFT;
		cmd[0] |= (u64)ent->tlbi.vmid << CMDQ_TLBI_0_VMID_SHIFT;
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
	case CMDQ_OP_PRI_RESP:
		ret = arm_smmu_cmdq_op_pri_resp(cmd, ent);
		if (ret)
			return ret;
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
		[CMDQ_ERR_CERROR_NONE_IDX]	= "No error",
		[CMDQ_ERR_CERROR_ILL_IDX]	= "Illegal command",
		[CMDQ_ERR_CERROR_ABT_IDX]	= "Abort on command fetch",
	};

	int i;
	u64 cmd[CMDQ_ENT_DWORDS];
	struct arm_smmu_queue *q = &smmu->cmdq.q;
	u32 cons = readl_relaxed(q->cons_reg);
	u32 idx = (cons >> CMDQ_ERR_SHIFT) & CMDQ_ERR_MASK;
	struct arm_smmu_cmdq_ent cmd_sync = {
		.opcode = CMDQ_OP_CMD_SYNC,
	};

	dev_err(smmu->dev, "CMDQ error (cons 0x%08x): %s\n", cons,
		idx < ARRAY_SIZE(cerror_str) ?  cerror_str[idx] : "Unknown");

	switch (idx) {
	case CMDQ_ERR_CERROR_ABT_IDX:
		dev_err(smmu->dev, "retrying command fetch\n");
		/* Fallthrough */
	case CMDQ_ERR_CERROR_NONE_IDX:
		return;
	case CMDQ_ERR_CERROR_ILL_IDX:
		/* Fallthrough */
	default:
		break;
	}

	/*
	 * We may have concurrent producers, so we need to be careful
	 * not to touch any of the shadow cmdq state.
	 */
	queue_read(cmd, Q_ENT(q, cons), q->ent_dwords, ARRAY_SIZE(cmd));
	dev_err(smmu->dev, "skipping command in error state:\n");
	for (i = 0; i < ARRAY_SIZE(cmd); ++i)
		dev_err(smmu->dev, "\t0x%016llx\n", (unsigned long long)cmd[i]);

	/* Convert the erroneous command into a CMD_SYNC */
	if (arm_smmu_cmdq_build_cmd(cmd, &cmd_sync, 0)) {
		dev_err(smmu->dev, "failed to convert to CMD_SYNC\n");
		return;
	}

	queue_write(Q_ENT(q, cons), cmd, q->ent_dwords, ARRAY_SIZE(cmd));
}

static struct reg_info media1_crg[] = {
	{"PERCLKEN0",  SOC_MEDIA1_CRG_PERCLKEN0_ADDR(0)},
	{"PERSTAT0", SOC_MEDIA1_CRG_PERSTAT0_ADDR(0)},
	{"PERCLKEN1", SOC_MEDIA1_CRG_PERCLKEN1_ADDR(0)},
	{"PERSTAT1", SOC_MEDIA1_CRG_PERSTAT1_ADDR(0)},
	{"PERCLKEN2",  SOC_MEDIA1_CRG_PERCLKEN2_ADDR(0)},
	{"PERSTAT2",  SOC_MEDIA1_CRG_PERSTAT2_ADDR(0)},
	{"PERRSTSTAT0", SOC_MEDIA1_CRG_PERRSTSTAT0_ADDR(0)},
	{"PERRSTSTAT1", SOC_MEDIA1_CRG_PERRSTSTAT1_ADDR(0)},
	{"PERI_STAT1", SOC_MEDIA1_CRG_PERI_STAT1_ADDR(0)},
	{"PERI_STAT2", SOC_MEDIA1_CRG_PERI_STAT2_ADDR(0)},
};

static struct reg_info media2_crg[] = {
	{"PERCLKEN0", SOC_MEDIA2_CRG_PERCLKEN0_ADDR(0)},
	{"PERSTAT0", SOC_MEDIA2_CRG_PERSTAT0_ADDR(0)},
#ifdef SOC_MEDIA2_CRG_PERCLKEN1_ADDR
	{"PERCLKEN1", SOC_MEDIA2_CRG_PERCLKEN1_ADDR(0)},
#endif
#ifdef SOC_MEDIA2_CRG_PERSTAT1_ADDR
	{"PERSTAT1", SOC_MEDIA2_CRG_PERSTAT1_ADDR(0)},
#endif
	{"PERRSTSTAT0", SOC_MEDIA2_CRG_PERRSTSTAT0_ADDR(0)},
#ifdef SOC_MEDIA2_CRG_PERRSTSTAT1_ADDR
	{"PERRSTSTAT1", SOC_MEDIA2_CRG_PERRSTSTAT1_ADDR(0)},
#endif
#ifdef SOC_MEDIA2_CRG_PERI_STAT1_ADDR
	{"PERI_STAT1", SOC_MEDIA2_CRG_PERI_STAT1_ADDR(0)},
#endif
#ifdef SOC_MEDIA2_CRG_PERI_STAT2_ADDR
	{"PERI_STAT2", SOC_MEDIA2_CRG_PERI_STAT2_ADDR(0)},
#endif
#ifdef SOC_MEDIA2_CRG_INTR_STAT_BUS_NONIDLE_PEND_ADDR
	{"STAT_BUS_NONIDLE_PEND", SOC_MEDIA2_CRG_INTR_STAT_BUS_NONIDLE_PEND_ADDR(0)},
#else
	{"STAT_NOCBUS_NONIDLE_PEND",
			SOC_MEDIA2_CRG_INTR_STAT_NOCBUS_NONIDLE_PEND_ADDR(0)},
#endif
#ifdef SOC_MEDIA2_CRG_INTR_BUS_NONIDLE_PEND_ADDR
	{"BUS_NONIDLE_PEND", SOC_MEDIA2_CRG_INTR_BUS_NONIDLE_PEND_ADDR(0)},
#else
	{"NOCBUS_NONIDLE_PEND", SOC_MEDIA2_CRG_INTR_NOCBUS_NONIDLE_PEND_ADDR(0)},
#endif
	{"IPCLKRST_BYPASS", SOC_MEDIA2_CRG_IPCLKRST_BYPASS_ADDR(0)},
};

static struct reg_info smmu_tcu_info[] = {
	{"SMMU_CR0", ARM_SMMU_CR0},
	{"SMMU_CR0ACK", ARM_SMMU_CR0ACK},
	{"SMMU_IRQ_CTRLACK", ARM_SMMU_IRQ_CTRLACK},
	{"SMMU_GERROR", ARM_SMMU_GERROR},
	{"SMMU_GERRORN", ARM_SMMU_GERRORN},
	{"SMMU_STRTAB_BASE0", ARM_SMMU_STRTAB_BASE},
	{"SMMU_STRTAB_BASE1", ARM_SMMU_STRTAB_BASE1},
	{"SMMU_CMDQ_BASE0", ARM_SMMU_CMDQ_BASE},
	{"SMMU_CMDQ_BASE1", ARM_SMMU_CMDQ_BASE1},
	{"SMMU_CMDQ_PROD", ARM_SMMU_CMDQ_PROD},
	{"SMMU_CMDQ_CONS", ARM_SMMU_CMDQ_CONS},
	{"SMMU_EVENTQ_BASE0", ARM_SMMU_EVTQ_BASE},
	{"SMMU_EVENTQ_BASE1", ARM_SMMU_EVTQ_BASE1},
	{"SMMU_EVENTQ_PROD", ARM_SMMU_EVTQ_PROD},
	{"SMMU_EVENTQ_CONS", ARM_SMMU_EVTQ_CONS},
	{"SMMU_TCU_LP_ACK", SMMU_LP_ACK},
	{"SMMU_TCU_IRPT_MASK_NS", ARM_SMMU_IRPT_MASK_NS},
	{"SMMU_TCU_IRPT_RAW_NS", ARM_SMMU_IRPT_RAW_NS},
};

static void arm_smmu_tcu_info_dump(struct arm_smmu_device *smmu)
{
	u32 reg1;
	u32 reg2;
	struct arm_smmu_queue *q = NULL;

	dev_err(smmu->dev, "tcu info blow:\n");
	reg1 = readl_relaxed(smmu->base + ARM_SMMU_CMDQ_PROD);
	reg2 = readl_relaxed(smmu->base + ARM_SMMU_CMDQ_CONS);
	dev_err(smmu->dev,
		"CMD PROD:0x%x, CONS:0x%x\n", reg1, reg2);

	q = &smmu->cmdq.q;
	dev_err(smmu->dev, "[soft]prod:0x%x, cons:0x%x\n",
		q->prod, q->cons);

	dev_err(smmu->dev, "powercnt:0x%lx\n", smmu->cnt);

	reg1 = readl_relaxed(smmu->base + SMMU_LP_ACK);
	reg2 = readl_relaxed(smmu->base + SMMU_LP_REQ);
	dev_err(smmu->dev, "lp_ack:0x%x, req:0x%x\n", reg1, reg2);

	reg1 = readl_relaxed(smmu->base + ARM_SMMU_IRPT_RAW_NS);
	dev_err(smmu->dev, "IRPT_RAW_NS:0x%x\n", reg1);

	reg1 = readl_relaxed(smmu->base + ARM_SMMU_CR0);
	reg2 = readl_relaxed(smmu->base + ARM_SMMU_CR0ACK);
	dev_err(smmu->dev, "CR0:0x%x, ACK:0x%x\n", reg1, reg2);

	mm_smmu_tcu_node_status(smmu->smmuid);
}

static void arm_smmu_crg_info_dump(struct arm_smmu_device *smmu)
{
	u32 i;
	u32 reg;

	if (!smmu->crg_vbase)
		return;

	dev_err(smmu->dev, "subsys crg info:\n");
	if (smmu->smmuid == SMMU_MEDIA1) {
		for (i = 0; i < ARRAY_SIZE(media1_crg); i++) {
			reg = readl_relaxed(smmu->crg_vbase +
				media1_crg[i].offset);
			dev_err(smmu->dev, "reg:%s, val:0x%x\n",
				media1_crg[i].name, reg);
		}
	} else if (smmu->smmuid == SMMU_MEDIA2) {
		for (i = 0; i < ARRAY_SIZE(media2_crg); i++) {
			reg = readl_relaxed(smmu->crg_vbase +
				media2_crg[i].offset);
			dev_err(smmu->dev, "reg:%s, val:0x%x\n",
				media2_crg[i].name, reg);
		}
	}
}

static void arm_smmu_reg_info_dump(struct arm_smmu_device *smmu)
{
	arm_smmu_tcu_info_dump(smmu);
	arm_smmu_crg_info_dump(smmu);
	mm_smmu_dmss_status(smmu->smmuid);
}

static void arm_smmu_cmdq_issue_cmd(struct arm_smmu_device *smmu,
				struct arm_smmu_cmdq_ent *ent)
{
	u64 cmd[CMDQ_ENT_DWORDS];
	unsigned long flags;
	bool wfe = !!(smmu->features & ARM_SMMU_FEAT_SEV);
	struct arm_smmu_queue *q = &smmu->cmdq.q;
	int count = 0;

	if (arm_smmu_cmdq_build_cmd(cmd, ent, ARRAY_SIZE(cmd))) {
		dev_warn(smmu->dev, "ignoring unknown CMDQ opcode 0x%x\n",
			 ent->opcode);
		return;
	}

	spin_lock_irqsave(&smmu->tcu_lock, flags);
	if (smmu->status == ARM_SMMU_DISABLE)
		goto out_unlock;

	while (queue_insert_raw(q, cmd, ARRAY_SIZE(cmd)) == -ENOSPC) {
		if (queue_poll_cons(q, false, wfe)) {
			if (count >= CMDQ_MAX_TIMEOUT_TIMES)
				break;
			dev_err_ratelimited(smmu->dev,
				"CMDQ timeout,cnt:%d\n", count);
			arm_smmu_reg_info_dump(smmu);
			count++;
		}
	}

	if (ent->opcode == CMDQ_OP_CMD_SYNC && queue_poll_cons(q, true, wfe)) {
		dev_err_ratelimited(smmu->dev, "CMD_SYNC timeout\n");
		arm_smmu_reg_info_dump(smmu);
	}

out_unlock:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
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

static u16 arm_smmu_get_ssid(struct iommu_fwspec *fwspec)
{
	if (fwspec->num_ids <= 1)
		return (u16)-1;

	return (u16)fwspec->ids[1];
}

static u32 arm_smmu_get_domain_type(struct device *dev)
{
	int ret;
	u32 domain_type;

	ret = of_property_read_u32(dev->of_node, "domain_type",
			&domain_type);
	if (ret)
		return ARM_SMMU_DOMAIN_DMA;

	return domain_type;
}

static u32 arm_smmu_get_sid(struct iommu_fwspec *fwspec)
{
	if (fwspec->num_ids <= 0)
		return (u32)-1;

	return fwspec->ids[0];
}

static void arm_smmu_write_ctx_desc(struct arm_smmu_device *smmu,
				struct arm_smmu_s1_cfg *cfg)
{
	u16 cdptr_offset;
	u64 val;
	u64 i;
	u64 max_ssid;
	u64 asid;

	max_ssid = 1UL << smmu->ssid_bits;
	if (cfg->cd.ssid >=  max_ssid) {
		pr_err("%s begin ssid %u error! ssid %u, smmu(%s)\n", __func__,
			cfg->cd.ssid, smmu->ssid_bits, dev_name(smmu->dev));
		return;
	}

	if (cfg->tlb_flush == ASID_FLUSH)
		asid = (u64)cfg->cd.asid;
	else
		asid = (u64)cfg->cd.ssid;

	for (i = 0; i < cfg->cd.ssid_nums; i++) {
		if ((max_ssid - cfg->cd.ssid) <=  i) {
			pr_err("%s ssid %u error! bits %u num %u smmu(%s)\n",
				__func__, cfg->cd.ssid, smmu->ssid_bits,
				cfg->cd.ssid_nums,
				dev_name(smmu->dev));
			return;
		}
		/*
		 * We don't need to issue any invalidation here, as we'll
		 * invalidate the STE when installing the new entry anyway.
		 */
		val = arm_smmu_cpu_tcr_to_cd(cfg->cd.tcr) |
#ifdef __BIG_ENDIAN
		CTXDESC_CD_0_ENDI |
#endif
		CTXDESC_CD_0_R | CTXDESC_CD_0_A | CTXDESC_CD_0_ASET_PRIVATE |
		CTXDESC_CD_0_AA64 |
		((u64)(asid + i) << CTXDESC_CD_0_ASID_SHIFT) | CTXDESC_CD_0_V;

		cdptr_offset = (cfg->cd.ssid + i) * CTXDESC_CD_DWORDS;
		cfg->cdptr[cdptr_offset] = cpu_to_le64(val);

		val = cfg->cd.ttbr & CTXDESC_CD_1_TTB0_MASK <<
			CTXDESC_CD_1_TTB0_SHIFT;
		cfg->cdptr[cdptr_offset + CTXDESC_CD_1_TTB0_OFFSET] =
			cpu_to_le64(val);

		cfg->cdptr[cdptr_offset + CTXDESC_CD_3_MAIR_OFFSET] =
			cpu_to_le64(cfg->cd.mair << CTXDESC_CD_3_MAIR_SHIFT);
	}
}

/* Stream table manipulation functions */
static void arm_smmu_write_strtab_l1_desc(__le64 *dst,
				struct arm_smmu_strtab_l1_desc *desc)
{
	u64 val = 0;

	val |= (desc->span & STRTAB_L1_DESC_SPAN_MASK)
		<< STRTAB_L1_DESC_SPAN_SHIFT;
	val |= desc->l2ptr_dma &
	       STRTAB_L1_DESC_L2PTR_MASK << STRTAB_L1_DESC_L2PTR_SHIFT;

	*dst = cpu_to_le64(val);
}

static void arm_smmu_sync_ste_for_sid(struct arm_smmu_device *smmu, u32 sid)
{
	struct arm_smmu_cmdq_ent cmd = {
		.opcode	= CMDQ_OP_CFGI_STE,
		.cfgi	= {
			.sid	= sid,
			.leaf	= true,
		},
	};

	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	cmd.opcode = CMDQ_OP_CMD_SYNC;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
}

static void arm_smmu_write_strtab_ent(struct arm_smmu_device *smmu,
				u32 sid, __le64 *dst,
				struct arm_smmu_strtab_ent *ste)
{
	/*
	 * This is hideously complicated, but we only really care about
	 * three cases at the moment:
	 *
	 * 1. Invalid (all zero) -> bypass/fault (init)
	 * 2. Bypass/fault -> translation/bypass (attach)
	 * 3. Translation/bypass -> bypass/fault (detach)
	 *
	 * Given that we can't update the STE atomically and the SMMU
	 * doesn't read the thing in a defined order, that leaves us
	 * with the following maintenance requirements:
	 *
	 * 1. Update Config, return (init time STEs aren't live)
	 * 2. Write everything apart from dword 0, sync, write dword 0, sync
	 * 3. Update Config, sync
	 */
	u64 val = le64_to_cpu(dst[0]);
	bool ste_live = false;
	struct arm_smmu_cmdq_ent prefetch_cmd = {
		.opcode		= CMDQ_OP_PREFETCH_CFG,
		.prefetch	= {
			.sid	= sid,
		},
	};

	if (val & STRTAB_STE_0_V) {
		u64 cfg;

		cfg = val & STRTAB_STE_0_CFG_MASK << STRTAB_STE_0_CFG_SHIFT;
		switch (cfg) {
		case STRTAB_STE_0_CFG_BYPASS:
			break;
		case STRTAB_STE_0_CFG_S1_TRANS:
		case STRTAB_STE_0_CFG_S2_TRANS:
			ste_live = true;
			break;
		case STRTAB_STE_0_CFG_ABORT:
			if (disable_bypass)
				break;
			/* fall-through */
		default:
			BUG(); /* STE corruption */
		}
	}

	/* Nuke the existing STE_0 value, as we're going to rewrite it */
	val = STRTAB_STE_0_V;

	/* Bypass/fault */
	if (!ste->assigned || !(ste->s1_cfg || ste->s2_cfg)) {
		if (!ste->assigned && disable_bypass)
			val |= STRTAB_STE_0_CFG_ABORT;
		else
			val |= STRTAB_STE_0_CFG_BYPASS;

		dst[0] = cpu_to_le64(val);
		dst[STRTAB_STE_1_SHCFG_OFFSET] = cpu_to_le64(
			STRTAB_STE_1_SHCFG_INCOMING <<
			STRTAB_STE_1_SHCFG_SHIFT);
		dst[STRTAB_STE_2_S2VMID_OFFSET] = 0; /* Nuke the VMID */
		if (ste_live)
			arm_smmu_sync_ste_for_sid(smmu, sid);
		return;
	}

	if (ste->s1_cfg) {
		BUG_ON(ste_live);
		dst[STRTAB_STE_1_SHCFG_OFFSET] = 0;
		val |= (ste->s1_cfg->cdptr_dma & STRTAB_STE_0_S1CTXPTR_MASK
			<< STRTAB_STE_0_S1CTXPTR_SHIFT)
			| STRTAB_STE_0_CFG_S1_TRANS;
		/* TLB flush by VMID(SID) + ASID/SSID */
		dst[STRTAB_STE_2_S2VMID_OFFSET] = cpu_to_le64(ste->s1_cfg->vmid
			<< STRTAB_STE_2_S2VMID_SHIFT);
	}

	if (ste->s2_cfg) {
		BUG_ON(ste_live);
		dst[STRTAB_STE_2_S2VMID_OFFSET] = cpu_to_le64(
			 (ste->s2_cfg->vmid << STRTAB_STE_2_S2VMID_SHIFT) |
			 ((ste->s2_cfg->vtcr & STRTAB_STE_2_VTCR_MASK) <<
			 STRTAB_STE_2_VTCR_SHIFT) |
#ifdef __BIG_ENDIAN /*lint !e436*/
			 STRTAB_STE_2_S2ENDI |
#endif
			 STRTAB_STE_2_S2PTW | STRTAB_STE_2_S2AA64 |
			 STRTAB_STE_2_S2R);

		dst[STRTAB_STE_3_S2TTB_OFFSET] =
			cpu_to_le64(ste->s2_cfg->vttbr &
			STRTAB_STE_3_S2TTB_MASK << STRTAB_STE_3_S2TTB_SHIFT);

		val |= STRTAB_STE_0_CFG_S2_TRANS;
	}

	/*
	 * number of CDs pointed to by
	 * S1ContextPtr,check cdmax
	 */
	val |= 6UL << STRTAB_STE_0_S1CDMAX_SHIFT; /* set cdmax */
	dst[0] = cpu_to_le64(val);
	arm_smmu_sync_ste_for_sid(smmu, sid);

	/* It's likely that we'll want to use the new STE soon */
	if (!(smmu->options & ARM_SMMU_OPT_SKIP_PREFETCH))
		arm_smmu_cmdq_issue_cmd(smmu, &prefetch_cmd);
}

static void arm_smmu_init_bypass_stes(u64 *strtab, unsigned int nent)
{
	unsigned int i;
	struct arm_smmu_strtab_ent ste = { .assigned = false };

	for (i = 0; i < nent; ++i) {
		arm_smmu_write_strtab_ent(NULL, -1, strtab, &ste);
		strtab += STRTAB_STE_DWORDS;
	}
}

static int arm_smmu_init_l2_strtab(struct arm_smmu_device *smmu, u32 sid)
{
	size_t size;
	void *strtab = NULL;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	struct arm_smmu_strtab_l1_desc *desc =
		&cfg->l1_desc[sid >> STRTAB_SPLIT];

	if (desc->l2ptr)
		return 0;

	size = 1 << (STRTAB_SPLIT + ilog2(STRTAB_STE_DWORDS) +
		DWORD_BYTES_NUM);
	strtab = &cfg->strtab[(sid >> STRTAB_SPLIT) * STRTAB_L1_DESC_DWORDS];

	desc->span = STRTAB_SPLIT + 1;
	desc->l2ptr = dmam_alloc_coherent(smmu->dev, size, &desc->l2ptr_dma,
					  GFP_KERNEL | __GFP_ZERO);
	if (!desc->l2ptr) {
		dev_err(smmu->dev,
			"failed to allocate l2 stream table for SID %u\n",
			sid);
		return -ENOMEM;
	}

	arm_smmu_init_bypass_stes(desc->l2ptr, 1 << STRTAB_SPLIT);
	arm_smmu_write_strtab_l1_desc(strtab, desc);
	return 0;
}

static void arm_smmu_dump_ste(struct arm_smmu_device *smmu, u32 sid)
{
	int i;
	__le64 *step = NULL;

	if (sid >= (1 << smmu->sid_bits)) {
		dev_info(smmu->dev, "%s, sid %u invalid, sid_bits: %u\n",
			__func__, sid, smmu->sid_bits);
		return;
	}

	dev_info(smmu->dev, "Dump sid %u STE:\n", sid);
	step = arm_smmu_get_step_for_sid(smmu, sid);
	for (i = 0; i < STRTAB_STE_DWORDS; i++)
		dev_info(smmu->dev, "0x%016llx\n", le64_to_cpu(step[i]));
}

static __le64 *arm_smmu_get_cd_from_ste(struct arm_smmu_device *smmu,
				u32 sid, u32 ssid)
{
	u64 cd;
	__le64 *step = NULL;
	__le64 *cdp = NULL;
	__le64 *cde = NULL;
	const u64 cd_mask = 0xfffffffffffc0;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	if (sid >= (1UL << smmu->sid_bits) ||
		ssid >= (1UL << smmu->ssid_bits)) {
		pr_err("%s, bad sid:%u, ssid:%u\n", __func__, sid, ssid);
		return NULL;
	}
	step = &cfg->strtab[sid * STRTAB_STE_DWORDS];
	cd = (step[0] & cd_mask); /* cd phys */
	cdp = phys_to_virt(cd); /* cd virt */
	cde = &cdp[ssid * CTXDESC_CD_DWORDS]; /* cd entry */

	return cde;
}

static void arm_smmu_dump_cd(struct arm_smmu_device *smmu,
				u32 sid, u32 ssid, u32 evt_id)
{
	int i;
	__le64 *cde = NULL;

	if (EVTQ_TYPE_WITHOUT_SSID(evt_id)) {
		pr_err("%s,event_id:%u not support\n", __func__, evt_id);
		return;
	}
	cde = arm_smmu_get_cd_from_ste(smmu, sid, ssid);
	if (!cde) {
		pr_err("%s,ssid:%u, get cde null\n", __func__, ssid);
		return;
	}
	pr_err("%s,dump ssid:%u cd context\n", __func__, ssid);
	for (i = 0; i < CTXDESC_CD_DWORDS; i++)
		pr_err("0x%016llx\n", cde[i]);
}

static void arm_smmu_dump_region_info(u64 *addr)
{
	int i = 0;
	const int region_size = 8;
	u64 index;
	u64 step = sizeof(unsigned long long);
	u64 addr_start = max(round_down((u64)addr, PAGE_SIZE),
		(u64)addr - region_size * step);
	u64 addr_end = min(round_up((u64)addr + 1, PAGE_SIZE),
		(u64)addr + region_size * step);

	pr_err("dump bad region addr value:\n");
	for (index = addr_start;
		index < addr_end; i++, index += step)
		pr_err("range[%d], val:0x%llx\n", i, addr[i]);
}

static void arm_smmu_record_region_info(u64 *dst, u64 *src, size_t size)
{
	unsigned int i;
	u64 *src_4k = NULL;

	if (!src || !dst) {
		pr_err("%s, bad input params!", __func__);
		return;
	}
	src_4k = (u64 *)round_down((u64)src, PAGE_SIZE);/*lint !e574*/
	for (i = 0; i < (size / 8); i++)
		dst[i] = src_4k[i];

	arm_smmu_dump_region_info(src);
}

static void arm_smmu_dump_pgd(pgd_t *pgd, phys_addr_t pgd_phy, void *pgd_virt)
{
	pgd_t *pgd_new = NULL;

	pgd_new = (pgd_t *)(pgd_virt + (pgd_phy & (PAGE_SIZE - 1)));
	pr_err("cache-pgd=%016llx, noncache-pgd=%016llx\n", pgd_val(*pgd),
		pgd_val(*pgd_new));

	if (pgd_none(*pgd) || pgd_bad(*pgd) || pgd_none(*pgd_new) ||
		pgd_bad(*pgd_new)) {
		pr_err("pgd is bad!\n");
		return;
	}

	if (pgd_val(*pgd) != pgd_val(*pgd_new))
		pr_err("pgd is not cache coherent\n");
}

static void arm_smmu_dump_pud(pud_t *pud, phys_addr_t pud_phy, void *pud_virt)
{
	pud_t *pud_new = NULL;

	pud_new = (pud_t *)(pud_virt + (pud_phy & (PAGE_SIZE - 1)));
	pr_err("cache-pud=%016llx,noncache-pud=%016llx\n", pud_val(*pud),
		pud_val(*pud_new));

	if (pud_none(*pud) || pud_bad(*pud) || pud_none(*pud_new) ||
		pud_bad(*pud_new)) {
		pr_err("pud is bad!\n");
		return;
	}

	if (pud_val(*pud) != pud_val(*pud_new))
		pr_err("pud is not cache coherent!\n");
}

static void arm_smmu_dump_pmd(pmd_t *pmd, phys_addr_t pmd_phy, void *pmd_virt)
{
	pmd_t *pmd_new = NULL;

	pmd_new = (pmd_t *)(pmd_virt + (pmd_phy & (PAGE_SIZE - 1)));
	pr_err("cache-pmd=%016llx,noncache-pmd=%016llx\n", pmd_val(*pmd),
		pmd_val(*pmd_new));

	if (pmd_none(*pmd) || pmd_bad(*pmd) || pmd_none(*pmd_new) ||
		pmd_bad(*pmd_new)) {
		pr_err("pmd is bad!\n");
		return;
	}

	if (pmd_val(*pmd) != pmd_val(*pmd_new))
		pr_err("pmd is not cache coherent!\n");
}

static void arm_smmu_dump_pte(pte_t *pte, phys_addr_t pte_phy, void *pte_virt)
{
	pte_t *pte_new = NULL;

	pte_new = (pte_t *)(pte_virt + (pte_phy & (PAGE_SIZE - 1)));
	pr_err("cache-pte=%016llx,noncache-pte=%016llx\n", pte_val(*pte),
		pte_val(*pte_new));

	if (pte_val(*pte) != pte_val(*pte_new))
		pr_err("pte is not cache coherent!\n");
}

static void *vmap_page_nc(phys_addr_t phys)
{
	void *va = NULL;
	struct page *page = NULL;
	pgprot_t pgprot;

	/*
	 * turn cached virt-addr to phy-addr,
	 * remap phy-addr to noncached virt-addr.
	 */
	page = phys_to_page(phys);
	pgprot = pgprot_writecombine(PAGE_KERNEL); /* noncached */
	va = vmap(&page, 1, VM_MAP, pgprot);

	return va;
}

static int __get_pgtbl_phys(phys_addr_t *phys, void **phys_virt, void *pgtbl)
{
	phys_addr_t _phys;
	void *_phys_virt = NULL;

	_phys = virt_to_phys(pgtbl);
	_phys_virt = vmap_page_nc(_phys);
	if (!_phys_virt) {
		pr_err("%s vmap_page_nc failed!\n", __func__);
		return -EINVAL;
	}
	*phys = _phys;
	*phys_virt = _phys_virt;
	return 0;
}

static void __put_pgtbl_phys(void *phys_virt)
{
	vunmap(phys_virt);
}

static void __arm_smmu_dump_pgtbl(pgd_t *base_pgd, unsigned long va,
				size_t pgtbl_size, u64 *dst, size_t buf_size)
{
	size_t step;
	pgd_t *pgd = NULL;
	pud_t *pud = NULL;
	pmd_t *pmd = NULL;
	pte_t *pte = NULL;
	unsigned long pfn;
	unsigned long mask = 0xfffffffff000;
	phys_addr_t pgtbl_phys;
	void *pgtbl_virt = NULL;

	if (va >= SZ_1G * 4UL) {
		pr_err("%s bad va:0x%lx\n", __func__, va);
		return;
	}

	do {
		step = SZ_4K;
		pr_err("%s [va:0x%08lx]\n", __func__, va);

		pgd = pgd_offset_raw(base_pgd, va);
		if (__get_pgtbl_phys(&pgtbl_phys, &pgtbl_virt, (void *)pgd))
			break;
		arm_smmu_dump_pgd(pgd, pgtbl_phys, pgtbl_virt);
		__put_pgtbl_phys(pgtbl_virt);

		pfn = __phys_to_pfn(pgd_val(*pgd) & mask);
		if (!pfn_valid(pfn) && dst) {
			arm_smmu_record_region_info(dst, (u64 *)pgd, buf_size);
			return;
		}
		if (pgd_none(*pgd) || pgd_bad(*pgd)) {
			step = SZ_1G;
			continue;
		}

		pud = pud_offset(pgd, va);
		if (__get_pgtbl_phys(&pgtbl_phys, &pgtbl_virt, (void *)pud))
			break;
		arm_smmu_dump_pud(pud, pgtbl_phys, pgtbl_virt);
		__put_pgtbl_phys(pgtbl_virt);

		if (pud_none(*pud) || pud_bad(*pud)) {
			step = SZ_1G;
			continue;
		}

		pmd = pmd_offset(pud, va);
		if (__get_pgtbl_phys(&pgtbl_phys, &pgtbl_virt, (void *)pmd))
			break;
		arm_smmu_dump_pmd(pmd, pgtbl_phys, pgtbl_virt);
		__put_pgtbl_phys(pgtbl_virt);

		pfn = __phys_to_pfn(pmd_val(*pmd) & mask);
		if (!pfn_valid(pfn) && dst) {
			arm_smmu_record_region_info(dst, (u64 *)pmd, buf_size);
			return;
		}
		if (pmd_none(*pmd) || pmd_bad(*pmd)) {
			step = SZ_2M;
			continue;
		}

		pte = pte_offset_map(pmd, va);
		if (__get_pgtbl_phys(&pgtbl_phys, &pgtbl_virt, (void *)pte))
			break;
		arm_smmu_dump_pte(pte, pgtbl_phys, pgtbl_virt);
		__put_pgtbl_phys(pgtbl_virt);

		pte_unmap(pte);
	} while (pgtbl_size >= step && (va += step, pgtbl_size -= step));
}

static pgd_t *arm_smmu_get_pgd(struct arm_smmu_device *smmu, u32 sid,
				u32 ssid, u32 evt_id)
{
	u64 ttbr;
	__le64 *cde = NULL;
	pgd_t *pgd = NULL;
	u64 ttbr_mask = 0xffffffffffff0;

	if (!EVTQ_TYPE_WITH_ADDR(evt_id)) {
		pr_err("%s,event_id:%u not support\n", __func__, evt_id);
		return NULL;
	}
	cde = arm_smmu_get_cd_from_ste(smmu, sid, ssid);
	if (!cde) {
		pr_err("%s,ssid:%u, get cde null\n", __func__, ssid);
		return NULL;
	}
	ttbr = (cde[1] & ttbr_mask);
	if (!ttbr || !pfn_valid(__phys_to_pfn(ttbr))) {
		pr_err("%s,ssid:%u, ttbr:0x%llx fail\n", __func__, ssid, ttbr);
		return NULL;
	}
	pgd = phys_to_virt(ttbr);
	return pgd;
}

static void arm_smmu_dump_pgtbl(struct arm_smmu_device *smmu,
				u64 *evt, int evt_len,
				u64 *dst, size_t buf_size)
{
	u32 evt_id;
	u32 sid, ssid;
	pgd_t *pgd = NULL;

	if (evt_len > EVTQ_ENT_DWORDS)
		return;

	evt_id = (evt[0] >> EVTQ_0_ID_SHIFT) & EVTQ_0_ID_MASK;
	sid = (evt[0] >> PRIQ_0_SID_SHIFT) & PRIQ_0_SID_MASK;
	ssid = (evt[0] >> EVTQ_0_SSID_SHIFT) & EVTQ_0_SSID_MASK;
	if (!evt_id)
		return;

	pgd = arm_smmu_get_pgd(smmu, sid, ssid, evt_id);
	if (!pgd) {
		pr_err("%s, get pgd fail,sid:%u,ssid:%u,va:0x%lx\n",
			__func__, sid, ssid, evt[2]);
		return;
	}
	__arm_smmu_dump_pgtbl(pgd, evt[2], SZ_4K, dst, buf_size);
}

static void arm_smmu_struct_dump(struct arm_smmu_device *smmu,
				u64 *evt, int evt_len)
{
	u32 evt_id;
	u32 sid, ssid;

	if (evt_len > EVTQ_ENT_DWORDS)
		return;

	evt_id = (evt[0] >> EVTQ_0_ID_SHIFT) & EVTQ_0_ID_MASK;
	sid = (evt[0] >> PRIQ_0_SID_SHIFT) & PRIQ_0_SID_MASK;
	ssid = (evt[0] >> EVTQ_0_SSID_SHIFT) & EVTQ_0_SSID_MASK;
	if (!evt_id)
		return;

	arm_smmu_dump_ste(smmu, sid);
	arm_smmu_dump_cd(smmu, sid, ssid, evt_id);
}

static void arm_smmu_event_dump(struct arm_smmu_device *smmu,
				void *dst, unsigned int size)
{
	unsigned int i;
	u64 evt[EVTQ_ENT_DWORDS];
	struct arm_smmu_queue *q = NULL;

	udelay(100); /* avoid multi access eventq */
	q = &smmu->evtq.q;
	if (!queue_remove_raw(q, evt, EVTQ_ENT_DWORDS)) {
		u8 id = (evt[0] >> EVTQ_0_ID_SHIFT) & EVTQ_0_ID_MASK;

		dev_err(smmu->dev, "smmu_event 0x%02x received:\n", id);
		for (i = 0; i < ARRAY_SIZE(evt); ++i)
			dev_err(smmu->dev, "\t0x%016llx\n",
				(unsigned long long)evt[i]);

		arm_smmu_struct_dump(smmu, evt, EVTQ_ENT_DWORDS);
		arm_smmu_dump_pgtbl(smmu, evt,
				EVTQ_ENT_DWORDS, dst, size);
	} else {
		for (i = 0; i < ARRAY_SIZE(evt); ++i)
			dev_err(smmu->dev, "\t0x%016llx\n",
				(unsigned long long)smmu->evt[i]);

		arm_smmu_struct_dump(smmu, smmu->evt, EVTQ_ENT_DWORDS);
		arm_smmu_dump_pgtbl(smmu, smmu->evt,
				EVTQ_ENT_DWORDS, dst, size);
	}
}

#if defined(CONFIG_HISI_DDR_SUBREASON) && defined(CONFIG_HISI_DDR_MONOCEROS)
static bool arm_smmu_modid_check(unsigned int modid)
{
	if (modid != MODID_DMSS_MEDIA_TCU) {
		pr_err("%s modid:0x%x not match!\n", __func__, modid);
		return false;
	}
	return true;
}
#else
static inline bool arm_smmu_modid_check(unsigned int modid)
{
	(void)modid;
	return false;
}
#endif

static void arm_smmu_rdr_event_dump(unsigned int modid,
				void *addr, unsigned int size)
{
	if (arm_smmu_modid_check(modid) == false) {
		pr_err("%s modid:0x%x!\n", __func__, modid);
		return;
	}
	pr_err("%s modid:0x%x, status:%u, cnt:0x%lx!\n",
		__func__, modid, smmu_media1->status, smmu_media1->cnt);

	if (smmu_media1->status == ARM_SMMU_ENABLE)
		arm_smmu_event_dump(smmu_media1, addr, size);
}

static int arm_smmu_rdr_dump(void *dump_addr, unsigned int size)
{
	struct rdr_exception_info_s *info = NULL;
	unsigned int bufsize = min(size, (u32)SZ_4K);

	pr_err("into %s, addr:%pK, size:0x%x\n", __func__, dump_addr, size);
	info = rdr_get_exce_info();
	if (!info) {
		pr_err("%s, rdr_get_exce_info failed!\n", __func__);
		return 0;
	}
	arm_smmu_rdr_event_dump(info->e_modid, dump_addr, bufsize);

	return 1;
}

static int arm_smmu_rdr_init(void)
{
	pr_err("into %s\n", __func__);
	(void)register_module_dump_mem_func(
			arm_smmu_rdr_dump, "SMMU", MODU_SMMU);
	return 0;
}

/* IRQ and event handlers */
static irqreturn_t arm_smmu_evtq_thread(int irq, void *dev)
{
	int i;
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->evtq.q;
	u64 evt[EVTQ_ENT_DWORDS];
	u64 pre_evt = 0;

	do {
		while (!queue_remove_raw(q, evt, EVTQ_ENT_DWORDS)) {
			u8 id = (evt[0] >> EVTQ_0_ID_SHIFT) & EVTQ_0_ID_MASK;

			/* The continuous same EventQ is output only once. */
			if (pre_evt == evt[0])
				continue;
			pre_evt = evt[0];

			dev_info(smmu->dev, "event 0x%02x received:\n", id);
			for (i = 0; i < ARRAY_SIZE(evt); ++i)
				dev_err(smmu->dev, "\t0x%016llx\n",
					 (unsigned long long)evt[i]);
			/* save last event */
			for (i = 0; i < EVTQ_ENT_DWORDS; i++)
				smmu->evt[i] = evt[i];

			arm_smmu_struct_dump(smmu, evt, EVTQ_ENT_DWORDS);

			/* print pgd when evtq has happened */
			arm_smmu_dump_pgtbl(smmu, evt, EVTQ_ENT_DWORDS, NULL, 0);

			if (arm_smmu_evt_notify(smmu, evt, EVTQ_ENT_DWORDS))
				dev_info(smmu->dev, "evt %u notify err\n", id);
		}

		/*
		 * Not much we can do on overflow, so scream and pretend we're
		 * trying harder.
		 */
		if (queue_sync_prod(q) == -EOVERFLOW)
			dev_info(smmu->dev,
				"EVTQ overflow detected -- events lost\n");
	} while (!queue_empty(q));

	/* Sync our overflow flag, as we believe we're up to speed */
	q->cons = Q_OVF(q, q->prod) | Q_WRP(q, q->cons) | Q_IDX(q, q->cons);

	return IRQ_HANDLED;
}

static void arm_smmu_handle_ppr(struct arm_smmu_device *smmu, u64 *evt,
				unsigned long evtlen)
{
	u32 sid, ssid;
	u16 grpid;
	bool ssv, last;

	sid = (evt[0] >> PRIQ_0_SID_SHIFT) & PRIQ_0_SID_MASK;
	ssv = evt[0] & PRIQ_0_SSID_V;
	ssid = ssv ? ((evt[0] >> PRIQ_0_SSID_SHIFT) & PRIQ_0_SSID_MASK) : 0;
	last = evt[0] & PRIQ_0_PRG_LAST;
	grpid = (evt[1] >> PRIQ_1_PRG_IDX_SHIFT) & PRIQ_1_PRG_IDX_MASK;

	dev_info(smmu->dev, "unexpected PRI request received:\n");
	dev_info(smmu->dev,
		 "\tsid 0x%08x.0x%05x: [%u%s] %sprivileged %s%s%s access at iova 0x%016llx\n",
		 sid, ssid, grpid, last ? "L" : "",
		 evt[0] & PRIQ_0_PERM_PRIV ? "" : "un",
		 evt[0] & PRIQ_0_PERM_READ ? "R" : "",
		 evt[0] & PRIQ_0_PERM_WRITE ? "W" : "",
		 evt[0] & PRIQ_0_PERM_EXEC ? "X" : "",
		 evt[1] & PRIQ_1_ADDR_MASK << PRIQ_1_ADDR_SHIFT);

	if (last) {
		struct arm_smmu_cmdq_ent cmd = {
			.opcode			= CMDQ_OP_PRI_RESP,
			.substream_valid	= ssv,
			.pri			= {
				.sid	= sid,
				.ssid	= ssid,
				.grpid	= grpid,
				.resp	= PRI_RESP_DENY,
			},
		};

		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	}
}

static irqreturn_t arm_smmu_priq_thread(int irq, void *dev)
{
	struct arm_smmu_device *smmu = dev;
	struct arm_smmu_queue *q = &smmu->priq.q;
	u64 evt[PRIQ_ENT_DWORDS];

	do {
		while (!queue_remove_raw(q, evt, EVTQ_ENT_DWORDS))
			arm_smmu_handle_ppr(smmu, evt, EVTQ_ENT_DWORDS);

		if (queue_sync_prod(q) == -EOVERFLOW)
			dev_err(smmu->dev, "PRIQ overflow detected -- requests lost\n");
	} while (!queue_empty(q));

	/* Sync our overflow flag, as we believe we're up to speed */
	q->cons = Q_OVF(q, q->prod) | Q_WRP(q, q->cons) | Q_IDX(q, q->cons);
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_cmdq_sync_handler(int irq, void *dev)
{
	/* We don't actually use CMD_SYNC interrupts for anything */
	return IRQ_HANDLED;
}

static int arm_smmu_device_disable(struct arm_smmu_device *smmu);

static irqreturn_t arm_smmu_gerror_handler(int irq, void *dev)
{
	u32 gerror, gerrorn, active;
	struct arm_smmu_device *smmu = dev;

	gerror = readl_relaxed(smmu->base + ARM_SMMU_GERROR);
	gerrorn = readl_relaxed(smmu->base + ARM_SMMU_GERRORN);

	active = gerror ^ gerrorn;
	if (!(active & GERROR_ERR_MASK))
		return IRQ_NONE; /* No errors pending */

	dev_warn(smmu->dev,
		 "unexpected global error reported (0x%08x), this could be serious\n",
		 active);

	if (active & GERROR_SFM_ERR) {
		dev_err(smmu->dev, "device has entered Service Failure Mode!\n");
		arm_smmu_device_disable(smmu);
	}

	if (active & GERROR_MSI_GERROR_ABT_ERR)
		dev_warn(smmu->dev, "GERROR MSI write aborted\n");

	if (active & GERROR_MSI_PRIQ_ABT_ERR)
		dev_warn(smmu->dev, "PRIQ MSI write aborted\n");

	if (active & GERROR_MSI_EVTQ_ABT_ERR)
		dev_warn(smmu->dev, "EVTQ MSI write aborted\n");

	if (active & GERROR_MSI_CMDQ_ABT_ERR) {
		dev_warn(smmu->dev, "CMDQ MSI write aborted\n");
		arm_smmu_cmdq_sync_handler(irq, smmu->dev);
	}

	if (active & GERROR_PRIQ_ABT_ERR)
		dev_err(smmu->dev, "PRIQ write aborted -- events may have been lost\n");

	if (active & GERROR_EVTQ_ABT_ERR)
		dev_err(smmu->dev, "EVTQ write aborted -- events may have been lost\n");

	if (active & GERROR_CMDQ_ERR)
		arm_smmu_cmdq_skip_err(smmu);

	writel(gerror, smmu->base + ARM_SMMU_GERRORN);
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_combined_irq_thread(int irq, void *dev)
{
	unsigned long flags;
	struct arm_smmu_device *smmu = dev;

	spin_lock_irqsave(&smmu->tcu_lock, flags);
	if (smmu->status != ARM_SMMU_ENABLE) {
		dev_info(smmu->dev, "%s, smmu status %d is not enable\n",
			__func__, smmu->status);
		goto out_unlock;
	}
	arm_smmu_evtq_thread(irq, dev);
	if (smmu->features & ARM_SMMU_FEAT_PRI)
		arm_smmu_priq_thread(irq, dev);

out_unlock:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
	return IRQ_HANDLED;
}

static irqreturn_t arm_smmu_combined_irq_handler(int irq, void *dev)
{
#ifdef HISI_SMMUV3_SUPPORT
	u32 reg = (TCU_EVENT_Q_IRQ_CLR | TCU_CMD_SYNC_IRQ_CLR
		| TCU_GERROR_IRQ_CLR | TCU_EVENTTO_CLR);
	struct arm_smmu_device *smmu = dev;

	spin_lock(&smmu->tcu_lock);
	if (smmu->status != ARM_SMMU_ENABLE) {
		dev_info(smmu->dev, "%s, smmu status %d is not enable\n",
			__func__, smmu->status);
		spin_unlock(&smmu->tcu_lock);
		return IRQ_HANDLED;
	}
	writel_relaxed(reg, smmu->base + ARM_SMMU_IRPT_CLR_NS);
#endif
	arm_smmu_gerror_handler(irq, dev);
	arm_smmu_cmdq_sync_handler(irq, dev);
	spin_unlock(&smmu->tcu_lock);

	return IRQ_WAKE_THREAD;
}

/* IO_PGTABLE API */
static void __arm_smmu_tlb_sync(struct arm_smmu_device *smmu)
{
	struct arm_smmu_cmdq_ent cmd;

	cmd.opcode = CMDQ_OP_CMD_SYNC;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
}

static void arm_smmu_tlb_sync(void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;

	__arm_smmu_tlb_sync(smmu_domain->smmu);
}

static void arm_smmu_tlb_inv_context(void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_cmdq_ent cmd;

	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		cmd.opcode	= CMDQ_OP_TLBI_NH_ASID;
		cmd.tlbi.asid	= smmu_domain->s1_cfg.cd.asid;
		cmd.tlbi.vmid	= smmu_domain->s1_cfg.vmid;
	} else {
		cmd.opcode	= CMDQ_OP_TLBI_S12_VMALL;
		cmd.tlbi.vmid	= smmu_domain->s2_cfg.vmid;
	}

	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
	__arm_smmu_tlb_sync(smmu);
}

static void arm_smmu_tlb_inv_range_nosync(unsigned long iova,
				size_t size, size_t granule, bool leaf,
				void *cookie)
{
	struct arm_smmu_domain *smmu_domain = cookie;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_cmdq_ent cmd = {
		.tlbi = {
			.leaf	= leaf,
			.addr	= iova,
		},
	};

	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		cmd.opcode	= CMDQ_OP_TLBI_NH_VA;
		cmd.tlbi.asid	= smmu_domain->s1_cfg.cd.asid;
		cmd.tlbi.vmid	= smmu_domain->s1_cfg.vmid;
	} else {
		cmd.opcode	= CMDQ_OP_TLBI_S2_IPA;
		cmd.tlbi.vmid	= smmu_domain->s2_cfg.vmid;
	}

	do {
		arm_smmu_cmdq_issue_cmd(smmu, &cmd);
		cmd.tlbi.addr += granule;
	} while (size -= granule);
}

static const struct iommu_gather_ops arm_smmu_gather_ops = {
	.tlb_flush_all	= arm_smmu_tlb_inv_context,
	.tlb_add_flush	= arm_smmu_tlb_inv_range_nosync,
	.tlb_sync	= arm_smmu_tlb_sync,
};

static const struct iommu_gather_ops arm_smmu_tlb_nosync_ops = {
	.tlb_flush_all	= arm_smmu_tlb_inv_context,
};

static int arm_smmu_dev_flush_tlb(struct iommu_domain *domain,
				unsigned int ssid)
{
	struct arm_smmu_domain *smmu_domain = NULL;
	struct arm_smmu_device *smmu = NULL;

	smmu_domain = to_smmu_domain(domain);
	if (smmu_domain->tlb_flush != DEV_FLUSH) {
		pr_err("%s, tlb_flush %u is not DEV_FLUSH!\n",
			__func__, smmu_domain->tlb_flush);
		return -EPERM;
	}

	smmu = smmu_domain->smmu;
	if (!smmu) {
		pr_err("%s, smmu is null\n", __func__);
		return -ENODEV;
	}

	if (ssid >=  (1UL << smmu->ssid_bits)) {
		dev_err(smmu->dev, "%s ssid %u error, ssid_bits (%u)\n",
			__func__, ssid, smmu->ssid_bits);
		return -EINVAL;
	}

	smmu_domain->s1_cfg.cd.asid = ssid;
	arm_smmu_tlb_inv_context(smmu_domain);
	return 0;
}

static void arm_smmu_flush_tlb(struct device *dev, struct iommu_domain *domain)
{
	u16 ssid;
	struct arm_smmu_domain *smmu_domain = NULL;

	smmu_domain = to_smmu_domain(domain);
	if (smmu_domain->tlb_flush != SSID_FLUSH)
		return;

	ssid = arm_smmu_get_ssid(dev->iommu_fwspec);
	smmu_domain->s1_cfg.cd.asid = ssid;
	arm_smmu_tlb_inv_context(smmu_domain);
}
/* IOMMU API */
static bool arm_smmu_capable(enum iommu_cap cap)
{
	switch (cap) {
	case IOMMU_CAP_CACHE_COHERENCY:
		return true;
	case IOMMU_CAP_NOEXEC:
		return true;
	default:
		return false;
	}
}

LIST_HEAD(smmuv3_domain_list);
static struct arm_v3_domain *to_hisi_v3_domain(struct arm_smmu_domain *dom)
{
	return container_of(dom, struct arm_v3_domain, smmu_domain);
}

static struct iommu_domain *arm_smmu_domain_alloc(unsigned int type)
{
	struct arm_v3_domain *mm_domain = NULL;

	if (type != IOMMU_DOMAIN_UNMANAGED &&
	    type != IOMMU_DOMAIN_DMA &&
	    type != IOMMU_DOMAIN_IDENTITY)
		return NULL;

	/*
	 * Allocate the domain and initialise some of its data structures.
	 * We can't really do anything meaningful until we've added a
	 * master.
	 */
	mm_domain = kzalloc(sizeof(*mm_domain), GFP_KERNEL);
	if (!mm_domain)
		return NULL;

	mutex_init(&mm_domain->smmu_domain.init_mutex);
	/* dmabuf is not allocated through the svm */
	if (type != IOMMU_DOMAIN_UNMANAGED)
		list_add_tail(&mm_domain->list, &smmuv3_domain_list);
	return &mm_domain->smmu_domain.domain;
}

static int arm_smmu_bitmap_alloc(unsigned long *map, int span)
{
	int idx;
	int size = 1 << span;

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

static bool is_svm_bind_task_domain(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if ((smmu_domain->type == ARM_SMMU_DOMAIN_SVM) &&
		(domain->type == IOMMU_DOMAIN_UNMANAGED)) {
		return true;
	}
	return false;
}

static void arm_smmu_domain_free(struct iommu_domain *domain)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_v3_domain *mm_domain = to_hisi_v3_domain(smmu_domain);
	size_t cd_size;

	iommu_put_dma_cookie(domain);
	if (smmu_domain->pgtbl_ops)
		free_io_pgtable_ops(smmu_domain->pgtbl_ops);

	/* Free the CD and ASID, if we allocated them */
	if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;

		if (is_svm_bind_task_domain(domain))
			goto domain_free;

		if (cfg->cdptr) {
			cd_size = (1UL << smmu->ssid_bits) *
				(CTXDESC_CD_DWORDS << DWORD_BYTES_NUM);
			dmam_free_coherent(smmu_domain->smmu->dev,
				cd_size, cfg->cdptr, cfg->cdptr_dma);
			if (smmu_domain->tlb_flush == ASID_FLUSH)
				arm_smmu_bitmap_free(smmu->asid_map,
					cfg->cd.asid);
		}
	} else {
		struct arm_smmu_s2_cfg *cfg = &smmu_domain->s2_cfg;

		if (cfg->vmid)
			arm_smmu_bitmap_free(smmu->vmid_map, cfg->vmid);
	}

domain_free:
	if ((mm_domain->list.prev) && (mm_domain->list.next))
		list_del(&mm_domain->list);
	kfree(mm_domain);
}

static int arm_smmu_domain_finalise_s1(struct arm_smmu_domain *smmu_domain,
				struct io_pgtable_cfg *pgtbl_cfg)
{
	int ret;
	int asid;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;

	asid = cfg->cd.ssid;
	if (smmu_domain->tlb_flush == ASID_FLUSH) {
		asid = arm_smmu_bitmap_alloc(smmu->asid_map, smmu->asid_bits);
		if (asid < 0)
			return asid;
		pr_err("into %s, asid:%d\n", __func__, asid);
	}
	cfg->cdptr = dmam_alloc_coherent(smmu->dev,
			((1UL << smmu->ssid_bits) *
			(CTXDESC_CD_DWORDS << DWORD_BYTES_NUM)),
			&cfg->cdptr_dma, GFP_KERNEL | __GFP_ZERO);
	if (!cfg->cdptr) {
		dev_warn(smmu->dev, "failed to allocate context descriptor\n");
		ret = -ENOMEM;
		goto out_free_asid;
	}

	cfg->cd.asid	= (u16)asid;
	cfg->cd.ttbr	= pgtbl_cfg->arm_lpae_s1_cfg.ttbr[0];
	cfg->cd.tcr	= pgtbl_cfg->arm_lpae_s1_cfg.tcr;
	cfg->cd.mair	= pgtbl_cfg->arm_lpae_s1_cfg.mair[0];
	return 0;

out_free_asid:
	if (smmu_domain->tlb_flush == ASID_FLUSH)
		arm_smmu_bitmap_free(smmu->asid_map, asid);
	return ret;
}

static struct arm_smmu_domain *arm_smmu_get_dev_smmu_domain(struct device *dev)
{
	struct arm_smmu_master_data *master = NULL;

	if (!dev->iommu_fwspec) {
		dev_err(dev, "%s, iommu_fwspec is null!!\n", __func__);
		return NULL;
	}

	master = dev->iommu_fwspec->iommu_priv;
	if (!master) {
		dev_err(dev, "%s, master is null\n", __func__);
		return NULL;
	}

	return master->smmu_domain;
}

static int arm_smmu_domain_set_pgd(struct arm_smmu_domain *smmu_domain,
				struct task_struct *task)
{
	unsigned long asid;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct mm_struct *mm = NULL;

	mm = get_task_mm(task);
	if (!mm) {
		pr_err("%s get mm is null\n", __func__);
		return -EINVAL;
	}
	asid = ASID(mm);
	mmput(mm);

	cfg->cd.asid = (u16)asid;
	cfg->cd.ttbr = virt_to_phys(mm->pgd) |
		(asid << CTXDESC_CD_0_ASID_SHIFT);

	pr_info("%s, pid: %d, process name: %s,pgd:%pK\n",
		       __func__, task->pid, task->comm, mm->pgd);

	pr_info("%s, asid:0x%lx,ttbr:0x%llx\n", __func__, asid, cfg->cd.ttbr);
	return 0;
}

static int arm_smmu_svm_domain_finalise_s1(struct arm_smmu_domain *smmu_domain,
				struct io_pgtable_cfg *pgtbl_cfg)
{
#ifdef CONFIG_MM_TCU
	int asid;
#endif
	int ssid;
	struct arm_smmu_s1_cfg *cfg = &smmu_domain->s1_cfg;
	struct arm_smmu_domain *dev_smmu_domain = NULL;

	dev_smmu_domain = arm_smmu_get_dev_smmu_domain(smmu_domain->dev);
	if (!dev_smmu_domain) {
		dev_err(smmu_domain->dev, "%s, dev_smmu_domain is null\n",
			__func__);
		return -EINVAL;
	}

	ssid = arm_smmu_bitmap_alloc(dev_smmu_domain->ssid_map,
				     smmu_domain->smmu->ssid_bits);
	if (ssid < 0) {
		dev_err(smmu_domain->dev, "%s, arm_smmu_bitmap_alloc ssid err!!\n", __func__);
		return -EINVAL;
	}

#ifdef CONFIG_MM_TCU
	asid = arm_smmu_bitmap_alloc(smmu_domain->smmu->asid_map,
				     smmu_domain->smmu->asid_bits);
	if (asid < 0) {
		dev_err(smmu_domain->dev, "%s, arm_smmu_bitmap_alloc asid err!!\n", __func__);
		arm_smmu_bitmap_free(dev_smmu_domain->ssid_map, ssid);
		return -EINVAL;
	}

	cfg->cd.asid = asid;

	/* clear the asid which is get from ASID(mm), and write new 8-bit asid */
	cfg->cd.ttbr &= CD_TTBR_DATA_MASK;
	cfg->cd.ttbr |= (u64)(unsigned int)asid << CTXDESC_CD_0_ASID_SHIFT;
#endif

	cfg->cdptr = dev_smmu_domain->s1_cfg.cdptr;
	cfg->cdptr_dma = dev_smmu_domain->s1_cfg.cdptr_dma;

	cfg->cd.ssid = (u16)ssid;
	cfg->cd.tcr = pgtbl_cfg->arm_lpae_s1_cfg.tcr;
	cfg->cd.mair = pgtbl_cfg->arm_lpae_s1_cfg.mair[0];
	cfg->cd.ssid_nums = 1;
	cfg->tlb_flush = ASID_FLUSH;

	dev_info(smmu_domain->dev, "%s,alloc ssid:%d, asid:%u,ttbr:0x%llx\n",
		       __func__, ssid, cfg->cd.asid, cfg->cd.ttbr);
	/* only build the cd desc */
	arm_smmu_write_ctx_desc(smmu_domain->smmu, cfg);

	arm_smmu_sync_cd(smmu_domain);
	return 0;
}

int arm_smmu_svm_get_ssid(struct iommu_domain *domain,
				u16 *ssid, u64 *ttbr, u64 *tcr)
{
	struct arm_smmu_domain *smmu_domain = NULL;

	smmu_domain = to_smmu_domain(domain);
	if (!smmu_domain)
		return -EINVAL;

	*ssid = smmu_domain->s1_cfg.cd.ssid;
	*ttbr = smmu_domain->s1_cfg.cd.ttbr;
	*tcr = smmu_domain->s1_cfg.cd.tcr;
	return 0;
}

static int arm_smmu_domain_finalise_s2(struct arm_smmu_domain *smmu_domain,
				struct io_pgtable_cfg *pgtbl_cfg)
{
	int vmid;
	struct arm_smmu_device *smmu = smmu_domain->smmu;
	struct arm_smmu_s2_cfg *cfg = &smmu_domain->s2_cfg;

	vmid = arm_smmu_bitmap_alloc(smmu->vmid_map, smmu->vmid_bits);
	if (vmid < 0)
		return vmid;

	cfg->vmid	= (u16)vmid;
	cfg->vttbr	= pgtbl_cfg->arm_lpae_s2_cfg.vttbr;
	cfg->vtcr	= pgtbl_cfg->arm_lpae_s2_cfg.vtcr;
	return 0;
}

static int arm_smmu_get_pgtbl_cfg(struct arm_smmu_domain *smmu_domain,
				struct io_pgtable_cfg *pgtbl_cfg)
{
	unsigned long ias, oas;
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	switch (smmu_domain->stage) {
	case ARM_SMMU_DOMAIN_S1:
		ias = VA_BITS;
		oas = smmu->ias;
		break;
	case ARM_SMMU_DOMAIN_NESTED:
	case ARM_SMMU_DOMAIN_S2:
		ias = smmu->ias;
		oas = smmu->oas;
		break;
	default:
		return -EINVAL;
	}

	*pgtbl_cfg = (struct io_pgtable_cfg) {
		.pgsize_bitmap	= smmu->pgsize_bitmap,
		.ias		= ias,
		.oas		= oas,
		.tlb		= &arm_smmu_gather_ops,
		.iommu_dev	= smmu->dev,
	};

	if (smmu_domain->tlb_flush != ASID_FLUSH)
		pgtbl_cfg->tlb = &arm_smmu_tlb_nosync_ops;
	return 0;
}

static int arm_smmu_get_finalise_stage_ops(struct iommu_domain *domain,
				struct finalise_stage_ops *ops)
{
	enum io_pgtable_fmt fmt;
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	switch (smmu_domain->stage) {
	case ARM_SMMU_DOMAIN_S1:
		fmt = ARM_64_LPAE_S1;
		/* bind task */
		if (is_svm_bind_task_domain(domain))
			ops->finalise_stage_fn =
				arm_smmu_svm_domain_finalise_s1;
		else
			ops->finalise_stage_fn = arm_smmu_domain_finalise_s1;
		break;
	case ARM_SMMU_DOMAIN_NESTED:
	case ARM_SMMU_DOMAIN_S2:
		fmt = ARM_64_LPAE_S2;
		ops->finalise_stage_fn = arm_smmu_domain_finalise_s2;
		break;
	default:
		return -EINVAL;
	}

	return fmt;
}

static int arm_smmu_set_tcr_ips(unsigned int oas, u64 *reg)
{
	switch (oas) {
	case ARM_SMMU_ADDR_SIZE_32:
		*reg |= (ARM_LPAE_TCR_PS_32_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case ARM_SMMU_ADDR_SIZE_36:
		*reg |= (ARM_LPAE_TCR_PS_36_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case ARM_SMMU_ADDR_SIZE_40:
		*reg |= (ARM_LPAE_TCR_PS_40_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case ARM_SMMU_ADDR_SIZE_42:
		*reg |= (ARM_LPAE_TCR_PS_42_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case ARM_SMMU_ADDR_SIZE_44:
		*reg |= (ARM_LPAE_TCR_PS_44_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	case ARM_SMMU_ADDR_SIZE_48:
		*reg |= (ARM_LPAE_TCR_PS_48_BIT << ARM_LPAE_TCR_IPS_SHIFT);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int arm_smmu_svm_alloc_pgtable_s1(struct io_pgtable_cfg *cfg)
{
	u64 reg = 0;

	if (arm_smmu_set_tcr_ips(cfg->oas, &reg)) {
		pr_err("%s, arm_smmu_set_tcr_ips err! oas:%u\n",
			__func__, cfg->oas);
		goto out;
	}

	reg |= (64ULL - cfg->ias) << ARM_LPAE_TCR_T0SZ_SHIFT;

	/* Disable speculative walks through TTBR1 */
	reg |= ARM_LPAE_TCR_EPD1;
	cfg->arm_lpae_s1_cfg.tcr = reg;

	/* MAIRs */
	reg = (ARM_LPAE_MAIR_ATTR_NC
	       << arm_lpae_mair_attr_shift(ARM_LPAE_MAIR_ATTR_IDX_NC)) |
	      (ARM_LPAE_MAIR_ATTR_WBRWA
	       << arm_lpae_mair_attr_shift(ARM_LPAE_MAIR_ATTR_IDX_CACHE)) |
	      (ARM_LPAE_MAIR_ATTR_DEVICE
	       << arm_lpae_mair_attr_shift(ARM_LPAE_MAIR_ATTR_IDX_DEV));

	cfg->arm_lpae_s1_cfg.mair[0] = reg;
	cfg->arm_lpae_s1_cfg.mair[1] = 0;

	/* Ensure the empty pgd is visible before any actual TTBR write */
	wmb();

	cfg->arm_lpae_s1_cfg.ttbr[1] = 0;

	return 0;
out:
	return -EINVAL;
}

static int arm_smmu_domain_finalise(struct iommu_domain *domain)
{
	int ret;
	enum io_pgtable_fmt fmt;
	struct io_pgtable_cfg pgtbl_cfg;
	struct io_pgtable_ops *pgtbl_ops = NULL;
	struct finalise_stage_ops finalise_stage_ops;
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_device *smmu = smmu_domain->smmu;

	if (domain->type == IOMMU_DOMAIN_IDENTITY) {
		smmu_domain->stage = ARM_SMMU_DOMAIN_BYPASS;
		return 0;
	}

	/* Restrict the stage to what we can actually support */
	if (!(smmu->features & ARM_SMMU_FEAT_TRANS_S1))
		smmu_domain->stage = ARM_SMMU_DOMAIN_S2;
	if (!(smmu->features & ARM_SMMU_FEAT_TRANS_S2))
		smmu_domain->stage = ARM_SMMU_DOMAIN_S1;

	fmt = arm_smmu_get_finalise_stage_ops(domain, &finalise_stage_ops);
	if (fmt < 0)
		return fmt;

	if (arm_smmu_get_pgtbl_cfg(smmu_domain, &pgtbl_cfg))
		return -EINVAL;

	if (smmu->features & ARM_SMMU_FEAT_COHERENCY)
		pgtbl_cfg.quirks = IO_PGTABLE_QUIRK_NO_DMA;

	/* bind task */
	if (is_svm_bind_task_domain(domain)) {
		ret = arm_smmu_svm_alloc_pgtable_s1(&pgtbl_cfg);
		if (ret)
			return -ENOMEM;
	} else {
		pgtbl_ops = alloc_io_pgtable_ops(fmt, &pgtbl_cfg, smmu_domain);
		if (!pgtbl_ops)
			return -ENOMEM;
	}
	domain->pgsize_bitmap = pgtbl_cfg.pgsize_bitmap;
	domain->geometry.aperture_end = (1UL << pgtbl_cfg.ias) - 1;
	domain->geometry.force_aperture = true;

	ret = finalise_stage_ops.finalise_stage_fn(smmu_domain, &pgtbl_cfg);
	if (ret < 0) {
		free_io_pgtable_ops(pgtbl_ops);
		return ret;
	}

	smmu_domain->pgtbl_ops = pgtbl_ops;
	return 0;
}

static __le64 *arm_smmu_get_step_for_sid(struct arm_smmu_device *smmu, u32 sid)
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
		step = &cfg->strtab[sid * STRTAB_STE_DWORDS];
	}

	return step;
}

static void arm_smmu_install_ste_for_dev(struct iommu_fwspec *fwspec)
{
	int i, j;
	struct arm_smmu_master_data *master = fwspec->iommu_priv;
	struct arm_smmu_device *smmu = master->smmu;

	for (i = 0; i < fwspec->num_ids; ++i) {
		u32 sid = fwspec->ids[i];
		__le64 *step = arm_smmu_get_step_for_sid(smmu, sid);

		/* Bridged PCI devices may end up with duplicated IDs */
		for (j = 0; j < i; j++)
			if (fwspec->ids[j] == sid)
				break;
		if (j < i)
			continue;
		pr_err("%s sid %u\n", __func__, sid);
		arm_smmu_write_strtab_ent(smmu, sid, step, &master->ste);
		break;
	}
#ifdef HISI_SMMUV3_SUPPORT
	if (fwspec->num_ids) {
		struct arm_smmu_strtab_ent ste = { .assigned = false };
		__le64 *step_new =
			arm_smmu_get_step_for_sid(smmu, STREAM_ID_BYPASS);

		arm_smmu_write_strtab_ent(smmu,
			STREAM_ID_BYPASS, step_new, &ste);
	}
#endif
}

static struct arm_smmu_device *svm_smmu;

#ifdef CONFIG_MM_TCU
static struct hisi_svm *get_svm_by_mm(struct mm_struct *mm)
{
	struct list_head *pos = NULL;
	struct hisi_svm *tmp = NULL;

	list_for_each(pos, &mm_svm_list) {
		tmp = list_entry(pos, struct hisi_svm, list);
		if (tmp->mm == mm) {
			return tmp;
		}
	}
	return NULL;
}

static u16 hisi_get_mm_domain_asid(struct mm_struct *mm)
{
	struct hisi_svm *svm = NULL;
	struct arm_smmu_domain *smmu_domain = NULL;

	svm = get_svm_by_mm(mm);
	if (!svm) {
		pr_err("%s: invalid mm_struct, no svm found!\n", __func__);
		return (u16)((1UL << svm_smmu->asid_bits) - 1);
	}

	smmu_domain = to_smmu_domain(svm->dom);
	return smmu_domain->s1_cfg.cd.asid;
}
#endif

void arm_smmu_svm_tlb_inv_context(struct mm_struct *mm)
{
	struct arm_smmu_cmdq_ent cmd;

	if (!svm_smmu) {
		pr_err("%s: svm smmu is null\n", __func__);
		return;
	}

	if (svm_smmu->status != ARM_SMMU_ENABLE)
		return;

	arm_smmu_invalid_tcu_cache(svm_smmu);

	cmd.opcode	= CMDQ_OP_TLBI_NH_ASID;
#ifdef CONFIG_MM_TCU
	cmd.tlbi.asid	= hisi_get_mm_domain_asid(mm);
#else
	cmd.tlbi.asid	= ASID(mm);
#endif
	cmd.tlbi.vmid	= ARM_SMMU_SVM_S1CFG_VMID;

	arm_smmu_cmdq_issue_cmd(svm_smmu, &cmd);
	__arm_smmu_tlb_sync(svm_smmu);
}

static void arm_smmu_sync_cd(struct arm_smmu_domain *dom)
{
	u32 sid;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_cmdq_ent cmd;

	smmu = dom->smmu;
	if (smmu->status != ARM_SMMU_ENABLE) {
		dev_err(dom->dev, "%s: smmu %s status %d invalid\n",
			__func__,  dev_name(smmu->dev), smmu->status);
		return;
	}

	arm_smmu_invalid_tcu_cache(smmu);

	sid = arm_smmu_get_sid(dom->dev->iommu_fwspec);
	if (!arm_smmu_sid_in_range(smmu, sid)) {
		dev_err(dom->dev, "%s: %u out of %s sid range\n",
			__func__, sid, dev_name(dom->smmu->dev));
		return;
	}

	cmd.opcode = CMDQ_OP_CFGI_CD;
	cmd.cfgi.ssid = dom->s1_cfg.cd.ssid;
	cmd.cfgi.sid = sid;
	cmd.cfgi.leaf = 1;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);

	cmd.opcode = CMDQ_OP_CMD_SYNC;
	arm_smmu_cmdq_issue_cmd(smmu, &cmd);
}

static void arm_smmu_svm_detach_dev(struct iommu_domain *domain,
				struct device *dev)
{
	u16 cdptr_offset;
	struct arm_smmu_domain *smmu_domain = NULL;
	struct arm_smmu_domain *dev_smmu_domain = NULL;
	struct arm_smmu_s1_cfg *cfg = NULL;

	if (unlikely(!domain)) {
		dev_err(dev, "domain is null %s\n", __func__);
		return;
	}

	smmu_domain = to_smmu_domain(domain);
	if (unlikely(!smmu_domain)) {
		dev_err(dev, "smmu_domain is null %s\n", __func__);
		return;
	}

	dev_smmu_domain = arm_smmu_get_dev_smmu_domain(smmu_domain->dev);
	if (!dev_smmu_domain) {
		pr_err("%s, dev_smmu_domain is null\n", __func__);
		return;
	}

	cfg = &smmu_domain->s1_cfg;
	if (cfg->cd.ssid >=  (1UL << smmu_domain->smmu->ssid_bits)) {
		dev_err(dev, "%s ssid %u error! ssid %u, smmu(%s)\n", __func__,
			cfg->cd.ssid, smmu_domain->smmu->ssid_bits,
			dev_name(smmu_domain->smmu->dev));
		return;
	}
	arm_smmu_bitmap_free(dev_smmu_domain->ssid_map, cfg->cd.ssid);
#ifdef CONFIG_MM_TCU
	arm_smmu_bitmap_free(smmu_domain->smmu->asid_map, cfg->cd.asid);
#endif
	/* clear cd decs */
	cdptr_offset = cfg->cd.ssid * CTXDESC_CD_DWORDS;
	memset(&cfg->cdptr[cdptr_offset], 0x0, CTXDESC_CD_DWORDS <<
		DWORD_BYTES_NUM); /* unsafe_function_ignore: memset */

	/* Invalidate the ctx desc table */
	arm_smmu_sync_cd(smmu_domain);
}

static void arm_smmu_detach_dev(struct device *dev)
{
	struct arm_smmu_master_data *master = dev->iommu_fwspec->iommu_priv;

	master->ste.assigned = false;
	arm_smmu_install_ste_for_dev(dev->iommu_fwspec);
}

static u32 of_get_ssid_nums(struct device_node *np)
{
	int ret;
	u32 ssid_nums = 0;

	if (!np)
		return 1;

	ret = of_property_read_u32(np, "ssid_nums", &ssid_nums);
	if (ret)
		return 1;

	pr_err("%s ssid nums %u\n", __func__, ssid_nums);
	return ssid_nums;
}

static u16 arm_smmu_get_s1_cfg_vmid(struct device *dev,
				struct arm_smmu_domain *smmu_domain)
{
	if (smmu_domain->type == ARM_SMMU_DOMAIN_SVM)
		return ARM_SMMU_SVM_S1CFG_VMID;

	return (u16)arm_smmu_get_sid(dev->iommu_fwspec);
}

static void arm_smmu_set_ste(struct arm_smmu_domain *smmu_domain,
				struct device *dev)
{
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_master_data *master = NULL;
	struct arm_smmu_strtab_ent *ste = NULL;

	master = dev->iommu_fwspec->iommu_priv;
	master->smmu_domain = smmu_domain;
	smmu = master->smmu;
	ste = &master->ste;
	ste->assigned = true;

	if (smmu_domain->stage == ARM_SMMU_DOMAIN_BYPASS) {
		ste->s1_cfg = NULL;
		ste->s2_cfg = NULL;
	} else if (smmu_domain->stage == ARM_SMMU_DOMAIN_S1) {
		ste->s1_cfg = &smmu_domain->s1_cfg;
		ste->s1_cfg->cd.ssid = arm_smmu_get_ssid(dev->iommu_fwspec);
		ste->s1_cfg->cd.ssid_nums = of_get_ssid_nums(dev->of_node);
		ste->s1_cfg->tlb_flush = master->tlb_flush;
		ste->s1_cfg->vmid = arm_smmu_get_s1_cfg_vmid(dev, smmu_domain);
		ste->s2_cfg = NULL;
		arm_smmu_write_ctx_desc(smmu, ste->s1_cfg);
	} else {
		ste->s1_cfg = NULL;
		ste->s2_cfg = &smmu_domain->s2_cfg;
	}
}

static int arm_smmu_domain_attch_dev(struct device *dev,
				struct arm_smmu_domain *smmu_domain)
{
	struct mm_dom_cookie *cookie = NULL;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_master_data *master = NULL;

	master = dev->iommu_fwspec->iommu_priv;
	smmu = master->smmu;

	arm_smmu_set_ste(smmu_domain, dev);

	cookie = smmu_domain->domain.iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s has no cookie!\n", __func__);
		return -ENOMEM;
	}
	if ((cookie->iova.iova_start != master->iova.iova_start) ||
		(cookie->iova.iova_size != master->iova.iova_size) ||
		(cookie->iova.iova_align != master->iova.iova_align)) {
		dev_err(dev, "%s iova mismatch!\n", __func__);
		dev_err(dev, "pool iova:0x%lx,0x%lx,0x%lx\n",
			cookie->iova.iova_start, cookie->iova.iova_size,
			cookie->iova.iova_align);
		dev_err(dev, "master iova:0x%lx,0x%lx,0x%lx\n",
			master->iova.iova_start, master->iova.iova_size,
			master->iova.iova_align);
	}
	if (smmu_domain->tlb_flush != master->tlb_flush)
		dev_err(dev, "domain tlb_flush:%u, master tlb_flush:%u\n",
			smmu_domain->tlb_flush, master->tlb_flush);
	return 0;
}

#ifdef CONFIG_IOVA_POOL
static void arm_smmu_iova_lazy_free(struct mm_dom_cookie *cookie)
{
	unsigned int pages_num, i, size, pong;
	unsigned int pingpong;
	unsigned long iova_start;
	u32 *free_size = NULL;

	pingpong = 1UL - cookie->lazy_free->pingpong;
	pages_num = DIV_ROUND_UP(cookie->iova.iova_size, PAGE_SIZE);
	iova_start = cookie->iova.iova_start;
	free_size = cookie->lazy_free->free_size;
	mutex_lock(&cookie->lazy_free->mutex);
	for (i = 0; i < pages_num; i++) {
		size = free_size[i] & IOVA_SIZE_MASK;
		if (size == 0)
			continue;

		pong = (free_size[i] & PINGPONG_MASK) >> PINGPONG_SHIFT;
		if (pong != pingpong)
			continue;

		free_size[i] = 0;
		gen_pool_free(cookie->iova_pool,
			(iova_start + ((unsigned long)i << PAGE_SHIFT)), size);
	}
	mutex_unlock(&cookie->lazy_free->mutex);
	cookie->lazy_free->end = true;
}

static int arm_smmu_iova_lazy_free_thread(void *p)
{
	struct device *dev = (struct device *)p;
	struct iommu_domain *domain = NULL;
	struct mm_dom_cookie *cookie = NULL;
	DEFINE_WAIT(wait);

	domain = iommu_get_domain_for_dev(dev);
	if (!domain) {
		dev_err(dev, "%s, domain is null\n", __func__);
		return -ENOENT;
	}

	cookie = (struct mm_dom_cookie *)domain->iova_cookie;
	if (!cookie) {
		dev_err(dev, "%s, iova_cookie is null\n", __func__);
		return -ENOENT;
	}

	while (!kthread_should_stop()) {
		prepare_to_wait(&cookie->lazy_free->wait_q, &wait,
			TASK_UNINTERRUPTIBLE);
		schedule();
		finish_wait(&cookie->lazy_free->wait_q, &wait);
		arm_smmu_flush_tlb(dev, domain);
		arm_smmu_iova_lazy_free(cookie);
	}
	return 0;
}

static void arm_smmu_cookie_alloc_lazy_free(struct device *dev,
				struct mm_dom_cookie *cookie)
{
	int ret;
	unsigned int pages_num;
	struct cpumask sched_cpus;
	struct mm_iova_lazy_free *lazy_free = NULL;
	struct sched_param param;

	if (cookie->iova.iova_free == IMME_FREE)
		return;

	lazy_free = kzalloc(sizeof(*lazy_free), GFP_KERNEL);
	if (!lazy_free)
		goto out_err;

	pages_num = DIV_ROUND_UP(cookie->iova.iova_size, PAGE_SIZE);
	lazy_free->free_size = kcalloc(pages_num, sizeof(u32), GFP_KERNEL);
	if (!lazy_free->free_size)
		goto out_free;

	init_waitqueue_head(&lazy_free->wait_q);
	mutex_init(&lazy_free->mutex);
	spin_lock_init(&lazy_free->lock);
	lazy_free->end = true;
	lazy_free->waterline = pages_num / LAZY_FREE_WATERLINE;
	if (lazy_free->waterline < LAZY_FREE_WATERLINE) {
		dev_info(dev, "%s,iova_size %lu too small,disable lazy free\n",
			__func__, cookie->iova.iova_size);
		goto out_free;
	}
	cookie->lazy_free = lazy_free;

	lazy_free->task = kthread_run(arm_smmu_iova_lazy_free_thread, dev,
		"iova.%s", dev_name(dev));
	if (IS_ERR(lazy_free->task)) {
		dev_err(dev, "%s, create lazy free task err %d\n",
			__func__, IS_ERR(lazy_free->task));
		goto out_stop;
	}

	cpumask_setall(&sched_cpus);
	cpumask_clear_cpu(WATCHDOG_CPU, &sched_cpus);
	cpumask_clear_cpu(WIFI_INTX_CPU, &sched_cpus);
	set_cpus_allowed_ptr(lazy_free->task, &sched_cpus);

	/* set thread priority and schedule policy */
	param.sched_priority = LAZY_FREE_SCHED_PRI;
	ret = sched_setscheduler(lazy_free->task, SCHED_RR, &param);
	if (ret)
		dev_info(dev, "%s, task set priority error\n", __func__);

	dev_info(dev, "%s, iova lazy free waterline %lu\n",
		__func__, lazy_free->waterline);

	return;

out_stop:
	kfree(lazy_free->free_size);
	cookie->lazy_free = NULL;
out_free:
	kfree(lazy_free);
out_err:
	cookie->iova.iova_free = IMME_FREE;
	dev_err(dev, "%s iova_free rollback imme free!\n", __func__);
}

static int arm_smmu_domain_alloc_cookie(struct device *dev,
				struct arm_smmu_domain *smmu_domain,
				struct iommu_domain *domain)
{
	struct mm_dom_cookie *cookie = NULL;
	struct arm_smmu_master_data *master = NULL;

	cookie = kzalloc(sizeof(*cookie), GFP_KERNEL);
	if (!cookie)
		return -ENOMEM;

	master = dev->iommu_fwspec->iommu_priv;
	cookie->iova_pool = iova_pool_setup(master->iova.iova_start,
		master->iova.iova_size, master->iova.iova_align);
	if (!cookie->iova_pool) {
		pr_err("setup dev(%s) iova pool fail\n", dev_name(dev));
		kfree(cookie);
		return -ENOMEM;
	}
	spin_lock_init(&cookie->iova_lock);
	cookie->iova_root = RB_ROOT;
	cookie->domain = domain;
	cookie->iova.iova_start = master->iova.iova_start;
	cookie->iova.iova_size = master->iova.iova_size;
	cookie->iova.iova_align = master->iova.iova_align;
	cookie->iova.iova_free = master->iova.iova_free;
	smmu_domain->domain.iova_cookie = cookie;

	arm_smmu_cookie_alloc_lazy_free(dev, cookie);

	return 0;
}
#endif

static int arm_smmu_attach_dev(struct iommu_domain *domain, struct device *dev)
{
	int ret;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);
	struct arm_smmu_master_data *master = NULL;

	if (!dev->iommu_fwspec)
		return -ENOENT;

	master = dev->iommu_fwspec->iommu_priv;
	smmu = master->smmu;

	/* Already attached to a different domain? */
	if ((master->ste.assigned) && (domain->type != IOMMU_DOMAIN_UNMANAGED))
		arm_smmu_detach_dev(dev);

	mutex_lock(&smmu_domain->init_mutex);

	if (!smmu_domain->smmu) {
		smmu_domain->smmu = smmu;
		smmu_domain->tlb_flush = master->tlb_flush;
		smmu_domain->dev = dev;
		smmu_domain->type = arm_smmu_get_domain_type(dev);
		ret = arm_smmu_domain_finalise(domain);
		if (ret) {
			smmu_domain->smmu = NULL;
			goto out_unlock;
		}
		/* svm bind task */
		if (is_svm_bind_task_domain(domain))
			goto out_unlock;
	} else if (smmu_domain->smmu != smmu) {
		dev_err(dev, "cannot attach to SMMU %s (upstream of %s)\n",
			dev_name(smmu_domain->smmu->dev), dev_name(smmu->dev));
		ret = -ENXIO;
		goto out_unlock;
	} else {
		/* Only first attach device install ste and alloc cookie */
		ret = arm_smmu_domain_attch_dev(dev, smmu_domain);
		goto out_unlock;
	}

	arm_smmu_set_ste(smmu_domain, dev);

	arm_smmu_install_ste_for_dev(dev->iommu_fwspec);
#ifdef CONFIG_IOVA_POOL
	ret = arm_smmu_domain_alloc_cookie(dev, smmu_domain, domain);
	if (ret) {
		dev_err(dev, "%s domain alloc cookie return %d error\n",
			dev_name(smmu->dev), ret);
		smmu_domain->smmu = NULL;
		goto out_unlock;
	}
#endif
out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	dev_err(dev, "%s attach to SMMU\n", __func__);
	return ret;
}

static int arm_smmu_flush_pgtbl_cache(struct iommu_domain *domain,
				unsigned long addr, size_t size)
{
	int ret = 0;
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;
	pgd_t *pgd = arm_get_pgd_from_ops(ops);

	if (!pgd) {
		pr_err("%s: pgd is null\n", __func__);
		return -EINVAL;
	}

	ret = mm_flush_pgtbl_cache(pgd, addr, size);

	return ret;
}

static int __arm_smmu_map(struct iommu_domain *domain, unsigned long iova,
				phys_addr_t paddr, size_t size, int prot)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;

	if (!ops)
		return -ENODEV;

	return ops->map(ops, iova, paddr, size, prot);
}

static int arm_smmu_map(struct iommu_domain *domain, unsigned long iova,
				phys_addr_t paddr, size_t size, int prot)
{
	int ret;

	ret = __arm_smmu_map(domain, iova, paddr, size, prot);
	if (ret)
		return ret;

	ret = arm_smmu_flush_pgtbl_cache(domain, iova, size);
	if (ret)
		return ret;

	return 0;
}

static size_t arm_smmu_unmap(struct iommu_domain *domain,
				unsigned long iova, size_t size)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;
	int unmap_size;
	size_t unmapped = 0;

	if (!ops)
		return 0;

	while(unmapped < size) {
		size_t pgsize = iommu_pgsize(domain, iova, size - unmapped);

		unmap_size = ops->unmap(ops, iova, pgsize);
		if (!unmap_size)
			break;

		iova += unmap_size;
		unmapped += unmap_size;
	}

	return unmapped;
}

static phys_addr_t arm_smmu_iova_to_phys(struct iommu_domain *domain,
				dma_addr_t iova)
{
	struct io_pgtable_ops *ops = to_smmu_domain(domain)->pgtbl_ops;

	if (domain->type == IOMMU_DOMAIN_IDENTITY)
		return iova;

	if (!ops)
		return 0;

	return ops->iova_to_phys(ops, iova);
}

static int arm_iommu_map(struct iommu_domain *domain, unsigned long iova,
				phys_addr_t paddr, size_t size, int prot)
{
	unsigned long orig_iova = iova;
	unsigned int min_pagesz;
	size_t orig_size = size;
	int ret = 0;

	if (unlikely(domain->ops->map == NULL ||
		     domain->pgsize_bitmap == 0UL))
		return -ENODEV;

	if (unlikely(!(domain->type & __IOMMU_DOMAIN_PAGING)))
		return -EINVAL;

	/* find out the minimum page size supported */
	min_pagesz = 1 << __ffs(domain->pgsize_bitmap);

	/*
	 * both the virtual address and the physical one, as well as
	 * the size of the mapping, must be aligned (at least) to the
	 * size of the smallest page supported by the hardware
	 */
	if (!IS_ALIGNED(iova | paddr | size, min_pagesz)) {
		pr_err("unaligned: iova 0x%lx pa %pa size 0x%zx min_pagesz 0x%x\n",
		       iova, &paddr, size, min_pagesz);
		return -EINVAL;
	}

	while (size) {
		size_t pgsize = iommu_pgsize(domain, iova | paddr, size);

		ret = __arm_smmu_map(domain, iova, paddr, pgsize, prot);
		if (ret)
			break;

		iova += pgsize;
		paddr += pgsize;
		size -= pgsize;
	}

	/* unroll mapping in case something went wrong */
	if (ret)
		iommu_unmap(domain, orig_iova, orig_size - size);

	return ret;
}

static size_t arm_smmu_map_sg(struct iommu_domain *domain, unsigned long iova,
			 struct scatterlist *sg, unsigned int nents, int prot)
{
	struct scatterlist *s = NULL;
	size_t mapped = 0;
	unsigned int i, min_pagesz;
	int ret;

	if (unlikely(domain->pgsize_bitmap == 0UL))
		return 0;

	min_pagesz = 1 << __ffs(domain->pgsize_bitmap);

	for_each_sg(sg, s, nents, i) {
		struct page *pg = sg_page(s);
		phys_addr_t phys = page_to_phys(pg) + s->offset;

		/*
		 * We are mapping on IOMMU page boundaries, so offset within
		 * the page must be 0. However, the IOMMU may support pages
		 * smaller than PAGE_SIZE, so s->offset may still represent
		 * an offset of that boundary within the CPU page.
		 */
		if (!IS_ALIGNED(s->offset, min_pagesz))
			goto out_err;

#ifdef CONFIG_HISI_LB
		if (PageLB(pg))
			phys |= PTE_LB(lb_page_to_gid(pg));
#endif

		ret = arm_iommu_map(domain,
				iova + mapped, phys, s->length, prot);
		if (ret)
			goto out_err;

		mapped += s->length;
	}
	(void)arm_smmu_flush_pgtbl_cache(domain, iova, mapped);
	return mapped;

out_err:
	/* undo mappings already done */
	iommu_unmap(domain, iova, mapped);

	return 0;
}

static struct platform_driver arm_smmu_driver;

static int arm_smmu_match_node(struct device *dev, void *data)
{
	return dev->fwnode == data;
}

static struct arm_smmu_device *arm_smmu_get_by_fwnode(
	struct fwnode_handle *fwnode)
{
	struct device *dev = driver_find_device(&arm_smmu_driver.driver, NULL,
						fwnode, arm_smmu_match_node);
	put_device(dev);
	return dev ? dev_get_drvdata(dev) : NULL;
}

static bool arm_smmu_sid_in_range(struct arm_smmu_device *smmu, u32 sid)
{
	unsigned long limit = smmu->strtab_cfg.num_l1_ents;

	if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB)
		limit *= 1UL << STRTAB_SPLIT;

	return sid < limit;
}

static struct iommu_ops arm_smmu_ops;


static int of_get_iova_info_smmu(struct device_node *np,
				struct iommu_domain_data *iova)
{
	struct device_node *node = NULL;
	int ret;

	iova->iova_start = IOVA_DEFAULT_ADDS;
	iova->iova_size = IOVA_DEFAULT_SIZE;
	iova->iova_align = PAGE_SIZE;

	if (!np)
		return -ENODEV;

	node = of_get_child_by_name(np, "iova_info");
	if (!node) {
		pr_err("no iova_info, default cfg(0x%lx, 0x%lx, 0x%lx, %u)\n",
			iova->iova_start, iova->iova_size,
			iova->iova_align, iova->iova_free);
		return 0;
	}
	ret = of_property_read_u64(node, "start-addr",
		(u64 *)&iova->iova_start);
	if (ret)
		pr_err("read iova start address error\n");

	ret = of_property_read_u64(node, "size", (u64 *)&iova->iova_size);
	if (ret)
		pr_err("read iova size error\n");

	ret = of_property_read_u64(node, "iova-align",
		(u64 *)&iova->iova_align);
	if (ret)
		pr_err("read iova align error\n");

	ret = of_property_read_u32(node, "iova-free", (u32 *)&iova->iova_free);
	if (ret)
		pr_info("iova free is default\n");

	pr_err("%s:start_addr 0x%lx, size 0x%lx align 0x%lx, free %u\n",
			__func__, iova->iova_start, iova->iova_size,
			iova->iova_align, iova->iova_free);

	return 0;
}

static u32 of_get_tlb_flush(struct device_node *np)
{
	u32 tlb_flush = ASID_FLUSH;
	int ret;

	if (!np)
		return ASID_FLUSH;

	ret = of_property_read_u32(np, "tlb_flush", &tlb_flush);
	if (ret)
		return ASID_FLUSH;

	pr_err("%s:tlb_flush %u\n", __func__, tlb_flush);
	return tlb_flush;
}

static int arm_smmu_add_device(struct device *dev)
{
	int i, ret;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_master_data *master = NULL;
	struct iommu_fwspec *fwspec = dev->iommu_fwspec;
	struct iommu_group *group = NULL;

	if (!fwspec || fwspec->ops != &arm_smmu_ops)
		return -ENODEV;

	/*
	 * We _can_ actually withstand dodgy bus code re-calling add_device()
	 * without an intervening remove_device()/of_xlate() sequence, but
	 * we're not going to do so quietly...
	 */
	if (WARN_ON_ONCE(fwspec->iommu_priv)) {
		master = fwspec->iommu_priv;
		smmu = master->smmu;
	} else {
		smmu = arm_smmu_get_by_fwnode(fwspec->iommu_fwnode);
		if (!smmu)
			return -ENODEV;

		master = kzalloc(sizeof(*master), GFP_KERNEL);
		if (!master)
			return -ENOMEM;

		master->smmu = smmu;
		master->tlb_flush = (u8)of_get_tlb_flush(dev->of_node);
		ret = of_get_iova_info_smmu(dev->of_node, &master->iova);
		if (ret)
			pr_err("get dev(%s) iova info fail\n", dev_name(dev));
		fwspec->iommu_priv = master;
	}

	/* Check the SIDs are in range of the SMMU and our stream table */
	for (i = 0; i < fwspec->num_ids; i++) {
		u32 sid = fwspec->ids[i];

		pr_err("%s: sid %u\n", __func__, sid);
		if (!arm_smmu_sid_in_range(smmu, sid)) {
			pr_err("%s: invalid sid range\n", __func__);
			return -ERANGE;
		}

		/* Ensure l2 strtab is initialised */
		if (smmu->features & ARM_SMMU_FEAT_2_LVL_STRTAB) {
			ret = arm_smmu_init_l2_strtab(smmu, sid);
			if (ret) {
				pr_err("%s: init_l2_strtab\n", __func__);
				return ret;
			}
		}
		break;
	}

	group = iommu_group_get_for_dev(dev);
	if (!IS_ERR(group)) {
		iommu_group_put(group);
		iommu_device_link(&smmu->iommu, dev);
	}

	mm_iommu_setup_dma_ops(dev);

	return PTR_ERR_OR_ZERO(group);
}

static void arm_smmu_remove_device(struct device *dev)
{
	struct iommu_fwspec *fwspec = dev->iommu_fwspec;
	struct arm_smmu_master_data *master = NULL;
	struct arm_smmu_device *smmu = NULL;

	if (!fwspec || fwspec->ops != &arm_smmu_ops)
		return;

	master = fwspec->iommu_priv;
	smmu = master->smmu;
	if (master && master->ste.assigned)
		arm_smmu_detach_dev(dev);
	iommu_group_remove_device(dev);
	iommu_device_unlink(&smmu->iommu, dev);
	kfree(master);
	iommu_fwspec_free(dev);
}

static struct iommu_group *arm_smmu_get_iommu_group_by_sid(struct device *dev)
{
	u32 sid;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu) {
		dev_err(dev, "%s, dev smmu is null\n", __func__);
		return NULL;
	}

	sid = arm_smmu_get_sid(dev->iommu_fwspec);
	if (!arm_smmu_sid_in_range(smmu, sid)) {
		dev_err(dev, "%s: %u out of %s sid range\n", __func__, sid,
			dev_name(smmu->dev));
		return NULL;
	}

	list_for_each_entry(arm_smmu_group, &(smmu->iommu_groups), list)
		if (arm_smmu_group->sid == sid)
			return  arm_smmu_group->group;

	return NULL;
}

static void arm_smmu_iommu_group_register(struct device *dev,
				struct iommu_group *group)
{
	u32 sid;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	if (IS_ERR_OR_NULL(group))
		return;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu) {
		dev_err(dev, "%s, dev smmu is null\n", __func__);
		return;
	}

	sid = arm_smmu_get_sid(dev->iommu_fwspec);
	if (!arm_smmu_sid_in_range(smmu, sid)) {
		dev_err(dev, "%s: %u out of %s sid range\n", __func__, sid,
			dev_name(smmu->dev));
		return;
	}

	arm_smmu_group = kzalloc(sizeof(*arm_smmu_group), GFP_KERNEL);
	if (!arm_smmu_group)
		return;

	arm_smmu_group->group = group;
	BLOCKING_INIT_NOTIFIER_HEAD(&arm_smmu_group->notifier);
	arm_smmu_group->sid = sid;
	spin_lock(&smmu->iommu_groups_lock);
	list_add_tail(&arm_smmu_group->list, &(smmu->iommu_groups));
	spin_unlock(&smmu->iommu_groups_lock);
}

static struct iommu_group *arm_smmu_device_group(struct device *dev)
{
	struct iommu_group *group = NULL;

	/*
	 * We don't support devices sharing stream IDs other than PCI RID
	 * aliases, since the necessary ID-to-device lookup becomes rather
	 * impractical given a potential sparse 32-bit stream ID space.
	 */
	group = arm_smmu_get_iommu_group_by_sid(dev);
	if (group)
		return group;

	if (dev_is_pci(dev))
		group = pci_device_group(dev);
	else
		group = generic_device_group(dev);

	arm_smmu_iommu_group_register(dev, group);
	return group;
}

static int arm_smmu_domain_get_attr(struct iommu_domain *domain,
				enum iommu_attr attr, void *data)
{
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (domain->type != IOMMU_DOMAIN_UNMANAGED)
		return -EINVAL;

	switch (attr) {
	case DOMAIN_ATTR_NESTING:
		*(int *)data = (smmu_domain->stage == ARM_SMMU_DOMAIN_NESTED);
		return 0;
	default:
		pr_err("%s: attr not support, attr = %d", __func__, (int)attr);
		return -ENODEV;
	}
}

static int arm_smmu_domain_set_attr(struct iommu_domain *domain,
				enum iommu_attr attr, void *data)
{
	int ret = 0;
	struct arm_smmu_domain *smmu_domain = to_smmu_domain(domain);

	if (domain->type != IOMMU_DOMAIN_UNMANAGED)
		return -EINVAL;

	mutex_lock(&smmu_domain->init_mutex);

	switch (attr) {
	case DOMAIN_ATTR_NESTING:
		if (smmu_domain->smmu) {
			ret = -EPERM;
			goto out_unlock;
		}

		if (*(int *)data)
			smmu_domain->stage = ARM_SMMU_DOMAIN_NESTED;
		else
			smmu_domain->stage = ARM_SMMU_DOMAIN_S1;

		break;
	case DOMAIN_ATTR_PGD:
		ret = arm_smmu_domain_set_pgd(smmu_domain,
			(struct task_struct *)data);
		break;
	default:
		ret = -ENODEV;
	}

out_unlock:
	mutex_unlock(&smmu_domain->init_mutex);
	return ret;
}

static int arm_smmu_of_xlate(struct device *dev, struct of_phandle_args *args)
{
	dev_err(dev, "%s, args->args_count %d\n", __func__, args->args_count);
	return iommu_fwspec_add_ids(dev, args->args, args->args_count);
}

static void arm_smmu_get_resv_regions(struct device *dev,
				struct list_head *head)
{
	struct iommu_resv_region *region;
	int prot = IOMMU_WRITE | IOMMU_NOEXEC | IOMMU_MMIO;

	region = iommu_alloc_resv_region(MSI_IOVA_BASE, MSI_IOVA_LENGTH,
					 prot, IOMMU_RESV_SW_MSI);
	if (!region)
		return;

	list_add_tail(&region->list, head);

	iommu_dma_get_resv_regions(dev, head);
}

static void arm_smmu_put_resv_regions(struct device *dev,
				struct list_head *head)
{
	struct iommu_resv_region *entry = NULL;
	struct iommu_resv_region *next = NULL;

	list_for_each_entry_safe(entry, next, head, list)
		kfree(entry);
}

static struct iommu_ops arm_smmu_ops = {
	.capable		= arm_smmu_capable,
	.domain_alloc		= arm_smmu_domain_alloc,
	.domain_free		= arm_smmu_domain_free,
	.attach_dev		= arm_smmu_attach_dev,
	.detach_dev		= arm_smmu_svm_detach_dev,
	.map			= arm_smmu_map,
	.unmap			= arm_smmu_unmap,
	.map_sg			= arm_smmu_map_sg,
	.iova_to_phys		= arm_smmu_iova_to_phys,
	.add_device		= arm_smmu_add_device,
	.remove_device		= arm_smmu_remove_device,
	.device_group		= arm_smmu_device_group,
	.domain_get_attr	= arm_smmu_domain_get_attr,
	.domain_set_attr	= arm_smmu_domain_set_attr,
	.of_xlate		= arm_smmu_of_xlate,
	.get_resv_regions	= arm_smmu_get_resv_regions,
	.put_resv_regions	= arm_smmu_put_resv_regions,
	.flush_tlb		= arm_smmu_flush_tlb,
	.dev_flush_tlb		= arm_smmu_dev_flush_tlb,
	.pgsize_bitmap		= -1UL, /* Restricted during device attach */
};

static int arm_smmu_comm_add_device(struct device *dev)
{
	if ((!dev->iommu_fwspec) || (!dev->iommu_fwspec->ops))
		return -ENODEV;

	return dev->iommu_fwspec->ops->add_device(dev);
}

static void arm_smmu_comm_remove_device(struct device *dev)
{
	if ((!dev->iommu_fwspec) || (!dev->iommu_fwspec->ops))
		return;

	dev->iommu_fwspec->ops->remove_device(dev);
}

static struct iommu_group *arm_smmu_comm_device_group(struct device *dev)
{
	if ((!dev->iommu_fwspec) || (!dev->iommu_fwspec->ops))
		return NULL;

	return dev->iommu_fwspec->ops->device_group(dev);
}

static struct iommu_ops arm_smmu_comm_ops = {
	.capable		= arm_smmu_capable,
	.add_device	= arm_smmu_comm_add_device,
	.remove_device	= arm_smmu_comm_remove_device,
	.device_group	= arm_smmu_comm_device_group,
	.get_resv_regions	= arm_smmu_get_resv_regions,
	.put_resv_regions	= arm_smmu_put_resv_regions,
};

/* Probing and initialisation functions */
static int arm_smmu_init_one_queue(struct arm_smmu_device *smmu,
				struct arm_smmu_queue *q,
				unsigned long prod_off,
				unsigned long cons_off,
				size_t dwords)
{
	size_t qsz = ((1 << q->max_n_shift) * dwords) << DWORD_BYTES_NUM;

	q->base = dmam_alloc_coherent(smmu->dev, qsz, &q->base_dma, GFP_KERNEL);
	if (!q->base) {
		dev_err(smmu->dev, "failed to allocate queue (0x%zx bytes)\n",
			qsz);
		return -ENOMEM;
	}

	q->prod_reg	= arm_smmu_page1_fixup(prod_off, smmu);
	q->cons_reg	= arm_smmu_page1_fixup(cons_off, smmu);
	q->ent_dwords	= dwords;

	q->q_base = q->base_dma & Q_BASE_ADDR_MASK << Q_BASE_ADDR_SHIFT;
	q->q_base |= (q->max_n_shift & Q_BASE_LOG2SIZE_MASK)
		     << Q_BASE_LOG2SIZE_SHIFT;

	q->prod = q->cons = 0;
	return 0;
}

static int arm_smmu_init_queues(struct arm_smmu_device *smmu)
{
	int ret;

	/* cmdq */
	spin_lock_init(&smmu->cmdq.lock);
	ret = arm_smmu_init_one_queue(smmu, &smmu->cmdq.q, ARM_SMMU_CMDQ_PROD,
				      ARM_SMMU_CMDQ_CONS, CMDQ_ENT_DWORDS);
	if (ret)
		return ret;

	/* evtq */
	ret = arm_smmu_init_one_queue(smmu, &smmu->evtq.q, ARM_SMMU_EVTQ_PROD,
				      ARM_SMMU_EVTQ_CONS, EVTQ_ENT_DWORDS);
	if (ret)
		return ret;

	/* priq */
	if (!(smmu->features & ARM_SMMU_FEAT_PRI))
		return 0;

	return arm_smmu_init_one_queue(smmu, &smmu->priq.q, ARM_SMMU_PRIQ_PROD,
				       ARM_SMMU_PRIQ_CONS, PRIQ_ENT_DWORDS);
}

static int arm_smmu_init_l1_strtab(struct arm_smmu_device *smmu)
{
	unsigned int i;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;
	size_t size = sizeof(*cfg->l1_desc) * cfg->num_l1_ents;
	void *strtab = smmu->strtab_cfg.strtab;

	cfg->l1_desc = devm_kzalloc(smmu->dev, size, GFP_KERNEL);
	if (!cfg->l1_desc)
		return -ENOMEM;

	for (i = 0; i < cfg->num_l1_ents; ++i) {
		arm_smmu_write_strtab_l1_desc(strtab, &cfg->l1_desc[i]);
		strtab += STRTAB_L1_DESC_DWORDS << DWORD_BYTES_NUM;
	}

	return 0;
}

static int arm_smmu_init_strtab_2lvl(struct arm_smmu_device *smmu)
{
	void *strtab = NULL;
	u64 reg;
	u32 size, l1size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	/* Calculate the L1 size, capped to the SIDSIZE. */
	size = STRTAB_L1_SZ_SHIFT
		- (ilog2(STRTAB_L1_DESC_DWORDS) + DWORD_BYTES_NUM);
	size = min(size, smmu->sid_bits - STRTAB_SPLIT);
	cfg->num_l1_ents = 1 << size;

	size += STRTAB_SPLIT;
	if (size < smmu->sid_bits)
		dev_warn(smmu->dev,
			 "2-level strtab only covers %u/%u bits of SID\n",
			 size, smmu->sid_bits);

	l1size = cfg->num_l1_ents * (STRTAB_L1_DESC_DWORDS << DWORD_BYTES_NUM);
	strtab = dmam_alloc_coherent(smmu->dev, l1size, &cfg->strtab_dma,
				     GFP_KERNEL | __GFP_ZERO);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate l1 stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;

	/* Configure strtab_base_cfg for 2 levels */
	reg  = STRTAB_BASE_CFG_FMT_2LVL;
	reg |= (size & STRTAB_BASE_CFG_LOG2SIZE_MASK)
		<< STRTAB_BASE_CFG_LOG2SIZE_SHIFT;
	reg |= (STRTAB_SPLIT & STRTAB_BASE_CFG_SPLIT_MASK)
		<< STRTAB_BASE_CFG_SPLIT_SHIFT;
	cfg->strtab_base_cfg = reg;

	return arm_smmu_init_l1_strtab(smmu);
}

static int arm_smmu_init_strtab_linear(struct arm_smmu_device *smmu)
{
	void *strtab;
	u64 reg;
	u32 size;
	struct arm_smmu_strtab_cfg *cfg = &smmu->strtab_cfg;

	size = (1 << smmu->sid_bits) *
		(STRTAB_STE_DWORDS << DWORD_BYTES_NUM);
	strtab = dmam_alloc_coherent(smmu->dev, size, &cfg->strtab_dma,
				     GFP_KERNEL | __GFP_ZERO);
	if (!strtab) {
		dev_err(smmu->dev,
			"failed to allocate linear stream table (%u bytes)\n",
			size);
		return -ENOMEM;
	}
	cfg->strtab = strtab;
	cfg->num_l1_ents = 1 << smmu->sid_bits;

	/* Configure strtab_base_cfg for a linear table covering all SIDs */
	reg  = STRTAB_BASE_CFG_FMT_LINEAR;
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
	reg  = smmu->strtab_cfg.strtab_dma &
	       STRTAB_BASE_ADDR_MASK << STRTAB_BASE_ADDR_SHIFT;

	smmu->strtab_cfg.strtab_base = reg;

	/* Allocate the first VMID for stage-2 bypass STEs */
	set_bit(0, smmu->vmid_map);
	return 0;
}

static int arm_smmu_init_structures(struct arm_smmu_device *smmu)
{
	int ret;

	ret = arm_smmu_init_queues(smmu);
	if (ret)
		return ret;

	return arm_smmu_init_strtab(smmu);
}

static int arm_smmu_write_reg_sync(struct arm_smmu_device *smmu, u32 val,
				unsigned int reg_off, unsigned int ack_off)
{
	u32 reg;

	writel_relaxed(val, smmu->base + reg_off);
	return readl_relaxed_poll_timeout_atomic(smmu->base + ack_off, reg,
		reg == val, 1, ARM_SMMU_POLL_TIMEOUT_US);
}

/* GBPA is "special" */
static int arm_smmu_update_gbpa(struct arm_smmu_device *smmu, u32 set, u32 clr)
{
	int ret;
	u32 reg;
	u32 __iomem *gbpa = smmu->base + ARM_SMMU_GBPA;

	ret = readl_relaxed_poll_timeout_atomic(gbpa, reg, !(reg & GBPA_UPDATE),
					 1, ARM_SMMU_POLL_TIMEOUT_US);
	if (ret)
		return ret;

	reg &= ~clr;
	reg |= set;
	writel_relaxed(reg | GBPA_UPDATE, gbpa);
	return readl_relaxed_poll_timeout_atomic(gbpa, reg,
		!(reg & GBPA_UPDATE), 1, ARM_SMMU_POLL_TIMEOUT_US);
}

static void arm_smmu_setup_unique_irqs(struct arm_smmu_device *smmu)
{
	int irq, ret;

	/* Request interrupt lines */
	irq = smmu->evtq.q.irq;
	if (irq) {
		ret = devm_request_threaded_irq(smmu->dev, irq, NULL,
						arm_smmu_evtq_thread,
						IRQF_ONESHOT,
						"arm-smmu-v3-evtq", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable evtq irq\n");
	}

	irq = smmu->cmdq.q.irq;
	if (irq) {
		ret = devm_request_irq(smmu->dev, irq,
				       arm_smmu_cmdq_sync_handler, 0,
				       "arm-smmu-v3-cmdq-sync", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable cmdq-sync irq\n");
	}

	irq = smmu->gerr_irq;
	if (irq) {
		ret = devm_request_irq(smmu->dev, irq, arm_smmu_gerror_handler,
				       0, "arm-smmu-v3-gerror", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable gerror irq\n");
	}

	if (smmu->features & ARM_SMMU_FEAT_PRI) {
		irq = smmu->priq.q.irq;
		if (irq) {
			ret = devm_request_threaded_irq(smmu->dev, irq, NULL,
							arm_smmu_priq_thread,
							IRQF_ONESHOT,
							"arm-smmu-v3-priq",
							smmu);
			if (ret < 0)
				dev_warn(smmu->dev,
					 "failed to enable priq irq\n");
		}
	}
}

static int arm_smmu_setup_irqs(struct arm_smmu_device *smmu)
{
	int ret;
	u32 irqen_flags = IRQ_CTRL_EVTQ_IRQEN | IRQ_CTRL_GERROR_IRQEN;

	/* Disable IRQs first */
	ret = arm_smmu_write_reg_sync(smmu, 0, ARM_SMMU_IRQ_CTRL,
				      ARM_SMMU_IRQ_CTRLACK);
	if (ret) {
		dev_err(smmu->dev, "failed to disable irqs\n");
		return ret;
	}

	if (smmu->features & ARM_SMMU_FEAT_PRI)
		irqen_flags |= IRQ_CTRL_PRIQ_IRQEN;

	/* Enable interrupt generation on the SMMU */
	ret = arm_smmu_write_reg_sync(smmu, irqen_flags,
				      ARM_SMMU_IRQ_CTRL, ARM_SMMU_IRQ_CTRLACK);
	if (ret)
		dev_warn(smmu->dev, "failed to enable irqs\n");
#ifdef HISI_SMMUV3_SUPPORT
	writel_relaxed(TCU_IRPT_CLR_NS_VAL_MASK,
			smmu->base + ARM_SMMU_IRPT_CLR_NS);
	writel_relaxed(TCU_EVENT_TO_MASK, smmu->base + ARM_SMMU_IRPT_MASK_NS);
#endif
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

static int arm_smmu_cmdq_reset(struct arm_smmu_device *smmu, u32 *enables)
{
	int ret;

	smmu->cmdq.q.prod = 0;
	smmu->cmdq.q.cons = 0;
	writeq_relaxed(smmu->cmdq.q.q_base, smmu->base + ARM_SMMU_CMDQ_BASE);
	writel_relaxed(smmu->cmdq.q.prod, smmu->base + ARM_SMMU_CMDQ_PROD);
	writel_relaxed(smmu->cmdq.q.cons, smmu->base + ARM_SMMU_CMDQ_CONS);

	(*enables)  |= CR0_CMDQEN;
	ret = arm_smmu_write_reg_sync(smmu, *enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret)
		dev_err(smmu->dev, "failed to enable command queue\n");

	return ret;
}

static int arm_smmu_evtq_reset(struct arm_smmu_device *smmu, u32 *enables)
{
	int ret;

	smmu->evtq.q.prod = 0;
	smmu->evtq.q.cons = 0;
	writeq_relaxed(smmu->evtq.q.q_base, smmu->base + ARM_SMMU_EVTQ_BASE);
	writel_relaxed(smmu->evtq.q.prod,
		       arm_smmu_page1_fixup(ARM_SMMU_EVTQ_PROD, smmu));
	writel_relaxed(smmu->evtq.q.cons,
		       arm_smmu_page1_fixup(ARM_SMMU_EVTQ_CONS, smmu));

	(*enables) |= CR0_EVTQEN;
	ret = arm_smmu_write_reg_sync(smmu, *enables, ARM_SMMU_CR0,
				      ARM_SMMU_CR0ACK);
	if (ret)
		dev_err(smmu->dev, "failed to enable event queue\n");

	return ret;
}

static int arm_smmu_priq_reset(struct arm_smmu_device *smmu, u32 *enables)
{
	int ret = 0;

	smmu->priq.q.prod = 0;
	smmu->priq.q.cons = 0;
	if (smmu->features & ARM_SMMU_FEAT_PRI) {
		writeq_relaxed(smmu->priq.q.q_base,
			       smmu->base + ARM_SMMU_PRIQ_BASE);
		writel_relaxed(smmu->priq.q.prod,
			       arm_smmu_page1_fixup(ARM_SMMU_PRIQ_PROD, smmu));
		writel_relaxed(smmu->priq.q.cons,
			       arm_smmu_page1_fixup(ARM_SMMU_PRIQ_CONS, smmu));

		(*enables) |= CR0_PRIQEN;
		ret = arm_smmu_write_reg_sync(smmu, *enables, ARM_SMMU_CR0,
					      ARM_SMMU_CR0ACK);
		if (ret)
			dev_err(smmu->dev, "failed to enable PRI queue\n");
	}

	return ret;
}

static void arm_smmu_invalid_cache(struct arm_smmu_device *smmu)
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
}

static int arm_smmu_device_reset(struct arm_smmu_device *smmu, bool bypass)
{
	int ret;
	u32 reg;
	u32 enables = 0;

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
	ret = arm_smmu_cmdq_reset(smmu, &enables);
	if (ret)
		return ret;

	/* Event queue */
	ret = arm_smmu_evtq_reset(smmu, &enables);
	if (ret)
		return ret;

	/* PRI queue */
	ret = arm_smmu_priq_reset(smmu, &enables);
	if (ret)
		return ret;

	ret = arm_smmu_setup_irqs(smmu);
	if (ret) {
		dev_err(smmu->dev, "failed to setup irqs\n");
		return ret;
	}

	hisi_tcu_cache_bypass(smmu); /* bypass tcu cache */

	/* Enable the SMMU interface, or ensure bypass */
	if (!bypass || disable_bypass) {
		enables |= CR0_SMMUEN;
	} else {
		ret = arm_smmu_update_gbpa(smmu, 0, GBPA_ABORT);
		if (ret) {
			dev_err(smmu->dev, "GBPA not responding to update\n");
			return ret;
		}
	}
	ret = arm_smmu_write_reg_sync(smmu, enables, ARM_SMMU_CR0,
				ARM_SMMU_CR0ACK);
	if (ret) {
		dev_err(smmu->dev, "failed to enable SMMU interface\n");
		return ret;
	}

	return 0;
}

static u32 arm_smmu_device_get_idr0(struct arm_smmu_device *smmu)
{
	int ret;
	u32 idr0;
	struct device *dev = smmu->dev;

	ret = of_property_read_u32(dev->of_node, "smmu-idr0", &idr0);
	if (ret) {
		dev_err(dev, "%s:get smmu-idr0 failed\n", __func__);
		return 0;
	}
	return idr0;
}

static u32 arm_smmu_device_get_idr1(struct arm_smmu_device *smmu)
{
	int ret;
	u32 idr1;
	struct device *dev = smmu->dev;

	ret = of_property_read_u32(dev->of_node, "smmu-idr1", &idr1);
	if (ret) {
		dev_err(dev, "%s:get smmu-idr0 failed\n", __func__);
		return 0;
	}
	return idr1;
}

static u32 arm_smmu_device_get_idr5(struct arm_smmu_device *smmu)
{
	int ret;
	u32 idr5;
	struct device *dev = smmu->dev;

	ret = of_property_read_u32(dev->of_node, "smmu-idr5", &idr5);
	if (ret) {
		dev_err(dev, "%s:get smmu-idr0 failed\n", __func__);
		return 0;
	}
	return idr5;
}

static int arm_smmu_set_trans_table_endia(struct arm_smmu_device *smmu,
				u32 reg)
{
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
	return 0;
}

static void arm_smmu_set_boolean(struct arm_smmu_device *smmu,
				u32 reg)
{
	if (IS_ENABLED(CONFIG_PCI_PRI) && (reg & IDR0_PRI))
		smmu->features |= ARM_SMMU_FEAT_PRI;

	if (IS_ENABLED(CONFIG_PCI_ATS) && (reg & IDR0_ATS))
		smmu->features |= ARM_SMMU_FEAT_ATS;

	if (reg & IDR0_SEV)
		smmu->features |= ARM_SMMU_FEAT_SEV;

	if (reg & IDR0_MSI)
		smmu->features |= ARM_SMMU_FEAT_MSI;

	if (reg & IDR0_HYP)
		smmu->features |= ARM_SMMU_FEAT_HYP;
}

static int arm_smmu_device_hw_probe_idr0(struct arm_smmu_device *smmu)
{
	int ret;
	u32 reg;
	bool coherent = smmu->features & ARM_SMMU_FEAT_COHERENCY;

	/* IDR0 */
	reg = arm_smmu_device_get_idr0(smmu);
	dev_err(smmu->dev, "IDR0:0x%x", reg);

	/* 2-level structures */
	if (IS_ENABLED(CONFIG_CAN_ST_2_LVL_STRTAB) &&
		(reg & IDR0_ST_LVL_MASK << IDR0_ST_LVL_SHIFT)
		 == IDR0_ST_LVL_2LVL)
		smmu->features |= ARM_SMMU_FEAT_2_LVL_STRTAB;

	if (IS_ENABLED(CONFIG_CAN_CD_2_LVL_CDTAB) && (reg & IDR0_CD2L))
		smmu->features |= ARM_SMMU_FEAT_2_LVL_CDTAB;

	/*
	 * Translation table endianness.
	 * We currently require the same endianness as the CPU, but this
	 * could be changed later by adding a new IO_PGTABLE_QUIRK.
	 */
	ret = arm_smmu_set_trans_table_endia(smmu, reg);
	if (ret) {
		dev_err(smmu->dev, "set TT endianness return %d err!\n", ret);
		return ret;
	}

	/* Boolean feature flags */
	arm_smmu_set_boolean(smmu, reg);

	/*
	 * The coherency feature as set by FW is used in preference to the ID
	 * register, but warn on mismatch.
	 */
	if (!!(reg & IDR0_COHACC) != coherent)
		dev_err(smmu->dev, "IDR0.COHACC overridden by dma-coherent property (%s)\n",
			 coherent ? "true" : "false");

	switch (reg & IDR0_STALL_MODEL_MASK << IDR0_STALL_MODEL_SHIFT) {
	case IDR0_STALL_MODEL_STALL:
		/* Fallthrough */
	case IDR0_STALL_MODEL_FORCE:
		smmu->features |= ARM_SMMU_FEAT_STALLS;
	}

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
		smmu->ias = ARM_SMMU_ADDR_SIZE_40;
		/* Fallthrough */
	case IDR0_TTF_AARCH64:
		break;
	default:
		dev_err(smmu->dev, "AArch64 table format not supported!\n");
		return -ENXIO;
	}

	/* ASID/VMID sizes */
	smmu->asid_bits = reg & IDR0_ASID16 ? ARM_SMMU_ID_SIZE_16 :
				ARM_SMMU_ID_SIZE_8;
	smmu->vmid_bits = reg & IDR0_VMID16 ? ARM_SMMU_ID_SIZE_16 :
				ARM_SMMU_ID_SIZE_8;
	return 0;
}

static int arm_smmu_device_hw_probe_idr1(struct arm_smmu_device *smmu)
{
	u32 reg;
	/* IDR1 */
	reg = arm_smmu_device_get_idr1(smmu);
	if (reg & (IDR1_TABLES_PRESET | IDR1_QUEUES_PRESET | IDR1_REL)) {
		dev_err(smmu->dev, "embedded implementation not supported\n");
		return -ENXIO;
	}

	/* Queue sizes, capped at 4k */
	smmu->cmdq.q.max_n_shift = min((u32)CMDQ_MAX_SZ_SHIFT,
				(reg >> IDR1_CMDQ_SHIFT) & IDR1_CMDQ_MASK);
	if (!smmu->cmdq.q.max_n_shift) {
		/* Odd alignment restrictions on the base, so ignore for now */
		dev_err(smmu->dev, "unit-length command queue not supported\n");
		return -ENXIO;
	}

	smmu->evtq.q.max_n_shift = min((u32)EVTQ_MAX_SZ_SHIFT,
				(reg >> IDR1_EVTQ_SHIFT) & IDR1_EVTQ_MASK);
	smmu->priq.q.max_n_shift = min((u32)PRIQ_MAX_SZ_SHIFT,
				(reg >> IDR1_PRIQ_SHIFT) & IDR1_PRIQ_MASK);

	/* SID/SSID sizes */
	smmu->ssid_bits = (reg >> IDR1_SSID_SHIFT) & IDR1_SSID_MASK;
	smmu->sid_bits = (reg >> IDR1_SID_SHIFT) & IDR1_SID_MASK;

	/*
	 * If the SMMU supports fewer bits than would fill a single L2 stream
	 * table, use a linear table instead.
	 */
	if (smmu->sid_bits <= STRTAB_SPLIT || ste_force_linear) {
		smmu->ssid_bits = STRTAB_SURPPOT_SPLIT;
		smmu->sid_bits = STRTAB_SURPPOT_SPLIT;
		smmu->features &= ~ARM_SMMU_FEAT_2_LVL_STRTAB;
	}
	return 0;
}

static int arm_smmu_device_hw_probe_idr5(struct arm_smmu_device *smmu)
{
	u32 reg;
	/* IDR5 */
	reg = arm_smmu_device_get_idr5(smmu);

	/* Maximum number of outstanding stalls */
	smmu->evtq.max_stalls = (reg >> IDR5_STALL_MAX_SHIFT)
				& IDR5_STALL_MAX_MASK;

	/* Page sizes */
	if (reg & IDR5_GRAN64K)
		smmu->pgsize_bitmap |= SZ_64K | SZ_512M;
	if (reg & IDR5_GRAN16K)
		smmu->pgsize_bitmap |= SZ_16K | SZ_32M;
	if (reg & IDR5_GRAN4K)
		smmu->pgsize_bitmap |= SZ_4K | SZ_2M | SZ_1G;

	if (arm_smmu_ops.pgsize_bitmap == -1UL)
		arm_smmu_ops.pgsize_bitmap = smmu->pgsize_bitmap;
	else
		arm_smmu_ops.pgsize_bitmap |= smmu->pgsize_bitmap;

	/* Output address size */
	switch (reg & IDR5_OAS_MASK << IDR5_OAS_SHIFT) {
	case IDR5_OAS_32_BIT:
		smmu->oas = ARM_SMMU_ADDR_SIZE_32;
		break;
	case IDR5_OAS_36_BIT:
		smmu->oas = ARM_SMMU_ADDR_SIZE_36;
		break;
	case IDR5_OAS_40_BIT:
		smmu->oas = ARM_SMMU_ADDR_SIZE_40;
		break;
	case IDR5_OAS_42_BIT:
		smmu->oas = ARM_SMMU_ADDR_SIZE_42;
		break;
	case IDR5_OAS_44_BIT:
		smmu->oas = ARM_SMMU_ADDR_SIZE_44;
		break;
	default:
		dev_err(smmu->dev,
			"unknown output address size. Truncating to 48-bit\n");
		/* Fallthrough */
	case IDR5_OAS_48_BIT:
		smmu->oas = ARM_SMMU_ADDR_SIZE_48;
	}

	/* Set the DMA mask for our table walker */
	if (dma_set_mask_and_coherent(smmu->dev, DMA_BIT_MASK(smmu->oas)))
		dev_warn(smmu->dev,
			 "failed to set DMA mask for table walker\n");

	smmu->ias = max(smmu->ias, smmu->oas);

	return 0;
}

static void arm_smmu_hw_probe_tcu_cache(struct arm_smmu_device *smmu)
{
	bool ret = false;
	struct device *dev = smmu->dev;

	ret = of_property_read_bool(dev->of_node, "tcu_cache_disable");
	if (ret)
		dev_info(dev, "%s:not support tcu_cache\n", __func__);
	else
		smmu->features |= ARM_SMMU_FEAT_TCU_CACHE;
}

static int arm_smmu_device_hw_probe(struct arm_smmu_device *smmu)
{
	int ret;

	arm_smmu_hw_probe_tcu_cache(smmu);

	ret = arm_smmu_device_hw_probe_idr0(smmu);
	if (ret)
		return ret;

	ret = arm_smmu_device_hw_probe_idr1(smmu);
	if (ret)
		return ret;

	ret = arm_smmu_device_hw_probe_idr5(smmu);
	if (ret)
		return ret;

	return 0;
}

#ifdef CONFIG_ACPI
static void acpi_smmu_get_options(u32 model, struct arm_smmu_device *smmu)
{
	switch (model) {
	case ACPI_IORT_SMMU_V3_CAVIUM_CN99XX:
		smmu->options |= ARM_SMMU_OPT_PAGE0_REGS_ONLY;
		break;
	case ACPI_IORT_SMMU_HISILICON_HI161X:
		smmu->options |= ARM_SMMU_OPT_SKIP_PREFETCH;
		break;
	}

	dev_notice(smmu->dev, "option mask 0x%x\n", smmu->options);
}

static int arm_smmu_device_acpi_probe(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	struct acpi_iort_smmu_v3 *iort_smmu;
	struct device *dev = smmu->dev;
	struct acpi_iort_node *node;

	node = *(struct acpi_iort_node **)dev_get_platdata(dev);

	/* Retrieve SMMUv3 specific data */
	iort_smmu = (struct acpi_iort_smmu_v3 *)node->node_data;

	acpi_smmu_get_options(iort_smmu->model, smmu);

	if (iort_smmu->flags & ACPI_IORT_SMMU_V3_COHACC_OVERRIDE)
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	return 0;
}
#else
static inline int arm_smmu_device_acpi_probe(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	return -ENODEV;
}
#endif

static int arm_smmu_device_dt_probe(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	struct device *dev = &pdev->dev;
	u32 cells;
	u32 smmuid = 0;
	int ret = -EINVAL;

	if (of_property_read_u32(dev->of_node, "smmuid", &smmuid))
		dev_err(dev, "missing smmuid property\n");

	if (of_property_read_u32(dev->of_node, "#iommu-cells", &cells))
		dev_err(dev, "missing #iommu-cells property\n");
	else if (cells != IOMMU_CELLS_VALUE)
		dev_err(dev, "invalid #iommu-cells value (%d)\n", cells);
	else
		ret = 0;

	if (arm_smmu_get_tcu_pw_cfg(smmu, dev) == false)
		ret = -EINVAL;

	if (smmuid == SMMU_NPU)
		svm_smmu = smmu;
	else if (smmuid == SMMU_MEDIA1)
		smmu_media1 = smmu;
	else if (smmuid == SMMU_MEDIA2)
		smmu_media2 = smmu;

	parse_driver_options(smmu);

	if (of_dma_is_coherent(dev->of_node))
		smmu->features |= ARM_SMMU_FEAT_COHERENCY;

	return ret;
}

#ifdef HISI_SMMUV3_SUPPORT
static bool __get_tcu_pw_cfg(struct device *dev, const char *propname,
	int *req, int *ack)
{
	int ret;

	ret = of_property_read_u32_index(dev->of_node, propname, 0, req);
	if (ret) {
		dev_err(dev, "%s: %s miss req prop\n", __func__, propname);
		return false;
	}

	ret = of_property_read_u32_index(dev->of_node, propname, 1, ack);
	if (ret) {
		dev_err(dev, "%s: %s miss ack prop\n", __func__, propname);
		return false;
	}
	return true;
}

static bool arm_smmu_get_tcu_pw_cfg(struct arm_smmu_device *smmu,
	struct device *dev)
{
	if (of_property_read_bool(dev->of_node, "tcu_pw_reg")) {
		smmu->pw_cfg.clk_gate_ack = INVALID_TCU_PW_CFG;
		smmu->pw_cfg.pw_stat_ack = INVALID_TCU_PW_CFG;
		if (__get_tcu_pw_cfg(dev, "tcu_pw_reg", &(smmu->pw_cfg.lp_req_reg),
			&(smmu->pw_cfg.lp_ack_reg)) == false)
			return false;

		if (__get_tcu_pw_cfg(dev, "tcu_clk_req", &(smmu->pw_cfg.clk_gate_req),
			&(smmu->pw_cfg.clk_gate_ack)) == false)
			return false;

		if (__get_tcu_pw_cfg(dev, "tcu_pw_cfg", &(smmu->pw_cfg.pw_stat_req),
			&(smmu->pw_cfg.pw_stat_ack)) == false)
			return false;
	} else {
		smmu->pw_cfg.lp_req_reg = SMMU_LP_REQ;
		smmu->pw_cfg.lp_ack_reg = SMMU_LP_ACK;
		smmu->pw_cfg.clk_gate_req = TCU_QREQN_CG;
		smmu->pw_cfg.clk_gate_ack = TCU_QACCEPTN_CG;
		smmu->pw_cfg.pw_stat_req = TCU_QREQN_PD;
		smmu->pw_cfg.pw_stat_ack = TCU_QACCEPTN_PD;
	}

	return true;
}

static int arm_smmu_reg_set(struct arm_smmu_device *smmu,
				unsigned int req_off, unsigned int ack_off,
				unsigned int req_bit, unsigned int ack_bit)
{
	u32 reg;
	u32 val;

	val = readl_relaxed(smmu->base + req_off);
	val |= req_bit;
	writel_relaxed(val, smmu->base + req_off);
	return readl_relaxed_poll_timeout_atomic(smmu->base + ack_off, reg,
		reg & ack_bit, 1, ARM_SMMU_POLL_TIMEOUT_US);
}

int arm_smmu_reg_unset(struct arm_smmu_device *smmu, unsigned int req_off,
				unsigned int ack_off, unsigned int req_bit,
				unsigned int ack_bit)
{
	u32 reg;

	reg = readl_relaxed(smmu->base + req_off);
	reg &= ~req_bit;
	writel_relaxed(reg, smmu->base + req_off);
	return readl_relaxed_poll_timeout_atomic(smmu->base + ack_off, reg,
		!(reg & ack_bit), 1, ARM_SMMU_POLL_TIMEOUT_US);
}

static int arm_smmu_tcu_init(struct arm_smmu_device *smmu)
{
	int ret;

	/* Enable TCU internal clk */
	if (smmu->pw_cfg.clk_gate_req != INVALID_TCU_PW_CFG) {
		ret = arm_smmu_reg_set(smmu,
			smmu->pw_cfg.lp_req_reg, smmu->pw_cfg.lp_ack_reg,
			smmu->pw_cfg.clk_gate_req, smmu->pw_cfg.clk_gate_ack);
		if (ret) {
			pr_err("Enable TCU clk failed\n");
			return -EINVAL;
		}
	}

	/* Enable TCU internal power */
	if (smmu->pw_cfg.pw_stat_req != INVALID_TCU_PW_CFG) {
		ret = arm_smmu_reg_set(smmu,
			smmu->pw_cfg.lp_req_reg, smmu->pw_cfg.lp_ack_reg,
			smmu->pw_cfg.pw_stat_req, smmu->pw_cfg.pw_stat_ack);
		if (ret) {
			pr_err("Enable TCU power failed\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int arm_smmu_tcu_disable(struct arm_smmu_device *smmu)
{
	if (smmu->pw_cfg.clk_gate_req != INVALID_TCU_PW_CFG)
		arm_smmu_reg_unset(smmu,
			smmu->pw_cfg.lp_req_reg, smmu->pw_cfg.lp_ack_reg,
			smmu->pw_cfg.pw_stat_req, smmu->pw_cfg.pw_stat_ack);

	if (smmu->pw_cfg.pw_stat_req != INVALID_TCU_PW_CFG)
		arm_smmu_reg_unset(smmu,
			smmu->pw_cfg.lp_req_reg, smmu->pw_cfg.lp_ack_reg,
			smmu->pw_cfg.clk_gate_req, smmu->pw_cfg.clk_gate_ack);
	return 0;
}

static void arm_smmu_invalid_tcu_cache(struct arm_smmu_device *smmu)
{
	u32 reg;
	unsigned long flags;
	u32 check_times = 0;

	if (!(smmu->features & ARM_SMMU_FEAT_TCU_CACHE))
		return;

	spin_lock_irqsave(&smmu->tcu_lock, flags);
	if (smmu->status != ARM_SMMU_ENABLE) {
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		return;
	}

	reg = readl_relaxed(smmu->base + TCU_CACHE_BYPASS);
	if (reg & 0x1) {
		spin_unlock_irqrestore(&smmu->tcu_lock, flags);
		return;
	}

	writel_relaxed(TCU_CACHE_SOFT_INV, smmu->base + TCU_CACHELINE_INV_ALL);
	do {
		reg = readl_relaxed(smmu->base + TCU_CACHELINE_INV_ALL);
		if (!(reg & 0x1))
			break;
		udelay(1);
		if (++check_times >= MAX_CHECK_TIMES) {
			dev_err(smmu->dev, "CACHELINE_INV_ALL failed !%s\n",
				       __func__);
			spin_unlock_irqrestore(&smmu->tcu_lock, flags);
			return;
		}
	} while (1);
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
}
#endif

static int arm_smmu_set_bus(void)
{
	int ret = 0;

#ifdef CONFIG_PCI
	if (pci_bus_type.iommu_ops != &arm_smmu_ops) {
		pci_request_acs();
		ret = bus_set_iommu(&pci_bus_type, &arm_smmu_ops);
		if (ret) {
			pr_err("%s, pci bus set failed!\n", __func__);
			return ret;
		}
	}
#endif
#ifdef CONFIG_ARM_AMBA
	if (amba_bustype.iommu_ops != &arm_smmu_ops) {
		ret = bus_set_iommu(&amba_bustype, &arm_smmu_ops);
		if (ret) {
			pr_err("%s, amba bus set failed!\n", __func__);
			return ret;
		}
	}
#endif
	if (platform_bus_type.iommu_ops != &arm_smmu_comm_ops) {
		platform_bus_type.iommu_ops = NULL;
		ret = bus_set_iommu(&platform_bus_type, &arm_smmu_comm_ops);
		if (ret) {
			pr_err("%s, platform bus set failed!\n", __func__);
			return ret;
		}
	}
	return ret;
}

static int arm_smmu_set_irq(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	int irq, ret;

	irq = platform_get_irq_byname(pdev, "combined");
	if (irq > 0) {
		smmu->combined_irq = irq;
	} else {
		irq = platform_get_irq_byname(pdev, "eventq");
		if (irq > 0)
			smmu->evtq.q.irq = irq;

		irq = platform_get_irq_byname(pdev, "priq");
		if (irq > 0)
			smmu->priq.q.irq = irq;

		irq = platform_get_irq_byname(pdev, "cmdq-sync");
		if (irq > 0)
			smmu->cmdq.q.irq = irq;

		irq = platform_get_irq_byname(pdev, "gerror");
		if (irq > 0)
			smmu->gerr_irq = irq;
	}

	irq = smmu->combined_irq;
	if (irq) {
		/*
		 * Cavium ThunderX2 implementation doesn't not support unique
		 * irq lines. Use single irq line for all the SMMUv3 interrupts.
		 */
		ret = devm_request_threaded_irq(smmu->dev, irq,
					arm_smmu_combined_irq_handler,
					arm_smmu_combined_irq_thread,
					IRQF_ONESHOT,
					"arm-smmu-v3-combined-irq", smmu);
		if (ret < 0)
			dev_warn(smmu->dev, "failed to enable combined irq\n");
	} else {
		arm_smmu_setup_unique_irqs(smmu);
	}

	return 0;
}

static int arm_smmu_set_iommu(struct arm_smmu_device *smmu,
				struct device *dev, resource_size_t ioaddr)
{
	int ret;

	INIT_LIST_HEAD(&smmu->iommu_groups);
	spin_lock_init(&smmu->iommu_groups_lock);
	spin_lock_init(&smmu->tcu_lock);

	/* And we're up. Go go go! */
	ret = iommu_device_sysfs_add(&smmu->iommu, dev, NULL,
				"smmu3.%pa", &ioaddr);
	if (ret) {
		pr_err("%s, sysfs add failed!\n", __func__);
		return ret;
	}

	iommu_device_set_ops(&smmu->iommu, &arm_smmu_ops);
	iommu_device_set_fwnode(&smmu->iommu, dev->fwnode);

	ret = iommu_device_register(&smmu->iommu);
	if (ret) {
		dev_err(dev, "Failed to register iommu\n");
		return ret;
	}
	return 0;
}

static int arm_smmu_device_poweron(struct arm_smmu_device *smmu)
{
	int ret;

#ifdef HISI_SMMUV3_SUPPORT
	ret = arm_smmu_tcu_init(smmu);
	if (ret) {
		dev_err(smmu->dev, "%s, tcu init failed!\n", __func__);
		return ret;
	}
#endif
	/* Reset the device */
	ret = arm_smmu_device_reset(smmu, smmu->bypass);
	if (ret) {
		dev_err(smmu->dev, "%s, device reset failed!\n", __func__);
		return ret;
	}

	return ret;
}

static int arm_smmu_tcu_poweron(struct arm_smmu_device *smmu)
{
	int ret;
	unsigned long flags;

	if (!smmu) {
		pr_err("%s, smmu is null\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&smmu->tcu_lock, flags);
	if (smmu->status == ARM_SMMU_ENABLE) {
		dev_err(smmu->dev, "%s, status is enable\n", __func__);
		ret = -EINVAL;
		goto out_unlock;
	}

	ret = arm_smmu_device_poweron(smmu);
	if (ret) {
		dev_err(smmu->dev, "%s, tcu poweron failed\n", __func__);
		goto out_unlock;
	}
	smmu->cnt++;
	smmu->status = ARM_SMMU_ENABLE;

out_unlock:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);

	/* pls avoid deadlock for tcu_lock */
	if (!ret && smmu->status == ARM_SMMU_ENABLE) {
		if (smmu->smmuid == SMMU_HSDT0)
			pr_err("into poweron,smmuid:%u, cmd flush tlb\n",
				smmu->smmuid);
		arm_smmu_invalid_tcu_cache(smmu);
		arm_smmu_invalid_cache(smmu);
	}

	return ret;
}

static void arm_smmu_tcu_disable_irq(struct arm_smmu_device *smmu)
{
#ifdef HISI_SMMUV3_SUPPORT
	int ret;
	/* disable all interrupt */
	ret = arm_smmu_write_reg_sync(smmu, 0,
		ARM_SMMU_IRQ_CTRL, ARM_SMMU_IRQ_CTRLACK);
	if (ret)
		dev_warn(smmu->dev, "failed to disable irqs\n");

	/* mask all interrupt */
	writel_relaxed(TCU_EVENT_MASK_ALL, smmu->base + ARM_SMMU_IRPT_MASK_NS);

	/* clear all interrupt */
	writel_relaxed(TCU_IRPT_CLR_NS_VAL_MASK,
		smmu->base + ARM_SMMU_IRPT_CLR_NS);
#endif
}

static int arm_smmu_tcu_poweroff(struct arm_smmu_device *smmu)
{
	int ret;
	unsigned long flags;

	if (!smmu) {
		pr_err("%s, smmu is null\n", __func__);
		return -EINVAL;
	}

	spin_lock_irqsave(&smmu->tcu_lock, flags);
	if (smmu->status == ARM_SMMU_DISABLE) {
		dev_err(smmu->dev, "%s, status is disable\n", __func__);
		goto out_unlock;
	}
	smmu->status = ARM_SMMU_DISABLE;

	/* clear hardware irq */
	arm_smmu_tcu_disable_irq(smmu);

	/* clear smmu cr0 */
	ret = arm_smmu_device_disable(smmu);
	if (ret)
		dev_err(smmu->dev, "%s, device disable failed\n", __func__);

	ret = arm_smmu_tcu_disable(smmu);
	if (ret)
		dev_err(smmu->dev, "%s, tcu disable failed\n", __func__);

	if (smmu->cnt)
		smmu->cnt--;

out_unlock:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
	/* to ensure subsys can be powered off successfully */
	return 0;
}

static int arm_smmu_tcu_pm_notifier(struct notifier_block *nb,
				unsigned long event, void *data)
{
	int ret;
	struct arm_smmu_device *smmu = NULL;

	if (!nb) {
		pr_err("%s, nb is null\n", __func__);
		return NOTIFY_BAD;
	}

	smmu = container_of(nb, struct arm_smmu_device, tcu_pm_nb);
	if (event == REGULATOR_EVENT_ENABLE) {
		ret = arm_smmu_tcu_poweron(smmu);
		if (ret) {
			pr_err("%s, tcu poweron failed\n", __func__);
			return NOTIFY_BAD;
		}
	} else if (event == REGULATOR_EVENT_PRE_DISABLE) {
		ret = arm_smmu_tcu_poweroff(smmu);
		if (ret) {
			pr_err("%s, tcu poweroff failed\n", __func__);
			return NOTIFY_BAD;
		}
	}
	return NOTIFY_OK;
}

static int arm_smmu_register_pm_notifier(struct arm_smmu_device *smmu)
{
	int ret;
	u32 smmuid = 0;
	struct regulator *subsys_supply = NULL;

	ret = of_property_read_u32(smmu->dev->of_node, "smmuid", &smmuid);
	if (ret) {
		dev_err(smmu->dev, "%s, get smmuid failed\n", __func__);
		return ret;
	}

	if ((smmuid != SMMU_MEDIA1) && (smmuid != SMMU_MEDIA2))
		return 0;

	subsys_supply = devm_regulator_get(smmu->dev, "smmu_tcu");
	if (IS_ERR_OR_NULL(subsys_supply)) {
		dev_err(smmu->dev, "%s, get smmu_tcu failed\n", __func__);
		return -ENXIO;
	}

	smmu->tcu_pm_nb.notifier_call = arm_smmu_tcu_pm_notifier;
	ret = devm_regulator_register_notifier(subsys_supply,
		&smmu->tcu_pm_nb);
	if (ret) {
		dev_err(smmu->dev, "%s, register regulator notifier err %d\n",
			__func__, ret);
		return ret;
	}

	return 0;
}

static struct arm_smmu_device *arm_smmu_get_dev_smmu(struct device *dev)
{
	struct arm_smmu_master_data *master = NULL;

	if (!dev->iommu_fwspec) {
		dev_err(dev, "%s, iommu_fwspec is null\n", __func__);
		return NULL;
	}

	master = dev->iommu_fwspec->iommu_priv;
	if (!master) {
		dev_err(dev, "%s, iommu_priv is null\n", __func__);
		return NULL;
	}

	return master->smmu;
}

int arm_smmu_poweron(struct device *dev)
{
	int ret;
	struct arm_smmu_device *smmu = NULL;

	smmu = arm_smmu_get_dev_smmu(dev);

	ret = arm_smmu_tcu_poweron(smmu);
	if (ret)
		dev_err(dev, "%s, smmu power on err %d\n", __func__, ret);

	return ret;
}

int arm_smmu_poweroff(struct device *dev)
{
	int ret;
	struct arm_smmu_device *smmu = NULL;

	smmu = arm_smmu_get_dev_smmu(dev);

	ret = arm_smmu_tcu_poweroff(smmu);
	if (ret)
		dev_err(dev, "%s, smmu power off err %d\n", __func__, ret);

	return ret;
}

void arm_smmu_tbu_status_print(struct device *dev)
{
	struct arm_smmu_device *smmu = NULL;
	unsigned long flags;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu) {
		pr_err("%s, smmu is null\n", __func__);
		return;
	}

	spin_lock_irqsave(&smmu->tcu_lock, flags);

	if (smmu->status != ARM_SMMU_ENABLE) {
		pr_err("%s, smmu is disable\n", __func__);
		goto out;
	}
	(void)mm_smmu_tcu_node_status(smmu->smmuid);
out:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
}

int arm_smmu_get_reg_info(struct device *dev, char *buffer, int length)
{
	struct arm_smmu_device *smmu = NULL;
	unsigned long flags;
	int remain = length;
	unsigned int i;
	u32 val;
	int ret = -1;

	smmu = arm_smmu_get_dev_smmu(dev);
	if (!smmu) {
		pr_err("%s, smmu is null\n", __func__);
		return ret;
	}

	spin_lock_irqsave(&smmu->tcu_lock, flags);

	if (smmu->status != ARM_SMMU_ENABLE) {
		pr_err("%s, smmu-%d is disable\n", __func__, smmu->smmuid);
		goto out;
	}

#ifdef CONFIG_HISI_NPU_PCIE
	/* only used for npu eventq dump in pcie_npu_device environment */
	if (smmu == svm_smmu)
		(void)arm_smmu_evtq_thread(0, smmu);
#endif

	ret = snprintf(buffer, remain, "connect staus = 0x%x\n",
		mm_smmu_tcu_node_status(smmu->smmuid));
	if (ret < 0)
		goto out;

	buffer = buffer + ret;
	remain = remain - ret;

	for (i = 0; i < ARRAY_SIZE(smmu_tcu_info); i++) {
		val = readl_relaxed(smmu->base + smmu_tcu_info[i].offset);
		ret = snprintf(buffer, remain, "%s = 0x%x\n",
			smmu_tcu_info[i].name, val);
		if (ret < 0)
			goto out;

		buffer = buffer + ret;
		remain = remain - ret;
	}
	ret = 0;
out:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);
	return ret;
}

static int arm_smmu_save_smmuid(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	int ret;
	u32 smmuid = 0;
	struct device *dev = &pdev->dev;

	ret = of_property_read_u32(dev->of_node, "smmuid", &smmuid);
	if (ret) {
		dev_err(dev, "%s:get smmuid failed\n", __func__);
		return -1;
	}
	smmu->smmuid = smmuid;

	return 0;
}

static void arm_smmu_init_dfx(struct platform_device *pdev,
				struct arm_smmu_device *smmu)
{
	int ret;
	u32 crgbase;
	void *crg_vbase = NULL;
	struct device *dev = &pdev->dev;

	ret = of_property_read_u32(dev->of_node, "smmu-crg", &crgbase);
	if (ret)
		return;

	crg_vbase = ioremap(crgbase, SZ_4K);/*lint !e446*/
	if (!crg_vbase)
		return;
	smmu->crg_vbase = crg_vbase;

	pr_err("%s, crg base:0x%x, vbase:0x%pK",
			__func__, crgbase, crg_vbase);

}

static int arm_smmu_device_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res = NULL;
	struct arm_smmu_device *smmu = NULL;
	struct device *dev = &pdev->dev;

	smmu = devm_kzalloc(dev, sizeof(*smmu), GFP_KERNEL);
	if (!smmu) {
		dev_err(dev, "failed to allocate arm_smmu_device\n");
		return -ENOMEM;
	}
	smmu->dev = dev;

	if (dev->of_node) {
		ret = arm_smmu_device_dt_probe(pdev, smmu);
	} else {
		ret = arm_smmu_device_acpi_probe(pdev, smmu);
		if (ret == -ENODEV) {
			dev_err(dev, "failed to acpi_probe\n");
			return ret;
		}
	}

	/* Set bypass mode according to firmware probing result */
	smmu->bypass = !!ret;

	/* Base address */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("%s:platform_get_resource failed!\n", __func__);
		return -ENODEV;
	}

	smmu->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(smmu->base))
		return PTR_ERR(smmu->base);

	/* Interrupt lines */
	arm_smmu_set_irq(pdev, smmu);

	ret = arm_smmu_register_pm_notifier(smmu);
	if (ret) {
		dev_err(dev, "%s, register pm notifier err %d\n",
			__func__, ret);
		return ret;
	}

	/* Probe the h/w */
	ret = arm_smmu_device_hw_probe(smmu);
	if (ret) {
		pr_err("%s, hw probe failed!\n", __func__);
		return ret;
	}

	/* Initialise in-memory data structures */
	ret = arm_smmu_init_structures(smmu);
	if (ret) {
		pr_err("%s, structure init failed!\n", __func__);
		return ret;
	}

	arm_smmu_init_dfx(pdev, smmu);
	/* Record our private device structure */
	platform_set_drvdata(pdev, smmu);

	arm_smmu_save_smmuid(pdev, smmu);

	ret = arm_smmu_set_iommu(smmu, dev, res->start);
	if (ret) {
		dev_err(dev, "Failed to set iommu\n");
		return ret;
	}

	ret = arm_smmu_set_bus();
	if (ret) {
		dev_err(dev, "Failed to set iommu bus\n");
		return ret;
	}

	smmu->status = ARM_SMMU_DISABLE;

	return 0;
}

void arm_smmu_dmabuf_release_iommu(struct dma_buf *dmabuf)
{
	struct arm_v3_domain *dom = NULL;
	struct arm_v3_domain *tmp = NULL;

	list_for_each_entry_safe(dom, tmp, &smmuv3_domain_list, list) {
		__dmabuf_release_iommu(dmabuf, dom->smmu_domain.domain);
	}
}

static struct blocking_notifier_head *arm_smmu_get_evt_notifier(
	struct device *dev)
{
	struct arm_smmu_master_data *master = NULL;
	struct arm_smmu_device *smmu = NULL;
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	if (!dev->iommu_fwspec) {
		dev_err(dev, "%s, iommu_fwspec is null\n", __func__);
		return NULL;
	}

	master = dev->iommu_fwspec->iommu_priv;
	if (!master) {
		dev_err(dev, "%s, iommu_priv is null\n", __func__);
		return NULL;
	}

	smmu = master->smmu;
	if (!smmu) {
		dev_err(dev, "%s, smmu is null\n", __func__);
		return NULL;
	}

	list_for_each_entry(arm_smmu_group, &(smmu->iommu_groups), list) {
		if (arm_smmu_group->group != dev->iommu_group)
			continue;
		return &arm_smmu_group->notifier;
	}

	dev_err(dev, "%s, iommu_group 0x%p not found in smmu %s\n",
		__func__, dev->iommu_group, dev_name(smmu->dev));

	return NULL;
}

int arm_smmu_evt_register_notify(struct device *dev, struct notifier_block *n)
{
	struct blocking_notifier_head *notifier = NULL;

	if (!dev) {
		pr_err("%s, dev is null\n", __func__);
		return -EINVAL;
	}

	notifier = arm_smmu_get_evt_notifier(dev);
	if (!notifier) {
		dev_err(dev, "%s, evt notifier is null\n", __func__);
		return -ENOENT;
	}

	return blocking_notifier_chain_cond_register(notifier, n);
}

int arm_smmu_evt_unregister_notify(struct device *dev,
				struct notifier_block *n)
{
	struct blocking_notifier_head *notifier = NULL;

	if (!dev) {
		pr_err("%s, dev is null\n", __func__);
		return -EINVAL;
	}

	notifier = arm_smmu_get_evt_notifier(dev);
	if (!notifier) {
		dev_err(dev, "%s, evt notifier is null\n", __func__);
		return -ENOENT;
	}

	return blocking_notifier_chain_unregister(notifier, n);
}

static int arm_smmu_evt_notify(struct arm_smmu_device *smmu, u64 *evt, u8 len)
{
	u32 sid;
	unsigned long bytes_len;
	struct arm_smmu_iommu_group *arm_smmu_group = NULL;

	sid = (evt[0] >> EVTQ_0_STREAM_ID_SHIFT) & EVTQ_0_STREAM_ID_MASK;
	bytes_len = len * sizeof(u64);
	list_for_each_entry(arm_smmu_group, &(smmu->iommu_groups), list) {
		if (arm_smmu_group->sid != sid)
			continue;
		return  blocking_notifier_call_chain(&arm_smmu_group->notifier,
				bytes_len, evt);
	}

	dev_err(smmu->dev, "%s, sid %u has no iommu group\n", __func__, sid);
	return -ENOENT;
}

static int arm_smmu_device_remove(struct platform_device *pdev)
{
	unsigned long flags;
	struct arm_smmu_device *smmu = platform_get_drvdata(pdev);

	if (!smmu) {
		pr_err("%s, smmu is null\n", __func__);
		return 0;
	}

	spin_lock_irqsave(&smmu->tcu_lock, flags);
	if (smmu->status != ARM_SMMU_ENABLE) {
		dev_err(smmu->dev, "%s, status %d is not enable\n", __func__,
			smmu->status);
		goto out_unlock;
	}

	arm_smmu_device_disable(smmu);

	smmu->status = ARM_SMMU_DISABLE;
	pr_err("out %s\n", __func__);

out_unlock:
	spin_unlock_irqrestore(&smmu->tcu_lock, flags);

	return 0;
}

/* An SMMUv3 instance */
static const struct of_device_id arm_smmu_of_match[] = {
	{ .compatible = "hisi,smmu-v3", },
	{ },
};
MODULE_DEVICE_TABLE(of, arm_smmu_of_match);

static struct platform_driver arm_smmu_driver = {
	.driver	= {
		.name		= "smmu-v3",
		.of_match_table	= of_match_ptr(arm_smmu_of_match),
	},
	.probe	= arm_smmu_device_probe,
	.remove	= arm_smmu_device_remove,
};

static int __init hisi_smmuv3_group_init(void)
{
	int ret = 0;

	pr_err("into %s\n", __func__);
	ret = platform_driver_register(&arm_smmu_driver);
	if (ret) {
		pr_err("%s failed, ret:%d\n", __func__, ret);
		return -EINVAL;
	}
	return 0;
}
fs_initcall_sync(hisi_smmuv3_group_init);
late_initcall_sync(arm_smmu_rdr_init);

IOMMU_OF_DECLARE(arm_smmuv3, "arm,smmu-v3", NULL);

MODULE_DESCRIPTION("IOMMU API for ARM architected SMMUv3 implementations");
MODULE_AUTHOR("Will Deacon <will.deacon@arm.com>");
MODULE_LICENSE("GPL v2");
