/*
 * npu_dpm.h
 *
 * about npu dpm
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
#ifndef __NPU_DMP_H
#define __NPU_DMP_H

#define AICORE_CLK_EN_ADDR(base)                     ((base) + (0x10))
#define AICORE_CLK_PERRSTDIS1_ADDR(base)             ((base) + (0x30))
#define AICORE_CLK_DIS_ADDR(base)                    ((base) + (0x14))
#define AICORE_CLK_STAT_ADDR(base)                   ((base) + (0x1C))

#define DPMONITOR_SOFT_SAMPLE_PULSE_ADDR(base)       ((base) + (0x0E8))
#define DPMONITOR_LOW_LOAD0_ADDR(base)               ((base) + (0x014))
#define DPM_UNSET_OFFSET 0x30

#define AICORE_DPMONITOR_SIGNAL_MODE   0x0503
#define SOC_NPU_CRG_SIZE 0x1000
#define SOC_NPU_DPM_SIZE 0x1000

#define DPM_TABLE_LEN         64
#define DPM_REG_NUM           52
#define DPM_DVFS_REG_NUM      48
#define DPM_REG_BASE_OFFSET   4
#define DPM_CLK_SLEEP_TIME    10
#define DPM_ENABLE_VAL        0x1

#define DPM_CLK_VAL BIT(16)
#define DPM_CHk_CLK(val) ((((val) & DPM_CLK_VAL) >> 16 != 0x1) ? 1 : 0)

void npu_dpm_init(void);

void npu_dpm_exit(void);

#endif /* __NPU_DMP_H */
