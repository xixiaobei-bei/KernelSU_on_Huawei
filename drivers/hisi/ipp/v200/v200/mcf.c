
#include <linux/printk.h>
#include "ipp.h"
#include "mcf_drv.h"
#include "mcf_drv_priv.h"
#include "mcf_reg_offset.h"
#include "mcf_reg_offset_field.h"
//#include "log.h"

#define LOG_TAG LOG_MODULE_MCF_DRV

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static int mcf_set_basic_cfg(mcf_dev_t * dev, mcf_basic_cfg_t* basic_cfg)
{
    U_MCF_CFG tmp_mcf_cfg;
    tmp_mcf_cfg.u32 = 0;
    tmp_mcf_cfg.bits.op_mode = basic_cfg->op_mode;
    if(basic_cfg->y_gf_mode.to_use == 1)
    {
        tmp_mcf_cfg.bits.y_gf_mode_in = basic_cfg->y_gf_mode.y_gf_mode_in;
        tmp_mcf_cfg.bits.y_gf_mode_out = basic_cfg->y_gf_mode.y_gf_mode_out;
    }
    tmp_mcf_cfg.bits.uv_pack = basic_cfg->uv_pack;

    CMDLST_SET_REG(dev->base_addr + MCF_MCF_CFG_REG, tmp_mcf_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_size_cfg(mcf_dev_t * dev, mcf_size_cfg_t* size_cfg)
{
    U_MCF_SIZE_CFG tmp_mcf_size_cfg;
    tmp_mcf_size_cfg.u32 = 0;
    tmp_mcf_size_cfg.bits.width = size_cfg->width -1;
    tmp_mcf_size_cfg.bits.height = size_cfg->height -1;

    CMDLST_SET_REG(dev->base_addr + MCF_MCF_SIZE_CFG_REG, tmp_mcf_size_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_get_cand(mcf_dev_t * dev, mcf_get_can_t* get_cand_cfg)
{
    U_UV_FSN_GET_CAND_CFG tmp_mcf_get_cand_cfg;
    tmp_mcf_get_cand_cfg.u32 = 0;
    tmp_mcf_get_cand_cfg.bits.th_u = get_cand_cfg->th_u;
    tmp_mcf_get_cand_cfg.bits.th_v = get_cand_cfg->th_v;
    tmp_mcf_get_cand_cfg.bits.const_value = get_cand_cfg->const_value;
    tmp_mcf_get_cand_cfg.bits.input_cnt = get_cand_cfg->input_cnt;

    CMDLST_SET_REG(dev->base_addr + MCF_UV_FSN_GET_CAND_CFG_REG, tmp_mcf_get_cand_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_get_final_uv(mcf_dev_t * dev, mcf_get_final_uv_t* get_final_uv_cfg)
{
    U_UV_FSN_GET_FUV_RATIO_CFG tmp_fuvratio_cfg;
    tmp_fuvratio_cfg.u32 = 0;
    tmp_fuvratio_cfg.bits.ratio_dist_uv_to_conf = get_final_uv_cfg->ratio_dist_uv_to_conf;
    tmp_fuvratio_cfg.bits.ratio_dist_to_conf = get_final_uv_cfg->ratio_dist_to_conf;
    CMDLST_SET_REG(dev->base_addr + MCF_UV_FSN_GET_FUV_RATIO_CFG_REG, tmp_fuvratio_cfg.u32);

    U_UV_FSN_GET_FUV_MAX_U_DIFF_CFG tmp_udiff_cfg;
    tmp_udiff_cfg.u32 = 0;
    tmp_udiff_cfg.bits.max_u_diff_const_inv = get_final_uv_cfg->max_u_diff_const_inv;
    CMDLST_SET_REG(dev->base_addr + MCF_UV_FSN_GET_FUV_MAX_U_DIFF_CFG_REG, tmp_udiff_cfg.u32);

    U_UV_FSN_GET_FUV_MAX_V_DIFF_CFG tmp_vdiff_cfg;
    tmp_vdiff_cfg.u32 = 0;
    tmp_vdiff_cfg.bits.max_v_diff_const_inv = get_final_uv_cfg->max_v_diff_const_inv;
    CMDLST_SET_REG(dev->base_addr + MCF_UV_FSN_GET_FUV_MAX_V_DIFF_CFG_REG, tmp_vdiff_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_get_conf(mcf_dev_t * dev, mcf_get_con_t* get_con_cfg)
{
    U_UV_FSN_GET_CONF_CFG tmp_conf_cfg;
    tmp_conf_cfg.u32 = 0;
    tmp_conf_cfg.bits.diff_hue_shift = get_con_cfg->diff_hue_shift;
    tmp_conf_cfg.bits.diff_hue_shift_ex = get_con_cfg->diff_hue_shift_ex;
    tmp_conf_cfg.bits.diff_hue_coff = get_con_cfg->diff_hue_coff;
    CMDLST_SET_REG(dev->base_addr + MCF_UV_FSN_GET_CONF_CFG_REG, tmp_conf_cfg.u32);

    U_UV_FSN_GET_CONF_MAX_DIFF_CFG tmp_maxdiff_cfg;
    tmp_maxdiff_cfg.u32 = 0;
    tmp_maxdiff_cfg.bits.max_diff_inv = get_con_cfg->max_diff_inv;
    tmp_maxdiff_cfg.bits.max_diff = get_con_cfg->max_diff;
    CMDLST_SET_REG(dev->base_addr + MCF_UV_FSN_GET_CONF_MAX_DIFF_CFG_REG, tmp_maxdiff_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_fill_conf(mcf_dev_t * dev, mcf_fill_conflict_cfg_t* fill_conf_cfg)
{
    U_FILL_CONFLICT_CFG tmp_fill_conf;
    tmp_fill_conf.u32 = 0;
    tmp_fill_conf.bits.conflict_th = fill_conf_cfg->conflict_th;
    tmp_fill_conf.bits.luma_th = fill_conf_cfg->luma_th;
    CMDLST_SET_REG(dev->base_addr + MCF_FILL_CONFLICT_CFG_REG, tmp_fill_conf.u32);

    return CPE_FW_OK;
}

static int mcf_set_smooth_blend(mcf_dev_t * dev, mcf_smooth_blend_cfg_t* smooth_blend_cfg)
{
    U_SMOOTH_BLEND_CFG tmp_sb_cfg;
    tmp_sb_cfg.u32 = 0;
    tmp_sb_cfg.bits.gauss_radius = smooth_blend_cfg->gauss_radius;
    tmp_sb_cfg.bits.mask_blend = smooth_blend_cfg->mask_blend;
    CMDLST_SET_REG(dev->base_addr + MCF_SMOOTH_BLEND_CFG_REG, tmp_sb_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_smooth_blend_gf(mcf_dev_t * dev, mcf_smooth_blend_gf_cfg_t* smooth_blend_gf_cfg)
{
    U_SMOOTH_BLEND_GF_CFG tmp_sb_gf_cfg;
    tmp_sb_gf_cfg.u32 = 0;
    tmp_sb_gf_cfg.bits.uv_gf_radius = smooth_blend_gf_cfg->uv_gf_radius;
    tmp_sb_gf_cfg.bits.uv_gf_eps = smooth_blend_gf_cfg->uv_gf_eps;
    CMDLST_SET_REG(dev->base_addr + MCF_SMOOTH_BLEND_GF_CFG_REG, tmp_sb_gf_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_adjcol_gf(mcf_dev_t * dev, mcf_adjust_col_gf_t* adjcol_gf_cfg)
{
    U_ADJUST_COLOR_GF_CFG tmp_adjcol_gf_cfg;
    tmp_adjcol_gf_cfg.u32 = 0;
    tmp_adjcol_gf_cfg.bits.y_gf_radius = adjcol_gf_cfg->y_gf_radius;
    tmp_adjcol_gf_cfg.bits.y_gf_eps = adjcol_gf_cfg->y_gf_eps;
    CMDLST_SET_REG(dev->base_addr + MCF_ADJUST_COLOR_GF_CFG_REG, tmp_adjcol_gf_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_adjcol_get_mask(mcf_dev_t * dev, mcf_adjust_get_mask_t* adjcol_mask_cfg)
{
    U_ADJUST_COLOR_GET_MASK_CFG tmp_adjcol_mask_cfg;
    tmp_adjcol_mask_cfg.u32 = 0;
    tmp_adjcol_mask_cfg.bits.mask_sub = adjcol_mask_cfg->mask_sub;
    tmp_adjcol_mask_cfg.bits.mask_mult = adjcol_mask_cfg->mask_mult;
    CMDLST_SET_REG(dev->base_addr + MCF_ADJUST_COLOR_GET_MASK_CFG_REG, tmp_adjcol_mask_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_adjcol_get_ratio(mcf_dev_t * dev, mcf_adjust_get_ratio_t* adjcol_ratio_cfg)
{
    U_ADJUST_COLOR_GET_RATIO_CFG tmp_adjcol_ratio_cfg;
    tmp_adjcol_ratio_cfg.u32 = 0;
    tmp_adjcol_ratio_cfg.bits.ratio_par_down = adjcol_ratio_cfg->ratio_par_down;
    tmp_adjcol_ratio_cfg.bits.ratio_par_up = adjcol_ratio_cfg->ratio_par_up;
    CMDLST_SET_REG(dev->base_addr + MCF_ADJUST_COLOR_GET_RATIO_CFG_REG, tmp_adjcol_ratio_cfg.u32);

    return CPE_FW_OK;
}

static int mcf_set_uv_fusion(mcf_dev_t * dev, mcf_uv_fusion_t* uv_fusion)
{
    if(uv_fusion->get_can.to_use == 1)
    {
        mcf_set_get_cand(dev,&uv_fusion->get_can);
    }
    if(uv_fusion->get_final_uv.to_use == 1)
    {
        mcf_set_get_final_uv(dev,&uv_fusion->get_final_uv);
    }
    if(uv_fusion->get_con.to_use == 1)
    {
        mcf_set_get_conf(dev,&uv_fusion->get_con);
    }

    return CPE_FW_OK;
}

static int mcf_set_mcf_uv(mcf_dev_t * dev, mcf_mcf_uv_t* mcf_uv)
{
    if(mcf_uv->fill_conflict_cfg.to_use == 1)
    {
        mcf_set_fill_conf(dev,&mcf_uv->fill_conflict_cfg);
    }

    if(mcf_uv->smooth_blend_cfg.to_use == 1)
    {
        mcf_set_smooth_blend(dev,&mcf_uv->smooth_blend_cfg);
    }

    if(mcf_uv->smooth_blend_gf.to_use == 1)
    {
        mcf_set_smooth_blend_gf(dev,&mcf_uv->smooth_blend_gf);
    }

    return CPE_FW_OK;
}

static int mcf_set_adjust_y(mcf_dev_t * dev, mcf_adjust_y_t* adj_y)
{
    if(adj_y->adjust_col_gf.to_use == 1)
    {
        mcf_set_adjcol_gf(dev,&adj_y->adjust_col_gf);
    }

    if(adj_y->adjust_get_mask.to_use == 1)
    {
        mcf_set_adjcol_get_mask(dev,&adj_y->adjust_get_mask);
    }

    if(adj_y->adjust_get_ratio.to_use == 1)
    {
        mcf_set_adjcol_get_ratio(dev,&adj_y->adjust_get_ratio);
    }

    return CPE_FW_OK;
}

static int mcf_do_config(mcf_dev_t * dev, mcf_config_table_t* cfg_tab)
{
    if(dev == NULL || cfg_tab == NULL)
    {
        return CPE_FW_ERR;
    }

    if(cfg_tab->to_use == 1)
    {
        mcf_set_basic_cfg(dev,&cfg_tab->basic_cfg);
        mcf_set_size_cfg(dev,&cfg_tab->size_cfg);
        if(cfg_tab->uv_fusion.to_use == 1)
        {
            mcf_set_uv_fusion(dev,&cfg_tab->uv_fusion);
        }

        if(cfg_tab->mcf_uv.to_use == 1)
        {
            mcf_set_mcf_uv(dev,&cfg_tab->mcf_uv);
        }

        if(cfg_tab->adjust_y.to_use == 1)
        {
            mcf_set_adjust_y(dev,&cfg_tab->adjust_y);
        }
    }

    return CPE_FW_OK;
}

int mcf_prepare_cmd(mcf_dev_t * dev, cmd_buf_t * cmd_buf, mcf_config_table_t * table)
{
    dev->cmd_buf = cmd_buf;

    loge_if_ret(mcf_do_config(dev, table));

    return 0;
}

static mcf_ops_t mcf_ops =
{
    .prepare_cmd   = mcf_prepare_cmd,
};

mcf_dev_t g_mcf_devs[] =
{
    [0] =
    {
        .base_addr = JPG_MCF_ADDR,
        .ops = &mcf_ops,
    },
};

#pragma GCC diagnostic pop
/*********************************end******************************/




