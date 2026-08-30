#include <linux/string.h>
#include <linux/printk.h>
#include "slamtest.h"
#include "cfg_table_slam.h"
#include "cvdr_drv.h"
#include "slam_drv.h"
#include "config_table_cvdr.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "slam_drv_priv.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define LOG_TAG LOG_MODULE_SLAM  

extern slam_dev_t g_slam_devs[];

static int slamtest_request_dump(msg_req_slamtest_request_t* req);

static int slamtest_set_irq_mode(msg_req_slamtest_request_t* req, schedule_cmdlst_link_t* cmd_link_entry,unsigned int stripe_cnt)
{
    cpe_cmdlst_irq_mode module_irq_mode = CMD_IRQ_MAX;
    unsigned int i = 0;
    if(req->basic_cfg.scoremap_en == 1)
    {
        module_irq_mode = CMD_IRQ_SLAM_SCOREMAP_MODE;
    }
    else if(req->basic_cfg.pyramid_en == 1)
    {
        module_irq_mode = CMD_IRQ_SLAM_PYRAMID_MODE;
    }
    else
    {
        module_irq_mode = CMD_IRQ_SLAM_NORMAL_MODE;
    }

    for(i = 0;i < stripe_cnt; i++)
    {
        cmd_link_entry[i].irq_mode = module_irq_mode;
    }

    return 0;
}

static int slamtest_update_cvdr_cfg_tab(msg_req_slamtest_request_t* slamtest_request, cfg_tab_cvdr_t* slamtest_cfg_tab)
{
    pr_info("[%s] +\n", __func__);
    cvdr_opt_fmt_t cfmt;
    pix_format_e format = PIXEL_FMT_CPE_MAX;
    
    if(slamtest_request->streams[BI_SLAM_Y].buf)
    {
        cfmt.id = RD0_SLAM;
        cfmt.width = slamtest_request->streams[BI_SLAM_Y].width;
		cfmt.full_width = slamtest_request->streams[BI_SLAM_Y].width/2;
        cfmt.line_size = slamtest_request->streams[BI_SLAM_Y].width/2;
        cfmt.height = slamtest_request->streams[BI_SLAM_Y].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_SLAM_IN;
        cfmt.addr = slamtest_request->streams[BI_SLAM_Y].buf;
        dataflow_cvdr_rd_cfg_vp(slamtest_cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

     if(slamtest_request->streams[BO_SLAM_Y].buf)
    {
        cfmt.id = WR0_SLAM;
        cfmt.width = slamtest_request->streams[BO_SLAM_Y].width;
		cfmt.full_width = slamtest_request->streams[BO_SLAM_Y].width/2;
        cfmt.line_size = slamtest_request->streams[BO_SLAM_Y].width/2;
        cfmt.height = slamtest_request->streams[BO_SLAM_Y].height;
        cfmt.addr = slamtest_request->streams[BO_SLAM_Y].buf;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_SLAM_OUT;
        dataflow_cvdr_wr_cfg_vp(slamtest_cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

     if(slamtest_request->streams[BO_SLAM_STAT].buf)
    {
        cfmt.id = WR1_SLAM;
        cfmt.width =slamtest_request->streams[BO_SLAM_STAT].width;
		cfmt.full_width =slamtest_request->streams[BO_SLAM_STAT].width;
        cfmt.line_size = slamtest_request->streams[BO_SLAM_STAT].width;
        cfmt.height = slamtest_request->streams[BO_SLAM_STAT].height;
        cfmt.addr = slamtest_request->streams[BO_SLAM_STAT].buf;
        format = PIXEL_FMT_CPE_SLAM_STAT;
		cfmt.expand = 0;
        dataflow_cvdr_wr_cfg_vp(slamtest_cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(slamtest_request->streams[BO_SLAM_SCORE_Y].buf)
    {
        cfmt.id = WR2_SLAM;
        cfmt.width = slamtest_request->streams[BO_SLAM_SCORE_Y].width;
		cfmt.full_width = slamtest_request->streams[BO_SLAM_SCORE_Y].width/2;
        cfmt.line_size = slamtest_request->streams[BO_SLAM_SCORE_Y].width/2;
        cfmt.height = slamtest_request->streams[BO_SLAM_SCORE_Y].height;
        cfmt.addr = slamtest_request->streams[BO_SLAM_SCORE_Y].buf; 
		cfmt.expand = 0;
        format = PIXEL_FMT_CPE_SLAM_SCORE;
        dataflow_cvdr_wr_cfg_vp(slamtest_cfg_tab, &cfmt, 0, CPE_CLK, format);
    }
    pr_info("[%s] -\n", __func__);
    return 0;
}

static void slamtest_basic_config(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{
	slamtest_cfg_tab->to_use = 1;
	slamtest_cfg_tab->basic_cfg.to_use   = 1;
	slamtest_cfg_tab->basic_cfg.slam_cfg.to_use  = 1;
	slamtest_cfg_tab->basic_cfg.slam_cfg.brief_en	= slamtest_request->basic_cfg.brief_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.fast_en	= slamtest_request->basic_cfg.fast_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.freak_en	= slamtest_request->basic_cfg.freak_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.gsblur_en = slamtest_request->basic_cfg.gsblur_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.nms_en = slamtest_request->basic_cfg.nms_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.orient_en = slamtest_request->basic_cfg.orient_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.pyramid_en = slamtest_request->basic_cfg.pyramid_en;
	slamtest_cfg_tab->basic_cfg.slam_cfg.scoremap_en = slamtest_request->basic_cfg.scoremap_en;
	slamtest_cfg_tab->basic_cfg.slam_size.width = slamtest_request->streams[BI_SLAM_Y].width;
	slamtest_cfg_tab->basic_cfg.slam_size.height = slamtest_request->streams[BI_SLAM_Y].height;
}

static void slamtest_pyramid_param_config(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{
	if(slamtest_request->basic_cfg.pyramid_en == 1)
	{         
		slamtest_cfg_tab->pyramid_param.to_use = 1;
		slamtest_cfg_tab->pyramid_param.scl_inc = slamtest_request->pyramid_cfg.scl_inc;
		slamtest_cfg_tab->pyramid_param.scl_vbottom = slamtest_request->pyramid_cfg.scl_vbottom;
		slamtest_cfg_tab->pyramid_param.scl_vtop = slamtest_request->pyramid_cfg.scl_vtop;
		slamtest_cfg_tab->pyramid_param.scl_hright = slamtest_request->pyramid_cfg.scl_hright;
		slamtest_cfg_tab->pyramid_param.scl_hleft = slamtest_request->pyramid_cfg.scl_hleft;
	}
	else
	{   
	    slamtest_cfg_tab->pyramid_param.to_use = 0;
    }	
}

static void slamtest_gsblur_coeff_config(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{
	if(slamtest_request->basic_cfg.gsblur_en == 1)
	{         
		slamtest_cfg_tab->gsblur_coeff.to_use = 1;
		slamtest_cfg_tab->gsblur_coeff.coeff_gauss_0 = slamtest_request->gsblur_coef_cfg.coeff_gauss_0;
		slamtest_cfg_tab->gsblur_coeff.coeff_gauss_1 = slamtest_request->gsblur_coef_cfg.coeff_gauss_1;
		slamtest_cfg_tab->gsblur_coeff.coeff_gauss_2 = slamtest_request->gsblur_coef_cfg.coeff_gauss_2;
		slamtest_cfg_tab->gsblur_coeff.coeff_gauss_3 = slamtest_request->gsblur_coef_cfg.coeff_gauss_3;
	}
	else
	{   
	    slamtest_cfg_tab->gsblur_coeff.to_use = 0;
    }
}

static void slamtest_threshold_param_config(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{
	if(slamtest_request->basic_cfg.fast_en == 1)
	{   
	    slamtest_cfg_tab->threshold_param.to_use = 1;	
		slamtest_cfg_tab->threshold_param.ini_th = slamtest_request->threshold_cfg.ini_th;
		slamtest_cfg_tab->threshold_param.min_th = slamtest_request->threshold_cfg.min_th;
	}
    else
	{
		slamtest_cfg_tab->threshold_param.to_use = 0;
	}
}

static void slamtest_nms_win_param_config(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{	
	if(slamtest_request->basic_cfg.nms_en == 1)
	{   
	    slamtest_cfg_tab->nms_win_param.to_use = 1;   
		slamtest_cfg_tab->nms_win_param.nmscell_h = slamtest_request->nms_win_cfg.nmscell_h;
		slamtest_cfg_tab->nms_win_param.nmscell_v = slamtest_request->nms_win_cfg.nmscell_v;
	}
	else
	{   
		slamtest_cfg_tab->nms_win_param.to_use = 0;
	}
}

static void slamtest_pattern_param_config(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{
   unsigned int i = 0;
    U_BRIEF_PATTERN    temp_pattern_param;

    temp_pattern_param.u32 = 0;
    if(slamtest_request->basic_cfg.brief_en == 1 ||slamtest_request->basic_cfg.freak_en == 1)
    {   
        for(i = 0; i < SLAM_PATTERN_NUM; i++)
        { 
            temp_pattern_param.u32 = 0;
            slamtest_cfg_tab->pattern_param.to_use = 1;
            temp_pattern_param.bits.pattern_y1 = slamtest_request->pattern_cfg[i].pattern_y1;
            temp_pattern_param.bits.pattern_x1 = slamtest_request->pattern_cfg[i].pattern_x1;
            temp_pattern_param.bits.pattern_y0 = slamtest_request->pattern_cfg[i].pattern_y0;
            temp_pattern_param.bits.pattern_x0 = slamtest_request->pattern_cfg[i].pattern_x0; 
            slamtest_cfg_tab->pattern_param.pattern_param[i] = temp_pattern_param.u32;
        }
    }
    else
    {
        slamtest_cfg_tab->pattern_param.to_use = 0;
    }
}

static void slamtest_update_request_cfg_tab(msg_req_slamtest_request_t* slamtest_request, slam_config_table_t* slamtest_cfg_tab)
{
    pr_info("[%s] +\n", __func__);
    
    slamtest_basic_config(slamtest_request,slamtest_cfg_tab);
    slamtest_pyramid_param_config(slamtest_request,slamtest_cfg_tab);
    slamtest_gsblur_coeff_config(slamtest_request,slamtest_cfg_tab);
    slamtest_threshold_param_config(slamtest_request,slamtest_cfg_tab);
    slamtest_nms_win_param_config(slamtest_request,slamtest_cfg_tab);
    slamtest_pattern_param_config(slamtest_request,slamtest_cfg_tab);
  
    pr_info("[%s] -\n", __func__);
}

static int slamtest_set_cmdlst_stripe(cmdlst_stripe_info_t* stripe, unsigned int stripe_cnt)
{
    pr_info("[%s] +\n", __func__); 
    unsigned int i = 0;
    for(i = 0; i < stripe_cnt; i++)
    {
        stripe[i].is_last_stripe   = 0;
    }
    stripe[stripe_cnt-1].is_last_stripe = 1;
	
    pr_info("[%s] -\n", __func__);
   return CPE_FW_OK;
}

static void slamtest_update_cmdlst_cfg_tab(cfg_tab_cmdlst_t* slamtest_cmdlst_cfg_tab,cmdlst_para_t * slamtest_cmdlst_para,cmd_buf_t * slamtest_cmd_buf)
{
    pr_info("[%s] +\n", __func__);
    
    slamtest_cmdlst_cfg_tab->cfg.to_use = 1;
    slamtest_cmdlst_cfg_tab->cfg.prefetch = 0;
    slamtest_cmdlst_cfg_tab->cfg.slowdown_nrt_channel = 0;

    slamtest_cmdlst_cfg_tab->ch_cfg.to_use = 1;
    slamtest_cmdlst_cfg_tab->ch_cfg.nrt_channel = 1;
    slamtest_cmdlst_cfg_tab->ch_cfg.active_token_nbr_en = 0;
    slamtest_cmdlst_cfg_tab->ch_cfg.active_token_nbr = 0;

    slamtest_cmdlst_cfg_tab->sw_ch_mngr_cfg.to_use = 1;
    slamtest_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_link_channel = 0;
    slamtest_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_link_token_nbr = 0;
    slamtest_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_ressource_request = 0;
    slamtest_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_priority = 0;

    slamtest_cmdlst_cfg_tab->vp_rd_cfg.to_use= 1;
    slamtest_cmdlst_cfg_tab->vp_rd_cfg.vp_rd_id = slamtest_cmdlst_para->channel_id;
    slamtest_cmdlst_cfg_tab->vp_rd_cfg.rd_addr  = slamtest_cmd_buf->start_addr_isp_map;
    slamtest_cmdlst_cfg_tab->vp_rd_cfg.rd_size  = slamtest_cmd_buf->data_size;
  
    pr_info("[%s] -\n", __func__);
}

int slamtest_request_handler(msg_req_slamtest_request_t* slamtest_request)
{
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = 0;

    pr_info("+\n");
    
    if (slamtest_request == NULL) {
        pr_err("[%s] failed : slamtest_request is null", __func__);
        return -1;
    }

    slamtest_request_dump(slamtest_request);

    if ((ret = cpe_mem_get(MEM_ID_SLAM_CFG_TAB, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_SLAM_CFG_TAB, va, da, ret);
        return CPE_FW_ERR;
    }
    slam_config_table_t* slamtest_cfg_tab = (slam_config_table_t*)va;

    if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_SLAM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CVDR_CFG_TAB_SLAM, va, da, ret);
        return CPE_FW_ERR;
    }
    cfg_tab_cvdr_t* slam_cvdr_test_cfg_tab = (cfg_tab_cvdr_t*)va;

    if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_SLAM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_PARA_SLAM, va, da, ret);
        return CPE_FW_ERR;
    }
    cmdlst_para_t* slam_cmdlst_test_para = (cmdlst_para_t*)va;
	
    slamtest_update_request_cfg_tab(slamtest_request, slamtest_cfg_tab);
    slamtest_update_cvdr_cfg_tab(slamtest_request, slam_cvdr_test_cfg_tab);
 
    slam_cmdlst_test_para->channel_id = 1;
    slam_cmdlst_test_para->stripe_cnt = 1;

    slamtest_set_cmdlst_stripe(&slam_cmdlst_test_para->cmd_stripe_info[0],slam_cmdlst_test_para->stripe_cnt);
    df_sched_prepare(slam_cmdlst_test_para);
    schedule_cmdlst_link_t* cmd_link_entry = (schedule_cmdlst_link_t*)slam_cmdlst_test_para->cmd_entry;
    slamtest_set_irq_mode(slamtest_request,cmd_link_entry,slam_cmdlst_test_para->stripe_cnt);
    df_sched_set_buffer_header(slam_cmdlst_test_para);

    slam_prepare_cmd(&g_slam_devs[0], &cmd_link_entry[0].cmd_buf, &slamtest_cfg_tab[0]);
    cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[0].cmd_buf, &slam_cvdr_test_cfg_tab[0]);
    cmdlst_set_buffer_padding(&cmd_link_entry[0].cmd_buf);
    
    cfg_tab_cmdlst_t cmdlst_cfg;
    slamtest_update_cmdlst_cfg_tab(&cmdlst_cfg,slam_cmdlst_test_para,&cmd_link_entry[0].cmd_buf);
    loge_if_ret(df_sched_start(&cmdlst_cfg,slam_cmdlst_test_para));
    
    cpe_mem_free(MEM_ID_SLAM_CFG_TAB);
    cpe_mem_free(MEM_ID_CVDR_CFG_TAB_SLAM);

    pr_info("-\n");
   return CPE_FW_OK;
   
}

int slamtest_eof_handler()
{
    pr_info("[%s] +\n", __func__);
    cmdlst_eop_handler(CMD_EOF_SLAMTEST_MODE);
    pr_info("[%s] -\n", __func__);
    return CPE_FW_OK;
}

static int slamtest_request_dump(msg_req_slamtest_request_t* req)
{
   
	unsigned int i=0;
    for(i = 0; i < SLAM_STREAM_MAX; i++)
    {                
        pr_info("streams[%d].width  = %x\n",i,req->streams[i].width);
		pr_info("streams[%d].height = %x\n",i,req->streams[i].height);
		pr_info("streams[%d].buf    = %x\n",i,req->streams[i].buf);
		pr_info("streams[%d].format = %x\n",i,req->streams[i].format);
    } 
	
	pr_info("frame_number  = %x\n", req->frame_number);
       pr_info("pyramid_layer = %x\n", req->pyramid_layer_num);
	pr_info("req->basic_cfg.scoremap_en = %x\n",req->basic_cfg.scoremap_en);
	pr_info("req->basic_cfg.freak_en    = %x\n",req->basic_cfg.freak_en);
	pr_info("req->basic_cfg.brief_en    = %x\n",req->basic_cfg.brief_en);
	pr_info("req->basic_cfg.orient_en   = %x\n",req->basic_cfg.orient_en);
	pr_info("req->basic_cfg.nms_en      = %x\n",req->basic_cfg.nms_en);
	pr_info("req->basic_cfg.fast_en     = %x\n",req->basic_cfg.fast_en);
	pr_info("req->basic_cfg.gsblur_en   = %x\n",req->basic_cfg.gsblur_en);
	pr_info("req->basic_cfg.pyramid_en  = %x\n",req->basic_cfg.pyramid_en);
	
	pr_info("req->request_cfg.pyramid_cfg.scl_inc     = %x\n",req->pyramid_cfg.scl_inc);
	pr_info("req->request_cfg.pyramid_cfg.scl_vbottom = %x\n",req->pyramid_cfg.scl_vbottom);
	pr_info("req->request_cfg.pyramid_cfg.scl_vtop    = %x\n",req->pyramid_cfg.scl_vtop);
 	pr_info("req->request_cfg.pyramid_cfg.scl_hright  = %x\n",req->pyramid_cfg.scl_hright);
 	pr_info("req->request_cfg.pyramid_cfg.scl_hleft   = %x\n",req->pyramid_cfg.scl_hleft);

       pr_info("req->request_cfg.gsblur_coef_cfg.coeff_gauss_0 = %x\n",req->gsblur_coef_cfg.coeff_gauss_0);
	pr_info("req->request_cfg.gsblur_coef_cfg.coeff_gauss_1 = %x\n",req->gsblur_coef_cfg.coeff_gauss_1);
       pr_info("req->request_cfg.gsblur_coef_cfg.coeff_gauss_2 = %x\n",req->gsblur_coef_cfg.coeff_gauss_2);
	pr_info("req->request_cfg.gsblur_coef_cfg.coeff_gauss_3 = %x\n",req->gsblur_coef_cfg.coeff_gauss_3);

	pr_info("req->request_cfg.threshold_cfg.ini_th = %x\n",req->threshold_cfg.ini_th);
       pr_info("req->request_cfg.threshold_cfg.min_th = %x\n",req->threshold_cfg.min_th);

	pr_info("req->request_cfg.nms_win_cfg.nmscell_h = %x\n",req->nms_win_cfg.nmscell_h);
	pr_info("req->request_cfg.nms_win_cfg.nmscell_v = %x\n",req->nms_win_cfg.nmscell_v);

   for(i = 0; i < 20; i++)
   {       
       pr_info("req->pattern_cfg[%d].pattern_y1 = %x\n",i,req->pattern_cfg[i].pattern_y1);
	   pr_info("req->pattern_cfg[%d].pattern_x1 = %x\n",i,req->pattern_cfg[i].pattern_x1);
	   pr_info("req->pattern_cfg[%d].pattern_y0 = %x\n",i,req->pattern_cfg[i].pattern_y0);
	   pr_info("req->pattern_cfg[%d].pattern_x0 = %x\n",i,req->pattern_cfg[i].pattern_x0);
   }
    return 0;
}
#pragma GCC diagnostic pop
