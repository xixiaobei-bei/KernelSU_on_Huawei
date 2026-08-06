
#include <linux/printk.h>
#include "ipp.h"
#include "mfnr_drv.h"
#include "mfnr_drv_priv.h"
#include "mfnr_reg_offset.h"
#include "mfnr_reg_offset_field.h"
//#include "log.h"

#define LOG_TAG LOG_MODULE_MFNR_DRV

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"

static int mfnr_basic_config(mfnr_dev_t* dev, mfnr_basic_cfg_t* mfnr_basic_cfg)
{
    U_MFNR_CFG       temp_mfnr_cfg;
    U_MFNR_SIZE_CFG  temp_mfnr_size;
    temp_mfnr_cfg.u32     = 0;
    temp_mfnr_size.u32    = 0;

    temp_mfnr_cfg.bits.op_mode               = mfnr_basic_cfg->mfnr_cfg.op_mode;
    temp_mfnr_cfg.bits.uv_unpack             = mfnr_basic_cfg->mfnr_cfg.uv_unpack;
    temp_mfnr_cfg.bits.uv_pack               = mfnr_basic_cfg->mfnr_cfg.uv_pack;
    temp_mfnr_cfg.bits.ghost_mask_wr_en      = mfnr_basic_cfg->mfnr_cfg.ghost_mask_wr_en;
    temp_mfnr_cfg.bits.ghost_stats_crop_h_en = mfnr_basic_cfg->mfnr_cfg.ghost_stats_crop_h_en;

    temp_mfnr_size.bits.width  = mfnr_basic_cfg->mfnr_size.width - 1;
    temp_mfnr_size.bits.height = mfnr_basic_cfg->mfnr_size.height - 1;


    CMDLST_SET_REG(dev->base_addr + MFNR_MFNR_CFG_REG , temp_mfnr_cfg.u32);
    CMDLST_SET_REG(dev->base_addr + MFNR_MFNR_SIZE_CFG_REG , temp_mfnr_size.u32);

    return CPE_FW_OK;
 }

static int mfnr_config_ghost_detection_y(mfnr_dev_t* dev, mfnr_ghost_detect_y_t* ghost_detection_y)
{
    U_GHOST_DETECT_CFG               temp_ghost_detect_cfg;
    U_GHOST_DETECT_US16_CROP_CFG     temp_us16_crop_cfg;
    U_GHOST_DETECT_STATS_CROP_H_CFG  temp_stats_crop_cfg;
    temp_ghost_detect_cfg.u32 = 0;
    temp_us16_crop_cfg.u32    = 0;
    temp_stats_crop_cfg.u32   = 0;

    temp_ghost_detect_cfg.bits.ghost_mask_th    = ghost_detection_y->ghost_detect_cfg.ghost_mask_th;
    
    temp_us16_crop_cfg.bits.us16_crop_h_start   = ghost_detection_y->us16_crop_cfg.us16_crop_h_start;
    temp_us16_crop_cfg.bits.us16_crop_h_end     = ghost_detection_y->us16_crop_cfg.us16_crop_h_end;
    
    temp_stats_crop_cfg.bits.stats_crop_h_start = ghost_detection_y->stats_crop_cfg.stats_crop_h_start;
    temp_stats_crop_cfg.bits.stats_crop_h_end   = ghost_detection_y->stats_crop_cfg.stats_crop_h_end;


    CMDLST_SET_REG(dev->base_addr + MFNR_GHOST_DETECT_CFG_REG , temp_ghost_detect_cfg.u32);
    CMDLST_SET_REG(dev->base_addr + MFNR_GHOST_DETECT_US16_CROP_CFG_REG , temp_us16_crop_cfg.u32);
    CMDLST_SET_REG(dev->base_addr + MFNR_GHOST_DETECT_STATS_CROP_H_CFG_REG , temp_stats_crop_cfg.u32);

    return CPE_FW_OK;
 }

static int mfnr_config_temporal_fusion_yuv(mfnr_dev_t* dev, temporal2_fusion_yuv_t* temporal_fusion)
{
    int i=0;
    
    if (NULL == dev || NULL == temporal_fusion)
    {
        printk("[%s] Failed : Input param is NULL!\n", __func__);
        return CPE_FW_ERR;
    }

    U_TEMPO_FUSION_CFG        temp_tempo_fusion;
    U_DEGHOST_SIGMA_STREN_CFG temp_sigma_stren;
    U_DEGHOST_BLEND_STREN_CFG temp_blend_stren;
    U_DEGHOST_MEAN2SIGMA_LUT  temp_mean2sigma;
    U_DEGHOST_GRAD2SIGMA_LUT  temp_grad2sigma;
    U_DEGHOST_MEAN2BLEND_LUT  temp_mean2blend;
    U_DEGHOST_GRAD2BLEND_LUT  temp_grad2blend;

    temp_tempo_fusion.u32 = 0;
    temp_sigma_stren.u32  = 0;
    temp_blend_stren.u32  = 0;
    temp_mean2sigma.u32   = 0;
    temp_grad2sigma.u32   = 0;
    temp_mean2blend.u32   = 0;
    temp_grad2blend.u32   = 0;

    temp_tempo_fusion.bits.weight          = temporal_fusion->tempo_fusion.weight;
    temp_tempo_fusion.bits.subscale        = temporal_fusion->tempo_fusion.subscale;
    temp_tempo_fusion.bits.ghost_region_th = temporal_fusion->tempo_fusion.ghost_region_th;

    temp_sigma_stren.bits.sigma                     = temporal_fusion->dghost_sigma_stren.sigma;
    temp_sigma_stren.bits.judge_grad_region_th      = temporal_fusion->dghost_sigma_stren.judge_grad_region_th;
    temp_sigma_stren.bits.min_grad2sigma_scale_bkpt = temporal_fusion->dghost_sigma_stren.min_grad2sigma_scale_bkpt;
    temp_sigma_stren.bits.max_grad2sigma_scale_bkpt = temporal_fusion->dghost_sigma_stren.max_grad2sigma_scale_bkpt;

    temp_blend_stren.bits.sup_blend_scale           = temporal_fusion->dghost_blend_stren.sup_blend_scale;
    temp_blend_stren.bits.sup_mean2blend_th         = temporal_fusion->dghost_blend_stren.sup_mean2blend_th;
    temp_blend_stren.bits.min_grad2blend_scale_bkpt = temporal_fusion->dghost_blend_stren.min_grad2blend_scale_bkpt;
    temp_blend_stren.bits.max_grad2blend_scale_bkpt = temporal_fusion->dghost_blend_stren.max_grad2blend_scale_bkpt;

    CMDLST_SET_REG(dev->base_addr + MFNR_TEMPO_FUSION_CFG_REG ,          temp_tempo_fusion.u32);
    CMDLST_SET_REG(dev->base_addr + MFNR_DEGHOST_SIGMA_STREN_CFG_REG ,    temp_sigma_stren.u32);
    CMDLST_SET_REG(dev->base_addr + MFNR_DEGHOST_BLEND_STREN_CFG_REG ,    temp_blend_stren.u32);

    for(i = 0; i < MFNR_MEAN2SIGMA; i++)
    {
        temp_mean2sigma.u32 = 0;
        temp_mean2sigma.bits.mean2sigma = temporal_fusion->dghost_mean2sigma_lut[i];
        CMDLST_SET_REG(dev->base_addr + MFNR_DEGHOST_MEAN2SIGMA_LUT_0_REG + i * 0x4, temp_mean2sigma.u32);
    }

    for(i = 0; i < MFNR_GRAD2SIGMA; i++)
    {
        temp_grad2sigma.u32 = 0;
        temp_grad2sigma.bits.grad2sigma = temporal_fusion->dghost_grad2sigma_lut[i];
        CMDLST_SET_REG(dev->base_addr + MFNR_DEGHOST_GRAD2SIGMA_LUT_0_REG + i * 0x4, temp_grad2sigma.u32);
    }

    for(i = 0; i < MFNR_MEAN2BLEND; i++)
    {
        temp_mean2blend.u32 = 0;
        temp_mean2blend.bits.mean2blend = temporal_fusion->dghost_mean2blend_lut[i];
        CMDLST_SET_REG(dev->base_addr + MFNR_DEGHOST_MEAN2BLEND_LUT_0_REG + i * 0x4, temp_mean2blend.u32);
    }

    for(i = 0; i < MFNR_GRAD2BLEND; i++)
    {
        temp_grad2blend.u32 = 0;
        temp_grad2blend.bits.grad2blend = temporal_fusion->dghost_grad2blend_lut[i];
        CMDLST_SET_REG(dev->base_addr + MFNR_DEGHOST_GRAD2BLEND_LUT_0_REG + i * 0x4, temp_grad2blend.u32);
    }

    return CPE_FW_OK;
 }

static int mfnr_do_config(mfnr_dev_t* dev, mfnr_config_table_t* cfg_tab)
{
    if((dev == NULL) || (cfg_tab == NULL))
    {
        return CPE_FW_ERR;
    }

    if(1 == cfg_tab->to_use)
    {
        cfg_tab->to_use = 0;

        if(1 == cfg_tab->basic_cfg.to_use)
        {
            cfg_tab->basic_cfg.to_use = 0;
            mfnr_basic_config(dev,&cfg_tab->basic_cfg);
        }

        if(1 == cfg_tab->ghost_detect_y.to_use)
        {
            cfg_tab->ghost_detect_y.to_use = 0;
            mfnr_config_ghost_detection_y(dev,&cfg_tab->ghost_detect_y);
        }

        if(1 == cfg_tab->temporal_fusion.to_use)
        {
            cfg_tab->temporal_fusion.to_use = 0;
            mfnr_config_temporal_fusion_yuv(dev,&cfg_tab->temporal_fusion);
        }
    }

    return CPE_FW_OK;
}

int mfnr_prepare_cmd(mfnr_dev_t * dev, cmd_buf_t * cmd_buf, mfnr_config_table_t * table)
{
    dev->cmd_buf = cmd_buf;

    loge_if_ret(mfnr_do_config(dev, table));

    return 0;
}

static mfnr_ops_t mfnr_ops =
{
    .prepare_cmd   = mfnr_prepare_cmd,
};

mfnr_dev_t g_mfnr_devs[] =
{
    [0] =
    {
        .base_addr = JPG_MFNR_ADDR,
        .ops = &mfnr_ops,
    },
};


#pragma GCC diagnostic pop

/********************************** END **********************************/
