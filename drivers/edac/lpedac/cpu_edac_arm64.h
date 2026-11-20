/*
 * cpu_edac_arm64.h
 *
 * CPU Cache EDAC driver
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef __CPU_EDAC_ARM64_H__
#define __CPU_EDAC_ARM64_H__

#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/rdr_hisi_platform.h>
#include <mntn_subtype_exception.h>

#ifdef CONFIG_CPU_EDAC_ARM64_PANIC_ON_UE
#define ARM64_ERP_PANIC_ON_UE	1
#else
#define ARM64_ERP_PANIC_ON_UE	0
#endif

/* Indicates the level that contained the error of MISC0 for DSU */
#define L3_LEVEL	0x2

/* CPU partnum */
#define CPU_PARTNUM_ANANKE		0xD05
#define CPU_PARTNUM_DEIMOS		0xD0D

/* Indicates the level that contained the error of MISC0 for ANANKE */
#define L1_BIT_ANANKE	0x0
#define L2_BIT_ANANKE	0x1

/* Indicates the unit which detected the error of MISC0 for NON-ANANKE */
#define L1_IC_BIT	0x1
#define L1_DC_BIT	0x4
#define L2_BIT		0x8
#define L2_TLB_BIT	0x2

/* Level of Cache */
#define L1	0x0
#define L2	0x1
#define L3	0x2

#define ERRXSTATUS_VALID(a)	(((a) >> 30) & 0x1)
#define ERRXSTATUS_UE(a)	(((a) >> 29) & 0x1)
#define ERRXSTATUS_SERR(a)	((a) & 0xFF)

#define ERRXMISC_LVL_ANANKE(a)	(((a) >> 1) & 0x7)
#define ERRXMISC_LVL_DEIMOS(a)	((a) & 0xF)

#define ERRXMISC_L3_LVL(a)		(((a) >> 1) & 0x7)
#define ERRXMISC_L1_L2_LVL(a)	((a) & 0xF)
#define ERRXMISC_WAY(a)		(((a) >> 28) & 0xF)

/* error code ID */
#define L1_CE	0
#define L1_UE	1
#define L2_CE	2
#define L2_UE	3
#define L3_CE	4
#define L3_UE	5

enum serr {
	NO_ERROR = 0,
	FAULT_INJECTION = 0x1,
	INTERNAL_DATA_BUFFER = 0x2,
	CACHE_DATA_RAM = 0x6,
	CACHE_TAG_DIRTY_RAM = 0x7,
	TLB_DATA_RAM = 0x8,
	TLB_TAG_RAM = 0x9,
	CACHE_COPYBACK = 0X12,
	BUS_ERROR = 0X12,
	DEFERRED_ERROR = 0x15,
	INVALID_VAL = 0xFF,
};

struct erp_cpu_drvdata {
	struct erp_drvdata *drvdata;
	int irq;
};

struct erp_drvdata {
	struct edac_device_ctl_info *edev_ctl;
	struct erp_cpu_drvdata __percpu *cpu_drv;
	cpumask_t supported_cpus;
	cpumask_t active_irqs;
	int l3_irq;
	struct notifier_block nb_pm;
	struct hlist_node node;
};

struct errors_edac {
	const char * const msg;
	void (*func)(struct edac_device_ctl_info *edac_dev,
		     int inst_nr, int block_nr, const char *msg);
	void (*mntn_handler)(struct edac_device_ctl_info *edac_dev,
			     int inst_nr, int block_nr,
			     u64 errxstatus, u64 errxmisc);
};

static void edac_mntn_hanlde_ce(struct edac_device_ctl_info *edac_dev,
				     int inst_nr, int block_nr,
				     u64 errxstatus, u64 errxmisc);

static void edac_mntn_hanlde_ue(struct edac_device_ctl_info *edac_dev,
				     int inst_nr, int block_nr,
				     u64 errxstatus, u64 errxmisc);

#define DEFINE_CPU_EDAC_EINFO(ERR_TYPE)					\
	{								\
		.e_modid            = (u32)MODID_AP_S_##ERR_TYPE,	\
		.e_modid_end        = (u32)MODID_AP_S_##ERR_TYPE,	\
		.e_process_priority = RDR_ERR,				\
		.e_reboot_priority  = RDR_REBOOT_NOW,			\
		.e_notify_core_mask = RDR_AP,				\
		.e_reset_core_mask  = RDR_AP,				\
		.e_from_core        = RDR_AP,				\
		.e_reentrant        = (u32)RDR_REENTRANT_DISALLOW,	\
		.e_exce_type        = (u32)AP_S_PANIC,			\
		.e_exce_subtype     = HI_APPANIC_##ERR_TYPE,		\
		.e_upload_flag      = (u32)RDR_UPLOAD_YES,		\
		.e_from_module      = "ap",				\
		.e_desc             = "ap",				\
	},

static struct rdr_exception_info_s cpu_edac_einfo[] = {
	DEFINE_CPU_EDAC_EINFO(CPU0_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU0_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU0_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU0_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU1_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU1_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU1_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU1_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU2_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU2_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU2_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU2_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU3_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU3_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU3_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU3_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU4_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU4_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU4_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU4_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU5_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU5_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU5_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU5_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU6_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU6_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU6_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU6_L2_UE)
	DEFINE_CPU_EDAC_EINFO(CPU7_L1_CE)
	DEFINE_CPU_EDAC_EINFO(CPU7_L1_UE)
	DEFINE_CPU_EDAC_EINFO(CPU7_L2_CE)
	DEFINE_CPU_EDAC_EINFO(CPU7_L2_UE)
	DEFINE_CPU_EDAC_EINFO(L3_CE)
	DEFINE_CPU_EDAC_EINFO(L3_UE)
};

#endif /* __CPU_EDAC_ARM64_H__ */
