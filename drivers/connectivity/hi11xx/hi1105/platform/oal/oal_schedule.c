

/* 头文件包含 */
#include "oal_schedule.h"
#include "securec.h"

/* 全局变量定义 */
#if ((KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))

oal_dlist_create_head(g_wakelock_head);
oal_define_spinlock(g_wakelock_lock);
/*lint -e19*/
oal_module_symbol(g_wakelock_head);
oal_module_symbol(g_wakelock_lock);
/*lint +e19*/
#endif

#ifdef _PRE_OAL_FEATURE_TASK_NEST_LOCK
void _oal_smp_task_lock_(oal_task_lock_stru *pst_lock, uintptr_t claim_addr)
{
#ifdef _PRE_WINDOWS_SUPPORT
    return;
#else
    DECLARE_WAITQUEUE(wait, current);

    unsigned long flags;

    if (oal_warn_on(in_interrupt() || in_atomic())) {
        declare_dft_trace_key_info("smp_task_lock_sched_warn", OAL_DFT_TRACE_EXCEP);
        return;
    }

    add_wait_queue(&pst_lock->wq, &wait);
    oal_spin_lock_irq_save(&pst_lock->lock, &flags);
    while (1) {
        /*lint -e666*/
        set_current_state(TASK_UNINTERRUPTIBLE);
        /*lint +e666*/
        if (!pst_lock->claimed || pst_lock->claimer == current) {
            break;
        }
        oal_spin_unlock_irq_restore(&pst_lock->lock, &flags);
        schedule();
        oal_spin_lock_irq_save(&pst_lock->lock, &flags);
    }
    /*lint -e666*/
    set_current_state(TASK_RUNNING);
    /*lint +e666*/

    pst_lock->claimed = 1;
    pst_lock->claimer = current;
    pst_lock->claim_cnt++;

    oal_spin_unlock_irq_restore(&pst_lock->lock, &flags);
    remove_wait_queue(&pst_lock->wq, &wait);
#endif
}
oal_module_symbol(_oal_smp_task_lock_);
#endif

int32_t oal_print_all_wakelock_buff(char *buf, int32_t buf_len)
{
    int32_t ret;
#if ((KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    oal_dlist_head_stru *pst_entry;
    oal_dlist_head_stru *pst_entry_temp;
    int32_t count = 0;
    if (buf_len < 1) {
        return 0;
    }
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
                     "lockname     lockcnt  active  debug  lockuser \n");
    if (ret < 0) {
        oal_io_print("log str format err line[%d]\n", __LINE__);
        return count;
    }
    count += ret;
    oal_dlist_search_for_each_safe(pst_entry, pst_entry_temp, &g_wakelock_head)
    {
        oal_wakelock_stru *pst_wakelock = (oal_wakelock_stru *)oal_dlist_get_entry(pst_entry, oal_wakelock_stru, list);
        if (buf_len > ret) {
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "%s     %lu  %d  %s %pf\n",
                             pst_wakelock->st_wakelock.name,
                             pst_wakelock->lock_count,
                             oal_wakelock_active(pst_wakelock),
                             pst_wakelock->debug ? "on " : "off",
                             (void *)pst_wakelock->locked_addr);
            if (ret < 0) {
                oal_io_print("log str format err line[%d]\n", __LINE__);
                return count;
            }
            count += ret;
        }
    }
    return count;
#else
    oal_reference(ret);
    oal_reference(buf);
    oal_reference(buf_len);
#endif
}

void oal_print_all_wakelock_info(void)
{
#if ((KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    void *pst_mem = oal_memalloc(PAGE_SIZE);
    if (pst_mem == NULL) {
        oal_io_print("mem alloc failed %u\n", (uint32_t)PAGE_SIZE);
        return;
    }

    oal_print_all_wakelock_buff((char *)pst_mem, PAGE_SIZE);
    oal_io_print("%s", (char *)pst_mem);
    oal_free(pst_mem);
#endif
}

int32_t oal_set_wakelock_debuglevel(const char *name, uint32_t level)
{
#if ((KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))

    oal_dlist_head_stru *pst_entry;
    oal_dlist_head_stru *pst_entry_temp;
    oal_io_print("lockname     lockcnt  debug  lockuser");
    oal_dlist_search_for_each_safe(pst_entry, pst_entry_temp, &g_wakelock_head)
    {
        oal_wakelock_stru *pst_wakelock = (oal_wakelock_stru *)oal_dlist_get_entry(pst_entry, oal_wakelock_stru, list);

        if (!oal_strcmp(name, pst_wakelock->st_wakelock.name)) {
            oal_io_print("set wakelock %s debuglevel from %u to %u\n",
                         pst_wakelock->st_wakelock.name, pst_wakelock->debug, level);
            pst_wakelock->debug = level;
            return OAL_SUCC;
        }
    }
#else
    oal_reference(name);
    oal_reference(level);
#endif
    return -OAL_ENODEV;
}
