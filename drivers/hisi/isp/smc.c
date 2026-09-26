/*
 *  ISP driver, qos.c
 *
 * Copyright (c) 2018 ISP Technologies CO., Ltd.
 *
 */

#include <securec.h>
#include <bl31_smc.h>

noinline int atfd_isp_service_isp_smc(u64 _funcid,
				u64 _arg0, u64 _arg1, u64 _arg2)
{
	register u64 funcid asm("x0") = _funcid;
	register u64 arg0 asm("x1") = _arg0;
	register u64 arg1 asm("x2") = _arg1;
	register u64 arg2 asm("x3") = _arg2;
	asm volatile (
			__asmeq("%0", "x0")
			__asmeq("%1", "x1")
			__asmeq("%2", "x2")
			__asmeq("%3", "x3")
			"smc    #0\n"
			: "+r" (funcid)
			: "r" (arg0), "r" (arg1), "r" (arg2));

	return (int)funcid;
}

int atfisp_setparams(u64 shrd_paddr)
{
	atfd_isp_service_isp_smc(ISP_FN_SET_PARAMS, shrd_paddr, 0, 0);
	return 0;
}

int atfisp_params_dump(u64 boot_pa_addr)
{
	atfd_isp_service_isp_smc(ISP_FN_PARAMS_DUMP, boot_pa_addr, 0, 0);
	return 0;
}

int atfisp_cpuinfo_nsec_dump(u64 param_pa_addr)
{
	atfd_isp_service_isp_smc(ISP_FN_ISPCPU_NS_DUMP, param_pa_addr, 0, 0);
	return 0;
}

void atfisp_isp_init(u64 pgd_base)
{
	atfd_isp_service_isp_smc(ISP_FN_ISP_INIT, pgd_base, 0, 0);
}

void atfisp_isp_exit(void)
{
	atfd_isp_service_isp_smc(ISP_FN_ISP_EXIT, 0, 0, 0);
}

int atfisp_ispcpu_init(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_ISPCPU_INIT, 0, 0, 0);
}

void atfisp_ispcpu_exit(void)
{
	atfd_isp_service_isp_smc(ISP_FN_ISPCPU_EXIT, 1, 0, 0);
}

int atfisp_ispcpu_map(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_ISPCPU_MAP, 0, 0, 0);
}

void atfisp_ispcpu_unmap(void)
{
	atfd_isp_service_isp_smc(ISP_FN_ISPCPU_UNMAP, 0, 0, 0);
}

void atfisp_set_nonsec(void)
{
	atfd_isp_service_isp_smc(ISP_FN_SET_NONSEC, 0, 0, 0);
}

int atfisp_disreset_ispcpu(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_DISRESET_ISPCPU, 0, 0, 0);
}

void atfisp_ispsmmu_ns_init(u64 pgt_addr)
{
	atfd_isp_service_isp_smc(ISP_FN_ISPSMMU_NS_INIT, pgt_addr, 0, 0);
}

int atfisp_get_ispcpu_idle(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_GET_ISPCPU_IDLE, 0, 0, 0);
}

int atfisp_send_fiq2ispcpu(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_SEND_FIQ_TO_ISPCPU, 0, 0, 0);
}

int atfisp_isptop_power_up(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_ISPTOP_PU, 0, 0, 0);
}

int atfisp_isptop_power_down(void)
{
	return atfd_isp_service_isp_smc(ISP_FN_ISPTOP_PD, 0, 0, 0);
}

int atfisp_phy_csi_connect(u64 info_addr)
{
	return atfd_isp_service_isp_smc(ISP_PHY_CSI_CONNECT, info_addr, 0, 0);
}

int atfisp_phy_csi_disconnect(void)
{
	return atfd_isp_service_isp_smc(ISP_PHY_CSI_DISCONNECT, 0, 0, 0);
}

