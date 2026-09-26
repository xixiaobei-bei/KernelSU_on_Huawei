#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/syscore_ops.h>
#include <bsp_ipc.h>
#include <n_bsp_ipf.h>
#include <n_bsp_psam.h>
#include <bsp_slice.h>
#include <bsp_pm_om.h>
#include <osl_bio.h>
#include <bsp_slice.h>
#include "ipf_balong.h"

extern ipf_ctx_t g_ipf_ctx;

/*
    save==0: ipf is still up, no need to resume
*/
static int ipf_is_really_down(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    return (sm->init.status.save == ipf_flag_reset)||(sm->init.status.save == ipf_flag_down);
}

static void ipf_pm_reset(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();

    if(ipf_flag_down == sm->init.status.save)
    {
        reset_ipf_psam_from_sys();
        g_ipf_ctx.psam_pm->restore_reg(&sm->reg_save[PSAM_ACORE_MAP(sm)]);
        g_ipf_ctx.stax.reset[g_ipf_ctx.stax.reset_index%8] = bsp_get_slice_value();
        g_ipf_ctx.stax.reset_index++;
    }
    
    sm->init.status.save = ipf_flag_reset;
}

static void ipf_up(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
    sm->init.status.acore=IPF_PWR_UP;
    if(sm->init.status.ccore==IPF_PWR_UP)
    {
        sm->init.status.save = ipf_flag_up;
    }
}


static void ipf_down(void)
{
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    
    sm->init.status.acore=IPF_PWR_DOWN;
    if(sm->init.status.ccore==IPF_PWR_DOWN)
    {
        sm->init.status.save = ipf_flag_down;
    }
}

static int ipf_pm_prepare(struct device *pdev)
{
    /* check whether the downstream channel is idle*/

    if(g_ipf_ctx.desc->is_busy&&(g_ipf_ctx.desc->is_busy()))
    {
		g_ipf_ctx.ipf_pm_busy++;
        return IPF_ERROR;
    }

    if(g_ipf_ctx.psam_pm->is_idle&&(!g_ipf_ctx.psam_pm->is_idle()))
    {
        bsp_trace(BSP_LOG_LEVEL_ERROR, BSP_MODU_IPF,"\r PSAM CHANNEL NOT IDLE! \n");
        g_ipf_ctx.psam_pm_busy++;
        return IPF_ERROR;
    }

    g_ipf_ctx.psam_pm_prepare++;
    return IPF_SUCCESS;
}

static int ipf_pm_suspend(struct device *pdev)
{
    unsigned long flags;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
    pr_info("%s +\n", __func__);

    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);
    
    g_ipf_ctx.desc->save_reg(&sm->reg_save[IPF_ACORE_MAP(sm)]);
    g_ipf_ctx.psam_pm->save_reg(&sm->reg_save[PSAM_ACORE_MAP(sm)]);/*lint !e679*/
    
    ipf_down();

    cache_sync();
    
    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, flags);

    g_ipf_ctx.stax.suspend++;
    (void)pdev;
    pr_info("%s -\n", __func__);
    return IPF_SUCCESS;
}

static int ipf_pm_resume(struct device *pdev)
{
    HI_IPF_INT1_T status;
    unsigned long flags;
    struct ipf_share_mem_map* sm = bsp_ipf_get_sharemem();
	
    pr_info("%s +\n", __func__);
    spin_lock_irqsave(&g_ipf_ctx.filter_spinlock, flags);
    bsp_ipc_spin_lock(IPC_SEM_IPF_PWCTRL);

    if(ipf_is_really_down()){
        ipf_pm_reset();
        g_ipf_ctx.desc->restore_reg(&sm->reg_save[IPF_ACORE_MAP(sm)]);
//        g_ipf_ctx.psam_pm->restore_reg(&sm->reg_save[PSAM_ACORE_MAP(sm)]);/*lint !e679*/
    }

    ipf_up();

    cache_sync();
    
    bsp_ipc_spin_unlock(IPC_SEM_IPF_PWCTRL);
    spin_unlock_irqrestore(&g_ipf_ctx.filter_spinlock, flags);
    
    status.u32 = ipf_readl(HI_IPF_INT1_OFFSET); 
    if (status.bits.dl_rpt_int1\
                    | status.bits.dl_timeout_int1\
                    | status.bits.dl_adq0_epty_int1 \
                    | status.bits.dl_adq1_epty_int1) 
    {
        g_ipf_ctx.status->resume_with_intr++;
    }

    g_ipf_ctx.stax.resume++;
    (void)pdev;
    pr_info("%s -\n", __func__);
    return IPF_SUCCESS;
}

struct dev_pm_ops ipf_dev_pm_ops={
    .prepare = ipf_pm_prepare,
    .suspend_noirq = ipf_pm_suspend,
    .resume_noirq = ipf_pm_resume,
};



