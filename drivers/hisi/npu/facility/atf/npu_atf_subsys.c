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
#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/hisi/hisi_npu_pm.h>
#include "npu_atf_subsys.h"
#include "npu_log.h"

noinline int atfd_hisi_service_npu_smc(u64 _service_id, u64 _cmd, u64 _arg1, u64 _arg2)
{
	register u64 service_id asm("x0") = _service_id;
	register u64 cmd asm("x1") = _cmd;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile(
		__asmeq("%0", "x0")
		__asmeq("%1", "x1")
		__asmeq("%2", "x2")
		__asmeq("%3", "x3")
		"smc    #0\n"
		: "+r" (service_id)
		: "r" (cmd), "r" (arg1), "r" (arg2));

	return (int)service_id;
}

int npuatf_change_slv_secmod(u64 cmd)
{
	int ret;
	NPU_DRV_DEBUG("start npuatf_change_slv_secmod\n");
	ret = atfd_hisi_service_npu_smc(NPU_SLV_SECMODE, cmd, 0, 0);
	return ret;
}

int npuatf_change_mst_secmod(u64 cmd)
{
	(void)atfd_hisi_service_npu_smc(NPU_MST_SECMODE, cmd, 0, 0);
	return 0;
}

int npuatf_start_secmod(u64 is_secure)
{
	int ret;
	ret = atfd_hisi_service_npu_smc(NPU_START_SECMODE, 0, is_secure, 0);
	return ret;
}

int npuatf_enable_secmod(u64 secure_mode)
{
	int ret;
	NPU_DRV_DEBUG("hisi_npu_power_on start\n");
	NPU_DRV_BOOT_TIME_TAG("start hisi_npu_power_on \n");  // based on hisi sw interface for npu subsys/npu bus poweron

	ret = hisi_npu_power_on();
	if (ret) {
		NPU_DRV_ERR("hisi_npu_power_on failed ,ret = %d \n", ret);
		return -1;
	}

	NPU_DRV_BOOT_TIME_TAG("start atfd_hisi_service_npu_smc \n");
	ret = atfd_hisi_service_npu_smc(NPU_ENABLE_SECMODE, secure_mode, 0, 0);
	if (ret) {
		NPU_DRV_ERR("NPU_ENABLE_SECMODE failed ,ret = %d \n", ret);
		return -1;
	}

	return ret;
}

int npuatf_powerup_aicore(u64 secure_mode, u32 aic_flag)
{
	int ret;

	NPU_DRV_DEBUG("hisi_npu_power_on aicore start\n");
	NPU_DRV_BOOT_TIME_TAG("start atfd_hisi_service_npu_smc \n");
	ret = atfd_hisi_service_npu_smc(NPU_ENABLE_SECMODE, secure_mode, NPUIP_AICORE, aic_flag); /* 1:powerup aicore */
	if (ret) {
		NPU_DRV_ERR("NPU_ENABLE_SECMODE failed ,ret = %d \n", ret);
		return -1;
	}

	return ret;
}

int npuatf_power_down_ts_secreg(u32 is_secure)
{
	int ret;
	ret = atfd_hisi_service_npu_smc(NPU_POWER_DOWN_TS_SEC_REG, is_secure, 0, 0);
	return ret;
}

int npuatf_power_down(void)
{
	int ret;
	// power down npu cpu
	ret = atfd_hisi_service_npu_smc(NPU_CPU_POWER_DOWN_SECMODE, 0, 0, 0);
	if (ret != 0)
		NPU_DRV_ERR("npu_smc or power_off fail NPU_CPU_POWER_DOWN_SECMODE ,ret: %d\n", ret);

	return ret;
}

int npuatf_power_down_nputop(void)
{
	int ret;
	// power down npu bus and subsystem
	ret = hisi_npu_power_off();
	if (ret != 0) {
		NPU_DRV_ERR("npu_smc or power_off fail, ret: %d\n", ret);
		return ret;
	}
	return 0;
}

int npuatf_power_down_aicore(u64 secure_mode, u32 aic_flag)
{
	int ret;

	NPU_DRV_DEBUG("hisi_npu_power_down aicore start\n");
	ret = atfd_hisi_service_npu_smc(NPU_CPU_POWER_DOWN_SECMODE, secure_mode, NPUIP_AICORE, aic_flag); /* 1:powerdown aicore */
	if (ret) {
		NPU_DRV_ERR("NPU_CPU_POWER_DOWN_SECMODE failed ,ret = %d \n", ret);
		return -1;
	}

	return ret;
}

int atf_query_gic0_state(u64 cmd)
{
	int ret;
	ret = atfd_hisi_service_npu_smc(GIC_CFG_CHECK_SECMODE, cmd, 0, 0);
	return ret;
}

int acpu_gic0_online_ready(u64 cmd)
{
	int ret;
	ret = atfd_hisi_service_npu_smc(GIC_ONLINE_READY_SECMODE, cmd, 0, 0);
	return ret;
}

int npuatf_enable_tbu(u64 cmd)
{
	int ret;
	NPU_DRV_DEBUG("npuatf_enable_tbu start\n");

	ret = atfd_hisi_service_npu_smc(NPU_POWER_UP_SMMU_TBU, cmd, 0, 0);
	if (ret) {
		NPU_DRV_ERR("npuatf_enable_tbu failed ,ret = %d \n", ret);
		return -1;
	}
	return ret;
}

int npuatf_disable_tbu(u64 cmd)
{
	int ret;
	NPU_DRV_DEBUG("npuatf_disable_tbu start\n");

	ret = atfd_hisi_service_npu_smc(NPU_POWER_DOWN_SMMU_TBU, cmd, 0, 0);
	if (ret) {
		NPU_DRV_ERR("atfd_hisi_service_npu_smc failed ,ret = %d \n", ret);
		return -1;
	}
	return ret;
}

