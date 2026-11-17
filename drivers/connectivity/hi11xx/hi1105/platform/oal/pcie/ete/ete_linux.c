

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#define HI11XX_LOG_MODULE_NAME "[PCIE_H]"
#define HISI_LOG_TAG           "[PCIE]"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/err.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/pci.h>
#include "board.h"
#include "plat_pm.h"
#endif
#include "oal_util.h"
#include "oal_net.h"
#include "oal_ext_if.h"
#include "hisi_ini.h"
#include "oal_thread.h"
#include "oam_ext_if.h"
#include "pcie_host.h"
#include "pcie_linux.h"
#include "oal_hcc_host_if.h"

OAL_STATIC int32_t oal_pcie_ete_tx_netbuf(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head, hcc_netbuf_queue_type qtype)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    ret = oal_pcie_check_tx_param(pst_bus, pst_head, qtype);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return 0;
    }

    ret = oal_ete_send_netbuf_list(pst_pci_lres->pst_pci_res, pst_head, qtype);
    return ret;
}

OAL_STATIC int32_t oal_pcie_ete_sleep_request_host(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    return oal_ete_sleep_request_host_check(pst_pci_lres->pst_pci_res);
}

#ifdef CONFIG_ARCH_KIRIN_PCIE
OAL_STATIC int32_t oal_ete_sr_wakeup_request_enable(oal_pcie_linux_res *pst_pci_lres,
                                                    hcc_bus *pst_bus)
{
    int32_t ret;

    if (oal_pcie_check_link_state(pst_pci_lres->pst_pci_res) == OAL_FALSE) {
        uint32_t version = 0x0;
        ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev, 0x0, &version);
        pci_print_log(PCI_LOG_ERR, "mem access error, maybe linkdown! config read version :0x%x, ret=%d", version, ret);
        return -OAL_ENODEV;
    }

    /* 唤醒流程，RES已经初始化 */
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_RES_UP);

    oal_atomic_set(&pst_pci_lres->st_pcie_wakeup_flag, 1);
    ret = oal_pcie_enable_device_func(pst_pci_lres);
    oal_atomic_set(&pst_pci_lres->st_pcie_wakeup_flag, 0);
    if (ret != OAL_SUCC) {
        oal_pcie_disable_regions(pst_pci_lres->pst_pci_res);
        return ret;
    }

    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_WORK_UP);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

    hcc_bus_enable_state(pst_bus, OAL_BUS_STATE_ALL);
    pst_pci_lres->power_status = PCIE_EP_IP_POWER_UP;

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_ete_sr_wakeup_request(oal_pcie_linux_res *pst_pci_lres,
                                             hcc_bus *pst_bus)
{
    int32_t ret;
    uint32_t ret_timeout;
    declare_time_cost_stru(cost);
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_get_time_cost_start(cost);

    if (pst_wlan_pm != NULL) {
        oal_init_completion(&pst_wlan_pm->st_wifi_powerup_done);
    }

    oal_atomic_set(&g_bus_powerup_dev_wait_ack, 1);
    oal_pcie_device_wakeup_handler(NULL);
    if (pst_wlan_pm != NULL) {
        ret_timeout = oal_wait_for_completion_timeout(&pst_wlan_pm->st_wifi_powerup_done,
                                                      (uint32_t)oal_msecs_to_jiffies(2000));
        if (oal_unlikely(ret_timeout == 0)) {
            /* 超时不做处理继续尝试建链 */
            declare_dft_trace_key_info("pcie_powerup_wakeup ack timeout", OAL_DFT_TRACE_FAIL);
        }
    } else {
        oal_msleep(100); // 100ms dealy if pm is null(powerup)
    }

    oal_atomic_set(&g_bus_powerup_dev_wait_ack, 0);

    ret = oal_pcie_reinit(pst_bus);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, "oal_pcie_reinit failed!ret=%d", ret);
        return ret;
    }

    oal_pcie_host_ip_init(pst_pci_lres);

    ret = oal_ete_transfer_deepwkup_restore(pst_pci_lres->pst_pci_res);
    if (ret != OAL_SUCC) {
        pci_print_log(PCI_LOG_ERR, " ete deep wakeup failed, ret=%d", ret);
        return ret;
    }

    ret = oal_ete_sr_wakeup_request_enable(pst_pci_lres, pst_bus);
    if (ret != OAL_SUCC) {
        return ret;
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    pci_print_log(PCI_LOG_INFO, "pcie power up init succ, cost %llu us\n", time_cost_var_sub(cost));

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_ete_wakeup_request(hcc_bus *pst_bus)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    // 1.拉高 Host WakeUp Device gpio
    // 2.调用kirin_pcie_pm_control 上电RC 检查建链
    // 3.restore state, load iatu config
    if (oal_unlikely(pst_pci_lres->pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        pci_print_log(PCI_LOG_WARN, "link invaild, wakeup failed, link_state:%s",
                      oal_pcie_get_link_state_str(pst_pci_lres->pst_pci_res->link_state));
        return -OAL_ENODEV;
    }

    if (pst_pci_lres->power_status == PCIE_EP_IP_POWER_DOWN) {
        ret = oal_ete_sr_wakeup_request(pst_pci_lres, pst_bus);
        if (ret != OAL_SUCC) {
            return ret;
        }
    } else {
        /* 正常单芯片唤醒拉高GPIO即可 */
        oal_pcie_device_wakeup_handler(NULL);
        pci_set_master(pst_pci_lres->pst_pcie_dev);
    }
    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_ete_wake_powerup_handler(void *data)
{
    uint32_t ret;
    uint32_t is_master;
    uint32_t retry_cnt = 0;
    const uint32_t max_retry_cnt = 3;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    oal_pci_dev_stru *pst_pci_dev = (oal_pci_dev_stru *)data;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);

    is_master = oal_pcie_is_master_ip(pst_pci_dev);
    if (is_master == OAL_TRUE) {
        declare_dft_trace_key_info("pcie_resume_master_powerup", OAL_DFT_TRACE_SUCC);
    } else {
        declare_dft_trace_key_info("pcie_resume_dual_powerup", OAL_DFT_TRACE_SUCC);
    }

    if (pst_wlan_pm == NULL) {
        pci_print_log(PCI_LOG_INFO, "pst_wlan_pm is null wakeup failed");
        return 0;
    }

    /* for dual pcie */
    mutex_lock(&pst_pci_lres->sr_lock);
    if (board_get_host_wakeup_dev_stat() == 0) {
retry:
        pci_print_log(PCI_LOG_INFO, "pcie_resume_powerup pull up gpio,retry_cnt=%u", retry_cnt);

        oal_init_completion(&pst_wlan_pm->st_wifi_powerup_done);

        oal_atomic_set(&g_bus_powerup_dev_wait_ack, 1);
        /* 这里保证解复位EP控制器时efuse已经稳定 */
        board_host_wakeup_dev_set(1);
        ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_wifi_powerup_done,
                                              (uint32_t)oal_msecs_to_jiffies(2000)); // 超时2000ms
        if (oal_unlikely(ret == 0)) { // timeout
            declare_dft_trace_key_info("pcie_resume_powerup ack timeout", OAL_DFT_TRACE_FAIL);
            (void)ssi_dump_err_regs(SSI_ERR_PCIE_SR_WAKEUP_FAIL);
            /* 最多尝试的次数，超过则出错 */
            if (retry_cnt++ < max_retry_cnt) {
                pci_print_log(PCI_LOG_INFO, "pull down wakeup gpio and retry");
                board_host_wakeup_dev_set(0);
                oal_msleep(5);
                goto retry;
            } else {
                declare_dft_trace_key_info("pcie_resume_powerup_ack_timeout_retry_failed", OAL_DFT_TRACE_FAIL);
                (void)ssi_dump_err_regs(SSI_ERR_PCIE_SR_WAKEUP_RETRY_FAIL);
            }
        } else {
            pci_print_log(PCI_LOG_INFO, "powerup done");
        }

        oal_atomic_set(&g_bus_powerup_dev_wait_ack, 0);
    } else {
        // WL_HOST_WAKEUP high, need't wakeup
        pci_print_log(PCI_LOG_INFO, "host wakeup gpio is high");
    }
    mutex_unlock(&pst_pci_lres->sr_lock);

    /* pull up pcie_rst pin
     * 拉高pcie_rst管脚,不需要等待pcie解复位完成,直接尝试建链 */
    board_set_pcie_reset(is_master, OAL_TRUE);

    return 0;
}

OAL_STATIC int32_t oal_pcie_ete_suspend_handler(void *data)
{
    oal_pci_dev_stru *pst_pci_dev = (oal_pci_dev_stru *)data;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    int32_t rc_idx = oal_pcie_get_pcie_rc_idx(pst_pci_dev);

    pst_pci_lres->power_status = PCIE_EP_IP_POWER_DOWN;
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DEEPSLEEP);

    pci_print_log(PCI_LOG_INFO, "pcie%d_suspend refcnt=%d pci_cnt=%d", rc_idx,
                  oal_atomic_read(&pst_pci_lres->refcnt), pst_pci_lres->pci_cnt);

    if (oal_pcie_is_master_ip(pst_pci_dev) == OAL_TRUE) {
        declare_dft_trace_key_info("pcie_suspend_master_powerdown", OAL_DFT_TRACE_SUCC);
        oal_ete_transfer_deepsleep_clean(pst_pci_lres->pst_pci_res);
    } else {
        declare_dft_trace_key_info("pcie_suspend_dual_powerdown", OAL_DFT_TRACE_SUCC);
    }

    /* pull down pcie_rst pin */
    board_set_pcie_reset(oal_pcie_is_master_ip(pst_pci_dev), OAL_FALSE);
#ifdef CONFIG_ARCH_KIRIN_PCIE
    kirin_pcie_power_notifiy_register(rc_idx,
                                      oal_pcie_ete_wake_powerup_handler, NULL, (void*)pst_pci_dev);
#endif
    return 0;
}

#else
OAL_STATIC int32_t oal_ete_wakeup_request(hcc_bus *pst_bus)
{
    oal_reference(pst_bus);
    return OAL_SUCC;
}
#endif

OAL_STATIC int32_t oal_pcie_ete_wakeup_complete(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("pst_pci_lres is null\n");
        return -OAL_EINVAL;
    }

    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_WORK_UP);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

    if (g_pcie_auto_disable_aspm != 0) {
        /* disable L1 after wakeup */
        oal_pcie_set_device_aspm_dync_disable(pst_pci_lres->pst_pci_res, OAL_TRUE);
        oal_pcie_device_xfer_pending_sig_clr(pst_pci_lres->pst_pci_res, OAL_FALSE);
    }

    oal_enable_pcie_irq(pst_pci_lres);

    oal_ete_sched_rx_threads(pst_pci_lres->pst_pci_res);

    return OAL_SUCC;
}

/* 双PCIE suspend函数会并发，函数需要考虑可重入 */
int32_t oal_pcie_ete_suspend(oal_pci_dev_stru *pst_pci_dev, oal_pm_message_t state)
{
    int32_t ret;
    int32_t down_flag = 0;
    hcc_bus *pst_bus = NULL;
    struct hcc_handler *pst_hcc = NULL;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);

    ret = oal_pcie_sr_para_check(pst_pci_lres, &pst_bus, &pst_hcc);
    if (ret != OAL_SUCC) {
        if (ret == -OAL_ENODEV) {
            return OAL_SUCC;
        } else {
            return ret;
        }
    }

    pci_print_log(PCI_LOG_INFO, "oal_pcie_suspend name=%s", dev_name(&pst_pci_dev->dev));

    if (atomic_inc_return(&pst_pci_lres->refcnt) == 1) {
        /* first pcie suspend */
        if (down_interruptible(&pst_bus->sr_wake_sema)) {
            oal_io_print(KERN_ERR "pcie_wake_sema down failed.");
            oal_atomic_dec(&pst_pci_lres->refcnt);
            return -OAL_EFAIL;
        }
        down_flag = 1;
    }

    if (oal_pcie_wakelock_active(pst_bus) == OAL_TRUE) {
        /* has wake lock so stop controller's suspend,
         * otherwise controller maybe error while sdio reinit */
        pci_print_log(PCI_LOG_INFO, "already wake up, break suspend");
        if (down_flag == 1) {
            up(&pst_bus->sr_wake_sema);
        }
        oal_atomic_dec(&pst_pci_lres->refcnt);
        return -OAL_EFAIL;
    }

    if (down_flag == 1) {
        wlan_pm_wkup_src_debug_set(OAL_TRUE);
    }

    declare_dft_trace_key_info("pcie_os_suspend", OAL_DFT_TRACE_SUCC);
    oal_pcie_save_default_resource(pst_pci_lres);
#ifdef CONFIG_ARCH_KIRIN_PCIE
    kirin_pcie_power_notifiy_register(oal_pcie_get_pcie_rc_idx(pst_pci_dev),
                                      NULL, oal_pcie_ete_suspend_handler, (void *)pst_pci_dev);
#endif
    return 0;
}

static uint32_t g_is_resume_failed = OAL_FALSE;
OAL_STATIC int32_t oal_pcie_ete_resume_linkup_check(oal_pcie_linux_res *pst_pci_lres,
                                                    oal_pci_dev_stru *pst_pci_dev)
{
    uint32_t version = 0x0;
    int32_t ret = oal_pci_read_config_dword(pst_pci_dev, 0x0, &version);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "read pci version failed ret=%d, name=%s",
                      ret, dev_name(&pst_pci_dev->dev));
        oal_msleep(1000);
        ret = oal_pci_read_config_dword(pst_pci_dev, 0x0, &version);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "read pci version failed ret=%d", ret);
        } else {
            pci_print_log(PCI_LOG_WARN, "version:0x%x is not match with:0x%x", version, pst_pci_lres->version);
        }

        g_is_resume_failed = OAL_TRUE;
        if (oal_atomic_read(&pst_pci_lres->refcnt) == 0) {
            return -OAL_EFAIL; // exception handler
        }
        return -OAL_ENODEV; // wait dual pcie handler done, bypass err
    }

    //  master pcie
    if (pst_pci_dev == pst_pci_lres->pst_pcie_dev) {
        if (version != pst_pci_lres->version) {
            pci_print_log(PCI_LOG_WARN, "version:0x%x is not match with:0x%x", version, pst_pci_lres->version);
        }
    }

    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_ete_resume_hold_sleep(oal_pcie_linux_res *pst_pci_lres,
                                               hcc_bus *pst_bus)
{
    if (oal_atomic_read(&pst_pci_lres->refcnt) == 0) {
        // resume all done, allow device deepsleep
        board_host_wakeup_dev_set(0);
        oal_usleep(WLAN_WAKEUP_DEV_EVENT_DELAY_US);
        pci_print_log(PCI_LOG_INFO, "oal_pcie_ete_resume done");
        up(&pst_bus->sr_wake_sema);
    }
}

int32_t oal_pcie_ete_resume(oal_pci_dev_stru *pst_pci_dev)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;
    struct hcc_handler *pst_hcc = NULL;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);

    ret = oal_pcie_sr_para_check(pst_pci_lres, &pst_bus, &pst_hcc);
    if (ret != OAL_SUCC) {
        if (ret == -OAL_ENODEV) {
            return OAL_SUCC;
        } else {
            return ret;
        }
    }

    declare_dft_trace_key_info("pcie_os_resume", OAL_DFT_TRACE_SUCC);

    mutex_lock(&pst_pci_lres->sr_lock);
    // last pcie resume
    if (atomic_dec_return(&pst_pci_lres->refcnt) == 0) {
        if (g_is_resume_failed == OAL_TRUE) {
            goto resume_exception;
        }
    }

    if (pst_pci_lres->power_status != PCIE_EP_IP_POWER_DOWN) {
        pci_print_log(PCI_LOG_INFO, "oal_pcie_resume");
        if (oal_atomic_read(&pst_pci_lres->refcnt) == 0) {
            up(&pst_bus->sr_wake_sema);
        }
        mutex_unlock(&pst_pci_lres->sr_lock);
        return OAL_SUCC;
    }

    // linkup failed
    ret = oal_pcie_ete_resume_linkup_check(pst_pci_lres, pst_pci_dev);
    if (ret != OAL_SUCC) {
        if (ret == -OAL_ENODEV) {
            mutex_unlock(&pst_pci_lres->sr_lock);
            return OAL_SUCC;
        } else {
            goto resume_exception;
        }
    }

    oal_pcie_ete_resume_hold_sleep(pst_pci_lres, pst_bus);

    mutex_unlock(&pst_pci_lres->sr_lock);
    return OAL_SUCC;
resume_exception:
    pci_print_log(PCI_LOG_INFO, "resume exception");
    g_is_resume_failed = OAL_FALSE;
    up(&pst_bus->sr_wake_sema);
    mutex_unlock(&pst_pci_lres->sr_lock);
    hcc_bus_exception_submit(pst_pci_lres->pst_bus, WIFI_TRANS_FAIL);
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_RESUME_FIRMWARE_DOWN);
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_host_ip_ete_exit(oal_pcie_linux_res *pst_pci_lres)
{
    if (pst_pci_lres->pst_pci_res->chip_info.ete_support != OAL_TRUE) {
        return;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_host_ip_ete_exit");
    (void)oal_ete_ip_exit(pst_pci_lres->pst_pci_res);
}

OAL_STATIC OAL_INLINE void oal_ete_bus_power_down(oal_pcie_linux_res *pst_pci_lres, hcc_bus *pst_bus)
{
    /* should stop ete when wifi poweroff too */
    oal_pcie_host_ip_ete_exit(pst_pci_lres);
    pcie_bus_power_down_action(pst_bus);
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
OAL_STATIC OAL_INLINE int32_t oal_ete_dual_pci_power_up(oal_pcie_linux_res *pst_pci_lres)
{
    if (pst_pci_lres->pst_pcie_dev_dual == NULL) {
        /* no dual pci */
        return OAL_SUCC;
    }

    oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev_dual, g_kirin_rc_idx_dual,
                                    oal_pcie_dual_power_on_callback, NULL, NULL);
    return oal_pcie_pm_control(pst_pci_lres->pst_pcie_dev_dual, g_kirin_rc_idx_dual, PCIE_POWER_ON);
}
#endif

OAL_STATIC OAL_INLINE int32_t oal_ete_bus_power_up(oal_pcie_linux_res *pst_pci_lres, hcc_bus *pst_bus)
{
    int32_t ret = OAL_SUCC;
    /* 上电之前打开PCIE HOST 控制器 */
#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
    declare_time_cost_stru(cost);
    oal_get_time_cost_start(cost);

    pst_pci_lres->l1_err_cnt = 0;

    ret = oal_pcie_pm_control(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, PCIE_POWER_ON);
    if (g_pcie_print_once) {
        g_pcie_print_once = 0;
        ssi_dump_device_regs(SSI_MODULE_MASK_COMM |
                             SSI_MODULE_MASK_PCIE_CFG |
                             SSI_MODULE_MASK_PCIE_DBI |
                             SSI_MODULE_MASK_WCTRL |
                             SSI_MODULE_MASK_BCTRL);
    }
    board_host_wakeup_dev_set(0);
    oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, NULL, NULL, NULL);
    if (ret) {
        oal_io_print(KERN_ERR "pcie power on and link failed, ret=%d\n", ret);
        (void)ssi_dump_err_regs(SSI_ERR_PCIE_POWER_UP_FAIL);
        return ret;
    }

    oal_atomic_set(&pst_pci_lres->refcnt, 0);
    pst_pci_lres->pci_cnt = 1; // 1 pcie ip

#if !defined(_PRE_HI375X_PCIE) && !defined(_PRE_PRODUCT_HI1620S_KUNPENG)
    kirin_pcie_register_event(&pst_pci_lres->pcie_event);
#endif
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    ret = oal_ete_dual_pci_power_up(pst_pci_lres);
    if (ret) {
        return ret;
    } else {
        pst_pci_lres->pci_cnt++;
    }
#endif

    pst_pci_lres->power_status = PCIE_EP_IP_POWER_UP;
    if (pst_pci_lres->pst_pci_res) {
        mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
        oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_UP);
        mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    }

    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    pci_print_log(PCI_LOG_INFO, "pcie link cost %llu us", time_cost_var_sub(cost));
#endif
    return ret;
}

OAL_STATIC OAL_INLINE int32_t oal_ete_bus_power_patch_lauch(oal_pcie_linux_res *pst_pci_lres, hcc_bus *pst_bus)
{
    int32_t ret;
    unsigned int timeout = hi110x_get_emu_timeout(HOST_WAIT_BOTTOM_INIT_TIMEOUT);
    oal_io_print("power patch lauch\n");

    /* Patch下载完后 初始化通道资源，然后等待业务初始化完成 */
    ret = oal_pcie_transfer_res_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        oal_io_print(KERN_ERR "pcie_transfer_res_init failed, ret=%d\n", ret);
        return ret;
    }

    oal_pcie_host_ip_init(pst_pci_lres);

    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_TRUE);

    oal_enable_pcie_irq(pst_pci_lres);

    ret = oal_pcie_enable_device_func(pst_pci_lres);
    if (ret != OAL_SUCC) {
        oal_io_print("enable pcie device func failed, ret=%d\n", ret);
        return ret;
    }

    if (oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
                                        (uint32_t)oal_msecs_to_jiffies(timeout)) == 0) {
        oal_io_print(KERN_ERR "wait device ready timeout... %d ms \n", timeout);
        up(&pst_bus->rx_sema);
        if (oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
                                            (uint32_t)oal_msecs_to_jiffies(5000)) == 0) {
            oal_io_print(KERN_ERR "retry 5 second hold, still timeout");
            return -OAL_ETIMEDOUT;
        } else {
            /* 强制调度成功，说明有可能是GPIO中断未响应 */
            oal_io_print(KERN_WARNING "[E]retry succ, maybe gpio interrupt issue");
            declare_dft_trace_key_info("pcie gpio int issue", OAL_DFT_TRACE_FAIL);
        }
    }

    /* ringbuf rebuild, after device ete late init! after ete clk en */
    ret = oal_ete_rx_ringbuf_build(pst_pci_lres->pst_pci_res);
    if (ret != OAL_SUCC) {
        declare_dft_trace_key_info("oal_ete_rx_ringbuf_build failed", OAL_DFT_TRACE_FAIL);
    }

    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_WORK_UP);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

    hcc_enable(hbus_to_hcc(pst_bus), OAL_TRUE);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_ete_power_action(hcc_bus *pst_bus, hcc_bus_power_action_type action)
{
    int32_t ret = OAL_SUCC;
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    if (action == HCC_BUS_POWER_DOWN) {
        oal_ete_bus_power_down(pst_pci_lres, pst_bus);
    }

    if (action == HCC_BUS_POWER_UP) {
        ret = oal_ete_bus_power_up(pst_pci_lres, pst_bus);
    }

    if (action == HCC_BUS_POWER_PATCH_LOAD_PREPARE) {
        /* close hcc */
        hcc_disable(hbus_to_hcc(pst_bus), OAL_TRUE);
        oal_init_completion(&pst_bus->st_device_ready);
        oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_FALSE);
    }

    if (action == HCC_BUS_POWER_PATCH_LAUCH) {
        ret = oal_ete_bus_power_patch_lauch(pst_pci_lres, pst_bus);
    }

    return ret;
}

OAL_STATIC int32_t oal_pcie_ete_tx_condition(hcc_bus *pst_bus, hcc_netbuf_queue_type qtype)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("pci linux res is null\n");
        return OAL_FALSE;
    }

    return oal_pcie_ete_tx_is_idle(pst_pci_lres->pst_pci_res, qtype);
}

OAL_STATIC void oal_ete_print_trans_info(hcc_bus *hi_bus, uint64_t print_flag)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    oal_ete_print_transfer_info(pst_pci_lres->pst_pci_res, print_flag);
}

OAL_STATIC void oal_ete_reset_trans_info(hcc_bus *hi_bus)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    oal_ete_reset_transfer_info(pst_pci_lres->pst_pci_res);
}

OAL_STATIC int32_t oal_pcie_ete_pending_signal_check(hcc_bus *hi_bus)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_unlikely(pst_pci_lres == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return OAL_FALSE;
    }
    return oal_ete_host_pending_signal_check(pst_pci_lres->pst_pci_res);
}

OAL_STATIC int32_t oal_pcie_ete_pending_signal_process(hcc_bus *hi_bus)
{
    int32_t ret = 0;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_unlikely(pst_pci_lres == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return 0;
    }

    if (OAL_TRUE == oal_ete_host_pending_signal_check(pst_pci_lres->pst_pci_res)) {
        hcc_tx_transfer_lock(hi_bus->bus_dev->hcc);
        if (OAL_TRUE == oal_ete_host_pending_signal_check(pst_pci_lres->pst_pci_res)) { /* for wlan power off */
            ret = oal_ete_host_pending_signal_process(pst_pci_lres->pst_pci_res);
        } else {
            pci_print_log(PCI_LOG_INFO, "pcie tx pending signal was cleared");
        }
        hcc_tx_transfer_unlock(hi_bus->bus_dev->hcc);
    }
    return ret;
}

OAL_STATIC int32_t oal_pcie_master_address_switch(hcc_bus *hi_bus, uint64_t src_addr,
                                                  uint64_t* dst_addr, int32_t is_host_va)
{
    int32_t ret;
    pci_addr_map addr_map;

    oal_pcie_res *pst_pci_res = NULL;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_unlikely(pst_pci_lres == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return -OAL_ENODEV;
    }

    pst_pci_res = pst_pci_lres->pst_pci_res;

    if (oal_unlikely(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_res is null");
        return -OAL_ENODEV;
    }

    if (oal_warn_on(dst_addr == NULL)) {
        return -OAL_EINVAL;
    }

    *dst_addr = 0x0;

    if (is_host_va == OAL_FALSE) {
        /* device cpu address -> host virtual address */
        ret = oal_pcie_inbound_ca_to_va(pst_pci_res, src_addr, &addr_map);
        if (ret == OAL_SUCC) {
            *dst_addr = (uint64_t)addr_map.va;
        }
        return ret;
    } else {
        /* host virtual address -> device cpu address */
        ret = oal_pcie_inbound_va_to_ca(pst_pci_res, src_addr, dst_addr);
        return ret;
    }
}

OAL_STATIC int32_t oal_pcie_slave_address_switch(hcc_bus *hi_bus, uint64_t src_addr,
                                                 uint64_t* dst_addr, int32_t is_host_iova)
{
    oal_pcie_res *pst_pci_res = NULL;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_unlikely(pst_pci_lres == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return -OAL_ENODEV;
    }

    pst_pci_res = pst_pci_lres->pst_pci_res;

    if (oal_unlikely(pst_pci_res == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_res is null");
        return -OAL_ENODEV;
    }

    if (oal_warn_on(dst_addr == NULL)) {
        return -OAL_EINVAL;
    }

    return oal_pcie_host_slave_address_switch(pst_pci_res, src_addr, dst_addr, is_host_iova);
}

OAL_STATIC hcc_bus_opt_ops g_pcie_ete_opt_ops = {
    .get_bus_state = oal_pcie_get_state,
    .disable_bus_state = oal_disable_pcie_state,
    .enable_bus_state = oal_enable_pcie_state,
    .rx_netbuf_list = oal_pcie_rx_netbuf,
    .tx_netbuf_list = oal_pcie_ete_tx_netbuf,
    .send_msg = oal_pcie_send_msg,
    .lock = oal_pcie_host_lock,
    .unlock = oal_pcie_host_unlock,
    .sleep_request = oal_pcie_sleep_request,
    .sleep_request_host = oal_pcie_ete_sleep_request_host,
    .wakeup_request = oal_ete_wakeup_request,
    .get_sleep_state = oal_pcie_get_sleep_state,
    .wakeup_complete = oal_pcie_ete_wakeup_complete,
    .rx_int_mask = oal_pcie_rx_int_mask,
    .power_action = oal_pcie_ete_power_action,
    .power_ctrl = oal_pcie_power_ctrl,
    .wlan_gpio_handler = oal_pcie_gpio_irq,
    .wlan_gpio_rxdata_proc = oal_pcie_gpio_rx_data,
    .reinit = oal_pcie_reinit,
    .deinit = oal_pcie_deinit,
    .tx_condition = oal_pcie_ete_tx_condition,
    .patch_read = oal_pcie_patch_read,
    .patch_write = oal_pcie_patch_write,
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
    .bindcpu = NULL,
#else
    .bindcpu = oal_pcie_bindcpu,
#endif
    .get_trans_count = NULL,
    .voltage_bias_init = NULL,
    .chip_info = NULL,

    .print_trans_info = oal_ete_print_trans_info,
    .reset_trans_info = oal_ete_reset_trans_info,
    .pending_signal_check = oal_pcie_ete_pending_signal_check,
    .pending_signal_process = oal_pcie_ete_pending_signal_process,
    .master_address_switch = oal_pcie_master_address_switch,
    .slave_address_switch = oal_pcie_slave_address_switch,
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
    .wlan_pm_vote = oal_pcie_wlan_pm_vote,
#endif
};

void oal_pcie_ete_bus_ops_init(hcc_bus *pst_bus)
{
    pst_bus->opt_ops = &g_pcie_ete_opt_ops;
}

#endif

