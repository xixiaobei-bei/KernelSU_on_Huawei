

/*****************************************************************************
  1 Í·ÎÄ¼þ°üº¬
*****************************************************************************/
#include "frw_task.h"

#include "platform_spec.h"
#include "frw_main.h"
#include "frw_event_main.h"
#include "hal_ext_if.h"

#if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)))
#include <linux/signal.h>
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TASK_C

/*****************************************************************************
  2 È«¾Ö±äÁ¿¶¨Òå
*****************************************************************************/
/******************************************************************************
    ÊÂ¼þ´¦ÀíÈ«¾Ö±äÁ¿
*******************************************************************************/
frw_task_stru event_task[WLAN_FRW_MAX_NUM_CORES];

/*****************************************************************************
  3 º¯ÊýÊµÏÖ
*****************************************************************************/

#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)

/*
 * º¯ Êý Ãû  : frw_set_thread_property
 * ¹¦ÄÜÃèÊö  : ÉèÖÃÏß³Ì²ÎÊýº¯Êý
 * ÊäÈë²ÎÊý  : p: µ±Ç°Ïß³Ì; policy: µ÷¶È²ßÂÔ; param:
 * Êä³ö²ÎÊý  : ÎÞ
 * ·µ »Ø Öµ  : ÎÞ
 */
OAL_STATIC void frw_set_thread_property(oal_task_stru *p, int policy, struct sched_param *param, long nice)
{
    if (p == NULL) {
        OAL_IO_PRINT("oal_task_stru p is null r failed!%s\n", __FUNCTION__);
        return;
    }

    if (param == NULL) {
        OAL_IO_PRINT("param is null r failed!%s\n", __FUNCTION__);
        return;
    }

    if (oal_sched_setscheduler(p, policy, param)) {
        OAL_IO_PRINT("[Error]set scheduler failed! %d\n", policy);
    }

    if (policy != SCHED_FIFO && policy != SCHED_RR) {
        OAL_IO_PRINT("set thread scheduler nice %ld\n", nice);
        oal_set_user_nice(p, nice);
    }
}

/*
 * º¯ Êý Ãû  : frw_task_thread
 * ¹¦ÄÜÃèÊö  : frw ÄÚºËÏß³ÌÖ÷³ÌÐò
 * ÊäÈë²ÎÊý  : ºËid
 * Êä³ö²ÎÊý  : ÎÞ
 * ·µ »Ø Öµ  : ÎÞ
 */
OAL_STATIC oal_int32 frw_task_thread(oal_void *arg)
{
    oal_uint32 ul_bind_cpu = (oal_uint32)(uintptr_t)arg;
    oal_int32 ret = 0;
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
    oal_uint32 ul_empty_count = 0;
    const oal_uint32 ul_count_loop_time = 10000;
#endif

    allow_signal(SIGTERM);

    for (;;) {
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        oal_uint32 ul_event_count;
#endif
        if (oal_kthread_should_stop()) {
            break;
        }

        /* stateÎªTASK_INTERRUPTIBLE£¬condition²»³ÉÁ¢ÔòÏß³Ì×èÈû£¬Ö±µ½±»»½ÐÑ½øÈëwaitqueue */
        /*lint -e730*/
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        frw_event_last_pc_trace(__FUNCTION__, __LINE__, ul_bind_cpu);
#endif
        ret = OAL_WAIT_EVENT_INTERRUPTIBLE(event_task[ul_bind_cpu].frw_wq,
                                           OAL_TRUE == frw_task_thread_condition_check((oal_uint)ul_bind_cpu));
        /*lint +e730*/
        if (OAL_UNLIKELY(ret == -ERESTARTSYS)) {
            OAL_IO_PRINT("wifi task %s was interrupted by a signal\n", oal_get_current_task_name());
            break;
        }
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        event_task[ul_bind_cpu].ul_total_loop_cnt++;

        ul_event_count = event_task[ul_bind_cpu].ul_total_event_cnt;
#endif
        frw_event_process_all_event((oal_uint)ul_bind_cpu);
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        if (ul_event_count == event_task[ul_bind_cpu].ul_total_event_cnt) {
            /* ¿Õ×ª */
            ul_empty_count++;
            if (ul_empty_count == ul_count_loop_time) {
                DECLARE_DFT_TRACE_KEY_INFO("frw_sched_too_much", OAL_DFT_TRACE_EXCEP);
            }
        } else {
            if (ul_empty_count > event_task[ul_bind_cpu].ul_max_empty_count) {
                event_task[ul_bind_cpu].ul_max_empty_count = ul_empty_count;
            }
            ul_empty_count = 0;
        }
#endif
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        frw_event_last_pc_trace(__FUNCTION__, __LINE__, ul_bind_cpu);
#endif
    }

    return 0;
}

/*
 * º¯ Êý Ãû  : frw_task_init
 * ¹¦ÄÜÃèÊö  : frw task³õÊ¼»¯½Ó¿Ú
 */
oal_uint32 frw_task_init(oal_void)
{
    oal_uint ul_core_id;
    oal_task_stru *pst_task;
    struct sched_param param = {0};

    memset_s(event_task, OAL_SIZEOF(event_task), 0, OAL_SIZEOF(event_task));

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        OAL_WAIT_QUEUE_INIT_HEAD(&event_task[ul_core_id].frw_wq);

        pst_task = oal_kthread_create(frw_task_thread, (oal_void *)(uintptr_t) ul_core_id, "hisi_frw/%lu", ul_core_id);
        if (OAL_IS_ERR_OR_NULL(pst_task)) {
            return OAL_FAIL;
        }

        oal_kthread_bind(pst_task, ul_core_id);

        event_task[ul_core_id].pst_event_kthread = pst_task;
        event_task[ul_core_id].uc_task_state = FRW_TASK_STATE_IRQ_UNBIND;

        param.sched_priority = 1;
        frw_set_thread_property(pst_task, SCHED_FIFO, &param, 0);

        oal_wake_up_process(event_task[ul_core_id].pst_event_kthread);
    }

    return OAL_SUCC;
}

/*
 * º¯ Êý Ãû  : frw_task_exit
 * ¹¦ÄÜÃèÊö  : frw taskÍË³öº¯Êý
 */
oal_void frw_task_exit(oal_void)
{
    oal_uint32 ul_core_id;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        if (event_task[ul_core_id].pst_event_kthread) {
            oal_kthread_stop(event_task[ul_core_id].pst_event_kthread);
            event_task[ul_core_id].pst_event_kthread = OAL_PTR_NULL;
        }
    }
}

/*
 * º¯ Êý Ãû  : frw_task_event_handler_register
 * ¹¦ÄÜÃèÊö  : ¹©Íâ²¿Ä£¿é×¢²átasklet´¦Àíº¯ÊýÖÐÖ´ÐÐµÄº¯ÊýÞ
 */
oal_void frw_task_event_handler_register(oal_void (*p_func)(oal_uint))
{
}

/*
 * º¯ Êý Ãû  : frw_task_sched
 * ¹¦ÄÜÃèÊö  : »½ÐÑeventÊ±¼ä´¦ÀíÏß³Ì£¬Óëwake_event_interruptible¶ÔÓ¦
 */
oal_void frw_task_sched(oal_uint32 ul_core_id)
{
    OAL_WAIT_QUEUE_WAKE_UP_INTERRUPT(&event_task[ul_core_id].frw_wq);
}

/*
 * º¯ Êý Ãû  : frw_task_set_state
 * ¹¦ÄÜÃèÊö  : ÉèÖÃÄÚºËÏß³ÌµÄ°ó¶¨×´Ì¬
 */
oal_void frw_task_set_state(oal_uint32 ul_core_id, oal_uint8 uc_task_state)
{
    event_task[ul_core_id].uc_task_state = uc_task_state;
}

/*
 * º¯ Êý Ãû  : frw_task_get_state
 * ¹¦ÄÜÃèÊö  : »ñÈ¡ÄÚºËÏß³ÌµÄ°ó¶¨×´Ì¬
 */
oal_uint8 frw_task_get_state(oal_uint32 ul_core_id)
{
    return event_task[ul_core_id].uc_task_state;
}

#elif (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_TASKLET)

// Ê¹ÓÃtasklet½øÐÐºË¼äÍ¨ÐÅ£¬tasklet³õÊ¼»¯Ê±Ö¸¶¨ºË¼äÍ¨ÐÅ·½Ïò
#if WLAN_FRW_MAX_NUM_CORES == 1
#define FRW_DST_CORE(this_core) 0
#elif WLAN_FRW_MAX_NUM_CORES == 2
#define FRW_DST_CORE(this_core) (this_core == 0 ? 1 : 0)
#else
#define FRW_DST_CORE(this_core) 0
#error enhance ipi to support more cores!
#endif

OAL_STATIC oal_void frw_task_ipi_handler(oal_uint ui_arg);

/*
 * º¯ Êý Ãû  : frw_task_init
 * ¹¦ÄÜÃèÊö  : tasklet³õÊ¼»¯½Ó¿Ú
 */
oal_uint32 frw_task_init(oal_void)
{
    oal_uint32 ul_core_id;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        oal_task_init(&event_task[ul_core_id].st_event_tasklet,
                      event_task[ul_core_id].p_event_handler_func, 0);
        oal_task_init(&event_task[ul_core_id].st_ipi_tasklet,
                      frw_task_ipi_handler, FRW_DST_CORE(ul_core_id));
    }
    return OAL_SUCC;
}

/*
 * º¯ Êý Ãû  : frw_task_exit
 * ¹¦ÄÜÃèÊö  : task ÍË³öº¯Êý
 */
oal_void frw_task_exit(oal_void)
{
    oal_uint32 ul_core_id;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        oal_task_kill(&event_task[ul_core_id].st_event_tasklet);
        oal_task_kill(&event_task[ul_core_id].st_ipi_tasklet);
    }
}

/*
 * º¯ Êý Ãû  : frw_task_event_handler_register
 * ¹¦ÄÜÃèÊö  : ¹©Íâ²¿Ä£¿é×¢²átasklet´¦Àíº¯ÊýÖÐÖ´ÐÐµÄº¯Êý
 */
oal_void frw_task_event_handler_register(oal_void (*p_func)(oal_uint))
{
    oal_uint32 ul_core_id;

    if (OAL_UNLIKELY(OAL_PTR_NULL == p_func)) {
        OAM_ERROR_LOG0(0, OAM_SF_FRW, "{frw_task_event_handler_register:: p_func is null ptr}");
        return;
    }

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        event_task[ul_core_id].p_event_handler_func = p_func;
    }
}

/*
 * º¯ Êý Ãû  : frw_remote_task_receive
 * ¹¦ÄÜÃèÊö  : ½«taskletµ÷¶ÈÖ´ÐÐ£¬±»IPIÖÐ¶Ïµ÷¶ÈÖ´ÐÐ
 */
OAL_STATIC oal_void frw_remote_task_receive(void *info)
{
    oal_tasklet_stru *pst_task = (oal_tasklet_stru *)info;
    oal_task_sched(pst_task);
}

/*
 * º¯ Êý Ãû  : frw_task_ipi_handler
 * ¹¦ÄÜÃèÊö  : Ê¹ÓÃIPIÖÐ¶Ï£¬µ÷¶ÈÄ¿±êcoreÉÏµÄtaskletÖ´ÐÐ´¦ÀíÊÂ¼þ
 */
OAL_STATIC oal_void frw_task_ipi_handler(oal_uint ui_arg)
{
    oal_uint32 ul_this_id   = OAL_GET_CORE_ID();
    oal_uint32 ul_remote_id = (oal_uint32)ui_arg;

    if (ul_this_id == ul_remote_id) {
        OAL_IO_PRINT("BUG:this core = remote core = %d\n", ul_this_id);
        return;
    }

    oal_smp_call_function_single(ul_remote_id, frw_remote_task_receive,
                                 &event_task[ul_remote_id].st_event_tasklet, 0);
}

/*
 * º¯ Êý Ãû  : frw_task_sched
 * ¹¦ÄÜÃèÊö  : taskµ÷¶È½Ó¿Ú
 */
oal_void frw_task_sched(oal_uint32 ul_core_id)
{
    oal_uint32 ul_this_cpu = OAL_GET_CORE_ID();

    if (oal_task_is_scheduled(&event_task[ul_core_id].st_event_tasklet)) {
        return;
    }

    if (ul_this_cpu == ul_core_id) {
        oal_task_sched(&event_task[ul_core_id].st_event_tasklet);
    } else {
        if (oal_task_is_scheduled(&event_task[ul_this_cpu].st_ipi_tasklet)) {
            return;
        }
        oal_task_sched(&event_task[ul_this_cpu].st_ipi_tasklet);
    }
}

/*
 * º¯ Êý Ãû  : frw_task_set_state
 * ¹¦ÄÜÃèÊö  : ÉèÖÃtaskletµÄ×´Ì¬
 */
oal_void frw_task_set_state(oal_uint32 ul_core_id, oal_uint8 uc_task_state)
{
}

/*
 * º¯ Êý Ãû  : frw_task_get_state
 * ¹¦ÄÜÃèÊö  : »ñÈ¡taskletµÄ×´Ì¬£¬taskletÒ»Ö±ÓëºË°ó¶¨
 */
oal_uint8 frw_task_get_state(oal_uint32 ul_core_id)
{
    return FRW_TASK_STATE_IRQ_BIND;
}

#endif

#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
oal_module_symbol(frw_task_set_state);
oal_module_symbol(frw_task_get_state);
#endif


