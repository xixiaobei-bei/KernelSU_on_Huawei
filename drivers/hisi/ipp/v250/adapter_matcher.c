#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/delay.h>

#include "memory.h"
#include "matcher.h"
#include "cfg_table_matcher.h"

#include "cvdr_drv.h"
#include "cvdr_opt.h"

#include "reorder_drv.h"
#include "compare_drv.h"

#include "compare_reg_offset.h"
#include "sub_ctrl_reg_offset.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define LOG_TAG LOG_MODULE_MATCHER
#define CVDR_KEY_POINT_OVERFLOW  0x8000

extern reorder_dev_t g_reorder_devs[];
extern compare_dev_t g_compare_devs[];

static int matcher_request_dump(msg_req_matcher_request_t *req);

static int matcher_reorder_update_cvdr_cfg_tab(msg_req_matcher_request_t *matcher_request,
		cfg_tab_cvdr_t *reorder_cfg_tab, unsigned int layer_num)
{
	cvdr_opt_fmt_t cfmt;
	unsigned int stride = 0;
	req_rdr_t *req_rdr = &(matcher_request->req_rdr);

	if (req_rdr->streams[layer_num][BI_RDR_FP].buf) {
		cfmt.id = RD_RDR_DESC;
		cfmt.width = req_rdr->streams[layer_num][BI_RDR_FP].width;
		cfmt.full_width = req_rdr->streams[layer_num][BI_RDR_FP].width;
		cfmt.line_size = req_rdr->streams[layer_num][BI_RDR_FP].width * 2;
		cfmt.height = req_rdr->streams[layer_num][BI_RDR_FP].height;
		cfmt.expand = EXP_PIX;
		stride = req_rdr->streams[layer_num][BI_RDR_FP].width;
		cfmt.addr = req_rdr->streams[layer_num][BI_RDR_FP].buf;
		cfg_tbl_cvdr_rd_cfg_d64(reorder_cfg_tab, &cfmt, (ipp_align_up(80 * 2047, CVDR_ALIGN_BYTES)), stride);
	}

	if (req_rdr->streams[layer_num][BO_RDR_FP_BLOCK].buf)
		cfg_tbl_cvdr_nr_wr_cfg(reorder_cfg_tab, WR_RDR_DESC);

	return 0;
}

static void matcher_reorder_update_request_cfg_tab(msg_req_matcher_request_t *matcher_request,
		cfg_tab_reorder_t *reorder_cfg_tab, unsigned int layer_num)
{
	req_rdr_t *req_rdr = &(matcher_request->req_rdr);
	reorder_cfg_tab->reorder_ctrl_cfg.to_use = 1;
	reorder_cfg_tab->reorder_ctrl_cfg.reorder_en = req_rdr->ctrl_cfg[layer_num].reorder_en;
	reorder_cfg_tab->reorder_ctrl_cfg.descriptor_type = req_rdr->ctrl_cfg[layer_num].descriptor_type;
	reorder_cfg_tab->reorder_ctrl_cfg.total_kpt = req_rdr->ctrl_cfg[layer_num].total_kpt;
	reorder_cfg_tab->reorder_block_cfg.to_use = 1;
	reorder_cfg_tab->reorder_block_cfg.blk_v_num = req_rdr->block_cfg[layer_num].blk_v_num;
	reorder_cfg_tab->reorder_block_cfg.blk_h_num = req_rdr->block_cfg[layer_num].blk_h_num;
	reorder_cfg_tab->reorder_block_cfg.blk_num   = req_rdr->block_cfg[layer_num].blk_num;
	reorder_cfg_tab->reorder_prefetch_cfg.to_use = 1;
	reorder_cfg_tab->reorder_prefetch_cfg.prefetch_enable = 1;
	reorder_cfg_tab->reorder_prefetch_cfg.first_32k_page = req_rdr->streams[layer_num][BO_RDR_FP_BLOCK].buf >> 15;
	reorder_cfg_tab->reorder_kptnum_cfg.to_use = 1;
	loge_if(memcpy_s(&(reorder_cfg_tab->reorder_kptnum_cfg.reorder_kpt_num[0]), MATCHER_KPT_NUM * sizeof(unsigned int),
					 &(req_rdr->reorder_kpt_num[layer_num][0]), MATCHER_KPT_NUM * sizeof(unsigned int)));
}

static int seg_src_cfg_rdr_cmdlst(msg_req_matcher_request_t *matcher_request, cmdlst_para_t *rdr_cmdlst_para)
{
	unsigned int i = 0;
	unsigned int stripe_cnt = matcher_request->rdr_pyramid_layer;
	unsigned int rdr_layer_stripe_num = 1;
	unsigned long long irq = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = rdr_cmdlst_para->cmd_stripe_info;
	stripe_cnt = stripe_cnt * rdr_layer_stripe_num;
	rdr_cmdlst_para->stripe_cnt = stripe_cnt;

	for (i = 0; i < stripe_cnt; i++) {
		irq = 0;
		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_RDR;
		cmdlst_stripe[i].en_link         = 0;
		cmdlst_stripe[i].ch_link         = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;
		cmdlst_stripe[i].is_need_set_sop = 0;
		irq = irq | (((unsigned long long)(1)) << IPP_RDR_CVDR_VP_RD_EOF_CMDSLT);
		irq = irq | (((unsigned long long)(1)) << IPP_RDR_IRQ_DONE);
		irq = irq | (((unsigned long long)(1)) << IPP_RDR_CVDR_VP_RD_EOF_FP);
		cmdlst_stripe[i].hw_priority     = CMD_PRIO_LOW;
		cmdlst_stripe[i].irq_mode        = (~irq) & (0x1F);

		if ((matcher_request->work_mode & (1 << 1)) == 0) {
			cmdlst_stripe[i].en_link = 1;

			if (i > 0) {
				cmdlst_stripe[i].ch_link = CMP_CHANNEL;
				cmdlst_stripe[i].ch_link_act_nbr = STRIPE_NUM_EACH_CMP;
			}
		} else {
			cmdlst_stripe[i].en_link = 1;
			cmdlst_stripe[i].ch_link = CMP_CHANNEL;
			cmdlst_stripe[i].ch_link_act_nbr = matcher_request->cmp_pyramid_layer * STRIPE_NUM_EACH_CMP;
		}
	}

	cmdlst_stripe[stripe_cnt - 1].is_last_stripe  = 1;
	return CPE_FW_OK;
}

static int matcher_compare_update_cvdr_cfg_tab(msg_req_matcher_request_t *matcher_request,
		cfg_tab_cvdr_t *compare_cfg_tab, unsigned int layer_num)
{
	cvdr_opt_fmt_t cfmt;
	unsigned int stride = 0;
	req_cmp_t *req_cmp = &(matcher_request->req_cmp);

	if (req_cmp->streams[layer_num][BI_CMP_REF_FP].buf) {
		cfmt.id = RD_CMP_REF;
		cfmt.width = req_cmp->streams[layer_num][BI_CMP_REF_FP].width;
		cfmt.full_width = req_cmp->streams[layer_num][BI_CMP_REF_FP].width;
		cfmt.line_size = req_cmp->streams[layer_num][BI_CMP_REF_FP].width * 2;
		cfmt.height = req_cmp->streams[layer_num][BI_CMP_REF_FP].height;
		cfmt.expand = 1;
		stride   = cfmt.width;
		cfmt.addr = req_cmp->streams[layer_num][BI_CMP_REF_FP].buf;
		cfg_tbl_cvdr_rd_cfg_d64(compare_cfg_tab, &cfmt, (ipp_align_up(CMP_IN_INDEX_NUM, CVDR_ALIGN_BYTES)), stride);
	}

	if (req_cmp->streams[layer_num][BI_CMP_CUR_FP].buf)
		cfg_tbl_cvdr_nr_rd_cfg(compare_cfg_tab, RD_CMP_CUR);

	return 0;
}

static void matcher_compare_update_request_cfg_tab(msg_req_matcher_request_t *matcher_request,
		cfg_tab_compare_t *compare_cfg_tab, unsigned int layer_num)
{
	unsigned int i = 0;
	req_cmp_t *req_cmp = &(matcher_request->req_cmp);
	compare_cfg_tab->compare_ctrl_cfg.to_use   = 1;
	compare_cfg_tab->compare_search_cfg.to_use = 1;
	compare_cfg_tab->compare_stat_cfg.to_use   = 1;
	compare_cfg_tab->compare_block_cfg.to_use  = 1;
	compare_cfg_tab->compare_prefetch_cfg.to_use = 1;
	compare_cfg_tab->compare_kptnum_cfg.to_use = 1;
	compare_cfg_tab->compare_ctrl_cfg.compare_en      = req_cmp->ctrl_cfg[layer_num].compare_en;
	compare_cfg_tab->compare_ctrl_cfg.descriptor_type = req_cmp->ctrl_cfg[layer_num].descriptor_type;
	compare_cfg_tab->compare_block_cfg.blk_v_num = req_cmp->block_cfg[layer_num].blk_v_num;
	compare_cfg_tab->compare_block_cfg.blk_h_num = req_cmp->block_cfg[layer_num].blk_h_num;
	compare_cfg_tab->compare_block_cfg.blk_num   = req_cmp->block_cfg[layer_num].blk_num;
	compare_cfg_tab->compare_search_cfg.v_radius = req_cmp->search_cfg[layer_num].v_radius;
	compare_cfg_tab->compare_search_cfg.h_radius = req_cmp->search_cfg[layer_num].h_radius;
	compare_cfg_tab->compare_search_cfg.dis_ratio = req_cmp->search_cfg[layer_num].dis_ratio;
	compare_cfg_tab->compare_search_cfg.dis_threshold = req_cmp->search_cfg[layer_num].dis_threshold;
	compare_cfg_tab->compare_stat_cfg.stat_en    = req_cmp->stat_cfg[layer_num].stat_en;
	compare_cfg_tab->compare_stat_cfg.max3_ratio = req_cmp->stat_cfg[layer_num].max3_ratio;
	compare_cfg_tab->compare_stat_cfg.bin_factor = req_cmp->stat_cfg[layer_num].bin_factor;
	compare_cfg_tab->compare_stat_cfg.bin_num    = req_cmp->stat_cfg[layer_num].bin_num;
	compare_cfg_tab->compare_prefetch_cfg.prefetch_enable = 1;
	compare_cfg_tab->compare_prefetch_cfg.first_32k_page = req_cmp->streams[layer_num][BI_CMP_CUR_FP].buf >> 15;

	for (i = 0; i < MATCHER_KPT_NUM; i++) {
		compare_cfg_tab->compare_kptnum_cfg.compare_ref_kpt_num[i] = req_cmp->kptnum_cfg[layer_num].compare_ref_kpt_num[i];
		compare_cfg_tab->compare_kptnum_cfg.compare_cur_kpt_num[i] = req_cmp->kptnum_cfg[layer_num].compare_cur_kpt_num[i];
	}
}

static int seg_src_cfg_cmp_cmdlst(msg_req_matcher_request_t *matcher_request, cmdlst_para_t *cmp_cmdlst_para)
{
	unsigned int i = 0;
	unsigned int stripe_cnt = matcher_request->cmp_pyramid_layer * STRIPE_NUM_EACH_CMP;
	unsigned long long irq = 0;
	cmdlst_stripe_info_t *cmdlst_stripe = cmp_cmdlst_para->cmd_stripe_info;
	cmp_cmdlst_para->stripe_cnt = stripe_cnt;

	for (i = 0; i < stripe_cnt; i++) {
		irq = 0;

		if (i % STRIPE_NUM_EACH_CMP != 0) {
			cmdlst_stripe[i].hw_priority     = CMD_PRIO_LOW;
			irq = irq | (((unsigned long long)(1)) << IPP_CMP_CVDR_VP_RD_EOF_CMDLST);
			irq = irq | (((unsigned long long)(1)) << IPP_CMP_CVDR_VP_WR_EOF_CMDLST);
		} else {
			cmdlst_stripe[i].hw_priority     = CMD_PRIO_LOW;
			irq = irq | (((unsigned long long)(1)) << IPP_CMP_CVDR_VP_RD_EOF_CMDLST);
			irq = irq | (((unsigned long long)(1)) << IPP_CMP_IRQ_DONE);
			irq = irq | (((unsigned long long)(1)) << IPP_CMP_CVDR_VP_RD_EOF_FP);
		}

		cmdlst_stripe[i].resource_share  = 1 << IPP_CMD_RES_SHARE_CMP;
		cmdlst_stripe[i].irq_mode        = (~irq) & (0x1F);
		cmdlst_stripe[i].en_link         = 0;
		cmdlst_stripe[i].ch_link         = 0;
		cmdlst_stripe[i].ch_link_act_nbr = 0;
		cmdlst_stripe[i].is_last_stripe  = 0;
		cmdlst_stripe[i].is_need_set_sop = 0;
	}

	cmdlst_stripe[stripe_cnt - 1].is_last_stripe  = 1;
	return CPE_FW_OK;
}

static int seg_src_set_cmp_cmdlst_para(cmdlst_para_t *cmdlst_para, msg_req_matcher_request_t *matcher_request,
									   cfg_tab_compare_t *compare_cfg_tab, cfg_tab_cvdr_t *compare_cvdr_cfg_tab)
{
	unsigned int i = 0;
	unsigned int cur_layer = 0;
	unsigned int wr_addr_in_cmdlst_buf = 0;
	unsigned int rd_addr_for_cmdlst    = 0;
	schedule_cmdlst_link_t *cmd_link_entry = (schedule_cmdlst_link_t *)cmdlst_para->cmd_entry;
	unsigned int cmp_layer_stripe_num = STRIPE_NUM_EACH_CMP;

	for (i = 0; i < cmdlst_para->stripe_cnt;) {
		cur_layer = i / cmp_layer_stripe_num;
		cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &compare_cvdr_cfg_tab[cur_layer]);
		compare_prepare_cmd(&g_compare_devs[0], &cmd_link_entry[i].cmd_buf, &compare_cfg_tab[cur_layer]);
		cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &compare_cvdr_cfg_tab[cur_layer]);
		cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
		i++;

		if (STRIPE_NUM_EACH_CMP > 1) {
			if (matcher_request->req_cmp.streams[cur_layer][BO_CMP_MATCHED_OUT].buf) {
				wr_addr_in_cmdlst_buf = matcher_request->req_cmp.streams[cur_layer][BO_CMP_MATCHED_OUT].buf;
				rd_addr_for_cmdlst    = JPG_COMPARE_ADDR + COMPARE_COMPARE_CFG_REG;
				loge_if_ret(seg_src_set_cmdlst_wr_buf(&cmd_link_entry[i].cmd_buf, wr_addr_in_cmdlst_buf, rd_addr_for_cmdlst,
													  CPMPARE_REG_NUM));
				cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
				i++;
			}
		}
	}

	return CPE_FW_OK;
}

void cmdlst_channel_cfg_init(unsigned int chan_id, unsigned int token_nbr_en, unsigned int nrt_channel)
{
	unsigned int cmdlst_channel_value = (token_nbr_en << 7) | (nrt_channel << 8);
	hispcpe_reg_set(CMDLIST_REG, 0x80 + chan_id * 0x80, cmdlst_channel_value);
}

static int matcher_request_config_compare(msg_req_matcher_request_t *matcher_request,
								unsigned long long *va, unsigned int *da)
{
	int ret = 0;
	unsigned int i = 0;
	cmdlst_para_t *compare_cmdlst_para = NULL;

	if ((ret = cpe_mem_get(MEM_ID_COMPARE_CFG_TAB, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_COMPARE_CFG_TAB);
		return CPE_FW_ERR;
	}

	cfg_tab_compare_t *compare_cfg_tab = (cfg_tab_compare_t *)(uintptr_t)(*va);

	if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_COMPARE, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_COMPARE);
		return CPE_FW_ERR;
	}

	cfg_tab_cvdr_t *compare_cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)(*va);

	if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_COMPARE, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_COMPARE);
		return CPE_FW_ERR;
	}

	compare_cmdlst_para = (cmdlst_para_t *)(uintptr_t)(*va);
	compare_cmdlst_para->channel_id = CMP_CHANNEL;

	for (i = 0; i < matcher_request->cmp_pyramid_layer; i++) {
		matcher_compare_update_request_cfg_tab(matcher_request, &compare_cfg_tab[i], i);
		matcher_compare_update_cvdr_cfg_tab(matcher_request, &compare_cvdr_cfg_tab[i], i);
	}

	seg_src_cfg_cmp_cmdlst(matcher_request, compare_cmdlst_para);
	df_sched_prepare(compare_cmdlst_para);
	df_sched_set_buffer_header(compare_cmdlst_para);
	seg_src_set_cmp_cmdlst_para(compare_cmdlst_para, matcher_request, compare_cfg_tab, compare_cvdr_cfg_tab);
	ipp_update_cmdlst_cfg_tab(compare_cmdlst_para);
	cpe_mem_free(MEM_ID_COMPARE_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_COMPARE);
	loge_if_ret(df_sched_start(compare_cmdlst_para));
	return CPE_FW_OK;
}

static int matcher_request_config_reorder(msg_req_matcher_request_t *matcher_request,
								unsigned long long *va, unsigned int *da)
{
	int ret = 0;
	unsigned int i = 0;

	if ((ret = cpe_mem_get(MEM_ID_REORDER_CFG_TAB, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_REORDER_CFG_TAB);
		return CPE_FW_ERR;
	}

	cfg_tab_reorder_t *reorder_cfg_tab = (cfg_tab_reorder_t *)(uintptr_t)(*va);

	if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_REORDER, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_REORDER);
		return CPE_FW_ERR;
	}

	cfg_tab_cvdr_t *reorder_cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)(*va);

	if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_REORDER, va, da)) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_REORDER);
		return CPE_FW_ERR;
	}

	cmdlst_para_t *reorder_cmdlst_para = (cmdlst_para_t *)(uintptr_t)(*va);
	reorder_cmdlst_para->channel_id = RDR_CHANNEL;
	reorder_cmdlst_para->stripe_cnt = matcher_request->rdr_pyramid_layer;
	seg_src_cfg_rdr_cmdlst(matcher_request, reorder_cmdlst_para);

	for (i = 0; i < reorder_cmdlst_para->stripe_cnt; i++) {
		matcher_reorder_update_request_cfg_tab(matcher_request, &reorder_cfg_tab[i], i);
		matcher_reorder_update_cvdr_cfg_tab(matcher_request, &reorder_cvdr_cfg_tab[i], i);
	}

	df_sched_prepare(reorder_cmdlst_para);
	schedule_cmdlst_link_t *rdr_cmd_link_entry = (schedule_cmdlst_link_t *)reorder_cmdlst_para->cmd_entry;
	df_sched_set_buffer_header(reorder_cmdlst_para);

	for (i = 0; i < reorder_cmdlst_para->stripe_cnt; i++) {
		cvdr_prepare_nr_cmd(&g_cvdr_devs[0], &rdr_cmd_link_entry[i].cmd_buf, &reorder_cvdr_cfg_tab[i]);
		reorder_prepare_cmd(&g_reorder_devs[0], &rdr_cmd_link_entry[i].cmd_buf, &reorder_cfg_tab[i]);
		cvdr_prepare_cmd(&g_cvdr_devs[0], &rdr_cmd_link_entry[i].cmd_buf, &reorder_cvdr_cfg_tab[i]);
		cmdlst_set_buffer_padding(&rdr_cmd_link_entry[i].cmd_buf);
	}

	ipp_update_cmdlst_cfg_tab(reorder_cmdlst_para);
	cpe_mem_free(MEM_ID_REORDER_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_REORDER);
	loge_if_ret(df_sched_start(reorder_cmdlst_para));
	return CPE_FW_OK;
}

int matcher_request_handler(msg_req_matcher_request_t *matcher_request)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	unsigned int i = 0;
	d("+\n");

	if (matcher_request == NULL) {
		e("[%s] failed : matcher_request is null", __func__);
		return -1;
	}

	matcher_request_dump(matcher_request);

	for (i = 0; i < matcher_request->rdr_pyramid_layer; i++) {
		if (matcher_request->req_rdr.ctrl_cfg[i].total_kpt == 0) {
			i("Check rdr[%d].total_kpt is 0", i);
			return 1;
		}
	}

	unsigned int rdr_token_nbr_en = 0;
	unsigned int cmp_token_nbr_en = (unsigned int)(((matcher_request->rdr_pyramid_layer != 0)
									&& (matcher_request->cmp_pyramid_layer != 0)) ? (1) : (0));
	unsigned int nrt_channel  = 1;
	cmdlst_channel_cfg_init(RDR_CHANNEL, rdr_token_nbr_en, nrt_channel);
	cmdlst_channel_cfg_init(CMP_CHANNEL, cmp_token_nbr_en, nrt_channel);

	if (matcher_request->cmp_pyramid_layer != 0) {
		if (matcher_request_config_compare(matcher_request, &va, &da) != CPE_FW_OK)
			return CPE_FW_ERR;
	}

	if (matcher_request->rdr_pyramid_layer != 0) {
		if (matcher_request_config_reorder(matcher_request, &va, &da) != CPE_FW_OK)
			return CPE_FW_ERR;
	}

	d("-\n");
	return CPE_FW_OK;
}

int matcher_eof_handler(msg_req_matcher_request_t *matcher_request)
{
	if (matcher_request->rdr_pyramid_layer != 0)
		loge_if_ret(ipp_eop_handler(CMD_EOF_REORDER_MODE));

	if (matcher_request->cmp_pyramid_layer != 0)
		loge_if_ret(ipp_eop_handler(CMD_EOF_COMPARE_MODE));

	return CPE_FW_OK;
}

#if FLAG_LOG_DEBUG
static int matcher_request_dump_req_rdr(msg_req_matcher_request_t *req)
{
	unsigned int i = 0;
	unsigned int j = 0;

	for (i = 0; i < req->rdr_pyramid_layer; i++) {
		for (j = 0; j < RDR_STREAM_MAX; j++) {
			d(" req->req_rdr.streams[%d][%d].width  = %d\n", i, j, req->req_rdr.streams[i][j].width);
			d(" req->req_rdr.streams[%d][%d].height = %d\n", i, j, req->req_rdr.streams[i][j].height);
			d(" req->req_rdr.streams[%d][%d].stride = %d\n", i, j, req->req_rdr.streams[i][j].stride);
			d(" req->req_rdr.streams[%d][%d].buf    = 0x%08x\n", i, j, req->req_rdr.streams[i][j].buf);
			d(" req->req_rdr.streams[%d][%d].format = %d\n", i, j, req->req_rdr.streams[i][j].format);
		}

		d(" req->req_rdr.ctrl_cfg[%d].reorder_en      = %d\n", i, req->req_rdr.ctrl_cfg[i].reorder_en);
		d(" req->req_rdr.ctrl_cfg[%d].descriptor_type = %d\n", i, req->req_rdr.ctrl_cfg[i].descriptor_type);
		d(" req->req_rdr.ctrl_cfg[%d].total_kpt       = %d\n", i, req->req_rdr.ctrl_cfg[i].total_kpt);
		d(" req->req_rdr.block_cfg[%d].blk_v_num = %d\n", i, req->req_rdr.block_cfg[i].blk_v_num);
		d(" req->req_rdr.block_cfg[%d].blk_h_num = %d\n", i, req->req_rdr.block_cfg[i].blk_h_num);
		d(" req->req_rdr.block_cfg[%d].blk_num   = %d\n", i, req->req_rdr.block_cfg[i].blk_num);

		for (j = 0; j < 5; j++)
			d(" req->req_rdr.reorder_kpt_num[%d][%d] = 0x%08x\n", i, j, req->req_rdr.reorder_kpt_num[i][j]);
	}

	return CPE_FW_OK;
}
#endif

static int matcher_request_dump(msg_req_matcher_request_t *req)
{
#if FLAG_LOG_DEBUG
	unsigned int i = 0;
	unsigned int j = 0;
	d("size_of_matcher_req = %d\n", (int)(sizeof(msg_req_matcher_request_t)));
	d("req->frame_number      = %d\n", req->frame_number);
	i("req->rdr_pyramid_layer = %d\n", req->rdr_pyramid_layer);
	d("req->cmp_pyramid_layer = %d\n", req->cmp_pyramid_layer);
	d("req->work_mode         = %d\n", req->work_mode);
	matcher_request_dump_req_rdr(req);

	for (i = 0; i < req->cmp_pyramid_layer; i++) {
		for (j = 0; j < CMP_STREAM_MAX; j++) {
			d(" req->req_cmp.streams[%d][%d].width  = %d\n", i, j, req->req_cmp.streams[i][j].width);
			d(" req->req_cmp.streams[%d][%d].height = %d\n", i, j, req->req_cmp.streams[i][j].height);
			d(" req->req_cmp.streams[%d][%d].stride = %d\n", i, j, req->req_cmp.streams[i][j].stride);
			d(" req->req_cmp.streams[%d][%d].buf    = 0x%08x\n", i, j, req->req_cmp.streams[i][j].buf);
			d(" req->req_cmp.streams[%d][%d].format = %d\n", i, j, req->req_cmp.streams[i][j].format);
		}

		d(" req->req_cmp.ctrl_cfg[%d].compare_en      = %d\n", i, req->req_cmp.ctrl_cfg[i].compare_en);
		d(" req->req_cmp.ctrl_cfg[%d].descriptor_type = %d\n", i, req->req_cmp.ctrl_cfg[i].descriptor_type);
		d(" req->req_cmp.block_cfg[%d].blk_v_num    = %d\n", i, req->req_cmp.block_cfg[i].blk_v_num);
		d(" req->req_cmp.block_cfg[%d].blk_h_num    = %d\n", i, req->req_cmp.block_cfg[i].blk_h_num);
		d(" req->req_cmp.block_cfg[%d].blk_num      = %d\n", i, req->req_cmp.block_cfg[i].blk_num);
		d(" req->req_cmp.search_cfg[%d].v_radius      = %d\n", i, req->req_cmp.search_cfg[i].v_radius);
		d(" req->req_cmp.search_cfg[%d].h_radius      = %d\n", i, req->req_cmp.search_cfg[i].h_radius);
		d(" req->req_cmp.search_cfg[%d].dis_ratio     = %d\n", i, req->req_cmp.search_cfg[i].dis_ratio);
		d(" req->req_cmp.search_cfg[%d].dis_threshold = %d\n", i, req->req_cmp.search_cfg[i].dis_threshold);
		d(" req->req_cmp.stat_cfg[%d].stat_en      = %d\n", i, req->req_cmp.stat_cfg[i].stat_en);
		d(" req->req_cmp.stat_cfg[%d].max3_ratio   = %d\n", i, req->req_cmp.stat_cfg[i].max3_ratio);
		d(" req->req_cmp.stat_cfg[%d].bin_factor   = %d\n", i, req->req_cmp.stat_cfg[i].bin_factor);
		d(" req->req_cmp.stat_cfg[%d].bin_num      = %d\n", i, req->req_cmp.stat_cfg[i].bin_num);

		for (j = 0; j < 5; j++)
			d(" req->req_cmp.compare_ref_kpt_num[%d][%d] = 0x%08x\n", i, j, req->req_cmp.kptnum_cfg[i].compare_ref_kpt_num[j]);

		for (j = 0; j < 5; j++)
			d(" req->req_cmp.compare_cur_kpt_num[%d][%d] = 0x%08x\n", i, j, req->req_cmp.kptnum_cfg[i].compare_cur_kpt_num[j]);
	}

	d(" req->matcher_rate_value      = %d\n", req->matcher_rate_value);
#else
	i("req->rdr_pyramid_layer = %d\n", req->rdr_pyramid_layer);
#endif
	return 0;
}

