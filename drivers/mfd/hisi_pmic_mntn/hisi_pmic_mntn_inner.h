/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 *
 * pmic_mntn_inner.h
 *
 * Head file for PMU DRIVER
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __PMIC_MNTN_H__
#define __PMIC_MNTN_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdesc.h>
#include <linux/irqnr.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/rtc.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/hardirq.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/of.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/string.h>
#include <linux/regulator/consumer.h>
#include <asm/uaccess.h>
#include <linux/hisi/util.h>
#include <soc_sctrl_interface.h>
#include <linux/mfd/hisi_pmic.h>

#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_hisi_platform.h>
#include <linux/hisi/rdr_pub.h>
#else
#include <linux/reboot.h>
#endif

#define set_reg_bit(reg_value, shift, mask, value)                             \
	((reg_value) = ((reg_value) & (~((((unsigned int)1 << (mask)) - 1) << (shift)))) |  \
		     ((unsigned char)((value) &  (((unsigned int)1 << (mask)) - 1)) << (shift)))

#define get_reg_bit(reg_value, shift, mask)                                    \
	(((reg_value) & ((((unsigned int)1 << (mask)) - 1) << (shift))) >> (shift))

#define PMIC_PRINT_BUF_SIZE     (4*1024)
#define PMIC_DTS_ATTR_LEN       64

#define PMIC_OCP_NONE           0
#define PMIC_OCP_HANDLED        1

#define SUPPLY_SD       "ldo16"
#define SUPPLY_SD_IO     "ldo9"
#define SUPPLY_SPEAKER  "classd"

#define INVALID_REG_ADDR        0xDEAD
#define CLOSE_REGULATOR_CONTROL 0x00
#define OPEN_REGULATOR_CONTROL  0x01
#define PMU_REG_WIDTH           8

#define PATH_BATTERY_CAPACITY "/sys/class/power_supply/Battery/capacity"

/* PMU0: 6421 or 6555 or 6556; PMU1: 6423 */
#define PMU0_MNTN_COMP_STR "hisilicon-hisi-pmic-mntn-spmi"
#define PMU1_MNTN_COMP_STR "hisi-sub-pmic-mntn-spmi"

enum pmic_status_type_enum {
	PMIC_HEALTH_STATUS_NORMAL,
	PMIC_HEALTH_OCP_EXCH_HAPPENED,
	PMIC_HEALTH_RECORD_EXCH_HAPPENED,
};

struct pmic_exch_reg {
	unsigned int inacceptable_event;
	unsigned int check_ocp_nofify;
	unsigned int check_ocp_reset;
	char **event_bit_name;
	unsigned int *event_ops_reg;
	unsigned int *check_ocp_num;
	unsigned int *check_count;
};

struct pmic_reg_ctrl {
	unsigned int addr;
	unsigned int shift;
	unsigned int mask;
};

struct special_ocp_attr {
	char *power_name;
	pmic_ocp_callback handler;
	struct special_ocp_attr *next;
};

struct battery_capacity_check_info {
	unsigned int bat_cap_threshold;
	unsigned int check_interval;
};

struct pmic_vsys_surge {
	/* vsys surge event record */
	unsigned int vsys_surge_reg;
	unsigned int vsys_surge_reg_mask;
#if defined CONFIG_HUAWEI_DSM
	unsigned int vsys_surge_dmd_offset;
#endif
};

struct pmic_mntn_desc {
	unsigned int health;
	unsigned int data_width;
	struct kobject *mntn_kobj;
	struct device *dev;
	struct regmap *regmap;

	/* Over Temperature Handle */
	int otmp_irq;
	unsigned int otmp_thshd_val;
	struct pmic_reg_ctrl otmp_thshd_reg;
	struct workqueue_struct *otmp_wq;
	struct work_struct otmp_wk;
	unsigned int otmp_hreset_pwrdown_flag;
	unsigned int otmp_hreset_pwrdown_val;
	struct pmic_reg_ctrl otmp_hreset_pwrdown_reg;

	/* SMPL Function */
	unsigned int smpl_en_val;
	struct pmic_reg_ctrl smpl_en_reg;
	unsigned int smpl_tm_val;
	struct pmic_reg_ctrl smpl_tm_reg;

#if defined(CONFIG_HUAWEI_VSYS_PWROFF_ABS_PD)
	/* vsys_pwroff_abs_pd */
	unsigned int vsys_pwroff_abs_pd_en_val;
	unsigned int vsys_pwroff_abs_pd_dis_val;
	struct pmic_reg_ctrl vsys_pwroff_abs_pd_tm_reg;
#endif

	/* SCP/OCP Handle */
	int ocp_irq;
	unsigned int ocp_reg_n;
	unsigned int *ocp_regs;
#if defined(CONFIG_HUAWEI_DSM)
	unsigned int ocp_error_dmd_offset_n;
	unsigned int *ocp_error_dmd_offset;
	unsigned int *dsm_record_regs_mask;
	unsigned int *dsm_ocp_reset_mask;
#endif
	unsigned int *ocp_event_buff;
	struct pmic_exch_reg *ocp_exch_desc;
	struct workqueue_struct *ocp_wq;
	struct work_struct ocp_wk;
	struct special_ocp_attr *ocp_list_head;
	struct special_ocp_attr *ocp_list_tail;
	struct notifier_block usb_nb;

	/* uv_mntn Handle */
	int uv_irq;
	struct delayed_work uv_mntn_delayed_work;
	struct battery_capacity_check_info bat_cap_info;

	/* Event Record */
	unsigned int record_reg_n;
	unsigned int *record_regs;
	unsigned int *record_event_buff;
	struct pmic_exch_reg *record_exch_desc;
	struct pmic_vsys_surge record_vsys_surge;

	/* Log */
	char *init_log_show;
	char *irq_log_show;

	/* ldo buck ocp mold ctrl */
	int ocp_mold_switch;
	unsigned int ocp_ctrl_n;
	unsigned int *ocp_ctrl_regs;
	unsigned int *ocp_ctrl_val;

	/* pmu record key register */
	unsigned int pmu_record_reg_n;
	unsigned int *pmu_record_regs;
};

int pmic_mntn_drv_init(struct pmic_mntn_desc *pmic_mntn);
void pmic_vsys_surge_init(struct pmic_mntn_desc *pmic_mntn);
void pmic_mntn_drv_deinit(void);

#endif
