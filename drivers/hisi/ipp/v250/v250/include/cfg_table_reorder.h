
#ifndef __CFG_TABLE_REORDER_CS_H__
#define __CFG_TABLE_REORDER_CS_H__

#include "slam_common.h"

#define REORDER_KPT_NUM          (94)

typedef struct _reorder_ctrl_cfg_t {
	unsigned int to_use;

	unsigned int  reorder_en;
	unsigned int  descriptor_type;
	unsigned int  total_kpt;
} reorder_ctrl_cfg_t;

typedef struct _reorder_block_cfg_t {
	unsigned int to_use;

	unsigned int  blk_v_num;
	unsigned int  blk_h_num;
	unsigned int  blk_num;
} reorder_block_cfg_t;

typedef struct _reorder_prefetch_cfg_t {
	unsigned int to_use;

	unsigned int  prefetch_enable;
	unsigned int  first_32k_page;
} reorder_prefetch_cfg_t;

typedef struct _reorder_kptnum_cfg_t {
	unsigned int to_use;

	unsigned int reorder_kpt_num[REORDER_KPT_NUM];
} reorder_kptnum_cfg_t;

typedef struct _cfg_tab_reorder_t {
	reorder_ctrl_cfg_t     reorder_ctrl_cfg;
	reorder_block_cfg_t    reorder_block_cfg;
	reorder_prefetch_cfg_t reorder_prefetch_cfg;
	reorder_kptnum_cfg_t   reorder_kptnum_cfg;

	unsigned int           reorder_kpt_addr;//address in cmdlst_buffer
} cfg_tab_reorder_t;

#endif/*__CFG_TABLE_REORDER_CS_H__*/
