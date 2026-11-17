

/* 头文件包含 */
#include "bfgx_exception_rst.h"
#include "plat_debug.h"
#include "bfgx_dev.h"
#include "plat_pm.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"
#include "plat_uart.h"
#include "oal_hcc_host_if.h"
#include "oal_ext_if.h"
#include "bfgx_data_parse.h"
#include "chr_errno.h"
#include "oam_rdr.h"
#include "securec.h"
#include "chr_user.h"

/* 全局变量定义 */
OAL_STATIC struct st_exception_info *g_hi110x_exception_info = NULL;

OAL_STATIC struct st_bfgx_reset_cmd g_bfgx_reset_msg[BFGX_BUTT] = {
    { BT_RESET_CMD_LEN,   { 0x04, 0xff, 0x01, 0xc7 }},
    { FM_RESET_CMD_LEN,   { 0xfb }},
    { GNSS_RESET_CMD_LEN, { 0xc7, 0x51, 0x0, 0x0, 0xa5, 0xa5, 0x0, 0x0, GNSS_SEPER_TAG_LAST }},
    { IR_RESET_CMD_LEN,   {0}},
    { NFC_RESET_CMD_LEN,  {0}},
};

OAL_STATIC uint8_t g_beat_timeout_reset_enable = 0;
#ifdef PLATFORM_DEBUG_ENABLE
OAL_STATIC uint8_t g_excp_test_cfg[EXCP_TEST_CFG_BOTT] = {
    DFR_TEST_DISABLE, DFR_TEST_DISABLE, DFR_TEST_DISABLE, DFR_TEST_DISABLE
};
#endif

memdump_info_t g_bcpu_memdump_cfg;
memdump_info_t g_gcpu_memdump_cfg;
memdump_info_t g_wcpu_memdump_cfg;

OAL_STATIC void plat_exception_reset_work(oal_work_stru *work);
OAL_STATIC int32_t wifi_exception_handler(void);
OAL_STATIC int32_t wifi_system_reset(void);
OAL_STATIC int32_t wifi_status_recovery(void);
OAL_STATIC int32_t bfgx_exception_handler(void);
OAL_STATIC int32_t bfgx_subthread_reset(void);
int32_t bfgx_subsystem_reset(void);
int32_t bfgx_system_reset(void);
OAL_STATIC void bfgx_dump_stack(void);
OAL_STATIC int32_t bfgx_status_recovery(void);
int32_t plat_bfgx_exception_rst_register(struct ps_plat_s *data);

/*
 * 函 数 名  : get_exception_info_reference
 * 功能描述  : 获得保存异常信息的全局变量
 */
struct st_exception_info *get_exception_info_reference(void)
{
    if (g_hi110x_exception_info == NULL) {
        ps_print_err("%s g_hi110x_exception_info is NULL\n", __func__);
    }

    return g_hi110x_exception_info;
}
EXPORT_SYMBOL_GPL(get_exception_info_reference);

/*
 * 函 数 名  : plat_dfr_cfg_set
 * 功能描述  : dfr全局配置
 */
void plat_dfr_cfg_set(unsigned long arg)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return;
    }

    pst_exception_data->exception_reset_enable = arg ? (PLAT_EXCEPTION_ENABLE) : (PLAT_EXCEPTION_DISABLE);

    ps_print_info("plat dfr cfg set value = %ld\n", arg);
}

/*
 * 函 数 名  : plat_beat_timeout_reset_set
 * 功能描述  : beat_timer全局配置
 */
void plat_beat_timeout_reset_set(unsigned long arg)
{
    g_beat_timeout_reset_enable = arg ? (PLAT_EXCEPTION_ENABLE) : (PLAT_EXCEPTION_DISABLE);
    ps_print_info("plat beat timer timeOut reset cfg set value = %ld\n", arg);
}

int32_t mod_beat_timer(uint8_t on)
{
    int ret;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (on) {
        ret = mod_timer(&pst_exception_data->bfgx_beat_timer, jiffies + BFGX_BEAT_TIME * HZ);
        atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_NOT_RECV_BEAT_INFO);
        ps_print_info("reset beat timer, ret=%d, jiffers=%lu, expires=%lu\n",
                      ret, jiffies, pst_exception_data->bfgx_beat_timer.expires);
    } else {
        ps_print_info("delete beat timer\n");
        del_timer_sync(&pst_exception_data->bfgx_beat_timer);
    }

    return 0;
}

/*
 * 函 数 名  : bfgx_beat_timer_expire
 * 功能描述  : bfgx心跳超时处理函数，该函数运行在软中断上下文中，不能有引起睡眠的操作
 * 输入参数  : uint64 data，不需要。加上是为了遵守内核的函数声明
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
OAL_STATIC void bfgx_beat_timer_expire(unsigned long data)
#else
OAL_STATIC void bfgx_beat_timer_expire(struct timer_list *t)
#endif
{
    int ret;
    static uint32_t timer_restart_cnt = 0;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;

    if (bfgx_is_shutdown()) {
        ps_print_warning("bfgx is closed\n");
        return;
    }

    pst_exception_data = get_exception_info_reference();
    if (unlikely(pst_exception_data == NULL || pst_exception_data->ps_plat_d == NULL)) {
        ps_print_err("ptr is NULL\n");
        return;
    }

    ps_core_d = pst_exception_data->ps_plat_d->core_data;

    /* bfgx睡眠时，没有心跳消息上报 */
    if (ps_core_d->pm_data->bfgx_dev_state_get(ps_core_d->pm_data) == BFGX_SLEEP) {
        ps_print_info("bfgx has sleep!\n");
        return;
    }

    if (atomic_read(&pst_exception_data->bfgx_beat_flag) == BFGX_NOT_RECV_BEAT_INFO) {
        ps_print_info("timeout!  HZ=%d, jiffers=%lu, expires=%lu\n",
                      HZ, jiffies, pst_exception_data->bfgx_beat_timer.expires);

        /* 异常超时，时间未到，重启定时器，连续尝试5次 */
        if (time_after(pst_exception_data->bfgx_beat_timer.expires, jiffies) && (timer_restart_cnt < 5)) {
            timer_restart_cnt++;
            ret = mod_timer(&pst_exception_data->bfgx_beat_timer, jiffies + msecs_to_jiffies(BFGX_BEAT_TIME * 1000));
            ps_print_err("error! timer is not expires and restart, ret=%d, jiffers=%lu, expires=%lu\n",
                ret, jiffies, pst_exception_data->bfgx_beat_timer.expires);
            return;
        }

        timer_restart_cnt = 0;

        ps_print_err("###########host can not recvive bfgx beat info@@@@@@@@@@@@@@!\n");

        declare_dft_trace_key_info("bfgx beat timeout", OAL_DFT_TRACE_EXCEP);

        (void)ssi_dump_err_regs(SSI_ERR_BFGX_HEART_TIMEOUT);

        if (g_beat_timeout_reset_enable == PLAT_EXCEPTION_ENABLE) {
            ps_print_err("bfgx beat timer bring to reset work!\n");
            plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, BFGX_BEATHEART_TIMEOUT);
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_DEV, CHR_PLAT_DRV_ERROR_BEAT_TIMEOUT);
            return;
        }
    }

    atomic_set(&pst_exception_data->bfgx_beat_flag, BFGX_NOT_RECV_BEAT_INFO);

    timer_restart_cnt = 0;
    ret = mod_timer(&pst_exception_data->bfgx_beat_timer, jiffies + BFGX_BEAT_TIME * HZ);
    ps_print_info("reset beat timer, ret=%d, jiffers=%lu, expires=%lu\n",
                  ret, jiffies, pst_exception_data->bfgx_beat_timer.expires);

    bfg_check_timer_work(ps_core_d->pm_data);

    return;
}

OAL_STATIC void plat_dfr_sysrst_type_cnt_inc(enum dfr_rst_system_type_e rst_type, enum subsystem_enum subs)
{
    struct st_exception_info *pst_exception_data = NULL;
    if ((rst_type >= DFR_SYSTEM_RST_TYPE_BOTT) || (subs >= SUBSYS_BOTTOM)) {
        goto exit;
    }
    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        goto exit;
    }
    if (pst_exception_data->subsys_type == subs) {
        pst_exception_data->etype_info[pst_exception_data->excetion_type].rst_type_cnt[rst_type]++;
    }
    return;
exit:
    ps_print_err("dfr rst type cnt inc fail\n");
}

OAL_STATIC excp_info_str_t g_excp_info_str_tab[] = {
    { .id = BFGX_BEATHEART_TIMEOUT, .name = "bfgx_beatheart_timeout" },
    { .id = BFGX_LASTWORD_PANIC,    .name = "bfgx_lastword_panic" },
    { .id = BFGX_TIMER_TIMEOUT,     .name = "bfgx_timer_timeout" },
    { .id = BFGX_ARP_TIMEOUT,       .name = "bfgx_arp_timeout" },
    { .id = BFGX_POWERON_FAIL,      .name = "bfgx_poweron_fail" },
    { .id = BFGX_WAKEUP_FAIL,       .name = "bfgx_wakeup_fail" },
    { .id = WIFI_WATCHDOG_TIMEOUT,  .name = "wifi_watchdog_timeout" },
    { .id = WIFI_POWERON_FAIL,      .name = "wifi_poweron_fail" },
    { .id = WIFI_WAKEUP_FAIL,       .name = "wifi_wakeup_fail" },
    { .id = WIFI_DEVICE_PANIC,      .name = "wifi_device_panic" },
    { .id = WIFI_TRANS_FAIL,        .name = "wifi_trans_fail" },
    { .id = SDIO_DUMPBCPU_FAIL,     .name = "sdio_dumpbcpu_fail" },
};

OAL_STATIC char *excp_info_str_get(int32_t id)
{
    int32_t i = 0;
    for (i = 0; i < sizeof(g_excp_info_str_tab) / sizeof(excp_info_str_t); i++) {
        if (id == g_excp_info_str_tab[i].id) {
            return g_excp_info_str_tab[i].name;
        }
    }
    return NULL;
}

int32_t plat_get_dfr_sinfo(char *buf, int32_t index)
{
    int i = index;
    int etype;
    int ret;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }
    ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1, "==========dfr info:=========\n");
    if (ret < 0) {
        return i;
    }
    i += ret;

    ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1, "total cnt:%-10d\n", plat_get_excp_total_cnt());
    if (ret < 0) {
        return i;
    }
    i += ret;

    for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
        ret = snprintf_s(buf + i, PAGE_SIZE - i, PAGE_SIZE - i - 1,
                         "id:%-30scnt:%-10dfail:%-10dsingle_sysrst:%-10dall_sysrst:%-10dmaxtime:%lldms\n",
                         excp_info_str_get(etype),
                         pst_exception_data->etype_info[etype].excp_cnt,
                         pst_exception_data->etype_info[etype].fail_cnt,
                         pst_exception_data->etype_info[etype].rst_type_cnt[DFR_SINGLE_SYS_RST],
                         pst_exception_data->etype_info[etype].rst_type_cnt[DFR_ALL_SYS_RST],
                         pst_exception_data->etype_info[etype].maxtime);
        if (ret < 0) {
            return i;
        }
        i += ret;
    }
    return i;
}

OAL_STATIC void plat_print_dfr_info(void)
{
    int etype;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return;
    }
    ps_print_info("========== dfr info:+++++++++++++++++++++++++++++++\n");
    ps_print_info("total cnt:%-10d\n", plat_get_excp_total_cnt());
    for (etype = 0; etype < EXCEPTION_TYPE_BOTTOM; etype++) {
        ps_print_info("type:%-30scnt:%-10dfail:%-10dsingle_sysrst:%-10dall_sysrst:%-10dmaxtime:%lldms\n",
                      excp_info_str_get(etype),
                      pst_exception_data->etype_info[etype].excp_cnt,
                      pst_exception_data->etype_info[etype].fail_cnt,
                      pst_exception_data->etype_info[etype].rst_type_cnt[DFR_SINGLE_SYS_RST],
                      pst_exception_data->etype_info[etype].rst_type_cnt[DFR_ALL_SYS_RST],
                      pst_exception_data->etype_info[etype].maxtime);
    }
    ps_print_info("========== dfr info:-----------------------------------\n");
}

int32_t plat_get_excp_total_cnt()
{
    int i;
    int total_cnt = 0;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }
    for (i = 0; i < EXCEPTION_TYPE_BOTTOM; i++) {
        total_cnt += pst_exception_data->etype_info[i].excp_cnt;
    }
    return total_cnt;
}

OAL_STATIC uint32_t plat_is_dfr_enable(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return OAL_FALSE;
    }

    if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
        ps_print_info("plat exception reset not enable!");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

OAL_STATIC int32_t plat_power_fail_excep_check(uint32_t subsys_type,
                                               uint32_t thread_type,
                                               uint32_t exception_type)
{
    if (subsys_type >= SUBSYS_BOTTOM) {
        ps_print_err("para subsys_type %u is error!\n", subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (((subsys_type == SUBSYS_WIFI) && (thread_type >= WIFI_THREAD_BOTTOM)) ||
        ((subsys_type == SUBSYS_BFGX) && (thread_type >= BFGX_THREAD_BOTTOM))) {
        ps_print_err("para thread_type %u is error! subsys_type is %u\n", thread_type, subsys_type);
        return -EXCEPTION_FAIL;
    }

    if (exception_type >= EXCEPTION_TYPE_BOTTOM) {
        ps_print_err("para exception_type %u is error!\n", exception_type);
        return -EXCEPTION_FAIL;
    }

    if (plat_is_dfr_enable() == OAL_FALSE) {
        ps_print_err("dfr disabled\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_power_fail_exception_info_set
 * 功能描述  : bfgx open失败时，调用该函数设置exception info
 * 输入参数  : subsys_type:调用异常处理的子系统，WIFI或者BFGX
 *             thread_type:子系统中的子线程
 *             exception_type:异常的类型
 */
int32_t plat_power_fail_exception_info_set(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type)
{
    struct st_exception_info *pst_exception_data = NULL;
    unsigned long flag;

    if (plat_power_fail_excep_check(subsys_type, thread_type, exception_type) !=
        EXCEPTION_SUCCESS) {
        ps_print_err("check failed\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    oal_spin_lock_irq_save(&pst_exception_data->exception_spin_lock, &flag);

    if (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_IDLE) {
        pst_exception_data->subsys_type = subsys_type;
        pst_exception_data->thread_type = thread_type;
        pst_exception_data->excetion_type = exception_type;

        /* 当前异常没有处理完成之前，不允许处理新的异常 */
        atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_BUSY);
    } else {
        ps_print_info("plat is processing exception! subsys=%d, exception type=%d\n",
                      pst_exception_data->subsys_type, pst_exception_data->excetion_type);
        oal_spin_unlock_irq_restore(&pst_exception_data->exception_spin_lock, &flag);
        return -EXCEPTION_FAIL;
    }

    /* 增加统计信息 */
    pst_exception_data->etype_info[exception_type].excp_cnt += 1;

    oal_spin_unlock_irq_restore(&pst_exception_data->exception_spin_lock, &flag);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_power_fail_process_done
 * 功能描述  : bfgx open失败时，异常处理完成
 */
void plat_power_fail_process_done(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return;
    }

    atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);

    ps_print_suc("bfgx open fail process done\n");

    return;
}

STATIC int32_t bfgx_excp_handle_before_rst(uint32_t exception_type)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL) || unlikely(ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EXCEPTION_FAIL;
    }

    pm_data = ps_core_d->pm_data;

    /* timer time out 中断调用时，不能再这里删除，死锁 */
    if (exception_type != BFGX_BEATHEART_TIMEOUT) {
        pm_data->operate_beat_timer(BEAT_TIMER_DELETE);
    }

    if (exception_type != BFGX_LASTWORD_PANIC) {
        ps_uart_state_dump(ps_core_d);
    }

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_exception_handler
 * 功能描述  : 平台host异常处理总入口函数，填充异常信息，并触发异常处理work
 * 输入参数  : subsys_type:调用异常处理的子系统，WIFI或者BFGX
 *             thread_type:子系统中的子线程
 *             exception_type:异常的类型
 * 返 回 值  : 异常处理成功返回0，否则返回1
 */
int32_t plat_exception_handler(uint32_t subsys_type, uint32_t thread_type, uint32_t exception_type)
{
    struct st_exception_info *pst_exception_data = NULL;
    unsigned long flag;
    int32_t timeout;

    if (plat_power_fail_excep_check(subsys_type, thread_type, exception_type) != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        return -EXCEPTION_FAIL;
    }

    /* 这里只能用spin lock，因为该函数会被心跳超时函数调用，心跳超时函数属于软中断，不允许睡眠 */
    oal_spin_lock_irq_save(&pst_exception_data->exception_spin_lock, &flag);
    if (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_IDLE) {
        pst_exception_data->subsys_type = subsys_type;
        pst_exception_data->thread_type = thread_type;
        pst_exception_data->excetion_type = exception_type;

        /* 当前异常没有处理完成之前，不允许处理新的异常 */
        atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_BUSY);
    } else {
        ps_print_info("plat is processing exception! subsys=%d, exception type=%d\n",
                      pst_exception_data->subsys_type, pst_exception_data->excetion_type);
        oal_spin_unlock_irq_restore(&pst_exception_data->exception_spin_lock, &flag);
        return EXCEPTION_SUCCESS;
    }
    oal_spin_unlock_irq_restore(&pst_exception_data->exception_spin_lock, &flag);

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG
    /* 等待SSI操作完成，防止NOC */
    timeout = wait_for_ssi_idle_timeout(HI110X_DFR_WAIT_SSI_IDLE_MS);
    if (timeout <= 0) {
        ps_print_err("[HI110X_DFR]wait for ssi idle failed\n");
        atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);
        return EXCEPTION_FAIL;
    }
    ps_print_info("[HI110X_DFR]wait for ssi idle cost time:%dms\n", HI110X_DFR_WAIT_SSI_IDLE_MS - timeout);
#endif

    if (subsys_type == SUBSYS_BFGX) {
        if (bfgx_excp_handle_before_rst(exception_type) < 0) {
            ps_print_err("bfgx excep before rst failed\n");
            return EXCEPTION_FAIL;
        }
    }

    pst_exception_data->etype_info[pst_exception_data->excetion_type].stime = ktime_get();
    ps_print_warning("[HI110X_DFR]plat start doing exception! subsys=%d, exception type=%d\n",
                     pst_exception_data->subsys_type, pst_exception_data->excetion_type);

    oal_wake_lock_timeout(&pst_exception_data->plat_exception_rst_wlock, 10 * 1000); /* 处理异常，持锁10秒 */
    /* 触发异常处理worker */
    queue_work(pst_exception_data->plat_exception_rst_workqueue, &pst_exception_data->plat_exception_rst_work);

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL(plat_exception_handler);

/*
 * 函 数 名  : plat_exception_reset_handler
 * 功能描述  : 执行平台的复位恢复动作
 * 返 回 值  : 异常处理成功返回0，否则返回负值
 */
OAL_STATIC int32_t plat_exception_reset_handler(struct st_exception_info *exception_data)
{
    ktime_t trans_time;
    unsigned long long total_time;
    if (exception_data->subsys_type == SUBSYS_WIFI) {
        if (wlan_is_shutdown()) {
            ps_print_warning("[HI110X_DFR]wifi is closed,stop wifi dfr\n");
            return -EXCEPTION_FAIL;
        }

        exception_data->etype_info[exception_data->excetion_type].excp_cnt += 1;
        if (wifi_exception_handler() != EXCEPTION_SUCCESS) {
            ps_print_err("[HI110X_DFR] wifi exception handler failed\n");
            return  -EXCEPTION_FAIL;
        }
    } else {
        if (bfgx_is_shutdown()) {
            /* bfgx下电接收到last word，memdump会失败
             * 下次bfgx上电会出现等memdump超时异常
             */
            atomic_set(&exception_data->is_memdump_runing, 0);
            ps_print_warning("[HI110X_DFR]bfgx is closed,stop bfgx dfr\n");
            return -EXCEPTION_FAIL;
        }

        exception_data->etype_info[exception_data->excetion_type].excp_cnt += 1;
        if (bfgx_exception_handler() != EXCEPTION_SUCCESS) {
            ps_print_err("[HI110X_DFR] bfgx exception handler failed\n");
            return  -EXCEPTION_FAIL;
        }
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
    trans_time = ktime_sub(ktime_get(), exception_data->etype_info[exception_data->excetion_type].stime);
    total_time = (unsigned long long)ktime_to_ms(trans_time);
    if (exception_data->etype_info[exception_data->excetion_type].maxtime < total_time) {
        exception_data->etype_info[exception_data->excetion_type].maxtime = total_time;
        ps_print_warning("[HI110X_DFR]time update:%llu,exception_type:%d \n",
                         total_time, exception_data->excetion_type);
    }
#endif
    ps_print_warning("[HI110X_DFR]plat execption recovery success, current time [%llu]ms, max time [%llu]ms\n",
                     total_time, exception_data->etype_info[exception_data->excetion_type].maxtime);

    /* 每次dfr完成，显示历史dfr处理结果 */
    plat_print_dfr_info();
    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_exception_reset_work
 * 功能描述  : 平台host异常处理work，判断产生异常的系统，调用相应的处理函数。
 *             这个函数会获取mutex，以避免并发处理。
 * 返 回 值  : 异常处理成功返回0，否则返回1
 */
OAL_STATIC void plat_exception_reset_work(oal_work_stru *work)
{
    struct pm_top* pm_top_data = pm_get_top();

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("[HI110X_DFR]get exception info reference is error\n");
        return;
    }

    ps_print_warning("[HI110X_DFR] enter plat_exception_reset_work\n");
    mutex_lock(&(pm_top_data->host_mutex));

    if (plat_exception_reset_handler(pst_exception_data) != EXCEPTION_SUCCESS) {
        ps_print_err("[HI110X_DFR] platform reset handler fail\n");
    }

    plat_bbox_msg_hander(pst_exception_data->subsys_type, pst_exception_data->excetion_type);
    atomic_set(&pst_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);
    mutex_unlock(&(pm_top_data->host_mutex));
}

OAL_STATIC int32_t is_subsystem_rst_enable(void)
{
#ifdef PLATFORM_DEBUG_ENABLE
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return DFR_TEST_DISABLE;
    }
    ps_print_info("#########is_subsystem_rst_enable:%d\n", pst_exception_data->subsystem_rst_en);
    return pst_exception_data->subsystem_rst_en;
#else
    return DFR_TEST_DISABLE;
#endif
}

/*
 * 函 数 名  : wifi_subsystem_reset
 * 功能描述  : wifi子系统复位，通过uart复位解复位WCPU,重新加载wifi firmware
 */
OAL_STATIC int32_t wifi_subsystem_reset(void)
{
    int32_t ret;

    if (hi1103_wifi_subsys_reset() != EXCEPTION_SUCCESS) {
        ps_print_err("wifi subsys reset failed\n");
        return -EXCEPTION_FAIL;
    }

    hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    if (firmware_download_function(WIFI_CFG) != EXCEPTION_SUCCESS) {
        ps_print_err("wifi firmware download failed\n");
        return -EXCEPTION_FAIL;
    }

    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        ps_print_err("HCC_BUS_POWER_PATCH_LAUCH failed, ret=%d", ret);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_DFR, CHR_PLAT_DRV_ERROR_WIFI_SUBSYS_WCPU_BOOTUP);

        return -EXCEPTION_FAIL;
    }

    // 下发定制化参数到device去
    wifi_customize_h2d();

    plat_dfr_sysrst_type_cnt_inc(DFR_SINGLE_SYS_RST, SUBSYS_WIFI);

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_WIFI_SUBSYS_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : wifi_exception_handler
 * 功能描述  : wifi异常处理入口函数，判断BFGN是否打开，如果打开则调用单系统复位流程，
 *             BFGN没开则调用全系统复位流程(单系统复位需要用到uart)。如果单系统复位
 *             失败，则会进行全系统复位。
 * 返 回 值  : 异常处理成功返回0，否则返回1
 */
OAL_STATIC int32_t wifi_exception_handler(void)
{
    int32_t ret = -EXCEPTION_FAIL;
    uint32_t exception_type;
    struct st_exception_info *pst_exception_data = NULL;

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_DFR, CHR_PLAT_DRV_ERROR_EXCP_WIFI_HANDLE);

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    exception_type = pst_exception_data->excetion_type;

    /* 如果bfgx已经打开，执行wifi子系统复位流程，否则执行wifi全系统复位流程 */
    if (!bfgx_is_shutdown()) {
        ps_print_info("bfgx is opened, start wifi subsystem reset!\n");
        bfgx_print_subsys_state();
        if (is_subsystem_rst_enable() != DFR_TEST_ENABLE) {
            ret = wifi_subsystem_reset();
            if (ret != EXCEPTION_SUCCESS) {
                ps_print_err("wifi subsystem reset failed, start wifi system reset!\n");
                ret = wifi_system_reset();
            }
        } else {
            ps_print_err("in debug mode,skip subsystem rst,do wifi system reset!\n");
            ret = wifi_system_reset();
        }
    } else {
        ps_print_info("bfgx is not opened, start wifi system reset!\n");
        ret = wifi_system_reset();
    }

    if (ret != EXCEPTION_SUCCESS) {
        ps_print_err("wifi execption recovery fail!\n");
        return ret;
    }

    if (pst_exception_data->wifi_callback->wifi_recovery_complete == NULL) {
        ps_print_err("wifi recovery complete callback not regist\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->wifi_callback->wifi_recovery_complete();

    ps_print_info("wifi recovery complete\n");

    return EXCEPTION_SUCCESS;
}

OAL_STATIC int32_t wifi_system_reset_recovery_bfgx(void)
{
    unsigned long timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL) || unlikely(ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EXCEPTION_FAIL;
    }

    pm_data = ps_core_d->pm_data;

    oal_reinit_completion(pm_data->dev_bootok_ack_comp);

    if (open_tty_drv(ps_core_d) != 0) {
        ps_print_err("open_tty_drv failed\n");
        return -EXCEPTION_FAIL;
    }

    if (wlan_pm_open_bcpu() != EXCEPTION_SUCCESS) {
        ps_print_err("wifi reset bcpu fail\n");
        if (release_tty_drv(ps_core_d) != 0) {
            ps_print_warning("wifi_system_reset, release_tty_drv fail, line = %d\n", __LINE__);
        }
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_DFR, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_RST_OPEN_BCPU);

        return -EXCEPTION_FAIL;
    }

    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        ps_print_err("wait bfgx boot ok timeout\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_DFR, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_RST_BCPU_BOOTUP);

        return -EXCEPTION_FAIL;
    }

    bfgx_pm_feature_set(pm_data);

    /* 恢复bfgx业务状态 */
    if (bfgx_status_recovery() != EXCEPTION_SUCCESS) {
        ps_print_err("bfgx status revocery failed!\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_RST_BFGX_RECOVERY);
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

OAL_STATIC void wifi_system_reset_pre_bfgx(void)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL) || unlikely(ps_core_d->pm_data == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    pm_data = ps_core_d->pm_data;

    bfgx_print_subsys_state();
    del_timer_sync(&pm_data->bfg_timer);
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    pm_data->operate_beat_timer(BEAT_TIMER_DELETE);
    if (release_tty_drv(ps_core_d) != 0) {
        ps_print_warning("wifi_system_reset, release_tty_drv fail, line = %d\n", __LINE__);
    }
}

/*
 * 函 数 名  : wifi_system_reset
 * 功能描述  : wifi全系统复位，对device重新上电，bfgn和wifi firmware重新加载，
 *             恢复全系统复位前bfgn的业务
 */
OAL_STATIC int32_t wifi_system_reset(void)
{
    int32_t ret;
    uint32_t dfr_type = DFR_SINGLE_SYS_RST;

    /* 重新上电，firmware重新加载 */
    hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LOAD_PREPARE);

    if (!bfgx_is_shutdown()) {
        dfr_type = DFR_ALL_SYS_RST;
        wifi_system_reset_pre_bfgx();
    }

    ps_print_info("wifi system reset, board power on\n");
    ret = board_power_reset(WLAN_POWER);
    if (ret) {
        ps_print_err("board_power_reset wlan failed=%d\n", ret);
        return -EXCEPTION_FAIL;
    }

    if (firmware_download_function(BFGX_AND_WIFI_CFG) != EXCEPTION_SUCCESS) {
        ps_print_err("hi110x system power reset failed!\n");
        return -EXCEPTION_FAIL;
    }

    if (bfgx_is_shutdown()) {
        hi1103_bfgx_disable();
    }

    ret = hcc_bus_power_action(hcc_get_current_110x_bus(), HCC_BUS_POWER_PATCH_LAUCH);
    if (ret != 0) {
        ps_print_err("HCC_BUS_POWER_PATCH_LAUCH failed ret=%d !!!!!!", ret);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_RST_WIFI_BOOTUP);

        return -EXCEPTION_FAIL;
    }

    // 下发定制化参数到device去
    wifi_customize_h2d();
    if (!bfgx_is_shutdown()) {
        if (wifi_system_reset_recovery_bfgx() != EXCEPTION_SUCCESS) {
            return -EXCEPTION_FAIL;
        }
    }

    plat_dfr_sysrst_type_cnt_inc(dfr_type, SUBSYS_WIFI);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_WIFI_SYSTEM_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : wifi_status_recovery
 * 功能描述  : 全系统复位以后，恢复wifi业务函数
 */
OAL_STATIC int32_t wifi_status_recovery(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (wifi_subsystem_reset() != EXCEPTION_SUCCESS) {
        ps_print_err("wifi subsystem reset failed\n");
        return -EXCEPTION_FAIL;
    }

    if (pst_exception_data->wifi_callback->wifi_recovery_complete == NULL) {
        ps_print_err("wifi recovery complete callback not regist\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->wifi_callback->wifi_recovery_complete();

    ps_print_info("wifi recovery complete\n");

    return EXCEPTION_SUCCESS;
}

OAL_STATIC int32_t wifi_device_reset(void)
{
    ps_print_info("reset wifi device by w_en gpio\n");

    hi1103_wifi_disable();

    if (hi1103_wifi_enable()) {
        ps_print_err("hi1103 wifi enable failed\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : wifi_exception_mem_dump
 * 功能描述  : 全系统复位，firmware重新加载的时候，导出device指定地址的内存
 * 输入参数  : pst_mem_dump_info  : 需要读取的内存信息
 *             count              : 需要读取的内存块个数
 */
OAL_STATIC int32_t wifi_exception_mem_dump(struct st_wifi_dump_mem_info *pst_mem_dump_info,
                                           uint32_t count, int32_t excep_type)
{
    int32_t ret = -OAL_EIO;
    int32_t reset_flag;
    hcc_bus_dev *pst_bus_dev = NULL;
    struct pm_top *pm_top_data = pm_get_top();

    if (excep_type != WIFI_DEVICE_PANIC && excep_type != WIFI_TRANS_FAIL) {
        ps_print_err("unsupport exception type :%d\n", excep_type);
        return -EXCEPTION_FAIL;
    }

    mutex_lock(&(pm_top_data->host_mutex));

    if (wlan_is_shutdown()) {
        ps_print_err("[E]dfr ignored, wifi shutdown before memdump\n");
        goto exit;
    }

    /* device panic不需要复位device，其他异常需要先复位device */
    reset_flag = (excep_type == WIFI_DEVICE_PANIC) ? 0 : 1;

retry:
    if (reset_flag == 1) {
        if (wifi_device_reset() != EXCEPTION_SUCCESS) {
            goto exit;
        }
        hcc_change_state_exception();
    }

    ret = -OAL_EIO;
    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev != NULL) {
        ret = hcc_bus_reinit(pst_bus_dev->cur_bus);
    }

    if (ret != OAL_SUCC) {
        ps_print_err("wifi mem dump:current bus reinit failed, ret=[%d]\n", ret);
        /* 如果panic恢复失败，会复位重试一次 */
        if (reset_flag == 0) {
            ps_print_err("reinit failed, try to reset wifi\n");
            reset_flag = 1;
            goto retry;
        }
        goto exit;
    }

    ret = wifi_device_mem_dump(pst_mem_dump_info, count);
    hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
    if (ret < 0) {
        ps_print_err("wifi_device_mem_dump failed, ret=[%d]\n", ret);
        /* 如果panic恢复失败，会复位重试一次 */
        if (reset_flag == 0) {
            ps_print_err("memdump failed, try to reset wifi\n");
            reset_flag = 1;
            goto retry;
        }
    }

exit:
    mutex_unlock(&(pm_top_data->host_mutex));
    return ret;
}

/*
 * 函 数 名  : wifi_exception_work_submit
 * 功能描述  : 异常恢复动作触发接口
 */
int32_t wifi_exception_work_submit(uint32_t wifi_excp_type)
{
    hcc_bus *hi_bus = hcc_get_current_110x_bus();
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if ((pst_exception_data == NULL) || (hi_bus == NULL)) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFR
    if (oal_work_is_busy(&g_hi110x_exception_info->wifi_excp_worker)) {
        ps_print_warning("WIFI DFR %pF Worker is Processing:doing wifi excp_work...need't submit\n",
                         (void *)g_hi110x_exception_info->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else if (oal_work_is_busy(&g_hi110x_exception_info->wifi_excp_recovery_worker)) {
        ps_print_warning(
            "WIFI DFR %pF Recovery_Worker is Processing:doing wifi wifi_excp_recovery_worker ...need't submit\n",
            (void *)g_hi110x_exception_info->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else if ((atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE) &&
               (pst_exception_data->subsys_type == SUBSYS_WIFI)) {
        ps_print_warning("WIFI DFR %pF Recovery_Worker is Processing:doing  plat wifi recovery ...need't submit\n",
                         (void *)g_hi110x_exception_info->wifi_excp_worker.func);
        return -OAL_EBUSY;
    } else {
        int32_t ret = -OAL_EFAIL;
        ret = wifi_exception_mem_dump(hi_bus->mem_info, hi_bus->mem_size, hi_bus->bus_excp_type);
        if (ret < 0) {
            ps_print_err("Panic File Save Failed!");
        } else {
            ps_print_info("Panic File Save OK!");
        }

        /* 为了在没有开启DFR  的情况下也能mem dump, 故在此处作判断 */
        if (pst_exception_data->exception_reset_enable != PLAT_EXCEPTION_ENABLE) {
            ps_print_info("palt exception reset not enable!");
            return EXCEPTION_SUCCESS;
        }

        ps_print_err("WiFi DFR %pF Worker Submit, excp_type[%d]\n",
                     (void *)g_hi110x_exception_info->wifi_excp_worker.func, wifi_excp_type);
        g_hi110x_exception_info->wifi_excp_type = wifi_excp_type;
        hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_ALL);
        queue_work(g_hi110x_exception_info->wifi_exception_workqueue, &g_hi110x_exception_info->wifi_excp_worker);
    }
#else
    ps_print_warning("Geting WIFI DFR, but _PRE_WLAN_FEATURE_DFR not open!");
#endif  // _PRE_WLAN_FEATURE_DFR
    return OAL_SUCC;
}

oal_workqueue_stru *wifi_get_exception_workqueue(void)
{
    if (g_hi110x_exception_info == NULL) {
        return NULL;
    }
    return g_hi110x_exception_info->wifi_exception_workqueue;
}

/*
 * 函 数 名  : bfgx_exception_handler
 * 功能描述  : bfgx异常处理入口函数，根据异常类型，调用不同的处理函数
 */
OAL_STATIC int32_t bfgx_exception_handler(void)
{
    int32_t ret = -EXCEPTION_FAIL;
    uint32_t exception_type;
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("pst_exception_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    cancel_work_sync(&pm_data->send_allow_sleep_work);
    del_timer_sync(&pm_data->bfg_timer);
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    if (pst_exception_data->excetion_type == BFGX_BEATHEART_TIMEOUT) {
        /* bfgx异常，先删除bfg timer和心跳timer，防止重复触发bfgx异常 */
        pm_data->operate_beat_timer(BEAT_TIMER_DELETE);
    }

    exception_type = pst_exception_data->excetion_type;

    /* ioctl下来的异常执行线程复位流程,当前执行系统全复位 */
    if (exception_type == BFGX_TIMER_TIMEOUT || exception_type == BFGX_ARP_TIMEOUT) {
        ret = bfgx_subthread_reset();
    } else {
        /* 异常恢复之前，尝试用平台命令读栈，不能保证一定成功，因为此时uart可能不通 */
        bfgx_dump_stack();

        ret = bfgx_subsystem_reset();
    }

    if (ret != EXCEPTION_SUCCESS) {
        ps_print_err("bfgx exception recovery fail!, exception_type = [%u]\n", exception_type);
        del_timer_sync(&pm_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;

        pm_data->operate_beat_timer(BEAT_TIMER_DELETE);
        return ret;
    }

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : bfgx_subthread_reset
 * 功能描述  : bfgx子线程复位函数，通过uart下发复位子线程命令，并等待复位
 *             成功ACK，收到ACK，异常处理完成，否则进入BFGX子系统复位流程
 */
OAL_STATIC int32_t bfgx_subthread_reset(void)
{
    /* 这里执行芯片全系统复位，保证芯片下电 */
    return bfgx_system_reset();
}

/*
 * 函 数 名  : __bfgx_subsystem_reset
 * 功能描述  : bfgx子系统复位函数，进入该函数，说明wifi已开，通过sdio下发复位
 *             解复位BCPU命令，进行bfgx子系统复位。子系统复位完成以后需要恢复
 *             复位前bfgin的业务
 * 返 回 值  : 成功返回0，否则返回1
 */
OAL_STATIC int32_t __bfgx_subsystem_reset(void)
{
    unsigned long timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EXCEPTION_FAIL;
    }

    if (ps_chk_bfg_active(ps_core_d) == false) {
        ps_print_err("bfgx no subsys is opened\n");
        return EXCEPTION_SUCCESS;
    }

    if (release_tty_drv(ps_core_d) != EXCEPTION_SUCCESS) {
        ps_print_err("close tty is err!\n");
        if (ps_core_d->tty_have_open == true) {
            return -EXCEPTION_FAIL;
        }
    }

    if (open_tty_drv(ps_core_d) != EXCEPTION_SUCCESS) {
        ps_print_err("open tty is err!\n");
        return -EXCEPTION_FAIL;
    }

    oal_reinit_completion(pm_data->dev_bootok_ack_comp);
    hi1103_bfgx_subsys_reset();
    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        ps_print_err("wait bfgx boot ok timeout, bfg wkeup host gpio level %d\n",
                     oal_gpio_get_value(pm_data->wakeup_host_gpio));
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SUB_SYSTEM_RST_BOOTUP);
        return -EXCEPTION_FAIL;
    }

    bfgx_pm_feature_set(pm_data);

    if (bfgx_status_recovery() != EXCEPTION_SUCCESS) {
        ps_print_err("bfgx recovery status failed\n");

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SUB_SYSTEM_RST_RECOVERY);
        return -EXCEPTION_FAIL;
    }

    plat_dfr_sysrst_type_cnt_inc(DFR_SINGLE_SYS_RST, SUBSYS_BFGX);

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SUBSYS_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : bfgx_subsystem_reset
 * 功能描述  : bfgx子系统复位函数，如果wifi打开，则通过sdio下发复位解复位BCPU命令，
 *             进行bfgx子系统复位，wifi没开，进入bfgx全系统复位流程。如果子系统
 *             复位不能将异常恢复也会进入全系统复位流程。子系统复位完成以后需要
 *             恢复复位前bfgin的业务
 * 返 回 值  : 成功返回0，否则返回1
 */
int32_t bfgx_subsystem_reset(void)
{
    if (!wlan_is_shutdown()) {
        ps_print_info("wifi is opened, start bfgx subsystem reset!\n");

        if (is_subsystem_rst_enable() != DFR_TEST_ENABLE) {
            if (__bfgx_subsystem_reset() != EXCEPTION_SUCCESS) {
                ps_print_err("bfgx subsystem reset failed, start bfgx system reset!\n");
                return bfgx_system_reset();
            }
        } else {
            ps_print_info("in debug mode,skip subsystem rst,do bfgx system reset!\n");
            return bfgx_system_reset();
        }

        return EXCEPTION_SUCCESS;
    } else {
        ps_print_info("wifi is not opened, start bfgx system reset!\n");
        return bfgx_system_reset();
    }
}

OAL_STATIC int32_t bfgx_power_reset(void)
{
    int32_t ret;
    unsigned long timeleft;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = NULL;
    struct pm_top* pm_top_data = pm_get_top();

    ps_core_d = ps_get_core_reference(BUART);
    if (ps_core_d == NULL || ps_core_d->pm_data == NULL) {
        ps_print_err("ps_core_d is err\n");
        return -FAILURE;
    }

    if (release_tty_drv(ps_core_d) != 0) {
        ps_print_warning("bfgx_power_reset, release_tty_drv fail\n");
    }

    ps_print_info("bfgx system reset, board power on\n");
    ret = board_power_reset(BFGX_POWER);
    if (ret) {
        ps_print_err("board_power_reset bfgx failed=%d\n", ret);
        return -EXCEPTION_FAIL;
    }

    if (open_tty_drv(ps_core_d) != 0) {
        ps_print_warning("bfgx_power_reset, open_tty_drv fail\n");
    }

    pm_data = ps_core_d->pm_data;
    oal_reinit_completion(pm_data->dev_bootok_ack_comp);

    if (firmware_download_function(BFGX_CFG) != EXCEPTION_SUCCESS) {
        hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);
        ps_print_err("bfgx power reset failed!\n");
        return -EXCEPTION_FAIL;
    }
    hcc_bus_disable_state(pm_top_data->wlan_pm_info->pst_bus, OAL_BUS_STATE_ALL);

    if (wlan_is_shutdown()) {
        hi1103_wifi_disable();
    }

    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        ps_print_err("wait bfgx boot ok timeout\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BCPU_PWR_RST_BOOTUP);

        return -EXCEPTION_FAIL;
    }

    bfgx_pm_feature_set(pm_data);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : bfgx_system_reset
 * 功能描述  : bfgin全系统复位函数，对device重新上电，wifi和bfgin firmware重新
 *            加载，恢复wifi，恢复bfgin业务。
 * 返 回 值  : 成功返回0，否则返回-1
 */
int32_t bfgx_system_reset(void)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (!wlan_is_shutdown()) {
        if (pst_exception_data->wifi_callback->notify_wifi_to_recovery != NULL) {
            ps_print_info("notify wifi bfgx start to power reset\n");
            pst_exception_data->wifi_callback->notify_wifi_to_recovery();
        }
    }

    /* 重新上电，firmware重新加载 */
    if (bfgx_power_reset() != EXCEPTION_SUCCESS) {
        ps_print_err("bfgx power reset failed!\n");
        return -EXCEPTION_FAIL;
    }

    if (bfgx_status_recovery() != EXCEPTION_SUCCESS) {
        ps_print_err("bfgx status revocery failed!\n");

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_RESET_BFGX_RECOVERY);
        return -EXCEPTION_FAIL;
    }

    if (!wlan_is_shutdown()) {
        if (wifi_status_recovery() != EXCEPTION_SUCCESS) {
            ps_print_err("wifi status revocery failed!\n");

            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_RESET_WIFI_RECOVERY);

            return -EXCEPTION_FAIL;
        }
        plat_dfr_sysrst_type_cnt_inc(DFR_ALL_SYS_RST, SUBSYS_BFGX);
    } else {
        plat_dfr_sysrst_type_cnt_inc(DFR_SINGLE_SYS_RST, SUBSYS_BFGX);
    }

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_RST, CHR_PLAT_DRV_ERROR_BFGX_SYSTEM_DFR_SUCC);

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : bfgx_dump_stack
 * 功能描述  : 心跳超时时，尝试通过uart读栈，不保证一定能成功，因为此时uart可能不通
 */
OAL_STATIC void bfgx_dump_stack(void)
{
    unsigned long timeleft;
    uint32_t exception_type;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return;
    }

    exception_type = pst_exception_data->excetion_type;

    if (exception_type != BFGX_LASTWORD_PANIC) {
        return;
    }

    if (unlikely(pst_exception_data->ps_plat_d == NULL)) {
        ps_print_err("pst_exception_data->ps_plat_d is NULL\n");
        return;
    }
    ps_core_d = pst_exception_data->ps_plat_d->core_data;

    oal_reinit_completion(pst_exception_data->wait_read_bfgx_stack);

    /* 等待读栈操作完成 */
    timeleft = wait_for_completion_timeout(&pst_exception_data->wait_read_bfgx_stack,
                                           msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d);
        atomic_set(&pst_exception_data->is_memdump_runing, 0);
        ps_print_err("read bfgx stack failed!\n");
    } else {
        ps_print_info("read bfgx stack success!\n");
    }

    return;
}

OAL_STATIC int32_t bfgx_reset_cmd_send(uint32_t subsys)
{
    int32_t ret;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EINVAL;
    }

    pst_bfgx_data = &ps_core_d->bfgx_info[subsys];

    ret = ps_push_skb_queue(ps_core_d, g_bfgx_reset_msg[subsys].cmd,
                            g_bfgx_reset_msg[subsys].len,
                            g_bfgx_rx_queue[subsys]);
    if (ret != EXCEPTION_SUCCESS) {
        ps_print_err("push %s reset cmd to skb fail\n", g_bfgx_subsys_name[subsys]);
        return -EXCEPTION_FAIL;
    }

    oal_wait_queue_wake_up_interrupt(&pst_bfgx_data->rx_wait);

    return EXCEPTION_SUCCESS;
}

#ifdef _PRE_H2D_GPIO_WKUP
OAL_STATIC int32_t bfgx_gpio_wkup_mode_recovery(struct ps_core_s *ps_core_d)
{
    struct pm_drv_data *pm_data = NULL;
    int32_t ret;

    pm_data = ps_core_d->pm_data;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    atomic_set(&pm_data->cfg_wkup_dev_flag, 0);
    ret = host_send_cfg_gpio_wkup_dev_msg(pm_data);
    if (ret != SUCCESS) {
        ps_print_err("bfgx_gpio_wkup_mode_recovery fail.\n");
        return -EXCEPTION_FAIL;
    }

    return EXCEPTION_SUCCESS;
}
#endif
/*
 * 函 数 名  : bfgx_status_recovery
 * 功能描述  : 全系统复位以后，恢复bfgin业务函数
 */
OAL_STATIC int32_t bfgx_status_recovery(void)
{
    uint32_t i;
    struct ps_core_s *ps_core_d = NULL;
    struct st_bfgx_data *pst_bfgx_data = NULL;

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -EXCEPTION_FAIL;
    }
#ifdef _PRE_H2D_GPIO_WKUP
    if (bfgx_gpio_wkup_mode_recovery(ps_core_d) != EXCEPTION_SUCCESS) {
        return -EXCEPTION_FAIL;
    }
#endif
    if (prepare_to_visit_node(ps_core_d) != EXCEPTION_SUCCESS) {
        ps_print_err("prepare work FAIL\n");
        return -EXCEPTION_FAIL;
    }

    for (i = 0; i < BFGX_BUTT; i++) {
        pst_bfgx_data = &ps_core_d->bfgx_info[i];
        if (atomic_read(&pst_bfgx_data->subsys_state) == POWER_STATE_SHUTDOWN) {
            continue;
        }

        ps_kfree_skb(ps_core_d, g_bfgx_rx_queue[i]);

        if (bfgx_open_cmd_send(i, BUART) != EXCEPTION_SUCCESS) {
            ps_print_err("bfgx open cmd fail\n");
            post_to_visit_node(ps_core_d);
            return -EXCEPTION_FAIL;
        }

        if (bfgx_reset_cmd_send(i) != EXCEPTION_SUCCESS) {
            ps_print_err("bfgx reset cmd send fail\n");
            post_to_visit_node(ps_core_d);
            return -EXCEPTION_FAIL;
        }
    }

    post_to_visit_node(ps_core_d);

    return EXCEPTION_SUCCESS;
}

int32_t wait_bfgx_memdump_complete(void)
{
    unsigned long timeleft;
    struct st_exception_info *pst_exception_data = NULL;

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (atomic_read(&pst_exception_data->is_memdump_runing) == 0) {
        return EXCEPTION_SUCCESS;
    }

    oal_reinit_completion(pst_exception_data->wait_read_bfgx_stack);
    /* wait for read stack completion */
    timeleft = wait_for_completion_timeout(&pst_exception_data->wait_read_bfgx_stack,
                                           msecs_to_jiffies(WAIT_BFGX_READ_STACK_TIME));
    if (!timeleft) {
        atomic_set(&pst_exception_data->is_memdump_runing, 0);
        ps_print_err("read bfgx stack failed!\n");\
        return -EXCEPTION_FAIL;
    }

    ps_print_info("read bfgx stack success!\n");
    return EXCEPTION_SUCCESS;
}

int32_t is_bfgx_exception(void)
{
    int32_t is_exception;
    unsigned long flag;

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return 0;
    }

    oal_spin_lock_irq_save(&pst_exception_data->exception_spin_lock, &flag);
    if (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_BUSY) {
        is_exception = PLAT_EXCEPTION_RESET_BUSY;
    } else {
        is_exception = PLAT_EXCEPTION_RESET_IDLE;
    }
    oal_spin_unlock_irq_restore(&pst_exception_data->exception_spin_lock, &flag);

    return is_exception;
}

int32_t plat_bfgx_exception_rst_register(struct ps_plat_s *data)
{
    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (data == NULL) {
        ps_print_err("para data is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    pst_exception_data->ps_plat_d = data;

    return EXCEPTION_SUCCESS;
}

int32_t plat_wifi_exception_rst_register(void *data)
{
    struct st_wifi_dfr_callback *pst_wifi_callback = NULL;

    struct st_exception_info *pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return -EXCEPTION_FAIL;
    }

    if (data == NULL) {
        ps_print_err("param data is null\n");
        return -EXCEPTION_FAIL;
    }

    /* wifi异常回调函数注册 */
    pst_wifi_callback = (struct st_wifi_dfr_callback *)data;
    pst_exception_data->wifi_callback = pst_wifi_callback;

    return EXCEPTION_SUCCESS;
}

EXPORT_SYMBOL_GPL(plat_wifi_exception_rst_register);

OAL_STATIC void plat_exception_dump_file_rotate_init(void)
{
    oal_wait_queue_init_head(&g_bcpu_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_bcpu_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_bcpu_memdump_cfg.quenue);
    oal_wait_queue_init_head(&g_wcpu_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_wcpu_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_wcpu_memdump_cfg.quenue);
    oal_wait_queue_init_head(&g_gcpu_memdump_cfg.dump_type_wait);
    skb_queue_head_init(&g_gcpu_memdump_cfg.dump_type_queue);
    skb_queue_head_init(&g_gcpu_memdump_cfg.quenue);
    ps_print_info("plat exception dump file rotate init success\n");
}

/*
 * 函 数 名  : plat_exception_reset_init
 * 功能描述  : 平台异常处理模块初始化函数
 */
int32_t plat_exception_reset_init(void)
{
    struct st_exception_info *p_exception_data = NULL;

    p_exception_data = (struct st_exception_info *)kzalloc(sizeof(struct st_exception_info), GFP_KERNEL);
    if (p_exception_data == NULL) {
        ps_print_err("kzalloc p_exception_data is failed!\n");
        return -EXCEPTION_FAIL;
    }

    p_exception_data->wifi_callback = NULL;

    p_exception_data->subsys_type = SUBSYS_BOTTOM;
    p_exception_data->thread_type = BFGX_THREAD_BOTTOM;
    p_exception_data->excetion_type = EXCEPTION_TYPE_BOTTOM;

    p_exception_data->exception_reset_enable = PLAT_EXCEPTION_DISABLE;
    p_exception_data->subsystem_rst_en = DFR_TEST_DISABLE;

    p_exception_data->ps_plat_d = NULL;

    atomic_set(&p_exception_data->bfgx_beat_flag, BFGX_NOT_RECV_BEAT_INFO);
    atomic_set(&p_exception_data->is_reseting_device, PLAT_EXCEPTION_RESET_IDLE);

    /* 初始化异常处理workqueue和work */
    p_exception_data->plat_exception_rst_workqueue = create_singlethread_workqueue("plat_exception_reset_queue");
    if (p_exception_data->plat_exception_rst_workqueue == NULL) {
        kfree(p_exception_data);
        ps_print_err("create plat_exception workqueue failed\n");
        return -EINTR;
    }

    INIT_WORK(&p_exception_data->plat_exception_rst_work, plat_exception_reset_work);
    oal_wake_lock_init(&p_exception_data->plat_exception_rst_wlock, "hi11xx_excep_rst_wlock");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0))
    /* 初始化心跳timer */
    init_timer(&p_exception_data->bfgx_beat_timer);
    p_exception_data->bfgx_beat_timer.function = bfgx_beat_timer_expire;
    p_exception_data->bfgx_beat_timer.data = 0;
#else
    timer_setup(&p_exception_data->bfgx_beat_timer, bfgx_beat_timer_expire, 0);
#endif
    p_exception_data->bfgx_beat_timer.expires = jiffies + BFGX_BEAT_TIME * HZ;

    /* 初始化异常处理自旋锁 */
    oal_spin_lock_init(&p_exception_data->exception_spin_lock);

    /* 初始化bfgx读栈完成量 */
    init_completion(&p_exception_data->wait_read_bfgx_stack);

    /* 调试使用的变量初始化 */
    p_exception_data->debug_beat_flag = 1;
    atomic_set(&p_exception_data->is_memdump_runing, 0);

    g_hi110x_exception_info = p_exception_data;

    /* 初始化dump文件轮替模块 */
    plat_exception_dump_file_rotate_init();

    hisi_conn_rdr_init();

    ps_print_suc("plat exception reset init success\n");

    return EXCEPTION_SUCCESS;
}

/*
 * 函 数 名  : plat_exception_reset_exit
 * 功能描述  : 平台异常处理模块退出清理函数
 */
int32_t plat_exception_reset_exit(void)
{
    struct st_exception_info *p_exception_data = NULL;

    p_exception_data = g_hi110x_exception_info;
    if (p_exception_data == NULL) {
        ps_print_err("g_hi110x_exception_info is NULL!\n");
        return -EXCEPTION_FAIL;
    }

    destroy_workqueue(p_exception_data->plat_exception_rst_workqueue);
    del_timer_sync(&p_exception_data->bfgx_beat_timer);

    oal_wake_lock_exit(&p_exception_data->plat_exception_rst_wlock);

    kfree(p_exception_data);
    g_hi110x_exception_info = NULL;

    ps_print_suc("plat exception reset exit success\n");

    hisi_conn_rdr_exit();

    return EXCEPTION_SUCCESS;
}

#ifdef PLATFORM_DEBUG_ENABLE
int32_t is_dfr_test_en(enum excp_test_cfg_em excp_cfg)
{
    if (excp_cfg >= EXCP_TEST_CFG_BOTT) {
        return DFR_TEST_DISABLE;
    }

    if (g_excp_test_cfg[excp_cfg] == DFR_TEST_ENABLE) {
        g_excp_test_cfg[excp_cfg] = DFR_TEST_DISABLE;
        return DFR_TEST_ENABLE;
    }
    return DFR_TEST_DISABLE;
}

void set_excp_test_en(enum excp_test_cfg_em excp_cfg)
{
    if (excp_cfg >= EXCP_TEST_CFG_BOTT) {
        return;
    }

    g_excp_test_cfg[excp_cfg] = DFR_TEST_ENABLE;
}
#endif

OAL_STATIC void excp_memdump_quenue_clear(memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;
    while ((skb = skb_dequeue(&memdump_t->quenue)) != NULL) {
        kfree_skb(skb);
    }
}

void bfgx_memdump_finish(struct ps_core_s *ps_core_d)
{
    uint8_t index = ps_core_d->dev_index;

    if (index == BUART) {
        g_bcpu_memdump_cfg.is_working = 0;
    } else if (index == GUART) {
        g_gcpu_memdump_cfg.is_working = 0;
    } else {
        ps_print_err("unknown dev index:%d\n", index);
    }
}

void wifi_memdump_finish(void)
{
    g_wcpu_memdump_cfg.is_working = 0;
}

/*
 * Prototype    : plat_excp_send_rotate_cmd_2_app
 * Description  : driver send rotate cmd to app for rotate file
 */
OAL_STATIC void plat_excp_send_rotate_cmd_2_app(uint32_t which_dump, memdump_info_t *memdump_info)
{
    struct sk_buff *skb = NULL;

    if (which_dump >= CMD_DUMP_BUFF) {
        ps_print_warning("which dump:%d error\n", which_dump);
        return;
    }
    if (skb_queue_len(&memdump_info->dump_type_queue) > MEMDUMP_ROTATE_QUEUE_MAX_LEN) {
        ps_print_warning("too many dump type in queue,dispose type:%d", which_dump);
        return;
    }

    skb = alloc_skb(sizeof(which_dump), (oal_in_interrupt() || oal_in_atomic()) ?
                    GFP_ATOMIC : GFP_KERNEL);
    if (skb == NULL) {
        ps_print_err("alloc errno skbuff failed! len=%d, errno=%x\n", (int32_t)sizeof(which_dump), which_dump);
        return;
    }
    skb_put(skb, sizeof(which_dump));
    *(uint32_t *)skb->data = which_dump;
    skb_queue_tail(&memdump_info->dump_type_queue, skb);
    ps_print_info("save rotate cmd [%d] in queue\n", which_dump);

    oal_wait_queue_wake_up_interrupt(&memdump_info->dump_type_wait);
}

int32_t notice_hal_memdump(memdump_info_t *memdump_t, uint32_t which_dump)
{
    PS_PRINT_FUNCTION_NAME;
    if (memdump_t->is_working) {
        ps_print_err("is doing memdump\n");
        return -1;
    }
    excp_memdump_quenue_clear(memdump_t);
    plat_excp_send_rotate_cmd_2_app(which_dump, memdump_t);
    memdump_t->is_working = 1;
    return 0;
}

int32_t bfgx_notice_hal_memdump(struct ps_core_s *ps_core_d)
{
    uint8_t index = ps_core_d->dev_index;

    if (index == BUART) {
        return notice_hal_memdump(&g_bcpu_memdump_cfg, CMD_READM_BFGX_UART);
    } else if (index == GUART) {
        return notice_hal_memdump(&g_gcpu_memdump_cfg, CMD_READM_ME_UART);
    } else {
        ps_print_err("unknown dev index:%d\n", index);
        return -EINVAL;
    }
}

int32_t wifi_notice_hal_memdump(void)
{
    return notice_hal_memdump(&g_wcpu_memdump_cfg, CMD_READM_WIFI_SDIO);
}

OAL_STATIC int32_t excp_memdump_queue(uint8_t *buf_ptr, uint16_t count, memdump_info_t *memdump_t)
{
    struct sk_buff *skb = NULL;

    ps_print_dbg("[send] len:%d\n", count);
    if (!memdump_t->is_working) {
        ps_print_err("excp_memdump_queue not allow\n");
        return -EINVAL;
    }
    if (buf_ptr == NULL) {
        ps_print_err("buf_ptr is NULL\n");
        return -EINVAL;
    }
    if (in_atomic() || in_softirq() || in_interrupt() || irqs_disabled()) {
        skb = alloc_skb(count, GFP_ATOMIC);
    } else {
        skb = alloc_skb(count, GFP_KERNEL);
    }

    if (skb == NULL) {
        ps_print_err("can't allocate mem for new debug skb, len=%d\n", count);
        return -EINVAL;
    }

    if (memcpy_s(skb_tail_pointer(skb), count, buf_ptr, count) != EOK) {
        ps_print_err("memcpy_s failed\n");
    }
    skb_put(skb, count);
    skb_queue_tail(&memdump_t->quenue, skb);
    ps_print_dbg("[excp_memdump_queue]qlen:%d,count:%d\n", memdump_t->quenue.qlen, count);
    return 0;
}

int32_t bfgx_memdump_enquenue(struct ps_core_s *ps_core_d, uint8_t *buf_ptr, uint16_t count)
{
    uint8_t index = ps_core_d->dev_index;

    if (index == BUART) {
        return excp_memdump_queue(buf_ptr, count, &g_bcpu_memdump_cfg);
    } else if (index == GUART) {
        return excp_memdump_queue(buf_ptr, count, &g_gcpu_memdump_cfg);
    } else {
        ps_print_err("unknown dev index:%d\n", index);
        return -EINVAL;
    }
}

int32_t wifi_memdump_enquenue(uint8_t *buf_ptr, uint16_t count)
{
    return excp_memdump_queue(buf_ptr, count, &g_wcpu_memdump_cfg);
}

