

/* 头文件包含 */
#include "oal_mem.h"
#include "oam_ext_if.h"
#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_MEM_C

/*
 * 结构名  : oal_mem_subpool_stat
 * 结构说明: 子内存池统计结构体，维测使用
 */
typedef struct {
    uint16_t us_free_cnt;  /* 本子内存池可用内存块数 */
    uint16_t us_total_cnt; /* 本子内存池内存块总数 */
} oal_mem_subpool_stat;

/*
 * 结构名  : oal_mem_pool_stat
 * 结构说明: 单个内存池统计结构体，维测使用
 */
typedef struct {
    uint16_t us_mem_used_cnt;  /* 本内存池已用内存块 */
    uint16_t us_mem_total_cnt; /* 本内存池一共有多少内存块 */
    oal_mem_subpool_stat ast_subpool_stat[WLAN_MEM_MAX_SUBPOOL_NUM];
} oal_mem_pool_stat;

/*
 * 结构名  : oal_mem_stat
 * 结构说明: 内存池统计结构体，维测使用
 */
typedef struct {
    oal_mem_pool_stat ast_mem_start_stat[OAL_MEM_POOL_ID_BUTT]; /* 起始统计信息 */
    oal_mem_pool_stat ast_mem_end_stat[OAL_MEM_POOL_ID_BUTT];   /* 终止统计信息 */
} oal_mem_stat;

/* 申请第一个描述符虚拟地址和物理地址保存 */
#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
uint32_t g_dscr_fstvirt_addr = 0;
uint32_t g_dscr_fstphy_addr = 0;
#endif

/* 内存池统计信息全局变量，维测使用 */
OAL_STATIC oal_mem_stat g_mem_stat;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_mempool_info_to_sdt_stru g_mempool_info = {
    .p_mempool_info_func = NULL,
    .p_memblock_info_func = NULL
};
#else
oal_mempool_info_to_sdt_stru g_mempool_info = { NULL, NULL };
#endif
/* 共享描述符内存池配置信息全局变量 */
OAL_STATIC oal_mem_subpool_cfg_stru g_shared_dscr_cfg_tab[] = {
    { WLAN_MEM_SHARED_RX_DSCR_SIZE + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE,  WLAN_MEM_SHARED_RX_DSCR_CNT },
    { WLAN_MEM_SHARED_TX_DSCR_SIZE1 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE, WLAN_MEM_SHARED_TX_DSCR_CNT1 },
    { WLAN_MEM_SHARED_TX_DSCR_SIZE2 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE, WLAN_MEM_SHARED_TX_DSCR_CNT2 },
};

/* 共享管理帧内存池配置信息全局变量 */
OAL_STATIC oal_mem_subpool_cfg_stru g_shared_mgmt_cfg_tab[] = {
    /* beacon、Probe Response、Auth Seq3帧大小，块数 */
    { WLAN_MEM_SHARED_MGMT_PKT_SIZE1 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE, WLAN_MEM_SHARED_MGMT_PKT_CNT1 },
};

/* 共享数据帧内存池配置信息全局变量 */
OAL_STATIC oal_mem_subpool_cfg_stru g_shared_data_cfg_tab[] = {
    /* 802.11MAC帧头+SNAP头+Ethernet头，块数 */
    { WLAN_MEM_SHARED_DATA_PKT_SIZE + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE, WLAN_MEM_SHARED_DATA_PKT_CNT }
};

/* TX RING内存池配置信息全局变量 */
OAL_STATIC oal_mem_subpool_cfg_stru g_tx_ring_cfg_table[] = {
    { WLAN_MEM_TX_RING_SIZE1 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE, WLAN_MEM_TX_RING_CNT1},
};

/* 本地数据内存池配置信息全局变量 */
#define TOTAL_WLAN_MEM_LOCAL_SIZE1 (WLAN_MEM_LOCAL_SIZE1 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_LOCAL_SIZE2 (WLAN_MEM_LOCAL_SIZE2 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_LOCAL_SIZE3 (WLAN_MEM_LOCAL_SIZE3 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_LOCAL_SIZE4 (WLAN_MEM_LOCAL_SIZE4 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_LOCAL_SIZE5 (WLAN_MEM_LOCAL_SIZE5 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_LOCAL_SIZE6 (WLAN_MEM_LOCAL_SIZE6 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_LOCAL_SIZE  (TOTAL_WLAN_MEM_LOCAL_SIZE1 * WLAN_MEM_LOCAL_CNT1 + \
                                    TOTAL_WLAN_MEM_LOCAL_SIZE2 * WLAN_MEM_LOCAL_CNT2 + \
                                    TOTAL_WLAN_MEM_LOCAL_SIZE3 * WLAN_MEM_LOCAL_CNT3 + \
                                    TOTAL_WLAN_MEM_LOCAL_SIZE4 * WLAN_MEM_LOCAL_CNT4 + \
                                    TOTAL_WLAN_MEM_LOCAL_SIZE5 * WLAN_MEM_LOCAL_CNT5 + \
                                    TOTAL_WLAN_MEM_LOCAL_SIZE6 * WLAN_MEM_LOCAL_CNT6)

OAL_STATIC oal_mem_subpool_cfg_stru g_local_cfg_tab[] = {
    { TOTAL_WLAN_MEM_LOCAL_SIZE1, WLAN_MEM_LOCAL_CNT1 }, /* 第一级大小，块数 */
    { TOTAL_WLAN_MEM_LOCAL_SIZE2, WLAN_MEM_LOCAL_CNT2 }, /* 第二级大小，块数 */
    { TOTAL_WLAN_MEM_LOCAL_SIZE3, WLAN_MEM_LOCAL_CNT3 },
    { TOTAL_WLAN_MEM_LOCAL_SIZE4, WLAN_MEM_LOCAL_CNT4 },
    { TOTAL_WLAN_MEM_LOCAL_SIZE5, WLAN_MEM_LOCAL_CNT5 },
    { TOTAL_WLAN_MEM_LOCAL_SIZE6, WLAN_MEM_LOCAL_CNT6 },
};
/* 检查内存子池是否对齐 */
// cppcheck-suppress * // 屏蔽cppcheck告警
#if (TOTAL_WLAN_MEM_LOCAL_SIZE1 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif
#if (TOTAL_WLAN_MEM_LOCAL_SIZE2 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif
#if (TOTAL_WLAN_MEM_LOCAL_SIZE3 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif
#if (TOTAL_WLAN_MEM_LOCAL_SIZE4 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif
#if (TOTAL_WLAN_MEM_LOCAL_SIZE5 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif
#if (TOTAL_WLAN_MEM_LOCAL_SIZE6 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif

/* 事件内存池配置信息全局变量 */
#define TOTAL_WLAN_MEM_EVENT_SIZE1 (WLAN_MEM_EVENT_SIZE1 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_EVENT_SIZE2 (WLAN_MEM_EVENT_SIZE2 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_EVENT_SIZE  (TOTAL_WLAN_MEM_EVENT_SIZE1 * WLAN_MEM_EVENT_CNT1 \
                                   + TOTAL_WLAN_MEM_EVENT_SIZE2 * WLAN_MEM_EVENT_CNT2)

OAL_STATIC oal_mem_subpool_cfg_stru g_event_cfg_tab[] = {
    { TOTAL_WLAN_MEM_EVENT_SIZE1, WLAN_MEM_EVENT_CNT1 },
    { TOTAL_WLAN_MEM_EVENT_SIZE2, WLAN_MEM_EVENT_CNT2 },
};

#if (TOTAL_WLAN_MEM_EVENT_SIZE1 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif
#if (TOTAL_WLAN_MEM_EVENT_SIZE2 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif

/* MIB内存池配置信息全局变量 */
#define TOTAL_WLAN_MEM_MIB_SIZE1 (WLAN_MEM_MIB_SIZE1 + OAL_MEM_INFO_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_MIB_SIZE  (TOTAL_WLAN_MEM_MIB_SIZE1 * WLAN_MEM_MIB_CNT1)

#if (TOTAL_WLAN_MEM_MIB_SIZE1 % OAL_MEM_INFO_SIZE)
#error alignment fault error
#endif

/* netbuf内存池配置信息全局变量 */
#define TOTAL_WLAN_MEM_NETBUF_SIZE1 (WLAN_MEM_NETBUF_SIZE1 + OAL_NETBUF_MAINTAINS_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_NETBUF_SIZE2 (WLAN_MEM_NETBUF_SIZE2 + OAL_NETBUF_MAINTAINS_SIZE + OAL_DOG_TAG_SIZE)
#define TOTAL_WLAN_MEM_NETBUF_SIZE3 (WLAN_MEM_NETBUF_SIZE3 + OAL_NETBUF_MAINTAINS_SIZE + OAL_DOG_TAG_SIZE)
oal_mem_subpool_cfg_stru g_netbuf_cfg_tab[] = {
    { TOTAL_WLAN_MEM_NETBUF_SIZE1, WLAN_MEM_NETBUF_CNT1 }, /* 克隆用netbuf */
    { TOTAL_WLAN_MEM_NETBUF_SIZE2, WLAN_MEM_NETBUF_CNT2 },
    { TOTAL_WLAN_MEM_NETBUF_SIZE3, WLAN_MEM_NETBUF_CNT3 }
};

/* sdt netbuf内存池配置信息全局变量 */
#define TOTAL_WLAN_MEM_SDT_NETBUF_SIZE1 (WLAN_MEM_SDT_NETBUF_SIZE1 + OAL_MEM_INFO_SIZE)
#define TOTAL_WLAN_MEM_SDT_NETBUF_SIZE2 (WLAN_MEM_SDT_NETBUF_SIZE2 + OAL_MEM_INFO_SIZE)
#define TOTAL_WLAN_MEM_SDT_NETBUF_SIZE3 (WLAN_MEM_SDT_NETBUF_SIZE3 + OAL_MEM_INFO_SIZE)
#define TOTAL_WLAN_MEM_SDT_NETBUF_SIZE4 (WLAN_MEM_SDT_NETBUF_SIZE4 + OAL_MEM_INFO_SIZE)
OAL_STATIC oal_mem_subpool_cfg_stru g_sdt_netbuf_cfg_tab[] = {
    { TOTAL_WLAN_MEM_SDT_NETBUF_SIZE1, WLAN_MEM_SDT_NETBUF_SIZE1_CNT },
    { TOTAL_WLAN_MEM_SDT_NETBUF_SIZE2, WLAN_MEM_SDT_NETBUF_SIZE2_CNT },
    { TOTAL_WLAN_MEM_SDT_NETBUF_SIZE3, WLAN_MEM_SDT_NETBUF_SIZE3_CNT },
    { TOTAL_WLAN_MEM_SDT_NETBUF_SIZE4, WLAN_MEM_SDT_NETBUF_SIZE4_CNT },
};

/* 总的内存池配置信息全局变量 */
OAL_STATIC oal_mem_pool_cfg_stru g_mem_pool_cfg_tab[] = {
    /*       内存池ID                       内存池子内存池个数               四字节对齐  内存池配置信息 */
    { OAL_MEM_POOL_ID_EVENT,           oal_array_size(g_event_cfg_tab),       { 0, 0 }, g_event_cfg_tab },
    { OAL_MEM_POOL_ID_SHARED_DATA_PKT, oal_array_size(g_shared_data_cfg_tab), { 0, 0 }, g_shared_data_cfg_tab },
    { OAL_MEM_POOL_ID_SHARED_MGMT_PKT, oal_array_size(g_shared_mgmt_cfg_tab), { 0, 0 }, g_shared_mgmt_cfg_tab },
    { OAL_MEM_POOL_ID_LOCAL,           oal_array_size(g_local_cfg_tab),       { 0, 0 }, g_local_cfg_tab },
    { OAL_MEM_POOL_ID_SHARED_DSCR,     oal_array_size(g_shared_dscr_cfg_tab), { 0, 0 }, g_shared_dscr_cfg_tab },
    { OAL_MEM_POOL_ID_TX_RING,         oal_array_size(g_tx_ring_cfg_table),   { 0, 0 }, g_tx_ring_cfg_table },
    { OAL_MEM_POOL_ID_SDT_NETBUF,      oal_array_size(g_sdt_netbuf_cfg_tab),  { 0, 0 }, g_sdt_netbuf_cfg_tab }
};

#ifdef WIN32
OAL_STATIC uint8_t *g_mem_pool[OAL_MEM_POOL_ID_BUTT] = {0};
OAL_STATIC uint8_t g_mem_pool_event[TOTAL_WLAN_MEM_EVENT_SIZE];
OAL_STATIC uint8_t g_mem_pool_shared_data_pkt[TOTAL_WLAN_MEM_SHARED_DATA_PKT_SIZE];
OAL_STATIC uint8_t g_mem_pool_shared_mgmt_pkt[TOTAL_WLAN_MEM_SHARED_MGMT_PKT_SIZE];
OAL_STATIC uint8_t g_mem_pool_local[TOTAL_WLAN_MEM_LOCAL_SIZE];
OAL_STATIC uint8_t g_mem_pool_shared_dscr[TOTAL_WLAN_MEM_SHARED_DSCR_SIZE];
OAL_STATIC uint8_t g_mem_pool_tx_ring_dscr[TOTAL_WLAN_MEM_TX_RING_SIZE];

#else
OAL_STATIC uint8_t *g_mem_pool[OAL_MEM_POOL_ID_BUTT] = {0};
OAL_STATIC uint8_t g_mem_pool_event[TOTAL_WLAN_MEM_EVENT_SIZE] __attribute__((aligned(8)));
OAL_STATIC uint8_t g_mem_pool_shared_data_pkt[TOTAL_WLAN_MEM_SHARED_DATA_PKT_SIZE] __attribute__((aligned(8)));
OAL_STATIC uint8_t g_mem_pool_shared_mgmt_pkt[TOTAL_WLAN_MEM_SHARED_MGMT_PKT_SIZE] __attribute__((aligned(8)));
OAL_STATIC uint8_t g_mem_pool_local[TOTAL_WLAN_MEM_LOCAL_SIZE] __attribute__((aligned(8)));
OAL_STATIC uint8_t g_mem_pool_shared_dscr[TOTAL_WLAN_MEM_SHARED_DSCR_SIZE] __attribute__((aligned(8)));
OAL_STATIC uint8_t g_mem_pool_tx_ring_dscr[TOTAL_WLAN_MEM_TX_RING_SIZE] __attribute__((aligned(8)));

#endif
/* 用于索引netbuf内存块与内存池ID的映射关系 */
OAL_STATIC uint32_t g_truesize_to_pool_id[oal_array_size(g_netbuf_cfg_tab)] = {0};

/* 用于索引sdt netbuf内存块与内存池ID的映射关系 */
OAL_STATIC uint32_t g_truesize_to_pool_id_sdt[oal_array_size(g_sdt_netbuf_cfg_tab)] = {0};

/* netbuf内存块data指针相对于head指针的偏移 */
uint32_t g_sdt_netbuf_def_data_offset[oal_array_size(g_sdt_netbuf_cfg_tab)] = {0};

/*
 * 内存池信息全局变量，存储整个内存管理中所有内存池信息
 * 所有内存管理的函数都基于此全局变量进行操作
 */
OAL_STATIC oal_mem_pool_stru g_st_mem_pool[OAL_MEM_POOL_ID_BUTT];

/* malloc内存指针记录 */
OAL_STATIC uint8_t *g_pool_base_addr[OAL_MEM_POOL_ID_BUTT] = { NULL };

/* dt netbuf内存索引表地址 */
OAL_STATIC oal_netbuf_stru **g_sdt_netbuf_stack_mem;

/* 总内存块个数 */
#define OAL_MEM_BLK_TOTAL_CNT (WLAN_MEM_SHARED_RX_DSCR_CNT + WLAN_MEM_SHARED_TX_DSCR_CNT1 +   \
                               WLAN_MEM_SHARED_TX_DSCR_CNT2 + WLAN_MEM_SHARED_MGMT_PKT_CNT1 + \
                               WLAN_MEM_SHARED_DATA_PKT_CNT + WLAN_MEM_LOCAL_CNT1 +           \
                               WLAN_MEM_LOCAL_CNT2 + WLAN_MEM_LOCAL_CNT3 +                    \
                               WLAN_MEM_LOCAL_CNT4 + WLAN_MEM_LOCAL_CNT5 +                    \
                               WLAN_MEM_LOCAL_CNT6 + WLAN_MEM_EVENT_CNT1 +                    \
                               WLAN_MEM_EVENT_CNT2 + WLAN_MEM_TX_RING_CNT1)

/* 一个内存块结构大小 + 一个指针大小 */
#define OAL_MEM_CTRL_BLK_SIZE (sizeof(oal_mem_stru *) + sizeof(oal_mem_stru))

/* netbuf内存块个数 */
#define OAL_MEM_NETBUF_BLK_TOTAL_CNT (WLAN_MEM_NETBUF_CNT1 + WLAN_MEM_NETBUF_CNT2 + WLAN_MEM_NETBUF_CNT3)

/* sdt netbuf内存块个数 */
#define OAL_MEM_SDT_NETBUF_BLK_TOTAL_CNT (WLAN_MEM_SDT_NETBUF_SIZE1_CNT + WLAN_MEM_SDT_NETBUF_SIZE2_CNT + \
                                          WLAN_MEM_SDT_NETBUF_SIZE3_CNT + WLAN_MEM_SDT_NETBUF_SIZE4_CNT)

/* netbuf内存指针大小 * 2 */
#define OAL_MEM_NETBUF_CTRL_BLK_SIZE (sizeof(oal_netbuf_stru *) * 2)

/* 内存块结构体内存大小 */
#define OAL_MEM_CTRL_BLK_TOTAL_SIZE (OAL_MEM_BLK_TOTAL_CNT * OAL_MEM_CTRL_BLK_SIZE + \
                                     OAL_MEM_SDT_NETBUF_BLK_TOTAL_CNT * OAL_MEM_NETBUF_CTRL_BLK_SIZE)

OAL_STATIC oal_netbuf_stru *g_sdt_netbuf_base_addr[OAL_MEM_SDT_NETBUF_BLK_TOTAL_CNT] = { NULL };

/*
 * 结构名  : oal_mem_ctrl_blk_stru
 * 结构说明: 将内存块所用空间封装成一个结构体
 */
typedef struct {
    uint8_t *puc_base_addr;
    uint32_t idx;
} oal_mem_ctrl_blk_stru;

/*
 * 控制块内存空间，为内存块结构体和指向内存块结构体的指针分配空间
 * 由函数oal_mem_ctrl_blk_alloc调用
 */
OAL_STATIC oal_mem_ctrl_blk_stru g_ctrl_blk = { NULL, 0 };

/* 四字节对齐填充缓存 */
#define OAL_MEM_MAX_WORD_ALIGNMENT_BUFFER 3

/*
 * 对于enhanced类型的申请接口与释放接口，每一个内存块都包含一个4字节的头部，
 * 用来指向内存块管理结构体oal_mem_struc，整个内存块的结构如下所示。
 * +-------------------+---------------------------------------------------+
 * | oal_mem_stru addr |                    payload                        |
 * +-------------------+---------------------------------------------------+
 * |      4 byte       |                                                   |
 * +-------------------+---------------------------------------------------+
 */
/* 内存池加锁 */
#define oal_mem_spin_lock_bh(_st_spinlock) \
    {                                      \
        oal_spin_lock_bh(&(_st_spinlock)); \
    }

/* 内存池解锁 */
#define oal_mem_spin_unlock_bh(_st_spinlock) \
    {                                        \
        oal_spin_unlock_bh(&(_st_spinlock)); \
    }

/* 内存池加锁(关中断) */
#define oal_mem_spin_lock_irqsave(_st_spinlock, _flag)     \
    {                                                         \
        oal_spin_lock_irq_save(&(_st_spinlock), &(_flag)); \
    }

/* 内存池解锁(开中断) */
#define oal_mem_spin_unlock_irqrestore(_st_spinlock, _flag)     \
    {                                                              \
        oal_spin_unlock_irq_restore(&(_st_spinlock), &(_flag)); \
    }

/*
 * 函 数 名  : oal_mem_ctrl_blk_alloc
 * 功能描述  : 为每个内存块结构体或指向内存块结构体的指针提供内存
 * 输入参数  : size:要分配内存的大小
 * 返 回 值  : 指向一块内存的指针 或空指针
 */
OAL_STATIC uint8_t *oal_mem_ctrl_blk_alloc(uint32_t size)
{
    uint8_t *puc_alloc;

    size = oal_get_4byte_align_value(size);
    if ((g_ctrl_blk.idx + size) > OAL_MEM_CTRL_BLK_TOTAL_SIZE) {
        oal_io_print("[file = %s, line = %d], oal_mem_ctrl_blk_alloc, not_enough memory!\n",
                     __FILE__, __LINE__);
        return NULL;
    }

    puc_alloc = g_ctrl_blk.puc_base_addr + g_ctrl_blk.idx;
    g_ctrl_blk.idx += size;

    return puc_alloc;
}
/*
 * 函 数 名  : oal_mem_get_ctrl_node
 * 功能描述  : 获取某内存块对应的信息节点
 * 输入参数  : void * p_data   payload内存块首地址
 * 返 回 值  : 指向内存块信息节点的指针 或空指针
 */
OAL_STATIC oal_mem_stru *oal_mem_get_ctrl_node(void *p_data)
{
    oal_mem_stru *pst_mem;
    uint32_t data;

    pst_mem = (oal_mem_stru *)(*((uintptr_t *)((uintptr_t)p_data - OAL_MEM_INFO_SIZE)));
    if (oal_unlikely(((uint8_t *)pst_mem < g_ctrl_blk.puc_base_addr)
                     || ((uint8_t *)pst_mem >= g_ctrl_blk.puc_base_addr + OAL_MEM_CTRL_BLK_TOTAL_SIZE))) {
        // 可能是因为前一个内存块被写穿了，将本内存块前面保存的内存块信息节点的地址覆盖了
        data = (*((oal_uint *)((uint8_t *)p_data - OAL_MEM_INFO_SIZE - OAL_DOG_TAG_SIZE)));
        oal_io_print("oal_mem_get_ctrl_node::mem covered 0x%x \n", data);
        return NULL;  //lint !e527
    }

    if (oal_unlikely(pst_mem->puc_data != (uint8_t *)p_data)) {
        // 异常，内存块信息节点记录的内存块地址与当前地址不同，严重问题!!!
        oal_io_print("oal_mem_get_ctrl_node::payload=0x%p,ctrl_node=0x%p\n",
                     (void *)p_data,
                     (void *)pst_mem);
        oal_io_print("[mem info] data_addr: 0x%p, user_cnt: %u, pool_id: %u, subpool_id: %u, len: %u, "
                     "alloc_core_id = %u, alloc_file_id: %u, alloc_line_num: %u, alloc_time_stamp: %u, "
                     "trace_file_id: %u, trace_line_num: %u, trace_time_stamp: %u.\n",
                     pst_mem->puc_data,
                     pst_mem->uc_user_cnt,
                     pst_mem->en_pool_id,
                     pst_mem->uc_subpool_id,
                     pst_mem->us_len,
                     pst_mem->alloc_core_id[0],
                     pst_mem->alloc_file_id[0],
                     pst_mem->alloc_line_num[0],
                     pst_mem->alloc_time_stamp[0],
                     pst_mem->trace_file_id,
                     pst_mem->trace_line_num,
                     pst_mem->trace_time_stamp);
        return NULL;  //lint !e527
    }
    return pst_mem;
}

/*
 * 函 数 名  : oal_mem_get_pool
 * 功能描述  : 根据内存池ID，获取内存池全局变量指针
 * 输入参数  : en_pool_id: 内存池ID
 * 返 回 值  : 指向内存池全局变量的指针 或空指针
 */
oal_mem_pool_stru *oal_mem_get_pool(oal_mem_pool_id_enum_uint8 en_pool_id)
{
    if (oal_unlikely(en_pool_id >= OAL_MEM_POOL_ID_BUTT)) {
        oal_io_print("[file = %s, line = %d], oal_mem_get_pool, array overflow!\n",
                     __FILE__, __LINE__);
        return NULL;
    }

    return &g_st_mem_pool[en_pool_id];
}
/*
 * 函 数 名  : oal_mem_get_pool_cfg_table
 * 功能描述  : 根据内存池ID，获取对应内存池配置信息
 * 输入参数  : en_pool_id: 内存池ID
 * 返 回 值  : 成功: 对应内存池配置信息结构的地址
 *             失败: 空指针
 */
oal_mem_pool_cfg_stru *oal_mem_get_pool_cfg_table(oal_mem_pool_id_enum_uint8 en_pool_id)
{
    if (oal_unlikely(en_pool_id >= OAL_MEM_POOL_ID_BUTT)) {
        oal_io_print("[file = %s, line = %d], oal_mem_get_pool_cfg_table, array overflow!\n",
                     __FILE__, __LINE__);
        return NULL;
    }

    return &g_mem_pool_cfg_tab[en_pool_id];
}

/*
 * 函 数 名  : oal_mem_find_available_netbuf
 * 功能描述  : 从本子内存池中找到一块可用的netbuf内存
 * 输入参数  : pst_mem_subpool: 子内存池结构体指针
 *             en_netbuf_id: netbuf内存池编号
 * 返 回 值  : 成功: 指向oal_netbuf_stru结构体的指针
 *             失败:NULLL
 */
OAL_STATIC oal_netbuf_stru *oal_mem_find_available_netbuf(oal_mem_subpool_stru *pst_mem_subpool,
                                                          oal_mem_pool_id_enum en_netbuf_id)
{
    oal_netbuf_stru *pst_netbuf = NULL;
    uint16_t us_top;
    uint16_t us_loop;
    uint16_t us_has_popped_netbuf = 0;
    oal_netbuf_stru **ppst_netbuf_stack_mem;

    us_top = pst_mem_subpool->us_free_cnt;

    if (en_netbuf_id == OAL_MEM_POOL_ID_SDT_NETBUF) {
        ppst_netbuf_stack_mem = g_sdt_netbuf_stack_mem;
    } else {
        return NULL;
    }

    while (us_top != 0) {
        us_top--;
        pst_netbuf = (oal_netbuf_stru *)pst_mem_subpool->ppst_free_stack[us_top];
        if (oal_netbuf_read_user(pst_netbuf) == 1) {
            break;
        }

        /* 如果netbuf的引用计数不为1，记录已弹出的netbuf内存指针的地址 */
        ppst_netbuf_stack_mem[us_has_popped_netbuf++] = pst_netbuf;
    }

    /* 将已弹出的netbuf内存指针再压回堆栈中 */
    for (us_loop = us_has_popped_netbuf; us_loop > 0; us_loop--) {
        pst_mem_subpool->ppst_free_stack[us_top++] = (oal_netbuf_stru *)ppst_netbuf_stack_mem[us_loop - 1];
    }

    /* 如果弹出的netbuf指针个数等于内存池可用内存个数，则认为该子内存池中暂时还没有可使用的内存(还被Qdisc缓存着) */
    if (us_has_popped_netbuf == pst_mem_subpool->us_free_cnt) {
        oal_io_print("popped netbuf index:%d\n", pst_mem_subpool->us_free_cnt);
        return NULL;
    }

    /* 更新子内存池可用内存块数 */
    pst_mem_subpool->us_free_cnt--;

    return pst_netbuf;
}

/*
 * 函 数 名  : oal_mem_release
 * 功能描述  : 恢复(释放)已经分配的内存
 */
OAL_STATIC void oal_mem_release(void)
{
    uint32_t pool_id;

#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
    uint32_t ret;
    uint32_t total_bytes = 0; /* 内存池总字节变量 */
#endif

    for (pool_id = 0; pool_id < OAL_MEM_POOL_ID_BUTT; pool_id++) {
        if (g_pool_base_addr[pool_id] != NULL) {
            g_pool_base_addr[pool_id] = NULL;
        }
    }

    if (g_ctrl_blk.puc_base_addr != NULL) {
        oal_free((void *)g_ctrl_blk.puc_base_addr);
        g_ctrl_blk.puc_base_addr = NULL;
    }
}

OAL_STATIC void oal_mem_sdt_netbuf_release(void)
{
    uint32_t loop;

    for (loop = 0; loop < OAL_MEM_SDT_NETBUF_BLK_TOTAL_CNT; loop++) {
        if (g_sdt_netbuf_base_addr[loop] == NULL) {
            continue;
        }

        /* 无论netbuf引用计数是多少，统一将其设置为1 */
        oal_netbuf_set_user(g_sdt_netbuf_base_addr[loop], 1);

        oal_netbuf_free(g_sdt_netbuf_base_addr[loop]);

        g_sdt_netbuf_base_addr[loop] = NULL;
    }
}

/*
 * 函 数 名  : oal_netbuf_duplicate
 * 功能描述  : 复制一个netbuf,只复制其中的内容，不复制计数
 * 输入参数  : oal_netbuf_stru* pst_src_netbuf 拷贝源netbuf
 *             uint8_t uc_out_subpool_id 指定新申请的netbuf 从哪个内存池拷贝
 *             uint32_t add_head_room  新增的headroom空间大小
 *             uint32_t add_tail_room  新增的tailroom空间大小
 * 返 回 值  : oal_netbuf_stru 一块可用的内存块的管理结构体,未申请到为NULL
 */
oal_netbuf_stru *oal_netbuf_duplicate(oal_netbuf_stru *pst_src_netbuf,
                                      uint8_t uc_out_subpool_id,
                                      uint32_t add_head_room,
                                      uint32_t add_tail_room)
{
    uint32_t dup_len = 0;
    oal_netbuf_stru *pst_dup_netbuf;
    int32_t ret;

    if (oal_warn_on(pst_src_netbuf == NULL)) {
        oal_io_print("%s error:pst_src_netbuf is null ", __FUNCTION__);
        return NULL;
    }

    dup_len += oal_netbuf_headroom(pst_src_netbuf);
    dup_len += oal_netbuf_len(pst_src_netbuf);
    dup_len += oal_netbuf_tailroom(pst_src_netbuf);
    dup_len += add_head_room;
    dup_len += add_tail_room;

    pst_dup_netbuf = oal_mem_netbuf_alloc(uc_out_subpool_id, dup_len, OAL_NETBUF_PRIORITY_MID);
    if (pst_dup_netbuf == NULL) {
        return NULL;
    }

    /* duplicate cb */
    ret = memcpy_s((void *)oal_netbuf_cb(pst_dup_netbuf), oal_netbuf_cb_size(),
                   (void *)oal_netbuf_cb(pst_src_netbuf), oal_netbuf_cb_size());
    if (ret != EOK) {
        oal_netbuf_free(pst_dup_netbuf);
        oal_io_print("duplicate cb failed.");
        return NULL;
    }
    skb_reserve(pst_dup_netbuf, (int32_t)oal_netbuf_headroom(pst_src_netbuf));
    oal_netbuf_put(pst_dup_netbuf, oal_netbuf_len(pst_src_netbuf));

    ret = memcpy_s((void *)oal_netbuf_data(pst_dup_netbuf), oal_netbuf_len(pst_dup_netbuf),
                   (void *)oal_netbuf_data(pst_src_netbuf), oal_netbuf_len(pst_src_netbuf));
    if (ret != EOK) {
        oal_netbuf_free(pst_dup_netbuf);
        oal_io_print("netbuf data copy failed.");
        return NULL;
    }

    return pst_dup_netbuf;
}

/*
 * 函 数 名  : oal_mem_init_ctrl_blk
 * 功能描述  : 初始化控制块内存(索引)
 */
OAL_STATIC void oal_mem_init_ctrl_blk(void)
{
    uint8_t *puc_base_addr;

    puc_base_addr = (uint8_t *)oal_memalloc(OAL_MEM_CTRL_BLK_TOTAL_SIZE);
    if (puc_base_addr == NULL) {
        oal_mem_release();
        oal_io_print("[file = %s, line = %d], oal_mem_init_ctrl_blk, memory allocation %u bytes fail!\n",
                     __FILE__, __LINE__, (uint32_t)OAL_MEM_CTRL_BLK_TOTAL_SIZE);
        return;
    }

    g_ctrl_blk.puc_base_addr = puc_base_addr;
    g_ctrl_blk.idx = 0;
}

/*
 * 函 数 名  : oal_mem_create_subpool
 * 功能描述  : 创建子内存池
 * 输入参数  : en_pool_id   : 内存池ID
 *             puc_base_addr: 内存池基地址
 * 返 回 值  : OAL_SUCC或其它错误码
 */
OAL_STATIC uint32_t oal_mem_create_subpool(oal_mem_pool_id_enum_uint8 en_pool_id, uint8_t *puc_base_addr)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_mem_stru *pst_mem;
    oal_mem_stru **ppst_stack_mem;
    uint8_t uc_subpool_id;
    uint32_t blk_id;

    if (puc_base_addr == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_subpool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem_pool = oal_mem_get_pool(en_pool_id);
    if (pst_mem_pool == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_subpool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_GET_POOL_FAIL;
    }

    /* 申请可用内存地址索引表，每个内存池申请一次，后面分割给每个子内存池使用 */
    ppst_stack_mem = (oal_mem_stru **)oal_mem_ctrl_blk_alloc(sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt);
    if (ppst_stack_mem == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_subpool, pointer is NULL!\n", __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_ALLOC_CTRL_BLK_FAIL;
    }

    /* 申请oal_mem_stru结构体，每个内存池申请一次，后面分割给每个子内存池使用 */
    pst_mem = (oal_mem_stru *)oal_mem_ctrl_blk_alloc(sizeof(oal_mem_stru) * pst_mem_pool->us_mem_total_cnt);
    if (pst_mem == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_subpool, pointer is NULL!\n", __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_ALLOC_CTRL_BLK_FAIL;
    }

    memset_s(ppst_stack_mem, sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt,
             0, sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt);
    memset_s((void *)pst_mem, sizeof(oal_mem_stru) * pst_mem_pool->us_mem_total_cnt,
             0, sizeof(oal_mem_stru) * pst_mem_pool->us_mem_total_cnt);

    /* 记录该内存池初始oal_mem_stru结构的指针，检查内存信息时使用 */
    pst_mem_pool->pst_mem_start_addr = pst_mem;

    /* 设置各子池所有内存块结构体信息，建立各内存块与payload的关系 */
    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        /* 得到每一级子内存池信息 */
        pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

        /* 建立子内存数索引表和可用内存索引表的关系 */
        pst_mem_subpool->ppst_free_stack = (void **)ppst_stack_mem;

        for (blk_id = 0; blk_id < pst_mem_subpool->us_total_cnt; blk_id++) {
            pst_mem->en_pool_id = en_pool_id;
            pst_mem->uc_subpool_id = uc_subpool_id;
            pst_mem->us_len = pst_mem_subpool->us_len;
            pst_mem->en_mem_state_flag = OAL_MEM_STATE_FREE;
            pst_mem->uc_user_cnt = 0;
            pst_mem->puc_origin_data = puc_base_addr; /* 建立oal_mem_st与对应payload的关系 */
            pst_mem->puc_data = pst_mem->puc_origin_data;

            memset_s(pst_mem->alloc_core_id, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM,
                     0, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM);
            memset_s(pst_mem->alloc_file_id, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM,
                     0, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM);
            memset_s(pst_mem->alloc_line_num, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM,
                     0, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM);
            memset_s(pst_mem->alloc_time_stamp, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM,
                     0, sizeof(uint32_t) * WLAN_MEM_MAX_USERS_NUM);
            pst_mem->trace_file_id = 0;
            pst_mem->trace_line_num = 0;
            pst_mem->trace_time_stamp = 0;
            /* 设置狗牌 */
            *((uint32_t *)(pst_mem->puc_origin_data + pst_mem->us_len - OAL_DOG_TAG_SIZE)) = (uint32_t)OAL_DOG_TAG;

            pst_mem->return_addr = 0;

            *ppst_stack_mem = pst_mem;
            ppst_stack_mem++;
            pst_mem++;

            puc_base_addr += pst_mem_subpool->us_len;
        }
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_mem_create_sdt_netbuf_subpool
 * 功能描述  : 创建netbuf子内存池
 * 输入参数  : en_pool_id: 内存池ID
 * 返 回 值  : OAL_SUCC或其它错误码
 */
OAL_STATIC uint32_t oal_mem_create_sdt_netbuf_subpool(oal_mem_pool_id_enum_uint8 en_pool_id)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_netbuf_stru **ppst_stack_mem;
    oal_netbuf_stru *pst_netbuf;
    uint8_t uc_subpool_id;
    uint32_t blk_id;
    uint32_t mem_total_cnt;

    pst_mem_pool = oal_mem_get_pool(en_pool_id);
    if (pst_mem_pool == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_sdt_netbuf_subpool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_GET_POOL_FAIL;
    }

    /* 申请可用内存地址索引表，每个内存池申请一次，后面分割给每个子内存池使用 */
    ppst_stack_mem = (oal_netbuf_stru **)oal_mem_ctrl_blk_alloc(sizeof(oal_netbuf_stru *) *
                                                                pst_mem_pool->us_mem_total_cnt);
    if (ppst_stack_mem == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_sdt_netbuf_subpool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_ALLOC_CTRL_BLK_FAIL;
    }

    /* 申请临时内存地址索引表，供后续查找可用内存使用 */
    g_sdt_netbuf_stack_mem = (oal_netbuf_stru **)oal_mem_ctrl_blk_alloc(sizeof(oal_netbuf_stru *) *
                                                                             pst_mem_pool->us_mem_total_cnt);
    if (g_sdt_netbuf_stack_mem == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_sdt_netbuf_subpool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_ALLOC_CTRL_BLK_FAIL;
    }
    memset_s(g_sdt_netbuf_stack_mem, sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt,
             0, sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt);
    memset_s(ppst_stack_mem, sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt,
             0, sizeof(oal_mem_stru *) * pst_mem_pool->us_mem_total_cnt);

    mem_total_cnt = 0;
    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        /* 得到每一级子内存池信息 */
        pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

        /* 建立子内存数索引表和可用内存索引表的关系 */
        pst_mem_subpool->ppst_free_stack = (void **)ppst_stack_mem;

        for (blk_id = 0; blk_id < pst_mem_subpool->us_total_cnt; blk_id++) {
            pst_netbuf = oal_mem_netbuf_alloc(OAL_NORMAL_NETBUF, pst_mem_subpool->us_len, OAL_NETBUF_PRIORITY_MID);
            if (oal_unlikely(pst_netbuf == NULL)) {
                oal_mem_sdt_netbuf_release();
                oal_io_print("[file = %s, line = %d], create_sdt_netbuf_subpool, alloc %u bytes, pointer is NULL!\n",
                             __FILE__, __LINE__, pst_mem_subpool->us_len);
                return OAL_ERR_CODE_ALLOC_MEM_FAIL;
            }

            if (blk_id == 0) {
                g_truesize_to_pool_id_sdt[uc_subpool_id] = pst_netbuf->truesize;

                g_sdt_netbuf_def_data_offset[uc_subpool_id] =
                    ((uintptr_t)pst_netbuf->data > (uintptr_t)pst_netbuf->head) ?
                        (uintptr_t)(pst_netbuf->data - pst_netbuf->head) : 0;
            }

            g_sdt_netbuf_base_addr[mem_total_cnt + blk_id] = pst_netbuf;

            *ppst_stack_mem = pst_netbuf;
            ppst_stack_mem++;
        }

        mem_total_cnt += pst_mem_subpool->us_total_cnt;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_mem_create_pool
 * 功能描述  : 设置每个内存池的子内存池结构体
 * 输入参数  : en_pool_id       : 内存池ID
 *             puc_data_mem_addr: 内存池基地址
 * 返 回 值  : OAL_SUCC 或其它错误码
 */
OAL_STATIC uint32_t oal_mem_create_pool(oal_mem_pool_id_enum_uint8 en_pool_id, uint8_t *puc_base_addr)
{
    uint8_t uc_subpool_id = 0;
    uint8_t uc_subpool_cnt;
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    uint32_t ret;
    const oal_mem_pool_cfg_stru *pst_mem_pool_cfg;
    const oal_mem_subpool_cfg_stru *pst_mem_subpool_cfg;

    /* 入参判断 */
    if (en_pool_id >= OAL_MEM_POOL_ID_BUTT) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_pool, array overflow!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    pst_mem_pool = oal_mem_get_pool(en_pool_id);
    if (pst_mem_pool == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_pool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_GET_POOL_FAIL;
    }

    pst_mem_pool_cfg = oal_mem_get_pool_cfg_table(en_pool_id);
    if (pst_mem_pool_cfg == NULL) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_pool, pointer is NULL!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_GET_CFG_TBL_FAIL;
    }

    /* 初始化内存池的通用变量 */
    uc_subpool_cnt = pst_mem_pool_cfg->uc_subpool_cnt;

    pst_mem_pool->uc_subpool_cnt = uc_subpool_cnt;
    pst_mem_pool->us_mem_used_cnt = 0;

    pst_mem_pool->us_max_byte_len = pst_mem_pool_cfg->pst_subpool_cfg_info[uc_subpool_cnt - 1].us_size;
    if (pst_mem_pool->us_max_byte_len >= WLAN_MEM_MAX_BYTE_LEN) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_pool, exceeds the max length!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_EXCEED_MAX_LEN;
    }

    if (pst_mem_pool->uc_subpool_cnt > WLAN_MEM_MAX_SUBPOOL_NUM) {
        oal_io_print("[file = %s, line = %d], oal_mem_create_pool, exceeds the max subpool number!\n",
                     __FILE__, __LINE__);
        return OAL_ERR_CODE_OAL_MEM_EXCEED_SUBPOOL_CNT;
    }

    /* 子池索引表初始清零 */
    memset_s((void *)pst_mem_pool->ast_subpool_table, sizeof(pst_mem_pool->ast_subpool_table),
             0, sizeof(pst_mem_pool->ast_subpool_table));

    /* 设置每一级子内存池 */
    for (uc_subpool_id = 0; uc_subpool_id < uc_subpool_cnt; uc_subpool_id++) {
        pst_mem_subpool_cfg = pst_mem_pool_cfg->pst_subpool_cfg_info + uc_subpool_id;
        pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

        pst_mem_subpool->us_free_cnt = pst_mem_subpool_cfg->us_cnt;
        pst_mem_subpool->us_total_cnt = pst_mem_subpool_cfg->us_cnt;
        pst_mem_subpool->us_len = pst_mem_subpool_cfg->us_size;

        oal_spin_lock_init(&pst_mem_subpool->st_spinlock);

        pst_mem_pool->us_mem_total_cnt += pst_mem_subpool_cfg->us_cnt; /* 设置总内存块数 */
    }

    if (en_pool_id == OAL_MEM_POOL_ID_SDT_NETBUF) {
        /* 创建sdt netbuf内存池 */
        ret = oal_mem_create_sdt_netbuf_subpool(en_pool_id);
    } else {
        /* 创建普通内存池 */
        ret = oal_mem_create_subpool(en_pool_id, puc_base_addr);
    }

    return ret;
}

/*
 * 函 数 名  : oal_mem_init_static_mem_map
 * 功能描述  : 初始化静态内存池
 */
OAL_STATIC void oal_mem_init_static_mem_map(void)
{
    memset_s(g_mem_pool_event, sizeof(g_mem_pool_event), 0, sizeof(g_mem_pool_event));
    memset_s(g_mem_pool_shared_data_pkt, sizeof(g_mem_pool_shared_data_pkt),
             0, sizeof(g_mem_pool_shared_data_pkt));
    memset_s(g_mem_pool_shared_mgmt_pkt, sizeof(g_mem_pool_shared_mgmt_pkt),
             0, sizeof(g_mem_pool_shared_mgmt_pkt));
    memset_s(g_mem_pool_local, sizeof(g_mem_pool_local), 0, sizeof(g_mem_pool_local));
    memset_s(g_mem_pool_shared_dscr, sizeof(g_mem_pool_shared_dscr),
             0, sizeof(g_mem_pool_shared_dscr));

    memset_s(g_mem_pool_tx_ring_dscr, sizeof(g_mem_pool_tx_ring_dscr), 0, sizeof(g_mem_pool_tx_ring_dscr));

    g_mem_pool[OAL_MEM_POOL_ID_EVENT] = g_mem_pool_event;
    g_mem_pool[OAL_MEM_POOL_ID_SHARED_DATA_PKT] = g_mem_pool_shared_data_pkt;
    g_mem_pool[OAL_MEM_POOL_ID_SHARED_MGMT_PKT] = g_mem_pool_shared_mgmt_pkt;
    g_mem_pool[OAL_MEM_POOL_ID_LOCAL] = g_mem_pool_local;
    g_mem_pool[OAL_MEM_POOL_ID_SHARED_DSCR] = g_mem_pool_shared_dscr;
    g_mem_pool[OAL_MEM_POOL_ID_TX_RING] = g_mem_pool_tx_ring_dscr;
}

/*
 * 函 数 名  : oal_mem_init_pool
 * 功能描述  : 初始化全部内存池
 * 返 回 值  : OAL_SUCC 或其它错误码
 */
uint32_t oal_mem_init_pool(void)
{
    uint32_t total_bytes = 0; /* 内存池总字节变量 */
    uint32_t pool_id = 0;     /* 内存池循环计数变量 */
    uint32_t ret;
    uint8_t *puc_base_addr; /* 定义malloc申请的内存基地址 */

    memset_s((void *)g_st_mem_pool, sizeof(g_st_mem_pool), 0, sizeof(g_st_mem_pool));
    memset_s((void *)g_pool_base_addr, sizeof(g_pool_base_addr),
             0, sizeof(g_pool_base_addr));
    /* 初始化控制块内存 */
    oal_mem_init_ctrl_blk();

    /* 初始化静态内存对应关系 */
    oal_mem_init_static_mem_map();

    for (pool_id = 0; pool_id < OAL_MEM_POOL_ID_SDT_NETBUF; pool_id++) {
        puc_base_addr = g_mem_pool[pool_id];

        if (puc_base_addr == NULL) {
            oal_mem_release();
            oal_io_print("[file = %s, line = %d], oal_mem_init_pool, memory allocation %u bytes fail!\n",
                         __FILE__, __LINE__, total_bytes + OAL_MEM_MAX_WORD_ALIGNMENT_BUFFER);
            return OAL_ERR_CODE_ALLOC_MEM_FAIL;
        }

        /* 记录每个内存池oal_malloc分配的地址 */
        g_pool_base_addr[pool_id] = puc_base_addr;

        puc_base_addr = (uint8_t *)(uintptr_t)oal_get_4byte_align_value((uintptr_t)puc_base_addr);

        ret = oal_mem_create_pool((uint8_t)pool_id, puc_base_addr);
        if (ret != OAL_SUCC) {
            oal_mem_release();
            oal_io_print("[file = %s, line = %d], oal_mem_init_pool, oal_mem_create_pool failed!\n",
                         __FILE__, __LINE__);
            return ret;
        }
    }

    /* 创建sdt netbuf内存池 */
    return oal_mem_create_pool(OAL_MEM_POOL_ID_SDT_NETBUF, NULL);
}

/*
 * 函 数 名  : oal_mem_alloc_enhanced
 * 功能描述  : 分配内存
 * 输入参数  : file_id  : 调用内存申请的文件ID
 *             line_num : 调用内存申请所在行号
 *             uc_pool_id  : 所申请内存的内存池ID
 *             us_len      : 所申请内存块长度
 *             uc_lock     : 是否需要加锁保护
 * 返 回 值  : 分配的内存块结构体指针，或空指针
 */
oal_mem_stru *oal_mem_alloc_enhanced(uint32_t file_id,
                                     uint32_t line_num,
                                     oal_mem_pool_id_enum_uint8 en_pool_id,
                                     uint16_t us_len,
                                     uint8_t uc_lock)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_mem_stru *pst_mem;
    uint8_t uc_subpool_id;
    oal_uint irq_flag = 0;
    uint8_t uc_subpool_cnt;
    oal_mem_subpool_stru *pst_subpool_table;

    /* 获取内存池 */
    pst_mem_pool = &g_st_mem_pool[en_pool_id];

    us_len += OAL_DOG_TAG_SIZE;

    /* 异常: 申请长度不在该内存池内  */
    if (oal_unlikely(us_len > pst_mem_pool->us_max_byte_len)) {
        return NULL;
    }

    pst_mem = NULL;

    uc_subpool_cnt = pst_mem_pool->uc_subpool_cnt;
    pst_subpool_table = pst_mem_pool->ast_subpool_table;

    for (uc_subpool_id = 0; uc_subpool_id < uc_subpool_cnt; uc_subpool_id++) {
        pst_mem_subpool = &(pst_subpool_table[uc_subpool_id]);
        oal_mem_spin_lock_irqsave(pst_mem_subpool->st_spinlock, irq_flag)
        if ((pst_mem_subpool->us_len < us_len) || (pst_mem_subpool->us_free_cnt == 0)) {
            oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)
            continue;
        }

        /* 获取一个没有使用的oal_mem_stru结点 */
        pst_mem_subpool->us_free_cnt--;
        pst_mem = (oal_mem_stru *)pst_mem_subpool->ppst_free_stack[pst_mem_subpool->us_free_cnt];

        pst_mem->puc_data = pst_mem->puc_origin_data;
        pst_mem->uc_user_cnt = 1;
        pst_mem->en_mem_state_flag = OAL_MEM_STATE_ALLOC;

        pst_mem_pool->us_mem_used_cnt++;

        pst_mem->alloc_core_id[0] = oal_get_core_id();
        pst_mem->alloc_file_id[0] = file_id;
        pst_mem->alloc_line_num[0] = line_num;
        pst_mem->alloc_time_stamp[0] = (uint32_t)oal_time_get_stamp_ms();

        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)
        break;
    }

    return pst_mem;
}

/*
 * 函 数 名  : oal_mem_free_enhanced
 * 功能描述  : 释放内存
 * 输入参数  : file_id  : 调用内存释放的文件ID
 *             line_num : 调用内存释放所在行号
 *             pst_mem     : 要释放内存块地址
 *             uc_lock     : 是否需要加锁保护
 * 返 回 值  : OAL_SUCC 或者其它错误码
 */
uint32_t oal_mem_free_enhanced(uint32_t file_id,
                               uint32_t line_num,
                               oal_mem_stru *pst_mem,
                               uint8_t uc_lock)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    uint32_t dog_tag;

    oal_uint irq_flag = 0;
    if (oal_unlikely(pst_mem == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem_pool = &g_st_mem_pool[pst_mem->en_pool_id];

    pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[pst_mem->uc_subpool_id]);

    oal_mem_spin_lock_irqsave(pst_mem_subpool->st_spinlock, irq_flag)

    /* 异常: 内存写越界 */
    dog_tag = (*((uint32_t *)(pst_mem->puc_origin_data + pst_mem->us_len - OAL_DOG_TAG_SIZE)));
    if (oal_unlikely(dog_tag != OAL_DOG_TAG)) {
        /* 恢复狗牌设置 */
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)
        return OAL_ERR_CODE_OAL_MEM_DOG_TAG;
    }

    /* 异常: 释放一块已经被释放的内存 */
    if (oal_unlikely(pst_mem->en_mem_state_flag == OAL_MEM_STATE_FREE)) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)
        return OAL_ERR_CODE_OAL_MEM_ALREADY_FREE;
    }

    /* 异常: 释放一块引用计数为0的内存 */
    if (oal_unlikely(pst_mem->uc_user_cnt == 0)) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)
        return OAL_ERR_CODE_OAL_MEM_USER_CNT_ERR;
    }

    pst_mem->alloc_core_id[pst_mem->uc_user_cnt - 1] = 0;
    pst_mem->alloc_file_id[pst_mem->uc_user_cnt - 1] = 0;
    pst_mem->alloc_line_num[pst_mem->uc_user_cnt - 1] = 0;
    pst_mem->alloc_time_stamp[pst_mem->uc_user_cnt - 1] = 0;

    pst_mem->uc_user_cnt--;

    /* 该内存块上是否还有其他共享用户，直接返回 */
    if (pst_mem->uc_user_cnt != 0) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)

        return OAL_SUCC;
    }

    /* 异常: 该子内存池可用内存块数目超过整个子内存池总内存块数 */
    if (oal_unlikely(pst_mem_subpool->us_free_cnt >= pst_mem_subpool->us_total_cnt)) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)

        return OAL_ERR_CODE_OAL_MEM_EXCEED_TOTAL_CNT;
    }

    pst_mem->trace_file_id = 0;
    pst_mem->trace_line_num = 0;
    pst_mem->trace_time_stamp = 0;

    pst_mem->return_addr = 0;

    pst_mem->en_mem_state_flag = OAL_MEM_STATE_FREE;

    pst_mem_subpool->ppst_free_stack[pst_mem_subpool->us_free_cnt] = (void *)pst_mem;
    pst_mem_subpool->us_free_cnt++;

    pst_mem_pool->us_mem_used_cnt--;

    oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_mem_free_enhanced_array
 * 功能描述  : 批量释放内存 (注意函数中不锁内存池, 要求内存块在同一个内存子池中)
 * 输入参数  : file_id         : 调用内存释放的文件ID
 *             line_num        : 调用内存释放所在行号
 *             p_data             : 要释放内存块数组头指针
 *             uc_mem_num         : 要释放的内存块数目
 * 返 回 值  : OAL_SUCC 或者其它错误码
 */
OAL_STATIC OAL_INLINE uint32_t oal_mem_free_enhanced_array(uint32_t file_id,
                                                           uint32_t line_num,
                                                           void *p_data,
                                                           uint8_t uc_mem_num)
{
    oal_mem_pool_stru *pst_mem_pool = NULL;
    oal_mem_subpool_stru *pst_mem_subpool = NULL;
    oal_mem_stru *pst_mem = NULL;
    uint8_t uc_index;
    oal_uint *past_mem;

    past_mem = (oal_uint *)p_data;

    for (uc_index = 0; uc_index < uc_mem_num; uc_index++) {
        pst_mem = (oal_mem_stru *)(*((uintptr_t *)((uintptr_t)past_mem[uc_index] - OAL_MEM_INFO_SIZE)));
        if (oal_unlikely(pst_mem == NULL)) {
            continue;
        }

        /* pool地址只需要取一次 */
        if (pst_mem_pool == NULL) {
            pst_mem_pool = &g_st_mem_pool[pst_mem->en_pool_id];
            pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[pst_mem->uc_subpool_id]);
        }

        /* 异常: 释放一块已经被释放的内存 */
        if (oal_unlikely(pst_mem->en_mem_state_flag == OAL_MEM_STATE_FREE)) {
            continue;
        }

        /* 异常: 释放一块引用计数为0的内存 */
        if (oal_unlikely(pst_mem->uc_user_cnt == 0)) {
            continue;
        }

        pst_mem->alloc_core_id[pst_mem->uc_user_cnt - 1] = 0;
        pst_mem->alloc_file_id[pst_mem->uc_user_cnt - 1] = 0;
        pst_mem->alloc_line_num[pst_mem->uc_user_cnt - 1] = 0;

        pst_mem->uc_user_cnt--;

        /* 该内存块上是否还有其他共享用户，直接返回 */
        if (oal_unlikely(pst_mem->uc_user_cnt != 0)) {
            continue;
        }

        if (oal_unlikely(pst_mem_subpool == NULL)) {
            continue;
        }

        /*lint -e613*/
        /* 异常: 该子内存池可用内存块数目超过整个子内存池总内存块数 */
        if (oal_unlikely(pst_mem_subpool->us_free_cnt >= pst_mem_subpool->us_total_cnt)) {
            continue;
        }

        pst_mem->en_mem_state_flag = OAL_MEM_STATE_FREE;
        /*lint -e613*/
        pst_mem_subpool->ppst_free_stack[pst_mem_subpool->us_free_cnt] = (void *)pst_mem;
        pst_mem_subpool->us_free_cnt++;
        /*lint +e613*/
        pst_mem_pool->us_mem_used_cnt--;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_mem_incr_user
 * 功能描述  : 内存块引用计数加1
 * 输入参数  : file_id  : 文件ID
 *             line_num : 行号
 *             pst_mem     : 要增加用户的内存块指针
 *             uc_lock     : 是否需要加锁保护
 * 返 回 值  : OAL_SUCC 或其它错误码
 */
uint32_t oal_mem_incr_user(uint32_t file_id,
                           uint32_t line_num,
                           oal_mem_stru *pst_mem,
                           uint8_t uc_lock)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_uint flag = 0;

    if (oal_unlikely(pst_mem == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem_pool = &g_st_mem_pool[pst_mem->en_pool_id];

    pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[pst_mem->uc_subpool_id]);

    oal_mem_spin_lock_irqsave(pst_mem_subpool->st_spinlock, flag)

    /* 异常: 内存块已经被释放 */
    if (oal_unlikely(pst_mem->en_mem_state_flag == OAL_MEM_STATE_FREE)) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, flag)

        return OAL_ERR_CODE_OAL_MEM_ALREADY_FREE;
    }

    /* 异常: 该内存块上的共享用户数已为最大值 */
    if (oal_unlikely((uint16_t)(pst_mem->uc_user_cnt + 1) > WLAN_MEM_MAX_USERS_NUM)) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, flag)

        return OAL_ERR_CODE_OAL_MEM_USER_CNT_ERR;
    }

    pst_mem->uc_user_cnt++;

    pst_mem->alloc_core_id[pst_mem->uc_user_cnt - 1] = oal_get_core_id();
    pst_mem->alloc_file_id[pst_mem->uc_user_cnt - 1] = file_id;
    pst_mem->alloc_line_num[pst_mem->uc_user_cnt - 1] = line_num;
    pst_mem->alloc_time_stamp[pst_mem->uc_user_cnt - 1] = (uint32_t)oal_time_get_stamp_ms();

    oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, flag)

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_mem_alloc
 * 功能描述  : 分配内存
 * 输入参数  : file_id  : 调用内存申请的文件ID
 *             line_num : 调用内存申请所在行号
 *             uc_pool_id  : 所申请内存的内存池ID
 *             us_len      : 所申请内存块长度
 *             uc_lock     : 是否需要加锁保护
 * 返 回 值  : 成功: 指向所分配内存起始地址的指针
 *             失败: 空指针
 */
void *oal_mem_alloc(uint32_t file_id,
                    uint32_t line_num,
                    oal_mem_pool_id_enum_uint8 en_pool_id,
                    uint16_t us_len,
                    uint8_t uc_lock)
{
    oal_mem_stru *pst_mem;

    /* 异常: 申请长度为零 */
    if (oal_unlikely(us_len == 0)) {
        return NULL;
    }

    us_len += OAL_MEM_INFO_SIZE;

    pst_mem = oal_mem_alloc_enhanced(file_id, line_num, en_pool_id, us_len, uc_lock);
    if (oal_unlikely(pst_mem == NULL)) {
        if (en_pool_id < OAL_MEM_POOL_ID_SHARED_DSCR) {
            oal_mem_print_normal_pool_info(en_pool_id);
        }

        return NULL;
    }

    pst_mem->return_addr = OAL_RET_ADDR;

    pst_mem->puc_data = pst_mem->puc_origin_data + OAL_MEM_INFO_SIZE;
    *((uintptr_t *)pst_mem->puc_origin_data) = (uintptr_t)pst_mem;
    return (void *)pst_mem->puc_data;
}

/*
 * 函 数 名  : oal_mem_free
 * 功能描述  : 释放内存
 * 输入参数  : file_id  : 调用内存释放的文件ID
 *             line_num : 调用内存释放所在行号
 *             p_data      : 要释放内存块地址
 *             uc_lock     : 是否需要加锁保护
 * 返 回 值  : OAL_SUCC 或者其它错误码
 */
uint32_t oal_mem_free(uint32_t file_id,
                      uint32_t line_num,
                      void *p_data,
                      uint8_t uc_lock)
{
    oal_mem_stru *pst_mem;
    uint32_t ret;

    if (oal_unlikely(p_data == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem = oal_mem_get_ctrl_node(p_data);
    if (oal_unlikely(pst_mem == NULL)) {
        return OAL_FAIL;
    }

    ret = oal_mem_free_enhanced(file_id, line_num, pst_mem, uc_lock);
    if (oal_unlikely(ret != OAL_SUCC)) {
        // 由于代码中几乎所有调用OAL_MEM_FREE接口释放内存资源时都未进行异常处理，所以在此处统一处理
        // 释放资源时出现异常，都值得引起重视!!!
        oal_io_print("oal_mem_free_enhanced failed!(%d)", ret);
    }
    return ret;
}

/*
 * 函 数 名  : oal_mem_free_array
 * 功能描述  : 批量释放内存 (注意函数中不锁内存池, 要求内存块在同一个内存子池中)
 * 输入参数  : file_id         : 调用内存释放的文件ID
 *             line_num        : 调用内存释放所在行号
 *             p_data             : 要释放内存块数组头指针
 *             uc_mem_num         : 要释放的内存块数目
 * 返 回 值  : OAL_SUCC 或者其它错误码
 */
/*lint -e695*/
uint32_t oal_mem_free_array(uint32_t file_id,
                            uint32_t line_num,
                            void *p_data,
                            uint8_t uc_mem_num)
{
    return oal_mem_free_enhanced_array(file_id, line_num, p_data, uc_mem_num);
}

/*lint +e695*/
/*
 * 函 数 名  : oal_mem_sdt_netbuf_alloc
 * 功能描述  : 分配netbuf内存
 * 输入参数  : us_len : 所申请内存块长度(即数据帧的长度，不包括netbuf结构体的长度)
 *             内存池的长度为netlink消息头+nlkpayload长度
 *             nlkpaload有sdt消息头+实际内容+sdt消息尾
 *             uc_lock: 是否需要加锁保护
 * 返 回 值  : 成功: 指向oal_netbuf_stru结构体的指针
 *             失败:NULLL
 */
oal_netbuf_stru *oal_mem_sdt_netbuf_alloc(uint16_t us_len, uint8_t uc_lock)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_netbuf_stru *pst_netbuf = NULL;
    uint8_t uc_subpool_id;
    oal_uint irq_flag = 0;
    uint32_t headroom;

    /* 获取内存池 */
    pst_mem_pool = &g_st_mem_pool[OAL_MEM_POOL_ID_SDT_NETBUF];

    /* 异常: 申请长度不在该内存池内  */
    if (oal_unlikely(us_len > pst_mem_pool->us_max_byte_len)) {
        oal_io_print("sdt netbuf alloc fail, len:%d, max len limit:%d\n", us_len, pst_mem_pool->us_max_byte_len);
        return NULL;
    }

    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

        oal_mem_spin_lock_irqsave(pst_mem_subpool->st_spinlock, irq_flag);

        /* 如果该子内存池可用内存块数为0，或者长度不符合，则跳到下一级子内存池寻找 */
        if ((pst_mem_subpool->us_len < us_len) || (pst_mem_subpool->us_free_cnt == 0)) {
            oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag);
            continue;
        }

        /* 如果该子内存池中找不到一块可用内存(引用计数为1的内存)，则跳到下一级子内存池寻找 */
        pst_netbuf = oal_mem_find_available_netbuf(pst_mem_subpool, OAL_MEM_POOL_ID_SDT_NETBUF);
        if (pst_netbuf == NULL) {
            oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag);
            continue;
        }

        /* 引用计数加1 */
        oal_netbuf_increase_user(pst_netbuf);

        pst_mem_pool->us_mem_used_cnt++;

        /* netbuf的data指针复位 */
        headroom = g_sdt_netbuf_def_data_offset[uc_subpool_id];

        if (oal_netbuf_headroom(pst_netbuf) > headroom) {
            oal_netbuf_push(pst_netbuf, oal_netbuf_headroom(pst_netbuf) - headroom);
        } else {
            oal_netbuf_pull(pst_netbuf, headroom - oal_netbuf_headroom(pst_netbuf));
        }

        if (oal_unlikely(oal_netbuf_headroom(pst_netbuf) != headroom)) {
            oal_io_print("%s error: oal_netbuf_headroom(pst_netbuf):%d not equal %u",
                         __FUNCTION__, (int32_t)oal_netbuf_headroom(pst_netbuf), headroom);
            oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag);
            break;
        }

        oal_netbuf_trim(pst_netbuf, pst_netbuf->len);

        if (oal_unlikely(pst_netbuf->len)) {
            oal_io_print("%s error: pst_netbuf->len:%d is not 0", __FUNCTION__, (int32_t)pst_netbuf->len);
        }
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag);

        break;
    }

    return pst_netbuf;
}

OAL_STATIC OAL_INLINE uint32_t oal_mem_find_netbuf_subpool_id(oal_netbuf_stru *pst_netbuf,
                                                              oal_mem_pool_stru *pst_mem_pool,
                                                              uint8_t *puc_subpool_id)
{
    uint8_t uc_subpool_id;

    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        if (g_truesize_to_pool_id[uc_subpool_id] == pst_netbuf->truesize) {
            *puc_subpool_id = uc_subpool_id;

            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/*
 * 函 数 名  : oal_mem_find_sdt_netbuf_subpool_id
 * 功能描述  : 查找sdt netbuf的子池
 */
OAL_STATIC OAL_INLINE uint32_t oal_mem_find_sdt_netbuf_subpool_id(oal_netbuf_stru *pst_netbuf,
                                                                  oal_mem_pool_stru *pst_mem_pool,
                                                                  uint8_t *puc_subpool_id)
{
    uint8_t uc_subpool_id;

    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        if (g_truesize_to_pool_id_sdt[uc_subpool_id] == pst_netbuf->truesize) {
            *puc_subpool_id = uc_subpool_id;

            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}

/*
 * 函 数 名  : oal_mem_sdt_netbuf_free
 * 功能描述  : 释放netbuf内存
 * 输入参数  : pst_netbuf: 指向oal_netbuf_stru结构体的指针
 *             uc_lock   : 是否需要加锁保护
 * 返 回 值  : OAL_SUCC 或者其它错误码
 */
uint32_t oal_mem_sdt_netbuf_free(oal_netbuf_stru *pst_netbuf, uint8_t uc_lock)
{
    uint8_t uc_subpool_id;
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    uint32_t ret;
    oal_uint irq_flag = 0;

    if (oal_unlikely(pst_netbuf == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem_pool = &g_st_mem_pool[OAL_MEM_POOL_ID_SDT_NETBUF];

    /* 获取该netbuf内存所属的子内存池ID */
    ret = oal_mem_find_sdt_netbuf_subpool_id(pst_netbuf, pst_mem_pool, &uc_subpool_id);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_netbuf_decrease_user(pst_netbuf);
        return OAL_ERR_CODE_OAL_MEM_SKB_SUBPOOL_ID_ERR;
    }

    pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

    oal_mem_spin_lock_irqsave(pst_mem_subpool->st_spinlock, irq_flag)

    /* 异常: 该子内存池可用内存块数目超过整个子内存池总内存块数 */
    if (oal_unlikely(pst_mem_subpool->us_free_cnt >= pst_mem_subpool->us_total_cnt)) {
        oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)

        return OAL_ERR_CODE_OAL_MEM_EXCEED_TOTAL_CNT;
    }

    /* 将该netbuf内存归还给相应的子内存池 */
    pst_mem_subpool->ppst_free_stack[pst_mem_subpool->us_free_cnt] = (void *)pst_netbuf;
    pst_mem_subpool->us_free_cnt++;

    pst_mem_pool->us_mem_used_cnt--;
    oal_netbuf_decrease_user(pst_netbuf);
    oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, irq_flag)

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_mempool_info_to_sdt_register
 * 功能描述  : 提供给oam模块的注册函数，注册的函数功能是将内存池信息上报SDT
 */
uint32_t oal_mempool_info_to_sdt_register(oal_stats_info_up_to_sdt p_up_mempool_info,
                                          oal_memblock_info_up_to_sdt p_up_memblock_info)
{
    g_mempool_info.p_mempool_info_func = p_up_mempool_info;
    g_mempool_info.p_memblock_info_func = p_up_memblock_info;

    return OAL_SUCC;
}

void oal_mem_info(oal_mem_pool_id_enum_uint8 en_pool_id)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    uint8_t uc_subpool_id;

    pst_mem_pool = oal_mem_get_pool(en_pool_id);
    if (oal_unlikely(pst_mem_pool == NULL)) {
        return;
    }

    oal_io_print("pool(%u) used/total=%u/%u:\r\n",
                 en_pool_id, pst_mem_pool->us_mem_used_cnt, pst_mem_pool->us_mem_total_cnt);
    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

        /* 子池使用情况，oal_mem_leak函数中会上报每个内存块的信息 */
        if (g_mempool_info.p_mempool_info_func != NULL) {
            g_mempool_info.p_mempool_info_func(en_pool_id,
                                               pst_mem_pool->us_mem_total_cnt,
                                               pst_mem_pool->us_mem_used_cnt,
                                               uc_subpool_id,
                                               pst_mem_subpool->us_total_cnt,
                                               pst_mem_subpool->us_free_cnt);
        }

        oal_io_print("    subpool(%d) free/total=%u/%u\r\n",
                     uc_subpool_id, pst_mem_subpool->us_free_cnt,
                     pst_mem_subpool->us_total_cnt);
    }
}

/*
 * 函 数 名  : oal_get_func_name
 * 功能描述  : 将函数地址转换为函数符号
 * 输入参数  : call_func_ddr: 输入参量，函数返回地址
 * 输出参数  : buff: 输出参量，存储函数符号
 * 返 回 值  : buf_cnt: buff存储字节长度
 */
uint8_t oal_get_func_name(uint8_t *buff, uintptr_t call_func_ddr)
{
    uint8_t buf_cnt;

#if (_PRE_OS_VERSION == _PRE_OS_VERSION_LINUX)
    buf_cnt = sprint_symbol(buff, call_func_ddr);
#else
    buf_cnt = 0;
    *buff = '\0';
#endif

    return buf_cnt;
}

/*
 * 函 数 名  : oal_print_func
 * 功能描述  : linux 下打印函数调用关系
 */
void oal_mem_print_funcname(uintptr_t func_addr)
{
    uint8_t size;
    uint8_t buff[sizeof("Func: ") + OAL_MEM_SPRINT_SYMBOL_SIZE] = {"Func: "};

    size = oal_get_func_name(buff + strlen("Func: "), func_addr);
    if (size > 0) {
        oam_print(buff); /* OTA打印函数符号 */
    } else {
        oal_io_print("mem printf func name failed.\n");
    }
}
oal_module_symbol(oal_mem_print_funcname);

/*
 * 函 数 名  : oal_mem_return_addr_count
 * 功能描述  : 统计某个内存池中，某个函数总共占用内存块个数
 */
uint32_t oal_mem_return_addr_count(oal_mem_subpool_stru *pst_mem_subpool, oal_mem_stru *pst_mem_base,
                                   uintptr_t call_func_addr)
{
    uint16_t us_loop;
    oal_mem_stru *pst_mem;
    uint32_t us_count = 0;

    pst_mem = pst_mem_base;

    for (us_loop = 0; us_loop < pst_mem_subpool->us_total_cnt; us_loop++) {
        if ((pst_mem->en_mem_state_flag == OAL_MEM_STATE_ALLOC) && (pst_mem->return_addr == call_func_addr)) {
            us_count++;
        }
        pst_mem++;
    }

    return us_count;
}

/*
 * 函 数 名  : oal_mem_func_addr_is_registerd
 * 功能描述  : 统计数组中是否已经存储某个函数地址
 * 输入参数  : ua_func_addr: 保存函数地址的数组
 *             call_func_addr: 函数地址
 * 输出参数  : p_func_loop: 数组中第一个为0的下标值
 */
uint8_t oal_mem_func_addr_is_registerd(uintptr_t *ua_func_addr, uint8_t uc_func_size,
                                       uint8_t *p_func_loop, uintptr_t call_func_addr)
{
    uint8_t uc_loop = 0;

    /* 非零则判断是否函数地址相同 */
    while (ua_func_addr[uc_loop]) {
        /* 如果存在则返回true */
        if (ua_func_addr[uc_loop] == call_func_addr) {
            return OAL_TRUE;
        }

        uc_loop++;
        if (uc_func_size == uc_loop) {
            break;
        }
    }

    if (uc_func_size == uc_loop) {
        uc_loop = 0;
    }

    /* 不存在则需要记录数组下标 */
    *p_func_loop = uc_loop;

    return OAL_FALSE;
}

/*
 * 函 数 名  : oal_mem_print_normal_pool_info
 * 功能描述  : 内存池溢出时，统计上报普通内存占用情况
 */
void oal_mem_print_normal_pool_info(oal_mem_pool_id_enum_uint8 en_pool_id)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_mem_stru *pst_mem;
    oal_mem_stru *pst_mem_base;
    uint16_t us_loop;
    uint8_t uc_subpool_id = 0;
    uintptr_t ua_func_addr[50] = {0}; /* 50个元素足够存放一组地址 */
    uint8_t us_func_loop = 0;
    uint32_t us_ret_count = 0;
    uint8_t buff[sizeof("[%d] mem blocks occupied by ") + OAL_MEM_SPRINT_SYMBOL_SIZE] = {
        "[%d] mem blocks occupied by "};
    uint8_t size;

    pst_mem_pool = oal_mem_get_pool(en_pool_id);
    if (oal_unlikely(pst_mem_pool == NULL)) {
        return;
    }

    oam_warning_log4(0, OAM_SF_CFG,
                     "{host memory info: pool id=%d, subpool cnt=%d, mem block total cnt=%d, used cnt=%d.}",
                     en_pool_id, pst_mem_pool->uc_subpool_cnt,
                     pst_mem_pool->us_mem_total_cnt,
                     pst_mem_pool->us_mem_used_cnt);

    pst_mem = pst_mem_pool->pst_mem_start_addr;

    /* 循环每一个子池 */
    for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
        /* 得到每一级子内存池信息 */
        pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

        oam_warning_log4(0, OAM_SF_CFG, "{host subpool id[%d] info: len =%d, mem block total cnt=%d, free cnt=%d.}",
                         uc_subpool_id, pst_mem_subpool->us_len,
                         pst_mem_subpool->us_total_cnt,
                         pst_mem_subpool->us_free_cnt);

        /* 保存新子池基地址 */
        pst_mem_base = pst_mem;

        /* 循环查询每一个子池的mem block */
        for (us_loop = 0; us_loop < pst_mem_subpool->us_total_cnt; us_loop++) {
            if ((pst_mem->return_addr == 0) && (pst_mem->en_mem_state_flag == OAL_MEM_STATE_ALLOC)) {
                oam_warning_log2(0, OAM_SF_CFG,
                    "{oal_mem_print_normal_pool_info:: subpool id[%d] mem block[%d] has no call func addr.}",
                    uc_subpool_id, us_loop);
            } else if ((pst_mem->en_mem_state_flag == OAL_MEM_STATE_ALLOC) &&
                       (oal_mem_func_addr_is_registerd(ua_func_addr, (uint8_t)sizeof(ua_func_addr), &us_func_loop,
                                                       pst_mem->return_addr) == OAL_FALSE)) {
                ua_func_addr[us_func_loop] = pst_mem->return_addr;

                us_ret_count = oal_mem_return_addr_count(pst_mem_subpool, pst_mem_base, pst_mem->return_addr);

                /* OTA打印函数符号 */
                size = oal_get_func_name(buff + strlen("[%d] mem blocks occupied by "), pst_mem->return_addr);
                if (size > 0) {
                    oam_print(buff);
                }
            }
            /* 查询下一个内存块 */
            pst_mem++;
        }
        /* 查询完一个子池，清空数组 */
        memset_s(ua_func_addr, sizeof(ua_func_addr), 0, sizeof(ua_func_addr));
    }
}
oal_module_symbol(oal_mem_print_normal_pool_info);

/*
 * 函 数 名  : oal_mem_print_pool_info
 * 功能描述  : 统计输出所有内存池信息
 */
void oal_mem_print_pool_info(void)
{
    uint8_t uc_loop;

    for (uc_loop = 0; uc_loop <= OAL_MEM_POOL_ID_SHARED_DSCR; uc_loop++) {
        oal_mem_print_normal_pool_info(uc_loop);
    }
}
oal_module_symbol(oal_mem_print_pool_info);

void oal_mem_leak(oal_mem_pool_id_enum_uint8 en_pool_id)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_stru *pst_mem;
    oal_bool_enum_uint8 en_flag = OAL_TRUE;
    uint16_t us_loop;

    pst_mem_pool = oal_mem_get_pool(en_pool_id);
    if (oal_unlikely(pst_mem_pool == NULL)) {
        return;
    }

    pst_mem = pst_mem_pool->pst_mem_start_addr;

    for (us_loop = 0; us_loop < pst_mem_pool->us_mem_total_cnt; us_loop++) {
        /* 如果行号 != 0，则说明有内存没有被释放(不可能在第0行申请内存) */
        if (pst_mem->alloc_line_num[0] != 0) {
            /* 打印当前时间戳 */
            if (en_flag == OAL_TRUE) {
                oal_io_print("[memory leak] current time stamp: %u.\n", (uint32_t)oal_time_get_stamp_ms());
                en_flag = OAL_FALSE;
            }

            oal_io_print("[memory leak] user_cnt: %u, pool_id: %u, subpool_id: %u, len: %u, "
                         "alloc_core_id = %u, alloc_file_id: %u, alloc_line_num: %u, alloc_time_stamp: %u, "
                         "trace_file_id: %u, trace_line_num: %u, trace_time_stamp: %u.\n",
                         pst_mem->uc_user_cnt,
                         pst_mem->en_pool_id,
                         pst_mem->uc_subpool_id,
                         pst_mem->us_len,
                         pst_mem->alloc_core_id[0],
                         pst_mem->alloc_file_id[0],
                         pst_mem->alloc_line_num[0],
                         pst_mem->alloc_time_stamp[0],
                         pst_mem->trace_file_id,
                         pst_mem->trace_line_num,
                         pst_mem->trace_time_stamp);
        }

        /* 每个内存块的信息，在oal_mem_info中会上报每个子池的信息 zouhongliang SDT */
        if (g_mempool_info.p_memblock_info_func != NULL) {
            g_mempool_info.p_memblock_info_func(pst_mem->puc_origin_data,
                                                pst_mem->uc_user_cnt,
                                                pst_mem->en_pool_id,
                                                pst_mem->uc_subpool_id,
                                                pst_mem->us_len,
                                                pst_mem->alloc_file_id[0],
                                                pst_mem->alloc_line_num[0]);
        }

        pst_mem++;
    }
}

/*
 * 函 数 名  : oal_mem_stat
 * 功能描述  : 统计各个内存池使用情况
 */
OAL_STATIC void oal_mem_statistics(oal_mem_pool_stat *past_mem_pool_stat)
{
    oal_mem_pool_id_enum_uint8 en_pool_id;
    uint8_t uc_subpool_id;
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;

    for (en_pool_id = 0; en_pool_id < OAL_MEM_POOL_ID_BUTT; en_pool_id++) {
        pst_mem_pool = &g_st_mem_pool[en_pool_id];

        /* 记录本内存池使用总量 */
        past_mem_pool_stat[en_pool_id].us_mem_used_cnt = pst_mem_pool->us_mem_used_cnt;
        past_mem_pool_stat[en_pool_id].us_mem_total_cnt = pst_mem_pool->us_mem_total_cnt;

        /* 记录各子池使用状况 */
        for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
            pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[uc_subpool_id]);

            past_mem_pool_stat[en_pool_id].ast_subpool_stat[uc_subpool_id].us_free_cnt = pst_mem_subpool->us_free_cnt;
            past_mem_pool_stat[en_pool_id].ast_subpool_stat[uc_subpool_id].us_total_cnt = pst_mem_subpool->us_total_cnt;
        }
    }
}

/*
 * 函 数 名  : oal_mem_check
 * 功能描述  : 检测内存池是否有泄漏
 */
OAL_STATIC uint32_t oal_mem_check(oal_mem_pool_stat *past_stat_start, oal_mem_pool_stat *past_stat_end)
{
    uint8_t uc_bitmap = 0;
    oal_mem_pool_id_enum_uint8 en_pool_id;
    uint8_t uc_subpool_id;
    oal_mem_pool_stru *pst_mem_pool;

    for (en_pool_id = 0; en_pool_id < OAL_MEM_POOL_ID_BUTT; en_pool_id++) {
        /* 查看本内存池总量(两次统计结果是否一致) */
        if ((past_stat_start[en_pool_id].us_mem_used_cnt != past_stat_end[en_pool_id].us_mem_used_cnt) ||
            (past_stat_start[en_pool_id].us_mem_total_cnt != past_stat_end[en_pool_id].us_mem_total_cnt)) {
            uc_bitmap |= (uint8_t)(1 << en_pool_id);
            continue;
        }

        pst_mem_pool = &g_st_mem_pool[en_pool_id];

        /* 查看各子内存池使用状况(两次统计结果是否一致) */
        for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
            if ((past_stat_start[en_pool_id].ast_subpool_stat[uc_subpool_id].us_free_cnt !=
                 past_stat_end[en_pool_id].ast_subpool_stat[uc_subpool_id].us_free_cnt) ||
                (past_stat_start[en_pool_id].ast_subpool_stat[uc_subpool_id].us_total_cnt !=
                 past_stat_end[en_pool_id].ast_subpool_stat[uc_subpool_id].us_total_cnt)) {
                uc_bitmap |= (uint8_t)(1 << en_pool_id);
                continue;
            }
        }
    }

    /* 两次统计结果一致(没有内存泄漏)，返回 */
    if (uc_bitmap == 0) {
        oal_io_print("no memory leak!\n");
        return OAL_FALSE;
    }

    /* 两次统计结果不一致(有内存泄漏)，打印有泄漏的内存池的统计信息 */
    oal_io_print("memory leak!\n");
    for (en_pool_id = 0; en_pool_id < OAL_MEM_POOL_ID_BUTT; en_pool_id++) {
        if (!(uc_bitmap & (1 << en_pool_id))) {
            continue;
        }

        pst_mem_pool = &g_st_mem_pool[en_pool_id];

        oal_io_print("                      Start\t\tEnd\t\n");
        oal_io_print("pool(%d) used cnt:     %d\t\t%d\t\n",
                     en_pool_id, past_stat_start[en_pool_id].us_mem_used_cnt,
                     past_stat_end[en_pool_id].us_mem_used_cnt);
        oal_io_print("pool(%d) total cnt:    %d\t\t%d\t\n",
                     en_pool_id, past_stat_start[en_pool_id].us_mem_total_cnt,
                     past_stat_end[en_pool_id].us_mem_total_cnt);

        for (uc_subpool_id = 0; uc_subpool_id < pst_mem_pool->uc_subpool_cnt; uc_subpool_id++) {
            oal_io_print("subpool(%d) free cnt:  %d\t\t%d\t\n",
                         uc_subpool_id, past_stat_start[en_pool_id].ast_subpool_stat[uc_subpool_id].us_free_cnt,
                         past_stat_end[en_pool_id].ast_subpool_stat[uc_subpool_id].us_free_cnt);
            oal_io_print("subpool(%d) total cnt: %d\t\t%d\t\n",
                         uc_subpool_id, past_stat_start[en_pool_id].ast_subpool_stat[uc_subpool_id].us_total_cnt,
                         past_stat_end[en_pool_id].ast_subpool_stat[uc_subpool_id].us_total_cnt);
        }

        oal_io_print("\n");
    }

    return OAL_TRUE;
}

/*
 * 函 数 名  : oal_mem_start_stat
 * 功能描述  : 内存检测接口(需要与oal_mem_end_stat配对使用)
 */
void oal_mem_start_stat(void)
{
    memset_s(&g_mem_stat, sizeof(g_mem_stat), 0, sizeof(g_mem_stat));

    /* 记录各内存池使用状况 */
    oal_mem_statistics(g_mem_stat.ast_mem_start_stat);
}

/*
 * 函 数 名  : oal_mem_end_stat
 * 功能描述  : 内存检测接口(需要与oal_mem_start_stat配对使用)
 * 返 回 值  : OAL_TRUE:  有内存泄漏
 *             OAL_FALSE: 无内存泄漏
 */
uint32_t oal_mem_end_stat(void)
{
    oal_mem_statistics(g_mem_stat.ast_mem_end_stat);

    /* 检测内存池是否有泄漏 */
    return oal_mem_check(g_mem_stat.ast_mem_start_stat, g_mem_stat.ast_mem_end_stat);
}

uint32_t oal_mem_trace_enhanced(uint32_t file_id,
                                uint32_t line_num,
                                oal_mem_stru *pst_mem,
                                uint8_t uc_lock)
{
    oal_mem_pool_stru *pst_mem_pool;
    oal_mem_subpool_stru *pst_mem_subpool;
    oal_uint flag = 0;

    if (oal_unlikely(pst_mem == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem_pool = &g_st_mem_pool[pst_mem->en_pool_id];

    pst_mem_subpool = &(pst_mem_pool->ast_subpool_table[pst_mem->uc_subpool_id]);

    oal_mem_spin_lock_irqsave(pst_mem_subpool->st_spinlock, flag)

    pst_mem->trace_file_id = file_id;
    pst_mem->trace_line_num = line_num;
    pst_mem->trace_time_stamp = (uint32_t)oal_time_get_stamp_ms();

    oal_mem_spin_unlock_irqrestore(pst_mem_subpool->st_spinlock, flag)

    return OAL_SUCC;
}

uint32_t oal_mem_trace(uint32_t file_id,
                       uint32_t line_num,
                       const void *p_data,
                       uint8_t uc_lock)
{
    oal_mem_stru *pst_mem;

    if (oal_unlikely(p_data == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_mem = (oal_mem_stru *)(*((uintptr_t *)((uintptr_t)p_data - OAL_MEM_INFO_SIZE)));

    return oal_mem_trace_enhanced(file_id, line_num, pst_mem, uc_lock);
}

/*
 * 函 数 名  : oal_mem_exit
 * 功能描述  : 内存模块卸载接口
 */
void oal_mem_exit(void)
{
    /* 卸载普通内存池 */
    oal_mem_release();

    oal_mem_sdt_netbuf_release();

    memset_s(g_st_mem_pool, sizeof(g_st_mem_pool), 0, sizeof(g_st_mem_pool));

    return;
}

/*lint -e19*/
oal_module_symbol(oal_mem_free);
oal_module_symbol(oal_mem_free_array);

oal_module_symbol(oal_mem_alloc);

/*lint -e19*/
oal_module_symbol(oal_netbuf_duplicate);
oal_module_symbol(oal_mem_alloc_enhanced);
oal_module_symbol(oal_mem_free_enhanced);
oal_module_symbol(oal_mem_incr_user);
oal_module_symbol(oal_mem_sdt_netbuf_alloc);
oal_module_symbol(oal_mem_sdt_netbuf_free);
oal_module_symbol(oal_mem_leak);
oal_module_symbol(oal_mem_info);
oal_module_symbol(oal_mem_trace_enhanced);
oal_module_symbol(oal_mem_trace);
oal_module_symbol(oal_mempool_info_to_sdt_register);
oal_module_symbol(g_sdt_netbuf_base_addr);
#ifdef _PRE_WLAN_CACHE_COHERENT_SUPPORT
oal_module_symbol(g_dscr_fstvirt_addr);
oal_module_symbol(g_dscr_fstphy_addr);
#endif
