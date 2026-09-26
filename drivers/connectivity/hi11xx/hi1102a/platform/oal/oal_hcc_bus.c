

/*lint -e801*/
#define HISI_LOG_TAG               "[HCC_BUS]"
#define HI11XX_LOG_MODULE_NAME     "[HCC_BUS]"
#define HI11XX_LOG_MODULE_NAME_VAR hcc_bus_loglevel
/* 头文件包含 */
#include "oal_hcc_bus.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#include "plat_exception_rst.h"
#endif

#include "oal_sdio_host_if.h"
#include "oal_hcc_host_if.h"
#include "oam_ext_if.h"
#include "plat_pm_wlan.h"
#include "plat_firmware.h"

#if defined(CONFIG_LOG_EXCEPTION) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
#include <log/log_usertype.h>
#endif

#include "securec.h"
#ifndef WIN32
#include "plat_pm.h"
#endif
#include "bfgx_exception_rst.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAL_HCC_BUS_C

/* 全局变量定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
OAL_STATIC oal_int32 hcc_bus_pm_state_switch_event(struct notifier_block *unused,
                                                   unsigned long wifi_state, void *ptr);
OAL_STATIC oal_dlist_create_head(g_hcc_bus_res_hdr);
OAL_STATIC oal_define_spinlock(g_hcc_bus_res_lock);
OAL_STATIC oal_dlist_create_head(g_hcc_bus_switch_hdr);

OAL_STATIC struct notifier_block g_hcc_bus_pm_notifier = {
    .notifier_call = hcc_bus_pm_state_switch_event,
};

/*
 * Set Format, split with ",", like below
 * "sdio,pcie,switchon" -- board support sdio & pcie, enable dynamic switch
 * "sdio,pcie,switchoff"
 * "pcie,sdio"          -- board support pcie & sdio, default we use pcie
 * "sdio"               -- only support sdio
 * "pcie"               -- only support pcie
 */
OAL_STATIC char *g_hisi_wifi_bus_select = "";
oal_debug_module_param(g_hisi_wifi_bus_select, charp, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_exception_enable = 1;
oal_debug_module_param(g_hcc_exception_enable, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_switch_bypass = 0;
oal_debug_module_param(g_hcc_bus_switch_bypass, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_switch_test_delay_time = 0;
oal_debug_module_param(g_hcc_bus_switch_test_delay_time, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_switch_test_break = 0;
oal_debug_module_param(g_hcc_bus_switch_test_break, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_auto_switch = 0;
oal_debug_module_param(g_hcc_bus_auto_switch, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_auto_bindcpu = 0;
oal_debug_module_param(g_hcc_bus_auto_bindcpu, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_manual_bindcpu = 1;
oal_debug_module_param(g_hcc_bus_manual_bindcpu, uint, S_IRUGO | S_IWUSR);

/* pps 按照大包计算 */
OAL_STATIC oal_uint32 g_hcc_bus_auto_bindcpu_limit = OAL_BUS_BINDCPU_LIMIT;
oal_debug_module_param(g_hcc_bus_auto_bindcpu_limit, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_current_pps = 0x0;
oal_debug_module_param(g_hcc_bus_current_pps, uint, S_IRUGO);

OAL_STATIC oal_uint32 g_hcc_bus_wakelock_debug = 0x0;
oal_debug_module_param(g_hcc_bus_wakelock_debug, uint, S_IRUGO | S_IWUSR);

OAL_STATIC oal_uint32 g_hcc_bus_auto_switch_limit = OAL_BUS_SWITCH_LIMIT;
oal_debug_module_param(g_hcc_bus_auto_switch_limit, uint, S_IRUGO | S_IWUSR);

OAL_STATIC char g_str_ini_hcc_bus_switch[OAL_BUS_INI_STR_LEN] = {0};
oal_debug_module_param_string(ini_hcc_bus_switch, g_str_ini_hcc_bus_switch,
                              sizeof(g_str_ini_hcc_bus_switch), S_IRUGO | S_IWUSR);
oal_debug_module_parm_desc(ini_hcc_bus_switch, "Ini string for hcc bus switch");

#else
OAL_STATIC oal_dlist_head_stru g_hcc_bus_res_hdr;
OAL_STATIC oal_spin_lock_stru g_hcc_bus_res_lock;
OAL_STATIC oal_dlist_head_stru g_hcc_bus_switch_hdr;
OAL_STATIC char *g_hisi_wifi_bus_select = "";
OAL_STATIC oal_uint32 g_hcc_exception_enable = 1;
OAL_STATIC oal_uint32 g_hcc_bus_switch_bypass = 0;
OAL_STATIC oal_uint32 g_hcc_bus_switch_test_delay_time = 0;
OAL_STATIC oal_uint32 g_hcc_bus_switch_test_break = 0;
OAL_STATIC char g_str_ini_hcc_bus_switch[OAL_BUS_INI_STR_LEN];
OAL_STATIC oal_uint32 g_hcc_bus_auto_switch = 0;
OAL_STATIC oal_uint32 g_hcc_bus_auto_bindcpu = 0;
OAL_STATIC oal_uint32 g_hcc_bus_manual_bindcpu = 1;
OAL_STATIC oal_uint32 g_hcc_bus_current_pps = 0x0;
OAL_STATIC oal_uint32 g_hcc_bus_wakelock_debug = 0x0;
/* pps 按照大包计算 */
OAL_STATIC oal_uint32 g_hcc_bus_auto_sdio_bindcpu_limit = OAL_BUS_SDIO_BINDCPU_LIMIT;
OAL_STATIC oal_uint32 g_hcc_bus_auto_pcie_bindcpu_limit = OAL_BUS_PCIE_BINDCPU_LIMIT;
OAL_STATIC oal_uint32 g_hcc_bus_auto_switch_limit = OAL_BUS_SWITCH_LIMIT;
#endif

OAL_STATIC oal_uint32 g_switch_ip_pwrdown_bypass = 1;

#ifdef WIN32
oal_uint32 jiffies;
#else
oal_debug_module_param(g_switch_ip_pwrdown_bypass, uint, S_IRUGO | S_IWUSR);
#endif

OAL_STATIC oal_int32 g_switch_total_count = 0;
/* 函数声明 */
oal_int32 oal_register_gpio_intr(hcc_bus_dev *pst_bus_dev);
oal_void oal_unregister_gpio_intr(hcc_bus_dev *pst_bus_dev);
OAL_STATIC oal_void hcc_bus_sched_gpio_task(hcc_bus *pst_bus, oal_int32 irq);
OAL_STATIC oal_void hcc_dev_pps_count_timeout(uintptr_t arg);
oal_int32 hcc_bus_auto_switch_is_support(oal_uint32 dev_id);
oal_int32 hcc_bus_auto_bindcpu_is_support(oal_uint32 dev_id);
oal_int32 hcc_bus_auto_sched_is_support(oal_uint32 dev_id);

OAL_STATIC hcc_bus *g_hisi_wifi_bus = OAL_PTR_NULL;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
/* 根据以下结构体来枚举 IP设备 */
OAL_STATIC hcc_bus_dev g_bus_dev_res[] = {
    {
        .dev_id = HCC_CHIP_110X_DEV,
#if defined(CONFIG_ARCH_SD56XX)
        .is_wakeup_gpio_support = 0,
        .init_bus_type = HCC_BUS_PCIE,
#else
        .init_bus_type = HCC_BUS_SDIO,
        .is_wakeup_gpio_support = 1,
#endif
#if defined(CONFIG_ARCH_SD56XX)
        .bus_cap = (HCC_BUS_PCIE_CAP),
#elif defined(_PRE_PLAT_FEATURE_HI110X_PCIE)
        .bus_cap = (HCC_BUS_SDIO_CAP) | (HCC_BUS_PCIE_CAP),
#else
        .bus_cap = (HCC_BUS_SDIO_CAP),
#endif
        .name = "hi110x_dev",
        .en_flowctrl_gpio_registered = OAL_FALSE,
    },
};
#else
OAL_STATIC hcc_bus_dev g_bus_dev_res[OAL_BUS_DEV_RES_SIZE];
#endif

OAL_STATIC oal_uint32 g_bus_dump_mem_flag = 0;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
oal_debug_module_param(g_bus_dump_mem_flag, uint, S_IRUGO | S_IWUSR);

oal_atomic g_wakeup_dev_wait_ack;
OAL_STATIC oal_atomic g_bus_powerup_dev_wait_ack; /* ip 上电握手标记 */
OAL_STATIC oal_uint32 g_pm_wakeup_event_flag = OAL_FALSE;
/*lint -e19*/
oal_module_symbol(g_pm_wakeup_event_flag);
/*lint +e19*/
#endif

/* -1 means did't bind cpus */
OAL_STATIC oal_int32 g_cpu_id = -1;

#ifdef WIN32
OAL_STATIC char *g_hcc_bus_type_str[HCC_BUS_BUTT] = {
    "sdio",
    "pcie",
    "usb"
};
#else
OAL_STATIC char *g_hcc_bus_type_str[HCC_BUS_BUTT] = {
    [HCC_BUS_SDIO] = "sdio",
    [HCC_BUS_PCIE] = "pcie",
    [HCC_BUS_USB] = "usb"
};
#endif

char *hcc_bus_get_bus_type_str(oal_uint32 bus_type)
{
    if (oal_unlikely(bus_type >= HCC_BUS_BUTT)) {
        return "unkown";
    }

    return g_hcc_bus_type_str[bus_type] ? g_hcc_bus_type_str[bus_type] : "uninit";
}

oal_void hcc_bus_stop_auto_pps_task(hcc_bus_dev *pst_bus_dev)
{
    /* cancel the timer */
    oal_timer_delete_sync(&pst_bus_dev->bus_pps_timer);
    pst_bus_dev->hcc->hcc_transer_info.tx_assem_info.pkt_cnt = 0;
    pst_bus_dev->hcc->hcc_transer_info.rx_assem_info.pkt_cnt = 0;
    hcc_dev_bindcpu(pst_bus_dev->dev_id, 0);
}

oal_void hcc_bus_start_auto_pps_task(hcc_bus_dev *pst_bus_dev)
{
    /* start the timer */
    pst_bus_dev->bus_pps_start_time = jiffies;
    oal_timer_start(&pst_bus_dev->bus_pps_timer, HCC_BUS_PPS_COUNT_TIMEOUT);
}

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
static oal_int32 hcc_bus_pm_state_switch_event(struct notifier_block *unused,
                                               unsigned long wifi_state, void *ptr)
{
    hcc_bus_dev *pst_bus_dev;
    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "110x dev is null");
        return -OAL_ENODEV;
    }

    if (wifi_state == WIFI_PM_POWERUP_EVENT) {
        g_switch_total_count = 0;
    }

    if (!hcc_bus_auto_sched_is_support(pst_bus_dev->dev_id)) {
        return OAL_SUCC;
    }

    if (wifi_state == WIFI_PM_WAKEUP_EVENT) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "110x dev wakeup");
        hcc_bus_start_auto_pps_task(pst_bus_dev);
    } else if (wifi_state == WIFI_PM_SLEEP_EVENT) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "110x dev sleep");
        hcc_bus_stop_auto_pps_task(pst_bus_dev);
    } else if (wifi_state == WIFI_PM_POWERDOWN_EVENT) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "110x dev powerdown");
        hcc_bus_stop_auto_pps_task(pst_bus_dev);
    } else if (wifi_state == WIFI_PM_POWERUP_EVENT) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "110x dev powerup");
        hcc_bus_start_auto_pps_task(pst_bus_dev);
    }

    return 0;
}
#endif

oal_int32 hcc_switch_action_register(hcc_switch_action *action, void *data)
{
    if (oal_unlikely(action == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    action->data = data;
    oal_dlist_add_tail(&action->list, &g_hcc_bus_switch_hdr);

    return OAL_SUCC;
}

oal_void hcc_switch_action_unregister(hcc_switch_action *action)
{
    if (oal_unlikely(action == NULL)) {
        oal_warn_on(1);
        return;
    }
    oal_dlist_delete_entry(&action->list);
}


static oal_int32 bus_chan_init_string_cmp(const char *start, int len, const char *pc_para)
{
    int cmp_len;

    cmp_len = (OAL_STRLEN(pc_para) > len) ? len : OAL_STRLEN(pc_para);
    if ((cmp_len == len) && (!oal_memcmp(start, pc_para, cmp_len))) {
        return OAL_SUCC;
    } else {
        return OAL_FAIL;
    }
}

static oal_uint32 hcc_bus_chan_set(hcc_bus_dev *pst_bus_dev, const char *start, uint32 len)
{
    if (len == 0) {
        return HCC_BUS_BUTT;
    }

    if (bus_chan_init_string_cmp(start, len, "sdio") == OAL_SUCC) {
        pst_bus_dev->bus_cap |= HCC_BUS_SDIO_CAP;
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hisi wifi support sdio");
        return HCC_BUS_SDIO;
    }

    if (bus_chan_init_string_cmp(start, len, "auto_bindcpu") == OAL_SUCC) {
        pst_bus_dev->bus_auto_bindcpu = 1;
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hisi hcc bus auto bindcpu enable");
        return HCC_BUS_BUTT;
    }

    return HCC_BUS_BUTT;
}

oal_void hcc_bus_chan_init_from_string(hcc_bus_dev *pst_bus_dev, char *bus_select)
{
    int len;
    char *start = NULL;
    char *end = NULL;
    oal_uint32 init_bus_type = HCC_BUS_BUTT;

    if (OAL_STRLEN(g_str_ini_hcc_bus_switch) != 0) {
        start = g_str_ini_hcc_bus_switch;
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "get ini string from module para [%s]", g_str_ini_hcc_bus_switch);
    } else {
        start = bus_select;
    }

    pst_bus_dev->bus_cap = 0;
    for (end = start;;) {
        /* search to separative sign */
        if ((*end != ',') && (*end != '\0')) {
            end++;
            continue;
        }
        len = end - start;

        init_bus_type = hcc_bus_chan_set(pst_bus_dev, start, len);
        /* select the first found bus type */
        if (init_bus_type != HCC_BUS_BUTT) {
            pst_bus_dev->init_bus_type = init_bus_type;
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "hisi wifi hcc bus type set to sdio");
        }

        if (*end == '\0') {
            break;
        }

        start = end + 1;
        end++;
    }
}

oal_int32 hcc_bus_cap_init(oal_uint32 dev_id, char *bus_select)
{
    hcc_bus_dev *pst_bus_dev;

    pst_bus_dev = hcc_get_bus_dev(dev_id);
    if (pst_bus_dev == NULL) {
        return -OAL_ENODEV;
    }

    if (g_hisi_wifi_bus_select != NULL &&
        OAL_STRLEN(g_hisi_wifi_bus_select) != 0) {
        hcc_bus_chan_init_from_string(pst_bus_dev, g_hisi_wifi_bus_select);
        return OAL_SUCC;
    }

    if (bus_select != NULL &&
        OAL_STRLEN(bus_select) != 0) {
        hcc_bus_chan_init_from_string(pst_bus_dev, bus_select);
        return OAL_SUCC;
    }

    /* default use sdio */
    hcc_bus_chan_init_from_string(pst_bus_dev, "sdio");

    return OAL_SUCC;
}

/* This interface is not good, remove later */
hcc_bus *hcc_get_current_110x_bus(oal_void)
{
    hcc_bus_dev *pst_bus_dev;
    pst_bus_dev = hcc_get_bus_dev(HCC_CHIP_110X_DEV);
    if (pst_bus_dev == NULL) {
        return NULL;
    }

    return pst_bus_dev->cur_bus;
}
/*lint -e19*/
oal_module_symbol(hcc_get_current_110x_bus);
/*lint +e19*/

oal_int32 hcc_bus_check_dev_id(oal_uint32 dev_id)
{
    if (oal_warn_on(dev_id == 0xFFFFFFFF)) {
        return -OAL_ENODEV;
    }

    if (oal_warn_on(dev_id >= (oal_uint32)oal_array_size(g_bus_dev_res))) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "invalid dev id :%d", dev_id);
        return -OAL_EINVAL;
    }

    return OAL_SUCC;
}

oal_int32 hcc_bus_is_match(hcc_bus *pst_new, hcc_bus *pst_old)
{
    if (pst_new->bus_type != pst_old->bus_type) {
        return OAL_FALSE;
    }

    if (pst_new->bus_id != pst_old->bus_id) {
        return OAL_FALSE;
    }

    return OAL_TRUE;
}

hcc_bus_dev *hcc_get_bus_dev(oal_uint32 dev_id)
{
    oal_int32 i;
    hcc_bus_dev *pst_bus_dev = NULL;

    if (hcc_bus_check_dev_id(dev_id) != OAL_SUCC) {
        return NULL;
    }

    for (i = 0; i < (oal_int32)oal_array_size(g_bus_dev_res); i++) {
        pst_bus_dev = &g_bus_dev_res[i];
        if (dev_id == pst_bus_dev->dev_id) {
            return pst_bus_dev;
        }
    }

    return NULL;
}

oal_int32 hcc_foreach_switch_actions(oal_uint32 dev_id, hcc_bus *old_bus, hcc_bus *new_bus)
{
    oal_int32 ret = OAL_SUCC;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_switch_action *pst_action = NULL;

    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_switch_hdr)
    {
        pst_action = oal_dlist_get_entry(pst_entry, hcc_switch_action, list);
        if (pst_action->switch_notify != NULL) {
            ret = pst_action->switch_notify(dev_id, old_bus, new_bus, pst_action->data);
            if (ret != OAL_SUCC) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc switch action probe failed,name:%s, ret=%d,func:%p",
                                     pst_action->name ? pst_action->name : "unkown", ret, pst_action->switch_notify);
                return ret;
            }
        }
    }

    return ret;
}

hcc_bus *hcc_find_bus_by_devid(oal_uint32 dev_id, oal_uint32 bus_type)
{
    oal_uint irqsave;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_bus *pst_bus_tmp = NULL;

    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);
    if (pst_bus_dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev_id %u invalid", dev_id);
        return NULL;
    }

    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_res_hdr)
    {
        pst_bus_tmp = oal_dlist_get_entry(pst_entry, hcc_bus, list);
        /* found the bus */
        if (pst_bus_tmp->bus_type == bus_type) {
            oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
            return pst_bus_tmp;
        }
    }
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);

    return NULL;
}

oal_int32 hcc_select_bus(oal_uint32 dev_id, oal_uint32 bus_type)
{
    oal_uint irqsave;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_bus *pst_bus_tmp = NULL;

    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);
    if (pst_bus_dev == NULL) {
        return -OAL_EINVAL;
    }

    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_res_hdr)
    {
        pst_bus_tmp = oal_dlist_get_entry(pst_entry, hcc_bus, list);
        if (pst_bus_tmp->bus_dev == pst_bus_dev) {
            if (pst_bus_tmp->bus_type == bus_type) {
                /* match */
                pst_bus_dev->cur_bus = pst_bus_tmp;
                oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
                return OAL_SUCC;
            }
        }
    }
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);

    return -OAL_ENODEV;
}

/* switch bus from sdio to pcie or pcie to sdio */
oal_int32 hcc_switch_bus(oal_uint32 dev_id, oal_uint32 bus_type)
{
    oal_int32 ret;
    hcc_bus *old_bus = NULL;
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);
    /* 暂时只允许在关机状态切换 */
    if (oal_warn_on(dev_id != HCC_CHIP_110X_DEV)) {
        return -OAL_EINVAL;
    }

    if (pst_bus_dev == NULL) {
        return -OAL_EINVAL;
    }

    if (oal_warn_on(pst_bus_dev->cur_bus == NULL)) {
        return -OAL_ENODEV;
    }

    oal_wake_lock(&pst_bus_dev->st_switch_wakelock);

    old_bus = pst_bus_dev->cur_bus;

    ret = hcc_select_bus(dev_id, bus_type);
    if (ret == OAL_SUCC) {
        /* call switch bus actions after bus changed */
        ret = hcc_foreach_switch_actions(dev_id, old_bus, pst_bus_dev->cur_bus);
    }

    oal_wake_unlock(&pst_bus_dev->st_switch_wakelock);
    hcc_sched_transfer(pst_bus_dev->hcc);
    return ret;
}

oal_int32 hcc_bus_isvalid(hcc_bus *pst_bus)
{
    if (pst_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus is null");
        return -OAL_EINVAL;
    }

    if (pst_bus->bus_dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus %d 's bus dev is null", pst_bus->bus_id);
        return -OAL_ENODEV;
    }

    return OAL_SUCC;
}

OAL_STATIC oal_int32 hcc_bus_rx_response(struct hcc_handler *hcc, oal_uint8 stype,
                                         hcc_netbuf_stru *pst_hcc_netbuf, oal_uint8 *pst_context)
{
    hcc_switch_response st_switch_info_resp;
    oal_uint16 transfer_type;
    oal_int32 ret;
    oal_reference(pst_context);

    ret = memcpy_s((oal_void *)&transfer_type, OAL_SIZEOF(transfer_type),
                   (oal_void *)oal_netbuf_data(pst_hcc_netbuf->pst_netbuf), OAL_SIZEOF(transfer_type));
    if (ret != EOK) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "get transfer type failed");
    }

    if (transfer_type == HCC_BUS_TRANSFER_SWITCH_ACTION) {
        if (oal_netbuf_len(pst_hcc_netbuf->pst_netbuf) < OAL_SIZEOF(transfer_type) + OAL_SIZEOF(st_switch_info_resp)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_rx_response netbuf too short, len %d < %d",
                                 (oal_int32)oal_netbuf_len(pst_hcc_netbuf->pst_netbuf),
                                 (oal_int32)(OAL_SIZEOF(transfer_type) + OAL_SIZEOF(st_switch_info_resp)));
            oal_print_hex_dump((oal_uint8 *)oal_netbuf_data(pst_hcc_netbuf->pst_netbuf),
                               oal_netbuf_len(pst_hcc_netbuf->pst_netbuf), HEX_DUMP_GROUP_SIZE, "switch resp: ");
        } else {
#ifdef WIN32
            ret = memcpy_s((char *)&st_switch_info_resp, OAL_SIZEOF(st_switch_info_resp),
                           (char *)oal_netbuf_data(pst_hcc_netbuf->pst_netbuf) + OAL_SIZEOF(transfer_type),
                           OAL_SIZEOF(st_switch_info_resp));
#else
            ret = memcpy_s((oal_void *)&st_switch_info_resp, OAL_SIZEOF(st_switch_info_resp),
                           (oal_void *)oal_netbuf_data(pst_hcc_netbuf->pst_netbuf) + OAL_SIZEOF(transfer_type),
                           OAL_SIZEOF(st_switch_info_resp));
#endif
            if (ret != EOK) {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "get hcc_switch_response failed");
            }

            if (st_switch_info_resp.action_type == HCC_BUS_SWITCH_POWERUP) {
                oal_print_hi11xx_log(HI11XX_LOG_INFO, "bus %d power up response, ret=%d",
                                     st_switch_info_resp.bus_id, st_switch_info_resp.is_succ);
                oal_complete(&hcc->bus_dev->st_switch_powerup_ready);
            } else if (st_switch_info_resp.action_type == HCC_BUS_SWITCH_POWERDOWN) {
                oal_print_hi11xx_log(HI11XX_LOG_INFO, "bus %d power down response, ret=%d",
                                     st_switch_info_resp.bus_id, st_switch_info_resp.is_succ);
                oal_complete(&hcc->bus_dev->st_switch_powerdown_ready);
            } else {
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_bus_rx_response unkown action type:%d",
                                     st_switch_info_resp.action_type);
            }
        }
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "unkown bus rx resposne type %d", transfer_type);
        oal_print_hex_dump((oal_uint8 *)oal_netbuf_data(pst_hcc_netbuf->pst_netbuf),
                           oal_netbuf_len(pst_hcc_netbuf->pst_netbuf), HEX_DUMP_GROUP_SIZE, "switch resp: ");
    }

    return OAL_SUCC;
}

oal_int32 hcc_bus_auto_switch_is_support(oal_uint32 dev_id)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);
    if (oal_unlikely(pst_bus_dev == NULL)) {
        return 0;
    }

    if (g_hcc_bus_auto_switch) {
        return 1;
    }

    return pst_bus_dev->bus_auto_switch;
}

oal_int32 hcc_bus_auto_bindcpu_is_support(oal_uint32 dev_id)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);
    if (oal_unlikely(pst_bus_dev == NULL)) {
        return 0;
    }

    if (g_hcc_bus_auto_bindcpu) {
        return 1;
    }

    return pst_bus_dev->bus_auto_bindcpu;
}

oal_int32 hcc_bus_auto_sched_is_support(oal_uint32 dev_id)
{
    if (!hcc_bus_auto_switch_is_support(dev_id) && !hcc_bus_auto_bindcpu_is_support(dev_id)) {
        return 0;
    }

    return 1;
}

oal_int32 hi110x_hcc_dev_bindcpu(oal_int32 is_bind)
{
    hcc_bus *pst_bus = NULL;
    oal_uint32 dev_id = HCC_CHIP_110X_DEV;
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);

    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "non-support switch dev id:%u", dev_id);
        return -OAL_EINVAL;
    }

    if (hcc_bus_auto_bindcpu_is_support(dev_id)) {
        /* 自动绑核打开直接返回 */
        return OAL_SUCC;
    }

    if (g_hcc_bus_manual_bindcpu == 0) {
        /* forbid manual control */
        return OAL_SUCC;
    }

    pst_bus = pst_bus_dev->cur_bus;

    return hcc_bus_bindcpu(pst_bus, HCC_DIR_COUNT, is_bind);
}
EXPORT_SYMBOL_GPL(hi110x_hcc_dev_bindcpu);

oal_int32 hcc_dev_bindcpu(oal_uint32 dev_id, oal_int32 is_bind)
{
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);

    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "non-support switch dev id:%u", dev_id);
        return -OAL_EINVAL;
    }

    pst_bus = pst_bus_dev->cur_bus;
    if (hcc_bus_auto_bindcpu_is_support(dev_id)) {
        return hcc_bus_bindcpu(pst_bus, HCC_DIR_COUNT, is_bind);
    } else {
        return -OAL_EIO;
    }
}
EXPORT_SYMBOL_GPL(hcc_dev_bindcpu);

/* bus性能调度 */
oal_int32 hcc_bus_performance_core_schedule(oal_uint32 dev_id)
{
    oal_uint32 run_time;
    oal_ulong current_time, pkt_pps_rx, pkt_pps_tx, pkt_pps_total;
    oal_ulong tx_cnt, rx_cnt, total_cnt;
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);

    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "non-support switch dev id:%u", dev_id);
        return -OAL_EINVAL;
    }

    if (!hcc_bus_auto_sched_is_support(dev_id)) {
        oal_print_hi11xx_log(HI11XX_LOG_VERBOSE, "hcc bus auto sched disabled");
        return OAL_SUCC;
    }

    current_time = jiffies;

    if (oal_time_after(current_time, (pst_bus_dev->bus_pps_start_time + 1 * HZ))) {
        /* time is up, calc pps */
        tx_cnt = pst_bus_dev->hcc->hcc_transer_info.tx_assem_info.pkt_cnt;
        rx_cnt = pst_bus_dev->hcc->hcc_transer_info.rx_assem_info.pkt_cnt;
        total_cnt = tx_cnt + rx_cnt;

        run_time = oal_jiffies_to_msecs(current_time)
                   - oal_jiffies_to_msecs(pst_bus_dev->bus_pps_start_time);
        if (run_time == 0) {
            run_time = 1; /* 1 msec at least */
        }

        pkt_pps_tx = (tx_cnt * MSEC_PER_SEC) / (oal_ulong)run_time;
        pkt_pps_rx = (rx_cnt * MSEC_PER_SEC) / (oal_ulong)run_time;

        pkt_pps_total = pkt_pps_tx + pkt_pps_rx;

        g_hcc_bus_current_pps = pkt_pps_total;

        if (hcc_bus_auto_bindcpu_is_support(dev_id)) {
            hcc_bus *pst_bus = pst_bus_dev->cur_bus;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
            if (pkt_pps_total >= g_hcc_bus_auto_bindcpu_limit) {
                hcc_bus_bindcpu(pst_bus, HCC_DIR_COUNT, 1);
            } else if (pkt_pps_total <= g_hcc_bus_auto_bindcpu_limit / 2) {
                hcc_bus_bindcpu(pst_bus, HCC_DIR_COUNT, 0);
            }
#endif
        }

        oal_print_hi11xx_log(HI11XX_LOG_DBG, "pkt_pps_tx: %lu , pkt_pps_rx: %lu, runtime:%u",
                             pkt_pps_tx, pkt_pps_rx, run_time);

        pst_bus_dev->hcc->hcc_transer_info.tx_assem_info.pkt_cnt = 0;
        pst_bus_dev->hcc->hcc_transer_info.rx_assem_info.pkt_cnt = 0;
        pst_bus_dev->bus_pps_start_time = current_time;
    }

    return OAL_SUCC;
}

oal_int32 hcc_bus_performance_core_init(oal_uint32 dev_id)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);

    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "non-support switch dev id:%u", dev_id);
        return -OAL_EINVAL;
    }

    pst_bus_dev->bus_pps_start_time = jiffies;
    pst_bus_dev->hcc->hcc_transer_info.tx_assem_info.pkt_cnt = 0;
    pst_bus_dev->hcc->hcc_transer_info.rx_assem_info.pkt_cnt = 0;
    oal_timer_init(&pst_bus_dev->bus_pps_timer, HCC_BUS_PPS_COUNT_TIMEOUT,
                   hcc_dev_pps_count_timeout, (uintptr_t)pst_bus_dev);

    return OAL_SUCC;
}

oal_int32 hcc_bus_performance_core_exit(oal_uint32 dev_id)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(dev_id);

    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "non-support switch dev id:%u", dev_id);
        return -OAL_EINVAL;
    }

    oal_timer_delete_sync(&pst_bus_dev->bus_pps_timer);
    return OAL_SUCC;
}

/* high speed chan */
oal_int32 hi110x_switch_to_hcc_highspeed_chan(oal_uint32 is_high)
{
    oal_uint32 target = is_high ? HCC_BUS_PCIE : HCC_BUS_SDIO;
    return hi110x_switch_hcc_bus_request(target);
}
/*lint -e19 */
oal_module_symbol(hi110x_switch_to_hcc_highspeed_chan);
/*lint +e19 */
oal_int32 hi110x_switch_hcc_bus_request(oal_uint32 target)
{
    if (g_hcc_bus_switch_bypass) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hi110x_switch_bus_request %d bypass", target);
        return OAL_SUCC;
    }

    if (hcc_bus_auto_switch_is_support(HCC_CHIP_110X_DEV)) {
        return OAL_SUCC;
    }

    return OAL_SUCC;
}
/*lint -e19 */
oal_module_symbol(hi110x_switch_hcc_bus_request);
/*lint +e19 */
oal_int32 hcc_switch_bus_loop_test(oal_uint32 dev_id, oal_uint32 loop_count)
{
    oal_uint32 i;
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *pst_bus_dev = NULL;
    const oal_uint32 ul_loop_times = loop_count * 2;

    for (i = 0; i < ul_loop_times; i++) {
        if (oal_unlikely(g_hcc_bus_switch_test_break)) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc_switch_bus_loop_test abort");
            return -OAL_EBUSY;
        }

        pst_bus_dev = hcc_get_bus_dev(dev_id);
        if (pst_bus_dev == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev id %d invalid", dev_id);
            return -OAL_ENODEV;
        }

        pst_bus = pst_bus_dev->cur_bus;
        if (pst_bus == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev id %d invalid, bus is null", dev_id);
            return -OAL_ENODEV;
        }

        if (g_hcc_bus_switch_test_delay_time) {
            oal_msleep(g_hcc_bus_switch_test_delay_time);
        }
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "test bus loop %u times done!", loop_count);

    return OAL_SUCC;
}

/* after all bus inited */
oal_int32 hcc_probe_bus(hcc_bus *pst_bus)
{
    hcc_bus_dev *pst_bus_dev = NULL;

    if (oal_warn_on(pst_bus->bus_dev != NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "%s had probed", pst_bus->name);
        return OAL_SUCC;
    }

    pst_bus_dev = hcc_get_bus_dev(pst_bus->dev_id);
    if (pst_bus_dev == NULL) {
        return -OAL_EINVAL;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "bus_type:%s, cap:%u, dev_name:%s",
                         hcc_bus_get_bus_type_str(pst_bus->bus_type), pst_bus_dev->bus_cap, pst_bus_dev->name);

    if ((1 << pst_bus->bus_type) & pst_bus_dev->bus_cap) {
        pst_bus->bus_dev = pst_bus_dev;
        pst_bus_dev->bus_num++;
        return OAL_SUCC;
    }

    return -OAL_ENODEV;
}

/*
 * Prototype    : hcc_message_register
 * Description  : used for sdio messsage callback, 1 bit to 1 msg event
 * Input        : struct hcc_handler *hcc
 */
oal_int32 hcc_message_register(struct hcc_handler *hcc, oal_uint8 msg,
                               hcc_msg_rx cb, oal_void *data)
{
    oal_int32 flag = 0;
    oal_int32 ret = OAL_SUCC;
    oal_uint irqsave;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_bus *pst_bus_tmp = NULL;

    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_res_hdr)
    {
        pst_bus_tmp = oal_dlist_get_entry(pst_entry, hcc_bus, list);
        if (pst_bus_tmp->bus_dev == hcc->bus_dev) {
            /* 注册在当前设备上的所有BUS 都需要注册 */
            ret = hcc_bus_message_register(pst_bus_tmp, msg, cb, data);
            if (ret != OAL_SUCC) {
                oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "register bus %d failed, ret=%d", pst_bus_tmp->bus_id, ret);
                return ret;
            }
            flag++;
        }
    }
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);

    if (flag) {
        return OAL_SUCC;
    } else {
        return -OAL_ENODEV;
    }
}

oal_void hcc_message_unregister(struct hcc_handler *hcc, oal_uint8 msg)
{
    oal_uint irqsave;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_bus *pst_bus_tmp = NULL;

    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return;
    }

    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_res_hdr)
    {
        pst_bus_tmp = oal_dlist_get_entry(pst_entry, hcc_bus, list);
        if (pst_bus_tmp->bus_dev == hcc->bus_dev) {
            /* 注册在当前设备上的所有BUS 都需要注册 */
            hcc_bus_message_unregister(pst_bus_tmp, msg);
        }
    }
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
}

hcc_bus *hcc_alloc_bus(oal_void)
{
    hcc_bus *pst_bus = OAL_PTR_NULL;

    pst_bus = oal_memalloc(OAL_SIZEOF(hcc_bus));
    if (pst_bus == OAL_PTR_NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc hcc bus failed, size:%u", (oal_uint32)OAL_SIZEOF(hcc_bus));
        return NULL;
    }

    memset_s((oal_void *)pst_bus, OAL_SIZEOF(hcc_bus), 0, OAL_SIZEOF(hcc_bus));

    pst_bus->dev_id = 0xFFFFFFFF;

    /* 设置4字节对齐 */
    pst_bus->cap.align_size[HCC_TX] = sizeof(oal_uint32);
    pst_bus->cap.align_size[HCC_RX] = sizeof(oal_uint32);

    pst_bus->cap.is_full_duplex = 0;

    pst_bus->cap.max_trans_size = 0x7fffffff;
    return pst_bus;
}

oal_void hcc_free_bus(hcc_bus *pst_bus)
{
    if (oal_likely(pst_bus != NULL)) {
        oal_free(pst_bus);
    }
}

oal_int32 hcc_send_message(struct hcc_handler *hcc, oal_uint32 val)
{
    oal_uint32 ret = OAL_SUCC;
    hcc_bus *hi_bus = NULL;

    if (oal_unlikely(hcc == NULL)) {
        return -OAL_ENODEV;
    }

    hi_bus = hcc_to_bus(hcc);

    hcc_tx_transfer_lock(hcc);
    if (hcc_bus_pm_wakeup_device(hi_bus) == OAL_SUCC) {
        ret = hcc_bus_send_message(hi_bus, val);
    } else {
        hcc_tx_transfer_unlock(hcc);
        return -OAL_EFAIL;
    }
    hcc_tx_transfer_unlock(hcc);

    return ret;
}

OAL_STATIC oal_void hcc_dev_res_exit_by_id(oal_int32 dev_max)
{
    oal_int32 i;
    hcc_bus_dev *pst_bus_dev = NULL;
    dev_max = oal_min(dev_max, (oal_int32)oal_array_size(g_bus_dev_res));
    for (i = 0; i < dev_max; i++) {
        pst_bus_dev = &g_bus_dev_res[i];
        oal_wake_lock_exit(&pst_bus_dev->st_switch_wakelock);
        if (pst_bus_dev->is_wakeup_gpio_support) {
            oal_unregister_gpio_intr(pst_bus_dev);
        }
        /* 注销GPIO流控 */
        oal_unregister_gpio_flowctrl_intr(pst_bus_dev);
    }
}

oal_void hcc_dev_res_exit(oal_void)
{
    hcc_dev_res_exit_by_id((oal_int32)oal_array_size(g_bus_dev_res));
}

oal_int32 hcc_dev_res_init(oal_void)
{
    oal_int32 i;
    oal_int32 ret;
    hcc_bus_dev *pst_bus_dev = NULL;
    for (i = 0; i < (oal_int32)oal_array_size(g_bus_dev_res); i++) {
        pst_bus_dev = &g_bus_dev_res[i];
        oal_spin_lock_init(&pst_bus_dev->st_irq_lock);
        oal_spin_lock_init(&pst_bus_dev->st_switch_lock);
        oal_wait_queue_init_head(&pst_bus_dev->st_switch_request_wq);
        oal_wake_lock_init(&pst_bus_dev->st_switch_wakelock, "bus_dev_wakelock");
        if (pst_bus_dev->is_wakeup_gpio_support) {
            ret = oal_register_gpio_intr(pst_bus_dev);
            if (ret) {
                goto failed_reg_gpio_int;
            }
        }
    }

    return OAL_SUCC;
failed_reg_gpio_int:
    hcc_dev_res_exit_by_id(i);
    return ret;
}

oal_int32 hcc_add_bus(hcc_bus *pst_bus, const char *bus_name)
{
    oal_int32 ret;
    oal_int32 init_flag = 0;
    oal_uint irqsave;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_bus *pst_bus_tmp = NULL;
    if (oal_warn_on(pst_bus == NULL)) {
        return -OAL_EINVAL;
    }

    /* Check duplicate bus */
    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_res_hdr)
    {
        pst_bus_tmp = oal_dlist_get_entry(pst_entry, hcc_bus, list);
        if (oal_warn_on(hcc_bus_is_match(pst_bus_tmp, pst_bus) == OAL_TRUE)) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus[%p] is duplicate, type:%s, id:%d",
                                 pst_bus, hcc_bus_get_bus_type_str(pst_bus->bus_type), pst_bus->bus_id);
            oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
            return -OAL_EBUSY;
        }
    }

    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);

    ret = hcc_bus_resource_alloc(pst_bus);
    if (ret != OAL_SUCC) {
        return ret;
    }

    ret = snprintf_s(pst_bus->name, OAL_SIZEOF(pst_bus->name), OAL_SIZEOF(pst_bus->name) - 1, "%s:%u",
                     bus_name, pst_bus->bus_id);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc_add_bus: snprintf_s failed.");
    }
    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    /* If empty init dev res */
    if (OAL_TRUE == oal_dlist_is_empty(&g_hcc_bus_res_hdr)) {
        oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
        ret = hcc_dev_res_init();
        oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
        if (ret) {
            goto failed_dev_res_init;
        }
        init_flag = 1;
    }

    oal_dlist_add_tail(&pst_bus->list, &g_hcc_bus_res_hdr);
    ret = hcc_probe_bus(pst_bus);
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hcc bus %s probed failed!", pst_bus->name);
        goto failed_probe_bus;
    }
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);

    return OAL_SUCC;
failed_probe_bus:
    oal_dlist_delete_entry(&pst_bus->list);
    if (init_flag) {
        oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
        hcc_dev_res_exit();
        oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    }
failed_dev_res_init:
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
    hcc_bus_resource_free(pst_bus);
    return ret;
}

oal_int32 hcc_remove_bus(hcc_bus *pst_bus)
{
    oal_uint irqsave;

    if (oal_warn_on(pst_bus == NULL)) {
        return OAL_SUCC;
    }

    /* delete bus and remove from dev resource */
    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);

    oal_dlist_delete_entry(&pst_bus->list);

    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
    hcc_bus_resource_free(pst_bus);
    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);

    if (oal_dlist_is_empty(&g_hcc_bus_res_hdr) == OAL_TRUE) {
        oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
        hcc_dev_res_exit();
        oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    }

    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
    return OAL_SUCC;
}

#if defined(CONFIG_ARCH_HISI)
/* Debug */
oal_int32 hcc_bus_printbindcpu(oal_void)
{
    hcc_bus *hi_bus;
    struct hcc_handler *hcc;
    struct cpumask slow_cpus, fast_cpus;

#ifdef CONFIG_ARCH_PLATFORM
    get_slow_cpus(&slow_cpus);
    get_fast_cpus(&fast_cpus);
#else
    hisi_get_slow_cpus(&slow_cpus);
    hisi_get_fast_cpus(&fast_cpus);
#endif

    hi_bus = hcc_get_current_110x_bus();
    if (hi_bus == NULL) {
        return -OAL_EFAIL;
    }

    hcc = hbus_to_hcc(hi_bus);
    if (hcc == NULL) {
        return -OAL_EFAIL;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc cpu mask:0x%lx",
                         cpumask_bits(&(hcc->hcc_transer_info.hcc_transfer_thread->cpus_allowed))[0]);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc cpu fast_cpus:0x%lx", cpumask_bits(&fast_cpus)[0]);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc cpu slow_cpus:0x%lx", cpumask_bits(&slow_cpus)[0]);

    return OAL_SUCC;
}
#endif

oal_int32 hcc_bus_bindcpu(hcc_bus *hi_bus, oal_uint32 chan, oal_int32 is_bind)
{
    struct hcc_handler *hcc = NULL;
    if (oal_warn_on(hi_bus == NULL)) {
        return -OAL_ENODEV;
    }

    if (oal_unlikely(hi_bus->opt_ops->bindcpu == NULL)) {
        return -OAL_EIO;
    }

    hcc = hbus_to_hcc(hi_bus);

    if (hi_bus->is_bind == is_bind) {
        return OAL_SUCC;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc %s cpu", is_bind ? "bind" : "unbind");

    hi_bus->is_bind = is_bind;

    if (oal_unlikely(hcc != NULL)) {
        if (hcc->hcc_transer_info.hcc_transfer_thread) {
#if defined(CONFIG_ARCH_HISI)
            struct cpumask slow_cpus, fast_cpus;
#ifdef CONFIG_ARCH_PLATFORM
            get_slow_cpus(&slow_cpus);
            get_fast_cpus(&fast_cpus);
#else
            hisi_get_slow_cpus(&slow_cpus);
            hisi_get_fast_cpus(&fast_cpus);
#endif
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "hcc thread hisi rt opt cpu");
            if (is_bind) {
                set_cpus_allowed_ptr(hcc->hcc_transer_info.hcc_transfer_thread, &fast_cpus);
            } else {
                set_cpus_allowed_ptr(hcc->hcc_transer_info.hcc_transfer_thread, &slow_cpus);
            }
#endif
        }
    }

    if (hi_bus->pst_rx_tsk != NULL) {
#if defined(CONFIG_ARCH_HISI)
        struct cpumask slow_cpus, fast_cpus;
#ifdef CONFIG_ARCH_PLATFORM
        get_slow_cpus(&slow_cpus);
        get_fast_cpus(&fast_cpus);
#else
        hisi_get_slow_cpus(&slow_cpus);
        hisi_get_fast_cpus(&fast_cpus);
#endif
        oal_print_hi11xx_log(HI11XX_LOG_DBG, "bus rx task hisi rt opt cpu");
        if (is_bind) {
            set_cpus_allowed_ptr(hi_bus->pst_rx_tsk, &fast_cpus);
        } else {
            set_cpus_allowed_ptr(hi_bus->pst_rx_tsk, &slow_cpus);
        }
#endif
    }

    return hi_bus->opt_ops->bindcpu(hi_bus, chan, is_bind);
}

EXPORT_SYMBOL_GPL(hcc_bus_bindcpu);

/* PM */
oal_int32 hcc_bus_pm_wakeup_device(hcc_bus *hi_bus)
{
    oal_uint32 uret;

    if (oal_warn_on(hi_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_bus is null");
        return -OAL_EIO;
    }

    if (oal_unlikely(hi_bus->pst_pm_callback == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_pm_callback is null");
        return -OAL_EIO;
    }

    if (oal_unlikely(hi_bus->pst_pm_callback->pm_wakeup_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pm_wakeup_dev is null");
        return -OAL_EIO;
    }

    uret = hi_bus->pst_pm_callback->pm_wakeup_dev();
    if (oal_unlikely(uret != OAL_SUCC)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pm_wakeup_dev failed ret=%u, bus_type=%u", uret, hi_bus->bus_type);
        wlan_pm_dump_host_info();
        return -OAL_EBUSY;
    }

    return OAL_SUCC;
}
EXPORT_SYMBOL_GPL(hcc_bus_pm_wakeup_device);

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
unsigned long long hcc_bus_excetpion_ssi_module_set(oal_void)
{
    unsigned long long set = 0x0;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "bfgx is %s  %s",
                         (bfgx_is_shutdown() == true) ? "shutdown" : "poweron",
#if defined(CONFIG_LOG_EXCEPTION) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
                         (get_logusertype_flag() == BETA_USER) ? ", is beta user" : ", not beta user"
#else
                         ""
#endif
                         );

    if ((bfgx_is_shutdown() == true)
#if defined(CONFIG_LOG_EXCEPTION) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT)
        || (get_logusertype_flag() == BETA_USER)
#endif
    ) {
        set = (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON | SSI_MODULE_MASK_SDIO);
    } else {
        set = 0x0;
    }

    return set;
}
oal_void hcc_bus_exception_submit(hcc_bus *hi_bus, oal_int32 excep_type)
{
    oal_ulong flags;
    oal_workqueue_stru *pst_excp_workqueue = NULL;
    declare_dft_trace_key_info("wifi_trans_exception", OAL_DFT_TRACE_EXCEP);
    if (oal_unlikely(hi_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]%s, hi_bus is null", __FUNCTION__);
        return;
    }

    pst_excp_workqueue = wifi_get_exception_workqueue();
    if (oal_unlikely(pst_excp_workqueue == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]%s, pst_excp_workqueue is null", __FUNCTION__);
        return;
    }

    /* disable bus */
    hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_RX);

    oal_spin_lock_irq_save(&hi_bus->bus_excp_lock, &flags);

    if (oal_work_is_busy(&hi_bus->bus_excp_worker)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "excep %d block, exception %d is working",
                             excep_type, hi_bus->bus_excp_type);
        oal_spin_unlock_irq_restore(&hi_bus->bus_excp_lock, &flags);
        return;
    }

    hcc_change_state_exception();
    hi_bus->bus_excp_type = excep_type;

    if ((excep_type != WIFI_DEVICE_PANIC) && (excep_type != RF_CALI_FAIL)) {
#if defined(_PRE_CONFIG_GPIO_TO_SSI_DEBUG)
        unsigned long long set = 0x0;
        oal_spin_unlock_irq_restore(&hi_bus->bus_excp_lock, &flags);
        if (hi11xx_get_os_build_variant() == HI1XX_OS_BUILD_VARIANT_USER) {
            set = hcc_bus_excetpion_ssi_module_set();

            if (!oal_print_rate_limit(24 * PRINT_RATE_HOUR)) { /* 24小时打印一次 */
                set = 0x0;
            }
        } else {
            if (hi_bus->bus_type == HCC_BUS_SDIO) {
                set  = SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON | SSI_MODULE_MASK_SDIO;
                set |= SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL;
            }

            if (hi_bus->bus_type == HCC_BUS_PCIE) {
                set = SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG;
#ifdef PLATFORM_SSI_FULL_LOG
                set |= SSI_MODULE_MASK_PCIE_DBI;
#endif
            }
        }

        ssi_dump_device_regs(set);
        oal_spin_lock_irq_save(&hi_bus->bus_excp_lock, &flags);
#endif
    }

    oal_dft_print_error_key_info();

    if (g_hcc_exception_enable) {
        queue_work(pst_excp_workqueue, &hi_bus->bus_excp_worker);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "wifi exception memdump function disabled");
    }
    oal_spin_unlock_irq_restore(&hi_bus->bus_excp_lock, &flags);
}
/*lint -e19*/
oal_module_symbol(hcc_bus_exception_submit);
/*lint +e19*/
oal_void hcc_bus_rx_irq_memalloc_work_submit(hcc_bus *hi_bus)
{
    if (work_busy(&hi_bus->st_bus_irq_memalloc_work)) {
        return;
    }
    oal_spin_lock(&hi_bus->st_mealloc_lock);
    /* 挂载在系统的wokrqueue上 */
    schedule_work(&hi_bus->st_bus_irq_memalloc_work);
    oal_spin_unlock(&hi_bus->st_mealloc_lock);
}
/*lint -e19*/
oal_module_symbol(hcc_bus_rx_irq_memalloc_work_submit);
/*lint +e19*/
oal_int32 hcc_bus_exception_is_busy(hcc_bus *hi_bus)
{
    if (oal_unlikely(hi_bus == NULL)) {
        return OAL_FALSE;
    }

    if (oal_work_is_busy(&hi_bus->bus_excp_worker)) {
        /* mem dump is processing,
          can't power off or submit repeat */
        return OAL_TRUE;
    }

    return OAL_FALSE;
}
/*lint -e19*/
oal_module_symbol(hcc_bus_exception_is_busy);
/*lint +e19*/
oal_int32 oal_trigger_bus_exception(hcc_bus *hi_bus, oal_int32 is_sync)
{
    oal_ulong timeout_jiffies;
    if (oal_unlikely(hi_bus == NULL)) {
        return OAL_FALSE;
    }

    if (hcc_bus_exception_is_busy(hi_bus) == OAL_TRUE) {
        return OAL_TRUE;
    }

    /* trigger device panic */
    if (hcc_bus_send_message(hi_bus, H2D_MSG_TEST)) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "send device panic message failed!");
        return OAL_FALSE;
    }

    if (is_sync != OAL_TRUE) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "wifi exception is doing...");
        return OAL_TRUE;
    }

    /* wait device panic */
    timeout_jiffies = jiffies + oal_msecs_to_jiffies(2 * MSEC_PER_SEC); /* 2 seconds timeout */
    for (;;) {
        if (hcc_bus_exception_is_busy(hi_bus) == OAL_TRUE) {
            break;
        }

        if (time_after(jiffies, timeout_jiffies)) {
            oal_print_hi11xx_log(HI11XX_LOG_WARN, "wait panic message timeout!");
            return OAL_FALSE;
        }

        oal_msleep(oal_jiffies_to_msecs(1));
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "trigger wifi exception manually sucuess");
    return OAL_TRUE;
}
/*lint -e19*/
oal_module_symbol(oal_trigger_bus_exception);
/*lint +e19*/
oal_void hcc_bus_exception_submit_test(oal_void)
{
    hcc_bus_exception_submit(hcc_get_current_110x_bus(), WIFI_TRANS_FAIL);
}

oal_int32 hcc_bus_device_panic_callback(void *data)
{
    hcc_bus *hi_bus = (hcc_bus *)data;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();

    hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_ALL);
    wlan_pm_stop_wdg(pst_wlan_pm);
    hcc_bus_exception_submit(hi_bus, WIFI_DEVICE_PANIC);

    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_DEV, CHR_PLAT_DRV_ERROR_WIFI_DEV_PANIC);

    return OAL_SUCC;
}

oal_int32 hcc_bus_device_init_ready(void *data)
{
    hcc_bus *hi_bus = (hcc_bus *)data;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device ready, bus:%s", hcc_bus_get_bus_type_str(hi_bus->bus_type));
    oal_complete(&hi_bus->st_device_ready);
    return OAL_SUCC;
}

static int32 wait_wlan_wkup_gpio_level(int32 gpio_level, unsigned long timeout)
{
    int32 gpio_value;
    gpio_level = (gpio_level == GPIO_LOWLEVEL) ? GPIO_LOWLEVEL : GPIO_HIGHLEVEL;
    for (;;) {
        gpio_value = board_get_wlan_wkup_gpio_val();
        gpio_value = (gpio_value == GPIO_LOWLEVEL) ? GPIO_LOWLEVEL : GPIO_HIGHLEVEL;
        if (gpio_value == gpio_level) {
            return SUCCESS;
        }

        if (time_after(jiffies, timeout)) {
            return -FAILURE;
        } else {
            if (wlan_is_shutdown()) {
                OAL_IO_PRINT("[E]hold_time_check:wifi is shutdown!\n");
                return (gpio_level == GPIO_LOWLEVEL) ? SUCCESS : -FAILURE;
            }
            oal_usleep_range(10, 20);
            cpu_relax();
        }
    }
}

/* 检查DEVICE WAKEUP HOST gpio 是否拉高。 */
oal_int32 oal_dev2host_gpio_hold_time_check(oal_uint32 switch_timeout, oal_uint32 hold_time)
{
    oal_ulong timeout;
    declare_time_cost_stru(cost);
    const oal_uint32 ul_default_timeout = 200;

    if (!switch_timeout) {
        switch_timeout = ul_default_timeout;
    }

    timeout = jiffies + oal_msecs_to_jiffies(switch_timeout);
    oal_get_time_cost_start(cost);

    // wait gpio high
    if (wait_wlan_wkup_gpio_level(GPIO_HIGHLEVEL, timeout) != SUCCESS) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]wait wakeup gpio to high timeout [%u] ms", switch_timeout);
        return OAL_FALSE;
    }
    oal_get_time_cost_end(cost);
    oal_calc_time_cost_sub(cost);
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "device reset  ip cost %llu us", time_cost_var_sub(cost));

    /* high level hold time */
    timeout = jiffies + oal_msecs_to_jiffies(hold_time);
    /* wait high level hold time */
    if (wait_wlan_wkup_gpio_level(GPIO_LOWLEVEL, timeout) == SUCCESS) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "[E]gpio pull down again!");
        return OAL_FALSE;
    }

    /* gpio high and hold enough time. */
    return OAL_TRUE;
}

/* check whether device had panic */
oal_int32 hcc_bus_panic_status_check(hcc_bus *hi_bus)
{
    oal_int32 ret = OAL_FALSE;

    if (hi_bus->bus_excp_type == WIFI_DEVICE_PANIC) {
        ret = OAL_TRUE;
    }

    /* gpio mode,两参数均代表设置的超时 */
    if (oal_dev2host_gpio_hold_time_check(5000, 100) == OAL_TRUE) {
        ret = OAL_TRUE;
    } else {
        ret = OAL_FALSE;
    }
    return ret;
}

/* when recv error data from device, dump device mem */
#define WCPU_PANIC_MEMDUMP_MAX_SIZE 0x3c00  /* 15K, 0x1004_0000--0x1005_FFFF , 128K pktmem */
#define WCPU_PANIC_MEMDUMP_MAX_ADDR 0x10060000
/* 0x1005C400 */
#define WCPU_PANIC_MEMDUMP_STORE_ADDR (WCPU_PANIC_MEMDUMP_MAX_ADDR - WCPU_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_TCM_12RAM_MAX_SIZE     0x80000 /* 512KB */
#define WCPU_TCM_12RAM_MAX_ADDR     0xF0000
/* 0x70000 */
#define WPCU_TCM_12RAM_STORE_ADDR   (WCPU_TCM_12RAM_MAX_ADDR - WCPU_TCM_12RAM_MAX_SIZE)

#define WCPU_PKT_MEM_MAX_SIZE       0x59800 /* 358KB */
#define WCPU_PKT_MEM_MAX_ADDR       0x10079800
/* 0x10020000 */
#define WCPU_PKT_MEM_STORE_ADDR     (WCPU_PKT_MEM_MAX_ADDR - WCPU_PKT_MEM_MAX_SIZE)
struct st_wifi_dump_mem_info g_hi1102_meminfo[] = {
    {   .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_PANIC_MEMDUMP_MAX_SIZE
    },
    {
        .file_name = "wifi_device_tcm_l2ram",
        .mem_addr = WPCU_TCM_12RAM_STORE_ADDR,
        .size = WCPU_TCM_12RAM_MAX_SIZE, /* 512KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_PKT_MEM_STORE_ADDR,
        .size = WCPU_PKT_MEM_MAX_SIZE, /* 358KB */
    },

};

/* for 1103 */
/* mpw2 config */
#define WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_SIZE   0x3c00  /* 15KB */
#define WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_ADDR   0x60080000
/* 0x6007c400 */
#define WCPU_1103_MPW2_PANIC_MEMDUMP_STORE_ADDR \
        (WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_ADDR - WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1103_MPW2_ITCM_RAM_MAX_SIZE        0x88000 /* 544KB */
#define WCPU_1103_MPW2_ITCM_RAM_MAX_ADDR        0x8c000
/* 0x4000 */
#define WCPU_1103_MPW2_ITCM_RAM_STORE_ADDR      (WCPU_1103_MPW2_ITCM_RAM_MAX_ADDR - WCPU_1103_MPW2_ITCM_RAM_MAX_SIZE)

#define WCPU_1103_MPW2_DTCM_RAM_MAX_SIZE        0x68000 /* 416KB */
#define WCPU_1103_MPW2_DTCM_RAM_MAX_ADDR        0x20068000
/* 0x20000000 */
#define WCPU_1103_MPW2_DTCM_RAM_STORE_ADDR      (WCPU_1103_MPW2_DTCM_RAM_MAX_ADDR - WCPU_1103_MPW2_DTCM_RAM_MAX_SIZE)

#define WCPU_1103_MPW2_PKT_MEM_MAX_SIZE         0x90000 /* 576KB */
#define WCPU_1103_MPW2_PKT_MEM_MAX_ADDR         0x60090000
/* 0x60000000 */
#define WCPU_1103_MPW2_PKT_MEM_STORE_ADDR       (WCPU_1103_MPW2_PKT_MEM_MAX_ADDR - WCPU_1103_MPW2_PKT_MEM_MAX_SIZE)
struct st_wifi_dump_mem_info g_hi1103_mpw2_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1103_MPW2_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1103_MPW2_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1103_MPW2_ITCM_RAM_STORE_ADDR,
        .size = WCPU_1103_MPW2_ITCM_RAM_MAX_SIZE, /* 544KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1103_MPW2_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1103_MPW2_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1103_MPW2_PKT_MEM_STORE_ADDR,
        .size = WCPU_1103_MPW2_PKT_MEM_MAX_SIZE, /* 576KB */
    },

};

/* pilot config */
#define WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_SIZE   0x3c00  /* 15KB */
#define WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_ADDR   0x60080000
/* 0x6007c400 */
#define WCPU_1103_PILOT_PANIC_MEMDUMP_STORE_ADDR \
        (WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_ADDR - WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1103_PILOT_TICM_RAM_MAX_SIZE        0x98000 /* 608KB */
#define WCPU_1103_PILOT_TICM_RAM_MAX_ADDR        0xa8000
/* 0x10000 */
#define WCPU_1103_PILOT_TICM_RAM_STORE_ADDR      (WCPU_1103_PILOT_TICM_RAM_MAX_ADDR - WCPU_1103_PILOT_TICM_RAM_MAX_SIZE)

#define WCPU_1103_PILOT_DTCM_RAM_MAX_SIZE        0x68000 /* 416KB */
#define WCPU_1103_PILOT_DTCM_RAM_MAX_ADDR        0x20080000
/* 0x20018000 */
#define WCPU_1103_PILOT_DTCM_RAM_STORE_ADDR      (WCPU_1103_PILOT_DTCM_RAM_MAX_ADDR - WCPU_1103_PILOT_DTCM_RAM_MAX_SIZE)

#define WCPU_1103_PILOT_PKT_MEM_MAX_SIZE         0x80000 /* 512KB */
#define WCPU_1103_PILOT_PKT_MEM_MAX_ADDR         0x60080000
/* 0x60000000 */
#define WCPU_1103_PILOT_PKT_MEM_STORE_ADDR       (WCPU_1103_PILOT_PKT_MEM_MAX_ADDR - WCPU_1103_PILOT_PKT_MEM_MAX_SIZE)
struct st_wifi_dump_mem_info g_hi1103_pilot_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1103_PILOT_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1103_PILOT_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1103_PILOT_TICM_RAM_STORE_ADDR,
        .size = WCPU_1103_PILOT_TICM_RAM_MAX_SIZE, /* 608KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1103_PILOT_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1103_PILOT_DTCM_RAM_MAX_SIZE, /* 416KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1103_PILOT_PKT_MEM_STORE_ADDR,
        .size = WCPU_1103_PILOT_PKT_MEM_MAX_SIZE, /* 512KB */
    },

};

/* 1102a pilot config */
#define WCPU_1102A_PILOT_PANIC_MEMDUMP_MAX_SIZE   0x3c00  /* 15KB */
#define WCPU_1102A_PILOT_PANIC_MEMDUMP_MAX_ADDR   0x60048000
/* 0x60044400 */
#define WCPU_1102A_PILOT_PANIC_MEMDUMP_STORE_ADDR \
        (WCPU_1102A_PILOT_PANIC_MEMDUMP_MAX_ADDR - WCPU_1102A_PILOT_PANIC_MEMDUMP_MAX_SIZE)

#define WCPU_1102A_PILOT_CODE_RAM_MAX_SIZE        0x94000 /* 592KB */
#define WCPU_1102A_PILOT_CODE_RAM_MAX_ADDR        0xa8000
/* 0x14000 */
#define WCPU_1102A_PILOT_CODE_RAM_STORE_ADDR      \
        (WCPU_1102A_PILOT_CODE_RAM_MAX_ADDR - WCPU_1102A_PILOT_CODE_RAM_MAX_SIZE)

#define WCPU_1102A_PILOT_DTCM_RAM_MAX_SIZE        0xc000  /* 48KB */
#define WCPU_1102A_PILOT_DTCM_RAM_MAX_ADDR        0x2000c000
/* 0x20000000 */
#define WCPU_1102A_PILOT_DTCM_RAM_STORE_ADDR      \
        (WCPU_1102A_PILOT_DTCM_RAM_MAX_ADDR - WCPU_1102A_PILOT_DTCM_RAM_MAX_SIZE)

#define WCPU_1102A_PILOT_PKT_MEM_MAX_SIZE         0x48000 /* 288KB */
#define WCPU_1102A_PILOT_PKT_MEM_MAX_ADDR         0x60048000
/* 0x60000000 */
#define WCPU_1102A_PILOT_PKT_MEM_STORE_ADDR       \
        (WCPU_1102A_PILOT_PKT_MEM_MAX_ADDR - WCPU_1102A_PILOT_PKT_MEM_MAX_SIZE)

#define WCPU_1102A_PILOT_ITCM_RAM_MAX_SIZE        0x3ffc  /* 16KB */
#define WCPU_1102A_PILOT_ITCM_RAM_MAX_ADDR        0x4000
/* 0x4 */
#define WCPU_1102A_PILOT_ITCM_RAM_STORE_ADDR      \
        (WCPU_1102A_PILOT_ITCM_RAM_MAX_ADDR - WCPU_1102A_PILOT_ITCM_RAM_MAX_SIZE)
OAL_STATIC struct st_wifi_dump_mem_info g_hi1102a_pilot_meminfo[] = {
    {
        .file_name = "wifi_device_panic_mem",
        .mem_addr = WCPU_1102A_PILOT_PANIC_MEMDUMP_STORE_ADDR,
        .size = WCPU_1102A_PILOT_PANIC_MEMDUMP_MAX_SIZE, /* 15KB */
    },
    {
        .file_name = "wifi_device_code_ram",
        .mem_addr = WCPU_1102A_PILOT_CODE_RAM_STORE_ADDR,
        .size = WCPU_1102A_PILOT_CODE_RAM_MAX_SIZE, /* 592KB */
    },
    {
        .file_name = "wifi_device_dtcm_ram",
        .mem_addr = WCPU_1102A_PILOT_DTCM_RAM_STORE_ADDR,
        .size = WCPU_1102A_PILOT_DTCM_RAM_MAX_SIZE, /* 48KB */
    },
    {
        .file_name = "wifi_device_pkt_mem",
        .mem_addr = WCPU_1102A_PILOT_PKT_MEM_STORE_ADDR,
        .size = WCPU_1102A_PILOT_PKT_MEM_MAX_SIZE, /* 288KB */
    },
    {
        .file_name = "wifi_device_itcm_ram",
        .mem_addr = WCPU_1102A_PILOT_ITCM_RAM_STORE_ADDR,
        .size = WCPU_1102A_PILOT_ITCM_RAM_MAX_SIZE, /* 16KB */
    },
};

void hcc_bus_exception_handler(oal_work_stru *work)
{
    hcc_bus *hi_bus = hcc_get_current_110x_bus();
    struct st_wifi_dump_mem_info *mem_info = NULL;
    oal_uint32 size;
    oal_int32 excp_type;

    if (hi_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_bus is null");
        return;
    }

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST)
    mem_info = g_hi1102_meminfo;
    size = sizeof(g_hi1102_meminfo) / sizeof(struct st_wifi_dump_mem_info);
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST)
    if (get_hi1103_asic_type() == HI1103_ASIC_PILOT) {
        mem_info = g_hi1103_pilot_meminfo;
        size = sizeof(g_hi1103_pilot_meminfo) / sizeof(struct st_wifi_dump_mem_info);
    } else {
        mem_info = g_hi1103_mpw2_meminfo;
        size = sizeof(g_hi1103_mpw2_meminfo) / sizeof(struct st_wifi_dump_mem_info);
    }
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102A_HOST)
    mem_info = g_hi1102a_pilot_meminfo;
    size = sizeof(g_hi1102a_pilot_meminfo) / sizeof(struct st_wifi_dump_mem_info);
#else
#error unadapt choose
#endif

    hi_bus->mem_info = mem_info;
    hi_bus->mem_size = size;

    hcc_bus_wake_lock(hi_bus);
    if (hi_bus->bus_excp_type == WIFI_DEVICE_PANIC) {
        declare_dft_trace_key_info("wifi_device_panic", OAL_DFT_TRACE_EXCEP);
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "[E]WiFi Device Panic");
    }

    if (hi_bus->bus_excp_type == WIFI_TRANS_FAIL) {
        declare_dft_trace_key_info("wifi_trans_fail", OAL_DFT_TRACE_EXCEP);
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "[E]wifi_trans_fail");
    }

    hcc_bus_disable_state(hi_bus, OAL_BUS_STATE_ALL);

    oal_dft_print_all_key_info();

#ifdef CONFIG_PRINTK
    hwifi_panic_log_dump(KERN_INFO);
#endif

    if (hi_bus->pst_pm_callback->pm_disable) {
        hi_bus->pst_pm_callback->pm_disable(OAL_FALSE);
    }

    excp_type = hi_bus->bus_excp_type;
    if (hcc_bus_panic_status_check(hi_bus) == OAL_TRUE) {
        /* device panic */
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "[E]check wakeup gpio ok, wcpu had panic");
        if (hi_bus->bus_excp_type != WIFI_DEVICE_PANIC) {
            OAM_ERROR_LOG1(0, OAM_SF_ANY, "[E]change sdio excp type from %d to device panic ", hi_bus->bus_excp_type);
        }
        hi_bus->bus_excp_type = WIFI_DEVICE_PANIC;
        oal_msleep(10);
    } else {
        /* device is't panic, reset sdio ip from host */
        OAM_ERROR_LOG0(0, OAM_SF_ANY, "[E]check wakeup gpio fail, wcpu crash");
        hi_bus->bus_excp_type = WIFI_TRANS_FAIL;
    }

    if (wlan_is_shutdown()) {
        hcc_bus_wake_unlock(hi_bus);
        OAL_IO_PRINT("[E]dfr ignored, wifi shutdown, wakelock cnt %lu\n", hi_bus->st_bus_wakelock.lock_count);
        return;
    }
    /* close sdio data transfer when device panic. */
    wifi_exception_work_submit(excp_type);
    hcc_bus_wake_unlock(hi_bus);
}

#undef COFNIG_TEST_SDIO_INT_LOSS
/*
 * Prototype    : hcc_bus_rxdata_thread
 * Description  : gpio rx data function
 * Input        : adapter   oal_sdio handler
 * Return Value : succ or fail
 */
OAL_STATIC oal_int32 hcc_bus_rxdata_thread(oal_void *data)
{
#ifdef COFNIG_TEST_SDIO_INT_LOSS
    int i = 0;
    const int l_loop_times = 5;
#endif
    hcc_bus *hi_bus;

    struct sched_param param;

    hi_bus = (hcc_bus *)data;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hisi wifi sched ctrl rx thread high speed");
    param.sched_priority = OAL_BUS_RXDATA_THREAD_PRIORITY;
    oal_set_thread_property(current,
                            OAL_BUS_RX_THREAD_POLICY,
                            &param,
                            MIN_NICE);

    allow_signal(SIGTERM);
    while (oal_likely(!down_interruptible(&hi_bus->rx_sema))) {
        if (oal_unlikely(kthread_should_stop())) {
            oal_print_hi11xx_log(HI11XX_LOG_INFO, "exit gpio rx thread");
            break;
        }
#ifdef COFNIG_TEST_SDIO_INT_LOSS
        for (i = 0; i < l_loop_times; i++) {
#endif
            hcc_bus_rx_transfer_lock(hi_bus);
            if (hi_bus->opt_ops->wlan_gpio_rxdata_proc) {
                hi_bus->opt_ops->wlan_gpio_rxdata_proc(hi_bus);
            }
            if (oal_netbuf_list_len(&hi_bus->rx_netbuf_head) <= (HCC_BUS_MEMALLOC_MAX >> 1)) {
                hcc_bus_rx_irq_memalloc_work_submit(hi_bus);
            }
            hcc_bus_rx_transfer_unlock(hi_bus);
#ifdef COFNIG_TEST_SDIO_INT_LOSS
        }
#endif
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "gpio rx data is terminated");
    return OAL_SUCC;
}

oal_uint64 oal_get_gpio_int_count_para(oal_void)
{
    hcc_bus *hi_bus;

    hi_bus = hcc_get_current_110x_bus();
    if (hi_bus == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "hi_bus is null");
        return 0;
    }

    return hi_bus->gpio_int_count;
}

OAL_STATIC oal_void hcc_bus_sched_gpio_task(hcc_bus *pst_bus, oal_int32 irq)
{
    if (oal_likely(pst_bus->opt_ops->wlan_gpio_handler)) {
        pst_bus->opt_ops->wlan_gpio_handler(pst_bus, irq);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wlan_gpio_handler is NULL, irq:%d, bus type:%s, disable gpio interrupt!",
                             irq, hcc_bus_get_bus_type_str(pst_bus->bus_type));
    }
}

/*
 * Prototype    : hcc_bus_wlan_gpio_irq
 * Description  : wlan irq process function
 * Return Value : OAL_STATIC irqreturn_t
 */
OAL_STATIC irqreturn_t hcc_bus_wlan_gpio_irq(oal_int32 irq, oal_void *dev_id)
{
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *pst_bus_dev;

    pst_bus_dev = (hcc_bus_dev *)dev_id;
    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev id  is NULL, irq:%d", irq);
        return IRQ_HANDLED;
    }

    pst_bus = hdev_to_hbus(pst_bus_dev);
    if (oal_unlikely(pst_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "curr bus is NULL, irq:%d, bus_dev id:%u", irq, pst_bus_dev->dev_id);
        return IRQ_HANDLED;
    }

    if (oal_likely(pst_bus->opt_ops->wlan_gpio_handler)) {
        hcc_bus_sched_gpio_task(pst_bus, irq);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_ERR,
                             "wlan_gpio_handler is NULL, irq:%d, bus_dev id:%u, disable gpio interrupt!",
                             irq, pst_bus_dev->dev_id);
        oal_wlan_gpio_intr_enable(pst_bus_dev, OAL_FALSE);
    }

    return IRQ_HANDLED;
}

/*
 * Prototype    : oal_register_gpio_intr
 * Description  : initialize sdio pm interface
 */
oal_int32 oal_register_gpio_intr(hcc_bus_dev *pst_bus_dev)
{
    oal_int32 ret;
    board_info *pst_board = get_hi110x_board_info();

    unsigned int wlan_irq = pst_board->wlan_irq;

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "wlan gpio irq = %u", wlan_irq);
    if (!wlan_irq) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[BUS][ERROR]wlan irq should't be 0");
        return -OAL_EFAIL;
    }

    pst_bus_dev->ul_wlan_irq = pst_board->wlan_irq;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    ret = request_irq(wlan_irq, hcc_bus_wlan_gpio_irq, IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_DISABLED,
                      "wifi_gpio_intr", pst_bus_dev);
#else
    /* 低版本内核没有IRQF_NO_SUSPEND标记 */
    ret = request_irq(wlan_irq, hcc_bus_wlan_gpio_irq, IRQF_TRIGGER_RISING | IRQF_DISABLED,
                      "wifi_gpio_intr", pst_bus_dev);
#endif
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to request wlan irq");

        return ret;
    }

    pst_bus_dev->ul_irq_stat = 0; /* irq enabled default. */

    oal_wlan_gpio_intr_enable(pst_bus_dev, OAL_FALSE);

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "success to register sdio gpio intr");
    return OAL_SUCC;
}

/*
 * Prototype    : oal_unregister_gpio_intr
 * Description  : initialize sdio pm interface
 */
oal_void oal_unregister_gpio_intr(hcc_bus_dev *pst_bus_dev)
{
    /* disable wlan irq */
    oal_wlan_gpio_intr_enable(pst_bus_dev, OAL_FALSE);

    /* free irq when sdio driver deinit */
    free_irq(pst_bus_dev->ul_wlan_irq, pst_bus_dev);
}

/*
 * 函 数 名  : oal_flowctrl_gpio_intr_enable
 * 功能描述  : 使能/关闭GPIO流控中断
 * 输入参数  : 1: enabled; 0: disabled
 * 返 回 值  : 成功或失败原因
 */
OAL_STATIC oal_void oal_flowctrl_gpio_intr_enable(hcc_bus_dev *pst_bus_dev, oal_bool_enum_uint8 en_enable)
{
    if (en_enable) {
        /* 使能GPIO流控中断 */
        enable_irq(pst_bus_dev->l_flowctrl_irq);
    } else {
        /* 关闭GPIO流控中断 */
        disable_irq_nosync(pst_bus_dev->l_flowctrl_irq);
    }
}

/*
 * 函 数 名  : hcc_bus_flowctrl_gpio_irq
 * 功能描述  : GPIO流控中断回调函数
 * 返 回 值  : 成功或失败原因
 */
OAL_STATIC irqreturn_t hcc_bus_flowctrl_gpio_irq(oal_int32 l_irq, oal_void *p_dev_id)
{
    hcc_bus *pst_bus = NULL;
    hcc_bus_dev *pst_bus_dev;

    pst_bus_dev = (hcc_bus_dev *)p_dev_id;
    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "dev id  is NULL, irq:%d", l_irq);
        return IRQ_HANDLED;
    }

    pst_bus = hdev_to_hbus(pst_bus_dev);
    if (oal_unlikely(pst_bus == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "curr bus is NULL, irq:%d, bus_dev id:%u", l_irq, pst_bus_dev->dev_id);
        return IRQ_HANDLED;
    }

    if (oal_likely(pst_bus->opt_ops->flowctrl_gpio_handler)) {
        /* 执行GPIO流控逻辑(oal_sdio_gpio_flowctrl_irq) */
        pst_bus->opt_ops->flowctrl_gpio_handler(pst_bus, l_irq);
    } else {
        /* flowctrl_gpio_handler不存在, 关闭GPIO流控功能 */
        oal_print_hi11xx_log(HI11XX_LOG_ERR,
                             "flowctrl_gpio_handler is NULL, irq:%d, bus_dev id:%u, disable gpio interrupt!",
                             l_irq, pst_bus_dev->dev_id);
        oal_flowctrl_gpio_intr_enable(pst_bus_dev, OAL_FALSE);
    }

    return IRQ_HANDLED;
}

/*
 * 函 数 名  : oal_register_gpio_flowctrl_intr
 * 功能描述  : 注册GPIO流控中断
 * 返 回 值  : 成功或失败原因
 */
oal_int32 oal_register_gpio_flowctrl_intr(hcc_bus_dev *pst_bus_dev)
{
    oal_int32 ret;
    board_info *pst_board = get_hi110x_board_info();
    oal_uint32 flowctrl_irq = pst_board->flowctrl_irq;

    if (!flowctrl_irq) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[BUS][ERROR]flowctrl irq should't be 0");
        return OAL_FAIL;
    }

    pst_bus_dev->l_flowctrl_irq = pst_board->flowctrl_irq;

    /* 调用内核接口注册GPIO流控中断, 中断触发时回调hcc_bus_flowctrl_gpio_irq */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    ret = request_irq(flowctrl_irq, hcc_bus_flowctrl_gpio_irq,
                      IRQF_NO_SUSPEND | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED,
                      "wifi_gpio_flowctrl_intr", pst_bus_dev);
#else
    /* 低版本内核没有IRQF_NO_SUSPEND标记 */
    ret = request_irq(flowctrl_irq, hcc_bus_flowctrl_gpio_irq,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_DISABLED,
                      "wifi_gpio_flowctrl_intr", pst_bus_dev);
#endif
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to request flowctrl irq, ret code = %d", ret);
        return OAL_FAIL;
    }

    /* 初始化GPIO流控中断为使能状态 */
    oal_flowctrl_gpio_intr_enable(pst_bus_dev, OAL_FALSE);
    oal_flowctrl_gpio_intr_enable(pst_bus_dev, OAL_TRUE);

    pst_bus_dev->en_flowctrl_gpio_registered = OAL_TRUE;

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_unregister_gpio_flowctrl_intr
 * 功能描述  : 注销GPIO流控中断
 * 返 回 值  : 成功或失败原因
 */
oal_void oal_unregister_gpio_flowctrl_intr(hcc_bus_dev *pst_bus_dev)
{
    if (pst_bus_dev->en_flowctrl_gpio_registered) {
        /* 关闭GPIO流控中断 */
        oal_flowctrl_gpio_intr_enable(pst_bus_dev, OAL_FALSE);

        /* 注销GPIO流控中断 */
        free_irq(pst_bus_dev->l_flowctrl_irq, pst_bus_dev);

        pst_bus_dev->en_flowctrl_gpio_registered = OAL_FALSE;
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "gpio flowctrl irq wasn't registered on dev[%d]", pst_bus_dev->dev_id);
    }
}

/*
 * 函 数 名  : hcc_bus_flowctrl_init
 * 功能描述  : 流控初始化接口, 流控类型来自私有定制化文件
 * 返 回 值  : 成功或失败原因
 */
oal_uint32 hcc_bus_flowctrl_init(oal_uint8 uc_hcc_flowctrl_type)
{
    hcc_bus_dev *pst_bus_dev = hcc_get_bus_dev(0);
    if (pst_bus_dev == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "flowctrl init get 110x dev is null");
        return OAL_FAIL;
    }

    /* 流控类型: 0.SDIO 1.GPIO */
    if (uc_hcc_flowctrl_type == 0) {
        oal_print_hi11xx_log(HI11XX_LOG_WARN, "flowctrl type = SDIO, already init");
        return OAL_SUCC;
    }

    /* hcc已注册GPIO中断 */
    if (pst_bus_dev->en_flowctrl_gpio_registered) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "flowctrl type = GPIO, already init");
        return OAL_SUCC;
    }

    /* Step1: 申请管脚 */
    if (board_flowctrl_gpio_init() != BOARD_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "request flowctrl gpio failed!");
        return OAL_FAIL;
    }

    /* Step2: 获取管脚对应中断号 */
    board_flowctrl_irq_init();

    /* Step3: 使用中断号注册GPIO中断 */
    if (oal_register_gpio_flowctrl_intr(pst_bus_dev) != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "request flowctrl gpio irq failed!");
        free_board_flowctrl_gpio();
        return OAL_FAIL;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_wlan_gpio_intr_enable
 * 功能描述  : 使能/关闭 WLAN GPIO 中断
 * 输入参数  : 1:enable; 0:disenable
 * 返 回 值  : 成功或失败原因
 */
oal_void oal_wlan_gpio_intr_enable(hcc_bus_dev *pst_bus_dev, oal_uint32 ul_en)
{
    oal_uint flags;

    if (oal_unlikely(pst_bus_dev == NULL)) {
        oal_warn_on(1);
        return;
    }

    oal_spin_lock_irq_save(&pst_bus_dev->st_irq_lock, &flags);
    if (ul_en) {
        /* 不再支持中断开关嵌套 */
        if (pst_bus_dev->ul_irq_stat) {
            enable_irq(pst_bus_dev->ul_wlan_irq);
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "gpio irq: %d enable", pst_bus_dev->ul_wlan_irq);
            pst_bus_dev->ul_irq_stat = 0;
        }
    } else {
        if (!pst_bus_dev->ul_irq_stat) {
            disable_irq_nosync(pst_bus_dev->ul_wlan_irq);
            oal_print_hi11xx_log(HI11XX_LOG_DBG, "gpio irq: %d disable nosync", pst_bus_dev->ul_wlan_irq);
            pst_bus_dev->ul_irq_stat = 1;
        }
    }
    oal_spin_unlock_irq_restore(&pst_bus_dev->st_irq_lock, &flags);
}

OAL_STATIC OAL_INLINE oal_void hcc_bus_print_state(oal_uint32 old_state, oal_uint32 new_state)
{
    if (old_state != new_state) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "bus state changed, tx[%s=>%s],rx[%s=>%s]",
                             (old_state & OAL_BUS_STATE_TX) ? "on " : "off",
                             (new_state & OAL_BUS_STATE_TX) ? "on " : "off",
                             (old_state & OAL_BUS_STATE_RX) ? "on " : "off",
                             (new_state & OAL_BUS_STATE_RX) ? "on " : "off");
    }
}

/*
 * 函 数 名  : hcc_bus_wakelocks_release_detect
 * 功能描述  : 强行释放wakelock锁
 * 返 回 值  : 成功或失败原因
 */
oal_void hcc_bus_wakelocks_release_detect(hcc_bus *pst_bus)
{
    /* before call this function , please make sure the rx/tx queue is empty and no data transfer!! */
    if (oal_warn_on(!pst_bus)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[E]pst_bus is error can not release wake locks");
        return;
    };

    if (hcc_bus_wakelock_active(pst_bus)) {
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "[E]We still hold %s   %lu wake locks, Now release all",
                             pst_bus->st_bus_wakelock.st_wakelock.name,
                             pst_bus->st_bus_wakelock.lock_count);
#endif
        declare_dft_trace_key_info("wlan_wakelock_error_hold", OAL_DFT_TRACE_EXCEP);
        pst_bus->st_bus_wakelock.lock_count = 1;
        hcc_bus_wake_unlock(pst_bus);
    }
}

oal_int32 hcc_bus_enable(hcc_bus *pst_bus)
{
    return OAL_SUCC;
}

oal_int32 hcc_bus_disable(hcc_bus *pst_bus)
{
    return OAL_SUCC;
}

oal_int32 hcc_bus_resource_alloc(hcc_bus *pst_bus)
{
    oal_int32 ret;
    oal_uint32 len;
    char *wakelock_name = NULL;
    const oal_uint32 ul_name_len = 200;
    oal_uint8 name[ul_name_len];

    if (oal_warn_on(!pst_bus)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "pst_bus is null,alloc fail ");
        return -OAL_EFAIL;
    };

    ret = snprintf_s(name, OAL_SIZEOF(name), OAL_SIZEOF(name) - 1, "wlan_bus_lock/%s:%u",
                     hcc_bus_get_bus_type_str(pst_bus->bus_type), pst_bus->dev_id);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "wakelock name formate err\n");
        return ret;
    }
    len = (oal_uint32)OAL_STRLEN(name) + 1;
    wakelock_name = oal_memalloc(len);
    if (wakelock_name == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc wakelock name space failed, len=%u, [%s]", len, name);
        return -OAL_ENOMEM;
    }

    memcpy_s((oal_void *)wakelock_name, len, (const oal_void *)name, len);

    oal_wake_lock_init(&pst_bus->st_bus_wakelock, wakelock_name);

    pst_bus->st_bus_wakelock.debug = g_hcc_bus_wakelock_debug;

    oal_init_completion(&pst_bus->st_device_ready);

    ret = snprintf_s(name, OAL_SIZEOF(name), OAL_SIZEOF(name) - 1, "wlan_bus_rx/%s:%u",
                     hcc_bus_get_bus_type_str(pst_bus->bus_type), pst_bus->dev_id);
    if (ret < 0) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus name formate err\n");
        oal_free(wakelock_name);
        oal_wake_lock_exit(&pst_bus->st_bus_wakelock);
        return ret;
    }

    /* create thread for gpio rx data in interrupt handler */
    pst_bus->pst_rx_tsk = oal_thread_create(hcc_bus_rxdata_thread,
                                            (hcc_bus *)pst_bus,
                                            &pst_bus->rx_sema,
                                            name,
                                            OAL_BUS_RX_THREAD_POLICY,
                                            OAL_BUS_RXDATA_THREAD_PRIORITY, g_cpu_id);
    if (pst_bus->pst_rx_tsk == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[SDIO][ERROR]failed to create rx data thread");
        oal_free(wakelock_name);
        oal_wake_lock_exit(&pst_bus->st_bus_wakelock);
        return -OAL_EFAIL;
    }
    mutex_init(&pst_bus->rx_transfer_lock);

    oal_atomic_set(&g_wakeup_dev_wait_ack, 0);
    oal_atomic_set(&g_bus_powerup_dev_wait_ack, 0);

    sema_init(&pst_bus->sr_wake_sema, 1);

    /* init exception work */
    oal_spin_lock_init(&pst_bus->bus_excp_lock);
    oal_init_work(&pst_bus->bus_excp_worker, hcc_bus_exception_handler);

    hcc_bus_message_register(pst_bus,
                             D2H_MSG_DEVICE_PANIC,
                             hcc_bus_device_panic_callback,
                             pst_bus);

    hcc_bus_message_register(pst_bus,
                             D2H_MSG_WLAN_READY,
                             hcc_bus_device_init_ready,
                             pst_bus);

    g_hisi_wifi_bus = pst_bus;

    return OAL_SUCC;
}

oal_void hcc_bus_resource_free(hcc_bus *pst_bus)
{
    char* name = NULL;
    if (oal_unlikely(pst_bus == NULL)) {
        oal_warn_on(1);
        return;
    }

    if (pst_bus->pst_rx_tsk != NULL) {
        /* include tread stop and irq free */
        oal_thread_stop(pst_bus->pst_rx_tsk, &pst_bus->rx_sema);
        pst_bus->pst_rx_tsk = NULL;
    }

    mutex_destroy(&pst_bus->rx_transfer_lock);

#if ((KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    name = (char *)pst_bus->st_bus_wakelock.st_wakelock.name;
#endif
    oal_wake_lock_exit(&pst_bus->st_bus_wakelock);

#if ((KERNEL_VERSION(2, 6, 37) <= LINUX_VERSION_CODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION))
    oal_free((oal_void *)name);
#endif
}

OAL_STATIC oal_void hcc_dev_pps_count_timeout(uintptr_t arg)
{
    hcc_bus_dev *pst_bus_dev = (hcc_bus_dev *)arg;
    if (oal_warn_on(pst_bus_dev == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus dev is null");
        return;
    }

    if (!hcc_bus_auto_sched_is_support(pst_bus_dev->dev_id)) {
        return;
    }

    hcc_bus_performance_core_schedule(pst_bus_dev->dev_id);

    oal_timer_start(&pst_bus_dev->bus_pps_timer, HCC_BUS_PPS_COUNT_TIMEOUT);
}

oal_void hcc_dev_exit(oal_void)
{
    oal_int32 i;
    hcc_bus_dev *pst_bus_dev = NULL;

    for (i = 0; i < (oal_int32)oal_array_size(g_bus_dev_res); i++) {
        pst_bus_dev = &g_bus_dev_res[i];

        hcc_bus_performance_core_exit(pst_bus_dev->dev_id);

        if (pst_bus_dev->hcc != NULL) {
            hcc_module_exit(pst_bus_dev->hcc);
            pst_bus_dev->hcc = NULL;
        }
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_unregister_notifier(&g_hcc_bus_pm_notifier);
#endif
}

oal_int32 hcc_dev_init(oal_void)
{
    oal_int32 i;
    oal_int32 ret;
    hcc_bus_dev *pst_bus_dev = NULL;
    oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc_dev_init we had %d devs", (oal_int32)oal_array_size(g_bus_dev_res));

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    wlan_pm_register_notifier(&g_hcc_bus_pm_notifier);
#endif

    for (i = 0; i < (oal_int32)oal_array_size(g_bus_dev_res); i++) {
        pst_bus_dev = &g_bus_dev_res[i];
        ret = hcc_select_bus(pst_bus_dev->dev_id, pst_bus_dev->init_bus_type);
        if (ret != OAL_SUCC) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus dev %s select bus failed, ret=%d", pst_bus_dev->name, ret);
            goto hcc_init_fail;
        }

        pst_bus_dev->hcc = hcc_module_init(pst_bus_dev);
        if (pst_bus_dev->hcc == NULL) {
            oal_print_hi11xx_log(HI11XX_LOG_ERR, "alloc hcc failed!");
            goto hcc_init_fail;
        }

        hcc_bus_performance_core_init(pst_bus_dev->dev_id);

        oal_init_completion(&pst_bus_dev->st_switch_powerup_ready);
        oal_init_completion(&pst_bus_dev->st_switch_powerdown_ready);
        oal_wait_queue_init_head(&pst_bus_dev->st_swtich_ack_wq);
        hcc_rx_register(pst_bus_dev->hcc, HCC_ACTION_TYPE_BUS, hcc_bus_rx_response, NULL);

        /* create switch bus thread */
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "hcc init dev %d succ", pst_bus_dev->dev_id);
    }

    return OAL_SUCC;

hcc_init_fail:
    hcc_dev_exit();
    return -OAL_EFAIL;
}

/* 枚举接口设备 */
oal_int32 oal_wifi_platform_load_dev(oal_void)
{
    oal_int32 ret;

    ret = oal_wifi_platform_load_sdio();
    if (ret != OAL_SUCC) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "load sdio failed ret=%d", ret);
        return ret;
    }

    return OAL_SUCC;
}

/* 卸载接口设备 */
oal_void oal_wifi_platform_unload_dev(oal_void)
{
    oal_wifi_platform_unload_sdio();
}

#else
oal_void hcc_dev_exit(oal_void)
{
}

oal_int32 hcc_dev_init(oal_void)
{
    return OAL_SUCC;
}

oal_int32 hcc_bus_resource_alloc(hcc_bus *pst_bus)
{
    oal_reference(pst_bus);
    return OAL_SUCC;
}

oal_void hcc_bus_resource_free(hcc_bus *pst_bus)
{
    oal_reference(pst_bus);
}

oal_int32 oal_register_gpio_flowctrl_intr(hcc_bus_dev *pst_bus_dev)
{
    oal_reference(pst_bus_dev);
    return OAL_SUCC;
}

oal_int32 oal_register_gpio_intr(hcc_bus_dev *pst_bus_dev)
{
    oal_reference(pst_bus_dev);
    return OAL_SUCC;
}

oal_void oal_unregister_gpio_flowctrl_intr(hcc_bus_dev *pst_bus_dev)
{
    oal_reference(pst_bus_dev);
}

oal_void oal_unregister_gpio_intr(hcc_bus_dev *pst_bus_dev)
{
    oal_reference(pst_bus_dev);
}

void hcc_bus_exception_handler(oal_work_stru *work)
{
    oal_reference(work);
}

oal_int32 oal_trigger_bus_exception(hcc_bus *hi_bus, oal_int32 is_sync)
{
    oal_reference(hi_bus);
    oal_reference(is_sync);
    return OAL_SUCC;
}

/* 枚举接口设备 */
oal_int32 oal_wifi_platform_load_dev(oal_void)
{
    return OAL_SUCC;
}

/* 卸载接口设备 */
oal_void oal_wifi_platform_unload_dev(oal_void)
{
}
#endif

oal_uint32 hcc_bus_dump_mem_check(hcc_bus *hi_bus)
{
    oal_reference(hi_bus);
    return !!g_bus_dump_mem_flag;
}
/*lint -e19*/
oal_module_symbol(hcc_bus_dump_mem_check);
/*lint +e19*/
/*
 * Prototype    : hcc_bus_message_register
 * Description  : msg irq
 */
oal_int32 hcc_bus_message_register(hcc_bus *hi_bus,
                                   oal_uint8 msg, hcc_bus_msg_rx cb,
                                   oal_void *data)
{
    if (hi_bus == NULL || msg >= D2H_MSG_COUNT) {
        return -OAL_EFAIL;
    }
    hi_bus->msg[msg].msg_rx = cb;
    hi_bus->msg[msg].data = data;
    return OAL_SUCC;
}

/*
 * Prototype    : hcc_bus_message_unregister
 * Description  : msg irq
 */
oal_void hcc_bus_message_unregister(hcc_bus *hi_bus,
                                    oal_uint8 msg)
{
    if (hi_bus == NULL || msg >= D2H_MSG_COUNT) {
        return;
    }
    hi_bus->msg[msg].msg_rx = NULL;
    hi_bus->msg[msg].msg_rx = NULL;
}

/*
 * Prototype    : hcc_bus_transfer_rx_register
 * Description  : msg irq
 */
oal_int32 hcc_bus_transfer_rx_register(hcc_bus *hi_bus, oal_void *data, hcc_bus_data_rx rx)
{
    if (oal_warn_on(!hi_bus)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "bus is null, register rx bus fail\n");
        return OAL_FAIL;
    }
    hi_bus->bus_ops.rx = rx;
    hi_bus->bus_ops_data = data;
    return OAL_SUCC;
}

/*
 * Prototype    : hcc_bus_transfer_rx_unregister
 * Description  : msg irq
 * Input        : hisdio_rx rx
 */
oal_void hcc_bus_transfer_rx_unregister(hcc_bus *hi_bus)
{
    if (oal_unlikely(hi_bus == NULL)) {
        oal_warn_on(1);
        return;
    }
    hi_bus->bus_ops.rx = NULL;
    hi_bus->bus_ops_data = NULL;
}

oal_int32 hcc_transfer_rx_register(struct hcc_handler *hcc, oal_void *data, hcc_bus_data_rx rx)
{
    oal_int32 flag = 0;
    oal_int32 ret = OAL_SUCC;
    oal_uint irqsave;
    oal_dlist_head_stru *pst_entry = NULL;
    hcc_bus *pst_bus_tmp = NULL;
    if (oal_unlikely(hcc == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }
    oal_spin_lock_irq_save(&g_hcc_bus_res_lock, &irqsave);
    oal_dlist_search_for_each(pst_entry, &g_hcc_bus_res_hdr)
    {
        pst_bus_tmp = oal_dlist_get_entry(pst_entry, hcc_bus, list);
        if (pst_bus_tmp->bus_dev == hcc->bus_dev) {
            /* 注册在当前设备上的所有BUS 都需要注册 */
            ret = hcc_bus_transfer_rx_register(pst_bus_tmp, data, rx);
            if (ret != OAL_SUCC) {
                oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);
                oal_print_hi11xx_log(HI11XX_LOG_ERR, "register rx bus %d failed, ret=%d", pst_bus_tmp->bus_id, ret);
                return ret;
            }
            flag++;
        }
    }
    oal_spin_unlock_irq_restore(&g_hcc_bus_res_lock, &irqsave);

    if (flag) {
        return OAL_SUCC;
    } else {
        return -OAL_ENODEV;
    }
}

oal_uint32 hcc_get_max_trans_size(struct hcc_handler *hcc)
{
    if (oal_warn_on(hcc == NULL)) {
        return 0;
    }

    if (oal_warn_on(hcc->bus_dev == NULL)) {
        return 0;
    }

    return hcc_bus_get_max_trans_size(hcc->bus_dev->cur_bus);
}

/*
 * Prototype    :oal_thread_create
 * Description  : create thread
 */
struct task_struct *oal_thread_create(int (*threadfn)(void *data),
                                      void *data,
                                      struct semaphore *sema_sync,
                                      const char *namefmt,
                                      oal_uint32 policy,
                                      oal_int32 prio,
                                      oal_int32 cpuid)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    int ret;
    struct task_struct *tsk = NULL;
    struct sched_param param;

    /* create thread for gpio rx data in interrupt handler */
    if (sema_sync != NULL) {
        sema_init(sema_sync, 0);
    }

    tsk = kthread_create(threadfn, data, "%s", namefmt);
    if (OAL_IS_ERR_OR_NULL(tsk)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "failed to run thread:%s", namefmt);
        return NULL;
    }

    /* set thread priority and schedule policy */
    param.sched_priority = prio;
    ret = sched_setscheduler(tsk, policy, &param);
    if (oal_unlikely(ret)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s sched_setscheduler failed! ret =%d, prio=%d", namefmt, ret, prio);
    }
    if (cpuid >= 0) {
        oal_kthread_bind(tsk, cpuid);
    } else {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "did not bind cpu...");
    }
    wake_up_process(tsk);
    return tsk;
#else
    return NULL;
#endif
}

/*lint -e19*/
oal_module_symbol(oal_thread_create);
/*lint +e19*/
void oal_set_thread_affinity(struct task_struct *pst_thread)
{
    struct cpumask cpu_mask;
    if (pst_thread == OAL_PTR_NULL) {
        return;
    }
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_NR_CPUS
    /* 固定绑定到CPU 1~7，内核自己调度 */
    cpumask_setall(&cpu_mask);
    cpumask_clear_cpu(0, &cpu_mask);
    set_cpus_allowed_ptr(pst_thread, &cpu_mask);
#endif
#endif
}
EXPORT_SYMBOL(oal_set_thread_affinity);

/*
 * Prototype    : oal_thread_stop
 * Description  : create thread
 */
void oal_thread_stop(struct task_struct *tsk,
                     struct semaphore *sema_sync)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    /* sema_sync can be NULL */
    /* create thread for gpio rx data in interrupt handler */
    send_sig(SIGTERM, tsk, 1);
    kthread_stop(tsk);
#endif
}
/*lint -e19*/
oal_module_symbol(oal_thread_stop);
/*lint +e19*/
void oal_set_thread_property(struct task_struct *p, int policy,
                             const struct sched_param *param,
                             long nice)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    if (oal_warn_on(!p)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "param set thread scheduler policy fail task_struct is null");
        return;
    };

    if (param == NULL) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "param set thread scheduler policy fail, param in null");
        return;
    };

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "set thread scheduler policy %d", policy);

    if (sched_setscheduler(p, policy, (struct sched_param *)param)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "[Error]set scheduler failed! %d", policy);
    }

    if (policy != SCHED_FIFO && policy != SCHED_RR) {
        oal_print_hi11xx_log(HI11XX_LOG_INFO, "set thread scheduler nice %ld", nice);
        set_user_nice(p, nice);
    }
#endif
}

/*lint -e19*/
oal_module_symbol(oal_set_thread_property);
/*lint +e19*/
/* Try to dump device mem, controlled by flag sdio_dump_mem_flag */
oal_void hcc_bus_try_to_dump_device_mem(hcc_bus *hi_bus, oal_int32 is_sync)
{
    if (oal_warn_on(hi_bus == NULL)) {
        return;
    }

    if (!hcc_bus_dump_mem_check(hi_bus)) {
        return;
    }

    oal_print_hi11xx_log(HI11XX_LOG_INFO, "Try to dump device mem!");
    oal_trigger_bus_exception(hi_bus, is_sync);
}
/*lint -e19*/
oal_module_symbol(hcc_bus_try_to_dump_device_mem);
oal_module_symbol(hcc_bus_transfer_rx_register);
oal_module_symbol(hcc_bus_flowctrl_init);
/*lint +e19*/
/*lint +e801*/
