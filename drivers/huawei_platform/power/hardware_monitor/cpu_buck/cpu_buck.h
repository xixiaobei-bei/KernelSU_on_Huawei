/*
 * cpu_buck.h
 *
 * cpu buck error monitor driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _CPU_BUCK_H_
#define _CPU_BUCK_H_

#define WEIGHT_10                  10
#define MAX_ERR_MSG_LEN            128
#define MAX_EXTRA_ERR_MSG_LEN      64

enum cpu_buck_error_number {
	/* for lp8758 */
	LP8758_INT_BUCK3 = 0,
	LP8758_INT_BUCK2,
	LP8758_INT_BUCK1,
	LP8758_INT_BUCK0,
	LP8758_TDIE_SD,
	LP8758_TDIE_WARN,
	LP8758_BUCK1_ILIM_INT,
	LP8758_BUCK0_PG_INT,
	LP8758_BUCK0_SC_INT,
	LP8758_BUCK0_ILIM_INT,

	/* for hi6422v100 */
	HI6422V100_VSYS_UV,
	HI6422V100_VSYS_OV,
	HI6422V100_OTMP_R,
	HI6422V100_OTMP150_D10R,
	HI6422V100_VBAT2P6_F,
	HI6422V100_VBAT2P3_2D,
	HI6422V100_BUCK34_SCP,
	HI6422V100_BUCK012_SCP,
	HI6422V100_OCPBUCK4,
	HI6422V100_OCPBUCK3,
	HI6422V100_OCPBUCK2,
	HI6422V100_OCPBUCK1,
	HI6422V100_OCPBUCK0,
	VBAT_OCPBUCK4,
	VBAT_OCPBUCK3,
	VBAT_OCPBUCK2,
	VBAT_OCPBUCK1,
	VBAT_OCPBUCK0,

	/* for hiv6422v200 */
	HI6422V200_VSYS_PWRON_D60UR, /* NP_IRQ1_RECORD_REG */
	HI6422V200_VSYS_OV_D200UR,
	HI6422V200_VSYS_PWROFF_ABS_2D,
	HI6422V200_THSD_OTMP125_D1MR,
	HI6422V200_THSD_OTMP140_D180UR,
	HI6422V200_BUCK3_OCP, /* NP_OCP_RECORD_REG */
	HI6422V200_BUCK2_OCP,
	HI6422V200_BUCK1_OCP,
	HI6422V200_BUCK0_OCP,
	HI6422V200_BUCK3_SCP, /* NP_SCP_RECORD_REG */
	HI6422V200_BUCK2_SCP,
	HI6422V200_BUCK1_SCP,
	HI6422V200_BUCK0_SCP,
};

enum cpu_buck_device_id {
	CPU_BUCK_HI6422V200_MAIN = 0,
	CPU_BUCK_HI6422V200_AUX,
	CPU_BUCK_HI6422V100,
	CPU_BUCK_LP8758,
};

struct cpu_buck_error_info {
	enum cpu_buck_error_number err_no;
	char err_mask;
	int reg_number;
	char err_msg[MAX_ERR_MSG_LEN];
};

struct cpu_buck_sample {
	struct cpu_buck_sample *cbs;
	struct cpu_buck_error_info *cbi;
	char *reg;
	int info_size;
	enum cpu_buck_device_id dev_id;
};

struct cpu_buck_device_info {
	struct device *dev;
	struct delayed_work monitor_work;
};

void cpu_buck_str_to_reg(char *str, char *reg, int size);
void cpu_buck_register(struct cpu_buck_sample *cbs);

#endif /* _CPU_BUCK_H_ */
