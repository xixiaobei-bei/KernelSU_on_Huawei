
#ifndef __CFG_TABLE_COMPARE_CS_H__
#define __CFG_TABLE_COMPARE_CS_H__

#include "slam_common.h"

#define COMPARE_KPT_NUM          (94)

typedef struct _comapre_ctrl_cfg_t {
	unsigned int to_use;

	unsigned int  compare_en;
	unsigned int  descriptor_type;
} compare_ctrl_cfg_t;

typedef struct _compare_block_cfg_t {
	unsigned int to_use;

	unsigned int  blk_v_num;
	unsigned int  blk_h_num;
	unsigned int  blk_num;
} compare_block_cfg_t;

typedef struct _compare_search_cfg_t {
	unsigned int to_use;

	unsigned int  v_radius;
	unsigned int  h_radius;
	unsigned int  dis_ratio;
	unsigned int  dis_threshold;
} compare_search_cfg_t;

typedef struct _compare_stat_cfg_t {
	unsigned int to_use;

	unsigned int  stat_en;
	unsigned int  max3_ratio;
	unsigned int  bin_factor;
	unsigned int  bin_num;
} compare_stat_cfg_t;

typedef struct _compare_prefetch_cfg_t {
	unsigned int to_use;

	unsigned int  prefetch_enable;
	unsigned int  first_32k_page;
} compare_prefetch_cfg_t;

typedef struct _compare_kptnum_cfg_t {
	unsigned int to_use;

	unsigned int compare_ref_kpt_num[COMPARE_KPT_NUM];
	unsigned int compare_cur_kpt_num[COMPARE_KPT_NUM];
} compare_kptnum_cfg_t;

typedef struct _cfg_tab_compare_t {
	compare_ctrl_cfg_t     compare_ctrl_cfg;
	compare_block_cfg_t    compare_block_cfg;
	compare_search_cfg_t   compare_search_cfg;
	compare_stat_cfg_t     compare_stat_cfg;
	compare_prefetch_cfg_t compare_prefetch_cfg;
	compare_kptnum_cfg_t   compare_kptnum_cfg;

	unsigned int           compare_ref_kpt_addr;//address in cmdlst_buffer
	unsigned int           compare_cur_kpt_addr;//address in cmdlst_buffer
} cfg_tab_compare_t;

#endif/*__CFG_TABLE_COMPARE_CS_H__*/
