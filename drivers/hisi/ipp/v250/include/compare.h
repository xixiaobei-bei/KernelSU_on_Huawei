#ifndef _COMPARE_CS_H
#define _COMPARE_CS_H

#include "ipp.h"
#include "slam_common.h"
#include "matcher.h"

typedef enum _compare_buf_e {
	BI_COMPARE_DESC_REF,
	BI_COMPARE_DESC_CUR,
	COMPARE_STREAM_MAX,
} compare_buf_e;

typedef struct _compare_req_prefetch_cfg_t {
	unsigned int  prefetch_enable;
	unsigned int  first_32k_page;
} compare_req_prefetch_cfg_t;

typedef struct _compare_reg_cfg_t {
	compare_req_ctrl_cfg_t     compare_ctrl_cfg;
	compare_req_block_cfg_t    compare_block_cfg;
	compare_req_search_cfg_t   compare_search_cfg;
	compare_req_stat_cfg_t     compare_stat_cfg;
	compare_req_prefetch_cfg_t compare_prefetch_cfg;
	compare_req_kptnum_cfg_t   compare_kptnum_cfg;

	unsigned int           compare_ref_kpt_addr;//address in cmdlst_buffer
	unsigned int           compare_cur_kpt_addr;//address in cmdlst_buffer
} compare_req_reg_cfg_t;

typedef struct _msg_req_compare_request_t {
	unsigned int      frame_number;
	slam_stream_t     streams[COMPARE_STREAM_MAX];
	unsigned short int      *compare_index;
	unsigned short int      *compare_matched_kpt;
	compare_req_reg_cfg_t reg_cfg;
	unsigned int cmp_rate_value;
} msg_req_compare_request_t;

int compare_request_handler(msg_req_compare_request_t *compare_request);
int compare_eof_handler(msg_req_compare_request_t *compare_request, unsigned int matched_kpt_num);
#endif
