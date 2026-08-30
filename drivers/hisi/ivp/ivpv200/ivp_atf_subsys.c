#include <asm/compiler.h>
#include <linux/compiler.h>
#include <linux/of.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include "ivp_platform.h"
#include "ivp.h"
#include "ivp_log.h"
#include "ivp_reg.h"
#include "ivp_atf_subsys.h"


noinline int atfd_hisi_service_ivp_smc(u64 _function_id, u64 _arg0, u64 _arg1, u64 _arg2)
{
    register u64 function_id asm("x0") = _function_id;
    register u64 arg0 asm("x1") = _arg0;
    register u64 arg1 asm("x2") = _arg1;
    register u64 arg2 asm("x3") = _arg2;

    asm volatile(
        __asmeq("%0", "x0")
        __asmeq("%1", "x1")
        __asmeq("%2", "x2")
        __asmeq("%3", "x3")
        "smc    #0\n"
    : "+r" (function_id)
    : "r" (arg0), "r" (arg1), "r" (arg2));

    return (int)function_id;
}

int ivpatf_change_slv_secmod(unsigned int mode)
{
    ivp_dbg("change slv mode to %d", mode);
    atfd_hisi_service_ivp_smc(IVP_SLV_SECMODE, mode, 0, 0);
    return 0;
}

int ivpatf_change_mst_secmod(unsigned int mode)
{
    ivp_dbg("change mst mode to %d", mode);
    atfd_hisi_service_ivp_smc(IVP_MST_SECMODE, mode, 0, 0);
    return 0;
}

int ivpatf_poweron_remap_secmod(unsigned int ivp_addr, unsigned int len, unsigned long ddr_addr)
{
    if ((ivp_addr & MASK_1MB) != 0 ||
        (ddr_addr & MASK_1MB) != 0 ||
        len >= 128 * SIZE_1MB) {
        ivp_err("not aligned");
        return -EINVAL;
    }
    len = (len + SIZE_1MB - 1) / SIZE_1MB - 1;
    atfd_hisi_service_ivp_smc(IVP_REMAP_SECMODE, ivp_addr/SIZE_1MB, ddr_addr/SIZE_1MB, len);
    return 0;
}

