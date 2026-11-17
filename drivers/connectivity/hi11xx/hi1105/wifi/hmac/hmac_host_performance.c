
#ifdef _PRE_HOST_PERFORMANCE
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"
#include "hmac_host_performance.h"
#include "wal_linux_ioctl.h"


#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_HOST_TIME_C

ktime_t g_host_performance_start[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS];
int64_t g_host_performance_end[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS] = {{0}};
ftrace_thread g_host_performance[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS];

uint32_t tx_bh_cnt[HMAC_START_PERFORMANCE_BUTT][CONFIG_NR_CPUS] = {{0}};

void ns_start(ktime_t *start)
{
    *start = ktime_get();
}

void ns_calc(int64_t *ave, ktime_t start_time, uint64_t div, ftrace_thread *frace)
{
    ktime_t time_cost;
    int64_t time_cost_ns;
    ktime_t end_time;

    end_time = ktime_get();
    /* 翻转不计数 */
    if (end_time < start_time) {
        oam_error_log3(0, 0, "ns_ave: start[%d] end[%d] div[%d]", start_time, end_time, div);
        return;
    }
    time_cost = ktime_sub(end_time, start_time);

    frace->total_count += div;
    frace->total_cost = ktime_add(frace->total_cost, time_cost);

    do_div(time_cost, div);
    time_cost_ns = ktime_to_ns(time_cost);
    *ave = time_cost_ns;
}

void instant_host_time()
{
    uint32_t i;
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        oam_error_log3(0, 0, "instant host_time:cpuid[%d]xmit[%d]tx_ring[%d]", i,
            g_host_performance_end[TX_XMIT_PROC][i], g_host_performance_end[TX_RING_PROC][i]);
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        oam_warning_log3(0, 0, "instant host_time:cpuid[%d] isr[%d] bh[%d]", i,
            g_host_performance_end[TX_ISR_PROC][i], g_host_performance_end[TX_BH_PROC][i]);
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        oam_warning_log3(0, 0, "instant host_time:cpuid[%d] rxisr[%d] rxbh[%d]", i,
            g_host_performance_end[RX_ISR_PROC][i], g_host_performance_end[RX_RING_PROC][i]);
    }
}
void percost_host_time()
{
    uint32_t i;
    unsigned long per_cost;

    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        per_cost = calc_host_time_per_cost(g_host_performance[TX_XMIT_PROC][i].total_cost,
            g_host_performance[TX_XMIT_PROC][i].total_count);
        oam_error_log4(0, 0, "tx_xmit :: cpuid[%d] total_cost(ns)[%d] total_count[%d]percost(ns)[%d]",
            i, ktime_to_ns(g_host_performance[TX_XMIT_PROC][i].total_cost),
            g_host_performance[TX_XMIT_PROC][i].total_count, ktime_to_ns(per_cost));
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        per_cost = calc_host_time_per_cost(g_host_performance[TX_RING_PROC][i].total_cost,
            g_host_performance[TX_RING_PROC][i].total_count);
        oam_warning_log4(0, 0, "tx_ring :: cpuid[%d] total_cost(ns)[%d] total_count[%d]percost(ns)[%d]",
            i, ktime_to_ns(g_host_performance[TX_RING_PROC][i].total_cost),
            g_host_performance[TX_RING_PROC][i].total_count, ktime_to_ns(per_cost));
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        per_cost = calc_host_time_per_cost(g_host_performance[TX_ISR_PROC][i].total_cost,
            g_host_performance[TX_ISR_PROC][i].total_count);
        oam_error_log4(0, 0, "tx_isr :: cpuid[%d] total_cost(ns)[%d] total_count[%d]percost(ns)[%d]",
            i, ktime_to_ns(g_host_performance[TX_ISR_PROC][i].total_cost),
            g_host_performance[TX_ISR_PROC][i].total_count, ktime_to_ns(per_cost));
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        per_cost = calc_host_time_per_cost(g_host_performance[TX_BH_PROC][i].total_cost,
            g_host_performance[TX_BH_PROC][i].total_count);
        oam_warning_log4(0, 0, "tx_bh :: cpuid[%d] total_cost(ns)[%d] total_count[%d]percost(ns)[%d]",
            i, ktime_to_ns(g_host_performance[TX_BH_PROC][i].total_cost),
            g_host_performance[TX_BH_PROC][i].total_count, ktime_to_ns(per_cost));
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        per_cost = calc_host_time_per_cost(g_host_performance[RX_ISR_PROC][i].total_cost,
            g_host_performance[RX_ISR_PROC][i].total_count);
        oam_error_log4(0, 0, "rx_isr :: cpuid[%d] total_cost(ns)[%d] total_count[%d]percost(ns)[%d]",
            i, ktime_to_ns(g_host_performance[RX_ISR_PROC][i].total_cost),
            g_host_performance[RX_ISR_PROC][i].total_count, ktime_to_ns(per_cost));
    }
    for (i = 0; i < CONFIG_NR_CPUS; i++) {
        per_cost = calc_host_time_per_cost(g_host_performance[RX_RING_PROC][i].total_cost,
            g_host_performance[RX_RING_PROC][i].total_count);
        oam_warning_log4(0, 0, "rx_bh :: cpuid[%d] total_cost(ns)[%d] total_count[%d]percost(ns)[%d]",
            i, ktime_to_ns(g_host_performance[RX_RING_PROC][i].total_cost),
            g_host_performance[RX_RING_PROC][i].total_count, ktime_to_ns(per_cost));
    }
}

uint32_t wal_hipriv_show_host_time(oal_net_device_stru *net_dev, int8_t *pc_param)
{
    uint32_t off_set = 0;
    uint32_t result;
    int8_t value[WAL_HIPRIV_CMD_VALUE_MAX_LEN];
    uint8_t flag;

    /* 获取常发模式开关标志 */
    result = wal_get_cmd_one_arg(pc_param, value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (result != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_show_host_time::get cmd one arg err[%d]!}", result);
        return result;
    }
    flag = (uint8_t)oal_atoi(value);
    if (flag > ENABLE) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_show_host_time::input should be 0 or 1.}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* 关闭的情况下不需要解析后面的参数 */

    if (flag == ENABLE) {
        instant_host_time();
        percost_host_time();
        return OAL_SUCC;
    } else if (flag == DISABLE) {
        host_time_init();
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_show_host_time::clear[%d].}");
    }
    return OAL_SUCC;
}


#endif

