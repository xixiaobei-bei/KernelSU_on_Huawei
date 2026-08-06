

/* Header File Including */
#include <linux/module.h> /* kernel module definitions */
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/ktime.h>
#include <linux/timer.h>
#include <linux/platform_device.h>
#include <linux/kobject.h>
#include <linux/irq.h>
#include <linux/mutex.h>
#include <linux/kernel.h>

#include <linux/mmc/sdio.h>
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <linux/gpio.h>
#include <linux/tty.h>
#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>

#include "board.h"
#include "hw_bfg_ps.h"
#include "plat_type.h"
#include "plat_debug.h"
#include "plat_uart.h"
#include "plat_firmware.h"
#include "plat_pm.h"
#include "plat_exception_rst.h"
#include "plat_pm.h"
#include "securec.h"
#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oal_schedule.h"
#include "plat_firmware.h"
#include "bfgx_exception_rst.h"
#include "oam_ext_if.h"
#include "lpcpu_feature.h"

/*****************************************************************************
  2 Global Variable Definition
*****************************************************************************/
#ifdef CONFIG_HUAWEI_DSM

    static struct dsm_dev g_dsm_bt = {
        .name = "dsm_bt",
        .device_name = NULL,
        .ic_name = NULL,
        .module_name = NULL,
        .fops = NULL,
        .buff_size = DSM_DEV_BUFF_SIZE,
    };

    struct dsm_client *g_hw_1102a_bt_dsm_client = NULL;

    void hw_1102a_register_bt_dsm_client(void)
    {
        if (g_hw_1102a_bt_dsm_client == NULL) {
            g_hw_1102a_bt_dsm_client = dsm_register_client(&g_dsm_bt);
        }
    }
    void hw_1102a_unregister_bt_dsm_client(void)
    {
        if (g_hw_1102a_bt_dsm_client != NULL) {
            dsm_unregister_client(g_hw_1102a_bt_dsm_client, &g_dsm_bt);
            g_hw_1102a_bt_dsm_client = NULL;
        }
    }
#define LOG_BUF_SIZE 512
    void hw_1102a_bt_dsm_client_notify(int dsm_id, const char *fmt, ...)
    {
        char buf[LOG_BUF_SIZE] = {0};
        va_list ap;

        declare_dft_trace_key_info("hw_1102a_bt_dsm_client_notify", OAL_DFT_TRACE_FAIL);
        va_start(ap, fmt);
        if (g_hw_1102a_bt_dsm_client && !dsm_client_ocuppy(g_hw_1102a_bt_dsm_client)) {
            if (fmt != NULL) {
                if (vsnprintf_s(buf, LOG_BUF_SIZE, LOG_BUF_SIZE - 1, fmt, ap) > 0) {
                    dsm_client_record(g_hw_1102a_bt_dsm_client, buf);
                }
            }
            dsm_client_notify(g_hw_1102a_bt_dsm_client, dsm_id);
            OAL_IO_PRINT("[I]bt dsm_client_notify success,dsm_id=%d[%s]\n", dsm_id, buf);
        } else {
            OAL_IO_PRINT("[E]bt dsm_client_notify failed,dsm_id=%d\n", dsm_id);
        }
        va_end(ap);
    }
    EXPORT_SYMBOL(hw_1102a_bt_dsm_client_notify);
#endif

/*
 * Function: pf_suspend_notify
 * Description: suspend notify call back
 * Ruturn: 0 -- success
 */
static int pf_suspend_notify(struct notifier_block *notify_block,
                             unsigned long mode, void *unused)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return IRQ_NONE;
    }

    switch (mode) {
        case PM_POST_SUSPEND:
            ps_print_info("host resume OK!\n");
            break;
        case PM_SUSPEND_PREPARE:
            ps_print_info("host suspend now!\n");
            break;
        default:
            break;
    }
    return 0;
}

static struct notifier_block g_pf_suspend_notifier = {
    .notifier_call = pf_suspend_notify,
    .priority = INT_MIN,
};

struct pm_drv_data *g_pm_drv_data_t = NULL;

struct pm_drv_data *pm_get_drvdata(void)
{
    return g_pm_drv_data_t;
}

static void pm_set_drvdata(struct pm_drv_data *data)
{
    g_pm_drv_data_t = data;
}

/*
 * Prototype    : check_bfg_state
 * Description  : check whether bfg is sleep or not
 * Return       : 0  -  sleep
 *                1  -  active
 *               -1 -  pm_data is null
 */
int32 check_bfg_state(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return pm_data->bfgx_dev_state;
}

STATIC void host_allow_devslp_in_node(struct ps_core_s *ps_core_d)
{
    /* make "host_allow_bfg_sleep()" happy */
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is null\n");
        return;
    }

    atomic_dec(&ps_core_d->node_visit_flag);
    if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
        ps_print_info("queue_work send_allow_sleep_work not return true\n");
    }
    /* recovery the original value */
    atomic_inc(&ps_core_d->node_visit_flag);
}

void bfgx_state_set(uint8 on)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }
    ps_print_warning("bfgx_state_set:%d --> %d\n", pm_data->bfgx_dev_state, on);
    pm_data->bfgx_dev_state = on;
}

int32 bfgx_state_get(void)
{
    return check_bfg_state();
}

STATIC void bfgx_uart_state_set(uint8 uart_state)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_warning("bfgx_uart_state_set:%d-->%d", pm_data->uart_state, uart_state);
    pm_data->uart_state = uart_state;
}

int8 bfgx_uart_state_get(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -1;
    }

    return pm_data->uart_state;
}

int32 bfgx_uart_get_baud_rate(void)
{
    struct ps_plat_s *ps_plat_d = NULL;

    ps_get_plat_reference(&ps_plat_d);
    if (unlikely(ps_plat_d == NULL)) {
        ps_print_err("ps_plat_d is NULL\n");
        return -EINVAL;
    }

    return ps_plat_d->baud_rate;
}

void bfgx_uart_baud_change_work(struct work_struct *work)
{
    int ret;
    uint32 wait_cnt = 0;
    const uint32 ul_max_wait_cnt = 10000;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct ps_core_s *ps_core_d = NULL;
    uint64 flags;

    ps_print_info("%s\n", __func__);

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core has not inited\n");
        return;
    }

    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    bfgx_uart_state_set(UART_BPS_CHG_SEND_ACK);
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    /* 切波特率时，此消息是host发送的最后一个UART数据 */
    ps_tx_urgent_cmd(ps_core_d, SYS_MSG, PL_BAUT_CHG_REQ_ACK);

    while (pm_data->uart_state != UART_BPS_CHG_IN_PROGRESS) {
        oal_udelay(200);
        wait_cnt++;
        if (wait_cnt >= ul_max_wait_cnt) {
            ps_print_err("wait device start baud change timeout\n");
            spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
            bfgx_uart_state_set(UART_READY);
            spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

            return;
        }
    }

    ret = ps_change_uart_baud_rate(pm_data->uart_baud_switch_to, FLOW_CTRL_ENABLE);
    if (ret != 0) {
        ps_print_err("It is bad!!!, change uart rate fail\n");
    }

    /* 等待device完成切换，pull down GPIO */
    while (board_get_bwkup_gpio_val() == 1) {
        oal_udelay(200);
        wait_cnt++;
        if (wait_cnt >= ul_max_wait_cnt) {
            ps_print_err("wait device bps change complete && pull down gpio fail\n");
            return;
        }
    }

    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    bfgx_uart_state_set(UART_BPS_CHG_SEND_COMPLETE);
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    /* 切波特率完成后的第一个握手消息 */
    ps_tx_urgent_cmd(ps_core_d, SYS_MSG, PL_BAUT_CHG_COMPLETE);

    mod_timer(&pm_data->baud_change_timer, jiffies + msecs_to_jiffies(100));

    return;
}

int32 bfgx_uart_rcv_baud_change_req(uint8 uc_msg_type)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    int32 cur_rate;

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_print_info("%s\n", __func__);

    cur_rate = bfgx_uart_get_baud_rate();
    if (((uc_msg_type == SYS_INF_BAUD_CHG_2M_REQ) && (cur_rate == HIGH_FREQ_BAUD_RATE)) ||
        ((uc_msg_type == SYS_INF_BAUD_CHG_6M_REQ) && (cur_rate == LOW_FREQ_BAUD_RATE))) {
        if (uc_msg_type == SYS_INF_BAUD_CHG_6M_REQ) {
            pm_data->uart_baud_switch_to = HIGH_FREQ_BAUD_RATE;
        } else {
            pm_data->uart_baud_switch_to = LOW_FREQ_BAUD_RATE;
        }
        queue_work(pm_data->wkup_dev_workqueue, &pm_data->baud_change_work);
    } else {
        ps_print_err("It is bad!!!, req = 0x%x,HIGH_FREQ_BAUD_RATE=%d,current = 0x%x\n",
                     HIGH_FREQ_BAUD_RATE, uc_msg_type, cur_rate);
    }

    return 0;
}

int32 bfgx_uart_rcv_baud_change_complete_ack(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct ps_core_s *ps_core_d = NULL;

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EINVAL;
    }

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core has not inited\n");
        return -EINVAL;
    }

    ps_print_info("%s\n", __func__);
    del_timer_sync(&pm_data->baud_change_timer);

    /* restart the tx work */
    queue_work(ps_core_d->ps_tx_workqueue, &ps_core_d->tx_skb_work);

    return 0;
}

void bfgx_uart_baud_change_expire(uint64 data)
{
    uint64 flags;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = (struct pm_drv_data *)(uintptr_t)data;

    if (unlikely(pm_data == NULL)) {
        ps_print_err("devack timer para is null\n");
        return;
    }

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core has not inited\n");
        return;
    }

    ps_print_info("%s\n", __func__);
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    if (pm_data->uart_state == UART_BPS_CHG_SEND_COMPLETE) {
        ps_tx_urgent_cmd(ps_core_d, SYS_MSG, PL_BAUT_CHG_COMPLETE);
        mod_timer(&pm_data->baud_change_timer, jiffies + msecs_to_jiffies(100));
    }
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
}

int32 bfgx_pm_feature_set(void)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -FAILURE;
    }

    if (pm_data->bfgx_pm_ctrl_enable == BFGX_PM_DISABLE) {
        ps_print_info("bfgx platform pm ctrl disable\n");
        msleep(50);
        return SUCCESS;
    }

    if (pm_data->bfgx_lowpower_enable == BFGX_PM_ENABLE) {
        ps_print_info("bfgx platform pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_PL_ENABLE_PM);
    } else {
        ps_print_info("bfgx platform pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_PL_DISABLE_PM);
    }

    if (pm_data->bfgx_bt_lowpower_enable == BFGX_PM_ENABLE) {
        ps_print_info("bfgx bt pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_BT_ENABLE_PM);
    } else {
        ps_print_info("bfgx bt pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_BT_DISABLE_PM);
    }

    if (pm_data->bfgx_gnss_lowpower_enable == BFGX_PM_ENABLE) {
        ps_print_info("bfgx gnss pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_GNSS_ENABLE_PM);
    } else {
        ps_print_info("bfgx gnss pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_GNSS_DISABLE_PM);
    }

    if (pm_data->bfgx_nfc_lowpower_enable == BFGX_PM_ENABLE) {
        ps_print_info("bfgx nfc pm enable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_NFC_ENABLE_PM);
    } else {
        ps_print_info("bfgx nfc pm disable\n");
        ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_NFC_DISABLE_PM);
    }

    msleep(20);

    return SUCCESS;
}

/*
 * Prototype    : bfg_wake_lock
 * Description  : control bfg wake lock
 */
void bfg_wake_lock(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    oal_wakelock_stru *pst_bfg_wake_lock;
    oal_ulong ul_flags;
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    pst_bfg_wake_lock = &pm_data->bfg_wake_lock;

    oal_spin_lock_irq_save(&pst_bfg_wake_lock->lock, &ul_flags);
    if (oal_wakelock_active(pst_bfg_wake_lock) == 0) {
        __pm_stay_awake(&pst_bfg_wake_lock->st_wakelock);
        pst_bfg_wake_lock->locked_addr = (uintptr_t)_RET_IP_;
        pst_bfg_wake_lock->lock_count++;
        if (oal_unlikely(pst_bfg_wake_lock->debug)) {
            printk(KERN_INFO "wakelock[%s] lockcnt:%lu <==%pf\n",
                   pst_bfg_wake_lock->st_wakelock.name, pst_bfg_wake_lock->lock_count, (oal_void *)_RET_IP_);
        }

        gps_ilde_sleep_vote(1);
        ps_print_info("bfg_wakelock active[%d],cnt %lu\n",
                      oal_wakelock_active(pst_bfg_wake_lock), pst_bfg_wake_lock->lock_count);
    }
    oal_spin_unlock_irq_restore(&pst_bfg_wake_lock->lock, &ul_flags);
#endif

    return;
}

void bfg_wake_unlock(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    oal_wakelock_stru *pst_bfg_wake_lock;
    oal_ulong ul_flags;
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    pst_bfg_wake_lock = &pm_data->bfg_wake_lock;

    oal_spin_lock_irq_save(&pst_bfg_wake_lock->lock, &ul_flags);

    if (oal_wakelock_active(pst_bfg_wake_lock)) {
        pst_bfg_wake_lock->lock_count--;
        __pm_relax(&pst_bfg_wake_lock->st_wakelock);
        pst_bfg_wake_lock->locked_addr = (oal_ulong)0x0;

        if (oal_unlikely(pst_bfg_wake_lock->debug)) {
            printk(KERN_INFO "wakeunlock[%s] lockcnt:%lu <==%pf\n",
                   pst_bfg_wake_lock->st_wakelock.name, pst_bfg_wake_lock->lock_count, (oal_void *)_RET_IP_);
        }

        gps_ilde_sleep_vote(0);
        ps_print_info("bfg_wakelock active[%d], cnt %lu\n",
                      oal_wakelock_active(pst_bfg_wake_lock), pst_bfg_wake_lock->lock_count);
    } else {
        ps_print_info("bfg_wakelock not active,cnt %lu\n", pst_bfg_wake_lock->lock_count);
    }
    oal_spin_unlock_irq_restore(&pst_bfg_wake_lock->lock, &ul_flags);
#endif

    return;
}

void ssi_dump_for_host_wkup_dev_work_only(unsigned long long module_set)
{
    if (hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_USER) {
        if (!oal_print_rate_limit(PRINT_RATE_MINUTE)) { /* 1分钟打印一次 */
            module_set = 0x0;
        }
    }
    ssi_dump_device_regs(module_set);
}

static inline int32 ps_change_baud_rate_retry(int64 baud_rate, uint8 flowctl_status)
{
    const uint32 set_baud_retry = 3;
    int ret = OAL_TRUE;
    uint32 j = set_baud_retry;
    while (ps_change_uart_baud_rate(baud_rate, flowctl_status) != 0) {
        ps_print_warning("change uart rate fail,left retry cnt:%d,do retry\n", j);
        declare_dft_trace_key_info("change uart rate fail", OAL_DFT_TRACE_FAIL);
        if (--j) {
            msleep(100);
        } else {
            ps_print_err("change uart rate %ld fail,retried %u but not succ\n", baud_rate, set_baud_retry);
            ret = OAL_FALSE;
            break;
        }
    }

    return ret;
}

static int32 process_host_wkup_dev_fail(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data)
{
    uint64 flags;
    int bwkup_gpio_val;

    if (!OAL_IS_ERR_OR_NULL(ps_core_d->tty) && tty_chars_in_buffer(ps_core_d->tty)) {
        ps_print_info("tty tx buf is not empty\n");
    }

    bwkup_gpio_val = board_get_bwkup_gpio_val();
    ps_print_info("bfg still NOT wkup, gpio level:%d\n", bwkup_gpio_val);

    if (bwkup_gpio_val == 0) {
        return OAL_FALSE;
    } else {
        ps_print_info("bfg wakeup ack lost, complete it\n");
        spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
        bfg_wake_lock();
        bfgx_state_set(BFGX_ACTIVE);
        complete(&pm_data->dev_ack_comp);
        spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);

        // set default baudrate. should not try again if failed, return succ
        (void)ps_change_baud_rate_retry(pm_data->uart_baud_switch_to, FLOW_CTRL_ENABLE);
        queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);
        return OAL_TRUE;
    }
    return OAL_FALSE;
}

static oal_int32 host_wkup_dev_via_uart(struct ps_core_s *ps_core_d, struct pm_drv_data *pm_data)
{
    const uint32 wkup_retry = 3;
    int32 ret;
    uint64 timeleft;
    uint8 zero_num = 0;
    int bwkup_gpio_val = 0;
    uint32 i;

    /* prepare baudrate */
    ret = ps_change_baud_rate_retry(WKUP_DEV_BAUD_RATE, FLOW_CTRL_DISABLE);
    if (ret != OAL_TRUE) {
        return OAL_FALSE;
    }

    for (i = 0; i < wkup_retry; i++) {
        bwkup_gpio_val = board_get_bwkup_gpio_val();
        ps_print_info("bfg wakeup dev,try %u,cur gpio level:%u\n", i + 1, bwkup_gpio_val);
        /* uart write long zero to wake up device */
        ps_write_tty(ps_core_d, &zero_num, sizeof(uint8));

        timeleft = wait_for_completion_timeout(&pm_data->dev_ack_comp,
                                               msecs_to_jiffies(WAIT_WKUP_DEVACK_TIMEOUT_MSEC));
        if (timeleft || (bfgx_state_get() == BFGX_ACTIVE)) {
            bwkup_gpio_val = board_get_bwkup_gpio_val();
            ps_print_info("bfg wkup OK, gpio level:%d\n", bwkup_gpio_val);

            // set default baudrate. should not try again if failed, return succ
            (void)ps_change_baud_rate_retry(pm_data->uart_baud_switch_to, FLOW_CTRL_ENABLE);
            return OAL_TRUE;
        } else {
            ret = process_host_wkup_dev_fail(ps_core_d, pm_data);
            if (ret == OAL_TRUE) {
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}

void host_wkup_dev_work(struct work_struct *work)
{
    uint64 timeleft;
    int32 ret;
    struct ps_core_s *ps_core_d = NULL;

    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("%s,dev:%d,uart:%d\n", __func__, bfgx_state_get(), bfgx_uart_get_baud_rate());

    ret = ps_get_core_reference(&ps_core_d);
    if ((ret != SUCCESS) || (ps_core_d == NULL)) {
        ps_print_err("%s,ps_core_d is null!\n", __func__);
        return;
    }

    if (is_tty_open(ps_core_d->pm_data)) {
        ps_print_err("%s,tty is closed skip!\n", __func__);
        return;
    }
    /*
     * if B send work item of wkup_dev before A's work item finished, then
     * B should not do actual wkup operation.
     */
    if (bfgx_state_get() == BFGX_ACTIVE) {
        if (waitqueue_active(&pm_data->host_wkup_dev_comp.wait)) {
            ps_print_info("it seems like dev ack with NoSleep\n");
            complete_all(&pm_data->host_wkup_dev_comp);
        } else { /* 目前用了一把host_mutex大锁，这种case不应存在，但低功耗模块不应依赖外部 */
            ps_print_dbg("B do wkup_dev work item after A do it but not finished\n");
        }
        return;
    }

    /* prepare to wake up device */
    ps_uart_state_pre(ps_core_d->tty);
    timeleft = wait_for_completion_timeout(&pm_data->dev_ack_comp, msecs_to_jiffies(WAIT_DEVACK_TIMEOUT_MSEC));
    if (!timeleft) {
        unsigned long long module_set = SSI_MODULE_MASK_COMM;
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait dev allow slp ack timeout\n");
        declare_dft_trace_key_info("wait dev allow slp ack timeout", OAL_DFT_TRACE_FAIL);
        ssi_dump_for_host_wkup_dev_work_only(module_set);

        return;
    }

    oal_reinit_completion(pm_data->dev_ack_comp);

    /* device doesn't agree to slp */
    if (bfgx_state_get() == BFGX_ACTIVE) {
        complete_all(&pm_data->host_wkup_dev_comp);
        ps_print_dbg("we know dev ack with NoSleep\n");
        return;
    }

    /* begin to wake up device via uart rxd */
    ret = host_wkup_dev_via_uart(ps_core_d, pm_data);
    if (ret != OAL_TRUE) {
        unsigned long long module_set = SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_BCTRL;
        ps_change_uart_baud_rate(pm_data->uart_baud_switch_to, FLOW_CTRL_ENABLE);
        ps_print_info("host wkup bfg fail\n");
        ssi_dump_for_host_wkup_dev_work_only(module_set);
    }
}

STATIC int32_t bfg_wait_tty_resume(struct ps_core_s *ps_core_d)
{
#define MAX_TTYRESUME_LOOPCNT 300

#ifdef ASYNCB_SUSPENDED
    uint32 loop_tty_resume_cnt = 0;
#endif

    if ((ps_core_d->tty) && (ps_core_d->tty->port)) {
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
        while (tty_port_suspended(ps_core_d->tty->port)) {
            if (loop_tty_resume_cnt++ >= MAX_TTYRESUME_LOOPCNT) {
                ps_print_err("tty is not ready, state:%d!\n", tty_port_suspended(ps_core_d->tty->port));
                break;
            }
            msleep(SLEEP_10_MSEC);
        }
        ps_print_info("tty state: 0x%x ,loop_tty_resume_cnt:%d\n",
                      tty_port_suspended(ps_core_d->tty->port), loop_tty_resume_cnt);
#else
        ps_print_info("tty port flag 0x%x\n", (unsigned int)ps_core_d->tty->port->flags);
#ifdef ASYNCB_SUSPENDED
        while (test_bit(ASYNCB_SUSPENDED, (volatile unsigned long *)&(ps_core_d->tty->port->flags))) {
            if (loop_tty_resume_cnt++ >= MAX_TTYRESUME_LOOPCNT) {
                ps_print_err("tty is not ready, flag is 0x%x!\n", (unsigned int)ps_core_d->tty->port->flags);
                break;
            }
            msleep(SLEEP_10_MSEC);
        }
#endif
#endif
    } else {
        ps_print_err("tty has not inited\n");
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

void host_send_disallow_msg(struct work_struct *work)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    uint64 flags;

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core has not inited\n");
        return;
    }

    /* 防止host睡眠情况下被dev唤醒进入gpio中断后直接在这里下发消息，
     * 此时uart可能还没有ready,所以这里等待tty resume之后才下发消息 */
    if (bfg_wait_tty_resume(ps_core_d) == OAL_FALSE) {
        ps_print_err("host send disallow msg wait tty resume fail!\n");
        return;
    }

    /* clear pf msg parsing buffer to avoid problem caused by wrong packet */
    reset_uart_rx_buf();

    /* 设置uart可用,下发disallow sleep消息,唤醒完成 */
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    bfgx_uart_state_set(UART_READY);
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    if (oal_atomic_read(&g_ir_only_mode) == 0) {
        if (ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_DISALLOW_SLP) != 0) {
            ps_print_info("SYS_CFG_DISALLOW_SLP MSG send fail, retry\n");
            msleep(SLEEP_10_MSEC);
            queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);
            return;
        }
    }

    /*
     * 这里设置完成量对于dev wkup host没有意义, 只是保证和host wkup dev的操作一致
     * 注意这就要求host wkup dev前需要INIT完成量计数
     */
    complete_all(&pm_data->host_wkup_dev_comp);

    if (oal_atomic_read(&g_ir_only_mode) == 0) {
        /* if any of BFNI is open, we should mod timer. */
        if ((!bfgx_other_subsys_all_shutdown(BFGX_GNSS)) ||
            (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP)) {
            mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
            pm_data->bfg_timer_mod_cnt++;
            ps_print_info("mod_timer:host_send_disallow_msg_etc\n");
        }
        ps_core_d->ps_pm->operate_beat_timer(BEAT_TIMER_RESET);
    }

    if (atomic_read(&pm_data->bfg_needwait_devboot_flag) == NEED_SET_FLAG) {
        complete(&pm_data->dev_bootok_ack_comp);
    }
}

void host_allow_bfg_sleep(struct work_struct *work)
{
    uint64 flags;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    ps_print_info("%s\n", __func__);

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core has not inited\n");
        return;
    }

    if (ps_core_d->tty_have_open == false) {
        ps_print_info("tty has closed, not send msg to dev\n");
        return;
    }

    get_exception_info_reference(&pst_exception_data);
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return;
    }
    if (atomic_read(&pst_exception_data->is_reseting_device) != PLAT_EXCEPTION_RESET_IDLE) {
        ps_print_err("plat is doing dfr not allow sleep\n");
        mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;
        return;
    }

    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);

    /* if someone is visiting the dev_node */
    if (atomic_read(&ps_core_d->node_visit_flag) > 0) {
        ps_print_info("someone visit node, not send allow sleep msg\n");
        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
        /* gnss write do NOT mod timer */
        mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;
        return;
    }

    if ((atomic_read(&pm_data->gnss_sleep_flag) != GNSS_AGREE_SLEEP) || (ps_chk_tx_queue_empty(ps_core_d) == false)) {
        ps_print_info("tx queue not empty, not send allow sleep msg\n");
        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
        mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;
        return;
    }
    /* 设置device状态为睡眠态，在host唤醒dev完成之前(或dev唤醒host前)uart不可用 */
    ps_print_info("%s,set UART_NOT_READY,BFGX_SLEEP\n", __func__);
    ps_core_d->ps_pm->bfgx_uart_state_set(UART_NOT_READY);
    ps_core_d->ps_pm->bfgx_dev_state_set(BFGX_SLEEP);
    /* clear mod cnt */
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    mod_timer(&pm_data->dev_ack_timer, jiffies + oal_msecs_to_jiffies(WAIT_DEVACK_MSEC));

    /*
     * we need reinit completion cnt as 0, to prevent such case:
     * 1)host allow dev sleep, dev ack with OK, cnt=1,
     * 2)device wkup host,
     * 3)host allow dev sleep,
     * 4)host wkup dev, it will wait dev_ack succ immediately since cnt==1,
     * 5)dev ack with ok, cnt=2,
     * this case will cause host wait dev_ack invalid.
     */
    oal_reinit_completion(pm_data->dev_ack_comp);

    if (ps_tx_urgent_cmd(ps_core_d, SYS_MSG, SYS_CFG_ALLOWDEV_SLP) != 0) {
        ps_print_info("SYS_CFG_ALLOWDEV_SLP MSG send fail\n");
    }
}

/*
 * Prototype    : bfg_check_timer_work
 * Description  : check bfg timer is work fine
 */
void bfg_check_timer_work(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return;
    }

    /* 10s后没有人启动bfg timer 补救:直接提交allow to sleep work */
    if ((pm_data->bfg_timer_mod_cnt == pm_data->bfg_timer_mod_cnt_pre) && (pm_data->bfg_timer_mod_cnt != 0) &&
        (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP)) {
        if (time_after(jiffies, pm_data->bfg_timer_check_time)) {
            declare_dft_trace_key_info("bfg_timer not work in 10s", OAL_DFT_TRACE_FAIL);
            if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
                ps_print_info("queue_work send_allow_sleep_work not return true\n");
            } else {
                ps_print_info("timer_state(%d),queue_work send_allow_sleep_work succ\n",
                              timer_pending(&pm_data->bfg_timer));
            }
        }
    } else {
        pm_data->bfg_timer_mod_cnt_pre = pm_data->bfg_timer_mod_cnt;
        pm_data->bfg_timer_check_time = jiffies + msecs_to_jiffies(PL_CHECK_TIMER_WORK);
    }
}

/*
 * Prototype    : bfg_timer_expire
 * Description  : bfg timer expired function
 */
void bfg_timer_expire(uint64 data)
{
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = (struct pm_drv_data *)(uintptr_t)data;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is null\n");
        return;
    }

    ps_core_d = pm_data->ps_pm_interface->ps_core_data;
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
        ps_print_dbg("lowpower function disabled\n");
        return;
    }
    if (pm_data->ps_pm_interface->bfgx_dev_state_get() == BFGX_SLEEP) {
        ps_print_dbg("dev has been sleep\n");
        return;
    }

    if (atomic_read(&pm_data->gnss_sleep_flag) == GNSS_AGREE_SLEEP &&
        (pm_data->uart_state < UART_BPS_CHG_SEND_ACK)) {
        if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
            ps_print_info("queue_work send_allow_sleep_work not return true\n");
        }
        pm_data->gnss_votesleep_check_cnt = 0;
        pm_data->rx_pkt_gnss_pre = 0;
    } else if (pm_data->uart_state < UART_BPS_CHG_SEND_ACK) {
        /* GNSS NOT AGREE SLEEP ,Check it */
        if (pm_data->rx_pkt_gnss_pre != ps_core_d->rx_pkt_num[BFGX_GNSS]) {
            pm_data->rx_pkt_gnss_pre = ps_core_d->rx_pkt_num[BFGX_GNSS];
            pm_data->gnss_votesleep_check_cnt = 0;

            mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
            pm_data->bfg_timer_mod_cnt++;
        } else {
            pm_data->gnss_votesleep_check_cnt++;
            if (pm_data->gnss_votesleep_check_cnt >= PL_CHECK_GNSS_VOTE_CNT) {
                ps_print_err("gnss_votesleep_check_cnt %d,set GNSS_AGREE_SLEEP\n", pm_data->gnss_votesleep_check_cnt);
                atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
                queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work);

                pm_data->gnss_votesleep_check_cnt = 0;
                pm_data->rx_pkt_gnss_pre = 0;
            } else {
                mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
                pm_data->bfg_timer_mod_cnt++;
            }
        }
    } else {
        ps_print_info("uart_state %d\n", pm_data->uart_state);
        mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(PLATFORM_SLEEP_TIME));
        pm_data->bfg_timer_mod_cnt++;
    }
}

int32 host_wkup_dev(void)
{
    uint64 timeleft;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    struct ps_core_s *ps_core_d = NULL;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }
    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d ot tty is NULL\n");
        return -EINVAL;
    }
    if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
        return 0;
    }
    ps_print_dbg("wkup start\n");

    oal_reinit_completion(pm_data->host_wkup_dev_comp);
    queue_work(pm_data->wkup_dev_workqueue, &pm_data->wkup_dev_work);
    ps_uart_state_pre(ps_core_d->tty);
    timeleft = wait_for_completion_timeout(&pm_data->host_wkup_dev_comp, msecs_to_jiffies(WAIT_WKUPDEV_MSEC));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);
        ps_print_err("wait wake up dev timeout\n");
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PM, CHR_PLAT_DRV_ERROR_BFG_WKUP_DEV);

        return -ETIMEDOUT;
    }
    ps_print_dbg("wkup over\n");

    return 0;
}

/*
 * Prototype    : bfgx_other_subsys_all_shutdown
 * Description  : check other subsystem is shutdown or not
 * Input        : subsys type: means one system to check
 * Return       : 0 - other subsystem are all shutdown
 *                1 - other subsystem are not all shutdown
 */
int32 bfgx_other_subsys_all_shutdown(uint8 subsys)
{
    int32 i = 0;
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    for (i = 0; i < BFGX_BUTT; i++) {
        if (i == subsys) {
            continue;
        }

        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) == POWER_STATE_OPEN) {
            return false;
        }
    }

    return true;
}

/*
 * Prototype    : bfgx_print_subsys_state
 * Description  : check all sub system state
 */
void bfgx_print_subsys_state(void)
{
    int32 i = 0;
    int32 total;
    int32 count = 0;
    const uint32 ul_print_str_len = 200;
    char print_str[ul_print_str_len];
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return;
    }

    total = 0;

    for (i = 0; i < BFGX_BUTT; i++) {
        if (atomic_read(&ps_core_d->bfgx_info[i].subsys_state) == POWER_STATE_OPEN) {
            total = snprintf_s(print_str + count, sizeof(print_str) - count, sizeof(print_str) - count - 1,
                               "%s:%s ", ps_core_d->bfgx_info[i].name, "on ");
            if (total < 0) {
                OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
                return;
            }
            count += total;
        } else {
            total = snprintf_s(print_str + count, sizeof(print_str) - count, sizeof(print_str) - count - 1,
                               "%s:%s ", ps_core_d->bfgx_info[i].name, "off");
            if (total < 0) {
                OAL_IO_PRINT("log str format err line[%d]\n", __LINE__);
                return;
            }
            count += total;
        }
    }

    ps_print_err("%s\n", print_str);
}

void bfgx_gpio_intr_enable(uint32 ul_en)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();

    uint64 flags;
    spin_lock_irqsave(&pm_data->bfg_irq_spinlock, flags);
    if (ul_en) {
        /* 不再支持中断开关嵌套 */
        if (pm_data->ul_irq_stat) {
            enable_irq(pm_data->bfg_irq);
            pm_data->ul_irq_stat = 0;
        }
    } else {
        if (!pm_data->ul_irq_stat) {
            disable_irq_nosync(pm_data->bfg_irq);
            pm_data->ul_irq_stat = 1;
        }
    }
    spin_unlock_irqrestore(&pm_data->bfg_irq_spinlock, flags);
}

int32 bfgx_dev_power_on(void)
{
    uint64 timeleft;
    int32 error;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    board_info *bd_info = NULL;

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return BFGX_POWER_FAILED;
    }
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return BFGX_POWER_FAILED;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return BFGX_POWER_FAILED;
    }
    /* 防止Host睡眠 */
    oal_wake_lock(&pm_data->bfg_wake_lock);

    bfgx_gpio_intr_enable(OAL_TRUE);
    gnss_timesync_gpio_intr_enable(OAL_TRUE);
    oal_reinit_completion(pm_data->dev_bootok_ack_comp);
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);
    error = bd_info->bd_ops.bfgx_dev_power_on();
    if (error != BFGX_POWER_SUCCESS) {
        goto bfgx_power_on_fail;
    }

    ps_uart_state_pre(ps_core_d->tty);
    /* WAIT_BFGX_BOOTOK_TIME:这个时间目前为1s，有1s不够的情况，需要关注 */
    timeleft = wait_for_completion_timeout(&pm_data->dev_bootok_ack_comp, msecs_to_jiffies(WAIT_BFGX_BOOTOK_TIME));
    if (!timeleft) {
        ps_uart_state_dump(ps_core_d->tty);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_BCPU_BOOTUP);

        if (wlan_is_shutdown()) {
            ps_print_err("wifi off, wait bfgx boot up ok timeout\n");
            error = BFGX_POWER_WIFI_OFF_BOOT_UP_FAIL;
            goto bfgx_power_on_fail;
        } else {
            ps_print_err("wifi on, wait bfgx boot up ok timeout\n");
            error = BFGX_POWER_WIFI_ON_BOOT_UP_FAIL;
            goto bfgx_power_on_fail;
        }
    }

    atomic_set(&pm_data->bfg_needwait_devboot_flag, NONEED_SET_FLAG);

    if (oal_atomic_read(&g_ir_only_mode) == 0) {
        if ((get_hi110x_subchip_type() == BOARD_VERSION_HI1103) ||
            (get_hi110x_subchip_type() == BOARD_VERSION_HI1102A)) {
            if (wlan_is_shutdown()) {
                ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_NOTIFY_WIFI_CLOSE);
            } else {
                ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_NOTIFY_WIFI_OPEN);
            }
        }

        bfgx_pm_feature_set();
    }

    return BFGX_POWER_SUCCESS;

bfgx_power_on_fail:
    oal_wake_unlock(&pm_data->bfg_wake_lock);
    return error;
}

/*
 * Prototype    : bfgx_dev_power_off
 * Description  : bfg device power off function
 * Return       : 0 means succeed,-1 means failed
 */
int32 bfgx_dev_power_off(void)
{
    int32 error = SUCCESS;
    struct ps_core_s *ps_core_d = NULL;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    board_info *bd_info = NULL;
    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -FAILURE;
    }

    /* 单红外没有心跳 */
    if (oal_atomic_read(&g_ir_only_mode) == 0) {
        pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
        del_timer_sync(&pm_data->bfg_timer);
        pm_data->bfg_timer_mod_cnt = 0;
        pm_data->bfg_timer_mod_cnt_pre = 0;
    }

    /* 下电即将完成，需要在此时设置下次上电要等待device上电成功的flag */
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);
    bd_info->bd_ops.bfgx_dev_power_off();

    ps_core_d->rx_pkt_sys = 0;
    ps_core_d->rx_pkt_oml = 0;
    bfg_wake_unlock();

    bfgx_gpio_intr_enable(OAL_FALSE);
    gnss_timesync_gpio_intr_enable(OAL_FALSE);

    return error;
}

/*
 * Prototype    : bfgx_dev_power_control
 * Description  : bfg power control function
 * Input        : uint8 flag: 1 means on, 0 means off
 *                uint8 subsys: means one of bfg system
 * Return       : 0 means succeed,-1 means failed
 */
int32 bfgx_dev_power_control(uint8 subsys, uint8 flag)
{
    int32 ret = 0;

    if (flag == BFG_POWER_GPIO_UP) {
        ret = bfgx_power_on(subsys);
        if (ret) {
            ps_print_err("bfgx power on is error!\n");
        }
    } else if (flag == BFG_POWER_GPIO_DOWN) {
        ret = bfgx_power_off(subsys);
        if (ret) {
            ps_print_err("bfgx power off is error!\n");
        }
    } else {
        ps_print_err("invalid input data!\n");
        ret = -FAILURE;
    }

    return ret;
}

void gnss_lowpower_vote(uint32_t vote)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();

    ps_print_info("gnss_lowpower_vote %d!\n", vote);
    if (vote == GNSS_AGREE_SLEEP) {
        if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
            ps_print_warning("gnss low power disabled!\n");
            return;
        }
        if (pm_data->ps_pm_interface->bfgx_dev_state_get() == BFGX_SLEEP) {
            ps_print_warning("gnss proc: dev has been sleep, not allow dev slp\n");
            return;
        }

        /* if bt and fm are both shutdown ,we will pull down gpio directly */
        if (!timer_pending(&pm_data->bfg_timer)) {
            ps_print_info("gnss_lowpower_vote sumit sleep work!\n");

            if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
                ps_print_info("queue_work send_allow_sleep_work not return true\n");
            }
        }

        /* set the flag to 1 means gnss request sleep */
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    } else if (vote == GNSS_NOT_AGREE_SLEEP) {
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_NOT_AGREE_SLEEP);
    } else {
        ps_print_err("PLAT_GNSS_SLEEP_VOTE_CMD invalid arg !\n");
    }
}

static void firmware_download_fail_proc(struct pm_drv_data *pm_data, int which_cfg)
{
    ps_print_err("firmware download fail!\n");
    declare_dft_trace_key_info("patch_download_fail", OAL_DFT_TRACE_FAIL);
    if (which_cfg == BFGX_CFG) {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_BFG_FW_DOWN);
    } else {
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_WIFI_FW_DOWN);
    }

#ifdef PLATFORM_SSI_FULL_LOG
    if (hi11xx_get_os_build_variant() != HI1XX_OS_BUILD_VARIANT_USER) {
        unsigned long long reg = SSI_MODULE_MASK_COMM;
        if (pm_data->pst_wlan_pm_info->pst_bus->bus_type == HCC_BUS_SDIO) {
            reg |= SSI_MODULE_MASK_SDIO;
        }

        ssi_dump_device_regs(reg);

        /* dump bootloader rw data, 0xa7800--0xa7e18 */
        ssi_read_reg_info_test(HI1102A_BOOTLOAD_DTCM_BASE_ADDR, HI1102A_BOOTLOAD_DTCM_SIZE, 1, SSI_RW_DWORD_MOD);
    }
#endif
}

/*
 * Prototype    : firmware_download_function
 * Description  : download wlan patch
 */
int firmware_download_function(uint32 which_cfg)
{
    int32 ret;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_time = 0;
    static unsigned long long count = 0;
    struct pm_drv_data *pm_data = pm_get_drvdata();

    ps_print_info("enter firmware_download_function\n");

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    if (which_cfg >= CFG_FILE_TOTAL) {
        ps_print_err("cfg file index [%d] outof range\n", which_cfg);
        return -FAILURE;
    }

    if (pm_data->pst_wlan_pm_info->pst_bus == NULL) {
        ps_print_err("pst_bus is null\n");
        return -FAILURE;
    }

    start_time = ktime_get();

    hcc_bus_wake_lock(pm_data->pst_wlan_pm_info->pst_bus);
    hcc_bus_lock(pm_data->pst_wlan_pm_info->pst_bus);

    ret = hcc_bus_reinit(pm_data->pst_wlan_pm_info->pst_bus);
    if (ret != OAL_SUCC) {
        hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        ps_print_err("sdio reinit failed, ret:%d!\n", ret);
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_FW_SDIO_INIT);
        return -FAILURE;
    }

    wlan_pm_init_dev();

    /* firmware_cfg_init(sdio) function should just be called once */
    if (!test_bit(FIRMWARE_CFG_INIT_OK, &pm_data->firmware_cfg_init_flag)) {
        ps_print_info("firmware_cfg_init begin\n");
        ret = firmware_cfg_init();
        if (ret) {
            ps_print_err("firmware_cfg_init failed, ret:%d!\n", ret);
            hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
            hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);

            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                                 CHR_PLT_DRV_EVENT_FW, CHR_PLAT_DRV_ERROR_CFG_FAIL_FIRMWARE_DOWN);
            return ret;
        }

        ps_print_info("firmware_cfg_init OK\n");
        set_bit(FIRMWARE_CFG_INIT_OK, &pm_data->firmware_cfg_init_flag);
    }

    ps_print_info("firmware_download begin\n");

    ret = firmware_download(which_cfg);
    if (ret < 0) {
        hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);
        if (ret != -OAL_EINTR) {
            firmware_download_fail_proc(pm_data, which_cfg);
        } else {
            /* download firmware interrupt */
            ps_print_info("firmware download interrupt!\n");
            declare_dft_trace_key_info("patch_download_interrupt", OAL_DFT_TRACE_FAIL);
        }
        return ret;
    }
    declare_dft_trace_key_info("patch_download_ok", OAL_DFT_TRACE_SUCC);

    hcc_bus_unlock(pm_data->pst_wlan_pm_info->pst_bus);
    hcc_bus_wake_unlock(pm_data->pst_wlan_pm_info->pst_bus);

    ps_print_info("firmware_download success\n");

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_time) {
        max_time = total_time;
    }

    count++;
    ps_print_warning("download firmware, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     count, total_time, max_time);

    return SUCCESS;
}

int32 wlan_is_shutdown(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return true;
    }

    return ((pm_data->pst_wlan_pm_info->ul_wlan_power_state == POWER_STATE_SHUTDOWN) ? true : false);
}

int32 bfgx_is_shutdown(void)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    return ps_chk_bfg_active(ps_core_d) ? false : true;
}
EXPORT_SYMBOL(bfgx_is_shutdown);

int32 wifi_power_fail_process(int32 error)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return WIFI_POWER_FAIL;
    }

    ps_print_info("wifi power fail, error=[%d]\n", error);

    switch (error) {
        case WIFI_POWER_SUCCESS:
        case WIFI_POWER_PULL_POWER_GPIO_FAIL:
            break;

        /* BFGX off，wifi firmware download fail和wait boot up fail，直接返回失败，上层重试，不走DFR */
        case WIFI_POWER_BFGX_OFF_BOOT_UP_FAIL:
            if (oal_trigger_bus_exception(pm_data->pst_wlan_pm_info->pst_bus, OAL_TRUE) == OAL_TRUE) {
                /* exception is processing, can't power off */
                ps_print_info("bfgx off,sdio exception is working\n");
                break;
            }
            ps_print_info("bfgx off,set wlan_power_state to shutdown\n");
            oal_wlan_gpio_intr_enable(hbus_to_dev(pm_data->pst_wlan_pm_info->pst_bus), OAL_FALSE);
            pm_data->pst_wlan_pm_info->ul_wlan_power_state = POWER_STATE_SHUTDOWN;
        case WIFI_POWER_BFGX_OFF_FIRMWARE_DOWNLOAD_FAIL:
        case WIFI_POWER_ON_CALI_FAIL:
            ps_print_info("wifi power fail: pull down power on gpio\n");
            board_power_off(WLAN_POWER);
            break;

        /* BFGX on，wifi上电失败，进行全系统复位，wifi本次返回失败，上层重试 */
        case WIFI_POWER_BFGX_ON_BOOT_UP_FAIL:
            if (oal_trigger_bus_exception(pm_data->pst_wlan_pm_info->pst_bus, OAL_TRUE) == OAL_TRUE) {
                /* exception is processing, can't power off */
                ps_print_info("bfgx on,sdio exception is working\n");
                break;
            }
            ps_print_info("bfgx on,set wlan_power_state to shutdown\n");
            oal_wlan_gpio_intr_enable(hbus_to_dev(pm_data->pst_wlan_pm_info->pst_bus), OAL_FALSE);
            pm_data->pst_wlan_pm_info->ul_wlan_power_state = POWER_STATE_SHUTDOWN;
        case WIFI_POWER_BFGX_DERESET_WCPU_FAIL:
        case WIFI_POWER_BFGX_ON_FIRMWARE_DOWNLOAD_FAIL:
            if (plat_power_fail_exception_info_set(SUBSYS_WIFI, THREAD_WIFI, WIFI_POWERON_FAIL) == WIFI_POWER_SUCCESS) {
                bfgx_system_reset();
                plat_power_fail_process_done();
            } else {
                ps_print_err("wifi power fail, set exception info fail\n");
            }
            break;

        default:
            ps_print_err("error is undefined, error=[%d]\n", error);
            break;
    }

    return WIFI_POWER_FAIL;
}

int32 wifi_notify_bfgx_status(uint8 status)
{
    struct ps_core_s *ps_core_d = NULL;
    int32 ret;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is err\n");
        return -EFAULT;
        ;
    }

    if (!bfgx_is_shutdown()) {
        ret = prepare_to_visit_node(ps_core_d);
        if (ret < 0) {
            ps_print_err("prepare work fail, bring to reset work\n");
            plat_exception_handler(SUBSYS_BFGX, THREAD_IDLE, BFGX_WAKEUP_FAIL);
            return ret;
        }

        ps_tx_sys_cmd(ps_core_d, SYS_MSG, status);

        post_to_visit_node(ps_core_d);
    }

    return 0;
}

int32 hitalk_power_on(void)
{
    int32 error;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_download_time = 0;
    static unsigned long long num = 0;
    board_info *bd_info = NULL;

    ps_print_info("hitalk power on!\n");

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -FAILURE;
    }

    /* wifi上电时如果单红外打开，则需要关闭单红外，下载全patch */
    if ((oal_atomic_read(&g_ir_only_mode) != 0)) {
        if (hw_ir_only_open_other_subsys() != BFGX_POWER_SUCCESS) {
            ps_print_err("ir only mode,but close ir only mode fail!\n");
            return -FAILURE;
        }
    }

    start_time = ktime_get();

    if (hcc_bus_exception_is_busy(hcc_get_current_110x_bus()) == OAL_TRUE) {
        declare_dft_trace_key_info("open_fail_sdio_is_busy", OAL_DFT_TRACE_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_HITALK_POWER_ON_EXCP);
        return -FAILURE;
    }

    error = bd_info->bd_ops.hitalk_power_on();
    if (error != WIFI_POWER_SUCCESS) {
        unsigned long long module_set = SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_BCTRL | SSI_MODULE_MASK_WCTRL;
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hitalk power on failed\n");
        if (hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_USER) {
            if (!oal_print_rate_limit(PRINT_RATE_MINUTE)) { /* 1分钟打印一次 */
                module_set = 0x0;
            }
        }
        ssi_dump_device_regs(module_set);

        goto hitalk_power_fail;
    }

    if ((get_hi110x_subchip_type() == BOARD_VERSION_HI1103) || (get_hi110x_subchip_type() == BOARD_VERSION_HI1102A)) {
        wifi_notify_bfgx_status(SYS_CFG_NOTIFY_WIFI_OPEN);
    }

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_download_time) {
        max_download_time = total_time;
    }

    num++;
    ps_print_warning("hitalk power on, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     num, total_time, max_download_time);

    return WIFI_POWER_SUCCESS;

hitalk_power_fail:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_HITALK_POWER_UP_WIFI);
    return wifi_power_fail_process(error);
}

int32 hitalk_power_off(void)
{
    int32 error;
    board_info *bd_info = NULL;

    ps_print_info("hitalk power off!\n");

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -FAILURE;
    }

    if ((get_hi110x_subchip_type() == BOARD_VERSION_HI1103) || (get_hi110x_subchip_type() == BOARD_VERSION_HI1102A)) {
        wifi_notify_bfgx_status(SYS_CFG_NOTIFY_WIFI_CLOSE);
    }

    error = bd_info->bd_ops.hitalk_power_off();
    if (error != SUCCESS) {
        return error;
    }

    return SUCCESS;
}

int32 wlan_power_on(void)
{
    int32 error;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_download_time = 0;
    static unsigned long long num = 0;
    board_info *bd_info = NULL;

    ps_print_info("wlan power on!\n");

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -FAILURE;
    }

    /* wifi上电时如果单红外打开，则需要关闭单红外，下载全patch */
    if (oal_atomic_read(&g_ir_only_mode) != 0) {
        if (hw_ir_only_open_other_subsys() != BFGX_POWER_SUCCESS) {
            ps_print_err("ir only mode,but close ir only mode fail!\n");
            return -FAILURE;
        }
    }

    start_time = ktime_get();

    if (hcc_bus_exception_is_busy(hcc_get_current_110x_bus()) == OAL_TRUE) {
        declare_dft_trace_key_info("open_fail_sdio_is_busy", OAL_DFT_TRACE_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_POWER_ON_EXCP);
        return -FAILURE;
    }

    error = bd_info->bd_ops.wlan_power_on();
    if (error != WIFI_POWER_SUCCESS) {
        goto wifi_power_fail;
    }

    if ((get_hi110x_subchip_type() == BOARD_VERSION_HI1103) || (get_hi110x_subchip_type() == BOARD_VERSION_HI1102A)) {
        wifi_notify_bfgx_status(SYS_CFG_NOTIFY_WIFI_OPEN);
    }

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_download_time) {
        max_download_time = total_time;
    }

    num++;
    ps_print_warning("power on, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     num, total_time, max_download_time);

    return WIFI_POWER_SUCCESS;

wifi_power_fail:

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_OPEN, CHR_PLAT_DRV_ERROR_POWER_UP_WIFI);
    return wifi_power_fail_process(error);
}

int32 wlan_power_off(void)
{
    int32 error;
    board_info *bd_info = NULL;

    ps_print_info("wlan power off!\n");

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return -FAILURE;
    }

    if ((get_hi110x_subchip_type() == BOARD_VERSION_HI1103) || (get_hi110x_subchip_type() == BOARD_VERSION_HI1102A)) {
        wifi_notify_bfgx_status(SYS_CFG_NOTIFY_WIFI_CLOSE);
    }

    error = bd_info->bd_ops.wlan_power_off();
    if (error != SUCCESS) {
        return error;
    }

    return SUCCESS;
}

int32 bfgx_power_on(uint8 subsys)
{
    int32 ret;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;
    static unsigned long long max_download_time = 0;
    static unsigned long long num = 0;

    start_time = ktime_get();

    if (bfgx_other_subsys_all_shutdown(subsys)) {
        ret = bfgx_dev_power_on();
        if (ret != BFGX_POWER_SUCCESS) {
            return ret;
        }
    }

    end_time = ktime_get();

    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    if (total_time > max_download_time) {
        max_download_time = total_time;
    }

    num++;
    ps_print_warning("power on, count [%llu], current time [%llu]us, max time [%llu]us\n",
                     num, total_time, max_download_time);

    return BFGX_POWER_SUCCESS;
}

int32 bfgx_power_off(uint8 subsys)
{
    struct ps_core_s *ps_core_d = NULL;

    ps_get_core_reference(&ps_core_d);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is null\n");
        return -FAILURE;
    }

    if (ps_chk_only_gnss_and_cldslp(ps_core_d)) {
        ps_print_suc("%s power off request sleep!\n", g_bfgx_subsys_name[subsys]);
        host_allow_devslp_in_node(ps_core_d);

        return SUCCESS;
    }

    ps_print_info("%s power off!\n", g_bfgx_subsys_name[subsys]);

    if (bfgx_other_subsys_all_shutdown(subsys)) {
        return bfgx_dev_power_off();
    }

    return SUCCESS;
}
int32 g_pro_memcheck_en = 0;
struct completion g_pro_memcheck_finish;
int32 memcheck_is_working(void)
{
    if (g_pro_memcheck_en) {
        complete(&g_pro_memcheck_finish);
        ps_print_info("is in product mem check test !bfg_wakeup_host=%d\n",
                      oal_gpio_get_value(g_board_info.bfgn_wakeup_host));
        return 0;
    }
    return -1;
}
void memcheck_bfgx_init(void)
{
    bfgx_gpio_intr_enable(OAL_TRUE);
    ps_print_info("memcheck_bfgx_init\n");
    g_pro_memcheck_en = 1;
    init_completion(&g_pro_memcheck_finish);
}
void memcheck_bfgx_exit(void)
{
    g_pro_memcheck_en = 0;
    bfgx_gpio_intr_enable(OAL_FALSE);
}

int32 memcheck_bfgx_is_succ(void)
{
    uint64 timeleft;
    PS_PRINT_FUNCTION_NAME;
    timeleft = wait_for_completion_timeout(&g_pro_memcheck_finish, msecs_to_jiffies(g_bfgx_mem_check_mdelay));
    if (!timeleft) {
        ps_print_err("wait bfgx memcheck_bfgx_is_succ timeout\n");
        return -1;
    }
    return 0;
}

static int32 device_mem_check_prepare(unsigned long long *time)
{
    if (time == NULL) {
        ps_print_err("param time is  NULL!\n");
        return -FAILURE;
    }
    if (!bfgx_is_shutdown()) {
        ps_print_suc("factory ram reg test need bfgx shut down!\n");
        bfgx_print_subsys_state();
        return -FAILURE;
    }
    if (!wlan_is_shutdown()) {
        ps_print_suc("factory ram reg test need wlan shut down!\n");
        return -FAILURE;
    }
    return SUCCESS;
}

static int32 device_mem_check_wcpu(void)
{
    int32 ret;
    uint8 *file_name = WIFI_DUMP_PATH "/readm_wifi";

    ret = board_power_on(WLAN_POWER);
    if (ret) {
        ps_print_err("WLAN_POWER on failed ret=%d\n", ret);
        return -FAILURE;
    }

    ret = firmware_download_function(RAM_REG_TEST_CFG);
    if (ret == SUCCESS) {
        /* 等待device信息处理 */
        mdelay(g_wlan_mem_check_mdelay);
        ret = is_device_mem_test_succ();
        get_device_test_mem(file_name);
        if (ret) {
            board_power_off(WLAN_POWER);
            return -FAILURE;
        }
    } else {
        ps_print_err("Memory check failed because the firmware fails to be downloaded. ret=%d\n", ret);
        board_power_off(WLAN_POWER);
        return ret;
    }

    board_power_off(WLAN_POWER);
    return SUCCESS;
}

static int32 device_mem_check_bcpu(void)
{
    int32 ret;
    uint8 *file_name = WIFI_DUMP_PATH "/readm_bfgx";

    board_power_on(WLAN_POWER);

    g_pilot_cfg_patch_in_vendor[RAM_REG_TEST_CFG] = RAM_BCPU_CHECK_CFG_HI1102A_PILOT_PATH;
    ret = firmware_get_cfg(g_cfg_path[RAM_REG_TEST_CFG], RAM_REG_TEST_CFG);
    if (ret) {
        ps_print_info("ini analysis fail!\n");
        board_power_off(WLAN_POWER);
        return -FAILURE;
    }

    memcheck_bfgx_init();

    ret = firmware_download_function(RAM_BCPU_REG_TEST_CFG);
    g_pilot_cfg_patch_in_vendor[RAM_REG_TEST_CFG] = RAM_CHECK_CFG_HI1102A_PILOT_PATH;
    if (ret == SUCCESS) {
        /* 等待device信息处理 */
        mdelay(g_bfgx_mem_check_mdelay);
        ret = is_device_mem_test_succ();
        get_device_test_mem(file_name);
        if (ret) {
            memcheck_bfgx_exit();
            board_power_off(WLAN_POWER);
            return -FAILURE;
        }
    } else {
        ps_print_err("Memory check failed because the firmware fails to be downloaded. ret=%d\n", ret);
        board_power_off(WLAN_POWER);
        return ret;
    }

    firmware_get_cfg(g_cfg_path[RAM_REG_TEST_CFG], RAM_REG_TEST_CFG);

    memcheck_bfgx_exit();
    board_power_off(WLAN_POWER);
    return SUCCESS;
}

int32 device_mem_check(unsigned long long *time)
{
    int32 ret;
    unsigned long long total_time;
    ktime_t start_time, end_time, trans_time;

    ps_print_info("device ram reg test!\n");
    ret = device_mem_check_prepare(time);
    if (ret != SUCCESS) {
        return ret;
    }

    start_time = ktime_get();

    ps_print_info("===================start wcpu ram reg test!\n");
    ret = device_mem_check_wcpu();
    if (ret == SUCCESS) {
        ps_print_info("======device wcpu ram reg test success!\n");
    } else {
        ps_print_info("==device wcpu ram reg test failed!\n");
        return ret;
    }

    ps_print_info("===================start bcpu ram reg test!\n");
    ret = device_mem_check_bcpu();
    if (ret == SUCCESS) {
        ps_print_info("======device bcpu ram reg test success!\n");
    } else {
        ps_print_info("==device bcpu ram reg test failed!\n");
        return ret;
    }

    end_time = ktime_get();
    trans_time = ktime_sub(end_time, start_time);
    total_time = (unsigned long long)ktime_to_us(trans_time);
    *time = total_time;
    ps_print_suc("device mem reg test time [%llu]us\n", total_time);

    return SUCCESS;
}

EXPORT_SYMBOL(device_mem_check);

/*
 * Prototype    : ps_pm_register
 * Description  : register interface for 3 in 1
 * Input        : struct ps_pm_s *new_pm: interface want to register
 * Return       : 0 means succeed,-1 means failed
 */
int32 ps_pm_register(struct ps_pm_s *new_pm)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL\n");
        return -FAILURE;
    }

    if (new_pm == NULL) {
        ps_print_err("new_pm is null!\n");
        return -FAILURE;
    }

    pm_data->ps_pm_interface = new_pm;
    pm_data->ps_pm_interface->pm_priv_data = pm_data;
    pm_data->ps_pm_interface->bfg_wake_lock = bfg_wake_lock;
    pm_data->ps_pm_interface->bfg_wake_unlock = bfg_wake_unlock;
    pm_data->ps_pm_interface->bfgx_dev_state_get = bfgx_state_get;
    pm_data->ps_pm_interface->bfgx_dev_state_set = bfgx_state_set;
    pm_data->ps_pm_interface->bfg_power_set = bfgx_dev_power_control;
    pm_data->ps_pm_interface->bfgx_uart_state_get = bfgx_uart_state_get;
    pm_data->ps_pm_interface->bfgx_uart_state_set = bfgx_uart_state_set;

    ps_print_suc("pm registered over!");

    return SUCCESS;
}
EXPORT_SYMBOL_GPL(ps_pm_register);

/*
 * Prototype    : ps_pm_unregister
 * Description  : unregister interface for 3 in 1
 * Input        : struct ps_pm_s *new_pm: interface want to unregister
 * Return       : 0 means succeed,-1 means failed
 */
int32 ps_pm_unregister(struct ps_pm_s *new_pm)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL\n");
        return -FAILURE;
    }

    ps_print_dbg("enter\n");

    if (new_pm == NULL) {
        ps_print_err("new_pm is null!\n");
        return -FAILURE;
    }

    new_pm->bfg_wake_lock = NULL;
    new_pm->bfg_wake_unlock = NULL;
    new_pm->bfgx_dev_state_get = NULL;
    new_pm->bfgx_dev_state_set = NULL;
    new_pm->bfg_power_set = NULL;
    new_pm->bfgx_uart_state_set = NULL;
    new_pm->bfgx_uart_state_get = NULL;
    pm_data->ps_pm_interface = NULL;

    ps_print_suc("pm unregistered over!");

    return SUCCESS;
}
EXPORT_SYMBOL_GPL(ps_pm_unregister);

/*
 * Prototype    : bfg_wake_host_isr
 * Description  : functions called when bt wake host via GPIO
 */
irqreturn_t bfg_wake_host_isr(int irq, void *dev_id)
{
    struct ps_core_s *ps_core_d = NULL;
    uint64 flags;
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (!memcheck_is_working()) {
        return IRQ_NONE;
    }
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return IRQ_NONE;
    }

    ps_print_info("%s\n", __func__);
    spin_lock_irqsave(&pm_data->uart_state_spinlock, flags);
    if (pm_data->uart_state >= UART_BPS_CHG_SEND_ACK) {
        if (pm_data->uart_state == UART_BPS_CHG_SEND_ACK) {
            bfgx_uart_state_set(UART_BPS_CHG_IN_PROGRESS);
        } else if (pm_data->uart_state == UART_BPS_CHG_SEND_COMPLETE) {
            bfgx_uart_state_set(UART_READY);
        }

        spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);
        return IRQ_HANDLED;
    }
    spin_unlock_irqrestore(&pm_data->uart_state_spinlock, flags);

    pm_data->bfg_wakeup_host++;

    g_wakeup_src_debug = 1;

    ps_get_core_reference(&ps_core_d);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is null\n");
        return IRQ_NONE;
    }

    spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
    ps_core_d->ps_pm->bfg_wake_lock();
    bfgx_state_set(BFGX_ACTIVE);
    complete(&pm_data->dev_ack_comp);
    spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);

    queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_disallow_msg_work);

    return IRQ_HANDLED;
}

/* return 1 for wifi power on,0 for off. */
oal_int32 hi110x_get_wifi_power_stat(oal_void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return 0;
    }
    return (pm_data->pst_wlan_pm_info->ul_wlan_power_state != POWER_STATE_SHUTDOWN);
}
EXPORT_SYMBOL(hi110x_get_wifi_power_stat);

STATIC int low_power_remove(void)
{
    struct pm_drv_data *pm_data = pm_get_drvdata();
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    wlan_pm_exit();

    free_irq(pm_data->bfg_irq, NULL);

    /* delete timer */
    del_timer_sync(&pm_data->bfg_timer);
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;

    del_timer_sync(&pm_data->dev_ack_timer);
    /* destory wake lock */
    oal_wake_lock_exit(&pm_data->bfg_wake_lock);
    oal_wake_lock_exit(&pm_data->bt_wake_lock);
    oal_wake_lock_exit(&pm_data->gnss_wake_lock);
    /* free platform driver data struct */
    kfree(pm_data);

    pm_data = NULL;

    pm_set_drvdata(NULL);

    return 0;
}

STATIC void devack_timer_expire(uint64 data)
{
    uint64 flags;
    struct pm_drv_data *pm_data = (struct pm_drv_data *)(uintptr_t)data;
    if (unlikely(pm_data == NULL)) {
        ps_print_err("devack timer para is null\n");
        return;
    }

    ps_print_info("%s\n", __func__);

    if (board_get_bwkup_gpio_val() == 1) {
        pm_data->uc_dev_ack_wait_cnt++;
        if (WAIT_DEVACK_CNT > pm_data->uc_dev_ack_wait_cnt) {
            mod_timer(&pm_data->dev_ack_timer, jiffies + oal_msecs_to_jiffies(WAIT_DEVACK_MSEC));
            return;
        }
        /* device doesn't agree to sleep */
        ps_print_info("device does not agree to sleep\n");
        if (unlikely(pm_data->rcvdata_bef_devack_flag == 1)) {
            ps_print_info("device send data to host before dev rcv allow slp msg\n");
            pm_data->rcvdata_bef_devack_flag = 0;
        }

        bfgx_state_set(BFGX_ACTIVE);
        bfgx_uart_state_set(UART_READY);
        /*
         * we mod timer at any time, since we could get another chance to sleep
         * in exception case like:dev agree to slp after this ack timer expired
         */
        if (!bfgx_other_subsys_all_shutdown(BFGX_GNSS)) {
            mod_timer(&pm_data->bfg_timer, jiffies + oal_msecs_to_jiffies(BT_SLEEP_TIME));
            pm_data->bfg_timer_mod_cnt++;
        }

        complete(&pm_data->dev_ack_comp);
    } else {
        spin_lock_irqsave(&pm_data->wakelock_protect_spinlock, flags);
        if (pm_data->bfgx_dev_state == BFGX_ACTIVE) {
            ps_print_info("wkup isr occur during wait for dev allow ack\n");
        } else {
            pm_data->ps_pm_interface->operate_beat_timer(BEAT_TIMER_DELETE);
            bfg_wake_unlock();
        }
        spin_unlock_irqrestore(&pm_data->wakelock_protect_spinlock, flags);

        complete(&pm_data->dev_ack_comp);

        pm_data->uc_dev_ack_wait_cnt = 0;
    }
}

STATIC int32_t low_power_irq_init(struct pm_drv_data *pm_data)
{
    int32_t ret;

    spin_lock_init(&pm_data->bfg_irq_spinlock);
    pm_data->board = get_hi110x_board_info();
    pm_data->bfg_irq = pm_data->board->bfgx_irq;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    ret = request_irq(pm_data->bfg_irq, bfg_wake_host_isr, IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND,
                      "bfgx_wake_host", NULL);
#else
    ret = request_irq(pm_data->bfg_irq, bfg_wake_host_isr, IRQF_DISABLED | IRQF_TRIGGER_RISING,
                      "bfgx_wake_host", NULL);
#endif
    if (ret < 0) {
        ps_print_err("couldn't acquire %s IRQ\n", PROC_NAME_GPIO_BFGX_WAKEUP_HOST);
        return OAL_FAIL;
    }

    disable_irq_nosync(pm_data->bfg_irq);
    pm_data->ul_irq_stat = 1; /* irq diabled default. */

    return OAL_SUCC;
}

STATIC int32_t low_power_workq_init(struct pm_drv_data *pm_data)
{
    struct workqueue_struct *host_wkup_dev_workq = NULL;

    /* create an ordered workqueue with @max_active = 1 & WQ_UNBOUND flag to wake up device */
    host_wkup_dev_workq = create_singlethread_workqueue("wkup_dev_workqueue");
    if (host_wkup_dev_workq == NULL) {
        ps_print_err("create wkup workqueue failed\n");
        return OAL_FAIL;
    }
    pm_data->wkup_dev_workqueue = host_wkup_dev_workq;
    INIT_WORK(&pm_data->wkup_dev_work, host_wkup_dev_work);
    INIT_WORK(&pm_data->send_disallow_msg_work, host_send_disallow_msg);
    INIT_WORK(&pm_data->send_allow_sleep_work, host_allow_bfg_sleep);
    INIT_WORK(&pm_data->baud_change_work, bfgx_uart_baud_change_work);

    /* init bfg wake lock */
    oal_wake_lock_init(&pm_data->bfg_wake_lock, BFG_LOCK_NAME);
    oal_wake_lock_init(&pm_data->bt_wake_lock, BT_LOCK_NAME);
    oal_wake_lock_init(&pm_data->gnss_wake_lock, GNSS_LOCK_NAME);
    /* init mutex */
    mutex_init(&pm_data->host_mutex);

    /* init spinlock */
    spin_lock_init(&pm_data->uart_state_spinlock);
    spin_lock_init(&pm_data->wakelock_protect_spinlock);

    /* init completion */
    init_completion(&pm_data->host_wkup_dev_comp);
    init_completion(&pm_data->dev_ack_comp);
    init_completion(&pm_data->dev_bootok_ack_comp);

    return OAL_SUCC;
}

STATIC void low_power_probe_timer_init(struct pm_drv_data *pm_data)
{
    /* init timer */
    init_timer(&pm_data->dev_ack_timer);
    pm_data->dev_ack_timer.function = devack_timer_expire;
    pm_data->dev_ack_timer.data = (uintptr_t)pm_data;
    pm_data->uc_dev_ack_wait_cnt = 0;

    /* init bfg data timer */
    init_timer(&pm_data->bfg_timer);
    pm_data->bfg_timer.function = bfg_timer_expire;
    pm_data->bfg_timer.data = (uintptr_t)pm_data;
    pm_data->bfg_timer_mod_cnt = 0;
    pm_data->bfg_timer_mod_cnt_pre = 0;
    pm_data->bfg_timer_check_time = 0;
    pm_data->rx_pkt_gnss_pre = 0;
    pm_data->gnss_votesleep_check_cnt = 0;

    ps_print_info("uart baud change support version\n");
    init_timer(&pm_data->baud_change_timer);
    pm_data->baud_change_timer.function = bfgx_uart_baud_change_expire;
    pm_data->baud_change_timer.data = (uintptr_t)pm_data;
}

STATIC int32_t low_power_probe(void)
{
    int32_t ret;
    struct pm_drv_data *pm_data = NULL;

    pm_data = kzalloc(sizeof(struct pm_drv_data), GFP_KERNEL);
    if (pm_data == NULL) {
        ps_print_err("no mem to allocate pm_data\n");
        goto PMDATA_MALLOC_FAIL;
    }

    pm_data->pst_wlan_pm_info = wlan_pm_init();
    if (pm_data->pst_wlan_pm_info == 0) {
        ps_print_err("wlan pm init failed!\n");
        goto WLAN_INIT_FAIL;
    }

    pm_data->firmware_cfg_init_flag = 0;
    pm_data->rcvdata_bef_devack_flag = 0;
    pm_data->bfgx_dev_state = BFGX_SLEEP;
    pm_data->bfgx_pm_ctrl_enable = BFGX_PM_DISABLE;
    pm_data->bfgx_lowpower_enable = BFGX_PM_ENABLE; /* enable host low_power function defaultly */

    pm_data->bfgx_bt_lowpower_enable = BFGX_PM_ENABLE;
    pm_data->bfgx_gnss_lowpower_enable = BFGX_PM_DISABLE;
    pm_data->bfgx_nfc_lowpower_enable = BFGX_PM_DISABLE;

    atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    atomic_set(&pm_data->bfg_needwait_devboot_flag, NEED_SET_FLAG);

    pm_data->uart_state = UART_NOT_READY;
    pm_data->uart_baud_switch_to = g_default_baud_rate;

    if (low_power_irq_init(pm_data) != OAL_SUCC) {
        goto REQ_IRQ_FAIL;
    }

    if (low_power_workq_init(pm_data) != OAL_SUCC) {
        goto CREATE_WORKQ_FAIL;
    }

    low_power_probe_timer_init(pm_data);

    /* set driver data */
    pm_set_drvdata(pm_data);

    /* register host pm */
    ret = register_pm_notifier(&g_pf_suspend_notifier);
    if (ret < 0) {
        ps_print_warning("%s : register_pm_notifier failed, ret = %d\n", __func__, ret);
    }

    return OAL_SUCC;

CREATE_WORKQ_FAIL:
    free_irq(pm_data->bfg_irq, NULL);
REQ_IRQ_FAIL:
WLAN_INIT_FAIL:
    kfree(pm_data);
PMDATA_MALLOC_FAIL:
    return -ENOMEM;
}

int low_power_init(void)
{
    int ret;

    ret = low_power_probe();
    if (ret != SUCCESS) {
        ps_print_err("low_power_init: low_power_probe fail\n");
    }

    ps_print_info("low_power_init: success\n");
    return ret;
}

void low_power_exit(void)
{
    low_power_remove();
    firmware_cfg_clear();
}

#ifdef CONFIG_HI110X_GPS_SYNC
#define GNSS_SYNC_IOREMAP_SIZE 0x1000
struct gnss_sync_data *g_gnss_sync_data_t = NULL;

struct gnss_sync_data *gnss_get_sync_data(void)
{
    return g_gnss_sync_data_t;
}

static void gnss_set_sync_data(struct gnss_sync_data *data)
{
    g_gnss_sync_data_t = data;
}

static int gnss_sync_probe(struct platform_device *pdev)
{
    struct gnss_sync_data *sync_info = NULL;
    struct device_node *np = pdev->dev.of_node;
    uint32 addr_base;
    int32 ret;
    uint32 version = 0;

    ps_print_info("[GPS] gnss sync probe start\n");

    ret = of_property_read_u32(np, "version", &version);
    if (ret != SUCCESS) {
        ps_print_err("[GPS] get gnss sync version failed!\n");
        return -FAILURE;
    }

    ps_print_info("[GPS] gnss sync version %d\n", version);
    if (version == 0) {
        return SUCCESS;
    }

    sync_info = kzalloc(sizeof(struct gnss_sync_data), GFP_KERNEL);
    if (sync_info == NULL) {
        ps_print_err("[GPS] alloc memory failed\n");
        return -ENOMEM;
    }

    sync_info->version = version;

    ret = of_property_read_u32(np, "addr_base", &addr_base);
    if (ret != SUCCESS) {
        ps_print_err("[GPS] get gnss sync reg base failed!\n");
        ret = -FAILURE;
        goto sync_get_resource_fail;
    }

    sync_info->addr_base_virt = ioremap(addr_base, GNSS_SYNC_IOREMAP_SIZE);
    if (sync_info->addr_base_virt == NULL) {
        ps_print_err("[GPS] gnss sync reg ioremap failed!\n");
        ret = -ENOMEM;
        goto sync_get_resource_fail;
    }

    ret = of_property_read_u32(np, "addr_offset", &sync_info->addr_offset);
    if (ret != SUCCESS) {
        ps_print_err("[GPS] get gnss sync reg offset failed!\n");
        ret = -FAILURE;
        goto sync_get_resource_fail;
    }

    ps_print_info("[GPS] gnss sync probe is finished!\n");

    gnss_set_sync_data(sync_info);

    return SUCCESS;

sync_get_resource_fail:
    gnss_set_sync_data(NULL);
    kfree(sync_info);
    return ret;
}

static void gnss_sync_shutdown(struct platform_device *pdev)
{
    struct gnss_sync_data *sync_info = gnss_get_sync_data();
    ps_print_info("[GPS] gnss sync shutdown!\n");

    if (sync_info == NULL) {
        ps_print_err("[GPS] gnss sync info is NULL.\n");
        return;
    }

    gnss_set_sync_data(NULL);
    kfree(sync_info);
    return;
}

#define DTS_COMP_GNSS_SYNC_NAME "hisilicon,hisi_gps_sync"

static const struct of_device_id g_gnss_sync_match_table[] = {
    {
        .compatible = DTS_COMP_GNSS_SYNC_NAME,  // compatible must match with which defined in dts
        .data = NULL,
    },
    {},
};

static struct platform_driver g_gnss_sync_driver = {
    .probe = gnss_sync_probe,
    .suspend = NULL,
    .remove = NULL,
    .shutdown = gnss_sync_shutdown,
    .driver = {
        .name = "hisi_gps_sync",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(g_gnss_sync_match_table),  // dts required code
    },
};

int gnss_sync_init(void)
{
    int ret;
    ret = platform_driver_register(&g_gnss_sync_driver);
    if (ret) {
        ps_print_err("[GPS] unable to register gnss sync driver!\n");
    } else {
        ps_print_info("[GPS] gnss sync init ok!\n");
    }
    return ret;
}

void gnss_sync_exit(void)
{
    platform_driver_unregister(&g_gnss_sync_driver);
}
#endif

