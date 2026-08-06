 

#ifndef __CFG_TABLE_MCF_H__
#define __CFG_TABLE_MCF_H__

typedef enum{
    MCF_FULL_MODE = 0,
    MCF_PARTIAL_MODE = 1,
    MCF_Y_GF_MODE =2 ,
}mcf_mode_e;

typedef struct _mcf_y_gf_mode_t
{
    unsigned int        to_use;
    unsigned int        y_gf_mode_in;
    unsigned int        y_gf_mode_out;
}mcf_y_gf_mode_t;

typedef struct _mcf_basic_cfg_t
{
    unsigned int        uv_pack;
    mcf_mode_e        op_mode;
    mcf_y_gf_mode_t        y_gf_mode;
}mcf_basic_cfg_t;

typedef struct _mcf_size_cfg_t
{
    unsigned int        width;
    unsigned int        height;
}mcf_size_cfg_t;

typedef struct _mcf_get_can_t
{
    unsigned int    to_use;
    unsigned int    th_u; 
    unsigned int    th_v; 
    unsigned int    const_value; 
    unsigned int    input_cnt; 
} mcf_get_can_t;

typedef struct _mcf_get_final_uv_t
{
    unsigned int    to_use;
    unsigned int    ratio_dist_uv_to_conf;
    unsigned int    ratio_dist_to_conf;
    unsigned int    max_u_diff_const_inv;
    unsigned int    max_v_diff_const_inv;
} mcf_get_final_uv_t;

typedef struct _mcf_get_con_t
{
    unsigned int    to_use;
    unsigned int    diff_hue_shift;
    unsigned int    diff_hue_coff;
    unsigned int    diff_hue_shift_ex;
    unsigned int    max_diff;
    unsigned int    max_diff_inv;
} mcf_get_con_t;

typedef struct _mcf_fill_conflict_cfg_t
{
    unsigned int    to_use;
    unsigned int    luma_th; 
    unsigned int    conflict_th; 
} mcf_fill_conflict_cfg_t;

typedef struct _mcf_smooth_blend_cfg_t
{
    unsigned int    to_use;
    unsigned int    gauss_radius; 
    unsigned int    mask_blend; 
} mcf_smooth_blend_cfg_t;

typedef struct _mcf_smooth_blend_gf_cfg_t
{
    unsigned int    to_use;
    unsigned int    uv_gf_radius; 
    unsigned int    uv_gf_eps; 
} mcf_smooth_blend_gf_cfg_t;

typedef struct _mcf_adjust_col_gf_t
{
    unsigned int    to_use;
    unsigned int    y_gf_radius;
    unsigned int    y_gf_eps;
} mcf_adjust_col_gf_t;

typedef struct _mcf_adjust_get_mask_t
{
    unsigned int    to_use;
    unsigned int    mask_sub;
    unsigned int    mask_mult;
} mcf_adjust_get_mask_t;

typedef struct _mcf_adjust_get_ratio_t
{
    unsigned int    to_use;
    unsigned int    ratio_par_down; 
    unsigned int    ratio_par_up; 
} mcf_adjust_get_ratio_t;

typedef struct _mcf_uv_fusion_t
{
    unsigned int    to_use;
    mcf_get_can_t      get_can; 
    mcf_get_final_uv_t get_final_uv;
    mcf_get_con_t      get_con;
} mcf_uv_fusion_t;

typedef struct _mcf_mcf_uv_t
{
    unsigned int    to_use;
    mcf_fill_conflict_cfg_t fill_conflict_cfg;
    mcf_smooth_blend_cfg_t           smooth_blend_cfg; 
    mcf_smooth_blend_gf_cfg_t    smooth_blend_gf; 
} mcf_mcf_uv_t;

typedef struct _mcf_adjust_y_t
{
    unsigned int    to_use;
    mcf_adjust_col_gf_t   adjust_col_gf; 
    mcf_adjust_get_mask_t adjust_get_mask;
    mcf_adjust_get_ratio_t adjust_get_ratio;
} mcf_adjust_y_t;

typedef struct _mcf_config_table_t
{
    unsigned int    to_use;
    mcf_basic_cfg_t basic_cfg;
    mcf_size_cfg_t size_cfg;
    mcf_uv_fusion_t   uv_fusion; 
    mcf_mcf_uv_t mcf_uv;
    mcf_adjust_y_t adjust_y;
} mcf_config_table_t;
#endif /*__CFG_TABLE_MCF_H__*/

/********************************** END **********************************/

