

#ifndef __OAM_LOG_H__
#define __OAM_LOG_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"

/* 宏定义 */
#define OAM_LOG_PARAM_MAX_NUM     4   /* 可打印最多的参数个数 */
#define OAM_LOG_PRINT_DATA_LENGTH 512 /* 每次写入文件的最大长度 */

#define OAM_LOG_VAP_INDEX_INTERVAL 2 /* 驱动的索引和产品上层适配层的索引间隔 */

/* STRUCT定义 */
typedef struct {
    uint32_t bit16_file_id : 16;
    uint32_t bit8_feature_id : 8;
    uint32_t bit4_vap_id : 4;
    uint32_t bit4_log_level : 4;
} om_log_wifi_para_stru;

void oam_log_init(void);

#endif /* end of oam_log.h */
