

#ifndef __HUT_MAIN_H__
#define __HUT_MAIN_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HUT_MAIN_H

/* 宏定义 */
#define HUT_ERR_LOG(_uc_vap_id, _puc_string)
#define HUT_ERR_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define HUT_ERR_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define HUT_ERR_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define HUT_ERR_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define HUT_ERR_VAR(_uc_vap_id, _c_fmt, ...)

#define HUT_WARNING_LOG(_uc_vap_id, _puc_string)
#define HUT_WARNING_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define HUT_WARNING_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define HUT_WARNING_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define HUT_WARNING_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define HUT_WARNING_VAR(_uc_vap_id, _c_fmt, ...)

#define HUT_INFO_LOG(_uc_vap_id, _puc_string)
#define HUT_INFO_LOG1(_uc_vap_id, _puc_string, _l_para1)
#define HUT_INFO_LOG2(_uc_vap_id, _puc_string, _l_para1, _l_para2)
#define HUT_INFO_LOG3(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3)
#define HUT_INFO_LOG4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define HUT_INFO_VAR(_uc_vap_id, _c_fmt, ...)

/* STRUCT定义 */
typedef struct {
    oal_uint8 *puc_base_addr_origin; /* 内存申请的起始地址 */
    oal_uint8 *puc_base_addr_align;  /* 4字节对齐后的起始地址 */
} hut_mem_addr_stru;

/* 私有帧头结构 */
typedef struct {
    oal_uint8 bit_flag : 1, /* 0: 不分段; 1: 分段 */
              bit_last : 1, /* 0: 中间的分段; 1: 最后一个分段 */
              bit_resv : 6;
    oal_uint8 uc_num;
    oal_uint16 us_len; /* 帧长(不包括帧头) */
} hut_frag_hdr_stru;

#endif /* end of hut_main.h */
