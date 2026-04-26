/*
 * npu_dpm.h
 *
 * about npu dpm
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_DMP_H
#define __NPU_DMP_H

#include <linux/types.h>

#define SOC_NPU_CRG_SIZE     0x1000
#define SOC_NPU_DPM_SIZE     0x1000
#define DATAREPORTING        0
#define BUSINESSFIT          1
#define DPM_REG_BASE_OFFSET  4

#if defined(CONFIG_DPM_HWMON_V1)
#define aicore_clk_en_addr(base)                     ((base) + 0x10)
#define aicore_clk_perrstdis1_addr(base)             ((base) + 0x30)
#define aicore_clk_dis_addr(base)                    ((base) + 0x14)
#define aicore_clk_stat_addr(base)                   ((base) + 0x1C)

#define dpmonitor_soft_sample_pulse_addr(base)       ((base) + 0x0E8)
#define dpmonitor_low_load0_addr(base)               ((base) + 0x014)
#define DPM_UNSET_OFFSET 0x30

#define AICORE_DPMONITOR_SIGNAL_MODE   0x0503

#define DPM_TABLE_LEN         64
#define DPM_REG_NUM           52
#define DPM_DVFS_REG_NUM      48
#define DPM_CLK_SLEEP_TIME    10
#define DPM_ENABLE_VAL        0x1

#define DPM_LOAD_NUM 16
#define DPM_LOAD_TYPE_NUM 3
#define DPM_PCLK_LOAD_NUM 4
#define NPU_READ_REG_NUM (DPM_LOAD_TYPE_NUM * DPM_LOAD_NUM)
#define NPU_PERI_VOTE_ID 62
#define EARLIEST_LAYER_VALUE 1

#define DPM_CLK_VAL BIT(16)
#define dpm_chk_clk(val) ((((val) & DPM_CLK_VAL) >> 16 != 0x1) ? 1 : 0)

int npu_dpm_fitting_coff(void);

int npu_dpm_get_peri_volt(int *curr_volt);
#elif defined(CONFIG_DPM_HWMON_V2)
#define NPU_DPM_NUM   4
#define DPM_ENERGY_NUM       1
#define TIMER_VAR_SIZE       2
#define NPU_DPM_PULSE_SLEEP  2

#define NPU_DPMONITOR_BUSY_CNT_REG_NUM          16
#define NPU_DPMONITOR_POWER_PARAM_REG_NUM       16
#define NPU_DPMONITOR_CONST_POWER_PARAM_REG_NUM 1
#define NPU_DPMONITOR_VOTE_PARAM_REG_NUM        8
#define NPU_DPMONITOR_FREQ_PARAM_REG_NUM        8

#define SOC_ACPU_PERI_CRG_SIZE      0x1000
#define SOC_NPU_PCR_SIZE            0x1000
#define SOC_NPU_SCTRL_SIZE          0x1000
#define SOC_NPU_AICORE_SIZE         0xFFFFF

#define NPU_DPM_PARA_NUM  (NPU_DPMONITOR_POWER_PARAM_REG_NUM +\
	NPU_DPMONITOR_CONST_POWER_PARAM_REG_NUM)

#define SIGNAL_LEVEL_MODE0        0xFFFF
#define SIGNAL_LEVEL_MODE1        0xFFFF
#define FREQ_PARAM_DEFAULT_VALUE  0x200

#define LITE_DPM0           0
#define LITE_DPM1           1
#define LITE_DPM2           2
#define LITE_DPM3           3

#ifndef write_uint64
#define write_uint64(ull_value, addr) \
	(*((volatile u64 *)((uintptr_t)(addr))) = (ull_value))
#endif

struct npu_dpm_addr {
	unsigned long long phy_addr;
	void __iomem *vir_addr;
};

bool npu_dpm_enable_flag(void);
#endif

void npu_dpm_init(void);

void npu_dpm_exit(void);

int npu_dpm_update_counter(void);

int npu_dpm_get_counter_for_fitting(int mode);

#endif /* __NPU_DMP_H */
