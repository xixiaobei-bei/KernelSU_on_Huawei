/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdm hal interface
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#include "basedef.h"
#include "vfmw.h"
#include "mem_manage.h"
#include "public.h"
#include "scd_drv.h"
#include "vdm_hal_api.h"
#include "vdm_hal_local.h"
#ifdef VFMW_MPEG2_SUPPORT
#include "vdm_hal_mpeg2.h"
#endif
#ifdef VFMW_H264_SUPPORT
#include "vdm_hal_h264.h"
#endif
#ifdef VFMW_HEVC_SUPPORT
#include "vdm_hal_hevc.h"
#endif
#ifdef VFMW_MPEG4_SUPPORT
#include "vdm_hal_mpeg4.h"
#endif
#ifdef VFMW_VP8_SUPPORT
#include "vdm_hal_vp8.h"
#endif
#ifdef VFMW_VP9_SUPPORT
#include "vdm_hal_vp9.h"
#endif
#include "vfmw_intf.h"
#ifdef HIVDEC_SMMU_SUPPORT
#include "smmu.h"
#endif

#define HW_HEIF_MEM_SIZE  (2 * 1024 * 1024)

// cppcheck-suppress *
#define vdh_check_cfg_addr_eq_return(vdhcfg, else_print, cmp_phy) \
do { \
	if ((vdhcfg == 0) || (vdhcfg != cmp_phy)) { \
		dprint(PRN_FATAL, "%s (%s) is not equal\n",\
		__func__, else_print);\
		return VDMHAL_ERR;\
	} \
} while (0)

// cppcheck-suppress *
#define vdh_check_cfg_addr_return(vdhcfg, else_print, start_phy, end_phy) \
do { \
	if ((vdhcfg == 0) || (vdhcfg < start_phy) || (vdhcfg > end_phy)) { \
		dprint(PRN_FATAL, "%s (%s) is out of range\n", \
		__func__, else_print); \
		return VDMHAL_ERR; \
		} \
} while (0)

vdmhal_hwmem_s  g_hw_mem[MAX_VDH_NUM];
vdmhal_backup_s g_vdm_reg_state;

static vdmdrv_sleep_stage_e s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_NONE;
static vdmdrv_statemachine_e s_vdm_state = VDM_IDLE_STATE;

VOID vdmhal_enableint(SINT32 vdh_id)
{
	UINT32 d32  = 0xFFFFFFFE;
	SINT32 *p32 = NULL;

	if (vdh_id != 0) {
		dprint(PRN_FATAL, "VDH ID is wrong\n");
		return;
	}

	if (g_hw_mem[vdh_id].p_vdm_reg_vir_addr == NULL) {
		p32 = (SINT32 *) mem_phy_2_vir(g_vdh_reg_base_addr);
		if (p32 == NULL) {
			dprint(PRN_FATAL,
				"vdm register virtual address not mapped, reset failed\n");
			return;
		}

		g_hw_mem[vdh_id].p_vdm_reg_vir_addr = p32;
	}

	wr_vreg(VREG_INT_MASK, d32, vdh_id);
}

static SINT32 vdm_hal_check_rp_cfg_address(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
{
#ifdef MSG_POOL_ADDR_CHECK
	SINT32 *p_msg_base = NULL;
	UINT32 d32;
	SINT32 payload_buf_num;
	SINT32 head_buf_num;
	SINT32 ret;
#endif
	vdmhal_assert_ret(p_vdh_mem_map != NULL, "p_vdh_mem_map is NULL");
	vdmhal_assert_ret(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].is_mapped != 0,
		"msg pool is not mapped");
	vdmhal_assert_ret(p_vdh_reg_cfg->vdh_avm_addr != 0, "vdhAvmAddr is NULL");

	if (p_vdh_reg_cfg->vdh_avm_addr !=
		(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_phy_addr
		+ (RP_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE))) {
		dprint(PRN_FATAL, "%s vdh_avm_addr is error", __func__);
		return VDMHAL_ERR;
	}
#ifdef MSG_POOL_ADDR_CHECK
	vdmhal_assert_ret(NULL != (SINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr),
			"repair msg virtual address is NULL");

	p_msg_base = (SINT32 *)(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].p_start_vir_addr
		+ (RP_MSG_SLOT_INDEX * 4 * USE_MSG_SLOT_SIZE));

	for (payload_buf_num = 0; payload_buf_num < 4; payload_buf_num++) {
		d32 = 0;
		rd_msgword((p_msg_base + payload_buf_num), d32);
		ret = check_frm_buf_addr(d32, p_vdh_mem_map);
		if (ret) {
			dprint(PRN_FATAL, "%s dst addr(%d) check failed\n",
				__func__, payload_buf_num);
			return VDMHAL_ERR;
		}
	}

	rd_msgword((p_msg_base + 7), d32);
	if (!(d32 & 0x10000))
		return VDMHAL_OK;

	for (head_buf_num = 8; head_buf_num < 12; head_buf_num++) {
		d32 = 0;
		rd_msgword((p_msg_base + head_buf_num), d32);
		ret = check_frm_buf_addr(d32, p_vdh_mem_map);
		if (ret) {
			dprint(PRN_FATAL, "%s head addr(%d) check failed\n",
				__func__, head_buf_num);
			return VDMHAL_ERR;
		}
	}
#endif
	return VDMHAL_OK;
}

SINT32 vdm_hal_cfg_rp_reg(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
{
	SINT32 d32 = 0;
	if (vdm_hal_check_rp_cfg_address(p_vdh_reg_cfg, p_vdh_mem_map) != VDMHAL_OK)
		return VDMHAL_ERR;
	wr_vreg(VREG_AVM_ADDR, p_vdh_reg_cfg->vdh_avm_addr, 0);

	d32 = 0x2000C203;
	wr_vreg(VREG_BASIC_CFG1, d32, 0);

	d32 = 0x00300C03;
	wr_vreg(VREG_SED_TO, d32, 0);
	wr_vreg(VREG_ITRANS_TO, d32, 0);
	wr_vreg(VREG_PMV_TO, d32, 0);
	wr_vreg(VREG_PRC_TO, d32, 0);
	wr_vreg(VREG_RCN_TO, d32, 0);
	wr_vreg(VREG_DBLK_TO, d32, 0);
	wr_vreg(VREG_PPFD_TO, d32, 0);

	return VDMHAL_OK;
}

#ifdef PLATFORM_HIVCODECV200
static VOID vdm_hal_cfg_mem_ctl_reg(void)
{
	UINT32 d32  = 0x03400260;

	wr_vreg(VREG_VDH_ROM_RFT_EMA, d32, 0);
	smmu_set_mem_ctl_reg();
}
#endif

VOID vdmhal_imp_init(void)
{
	hi_s32 ret;

	ret = memset_s(g_hw_mem, sizeof(g_hw_mem), 0, sizeof(g_hw_mem));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return;
	}
	ret = memset_s(&g_vdm_reg_state, sizeof(g_vdm_reg_state), 0,
		sizeof(g_vdm_reg_state));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memset_s err in function\n", __func__, __LINE__);
		return;
	}

	g_hw_mem[0].p_vdm_reg_vir_addr  = (SINT32 *) mem_phy_2_vir(g_vdh_reg_base_addr);

	vdmhal_imp_glbreset();
	s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_NONE;
	s_vdm_state = VDM_IDLE_STATE;
}

VOID vdmhal_imp_deinit(void)
{
	s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_NONE;
	s_vdm_state = VDM_IDLE_STATE;
}

#ifdef MSG_POOL_ADDR_CHECK
SINT32 vdmhal_imp_open_hal(mem_buffer_s *p_open_param)
{
	UINT32 i = 0;
	UADDR phy_addr;
	UADDR phy_addr_hevc;
	UADDR mem_base_addr;
	SINT32 vdh_size, vdh_id;

	vdmhal_assert_ret(p_open_param != NULL, "p_open_param is error ");
	vdmhal_assert_ret(p_open_param->is_mapped != 0, "Message no map");

	mem_base_addr = p_open_param->start_phy_addr;
	vdh_size = p_open_param->size;
	vdh_id = 0;

	vdmhal_assert_ret(mem_base_addr != 0, "mem_base_addr error");
#ifdef VFMW_HEVC_SUPPORT
	vdmhal_assert_ret(vdh_size >= HW_HEVC_MEM_SIZE,
	"VDMHAL_OpenHAL: vdh_size error");
#else
	vdmhal_assert_ret(vdh_size >= HW_MEM_SIZE, "VDMHAL_OpenHAL: vdh_size error");
#endif

	/* overall info */
	g_hw_mem[vdh_id].hal_mem_base_addr = (mem_base_addr + 1023) & (~1023);
	g_hw_mem[vdh_id].hal_mem_size = vdh_size -
		(g_hw_mem[vdh_id].hal_mem_base_addr - mem_base_addr);
	g_hw_mem[vdh_id].vahb_stride = VHB_STRIDE_BYTE;

	phy_addr = g_hw_mem[vdh_id].hal_mem_base_addr;

#ifdef VFMW_HEVC_SUPPORT
	for (i = 0; i < HEVC_MAX_SLOT_NUM; i++)
		g_hw_mem[vdh_id].msg_slot_addr[i] =
			phy_addr + i * 4 * HEVC_ONE_MSG_SLOT_LEN;

	phy_addr += HEVC_MAX_SLOT_NUM * 4 * HEVC_ONE_MSG_SLOT_LEN;
	g_hw_mem[vdh_id].valid_msg_slot_num = HEVC_MAX_SLOT_NUM;
#else
	for (i = 0; i < MSG_SLOT_NUM; i++)
		g_hw_mem[vdh_id].msg_slot_addr[i] = phy_addr + i * 4 * MSG_SLOT_SIZE;

	phy_addr += MSG_SLOT_NUM * 4 * MSG_SLOT_SIZE;
	g_hw_mem[vdh_id].valid_msg_slot_num = MSG_SLOT_NUM;
#endif

	/* h264_mn_addr mn_phy_addr */
	/* cabac mn tab */
	phy_addr = (phy_addr + 1023) & (~1023);
	g_hw_mem[vdh_id].h264_mn_addr = phy_addr;
	phy_addr += (CA_MN_ADDR_LEN + 0xF) & (~0xF);

#ifdef VFMW_HEVC_SUPPORT
	phy_addr = (phy_addr + 1023) & (~1023);
	g_hw_mem[vdh_id].mn_phy_addr = phy_addr;
	phy_addr += (CA_HEVC_MN_ADDR_LEN + 0xF) & (~0xF);
#endif

	/* code table */
	phy_addr_hevc = phy_addr = (phy_addr + 0x8000) & (~0x8000);
	g_hw_mem[vdh_id].dblk_top_addr = phy_addr;
	phy_addr += DBLK_TOP_ADDR_LEN;
	g_hw_mem[vdh_id].itrans_top_addr = phy_addr;
	phy_addr += ITRANS_TOP_ADDR_LEN;
	/*sed_top + pmv_top + rcn_top*/
#ifndef VFMW_VP9_SUPPORT
	g_hw_mem[vdh_id].sed_top_addr = phy_addr;
	phy_addr += SED_TOP_ADDR_LEN;

	g_hw_mem[vdh_id].pmv_top_addr = phy_addr;
	phy_addr += PMV_TOP_ADDR_LEN;

	g_hw_mem[vdh_id].rcn_top_addr = phy_addr;
	phy_addr += 0;

#else
	g_hw_mem[vdh_id].sed_top_addr = phy_addr;
	phy_addr += VP9_SED_TOP_ADDR_LEN;

	g_hw_mem[vdh_id].pmv_top_addr = phy_addr;
	phy_addr += VP9_PMV_TOP_ADDR_LEN;

	g_hw_mem[vdh_id].rcn_top_addr = phy_addr;
	phy_addr += VP9_RCN_TOP_ADDR_LEN;

	g_hw_mem[vdh_id].dblk_top_addr = phy_addr;
	phy_addr += VP9_DBLK_TOP_ADDR_LEN;

	g_hw_mem[vdh_id].dblk_left_addr = phy_addr;
	phy_addr += VP9_DBLK_LEFT_ADDR_LEN;
#endif
	g_hw_mem[vdh_id].ppfd_buf_addr = phy_addr;
	g_hw_mem[vdh_id].p_pfd_buf_len = (PPFD_BUF_LEN_DEFAULT / 128) - 1;
	phy_addr += (PPFD_BUF_LEN_DEFAULT + 0xF) & (~0xF);

	g_hw_mem[vdh_id].intensity_conv_tab_addr = phy_addr;
	phy_addr += INTENSITY_CONV_TAB_ADDR_LEN;

	g_hw_mem[vdh_id].vp8_tab_addr = phy_addr;
	phy_addr += VP8_TAB_ADDR_LEN;

	g_hw_mem[vdh_id].vp6_tab_addr = phy_addr;
	phy_addr += VP6_TAB_ADDR_LEN;
#ifdef VFMW_VP9_SUPPORT
	g_hw_mem[vdh_id].vp9_prob_tab_addr = phy_addr;
	phy_addr += VP9_PROB_TAB_ADDR_LEN;
	g_hw_mem[vdh_id].vp9_prob_cnt_addr = phy_addr;
#endif
#ifdef VFMW_HEVC_SUPPORT
	g_hw_mem[vdh_id].sed_top_phy_addr = phy_addr_hevc;

	if (HEVC_MAX_PIX_WIDTH > 4096)
		phy_addr_hevc += (HEVC_SED_TOP_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].pmv_top_phy_addr = phy_addr_hevc;

	if (HEVC_MAX_PIX_WIDTH > 4096)
		phy_addr_hevc += (HEVC_PMV_TOP_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].pmv_left_phy_addr = phy_addr_hevc;
	phy_addr_hevc += (HEVC_PMV_LEFT_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].rcn_top_phy_addr = phy_addr_hevc;

	if (HEVC_MAX_PIX_WIDTH > 4096)
		phy_addr_hevc += (HEVC_RCN_TOP_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].tile_segment_info_phy_addr = phy_addr_hevc;
	phy_addr_hevc += (HEVC_TILE_SEGMENT_INFO_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].dblk_left_phy_addr = phy_addr_hevc;
	phy_addr_hevc += (HEVC_DBLK_LEFT_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].dblk_top_phy_addr = phy_addr_hevc;

	if (HEVC_MAX_PIX_WIDTH > 4096)
		phy_addr_hevc += (HEVC_DBLK_TOP_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].sao_left_phy_addr = phy_addr_hevc;
	phy_addr_hevc += (HEVC_SAO_LEFT_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].sao_top_phy_addr = phy_addr_hevc;

	if (HEVC_MAX_PIX_WIDTH > 4096)
		phy_addr_hevc += (HEVC_SAO_TOP_ADDR_LEN + 0xF) & (~0xF);

	g_hw_mem[vdh_id].ppfd_phy_addr = phy_addr_hevc;
	g_hw_mem[vdh_id].ppfd_buf_len = 0;
#endif
	return VDMHAL_OK;
}


#define HEIF_UPMSG_SLOT_INDEX            0
#define HEIF_DOWNMSG_HEAD_SLOT_INDEX     4
#define HEIF_DOWNMSG_SLOT_INDEX          5
#define HEIF_DOWNMSG_SLICE_SLOT_INDEX    6
#define HEIF_DOWNMSG_SLICE_NUM_MAX       5

#define HEIF_VDM_BUF_ALIGN_SIZE          16 // by byte
#define HEIF_MSG_UNIT_SIZE               4  // by word
#define HEIF_UPMSG_STEP_SIZE             4
// down msg maybe add one slice for fault-tolerant, so up msg max num add 1
#define HEIF_UPMSG_SLICE_NUM_MAX        (HEIF_DOWNMSG_SLICE_NUM_MAX + 1)
#define HEIF_UP_MSG_SIZE                (HEIF_UPMSG_SLICE_NUM_MAX * HEIF_MSG_UNIT_SIZE) // by byte

#define HEIF_BURST_SIZE                  64 // by word
#define HEIF_BURST_NUM_MAX               5
#define HEIF_VDM_MAX_MSG_NUM            (HEIF_DOWNMSG_SLICE_NUM_MAX + HEIF_DOWNMSG_SLICE_SLOT_INDEX)
#define HEIF_VDM_ONE_MSG_SIZE           (HEIF_BURST_SIZE * HEIF_BURST_NUM_MAX) // by word

#define HEIF_HEVC_CA_MN_BUF_SIZE         1024
#define HEIF_TILE_SEGMENT_INFO_LEN       2048

//heif needn't it, support it for video stream
#define HEIF_PMV_LEFT_BUF_SIZE          (64 * 4 * 4096)

#define HEIF_VDM_MSG_BUF_SIZE \
	(HEIF_VDM_MAX_MSG_NUM * HEIF_MSG_UNIT_SIZE * HEIF_VDM_ONE_MSG_SIZE + \
	HEIF_HEVC_CA_MN_BUF_SIZE + \
	HEIF_TILE_SEGMENT_INFO_LEN + \
	HEIF_PMV_LEFT_BUF_SIZE + \
	HEIF_VDM_BUF_ALIGN_SIZE)

SINT32 vdmhal_imp_open_heif_hal(mem_buffer_s *p_open_param)
{
	UINT32 i = 0;
	UADDR  phy_addr;
	UADDR  mem_base_addr;
	SINT32 vdh_size, vdh_id;

	vdmhal_assert_ret(p_open_param != NULL, "p_open_param is error ");
	vdmhal_assert_ret(p_open_param->is_mapped != 0, "Message no map");

	mem_base_addr = p_open_param->start_phy_addr;
	vdh_size = p_open_param->size;
	vdh_id = 1;

	vdmhal_assert_ret(mem_base_addr,
		"mem_base_addr error" != 0);
	vdmhal_assert_ret(vdh_size >= HEIF_VDM_MSG_BUF_SIZE,
		"VDMHAL_OpenHAL: Heif vdh_size error");

	/* overall info */
	g_hw_mem[vdh_id].hal_mem_base_addr = mem_base_addr;
	g_hw_mem[vdh_id].hal_mem_size = vdh_size;

	/* message pool */
	phy_addr = g_hw_mem[vdh_id].hal_mem_base_addr;
	phy_addr = (phy_addr + 0xF) & (~0xF);
	for (i = 0; i < HEIF_VDM_MAX_MSG_NUM; i++) {
		g_hw_mem[vdh_id].msg_slot_addr[i] = phy_addr + i *
			HEIF_MSG_UNIT_SIZE * HEIF_VDM_ONE_MSG_SIZE;
	}

	phy_addr += HEIF_VDM_MAX_MSG_NUM * HEIF_MSG_UNIT_SIZE *
		HEIF_VDM_ONE_MSG_SIZE;
	g_hw_mem[vdh_id].valid_msg_slot_num = HEIF_VDM_MAX_MSG_NUM;

	g_hw_mem[vdh_id].mn_phy_addr = phy_addr;

	phy_addr += HEIF_HEVC_CA_MN_BUF_SIZE;

	g_hw_mem[vdh_id].tile_segment_info_phy_addr = phy_addr;
	phy_addr += HEIF_TILE_SEGMENT_INFO_LEN;

	g_hw_mem[vdh_id].pmv_left_phy_addr = phy_addr;
	phy_addr += HEIF_PMV_LEFT_BUF_SIZE;

	return VDMHAL_OK;
}

#endif

VOID vdmhal_imp_reset_vdm(SINT32 vdh_id)
{
	SINT32 i;
	SINT32 tmp = 0;
	UINT32 reg;
	UINT32 reg_rst_ok;
	UINT32 *p_vdm_reset_vir_addr = NULL;
	UINT32 *p_vdm_reset_ok_vir_addr = NULL;

	if (vdh_id > (MAX_VDH_NUM - 1)) {
	    dprint(PRN_FATAL, "%s: vdh_id : %d is more than the MAX\n", __func__, vdh_id);
	    return;
	}

	p_vdm_reset_vir_addr   = (UINT32 *) mem_phy_2_vir(g_soft_rst_req_addr);
	p_vdm_reset_ok_vir_addr = (UINT32 *) mem_phy_2_vir(g_soft_rst_ok_addr);
	if ((p_vdm_reset_vir_addr == NULL)
		|| (p_vdm_reset_ok_vir_addr == NULL)
		|| (g_hw_mem[vdh_id].p_vdm_reg_vir_addr == NULL)) {
		dprint(PRN_FATAL,
			"map vdm register fail, vir(p_vdm_reset_vir_addr) : (%pK),vir(p_vdm_reset_ok_vir_addr) : (%pK)\n",
			p_vdm_reset_vir_addr, p_vdm_reset_ok_vir_addr);
		return;
	}

	rd_vreg(VREG_INT_MASK, tmp, vdh_id);

	/* require mfde reset */
	reg = *(volatile UINT32 *)p_vdm_reset_vir_addr;
	*(volatile UINT32 *)p_vdm_reset_vir_addr = reg |
		(UINT32)(1 << MFDE_RESET_CTRL_BIT);

	/* wait for reset ok */
	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)p_vdm_reset_ok_vir_addr;
		if (reg_rst_ok & (1 << MFDE_RESET_OK_BIT))
			break;
		VFMW_OSAL_U_DELAY(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "%s reset failed\n", __func__);

	/* clear reset require */
	*(volatile UINT32 *)p_vdm_reset_vir_addr = reg &
	(UINT32)(~(1 << MFDE_RESET_CTRL_BIT));

	wr_vreg(VREG_INT_MASK, tmp, vdh_id);
	s_vdm_state = VDM_IDLE_STATE;
}

VOID vdmhal_imp_glbreset(void)
{
	SINT32 i;
	UINT32 reg, reg_rst_ok;
	UINT32 *p_reset_vir_addr   = NULL;
	UINT32 *p_reset_ok_vir_addr = NULL;

	p_reset_vir_addr   = (UINT32 *) mem_phy_2_vir(g_soft_rst_req_addr);
	p_reset_ok_vir_addr = (UINT32 *) mem_phy_2_vir(g_soft_rst_ok_addr);
	if (p_reset_vir_addr == NULL || p_reset_ok_vir_addr == NULL) {
		dprint(PRN_FATAL,
			"VDMHAL_GlbReset: map vdm register fail,vir(p_reset_vir_addr) : (%pK), vir(p_reset_ok_vir_addr) : (%pK)\n",
			p_reset_vir_addr, p_reset_ok_vir_addr);
		return;
	}

	/* require all reset, include mfde scd bpd */
	reg = *(volatile UINT32 *)p_reset_vir_addr;
	*(volatile UINT32 *)p_reset_vir_addr = reg |
		(UINT32)(1 << ALL_RESET_CTRL_BIT);

	/* wait for reset ok */
	for (i = 0; i < 100; i++) {
		reg_rst_ok = *(volatile UINT32 *)p_reset_ok_vir_addr;
		if (reg_rst_ok & (1 << ALL_RESET_OK_BIT))
			break;
		VFMW_OSAL_U_DELAY(10);
	}

	if (i >= 100)
		dprint(PRN_FATAL, "Glb Reset Failed\n");

	/* clear reset require */
	*(volatile UINT32 *)p_reset_vir_addr = reg &
	(UINT32)(~(1 << ALL_RESET_CTRL_BIT));

#ifdef PLATFORM_HIVCODECV200
	vdm_hal_cfg_mem_ctl_reg();
#endif
}

VOID vdmhal_imp_clearintstate(SINT32 vdh_id)
{
	SINT32 *p32 = NULL;
	UINT32 d32 = 0xFFFFFFFF;

	if (vdh_id > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: vdh_id : %d is more than %d\n",
			__func__, vdh_id, (MAX_VDH_NUM - 1));
		return;
	}

	if (g_hw_mem[vdh_id].p_vdm_reg_vir_addr == NULL) {
		p32 = (SINT32 *) mem_phy_2_vir(g_vdh_reg_base_addr);
		if (p32 != NULL) {
			g_hw_mem[vdh_id].p_vdm_reg_vir_addr = p32;
		} else {
			dprint(PRN_FATAL,
				" %s %d vdm register virtual address not mapped, reset failed\n",
				__func__, __LINE__);
			return;
		}
	}

	wr_vreg(VREG_INT_STATE, d32, vdh_id);
}

SINT32 vdmhal_imp_checkreg(reg_id_e reg_id, SINT32 vdh_id)
{
	SINT32 *p32 = NULL;
	SINT32 dat = 0;
	UINT32 reg_type;

	if (vdh_id > (MAX_VDH_NUM - 1)) {
		dprint(PRN_FATAL, "%s: Invalid vdh_id is %d\n", __func__, vdh_id);
		return VDMHAL_ERR;
	}

	if (g_hw_mem[vdh_id].p_vdm_reg_vir_addr == NULL) {
		p32 = (SINT32 *) mem_phy_2_vir(g_vdh_reg_base_addr);
		if (p32 != NULL) {
			g_hw_mem[vdh_id].p_vdm_reg_vir_addr = p32;
		} else {
			dprint(PRN_FATAL, " %s %d vdm register virtual address not mapped, reset failed\n", __func__, __LINE__);
			return 0;
		}
	}

	switch (reg_id) {
	case VDH_STATE_REG:
		reg_type = VREG_VDH_STATE;
		break;

	case INT_STATE_REG:
		reg_type = VREG_INT_STATE;
		break;

	case INT_MASK_REG:
		reg_type = VREG_INT_MASK;
		break;

	case VCTRL_STATE_REG:
		reg_type = VREG_VCTRL_STATE;
		break;

	default:
		dprint(PRN_FATAL, "%s: unkown reg_id is %d\n", __func__, reg_id);
		return 0;
	}

	rd_vreg(reg_type, dat, 0);
	return dat;
}

SINT32 vdmhal_imp_check_cfg_address(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
{
	hi_u32 index;
	hi_s32 ret;
	hi_u32 vdh_cfg;
	hi_u32 u32_map_msg_addr;
	hi_u32 u32_start_stream_phy_addr;
	hi_u32 u32_end_stream_phy_addr;
	hi_u32 vdh_stream_buf_num;
#ifdef VFMW_HEVC_SUPPORT
	hi_s32 msg_slot_size = HEVC_ONE_MSG_SLOT_LEN;
#else
	hi_s32 msg_slot_size = MSG_SLOT_SIZE;
#endif
	hi_s32 is_mapped = 0;

	vdmhal_assert_ret(p_vdh_mem_map != NULL, "p_vdh_mem_map is NULL");
	vdmhal_assert_ret(p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].is_mapped
		!= 0, "MESSAGE_POOL is not map");
	vdmhal_assert_ret(p_vdh_reg_cfg->vdh_stream_buf_num
		<= VDH_STREAM_BUF_CNT, "stream buffer num is out of range");

	vdh_stream_buf_num = VDH_SHAREFD_STREAM_BUF + p_vdh_reg_cfg->vdh_stream_buf_num;
	for (index = VDH_SHAREFD_STREAM_BUF; index < vdh_stream_buf_num; index++) {
		vdmhal_assert_ret(p_vdh_mem_map[index].is_mapped
			!= 0, "STREAM_BUF is not map");
		vdmhal_assert_ret(p_vdh_mem_map[index].share_fd
			!= INVALID_SHAREFD, "p_vdh_mem_map fd is 0");
	}

	vdmhal_assert_ret(p_vdh_reg_cfg->vdh_yst_addr != 0,
		"p_vdh_reg_cfg->vdh_yst_addr is NULL");
	vdmhal_assert_ret((p_vdh_reg_cfg->vdh_ystride >= 0)
		&& (p_vdh_reg_cfg->vdh_ystride <= MAX_STRIDE),
		"p_vdh_reg_cfg->vdh_ystride is error");
	vdmhal_assert_ret((p_vdh_reg_cfg->vdh_uvstride >= 0)
		&& (p_vdh_reg_cfg->vdh_uvstride <= p_vdh_reg_cfg->vdh_ystride),
		"p_vdh_reg_cfg->vdh_uvstride is error");
	vdmhal_assert_ret((((basic_cfg0 *)(&p_vdh_reg_cfg->vdh_basic_cfg0))
		->mbamt_to_dec) <= MAX_MB_NUM_IN_PIC,
		"p_vdh_reg_cfg->mbamt_to_dec is error");

	/* down message check */
	u32_map_msg_addr = (p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_phy_addr
		+ (DN_MSG_SLOT_INDEX * 4 * msg_slot_size));
	vdh_check_cfg_addr_eq_return(p_vdh_reg_cfg->vdh_avm_addr,
		"down message", u32_map_msg_addr);

	/* up message check */
	u32_map_msg_addr = (p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_phy_addr
		+ (UP_MSG_SLOT_INDEX * 4 * msg_slot_size));
	vdh_check_cfg_addr_eq_return(p_vdh_reg_cfg->vdh_vam_addr,
		"up message", u32_map_msg_addr);

	/* hfbc message check */
	u32_map_msg_addr = (p_vdh_mem_map[VDH_SHAREFD_MESSAGE_POOL].start_phy_addr
		+ (DN_MSG_HEAD_SLOT_INDEX  * 4 * msg_slot_size));
	vdh_check_cfg_addr_eq_return(p_vdh_reg_cfg->vdh_vfg_info_addr,
		"hfbc message", u32_map_msg_addr);

	/* stream  check */
	for (index = VDH_SHAREFD_STREAM_BUF; index < vdh_stream_buf_num; index++) {
		u32_start_stream_phy_addr = p_vdh_mem_map[index].start_phy_addr;
		u32_end_stream_phy_addr = (p_vdh_mem_map[index].start_phy_addr
			+ p_vdh_mem_map[index].size);
		vdh_cfg = ((stream_basic_addr *)
			(&(p_vdh_reg_cfg->vdh_stream_base_addr)))->stream_base_addr;

		if ((vdh_cfg != 0) && (vdh_cfg >= u32_start_stream_phy_addr)
			&& (vdh_cfg <= u32_end_stream_phy_addr)) {
			ret = VDMHAL_OK;
			break;
		}

		ret = VDMHAL_ERR;
	}

	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL,
			"%s vdh_stream_base_addr is out of range\n", __func__);
		return ret;
	}

	/* output  check */
	for (index = VDH_SHAREFD_FRM_BUF; index < VDH_SHAREFD_MAX; index++) {
		if (p_vdh_mem_map[index].is_mapped == 0)
			break;
		if (((USIGN)(p_vdh_reg_cfg->vdh_yst_addr)
			>= p_vdh_mem_map[index].start_phy_addr)
			&& ((p_vdh_reg_cfg->vdh_uv_offset > 0)
				&& ((hi_u32)p_vdh_reg_cfg->vdh_uv_offset
				< p_vdh_mem_map[index].size))
			&& ((USIGN)(p_vdh_reg_cfg->vdh_yst_addr)
				< (p_vdh_mem_map[index].start_phy_addr
				   + p_vdh_mem_map[index].size
				   - p_vdh_reg_cfg->vdh_uv_offset))) {
			is_mapped = 1;
			break;
		}
	}
	if (is_mapped != 1) {
		dprint(PRN_FATAL, "%s vdh_yst_addr is out of range\n", __func__);
		return VDMHAL_ERR;
	}

	return VDMHAL_OK;
}
SINT32 vdmhal_imp_prepare_dec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
{
	vdmhal_hwmem_s *p_hw_mem = &(g_hw_mem[0]);
	SINT32 *p32 = NULL;
	SINT32 ret;

	if (p_hw_mem->p_vdm_reg_vir_addr == NULL) {
		p32 = (SINT32 *)mem_phy_2_vir(g_vdh_reg_base_addr);
		if (p32 != NULL) {
			p_hw_mem->p_vdm_reg_vir_addr = p32;
		} else {
			dprint(PRN_FATAL,
				"vdm register virtual address not mapped,VDMHAL_PrepareDecfailed\n");
			return VDMHAL_ERR;
		}
	}

	ret = vdmhal_imp_check_cfg_address(p_vdh_reg_cfg, p_vdh_mem_map);
	if (ret != VDMHAL_OK) {
		dprint(PRN_FATAL, "vdmhal_imp_check_cfg_address is fail\n");
		return VDMHAL_ERR;
	}

	if (p_vdh_reg_cfg->vid_std == VFMW_AVS)
		wr_scd_reg(REG_AVS_FLAG, 0x00000001);
	else
		wr_scd_reg(REG_AVS_FLAG, 0x00000000);

	wr_scd_reg(REG_VDH_SELRST, 0x00000001);

	switch (p_vdh_reg_cfg->vid_std) {
#ifdef VFMW_H264_SUPPORT
	case VFMW_H264:
#ifdef MSG_POOL_ADDR_CHECK
		return h264hal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return h264hal_startdec(p_vdh_reg_cfg);
#endif
#endif
#ifdef VFMW_HEVC_SUPPORT
	case VFMW_HEVC:
#ifdef MSG_POOL_ADDR_CHECK
		return hevchal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return hevchal_startdec(p_vdh_reg_cfg);
#endif
#endif
#ifdef VFMW_MPEG2_SUPPORT
	case VFMW_MPEG2:
#ifdef MSG_POOL_ADDR_CHECK
		return mp2hal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return mp2hal_startdec(p_vdh_reg_cfg);
#endif
#endif
#ifdef VFMW_MPEG4_SUPPORT
	case VFMW_MPEG4:
#ifdef MSG_POOL_ADDR_CHECK
		return mp4hal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return mp4hal_startdec(p_vdh_reg_cfg);
#endif
#endif
#ifdef VFMW_VP8_SUPPORT
	case VFMW_VP8:
#ifdef MSG_POOL_ADDR_CHECK
		return vp8hal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return vp8hal_startdec(p_vdh_reg_cfg);
#endif
#endif
#ifdef VFMW_VP9_SUPPORT
	case VFMW_VP9:
#ifdef MSG_POOL_ADDR_CHECK
		return vp9hal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return vp9hal_startdec(p_vdh_reg_cfg);
#endif
#endif
#ifdef VFMW_MVC_SUPPORT
	case VFMW_MVC:
#ifdef MSG_POOL_ADDR_CHECK
		return h264hal_startdec(p_vdh_reg_cfg, p_vdh_mem_map);
#else
		return h264hal_startdec(p_vdh_reg_cfg);
#endif
#endif
	default:
		break;
	}

	return VDMHAL_ERR;
}

SINT32 vdmhal_is_vdm_run(SINT32 vdh_id)
{
	SINT32 data_32 = 0;

	if (vdh_id > (MAX_VDH_NUM - 1)) {
	    dprint(PRN_FATAL, "%s: Invalid vdh_id is %d\n", __func__, vdh_id);
	    return VDMHAL_ERR;
	}
	if (g_hw_mem[vdh_id].p_vdm_reg_vir_addr == NULL) {
		dprint(PRN_FATAL, "VDM register not mapped yet\n");
		return 0;
	}

	rd_vreg(VREG_VCTRL_STATE, data_32, vdh_id);

	return (data_32 == 1 ? 0 : 1);
}

SINT32 vdmhal_imp_backupinfo(void)
{
	SINT32 i = 0;
	SINT32 reg_tmp;
	g_vdm_reg_state.int_state_reg = vdmhal_imp_checkreg(INT_STATE_REG, 0);

	rd_vreg(VREG_BASIC_CFG1, g_vdm_reg_state.basic_cfg1, 0);
	rd_vreg(VREG_VDH_STATE, g_vdm_reg_state.vdm_state, 0);

	rd_vreg(VREG_MB0_QP_IN_CURR_PIC, g_vdm_reg_state.mb0_qp_in_curr_pic, 0);
	rd_vreg(VREG_SWITCH_ROUNDING, g_vdm_reg_state.switch_rounding, 0);

	{
		rd_vreg(VREG_SED_STA, g_vdm_reg_state.sed_sta, 0);
		rd_vreg(VREG_SED_END0, g_vdm_reg_state.sed_end_0, 0);
		rd_vreg(VREG_DEC_CYCLEPERPIC, g_vdm_reg_state.dec_cycle_per_pic, 0);
		rd_vreg(VREG_RD_BDWIDTH_PERPIC,
			g_vdm_reg_state.rd_bdwidth_per_pic, 0);
		rd_vreg(VREG_WR_BDWIDTH_PERPIC,
			g_vdm_reg_state.wr_bd_width_per_pic, 0);
		rd_vreg(VREG_RD_REQ_PERPIC, g_vdm_reg_state.rd_req_per_pic, 0);
		rd_vreg(VREG_WR_REQ_PERPIC, g_vdm_reg_state.wr_req_per_pic, 0);
		rd_vreg(VREG_LUMA_SUM_LOW, g_vdm_reg_state.luma_sum_low, 0);
		rd_vreg(VREG_LUMA_SUM_HIGH, g_vdm_reg_state.luma_sum_high, 0);
	}
	for (i = 0; i < 32; i++) {
		reg_tmp = VREG_LUMA_HISTORGRAM + i * 4;
		rd_vreg(reg_tmp, g_vdm_reg_state.luma_historgam[i], 0);
	}

	return VDMHAL_OK;
}

VOID vdmhal_getregstate(vdmhal_backup_s *p_vdm_reg_state)
{
	hi_s32 ret;

	ret = memcpy_s(p_vdm_reg_state, sizeof(*p_vdm_reg_state), &g_vdm_reg_state,
		sizeof(*p_vdm_reg_state));
	if (ret != EOK) {
		dprint(PRN_FATAL, " %s %d memcpy_s err in function\n", __func__, __LINE__);
		return;
	}
	s_vdm_state = VDM_IDLE_STATE;
}

SINT32 vdmhal_imp_prepare_repair(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map)
{
	vdmhal_hwmem_s *p_hw_mem = &(g_hw_mem[0]);
	SINT32 *p32 = NULL;
	SINT32 ret;

	if (p_vdh_reg_cfg == NULL) {
		dprint(PRN_FATAL, "%s: parameter is NULL\n", __func__);
		return VDMHAL_ERR;
	}
	if (p_hw_mem->p_vdm_reg_vir_addr == NULL) {
		p32 = (SINT32 *)mem_phy_2_vir(g_vdh_reg_base_addr);
		if (p32 != NULL) {
			p_hw_mem->p_vdm_reg_vir_addr = p32;
		} else {
			dprint(PRN_FATAL,
				"vdm register virtual address not mapped, VDMHAL_PrepareRepair failed\n");
			return VDMHAL_ERR;
		}
	}
	if (p_vdh_reg_cfg->repair_time == FIRST_REPAIR) {
		if (p_vdh_reg_cfg->valid_group_num_0 > 0)
			ret = vdm_hal_cfg_rp_reg(p_vdh_reg_cfg, p_vdh_mem_map);
		else
			ret = VDMHAL_ERR;
	} else if (p_vdh_reg_cfg->repair_time == SECOND_REPAIR) {
		dprint(PRN_FATAL, "SECOND_REPAIR Parameter Error\n");
		ret = VDMHAL_ERR;
	} else {
		dprint(PRN_FATAL, " parameter error\n");
		ret = VDMHAL_ERR;
	}

	return ret;
}

VOID vdmhal_imp_start_hw_repair(SINT32 vdh_id)
{
	SINT32 d32 = 0;

	rd_vreg(VREG_BASIC_CFG0, d32, vdh_id);
#ifdef ENV_SOS_KERNEL
	d32 = 0x84000000;
#else
	d32 = 0x4000000;
#endif
	wr_vreg(VREG_BASIC_CFG0, d32, vdh_id);

#ifdef HIVDEC_SMMU_SUPPORT
#ifdef ENV_SOS_KERNEL
	smmu_set_master_reg(MFDE, SECURE_ON, SMMU_OFF);
#else
	smmu_set_master_reg(MFDE, SECURE_OFF, SMMU_ON);
#endif
#endif

	vdmhal_imp_clearintstate(vdh_id);
	vdmhal_enableint(vdh_id);

	VFMW_OSAL_MB();
	wr_vreg(VREG_VDH_START, 0, vdh_id);
	wr_vreg(VREG_VDH_START, 1, vdh_id);
	wr_vreg(VREG_VDH_START, 0, vdh_id);
}

VOID vdmhal_imp_start_hw_decode(SINT32 vdh_id)
{
#ifdef HIVDEC_SMMU_SUPPORT
#ifdef ENV_SOS_KERNEL
	smmu_set_master_reg(MFDE, SECURE_ON, SMMU_OFF);
#else
	smmu_set_master_reg(MFDE, SECURE_OFF, SMMU_ON);
#endif
#endif

	vdmhal_imp_clearintstate(vdh_id);
	vdmhal_enableint(vdh_id);

	VFMW_OSAL_MB();
	wr_vreg(VREG_VDH_START, 0, 0);
	wr_vreg(VREG_VDH_START, 1, 0);
	wr_vreg(VREG_VDH_START, 0, 0);
}

VOID vdmhal_isr(SINT32 vdh_id)
{
	vdmhal_imp_backupinfo();
	vdmhal_imp_clearintstate(vdh_id);
	VFMW_OSAL_GIVE_EVENT(G_VDMHWDONEEVENT);
}

SINT32 vdmhal_hw_dec_proc(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s* p_vdh_mem_map)
{
	SINT32 ret = VDMDRV_ERR;

	s_vdm_state = VDM_DECODE_STATE;

	if (p_vdh_reg_cfg->vdh_start_hw_dec_flag == 1) {
		ret = vdmhal_imp_prepare_dec(p_vdh_reg_cfg, p_vdh_mem_map);
		if (ret == VDMDRV_OK) {
			vdmhal_imp_start_hw_decode(0);
		} else {
			ret = VDMDRV_ERR;
			dprint(PRN_FATAL, "%s prepare dec fail\n", __func__);
		}
	} else if (p_vdh_reg_cfg->vdh_start_repair_flag == 1) {
		ret = vdmhal_imp_prepare_repair(p_vdh_reg_cfg, p_vdh_mem_map);
		if (ret == VDMDRV_OK) {
			vdmhal_imp_start_hw_repair(0);
		} else {
			ret = VDMDRV_ERR;
			dprint(PRN_FATAL,
				"%s prepare repair fail\n", __func__);
		}
	} else {
		ret = VDMDRV_ERR;
		dprint(PRN_FATAL, "%s process type error\n", __func__);
	}

	if (ret != VDMDRV_OK)
		s_vdm_state = VDM_IDLE_STATE;
	return ret;
}

SINT32 vdmhal_prepare_sleep(void)
{
	SINT32 ret = VDMDRV_OK;

	VFMW_OSAL_SEMA_DOWN(G_VDH_SEM);
	if (s_evdm_drv_sleep_state == VDMDRV_SLEEP_STAGE_NONE) {
		if (s_vdm_state == VDM_IDLE_STATE) {
			dprint(PRN_ALWS, "%s, idle state\n", __func__);
			s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_SLEEP;
		} else {
			dprint(PRN_ALWS, "%s, work state\n", __func__);
			s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_PREPARE;
		}

		ret = VDMDRV_OK;
	} else {
		ret = VDMDRV_ERR;
	}

	VFMW_OSAL_SEMA_UP(G_VDH_SEM);
	return ret;
}

VOID vdmhal_force_sleep(void)
{
	dprint(PRN_ALWS, "%s, force state\n", __func__);
	VFMW_OSAL_SEMA_DOWN(G_VDH_SEM);
	if (s_evdm_drv_sleep_state != VDMDRV_SLEEP_STAGE_SLEEP) {
		vdmhal_imp_reset_vdm(0);
		s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_SLEEP;
	}

	VFMW_OSAL_SEMA_UP(G_VDH_SEM);
}

VOID vdmhal_exit_sleep(void)
{
	VFMW_OSAL_SEMA_DOWN(G_VDH_SEM);
	s_evdm_drv_sleep_state = VDMDRV_SLEEP_STAGE_NONE;
	VFMW_OSAL_SEMA_UP(G_VDH_SEM);
}

vdmdrv_sleep_stage_e vdmhal_get_sleep_stage(void)
{
	return s_evdm_drv_sleep_state;
}

VOID vdmhal_set_sleep_stage(vdmdrv_sleep_stage_e sleep_state)
{
	VFMW_OSAL_SEMA_DOWN(G_VDH_SEM);
	s_evdm_drv_sleep_state = sleep_state;
	VFMW_OSAL_SEMA_UP(G_VDH_SEM);
}
