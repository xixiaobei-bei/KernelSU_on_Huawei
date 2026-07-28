#include <linux/string.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include "mfnr.h"
#include "cfg_table_mfnr.h"
#include "cvdr_drv.h"
#include "mfnr_drv.h"
#include "ipp_top_drv.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"
#include "sub_ctrl_reg_offset.h"

#define LOG_TAG LOG_MODULE_MFNR
#define ALIGN_MFNR_FULL       (128)
#define ALIGN_MFNR_PARTIAL    (16)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wframe-larger-than="
/*lint -e429  -e613 -e668*/

extern mfnr_dev_t g_mfnr_devs[];
extern unsigned int g_mfnr_read_stat_flag;
extern int memset_s(void *dest, size_t destMax, int c, size_t count);
extern int memcpy_s(void* dest, size_t destMax, const void* src, size_t count);

static int mfnr_dump_request(msg_req_mfnr_request_t* req);
static int mfnr_calc_stripe_info(unsigned int is_full_mode,cpe_stream_t* stream, isp_stripe_info_t* stripe_mfnr_origin, isp_stripe_info_t* stripe_mfnr_ds4, isp_stripe_info_t* stripe_mfnr_ds16, isp_stripe_info_t* stripe_mfnr_gm);

static int mfnr_update_top_cfg_tab(msg_req_mfnr_request_t* mfnr_request, cpe_top_config_table_t* cfg_tab, isp_stripe_info_t* stripe_mfnr_origin, isp_stripe_info_t* stripe_mfnr_ds4, isp_stripe_info_t* stripe_mfnr_gm, unsigned int stripe_index)
{
    D("[%s] +\n", __func__);
    cfg_tab->to_use = 1;
    cfg_tab->crop_vpwr.to_use = 1;

    if(mfnr_request->streams[BO_MFNR_RESULT_Y].buf)
    {
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft = stripe_mfnr_origin->overlap_left[stripe_index];
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright = stripe_mfnr_origin->stripe_width[stripe_index] - stripe_mfnr_origin->overlap_right[stripe_index];
    }
    else
    {
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft = 0;
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright = CROP_DEFAULT_VALUE;
    }

    if(mfnr_request->streams[BO_MFNR_RESULT_UV].buf)
    {
        cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = stripe_mfnr_ds4->overlap_left[stripe_index];
        cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = stripe_mfnr_ds4->stripe_width[stripe_index] - stripe_mfnr_ds4->overlap_right[stripe_index];
    }
    else
    {
        cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = 0;
        cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = CROP_DEFAULT_VALUE;
    }

    if(mfnr_request->streams[BO_MFNR_GHOST_MASK].buf)
    {
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft = stripe_mfnr_gm->overlap_left[stripe_index];
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright = stripe_mfnr_gm->stripe_width[stripe_index] - stripe_mfnr_gm->overlap_right[stripe_index];
    }
    else
    {
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft = 0;
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright = CROP_DEFAULT_VALUE;
    }
    D("[%s] -\n", __func__);
    
    return CPE_FW_OK;
}

static int mfnr_update_cvdr_rd_cfg_tab(msg_req_mfnr_request_t* mfnr_request, cfg_tab_cvdr_t* cfg_tab, isp_stripe_info_t* stripe_mfnr_origin, isp_stripe_info_t* stripe_mfnr_ds4, isp_stripe_info_t* stripe_mfnr_ds16, isp_stripe_info_t* stripe_mfnr_gm, unsigned int stripe_index)
{
    D("[%s] +\n", __func__);
    cvdr_opt_fmt_t cfmt;
    pix_format_e format = PIXEL_FMT_CPE_MAX;

    if(mfnr_request->streams[BI_MFNR_REF_MATCH_YUV_DS16].buf)
    {
        cfmt.id = RD0_L0;
        cfmt.height = mfnr_request->streams[BI_MFNR_REF_MATCH_YUV_DS16].height;
        cfmt.expand = 0;
        if(mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_COLOR_MODE)
        {
            cfmt.width = stripe_mfnr_ds16->stripe_width[stripe_index];
            cfmt.line_size = stripe_mfnr_ds16->stripe_width[stripe_index];
            format = PIXEL_FMT_CPE_RM_PACKED_YUV;
            cfmt.addr = mfnr_request->streams[BI_MFNR_REF_MATCH_YUV_DS16].buf + (stripe_mfnr_ds16->stripe_start_point[stripe_index] * 6);
            cfmt.full_width = stripe_mfnr_ds16->full_size.width;
	    cfmt.pix_fmt = DF_FMT_INVALID;
            pr_info("cfmt.width=%d  cfmt.height=%d\n", cfmt.width, cfmt.height);
        }
        else if(mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_MONO_MODE)
        {
            cfmt.width = stripe_mfnr_ds16->stripe_width[stripe_index];
            cfmt.line_size = stripe_mfnr_ds16->stripe_width[stripe_index];
            format = PIXEL_FMT_CPE_RM_PACKED_Y;
            cfmt.addr = mfnr_request->streams[BI_MFNR_REF_MATCH_YUV_DS16].buf + (stripe_mfnr_ds16->stripe_start_point[stripe_index] * 2);
            cfmt.full_width = stripe_mfnr_ds16->full_size.width;
	    cfmt.pix_fmt = DF_FMT_INVALID;
            pr_info("cfmt.width=%d  cfmt.height=%d\n", cfmt.width, cfmt.height);
        }
        else if((mfnr_request->mfnr_cfg.op_mode == MFNR_PARTIAL_COLOR_MODE) || (mfnr_request->mfnr_cfg.op_mode == MFNR_PARTIAL_MONO_MODE))
        {
            cfmt.width = stripe_mfnr_origin->stripe_width[stripe_index];
            cfmt.line_size = stripe_mfnr_origin->stripe_width[stripe_index];
            format = PIXEL_FMT_CPE_G8;
            cfmt.addr = mfnr_request->streams[BI_MFNR_REF_MATCH_YUV_DS16].buf + (stripe_mfnr_origin->stripe_start_point[stripe_index]);
            cfmt.full_width = stripe_mfnr_origin->full_size.width;
	    cfmt.pix_fmt = DF_FMT_INVALID;
            pr_info("cfmt.width=%d  cfmt.height=%d\n", cfmt.width, cfmt.height);
        }
        else
        {
            pr_err("MFNR mode false %d\n", mfnr_request->mfnr_cfg.op_mode);
        }
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BI_MFNR_THR_YUV_DS16].buf)
    {
        cfmt.id = RD1_L1;
        cfmt.width = stripe_mfnr_ds16->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_ds16->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BI_MFNR_THR_YUV_DS16].height;
        cfmt.expand = 0;
        cfmt.full_width = stripe_mfnr_ds16->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        if(mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_COLOR_MODE)
        {
            format = PIXEL_FMT_CPE_PACKED_YUV;
            cfmt.addr = mfnr_request->streams[BI_MFNR_THR_YUV_DS16].buf + (stripe_mfnr_ds16->stripe_start_point[stripe_index] * 3);
        }
        else if(mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_MONO_MODE)
        {
            format = PIXEL_FMT_CPE_Y8;
            cfmt.addr = mfnr_request->streams[BI_MFNR_THR_YUV_DS16].buf + (stripe_mfnr_ds16->stripe_start_point[stripe_index]);
        }
        else
        {
            pr_err("MFNR mode false %d\n", mfnr_request->mfnr_cfg.op_mode);
        }
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BI_MFNR_TEMPO_REF_UV].buf)
    {
        cfmt.id = RD2_L2;
        cfmt.width = stripe_mfnr_ds4->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_ds4->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BI_MFNR_TEMPO_REF_UV].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_PACKED_UV;
        cfmt.full_width = stripe_mfnr_ds4->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BI_MFNR_TEMPO_REF_UV].buf + (stripe_mfnr_ds4->stripe_start_point[stripe_index] * 2);
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BI_MFNR_TEMPO_REF_Y].buf)
    {
        cfmt.id = RD3_MONO_DS4;
        cfmt.width = stripe_mfnr_origin->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_origin->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BI_MFNR_TEMPO_REF_Y].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_Y8;
        cfmt.full_width = stripe_mfnr_origin->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BI_MFNR_TEMPO_REF_Y].buf + (stripe_mfnr_origin->stripe_start_point[stripe_index]);

        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BI_MFNR_CURR_UV].buf)
    {
        cfmt.id = RD4_MONO;
        cfmt.width = stripe_mfnr_ds4->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_ds4->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BI_MFNR_CURR_UV].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_PACKED_UV;
        cfmt.full_width = stripe_mfnr_ds4->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BI_MFNR_CURR_UV].buf + (stripe_mfnr_ds4->stripe_start_point[stripe_index] * 2);

        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BI_MFNR_CURR_Y].buf)
    {
        cfmt.id = RD5_COL_Y_DS4;
        cfmt.width = stripe_mfnr_origin->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_origin->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BI_MFNR_CURR_Y].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_Y8;
        cfmt.full_width = stripe_mfnr_origin->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BI_MFNR_CURR_Y].buf + (stripe_mfnr_origin->stripe_start_point[stripe_index]);

        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }
    
    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

static int mfnr_update_cvdr_wr_cfg_tab(msg_req_mfnr_request_t* mfnr_request, cfg_tab_cvdr_t* cfg_tab, isp_stripe_info_t* stripe_mfnr_origin, isp_stripe_info_t* stripe_mfnr_ds4, isp_stripe_info_t* stripe_mfnr_ds16, isp_stripe_info_t* stripe_mfnr_gm, unsigned int stripe_index)
{
    D("[%s] +\n", __func__);
    cvdr_opt_fmt_t cfmt;
    pix_format_e format = PIXEL_FMT_CPE_MAX;

    if(mfnr_request->streams[BO_MFNR_RESULT_UV].buf)
    {
        cfmt.id = WR0_UV;
        cfmt.width = stripe_mfnr_ds4->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_ds4->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BO_MFNR_RESULT_UV].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_PACKED_UV;
        cfmt.full_width = stripe_mfnr_ds4->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BO_MFNR_RESULT_UV].buf + (stripe_mfnr_ds4->stripe_start_point[stripe_index] + stripe_mfnr_ds4->overlap_left[stripe_index])*2;

        dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BO_MFNR_RESULT_Y].buf)
    {
        cfmt.id = WR1_Y;
        cfmt.width = stripe_mfnr_origin->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_origin->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BO_MFNR_RESULT_Y].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_Y8;
        cfmt.full_width = stripe_mfnr_origin->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BO_MFNR_RESULT_Y].buf + (stripe_mfnr_origin->stripe_start_point[stripe_index] + stripe_mfnr_origin->overlap_left[stripe_index]);

        dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mfnr_request->streams[BO_MFNR_GHOST_MASK].buf)
    {
        cfmt.id = WR2_C;
        cfmt.width = stripe_mfnr_gm->stripe_width[stripe_index];
        cfmt.line_size = stripe_mfnr_gm->stripe_width[stripe_index];
        cfmt.height = mfnr_request->streams[BO_MFNR_GHOST_MASK].height;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_G8;
        cfmt.full_width = stripe_mfnr_gm->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.addr = mfnr_request->streams[BO_MFNR_GHOST_MASK].buf + (stripe_mfnr_gm->stripe_start_point[stripe_index] + stripe_mfnr_gm->overlap_left[stripe_index]);

        dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }
    D("[%s] -\n", __func__);
    
    return CPE_FW_OK;
}

static int mfnr_update_mfnr_cfg_tab(unsigned int current_stripe, unsigned int stripe_cnt, msg_req_mfnr_request_t* mfnr_request, mfnr_config_table_t* cfg_tab, unsigned int stripe_width)
{
    D("[%s] +\n", __func__);
    
    cfg_tab->to_use = 1;
    cfg_tab->basic_cfg.to_use                         = 1;
    cfg_tab->basic_cfg.mfnr_cfg.op_mode               = mfnr_request->mfnr_cfg.op_mode;
    cfg_tab->basic_cfg.mfnr_cfg.uv_unpack             = mfnr_request->mfnr_cfg.uv_unpack;
    cfg_tab->basic_cfg.mfnr_cfg.uv_pack               = mfnr_request->mfnr_cfg.uv_pack;
    cfg_tab->basic_cfg.mfnr_cfg.ghost_mask_wr_en      = mfnr_request->mfnr_cfg.ghost_mask_wr_en;
    cfg_tab->basic_cfg.mfnr_cfg.ghost_stats_crop_h_en = mfnr_request->mfnr_cfg.ghost_stats_crop_h_en;
    cfg_tab->basic_cfg.mfnr_size.width                = stripe_width;
    cfg_tab->basic_cfg.mfnr_size.height               = mfnr_request->streams[BI_MFNR_REF_MATCH_YUV_DS16].height;
    
    switch(mfnr_request->mfnr_cfg.op_mode)
    {
        case MFNR_FULL_COLOR_MODE:
        {
            mfnr_full_color_mode_t para = mfnr_request->u.full_color_mode_param;

            cfg_tab->ghost_detect_y.to_use = 1;
            cfg_tab->ghost_detect_y.ghost_detect_cfg.ghost_mask_th    = para.ghost_detection_y.ghost_detect_cfg.ghost_mask_th;
            if(stripe_cnt == 1)
            {
                cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 0;
                cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4;
                cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 0;
                cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width;
            }
            else
            {
                if(current_stripe == 0)
                {
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 0;
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4 -48;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 0;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width - 16;
                }
                else if(current_stripe == stripe_cnt - 1)
                {
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 48;
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 16;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width;
                }
                else
                {
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 48;
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4 - 48;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 16;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width - 16;
                }
            }

            cfg_tab->temporal_fusion.to_use = 1;
            cfg_tab->temporal_fusion.tempo_fusion.weight          = para.temporal_fusion_yuv.tempo_fusion.weight;
            cfg_tab->temporal_fusion.tempo_fusion.ghost_region_th = para.temporal_fusion_yuv.tempo_fusion.ghost_region_th;
            cfg_tab->temporal_fusion.tempo_fusion.subscale        = para.temporal_fusion_yuv.tempo_fusion.subscale;

            cfg_tab->temporal_fusion.dghost_sigma_stren.max_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.max_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.min_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.min_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.judge_grad_region_th      = para.temporal_fusion_yuv.dghost_sigma_stren.judge_grad_region_th;
            cfg_tab->temporal_fusion.dghost_sigma_stren.sigma                     = para.temporal_fusion_yuv.dghost_sigma_stren.sigma;

            cfg_tab->temporal_fusion.dghost_blend_stren.max_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.max_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.min_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.min_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_mean2blend_th         = para.temporal_fusion_yuv.dghost_blend_stren.sup_mean2blend_th;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_blend_scale           = para.temporal_fusion_yuv.dghost_blend_stren.sup_blend_scale;

            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int)); 
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int));
            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int)); 
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int));

            break;
        }
        case MFNR_FULL_MONO_MODE:
        {
            mfnr_full_mono_mode_t para = mfnr_request->u.full_mono_mode_param;

            cfg_tab->ghost_detect_y.to_use = 1;
            cfg_tab->ghost_detect_y.ghost_detect_cfg.ghost_mask_th    = para.ghost_detection_y.ghost_detect_cfg.ghost_mask_th;
            if(stripe_cnt == 1)
            {
                cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 0;
                cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4;
                cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 0;
                cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width;
            }
            else
            {
                if(current_stripe == 0)
                {
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 0;
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4 -48;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 0;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width - 16;
                }
                else if(current_stripe == stripe_cnt - 1)
                {
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 48;
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 16;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width;
                }
                else
                {
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 48;
                    cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = cfg_tab->basic_cfg.mfnr_size.width * 4 - 48;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 16;
                    cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = cfg_tab->basic_cfg.mfnr_size.width - 16;
                }
            }

            cfg_tab->temporal_fusion.to_use = 1;
            cfg_tab->temporal_fusion.tempo_fusion.weight          = para.temporal_fusion_yuv.tempo_fusion.weight;
            cfg_tab->temporal_fusion.tempo_fusion.ghost_region_th = para.temporal_fusion_yuv.tempo_fusion.ghost_region_th;
            cfg_tab->temporal_fusion.tempo_fusion.subscale        = para.temporal_fusion_yuv.tempo_fusion.subscale;

            cfg_tab->temporal_fusion.dghost_sigma_stren.max_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.max_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.min_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.min_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.judge_grad_region_th      = para.temporal_fusion_yuv.dghost_sigma_stren.judge_grad_region_th;
            cfg_tab->temporal_fusion.dghost_sigma_stren.sigma                     = para.temporal_fusion_yuv.dghost_sigma_stren.sigma;

            cfg_tab->temporal_fusion.dghost_blend_stren.max_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.max_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.min_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.min_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_mean2blend_th         = para.temporal_fusion_yuv.dghost_blend_stren.sup_mean2blend_th;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_blend_scale           = para.temporal_fusion_yuv.dghost_blend_stren.sup_blend_scale;

            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int));// unsafe_function_ignore: memcpy

            break;
        }
        case MFNR_PARTIAL_COLOR_MODE:
        {
            mfnr_partial_color_mode_t para = mfnr_request->u.partial_color_mode_param;
            cfg_tab->temporal_fusion.to_use = 1;

            cfg_tab->ghost_detect_y.to_use = 1;
            cfg_tab->ghost_detect_y.ghost_detect_cfg.ghost_mask_th    = 0;
            cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 0;
            cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = 0;
            cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 0;
            cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = 0;

            cfg_tab->temporal_fusion.tempo_fusion.weight          = para.temporal_fusion_yuv.tempo_fusion.weight;
            cfg_tab->temporal_fusion.tempo_fusion.ghost_region_th = para.temporal_fusion_yuv.tempo_fusion.ghost_region_th;
            cfg_tab->temporal_fusion.tempo_fusion.subscale        = para.temporal_fusion_yuv.tempo_fusion.subscale;

            cfg_tab->temporal_fusion.dghost_sigma_stren.max_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.max_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.min_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.min_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.judge_grad_region_th      = para.temporal_fusion_yuv.dghost_sigma_stren.judge_grad_region_th;
            cfg_tab->temporal_fusion.dghost_sigma_stren.sigma                     = para.temporal_fusion_yuv.dghost_sigma_stren.sigma;

            cfg_tab->temporal_fusion.dghost_blend_stren.max_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.max_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.min_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.min_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_mean2blend_th         = para.temporal_fusion_yuv.dghost_blend_stren.sup_mean2blend_th;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_blend_scale           = para.temporal_fusion_yuv.dghost_blend_stren.sup_blend_scale;

            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2sigma_lut,MFNR_MEAN2SIGMA*sizeof(unsigned int), para.temporal_fusion_yuv.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int));// unsafe_function_ignore: memcpy

            break;
        }
        case MFNR_PARTIAL_MONO_MODE:
        {
            mfnr_partial_mono_mode_t para = mfnr_request->u.partial_mono_mode_param;

            cfg_tab->ghost_detect_y.to_use = 1;
            cfg_tab->ghost_detect_y.ghost_detect_cfg.ghost_mask_th    = 0;
            cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_start   = 0;
            cfg_tab->ghost_detect_y.us16_crop_cfg.us16_crop_h_end     = 0;
            cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_start = 0;
            cfg_tab->ghost_detect_y.stats_crop_cfg.stats_crop_h_end   = 0;

            cfg_tab->temporal_fusion.to_use = 1;
            cfg_tab->temporal_fusion.tempo_fusion.weight          = para.temporal_fusion_yuv.tempo_fusion.weight;
            cfg_tab->temporal_fusion.tempo_fusion.ghost_region_th = para.temporal_fusion_yuv.tempo_fusion.ghost_region_th;
            cfg_tab->temporal_fusion.tempo_fusion.subscale        = para.temporal_fusion_yuv.tempo_fusion.subscale;

            cfg_tab->temporal_fusion.dghost_sigma_stren.max_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.max_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.min_grad2sigma_scale_bkpt = para.temporal_fusion_yuv.dghost_sigma_stren.min_grad2sigma_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_sigma_stren.judge_grad_region_th      = para.temporal_fusion_yuv.dghost_sigma_stren.judge_grad_region_th;
            cfg_tab->temporal_fusion.dghost_sigma_stren.sigma                     = para.temporal_fusion_yuv.dghost_sigma_stren.sigma;

            cfg_tab->temporal_fusion.dghost_blend_stren.max_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.max_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.min_grad2blend_scale_bkpt = para.temporal_fusion_yuv.dghost_blend_stren.min_grad2blend_scale_bkpt;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_mean2blend_th         = para.temporal_fusion_yuv.dghost_blend_stren.sup_mean2blend_th;
            cfg_tab->temporal_fusion.dghost_blend_stren.sup_blend_scale           = para.temporal_fusion_yuv.dghost_blend_stren.sup_blend_scale;

            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_mean2sigma_lut, MFNR_MEAN2SIGMA*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2sigma_lut, MFNR_GRAD2SIGMA*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_mean2blend_lut,MFNR_MEAN2BLEND*sizeof(unsigned int), para.temporal_fusion_yuv.dghost_mean2blend_lut, MFNR_MEAN2BLEND*sizeof(unsigned int));// unsafe_function_ignore: memcpy
            memcpy_s(cfg_tab->temporal_fusion.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int),para.temporal_fusion_yuv.dghost_grad2blend_lut, MFNR_GRAD2BLEND*sizeof(unsigned int));// unsafe_function_ignore: memcpy

            break;
        }
        default:
            break;
    }
    D("[%s] -\n", __func__);
    
    return CPE_FW_OK;
}

static void mfnr_update_cmdlst_cfg_tab(cfg_tab_cmdlst_t* cmdlst_cfg, unsigned int channel_id, schedule_cmdlst_link_t* cmd_link_entry)
{
    D("[%s] +\n", __func__);

    cmdlst_cfg->cfg.to_use = 1;
    cmdlst_cfg->cfg.prefetch       = 0;
    cmdlst_cfg->cfg.slowdown_nrt_channel       = 0;

    cmdlst_cfg->ch_cfg.to_use = 1;
    cmdlst_cfg->ch_cfg.nrt_channel = 1;
    cmdlst_cfg->ch_cfg.active_token_nbr_en = 0;
    cmdlst_cfg->ch_cfg.active_token_nbr = 0;

    cmdlst_cfg->sw_ch_mngr_cfg.to_use = 1;
    cmdlst_cfg->sw_ch_mngr_cfg.sw_link_channel = 0;
    cmdlst_cfg->sw_ch_mngr_cfg.sw_link_token_nbr = 0;
    cmdlst_cfg->sw_ch_mngr_cfg.sw_ressource_request = 0;
    cmdlst_cfg->sw_ch_mngr_cfg.sw_priority = 0;

    cmdlst_cfg->vp_rd_cfg.to_use= 1;
    cmdlst_cfg->vp_rd_cfg.vp_rd_id = channel_id;
    cmdlst_cfg->vp_rd_cfg.rd_addr  = cmd_link_entry->cmd_buf.start_addr_isp_map;
    cmdlst_cfg->vp_rd_cfg.rd_size  = cmd_link_entry->cmd_buf.data_size;

    D("[%s] -\n", __func__);
}

static void mfnr_set_cmdlst_stripe(cmdlst_stripe_info_t* stripe, unsigned int stripe_cnt)
{
    D("[%s] +\n", __func__);

    unsigned int i = 0;
    unsigned int mfnr_stat_addr = 0;
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = 0;

    if ((ret = cpe_mem_get(MEM_ID_MFNR_STAT, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_MFNR_STAT, va, da, ret);
        return;
    }
    mfnr_stat_addr = da;

    for(i = 0; i < stripe_cnt; i++)
    {
        if(g_mfnr_read_stat_flag && i % 2 != 0)
        {
            stripe[i].rd_cfg_info.fs           = mfnr_stat_addr + 0x20 * (i - 1) /2;
            stripe[i].rd_cfg_info.rd_cfg_num   = 1;
            stripe[i].rd_cfg_info.rd_cfg[0]    = 0x0300B271;
        }
        else
        {
            stripe[i].rd_cfg_info.fs = 0;
        }
        stripe[i].is_last_stripe   = 0;
    }
    stripe[stripe_cnt-1].is_last_stripe = 1;

    D("[%s] -\n", __func__);
}

static int mfnr_set_irq_mode(msg_req_mfnr_request_t* mfnr_request, schedule_cmdlst_link_t* cmd_link_entry,unsigned int stripe_cnt)
{
    D("[%s] +\n", __func__);
    cpe_cmdlst_irq_mode module_irq_mode = CMD_IRQ_MAX;
    unsigned int i = 0;

    if(mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_COLOR_MODE)
    {
        if(mfnr_request->mfnr_cfg.ghost_mask_wr_en == 1)
        {
           module_irq_mode = CMD_IRQ_MCF_MFNR_FULL_MODE;
        }
        else
        {
            module_irq_mode = CMD_IRQ_MFNR_FULL_COLOR_NO_MASK_MODE;
        }
    }
    else if(mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_MONO_MODE)
    {
        if(mfnr_request->mfnr_cfg.ghost_mask_wr_en == 1)
        {
            module_irq_mode = CMD_IRQ_MFNR_FULL_MONO_MODE;
        }
        else
        {
            module_irq_mode = CMD_IRQ_MFNR_FULL_MONO_NO_MASK_MODE;
        }
    }
    else if(mfnr_request->mfnr_cfg.op_mode == MFNR_PARTIAL_COLOR_MODE)
    {
        module_irq_mode = CMD_IRQ_MFNR_PARTIAL_COLOR_MODE;
    }
    else if(mfnr_request->mfnr_cfg.op_mode == MFNR_PARTIAL_MONO_MODE)
    {
        module_irq_mode = CMD_IRQ_MFNR_PARTIAL_MONO_MODE;
    }
    else
    {
        pr_err("MFNR invalid mode!");
    }

    for(i = 0;i < stripe_cnt; i++)
    {
        if(g_mfnr_read_stat_flag && i % 2 != 0)
        {
            cmd_link_entry[i].irq_mode = CMD_IRQ_MFNR_READ_STAT_MODE;
        }
        else
        {
            cmd_link_entry[i].irq_mode = module_irq_mode;
        }
    }
    
    D("[%s] -\n", __func__);
    return 0;
}

int mfnr_request_handler(msg_req_mfnr_request_t* mfnr_request)
{
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = CPE_FW_OK;

    D("[%s] +\n", __func__);
    
    if (mfnr_request == NULL) {
        pr_err("[%s] failed : mfnr_request is null", __func__);
        return -1;
    }

    mfnr_dump_request(mfnr_request);
    hispcpe_reg_set(CPE_TOP,0x240,1);

    unsigned int is_full_mode = (mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_COLOR_MODE || mfnr_request->mfnr_cfg.op_mode == MFNR_FULL_MONO_MODE ) ? 1 : 0;
    isp_stripe_info_t* stripe_mfnr_origin = NULL;
    isp_stripe_info_t* stripe_mfnr_ds4 = NULL;
    isp_stripe_info_t* stripe_mfnr_ds16 =NULL;
    isp_stripe_info_t* stripe_mfnr_gm = NULL;
    stripe_mfnr_origin= (isp_stripe_info_t*)kmalloc(sizeof(isp_stripe_info_t), GFP_KERNEL);
    if (NULL == stripe_mfnr_origin)
    {
        pr_err(" fail to kmalloc stripe_mfnr_origin\n");
	goto fail_get_mem;
    }

    stripe_mfnr_ds4 = (isp_stripe_info_t*)kmalloc(sizeof(isp_stripe_info_t), GFP_KERNEL);
    if (NULL == stripe_mfnr_ds4)
    {
        pr_err(" fail to kmalloc stripe_mfnr_ds4\n");
	goto fail_get_mem;
    }
    
    stripe_mfnr_ds16 = (isp_stripe_info_t*)kmalloc(sizeof(isp_stripe_info_t), GFP_KERNEL);
    if (NULL == stripe_mfnr_ds16)
    {
        pr_err(" fail to kmalloc stripe_mfnr_ds16\n");
	goto fail_get_mem;
    }
   
    stripe_mfnr_gm = (isp_stripe_info_t*)kmalloc(sizeof(isp_stripe_info_t), GFP_KERNEL);
    if (NULL == stripe_mfnr_gm)
    {
        pr_err(" fail to kmalloc stripe_mfnr_gm\n");
	goto fail_get_mem;
    }

    mfnr_calc_stripe_info(is_full_mode, &mfnr_request->streams[BI_MFNR_CURR_Y], stripe_mfnr_origin, stripe_mfnr_ds4, stripe_mfnr_ds16,stripe_mfnr_gm);
    
    unsigned int stripe_cnt = stripe_mfnr_origin->stripe_cnt;
    if ((ret = cpe_mem_get(MEM_ID_MFNR_CFG_TAB, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_MFNR_CFG_TAB, va, da, ret);
       goto  fail_get_mem;
    }
    mfnr_config_table_t* mfnr_cfg_tab = (mfnr_config_table_t*)va;
    if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_MM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CVDR_CFG_TAB_MM, va, da, ret);
        goto  fail_get_mem;
    }
    cfg_tab_cvdr_t* cvdr_cfg_tab = (cfg_tab_cvdr_t*)va;
    if ((ret = cpe_mem_get(MEM_ID_CPETOP_CFG_TAB_MM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CPETOP_CFG_TAB_MM, va, da, ret);
        goto  fail_get_mem;
    }
    cpe_top_config_table_t* top_cfg_tab = (cpe_top_config_table_t*)va;
    if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_MM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_PARA_MM, va, da, ret);
        goto  fail_get_mem;
    }
    cmdlst_para_t* cmdlst_para = (cmdlst_para_t*)va;

    unsigned int i = 0;
    for(i = 0;i < stripe_cnt;i++)
    {
        if(is_full_mode)
        {
            mfnr_update_mfnr_cfg_tab(i, stripe_cnt, mfnr_request, &mfnr_cfg_tab[i], stripe_mfnr_ds16->stripe_width[i]);
            g_mfnr_read_stat_flag = 1;
        }
        else
        {
            mfnr_update_mfnr_cfg_tab(i, stripe_cnt,mfnr_request, &mfnr_cfg_tab[i], stripe_mfnr_origin->stripe_width[i]);
        }
        mfnr_update_cvdr_wr_cfg_tab(mfnr_request, &cvdr_cfg_tab[i], stripe_mfnr_origin, stripe_mfnr_ds4, stripe_mfnr_ds16, stripe_mfnr_gm, i);
        mfnr_update_cvdr_rd_cfg_tab(mfnr_request, &cvdr_cfg_tab[i], stripe_mfnr_origin, stripe_mfnr_ds4, stripe_mfnr_ds16, stripe_mfnr_gm, i);
        mfnr_update_top_cfg_tab(mfnr_request, &top_cfg_tab[i], stripe_mfnr_origin, stripe_mfnr_ds4, stripe_mfnr_gm, i);
    }


    cmdlst_para->channel_id = 0;

    if(g_mfnr_read_stat_flag == 1)
    {
        cmdlst_para->stripe_cnt = stripe_cnt * 2;
    }
    else
    {
        cmdlst_para->stripe_cnt = stripe_cnt;
    }


    mfnr_set_cmdlst_stripe(&cmdlst_para->cmd_stripe_info[0],cmdlst_para->stripe_cnt);
    df_sched_prepare(cmdlst_para);

    schedule_cmdlst_link_t* cmd_link_entry = (schedule_cmdlst_link_t*)cmdlst_para->cmd_entry;
    mfnr_set_irq_mode(mfnr_request,cmd_link_entry,cmdlst_para->stripe_cnt);
    df_sched_set_buffer_header(cmdlst_para);

    for(i = 0;i < cmdlst_para->stripe_cnt; i++)
    {
        if(g_mfnr_read_stat_flag)
        {
            if(i % 2 != 0)
            {
                continue;
            }
            else
            {
                mfnr_prepare_cmd(&g_mfnr_devs[0], &cmd_link_entry[i].cmd_buf, &mfnr_cfg_tab[i/2]);
                cpe_top_prepare_cmd(&g_cpe_top_devs[0], &cmd_link_entry[i].cmd_buf, &top_cfg_tab[i/2]);
                cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &cvdr_cfg_tab[i/2]);
                cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
            }
        }
        else
        {
                mfnr_prepare_cmd(&g_mfnr_devs[0], &cmd_link_entry[i].cmd_buf, &mfnr_cfg_tab[i]);
                cpe_top_prepare_cmd(&g_cpe_top_devs[0], &cmd_link_entry[i].cmd_buf, &top_cfg_tab[i]);
                cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &cvdr_cfg_tab[i]);
                cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
        }
    }

    cfg_tab_cmdlst_t cmdlst_cfg;
    mfnr_update_cmdlst_cfg_tab(&cmdlst_cfg, cmdlst_para->channel_id, &cmd_link_entry[0]);
    
    df_sched_start(&cmdlst_cfg,cmdlst_para);

    cpe_mem_free(MEM_ID_MFNR_CFG_TAB);
    cpe_mem_free(MEM_ID_CVDR_CFG_TAB_MM);
    cpe_mem_free(MEM_ID_CPETOP_CFG_TAB_MM);

    fail_get_mem:
    if (stripe_mfnr_origin){
        kfree(stripe_mfnr_origin);
	stripe_mfnr_origin = NULL;
    }
    if(stripe_mfnr_ds4){
        kfree(stripe_mfnr_ds4);
	stripe_mfnr_ds4 = NULL;
    }
    if (stripe_mfnr_ds16){
        kfree(stripe_mfnr_ds16);
	stripe_mfnr_ds16 =NULL;
    }
    if (stripe_mfnr_gm){
        kfree(stripe_mfnr_gm);
	stripe_mfnr_gm =NULL;
    }

    D("[%s] -\n", __func__);
    
    return ret;
}

int mfnr_eof_handler()
{
    D("[%s] +\n", __func__);
    cmdlst_eop_handler(CMD_EOF_MFNR_MODE);


    cpe_mem_free(MEM_ID_MFNR_STAT);
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

static int mfnr_dump_request(msg_req_mfnr_request_t* req)
{
    D("[%s] +\n", __func__);
    int i=0;
    pr_info("size of msg_req_mfnr_request_t = %d\n", (int)(sizeof(msg_req_mfnr_request_t)));
    pr_info("frame_number = %d\n", req->frame_number);
    pr_info("mfnr_cfg.mode = %d\n", req->mfnr_cfg.op_mode);
    pr_info("mfnr_cfg.uv_unpack = %d\n", req->mfnr_cfg.uv_unpack);
    pr_info("mfnr_cfg.uv_pack = %d\n", req->mfnr_cfg.uv_pack);
    pr_info("mfnr_cfg.ghost_mask_wr_en = %d\n", req->mfnr_cfg.ghost_mask_wr_en);
    pr_info("mfnr_cfg.ghost_stats_crop_h_en = %d\n", req->mfnr_cfg.ghost_stats_crop_h_en);
    for(i = 0; i < MFNR_STREAM_MAX; i++)
    {
    pr_info("streams[%d].width+height+buf+format=  %d  %d  %d  %d\n",
         i, req->streams[i].width, req->streams[i].height, req->streams[i].buf, req->streams[i].format);
    }

    if(req->mfnr_cfg.op_mode == MFNR_FULL_COLOR_MODE)
    {
        pr_info("ghost_detect_y_t:ghost_mask_th+us16_crop_h_start+us16_crop_h_end: %d  %d  %d\n",
            req->u.full_color_mode_param.ghost_detection_y.ghost_detect_cfg.ghost_mask_th,
            req->u.full_color_mode_param.ghost_detection_y.us16_crop_cfg.us16_crop_h_start,
            req->u.full_color_mode_param.ghost_detection_y.us16_crop_cfg.us16_crop_h_end);
        pr_info("temporal_fusion_yuv_t:weight+ghost_region_th+subscale: %d  %d  %d",
            req->u.full_color_mode_param.temporal_fusion_yuv.tempo_fusion.weight,
            req->u.full_color_mode_param.temporal_fusion_yuv.tempo_fusion.ghost_region_th,
            req->u.full_color_mode_param.temporal_fusion_yuv.tempo_fusion.subscale);
    }
    else if(req->mfnr_cfg.op_mode == MFNR_FULL_MONO_MODE)
    {
        pr_info("ghost_detect_y_t:ghost_mask_th+us16_crop_h_start+us16_crop_h_end: %d  %d  %d\n",
            req->u.full_mono_mode_param.ghost_detection_y.ghost_detect_cfg.ghost_mask_th,
            req->u.full_mono_mode_param.ghost_detection_y.us16_crop_cfg.us16_crop_h_start,
            req->u.full_mono_mode_param.ghost_detection_y.us16_crop_cfg.us16_crop_h_end);
        pr_info("temporal_fusion_yuv_t:weight+ghost_region_th+subscale: %d  %d  %d\n",
            req->u.full_mono_mode_param.temporal_fusion_yuv.tempo_fusion.weight,
            req->u.full_mono_mode_param.temporal_fusion_yuv.tempo_fusion.ghost_region_th,
            req->u.full_mono_mode_param.temporal_fusion_yuv.tempo_fusion.subscale);
    }
    else if(req->mfnr_cfg.op_mode == MFNR_PARTIAL_COLOR_MODE)
    {
        pr_info("temporal_fusion_yuv_t:weight+ghost_region_th+subscale: %d  %d  %d\n",
            req->u.partial_color_mode_param.temporal_fusion_yuv.tempo_fusion.weight,
            req->u.partial_color_mode_param.temporal_fusion_yuv.tempo_fusion.ghost_region_th,
            req->u.partial_color_mode_param.temporal_fusion_yuv.tempo_fusion.subscale);
    }
    else if(req->mfnr_cfg.op_mode == MFNR_PARTIAL_MONO_MODE)
    {
        pr_info("temporal_fusion_yuv_t:weight+ghost_region_th+subscale: %d  %d  %d\n",
            req->u.partial_mono_mode_param.temporal_fusion_yuv.tempo_fusion.weight,
            req->u.partial_mono_mode_param.temporal_fusion_yuv.tempo_fusion.ghost_region_th,
            req->u.partial_mono_mode_param.temporal_fusion_yuv.tempo_fusion.subscale);
    }
    else
    {
        pr_err("Invalid mode input for MFNR!");
    }

    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

static int mfnr_calc_stripe_info(unsigned int is_full_mode,cpe_stream_t* stream, isp_stripe_info_t* stripe_mfnr_origin, isp_stripe_info_t* stripe_mfnr_ds4, isp_stripe_info_t* stripe_mfnr_ds16, isp_stripe_info_t* stripe_mfnr_gm)
{
    D("[%s] +\n", __func__);
    unsigned int i             = 0;
    unsigned int overlap       = is_full_mode ? 64 : 16;
    unsigned int constrain_cnt = 1;
    df_size_constrain_t  p_size_constrain;
    isp_stripe_info_t stripe_info;

    p_size_constrain.hinc      = 1*65536;
    p_size_constrain.pix_align = is_full_mode ? ALIGN_MFNR_FULL : ALIGN_MFNR_PARTIAL;
    p_size_constrain.out_width = 8912;
    pr_info("overlap =%d mono_width =%d \n", overlap, stream->width);

    df_size_split_stripe(constrain_cnt, &p_size_constrain, &stripe_info, overlap, stream->width);////

    stripe_mfnr_origin->stripe_cnt = stripe_info.stripe_cnt;
    stripe_mfnr_ds4->stripe_cnt = stripe_info.stripe_cnt;
    stripe_mfnr_ds16->stripe_cnt   = stripe_info.stripe_cnt;
    stripe_mfnr_gm->stripe_cnt   = stripe_info.stripe_cnt;
    
    if(1 == is_full_mode)
    {
        stripe_mfnr_origin->full_size.width = stream->width;
        stripe_mfnr_origin->full_size.height = stream->height;
        stripe_mfnr_ds16->full_size.width = stream->width / 4;
        stripe_mfnr_ds16->full_size.height = stream->height / 4;
        stripe_mfnr_ds4->full_size.width = stream->width / 2;
        stripe_mfnr_ds4->full_size.height = stream->height / 2;
        stripe_mfnr_gm->full_size.width = stream->width / 8;
        stripe_mfnr_gm->full_size.height = stream->height;
        
        for(i=0; i<stripe_info.stripe_cnt;i++)
        {
            stripe_mfnr_ds16->stripe_width[i]       = stripe_info.stripe_width[i]/4;
            stripe_mfnr_ds16->stripe_start_point[i] = stripe_info.stripe_start_point[i]/4;
            stripe_mfnr_ds16->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/4;
            stripe_mfnr_ds16->overlap_left[i]       = stripe_info.overlap_left[i]/4;
            stripe_mfnr_ds16->overlap_right[i]      = stripe_info.overlap_right[i]/4;

            if(stripe_info.stripe_cnt == 1)
            {
                
                stripe_mfnr_origin->stripe_width[i]       = stripe_info.stripe_width[i];
                stripe_mfnr_origin->stripe_start_point[i] = stripe_info.stripe_start_point[i];
                stripe_mfnr_origin->stripe_end_point[i]   = stripe_info.stripe_end_point[i];
                stripe_mfnr_origin->overlap_left[i]       = stripe_info.overlap_left[i];
                stripe_mfnr_origin->overlap_right[i]      = stripe_info.overlap_right[i];

                stripe_mfnr_ds4->stripe_width[i]       = stripe_info.stripe_width[i]/2;
                stripe_mfnr_ds4->stripe_start_point[i] = stripe_info.stripe_start_point[i]/2;
                stripe_mfnr_ds4->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/2;
                stripe_mfnr_ds4->overlap_left[i]       = stripe_info.overlap_left[i]/2;
                stripe_mfnr_ds4->overlap_right[i]      = stripe_info.overlap_right[i]/2;

                stripe_mfnr_gm->stripe_width[i]       = stripe_info.stripe_width[i]/8;
                stripe_mfnr_gm->stripe_start_point[i] = stripe_info.stripe_start_point[i]/8;
                stripe_mfnr_gm->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/8;
                stripe_mfnr_gm->overlap_left[i]       = stripe_info.overlap_left[i]/8;
                stripe_mfnr_gm->overlap_right[i]      = stripe_info.overlap_right[i]/8;
            }
            else
            {
                if(i == 0)
                {
                    stripe_mfnr_origin->stripe_width[i]       = stripe_info.stripe_width[i] - 48;
                    stripe_mfnr_origin->stripe_start_point[i] = 0;
                    stripe_mfnr_origin->stripe_end_point[i]   = stripe_info.stripe_end_point[i] -48;
                    stripe_mfnr_origin->overlap_left[i]       = 0;
                    stripe_mfnr_origin->overlap_right[i]      = stripe_info.overlap_right[i] - 48;

                    stripe_mfnr_ds4->stripe_width[i]       = stripe_info.stripe_width[i]/2 - 24;
                    stripe_mfnr_ds4->stripe_start_point[i] = 0;
                    stripe_mfnr_ds4->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/2 -24;
                    stripe_mfnr_ds4->overlap_left[i]       = 0;
                    stripe_mfnr_ds4->overlap_right[i]      = stripe_info.overlap_right[i]/2 - 24;

                    stripe_mfnr_gm->stripe_width[i]       = stripe_info.stripe_width[i]/8 - 6;
                    stripe_mfnr_gm->stripe_start_point[i] = 0;
                    stripe_mfnr_gm->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/8 - 6;
                    stripe_mfnr_gm->overlap_left[i]       = 0;
                    stripe_mfnr_gm->overlap_right[i]      = stripe_info.overlap_right[i]/8 - 6;

                }
                else if(i == stripe_info.stripe_cnt-1)
                {
                    stripe_mfnr_origin->stripe_width[i]       = stripe_info.stripe_width[i] - 48;
                    stripe_mfnr_origin->stripe_start_point[i] = stripe_info.stripe_start_point[i] +48;
                    stripe_mfnr_origin->stripe_end_point[i]   = stripe_info.stripe_end_point[i];
                    stripe_mfnr_origin->overlap_left[i]       = stripe_info.overlap_left[i] -48;
                    stripe_mfnr_origin->overlap_right[i]      = 0;

                    stripe_mfnr_ds4->stripe_width[i]       = stripe_info.stripe_width[i]/2 - 24;
                    stripe_mfnr_ds4->stripe_start_point[i] = stripe_info.stripe_start_point[i]/2 +24;
                    stripe_mfnr_ds4->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/2;
                    stripe_mfnr_ds4->overlap_left[i]       = stripe_info.overlap_left[i]/2 -24;
                    stripe_mfnr_ds4->overlap_right[i]      = 0;
                
                    stripe_mfnr_gm->stripe_width[i]       = stripe_info.stripe_width[i]/8 - 6;
                    stripe_mfnr_gm->stripe_start_point[i] = stripe_info.stripe_start_point[i]/8 + 6;
                    stripe_mfnr_gm->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/8;
                    stripe_mfnr_gm->overlap_left[i]       = stripe_info.overlap_left[i]/8 - 6;
                    stripe_mfnr_gm->overlap_right[i]      = 0;
                }
                else
                {
                    stripe_mfnr_origin->stripe_width[i]       = stripe_info.stripe_width[i] - 2*48;
                    stripe_mfnr_origin->stripe_start_point[i] = stripe_info.stripe_start_point[i] +48;
                    stripe_mfnr_origin->stripe_end_point[i]   = stripe_info.stripe_end_point[i] -48;
                    stripe_mfnr_origin->overlap_left[i]       = stripe_info.overlap_left[i] -48;
                    stripe_mfnr_origin->overlap_right[i]      = stripe_info.overlap_right[i] - 48;

                    stripe_mfnr_ds4->stripe_width[i]       = stripe_info.stripe_width[i]/2 - 2*24;
                    stripe_mfnr_ds4->stripe_start_point[i] = stripe_info.stripe_start_point[i]/2 +24;
                    stripe_mfnr_ds4->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/2 -24;
                    stripe_mfnr_ds4->overlap_left[i]       = stripe_info.overlap_left[i]/2 -24;
                    stripe_mfnr_ds4->overlap_right[i]      = stripe_info.overlap_right[i]/2 - 24;

                    stripe_mfnr_gm->stripe_width[i]       = stripe_info.stripe_width[i]/8 - 2*6;
                    stripe_mfnr_gm->stripe_start_point[i] = stripe_info.stripe_start_point[i]/8 + 6;
                    stripe_mfnr_gm->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/8 - 6;
                    stripe_mfnr_gm->overlap_left[i]       = stripe_info.overlap_left[i]/8 - 6;
                    stripe_mfnr_gm->overlap_right[i]      = stripe_info.overlap_right[i]/8 - 6;
                }

            }
        }
        df_size_dump_stripe_info(stripe_mfnr_ds16, "stripe_mfnr_ds16");
        df_size_dump_stripe_info(stripe_mfnr_gm, "stripe_mfnr_gm");
    }
    else
    {
        stripe_mfnr_origin->full_size.width = stream->width;
        stripe_mfnr_origin->full_size.height = stream->height;
        stripe_mfnr_ds4->full_size.width = stream->width / 2;
        stripe_mfnr_ds4->full_size.height = stream->height / 2;
        for(i=0; i<stripe_info.stripe_cnt;i++)
        {
            stripe_mfnr_origin->stripe_width[i]       = stripe_info.stripe_width[i];
            stripe_mfnr_origin->stripe_start_point[i] = stripe_info.stripe_start_point[i];
            stripe_mfnr_origin->stripe_end_point[i]   = stripe_info.stripe_end_point[i];
            stripe_mfnr_origin->overlap_left[i]       = stripe_info.overlap_left[i];
            stripe_mfnr_origin->overlap_right[i]      = stripe_info.overlap_right[i];

            stripe_mfnr_ds4->stripe_width[i]       = stripe_info.stripe_width[i]/2;
            stripe_mfnr_ds4->stripe_start_point[i] = stripe_info.stripe_start_point[i]/2;
            stripe_mfnr_ds4->stripe_end_point[i]   = stripe_info.stripe_end_point[i]/2;
            stripe_mfnr_ds4->overlap_left[i]       = stripe_info.overlap_left[i]/2;
            stripe_mfnr_ds4->overlap_right[i]      = stripe_info.overlap_right[i]/2;
        }
    }

    df_size_dump_stripe_info(stripe_mfnr_origin, "stripe_mfnr_origin");
    df_size_dump_stripe_info(stripe_mfnr_ds4, "stripe_mfnr_ds4");
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

/*lint +e429 +e613 +e668*/


#pragma GCC diagnostic pop

