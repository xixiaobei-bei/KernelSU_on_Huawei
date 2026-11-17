#ifndef __SOC_DMSS_INTERFACE_H__
#define __SOC_DMSS_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#define SOC_DMSS_ASI_RTL_INF0_ADDR(base,asi_base) ((base) + (0x000+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF1_ADDR(base,asi_base) ((base) + (0x004+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF2_ADDR(base,asi_base) ((base) + (0x008+0x800*(asi_base)))
#define SOC_DMSS_ASI_RTL_INF3_ADDR(base,asi_base) ((base) + (0x00C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STA_CKG_ADDR(base,asi_base) ((base) + (0x010+0x800*(asi_base)))
#define SOC_DMSS_ASI_DYN_CKG_ADDR(base,asi_base) ((base) + (0x014+0x800*(asi_base)))
#define SOC_DMSS_ASI_ASYNC_BRG_ADDR(base,asi_base) ((base) + (0x018+0x800*(asi_base)))
#define SOC_DMSS_ASI_ADDR_SHIFT_ADDR(base,asi_base) ((base) + (0x020+0x800*(asi_base)))
#define SOC_DMSS_ASI_RATE_ADPT_ADDR(base,asi_base) ((base) + (0x030+0x800*(asi_base)))
#define SOC_DMSS_ASI_RDR_CTRL0_ADDR(base,asi_base) ((base) + (0x040+0x800*(asi_base)))
#define SOC_DMSS_ASI_RDR_CTRL1_ADDR(base,asi_base) ((base) + (0x044+0x800*(asi_base)))
#define SOC_DMSS_ASI_RDR_CTRL2_ADDR(base,asi_base) ((base) + (0x048+0x800*(asi_base)))
#define SOC_DMSS_ASI_RBUF_INTLV0_ADDR(base,asi_base) ((base) + (0x060+0x800*(asi_base)))
#define SOC_DMSS_ASI_RBUF_INTLV1_ADDR(base,asi_base) ((base) + (0x064+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_CTRL_ADDR(base,asi_base) ((base) + (0x100+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_WRPRI_ADDR(base,asi_base) ((base) + (0x104+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDPRI_ADDR(base,asi_base) ((base) + (0x108+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_ADPT_ADDR(base,asi_base) ((base) + (0x10C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LATENCY_ADDR(base,asi_base) ((base) + (0x110+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR0_ADDR(base,asi_base) ((base) + (0x120+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_ADDR(base,asi_base) ((base) + (0x124+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR1_ADDR(base,asi_base) ((base) + (0x128+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_ADDR(base,asi_base) ((base) + (0x12C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR0_ADDR(base,asi_base) ((base) + (0x140+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR1_ADDR(base,asi_base) ((base) + (0x144+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR2_ADDR(base,asi_base) ((base) + (0x148+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_ADDR(base,asi_base) ((base) + (0x150+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_ADDR(base,asi_base) ((base) + (0x154+0x800*(asi_base)))
#define SOC_DMSS_ASI_DCQ_CMDLVL_ADDR(base,asi_base) ((base) + (0x15C+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDOSTD_ADDR(base,asi_base) ((base) + (0x160+0x800*(asi_base)))
#define SOC_DMSS_ASI_QOS_RDOSTD_ST_ADDR(base,asi_base) ((base) + (0x164+0x800*(asi_base)))
#define SOC_DMSS_ASI_PUSH_WRMID_ADDR(base,asi_base) ((base) + (0x180+0x800*(asi_base)))
#define SOC_DMSS_ASI_PUSH_RDMID_ADDR(base,asi_base) ((base) + (0x184+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_MASK_ADDR(base,asi_base) ((base) + (0x200+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_CYCLE_ADDR(base,asi_base) ((base) + (0x208+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_ADDR(base,asi_base) ((base) + (0x210+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_ADDR(base,asi_base) ((base) + (0x214+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_WR_ADDR(base,asi_base) ((base) + (0x218+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_FLUX_RD_ADDR(base,asi_base) ((base) + (0x21C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR0_ADDR(base,asi_base) ((base) + (0x220+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR1_ADDR(base,asi_base) ((base) + (0x224+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_LMTR2_ADDR(base,asi_base) ((base) + (0x228+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_RDLAT_ADDR(base,asi_base) ((base) + (0x22C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI0_ADDR(base,asi_base) ((base) + (0x230+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI1_ADDR(base,asi_base) ((base) + (0x234+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI2_ADDR(base,asi_base) ((base) + (0x238+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI3_ADDR(base,asi_base) ((base) + (0x23C+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI4_ADDR(base,asi_base) ((base) + (0x240+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI5_ADDR(base,asi_base) ((base) + (0x244+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI6_ADDR(base,asi_base) ((base) + (0x248+0x800*(asi_base)))
#define SOC_DMSS_ASI_STAT_PRI7_ADDR(base,asi_base) ((base) + (0x24C+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_LD_SEL_ADDR(base,asi_base) ((base) + (0x410+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_INT_EN_ADDR(base,asi_base) ((base) + (0x420+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_INT_STATUS_ADDR(base,asi_base) ((base) + (0x424+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_ADDR(base,asi_base) ((base) + (0x480+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_ADDR(base,asi_base) ((base) + (0x484+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_ADDR(base,asi_base) ((base) + (0x488+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_ITCRG_ADDR(base,asi_base) ((base) + (0x4C0+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_ITIP_ADDR(base,asi_base) ((base) + (0x4C4+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_ITOP_ADDR(base,asi_base) ((base) + (0x4C8+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_RGN_MAP_ADDR(base,sec_rgns,asi_base) ((base) + (0x500+0x10*(sec_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_ADDR(base,sec_rgns,asi_base) ((base) + (0x504+0x10*(sec_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_MID_WR_ADDR(base,sec_rgns,asi_base) ((base) + (0x508+0x10*(sec_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_SEC_MID_RD_ADDR(base,sec_rgns,asi_base) ((base) + (0x50C+0x10*(sec_rgns)+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_MODULE_ADDR(base,asi_base) ((base) + (0x700+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_FIFO_ADDR(base,asi_base) ((base) + (0x704+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_FSM_ADDR(base,asi_base) ((base) + (0x708+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_WROSTD_ADDR(base,asi_base) ((base) + (0x70C+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_RDR0_ADDR(base,asi_base) ((base) + (0x710+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_RDR1_ADDR(base,asi_base) ((base) + (0x714+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_INT_EN_ADDR(base,asi_base) ((base) + (0x720+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_INT_STATUS_ADDR(base,asi_base) ((base) + (0x724+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_ABNM_INF_ADDR(base,asi_base) ((base) + (0x728+0x800*(asi_base)))
#define SOC_DMSS_ASI_DFX_LOAD_ADDR(base,asi_base) ((base) + (0x760+0x800*(asi_base)))
#define SOC_DMSS_GLB_RTL_VER_ADDR(base) ((base) + (0x6000))
#define SOC_DMSS_GLB_RTL_INF0_ADDR(base) ((base) + (0x6004))
#define SOC_DMSS_GLB_RTL_INF1_ADDR(base) ((base) + (0x6008))
#define SOC_DMSS_GLB_RAM_TMOD_ADDR(base) ((base) + (0x6018))
#define SOC_DMSS_GLB_CFG_LOCK_ADDR(base) ((base) + (0x6020))
#define SOC_DMSS_GLB_DYN_CKG_ADDR(base) ((base) + (0x6024))
#define SOC_DMSS_GLB_INT_STATUS_ADDR(base) ((base) + (0x6030))
#define SOC_DMSS_GLB_INT_CLEAR_ADDR(base) ((base) + (0x6034))
#define SOC_DMSS_GLB_EXCLU_ADDR(base) ((base) + (0x6040))
#define SOC_DMSS_GLB_ADDR_INTLV_ADDR(base) ((base) + (0x6100))
#define SOC_DMSS_GLB_ADDR_SCRMBL_ADDR(base) ((base) + (0x6110))
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_ADDR(base) ((base) + (0x6140))
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_ADDR(base) ((base) + (0x6144))
#define SOC_DMSS_GLB_RBUF_BYP_PATH_ADDR(base) ((base) + (0x6148))
#define SOC_DMSS_GLB_RBUF_BP_ADDR(base,rbuf_bp_grps) ((base) + (0x6150+0x4*(rbuf_bp_grps)))
#define SOC_DMSS_GLB_DCQ_CTRL_ADDR(base) ((base) + (0x6200))
#define SOC_DMSS_GLB_DCQ_ST_ADDR(base) ((base) + (0x6204))
#define SOC_DMSS_GLB_DCQ_PRILVL0_ADDR(base) ((base) + (0x6208))
#define SOC_DMSS_GLB_DCQ_PRILVL1_ADDR(base) ((base) + (0x620C))
#define SOC_DMSS_GLB_DCQ_MST_ADDR(base,dcq_msts) ((base) + (0x6210+0x4*(dcq_msts)))
#define SOC_DMSS_GLB_DCQ_INT_EN_ADDR(base) ((base) + (0x6250))
#define SOC_DMSS_GLB_STAT_CTRL_ADDR(base) ((base) + (0x6260))
#define SOC_DMSS_GLB_STAT_ALL_ALLOW_ADDR(base) ((base) + (0x6270))
#define SOC_DMSS_GLB_STAT_PRI1_ALLOW_ADDR(base) ((base) + (0x6274))
#define SOC_DMSS_GLB_STAT_PRI2_ALLOW_ADDR(base) ((base) + (0x6278))
#define SOC_DMSS_GLB_STAT_PRI3_ALLOW_ADDR(base) ((base) + (0x627C))
#define SOC_DMSS_GLB_STAT_PRI4_ALLOW_ADDR(base) ((base) + (0x6280))
#define SOC_DMSS_GLB_STAT_PRI5_ALLOW_ADDR(base) ((base) + (0x6284))
#define SOC_DMSS_GLB_STAT_PRI6_ALLOW_ADDR(base) ((base) + (0x6288))
#define SOC_DMSS_GLB_STAT_PRI7_ALLOW_ADDR(base) ((base) + (0x628C))
#define SOC_DMSS_GLB_DFX_RBUF0_ADDR(base,chans) ((base) + (0x6700+0x10*(chans)))
#define SOC_DMSS_GLB_DFX_RBUF1_ADDR(base,chans) ((base) + (0x6704+0x10*(chans)))
#define SOC_DMSS_GLB_DFX_RBUF2_ADDR(base,chans) ((base) + (0x6708+0x10*(chans)))
#define SOC_DMSS_GLB_DFX_RBUF3_ADDR(base,chans) ((base) + (0x670C+0x10*(chans)))
#define SOC_DMSS_GLB_DFX_DMI_GRNT_ADDR(base) ((base) + (0x6740))
#define SOC_DMSS_GLB_DFX_DCQ_ADDR(base) ((base) + (0x6744))
#define SOC_DMSS_GLB_DFX_ASI_BUSY_ADDR(base) ((base) + (0x6750))
#define SOC_DMSS_GLB_DFX_LOAD_ADDR(base) ((base) + (0x6760))
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_data_width : 1;
        unsigned int reserved_0 : 3;
        unsigned int rtl_clk_mode : 2;
        unsigned int reserved_1 : 10;
        unsigned int rtl_rdr_que_depth : 2;
        unsigned int reserved_2 : 2;
        unsigned int rtl_rdr_buf_num : 2;
        unsigned int reserved_3 : 2;
        unsigned int rtl_rdr_en : 1;
        unsigned int reserved_4 : 7;
    } reg;
} SOC_DMSS_ASI_RTL_INF0_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF0_rtl_data_width_START (0)
#define SOC_DMSS_ASI_RTL_INF0_rtl_data_width_END (0)
#define SOC_DMSS_ASI_RTL_INF0_rtl_clk_mode_START (4)
#define SOC_DMSS_ASI_RTL_INF0_rtl_clk_mode_END (5)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_que_depth_START (16)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_que_depth_END (17)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_buf_num_START (20)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_buf_num_END (21)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_en_START (24)
#define SOC_DMSS_ASI_RTL_INF0_rtl_rdr_en_END (24)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_wcmd_fifo_depth : 5;
        unsigned int reserved_0 : 3;
        unsigned int rtl_wd_fifo_depth : 5;
        unsigned int reserved_1 : 3;
        unsigned int rtl_b_fifo_depth : 5;
        unsigned int reserved_2 : 3;
        unsigned int rtl_ra_fifo_depth : 5;
        unsigned int reserved_3 : 3;
    } reg;
} SOC_DMSS_ASI_RTL_INF1_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF1_rtl_wcmd_fifo_depth_START (0)
#define SOC_DMSS_ASI_RTL_INF1_rtl_wcmd_fifo_depth_END (4)
#define SOC_DMSS_ASI_RTL_INF1_rtl_wd_fifo_depth_START (8)
#define SOC_DMSS_ASI_RTL_INF1_rtl_wd_fifo_depth_END (12)
#define SOC_DMSS_ASI_RTL_INF1_rtl_b_fifo_depth_START (16)
#define SOC_DMSS_ASI_RTL_INF1_rtl_b_fifo_depth_END (20)
#define SOC_DMSS_ASI_RTL_INF1_rtl_ra_fifo_depth_START (24)
#define SOC_DMSS_ASI_RTL_INF1_rtl_ra_fifo_depth_END (28)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_rbar_fifo_depth : 5;
        unsigned int reserved_0 : 3;
        unsigned int rtl_rd_fifo_depth : 5;
        unsigned int reserved_1 : 19;
    } reg;
} SOC_DMSS_ASI_RTL_INF2_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF2_rtl_rbar_fifo_depth_START (0)
#define SOC_DMSS_ASI_RTL_INF2_rtl_rbar_fifo_depth_END (4)
#define SOC_DMSS_ASI_RTL_INF2_rtl_rd_fifo_depth_START (8)
#define SOC_DMSS_ASI_RTL_INF2_rtl_rd_fifo_depth_END (12)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_sec_rgn_num : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_DMSS_ASI_RTL_INF3_UNION;
#endif
#define SOC_DMSS_ASI_RTL_INF3_rtl_sec_rgn_num_START (0)
#define SOC_DMSS_ASI_RTL_INF3_rtl_sec_rgn_num_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int gt_en_asi : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_STA_CKG_UNION;
#endif
#define SOC_DMSS_ASI_STA_CKG_gt_en_asi_START (0)
#define SOC_DMSS_ASI_STA_CKG_gt_en_asi_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_byp_asi : 1;
        unsigned int ckg_byp_ra : 1;
        unsigned int ckg_byp_ax : 1;
        unsigned int ckg_byp_cs : 1;
        unsigned int ckg_byp_wd : 1;
        unsigned int ckg_byp_b : 1;
        unsigned int ckg_byp_rd : 1;
        unsigned int ckg_byp_fkrd : 1;
        unsigned int ckg_byp_sec : 1;
        unsigned int ckg_byp_rdr_est : 1;
        unsigned int ckg_byp_rdr_buf : 1;
        unsigned int ckg_byp_async : 1;
        unsigned int reserved : 20;
    } reg;
} SOC_DMSS_ASI_DYN_CKG_UNION;
#endif
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_asi_START (0)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_asi_END (0)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ra_START (1)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ra_END (1)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ax_START (2)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_ax_END (2)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_cs_START (3)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_cs_END (3)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_wd_START (4)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_wd_END (4)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_b_START (5)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_b_END (5)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rd_START (6)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rd_END (6)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_fkrd_START (7)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_fkrd_END (7)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_sec_START (8)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_sec_END (8)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_est_START (9)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_est_END (9)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_buf_START (10)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_rdr_buf_END (10)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_async_START (11)
#define SOC_DMSS_ASI_DYN_CKG_ckg_byp_async_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cactive_wl : 7;
        unsigned int reserved : 25;
    } reg;
} SOC_DMSS_ASI_ASYNC_BRG_UNION;
#endif
#define SOC_DMSS_ASI_ASYNC_BRG_cactive_wl_START (0)
#define SOC_DMSS_ASI_ASYNC_BRG_cactive_wl_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int addr_shift_mode : 2;
        unsigned int reserved : 30;
    } reg;
} SOC_DMSS_ASI_ADDR_SHIFT_UNION;
#endif
#define SOC_DMSS_ASI_ADDR_SHIFT_addr_shift_mode_START (0)
#define SOC_DMSS_ASI_ADDR_SHIFT_addr_shift_mode_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ra_en : 1;
        unsigned int reserved_0 : 3;
        unsigned int ra_trsfr_wl : 4;
        unsigned int reserved_1 : 24;
    } reg;
} SOC_DMSS_ASI_RATE_ADPT_UNION;
#endif
#define SOC_DMSS_ASI_RATE_ADPT_ra_en_START (0)
#define SOC_DMSS_ASI_RATE_ADPT_ra_en_END (0)
#define SOC_DMSS_ASI_RATE_ADPT_ra_trsfr_wl_START (4)
#define SOC_DMSS_ASI_RATE_ADPT_ra_trsfr_wl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_id_match : 32;
    } reg;
} SOC_DMSS_ASI_RDR_CTRL0_UNION;
#endif
#define SOC_DMSS_ASI_RDR_CTRL0_rdr_id_match_START (0)
#define SOC_DMSS_ASI_RDR_CTRL0_rdr_id_match_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_id_mask : 32;
    } reg;
} SOC_DMSS_ASI_RDR_CTRL1_UNION;
#endif
#define SOC_DMSS_ASI_RDR_CTRL1_rdr_id_mask_START (0)
#define SOC_DMSS_ASI_RDR_CTRL1_rdr_id_mask_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_id_mode : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_RDR_CTRL2_UNION;
#endif
#define SOC_DMSS_ASI_RDR_CTRL2_rdr_id_mode_START (0)
#define SOC_DMSS_ASI_RDR_CTRL2_rdr_id_mode_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intlv_mid_0 : 2;
        unsigned int intlv_mid_1 : 2;
        unsigned int intlv_mid_2 : 2;
        unsigned int intlv_mid_3 : 2;
        unsigned int intlv_mid_4 : 2;
        unsigned int intlv_mid_5 : 2;
        unsigned int intlv_mid_6 : 2;
        unsigned int intlv_mid_7 : 2;
        unsigned int intlv_mid_8 : 2;
        unsigned int intlv_mid_9 : 2;
        unsigned int intlv_mid_10 : 2;
        unsigned int intlv_mid_11 : 2;
        unsigned int intlv_mid_12 : 2;
        unsigned int intlv_mid_13 : 2;
        unsigned int intlv_mid_14 : 2;
        unsigned int intlv_mid_15 : 2;
    } reg;
} SOC_DMSS_ASI_RBUF_INTLV0_UNION;
#endif
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_0_START (0)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_0_END (1)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_1_START (2)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_1_END (3)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_2_START (4)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_2_END (5)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_3_START (6)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_3_END (7)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_4_START (8)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_4_END (9)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_5_START (10)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_5_END (11)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_6_START (12)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_6_END (13)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_7_START (14)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_7_END (15)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_8_START (16)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_8_END (17)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_9_START (18)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_9_END (19)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_10_START (20)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_10_END (21)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_11_START (22)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_11_END (23)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_12_START (24)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_12_END (25)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_13_START (26)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_13_END (27)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_14_START (28)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_14_END (29)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_15_START (30)
#define SOC_DMSS_ASI_RBUF_INTLV0_intlv_mid_15_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intlv_mid_16 : 2;
        unsigned int intlv_mid_17 : 2;
        unsigned int intlv_mid_18 : 2;
        unsigned int intlv_mid_19 : 2;
        unsigned int intlv_mid_20 : 2;
        unsigned int intlv_mid_21 : 2;
        unsigned int intlv_mid_22 : 2;
        unsigned int intlv_mid_23 : 2;
        unsigned int intlv_mid_24 : 2;
        unsigned int intlv_mid_25 : 2;
        unsigned int intlv_mid_26 : 2;
        unsigned int intlv_mid_27 : 2;
        unsigned int intlv_mid_28 : 2;
        unsigned int intlv_mid_29 : 2;
        unsigned int intlv_mid_30 : 2;
        unsigned int intlv_mid_31 : 2;
    } reg;
} SOC_DMSS_ASI_RBUF_INTLV1_UNION;
#endif
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_16_START (0)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_16_END (1)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_17_START (2)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_17_END (3)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_18_START (4)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_18_END (5)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_19_START (6)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_19_END (7)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_20_START (8)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_20_END (9)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_21_START (10)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_21_END (11)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_22_START (12)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_22_END (13)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_23_START (14)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_23_END (15)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_24_START (16)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_24_END (17)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_25_START (18)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_25_END (19)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_26_START (20)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_26_END (21)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_27_START (22)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_27_END (23)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_28_START (24)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_28_END (25)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_29_START (26)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_29_END (27)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_30_START (28)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_30_END (29)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_31_START (30)
#define SOC_DMSS_ASI_RBUF_INTLV1_intlv_mid_31_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int pri_push_en : 1;
        unsigned int push_qos_en : 1;
        unsigned int hurry_qos_en : 1;
        unsigned int arb_pri_use : 1;
        unsigned int rw_arb_mode : 1;
        unsigned int reserved_0 : 3;
        unsigned int rw_arb_dis : 1;
        unsigned int reserved_1 : 3;
        unsigned int rd_wrap_split_en : 1;
        unsigned int reserved_2 : 3;
        unsigned int dcq_prilvl_en : 1;
        unsigned int reserved_3 : 15;
    } reg;
} SOC_DMSS_ASI_QOS_CTRL_UNION;
#endif
#define SOC_DMSS_ASI_QOS_CTRL_pri_push_en_START (0)
#define SOC_DMSS_ASI_QOS_CTRL_pri_push_en_END (0)
#define SOC_DMSS_ASI_QOS_CTRL_push_qos_en_START (1)
#define SOC_DMSS_ASI_QOS_CTRL_push_qos_en_END (1)
#define SOC_DMSS_ASI_QOS_CTRL_hurry_qos_en_START (2)
#define SOC_DMSS_ASI_QOS_CTRL_hurry_qos_en_END (2)
#define SOC_DMSS_ASI_QOS_CTRL_arb_pri_use_START (3)
#define SOC_DMSS_ASI_QOS_CTRL_arb_pri_use_END (3)
#define SOC_DMSS_ASI_QOS_CTRL_rw_arb_mode_START (4)
#define SOC_DMSS_ASI_QOS_CTRL_rw_arb_mode_END (4)
#define SOC_DMSS_ASI_QOS_CTRL_rw_arb_dis_START (8)
#define SOC_DMSS_ASI_QOS_CTRL_rw_arb_dis_END (8)
#define SOC_DMSS_ASI_QOS_CTRL_rd_wrap_split_en_START (12)
#define SOC_DMSS_ASI_QOS_CTRL_rd_wrap_split_en_END (12)
#define SOC_DMSS_ASI_QOS_CTRL_dcq_prilvl_en_START (16)
#define SOC_DMSS_ASI_QOS_CTRL_dcq_prilvl_en_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_pri0 : 3;
        unsigned int reserved_0: 1;
        unsigned int wr_pri1 : 3;
        unsigned int reserved_1: 1;
        unsigned int wr_pri2 : 3;
        unsigned int reserved_2: 1;
        unsigned int wr_pri3 : 3;
        unsigned int reserved_3: 1;
        unsigned int wr_pri4 : 3;
        unsigned int reserved_4: 1;
        unsigned int wr_pri5 : 3;
        unsigned int reserved_5: 1;
        unsigned int wr_pri6 : 3;
        unsigned int reserved_6: 1;
        unsigned int wr_pri7 : 3;
        unsigned int reserved_7: 1;
    } reg;
} SOC_DMSS_ASI_QOS_WRPRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri0_START (0)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri0_END (2)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri1_START (4)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri1_END (6)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri2_START (8)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri2_END (10)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri3_START (12)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri3_END (14)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri4_START (16)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri4_END (18)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri5_START (20)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri5_END (22)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri6_START (24)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri6_END (26)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri7_START (28)
#define SOC_DMSS_ASI_QOS_WRPRI_wr_pri7_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_pri0 : 3;
        unsigned int reserved_0: 1;
        unsigned int rd_pri1 : 3;
        unsigned int reserved_1: 1;
        unsigned int rd_pri2 : 3;
        unsigned int reserved_2: 1;
        unsigned int rd_pri3 : 3;
        unsigned int reserved_3: 1;
        unsigned int rd_pri4 : 3;
        unsigned int reserved_4: 1;
        unsigned int rd_pri5 : 3;
        unsigned int reserved_5: 1;
        unsigned int rd_pri6 : 3;
        unsigned int reserved_6: 1;
        unsigned int rd_pri7 : 3;
        unsigned int reserved_7: 1;
    } reg;
} SOC_DMSS_ASI_QOS_RDPRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri0_START (0)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri0_END (2)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri1_START (4)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri1_END (6)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri2_START (8)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri2_END (10)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri3_START (12)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri3_END (14)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri4_START (16)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri4_END (18)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri5_START (20)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri5_END (22)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri6_START (24)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri6_END (26)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri7_START (28)
#define SOC_DMSS_ASI_QOS_RDPRI_rd_pri7_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_adpt_low : 4;
        unsigned int rd_adpt_lvl : 3;
        unsigned int reserved_0 : 1;
        unsigned int rd_adpt_high : 4;
        unsigned int reserved_1 : 4;
        unsigned int wr_adpt_low : 4;
        unsigned int wr_adpt_lvl : 3;
        unsigned int reserved_2 : 1;
        unsigned int wr_adpt_high : 4;
        unsigned int reserved_3 : 4;
    } reg;
} SOC_DMSS_ASI_QOS_ADPT_UNION;
#endif
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_low_START (0)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_low_END (3)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_lvl_START (4)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_lvl_END (6)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_high_START (8)
#define SOC_DMSS_ASI_QOS_ADPT_rd_adpt_high_END (11)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_low_START (16)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_low_END (19)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_lvl_START (20)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_lvl_END (22)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_high_START (24)
#define SOC_DMSS_ASI_QOS_ADPT_wr_adpt_high_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lat_target : 12;
        unsigned int lat_factor : 3;
        unsigned int reserved_0 : 1;
        unsigned int lat_pri_min : 3;
        unsigned int reserved_1 : 1;
        unsigned int lat_pri_max : 3;
        unsigned int reserved_2 : 1;
        unsigned int lat_flux_req_dly : 4;
        unsigned int lat_flux_req_en : 1;
        unsigned int lat_quiesce : 1;
        unsigned int lat_mode : 1;
        unsigned int lat_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LATENCY_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LATENCY_lat_target_START (0)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_target_END (11)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_factor_START (12)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_factor_END (14)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_min_START (16)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_min_END (18)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_max_START (20)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_pri_max_END (22)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_dly_START (24)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_dly_END (27)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_en_START (28)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_flux_req_en_END (28)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_quiesce_START (29)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_quiesce_END (29)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_mode_START (30)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_mode_END (30)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_en_START (31)
#define SOC_DMSS_ASI_QOS_LATENCY_lat_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int rglr_bandwidth : 13;
        unsigned int reserved_1 : 1;
        unsigned int rglr_type : 1;
        unsigned int rglr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR0_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_type_START (30)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_type_END (30)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_en_START (31)
#define SOC_DMSS_ASI_QOS_RGLR0_rglr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_pri0 : 3;
        unsigned int reserved_0: 5;
        unsigned int rglr_pri1 : 3;
        unsigned int reserved_1: 21;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR0_PRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri0_START (0)
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri0_END (2)
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri1_START (8)
#define SOC_DMSS_ASI_QOS_RGLR0_PRI_rglr_pri1_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int rglr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int rglr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR1_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_en_START (31)
#define SOC_DMSS_ASI_QOS_RGLR1_rglr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rglr_pri0 : 3;
        unsigned int reserved_0: 5;
        unsigned int rglr_pri1 : 3;
        unsigned int reserved_1: 21;
    } reg;
} SOC_DMSS_ASI_QOS_RGLR1_PRI_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri0_START (0)
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri0_END (2)
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri1_START (8)
#define SOC_DMSS_ASI_QOS_RGLR1_PRI_rglr_pri1_END (10)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 1;
        unsigned int lmtr_type : 1;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR0_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_type_START (30)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_type_END (30)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR0_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR1_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR1_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_saturation : 14;
        unsigned int reserved_0 : 2;
        unsigned int lmtr_bandwidth : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR2_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_saturation_START (0)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_saturation_END (13)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_bandwidth_START (16)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_bandwidth_END (28)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR2_lmtr_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_ovrd_prd : 10;
        unsigned int reserved_0 : 6;
        unsigned int lmtr_ovrd_bw : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_ovrd_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR0_OVRD_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_lmtr_ovrd_prd_START (0)
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_lmtr_ovrd_prd_END (9)
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_lmtr_ovrd_bw_START (16)
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_lmtr_ovrd_bw_END (28)
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_lmtr_ovrd_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR0_OVRD_lmtr_ovrd_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int lmtr_ovrd_prd : 10;
        unsigned int reserved_0 : 6;
        unsigned int lmtr_ovrd_bw : 13;
        unsigned int reserved_1 : 2;
        unsigned int lmtr_ovrd_en : 1;
    } reg;
} SOC_DMSS_ASI_QOS_LMTR1_OVRD_UNION;
#endif
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_lmtr_ovrd_prd_START (0)
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_lmtr_ovrd_prd_END (9)
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_lmtr_ovrd_bw_START (16)
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_lmtr_ovrd_bw_END (28)
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_lmtr_ovrd_en_START (31)
#define SOC_DMSS_ASI_QOS_LMTR1_OVRD_lmtr_ovrd_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_wr_lvl : 8;
        unsigned int dcq_rd_lvl : 8;
        unsigned int dcq_lvl : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_DMSS_ASI_DCQ_CMDLVL_UNION;
#endif
#define SOC_DMSS_ASI_DCQ_CMDLVL_dcq_wr_lvl_START (0)
#define SOC_DMSS_ASI_DCQ_CMDLVL_dcq_wr_lvl_END (7)
#define SOC_DMSS_ASI_DCQ_CMDLVL_dcq_rd_lvl_START (8)
#define SOC_DMSS_ASI_DCQ_CMDLVL_dcq_rd_lvl_END (15)
#define SOC_DMSS_ASI_DCQ_CMDLVL_dcq_lvl_START (16)
#define SOC_DMSS_ASI_DCQ_CMDLVL_dcq_lvl_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_ostd_lvl : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_ASI_QOS_RDOSTD_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RDOSTD_rd_ostd_lvl_START (0)
#define SOC_DMSS_ASI_QOS_RDOSTD_rd_ostd_lvl_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_ostd_st : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_ASI_QOS_RDOSTD_ST_UNION;
#endif
#define SOC_DMSS_ASI_QOS_RDOSTD_ST_rd_ostd_st_START (0)
#define SOC_DMSS_ASI_QOS_RDOSTD_ST_rd_ostd_st_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_mid_sel : 32;
    } reg;
} SOC_DMSS_ASI_PUSH_WRMID_UNION;
#endif
#define SOC_DMSS_ASI_PUSH_WRMID_wr_mid_sel_START (0)
#define SOC_DMSS_ASI_PUSH_WRMID_wr_mid_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rd_mid_sel : 32;
    } reg;
} SOC_DMSS_ASI_PUSH_RDMID_UNION;
#endif
#define SOC_DMSS_ASI_PUSH_RDMID_rd_mid_sel_START (0)
#define SOC_DMSS_ASI_PUSH_RDMID_rd_mid_sel_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_mask_flux_rd : 1;
        unsigned int stat_mask_flux_wr : 1;
        unsigned int reserved_0 : 2;
        unsigned int stat_mask_lmtr0 : 1;
        unsigned int stat_mask_lmtr1 : 1;
        unsigned int stat_mask_lmtr2 : 1;
        unsigned int stat_mask_rdlat : 1;
        unsigned int stat_mask_pri : 1;
        unsigned int reserved_1 : 23;
    } reg;
} SOC_DMSS_ASI_STAT_MASK_UNION;
#endif
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_rd_START (0)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_rd_END (0)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_wr_START (1)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_flux_wr_END (1)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr0_START (4)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr0_END (4)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr1_START (5)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr1_END (5)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr2_START (6)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_lmtr2_END (6)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_rdlat_START (7)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_rdlat_END (7)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_pri_START (8)
#define SOC_DMSS_ASI_STAT_MASK_stat_mask_pri_END (8)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_cycle : 32;
    } reg;
} SOC_DMSS_ASI_STAT_CYCLE_UNION;
#endif
#define SOC_DMSS_ASI_STAT_CYCLE_stat_cycle_START (0)
#define SOC_DMSS_ASI_STAT_CYCLE_stat_cycle_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_id_match : 32;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_FILTER0_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_stat_flux_id_match_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER0_stat_flux_id_match_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_id_mask : 32;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_FILTER1_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_stat_flux_id_mask_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_FILTER1_stat_flux_id_mask_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_wr : 32;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_WR_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_WR_stat_flux_wr_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_WR_stat_flux_wr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_flux_rd : 32;
    } reg;
} SOC_DMSS_ASI_STAT_FLUX_RD_UNION;
#endif
#define SOC_DMSS_ASI_STAT_FLUX_RD_stat_flux_rd_START (0)
#define SOC_DMSS_ASI_STAT_FLUX_RD_stat_flux_rd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr0 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR0_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR0_stat_lmtr0_START (0)
#define SOC_DMSS_ASI_STAT_LMTR0_stat_lmtr0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr1 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR1_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR1_stat_lmtr1_START (0)
#define SOC_DMSS_ASI_STAT_LMTR1_stat_lmtr1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_lmtr2 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_LMTR2_UNION;
#endif
#define SOC_DMSS_ASI_STAT_LMTR2_stat_lmtr2_START (0)
#define SOC_DMSS_ASI_STAT_LMTR2_stat_lmtr2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_rdlat : 32;
    } reg;
} SOC_DMSS_ASI_STAT_RDLAT_UNION;
#endif
#define SOC_DMSS_ASI_STAT_RDLAT_stat_rdlat_START (0)
#define SOC_DMSS_ASI_STAT_RDLAT_stat_rdlat_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri0 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI0_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI0_stat_pri0_START (0)
#define SOC_DMSS_ASI_STAT_PRI0_stat_pri0_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri1 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI1_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI1_stat_pri1_START (0)
#define SOC_DMSS_ASI_STAT_PRI1_stat_pri1_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri2 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI2_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI2_stat_pri2_START (0)
#define SOC_DMSS_ASI_STAT_PRI2_stat_pri2_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri3 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI3_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI3_stat_pri3_START (0)
#define SOC_DMSS_ASI_STAT_PRI3_stat_pri3_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri4 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI4_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI4_stat_pri4_START (0)
#define SOC_DMSS_ASI_STAT_PRI4_stat_pri4_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri5 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI5_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI5_stat_pri5_START (0)
#define SOC_DMSS_ASI_STAT_PRI5_stat_pri5_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri6 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI6_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI6_stat_pri6_START (0)
#define SOC_DMSS_ASI_STAT_PRI6_stat_pri6_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_pri7 : 32;
    } reg;
} SOC_DMSS_ASI_STAT_PRI7_UNION;
#endif
#define SOC_DMSS_ASI_STAT_PRI7_stat_pri7_START (0)
#define SOC_DMSS_ASI_STAT_PRI7_stat_pri7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ld_rgn_map : 1;
        unsigned int ld_rgn_attrib : 1;
        unsigned int ld_mid_rd : 1;
        unsigned int ld_mid_wr : 1;
        unsigned int reserved_0 : 12;
        unsigned int ld_rgns_sel : 5;
        unsigned int reserved_1 : 11;
    } reg;
} SOC_DMSS_ASI_SEC_LD_SEL_UNION;
#endif
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgn_map_START (0)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgn_map_END (0)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgn_attrib_START (1)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgn_attrib_END (1)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_mid_rd_START (2)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_mid_rd_END (2)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_mid_wr_START (3)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_mid_wr_END (3)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgns_sel_START (16)
#define SOC_DMSS_ASI_SEC_LD_SEL_ld_rgns_sel_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_SEC_INT_EN_UNION;
#endif
#define SOC_DMSS_ASI_SEC_INT_EN_int_en_START (0)
#define SOC_DMSS_ASI_SEC_INT_EN_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_status : 1;
        unsigned int int_overrun : 1;
        unsigned int reserved : 30;
    } reg;
} SOC_DMSS_ASI_SEC_INT_STATUS_UNION;
#endif
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_status_START (0)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_status_END (0)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_overrun_START (1)
#define SOC_DMSS_ASI_SEC_INT_STATUS_int_overrun_END (1)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_low : 32;
    } reg;
} SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_UNION;
#endif
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_address_low_START (0)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF0_address_low_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int address_high : 8;
        unsigned int reserved_0 : 8;
        unsigned int privileged : 1;
        unsigned int nonsecure : 1;
        unsigned int reserved_1 : 2;
        unsigned int access_type : 1;
        unsigned int reserved_2 : 11;
    } reg;
} SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_UNION;
#endif
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_address_high_START (0)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_address_high_END (7)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_privileged_START (16)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_privileged_END (16)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_nonsecure_START (17)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_nonsecure_END (17)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_type_START (20)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF1_access_type_END (20)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int id : 24;
        unsigned int mid : 8;
    } reg;
} SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_UNION;
#endif
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_id_START (0)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_id_END (23)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_mid_START (24)
#define SOC_DMSS_ASI_SEC_FAIL_CMD_INF2_mid_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int int_test_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_SEC_ITCRG_UNION;
#endif
#define SOC_DMSS_ASI_SEC_ITCRG_int_test_en_START (0)
#define SOC_DMSS_ASI_SEC_ITCRG_int_test_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itip_secure_boot_lock : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_SEC_ITIP_UNION;
#endif
#define SOC_DMSS_ASI_SEC_ITIP_itip_secure_boot_lock_START (0)
#define SOC_DMSS_ASI_SEC_ITIP_itip_secure_boot_lock_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int itop_int : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_SEC_ITOP_UNION;
#endif
#define SOC_DMSS_ASI_SEC_ITOP_itop_int_START (0)
#define SOC_DMSS_ASI_SEC_ITOP_itop_int_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rgn_base_addr : 24;
        unsigned int rgn_size : 6;
        unsigned int reserved : 1;
        unsigned int rgn_en : 1;
    } reg;
} SOC_DMSS_ASI_SEC_RGN_MAP_UNION;
#endif
#define SOC_DMSS_ASI_SEC_RGN_MAP_rgn_base_addr_START (0)
#define SOC_DMSS_ASI_SEC_RGN_MAP_rgn_base_addr_END (23)
#define SOC_DMSS_ASI_SEC_RGN_MAP_rgn_size_START (24)
#define SOC_DMSS_ASI_SEC_RGN_MAP_rgn_size_END (29)
#define SOC_DMSS_ASI_SEC_RGN_MAP_rgn_en_START (31)
#define SOC_DMSS_ASI_SEC_RGN_MAP_rgn_en_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int sp : 4;
        unsigned int security_inv : 1;
        unsigned int reserved_0 : 3;
        unsigned int mid_en : 1;
        unsigned int mid_inv : 1;
        unsigned int reserved_1 : 6;
        unsigned int subrgn_disable : 16;
    } reg;
} SOC_DMSS_ASI_SEC_RGN_ATTRIB_UNION;
#endif
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_sp_START (0)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_sp_END (3)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_security_inv_START (4)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_security_inv_END (4)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_mid_en_START (8)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_mid_en_END (8)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_mid_inv_START (9)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_mid_inv_END (9)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_subrgn_disable_START (16)
#define SOC_DMSS_ASI_SEC_RGN_ATTRIB_subrgn_disable_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_wr : 32;
    } reg;
} SOC_DMSS_ASI_SEC_MID_WR_UNION;
#endif
#define SOC_DMSS_ASI_SEC_MID_WR_mid_sel_wr_START (0)
#define SOC_DMSS_ASI_SEC_MID_WR_mid_sel_wr_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int mid_sel_rd : 32;
    } reg;
} SOC_DMSS_ASI_SEC_MID_RD_UNION;
#endif
#define SOC_DMSS_ASI_SEC_MID_RD_mid_sel_rd_START (0)
#define SOC_DMSS_ASI_SEC_MID_RD_mid_sel_rd_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_busy : 1;
        unsigned int asi_temp_busy : 1;
        unsigned int reserved_0 : 2;
        unsigned int ra_busy : 1;
        unsigned int ax_busy : 1;
        unsigned int cs_busy : 1;
        unsigned int wd_busy : 1;
        unsigned int b_busy : 1;
        unsigned int rd_busy : 1;
        unsigned int fkrd_busy : 1;
        unsigned int sec_busy : 1;
        unsigned int rdr_est_busy : 1;
        unsigned int rdr_buf_busy : 1;
        unsigned int reserved_1 : 18;
    } reg;
} SOC_DMSS_ASI_DFX_MODULE_UNION;
#endif
#define SOC_DMSS_ASI_DFX_MODULE_asi_busy_START (0)
#define SOC_DMSS_ASI_DFX_MODULE_asi_busy_END (0)
#define SOC_DMSS_ASI_DFX_MODULE_asi_temp_busy_START (1)
#define SOC_DMSS_ASI_DFX_MODULE_asi_temp_busy_END (1)
#define SOC_DMSS_ASI_DFX_MODULE_ra_busy_START (4)
#define SOC_DMSS_ASI_DFX_MODULE_ra_busy_END (4)
#define SOC_DMSS_ASI_DFX_MODULE_ax_busy_START (5)
#define SOC_DMSS_ASI_DFX_MODULE_ax_busy_END (5)
#define SOC_DMSS_ASI_DFX_MODULE_cs_busy_START (6)
#define SOC_DMSS_ASI_DFX_MODULE_cs_busy_END (6)
#define SOC_DMSS_ASI_DFX_MODULE_wd_busy_START (7)
#define SOC_DMSS_ASI_DFX_MODULE_wd_busy_END (7)
#define SOC_DMSS_ASI_DFX_MODULE_b_busy_START (8)
#define SOC_DMSS_ASI_DFX_MODULE_b_busy_END (8)
#define SOC_DMSS_ASI_DFX_MODULE_rd_busy_START (9)
#define SOC_DMSS_ASI_DFX_MODULE_rd_busy_END (9)
#define SOC_DMSS_ASI_DFX_MODULE_fkrd_busy_START (10)
#define SOC_DMSS_ASI_DFX_MODULE_fkrd_busy_END (10)
#define SOC_DMSS_ASI_DFX_MODULE_sec_busy_START (11)
#define SOC_DMSS_ASI_DFX_MODULE_sec_busy_END (11)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_est_busy_START (12)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_est_busy_END (12)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_buf_busy_START (13)
#define SOC_DMSS_ASI_DFX_MODULE_rdr_buf_busy_END (13)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ra_wfifo_st : 2;
        unsigned int wd_cmdfifo_st : 2;
        unsigned int b_respfifo_st : 2;
        unsigned int rd_rfifo_st : 2;
        unsigned int rd_barfifo_st : 2;
        unsigned int fkrd_fkfifo_st : 2;
        unsigned int reserved_0 : 4;
        unsigned int dmi0_wfifo_st : 2;
        unsigned int dmi1_wfifo_st : 2;
        unsigned int dmi2_wfifo_st : 2;
        unsigned int dmi3_wfifo_st : 2;
        unsigned int reserved_1 : 8;
    } reg;
} SOC_DMSS_ASI_DFX_FIFO_UNION;
#endif
#define SOC_DMSS_ASI_DFX_FIFO_ra_wfifo_st_START (0)
#define SOC_DMSS_ASI_DFX_FIFO_ra_wfifo_st_END (1)
#define SOC_DMSS_ASI_DFX_FIFO_wd_cmdfifo_st_START (2)
#define SOC_DMSS_ASI_DFX_FIFO_wd_cmdfifo_st_END (3)
#define SOC_DMSS_ASI_DFX_FIFO_b_respfifo_st_START (4)
#define SOC_DMSS_ASI_DFX_FIFO_b_respfifo_st_END (5)
#define SOC_DMSS_ASI_DFX_FIFO_rd_rfifo_st_START (6)
#define SOC_DMSS_ASI_DFX_FIFO_rd_rfifo_st_END (7)
#define SOC_DMSS_ASI_DFX_FIFO_rd_barfifo_st_START (8)
#define SOC_DMSS_ASI_DFX_FIFO_rd_barfifo_st_END (9)
#define SOC_DMSS_ASI_DFX_FIFO_fkrd_fkfifo_st_START (10)
#define SOC_DMSS_ASI_DFX_FIFO_fkrd_fkfifo_st_END (11)
#define SOC_DMSS_ASI_DFX_FIFO_dmi0_wfifo_st_START (16)
#define SOC_DMSS_ASI_DFX_FIFO_dmi0_wfifo_st_END (17)
#define SOC_DMSS_ASI_DFX_FIFO_dmi1_wfifo_st_START (18)
#define SOC_DMSS_ASI_DFX_FIFO_dmi1_wfifo_st_END (19)
#define SOC_DMSS_ASI_DFX_FIFO_dmi2_wfifo_st_START (20)
#define SOC_DMSS_ASI_DFX_FIFO_dmi2_wfifo_st_END (21)
#define SOC_DMSS_ASI_DFX_FIFO_dmi3_wfifo_st_START (22)
#define SOC_DMSS_ASI_DFX_FIFO_dmi3_wfifo_st_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cs_fsm_st : 8;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_ASI_DFX_FSM_UNION;
#endif
#define SOC_DMSS_ASI_DFX_FSM_cs_fsm_st_START (0)
#define SOC_DMSS_ASI_DFX_FSM_cs_fsm_st_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int wr_ostd_st : 5;
        unsigned int reserved : 27;
    } reg;
} SOC_DMSS_ASI_DFX_WROSTD_UNION;
#endif
#define SOC_DMSS_ASI_DFX_WROSTD_wr_ostd_st_START (0)
#define SOC_DMSS_ASI_DFX_WROSTD_wr_ostd_st_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_cmd_que_cnt_st : 8;
        unsigned int rdr_buf_cnt_st : 8;
        unsigned int rdr_bid_use_cnt_st : 8;
        unsigned int reserved : 8;
    } reg;
} SOC_DMSS_ASI_DFX_RDR0_UNION;
#endif
#define SOC_DMSS_ASI_DFX_RDR0_rdr_cmd_que_cnt_st_START (0)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_cmd_que_cnt_st_END (7)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_buf_cnt_st_START (8)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_buf_cnt_st_END (15)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_bid_use_cnt_st_START (16)
#define SOC_DMSS_ASI_DFX_RDR0_rdr_bid_use_cnt_st_END (23)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rdr_buf_rcv_done_st : 32;
    } reg;
} SOC_DMSS_ASI_DFX_RDR1_UNION;
#endif
#define SOC_DMSS_ASI_DFX_RDR1_rdr_buf_rcv_done_st_START (0)
#define SOC_DMSS_ASI_DFX_RDR1_rdr_buf_rcv_done_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_int_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_DFX_INT_EN_UNION;
#endif
#define SOC_DMSS_ASI_DFX_INT_EN_dfx_int_en_START (0)
#define SOC_DMSS_ASI_DFX_INT_EN_dfx_int_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dfx_int_status : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_ASI_DFX_INT_STATUS_UNION;
#endif
#define SOC_DMSS_ASI_DFX_INT_STATUS_dfx_int_status_START (0)
#define SOC_DMSS_ASI_DFX_INT_STATUS_dfx_int_status_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int abnm_link_err : 1;
        unsigned int abnm_rdr_split : 1;
        unsigned int abnm_ex_len : 1;
        unsigned int abnm_nar_wrap : 1;
        unsigned int abnm_fix : 1;
        unsigned int abnm_len16 : 1;
        unsigned int reserved : 10;
        unsigned int abnm_cmd_mid : 15;
        unsigned int abnm_cmd_type : 1;
    } reg;
} SOC_DMSS_ASI_DFX_ABNM_INF_UNION;
#endif
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_link_err_START (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_link_err_END (0)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_rdr_split_START (1)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_rdr_split_END (1)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_ex_len_START (2)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_ex_len_END (2)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_nar_wrap_START (3)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_nar_wrap_END (3)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_fix_START (4)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_fix_END (4)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_len16_START (5)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_len16_END (5)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_cmd_mid_START (16)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_cmd_mid_END (30)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_cmd_type_START (31)
#define SOC_DMSS_ASI_DFX_ABNM_INF_abnm_cmd_type_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ra_wfifo_load_st : 1;
        unsigned int wd_cmdfifo_load_st : 1;
        unsigned int b_respfifo_load_st : 1;
        unsigned int rd_rfifo_load_st : 1;
        unsigned int rd_barfifo_load_st : 1;
        unsigned int fkrd_fkfifo_load_st : 1;
        unsigned int rdr_cmd_que_load_st : 1;
        unsigned int rdr_bid_use_load_st : 1;
        unsigned int dmi0_wfifo_load_st : 1;
        unsigned int dmi1_wfifo_load_st : 1;
        unsigned int dmi2_wfifo_load_st : 1;
        unsigned int dmi3_wfifo_load_st : 1;
        unsigned int reserved : 20;
    } reg;
} SOC_DMSS_ASI_DFX_LOAD_UNION;
#endif
#define SOC_DMSS_ASI_DFX_LOAD_ra_wfifo_load_st_START (0)
#define SOC_DMSS_ASI_DFX_LOAD_ra_wfifo_load_st_END (0)
#define SOC_DMSS_ASI_DFX_LOAD_wd_cmdfifo_load_st_START (1)
#define SOC_DMSS_ASI_DFX_LOAD_wd_cmdfifo_load_st_END (1)
#define SOC_DMSS_ASI_DFX_LOAD_b_respfifo_load_st_START (2)
#define SOC_DMSS_ASI_DFX_LOAD_b_respfifo_load_st_END (2)
#define SOC_DMSS_ASI_DFX_LOAD_rd_rfifo_load_st_START (3)
#define SOC_DMSS_ASI_DFX_LOAD_rd_rfifo_load_st_END (3)
#define SOC_DMSS_ASI_DFX_LOAD_rd_barfifo_load_st_START (4)
#define SOC_DMSS_ASI_DFX_LOAD_rd_barfifo_load_st_END (4)
#define SOC_DMSS_ASI_DFX_LOAD_fkrd_fkfifo_load_st_START (5)
#define SOC_DMSS_ASI_DFX_LOAD_fkrd_fkfifo_load_st_END (5)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_cmd_que_load_st_START (6)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_cmd_que_load_st_END (6)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_bid_use_load_st_START (7)
#define SOC_DMSS_ASI_DFX_LOAD_rdr_bid_use_load_st_END (7)
#define SOC_DMSS_ASI_DFX_LOAD_dmi0_wfifo_load_st_START (8)
#define SOC_DMSS_ASI_DFX_LOAD_dmi0_wfifo_load_st_END (8)
#define SOC_DMSS_ASI_DFX_LOAD_dmi1_wfifo_load_st_START (9)
#define SOC_DMSS_ASI_DFX_LOAD_dmi1_wfifo_load_st_END (9)
#define SOC_DMSS_ASI_DFX_LOAD_dmi2_wfifo_load_st_START (10)
#define SOC_DMSS_ASI_DFX_LOAD_dmi2_wfifo_load_st_END (10)
#define SOC_DMSS_ASI_DFX_LOAD_dmi3_wfifo_load_st_START (11)
#define SOC_DMSS_ASI_DFX_LOAD_dmi3_wfifo_load_st_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_tag : 8;
        unsigned int reserved_0: 8;
        unsigned int version : 12;
        unsigned int reserved_1: 4;
    } reg;
} SOC_DMSS_GLB_RTL_VER_UNION;
#endif
#define SOC_DMSS_GLB_RTL_VER_rtl_tag_START (0)
#define SOC_DMSS_GLB_RTL_VER_rtl_tag_END (7)
#define SOC_DMSS_GLB_RTL_VER_version_START (16)
#define SOC_DMSS_GLB_RTL_VER_version_END (27)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_addr_width : 4;
        unsigned int rtl_id_width : 5;
        unsigned int reserved_0 : 3;
        unsigned int rtl_mid_width : 4;
        unsigned int rtl_dmi_num : 1;
        unsigned int reserved_1 : 1;
        unsigned int rtl_ch_num_pre_dmi : 1;
        unsigned int reserved_2 : 1;
        unsigned int rtl_asi_num : 4;
        unsigned int rtl_exclusive_num : 6;
        unsigned int reserved_3 : 2;
    } reg;
} SOC_DMSS_GLB_RTL_INF0_UNION;
#endif
#define SOC_DMSS_GLB_RTL_INF0_rtl_addr_width_START (0)
#define SOC_DMSS_GLB_RTL_INF0_rtl_addr_width_END (3)
#define SOC_DMSS_GLB_RTL_INF0_rtl_id_width_START (4)
#define SOC_DMSS_GLB_RTL_INF0_rtl_id_width_END (8)
#define SOC_DMSS_GLB_RTL_INF0_rtl_mid_width_START (12)
#define SOC_DMSS_GLB_RTL_INF0_rtl_mid_width_END (15)
#define SOC_DMSS_GLB_RTL_INF0_rtl_dmi_num_START (16)
#define SOC_DMSS_GLB_RTL_INF0_rtl_dmi_num_END (16)
#define SOC_DMSS_GLB_RTL_INF0_rtl_ch_num_pre_dmi_START (18)
#define SOC_DMSS_GLB_RTL_INF0_rtl_ch_num_pre_dmi_END (18)
#define SOC_DMSS_GLB_RTL_INF0_rtl_asi_num_START (20)
#define SOC_DMSS_GLB_RTL_INF0_rtl_asi_num_END (23)
#define SOC_DMSS_GLB_RTL_INF0_rtl_exclusive_num_START (24)
#define SOC_DMSS_GLB_RTL_INF0_rtl_exclusive_num_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rtl_rbuf_buf_num : 6;
        unsigned int reserved : 26;
    } reg;
} SOC_DMSS_GLB_RTL_INF1_UNION;
#endif
#define SOC_DMSS_GLB_RTL_INF1_rtl_rbuf_buf_num_START (0)
#define SOC_DMSS_GLB_RTL_INF1_rtl_rbuf_buf_num_END (5)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ram_tmod : 16;
        unsigned int reserved : 16;
    } reg;
} SOC_DMSS_GLB_RAM_TMOD_UNION;
#endif
#define SOC_DMSS_GLB_RAM_TMOD_ram_tmod_START (0)
#define SOC_DMSS_GLB_RAM_TMOD_ram_tmod_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int cfg_lock : 1;
        unsigned int reserved_0 : 15;
        unsigned int cfg_lock_int_en : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_DMSS_GLB_CFG_LOCK_UNION;
#endif
#define SOC_DMSS_GLB_CFG_LOCK_cfg_lock_START (0)
#define SOC_DMSS_GLB_CFG_LOCK_cfg_lock_END (0)
#define SOC_DMSS_GLB_CFG_LOCK_cfg_lock_int_en_START (16)
#define SOC_DMSS_GLB_CFG_LOCK_cfg_lock_int_en_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int ckg_byp_rbuf_src : 1;
        unsigned int ckg_byp_rbuf_dst : 1;
        unsigned int ckg_byp_dmi_wdfifo : 1;
        unsigned int ckg_byp_dmi : 1;
        unsigned int ckg_byp_dmi_dcq : 1;
        unsigned int reserved : 27;
    } reg;
} SOC_DMSS_GLB_DYN_CKG_UNION;
#endif
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_src_START (0)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_src_END (0)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_dst_START (1)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_rbuf_dst_END (1)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dmi_wdfifo_START (2)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dmi_wdfifo_END (2)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dmi_START (3)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dmi_END (3)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dmi_dcq_START (4)
#define SOC_DMSS_GLB_DYN_CKG_ckg_byp_dmi_dcq_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_sec_int : 12;
        unsigned int reserved_0 : 4;
        unsigned int asi_dfx_int : 12;
        unsigned int cfg_lock_int : 1;
        unsigned int pri_allow_int : 1;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_DMSS_GLB_INT_STATUS_UNION;
#endif
#define SOC_DMSS_GLB_INT_STATUS_asi_sec_int_START (0)
#define SOC_DMSS_GLB_INT_STATUS_asi_sec_int_END (11)
#define SOC_DMSS_GLB_INT_STATUS_asi_dfx_int_START (16)
#define SOC_DMSS_GLB_INT_STATUS_asi_dfx_int_END (27)
#define SOC_DMSS_GLB_INT_STATUS_cfg_lock_int_START (28)
#define SOC_DMSS_GLB_INT_STATUS_cfg_lock_int_END (28)
#define SOC_DMSS_GLB_INT_STATUS_pri_allow_int_START (29)
#define SOC_DMSS_GLB_INT_STATUS_pri_allow_int_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_sec_int_clear : 12;
        unsigned int reserved_0 : 4;
        unsigned int asi_dfx_int_clear : 12;
        unsigned int cfg_lock_int_clear : 1;
        unsigned int pri_allow_int_clear : 1;
        unsigned int reserved_1 : 2;
    } reg;
} SOC_DMSS_GLB_INT_CLEAR_UNION;
#endif
#define SOC_DMSS_GLB_INT_CLEAR_asi_sec_int_clear_START (0)
#define SOC_DMSS_GLB_INT_CLEAR_asi_sec_int_clear_END (11)
#define SOC_DMSS_GLB_INT_CLEAR_asi_dfx_int_clear_START (16)
#define SOC_DMSS_GLB_INT_CLEAR_asi_dfx_int_clear_END (27)
#define SOC_DMSS_GLB_INT_CLEAR_cfg_lock_int_clear_START (28)
#define SOC_DMSS_GLB_INT_CLEAR_cfg_lock_int_clear_END (28)
#define SOC_DMSS_GLB_INT_CLEAR_pri_allow_int_clear_START (29)
#define SOC_DMSS_GLB_INT_CLEAR_pri_allow_int_clear_END (29)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int exclu_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_GLB_EXCLU_UNION;
#endif
#define SOC_DMSS_GLB_EXCLU_exclu_en_START (0)
#define SOC_DMSS_GLB_EXCLU_exclu_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int intlv_mode : 1;
        unsigned int reserved_0 : 3;
        unsigned int intlv_granule : 3;
        unsigned int reserved_1 : 25;
    } reg;
} SOC_DMSS_GLB_ADDR_INTLV_UNION;
#endif
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_mode_START (0)
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_mode_END (0)
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_granule_START (4)
#define SOC_DMSS_GLB_ADDR_INTLV_intlv_granule_END (6)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int bnk_scrmbl : 4;
        unsigned int reserved : 28;
    } reg;
} SOC_DMSS_GLB_ADDR_SCRMBL_UNION;
#endif
#define SOC_DMSS_GLB_ADDR_SCRMBL_bnk_scrmbl_START (0)
#define SOC_DMSS_GLB_ADDR_SCRMBL_bnk_scrmbl_END (3)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 4;
        unsigned int trsfr_wl_bl_2 : 1;
        unsigned int reserved_1 : 3;
        unsigned int trsfr_wl_bl_3 : 2;
        unsigned int reserved_2 : 2;
        unsigned int trsfr_wl_bl_4 : 2;
        unsigned int reserved_3 : 2;
        unsigned int trsfr_wl_bl_5 : 3;
        unsigned int reserved_4 : 1;
        unsigned int trsfr_wl_bl_6 : 3;
        unsigned int reserved_5 : 1;
        unsigned int trsfr_wl_bl_7 : 3;
        unsigned int reserved_6 : 1;
        unsigned int trsfr_wl_bl_8 : 3;
        unsigned int reserved_7 : 1;
    } reg;
} SOC_DMSS_GLB_RBUF_TRSFR_WL0_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_2_START (4)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_2_END (4)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_3_START (8)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_3_END (9)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_4_START (12)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_4_END (13)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_5_START (16)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_5_END (18)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_6_START (20)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_6_END (22)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_7_START (24)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_7_END (26)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_8_START (28)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL0_trsfr_wl_bl_8_END (30)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int trsfr_wl_bl_9 : 4;
        unsigned int trsfr_wl_bl_10 : 4;
        unsigned int trsfr_wl_bl_11 : 4;
        unsigned int trsfr_wl_bl_12 : 4;
        unsigned int trsfr_wl_bl_13 : 4;
        unsigned int trsfr_wl_bl_14 : 4;
        unsigned int trsfr_wl_bl_15 : 4;
        unsigned int trsfr_wl_bl_16 : 4;
    } reg;
} SOC_DMSS_GLB_RBUF_TRSFR_WL1_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_9_START (0)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_9_END (3)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_10_START (4)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_10_END (7)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_11_START (8)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_11_END (11)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_12_START (12)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_12_END (15)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_13_START (16)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_13_END (19)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_14_START (20)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_14_END (23)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_15_START (24)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_15_END (27)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_16_START (28)
#define SOC_DMSS_GLB_RBUF_TRSFR_WL1_trsfr_wl_bl_16_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf_byp_path_en : 1;
        unsigned int reserved : 31;
    } reg;
} SOC_DMSS_GLB_RBUF_BYP_PATH_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_BYP_PATH_rbuf_byp_path_en_START (0)
#define SOC_DMSS_GLB_RBUF_BYP_PATH_rbuf_byp_path_en_END (0)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf_bp_grp : 16;
        unsigned int rbuf_bp_wl : 16;
    } reg;
} SOC_DMSS_GLB_RBUF_BP_UNION;
#endif
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_grp_START (0)
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_grp_END (15)
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_wl_START (16)
#define SOC_DMSS_GLB_RBUF_BP_rbuf_bp_wl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_en : 1;
        unsigned int reserved_0: 3;
        unsigned int dcq_mode : 1;
        unsigned int reserved_1: 27;
    } reg;
} SOC_DMSS_GLB_DCQ_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_en_START (0)
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_en_END (0)
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_mode_START (4)
#define SOC_DMSS_GLB_DCQ_CTRL_dcq_mode_END (4)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq0_wr_que_st : 8;
        unsigned int dcq0_rd_que_st : 8;
        unsigned int dcq1_wr_que_st : 8;
        unsigned int dcq1_rd_que_st : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_ST_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_ST_dcq0_wr_que_st_START (0)
#define SOC_DMSS_GLB_DCQ_ST_dcq0_wr_que_st_END (7)
#define SOC_DMSS_GLB_DCQ_ST_dcq0_rd_que_st_START (8)
#define SOC_DMSS_GLB_DCQ_ST_dcq0_rd_que_st_END (15)
#define SOC_DMSS_GLB_DCQ_ST_dcq1_wr_que_st_START (16)
#define SOC_DMSS_GLB_DCQ_ST_dcq1_wr_que_st_END (23)
#define SOC_DMSS_GLB_DCQ_ST_dcq1_rd_que_st_START (24)
#define SOC_DMSS_GLB_DCQ_ST_dcq1_rd_que_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved : 8;
        unsigned int dcq_pri1_lvl : 8;
        unsigned int dcq_pri2_lvl : 8;
        unsigned int dcq_pri3_lvl : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_PRILVL0_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri1_lvl_START (8)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri1_lvl_END (15)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri2_lvl_START (16)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri2_lvl_END (23)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri3_lvl_START (24)
#define SOC_DMSS_GLB_DCQ_PRILVL0_dcq_pri3_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_pri4_lvl : 8;
        unsigned int dcq_pri5_lvl : 8;
        unsigned int dcq_pri6_lvl : 8;
        unsigned int dcq_pri7_lvl : 8;
    } reg;
} SOC_DMSS_GLB_DCQ_PRILVL1_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri4_lvl_START (0)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri4_lvl_END (7)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri5_lvl_START (8)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri5_lvl_END (15)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri6_lvl_START (16)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri6_lvl_END (23)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri7_lvl_START (24)
#define SOC_DMSS_GLB_DCQ_PRILVL1_dcq_pri7_lvl_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_mst_req_pri : 3;
        unsigned int dcq_mst_req_en : 1;
        unsigned int dcq_mst_ctrl_pri : 3;
        unsigned int dcq_mst_ctrl_en : 1;
        unsigned int reserved : 24;
    } reg;
} SOC_DMSS_GLB_DCQ_MST_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_req_pri_START (0)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_req_pri_END (2)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_req_en_START (3)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_req_en_END (3)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_ctrl_pri_START (4)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_ctrl_pri_END (6)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_ctrl_en_START (7)
#define SOC_DMSS_GLB_DCQ_MST_dcq_mst_ctrl_en_END (7)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int reserved_0 : 1;
        unsigned int dcq_pri1_int_en : 1;
        unsigned int dcq_pri2_int_en : 1;
        unsigned int dcq_pri3_int_en : 1;
        unsigned int dcq_pri4_int_en : 1;
        unsigned int dcq_pri5_int_en : 1;
        unsigned int dcq_pri6_int_en : 1;
        unsigned int dcq_pri7_int_en : 1;
        unsigned int reserved_1 : 1;
        unsigned int mst_pri1_int_en : 1;
        unsigned int mst_pri2_int_en : 1;
        unsigned int mst_pri3_int_en : 1;
        unsigned int mst_pri4_int_en : 1;
        unsigned int mst_pri5_int_en : 1;
        unsigned int mst_pri6_int_en : 1;
        unsigned int mst_pri7_int_en : 1;
        unsigned int reserved_2 : 16;
    } reg;
} SOC_DMSS_GLB_DCQ_INT_EN_UNION;
#endif
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri1_int_en_START (1)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri1_int_en_END (1)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri2_int_en_START (2)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri2_int_en_END (2)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri3_int_en_START (3)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri3_int_en_END (3)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri4_int_en_START (4)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri4_int_en_END (4)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri5_int_en_START (5)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri5_int_en_END (5)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri6_int_en_START (6)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri6_int_en_END (6)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri7_int_en_START (7)
#define SOC_DMSS_GLB_DCQ_INT_EN_dcq_pri7_int_en_END (7)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri1_int_en_START (9)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri1_int_en_END (9)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri2_int_en_START (10)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri2_int_en_END (10)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri3_int_en_START (11)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri3_int_en_END (11)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri4_int_en_START (12)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri4_int_en_END (12)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri5_int_en_START (13)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri5_int_en_END (13)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri6_int_en_START (14)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri6_int_en_END (14)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri7_int_en_START (15)
#define SOC_DMSS_GLB_DCQ_INT_EN_mst_pri7_int_en_END (15)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_stat_en : 12;
        unsigned int reserved_0 : 4;
        unsigned int pri_allow_stat_en : 1;
        unsigned int reserved_1 : 15;
    } reg;
} SOC_DMSS_GLB_STAT_CTRL_UNION;
#endif
#define SOC_DMSS_GLB_STAT_CTRL_asi_stat_en_START (0)
#define SOC_DMSS_GLB_STAT_CTRL_asi_stat_en_END (11)
#define SOC_DMSS_GLB_STAT_CTRL_pri_allow_stat_en_START (16)
#define SOC_DMSS_GLB_STAT_CTRL_pri_allow_stat_en_END (16)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_all_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_ALL_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_ALL_ALLOW_stat_dcq_all_allow_START (0)
#define SOC_DMSS_GLB_STAT_ALL_ALLOW_stat_dcq_all_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri1_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI1_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI1_ALLOW_stat_dcq_pri1_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI1_ALLOW_stat_dcq_pri1_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri2_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI2_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI2_ALLOW_stat_dcq_pri2_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI2_ALLOW_stat_dcq_pri2_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri3_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI3_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI3_ALLOW_stat_dcq_pri3_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI3_ALLOW_stat_dcq_pri3_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri4_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI4_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI4_ALLOW_stat_dcq_pri4_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI4_ALLOW_stat_dcq_pri4_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri5_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI5_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI5_ALLOW_stat_dcq_pri5_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI5_ALLOW_stat_dcq_pri5_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri6_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI6_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI6_ALLOW_stat_dcq_pri6_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI6_ALLOW_stat_dcq_pri6_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int stat_dcq_pri7_allow : 32;
    } reg;
} SOC_DMSS_GLB_STAT_PRI7_ALLOW_UNION;
#endif
#define SOC_DMSS_GLB_STAT_PRI7_ALLOW_stat_dcq_pri7_allow_START (0)
#define SOC_DMSS_GLB_STAT_PRI7_ALLOW_stat_dcq_pri7_allow_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vque0_cnt_st : 8;
        unsigned int vque1_cnt_st : 8;
        unsigned int vque2_cnt_st : 8;
        unsigned int vque3_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF0_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF0_vque0_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_RBUF0_vque0_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_RBUF0_vque1_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_RBUF0_vque1_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_RBUF0_vque2_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_RBUF0_vque2_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_RBUF0_vque3_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_RBUF0_vque3_cnt_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vque4_cnt_st : 8;
        unsigned int vque5_cnt_st : 8;
        unsigned int vque6_cnt_st : 8;
        unsigned int vque7_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF1_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF1_vque4_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_RBUF1_vque4_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_RBUF1_vque5_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_RBUF1_vque5_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_RBUF1_vque6_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_RBUF1_vque6_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_RBUF1_vque7_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_RBUF1_vque7_cnt_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vque8_cnt_st : 8;
        unsigned int vque9_cnt_st : 8;
        unsigned int vque10_cnt_st : 8;
        unsigned int vque11_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF2_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF2_vque8_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_RBUF2_vque8_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_RBUF2_vque9_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_RBUF2_vque9_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_RBUF2_vque10_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_RBUF2_vque10_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_RBUF2_vque11_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_RBUF2_vque11_cnt_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int vque_grp0_cnt_st : 8;
        unsigned int vque_grp1_cnt_st : 8;
        unsigned int vque_grp2_cnt_st : 8;
        unsigned int vque_grp3_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_RBUF3_UNION;
#endif
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp0_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp0_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp1_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp1_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp2_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp2_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp3_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_RBUF3_vque_grp3_cnt_st_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dmi0_grant : 16;
        unsigned int dmi1_grant : 16;
    } reg;
} SOC_DMSS_GLB_DFX_DMI_GRNT_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DMI_GRNT_dmi0_grant_START (0)
#define SOC_DMSS_GLB_DFX_DMI_GRNT_dmi0_grant_END (15)
#define SOC_DMSS_GLB_DFX_DMI_GRNT_dmi1_grant_START (16)
#define SOC_DMSS_GLB_DFX_DMI_GRNT_dmi1_grant_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int dcq_pri_allow : 3;
        unsigned int reserved_0 : 1;
        unsigned int mst_pri_allow : 3;
        unsigned int reserved_1 : 1;
        unsigned int dcq_pri_int_st : 3;
        unsigned int reserved_2 : 1;
        unsigned int mst_pri_int_st : 3;
        unsigned int reserved_3 : 17;
    } reg;
} SOC_DMSS_GLB_DFX_DCQ_UNION;
#endif
#define SOC_DMSS_GLB_DFX_DCQ_dcq_pri_allow_START (0)
#define SOC_DMSS_GLB_DFX_DCQ_dcq_pri_allow_END (2)
#define SOC_DMSS_GLB_DFX_DCQ_mst_pri_allow_START (4)
#define SOC_DMSS_GLB_DFX_DCQ_mst_pri_allow_END (6)
#define SOC_DMSS_GLB_DFX_DCQ_dcq_pri_int_st_START (8)
#define SOC_DMSS_GLB_DFX_DCQ_dcq_pri_int_st_END (10)
#define SOC_DMSS_GLB_DFX_DCQ_mst_pri_int_st_START (12)
#define SOC_DMSS_GLB_DFX_DCQ_mst_pri_int_st_END (14)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int asi_busy : 12;
        unsigned int reserved : 20;
    } reg;
} SOC_DMSS_GLB_DFX_ASI_BUSY_UNION;
#endif
#define SOC_DMSS_GLB_DFX_ASI_BUSY_asi_busy_START (0)
#define SOC_DMSS_GLB_DFX_ASI_BUSY_asi_busy_END (11)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int rbuf0_load_cnt_st : 8;
        unsigned int rbuf1_load_cnt_st : 8;
        unsigned int rbuf2_load_cnt_st : 8;
        unsigned int rbuf3_load_cnt_st : 8;
    } reg;
} SOC_DMSS_GLB_DFX_LOAD_UNION;
#endif
#define SOC_DMSS_GLB_DFX_LOAD_rbuf0_load_cnt_st_START (0)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf0_load_cnt_st_END (7)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf1_load_cnt_st_START (8)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf1_load_cnt_st_END (15)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf2_load_cnt_st_START (16)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf2_load_cnt_st_END (23)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf3_load_cnt_st_START (24)
#define SOC_DMSS_GLB_DFX_LOAD_rbuf3_load_cnt_st_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
