

#ifndef __FRW_IPC_MSGQUEUE_H__
#define __FRW_IPC_MSGQUEUE_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_IPC_MSGQUEUE_H

/* 宏定义 */
#define FRW_IPC_MASTER_TO_SLAVE_QUEUE_MAX_NUM (1 << 7) /* 发送消息队列大小 必须为2的整数次方 */
#define FRW_IPC_SLAVE_TO_MASTER_QUEUE_MAX_NUM (1 << 8) /* 接受消息队列大小 必须为2的整数次方 */

/* 判断队列是否满 */
#define frw_ipc_ring_full(_head, _tail, _length) (((_tail) + 1) % (_length) == (_head))

/* 判断队列是否为空 */
#define frw_ipc_ring_empty(_head, _tail) ((_head) == (_tail))

/* 可翻转的队列头尾+1操作 */
#define frw_ipc_ring_incr(_val, _lim) ((_val) = (((_val) + 1) & ((_lim) - 1)))

/* 发送消息队列入队尾+1操作 */
#define frw_ipc_ring_tx_incr(_val) (frw_ipc_ring_incr((_val), (FRW_IPC_MASTER_TO_SLAVE_QUEUE_MAX_NUM)))

/* 接受消息队列入队头+1操作 */
#define frw_ipc_ring_rx_incr(_val) (frw_ipc_ring_incr((_val), (FRW_IPC_SLAVE_TO_MASTER_QUEUE_MAX_NUM)))

/* 核间通信丢包告警打印 */
#define frw_ipc_lost_warning_log1(_uc_vap_id, _puc_string, _l_para1)
#define frw_ipc_lost_warning_log2(_uc_vap_id, _puc_string, _l_para1, _l_para2)

/* 核间通信日志维测信息打印 */
#define frw_ipc_log_info_print1(_uc_vap_id, _puc_string, _l_para1)
#define frw_ipc_log_info_print4(_uc_vap_id, _puc_string, _l_para1, _l_para2, _l_para3, _l_para4)
#define MAX_LOG_RECORD 100 /* 日志记录最大条数 */

/* 枚举定义 */
typedef enum {
    FRW_IPC_TX_CTRL_ENABLED = 0,  /* 目标核空闲可以发送核间通讯 */
    FRW_IPC_TX_CTRL_DISABLED = 1, /* 目标核忙不可启动间通讯中断 */

    FRW_IPC_TX_BUTT
} frw_ipc_tx_ctrl_enum;

typedef oal_mem_stru frw_ipc_msg_mem_stru; /* 事件结构体内存块转定义 */

/* STRUCT 定义 */
typedef struct {
    frw_ipc_msg_mem_stru *pst_msg_mem;
} frw_ipc_msg_dscr_stru;

typedef struct {
    OAL_VOLATILE uint32_t ul_tail; /* 指向取出下一个消息位置 */
    OAL_VOLATILE uint32_t ul_head; /* 指向要插入下一个消息位置 */
    uint32_t ul_max_num;           /* 环形消息队列大小 */
    frw_ipc_msg_dscr_stru *pst_dscr; /* 环形消息描述符队列首地址 */
} frw_ipc_msg_queue_stru;

typedef struct {
    void (*p_tx_complete_func)(frw_ipc_msg_mem_stru *); /* 发送完成回调处理 */
    void (*p_rx_complete_func)(frw_ipc_msg_mem_stru *); /* 接收回调处理 */
} frw_ipc_msg_callback_stru;

typedef struct {
    uint16_t us_seq_num;     /* 接收消息的序列号 */
    uint8_t uc_target_cpuid; /* 目标核cpuid */
    uint8_t uc_msg_type;     /* 消息类型 uint8_t */
    int32_t l_time_stamp;    /* 发送或接收消息的时间戳 */
} frw_ipc_log_record_stru;

typedef struct {
    OAL_VOLATILE uint32_t ul_stats_recv_lost;                 /* 接收丢包统计 */
    OAL_VOLATILE uint32_t ul_stats_send_lost;                 /* 发送丢包统计 */
    OAL_VOLATILE uint32_t ul_stats_assert;                    /* 告警统计 */
    OAL_VOLATILE uint32_t ul_stats_send;                      /* 发送次数 */
    OAL_VOLATILE uint32_t ul_stats_recv;                      /* 接受次数 */
    OAL_VOLATILE uint32_t ul_tx_index;                        /* 发送日志结构体数组下标 */
    OAL_VOLATILE uint32_t ul_rx_index;                        /* 接收日志结构体数组下标 */
    frw_ipc_log_record_stru st_tx_stats_record[MAX_LOG_RECORD]; /* 发送日志信息结构体数组 */
    frw_ipc_log_record_stru st_rx_stats_record[MAX_LOG_RECORD]; /* 接收日志信息结构体数组 */
} frw_ipc_log_stru;

/* 函数声明 */
uint32_t frw_ipc_msg_queue_init(frw_ipc_msg_queue_stru *pst_msg_queue, uint32_t ul_queue_len);
uint32_t frw_ipc_msg_queue_destroy(frw_ipc_msg_queue_stru *pst_msg_queue);
uint32_t frw_ipc_msg_queue_recv(void *p_arg);
uint32_t frw_ipc_msg_queue_send(frw_ipc_msg_queue_stru *pst_ipc_tx_msg_queue,
                                frw_ipc_msg_mem_stru *pst_msg_input, uint8_t uc_flags, uint8_t uc_cpuid);
uint32_t frw_ipc_msg_queue_register_callback(frw_ipc_msg_callback_stru *p_ipc_msg_handler);
uint32_t frw_ipc_log_exit(frw_ipc_log_stru *pst_log);
uint32_t frw_ipc_log_init(frw_ipc_log_stru *pst_log);
uint32_t frw_ipc_log_recv_alarm(frw_ipc_log_stru *pst_log, uint32_t ul_lost);
uint32_t frw_ipc_log_send_alarm(frw_ipc_log_stru *pst_log);
uint32_t frw_ipc_log_send(frw_ipc_log_stru *pst_log, uint16_t us_seq_num, uint8_t uc_target_cpuid, uint8_t uc_msg_type);
uint32_t frw_ipc_log_recv(frw_ipc_log_stru *pst_log, uint16_t us_seq_num, uint8_t uc_target_cpuid, uint8_t uc_msg_type);
uint32_t frw_ipc_log_tx_print(frw_ipc_log_stru *pst_log);
uint32_t frw_ipc_log_rx_print(frw_ipc_log_stru *pst_log);

#endif /* end of frw_ipc_msgqueue.h */
