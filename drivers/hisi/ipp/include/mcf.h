#ifndef _MCF_H__
#define _MCF_H__

#include "ipp.h"
#include "cfg_table_mcf.h"

typedef enum{
    SINGLE_INPUT = 0,
    DUAL_INPUT = 1,
}y_gf_in_mode_e;

typedef enum{
    LF_ONLY = 0,
    A_B = 1,
    LF_HF =2 ,
}y_gf_out_mode_e;

typedef enum _mcf_buf_usage_e
{
    BI_MONO_DS4 = 0, 
    BI_WARP_0_UVC ,
    BI_WARP_1_UVC,
    BI_WARP_2_UVC,
    BI_MONO,
    BI_WARP_COL_Y_DS4,
    BO_RESULT_UV,
    BO_RESULT_C,
    BO_RESULT_Y,
    MCF_STREAM_MAX,
}mcf_buf_usage_e;

typedef struct _mcf_config_t
{
    unsigned int  uv_pack;
    mcf_mode_e mode;
} mcf_config_t;

typedef struct
{
    unsigned int    th_u; 
    unsigned int    th_v; 
    unsigned int    const_value; 
    unsigned int    input_cnt; 
} get_can_t;

typedef struct
{
    unsigned int    ratio_dist_uv_to_conf;
    unsigned int    ratio_dist_to_conf;
    unsigned int    max_u_diff_const_inv;
    unsigned int    max_v_diff_const_inv;
} get_final_uv_t;

typedef struct
{
    unsigned int    hybrid_diff_hue_shift;
    unsigned int    hybrid_diff_hue_coff;
    unsigned int    hybrid_diff_hue_shift_ex;
    unsigned int    max_diff;
    unsigned int    max_diff_inv;
} get_con_t;

typedef struct
{
    unsigned int    luma_th; 
    unsigned int    conflict_th; 
} fill_conflict_cfg_t;

typedef struct
{
    unsigned int    gauss_radius; 
    unsigned int    mask_blend; 
} smooth_blend_cfg_t;

typedef struct
{
    unsigned int    uv_gf_radius; 
    unsigned int    uv_gf_eps; 
} smooth_blend_gf_cfg_t;

typedef struct
{
    unsigned int    y_gf_radius;
    unsigned int    y_gf_eps;
} adjust_col_gf_t;

typedef struct
{
    unsigned int    mask_sub;
    unsigned int    mask_mult;
} adjust_get_mask_t;

typedef struct
{
    unsigned int    ratio_par_down; 
    unsigned int    ratio_par_up; 
} adjust_get_ratio_t;

typedef struct
{
    get_can_t      get_can; 
    get_final_uv_t get_final_uv;
    get_con_t      get_con;
} uv_fusion_t;

typedef struct
{
    fill_conflict_cfg_t fill_conflict_cfg;
    smooth_blend_cfg_t           smooth_blend_cfg; 
    smooth_blend_gf_cfg_t    smooth_blend_gf; 
} mcf_uv_t;

typedef struct
{
    adjust_col_gf_t   adjust_col_gf; 
    adjust_get_mask_t adjust_get_mask;
    adjust_get_ratio_t adjust_get_ratio;
} adjust_y_t;

typedef struct
{
    uv_fusion_t    uv_fusion; 
    mcf_uv_t mcf_uv_cfg; 
    adjust_y_t     adjust_y;
} full_mode_t;


typedef struct
{
    mcf_uv_t mcf_uv_cfg; 
    adjust_y_t     adjust_y;
} partial_mode_t;

typedef struct
{
    y_gf_in_mode_e input_mode;
    y_gf_out_mode_e output_mode;
    adjust_y_t     adjust_y;
} y_gf_mode_t;

typedef struct _msg_req_mcf_request_t
{
    unsigned int  frame_number;
    mcf_config_t mcf_cfg;
    cpe_stream_t streams[MCF_STREAM_MAX];
    union
    {
        full_mode_t full_mode_param;
        partial_mode_t partial_mode_param;
        y_gf_mode_t y_lf_gf_mode_param;
    }u;
} msg_req_mcf_request_t;

int mcf_request_handler(msg_req_mcf_request_t* mcf_request);
int mcf_eof_handler(void);
extern void *get_cpe_addr_va(void);
#endif /* _MCF_H__ */
