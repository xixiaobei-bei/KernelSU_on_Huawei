

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

OAL_STATIC pcie_h2d_ringbuf_qtype g_pcie_hcc2edma_qmap[HCC_NETBUF_QUEUE_BUTT] = {
    [HCC_NETBUF_NORMAL_QUEUE] = PCIE_H2D_QTYPE_NORMAL,
    [HCC_NETBUF_HIGH_QUEUE] = PCIE_H2D_QTYPE_HIGH,
    [HCC_NETBUF_HIGH2_QUEUE] = PCIE_H2D_QTYPE_NORMAL,
};

OAL_STATIC pcie_h2d_ringbuf_qtype oal_pcie_hcc_qtype_to_edma_qtype(hcc_netbuf_queue_type qtype)
{
    return g_pcie_hcc2edma_qmap[qtype];
}

OAL_STATIC int32_t oal_pcie_edma_tx_netbuf(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head,
                                           hcc_netbuf_queue_type qtype)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    ret = oal_pcie_check_tx_param(pst_bus, pst_head, qtype);
    if (oal_unlikely(ret != OAL_SUCC)) {
        return 0;
    }

    ret = oal_pcie_send_netbuf_list(pst_pci_lres->pst_pci_res, pst_head, oal_pcie_hcc_qtype_to_edma_qtype(qtype));
    return ret;
}

OAL_STATIC int32_t oal_pcie_edma_sleep_request_host(hcc_bus *pst_bus)
{
    /* check host sleep condtion */
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    return oal_pcie_edma_sleep_request_host_check(pst_pci_lres->pst_pci_res);
}

OAL_STATIC int32_t oal_pcie_edma_wakeup_complete(hcc_bus *pst_bus)
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

    oal_pcie_shced_rx_hi_thread(pst_pci_lres->pst_pci_res);

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE int32_t oal_edma_bus_power_patch_lauch(oal_pcie_linux_res *pst_pci_lres, hcc_bus *pst_bus)
{
    int32_t ret;
    oal_io_print("power patch lauch\n");

    /* Patch下载完后 初始化通道资源，然后等待业务初始化完成 */
    ret = oal_pcie_transfer_res_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        oal_io_print(KERN_ERR "pcie_transfer_res_init failed, ret=%d\n", ret);
        return ret;
    }

    oal_pcie_host_ip_init(pst_pci_lres);

    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_TRUE);

#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
    oal_enable_pcie_irq_with_request(pst_pci_lres);
#else
    oal_enable_pcie_irq(pst_pci_lres);
#endif

    ret = oal_pcie_enable_device_func(pst_pci_lres);
    if (ret != OAL_SUCC) {
        oal_io_print("enable pcie device func failed, ret=%d\n", ret);
        return ret;
    }

    if (oal_wait_for_completion_timeout(&pst_bus->st_device_ready,
                                        (uint32_t)oal_msecs_to_jiffies(HOST_WAIT_BOTTOM_INIT_TIMEOUT)) == 0) {
        oal_io_print(KERN_ERR "wait device ready timeout... %d ms \n", HOST_WAIT_BOTTOM_INIT_TIMEOUT);
        up(&pst_bus->rx_sema);
        if (oal_wait_for_completion_timeout(&pst_bus->st_device_ready, (uint32_t)oal_msecs_to_jiffies(5000)) == 0) {
            oal_io_print(KERN_ERR "retry 5 second hold, still timeout");
            return -OAL_ETIMEDOUT;
        } else {
            /* 强制调度成功，说明有可能是GPIO中断未响应 */
            oal_io_print(KERN_WARNING "[E]retry succ, maybe gpio interrupt issue");
            declare_dft_trace_key_info("pcie gpio int issue", OAL_DFT_TRACE_FAIL);
        }
    }

    if (g_pcie_ringbuf_bugfix_enable == 0) {
        ret = oal_pcie_ringbuf_h2d_refresh(pst_pci_lres->pst_pci_res);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_WORK_UP);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

    hcc_enable(hbus_to_hcc(pst_bus), OAL_TRUE);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_edma_power_action(hcc_bus *pst_bus, hcc_bus_power_action_type action)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    oal_print_inject_check_stack();

    if (action == HCC_BUS_POWER_DOWN) {
        pcie_bus_power_down_action(pst_bus);
    }

    if (action == HCC_BUS_POWER_UP) {
        /* 上电之前打开PCIE HOST 控制器 */
#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
        declare_time_cost_stru(cost);
        if (action == HCC_BUS_POWER_UP) {
            oal_get_time_cost_start(cost);
        }

        pst_pci_lres->l1_err_cnt = 0;

        ret = oal_pcie_pm_control(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, PCIE_POWER_ON);
        if (g_pcie_print_once) {
            g_pcie_print_once = 0;
            ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI |
              SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
        }
        board_host_wakeup_dev_set(0);
        oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, NULL, NULL, NULL);
        if (ret) {
            oal_io_print(KERN_ERR "pcie power on and link failed, ret=%d\n", ret);
            (void)ssi_dump_err_regs(SSI_ERR_PCIE_POWER_UP_FAIL);
            return ret;
        }

#if !defined(_PRE_HI375X_PCIE) && !defined(_PRE_PRODUCT_HI1620S_KUNPENG)
        kirin_pcie_register_event(&pst_pci_lres->pcie_event);
#endif

        pst_pci_lres->power_status = PCIE_EP_IP_POWER_UP;
        if (pst_pci_lres->pst_pci_res) {
            mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
            oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_UP);
            mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
        }

        if (action == HCC_BUS_POWER_UP) {
            oal_get_time_cost_end(cost);
            oal_calc_time_cost_sub(cost);
            pci_print_log(PCI_LOG_INFO, "pcie link cost %llu us", time_cost_var_sub(cost));
        }
#endif
    }

    if (action == HCC_BUS_POWER_PATCH_LOAD_PREPARE) {
        /* close hcc */
        hcc_disable(hbus_to_hcc(pst_bus), OAL_TRUE);
        oal_init_completion(&pst_bus->st_device_ready);
        oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_FALSE);
    }

    if (action == HCC_BUS_POWER_PATCH_LAUCH) {
        ret = oal_edma_bus_power_patch_lauch(pst_pci_lres, pst_bus);
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_edma_tx_condition(hcc_bus *pst_bus, hcc_netbuf_queue_type qtype)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("pci linux res is null\n");
        return OAL_FALSE;
    }

    return oal_pcie_edma_tx_is_idle(pst_pci_lres->pst_pci_res, oal_pcie_hcc_qtype_to_edma_qtype(qtype));
}

OAL_STATIC int32_t oal_pcie_edma_pending_signal_check(hcc_bus *hi_bus)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_unlikely(pst_pci_lres == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return OAL_FALSE;
    }
    return oal_pcie_host_pending_signal_check(pst_pci_lres->pst_pci_res);
}

OAL_STATIC int32_t oal_pcie_edma_pending_signal_process(hcc_bus *hi_bus)
{
    int32_t ret = 0;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_unlikely(pst_pci_lres == NULL)) {
        pci_print_log(PCI_LOG_INFO, "pst_pci_lres is null");
        return 0;
    }

    if (OAL_TRUE == oal_pcie_host_pending_signal_check(pst_pci_lres->pst_pci_res)) {
        hcc_tx_transfer_lock(hi_bus->bus_dev->hcc);
        if (OAL_TRUE == oal_pcie_host_pending_signal_check(pst_pci_lres->pst_pci_res)) { /* for wlan power off */
            ret = oal_pcie_host_pending_signal_process(pst_pci_lres->pst_pci_res);
        } else {
            pci_print_log(PCI_LOG_INFO, "pcie tx pending signal was cleared");
        }
        hcc_tx_transfer_unlock(hi_bus->bus_dev->hcc);
    }
    return ret;
}

#ifdef CONFIG_ARCH_KIRIN_PCIE
OAL_STATIC int32_t oal_pcie_edma_wake_powerup_handler(void *data)
{
    uint32_t ul_ret;
    uint32_t ul_retry_cnt = 0;
    const uint32_t ul_max_retry_cnt = 3;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    oal_reference(data);
    declare_dft_trace_key_info("pcie_resume_powerup", OAL_DFT_TRACE_SUCC);
retry:
    pci_print_log(PCI_LOG_INFO, "pcie_resume_powerup pull up gpio,retry_cnt=%u", ul_retry_cnt);
    if (pst_wlan_pm != NULL) {
        oal_init_completion(&pst_wlan_pm->st_wifi_powerup_done);
    }

    oal_atomic_set(&g_bus_powerup_dev_wait_ack, 1);
    /* 这里保证解复位EP控制器时efuse已经稳定 */
    board_host_wakeup_dev_set(1);
    if (pst_wlan_pm != NULL) {
        ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_wifi_powerup_done,
                                                 (uint32_t)oal_msecs_to_jiffies(2000));
        if (oal_unlikely(ul_ret == 0)) {
            /* 超时不做处理继续尝试建链 */
            declare_dft_trace_key_info("pcie_resume_powerup ack timeout", OAL_DFT_TRACE_FAIL);
            (void)ssi_dump_err_regs(SSI_ERR_PCIE_SR_WAKEUP_FAIL);

            /* 最多尝试的次数，超过则出错 */
            if (ul_retry_cnt++ < ul_max_retry_cnt) {
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
    } else {
        oal_msleep(100); /* 这里要用GPIO 做ACK 延迟不可靠, S/R 唤醒 时间较长 */
    }

    oal_atomic_set(&g_bus_powerup_dev_wait_ack, 0);

    return 0;
}

OAL_STATIC int32_t oal_pcie_edma_suspend_handler(void *data)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)data;

    /*
     * 下电在麒麟代码中处理
     * kirin_pcie_suspend_noirq,
     * 无法判断turnoff 是否成功发送
     */
    /* 此处不一定是真的下电了 */
    pst_pci_lres->power_status = PCIE_EP_IP_POWER_DOWN;

    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DEEPSLEEP);

    pci_print_log(PCI_LOG_INFO, "pcie_suspend_powerdown");
    declare_dft_trace_key_info("pcie_suspend_powerdown", OAL_DFT_TRACE_SUCC);
#ifdef CONFIG_ARCH_KIRIN_PCIE
    kirin_pcie_power_notifiy_register(g_kirin_rc_idx, oal_pcie_edma_wake_powerup_handler, NULL, NULL);
#endif

    return 0;
}
#endif

int32_t oal_pcie_edma_suspend(oal_pci_dev_stru *pst_pci_dev, oal_pm_message_t state)
{
    int32_t ret;
    uint32_t version = 0x0;
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

    pci_print_log(PCI_LOG_INFO, "oal_pcie_suspend");

    if (down_interruptible(&pst_bus->sr_wake_sema)) {
        oal_io_print(KERN_ERR "pcie_wake_sema down failed.");
        return -OAL_EFAIL;
    }

    if (oal_pcie_wakelock_active(pst_bus) == OAL_TRUE) {
        /* has wake lock so stop controller's suspend,
         * otherwise controller maybe error while sdio reinit */
        pci_print_log(PCI_LOG_INFO, "already wake up, break suspend");
        up(&pst_bus->sr_wake_sema);
        return -OAL_EFAIL;
    }

    ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev, 0x0, &version);
    if ((ret != 0) || (version != pst_pci_lres->version)) {
        pci_print_log(PCI_LOG_ERR, "suspend read pci version failed ret=%d, version[0x%x!=0x%x]",
                      ret, version, pst_pci_lres->version);
        up(&pst_bus->sr_wake_sema);
        declare_dft_trace_key_info("pci_version_suspend_check_failed", OAL_DFT_TRACE_FAIL);
        return -OAL_EFAIL;
    }

    pci_print_log(PCI_LOG_INFO, "read pcie version succ");

    wlan_pm_wkup_src_debug_set(OAL_TRUE);

    declare_dft_trace_key_info("pcie_os_suspend", OAL_DFT_TRACE_SUCC);

    oal_pcie_save_default_resource(pst_pci_lres);
#ifdef CONFIG_ARCH_KIRIN_PCIE
    kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, oal_pcie_edma_suspend_handler, (void *)pst_pci_lres);
#endif
    return 0;
}

int32_t oal_pcie_edma_resume(oal_pci_dev_stru *pst_pci_dev)
{
    int32_t ret;
    uint32_t version = 0x0;
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

    if (pst_pci_lres->power_status != PCIE_EP_IP_POWER_DOWN) {
        pci_print_log(PCI_LOG_INFO, "oal_pcie_resume");
        up(&pst_bus->sr_wake_sema);
        return OAL_SUCC;
    }

    ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev, 0x0, &version);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "read pci version failed ret=%d", ret);
        oal_msleep(1000);
        ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev, 0x0, &version);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "read pci version failed ret=%d", ret);
        } else {
            pci_print_log(PCI_LOG_WARN, "version:0x%x is not match with:0x%x", version, pst_pci_lres->version);
        }
        up(&pst_bus->sr_wake_sema);
        hcc_bus_exception_submit(pst_pci_lres->pst_bus, WIFI_TRANS_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_RESUME_FIRMWARE_DOWN);
        return OAL_SUCC;
    }

    if (version != pst_pci_lres->version) {
        pci_print_log(PCI_LOG_WARN, "version:0x%x is not match with:0x%x", version, pst_pci_lres->version);
    }

    board_host_wakeup_dev_set(0);
    oal_usleep(WLAN_WAKEUP_DEV_EVENT_DELAY_US);
    pci_print_log(PCI_LOG_INFO, "oal_pcie_edma_resume done");
    up(&pst_bus->sr_wake_sema);
    return 0;
}

OAL_STATIC hcc_bus_opt_ops g_pcie_edma_opt_ops = {
    .get_bus_state = oal_pcie_get_state,
    .disable_bus_state = oal_disable_pcie_state,
    .enable_bus_state = oal_enable_pcie_state,
    .rx_netbuf_list = oal_pcie_rx_netbuf,
    .tx_netbuf_list = oal_pcie_edma_tx_netbuf,
    .send_msg = oal_pcie_send_msg,
    .lock = oal_pcie_host_lock,
    .unlock = oal_pcie_host_unlock,
    .sleep_request = oal_pcie_sleep_request,
    .sleep_request_host = oal_pcie_edma_sleep_request_host,
    .wakeup_request = oal_pcie_wakeup_request,
    .get_sleep_state = oal_pcie_get_sleep_state,
    .wakeup_complete = oal_pcie_edma_wakeup_complete,
    .rx_int_mask = oal_pcie_rx_int_mask,
    .power_action = oal_pcie_edma_power_action,
    .power_ctrl = oal_pcie_power_ctrl,
    .wlan_gpio_handler = oal_pcie_gpio_irq,
    .wlan_gpio_rxdata_proc = oal_pcie_gpio_rx_data,
    .reinit = oal_pcie_reinit,
    .deinit = oal_pcie_deinit,
    .tx_condition = oal_pcie_edma_tx_condition,
    .patch_read = oal_pcie_patch_read,
    .patch_write = oal_pcie_patch_write,
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
    .bindcpu = NULL,
#else
    .bindcpu = oal_pcie_bindcpu,
#endif
    .get_trans_count = oal_pcie_get_trans_count,
    .voltage_bias_init = oal_pcie_ip_voltage_bias_init,
    .chip_info = oal_pcie_chip_info,

    .print_trans_info = oal_pcie_print_trans_info,
    .reset_trans_info = oal_pcie_reset_trans_info,
    .pending_signal_check = oal_pcie_edma_pending_signal_check,
    .pending_signal_process = oal_pcie_edma_pending_signal_process,
    .master_address_switch = NULL,
    .slave_address_switch = NULL,
#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
    .wlan_pm_vote = oal_pcie_wlan_pm_vote,
#endif
};

void oal_pcie_edma_bus_ops_init(hcc_bus *pst_bus)
{
    pst_bus->opt_ops = &g_pcie_edma_opt_ops;
}

#endif

