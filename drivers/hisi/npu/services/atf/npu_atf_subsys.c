#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/hisi/hisi_npu_pm.h>

#include "npu_atf_subsys.h"
#include "drv_log.h"

noinline int atfd_hisi_service_npu_smc(u64 _service_id, u64 _cmd, \
										u64 _arg1, u64 _arg2)
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
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(NPU_SLV_SECMODE, cmd, 0, 0);
	return ret;
}

int npuatf_change_mst_secmod(u64 cmd)
{
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(NPU_MST_SECMODE, cmd, 0, 0);
	return 0;
}

int npuatf_start_secmod(u64 ts_cpu_addr)
{
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(NPU_START_SECMODE, 0,
									ts_cpu_addr, 0);
	return ret;
}

int npuatf_enable_secmod(u64 cmd)
{
	int ret = 0;
	devdrv_drv_debug("hisi_npu_power_on start.\n");
	NPU_DRV_BOOT_TIME_TAG("start hisi_npu_power_on \n");
	ret = hisi_npu_power_on();
	if (ret) {
		devdrv_drv_err("hisi_npu_power_on failed ,ret = %d \n",ret);
		return -1;
	}
	NPU_DRV_BOOT_TIME_TAG("start atfd_hisi_service_npu_smc \n");
	ret = atfd_hisi_service_npu_smc(NPU_ENABLE_SECMODE, cmd, 0, 0);
	if (ret) {
		devdrv_drv_err("NPU_ENABLE_SECMODE failed ,ret = %d \n",ret);
		return -1;
	}
	return ret;
}

int npuatf_inform_power_down(int gic_cmd)
{
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(NPU_INFORM_POWER_DOWN_SECMODE, gic_cmd, 0, 0);
	return ret;
}

int npuatf_power_down_ts_secreg(void)
{
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(NPU_POWER_DOWN_TS_SEC_REG, 0, 0, 0);
	return ret;
}

int npuatf_power_down(void)
{
	int ret = 0;

	//power down npu cpu
	ret = atfd_hisi_service_npu_smc(NPU_CPU_POWER_DOWN_SECMODE, 0, 0, 0);
	if (ret !=0)
	{
		devdrv_drv_err("npu_smc or power_off fail ,ret: %d\n", ret);
	}
	//power down npu bus and subsystem
	ret = hisi_npu_power_off();
	if (ret !=0)
	{
		devdrv_drv_err("npu_smc or power_off fail ,ret: %d\n", ret);
		return ret;
	}

	return 0;
}

int atf_query_gic0_state(u64 cmd)
{
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(GIC_CFG_CHECK_SECMODE, cmd, 0, 0);
	return ret;
}

int acpu_gic0_online_ready(u64 cmd)
{
	int ret = 0;
	ret = atfd_hisi_service_npu_smc(GIC_ONLINE_READY_SECMODE, cmd, 0, 0);
	return ret;
}
