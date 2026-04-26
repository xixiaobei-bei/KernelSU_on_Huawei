#ifndef _SLAM_CS_H
#define _SLAM_CS_H

#include "ipp.h"
#include "slam_common.h"

typedef enum _slam_buf_usage_e {
	BI_SLAM_Y = 0,
	BO_SLAM_Y,
	BO_SLAM_STAT,
	BO_SLAM_SCORE_MAP,
	BO_SLAM_SCORE_HIST,
	BO_SLAM_KPT_NUM,
	BO_SLAM_TOTAL_KPT,
	SLAM_STREAM_MAX,
} slam_buf_usage_e;

typedef struct _slam_req_ctrl_cfg_t {
	unsigned int   pyramid_en;
	unsigned int   gsblur_en;
	unsigned int   fast_en;
	unsigned int   nms_en;
	unsigned int   orient_en;
	unsigned int   brief_en;
	unsigned int   freak_en;
	unsigned int   scoremap_en;
	unsigned int   gridstat_en;
	unsigned int   undistort_en;
} slam_req_ctrl_cfg_t;

typedef struct _slam_req_size_cfg_t {
	unsigned int   width;      //IMG_WIDTH-1
	unsigned int   height;     //IMG_HEIGHT-1
} slam_req_size_cfg_t;

typedef struct _slam_req_pyramid_scaler_cfg_t {
	unsigned int  scl_inc;
	unsigned int  scl_vbottom;
	unsigned int  scl_vtop;
	unsigned int  scl_hright;
	unsigned int  scl_hleft;
} slam_req_pyramid_scaler_cfg_t;

typedef struct _slam_req_gsblur_coef_cfg_t {
	unsigned int   coeff_gauss_0;
	unsigned int   coeff_gauss_1;
	unsigned int   coeff_gauss_2;
	unsigned int   coeff_gauss_3;
} slam_req_gsblur_coef_cfg_t;

typedef struct _slam_req_fast_coef_cfg_t {
	unsigned int   ini_th;
	unsigned int   min_th;
} slam_req_fast_coef_cfg_t;

typedef struct _slam_req_nms_win_cfg_t {
	unsigned int   nmscell_h;
	unsigned int   nmscell_v;
} slam_req_nms_win_cfg_t;

typedef struct _slam_req_gridstat_cfg_t {
	unsigned int  blk_h_num;
	unsigned int  blk_v_num;
	unsigned int  blk_h_size;
	unsigned int  blk_v_size;
} slam_req_gridstat_cfg_t;

typedef struct _slam_req_octree_cfg_t {
	unsigned int  max_kpnum;
	unsigned int  grid_max_kpnum;
	unsigned int  inc_level;
	unsigned int  score_th[SLAM_SCORE_TH_RANGE];
} slam_req_octree_cfg_t;

typedef struct _slam_req_undistort_cfg_t {
	unsigned int  cx;
	unsigned int  cy;
	unsigned int  fx;
	unsigned int  fy;
	unsigned int  invfx;
	unsigned int  invfy;
	unsigned int  k1;
	unsigned int  k2;
	unsigned int  k3;
	unsigned int  p1;
	unsigned int  p2;
} slam_req_undistort_cfg_t;

typedef struct _slam_req_pattern_cfg_t {
	unsigned int  pattern_x0[SLAM_PATTERN_RANGE];
	unsigned int  pattern_y0[SLAM_PATTERN_RANGE];
	unsigned int  pattern_x1[SLAM_PATTERN_RANGE];
	unsigned int  pattern_y1[SLAM_PATTERN_RANGE];
} slam_req_pattern_cfg_t;

typedef struct _slam_reg_cfg_t {
	slam_req_ctrl_cfg_t           ctrl;
	slam_req_size_cfg_t           size_cfg;
	slam_req_pyramid_scaler_cfg_t pyramid_scaler_cfg;
	slam_req_gsblur_coef_cfg_t    gsblur_coef_cfg;
	slam_req_fast_coef_cfg_t      fast_coef;
	slam_req_nms_win_cfg_t        nms_win;
	slam_req_gridstat_cfg_t       gridstat_cfg;
	slam_req_octree_cfg_t         octree_cfg;
	slam_req_undistort_cfg_t      undistort_cfg;
	slam_req_pattern_cfg_t        pattern_cfg;
} slam_reg_cfg_t;

typedef struct _req_orb_t {
	slam_stream_t   streams[SLAM_LAYER_MAX][SLAM_STREAM_MAX];
	slam_reg_cfg_t  reg_cfg[SLAM_LAYER_MAX];
	unsigned int    secure_flag;
	unsigned int    slam_rate_value;
} req_orb_t;

typedef struct _msg_req_slam_request_t {
	unsigned int  frame_num;
	unsigned int  slam_pyramid_layer;
	req_orb_t  req_orb;
} msg_req_slam_request_t;

int slam_request_handler(msg_req_slam_request_t *slam_request);

#endif
