/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 */

#include "memory_layout.h"
#include <linux/module.h>
#include <linux/version.h>
#include <mdrv_memory.h>
#include <product_config.h>
#include <bsp_dt.h>
#include <securec.h>

mem_node_info g_mem_layout_info[] = {
#ifdef DDR_MNTN_ADDR
    {DDR_MNTN_ADDR,             DDR_MNTN_SIZE,       "mntn_ddr"},
#endif
#ifdef DDR_SHARED_MEM_ADDR
    {DDR_SHARED_MEM_ADDR,       DDR_SHARED_MEM_SIZE, "share_mem_ddr"},
#endif
#ifdef DDR_TLPHY_IMAGE_ADDR
    {DDR_TLPHY_IMAGE_ADDR,      DDR_TLPHY_IMAGE_SIZE, "tlphy_ddr"},
#endif
#ifdef DDR_MCORE_DTS_ADDR
    {DDR_MCORE_DTS_ADDR,        DDR_MCORE_DTS_SIZE,   "mdm_dts_ddr"},
#endif
#ifdef DDR_MCORE_ADDR
    {DDR_MCORE_ADDR,            DDR_MCORE_SIZE,       "mdm_ddr"},
#endif
#ifdef DDR_CBBE_IMAGE_ADDR
    {DDR_CBBE_IMAGE_ADDR,       DDR_CBBE_IMAGE_SIZE,  "cbbe_ddr"},
#endif
#ifdef DDR_SOCP_ADDR
    {DDR_SOCP_ADDR,             DDR_SOCP_SIZE,        "socp_bbpds_ddr"},
#endif
#ifdef DDR_RFIC_SUB6G_IMAGE_ADDR
    {DDR_RFIC_SUB6G_IMAGE_ADDR, DDR_RFIC_SUB6G_IMAGE_SIZE, "rf_sub6g_ddr"},
#endif
#ifdef DDR_RFIC_HF_IMAGE_ADDR
    {DDR_RFIC_HF_IMAGE_ADDR,    DDR_RFIC_HF_IMAGE_SIZE, "rf_hf_ddr"},
#endif
#ifdef DDR_RFIC_DUMP_ADDR
    {DDR_RFIC_DUMP_ADDR,        DDR_RFIC_DUMP_SIZE,     "rf_dump_ddr"},
#endif
#ifdef DDR_SEC_SHARED_ADDR
    {DDR_SEC_SHARED_ADDR,       DDR_SEC_SHARED_SIZE,    "sec_share_ddr"},
#endif
#ifdef DDR_NRPHY_IMAGE_ADDR
    {DDR_NRPHY_IMAGE_ADDR,      DDR_NRPHY_IMAGE_SIZE,   "nrphy_ddr"},
#endif
#ifdef DDR_PDE_IMAGE_ADDR
    {DDR_PDE_IMAGE_ADDR,        DDR_PDE_IMAGE_SIZE,     "pde_ddr"},
#endif
#ifdef DDR_MAA_MDM_ADDR
    {DDR_MAA_MDM_ADDR,          DDR_MAA_MDM_SIZE,       "mdm_maa_ddr"},
#endif
#ifdef DDR_EMU_HAC_LOAD_ADDR
    {DDR_EMU_HAC_LOAD_ADDR,     DDR_EMU_HAC_LOAD_SIZE,  "emu_hac_ddr"},
#endif
#ifdef DDR_SHAREDNR_MEM_ADDR
    {DDR_SHAREDNR_MEM_ADDR,     DDR_MCORE_NR_SHARED_MEM_SIZE, "sharednr_ddr"},
#endif
#ifdef DDR_MCORE_NR_ADDR
    {DDR_MCORE_NR_ADDR,         DDR_MCORE_NR_SIZE,      "mdm_nr_ddr"},
#endif
};

unsigned long mdrv_mem_region_get(const char *name, unsigned int *size)
{
    mem_node_info *mem_info = NULL;
    unsigned int max_rec;
    unsigned long phy_addr;
    unsigned int i;

    if (name == NULL || size == NULL) {
        MEM_ERR("[memory_layout]input error!\n");
        return 0;
    }

    mem_info = g_mem_layout_info;
    max_rec = sizeof(g_mem_layout_info) / sizeof(mem_node_info);
    for (i = 0; i < max_rec; i++) {
        if (strcmp((mem_info + i)->name, name) == 0) {
            phy_addr = (unsigned long)((mem_info + i)->addr);
            *size = (mem_info + i)->size;
            return phy_addr;
        }
    }
    *size = 0;
    MEM_ERR("[memory_layout]get memory %s error!\n", name);
    return 0;
}
EXPORT_SYMBOL_GPL(mdrv_mem_region_get);


