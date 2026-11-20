
#ifndef __HMAC_HOST_TIME_H__
#define __HMAC_HOST_TIME_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HOST_TIME_H
typedef enum {
    TX_XMIT_PROC = 0,
    TX_RING_PROC,
    TX_ISR_PROC,
    TX_BH_PROC,
    RX_ISR_PROC,
    RX_RING_PROC,
    HMAC_START_PERFORMANCE_BUTT
} hmac_performance_proc_pos;
typedef uint8_t host_performance_proc_enum_uint8;

#ifdef _PRE_HOST_PERFORMANCE
typedef struct ftrace_thread {
    uint32_t total_count;
    ktime_t total_cost;
} ftrace_thread;

extern ktime_t g_host_performance_start[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS];
extern int64_t g_host_performance_end[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS];

extern uint32_t tx_bh_cnt[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS];
extern ftrace_thread g_host_performance[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS];

void ns_start(int64_t *start);
void ns_calc(int64_t *ave, ktime_t start_time, uint64_t div, ftrace_thread *frace);
uint32_t wal_hipriv_show_host_time(oal_net_device_stru *net_dev, int8_t *pc_param);

OAL_STATIC OAL_INLINE void host_time_init()
{
    memset_s(&g_host_performance[0][0],
        sizeof(ftrace_thread) * HMAC_START_PERFORMANCE_BUTT * CONFIG_NR_CPUS, 0,
        sizeof(ftrace_thread) * HMAC_START_PERFORMANCE_BUTT * CONFIG_NR_CPUS);
}

OAL_STATIC OAL_INLINE unsigned long calc_host_time_per_cost(ktime_t total_cost, uint32_t total_count)
{
    unsigned long per_cost = total_cost;
    if (likely(total_count != 0)) {
        do_div(per_cost, total_count);
    } else {
        per_cost = 0;
    }
    return per_cost;
}

OAL_STATIC OAL_INLINE void host_time_start_record(host_performance_proc_enum_uint8 proc)
{
    ns_start(&g_host_performance_start[proc][smp_processor_id()]);
}

OAL_STATIC OAL_INLINE void host_time_calc_end(uint32_t cnt, host_performance_proc_enum_uint8 proc)
{
    uint32_t cpuid = smp_processor_id();
    if (cnt) {
        ns_calc(&g_host_performance_end[proc][cpuid], g_host_performance_start[proc][cpuid],
            cnt, &g_host_performance[proc][cpuid]);
    }
}

OAL_STATIC OAL_INLINE void host_time_cnt_inc(host_performance_proc_enum_uint8 proc)
{
    tx_bh_cnt[proc][smp_processor_id()]++;
}

OAL_STATIC OAL_INLINE uint32_t host_time_cnt_get(host_performance_proc_enum_uint8 proc)
{
    return tx_bh_cnt[proc][smp_processor_id()];
}
OAL_STATIC OAL_INLINE void host_time_cnt_init(host_performance_proc_enum_uint8 proc)
{
    tx_bh_cnt[proc][smp_processor_id()] = 0;
}

#define host_start_record_performance(a) host_time_start_record(a)
#define host_end_record_performance(a, b) host_time_calc_end(a, b)
#define host_cnt_init_record_performance(a) host_time_cnt_init(a)
#define host_cnt_get_record_performance(a) host_time_cnt_get(a)
#define host_cnt_inc_record_performance(a) host_time_cnt_inc(a)

#else

#define host_start_record_performance(a)
#define host_end_record_performance(a, b)
#define host_cnt_init_record_performance(a)
#define host_cnt_get_record_performance(a)
#define host_cnt_inc_record_performance(a)


#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
