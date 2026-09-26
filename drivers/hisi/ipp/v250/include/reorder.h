#ifndef _REORDER_CS_H
#define _REORDER_CS_H

#include "ipp.h"
#include "slam_common.h"
#include "matcher.h"

typedef struct _reorder_req_prefetch_cfg_t {
	unsigned int  prefetch_enable;
	unsigned int  first_32k_page;
} reorder_req_prefetch_cfg_t;

typedef struct _reorder_req_reg_cfg_t {
	reorder_req_ctrl_cfg_t     reorder_ctrl_cfg;
	reorder_req_block_cfg_t    reorder_block_cfg;
	reorder_req_prefetch_cfg_t reorder_prefetch_cfg;
	unsigned int           reorder_kpt_num[MATCHER_KPT_NUM];

	unsigned int           reorder_kpt_addr;//address in cmdlst_buffer
} reorder_req_reg_cfg_t;

typedef struct _msg_req_reorder_request_t {
	unsigned int      frame_number;
	slam_stream_t     streams[RDR_STREAM_MAX];
	reorder_req_reg_cfg_t reg_cfg;
	unsigned int rdr_rate_value;
} msg_req_reorder_request_t;

int reorder_request_handler(msg_req_reorder_request_t *reorder_request);

#endif
