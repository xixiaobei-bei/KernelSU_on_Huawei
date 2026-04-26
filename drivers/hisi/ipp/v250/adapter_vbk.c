#include <linux/printk.h>
#include <linux/slab.h>
#include "vbk.h"
#include "cfg_table_vbk.h"
#include "cvdr_drv.h"
#include "vbk_drv.h"
#include "ipp_top_drv.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "sub_ctrl_reg_offset.h"

#define LOG_TAG LOG_MODULE_VBK

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
/*lint -e429  -e613  -e668*/

extern vbk_dev_t g_vbk_devs[];

static int vbk_dump_request(msg_req_vbk_request_t *req);

static int vbk_update_cvdr_rd_cfg_tab_src_ds16(msg_req_vbk_request_t *vbk_request, cfg_tab_cvdr_t *cfg_tab,
					cvdr_opt_fmt_t *cfmt)
{
	pix_format_e format = PIXEL_FMT_CPE_MAX;

	if (vbk_request->streams[BI_VBK_SRC_Y_DS16].buf) {
		cfmt->id = CPE_RD0;
		cfmt->width = vbk_request->streams[BI_VBK_SRC_Y_DS16].width;
		cfmt->line_size = vbk_request->streams[BI_VBK_SRC_Y_DS16].width;
		cfmt->height = vbk_request->streams[BI_VBK_SRC_Y_DS16].height;
		cfmt->addr = vbk_request->streams[BI_VBK_SRC_Y_DS16].buf;
		cfmt->full_width = vbk_request->streams[BI_VBK_SRC_Y_DS16].width;
		cfmt->expand = 0;
		format = vbk_request->streams[BI_VBK_SRC_Y_DS16].format;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, cfmt, 0, CPE_CLK, format));
	}

	if (vbk_request->streams[BI_VBK_SRC_UV_DS16].buf) {
		cfmt->id = CPE_RD1;
		cfmt->width = vbk_request->streams[BI_VBK_SRC_UV_DS16].width;
		cfmt->line_size = vbk_request->streams[BI_VBK_SRC_UV_DS16].width;
		cfmt->height = vbk_request->streams[BI_VBK_SRC_UV_DS16].height;
		cfmt->addr = vbk_request->streams[BI_VBK_SRC_UV_DS16].buf;
		cfmt->full_width = vbk_request->streams[BI_VBK_SRC_UV_DS16].width;
		cfmt->expand = 0;
		format = vbk_request->streams[BI_VBK_SRC_UV_DS16].format;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, cfmt, 0, CPE_CLK, format));
	}

	return CPE_FW_OK;
}

static int vbk_update_cvdr_rd_cfg_tab_src_uv(msg_req_vbk_request_t *vbk_request, cfg_tab_cvdr_t *cfg_tab,
				cvdr_opt_fmt_t *cfmt)
{
	pix_format_e format = PIXEL_FMT_CPE_MAX;

	if (vbk_request->streams[BI_VBK_SRC_UV].buf) {
		cfmt->id = CPE_RD2;
		cfmt->width = vbk_request->streams[BI_VBK_SRC_UV].width;
		cfmt->line_size = vbk_request->streams[BI_VBK_SRC_UV].width;
		cfmt->height = vbk_request->streams[BI_VBK_SRC_UV].height;
		cfmt->addr = vbk_request->streams[BI_VBK_SRC_UV].buf;
		cfmt->full_width = vbk_request->streams[BI_VBK_SRC_UV].width;
		cfmt->expand = 0;
		format = vbk_request->streams[BI_VBK_SRC_UV].format;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, cfmt, 0, CPE_CLK, format));
	}

	return CPE_FW_OK;
}

static int vbk_update_cvdr_rd_cfg_tab(msg_req_vbk_request_t *vbk_request, cfg_tab_cvdr_t *cfg_tab)
{
	cvdr_opt_fmt_t cfmt;
	loge_if(memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t)));
	pix_format_e format = PIXEL_FMT_CPE_MAX;

	vbk_update_cvdr_rd_cfg_tab_src_ds16(vbk_request, cfg_tab, &cfmt);
	vbk_update_cvdr_rd_cfg_tab_src_uv(vbk_request, cfg_tab, &cfmt);

	if (vbk_request->streams[BI_VBK_SIGMA].buf) {
		cfmt.id = CPE_RD3;
		cfmt.width = vbk_request->streams[BI_VBK_SIGMA].width;
		cfmt.line_size = vbk_request->streams[BI_VBK_SIGMA].width;
		cfmt.height = vbk_request->streams[BI_VBK_SIGMA].height;
		cfmt.addr = vbk_request->streams[BI_VBK_SIGMA].buf;
		cfmt.full_width = vbk_request->streams[BI_VBK_SIGMA].width;
		cfmt.expand = 0;
		format = vbk_request->streams[BI_VBK_SIGMA].format;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	if (vbk_request->streams[BI_VBK_SRC_Y].buf) {
		cfmt.id = CPE_RD4;
		cfmt.width = vbk_request->streams[BI_VBK_SRC_Y].width;
		cfmt.line_size = vbk_request->streams[BI_VBK_SRC_Y].width / 2;
		cfmt.height = vbk_request->streams[BI_VBK_SRC_Y].height;
		cfmt.addr = vbk_request->streams[BI_VBK_SRC_Y].buf;
		cfmt.full_width = vbk_request->streams[BI_VBK_SRC_Y].width / 2;
		cfmt.expand = 0;
		format = vbk_request->streams[BI_VBK_SRC_Y].format;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	if (vbk_request->streams[BI_VBK_FOREMASK].buf) {
		cfmt.id = CPE_RD5;
		cfmt.width = vbk_request->streams[BI_VBK_FOREMASK].width;
		cfmt.line_size = vbk_request->streams[BI_VBK_FOREMASK].width;
		cfmt.height = vbk_request->streams[BI_VBK_FOREMASK].height;
		cfmt.addr = vbk_request->streams[BI_VBK_FOREMASK].buf;
		cfmt.full_width = vbk_request->streams[BI_VBK_FOREMASK].width;
		cfmt.expand = 0;
		format = vbk_request->streams[BI_VBK_FOREMASK].format;
		loge_if_ret(dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	return CPE_FW_OK;
}

static int vbk_update_cvdr_wr_cfg_tab(msg_req_vbk_request_t *vbk_request, cfg_tab_cvdr_t *cfg_tab)
{
	cvdr_opt_fmt_t cfmt;
	loge_if(memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t)));
	pix_format_e format = PIXEL_FMT_CPE_MAX;

	if (vbk_request->streams[BO_VBK_RESULT_UV].buf) {
		cfmt.id = CPE_WR0;
		cfmt.width = vbk_request->streams[BO_VBK_RESULT_UV].width;
		cfmt.line_size = vbk_request->streams[BO_VBK_RESULT_UV].width;
		cfmt.full_width = vbk_request->streams[BO_VBK_RESULT_UV].width;
		cfmt.height = vbk_request->streams[BO_VBK_RESULT_UV].height;
		cfmt.expand = 0;
		format = vbk_request->streams[BO_VBK_RESULT_UV].format;
		cfmt.addr = vbk_request->streams[BO_VBK_RESULT_UV].buf;
		loge_if_ret(dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	if (vbk_request->streams[BO_VBK_RESULT_Y].buf) {
		cfmt.id = CPE_WR1;
		cfmt.width = vbk_request->streams[BO_VBK_RESULT_Y].width;
		cfmt.line_size = vbk_request->streams[BO_VBK_RESULT_Y].width / 2;
		cfmt.full_width = vbk_request->streams[BO_VBK_RESULT_Y].width / 2;
		cfmt.height = vbk_request->streams[BO_VBK_RESULT_Y].height;
		cfmt.expand = 0;
		format = vbk_request->streams[BO_VBK_RESULT_Y].format;
		cfmt.addr = vbk_request->streams[BO_VBK_RESULT_Y].buf;
		loge_if_ret(dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	if (vbk_request->streams[BO_VBK_RESULT_FOREMASK].buf) {
		cfmt.id = CPE_WR2;
		cfmt.width = vbk_request->streams[BO_VBK_RESULT_FOREMASK].width;
		cfmt.line_size = vbk_request->streams[BO_VBK_RESULT_FOREMASK].width;
		cfmt.full_width = vbk_request->streams[BO_VBK_RESULT_FOREMASK].width;
		cfmt.height = vbk_request->streams[BO_VBK_RESULT_FOREMASK].height;
		cfmt.expand = 0;
		format = vbk_request->streams[BO_VBK_RESULT_FOREMASK].format;
		cfmt.addr = vbk_request->streams[BO_VBK_RESULT_FOREMASK].buf;
		loge_if_ret(dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format));
	}

	return CPE_FW_OK;
}

static int vbk_update_vbk_cfg_tab(msg_req_vbk_request_t *vbk_request, vbk_config_table_t *cfg_tab)
{
	cfg_tab->to_use = 1;
	cfg_tab->vbk_ctrl.to_use = 1;
	cfg_tab->vbk_ctrl.op_mode = vbk_request->vbk_hw_config.op_mode;
	cfg_tab->vbk_ctrl.y_ds16_gauss_en = vbk_request->vbk_hw_config.y_ds16_gauss_en;
	cfg_tab->vbk_ctrl.uv_ds16_gauss_en = vbk_request->vbk_hw_config.uv_ds16_gauss_en;
	cfg_tab->vbk_ctrl.sigma_gauss_en = vbk_request->vbk_hw_config.sigma_gauss_en;
	cfg_tab->vbk_ctrl.sigma2alpha_en = vbk_request->vbk_hw_config.sigma2alpha_en;
	cfg_tab->vbk_ctrl.in_width_ds4 = vbk_request->vbk_hw_config.in_width_ds4;
	cfg_tab->vbk_ctrl.in_height_ds4 = vbk_request->vbk_hw_config.in_height_ds4;
	cfg_tab->yds16_gauss_coeff.to_use = cfg_tab->vbk_ctrl.y_ds16_gauss_en;
	cfg_tab->yds16_gauss_coeff.g00 = vbk_request->vbk_hw_config.yds16_gauss_coeff_g00;
	cfg_tab->yds16_gauss_coeff.g01 = vbk_request->vbk_hw_config.yds16_gauss_coeff_g01;
	cfg_tab->yds16_gauss_coeff.g02 = vbk_request->vbk_hw_config.yds16_gauss_coeff_g02;
	cfg_tab->yds16_gauss_coeff.g03 = vbk_request->vbk_hw_config.yds16_gauss_coeff_g03;
	cfg_tab->yds16_gauss_coeff.g04 = vbk_request->vbk_hw_config.yds16_gauss_coeff_g04;
	cfg_tab->yds16_gauss_coeff.inv = vbk_request->vbk_hw_config.yds16_gauss_coeff_inv;
	cfg_tab->sigma_gauss_coeff.to_use = cfg_tab->vbk_ctrl.sigma_gauss_en;
	cfg_tab->sigma_gauss_coeff.g00 = vbk_request->vbk_hw_config.sigma_gauss_coeff_g00;
	cfg_tab->sigma_gauss_coeff.g01 = vbk_request->vbk_hw_config.sigma_gauss_coeff_g01;
	cfg_tab->sigma_gauss_coeff.g02 = vbk_request->vbk_hw_config.sigma_gauss_coeff_g02;
	cfg_tab->sigma_gauss_coeff.g03 = vbk_request->vbk_hw_config.sigma_gauss_coeff_g03;
	cfg_tab->sigma_gauss_coeff.g04 = vbk_request->vbk_hw_config.sigma_gauss_coeff_g04;
	cfg_tab->sigma_gauss_coeff.inv = vbk_request->vbk_hw_config.sigma_gauss_coeff_inv;
	cfg_tab->uvds16_gauss_coeff.to_use = cfg_tab->vbk_ctrl.uv_ds16_gauss_en;
	cfg_tab->uvds16_gauss_coeff.g00 = vbk_request->vbk_hw_config.uvds16_gauss_coeff_g00;
	cfg_tab->uvds16_gauss_coeff.g01 = vbk_request->vbk_hw_config.uvds16_gauss_coeff_g01;
	cfg_tab->uvds16_gauss_coeff.g02 = vbk_request->vbk_hw_config.uvds16_gauss_coeff_g02;
	cfg_tab->uvds16_gauss_coeff.inv = vbk_request->vbk_hw_config.uvds16_gauss_coeff_inv;
	cfg_tab->sigma2alpha.to_use = cfg_tab->vbk_ctrl.sigma2alpha_en;
	loge_if(memcpy_s(&cfg_tab->sigma2alpha.sigma2alpha[0], VBK_SIGMA2ALPHA_NUM * sizeof(unsigned int),
					 &vbk_request->vbk_hw_config.sigma2alpha_lut[0], VBK_SIGMA2ALPHA_NUM * sizeof(unsigned int)));
	cfg_tab->addnoise.addnoise_th = vbk_request->vbk_hw_config.addnoise_th;
	cfg_tab->foremask_cfg.foremask_coeff = vbk_request->vbk_hw_config.foremask_coeff;
	cfg_tab->foremask_cfg.foremask_th = vbk_request->vbk_hw_config.foremask_th;
	cfg_tab->foremask_cfg.foremask_weighted_filter_en = vbk_request->vbk_hw_config.foremask_weighted_filter_en;
	cfg_tab->foremask_cfg.foremask_dilation_radius = vbk_request->vbk_hw_config.foremask_dilation_radius;

	if (cfg_tab->vbk_ctrl.op_mode == VBK_FRONT_FACE_MODE) {
		cfg_tab->addnoise.to_use = 1;
		cfg_tab->foremask_cfg.to_use = 0;
	} else if (cfg_tab->vbk_ctrl.op_mode == VBK_BOKEH_MODE) {
		cfg_tab->addnoise.to_use = 0;
		cfg_tab->foremask_cfg.to_use = 1;
	} else {
		e("[%s] Failed : Wrong mode input for VBK, mode = %d\n", __func__, cfg_tab->vbk_ctrl.op_mode);
		return CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

static int vbk_set_irq_mode(msg_req_vbk_request_t *vbk_request, cmdlst_para_t *cmdlst_para)
{
	unsigned int irq_mode = 0;

	if (vbk_request->vbk_hw_config.op_mode == VBK_BOKEH_MODE) {
		irq_mode = 0x03FFFA00;
	} else if (vbk_request->vbk_hw_config.op_mode == VBK_FRONT_FACE_MODE) {
		irq_mode = 0x03FFFB04;
	} else {
		e("[%s] failed : vbk mode invilid, %d", __func__, vbk_request->vbk_hw_config.op_mode);
		return CPE_FW_ERR;
	}

	cmdlst_para->cmd_stripe_info[0].irq_mode = irq_mode;
	return CPE_FW_OK;
}

int vbk_request_handler(msg_req_vbk_request_t *vbk_request)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;

	if (vbk_request == NULL) {
		e("[%s] failed : vbk_request is null", __func__);
		return CPE_FW_ERR;
	}

	vbk_dump_request(vbk_request);
	hispcpe_reg_set(CPE_TOP, SUB_CTRL_CPE_MODE_CFG_REG, 2);

	if ((ret = cpe_mem_get(MEM_ID_VBK_CFG_TAB, &va, &da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_VBK_CFG_TAB);
		return CPE_FW_ERR;
	}

	vbk_config_table_t *vbk_cfg_tab = (vbk_config_table_t *)(uintptr_t)va;

	if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_MM, &va, &da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_MM);
		return CPE_FW_ERR;
	}

	cfg_tab_cvdr_t *cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)va;
	vbk_update_vbk_cfg_tab(vbk_request, vbk_cfg_tab);
	vbk_update_cvdr_wr_cfg_tab(vbk_request, cvdr_cfg_tab);
	vbk_update_cvdr_rd_cfg_tab(vbk_request, cvdr_cfg_tab);

	if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_MM, &va, &da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_MM);
		return CPE_FW_ERR;
	}

	cmdlst_para_t *cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	cmdlst_para->channel_id = CPE_CHANNEL;
	cmdlst_para->stripe_cnt = 1;
	cpe_set_cmdlst_stripe(&cmdlst_para->cmd_stripe_info[0], cmdlst_para->stripe_cnt);
	df_sched_prepare(cmdlst_para);
	schedule_cmdlst_link_t *cmd_link_entry = cmdlst_para->cmd_entry;
	vbk_set_irq_mode(vbk_request, cmdlst_para);
	df_sched_set_buffer_header(cmdlst_para);
	vbk_prepare_cmd(&g_vbk_devs[0], &cmd_link_entry->cmd_buf, vbk_cfg_tab);
	cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry->cmd_buf, cvdr_cfg_tab);
	cmdlst_set_buffer_padding(&cmd_link_entry->cmd_buf);
	ipp_update_cmdlst_cfg_tab(cmdlst_para);
	loge_if_ret(df_sched_start(cmdlst_para));
	cpe_mem_free(MEM_ID_VBK_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_MM);
	return CPE_FW_OK;
}

static int vbk_dump_request(msg_req_vbk_request_t *req)
{
#if FLAG_LOG_DEBUG
	int i = 0;
	i("size of msg_req_vbk_request_t = %lu", sizeof(msg_req_vbk_request_t));
	i("frame_number = %d", req->frame_number);

	for (i = 0; i < VBK_STREAM_MAX; i++)
		i("streams[%d].width = %d,height = %d,buf = 0x%x,format = %d\n",
		  i, req->streams[i].width, req->streams[i].height, req->streams[i].buf, req->streams[i].format);

	d("vbk_hw_config_t.mode = %d", req->vbk_hw_config.op_mode);
	d("req->vbk_hw_config_t.y_ds16_gauss_en = %d ", req->vbk_hw_config.y_ds16_gauss_en);
	d("req->vbk_hw_config_t.uv_ds16_gauss_en = %d ", req->vbk_hw_config.uv_ds16_gauss_en);
	d("req->vbk_hw_config_t.sigma_gauss_en = %d ", req->vbk_hw_config.sigma_gauss_en);
	d("req->vbk_hw_config_t.sigma2alpha_en = %d ", req->vbk_hw_config.sigma2alpha_en);
	d("req->vbk_hw_config_t.in_width_ds4 = %d ", req->vbk_hw_config.in_width_ds4);
	d("req->vbk_hw_config_t.in_height_ds4 = %d ", req->vbk_hw_config.in_height_ds4);
	d("req->vbk_hw_config_t.yds16_gauss_coeff_g00 = %d ", req->vbk_hw_config.yds16_gauss_coeff_g00);
	d("req->vbk_hw_config_t.yds16_gauss_coeff_inv = %d ", req->vbk_hw_config.yds16_gauss_coeff_inv);
	d("req->vbk_hw_config_t.sigma_gauss_coeff_g00 = %d ", req->vbk_hw_config.sigma_gauss_coeff_g00);
	d("req->vbk_hw_config_t.sigma_gauss_coeff_inv = %d ", req->vbk_hw_config.sigma_gauss_coeff_inv);
	d("req->vbk_hw_config_t.uvds16_gauss_coeff_g00 = %d ", req->vbk_hw_config.uvds16_gauss_coeff_g00);
	d("req->vbk_hw_config_t.uvds16_gauss_coeff_inv = %d ", req->vbk_hw_config.uvds16_gauss_coeff_inv);
	d("req->vbk_hw_config_t.addnoise_th = %d ", req->vbk_hw_config.addnoise_th);
	d("req->vbk_hw_config_t.foremask_coeff = %d ", req->vbk_hw_config.foremask_coeff);
	d("req->vbk_hw_config_t.foremask_th = %d ", req->vbk_hw_config.foremask_th);
	d("req->vbk_hw_config_t.foremask_weighted_filter_en = %d ", req->vbk_hw_config.foremask_weighted_filter_en);
	d("req->vbk_hw_config_t.foremask_dilation_radius = %d ", req->vbk_hw_config.foremask_dilation_radius);
	d("req->vbk_hw_config_t.sigma2alpha_lut[0] = %d ", req->vbk_hw_config.sigma2alpha_lut[0]);
	d("req->vbk_hw_config_t.sigma2alpha_lut[50] = %d ", req->vbk_hw_config.sigma2alpha_lut[50]);
	d("req->vbk_hw_config_t.sigma2alpha_lut[100] = %d ", req->vbk_hw_config.sigma2alpha_lut[100]);
	d("req->vbk_hw_config_t.sigma2alpha_lut[255] = %d ", req->vbk_hw_config.sigma2alpha_lut[255]);
#else
	i("streams[4].width = %d,height = %d,format = %d\n",
		req->streams[4].width, req->streams[4].height, req->streams[4].format);
#endif

#if VBK_RATE_SET_BY_HAL
	d("req->vbk_rate_value = %d ", req->vbk_rate_value);
#endif
	return CPE_FW_OK;
}
