#ifndef __SOC_SMMU_INTERFACE_H__
#define __SOC_SMMU_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_SMMU_SCR_ADDR(base) ((base) + (0x0000))
#define SOC_SMMU_MEMCTRL_ADDR(base) ((base) + (0x0004))
#define SOC_SMMU_LP_CTRL_ADDR(base) ((base) + (0x0008))
#define SOC_SMMU_PRESS_REMAP_ADDR(base) ((base) + (0x000C))
#define SOC_SMMU_INTMASK_NS_ADDR(base) ((base) + (0x0010))
#define SOC_SMMU_INTRAW_NS_ADDR(base) ((base) + (0x0014))
#define SOC_SMMU_INTSTAT_NS_ADDR(base) ((base) + (0x0018))
#define SOC_SMMU_INTCLR_NS_ADDR(base) ((base) + (0x001C))
#define SOC_SMMU_SMRX_NS_ADDR(base,n) ((base) + (0x0020+(n)*0x4))
#define SOC_SMMU_RLD_EN0_NS_ADDR(base) ((base) + (0x01F0))
#define SOC_SMMU_RLD_EN1_NS_ADDR(base) ((base) + (0x01F4))
#define SOC_SMMU_RLD_EN2_NS_ADDR(base) ((base) + (0x01F8))
#define SOC_SMMU_GLB_ALLOC_ST_ADDR(base) ((base) + (0x01FC))
#define SOC_SMMU_CB_SCTRL_ADDR(base) ((base) + (0x0200))
#define SOC_SMMU_CB_TTBR0_ADDR(base) ((base) + (0x0204))
#define SOC_SMMU_CB_TTBR1_ADDR(base) ((base) + (0x0208))
#define SOC_SMMU_CB_TTBCR_ADDR(base) ((base) + (0x020C))
#define SOC_SMMU_OFFSET_ADDR_NS_ADDR(base) ((base) + (0x0210))
#define SOC_SMMU_SCACHEI_ALL_ADDR(base) ((base) + (0x0214))
#define SOC_SMMU_SCACHEI_L1_ADDR(base) ((base) + (0x0218))
#define SOC_SMMU_SCACHEI_L2L3_ADDR(base) ((base) + (0x021C))
#define SOC_SMMU_FAMA_CTRL0_NS_ADDR(base) ((base) + (0x0220))
#define SOC_SMMU_FAMA_CTRL1_NS_ADDR(base) ((base) + (0x0224))
#define SOC_SMMU_MAIR_NS_ADDR(base) ((base) + (0x0228))
#define SOC_SMMU_ADDR_MSB_ADDR(base) ((base) + (0x0300))
#define SOC_SMMU_ERR_RDADDR_ADDR(base) ((base) + (0x0304))
#define SOC_SMMU_ERR_WRADDR_ADDR(base) ((base) + (0x0308))
#define SOC_SMMU_FAULT_ADDR_TCU_ADDR(base) ((base) + (0x0310))
#define SOC_SMMU_FAULT_ID_TCU_ADDR(base) ((base) + (0x0314))
#define SOC_SMMU_FAULT_ADDR_TBUX_ADDR(base,f) ((base) + (0x0320+(f)*0x10))
#define SOC_SMMU_FAULT_ID_TBUX_ADDR(base,f) ((base) + (0x0324+(f)*0x10))
#define SOC_SMMU_FAULT_INFOX_ADDR(base,f) ((base) + (0x0328+(f)*0x10))
#define SOC_SMMU_DBGRPTR_TLB_ADDR(base) ((base) + (0x0380))
#define SOC_SMMU_DBGRDATA_TLB_ADDR(base) ((base) + (0x0384))
#define SOC_SMMU_DBGRPTR_CACHE_ADDR(base) ((base) + (0x0388))
#define SOC_SMMU_DBGRDATA0_CACHE_ADDR(base) ((base) + (0x038C))
#define SOC_SMMU_DBGRDATA1_CACHE_ADDR(base) ((base) + (0x0390))
#define SOC_SMMU_DBGAXI_CTRL_ADDR(base) ((base) + (0x0394))
#define SOC_SMMU_OVA_ADDR_ADDR(base) ((base) + (0x0398))
#define SOC_SMMU_OPA_ADDR_ADDR(base) ((base) + (0x039C))
#define SOC_SMMU_OVA_CTRL_ADDR(base) ((base) + (0x03A0))
#define SOC_SMMU_OPREF_ADDR_ADDR(base) ((base) + (0x03A4))
#define SOC_SMMU_OPREF_CTRL_ADDR(base) ((base) + (0x03A8))
#define SOC_SMMU_OPREF_CNT_ADDR(base) ((base) + (0x03AC))
#define SOC_SMMU_SMRX_S_ADDR(base,n) ((base) + (0x0500+(n)*0x4))
#define SOC_SMMU_RLD_EN0_S_ADDR(base) ((base) + (0x06F0))
#define SOC_SMMU_RLD_EN1_S_ADDR(base) ((base) + (0x06F4))
#define SOC_SMMU_RLD_EN2_S_ADDR(base) ((base) + (0x06F8))
#define SOC_SMMU_INTMAS_S_ADDR(base) ((base) + (0x0700))
#define SOC_SMMU_INTRAW_S_ADDR(base) ((base) + (0x0704))
#define SOC_SMMU_INTSTAT_S_ADDR(base) ((base) + (0x0708))
#define SOC_SMMU_INTCLR_S_ADDR(base) ((base) + (0x070C))
#define SOC_SMMU_SCR_S_ADDR(base) ((base) + (0x0710))
#define SOC_SMMU_SCB_SCTRL_ADDR(base) ((base) + (0x0714))
#define SOC_SMMU_SCB_TTBR_ADDR(base) ((base) + (0x0718))
#define SOC_SMMU_SCB_TTBCR_ADDR(base) ((base) + (0x071C))
#define SOC_SMMU_OFFSET_ADDR_S_ADDR(base) ((base) + (0x0720))
#define SOC_SMMU_FAMA_CTRL0_S_ADDR(base) ((base) + (0x0724))
#define SOC_SMMU_FAMA_CTRL1_S_ADDR(base) ((base) + (0x0728))
#define SOC_SMMU_DBGRPTR_TLB_S_ADDR(base) ((base) + (0x072C))
#define SOC_SMMU_DBGRPTR_CACHE_S_ADDR(base) ((base) + (0x0730))
#define SOC_SMMU_OVERRIDE_CTRL_S_ADDR(base) ((base) + (0x0734))
#define SOC_SMMU_MAIR_S_ADDR(base) ((base) + (0x0738))
#define SOC_SMMU_SMRX_S1_ADDR(base,n) ((base) + ((n)*0x4+0x740))
#define SOC_SMMU_SMRX_P_ADDR(base,n) ((base) + (0x10000+(n)*0x4))
#define SOC_SMMU_RLD_EN0_P_ADDR(base) ((base) + (0x101F0))
#define SOC_SMMU_RLD_EN1_P_ADDR(base) ((base) + (0x101F4))
#define SOC_SMMU_RLD_EN2_P_ADDR(base) ((base) + (0x101F8))
#define SOC_SMMU_INTMAS_P_ADDR(base) ((base) + (0x10200))
#define SOC_SMMU_INTRAW_P_ADDR(base) ((base) + (0x10204))
#define SOC_SMMU_INTSTAT_P_ADDR(base) ((base) + (0x10208))
#define SOC_SMMU_INTCLR_P_ADDR(base) ((base) + (0x1020C))
#define SOC_SMMU_SCR_P_ADDR(base) ((base) + (0x10210))
#define SOC_SMMU_PCB_SCTRL_ADDR(base) ((base) + (0x10214))
#define SOC_SMMU_PCB_TTBR_ADDR(base) ((base) + (0x10218))
#define SOC_SMMU_PCB_TTBCR_ADDR(base) ((base) + (0x1021C))
#define SOC_SMMU_OFFSET_ADDR_P_ADDR(base) ((base) + (0x10220))
#define SOC_SMMU_FAMA_CTRL0_P_ADDR(base) ((base) + (0x10224))
#define SOC_SMMU_FAMA_CTRL1_P_ADDR(base) ((base) + (0x10228))
#define SOC_SMMU_MAIR_P_ADDR(base) ((base) + (0x1022C))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_bypass : 1;
        unsigned int rqos_en : 1;
        unsigned int wqos_en : 1;
        unsigned int glb_alloc_en : 1;
        unsigned int cache_l1_en : 1;
        unsigned int cache_l2_en : 1;
        unsigned int rqos : 4;
        unsigned int wqos : 4;
        unsigned int reserved_0 : 1;
        unsigned int smr_rd_shadow : 1;
        unsigned int ptw_pf : 4;
        unsigned int ptw_mid : 6;
        unsigned int ctx_racfg_ns : 2;
        unsigned int ctx_wacfg_ns : 2;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_SMMU_SCR_UNION;
#endif
#define SOC_SMMU_SCR_glb_bypass_START (0)
#define SOC_SMMU_SCR_glb_bypass_END (0)
#define SOC_SMMU_SCR_rqos_en_START (1)
#define SOC_SMMU_SCR_rqos_en_END (1)
#define SOC_SMMU_SCR_wqos_en_START (2)
#define SOC_SMMU_SCR_wqos_en_END (2)
#define SOC_SMMU_SCR_glb_alloc_en_START (3)
#define SOC_SMMU_SCR_glb_alloc_en_END (3)
#define SOC_SMMU_SCR_cache_l1_en_START (4)
#define SOC_SMMU_SCR_cache_l1_en_END (4)
#define SOC_SMMU_SCR_cache_l2_en_START (5)
#define SOC_SMMU_SCR_cache_l2_en_END (5)
#define SOC_SMMU_SCR_rqos_START (6)
#define SOC_SMMU_SCR_rqos_END (9)
#define SOC_SMMU_SCR_wqos_START (10)
#define SOC_SMMU_SCR_wqos_END (13)
#define SOC_SMMU_SCR_smr_rd_shadow_START (15)
#define SOC_SMMU_SCR_smr_rd_shadow_END (15)
#define SOC_SMMU_SCR_ptw_pf_START (16)
#define SOC_SMMU_SCR_ptw_pf_END (19)
#define SOC_SMMU_SCR_ptw_mid_START (20)
#define SOC_SMMU_SCR_ptw_mid_END (25)
#define SOC_SMMU_SCR_ctx_racfg_ns_START (26)
#define SOC_SMMU_SCR_ctx_racfg_ns_END (27)
#define SOC_SMMU_SCR_ctx_wacfg_ns_START (28)
#define SOC_SMMU_SCR_ctx_wacfg_ns_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mem_ctrl_s : 26;
        unsigned int reserved : 6;
    } reg;
} SOC_SMMU_MEMCTRL_UNION;
#endif
#define SOC_SMMU_MEMCTRL_mem_ctrl_s_START (0)
#define SOC_SMMU_MEMCTRL_mem_ctrl_s_END (25)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int auto_clk_gt_en : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_SMMU_LP_CTRL_UNION;
#endif
#define SOC_SMMU_LP_CTRL_auto_clk_gt_en_START (0)
#define SOC_SMMU_LP_CTRL_auto_clk_gt_en_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int remap_sel0 : 2;
        unsigned int remap_sel1 : 2;
        unsigned int remap_sel2 : 2;
        unsigned int remap_sel3 : 2;
        unsigned int remap_sel4 : 2;
        unsigned int remap_sel5 : 2;
        unsigned int remap_sel6 : 2;
        unsigned int remap_sel7 : 2;
        unsigned int remap_sel8 : 2;
        unsigned int remap_sel9 : 2;
        unsigned int remap_sel10 : 2;
        unsigned int remap_sel11 : 2;
        unsigned int remap_sel12 : 2;
        unsigned int remap_sel13 : 2;
        unsigned int remap_sel14 : 2;
        unsigned int remap_sel15 : 2;
    } reg;
} SOC_SMMU_PRESS_REMAP_UNION;
#endif
#define SOC_SMMU_PRESS_REMAP_remap_sel0_START (0)
#define SOC_SMMU_PRESS_REMAP_remap_sel0_END (1)
#define SOC_SMMU_PRESS_REMAP_remap_sel1_START (2)
#define SOC_SMMU_PRESS_REMAP_remap_sel1_END (3)
#define SOC_SMMU_PRESS_REMAP_remap_sel2_START (4)
#define SOC_SMMU_PRESS_REMAP_remap_sel2_END (5)
#define SOC_SMMU_PRESS_REMAP_remap_sel3_START (6)
#define SOC_SMMU_PRESS_REMAP_remap_sel3_END (7)
#define SOC_SMMU_PRESS_REMAP_remap_sel4_START (8)
#define SOC_SMMU_PRESS_REMAP_remap_sel4_END (9)
#define SOC_SMMU_PRESS_REMAP_remap_sel5_START (10)
#define SOC_SMMU_PRESS_REMAP_remap_sel5_END (11)
#define SOC_SMMU_PRESS_REMAP_remap_sel6_START (12)
#define SOC_SMMU_PRESS_REMAP_remap_sel6_END (13)
#define SOC_SMMU_PRESS_REMAP_remap_sel7_START (14)
#define SOC_SMMU_PRESS_REMAP_remap_sel7_END (15)
#define SOC_SMMU_PRESS_REMAP_remap_sel8_START (16)
#define SOC_SMMU_PRESS_REMAP_remap_sel8_END (17)
#define SOC_SMMU_PRESS_REMAP_remap_sel9_START (18)
#define SOC_SMMU_PRESS_REMAP_remap_sel9_END (19)
#define SOC_SMMU_PRESS_REMAP_remap_sel10_START (20)
#define SOC_SMMU_PRESS_REMAP_remap_sel10_END (21)
#define SOC_SMMU_PRESS_REMAP_remap_sel11_START (22)
#define SOC_SMMU_PRESS_REMAP_remap_sel11_END (23)
#define SOC_SMMU_PRESS_REMAP_remap_sel12_START (24)
#define SOC_SMMU_PRESS_REMAP_remap_sel12_END (25)
#define SOC_SMMU_PRESS_REMAP_remap_sel13_START (26)
#define SOC_SMMU_PRESS_REMAP_remap_sel13_END (27)
#define SOC_SMMU_PRESS_REMAP_remap_sel14_START (28)
#define SOC_SMMU_PRESS_REMAP_remap_sel14_END (29)
#define SOC_SMMU_PRESS_REMAP_remap_sel15_START (30)
#define SOC_SMMU_PRESS_REMAP_remap_sel15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intns_permis_msk : 1;
        unsigned int intns_ext_msk : 1;
        unsigned int intns_tlbmiss_msk : 1;
        unsigned int intns_ptw_trans_msk : 1;
        unsigned int intns_ptw_invalid_msk : 1;
        unsigned int intns_ptw_ns_msk : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTMASK_NS_UNION;
#endif
#define SOC_SMMU_INTMASK_NS_intns_permis_msk_START (0)
#define SOC_SMMU_INTMASK_NS_intns_permis_msk_END (0)
#define SOC_SMMU_INTMASK_NS_intns_ext_msk_START (1)
#define SOC_SMMU_INTMASK_NS_intns_ext_msk_END (1)
#define SOC_SMMU_INTMASK_NS_intns_tlbmiss_msk_START (2)
#define SOC_SMMU_INTMASK_NS_intns_tlbmiss_msk_END (2)
#define SOC_SMMU_INTMASK_NS_intns_ptw_trans_msk_START (3)
#define SOC_SMMU_INTMASK_NS_intns_ptw_trans_msk_END (3)
#define SOC_SMMU_INTMASK_NS_intns_ptw_invalid_msk_START (4)
#define SOC_SMMU_INTMASK_NS_intns_ptw_invalid_msk_END (4)
#define SOC_SMMU_INTMASK_NS_intns_ptw_ns_msk_START (5)
#define SOC_SMMU_INTMASK_NS_intns_ptw_ns_msk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intns_permis_raw : 1;
        unsigned int intns_ext_raw : 1;
        unsigned int intns_tlbmiss_raw : 1;
        unsigned int intns_ptw_trans_raw : 1;
        unsigned int intns_ptw_invalid_raw : 1;
        unsigned int intns_ptw_ns_raw : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTRAW_NS_UNION;
#endif
#define SOC_SMMU_INTRAW_NS_intns_permis_raw_START (0)
#define SOC_SMMU_INTRAW_NS_intns_permis_raw_END (0)
#define SOC_SMMU_INTRAW_NS_intns_ext_raw_START (1)
#define SOC_SMMU_INTRAW_NS_intns_ext_raw_END (1)
#define SOC_SMMU_INTRAW_NS_intns_tlbmiss_raw_START (2)
#define SOC_SMMU_INTRAW_NS_intns_tlbmiss_raw_END (2)
#define SOC_SMMU_INTRAW_NS_intns_ptw_trans_raw_START (3)
#define SOC_SMMU_INTRAW_NS_intns_ptw_trans_raw_END (3)
#define SOC_SMMU_INTRAW_NS_intns_ptw_invalid_raw_START (4)
#define SOC_SMMU_INTRAW_NS_intns_ptw_invalid_raw_END (4)
#define SOC_SMMU_INTRAW_NS_intns_ptw_ns_raw_START (5)
#define SOC_SMMU_INTRAW_NS_intns_ptw_ns_raw_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intns_permis_stat : 1;
        unsigned int intns_ext_stat : 1;
        unsigned int intns_tlbmiss_stat : 1;
        unsigned int intns_ptw_trans_stat : 1;
        unsigned int intns_ptw_invalid_stat : 1;
        unsigned int intns_ptw_ns_stat : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTSTAT_NS_UNION;
#endif
#define SOC_SMMU_INTSTAT_NS_intns_permis_stat_START (0)
#define SOC_SMMU_INTSTAT_NS_intns_permis_stat_END (0)
#define SOC_SMMU_INTSTAT_NS_intns_ext_stat_START (1)
#define SOC_SMMU_INTSTAT_NS_intns_ext_stat_END (1)
#define SOC_SMMU_INTSTAT_NS_intns_tlbmiss_stat_START (2)
#define SOC_SMMU_INTSTAT_NS_intns_tlbmiss_stat_END (2)
#define SOC_SMMU_INTSTAT_NS_intns_ptw_trans_stat_START (3)
#define SOC_SMMU_INTSTAT_NS_intns_ptw_trans_stat_END (3)
#define SOC_SMMU_INTSTAT_NS_intns_ptw_invalid_stat_START (4)
#define SOC_SMMU_INTSTAT_NS_intns_ptw_invalid_stat_END (4)
#define SOC_SMMU_INTSTAT_NS_intns_ptw_ns_stat_START (5)
#define SOC_SMMU_INTSTAT_NS_intns_ptw_ns_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intns_permis_clr : 1;
        unsigned int intns_ext_clr : 1;
        unsigned int intns_tlbmiss_clr : 1;
        unsigned int intns_ptw_trans_clr : 1;
        unsigned int intns_ptw_invalid_clr : 1;
        unsigned int intns_ptw_ns_clr : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTCLR_NS_UNION;
#endif
#define SOC_SMMU_INTCLR_NS_intns_permis_clr_START (0)
#define SOC_SMMU_INTCLR_NS_intns_permis_clr_END (0)
#define SOC_SMMU_INTCLR_NS_intns_ext_clr_START (1)
#define SOC_SMMU_INTCLR_NS_intns_ext_clr_END (1)
#define SOC_SMMU_INTCLR_NS_intns_tlbmiss_clr_START (2)
#define SOC_SMMU_INTCLR_NS_intns_tlbmiss_clr_END (2)
#define SOC_SMMU_INTCLR_NS_intns_ptw_trans_clr_START (3)
#define SOC_SMMU_INTCLR_NS_intns_ptw_trans_clr_END (3)
#define SOC_SMMU_INTCLR_NS_intns_ptw_invalid_clr_START (4)
#define SOC_SMMU_INTCLR_NS_intns_ptw_invalid_clr_END (4)
#define SOC_SMMU_INTCLR_NS_intns_ptw_ns_clr_START (5)
#define SOC_SMMU_INTCLR_NS_intns_ptw_ns_clr_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_bypass : 1;
        unsigned int ptw_racfg_ns : 1;
        unsigned int smr_ptw_qos : 2;
        unsigned int smr_invld_en : 1;
        unsigned int tbu_racfg_ns : 2;
        unsigned int tbu_wacfg_ns : 2;
        unsigned int tbu_alloc_en_ns : 1;
        unsigned int reserved : 2;
        unsigned int smr_offset_addr : 20;
    } reg;
} SOC_SMMU_SMRX_NS_UNION;
#endif
#define SOC_SMMU_SMRX_NS_smr_bypass_START (0)
#define SOC_SMMU_SMRX_NS_smr_bypass_END (0)
#define SOC_SMMU_SMRX_NS_ptw_racfg_ns_START (1)
#define SOC_SMMU_SMRX_NS_ptw_racfg_ns_END (1)
#define SOC_SMMU_SMRX_NS_smr_ptw_qos_START (2)
#define SOC_SMMU_SMRX_NS_smr_ptw_qos_END (3)
#define SOC_SMMU_SMRX_NS_smr_invld_en_START (4)
#define SOC_SMMU_SMRX_NS_smr_invld_en_END (4)
#define SOC_SMMU_SMRX_NS_tbu_racfg_ns_START (5)
#define SOC_SMMU_SMRX_NS_tbu_racfg_ns_END (6)
#define SOC_SMMU_SMRX_NS_tbu_wacfg_ns_START (7)
#define SOC_SMMU_SMRX_NS_tbu_wacfg_ns_END (8)
#define SOC_SMMU_SMRX_NS_tbu_alloc_en_ns_START (9)
#define SOC_SMMU_SMRX_NS_tbu_alloc_en_ns_END (9)
#define SOC_SMMU_SMRX_NS_smr_offset_addr_START (12)
#define SOC_SMMU_SMRX_NS_smr_offset_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en0 : 32;
    } reg;
} SOC_SMMU_RLD_EN0_NS_UNION;
#endif
#define SOC_SMMU_RLD_EN0_NS_smr_rld_en0_START (0)
#define SOC_SMMU_RLD_EN0_NS_smr_rld_en0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en1 : 32;
    } reg;
} SOC_SMMU_RLD_EN1_NS_UNION;
#endif
#define SOC_SMMU_RLD_EN1_NS_smr_rld_en1_START (0)
#define SOC_SMMU_RLD_EN1_NS_smr_rld_en1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en2 : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SMMU_RLD_EN2_NS_UNION;
#endif
#define SOC_SMMU_RLD_EN2_NS_smr_rld_en2_START (0)
#define SOC_SMMU_RLD_EN2_NS_smr_rld_en2_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_alloc_finish : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_GLB_ALLOC_ST_UNION;
#endif
#define SOC_SMMU_GLB_ALLOC_ST_glb_alloc_finish_START (0)
#define SOC_SMMU_GLB_ALLOC_ST_glb_alloc_finish_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cb_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_CB_SCTRL_UNION;
#endif
#define SOC_SMMU_CB_SCTRL_cb_bypass_START (0)
#define SOC_SMMU_CB_SCTRL_cb_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cb_ttbr0 : 32;
    } reg;
} SOC_SMMU_CB_TTBR0_UNION;
#endif
#define SOC_SMMU_CB_TTBR0_cb_ttbr0_START (0)
#define SOC_SMMU_CB_TTBR0_cb_ttbr0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cb_ttbr1 : 32;
    } reg;
} SOC_SMMU_CB_TTBR1_UNION;
#endif
#define SOC_SMMU_CB_TTBR1_cb_ttbr1_START (0)
#define SOC_SMMU_CB_TTBR1_cb_ttbr1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cb_ttbcr_des : 1;
        unsigned int cb_ttbcr_t0sz : 3;
        unsigned int cb_ttbcr_n : 3;
        unsigned int reserved_0 : 9;
        unsigned int cb_ttbcr_t1sz : 3;
        unsigned int reserved_1 : 13;
    } reg;
} SOC_SMMU_CB_TTBCR_UNION;
#endif
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_des_START (0)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_des_END (0)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_t0sz_START (1)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_t0sz_END (3)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_n_START (4)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_n_END (6)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_t1sz_START (16)
#define SOC_SMMU_CB_TTBCR_cb_ttbcr_t1sz_END (18)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int offset_addr_ns : 14;
        unsigned int reserved : 18;
    } reg;
} SOC_SMMU_OFFSET_ADDR_NS_UNION;
#endif
#define SOC_SMMU_OFFSET_ADDR_NS_offset_addr_ns_START (0)
#define SOC_SMMU_OFFSET_ADDR_NS_offset_addr_ns_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cache_all_invalid : 1;
        unsigned int cache_all_level : 2;
        unsigned int reserved : 29;
    } reg;
} SOC_SMMU_SCACHEI_ALL_UNION;
#endif
#define SOC_SMMU_SCACHEI_ALL_cache_all_invalid_START (0)
#define SOC_SMMU_SCACHEI_ALL_cache_all_invalid_END (0)
#define SOC_SMMU_SCACHEI_ALL_cache_all_level_START (1)
#define SOC_SMMU_SCACHEI_ALL_cache_all_level_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cache_l1_invalid : 1;
        unsigned int cache_l1_security : 2;
        unsigned int reserved : 29;
    } reg;
} SOC_SMMU_SCACHEI_L1_UNION;
#endif
#define SOC_SMMU_SCACHEI_L1_cache_l1_invalid_START (0)
#define SOC_SMMU_SCACHEI_L1_cache_l1_invalid_END (0)
#define SOC_SMMU_SCACHEI_L1_cache_l1_security_START (1)
#define SOC_SMMU_SCACHEI_L1_cache_l1_security_END (2)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cache_l2l3_invalid : 1;
        unsigned int cache_l2l3_strmid : 15;
        unsigned int reserved : 16;
    } reg;
} SOC_SMMU_SCACHEI_L2L3_UNION;
#endif
#define SOC_SMMU_SCACHEI_L2L3_cache_l2l3_invalid_START (0)
#define SOC_SMMU_SCACHEI_L2L3_cache_l2l3_invalid_END (0)
#define SOC_SMMU_SCACHEI_L2L3_cache_l2l3_strmid_START (1)
#define SOC_SMMU_SCACHEI_L2L3_cache_l2l3_strmid_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fama_sdes_msb_ns : 7;
        unsigned int fama_chn_sel_ns : 1;
        unsigned int fama_bps_msb_ns : 6;
        unsigned int reserved : 18;
    } reg;
} SOC_SMMU_FAMA_CTRL0_NS_UNION;
#endif
#define SOC_SMMU_FAMA_CTRL0_NS_fama_sdes_msb_ns_START (0)
#define SOC_SMMU_FAMA_CTRL0_NS_fama_sdes_msb_ns_END (6)
#define SOC_SMMU_FAMA_CTRL0_NS_fama_chn_sel_ns_START (7)
#define SOC_SMMU_FAMA_CTRL0_NS_fama_chn_sel_ns_END (7)
#define SOC_SMMU_FAMA_CTRL0_NS_fama_bps_msb_ns_START (8)
#define SOC_SMMU_FAMA_CTRL0_NS_fama_bps_msb_ns_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fama_ptw_msb_ns : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_SMMU_FAMA_CTRL1_NS_UNION;
#endif
#define SOC_SMMU_FAMA_CTRL1_NS_fama_ptw_msb_ns_START (0)
#define SOC_SMMU_FAMA_CTRL1_NS_fama_ptw_msb_ns_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mair_ns : 32;
    } reg;
} SOC_SMMU_MAIR_NS_UNION;
#endif
#define SOC_SMMU_MAIR_NS_mair_ns_START (0)
#define SOC_SMMU_MAIR_NS_mair_ns_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int msb_errrd : 7;
        unsigned int msb_errwr : 7;
        unsigned int msb_ova : 7;
        unsigned int reserved : 11;
    } reg;
} SOC_SMMU_ADDR_MSB_UNION;
#endif
#define SOC_SMMU_ADDR_MSB_msb_errrd_START (0)
#define SOC_SMMU_ADDR_MSB_msb_errrd_END (6)
#define SOC_SMMU_ADDR_MSB_msb_errwr_START (7)
#define SOC_SMMU_ADDR_MSB_msb_errwr_END (13)
#define SOC_SMMU_ADDR_MSB_msb_ova_START (14)
#define SOC_SMMU_ADDR_MSB_msb_ova_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int err_rd_addr : 32;
    } reg;
} SOC_SMMU_ERR_RDADDR_UNION;
#endif
#define SOC_SMMU_ERR_RDADDR_err_rd_addr_START (0)
#define SOC_SMMU_ERR_RDADDR_err_rd_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int err_wr_addr : 32;
    } reg;
} SOC_SMMU_ERR_WRADDR_UNION;
#endif
#define SOC_SMMU_ERR_WRADDR_err_wr_addr_START (0)
#define SOC_SMMU_ERR_WRADDR_err_wr_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fault_addr_tcu : 32;
    } reg;
} SOC_SMMU_FAULT_ADDR_TCU_UNION;
#endif
#define SOC_SMMU_FAULT_ADDR_TCU_fault_addr_tcu_START (0)
#define SOC_SMMU_FAULT_ADDR_TCU_fault_addr_tcu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fault_index_id_tcu : 16;
        unsigned int fault_strm_id_tcu : 16;
    } reg;
} SOC_SMMU_FAULT_ID_TCU_UNION;
#endif
#define SOC_SMMU_FAULT_ID_TCU_fault_index_id_tcu_START (0)
#define SOC_SMMU_FAULT_ID_TCU_fault_index_id_tcu_END (15)
#define SOC_SMMU_FAULT_ID_TCU_fault_strm_id_tcu_START (16)
#define SOC_SMMU_FAULT_ID_TCU_fault_strm_id_tcu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fault_addr_tbu : 32;
    } reg;
} SOC_SMMU_FAULT_ADDR_TBUX_UNION;
#endif
#define SOC_SMMU_FAULT_ADDR_TBUX_fault_addr_tbu_START (0)
#define SOC_SMMU_FAULT_ADDR_TBUX_fault_addr_tbu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fault_index_id_tbu : 16;
        unsigned int fault_strm_id_tbu : 16;
    } reg;
} SOC_SMMU_FAULT_ID_TBUX_UNION;
#endif
#define SOC_SMMU_FAULT_ID_TBUX_fault_index_id_tbu_START (0)
#define SOC_SMMU_FAULT_ID_TBUX_fault_index_id_tbu_END (15)
#define SOC_SMMU_FAULT_ID_TBUX_fault_strm_id_tbu_START (16)
#define SOC_SMMU_FAULT_ID_TBUX_fault_strm_id_tbu_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fault_tlbmiss_err_tbu : 2;
        unsigned int fault_permis_err_tbu : 3;
        unsigned int fautl_ext_err_tbu : 2;
        unsigned int fault_ext_err_id_tbu : 9;
        unsigned int reserved : 16;
    } reg;
} SOC_SMMU_FAULT_INFOX_UNION;
#endif
#define SOC_SMMU_FAULT_INFOX_fault_tlbmiss_err_tbu_START (0)
#define SOC_SMMU_FAULT_INFOX_fault_tlbmiss_err_tbu_END (1)
#define SOC_SMMU_FAULT_INFOX_fault_permis_err_tbu_START (2)
#define SOC_SMMU_FAULT_INFOX_fault_permis_err_tbu_END (4)
#define SOC_SMMU_FAULT_INFOX_fautl_ext_err_tbu_START (5)
#define SOC_SMMU_FAULT_INFOX_fautl_ext_err_tbu_END (6)
#define SOC_SMMU_FAULT_INFOX_fault_ext_err_id_tbu_START (7)
#define SOC_SMMU_FAULT_INFOX_fault_ext_err_id_tbu_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_tlbword_pointer : 3;
        unsigned int dbg_tlbentry_pointer : 13;
        unsigned int reserved_0 : 14;
        unsigned int dbg_tlb_type : 1;
        unsigned int reserved_1 : 1;
    } reg;
} SOC_SMMU_DBGRPTR_TLB_UNION;
#endif
#define SOC_SMMU_DBGRPTR_TLB_dbg_tlbword_pointer_START (0)
#define SOC_SMMU_DBGRPTR_TLB_dbg_tlbword_pointer_END (2)
#define SOC_SMMU_DBGRPTR_TLB_dbg_tlbentry_pointer_START (3)
#define SOC_SMMU_DBGRPTR_TLB_dbg_tlbentry_pointer_END (15)
#define SOC_SMMU_DBGRPTR_TLB_dbg_tlb_type_START (30)
#define SOC_SMMU_DBGRPTR_TLB_dbg_tlb_type_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_tlb_rdata : 31;
        unsigned int reserved : 1;
    } reg;
} SOC_SMMU_DBGRDATA_TLB_UNION;
#endif
#define SOC_SMMU_DBGRDATA_TLB_dbg_tlb_rdata_START (0)
#define SOC_SMMU_DBGRDATA_TLB_dbg_tlb_rdata_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cache_level : 2;
        unsigned int dbg_cache_l1_pointer : 2;
        unsigned int dbg_cache_l1_ns : 1;
        unsigned int dbg_cache_l2_strmid : 11;
        unsigned int dbg_cache_l1_prot : 1;
        unsigned int reserved : 15;
    } reg;
} SOC_SMMU_DBGRPTR_CACHE_UNION;
#endif
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_level_START (0)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_level_END (1)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l1_pointer_START (2)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l1_pointer_END (3)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l1_ns_START (4)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l1_ns_END (4)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l2_strmid_START (5)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l2_strmid_END (15)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l1_prot_START (16)
#define SOC_SMMU_DBGRPTR_CACHE_dbg_cache_l1_prot_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cache_rdata0 : 29;
        unsigned int reserved : 3;
    } reg;
} SOC_SMMU_DBGRDATA0_CACHE_UNION;
#endif
#define SOC_SMMU_DBGRDATA0_CACHE_dbg_cache_rdata0_START (0)
#define SOC_SMMU_DBGRDATA0_CACHE_dbg_cache_rdata0_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cache_rdata1 : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_SMMU_DBGRDATA1_CACHE_UNION;
#endif
#define SOC_SMMU_DBGRDATA1_CACHE_dbg_cache_rdata1_START (0)
#define SOC_SMMU_DBGRDATA1_CACHE_dbg_cache_rdata1_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_axilock_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_DBGAXI_CTRL_UNION;
#endif
#define SOC_SMMU_DBGAXI_CTRL_dbg_axilock_en_START (0)
#define SOC_SMMU_DBGAXI_CTRL_dbg_axilock_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int override_va : 32;
    } reg;
} SOC_SMMU_OVA_ADDR_UNION;
#endif
#define SOC_SMMU_OVA_ADDR_override_va_START (0)
#define SOC_SMMU_OVA_ADDR_override_va_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int override_acquire_pa : 27;
        unsigned int reserved : 4;
        unsigned int override_pa_done : 1;
    } reg;
} SOC_SMMU_OPA_ADDR_UNION;
#endif
#define SOC_SMMU_OPA_ADDR_override_acquire_pa_START (0)
#define SOC_SMMU_OPA_ADDR_override_acquire_pa_END (26)
#define SOC_SMMU_OPA_ADDR_override_pa_done_START (31)
#define SOC_SMMU_OPA_ADDR_override_pa_done_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int override_va_cfg : 1;
        unsigned int reserved : 1;
        unsigned int override_va_type : 1;
        unsigned int override_va_indexid : 13;
        unsigned int override_va_strmid : 12;
        unsigned int override_tbu_num : 4;
    } reg;
} SOC_SMMU_OVA_CTRL_UNION;
#endif
#define SOC_SMMU_OVA_CTRL_override_va_cfg_START (0)
#define SOC_SMMU_OVA_CTRL_override_va_cfg_END (0)
#define SOC_SMMU_OVA_CTRL_override_va_type_START (2)
#define SOC_SMMU_OVA_CTRL_override_va_type_END (2)
#define SOC_SMMU_OVA_CTRL_override_va_indexid_START (3)
#define SOC_SMMU_OVA_CTRL_override_va_indexid_END (15)
#define SOC_SMMU_OVA_CTRL_override_va_strmid_START (16)
#define SOC_SMMU_OVA_CTRL_override_va_strmid_END (27)
#define SOC_SMMU_OVA_CTRL_override_tbu_num_START (28)
#define SOC_SMMU_OVA_CTRL_override_tbu_num_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int override_pref_addr : 32;
    } reg;
} SOC_SMMU_OPREF_ADDR_UNION;
#endif
#define SOC_SMMU_OPREF_ADDR_override_pref_addr_START (0)
#define SOC_SMMU_OPREF_ADDR_override_pref_addr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int override_pref_cfg : 1;
        unsigned int reserved : 1;
        unsigned int override_pref_type : 1;
        unsigned int override_pref_initial : 1;
        unsigned int override_pref_indexid : 12;
        unsigned int override_pref_strmid : 16;
    } reg;
} SOC_SMMU_OPREF_CTRL_UNION;
#endif
#define SOC_SMMU_OPREF_CTRL_override_pref_cfg_START (0)
#define SOC_SMMU_OPREF_CTRL_override_pref_cfg_END (0)
#define SOC_SMMU_OPREF_CTRL_override_pref_type_START (2)
#define SOC_SMMU_OPREF_CTRL_override_pref_type_END (2)
#define SOC_SMMU_OPREF_CTRL_override_pref_initial_START (3)
#define SOC_SMMU_OPREF_CTRL_override_pref_initial_END (3)
#define SOC_SMMU_OPREF_CTRL_override_pref_indexid_START (4)
#define SOC_SMMU_OPREF_CTRL_override_pref_indexid_END (15)
#define SOC_SMMU_OPREF_CTRL_override_pref_strmid_START (16)
#define SOC_SMMU_OPREF_CTRL_override_pref_strmid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int override_pref_cnt : 32;
    } reg;
} SOC_SMMU_OPREF_CNT_UNION;
#endif
#define SOC_SMMU_OPREF_CNT_override_pref_cnt_START (0)
#define SOC_SMMU_OPREF_CNT_override_pref_cnt_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_nscfg : 1;
        unsigned int smr_nscfg_en : 1;
        unsigned int smr_bypass_s : 1;
        unsigned int ptw_mid_sel_s : 1;
        unsigned int reserved : 1;
        unsigned int smr_mid_en_s : 1;
        unsigned int smr_mid_s : 6;
        unsigned int smr_offset_addr_s : 20;
    } reg;
} SOC_SMMU_SMRX_S_UNION;
#endif
#define SOC_SMMU_SMRX_S_smr_nscfg_START (0)
#define SOC_SMMU_SMRX_S_smr_nscfg_END (0)
#define SOC_SMMU_SMRX_S_smr_nscfg_en_START (1)
#define SOC_SMMU_SMRX_S_smr_nscfg_en_END (1)
#define SOC_SMMU_SMRX_S_smr_bypass_s_START (2)
#define SOC_SMMU_SMRX_S_smr_bypass_s_END (2)
#define SOC_SMMU_SMRX_S_ptw_mid_sel_s_START (3)
#define SOC_SMMU_SMRX_S_ptw_mid_sel_s_END (3)
#define SOC_SMMU_SMRX_S_smr_mid_en_s_START (5)
#define SOC_SMMU_SMRX_S_smr_mid_en_s_END (5)
#define SOC_SMMU_SMRX_S_smr_mid_s_START (6)
#define SOC_SMMU_SMRX_S_smr_mid_s_END (11)
#define SOC_SMMU_SMRX_S_smr_offset_addr_s_START (12)
#define SOC_SMMU_SMRX_S_smr_offset_addr_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en0_s : 32;
    } reg;
} SOC_SMMU_RLD_EN0_S_UNION;
#endif
#define SOC_SMMU_RLD_EN0_S_smr_rld_en0_s_START (0)
#define SOC_SMMU_RLD_EN0_S_smr_rld_en0_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en1_s : 32;
    } reg;
} SOC_SMMU_RLD_EN1_S_UNION;
#endif
#define SOC_SMMU_RLD_EN1_S_smr_rld_en1_s_START (0)
#define SOC_SMMU_RLD_EN1_S_smr_rld_en1_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en2_s : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SMMU_RLD_EN2_S_UNION;
#endif
#define SOC_SMMU_RLD_EN2_S_smr_rld_en2_s_START (0)
#define SOC_SMMU_RLD_EN2_S_smr_rld_en2_s_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ints_permis_msk : 1;
        unsigned int ints_ext_msk : 1;
        unsigned int ints_tlbmiss_msk : 1;
        unsigned int ints_ptw_trans_msk : 1;
        unsigned int ints_ptw_invalid_msk : 1;
        unsigned int ints_ptw_ns_msk : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTMAS_S_UNION;
#endif
#define SOC_SMMU_INTMAS_S_ints_permis_msk_START (0)
#define SOC_SMMU_INTMAS_S_ints_permis_msk_END (0)
#define SOC_SMMU_INTMAS_S_ints_ext_msk_START (1)
#define SOC_SMMU_INTMAS_S_ints_ext_msk_END (1)
#define SOC_SMMU_INTMAS_S_ints_tlbmiss_msk_START (2)
#define SOC_SMMU_INTMAS_S_ints_tlbmiss_msk_END (2)
#define SOC_SMMU_INTMAS_S_ints_ptw_trans_msk_START (3)
#define SOC_SMMU_INTMAS_S_ints_ptw_trans_msk_END (3)
#define SOC_SMMU_INTMAS_S_ints_ptw_invalid_msk_START (4)
#define SOC_SMMU_INTMAS_S_ints_ptw_invalid_msk_END (4)
#define SOC_SMMU_INTMAS_S_ints_ptw_ns_msk_START (5)
#define SOC_SMMU_INTMAS_S_ints_ptw_ns_msk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ints_permis_raw : 1;
        unsigned int ints_ext_raw : 1;
        unsigned int ints_tlbmiss_raw : 1;
        unsigned int ints_ptw_trans_raw : 1;
        unsigned int ints_ptw_invalid_raw : 1;
        unsigned int ints_ptw_ns_raw : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTRAW_S_UNION;
#endif
#define SOC_SMMU_INTRAW_S_ints_permis_raw_START (0)
#define SOC_SMMU_INTRAW_S_ints_permis_raw_END (0)
#define SOC_SMMU_INTRAW_S_ints_ext_raw_START (1)
#define SOC_SMMU_INTRAW_S_ints_ext_raw_END (1)
#define SOC_SMMU_INTRAW_S_ints_tlbmiss_raw_START (2)
#define SOC_SMMU_INTRAW_S_ints_tlbmiss_raw_END (2)
#define SOC_SMMU_INTRAW_S_ints_ptw_trans_raw_START (3)
#define SOC_SMMU_INTRAW_S_ints_ptw_trans_raw_END (3)
#define SOC_SMMU_INTRAW_S_ints_ptw_invalid_raw_START (4)
#define SOC_SMMU_INTRAW_S_ints_ptw_invalid_raw_END (4)
#define SOC_SMMU_INTRAW_S_ints_ptw_ns_raw_START (5)
#define SOC_SMMU_INTRAW_S_ints_ptw_ns_raw_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ints_permis_stat : 1;
        unsigned int ints_ext_stat : 1;
        unsigned int ints_tlbmiss_stat : 1;
        unsigned int ints_ptw_trans_stat : 1;
        unsigned int ints_ptw_invalid_stat : 1;
        unsigned int ints_ptw_ns_stat : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTSTAT_S_UNION;
#endif
#define SOC_SMMU_INTSTAT_S_ints_permis_stat_START (0)
#define SOC_SMMU_INTSTAT_S_ints_permis_stat_END (0)
#define SOC_SMMU_INTSTAT_S_ints_ext_stat_START (1)
#define SOC_SMMU_INTSTAT_S_ints_ext_stat_END (1)
#define SOC_SMMU_INTSTAT_S_ints_tlbmiss_stat_START (2)
#define SOC_SMMU_INTSTAT_S_ints_tlbmiss_stat_END (2)
#define SOC_SMMU_INTSTAT_S_ints_ptw_trans_stat_START (3)
#define SOC_SMMU_INTSTAT_S_ints_ptw_trans_stat_END (3)
#define SOC_SMMU_INTSTAT_S_ints_ptw_invalid_stat_START (4)
#define SOC_SMMU_INTSTAT_S_ints_ptw_invalid_stat_END (4)
#define SOC_SMMU_INTSTAT_S_ints_ptw_ns_stat_START (5)
#define SOC_SMMU_INTSTAT_S_ints_ptw_ns_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ints_permis_clr : 1;
        unsigned int ints_ext_clr : 1;
        unsigned int ints_tlbmiss_clr : 1;
        unsigned int ints_ptw_trans_clr : 1;
        unsigned int ints_ptw_invalid_clr : 1;
        unsigned int ints_ptw_ns_clr : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTCLR_S_UNION;
#endif
#define SOC_SMMU_INTCLR_S_ints_permis_clr_START (0)
#define SOC_SMMU_INTCLR_S_ints_permis_clr_END (0)
#define SOC_SMMU_INTCLR_S_ints_ext_clr_START (1)
#define SOC_SMMU_INTCLR_S_ints_ext_clr_END (1)
#define SOC_SMMU_INTCLR_S_ints_tlbmiss_clr_START (2)
#define SOC_SMMU_INTCLR_S_ints_tlbmiss_clr_END (2)
#define SOC_SMMU_INTCLR_S_ints_ptw_trans_clr_START (3)
#define SOC_SMMU_INTCLR_S_ints_ptw_trans_clr_END (3)
#define SOC_SMMU_INTCLR_S_ints_ptw_invalid_clr_START (4)
#define SOC_SMMU_INTCLR_S_ints_ptw_invalid_clr_END (4)
#define SOC_SMMU_INTCLR_S_ints_ptw_ns_clr_START (5)
#define SOC_SMMU_INTCLR_S_ints_ptw_ns_clr_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_nscfg : 2;
        unsigned int mid_s : 6;
        unsigned int glb_bypass_s : 1;
        unsigned int mid_en_s : 1;
        unsigned int reserved_0 : 16;
        unsigned int ctx_racfg_s : 2;
        unsigned int ctx_wacfg_s : 2;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_SMMU_SCR_S_UNION;
#endif
#define SOC_SMMU_SCR_S_glb_nscfg_START (0)
#define SOC_SMMU_SCR_S_glb_nscfg_END (1)
#define SOC_SMMU_SCR_S_mid_s_START (2)
#define SOC_SMMU_SCR_S_mid_s_END (7)
#define SOC_SMMU_SCR_S_glb_bypass_s_START (8)
#define SOC_SMMU_SCR_S_glb_bypass_s_END (8)
#define SOC_SMMU_SCR_S_mid_en_s_START (9)
#define SOC_SMMU_SCR_S_mid_en_s_END (9)
#define SOC_SMMU_SCR_S_ctx_racfg_s_START (26)
#define SOC_SMMU_SCR_S_ctx_racfg_s_END (27)
#define SOC_SMMU_SCR_S_ctx_wacfg_s_START (28)
#define SOC_SMMU_SCR_S_ctx_wacfg_s_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scb_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_SCB_SCTRL_UNION;
#endif
#define SOC_SMMU_SCB_SCTRL_scb_bypass_START (0)
#define SOC_SMMU_SCB_SCTRL_scb_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scb_ttbr : 32;
    } reg;
} SOC_SMMU_SCB_TTBR_UNION;
#endif
#define SOC_SMMU_SCB_TTBR_scb_ttbr_START (0)
#define SOC_SMMU_SCB_TTBR_scb_ttbr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int scb_ttbcr_des : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_SCB_TTBCR_UNION;
#endif
#define SOC_SMMU_SCB_TTBCR_scb_ttbcr_des_START (0)
#define SOC_SMMU_SCB_TTBCR_scb_ttbcr_des_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int offset_addr_s : 14;
        unsigned int reserved : 18;
    } reg;
} SOC_SMMU_OFFSET_ADDR_S_UNION;
#endif
#define SOC_SMMU_OFFSET_ADDR_S_offset_addr_s_START (0)
#define SOC_SMMU_OFFSET_ADDR_S_offset_addr_s_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fama_sdes_msb_s : 7;
        unsigned int fama_chn_sel_s : 1;
        unsigned int fama_bps_msb_s : 6;
        unsigned int reserved : 18;
    } reg;
} SOC_SMMU_FAMA_CTRL0_S_UNION;
#endif
#define SOC_SMMU_FAMA_CTRL0_S_fama_sdes_msb_s_START (0)
#define SOC_SMMU_FAMA_CTRL0_S_fama_sdes_msb_s_END (6)
#define SOC_SMMU_FAMA_CTRL0_S_fama_chn_sel_s_START (7)
#define SOC_SMMU_FAMA_CTRL0_S_fama_chn_sel_s_END (7)
#define SOC_SMMU_FAMA_CTRL0_S_fama_bps_msb_s_START (8)
#define SOC_SMMU_FAMA_CTRL0_S_fama_bps_msb_s_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fama_ptw_msb_s : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_SMMU_FAMA_CTRL1_S_UNION;
#endif
#define SOC_SMMU_FAMA_CTRL1_S_fama_ptw_msb_s_START (0)
#define SOC_SMMU_FAMA_CTRL1_S_fama_ptw_msb_s_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_tlb_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_DBGRPTR_TLB_S_UNION;
#endif
#define SOC_SMMU_DBGRPTR_TLB_S_dbg_tlb_en_START (0)
#define SOC_SMMU_DBGRPTR_TLB_S_dbg_tlb_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dbg_cache_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_DBGRPTR_CACHE_S_UNION;
#endif
#define SOC_SMMU_DBGRPTR_CACHE_S_dbg_cache_en_START (0)
#define SOC_SMMU_DBGRPTR_CACHE_S_dbg_cache_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int override_va_security : 1;
        unsigned int reserved_1 : 30;
    } reg;
} SOC_SMMU_OVERRIDE_CTRL_S_UNION;
#endif
#define SOC_SMMU_OVERRIDE_CTRL_S_override_va_security_START (1)
#define SOC_SMMU_OVERRIDE_CTRL_S_override_va_security_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mair_s : 32;
    } reg;
} SOC_SMMU_MAIR_S_UNION;
#endif
#define SOC_SMMU_MAIR_S_mair_s_START (0)
#define SOC_SMMU_MAIR_S_mair_s_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int ptw_racfg_s : 1;
        unsigned int reserved_1 : 3;
        unsigned int tbu_racfg_s : 2;
        unsigned int tbu_wacfg_s : 2;
        unsigned int tbu_alloc_en_s : 1;
        unsigned int reserved_2 : 22;
    } reg;
} SOC_SMMU_SMRX_S1_UNION;
#endif
#define SOC_SMMU_SMRX_S1_ptw_racfg_s_START (1)
#define SOC_SMMU_SMRX_S1_ptw_racfg_s_END (1)
#define SOC_SMMU_SMRX_S1_tbu_racfg_s_START (5)
#define SOC_SMMU_SMRX_S1_tbu_racfg_s_END (6)
#define SOC_SMMU_SMRX_S1_tbu_wacfg_s_START (7)
#define SOC_SMMU_SMRX_S1_tbu_wacfg_s_END (8)
#define SOC_SMMU_SMRX_S1_tbu_alloc_en_s_START (9)
#define SOC_SMMU_SMRX_S1_tbu_alloc_en_s_END (9)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_protect_en : 1;
        unsigned int ptw_racfg_p : 1;
        unsigned int smr_bypass_p : 1;
        unsigned int reserved_0 : 2;
        unsigned int tbu_racfg_p : 2;
        unsigned int tbu_wacfg_p : 2;
        unsigned int tbu_alloc_en_p : 1;
        unsigned int reserved_1 : 2;
        unsigned int smr_offset_addr_p : 20;
    } reg;
} SOC_SMMU_SMRX_P_UNION;
#endif
#define SOC_SMMU_SMRX_P_smr_protect_en_START (0)
#define SOC_SMMU_SMRX_P_smr_protect_en_END (0)
#define SOC_SMMU_SMRX_P_ptw_racfg_p_START (1)
#define SOC_SMMU_SMRX_P_ptw_racfg_p_END (1)
#define SOC_SMMU_SMRX_P_smr_bypass_p_START (2)
#define SOC_SMMU_SMRX_P_smr_bypass_p_END (2)
#define SOC_SMMU_SMRX_P_tbu_racfg_p_START (5)
#define SOC_SMMU_SMRX_P_tbu_racfg_p_END (6)
#define SOC_SMMU_SMRX_P_tbu_wacfg_p_START (7)
#define SOC_SMMU_SMRX_P_tbu_wacfg_p_END (8)
#define SOC_SMMU_SMRX_P_tbu_alloc_en_p_START (9)
#define SOC_SMMU_SMRX_P_tbu_alloc_en_p_END (9)
#define SOC_SMMU_SMRX_P_smr_offset_addr_p_START (12)
#define SOC_SMMU_SMRX_P_smr_offset_addr_p_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en0_p : 32;
    } reg;
} SOC_SMMU_RLD_EN0_P_UNION;
#endif
#define SOC_SMMU_RLD_EN0_P_smr_rld_en0_p_START (0)
#define SOC_SMMU_RLD_EN0_P_smr_rld_en0_p_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en1_p : 32;
    } reg;
} SOC_SMMU_RLD_EN1_P_UNION;
#endif
#define SOC_SMMU_RLD_EN1_P_smr_rld_en1_p_START (0)
#define SOC_SMMU_RLD_EN1_P_smr_rld_en1_p_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int smr_rld_en2_p : 24;
        unsigned int reserved : 8;
    } reg;
} SOC_SMMU_RLD_EN2_P_UNION;
#endif
#define SOC_SMMU_RLD_EN2_P_smr_rld_en2_p_START (0)
#define SOC_SMMU_RLD_EN2_P_smr_rld_en2_p_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intp_permis_msk : 1;
        unsigned int intp_ext_msk : 1;
        unsigned int intp_tlbmiss_msk : 1;
        unsigned int intp_ptw_trans_msk : 1;
        unsigned int intp_ptw_invalid_msk : 1;
        unsigned int intp_ptw_ns_msk : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTMAS_P_UNION;
#endif
#define SOC_SMMU_INTMAS_P_intp_permis_msk_START (0)
#define SOC_SMMU_INTMAS_P_intp_permis_msk_END (0)
#define SOC_SMMU_INTMAS_P_intp_ext_msk_START (1)
#define SOC_SMMU_INTMAS_P_intp_ext_msk_END (1)
#define SOC_SMMU_INTMAS_P_intp_tlbmiss_msk_START (2)
#define SOC_SMMU_INTMAS_P_intp_tlbmiss_msk_END (2)
#define SOC_SMMU_INTMAS_P_intp_ptw_trans_msk_START (3)
#define SOC_SMMU_INTMAS_P_intp_ptw_trans_msk_END (3)
#define SOC_SMMU_INTMAS_P_intp_ptw_invalid_msk_START (4)
#define SOC_SMMU_INTMAS_P_intp_ptw_invalid_msk_END (4)
#define SOC_SMMU_INTMAS_P_intp_ptw_ns_msk_START (5)
#define SOC_SMMU_INTMAS_P_intp_ptw_ns_msk_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intp_permis_raw : 1;
        unsigned int intp_ext_raw : 1;
        unsigned int intp_tlbmiss_raw : 1;
        unsigned int intp_ptw_trans_raw : 1;
        unsigned int intp_ptw_invalid_raw : 1;
        unsigned int intp_ptw_ns_raw : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTRAW_P_UNION;
#endif
#define SOC_SMMU_INTRAW_P_intp_permis_raw_START (0)
#define SOC_SMMU_INTRAW_P_intp_permis_raw_END (0)
#define SOC_SMMU_INTRAW_P_intp_ext_raw_START (1)
#define SOC_SMMU_INTRAW_P_intp_ext_raw_END (1)
#define SOC_SMMU_INTRAW_P_intp_tlbmiss_raw_START (2)
#define SOC_SMMU_INTRAW_P_intp_tlbmiss_raw_END (2)
#define SOC_SMMU_INTRAW_P_intp_ptw_trans_raw_START (3)
#define SOC_SMMU_INTRAW_P_intp_ptw_trans_raw_END (3)
#define SOC_SMMU_INTRAW_P_intp_ptw_invalid_raw_START (4)
#define SOC_SMMU_INTRAW_P_intp_ptw_invalid_raw_END (4)
#define SOC_SMMU_INTRAW_P_intp_ptw_ns_raw_START (5)
#define SOC_SMMU_INTRAW_P_intp_ptw_ns_raw_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intp_permis_stat : 1;
        unsigned int intp_ext_stat : 1;
        unsigned int intp_tlbmiss_stat : 1;
        unsigned int intp_ptw_trans_stat : 1;
        unsigned int intp_ptw_invalid_stat : 1;
        unsigned int intp_ptw_ns_stat : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTSTAT_P_UNION;
#endif
#define SOC_SMMU_INTSTAT_P_intp_permis_stat_START (0)
#define SOC_SMMU_INTSTAT_P_intp_permis_stat_END (0)
#define SOC_SMMU_INTSTAT_P_intp_ext_stat_START (1)
#define SOC_SMMU_INTSTAT_P_intp_ext_stat_END (1)
#define SOC_SMMU_INTSTAT_P_intp_tlbmiss_stat_START (2)
#define SOC_SMMU_INTSTAT_P_intp_tlbmiss_stat_END (2)
#define SOC_SMMU_INTSTAT_P_intp_ptw_trans_stat_START (3)
#define SOC_SMMU_INTSTAT_P_intp_ptw_trans_stat_END (3)
#define SOC_SMMU_INTSTAT_P_intp_ptw_invalid_stat_START (4)
#define SOC_SMMU_INTSTAT_P_intp_ptw_invalid_stat_END (4)
#define SOC_SMMU_INTSTAT_P_intp_ptw_ns_stat_START (5)
#define SOC_SMMU_INTSTAT_P_intp_ptw_ns_stat_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intp_permis_clr : 1;
        unsigned int intp_ext_clr : 1;
        unsigned int intp_tlbmiss_clr : 1;
        unsigned int intp_ptw_trans_clr : 1;
        unsigned int intp_ptw_invalid_clr : 1;
        unsigned int intp_ptw_ns_clr : 1;
        unsigned int reserved : 26;
    } reg;
} SOC_SMMU_INTCLR_P_UNION;
#endif
#define SOC_SMMU_INTCLR_P_intp_permis_clr_START (0)
#define SOC_SMMU_INTCLR_P_intp_permis_clr_END (0)
#define SOC_SMMU_INTCLR_P_intp_ext_clr_START (1)
#define SOC_SMMU_INTCLR_P_intp_ext_clr_END (1)
#define SOC_SMMU_INTCLR_P_intp_tlbmiss_clr_START (2)
#define SOC_SMMU_INTCLR_P_intp_tlbmiss_clr_END (2)
#define SOC_SMMU_INTCLR_P_intp_ptw_trans_clr_START (3)
#define SOC_SMMU_INTCLR_P_intp_ptw_trans_clr_END (3)
#define SOC_SMMU_INTCLR_P_intp_ptw_invalid_clr_START (4)
#define SOC_SMMU_INTCLR_P_intp_ptw_invalid_clr_END (4)
#define SOC_SMMU_INTCLR_P_intp_ptw_ns_clr_START (5)
#define SOC_SMMU_INTCLR_P_intp_ptw_ns_clr_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int glb_prot_cfg : 1;
        unsigned int reserved_0 : 7;
        unsigned int glb_bypass_p : 1;
        unsigned int reserved_1 : 17;
        unsigned int ctx_racfg_p : 2;
        unsigned int ctx_wacfg_p : 2;
        unsigned int reserved_2 : 2;
    } reg;
} SOC_SMMU_SCR_P_UNION;
#endif
#define SOC_SMMU_SCR_P_glb_prot_cfg_START (0)
#define SOC_SMMU_SCR_P_glb_prot_cfg_END (0)
#define SOC_SMMU_SCR_P_glb_bypass_p_START (8)
#define SOC_SMMU_SCR_P_glb_bypass_p_END (8)
#define SOC_SMMU_SCR_P_ctx_racfg_p_START (26)
#define SOC_SMMU_SCR_P_ctx_racfg_p_END (27)
#define SOC_SMMU_SCR_P_ctx_wacfg_p_START (28)
#define SOC_SMMU_SCR_P_ctx_wacfg_p_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcb_bypass : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_PCB_SCTRL_UNION;
#endif
#define SOC_SMMU_PCB_SCTRL_pcb_bypass_START (0)
#define SOC_SMMU_PCB_SCTRL_pcb_bypass_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcb_ttbr : 32;
    } reg;
} SOC_SMMU_PCB_TTBR_UNION;
#endif
#define SOC_SMMU_PCB_TTBR_pcb_ttbr_START (0)
#define SOC_SMMU_PCB_TTBR_pcb_ttbr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pcb_ttbcr_des : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_SMMU_PCB_TTBCR_UNION;
#endif
#define SOC_SMMU_PCB_TTBCR_pcb_ttbcr_des_START (0)
#define SOC_SMMU_PCB_TTBCR_pcb_ttbcr_des_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int offset_addr_p : 14;
        unsigned int reserved : 18;
    } reg;
} SOC_SMMU_OFFSET_ADDR_P_UNION;
#endif
#define SOC_SMMU_OFFSET_ADDR_P_offset_addr_p_START (0)
#define SOC_SMMU_OFFSET_ADDR_P_offset_addr_p_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fama_sdes_msb_p : 7;
        unsigned int fama_chn_sel_p : 1;
        unsigned int fama_bps_msb_p : 6;
        unsigned int reserved : 18;
    } reg;
} SOC_SMMU_FAMA_CTRL0_P_UNION;
#endif
#define SOC_SMMU_FAMA_CTRL0_P_fama_sdes_msb_p_START (0)
#define SOC_SMMU_FAMA_CTRL0_P_fama_sdes_msb_p_END (6)
#define SOC_SMMU_FAMA_CTRL0_P_fama_chn_sel_p_START (7)
#define SOC_SMMU_FAMA_CTRL0_P_fama_chn_sel_p_END (7)
#define SOC_SMMU_FAMA_CTRL0_P_fama_bps_msb_p_START (8)
#define SOC_SMMU_FAMA_CTRL0_P_fama_bps_msb_p_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int fama_ptw_msb_p : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_SMMU_FAMA_CTRL1_P_UNION;
#endif
#define SOC_SMMU_FAMA_CTRL1_P_fama_ptw_msb_p_START (0)
#define SOC_SMMU_FAMA_CTRL1_P_fama_ptw_msb_p_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mair_p : 32;
    } reg;
} SOC_SMMU_MAIR_P_UNION;
#endif
#define SOC_SMMU_MAIR_P_mair_p_START (0)
#define SOC_SMMU_MAIR_P_mair_p_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
