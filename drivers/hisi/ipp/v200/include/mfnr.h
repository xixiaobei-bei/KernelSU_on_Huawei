#ifndef _MFNR_H__
#define _MFNR_H__

#include "ipp.h"
#include "cfg_table_mfnr.h"

#define MFNR_MEAN2SIGMA  (26)
#define MFNR_GRAD2SIGMA  (65)
#define MFNR_MEAN2BLEND  (26)
#define MFNR_GRAD2BLEND  (32)

typedef enum _mfnr_buf_usage_e
{
    BI_MFNR_REF_MATCH_YUV_DS16 = 0,
    BI_MFNR_THR_YUV_DS16,
    BI_MFNR_TEMPO_REF_UV,
    BI_MFNR_TEMPO_REF_Y,
    BI_MFNR_CURR_UV,
    BI_MFNR_CURR_Y,
    BO_MFNR_RESULT_UV,
    BO_MFNR_RESULT_Y,
    BO_MFNR_GHOST_MASK,
    MFNR_STREAM_MAX,
}mfnr_buf_usage_e;

typedef struct _mfnr_stream_t
{
    unsigned int  width;
    unsigned int  height;
    unsigned int  buf;
    pix_format_e  format;
} mfnr_stream_t;

typedef struct _mfnr_config_t
{
    unsigned int  op_mode;
    unsigned int  uv_unpack;
    unsigned int  uv_pack;
    unsigned int  ghost_mask_wr_en;
    unsigned int  ghost_stats_crop_h_en;
} mfnr_config_t;

typedef struct _ghost_detect_cfg_t
{
    unsigned int  ghost_mask_th;
} ghost_detect_cfg_t;

typedef struct _ghost_detect_us16_crop_cfg_t
{
    unsigned int  us16_crop_h_start;
    unsigned int  us16_crop_h_end;
} ghost_detect_us16_crop_cfg_t;

typedef struct _ghost_detect_stats_crop_cfg_t
{
    unsigned int  stats_crop_h_start;
    unsigned int  stats_crop_h_end;
} ghost_detect_stats_crop_cfg_t;

typedef struct _tempo_fusion_cfg_t
{
    unsigned int  weight;
    unsigned int  ghost_region_th;
    unsigned int  subscale;
} tempo_fusion_cfg_t;

typedef struct _dghost_sigma_stren_t
{
    unsigned int  max_grad2sigma_scale_bkpt;
    unsigned int  min_grad2sigma_scale_bkpt;
    unsigned int  judge_grad_region_th;
    unsigned int  sigma;
} dghost_sigma_stren_t;

typedef struct _dghost_blend_stren_t
{
    unsigned int  max_grad2blend_scale_bkpt; 
    unsigned int  min_grad2blend_scale_bkpt; 
    unsigned int  sup_mean2blend_th ; 
    unsigned int  sup_blend_scale ; 
} dghost_blend_stren_t;

typedef struct _ghost_detect_ghost_size_stats_t
{
    unsigned int    ghost_size; 
} ghost_detect_ghost_size_stats_t;

typedef struct _ghost_detect_y_t
{
    ghost_detect_cfg_t               ghost_detect_cfg;
    ghost_detect_us16_crop_cfg_t     us16_crop_cfg;
    ghost_detect_stats_crop_cfg_t    stats_crop_cfg;
    ghost_detect_ghost_size_stats_t  ghost_size_stats;
} ghost_detect_y_t;

typedef struct _temporal_fusion_yuv_t
{
    tempo_fusion_cfg_t    tempo_fusion;
    dghost_sigma_stren_t  dghost_sigma_stren;
    dghost_blend_stren_t  dghost_blend_stren;
    unsigned int  dghost_mean2sigma_lut[MFNR_MEAN2SIGMA];
    unsigned int  dghost_grad2sigma_lut[MFNR_GRAD2SIGMA];
    unsigned int  dghost_mean2blend_lut[MFNR_MEAN2BLEND];
    unsigned int  dghost_grad2blend_lut[MFNR_GRAD2BLEND];
} temporal_fusion_yuv_t;

typedef struct
{
    ghost_detect_y_t       ghost_detection_y;
    temporal_fusion_yuv_t  temporal_fusion_yuv;
} mfnr_full_color_mode_t;

typedef struct
{
    ghost_detect_y_t       ghost_detection_y;
    temporal_fusion_yuv_t  temporal_fusion_yuv;
} mfnr_full_mono_mode_t;

typedef struct
{
    temporal_fusion_yuv_t  temporal_fusion_yuv;
} mfnr_partial_color_mode_t;

typedef struct
{
    temporal_fusion_yuv_t  temporal_fusion_yuv;
} mfnr_partial_mono_mode_t;

typedef struct _msg_req_mfnr_request_t
{
    unsigned int  frame_number;
    mfnr_config_t mfnr_cfg;
    cpe_stream_t streams[MFNR_STREAM_MAX];
    union
    {
        mfnr_full_color_mode_t    full_color_mode_param;
        mfnr_full_mono_mode_t     full_mono_mode_param;
        mfnr_partial_color_mode_t partial_color_mode_param;
        mfnr_partial_mono_mode_t  partial_mono_mode_param;
    }u;
} msg_req_mfnr_request_t;

int mfnr_request_handler(msg_req_mfnr_request_t* mfnr_request);
int mfnr_eof_handler(void);

#endif/*_MFNR_H__*/
