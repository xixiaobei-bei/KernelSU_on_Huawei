
#ifndef __CFG_TABLE_SLAM_CS_H__
#define __CFG_TABLE_SLAM_CS_H__

#include "slam_common.h"

typedef struct _slam_ctrl_cfg_t {
	unsigned int to_use;

	unsigned int pyramid_en;
	unsigned int gsblur_en;
	unsigned int fast_en;
	unsigned int nms_en;
	unsigned int orient_en;
	unsigned int brief_en;
	unsigned int freak_en;
	unsigned int scoremap_en;
	unsigned int gridstat_en;
	unsigned int undistort_en;
} slam_ctrl_cfg_t;

typedef struct _slam_size_cfg_t {
	unsigned int to_use;

	unsigned int width;
	unsigned int height;
} slam_size_cfg_t;

typedef struct _slam_pyramid_scaler_cfg_t {
	unsigned int to_use;

	unsigned int scl_inc;
	unsigned int scl_vbottom;
	unsigned int scl_vtop;
	unsigned int scl_hright;
	unsigned int scl_hleft;
} slam_pyramid_scaler_cfg_t;

typedef struct _slam_gsblur_coef_cfg_t {
	unsigned int to_use;

	unsigned int coeff_gauss_0;
	unsigned int coeff_gauss_1;
	unsigned int coeff_gauss_2;
	unsigned int coeff_gauss_3;
} slam_gsblur_coef_cfg_t;

typedef struct _slam_fast_coef_t {
	unsigned int to_use;

	unsigned int ini_th;
	unsigned int min_th;
} slam_fast_coef_t;

typedef struct _slam_nms_win_t {
	unsigned int to_use;

	unsigned int nmscell_h;
	unsigned int nmscell_v;
} slam_nms_win_t;

typedef struct _slam_block_num_t {
	unsigned int blk_h_num;
	unsigned int blk_v_num;
} slam_block_num_t;

typedef struct _slam_block_size_t {
	unsigned int blk_h_size;
	unsigned int blk_v_size;
} slam_block_size_t;

typedef struct _slam_gridstat_cfg_t {
	unsigned int to_use;

	slam_block_num_t  block_num;
	slam_block_size_t block_size;
} slam_gridstat_cfg_t;

typedef struct _slam_octree_cfg_t {
	unsigned int to_use;

	unsigned int max_kpnum;
	unsigned int grid_max_kpnum;
	unsigned int inc_level;
	unsigned int score_th[SLAM_SCORE_TH_RANGE];
} slam_octree_cfg_t;

typedef struct _slam_undistort_cfg_t {
	unsigned int to_use;

	unsigned int cx;
	unsigned int cy;
	unsigned int fx;
	unsigned int fy;
	unsigned int invfx;
	unsigned int invfy;
	unsigned int k1;
	unsigned int k2;
	unsigned int k3;
	unsigned int p1;
	unsigned int p2;
} slam_undistort_cfg_t;

typedef struct _slam_pattern_cfg_t {
	unsigned int pattern_x0[SLAM_PATTERN_RANGE];
	unsigned int pattern_y0[SLAM_PATTERN_RANGE];
	unsigned int pattern_x1[SLAM_PATTERN_RANGE];
	unsigned int pattern_y1[SLAM_PATTERN_RANGE];
} slam_pattern_cfg_t;

typedef struct _slam_cvdr_cfg_t {
	unsigned int to_use;

	unsigned int vprd_horizontal_blanking;
	unsigned int vprd_line_wrap;
} slam_cvdr_cfg_t;

typedef struct _cfg_tab_slam_t {
	slam_ctrl_cfg_t           ctrl;
	slam_size_cfg_t           size_cfg;
	slam_pyramid_scaler_cfg_t pyramid_scaler_cfg;
	slam_gsblur_coef_cfg_t    gsblur_coef_cfg;
	slam_fast_coef_t          fast_coef;
	slam_nms_win_t            nms_win;
	slam_gridstat_cfg_t       gridstat_cfg;
	slam_octree_cfg_t         octree_cfg;
	slam_undistort_cfg_t      undistort_cfg;
	slam_pattern_cfg_t        pattern_cfg;
	slam_cvdr_cfg_t           cvdr_cfg;
} cfg_tab_slam_t;

#endif/*__CFG_TABLE_SLAM_CS_H__*/
