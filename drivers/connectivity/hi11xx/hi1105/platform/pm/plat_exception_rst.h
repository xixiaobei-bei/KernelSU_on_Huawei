

#ifndef __PLAT_EXCEPTION_RST_H__
#define __PLAT_EXCEPTION_RST_H__

/* 其他头文件包含 */
#include "plat_type.h"
#include "oal_net.h"
#include "oal_workqueue.h"

/* 宏定义 */
#define WAIT_BFGX_READ_STACK_TIME 10000 /* 等待bfgx读栈操作完成的时间，5000毫秒 */
#define BFGX_BEAT_TIME            3     /* bfgx心跳超时时间为3秒钟 */

#define PLAT_EXCEPTION_RESET_IDLE 0 /* plat 没有在处理异常 */
#define PLAT_EXCEPTION_RESET_BUSY 1 /* plat 正在处理异常 */

#define BFGX_NOT_RECV_BEAT_INFO 0 /* host没有收到心跳消息 */
#define BFGX_RECV_BEAT_INFO     1 /* host收到心跳消息 */

#define EXCEPTION_SUCCESS 0
#define EXCEPTION_FAIL    1

#define BEAT_TIMER_DELETE 0
#define BEAT_TIMER_RESET  1

#define DFR_TEST_DISABLE 0
#define DFR_TEST_ENABLE  1

#define BFGX_MAX_RESET_CMD_LEN 9
#define BT_RESET_CMD_LEN       4
#define FM_RESET_CMD_LEN       1
#define GNSS_RESET_CMD_LEN     9
#define IR_RESET_CMD_LEN       4
#define NFC_RESET_CMD_LEN      4

#define MEMDUMP_ROTATE_QUEUE_MAX_LEN 10

/* STRUCT 定义 */
enum dfr_rst_system_type_e {
    DFR_SINGLE_SYS_RST = 0,
    DFR_ALL_SYS_RST = 1,
    DFR_SYSTEM_RST_TYPE_BOTT
};

enum subsystem_enum {
    SUBSYS_WIFI = 0,
    SUBSYS_BFGX = 1,

    SUBSYS_BOTTOM
};

enum wifi_thread_enum {
    THREAD_WIFI = 0,

    WIFI_THREAD_BOTTOM
};

enum bfgx_thread_enum {
    THREAD_BT = 0,
    THREAD_FM = 1,
    THREAD_GNSS = 2,
    THREAD_IR = 3,
    THREAD_NFC = 4,
    THREAD_IDLE = 5,
    BFGX_THREAD_BOTTOM
};

enum exception_type_enum {
    /* bfgx */
    BFGX_BEATHEART_TIMEOUT = 0, /* 心跳超时 */
    BFGX_LASTWORD_PANIC = 1,    /* CPU挂死，临终遗言 */
    BFGX_TIMER_TIMEOUT = 2,     /* 定时器超时 */
    BFGX_ARP_TIMEOUT = 3,       /* arp verifier超时，有发无收 */
    BFGX_POWERON_FAIL = 4,
    BFGX_WAKEUP_FAIL = 5,

    /* wifi */
    WIFI_WATCHDOG_TIMEOUT = 6, /* 看门狗超时 */
    WIFI_POWERON_FAIL = 7,
    WIFI_WAKEUP_FAIL = 8,
    WIFI_DEVICE_PANIC = 9, /* wcpu arm exception */
    WIFI_TRANS_FAIL = 10,  /* sdio read or write failed */
    SDIO_DUMPBCPU_FAIL,

    EXCEPTION_TYPE_BOTTOM
};

enum dump_cmd_type {
    CMD_READM_WIFI_SDIO = 0,
    CMD_READM_WIFI_UART = 1,
    CMD_READM_BFGX_UART = 2,
    CMD_READM_BFGX_SDIO = 3,
    CMD_READM_ME_UART = 4,
    CMD_READM_ME_SDIO = 5,

    CMD_DUMP_BUFF
};

enum excp_test_cfg_em {
    BFGX_POWEON_FAULT = 0,
    BFGX_POWEOFF_FAULT = 1,
    WIFI_WKUP_FAULT = 2,
    WIFI_POWER_ON_FAULT = 3,
    EXCP_TEST_CFG_BOTT
};

struct st_wifi_dump_mem_info {
    unsigned long mem_addr;
    uint32_t size;
    uint8_t *file_name;
};

struct st_wifi_dfr_callback {
    void (*wifi_recovery_complete)(void);
    void (*notify_wifi_to_recovery)(void);
};

struct st_bfgx_reset_cmd {
    uint32_t len;
    uint8_t cmd[BFGX_MAX_RESET_CMD_LEN];
};

struct excp_type_info_s {
    uint32_t excp_cnt;
    uint32_t fail_cnt;
    uint32_t rst_type_cnt[DFR_SYSTEM_RST_TYPE_BOTT];
    ktime_t stime;
    unsigned long long maxtime;
};

struct st_exception_info {
    uint32_t exception_reset_enable;
    uint32_t subsys_type;
    uint32_t thread_type;
    uint32_t excetion_type;

    atomic_t bfgx_beat_flag;
    atomic_t is_reseting_device;
    atomic_t is_memdump_runing;

    oal_workqueue_stru *plat_exception_rst_workqueue;
    oal_work_stru plat_exception_rst_work;
    oal_wakelock_stru plat_exception_rst_wlock;
    struct timer_list bfgx_beat_timer;

    struct completion wait_read_bfgx_stack;

    struct ps_plat_s *ps_plat_d;
    struct st_wifi_dfr_callback *wifi_callback;

    oal_spin_lock_stru exception_spin_lock;

    struct excp_type_info_s etype_info[EXCEPTION_TYPE_BOTTOM];

    /* 下边的变量调试使用 */
    uint32_t debug_beat_flag;
    uint32_t subsystem_rst_en;

    /* wifi异常触发 */
    oal_work_stru wifi_excp_worker;
    oal_work_stru wifi_excp_recovery_worker;
    oal_workqueue_stru *wifi_exception_workqueue;
    uint32_t wifi_excp_type;
};

typedef struct excp_info_str_s {
    uint32_t id;
    char *name;
} excp_info_str_t;

/* EXTERN FUNCTION */
int32_t mod_beat_timer(uint8_t on);
int32_t is_bfgx_exception(void);
struct st_exception_info *get_exception_info_reference(void);
int32_t wait_bfgx_memdump_complete(void);
int32_t plat_exception_handler(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type);
int32_t plat_bfgx_exception_rst_register(struct ps_plat_s *data);
int32_t plat_wifi_exception_rst_register(void *data);
int32_t plat_exception_reset_init(void);
int32_t plat_exception_reset_exit(void);
int32_t wifi_exception_work_submit(uint32_t wifi_excp_type);
int32_t plat_power_fail_exception_info_set(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type);
void plat_power_fail_process_done(void);
int32_t bfgx_subsystem_reset(void);
int32_t bfgx_system_reset(void);
#endif
