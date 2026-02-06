

#ifndef __FRW_EXT_IF_H__
#define __FRW_EXT_IF_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_EXT_IF_H

#define frw_timer_restart_timer_m(_pst_timeout, _timeout, _en_is_periodic) \
    frw_timer_restart_timer(_pst_timeout, _timeout, _en_is_periodic)
#define frw_timer_stop_timer_m(_pst_timeout) \
    frw_timer_stop_timer(_pst_timeout)
#define frw_timer_create_timer_m(_pst_timeout, _p_timeout_func, _timeout, \
                               _p_timeout_arg, _en_is_periodic, _en_module_id, _core_id) \
    frw_timer_create_timer(THIS_FILE_ID, __LINE__, _pst_timeout, _p_timeout_func, _timeout, \
                               _p_timeout_arg, _en_is_periodic, _en_module_id, _core_id)
#define frw_timer_destroy_timer(_pst_timeout) \
    frw_timer_immediate_destroy_timer(THIS_FILE_ID, __LINE__, _pst_timeout)
#define frw_timer_immediate_destroy_timer_m(_pst_timeout) \
    frw_timer_immediate_destroy_timer(THIS_FILE_ID, __LINE__, _pst_timeout)

#define FRW_TIMER_TRACK_NUM   256
#define FRW_TIMEOUT_TRACK_NUM 256

#define FRW_PROCESS_MAX_EVENT 100

/* 枚举定义 */
/* 此枚举用于定义从FRW模块以上各模块的初始化状态 */
typedef enum {
    FRW_INIT_STATE_START,                /* 表示初始化刚启动，即FRW初始化开始 */
    FRW_INIT_STATE_FRW_SUCC,             /* 表示FRW模块初始化成功 */
    FRW_INIT_STATE_HAL_SUCC,             /* 表示HAL模块初始化成功 */
    FRW_INIT_STATE_DMAC_CONFIG_VAP_SUCC, /* 表示DMAC模块启动初始化成功，配置VAP已正常；如果在此状态之后再初始化，即为业务VAP的初始化 */
    FRW_INIT_STATE_HMAC_CONFIG_VAP_SUCC, /* 表示HMAC模块启动初始化成功，配置VAP已正常；如果在此状态之后再初始化，即为业务VAP的初始化 */
    FRW_INIT_STATE_ALL_SUCC,             /* 此状态表示HMAC以上模块均已初始化成功 */

    FRW_INIT_STATE_BUTT
} frw_init_enum;

/*
 * 枚举名  : uint8_t
 * 枚举说明: 事件类型
 */
typedef enum {
    FRW_EVENT_TYPE_HIGH_PRIO = 0,    /* 高优先级事件队列 */
    FRW_EVENT_TYPE_HOST_CRX,         /* 接收Host侧发来的配置事件 */
    FRW_EVENT_TYPE_HOST_DRX,         /* 接收Host侧发来的数据事件 */
    FRW_EVENT_TYPE_HOST_CTX,         /* 发向HOST侧的配置事件 */
    FRW_EVENT_TYPE_HOST_SDT_REG = 4, /* SDT读取寄存器，wifi驱动侧上报SDT */
    FRW_EVENT_TYPE_WLAN_CRX,         /* 接收Wlan侧发来的管理/控制帧事件 */
    FRW_EVENT_TYPE_WLAN_DRX,         /* 接收Wlan侧发来的数据帧事件 */
    FRW_EVENT_TYPE_WLAN_CTX,         /* 管理/控制帧发送至Wlan侧事件 */
    FRW_EVENT_TYPE_WLAN_DTX,         /* 数据帧发送至Wlan侧事件 */
    FRW_EVENT_TYPE_WLAN_TX_COMP = 9, /* 发送完成事件 */
    FRW_EVENT_TYPE_TBTT,             /* TBTT中断事件 */
    FRW_EVENT_TYPE_TIMEOUT,          /* FRW普通超时事件 */
    FRW_EVENT_TYPE_HMAC_MISC,        /* HMAC杂散事件，例如雷达检测事件 */
    FRW_EVENT_TYPE_DMAC_MISC = 13,   /* DMAC杂散事件 */
    FRW_EVENT_TYPE_HOST_DDR_DRX = 14, /* HOST DDR事件 */

    FRW_EVENT_TYPE_BUTT
} frw_event_type_enum;

/* HCC事件子类型定义 */
typedef enum {
    DMAC_HCC_TX_EVENT_SUB_TYPE,

    DMAC_HCC_TX_EVENT_SUB_TYPE_BUTT
} dmac_hcc_tx_event_sub_type_enum;

typedef enum {
    DMAC_HCC_RX_EVENT_SUB_TYPE,

    DMAC_HCC_RX_EVENT_SUB_TYPE_BUTT
} dmac_hcc_rx_event_sub_type_enum;

typedef struct {
    uint8_t en_nest_type; /* 嵌套的业务事件的主类型 */
    uint8_t uc_nest_sub_type;             /* 嵌套的业务事件的子类型 */
    void *pst_netbuf;                       /* 如果是数据事件，则对应于链表头netbuf的地址;其他事件，则对应于buff首地址 */
    uint32_t buf_len;                  /* 如果是数据事件，则对应于netbuf个数;其他事件，则对应于buff len */
} hcc_event_stru;

/* record the data type by the hcc */
#pragma pack(push, 1)
/* 4B */
struct frw_hcc_extend_hdr {
    uint8_t en_nest_type;
    uint8_t uc_nest_sub_type;
    uint8_t chip_id : 2; /* this is not good */
    uint8_t device_id : 2;
    uint8_t vap_id : 4;
} __OAL_DECLARE_PACKED;
#pragma pack(pop)

typedef uint32_t (*frw_timeout_func)(void *);

typedef struct {
    void *p_timeout_arg;             /* 超时处理函数入参 */
    frw_timeout_func p_func;             /* 超时处理函数 */
    uint32_t time_stamp;            /* 定时器启动时间 */
    uint32_t curr_time_stamp;       /* 定时器进入当前时间 */
    uint32_t timeout;               /* 过多长时间定时器超时 */
    oal_bool_enum_uint8 en_is_registerd; /* 定时器是否已经注册 */
    oal_bool_enum_uint8 en_is_periodic;  /* 定时器是否为周期的 */
    oal_bool_enum_uint8 en_is_enabled;   /* 定时器是否使能 */
    uint8_t uc_pad;
    oam_module_id_enum_uint16 en_module_id; /* 维测用模块id */
    uint32_t core_id;                  /* 绑定的核id */
    uint32_t file_id;
    uint32_t line_num;
    oal_dlist_head_stru st_entry; /* 定期器链表索引 */
} frw_timeout_stru;
/*
 * 枚举名  : uint8_t
 * 枚举说明: 事件分段号，取值[0, 1]
 */
typedef enum {
    FRW_EVENT_PIPELINE_STAGE_0 = 0,  // 抛事件
    FRW_EVENT_PIPELINE_STAGE_1,      // 直接调用
    FRW_EVENT_PIPELINE_STAGE_BUTT
} frw_event_pipeline_enum;

#define FRW_RX_EVENT_TRACK_NUM 256
#define FRW_EVENT_TRACK_NUM    128

typedef oal_mem_stru frw_event_mem_stru; /* 事件结构体内存块转定义 */

/*
 * 结构名  : frw_event_hdr_stru
 * 结构说明: 事件头结构体,
 * 备注    : uc_length的值为(payload长度 + 事件头长度 - 2)
 */
typedef struct {
    uint8_t en_type;         /* 事件类型 */
    uint8_t uc_sub_type;                     /* 事件子类型 */
    uint16_t us_length;                      /* 事件总体长度 */
    uint8_t en_pipeline; /* 事件分段号 */
    uint8_t uc_chip_id;                      /* 芯片ID */
    uint8_t uc_device_id;                    /* 设备ID */
    uint8_t uc_vap_id;                       /* VAP ID */
} frw_event_hdr_stru;

/*
 * 结构名  : frw_event_stru
 * 结构说明: 事件结构体
 */
typedef struct {
    frw_event_hdr_stru st_event_hdr; /* 事件头 */
    uint8_t auc_event_data[4];     /* payload 4data */
} frw_event_stru;

/*
 * 结构名  : frw_event_sub_table_item_stru
 * 结构说明: 事件子表结构体
 */
typedef struct {
    uint32_t (*p_func)(frw_event_mem_stru *); /* (type, subtype, pipeline)类型的事件对应的处理函数 */
    uint32_t (*p_tx_adapt_func)(frw_event_mem_stru *);
    frw_event_mem_stru *(*p_rx_adapt_func)(frw_event_mem_stru *);
} frw_event_sub_table_item_stru;

/*
 * 结构名  : frw_event_table_item_stru
 * 结构说明: 事件表结构体
 */
typedef struct {
    frw_event_sub_table_item_stru *pst_sub_table; /* 指向子表的指针 */
} frw_event_table_item_stru;

/*
 * 结构名  : frw_ipc_msg_header_stru
 * 结构说明: IPC(核间通信)头结构体
 */
typedef struct {
    uint16_t us_seq_number;  /* 核间消息序号 */
    uint8_t uc_target_cpuid; /* 目标核cpuid frw_ipc_cpu_id_enum_uint8 */
    uint8_t uc_msg_type;     /* 消息类型 uint8_t */
} frw_ipc_msg_header_stru;

typedef struct {
    uint32_t event_cnt;
    uint32_t event_time[FRW_EVENT_TRACK_NUM];
    uint16_t us_event_type[FRW_EVENT_TRACK_NUM];
    uint16_t us_event_sub_type[FRW_EVENT_TRACK_NUM];
} frw_event_track_time_stru;

#define FRW_IPC_MSG_HEADER_LENGTH (sizeof(frw_ipc_msg_header_stru))

/* 事件头长度 */
#define FRW_EVENT_HDR_LEN sizeof(frw_event_hdr_stru)

/* 事件队列最大个数 */
#define FRW_EVENT_MAX_NUM_QUEUES (FRW_EVENT_TYPE_BUTT * WLAN_VAP_SUPPORT_MAX_NUM_LIMIT)

#define frw_field_setup(_p, _m, _v) ((_p)->_m = (_v))

/* 事件头修改宏(修改事件头中的pipeline和subtype) */
#define frw_event_hdr_modify_pipeline_and_subtype(_pst_event_hdr, _uc_sub_type) \
    do {                                                                        \
        frw_field_setup((_pst_event_hdr), en_pipeline, 1);                      \
        frw_field_setup((_pst_event_hdr), uc_sub_type, (_uc_sub_type));         \
    } while (0)

#define frw_event_hdr_modify_pipeline_and_type_and_subtype(_pst_event_hdr, _en_type, _uc_sub_type) \
    do {                                                                                           \
        frw_field_setup((_pst_event_hdr), en_pipeline, 1);                                         \
        frw_field_setup((_pst_event_hdr), uc_sub_type, (_uc_sub_type));                            \
        frw_field_setup((_pst_event_hdr), en_type, (_en_type));                                    \
    } while (0)

#define frw_event_alloc_m(_us_len) \
    frw_event_alloc(THIS_FILE_ID, __LINE__, _us_len);

#define frw_event_free_m(_pst_event_mem) \
    frw_event_free(THIS_FILE_ID, __LINE__, _pst_event_mem)

/* Hi10X共仓代码适配51，Hi10X为了编译时进行检查，后续可能会有调整 */
#define frw_event_alloc_big(_us_len) \
    frw_event_alloc(THIS_FILE_ID, __LINE__, _us_len);

#define frw_event_alloc_large(_us_len) \
    frw_event_alloc(THIS_FILE_ID, __LINE__, _us_len);

/* 事件头初始化宏 */
#define frw_event_hdr_init(_pst_event_hdr, _en_type, _uc_sub_type, _us_length,              \
                           _en_pipeline, _uc_chip_id, _uc_device_id, _uc_vap_id)            \
    do {                                                                                    \
        frw_field_setup((_pst_event_hdr), us_length, ((_us_length) + FRW_EVENT_HDR_LEN));     \
        frw_field_setup((_pst_event_hdr), en_type, (_en_type));                             \
        frw_field_setup((_pst_event_hdr), uc_sub_type, (_uc_sub_type));                     \
        frw_field_setup((_pst_event_hdr), en_pipeline, (_en_pipeline));                     \
        frw_field_setup((_pst_event_hdr), uc_chip_id, (_uc_chip_id));                       \
        frw_field_setup((_pst_event_hdr), uc_device_id, (_uc_device_id));                   \
        frw_field_setup((_pst_event_hdr), uc_vap_id, (_uc_vap_id));                         \
    } while (0)

/* 为了hi110x和51共仓代码一致，这里保留该宏定义，使用时注意和frw_get_event_stru进行区分 */
#define frw_get_event_data(pst_event_mem) ((pst_event_mem)->puc_data)

#define frw_get_event_stru(pst_event_mem)    ((frw_event_stru *)(pst_event_mem)->puc_data)
#define frw_get_event_hdr(pst_event_mem)     ((frw_event_hdr_stru *)(&((frw_event_stru *) \
                                                                    (pst_event_mem)->puc_data)->st_event_hdr))
#define frw_get_event_payload(pst_event_mem) ((uint8_t *)((frw_event_stru *) \
                                                            (pst_event_mem)->puc_data)->auc_event_data)

/*
 * 函 数 名  : frw_event_alloc
 * 功能描述  : 申请事件内存
 * 输入参数  : us_length: payload长度 + 事件头长度
 * 返 回 值  : 成功: 指向frw_event_mem_stru的指针
 *             失败:NULLL
 */
OAL_STATIC OAL_INLINE frw_event_mem_stru *frw_event_alloc(uint32_t file_id, uint32_t line_num, uint16_t payload_length)
{
    frw_event_mem_stru *pst_event_mem;

    payload_length += OAL_MEM_INFO_SIZE;

    pst_event_mem = oal_mem_alloc_enhanced(file_id, line_num, OAL_MEM_POOL_ID_EVENT,
                                           (payload_length + FRW_EVENT_HDR_LEN), OAL_TRUE);
    if (oal_unlikely(pst_event_mem == NULL)) {
        oal_mem_print_normal_pool_info(OAL_MEM_POOL_ID_EVENT);

        return NULL;
    }

    pst_event_mem->return_addr = OAL_RET_ADDR;

#if (_PRE_MULTI_CORE_MODE_SMP == _PRE_MULTI_CORE_MODE)
    pst_event_mem->puc_data += FRW_IPC_MSG_HEADER_LENGTH;
#endif

    return pst_event_mem;
}

/*
 * 函 数 名  : frw_event_free
 * 功能描述  : 释放事件所占用的内存
 */
OAL_STATIC OAL_INLINE uint32_t frw_event_free(uint32_t file_id,
                                              uint32_t line_num,
                                              frw_event_mem_stru *pst_event_mem)
{
    uint32_t ret;
    frw_event_stru *pst_frw_event = NULL;

    ret = oal_mem_free_enhanced(file_id, line_num, pst_event_mem, OAL_TRUE);
    if (oal_warn_on(ret != OAL_SUCC)) {
        pst_frw_event = frw_get_event_stru(pst_event_mem);
        oal_io_print("[E]frw event free failed!, ret:%d, type:%d, subtype:%d\r\n",
                     ret, pst_frw_event->st_event_hdr.en_type,
                     pst_frw_event->st_event_hdr.uc_sub_type);
        oal_dump_stack();
    }

    return ret;
}

/*
  10 函数声明
 */
extern int32_t frw_main_init(void);
extern void frw_main_exit(void);
extern void frw_set_init_state(uint16_t en_init_state);
extern uint16_t frw_get_init_state(void);
extern uint32_t frw_event_exit(void);
extern uint32_t frw_event_init(void);
extern uint32_t frw_event_dispatch_event(frw_event_mem_stru *pst_mem);

extern void frw_event_table_register(uint8_t en_event_type,
                                     frw_event_pipeline_enum en_pipeline,
                                     frw_event_sub_table_item_stru *pst_sub_table);
void frw_event_sub_rx_adapt_table_init(frw_event_sub_table_item_stru *pst_sub_table, uint32_t table_nums,
                                       frw_event_mem_stru *(*p_rx_adapt_func)(frw_event_mem_stru *));
extern void frw_event_dump_event(uint8_t *puc_event);
extern uint32_t frw_event_flush_event_queue(uint8_t uc_event_type);
extern void frw_event_vap_pause_event(uint8_t uc_vap_id);
extern void frw_event_vap_resume_event(uint8_t uc_vap_id);
extern uint32_t frw_event_vap_flush_event(uint8_t uc_vap_id,
                                          uint8_t en_event_type,
                                          oal_bool_enum_uint8 en_drop);
extern void frw_timer_create_timer(uint32_t file_id,
                                   uint32_t line_num,
                                   frw_timeout_stru *pst_timeout,
                                   frw_timeout_func p_timeout_func,
                                   uint32_t timeout,
                                   void *p_timeout_arg,
                                   oal_bool_enum_uint8 en_is_periodic,
                                   oam_module_id_enum_uint16 en_module_id,
                                   uint32_t core_id);
extern void frw_timer_immediate_destroy_timer(uint32_t file_id,
                                              uint32_t line_num,
                                              frw_timeout_stru *pst_timeout);
extern void frw_timer_restart_timer(frw_timeout_stru *pst_timeout, uint32_t timeout,
                                    oal_bool_enum_uint8 en_is_periodic);
extern void frw_timer_add_timer(frw_timeout_stru *pst_timeout);
extern void frw_timer_stop_timer(frw_timeout_stru *pst_timeout);
extern void frw_timer_delete_all_timer(void);
extern uint32_t frw_event_queue_info(void);
extern void frw_event_process_all_event(oal_uint ui_data);
extern oal_bool_enum_uint8 frw_is_event_queue_empty(uint8_t uc_event_type);
extern oal_bool_enum_uint8 frw_is_vap_event_queue_empty(uint32_t core_id, uint8_t uc_vap_id,
                                                        uint8_t event_type);
extern uint8_t frw_task_thread_condition_check(uint32_t core_id);

extern void hcc_host_update_vi_flowctl_param(uint32_t be_cwmin, uint32_t vi_cwmin);
extern void frw_timer_clean_timer(oam_module_id_enum_uint16 en_module_id);

#endif /* end of frw_ext_if.h */
