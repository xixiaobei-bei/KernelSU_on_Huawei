#include <linux/printk.h>
#include <linux/slab.h>
#include "mcf.h"
#include "cfg_table_mcf.h"
#include "cvdr_drv.h"
#include "mcf_drv.h"
#include "ipp_top_drv.h"
#include "cvdr_opt.h"
#include "adapter_common.h"
#include "memory.h"

#define LOG_TAG LOG_MODULE_MCF

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
/*lint -e429  -e613  -e668*/

extern mcf_dev_t g_mcf_devs[];
static int mcf_dump_request(msg_req_mcf_request_t* req);
static int mcf_calc_stripe_info(unsigned int is_ygf_mode,cpe_stream_t* stream, isp_stripe_info_t* p_stripe_info, isp_stripe_info_t* p_stripe_info_DS4);

static int mcf_update_top_cfg_tab(msg_req_mcf_request_t* mcf_request, cpe_top_config_table_t* cfg_tab, isp_stripe_info_t* stripe_info, isp_stripe_info_t* stripe_info_DS4, unsigned int stripe_index)
{
    D("[%s] +\n", __func__);
    cfg_tab->to_use = 1;
    cfg_tab->crop_vpwr.to_use = 1;

    if(mcf_request->streams[BO_RESULT_Y].buf)
    {
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft = stripe_info->overlap_left[stripe_index];
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright = stripe_info->stripe_width[stripe_index] - stripe_info->overlap_right[stripe_index];
    }
    else
    {
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft = 0;
        cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright = CROP_DEFAULT_VALUE;
    }

    if(mcf_request->streams[BO_RESULT_UV].buf)
    {
        if(mcf_request->mcf_cfg.mode == MCF_FULL_MODE || mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE)
        {
            cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = stripe_info_DS4->overlap_left[stripe_index];
            cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
        }
        else
        {
            cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = stripe_info->overlap_left[stripe_index];
            cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = stripe_info->stripe_width[stripe_index] - stripe_info->overlap_right[stripe_index];
        }
    }
    else
    {
        cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft = 0;
        cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright = CROP_DEFAULT_VALUE;
    }

    if(mcf_request->streams[BO_RESULT_C].buf)
    {
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft = stripe_info_DS4->overlap_left[stripe_index];
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright = stripe_info_DS4->stripe_width[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
    }
    else
    {
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft = 0;
        cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright = CROP_DEFAULT_VALUE;
    }

    pr_info("crop0 = %d,%d,crop1 = %d,%d,crop2 = %d,%d",cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihleft,cfg_tab->crop_vpwr.crop_vpwr_0.vpwr_ihright
        ,cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihleft,cfg_tab->crop_vpwr.crop_vpwr_1.vpwr_ihright
        ,cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihleft,cfg_tab->crop_vpwr.crop_vpwr_2.vpwr_ihright);
    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}



static int mcf_update_cvdr_rd_cfg_tab(msg_req_mcf_request_t* mcf_request, cfg_tab_cvdr_t* cfg_tab, isp_stripe_info_t* stripe_info, isp_stripe_info_t* stripe_info_DS4, unsigned int stripe_index)
{
    D("[%s] +\n", __func__);
    cvdr_opt_fmt_t cfmt;
    pix_format_e format = PIXEL_FMT_CPE_MAX;
    cfmt.pix_fmt = DF_FMT_INVALID;

    if(mcf_request->streams[BI_WARP_0_UVC].buf)
    {
        cfmt.id = RD0_L0;
        cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.height = mcf_request->streams[BI_WARP_0_UVC].height;
        cfmt.addr = mcf_request->streams[BI_WARP_0_UVC].buf + stripe_info_DS4->stripe_start_point[stripe_index] * 3;
        cfmt.full_width = stripe_info_DS4->full_size.width;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_WARP_UVC;

        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mcf_request->streams[BI_WARP_1_UVC].buf)
    {
        cfmt.id = RD1_L1;
        cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.height = mcf_request->streams[BI_WARP_1_UVC].height;
        cfmt.addr = mcf_request->streams[BI_WARP_1_UVC].buf + stripe_info_DS4->stripe_start_point[stripe_index] * 3;
        cfmt.full_width = stripe_info_DS4->full_size.width;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_WARP_UVC;
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mcf_request->streams[BI_WARP_2_UVC].buf)
    {
        cfmt.id = RD2_L2;
        cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.height = mcf_request->streams[BI_WARP_2_UVC].height;
        cfmt.addr = mcf_request->streams[BI_WARP_2_UVC].buf + stripe_info_DS4->stripe_start_point[stripe_index] * 3;
        cfmt.full_width = stripe_info_DS4->full_size.width;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_WARP_UVC;
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mcf_request->streams[BI_MONO_DS4].buf)
    {
        cfmt.id = RD3_MONO_DS4;
        cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
        cfmt.height = mcf_request->streams[BI_MONO_DS4].height;
        cfmt.addr = mcf_request->streams[BI_MONO_DS4].buf + stripe_info_DS4->stripe_start_point[stripe_index];
        cfmt.full_width = stripe_info_DS4->full_size.width;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_Y8;
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mcf_request->streams[BI_MONO].buf)
    {
        cfmt.id = RD4_MONO;
        cfmt.width = stripe_info->stripe_width[stripe_index];
        cfmt.line_size = stripe_info->stripe_width[stripe_index];
        cfmt.height = mcf_request->streams[BI_MONO].height;
        cfmt.addr = mcf_request->streams[BI_MONO].buf + stripe_info->stripe_start_point[stripe_index];
        cfmt.full_width = stripe_info->full_size.width;
        cfmt.expand = 0;
        format = PIXEL_FMT_CPE_Y8;
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mcf_request->streams[BI_WARP_COL_Y_DS4].buf)
    {
        cfmt.id = RD5_COL_Y_DS4;
        if(mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE)
        {
            cfmt.width = stripe_info->stripe_width[stripe_index];
            cfmt.line_size = stripe_info->stripe_width[stripe_index];
            cfmt.height = mcf_request->streams[BI_WARP_COL_Y_DS4].height;
            cfmt.addr = mcf_request->streams[BI_WARP_COL_Y_DS4].buf + stripe_info->stripe_start_point[stripe_index];
            cfmt.full_width = stripe_info->full_size.width;
            cfmt.expand = 0;
            format = PIXEL_FMT_CPE_Y8;
        }
        else
        {
            cfmt.width = stripe_info_DS4->stripe_width[stripe_index];
            cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index];
            cfmt.height = mcf_request->streams[BI_WARP_COL_Y_DS4].height;
            cfmt.addr = mcf_request->streams[BI_WARP_COL_Y_DS4].buf + stripe_info_DS4->stripe_start_point[stripe_index];
            cfmt.full_width = stripe_info_DS4->full_size.width;
            cfmt.expand = 0;
            format = PIXEL_FMT_CPE_Y8;
        }
        dataflow_cvdr_rd_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

static int mcf_update_cvdr_wr_cfg_tab(msg_req_mcf_request_t* mcf_request, cfg_tab_cvdr_t* cfg_tab, isp_stripe_info_t* stripe_info, isp_stripe_info_t* stripe_info_DS4, unsigned int stripe_index)
{
    D("[%s] +\n", __func__);
    cvdr_opt_fmt_t cfmt;
    pix_format_e format = PIXEL_FMT_CPE_MAX;
    cfmt.pix_fmt = DF_FMT_INVALID;

    if(mcf_request->streams[BO_RESULT_Y].buf)
    {
        cfmt.id = WR1_Y;
        cfmt.width = stripe_info->stripe_width[stripe_index] -stripe_info->overlap_left[stripe_index] - stripe_info->overlap_right[stripe_index];
        cfmt.line_size = stripe_info->stripe_width[stripe_index] -stripe_info->overlap_left[stripe_index] - stripe_info->overlap_right[stripe_index];
        cfmt.height = mcf_request->streams[BO_RESULT_Y].height;
        cfmt.full_width = stripe_info->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.expand = 0;
        if(mcf_request->mcf_cfg.mode == MCF_FULL_MODE)
        {
            format = PIXEL_FMT_CPE_Y8;
            cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] + stripe_info->overlap_left[stripe_index]);
        }
        else if(mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE)
        {
            format = PIXEL_FMT_CPE_Y8;
            cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] + stripe_info->overlap_left[stripe_index]);
        }
        else if(mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE)
        {
            if(mcf_request->u.y_lf_gf_mode_param.output_mode == A_B)
            {
                format = PIXEL_FMT_CPE_A_B;
                cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] + stripe_info->overlap_left[stripe_index])*2;
            }
            else
            {
                format = PIXEL_FMT_CPE_LF_HF;
                cfmt.addr = mcf_request->streams[BO_RESULT_Y].buf + (stripe_info->stripe_start_point[stripe_index] + stripe_info->overlap_left[stripe_index]);
            }
        }
        dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }


    if(mcf_request->streams[BO_RESULT_UV].buf)
    {
        cfmt.id = WR0_UV;
        cfmt.height = mcf_request->streams[BO_RESULT_UV].height;
        cfmt.expand = 0;
        if(mcf_request->mcf_cfg.mode == MCF_FULL_MODE)
        {
            format = PIXEL_FMT_CPE_PACKED_UV;
            cfmt.width = stripe_info_DS4->stripe_width[stripe_index] -stripe_info_DS4->overlap_left[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
            cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index] -stripe_info_DS4->overlap_left[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
            cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info_DS4->stripe_start_point[stripe_index] + stripe_info_DS4->overlap_left[stripe_index])*2;
            cfmt.full_width = stripe_info_DS4->full_size.width;
        }
        else if(mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE)
        {
            format = PIXEL_FMT_CPE_PACKED_UV;
            cfmt.width = stripe_info_DS4->stripe_width[stripe_index] -stripe_info_DS4->overlap_left[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
            cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index] -stripe_info_DS4->overlap_left[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
            cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info_DS4->stripe_start_point[stripe_index] + stripe_info_DS4->overlap_left[stripe_index])*2;
            cfmt.full_width = stripe_info_DS4->full_size.width;
        }
        else if(mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE)
        {
            cfmt.width = stripe_info->stripe_width[stripe_index] -stripe_info->overlap_left[stripe_index] - stripe_info->overlap_right[stripe_index];
            cfmt.full_width = stripe_info->full_size.width;
            cfmt.line_size = stripe_info->stripe_width[stripe_index] -stripe_info->overlap_left[stripe_index] - stripe_info->overlap_right[stripe_index];
            if(mcf_request->u.y_lf_gf_mode_param.output_mode == A_B)
            {
                format = PIXEL_FMT_CPE_A_B;
                cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info->stripe_start_point[stripe_index] + stripe_info->overlap_left[stripe_index])*2;
            }
            else
            {
                format = PIXEL_FMT_CPE_LF_HF;
                cfmt.addr = mcf_request->streams[BO_RESULT_UV].buf + (stripe_info->stripe_start_point[stripe_index] + stripe_info->overlap_left[stripe_index]);
            }

        }
        dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }

    if(mcf_request->streams[BO_RESULT_C].buf)
    {
        cfmt.id = WR2_C;
        cfmt.width = stripe_info_DS4->stripe_width[stripe_index] -stripe_info_DS4->overlap_left[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
        cfmt.line_size = stripe_info_DS4->stripe_width[stripe_index] -stripe_info_DS4->overlap_left[stripe_index] - stripe_info_DS4->overlap_right[stripe_index];
        cfmt.height = mcf_request->streams[BO_RESULT_C].height;
        cfmt.addr = mcf_request->streams[BO_RESULT_C].buf + (stripe_info_DS4->stripe_start_point[stripe_index] + stripe_info_DS4->overlap_left[stripe_index]);
        cfmt.full_width = stripe_info_DS4->full_size.width;
	cfmt.pix_fmt = DF_FMT_INVALID;
        cfmt.expand = 0;
        if(mcf_request->mcf_cfg.mode == MCF_FULL_MODE)
        {
            format = PIXEL_FMT_CPE_C8;
        }
        dataflow_cvdr_wr_cfg_vp(cfg_tab, &cfmt, 0, CPE_CLK, format);
    }
    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

static int mcf_update_mcf_cfg_tab(msg_req_mcf_request_t* mcf_request, mcf_config_table_t* cfg_tab, unsigned int stripe_width)
{
    D("[%s] +\n", __func__);
    switch(mcf_request->mcf_cfg.mode)
    {
        case MCF_FULL_MODE:
        {
            full_mode_t *para = (full_mode_t *)&(mcf_request->u.full_mode_param);
            cfg_tab->to_use = 1;
            cfg_tab->basic_cfg.uv_pack = mcf_request->mcf_cfg.uv_pack;
            cfg_tab->basic_cfg.op_mode = mcf_request->mcf_cfg.mode;
            cfg_tab->basic_cfg.y_gf_mode.to_use = 1;
            cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_in = DUAL_INPUT;
            cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_out = LF_ONLY;
            cfg_tab->size_cfg.width = stripe_width;
            cfg_tab->size_cfg.height = mcf_request->streams[BI_MONO].height;
            cfg_tab->adjust_y.to_use = 1;
            cfg_tab->adjust_y.adjust_col_gf.to_use = 1;
            cfg_tab->adjust_y.adjust_col_gf.y_gf_eps = para->adjust_y.adjust_col_gf.y_gf_eps;
            cfg_tab->adjust_y.adjust_col_gf.y_gf_radius = para->adjust_y.adjust_col_gf.y_gf_radius;

            cfg_tab->adjust_y.adjust_get_mask.to_use = 1;
            cfg_tab->adjust_y.adjust_get_mask.mask_mult = para->adjust_y.adjust_get_mask.mask_mult;
            cfg_tab->adjust_y.adjust_get_mask.mask_sub = para->adjust_y.adjust_get_mask.mask_sub;

            cfg_tab->adjust_y.adjust_get_ratio.to_use = 1;
            cfg_tab->adjust_y.adjust_get_ratio.ratio_par_up = para->adjust_y.adjust_get_ratio.ratio_par_up;
            cfg_tab->adjust_y.adjust_get_ratio.ratio_par_down = para->adjust_y.adjust_get_ratio.ratio_par_down;

            cfg_tab->mcf_uv.to_use = 1;
            cfg_tab->mcf_uv.fill_conflict_cfg.to_use = 1;
            cfg_tab->mcf_uv.fill_conflict_cfg.conflict_th = para->mcf_uv_cfg.fill_conflict_cfg.conflict_th;
            cfg_tab->mcf_uv.fill_conflict_cfg.luma_th = para->mcf_uv_cfg.fill_conflict_cfg.luma_th;

            cfg_tab->mcf_uv.smooth_blend_cfg.to_use = 1;
            cfg_tab->mcf_uv.smooth_blend_cfg.mask_blend = para->mcf_uv_cfg.smooth_blend_cfg.mask_blend;
            cfg_tab->mcf_uv.smooth_blend_cfg.gauss_radius = para->mcf_uv_cfg.smooth_blend_cfg.gauss_radius;

            cfg_tab->mcf_uv.smooth_blend_gf.to_use = 1;
            cfg_tab->mcf_uv.smooth_blend_gf.uv_gf_eps = para->mcf_uv_cfg.smooth_blend_gf.uv_gf_eps;
            cfg_tab->mcf_uv.smooth_blend_gf.uv_gf_radius = para->mcf_uv_cfg.smooth_blend_gf.uv_gf_radius;

            cfg_tab->uv_fusion.to_use = 1;
            cfg_tab->uv_fusion.get_can.to_use = 1;
            cfg_tab->uv_fusion.get_can.input_cnt = para->uv_fusion.get_can.input_cnt;
            cfg_tab->uv_fusion.get_can.const_value = para->uv_fusion.get_can.const_value;
            cfg_tab->uv_fusion.get_can.th_v = para->uv_fusion.get_can.th_v;
            cfg_tab->uv_fusion.get_can.th_u = para->uv_fusion.get_can.th_u;


            cfg_tab->uv_fusion.get_final_uv.to_use = 1;
            cfg_tab->uv_fusion.get_final_uv.max_v_diff_const_inv = para->uv_fusion.get_final_uv.max_v_diff_const_inv;
            cfg_tab->uv_fusion.get_final_uv.max_u_diff_const_inv = para->uv_fusion.get_final_uv.max_u_diff_const_inv;
            cfg_tab->uv_fusion.get_final_uv.ratio_dist_to_conf = para->uv_fusion.get_final_uv.ratio_dist_to_conf;
            cfg_tab->uv_fusion.get_final_uv.ratio_dist_uv_to_conf = para->uv_fusion.get_final_uv.ratio_dist_uv_to_conf;


            cfg_tab->uv_fusion.get_con.to_use = 1;
            cfg_tab->uv_fusion.get_con.max_diff_inv = para->uv_fusion.get_con.max_diff_inv;
            cfg_tab->uv_fusion.get_con.max_diff = para->uv_fusion.get_con.max_diff;
            cfg_tab->uv_fusion.get_con.diff_hue_shift_ex = para->uv_fusion.get_con.hybrid_diff_hue_shift_ex;
            cfg_tab->uv_fusion.get_con.diff_hue_coff = para->uv_fusion.get_con.hybrid_diff_hue_coff;
            cfg_tab->uv_fusion.get_con.diff_hue_shift = para->uv_fusion.get_con.hybrid_diff_hue_shift;

            break;
        }
        case MCF_PARTIAL_MODE:
        {
            partial_mode_t *para = (partial_mode_t *)&(mcf_request->u.partial_mode_param);
            cfg_tab->to_use = 1;
            cfg_tab->basic_cfg.uv_pack = mcf_request->mcf_cfg.uv_pack;
            cfg_tab->basic_cfg.op_mode = mcf_request->mcf_cfg.mode;
            cfg_tab->basic_cfg.y_gf_mode.to_use = 1;
            cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_in = DUAL_INPUT;
            cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_out = LF_ONLY;
            cfg_tab->size_cfg.width = stripe_width;
            cfg_tab->size_cfg.height = mcf_request->streams[BI_MONO].height;
            cfg_tab->adjust_y.to_use = 1;
            cfg_tab->adjust_y.adjust_col_gf.to_use = 1;
            cfg_tab->adjust_y.adjust_col_gf.y_gf_eps = para->adjust_y.adjust_col_gf.y_gf_eps;
            cfg_tab->adjust_y.adjust_col_gf.y_gf_radius = para->adjust_y.adjust_col_gf.y_gf_radius;

            cfg_tab->adjust_y.adjust_get_mask.to_use = 1;
            cfg_tab->adjust_y.adjust_get_mask.mask_mult = para->adjust_y.adjust_get_mask.mask_mult;
            cfg_tab->adjust_y.adjust_get_mask.mask_sub = para->adjust_y.adjust_get_mask.mask_sub;

            cfg_tab->adjust_y.adjust_get_ratio.to_use = 1;
            cfg_tab->adjust_y.adjust_get_ratio.ratio_par_up = para->adjust_y.adjust_get_ratio.ratio_par_up;
            cfg_tab->adjust_y.adjust_get_ratio.ratio_par_down = para->adjust_y.adjust_get_ratio.ratio_par_down;

            cfg_tab->mcf_uv.to_use = 1;
            cfg_tab->mcf_uv.fill_conflict_cfg.to_use = 1;
            cfg_tab->mcf_uv.fill_conflict_cfg.conflict_th = para->mcf_uv_cfg.fill_conflict_cfg.conflict_th;
            cfg_tab->mcf_uv.fill_conflict_cfg.luma_th = para->mcf_uv_cfg.fill_conflict_cfg.luma_th;

            cfg_tab->mcf_uv.smooth_blend_cfg.to_use = 1;
            cfg_tab->mcf_uv.smooth_blend_cfg.mask_blend = para->mcf_uv_cfg.smooth_blend_cfg.mask_blend;
            cfg_tab->mcf_uv.smooth_blend_cfg.gauss_radius = para->mcf_uv_cfg.smooth_blend_cfg.gauss_radius;

            cfg_tab->mcf_uv.smooth_blend_gf.to_use = 1;
            cfg_tab->mcf_uv.smooth_blend_gf.uv_gf_eps = para->mcf_uv_cfg.smooth_blend_gf.uv_gf_eps;
            cfg_tab->mcf_uv.smooth_blend_gf.uv_gf_radius = para->mcf_uv_cfg.smooth_blend_gf.uv_gf_radius;

            cfg_tab->uv_fusion.to_use = 0;
            break;
        }
        case MCF_Y_GF_MODE:
        {
            y_gf_mode_t *para = (y_gf_mode_t *)&(mcf_request->u.y_lf_gf_mode_param);
            cfg_tab->to_use = 1;
            cfg_tab->basic_cfg.uv_pack = mcf_request->mcf_cfg.uv_pack;
            cfg_tab->basic_cfg.op_mode = mcf_request->mcf_cfg.mode;
            cfg_tab->basic_cfg.y_gf_mode.to_use = 1;
            cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_in = para->input_mode;
            cfg_tab->basic_cfg.y_gf_mode.y_gf_mode_out = para->output_mode;
            cfg_tab->size_cfg.width = stripe_width;
            cfg_tab->size_cfg.height = mcf_request->streams[BI_MONO].height;
            cfg_tab->adjust_y.to_use = 1;
            cfg_tab->adjust_y.adjust_col_gf.to_use = 1;
            cfg_tab->adjust_y.adjust_col_gf.y_gf_eps = para->adjust_y.adjust_col_gf.y_gf_eps;
            cfg_tab->adjust_y.adjust_col_gf.y_gf_radius = para->adjust_y.adjust_col_gf.y_gf_radius;

            cfg_tab->adjust_y.adjust_get_mask.to_use = 1;
            cfg_tab->adjust_y.adjust_get_mask.mask_mult = para->adjust_y.adjust_get_mask.mask_mult;
            cfg_tab->adjust_y.adjust_get_mask.mask_sub = para->adjust_y.adjust_get_mask.mask_sub;

            cfg_tab->adjust_y.adjust_get_ratio.to_use = 1;
            cfg_tab->adjust_y.adjust_get_ratio.ratio_par_up = para->adjust_y.adjust_get_ratio.ratio_par_up;
            cfg_tab->adjust_y.adjust_get_ratio.ratio_par_down = para->adjust_y.adjust_get_ratio.ratio_par_down;

            cfg_tab->mcf_uv.to_use = 0;
            cfg_tab->uv_fusion.to_use = 0;
            break;
        }
        default:
            break;
    }
    D("[%s] -\n", __func__);

    return CPE_FW_OK;
}

static void mcf_update_cmdlst_cfg_tab(cfg_tab_cmdlst_t* cmdlst_cfg, unsigned int channel_id, schedule_cmdlst_link_t* cmd_link_entry)
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

static void mcf_set_cmdlst_stripe(cmdlst_stripe_info_t* stripe, unsigned int stripe_cnt)
{
    D("[%s] +\n", __func__);
    unsigned int i = 0;
    for(i = 0; i < stripe_cnt; i++)
    {
        if(i == stripe_cnt - 1)
        {
            stripe[i].is_last_stripe   = 1;
        }
        else
        {
            stripe[i].is_last_stripe   = 0;
        }
    }
    D("[%s] -\n", __func__);
}

static int mcf_set_irq_mode(msg_req_mcf_request_t* mcf_request, schedule_cmdlst_link_t* cmd_link_entry,unsigned int stripe_cnt)
{
    D("[%s] +\n", __func__);
    cpe_cmdlst_irq_mode irq_mode = CMD_IRQ_MAX;
    unsigned int i = 0;
    if(mcf_request->mcf_cfg.mode == MCF_FULL_MODE)
    {
        full_mode_t *para = (full_mode_t *)&(mcf_request->u.full_mode_param);
        if(para->uv_fusion.get_can.input_cnt == 0)
        {
            irq_mode = CMD_IRQ_MCF_FULL_MODE_ONE_INPUT;
        }
        else if(para->uv_fusion.get_can.input_cnt == 1)
        {
            irq_mode = CMD_IRQ_MCF_FULL_MODE_TWO_INPUT;
        }
        else if(para->uv_fusion.get_can.input_cnt == 2)
        {
            irq_mode = CMD_IRQ_MCF_MFNR_FULL_MODE;
        }
    }
    else if(mcf_request->mcf_cfg.mode == MCF_PARTIAL_MODE)
    {
        irq_mode = CMD_IRQ_MCF_PARTIAL_MODE;
    }
    else if(mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE)
    {
        y_gf_mode_t *para = (y_gf_mode_t *)&(mcf_request->u.y_lf_gf_mode_param);
        if(para->input_mode == SINGLE_INPUT)
        {
            irq_mode = CMD_IRQ_MCF_YGF_SINGLE_MODE;
            
            if(para->output_mode == LF_ONLY)
            {
                irq_mode = CMD_IRQ_MCF_YGF_SINGLE_MODE_LF_ONLY;
            }
        }
        else if(para->input_mode == DUAL_INPUT)
        {
            irq_mode = CMD_IRQ_MCF_YGF_DUAL_MODE;
            if(para->output_mode == LF_ONLY)
            {
                irq_mode = CMD_IRQ_MCF_YGF_DUAL_MODE_LF_ONLY;
            }
        }
    }


    for(i = 0;i < stripe_cnt; i++)
    {
        cmd_link_entry[i].irq_mode = irq_mode;
    }
    D("[%s] -\n", __func__);
    return 0;
}

int mcf_request_handler(msg_req_mcf_request_t* mcf_request)
{
    unsigned long long va = 0;
    unsigned int da = 0;
    int ret = CPE_FW_OK;

    D("[%s] +\n", __func__);
    if (mcf_request == NULL) {
        pr_err("[%s] failed : mcf_request is null", __func__);
        return -1;
    }
    
    mcf_dump_request(mcf_request);
    hispcpe_reg_set(CPE_TOP,0x240,0);//SET TOP MODE = MCF

    unsigned int mode = (mcf_request->mcf_cfg.mode == MCF_Y_GF_MODE) ? 1 : 0;
    isp_stripe_info_t* stripe_info = NULL;
    isp_stripe_info_t* stripe_info_DS4 = NULL;

    stripe_info = (isp_stripe_info_t*)kmalloc(sizeof(isp_stripe_info_t), GFP_KERNEL);
     if (NULL == stripe_info)
    {
        pr_err(" fail to kmalloc stripe_info\n");
	goto fail_get_mem;
    }
    
    stripe_info_DS4 = (isp_stripe_info_t*)kmalloc(sizeof(isp_stripe_info_t), GFP_KERNEL);
    if (NULL == stripe_info_DS4)
    {
        pr_err(" fail to kmalloc stripe_info_DS4\n");
	goto fail_get_mem;
    }

    mcf_calc_stripe_info(mode, &mcf_request->streams[BI_MONO], stripe_info, stripe_info_DS4);

    unsigned int stripe_cnt = stripe_info->stripe_cnt;

    if ((ret = cpe_mem_get(MEM_ID_MCF_CFG_TAB, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_MCF_CFG_TAB, va, da, ret);
        goto fail_get_mem;
    }
    mcf_config_table_t* mcf_cfg_tab = (mcf_config_table_t*)va;

    if ((ret = cpe_mem_get(MEM_ID_CVDR_CFG_TAB_MM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CVDR_CFG_TAB_MM, va, da, ret);
        goto fail_get_mem;
    }
    cfg_tab_cvdr_t* cvdr_cfg_tab = (cfg_tab_cvdr_t*)va;

    if ((ret = cpe_mem_get(MEM_ID_CPETOP_CFG_TAB_MM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CPETOP_CFG_TAB_MM, va, da, ret);
        goto fail_get_mem;
    }
    cpe_top_config_table_t* top_cfg_tab = (cpe_top_config_table_t*)va;

    unsigned int i = 0;
    for(i = 0;i < stripe_cnt;i++)
    {
        mcf_update_mcf_cfg_tab(mcf_request, &mcf_cfg_tab[i], stripe_info->stripe_width[i]);
        mcf_update_cvdr_wr_cfg_tab(mcf_request, &cvdr_cfg_tab[i], stripe_info, stripe_info_DS4, i);
        mcf_update_cvdr_rd_cfg_tab(mcf_request, &cvdr_cfg_tab[i], stripe_info, stripe_info_DS4, i);
        mcf_update_top_cfg_tab(mcf_request, &top_cfg_tab[i], stripe_info, stripe_info_DS4, i);
    }

    if ((ret = cpe_mem_get(MEM_ID_CMDLST_PARA_MM, &va, &da)) != 0) {
        pr_err("[%s] Failed : cpe_mem_get.(%d, 0x%llx, 0x%x).%d\n", __func__, MEM_ID_CMDLST_PARA_MM, va, da, ret);
        goto fail_get_mem;
    }
    cmdlst_para_t* cmdlst_para = (cmdlst_para_t*)va;
    cmdlst_para->channel_id = 0;
    cmdlst_para->stripe_cnt = stripe_cnt;

    mcf_set_cmdlst_stripe(&cmdlst_para->cmd_stripe_info[0],stripe_cnt);
    df_sched_prepare(cmdlst_para);

    schedule_cmdlst_link_t* cmd_link_entry = (schedule_cmdlst_link_t*)cmdlst_para->cmd_entry;
    mcf_set_irq_mode(mcf_request,cmd_link_entry,stripe_cnt);
    df_sched_set_buffer_header(cmdlst_para);

    for(i = 0;i < stripe_cnt;i++)
    {
        mcf_prepare_cmd(&g_mcf_devs[0], &cmd_link_entry[i].cmd_buf, &mcf_cfg_tab[i]);
        cpe_top_prepare_cmd(&g_cpe_top_devs[0], &cmd_link_entry[i].cmd_buf, &top_cfg_tab[i]);
        cvdr_prepare_cmd(&g_cvdr_devs[0], &cmd_link_entry[i].cmd_buf, &cvdr_cfg_tab[i]);
        cmdlst_set_buffer_padding(&cmd_link_entry[i].cmd_buf);
    }

    cfg_tab_cmdlst_t cmdlst_cfg;
    mcf_update_cmdlst_cfg_tab(&cmdlst_cfg, cmdlst_para->channel_id, &cmd_link_entry[0]);

    df_sched_start(&cmdlst_cfg, cmdlst_para);

    cpe_mem_free(MEM_ID_MCF_CFG_TAB);
    cpe_mem_free(MEM_ID_CVDR_CFG_TAB_MM);
    cpe_mem_free(MEM_ID_CPETOP_CFG_TAB_MM);
	
    fail_get_mem:
    if (stripe_info){
        kfree(stripe_info);
	stripe_info = NULL;
    }
    if (stripe_info_DS4){
        kfree(stripe_info_DS4);
	stripe_info_DS4 = NULL;
    }

    D("[%s] -\n", __func__);

    return ret;
}

int mcf_eof_handler()
{
    D("[%s] +\n", __func__);
    cmdlst_eop_handler(CMD_EOF_MCF_MODE);
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}
static int mcf_dump_request(msg_req_mcf_request_t* req)
{
    D("[%s] +\n", __func__);

    mcf_uv_t* mcf_uv_tab;
    adjust_y_t* adjust_y_tab;

    int i = 0;

    pr_info("frame_number = %d, uv_pack = %d\n", req->frame_number,req->mcf_cfg.uv_pack);
    for(i = 0; i < 9; i++)
    {
        pr_info("streams[%d].width = %d,height = %d,buf = %d,format = %d\n",
            i, req->streams[i].width, req->streams[i].height, req->streams[i].buf, req->streams[i].format);
    }

    if(req->mcf_cfg.mode == MCF_FULL_MODE)
    {
        pr_info("MCF FULL MODE DUMP: \n");
        uv_fusion_t* uv_fusion_tab = &req->u.full_mode_param.uv_fusion;
        mcf_uv_tab = &req->u.full_mode_param.mcf_uv_cfg;
        adjust_y_tab = &req->u.full_mode_param.adjust_y;
        pr_info("[uv_fusion][get_can_t]th_u = %d,th_v = %d, const_value = %d, input_cnt = %d\n",
            uv_fusion_tab->get_can.th_u, uv_fusion_tab->get_can.th_v,
            uv_fusion_tab->get_can.const_value, uv_fusion_tab->get_can.input_cnt);
        pr_info("[uv_fusion][get_final_uv]ratio_dist_uv_to_conf = %d, ratio_dist_to_conf = %d, max_u_diff_const_inv = %d, max_v_diff_const_inv = %d\n",
            uv_fusion_tab->get_final_uv.ratio_dist_uv_to_conf, uv_fusion_tab->get_final_uv.ratio_dist_to_conf,
            uv_fusion_tab->get_final_uv.max_u_diff_const_inv, uv_fusion_tab->get_final_uv.max_v_diff_const_inv);
        pr_info("[uv_fusion][get_con]hybrid_diff_hue_shift = %d, hybrid_diff_hue_coff = %d, hybrid_diff_hue_shift_ex = %d, max_diff = %d, max_diff_inv = %d\n",
            uv_fusion_tab->get_con.hybrid_diff_hue_shift, uv_fusion_tab->get_con.hybrid_diff_hue_coff,
            uv_fusion_tab->get_con.hybrid_diff_hue_shift_ex, uv_fusion_tab->get_con.max_diff,
            uv_fusion_tab->get_con.max_diff_inv);


        pr_info("[mcf_uv_cfg][fill_conflict_cfg]luma_th = %d, conflict_th = %d\n",
            mcf_uv_tab->fill_conflict_cfg.luma_th, mcf_uv_tab->fill_conflict_cfg.conflict_th);
        pr_info("[mcf_uv_cfg][fill_conflict_cfg]gauss_radius = %d, mask_blend = %d\n",
            mcf_uv_tab->smooth_blend_cfg.gauss_radius, mcf_uv_tab->smooth_blend_cfg.mask_blend);
        pr_info("[mcf_uv_cfg][smooth_blend_gf]uv_gf_radius = %d, uv_gf_eps = %d\n",
            mcf_uv_tab->smooth_blend_gf.uv_gf_radius, mcf_uv_tab->smooth_blend_gf.uv_gf_eps);

        pr_info("[adjust_y][adjust_col_gf]y_gf_radius = %d, y_gf_eps = %d\n",
            adjust_y_tab->adjust_col_gf.y_gf_radius, adjust_y_tab->adjust_col_gf.y_gf_eps);
        pr_info("[adjust_y][adjust_get_mask]mask_sub = %d, mask_mult = %d\n",
            adjust_y_tab->adjust_get_mask.mask_sub, adjust_y_tab->adjust_get_mask.mask_mult);
        pr_info("[adjust_y][adjust_get_ratio]ratio_par_down = %d, ratio_par_up = %d\n",
            adjust_y_tab->adjust_get_ratio.ratio_par_down, adjust_y_tab->adjust_get_ratio.ratio_par_up);
    }
    else if(req->mcf_cfg.mode == MCF_PARTIAL_MODE)
    {
        pr_info("MCF PARTIAL MODE DUMP: \n");
        mcf_uv_tab = &req->u.partial_mode_param.mcf_uv_cfg;
        adjust_y_tab = &req->u.partial_mode_param.adjust_y;

        pr_info("[mcf_uv_cfg][fill_conflict_cfg]luma_th = %d, conflict_th = %d\n",
            mcf_uv_tab->fill_conflict_cfg.luma_th, mcf_uv_tab->fill_conflict_cfg.conflict_th);
        pr_info("[mcf_uv_cfg][fill_conflict_cfg]gauss_radius = %d, mask_blend = %d\n",
            mcf_uv_tab->smooth_blend_cfg.gauss_radius, mcf_uv_tab->smooth_blend_cfg.mask_blend);
        pr_info("[mcf_uv_cfg][smooth_blend_gf]uv_gf_radius = %d, uv_gf_eps = %d\n",
            mcf_uv_tab->smooth_blend_gf.uv_gf_radius, mcf_uv_tab->smooth_blend_gf.uv_gf_eps);

        pr_info("[adjust_y][adjust_col_gf]y_gf_radius = %d, y_gf_eps = %d\n",
            adjust_y_tab->adjust_col_gf.y_gf_radius, adjust_y_tab->adjust_col_gf.y_gf_eps);
        pr_info("[adjust_y][adjust_get_mask]mask_sub = %d, mask_mult = %d\n",
            adjust_y_tab->adjust_get_mask.mask_sub, adjust_y_tab->adjust_get_mask.mask_mult);
        pr_info("[adjust_y][adjust_get_ratio]ratio_par_down = %d, ratio_par_up = %d\n",
            adjust_y_tab->adjust_get_ratio.ratio_par_down, adjust_y_tab->adjust_get_ratio.ratio_par_up);
    }
    else if(req->mcf_cfg.mode == MCF_Y_GF_MODE)
    {
        adjust_y_tab = &req->u.y_lf_gf_mode_param.adjust_y;
        pr_info("MCF YGF MODE DUMP: \n");

        pr_info("input mode = %d, out mode = %d\n",
            req->u.y_lf_gf_mode_param.input_mode, req->u.y_lf_gf_mode_param.output_mode);
        pr_info("[adjust_y][adjust_col_gf]y_gf_radius = %d, y_gf_eps = %d\n",
            adjust_y_tab->adjust_col_gf.y_gf_radius, adjust_y_tab->adjust_col_gf.y_gf_eps);
        pr_info("[adjust_y][adjust_get_mask]mask_sub = %d, mask_mult = %d\n",
            adjust_y_tab->adjust_get_mask.mask_sub, adjust_y_tab->adjust_get_mask.mask_mult);
        pr_info("[adjust_y][adjust_get_ratio]ratio_par_down = %d, ratio_par_up = %d\n",
            adjust_y_tab->adjust_get_ratio.ratio_par_down, adjust_y_tab->adjust_get_ratio.ratio_par_up);
    }
    D("[%s] -\n", __func__);
    return CPE_FW_OK;
}

static int mcf_calc_stripe_info(unsigned int is_ygf_mode,cpe_stream_t* stream, isp_stripe_info_t* p_stripe_info, isp_stripe_info_t* p_stripe_info_DS4)
{
    D("[%s] +\n", __func__);
    unsigned int i            = 0;
    unsigned int overlap      = is_ygf_mode ? 16 : 32;
    df_size_constrain_t  p_size_constrain;

    unsigned int constrain_cnt = 1;

    p_size_constrain.hinc      = 1*65536;
    p_size_constrain.pix_align = is_ygf_mode ? 16 : 32;
    p_size_constrain.out_width = 8912;

    pr_info("overlap =%d mono_width =%d \n", overlap, stream->width);

    df_size_split_stripe(constrain_cnt, &p_size_constrain, p_stripe_info,overlap, stream->width);

    p_stripe_info->full_size.width = stream->width;
    p_stripe_info->full_size.height = stream->height;
    
    df_size_dump_stripe_info(p_stripe_info, "org_stripe");

    if(0 == is_ygf_mode)
    {
        p_stripe_info_DS4->stripe_cnt = p_stripe_info->stripe_cnt;
        p_stripe_info_DS4->full_size.width = stream->width/2;
        p_stripe_info_DS4->full_size.height = stream->height/2;
        for(i=0;i<p_stripe_info->stripe_cnt;i++)
        {
            p_stripe_info_DS4->stripe_width[i]       = p_stripe_info->stripe_width[i]/2;
            p_stripe_info_DS4->stripe_start_point[i] = p_stripe_info->stripe_start_point[i]/2;
            p_stripe_info_DS4->stripe_end_point[i]   = p_stripe_info->stripe_end_point[i]/2;
            p_stripe_info_DS4->overlap_left[i]       = p_stripe_info->overlap_left[i]/2;
            p_stripe_info_DS4->overlap_right[i]      = p_stripe_info->overlap_right[i]/2;
        }
        df_size_dump_stripe_info(p_stripe_info_DS4, "ds_4_stripe");
    }

    return CPE_FW_OK;
}

/*lint +e429  +e613  +e668*/
#pragma GCC diagnostic pop
