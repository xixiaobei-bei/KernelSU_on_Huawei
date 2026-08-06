

/* 头文件包含 */
#include "frw_task.h"

#include "frw_main.h"
#include "frw_event_main.h"
#include "hal_ext_if.h"

#if ((_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)))
#include <linux/signal.h>
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_FRW_TASK_C

/* 全局变量定义 */
/* 事件处理全局变量 */
frw_task_stru g_event_task[WLAN_FRW_MAX_NUM_CORES];

/* 函数实现 */
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)

/*
 * 函 数 名  : frw_set_thread_property
 * 功能描述  : 设置线程参数函数
 * 输入参数  : p: 当前线程; policy: 调度策略; param:
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
 * 函 数 名  : frw_task_thread
 * 功能描述  : frw 内核线程主程序
 * 输入参数  : 核id
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

        /* state为TASK_INTERRUPTIBLE，condition不成立则线程阻塞，直到被唤醒进入waitqueue */
        /*lint -e730*/
#ifdef _PRE_FRW_EVENT_PROCESS_TRACE_DEBUG
        frw_event_last_pc_trace(__FUNCTION__, __LINE__, ul_bind_cpu);
#endif
        ret = oal_wait_event_interruptible(g_event_task[ul_bind_cpu].frw_wq,
                                           frw_task_thread_condition_check((oal_uint)ul_bind_cpu) == OAL_TRUE);
        /*lint +e730*/
        if (oal_unlikely(ret == -ERESTARTSYS)) {
            OAL_IO_PRINT("wifi task %s was interrupted by a signal\n", oal_get_current_task_name());
            break;
        }
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        g_event_task[ul_bind_cpu].ul_total_loop_cnt++;

        ul_event_count = g_event_task[ul_bind_cpu].ul_total_event_cnt;
#endif
        frw_event_process_all_event((oal_uint)ul_bind_cpu);
#if (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_THREAD)
        if (ul_event_count == g_event_task[ul_bind_cpu].ul_total_event_cnt) {
            /* 空转 */
            ul_empty_count++;
            if (ul_empty_count == ul_count_loop_time) {
                declare_dft_trace_key_info("frw_sched_too_much", OAL_DFT_TRACE_EXCEP);
            }
        } else {
            if (ul_empty_count > g_event_task[ul_bind_cpu].ul_max_empty_count) {
                g_event_task[ul_bind_cpu].ul_max_empty_count = ul_empty_count;
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
 * 函 数 名  : frw_task_init
 * 功能描述  : frw task初始化接口
 */
oal_uint32 frw_task_init(oal_void)
{
    oal_uint core_id;
    oal_task_stru *pst_task = NULL;
    struct sched_param param = {0};

    memset_s(g_event_task, OAL_SIZEOF(g_event_task), 0, OAL_SIZEOF(g_event_task));

    for (core_id = 0; core_id < WLAN_FRW_MAX_NUM_CORES; core_id++) {
        oal_wait_queue_init_head(&g_event_task[core_id].frw_wq);

        pst_task = oal_kthread_create(frw_task_thread, (oal_void *)(uintptr_t) core_id, "hisi_frw/%lu", core_id);
        if (OAL_IS_ERR_OR_NULL(pst_task)) {
            return OAL_FAIL;
        }

        oal_kthread_bind(pst_task, core_id);

        g_event_task[core_id].pst_event_kthread = pst_task;
        g_event_task[core_id].uc_task_state = FRW_TASK_STATE_IRQ_UNBIND;

        param.sched_priority = 1;
        frw_set_thread_property(pst_task, SCHED_FIFO, &param, 0);

        oal_wake_up_process(g_event_task[core_id].pst_event_kthread);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : frw_task_exit
 * 功能描述  : frw task退出函数
 */
oal_void frw_task_exit(oal_void)
{
    oal_uint32 ul_core_id;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        if (g_event_task[ul_core_id].pst_event_kthread) {
            oal_kthread_stop(g_event_task[ul_core_id].pst_event_kthread);
            g_event_task[ul_core_id].pst_event_kthread = OAL_PTR_NULL;
        }
    }
}

/*
 * 函 数 名  : frw_task_event_handler_register
 * 功能描述  : 供外部模块注册tasklet处理函数中执行的函数? */
oal_void frw_task_event_handler_register(oal_void (*p_func)(oal_uint))
{
}

/*
 * 函 数 名  : frw_task_sched
 * 功能描述  : 唤醒event时间处理线程，与wake_event_interruptible对应
 */
oal_void frw_task_sched(oal_uint32 ul_core_id)
{
    oal_wait_queue_wake_up_interrupt(&g_event_task[ul_core_id].frw_wq);
}

/*
 * 函 数 名  : frw_task_set_state
 * 功能描述  : 设置内核线程的绑定状态
 */
oal_void frw_task_set_state(oal_uint32 ul_core_id, oal_uint8 uc_task_state)
{
    g_event_task[ul_core_id].uc_task_state = uc_task_state;
}

/*
 * 函 数 名  : frw_task_get_state
 * 功能描述  : 获取内核线程的绑定状态
 */
oal_uint8 frw_task_get_state(oal_uint32 ul_core_id)
{
    return g_event_task[ul_core_id].uc_task_state;
}

#elif (_PRE_FRW_FEATURE_PROCCESS_ENTITY_TYPE == _PRE_FRW_FEATURE_PROCCESS_ENTITY_TASKLET)

// 使用tasklet进行核间通信，tasklet初始化时指定核间通信方向
#if WLAN_FRW_MAX_NUM_CORES == 1
#define frw_dst_core(this_core) 0
#elif WLAN_FRW_MAX_NUM_CORES == 2
#define frw_dst_core(this_core) (this_core == 0 ? 1 : 0)
#else
#define frw_dst_core(this_core) 0
#error enhance ipi to support more cores!
#endif

OAL_STATIC oal_void frw_task_ipi_handler(oal_uint ui_arg);

/*
 * 函 数 名  : frw_task_init
 * 功能描述  : tasklet初始化接口
 */
oal_uint32 frw_task_init(oal_void)
{
    oal_uint32 ul_core_id;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        oal_task_init(&g_event_task[ul_core_id].st_event_tasklet,
                      g_event_task[ul_core_id].p_event_handler_func, 0);
        oal_task_init(&g_event_task[ul_core_id].st_ipi_tasklet,
                      frw_task_ipi_handler, frw_dst_core(ul_core_id));
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : frw_task_exit
 * 功能描述  : task 退出函数
 */
oal_void frw_task_exit(oal_void)
{
    oal_uint32 ul_core_id;

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        oal_task_kill(&g_event_task[ul_core_id].st_event_tasklet);
        oal_task_kill(&g_event_task[ul_core_id].st_ipi_tasklet);
    }
}

/*
 * 函 数 名  : frw_task_event_handler_register
 * 功能描述  : 供外部模块注册tasklet处理函数中执行的函数
 */
oal_void frw_task_event_handler_register(oal_void (*p_func)(oal_uint))
{
    oal_uint32 ul_core_id;

    if (oal_unlikely(p_func == OAL_PTR_NULL)) {
        OAM_ERROR_LOG0(0, OAM_SF_FRW, "{frw_task_event_handler_register:: p_func is null ptr}");
        return;
    }

    for (ul_core_id = 0; ul_core_id < WLAN_FRW_MAX_NUM_CORES; ul_core_id++) {
        g_event_task[ul_core_id].p_event_handler_func = p_func;
    }
}

/*
 * 函 数 名  : frw_remote_task_receive
 * 功能描述  : 将tasklet调度执行，被IPI中断调度执行
 */
OAL_STATIC oal_void frw_remote_task_receive(void *info)
{
    oal_tasklet_stru *pst_task = (oal_tasklet_stru *)info;
    oal_task_sched(pst_task);
}

/*
 * 函 数 名  : frw_task_ipi_handler
 * 功能描述  : 使用IPI中断，调度目标core上的tasklet执行处理事件
 */
OAL_STATIC oal_void frw_task_ipi_handler(oal_uint ui_arg)
{
    oal_uint32 ul_this_id   = oal_get_core_id();
    oal_uint32 ul_remote_id = (oal_uint32)ui_arg;

    if (ul_this_id == ul_remote_id) {
        OAL_IO_PRINT("BUG:this core = remote core = %d\n", ul_this_id);
        return;
    }

    oal_smp_call_function_single(ul_remote_id, frw_remote_task_receive,
                                 &g_event_task[ul_remote_id].st_event_tasklet, 0);
}

/*
 * 函 数 名  : frw_task_sched
 * 功能描述  : task调度接口
 */
oal_void frw_task_sched(oal_uint32 ul_core_id)
{
    oal_uint32 ul_this_cpu = oal_get_core_id();

    if (oal_task_is_scheduled(&g_event_task[ul_core_id].st_event_tasklet)) {
        return;
    }

    if (ul_this_cpu == ul_core_id) {
        oal_task_sched(&g_event_task[ul_core_id].st_event_tasklet);
    } else {
        if (oal_task_is_scheduled(&g_event_task[ul_this_cpu].st_ipi_tasklet)) {
            return;
        }
        oal_task_sched(&g_event_task[ul_this_cpu].st_ipi_tasklet);
    }
}

/*
 * 函 数 名  : frw_task_set_state
 * 功能描述  : 设置tasklet的状态
 */
oal_void frw_task_set_state(oal_uint32 ul_core_id, oal_uint8 uc_task_state)
{
}

/*
 * 函 数 名  : frw_task_get_state
 * 功能描述  : 获取tasklet的状态，tasklet一直与核绑定
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
