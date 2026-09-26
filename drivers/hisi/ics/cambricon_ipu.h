/*
 * Generic driver head file for the cambricon ipu device.
 *
 * Copyright (C) 2016 Cambricon Limited
 *
 * Licensed under the GPL v2 or later.
 */
#ifndef _CAMBRICON_IPU_H
#define _CAMBRICON_IPU_H

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/wakelock.h>
#include <linux/timer.h>
#include <linux/kthread.h>

#include "ipu_smmu_drv.h"
#include "ipu_clock.h"

struct irq_reg_offset {
	unsigned int ics_irq_base_addr;
	unsigned int ics_irq_mask_ns;
	unsigned int ics_irq_clr_ns;
};

struct ics_noc_bus_reg_offset {
	unsigned int base_addr;
	unsigned int qos_type;
	unsigned int factor;
	unsigned int saturation;
	unsigned int qos_extcontrol;
};

struct pmctrl_reg_offset {
	unsigned int base_addr;
	unsigned int noc_power_idle_req;
	unsigned int noc_power_idle_ack;
	unsigned int noc_power_idle_stat;
};

struct pctrl_reg_offset {
	unsigned int base_addr;
	unsigned int peri_stat3;
};

struct media2_reg_offset {
	unsigned int base_addr;
	unsigned int peren0;
	unsigned int perdis0;
	unsigned int perclken0;
	unsigned int perstat0;
	unsigned int perrsten0;
	unsigned int perrstdis0;
	unsigned int perrststat0;
};

struct peri_reg_offset {
	unsigned int base_addr;
	unsigned int clkdiv8;
	unsigned int clkdiv18;
	unsigned int perpwrstat;
	unsigned int perpwrack;
	unsigned int peristat7;
};

enum ipu_reset_trategy {
	IPU_RESET_UNSUPPORT = 0,
	IPU_RESET_BY_CONFIG_NOC_BUS,
	IPU_SOFT_RESET,
};

enum ipu_bandwidth_lmt_trategy {
	IPU_BANDWIDTH_LMT_UNSUPPORT = 0,
	IPU_BANDWIDTH_LMT_BY_QOS,
	IPU_BANDWIDTH_LMT_BY_RW_OSD,
};

struct ics_feature_tree {
	bool finish_irq_expand_ns;
	bool finish_irq_expand_p;
	bool finish_irq_expand_s;
	bool finish_irq_to_hifi;
	bool finish_irq_to_ivp;
	bool finish_irq_to_isp;
	bool finish_irq_to_lpm3;
	bool finish_irq_to_iocmu;
	bool smmu_port_select;
	bool level1_irq;
	bool performance_monitor;
	bool wr_qword;
	bool soft_watchdog_enable;
	bool lpm3_set_vcodecbus;
	bool smmu_mstr_hardware_start;
	unsigned int ipu_reset_when_in_error;
	unsigned int ipu_bandwidth_lmt;
};
typedef int Priority;

/* the ipu task type */
typedef enum taskType {
	IPU_TASK_ENUM_START = 0,
	IPU_NONE_TASK = IPU_TASK_ENUM_START,
	/*First*/
	IPU_COMPUTE_TASK,
	IPU_SYNC_TASK,
	//Add new to here!
	IPU_TASK_ENUM_END
} taskType_t;

typedef enum taskFlag{
	IPU_TASK_FLAG_START = 0,
	IPU_TASK_FLAG_NONE = IPU_TASK_FLAG_START,
	/*First*/
	IPU_TASK_FLAG_SYNC_WAITING,
	IPU_TASK_FLAG_SYNC_DONE,
	//Add new to here!
	IPU_TASK_FLAG_END
} taskFlag_t;

struct ipu_wtd {
	bool enable;
	int status;
	struct mutex timer_mutex;
	struct timer_list timer;
	struct task_struct *task;
	struct semaphore sem;
};

/* the ipu task element */
typedef struct taskStruct {
	taskType_t taskType;//define different cmd type
	taskFlag_t* ptaskFlag;//use to mark task status
	unsigned long offchipInstAddr;
	unsigned long taskId;
	Priority prior;
} taskElement;

#define BOOT_INST_SIZE (64)
#define BOOT_INST_NUMBER (4)

/* this struct stores the temp data of boot instrument for IPU initialization after power-up */
struct boot_inst_set {
	/* mutex */
	struct mutex boot_mutex;

	/* store the value from IOCTL WRITE when IPU is off */
	unsigned int ipu_access_ddr_addr;

	/* boot-instrument data */
	unsigned char boot_inst[BOOT_INST_SIZE * BOOT_INST_NUMBER];

	/* size of boot-inst data */
	unsigned int boot_inst_size;

	/* bool flag to record whether access_ddr_addr is recorded */
	bool access_ddr_addr_is_config;

	/* bool flag to record whether boot_inst is recorded */
	bool boot_inst_recorded_is_config;
};

/* cambricon ipu private data */
struct cambricon_ipu_private
{
	const char *name;
	unsigned int irq;
	bool ipu_device_opened;
	bool ipu_power_up;
	struct wake_lock wakelock;
	struct mutex power_mutex;
	struct mutex open_mutex;
	struct mutex bandwidth_lmt_mutex;
	struct boot_inst_set boot_inst_set;

	struct smmu_statistic stat;
	bool smmu_stat_en;
	struct mutex stat_mutex;

	/* config reg addr */
	unsigned int config_reg_length;
	phys_addr_t config_reg_phys_addr;
	void __iomem *config_reg_virt_addr;

	/* inst reg addr */
	unsigned int inst_ram_size;
	phys_addr_t inst_ram_phys_addr;
	void __iomem *inst_ram_virt_addr;

	/* ioremap addr */
	void __iomem *ics_irq_io_addr;
	void __iomem *noc_bus_io_addr;
	void __iomem *pmctrl_io_addr;
	void __iomem *pctrl_io_addr;
	void __iomem *media2_io_addr;
	void __iomem *peri_io_addr;

	struct semaphore config_reg_sem;
	struct semaphore inst_ram_sem;
	struct semaphore llseek_sem;
	struct semaphore task_fifo_sem;

	/* char device */
	struct cdev	cdev; /* ipu char device */

	/* platform device resource */
	struct resource *inst_mem, *cfg_mem;
	struct regulator *vipu_ip;

	/* clock */
	struct ics_clock clk;

	struct mutex reset_mutex;
	unsigned long reset_va;
	unsigned long smmu_ttbr0;
	void *smmu_rw_err_virtual_addr;

	struct irq_reg_offset irq_reg_offset;
	struct ics_noc_bus_reg_offset ics_noc_bus_reg_offset;
	struct pmctrl_reg_offset pmctrl_reg_offset;
	struct pctrl_reg_offset pctrl_reg_offset;
	struct media2_reg_offset media2_reg_offset;
	struct peri_reg_offset peri_reg_offset;

	struct ics_feature_tree feature_tree;

	struct ipu_wtd reset_wtd;

	unsigned long computed_task_cnt;

};

#endif
