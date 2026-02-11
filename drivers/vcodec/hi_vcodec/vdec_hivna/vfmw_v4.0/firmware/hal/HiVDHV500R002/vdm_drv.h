

#ifndef __VDM_DRV_HEADER__
#define __VDM_DRV_HEADER__
#include "vfmw.h"
#include "sysconfig.h"

#define VDMDRV_OK                   0
#define VDMDRV_ERR                (-1)

#define MSG_SLOT_SIZE              256

#define LUMA_HISTORGAM_NUM         32

#define HEVC_ONE_MSG_SLOT_LEN      320  // 64*5
#define MAX_FRAME_NUM              32
#define VDH_STREAM_BUF_CNT         5

#ifdef VFMW_HEVC_SUPPORT
#define USE_MSG_SLOT_SIZE         HEVC_ONE_MSG_SLOT_LEN
#else
#define USE_MSG_SLOT_SIZE         MSG_SLOT_SIZE
#endif
typedef enum {
	VDH_SHAREFD_MESSAGE_POOL = 0,
	VDH_SHAREFD_STREAM_BUF   = 1,
	VDH_SHAREFD_PMV_BUF      = (VDH_SHAREFD_STREAM_BUF + VDH_STREAM_BUF_CNT),
	VDH_SHAREFD_FRM_BUF,
	VDH_SHAREFD_MAX          = (VDH_SHAREFD_FRM_BUF + MAX_FRAME_NUM)
} vdh_sharefd;
typedef enum {
	VDH_STATE_REG   = 1,
	INT_STATE_REG   = 2,
	INT_MASK_REG    = 3,
	VCTRL_STATE_REG = 4,
} reg_id_e;

typedef enum {
	VDM_IDLE_STATE     = 0,
	VDM_DECODE_STATE   = 1,
	VDM_REPAIR_STATE_0 = 2,
	VDM_REPAIR_STATE_1 = 3
} vdmdrv_statemachine_e;

typedef enum {
	VDMDRV_SLEEP_STAGE_NONE = 0,
	VDMDRV_SLEEP_STAGE_PREPARE,
	VDMDRV_SLEEP_STAGE_SLEEP
} vdmdrv_sleep_stage_e;

typedef enum {
	FIRST_REPAIR = 0,
	SECOND_REPAIR
} repalrtime_s;

typedef enum hi_CONFIG_VDH_CMD {
	CONFIG_VDH_AFTERDEC_CMD = 200,
	CONFIG_VDH_ACTIVATEDEC_CMD
} config_vdh_cmd;

typedef struct {
	UINT32 vdh_reset_flag;
	UINT32 glb_reset_flag;
	SINT32 vdh_start_repair_flag;
	SINT32 vdh_start_hw_dec_flag;
	SINT32 vdh_basic_cfg0;
	SINT32 vdh_basic_cfg1;
	SINT32 vdh_avm_addr;
	SINT32 vdh_vam_addr;
	SINT32 vdh_stream_base_addr;
	SINT32 vdh_emar_id;
	SINT32 vdh_yst_addr;
	SINT32 vdh_ystride;
	SINT32 vdh_uvstride;     // VREG_UVSTRIDE_1D
	SINT32 vdh_vfg_info_addr;  // CFGINFO_ADDR
	SINT32 vdh_uv_offset;
	SINT32 vdh_ref_pic_type;
	SINT32 vdh_ff_apt_en;
	repalrtime_s repair_time;
	vid_std_e vid_std;
	SINT32 valid_group_num_0;
	SINT32 vdh_share_fd[VDH_SHAREFD_MAX];
	UINT32 vdh_stream_buf_num;
	UINT32 vdh_frm_buf_num;
	hi_bool is_frm_buf_remap;
	hi_bool is_pmv_buf_remap;
	hi_bool is_all_buf_remap;
} omxvdh_reg_cfg_s;

typedef struct {
	// vdm register base vir addr
	SINT32 *p_vdm_reg_vir_addr;

	// vdm hal base addr
	UADDR hal_mem_base_addr;
	SINT32 hal_mem_size;
	SINT32 vahb_stride;

	/* message pool */
	UADDR msg_slot_addr[256];
	SINT32 valid_msg_slot_num;

	/* vlc code table */
	UADDR h264_tab_addr;  /* 32 Kbyte */
	UADDR mpeg2_tab_addr; /* 32 Kbyte */
	UADDR mpeg4_tab_addr; /* 32 Kbyte */
	UADDR avs_tab_addr;   /* 32 Kbyte */
	UADDR vc1_tab_addr;
	/* cabac table */
	UADDR h264_mn_addr;
	/* nei info for vdh for hevc */
	UADDR sed_top_phy_addr;
	UADDR pmv_top_phy_addr;
	UADDR pmv_left_phy_addr;
	UADDR rcn_top_phy_addr;
	UADDR mn_phy_addr;
	UADDR tile_segment_info_phy_addr;
	UADDR dblk_left_phy_addr;
	UADDR dblk_top_phy_addr;
	UADDR sao_left_phy_addr;
	UADDR sao_top_phy_addr;
	UADDR ppfd_phy_addr;
	SINT32 ppfd_buf_len;

	/* nei info for vdh */
	UADDR sed_top_addr; /* len = 64*4*x */
	UADDR pmv_top_addr; /* len = 64*4*x */
	UADDR rcn_top_addr; /* len = 64*4*x */
	UADDR itrans_top_addr;
	UADDR dblk_top_addr;
	UADDR ppfd_buf_addr;
	UADDR p_pfd_buf_len;

	UADDR intensity_conv_tab_addr;
	UADDR bitplane_info_addr;
	UADDR vp6_tab_addr;
	UADDR vp8_tab_addr;

	/* VP9 */
	UADDR dblk_left_addr;
	UADDR vp9_prob_tab_addr;
	UADDR vp9_prob_cnt_addr;

	UINT8 *luma_2d_vir_addr;
	UADDR luma_2d_phy_addr;
	UINT8 *chrom_2d_vir_addr;
	UADDR chrom_2d_phy_addr;
} vdmhal_hwmem_s;

typedef struct {
	UINT32 int_state_reg;

	UINT32 basic_cfg1;
	UINT32 vdm_state;
	UINT32 mb0_qp_in_curr_pic;
	UINT32 switch_rounding;
	UINT32 sed_sta;
	UINT32 sed_end_0;

	UINT32 dec_cycle_per_pic;
	UINT32 rd_bdwidth_per_pic;
	UINT32 wr_bd_width_per_pic;
	UINT32 rd_req_per_pic;
	UINT32 wr_req_per_pic;
	UINT32 luma_sum_high;
	UINT32 luma_sum_low;
	UINT32 luma_historgam[LUMA_HISTORGAM_NUM];
} vdmhal_backup_s;

extern vdmhal_hwmem_s g_hw_mem[MAX_VDH_NUM];
#endif
