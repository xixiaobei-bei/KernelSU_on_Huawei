/*
 * npu_platform_register.h
 *
 * about npu platform register
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
#ifndef __DEVDRV_PLATFORM_REGISTER_H
#define __DEVDRV_PLATFORM_REGISTER_H

#define SOC_HARDWARE_VERSION (DEVDRV_PLATFORM_LITE_ORLANDO)

#include "soc_npu_ts_sysctrl_reg_offset.h"
#include "soc_npu_tscpu_cfg_reg_offset.h"

#define DEVDRV_SC_TESTREG0_OFFSET \
		(SOC_TS_SYSCTRL_SC_TESTREG0_REG - SOC_TS_SYSCTRL_BASE)

#define DEVDRV_SC_TESTREG1_OFFSET \
		(SOC_TS_SYSCTRL_SC_TESTREG1_REG - SOC_TS_SYSCTRL_BASE)

#define TS_BOOT_STATUS_OBSERVATION_REG \
		(SOC_TS_SYSCTRL_SC_TESTREG6_REG - SOC_TS_SYSCTRL_BASE)

#define DEVDRV_SC_TESTREG8_OFFSET \
		(SOC_TS_SYSCTRL_SC_TESTREG8_REG - SOC_TS_SYSCTRL_BASE)

#define DEVDRV_CFG_STAT0_OFFSET \
		(SOC_NPU_TSCPU_CFG_TS_CPU_STAT0_REG - SOC_TS_SYSCTRL_BASE)

#define SOC_NPU0_MID  0x70
#define SOC_NPU1_MID  0x71

#define SOC_CRGPERIPH_NPUCRG_STAT_ADDR(base)   ((base) + (0x98UL))
#define SOC_CRGPERIPH_NPUCRG_IP_RST_BIT_OFFSET_START  25

#endif
