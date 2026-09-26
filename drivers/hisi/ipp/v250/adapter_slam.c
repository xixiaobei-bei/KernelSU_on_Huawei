#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "slam.h"
#include "cfg_table_slam.h"
#include "cvdr_drv.h"
#include "slam_drv.h"
#include "config_table_cvdr.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "slam_drv_priv.h"
#include "slam_reg_offset.h"

#include "sub_ctrl_reg_offset.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define LOG_TAG LOG_MODULE_SLAM
#define CVDR_KEY_POINT_OVERFLOW  0x8000
#define CMDLST_READ_FLAG  1

extern slam_dev_t g_slam_devs[];

static int slam_request_dump(msg_req_slam_request_t *req);

static int slam_update_cvdr_cfg_tab_bi_slam_y(cvdr_opt_fmt_t *cfmt, msg_req_slam_request_t *slam_request,
		cfg_tab_cvdr_t *slam_cvdr_cfg_tab, unsigned int pyramid_index)
{
	pix_format_e format = PIXEL_FMT_CPE_MAX;
	unsigned int slam_stride = 0;
	cfmt->id = RD0_SLAM;
	cfmt->width = slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].width;
	cfmt->full_width = slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].width / 2;
	cfmt->line_size = slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].width / 2;
	cfmt->height = slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].height;
	cfmt->expand = 0;
	format = PIXEL_FMT_CPE_2PF8;
	cfmt->addr = slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].buf;

	if (0 == pyramid_index) {
		slam_stride = slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].stride;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(slam_cvdr_cfg_tab, cfmt, slam_stride, CPE_CLK, format));
	} else {
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(slam_cvdr_cfg_tab, cfmt, 0, CPE_CLK, format));
	}

	return 0;
}

static int slam_update_cvdr_cfg_tab(msg_req_slam_request_t *slam_request, cfg_tab_cvdr_t *slam_cvdr_cfg_tab,
									unsigned int pyramid_index)
{
	cvdr_opt_fmt_t cfmt;
	pix_format_e format = PIXEL_FMT_CPE_MAX;

	if (slam_request->req_orb.streams[pyramid_index][BI_SLAM_Y].buf)
		slam_update_cvdr_cfg_tab_bi_slam_y(&cfmt, slam_request, slam_cvdr_cfg_tab, pyramid_index);

	if (slam_request->req_orb.streams[pyramid_index][BO_SLAM_Y].buf) {
		cfmt.id = WR0_SLAM;
		cfmt.width = slam_request->req_orb.streams[pyramid_index][BO_SLAM_Y].width;
		cfmt.full_width = slam_request->req_orb.streams[pyramid_index][BO_SLAM_Y].width / 2;
		cfmt.line_size = slam_request->req_orb.streams[pyramid_index][BO_SLAM_Y].width / 2;
		cfmt.height = slam_request->req_orb.streams[pyramid_index][BO_SLAM_Y].height;
		cfmt.addr = slam_request->req_orb.streams[pyramid_index][BO_SLAM_Y].buf;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_2PF8;
		loge_if_ret(dataflow_cvdr_wr_cfg_vp(slam_cvdr_cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	if (slam_request->req_orb.streams[pyramid_index][BO_SLAM_STAT].buf) {
		cfmt.id = WR1_SLAM;
		cfmt.addr = slam_request->req_orb.streams[pyramid_index][BO_SLAM_STAT].buf;
		loge_if_ret(cfg_tbl_cvdr_wr_cfg_d64(slam_cvdr_cfg_tab, &cfmt, SLAM_OUT_DESC_SIZE));
	}

	if (slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].buf
		&& (1 == slam_request->req_orb.reg_cfg[pyramid_index].ctrl.scoremap_en)) {
		cfmt.id = WR2_SLAM;
		cfmt.width = slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].width;
		cfmt.full_width = slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].width / 2;
		cfmt.line_size = slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].width / 2;
		cfmt.height = slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].height;
		cfmt.addr = slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].buf;
		cfmt.expand = 0;
		format = PIXEL_FMT_CPE_2PF8;
		loge_if_ret(dataflow_cvdr_wr_cfg_vp(slam_cvdr_cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	if (slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].buf
		&& (1 == slam_request->req_orb.reg_cfg[pyramid_index].ctrl.gridstat_en)) {
		cfmt.id = WR2_SLAM;
		cfmt.addr = slam_request->req_orb.streams[pyramid_index][BO_SLAM_SCORE_MAP].buf;
		cfmt.expand = 0;
		loge_if_ret(dataflow_cvdr_wr_cfg_d32(slam_cvdr_cfg_tab, &cfmt, SLAM_OUT_GRID_STAT_SIZE));
	}

	return 0;
}

static void slam_update_request_undistort_cfg(msg_req_slam_request_t *slam_request, cfg_tab_slam_t *slam_cfg_tab,
		unsigned int layer_index)
{
	slam_cfg_tab->undistort_cfg.cx    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.cx;
	slam_cfg_tab->undistort_cfg.cy    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.cy;
	slam_cfg_tab->undistort_cfg.fx    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.fx;
	slam_cfg_tab->undistort_cfg.fy    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.fy;
	slam_cfg_tab->undistort_cfg.invfx = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.invfx;
	slam_cfg_tab->undistort_cfg.invfy = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.invfy;
	slam_cfg_tab->undistort_cfg.k1    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.k1;
	slam_cfg_tab->undistort_cfg.k2    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.k2;
	slam_cfg_tab->undistort_cfg.k3    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.k3;
	slam_cfg_tab->undistort_cfg.p1    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.p1;
	slam_cfg_tab->undistort_cfg.p2    = slam_request->req_orb.reg_cfg[layer_index].undistort_cfg.p2;
	slam_cfg_tab->cvdr_cfg.vprd_horizontal_blanking = 0x0;
	slam_cfg_tab->cvdr_cfg.vprd_line_wrap           = 0x0;
}

static void slam_update_request_cfg_tab(msg_req_slam_request_t *slam_request, cfg_tab_slam_t *slam_cfg_tab,
										unsigned int layer_index)
{
	slam_cfg_tab->ctrl.to_use = 1;
	slam_cfg_tab->ctrl.pyramid_en            = slam_request->req_orb.reg_cfg[layer_index].ctrl.pyramid_en;
	slam_cfg_tab->ctrl.gsblur_en             = slam_request->req_orb.reg_cfg[layer_index].ctrl.gsblur_en;
	slam_cfg_tab->ctrl.fast_en               = slam_request->req_orb.reg_cfg[layer_index].ctrl.fast_en;
	slam_cfg_tab->ctrl.nms_en                = slam_request->req_orb.reg_cfg[layer_index].ctrl.nms_en;
	slam_cfg_tab->ctrl.orient_en             = slam_request->req_orb.reg_cfg[layer_index].ctrl.orient_en;
	slam_cfg_tab->ctrl.brief_en              = slam_request->req_orb.reg_cfg[layer_index].ctrl.brief_en;
	slam_cfg_tab->ctrl.freak_en              = slam_request->req_orb.reg_cfg[layer_index].ctrl.freak_en;
	slam_cfg_tab->ctrl.scoremap_en           = slam_request->req_orb.reg_cfg[layer_index].ctrl.scoremap_en;
	slam_cfg_tab->ctrl.gridstat_en           = slam_request->req_orb.reg_cfg[layer_index].ctrl.gridstat_en;
	slam_cfg_tab->ctrl.undistort_en          = slam_request->req_orb.reg_cfg[layer_index].ctrl.undistort_en;
	slam_cfg_tab->size_cfg.to_use            = 1;
	slam_cfg_tab->pyramid_scaler_cfg.to_use  = slam_cfg_tab->ctrl.pyramid_en;
	slam_cfg_tab->gsblur_coef_cfg.to_use     = slam_cfg_tab->ctrl.gsblur_en;
	slam_cfg_tab->fast_coef.to_use           = slam_cfg_tab->ctrl.fast_en;
	slam_cfg_tab->nms_win.to_use             = slam_cfg_tab->ctrl.nms_en;
	slam_cfg_tab->gridstat_cfg.to_use        = slam_cfg_tab->ctrl.gridstat_en;
	slam_cfg_tab->octree_cfg.to_use          = 1;
	slam_cfg_tab->undistort_cfg.to_use       = slam_cfg_tab->ctrl.undistort_en;
	slam_cfg_tab->cvdr_cfg.to_use            = 0;
	slam_cfg_tab->size_cfg.width             = slam_request->req_orb.reg_cfg[layer_index].size_cfg.width;
	slam_cfg_tab->size_cfg.height            = slam_request->req_orb.reg_cfg[layer_index].size_cfg.height;
	slam_cfg_tab->pyramid_scaler_cfg.scl_inc     = slam_request->req_orb.reg_cfg[layer_index].pyramid_scaler_cfg.scl_inc;
	slam_cfg_tab->pyramid_scaler_cfg.scl_vtop    = slam_request->req_orb.reg_cfg[layer_index].pyramid_scaler_cfg.scl_vtop;
	slam_cfg_tab->pyramid_scaler_cfg.scl_vbottom =
		slam_request->req_orb.reg_cfg[layer_index].pyramid_scaler_cfg.scl_vbottom;
	slam_cfg_tab->pyramid_scaler_cfg.scl_hright  = slam_request->req_orb.reg_cfg[layer_index].pyramid_scaler_cfg.scl_hright;
	slam_cfg_tab->pyramid_scaler_cfg.scl_hleft   = slam_request->req_orb.reg_cfg[layer_index].pyramid_scaler_cfg.scl_hleft;
	slam_cfg_tab->gsblur_coef_cfg.coeff_gauss_0  = slam_request->req_orb.reg_cfg[layer_index].gsblur_coef_cfg.coeff_gauss_0;
	slam_cfg_tab->gsblur_coef_cfg.coeff_gauss_1  = slam_request->req_orb.reg_cfg[layer_index].gsblur_coef_cfg.coeff_gauss_1;
	slam_cfg_tab->gsblur_coef_cfg.coeff_gauss_2  = slam_request->req_orb.reg_cfg[layer_index].gsblur_coef_cfg.coeff_gauss_2;
	slam_cfg_tab->gsblur_coef_cfg.coeff_gauss_3  = slam_request->req_orb.reg_cfg[layer_index].gsblur_coef_cfg.coeff_gauss_3;
	slam_cfg_tab->fast_coef.ini_th               = slam_request->req_orb.reg_cfg[layer_index].fast_coef.ini_th;
	slam_cfg_tab->fast_coef.min_th               = slam_request->req_orb.reg_cfg[layer_index].fast_coef.min_th;
	slam_cfg_tab->nms_win.nmscell_h              = slam_request->req_orb.reg_cfg[layer_index].nms_win.nmscell_h;
	slam_cfg_tab->nms_win.nmscell_v              = slam_request->req_orb.reg_cfg[layer_index].nms_win.nmscell_v;
	slam_cfg_tab->gridstat_cfg.block_num.blk_h_num = slam_request->req_orb.reg_cfg[layer_index].gridstat_cfg.blk_h_num;
	slam_cfg_tab->gridstat_cfg.block_num.blk_v_num = slam_request->req_orb.reg_cfg[layer_index].gridstat_cfg.blk_v_num;
	slam_cfg_tab->gridstat_cfg.block_size.blk_h_size = slam_request->req_orb.reg_cfg[layer_index].gridstat_cfg.blk_h_size;
	slam_cfg_tab->gridstat_cfg.block_size.blk_v_size = slam_request->req_orb.reg_cfg[layer_index].gridstat_cfg.blk_v_size;
	slam_cfg_tab->octree_cfg.max_kpnum           = slam_request->req_orb.reg_cfg[layer_index].octree_cfg.max_kpnum;
	slam_cfg_tab->octree_cfg.grid_max_kpnum      = slam_request->req_orb.reg_cfg[layer_index].octree_cfg.grid_max_kpnum;
	slam_cfg_tab->octree_cfg.inc_level           = slam_request->req_orb.reg_cfg[layer_index].octree_cfg.inc_level;
	slam_update_request_undistort_cfg(slam_request, slam_cfg_tab, layer_index);
	loge_if(memcpy_s(slam_cfg_tab->octree_cfg.score_th, SLAM_SCORE_TH_RANGE * sizeof(unsigned int),
					 slam_request->req_orb.reg_cfg[layer_index].octree_cfg.score_th, SLAM_SCORE_TH_RANGE * sizeof(unsigned int)));
	loge_if(memcpy_s(&(slam_cfg_tab->pattern_cfg), 4 * SLAM_PATTERN_RANGE * sizeof(unsigned int),
					 &(slam_request->req_orb.reg_cfg[layer_index].pattern_cfg), 4 * SLAM_PATTERN_RANGE * sizeof(unsigned int)));
}

static int seg_src_cfg_slam_cmdlst_param(msg_req_slam_request_t *slam_request, cmdlst_para_t *slam_cmdlst_para,
		unsigned int slam_layer_stripe_num, unsigned int is_cmdlst_wr_en, unsigned int stripe_cnt)
{
	unsigned int i = 0;
	unsigned long long irq = 0;
	unsigned int cur_layer = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = slam_cmdlst_para->cmd_stripe_info;

	for (i = 0; i < stripe_cnt; i++) {
		irq = 0;
		cur_layer = i / slam_layer_stripe_num;
		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_SLAM;
		cmdlst_stripe[i].is_need_set_sop = 0;
		cmdlst_stripe[i].en_link         = 0;
		cmdlst_stripe[i].ch_link         = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;

		if (is_cmdlst_wr_en && (i % slam_layer_stripe_num != 0)) {
			irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_RD_EOF_CMDLST);
			irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_WR_EOF_CMDLST);
		} else {
			irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_RD_EOF_CMDLST);
			irq = irq | (((unsigned long long)(1)) << IPP_SLAM_IRQ_DONE);
			irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_RD_EOF_Y);

			if (slam_request->req_orb.streams[cur_layer][BO_SLAM_Y].buf)
				irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_WR_EOF_Y);

			if (slam_request->req_orb.streams[cur_layer][BO_SLAM_STAT].buf)
				irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_WR_EOF_STAT);

			if (slam_request->req_orb.streams[cur_layer][BO_SLAM_SCORE_MAP].buf)
				irq = irq | (((unsigned long long)(1)) << IPP_SLAM_CVDR_VP_WR_EOF_SCORE);
		}

		cmdlst_stripe[i].hw_priority = CMD_PRIO_LOW;
		cmdlst_stripe[i].irq_mode    = (~irq) & (0x3FFF);
	}

	cmdlst_stripe[stripe_cnt - 1].is_last_stripe  = 1;
	return CPE_FW_OK;
}


static int seg_src_cfg_slam_cmdlst(msg_req_slam_request_t *slam_request, cmdlst_para_t *slam_cmdlst_para)
{
	unsigned int slam_layer_stripe_num = 1;
	unsigned int cur_layer = 0;
	unsigned int is_cmdlst_wr_en = 0;
#if CMDLST_READ_FLAG

	if (slam_request->req_orb.streams[cur_layer][BO_SLAM_SCORE_HIST].buf) {
		is_cmdlst_wr_en = 1;
		slam_layer_stripe_num += 1;
	}

	if (slam_request->req_orb.streams[cur_layer][BO_SLAM_KPT_NUM].buf) {
		is_cmdlst_wr_en = 1;
		slam_layer_stripe_num += 1;
	}

	if (slam_request->req_orb.streams[cur_layer][BO_SLAM_TOTAL_KPT].buf) {
		is_cmdlst_wr_en = 1;
		slam_layer_stripe_num += 1;
	}

#endif
	unsigned int stripe_cnt = slam_request->slam_pyramid_layer * slam_layer_stripe_num;
	slam_cmdlst_para->stripe_cnt = stripe_cnt;
	i("slam_cmdlst_para->stripe_cnt = %d", slam_cmdlst_para->stripe_cnt);
	seg_src_cfg_slam_cmdlst_param(slam_request, slam_cmdlst_para, slam_layer_stripe_num, is_cmdlst_wr_en, stripe_cnt);
	return CPE_FW_OK;
}

static int seg_src_set_slam_cmdlst_para(cmdlst_para_t *cmdlst_para, msg_req_slam_request_t *slam_request,
										cfg_tab_slam_t *slam_cfg_tab, cfg_tab_cvdr_t *slam_cvdr_cfg_tab)
{
	unsigned int cur_layer = 0;
	unsigned int i = 0;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;
	unsigned int slam_layer_stripe_num = (cmdlst_para->stripe_cnt) / (slam_request->slam_pyramid_layer);

	for (i = 0; i < cmdlst_para->stripe_cnt;) {
		cur_layer = i / slam_layer_stripe_num;
		slam_prepare_cmd(&g_slam_devs[0], &cmd_link_entry[i].cmd_buf, &slam_cfg_tab[cur_layer]);
		cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &slam_cvdr_cfg_tab[cur_layer]);
		cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
		i++;
#if CMDLST_READ_FLAG
		unsigned int wr_addr_in_cmdlst_buf = 0;
		unsigned int rd_addr_for_cmdlst    = 0;

		if (slam_request->req_orb.streams[cur_layer][BO_SLAM_SCORE_HIST].buf) {
			wr_addr_in_cmdlst_buf = slam_request->req_orb.streams[cur_layer][BO_SLAM_SCORE_HIST].buf;
			rd_addr_for_cmdlst    = JPG_SLAM_ADDR + SLAM_SCORE_HIST_0_REG;
			loge_if_ret(seg_src_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, wr_addr_in_cmdlst_buf, rd_addr_for_cmdlst,
												  SLAM_SCOREHIST_NUM));
		}

		if (slam_request->req_orb.streams[cur_layer][BO_SLAM_KPT_NUM].buf) {
			wr_addr_in_cmdlst_buf = slam_request->req_orb.streams[cur_layer][BO_SLAM_KPT_NUM].buf;
			rd_addr_for_cmdlst    = JPG_SLAM_ADDR + SLAM_KPT_NUMBER_0_REG;
			loge_if_ret(seg_src_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, wr_addr_in_cmdlst_buf, rd_addr_for_cmdlst,
												  SLAM_KPT_NUM_RANGE));
		}

		if (slam_request->req_orb.streams[cur_layer][BO_SLAM_TOTAL_KPT].buf) {
			wr_addr_in_cmdlst_buf = slam_request->req_orb.streams[cur_layer][BO_SLAM_TOTAL_KPT].buf;
			rd_addr_for_cmdlst    = JPG_SLAM_ADDR + SLAM_TOTAL_KPT_NUM_REG;
			loge_if_ret(seg_src_set_cmdlst_wr_buf(&cmd_link_entry[i++].cmd_buf, wr_addr_in_cmdlst_buf, rd_addr_for_cmdlst, 1));
		}

#endif
	}

	return CPE_FW_OK;
}


int slam_request_handler(msg_req_slam_request_t *slam_request)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;
	d("+\n");

	if (slam_request == NULL) {
		e("[%s] failed : slam_request is null", __func__);
		return -1;
	}

	if (slam_request->slam_pyramid_layer == 0) {
		e("[%s] failed : slam_pyramid_layer is 0", __func__);
		return -1;
	}

	slam_request_dump(slam_request);

	if ((ret = cpe_mem_get(MEM_ID_SLAM_CFG_TAB, &va, &da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_SLAM_CFG_TAB);
		return CPE_FW_ERR;
	}

	cfg_tab_slam_t *slam_cfg_tab = (cfg_tab_slam_t *)(uintptr_t)va;

	if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_SLAM, &va, &da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_SLAM);
		return CPE_FW_ERR;
	}

	cfg_tab_cvdr_t *slam_cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)va;

	if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_SLAM, &va, &da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_SLAM);
		return CPE_FW_ERR;
	}

	cmdlst_para_t *slam_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	unsigned int i = 0;

	for (i = 0; i < slam_request->slam_pyramid_layer; i++) {
		slam_update_request_cfg_tab(slam_request, &slam_cfg_tab[i], i);
		slam_update_cvdr_cfg_tab(slam_request, &slam_cvdr_cfg_tab[i], i);
	}

	slam_cmdlst_para->channel_id = SLAM_CHANNEL;
	slam_cmdlst_para->stripe_cnt = slam_request->slam_pyramid_layer;
	seg_src_cfg_slam_cmdlst(slam_request, slam_cmdlst_para);
	df_sched_prepare(slam_cmdlst_para);
	df_sched_set_buffer_header(slam_cmdlst_para);
	seg_src_set_slam_cmdlst_para(slam_cmdlst_para, slam_request, slam_cfg_tab, slam_cvdr_cfg_tab);
	ipp_update_cmdlst_cfg_tab(slam_cmdlst_para);
	loge_if_ret(df_sched_start(slam_cmdlst_para));
	cpe_mem_free(MEM_ID_SLAM_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_SLAM);
	d("-\n");
	return CPE_FW_OK;
}

#if FLAG_LOG_DEBUG
static int slam_request_dump_ctrl_size(msg_req_slam_request_t *req, unsigned int i)
{
	d("req->req_orb.reg_cfg[%d].ctrl.pyramid_en 				 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.pyramid_en);
	d("req->req_orb.reg_cfg[%d].ctrl.gsblur_en					 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.gsblur_en);
	d("req->req_orb.reg_cfg[%d].ctrl.fast_en					 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.fast_en);
	d("req->req_orb.reg_cfg[%d].ctrl.nms_en 					 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.nms_en);
	d("req->req_orb.reg_cfg[%d].ctrl.orient_en					 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.orient_en);
	d("req->req_orb.reg_cfg[%d].ctrl.brief_en					 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.brief_en);
	d("req->req_orb.reg_cfg[%d].ctrl.freak_en					 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.freak_en);
	d("req->req_orb.reg_cfg[%d].ctrl.scoremap_en				 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.scoremap_en);
	d("req->req_orb.reg_cfg[%d].ctrl.gridstat_en				 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.gridstat_en);
	d("req->req_orb.reg_cfg[%d].ctrl.undistort_en				 = %d\n", i,
	  req->req_orb.reg_cfg[i].ctrl.undistort_en);
	d("req->req_orb.reg_cfg[%d].size_cfg.width					 = %d\n", i,
	  req->req_orb.reg_cfg[i].size_cfg.width);
	d("req->req_orb.reg_cfg[%d].size_cfg.height 				 = %d\n", i,
	  req->req_orb.reg_cfg[i].size_cfg.height);

	return CPE_FW_OK;
}

static int slam_request_dump_pyramid_gsblur(msg_req_slam_request_t *req, unsigned int i)
{
	d("req->req_orb.reg_cfg[%d].pyramid_scaler_cfg.scl_inc		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pyramid_scaler_cfg.scl_inc);
	d("req->req_orb.reg_cfg[%d].pyramid_scaler_cfg.scl_vtop 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pyramid_scaler_cfg.scl_vtop);
	d("req->req_orb.reg_cfg[%d].pyramid_scaler_cfg.scl_vbottom	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pyramid_scaler_cfg.scl_vbottom);
	d("req->req_orb.reg_cfg[%d].pyramid_scaler_cfg.scl_hright	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pyramid_scaler_cfg.scl_hright);
	d("req->req_orb.reg_cfg[%d].pyramid_scaler_cfg.scl_hleft	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pyramid_scaler_cfg.scl_hleft);
	d("req->req_orb.reg_cfg[%d].gsblur_coef_cfg.coeff_gauss_0	 = %d\n", i,
	  req->req_orb.reg_cfg[i].gsblur_coef_cfg.coeff_gauss_0);
	d("req->req_orb.reg_cfg[%d].gsblur_coef_cfg.coeff_gauss_1	 = %d\n", i,
	  req->req_orb.reg_cfg[i].gsblur_coef_cfg.coeff_gauss_1);
	d("req->req_orb.reg_cfg[%d].gsblur_coef_cfg.coeff_gauss_2	 = %d\n", i,
	  req->req_orb.reg_cfg[i].gsblur_coef_cfg.coeff_gauss_2);
	d("req->req_orb.reg_cfg[%d].gsblur_coef_cfg.coeff_gauss_3	 = %d\n", i,
	  req->req_orb.reg_cfg[i].gsblur_coef_cfg.coeff_gauss_3);
	d("req->req_orb.reg_cfg[%d].fast_coef.ini_th				 = %d\n", i,
	  req->req_orb.reg_cfg[i].fast_coef.ini_th);
	d("req->req_orb.reg_cfg[%d].fast_coef.min_th				 = %d\n", i,
	  req->req_orb.reg_cfg[i].fast_coef.min_th);
	d("req->req_orb.reg_cfg[%d].nms_win.nmscell_h				 = %d\n", i,
	  req->req_orb.reg_cfg[i].nms_win.nmscell_h);
	d("req->req_orb.reg_cfg[%d].nms_win.nmscell_v				 = %d\n", i,
	  req->req_orb.reg_cfg[i].nms_win.nmscell_v);
	d("req->req_orb.reg_cfg[%d].gridstat_cfg.blk_h_num			 = %d\n", i,
	  req->req_orb.reg_cfg[i].gridstat_cfg.blk_h_num);
	d("req->req_orb.reg_cfg[%d].gridstat_cfg.blk_v_num			 = %d\n", i,
	  req->req_orb.reg_cfg[i].gridstat_cfg.blk_v_num);
	d("req->req_orb.reg_cfg[%d].gridstat_cfg.blk_h_size 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].gridstat_cfg.blk_h_size);
	d("req->req_orb.reg_cfg[%d].gridstat_cfg.blk_v_size 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].gridstat_cfg.blk_v_size);

	return CPE_FW_OK;
}

static int slam_request_dump_octree_undistort(msg_req_slam_request_t *req, unsigned int i)
{
	d("req->req_orb.reg_cfg[%d].octree_cfg.max_kpnum			 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.max_kpnum);
	d("req->req_orb.reg_cfg[%d].octree_cfg.grid_max_kpnum		 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.grid_max_kpnum);
	d("req->req_orb.reg_cfg[%d].octree_cfg.inc_level			 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.inc_level);
	d("req->req_orb.reg_cfg[%d].octree_cfg.score_th[0]			 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.score_th[0]);
	d("req->req_orb.reg_cfg[%d].octree_cfg.score_th[20] 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.score_th[20]);
	d("req->req_orb.reg_cfg[%d].octree_cfg.score_th[40] 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.score_th[40]);
	d("req->req_orb.reg_cfg[%d].octree_cfg.score_th[60] 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.score_th[60]);
	d("req->req_orb.reg_cfg[%d].octree_cfg.score_th[80] 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.score_th[80]);
	d("req->req_orb.reg_cfg[%d].octree_cfg.score_th[93] 		 = %d\n", i,
	  req->req_orb.reg_cfg[i].octree_cfg.score_th[93]);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.cx				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.cx);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.cy				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.cy);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.fx				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.fx);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.fy				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.fy);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.invfx 			 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.invfx);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.invfy 			 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.invfy);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.k1				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.k1);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.k2				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.k2);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.k3				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.k3);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.p1				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.p1);
	d("req->req_orb.reg_cfg[%d].undistort_cfg.p2				 = %d\n", i,
	  req->req_orb.reg_cfg[i].undistort_cfg.p2);

	return CPE_FW_OK;
}

static int slam_request_dump_pattern_cfg(msg_req_slam_request_t *req, unsigned int i)
{
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x0[0]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x0[0]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y0[0]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y0[0]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x1[0]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x1[0]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y1[0]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y1[0]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x0[50]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x0[50]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y0[50]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y0[50]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x1[50]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x1[50]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y1[50]		 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y1[50]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x0[150] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x0[150]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y0[150] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y0[150]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x1[150] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x1[150]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y1[150] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y1[150]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x0[255] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x0[255]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y0[255] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y0[255]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_x1[255] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_x1[255]);
	d("req->req_orb.reg_cfg[%d].pattern_cfg.pattern_y1[255] 	 = %d\n", i,
	  req->req_orb.reg_cfg[i].pattern_cfg.pattern_y1[255]);

	return CPE_FW_OK;
}
#endif

static int slam_request_dump(msg_req_slam_request_t *req)
{
#if FLAG_LOG_DEBUG
	unsigned int i = 0;
	unsigned int j = 0;
	d("size_of_orb_req = %d\n", (int)(sizeof(msg_req_slam_request_t)));
	d("req->frame_num = %d\n", req->frame_num);
	i("req->slam_pyramid_layer = %d\n", req->slam_pyramid_layer);

	for (i = 0; i < req->slam_pyramid_layer; i++) {
		for (j = 0; j < SLAM_STREAM_MAX; j++) {
			d("orb streams[%d][%d].width = %d\n",  i, j, req->req_orb.streams[i][j].width);
			d("orb streams[%d][%d].height = %d\n", i, j, req->req_orb.streams[i][j].height);
			d("orb streams[%d][%d].stride = %d\n", i, j, req->req_orb.streams[i][j].stride);
			d("orb streams[%d][%d].buf = 0x%x",    i, j, req->req_orb.streams[i][j].buf);
			d("orb streams[%d][%d].format = %d\n", i, j, req->req_orb.streams[i][j].format);
		}

		slam_request_dump_ctrl_size(req, i);
		slam_request_dump_pyramid_gsblur(req, i);
		slam_request_dump_octree_undistort(req, i);
		slam_request_dump_pattern_cfg(req, i);
	}

	d("req->req_orb.secure_flag 								 = %d\n", req->req_orb.secure_flag);
	d("req->req_orb.slam_rate_value 							 = %d\n", req->req_orb.slam_rate_value);
#else
	i("req->slam_pyramid_layer = %d\n", req->slam_pyramid_layer);
#endif
	return 0;
}

#pragma GCC diagnostic pop
