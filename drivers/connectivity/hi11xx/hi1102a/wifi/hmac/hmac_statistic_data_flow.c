


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include  "oal_net.h"
#include  "oal_types.h"
#include  "oam_ext_if.h"
#include  "oal_schedule.h"
#include  "mac_vap.h"
#include  "mac_resource.h"
#include  "hmac_vap.h"
#include  "hmac_statistic_data_flow.h"
#include  "hmac_ext_if.h"
#include  "hmac_blockack.h"
#include  "hmac_tx_data.h"
#include  "hmac_config.h"
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/pm_qos.h>
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
#include "hmac_auto_ddr_freq.h"
#endif
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#include <linux/hisi/hisi_cpufreq_req.h>
#include <linux/cpufreq.h>
#include <linux/hisi/hisi_core_ctl.h>
#endif
#endif

#include "hmac_dscr_th_opt.h"
#include "hmac_tcp_ack_filter.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_STATISTIC_DATA_FLOW_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
OAL_STATIC wifi_txrx_pkt_statis_stru g_st_wifi_rxtx_statis = {0};
extern hmac_rxdata_thread_stru     g_st_rxdata_thread;

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
extern struct cpufreq_req g_ast_cpufreq[OAL_BUS_MAXCPU_NUM];
extern struct pm_qos_request g_st_pmqos_requset;
extern oal_uint32 g_aul_cpumaxfreq[OAL_BUS_MAXCPU_NUM];
#endif

#define STA_OPEN_TXOPLIMIT_THROUGHTPUT    280
#define STA_CLOSE_TXOPLIMIT_THROUGHTPUT   260
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ

host_speed_freq_level_stru g_host_speed_freq_level[] = {
    /*pps门限                   CPU主频下限                     DDR主频下限*/
    {PPS_VALUE_0,          CPU_MIN_FREQ_VALUE_0,            DDR_MIN_FREQ_VALUE_0},
    {PPS_VALUE_1,          CPU_MIN_FREQ_VALUE_1,            DDR_MIN_FREQ_VALUE_1},
    {PPS_VALUE_2,          CPU_MIN_FREQ_VALUE_2,            DDR_MIN_FREQ_VALUE_2},
    {PPS_VALUE_3,          CPU_MIN_FREQ_VALUE_3,            DDR_MIN_FREQ_VALUE_3},
};
device_speed_freq_level_stru g_device_speed_freq_level[] = {
    /*device主频类型*/
    {FREQ_IDLE},
    {FREQ_MIDIUM},
    {FREQ_HIGHEST},
    {FREQ_HIGHEST},
};
thread_bindcpu_stru g_st_thread_bindcpu = {
    WLAN_BUSY_CPU_PPS_THROUGHT,
    WLAN_IDLE_CPU_PPS_THROUGHT,
    WLAN_BUSY_CPU_BTCOEX_PPS_THROUGHT,
    WLAN_IDLE_CPU_BTCOEX_PPS_THROUGHT,
    OAL_FALSE,
    OAL_TRUE,
    WLAN_BINDCPU_DEFAULT_MASK,
    OAL_FALSE};
#endif

/*****************************************************************************
  3 函数实现
*****************************************************************************/
oal_void hmac_wifi_statistic_rx_tcp_ack(oal_uint32 ul_small_pkt_cnt, oal_uint32 ul_large_pkt_cnt)
{
    g_st_wifi_rxtx_statis.ul_rx_pkts += ul_small_pkt_cnt + ul_large_pkt_cnt;
    g_st_wifi_rxtx_statis.ul_rx_small_pkt += ul_small_pkt_cnt;
    g_st_wifi_rxtx_statis.ul_rx_large_pkt += ul_large_pkt_cnt;
}


oal_void hmac_wifi_statistic_rx_packets(oal_uint32 ul_pkt_bytes)
{
    g_st_wifi_rxtx_statis.ul_rx_pkts++;
    g_st_wifi_rxtx_statis.ul_rx_bytes += ul_pkt_bytes;

    if (ul_pkt_bytes <= WLAN_PKT_MEM_OPT_MIN_PKT_LEN)
    {
        g_st_wifi_rxtx_statis.ul_rx_small_pkt++;
    }
    else
    {
        g_st_wifi_rxtx_statis.ul_rx_large_pkt++;
    }
}


oal_void hmac_wifi_statistic_tx_packets(oal_uint32 ul_pkt_bytes)
{
    g_st_wifi_rxtx_statis.ul_tx_pkts++;
    g_st_wifi_rxtx_statis.ul_tx_bytes += ul_pkt_bytes;

    if (ul_pkt_bytes <= WLAN_PKT_MEM_OPT_MIN_PKT_LEN)
    {
        g_st_wifi_rxtx_statis.ul_tx_small_pkt++;
    }
    else
    {
        g_st_wifi_rxtx_statis.ul_tx_large_pkt++;
    }
}
oal_bool_enum_uint8 hmac_wifi_rx_is_busy(oal_void)
{
    return g_st_wifi_rxtx_statis.en_wifi_rx_busy;
}


oal_bool_enum_uint8 hmac_wifi_alg_txop_opt(oal_uint32 ul_tx_throughput_mbps)
{
    oal_uint                 us_len = OAL_SIZEOF(mac_ioctl_alg_param_stru);
    oal_uint32               ul_ret;
    mac_ioctl_alg_param_stru st_alg_param;
    oal_bool_enum_uint8      en_txop_limit;
    mac_device_stru         *pst_device;
    hmac_device_stru        *pst_hmac_device;
    mac_vap_stru            *pst_mac_vap = OAL_PTR_NULL;

    st_alg_param.en_alg_cfg = MAC_ALG_CFG_TXOP_LIMIT_STA_EN;
    st_alg_param.ul_value   = 0;

    pst_device = mac_res_get_dev(0);
    pst_hmac_device = hmac_res_get_mac_dev(0);

    if (OAL_PTR_NULL == pst_hmac_device)
    {
        return OAL_FALSE;
    }

    /* UP的VAP，不依赖于STA P2P创建的顺序 */
    ul_ret = mac_device_find_up_vap(pst_device, &pst_mac_vap);

    en_txop_limit = pst_hmac_device->en_txop_limit;

    if (pst_mac_vap != OAL_PTR_NULL)
    {
        if (ul_tx_throughput_mbps > STA_OPEN_TXOPLIMIT_THROUGHTPUT)
        {
            st_alg_param.ul_value = 1;
            en_txop_limit = OAL_TRUE;

        }
        else if (ul_tx_throughput_mbps < STA_CLOSE_TXOPLIMIT_THROUGHTPUT)
        {
            st_alg_param.ul_value = 0;
            en_txop_limit = OAL_FALSE;
        }

        if (en_txop_limit != pst_hmac_device->en_txop_limit)
        {
            pst_hmac_device->en_txop_limit = en_txop_limit;

            /* 下发关闭动态调节txop事件 */
            hmac_config_alg_send_event(pst_mac_vap, WLAN_CFGID_ALG_PARAM, us_len, (oal_uint8 *) &st_alg_param);
            OAM_WARNING_LOG2(pst_mac_vap->uc_vap_id, OAM_SF_ANY, "hmac_wifi_alg_txop_opt: txop opt open[%d],tx[%d]Mbps", st_alg_param.ul_value,ul_tx_throughput_mbps);
        }
    }

    return OAL_TRUE;
}

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)

OAL_STATIC oal_void hmac_thread_set_allowed_cpu(struct cpumask *pst_cpus_mask)
{
#if !defined(WIN32)
    set_cpus_allowed_ptr(g_st_rxdata_thread.pst_rxdata_thread, pst_cpus_mask);
    set_cpus_allowed_ptr(hcc_get_110x_handler()->hcc_transer_info.hcc_transfer_thread, pst_cpus_mask);
    set_cpus_allowed_ptr(hcc_get_110x_handler()->bus_dev->cur_bus->pst_rx_tsk, pst_cpus_mask);
#endif
}


OAL_STATIC oal_void hmac_thread_bind_fast_cpu(oal_void)
{
#if defined(CONFIG_ARCH_HISI)
#ifdef CONFIG_NR_CPUS
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    oal_uint8 uc_cpumask;
    oal_uint8 uc_cpuid = 0;
    struct cpumask cpus_mask;

    uc_cpumask = g_st_thread_bindcpu.uc_cpumask;
    cpumask_clear(&cpus_mask);

    while (uc_cpumask)
    {
        if (uc_cpumask & 0x1)
        {
            cpumask_set_cpu(uc_cpuid, &cpus_mask);
        }

        uc_cpuid++;
        uc_cpumask = uc_cpumask >> 1;
    }

    hmac_thread_set_allowed_cpu(&cpus_mask);
#endif
#endif
#endif

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* 所有核都锁定最高频率 */
    for (uc_cpuid = 0; uc_cpuid< OAL_BUS_MAXCPU_NUM; uc_cpuid++)
    {
        hisi_cpufreq_update_req(&g_ast_cpufreq[uc_cpuid], g_aul_cpumaxfreq[uc_cpuid]);
        OAM_WARNING_LOG2(0, OAM_SF_ANY, "hmac_thread_bind_fast_cpu:lock cpu freq cpu[%d]->%d",uc_cpuid,g_aul_cpumaxfreq[uc_cpuid]);
    }
#endif
#endif
}


OAL_STATIC oal_void hmac_thread_bind_slow_cpu(oal_void)
{
#if defined(CONFIG_ARCH_HISI)
#ifdef CONFIG_NR_CPUS
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    struct cpumask cpus_mask;
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
    oal_uint8 uc_cpuid = 0;
#endif
    hisi_get_slow_cpus(&cpus_mask);
    cpumask_clear_cpu(0, &cpus_mask);

    hmac_thread_set_allowed_cpu(&cpus_mask);
#endif
#endif
#endif

#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    for (uc_cpuid = 0; uc_cpuid< OAL_BUS_MAXCPU_NUM; uc_cpuid++)
    {
        hisi_cpufreq_update_req(&g_ast_cpufreq[uc_cpuid], 0);
    }
    OAM_WARNING_LOG0(0, OAM_SF_ANY, "hmac_thread_bind_fast_cpu:clear cpu lockfreq request");
#endif
#endif
}

oal_void hmac_thread_bindcpu(oal_uint32 ul_total_pps)
{
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
    oal_uint8            en_irq_cpu_state;
    oal_uint16           us_throughput_irq_pps_high;
    oal_uint16           us_throughput_irq_pps_low;

    if (!g_st_thread_bindcpu.en_irq_bindcpu)
    {
        return;
    }

    us_throughput_irq_pps_high = g_st_thread_bindcpu.us_throughput_pps_irq_high;
    us_throughput_irq_pps_low  = g_st_thread_bindcpu.us_throughput_pps_irq_low;

    /* 如果共存绑核参数生效则使用共存绑核门限值 */
    if (OAL_TRUE == g_st_thread_bindcpu.en_btcoex_flag)
    {
        us_throughput_irq_pps_high = g_st_thread_bindcpu.us_btcoex_throughput_pps_irq_high;
        us_throughput_irq_pps_low  = g_st_thread_bindcpu.us_btcoex_throughput_pps_irq_low;
    }
    /* 根据流量决定绑核模式 */
    if (ul_total_pps > us_throughput_irq_pps_high)
    {
        en_irq_cpu_state = WLAN_IRQ_AFFINITY_BUSY_CPU;
        /* 关闭低功耗200ms,避免峰值跑流时CPU消耗过低被迁走 */
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        core_ctl_set_boost(2*WLAN_STATIS_DATA_TIMER_PERIOD);
        pm_qos_update_request(&g_st_pmqos_requset, PM_QOS_THOUGHTPUT_VALUE);
#endif
#endif
    }
    else if (ul_total_pps < us_throughput_irq_pps_low)
    {
        en_irq_cpu_state = WLAN_IRQ_AFFINITY_IDLE_CPU;
#if defined(_PRE_FEATURE_PLAT_LOCK_CPUFREQ) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
        pm_qos_update_request(&g_st_pmqos_requset, PM_QOS_DEFAULT_VALUE);
#endif
#endif
    }
    else
    {
        return;
    }

    /* 绑核状态变动时才需要执行绑核动作 */
    if (en_irq_cpu_state != g_st_thread_bindcpu.en_irq_cpu_state)
    {
        OAM_WARNING_LOG3(0,OAM_SF_ANY, "{hmac_rxdata_thread_bindcpu:rxdata_thread bind cpu:[%d] total_pps[%d],btcoex_flag[%d]}",
            en_irq_cpu_state, ul_total_pps, g_st_thread_bindcpu.en_btcoex_flag);

        /* 流量较大时将收发线程绑定在大核 */
        if (WLAN_IRQ_AFFINITY_BUSY_CPU == en_irq_cpu_state)
        {
            hmac_thread_bind_fast_cpu();
        }
        else
        {
            hmac_thread_bind_slow_cpu();
        }

        /* 更新状态 */
        g_st_thread_bindcpu.en_irq_cpu_state = en_irq_cpu_state;
    }
#endif
}
#endif


OAL_STATIC oal_void hmac_wifi_reset_stat(oal_void)
{
    g_st_wifi_rxtx_statis.uc_timer_cycles = 0;
    g_st_wifi_rxtx_statis.ul_rx_pkts  = 0;
    g_st_wifi_rxtx_statis.ul_tx_pkts  = 0;
    g_st_wifi_rxtx_statis.ul_rx_bytes = 0;
    g_st_wifi_rxtx_statis.ul_tx_bytes = 0;
    g_st_wifi_rxtx_statis.ul_tx_large_pkt = 0;
    g_st_wifi_rxtx_statis.ul_rx_large_pkt = 0;
    g_st_wifi_rxtx_statis.ul_tx_small_pkt = 0;
    g_st_wifi_rxtx_statis.ul_rx_small_pkt = 0;
    g_st_wifi_rxtx_statis.ul_pre_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();
}


OAL_STATIC oal_uint32 hmac_wifi_calculate_stat(wifi_txrx_calculated_stat_stru *pst_calculated_stat)
{
    oal_uint32 ul_trx_total;
    oal_uint32 ul_cur_time;
    oal_uint32 ul_dur_ms;

    g_st_wifi_rxtx_statis.uc_timer_cycles++;

    ul_cur_time = (oal_uint32) OAL_TIME_GET_STAMP_MS();
    ul_dur_ms = OAL_TIME_GET_RUNTIME(g_st_wifi_rxtx_statis.ul_pre_time, ul_cur_time);

    /* 如果当前统计时间不足定时器周期的一半,会导致统计PPS值偏大返回 */
    if (0 == ul_dur_ms || ul_dur_ms < (WLAN_STATIS_DATA_TIMER_PERIOD >> 1))
    {
        return OAL_FAIL;
    }

    /* 超过一定时常则不统计，排除定时器异常 */
    if (ul_dur_ms > (WLAN_STATIS_DATA_TIMER_PERIOD * WLAN_THROUGHPUT_STA_PERIOD) << 2)
    {
        hmac_wifi_reset_stat();
        return OAL_FAIL;
    }

    ul_trx_total = g_st_wifi_rxtx_statis.ul_rx_pkts + g_st_wifi_rxtx_statis.ul_tx_pkts;
    g_st_wifi_rxtx_statis.ul_total_sdio_pps = (ul_trx_total * 1000) / ul_dur_ms;

    pst_calculated_stat->ul_tx_pps = (g_st_wifi_rxtx_statis.ul_tx_pkts * 1000) / ul_dur_ms;

    pst_calculated_stat->ul_tx_large_pps = (g_st_wifi_rxtx_statis.ul_tx_large_pkt * 1000) / ul_dur_ms;
    pst_calculated_stat->ul_rx_large_pps = (g_st_wifi_rxtx_statis.ul_rx_large_pkt * 1000) / ul_dur_ms;
    pst_calculated_stat->ul_tx_small_pps = (g_st_wifi_rxtx_statis.ul_tx_small_pkt * 1000) / ul_dur_ms;
    pst_calculated_stat->ul_rx_small_pps = (g_st_wifi_rxtx_statis.ul_rx_small_pkt * 1000) / ul_dur_ms;

    pst_calculated_stat->ul_rx_throughput_mbps = (g_st_wifi_rxtx_statis.ul_rx_bytes >> 7) / ul_dur_ms;
    pst_calculated_stat->ul_tx_throughput_mbps = (g_st_wifi_rxtx_statis.ul_tx_bytes >> 7) / ul_dur_ms;

    /* Wi-Fi 业务负载标记 */
    if (pst_calculated_stat->ul_rx_throughput_mbps <= WLAN_THROUGHPUT_LOAD_LOW)
    {
        g_st_wifi_rxtx_statis.en_wifi_rx_busy = OAL_FALSE;
    }
    else
    {
        g_st_wifi_rxtx_statis.en_wifi_rx_busy = OAL_TRUE;
    }

    return OAL_SUCC;
}


OAL_STATIC oal_void hmac_wifi_update_func_state(wifi_txrx_calculated_stat_stru *pst_calculated_stat)
{
    oal_uint32 ul_tx_throughput_mbps = pst_calculated_stat->ul_tx_throughput_mbps;
    oal_uint32 ul_rx_throughput_mbps = pst_calculated_stat->ul_rx_throughput_mbps;
    oal_uint32 ul_tx_pps = pst_calculated_stat->ul_tx_pps;
    oal_uint32 ul_tx_large_pps = pst_calculated_stat->ul_tx_large_pps;
    oal_uint32 ul_rx_large_pps = pst_calculated_stat->ul_rx_large_pps;
    oal_uint32 ul_tx_small_pps = pst_calculated_stat->ul_tx_small_pps;
    oal_uint32 ul_rx_small_pps = pst_calculated_stat->ul_rx_small_pps;
#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
    oal_uint32 ul_total_pps = ul_tx_pps + ul_rx_large_pps + ul_rx_small_pps;
#endif

#ifdef _PRE_WLAN_FEATURE_MULTI_NETBUF_AMSDU
    /* AMSDU+AMPDU聚合切换入口 */
    hmac_tx_amsdu_ampdu_switch(ul_tx_throughput_mbps);
#endif
    hmac_tx_small_amsdu_switch(ul_rx_throughput_mbps, ul_tx_pps);

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    hmac_tx_tcp_ack_buf_switch(ul_rx_throughput_mbps);
#endif

#ifdef _PRE_WLAN_TCP_OPT
    /* 根据rx流量Mbps 来控制 启动/关闭 TCP_ACK filter优化功能 */
    hmac_tcp_ack_opt_switch_ctrol(ul_rx_throughput_mbps);
#endif

#ifdef _PRE_WLAN_FEATURE_DYN_BYPASS_EXTLNA
    /* 根据吞吐量，判断是否需要bypass 外置LNA */
    hmac_rx_dyn_bypass_extlna_switch(ul_tx_throughput_mbps, ul_rx_throughput_mbps);
#endif

#if(_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
    hmac_thread_bindcpu(ul_total_pps);
#endif

    hmac_rx_dscr_th_opt(ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps, ul_rx_small_pps);

    hmac_rx_tcp_ack_filter_switch(ul_tx_large_pps, ul_rx_large_pps, ul_tx_small_pps, ul_rx_small_pps);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
    hmac_auto_set_ddr_freq(ul_total_pps);
#endif

#ifdef _PRE_WLAN_FEATURE_APF
    hmac_auto_set_apf_switch_in_suspend(ul_total_pps);
#endif

    if (g_st_wifi_rxtx_statis.uc_timer_cycles >= WLAN_THROUGHPUT_STA_PERIOD)
    {
        /* 平滑处理，每2s检查一次是否需要修改低功耗定时器的值 */
        hmac_set_psm_activity_timer(g_st_wifi_rxtx_statis.ul_total_sdio_pps);

        /* 2s周期清零一次 */
        hmac_wifi_reset_stat();
    }
}


oal_void hmac_wifi_calculate_throughput(oal_void)
{
    wifi_txrx_calculated_stat_stru st_calculated_stat = {0};

    if (OAL_SUCC != hmac_wifi_calculate_stat(&st_calculated_stat))
    {
        return;
    }

    hmac_wifi_update_func_state(&st_calculated_stat);
}


oal_void hmac_wifi_statis_data_timeout(oal_void)
{
    hmac_wifi_calculate_throughput();
}


oal_void  hmac_wifi_statis_data_timer_init(oal_void)
{

    /*如果这个定时器已经注册成功，则不能再次被注册！*/
    if (OAL_TRUE == g_st_wifi_rxtx_statis.st_statis_data_timer.en_is_registerd
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE)&&(_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    || OAL_FALSE == wlan_pm_is_poweron()
#endif
    )
    {
         return;
    }

    /* 清空统计 */
    OAL_MEMZERO(&g_st_wifi_rxtx_statis, (OAL_SIZEOF(g_st_wifi_rxtx_statis) - OAL_SIZEOF(g_st_wifi_rxtx_statis.st_statis_data_timer)));

    FRW_TIMER_CREATE_TIMER(&g_st_wifi_rxtx_statis.st_statis_data_timer, //pst_timeout
                        (oal_void*)hmac_wifi_statis_data_timeout,   //p_timeout_func
                        WLAN_STATIS_DATA_TIMER_PERIOD,             //ul_timeout
                        OAL_PTR_NULL,                          // p_timeout_arg
                        OAL_TRUE,                             //en_is_periodic
                        OAM_MODULE_ID_HMAC,0 );             //en_module_id && ul_core_id

}


oal_void  hmac_wifi_statis_data_timer_deinit(oal_void)
{
    if (OAL_FALSE == g_st_wifi_rxtx_statis.st_statis_data_timer.en_is_registerd)
    {
        return;
    }

    FRW_TIMER_IMMEDIATE_DESTROY_TIMER(&g_st_wifi_rxtx_statis.st_statis_data_timer);

}

oal_void  hmac_wifi_pm_state_notify(oal_bool_enum_uint8 en_wake_up)
{
    if (OAL_TRUE == en_wake_up)
    {
        /* WIFI唤醒,启动吞吐量统计定时器 */
        hmac_wifi_statis_data_timer_init();
    }
    else
    {
        /* WIFI睡眠,关闭吞吐量统计定时器 */
        hmac_wifi_statis_data_timer_deinit();
    }

}


oal_void  hmac_wifi_state_notify(oal_bool_enum_uint8 en_wifi_on)
{
    if (OAL_TRUE == en_wifi_on)
    {
        /* WIFI上电,启动吞吐量统计定时器,打开调频功能 */
#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
        hmac_set_device_freq_mode(H2D_FREQ_MODE_ENABLE);
#endif
        /* WIFI打开时,定时器参数更新 */
        g_st_wifi_rxtx_statis.ul_pre_time = (oal_uint32)OAL_TIME_GET_STAMP_MS();

        hmac_wifi_statis_data_timer_init();
    }
    else
    {
        /* WIFI下电,关闭吞吐量统计定时器,关闭调频功能 */
        hmac_wifi_statis_data_timer_deinit();

#ifdef _PRE_WLAN_FEATURE_AUTO_FREQ
        hmac_set_device_freq_mode(H2D_FREQ_MODE_DISABLE);
#endif
    }

}

#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif


