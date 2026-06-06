
#ifndef _VDM_HAL_LOCAL_HEADER_
#define _VDM_HAL_LOCAL_HEADER_

#include "basedef.h"
#include "mem_manage.h"
#include "vfmw.h"
#include "vdm_hal.h"

#define    VHB_STRIDE_BYTE             (0x400)                       // byte stride
#define    HW_MEM_SIZE                 (640 * 1024 + 64 * 4)
#define    HW_HEVC_MEM_SIZE            (2 * 1024 * 1024 + 64 * 4)                //(4*1024*1024 + 100*1024)

#define    MSG_SLOT_NUM                (MAX_SLICE_SLOT_NUM + 5 + 1)
#define    UP_MSG_SLOT_INDEX           (0)
#define    RP_MSG_SLOT_INDEX           (2)
#define    DN_MSG_HEAD_SLOT_INDEX      (4)
#define    DN_MSG_SLOT_INDEX           (5)
#define    CA_HEVC_MN_ADDR_LEN         (1024)
#define    CA_MN_ADDR_LEN              (64 * 4 * 20)
#define    SED_TOP_ADDR_LEN            (352 * 96)   //(64*4*96)
#define    PMV_TOP_ADDR_LEN            (352 * 128)  //(64*4*96)
#define    RCN_TOP_ADDR_LEN            (352 * 128)  //(64*4*96)
#define    ITRANS_TOP_ADDR_LEN         (352 * 128)  //(128*128)
#define    DBLK_TOP_ADDR_LEN           (352 * 192)  //(128*512)
#define    PPFD_BUF_LEN_DEFAULT        (64 * 4 * 400) //(64*4*800)

#define    INTENSITY_CONV_TAB_ADDR_LEN (256 * 2 * 2 * 3)
#define    VP8_TAB_ADDR_LEN            (32 * 1024)
#define    VP6_TAB_ADDR_LEN            (4 * 1024)
#define    MAX_REF_FRAME_NUM           (16)

/*********************************************************
                       for hevc
**********************************************************/
#ifdef VFMW_HEVC_SUPPORT

/*** UHD decode memory define ***/
#define CFG_HEVC_PROFILE_LEVEL_5
#define CFG_HEVC_MAX_PIX_HEIGHT                     (2304)

#if defined (CFG_HEVC_PROFILE_LEVEL_5)
#define HEVC_PROFILE_LEVEL_5
#else
#define HEVC_PROFILE_LEVEL_6
#endif

#if defined (HEVC_PROFILE_LEVEL_5)
#define HEVC_MAX_SLICE_NUM                           (200)
#else
#define HEVC_MAX_SLICE_NUM                           (600)
#endif

#if !defined (CFG_HEVC_MAX_PIX_WIDTH)
#define HEVC_MAX_PIX_WIDTH                           (4096)
#else
#define HEVC_MAX_PIX_WIDTH                           (CFG_HEVC_MAX_PIX_WIDTH)
#endif

// 3798MV100 logic support tile num is 20x22
#if !defined (CFG_HEVC_MAX_PIX_HEIGHT)
#define HEVC_MAX_PIX_HEIGHT                          (4096)
#else
#define HEVC_MAX_PIX_HEIGHT                          (CFG_HEVC_MAX_PIX_HEIGHT)
#endif


#define HEVC_SED_TOP_ADDR_LEN                        (64*4*HEVC_MAX_PIX_WIDTH)
#define HEVC_PMV_TOP_ADDR_LEN                        (64*4*HEVC_MAX_PIX_WIDTH)
#define HEVC_PMV_LEFT_ADDR_LEN                       (64*4*HEVC_MAX_PIX_HEIGHT)
#define HEVC_RCN_TOP_ADDR_LEN                        (64*4*HEVC_MAX_PIX_WIDTH)
#define HEVC_TILE_SEGMENT_INFO_LEN                	 (2048)
#define HEVC_SAO_LEFT_ADDR_LEN                       (64*4*HEVC_MAX_PIX_HEIGHT)
#define HEVC_DBLK_LEFT_ADDR_LEN                      (64*4*HEVC_MAX_PIX_HEIGHT)
#define HEVC_SAO_TOP_ADDR_LEN                        (64*4*HEVC_MAX_PIX_WIDTH)
#define HEVC_DBLK_TOP_ADDR_LEN                       (64*4*HEVC_MAX_PIX_WIDTH)
/*********************** MSG SLOT PARA DEFINE *********************/
#define HEVC_MAX_SLOT_NUM                            (HEVC_MAX_SLICE_NUM+5)

#ifdef VFMW_VP9_SUPPORT
#define VP9_MAX_PIC_WIDTH           8192
#define VP9_MAX_PIC_HEIGHT          8192

//============================== VP9 =============================
#define  VP9_SED_TOP_ADDR_LEN                     (64*4*VP9_MAX_PIC_WIDTH)
#define  VP9_PMV_TOP_ADDR_LEN                     (64*4*VP9_MAX_PIC_WIDTH)
#define  VP9_RCN_TOP_ADDR_LEN                     (64*4*VP9_MAX_PIC_WIDTH)
#define  VP9_DBLK_TOP_ADDR_LEN                    (64*4*VP9_MAX_PIC_WIDTH)
#define  VP9_DBLK_LEFT_ADDR_LEN                   (VP9_MAX_PIC_HEIGHT * 8)

#ifndef VP9_NEW_PROB
#define  VP9_PROB_TAB_ADDR_LEN                    (20 * 64 * 4)//(18 * 64 * 4)
#define  VP9_PROB_CNT_ADDR_LEN                    (VP9_MAX_PIC_WIDTH * 4)    /* 3301 is enought */
#else
#define  VP9_PROB_TAB_ADDR_LEN                    (20*64*4)
#define  VP9_PROB_CNT_ADDR_LEN                    (20*64*4)
#endif
#endif

#endif


//control registers
#define    VREG_VDH_START                      0x000
#define    VREG_BASIC_CFG0                     0x008
#define    VREG_BASIC_CFG1                     0x00c
#define    VREG_AVM_ADDR                       0x010
#define    VREG_VAM_ADDR                       0x014
#define    VREG_STREAM_BASE_ADDR               0x018
//state registers
#define    VREG_VDH_STATE                      0x01c
#define    VREG_VCTRL_STATE                    0x028
//0x0001FF00: all  RAM OnChip
//0x0002FF00: all  RAM OnChip, except DBLK RAM
#define    VREG_SED_TO                         0x03c
#define    VREG_ITRANS_TO                      0x040
#define    VREG_PMV_TO                         0x044
#define    VREG_PRC_TO                         0x048
#define    VREG_RCN_TO                         0x04c
#define    VREG_DBLK_TO                        0x050
#define    VREG_PPFD_TO                        0x054
#define    VREG_PART_DEC_OVER_INT_LEVEL        0x05c
//1d registers
#define    VREG_YSTADDR_1D                     0x060
#define    VREG_YSTRIDE_1D                     0x064
#define    VREG_UVOFFSET_1D                    0x068
//prc registers
#define    VREG_HEAD_INF_OFFSET                0x06c
//ppfd registers
#define    VREG_PPFD_BUF_ADDR                  0x080
#define    VREG_PPFD_BUF_LEN                   0x084
#define    VREG_REF_PIC_TYPE                   0x094
#define    VREG_FF_APT_EN                      0x098

//mask & clear
#define    VREG_SAFE_INT_STATE                 0x0a8
#define    VREG_SAFE_INT_MASK                  0x0aC
#define    VREG_NORM_INT_STATE                 0x020
#define    VREG_NORM_INT_MASK                  0x024

#ifdef ENV_SOS_KERNEL
#define    VREG_INT_STATE                      VREG_SAFE_INT_STATE
#define    VREG_INT_MASK                       VREG_SAFE_INT_MASK
#else
#define    VREG_INT_STATE                      VREG_NORM_INT_STATE
#define    VREG_INT_MASK                       VREG_NORM_INT_MASK
#endif

#define    VREG_UVSTRIDE_1D                    0x0c4
#define    VREG_CFGINFO_ADDR                   0x0C8
#define    VREG_DDR_INTERLEAVE_MODE            0x0F4
//clock div offset
#define    PERI_CRG_CORE_DIV                   0xCC
#define    PERI_CRG_AXI_DIV                    0xD0

//performance count registers
#define    VREG_DEC_CYCLEPERPIC                0x0B0
#define    VREG_RD_BDWIDTH_PERPIC              0x0B4
#define    VREG_WR_BDWIDTH_PERPIC              0x0B8
#define    VREG_RD_REQ_PERPIC                  0x0BC
#define    VREG_WR_REQ_PERPIC                  0x0C0
#define    VREG_MB0_QP_IN_CURR_PIC             0x0D0
#define    VREG_SWITCH_ROUNDING                0x0D4
//sed registers
#define    VREG_SED_STA                        0x1000
#define    VREG_SED_END0                       0x1014
#define    VREG_LUMA_HISTORGRAM                0x8100
#define    VREG_LUMA_SUM_LOW                   0x8180
#define    VREG_LUMA_SUM_HIGH                  0x8184
//vclk registers
#ifdef PLATFORM_HIVCODECV200
#define    VREG_VDH_ROM_RFT_EMA                0xc074
#endif

typedef struct {
	USIGN mbamt_to_dec           :20;
	USIGN memory_clock_gating_en :1;
	USIGN module_clock_gating_en :1;
	USIGN marker_bit_detect_en   :1;
	USIGN ac_last_detect_en      :1;
	USIGN coef_idx_detect_en     :1;
	USIGN vop_type_detect_en     :1;
	USIGN reserved               :2;
	USIGN luma_sum_en            :1;
	USIGN luma_historgam_en      :1;
	USIGN load_qmatrix_flag      :1;
	USIGN sec_mode_en            :1;
} BASIC_CFG0;

typedef struct {
	USIGN mbamt_to_dec           :20;
	USIGN memory_clock_gating_en :1;
	USIGN module_clock_gating_en :1;
	USIGN marker_bit_detect_en   :1;
	USIGN ac_last_detect_en      :1;
	USIGN coef_idx_detect_en     :1;
	USIGN vop_type_detect_en     :1;
	USIGN work_mode              :2;
	USIGN luma_sum_en            :1;
	USIGN luma_histogram_en      :1;
	USIGN load_qmatrix_flag      :1;
	USIGN vdh_safe_flag          :1;
} HEVC_BASIC_CFG0;

typedef struct {
	USIGN video_standard     :4;
	USIGN reserved:                           9;
	USIGN uv_order_en        :1;
	USIGN fst_slc_grp        :1;
	USIGN mv_output_en       :1;
	USIGN max_slcgrp_num     :12;
	USIGN line_num_output_en :1;
	USIGN vdh_2d_en          :1;
	USIGN compress_en        :1;
	USIGN ppfd_en            :1;
} BASIC_CFG1;

typedef struct {
	USIGN video_standard     :4;
	USIGN reserved           :9;
	USIGN uv_order_en        :1;
	USIGN fst_slc_grp        :1;
	USIGN mv_output_en       :1;
	USIGN max_slcgrp_num     :12;
	USIGN line_num_output_en :1;
	USIGN vdh_2d_en          :1;
	USIGN frm_cmp_en         :1;
	USIGN ppfd_en            :1;
} HEVC_BASIC_CFG1;

typedef struct {
	USIGN video_standard:                       4;
	USIGN reserved:                             9;
	USIGN uv_order_en:                          1;
	USIGN fst_slc_grp:                          1;
	USIGN mv_output_en:                         1;
	USIGN max_slcgrp_num:                       12;
	USIGN line_num_output_en:                   1;
	USIGN vdh_2d_en:                            1;
	USIGN frm_cmp_en:                           1;
	USIGN ppfd_en:                              1;
} VP9_BASIC_CFG1;

typedef struct {
	USIGN av_msg_addr        :32;
} AVM_ADDR;

typedef struct {
	USIGN va_msg_addr        :32;
} VAM_ADDR;

typedef struct {
	USIGN stream_base_addr   :32;
} STREAM_BASE_ADDR;

typedef struct {
	USIGN ystaddr_1d         :32;
} YSTADDR_1D;

typedef struct {
	USIGN ff_apt_en          :1;
	USIGN reserved           :31;
} FF_APT_EN;

typedef struct {
	USIGN ref_pic_type_0     :2;
	USIGN ref_pic_type_1     :2;
	USIGN ref_pic_type_2     :2;
	USIGN ref_pic_type_3     :2;
	USIGN ref_pic_type_4     :2;
	USIGN ref_pic_type_5     :2;
	USIGN ref_pic_type_6     :2;
	USIGN ref_pic_type_7     :2;

	USIGN ref_pic_type_8     :2;
	USIGN ref_pic_type_9     :2;
	USIGN ref_pic_type_10    :2;
	USIGN ref_pic_type_11    :2;
	USIGN ref_pic_type_12    :2;
	USIGN ref_pic_type_13    :2;
	USIGN ref_pic_type_14    :2;
	USIGN ref_pic_type_15    :2;
} REF_PIC_TYPE;

typedef struct {
	USIGN ppfd_buf_addr     :32;
} PPFD_BUF_ADDR;

typedef struct {
	USIGN ppfd_buf_len      :16;
	USIGN reserved          :16;
} PPFD_BUF_LEN;

#endif
