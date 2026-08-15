

#ifndef __OAL_SCHEDULE_H__
#define __OAL_SCHEDULE_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "oal_hardware.h"
#include "oal_mm.h"
#include "arch/oal_schedule.h"
#include "oal_list.h"
#include "securec.h"
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
#include <linux/pm_wakeup.h>
#endif
/*****************************************************************************
  2 宏定义
*****************************************************************************/
#if (_PRE_CHIP_BITS_MIPS32 == _PRE_CHIP_BITS)
/* 32位寄存器最大长度 */
#define OAL_TIME_US_MAX_LEN 0xFFFFFFFF

#elif (_PRE_CHIP_BITS_MIPS64 == _PRE_CHIP_BITS)
/* 64位寄存器最大长度 */
#define OAL_TIME_US_MAX_LEN 0xFFFFFFFFFFFFFFFF

#endif

#define oal_sys_wdt_start(mode, msec)
#define oal_sys_wdt_start_int(msec)
#define oal_sys_wdt_start_noint(msec)
#define oal_sys_wdt_stop()
#define oal_sys_wdt_feed()

#define oal_sys_bus_idle()

#if defined(PLATFORM_DEBUG_ENABLE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define oal_debug_module_param        module_param
#define oal_debug_module_param_string module_param_string
#define OAL_DEBUG_MODULE_PARM_DESC    MODULE_PARM_DESC
#else
#define oal_debug_module_param(_symbol, _type, _name)
#define oal_debug_module_param_string(arg1, arg2, arg3, arg4)
#define OAL_DEBUG_MODULE_PARM_DESC(arg1, arg2)
#endif

typedef enum {
    OAL_DFT_TRACE_OTHER = 0x0,
    OAL_DFT_TRACE_SUCC,
    OAL_DFT_TRACE_FAIL,  /* > fail while dump the key_info to kernel */
    OAL_DFT_TRACE_EXCEP, /* exception */
    OAL_DFT_TRACE_BUTT,
} oal_dft_type;

#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
#include <linux/rtc.h>
typedef struct _oal_dft_trace_item_ {
    oal_list_entry_stru list;
    char *name;
    oal_uint16 dft_type;
    oal_uint16 trace_flag; /* add the the list */
    oal_uint32 trace_count;
    struct timeval first_timestamp; /* first keyinfo timestamp */
    struct timeval last_timestamp;  /* last keyinfo timestamp */
} oal_dft_trace_item;

extern oal_spin_lock_stru dft_head_lock;
extern oal_list_head_stru dft_head;
extern oal_int32 oal_dft_init(oal_void);
extern oal_void oal_dft_exit(oal_void);
extern oal_void oal_dft_print_error_key_info(oal_void);
extern oal_void oal_dft_print_all_key_info(oal_void);

OAL_STATIC OAL_INLINE oal_void oal_dft_trace_key_info_func(oal_dft_trace_item* pst_dft_item, 
                                                           char* dname,
                                                           oal_uint16 dtype)
{
    oal_ulong flags__;
    oal_spin_lock_irq_save(&dft_head_lock, &flags__);
    if (!pst_dft_item->trace_flag) {
        oal_list_add(&pst_dft_item->list, &dft_head);
        pst_dft_item->name = dname;
        pst_dft_item->dft_type = dtype;
        pst_dft_item->trace_flag = 1;
    }
    pst_dft_item->trace_count++;
    oal_spin_unlock_irq_restore(&dft_head_lock, &flags__);
    if ((!pst_dft_item->first_timestamp.tv_sec) && (!pst_dft_item->first_timestamp.tv_usec)) {
        do_gettimeofday(&pst_dft_item->first_timestamp);
    } else {
        do_gettimeofday(&pst_dft_item->last_timestamp);
    }
    if (OAL_UNLIKELY(dtype >= OAL_DFT_TRACE_FAIL)) {
        printk(KERN_ERR "[E]key_info:%s happened[%u]!\n", dname, pst_dft_item->trace_count);
    }
}

/* 关键流程发生时间点记录，有加锁动作，慎用 */
#define DECLARE_DFT_TRACE_KEY_INFO(dname, dtype)                                               \
    do {                                                                                       \
        OAL_STATIC oal_dft_trace_item st_dft_item = {{0}, NULL, 0, 0, 0, {0}, {0}};    \
        oal_dft_trace_key_info_func(&st_dft_item, dname, dtype);                                      \
    } while (0)
#else
#define DECLARE_DFT_TRACE_KEY_INFO(dname, dtype)
#endif

#define PRINT_RATE_SECOND (1000)
#define PRINT_RATE_MINUTE (60 * 1000)
#define PRINT_RATE_HOUR   (60 * PRINT_RATE_MINUTE)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC OAL_INLINE int oal_print_rate_limit_func(int* first_print, oal_ulong* begin, 
                                                        int* missed, oal_uint32 timeout, oal_int32 linenum)
{
    int ret__;
    if ((!!(*first_print)) && ((timeout != 0) && (!time_is_before_jiffies((*begin) + msecs_to_jiffies(timeout))))) {
        ret__ = 0;
        (*missed)++;
    } else {
        if (*missed) {
            printk(KERN_DEBUG "[%d]logs dropped line %d[timeout=%u]\n", *missed, linenum, timeout);
        }
        ret__ = 1;
        *missed = 0;
        *begin = jiffies;
        *first_print = 1;
    }

    return ret__;
}

/* 微妙超时，当timeout=1000 说明当打印频率高于1000ms时返回true
  否者压制打印返回false, 当timeout=0 侧不压制打印 */

#define oal_print_rate_limit(timeout) ({                                                                      \
    int ret__;                                                                                                   \
    static int first_print = 0;                                                                                \
    static oal_ulong begin = 0;                                                                                \
    static int missed = 0;                                                                                     \
    ret__ =  oal_print_rate_limit_func(&first_print, &begin, &missed, timeout, __LINE__);                                          \
    (ret__);                                                                                                     \
})

#else
OAL_STATIC OAL_INLINE oal_int32 oal_print_rate_limit(oal_ulong timeout)
{
    return 1;
}
#endif

typedef struct _oal_wakelock_stru_ {
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    struct wakeup_source st_wakelock;  // wakelock锁
    oal_spin_lock_stru lock;           // wakelock锁操作spinlock锁
    oal_dlist_head_stru list;
#endif
    oal_ulong lock_count;  // 持有wakelock锁的次数
    uintptr_t locked_addr; /* the locked address */
    oal_uint32 debug;      // debug flag
} oal_wakelock_stru;

/* 获取从_ul_start到_ul_end的时间差 */
#define OAL_TIME_GET_RUNTIME(_ul_start, _ul_end) \
    (((_ul_start) > (_ul_end)) ? (OAL_TIME_CALC_RUNTIME((_ul_start), (_ul_end))) : ((_ul_end) - (_ul_start)))

/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern oal_spin_lock_stru g_wakelock_lock;
extern oal_dlist_head_stru g_wakelock_head;

/*****************************************************************************
  5 消息头定义
*****************************************************************************/

/*****************************************************************************
  6 消息定义
*****************************************************************************/

/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

/*****************************************************************************
  8 UNION定义
*****************************************************************************/

/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/

/*****************************************************************************
  10 函数声明
*****************************************************************************/
#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
extern oal_int32 oal_softwdt_init(oal_void);
extern oal_void oal_softwdt_exit(oal_void);
#else
OAL_STATIC OAL_INLINE oal_int32 oal_softwdt_init(oal_void)
{
    return OAL_SUCC;
}
OAL_STATIC OAL_INLINE oal_void oal_softwdt_exit(oal_void)
{
    return;
}
#endif

oal_int32 oal_print_all_wakelock_buff(char *buf, oal_int32 buf_len);
oal_int32 oal_set_wakelock_debuglevel(const char *name, oal_uint32 level);

OAL_STATIC OAL_INLINE oal_void oal_wake_lock_init(oal_wakelock_stru *pst_wakelock, char *name)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    oal_ulong flags;
    memset_s((oal_void *)pst_wakelock, sizeof(oal_wakelock_stru), 0, sizeof(oal_wakelock_stru));

    wakeup_source_init(&pst_wakelock->st_wakelock, name ? name : "wake_lock_null");
    oal_spin_lock_init(&pst_wakelock->lock);
    pst_wakelock->lock_count = 0;
    pst_wakelock->locked_addr = 0;

    /* add to local list */
    oal_spin_lock_irq_save(&g_wakelock_lock, &flags);
    oal_dlist_add_tail(&pst_wakelock->list, &g_wakelock_head);
    oal_spin_unlock_irq_restore(&g_wakelock_lock, &flags);
#endif
}

OAL_STATIC OAL_INLINE oal_void oal_wake_lock_exit(oal_wakelock_stru *pst_wakelock)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    oal_ulong flags;
    oal_spin_lock_irq_save(&g_wakelock_lock, &flags);
    oal_dlist_delete_entry(&pst_wakelock->list);
    oal_spin_unlock_irq_restore(&g_wakelock_lock, &flags);
    wakeup_source_trash(&pst_wakelock->st_wakelock);
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_lock(oal_wakelock_stru *pst_wakelock)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))

    oal_ulong ul_flags;

    if (OAL_UNLIKELY(pst_wakelock == NULL)) {
        OAL_WARN_ON(1);
        return;
    }

    oal_spin_lock_irq_save(&pst_wakelock->lock, &ul_flags);
    if (!pst_wakelock->lock_count) {
        __pm_stay_awake(&pst_wakelock->st_wakelock);
        pst_wakelock->locked_addr = (uintptr_t)_RET_IP_;
    }
    pst_wakelock->lock_count++;
    if (OAL_UNLIKELY(pst_wakelock->debug)) {
        printk(KERN_DEBUG "wakelock[%s] lockcnt:%lu <==%pf\n",
               pst_wakelock->st_wakelock.name, pst_wakelock->lock_count, (oal_void *)_RET_IP_);
    }
    oal_spin_unlock_irq_restore(&pst_wakelock->lock, &ul_flags);
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_unlock(oal_wakelock_stru *pst_wakelock)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))

    oal_ulong ul_flags;

    if (OAL_UNLIKELY(pst_wakelock == NULL)) {
        OAL_WARN_ON(1);
        return;
    }

    oal_spin_lock_irq_save(&pst_wakelock->lock, &ul_flags);
    if (pst_wakelock->lock_count) {
        pst_wakelock->lock_count--;
        if (!pst_wakelock->lock_count) {
            __pm_relax(&pst_wakelock->st_wakelock);
            pst_wakelock->locked_addr = (uintptr_t)0x0;
        }

        if (OAL_UNLIKELY(pst_wakelock->debug)) {
            printk(KERN_DEBUG "wakeunlock[%s] lockcnt:%lu <==%pf\n",
                   pst_wakelock->st_wakelock.name, pst_wakelock->lock_count, (oal_void *)_RET_IP_);
        }
    }
    oal_spin_unlock_irq_restore(&pst_wakelock->lock, &ul_flags);
#endif
}

OAL_STATIC OAL_INLINE oal_int32 oal_wakelock_active(oal_wakelock_stru *pst_wakelock)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    if (OAL_UNLIKELY(pst_wakelock == NULL)) {
        OAL_WARN_ON(1);
        return 0;
    }
    return pst_wakelock->st_wakelock.active;
#else
    return 0;
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_lock_timeout(oal_wakelock_stru *pst_wakelock, oal_uint32 msec)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))

    oal_ulong ul_flags;

    oal_spin_lock_irq_save(&pst_wakelock->lock, &ul_flags);

    __pm_wakeup_event(&pst_wakelock->st_wakelock, msec);
    pst_wakelock->locked_addr = _RET_IP_;

    oal_spin_unlock_irq_restore(&pst_wakelock->lock, &ul_flags);
#endif
}

OAL_STATIC OAL_INLINE void oal_wake_unlock_force(oal_wakelock_stru *pst_wakelock)
{
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))

    oal_ulong ul_flags;

    oal_spin_lock_irq_save(&pst_wakelock->lock, &ul_flags);

    __pm_relax(&pst_wakelock->st_wakelock);
    pst_wakelock->locked_addr = (oal_ulong)0x0;
    pst_wakelock->lock_count = 0;

    oal_spin_unlock_irq_restore(&pst_wakelock->lock, &ul_flags);
#endif
}

#endif /* end of oal_schedule.h */
