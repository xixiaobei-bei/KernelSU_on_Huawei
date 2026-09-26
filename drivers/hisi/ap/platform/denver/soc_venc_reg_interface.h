#ifndef __SOC_VENC_REG_INTERFACE_H__
#define __SOC_VENC_REG_INTERFACE_H__ 
typedef union {
    struct {
        unsigned int vcpi_enable_ve_eop : 1 ;
        unsigned int vcpi_enable_vedu_slice_end : 1 ;
        unsigned int vcpi_enable_ve_buffull : 1 ;
        unsigned int vcpi_enable_ve_pbitsover : 1 ;
        unsigned int vcpi_enable_line_pos : 1 ;
        unsigned int vcpi_enable_ppdf_dec_err : 1 ;
        unsigned int vcpi_enable_vedu_timeout : 1 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_INTMASK;
typedef union {
    struct {
        unsigned int vcpi_clr_ve_eop : 1 ;
        unsigned int vcpi_clr_vedu_slice_end : 1 ;
        unsigned int vcpi_clr_ve_buffull : 1 ;
        unsigned int vcpi_clr_ve_pbitsover : 1 ;
        unsigned int vcpi_clr_line_pos : 1 ;
        unsigned int vcpi_clr_ppdf_dec_err : 1 ;
        unsigned int vcpi_clr_vedu_timeout : 1 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_INTCLR;
typedef union {
    struct {
        unsigned int vcpi_vstart : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_START;
typedef union {
    struct {
        unsigned int vedu_bus_idle_req : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_BUS_IDLE_REQ;
typedef union {
    struct {
        unsigned int vcpi_vedsel : 1 ;
        unsigned int vedu_selfrst_en : 1 ;
        unsigned int vcpi_protocol : 2 ;
        unsigned int vcpi_cfg_mode : 1 ;
        unsigned int vcpi_slice_int_en : 1 ;
        unsigned int vcpi_sao_luma : 1 ;
        unsigned int vcpi_sao_chroma : 1 ;
        unsigned int vcpi_rec_cmp_en : 1 ;
        unsigned int vcpi_img_improve_en : 1 ;
        unsigned int vcpi_frame_type : 2 ;
        unsigned int vcpi_entropy_mode : 1 ;
        unsigned int vcpi_long_term_refpic : 1 ;
        unsigned int vcpi_ref_num : 1 ;
        unsigned int vcpi_pr_inter_en : 1 ;
        unsigned int vcpi_idr_pic : 1 ;
        unsigned int vcpi_pskip_en : 1 ;
        unsigned int vcpi_trans_mode : 2 ;
        unsigned int vcpi_blk8_inter : 1 ;
        unsigned int vcpi_sobel_weight_en : 1 ;
        unsigned int vcpi_high_speed_en : 1 ;
        unsigned int vcpi_tiles_en : 1 ;
        unsigned int vcpi_10bit_mode : 2 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vcpi_time_en : 1 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vcpi_ref_cmp_en : 1 ;
        unsigned int vcpi_refc_nload : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_MODE;
typedef union {
    struct {
        unsigned int emar_rec_b7_scramble : 1 ;
        unsigned int emar_rec_b8_scramble : 2 ;
        unsigned int emar_ori_y_b7_scramble : 2 ;
        unsigned int emar_ori_y_b8_scramble : 2 ;
        unsigned int emar_ori_y_b9_scramble : 2 ;
        unsigned int emar_ori_uv_b7_scramble : 2 ;
        unsigned int emar_ori_uv_b8_scramble : 2 ;
        unsigned int emar_ori_uv_b9_scramble : 2 ;
        unsigned int reserved_0 : 17 ;
    } bits;
    unsigned int u32;
} U_VEDU_EMAR_SCRAMBLE_TYPE;
typedef union {
    struct {
        unsigned int vcpi_rc_cu_qp_en : 1 ;
        unsigned int vcpi_rc_row_qp_en : 1 ;
        unsigned int vcpi_move_scene_en : 1 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_strong_edge_move_en : 1 ;
        unsigned int reserved_1 : 1 ;
        unsigned int reserved_2 : 2 ;
        unsigned int vcpi_rc_low_luma_en : 1 ;
        unsigned int reserved_3 : 1 ;
        unsigned int reserved_4 : 1 ;
        unsigned int vcpi_rd_min_sad_flag_en : 1 ;
        unsigned int reserved_5 : 2 ;
        unsigned int vcpi_low_min_sad_en : 1 ;
        unsigned int vcpi_prev_min_sad_en : 1 ;
        unsigned int vcpi_qpgld_en : 1 ;
        unsigned int vcpi_map_roikeep_en : 1 ;
        unsigned int vcpi_flat_region_en : 1 ;
        unsigned int vcpi_qp_restrain_large_sad : 1 ;
        unsigned int reserved_6 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_RC_ENABLE;
typedef union {
    struct {
        unsigned int vlcst_ptbits_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_VLCST_PTBITS_EN;
typedef union {
    struct {
        unsigned int ppfd_st_bypass_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_PPFD_ST_CFG;
typedef union {
    struct {
        unsigned int vcpi_cr_qp_offset : 5 ;
        unsigned int vcpi_cb_qp_offset : 5 ;
        unsigned int vcpi_frm_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vcpi_intra_det_qp_en : 1 ;
        unsigned int vcpi_rc_cu_madi_en : 1 ;
        unsigned int reserved_1 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_QPCFG;
typedef union {
    struct {
        unsigned int qpg_min_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_cu_qp_delta_enable_flag : 1 ;
        unsigned int reserved_2 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_MAX_MIN_QP;
typedef union {
    struct {
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_smart_get_cu32_qp_mode : 2 ;
        unsigned int qpg_smart_get_cu64_qp_mode : 2 ;
        unsigned int qpg_qp_detlta_size_cu64 : 1 ;
        unsigned int reserved_1 : 25 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_SMART_REG;
typedef union {
    struct {
        unsigned int qpg_qp_delta : 4 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_row_target_bits : 25 ;
        unsigned int reserved_1 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_ROW_TARGET_BITS;
typedef union {
    struct {
        unsigned int qpg_ave_lcu_bits : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_AVERAGE_LCU_BITS;
typedef union {
    struct {
        unsigned int qpg_cu_qp_delta_thresh0 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh1 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh2 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh3 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0;
typedef union {
    struct {
        unsigned int qpg_cu_qp_delta_thresh4 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh5 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh6 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh7 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1;
typedef union {
    struct {
        unsigned int qpg_cu_qp_delta_thresh8 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh9 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh10 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh11 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2;
typedef union {
    struct {
        unsigned int qpg_cu_qp_delta_thresh12 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh13 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh14 : 8 ;
        unsigned int qpg_cu_qp_delta_thresh15 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3;
typedef union {
    struct {
        unsigned int qpg_qp_delta_level_0 : 2 ;
        unsigned int qpg_qp_delta_level_1 : 2 ;
        unsigned int qpg_qp_delta_level_2 : 2 ;
        unsigned int qpg_qp_delta_level_3 : 2 ;
        unsigned int qpg_qp_delta_level_4 : 2 ;
        unsigned int qpg_qp_delta_level_5 : 2 ;
        unsigned int qpg_qp_delta_level_6 : 2 ;
        unsigned int qpg_qp_delta_level_7 : 2 ;
        unsigned int qpg_qp_delta_level_8 : 2 ;
        unsigned int qpg_qp_delta_level_9 : 2 ;
        unsigned int qpg_qp_delta_level_10 : 2 ;
        unsigned int qpg_qp_delta_level_11 : 2 ;
        unsigned int qpg_qp_delta_level_12 : 2 ;
        unsigned int qpg_qp_delta_level_13 : 2 ;
        unsigned int qpg_qp_delta_level_14 : 2 ;
        unsigned int qpg_qp_delta_level_15 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_DELTA_LEVEL;
typedef union {
    struct {
        unsigned int qpg_qp_madi_switch_thr : 5 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_MADI_SWITCH_THR;
typedef union {
    struct {
        unsigned int vcpi_rc_cu_sad_en : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_sad_switch_thr : 5 ;
        unsigned int reserved_1 : 7 ;
        unsigned int vcpi_rc_cu_sad_mod : 2 ;
        unsigned int vcpi_rc_cu_sad_offset : 8 ;
        unsigned int vcpi_rc_cu_sad_gain : 4 ;
        unsigned int reserved_2 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CURR_SAD_EN;
typedef union {
    struct {
        unsigned int vcpi_curr_sad_level_0 : 2 ;
        unsigned int vcpi_curr_sad_level_1 : 2 ;
        unsigned int vcpi_curr_sad_level_2 : 2 ;
        unsigned int vcpi_curr_sad_level_3 : 2 ;
        unsigned int vcpi_curr_sad_level_4 : 2 ;
        unsigned int vcpi_curr_sad_level_5 : 2 ;
        unsigned int vcpi_curr_sad_level_6 : 2 ;
        unsigned int vcpi_curr_sad_level_7 : 2 ;
        unsigned int vcpi_curr_sad_level_8 : 2 ;
        unsigned int vcpi_curr_sad_level_9 : 2 ;
        unsigned int vcpi_curr_sad_level_10 : 2 ;
        unsigned int vcpi_curr_sad_level_11 : 2 ;
        unsigned int vcpi_curr_sad_level_12 : 2 ;
        unsigned int vcpi_curr_sad_level_13 : 2 ;
        unsigned int vcpi_curr_sad_level_14 : 2 ;
        unsigned int vcpi_curr_sad_level_15 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CURR_SAD_LEVEL;
typedef union {
    struct {
        unsigned int vcpi_curr_sad_thresh_0 : 8 ;
        unsigned int vcpi_curr_sad_thresh_1 : 8 ;
        unsigned int vcpi_curr_sad_thresh_2 : 8 ;
        unsigned int vcpi_curr_sad_thresh_3 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CURR_SAD_THRESH0;
typedef union {
    struct {
        unsigned int vcpi_curr_sad_thresh_4 : 8 ;
        unsigned int vcpi_curr_sad_thresh_5 : 8 ;
        unsigned int vcpi_curr_sad_thresh_6 : 8 ;
        unsigned int vcpi_curr_sad_thresh_7 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CURR_SAD_THRESH1;
typedef union {
    struct {
        unsigned int vcpi_curr_sad_thresh_8 : 8 ;
        unsigned int vcpi_curr_sad_thresh_9 : 8 ;
        unsigned int vcpi_curr_sad_thresh_10 : 8 ;
        unsigned int vcpi_curr_sad_thresh_11 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CURR_SAD_THRESH2;
typedef union {
    struct {
        unsigned int vcpi_curr_sad_thresh_12 : 8 ;
        unsigned int vcpi_curr_sad_thresh_13 : 8 ;
        unsigned int vcpi_curr_sad_thresh_14 : 8 ;
        unsigned int vcpi_curr_sad_thresh_15 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CURR_SAD_THRESH3;
typedef union {
    struct {
        unsigned int vcpi_rc_luma_en : 1 ;
        unsigned int vcpi_rc_luma_mode : 2 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_rc_luma_switch_thr : 5 ;
        unsigned int reserved_1 : 3 ;
        unsigned int vcpi_rc_luma_low_madi_thr : 8 ;
        unsigned int vcpi_rc_luma_high_madi_thr : 8 ;
        unsigned int reserved_2 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_LUMA_RC;
typedef union {
    struct {
        unsigned int vcpi_rc_luma_level_0 : 2 ;
        unsigned int vcpi_rc_luma_level_1 : 2 ;
        unsigned int vcpi_rc_luma_level_2 : 2 ;
        unsigned int vcpi_rc_luma_level_3 : 2 ;
        unsigned int vcpi_rc_luma_level_4 : 2 ;
        unsigned int vcpi_rc_luma_level_5 : 2 ;
        unsigned int vcpi_rc_luma_level_6 : 2 ;
        unsigned int vcpi_rc_luma_level_7 : 2 ;
        unsigned int vcpi_rc_luma_level_8 : 2 ;
        unsigned int vcpi_rc_luma_level_9 : 2 ;
        unsigned int vcpi_rc_luma_level_10 : 2 ;
        unsigned int vcpi_rc_luma_level_11 : 2 ;
        unsigned int vcpi_rc_luma_level_12 : 2 ;
        unsigned int vcpi_rc_luma_level_13 : 2 ;
        unsigned int vcpi_rc_luma_level_14 : 2 ;
        unsigned int vcpi_rc_luma_level_15 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_LUMA_LEVEL;
typedef union {
    struct {
        unsigned int vcpi_rc_luma_thresh_0 : 8 ;
        unsigned int vcpi_rc_luma_thresh_1 : 8 ;
        unsigned int vcpi_rc_luma_thresh_2 : 8 ;
        unsigned int vcpi_rc_luma_thresh_3 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_LUMA_THRESH0;
typedef union {
    struct {
        unsigned int vcpi_rc_luma_thresh_4 : 8 ;
        unsigned int vcpi_rc_luma_thresh_5 : 8 ;
        unsigned int vcpi_rc_luma_thresh_6 : 8 ;
        unsigned int vcpi_rc_luma_thresh_7 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_LUMA_THRESH1;
typedef union {
    struct {
        unsigned int vcpi_rc_luma_thresh_8 : 8 ;
        unsigned int vcpi_rc_luma_thresh_9 : 8 ;
        unsigned int vcpi_rc_luma_thresh_10 : 8 ;
        unsigned int vcpi_rc_luma_thresh_11 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_LUMA_THRESH2;
typedef union {
    struct {
        unsigned int vcpi_rc_luma_thresh_12 : 8 ;
        unsigned int vcpi_rc_luma_thresh_13 : 8 ;
        unsigned int vcpi_rc_luma_thresh_14 : 8 ;
        unsigned int vcpi_rc_luma_thresh_15 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_LUMA_THRESH3;
typedef union {
    struct {
        unsigned int vcpi_bg_info_st_en : 1 ;
        unsigned int vcpi_bg_refresh_st_en : 1 ;
        unsigned int vcpi_bg_ld_en : 1 ;
        unsigned int vcpi_bg_en : 1 ;
        unsigned int vcpi_bg_stat_frame : 1 ;
        unsigned int vcpi_bg_th_frame : 1 ;
        unsigned int vcpi_bg_percentage_en : 1 ;
        unsigned int vcpi_bg_start_idc : 1 ;
        unsigned int vcpi_bg_start_frame : 1 ;
        unsigned int vcpi_bg_reset_diff_en : 1 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_ENABLE;
typedef union {
    struct {
        unsigned int vcpi_bg_alpha_fix_0 : 11 ;
        unsigned int reserved_0 : 5 ;
        unsigned int vcpi_bg_alpha_fix_1 : 11 ;
        unsigned int reserved_1 : 5 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_FLT_PARA0;
typedef union {
    struct {
        unsigned int vcpi_bg_belta_fix_0 : 11 ;
        unsigned int reserved_0 : 5 ;
        unsigned int vcpi_bg_belta_fix_1 : 11 ;
        unsigned int reserved_1 : 5 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_FLT_PARA1;
typedef union {
    struct {
        unsigned int vcpi_bg_delta_fix_0 : 11 ;
        unsigned int reserved_0 : 5 ;
        unsigned int vcpi_bg_delta_fix_1 : 11 ;
        unsigned int reserved_1 : 5 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_FLT_PARA2;
typedef union {
    struct {
        unsigned int vcpi_bg_th_ave : 10 ;
        unsigned int vcpi_bg_ave_update_th : 10 ;
        unsigned int vcpi_bg_stat_th : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_THR0;
typedef union {
    struct {
        unsigned int vcpi_bg_dist_th : 10 ;
        unsigned int vcpi_bg_frame_num : 10 ;
        unsigned int vcpi_bg_min_diff : 10 ;
        unsigned int reserved_0 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_THR1;
typedef union {
    struct {
        unsigned int vcpi_bgl_stride : 16 ;
        unsigned int vcpi_bgc_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_STRIDE;
typedef union {
    struct {
        unsigned int vcpi_bgl_ext_stride : 16 ;
        unsigned int vcpi_bgc_ext_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_BG_EXT_STRIDE;
typedef union {
    struct {
        unsigned int vcpi_r_outstanding : 7 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_w_outstanding : 4 ;
        unsigned int reserved_1 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OUTSTD;
typedef union {
    struct {
        unsigned int vctrl_lcu_performance_baseline : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_LCU_BASELINE;
typedef union {
    struct {
        unsigned int rashds_svt_rtsel : 2 ;
        unsigned int rashds_lvt_rtsel : 2 ;
        unsigned int rashds_svt_wtsel : 2 ;
        unsigned int rashds_lvt_wtsel : 2 ;
        unsigned int rashdm_svt_rtsel : 2 ;
        unsigned int rashdm_lvt_rtsel : 2 ;
        unsigned int rashdm_svt_wtsel : 2 ;
        unsigned int rashdm_lvt_wtsel : 2 ;
        unsigned int rasshcsu_svt_rtsel : 2 ;
        unsigned int rasshcsu_lvt_rtsel : 2 ;
        unsigned int rasshcsu_svt_wtsel : 2 ;
        unsigned int rasshcsu_lvt_wtsel : 2 ;
        unsigned int rasshcss_svt_rtsel : 2 ;
        unsigned int rasshcss_lvt_rtsel : 2 ;
        unsigned int rasshcss_svt_wtsel : 2 ;
        unsigned int rasshcss_lvt_wtsel : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_SPMEM_CTRL_1;
typedef union {
    struct {
        unsigned int rasshcms_svt_rtsel : 2 ;
        unsigned int rasshcms_lvt_rtsel : 2 ;
        unsigned int rasshcms_svt_wtsel : 2 ;
        unsigned int rasshcms_lvt_wtsel : 2 ;
        unsigned int rasshcmu_svt_rtsel : 2 ;
        unsigned int rasshcmu_lvt_rtsel : 2 ;
        unsigned int rasshcmu_svt_wtsel : 2 ;
        unsigned int rasshcmu_lvt_wtsel : 2 ;
        unsigned int ras_svt_rtsel : 2 ;
        unsigned int ras_lvt_rtsel : 2 ;
        unsigned int ras_svt_wtsel : 2 ;
        unsigned int ras_lvt_wtsel : 2 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_SPMEM_CTRL_2;
typedef union {
    struct {
        unsigned int wide_ynarrow_en : 1 ;
        unsigned int wide_cnarrow_en : 1 ;
        unsigned int rgb_clip_en : 1 ;
        unsigned int mlsb_sel : 1 ;
        unsigned int reserved_0 : 1 ;
        unsigned int reserved_1 : 1 ;
        unsigned int reserved_2 : 1 ;
        unsigned int reserved_3 : 1 ;
        unsigned int reserved_4 : 6 ;
        unsigned int curld_col2gray_en : 1 ;
        unsigned int curld_clip_en : 1 ;
        unsigned int reserved_5 : 4 ;
        unsigned int curld_read_interval : 8 ;
        unsigned int curld_lowdly_en : 1 ;
        unsigned int reserved_6 : 1 ;
        unsigned int reserved_7 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_GCFG;
typedef union {
    struct {
        unsigned int vcpi_osd0_en : 1 ;
        unsigned int vcpi_osd1_en : 1 ;
        unsigned int vcpi_osd2_en : 1 ;
        unsigned int vcpi_osd3_en : 1 ;
        unsigned int vcpi_osd4_en : 1 ;
        unsigned int vcpi_osd5_en : 1 ;
        unsigned int vcpi_osd6_en : 1 ;
        unsigned int vcpi_osd7_en : 1 ;
        unsigned int vcpi_osd0_absqp : 1 ;
        unsigned int vcpi_osd1_absqp : 1 ;
        unsigned int vcpi_osd2_absqp : 1 ;
        unsigned int vcpi_osd3_absqp : 1 ;
        unsigned int vcpi_osd4_absqp : 1 ;
        unsigned int vcpi_osd5_absqp : 1 ;
        unsigned int vcpi_osd6_absqp : 1 ;
        unsigned int vcpi_osd7_absqp : 1 ;
        unsigned int vcpi_roi_osd_sel_0 : 1 ;
        unsigned int vcpi_roi_osd_sel_1 : 1 ;
        unsigned int vcpi_roi_osd_sel_2 : 1 ;
        unsigned int vcpi_roi_osd_sel_3 : 1 ;
        unsigned int vcpi_roi_osd_sel_4 : 1 ;
        unsigned int vcpi_roi_osd_sel_5 : 1 ;
        unsigned int vcpi_roi_osd_sel_6 : 1 ;
        unsigned int vcpi_roi_osd_sel_7 : 1 ;
        unsigned int vcpi_osd_en : 1 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_ENABLE;
typedef union {
    struct {
        unsigned int vcpi_crop_en : 1 ;
        unsigned int vcpi_scale_en : 1 ;
        unsigned int vcpi_store_mode : 1 ;
        unsigned int vcpi_blk_type : 3 ;
        unsigned int vcpi_str_fmt : 4 ;
        unsigned int vcpi_package_sel : 4 ;
        unsigned int vcpi_recst_disable : 1 ;
        unsigned int vcpi_recst_hfbc_raw_en : 1 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_STRFMT;
typedef union {
    struct {
        unsigned int vcpi_intra_cu_en : 4 ;
        unsigned int vcpi_ipcm_en : 1 ;
        unsigned int vcpi_intra_h264_cutdiag : 1 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vcpi_fme_cu_en : 4 ;
        unsigned int vcpi_mrg_cu_en : 4 ;
        unsigned int reserved_1 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_INTRA_INTER_CU_EN;
typedef union {
    struct {
        unsigned int vcpi_cross_slice : 1 ;
        unsigned int vcpi_cross_tile : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_CROSS_TILE_SLC;
typedef union {
    struct {
        unsigned int vcpi_slice_size : 16 ;
        unsigned int vcpi_slcspilt_mod : 1 ;
        unsigned int vcpi_multislc_en : 1 ;
        unsigned int reserved_0 : 14 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_MULTISLC;
typedef union {
    struct {
        unsigned int vctrl_lcu_target_bit : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_LCU_TARGET_BIT;
typedef union {
    struct {
        unsigned int vcpi_sw_l0_width : 7 ;
        unsigned int reserved_0 : 9 ;
        unsigned int vcpi_sw_l0_height : 6 ;
        unsigned int reserved_1 : 10 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_SW_L0_SIZE;
typedef union {
    struct {
        unsigned int vcpi_sw_l1_width : 7 ;
        unsigned int reserved_0 : 9 ;
        unsigned int vcpi_sw_l1_height : 6 ;
        unsigned int reserved_1 : 10 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_SW_L1_SIZE;
typedef union {
    struct {
        unsigned int vcpi_insert_i_slc_idx : 8 ;
        unsigned int vcpi_insert_i_slc_en : 1 ;
        unsigned int reserved_0 : 23 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_I_SLC_INSERT;
typedef union {
    struct {
        unsigned int pme_safe_line : 16 ;
        unsigned int pme_safe_line_val : 1 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SAFE_CFG;
typedef union {
    struct {
        unsigned int pme_iblk_refresh_start_num : 18 ;
        unsigned int reserved_0 : 14 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_IBLK_REFRESH;
typedef union {
    struct {
        unsigned int pme_iblk_refresh_num : 18 ;
        unsigned int reserved_0 : 14 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_IBLK_REFRESH_NUM;
typedef union {
    struct {
        unsigned int intra_cu4_mode0_en : 1 ;
        unsigned int intra_cu4_mode1_en : 1 ;
        unsigned int intra_cu4_mode2_en : 1 ;
        unsigned int intra_cu4_mode3_en : 1 ;
        unsigned int intra_cu4_mode4_en : 1 ;
        unsigned int intra_cu4_mode5_en : 1 ;
        unsigned int intra_cu4_mode6_en : 1 ;
        unsigned int intra_cu4_mode7_en : 1 ;
        unsigned int intra_cu4_mode8_en : 1 ;
        unsigned int intra_cu4_mode9_en : 1 ;
        unsigned int intra_cu4_mode10_en : 1 ;
        unsigned int intra_cu4_mode11_en : 1 ;
        unsigned int intra_cu4_mode12_en : 1 ;
        unsigned int intra_cu4_mode13_en : 1 ;
        unsigned int intra_cu4_mode14_en : 1 ;
        unsigned int intra_cu4_mode15_en : 1 ;
        unsigned int intra_cu4_mode16_en : 1 ;
        unsigned int intra_cu4_mode17_en : 1 ;
        unsigned int intra_cu4_mode18_en : 1 ;
        unsigned int intra_cu4_mode19_en : 1 ;
        unsigned int intra_cu4_mode20_en : 1 ;
        unsigned int intra_cu4_mode21_en : 1 ;
        unsigned int intra_cu4_mode22_en : 1 ;
        unsigned int intra_cu4_mode23_en : 1 ;
        unsigned int intra_cu4_mode24_en : 1 ;
        unsigned int intra_cu4_mode25_en : 1 ;
        unsigned int intra_cu4_mode26_en : 1 ;
        unsigned int intra_cu4_mode27_en : 1 ;
        unsigned int intra_cu4_mode28_en : 1 ;
        unsigned int intra_cu4_mode29_en : 1 ;
        unsigned int intra_cu4_mode30_en : 1 ;
        unsigned int intra_cu4_mode31_en : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL4_ANG_0EN;
typedef union {
    struct {
        unsigned int intra_cu4_mode32_en : 1 ;
        unsigned int intra_cu4_mode33_en : 1 ;
        unsigned int intra_cu4_mode34_en : 1 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL4_ANG_1EN;
typedef union {
    struct {
        unsigned int intra_cu8_mode0_en : 1 ;
        unsigned int intra_cu8_mode1_en : 1 ;
        unsigned int intra_cu8_mode2_en : 1 ;
        unsigned int intra_cu8_mode3_en : 1 ;
        unsigned int intra_cu8_mode4_en : 1 ;
        unsigned int intra_cu8_mode5_en : 1 ;
        unsigned int intra_cu8_mode6_en : 1 ;
        unsigned int intra_cu8_mode7_en : 1 ;
        unsigned int intra_cu8_mode8_en : 1 ;
        unsigned int intra_cu8_mode9_en : 1 ;
        unsigned int intra_cu8_mode10_en : 1 ;
        unsigned int intra_cu8_mode11_en : 1 ;
        unsigned int intra_cu8_mode12_en : 1 ;
        unsigned int intra_cu8_mode13_en : 1 ;
        unsigned int intra_cu8_mode14_en : 1 ;
        unsigned int intra_cu8_mode15_en : 1 ;
        unsigned int intra_cu8_mode16_en : 1 ;
        unsigned int intra_cu8_mode17_en : 1 ;
        unsigned int intra_cu8_mode18_en : 1 ;
        unsigned int intra_cu8_mode19_en : 1 ;
        unsigned int intra_cu8_mode20_en : 1 ;
        unsigned int intra_cu8_mode21_en : 1 ;
        unsigned int intra_cu8_mode22_en : 1 ;
        unsigned int intra_cu8_mode23_en : 1 ;
        unsigned int intra_cu8_mode24_en : 1 ;
        unsigned int intra_cu8_mode25_en : 1 ;
        unsigned int intra_cu8_mode26_en : 1 ;
        unsigned int intra_cu8_mode27_en : 1 ;
        unsigned int intra_cu8_mode28_en : 1 ;
        unsigned int intra_cu8_mode29_en : 1 ;
        unsigned int intra_cu8_mode30_en : 1 ;
        unsigned int intra_cu8_mode31_en : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL8_ANG_0EN;
typedef union {
    struct {
        unsigned int intra_cu8_mode32_en : 1 ;
        unsigned int intra_cu8_mode33_en : 1 ;
        unsigned int intra_cu8_mode34_en : 1 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL8_ANG_1EN;
typedef union {
    struct {
        unsigned int intra_cu16_mode0_en : 1 ;
        unsigned int intra_cu16_mode1_en : 1 ;
        unsigned int intra_cu16_mode2_en : 1 ;
        unsigned int intra_cu16_mode3_en : 1 ;
        unsigned int intra_cu16_mode4_en : 1 ;
        unsigned int intra_cu16_mode5_en : 1 ;
        unsigned int intra_cu16_mode6_en : 1 ;
        unsigned int intra_cu16_mode7_en : 1 ;
        unsigned int intra_cu16_mode8_en : 1 ;
        unsigned int intra_cu16_mode9_en : 1 ;
        unsigned int intra_cu16_mode10_en : 1 ;
        unsigned int intra_cu16_mode11_en : 1 ;
        unsigned int intra_cu16_mode12_en : 1 ;
        unsigned int intra_cu16_mode13_en : 1 ;
        unsigned int intra_cu16_mode14_en : 1 ;
        unsigned int intra_cu16_mode15_en : 1 ;
        unsigned int intra_cu16_mode16_en : 1 ;
        unsigned int intra_cu16_mode17_en : 1 ;
        unsigned int intra_cu16_mode18_en : 1 ;
        unsigned int intra_cu16_mode19_en : 1 ;
        unsigned int intra_cu16_mode20_en : 1 ;
        unsigned int intra_cu16_mode21_en : 1 ;
        unsigned int intra_cu16_mode22_en : 1 ;
        unsigned int intra_cu16_mode23_en : 1 ;
        unsigned int intra_cu16_mode24_en : 1 ;
        unsigned int intra_cu16_mode25_en : 1 ;
        unsigned int intra_cu16_mode26_en : 1 ;
        unsigned int intra_cu16_mode27_en : 1 ;
        unsigned int intra_cu16_mode28_en : 1 ;
        unsigned int intra_cu16_mode29_en : 1 ;
        unsigned int intra_cu16_mode30_en : 1 ;
        unsigned int intra_cu16_mode31_en : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL16_ANG_0EN;
typedef union {
    struct {
        unsigned int intra_cu16_mode32_en : 1 ;
        unsigned int intra_cu16_mode33_en : 1 ;
        unsigned int intra_cu16_mode34_en : 1 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL16_ANG_1EN;
typedef union {
    struct {
        unsigned int intra_cu32_mode0_en : 1 ;
        unsigned int intra_cu32_mode1_en : 1 ;
        unsigned int intra_cu32_mode2_en : 1 ;
        unsigned int intra_cu32_mode3_en : 1 ;
        unsigned int intra_cu32_mode4_en : 1 ;
        unsigned int intra_cu32_mode5_en : 1 ;
        unsigned int intra_cu32_mode6_en : 1 ;
        unsigned int intra_cu32_mode7_en : 1 ;
        unsigned int intra_cu32_mode8_en : 1 ;
        unsigned int intra_cu32_mode9_en : 1 ;
        unsigned int intra_cu32_mode10_en : 1 ;
        unsigned int intra_cu32_mode11_en : 1 ;
        unsigned int intra_cu32_mode12_en : 1 ;
        unsigned int intra_cu32_mode13_en : 1 ;
        unsigned int intra_cu32_mode14_en : 1 ;
        unsigned int intra_cu32_mode15_en : 1 ;
        unsigned int intra_cu32_mode16_en : 1 ;
        unsigned int intra_cu32_mode17_en : 1 ;
        unsigned int intra_cu32_mode18_en : 1 ;
        unsigned int intra_cu32_mode19_en : 1 ;
        unsigned int intra_cu32_mode20_en : 1 ;
        unsigned int intra_cu32_mode21_en : 1 ;
        unsigned int intra_cu32_mode22_en : 1 ;
        unsigned int intra_cu32_mode23_en : 1 ;
        unsigned int intra_cu32_mode24_en : 1 ;
        unsigned int intra_cu32_mode25_en : 1 ;
        unsigned int intra_cu32_mode26_en : 1 ;
        unsigned int intra_cu32_mode27_en : 1 ;
        unsigned int intra_cu32_mode28_en : 1 ;
        unsigned int intra_cu32_mode29_en : 1 ;
        unsigned int intra_cu32_mode30_en : 1 ;
        unsigned int intra_cu32_mode31_en : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL32_ANG_0EN;
typedef union {
    struct {
        unsigned int intra_cu32_mode32_en : 1 ;
        unsigned int intra_cu32_mode33_en : 1 ;
        unsigned int intra_cu32_mode34_en : 1 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CHNL32_ANG_1EN;
typedef union {
    struct {
        unsigned int pack_vcpi2cu_tq_bypass_enabled_flag : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int pack_vcpi2cu_qp_min_cu_size : 3 ;
        unsigned int reserved_1 : 25 ;
    } bits;
    unsigned int u32;
} U_VEDU_PACK_CU_PARAMETER;
typedef union {
    struct {
        unsigned int pack_vcpi2pu_log2_min_ipcm_cbsizey : 3 ;
        unsigned int reserved_0 : 1 ;
        unsigned int pack_vcpi2pu_log2_max_ipcm_cbsizey : 3 ;
        unsigned int reserved_1 : 25 ;
    } bits;
    unsigned int u32;
} U_VEDU_PACK_PCM_PARAMETER;
typedef union {
    struct {
        unsigned int vcpi_tile_qpg_readline_interval : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_READLINE_INTERVAL;
typedef union {
    struct {
        unsigned int vcpi_pmv_readline_interval : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_PMV_READLINE_INTERVAL;
typedef union {
    struct {
        unsigned int rgb2yuv_y_0coef : 15 ;
        unsigned int reserved_0 : 1 ;
        unsigned int rgb2yuv_y_1coef : 15 ;
        unsigned int reserved_1 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_COEF_P0;
typedef union {
    struct {
        unsigned int rgb2yuv_y_2coef : 15 ;
        unsigned int reserved_0 : 1 ;
        unsigned int rgb2yuv_u_0coef : 15 ;
        unsigned int reserved_1 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_COEF_P1;
typedef union {
    struct {
        unsigned int rgb2yuv_u_1coef : 15 ;
        unsigned int reserved_0 : 1 ;
        unsigned int rgb2yuv_u_2coef : 15 ;
        unsigned int reserved_1 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_COEF_P2;
typedef union {
    struct {
        unsigned int rgb_y_rnd : 16 ;
        unsigned int rgb_u_rnd : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_OFFSET;
typedef union {
    struct {
        unsigned int rgb2yuv_clip_min_y : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int rgb2yuv_clip_max_y : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_CLIP_THR_Y;
typedef union {
    struct {
        unsigned int rgb2yuv_clip_min_c : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int rgb2yuv_clip_max_c : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_CLIP_THR_C;
typedef union {
    struct {
        unsigned int rgb2yuv_shift_width : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_RGB2YUV_SHIFT_WIDTH;
typedef union {
    struct {
        unsigned int rgb2yuv_v_0coef : 15 ;
        unsigned int reserved_0 : 1 ;
        unsigned int rgb2yuv_v_1coef : 15 ;
        unsigned int reserved_1 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_POS_0;
typedef union {
    struct {
        unsigned int rgb2yuv_v_2coef : 15 ;
        unsigned int reserved_0 : 1 ;
        unsigned int rgb_v_rnd : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_POS_1;
typedef union
{
    struct
    {
        unsigned int curld_narrow_chrm_min : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int curld_narrow_chrm_max : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_POS_4;
typedef union
{
    struct
    {
        unsigned int curld_narrow_luma_min : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int curld_narrow_luma_max : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_POS_5;
typedef union {
    struct {
        unsigned int vcpi_osd0_qp : 7 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_osd1_qp : 7 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vcpi_osd2_qp : 7 ;
        unsigned int reserved_2 : 1 ;
        unsigned int vcpi_osd3_qp : 7 ;
        unsigned int reserved_3 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_QP0;
typedef union {
    struct {
        unsigned int vcpi_osd4_qp : 7 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_osd5_qp : 7 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vcpi_osd6_qp : 7 ;
        unsigned int reserved_2 : 1 ;
        unsigned int vcpi_osd7_qp : 7 ;
        unsigned int reserved_3 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_OSD_QP1;
typedef union {
    struct {
        unsigned int rgb_clip_min : 10 ;
        unsigned int rgb_clip_max : 10 ;
        unsigned int curld_hfbcd_clk_gt_en : 1 ;
        unsigned int curld_hfbcd_bypass_en : 1 ;
        unsigned int curld_hfbcd_raw_en : 1 ;
        unsigned int reserved_0 : 9 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_OSD01_ALPHA;
typedef union {
    struct {
        unsigned int vcpi_filter_hor_0coef : 4 ;
        unsigned int vcpi_filter_hor_1coef : 4 ;
        unsigned int vcpi_filter_hor_2coef : 4 ;
        unsigned int vcpi_filter_hor_3coef : 4 ;
        unsigned int vcpi_filter_hor_rnd : 4 ;
        unsigned int vcpi_filter_hor_shift : 3 ;
        unsigned int reserved_0 : 9 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_OSD23_ALPHA;
typedef union {
    struct {
        unsigned int vcpi_filter_ver_0coef : 4 ;
        unsigned int vcpi_filter_ver_1coef : 4 ;
        unsigned int vcpi_filter_ver_2coef : 4 ;
        unsigned int vcpi_filter_ver_3coef : 4 ;
        unsigned int vcpi_filter_ver_rnd : 4 ;
        unsigned int vcpi_filter_ver_shift : 3 ;
        unsigned int reserved_0 : 9 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_OSD45_ALPHA;
typedef union {
    struct {
        unsigned int srcyh_stride : 16 ;
        unsigned int srcch_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_OSD67_ALPHA;
typedef union {
    struct {
        unsigned int vcpi_tbldst_readline_interval : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_TBLDST_READLINE_INTERVAL;
typedef union {
    struct {
        unsigned int irq_position_cnt_y : 10 ;
        unsigned int reserved_0 : 7 ;
        unsigned int reserved_1 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_NEW_CFG0;
typedef union {
    struct {
        unsigned int vctrl_region0en : 1 ;
        unsigned int vctrl_region1en : 1 ;
        unsigned int vctrl_region2en : 1 ;
        unsigned int vctrl_region3en : 1 ;
        unsigned int vctrl_region4en : 1 ;
        unsigned int vctrl_region5en : 1 ;
        unsigned int vctrl_region6en : 1 ;
        unsigned int vctrl_region7en : 1 ;
        unsigned int vctrl_absqp0 : 1 ;
        unsigned int vctrl_absqp1 : 1 ;
        unsigned int vctrl_absqp2 : 1 ;
        unsigned int vctrl_absqp3 : 1 ;
        unsigned int vctrl_absqp4 : 1 ;
        unsigned int vctrl_absqp5 : 1 ;
        unsigned int vctrl_absqp6 : 1 ;
        unsigned int vctrl_absqp7 : 1 ;
        unsigned int vctrl_region0keep : 1 ;
        unsigned int vctrl_region1keep : 1 ;
        unsigned int vctrl_region2keep : 1 ;
        unsigned int vctrl_region3keep : 1 ;
        unsigned int vctrl_region4keep : 1 ;
        unsigned int vctrl_region5keep : 1 ;
        unsigned int vctrl_region6keep : 1 ;
        unsigned int vctrl_region7keep : 1 ;
        unsigned int vctrl_roi_en : 1 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_CFG0;
typedef union {
    struct {
        unsigned int vctrl_roiqp0 : 7 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vctrl_roiqp1 : 7 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vctrl_roiqp2 : 7 ;
        unsigned int reserved_2 : 1 ;
        unsigned int vctrl_roiqp3 : 7 ;
        unsigned int reserved_3 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_CFG1;
typedef union {
    struct {
        unsigned int vctrl_roiqp4 : 7 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vctrl_roiqp5 : 7 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vctrl_roiqp6 : 7 ;
        unsigned int reserved_2 : 1 ;
        unsigned int vctrl_roiqp7 : 7 ;
        unsigned int reserved_3 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_CFG2;
typedef union {
    struct {
        unsigned int vctrl_size0_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size0_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_0;
typedef union {
    struct {
        unsigned int vctrl_size1_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size1_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_1;
typedef union {
    struct {
        unsigned int vctrl_size2_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size2_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_2;
typedef union {
    struct {
        unsigned int vctrl_size3_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size3_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_3;
typedef union {
    struct {
        unsigned int vctrl_size4_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size4_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_4;
typedef union {
    struct {
        unsigned int vctrl_size5_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size5_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_5;
typedef union {
    struct {
        unsigned int vctrl_size6_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size6_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_6;
typedef union {
    struct {
        unsigned int vctrl_size7_roiwidth : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vctrl_size7_roiheight : 13 ;
        unsigned int reserved_1 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_SIZE_7;
typedef union {
    struct {
        unsigned int vctrl_start0_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start0_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_0;
typedef union {
    struct {
        unsigned int vctrl_start1_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start1_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_1;
typedef union {
    struct {
        unsigned int vctrl_start2_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start2_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_2;
typedef union {
    struct {
        unsigned int vctrl_start3_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start3_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_3;
typedef union {
    struct {
        unsigned int vctrl_start4_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start4_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_4;
typedef union {
    struct {
        unsigned int vctrl_start5_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start5_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_5;
typedef union {
    struct {
        unsigned int vctrl_start6_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start6_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_6;
typedef union {
    struct {
        unsigned int vctrl_start7_roistartx : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vctrl_start7_roistarty : 12 ;
        unsigned int reserved_1 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ROI_START_7;
typedef union {
    struct {
        unsigned int vedu_curr_cell_mode : 1 ;
        unsigned int vedu_left_cell_mode : 1 ;
        unsigned int vedu_top_cell_mode : 1 ;
        unsigned int vedu_topleft_cell_mode : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_TILE_MODE;
typedef union {
    struct {
        unsigned int vcpi_imgwidth_pix : 16 ;
        unsigned int vcpi_imgheight_pix : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PICSIZE_PIX;
typedef union {
    struct {
        unsigned int vedu_tile_start_x : 16 ;
        unsigned int vedu_tile_start_y : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_TILE_POS;
typedef union {
    struct {
        unsigned int vcpi_tile_width : 16 ;
        unsigned int vcpi_tile_height : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_TILE_SIZE;
typedef union {
    struct {
        unsigned int vcpi_ref_idc : 2 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vcpi_cabac_init_idc : 2 ;
        unsigned int reserved_1 : 6 ;
        unsigned int vcpi_byte_stuffing : 1 ;
        unsigned int reserved_2 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_VLC_CONFIG;
typedef union {
    struct {
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_col_from_l0_flag : 1 ;
        unsigned int vcpi_curr_ref_long_flag : 2 ;
        unsigned int vcpi_col_long_flag : 2 ;
        unsigned int vcpi_predflag_sel : 2 ;
        unsigned int reserved_1 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_REF_FLAG;
typedef union {
    struct {
        unsigned int pmv_tmv_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_PMV_TMV_EN;
typedef union {
    struct {
        unsigned int vcpi_tmv_wr_rd_avail : 2 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_TMV_LOAD;
typedef union {
    struct {
        unsigned int reserved_0 : 8 ;
        unsigned int cabac_max_num_mergecand : 3 ;
        unsigned int reserved_1 : 5 ;
        unsigned int cabac_nal_unit_head : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_CABAC_GLB_CFG;
typedef union {
    struct {
        unsigned int cabac_slchdr_size_part1 : 5 ;
        unsigned int reserved_0 : 11 ;
        unsigned int cabac_slchdr_size_part2 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_CABAC_SLCHDR_SIZE;
typedef union {
    struct {
        unsigned int cabac_slchdr_part1 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_CABAC_SLCHDR_PART1;
typedef union {
    struct {
        unsigned int cabac_slchdr_size_part1_i : 5 ;
        unsigned int reserved_0 : 11 ;
        unsigned int cabac_slchdr_size_part2_i : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_CABAC_SLCHDR_SIZE_I;
typedef union {
    struct {
        unsigned int cabac_slchdr_part1_i : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_CABAC_SLCHDR_PART1_I;
typedef union {
    struct {
        unsigned int vlc_markingbit : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vlc_reorderbit : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int vlc_parabit : 7 ;
        unsigned int reserved_2 : 9 ;
    } bits;
    unsigned int u32;
} U_VEDU_VLC_SLCHDRPARA;
typedef union {
    struct {
        unsigned int vlc_svc_en : 1 ;
        unsigned int vlc_svc_strm : 24 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_VLC_SVC;
typedef union {
    struct {
        unsigned int vlc_markingbit_i : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vlc_reorderbit_i : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int vlc_parabit_i : 7 ;
        unsigned int reserved_2 : 9 ;
    } bits;
    unsigned int u32;
} U_VEDU_VLC_SLCHDRPARA_I;
typedef union {
    struct {
        unsigned int vcfg_hfbc_clkgate_en : 1 ;
        unsigned int reserved_0 : 1 ;
        unsigned int rfs_emaw : 2 ;
        unsigned int rfs_ema : 3 ;
        unsigned int rft_colldisn : 1 ;
        unsigned int rft_emaa : 3 ;
        unsigned int rft_emab : 3 ;
        unsigned int rft_emasa : 1 ;
        unsigned int rfsuhd_wtsel : 2 ;
        unsigned int rfsuhd_rtsel : 2 ;
        unsigned int rfs_wtsel : 2 ;
        unsigned int rfs_rtsel : 2 ;
        unsigned int rfts_wct : 2 ;
        unsigned int rfts_rct : 2 ;
        unsigned int rfts_kp : 3 ;
        unsigned int rftf_wct : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_ICE_CMC_MODE_CFG0;
typedef union {
    struct {
        unsigned int rftf_rct : 2 ;
        unsigned int rftf_kp : 3 ;
        unsigned int rfthd_wtsel : 2 ;
        unsigned int rfthd_mtsel : 2 ;
        unsigned int rfthd_rtsel : 2 ;
        unsigned int rasshds_wtsel : 2 ;
        unsigned int rasshds_rtsel : 2 ;
        unsigned int rasshdm_wtsel : 2 ;
        unsigned int rasshdm_rtsel : 2 ;
        unsigned int rashds_wtsel : 2 ;
        unsigned int rashds_rtsel : 2 ;
        unsigned int rashdm_wtsel : 2 ;
        unsigned int rashdm_rtsel : 2 ;
        unsigned int ras_wtsel : 2 ;
        unsigned int ras_rtsel : 2 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_ICE_CMC_MODE_CFG1;
typedef union {
    struct {
        unsigned int qpg_lowluma_min_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_lowluma_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_lowluma_qp_delta : 4 ;
        unsigned int reserved_2 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_LOWLUMA;
typedef union {
    struct {
        unsigned int qpg_hedge_mim_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_hedge_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_hedge_qp_delta : 4 ;
        unsigned int reserved_2 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_HEDGE;
typedef union {
    struct {
        unsigned int qpg_hedge_move_min_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_hedge_move_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_hedge_move_qp_delta : 4 ;
        unsigned int reserved_2 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_HEDGE_MOVE;
typedef union {
    struct {
        unsigned int qpg_skin_min_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_skin_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_skin_qp_delta : 4 ;
        unsigned int reserved_2 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_SKIN;
typedef union {
    struct {
        unsigned int qpg_intra_det_min_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_intra_det_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_intra_det_qp_delta : 4 ;
        unsigned int reserved_2 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_INTRA_DET;
typedef union {
    struct {
        unsigned int qpg_cu32_delta_low : 4 ;
        unsigned int qpg_cu32_delta_high : 4 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CU32_DELTA;
typedef union {
    struct {
        unsigned int qpg_lambda_qp_offset : 5 ;
        unsigned int reserved_0 : 3 ;
        unsigned int qpg_rdo_lambda_choose_mode : 2 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_lambda_inter_stredge_en : 1 ;
        unsigned int reserved_2 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_LAMBDA_MODE;
typedef union {
    struct {
        unsigned int qpg_qp_restrain_madi_thr : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_qp_restrain_en : 1 ;
        unsigned int qpg_qp_restrain_mode : 2 ;
        unsigned int reserved_1 : 1 ;
        unsigned int qpg_qp_restrain_delta_blk16 : 4 ;
        unsigned int qpg_qp_restrain_delta_blk32 : 4 ;
        unsigned int reserved_2 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_RESTRAIN;
typedef union {
    struct {
        unsigned int qpg_min_sad_level : 8 ;
        unsigned int reserved_0 : 8 ;
        unsigned int qpg_low_min_sad_mode : 2 ;
        unsigned int reserved_1 : 2 ;
        unsigned int qpg_min_sad_madi_en : 1 ;
        unsigned int qpg_min_sad_qp_restrain_en : 1 ;
        unsigned int reserved_2 : 10 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_CU_MIN_SAD_REG;
typedef union {
    struct {
        unsigned int qpg_flat_region_qp_delta : 4 ;
        unsigned int qpg_flat_region_max_qp : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int qpg_flat_region_min_qp : 6 ;
        unsigned int reserved_1 : 6 ;
        unsigned int vcpi_cu32_use_cu16_mean_en : 1 ;
        unsigned int reserved_2 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_FLAT_REGION;
typedef union {
    struct {
        unsigned int vcpi_res_coef_en : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_large_res_coef : 5 ;
        unsigned int reserved_1 : 3 ;
        unsigned int vcpi_small_res_coef : 5 ;
        unsigned int reserved_2 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_RES_COEF;
typedef union {
    struct {
        unsigned int vcpi_tile_level_rc_en : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_tile_line_qp_delta : 4 ;
        unsigned int vcpi_tile_bound_qp_delta : 4 ;
        unsigned int reserved_1 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_TILE_RC;
typedef union {
    struct {
        unsigned int vcpi_chroma_prot_en : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_chroma_max_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int vcpi_chroma_in_qp : 6 ;
        unsigned int reserved_2 : 2 ;
        unsigned int vcpi_chroma_qp_delta : 4 ;
        unsigned int reserved_3 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_PROTECT;
typedef union {
    struct {
        unsigned int reserved_0 : 16 ;
        unsigned int pme_madi_dif_thr : 7 ;
        unsigned int pme_cur_madi_dif_thr : 7 ;
        unsigned int reserved_1 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_QPG_RC_THR0;
typedef union {
    struct {
        unsigned int pme_min_sad_thr_offset : 8 ;
        unsigned int pme_min_sad_thr_gain : 4 ;
        unsigned int reserved_0 : 8 ;
        unsigned int pme_min_sad_thr_offset_cur : 8 ;
        unsigned int pme_min_sad_thr_gain_cur : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_QPG_RC_THR1;
typedef union {
    struct {
        unsigned int pme_low_luma_thr : 8 ;
        unsigned int pme_low_luma_madi_thr : 8 ;
        unsigned int pme_high_luma_thr : 8 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_LOW_LUMA_THR;
typedef union {
    struct {
        unsigned int pme_flat_v_thr_high : 8 ;
        unsigned int pme_flat_v_thr_low : 8 ;
        unsigned int pme_flat_u_thr_high : 8 ;
        unsigned int pme_flat_u_thr_low : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_CHROMA_FLAT;
typedef union {
    struct {
        unsigned int pme_flat_pmemv_thr : 8 ;
        unsigned int pme_flat_luma_madi_thr : 8 ;
        unsigned int pme_flat_low_luma_thr : 8 ;
        unsigned int pme_flat_high_luma_thr : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_LUMA_FLAT;
typedef union {
    struct {
        unsigned int pme_flat_pmesad_thr : 14 ;
        unsigned int pme_flat_icount_thr : 9 ;
        unsigned int pme_flat_region_cnt : 5 ;
        unsigned int pme_flat_madi_times : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_MADI_FLAT;
typedef union {
    struct {
        unsigned int vlcst_idrind : 1 ;
        unsigned int reserved_0 : 15 ;
        unsigned int vlcst_chnid : 8 ;
        unsigned int reserved_1 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_VLCST_DESCRIPTOR;
typedef union {
    struct {
        unsigned int curld_clip_luma_min : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int curld_clip_luma_max : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_CLIP_LUMA;
typedef union {
    struct {
        unsigned int curld_clip_chroma_min : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int curld_clip_chroma_max : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_CURLD_CLIP_CHROMA;
typedef union {
    struct {
        unsigned int tqitq_deadzone_intra_slice : 8 ;
        unsigned int tqitq_deadzone_inter_slice : 8 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_TQITQ_DEADZONE;
typedef union {
    struct {
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_move_sad_en : 1 ;
        unsigned int reserved_1 : 14 ;
        unsigned int vcpi_pblk_pre_mvx_thr : 8 ;
        unsigned int vcpi_pblk_pre_mvy_thr : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PME_PARAM;
typedef union {
    struct {
        unsigned int vcpi_skin_en : 1 ;
        unsigned int vcpi_strong_edge_en : 1 ;
        unsigned int vcpi_still_en : 1 ;
        unsigned int vcpi_skin_close_angle : 1 ;
        unsigned int vcpi_rounding_sobel_en : 1 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PIC_STRONG_EN;
typedef union {
    struct {
        unsigned int vcpi_pintra_pu16_amp_th : 8 ;
        unsigned int vcpi_pintra_pu32_amp_th : 8 ;
        unsigned int vcpi_pintra_pu64_amp_th : 8 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PINTRA_THRESH0;
typedef union {
    struct {
        unsigned int vcpi_pintra_pu16_std_th : 8 ;
        unsigned int vcpi_pintra_pu32_std_th : 8 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PINTRA_THRESH1;
typedef union {
    struct {
        unsigned int vcpi_pintra_pu16_angel_cost_th : 10 ;
        unsigned int vcpi_pintra_pu32_angel_cost_th : 10 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vcpi_rpintra_pu4_strong_edge_th : 3 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vcpi_rpintra_pu4_mode_distance_th : 2 ;
        unsigned int vcpi_rpintra_bypass : 1 ;
        unsigned int reserved_2 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PINTRA_THRESH2;
typedef union {
    struct {
        unsigned int vcpi_intra32_low_power_thr : 16 ;
        unsigned int vcpi_intra32_low_power_en : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_intra32_low_power_gain : 6 ;
        unsigned int vcpi_intra32_low_power_offset : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_INTRA32_LOW_POWER;
typedef union {
    struct {
        unsigned int vcpi_intra16_low_power_thr : 16 ;
        unsigned int vcpi_intra16_low_power_en : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_intra16_low_power_gain : 6 ;
        unsigned int vcpi_intra16_low_power_offset : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_INTRA16_LOW_POWER;
typedef union {
    struct {
        unsigned int vcpi_intra_reduce_rdo_num_thr : 12 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vcpi_intra_reduce_rdo_num_en : 1 ;
        unsigned int reserved_1 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM;
typedef union {
    struct {
        unsigned int vcpi_bislayer0flag : 1 ;
        unsigned int vcpi_bnoforcezero_flag : 1 ;
        unsigned int vcpi_bnoforcezero_posx : 4 ;
        unsigned int vcpi_bnoforcezero_posy : 4 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_NOFORCEZERO;
typedef union {
    struct {
        unsigned int pme_skip_sad_thr_offset : 8 ;
        unsigned int pme_skip_sad_thr_gain : 4 ;
        unsigned int pme_skip_large_res_det : 1 ;
        unsigned int reserved_0 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SKIP_LARGE_RES;
typedef union {
    struct {
        unsigned int vcpi_skin_sad_thr_offset : 8 ;
        unsigned int reserved_0 : 4 ;
        unsigned int vcpi_skin_sad_thr_gain : 4 ;
        unsigned int reserved_1 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SKIN_SAD_THR;
typedef union {
    struct {
        unsigned int vctrl_ChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_ChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_IChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_LowFreqACBlk16 : 4 ;
        unsigned int vctrl_ChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_IChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_LowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_NM_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_ChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_ChosOthBlkEngThr : 4 ;
        unsigned int vctrl_ChosOthBlkThr3 : 6 ;
        unsigned int vctrl_ChosOthBlkThr2 : 6 ;
        unsigned int vctrl_ChosOthBlkThr : 5 ;
        unsigned int vctrl_RmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_NM_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_RingEngThr : 4 ;
        unsigned int vctrl_RingACThr : 4 ;
        unsigned int vctrl_PChosOthBlkOffset : 4 ;
        unsigned int vctrl_PChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_IChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_LowFreqACBlk8 : 3 ;
        unsigned int vctrl_Blk8EnableFlag : 1 ;
        unsigned int vctrl_ChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_ChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_NM_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_SkinChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_SkinChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_SkinIChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_SkinLowFreqACBlk16 : 4 ;
        unsigned int vctrl_SkinChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_SkinIChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_SkinLowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SK_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_SkinChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_SkinChosOthBlkEngThr : 4 ;
        unsigned int vctrl_SkinChosOthBlkThr3 : 6 ;
        unsigned int vctrl_SkinChosOthBlkThr2 : 6 ;
        unsigned int vctrl_SkinChosOthBlkThr : 5 ;
        unsigned int vctrl_SkinRmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SK_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_SkinRingEngThr : 4 ;
        unsigned int vctrl_SkinRingACThr : 4 ;
        unsigned int vctrl_SkinPChosOthBlkOffset : 4 ;
        unsigned int vctrl_SkinPChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_SkinIChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_SkinLowFreqACBlk8 : 3 ;
        unsigned int vctrl_SkinBlk8EnableFlag : 1 ;
        unsigned int vctrl_SkinChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_SkinChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SK_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_StillChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_StillChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_StillIChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_StillLowFreqACBlk16 : 4 ;
        unsigned int vctrl_StillChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_StillIChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_StillLowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ST_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_StillChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_StillChosOthBlkEngThr : 4 ;
        unsigned int vctrl_StillChosOthBlkThr3 : 6 ;
        unsigned int vctrl_StillChosOthBlkThr2 : 6 ;
        unsigned int vctrl_StillChosOthBlkThr : 5 ;
        unsigned int vctrl_StillRmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ST_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_StillRingEngThr : 4 ;
        unsigned int vctrl_StillRingACThr : 4 ;
        unsigned int vctrl_StillPChosOthBlkOffset : 4 ;
        unsigned int vctrl_StillPChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_StillIChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_StillLowFreqACBlk8 : 3 ;
        unsigned int vctrl_StillBlk8EnableFlag : 1 ;
        unsigned int vctrl_StillChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_StillChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_ST_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_EdgeChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_EdgeChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_EdgeIChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_EdgeLowFreqACBlk16 : 4 ;
        unsigned int vctrl_EdgeChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_EdgeIChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_EdgeLowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SE_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_EdgeChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_EdgeChosOthBlkEngThr : 4 ;
        unsigned int vctrl_EdgeChosOthBlkThr3 : 6 ;
        unsigned int vctrl_EdgeChosOthBlkThr2 : 6 ;
        unsigned int vctrl_EdgeChosOthBlkThr : 5 ;
        unsigned int vctrl_EdgeRmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SE_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_EdgeRingEngThr : 4 ;
        unsigned int vctrl_EdgeRingACThr : 4 ;
        unsigned int vctrl_EdgePChosOthBlkOffset : 4 ;
        unsigned int vctrl_EdgePChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_EdgeIChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_EdgeLowFreqACBlk8 : 3 ;
        unsigned int vctrl_EdgeBlk8EnableFlag : 1 ;
        unsigned int vctrl_EdgeChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_EdgeChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SE_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_EdgeAndMoveChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_EdgeAndMoveIChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_EdgeAndMoveLowFreqACBlk16 : 4 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_EdgeAndMoveIChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_EdgeAndMoveLowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SELM_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_EdgeAndMoveChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkEngThr : 4 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkThr3 : 6 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkThr2 : 6 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkThr : 5 ;
        unsigned int vctrl_EdgeAndMoveRmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SELM_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_EdgeAndMoveRingEngThr : 4 ;
        unsigned int vctrl_EdgeAndMoveRingACThr : 4 ;
        unsigned int vctrl_EdgeAndMovePChosOthBlkOffset : 4 ;
        unsigned int vctrl_EdgeAndMovePChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_EdgeAndMoveIChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_EdgeAndMoveLowFreqACBlk8 : 3 ;
        unsigned int vctrl_EdgeAndMoveBlk8EnableFlag : 1 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_EdgeAndMoveChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SELM_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_WeakChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_WeakChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_WeakIChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_WeakLowFreqACBlk16 : 4 ;
        unsigned int vctrl_WeakChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_WeakIChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_WeakLowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_WS_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_WeakChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_WeakChosOthBlkEngThr : 4 ;
        unsigned int vctrl_WeakChosOthBlkThr3 : 6 ;
        unsigned int vctrl_WeakChosOthBlkThr2 : 6 ;
        unsigned int vctrl_WeakChosOthBlkThr : 5 ;
        unsigned int vctrl_WeakRmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_WS_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_WeakRingEngThr : 4 ;
        unsigned int vctrl_WeakRingACThr : 4 ;
        unsigned int vctrl_WeakPChosOthBlkOffset : 4 ;
        unsigned int vctrl_WeakPChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_WeakIChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_WeakLowFreqACBlk8 : 3 ;
        unsigned int vctrl_WeakBlk8EnableFlag : 1 ;
        unsigned int vctrl_WeakChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_WeakChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_WS_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_StrongChosOthBlkSpecOffset : 4 ;
        unsigned int vctrl_StrongChosOthBlkOffset16 : 4 ;
        unsigned int vctrl_StrongIChosCurBlkOffset16 : 4 ;
        unsigned int vctrl_StrongLowFreqACBlk16 : 4 ;
        unsigned int vctrl_StrongChosOthBlkOffset32 : 4 ;
        unsigned int vctrl_StrongIChosCurBlkOffset32 : 4 ;
        unsigned int vctrl_StrongLowFreqACBlk32 : 4 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SSSE_ACOFFSET_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_StrongChosOthBlkSpecThr : 4 ;
        unsigned int vctrl_StrongChosOthBlkEngThr : 4 ;
        unsigned int vctrl_StrongChosOthBlkThr3 : 6 ;
        unsigned int vctrl_StrongChosOthBlkThr2 : 6 ;
        unsigned int vctrl_StrongChosOthBlkThr : 5 ;
        unsigned int vctrl_StrongRmAllHighACThr : 4 ;
        unsigned int reserved_0 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SSSE_ENGTHR_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_StrongRingEngThr : 4 ;
        unsigned int vctrl_StrongRingACThr : 4 ;
        unsigned int vctrl_StrongPChosOthBlkOffset : 4 ;
        unsigned int vctrl_StrongPChosOthBlkOffset8 : 4 ;
        unsigned int vctrl_StrongIChosCurBlkOffset8 : 4 ;
        unsigned int vctrl_StrongLowFreqACBlk8 : 3 ;
        unsigned int vctrl_StrongBlk8EnableFlag : 1 ;
        unsigned int vctrl_StrongChosOthBlkOffsetSec16 : 4 ;
        unsigned int vctrl_StrongChosOthBlkOffsetSec32 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_SSSE_TU8_DENOISE;
typedef union {
    struct {
        unsigned int vctrl_norm_intra_cu4_rdcost_offset : 4 ;
        unsigned int vctrl_norm_intra_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_norm_intra_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_norm_intra_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_intra_cu4_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_intra_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_intra_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_intra_cu32_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_INTRA_RDO_FACTOR_0;
typedef union {
    struct {
        unsigned int vctrl_skin_intra_cu4_rdcost_offset : 4 ;
        unsigned int vctrl_skin_intra_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_skin_intra_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_skin_intra_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_intra_cu4_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_intra_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_intra_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_intra_cu32_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_INTRA_RDO_FACTOR_1;
typedef union {
    struct {
        unsigned int vctrl_hedge_intra_cu4_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_intra_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_intra_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_intra_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_intra_cu4_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_intra_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_intra_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_intra_cu32_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_INTRA_RDO_FACTOR_2;
typedef union {
    struct {
        unsigned int vctrl_norm_mrg_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_norm_mrg_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_norm_mrg_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_norm_mrg_cu64_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_mrg_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_mrg_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_mrg_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_mrg_cu64_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_MRG_RDO_FACTOR_0;
typedef union {
    struct {
        unsigned int vctrl_skin_mrg_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_skin_mrg_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_skin_mrg_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_skin_mrg_cu64_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_mrg_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_mrg_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_mrg_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_mrg_cu64_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_MRG_RDO_FACTOR_1;
typedef union {
    struct {
        unsigned int vctrl_hedge_mrg_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_mrg_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_mrg_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_mrg_cu64_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_mrg_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_mrg_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_mrg_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_mrg_cu64_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_MRG_RDO_FACTOR_2;
typedef union {
    struct {
        unsigned int vctrl_norm_fme_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_norm_fme_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_norm_fme_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_norm_fme_cu64_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_fme_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_fme_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_fme_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_strmov_fme_cu64_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_FME_RDO_FACTOR_0;
typedef union {
    struct {
        unsigned int vctrl_skin_fme_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_skin_fme_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_skin_fme_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_skin_fme_cu64_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_fme_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_fme_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_fme_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_str_fme_cu64_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_FME_RDO_FACTOR_1;
typedef union {
    struct {
        unsigned int vctrl_hedge_fme_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_fme_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_fme_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_hedge_fme_cu64_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_fme_cu8_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_fme_cu16_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_fme_cu32_rdcost_offset : 4 ;
        unsigned int vctrl_sobel_tex_fme_cu64_rdcost_offset : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCTRL_FME_RDO_FACTOR_2;
typedef union {
    struct {
        unsigned int pme_cost_lamda0 : 4 ;
        unsigned int pme_cost_lamda1 : 4 ;
        unsigned int pme_cost_lamda2 : 4 ;
        unsigned int pme_new_cost_en : 2 ;
        unsigned int reserved_0 : 2 ;
        unsigned int pme_cost_lamda_en : 2 ;
        unsigned int pme_mvp3median_en : 1 ;
        unsigned int reserved_1 : 13 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_NEW_COST;
typedef union {
    struct {
        unsigned int pme_l0_cost_offset : 16 ;
        unsigned int pme_l1_cost_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_COST_OFFSET;
typedef union {
    struct {
        unsigned int pme_adjust_pmemv_dist_times : 8 ;
        unsigned int pme_adjust_pmemv_en : 1 ;
        unsigned int reserved_0 : 23 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_ADJUST_PMEMV_H264;
typedef union {
    struct {
        unsigned int pme_intrablk_det_cost_thr0 : 16 ;
        unsigned int pme_pskip_mvy_consistency_thr : 8 ;
        unsigned int pme_pskip_mvx_consistency_thr : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_INTRABLK_DET;
typedef union {
    struct {
        unsigned int pme_intrablk_det_mv_dif_thr1 : 8 ;
        unsigned int pme_intrablk_det_mv_dif_thr0 : 8 ;
        unsigned int pme_intrablk_det_mvy_thr : 8 ;
        unsigned int pme_intrablk_det_mvx_thr : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_INTRABLK_DET_THR;
typedef union {
    struct {
        unsigned int pme_skin_u_max_thr : 8 ;
        unsigned int pme_skin_u_min_thr : 8 ;
        unsigned int pme_skin_v_max_thr : 8 ;
        unsigned int pme_skin_v_min_thr : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SKIN_THR;
typedef union {
    struct {
        unsigned int pme_skin_num : 9 ;
        unsigned int pme_strong_edge_thr : 8 ;
        unsigned int pme_strong_edge_cnt : 5 ;
        unsigned int pme_still_scene_thr : 9 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_STRONG_EDGE;
typedef union {
    struct {
        unsigned int pme_move_scene_thr : 8 ;
        unsigned int pme_move_sad_thr : 14 ;
        unsigned int reserved_0 : 10 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_LARGE_MOVE_THR;
typedef union {
    struct {
        unsigned int pme_interdiff_max_min_madi_abs : 8 ;
        unsigned int pme_interdiff_max_min_madi_times : 8 ;
        unsigned int pme_interstrongedge_madi_thr : 8 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_INTER_STRONG_EDGE;
typedef union {
    struct {
        unsigned int vcpi_strong_edge_thr_u : 8 ;
        unsigned int reserved_0 : 2 ;
        unsigned int vcpi_strong_edge_cnt_u : 5 ;
        unsigned int reserved_1 : 1 ;
        unsigned int vcpi_strong_edge_thr_v : 8 ;
        unsigned int reserved_2 : 2 ;
        unsigned int vcpi_strong_edge_cnt_v : 5 ;
        unsigned int reserved_3 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_CHROMA_STRONG_EDGE;
typedef union {
    struct {
        unsigned int vcpi_chroma_sad_thr_offset : 8 ;
        unsigned int vcpi_chroma_sad_thr_gain : 4 ;
        unsigned int reserved_0 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_SAD_THR;
typedef union {
    struct {
        unsigned int vcpi_chroma_u0_thr_min : 8 ;
        unsigned int vcpi_chroma_u0_thr_max : 8 ;
        unsigned int vcpi_chroma_v0_thr_min : 8 ;
        unsigned int vcpi_chroma_v0_thr_max : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_FG_THR;
typedef union {
    struct {
        unsigned int vcpi_chroma_u1_thr_min : 8 ;
        unsigned int vcpi_chroma_u1_thr_max : 8 ;
        unsigned int vcpi_chroma_v1_thr_min : 8 ;
        unsigned int vcpi_chroma_v1_thr_max : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_BG_THR;
typedef union {
    struct {
        unsigned int vcpi_chroma_uv0_thr_min : 9 ;
        unsigned int reserved_0 : 7 ;
        unsigned int vcpi_chroma_uv0_thr_max : 9 ;
        unsigned int reserved_1 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_SUM_FG_THR;
typedef union {
    struct {
        unsigned int vcpi_chroma_uv1_thr_min : 9 ;
        unsigned int reserved_0 : 7 ;
        unsigned int vcpi_chroma_uv1_thr_max : 9 ;
        unsigned int reserved_1 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_SUM_BG_THR;
typedef union {
    struct {
        unsigned int vcpi_chroma_count0_thr_min : 9 ;
        unsigned int reserved_0 : 7 ;
        unsigned int vcpi_chroma_count0_thr_max : 9 ;
        unsigned int reserved_1 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_FG_COUNT_THR;
typedef union {
    struct {
        unsigned int vcpi_chroma_count1_thr_min : 9 ;
        unsigned int reserved_0 : 7 ;
        unsigned int vcpi_chroma_count1_thr_max : 9 ;
        unsigned int reserved_1 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_CHROMA_BG_COUNT_THR;
typedef union {
    struct {
        unsigned int vcpi_move_scene_mv_thr : 13 ;
        unsigned int reserved_0 : 3 ;
        unsigned int vcpi_move_scene_mv_en : 1 ;
        unsigned int reserved_1 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_MOVE_SCENE_THR;
typedef union {
    struct {
        unsigned int vcpi_new_madi_th0 : 8 ;
        unsigned int vcpi_new_madi_th1 : 8 ;
        unsigned int vcpi_new_madi_th2 : 8 ;
        unsigned int vcpi_new_madi_th3 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_NEW_MADI_TH;
typedef union {
    struct {
        unsigned int vcpi_new_lambda : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_NEW_LAMBDA;
typedef union {
    struct {
        unsigned int vcpi_dblk_beta : 4 ;
        unsigned int vcpi_dblk_alpha : 4 ;
        unsigned int vcpi_dblk_filter_flag : 2 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_DBLKCFG;
typedef union {
    struct {
        unsigned int fme_pu8_bias_cost : 16 ;
        unsigned int fme_pu16_bias_cost : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_FME_BIAS_COST0;
typedef union {
    struct {
        unsigned int fme_pu32_bias_cost : 16 ;
        unsigned int fme_pu64_bias_cost : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_FME_BIAS_COST1;
typedef union {
    struct {
        unsigned int mrg_pu8_bias_cost : 16 ;
        unsigned int mrg_pu16_bias_cost : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_BIAS_COST0;
typedef union {
    struct {
        unsigned int mrg_pu32_bias_cost : 16 ;
        unsigned int mrg_pu64_bias_cost : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_BIAS_COST1;
typedef union {
    struct {
        unsigned int mrg_pu8_abs_offset : 16 ;
        unsigned int mrg_pu16_abs_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_ABS_OFFSET0;
typedef union {
    struct {
        unsigned int mrg_pu32_abs_offset : 16 ;
        unsigned int mrg_pu64_abs_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_ABS_OFFSET1;
typedef union {
    struct {
        unsigned int cu8_fz_weight : 4 ;
        unsigned int cu16_fz_weight : 4 ;
        unsigned int cu32_fz_weight : 4 ;
        unsigned int cu64_fz_weight : 4 ;
        unsigned int cu8_fz_adapt_weight : 4 ;
        unsigned int cu16_fz_adapt_weight : 4 ;
        unsigned int cu32_fz_adapt_weight : 4 ;
        unsigned int cu64_fz_adapt_weight : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_ADJ_WEIGHT;
typedef union {
    struct {
        unsigned int intra_bit_weight : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_BIT_WEIGHT;
typedef union {
    struct {
        unsigned int intra_cu16_rdo_cost_offset : 16 ;
        unsigned int intra_cu32_rdo_cost_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_RDO_COST_OFFSET_0;
typedef union {
    struct {
        unsigned int intra_cu4_rdo_cost_offset : 16 ;
        unsigned int intra_cu8_rdo_cost_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_RDO_COST_OFFSET_1;
typedef union {
    struct {
        unsigned int intra_cu16_non_dc_mode_offset : 16 ;
        unsigned int intra_cu32_non_dc_mode_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_NO_DC_COST_OFFSET_0;
typedef union {
    struct {
        unsigned int intra_cu4_non_dc_mode_offset : 16 ;
        unsigned int intra_cu8_non_dc_mode_offset : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_NO_DC_COST_OFFSET_1;
typedef union {
    struct {
        unsigned int intra_h264_rdo_cost_offset : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_RDO_COST_OFFSET_3;
typedef union {
    struct {
        unsigned int sel_offset_strength : 3 ;
        unsigned int reserved_0 : 29 ;
    } bits;
    unsigned int u32;
} U_VEDU_SEL_OFFSET_STRENGTH;
typedef union {
    struct {
        unsigned int sel_cu32_dc_ac_th_offset : 2 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_SEL_CU32_DC_AC_TH_OFFSET;
typedef union {
    struct {
        unsigned int sel_cu32_qp0_th : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int sel_cu32_qp1_th : 6 ;
        unsigned int reserved_1 : 18 ;
    } bits;
    unsigned int u32;
} U_VEDU_SEL_CU32_QP_TH;
typedef union {
    struct {
        unsigned int sel_res16_luma_dc_th : 4 ;
        unsigned int sel_res16_chroma_dc_th : 4 ;
        unsigned int sel_res16_luma_ac_th : 4 ;
        unsigned int sel_res16_chroma_ac_th : 4 ;
        unsigned int sel_res32_luma_dc_th : 4 ;
        unsigned int sel_res32_chroma_dc_th : 4 ;
        unsigned int sel_res32_luma_ac_th : 4 ;
        unsigned int sel_res32_chroma_ac_th : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_SEL_RES_DC_AC_TH;
typedef union {
    struct {
        unsigned int qpg_lambda00 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG00;
typedef union {
    struct {
        unsigned int qpg_lambda01 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG01;
typedef union {
    struct {
        unsigned int qpg_lambda02 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG02;
typedef union {
    struct {
        unsigned int qpg_lambda03 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG03;
typedef union {
    struct {
        unsigned int qpg_lambda04 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG04;
typedef union {
    struct {
        unsigned int qpg_lambda05 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG05;
typedef union {
    struct {
        unsigned int qpg_lambda06 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG06;
typedef union {
    struct {
        unsigned int qpg_lambda07 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG07;
typedef union {
    struct {
        unsigned int qpg_lambda08 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG08;
typedef union {
    struct {
        unsigned int qpg_lambda09 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG09;
typedef union {
    struct {
        unsigned int qpg_lambda10 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG10;
typedef union {
    struct {
        unsigned int qpg_lambda11 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG11;
typedef union {
    struct {
        unsigned int qpg_lambda12 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG12;
typedef union {
    struct {
        unsigned int qpg_lambda13 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG13;
typedef union {
    struct {
        unsigned int qpg_lambda14 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG14;
typedef union {
    struct {
        unsigned int qpg_lambda15 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG15;
typedef union {
    struct {
        unsigned int qpg_lambda16 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG16;
typedef union {
    struct {
        unsigned int qpg_lambda17 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG17;
typedef union {
    struct {
        unsigned int qpg_lambda18 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG18;
typedef union {
    struct {
        unsigned int qpg_lambda19 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG19;
typedef union {
    struct {
        unsigned int qpg_lambda20 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG20;
typedef union {
    struct {
        unsigned int qpg_lambda21 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG21;
typedef union {
    struct {
        unsigned int qpg_lambda22 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG22;
typedef union {
    struct {
        unsigned int qpg_lambda23 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG23;
typedef union {
    struct {
        unsigned int qpg_lambda24 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG24;
typedef union {
    struct {
        unsigned int qpg_lambda25 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG25;
typedef union {
    struct {
        unsigned int qpg_lambda26 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG26;
typedef union {
    struct {
        unsigned int qpg_lambda27 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG27;
typedef union {
    struct {
        unsigned int qpg_lambda28 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG28;
typedef union {
    struct {
        unsigned int qpg_lambda29 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG29;
typedef union {
    struct {
        unsigned int qpg_lambda30 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG30;
typedef union {
    struct {
        unsigned int qpg_lambda31 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG31;
typedef union {
    struct {
        unsigned int qpg_lambda32 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG32;
typedef union {
    struct {
        unsigned int qpg_lambda33 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG33;
typedef union {
    struct {
        unsigned int qpg_lambda34 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG34;
typedef union {
    struct {
        unsigned int qpg_lambda35 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG35;
typedef union {
    struct {
        unsigned int qpg_lambda36 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG36;
typedef union {
    struct {
        unsigned int qpg_lambda37 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG37;
typedef union {
    struct {
        unsigned int qpg_lambda38 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG38;
typedef union {
    struct {
        unsigned int qpg_lambda39 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG39;
typedef union {
    struct {
        unsigned int qpg_lambda40 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG40;
typedef union {
    struct {
        unsigned int qpg_lambda41 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG41;
typedef union {
    struct {
        unsigned int qpg_lambda42 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG42;
typedef union {
    struct {
        unsigned int qpg_lambda43 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG43;
typedef union {
    struct {
        unsigned int qpg_lambda44 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG44;
typedef union {
    struct {
        unsigned int qpg_lambda45 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG45;
typedef union {
    struct {
        unsigned int qpg_lambda46 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG46;
typedef union {
    struct {
        unsigned int qpg_lambda47 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG47;
typedef union {
    struct {
        unsigned int qpg_lambda48 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG48;
typedef union {
    struct {
        unsigned int qpg_lambda49 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG49;
typedef union {
    struct {
        unsigned int qpg_lambda50 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG50;
typedef union {
    struct {
        unsigned int qpg_lambda51 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG51;
typedef union {
    struct {
        unsigned int qpg_lambda52 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG52;
typedef union {
    struct {
        unsigned int qpg_lambda53 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG53;
typedef union {
    struct {
        unsigned int qpg_lambda54 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG54;
typedef union {
    struct {
        unsigned int qpg_lambda55 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG55;
typedef union {
    struct {
        unsigned int qpg_lambda56 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG56;
typedef union {
    struct {
        unsigned int qpg_lambda57 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG57;
typedef union {
    struct {
        unsigned int qpg_lambda58 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG58;
typedef union {
    struct {
        unsigned int qpg_lambda59 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG59;
typedef union {
    struct {
        unsigned int qpg_lambda60 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG60;
typedef union {
    struct {
        unsigned int qpg_lambda61 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG61;
typedef union {
    struct {
        unsigned int qpg_lambda62 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG62;
typedef union {
    struct {
        unsigned int qpg_lambda63 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG63;
typedef union {
    struct {
        unsigned int qpg_lambda64 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG64;
typedef union {
    struct {
        unsigned int qpg_lambda65 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG65;
typedef union {
    struct {
        unsigned int qpg_lambda66 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG66;
typedef union {
    struct {
        unsigned int qpg_lambda67 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG67;
typedef union {
    struct {
        unsigned int qpg_lambda68 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG68;
typedef union {
    struct {
        unsigned int qpg_lambda69 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG69;
typedef union {
    struct {
        unsigned int qpg_lambda70 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG70;
typedef union {
    struct {
        unsigned int qpg_lambda71 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG71;
typedef union {
    struct {
        unsigned int qpg_lambda72 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG72;
typedef union {
    struct {
        unsigned int qpg_lambda73 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG73;
typedef union {
    struct {
        unsigned int qpg_lambda74 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG74;
typedef union {
    struct {
        unsigned int qpg_lambda75 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG75;
typedef union {
    struct {
        unsigned int qpg_lambda76 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG76;
typedef union {
    struct {
        unsigned int qpg_lambda77 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG77;
typedef union {
    struct {
        unsigned int qpg_lambda78 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG78;
typedef union {
    struct {
        unsigned int qpg_lambda79 : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_QPG_QP_LAMBDA_CTRL_REG79;
typedef union {
    struct {
        unsigned int ime_lambdaoff8 : 16 ;
        unsigned int ime_lambdaoff16 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_RDOCFG;
typedef union {
    struct {
        unsigned int mrg_force_zero_en : 1 ;
        unsigned int mrg_force_y_zero_en : 1 ;
        unsigned int mrg_force_u_zero_en : 1 ;
        unsigned int mrg_force_v_zero_en : 1 ;
        unsigned int fme_rdo_lpw_en : 1 ;
        unsigned int dct4_en : 1 ;
        unsigned int force_adapt_en : 1 ;
        unsigned int reserved_0 : 5 ;
        unsigned int rqt_bias_weight : 4 ;
        unsigned int fme_rdo_lpw_th : 10 ;
        unsigned int mrg_skip_weight_en : 1 ;
        unsigned int reserved_1 : 5 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_FORCE_ZERO_EN;
typedef union {
    struct {
        unsigned int vcpi_intra_lowpow_en : 1 ;
        unsigned int vcpi_fme_lowpow_en : 1 ;
        unsigned int vcpi_ime_lowpow_en : 1 ;
        unsigned int powerpro_func_bypass : 1 ;
        unsigned int vcpi_ddr_cross_idx : 11 ;
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_tqitq_gtck_en : 1 ;
        unsigned int vcpi_mrg_gtck_en : 1 ;
        unsigned int vcpi_recst_hfbc_clkgate_en : 1 ;
        unsigned int vcpi_clkgate_en : 2 ;
        unsigned int vcpi_mem_clkgate_en : 1 ;
        unsigned int venc_dpm_apb_clk_en : 1 ;
        unsigned int vcpi_ddr_cross_en : 1 ;
        unsigned int vcpi_osd_clkgete_en : 1 ;
        unsigned int vcpi_ghdr_clkgate_en : 1 ;
        unsigned int vcpi_curld_dcmp_clkgate_en : 1 ;
        unsigned int vcpi_refld_dcmp_clkgate_en : 1 ;
        unsigned int vcpi_cpi_clkgate_en : 1 ;
        unsigned int vcpi_vlc_clkgate_en : 1 ;
        unsigned int venc_axi_dfx_clk_en : 1 ;
        unsigned int venc_apb_cfg_force_clk_on : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_LOW_POWER;
typedef union {
    struct {
        unsigned int ime_layer3to2_en : 1 ;
        unsigned int ime_inter8x8_en : 1 ;
        unsigned int ime_flat_region_force_low3layer : 1 ;
        unsigned int reserved_0 : 1 ;
        unsigned int ime_high3pre_en : 2 ;
        unsigned int reserved_1 : 2 ;
        unsigned int ime_intra4_lowpow_en : 1 ;
        unsigned int tile_boundry_improve_en : 1 ;
        unsigned int reserved_2 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_INTER_MODE;
typedef union {
    struct {
        unsigned int reserved_0 : 1 ;
        unsigned int vcpi_pblk_pre_en : 1 ;
        unsigned int vcpi_force_inter : 1 ;
        unsigned int vcpi_pintra_inter_flag_disable : 1 ;
        unsigned int vcpi_ext_edge_en : 1 ;
        unsigned int reserved_1 : 27 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PRE_JUDGE_EXT_EN;
typedef union {
    struct {
        unsigned int pme_l0_psw_adapt_en : 1 ;
        unsigned int pme_l1_psw_adapt_en : 1 ;
        unsigned int reserved_0 : 30 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SW_ADAPT_EN;
typedef union {
    struct {
        unsigned int pme_l0_win0_width : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int pme_l0_win0_height : 9 ;
        unsigned int reserved_1 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_WINDOW_SIZE0_L0;
typedef union {
    struct {
        unsigned int pme_l1_win0_width : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int pme_l1_win0_height : 9 ;
        unsigned int reserved_1 : 7 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_WINDOW_SIZE0_L1;
typedef union {
    struct {
        unsigned int pme_skipblk_pre_cost_thr : 16 ;
        unsigned int pme_skipblk_pre_en : 1 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SKIP_PRE;
typedef union {
    struct {
        unsigned int pme_pblk_pre_mv_dif_thr1 : 8 ;
        unsigned int pme_pblk_pre_mv_dif_thr0 : 8 ;
        unsigned int pme_pblk_pre_mv_dif_cost_thr : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_PBLK_PRE1;
typedef union {
    struct {
        unsigned int vcpi_iblk_pre_cost_thr : 16 ;
        unsigned int vcpi_pblk_pre_cost_thr : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_PRE_JUDGE_COST_THR;
typedef union {
    struct {
        unsigned int vcpi_iblk_pre_mv_dif_thr0 : 8 ;
        unsigned int vcpi_iblk_pre_mv_dif_thr1 : 8 ;
        unsigned int vcpi_iblk_pre_mvx_thr : 8 ;
        unsigned int vcpi_iblk_pre_mvy_thr : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_IBLK_PRE_MV_THR;
typedef union {
    struct {
        unsigned int pme_iblk_pre_cost_thr_h264 : 16 ;
        unsigned int pme_intrablk_det_cost_thr1 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_IBLK_COST_THR;
typedef union {
    struct {
        unsigned int pme_tr_weightx_0 : 9 ;
        unsigned int pme_tr_weightx_1 : 9 ;
        unsigned int pme_tr_weightx_2 : 9 ;
        unsigned int reserved_0 : 5 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_TR_WEIGHTX;
typedef union {
    struct {
        unsigned int pme_tr_weighty_0 : 8 ;
        unsigned int pme_tr_weighty_1 : 8 ;
        unsigned int pme_tr_weighty_2 : 8 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_TR_WEIGHTY;
typedef union {
    struct {
        unsigned int pme_sr_weight_0 : 4 ;
        unsigned int pme_sr_weight_1 : 4 ;
        unsigned int pme_sr_weight_2 : 4 ;
        unsigned int reserved_0 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SR_WEIGHT;
typedef union {
    struct {
        unsigned int pme_intra16_madi_thr : 8 ;
        unsigned int pme_intra32_madi_thr : 8 ;
        unsigned int pme_intra_lowpow_en : 1 ;
        unsigned int pme_inter_first : 1 ;
        unsigned int reserved_0 : 14 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_INTRA_LOWPOW;
typedef union {
    struct {
        unsigned int ime_lowpow_fme_thr0 : 6 ;
        unsigned int ime_lowpow_fme_thr1 : 6 ;
        unsigned int reserved_0 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_FME_LPOW_THR;
typedef union {
    struct {
        unsigned int vcpi_pskip_strongedge_madi_thr : 8 ;
        unsigned int vcpi_pskip_strongedge_madi_times : 8 ;
        unsigned int vcpi_pskip_flatregion_madi_thr : 8 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_SKIP_FLAG;
typedef union {
    struct {
        unsigned int pme_psw_lp_diff_thy : 4 ;
        unsigned int reserved_0 : 4 ;
        unsigned int pme_psw_lp_diff_thx : 4 ;
        unsigned int reserved_1 : 20 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_PSW_LPW;
typedef union {
    struct {
        unsigned int pme_pblk_pre_madi_times : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_VEDU_PME_PBLK_PRE2;
typedef union {
    struct {
        unsigned int ime_layer3to2_thr0 : 10 ;
        unsigned int reserved_0 : 6 ;
        unsigned int ime_layer3to2_thr1 : 10 ;
        unsigned int reserved_1 : 6 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_LAYER3TO2_THR;
typedef union {
    struct {
        unsigned int ime_layer3to2_cost_diff_thr : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_LAYER3TO2_THR1;
typedef union {
    struct {
        unsigned int ime_layer3to1_en : 1 ;
        unsigned int reserved_0 : 7 ;
        unsigned int ime_layer3to1_pu64_madi_thr : 7 ;
        unsigned int reserved_1 : 17 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_LAYER3TO1_THR;
typedef union {
    struct {
        unsigned int ime_layer3to1_pu32_cost_thr : 16 ;
        unsigned int ime_layer3to1_pu64_cost_thr : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_IME_LAYER3TO1_THR1;
typedef union {
    struct {
        unsigned int fme_pu64_lwp_flag : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_FME_PU64_LWP;
typedef union {
    struct {
        unsigned int inter32_use_tu16_en : 1 ;
        unsigned int mrg_not_use_sad_en : 1 ;
        unsigned int reserved_0 : 2 ;
        unsigned int mrg_force_skip_en : 1 ;
        unsigned int mrg_not_use_sad_th : 18 ;
        unsigned int reserved_1 : 9 ;
    } bits;
    unsigned int u32;
} U_VEDU_MRG_FORCE_SKIP_EN;
typedef union {
    struct {
        unsigned int constrained_intra_pred_flag : 1 ;
        unsigned int reserved_0 : 3 ;
        unsigned int intra_smooth : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int vcpi_force_cu16_low_pow : 1 ;
        unsigned int reserved_2 : 3 ;
        unsigned int vcpi_force_cu32_low_pow : 1 ;
        unsigned int reserved_3 : 19 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_CFG;
typedef union {
    struct {
        unsigned int cu16_weak_ang_thr : 4 ;
        unsigned int cu16_medium_ang_thr : 4 ;
        unsigned int cu16_strong_ang_thr : 4 ;
        unsigned int cu16_rdo_num : 3 ;
        unsigned int cu16_adaptive_reduce_rdo_en : 1 ;
        unsigned int cu32_weak_ang_thr : 4 ;
        unsigned int cu32_medium_ang_thr : 4 ;
        unsigned int cu32_strong_ang_thr : 4 ;
        unsigned int cu32_rdo_num : 3 ;
        unsigned int cu32_adaptive_reduce_rdo_en : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_INTRA_LOW_POW;
typedef union {
    struct {
        unsigned int sao_area0_start_lcux : 16 ;
        unsigned int sao_area0_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA0_START;
typedef union {
    struct {
        unsigned int sao_area0_end_lcux : 16 ;
        unsigned int sao_area0_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA0_END;
typedef union {
    struct {
        unsigned int sao_area1_start_lcux : 16 ;
        unsigned int sao_area1_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA1_START;
typedef union {
    struct {
        unsigned int sao_area1_end_lcux : 16 ;
        unsigned int sao_area1_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA1_END;
typedef union {
    struct {
        unsigned int sao_area2_start_lcux : 16 ;
        unsigned int sao_area2_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA2_START;
typedef union {
    struct {
        unsigned int sao_area2_end_lcux : 16 ;
        unsigned int sao_area2_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA2_END;
typedef union {
    struct {
        unsigned int sao_area3_start_lcux : 16 ;
        unsigned int sao_area3_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA3_START;
typedef union {
    struct {
        unsigned int sao_area3_end_lcux : 16 ;
        unsigned int sao_area3_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA3_END;
typedef union {
    struct {
        unsigned int sao_area4_start_lcux : 16 ;
        unsigned int sao_area4_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA4_START;
typedef union {
    struct {
        unsigned int sao_area4_end_lcux : 16 ;
        unsigned int sao_area4_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA4_END;
typedef union {
    struct {
        unsigned int sao_area5_start_lcux : 16 ;
        unsigned int sao_area5_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA5_START;
typedef union {
    struct {
        unsigned int sao_area5_end_lcux : 16 ;
        unsigned int sao_area5_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA5_END;
typedef union {
    struct {
        unsigned int sao_area6_start_lcux : 16 ;
        unsigned int sao_area6_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA6_START;
typedef union {
    struct {
        unsigned int sao_area6_end_lcux : 16 ;
        unsigned int sao_area6_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA6_END;
typedef union {
    struct {
        unsigned int sao_area7_start_lcux : 16 ;
        unsigned int sao_area7_start_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA7_START;
typedef union {
    struct {
        unsigned int sao_area7_end_lcux : 16 ;
        unsigned int sao_area7_end_lcuy : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_SAO_SSD_AREA7_END;
typedef union {
    struct {
        unsigned int vcpi_curld_c_stride : 16 ;
        unsigned int vcpi_curld_y_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_STRIDE;
typedef union {
    struct {
        unsigned int vcpi_curld_ext_c_stride : 16 ;
        unsigned int vcpi_curld_ext_y_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_EXT_STRIDE;
typedef union {
    struct {
        unsigned int vcpi_refc_l0_stride : 16 ;
        unsigned int vcpi_refy_l0_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_REF_L0_STRIDE;
typedef union {
    struct {
        unsigned int vcpi_refc_l1_stride : 16 ;
        unsigned int vcpi_refy_l1_stride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_REF_L1_STRIDE;
typedef union {
    struct {
        unsigned int vcpi_recst_ystride : 16 ;
        unsigned int vcpi_recst_cstride : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_VCPI_REC_STRIDE;
typedef union {
    struct {
        unsigned int ghdr_en : 1 ;
        unsigned int ghdr_ck_gt_en : 1 ;
        unsigned int reserved_0 : 10 ;
        unsigned int ghdr_demo_en : 1 ;
        unsigned int ghdr_demo_mode : 1 ;
        unsigned int reserved_1 : 2 ;
        unsigned int ghdr_demo_pos : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_CTRL;
typedef union {
    struct {
        unsigned int ghdr_degmm_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DEGAMMA_CTRL;
typedef union {
    struct {
        unsigned int g_degmm_x1_step : 4 ;
        unsigned int g_degmm_x2_step : 4 ;
        unsigned int g_degmm_x3_step : 4 ;
        unsigned int g_degmm_x4_step : 4 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DEGAMMA_STEP;
typedef union {
    struct {
        unsigned int g_degmm_x1_pos : 10 ;
        unsigned int g_degmm_x2_pos : 10 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DEGAMMA_POS1;
typedef union {
    struct {
        unsigned int g_degmm_x3_pos : 10 ;
        unsigned int g_degmm_x4_pos : 10 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DEGAMMA_POS2;
typedef union {
    struct {
        unsigned int g_degmm_x1_num : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int g_degmm_x2_num : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int g_degmm_x3_num : 6 ;
        unsigned int reserved_2 : 2 ;
        unsigned int g_degmm_x4_num : 6 ;
        unsigned int reserved_3 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DEGAMMA_NUM;
typedef union {
    struct {
        unsigned int ghdr_gamut_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_CTRL;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef00 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF00;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef01 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF01;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef02 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF02;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef10 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF10;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef11 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF11;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef12 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF12;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef20 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF20;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef21 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF21;
typedef union {
    struct {
        unsigned int ghdr_gamut_coef22 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_COEF22;
typedef union {
    struct {
        unsigned int ghdr_gamut_scale : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_SCALE;
typedef union {
    struct {
        unsigned int ghdr_gamut_clip_min : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_CLIP_MIN;
typedef union {
    struct {
        unsigned int ghdr_gamut_clip_max : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMUT_CLIP_MAX;
typedef union {
    struct {
        unsigned int ghdr_tmap_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_CTRL;
typedef union {
    struct {
        unsigned int ghdr_tmap_rd_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_REN;
typedef union {
    struct {
        unsigned int g_tmap_x1_step : 4 ;
        unsigned int reserved_0 : 4 ;
        unsigned int g_tmap_x2_step : 4 ;
        unsigned int reserved_1 : 4 ;
        unsigned int g_tmap_x3_step : 4 ;
        unsigned int reserved_2 : 4 ;
        unsigned int g_tmap_x4_step : 4 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_STEP;
typedef union {
    struct {
        unsigned int g_tmap_x1_pos : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_POS1;
typedef union {
    struct {
        unsigned int g_tmap_x2_pos : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_POS2;
typedef union {
    struct {
        unsigned int g_tmap_x3_pos : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_POS3;
typedef union {
    struct {
        unsigned int g_tmap_x4_pos : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_POS4;
typedef union {
    struct {
        unsigned int g_tmap_x1_num : 5 ;
        unsigned int reserved_0 : 3 ;
        unsigned int g_tmap_x2_num : 5 ;
        unsigned int reserved_1 : 3 ;
        unsigned int g_tmap_x3_num : 5 ;
        unsigned int reserved_2 : 3 ;
        unsigned int g_tmap_x4_num : 5 ;
        unsigned int reserved_3 : 3 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_NUM;
typedef union {
    struct {
        unsigned int ghdr_tmap_luma_coef0 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_LUMA_COEF0;
typedef union {
    struct {
        unsigned int ghdr_tmap_luma_coef1 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_LUMA_COEF1;
typedef union {
    struct {
        unsigned int ghdr_tmap_luma_coef2 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_LUMA_COEF2;
typedef union {
    struct {
        unsigned int ghdr_tmap_luma_scale : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_LUMA_SCALE;
typedef union {
    struct {
        unsigned int ghdr_tmap_coef_scale : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_COEF_SCALE;
typedef union {
    struct {
        unsigned int ghdr_tmap_out_clip_min : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_OUT_CLIP_MIN;
typedef union {
    struct {
        unsigned int ghdr_tmap_out_clip_max : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_TONEMAP_OUT_CLIP_MAX;
typedef union {
    struct {
        unsigned int ghdr_gmm_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_CTRL;
typedef union {
    struct {
        unsigned int ghdr_gamma_rd_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_REN;
typedef union {
    struct {
        unsigned int g_gmm_x1_step : 4 ;
        unsigned int reserved_0 : 4 ;
        unsigned int g_gmm_x2_step : 4 ;
        unsigned int reserved_1 : 4 ;
        unsigned int g_gmm_x3_step : 4 ;
        unsigned int reserved_2 : 4 ;
        unsigned int g_gmm_x4_step : 4 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_STEP1;
typedef union {
    struct {
        unsigned int g_gmm_x5_step : 4 ;
        unsigned int reserved_0 : 4 ;
        unsigned int g_gmm_x6_step : 4 ;
        unsigned int reserved_1 : 4 ;
        unsigned int g_gmm_x7_step : 4 ;
        unsigned int reserved_2 : 4 ;
        unsigned int g_gmm_x8_step : 4 ;
        unsigned int reserved_3 : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_STEP2;
typedef union {
    struct {
        unsigned int g_gmm_x1_pos : 16 ;
        unsigned int g_gmm_x2_pos : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_POS1;
typedef union {
    struct {
        unsigned int g_gmm_x3_pos : 16 ;
        unsigned int g_gmm_x4_pos : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_POS2;
typedef union {
    struct {
        unsigned int g_gmm_x5_pos : 16 ;
        unsigned int g_gmm_x6_pos : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_POS3;
typedef union {
    struct {
        unsigned int g_gmm_x7_pos : 16 ;
        unsigned int g_gmm_x8_pos : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_POS4;
typedef union {
    struct {
        unsigned int g_gmm_x1_num : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int g_gmm_x2_num : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int g_gmm_x3_num : 6 ;
        unsigned int reserved_2 : 2 ;
        unsigned int g_gmm_x4_num : 6 ;
        unsigned int reserved_3 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_NUM1;
typedef union {
    struct {
        unsigned int g_gmm_x5_num : 6 ;
        unsigned int reserved_0 : 2 ;
        unsigned int g_gmm_x6_num : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int g_gmm_x7_num : 6 ;
        unsigned int reserved_2 : 2 ;
        unsigned int g_gmm_x8_num : 6 ;
        unsigned int reserved_3 : 2 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_GAMMA_NUM2;
typedef union {
    struct {
        unsigned int ghdr_dither_tap_mode : 2 ;
        unsigned int ghdr_dither_domain_mode : 1 ;
        unsigned int ghdr_dither_round : 1 ;
        unsigned int ghdr_dither_mode : 1 ;
        unsigned int ghdr_dither_en : 1 ;
        unsigned int ghdr_dither_round_unlim : 1 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_CTRL;
typedef union {
    struct {
        unsigned int ghdr_dither_thr_min : 16 ;
        unsigned int ghdr_dither_thr_max : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_THR;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_y0 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_Y0;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_u0 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_U0;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_v0 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_V0;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_w0 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_W0;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_y1 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_Y1;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_u1 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_U1;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_v1 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_V1;
typedef union {
    struct {
        unsigned int ghdr_dither_sed_w1 : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_VEDU_GHDR_DITHER_SED_W1;
typedef union {
    struct {
        unsigned int hihdr_r2y_en : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_CTRL;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef00 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF0;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef01 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF1;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef02 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF2;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef10 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF3;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef11 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF4;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef12 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF5;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef20 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF6;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef21 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF7;
typedef union {
    struct {
        unsigned int hihdr_r2y_coef22 : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_COEF8;
typedef union {
    struct {
        unsigned int hihdr_r2y_scale2p : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_SCALE2P;
typedef union {
    struct {
        unsigned int hihdr_r2y_idc0 : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_IDC0;
typedef union {
    struct {
        unsigned int hihdr_r2y_idc1 : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_IDC1;
typedef union {
    struct {
        unsigned int hihdr_r2y_idc2 : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_IDC2;
typedef union {
    struct {
        unsigned int hihdr_r2y_odc0 : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_ODC0;
typedef union {
    struct {
        unsigned int hihdr_r2y_odc1 : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_ODC1;
typedef union {
    struct {
        unsigned int hihdr_r2y_odc2 : 11 ;
        unsigned int reserved_0 : 21 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_ODC2;
typedef union {
    struct {
        unsigned int hihdr_r2y_clip_min : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_MIN;
typedef union {
    struct {
        unsigned int hihdr_r2y_clip_max : 10 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_RGB2YUV_MAX;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef00 : 16 ;
        unsigned int hihdr_tonemap_coef01 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_00_01;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef02 : 16 ;
        unsigned int hihdr_tonemap_coef03 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_02_03;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef04 : 16 ;
        unsigned int hihdr_tonemap_coef05 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_04_05;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef06 : 16 ;
        unsigned int hihdr_tonemap_coef07 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_06_07;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef08 : 16 ;
        unsigned int hihdr_tonemap_coef09 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_08_09;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef10 : 16 ;
        unsigned int hihdr_tonemap_coef11 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_10_11;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef12 : 16 ;
        unsigned int hihdr_tonemap_coef13 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_12_13;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef14 : 16 ;
        unsigned int hihdr_tonemap_coef15 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_14_15;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef16 : 16 ;
        unsigned int hihdr_tonemap_coef17 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_16_17;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef18 : 16 ;
        unsigned int hihdr_tonemap_coef19 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_18_19;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef20 : 16 ;
        unsigned int hihdr_tonemap_coef21 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_20_21;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef22 : 16 ;
        unsigned int hihdr_tonemap_coef23 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_22_23;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef24 : 16 ;
        unsigned int hihdr_tonemap_coef25 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_24_25;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef26 : 16 ;
        unsigned int hihdr_tonemap_coef27 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_26_27;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef28 : 16 ;
        unsigned int hihdr_tonemap_coef29 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_28_29;
typedef union {
    struct {
        unsigned int hihdr_tonemap_coef30 : 16 ;
        unsigned int hihdr_tonemap_coef31 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_TONEMAP_COEF_30_31;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef0 : 12 ;
        unsigned int hihdr_gmm_coef1 : 12 ;
        unsigned int hihdr_gmm_coef2_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_00_09_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef2_h4b : 4 ;
        unsigned int hihdr_gmm_coef3 : 12 ;
        unsigned int hihdr_gmm_coef4 : 12 ;
        unsigned int hihdr_gmm_coef5_l4b : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_00_09_P1;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef5_h8b : 8 ;
        unsigned int hihdr_gmm_coef6 : 12 ;
        unsigned int hihdr_gmm_coef7 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_00_09_P2;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef8 : 12 ;
        unsigned int hihdr_gmm_coef9 : 12 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_00_09_P3;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef10 : 12 ;
        unsigned int hihdr_gmm_coef11 : 12 ;
        unsigned int hihdr_gmm_coef12_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_10_19_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef12_h4b : 4 ;
        unsigned int hihdr_gmm_coef13 : 12 ;
        unsigned int hihdr_gmm_coef14 : 12 ;
        unsigned int hihdr_gmm_coef15_l4b : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_10_19_P1;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef15_h8b : 8 ;
        unsigned int hihdr_gmm_coef16 : 12 ;
        unsigned int hihdr_gmm_coef17 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_10_19_P2;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef18 : 12 ;
        unsigned int hihdr_gmm_coef19 : 12 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_10_19_P3;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef20 : 12 ;
        unsigned int hihdr_gmm_coef21 : 12 ;
        unsigned int hihdr_gmm_coef22_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_20_29_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef22_h4b : 4 ;
        unsigned int hihdr_gmm_coef23 : 12 ;
        unsigned int hihdr_gmm_coef24 : 12 ;
        unsigned int hihdr_gmm_coef25_l4b : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_20_29_P1;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef25_h8b : 8 ;
        unsigned int hihdr_gmm_coef26 : 12 ;
        unsigned int hihdr_gmm_coef27 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_20_29_P2;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef28 : 12 ;
        unsigned int hihdr_gmm_coef29 : 12 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_20_29_P3;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef30 : 12 ;
        unsigned int hihdr_gmm_coef31 : 12 ;
        unsigned int hihdr_gmm_coef32_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_30_39_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef32_h4b : 4 ;
        unsigned int hihdr_gmm_coef33 : 12 ;
        unsigned int hihdr_gmm_coef34 : 12 ;
        unsigned int hihdr_gmm_coef35_l4b : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_30_39_P1;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef35_h8b : 8 ;
        unsigned int hihdr_gmm_coef36 : 12 ;
        unsigned int hihdr_gmm_coef37 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_30_39_P2;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef38 : 12 ;
        unsigned int hihdr_gmm_coef39 : 12 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_30_39_P3;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef40 : 12 ;
        unsigned int hihdr_gmm_coef41 : 12 ;
        unsigned int hihdr_gmm_coef42_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_40_49_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef42_h4b : 4 ;
        unsigned int hihdr_gmm_coef43 : 12 ;
        unsigned int hihdr_gmm_coef44 : 12 ;
        unsigned int hihdr_gmm_coef45_l4b : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_40_49_P1;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef45_h8b : 8 ;
        unsigned int hihdr_gmm_coef46 : 12 ;
        unsigned int hihdr_gmm_coef47 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_40_49_P2;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef48 : 12 ;
        unsigned int hihdr_gmm_coef49 : 12 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_40_49_P3;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef50 : 12 ;
        unsigned int hihdr_gmm_coef51 : 12 ;
        unsigned int hihdr_gmm_coef52_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_50_59_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef52_h4b : 4 ;
        unsigned int hihdr_gmm_coef53 : 12 ;
        unsigned int hihdr_gmm_coef54 : 12 ;
        unsigned int hihdr_gmm_coef55_l4b : 4 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_50_59_P1;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef55_h8b : 8 ;
        unsigned int hihdr_gmm_coef56 : 12 ;
        unsigned int hihdr_gmm_coef57 : 12 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_50_59_P2;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef58 : 12 ;
        unsigned int hihdr_gmm_coef59 : 12 ;
        unsigned int reserved_0 : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_50_59_P3;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef60 : 12 ;
        unsigned int hihdr_gmm_coef61 : 12 ;
        unsigned int hihdr_gmm_coef62_l8b : 8 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_60_69_P0;
typedef union {
    struct {
        unsigned int hihdr_gmm_coef62_h4b : 4 ;
        unsigned int hihdr_gmm_coef63 : 12 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_VEDU_HIHDR_G_GMM_COEF_60_69_P1;
typedef union {
    struct {
        unsigned int venc_top_secure_n : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_VEDU_SECURE_SET;
typedef union {
    struct {
        unsigned int vcpi_int_ve_eop : 1 ;
        unsigned int vcpi_int_vedu_slice_end : 1 ;
        unsigned int vcpi_int_ve_buffull : 1 ;
        unsigned int vcpi_int_ve_pbitsover : 1 ;
        unsigned int vcpi_int_line_pos : 1 ;
        unsigned int vcpi_int_ppfd_dec_err : 1 ;
        unsigned int vcpi_int_vedu_timeout : 1 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_FUNC_VCPI_INTSTAT;
typedef union {
    struct {
        unsigned int vcpi_rint_ve_eop : 1 ;
        unsigned int vcpi_rint_vedu_slice_end : 1 ;
        unsigned int vcpi_rint_ve_buffull : 1 ;
        unsigned int vcpi_rint_ve_pbitsover : 1 ;
        unsigned int vcpi_rint_line_pos : 1 ;
        unsigned int vcpi_rint_ppfd_dec_err : 1 ;
        unsigned int vcpi_rint_vedu_timeout : 1 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_FUNC_VCPI_RAWINT;
typedef union {
    struct {
        unsigned int bus_idle_flag : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_FUNC_VCPI_BUS_IDLE_FLAG;
typedef union {
    struct {
        unsigned int pme_madi_sum : 25 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADI_SUM;
typedef union {
    struct {
        unsigned int pme_madp_sum : 25 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADP_SUM;
typedef union {
    struct {
        unsigned int pme_madi_num : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADI_NUM;
typedef union {
    struct {
        unsigned int pme_madp_num : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MADP_NUM;
typedef union {
    struct {
        unsigned int large_sad_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_LARGE_SAD_SUM;
typedef union {
    struct {
        unsigned int low_luma_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_LOW_LUMA_SUM;
typedef union {
    struct {
        unsigned int chroma_prot_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_CHROMA_SCENE_SUM;
typedef union {
    struct {
        unsigned int move_scene_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_MOVE_SCENE_SUM;
typedef union {
    struct {
        unsigned int skin_region_sum : 20 ;
        unsigned int reserved_0 : 12 ;
    } bits;
    unsigned int u32;
} U_FUNC_PME_SKIN_REGION_SUM;
typedef union {
    struct {
        unsigned int vcpi_bggen_block_count : 18 ;
        unsigned int reserved_0 : 14 ;
    } bits;
    unsigned int u32;
} U_FUNC_BGSTR_BLOCK_COUNT;
typedef union {
    struct {
        unsigned int vcpi_frame_bgm_dist : 31 ;
        unsigned int reserved_0 : 1 ;
    } bits;
    unsigned int u32;
} U_FUNC_BGSTR_FRAME_BGM_DIST;
typedef union {
    struct {
        unsigned int opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int intra_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int pcm_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int inter_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int fme_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int mrg_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SELINTER_MERGE_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int mrg_skip_opt_8x8_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
typedef union {
    struct {
        unsigned int opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_16X16_CNT;
typedef union {
    struct {
        unsigned int intra_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_OPT_16X16_CNT;
typedef union {
    struct {
        unsigned int opt_4x4_cnt : 19 ;
        unsigned int reserved_0 : 13 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_4X4_CNT;
typedef union {
    struct {
        unsigned int inter_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_OPT_16X16_CNT;
typedef union {
    struct {
        unsigned int fme_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_16X16_CNT;
typedef union {
    struct {
        unsigned int mrg_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
typedef union {
    struct {
        unsigned int mrg_skip_opt_16x16_cnt : 17 ;
        unsigned int reserved_0 : 15 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
typedef union {
    struct {
        unsigned int opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_32X32_CNT;
typedef union {
    struct {
        unsigned int intra_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTRA_OPT_32X32_CNT;
typedef union {
    struct {
        unsigned int inter_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_OPT_32X32_CNT;
typedef union {
    struct {
        unsigned int fme_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_32X32_CNT;
typedef union {
    struct {
        unsigned int mrg_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
typedef union {
    struct {
        unsigned int mrg_skip_opt_32x32_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
typedef union {
    struct {
        unsigned int opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_OPT_64X64_CNT;
typedef union {
    struct {
        unsigned int fme_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_FME_OPT_64X64_CNT;
typedef union {
    struct {
        unsigned int mrg_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
typedef union {
    struct {
        unsigned int mrg_skip_opt_64x64_cnt : 16 ;
        unsigned int reserved_0 : 16 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
typedef union {
    struct {
        unsigned int total_luma_qp : 26 ;
        unsigned int reserved_0 : 6 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_TOTAL_LUMA_QP;
typedef union {
    struct {
        unsigned int min_luma_qp : 6 ;
        unsigned int reserved_1 : 2 ;
        unsigned int max_luma_qp : 6 ;
        unsigned int reserved_0 : 18 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_MAX_MIN_LUMA_QP;
typedef union {
    struct {
        unsigned int luma_qp0_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP0_CNT;
typedef union {
    struct {
        unsigned int luma_qp1_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP1_CNT;
typedef union {
    struct {
        unsigned int luma_qp2_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP2_CNT;
typedef union {
    struct {
        unsigned int luma_qp3_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP3_CNT;
typedef union {
    struct {
        unsigned int luma_qp4_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP4_CNT;
typedef union {
    struct {
        unsigned int luma_qp5_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP5_CNT;
typedef union {
    struct {
        unsigned int luma_qp6_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP6_CNT;
typedef union {
    struct {
        unsigned int luma_qp7_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP7_CNT;
typedef union {
    struct {
        unsigned int luma_qp8_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP8_CNT;
typedef union {
    struct {
        unsigned int luma_qp9_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP9_CNT;
typedef union {
    struct {
        unsigned int luma_qp10_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP10_CNT;
typedef union {
    struct {
        unsigned int luma_qp11_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP11_CNT;
typedef union {
    struct {
        unsigned int luma_qp12_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP12_CNT;
typedef union {
    struct {
        unsigned int luma_qp13_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP13_CNT;
typedef union {
    struct {
        unsigned int luma_qp14_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP14_CNT;
typedef union {
    struct {
        unsigned int luma_qp15_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP15_CNT;
typedef union {
    struct {
        unsigned int luma_qp16_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP16_CNT;
typedef union {
    struct {
        unsigned int luma_qp17_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP17_CNT;
typedef union {
    struct {
        unsigned int luma_qp18_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP18_CNT;
typedef union {
    struct {
        unsigned int luma_qp19_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP19_CNT;
typedef union {
    struct {
        unsigned int luma_qp20_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP20_CNT;
typedef union {
    struct {
        unsigned int luma_qp21_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP21_CNT;
typedef union {
    struct {
        unsigned int luma_qp22_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP22_CNT;
typedef union {
    struct {
        unsigned int luma_qp23_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP23_CNT;
typedef union {
    struct {
        unsigned int luma_qp24_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP24_CNT;
typedef union {
    struct {
        unsigned int luma_qp25_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP25_CNT;
typedef union {
    struct {
        unsigned int luma_qp26_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP26_CNT;
typedef union {
    struct {
        unsigned int luma_qp27_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP27_CNT;
typedef union {
    struct {
        unsigned int luma_qp28_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP28_CNT;
typedef union {
    struct {
        unsigned int luma_qp29_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP29_CNT;
typedef union {
    struct {
        unsigned int luma_qp30_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP30_CNT;
typedef union {
    struct {
        unsigned int luma_qp31_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP31_CNT;
typedef union {
    struct {
        unsigned int luma_qp32_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP32_CNT;
typedef union {
    struct {
        unsigned int luma_qp33_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP33_CNT;
typedef union {
    struct {
        unsigned int luma_qp34_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP34_CNT;
typedef union {
    struct {
        unsigned int luma_qp35_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP35_CNT;
typedef union {
    struct {
        unsigned int luma_qp36_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP36_CNT;
typedef union {
    struct {
        unsigned int luma_qp37_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP37_CNT;
typedef union {
    struct {
        unsigned int luma_qp38_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP38_CNT;
typedef union {
    struct {
        unsigned int luma_qp39_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP39_CNT;
typedef union {
    struct {
        unsigned int luma_qp40_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP40_CNT;
typedef union {
    struct {
        unsigned int luma_qp41_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP41_CNT;
typedef union {
    struct {
        unsigned int luma_qp42_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP42_CNT;
typedef union {
    struct {
        unsigned int luma_qp43_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP43_CNT;
typedef union {
    struct {
        unsigned int luma_qp44_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP44_CNT;
typedef union {
    struct {
        unsigned int luma_qp45_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP45_CNT;
typedef union {
    struct {
        unsigned int luma_qp46_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP46_CNT;
typedef union {
    struct {
        unsigned int luma_qp47_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP47_CNT;
typedef union {
    struct {
        unsigned int luma_qp48_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP48_CNT;
typedef union {
    struct {
        unsigned int luma_qp49_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP49_CNT;
typedef union {
    struct {
        unsigned int luma_qp50_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP50_CNT;
typedef union {
    struct {
        unsigned int luma_qp51_cnt : 21 ;
        unsigned int reserved_0 : 11 ;
    } bits;
    unsigned int u32;
} U_FUNC_SEL_LUMA_QP51_CNT;
typedef union {
    struct {
        unsigned int sao2Vcpi_saooff_num_luma : 14 ;
        unsigned int reserved_1 : 2 ;
        unsigned int sao2Vcpi_saooff_num_chroma : 14 ;
        unsigned int reserved_0 : 2 ;
    } bits;
    unsigned int u32;
} U_FUNC_SAO_OFF_NUM;
typedef union {
    struct {
        unsigned int sao2vcpi_mse_cnt : 15 ;
        unsigned int reserved_0 : 17 ;
    } bits;
    unsigned int u32;
} U_FUNC_SAO_MSE_CNT;
typedef union {
    struct {
        unsigned int sao2vcpi_mse_max : 28 ;
        unsigned int reserved_0 : 4 ;
    } bits;
    unsigned int u32;
} U_FUNC_SAO_MSE_MAX;
typedef union {
    struct {
        unsigned int dfx_ar_r_err : 1 ;
        unsigned int dfx_aw_w_err : 1 ;
        unsigned int dfx_aw_b_err : 1 ;
        unsigned int dfx_arid_err : 1 ;
        unsigned int dfx_rid_err : 1 ;
        unsigned int dfx_awid_err : 1 ;
        unsigned int dfx_wid_err : 1 ;
        unsigned int dfx_bid_err : 1 ;
        unsigned int dfx_arid_tx_err : 1 ;
        unsigned int dfx_rid_rx_err : 1 ;
        unsigned int dfx_awid_tx_err : 1 ;
        unsigned int dfx_bid_rx_err : 1 ;
        unsigned int dfx_arid_len_err : 1 ;
        unsigned int dfx_awid_len : 1 ;
        unsigned int dfx_rresp_err : 1 ;
        unsigned int dfx_bresp_err : 1 ;
        unsigned int dfx_ar_ovr_err : 1 ;
        unsigned int dfx_r_ovr_err : 1 ;
        unsigned int dfx_aw_ovr_err : 1 ;
        unsigned int dfx_w_ovr_err : 1 ;
        unsigned int dfx_b_ovr_err : 1 ;
        unsigned int dfx_ar_outstanding_err : 1 ;
        unsigned int dfx_aw_outstanding_err : 1 ;
        unsigned int dfx_arlen_err : 1 ;
        unsigned int dfx_awlen_err : 1 ;
        unsigned int reserved_0 : 7 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR;
typedef union {
    struct {
        unsigned int ar_r_cnt : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AR_R_CNT;
typedef union {
    struct {
        unsigned int aw_w_cnt : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AW_W_CNT;
typedef union {
    struct {
        unsigned int aw_b_cnt : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AW_B_CNT;
typedef union {
    struct {
        unsigned int dfx_arid_err : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int dfx_rid_err : 1 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AR_R_ID_ERR;
typedef union {
    struct {
        unsigned int dfx_err_arid : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_ARID;
typedef union {
    struct {
        unsigned int dfx_err_rid : 7 ;
        unsigned int reserved_0 : 25 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_RID;
typedef union {
    struct {
        unsigned int dfx_awid_err : 1 ;
        unsigned int reserved_2 : 3 ;
        unsigned int dfx_wid_err : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int dfx_bid_err : 1 ;
        unsigned int reserved_0 : 23 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AW_W_B_ID_ERR;
typedef union {
    struct {
        unsigned int dfx_err_awid : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_AWID;
typedef union {
    struct {
        unsigned int dfx_err_wid : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_WID;
typedef union {
    struct {
        unsigned int dfx_err_bid : 6 ;
        unsigned int reserved_0 : 26 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_BID;
typedef union {
    struct {
        unsigned int dfx_arid0_tx_err : 1 ;
        unsigned int dfx_arid1_tx_err : 1 ;
        unsigned int dfx_arid2_tx_err : 1 ;
        unsigned int dfx_arid3_tx_err : 1 ;
        unsigned int dfx_arid4_tx_err : 1 ;
        unsigned int dfx_arid5_tx_err : 1 ;
        unsigned int dfx_arid6_tx_err : 1 ;
        unsigned int dfx_arid7_tx_err : 1 ;
        unsigned int dfx_arid8_tx_err : 1 ;
        unsigned int dfx_arid9_tx_err : 1 ;
        unsigned int dfx_arid10_tx_err : 1 ;
        unsigned int dfx_arid11_tx_err : 1 ;
        unsigned int dfx_arid12_tx_err : 1 ;
        unsigned int dfx_arid13_tx_err : 1 ;
        unsigned int dfx_arid14_tx_err : 1 ;
        unsigned int dfx_arid15_tx_err : 1 ;
        unsigned int dfx_arid16_tx_err : 1 ;
        unsigned int dfx_arid17_tx_err : 1 ;
        unsigned int dfx_arid18_tx_err : 1 ;
        unsigned int dfx_arid19_tx_err : 1 ;
        unsigned int dfx_arid20_tx_err : 1 ;
        unsigned int dfx_arid21_tx_err : 1 ;
        unsigned int dfx_arid22_tx_err : 1 ;
        unsigned int dfx_arid23_tx_err : 1 ;
        unsigned int dfx_arid24_tx_err : 1 ;
        unsigned int dfx_arid25_tx_err : 1 ;
        unsigned int dfx_arid26_tx_err : 1 ;
        unsigned int dfx_arid27_tx_err : 1 ;
        unsigned int dfx_arid28_tx_err : 1 ;
        unsigned int dfx_arid29_tx_err : 1 ;
        unsigned int dfx_arid30_tx_err : 1 ;
        unsigned int dfx_arid31_tx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_TX_0ERR;
typedef union {
    struct {
        unsigned int dfx_arid32_tx_err : 1 ;
        unsigned int dfx_arid33_tx_err : 1 ;
        unsigned int dfx_arid34_tx_err : 1 ;
        unsigned int dfx_arid35_tx_err : 1 ;
        unsigned int dfx_arid36_tx_err : 1 ;
        unsigned int dfx_arid37_tx_err : 1 ;
        unsigned int dfx_arid38_tx_err : 1 ;
        unsigned int dfx_arid39_tx_err : 1 ;
        unsigned int dfx_arid40_tx_err : 1 ;
        unsigned int dfx_arid41_tx_err : 1 ;
        unsigned int dfx_arid42_tx_err : 1 ;
        unsigned int dfx_arid43_tx_err : 1 ;
        unsigned int dfx_arid44_tx_err : 1 ;
        unsigned int dfx_arid45_tx_err : 1 ;
        unsigned int dfx_arid46_tx_err : 1 ;
        unsigned int dfx_arid47_tx_err : 1 ;
        unsigned int dfx_arid48_tx_err : 1 ;
        unsigned int dfx_arid49_tx_err : 1 ;
        unsigned int dfx_arid50_tx_err : 1 ;
        unsigned int dfx_arid51_tx_err : 1 ;
        unsigned int dfx_arid52_tx_err : 1 ;
        unsigned int dfx_arid53_tx_err : 1 ;
        unsigned int dfx_arid54_tx_err : 1 ;
        unsigned int dfx_arid55_tx_err : 1 ;
        unsigned int dfx_arid56_tx_err : 1 ;
        unsigned int dfx_arid57_tx_err : 1 ;
        unsigned int dfx_arid58_tx_err : 1 ;
        unsigned int dfx_arid59_tx_err : 1 ;
        unsigned int dfx_arid60_tx_err : 1 ;
        unsigned int dfx_arid61_tx_err : 1 ;
        unsigned int dfx_arid62_tx_err : 1 ;
        unsigned int dfx_arid63_tx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_TX_1ERR;
typedef union {
    struct {
        unsigned int dfx_arid64_tx_err : 1 ;
        unsigned int dfx_arid65_tx_err : 1 ;
        unsigned int dfx_arid66_tx_err : 1 ;
        unsigned int dfx_arid67_tx_err : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_TX_2ERR;
typedef union {
    struct {
        unsigned int dfx_rid0_rx_err : 1 ;
        unsigned int dfx_rid1_rx_err : 1 ;
        unsigned int dfx_rid2_rx_err : 1 ;
        unsigned int dfx_rid3_rx_err : 1 ;
        unsigned int dfx_rid4_rx_err : 1 ;
        unsigned int dfx_rid5_rx_err : 1 ;
        unsigned int dfx_rid6_rx_err : 1 ;
        unsigned int dfx_rid7_rx_err : 1 ;
        unsigned int dfx_rid8_rx_err : 1 ;
        unsigned int dfx_rid9_rx_err : 1 ;
        unsigned int dfx_rid10_rx_err : 1 ;
        unsigned int dfx_rid11_rx_err : 1 ;
        unsigned int dfx_rid12_rx_err : 1 ;
        unsigned int dfx_rid13_rx_err : 1 ;
        unsigned int dfx_rid14_rx_err : 1 ;
        unsigned int dfx_rid15_rx_err : 1 ;
        unsigned int dfx_rid16_rx_err : 1 ;
        unsigned int dfx_rid17_rx_err : 1 ;
        unsigned int dfx_rid18_rx_err : 1 ;
        unsigned int dfx_rid19_rx_err : 1 ;
        unsigned int dfx_rid20_rx_err : 1 ;
        unsigned int dfx_rid21_rx_err : 1 ;
        unsigned int dfx_rid22_rx_err : 1 ;
        unsigned int dfx_rid23_rx_err : 1 ;
        unsigned int dfx_rid24_rx_err : 1 ;
        unsigned int dfx_rid25_rx_err : 1 ;
        unsigned int dfx_rid26_rx_err : 1 ;
        unsigned int dfx_rid27_rx_err : 1 ;
        unsigned int dfx_rid28_rx_err : 1 ;
        unsigned int dfx_rid29_rx_err : 1 ;
        unsigned int dfx_rid30_rx_err : 1 ;
        unsigned int dfx_rid31_rx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RID_RX_0ERR;
typedef union {
    struct {
        unsigned int dfx_rid32_rx_err : 1 ;
        unsigned int dfx_rid33_rx_err : 1 ;
        unsigned int dfx_rid34_rx_err : 1 ;
        unsigned int dfx_rid35_rx_err : 1 ;
        unsigned int dfx_rid36_rx_err : 1 ;
        unsigned int dfx_rid37_rx_err : 1 ;
        unsigned int dfx_rid38_rx_err : 1 ;
        unsigned int dfx_rid39_rx_err : 1 ;
        unsigned int dfx_rid40_rx_err : 1 ;
        unsigned int dfx_rid41_rx_err : 1 ;
        unsigned int dfx_rid42_rx_err : 1 ;
        unsigned int dfx_rid43_rx_err : 1 ;
        unsigned int dfx_rid44_rx_err : 1 ;
        unsigned int dfx_rid45_rx_err : 1 ;
        unsigned int dfx_rid46_rx_err : 1 ;
        unsigned int dfx_rid47_rx_err : 1 ;
        unsigned int dfx_rid48_rx_err : 1 ;
        unsigned int dfx_rid49_rx_err : 1 ;
        unsigned int dfx_rid50_rx_err : 1 ;
        unsigned int dfx_rid51_rx_err : 1 ;
        unsigned int dfx_rid52_rx_err : 1 ;
        unsigned int dfx_rid53_rx_err : 1 ;
        unsigned int dfx_rid54_rx_err : 1 ;
        unsigned int dfx_rid55_rx_err : 1 ;
        unsigned int dfx_rid56_rx_err : 1 ;
        unsigned int dfx_rid57_rx_err : 1 ;
        unsigned int dfx_rid58_rx_err : 1 ;
        unsigned int dfx_rid59_rx_err : 1 ;
        unsigned int dfx_rid60_rx_err : 1 ;
        unsigned int dfx_rid61_rx_err : 1 ;
        unsigned int dfx_rid62_rx_err : 1 ;
        unsigned int dfx_rid63_rx_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RID_RX_1ERR;
typedef union {
    struct {
        unsigned int dfx_rid64_rx_err : 1 ;
        unsigned int dfx_rid65_rx_err : 1 ;
        unsigned int dfx_rid66_rx_err : 1 ;
        unsigned int dfx_rid67_rx_err : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RID_RX_2ERR;
typedef union {
    struct {
        unsigned int dfx_awid0_rx_err : 1 ;
        unsigned int dfx_awid1_rx_err : 1 ;
        unsigned int dfx_awid2_rx_err : 1 ;
        unsigned int dfx_awid3_rx_err : 1 ;
        unsigned int dfx_awid4_rx_err : 1 ;
        unsigned int dfx_awid5_rx_err : 1 ;
        unsigned int dfx_awid6_rx_err : 1 ;
        unsigned int dfx_awid7_rx_err : 1 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_RX_0ERR;
typedef union {
    struct {
        unsigned int dfx_bid0_rx_err : 1 ;
        unsigned int dfx_bid1_rx_err : 1 ;
        unsigned int dfx_bid2_rx_err : 1 ;
        unsigned int dfx_bid3_rx_err : 1 ;
        unsigned int dfx_bid4_rx_err : 1 ;
        unsigned int dfx_bid5_rx_err : 1 ;
        unsigned int dfx_bid6_rx_err : 1 ;
        unsigned int dfx_bid7_rx_err : 1 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_BID_RX_ERR;
typedef union {
    struct {
        unsigned int dfx_arid0_len_err : 1 ;
        unsigned int dfx_arid1_len_err : 1 ;
        unsigned int dfx_arid2_len_err : 1 ;
        unsigned int dfx_arid3_len_err : 1 ;
        unsigned int dfx_arid4_len_err : 1 ;
        unsigned int dfx_arid5_len_err : 1 ;
        unsigned int dfx_arid6_len_err : 1 ;
        unsigned int dfx_arid7_len_err : 1 ;
        unsigned int dfx_arid8_len_err : 1 ;
        unsigned int dfx_arid9_len_err : 1 ;
        unsigned int dfx_arid10_len_err : 1 ;
        unsigned int dfx_arid11_len_err : 1 ;
        unsigned int dfx_arid12_len_err : 1 ;
        unsigned int dfx_arid13_len_err : 1 ;
        unsigned int dfx_arid14_len_err : 1 ;
        unsigned int dfx_arid15_len_err : 1 ;
        unsigned int dfx_arid16_len_err : 1 ;
        unsigned int dfx_arid17_len_err : 1 ;
        unsigned int dfx_arid18_len_err : 1 ;
        unsigned int dfx_arid19_len_err : 1 ;
        unsigned int dfx_arid20_len_err : 1 ;
        unsigned int dfx_arid21_len_err : 1 ;
        unsigned int dfx_arid22_len_err : 1 ;
        unsigned int dfx_arid23_len_err : 1 ;
        unsigned int dfx_arid24_len_err : 1 ;
        unsigned int dfx_arid25_len_err : 1 ;
        unsigned int dfx_arid26_len_err : 1 ;
        unsigned int dfx_arid27_len_err : 1 ;
        unsigned int dfx_arid28_len_err : 1 ;
        unsigned int dfx_arid29_len_err : 1 ;
        unsigned int dfx_arid30_len_err : 1 ;
        unsigned int dfx_arid31_len_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_LEN_0ERR;
typedef union {
    struct {
        unsigned int dfx_arid32_len_err : 1 ;
        unsigned int dfx_arid33_len_err : 1 ;
        unsigned int dfx_arid34_len_err : 1 ;
        unsigned int dfx_arid35_len_err : 1 ;
        unsigned int dfx_arid36_len_err : 1 ;
        unsigned int dfx_arid37_len_err : 1 ;
        unsigned int dfx_arid38_len_err : 1 ;
        unsigned int dfx_arid39_len_err : 1 ;
        unsigned int dfx_arid40_len_err : 1 ;
        unsigned int dfx_arid41_len_err : 1 ;
        unsigned int dfx_arid42_len_err : 1 ;
        unsigned int dfx_arid43_len_err : 1 ;
        unsigned int dfx_arid44_len_err : 1 ;
        unsigned int dfx_arid45_len_err : 1 ;
        unsigned int dfx_arid46_len_err : 1 ;
        unsigned int dfx_arid47_len_err : 1 ;
        unsigned int dfx_arid48_len_err : 1 ;
        unsigned int dfx_arid49_len_err : 1 ;
        unsigned int dfx_arid50_len_err : 1 ;
        unsigned int dfx_arid51_len_err : 1 ;
        unsigned int dfx_arid52_len_err : 1 ;
        unsigned int dfx_arid53_len_err : 1 ;
        unsigned int dfx_arid54_len_err : 1 ;
        unsigned int dfx_arid55_len_err : 1 ;
        unsigned int dfx_arid56_len_err : 1 ;
        unsigned int dfx_arid57_len_err : 1 ;
        unsigned int dfx_arid58_len_err : 1 ;
        unsigned int dfx_arid59_len_err : 1 ;
        unsigned int dfx_arid60_len_err : 1 ;
        unsigned int dfx_arid61_len_err : 1 ;
        unsigned int dfx_arid62_len_err : 1 ;
        unsigned int dfx_arid63_len_err : 1 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_LEN_1ERR;
typedef union {
    struct {
        unsigned int dfx_arid64_len_err : 1 ;
        unsigned int dfx_arid65_len_err : 1 ;
        unsigned int dfx_arid66_len_err : 1 ;
        unsigned int dfx_arid67_len_err : 1 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ARID_LEN_2ERR;
typedef union {
    struct {
        unsigned int dfx_rresp_err : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int dfx_bresp_err : 1 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_RESP_ERR;
typedef union {
    struct {
        unsigned int dfx_err_rresp : 2 ;
        unsigned int dfx_err_bresp : 2 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_RESP;
typedef union {
    struct {
        unsigned int dfx_arlen_err : 1 ;
        unsigned int reserved_1 : 3 ;
        unsigned int dfx_awlen_err : 1 ;
        unsigned int reserved_0 : 27 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_LEN_ERR;
typedef union {
    struct {
        unsigned int dfx_err_arlen : 4 ;
        unsigned int dfx_err_awlen : 4 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_ERR_LEN;
typedef union {
    struct {
        unsigned int dfx_2rid_flag : 4 ;
        unsigned int reserved_0 : 28 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_2RID_FLAG;
typedef union {
    struct {
        unsigned int dfx_wid_flag : 8 ;
        unsigned int reserved_0 : 24 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_WID_FLAG;
typedef union {
    struct {
        unsigned int arvalid : 1 ;
        unsigned int arready : 1 ;
        unsigned int rvalid : 1 ;
        unsigned int rready : 1 ;
        unsigned int awvalid : 1 ;
        unsigned int awready : 1 ;
        unsigned int wvalid : 1 ;
        unsigned int wready : 1 ;
        unsigned int bvalid : 1 ;
        unsigned int bready : 1 ;
        unsigned int reserved_0 : 22 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_AXI_ST;
typedef union {
    struct {
        unsigned int axi_soft_rst_req : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_SOFT_RST_REQ;
typedef union {
    struct {
        unsigned int axi_soft_rst_ack : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_SOFT_RST_ACK;
typedef union {
    struct {
        unsigned int axi_soft_rst_force_req_ack : 1 ;
        unsigned int reserved_0 : 31 ;
    } bits;
    unsigned int u32;
} U_AXIDFX_SOFT_RST_FORCE_REQ_ACK;
typedef struct {
    volatile U_VEDU_VCPI_INTMASK VEDU_VCPI_INTMASK;
    volatile U_VEDU_VCPI_INTCLR VEDU_VCPI_INTCLR;
    volatile U_VEDU_VCPI_START VEDU_VCPI_START;
    volatile U_VEDU_BUS_IDLE_REQ VEDU_BUS_IDLE_REQ;
    volatile unsigned int VEDU_VCPI_FRAMENO;
    volatile unsigned int VEDU_VCPI_TIMEOUT;
    volatile U_VEDU_VCPI_MODE VEDU_VCPI_MODE;
    volatile unsigned int VEDU_RESERVED_00;
    volatile unsigned int VEDU_RESERVED_01;
    volatile unsigned int VEDU_RESERVED_02;
    volatile unsigned int VEDU_RESERVED_03;
    volatile unsigned int VEDU_RESERVED_04;
    volatile U_VEDU_EMAR_SCRAMBLE_TYPE VEDU_EMAR_SCRAMBLE_TYPE;
    volatile unsigned int VEDU_VCPI_LLILD_ADDR_L;
    volatile unsigned int VEDU_VCPI_LLILD_ADDR_H;
    volatile U_VEDU_VCPI_RC_ENABLE VEDU_VCPI_RC_ENABLE;
    volatile U_VEDU_VLCST_PTBITS_EN VEDU_VLCST_PTBITS_EN;
    volatile unsigned int VEDU_VLCST_PTBITS;
    volatile U_VEDU_PPFD_ST_CFG VEDU_PPFD_ST_CFG;
    volatile U_VEDU_VCPI_QPCFG VEDU_VCPI_QPCFG;
    volatile U_VEDU_QPG_MAX_MIN_QP VEDU_QPG_MAX_MIN_QP;
    volatile U_VEDU_QPG_SMART_REG VEDU_QPG_SMART_REG;
    volatile U_VEDU_QPG_ROW_TARGET_BITS VEDU_QPG_ROW_TARGET_BITS;
    volatile U_VEDU_QPG_AVERAGE_LCU_BITS VEDU_QPG_AVERAGE_LCU_BITS;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0 VEDU_QPG_CU_QP_DELTA_THRESH_REG0;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1 VEDU_QPG_CU_QP_DELTA_THRESH_REG1;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2 VEDU_QPG_CU_QP_DELTA_THRESH_REG2;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3 VEDU_QPG_CU_QP_DELTA_THRESH_REG3;
    volatile U_VEDU_QPG_DELTA_LEVEL VEDU_QPG_DELTA_LEVEL;
    volatile U_VEDU_QPG_MADI_SWITCH_THR VEDU_QPG_MADI_SWITCH_THR;
    volatile U_VEDU_QPG_CURR_SAD_EN VEDU_QPG_CURR_SAD_EN;
    volatile U_VEDU_QPG_CURR_SAD_LEVEL VEDU_QPG_CURR_SAD_LEVEL;
    volatile U_VEDU_QPG_CURR_SAD_THRESH0 VEDU_QPG_CURR_SAD_THRESH0;
    volatile U_VEDU_QPG_CURR_SAD_THRESH1 VEDU_QPG_CURR_SAD_THRESH1;
    volatile U_VEDU_QPG_CURR_SAD_THRESH2 VEDU_QPG_CURR_SAD_THRESH2;
    volatile U_VEDU_QPG_CURR_SAD_THRESH3 VEDU_QPG_CURR_SAD_THRESH3;
    volatile U_VEDU_LUMA_RC VEDU_LUMA_RC;
    volatile U_VEDU_LUMA_LEVEL VEDU_LUMA_LEVEL;
    volatile U_VEDU_LUMA_THRESH0 VEDU_LUMA_THRESH0;
    volatile U_VEDU_LUMA_THRESH1 VEDU_LUMA_THRESH1;
    volatile U_VEDU_LUMA_THRESH2 VEDU_LUMA_THRESH2;
    volatile U_VEDU_LUMA_THRESH3 VEDU_LUMA_THRESH3;
    volatile U_VEDU_VCPI_BG_ENABLE VEDU_VCPI_BG_ENABLE;
    volatile U_VEDU_VCPI_BG_FLT_PARA0 VEDU_VCPI_BG_FLT_PARA0;
    volatile U_VEDU_VCPI_BG_FLT_PARA1 VEDU_VCPI_BG_FLT_PARA1;
    volatile U_VEDU_VCPI_BG_FLT_PARA2 VEDU_VCPI_BG_FLT_PARA2;
    volatile U_VEDU_VCPI_BG_THR0 VEDU_VCPI_BG_THR0;
    volatile U_VEDU_VCPI_BG_THR1 VEDU_VCPI_BG_THR1;
    volatile U_VEDU_VCPI_BG_STRIDE VEDU_VCPI_BG_STRIDE;
    volatile U_VEDU_VCPI_BG_EXT_STRIDE VEDU_VCPI_BG_EXT_STRIDE;
    volatile unsigned int VEDU_VCPI_BGL_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGL_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGC_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGC_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGL_EXT_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGL_EXT_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGC_EXT_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGC_EXT_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGINF_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGINF_ADDR_H;
    volatile U_VEDU_VCPI_OUTSTD VEDU_VCPI_OUTSTD;
    volatile U_VEDU_VCTRL_LCU_BASELINE VEDU_VCTRL_LCU_BASELINE;
    volatile unsigned int VEDU_VCPI_SPMEM_CTRL_0;
    volatile U_VEDU_VCPI_SPMEM_CTRL_1 VEDU_VCPI_SPMEM_CTRL_1;
    volatile U_VEDU_VCPI_SPMEM_CTRL_2 VEDU_VCPI_SPMEM_CTRL_2;
    volatile unsigned int VEDU_VCPI_TPMEM_CTRL_0;
    volatile unsigned int VEDU_VCPI_TPMEM_CTRL_1;
    volatile unsigned int VEDU_RESERVED_06[61];
    volatile U_VEDU_CURLD_GCFG VEDU_CURLD_GCFG;
    volatile U_VEDU_VCPI_OSD_ENABLE VEDU_VCPI_OSD_ENABLE;
    volatile U_VEDU_VCPI_STRFMT VEDU_VCPI_STRFMT;
    volatile U_VEDU_VCPI_INTRA_INTER_CU_EN VEDU_VCPI_INTRA_INTER_CU_EN;
    volatile U_VEDU_VCPI_CROSS_TILE_SLC VEDU_VCPI_CROSS_TILE_SLC;
    volatile U_VEDU_VCPI_MULTISLC VEDU_VCPI_MULTISLC;
    volatile U_VEDU_VCTRL_LCU_TARGET_BIT VEDU_VCTRL_LCU_TARGET_BIT;
    volatile U_VEDU_VCPI_SW_L0_SIZE VEDU_VCPI_SW_L0_SIZE;
    volatile U_VEDU_VCPI_SW_L1_SIZE VEDU_VCPI_SW_L1_SIZE;
    volatile U_VEDU_VCPI_I_SLC_INSERT VEDU_VCPI_I_SLC_INSERT;
    volatile U_VEDU_PME_SAFE_CFG VEDU_PME_SAFE_CFG;
    volatile U_VEDU_PME_IBLK_REFRESH VEDU_PME_IBLK_REFRESH;
    volatile U_VEDU_PME_IBLK_REFRESH_NUM VEDU_PME_IBLK_REFRESH_NUM;
    volatile U_VEDU_INTRA_CHNL4_ANG_0EN VEDU_INTRA_CHNL4_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL4_ANG_1EN VEDU_INTRA_CHNL4_ANG_1EN;
    volatile U_VEDU_INTRA_CHNL8_ANG_0EN VEDU_INTRA_CHNL8_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL8_ANG_1EN VEDU_INTRA_CHNL8_ANG_1EN;
    volatile U_VEDU_INTRA_CHNL16_ANG_0EN VEDU_INTRA_CHNL16_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL16_ANG_1EN VEDU_INTRA_CHNL16_ANG_1EN;
    volatile U_VEDU_INTRA_CHNL32_ANG_0EN VEDU_INTRA_CHNL32_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL32_ANG_1EN VEDU_INTRA_CHNL32_ANG_1EN;
    volatile U_VEDU_PACK_CU_PARAMETER VEDU_PACK_CU_PARAMETER;
    volatile U_VEDU_PACK_PCM_PARAMETER VEDU_PACK_PCM_PARAMETER;
    volatile U_VEDU_QPG_READLINE_INTERVAL VEDU_QPG_READLINE_INTERVAL;
    volatile U_VEDU_PMV_READLINE_INTERVAL VEDU_PMV_READLINE_INTERVAL;
    volatile U_VEDU_RGB2YUV_COEF_P0 VEDU_RGB2YUV_COEF_P0;
    volatile U_VEDU_RGB2YUV_COEF_P1 VEDU_RGB2YUV_COEF_P1;
    volatile U_VEDU_RGB2YUV_COEF_P2 VEDU_RGB2YUV_COEF_P2;
    volatile U_VEDU_RGB2YUV_OFFSET VEDU_RGB2YUV_OFFSET;
    volatile U_VEDU_RGB2YUV_CLIP_THR_Y VEDU_RGB2YUV_CLIP_THR_Y;
    volatile U_VEDU_RGB2YUV_CLIP_THR_C VEDU_RGB2YUV_CLIP_THR_C;
    volatile U_VEDU_RGB2YUV_SHIFT_WIDTH VEDU_RGB2YUV_SHIFT_WIDTH;
    volatile U_VEDU_VCPI_OSD_POS_0 VEDU_VCPI_OSD_POS_0;
    volatile U_VEDU_VCPI_OSD_POS_1 VEDU_VCPI_OSD_POS_1;
    volatile unsigned int VEDU_VCPI_OSD_POS_2;
    volatile unsigned int VEDU_VCPI_OSD_POS_3;
    volatile U_VEDU_VCPI_OSD_POS_4 VEDU_VCPI_OSD_POS_4;
    volatile U_VEDU_VCPI_OSD_POS_5 VEDU_VCPI_OSD_POS_5;
    volatile unsigned int VEDU_VCPI_OSD_POS_6;
    volatile unsigned int VEDU_VCPI_OSD_POS_7;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_0;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_1;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_2;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_3;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_4;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_5;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_6;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_7;
    volatile unsigned int VEDU_VCPI_OSD_LAYERID;
    volatile U_VEDU_VCPI_OSD_QP0 VEDU_VCPI_OSD_QP0;
    volatile U_VEDU_VCPI_OSD_QP1 VEDU_VCPI_OSD_QP1;
    volatile U_VEDU_CURLD_OSD01_ALPHA VEDU_CURLD_OSD01_ALPHA;
    volatile U_VEDU_CURLD_OSD23_ALPHA VEDU_CURLD_OSD23_ALPHA;
    volatile U_VEDU_CURLD_OSD45_ALPHA VEDU_CURLD_OSD45_ALPHA;
    volatile U_VEDU_CURLD_OSD67_ALPHA VEDU_CURLD_OSD67_ALPHA;
    volatile unsigned int VEDU_CURLD_OSD_GALPHA0;
    volatile unsigned int VEDU_CURLD_OSD_GALPHA1;
    volatile U_VEDU_TBLDST_READLINE_INTERVAL VEDU_TBLDST_READLINE_INTERVAL;
    volatile unsigned int VEDU_RESERVED_07[6];
    volatile unsigned int VEDU_CURLD_OSD0_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD0_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD1_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD1_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD2_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD2_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD3_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD3_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD4_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD4_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD5_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD5_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD6_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD6_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD7_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD7_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD01_STRIDE;
    volatile unsigned int VEDU_CURLD_OSD23_STRIDE;
    volatile unsigned int VEDU_CURLD_OSD45_STRIDE;
    volatile unsigned int VEDU_CURLD_OSD67_STRIDE;
    volatile U_VEDU_NEW_CFG0 VEDU_NEW_CFG0;
    volatile unsigned int VEDU_RESERVED_08[11];
    volatile U_VEDU_VCTRL_ROI_CFG0 VEDU_VCTRL_ROI_CFG0;
    volatile U_VEDU_VCTRL_ROI_CFG1 VEDU_VCTRL_ROI_CFG1;
    volatile U_VEDU_VCTRL_ROI_CFG2 VEDU_VCTRL_ROI_CFG2;
    volatile U_VEDU_VCTRL_ROI_SIZE_0 VEDU_VCTRL_ROI_SIZE_0;
    volatile U_VEDU_VCTRL_ROI_SIZE_1 VEDU_VCTRL_ROI_SIZE_1;
    volatile U_VEDU_VCTRL_ROI_SIZE_2 VEDU_VCTRL_ROI_SIZE_2;
    volatile U_VEDU_VCTRL_ROI_SIZE_3 VEDU_VCTRL_ROI_SIZE_3;
    volatile U_VEDU_VCTRL_ROI_SIZE_4 VEDU_VCTRL_ROI_SIZE_4;
    volatile U_VEDU_VCTRL_ROI_SIZE_5 VEDU_VCTRL_ROI_SIZE_5;
    volatile U_VEDU_VCTRL_ROI_SIZE_6 VEDU_VCTRL_ROI_SIZE_6;
    volatile U_VEDU_VCTRL_ROI_SIZE_7 VEDU_VCTRL_ROI_SIZE_7;
    volatile U_VEDU_VCTRL_ROI_START_0 VEDU_VCTRL_ROI_START_0;
    volatile U_VEDU_VCTRL_ROI_START_1 VEDU_VCTRL_ROI_START_1;
    volatile U_VEDU_VCTRL_ROI_START_2 VEDU_VCTRL_ROI_START_2;
    volatile U_VEDU_VCTRL_ROI_START_3 VEDU_VCTRL_ROI_START_3;
    volatile U_VEDU_VCTRL_ROI_START_4 VEDU_VCTRL_ROI_START_4;
    volatile U_VEDU_VCTRL_ROI_START_5 VEDU_VCTRL_ROI_START_5;
    volatile U_VEDU_VCTRL_ROI_START_6 VEDU_VCTRL_ROI_START_6;
    volatile U_VEDU_VCTRL_ROI_START_7 VEDU_VCTRL_ROI_START_7;
    volatile unsigned int VEDU_RESERVED_09[13];
    volatile U_VEDU_TILE_MODE VEDU_TILE_MODE;
    volatile U_VEDU_VCPI_PICSIZE_PIX VEDU_VCPI_PICSIZE_PIX;
    volatile U_VEDU_TILE_POS VEDU_TILE_POS;
    volatile U_VEDU_VCPI_TILE_SIZE VEDU_VCPI_TILE_SIZE;
    volatile U_VEDU_VCPI_VLC_CONFIG VEDU_VCPI_VLC_CONFIG;
    volatile U_VEDU_VCPI_REF_FLAG VEDU_VCPI_REF_FLAG;
    volatile U_VEDU_PMV_TMV_EN VEDU_PMV_TMV_EN;
    volatile U_VEDU_VCPI_TMV_LOAD VEDU_VCPI_TMV_LOAD;
    volatile unsigned int VEDU_PMV_POC_0;
    volatile unsigned int VEDU_PMV_POC_1;
    volatile unsigned int VEDU_PMV_POC_2;
    volatile unsigned int VEDU_PMV_POC_3;
    volatile unsigned int VEDU_PMV_POC_4;
    volatile unsigned int VEDU_PMV_POC_5;
    volatile U_VEDU_CABAC_GLB_CFG VEDU_CABAC_GLB_CFG;
    volatile U_VEDU_CABAC_SLCHDR_SIZE VEDU_CABAC_SLCHDR_SIZE;
    volatile U_VEDU_CABAC_SLCHDR_PART1 VEDU_CABAC_SLCHDR_PART1;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG1;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG2;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG3;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG4;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG5;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG6;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG7;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG8;
    volatile U_VEDU_CABAC_SLCHDR_SIZE_I VEDU_CABAC_SLCHDR_SIZE_I;
    volatile U_VEDU_CABAC_SLCHDR_PART1_I VEDU_CABAC_SLCHDR_PART1_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG1_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG2_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG3_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG4_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG5_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG6_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG7_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG8_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM0;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM1;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM2;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM3;
    volatile unsigned int VEDU_VLC_REORDERSTRM0;
    volatile unsigned int VEDU_VLC_REORDERSTRM1;
    volatile unsigned int VEDU_VLC_MARKINGSTRM0;
    volatile unsigned int VEDU_VLC_MARKINGSTRM1;
    volatile U_VEDU_VLC_SLCHDRPARA VEDU_VLC_SLCHDRPARA;
    volatile U_VEDU_VLC_SVC VEDU_VLC_SVC;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM0_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM1_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM2_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM3_I;
    volatile unsigned int VEDU_VLC_REORDERSTRM0_I;
    volatile unsigned int VEDU_VLC_REORDERSTRM1_I;
    volatile unsigned int VEDU_VLC_MARKINGSTRM0_I;
    volatile unsigned int VEDU_VLC_MARKINGSTRM1_I;
    volatile U_VEDU_VLC_SLCHDRPARA_I VEDU_VLC_SLCHDRPARA_I;
    volatile unsigned int VEDU_VLCST_STRMBUFLEN;
    volatile unsigned int VEDU_VLCST_SLC_CFG0;
    volatile unsigned int VEDU_VLCST_SLC_CFG1;
    volatile unsigned int VEDU_VLCST_SLC_CFG2;
    volatile unsigned int VEDU_VLCST_SLC_CFG3;
    volatile unsigned int VEDU_ICE_V2R1_SEG_256X1_DCMP_CTRL;
    volatile U_VEDU_ICE_CMC_MODE_CFG0 VEDU_ICE_CMC_MODE_CFG0;
    volatile U_VEDU_ICE_CMC_MODE_CFG1 VEDU_ICE_CMC_MODE_CFG1;
    volatile unsigned int VEDU_RESERVED_10[2];
    volatile U_VEDU_QPG_LOWLUMA VEDU_QPG_LOWLUMA;
    volatile U_VEDU_QPG_HEDGE VEDU_QPG_HEDGE;
    volatile U_VEDU_QPG_HEDGE_MOVE VEDU_QPG_HEDGE_MOVE;
    volatile U_VEDU_QPG_SKIN VEDU_QPG_SKIN;
    volatile U_VEDU_QPG_INTRA_DET VEDU_QPG_INTRA_DET;
    volatile unsigned int VEDU_QPG_H264_SMOOTH;
    volatile U_VEDU_QPG_CU32_DELTA VEDU_QPG_CU32_DELTA;
    volatile U_VEDU_QPG_LAMBDA_MODE VEDU_QPG_LAMBDA_MODE;
    volatile U_VEDU_QPG_QP_RESTRAIN VEDU_QPG_QP_RESTRAIN;
    volatile unsigned int VEDU_QPG_CU_MIN_SAD_THRESH_0;
    volatile U_VEDU_QPG_CU_MIN_SAD_REG VEDU_QPG_CU_MIN_SAD_REG;
    volatile U_VEDU_QPG_FLAT_REGION VEDU_QPG_FLAT_REGION;
    volatile U_VEDU_QPG_RES_COEF VEDU_QPG_RES_COEF;
    volatile U_VEDU_TILE_RC VEDU_TILE_RC;
    volatile U_VEDU_CHROMA_PROTECT VEDU_CHROMA_PROTECT;
    volatile U_VEDU_PME_QPG_RC_THR0 VEDU_PME_QPG_RC_THR0;
    volatile U_VEDU_PME_QPG_RC_THR1 VEDU_PME_QPG_RC_THR1;
    volatile U_VEDU_PME_LOW_LUMA_THR VEDU_PME_LOW_LUMA_THR;
    volatile U_VEDU_PME_CHROMA_FLAT VEDU_PME_CHROMA_FLAT;
    volatile U_VEDU_PME_LUMA_FLAT VEDU_PME_LUMA_FLAT;
    volatile U_VEDU_PME_MADI_FLAT VEDU_PME_MADI_FLAT;
    volatile U_VEDU_VLCST_DESCRIPTOR VEDU_VLCST_DESCRIPTOR;
    volatile unsigned int VEDU_PPFD_ST_LEN0;
    volatile U_VEDU_CURLD_CLIP_LUMA VEDU_CURLD_CLIP_LUMA;
    volatile U_VEDU_CURLD_CLIP_CHROMA VEDU_CURLD_CLIP_CHROMA;
    volatile U_VEDU_TQITQ_DEADZONE VEDU_TQITQ_DEADZONE;
    volatile U_VEDU_VCPI_PME_PARAM VEDU_VCPI_PME_PARAM;
    volatile U_VEDU_VCPI_PIC_STRONG_EN VEDU_VCPI_PIC_STRONG_EN;
    volatile U_VEDU_VCPI_PINTRA_THRESH0 VEDU_VCPI_PINTRA_THRESH0;
    volatile U_VEDU_VCPI_PINTRA_THRESH1 VEDU_VCPI_PINTRA_THRESH1;
    volatile U_VEDU_VCPI_PINTRA_THRESH2 VEDU_VCPI_PINTRA_THRESH2;
    volatile U_VEDU_VCPI_INTRA32_LOW_POWER VEDU_VCPI_INTRA32_LOW_POWER;
    volatile U_VEDU_VCPI_INTRA16_LOW_POWER VEDU_VCPI_INTRA16_LOW_POWER;
    volatile U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM VEDU_VCPI_INTRA_REDUCE_RDO_NUM;
    volatile U_VEDU_VCPI_NOFORCEZERO VEDU_VCPI_NOFORCEZERO;
    volatile U_VEDU_PME_SKIP_LARGE_RES VEDU_PME_SKIP_LARGE_RES;
    volatile U_VEDU_PME_SKIN_SAD_THR VEDU_PME_SKIN_SAD_THR;
    volatile U_VEDU_VCTRL_NM_ACOFFSET_DENOISE VEDU_VCTRL_NM_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_NM_ENGTHR_DENOISE VEDU_VCTRL_NM_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_NM_TU8_DENOISE VEDU_VCTRL_NM_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SK_ACOFFSET_DENOISE VEDU_VCTRL_SK_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SK_ENGTHR_DENOISE VEDU_VCTRL_SK_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SK_TU8_DENOISE VEDU_VCTRL_SK_TU8_DENOISE;
    volatile U_VEDU_VCTRL_ST_ACOFFSET_DENOISE VEDU_VCTRL_ST_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_ST_ENGTHR_DENOISE VEDU_VCTRL_ST_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_ST_TU8_DENOISE VEDU_VCTRL_ST_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SE_ACOFFSET_DENOISE VEDU_VCTRL_SE_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SE_ENGTHR_DENOISE VEDU_VCTRL_SE_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SE_TU8_DENOISE VEDU_VCTRL_SE_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SELM_ACOFFSET_DENOISE VEDU_VCTRL_SELM_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SELM_ENGTHR_DENOISE VEDU_VCTRL_SELM_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SELM_TU8_DENOISE VEDU_VCTRL_SELM_TU8_DENOISE;
    volatile U_VEDU_VCTRL_WS_ACOFFSET_DENOISE VEDU_VCTRL_WS_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_WS_ENGTHR_DENOISE VEDU_VCTRL_WS_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_WS_TU8_DENOISE VEDU_VCTRL_WS_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SSSE_ACOFFSET_DENOISE VEDU_VCTRL_SSSE_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SSSE_ENGTHR_DENOISE VEDU_VCTRL_SSSE_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SSSE_TU8_DENOISE VEDU_VCTRL_SSSE_TU8_DENOISE;
    volatile unsigned int VEDU_RESERVED_11[6];
    volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_0 VEDU_VCTRL_INTRA_RDO_FACTOR_0;
    volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_1 VEDU_VCTRL_INTRA_RDO_FACTOR_1;
    volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_2 VEDU_VCTRL_INTRA_RDO_FACTOR_2;
    volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_0 VEDU_VCTRL_MRG_RDO_FACTOR_0;
    volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_1 VEDU_VCTRL_MRG_RDO_FACTOR_1;
    volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_2 VEDU_VCTRL_MRG_RDO_FACTOR_2;
    volatile U_VEDU_VCTRL_FME_RDO_FACTOR_0 VEDU_VCTRL_FME_RDO_FACTOR_0;
    volatile U_VEDU_VCTRL_FME_RDO_FACTOR_1 VEDU_VCTRL_FME_RDO_FACTOR_1;
    volatile U_VEDU_VCTRL_FME_RDO_FACTOR_2 VEDU_VCTRL_FME_RDO_FACTOR_2;
    volatile U_VEDU_PME_NEW_COST VEDU_PME_NEW_COST;
    volatile U_VEDU_PME_COST_OFFSET VEDU_PME_COST_OFFSET;
    volatile U_VEDU_PME_ADJUST_PMEMV_H264 VEDU_PME_ADJUST_PMEMV_H264;
    volatile U_VEDU_PME_INTRABLK_DET VEDU_PME_INTRABLK_DET;
    volatile U_VEDU_PME_INTRABLK_DET_THR VEDU_PME_INTRABLK_DET_THR;
    volatile U_VEDU_PME_SKIN_THR VEDU_PME_SKIN_THR;
    volatile U_VEDU_PME_STRONG_EDGE VEDU_PME_STRONG_EDGE;
    volatile U_VEDU_PME_LARGE_MOVE_THR VEDU_PME_LARGE_MOVE_THR;
    volatile U_VEDU_PME_INTER_STRONG_EDGE VEDU_PME_INTER_STRONG_EDGE;
    volatile U_VEDU_PME_CHROMA_STRONG_EDGE VEDU_PME_CHROMA_STRONG_EDGE;
    volatile U_VEDU_CHROMA_SAD_THR VEDU_CHROMA_SAD_THR;
    volatile U_VEDU_CHROMA_FG_THR VEDU_CHROMA_FG_THR;
    volatile U_VEDU_CHROMA_BG_THR VEDU_CHROMA_BG_THR;
    volatile U_VEDU_CHROMA_SUM_FG_THR VEDU_CHROMA_SUM_FG_THR;
    volatile U_VEDU_CHROMA_SUM_BG_THR VEDU_CHROMA_SUM_BG_THR;
    volatile U_VEDU_CHROMA_FG_COUNT_THR VEDU_CHROMA_FG_COUNT_THR;
    volatile U_VEDU_CHROMA_BG_COUNT_THR VEDU_CHROMA_BG_COUNT_THR;
    volatile U_VEDU_PME_MOVE_SCENE_THR VEDU_PME_MOVE_SCENE_THR;
    volatile U_VEDU_PME_NEW_MADI_TH VEDU_PME_NEW_MADI_TH;
    volatile U_VEDU_PME_NEW_LAMBDA VEDU_PME_NEW_LAMBDA;
    volatile U_VEDU_VCPI_DBLKCFG VEDU_VCPI_DBLKCFG;
    volatile U_VEDU_FME_BIAS_COST0 VEDU_FME_BIAS_COST0;
    volatile U_VEDU_FME_BIAS_COST1 VEDU_FME_BIAS_COST1;
    volatile U_VEDU_MRG_BIAS_COST0 VEDU_MRG_BIAS_COST0;
    volatile U_VEDU_MRG_BIAS_COST1 VEDU_MRG_BIAS_COST1;
    volatile U_VEDU_MRG_ABS_OFFSET0 VEDU_MRG_ABS_OFFSET0;
    volatile U_VEDU_MRG_ABS_OFFSET1 VEDU_MRG_ABS_OFFSET1;
    volatile U_VEDU_MRG_ADJ_WEIGHT VEDU_MRG_ADJ_WEIGHT;
    volatile U_VEDU_INTRA_BIT_WEIGHT VEDU_INTRA_BIT_WEIGHT;
    volatile U_VEDU_INTRA_RDO_COST_OFFSET_0 VEDU_INTRA_RDO_COST_OFFSET_0;
    volatile U_VEDU_INTRA_RDO_COST_OFFSET_1 VEDU_INTRA_RDO_COST_OFFSET_1;
    volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_0 VEDU_INTRA_NO_DC_COST_OFFSET_0;
    volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_1 VEDU_INTRA_NO_DC_COST_OFFSET_1;
    volatile U_VEDU_INTRA_RDO_COST_OFFSET_3 VEDU_INTRA_RDO_COST_OFFSET_3;
    volatile U_VEDU_SEL_OFFSET_STRENGTH VEDU_SEL_OFFSET_STRENGTH;
    volatile U_VEDU_SEL_CU32_DC_AC_TH_OFFSET VEDU_SEL_CU32_DC_AC_TH_OFFSET;
    volatile U_VEDU_SEL_CU32_QP_TH VEDU_SEL_CU32_QP_TH;
    volatile U_VEDU_SEL_RES_DC_AC_TH VEDU_SEL_RES_DC_AC_TH;
    volatile unsigned int VEDU_RESERVED_12[17];
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG00 VEDU_QPG_QP_LAMBDA_CTRL_REG00;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG01 VEDU_QPG_QP_LAMBDA_CTRL_REG01;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG02 VEDU_QPG_QP_LAMBDA_CTRL_REG02;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG03 VEDU_QPG_QP_LAMBDA_CTRL_REG03;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG04 VEDU_QPG_QP_LAMBDA_CTRL_REG04;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG05 VEDU_QPG_QP_LAMBDA_CTRL_REG05;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG06 VEDU_QPG_QP_LAMBDA_CTRL_REG06;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG07 VEDU_QPG_QP_LAMBDA_CTRL_REG07;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG08 VEDU_QPG_QP_LAMBDA_CTRL_REG08;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG09 VEDU_QPG_QP_LAMBDA_CTRL_REG09;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG10 VEDU_QPG_QP_LAMBDA_CTRL_REG10;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG11 VEDU_QPG_QP_LAMBDA_CTRL_REG11;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG12 VEDU_QPG_QP_LAMBDA_CTRL_REG12;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG13 VEDU_QPG_QP_LAMBDA_CTRL_REG13;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG14 VEDU_QPG_QP_LAMBDA_CTRL_REG14;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG15 VEDU_QPG_QP_LAMBDA_CTRL_REG15;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG16 VEDU_QPG_QP_LAMBDA_CTRL_REG16;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG17 VEDU_QPG_QP_LAMBDA_CTRL_REG17;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG18 VEDU_QPG_QP_LAMBDA_CTRL_REG18;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG19 VEDU_QPG_QP_LAMBDA_CTRL_REG19;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG20 VEDU_QPG_QP_LAMBDA_CTRL_REG20;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG21 VEDU_QPG_QP_LAMBDA_CTRL_REG21;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG22 VEDU_QPG_QP_LAMBDA_CTRL_REG22;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG23 VEDU_QPG_QP_LAMBDA_CTRL_REG23;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG24 VEDU_QPG_QP_LAMBDA_CTRL_REG24;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG25 VEDU_QPG_QP_LAMBDA_CTRL_REG25;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG26 VEDU_QPG_QP_LAMBDA_CTRL_REG26;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG27 VEDU_QPG_QP_LAMBDA_CTRL_REG27;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG28 VEDU_QPG_QP_LAMBDA_CTRL_REG28;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG29 VEDU_QPG_QP_LAMBDA_CTRL_REG29;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG30 VEDU_QPG_QP_LAMBDA_CTRL_REG30;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG31 VEDU_QPG_QP_LAMBDA_CTRL_REG31;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG32 VEDU_QPG_QP_LAMBDA_CTRL_REG32;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG33 VEDU_QPG_QP_LAMBDA_CTRL_REG33;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG34 VEDU_QPG_QP_LAMBDA_CTRL_REG34;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG35 VEDU_QPG_QP_LAMBDA_CTRL_REG35;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG36 VEDU_QPG_QP_LAMBDA_CTRL_REG36;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG37 VEDU_QPG_QP_LAMBDA_CTRL_REG37;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG38 VEDU_QPG_QP_LAMBDA_CTRL_REG38;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG39 VEDU_QPG_QP_LAMBDA_CTRL_REG39;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG40 VEDU_QPG_QP_LAMBDA_CTRL_REG40;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG41 VEDU_QPG_QP_LAMBDA_CTRL_REG41;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG42 VEDU_QPG_QP_LAMBDA_CTRL_REG42;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG43 VEDU_QPG_QP_LAMBDA_CTRL_REG43;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG44 VEDU_QPG_QP_LAMBDA_CTRL_REG44;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG45 VEDU_QPG_QP_LAMBDA_CTRL_REG45;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG46 VEDU_QPG_QP_LAMBDA_CTRL_REG46;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG47 VEDU_QPG_QP_LAMBDA_CTRL_REG47;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG48 VEDU_QPG_QP_LAMBDA_CTRL_REG48;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG49 VEDU_QPG_QP_LAMBDA_CTRL_REG49;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG50 VEDU_QPG_QP_LAMBDA_CTRL_REG50;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG51 VEDU_QPG_QP_LAMBDA_CTRL_REG51;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG52 VEDU_QPG_QP_LAMBDA_CTRL_REG52;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG53 VEDU_QPG_QP_LAMBDA_CTRL_REG53;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG54 VEDU_QPG_QP_LAMBDA_CTRL_REG54;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG55 VEDU_QPG_QP_LAMBDA_CTRL_REG55;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG56 VEDU_QPG_QP_LAMBDA_CTRL_REG56;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG57 VEDU_QPG_QP_LAMBDA_CTRL_REG57;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG58 VEDU_QPG_QP_LAMBDA_CTRL_REG58;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG59 VEDU_QPG_QP_LAMBDA_CTRL_REG59;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG60 VEDU_QPG_QP_LAMBDA_CTRL_REG60;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG61 VEDU_QPG_QP_LAMBDA_CTRL_REG61;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG62 VEDU_QPG_QP_LAMBDA_CTRL_REG62;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG63 VEDU_QPG_QP_LAMBDA_CTRL_REG63;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG64 VEDU_QPG_QP_LAMBDA_CTRL_REG64;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG65 VEDU_QPG_QP_LAMBDA_CTRL_REG65;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG66 VEDU_QPG_QP_LAMBDA_CTRL_REG66;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG67 VEDU_QPG_QP_LAMBDA_CTRL_REG67;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG68 VEDU_QPG_QP_LAMBDA_CTRL_REG68;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG69 VEDU_QPG_QP_LAMBDA_CTRL_REG69;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG70 VEDU_QPG_QP_LAMBDA_CTRL_REG70;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG71 VEDU_QPG_QP_LAMBDA_CTRL_REG71;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG72 VEDU_QPG_QP_LAMBDA_CTRL_REG72;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG73 VEDU_QPG_QP_LAMBDA_CTRL_REG73;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG74 VEDU_QPG_QP_LAMBDA_CTRL_REG74;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG75 VEDU_QPG_QP_LAMBDA_CTRL_REG75;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG76 VEDU_QPG_QP_LAMBDA_CTRL_REG76;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG77 VEDU_QPG_QP_LAMBDA_CTRL_REG77;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG78 VEDU_QPG_QP_LAMBDA_CTRL_REG78;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG79 VEDU_QPG_QP_LAMBDA_CTRL_REG79;
    volatile U_VEDU_IME_RDOCFG VEDU_IME_RDOCFG;
    volatile U_VEDU_MRG_FORCE_ZERO_EN VEDU_MRG_FORCE_ZERO_EN;
    volatile unsigned int VEDU_VCPI_INTRA8_LOW_POWER;
    volatile U_VEDU_VCPI_LOW_POWER VEDU_VCPI_LOW_POWER;
    volatile U_VEDU_IME_INTER_MODE VEDU_IME_INTER_MODE;
    volatile U_VEDU_VCPI_PRE_JUDGE_EXT_EN VEDU_VCPI_PRE_JUDGE_EXT_EN;
    volatile unsigned int VEDU_RESERVED_13[10];
    volatile U_VEDU_PME_SW_ADAPT_EN VEDU_PME_SW_ADAPT_EN;
    volatile U_VEDU_PME_WINDOW_SIZE0_L0 VEDU_PME_WINDOW_SIZE0_L0;
    volatile U_VEDU_PME_WINDOW_SIZE0_L1 VEDU_PME_WINDOW_SIZE0_L1;
    volatile U_VEDU_PME_SKIP_PRE VEDU_PME_SKIP_PRE;
    volatile U_VEDU_PME_PBLK_PRE1 VEDU_PME_PBLK_PRE1;
    volatile U_VEDU_VCPI_PRE_JUDGE_COST_THR VEDU_VCPI_PRE_JUDGE_COST_THR;
    volatile U_VEDU_VCPI_IBLK_PRE_MV_THR VEDU_VCPI_IBLK_PRE_MV_THR;
    volatile U_VEDU_PME_IBLK_COST_THR VEDU_PME_IBLK_COST_THR;
    volatile U_VEDU_PME_TR_WEIGHTX VEDU_PME_TR_WEIGHTX;
    volatile U_VEDU_PME_TR_WEIGHTY VEDU_PME_TR_WEIGHTY;
    volatile U_VEDU_PME_SR_WEIGHT VEDU_PME_SR_WEIGHT;
    volatile U_VEDU_PME_INTRA_LOWPOW VEDU_PME_INTRA_LOWPOW;
    volatile U_VEDU_IME_FME_LPOW_THR VEDU_IME_FME_LPOW_THR;
    volatile U_VEDU_PME_SKIP_FLAG VEDU_PME_SKIP_FLAG;
    volatile U_VEDU_PME_PSW_LPW VEDU_PME_PSW_LPW;
    volatile U_VEDU_PME_PBLK_PRE2 VEDU_PME_PBLK_PRE2;
    volatile U_VEDU_IME_LAYER3TO2_THR VEDU_IME_LAYER3TO2_THR;
    volatile U_VEDU_IME_LAYER3TO2_THR1 VEDU_IME_LAYER3TO2_THR1;
    volatile U_VEDU_IME_LAYER3TO1_THR VEDU_IME_LAYER3TO1_THR;
    volatile U_VEDU_IME_LAYER3TO1_THR1 VEDU_IME_LAYER3TO1_THR1;
    volatile U_VEDU_FME_PU64_LWP VEDU_FME_PU64_LWP;
    volatile U_VEDU_MRG_FORCE_SKIP_EN VEDU_MRG_FORCE_SKIP_EN;
    volatile U_VEDU_INTRA_CFG VEDU_INTRA_CFG;
    volatile U_VEDU_INTRA_LOW_POW VEDU_INTRA_LOW_POW;
    volatile unsigned int VEDU_RESERVED_14[8];
    volatile unsigned int VEDU_VCPI_TUNLCELL_ADDR_L;
    volatile unsigned int VEDU_VCPI_TUNLCELL_ADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_YADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_YADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_EXT_YADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_EXT_YADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_CADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_CADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_EXT_CADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_EXT_CADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_VADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_VADDR_H;
    volatile unsigned int VEDU_VCPI_YH_ADDR_L;
    volatile unsigned int VEDU_VCPI_YH_ADDR_H;
    volatile unsigned int VEDU_VCPI_CH_ADDR_L;
    volatile unsigned int VEDU_VCPI_CH_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFY_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFY_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFC_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFC_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFYH_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFYH_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFCH_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFCH_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFY_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFY_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFC_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFC_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFYH_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFYH_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFCH_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFCH_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REC_YADDR_L;
    volatile unsigned int VEDU_VCPI_REC_YADDR_H;
    volatile unsigned int VEDU_VCPI_REC_CADDR_L;
    volatile unsigned int VEDU_VCPI_REC_CADDR_H;
    volatile unsigned int VEDU_VCPI_REC_YH_ADDR_L;
    volatile unsigned int VEDU_VCPI_REC_YH_ADDR_H;
    volatile unsigned int VEDU_VCPI_REC_CH_ADDR_L;
    volatile unsigned int VEDU_VCPI_REC_CH_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMELD_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMELD_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMELD_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMELD_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMEST_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEST_ADDR_H;
    volatile unsigned int VEDU_VCPI_NBI_MVST_ADDR_L;
    volatile unsigned int VEDU_VCPI_NBI_MVST_ADDR_H;
    volatile unsigned int VEDU_VCPI_NBI_MVLD_ADDR_L;
    volatile unsigned int VEDU_VCPI_NBI_MVLD_ADDR_H;
    volatile unsigned int VEDU_VCPI_STRMADDR_L;
    volatile unsigned int VEDU_VCPI_STRMADDR_H;
    volatile unsigned int VEDU_VCPI_SWPTRADDR_L;
    volatile unsigned int VEDU_VCPI_SWPTRADDR_H;
    volatile unsigned int VEDU_VCPI_SRPTRADDR_L;
    volatile unsigned int VEDU_VCPI_SRPTRADDR_H;
    volatile unsigned int VEDU_VCPI_PMEINFO_ST_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEINFO_ST_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD0_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD0_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD1_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD1_ADDR_H;
    volatile unsigned int VEDU_VCPI_QPGLD_INF_ADDR_L;
    volatile unsigned int VEDU_VCPI_QPGLD_INF_ADDR_H;
    volatile unsigned int VEDU_LEFT_CELL_RADDR_L;
    volatile unsigned int VEDU_LEFT_CELL_RADDR_H;
    volatile unsigned int VEDU_TOP_CELL_RADDR_L;
    volatile unsigned int VEDU_TOP_CELL_RADDR_H;
    volatile unsigned int VEDU_CURR_CELL_WADDR_L;
    volatile unsigned int VEDU_CURR_CELL_WADDR_H;
    volatile unsigned int VEDU_TOPLEFT_CELL_RADDR_L;
    volatile unsigned int VEDU_TOPLEFT_CELL_RADDR_H;
    volatile unsigned int VEDU_BOT_RC_INFO_WADDR_L;
    volatile unsigned int VEDU_BOT_RC_INFO_WADDR_H;
    volatile unsigned int VEDU_RIGHT_RC_INFO_WADDR_L;
    volatile unsigned int VEDU_RIGHT_RC_INFO_WADDR_H;
    volatile unsigned int VEDU_RIGHT_BS_INFO_WADDR_L;
    volatile unsigned int VEDU_RIGHT_BS_INFO_WADDR_H;
    volatile unsigned int VEDU_LEFT_BS_INFO_RADDR_L;
    volatile unsigned int VEDU_LEFT_BS_INFO_RADDR_H;
    volatile unsigned int VEDU_BOT_TB_INFO_WADDR_L;
    volatile unsigned int VEDU_BOT_TB_INFO_WADDR_H;
    volatile unsigned int VEDU_RIGHT_TB_INFO_WADDR_L;
    volatile unsigned int VEDU_RIGHT_TB_INFO_WADDR_H;
    volatile unsigned int VEDU_BOTRIGHT_TB_INFO_WADDR_L;
    volatile unsigned int VEDU_BOTRIGHT_TB_INFO_WADDR_H;
    volatile unsigned int VEDU_TOP_RC_INFO_RADDR_L;
    volatile unsigned int VEDU_TOP_RC_INFO_RADDR_H;
    volatile unsigned int VEDU_LEFT_RC_INFO_RADDR_L;
    volatile unsigned int VEDU_LEFT_RC_INFO_RADDR_H;
    volatile unsigned int VEDU_TOP_TB_INFO_RADDR_L;
    volatile unsigned int VEDU_TOP_TB_INFO_RADDR_H;
    volatile unsigned int VEDU_LEFT_TB_INFO_RADDR_L;
    volatile unsigned int VEDU_LEFT_TB_INFO_RADDR_H;
    volatile unsigned int VEDU_TOPLEFT_TB_INFO_RADDR_L;
    volatile unsigned int VEDU_TOPLEFT_TB_INFO_RADDR_H;
    volatile unsigned int VEDU_PPFD_ST_ADDR0_L;
    volatile unsigned int VEDU_PPFD_ST_ADDR0_H;
    volatile unsigned int VEDU_SKIPWEIGHT_LD_ADDR_L;
    volatile unsigned int VEDU_SKIPWEIGHT_LD_ADDR_H;
    volatile unsigned int VEDU_RESERVED_15[28];
    volatile U_VEDU_SAO_SSD_AREA0_START VEDU_SAO_SSD_AREA0_START;
    volatile U_VEDU_SAO_SSD_AREA0_END VEDU_SAO_SSD_AREA0_END;
    volatile U_VEDU_SAO_SSD_AREA1_START VEDU_SAO_SSD_AREA1_START;
    volatile U_VEDU_SAO_SSD_AREA1_END VEDU_SAO_SSD_AREA1_END;
    volatile U_VEDU_SAO_SSD_AREA2_START VEDU_SAO_SSD_AREA2_START;
    volatile U_VEDU_SAO_SSD_AREA2_END VEDU_SAO_SSD_AREA2_END;
    volatile U_VEDU_SAO_SSD_AREA3_START VEDU_SAO_SSD_AREA3_START;
    volatile U_VEDU_SAO_SSD_AREA3_END VEDU_SAO_SSD_AREA3_END;
    volatile U_VEDU_SAO_SSD_AREA4_START VEDU_SAO_SSD_AREA4_START;
    volatile U_VEDU_SAO_SSD_AREA4_END VEDU_SAO_SSD_AREA4_END;
    volatile U_VEDU_SAO_SSD_AREA5_START VEDU_SAO_SSD_AREA5_START;
    volatile U_VEDU_SAO_SSD_AREA5_END VEDU_SAO_SSD_AREA5_END;
    volatile U_VEDU_SAO_SSD_AREA6_START VEDU_SAO_SSD_AREA6_START;
    volatile U_VEDU_SAO_SSD_AREA6_END VEDU_SAO_SSD_AREA6_END;
    volatile U_VEDU_SAO_SSD_AREA7_START VEDU_SAO_SSD_AREA7_START;
    volatile U_VEDU_SAO_SSD_AREA7_END VEDU_SAO_SSD_AREA7_END;
    volatile unsigned int VEDU_RESERVED_16[16];
    volatile U_VEDU_VCPI_STRIDE VEDU_VCPI_STRIDE;
    volatile U_VEDU_VCPI_EXT_STRIDE VEDU_VCPI_EXT_STRIDE;
    volatile U_VEDU_VCPI_REF_L0_STRIDE VEDU_VCPI_REF_L0_STRIDE;
    volatile U_VEDU_VCPI_REF_L1_STRIDE VEDU_VCPI_REF_L1_STRIDE;
    volatile U_VEDU_VCPI_REC_STRIDE VEDU_VCPI_REC_STRIDE;
    volatile unsigned int VEDU_VCPI_PMEST_STRIDE;
    volatile unsigned int VEDU_VCPI_PMELD_STRIDE;
    volatile unsigned int VEDU_RESERVED_17[57];
    volatile U_VEDU_GHDR_CTRL VEDU_GHDR_CTRL;
    volatile U_VEDU_GHDR_DEGAMMA_CTRL VEDU_GHDR_DEGAMMA_CTRL;
    volatile U_VEDU_GHDR_DEGAMMA_STEP VEDU_GHDR_DEGAMMA_STEP;
    volatile U_VEDU_GHDR_DEGAMMA_POS1 VEDU_GHDR_DEGAMMA_POS1;
    volatile U_VEDU_GHDR_DEGAMMA_POS2 VEDU_GHDR_DEGAMMA_POS2;
    volatile U_VEDU_GHDR_DEGAMMA_NUM VEDU_GHDR_DEGAMMA_NUM;
    volatile U_VEDU_GHDR_GAMUT_CTRL VEDU_GHDR_GAMUT_CTRL;
    volatile U_VEDU_GHDR_GAMUT_COEF00 VEDU_GHDR_GAMUT_COEF00;
    volatile U_VEDU_GHDR_GAMUT_COEF01 VEDU_GHDR_GAMUT_COEF01;
    volatile U_VEDU_GHDR_GAMUT_COEF02 VEDU_GHDR_GAMUT_COEF02;
    volatile U_VEDU_GHDR_GAMUT_COEF10 VEDU_GHDR_GAMUT_COEF10;
    volatile U_VEDU_GHDR_GAMUT_COEF11 VEDU_GHDR_GAMUT_COEF11;
    volatile U_VEDU_GHDR_GAMUT_COEF12 VEDU_GHDR_GAMUT_COEF12;
    volatile U_VEDU_GHDR_GAMUT_COEF20 VEDU_GHDR_GAMUT_COEF20;
    volatile U_VEDU_GHDR_GAMUT_COEF21 VEDU_GHDR_GAMUT_COEF21;
    volatile U_VEDU_GHDR_GAMUT_COEF22 VEDU_GHDR_GAMUT_COEF22;
    volatile U_VEDU_GHDR_GAMUT_SCALE VEDU_GHDR_GAMUT_SCALE;
    volatile U_VEDU_GHDR_GAMUT_CLIP_MIN VEDU_GHDR_GAMUT_CLIP_MIN;
    volatile U_VEDU_GHDR_GAMUT_CLIP_MAX VEDU_GHDR_GAMUT_CLIP_MAX;
    volatile U_VEDU_GHDR_TONEMAP_CTRL VEDU_GHDR_TONEMAP_CTRL;
    volatile U_VEDU_GHDR_TONEMAP_REN VEDU_GHDR_TONEMAP_REN;
    volatile unsigned int VEDU_GHDR_RESERVED0;
    volatile U_VEDU_GHDR_TONEMAP_STEP VEDU_GHDR_TONEMAP_STEP;
    volatile U_VEDU_GHDR_TONEMAP_POS1 VEDU_GHDR_TONEMAP_POS1;
    volatile U_VEDU_GHDR_TONEMAP_POS2 VEDU_GHDR_TONEMAP_POS2;
    volatile U_VEDU_GHDR_TONEMAP_POS3 VEDU_GHDR_TONEMAP_POS3;
    volatile U_VEDU_GHDR_TONEMAP_POS4 VEDU_GHDR_TONEMAP_POS4;
    volatile U_VEDU_GHDR_TONEMAP_NUM VEDU_GHDR_TONEMAP_NUM;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_COEF0 VEDU_GHDR_TONEMAP_LUMA_COEF0;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_COEF1 VEDU_GHDR_TONEMAP_LUMA_COEF1;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_COEF2 VEDU_GHDR_TONEMAP_LUMA_COEF2;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_SCALE VEDU_GHDR_TONEMAP_LUMA_SCALE;
    volatile U_VEDU_GHDR_TONEMAP_COEF_SCALE VEDU_GHDR_TONEMAP_COEF_SCALE;
    volatile U_VEDU_GHDR_TONEMAP_OUT_CLIP_MIN VEDU_GHDR_TONEMAP_OUT_CLIP_MIN;
    volatile U_VEDU_GHDR_TONEMAP_OUT_CLIP_MAX VEDU_GHDR_TONEMAP_OUT_CLIP_MAX;
    volatile U_VEDU_GHDR_GAMMA_CTRL VEDU_GHDR_GAMMA_CTRL;
    volatile U_VEDU_GHDR_GAMMA_REN VEDU_GHDR_GAMMA_REN;
    volatile unsigned int VEDU_GHDR_RESERVED1;
    volatile U_VEDU_GHDR_GAMMA_STEP1 VEDU_GHDR_GAMMA_STEP1;
    volatile U_VEDU_GHDR_GAMMA_STEP2 VEDU_GHDR_GAMMA_STEP2;
    volatile U_VEDU_GHDR_GAMMA_POS1 VEDU_GHDR_GAMMA_POS1;
    volatile U_VEDU_GHDR_GAMMA_POS2 VEDU_GHDR_GAMMA_POS2;
    volatile U_VEDU_GHDR_GAMMA_POS3 VEDU_GHDR_GAMMA_POS3;
    volatile U_VEDU_GHDR_GAMMA_POS4 VEDU_GHDR_GAMMA_POS4;
    volatile U_VEDU_GHDR_GAMMA_NUM1 VEDU_GHDR_GAMMA_NUM1;
    volatile U_VEDU_GHDR_GAMMA_NUM2 VEDU_GHDR_GAMMA_NUM2;
    volatile U_VEDU_GHDR_DITHER_CTRL VEDU_GHDR_DITHER_CTRL;
    volatile U_VEDU_GHDR_DITHER_THR VEDU_GHDR_DITHER_THR;
    volatile U_VEDU_GHDR_DITHER_SED_Y0 VEDU_GHDR_DITHER_SED_Y0;
    volatile U_VEDU_GHDR_DITHER_SED_U0 VEDU_GHDR_DITHER_SED_U0;
    volatile U_VEDU_GHDR_DITHER_SED_V0 VEDU_GHDR_DITHER_SED_V0;
    volatile U_VEDU_GHDR_DITHER_SED_W0 VEDU_GHDR_DITHER_SED_W0;
    volatile U_VEDU_GHDR_DITHER_SED_Y1 VEDU_GHDR_DITHER_SED_Y1;
    volatile U_VEDU_GHDR_DITHER_SED_U1 VEDU_GHDR_DITHER_SED_U1;
    volatile U_VEDU_GHDR_DITHER_SED_V1 VEDU_GHDR_DITHER_SED_V1;
    volatile U_VEDU_GHDR_DITHER_SED_W1 VEDU_GHDR_DITHER_SED_W1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_CTRL VEDU_HIHDR_G_RGB2YUV_CTRL;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF0 VEDU_HIHDR_G_RGB2YUV_COEF0;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF1 VEDU_HIHDR_G_RGB2YUV_COEF1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF2 VEDU_HIHDR_G_RGB2YUV_COEF2;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF3 VEDU_HIHDR_G_RGB2YUV_COEF3;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF4 VEDU_HIHDR_G_RGB2YUV_COEF4;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF5 VEDU_HIHDR_G_RGB2YUV_COEF5;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF6 VEDU_HIHDR_G_RGB2YUV_COEF6;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF7 VEDU_HIHDR_G_RGB2YUV_COEF7;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF8 VEDU_HIHDR_G_RGB2YUV_COEF8;
    volatile U_VEDU_HIHDR_G_RGB2YUV_SCALE2P VEDU_HIHDR_G_RGB2YUV_SCALE2P;
    volatile U_VEDU_HIHDR_G_RGB2YUV_IDC0 VEDU_HIHDR_G_RGB2YUV_IDC0;
    volatile U_VEDU_HIHDR_G_RGB2YUV_IDC1 VEDU_HIHDR_G_RGB2YUV_IDC1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_IDC2 VEDU_HIHDR_G_RGB2YUV_IDC2;
    volatile U_VEDU_HIHDR_G_RGB2YUV_ODC0 VEDU_HIHDR_G_RGB2YUV_ODC0;
    volatile U_VEDU_HIHDR_G_RGB2YUV_ODC1 VEDU_HIHDR_G_RGB2YUV_ODC1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_ODC2 VEDU_HIHDR_G_RGB2YUV_ODC2;
    volatile U_VEDU_HIHDR_G_RGB2YUV_MIN VEDU_HIHDR_G_RGB2YUV_MIN;
    volatile U_VEDU_HIHDR_G_RGB2YUV_MAX VEDU_HIHDR_G_RGB2YUV_MAX;
    volatile unsigned int VEDU_RESERVED_18[21];
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_00_01 VEDU_HIHDR_G_TONEMAP_COEF_00_01;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_02_03 VEDU_HIHDR_G_TONEMAP_COEF_02_03;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_04_05 VEDU_HIHDR_G_TONEMAP_COEF_04_05;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_06_07 VEDU_HIHDR_G_TONEMAP_COEF_06_07;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_08_09 VEDU_HIHDR_G_TONEMAP_COEF_08_09;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_10_11 VEDU_HIHDR_G_TONEMAP_COEF_10_11;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_12_13 VEDU_HIHDR_G_TONEMAP_COEF_12_13;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_14_15 VEDU_HIHDR_G_TONEMAP_COEF_14_15;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_16_17 VEDU_HIHDR_G_TONEMAP_COEF_16_17;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_18_19 VEDU_HIHDR_G_TONEMAP_COEF_18_19;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_20_21 VEDU_HIHDR_G_TONEMAP_COEF_20_21;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_22_23 VEDU_HIHDR_G_TONEMAP_COEF_22_23;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_24_25 VEDU_HIHDR_G_TONEMAP_COEF_24_25;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_26_27 VEDU_HIHDR_G_TONEMAP_COEF_26_27;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_28_29 VEDU_HIHDR_G_TONEMAP_COEF_28_29;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_30_31 VEDU_HIHDR_G_TONEMAP_COEF_30_31;
    volatile unsigned int VEDU_RESERVED_19[16];
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P0 VEDU_HIHDR_G_GMM_COEF_00_09_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P1 VEDU_HIHDR_G_GMM_COEF_00_09_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P2 VEDU_HIHDR_G_GMM_COEF_00_09_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P3 VEDU_HIHDR_G_GMM_COEF_00_09_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P0 VEDU_HIHDR_G_GMM_COEF_10_19_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P1 VEDU_HIHDR_G_GMM_COEF_10_19_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P2 VEDU_HIHDR_G_GMM_COEF_10_19_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P3 VEDU_HIHDR_G_GMM_COEF_10_19_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P0 VEDU_HIHDR_G_GMM_COEF_20_29_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P1 VEDU_HIHDR_G_GMM_COEF_20_29_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P2 VEDU_HIHDR_G_GMM_COEF_20_29_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P3 VEDU_HIHDR_G_GMM_COEF_20_29_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P0 VEDU_HIHDR_G_GMM_COEF_30_39_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P1 VEDU_HIHDR_G_GMM_COEF_30_39_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P2 VEDU_HIHDR_G_GMM_COEF_30_39_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P3 VEDU_HIHDR_G_GMM_COEF_30_39_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P0 VEDU_HIHDR_G_GMM_COEF_40_49_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P1 VEDU_HIHDR_G_GMM_COEF_40_49_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P2 VEDU_HIHDR_G_GMM_COEF_40_49_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P3 VEDU_HIHDR_G_GMM_COEF_40_49_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P0 VEDU_HIHDR_G_GMM_COEF_50_59_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P1 VEDU_HIHDR_G_GMM_COEF_50_59_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P2 VEDU_HIHDR_G_GMM_COEF_50_59_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P3 VEDU_HIHDR_G_GMM_COEF_50_59_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_60_69_P0 VEDU_HIHDR_G_GMM_COEF_60_69_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_60_69_P1 VEDU_HIHDR_G_GMM_COEF_60_69_P1;
    volatile unsigned int VEDU_RESERVED_20[66];
    volatile U_VEDU_SECURE_SET VEDU_SECURE_SET;
    volatile unsigned int VEDU_RESERVED_21[3];
    volatile unsigned int VEDU_RESERVED_22[2048];
    volatile U_FUNC_VCPI_INTSTAT FUNC_VCPI_INTSTAT;
    volatile U_FUNC_VCPI_RAWINT FUNC_VCPI_RAWINT;
    volatile unsigned int FUNC_VCPI_VEDU_TIMER;
    volatile unsigned int FUNC_VCPI_IDLE_TIMER;
    volatile U_FUNC_VCPI_BUS_IDLE_FLAG FUNC_VCPI_BUS_IDLE_FLAG;
    volatile unsigned int RESERVED_FUNC_VCPI[11];
    volatile unsigned int RESERVED_FUNC_VCTRL[16];
    volatile unsigned int RESERVED_FUNC_QPGLD[16];
    volatile unsigned int RESERVED_FUNC_CURLD[16];
    volatile unsigned int RESERVED_FUNC_NBI[16];
    volatile unsigned int RESERVED_FUNC_PMELD[16];
    volatile unsigned int RESERVED_FUNC_PMEINFO_LD[16];
    volatile U_FUNC_PME_MADI_SUM FUNC_PME_MADI_SUM;
    volatile U_FUNC_PME_MADP_SUM FUNC_PME_MADP_SUM;
    volatile U_FUNC_PME_MADI_NUM FUNC_PME_MADI_NUM;
    volatile U_FUNC_PME_MADP_NUM FUNC_PME_MADP_NUM;
    volatile U_FUNC_PME_LARGE_SAD_SUM FUNC_PME_LARGE_SAD_SUM;
    volatile U_FUNC_PME_LOW_LUMA_SUM FUNC_PME_LOW_LUMA_SUM;
    volatile U_FUNC_PME_CHROMA_SCENE_SUM FUNC_PME_CHROMA_SCENE_SUM;
    volatile U_FUNC_PME_MOVE_SCENE_SUM FUNC_PME_MOVE_SCENE_SUM;
    volatile U_FUNC_PME_SKIN_REGION_SUM FUNC_PME_SKIN_REGION_SUM;
    volatile unsigned int RESERVED_FUNC_PME[7];
    volatile unsigned int RESERVED_FUNC_PMEST[16];
    volatile unsigned int RESERVED_FUNC_PMEINFO_ST[16];
    volatile U_FUNC_BGSTR_BLOCK_COUNT FUNC_BGSTR_BLOCK_COUNT;
    volatile U_FUNC_BGSTR_FRAME_BGM_DIST FUNC_BGSTR_FRAME_BGM_DIST;
    volatile unsigned int RESERVED_FUNC_BGSTR[14];
    volatile unsigned int RESERVED_FUNC_QPG[16];
    volatile unsigned int RESERVED_FUNC_REFLD[16];
    volatile unsigned int RESERVED_FUNC_PINTRA[16];
    volatile unsigned int RESERVED_FUNC_IME[16];
    volatile unsigned int RESERVED_FUNC_FME[16];
    volatile unsigned int RESERVED_FUNC_MRG[16];
    volatile unsigned int RESERVED_FUNC_BGGEN[16];
    volatile unsigned int RESERVED_FUNC_INTRA[16];
    volatile unsigned int RESERVED_FUNC_NU0[24];
    volatile unsigned int FUNC_PMV_MV0_COUNT;
    volatile unsigned int FUNC_PMV_MV1_COUNT;
    volatile unsigned int FUNC_PMV_MV2_COUNT;
    volatile unsigned int FUNC_PMV_MV3_COUNT;
    volatile unsigned int FUNC_PMV_MV4_COUNT;
    volatile unsigned int FUNC_PMV_MV5_COUNT;
    volatile unsigned int FUNC_PMV_MV6_COUNT;
    volatile unsigned int FUNC_PMV_MV7_COUNT;
    volatile unsigned int FUNC_PMV_MV8_COUNT;
    volatile unsigned int FUNC_PMV_MV9_COUNT;
    volatile unsigned int FUNC_PMV_MV10_COUNT;
    volatile unsigned int FUNC_PMV_MV11_COUNT;
    volatile unsigned int FUNC_PMV_MV12_COUNT;
    volatile unsigned int FUNC_PMV_MV13_COUNT;
    volatile unsigned int FUNC_PMV_MV14_COUNT;
    volatile unsigned int FUNC_PMV_MV15_COUNT;
    volatile unsigned int FUNC_PMV_MV16_COUNT;
    volatile unsigned int FUNC_PMV_MV17_COUNT;
    volatile unsigned int FUNC_PMV_MV18_COUNT;
    volatile unsigned int FUNC_PMV_MV19_COUNT;
    volatile unsigned int FUNC_PMV_MV20_COUNT;
    volatile unsigned int FUNC_PMV_MV21_COUNT;
    volatile unsigned int FUNC_PMV_MV22_COUNT;
    volatile unsigned int RESERVED_FUNC_INVALID;
    volatile unsigned int RESERVED_FUNC_TQITQ0[16];
    volatile unsigned int RESERVED_FUNC_NU2[16];
    volatile unsigned int RESERVED_FUNC_BGLD[16];
    volatile unsigned int RESERVED_FUNC_LFLDST[16];
    volatile unsigned int RESERVED_FUNC_DBLK[16];
    volatile unsigned int RESERVED_FUNC_NU4[16];
    volatile unsigned int RESERVED_FUNC_RECST[16];
    volatile unsigned int RESERVED_FUNC_PACK0[16];
    volatile unsigned int RESERVED_FUNC_PACK1[16];
    volatile unsigned int FUNC_CABAC_PIC_STRMSIZE;
    volatile unsigned int FUNC_CABAC_BIN_NUM;
    volatile unsigned int FUNC_CABAC_RES_BIN_NUM;
    volatile unsigned int FUNC_CABAC_HDR_BIN_NUM;
    volatile unsigned int FUNC_CABAC_MOVE_SCENE_BITS;
    volatile unsigned int FUNC_CABAC_STREDGE_MOVE_BITS;
    volatile unsigned int FUNC_CABAC_SKIN_BITS;
    volatile unsigned int FUNC_CABAC_LOWLUMA_BITS;
    volatile unsigned int FUNC_CABAC_CHROMAPROT_BITS;
    volatile unsigned int RESERVED_FUNC_CABAC[7];
    volatile unsigned int FUNC_VLC_CABAC_HRD_BITS;
    volatile unsigned int FUNC_VLC_CABAC_RES_BITS;
    volatile unsigned int FUNC_VLC_CAVLC_HRD_BITS;
    volatile unsigned int FUNC_VLC_CAVLC_RES_BITS;
    volatile unsigned int FUNC_VLC_PIC_STRMSIZE;
    volatile unsigned int RESERVED_FUNC_VLC[11];
    volatile unsigned int FUNC_VLCST_SLC_LEN_CNT;
    volatile unsigned int FUNC_VLCST_WPTR;
    volatile unsigned int FUNC_VLCST_RPTR;
    volatile unsigned int RESERVED_FUNC_VLCST[125];
    volatile unsigned int RESERVED_FUNC_EMAR[16];
    volatile unsigned int RESERVED_FUNC_PPFD[16];
    volatile U_FUNC_SEL_OPT_8X8_CNT FUNC_SEL_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_8X8_CNT FUNC_SEL_INTRA_OPT_8X8_CNT;
    volatile unsigned int FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_OPT_8X8_CNT FUNC_SEL_INTER_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_8X8_CNT FUNC_SEL_INTER_FME_OPT_8X8_CNT;
    volatile U_FUNC_SELINTER_MERGE_OPT_8X8_CNT FUNC_SELINTER_MERGE_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
    volatile U_FUNC_SEL_OPT_16X16_CNT FUNC_SEL_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_16X16_CNT FUNC_SEL_INTRA_OPT_16X16_CNT;
    volatile U_FUNC_SEL_OPT_4X4_CNT FUNC_SEL_OPT_4X4_CNT;
    volatile unsigned int RESERVED0_FUNC_SEL;
    volatile U_FUNC_SEL_INTER_OPT_16X16_CNT FUNC_SEL_INTER_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_16X16_CNT FUNC_SEL_INTER_FME_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
    volatile U_FUNC_SEL_OPT_32X32_CNT FUNC_SEL_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_32X32_CNT FUNC_SEL_INTRA_OPT_32X32_CNT;
    volatile unsigned int RESERVED1_FUNC_SEL;
    volatile U_FUNC_SEL_INTER_OPT_32X32_CNT FUNC_SEL_INTER_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_32X32_CNT FUNC_SEL_INTER_FME_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
    volatile U_FUNC_SEL_OPT_64X64_CNT FUNC_SEL_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_64X64_CNT FUNC_SEL_INTER_FME_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
    volatile U_FUNC_SEL_TOTAL_LUMA_QP FUNC_SEL_TOTAL_LUMA_QP;
    volatile U_FUNC_SEL_MAX_MIN_LUMA_QP FUNC_SEL_MAX_MIN_LUMA_QP;
    volatile unsigned int RESERVED2_FUNC_SEL[35];
    volatile U_FUNC_SEL_LUMA_QP0_CNT FUNC_SEL_LUMA_QP0_CNT;
    volatile U_FUNC_SEL_LUMA_QP1_CNT FUNC_SEL_LUMA_QP1_CNT;
    volatile U_FUNC_SEL_LUMA_QP2_CNT FUNC_SEL_LUMA_QP2_CNT;
    volatile U_FUNC_SEL_LUMA_QP3_CNT FUNC_SEL_LUMA_QP3_CNT;
    volatile U_FUNC_SEL_LUMA_QP4_CNT FUNC_SEL_LUMA_QP4_CNT;
    volatile U_FUNC_SEL_LUMA_QP5_CNT FUNC_SEL_LUMA_QP5_CNT;
    volatile U_FUNC_SEL_LUMA_QP6_CNT FUNC_SEL_LUMA_QP6_CNT;
    volatile U_FUNC_SEL_LUMA_QP7_CNT FUNC_SEL_LUMA_QP7_CNT;
    volatile U_FUNC_SEL_LUMA_QP8_CNT FUNC_SEL_LUMA_QP8_CNT;
    volatile U_FUNC_SEL_LUMA_QP9_CNT FUNC_SEL_LUMA_QP9_CNT;
    volatile U_FUNC_SEL_LUMA_QP10_CNT FUNC_SEL_LUMA_QP10_CNT;
    volatile U_FUNC_SEL_LUMA_QP11_CNT FUNC_SEL_LUMA_QP11_CNT;
    volatile U_FUNC_SEL_LUMA_QP12_CNT FUNC_SEL_LUMA_QP12_CNT;
    volatile U_FUNC_SEL_LUMA_QP13_CNT FUNC_SEL_LUMA_QP13_CNT;
    volatile U_FUNC_SEL_LUMA_QP14_CNT FUNC_SEL_LUMA_QP14_CNT;
    volatile U_FUNC_SEL_LUMA_QP15_CNT FUNC_SEL_LUMA_QP15_CNT;
    volatile U_FUNC_SEL_LUMA_QP16_CNT FUNC_SEL_LUMA_QP16_CNT;
    volatile U_FUNC_SEL_LUMA_QP17_CNT FUNC_SEL_LUMA_QP17_CNT;
    volatile U_FUNC_SEL_LUMA_QP18_CNT FUNC_SEL_LUMA_QP18_CNT;
    volatile U_FUNC_SEL_LUMA_QP19_CNT FUNC_SEL_LUMA_QP19_CNT;
    volatile U_FUNC_SEL_LUMA_QP20_CNT FUNC_SEL_LUMA_QP20_CNT;
    volatile U_FUNC_SEL_LUMA_QP21_CNT FUNC_SEL_LUMA_QP21_CNT;
    volatile U_FUNC_SEL_LUMA_QP22_CNT FUNC_SEL_LUMA_QP22_CNT;
    volatile U_FUNC_SEL_LUMA_QP23_CNT FUNC_SEL_LUMA_QP23_CNT;
    volatile U_FUNC_SEL_LUMA_QP24_CNT FUNC_SEL_LUMA_QP24_CNT;
    volatile U_FUNC_SEL_LUMA_QP25_CNT FUNC_SEL_LUMA_QP25_CNT;
    volatile U_FUNC_SEL_LUMA_QP26_CNT FUNC_SEL_LUMA_QP26_CNT;
    volatile U_FUNC_SEL_LUMA_QP27_CNT FUNC_SEL_LUMA_QP27_CNT;
    volatile U_FUNC_SEL_LUMA_QP28_CNT FUNC_SEL_LUMA_QP28_CNT;
    volatile U_FUNC_SEL_LUMA_QP29_CNT FUNC_SEL_LUMA_QP29_CNT;
    volatile U_FUNC_SEL_LUMA_QP30_CNT FUNC_SEL_LUMA_QP30_CNT;
    volatile U_FUNC_SEL_LUMA_QP31_CNT FUNC_SEL_LUMA_QP31_CNT;
    volatile U_FUNC_SEL_LUMA_QP32_CNT FUNC_SEL_LUMA_QP32_CNT;
    volatile U_FUNC_SEL_LUMA_QP33_CNT FUNC_SEL_LUMA_QP33_CNT;
    volatile U_FUNC_SEL_LUMA_QP34_CNT FUNC_SEL_LUMA_QP34_CNT;
    volatile U_FUNC_SEL_LUMA_QP35_CNT FUNC_SEL_LUMA_QP35_CNT;
    volatile U_FUNC_SEL_LUMA_QP36_CNT FUNC_SEL_LUMA_QP36_CNT;
    volatile U_FUNC_SEL_LUMA_QP37_CNT FUNC_SEL_LUMA_QP37_CNT;
    volatile U_FUNC_SEL_LUMA_QP38_CNT FUNC_SEL_LUMA_QP38_CNT;
    volatile U_FUNC_SEL_LUMA_QP39_CNT FUNC_SEL_LUMA_QP39_CNT;
    volatile U_FUNC_SEL_LUMA_QP40_CNT FUNC_SEL_LUMA_QP40_CNT;
    volatile U_FUNC_SEL_LUMA_QP41_CNT FUNC_SEL_LUMA_QP41_CNT;
    volatile U_FUNC_SEL_LUMA_QP42_CNT FUNC_SEL_LUMA_QP42_CNT;
    volatile U_FUNC_SEL_LUMA_QP43_CNT FUNC_SEL_LUMA_QP43_CNT;
    volatile U_FUNC_SEL_LUMA_QP44_CNT FUNC_SEL_LUMA_QP44_CNT;
    volatile U_FUNC_SEL_LUMA_QP45_CNT FUNC_SEL_LUMA_QP45_CNT;
    volatile U_FUNC_SEL_LUMA_QP46_CNT FUNC_SEL_LUMA_QP46_CNT;
    volatile U_FUNC_SEL_LUMA_QP47_CNT FUNC_SEL_LUMA_QP47_CNT;
    volatile U_FUNC_SEL_LUMA_QP48_CNT FUNC_SEL_LUMA_QP48_CNT;
    volatile U_FUNC_SEL_LUMA_QP49_CNT FUNC_SEL_LUMA_QP49_CNT;
    volatile U_FUNC_SEL_LUMA_QP50_CNT FUNC_SEL_LUMA_QP50_CNT;
    volatile U_FUNC_SEL_LUMA_QP51_CNT FUNC_SEL_LUMA_QP51_CNT;
    volatile unsigned int RESERVED3_FUNC_SEL[12];
    volatile unsigned int RESERVED_FUNC_TBLDST[128];
    volatile U_FUNC_SAO_OFF_NUM FUNC_SAO_OFF_NUM;
    volatile unsigned int FUNC_SAO_MSE_SUM;
    volatile U_FUNC_SAO_MSE_CNT FUNC_SAO_MSE_CNT;
    volatile U_FUNC_SAO_MSE_MAX FUNC_SAO_MSE_MAX;
    volatile unsigned int FUNC_SAO_SSD_AREA0_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA1_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA2_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA3_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA4_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA5_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA6_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA7_SUM;
    volatile unsigned int RESERVED_FUNC_SAO[20];
    volatile unsigned int RESERVED_FUNC_NU3[48];
    volatile unsigned int RESERVED_INVALID[24576];
    volatile U_AXIDFX_ERR AXIDFX_ERR;
    volatile unsigned int RESERVED_1[3];
    volatile U_AXIDFX_AR_R_CNT AXIDFX_AR_R_CNT;
    volatile U_AXIDFX_AW_W_CNT AXIDFX_AW_W_CNT;
    volatile U_AXIDFX_AW_B_CNT AXIDFX_AW_B_CNT;
    volatile unsigned int RESERVED_2;
    volatile U_AXIDFX_AR_R_ID_ERR AXIDFX_AR_R_ID_ERR;
    volatile U_AXIDFX_ERR_ARID AXIDFX_ERR_ARID;
    volatile U_AXIDFX_ERR_RID AXIDFX_ERR_RID;
    volatile unsigned int RESERVED_3;
    volatile U_AXIDFX_AW_W_B_ID_ERR AXIDFX_AW_W_B_ID_ERR;
    volatile U_AXIDFX_ERR_AWID AXIDFX_ERR_AWID;
    volatile U_AXIDFX_ERR_WID AXIDFX_ERR_WID;
    volatile U_AXIDFX_ERR_BID AXIDFX_ERR_BID;
    volatile U_AXIDFX_ARID_TX_0ERR AXIDFX_ARID_TX_0ERR;
    volatile U_AXIDFX_ARID_TX_1ERR AXIDFX_ARID_TX_1ERR;
    volatile U_AXIDFX_ARID_TX_2ERR AXIDFX_ARID_TX_2ERR;
    volatile unsigned int RESERVED_4;
    volatile U_AXIDFX_RID_RX_0ERR AXIDFX_RID_RX_0ERR;
    volatile U_AXIDFX_RID_RX_1ERR AXIDFX_RID_RX_1ERR;
    volatile U_AXIDFX_RID_RX_2ERR AXIDFX_RID_RX_2ERR;
    volatile unsigned int RESERVED_5;
    volatile U_AXIDFX_ARID_RX_0ERR AXIDFX_ARID_RX_0ERR;
    volatile U_AXIDFX_BID_RX_ERR AXIDFX_BID_RX_ERR;
    volatile unsigned int RESERVED_6[2];
    volatile U_AXIDFX_ARID_LEN_0ERR AXIDFX_ARID_LEN_0ERR;
    volatile U_AXIDFX_ARID_LEN_1ERR AXIDFX_ARID_LEN_1ERR;
    volatile U_AXIDFX_ARID_LEN_2ERR AXIDFX_ARID_LEN_2ERR;
    volatile unsigned int RESERVED_7;
    volatile unsigned int AXIDFX_AWLEN_CNT;
    volatile unsigned int AXIDFX_WLEN_CNT;
    volatile unsigned int RESERVED_8[2];
    volatile U_AXIDFX_RESP_ERR AXIDFX_RESP_ERR;
    volatile U_AXIDFX_ERR_RESP AXIDFX_ERR_RESP;
    volatile U_AXIDFX_LEN_ERR AXIDFX_LEN_ERR;
    volatile U_AXIDFX_ERR_LEN AXIDFX_ERR_LEN;
    volatile unsigned int AXIDFX_0RID_FLAG;
    volatile unsigned int AXIDFX_1RID_FLAG;
    volatile U_AXIDFX_2RID_FLAG AXIDFX_2RID_FLAG;
    volatile U_AXIDFX_WID_FLAG AXIDFX_WID_FLAG;
    volatile U_AXIDFX_AXI_ST AXIDFX_AXI_ST;
    volatile unsigned int RESERVED_9[19];
    volatile U_AXIDFX_SOFT_RST_REQ AXIDFX_SOFT_RST_REQ;
    volatile U_AXIDFX_SOFT_RST_ACK AXIDFX_SOFT_RST_ACK;
    volatile unsigned int RESERVED_11[2];
    volatile U_AXIDFX_SOFT_RST_FORCE_REQ_ACK AXIDFX_SOFT_RST_FORCE_REQ_ACK;
    volatile unsigned int AXIDFX_SOFT_RST_STATE0;
    volatile unsigned int AXIDFX_SOFT_RST_STATE1;
    volatile unsigned int RESERVED_12[57];
    volatile unsigned int RESERVED_S_NS_0;
    volatile unsigned int RESERVED_S_NS_1;
    volatile unsigned int RESERVED_S_NS_2;
    volatile unsigned int RESERVED_S_NS_3;
    volatile unsigned int RESERVED_S_NS_4;
    volatile unsigned int RESERVED_S_NS_5;
    volatile unsigned int RESERVED_S_NS_6;
    volatile unsigned int RESERVED_S_NS_7;
} S_HEVC_AVC_REGS_TYPE;
typedef struct {
    volatile U_VEDU_VCPI_INTMASK VEDU_VCPI_INTMASK;
    volatile U_VEDU_VCPI_INTCLR VEDU_VCPI_INTCLR;
    volatile U_VEDU_VCPI_START VEDU_VCPI_START;
    volatile U_VEDU_BUS_IDLE_REQ VEDU_BUS_IDLE_REQ;
    volatile unsigned int VEDU_VCPI_FRAMENO;
    volatile unsigned int VEDU_VCPI_TIMEOUT;
    volatile U_VEDU_VCPI_MODE VEDU_VCPI_MODE;
    volatile U_VEDU_EMAR_SCRAMBLE_TYPE VEDU_EMAR_SCRAMBLE_TYPE;
    volatile unsigned int VEDU_VCPI_LLILD_ADDR_L;
    volatile unsigned int VEDU_VCPI_LLILD_ADDR_H;
    volatile U_VEDU_VCPI_RC_ENABLE VEDU_VCPI_RC_ENABLE;
    volatile U_VEDU_VLCST_PTBITS_EN VEDU_VLCST_PTBITS_EN;
    volatile unsigned int VEDU_VLCST_PTBITS;
    volatile U_VEDU_PPFD_ST_CFG VEDU_PPFD_ST_CFG;
    volatile U_VEDU_VCPI_QPCFG VEDU_VCPI_QPCFG;
    volatile U_VEDU_QPG_MAX_MIN_QP VEDU_QPG_MAX_MIN_QP;
    volatile U_VEDU_QPG_SMART_REG VEDU_QPG_SMART_REG;
    volatile U_VEDU_QPG_ROW_TARGET_BITS VEDU_QPG_ROW_TARGET_BITS;
    volatile U_VEDU_QPG_AVERAGE_LCU_BITS VEDU_QPG_AVERAGE_LCU_BITS;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG0 VEDU_QPG_CU_QP_DELTA_THRESH_REG0;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG1 VEDU_QPG_CU_QP_DELTA_THRESH_REG1;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG2 VEDU_QPG_CU_QP_DELTA_THRESH_REG2;
    volatile U_VEDU_QPG_CU_QP_DELTA_THRESH_REG3 VEDU_QPG_CU_QP_DELTA_THRESH_REG3;
    volatile U_VEDU_QPG_DELTA_LEVEL VEDU_QPG_DELTA_LEVEL;
    volatile U_VEDU_QPG_MADI_SWITCH_THR VEDU_QPG_MADI_SWITCH_THR;
    volatile U_VEDU_QPG_CURR_SAD_EN VEDU_QPG_CURR_SAD_EN;
    volatile U_VEDU_QPG_CURR_SAD_LEVEL VEDU_QPG_CURR_SAD_LEVEL;
    volatile U_VEDU_QPG_CURR_SAD_THRESH0 VEDU_QPG_CURR_SAD_THRESH0;
    volatile U_VEDU_QPG_CURR_SAD_THRESH1 VEDU_QPG_CURR_SAD_THRESH1;
    volatile U_VEDU_QPG_CURR_SAD_THRESH2 VEDU_QPG_CURR_SAD_THRESH2;
    volatile U_VEDU_QPG_CURR_SAD_THRESH3 VEDU_QPG_CURR_SAD_THRESH3;
    volatile U_VEDU_LUMA_RC VEDU_LUMA_RC;
    volatile U_VEDU_LUMA_LEVEL VEDU_LUMA_LEVEL;
    volatile U_VEDU_LUMA_THRESH0 VEDU_LUMA_THRESH0;
    volatile U_VEDU_LUMA_THRESH1 VEDU_LUMA_THRESH1;
    volatile U_VEDU_LUMA_THRESH2 VEDU_LUMA_THRESH2;
    volatile U_VEDU_LUMA_THRESH3 VEDU_LUMA_THRESH3;
    volatile U_VEDU_VCPI_BG_ENABLE VEDU_VCPI_BG_ENABLE;
    volatile U_VEDU_VCPI_BG_FLT_PARA0 VEDU_VCPI_BG_FLT_PARA0;
    volatile U_VEDU_VCPI_BG_FLT_PARA1 VEDU_VCPI_BG_FLT_PARA1;
    volatile U_VEDU_VCPI_BG_FLT_PARA2 VEDU_VCPI_BG_FLT_PARA2;
    volatile U_VEDU_VCPI_BG_THR0 VEDU_VCPI_BG_THR0;
    volatile U_VEDU_VCPI_BG_THR1 VEDU_VCPI_BG_THR1;
    volatile U_VEDU_VCPI_BG_STRIDE VEDU_VCPI_BG_STRIDE;
    volatile U_VEDU_VCPI_BG_EXT_STRIDE VEDU_VCPI_BG_EXT_STRIDE;
    volatile unsigned int VEDU_VCPI_BGL_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGL_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGC_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGC_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGL_EXT_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGL_EXT_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGC_EXT_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGC_EXT_ADDR_H;
    volatile unsigned int VEDU_VCPI_BGINF_ADDR_L;
    volatile unsigned int VEDU_VCPI_BGINF_ADDR_H;
    volatile U_VEDU_VCPI_OUTSTD VEDU_VCPI_OUTSTD;
    volatile U_VEDU_VCTRL_LCU_BASELINE VEDU_VCTRL_LCU_BASELINE;
    volatile unsigned int VEDU_VCPI_SPMEM_CTRL_0;
    volatile U_VEDU_VCPI_SPMEM_CTRL_1 VEDU_VCPI_SPMEM_CTRL_1;
    volatile U_VEDU_VCPI_SPMEM_CTRL_2 VEDU_VCPI_SPMEM_CTRL_2;
    volatile unsigned int VEDU_VCPI_TPMEM_CTRL_0;
    volatile unsigned int VEDU_VCPI_TPMEM_CTRL_1;
    volatile U_VEDU_CURLD_GCFG VEDU_CURLD_GCFG;
    volatile U_VEDU_VCPI_OSD_ENABLE VEDU_VCPI_OSD_ENABLE;
    volatile U_VEDU_VCPI_STRFMT VEDU_VCPI_STRFMT;
    volatile U_VEDU_VCPI_INTRA_INTER_CU_EN VEDU_VCPI_INTRA_INTER_CU_EN;
    volatile U_VEDU_VCPI_CROSS_TILE_SLC VEDU_VCPI_CROSS_TILE_SLC;
    volatile U_VEDU_VCPI_MULTISLC VEDU_VCPI_MULTISLC;
    volatile U_VEDU_VCTRL_LCU_TARGET_BIT VEDU_VCTRL_LCU_TARGET_BIT;
    volatile U_VEDU_VCPI_SW_L0_SIZE VEDU_VCPI_SW_L0_SIZE;
    volatile U_VEDU_VCPI_SW_L1_SIZE VEDU_VCPI_SW_L1_SIZE;
    volatile U_VEDU_VCPI_I_SLC_INSERT VEDU_VCPI_I_SLC_INSERT;
    volatile U_VEDU_PME_SAFE_CFG VEDU_PME_SAFE_CFG;
    volatile U_VEDU_PME_IBLK_REFRESH VEDU_PME_IBLK_REFRESH;
    volatile U_VEDU_PME_IBLK_REFRESH_NUM VEDU_PME_IBLK_REFRESH_NUM;
    volatile U_VEDU_INTRA_CHNL4_ANG_0EN VEDU_INTRA_CHNL4_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL4_ANG_1EN VEDU_INTRA_CHNL4_ANG_1EN;
    volatile U_VEDU_INTRA_CHNL8_ANG_0EN VEDU_INTRA_CHNL8_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL8_ANG_1EN VEDU_INTRA_CHNL8_ANG_1EN;
    volatile U_VEDU_INTRA_CHNL16_ANG_0EN VEDU_INTRA_CHNL16_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL16_ANG_1EN VEDU_INTRA_CHNL16_ANG_1EN;
    volatile U_VEDU_INTRA_CHNL32_ANG_0EN VEDU_INTRA_CHNL32_ANG_0EN;
    volatile U_VEDU_INTRA_CHNL32_ANG_1EN VEDU_INTRA_CHNL32_ANG_1EN;
    volatile U_VEDU_PACK_CU_PARAMETER VEDU_PACK_CU_PARAMETER;
    volatile U_VEDU_PACK_PCM_PARAMETER VEDU_PACK_PCM_PARAMETER;
    volatile U_VEDU_QPG_READLINE_INTERVAL VEDU_QPG_READLINE_INTERVAL;
    volatile U_VEDU_PMV_READLINE_INTERVAL VEDU_PMV_READLINE_INTERVAL;
    volatile U_VEDU_RGB2YUV_COEF_P0 VEDU_RGB2YUV_COEF_P0;
    volatile U_VEDU_RGB2YUV_COEF_P1 VEDU_RGB2YUV_COEF_P1;
    volatile U_VEDU_RGB2YUV_COEF_P2 VEDU_RGB2YUV_COEF_P2;
    volatile U_VEDU_RGB2YUV_OFFSET VEDU_RGB2YUV_OFFSET;
    volatile U_VEDU_RGB2YUV_CLIP_THR_Y VEDU_RGB2YUV_CLIP_THR_Y;
    volatile U_VEDU_RGB2YUV_CLIP_THR_C VEDU_RGB2YUV_CLIP_THR_C;
    volatile U_VEDU_RGB2YUV_SHIFT_WIDTH VEDU_RGB2YUV_SHIFT_WIDTH;
    volatile U_VEDU_VCPI_OSD_POS_0 VEDU_VCPI_OSD_POS_0;
    volatile U_VEDU_VCPI_OSD_POS_1 VEDU_VCPI_OSD_POS_1;
    volatile unsigned int VEDU_VCPI_OSD_POS_2;
    volatile unsigned int VEDU_VCPI_OSD_POS_3;
    volatile U_VEDU_VCPI_OSD_POS_4 VEDU_VCPI_OSD_POS_4;
    volatile U_VEDU_VCPI_OSD_POS_5 VEDU_VCPI_OSD_POS_5;
    volatile unsigned int VEDU_VCPI_OSD_POS_6;
    volatile unsigned int VEDU_VCPI_OSD_POS_7;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_0;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_1;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_2;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_3;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_4;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_5;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_6;
    volatile unsigned int VEDU_VCPI_OSD_SIZE_7;
    volatile unsigned int VEDU_VCPI_OSD_LAYERID;
    volatile U_VEDU_VCPI_OSD_QP0 VEDU_VCPI_OSD_QP0;
    volatile U_VEDU_VCPI_OSD_QP1 VEDU_VCPI_OSD_QP1;
    volatile U_VEDU_CURLD_OSD01_ALPHA VEDU_CURLD_OSD01_ALPHA;
    volatile U_VEDU_CURLD_OSD23_ALPHA VEDU_CURLD_OSD23_ALPHA;
    volatile U_VEDU_CURLD_OSD45_ALPHA VEDU_CURLD_OSD45_ALPHA;
    volatile U_VEDU_CURLD_OSD67_ALPHA VEDU_CURLD_OSD67_ALPHA;
    volatile unsigned int VEDU_CURLD_OSD_GALPHA0;
    volatile unsigned int VEDU_CURLD_OSD_GALPHA1;
    volatile U_VEDU_TBLDST_READLINE_INTERVAL VEDU_TBLDST_READLINE_INTERVAL;
    volatile unsigned int VEDU_CURLD_OSD0_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD0_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD1_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD1_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD2_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD2_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD3_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD3_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD4_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD4_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD5_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD5_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD6_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD6_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD7_ADDR_L;
    volatile unsigned int VEDU_CURLD_OSD7_ADDR_H;
    volatile unsigned int VEDU_CURLD_OSD01_STRIDE;
    volatile unsigned int VEDU_CURLD_OSD23_STRIDE;
    volatile unsigned int VEDU_CURLD_OSD45_STRIDE;
    volatile unsigned int VEDU_CURLD_OSD67_STRIDE;
    volatile U_VEDU_NEW_CFG0 VEDU_NEW_CFG0;
    volatile U_VEDU_VCTRL_ROI_CFG0 VEDU_VCTRL_ROI_CFG0;
    volatile U_VEDU_VCTRL_ROI_CFG1 VEDU_VCTRL_ROI_CFG1;
    volatile U_VEDU_VCTRL_ROI_CFG2 VEDU_VCTRL_ROI_CFG2;
    volatile U_VEDU_VCTRL_ROI_SIZE_0 VEDU_VCTRL_ROI_SIZE_0;
    volatile U_VEDU_VCTRL_ROI_SIZE_1 VEDU_VCTRL_ROI_SIZE_1;
    volatile U_VEDU_VCTRL_ROI_SIZE_2 VEDU_VCTRL_ROI_SIZE_2;
    volatile U_VEDU_VCTRL_ROI_SIZE_3 VEDU_VCTRL_ROI_SIZE_3;
    volatile U_VEDU_VCTRL_ROI_SIZE_4 VEDU_VCTRL_ROI_SIZE_4;
    volatile U_VEDU_VCTRL_ROI_SIZE_5 VEDU_VCTRL_ROI_SIZE_5;
    volatile U_VEDU_VCTRL_ROI_SIZE_6 VEDU_VCTRL_ROI_SIZE_6;
    volatile U_VEDU_VCTRL_ROI_SIZE_7 VEDU_VCTRL_ROI_SIZE_7;
    volatile U_VEDU_VCTRL_ROI_START_0 VEDU_VCTRL_ROI_START_0;
    volatile U_VEDU_VCTRL_ROI_START_1 VEDU_VCTRL_ROI_START_1;
    volatile U_VEDU_VCTRL_ROI_START_2 VEDU_VCTRL_ROI_START_2;
    volatile U_VEDU_VCTRL_ROI_START_3 VEDU_VCTRL_ROI_START_3;
    volatile U_VEDU_VCTRL_ROI_START_4 VEDU_VCTRL_ROI_START_4;
    volatile U_VEDU_VCTRL_ROI_START_5 VEDU_VCTRL_ROI_START_5;
    volatile U_VEDU_VCTRL_ROI_START_6 VEDU_VCTRL_ROI_START_6;
    volatile U_VEDU_VCTRL_ROI_START_7 VEDU_VCTRL_ROI_START_7;
    volatile U_VEDU_TILE_MODE VEDU_TILE_MODE;
    volatile U_VEDU_VCPI_PICSIZE_PIX VEDU_VCPI_PICSIZE_PIX;
    volatile U_VEDU_TILE_POS VEDU_TILE_POS;
    volatile U_VEDU_VCPI_TILE_SIZE VEDU_VCPI_TILE_SIZE;
    volatile U_VEDU_VCPI_VLC_CONFIG VEDU_VCPI_VLC_CONFIG;
    volatile U_VEDU_VCPI_REF_FLAG VEDU_VCPI_REF_FLAG;
    volatile U_VEDU_PMV_TMV_EN VEDU_PMV_TMV_EN;
    volatile U_VEDU_VCPI_TMV_LOAD VEDU_VCPI_TMV_LOAD;
    volatile unsigned int VEDU_PMV_POC_0;
    volatile unsigned int VEDU_PMV_POC_1;
    volatile unsigned int VEDU_PMV_POC_2;
    volatile unsigned int VEDU_PMV_POC_3;
    volatile unsigned int VEDU_PMV_POC_4;
    volatile unsigned int VEDU_PMV_POC_5;
    volatile U_VEDU_CABAC_GLB_CFG VEDU_CABAC_GLB_CFG;
    volatile U_VEDU_CABAC_SLCHDR_SIZE VEDU_CABAC_SLCHDR_SIZE;
    volatile U_VEDU_CABAC_SLCHDR_PART1 VEDU_CABAC_SLCHDR_PART1;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG1;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG2;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG3;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG4;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG5;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG6;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG7;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG8;
    volatile U_VEDU_CABAC_SLCHDR_SIZE_I VEDU_CABAC_SLCHDR_SIZE_I;
    volatile U_VEDU_CABAC_SLCHDR_PART1_I VEDU_CABAC_SLCHDR_PART1_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG1_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG2_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG3_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG4_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG5_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG6_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG7_I;
    volatile unsigned int VEDU_CABAC_SLCHDR_PART2_SEG8_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM0;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM1;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM2;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM3;
    volatile unsigned int VEDU_VLC_REORDERSTRM0;
    volatile unsigned int VEDU_VLC_REORDERSTRM1;
    volatile unsigned int VEDU_VLC_MARKINGSTRM0;
    volatile unsigned int VEDU_VLC_MARKINGSTRM1;
    volatile U_VEDU_VLC_SLCHDRPARA VEDU_VLC_SLCHDRPARA;
    volatile U_VEDU_VLC_SVC VEDU_VLC_SVC;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM0_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM1_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM2_I;
    volatile unsigned int VEDU_VLC_SLCHDRSTRM3_I;
    volatile unsigned int VEDU_VLC_REORDERSTRM0_I;
    volatile unsigned int VEDU_VLC_REORDERSTRM1_I;
    volatile unsigned int VEDU_VLC_MARKINGSTRM0_I;
    volatile unsigned int VEDU_VLC_MARKINGSTRM1_I;
    volatile U_VEDU_VLC_SLCHDRPARA_I VEDU_VLC_SLCHDRPARA_I;
    volatile unsigned int VEDU_VLCST_STRMBUFLEN;
    volatile unsigned int VEDU_VLCST_SLC_CFG0;
    volatile unsigned int VEDU_VLCST_SLC_CFG1;
    volatile unsigned int VEDU_VLCST_SLC_CFG2;
    volatile unsigned int VEDU_VLCST_SLC_CFG3;
    volatile unsigned int VEDU_ICE_V2R1_SEG_256X1_DCMP_CTRL;
    volatile U_VEDU_ICE_CMC_MODE_CFG0 VEDU_ICE_CMC_MODE_CFG0;
    volatile U_VEDU_ICE_CMC_MODE_CFG1 VEDU_ICE_CMC_MODE_CFG1;
    volatile U_VEDU_QPG_LOWLUMA VEDU_QPG_LOWLUMA;
    volatile U_VEDU_QPG_HEDGE VEDU_QPG_HEDGE;
    volatile U_VEDU_QPG_HEDGE_MOVE VEDU_QPG_HEDGE_MOVE;
    volatile U_VEDU_QPG_SKIN VEDU_QPG_SKIN;
    volatile U_VEDU_QPG_INTRA_DET VEDU_QPG_INTRA_DET;
    volatile unsigned int VEDU_QPG_H264_SMOOTH;
    volatile U_VEDU_QPG_CU32_DELTA VEDU_QPG_CU32_DELTA;
    volatile U_VEDU_QPG_LAMBDA_MODE VEDU_QPG_LAMBDA_MODE;
    volatile U_VEDU_QPG_QP_RESTRAIN VEDU_QPG_QP_RESTRAIN;
    volatile unsigned int VEDU_QPG_CU_MIN_SAD_THRESH_0;
    volatile U_VEDU_QPG_CU_MIN_SAD_REG VEDU_QPG_CU_MIN_SAD_REG;
    volatile U_VEDU_QPG_FLAT_REGION VEDU_QPG_FLAT_REGION;
    volatile U_VEDU_QPG_RES_COEF VEDU_QPG_RES_COEF;
    volatile U_VEDU_TILE_RC VEDU_TILE_RC;
    volatile U_VEDU_CHROMA_PROTECT VEDU_CHROMA_PROTECT;
    volatile U_VEDU_PME_QPG_RC_THR0 VEDU_PME_QPG_RC_THR0;
    volatile U_VEDU_PME_QPG_RC_THR1 VEDU_PME_QPG_RC_THR1;
    volatile U_VEDU_PME_LOW_LUMA_THR VEDU_PME_LOW_LUMA_THR;
    volatile U_VEDU_PME_CHROMA_FLAT VEDU_PME_CHROMA_FLAT;
    volatile U_VEDU_PME_LUMA_FLAT VEDU_PME_LUMA_FLAT;
    volatile U_VEDU_PME_MADI_FLAT VEDU_PME_MADI_FLAT;
    volatile U_VEDU_VLCST_DESCRIPTOR VEDU_VLCST_DESCRIPTOR;
    volatile unsigned int VEDU_PPFD_ST_LEN0;
    volatile U_VEDU_CURLD_CLIP_LUMA VEDU_CURLD_CLIP_LUMA;
    volatile U_VEDU_CURLD_CLIP_CHROMA VEDU_CURLD_CLIP_CHROMA;
    volatile U_VEDU_TQITQ_DEADZONE VEDU_TQITQ_DEADZONE;
    volatile U_VEDU_VCPI_PME_PARAM VEDU_VCPI_PME_PARAM;
    volatile U_VEDU_VCPI_PIC_STRONG_EN VEDU_VCPI_PIC_STRONG_EN;
    volatile U_VEDU_VCPI_PINTRA_THRESH0 VEDU_VCPI_PINTRA_THRESH0;
    volatile U_VEDU_VCPI_PINTRA_THRESH1 VEDU_VCPI_PINTRA_THRESH1;
    volatile U_VEDU_VCPI_PINTRA_THRESH2 VEDU_VCPI_PINTRA_THRESH2;
    volatile U_VEDU_VCPI_INTRA32_LOW_POWER VEDU_VCPI_INTRA32_LOW_POWER;
    volatile U_VEDU_VCPI_INTRA16_LOW_POWER VEDU_VCPI_INTRA16_LOW_POWER;
    volatile U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM VEDU_VCPI_INTRA_REDUCE_RDO_NUM;
    volatile U_VEDU_VCPI_NOFORCEZERO VEDU_VCPI_NOFORCEZERO;
    volatile U_VEDU_PME_SKIP_LARGE_RES VEDU_PME_SKIP_LARGE_RES;
    volatile U_VEDU_PME_SKIN_SAD_THR VEDU_PME_SKIN_SAD_THR;
    volatile U_VEDU_VCTRL_NM_ACOFFSET_DENOISE VEDU_VCTRL_NM_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_NM_ENGTHR_DENOISE VEDU_VCTRL_NM_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_NM_TU8_DENOISE VEDU_VCTRL_NM_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SK_ACOFFSET_DENOISE VEDU_VCTRL_SK_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SK_ENGTHR_DENOISE VEDU_VCTRL_SK_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SK_TU8_DENOISE VEDU_VCTRL_SK_TU8_DENOISE;
    volatile U_VEDU_VCTRL_ST_ACOFFSET_DENOISE VEDU_VCTRL_ST_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_ST_ENGTHR_DENOISE VEDU_VCTRL_ST_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_ST_TU8_DENOISE VEDU_VCTRL_ST_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SE_ACOFFSET_DENOISE VEDU_VCTRL_SE_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SE_ENGTHR_DENOISE VEDU_VCTRL_SE_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SE_TU8_DENOISE VEDU_VCTRL_SE_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SELM_ACOFFSET_DENOISE VEDU_VCTRL_SELM_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SELM_ENGTHR_DENOISE VEDU_VCTRL_SELM_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SELM_TU8_DENOISE VEDU_VCTRL_SELM_TU8_DENOISE;
    volatile U_VEDU_VCTRL_WS_ACOFFSET_DENOISE VEDU_VCTRL_WS_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_WS_ENGTHR_DENOISE VEDU_VCTRL_WS_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_WS_TU8_DENOISE VEDU_VCTRL_WS_TU8_DENOISE;
    volatile U_VEDU_VCTRL_SSSE_ACOFFSET_DENOISE VEDU_VCTRL_SSSE_ACOFFSET_DENOISE;
    volatile U_VEDU_VCTRL_SSSE_ENGTHR_DENOISE VEDU_VCTRL_SSSE_ENGTHR_DENOISE;
    volatile U_VEDU_VCTRL_SSSE_TU8_DENOISE VEDU_VCTRL_SSSE_TU8_DENOISE;
    volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_0 VEDU_VCTRL_INTRA_RDO_FACTOR_0;
    volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_1 VEDU_VCTRL_INTRA_RDO_FACTOR_1;
    volatile U_VEDU_VCTRL_INTRA_RDO_FACTOR_2 VEDU_VCTRL_INTRA_RDO_FACTOR_2;
    volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_0 VEDU_VCTRL_MRG_RDO_FACTOR_0;
    volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_1 VEDU_VCTRL_MRG_RDO_FACTOR_1;
    volatile U_VEDU_VCTRL_MRG_RDO_FACTOR_2 VEDU_VCTRL_MRG_RDO_FACTOR_2;
    volatile U_VEDU_VCTRL_FME_RDO_FACTOR_0 VEDU_VCTRL_FME_RDO_FACTOR_0;
    volatile U_VEDU_VCTRL_FME_RDO_FACTOR_1 VEDU_VCTRL_FME_RDO_FACTOR_1;
    volatile U_VEDU_VCTRL_FME_RDO_FACTOR_2 VEDU_VCTRL_FME_RDO_FACTOR_2;
    volatile U_VEDU_PME_NEW_COST VEDU_PME_NEW_COST;
    volatile U_VEDU_PME_COST_OFFSET VEDU_PME_COST_OFFSET;
    volatile U_VEDU_PME_ADJUST_PMEMV_H264 VEDU_PME_ADJUST_PMEMV_H264;
    volatile U_VEDU_PME_INTRABLK_DET VEDU_PME_INTRABLK_DET;
    volatile U_VEDU_PME_INTRABLK_DET_THR VEDU_PME_INTRABLK_DET_THR;
    volatile U_VEDU_PME_SKIN_THR VEDU_PME_SKIN_THR;
    volatile U_VEDU_PME_STRONG_EDGE VEDU_PME_STRONG_EDGE;
    volatile U_VEDU_PME_LARGE_MOVE_THR VEDU_PME_LARGE_MOVE_THR;
    volatile U_VEDU_PME_INTER_STRONG_EDGE VEDU_PME_INTER_STRONG_EDGE;
    volatile U_VEDU_PME_CHROMA_STRONG_EDGE VEDU_PME_CHROMA_STRONG_EDGE;
    volatile U_VEDU_CHROMA_SAD_THR VEDU_CHROMA_SAD_THR;
    volatile U_VEDU_CHROMA_FG_THR VEDU_CHROMA_FG_THR;
    volatile U_VEDU_CHROMA_BG_THR VEDU_CHROMA_BG_THR;
    volatile U_VEDU_CHROMA_SUM_FG_THR VEDU_CHROMA_SUM_FG_THR;
    volatile U_VEDU_CHROMA_SUM_BG_THR VEDU_CHROMA_SUM_BG_THR;
    volatile U_VEDU_CHROMA_FG_COUNT_THR VEDU_CHROMA_FG_COUNT_THR;
    volatile U_VEDU_CHROMA_BG_COUNT_THR VEDU_CHROMA_BG_COUNT_THR;
    volatile U_VEDU_PME_MOVE_SCENE_THR VEDU_PME_MOVE_SCENE_THR;
    volatile U_VEDU_PME_NEW_MADI_TH VEDU_PME_NEW_MADI_TH;
    volatile U_VEDU_PME_NEW_LAMBDA VEDU_PME_NEW_LAMBDA;
    volatile U_VEDU_VCPI_DBLKCFG VEDU_VCPI_DBLKCFG;
    volatile U_VEDU_FME_BIAS_COST0 VEDU_FME_BIAS_COST0;
    volatile U_VEDU_FME_BIAS_COST1 VEDU_FME_BIAS_COST1;
    volatile U_VEDU_MRG_BIAS_COST0 VEDU_MRG_BIAS_COST0;
    volatile U_VEDU_MRG_BIAS_COST1 VEDU_MRG_BIAS_COST1;
    volatile U_VEDU_MRG_ABS_OFFSET0 VEDU_MRG_ABS_OFFSET0;
    volatile U_VEDU_MRG_ABS_OFFSET1 VEDU_MRG_ABS_OFFSET1;
    volatile U_VEDU_MRG_ADJ_WEIGHT VEDU_MRG_ADJ_WEIGHT;
    volatile U_VEDU_INTRA_BIT_WEIGHT VEDU_INTRA_BIT_WEIGHT;
    volatile U_VEDU_INTRA_RDO_COST_OFFSET_0 VEDU_INTRA_RDO_COST_OFFSET_0;
    volatile U_VEDU_INTRA_RDO_COST_OFFSET_1 VEDU_INTRA_RDO_COST_OFFSET_1;
    volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_0 VEDU_INTRA_NO_DC_COST_OFFSET_0;
    volatile U_VEDU_INTRA_NO_DC_COST_OFFSET_1 VEDU_INTRA_NO_DC_COST_OFFSET_1;
    volatile U_VEDU_INTRA_RDO_COST_OFFSET_3 VEDU_INTRA_RDO_COST_OFFSET_3;
    volatile U_VEDU_SEL_OFFSET_STRENGTH VEDU_SEL_OFFSET_STRENGTH;
    volatile U_VEDU_SEL_CU32_DC_AC_TH_OFFSET VEDU_SEL_CU32_DC_AC_TH_OFFSET;
    volatile U_VEDU_SEL_CU32_QP_TH VEDU_SEL_CU32_QP_TH;
    volatile U_VEDU_SEL_RES_DC_AC_TH VEDU_SEL_RES_DC_AC_TH;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG00 VEDU_QPG_QP_LAMBDA_CTRL_REG00;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG01 VEDU_QPG_QP_LAMBDA_CTRL_REG01;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG02 VEDU_QPG_QP_LAMBDA_CTRL_REG02;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG03 VEDU_QPG_QP_LAMBDA_CTRL_REG03;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG04 VEDU_QPG_QP_LAMBDA_CTRL_REG04;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG05 VEDU_QPG_QP_LAMBDA_CTRL_REG05;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG06 VEDU_QPG_QP_LAMBDA_CTRL_REG06;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG07 VEDU_QPG_QP_LAMBDA_CTRL_REG07;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG08 VEDU_QPG_QP_LAMBDA_CTRL_REG08;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG09 VEDU_QPG_QP_LAMBDA_CTRL_REG09;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG10 VEDU_QPG_QP_LAMBDA_CTRL_REG10;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG11 VEDU_QPG_QP_LAMBDA_CTRL_REG11;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG12 VEDU_QPG_QP_LAMBDA_CTRL_REG12;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG13 VEDU_QPG_QP_LAMBDA_CTRL_REG13;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG14 VEDU_QPG_QP_LAMBDA_CTRL_REG14;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG15 VEDU_QPG_QP_LAMBDA_CTRL_REG15;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG16 VEDU_QPG_QP_LAMBDA_CTRL_REG16;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG17 VEDU_QPG_QP_LAMBDA_CTRL_REG17;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG18 VEDU_QPG_QP_LAMBDA_CTRL_REG18;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG19 VEDU_QPG_QP_LAMBDA_CTRL_REG19;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG20 VEDU_QPG_QP_LAMBDA_CTRL_REG20;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG21 VEDU_QPG_QP_LAMBDA_CTRL_REG21;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG22 VEDU_QPG_QP_LAMBDA_CTRL_REG22;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG23 VEDU_QPG_QP_LAMBDA_CTRL_REG23;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG24 VEDU_QPG_QP_LAMBDA_CTRL_REG24;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG25 VEDU_QPG_QP_LAMBDA_CTRL_REG25;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG26 VEDU_QPG_QP_LAMBDA_CTRL_REG26;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG27 VEDU_QPG_QP_LAMBDA_CTRL_REG27;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG28 VEDU_QPG_QP_LAMBDA_CTRL_REG28;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG29 VEDU_QPG_QP_LAMBDA_CTRL_REG29;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG30 VEDU_QPG_QP_LAMBDA_CTRL_REG30;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG31 VEDU_QPG_QP_LAMBDA_CTRL_REG31;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG32 VEDU_QPG_QP_LAMBDA_CTRL_REG32;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG33 VEDU_QPG_QP_LAMBDA_CTRL_REG33;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG34 VEDU_QPG_QP_LAMBDA_CTRL_REG34;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG35 VEDU_QPG_QP_LAMBDA_CTRL_REG35;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG36 VEDU_QPG_QP_LAMBDA_CTRL_REG36;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG37 VEDU_QPG_QP_LAMBDA_CTRL_REG37;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG38 VEDU_QPG_QP_LAMBDA_CTRL_REG38;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG39 VEDU_QPG_QP_LAMBDA_CTRL_REG39;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG40 VEDU_QPG_QP_LAMBDA_CTRL_REG40;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG41 VEDU_QPG_QP_LAMBDA_CTRL_REG41;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG42 VEDU_QPG_QP_LAMBDA_CTRL_REG42;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG43 VEDU_QPG_QP_LAMBDA_CTRL_REG43;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG44 VEDU_QPG_QP_LAMBDA_CTRL_REG44;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG45 VEDU_QPG_QP_LAMBDA_CTRL_REG45;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG46 VEDU_QPG_QP_LAMBDA_CTRL_REG46;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG47 VEDU_QPG_QP_LAMBDA_CTRL_REG47;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG48 VEDU_QPG_QP_LAMBDA_CTRL_REG48;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG49 VEDU_QPG_QP_LAMBDA_CTRL_REG49;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG50 VEDU_QPG_QP_LAMBDA_CTRL_REG50;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG51 VEDU_QPG_QP_LAMBDA_CTRL_REG51;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG52 VEDU_QPG_QP_LAMBDA_CTRL_REG52;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG53 VEDU_QPG_QP_LAMBDA_CTRL_REG53;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG54 VEDU_QPG_QP_LAMBDA_CTRL_REG54;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG55 VEDU_QPG_QP_LAMBDA_CTRL_REG55;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG56 VEDU_QPG_QP_LAMBDA_CTRL_REG56;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG57 VEDU_QPG_QP_LAMBDA_CTRL_REG57;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG58 VEDU_QPG_QP_LAMBDA_CTRL_REG58;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG59 VEDU_QPG_QP_LAMBDA_CTRL_REG59;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG60 VEDU_QPG_QP_LAMBDA_CTRL_REG60;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG61 VEDU_QPG_QP_LAMBDA_CTRL_REG61;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG62 VEDU_QPG_QP_LAMBDA_CTRL_REG62;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG63 VEDU_QPG_QP_LAMBDA_CTRL_REG63;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG64 VEDU_QPG_QP_LAMBDA_CTRL_REG64;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG65 VEDU_QPG_QP_LAMBDA_CTRL_REG65;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG66 VEDU_QPG_QP_LAMBDA_CTRL_REG66;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG67 VEDU_QPG_QP_LAMBDA_CTRL_REG67;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG68 VEDU_QPG_QP_LAMBDA_CTRL_REG68;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG69 VEDU_QPG_QP_LAMBDA_CTRL_REG69;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG70 VEDU_QPG_QP_LAMBDA_CTRL_REG70;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG71 VEDU_QPG_QP_LAMBDA_CTRL_REG71;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG72 VEDU_QPG_QP_LAMBDA_CTRL_REG72;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG73 VEDU_QPG_QP_LAMBDA_CTRL_REG73;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG74 VEDU_QPG_QP_LAMBDA_CTRL_REG74;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG75 VEDU_QPG_QP_LAMBDA_CTRL_REG75;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG76 VEDU_QPG_QP_LAMBDA_CTRL_REG76;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG77 VEDU_QPG_QP_LAMBDA_CTRL_REG77;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG78 VEDU_QPG_QP_LAMBDA_CTRL_REG78;
    volatile U_VEDU_QPG_QP_LAMBDA_CTRL_REG79 VEDU_QPG_QP_LAMBDA_CTRL_REG79;
    volatile U_VEDU_IME_RDOCFG VEDU_IME_RDOCFG;
    volatile U_VEDU_MRG_FORCE_ZERO_EN VEDU_MRG_FORCE_ZERO_EN;
    volatile unsigned int VEDU_VCPI_INTRA8_LOW_POWER;
    volatile U_VEDU_VCPI_LOW_POWER VEDU_VCPI_LOW_POWER;
    volatile U_VEDU_IME_INTER_MODE VEDU_IME_INTER_MODE;
    volatile U_VEDU_VCPI_PRE_JUDGE_EXT_EN VEDU_VCPI_PRE_JUDGE_EXT_EN;
    volatile U_VEDU_PME_SW_ADAPT_EN VEDU_PME_SW_ADAPT_EN;
    volatile U_VEDU_PME_WINDOW_SIZE0_L0 VEDU_PME_WINDOW_SIZE0_L0;
    volatile U_VEDU_PME_WINDOW_SIZE0_L1 VEDU_PME_WINDOW_SIZE0_L1;
    volatile U_VEDU_PME_SKIP_PRE VEDU_PME_SKIP_PRE;
    volatile U_VEDU_PME_PBLK_PRE1 VEDU_PME_PBLK_PRE1;
    volatile U_VEDU_VCPI_PRE_JUDGE_COST_THR VEDU_VCPI_PRE_JUDGE_COST_THR;
    volatile U_VEDU_VCPI_IBLK_PRE_MV_THR VEDU_VCPI_IBLK_PRE_MV_THR;
    volatile U_VEDU_PME_IBLK_COST_THR VEDU_PME_IBLK_COST_THR;
    volatile U_VEDU_PME_TR_WEIGHTX VEDU_PME_TR_WEIGHTX;
    volatile U_VEDU_PME_TR_WEIGHTY VEDU_PME_TR_WEIGHTY;
    volatile U_VEDU_PME_SR_WEIGHT VEDU_PME_SR_WEIGHT;
    volatile U_VEDU_PME_INTRA_LOWPOW VEDU_PME_INTRA_LOWPOW;
    volatile U_VEDU_IME_FME_LPOW_THR VEDU_IME_FME_LPOW_THR;
    volatile U_VEDU_PME_SKIP_FLAG VEDU_PME_SKIP_FLAG;
    volatile U_VEDU_PME_PSW_LPW VEDU_PME_PSW_LPW;
    volatile U_VEDU_PME_PBLK_PRE2 VEDU_PME_PBLK_PRE2;
    volatile U_VEDU_IME_LAYER3TO2_THR VEDU_IME_LAYER3TO2_THR;
    volatile U_VEDU_IME_LAYER3TO2_THR1 VEDU_IME_LAYER3TO2_THR1;
    volatile U_VEDU_IME_LAYER3TO1_THR VEDU_IME_LAYER3TO1_THR;
    volatile U_VEDU_IME_LAYER3TO1_THR1 VEDU_IME_LAYER3TO1_THR1;
    volatile U_VEDU_FME_PU64_LWP VEDU_FME_PU64_LWP;
    volatile U_VEDU_MRG_FORCE_SKIP_EN VEDU_MRG_FORCE_SKIP_EN;
    volatile U_VEDU_INTRA_CFG VEDU_INTRA_CFG;
    volatile U_VEDU_INTRA_LOW_POW VEDU_INTRA_LOW_POW;
    volatile unsigned int VEDU_VCPI_TUNLCELL_ADDR_L;
    volatile unsigned int VEDU_VCPI_TUNLCELL_ADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_YADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_YADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_EXT_YADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_EXT_YADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_CADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_CADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_EXT_CADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_EXT_CADDR_H;
    volatile unsigned int VEDU_VCPI_SRC_VADDR_L;
    volatile unsigned int VEDU_VCPI_SRC_VADDR_H;
    volatile unsigned int VEDU_VCPI_YH_ADDR_L;
    volatile unsigned int VEDU_VCPI_YH_ADDR_H;
    volatile unsigned int VEDU_VCPI_CH_ADDR_L;
    volatile unsigned int VEDU_VCPI_CH_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFY_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFY_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFC_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFC_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFYH_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFYH_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFCH_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFCH_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFY_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFY_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFC_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFC_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFYH_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFYH_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REFCH_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_REFCH_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_REC_YADDR_L;
    volatile unsigned int VEDU_VCPI_REC_YADDR_H;
    volatile unsigned int VEDU_VCPI_REC_CADDR_L;
    volatile unsigned int VEDU_VCPI_REC_CADDR_H;
    volatile unsigned int VEDU_VCPI_REC_YH_ADDR_L;
    volatile unsigned int VEDU_VCPI_REC_YH_ADDR_H;
    volatile unsigned int VEDU_VCPI_REC_CH_ADDR_L;
    volatile unsigned int VEDU_VCPI_REC_CH_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMELD_L0_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMELD_L0_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMELD_L1_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMELD_L1_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMEST_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEST_ADDR_H;
    volatile unsigned int VEDU_VCPI_NBI_MVST_ADDR_L;
    volatile unsigned int VEDU_VCPI_NBI_MVST_ADDR_H;
    volatile unsigned int VEDU_VCPI_NBI_MVLD_ADDR_L;
    volatile unsigned int VEDU_VCPI_NBI_MVLD_ADDR_H;
    volatile unsigned int VEDU_VCPI_STRMADDR_L;
    volatile unsigned int VEDU_VCPI_STRMADDR_H;
    volatile unsigned int VEDU_VCPI_SWPTRADDR_L;
    volatile unsigned int VEDU_VCPI_SWPTRADDR_H;
    volatile unsigned int VEDU_VCPI_SRPTRADDR_L;
    volatile unsigned int VEDU_VCPI_SRPTRADDR_H;
    volatile unsigned int VEDU_VCPI_PMEINFO_ST_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEINFO_ST_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD0_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD0_ADDR_H;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD1_ADDR_L;
    volatile unsigned int VEDU_VCPI_PMEINFO_LD1_ADDR_H;
    volatile unsigned int VEDU_VCPI_QPGLD_INF_ADDR_L;
    volatile unsigned int VEDU_VCPI_QPGLD_INF_ADDR_H;
    volatile unsigned int VEDU_LEFT_CELL_RADDR_L;
    volatile unsigned int VEDU_LEFT_CELL_RADDR_H;
    volatile unsigned int VEDU_TOP_CELL_RADDR_L;
    volatile unsigned int VEDU_TOP_CELL_RADDR_H;
    volatile unsigned int VEDU_CURR_CELL_WADDR_L;
    volatile unsigned int VEDU_CURR_CELL_WADDR_H;
    volatile unsigned int VEDU_TOPLEFT_CELL_RADDR_L;
    volatile unsigned int VEDU_TOPLEFT_CELL_RADDR_H;
    volatile unsigned int VEDU_BOT_RC_INFO_WADDR_L;
    volatile unsigned int VEDU_BOT_RC_INFO_WADDR_H;
    volatile unsigned int VEDU_RIGHT_RC_INFO_WADDR_L;
    volatile unsigned int VEDU_RIGHT_RC_INFO_WADDR_H;
    volatile unsigned int VEDU_RIGHT_BS_INFO_WADDR_L;
    volatile unsigned int VEDU_RIGHT_BS_INFO_WADDR_H;
    volatile unsigned int VEDU_LEFT_BS_INFO_RADDR_L;
    volatile unsigned int VEDU_LEFT_BS_INFO_RADDR_H;
    volatile unsigned int VEDU_BOT_TB_INFO_WADDR_L;
    volatile unsigned int VEDU_BOT_TB_INFO_WADDR_H;
    volatile unsigned int VEDU_RIGHT_TB_INFO_WADDR_L;
    volatile unsigned int VEDU_RIGHT_TB_INFO_WADDR_H;
    volatile unsigned int VEDU_BOTRIGHT_TB_INFO_WADDR_L;
    volatile unsigned int VEDU_BOTRIGHT_TB_INFO_WADDR_H;
    volatile unsigned int VEDU_TOP_RC_INFO_RADDR_L;
    volatile unsigned int VEDU_TOP_RC_INFO_RADDR_H;
    volatile unsigned int VEDU_LEFT_RC_INFO_RADDR_L;
    volatile unsigned int VEDU_LEFT_RC_INFO_RADDR_H;
    volatile unsigned int VEDU_TOP_TB_INFO_RADDR_L;
    volatile unsigned int VEDU_TOP_TB_INFO_RADDR_H;
    volatile unsigned int VEDU_LEFT_TB_INFO_RADDR_L;
    volatile unsigned int VEDU_LEFT_TB_INFO_RADDR_H;
    volatile unsigned int VEDU_TOPLEFT_TB_INFO_RADDR_L;
    volatile unsigned int VEDU_TOPLEFT_TB_INFO_RADDR_H;
    volatile unsigned int VEDU_PPFD_ST_ADDR0_L;
    volatile unsigned int VEDU_PPFD_ST_ADDR0_H;
    volatile unsigned int VEDU_SKIPWEIGHT_LD_ADDR_L;
    volatile unsigned int VEDU_SKIPWEIGHT_LD_ADDR_H;
    volatile U_VEDU_SAO_SSD_AREA0_START VEDU_SAO_SSD_AREA0_START;
    volatile U_VEDU_SAO_SSD_AREA0_END VEDU_SAO_SSD_AREA0_END;
    volatile U_VEDU_SAO_SSD_AREA1_START VEDU_SAO_SSD_AREA1_START;
    volatile U_VEDU_SAO_SSD_AREA1_END VEDU_SAO_SSD_AREA1_END;
    volatile U_VEDU_SAO_SSD_AREA2_START VEDU_SAO_SSD_AREA2_START;
    volatile U_VEDU_SAO_SSD_AREA2_END VEDU_SAO_SSD_AREA2_END;
    volatile U_VEDU_SAO_SSD_AREA3_START VEDU_SAO_SSD_AREA3_START;
    volatile U_VEDU_SAO_SSD_AREA3_END VEDU_SAO_SSD_AREA3_END;
    volatile U_VEDU_SAO_SSD_AREA4_START VEDU_SAO_SSD_AREA4_START;
    volatile U_VEDU_SAO_SSD_AREA4_END VEDU_SAO_SSD_AREA4_END;
    volatile U_VEDU_SAO_SSD_AREA5_START VEDU_SAO_SSD_AREA5_START;
    volatile U_VEDU_SAO_SSD_AREA5_END VEDU_SAO_SSD_AREA5_END;
    volatile U_VEDU_SAO_SSD_AREA6_START VEDU_SAO_SSD_AREA6_START;
    volatile U_VEDU_SAO_SSD_AREA6_END VEDU_SAO_SSD_AREA6_END;
    volatile U_VEDU_SAO_SSD_AREA7_START VEDU_SAO_SSD_AREA7_START;
    volatile U_VEDU_SAO_SSD_AREA7_END VEDU_SAO_SSD_AREA7_END;
    volatile U_VEDU_VCPI_STRIDE VEDU_VCPI_STRIDE;
    volatile U_VEDU_VCPI_EXT_STRIDE VEDU_VCPI_EXT_STRIDE;
    volatile U_VEDU_VCPI_REF_L0_STRIDE VEDU_VCPI_REF_L0_STRIDE;
    volatile U_VEDU_VCPI_REF_L1_STRIDE VEDU_VCPI_REF_L1_STRIDE;
    volatile U_VEDU_VCPI_REC_STRIDE VEDU_VCPI_REC_STRIDE;
    volatile unsigned int VEDU_VCPI_PMEST_STRIDE;
    volatile unsigned int VEDU_VCPI_PMELD_STRIDE;
    volatile U_VEDU_GHDR_CTRL VEDU_GHDR_CTRL;
    volatile U_VEDU_GHDR_DEGAMMA_CTRL VEDU_GHDR_DEGAMMA_CTRL;
    volatile U_VEDU_GHDR_DEGAMMA_STEP VEDU_GHDR_DEGAMMA_STEP;
    volatile U_VEDU_GHDR_DEGAMMA_POS1 VEDU_GHDR_DEGAMMA_POS1;
    volatile U_VEDU_GHDR_DEGAMMA_POS2 VEDU_GHDR_DEGAMMA_POS2;
    volatile U_VEDU_GHDR_DEGAMMA_NUM VEDU_GHDR_DEGAMMA_NUM;
    volatile U_VEDU_GHDR_GAMUT_CTRL VEDU_GHDR_GAMUT_CTRL;
    volatile U_VEDU_GHDR_GAMUT_COEF00 VEDU_GHDR_GAMUT_COEF00;
    volatile U_VEDU_GHDR_GAMUT_COEF01 VEDU_GHDR_GAMUT_COEF01;
    volatile U_VEDU_GHDR_GAMUT_COEF02 VEDU_GHDR_GAMUT_COEF02;
    volatile U_VEDU_GHDR_GAMUT_COEF10 VEDU_GHDR_GAMUT_COEF10;
    volatile U_VEDU_GHDR_GAMUT_COEF11 VEDU_GHDR_GAMUT_COEF11;
    volatile U_VEDU_GHDR_GAMUT_COEF12 VEDU_GHDR_GAMUT_COEF12;
    volatile U_VEDU_GHDR_GAMUT_COEF20 VEDU_GHDR_GAMUT_COEF20;
    volatile U_VEDU_GHDR_GAMUT_COEF21 VEDU_GHDR_GAMUT_COEF21;
    volatile U_VEDU_GHDR_GAMUT_COEF22 VEDU_GHDR_GAMUT_COEF22;
    volatile U_VEDU_GHDR_GAMUT_SCALE VEDU_GHDR_GAMUT_SCALE;
    volatile U_VEDU_GHDR_GAMUT_CLIP_MIN VEDU_GHDR_GAMUT_CLIP_MIN;
    volatile U_VEDU_GHDR_GAMUT_CLIP_MAX VEDU_GHDR_GAMUT_CLIP_MAX;
    volatile U_VEDU_GHDR_TONEMAP_CTRL VEDU_GHDR_TONEMAP_CTRL;
    volatile U_VEDU_GHDR_TONEMAP_REN VEDU_GHDR_TONEMAP_REN;
    volatile unsigned int VEDU_GHDR_RESERVED0;
    volatile U_VEDU_GHDR_TONEMAP_STEP VEDU_GHDR_TONEMAP_STEP;
    volatile U_VEDU_GHDR_TONEMAP_POS1 VEDU_GHDR_TONEMAP_POS1;
    volatile U_VEDU_GHDR_TONEMAP_POS2 VEDU_GHDR_TONEMAP_POS2;
    volatile U_VEDU_GHDR_TONEMAP_POS3 VEDU_GHDR_TONEMAP_POS3;
    volatile U_VEDU_GHDR_TONEMAP_POS4 VEDU_GHDR_TONEMAP_POS4;
    volatile U_VEDU_GHDR_TONEMAP_NUM VEDU_GHDR_TONEMAP_NUM;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_COEF0 VEDU_GHDR_TONEMAP_LUMA_COEF0;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_COEF1 VEDU_GHDR_TONEMAP_LUMA_COEF1;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_COEF2 VEDU_GHDR_TONEMAP_LUMA_COEF2;
    volatile U_VEDU_GHDR_TONEMAP_LUMA_SCALE VEDU_GHDR_TONEMAP_LUMA_SCALE;
    volatile U_VEDU_GHDR_TONEMAP_COEF_SCALE VEDU_GHDR_TONEMAP_COEF_SCALE;
    volatile U_VEDU_GHDR_TONEMAP_OUT_CLIP_MIN VEDU_GHDR_TONEMAP_OUT_CLIP_MIN;
    volatile U_VEDU_GHDR_TONEMAP_OUT_CLIP_MAX VEDU_GHDR_TONEMAP_OUT_CLIP_MAX;
    volatile U_VEDU_GHDR_GAMMA_CTRL VEDU_GHDR_GAMMA_CTRL;
    volatile U_VEDU_GHDR_GAMMA_REN VEDU_GHDR_GAMMA_REN;
    volatile unsigned int VEDU_GHDR_RESERVED1;
    volatile U_VEDU_GHDR_GAMMA_STEP1 VEDU_GHDR_GAMMA_STEP1;
    volatile U_VEDU_GHDR_GAMMA_STEP2 VEDU_GHDR_GAMMA_STEP2;
    volatile U_VEDU_GHDR_GAMMA_POS1 VEDU_GHDR_GAMMA_POS1;
    volatile U_VEDU_GHDR_GAMMA_POS2 VEDU_GHDR_GAMMA_POS2;
    volatile U_VEDU_GHDR_GAMMA_POS3 VEDU_GHDR_GAMMA_POS3;
    volatile U_VEDU_GHDR_GAMMA_POS4 VEDU_GHDR_GAMMA_POS4;
    volatile U_VEDU_GHDR_GAMMA_NUM1 VEDU_GHDR_GAMMA_NUM1;
    volatile U_VEDU_GHDR_GAMMA_NUM2 VEDU_GHDR_GAMMA_NUM2;
    volatile U_VEDU_GHDR_DITHER_CTRL VEDU_GHDR_DITHER_CTRL;
    volatile U_VEDU_GHDR_DITHER_THR VEDU_GHDR_DITHER_THR;
    volatile U_VEDU_GHDR_DITHER_SED_Y0 VEDU_GHDR_DITHER_SED_Y0;
    volatile U_VEDU_GHDR_DITHER_SED_U0 VEDU_GHDR_DITHER_SED_U0;
    volatile U_VEDU_GHDR_DITHER_SED_V0 VEDU_GHDR_DITHER_SED_V0;
    volatile U_VEDU_GHDR_DITHER_SED_W0 VEDU_GHDR_DITHER_SED_W0;
    volatile U_VEDU_GHDR_DITHER_SED_Y1 VEDU_GHDR_DITHER_SED_Y1;
    volatile U_VEDU_GHDR_DITHER_SED_U1 VEDU_GHDR_DITHER_SED_U1;
    volatile U_VEDU_GHDR_DITHER_SED_V1 VEDU_GHDR_DITHER_SED_V1;
    volatile U_VEDU_GHDR_DITHER_SED_W1 VEDU_GHDR_DITHER_SED_W1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_CTRL VEDU_HIHDR_G_RGB2YUV_CTRL;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF0 VEDU_HIHDR_G_RGB2YUV_COEF0;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF1 VEDU_HIHDR_G_RGB2YUV_COEF1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF2 VEDU_HIHDR_G_RGB2YUV_COEF2;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF3 VEDU_HIHDR_G_RGB2YUV_COEF3;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF4 VEDU_HIHDR_G_RGB2YUV_COEF4;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF5 VEDU_HIHDR_G_RGB2YUV_COEF5;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF6 VEDU_HIHDR_G_RGB2YUV_COEF6;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF7 VEDU_HIHDR_G_RGB2YUV_COEF7;
    volatile U_VEDU_HIHDR_G_RGB2YUV_COEF8 VEDU_HIHDR_G_RGB2YUV_COEF8;
    volatile U_VEDU_HIHDR_G_RGB2YUV_SCALE2P VEDU_HIHDR_G_RGB2YUV_SCALE2P;
    volatile U_VEDU_HIHDR_G_RGB2YUV_IDC0 VEDU_HIHDR_G_RGB2YUV_IDC0;
    volatile U_VEDU_HIHDR_G_RGB2YUV_IDC1 VEDU_HIHDR_G_RGB2YUV_IDC1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_IDC2 VEDU_HIHDR_G_RGB2YUV_IDC2;
    volatile U_VEDU_HIHDR_G_RGB2YUV_ODC0 VEDU_HIHDR_G_RGB2YUV_ODC0;
    volatile U_VEDU_HIHDR_G_RGB2YUV_ODC1 VEDU_HIHDR_G_RGB2YUV_ODC1;
    volatile U_VEDU_HIHDR_G_RGB2YUV_ODC2 VEDU_HIHDR_G_RGB2YUV_ODC2;
    volatile U_VEDU_HIHDR_G_RGB2YUV_MIN VEDU_HIHDR_G_RGB2YUV_MIN;
    volatile U_VEDU_HIHDR_G_RGB2YUV_MAX VEDU_HIHDR_G_RGB2YUV_MAX;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_00_01 VEDU_HIHDR_G_TONEMAP_COEF_00_01;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_02_03 VEDU_HIHDR_G_TONEMAP_COEF_02_03;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_04_05 VEDU_HIHDR_G_TONEMAP_COEF_04_05;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_06_07 VEDU_HIHDR_G_TONEMAP_COEF_06_07;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_08_09 VEDU_HIHDR_G_TONEMAP_COEF_08_09;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_10_11 VEDU_HIHDR_G_TONEMAP_COEF_10_11;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_12_13 VEDU_HIHDR_G_TONEMAP_COEF_12_13;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_14_15 VEDU_HIHDR_G_TONEMAP_COEF_14_15;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_16_17 VEDU_HIHDR_G_TONEMAP_COEF_16_17;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_18_19 VEDU_HIHDR_G_TONEMAP_COEF_18_19;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_20_21 VEDU_HIHDR_G_TONEMAP_COEF_20_21;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_22_23 VEDU_HIHDR_G_TONEMAP_COEF_22_23;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_24_25 VEDU_HIHDR_G_TONEMAP_COEF_24_25;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_26_27 VEDU_HIHDR_G_TONEMAP_COEF_26_27;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_28_29 VEDU_HIHDR_G_TONEMAP_COEF_28_29;
    volatile U_VEDU_HIHDR_G_TONEMAP_COEF_30_31 VEDU_HIHDR_G_TONEMAP_COEF_30_31;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P0 VEDU_HIHDR_G_GMM_COEF_00_09_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P1 VEDU_HIHDR_G_GMM_COEF_00_09_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P2 VEDU_HIHDR_G_GMM_COEF_00_09_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_00_09_P3 VEDU_HIHDR_G_GMM_COEF_00_09_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P0 VEDU_HIHDR_G_GMM_COEF_10_19_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P1 VEDU_HIHDR_G_GMM_COEF_10_19_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P2 VEDU_HIHDR_G_GMM_COEF_10_19_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_10_19_P3 VEDU_HIHDR_G_GMM_COEF_10_19_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P0 VEDU_HIHDR_G_GMM_COEF_20_29_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P1 VEDU_HIHDR_G_GMM_COEF_20_29_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P2 VEDU_HIHDR_G_GMM_COEF_20_29_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_20_29_P3 VEDU_HIHDR_G_GMM_COEF_20_29_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P0 VEDU_HIHDR_G_GMM_COEF_30_39_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P1 VEDU_HIHDR_G_GMM_COEF_30_39_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P2 VEDU_HIHDR_G_GMM_COEF_30_39_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_30_39_P3 VEDU_HIHDR_G_GMM_COEF_30_39_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P0 VEDU_HIHDR_G_GMM_COEF_40_49_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P1 VEDU_HIHDR_G_GMM_COEF_40_49_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P2 VEDU_HIHDR_G_GMM_COEF_40_49_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_40_49_P3 VEDU_HIHDR_G_GMM_COEF_40_49_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P0 VEDU_HIHDR_G_GMM_COEF_50_59_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P1 VEDU_HIHDR_G_GMM_COEF_50_59_P1;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P2 VEDU_HIHDR_G_GMM_COEF_50_59_P2;
    volatile U_VEDU_HIHDR_G_GMM_COEF_50_59_P3 VEDU_HIHDR_G_GMM_COEF_50_59_P3;
    volatile U_VEDU_HIHDR_G_GMM_COEF_60_69_P0 VEDU_HIHDR_G_GMM_COEF_60_69_P0;
    volatile U_VEDU_HIHDR_G_GMM_COEF_60_69_P1 VEDU_HIHDR_G_GMM_COEF_60_69_P1;
    volatile U_VEDU_SECURE_SET VEDU_SECURE_SET;
    volatile U_FUNC_VCPI_INTSTAT FUNC_VCPI_INTSTAT;
    volatile U_FUNC_VCPI_RAWINT FUNC_VCPI_RAWINT;
    volatile unsigned int FUNC_VCPI_VEDU_TIMER;
    volatile unsigned int FUNC_VCPI_IDLE_TIMER;
    volatile U_FUNC_VCPI_BUS_IDLE_FLAG FUNC_VCPI_BUS_IDLE_FLAG;
    volatile U_FUNC_PME_MADI_SUM FUNC_PME_MADI_SUM;
    volatile U_FUNC_PME_MADP_SUM FUNC_PME_MADP_SUM;
    volatile U_FUNC_PME_MADI_NUM FUNC_PME_MADI_NUM;
    volatile U_FUNC_PME_MADP_NUM FUNC_PME_MADP_NUM;
    volatile U_FUNC_PME_LARGE_SAD_SUM FUNC_PME_LARGE_SAD_SUM;
    volatile U_FUNC_PME_LOW_LUMA_SUM FUNC_PME_LOW_LUMA_SUM;
    volatile U_FUNC_PME_CHROMA_SCENE_SUM FUNC_PME_CHROMA_SCENE_SUM;
    volatile U_FUNC_PME_MOVE_SCENE_SUM FUNC_PME_MOVE_SCENE_SUM;
    volatile U_FUNC_PME_SKIN_REGION_SUM FUNC_PME_SKIN_REGION_SUM;
    volatile U_FUNC_BGSTR_BLOCK_COUNT FUNC_BGSTR_BLOCK_COUNT;
    volatile U_FUNC_BGSTR_FRAME_BGM_DIST FUNC_BGSTR_FRAME_BGM_DIST;
    volatile unsigned int FUNC_PMV_MV0_COUNT;
    volatile unsigned int FUNC_PMV_MV1_COUNT;
    volatile unsigned int FUNC_PMV_MV2_COUNT;
    volatile unsigned int FUNC_PMV_MV3_COUNT;
    volatile unsigned int FUNC_PMV_MV4_COUNT;
    volatile unsigned int FUNC_PMV_MV5_COUNT;
    volatile unsigned int FUNC_PMV_MV6_COUNT;
    volatile unsigned int FUNC_PMV_MV7_COUNT;
    volatile unsigned int FUNC_PMV_MV8_COUNT;
    volatile unsigned int FUNC_PMV_MV9_COUNT;
    volatile unsigned int FUNC_PMV_MV10_COUNT;
    volatile unsigned int FUNC_PMV_MV11_COUNT;
    volatile unsigned int FUNC_PMV_MV12_COUNT;
    volatile unsigned int FUNC_PMV_MV13_COUNT;
    volatile unsigned int FUNC_PMV_MV14_COUNT;
    volatile unsigned int FUNC_PMV_MV15_COUNT;
    volatile unsigned int FUNC_PMV_MV16_COUNT;
    volatile unsigned int FUNC_PMV_MV17_COUNT;
    volatile unsigned int FUNC_PMV_MV18_COUNT;
    volatile unsigned int FUNC_PMV_MV19_COUNT;
    volatile unsigned int FUNC_PMV_MV20_COUNT;
    volatile unsigned int FUNC_PMV_MV21_COUNT;
    volatile unsigned int FUNC_PMV_MV22_COUNT;
    volatile unsigned int FUNC_CABAC_PIC_STRMSIZE;
    volatile unsigned int FUNC_CABAC_BIN_NUM;
    volatile unsigned int FUNC_CABAC_RES_BIN_NUM;
    volatile unsigned int FUNC_CABAC_HDR_BIN_NUM;
    volatile unsigned int FUNC_CABAC_MOVE_SCENE_BITS;
    volatile unsigned int FUNC_CABAC_STREDGE_MOVE_BITS;
    volatile unsigned int FUNC_CABAC_SKIN_BITS;
    volatile unsigned int FUNC_CABAC_LOWLUMA_BITS;
    volatile unsigned int FUNC_CABAC_CHROMAPROT_BITS;
    volatile unsigned int FUNC_VLC_CABAC_HRD_BITS;
    volatile unsigned int FUNC_VLC_CABAC_RES_BITS;
    volatile unsigned int FUNC_VLC_CAVLC_HRD_BITS;
    volatile unsigned int FUNC_VLC_CAVLC_RES_BITS;
    volatile unsigned int FUNC_VLC_PIC_STRMSIZE;
    volatile unsigned int FUNC_VLCST_SLC_LEN_CNT;
    volatile unsigned int FUNC_VLCST_WPTR;
    volatile unsigned int FUNC_VLCST_RPTR;
    volatile U_FUNC_SEL_OPT_8X8_CNT FUNC_SEL_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_8X8_CNT FUNC_SEL_INTRA_OPT_8X8_CNT;
    volatile unsigned int FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTRA_PCM_OPT_8X8_CNT FUNC_SEL_INTRA_PCM_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_OPT_8X8_CNT FUNC_SEL_INTER_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_8X8_CNT FUNC_SEL_INTER_FME_OPT_8X8_CNT;
    volatile U_FUNC_SELINTER_MERGE_OPT_8X8_CNT FUNC_SELINTER_MERGE_OPT_8X8_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_8X8_CNT FUNC_SEL_INTER_SKIP_OPT_8X8_CNT;
    volatile U_FUNC_SEL_OPT_16X16_CNT FUNC_SEL_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_16X16_CNT FUNC_SEL_INTRA_OPT_16X16_CNT;
    volatile U_FUNC_SEL_OPT_4X4_CNT FUNC_SEL_OPT_4X4_CNT;
    volatile U_FUNC_SEL_INTER_OPT_16X16_CNT FUNC_SEL_INTER_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_16X16_CNT FUNC_SEL_INTER_FME_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_16X16_CNT FUNC_SEL_INTER_MERGE_OPT_16X16_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_16X16_CNT FUNC_SEL_INTER_SKIP_OPT_16X16_CNT;
    volatile U_FUNC_SEL_OPT_32X32_CNT FUNC_SEL_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTRA_OPT_32X32_CNT FUNC_SEL_INTRA_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_OPT_32X32_CNT FUNC_SEL_INTER_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_32X32_CNT FUNC_SEL_INTER_FME_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_32X32_CNT FUNC_SEL_INTER_MERGE_OPT_32X32_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_32X32_CNT FUNC_SEL_INTER_SKIP_OPT_32X32_CNT;
    volatile U_FUNC_SEL_OPT_64X64_CNT FUNC_SEL_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_FME_OPT_64X64_CNT FUNC_SEL_INTER_FME_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_MERGE_OPT_64X64_CNT FUNC_SEL_INTER_MERGE_OPT_64X64_CNT;
    volatile U_FUNC_SEL_INTER_SKIP_OPT_64X64_CNT FUNC_SEL_INTER_SKIP_OPT_64X64_CNT;
    volatile U_FUNC_SEL_TOTAL_LUMA_QP FUNC_SEL_TOTAL_LUMA_QP;
    volatile U_FUNC_SEL_MAX_MIN_LUMA_QP FUNC_SEL_MAX_MIN_LUMA_QP;
    volatile U_FUNC_SEL_LUMA_QP0_CNT FUNC_SEL_LUMA_QP0_CNT;
    volatile U_FUNC_SEL_LUMA_QP1_CNT FUNC_SEL_LUMA_QP1_CNT;
    volatile U_FUNC_SEL_LUMA_QP2_CNT FUNC_SEL_LUMA_QP2_CNT;
    volatile U_FUNC_SEL_LUMA_QP3_CNT FUNC_SEL_LUMA_QP3_CNT;
    volatile U_FUNC_SEL_LUMA_QP4_CNT FUNC_SEL_LUMA_QP4_CNT;
    volatile U_FUNC_SEL_LUMA_QP5_CNT FUNC_SEL_LUMA_QP5_CNT;
    volatile U_FUNC_SEL_LUMA_QP6_CNT FUNC_SEL_LUMA_QP6_CNT;
    volatile U_FUNC_SEL_LUMA_QP7_CNT FUNC_SEL_LUMA_QP7_CNT;
    volatile U_FUNC_SEL_LUMA_QP8_CNT FUNC_SEL_LUMA_QP8_CNT;
    volatile U_FUNC_SEL_LUMA_QP9_CNT FUNC_SEL_LUMA_QP9_CNT;
    volatile U_FUNC_SEL_LUMA_QP10_CNT FUNC_SEL_LUMA_QP10_CNT;
    volatile U_FUNC_SEL_LUMA_QP11_CNT FUNC_SEL_LUMA_QP11_CNT;
    volatile U_FUNC_SEL_LUMA_QP12_CNT FUNC_SEL_LUMA_QP12_CNT;
    volatile U_FUNC_SEL_LUMA_QP13_CNT FUNC_SEL_LUMA_QP13_CNT;
    volatile U_FUNC_SEL_LUMA_QP14_CNT FUNC_SEL_LUMA_QP14_CNT;
    volatile U_FUNC_SEL_LUMA_QP15_CNT FUNC_SEL_LUMA_QP15_CNT;
    volatile U_FUNC_SEL_LUMA_QP16_CNT FUNC_SEL_LUMA_QP16_CNT;
    volatile U_FUNC_SEL_LUMA_QP17_CNT FUNC_SEL_LUMA_QP17_CNT;
    volatile U_FUNC_SEL_LUMA_QP18_CNT FUNC_SEL_LUMA_QP18_CNT;
    volatile U_FUNC_SEL_LUMA_QP19_CNT FUNC_SEL_LUMA_QP19_CNT;
    volatile U_FUNC_SEL_LUMA_QP20_CNT FUNC_SEL_LUMA_QP20_CNT;
    volatile U_FUNC_SEL_LUMA_QP21_CNT FUNC_SEL_LUMA_QP21_CNT;
    volatile U_FUNC_SEL_LUMA_QP22_CNT FUNC_SEL_LUMA_QP22_CNT;
    volatile U_FUNC_SEL_LUMA_QP23_CNT FUNC_SEL_LUMA_QP23_CNT;
    volatile U_FUNC_SEL_LUMA_QP24_CNT FUNC_SEL_LUMA_QP24_CNT;
    volatile U_FUNC_SEL_LUMA_QP25_CNT FUNC_SEL_LUMA_QP25_CNT;
    volatile U_FUNC_SEL_LUMA_QP26_CNT FUNC_SEL_LUMA_QP26_CNT;
    volatile U_FUNC_SEL_LUMA_QP27_CNT FUNC_SEL_LUMA_QP27_CNT;
    volatile U_FUNC_SEL_LUMA_QP28_CNT FUNC_SEL_LUMA_QP28_CNT;
    volatile U_FUNC_SEL_LUMA_QP29_CNT FUNC_SEL_LUMA_QP29_CNT;
    volatile U_FUNC_SEL_LUMA_QP30_CNT FUNC_SEL_LUMA_QP30_CNT;
    volatile U_FUNC_SEL_LUMA_QP31_CNT FUNC_SEL_LUMA_QP31_CNT;
    volatile U_FUNC_SEL_LUMA_QP32_CNT FUNC_SEL_LUMA_QP32_CNT;
    volatile U_FUNC_SEL_LUMA_QP33_CNT FUNC_SEL_LUMA_QP33_CNT;
    volatile U_FUNC_SEL_LUMA_QP34_CNT FUNC_SEL_LUMA_QP34_CNT;
    volatile U_FUNC_SEL_LUMA_QP35_CNT FUNC_SEL_LUMA_QP35_CNT;
    volatile U_FUNC_SEL_LUMA_QP36_CNT FUNC_SEL_LUMA_QP36_CNT;
    volatile U_FUNC_SEL_LUMA_QP37_CNT FUNC_SEL_LUMA_QP37_CNT;
    volatile U_FUNC_SEL_LUMA_QP38_CNT FUNC_SEL_LUMA_QP38_CNT;
    volatile U_FUNC_SEL_LUMA_QP39_CNT FUNC_SEL_LUMA_QP39_CNT;
    volatile U_FUNC_SEL_LUMA_QP40_CNT FUNC_SEL_LUMA_QP40_CNT;
    volatile U_FUNC_SEL_LUMA_QP41_CNT FUNC_SEL_LUMA_QP41_CNT;
    volatile U_FUNC_SEL_LUMA_QP42_CNT FUNC_SEL_LUMA_QP42_CNT;
    volatile U_FUNC_SEL_LUMA_QP43_CNT FUNC_SEL_LUMA_QP43_CNT;
    volatile U_FUNC_SEL_LUMA_QP44_CNT FUNC_SEL_LUMA_QP44_CNT;
    volatile U_FUNC_SEL_LUMA_QP45_CNT FUNC_SEL_LUMA_QP45_CNT;
    volatile U_FUNC_SEL_LUMA_QP46_CNT FUNC_SEL_LUMA_QP46_CNT;
    volatile U_FUNC_SEL_LUMA_QP47_CNT FUNC_SEL_LUMA_QP47_CNT;
    volatile U_FUNC_SEL_LUMA_QP48_CNT FUNC_SEL_LUMA_QP48_CNT;
    volatile U_FUNC_SEL_LUMA_QP49_CNT FUNC_SEL_LUMA_QP49_CNT;
    volatile U_FUNC_SEL_LUMA_QP50_CNT FUNC_SEL_LUMA_QP50_CNT;
    volatile U_FUNC_SEL_LUMA_QP51_CNT FUNC_SEL_LUMA_QP51_CNT;
    volatile U_FUNC_SAO_OFF_NUM FUNC_SAO_OFF_NUM;
    volatile unsigned int FUNC_SAO_MSE_SUM;
    volatile U_FUNC_SAO_MSE_CNT FUNC_SAO_MSE_CNT;
    volatile U_FUNC_SAO_MSE_MAX FUNC_SAO_MSE_MAX;
    volatile unsigned int FUNC_SAO_SSD_AREA0_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA1_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA2_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA3_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA4_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA5_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA6_SUM;
    volatile unsigned int FUNC_SAO_SSD_AREA7_SUM;
    volatile U_AXIDFX_ERR AXIDFX_ERR;
    volatile U_AXIDFX_AR_R_CNT AXIDFX_AR_R_CNT;
    volatile U_AXIDFX_AW_W_CNT AXIDFX_AW_W_CNT;
    volatile U_AXIDFX_AW_B_CNT AXIDFX_AW_B_CNT;
    volatile U_AXIDFX_AR_R_ID_ERR AXIDFX_AR_R_ID_ERR;
    volatile U_AXIDFX_ERR_ARID AXIDFX_ERR_ARID;
    volatile U_AXIDFX_ERR_RID AXIDFX_ERR_RID;
    volatile U_AXIDFX_AW_W_B_ID_ERR AXIDFX_AW_W_B_ID_ERR;
    volatile U_AXIDFX_ERR_AWID AXIDFX_ERR_AWID;
    volatile U_AXIDFX_ERR_WID AXIDFX_ERR_WID;
    volatile U_AXIDFX_ERR_BID AXIDFX_ERR_BID;
    volatile U_AXIDFX_ARID_TX_0ERR AXIDFX_ARID_TX_0ERR;
    volatile U_AXIDFX_ARID_TX_1ERR AXIDFX_ARID_TX_1ERR;
    volatile U_AXIDFX_ARID_TX_2ERR AXIDFX_ARID_TX_2ERR;
    volatile U_AXIDFX_RID_RX_0ERR AXIDFX_RID_RX_0ERR;
    volatile U_AXIDFX_RID_RX_1ERR AXIDFX_RID_RX_1ERR;
    volatile U_AXIDFX_RID_RX_2ERR AXIDFX_RID_RX_2ERR;
    volatile U_AXIDFX_ARID_RX_0ERR AXIDFX_ARID_RX_0ERR;
    volatile U_AXIDFX_BID_RX_ERR AXIDFX_BID_RX_ERR;
    volatile U_AXIDFX_ARID_LEN_0ERR AXIDFX_ARID_LEN_0ERR;
    volatile U_AXIDFX_ARID_LEN_1ERR AXIDFX_ARID_LEN_1ERR;
    volatile U_AXIDFX_ARID_LEN_2ERR AXIDFX_ARID_LEN_2ERR;
    volatile unsigned int AXIDFX_AWLEN_CNT;
    volatile unsigned int AXIDFX_WLEN_CNT;
    volatile U_AXIDFX_RESP_ERR AXIDFX_RESP_ERR;
    volatile U_AXIDFX_ERR_RESP AXIDFX_ERR_RESP;
    volatile U_AXIDFX_LEN_ERR AXIDFX_LEN_ERR;
    volatile U_AXIDFX_ERR_LEN AXIDFX_ERR_LEN;
    volatile unsigned int AXIDFX_0RID_FLAG;
    volatile unsigned int AXIDFX_1RID_FLAG;
    volatile U_AXIDFX_2RID_FLAG AXIDFX_2RID_FLAG;
    volatile U_AXIDFX_WID_FLAG AXIDFX_WID_FLAG;
    volatile U_AXIDFX_AXI_ST AXIDFX_AXI_ST;
    volatile U_AXIDFX_SOFT_RST_REQ AXIDFX_SOFT_RST_REQ;
    volatile U_AXIDFX_SOFT_RST_ACK AXIDFX_SOFT_RST_ACK;
    volatile U_AXIDFX_SOFT_RST_FORCE_REQ_ACK AXIDFX_SOFT_RST_FORCE_REQ_ACK;
    volatile unsigned int AXIDFX_SOFT_RST_STATE0;
    volatile unsigned int AXIDFX_SOFT_RST_STATE1;
    volatile unsigned int RESERVED_S_NS_0;
    volatile unsigned int RESERVED_S_NS_1;
    volatile unsigned int RESERVED_S_NS_2;
    volatile unsigned int RESERVED_S_NS_3;
    volatile unsigned int RESERVED_S_NS_4;
    volatile unsigned int RESERVED_S_NS_5;
    volatile unsigned int RESERVED_S_NS_6;
    volatile unsigned int RESERVED_S_NS_7;
} S_HEVC_AVC_REGS_TYPE_CFG;
#endif
