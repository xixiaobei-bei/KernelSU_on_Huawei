

#define HI11XX_LOG_MODULE_NAME     "[PCIEL]"
#define HI11XX_LOG_MODULE_NAME_VAR pciel_loglevel
#define HISI_LOG_TAG               "[PCIEL]"
#include "pcie_linux.h"

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
#include "pcie_firmware.h"
#include "oal_hcc_host_if.h"
#include "ete_host.h"

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
#include <linux/hisi/hisi_pcie_idle_sleep.h>
#endif

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_PCIE_LINUX_C

OAL_STATIC oal_pcie_linux_res *g_pcie_linux_res = NULL;

oal_completion g_probe_complete;    /* 初始化信号量 */
OAL_VOLATILE int32_t g_probe_ret; /* probe 返回值 */

int32_t g_pcie_enum_fail_reg_dump_flag = 0;

int32_t g_hipci_msi_enable = 0; /* 0 -intx 1-pci */
int32_t g_hipci_gen_select = PCIE_GEN2;
int32_t g_hipci_sync_flush_cache_enable = 0; // for device
int32_t g_hipci_sync_inv_cache_enable = 0;   // for cpu
int32_t g_ft_pcie_aspm_check_bypass = 0;
int32_t g_ft_pcie_gen_check_bypass = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_hipci_msi_enable, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hipci_gen_select, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hipci_sync_flush_cache_enable, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_hipci_sync_inv_cache_enable, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_ft_pcie_aspm_check_bypass, int, S_IRUGO | S_IWUSR);
oal_debug_module_param(g_ft_pcie_gen_check_bypass, int, S_IRUGO | S_IWUSR);
#endif

#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
int32_t g_kirin_rc_idx = 0;
oal_debug_module_param(g_kirin_rc_idx, int, S_IRUGO | S_IWUSR);
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
/* 双PCIE，EP PCIE1对应的KIRIN PCIE */
int32_t g_kirin_rc_idx_dual = 0;
oal_debug_module_param(g_kirin_rc_idx_dual, int, S_IRUGO | S_IWUSR);
#endif
#endif

int32_t g_pcie_print_once = 0;
oal_debug_module_param(g_pcie_print_once, int, S_IRUGO | S_IWUSR);

OAL_STATIC int32_t g_pcie_aspm_enable = 1;
oal_debug_module_param(g_pcie_aspm_enable, int, S_IRUGO | S_IWUSR);

OAL_STATIC int32_t g_pcie_performance_mode = 0;
oal_debug_module_param(g_pcie_performance_mode, int, S_IRUGO | S_IWUSR);

/* WCPU芯片唤醒阶段是否自动关闭L1降低单包延迟 */
int32_t g_pcie_auto_disable_aspm = 0;
oal_debug_module_param(g_pcie_auto_disable_aspm, int, S_IRUGO | S_IWUSR);

OAL_STATIC int32_t g_pcie_shutdown_panic = 0;
#ifdef PLATFORM_DEBUG_ENABLE
module_param(g_pcie_shutdown_panic, int, S_IRUGO | S_IWUSR);
#endif
/* Function Declare */
OAL_STATIC int32_t oal_pcie_probe(oal_pci_dev_stru *pst_pci_dev, const oal_pci_device_id_stru *pst_id);
OAL_STATIC hcc_bus *oal_pcie_bus_init(oal_pcie_linux_res *pst_pci_lres);
OAL_STATIC void oal_pcie_bus_exit(hcc_bus *pst_bus);

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
OAL_STATIC int32_t oal_pcie_dual_power_off_fail_callback(void *data);
OAL_STATIC int32_t wlan_dual_first_power_off_callback(void *data);
#endif

OAL_STATIC const oal_pci_device_id_stru g_pcie_id_tab[] = {
    { PCIE_HISI_VENDOR_ID, PCIE_HISI_DEVICE_ID_HI1103, PCI_ANY_ID, PCI_ANY_ID }, /* 1103 PCIE */
    { PCIE_HISI_VENDOR_ID, PCIE_HISI_DEVICE_ID_HI1105, PCI_ANY_ID, PCI_ANY_ID }, /* 1105 PCIE */
    { PCIE_HISI_VENDOR_ID, PCIE_HISI_DEVICE_ID_HI1106FPGA, PCI_ANY_ID, PCI_ANY_ID }, /* 1106 FPGA PCIE */
    { PCIE_HISI_VENDOR_ID_HI1106, PCIE_HISI_DEVICE_ID_HI1106, PCI_ANY_ID, PCI_ANY_ID }, /* 1106 PCIE */
    { PCIE_HISI_VENDOR_ID, PCIE_HISI_DEVICE_ID_HI1106, PCI_ANY_ID, PCI_ANY_ID }, /* 1106 tmp PCIE */
    { 0, }
};

int32_t oal_pcie_set_loglevel(int32_t loglevel)
{
    int32_t ret = g_hipcie_loglevel;
    g_hipcie_loglevel = loglevel;
    return ret;
}

int32_t oal_pcie_set_hi11xx_loglevel(int32_t loglevel)
{
    int32_t ret = HI11XX_LOG_MODULE_NAME_VAR;
    HI11XX_LOG_MODULE_NAME_VAR = loglevel;
    return ret;
}

int32_t oal_pcie_enumerate(int32_t rc_idx)
{
    int32_t ret = -OAL_ENODEV;
#if defined(CONFIG_ARCH_KIRIN_PCIE)
    oal_io_print("notify host to scan rc:%d\n", rc_idx);
    ret = kirin_pcie_enumerate(rc_idx);
#endif
    return ret;
}

void oal_pcie_rx_netbuf_hdr_init(oal_pci_dev_stru *hwdev, oal_netbuf_stru *pst_netbuf)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (g_pcie_dma_data_rx_hdr_init != 0) {
        memset_s((void *)oal_netbuf_data(pst_netbuf), oal_netbuf_len(pst_netbuf), 0xff, oal_netbuf_len(pst_netbuf));
    }
    /* 申请rx_buf时：利用cpu，赋初值 */
    *(volatile uint32_t*)oal_netbuf_data(pst_netbuf) = PCIE_RX_TRANS_FLAG;

#endif
}

OAL_STATIC int32_t oal_pcie_enable_single_device(oal_pci_dev_stru *pst_pcie_dev, struct pci_saved_state **default_state)
{
    u16 old_cmd = 0;
    int32_t ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))

    if (oal_warn_on(*default_state == NULL)) {
        oal_io_print("pcie had't any saved state!\n");
        return -OAL_ENODEV;
    }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
    /*
     * Updated with pci_load_and_free_saved_state to compatible
     * with kernel 3.14 or higher
     */
    pci_load_and_free_saved_state(pst_pcie_dev, default_state);

    /* Update default state */
    *default_state = pci_store_saved_state(pst_pcie_dev);
#else
    pci_load_saved_state(pst_pcie_dev, *default_state);
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) */
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)) */

    pci_restore_state(pst_pcie_dev);

    ret = oal_pci_enable_device(pst_pcie_dev);
    if (ret) {
        oal_io_print("enable device failed!ret=%d\n", ret);
        pci_disable_device(pst_pcie_dev);
    } else {
        pci_set_master(pst_pcie_dev);
    }

    pci_read_config_word(pst_pcie_dev, PCI_COMMAND, &old_cmd);
    oal_io_print("PCI_COMMAND:0x%x\n", old_cmd);

    return ret;
}

OAL_STATIC int32_t oal_pcie_save_resource(oal_pci_dev_stru *pst_pcie_dev, struct pci_saved_state **default_state)
{
    int32_t ret = OAL_SUCC;

    pci_save_state(pst_pcie_dev);
    oal_io_print("save resource\n");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
    if (*default_state != NULL) {
        /* There is already exist pcie state */
        struct pci_saved_state *pst_state = *default_state;
        *default_state = NULL;
        kfree(pst_state);
        oal_io_print("default state already exist, free first\n");
    }

    *default_state = pci_store_saved_state(pst_pcie_dev);
    if (oal_unlikely(*default_state == NULL)) {
        oal_pci_disable_device(pst_pcie_dev);
        return -OAL_ENOMEM;
    }
#endif
    return ret;
}

OAL_STATIC int32_t oal_pcie_enable_device(oal_pcie_linux_res *pst_pci_lres)
{
    oal_pci_dev_stru *pst_pcie_dev = pst_pci_lres->pst_pcie_dev;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENOMEM;
    }

    if (oal_warn_on(pst_pcie_dev == NULL)) {
        return -OAL_ENODEV;
    }
    return oal_pcie_enable_single_device(pst_pcie_dev, &pst_pci_lres->default_state);
}

OAL_STATIC void oal_pcie_disable_device(oal_pcie_linux_res *pst_pci_lres)
{
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    if (oal_warn_on(pst_pci_lres->pst_pcie_dev == NULL)) {
        return;
    }

    oal_pci_disable_device(pst_pci_lres->pst_pcie_dev);
}

int32_t oal_pcie_save_default_resource(oal_pcie_linux_res *pst_pci_lres)
{
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_warn_on(pst_pci_lres->pst_pcie_dev == NULL)) {
        return -OAL_ENODEV;
    }

    return oal_pcie_save_resource(pst_pci_lres->pst_pcie_dev, &pst_pci_lres->default_state);
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
int32_t oal_pcie_enable_dual_device(oal_pcie_linux_res *pst_pci_lres)
{
    oal_pci_dev_stru *pst_pcie_dev = pst_pci_lres->pst_pcie_dev_dual;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENOMEM;
    }

    if (oal_warn_on(pst_pcie_dev == NULL)) {
        return -OAL_ENODEV;
    }

    return oal_pcie_enable_single_device(pst_pcie_dev, &pst_pci_lres->default_state_dual);
}

void oal_pcie_disable_dual_device(oal_pcie_linux_res *pst_pci_lres)
{
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    if (oal_warn_on(pst_pci_lres->pst_pcie_dev_dual == NULL)) {
        return;
    }

    oal_pci_disable_device(pst_pci_lres->pst_pcie_dev_dual);
}

int32_t oal_pcie_save_default_dual_resource(oal_pcie_linux_res *pst_pci_lres)
{
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_EINVAL;
    }

    if (oal_warn_on(pst_pci_lres->pst_pcie_dev_dual == NULL)) {
        return -OAL_ENODEV;
    }

    return oal_pcie_save_resource(pst_pci_lres->pst_pcie_dev_dual, &pst_pci_lres->default_state_dual);
}
#endif

OAL_STATIC irqreturn_t oal_pcie_intx_edma_isr(int irq, void *dev_id)
{
    /*
     * 中断处理内容太多，目前无法下移，因为中断需要每次读空 ，而不在中断读的话，
     * 要先锁住中断，否则中断会狂报
     */
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)dev_id;
    if (oal_unlikely(oal_pcie_transfer_done(pst_pci_lres->pst_pci_res) < 0)) {
        pci_print_log(PCI_LOG_ERR, "froce to disable pcie intx");
        declare_dft_trace_key_info("transfer_done_fail", OAL_DFT_TRACE_EXCEP);
        oal_disable_pcie_irq(pst_pci_lres);

        /* DFR trigger */
        oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);
        hcc_bus_exception_submit(pst_pci_lres->pst_bus, WIFI_TRANS_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_INTX_ISR_PCIE_LINK_DOWN);
        /* 关闭低功耗 */
    }
    return IRQ_HANDLED;
}

irqreturn_t oal_pcie_intx_ete_isr(int irq, void *dev_id)
{
    /*
     * 中断处理内容太多，目前无法下移，因为中断需要每次读空 ，而不在中断读的话，
     * 要先锁住中断，否则中断会狂报
     */
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)dev_id;
    if (oal_unlikely(oal_pcie_transfer_ete_done(pst_pci_lres->pst_pci_res) < 0)) {
        pci_print_log(PCI_LOG_ERR, "froce to disable pcie intx");
        declare_dft_trace_key_info("transfer_done_fail", OAL_DFT_TRACE_EXCEP);
        oal_disable_pcie_irq(pst_pci_lres);

        /* DFR trigger */
        oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);
        hcc_bus_exception_submit(pst_pci_lres->pst_bus, WIFI_TRANS_FAIL);

        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_INTX_ISR_PCIE_LINK_DOWN);
        /* 关闭低功耗 */
    }
    return IRQ_HANDLED;
}

OAL_STATIC void oal_pcie_intx_task(uintptr_t data)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)data;
    oal_pcie_mips_start(PCIE_MIPS_HCC_RX_TOTAL);
    if (oal_pcie_transfer_done(pst_pci_lres->pst_pci_res)) {
        /* 非0说明还需要调度 */
        oal_pcie_mips_end(PCIE_MIPS_HCC_RX_TOTAL);
        oal_task_sched(&pst_pci_lres->st_rx_task);
    }
    oal_pcie_mips_end(PCIE_MIPS_HCC_RX_TOTAL);
}

#ifndef _PRE_HI375X_PCIE
#ifdef CONFIG_ARCH_KIRIN_PCIE
OAL_STATIC void oal_pcie_linkdown_callback(struct kirin_pcie_notify *noti)
{
    oal_pcie_linux_res *pst_pci_lres = NULL;
    oal_pci_dev_stru *pst_pci_dev = (oal_pci_dev_stru *)noti->user;
    if (pst_pci_dev == NULL) {
        oal_io_print(KERN_ERR "pcie linkdown, pci dev is null!!\n");
        return;
    }

    pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (pst_pci_lres == NULL) {
        oal_io_print(KERN_ERR "pcie linkdown, lres is null\n");
        return;
    }

    if (pst_pci_lres->pst_bus == NULL) {
        oal_io_print(KERN_ERR "pcie linkdown, pst_bus is null\n");
        return;
    }

    oal_io_print(KERN_ERR "pcie dev[%s] [%d:%d] linkdown\n",
                 dev_name(&pst_pci_dev->dev), pst_pci_dev->vendor, pst_pci_dev->device);
    declare_dft_trace_key_info("pcie_link_down", OAL_DFT_TRACE_EXCEP);

    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);

    if (board_get_wlan_wkup_gpio_val() == 0) {
        hcc_bus_exception_submit(pst_pci_lres->pst_bus, WIFI_TRANS_FAIL);
#ifdef CONFIG_HUAWEI_DSM
        hw_110x_dsm_client_notify(SYSTEM_TYPE_PLATFORM, DSM_WIFI_PCIE_LINKDOWN, "%s: pcie linkdown\n", __FUNCTION__);
#endif
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                             CHR_PLT_DRV_EVENT_PCIE, CHR_PLAT_DRV_ERROR_WKUP_GPIO_PCIE_LINK_DOWN);
    } else {
        pci_print_log(PCI_LOG_INFO, "dev wakeup gpio is high, dev maybe panic");
    }
}
#endif
#endif

OAL_STATIC void oal_pcie_unregister_msi_by_index(oal_pcie_linux_res *pst_pci_lres, int32_t range)
{
    int32_t i, j;
    oal_pci_dev_stru *pst_pci_dev;
    pst_pci_dev = pst_pci_lres->pst_pcie_dev;

    j = (range > pst_pci_lres->st_msi.msi_num) ? pst_pci_lres->st_msi.msi_num : range;

    if (pst_pci_lres->st_msi.func == NULL) {
        return;
    }

    for (i = 0; i < j; i++) {
        oal_irq_handler_t msi_handler = pst_pci_lres->st_msi.func[i];
        if (msi_handler != NULL) {
            free_irq(pst_pci_dev->irq + i, (void *)pst_pci_lres->pst_pci_res);
        }
    }
}

OAL_STATIC void oal_pcie_unregister_msi(oal_pcie_linux_res *pst_pci_lres)
{
    oal_pcie_unregister_msi_by_index(pst_pci_lres, pst_pci_lres->st_msi.msi_num);
}

OAL_STATIC int32_t oal_pcie_register_msi(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t i;
    int32_t ret;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    if (pst_pci_lres->st_msi.msi_num <= 0) {
        return -OAL_ENODEV;
    }

    if (pst_pci_lres->st_msi.func == NULL) {
        return -OAL_ENODEV;
    }

    pst_pci_dev = pst_pci_lres->pst_pcie_dev;

    for (i = 0; i < pst_pci_lres->st_msi.msi_num; i++) {
        oal_irq_handler_t msi_handler = pst_pci_lres->st_msi.func[i];
        if (msi_handler != NULL) {
            pci_print_log(PCI_LOG_DBG, "try to request msi irq:%d", pst_pci_dev->irq + i);
            ret = request_irq(pst_pci_dev->irq + i, msi_handler, IRQF_SHARED,
                              "hisi_pci_msi", (void *)pst_pci_lres);
            if (ret) {
                oal_io_print("msi request irq failed, base irq:%u, msi index:%d, ret=%d\n", pst_pci_dev->irq, i, ret);
                goto failed_request_msi;
            }
        }
    }

    return OAL_SUCC;
failed_request_msi:
    oal_pcie_unregister_msi_by_index(pst_pci_lres, i);
    return ret;
}

#ifdef CONFIG_ARCH_KIRIN_PCIE
OAL_STATIC void oal_kirin_pcie_deregister_event(oal_pcie_linux_res *pst_pci_lres)
{
#ifndef _PRE_HI375X_PCIE
    kirin_pcie_deregister_event(&pst_pci_lres->pcie_event);
#endif
    return;
}

OAL_STATIC void oal_kirin_pcie_register_event(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres)
{
#ifndef _PRE_HI375X_PCIE
    pst_pci_lres->pcie_event.events = KIRIN_PCIE_EVENT_LINKDOWN;
    pst_pci_lres->pcie_event.user = pst_pci_dev;
    pst_pci_lres->pcie_event.mode = KIRIN_PCIE_TRIGGER_CALLBACK;
    pst_pci_lres->pcie_event.callback = oal_pcie_linkdown_callback;
    kirin_pcie_register_event(&pst_pci_lres->pcie_event);
#endif
    return;
}
#endif

OAL_STATIC int32_t oal_pcie_register_int(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret = -OAL_EFAIL;

    /* interrupt register */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_io_print("raw irq: %d\n", pst_pci_dev->irq);

    /* Read from DTS later */
    pst_pci_lres->st_msi.is_msi_support = g_hipci_msi_enable;

    if (pst_pci_lres->st_msi.is_msi_support) {
        /* Try to enable msi */
        ret = pci_enable_msi(pst_pci_dev);
        if (ret > 0) {
            /* ep support msi */
            oal_io_print("msi irq:%d, msi int nums:%d\n", pst_pci_dev->irq, ret);
            ret = oal_pcie_register_msi(pst_pci_lres);
            if (ret != OAL_SUCC) {
                goto failed_register_msi;
            } else {
                pst_pci_lres->irq_stat = 0;
            }
        } else {
            /* non msi drivers */
            oal_io_print("msi request failed, disable msi... ret=%d\n", ret);
            pst_pci_lres->st_msi.is_msi_support = 0;
        }
    }

    /* Try to register intx */
    if (!pst_pci_lres->st_msi.is_msi_support) {
        if (pst_pci_lres->pst_pci_res->chip_info.edma_support == OAL_TRUE) {
            ret = request_irq(pst_pci_dev->irq, oal_pcie_intx_edma_isr, IRQF_SHARED,
                              "hisi_edma_intx", (void *)pst_pci_lres);
        }
        if (pst_pci_lres->pst_pci_res->chip_info.ete_support == OAL_TRUE) {
            ret = request_irq(pst_pci_dev->irq, oal_pcie_intx_ete_isr, IRQF_SHARED,
                              "hisi_ete_intx", (void *)pst_pci_lres);
        }

        if (ret < 0) {
            oal_io_print("request intx failed, ret=%d\n", ret);
            goto failed_pci_intx_request;
        }

        pst_pci_lres->irq_stat = 0;
    }

#endif

    oal_io_print("request pcie intx irq %d succ\n", pst_pci_dev->irq);
    return OAL_SUCC;

    /* fail process */
failed_pci_intx_request:
    if (pst_pci_lres->st_msi.is_msi_support) {
        oal_pcie_unregister_msi(pst_pci_lres);
    }
failed_register_msi:
    return ret;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
OAL_STATIC int32_t oal_pcie_dual_pci_init(oal_pci_dev_stru *pst_pci_dev, oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret;
    if (pst_pci_lres->pst_pci_res == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_pci_res is null");
        return -OAL_ENODEV;
    }

    ret = oal_pci_enable_device(pst_pci_dev);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "enable dual pcie failed, ret=%d", ret);
        return ret;
    }

    ret = oal_pcie_dual_bar_init(pst_pci_lres->pst_pci_res, pst_pci_dev);
    if (ret != OAL_SUCC) {
        return ret;
    }
    ret = oal_pcie_bar1_init(&pst_pci_lres->pst_pci_res->st_iatu_dual_bar);
    if (ret != OAL_SUCC) {
        return ret;
    }
    ret = oal_pcie_save_default_dual_resource(pst_pci_lres);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "save dual resource failed, ret=%d", ret);
        oal_pcie_bar1_exit(&pst_pci_lres->pst_pci_res->st_iatu_dual_bar);
        return ret;
    }
    return ret;
}

OAL_STATIC void oal_pcie_dual_pci_exit(oal_pcie_linux_res *pst_pci_lres)
{
    if (pst_pci_lres->pst_pci_res == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_pci_res is null");
        return;
    }
    oal_pcie_bar1_exit(&pst_pci_lres->pst_pci_res->st_iatu_dual_bar);
}

OAL_STATIC int32_t oal_pcie_dual_pci_probe(oal_pci_dev_stru *pst_pci_dev, const oal_pci_device_id_stru *pst_id)
{
    int32_t ret;
    uint32_t dword = 0;
    if (g_dual_pci_support == OAL_FALSE) {
        if (g_pcie_linux_res) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "pcie device init failed, already init!\n");
            return -OAL_EBUSY;
        }
    } else {
        /* dual pcie, pcie0&pcie1 init flow */
        ret = oal_pci_read_config_dword(pst_pci_dev, PCI_EXP_DEVICE2_DUAL_FLAG, &dword);
        if (ret) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "read dual flag failed, ret=%d", ret);
            return ret;
        }

        oal_print_hi11xx_log(HI11XX_LOG_INFO, "dual pcie flag: 0x%x", dword);

        if ((uint16_t)dword == (uint16_t)PCIE_DUAL_PCI_MASTER_FLAG) {
            /* pcie0 */
            if (g_pcie_linux_res != NULL) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "dual pcie must probe master ip first!");
                return -OAL_EBUSY;
            }
        }

        if ((uint16_t)dword == (uint16_t)PCIE_DUAL_PCI_SLAVE_FLAG) {
            /* pcie1 */
            if (g_pcie_linux_res == NULL) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "dual pcie must probe master ip first!");
                return -OAL_EBUSY;
            } else {
                g_pcie_linux_res->pst_pcie_dev_dual = pst_pci_dev;
                pci_set_drvdata(pst_pci_dev, g_pcie_linux_res);
                /* dual  pcie probe init */
                ret = oal_pcie_dual_pci_init(pst_pci_dev, g_pcie_linux_res);
                if (ret != OAL_SUCC) {
                    return ret;
                }
            }
        }
    }

    return OAL_SUCC;
}
#else
OAL_STATIC int32_t oal_pcie_dual_pci_probe(oal_pci_dev_stru *pst_pci_dev, const oal_pci_device_id_stru *pst_id)
{
    if (g_pcie_linux_res) {
        oal_io_print("pcie device init failed, already init!\n");
        return -OAL_EBUSY;
    }
    return OAL_SUCC;
}
#endif

OAL_STATIC void oal_pcie_init_dma_attr(oal_pci_dev_stru *pst_pci_dev)
{
    const struct dma_map_ops *ops = get_dma_ops(&pst_pci_dev->dev);
    if (ops == NULL) {
        g_hipci_sync_flush_cache_enable = 1;
        g_hipci_sync_inv_cache_enable   = 1;
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie dma is direct");
        return;
    }

    if (ops->sync_single_for_cpu == NULL) {
        // smmu old driver
        g_hipci_sync_inv_cache_enable = 0;
        oal_pcie_set_dma_data_rx_check_flag(0); // inv func null
    } else {
        g_hipci_sync_inv_cache_enable = 1;
    }

    if (ops->sync_single_for_device == NULL) {
        // smmu old driver
        g_hipci_sync_flush_cache_enable = 0;
    } else {
        g_hipci_sync_flush_cache_enable = 1;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "pci dma sync flush cache is %s, inv cache is %s",
                         g_hipci_sync_flush_cache_enable == 0 ? "disable" : "enable",
                         g_hipci_sync_inv_cache_enable == 0 ? "disable" : "enable");
}

/* 探测到一个PCIE设备, probe 函数可能会触发多次 */
OAL_STATIC int32_t oal_pcie_probe(oal_pci_dev_stru *pst_pci_dev, const oal_pci_device_id_stru *pst_id)
{
    uint8_t reg8 = 0;
    int32_t ret = -OAL_EFAIL;
    unsigned short device_id;
    hcc_bus *pst_bus = NULL;
    oal_pcie_linux_res *pst_pci_lres = NULL;

    if (oal_any_null_ptr2(pst_pci_dev, pst_id)) {
        /* never touch here */
        oal_io_print("oal_pcie_probe failed, pst_pci_dev:%p, pst_id:%p\n", pst_pci_dev, pst_id);
        g_probe_ret = -OAL_EIO;
        oal_complete(&g_probe_complete);
        return -OAL_EIO;
    }

    device_id = oal_pci_get_dev_id(pst_pci_dev);

    oal_pcie_init_dma_attr(pst_pci_dev);

    /* 设备ID 和 产品ID */
    oal_print_hi11xx_log(HI11XX_LOG_DBG,
        "[PCIe][%s]devfn:0x%x , vendor:0x%x , device:0x%x , subsystem_vendor:0x%x , subsystem_device:0x%x , class:0x%x "
        "\n",
        dev_name(&pst_pci_dev->dev), pst_pci_dev->devfn, pst_pci_dev->vendor, pst_pci_dev->device,
        pst_pci_dev->subsystem_vendor, pst_pci_dev->subsystem_device, pst_pci_dev->class);

    ret = oal_pcie_dual_pci_probe(pst_pci_dev, pst_id);
    if (ret != OAL_SUCC) {
        g_probe_ret = ret;
        oal_complete(&g_probe_complete);
        return ret;
    }

    ret = oal_pci_enable_device(pst_pci_dev);
    if (ret) {
        oal_io_print("pci: pci_enable_device error ret=%d\n", ret);
        g_probe_ret = ret;
        oal_complete(&g_probe_complete);
        return ret;
    }

    /* If slave pcie, end the probe */
    if (g_pcie_linux_res != NULL) {
        g_probe_ret = OAL_SUCC;
        oal_complete(&g_probe_complete);
        return OAL_SUCC;
    }

    /* alloc pcie resources */
    pst_pci_lres = (oal_pcie_linux_res *)oal_memalloc(sizeof(oal_pcie_linux_res));
    if (pst_pci_lres == NULL) {
        ret = -OAL_ENOMEM;
        oal_io_print("%s alloc res failed,size:%lu\n", __FUNCTION__, (unsigned long)sizeof(oal_pcie_linux_res));
        goto failed_res_alloc;
    }

    memset_s((void *)pst_pci_lres, sizeof(oal_pcie_linux_res), 0, sizeof(oal_pcie_linux_res));

    pst_pci_lres->pst_pcie_dev = pst_pci_dev;

    pci_set_drvdata(pst_pci_dev, pst_pci_lres);

    oal_init_completion(&pst_pci_lres->st_pcie_ready);
    oal_atomic_set(&pst_pci_lres->st_pcie_wakeup_flag, 0);

    oal_task_init(&pst_pci_lres->st_rx_task, oal_pcie_intx_task, (uintptr_t)pst_pci_lres);

    oal_spin_lock_init(&pst_pci_lres->st_lock);
    mutex_init(&pst_pci_lres->sr_lock);

    oal_wake_lock_init(&pst_pci_lres->st_sr_bugfix, "pcie_sr_bugfix");

    pst_pci_lres->version = (pst_pci_dev->vendor) | (pst_pci_dev->device << 16);

    ret = oal_pci_read_config_byte(pst_pci_dev, PCI_REVISION_ID, &reg8);
    if (ret) {
        oal_io_print("pci: read revision id  failed, ret=%d\n", ret);
        ret = -OAL_ENODEV;
        goto failed_pci_host_init;
    }

    pst_pci_lres->revision = (uint32_t)reg8;

    pci_print_log(PCI_LOG_INFO, "wifi pcie version:0x%8x, revision:%d", pst_pci_lres->version, pst_pci_lres->revision);

    /* soft resource init */
    pst_pci_lres->pst_pci_res = oal_pcie_host_init((void *)pst_pci_dev, &pst_pci_lres->st_msi, pst_pci_lres->revision);
    if (pst_pci_lres->pst_pci_res == NULL) {
        ret = -OAL_ENODEV;
        oal_io_print("pci: oal_pcie_host_init failed\n");
        goto failed_pci_host_init;
    }

    pst_bus = oal_pcie_bus_init(pst_pci_lres);
    if (pst_bus == NULL) {
        goto failed_pci_bus_init;
    }

    /* 硬件设备资源初始化, 5610+1103 FPGA 没有上下电接口 */
    ret = oal_pcie_dev_init(pst_pci_lres->pst_pci_res);
    if (ret != OAL_SUCC) {
        goto failed_pci_dev_init;
    }

    ret = oal_pcie_register_int(pst_pci_dev, pst_pci_lres);
    if (ret != OAL_SUCC) {
        goto failed_register_msi;
    }

#ifdef CONFIG_ARCH_KIRIN_PCIE
    oal_kirin_pcie_register_event(pst_pci_dev, pst_pci_lres);
#endif /* KIRIN_PCIE_LINKDOWN_RECOVERY */

    g_pcie_linux_res = pst_pci_lres;
    g_probe_ret = ret;
    oal_complete(&g_probe_complete);

    return ret;
failed_register_msi:
    oal_pcie_dev_deinit(pst_pci_lres->pst_pci_res);

failed_pci_dev_init:
    oal_pcie_bus_exit(pst_bus);
failed_pci_bus_init:
    oal_pcie_host_exit(pst_pci_lres->pst_pci_res);
failed_pci_host_init:
    pci_set_drvdata(pst_pci_dev, NULL);
    oal_wake_lock_exit(&pst_pci_lres->st_sr_bugfix);
    oal_free(pst_pci_lres);
failed_res_alloc:
    oal_pci_disable_device(pst_pci_dev);
    g_probe_ret = ret;
    oal_complete(&g_probe_complete);
    return ret;
}

OAL_STATIC void oal_pcie_remove(oal_pci_dev_stru *pst_pci_dev)
{
    unsigned short device_id;
    oal_pcie_linux_res *pst_pci_lres = NULL;

    device_id = oal_pci_get_dev_id(pst_pci_dev);

    oal_io_print("pcie driver remove 0x%x, name:%s\n", device_id, dev_name(&pst_pci_dev->dev));

    pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (pst_pci_lres == NULL) {
        oal_io_print("oal_pcie_remove lres is null\n");
        return;
    }

    g_pcie_linux_res = NULL;

#ifdef CONFIG_ARCH_KIRIN_PCIE
    oal_kirin_pcie_deregister_event(pst_pci_lres);
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (pst_pci_lres->st_msi.is_msi_support) {
        oal_pcie_unregister_msi(pst_pci_lres);
    } else {
        free_irq(pst_pci_dev->irq, pst_pci_lres);
    }

#endif

    mutex_destroy(&pst_pci_lres->sr_lock);

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    oal_pcie_dual_pci_exit(pst_pci_lres);
#endif
    oal_wake_lock_exit(&pst_pci_lres->st_sr_bugfix);

    oal_task_kill(&pst_pci_lres->st_rx_task);

    oal_pcie_dev_deinit(pst_pci_lres->pst_pci_res);

    oal_pcie_host_exit(pst_pci_lres->pst_pci_res);

    pst_pci_lres->pst_pcie_dev = NULL;
    oal_free(pst_pci_lres);

    oal_pci_disable_device(pst_pci_dev);
}

#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
int32_t oal_pcie_device_wakeup_handler(const void *data)
{
    oal_reference(data);
    /* 这里保证解复位EP控制器时efuse已经稳定 */
    board_host_wakeup_dev_set(1);
    pci_print_log(PCI_LOG_INFO, "pcie wakeup device control, pull up gpio");
    return 0;
}
#endif

#ifdef CONFIG_ARCH_KIRIN_PCIE
uint32_t oal_pcie_is_master_ip(oal_pci_dev_stru *pst_pci_dev)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    return ((pst_pci_dev == pst_pci_lres->pst_pcie_dev) ? OAL_TRUE : OAL_FALSE);
}

int32_t oal_pcie_get_pcie_rc_idx(oal_pci_dev_stru *pst_pci_dev)
{
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    return (oal_pcie_is_master_ip(pst_pci_dev) == OAL_TRUE ? g_kirin_rc_idx : g_kirin_rc_idx_dual);
#else
    return g_kirin_rc_idx;
#endif
}
#endif

int32_t oal_pcie_sr_para_check(oal_pcie_linux_res *pst_pci_lres,
                               hcc_bus **pst_bus, struct hcc_handler **pst_hcc)
{
    if (pst_pci_lres == NULL) {
        oal_io_print("lres is null\n");
        return -OAL_ENODEV;
    }

    *pst_bus = pst_pci_lres->pst_bus;
    if (*pst_bus == NULL) {
        oal_io_print(KERN_ERR "sr pst_bus is null\n");
        return -OAL_EFAIL;
    }

    if (hbus_to_dev(*pst_bus) == NULL) {
        oal_io_print("sr,pcie is not work...\n");
        return -OAL_ENODEV;
    }

    *pst_hcc = hbus_to_hcc(*pst_bus);
    if (*pst_hcc == NULL) {
        pci_print_log(PCI_LOG_WARN, "pcie dev's hcc handler is null!");
        return -OAL_ENODEV;
    }

    if (*pst_bus != hdev_to_hbus(hbus_to_dev(*pst_bus))) {
        /* pcie非当前接口 */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie is not current bus, return");
        return -OAL_ENODEV;
    }

    if (oal_atomic_read(&(*pst_hcc)->state) != HCC_ON) {
        pci_print_log(PCI_LOG_INFO, "wifi is closed");
        return -OAL_ENODEV;
    }

    return OAL_SUCC;
}

// 判断pcie suspend之前是否持锁
int32_t oal_pcie_wakelock_active(hcc_bus *pst_bus)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    if (pst_wlan_pm != NULL) {
        if (oal_wakelock_active(&pst_wlan_pm->st_pm_wakelock)) {
            pci_print_log(PCI_LOG_INFO, "st_pm_wakelock active");
            return OAL_TRUE;
        }
    }

    if (hcc_bus_wakelock_active(pst_bus)) {
        pci_print_log(PCI_LOG_INFO, "st_bus_wakelock active");
        return OAL_TRUE;
    }

    return OAL_FALSE;
}

OAL_STATIC int32_t oal_pcie_suspend(oal_pci_dev_stru *pst_pci_dev, oal_pm_message_t state)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (pst_pci_lres == NULL) {
        oal_io_print("pcie_suspend lres is null,pci_dev_name=%s\n", dev_name(&pst_pci_dev->dev));
        return 0;
    }

    if (pst_pci_lres->pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        ret = oal_pcie_edma_suspend(pst_pci_dev, state);
    } else if (pst_pci_lres->pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        ret = oal_pcie_ete_suspend(pst_pci_dev, state);
    } else {
        ret = -OAL_ENODEV;
    }

    return ret;
}

OAL_STATIC int32_t oal_pcie_resume(oal_pci_dev_stru *pst_pci_dev)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pci_dev);
    if (pst_pci_lres == NULL) {
        oal_io_print("pcie_suspend lres is null,pci_dev_name=%s\n", dev_name(&pst_pci_dev->dev));
        return 0;
    }

    if (pst_pci_lres->pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        ret = oal_pcie_edma_resume(pst_pci_dev);
    } else if (pst_pci_lres->pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        ret = oal_pcie_ete_resume(pst_pci_dev);
    } else {
        ret = -OAL_ENODEV;
    }

    return ret;
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC void oal_pcie_shutdown(oal_pci_dev_stru *pst_pcie_dev)
{
    declare_time_cost_stru(cost);

    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)oal_pci_get_drvdata(pst_pcie_dev);
    g_chip_err_block = 0;
    if (pst_pci_lres == NULL) {
        return;
    }

    ssi_excetpion_dump_disable();
    /* system shutdown, should't write sdt file */
    oam_set_output_type(OAM_OUTPUT_TYPE_CONSOLE);

    if (pst_pci_lres->pst_bus == NULL) {
        pci_print_log(PCI_LOG_INFO, "pcie bus is null");
        return;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
    time_cost_var_start(cost) = 0;
#else
    time_cost_var_start(cost) = (ktime_t) { .tv64 = 0 };
#endif

    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_pci_lres->pst_bus), OAL_FALSE);
    oal_disable_pcie_irq(pst_pci_lres);

    /* power off wifi */
    if (g_pcie_shutdown_panic) {
        oal_get_time_cost_start(cost);
    }

    if (in_interrupt() || irqs_disabled() || in_atomic()) {
        pci_print_log(PCI_LOG_INFO, "no call wlan_pm_close_by_shutdown interrupt");
    } else {
        wlan_pm_close_by_shutdown();
    }

    if (g_pcie_shutdown_panic) {
        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie shutdown cost %llu us", time_cost_var_sub(cost));
#ifdef PLATFORM_DEBUG_ENABLE
        /* debug shutdown process when after wifi shutdown */
        if (time_cost_var_sub(cost) > (uint64_t)(uint32_t)g_pcie_shutdown_panic) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "pcie shutdown panic");
            BUG_ON(1);
        }
#endif
    }
}

OAL_STATIC oal_pci_driver_stru g_pcie_drv = {
    .name = "hi110x_pci",
    .id_table = g_pcie_id_tab,
    .probe = oal_pcie_probe,
    .remove = oal_pcie_remove,
    .suspend = oal_pcie_suspend,
    .resume = oal_pcie_resume,
    .shutdown = oal_pcie_shutdown,
};
#endif

int32_t oal_pcie_get_state(hcc_bus *pst_bus, uint32_t mask)
{
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    oal_reference(pst_pci_lres);

    return OAL_TRUE;
}

void oal_enable_pcie_state(hcc_bus *pst_bus, uint32_t mask)
{
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    oal_reference(pst_pci_lres);
}

void oal_disable_pcie_state(hcc_bus *pst_bus, uint32_t mask)
{
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    oal_reference(pst_pci_lres);
}

int32_t oal_pcie_rx_netbuf(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head)
{
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    oal_reference(pst_pci_lres);
    return OAL_SUCC;
}

int32_t oal_pcie_check_tx_param(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head,
                                hcc_netbuf_queue_type qtype)
{
    oal_pcie_linux_res *pst_pci_lres = NULL;
    struct hcc_handler *pst_hcc = NULL;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("pcie tx netbuf failed, lres is null\n");
        hcc_tx_netbuf_list_free(pst_head);
        return -OAL_ENODEV;
    }
    pst_hcc = hbus_to_hcc(pst_bus);
    if (pst_hcc == NULL) {
        pci_print_log(PCI_LOG_WARN, "pcie dev's hcc handler is null!");
        hcc_tx_netbuf_list_free(pst_head);
        return -OAL_ENODEV;
    }

    if (oal_unlikely(oal_atomic_read(&pst_hcc->state) != HCC_ON)) {
        /* drop netbuf list, wlan close or exception */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "drop pcie netbuflist %u", oal_netbuf_list_len(pst_head));
        hcc_tx_netbuf_list_free(pst_head);
        return -OAL_EFAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_send_message2dev(oal_pcie_linux_res *pst_pci_lres, uint32_t message)
{
    hcc_bus *pst_bus;
    int32_t ret;
    pst_bus = pst_pci_lres->pst_bus;

    if (oal_warn_on(pst_bus == NULL)) {
        return -OAL_ENODEV;
    }

    pci_print_log(PCI_LOG_DBG, "send msg to device [0x%8x]\n", (uint32_t)message);

    hcc_bus_wake_lock(pst_bus);
    ret = oal_pcie_send_message_to_dev(pst_pci_lres->pst_pci_res, message);
    hcc_bus_wake_unlock(pst_bus);

    return ret;
}

int32_t oal_pcie_send_msg(hcc_bus *pst_bus, uint32_t val)
{
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    if (val >= H2D_MSG_COUNT) {
        oal_io_print("[Error]invaild param[%u]!\n", val);
        return -OAL_EINVAL;
    }

    return oal_pcie_send_message2dev(pst_pci_lres, (uint32_t)val);
}
#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#else


OAL_STATIC void oal_pcie_bindcpu_autoctl(oal_pcie_linux_res *pci_lres,
                                         oal_pci_dev_stru *pci_dev,
                                         int32_t is_bind)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_HISI_BINDCPU
    struct cpumask slow_cpus, fast_cpus;

#ifdef CONFIG_ARCH_PLATFORM
    get_slow_cpus(&slow_cpus);
    get_fast_cpus(&fast_cpus);
#else
    hisi_get_slow_cpus(&slow_cpus);
    hisi_get_fast_cpus(&fast_cpus);
#endif
#endif

    if (is_bind) {
#ifdef CONFIG_NR_CPUS
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "pcie bindcpu irq =%u, bind to cpu %d", pci_dev->irq, OAL_BUS_HPCPU_NUM);
        irq_set_affinity_hint(pci_dev->irq, cpumask_of(OAL_BUS_HPCPU_NUM));
#else
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "pcie bindcpu fail, cpu:%d irq:%u", CONFIG_NR_CPUS, pci_dev->irq);
#endif
#endif

#ifdef _PRE_HISI_BINDCPU
        if (pci_lres->pst_pci_res->pst_rx_hi_task) {
            /* 中断所在核会影响线程迁移 */
            cpumask_clear_cpu(OAL_BUS_HPCPU_NUM, &fast_cpus);
            set_cpus_allowed_ptr(pci_lres->pst_pci_res->pst_rx_hi_task, &fast_cpus);
        }
#endif
    } else {
#ifdef CONFIG_NR_CPUS
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "pcie unbind cpu irq =%u", pci_dev->irq);
#if (KERNEL_VERSION(2, 6, 35) <= LINUX_VERSION_CODE)
        /* 2.6.35-rc1 */
        irq_set_affinity_hint(pci_dev->irq, cpumask_of(0));
#endif
#ifdef _PRE_HISI_BINDCPU
        if (pci_lres->pst_pci_res->pst_rx_hi_task) {
            set_cpus_allowed_ptr(pci_lres->pst_pci_res->pst_rx_hi_task, &slow_cpus);
        }
#endif
#endif
    }
#endif
}


OAL_STATIC void oal_pcie_bindcpu_userctl(oal_pcie_linux_res *pci_lres,
                                         oal_pci_dev_stru *pci_dev,
                                         uint8_t irq_cpu,
                                         uint8_t thread_cmd)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef _PRE_HISI_BINDCPU
    struct cpumask thread_cpu_mask;
#endif

#if defined(CONFIG_NR_CPUS) &&  defined(_PRE_HISI_BINDCPU)
    oal_print_hi11xx_log(HI11XX_LOG_DBG, "pcie bindcpu irq =%u, bind to cpu %d", pci_dev->irq, irq_cpu);
    irq_set_affinity_hint(pci_dev->irq, cpumask_of(irq_cpu));
#endif

#ifdef _PRE_HISI_BINDCPU
    if (pci_lres->pst_pci_res->pst_rx_hi_task) {
        thread_cpu_mask = *((struct cpumask *)&thread_cmd);
        if (irq_cpu >= OAL_BUS_HPCPU_NUM) {
            /* 中断所在核会影响线程迁移 */
            cpumask_clear_cpu(irq_cpu, &thread_cpu_mask);
        }
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "pst_rx_hi_task bind to cpu[0x%x]", thread_cmd);
        set_cpus_allowed_ptr(pci_lres->pst_pci_res->pst_rx_hi_task, &thread_cpu_mask);
    }
#endif
#endif
}

int32_t oal_pcie_bindcpu(hcc_bus *pst_bus, uint32_t chan, int32_t is_bind)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_pcie_bindcpu_cfg bindcpu_cfd = *(oal_pcie_bindcpu_cfg *)&is_bind;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    oal_pci_dev_stru *pst_pci_dev = NULL;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (pst_pci_lres == NULL) {
        return OAL_SUCC;
    }

    pst_pci_dev = pst_pci_lres->pst_pcie_dev;
    if (pst_pci_dev == NULL) {
        return OAL_SUCC;
    }

    if (bindcpu_cfd.is_userctl == OAL_TRUE) {
        /* 根据用户命令绑定CPU */
        oal_pcie_bindcpu_userctl(pst_pci_lres, pst_pci_dev, bindcpu_cfd.irq_cpu, bindcpu_cfd.thread_cmd);
    } else {
        /* 根据吞吐量与PPS门限自动绑定CPU */
        oal_pcie_bindcpu_autoctl(pst_pci_lres, pst_pci_dev, bindcpu_cfd.is_bind);
    }
#endif
    return OAL_SUCC;
}
#endif

int32_t oal_pcie_get_trans_count(hcc_bus *hi_bus, uint64_t *tx, uint64_t *rx)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    return oal_pcie_get_host_trans_count(pst_pci_lres->pst_pci_res, tx, rx);
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
OAL_STATIC int32_t oal_pcie_dual_reinit(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret;
    uint32_t version = 0x0;
    declare_time_cost_stru(reinit);

    if (pst_pci_lres->pst_pcie_dev_dual == NULL) {
        return OAL_SUCC; /* dual device not exist */
    }

    oal_get_time_cost_start(reinit);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_dual_reinit");

    ret = oal_pcie_enable_dual_device(pst_pci_lres);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* check link status */
    ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev_dual, 0x0, &version);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "read pci version failed, enable device failed, ret=%d, version=0x%x",
                      ret, version);
        declare_dft_trace_key_info("pcie_enable_dual_device_reinit_fail", OAL_DFT_TRACE_FAIL);
        return -OAL_ENODEV;
    }

    if (version != pst_pci_lres->version) {
        pci_print_log(PCI_LOG_WARN, "version:0x%x is not match with:0x%x", version, pst_pci_lres->version);
        return -OAL_ENODEV;
    } else {
        pci_print_log(PCI_LOG_INFO, "pcie enable device check succ");
    }

    /* 初始化PCIE资源 */
    ret = oal_pcie_set_outbound_membar(pst_pci_lres->pst_pci_res,
                                       &pst_pci_lres->pst_pci_res->st_iatu_dual_bar);
    if (ret) {
        oal_io_print(KERN_ERR "enable regions failed, ret=%d\n", ret);
        return ret;
    }

    oal_get_time_cost_end(reinit);
    oal_calc_time_cost_sub(reinit);
    pci_print_log(PCI_LOG_INFO, "pcie dual reinit cost %llu us", time_cost_var_sub(reinit));
    return ret;
}
#endif

int32_t oal_pcie_deinit(hcc_bus *pst_bus)
{
    return OAL_SUCC;
}

/* 1106 cs2 to be deleted: disable group2 cputrace */
#define DIAG_CTL_GROUP2_SAMPLE_ENABLE_REG 0x40124C80
OAL_STATIC int32_t oal_pcie_disable_cputrace(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret;
    pci_addr_map addr_map;
    uint32_t cpu_address = DIAG_CTL_GROUP2_SAMPLE_ENABLE_REG;
    uint32_t value = 0x0;
    oal_pci_dev_stru *pst_pci_dev = pcie_res_to_dev(pst_pci_lres->pst_pci_res);
    if ((pst_pci_dev->device != PCIE_HISI_DEVICE_ID_HI1106)
        && (pst_pci_dev->device != PCIE_HISI_DEVICE_ID_HI1106FPGA)) {
        return OAL_SUCC;
    }

    ret = oal_pcie_inbound_ca_to_va(pst_pci_lres->pst_pci_res, cpu_address, &addr_map);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oal_io_print("oal_pcie_disable_cputrace 0x%8x unmap, write failed!\n", cpu_address);
        return -OAL_EFAIL;
    }

    oal_writel(value, (void *)addr_map.va);
    oal_io_print("oal_pcie_disable_cputrace 0x%x = %x, write success!\n", cpu_address, value);
    return ret;
}

/* reinit pcie ep control */
int32_t oal_pcie_reinit(hcc_bus *pst_bus)
{
    int32_t ret;
    uint32_t version = 0x0;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    declare_time_cost_stru(reinit);

    oal_get_time_cost_start(reinit);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wake_sema_count=%d", pst_bus->sr_wake_sema.count);
    sema_init(&pst_bus->sr_wake_sema, 1); /* S/R信号量 */

    hcc_bus_disable_state(pst_bus, OAL_BUS_STATE_ALL);
    ret = oal_pcie_enable_device(pst_pci_lres);
    if (ret == OAL_SUCC) {
#ifdef _PRE_COMMENT_CODE_
        /* 需要在初始化完成后打开 */
        hcc_bus_enable_state(pst_bus, OAL_BUS_STATE_ALL);
#endif
    }

    /* check link status */
    if (pst_pci_lres->pst_pcie_dev != NULL) {
        ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev, 0x0, &version);
        if (ret) {
            pci_print_log(PCI_LOG_ERR, "read pci version failed, enable device failed, ret=%d, version=0x%x",
                          ret, version);
            declare_dft_trace_key_info("pcie_enable_device_reinit_fail", OAL_DFT_TRACE_FAIL);
            return -OAL_ENODEV;
        }

        if (version != pst_pci_lres->version) {
            pci_print_log(PCI_LOG_WARN, "version:0x%x is not match with:0x%x", version, pst_pci_lres->version);
        } else {
            pci_print_log(PCI_LOG_INFO, "pcie enable device check succ");
        }
    }

    /* 初始化PCIE资源 */
    ret = oal_pcie_enable_regions(pst_pci_lres->pst_pci_res);
    if (ret) {
        oal_io_print(KERN_ERR "enable regions failed, ret=%d\n", ret);
        return ret;
    }

    ret = oal_pcie_iatu_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        oal_io_print(KERN_ERR "iatu init failed, ret=%d\n", ret);
        return ret;
    }

    oal_get_time_cost_end(reinit);
    oal_calc_time_cost_sub(reinit);
    pci_print_log(PCI_LOG_INFO, "pcie reinit cost %llu us", time_cost_var_sub(reinit));
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    ret = oal_pcie_dual_reinit(pst_pci_lres);
#endif

    /* 1106 cs2 to be deleted: disable group2 cputrace */
    ret = oal_pcie_disable_cputrace(pst_pci_lres);

    return ret;
}

int32_t oal_pcie_host_lock(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    return OAL_SUCC;
}

int32_t oal_pcie_host_unlock(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    return OAL_SUCC;
}

/* 1103 PCIE 通过 host_wakeup_dev gpio 来唤醒和通知WCPU睡眠 */
int32_t oal_pcie_sleep_request(hcc_bus *pst_bus)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    oal_disable_pcie_irq(pst_pci_lres);

    if (g_pcie_auto_disable_aspm != 0) {
        /* enable L1 after wakeup */
        oal_pcie_set_device_aspm_dync_disable(pst_pci_lres->pst_pci_res, OAL_FALSE);
        oal_pcie_device_xfer_pending_sig_clr(pst_pci_lres->pst_pci_res, OAL_TRUE);
    }

    /* 拉低GPIO，PCIE只有在system suspend的时候才会下电 GPIO 拉低之后 DEV 随时可能进深睡，不允许再通过PCIE 访问 */
    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_UP);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

    pci_clear_master(pst_pci_lres->pst_pcie_dev);

    ret = board_host_wakeup_dev_set(0);
    oal_usleep(WLAN_WAKEUP_DEV_EVENT_DELAY_US);
    return ret;
}

#ifdef _PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE
int32_t oal_pcie_wakeup_request(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres = NULL;

    if (pst_bus == NULL) {
        pci_print_log(PCI_LOG_WARN, "oal_kupeng_pcie_wakeup_request pst_bus is null.\n");
        return -OAL_ENODEV;
    }

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    oal_pcie_device_wakeup_handler(NULL);
    pci_set_master(pst_pci_lres->pst_pcie_dev);

    return OAL_SUCC;
}
#else
int32_t oal_pcie_wakeup_request(hcc_bus *pst_bus)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
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
        uint32_t ul_ret;
        declare_time_cost_stru(cost);
        struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
        oal_get_time_cost_start(cost);

        if (pst_wlan_pm != NULL) {
            oal_init_completion(&pst_wlan_pm->st_wifi_powerup_done);
        }

        oal_atomic_set(&g_bus_powerup_dev_wait_ack, 1);
        oal_pcie_device_wakeup_handler(NULL);
        if (pst_wlan_pm != NULL) {
            ul_ret = oal_wait_for_completion_timeout(&pst_wlan_pm->st_wifi_powerup_done,
                                                     (uint32_t)oal_msecs_to_jiffies(2000));
            if (oal_unlikely(ul_ret == 0)) {
                /* 超时不做处理继续尝试建链 */
                declare_dft_trace_key_info("pcie_powerup_wakeup ack timeout", OAL_DFT_TRACE_FAIL);
            }
        } else {
            oal_msleep(100);
        }

        oal_atomic_set(&g_bus_powerup_dev_wait_ack, 0);

        /* iatu */
        ret = oal_pcie_reinit(pst_bus);
        if (ret != OAL_SUCC) {
            pci_print_log(PCI_LOG_ERR, "oal_pcie_reinit failed!ret=%d", ret);
            return ret;
        }

        oal_pcie_host_ip_init(pst_pci_lres);

        if (oal_pcie_check_link_state(pst_pci_lres->pst_pci_res) == OAL_FALSE) {
            uint32_t version = 0x0;
            ret = oal_pci_read_config_dword(pst_pci_lres->pst_pcie_dev, 0x0, &version);
            pci_print_log(PCI_LOG_ERR, "mem access error, maybe linkdown! config read version :0x%x, ret=%d",
                          version, ret);
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

        oal_get_time_cost_end(cost);
        oal_calc_time_cost_sub(cost);
        pci_print_log(PCI_LOG_INFO, "pcie power up init succ, cost %llu us\n", time_cost_var_sub(cost));
    } else {
        /* 正常单芯片唤醒拉高GPIO即可 */
        oal_pcie_device_wakeup_handler(NULL);
        pci_set_master(pst_pci_lres->pst_pcie_dev);
    }

#endif
    return OAL_SUCC;
}
#endif

int32_t oal_pcie_get_sleep_state(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("pst_pci_lres is null\n");
        return -OAL_EINVAL;
    }

    return (board_get_host_wakeup_dev_stat() == 1) ? DISALLOW_TO_SLEEP_VALUE : ALLOW_TO_SLEEP_VALUE;
}

int32_t oal_pcie_rx_int_mask(hcc_bus *pst_bus, int32_t is_mask)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;

    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("pst_pci_lres is null\n");
        return -OAL_EINVAL;
    }
    return OAL_SUCC;
}

int32_t oal_pcie_pm_control(struct pci_dev *pdev, u32 rc_idx, int power_option)
{
    int32_t ret;

#ifdef _PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE
    if (pdev == NULL) {
        return -OAL_EFAIL;
    }

    ret = pci_rp_power_ctrl(pdev, power_option);
    pci_print_log(PCI_LOG_INFO, "pci_rp_power_ctrl ret=%d\n", ret);
    return ret;
#endif

    if (power_option == PCIE_POWER_ON) {
        wlan_pm_idle_sleep_vote(DISALLOW_IDLESLEEP);
    }

#ifdef CONFIG_ARCH_KIRIN_PCIE
    ret = kirin_pcie_pm_control(power_option, rc_idx);
    pci_print_log(PCI_LOG_INFO, "host_pcie_pm_control ret=%d\n", ret);
    if ((power_option == PCIE_POWER_OFF_WITH_L3MSG) || (power_option == PCIE_POWER_OFF_WITHOUT_L3MSG)) {
        wlan_pm_idle_sleep_vote(ALLOW_IDLESLEEP);
    }
    return ret;
#endif
}

int32_t oal_pcie_power_notifiy_register(struct pci_dev *pdev, u32 rc_idx, int (*poweron)(void *data),
                                        int (*poweroff)(void *data), void *data)
{
    int32_t ret;

#ifdef _PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE
    if (pdev == NULL) {
        return -OAL_EFAIL;
    }

    ret = pci_power_notify_register(pdev, poweron, poweroff, data);
    pci_print_log(PCI_LOG_INFO, "pci_power_notify_register ret=%d\n", ret);
    return ret;
#endif

#ifdef CONFIG_ARCH_KIRIN_PCIE
    ret = kirin_pcie_power_notifiy_register(rc_idx, poweron, poweroff, data);
    pci_print_log(PCI_LOG_INFO, "host_pcie_power_notifiy_register ret=%d\n", ret);
    return ret;
#endif
}

int32_t oal_pcie_power_ctrl(hcc_bus *hi_bus, hcc_bus_power_ctrl_type ctrl,
                            int (*func)(void *data), void *data)
{
    int32_t ret = -OAL_EFAIL;

    oal_pcie_linux_res *pst_pci_lres = NULL;
    if (hi_bus == NULL) {
        return ret;
    }
    pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;

    if (ctrl == HCC_BUS_CTRL_POWER_UP) {
        ret = oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, func, NULL, data);
    }

    if (ctrl == HCC_BUS_CTRL_POWER_DOWN) {
        ret = oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, NULL, func, data);
    }

    return ret;
}

int32_t oal_pcie_enable_device_func(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t i;
    int32_t ret;
    unsigned int timeout = hi110x_get_emu_timeout(HOST_WAIT_BOTTOM_INIT_TIMEOUT);
    const int32_t retry = 2;
    hcc_bus *pst_bus = pst_pci_lres->pst_bus;

    if (pst_bus == NULL) {
        return -OAL_ENODEV;
    }

    for (i = 0; i < retry; i++) {
        oal_init_completion(&pst_pci_lres->st_pcie_ready);

        ret = oal_pcie_send_message2dev(pst_pci_lres, PCIE_H2D_MESSAGE_HOST_READY);
        if (ret != OAL_SUCC) {
            oal_io_print(KERN_ERR "oal_pcie_send_message2dev failed, ret=%d\n", ret);
            continue;
        }

        /* 第一个中断有可能在中断使能之前上报，强制调度一次RX Thread */
        up(&pst_bus->rx_sema);

        if (oal_wait_for_completion_timeout(&pst_pci_lres->st_pcie_ready,
                                            (uint32_t)oal_msecs_to_jiffies(timeout)) == 0) {
            oal_io_print(KERN_ERR "wait pcie ready timeout... %u ms \n", timeout);
            up(&pst_bus->rx_sema);
            if (oal_wait_for_completion_timeout(&pst_pci_lres->st_pcie_ready,
                                                (uint32_t)oal_msecs_to_jiffies(5000)) == 0) {
                oal_io_print(KERN_ERR "pcie retry 5 second hold, still timeout");
                if (i == 0) {
                    declare_dft_trace_key_info("wait pcie ready when power on, retry", OAL_DFT_TRACE_FAIL);
                } else {
                    declare_dft_trace_key_info("wait pcie ready when power on, retry still failed", OAL_DFT_TRACE_FAIL);
                }
                continue;
            } else {
                /* 强制调度成功，说明有可能是GPIO中断未响应 */
                oal_io_print(KERN_WARNING "[E]retry succ, maybe gpio interrupt issue");
                declare_dft_trace_key_info("pcie gpio int issue", OAL_DFT_TRACE_FAIL);
                break;
            }
        } else {
            break;
        }
    }

    if (i >= retry) {
        return ret;
    }

    return OAL_SUCC;
}

int32_t oal_enable_pcie_irq(oal_pcie_linux_res *pst_pci_lres)
{
    oal_uint flag;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    oal_spin_lock_irq_save(&pst_pci_lres->st_lock, &flag);
    if (pst_pci_lres->irq_stat == 1) {
        if (!pst_pci_lres->st_msi.is_msi_support) {
            /* intx */
            pci_print_log(PCI_LOG_DBG, "enable_pcie_irq");
            enable_irq(pst_pci_lres->pst_pcie_dev->irq);
        } else {
            /* msi */
        }
        pst_pci_lres->irq_stat = 0;
    }
    oal_spin_unlock_irq_restore(&pst_pci_lres->st_lock, &flag);

    return OAL_SUCC;
}

int32_t oal_disable_pcie_irq(oal_pcie_linux_res *pst_pci_lres)
{
    oal_uint flag;

    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    oal_spin_lock_irq_save(&pst_pci_lres->st_lock, &flag);
    if (pst_pci_lres->irq_stat == 0) {
        pst_pci_lres->irq_stat = 1;
        if (!pst_pci_lres->st_msi.is_msi_support) {
            /* intx */
            pci_print_log(PCI_LOG_DBG, "disable_pcie_irq");
            if (in_irq()) {
                disable_irq_nosync(pst_pci_lres->pst_pcie_dev->irq);
            } else {
                /* process context */
                disable_irq_nosync(pst_pci_lres->pst_pcie_dev->irq);
                oal_spin_unlock_irq_restore(&pst_pci_lres->st_lock, &flag);
                synchronize_irq(pst_pci_lres->pst_pcie_dev->irq);
                oal_spin_lock_irq_save(&pst_pci_lres->st_lock, &flag);
            }
        } else {
            /* msi */
        }
    }
    oal_spin_unlock_irq_restore(&pst_pci_lres->st_lock, &flag);

    return OAL_SUCC;
}

OAL_STATIC int32_t oal_pcie_host_aspm_init(oal_pcie_linux_res *pst_pci_lres)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    uint16_t val = 0;
    uint32_t reg;
    oal_pci_dev_stru *pst_rc_dev;
    pst_rc_dev = pci_upstream_bridge(pst_pci_lres->pst_pcie_dev);

    /* 使能/去使能ASPM，RC & EP */
    kirin_pcie_lp_ctrl(g_kirin_rc_idx, 0);
#endif

    if (pst_pci_lres->pst_pci_res != NULL) {
        oal_pcie_device_phy_config(pst_pci_lres->pst_pci_res);
    }

#ifdef  CONFIG_ARCH_KIRIN_PCIE
    if (g_pcie_aspm_enable) {
        /* L1SS config */
        if ((pst_pci_lres->pst_pci_res != NULL) && (pst_pci_lres->pst_pci_res->pst_pci_dbi_base != NULL)) {
            reg = oal_readl(pst_pci_lres->pst_pci_res->pst_pci_dbi_base + PCIE_ACK_F_ASPM_CTRL_OFF);
#ifdef _PRE_COMMENT_CODE_
            reg &= ~((0x7 << 24) | (0x7 << 27));
#endif
            reg &= ~((0x7 << 27));
#ifdef _PRE_COMMENT_CODE_
            /* L0s 7us entry, L1 64us entery, tx & rx */
            reg |= (0x7 << 24) | (0x7 << 27);
#endif
            reg |= (0x3 << 27);
            oal_writel(reg, pst_pci_lres->pst_pci_res->pst_pci_dbi_base + PCIE_ACK_F_ASPM_CTRL_OFF);
            pci_print_log(PCI_LOG_INFO, "ack aspm ctrl val:0x%x", reg);
#ifdef _PRE_COMMENT_CODE_
            reg = oal_readl(pst_pci_lres->pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);
            reg &= ~(0x3ff);
            reg |= (0x2 << 0); /* aux_clk 2m,actual 1.92M  38.4M/20 */
            oal_writel(reg, pst_pci_lres->pst_pci_res->pst_pci_dbi_base + PCIE_AUX_CLK_FREQ_OFF);
            pci_print_log(PCI_LOG_INFO, "aux_clk_freq val:0x%x", reg);
#endif
        } else {
            pci_print_log(PCI_LOG_ERR, "pci res null or ctrl_base is null");
        }

        kirin_pcie_lp_ctrl(g_kirin_rc_idx, 1);
    }
    pci_print_log(PCI_LOG_INFO, "g_pcie_aspm_enable:%d", g_pcie_aspm_enable);

    /* rc noc protect */
    if (pst_rc_dev != NULL) {
        oal_pci_read_config_word(pst_rc_dev, oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_DEVCTL2, &val);
#ifdef _PRE_COMMENT_CODE_
        /* 调整rc pcie rsp 超时时间，对kirin noc有影响 */
        val |= (0xe << 0);
        oal_pci_write_config_word(pst_rc_dev, oal_pci_pcie_cap(pst_rc_dev) + PCI_EXP_DEVCTL2, val);
#endif
        pci_print_log(PCI_LOG_INFO, "devctrl:0x%x", val);
    }
#endif

    return OAL_SUCC;
}

#ifdef CONFIG_ARCH_KIRIN_PCIE
OAL_STATIC int32_t oal_pcie_get_mps(oal_pci_dev_stru *pst_pcie_dev)
{
    int32_t ret;
    int32_t pos;
    uint16_t reg16 = 0;

    pos = pci_find_capability(pst_pcie_dev, PCI_CAP_ID_EXP);
    if (!pos) {
        return -1;
    }

    ret = oal_pci_read_config_word(pst_pcie_dev, pos + PCI_EXP_DEVCAP, &reg16);
    if (ret) {
        pci_print_log(PCI_LOG_ERR, "pci read devcap failed ret=%d", ret);
        return -1;
    }

    return (int32_t)(reg16 & PCI_EXP_DEVCAP_PAYLOAD);
}
#endif

/* Max Payload Size Supported,  must config beofre pcie access */
OAL_STATIC int32_t oal_pcie_mps_init(oal_pcie_linux_res *pst_pci_lres)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    int32_t rc_mps, ep_mps;
    uint32_t mps, mrq;
    oal_pci_dev_stru *pst_rc_dev = NULL;

    if (!g_pcie_performance_mode) {
        return OAL_SUCC;
    }

    pst_rc_dev = pci_upstream_bridge(pst_pci_lres->pst_pcie_dev);
    if (pst_rc_dev == NULL) {
        pci_print_log(PCI_LOG_ERR, "no upstream dev");
        return -OAL_ENODEV;
    }

    rc_mps = oal_pcie_get_mps(pst_rc_dev);
    ep_mps = oal_pcie_get_mps(pst_pci_lres->pst_pcie_dev);
    if (rc_mps < 0 || ep_mps < 0) {
        pci_print_log(PCI_LOG_ERR, "mps get failed, rc_mps:%d  , ep_mps:%d", rc_mps, ep_mps);
        return -OAL_EFAIL;
    }

    mps = (uint32_t)oal_min(rc_mps, ep_mps);
    mrq = mps;
    pci_print_log(PCI_LOG_INFO, "rc/ep max payload size, rc_mps:%d  , ep_mps:%d, select mps:%d bytes",
                  rc_mps, ep_mps, OAL_PCIE_MIN_MPS << mps);
    mps <<= 5;  /* PCI_EXP_DEVCTL_PAYLOAD = 0b1110 0000 */
    mrq <<= 12; /* PCI_EXP_DEVCTL_READRQ  = 0b0111 0000 0000 0000 */

    pcie_capability_clear_and_set_word(pst_pci_lres->pst_pcie_dev, PCI_EXP_DEVCTL,
                                       PCI_EXP_DEVCTL_READRQ, mrq);
    pcie_capability_clear_and_set_word(pst_pci_lres->pst_pcie_dev, PCI_EXP_DEVCTL,
                                       PCI_EXP_DEVCTL_PAYLOAD, mps);
    pcie_capability_clear_and_set_word(pst_rc_dev, PCI_EXP_DEVCTL,
                                       PCI_EXP_DEVCTL_PAYLOAD, mps);
    pcie_capability_clear_and_set_word(pst_rc_dev, PCI_EXP_DEVCTL,
                                       PCI_EXP_DEVCTL_READRQ, mrq);
#endif
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_host_ip_edma_init(oal_pcie_linux_res *pst_pci_lres)
{
    if (pst_pci_lres->pst_pci_res->chip_info.edma_support != OAL_TRUE) {
        return;
    }
    oal_pcie_set_device_soft_fifo_enable(pst_pci_lres->pst_pci_res);
    oal_pcie_set_device_ringbuf_bugfix_enable(pst_pci_lres->pst_pci_res);
    oal_pcie_set_device_dma_check_enable(pst_pci_lres->pst_pci_res);
}

OAL_STATIC void oal_pcie_host_ip_ete_init(oal_pcie_linux_res *pst_pci_lres)
{
    if (pst_pci_lres->pst_pci_res->chip_info.ete_support != OAL_TRUE) {
        return;
    }
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "oal_pcie_host_ip_ete_init");
    (void)oal_ete_ip_init(pst_pci_lres->pst_pci_res);
}

int32_t oal_pcie_host_ip_init(oal_pcie_linux_res *pst_pci_lres)
{
    oal_pcie_mps_init(pst_pci_lres);
    oal_pcie_host_aspm_init(pst_pci_lres);
    oal_pcie_host_ip_edma_init(pst_pci_lres);
    oal_pcie_host_ip_ete_init(pst_pci_lres);
    return OAL_SUCC;
}

void oal_pcie_print_chip_info(oal_pcie_linux_res *pst_pci_lres, uint32_t is_full_log)
{
    /* L1 recovery count */
    uint32_t l1_err_cnt = oal_readl(pst_pci_lres->pst_pci_res->pst_pci_ctrl_base + PCIE_STAT_CNT_LTSSM_ENTER_RCVRY_OFF);
    uint32_t l1_recvy_cnt = oal_readl(pst_pci_lres->pst_pci_res->pst_pci_ctrl_base + PCIE_STAT_CNT_L1_ENTER_RCVRY_OFF);
    const uint32_t l1_max_err_cnt = 3;

    if (pst_pci_lres->l1_err_cnt == 0) {
        pst_pci_lres->l1_err_cnt = l1_err_cnt;
    }

    if (is_full_log) {
        if (l1_err_cnt > l1_max_err_cnt - 1) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "not l1 recovry error count %u, link_state unstable", l1_err_cnt);
        } else {
            /* 建链过程中会进入2次 */
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "link_state stable, l1 excp count:%u", l1_err_cnt);
        }

        if (l1_recvy_cnt) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "l1 enter count is %u", l1_recvy_cnt);
        } else {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "l1 maybe not enable");
        }
    } else {
        if (l1_err_cnt > l1_max_err_cnt - 1) {
            if ((pst_pci_lres->l1_err_cnt == 0) || (l1_err_cnt > pst_pci_lres->l1_err_cnt)) {
                /* link state error */
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "not l1 recovry error count %u, link_state unstable", l1_err_cnt);
                declare_dft_trace_key_info("pcie_link_state_unstable", OAL_DFT_TRACE_FAIL);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_DBG, "link_state stable, l1 excp count:%u", l1_err_cnt);
            }
        }
    }

    pst_pci_lres->l1_err_cnt = l1_err_cnt;
}

void oal_pcie_chip_info(hcc_bus *pst_bus, uint32_t is_need_wakeup, uint32_t is_full_log)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    if (oal_unlikely(pst_pci_lres->pst_pci_res->link_state <= PCI_WLAN_LINK_DOWN)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "link_state:%s",
                             oal_pcie_get_link_state_str(pst_pci_lres->pst_pci_res->link_state));
        return;
    }

    if (is_need_wakeup == OAL_TRUE) {
        if (pst_bus->bus_dev == NULL) {
            return;
        }
    }

    if (is_need_wakeup == OAL_TRUE) {
        hcc_tx_transfer_lock(pst_bus->bus_dev->hcc);
        if (oal_unlikely(hcc_bus_pm_wakeup_device(pst_pci_lres->pst_bus) != OAL_SUCC)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "wakeup device failed");
            hcc_tx_transfer_unlock(pst_bus->bus_dev->hcc);
            return;
        }
    }

    oal_pcie_print_chip_info(pst_pci_lres, is_full_log);

    if (is_need_wakeup == OAL_TRUE) {
        hcc_tx_transfer_unlock(pst_bus->bus_dev->hcc);
    }
}

int32_t oal_pcie_ip_init(hcc_bus *pst_bus)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = NULL;
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null");
        return -OAL_ENODEV;
    }

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_pci_lres is null");
        return -OAL_ENODEV;
    }

    ret = oal_pcie_device_auxclk_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }

    ret = oal_pcie_device_aspm_init(pst_pci_lres->pst_pci_res);
    if (ret) {
        return ret;
    }

    /* 使能低功耗 */
    ret = oal_pcie_host_aspm_init(pst_pci_lres);
    if (ret) {
        return ret;
    }

    return OAL_SUCC;
}

int32_t oal_pcie_ip_voltage_bias_init(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres;
    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }
    return oal_pcie_voltage_bias_init(pst_pci_lres->pst_pci_res);
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
OAL_STATIC OAL_INLINE int32_t oal_ete_dual_pci_power_down(oal_pcie_linux_res *pst_pci_lres)
{
    if (pst_pci_lres->pst_pcie_dev_dual == NULL) {
        /* no dual pci */
        return OAL_SUCC;
    }
    oal_pcie_power_notifiy_register(g_pcie_linux_res->pst_pcie_dev_dual, g_kirin_rc_idx_dual, NULL,
                                    wlan_dual_first_power_off_callback, NULL);
    return oal_pcie_pm_control(g_pcie_linux_res->pst_pcie_dev_dual, g_kirin_rc_idx_dual, PCIE_POWER_OFF_WITHOUT_L3MSG);
}
#endif

#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
/* request 会有申请内存操作，这里不能加自旋锁保护;  该函数只限于上下电调用，不会引起竞争 */
int32_t oal_enable_pcie_irq_with_request(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret = OAL_SUCC;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    if (pst_pci_lres->irq_stat == 1) {
        if (!pst_pci_lres->st_msi.is_msi_support) {
            /* intx */
            pci_print_log(PCI_LOG_INFO, "enable_pcie_irq with request_irq");
            ret = request_irq(pst_pci_lres->pst_pcie_dev->irq, oal_pcie_intx_edma_isr, IRQF_SHARED,
                "hisi_edma_intx", (void *)pst_pci_lres);
            if (ret < 0) {
                oal_io_print("enable_pcie_irq with request_irq error\n");
                oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);
                return -OAL_EINTR;
            }
        }
        pst_pci_lres->irq_stat = 0;
    }

    return OAL_SUCC;
}

int32_t oal_disable_pcie_irq_with_free(oal_pcie_linux_res *pst_pci_lres)
{
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    if (pst_pci_lres->irq_stat == 0) {
        if (!pst_pci_lres->st_msi.is_msi_support) {
            /* intx */
            pci_print_log(PCI_LOG_INFO, "disable_pcie_irq with free_irq");
            free_irq(pst_pci_lres->pst_pcie_dev->irq, pst_pci_lres);
            pst_pci_lres->irq_stat = 1;
        }
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC void oal_pcie_power_down(hcc_bus *pst_bus)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    pcie_ep_ip_status old_power_status = pst_pci_lres->power_status;
    int32_t ret;

    /* disable intx gpio... 等待中断处理完 */
#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
    ret = oal_disable_pcie_irq_with_free(pst_pci_lres);
#else
    ret = oal_disable_pcie_irq(pst_pci_lres);
#endif
    if (ret != OAL_SUCC) {
        return;
    }

    pst_pci_lres->power_status = PCIE_EP_IP_POWER_DOWN;
    if (pst_pci_lres->pst_pci_res == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_pci_res is null");
        return;
    }

    pst_pci_lres->l1_err_cnt = 0;

    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_change_link_state(pst_pci_lres->pst_pci_res, PCI_WLAN_LINK_DOWN);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

    oal_task_kill(&pst_pci_lres->st_rx_task);

    mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    oal_pcie_disable_regions(pst_pci_lres->pst_pci_res);
    oal_pcie_transfer_res_exit(pst_pci_lres->pst_pci_res);
    mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);

#ifdef CONFIG_ARCH_KIRIN_PCIE
    oal_kirin_pcie_deregister_event(pst_pci_lres);
#endif

    /* 下电之前关闭 PCIE HOST 控制器 */
#ifdef _PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE
    ret = oal_pcie_pm_control(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, PCIE_POWER_OFF_WITH_L3MSG);
#elif defined(CONFIG_ARCH_KIRIN_PCIE)

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
    ret = oal_pcie_pm_control(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, PCIE_POWER_OFF_WITHOUT_L3MSG);
#else
    ret = oal_pcie_pm_control(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, PCIE_POWER_OFF_WITH_L3MSG);
#endif

#endif

#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
    if (old_power_status != PCIE_EP_IP_POWER_DOWN) {
        if (ret) {
            declare_dft_trace_key_info("pcie_power_down_fail", OAL_DFT_TRACE_EXCEP);
        }
    }

    oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, NULL, NULL, NULL);
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    oal_ete_dual_pci_power_down(pst_pci_lres);
#endif
#endif
}

void pcie_bus_power_down_action(hcc_bus *pst_bus)
{
    /* 关掉LINKDOWN注册回调函数 */
    hcc_disable(hbus_to_hcc(pst_bus), OAL_TRUE);
    oal_wlan_gpio_intr_enable(hbus_to_dev(pst_bus), OAL_FALSE);
    hcc_transfer_lock(hbus_to_hcc(pst_bus));
    oal_pcie_power_down(pst_bus);
    hcc_transfer_unlock(hbus_to_hcc(pst_bus));
    board_host_wakeup_dev_set(0);
}

int32_t oal_pcie_patch_read(hcc_bus *pst_bus, uint8_t *buff, int32_t len, uint32_t timeout)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_likely(pst_pci_lres->pst_pci_res)) {
        mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    }

    ret = oal_pcie_firmware_read(pst_pci_lres, buff, len, timeout);

    if (oal_likely(pst_pci_lres->pst_pci_res)) {
        mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    }

    return ret;
}

int32_t oal_pcie_patch_write(hcc_bus *pst_bus, uint8_t *buff, int32_t len)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres;

    pst_pci_lres = (oal_pcie_linux_res *)pst_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        oal_io_print("oal_pcie_patch_write failed, lres is null\n");
        return -OAL_ENODEV;
    }

    if (oal_likely(pst_pci_lres->pst_pci_res)) {
        mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    }
    ret = oal_pcie_firmware_write(pst_pci_lres, buff, len);
    if (oal_likely(pst_pci_lres->pst_pci_res)) {
        mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
    }
    return ret;
}

int32_t oal_pcie_gpio_irq(hcc_bus *hi_bus, int32_t irq)
{
    oal_uint ul_state;
    oal_pcie_linux_res *pst_pci_lres;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;

    if (oal_unlikely(hi_bus == NULL)) {
        oal_io_print(KERN_ERR "pcie bus is null, irq:%d\n", irq);
        return -OAL_EINVAL;
    }

    if (!hi_bus->pst_pm_callback || !hi_bus->pst_pm_callback->pm_state_get) {
        oal_io_print("GPIO interrupt function param is NULL\r\n");
        return -OAL_EINVAL;
    }

    hi_bus->gpio_int_count++;

    if (oal_atomic_read(&g_wakeup_dev_wait_ack)) {
        pci_print_log(PCI_LOG_INFO, "pcie_gpio_irq wakeup dev ack");
        hi_bus->pst_pm_callback->pm_wakeup_dev_ack();
    }

    if (oal_atomic_read(&g_bus_powerup_dev_wait_ack)) {
        pci_print_log(PCI_LOG_INFO, "pcie_gpio_irq powerup dev ack");
        if (oal_likely(pst_wlan_pm != NULL)) {
            oal_complete(&pst_wlan_pm->st_wifi_powerup_done);
        } else {
            pci_print_log(PCI_LOG_INFO, "pst_wlan_pm is null");
        }
    }

    if (oal_unlikely(pst_pci_lres != NULL)) {
        if (oal_atomic_read(&pst_pci_lres->st_pcie_wakeup_flag)) {
            pci_print_log(PCI_LOG_INFO, "pcie_gpio_irq dev ready ack");
            if (oal_likely(pst_wlan_pm != NULL)) {
                oal_complete(&pst_wlan_pm->st_wifi_powerup_done);
                up(&hi_bus->rx_sema);
            } else {
                pci_print_log(PCI_LOG_INFO, "pst_wlan_pm is null");
            }
        }
    }

    ul_state = hi_bus->pst_pm_callback->pm_state_get();
    if (ul_state == 0) {
        /* 0==HOST_DISALLOW_TO_SLEEP表示不允许休眠 */
        hi_bus->data_int_count++;
        /* PCIE message use gpio interrupt */
        pci_print_log(PCI_LOG_DBG, "pcie message come..");
        up(&hi_bus->rx_sema);
    } else {
        /* 1==HOST_ALLOW_TO_SLEEP表示当前是休眠，唤醒host */
        if (oal_warn_on(!hi_bus->pst_pm_callback->pm_wakeup_host)) {
            pci_print_log(PCI_LOG_ERR, "%s error:hi_bus->pst_pm_callback->pm_wakeup_host is null", __FUNCTION__);
            return -OAL_FAIL;
        }

        hi_bus->wakeup_int_count++;
    pci_print_log(PCI_LOG_INFO, "trigger wakeup work...");
        hi_bus->pst_pm_callback->pm_wakeup_host();
    }
    return OAL_SUCC;
}

int32_t oal_pcie_gpio_rx_data(hcc_bus *hi_bus)
{
    int32_t ret;
    uint32_t message;
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return -OAL_EINVAL;
    }

    /* read message from device, read until ringbuf empty */
    forever_loop() {
        if (!pst_pci_lres->pst_pci_res->regions.inited) {
            oal_io_print("pcie rx data region is disabled!\n");
            return -OAL_ENODEV;
        }

        /* read one message */
        hcc_bus_wake_lock(hi_bus);
        mutex_lock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
        ret = oal_pcie_read_d2h_message(pst_pci_lres->pst_pci_res, &message);
        mutex_unlock(&pst_pci_lres->pst_pci_res->st_rx_mem_lock);
        hcc_bus_wake_unlock(hi_bus);
        if (ret != OAL_SUCC) {
            break;
        }

        pci_print_log(PCI_LOG_DBG, "pci  message : %u", message);
        /* get message succ. */
        if (message < D2H_MSG_COUNT) {
            /* standard message */
            hi_bus->msg[message].count++;
            hi_bus->last_msg = message;
            hi_bus->msg[message].cpu_time = cpu_clock(UINT_MAX);
            if (hi_bus->msg[message].msg_rx) {
                hi_bus->msg[message].msg_rx(hi_bus->msg[message].data);
            } else {
                oal_io_print("pcie mssage :%u no callback functions\n", message);
            }
        } else {
            if (message == PCIE_D2H_MESSAGE_HOST_READY_ACK) {
                pci_print_log(PCI_LOG_INFO, "d2h host read ack");
                oal_complete(&pst_pci_lres->st_pcie_ready);
            }
        }
    }

    return OAL_SUCC;
}

void oal_pcie_print_trans_info(hcc_bus *hi_bus, uint64_t print_flag)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    oal_pcie_print_transfer_info(pst_pci_lres->pst_pci_res, print_flag);
}

void oal_pcie_reset_trans_info(hcc_bus *hi_bus)
{
    oal_pcie_linux_res *pst_pci_lres = (oal_pcie_linux_res *)hi_bus->data;
    if (oal_warn_on(pst_pci_lres == NULL)) {
        return;
    }

    oal_pcie_reset_transfer_info(pst_pci_lres->pst_pci_res);
}

#ifdef CONFIG_KIRIN_PCIE_L1SS_IDLE_SLEEP
void oal_pcie_wlan_pm_vote(hcc_bus *hi_bus, uint8_t uc_allow)
{
    /* PCIe 32K feature, control rc fnpll clock */
    /* 0 means allow to fnpll gating */
    /* 1 means disallow to fnpll gating */
    kirin_pcie_refclk_device_vote(PCIE_DEVICE_WLAN, g_kirin_rc_idx, (uc_allow == ALLOW_IDLESLEEP) ? 0 : 1);
}
#endif

OAL_STATIC hcc_bus *oal_pcie_bus_init(oal_pcie_linux_res *pst_pci_lres)
{
    int32_t ret;
    hcc_bus *pst_bus = NULL;

    pst_bus = hcc_alloc_bus();
    if (pst_bus == NULL) {
        oal_io_print("alloc pcie hcc bus failed, size:%u\n", (uint32_t)sizeof(hcc_bus));
        return NULL;
    }

    pst_bus->bus_type = HCC_BUS_PCIE;
    pst_bus->bus_id = 0x0;
    pst_bus->dev_id = HCC_CHIP_110X_DEV; /* 这里可以根据 vendor id 区分110X 和118X */

    /* PCIE 只需要4字节对齐, burst大小对性能的影响有限 */
    pst_bus->cap.align_size[HCC_TX] = sizeof(uint32_t);
    pst_bus->cap.align_size[HCC_RX] = sizeof(uint32_t);
    pst_bus->cap.max_trans_size = 0x7fffffff;

    if (pst_pci_lres->pst_pci_res->chip_info.edma_support == OAL_TRUE) {
        oal_pcie_edma_bus_ops_init(pst_bus);
    } else if (pst_pci_lres->pst_pci_res->chip_info.ete_support == OAL_TRUE) {
        oal_pcie_ete_bus_ops_init(pst_bus);
    } else {
        pst_bus->opt_ops = NULL;
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unsupport chip, bus init failed!\n");
        hcc_free_bus(pst_bus);
        return NULL;
    }

    pst_bus->data = (void *)pst_pci_lres;

    ret = hcc_add_bus(pst_bus, "pcie");
    if (ret) {
        oal_io_print("add pcie bus failed, ret=%d\n", ret);
        hcc_free_bus(pst_bus);
        return NULL;
    }

    pst_pci_lres->pst_bus = pst_bus;
    return pst_bus;
}

OAL_STATIC void oal_pcie_bus_exit(hcc_bus *pst_bus)
{
    hcc_remove_bus(pst_bus);
    hcc_free_bus(pst_bus);
}

#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
int32_t wlan_first_power_on_callback(void *data)
{
    oal_reference(data);
    /* before wlan chip power up, rc clock must on. */
    /* bootloader had delay 20us before reset pcie, soc requet 10us delay, need't delay here */
    hi_wlan_power_set(1);
    return 0;
}

int32_t wlan_first_power_off_fail_callback(void *data)
{
    oal_reference(data);
    /* 阻止麒麟枚举失败后下电关时钟 */
    oal_io_print("wlan_first_power_off_fail_callback\n");
    g_pcie_enum_fail_reg_dump_flag = 1;
    (void)ssi_dump_err_regs(SSI_ERR_PCIE_FST_POWER_OFF_FAIL);
    oal_chip_error_task_block();
    return -1;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
int32_t oal_pcie_dual_power_on_callback(void *data)
{
    oal_reference(data);
    oal_io_print("board_set_pcie_reset\n");
    board_set_pcie_reset(OAL_FALSE, OAL_TRUE);
    mdelay(10); /* need't delay */
    return 0;
}

OAL_STATIC int32_t oal_pcie_dual_power_off_fail_callback(void *data)
{
    oal_reference(data);
    /* 阻止麒麟枚举失败后下电关时钟 */
    oal_io_print("oal_pcie_dual_power_off_fail_callback\n");
    g_pcie_enum_fail_reg_dump_flag = 1;
    (void)ssi_dump_err_regs(SSI_ERR_PCIE_FST_POWER_OFF_FAIL);
    oal_chip_error_task_block();
    return -1;
}

OAL_STATIC int32_t wlan_dual_first_power_off_callback(void *data)
{
    oal_reference(data);
    board_set_pcie_reset(OAL_FALSE, OAL_FALSE);
    return 0;
}
#endif
int32_t wlan_first_power_off_callback(void *data)
{
    oal_reference(data);
    hi_wlan_power_set(0);
    return 0;
}
#endif

OAL_STATIC void oal_pcie_ringbuf_bugfix_init(void)
{
    char buff[100]; /* 100字节足够存放要读取的参数 */

    memset_s(buff, sizeof(buff), 0, sizeof(buff));

    if (get_cust_conf_string(INI_MODU_PLAT, "pcie_ringbuf_bugfix", buff, sizeof(buff) - 1) == INI_FAILED) {
        pci_print_log(PCI_LOG_INFO, "can't found ini:pcie_ringbuf_bugfix, bugfix stat:%d",
                      g_pcie_ringbuf_bugfix_enable);
        return;
    }

    if (!oal_strncmp("enable", buff, OAL_STRLEN("enable"))) {
        g_pcie_ringbuf_bugfix_enable = 1;
    } else if (!strncmp("disable", buff, OAL_STRLEN("disable"))) {
        g_pcie_ringbuf_bugfix_enable = 0;
    } else {
        pci_print_log(PCI_LOG_WARN, "unkdown ringbuf bugfix ini:%s", buff);
    }

    pci_print_log(PCI_LOG_INFO, "ringbuf bugfix %s", g_pcie_ringbuf_bugfix_enable ? "enable" : "disable");
}

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
OAL_STATIC int32_t oal_pcie_dts_init(void)
{
    return OAL_SUCC;
}
#else

OAL_STATIC int32_t oal_pcie_dts_init(void)
{
#ifndef _PRE_HI375X_PCIE
#ifdef _PRE_CONFIG_USE_DTS
    int ret;
    u32 pcie_rx_idx = 0;
    struct device_node *np = NULL;
    np = of_find_compatible_node(NULL, NULL, DTS_NODE_HI110X_WIFI);
    if (np == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "can't find node [%s]", DTS_NODE_HI110X_WIFI);
        return -OAL_ENODEV;
    }

    ret = of_property_read_u32(np, DTS_PROP_HI110X_PCIE_RC_IDX, &pcie_rx_idx);
    if (ret) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "read prop [%s] fail, ret=%d", DTS_PROP_HI110X_PCIE_RC_IDX, ret);
        return ret;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "%s=%d", DTS_PROP_HI110X_PCIE_RC_IDX, pcie_rx_idx);
#ifdef CONFIG_ARCH_KIRIN_PCIE
    g_kirin_rc_idx = pcie_rx_idx;
#endif
#endif
#endif
    return OAL_SUCC;
}
#endif

OAL_STATIC int32_t oal_pci_resource_init(void)
{
    int32_t ret;
    oal_pcie_linux_res *pst_pci_lres = NULL;

    /* Power off Wlan Chip */
    if (g_pcie_linux_res) {
        hcc_bus_disable_state(g_pcie_linux_res->pst_bus, OAL_BUS_STATE_ALL);
#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
        pst_pci_lres = (oal_pcie_linux_res *)g_pcie_linux_res->pst_bus->data;
#endif

        /* 保存PCIE 配置寄存器 */
        ret = oal_pcie_save_default_resource(g_pcie_linux_res);
        if (ret != OAL_SUCC) {
            oal_pcie_disable_device(g_pcie_linux_res);
#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
            oal_disable_pcie_irq_with_free(g_pcie_linux_res);
#else
            oal_disable_pcie_irq(g_pcie_linux_res);
#endif
            oal_pci_unregister_driver(&g_pcie_drv);
            hi_wlan_power_set(0);
#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
            oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, NULL, NULL, NULL);
#endif
            return ret;
        }

#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
        oal_pcie_power_notifiy_register(pst_pci_lres->pst_pcie_dev, g_kirin_rc_idx, NULL,
                                        wlan_first_power_off_callback, NULL);
#endif
#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
        oal_disable_pcie_irq_with_free(g_pcie_linux_res);
#else
        oal_disable_pcie_irq(g_pcie_linux_res);
#endif
        hcc_bus_power_action(g_pcie_linux_res->pst_bus, HCC_BUS_POWER_DOWN);
        hi_wlan_power_set(0);
        } else {
        oal_io_print("g_pcie_linux_res is null\n");
#ifdef CONFIG_ARCH_KIRIN_PCIE
        kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
#endif
        return -OAL_ENODEV;
    }
    return OAL_SUCC;
}

OAL_STATIC void oal_pcie_110x_enum_fail_proc(void)
{
    if (!g_pcie_enum_fail_reg_dump_flag) {
        (void)ssi_dump_err_regs(SSI_ERR_PCIE_ENUM_FAIL);
    }
}

OAL_STATIC int32_t oal_wait_pcie_probe_complete(void)
{
    int32_t ret = OAL_SUCC;
    unsigned int timeout = hi110x_get_emu_timeout(10 * HZ); // 10 sec
    if (oal_wait_for_completion_timeout(&g_probe_complete, (uint32_t)timeout)) {
        if (g_probe_ret != OAL_SUCC) {
            oal_io_print("pcie driver probe failed ret=%d, driname:%s\n", g_probe_ret, g_pcie_drv.name);
#ifdef CONFIG_HWCONNECTIVITY_PC
            ssi_dump_device_regs(SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI |
                SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL);
            chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_PCIE_PROBE_FAIL);
#endif
#ifdef CONFIG_ARCH_KIRIN_PCIE
            kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
#endif
            return g_probe_ret;
        }
    } else {
        oal_io_print("pcie driver probe timeout  driname:%s\n", g_pcie_drv.name);
        oal_pcie_110x_enum_fail_proc();
#ifdef CONFIG_HWCONNECTIVITY_PC
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
            CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_PCIE_PROBE_TIMEOUT);
#endif
#ifdef CONFIG_ARCH_KIRIN_PCIE
        kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
#endif
        return -OAL_ETIMEDOUT;
    }
    return ret;
}

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
int32_t oal_pcie_hi110x_dual_init(void)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    int32_t ret;
#endif
    if (g_dual_pci_support == OAL_FALSE) {
        oal_io_print("dual pcie not support\n");
        return OAL_SUCC;
    }

    g_probe_ret = -OAL_EFAUL;
    g_pcie_enum_fail_reg_dump_flag = 0;
    oal_init_completion(&g_probe_complete);
    oal_io_print("%s PCIe driver dual device start\n", g_pcie_drv.name);
#ifdef CONFIG_ARCH_KIRIN_PCIE
    /* 打开参考时钟 */
    ret = kirin_pcie_power_notifiy_register(g_kirin_rc_idx_dual, oal_pcie_dual_power_on_callback,
                                            oal_pcie_dual_power_off_fail_callback, NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("enumerate_prepare failed ret=%d\n", ret);
        return ret;
    }

    ret = oal_pcie_enumerate(g_kirin_rc_idx_dual);
    if (ret != OAL_SUCC) {
        oal_io_print("enumerate failed ret=%d\n", ret);
        kirin_pcie_power_notifiy_register(g_kirin_rc_idx_dual, NULL, NULL, NULL);
        return ret;
    }

    if (g_pcie_linux_res == NULL || g_pcie_linux_res->pst_pcie_dev_dual == NULL) {
        oal_io_print("no dual pcie device\n");
        return -OAL_ENODEV;
    }

    /* poweroff */
    oal_pcie_power_notifiy_register(g_pcie_linux_res->pst_pcie_dev_dual, g_kirin_rc_idx_dual, NULL,
                                    wlan_dual_first_power_off_callback, NULL);
    oal_pcie_pm_control(g_pcie_linux_res->pst_pcie_dev_dual, g_kirin_rc_idx_dual, PCIE_POWER_OFF_WITHOUT_L3MSG);
#endif
    return OAL_SUCC;
}
#endif

static void oal_pcie_110x_pre_init(void)
{
    /* init host pcie */
    oal_io_print("%s PCIe driver register start\n", g_pcie_drv.name); /* Debug */
    oal_init_completion(&g_probe_complete);

    g_probe_ret = -OAL_EFAUL;

    g_pcie_enum_fail_reg_dump_flag = 0;

    oal_pcie_dts_init();
}

/* trigger pcie probe, alloc pcie resource */
int32_t oal_pcie_110x_init(void)
{
    int32_t ret;

    oal_pcie_110x_pre_init();

    ret = oal_pci_register_driver(&g_pcie_drv);
    if (ret) {
        oal_io_print("pcie driver register failed ret=%d, driname:%s\n", ret, g_pcie_drv.name);
        return ret;
    }

#ifdef CONFIG_ARCH_KIRIN_PCIE
    /* 打开参考时钟 */
    ret = kirin_pcie_power_notifiy_register(g_kirin_rc_idx, wlan_first_power_on_callback,
                                            wlan_first_power_off_fail_callback, NULL);
    if (ret != OAL_SUCC) {
        oal_io_print("enumerate_prepare failed ret=%d\n", ret);
        oal_pci_unregister_driver(&g_pcie_drv);
        return ret;
    }

    ret = oal_pcie_enumerate(g_kirin_rc_idx);
    if (ret != OAL_SUCC) {
        oal_pcie_110x_enum_fail_proc();
        oal_io_print("enumerate failed ret=%d\n", ret);
        kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
        oal_pci_unregister_driver(&g_pcie_drv);
        return ret;
    }
#endif

    ret = oal_wait_pcie_probe_complete();
    if (ret != OAL_SUCC) {
        return ret;
    }
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    ret = oal_pcie_hi110x_dual_init();
    if (ret != OAL_SUCC) {
        return ret;
    }
#endif

    oal_pcie_voltage_bias_param_init();
    oal_pcie_ringbuf_bugfix_init();

    ret = oal_pci_resource_init();
    if (ret != OAL_SUCC) {
        oal_io_print("%s PCIe driver register not succ\n", g_pcie_drv.name);
#ifdef CONFIG_HWCONNECTIVITY_PC
        chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
            CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_PCIE_DRV_REG_FAIL);
#endif
        return ret;
    }

    oal_io_print("%s PCIe driver register succ\n", g_pcie_drv.name);
    return OAL_SUCC;
}

void oal_pcie_110x_exit(void)
{
#ifdef CONFIG_ARCH_KIRIN_PCIE
    kirin_pcie_power_notifiy_register(g_kirin_rc_idx, NULL, NULL, NULL);
#endif
    oal_pci_unregister_driver(&g_pcie_drv);
}
#endif

int32_t oal_pcie_110x_working_check(void)
{
    hcc_bus_dev *pst_bus_dev;
    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev == NULL) {
        return OAL_FALSE;
    }

    if (pst_bus_dev->bus_cap & HCC_BUS_PCIE_CAP) {
        return OAL_TRUE;
    } else {
        return OAL_FALSE;
    }
}

int32_t oal_wifi_platform_load_pcie(void)
{
    int32_t ret = OAL_SUCC;

    if (oal_pcie_110x_working_check() != OAL_TRUE) {
        /* pcie driver don't support */
        oal_io_print("pcie driver don't support\n");
        return OAL_SUCC;
    }

    /* WiFi 芯片上电 + PCIE 枚举 */
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
    ret = oal_pcie_110x_init();
#endif
#if defined(CONFIG_PCIE_KIRIN_SLT_HI110X) && defined(CONFIG_HISI_DEBUG_FS)
    if (ret == OAL_SUCC) {
        hi1103_pcie_chip_rc_slt_register();
    }
#endif
    return ret;
}

void oal_wifi_platform_unload_pcie(void)
{
    if (oal_pcie_110x_working_check() != OAL_TRUE) {
        return;
    }

#if defined(CONFIG_PCIE_KIRIN_SLT_HI110X) && defined(CONFIG_HISI_DEBUG_FS)
    hi1103_pcie_chip_rc_slt_unregister();
#endif
}

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
oal_pcie_linux_res *oal_get_pcie_linux_res(void)
{
    return g_pcie_linux_res;
}
#else
oal_pcie_linux_res *oal_get_pcie_linux_res(void)
{
    return NULL;
}
#endif
oal_module_symbol(oal_get_pcie_linux_res);

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
oal_pci_dev_stru *oal_get_wifi_pcie_dev(void)
{
    oal_pcie_linux_res *pcie_linux_res = oal_get_pcie_linux_res();
    if (pcie_linux_res == NULL) {
        return NULL;
    }
    return pcie_linux_res->pst_pcie_dev;
}
#else
oal_pci_dev_stru *oal_get_wifi_pcie_dev(void)
{
    return NULL;
}
#endif
oal_module_symbol(oal_get_wifi_pcie_dev);
