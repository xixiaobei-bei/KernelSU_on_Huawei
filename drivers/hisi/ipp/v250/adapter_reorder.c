/*lint -e679
-esym(679,*)*/

#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "reorder.h"
#include "cfg_table_reorder.h"
#include "cvdr_drv.h"
#include "reorder_drv.h"
#include "config_table_cvdr.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "reorder_drv_priv.h"
#include "sub_ctrl_reg_offset.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define LOG_TAG LOG_MODULE_REORDER
#define CVDR_KEY_POINT_OVERFLOW  0x8000

extern reorder_dev_t g_reorder_devs[];
unsigned int g_reorder_read_st = 1;

static int reorder_request_dump(msg_req_reorder_request_t *req);

static int reorder_set_irq_mode(cmdlst_para_t *cmdlst_para, unsigned int stripe_cnt)
{
	unsigned int i = 0;

	for (i = 0; i < stripe_cnt; i++)
		cmdlst_para->cmd_stripe_info[i].irq_mode = 0x14;

	return 0;
}

static int reorder_update_cvdr_cfg_tab(msg_req_reorder_request_t *reorder_request, cfg_tab_cvdr_t *reorder_cfg_tab)
{
	cvdr_opt_fmt_t cfmt;
	unsigned int stride = 0;

	if (reorder_request->streams[BI_RDR_FP].buf) {
		cfmt.id = RD_RDR_DESC;
		cfmt.width = reorder_request->streams[BI_RDR_FP].width;
		cfmt.full_width = reorder_request->streams[BI_RDR_FP].width;
		cfmt.line_size = reorder_request->streams[BI_RDR_FP].width * 2;
		cfmt.height = reorder_request->streams[BI_RDR_FP].height;
		cfmt.expand = EXP_PIX;
		stride = reorder_request->streams[BI_RDR_FP].width;
		cfmt.addr = reorder_request->streams[BI_RDR_FP].buf;
		cfg_tbl_cvdr_rd_cfg_d64(reorder_cfg_tab, &cfmt, (ipp_align_up(80 * 4000, CVDR_ALIGN_BYTES)), stride);
	}

	if (reorder_request->streams[BO_RDR_FP_BLOCK].buf)
		cfg_tbl_cvdr_nr_wr_cfg(reorder_cfg_tab, WR_RDR_DESC);

	return 0;
}

static void reorder_update_request_cfg_tab(msg_req_reorder_request_t *reorder_request,
		cfg_tab_reorder_t *reorder_cfg_tab)
{
	reorder_cfg_tab->reorder_ctrl_cfg.to_use = 1;
	reorder_cfg_tab->reorder_ctrl_cfg.reorder_en = reorder_request->reg_cfg.reorder_ctrl_cfg.reorder_en;
	reorder_cfg_tab->reorder_ctrl_cfg.descriptor_type = reorder_request->reg_cfg.reorder_ctrl_cfg.descriptor_type;
	reorder_cfg_tab->reorder_ctrl_cfg.total_kpt = reorder_request->reg_cfg.reorder_ctrl_cfg.total_kpt;
	reorder_cfg_tab->reorder_block_cfg.to_use = 1;
	reorder_cfg_tab->reorder_block_cfg.blk_v_num = reorder_request->reg_cfg.reorder_block_cfg.blk_v_num;
	reorder_cfg_tab->reorder_block_cfg.blk_h_num = reorder_request->reg_cfg.reorder_block_cfg.blk_h_num;
	reorder_cfg_tab->reorder_block_cfg.blk_num = reorder_request->reg_cfg.reorder_block_cfg.blk_num;
	reorder_cfg_tab->reorder_prefetch_cfg.to_use = 1;
	reorder_cfg_tab->reorder_prefetch_cfg.prefetch_enable = 1;
	reorder_cfg_tab->reorder_prefetch_cfg.first_32k_page = reorder_request->streams[BO_RDR_FP_BLOCK].buf >> 15;
	reorder_cfg_tab->reorder_kptnum_cfg.to_use = 1;
	loge_if(memcpy_s(&(reorder_cfg_tab->reorder_kptnum_cfg.reorder_kpt_num[0]), REORDER_KPT_NUM * sizeof(unsigned int),
					 &(reorder_request->reg_cfg.reorder_kpt_num[0]), REORDER_KPT_NUM * sizeof(unsigned int)));
}

static void reorder_set_cmdlst_stripe(cmdlst_stripe_info_t *stripe, unsigned int stripe_cnt)
{
	unsigned int i = 0;

	for (i = 0; i < stripe_cnt; i++) {
		if (i == stripe_cnt - 1)
			stripe[i].is_last_stripe   = 1;
		else
			stripe[i].is_last_stripe   = 0;
	}
}

int reorder_request_handler(msg_req_reorder_request_t *reorder_request)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	unsigned int i = 0;
	d("+\n");

	if (reorder_request == NULL) {
		e("[%s] failed : reorder_request is null", __func__);
		return -1;
	}

	reorder_request_dump(reorder_request);

	if (cpe_mem_get(MEM_ID_REORDER_CFG_TAB, &va, &da) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_REORDER_CFG_TAB);
		return CPE_FW_ERR;
	}

	cfg_tab_reorder_t *reorder_cfg_tab = (cfg_tab_reorder_t *)(uintptr_t)va;

	if (cpe_mem_get(MEM_ID_CVDR_CFG_TAB_REORDER, &va, &da) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_REORDER);
		return CPE_FW_ERR;
	}

	cfg_tab_cvdr_t *reorder_cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)va;

	if (cpe_mem_get(MEM_ID_CMDLST_PARA_REORDER, &va, &da) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_REORDER);
		return CPE_FW_ERR;
	}

	cmdlst_para_t *reorder_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	reorder_update_request_cfg_tab(reorder_request, reorder_cfg_tab);
	reorder_update_cvdr_cfg_tab(reorder_request, reorder_cvdr_cfg_tab);
	reorder_cmdlst_para->channel_id = RDR_CHANNEL;
	reorder_cmdlst_para->stripe_cnt = 1;
	reorder_set_cmdlst_stripe(&reorder_cmdlst_para->cmd_stripe_info[0], reorder_cmdlst_para->stripe_cnt);
	df_sched_prepare(reorder_cmdlst_para);
	schedule_cmdlst_link_t *cmd_link_entry = reorder_cmdlst_para->cmd_entry;
	reorder_set_irq_mode(reorder_cmdlst_para, reorder_cmdlst_para->stripe_cnt);
	df_sched_set_buffer_header(reorder_cmdlst_para);

	for (i = 0; i < reorder_cmdlst_para->stripe_cnt; i++) {
		cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, reorder_cvdr_cfg_tab);
		reorder_prepare_cmd(&g_reorder_devs[0], &cmd_link_entry[i].cmd_buf, reorder_cfg_tab);
		cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, reorder_cvdr_cfg_tab);
		cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
	}

	ipp_update_cmdlst_cfg_tab(reorder_cmdlst_para);
	loge_if_ret(df_sched_start(reorder_cmdlst_para));
	cpe_mem_free(MEM_ID_REORDER_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_REORDER);
	d("-\n");
	return CPE_FW_OK;
}

static int reorder_request_dump(msg_req_reorder_request_t *req)
{
	unsigned int i = 0;
	i("size_of_reorder_req = %d\n", (int)(sizeof(msg_req_reorder_request_t)));
	i("frame_number = %d\n", req->frame_number);

	for (i = 0; i < RDR_STREAM_MAX; i++) {
		d("streams[%d].width = %d\n",  i, req->streams[i].width);
		d("streams[%d].height = %d\n", i, req->streams[i].height);
		d("streams[%d].stride = %d\n", i, req->streams[i].stride);
		d("streams[%d].buf = 0x%x\n",  i, req->streams[i].buf);
		d("streams[%d].format = %d\n", i, req->streams[i].format);
	}

	d("req->reorder_ctrl_cfg.reorder_en         = %d\n", req->reg_cfg.reorder_ctrl_cfg.reorder_en);
	d("req->reorder_ctrl_cfg.descriptor_type    = %d\n", req->reg_cfg.reorder_ctrl_cfg.descriptor_type);
	d("req->reorder_ctrl_cfg.total_kpt          = %d\n", req->reg_cfg.reorder_ctrl_cfg.total_kpt);
	d("req->reorder_block_cfg.blk_v_num         = %d\n", req->reg_cfg.reorder_block_cfg.blk_v_num);
	d("req->reorder_block_cfg.blk_h_num         = %d\n", req->reg_cfg.reorder_block_cfg.blk_h_num);
	d("req->reorder_block_cfg.blk_num           = %d\n", req->reg_cfg.reorder_block_cfg.blk_num);
	d("req->reorder_prefetch_cfg.prefetch_en    = %d\n", req->reg_cfg.reorder_prefetch_cfg.prefetch_enable);
	d("req->reorder_prefetch_cfg.first_32k_page = %d\n", req->reg_cfg.reorder_prefetch_cfg.first_32k_page);
	d("req->reg_cfg.reorder_kptnum_cfg.reorder_kpt_num = \n");

	for (i = 0; i < 94 / 4 - 1; i++)
		d("0x%08x  0x%08x  0x%08x  0x%08x\n", req->reg_cfg.reorder_kpt_num[0 + 4 * i],
		  req->reg_cfg.reorder_kpt_num[1 + 4 * i],
		  req->reg_cfg.reorder_kpt_num[2 + 4 * i],
		  req->reg_cfg.reorder_kpt_num[3 + 4 * i]);

	return 0;
}

