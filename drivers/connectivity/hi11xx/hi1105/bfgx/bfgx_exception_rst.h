

#ifndef __BFGX_EXCEPTION_RST_H__
#define __BFGX_EXCEPTION_RST_H__

/* 其他头文件包含 */
#include "plat_exception_rst.h"
#include "hw_bfg_ps.h"
#include "oal_workqueue.h"

/* plat cfg cmd */
#define PLAT_CFG_IOC_MAGIC                   'z'
#define PLAT_DFR_CFG_CMD                     _IOW(PLAT_CFG_IOC_MAGIC, PLAT_DFR_CFG, int)
#define PLAT_BEATTIMER_TIMEOUT_RESET_CFG_CMD _IOW(PLAT_CFG_IOC_MAGIC, PLAT_BEATTIMER_TIMEOUT_RESET_CFG, int)

#define PLAT_EXCEPTION_ENABLE  1
#define PLAT_EXCEPTION_DISABLE 0

#define HI110X_DFR_WAIT_SSI_IDLE_MS 5000

#ifdef HISI_TOP_LOG_DIR
#define BFGX_DUMP_PATH HISI_TOP_LOG_DIR "/hi110x/memdump"
#define WIFI_DUMP_PATH HISI_TOP_LOG_DIR "/wifi/memdump"
#else
#define BFGX_DUMP_PATH "/data/memdump"
#define WIFI_DUMP_PATH "/data/memdump"
#endif

#define PLAT_BFGX_EXCP_CFG_IOC_MAGIC 'b'
#define DFR_HAL_GNSS_CFG_CMD         _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_GNSS, int)
#define DFR_HAL_BT_CFG_CMD           _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_BT, int)
#define DFR_HAL_FM_CFG_CMD           _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_FM, int)
#define DFR_HAL_IR_CFG_CMD           _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, DFR_HAL_IR, int)
#define PLAT_BFGX_DUMP_FILE_READ_CMD _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, PLAT_BFGX_DUMP_FILE_READ, int)
#define PLAT_ME_DUMP_FILE_READ_CMD   _IOW(PLAT_BFGX_EXCP_CFG_IOC_MAGIC, PLAT_ME_DUMP_FILE_READ, int)

typedef enum {
    DFR_HAL_GNSS = 0,
    DFR_HAL_BT = 1,
    DFR_HAL_FM = 2,
    DFR_HAL_IR = 3,
    PLAT_BFGX_DUMP_FILE_READ = 4,
    PLAT_ME_DUMP_FILE_READ = 5,

    PLAT_BFGX_EXCP_CMD_BOTT
} plat_bfgx_excp_cmd;

#define PLAT_WIFI_EXCP_CFG_IOC_MAGIC 'w'
#define PLAT_WIFI_DUMP_FILE_READ_CMD _IOW(PLAT_WIFI_EXCP_CFG_IOC_MAGIC, PLAT_WIFI_DUMP_FILE_READ, int)

typedef enum {
    DFR_HAL_WIFI = 0,
    PLAT_WIFI_DUMP_FILE_READ = 1
} plat_wifi_excp_cmd;

typedef struct {
    struct sk_buff_head quenue;
    int32_t is_open;
    int32_t is_working;
    wait_queue_head_t dump_type_wait;
    struct sk_buff_head dump_type_queue;
} memdump_info_t;

extern memdump_info_t g_bcpu_memdump_cfg;
extern memdump_info_t g_gcpu_memdump_cfg;
extern memdump_info_t g_wcpu_memdump_cfg;

enum plat_cfg {
    PLAT_DFR_CFG = 0,
    PLAT_BEATTIMER_TIMEOUT_RESET_CFG = 1,
    PLAT_CFG_BUFF,

    PLAT_DUMP_FILE_READ = 100,
    PLAT_DUMP_ROTATE_FINISH = 101
};

void plat_dfr_cfg_set(unsigned long arg);
void plat_beat_timeout_reset_set(unsigned long arg);
int32_t plat_get_excp_total_cnt(void);
int32_t plat_get_dfr_sinfo(char *buf, int32_t index);
int32_t is_dfr_test_en(enum excp_test_cfg_em excp_cfg);
void set_excp_test_en(enum excp_test_cfg_em excp_cfg);
void bfgx_memdump_finish(struct ps_core_s *ps_core_d);
void wifi_memdump_finish(void);
int32_t wifi_notice_hal_memdump(void);
int32_t bfgx_notice_hal_memdump(struct ps_core_s *ps_core_d);
int32_t bfgx_memdump_enquenue(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint16_t count);
int32_t wifi_memdump_enquenue(uint8_t *buf_ptr, uint16_t count);
oal_workqueue_stru *wifi_get_exception_workqueue(void);
int32_t plat_exception_handler(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type);
#endif
