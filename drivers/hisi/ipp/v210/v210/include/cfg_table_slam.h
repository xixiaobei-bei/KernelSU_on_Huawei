 
#ifndef __CFG_TABLE_SLAM_H__
#define __CFG_TABLE_SLAM_H__

#include "slam_common.h"
typedef struct _slam_cfg_t
{
    unsigned int  to_use;
    unsigned int  pyramid_en;           
    unsigned int  gsblur_en;             
    unsigned int  fast_en;               
    unsigned int  nms_en;                
    unsigned int  orient_en;             
    unsigned int  brief_en;              
    unsigned int  freak_en;              
    unsigned int  scoremap_en;           
} slam_cfg_t;

typedef struct _slam_size_cfg_t
{
    unsigned int  width;
    unsigned int  height;
} slam_size_t;

typedef struct _pyramid_cfg_t
{
    unsigned int    to_use;
    unsigned int    scl_inc; 
    unsigned int    scl_vbottom; 
    unsigned int    scl_vtop; 
    unsigned int    scl_hright; 
    unsigned int    scl_hleft;
} slam_pyramid_param_cfg_t;

typedef struct _gsblur_coef_cfg_t
{
    unsigned int    to_use;
    unsigned int    coeff_gauss_0;
    unsigned int    coeff_gauss_1;
    unsigned int    coeff_gauss_2;
    unsigned int    coeff_gauss_3;
} slam_gsblur_coeff_cfg_t;

typedef struct _threshold_cfg_t
{
    unsigned int    to_use;
    unsigned int    ini_th;
    unsigned int    min_th;
} slam_threshold_param_cfg_t;

typedef struct _nms_win_cfg_t
{
    unsigned int    to_use;
    unsigned int    nmscell_h; 
    unsigned int    nmscell_v; 
} slam_nms_win_param_cfg_t;

typedef struct _score_his_cfg_t
{
    unsigned int    score_histbin[SLAM_SCOREHIST_NUM];
} slam_score_hist_cfg_t;

typedef struct _pattern_cfg_t                             
{
    unsigned   int    to_use;
    unsigned int      pattern_param[SLAM_PATTERN_NUM];
} slam_pattern_param_cfg_t;

typedef struct _slam_basic_cfg_t
{
    unsigned int  to_use;
    slam_cfg_t    slam_cfg;
    slam_size_t   slam_size;
} slam_basic_cfg_t;

typedef struct _slam_config_table_t
{
    unsigned int                to_use;
    slam_basic_cfg_t            basic_cfg;
    slam_pyramid_param_cfg_t    pyramid_param;
    slam_gsblur_coeff_cfg_t     gsblur_coeff;
    slam_threshold_param_cfg_t  threshold_param;
    slam_nms_win_param_cfg_t    nms_win_param;
    slam_pattern_param_cfg_t    pattern_param;
} slam_config_table_t;

#endif/*__CFG_TABLE_SLAM_H__*/

