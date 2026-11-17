

#ifndef __HW_BFG_PS_H__
#define __HW_BFG_PS_H__

/* 其他头文件包含 */
#include "oal_net.h"
#include "oal_schedule.h"
#include <linux/kernel.h>
#include <linux/types.h>
#include "oal_workqueue.h"
#include "plat_type.h"

/* 宏定义 */
#define BT_TX_MAX_FRAME   3000  /* tx bt data max lenth */
#define GNSS_TX_MAX_FRAME 30000 /* gnss send buf max lenth */
#define FM_TX_MAX_FRAME   512   /* fm cmd data max lenth */
#define NFC_TX_MAX_FRAME  512   /* nfc cmd data max lenth */
#define IR_TX_MAX_FRAME   5120  /* ir cmd data max lenth */
#define DBG_TX_MAX_FRAME  10000 /* dbg cmd data max lenth */

#define BT_RX_MAX_FRAME                1500  /* RX bt data max lenth */
#define FM_RX_MAX_FRAME                512   /* fm recv buf max lenth */
#define NFC_RX_MAX_FRAME               512   /* nfc recv buf max lenth */
#define IR_RX_MAX_FRAME                5120  /* ir recv buf max lenth */
#define DBG_RX_MAX_FRAME               10000 /* dbg recv buf max lenth */
#define GNSS_RX_MAX_FRAME              30000 /* gnss recv buf max lenth */

/* skb has skb_shared_info, and SKB_DATA_ALIGN */
#define GNSS_RX_SEPERATE_FRAME_MAX_LEN (4 * 1024 - SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) - SKB_DATA_ALIGN(1))

#define PS_TX_PACKET_LIMIT   200

#define HISI_UART_DEV_NAME_LEN 32

/* define for tx and rx packet queue */
#define TX_URGENT_QUEUE 0
#define TX_HIGH_QUEUE   1
#define TX_LOW_QUEUE    2
#define RX_GNSS_QUEUE   3
#define RX_FM_QUEUE     4
#define RX_BT_QUEUE     5
#define RX_DBG_QUEUE    6
#define RX_NFC_QUEUE    7
#define RX_IR_QUEUE     8

/* max number for tx and rx packet queue */
#define TX_HIGH_QUE_MAX_NUM 50
#define TX_LOW_QUE_MAX_NUM  250
#define RX_GNSS_QUE_MAX_NUM 50
#define RX_FM_QUE_MAX_NUM   50
#define RX_BT_QUE_MAX_NUM   50
#define RX_DBG_QUE_MAX_NUM  50
#define RX_NFC_QUE_MAX_NUM  50
#define RX_IR_QUE_MAX_NUM   50

/* timeout when after download patch */
#define WAIT_BFGX_BOOTOK_TIME 1000
#define WAIT_BT_OPEN_TIME     3000
#define WAIT_BT_CLOSE_TIME    3000
#define WAIT_FM_OPEN_TIME     3000
#define WAIT_FM_CLOSE_TIME    3000
#define WAIT_GNSS_OPEN_TIME   3000
#define WAIT_GNSS_CLOSE_TIME  3000
#define WAIT_IR_OPEN_TIME     3000
#define WAIT_IR_CLOSE_TIME    3000
#define WAIT_NFC_OPEN_TIME    1000
#define WAIT_NFC_CLOSE_TIME   2000

#define BFG_POWER_GPIO_DOWN 0
#define BFG_POWER_GPIO_UP   1

#define NONEED_SET_FLAG 0
#define NEED_SET_FLAG   1

#define DEV_THREAD_EXIT 0
#define DEV_THREAD_INIT 1

#define SYS_INF_HEARTBEAT_CMD_BASE 128

enum BFGN_DATA_MSG_TYPE_ENUM {
    SYS_MSG = 0x00,         /* 系统串口消息 */
    BT_MSG = 0x01,          /* BT串口消息 */
    GNSS_FIRST_MSG = 0x02,  /* GNSS串口消息，第一个分段消息 */
    GNSS_COMMON_MSG = 0x03, /* GNSS串口消息，中间分段消息 */
    GNSS_LAST_MSG = 0x04,   /* GNSS串口消息，最后一个分段消息 */
    FM_FIRST_MSG = 0x05,    /* FM串口消息，第一个分段消息 */
    FM_COMMON_MSG = 0x06,   /* FM串口消息，中间分段消息 */
    FM_LAST_MSG = 0x07,     /* FM串口消息，最后一个分段消息 */
    IR_FIRST_MSG = 0x08,    /* 红外串口消息，第一个分段消息 */
    IR_COMMON_MSG = 0x09,   /* 红外串口消息，中间分段消息 */
    IR_LAST_MSG = 0x0A,     /* 红外串口消息，最后一个分段消息 */
    NFC_FIRST_MSG = 0x0B,   /* NFC串口消息，第一个分段消息 */
    NFC_COMMON_MSG = 0x0C,  /* NFC串口消息，中间分段消息 */
    NFC_LAST_MSG = 0x0D,    /* NFC串口消息，最后一个分段消息 */
    OML_MSG = 0x0E,         /* 可维可测消息 */
    MEM_DUMP_SIZE = 0x0f,   /* bfgx异常时，要dump的mem长度消息 */
    MEM_DUMP = 0x10,        /* bfgx异常时，内存dump消息 */
    WIFI_MEM_DUMP = 0x11,   /* UART READ WIFI MEM，内存dump消息 */
    BFGX_CALI_MSG = 0x12,   /* BFGX 校准数据上报 */

    MSG_BUTT,
};

/* below is tx sys content define */
enum platform_event_enum {
    SYS_CFG_OPEN_BT = 0,
    SYS_CFG_CLOSE_BT = 1,
    SYS_CFG_OPEN_GNSS = 2,
    SYS_CFG_CLOSE_GNSS = 3,
    SYS_CFG_OPEN_FM = 4,
    SYS_CFG_CLOSE_FM = 5,
#ifdef HAVE_HISI_NFC
    SYS_CFG_OPEN_NFC = 6,
    SYS_CFG_CLOSE_NFC = 7,
#else
    PL_FAKE_CLOSE_BT_EVT = 7,
#endif
    SYS_CFG_OPEN_IR = 8,
    SYS_CFG_CLOSE_IR = 9,
    SYS_CFG_OPEN_WIFI = 0xa,        /* host通过uart打开WCPU */
    SYS_CFG_CLOSE_WIFI = 0xb,       /* host通过uart关闭WCPU */
    SYS_CFG_READ_STACK = 0xc,       /* host通过uart读栈 */
    SYS_CFG_QUERY_RF_TEMP = 0xd,    /* host通过uart查询rf温度 */
    SYS_CFG_ALLOWDEV_SLP = 0xe,     /* host allow device sleep */
    SYS_CFG_DISALLOW_SLP = 0xf,     /* host disallow device sleep */
    SYS_CFG_SHUTDOWN_SLP = 0x10,    /* shutdown bcpu */
    SYS_CFG_PL_ENABLE_PM = 0x11,    /* enable plat dev lowpower feature */
    SYS_CFG_PL_DISABLE_PM = 0x12,   /* disable plat dev lowpower feature */
    SYS_CFG_BT_ENABLE_PM = 0x13,    /* enable BT dev lowpower feature */
    SYS_CFG_BT_DISABLE_PM = 0x14,   /* disable BT dev lowpower feature */
    SYS_CFG_GNSS_ENABLE_PM = 0x15,  /* enable GNSS dev lowpower feature */
    SYS_CFG_GNSS_DISABLE_PM = 0x16, /* disable GNSS dev lowpower feature */
    SYS_CFG_NFC_ENABLE_PM = 0x17,   /* enable NFC dev lowpower feature */
    SYS_CFG_NFC_DISABLE_PM = 0x18,  /* disable NFC dev lowpower feature */

    SYS_CFG_DEV_PANIC = 0x19,       /* cause device into exception */
    SYS_CFG_DUMP_RESET_WCPU = 0x1a, /* host通过uart不掉电复位WCPU */

    SYS_CFG_HALT_WCPU = 0x1b,          /* halt WCPU */
    SYS_CFG_READ_WLAN_PUB_REG = 0x1c,  /* 读取wcpu的公共寄存器 */
    SYS_CFG_READ_WLAN_PRIV_REG = 0x1d, /* 读取wcpu的私有寄存器 */
    SYS_CFG_READ_WLAN_MEM = 0x1e,      /* 读取wcpu的mem */
    SYS_CFG_EXCEP_RESET_WCPU = 0x1f,   /* wifi DFR WCPU复位 */

    SYS_CFG_SET_UART_LOOP_HANDLER = 0x20, /* 设置device侧uart环回处理函数 */
    SYS_CFG_SET_UART_LOOP_FINISH = 0x21,  /* UART环回test ok，恢复gnss消息处理函数 */

    PL_PM_DEBUG = 0x22,          /* 低功耗维测开关 */
    PL_BAUT_CHG_REQ_ACK = 0x23,  /* 波特率切换请求ACK */
    PL_BAUT_CHG_COMPLETE = 0x24, /* 波特率切换完成 */

    SYS_CFG_NOTIFY_WIFI_OPEN = 0x25,  /* wifi open  通知 */
    SYS_CFG_NOTIFY_WIFI_CLOSE = 0x26, /* wifi close 通知 */
#ifdef _PRE_H2D_GPIO_WKUP
    SYS_CFG_ENABLE_GPIO_WKUP_BFG = 0x27, /* enable gpio wakeup bfg */
#endif

    PL_EVT_BUTT
};

/* below is rx sys content define from device */
enum SYS_INF_MSG_VALUE_ENUM {
    SYS_INF_PF_INIT = 0x00,                /* 平台软件初始化完成 */
    SYS_INF_BT_INIT = 0x01,                /* BT软件初始化完成 */
    SYS_INF_GNSS_INIT = 0x02,              /* GNSS软件初始化完成 */
    SYS_INF_FM_INIT = 0x03,                /* FM软件初始化完成 */
    SYS_INF_BT_DISABLE = 0x04,             /* BT禁能 */
    SYS_INF_GNSS_DISABLE = 0x05,           /* GNSS禁能 */
    SYS_INF_FM_DISABLE = 0x06,             /* FM禁能 */
    SYS_INF_BT_EXIT = 0x07,                /* BT退出 */
    SYS_INF_GNSS_EXIT = 0x08,              /* GNSS退出 */
    SYS_INF_FM_EXIT = 0x09,                /* FM退出 */
    SYS_INF_GNSS_WAIT_DOWNLOAD = 0x0A,     /* 等待GNSS代码下载 */
    SYS_INF_GNSS_DOWNLOAD_COMPLETE = 0x0B, /* GNSS代码下载完毕 */
    SYS_INF_BFG_HEART_BEAT = 0x0C,         /* 心跳信号 */
    SYS_INF_DEV_AGREE_HOST_SLP = 0x0D,     /* device回复host可睡 */
    SYS_INF_DEV_NOAGREE_HOST_SLP = 0x0E,   /* device回复host不可睡 */
    SYS_INF_WIFI_OPEN = 0x0F,              /* WCPU上电完成 */
    SYS_INF_IR_INIT = 0x10,                /* IR软件初始化完成 */
    SYS_INF_IR_EXIT = 0x11,                /* IR退出 */
#ifdef HAVE_HISI_NFC
    SYS_INF_NFC_INIT      =  0x12,         /* NFC软件初始化完成 */
    SYS_INF_NFC_EXIT      =  0x13,         /* NFC退出 */
#else
    SYS_INF_FAKE_CLOSE_BT =  0x13,         /* FAKE CLOSE BT的ACK */
#endif
    SYS_INF_WIFI_CLOSE = 0x14,             /* WCPU下电完成 */
    SYS_INF_RF_TEMP_NORMAL = 0x15,         /* RF温度正常 */
    SYS_INF_RF_TEMP_OVERHEAT = 0x16,       /* RF温度过热 */
    SYS_INF_MEM_DUMP_COMPLETE = 0x17,      /* bfgx异常时，MEM DUMP已完成 */
    SYS_INF_WIFI_MEM_DUMP_COMPLETE = 0X18, /* bfgx异常时，MEM DUMP已完成 */
    SYS_INF_UART_HALT_WCPU = 0x19,         /* uart halt wcpu ok */
    SYS_INF_UART_LOOP_SET_DONE = 0x1a,     /* device 设置uart环回ok */
    SYS_INF_CHR_ERRNO_REPORT = 0x1b,       /* device向host上报CHR异常码 */

    SYS_INF_RSV0 = 0x1c,                   /* rsv */
    SYS_INF_RSV1 = 0x1d,                   /* rsv */
    SYS_INF_RSV2 = 0x1e,                   /* rsv */

    SYS_INF_GNSS_LPPE_INIT = 0x1F,     /* 1103 GNSS 新增的线程初始化完成 */
    SYS_INF_GNSS_LPPE_EXIT = 0x20,     /* 1103 GNSS 新增的线程退出 */
    SYS_INF_GNSS_TRICKLE_SLEEP = 0x21, /* 1103 GNSS_TRICKLE_SLEEP */
#ifdef _PRE_H2D_GPIO_WKUP
    SYS_INF_ENABLE_GPIO_WKUP_BFG_RSP = 0x22,
#endif

    SYS_INF_BUTT, /* 枚举定义最大不能超过127，因为128~255被用来当做心跳的时间戳 */
};

typedef enum {
    BFGX_BT = 0x00,
    BFGX_FM = 0x01,
    BFGX_GNSS = 0x02,
    BFGX_IR = 0x03,
    BFGX_NFC = 0x04,
    BFGX_BUTT = 0x05,
} bfgx_subsys_type_enum;

#define WIFI_SUBSYS        BFGX_BUTT           /* wifi和BFG子系统统一编号 */
#define PLAT_SUBSYS        (WIFI_SUBSYS + 1)

#define HI110X_BT_SUB_MASK    (1<<BFGX_BT)
#define HI110X_FM_SUB_MASK    (1<<BFGX_FM)
#define HI110X_GNSS_SUB_MASK  (1<<BFGX_GNSS)
#define HI110X_IR_SUB_MASK    (1<<BFGX_IR)
#define HI110X_NFC_SUB_MASK   (1<<BFGX_NFC)
#define HI110X_WIFI_SUB_MASK  (1<<WIFI_SUBSYS)
#define HI110X_PLAT_SUB_MASK  (1<<PLAT_SUBSYS)

typedef enum {
    POWER_STATE_SHUTDOWN = 0,
    POWER_STATE_OPEN = 1,
    POWER_STATE_BUTT = 2,
} power_state_enum;

typedef enum {
    RX_SEQ_NULL = 0,
    RX_SEQ_START = 1,
    RX_SEQ_INT = 2,
    RX_SEQ_LAST = 3,

    RX_SEQ_BUTT,
} bfgx_rx_sepreted_pkt_type;

typedef enum {
    KZALLOC = 0,
    VMALLOC = 1,

    ALLOC_BUFF,
} alloc_type;

typedef enum {
    GNSS_SEPER_TAG_INIT,
    GNSS_SEPER_TAG_LAST,
} gnss_seperate_tag;

typedef enum {
    BUART = 0,
    GUART = 1,
    UART_BUTT,
} uart_enum;

struct bfgx_sepreted_rx_st {
    oal_spin_lock_stru sepreted_rx_lock;
    uint8_t *rx_buf_ptr;
    uint8_t *rx_buf_org_ptr;
    uint16_t rx_pkt_len;
    uint16_t rx_buf_all_len;
    uint8_t rx_prev_seq;
};

struct st_bfgx_data {
    oal_spin_lock_stru rx_lock;
    oal_netbuf_head_stru rx_queue;
    oal_completion wait_opened;
    oal_completion wait_closed;
    struct bfgx_sepreted_rx_st sepreted_rx; /* bfgx分包接收数据结构 */
    oal_wait_queue_head_stru rx_wait;              /* wait queue for rx packet */
    atomic_t subsys_state;
    const char *name;
    uint16_t read_delay;
    unsigned long rx_pkt_num;
    unsigned long tx_pkt_num;
};

/*
 * struct ps_core_s - PS core internal structure
 * @tty: tty provided by the TTY core for line disciplines.
 * @txq: the list of skbs which needs to be sent onto the TTY.
 * @pm_data: reference to the parent encapsulating structure.
 *
 */
struct ps_core_s {
    void *ps_plat;
    struct pm_drv_data *pm_data;;
    struct tty_struct *tty;
    struct st_bfgx_data bfgx_info[BFGX_BUTT];
    /* queue head define for all sk_buff */
    oal_netbuf_head_stru tx_high_seq;
    oal_netbuf_head_stru tx_low_seq;

    oal_netbuf_head_stru rx_dbg_seq;
    /* wait queue for rx packet */
    oal_wait_queue_head_stru rx_dbg_wait;
    /* force tx workqueue exit, release tty mutex */
    atomic_t force_tx_exit;
    atomic_t dbg_recv_dev_log;
    atomic_t dbg_read_delay;

    /* define for new workqueue */
    oal_workqueue_stru *ps_tx_workqueue;
    /* define new work struct */
    oal_work_stru tx_skb_work;

    oal_spin_lock_stru rx_lock;

    /* mark user doing something on node */
    atomic_t node_visit_flag;

    /* define variable for decode rx data from uart */
    uint8_t rx_pkt_type;
    uint16_t rx_pkt_total_len;
    uint16_t rx_have_recv_pkt_len;
    uint16_t rx_have_del_public_len;
    uint8_t *rx_decode_tty_ptr;
    uint8_t *rx_public_buf_org_ptr;
    uint8_t *rx_decode_public_buf_ptr;

    uint8_t tty_have_open;
    uint8_t dev_index;

    unsigned long rx_pkt_sys;
    unsigned long rx_pkt_oml;

    /* 统计tty数据的收发 */
    oal_mutex_stru tty_mutex;
    atomic_t tty_tx_cnt;
    atomic_t tty_rx_cnt;

    // 在SR流程中使用
    struct semaphore sr_wake_sema;
    atomic_t sr_cnt;
};

/*
 * struct ps_plat_s - the PM internal data, embedded as the
 *  platform's drv data. One for each PS device in the system.
 * @pm_pdev: the platform device added in one of the board-XX.c file
 *  in arch/XX/ directory, 1 for each PS device.
 * @core_data: PS core's data, which mainly is the tty's disc_data
 *
 */
struct ps_plat_s {
    struct platform_device *pm_pdev;
    struct ps_core_s *core_data;
    uint8_t dev_name[HISI_UART_DEV_NAME_LEN];
    unsigned long baud_rate;
    uint8_t flow_cntrl;
    uint16_t ldisc_num;
};

/*
 * struct hw_ps_plat_data - platform data shared between PS driver and
 *  platform specific board file which adds the PS device.
 * @dev_name: The UART/TTY name to which chip is interfaced. (eg: /dev/ttyS1)
 * @flow_cntrl: Should always be 1, since UART's CTS/RTS is used for PM
 *  purposes.
 * @baud_rate: The baud rate supported by the Host UART controller, this will
 *  be shared across with the chip via a HCI VS command from User-Space Init
 *  Mgr application.
 * @suspend:
 * @resume: legacy PM routines hooked to platform specific board file, so as
 *  to take chip-host interface specific action.
 * @set_bt_power: set gpio BT_EN to high.
 */
struct hw_ps_plat_data {
    int32_t (*suspend)(struct platform_device *, oal_pm_message_t);
    int32_t (*resume)(struct platform_device *);
    uint8_t dev_name[HISI_UART_DEV_NAME_LEN]; /* uart name */
    unsigned long baud_rate;
    uint8_t flow_cntrl; /* flow control flag */
    uint16_t ldisc_num;
    uint8_t fill_byte2;
};

#endif /* HW_BFG_PS_H */
