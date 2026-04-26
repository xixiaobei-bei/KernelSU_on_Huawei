/*lint -e679
-esym(679,*)*/

#include <linux/string.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "compare.h"
#include "cfg_table_compare.h"
#include "cvdr_drv.h"
#include "compare_drv.h"
#include "config_table_cvdr.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "compare_drv_priv.h"
#include "sub_ctrl_reg_offset.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define LOG_TAG LOG_MODULE_COMPARE
#define CVDR_KEY_POINT_OVERFLOW  0x8000

extern compare_dev_t g_compare_devs[];
unsigned int               g_compare_read_st = 1;
static unsigned long long  g_compare_index_addr = 0;

static int compare_request_dump(msg_req_compare_request_t *req);

static int compare_set_irq_mode(cmdlst_para_t *cmdlst_para, unsigned int stripe_cnt)
{
	unsigned int i = 0;

	for (i = 0; i < stripe_cnt; i++) {
		if (g_compare_read_st && (i % 2 != 0))
			cmdlst_para->cmd_stripe_info[i].irq_mode = 0x13;
		else
			cmdlst_para->cmd_stripe_info[i].irq_mode = 0x14;
	}

	return 0;
}

static int compare_update_cvdr_cfg_tab(msg_req_compare_request_t *compare_request, cfg_tab_cvdr_t *compare_cfg_tab)
{
	cvdr_opt_fmt_t cfmt;
	loge_if(memset_s(&cfmt, sizeof(cvdr_opt_fmt_t), 0, sizeof(cvdr_opt_fmt_t)));
	unsigned int stride = 0;

	if (compare_request->streams[BI_COMPARE_DESC_REF].buf) {
		cfmt.id = RD_CMP_REF;
		cfmt.width = compare_request->streams[BI_COMPARE_DESC_REF].width;
		cfmt.full_width = compare_request->streams[BI_COMPARE_DESC_REF].width;
		cfmt.line_size = compare_request->streams[BI_COMPARE_DESC_REF].width * 2;
		cfmt.height = compare_request->streams[BI_COMPARE_DESC_REF].height;
		cfmt.expand = 1;
		stride   = cfmt.width;
		cfmt.addr = compare_request->streams[BI_COMPARE_DESC_REF].buf;
		loge_if_ret(cfg_tbl_cvdr_rd_cfg_d64(compare_cfg_tab, &cfmt, (ipp_align_up(CMP_IN_INDEX_NUM, CVDR_ALIGN_BYTES)),
											stride));
	}

	if (compare_request->streams[BI_COMPARE_DESC_CUR].buf)
		loge_if_ret(cfg_tbl_cvdr_nr_rd_cfg(compare_cfg_tab, RD_CMP_CUR));

	return 0;
}

static void compare_update_request_cfg_tab(msg_req_compare_request_t *compare_request,
		cfg_tab_compare_t *compare_cfg_tab)
{
	unsigned int i = 0;
	compare_cfg_tab->compare_ctrl_cfg.to_use = 1;
	compare_cfg_tab->compare_search_cfg.to_use = 1;
	compare_cfg_tab->compare_stat_cfg.to_use = 1;
	compare_cfg_tab->compare_block_cfg.to_use = 1;
	compare_cfg_tab->compare_prefetch_cfg.to_use = 1;
	compare_cfg_tab->compare_kptnum_cfg.to_use = 1;
	compare_cfg_tab->compare_ctrl_cfg.compare_en = compare_request->reg_cfg.compare_ctrl_cfg.compare_en;
	compare_cfg_tab->compare_ctrl_cfg.descriptor_type = compare_request->reg_cfg.compare_ctrl_cfg.descriptor_type;
	compare_cfg_tab->compare_block_cfg.blk_v_num = compare_request->reg_cfg.compare_block_cfg.blk_v_num;
	compare_cfg_tab->compare_block_cfg.blk_h_num = compare_request->reg_cfg.compare_block_cfg.blk_h_num;
	compare_cfg_tab->compare_block_cfg.blk_num = compare_request->reg_cfg.compare_block_cfg.blk_num;
	compare_cfg_tab->compare_search_cfg.v_radius = compare_request->reg_cfg.compare_search_cfg.v_radius;
	compare_cfg_tab->compare_search_cfg.h_radius = compare_request->reg_cfg.compare_search_cfg.h_radius;
	compare_cfg_tab->compare_search_cfg.dis_ratio = compare_request->reg_cfg.compare_search_cfg.dis_ratio;
	compare_cfg_tab->compare_search_cfg.dis_threshold = compare_request->reg_cfg.compare_search_cfg.dis_threshold;
	compare_cfg_tab->compare_stat_cfg.stat_en = compare_request->reg_cfg.compare_stat_cfg.stat_en;
	compare_cfg_tab->compare_stat_cfg.max3_ratio = compare_request->reg_cfg.compare_stat_cfg.max3_ratio;
	compare_cfg_tab->compare_stat_cfg.bin_factor = compare_request->reg_cfg.compare_stat_cfg.bin_factor;
	compare_cfg_tab->compare_stat_cfg.bin_num = compare_request->reg_cfg.compare_stat_cfg.bin_num;
	compare_cfg_tab->compare_prefetch_cfg.prefetch_enable = 1;
	compare_cfg_tab->compare_prefetch_cfg.first_32k_page = compare_request->streams[BI_COMPARE_DESC_CUR].buf >> 15;

	for (i = 0; i < MATCHER_KPT_NUM; i++) {
		compare_cfg_tab->compare_kptnum_cfg.compare_ref_kpt_num[i] =
			compare_request->reg_cfg.compare_kptnum_cfg.compare_ref_kpt_num[i];
		compare_cfg_tab->compare_kptnum_cfg.compare_cur_kpt_num[i] =
			compare_request->reg_cfg.compare_kptnum_cfg.compare_cur_kpt_num[i];
	}
}

static void compare_set_cmdlst_stripe(cmdlst_stripe_info_t *stripe, unsigned int stripe_cnt)
{
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int compare_stat_addr = 0;
	unsigned long long va = 0;
	unsigned int da = 0;
	int ret = 0;

	if ((ret = cpe_mem_get(MEM_ID_READ_COMPARE_INDEX, &va, &da)) != 0)
		e("[%s] Failed : cpe_mem_get %d\n", __func__,  MEM_ID_READ_COMPARE_INDEX);

	compare_stat_addr = da;
	g_compare_index_addr = va;

	for (i = 0; i < stripe_cnt; i++) {
		if (g_compare_read_st && i % 2 != 0) {
			stripe[i].rd_cfg_info.fs           = compare_stat_addr;
			stripe[i].rd_cfg_info.rd_cfg_num   = 7;

			for (j = 0; j < stripe[i].rd_cfg_info.rd_cfg_num - 1; j++)
				stripe[i].rd_cfg_info.rd_cfg[j]    = 0xff00E501 + j * 0x400;

			stripe[i].rd_cfg_info.rd_cfg[j]    = 0x3f00E501 + j * 0x400;
		} else {
			stripe[i].rd_cfg_info.fs = 0;
		}

		stripe[i].is_last_stripe   = 0;
	}

	stripe[stripe_cnt - 1].is_last_stripe = 1;
}

static int compare_request_do_config(unsigned int i, cmd_buf_t *cmd_buf, cmdlst_para_t *compare_cmdlst_para,
									 cfg_tab_cvdr_t *compare_cvdr_cfg_tab, cfg_tab_compare_t *compare_cfg_tab)
{
	if (g_compare_read_st) {
		if (i % 2 != 0) {
			cmdlst_read_buffer(i, cmd_buf, compare_cmdlst_para);
		} else {
			cvdr_prepare_nr_cmd(&g_cvdr_devs[0], cmd_buf, compare_cvdr_cfg_tab);
			compare_prepare_cmd(&g_compare_devs[0], cmd_buf, compare_cfg_tab);
			cvdr_prepare_cmd(&g_cvdr_devs[0], cmd_buf, compare_cvdr_cfg_tab);
			cmdlst_set_buffer_padding(cmd_buf);
		}
	} else {
		compare_prepare_cmd(&g_compare_devs[0], cmd_buf, compare_cfg_tab);
		cvdr_prepare_cmd(&g_cvdr_devs[0], cmd_buf, compare_cvdr_cfg_tab);
		cmdlst_set_buffer_padding(cmd_buf);
	}

	return 0;
}

int compare_request_handler(msg_req_compare_request_t *compare_request)
{
	unsigned long long va = 0;
	unsigned int da = 0;
	unsigned int i = 0;
	d("+\n");

	if (compare_request == NULL) {
		e("[%s] failed : compare_request is null", __func__);
		return -1;
	}

	compare_request_dump(compare_request);

	if (cpe_mem_get(MEM_ID_COMPARE_CFG_TAB, &va, &da) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_COMPARE_CFG_TAB);
		return CPE_FW_ERR;
	}

	cfg_tab_compare_t *compare_cfg_tab = (cfg_tab_compare_t *)(uintptr_t)va;

	if (cpe_mem_get(MEM_ID_CVDR_CFG_TAB_COMPARE, &va, &da) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CVDR_CFG_TAB_COMPARE);
		return CPE_FW_ERR;
	}

	cfg_tab_cvdr_t *compare_cvdr_cfg_tab = (cfg_tab_cvdr_t *)(uintptr_t)va;

	if (cpe_mem_get(MEM_ID_CMDLST_PARA_COMPARE, &va, &da) != 0) {
		e("[%s] Failed : cpe_mem_get %d\n", __func__, MEM_ID_CMDLST_PARA_COMPARE);
		return CPE_FW_ERR;
	}

	cmdlst_para_t *compare_cmdlst_para = (cmdlst_para_t *)(uintptr_t)va;
	compare_update_request_cfg_tab(compare_request, compare_cfg_tab);
	compare_update_cvdr_cfg_tab(compare_request, compare_cvdr_cfg_tab);
	compare_cmdlst_para->channel_id = CMP_CHANNEL;
	compare_cmdlst_para->stripe_cnt = 1;

	if (1 == g_compare_read_st)
		compare_cmdlst_para->stripe_cnt = compare_cmdlst_para->stripe_cnt * 2;

	compare_set_cmdlst_stripe(&compare_cmdlst_para->cmd_stripe_info[0], compare_cmdlst_para->stripe_cnt);
	df_sched_prepare(compare_cmdlst_para);
	schedule_cmdlst_link_t *cmd_link_entry = compare_cmdlst_para->cmd_entry;
	compare_set_irq_mode(compare_cmdlst_para, compare_cmdlst_para->stripe_cnt);
	df_sched_set_buffer_header(compare_cmdlst_para);

	for (i = 0; i < compare_cmdlst_para->stripe_cnt; i++)
		compare_request_do_config(i, &cmd_link_entry[i].cmd_buf, compare_cmdlst_para, compare_cvdr_cfg_tab, compare_cfg_tab);

	ipp_update_cmdlst_cfg_tab(compare_cmdlst_para);
	loge_if_ret(df_sched_start(compare_cmdlst_para));
	cpe_mem_free(MEM_ID_COMPARE_CFG_TAB);
	cpe_mem_free(MEM_ID_CVDR_CFG_TAB_COMPARE);
	d("-\n");
	return CPE_FW_OK;
}

static int compare_eof_handler_copy_to_user(unsigned short *index_local, void __user *args_user_index,
									 void __user *args_user_kpt_num,
									 unsigned short *kptnum_local)
{
	unsigned int i            = 0;
	unsigned int temp_value   = 0;
	unsigned int ret          = 0;
	unsigned short *index_tmp = NULL;
	index_tmp                 = index_local;

	for (i = 0; i < CPMPARE_INDEX_NUM; i++) {
		temp_value = (unsigned int)readl((volatile void __iomem *)(uintptr_t)(g_compare_index_addr + i * 4));
		*index_tmp = temp_value;
		index_tmp++;
		*index_tmp = temp_value >> 16;
		index_tmp++;
	}

	ret = copy_to_user(args_user_index, index_local, sizeof(unsigned short) * CPMPARE_INDEX_NUM * 2);

	if (ret != 0) {
		e("[%s] failed : copy_to_user.%d\n", __func__, ret);
		kfree(index_local);
		index_local = NULL;
		kfree(kptnum_local);
		kptnum_local = NULL;
		return -CPE_FW_ERR;
	}

	ret = copy_to_user(args_user_kpt_num, kptnum_local, sizeof(unsigned short) * 2);

	if (ret != 0) {
		e("[%s] failed : copy_to_user.%d\n", __func__, ret);
		kfree(index_local);
		index_local = NULL;
		kfree(kptnum_local);
		kptnum_local = NULL;
		return -CPE_FW_ERR;
	}

	return CPE_FW_OK;
}

int compare_eof_handler(msg_req_compare_request_t *compare_request, unsigned int matched_kpt_num)
{
	if (NULL == compare_request->compare_index) {
		e("[%s] failed : user_index  is null", __func__);
		return -CPE_FW_ERR;
	}

	void __user *args_user_index = (void __user *)(uintptr_t)(compare_request->compare_index);

	if (args_user_index == NULL) {
		pr_err("[%s] args_user_index.%pK\n", __func__, args_user_index);
		return -CPE_FW_ERR;
	}

	if (NULL == compare_request->compare_matched_kpt) {
		e("[%s] failed : user_kpt_num  is null", __func__);
		return -CPE_FW_ERR;
	}

	void __user *args_user_kpt_num = (void __user *)(uintptr_t)(compare_request->compare_matched_kpt);

	if (args_user_kpt_num == NULL) {
		pr_err("[%s] args_user_kpt_num.%pK\n", __func__, args_user_kpt_num);
		return -CPE_FW_ERR;
	}

	unsigned short   *index_local = NULL;
	index_local = (unsigned short *)kmalloc(sizeof(unsigned short) * CPMPARE_INDEX_NUM * 2, GFP_KERNEL);

	if (NULL == index_local) {
		e("[%s] failed : kmalloc index_local  is null", __func__);
		return -1;
	}

	loge_if(memset_s(index_local, sizeof(unsigned short) * CPMPARE_INDEX_NUM * 2, 0,
					 sizeof(unsigned short) * CPMPARE_INDEX_NUM * 2));
	unsigned short   *kptnum_local = NULL;
	kptnum_local = (unsigned short *)kmalloc(sizeof(unsigned short) * 2, GFP_KERNEL);

	if (NULL == kptnum_local) {
		e("[%s] failed : kmalloc kptnum_local  is null", __func__);
		kfree(index_local);
		index_local = NULL;
		return -1;
	}

	loge_if(memset_s(kptnum_local, sizeof(unsigned short) * 2, 0, sizeof(unsigned short) * 2));
	*kptnum_local = (unsigned short)matched_kpt_num;

	if (compare_eof_handler_copy_to_user(index_local, args_user_index, args_user_kpt_num, kptnum_local) != CPE_FW_OK)
		return -CPE_FW_ERR;

	kfree(index_local);
	index_local = NULL;
	kfree(kptnum_local);
	kptnum_local = NULL;
	g_compare_index_addr = 0;
	ipp_eop_handler(CMD_EOF_COMPARE_MODE);
	cpe_mem_free(MEM_ID_READ_COMPARE_INDEX);
	return CPE_FW_OK;
}

static int compare_request_dump(msg_req_compare_request_t *req)
{
	unsigned int i = 0;
	i("size_of_compare_req = %d\n", (int)(sizeof(msg_req_compare_request_t)));
	i("frame_number = %d\n", req->frame_number);

	for (i = 0; i < COMPARE_STREAM_MAX; i++) {
		d("streams[%d].width = %d\n",  i, req->streams[i].width);
		d("streams[%d].height = %d\n", i, req->streams[i].height);
		d("streams[%d].stride = %d\n", i, req->streams[i].stride);
		d("streams[%d].buf = 0x%x\n",  i, req->streams[i].buf);
		d("streams[%d].format = %d\n", i, req->streams[i].format);
	}

	d("req->reg_cfg.compare_ctrl_cfg.compare_en          = %d\n", req->reg_cfg.compare_ctrl_cfg.compare_en);
	d("req->reg_cfg.compare_ctrl_cfg.descriptor_type     = %d\n", req->reg_cfg.compare_ctrl_cfg.descriptor_type);
	d("req->reg_cfg.compare_block_cfg.blk_v_num          = %d\n", req->reg_cfg.compare_block_cfg.blk_v_num);
	d("req->reg_cfg.compare_block_cfg.blk_h_num          = %d\n", req->reg_cfg.compare_block_cfg.blk_h_num);
	d("req->reg_cfg.compare_block_cfg.blk_num            = %d\n", req->reg_cfg.compare_block_cfg.blk_num);
	d("req->reg_cfg.compare_search_cfg.v_radius          = %d\n", req->reg_cfg.compare_search_cfg.v_radius);
	d("req->reg_cfg.compare_search_cfg.h_radius          = %d\n", req->reg_cfg.compare_search_cfg.h_radius);
	d("req->reg_cfg.compare_search_cfg.dis_ratio         = %d\n", req->reg_cfg.compare_search_cfg.dis_ratio);
	d("req->reg_cfg.compare_search_cfg.dis_threshold     = %d\n", req->reg_cfg.compare_search_cfg.dis_threshold);
	d("req->reg_cfg.compare_stat_cfg.stat_en             = %d\n", req->reg_cfg.compare_stat_cfg.stat_en);
	d("req->reg_cfg.compare_stat_cfg.max3_ratio          = %d\n", req->reg_cfg.compare_stat_cfg.max3_ratio);
	d("req->reg_cfg.compare_stat_cfg.bin_factor          = %d\n", req->reg_cfg.compare_stat_cfg.bin_factor);
	d("req->reg_cfg.compare_stat_cfg.bin_num             = %d\n", req->reg_cfg.compare_stat_cfg.bin_num);
	d("req->reg_cfg.compare_prefetch_cfg.prefetch_enable = %d\n", req->reg_cfg.compare_prefetch_cfg.prefetch_enable);
	d("req->reg_cfg.compare_prefetch_cfg.first_32k_page  = %d\n", req->reg_cfg.compare_prefetch_cfg.first_32k_page);
	d("req->reg_cfg.compare_kptnum_cfg.compare_ref_kpt_num = \n");

	for (i = 0; i < 94 / 4 - 1; i++)
		d("0x%08x  0x%08x  0x%08x  0x%08x\n", req->reg_cfg.compare_kptnum_cfg.compare_ref_kpt_num[0 + 4 * i],
		  req->reg_cfg.compare_kptnum_cfg.compare_ref_kpt_num[1 + 4 * i],
		  req->reg_cfg.compare_kptnum_cfg.compare_ref_kpt_num[2 + 4 * i],
		  req->reg_cfg.compare_kptnum_cfg.compare_ref_kpt_num[3 + 4 * i]);

	d("req->reg_cfg.compare_kptnum_cfg.compare_cur_kpt_num = \n");

	for (i = 0; i < 94 / 4 - 1; i++)
		d("0x%08x  0x%08x  0x%08x  0x%08x\n", req->reg_cfg.compare_kptnum_cfg.compare_cur_kpt_num[0 + 4 * i],
		  req->reg_cfg.compare_kptnum_cfg.compare_cur_kpt_num[1 + 4 * i],
		  req->reg_cfg.compare_kptnum_cfg.compare_cur_kpt_num[2 + 4 * i],
		  req->reg_cfg.compare_kptnum_cfg.compare_cur_kpt_num[3 + 4 * i]);

	return 0;
}

#pragma GCC diagnostic pop
