// ******************************************************************************
// Copyright     :  Copyright (C) 2018, Hisilicon Technologies Co. Ltd.
// File name     :  axi_jpeg_drv_priv.h
// Project line  :  ISP
// Department    :
// Author        :  Anthony Sixta
// Version       :  1.0
// Date          :  2011/11/29
// Description   :  Image Sensor processing
// Others        :  Generated automatically by nManager V4.0.2.5
// History       :  Anthony Sixta 2018/08/19 15:32:36 Create file
// ******************************************************************************

#ifndef __AXI_JPEG_DRV_PRIV_CS_H__
#define __AXI_JPEG_DRV_PRIV_CS_H__

/* Define the union U_AXI_JPEG_CVDR_CFG */
union u_axi_jpeg_cvdr_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    axiwrite_du_threshold : 6   ; /* [5..0]  */
		unsigned int    reserved_0            : 2   ; /* [7..6]  */
		unsigned int    du_threshold_reached  : 8   ; /* [15..8]  */
		unsigned int    max_axiread_id        : 6   ; /* [21..16]  */
		unsigned int    reserved_1            : 2   ; /* [23..22]  */
		unsigned int    max_axiwrite_id       : 5   ; /* [28..24]  */
		unsigned int    reserved_2            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_CVDR_DEBUG_EN */
union u_axi_jpeg_cvdr_debug_en {
	/* Define the struct bits */
	struct {
		unsigned int    wr_peak_en            : 1   ; /* [0]  */
		unsigned int    reserved_0            : 7   ; /* [7..1]  */
		unsigned int    rd_peak_en            : 1   ; /* [8]  */
		unsigned int    reserved_1            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_CVDR_DEBUG */
union u_axi_jpeg_cvdr_debug {
	/* Define the struct bits */
	struct {
		unsigned int    wr_peak               : 8   ; /* [7..0]  */
		unsigned int    rd_peak               : 8   ; /* [15..8]  */
		unsigned int    reserved_0            : 16  ; /* [31..16]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_CVDR_WR_QOS_CFG */
union u_axi_jpeg_cvdr_wr_qos_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    wr_qos_threshold_01_stop : 4   ; /* [3..0]  */
		unsigned int    wr_qos_threshold_01_start : 4   ; /* [7..4]  */
		unsigned int    wr_qos_threshold_10_stop : 4   ; /* [11..8]  */
		unsigned int    wr_qos_threshold_10_start : 4   ; /* [15..12]  */
		unsigned int    wr_qos_threshold_11_stop : 4   ; /* [19..16]  */
		unsigned int    wr_qos_threshold_11_start : 4   ; /* [23..20]  */
		unsigned int    reserved_0            : 2   ; /* [25..24]  */
		unsigned int    wr_qos_min            : 2   ; /* [27..26]  */
		unsigned int    wr_qos_max            : 2   ; /* [29..28]  */
		unsigned int    wr_qos_sr             : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_CVDR_RD_QOS_CFG */
union u_axi_jpeg_cvdr_rd_qos_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    rd_qos_threshold_01_stop : 4   ; /* [3..0]  */
		unsigned int    rd_qos_threshold_01_start : 4   ; /* [7..4]  */
		unsigned int    rd_qos_threshold_10_stop : 4   ; /* [11..8]  */
		unsigned int    rd_qos_threshold_10_start : 4   ; /* [15..12]  */
		unsigned int    rd_qos_threshold_11_stop : 4   ; /* [19..16]  */
		unsigned int    rd_qos_threshold_11_start : 4   ; /* [23..20]  */
		unsigned int    reserved_0            : 2   ; /* [25..24]  */
		unsigned int    rd_qos_min            : 2   ; /* [27..26]  */
		unsigned int    rd_qos_max            : 2   ; /* [29..28]  */
		unsigned int    rd_qos_sr             : 2   ; /* [31..30]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_FORCE_CLK */
union u_axi_jpeg_force_clk {
	/* Define the struct bits */
	struct {
		unsigned int    force_vprd_clk_on     : 1   ; /* [0]  */
		unsigned int    force_vpwr_clk_on     : 1   ; /* [1]  */
		unsigned int    force_nrrd_clk_on     : 1   ; /* [2]  */
		unsigned int    force_nrwr_clk_on     : 1   ; /* [3]  */
		unsigned int    force_axi_rd_clk_on   : 1   ; /* [4]  */
		unsigned int    force_axi_wr_clk_on   : 1   ; /* [5]  */
		unsigned int    force_du_rd_clk_on    : 1   ; /* [6]  */
		unsigned int    force_du_wr_clk_on    : 1   ; /* [7]  */
		unsigned int    force_cfg_clk_on      : 1   ; /* [8]  */
		unsigned int    reserved_0            : 23  ; /* [31..9]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_OTHER_RO */
union u_axi_jpeg_other_ro {
	/* Define the struct bits  */
	struct {
		unsigned int other_ro               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_OTHER_RW */
union u_axi_jpeg_other_rw {
	/* Define the struct bits  */
	struct {
		unsigned int other_rw               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_CFG */
union u_axi_jpeg_vp_wr_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    vpwr_pixel_format   : 5   ; /* [4..0]  */
		unsigned int    vpwr_pixel_expansion : 1   ; /* [5]  */
		unsigned int    reserved_0            : 7   ; /* [12..6]  */
		unsigned int    vpwr_last_page      : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_WR_AXI_FS */
union u_axi_jpeg_vp_wr_axi_fs {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 2   ; /* [1..0]  */
		unsigned int    vpwr_address_frame_start : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_WR_AXI_LINE_4 */
union u_axi_jpeg_vp_wr_axi_line {
	/* Define the struct bits */
	struct {
		unsigned int    vpwr_line_stride    : 11  ; /* [10..0]  */
		unsigned int    vpwr_line_start_wstrb : 4   ; /* [14..11]  */
		unsigned int    vpwr_line_wrap      : 14  ; /* [28..15]  */
		unsigned int    reserved_0            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_WR_IF_CFG */
union u_axi_jpeg_vp_wr_if_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 16  ; /* [15..0]  */
		unsigned int    vp_wr_stop_enable_du_threshold_reached : 1   ; /* [16]  */
		unsigned int    vp_wr_stop_enable_flux_ctrl : 1   ; /* [17]  */
		unsigned int    vp_wr_stop_enable_pressure : 1   ; /* [18]  */
		unsigned int    reserved_1            : 5   ; /* [23..19]  */
		unsigned int    vp_wr_stop_ok       : 1   ; /* [24]  */
		unsigned int    vp_wr_stop          : 1   ; /* [25]  */
		unsigned int    reserved_2            : 5   ; /* [30..26]  */
		unsigned int    vpwr_prefetch_bypass : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};


/* Define the union U_AXI_JPEG_LIMITER_VP_WR */
union u_axi_jpeg_limiter_vp_wr {
	/* Define the struct bits */
	struct {
		unsigned int    vpwr_access_limiter_0 : 4   ; /* [3..0]  */
		unsigned int    vpwr_access_limiter_1 : 4   ; /* [7..4]  */
		unsigned int    vpwr_access_limiter_2 : 4   ; /* [11..8]  */
		unsigned int    vpwr_access_limiter_3 : 4   ; /* [15..12]  */
		unsigned int    reserved_0            : 8   ; /* [23..16]  */
		unsigned int    vpwr_access_limiter_reload : 4   ; /* [27..24]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_CFG */
union u_axi_jpeg_vp_rd_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_pixel_format   : 5   ; /* [4..0]  */
		unsigned int    vprd_pixel_expansion : 1   ; /* [5]  */
		unsigned int    vprd_allocated_du  : 5   ; /* [10..6]  */
		unsigned int    reserved_0            : 2   ; /* [12..11]  */
		unsigned int    vprd_last_page      : 19  ; /* [31..13]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_LWG */
union u_axi_jpeg_vp_rd_lwg {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_size      : 13  ; /* [12..0]  */
		unsigned int    reserved_0            : 3   ; /* [15..13]  */
		unsigned int    vprd_horizontal_blanking : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_FHG */
union u_axi_jpeg_vp_rd_fhg {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_frame_size     : 13  ; /* [12..0]  */
		unsigned int    reserved_0            : 3   ; /* [15..13]  */
		unsigned int    vprd_vertical_blanking : 8   ; /* [23..16]  */
		unsigned int    reserved_1            : 8   ; /* [31..24]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_AXI_FS */
union u_axi_jpeg_vp_rd_axi_fs {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 2   ; /* [1..0]  */
		unsigned int    vprd_axi_frame_start : 30  ; /* [31..2]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_AXI_LINE */
union u_axi_jpeg_vp_rd_axi_line {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_line_stride    : 11  ; /* [10..0]  */
		unsigned int    reserved_0            : 5   ; /* [15..11]  */
		unsigned int    vprd_line_wrap      : 13  ; /* [28..16]  */
		unsigned int    reserved_1            : 3   ; /* [31..29]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_IF_CFG */
union u_axi_jpeg_vp_rd_if_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 16  ; /* [15..0]  */
		unsigned int    vp_rd_stop_enable_du_threshold_reached : 1   ; /* [16]  */
		unsigned int    vp_rd_stop_enable_flux_ctrl : 1   ; /* [17]  */
		unsigned int    vp_rd_stop_enable_pressure : 1   ; /* [18]  */
		unsigned int    reserved_1            : 5   ; /* [23..19]  */
		unsigned int    vp_rd_stop_ok       : 1   ; /* [24]  */
		unsigned int    vp_rd_stop         : 1   ; /* [25]  */
		unsigned int    reserved_2            : 5   ; /* [30..26]  */
		unsigned int    vprd_prefetch_bypass : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_VP_RD_DEBUG_4 */
union u_axi_jpeg_vp_rd_debug {
	/* Define the struct bits  */
	struct {
		unsigned int vp_rd_debug          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_LIMITER_VP_RD */
union u_axi_jpeg_limiter_vp_rd {
	/* Define the struct bits */
	struct {
		unsigned int    vprd_access_limiter_0 : 4   ; /* [3..0]  */
		unsigned int    vprd_access_limiter_1 : 4   ; /* [7..4]  */
		unsigned int    vprd_access_limiter_2 : 4   ; /* [11..8]  */
		unsigned int    vprd_access_limiter_3 : 4   ; /* [15..12]  */
		unsigned int    reserved_0            : 8   ; /* [23..16]  */
		unsigned int    vprd_access_limiter_reload : 4   ; /* [27..24]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_NR_WR_CFG */
union u_axi_jpeg_nr_wr_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 16  ; /* [15..0]  */
		unsigned int    nr_wr_stop_enable_du_threshold_reached : 1   ; /* [16]  */
		unsigned int    nr_wr_stop_enable_flux_ctrl : 1   ; /* [17]  */
		unsigned int    nr_wr_stop_enable_pressure : 1   ; /* [18]  */
		unsigned int    reserved_1            : 5   ; /* [23..19]  */
		unsigned int    nr_wr_stop_ok       : 1   ; /* [24]  */
		unsigned int    nr_wr_stop         : 1   ; /* [25]  */
		unsigned int    reserved_2            : 5   ; /* [30..26]  */
		unsigned int    nrwr_enable         : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_NR_WR_DEBUG*/
union u_axi_jpeg_nr_wr_debug {
	/* Define the struct bits  */
	struct {
		unsigned int nr_wr_debug          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_LIMITER_NR_WR */
union u_axi_jpeg_limiter_nr_wr {
	/* Define the struct bits */
	struct {
		unsigned int    nrwr_access_limiter_0 : 4   ; /* [3..0]  */
		unsigned int    nrwr_access_limiter_1 : 4   ; /* [7..4]  */
		unsigned int    nrwr_access_limiter_2 : 4   ; /* [11..8]  */
		unsigned int    nrwr_access_limiter_3 : 4   ; /* [15..12]  */
		unsigned int    reserved_0            : 8   ; /* [23..16]  */
		unsigned int    nrwr_access_limiter_reload : 4   ; /* [27..24]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_NR_RD_CFG */
union u_axi_jpeg_nr_rd_cfg {
	/* Define the struct bits */
	struct {
		unsigned int    reserved_0            : 5   ; /* [4..0]  */
		unsigned int    nrrd_allocated_du   : 5   ; /* [9..5]  */
		unsigned int    reserved_1            : 6   ; /* [15..10]  */
		unsigned int    nr_rd_stop_enable_du_threshold_reached : 1   ; /* [16]  */
		unsigned int    nr_rd_stop_enable_flux_ctrl : 1   ; /* [17]  */
		unsigned int    nr_rd_stop_enable_pressure : 1   ; /* [18]  */
		unsigned int    reserved_2            : 5   ; /* [23..19]  */
		unsigned int    nr_rd_stop_ok       : 1   ; /* [24]  */
		unsigned int    nr_rd_stop          : 1   ; /* [25]  */
		unsigned int    reserved_3            : 5   ; /* [30..26]  */
		unsigned int    nrrd_enable         : 1   ; /* [31]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_NR_RD_DEBUG */
union u_axi_jpeg_nr_rd_debug {
	/* Define the struct bits  */
	struct {
		unsigned int nr_rd_debug          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_LIMITER_NR_RD */
union u_axi_jpeg_limiter_nr_rd {
	/* Define the struct bits */
	struct {
		unsigned int    nrrd_access_limiter_0 : 4   ; /* [3..0]  */
		unsigned int    nrrd_access_limiter_1 : 4   ; /* [7..4]  */
		unsigned int    nrrd_access_limiter_2 : 4   ; /* [11..8]  */
		unsigned int    nrrd_access_limiter_3 : 4   ; /* [15..12]  */
		unsigned int    reserved_0            : 8   ; /* [23..16]  */
		unsigned int    nrrd_access_limiter_reload : 4   ; /* [27..24]  */
		unsigned int    reserved_1            : 4   ; /* [31..28]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};

/* Define the union U_AXI_JPEG_SPARE_0 */
union u_axi_jpeg_spare_0 {
	/* Define the struct bits  */
	struct {
		unsigned int spare_0                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_SPARE_1 */
union u_axi_jpeg_spare_1 {
	/* Define the struct bits  */
	struct {
		unsigned int spare_1                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_SPARE_2 */
union u_axi_jpeg_spare_2 {
	/* Define the struct bits  */
	struct {
		unsigned int spare_2                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_SPARE_3 */
union u_axi_jpeg_spare_3 {
	/* Define the struct bits  */
	struct {
		unsigned int spare_3                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_4 */
union u_axi_jpeg_vp_wr_debug_4 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_4          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_5 */
union u_axi_jpeg_vp_wr_debug_5 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_5          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_6 */
union u_axi_jpeg_vp_wr_debug_6 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_6          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_8 */
union u_axi_jpeg_vp_wr_debug_8 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_8          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_9 */
union u_axi_jpeg_vp_wr_debug_9 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_9          : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_10 */
union u_axi_jpeg_vp_wr_debug_10 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_10         : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_25 */
union u_axi_jpeg_vp_wr_debug_25 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_25         : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_VP_WR_DEBUG_30 */
union u_axi_jpeg_vp_wr_debug_30 {
	/* Define the struct bits  */
	struct {
		unsigned int vp_wr_debug_30         : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_0 */
union u_axi_jpeg_debug_0 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_0                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_1 */
union u_axi_jpeg_debug_1 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_1                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_2 */
union u_axi_jpeg_debug_2 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_2                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_3 */
union u_axi_jpeg_debug_3 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_3                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_4 */
union u_axi_jpeg_debug_4 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_4                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_5 */
union u_axi_jpeg_debug_5 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_5                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_6 */
union u_axi_jpeg_debug_6 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_6                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_7 */
union u_axi_jpeg_debug_7 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_7                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_8 */
union u_axi_jpeg_debug_8 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_8                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_9 */
union u_axi_jpeg_debug_9 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_9                : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_10 */
union u_axi_jpeg_debug_10 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_10               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_11 */
union u_axi_jpeg_debug_11 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_11               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_12 */
union u_axi_jpeg_debug_12 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_12               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_13 */
union u_axi_jpeg_debug_13 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_13               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_14 */
union u_axi_jpeg_debug_14 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_14               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
/* Define the union U_AXI_JPEG_DEBUG_15 */
union u_axi_jpeg_debug_15 {
	/* Define the struct bits  */
	struct {
		unsigned int debug_15               : 32  ; /* [31..0]  */
	} bits;

	/* Define an unsigned member */
	unsigned int    u32;

};
//==============================================================================
/* Define the global struct */
struct s_axi_jpeg_regs_type {


};

/* Declare the struct pointor of the module AXI_JPEG */
extern struct s_axi_jpeg_regs_type *gopaxi_jpegallreg;


#endif /* __AXI_JPEG_DRV_PRIV_CS_H__ */
