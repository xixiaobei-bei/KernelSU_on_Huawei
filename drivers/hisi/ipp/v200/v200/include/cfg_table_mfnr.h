 

#ifndef __CFG_TABLE_MFNR_H__
#define __CFG_TABLE_MFNR_H__

#define MFNR_MEAN2SIGMA  (26)
#define MFNR_GRAD2SIGMA  (65)
#define MFNR_MEAN2BLEND  (26)
#define MFNR_GRAD2BLEND  (32)

typedef enum{
    MFNR_FULL_COLOR_MODE     = 0,
    MFNR_FULL_MONO_MODE      = 1,
    MFNR_PARTIAL_COLOR_MODE  = 2,
    MFNR_PARTIAL_MONO_MODE   = 3,
}mfnr_mode_e;

typedef struct _mfnr_cfg_t
{
    unsigned int  to_use;
    unsigned int  op_mode;
    unsigned int  uv_unpack;
    unsigned int  uv_pack;
    unsigned int  ghost_mask_wr_en;
    unsigned int  ghost_stats_crop_h_en;
} mfnr_cfg_t;

typedef struct _mfnr_size_cfg_t
{
    unsigned int  width;
    unsigned int  height;
} mfnr_size_t;

typedef struct _mfnr_ghost_detect_cfg_t
{
    unsigned int  ghost_mask_th;
} mfnr_ghost_detect_cfg_t;

typedef struct _mfnr_us16_crop_cfg_t
{
    unsigned int  us16_crop_h_start;
    unsigned int  us16_crop_h_end;
} mfnr_us16_crop_cfg_t;

typedef struct _mfnr_stats_crop_cfg_t
{
    unsigned int  stats_crop_h_start;
    unsigned int  stats_crop_h_end;
} mfnr_stats_crop_cfg_t;

typedef struct _mfnr_tempo_fusion_cfg_t
{
    unsigned int  weight;
    unsigned int  ghost_region_th;
    unsigned int  subscale;
} mfnr_tempo_fusion_cfg_t;

typedef struct _mfnr_sigma_stren_t
{
    unsigned int  max_grad2sigma_scale_bkpt;
    unsigned int  min_grad2sigma_scale_bkpt;
    unsigned int  judge_grad_region_th;
    unsigned int  sigma;
} mfnr_sigma_stren_t;

typedef struct _mfnr_blend_stren_t
{
    unsigned int  max_grad2blend_scale_bkpt; 
    unsigned int  min_grad2blend_scale_bkpt; 
    unsigned int  sup_mean2blend_th ; 
    unsigned int  sup_blend_scale ; 
} mfnr_blend_stren_t;

typedef struct _mfnr_ghost_size_stats_t
{
    unsigned int    ghost_size; 
} mfnr_ghost_size_stats_t;

typedef struct _mfnr_basic_cfg_t
{
    unsigned int  to_use;
    mfnr_cfg_t    mfnr_cfg;
    mfnr_size_t   mfnr_size;
} mfnr_basic_cfg_t;

typedef struct _mfnr_ghost_detect_y_t
{
    unsigned int             to_use;
    mfnr_ghost_detect_cfg_t  ghost_detect_cfg;
    mfnr_us16_crop_cfg_t     us16_crop_cfg;
    mfnr_stats_crop_cfg_t    stats_crop_cfg;
    mfnr_ghost_size_stats_t  ghost_size_stats;
} mfnr_ghost_detect_y_t;


typedef struct _temporal2_fusion_yuv_t
{
    unsigned int             to_use;
    mfnr_tempo_fusion_cfg_t  tempo_fusion;
    mfnr_sigma_stren_t       dghost_sigma_stren;
    mfnr_blend_stren_t       dghost_blend_stren;
    unsigned int             dghost_mean2sigma_lut[MFNR_MEAN2SIGMA];
    unsigned int             dghost_grad2sigma_lut[MFNR_GRAD2SIGMA];
    unsigned int             dghost_mean2blend_lut[MFNR_MEAN2BLEND];
    unsigned int             dghost_grad2blend_lut[MFNR_GRAD2BLEND];
} temporal2_fusion_yuv_t;

typedef struct _mfnr_config_table_t
{
    unsigned int           to_use;
    mfnr_basic_cfg_t       basic_cfg;
    mfnr_ghost_detect_y_t  ghost_detect_y;
    temporal2_fusion_yuv_t temporal_fusion;
} mfnr_config_table_t;

#endif/*__CFG_TABLE_MFNR_H__*/

