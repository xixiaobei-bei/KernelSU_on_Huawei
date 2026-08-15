#ifndef _SLAM_H
#define _SLAM_H

#include "ipp.h"
#include "slam_common.h"

typedef struct _slam_stream_t
{
    unsigned int width;
    unsigned int height;
    unsigned int stride;
    unsigned int buf;
    pix_format_e format;
} slam_stream_t;

typedef struct _slam_config_t
{
    unsigned int scoremap_en;
    unsigned int freak_en;
    unsigned int brief_en;
    unsigned int orient_en;
    unsigned int nms_en;
    unsigned int fast_en;
    unsigned int gsblur_en;
} slam_config_t;

typedef struct
{
    unsigned int    pyramid_en;
    unsigned int    scl_inc; 
    unsigned int    scl_vbottom; 
    unsigned int    scl_vtop; 
    unsigned int    scl_hright; 
    unsigned int    scl_hleft;
} pyramid_cfg_t;

typedef struct
{
    unsigned int    coeff_gauss_0;
    unsigned int    coeff_gauss_1;
    unsigned int    coeff_gauss_2;
    unsigned int    coeff_gauss_3;
} gsblur_coef_cfg_t;

typedef struct
{
    unsigned int    ini_th;
    unsigned int    min_th;
} threshold_cfg_t;

typedef struct
{
    unsigned int    nmscell_h; 
    unsigned int    nmscell_v; 
} nms_win_cfg_t;

typedef struct
{
    int    pattern_y1; 
    int    pattern_x1;
    int    pattern_y0;
    int    pattern_x0;
} pattern_cfg_t;

typedef struct _msg_req_slam_request_t
{
    unsigned int       frame_number;
    unsigned int       pyramid_layer_num;
    slam_stream_t      streams[MAX_PYRAMID_LAYER][SLAM_STREAM_MAX];
    slam_config_t      basic_cfg;
    pyramid_cfg_t      pyramid_cfg[MAX_PYRAMID_LAYER];
    gsblur_coef_cfg_t  gsblur_coef_cfg;
    threshold_cfg_t    threshold_cfg;
    nms_win_cfg_t      nms_win_cfg[MAX_PYRAMID_LAYER];
    pattern_cfg_t      pattern_cfg[SLAM_PATTERN_NUM];
    unsigned short int      *score_hist;	
    unsigned int  secure_flag;
    unsigned int  slam_rate_value;
} msg_req_slam_request_t;

int slam_request_handler(msg_req_slam_request_t* slam_request);

int slam_eof_handler(msg_req_slam_request_t* slam_request);

#endif

