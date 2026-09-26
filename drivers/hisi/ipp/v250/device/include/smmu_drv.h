

#ifndef __SMMU_COMN_DRV_PRIV_CS_H__
#define __SMMU_COMN_DRV_PRIV_CS_H__

#define SMRX_NUM (88)

/* Define the union U_SMMU_SCR */
union u_smmu_scr {
	/* Define the struct bits */
	struct {
		unsigned int    glb_bypass    : 1  ; /* [0] */
		unsigned int    rqos_en       : 1  ; /* [1] */
		unsigned int    wqos_en       : 1  ; /* [2] */
		unsigned int    rsv_0         : 1  ; /* [3] */
		unsigned int    cache_l1_en   : 1  ; /* [4] */
		unsigned int    cache_l2_en   : 1  ; /* [5] */
		unsigned int    rqos          : 4  ; /* [9:6] */
		unsigned int    wqos          : 4  ; /* [13:10] */
		unsigned int    rsv_1         : 1  ; /* [14] */
		unsigned int    smr_rd_shadow : 1  ; /* [15] */
		unsigned int    ptw_pf        : 4  ; /* [19:16] */
		unsigned int    ptw_mid       : 6  ; /* [25:20] */
		unsigned int    rsv_2         : 6  ; /* [31:26] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_MEMCTRL */
union u_smmu_memctrl {
	/* Define the struct bits */
	struct {
		unsigned int    mem_ctrl_wr : 16  ; /* [15:0] */
		unsigned int    mem_ctrl_rd : 16  ; /* [31:16] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_LP_CTRL */
union u_smmu_lp_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int    auto_clk_gt_en : 2  ; /* [1:0] */
		unsigned int    rsv_3          : 30  ; /* [31:2] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_PRESS_REMAP */
union u_smmu_press_remap {
	/* Define the struct bits */
	struct {
		unsigned int    remap_sel0  : 2  ; /* [1:0] */
		unsigned int    remap_sel1  : 2  ; /* [3:2] */
		unsigned int    remap_sel2  : 2  ; /* [5:4] */
		unsigned int    remap_sel3  : 2  ; /* [7:6] */
		unsigned int    remap_sel4  : 2  ; /* [9:8] */
		unsigned int    remap_sel5  : 2  ; /* [11:10] */
		unsigned int    remap_sel6  : 2  ; /* [13:12] */
		unsigned int    remap_sel7  : 2  ; /* [15:14] */
		unsigned int    remap_sel8  : 2  ; /* [17:16] */
		unsigned int    remap_sel9  : 2  ; /* [19:18] */
		unsigned int    remap_sel10 : 2  ; /* [21:20] */
		unsigned int    remap_sel11 : 2  ; /* [23:22] */
		unsigned int    remap_sel12 : 2  ; /* [25:24] */
		unsigned int    remap_sel13 : 2  ; /* [27:26] */
		unsigned int    remap_sel14 : 2  ; /* [29:28] */
		unsigned int    remap_sel15 : 2  ; /* [31:30] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTMASK_NS */
union u_smmu_intmask_ns {
	/* Define the struct bits */
	struct {
		unsigned int    intns_permis_msk      : 1  ; /* [0] */
		unsigned int    intns_ext_msk         : 1  ; /* [1] */
		unsigned int    intns_tlbmiss_msk     : 1  ; /* [2] */
		unsigned int    intns_ptw_trans_msk   : 1  ; /* [3] */
		unsigned int    intns_ptw_invalid_msk : 1  ; /* [4] */
		unsigned int    intns_ptw_ns_msk      : 1  ; /* [5] */
		unsigned int    rsv_4                 : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTRAW_NS */
union u_smmu_intraw_ns {
	/* Define the struct bits */
	struct {
		unsigned int    intns_permis_raw      : 1  ; /* [0] */
		unsigned int    intns_ext_raw         : 1  ; /* [1] */
		unsigned int    intns_tlbmiss_raw     : 1  ; /* [2] */
		unsigned int    intns_ptw_trans_raw   : 1  ; /* [3] */
		unsigned int    intns_ptw_invalid_raw : 1  ; /* [4] */
		unsigned int    intns_ptw_ns_raw      : 1  ; /* [5] */
		unsigned int    rsv_5                 : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTSTAT_NS */
union u_smmu_intstat_ns {
	/* Define the struct bits */
	struct {
		unsigned int    intns_permis_stat      : 1  ; /* [0] */
		unsigned int    intns_ext_stat         : 1  ; /* [1] */
		unsigned int    intns_tlbmiss_stat     : 1  ; /* [2] */
		unsigned int    intns_ptw_trans_stat   : 1  ; /* [3] */
		unsigned int    intns_ptw_invalid_stat : 1  ; /* [4] */
		unsigned int    intns_ptw_ns_stat      : 1  ; /* [5] */
		unsigned int    rsv_6                  : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTCLR_NS */
union u_smmu_intclr_ns {
	/* Define the struct bits */
	struct {
		unsigned int    intns_permis_clr      : 1  ; /* [0] */
		unsigned int    intns_ext_clr         : 1  ; /* [1] */
		unsigned int    intns_tlbmiss_clr     : 1  ; /* [2] */
		unsigned int    intns_ptw_trans_clr   : 1  ; /* [3] */
		unsigned int    intns_ptw_invalid_clr : 1  ; /* [4] */
		unsigned int    intns_ptw_ns_clr      : 1  ; /* [5] */
		unsigned int    rsv_7                 : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SMRX_NS */
union u_smmu_smrx_ns {
	/* Define the struct bits */
	struct {
		unsigned int    smr_bypass      : 1  ; /* [0] */
		unsigned int    rsv_8           : 1  ; /* [1] */
		unsigned int    smr_ptw_qos     : 2  ; /* [3:2] */
		unsigned int    smr_invld_en    : 1  ; /* [4] */
		unsigned int    rsv_9           : 7  ; /* [11:5] */
		unsigned int    smr_offset_addr : 20  ; /* [31:12] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN0_NS */
union u_smmu_rld_en0_ns {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en0 : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN1_NS */
union u_smmu_rld_en1_ns {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en1 : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN2_NS */
union u_smmu_rld_en2_ns {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en2 : 24  ; /* [23:0] */
		unsigned int    rsv_10      : 8  ; /* [31:24] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_CB_SCTRL */
union u_smmu_cb_sctrl {
	/* Define the struct bits */
	struct {
		unsigned int    cb_bypass : 1  ; /* [0] */
		unsigned int    rsv_11    : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_CB_TTBR0 */
union u_smmu_cb_ttbr0 {
	/* Define the struct bits */
	struct {
		unsigned int    cb_ttbr0 : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_CB_TTBR1 */
union u_smmu_cb_ttbr1 {
	/* Define the struct bits */
	struct {
		unsigned int    cb_ttbr1 : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_CB_TTBCR */
union u_smmu_cb_ttbcr {
	/* Define the struct bits */
	struct {
		unsigned int    cb_ttbcr_des  : 1  ; /* [0] */
		unsigned int    cb_ttbcr_t0sz : 3  ; /* [3:1] */
		unsigned int    cb_ttbcr_n    : 3  ; /* [6:4] */
		unsigned int    rsv_12        : 9  ; /* [15:7] */
		unsigned int    cb_ttbcr_t1sz : 3  ; /* [18:16] */
		unsigned int    rsv_13        : 13  ; /* [31:19] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OFFSET_ADDR_NS */
union u_smmu_offset_addr_ns {
	/* Define the struct bits */
	struct {
		unsigned int    offset_addr_ns : 14  ; /* [13:0] */
		unsigned int    rsv_14         : 18  ; /* [31:14] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCACHEI_ALL */
union u_smmu_scachei_all {
	/* Define the struct bits */
	struct {
		unsigned int    cache_all_invalid : 1  ; /* [0] */
		unsigned int    cache_all_level   : 2  ; /* [2:1] */
		unsigned int    rsv_15            : 29  ; /* [31:3] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCACHEI_L1 */
union u_smmu_scachei_l1 {
	/* Define the struct bits */
	struct {
		unsigned int    cache_l1_invalid  : 1  ; /* [0] */
		unsigned int    cache_l1_security : 2  ; /* [2:1] */
		unsigned int    rsv_16            : 29  ; /* [31:3] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCACHEI_L2L3 */
union u_smmu_scachei_l2l3 {
	/* Define the struct bits */
	struct {
		unsigned int    cache_l2l3_invalid : 1  ; /* [0] */
		unsigned int    cache_l2l3_strmid  : 15  ; /* [15:1] */
		unsigned int    rsv_17             : 16  ; /* [31:16] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAMA_CTRL0_NS */
union u_smmu_fama_ctrl0_ns {
	/* Define the struct bits */
	struct {
		unsigned int    fama_sdes_msb_ns : 7  ; /* [6:0] */
		unsigned int    fama_chn_sel_ns  : 1  ; /* [7] */
		unsigned int    fama_bps_msb_ns  : 6  ; /* [13:8] */
		unsigned int    rsv_18           : 18  ; /* [31:14] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAMA_CTRL1_NS */
union u_smmu_fama_ctrl1_ns {
	/* Define the struct bits */
	struct {
		unsigned int    fama_ptw_msb_ns : 7  ; /* [6:0] */
		unsigned int    rsv_19          : 25  ; /* [31:7] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_ADDR_MSB */
union u_smmu_addr_msb {
	/* Define the struct bits */
	struct {
		unsigned int    msb_errrd : 7  ; /* [6:0] */
		unsigned int    msb_errwr : 7  ; /* [13:7] */
		unsigned int    msb_ova   : 7  ; /* [20:14] */
		unsigned int    rsv_20    : 11  ; /* [31:21] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_ERR_RDADDR */
union u_smmu_err_rdaddr {
	/* Define the struct bits */
	struct {
		unsigned int    err_rd_addr : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_ERR_WRADDR */
union u_smmu_err_wraddr {
	/* Define the struct bits */
	struct {
		unsigned int    err_wr_addr : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAULT_ADDR_TCU */
union u_smmu_fault_addr_tcu {
	/* Define the struct bits */
	struct {
		unsigned int    fault_addr_tcu : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAULT_ID_TCU */
union u_smmu_fault_id_tcu {
	/* Define the struct bits */
	struct {
		unsigned int    fault_index_id_tcu : 16  ; /* [15:0] */
		unsigned int    fault_strm_id_tcu  : 16  ; /* [31:16] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAULT_ADDR_TBUX */
union u_smmu_fault_addr_tbux {
	/* Define the struct bits */
	struct {
		unsigned int    fault_addr_tbu : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAULT_ID_TBUX */
union u_smmu_fault_id_tbux {
	/* Define the struct bits */
	struct {
		unsigned int    fault_index_id_tbu : 16  ; /* [15:0] */
		unsigned int    fault_strm_id_tbu  : 16  ; /* [31:16] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAULT_INFOX */
union u_smmu_fault_infox {
	/* Define the struct bits */
	struct {
		unsigned int    fault_tlbmiss_err_tbu : 2  ; /* [1:0] */
		unsigned int    fault_permis_err_tbu  : 3  ; /* [4:2] */
		unsigned int    fautl_ext_err_tbu     : 2  ; /* [6:5] */
		unsigned int    fault_ext_err_id_tbu  : 9  ; /* [15:7] */
		unsigned int    rsv_21                : 16  ; /* [31:16] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRPTR_TLB */
union u_smmu_dbgrptr_tlb {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_tlbword_pointer  : 3  ; /* [2:0] */
		unsigned int    dbg_tlbentry_pointer : 13  ; /* [15:3] */
		unsigned int    rsv_22               : 14  ; /* [29:16] */
		unsigned int    dbg_tlb_type         : 1  ; /* [30] */
		unsigned int    rsv_23               : 1  ; /* [31] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRDATA_TLB */
union u_smmu_dbgrdata_tlb {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_tlb_rdata : 31  ; /* [30:0] */
		unsigned int    rsv_24        : 1  ; /* [31] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRPTR_CACHE */
union u_smmu_dbgrptr_cache {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_cache_level      : 2  ; /* [1:0] */
		unsigned int    dbg_cache_l1_pointer : 2  ; /* [3:2] */
		unsigned int    dbg_cache_l1_ns      : 1  ; /* [4] */
		unsigned int    dbg_cache_l2_strmid  : 11  ; /* [15:5] */
		unsigned int    dbg_cache_l1_prot    : 1  ; /* [16] */
		unsigned int    rsv_25               : 15  ; /* [31:17] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRDATA0_CACHE */
union u_smmu_dbgrdata0_cache {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_cache_rdata0 : 29  ; /* [28:0] */
		unsigned int    rsv_26           : 3  ; /* [31:29] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRDATA1_CACHE */
union u_smmu_dbgrdata1_cache {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_cache_rdata1 : 12  ; /* [11:0] */
		unsigned int    rsv_27           : 20  ; /* [31:12] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGAXI_CTRL */
union u_smmu_dbgaxi_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_axilock_en : 1  ; /* [0] */
		unsigned int    rsv_28         : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OVA_ADDR */
union u_smmu_ova_addr {
	/* Define the struct bits */
	struct {
		unsigned int    override_va : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OPA_ADDR */
union u_smmu_opa_addr {
	/* Define the struct bits */
	struct {
		unsigned int    override_acquire_pa : 27  ; /* [26:0] */
		unsigned int    rsv_29              : 4  ; /* [30:27] */
		unsigned int    override_pa_done    : 1  ; /* [31] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OVA_CTRL */
union u_smmu_ova_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int    override_va_cfg     : 1  ; /* [0] */
		unsigned int    rsv_30              : 1  ; /* [1] */
		unsigned int    override_va_type    : 1  ; /* [2] */
		unsigned int    override_va_indexid : 13  ; /* [15:3] */
		unsigned int    override_va_strmid  : 12  ; /* [27:16] */
		unsigned int    override_tbu_num    : 4  ; /* [31:28] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OPREF_ADDR */
union u_smmu_opref_addr {
	/* Define the struct bits */
	struct {
		unsigned int    override_pref_addr : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OPREF_CTRL */
union u_smmu_opref_ctrl {
	/* Define the struct bits */
	struct {
		unsigned int    override_pref_cfg     : 1  ; /* [0] */
		unsigned int    rsv_31                : 1  ; /* [1] */
		unsigned int    override_pref_type    : 1  ; /* [2] */
		unsigned int    override_pref_initial : 1  ; /* [3] */
		unsigned int    override_pref_indexid : 12  ; /* [15:4] */
		unsigned int    override_pref_strmid  : 16  ; /* [31:16] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OPREF_CNT */
union u_smmu_opref_cnt {
	/* Define the struct bits */
	struct {
		unsigned int    override_pref_cnt : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SMRX_S */
union u_smmu_smrx_s {
	/* Define the struct bits */
	struct {
		unsigned int    smr_nscfg         : 1  ; /* [0] */
		unsigned int    smr_nscfg_en      : 1  ; /* [1] */
		unsigned int    smr_bypass_s      : 1  ; /* [2] */
		unsigned int    ptw_mid_sel_s     : 1  ; /* [3] */
		unsigned int    rsv_32            : 1  ; /* [4] */
		unsigned int    smr_mid_en_s      : 1  ; /* [5] */
		unsigned int    smr_mid_s         : 6  ; /* [11:6] */
		unsigned int    smr_offset_addr_s : 20  ; /* [31:12] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN0_S */
union u_smmu_rld_en0_s {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en0_s : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN1_S */
union u_smmu_rld_en1_s {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en1_s : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN2_S */
union u_smmu_rld_en2_s {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en2_s : 24  ; /* [23:0] */
		unsigned int    rsv_33        : 8  ; /* [31:24] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTMAS_S */
union u_smmu_intmas_s {
	/* Define the struct bits */
	struct {
		unsigned int    ints_permis_msk      : 1  ; /* [0] */
		unsigned int    ints_ext_msk         : 1  ; /* [1] */
		unsigned int    ints_tlbmiss_msk     : 1  ; /* [2] */
		unsigned int    ints_ptw_trans_msk   : 1  ; /* [3] */
		unsigned int    ints_ptw_invalid_msk : 1  ; /* [4] */
		unsigned int    ints_ptw_ns_msk      : 1  ; /* [5] */
		unsigned int    rsv_34               : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTRAW_S */
union u_smmu_intraw_s {
	/* Define the struct bits */
	struct {
		unsigned int    ints_permis_raw      : 1  ; /* [0] */
		unsigned int    ints_ext_raw         : 1  ; /* [1] */
		unsigned int    ints_tlbmiss_raw     : 1  ; /* [2] */
		unsigned int    ints_ptw_trans_raw   : 1  ; /* [3] */
		unsigned int    ints_ptw_invalid_raw : 1  ; /* [4] */
		unsigned int    ints_ptw_ns_raw      : 1  ; /* [5] */
		unsigned int    rsv_35               : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTSTAT_S */
union u_smmu_intstat_s {
	/* Define the struct bits */
	struct {
		unsigned int    ints_permis_stat      : 1  ; /* [0] */
		unsigned int    ints_ext_stat         : 1  ; /* [1] */
		unsigned int    ints_tlbmiss_stat     : 1  ; /* [2] */
		unsigned int    ints_ptw_trans_stat   : 1  ; /* [3] */
		unsigned int    ints_ptw_invalid_stat : 1  ; /* [4] */
		unsigned int    ints_ptw_ns_stat      : 1  ; /* [5] */
		unsigned int    rsv_36                : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTCLR_S */
union u_smmu_intclr_s {
	/* Define the struct bits */
	struct {
		unsigned int    ints_permis_clr      : 1  ; /* [0] */
		unsigned int    ints_ext_clr         : 1  ; /* [1] */
		unsigned int    ints_tlbmiss_clr     : 1  ; /* [2] */
		unsigned int    ints_ptw_trans_clr   : 1  ; /* [3] */
		unsigned int    ints_ptw_invalid_clr : 1  ; /* [4] */
		unsigned int    ints_ptw_ns_clr      : 1  ; /* [5] */
		unsigned int    rsv_37               : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCR_S */
union u_smmu_scr_s {
	/* Define the struct bits */
	struct {
		unsigned int    glb_nscfg    : 2  ; /* [1:0] */
		unsigned int    mid_s        : 6  ; /* [7:2] */
		unsigned int    glb_bypass_s : 1  ; /* [8] */
		unsigned int    mid_en_s     : 1  ; /* [9] */
		unsigned int    rsv_38       : 22  ; /* [31:10] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCB_SCTRL */
union u_smmu_scb_sctrl {
	/* Define the struct bits */
	struct {
		unsigned int    scb_bypass : 1  ; /* [0] */
		unsigned int    rsv_39     : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCB_TTBR */
union u_smmu_scb_ttbr {
	/* Define the struct bits */
	struct {
		unsigned int    scb_ttbr : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCB_TTBCR */
union u_smmu_scb_ttbcr {
	/* Define the struct bits */
	struct {
		unsigned int    scb_ttbcr_des : 1  ; /* [0] */
		unsigned int    rsv_40        : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OFFSET_ADDR_S */
union u_smmu_offset_addr_s {
	/* Define the struct bits */
	struct {
		unsigned int    offset_addr_s : 14  ; /* [13:0] */
		unsigned int    rsv_41        : 18  ; /* [31:14] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAMA_CTRL0_S */
union u_smmu_fama_ctrl0_s {
	/* Define the struct bits */
	struct {
		unsigned int    fama_sdes_msb_s : 7  ; /* [6:0] */
		unsigned int    fama_chn_sel_s  : 1  ; /* [7] */
		unsigned int    fama_bps_msb_s  : 6  ; /* [13:8] */
		unsigned int    rsv_42          : 18  ; /* [31:14] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAMA_CTRL1_S */
union u_smmu_fama_ctrl1_s {
	/* Define the struct bits */
	struct {
		unsigned int    fama_ptw_msb_s : 7  ; /* [6:0] */
		unsigned int    rsv_43         : 25  ; /* [31:7] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRPTR_TLB_S */
union u_smmu_dbgrptr_tlb_s {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_tlb_en : 1  ; /* [0] */
		unsigned int    rsv_44     : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_DBGRPTR_CACHE_S */
union u_smmu_dbgrptr_cache_s {
	/* Define the struct bits */
	struct {
		unsigned int    dbg_cache_en : 1  ; /* [0] */
		unsigned int    rsv_45       : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OVERRIDE_CTRL_S */
union u_smmu_override_ctrl_s {
	/* Define the struct bits */
	struct {
		unsigned int    rsv_46               : 1  ; /* [0] */
		unsigned int    override_va_security : 1  ; /* [1] */
		unsigned int    rsv_47               : 30  ; /* [31:2] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SMRX_P */
union u_smmu_smrx_p {
	/* Define the struct bits */
	struct {
		unsigned int    smr_protect_en    : 1  ; /* [0] */
		unsigned int    rsv_48            : 1  ; /* [1] */
		unsigned int    smr_bypass_p      : 1  ; /* [2] */
		unsigned int    rsv_49            : 9  ; /* [11:3] */
		unsigned int    smr_offset_addr_p : 20  ; /* [31:12] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN0_P */
union u_smmu_rld_en0_p {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en0_p : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN1_P */
union u_smmu_rld_en1_p {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en1_p : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_RLD_EN2_P */
union u_smmu_rld_en2_p {
	/* Define the struct bits */
	struct {
		unsigned int    smr_rld_en2_p : 24  ; /* [23:0] */
		unsigned int    rsv_50        : 8  ; /* [31:24] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTMAS_P */
union u_smmu_intmas_p {
	/* Define the struct bits */
	struct {
		unsigned int    intp_permis_msk      : 1  ; /* [0] */
		unsigned int    intp_ext_msk         : 1  ; /* [1] */
		unsigned int    intp_tlbmiss_msk     : 1  ; /* [2] */
		unsigned int    intp_ptw_trans_msk   : 1  ; /* [3] */
		unsigned int    intp_ptw_invalid_msk : 1  ; /* [4] */
		unsigned int    intp_ptw_ns_msk      : 1  ; /* [5] */
		unsigned int    rsv_51               : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTRAW_P */
union u_smmu_intraw_p {
	/* Define the struct bits */
	struct {
		unsigned int    intp_permis_raw      : 1  ; /* [0] */
		unsigned int    intp_ext_raw         : 1  ; /* [1] */
		unsigned int    intp_tlbmiss_raw     : 1  ; /* [2] */
		unsigned int    intp_ptw_trans_raw   : 1  ; /* [3] */
		unsigned int    intp_ptw_invalid_raw : 1  ; /* [4] */
		unsigned int    intp_ptw_ns_raw      : 1  ; /* [5] */
		unsigned int    rsv_52               : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTSTAT_P */
union u_smmu_intstat_p {
	/* Define the struct bits */
	struct {
		unsigned int    intp_permis_stat      : 1  ; /* [0] */
		unsigned int    intp_ext_stat         : 1  ; /* [1] */
		unsigned int    intp_tlbmiss_stat     : 1  ; /* [2] */
		unsigned int    intp_ptw_trans_stat   : 1  ; /* [3] */
		unsigned int    intp_ptw_invalid_stat : 1  ; /* [4] */
		unsigned int    intp_ptw_ns_stat      : 1  ; /* [5] */
		unsigned int    rsv_53                : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_INTCLR_P */
union u_smmu_intclr_p {
	/* Define the struct bits */
	struct {
		unsigned int    intp_permis_clr      : 1  ; /* [0] */
		unsigned int    intp_ext_clr         : 1  ; /* [1] */
		unsigned int    intp_tlbmiss_clr     : 1  ; /* [2] */
		unsigned int    intp_ptw_trans_clr   : 1  ; /* [3] */
		unsigned int    intp_ptw_invalid_clr : 1  ; /* [4] */
		unsigned int    intp_ptw_ns_clr      : 1  ; /* [5] */
		unsigned int    rsv_54               : 26  ; /* [31:6] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_SCR_P */
union u_smmu_scr_p {
	/* Define the struct bits */
	struct {
		unsigned int    glb_prot_cfg : 1  ; /* [0] */
		unsigned int    rsv_55       : 7  ; /* [7:1] */
		unsigned int    glb_bypass_p : 1  ; /* [8] */
		unsigned int    rsv_56       : 23  ; /* [31:9] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_PCB_SCTRL */
union u_smmu_pcb_sctrl {
	/* Define the struct bits */
	struct {
		unsigned int    pcb_bypass : 1  ; /* [0] */
		unsigned int    rsv_57     : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_PCB_TTBR */
union u_smmu_pcb_ttbr {
	/* Define the struct bits */
	struct {
		unsigned int    pcb_ttbr : 32  ; /* [31:0] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_PCB_TTBCR */
union u_smmu_pcb_ttbcr {
	/* Define the struct bits */
	struct {
		unsigned int    pcb_ttbcr_des : 1  ; /* [0] */
		unsigned int    rsv_58        : 31  ; /* [31:1] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_OFFSET_ADDR_P */
union u_smmu_offset_addr_p {
	/* Define the struct bits */
	struct {
		unsigned int    offset_addr_p : 14  ; /* [13:0] */
		unsigned int    rsv_59        : 18  ; /* [31:14] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAMA_CTRL0_P */
union u_smmu_fama_ctrl0_p {
	/* Define the struct bits */
	struct {
		unsigned int    fama_sdes_msb_p : 7  ; /* [6:0] */
		unsigned int    fama_chn_sel_p  : 1  ; /* [7] */
		unsigned int    fama_bps_msb_p  : 6  ; /* [13:8] */
		unsigned int    rsv_60          : 18  ; /* [31:14] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_SMMU_FAMA_CTRL1_P */
union u_smmu_fama_ctrl1_p {
	/* Define the struct bits */
	struct {
		unsigned int    fama_ptw_msb_p : 7  ; /* [6:0] */
		unsigned int    rsv_61         : 25  ; /* [31:7] */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};


//==============================================================================
/* Define the global struct */
struct s_smmu_comn_regs_type {
	volatile union u_smmu_scr             smmu_scr                ; /* 0 */
	volatile union u_smmu_memctrl         smmu_memctrl            ; /* 4 */
	volatile union u_smmu_lp_ctrl         smmu_lp_ctrl            ; /* 8 */
	volatile union u_smmu_press_remap     smmu_press_remap        ; /* c */
	volatile union u_smmu_intmask_ns      smmu_intmask_ns         ; /* 10 */
	volatile union u_smmu_intraw_ns       smmu_intraw_ns          ; /* 14 */
	volatile union u_smmu_intstat_ns      smmu_intstat_ns         ; /* 18 */
	volatile union u_smmu_intclr_ns       smmu_intclr_ns          ; /* 1c */
	volatile union u_smmu_smrx_ns         smmu_smrx_ns[SMRX_NUM]  ; /* 20 */
	volatile union u_smmu_rld_en0_ns      smmu_rld_en0_ns         ; /* 1f0 */
	volatile union u_smmu_rld_en1_ns      smmu_rld_en1_ns         ; /* 1f4 */
	volatile union u_smmu_rld_en2_ns      smmu_rld_en2_ns         ; /* 1f8 */
	volatile union u_smmu_cb_sctrl        smmu_cb_sctrl           ; /* 200 */
	volatile union u_smmu_cb_ttbr0        smmu_cb_ttbr0           ; /* 204 */
	volatile union u_smmu_cb_ttbr1        smmu_cb_ttbr1           ; /* 208 */
	volatile union u_smmu_cb_ttbcr        smmu_cb_ttbcr           ; /* 20c */
	volatile union u_smmu_offset_addr_ns  smmu_offset_addr_ns     ; /* 210 */
	volatile union u_smmu_scachei_all     smmu_scachei_all        ; /* 214 */
	volatile union u_smmu_scachei_l1      smmu_scachei_l1         ; /* 218 */
	volatile union u_smmu_scachei_l2l3    smmu_scachei_l2l3       ; /* 21c */
	volatile union u_smmu_fama_ctrl0_ns   smmu_fama_ctrl0_ns      ; /* 220 */
	volatile union u_smmu_fama_ctrl1_ns   smmu_fama_ctrl1_ns      ; /* 224 */
	volatile union u_smmu_addr_msb        smmu_addr_msb           ; /* 300 */
	volatile union u_smmu_err_rdaddr      smmu_err_rdaddr         ; /* 304 */
	volatile union u_smmu_err_wraddr      smmu_err_wraddr         ; /* 308 */
	volatile union u_smmu_fault_addr_tcu  smmu_fault_addr_tcu     ; /* 310 */
	volatile union u_smmu_fault_id_tcu    smmu_fault_id_tcu       ; /* 314 */
	volatile union u_smmu_fault_addr_tbux smmu_fault_addr_tbux[4] ; /* 320 */
	volatile union u_smmu_fault_id_tbux   smmu_fault_id_tbux[4]   ; /* 324 */
	volatile union u_smmu_fault_infox     smmu_fault_infox[4]     ; /* 328 */
	volatile union u_smmu_dbgrptr_tlb     smmu_dbgrptr_tlb        ; /* 380 */
	volatile union u_smmu_dbgrdata_tlb    smmu_dbgrdata_tlb       ; /* 384 */
	volatile union u_smmu_dbgrptr_cache   smmu_dbgrptr_cache      ; /* 388 */
	volatile union u_smmu_dbgrdata0_cache smmu_dbgrdata0_cache    ; /* 38c */
	volatile union u_smmu_dbgrdata1_cache smmu_dbgrdata1_cache    ; /* 390 */
	volatile union u_smmu_dbgaxi_ctrl     smmu_dbgaxi_ctrl        ; /* 394 */
	volatile union u_smmu_ova_addr        smmu_ova_addr           ; /* 398 */
	volatile union u_smmu_opa_addr        smmu_opa_addr           ; /* 39c */
	volatile union u_smmu_ova_ctrl        smmu_ova_ctrl           ; /* 3a0 */
	volatile union u_smmu_opref_addr      smmu_opref_addr         ; /* 3a4 */
	volatile union u_smmu_opref_ctrl      smmu_opref_ctrl         ; /* 3a8 */
	volatile union u_smmu_opref_cnt       smmu_opref_cnt          ; /* 3ac */
	volatile union u_smmu_smrx_s          smmu_smrx_s[SMRX_NUM]         ; /* 500 */
	volatile union u_smmu_rld_en0_s       smmu_rld_en0_s          ; /* 6f0 */
	volatile union u_smmu_rld_en1_s       smmu_rld_en1_s          ; /* 6f4 */
	volatile union u_smmu_rld_en2_s       smmu_rld_en2_s          ; /* 6f8 */
	volatile union u_smmu_intmas_s        smmu_intmas_s           ; /* 700 */
	volatile union u_smmu_intraw_s        smmu_intraw_s           ; /* 704 */
	volatile union u_smmu_intstat_s       smmu_intstat_s          ; /* 708 */
	volatile union u_smmu_intclr_s        smmu_intclr_s           ; /* 70c */
	volatile union u_smmu_scr_s           smmu_scr_s              ; /* 710 */
	volatile union u_smmu_scb_sctrl       smmu_scb_sctrl          ; /* 714 */
	volatile union u_smmu_scb_ttbr        smmu_scb_ttbr           ; /* 718 */
	volatile union u_smmu_scb_ttbcr       smmu_scb_ttbcr          ; /* 71c */
	volatile union u_smmu_offset_addr_s   smmu_offset_addr_s      ; /* 720 */
	volatile union u_smmu_fama_ctrl0_s    smmu_fama_ctrl0_s       ; /* 724 */
	volatile union u_smmu_fama_ctrl1_s    smmu_fama_ctrl1_s       ; /* 728 */
	volatile union u_smmu_dbgrptr_tlb_s   smmu_dbgrptr_tlb_s      ; /* 72c */
	volatile union u_smmu_dbgrptr_cache_s smmu_dbgrptr_cache_s    ; /* 730 */
	volatile union u_smmu_override_ctrl_s smmu_override_ctrl_s    ; /* 734 */
	volatile union u_smmu_smrx_p          smmu_smrx_p[SMRX_NUM]         ; /* 10000 */
	volatile union u_smmu_rld_en0_p       smmu_rld_en0_p          ; /* 101f0 */
	volatile union u_smmu_rld_en1_p       smmu_rld_en1_p          ; /* 101f4 */
	volatile union u_smmu_rld_en2_p       smmu_rld_en2_p          ; /* 101f8 */
	volatile union u_smmu_intmas_p        smmu_intmas_p           ; /* 10200 */
	volatile union u_smmu_intraw_p        smmu_intraw_p           ; /* 10204 */
	volatile union u_smmu_intstat_p       smmu_intstat_p          ; /* 10208 */
	volatile union u_smmu_intclr_p        smmu_intclr_p           ; /* 1020c */
	volatile union u_smmu_scr_p           smmu_scr_p              ; /* 10210 */
	volatile union u_smmu_pcb_sctrl       smmu_pcb_sctrl          ; /* 10214 */
	volatile union u_smmu_pcb_ttbr        smmu_pcb_ttbr           ; /* 10218 */
	volatile union u_smmu_pcb_ttbcr       smmu_pcb_ttbcr          ; /* 1021c */
	volatile union u_smmu_offset_addr_p   smmu_offset_addr_p      ; /* 10220 */
	volatile union u_smmu_fama_ctrl0_p    smmu_fama_ctrl0_p       ; /* 10224 */
	volatile union u_smmu_fama_ctrl1_p    smmu_fama_ctrl1_p       ; /* 10228 */

};

struct smmu_nontrus_type_r {
	volatile union u_smmu_scr             smmu_scr                ; /* 0 */
	volatile union u_smmu_memctrl         smmu_memctrl            ; /* 4 */
	volatile union u_smmu_lp_ctrl         smmu_lp_ctrl            ; /* 8 */
	volatile union u_smmu_press_remap     smmu_press_remap        ; /* c */
	volatile union u_smmu_intmask_ns      smmu_intmask_ns         ; /* 10 */
	volatile union u_smmu_intraw_ns       smmu_intraw_ns          ; /* 14 */
	volatile union u_smmu_intstat_ns      smmu_intstat_ns         ; /* 18 */
	volatile union u_smmu_intclr_ns       smmu_intclr_ns          ; /* 1c */
	volatile union u_smmu_smrx_ns         smmu_smrx_ns[SMRX_NUM]        ; /* 20 */
	volatile union u_smmu_rld_en0_ns      smmu_rld_en0_ns         ; /* 1f0 */
	volatile union u_smmu_rld_en1_ns      smmu_rld_en1_ns         ; /* 1f4 */
	volatile union u_smmu_rld_en2_ns      smmu_rld_en2_ns         ; /* 1f8 */
	volatile union u_smmu_cb_sctrl        smmu_cb_sctrl           ; /* 200 */
	volatile union u_smmu_cb_ttbr0        smmu_cb_ttbr0           ; /* 204 */
	volatile union u_smmu_cb_ttbr1        smmu_cb_ttbr1           ; /* 208 */
	volatile union u_smmu_cb_ttbcr        smmu_cb_ttbcr           ; /* 20c */
	volatile union u_smmu_offset_addr_ns  smmu_offset_addr_ns     ; /* 210 */
	volatile union u_smmu_scachei_all     smmu_scachei_all        ; /* 214 */
	volatile union u_smmu_scachei_l1      smmu_scachei_l1         ; /* 218 */
	volatile union u_smmu_scachei_l2l3    smmu_scachei_l2l3       ; /* 21c */
	volatile union u_smmu_fama_ctrl0_ns   smmu_fama_ctrl0_ns      ; /* 220 */
	volatile union u_smmu_fama_ctrl1_ns   smmu_fama_ctrl1_ns      ; /* 224 */
	volatile union u_smmu_addr_msb        smmu_addr_msb           ; /* 300 */
	volatile union u_smmu_err_rdaddr      smmu_err_rdaddr         ; /* 304 */
	volatile union u_smmu_err_wraddr      smmu_err_wraddr         ; /* 308 */
	volatile union u_smmu_fault_addr_tcu  smmu_fault_addr_tcu     ; /* 310 */
	volatile union u_smmu_fault_id_tcu    smmu_fault_id_tcu       ; /* 314 */
	volatile union u_smmu_fault_addr_tbux smmu_fault_addr_tbux[4] ; /* 320 */
	volatile union u_smmu_fault_id_tbux   smmu_fault_id_tbux[4]   ; /* 324 */
	volatile union u_smmu_fault_infox     smmu_fault_infox[4]     ; /* 328 */
	volatile union u_smmu_dbgrptr_tlb     smmu_dbgrptr_tlb        ; /* 380 */
	volatile union u_smmu_dbgrdata_tlb    smmu_dbgrdata_tlb       ; /* 384 */
	volatile union u_smmu_dbgrptr_cache   smmu_dbgrptr_cache      ; /* 388 */
	volatile union u_smmu_dbgrdata0_cache smmu_dbgrdata0_cache    ; /* 38c */
	volatile union u_smmu_dbgrdata1_cache smmu_dbgrdata1_cache    ; /* 390 */
	volatile union u_smmu_dbgaxi_ctrl     smmu_dbgaxi_ctrl        ; /* 394 */
	volatile union u_smmu_ova_addr        smmu_ova_addr           ; /* 398 */
	volatile union u_smmu_opa_addr        smmu_opa_addr           ; /* 39c */
	volatile union u_smmu_ova_ctrl        smmu_ova_ctrl           ; /* 3a0 */
	volatile union u_smmu_opref_addr      smmu_opref_addr         ; /* 3a4 */
	volatile union u_smmu_opref_ctrl      smmu_opref_ctrl         ; /* 3a8 */
	volatile union u_smmu_opref_cnt       smmu_opref_cnt          ; /* 3ac */
};

struct smmu_trusted_type_r {
	volatile union u_smmu_smrx_s          smmu_smrx_s[SMRX_NUM]         ; /* 500 */
	volatile union u_smmu_rld_en0_s       smmu_rld_en0_s          ; /* 6f0 */
	volatile union u_smmu_rld_en1_s       smmu_rld_en1_s          ; /* 6f4 */
	volatile union u_smmu_rld_en2_s       smmu_rld_en2_s          ; /* 6f8 */
	volatile union u_smmu_intmas_s        smmu_intmas_s           ; /* 700 */
	volatile union u_smmu_intraw_s        smmu_intraw_s           ; /* 704 */
	volatile union u_smmu_intstat_s       smmu_intstat_s          ; /* 708 */
	volatile union u_smmu_intclr_s        smmu_intclr_s           ; /* 70c */
	volatile union u_smmu_scr_s           smmu_scr_s              ; /* 710 */
	volatile union u_smmu_scb_sctrl       smmu_scb_sctrl          ; /* 714 */
	volatile union u_smmu_scb_ttbr        smmu_scb_ttbr           ; /* 718 */
	volatile union u_smmu_scb_ttbcr       smmu_scb_ttbcr          ; /* 71c */
	volatile union u_smmu_offset_addr_s   smmu_offset_addr_s      ; /* 720 */
	volatile union u_smmu_fama_ctrl0_s    smmu_fama_ctrl0_s       ; /* 724 */
	volatile union u_smmu_fama_ctrl1_s    smmu_fama_ctrl1_s       ; /* 728 */
	volatile union u_smmu_dbgrptr_tlb_s   smmu_dbgrptr_tlb_s      ; /* 72c */
	volatile union u_smmu_dbgrptr_cache_s smmu_dbgrptr_cache_s    ; /* 730 */
	volatile union u_smmu_override_ctrl_s smmu_override_ctrl_s    ; /* 734 */
};

struct smmu_protect_type_r {
	volatile union u_smmu_smrx_p          smmu_smrx_p[SMRX_NUM]         ; /* 10000 */
	volatile union u_smmu_rld_en0_p       smmu_rld_en0_p          ; /* 101f0 */
	volatile union u_smmu_rld_en1_p       smmu_rld_en1_p          ; /* 101f4 */
	volatile union u_smmu_rld_en2_p       smmu_rld_en2_p          ; /* 101f8 */
	volatile union u_smmu_intmas_p        smmu_intmas_p           ; /* 10200 */
	volatile union u_smmu_intraw_p        smmu_intraw_p           ; /* 10204 */
	volatile union u_smmu_intstat_p       smmu_intstat_p          ; /* 10208 */
	volatile union u_smmu_intclr_p        smmu_intclr_p           ; /* 1020c */
	volatile union u_smmu_scr_p           smmu_scr_p              ; /* 10210 */
	volatile union u_smmu_pcb_sctrl       smmu_pcb_sctrl          ; /* 10214 */
	volatile union u_smmu_pcb_ttbr        smmu_pcb_ttbr           ; /* 10218 */
	volatile union u_smmu_pcb_ttbcr       smmu_pcb_ttbcr          ; /* 1021c */
	volatile union u_smmu_offset_addr_p   smmu_offset_addr_p      ; /* 10220 */
	volatile union u_smmu_fama_ctrl0_p    smmu_fama_ctrl0_p       ; /* 10224 */
	volatile union u_smmu_fama_ctrl1_p    smmu_fama_ctrl1_p       ; /* 10228 */
};

/* Declare the struct pointor of the module smmu_comn */
extern volatile struct s_smmu_comn_regs_type *gopsmmu_comnallreg;

/* Declare the functions that set the member value */
int iSetSMMU_SCR_glb_bypass(unsigned int uglb_bypass);
int iSetSMMU_SCR_rqos_en(unsigned int urqos_en);
int iSetSMMU_SCR_wqos_en(unsigned int uwqos_en);
int iSetSMMU_SCR_cache_l1_en(unsigned int ucache_l1_en);
int iSetSMMU_SCR_cache_l2_en(unsigned int ucache_l2_en);
int iSetSMMU_SCR_rqos(unsigned int urqos);
int iSetSMMU_SCR_wqos(unsigned int uwqos);
int iSetSMMU_SCR_smr_rd_shadow(unsigned int usmr_rd_shadow);
int iSetSMMU_SCR_ptw_pf(unsigned int uptw_pf);
int iSetSMMU_SCR_ptw_mid(unsigned int uptw_mid);
int iSetSMMU_MEMCTRL_mem_ctrl_wr(unsigned int umem_ctrl_wr);
int iSetSMMU_MEMCTRL_mem_ctrl_rd(unsigned int umem_ctrl_rd);
int iSetSMMU_LP_CTRL_auto_clk_gt_en(unsigned int uauto_clk_gt_en);
int iSetSMMU_PRESS_REMAP_remap_sel0(unsigned int uremap_sel0);
int iSetSMMU_PRESS_REMAP_remap_sel1(unsigned int uremap_sel1);
int iSetSMMU_PRESS_REMAP_remap_sel2(unsigned int uremap_sel2);
int iSetSMMU_PRESS_REMAP_remap_sel3(unsigned int uremap_sel3);
int iSetSMMU_PRESS_REMAP_remap_sel4(unsigned int uremap_sel4);
int iSetSMMU_PRESS_REMAP_remap_sel5(unsigned int uremap_sel5);
int iSetSMMU_PRESS_REMAP_remap_sel6(unsigned int uremap_sel6);
int iSetSMMU_PRESS_REMAP_remap_sel7(unsigned int uremap_sel7);
int iSetSMMU_PRESS_REMAP_remap_sel8(unsigned int uremap_sel8);
int iSetSMMU_PRESS_REMAP_remap_sel9(unsigned int uremap_sel9);
int iSetSMMU_PRESS_REMAP_remap_sel10(unsigned int uremap_sel10);
int iSetSMMU_PRESS_REMAP_remap_sel11(unsigned int uremap_sel11);
int iSetSMMU_PRESS_REMAP_remap_sel12(unsigned int uremap_sel12);
int iSetSMMU_PRESS_REMAP_remap_sel13(unsigned int uremap_sel13);
int iSetSMMU_PRESS_REMAP_remap_sel14(unsigned int uremap_sel14);
int iSetSMMU_PRESS_REMAP_remap_sel15(unsigned int uremap_sel15);
int iSetSMMU_INTMASK_NS_intns_permis_msk(unsigned int uintns_permis_msk);
int iSetSMMU_INTMASK_NS_intns_ext_msk(unsigned int uintns_ext_msk);
int iSetSMMU_INTMASK_NS_intns_tlbmiss_msk(unsigned int uintns_tlbmiss_msk);
int iSetSMMU_INTMASK_NS_intns_ptw_trans_msk(unsigned int uintns_ptw_trans_msk);
int iSetSMMU_INTMASK_NS_intns_ptw_invalid_msk(unsigned int uintns_ptw_invalid_msk);
int iSetSMMU_INTMASK_NS_intns_ptw_ns_msk(unsigned int uintns_ptw_ns_msk);
int iSetSMMU_INTRAW_NS_intns_permis_raw(unsigned int uintns_permis_raw);
int iSetSMMU_INTRAW_NS_intns_ext_raw(unsigned int uintns_ext_raw);
int iSetSMMU_INTRAW_NS_intns_tlbmiss_raw(unsigned int uintns_tlbmiss_raw);
int iSetSMMU_INTRAW_NS_intns_ptw_trans_raw(unsigned int uintns_ptw_trans_raw);
int iSetSMMU_INTRAW_NS_intns_ptw_invalid_raw(unsigned int uintns_ptw_invalid_raw);
int iSetSMMU_INTRAW_NS_intns_ptw_ns_raw(unsigned int uintns_ptw_ns_raw);
int iSetSMMU_INTSTAT_NS_intns_permis_stat(unsigned int uintns_permis_stat);
int iSetSMMU_INTSTAT_NS_intns_ext_stat(unsigned int uintns_ext_stat);
int iSetSMMU_INTSTAT_NS_intns_tlbmiss_stat(unsigned int uintns_tlbmiss_stat);
int iSetSMMU_INTSTAT_NS_intns_ptw_trans_stat(unsigned int uintns_ptw_trans_stat);
int iSetSMMU_INTSTAT_NS_intns_ptw_invalid_stat(unsigned int uintns_ptw_invalid_stat);
int iSetSMMU_INTSTAT_NS_intns_ptw_ns_stat(unsigned int uintns_ptw_ns_stat);
int iSetSMMU_INTCLR_NS_intns_permis_clr(unsigned int uintns_permis_clr);
int iSetSMMU_INTCLR_NS_intns_ext_clr(unsigned int uintns_ext_clr);
int iSetSMMU_INTCLR_NS_intns_tlbmiss_clr(unsigned int uintns_tlbmiss_clr);
int iSetSMMU_INTCLR_NS_intns_ptw_trans_clr(unsigned int uintns_ptw_trans_clr);
int iSetSMMU_INTCLR_NS_intns_ptw_invalid_clr(unsigned int uintns_ptw_invalid_clr);
int iSetSMMU_INTCLR_NS_intns_ptw_ns_clr(unsigned int uintns_ptw_ns_clr);
int iSetSMMU_SMRX_NS_smr_bypass(unsigned int usmr_bypass);
int iSetSMMU_SMRX_NS_smr_ptw_qos(unsigned int usmr_ptw_qos);
int iSetSMMU_SMRX_NS_smr_invld_en(unsigned int usmr_invld_en);
int iSetSMMU_SMRX_NS_smr_offset_addr(unsigned int usmr_offset_addr);
int iSetSMMU_RLD_EN0_NS_smr_rld_en0(unsigned int usmr_rld_en0);
int iSetSMMU_RLD_EN1_NS_smr_rld_en1(unsigned int usmr_rld_en1);
int iSetSMMU_RLD_EN2_NS_smr_rld_en2(unsigned int usmr_rld_en2);
int iSetSMMU_CB_SCTRL_cb_bypass(unsigned int ucb_bypass);
int iSetSMMU_CB_TTBR0_cb_ttbr0(unsigned int ucb_ttbr0);
int iSetSMMU_CB_TTBR1_cb_ttbr1(unsigned int ucb_ttbr1);
int iSetSMMU_CB_TTBCR_cb_ttbcr_des(unsigned int ucb_ttbcr_des);
int iSetSMMU_CB_TTBCR_cb_ttbcr_t0sz(unsigned int ucb_ttbcr_t0sz);
int iSetSMMU_CB_TTBCR_cb_ttbcr_n(unsigned int ucb_ttbcr_n);
int iSetSMMU_CB_TTBCR_cb_ttbcr_t1sz(unsigned int ucb_ttbcr_t1sz);
int iSetSMMU_OFFSET_ADDR_NS_offset_addr_ns(unsigned int uoffset_addr_ns);
int iSetSMMU_SCACHEI_ALL_cache_all_invalid(unsigned int ucache_all_invalid);
int iSetSMMU_SCACHEI_ALL_cache_all_level(unsigned int ucache_all_level);
int iSetSMMU_SCACHEI_L1_cache_l1_invalid(unsigned int ucache_l1_invalid);
int iSetSMMU_SCACHEI_L1_cache_l1_security(unsigned int ucache_l1_security);
int iSetSMMU_SCACHEI_L2L3_cache_l2l3_invalid(unsigned int ucache_l2l3_invalid);
int iSetSMMU_SCACHEI_L2L3_cache_l2l3_strmid(unsigned int ucache_l2l3_strmid);
int iSetSMMU_FAMA_CTRL0_NS_fama_sdes_msb_ns(unsigned int ufama_sdes_msb_ns);
int iSetSMMU_FAMA_CTRL0_NS_fama_chn_sel_ns(unsigned int ufama_chn_sel_ns);
int iSetSMMU_FAMA_CTRL0_NS_fama_bps_msb_ns(unsigned int ufama_bps_msb_ns);
int iSetSMMU_FAMA_CTRL1_NS_fama_ptw_msb_ns(unsigned int ufama_ptw_msb_ns);
int iSetSMMU_ADDR_MSB_msb_errrd(unsigned int umsb_errrd);
int iSetSMMU_ADDR_MSB_msb_errwr(unsigned int umsb_errwr);
int iSetSMMU_ADDR_MSB_msb_ova(unsigned int umsb_ova);
int iSetSMMU_ERR_RDADDR_err_rd_addr(unsigned int uerr_rd_addr);
int iSetSMMU_ERR_WRADDR_err_wr_addr(unsigned int uerr_wr_addr);
int iSetSMMU_FAULT_ADDR_TCU_fault_addr_tcu(unsigned int ufault_addr_tcu);
int iSetSMMU_FAULT_ID_TCU_fault_index_id_tcu(unsigned int ufault_index_id_tcu);
int iSetSMMU_FAULT_ID_TCU_fault_strm_id_tcu(unsigned int ufault_strm_id_tcu);
int iSetSMMU_FAULT_ADDR_TBUX_fault_addr_tbu(unsigned int ufault_addr_tbu);
int iSetSMMU_FAULT_ID_TBUX_fault_index_id_tbu(unsigned int ufault_index_id_tbu);
int iSetSMMU_FAULT_ID_TBUX_fault_strm_id_tbu(unsigned int ufault_strm_id_tbu);
int iSetSMMU_FAULT_INFOX_fault_tlbmiss_err_tbu(unsigned int ufault_tlbmiss_err_tbu);
int iSetSMMU_FAULT_INFOX_fault_permis_err_tbu(unsigned int ufault_permis_err_tbu);
int iSetSMMU_FAULT_INFOX_fautl_ext_err_tbu(unsigned int ufautl_ext_err_tbu);
int iSetSMMU_FAULT_INFOX_fault_ext_err_id_tbu(unsigned int ufault_ext_err_id_tbu);
int iSetSMMU_DBGRPTR_TLB_dbg_tlbword_pointer(unsigned int udbg_tlbword_pointer);
int iSetSMMU_DBGRPTR_TLB_dbg_tlbentry_pointer(unsigned int udbg_tlbentry_pointer);
int iSetSMMU_DBGRPTR_TLB_dbg_tlb_type(unsigned int udbg_tlb_type);
int iSetSMMU_DBGRDATA_TLB_dbg_tlb_rdata(unsigned int udbg_tlb_rdata);
int iSetSMMU_DBGRPTR_CACHE_dbg_cache_level(unsigned int udbg_cache_level);
int iSetSMMU_DBGRPTR_CACHE_dbg_cache_l1_pointer(unsigned int udbg_cache_l1_pointer);
int iSetSMMU_DBGRPTR_CACHE_dbg_cache_l1_ns(unsigned int udbg_cache_l1_ns);
int iSetSMMU_DBGRPTR_CACHE_dbg_cache_l2_strmid(unsigned int udbg_cache_l2_strmid);
int iSetSMMU_DBGRPTR_CACHE_dbg_cache_l1_prot(unsigned int udbg_cache_l1_prot);
int iSetSMMU_DBGRDATA0_CACHE_dbg_cache_rdata0(unsigned int udbg_cache_rdata0);
int iSetSMMU_DBGRDATA1_CACHE_dbg_cache_rdata1(unsigned int udbg_cache_rdata1);
int iSetSMMU_DBGAXI_CTRL_dbg_axilock_en(unsigned int udbg_axilock_en);
int iSetSMMU_OVA_ADDR_override_va(unsigned int uoverride_va);
int iSetSMMU_OPA_ADDR_override_acquire_pa(unsigned int uoverride_acquire_pa);
int iSetSMMU_OPA_ADDR_override_pa_done(unsigned int uoverride_pa_done);
int iSetSMMU_OVA_CTRL_override_va_cfg(unsigned int uoverride_va_cfg);
int iSetSMMU_OVA_CTRL_override_va_type(unsigned int uoverride_va_type);
int iSetSMMU_OVA_CTRL_override_va_indexid(unsigned int uoverride_va_indexid);
int iSetSMMU_OVA_CTRL_override_va_strmid(unsigned int uoverride_va_strmid);
int iSetSMMU_OVA_CTRL_override_tbu_num(unsigned int uoverride_tbu_num);
int iSetSMMU_OPREF_ADDR_override_pref_addr(unsigned int uoverride_pref_addr);
int iSetSMMU_OPREF_CTRL_override_pref_cfg(unsigned int uoverride_pref_cfg);
int iSetSMMU_OPREF_CTRL_override_pref_type(unsigned int uoverride_pref_type);
int iSetSMMU_OPREF_CTRL_override_pref_initial(unsigned int uoverride_pref_initial);
int iSetSMMU_OPREF_CTRL_override_pref_indexid(unsigned int uoverride_pref_indexid);
int iSetSMMU_OPREF_CTRL_override_pref_strmid(unsigned int uoverride_pref_strmid);
int iSetSMMU_OPREF_CNT_override_pref_cnt(unsigned int uoverride_pref_cnt);
int iSetSMMU_SMRX_S_smr_nscfg(unsigned int usmr_nscfg);
int iSetSMMU_SMRX_S_smr_nscfg_en(unsigned int usmr_nscfg_en);
int iSetSMMU_SMRX_S_smr_bypass_s(unsigned int usmr_bypass_s);
int iSetSMMU_SMRX_S_ptw_mid_sel_s(unsigned int uptw_mid_sel_s);
int iSetSMMU_SMRX_S_smr_mid_en_s(unsigned int usmr_mid_en_s);
int iSetSMMU_SMRX_S_smr_mid_s(unsigned int usmr_mid_s);
int iSetSMMU_SMRX_S_smr_offset_addr_s(unsigned int usmr_offset_addr_s);
int iSetSMMU_RLD_EN0_S_smr_rld_en0_s(unsigned int usmr_rld_en0_s);
int iSetSMMU_RLD_EN1_S_smr_rld_en1_s(unsigned int usmr_rld_en1_s);
int iSetSMMU_RLD_EN2_S_smr_rld_en2_s(unsigned int usmr_rld_en2_s);
int iSetSMMU_INTMAS_S_ints_permis_msk(unsigned int uints_permis_msk);
int iSetSMMU_INTMAS_S_ints_ext_msk(unsigned int uints_ext_msk);
int iSetSMMU_INTMAS_S_ints_tlbmiss_msk(unsigned int uints_tlbmiss_msk);
int iSetSMMU_INTMAS_S_ints_ptw_trans_msk(unsigned int uints_ptw_trans_msk);
int iSetSMMU_INTMAS_S_ints_ptw_invalid_msk(unsigned int uints_ptw_invalid_msk);
int iSetSMMU_INTMAS_S_ints_ptw_ns_msk(unsigned int uints_ptw_ns_msk);
int iSetSMMU_INTRAW_S_ints_permis_raw(unsigned int uints_permis_raw);
int iSetSMMU_INTRAW_S_ints_ext_raw(unsigned int uints_ext_raw);
int iSetSMMU_INTRAW_S_ints_tlbmiss_raw(unsigned int uints_tlbmiss_raw);
int iSetSMMU_INTRAW_S_ints_ptw_trans_raw(unsigned int uints_ptw_trans_raw);
int iSetSMMU_INTRAW_S_ints_ptw_invalid_raw(unsigned int uints_ptw_invalid_raw);
int iSetSMMU_INTRAW_S_ints_ptw_ns_raw(unsigned int uints_ptw_ns_raw);
int iSetSMMU_INTSTAT_S_ints_permis_stat(unsigned int uints_permis_stat);
int iSetSMMU_INTSTAT_S_ints_ext_stat(unsigned int uints_ext_stat);
int iSetSMMU_INTSTAT_S_ints_tlbmiss_stat(unsigned int uints_tlbmiss_stat);
int iSetSMMU_INTSTAT_S_ints_ptw_trans_stat(unsigned int uints_ptw_trans_stat);
int iSetSMMU_INTSTAT_S_ints_ptw_invalid_stat(unsigned int uints_ptw_invalid_stat);
int iSetSMMU_INTSTAT_S_ints_ptw_ns_stat(unsigned int uints_ptw_ns_stat);
int iSetSMMU_INTCLR_S_ints_permis_clr(unsigned int uints_permis_clr);
int iSetSMMU_INTCLR_S_ints_ext_clr(unsigned int uints_ext_clr);
int iSetSMMU_INTCLR_S_ints_tlbmiss_clr(unsigned int uints_tlbmiss_clr);
int iSetSMMU_INTCLR_S_ints_ptw_trans_clr(unsigned int uints_ptw_trans_clr);
int iSetSMMU_INTCLR_S_ints_ptw_invalid_clr(unsigned int uints_ptw_invalid_clr);
int iSetSMMU_INTCLR_S_ints_ptw_ns_clr(unsigned int uints_ptw_ns_clr);
int iSetSMMU_SCR_S_glb_nscfg(unsigned int uglb_nscfg);
int iSetSMMU_SCR_S_mid_s(unsigned int umid_s);
int iSetSMMU_SCR_S_glb_bypass_s(unsigned int uglb_bypass_s);
int iSetSMMU_SCR_S_mid_en_s(unsigned int umid_en_s);
int iSetSMMU_SCB_SCTRL_scb_bypass(unsigned int uscb_bypass);
int iSetSMMU_SCB_TTBR_scb_ttbr(unsigned int uscb_ttbr);
int iSetSMMU_SCB_TTBCR_scb_ttbcr_des(unsigned int uscb_ttbcr_des);
int iSetSMMU_OFFSET_ADDR_S_offset_addr_s(unsigned int uoffset_addr_s);
int iSetSMMU_FAMA_CTRL0_S_fama_sdes_msb_s(unsigned int ufama_sdes_msb_s);
int iSetSMMU_FAMA_CTRL0_S_fama_chn_sel_s(unsigned int ufama_chn_sel_s);
int iSetSMMU_FAMA_CTRL0_S_fama_bps_msb_s(unsigned int ufama_bps_msb_s);
int iSetSMMU_FAMA_CTRL1_S_fama_ptw_msb_s(unsigned int ufama_ptw_msb_s);
int iSetSMMU_DBGRPTR_TLB_S_dbg_tlb_en(unsigned int udbg_tlb_en);
int iSetSMMU_DBGRPTR_CACHE_S_dbg_cache_en(unsigned int udbg_cache_en);
int iSetSMMU_OVERRIDE_CTRL_S_override_va_security(unsigned int uoverride_va_security);
int iSetSMMU_SMRX_P_smr_protect_en(unsigned int usmr_protect_en);
int iSetSMMU_SMRX_P_smr_bypass_p(unsigned int usmr_bypass_p);
int iSetSMMU_SMRX_P_smr_offset_addr_p(unsigned int usmr_offset_addr_p);
int iSetSMMU_RLD_EN0_P_smr_rld_en0_p(unsigned int usmr_rld_en0_p);
int iSetSMMU_RLD_EN1_P_smr_rld_en1_p(unsigned int usmr_rld_en1_p);
int iSetSMMU_RLD_EN2_P_smr_rld_en2_p(unsigned int usmr_rld_en2_p);
int iSetSMMU_INTMAS_P_intp_permis_msk(unsigned int uintp_permis_msk);
int iSetSMMU_INTMAS_P_intp_ext_msk(unsigned int uintp_ext_msk);
int iSetSMMU_INTMAS_P_intp_tlbmiss_msk(unsigned int uintp_tlbmiss_msk);
int iSetSMMU_INTMAS_P_intp_ptw_trans_msk(unsigned int uintp_ptw_trans_msk);
int iSetSMMU_INTMAS_P_intp_ptw_invalid_msk(unsigned int uintp_ptw_invalid_msk);
int iSetSMMU_INTMAS_P_intp_ptw_ns_msk(unsigned int uintp_ptw_ns_msk);
int iSetSMMU_INTRAW_P_intp_permis_raw(unsigned int uintp_permis_raw);
int iSetSMMU_INTRAW_P_intp_ext_raw(unsigned int uintp_ext_raw);
int iSetSMMU_INTRAW_P_intp_tlbmiss_raw(unsigned int uintp_tlbmiss_raw);
int iSetSMMU_INTRAW_P_intp_ptw_trans_raw(unsigned int uintp_ptw_trans_raw);
int iSetSMMU_INTRAW_P_intp_ptw_invalid_raw(unsigned int uintp_ptw_invalid_raw);
int iSetSMMU_INTRAW_P_intp_ptw_ns_raw(unsigned int uintp_ptw_ns_raw);
int iSetSMMU_INTSTAT_P_intp_permis_stat(unsigned int uintp_permis_stat);
int iSetSMMU_INTSTAT_P_intp_ext_stat(unsigned int uintp_ext_stat);
int iSetSMMU_INTSTAT_P_intp_tlbmiss_stat(unsigned int uintp_tlbmiss_stat);
int iSetSMMU_INTSTAT_P_intp_ptw_trans_stat(unsigned int uintp_ptw_trans_stat);
int iSetSMMU_INTSTAT_P_intp_ptw_invalid_stat(unsigned int uintp_ptw_invalid_stat);
int iSetSMMU_INTSTAT_P_intp_ptw_ns_stat(unsigned int uintp_ptw_ns_stat);
int iSetSMMU_INTCLR_P_intp_permis_clr(unsigned int uintp_permis_clr);
int iSetSMMU_INTCLR_P_intp_ext_clr(unsigned int uintp_ext_clr);
int iSetSMMU_INTCLR_P_intp_tlbmiss_clr(unsigned int uintp_tlbmiss_clr);
int iSetSMMU_INTCLR_P_intp_ptw_trans_clr(unsigned int uintp_ptw_trans_clr);
int iSetSMMU_INTCLR_P_intp_ptw_invalid_clr(unsigned int uintp_ptw_invalid_clr);
int iSetSMMU_INTCLR_P_intp_ptw_ns_clr(unsigned int uintp_ptw_ns_clr);
int iSetSMMU_SCR_P_glb_prot_cfg(unsigned int uglb_prot_cfg);
int iSetSMMU_SCR_P_glb_bypass_p(unsigned int uglb_bypass_p);
int iSetSMMU_PCB_SCTRL_pcb_bypass(unsigned int upcb_bypass);
int iSetSMMU_PCB_TTBR_pcb_ttbr(unsigned int upcb_ttbr);
int iSetSMMU_PCB_TTBCR_pcb_ttbcr_des(unsigned int upcb_ttbcr_des);
int iSetSMMU_OFFSET_ADDR_P_offset_addr_p(unsigned int uoffset_addr_p);
int iSetSMMU_FAMA_CTRL0_P_fama_sdes_msb_p(unsigned int ufama_sdes_msb_p);
int iSetSMMU_FAMA_CTRL0_P_fama_chn_sel_p(unsigned int ufama_chn_sel_p);
int iSetSMMU_FAMA_CTRL0_P_fama_bps_msb_p(unsigned int ufama_bps_msb_p);
int iSetSMMU_FAMA_CTRL1_P_fama_ptw_msb_p(unsigned int ufama_ptw_msb_p);

#endif /* __SMMU_COMN_DRV_PRIV_CS_H__ */
