

#ifndef __OAL_MEM_H__
#define __OAL_MEM_H__

/* 其他头文件包含 */
#include "platform_spec.h"
#include "oal_cache.h"
#include "oal_list.h"
#include "oal_schedule.h"
#include "oal_net.h"
#include "oal_util.h"
#include "arch/oal_mem.h"

#if (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
#include <linux/kallsyms.h>
#endif
/* 宏定义 */
#define oal_mem_netbuf_alloc(_uc_out_subpool_id, _us_len, _uc_netbuf_priority) \
    oal_netbuf_alloc(_us_len, 0, WLAN_MEM_NETBUF_ALIGN)

#define oal_mem_netbuf_trace(_pst_netbuf, _uc_lock)

/* Local Mem 所有接口的 _uc_lock 参数不再生效 2016/8/18 */
/* 除netbuf内存池外的其他内存池的对外提供接口 host和device共用接口 */
#define oal_mem_alloc_m(_en_pool_id, _us_len, _uc_lock) \
    oal_mem_alloc(THIS_FILE_ID, __LINE__, _en_pool_id, _us_len, _uc_lock)
#define oal_mem_free_m(_p_data, _uc_lock) \
    oal_mem_free(THIS_FILE_ID, __LINE__, _p_data, _uc_lock)

#define oal_get_mem_flag(_p_data, _puc_state)

#define oal_mem_free_array_m(_p_data, _num) \
    oal_mem_free_array(THIS_FILE_ID, __LINE__, _p_data, _num)

#define oal_mem_incr_user_m(_pst_mem, _uc_lock) \
    oal_mem_incr_user(THIS_FILE_ID, __LINE__, _pst_mem, _uc_lock)

#define oal_mem_alloc_enhanced_m(_en_pool_id, _us_len, _uc_lock) \
    oal_mem_alloc_enhanced(THIS_FILE_ID, __LINE__, _en_pool_id, _us_len, _uc_lock)

#define oal_mem_free_enhanced_m(_pst_mem, _uc_lock) \
    oal_mem_free_enhanced(THIS_FILE_ID, __LINE__, _pst_mem, _uc_lock)

/* host输出内存信息接口 */
#define oal_mem_info_print(_pool_id)

#define oal_mem_trace_m(_p_data, _uc_lock) \
    oal_mem_trace(THIS_FILE_ID, __LINE__, _p_data, _uc_lock)

#define oal_mem_trace_enhanced_m(_pst_mem, _uc_lock) \
    oal_mem_trace_enhanced(THIS_FILE_ID, __LINE__, _pst_mem, _uc_lock)

/* 对于enhanced类型的申请接口与释放接口，每一个内存块都包含一个4字节的头部， */
/* 用来指向内存块管理结构体oal_mem_struc，整个内存块的结构如下所示。           */
/*                                                                           */
/* +-------------------+------------------------------------------+---------+ */
/* | oal_mem_stru addr |                    payload               | dog tag | */
/* +-------------------+------------------------------------------+---------+ */
/* |      4/8 byte       |                                          | 4 byte  | */
/* +-------------------+------------------------------------------+---------+ */
#define OAL_DOG_TAG 0x5b3a293e /* 狗牌，用于检测内存越界 */

#if defined(CONFIG_ARM64) || defined(_PRE_WINDOWS_SUPPORT)
#define OAL_DOG_TAG_SIZE 8 /* 狗牌大小(8字节) */
#else
#define OAL_DOG_TAG_SIZE 4 /* 狗牌大小(4字节) */
#endif
#define OAL_NETBUF_MAINTAINS_SIZE 24 /* alloc_file_id(4B) + alloc_line_num(4B) + alloc_timestamp(4B) + */

#define OAL_NETBUF_DEFAULT_HEADROOM 256 /* 申请netbuf时候默认的headroom */
#define OAL_MAX_TX_DSCR_CNT_IN_LIST 100
#define OAL_TX_DSCR_ITEM_NUM        200
#define OAL_TX_DSCR_RCD_TAIL_CNT    20

#if (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
#define OAL_MEM_SPRINT_SYMBOL_SIZE KSYM_SYMBOL_LEN
#else
#define OAL_MEM_SPRINT_SYMBOL_SIZE 1
#endif
/* 枚举定义 */
/*
 * 枚举名  : oal_mem_pool_id_enum_uint8
 * 枚举说明: HOST侧内存池ID
 */
typedef enum {
    OAL_MEM_POOL_ID_EVENT = 0,       /* 事件内存池 */
    OAL_MEM_POOL_ID_SHARED_DATA_PKT, /* 共享数据帧内存池 */
    OAL_MEM_POOL_ID_SHARED_MGMT_PKT, /* 共享管理帧内存池 */
    OAL_MEM_POOL_ID_LOCAL,           /* 本地变量内存池  */
    OAL_MEM_POOL_ID_SHARED_DSCR,     /* 共享描述符内存池 */
    OAL_MEM_POOL_ID_TX_RING,         /* Tx TID Ring内存池 */

    OAL_MEM_POOL_ID_SDT_NETBUF, /* SDT netbuf内存池 */

    OAL_MEM_POOL_ID_BUTT
} oal_mem_pool_id_enum;
typedef uint8_t oal_mem_pool_id_enum_uint8;
/*
 * 枚举名  : oal_netbuf_priority_enum_uint8
 * 枚举说明: netbuf优先级
 */
typedef enum {
    OAL_NETBUF_PRIORITY_LOW = 0,   // 最低优先级,不能跨池申请
    OAL_NETBUF_PRIORITY_MID = 1,   // 中间优先级，可以跨池申请，但不能申请大包的最后N片
    OAL_NETBUF_PRIORITY_HIGH = 2,  // 最高优先级，可以跨池申请，且可以申请大包的最后N片

    OAL_NETBUF_PRIORITY_BUTT
} oal_netbuf_priority_enum;
typedef uint8_t oal_netbuf_priority_enum_uint8;

/*
 * 枚举名  : oal_netbuf_id_enum_uint8
 * 枚举说明: 对外提供netbuf内存池ID
 */
typedef enum {
    OAL_NORMAL_NETBUF = 0, /* 包括短帧、长帧内存池 */
    OAL_MGMT_NETBUF = 1,   /* 管理帧内存池 */
    OAL_OAM_NETBUF = 2,    /* SDT内存池 */

    OAL_NETBUF_POOL_ID_BUTT
} oal_netbuf_id_enum;
typedef uint8_t oal_netbuf_id_enum_uint8;

#define OAL_MEM_POOL_ID_HI_LOCAL      OAL_MEM_POOL_ID_LOCAL

/* STRUCT定义 */
typedef struct {
    uint32_t (*p_mempool_info_func)(uint8_t en_pool_id,
                                      uint16_t us_mem_total_cnt,
                                      uint16_t us_mem_used_cnt,
                                      uint8_t uc_subpool_id,
                                      uint16_t us_total_cnt,
                                      uint16_t us_free_cnt);
    uint32_t (*p_memblock_info_func)(uint8_t *puc_origin_data,
                                       uint8_t uc_user_cnt,
                                       oal_mem_pool_id_enum_uint8 en_pool_id,
                                       uint8_t uc_subpool_id,
                                       uint16_t us_len,
                                       uint32_t alloc_file_id,
                                       uint32_t alloc_line_num);
} oal_mempool_info_to_sdt_stru;

/*
 * 结构名  : oal_mem_netbuf_info_stru
 * 结构说明: netbuf内存块维测结构体
 */
typedef struct {
    uint32_t dog_tag;          /* 狗牌，用于内存写越界检查 */
    uint32_t alloc_file_id;    /* 申请netbuf内存时的文件ID */
    uint32_t alloc_line_num;   /* 申请netbuf内存时的行号 */
    uint32_t alloc_time_stamp; /* 申请netbuf内存时的时间戳 */
    uint32_t trace_file_id;    /* netbuf内存在关键路径上的文件ID */
    uint32_t trace_line_num;   /* netbuf内存在关键路径上的行号 */
    uint32_t trace_time_stamp; /* netbuf内存在关键路径上的时间戳 */
} oal_mem_netbuf_info_stru;

/*
 * 结构名  : oal_mem_stru
 * 结构说明: 内存块结构体
 */
struct oal_mem_stru_tag {
    uint8_t *puc_data;        /* 存放数据的指针 */
    uint8_t *puc_origin_data; /* 记录数据的原始指针 */

    uint16_t us_len;                              /* 内存块的长度 */
    uint8_t uc_user_cnt : 4;                      /* 申请本内存块的用户计数 */
    oal_mem_state_enum_uint8 en_mem_state_flag : 4; /* 内存块状态 */
    oal_mem_pool_id_enum_uint8 en_pool_id : 4;      /* 本内存属于哪一个内存池 */
    uint8_t uc_subpool_id : 4;                    /* 本内存是属于哪一级子内存池 */

    uintptr_t return_addr; /* alloc内存的函数地址 新增 */

/* 内存调试信息 */
    uint32_t alloc_core_id[WLAN_MEM_MAX_USERS_NUM];    /* 申请内存时的内核ID */
    uint32_t alloc_file_id[WLAN_MEM_MAX_USERS_NUM];    /* 申请内存时的文件ID */
    uint32_t alloc_line_num[WLAN_MEM_MAX_USERS_NUM];   /* 申请内存时的行号 */
    uint32_t alloc_time_stamp[WLAN_MEM_MAX_USERS_NUM]; /* 申请内存时的时间戳 */
    uint32_t trace_file_id;                            /* 内存在关键路径上的文件ID */
    uint32_t trace_line_num;                           /* 内存在关键路径上的行号 */
    uint32_t trace_time_stamp;                         /* 内存在关键路径上的时间戳 */
} OAL_CACHELINE_ALIGNED;
typedef struct oal_mem_stru_tag oal_mem_stru;

/*
 * 结构名  : oal_mem_subpool_stru
 * 结构说明: 子内存池结构体
 */
typedef struct {
    oal_spin_lock_stru st_spinlock;
    uint16_t us_len;      /* 本子内存池的内存块长度 */
    uint16_t us_free_cnt; /* 本子内存池可用内存块数 */

    /* 记录oal_mem_stru可用内存索引表的栈顶元素，其内容为oal_mem_stru指针 */
    void **ppst_free_stack;

    uint16_t us_total_cnt; /* 本子内存池内存块总数 */
    uint8_t auc_resv[2];
} oal_mem_subpool_stru;

/*
 * 结构名  : oal_mem_pool_stru
 * 结构说明: 内存池结构体
 */
typedef struct {
    uint16_t us_max_byte_len; /* 本内存池可分配内存块最大长度 */
    uint8_t uc_subpool_cnt;   /* 本内存池一共有多少子内存池 */
    uint8_t uc_resv;
    /* 子内存池索引表数组 */
    oal_mem_subpool_stru ast_subpool_table[WLAN_MEM_MAX_SUBPOOL_NUM];

    uint16_t us_mem_used_cnt;  /* 本内存池已用内存块 */
    uint16_t us_mem_total_cnt; /* 本内存池一共有多少内存块 */
    oal_mem_stru *pst_mem_start_addr;
} oal_mem_pool_stru;

/*
 * 结构名  : oal_mem_subpool_cfg_stru
 * 结构说明: 子内存池配置结构体
 */
typedef struct {
    uint16_t us_size; /* 本子内存池内存块大小 */
    uint16_t us_cnt;  /* 本子内存池内存块个数 */
} oal_mem_subpool_cfg_stru;

/*
 * 结构名  : oal_mem_pool_cfg_stru
 * 结构说明: 内存池配置结构体
 */
typedef struct {
    oal_mem_pool_id_enum_uint8 en_pool_id; /* 内存池ID */
    uint8_t uc_subpool_cnt;              /* 本内存池子内存池个数 */

    uint8_t auc_resv[2]; /* 保留2个字节 */

    /* 指向每一子内存池具体配置信息 */
    oal_mem_subpool_cfg_stru *pst_subpool_cfg_info;
} oal_mem_pool_cfg_stru;

/* 函数声明 */
extern void *oal_mem_alloc(uint32_t file_id,
                           uint32_t line_num,
                           oal_mem_pool_id_enum_uint8 en_pool_id,
                           uint16_t us_len,
                           uint8_t uc_lock);
extern uint32_t oal_mem_free(uint32_t file_id, uint32_t line_num, void *pst_mem, uint8_t uc_lock);

extern uint32_t oal_mem_free_array(uint32_t file_id,
                                   uint32_t line_num,
                                   void *p_data,
                                   uint8_t uc_mem_num);

extern oal_mem_stru *oal_mem_alloc_enhanced(uint32_t file_id,
                                            uint32_t line_num,
                                            oal_mem_pool_id_enum_uint8 en_pool_id,
                                            uint16_t us_len,
                                            uint8_t uc_lock);
extern uint32_t oal_mem_free_enhanced(uint32_t file_id, uint32_t line_num,
                                      oal_mem_stru *p_data, uint8_t uc_lock);
extern uint32_t oal_mem_init_pool(void);
extern void oal_mem_exit(void);
extern uint32_t oal_mem_incr_user(uint32_t file_id,
                                  uint32_t line_num,
                                  oal_mem_stru *pst_mem,
                                  uint8_t uc_lock);
extern void oal_mem_info(oal_mem_pool_id_enum_uint8 en_pool_id);
extern void oal_mem_leak(oal_mem_pool_id_enum_uint8 en_pool_id);

extern void oal_mem_start_stat(void);
extern uint32_t oal_mem_end_stat(void);

extern uint32_t oal_mem_trace_enhanced(uint32_t file_id,
                                       uint32_t line_num,
                                       oal_mem_stru *pst_mem,
                                       uint8_t uc_lock);
extern uint32_t oal_mem_trace(uint32_t file_id,
                              uint32_t line_num,
                              const void *p_data,
                              uint8_t uc_lock);

/* device 侧裸系统和 host侧的函数输出 */
extern oal_mem_pool_stru *oal_mem_get_pool(oal_mem_pool_id_enum_uint8 en_pool_id);

extern oal_mem_pool_cfg_stru *oal_mem_get_pool_cfg_table(oal_mem_pool_id_enum_uint8 en_pool_id);
extern oal_netbuf_stru *oal_mem_sdt_netbuf_alloc(uint16_t us_len, uint8_t uc_lock);
extern uint32_t oal_mem_sdt_netbuf_free(oal_netbuf_stru *pst_netbuf, uint8_t uc_lock);
extern oal_netbuf_stru *oal_netbuf_duplicate(oal_netbuf_stru *pst_src_netbuf,
                                             uint8_t uc_out_subpool_id,
                                             uint32_t add_head_room,
                                             uint32_t add_tail_room);

extern void oal_mem_print_normal_pool_info(oal_mem_pool_id_enum_uint8 en_pool_id);
extern void oal_mem_print_pool_info(void);
extern void oal_mem_print_funcname(uintptr_t func_addr);

#define oal_netbuf_duplicate_normal(pst_src_netbuf, add_head_room, add_tail_room) \
    oal_netbuf_duplicate(pst_src_netbuf, OAL_NORMAL_NETBUF, add_head_room, add_tail_room)

#endif /* end of oal_mm.h */

