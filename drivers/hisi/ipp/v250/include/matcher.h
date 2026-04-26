#ifndef _MATCHER_CS_H
#define _MATCHER_CS_H

#include "ipp.h"
#include "slam_common.h"

#define MATCHER_KPT_NUM          (94)

typedef enum _rdr_buf_usage_e {
	BI_RDR_FP = 0,
	BO_RDR_FP_BLOCK,
	RDR_STREAM_MAX,
} rdr_buf_usage_e;

typedef struct _reorder_req_ctrl_cfg_t {
	unsigned int   reorder_en;
	unsigned int   descriptor_type;
	unsigned int   total_kpt;
} reorder_req_ctrl_cfg_t;

typedef struct _reorder_req_block_cfg_t {
	unsigned int   blk_v_num;
	unsigned int   blk_h_num;
	unsigned int   blk_num;
} reorder_req_block_cfg_t;

typedef struct _req_rdr_t {
	slam_stream_t              streams[SLAM_LAYER_MAX][RDR_STREAM_MAX];
	reorder_req_ctrl_cfg_t     ctrl_cfg[SLAM_LAYER_MAX];
	reorder_req_block_cfg_t    block_cfg[SLAM_LAYER_MAX];
	unsigned int               reorder_kpt_num[SLAM_LAYER_MAX][MATCHER_KPT_NUM];
} req_rdr_t;

typedef enum _cmp_buf_usage_e {
	BI_CMP_REF_FP = 0,
	BI_CMP_CUR_FP,
	BO_CMP_MATCHED_OUT,
	CMP_STREAM_MAX,
} cmp_buf_usage_e;

typedef struct _compare_req_ctrl_cfg_t {
	unsigned int  compare_en;
	unsigned int  descriptor_type;
} compare_req_ctrl_cfg_t;

typedef struct _compare_req_block_cfg_t {
	unsigned int  blk_v_num;
	unsigned int  blk_h_num;
	unsigned int  blk_num;
} compare_req_block_cfg_t;

typedef struct _compare_req_search_cfg_t {
	unsigned int  v_radius;
	unsigned int  h_radius;
	unsigned int  dis_ratio;
	unsigned int  dis_threshold;
} compare_req_search_cfg_t;

typedef struct _compare_req_stat_cfg_t {
	unsigned int  stat_en;
	unsigned int  max3_ratio;
	unsigned int  bin_factor;
	unsigned int  bin_num;
} compare_req_stat_cfg_t;

typedef struct _compare_req_kptnum_cfg_t {
	unsigned int   compare_ref_kpt_num[MATCHER_KPT_NUM];
	unsigned int   compare_cur_kpt_num[MATCHER_KPT_NUM];
} compare_req_kptnum_cfg_t;

typedef struct _req_cmp_t {
	slam_stream_t            streams[SLAM_LAYER_MAX][CMP_STREAM_MAX];
	compare_req_ctrl_cfg_t   ctrl_cfg[SLAM_LAYER_MAX];
	compare_req_block_cfg_t  block_cfg[SLAM_LAYER_MAX];
	compare_req_search_cfg_t search_cfg[SLAM_LAYER_MAX];
	compare_req_stat_cfg_t   stat_cfg[SLAM_LAYER_MAX];
	compare_req_kptnum_cfg_t kptnum_cfg[SLAM_LAYER_MAX];
} req_cmp_t;

typedef struct _msg_req_matcher_request_t {
	unsigned int  frame_number;
	unsigned int  rdr_pyramid_layer;
	unsigned int  cmp_pyramid_layer;
	unsigned int  work_mode;
	req_rdr_t     req_rdr;
	req_cmp_t     req_cmp;
	unsigned int  matcher_rate_value;      //0=TURBO,1=NORMAL,2=SVS, 3=LOW
} msg_req_matcher_request_t;

int matcher_request_handler(msg_req_matcher_request_t *matcher_request);
int matcher_eof_handler(msg_req_matcher_request_t *matcher_request);

#endif