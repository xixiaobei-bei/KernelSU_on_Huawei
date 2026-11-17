

#ifndef __PCIE_LINUX_H__
#define __PCIE_LINUX_H__

#include "oal_util.h"
#include "pcie_host.h"
#include "oal_schedule.h"
#ifdef CONFIG_ARCH_KIRIN_PCIE

#ifndef _PRE_HI375X_PCIE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <linux/platform_drivers/pcie-kirin-api.h>
#else
#include <linux/hisi/pcie-kirin-api.h>
#endif
#endif

#endif

#define OAL_PCIE_MIN_MPS 128

#define OAL_CONTINUOUS_LINK_UP_SUCC_TIMES 3

typedef enum _pcie_ep_ip_status_ {
    PCIE_EP_IP_POWER_DOWN = 0,
    PCIE_EP_IP_POWER_UP
} pcie_ep_ip_status;

#define  PCIE_HISI_VENDOR_ID             (0x19e5)
#define  PCIE_HISI_VENDOR_ID_HI1106      (0x19e7)
#define  PCIE_HISI_DEVICE_ID_HI1103      (0x1103)
#define  PCIE_HISI_DEVICE_ID_HI1105      (0x1105)
#define  PCIE_HISI_DEVICE_ID_HI1106FPGA  (0x1106)
#define  PCIE_HISI_DEVICE_ID_HI1106      (0x0006)


#define PCIE_DUAL_PCI_MASTER_FLAG  0x0000
#define PCIE_DUAL_PCI_SLAVE_FLAG   0x0001

#define PCIE_POWER_ON                1
#define PCIE_POWER_OFF_WITH_L3MSG    0 /* power off with L3 message */
#define PCIE_POWER_OFF_WITHOUT_L3MSG 2 /* power off without L3 message */

typedef struct _oal_pcie_linux_res__ {
    uint32_t version;  /* pcie version */
    uint32_t revision; /* 1:4.7a  , 2:5.00a */
    uint32_t irq_stat; /* 0:enabled, 1:disabled */
    uint32_t l1_err_cnt;
    oal_spin_lock_stru st_lock;
    oal_pci_dev_stru *pst_pcie_dev; /* Linux PCIe device hander */
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    oal_pci_dev_stru *pst_pcie_dev_dual; /* dual pcie, pcie1 */
#endif
    hcc_bus *pst_bus;
    oal_pcie_res *pst_pci_res;
    oal_pcie_msi_stru st_msi;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    struct pci_saved_state *default_state;
    struct pci_saved_state *state;
#endif
#ifdef CONFIG_ARCH_KIRIN_PCIE

#ifndef _PRE_HI375X_PCIE
    struct kirin_pcie_register_event pcie_event;
#endif
#endif

    oal_atomic  refcnt; /* for dual interface */
    int32_t     pci_cnt; /* pcie ip count */
    oal_mutex_stru sr_lock; /* for dual pcie */
#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
    struct pci_saved_state *default_state_dual;
    struct pci_saved_state *state_dual;
#ifdef CONFIG_ARCH_KIRIN_PCIE
    struct kirin_pcie_register_event pcie_event_dual;
#endif
#endif
    oal_tasklet_stru st_rx_task;

    pcie_ep_ip_status power_status;
    oal_atomic st_pcie_wakeup_flag;

    oal_completion st_pcie_ready;

    oal_wakelock_stru st_sr_bugfix; /* 暂时规避PCIE S/R失败的问题 */
} oal_pcie_linux_res;

typedef struct _pcie_wlan_callback_group_ {
    pcie_callback_stru pcie_mac_2g_isr_callback;
    pcie_callback_stru pcie_mac_5g_isr_callback;
    pcie_callback_stru pcie_phy_2g_isr_callback;
    pcie_callback_stru pcie_phy_5g_isr_callback;
} pcie_wlan_callback_group_stru;

typedef struct _oal_pcie_bindcpu_cfg_ {
    uint8_t is_bind;    /* 自动绑核指令,需保证存放在最低位 */
    uint8_t irq_cpu;    /* 用户绑定硬中断命令 */
    uint8_t thread_cmd; /* 用户绑定线程命令 */
    oal_bool_enum_uint8 is_userctl; /* 是否根据用户命令绑核 */
}oal_pcie_bindcpu_cfg;

#ifdef _PRE_PLAT_FEATURE_HI110X_DUAL_PCIE
extern int32_t g_dual_pci_support;
extern int32_t g_kirin_rc_idx_dual;
int32_t oal_pcie_dual_power_on_callback(void *data);
#endif

#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
int32_t pcie_wlan_func_register(pcie_wlan_callback_group_stru *p_func);
#else
OAL_STATIC OAL_INLINE int32_t pcie_wlan_func_register(pcie_wlan_callback_group_stru *p_func)
{
    return OAL_SUCC;
}
#endif

int32_t oal_wifi_platform_load_pcie(void);
void oal_wifi_platform_unload_pcie(void);

int32_t oal_pcie_set_loglevel(int32_t loglevel);
int32_t oal_pcie_set_hi11xx_loglevel(int32_t loglevel);
int32_t oal_disable_pcie_irq(oal_pcie_linux_res *pst_pci_lres);
int32_t oal_pcie_ip_factory_test(hcc_bus *pst_bus, int32_t test_count);
int32_t oal_pcie_ip_voltage_bias_init(hcc_bus *pst_bus);
int32_t oal_pcie_rc_slt_chip_transfer(hcc_bus *pst_bus, void *ddr_address,
                                      uint32_t data_size, int32_t direction);
int32_t oal_pcie_ip_init(hcc_bus *pst_bus);
int32_t oal_pcie_110x_working_check(void);

#if defined(_PRE_PRODUCT_HI1620S_KUNPENG) || defined(_PRE_WINDOWS_SUPPORT)
#undef CONFIG_ARCH_KIRIN_PCIE
#ifdef _PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE
int pci_power_notify_register(struct pci_dev *pdev, int (*poweron)(void* data),
                              int (*poweroff)(void* data), void* data);
int pci_rp_power_ctrl(struct pci_dev *pdev, int power_option);
int pci_dev_re_enumerate(struct pci_dev *pdev);
#endif
#endif
extern int32_t g_kirin_rc_idx;
#ifdef _PRE_HI375X_PCIE
int hipcie_enumerate(u32 rc_idx);
u32 show_link_state(u32 rc_id);
int hipcie_pm_control(int resume_flag, u32 rc_idx);
int hipcie_lp_ctrl(u32 rc_idx, u32 enable);
int hipcie_power_notifiy_register(u32 rc_idx, int (*poweron)(void* data),
                                  int (*poweroff)(void* data), void* data);
#define kirin_pcie_enumerate                hipcie_enumerate
#define kirin_pcie_pm_control               hipcie_pm_control
#define kirin_pcie_lp_ctrl                  hipcie_lp_ctrl
#define kirin_pcie_power_notifiy_register   hipcie_power_notifiy_register

#else
#ifdef CONFIG_ARCH_KIRIN_PCIE
/* hisi kirin PCIe functions */
int kirin_pcie_enumerate(u32 rc_idx);
u32 show_link_state(u32 rc_id);
int kirin_pcie_pm_control(int resume_flag, u32 rc_idx);
int kirin_pcie_lp_ctrl(u32 rc_idx, u32 enable);
/* notify WiFi when RC PCIE power on/off */
int kirin_pcie_power_notifiy_register(u32 rc_idx, int (*poweron)(void *data),
                                      int (*poweroff)(void *data), void *data);
#endif
#endif

oal_pcie_linux_res *oal_get_pcie_linux_res(void);
oal_pci_dev_stru *oal_get_wifi_pcie_dev(void);
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
// for hcc bus ops
int32_t oal_pcie_get_state(hcc_bus *pst_bus, uint32_t mask);
void oal_enable_pcie_state(hcc_bus *pst_bus, uint32_t mask);
void oal_disable_pcie_state(hcc_bus *pst_bus, uint32_t mask);
int32_t oal_pcie_rx_netbuf(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head);
int32_t oal_pcie_send_msg(hcc_bus *pst_bus, uint32_t val);
int32_t oal_pcie_host_lock(hcc_bus *pst_bus);
int32_t oal_pcie_host_unlock(hcc_bus *pst_bus);
int32_t oal_pcie_sleep_request(hcc_bus *pst_bus);
int32_t oal_pcie_wakeup_request(hcc_bus *pst_bus);
int32_t oal_pcie_get_sleep_state(hcc_bus *pst_bus);
int32_t oal_pcie_rx_int_mask(hcc_bus *pst_bus, int32_t is_mask);
int32_t oal_pcie_power_ctrl(hcc_bus *hi_bus, hcc_bus_power_ctrl_type ctrl,
                            int (*func)(void *data), void *data);
int32_t oal_pcie_gpio_irq(hcc_bus *hi_bus, int32_t irq);
int32_t oal_pcie_gpio_rx_data(hcc_bus *hi_bus);
int32_t oal_pcie_reinit(hcc_bus *pst_bus);
int32_t oal_pcie_deinit(hcc_bus *pst_bus);
int32_t oal_pcie_patch_read(hcc_bus *pst_bus, uint8_t *buff, int32_t len, uint32_t timeout);
int32_t oal_pcie_patch_write(hcc_bus *pst_bus, uint8_t *buff, int32_t len);
int32_t oal_pcie_bindcpu(hcc_bus *pst_bus, uint32_t chan, int32_t is_bind);
int32_t oal_pcie_get_trans_count(hcc_bus *hi_bus, uint64_t *tx, uint64_t *rx);
void oal_pcie_chip_info(hcc_bus *pst_bus, uint32_t is_need_wakeup, uint32_t is_full_log);
void oal_pcie_print_trans_info(hcc_bus *hi_bus, uint64_t print_flag);
void oal_pcie_reset_trans_info(hcc_bus *hi_bus);
void oal_pcie_wlan_pm_vote(hcc_bus *hi_bus, uint8_t uc_allow);

int32_t oal_enable_pcie_irq(oal_pcie_linux_res *pst_pci_lres);
void pcie_bus_power_down_action(hcc_bus *pst_bus);
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
int32_t oal_pcie_pm_control(struct pci_dev *pdev, u32 rc_idx, int power_option);
int32_t oal_pcie_power_notifiy_register(struct pci_dev *pdev, u32 rc_idx, int (*poweron)(void *data),
                                        int (*poweroff)(void *data), void *data);
#endif
int32_t oal_pcie_host_ip_init(oal_pcie_linux_res *pst_pci_lres);
#ifdef _PRE_CONFIG_PCIE_SHARED_INTX_IRQ
int32_t oal_enable_pcie_irq_with_request(oal_pcie_linux_res *pst_pci_lres);
#endif
int32_t oal_pcie_enable_device_func(oal_pcie_linux_res *pst_pci_lres);
void oal_pcie_print_chip_info(oal_pcie_linux_res *pst_pci_lres, uint32_t is_full_log);

int32_t oal_pcie_check_tx_param(hcc_bus *pst_bus, oal_netbuf_head_stru *pst_head,
                                hcc_netbuf_queue_type qtype);
void oal_pcie_rx_netbuf_hdr_init(oal_pci_dev_stru *hwdev, oal_netbuf_stru *pst_netbuf);

#if defined(CONFIG_ARCH_KIRIN_PCIE) || defined(_PRE_CONFIG_ARCH_HI1620S_KUNPENG_PCIE)
int32_t oal_pcie_device_wakeup_handler(const void *data);
#endif

void oal_pcie_edma_bus_ops_init(hcc_bus *pst_bus);
void oal_pcie_ete_bus_ops_init(hcc_bus *pst_bus);

int32_t oal_pcie_wakelock_active(hcc_bus *pst_bus);

int32_t oal_pcie_sr_para_check(oal_pcie_linux_res *pst_pci_lres,
                               hcc_bus **pst_bus, struct hcc_handler **pst_hcc);
int32_t oal_pcie_edma_suspend(oal_pci_dev_stru *pst_pci_dev, oal_pm_message_t state);
int32_t oal_pcie_edma_resume(oal_pci_dev_stru *pst_pci_dev);
int32_t oal_pcie_ete_suspend(oal_pci_dev_stru *pst_pci_dev, oal_pm_message_t state);
int32_t oal_pcie_ete_resume(oal_pci_dev_stru *pst_pci_dev);
int32_t oal_pcie_save_default_resource(oal_pcie_linux_res *pst_pci_lres);
#ifdef CONFIG_ARCH_KIRIN_PCIE
uint32_t oal_pcie_is_master_ip(oal_pci_dev_stru *pst_pci_dev);
int32_t oal_pcie_get_pcie_rc_idx(oal_pci_dev_stru *pst_pci_dev);
#endif

/* var declare */
extern int32_t g_pcie_print_once;
extern int32_t g_pcie_auto_disable_aspm;
extern int32_t g_hipci_msi_enable; /* 0 -intx 1-pci */
extern int32_t g_hipci_gen_select;
extern int32_t g_ft_pcie_aspm_check_bypass;
extern int32_t g_ft_pcie_gen_check_bypass;
extern int32_t g_hipci_sync_flush_cache_enable; // for device
extern int32_t g_hipci_sync_inv_cache_enable;   // for cpu
#endif
#endif

