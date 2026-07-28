#ifndef _SLAMTEST_H
#define _SLAMTEST_H

#include "ipp.h"
#include "slam_common.h"

typedef struct _slamtest_stream_t
{
    unsigned int width;
    unsigned int height;
    unsigned int buf;
    pix_format_e format;
} slamtest_stream_t;

typedef struct _slamtest_config_t
{
    unsigned int scoremap_en;
    unsigned int freak_en;
    unsigned int brief_en;
    unsigned int orient_en;
    unsigned int nms_en;
    unsigned int fast_en;
    unsigned int gsblur_en;
    unsigned int pyramid_en;
} slamtest_config_t;

typedef struct
{
    unsigned     scl_inc; 
    unsigned     scl_vbottom; 
    unsigned     scl_vtop; 
    unsigned     scl_hright; 
    unsigned     scl_hleft;
} slamtest_pyramid_cfg_t;

typedef struct
{
    unsigned int    coeff_gauss_0;
    unsigned int    coeff_gauss_1;
    unsigned int    coeff_gauss_2;
    unsigned int    coeff_gauss_3;
} slamtest_gsblur_coef_cfg_t;

typedef struct
{
    unsigned int    ini_th;
    unsigned int    min_th;
} slamtest_threshold_cfg_t;

typedef struct
{
    unsigned int    nmscell_h; 
    unsigned int    nmscell_v; 
} slamtest_nms_win_cfg_t;

typedef struct
{
    int    pattern_y1; 
    int    pattern_x1;
    int    pattern_y0;
    int    pattern_x0;
} slamtest_pattern_cfg_t;

typedef struct _msg_req_slamtest_request_t
{
    unsigned int       frame_number;
    unsigned int       pyramid_layer_num;
    slamtest_stream_t      streams[SLAM_STREAM_MAX];
    slamtest_config_t      basic_cfg;
    slamtest_pyramid_cfg_t      pyramid_cfg;
    slamtest_gsblur_coef_cfg_t  gsblur_coef_cfg;
    slamtest_threshold_cfg_t    threshold_cfg;
    slamtest_nms_win_cfg_t      nms_win_cfg;
    slamtest_pattern_cfg_t      pattern_cfg[SLAM_PATTERN_NUM];
} msg_req_slamtest_request_t;

int slamtest_request_handler(msg_req_slamtest_request_t* slam_test_request);
int slamtest_eof_handler(void);

#endif


