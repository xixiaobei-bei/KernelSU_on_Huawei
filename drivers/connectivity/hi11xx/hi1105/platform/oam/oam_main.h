

#ifndef __OAM_MAIN_H__
#define __OAM_MAIN_H__

#include "oal_types.h"
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#define OAM_BEACON_HDR_LEN 24
#define OAM_TIMER_MAX_LEN  36
#define OAM_PRINT_CRLF_NUM 20 /* 输出换行符的个数 */

#define OAM_FEATURE_NAME_ABBR_LEN 12 /* 特性名称缩写最大长度 */

/* 打印方式定义 */
typedef uint32_t (*oal_print_func)(const char *pc_string);

/* 变参数据格式定义 */
typedef int8_t *oam_va_list;

/* STRUCT定义 */
/* 日志特性列表结构体定义 */
typedef struct {
    oam_feature_enum_uint8 en_feature_id;
    uint8_t auc_feature_name_abbr[OAM_FEATURE_NAME_ABBR_LEN]; /* 以0结束 */
} oam_software_feature_stru;

extern oam_software_feature_stru g_oam_feature_list[];

extern int32_t oam_main_init(void);
extern void oam_main_exit(void);
extern uint32_t oam_set_output_type(oam_output_type_enum_uint8 en_output_type);
extern uint32_t oam_print_to_sdt(const char *pc_string);
extern uint32_t oam_print_to_console(const char *pc_string);
extern uint32_t oam_upload_log_to_sdt(int8_t *pc_string);
extern void oam_dump_buff_by_hex(uint8_t *puc_buff, int32_t l_len, int32_t l_num);

#endif /* end of oam_main */
