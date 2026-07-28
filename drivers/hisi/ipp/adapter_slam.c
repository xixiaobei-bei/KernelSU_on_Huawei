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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

#define LOG_TAG LOG_MODULE_SLAM  
#define CVDR_KEY_POINT_OVERFLOW  0x8000

extern slam_dev_t g_slam_devs[];
unsigned int g_slam_read_st = 1;
static unsigned long long  g_slam_stat_addr = 0;

static int slam_request_dump(msg_req_slam_request_t* req);

static int slam_set_irq_mode(msg_req_slam_request_t* req, schedule_cmdlst_link_t* cmd_link_entry,unsigned int stripe_cnt)
{
    unsigned int i = 0;

    for(i = 0;i < stripe_cnt; i++)
    {
        if(g_slam_read_st && i % 2 != 0)
        {
            cmd_link_entry[i].irq_mode = CMD_IRQ_SLAM_READ_STAT_MODE;
        }
        else
        {
            if(req->basic_cfg.scoremap_en == 1)
            {
                cmd_link_entry[i].irq_mode = CMD_IRQ_SLAM_SCOREMAP_MODE;
            }
            else if(req->pyramid_cfg[i / 2].pyramid_en == 1)
            {
                cmd_link_entry[i].irq_mode = CMD_IRQ_SLAM_PYRAMID_MODE;
            }
            else
            {
                cmd_link_entry[i].irq_mode = CMD_IRQ_SLAM_NORMAL_MODE;
            }
        }
    }

    return 0;
}

static int slam_update_cvdr_cfg_tab(msg_req_slam_request_t* slam_request, cfg_tab_cvdr_t* slam_cfg_tab, unsigned int pyramid_index)
{
    D("[%s] +\n", __func__);
    cvdr_opt_fmt_t cfmt;
    pix_format_e format = PIXEL_FMT_CPE_MAX;
    unsigned int slam_stride = 0;
    if(slam_request->streams[pyramid_index][BI_SLAM_Y].buf)
    {
        cfmt.id = RD0_SLAM;
        cfmt.width = slam_request->streams[pyramid_index][BI_SLAM_Y].width;
        cfmt.full_width = slam_request->streams[pyramid_index][BI_SLAM_Y].width/2;
        cfmt.line_size = slam_request->streams[pyramid_index][BI_SLAM_Y].width/2;
        cfmt.height = slam_request->streams[pyramid_index][BI_SLAM_Y].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_SLAM_IN;
        cfmt.addr = slam_request->streams[pyramid_index][BI_SLAM_Y].buf;

        if (0 == pyramid_index)
        {
            slam_stride = slam_request->streams[pyramid_index][BI_SLAM_Y].stride / 2;
            dataflow_cvdr_rd_cfg_vp(slam_cfg_tab, &cfmt, slam_stride, CPE_CLK, format);
        }
        else
        {
            dataflow_cvdr_rd_cfg_vp(slam_cfg_tab, &cfmt, 0, CPE_CLK, format);
        }

    }

     if(slam_request->streams[pyramid_index][BO_SLAM_Y].buf)
    {
        cfmt.id = WR0_SLAM;
        cfmt.width = slam_request->streams[pyramid_index][BO_SLAM_Y].width;
        cfmt.full_width = slam_request->streams[pyramid_index][BO_SLAM_Y].width/2;
        cfmt.line_size = slam_request->streams[pyramid_index][BO_SLAM_Y].width/2;
        cfmt.height = slam_request->streams[pyramid_index][BO_SLAM_Y].height;
        cfmt.addr = slam_request->streams[pyramid_index][BO_SLAM_Y].buf;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_SLAM_OUT;
        dataflow_cvdr_wr_cfg_vp(slam_cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

     if(slam_request->streams[pyramid_index][BO_SLAM_STAT].buf)
    {
        cfmt.id = WR1_SLAM;
        cfmt.addr = slam_request->streams[pyramid_index][BO_SLAM_STAT].buf;
        if(slam_request->basic_cfg.brief_en == 1)
        {
            dataflow_cvdr_cfg_d32(slam_cfg_tab, &cfmt, slam_request->streams[pyramid_index][BO_SLAM_STAT].width * 40 + 4+CVDR_KEY_POINT_OVERFLOW);
        }
        else
        {
            dataflow_cvdr_cfg_d32(slam_cfg_tab, &cfmt, slam_request->streams[pyramid_index][BO_SLAM_STAT].width * 72 + 4+CVDR_KEY_POINT_OVERFLOW);
        }
    }

    if(slam_request->streams[pyramid_index][BO_SLAM_SCORE_Y].buf)
    {
        cfmt.id = WR2_SLAM;
        cfmt.width = slam_request->streams[pyramid_index][BO_SLAM_SCORE_Y].width;
        cfmt.full_width = slam_request->streams[pyramid_index][BO_SLAM_SCORE_Y].width/2;
        cfmt.line_size = slam_request->streams[pyramid_index][BO_SLAM_SCORE_Y].width/2;
        cfmt.height = slam_request->streams[pyramid_index][BO_SLAM_SCORE_Y].height;
        cfmt.addr = slam_request->streams[pyramid_index][BO_SLAM_SCORE_Y].buf; 
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_SLAM_SCORE;
        dataflow_cvdr_wr_cfg_vp(slam_cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    D("[%s] -\n", __func__);
    return 0;
}

static void slam_basic_config(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab, unsigned int pyramid_index)
{
    slam_cfg_tab->to_use = 1;
    slam_cfg_tab->basic_cfg.to_use   = 1;
    slam_cfg_tab->basic_cfg.slam_cfg.to_use  = 1;
    slam_cfg_tab->basic_cfg.slam_cfg.brief_en = slam_request->basic_cfg.brief_en;
    slam_cfg_tab->basic_cfg.slam_cfg.fast_en = slam_request->basic_cfg.fast_en;
    slam_cfg_tab->basic_cfg.slam_cfg.freak_en = slam_request->basic_cfg.freak_en;
    slam_cfg_tab->basic_cfg.slam_cfg.gsblur_en = slam_request->basic_cfg.gsblur_en;
    slam_cfg_tab->basic_cfg.slam_cfg.nms_en = slam_request->basic_cfg.nms_en;
    slam_cfg_tab->basic_cfg.slam_cfg.orient_en = slam_request->basic_cfg.orient_en;
    slam_cfg_tab->basic_cfg.slam_cfg.pyramid_en = slam_request->pyramid_cfg[pyramid_index].pyramid_en;
    slam_cfg_tab->basic_cfg.slam_cfg.scoremap_en = slam_request->basic_cfg.scoremap_en;
    slam_cfg_tab->basic_cfg.slam_size.width = slam_request->streams[pyramid_index][BI_SLAM_Y].width;
    slam_cfg_tab->basic_cfg.slam_size.height = slam_request->streams[pyramid_index][BI_SLAM_Y].height;
}

static void slam_pyramid_param_config(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab, unsigned int pyramid_index)
{
    if(slam_request->pyramid_cfg[pyramid_index].pyramid_en == 1)
    {         
        slam_cfg_tab->pyramid_param.to_use = 1;
        slam_cfg_tab->pyramid_param.scl_inc = slam_request->pyramid_cfg[pyramid_index].scl_inc;
        slam_cfg_tab->pyramid_param.scl_vbottom = slam_request->pyramid_cfg[pyramid_index].scl_vbottom;
        slam_cfg_tab->pyramid_param.scl_vtop = slam_request->pyramid_cfg[pyramid_index].scl_vtop;
        slam_cfg_tab->pyramid_param.scl_hright = slam_request->pyramid_cfg[pyramid_index].scl_hright;
        slam_cfg_tab->pyramid_param.scl_hleft = slam_request->pyramid_cfg[pyramid_index].scl_hleft;
    }
    else
    {   
        slam_cfg_tab->pyramid_param.to_use = 0;
    }
}

static void slam_gsblur_coeff_config(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab)
{
    if(slam_request->basic_cfg.gsblur_en == 1)
    {         
        slam_cfg_tab->gsblur_coeff.to_use = 1;
        slam_cfg_tab->gsblur_coeff.coeff_gauss_0 = slam_request->gsblur_coef_cfg.coeff_gauss_0;
        slam_cfg_tab->gsblur_coeff.coeff_gauss_1 = slam_request->gsblur_coef_cfg.coeff_gauss_1;
        slam_cfg_tab->gsblur_coeff.coeff_gauss_2 = slam_request->gsblur_coef_cfg.coeff_gauss_2;
        slam_cfg_tab->gsblur_coeff.coeff_gauss_3 = slam_request->gsblur_coef_cfg.coeff_gauss_3;
    }
    else
    {   
        slam_cfg_tab->gsblur_coeff.to_use = 0;
    }
}

static void slam_threshold_param_config(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab)
{
    if(slam_request->basic_cfg.fast_en == 1)
    {   
        slam_cfg_tab->threshold_param.to_use = 1;
        slam_cfg_tab->threshold_param.ini_th = slam_request->threshold_cfg.ini_th;
        slam_cfg_tab->threshold_param.min_th = slam_request->threshold_cfg.min_th;
    }
    else
    {
        slam_cfg_tab->threshold_param.to_use = 0;
    }
}

static void slam_nms_win_param_config(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab, unsigned int pyramid_index)
{
    if(slam_request->basic_cfg.nms_en == 1)
    {   
        slam_cfg_tab->nms_win_param.to_use = 1;   
        slam_cfg_tab->nms_win_param.nmscell_h = slam_request->nms_win_cfg[pyramid_index].nmscell_h;
        slam_cfg_tab->nms_win_param.nmscell_v = slam_request->nms_win_cfg[pyramid_index].nmscell_v;
    }
    else
    {   
        slam_cfg_tab->nms_win_param.to_use = 0;
    }
}

static void slam_pattern_param_config(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab)
{
    unsigned int i = 0;
    U_BRIEF_PATTERN    temp_pattern_param;

    temp_pattern_param.u32 = 0;
    if(slam_request->basic_cfg.brief_en == 1 ||slam_request->basic_cfg.freak_en == 1)
    {   
        for(i = 0; i < SLAM_PATTERN_NUM; i++)
        { 
            temp_pattern_param.u32 = 0;
            slam_cfg_tab->pattern_param.to_use = 1;
            temp_pattern_param.bits.pattern_y1 = slam_request->pattern_cfg[i].pattern_y1;
            temp_pattern_param.bits.pattern_x1 = slam_request->pattern_cfg[i].pattern_x1;
            temp_pattern_param.bits.pattern_y0 = slam_request->pattern_cfg[i].pattern_y0;
            temp_pattern_param.bits.pattern_x0 = slam_request->pattern_cfg[i].pattern_x0; 
            slam_cfg_tab->pattern_param.pattern_param[i] = temp_pattern_param.u32;
        }
    }
    else
    {
        slam_cfg_tab->pattern_param.to_use = 0;
    }
}

static void slam_update_request_cfg_tab(msg_req_slam_request_t* slam_request, slam_config_table_t* slam_cfg_tab, unsigned int pyramid_index)
{
    D("[%s] +\n", __func__);

    slam_basic_config(slam_request,slam_cfg_tab,pyramid_index);
    slam_pyramid_param_config(slam_request,slam_cfg_tab,pyramid_index);
    slam_gsblur_coeff_config(slam_request,slam_cfg_tab);
    slam_threshold_param_config(slam_request,slam_cfg_tab);
    slam_nms_win_param_config(slam_request,slam_cfg_tab,pyramid_index);
    slam_pattern_param_config(slam_request,slam_cfg_tab);

    D("[%s] -\n", __func__);
}

static int slam_set_cmdlst_stripe(cmdlst_stripe_info_t* stripe, unsigned int stripe_cnt)
{
    D("[%s] +\n", __func__); 
    unsigned int i = 0;
    unsigned int slam_stat_addr = 0;
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = 0;

    if ((ret = cpe_mem_get(MEM_ID_SLAM_STAT, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_SLAM_STAT, va, da, ret);
        return CPE_FW_ERR;
    }
    slam_stat_addr = da;
    g_slam_stat_addr = va;
    for(i = 0; i < stripe_cnt; i++)
    {
        if(g_slam_read_st && i % 2 != 0)
        {
            stripe[i].rd_cfg_info.fs           = slam_stat_addr + 8 * SLAM_SCOREHIST_NUM * (i - 1) /2;
            stripe[i].rd_cfg_info.rd_cfg_num   = 1;
            stripe[i].rd_cfg_info.rd_cfg[0]    = 0x3f00C101;
        }
        else
        {
            stripe[i].rd_cfg_info.fs = 0;
        }
        stripe[i].is_last_stripe   = 0;
    }
    stripe[stripe_cnt-1].is_last_stripe = 1;

    D("[%s] -\n", __func__);
   return CPE_FW_OK;
}

static void slam_update_cmdlst_cfg_tab(cfg_tab_cmdlst_t* slam_cmdlst_cfg_tab,cmdlst_para_t * slam_cmdlst_para,cmd_buf_t * slam_cmd_buf)
{
    D("[%s] +\n", __func__);
    
    slam_cmdlst_cfg_tab->cfg.to_use = 1;
    slam_cmdlst_cfg_tab->cfg.prefetch = 0;
    slam_cmdlst_cfg_tab->cfg.slowdown_nrt_channel = 0;

    slam_cmdlst_cfg_tab->ch_cfg.to_use = 1;
    slam_cmdlst_cfg_tab->ch_cfg.nrt_channel = 1;
    slam_cmdlst_cfg_tab->ch_cfg.active_token_nbr_en = 0;
    slam_cmdlst_cfg_tab->ch_cfg.active_token_nbr = 0;

    slam_cmdlst_cfg_tab->sw_ch_mngr_cfg.to_use = 1;
    slam_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_link_channel = 0;
    slam_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_link_token_nbr = 0;
    slam_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_ressource_request = 0;
    slam_cmdlst_cfg_tab->sw_ch_mngr_cfg.sw_priority = 0;

    slam_cmdlst_cfg_tab->vp_rd_cfg.to_use= 1;
    slam_cmdlst_cfg_tab->vp_rd_cfg.vp_rd_id = slam_cmdlst_para->channel_id;
    slam_cmdlst_cfg_tab->vp_rd_cfg.rd_addr  = slam_cmd_buf->start_addr_isp_map;
    slam_cmdlst_cfg_tab->vp_rd_cfg.rd_size  = slam_cmd_buf->data_size;
  
    D("[%s] -\n", __func__);
}

int slam_request_handler(msg_req_slam_request_t* slam_request)
{
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = 0;

    D("+\n");
    
    if (slam_request == NULL) {
        pr_err("[%s] failed : slam_request is null", __func__);
        return -1;
    }

    slam_request_dump(slam_request);

    if ((ret = cpe_mem_get(MEM_ID_SLAM_CFG_TAB, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_SLAM_CFG_TAB, va, da, ret);
        return CPE_FW_ERR;
    }
    slam_config_table_t* slam_cfg_tab = (slam_config_table_t*)va;

    if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_SLAM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CVDR_CFG_TAB_SLAM, va, da, ret);
        return CPE_FW_ERR;
    }
    cfg_tab_cvdr_t* slam_cvdr_cfg_tab = (cfg_tab_cvdr_t*)va;

    if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_SLAM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_PARA_SLAM, va, da, ret);
        return CPE_FW_ERR;
    }
    cmdlst_para_t* slam_cmdlst_para = (cmdlst_para_t*)va;
    unsigned int i = 0;
    for(i = 0;i < slam_request->pyramid_layer_num; i++)
    {
        slam_update_request_cfg_tab(slam_request, &slam_cfg_tab[i],i);
        slam_update_cvdr_cfg_tab(slam_request, &slam_cvdr_cfg_tab[i],i);
    }
    slam_cmdlst_para->channel_id = 1;
    if(g_slam_read_st)
    {
        slam_cmdlst_para->stripe_cnt = slam_request->pyramid_layer_num * 2;
    }
    else
    {
        slam_cmdlst_para->stripe_cnt = slam_request->pyramid_layer_num;
    }
    

    slam_set_cmdlst_stripe(&slam_cmdlst_para->cmd_stripe_info[0],slam_cmdlst_para->stripe_cnt);

    df_sched_prepare(slam_cmdlst_para);
    schedule_cmdlst_link_t* cmd_link_entry = (schedule_cmdlst_link_t*)slam_cmdlst_para->cmd_entry;
    slam_set_irq_mode(slam_request,cmd_link_entry,slam_cmdlst_para->stripe_cnt);
    df_sched_set_buffer_header(slam_cmdlst_para);

    for(i = 0;i < slam_cmdlst_para->stripe_cnt; i++)
    {
        if(g_slam_read_st)
        {
            if(i % 2 != 0)
            {
                continue;
            }
            else
            {    
                slam_prepare_cmd(&g_slam_devs[0], &cmd_link_entry[i].cmd_buf, &slam_cfg_tab[i/2]);	
                cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &slam_cvdr_cfg_tab[i/2]);
                cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
            }
        }
        else
        {
            slam_prepare_cmd(&g_slam_devs[0], &cmd_link_entry[i].cmd_buf, &slam_cfg_tab[i]);
            cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &slam_cvdr_cfg_tab[i]);
            cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
        }
    }
    
    cfg_tab_cmdlst_t cmdlst_cfg;
    slam_update_cmdlst_cfg_tab(&cmdlst_cfg,slam_cmdlst_para,&cmd_link_entry[0].cmd_buf);

    loge_if_ret(df_sched_start(&cmdlst_cfg,slam_cmdlst_para));

    
    cpe_mem_free(MEM_ID_SLAM_CFG_TAB);
    cpe_mem_free(MEM_ID_CVDR_CFG_TAB_SLAM);
    
    D("-\n");
    return CPE_FW_OK;
   
}

int slam_eof_handler(msg_req_slam_request_t* slam_request)
{
    D("[%s] +\n", __func__);
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int ret = 0;
    unsigned short *  user_score_hist = slam_request->score_hist;
	
    if (NULL == user_score_hist)
    {
        pr_err("[%s] failed : user_score_hist  is null", __func__);
        return -CPE_FW_ERR;
    }
    unsigned short *  score_hist_local = NULL;
    score_hist_local = (unsigned short *)kmalloc(sizeof(unsigned short) * MAX_PYRAMID_LAYER * SLAM_SCOREHIST_NUM, GFP_KERNEL);
    if (NULL == score_hist_local)
    {
        pr_err("[%s] failed : kmalloc score_hist_local  is null", __func__);
        return -1;
    }
    unsigned short*   score_hist_local_tmp = NULL;
    score_hist_local_tmp = score_hist_local;
    for (i = 0; i < MAX_PYRAMID_LAYER; i++)
    {
        pr_info("-----layer %d-----", i);
        for (j = 0; j < SLAM_SCOREHIST_NUM; j++)
        {
            *score_hist_local_tmp = (unsigned short)readl((volatile void __iomem *)(g_slam_stat_addr 
               + 8 * j + i * 8 * SLAM_SCOREHIST_NUM));
	     score_hist_local_tmp++;
        }
    }
    ret = copy_to_user((void __user *)user_score_hist, score_hist_local, sizeof(unsigned short) * MAX_PYRAMID_LAYER * SLAM_SCOREHIST_NUM);
    kfree(score_hist_local);
    if (ret != 0)
    {
        pr_err("[%s] failed : copy_to_user.%d\n", __func__, ret);
        return -CPE_FW_ERR;
    }
    g_slam_stat_addr = 0;
    cmdlst_eop_handler(CMD_EOF_SLAM_MODE);
    cpe_mem_free(MEM_ID_SLAM_STAT);
   
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

static int slam_request_dump(msg_req_slam_request_t* req)
{
    unsigned int i = 0,j = 0;
    pr_info("frame_num = %d,pyramid_layer = %d",req->frame_number,req->pyramid_layer_num);
    pr_info("basic_config:[scoremap_en = %d][freak_en = %d][brief_en = %d][orient_en = %d][nms_en = %d][fast_en = %d][gsblur_en = %d]"
        ,req->basic_cfg.scoremap_en,req->basic_cfg.freak_en,req->basic_cfg.brief_en
        ,req->basic_cfg.orient_en,req->basic_cfg.nms_en,req->basic_cfg.fast_en,req->basic_cfg.gsblur_en);
    pr_info("coeff_gauss = 0x%x,0x%x,0x%x,0x%x\n",req->gsblur_coef_cfg.coeff_gauss_0,req->gsblur_coef_cfg.coeff_gauss_1,req->gsblur_coef_cfg.coeff_gauss_2,req->gsblur_coef_cfg.coeff_gauss_3);
    pr_info("ini_th = %d,min_th = %d\n",req->threshold_cfg.ini_th,req->threshold_cfg.min_th);

    for(j = 0; j < req->pyramid_layer_num; j++)
    {
        pr_info("------layer%d------",j);
        for(i = 0; i < SLAM_STREAM_MAX; i++)
        {                
            pr_info("streams[%d] width  = %d,height = %d,buf = 0x%x,format = %d\n",
                i,req->streams[j][i].width,req->streams[j][i].height,req->streams[j][i].buf,req->streams[j][i].format);
        }
        pr_info("pyramid_en = %d",req->pyramid_cfg[j].pyramid_en);
        if(req->pyramid_cfg[j].pyramid_en)
        {
            pr_info("pyramid_cfg:scl_inc = 0x%x,scl_vbottom = 0x%x,scl_vtop = 0x%x,scl_hright = 0x%x,scl_hleft = 0x%x\n",req->pyramid_cfg[j].scl_inc,req->pyramid_cfg[j].scl_vbottom
                ,req->pyramid_cfg[j].scl_vtop,req->pyramid_cfg[j].scl_hright,req->pyramid_cfg[j].scl_hleft);
        }
        pr_info("nmscell_h = %d,nmscell_v = %d\n",req->nms_win_cfg[j].nmscell_h,req->nms_win_cfg[j].nmscell_v);
     //   pr_info("scorehist addr = 0x%llx",(unsigned long long)req->score_hist[j]);
    }

    return 0;
}

#pragma GCC diagnostic pop
