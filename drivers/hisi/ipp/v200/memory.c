#include <linux/err.h>
#include <linux/printk.h>
#include "memory.h"

#define MEM_HISPCPE_SIZE   (0x00300000) /*  the cpe iova size */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static unsigned int       g_mem_used[MEM_ID_MAX]  = {0};
static unsigned long long g_mem_va[MEM_ID_MAX] = {0};
static unsigned int       g_mem_da[MEM_ID_MAX] = {0};

unsigned int g_mem_offset[MEM_ID_MAX] = 
{
    ALIGN_UP(CMDLST_BUFFER_SIZE * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CMDLST_BUF_MM_OFFSET
    ALIGN_UP(sizeof(schedule_cmdlst_link_t) * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CMDLST_ENTRY_MM_OFFSET
    ALIGN_UP(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),//MEM_CMDLST_PARA_MM_OFFSET
    ALIGN_UP(sizeof(mcf_config_table_t) * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_MCF_CFG_TAB_OFFSET
    ALIGN_UP(sizeof(mfnr_config_table_t) * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_MFNR_CFG_TAB_OFFSET
    ALIGN_UP(sizeof(cpe_top_config_table_t) * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CPETOP_CFG_TAB_MM_OFFSET
    ALIGN_UP(sizeof(cfg_tab_cvdr_t) * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_CVDR_CFG_TAB_MM_OFFSET
    ALIGN_UP(sizeof(slam_config_table_t) * MAX_PYRAMID_LAYER*2, CVDR_ALIGN_BYTES),//MEM_SLAM_CFG_TAB_OFFSET
    ALIGN_UP(CMDLST_BUFFER_SIZE * MAX_PYRAMID_LAYER*2, CVDR_ALIGN_BYTES),//MEM_CMDLST_BUF_SLAM_OFFSET
    ALIGN_UP(sizeof(schedule_cmdlst_link_t) * MAX_PYRAMID_LAYER*2, CVDR_ALIGN_BYTES),//MEM_CMDLST_ENTRY_SLAM_OFFSET
    ALIGN_UP(sizeof(cmdlst_para_t), CVDR_ALIGN_BYTES),//MEM_CMDLST_PARA_SLAM_OFFSET
    ALIGN_UP(sizeof(cpe_top_config_table_t) * MAX_PYRAMID_LAYER*2, CVDR_ALIGN_BYTES),//MEM_CPETOP_CFG_TAB_SLAM_OFFSET
    ALIGN_UP(sizeof(cfg_tab_cvdr_t) * MAX_PYRAMID_LAYER*2, CVDR_ALIGN_BYTES),//MEM_CVDR_CFG_TAB_SLAM_OFFSET
    ALIGN_UP(0x100 * MAX_STRIPE_NUM, CVDR_ALIGN_BYTES),//MEM_MFNR_STAT_OFFSET
    ALIGN_UP(8 * SLAM_SCOREHIST_NUM * MAX_PYRAMID_LAYER, CVDR_ALIGN_BYTES),//MEM_SLAM_STAT_OFFSET 
};

int cpe_mem_init(unsigned long long va, unsigned int da, unsigned int size)
{
    int i = 0;

    pr_info("[%s] +\n", __func__);

    for(i = 0; i < MEM_ID_MAX; i++)
    {
        if(i == 0)
        {
            g_mem_va[i] = va;
            g_mem_da[i] = da;
        }
        else
        {
            g_mem_va[i] = g_mem_va[i-1]+g_mem_offset[i-1];
            g_mem_da[i] = g_mem_da[i-1]+g_mem_offset[i-1];
        }
    }

    if(g_mem_va[MEM_ID_MAX - 1] - va > size)
    {
        pr_err("[%s] Failed : vaddr overflow.(0x%llx - 0x%llx > 0x%x)\n", __func__, g_mem_va[MEM_ID_MAX - 1], va, size);
        return -ENOMEM;
    }

    if(g_mem_da[MEM_ID_MAX - 1] - da > size)
    {
        pr_err("[%s] Failed : daddr overflow.(0x%x - 0x%x > 0x%x)\n", __func__, g_mem_da[MEM_ID_MAX - 1], da, size);
        return -ENOMEM;
    }

    for (i = 0; i < MEM_ID_MAX; i ++)
    {
        pr_info("g_mem_va[%d] = 0x%llx, g_mem_da[%d] = 0x%x", i, g_mem_va[i], i, g_mem_da[i]);
    }
    
    pr_info("[%s] -\n", __func__);

    return 0;
}

int cpe_init_memory(void)
{
    unsigned int da = 0;
    unsigned long long va = 0;
    int ret = 0;

    pr_info("[%s] +\n", __func__);
    if ((da = get_cpe_addr_da()) == 0) {
        pr_err("[%s] Failed : CPE Device da false.\n", __func__);
        return -ENOMEM;
    }

    if((va = (unsigned long long)(get_cpe_addr_va())) == 0) {
        pr_err("[%s] Failed : CPE Device va false.\n", __func__);
        return -ENOMEM;
    }

    pr_info("For request va = 0x%llx, da = 0x%x.\n", va, da);

    if ((ret = cpe_mem_init(va, da, MEM_HISPCPE_SIZE)) != 0) {
        pr_err("[%s] Failed : cpe_mem_init.(0x%llx, 0x%x, 0x%x).%d\n", __func__, va, da, MEM_HISPCPE_SIZE, ret);
        return -ENOMEM;
    }

    pr_info("[%s] -\n", __func__);

    return 0;
}

int cpe_mem_get(cpe_mem_id mem_id, unsigned long long *va, unsigned int *da)
{
    pr_info("[%s] +\n", __func__);
    if (!va || !da) {
        pr_err("[%s] Failed : va.%pK, da.%pK\n", __func__, va, da);
        return -ENOMEM;
    }

    if (mem_id >= MEM_ID_MAX) {
        pr_err("[%s] Failed : mem_id.(%u >= %u)\n", __func__, mem_id, MEM_ID_MAX);
        return -ENOMEM;
    }

    if (g_mem_used[mem_id] == 1) {
        pr_err("[%s] Failed : g_mem_used[%u].%u\n", __func__, mem_id, g_mem_used[mem_id]);
        return -ENOMEM;
    }

    if (!(*da = (unsigned int)g_mem_da[mem_id]) || !(*va = (unsigned long long)g_mem_va[mem_id])) {
        pr_err("[%s] Failed : g_mem_da[%u].0x%x, g_mem_va[%u].0x%llx\n", __func__, mem_id, g_mem_da[mem_id], mem_id, g_mem_va[mem_id]);
        return -ENOMEM;
    }
    g_mem_used[mem_id] = 1;

    return 0;
}

void cpe_mem_free(cpe_mem_id mem_id)
{
    pr_info("[%s] +\n", __func__);
    
    if (mem_id >= MEM_ID_MAX) {
        pr_err("Invalid parameter! mem_id = %u\n", mem_id);
        return;
    }

    if (g_mem_used[mem_id] == 0) {
        pr_err("Unable to free, g_mem_used[%u].%u", mem_id, g_mem_used[mem_id]);
        return;
    }

    g_mem_used[mem_id] = 0;

	pr_info("[%s] -\n", __func__);

    return;
}

#pragma GCC diagnostic pop

