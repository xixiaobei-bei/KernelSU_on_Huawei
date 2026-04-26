/*
 * ras_edac.h
 *
 * RAS EDAC driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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
#ifndef __RAS_EDAC_H__
#define __RAS_EDAC_H__

#include <linux/types.h>
#include <asm/sysreg.h>
#include <linux/io.h>
#include <soc_fcm_namsto_interface.h>
#include <soc_acpu_baseaddr_interface.h>

/* RAS system registers */
#define DISR_EL1                S3_0_C12_C1_1
#define DISR_A_BIT              U(31)

#define ERRIDR_EL1              S3_0_C5_C3_0
#define ERRIDR_MASK             U(0xffff)

#define ERRSELR_EL1             S3_0_C5_C3_1

/* System register access to Standard Error Record registers */
#define ERXFR_EL1               S3_0_C5_C4_0
#define ERXCTLR_EL1             S3_0_C5_C4_1
#define ERXSTATUS_EL1           S3_0_C5_C4_2
#define ERXADDR_EL1             S3_0_C5_C4_3
#define ERXMISC0_EL1            S3_0_C5_C5_0
#define ERXMISC1_EL1            S3_0_C5_C5_1

/* System register error inject */
#define ERXPFGF_EL1             S3_0_C5_C4_4
#define ERXPFGCTL_EL1           S3_0_C5_C4_5
#define ERXPFGCDN_EL1           S3_0_C5_C4_6

/* Deimos system register errori inject */
#define DEIMOS_ERXPFGF_EL1	S3_0_C15_C2_0
#define DEIMOS_ERXPFGCTL_EL1	S3_0_C15_C2_1
#define DEIMOS_ERXPFGCDN_EL1	S3_0_C15_C2_2

#define ERXCTLR_ED_BIT          (U(1) << 0)
#define ERXCTLR_UE_BIT          (U(1) << 4)

#define ERXPFGCTL_UC_BIT        (U(1) << 1)
#define ERXPFGCTL_UEU_BIT       (U(1) << 2)
#define ERXPFGCTL_CDEN_BIT      (U(1) << 31)

/* L3 memory map register */
#define L3_ERRFR	SOC_FCM_NAMSTO_ERR1FR_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)
#define L3_ERRCTRL	SOC_FCM_NAMSTO_ERR1CTRL_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)
#define L3_ERRMISC0	SOC_FCM_NAMSTO_ERR1MISC0_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)
#define L3_ERRMISC1	SOC_FCM_NAMSTO_ERR1MISC1_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)
#define L3_ERRSTATUS	SOC_FCM_NAMSTO_ERR1STATUS_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)

/* L3 memory map err inject */
#define L3_ERRPFGF	SOC_FCM_NAMSTO_CPUPFGCTL_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)
#define L3_ERRPFGCTL	SOC_FCM_NAMSTO_CPUPFGCTL_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)
#define L3_ERRPFGCDN	SOC_FCM_NAMSTO_CPUPFGCDN_ADDR(SOC_ACPU_namtso_cfg_BASE_ADDR)

#define ERR_STATUS_V_SHIFT	30
#define ERR_STATUS_V_MASK	0x1
#define ERR_STATUS_SERR_SHIFT	0
#define ERR_STATUS_SERR_MASK	0xff
#define ERR_STATUS_GET_FIELD(_status, _field) \
	(((_status) >> ERR_STATUS_ ##_field ##_SHIFT) & ERR_STATUS_ ##_field ##_MASK)

#define _DEFINE_SYSREG_READ_FUNC(_name, _reg_name)              \
static inline u64 read_ ## _name(void)				\
{								\
	return read_sysreg(_reg_name);				\
}

#define _DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)		\
static inline void write_ ## _name(u64 v)			\
{								\
	write_sysreg(v, _reg_name);				\
}

static inline u64 read_mem_data(u64 reg_address)
{
	u64 val;
	void __iomem *reg_data = NULL;

	reg_data = ioremap(reg_address, 0x8);
	val = readq(reg_data);
	iounmap(reg_data);
	return val;
}

static inline void write_mem_data(u64 v, u64 reg_address)
{
	void __iomem *reg_data = NULL;

	reg_data = ioremap(reg_address, 0x8);
	writeq(v, reg_data);
	iounmap(reg_data);
}

/* Define read & write function for renamed system register */
#define DEFINE_RENAME_SYSREG_RW_FUNCS(_name, _reg_name)		\
	_DEFINE_SYSREG_READ_FUNC(_name, _reg_name)		\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)

/* Define read function for renamed system register */
#define DEFINE_RENAME_SYSREG_READ_FUNC(_name, _reg_name)	\
	_DEFINE_SYSREG_READ_FUNC(_name, _reg_name)

/* Define write function for renamed system register */
#define DEFINE_RENAME_SYSREG_WRITE_FUNC(_name, _reg_name)	\
	_DEFINE_SYSREG_WRITE_FUNC(_name, _reg_name)

DEFINE_RENAME_SYSREG_READ_FUNC(erridr_el1, ERRIDR_EL1)
DEFINE_RENAME_SYSREG_WRITE_FUNC(errselr_el1, ERRSELR_EL1)

DEFINE_RENAME_SYSREG_READ_FUNC(erxfr_el1, ERXFR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxctlr_el1, ERXCTLR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxstatus_el1, ERXSTATUS_EL1)
DEFINE_RENAME_SYSREG_READ_FUNC(erxaddr_el1, ERXADDR_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxmisc0_el1, ERXMISC0_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxmisc1_el1, ERXMISC1_EL1)

/* Error inject */
DEFINE_RENAME_SYSREG_READ_FUNC(erxpfgf_el1, ERXPFGF_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxpfgctl_el1, ERXPFGCTL_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(erxpfgcdn_el1, ERXPFGCDN_EL1)

/* Deimos error inject */
DEFINE_RENAME_SYSREG_READ_FUNC(deimos_erxpfgf_el1, DEIMOS_ERXPFGF_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(deimos_erxpfgctl_el1, DEIMOS_ERXPFGCTL_EL1)
DEFINE_RENAME_SYSREG_RW_FUNCS(deimos_erxpfgcdn_el1, DEIMOS_ERXPFGCDN_EL1)

/* define cpu part number */
#define CPU_PARTNUM_LINX		0xD02
#define CPU_PARTNUM_ANANKE		0xD05
#define CPU_PARTNUM_HERCULES	0xD41
#define CPU_PARTNUM_HERA		0xD44
#define CPU_PARTNUM_DEIMOS		0xD0D
#define CPU_PARTNUM_KLEIN		0xD46

/* custom define part_num */
#define CPU_L2_CLUSTER		0x100
#define CPU_L3_MEM			0x200
#define L3_BUS_ERROR		0x300

/* errmisc reset value */
#define IFU_ERRXMISC		0x7fff00000000
#define L2C_ERRXMISC		0x7fff00000000
#define MMU_ERRXMISC		0x7f00000000
#define LSU_ERRXMISC		0x7fff00000000
#define ARM_ERRXMISC		0x7F7F00000000

#define IFU_RAS_ERROR_LIST(X)	\
	/* Name, SERR, ISA Desc */	\
	X(IFU, 0x6, "data value from associative memory(ecc error on cache data)") 	\
	X(IFU, 0x7, "address/control value from associative memory(ecc error on cache tag)")

#define LSU_RAS_ERROR_LIST(X)	\
	/* Name, SERR, ISA Desc */	\
	X(LSU, 0x1, "No Error")			\
	X(LSU, 0x5, "Internal data path. For example, parity on ALU result")	\
	X(LSU, 0x6, "data value from associative memory(ecc error on cache data)")	\
	X(LSU, 0x7, "address/control value from associative memory(ecc error on cache tag)")	\
	X(LSU, 0xA, "Data value from producer. For example, parity error on write data bus")

#define MMU_RAS_ERROR_LIST(X)	\
	/* Name, SERR, ISA Desc */    \
	X(MMU, 0x8, "Data value from a TLB. For example, ECC error on TLB data")	\
	X(MMU, 0x9, "Address/control value(s) from a TLB. For example, ECC error on TLB tag")

#define L2C_RAS_ERROR_LIST(X)	\
	/* Name, ERR_CTRL, SERR, ISA Desc */    \
	X(L2C, 0x6, "data value from associative memory(ecc error on cache data )") \
	X(L2C, 0x7, "address/control value from associative memory(ecc error on cache tag)")

/* demios serr type */
#define L1_L2_RAS_ERROR_LIST(X)	\
	/* Name, SERR, ISA Desc */    \
	X(L1_l2, 0, "No error")	\
	X(L1_L2, 0x1, "Errors due to fault injection")	\
	X(L1_L2, 0x2, "ECC error from internal data buffer")	\
	X(L1_L2, 0x6, "ECC error on cache data RAM.")		\
	X(L1_L2, 0x7, "ECC error on cache tag or dirty RAM.")	\
	X(L1_L2, 0x8, "Parity error on TLB data RAM.")	\
	X(L1_L2, 0x12, "Error response for a cache copyback.")  \
	X(L1_L2, 0x15, "Deferred error from slave not supported at the consumer.")

#define L1C_RAS_ERROR_LIST(X)	\
	/* Name, SERR, ISA Desc */	\
	X(L1C, 0x6, "Data value from associative memory.")	\
	X(L1C, 0x7, "Address/control value from associative memory.")	\
	X(L1C, 0x8, "Data value from a TLB.")		\
	X(L1C, 0xC, "Data value from (non-associative) external memory")	\
	X(L1C, 0x12, "Error response from slave.")	\
	X(L1C, 0x15, "Deferred error from slave not supported at master.")

#define L2_CLUSTER_RAS_ERROR_LIST(X)						\
	/* Name, SERR, ISA Desc */	\
	X(L2_CLUSTER, 0x6, "Data value from associative memory.")	\
	X(L2_CLUSTER, 0x7, "Address/control value from associative memory.")	\
	X(L2_CLUSTER, 0x8, "Data value from a TLB.")	\
	X(L2_CLUSTER, 0xC, "Data value from (non-associative) external memory")	\
	X(L2_CLUSTER, 0x12, "Error response from slave.")	\
	X(L2_CLUSTER, 0x15, "Deferred error from slave not supported at master.")

#define L3_RAS_ERROR_LIST(X)					\
	/* Name, SERR, ISA Desc */				\
	X(L3, 0x0, "No error")					\
	X(L3, 0x2, "ECC error from internal data buffer.")	\
	X(L3, 0x6, "ECC error on cache data RAM.")		\
	X(L3, 0x7, "ECC error on cache tag or dirty RAM.")	\
	X(L3, 0x12, "Bus error")

#define DEFINE_ONE_RAS_AUX_DATA(node, misc_data)		\
	{							\
		.name = #node,					\
		.error_records = node##_uncorr_ras_errors,	\
		.err_misc = (misc_data)				\
	},

#define DEFINE_ONE_RAS_NODE(node, misc_data)					\
static const struct ras_error node##_uncorr_ras_errors[] = {		\
	node##_RAS_ERROR_LIST(DEFINE_ONE_RAS_ERROR_MSG)			\
	{								\
		NULL,							\
		0U							\
	},								\
};

#define DEFINE_ONE_RAS_ERROR_MSG(unit, ierr, msg)			\
	{								\
		.error_msg = (msg),					\
		.serr_idx = (ierr)					\
	},

#define LINX_CORE_RAS_NODE_LIST(X)			\
	X(IFU, IFU_ERRXMISC)				\
	X(LSU, LSU_ERRXMISC)				\
	X(MMU, MMU_ERRXMISC)				\
	X(L2C, L2C_ERRXMISC)

#define DEMIOS_CORE_RAS_NODE_LIST(X)	X(L1_L2, ARM_ERRXMISC)

#define KLEIN_CORE_RAS_NODE_LIST(X)	X(L1C, ARM_ERRXMISC)

#define L2_CLUSTER_RAS_NODE_LIST(X)	X(L2_CLUSTER, ARM_ERRXMISC)

#define L3_MEM_RAS_NODE_LIST(X)		X(L3, ARM_ERRXMISC)

#define LINX_CORE_RAS_GROUP_NODES	LINX_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#define KLEIN_CORE_RAS_GROUP_NODES	KLEIN_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#define DEMIOS_CORE_RAS_GROUP_NODES	DEMIOS_CORE_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

#define L2_CLUSTER_RAS_GROUP_NODES	L2_CLUSTER_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)
#define L3_MEM_RAS_GROUP_NODES		L3_MEM_RAS_NODE_LIST(DEFINE_ONE_RAS_AUX_DATA)

struct ras_error {
	const char *error_msg;
	/* SERR(bits[7:0]) from ERR<n>STATUS */
	u8 serr_idx;
};

struct ras_aux_data {
	const char *name;
	const struct ras_error *error_records;
	u64 err_misc;
};

struct err_record_mapping {
	struct err_record_info *err_records;
	size_t num_err_records;
};

#define REGISTER_ERR_RECORD_INFO(_records) \
	const struct err_record_mapping err_record_mappings = { \
		.err_records = (_records), \
		.num_err_records = ARRAY_SIZE(_records), \
	}

/* Error record info iterator */
#define for_each_err_record_info(_i, _info) \
	for ((_i) = 0, (_info) = err_record_mappings.err_records; \
	    (_i) < err_record_mappings.num_err_records; \
	    (_i)++, (_info)++)

#define ADD_ONE_ERR_GROUP(part_num, errselr_start, group)               \
	{                                                               \
		.node_number = (part_num),                              \
		.sysreg.start_idx = (errselr_start),                    \
		.sysreg.num_idx = ((u32)ARRAY_SIZE((group))),           \
		.aux_data = (group)                                     \
	}

/* Define EDAC data structure */
struct erp_cpu_drvdata {
	int *irq;
	int irq_num;
};

struct erp_l2_drvdata {
	struct	erp_l2_drvdata *l2_drv_next;
	int *irq;
	int irq_num;
	cpumask_t l2_affinity_cpu;
};

struct erp_drvdata {
	struct edac_device_ctl_info *edev_ctl;
	struct erp_cpu_drvdata __percpu *cpu_drv;

	/* Head node for L2 irq list */
	struct erp_l2_drvdata *l2_drv;
	cpumask_t supported_cpus;
	cpumask_t active_irqs;
	int l3_irq;
	struct hlist_node node;
};

struct err_record_info {
	unsigned int node_number;
	struct ras_aux_data *aux_data;
	struct {
		unsigned int start_idx;
		unsigned int num_idx;
	} sysreg;
};

struct errors_edac {
	const char * const msg;
	void (*func)(struct edac_device_ctl_info *edac_dev,
		     int inst_nr, int block_nr, const char *msg);
	void (*mntn_handler)(struct edac_device_ctl_info *edac_dev,
			     int inst_nr, int block_nr,
			     u64 errxstatus, u64 errxmisc);
};

#endif /* __RAS_EDAC_H__ */
