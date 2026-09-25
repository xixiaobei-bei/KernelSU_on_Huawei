/*
 * npu_atf_subsys.c
 *
 * about npu atf subsys
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
#ifndef _NPU_ATF_H_
#define _NPU_ATF_H_

/* 1dd must be the same */
#define NPU_SLV_SECMODE                 0xC501dd00
#define NPU_MST_SECMODE                 0xC501dd01
#define NPU_START_SECMODE               0xC501dd02
#define NPU_ENABLE_SECMODE              0xC501dd03
#define GIC_CFG_CHECK_SECMODE           0xC501dd04
#define GIC_ONLINE_READY_SECMODE        0xC501dd05
#define NPU_CPU_POWER_DOWN_SECMODE      0xC501dd06
#define NPU_INFORM_POWER_DOWN_SECMODE   0xC501dd07
#define NPU_POWER_DOWN_TS_SEC_REG       0xC501dd08
#define NPU_SMMU_TCU_INIT_NS            0xC501dd09
#define NPU_SMMU_TCU_CACHE_INIT         0xC501dd0a
#define NPU_SMMU_TCU_DISABLE            0xC501dd0b
#define NPU_POWER_UP_SMMU_TBU           0xC501dd0c
#define NPU_POWER_DOWN_SMMU_TBU         0xC501dd0d

enum npu_atf_error_code {
	NPU_ATF_SUCC = 0,
	NPU_ATF_MULTICHIP_GIC_ONLINE_FAILED  = 1,
	NPU_ATF_MULTICHIP_GIC_RWP_FAILED = 2,
	NPU_ATF_MULTICHIP_GIC_PUP_FAILED = 4, // Power update in Progress check
	NPU_ATF_MULTICHIP_GIC_RTS_FAILED = 8, // Route Table Status check
};

enum npu_gic_chipid {
	NPU_GIC_1  = 1
};

enum {
	HW_IRQ_LEVEL_TRIGGER_TYPE = 0x0,
	HW_IRQ_EDGE_TRIGGER_TYPE = 0x1
};

enum npuip {
	NPUIP_NONE = 0,
	NPUIP_NPU_SUBSYS,
	NPUIP_NPU_BUS,
	NPUIP_TS_SUBSYS,
	NPUIP_AICORE,
	NPUIP_AICPU,
	NPUIP_NPUCPU_SUBSYS, // kirin990 cs
	NPUIP_MAX
};

enum npu_gic_cmd {
	NPU_GIC_POWER_OFF_INFORM = 0,
	TSCPU_GIC_WAKER_OPS_READY = 100,
	TSCPU_GIC_WAKER_OPS_DONE = 200,
	TSCPU_GIC_PWRR_OPS_READY = 300,
	TSCPU_GIC_PWRR_OPS_DONE = 400,
	AICPU_GIC_WAKER_OPS_READY = 500,
	AICPU_GIC_WAKER_OPS_DOWN = 600,
	AICPU_GIC_PWRR_OPS_READY = 700,
	AICPU_GIC_PWRR_OPS_DOWN = 800
};

int npuatf_change_slv_secmod(u64 cmd);

int npuatf_change_mst_secmod(u64 cmd);

int npuatf_start_secmod(u64 ts_cpu_addr);

int npuatf_enable_secmod(u64 cmd);

int npuatf_powerup_aicore(u64 secure_mode, u32 aic_flag);

int npuatf_power_down_aicore(u64 is_secure, u32 aic_flag);

int npuatf_power_down(void);

int npuatf_power_down_nputop(void);

int atf_query_gic0_state(u64 cmd);

int acpu_gic0_online_ready(u64 cmd);

int npuatf_power_down_ts_secreg(u32 is_secure);

int npuatf_inform_power_down(int gic_cmd);

int npuatf_enable_tbu(u64 cmd);

int npuatf_disable_tbu(u64 cmd);

#endif
