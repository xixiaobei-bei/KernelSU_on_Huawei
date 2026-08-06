

/* 头文件包含 */
#include "bfgx_user_ctrl.h"
#include "plat_debug.h"
#include "plat_pm.h"
#include "bfgx_exception_rst.h"
#include "bfgx_dev.h"
#include "oal_kernel_file.h"
#include "oal_hcc_host_if.h"
#include "plat_pm_wlan.h"
#include "hisi_ini.h"
#include "plat_uart.h"
#include "plat_firmware.h"
#include "oam_rdr.h"
#include "securec.h"
#include "bfgx_data_parse.h"

/* 全局变量定义 */
static struct kobject *g_sysfs_hi110x_bfgx = NULL;
static struct kobject *g_sysfs_hi110x_pmdbg = NULL;

#ifdef PLATFORM_DEBUG_ENABLE
static struct kobject *g_sysfs_hi110x_debug = NULL;
#endif

int32_t g_hi110x_loglevel = PLAT_LOG_INFO;
oal_module_symbol(g_hi110x_loglevel);

#ifdef CONFIG_HISI_PMU_RTC_READCOUNT
extern unsigned long hisi_pmu_rtc_readcount(void);
#elif defined(CONFIG_PMU_RTC_READCOUNT)
extern unsigned long pmu_rtc_readcount(void);
#endif

STATIC ssize_t show_wifi_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int32_t ret;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "wifi_pm_debug usage: \n"
                     " 1:dump host info 2:dump device info\n"
                     " 3:open wifi      4:close wifi \n"
                     " 5:enable pm      6:disable pm \n");
#else
    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "wifi_pm_debug usage: \n"
                     " 1:dump host info 2:dump device info\n");
#endif
    if (ret < 0) {
        return ret;
    }

    return ret;
}

STATIC ssize_t store_wifi_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    int input;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    input = oal_atoi(buf);

    if (pst_wlan_pm == NULL) {
        oal_io_print("pm_data is NULL!\n");
        return -FAILURE;
    }

    /* case x => echo x->测试节点用 */
    switch (input) {
        case 1:
            wlan_pm_dump_host_info();
            break;
        case 2:
            wlan_pm_dump_device_info();
            break;
#ifdef PLATFORM_DEBUG_ENABLE
        case 3:
            wlan_pm_open();
            break;
        case 4:
            wlan_pm_close();
            break;
        case 5:
            wlan_pm_enable();
            break;
        case 6:
            wlan_pm_disable();
            break;
#endif
        default:
            break;
    }

    return count;
}

STATIC ssize_t show_bfgx_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "cmd       func     \n"
                      "  1  set pmdbg bfgx\n  2  set pmdbg gnss\n"
                      "  3  plat pm enable\n  4  plat pm disable\n"
                      "  5   bt  pm enable\n  6   bt  pm disable\n"
                      "  7  gnss pm enable\n  8  gnss pm disable\n"
                      "  9  pm ctrl enable\n  10 pm ctrl disable\n"
                      "  11 open bt\n  12 close bt\n"
                      "  13 open gnss\n  14 close gnss\n"
                      "  15 pm debug switch\n");
}

STATIC ssize_t store_bfgx_pmdbg(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    struct pm_drv_data *pm_data = NULL;
    struct ps_core_s *ps_core_d = NULL;
    int32_t cmd;
    int32_t ret;
    static int32_t uart_type = BUART;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    cmd = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    ps_print_info("cmd:%d\n", cmd);

    pm_data = pm_get_drvdata(uart_type);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    ps_core_d = ps_get_core_reference(uart_type);
    if (ps_core_d == NULL) {
        ps_print_err("ps_core_d is NULL\n");
        return -FAILURE;
    }

    ret = prepare_to_visit_node(ps_core_d);
    if (ret < 0) {
        ps_print_err("prepare work FAIL\n");
        return -FAILURE;
    }

    /* case x => echo x->测试节点用 */
    switch (cmd) {
        case 1: /* 1-set pmdbg bfgx */
            uart_type = BUART;
            ps_print_info("pmdbg from [%s] to [%s]\n", index2name(ps_core_d->pm_data->index), index2name(uart_type));
            break;
        case 2: /* 2-set pmdbg gnss */
            uart_type = GUART;
            ps_print_info("pmdbg from [%s] to [%s]\n", index2name(ps_core_d->pm_data->index), index2name(uart_type));
            break;
        case 3: /* 3-enable plat lowpower function */
            pm_data->bfgx_lowpower_enable = BFGX_PM_ENABLE;
            ps_print_info("bfgx platform pm enable\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_PL_ENABLE_PM);
            mod_timer(&pm_data->bfg_timer, jiffies + msecs_to_jiffies(BT_SLEEP_TIME));
            pm_data->bfg_timer_mod_cnt++;
            break;
        case 4: /* 4-disable plat lowpower function */
            pm_data->bfgx_lowpower_enable = BFGX_PM_DISABLE;
            ps_print_info("bfgx platform pm disable\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_PL_DISABLE_PM);
            break;
        case 5: /* 5-enable bt lowpower function */
            pm_data->bfgx_bt_lowpower_enable = BFGX_PM_ENABLE;
            ps_print_info("bfgx bt pm enable\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_BT_ENABLE_PM);
            break;
        case 6: /* 6-disable bt lowpower function */
            pm_data->bfgx_bt_lowpower_enable = BFGX_PM_DISABLE;
            ps_print_info("bfgx bt pm disable\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_BT_DISABLE_PM);
            break;
        case 7: /* 7-enable gnss lowpower function */
            pm_data->bfgx_gnss_lowpower_enable = BFGX_PM_ENABLE;
            ps_print_info("bfgx gnss pm enable\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_GNSS_ENABLE_PM);
            break;
        case 8: /* 8-disable gnss lowpower function */
            pm_data->bfgx_gnss_lowpower_enable = BFGX_PM_DISABLE;
            ps_print_info("bfgx gnss pm disable\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_GNSS_DISABLE_PM);
            break;
        case 9: /* 9-enable pm ctrl function */
            pm_data->bfgx_pm_ctrl_enable = BFGX_PM_ENABLE;
            break;
        case 10: /* 10-disable pm ctrl function */
            pm_data->bfgx_pm_ctrl_enable = BFGX_PM_DISABLE;
            break;
        case 11: /* 11-open bt */
            ps_print_info("open bt\n");
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_BT, BFG_POWER_GPIO_UP);
            break;
        case 12: /* 12-close bt */
            ps_print_info("close bt\n");
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_BT, BFG_POWER_GPIO_DOWN);
            break;
        case 13: /* 13-open gnss */
            ps_print_info("open gnss\n");
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_GNSS, BFG_POWER_GPIO_UP);
            break;
        case 14: /* 14-close gnss */
            ps_print_info("close gnss\n");
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_GNSS, BFG_POWER_GPIO_DOWN);
            break;
        case 15: /* 15-send cmd to dev */
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, PL_PM_DEBUG);
            break;
        default:
            ps_print_err("unknown cmd %d\n", cmd);
            break;
    }

    post_to_visit_node(ps_core_d);

    return count;
}

STATIC struct kobj_attribute g_wifi_pmdbg =
    __ATTR(wifi_pm, 0644, (void *)show_wifi_pmdbg, (void *)store_wifi_pmdbg);

STATIC struct kobj_attribute g_bfgx_pmdbg =
    __ATTR(bfgx_pm, 0644, (void *)show_bfgx_pmdbg, (void *)store_bfgx_pmdbg);

STATIC struct attribute *g_pmdbg_attrs[] = {
    &g_wifi_pmdbg.attr,
    &g_bfgx_pmdbg.attr,
    NULL,
};

STATIC struct attribute_group g_pmdbg_attr_grp = {
    .attrs = g_pmdbg_attrs,
};

STATIC ssize_t show_install(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return 0;
}

/* read by octty from hal to decide open which uart */
STATIC ssize_t show_dev_name(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%s\n", pm_data->dev_name);
}

/* read by octty from hal to decide what baud rate to use */
STATIC ssize_t show_baud_rate(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%ld\n", pm_data->baud_rate);
}

/* read by octty from hal to decide whether or not use flow cntrl */
STATIC ssize_t show_flow_cntrl(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;

    PS_PRINT_FUNCTION_NAME;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", pm_data->flow_cntrl);
}

/* show curr bfgx proto yes or not opened state */
STATIC ssize_t show_bfgx_active_state(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct ps_plat_s *pm_data = NULL;
    uint8_t bt_state = POWER_STATE_SHUTDOWN;
    uint8_t fm_state = POWER_STATE_SHUTDOWN;
    uint8_t gnss_state = POWER_STATE_SHUTDOWN;
#ifdef HAVE_HISI_IR
    uint8_t ir_state = POWER_STATE_SHUTDOWN;
#endif

    ps_print_dbg("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    ps_get_plat_reference(&pm_data);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -EFAULT;
    }

    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_BT].subsys_state) == true) {
        bt_state = POWER_STATE_OPEN;
    }

    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_FM].subsys_state) == true) {
        fm_state = POWER_STATE_OPEN;
    }

    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_GNSS].subsys_state) == true) {
        gnss_state = POWER_STATE_OPEN;
    }

#ifdef HAVE_HISI_IR
    if (atomic_read(&pm_data->core_data->bfgx_info[BFGX_IR].subsys_state) == true) {
        ir_state = POWER_STATE_OPEN;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "bt:%d; fm:%d; gnss:%d; ir:%d;\n",
                      bt_state, fm_state, gnss_state, ir_state);
#else
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "bt:%d; fm:%d; gnss:%d;\n",
                      bt_state, fm_state, gnss_state);
#endif
}

STATIC ssize_t show_ini_file_name(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, INI_FILE_PATH_LEN, "%s", get_ini_file_name());
}

STATIC ssize_t show_exception_info(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return hisi_conn_save_stat_info(buf, plat_get_dfr_sinfo(buf, 0), PAGE_SIZE);
}

STATIC ssize_t show_exception_count(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_dbg("exception debug: wifi rst count is %d\n", plat_get_excp_total_cnt());
    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", plat_get_excp_total_cnt());
}
#ifdef HAVE_HISI_GNSS
STATIC ssize_t show_gnss_lowpower_state(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    ps_print_dbg("show_gnss_lowpower_state!\n");

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", atomic_read(&pm_data->gnss_sleep_flag));
}

STATIC ssize_t gnss_lowpower_state_store(struct kobject *kobj, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    int flag;
    struct pm_drv_data *pm_data = NULL;

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    pm_data = pm_get_drvdata(BUART);
    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    flag = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    ps_print_info("flag = %d!\n", flag);

    /* gnss write the flag to request sleep */
    if (flag == 1) {
        if (pm_data->bfgx_lowpower_enable == BFGX_PM_DISABLE) {
            ps_print_warning("gnss low power disabled!\n");
            return -FAILURE;
        }
        if (pm_data->bfgx_dev_state_get(pm_data) == BFGX_SLEEP) {
            ps_print_warning("gnss proc: dev has been sleep, not allow dev slp\n");
            return -FAILURE;
        }

        /* if bt and fm are both shutdown ,we will pull down gpio directly */
        if (!timer_pending(&pm_data->bfg_timer)) {
            ps_print_info("gnss low power request sleep!\n");

            if (queue_work(pm_data->wkup_dev_workqueue, &pm_data->send_allow_sleep_work) != true) {
                ps_print_info("queue_work send_allow_sleep_work not return true\n");
            }
        }

        /* set the flag to 1 means gnss request sleep */
        atomic_set(&pm_data->gnss_sleep_flag, GNSS_AGREE_SLEEP);
    } else {
        ps_print_err("invalid gnss lowpower data!\n");
        return -FAILURE;
    }

    return count;
}
#endif

STATIC ssize_t show_loglevel(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
        "curr loglevel=%d\nerr:1\nwarning:2\nfunc|succ|info:3\ndebug:4\n", g_hi110x_loglevel);
}

STATIC ssize_t store_loglevel(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t loglevel;

    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (*buf == 'r') {
        ps_set_device_log_status(true);
        ps_print_info("enable device log\n");
        return count;
    } else if (*buf == 'R') {
        ps_set_device_log_status(false);
        ps_print_info("disable device log\n");
        return count;
    }

    loglevel = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    if (loglevel < PLAT_LOG_ALERT) {
        g_hi110x_loglevel = PLAT_LOG_ALERT;
    } else if (loglevel > PLAT_LOG_DEBUG) {
        g_hi110x_loglevel = PLAT_LOG_DEBUG;
    } else {
        g_hi110x_loglevel = loglevel;
    }

    return count;
}

#ifdef HAVE_HISI_IR
STATIC ssize_t show_ir_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (!g_st_board_info.have_ir) {
        ps_print_err("board have no ir module");
        return -FAILURE;
    }

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (g_st_board_info.irled_power_type == IR_GPIO_CTRL) {
        return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", gpio_get_value(g_st_board_info.bfgx_ir_ctrl_gpio));
    } else if (g_st_board_info.irled_power_type == IR_LDO_CTRL) {
        return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n",
                          (regulator_is_enabled(g_st_board_info.bfgn_ir_ctrl_ldo)) > 0 ? 1 : 0);
    }

    return -FAILURE;
}

STATIC ssize_t store_ir_mode(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t ir_ctrl_level;
    int ret;

    ps_print_info("into %s,irled_power_type is %d\n", __func__, g_st_board_info.irled_power_type);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (!g_st_board_info.have_ir) {
        ps_print_err("board have no ir module");
        return -FAILURE;
    }

    if (g_st_board_info.irled_power_type == IR_GPIO_CTRL) {
        ir_ctrl_level = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
        if (ir_ctrl_level == GPIO_LOWLEVEL) {
            gpio_direction_output(g_st_board_info.bfgx_ir_ctrl_gpio, GPIO_LOWLEVEL);
        } else if (ir_ctrl_level == GPIO_HIGHLEVEL) {
            gpio_direction_output(g_st_board_info.bfgx_ir_ctrl_gpio, GPIO_HIGHLEVEL);
        } else {
            ps_print_err("gpio level should be 0 or 1, cur value is [%d]\n", ir_ctrl_level);
            return -FAILURE;
        }
    } else if (g_st_board_info.irled_power_type == IR_LDO_CTRL) {
        if (IS_ERR(g_st_board_info.bfgn_ir_ctrl_ldo)) {
            ps_print_err("ir_ctrl get ird ldo failed\n");
            return -FAILURE;
        }

        ir_ctrl_level = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
        if (ir_ctrl_level == GPIO_LOWLEVEL) {
            ret = regulator_disable(g_st_board_info.bfgn_ir_ctrl_ldo);
            if (ret) {
                ps_print_err("ir_ctrl disable ldo failed\n");
            }
        } else if (ir_ctrl_level == GPIO_HIGHLEVEL) {
            ret = regulator_enable(g_st_board_info.bfgn_ir_ctrl_ldo);
            if (ret) {
                ps_print_err("ir_ctrl enable ldo failed\n");
            }
        } else {
            ps_print_err("ir_ctrl level should be 0 or 1, cur value is [%d]\n", ir_ctrl_level);
            return -FAILURE;
        }
    } else {
        ps_print_err("get ir_ldo_type error! ir_ldo_type is %d!\n", g_st_board_info.irled_power_type);
        return -FAILURE;
    }
    ps_print_info("set ir ctrl mode as %d!\n", (int)ir_ctrl_level);
    return count;
}
#endif

STATIC ssize_t bfgx_sleep_state_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", pm_data->bfgx_dev_state);
}

STATIC ssize_t bfgx_wkup_host_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct pm_drv_data *pm_data = pm_get_drvdata(BUART);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (pm_data == NULL) {
        ps_print_err("pm_data is NULL!\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", pm_data->bfg_wakeup_host);
}

#ifdef CONFIG_HISI_PMU_RTC_READCOUNT
STATIC ssize_t show_hisi_gnss_ext_rtc_count(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned long rtc_count;

    if ((kobj == NULL) || (attr == NULL) || (buf == NULL)) {
        ps_print_err("paramater is NULL\n");
        return 0;
    }

    rtc_count = hisi_pmu_rtc_readcount();
    ps_print_info("show_hisi_gnss_ext_rtc_count: %ld\n", rtc_count);
    if (memcpy_s(buf, PAGE_SIZE, (char *)&rtc_count, sizeof(rtc_count)) != EOK) {
        ps_print_err("memcpy_s error, destlen=%lu, srclen=%lu\n ", PAGE_SIZE, sizeof(rtc_count));
        return 0;
    }
    return sizeof(rtc_count);
}
#elif defined(CONFIG_PMU_RTC_READCOUNT)
STATIC ssize_t show_hisi_gnss_ext_rtc_count(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    unsigned long rtc_count;

    if ((kobj == NULL) || (attr == NULL) || (buf == NULL)) {
        ps_print_err("paramater is NULL\n");
        return 0;
    }

    rtc_count = pmu_rtc_readcount();
    ps_print_info("show_hisi_gnss_ext_rtc_count: %ld\n", rtc_count);
    if (memcpy_s(buf, PAGE_SIZE, (char *)&rtc_count, sizeof(rtc_count)) != EOK) {
        ps_print_err("memcpy_s error, destlen=%lu, srclen=%lu\n ", PAGE_SIZE, sizeof(rtc_count));
        return 0;
    }
    return sizeof(rtc_count);
}
#endif
STATIC struct kobj_attribute g_plat_exception_info =
    __ATTR(excp_info, 0444, (void *)show_exception_info, NULL);

#ifdef CONFIG_HI110X_GPS_SYNC
STATIC int32_t gnss_sync_convert_mode_modem0(int32_t rat_mode, uint32_t version)
{
    int32_t sync_mode = GNSS_SYNC_MODE_UNKNOWN;
    switch (rat_mode) {
        case GNSS_RAT_MODE_GSM:
            sync_mode = GNSS_SYNC_MODE_G1;
            break;
        case GNSS_RAT_MODE_WCDMA:
            sync_mode = GNSS_SYNC_MODE_PW;
            break;
        case GNSS_RAT_MODE_LTE:
            sync_mode = GNSS_SYNC_MODE_LTE;
            break;
        case GNSS_RAT_MODE_CDMA:
            sync_mode = GNSS_SYNC_MODE_CDMA;
            break;
        case GNSS_RAT_MODE_NR:
            if (version >= GNSS_SYNC_VERSION_5G) {
                sync_mode = GNSS_SYNC_MODE_NSTU;
            }
            break;
        default:
            sync_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }
    return sync_mode;
}

STATIC int32_t gnss_sync_convert_mode_modem1(int32_t rat_mode)
{
    int32_t sync_mode;
    switch (rat_mode) {
        case GNSS_RAT_MODE_GSM:
            sync_mode = GNSS_SYNC_MODE_G2;
            break;
        case GNSS_RAT_MODE_WCDMA:
            sync_mode = GNSS_SYNC_MODE_SW;
            break;
        case GNSS_RAT_MODE_LTE:
            sync_mode = GNSS_SYNC_MODE_LTE2;
            break;
        case GNSS_RAT_MODE_CDMA:
            sync_mode = GNSS_SYNC_MODE_CDMA;
            break;
        default:
            sync_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }
    return sync_mode;
}

STATIC int32_t gnss_sync_convert_mode_modem2(int32_t rat_mode)
{
    int32_t sync_mode;
    switch (rat_mode) {
        case GNSS_RAT_MODE_GSM:
            sync_mode = GNSS_SYNC_MODE_G3;
            break;
        case GNSS_RAT_MODE_CDMA:
            sync_mode = GNSS_SYNC_MODE_CDMA;
            break;
        default:
            sync_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }
    return sync_mode;
}

/*
 * Driver与Host接口定义为以下几个参数，每个参数之间用‘,’分割
 * "modem_id,rat_mode"
 */
STATIC int32_t gnss_sync_convert_mode(const char *rcv_data, int32_t *set_mode, uint32_t version)
{
    char *pc_split = ",";
    char *pc_str1 = (char *)rcv_data;
    char *pc_str2 = NULL;
    int32_t modem_id;
    int32_t rat_mode;

    if (pc_str1 == NULL || set_mode == NULL) {
        ps_print_err("received data or set mode is null\n");
        return -FAILURE;
    }

    modem_id = oal_atoi(pc_str1);
    if (modem_id < 0) {
        ps_print_err("modem id is invalid\n");
        return -FAILURE;
    }

    pc_str2 = oal_strstr(pc_str1, pc_split);
    if (pc_str2 == NULL) {
        ps_print_err("cannot find the split\n");
        return -FAILURE;
    }

    pc_str2++;
    rat_mode = oal_atoi(pc_str2);
    if (rat_mode < GNSS_RAT_MODE_NO_SERVICE || rat_mode >= GNSS_RAT_MODE_BUTT) {
        ps_print_err("rat mode is invalid\n");
        return -FAILURE;
    }

    /* 根据输入的值判断 */
    switch (modem_id) {
        case 0:
            *set_mode = gnss_sync_convert_mode_modem0(rat_mode, version);
            break;
        case 1:
            *set_mode = gnss_sync_convert_mode_modem1(rat_mode);
            break;
        case 2:
            *set_mode = gnss_sync_convert_mode_modem2(rat_mode);
            break;
        default:
            *set_mode = GNSS_SYNC_MODE_UNKNOWN;
            break;
    }

    return SUCCESS;
}

STATIC int32_t gnss_sync_mode_ctrl(struct gnss_sync_data *sync_data, int32_t set_mode)
{
    int32_t curr_mode = 0;
    if (set_mode != GNSS_SYNC_MODE_UNKNOWN) {
        curr_mode = readl(sync_data->addr_base_virt + sync_data->addr_offset);
        curr_mode = (int32_t)(((uint32_t)curr_mode & 0xfffffff0) | (uint32_t)set_mode); // only bit 3:0
        writel(curr_mode, (sync_data->addr_base_virt + sync_data->addr_offset));         // model selection
    } else {
        return -FAILURE;
    }

    return SUCCESS;
}

STATIC ssize_t show_hisi_gnss_sync_mode(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    int32_t mode;
    struct gnss_sync_data *sync_data = gnss_get_sync_data();

    ps_print_dbg("show hisi gnss sync mode!\n");

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    if (sync_data == NULL) {
        ps_print_err("sync_data is NULL!\n");
        return -FAILURE;
    }

    if (sync_data->addr_base_virt == NULL) {
        ps_print_err("register virutal address is NULL!\n");
        return -FAILURE;
    }

    mode = readl(sync_data->addr_base_virt + sync_data->addr_offset);

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1, "%d\n", mode);
}

STATIC ssize_t store_hisi_gnss_sync_mode(struct kobject *kobj, struct kobj_attribute *attr,
                                         const char *buf, size_t count)
{
    int32_t ret;
    int32_t set_mode = GNSS_SYNC_MODE_UNKNOWN;
    struct gnss_sync_data *sync_data = NULL;

    ps_print_info("store hisi gnss sync mode!\n");

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    sync_data = gnss_get_sync_data();
    if (sync_data == NULL) {
        ps_print_err("sync_data is NULL!\n");
        return -FAILURE;
    }

    if (sync_data->addr_base_virt == NULL) {
        ps_print_err("register virutal address is NULL!\n");
        return -FAILURE;
    }

    if (sync_data->version == GNSS_SYNC_VERSION_OFF) {
        ps_print_info("sync version off!!\n");
        return -EACCES;
    } else if (sync_data->version >= GNSS_SYNC_VERSION_BUTT) {
        ps_print_warning("sync version %d has not been realized!\n", sync_data->version);
        return -FAILURE;
    }

    ret = gnss_sync_convert_mode(buf, &set_mode, sync_data->version);
    if (ret != SUCCESS) {
        return ret;
    }
    ps_print_info("gnss sync mode \"%s\" convert to %d", buf, set_mode);

    ret = gnss_sync_mode_ctrl(sync_data, set_mode);
    if (ret != SUCCESS) {
        return ret;
    }

    return count;
}
#endif

STATIC struct kobj_attribute g_ldisc_install =
    __ATTR(install, 0444, (void *)show_install, NULL);

STATIC struct kobj_attribute g_uart_dev_name =
    __ATTR(dev_name, 0444, (void *)show_dev_name, NULL);

STATIC struct kobj_attribute g_uart_baud_rate =
    __ATTR(baud_rate, 0444, (void *)show_baud_rate, NULL);

STATIC struct kobj_attribute g_uart_flow_cntrl =
    __ATTR(flow_cntrl, 0444, (void *)show_flow_cntrl, NULL);

STATIC struct kobj_attribute g_bfgx_active_state =
    __ATTR(bfgx_state, 0444, (void *)show_bfgx_active_state, NULL);

STATIC struct kobj_attribute g_hisi_ini_file_name =
    __ATTR(ini_file_name, 0444, (void *)show_ini_file_name, NULL);

STATIC struct kobj_attribute g_rst_count =
    __ATTR(rst_count, 0444, (void *)show_exception_count, NULL);

#ifdef HAVE_HISI_GNSS
STATIC struct kobj_attribute g_gnss_lowpower_cntrl =
    __ATTR(gnss_lowpower_state, 0644, (void *)show_gnss_lowpower_state, (void *)gnss_lowpower_state_store);
#endif

STATIC struct kobj_attribute g_bfgx_loglevel =
    __ATTR(loglevel, 0664, (void *)show_loglevel, (void *)store_loglevel);

#ifdef HAVE_HISI_IR
STATIC struct kobj_attribute g_bfgx_ir_ctrl =
    __ATTR(ir_ctrl, 0664, (void *)show_ir_mode, (void *)store_ir_mode);
#endif

#if defined(CONFIG_HISI_PMU_RTC_READCOUNT) || defined(CONFIG_PMU_RTC_READCOUNT)
STATIC struct kobj_attribute g_hisi_gnss_ext_rtc =
    __ATTR(gnss_ext_rtc, 0444, (void *)show_hisi_gnss_ext_rtc_count, NULL);
#endif

#ifdef CONFIG_HI110X_GPS_SYNC
STATIC struct kobj_attribute g_hisi_gnss_sync =
    __ATTR(gnss_sync, 0664, (void *)show_hisi_gnss_sync_mode, (void *)store_hisi_gnss_sync_mode);
#endif

STATIC struct kobj_attribute g_bfgx_sleep_attr =
    __ATTR(bfgx_sleep_state, 0444, (void *)bfgx_sleep_state_show, NULL);

STATIC struct kobj_attribute g_bfgx_wkup_host_count_attr =
    __ATTR(bfgx_wkup_host_count, 0444, (void *)bfgx_wkup_host_count_show, NULL);

STATIC struct attribute *g_bfgx_attrs[] = {
    &g_ldisc_install.attr,
    &g_uart_dev_name.attr,
    &g_uart_baud_rate.attr,
    &g_uart_flow_cntrl.attr,
    &g_bfgx_active_state.attr,
    &g_hisi_ini_file_name.attr,
    &g_rst_count.attr,
    &g_plat_exception_info.attr,
#ifdef HAVE_HISI_GNSS
    &g_gnss_lowpower_cntrl.attr,
#endif
    &g_bfgx_loglevel.attr,
#ifdef HAVE_HISI_IR
    &g_bfgx_ir_ctrl.attr,
#endif
    &g_bfgx_sleep_attr.attr,
    &g_bfgx_wkup_host_count_attr.attr,
#if defined(CONFIG_HISI_PMU_RTC_READCOUNT) || defined(CONFIG_PMU_RTC_READCOUNT)
    &g_hisi_gnss_ext_rtc.attr,
#endif
#ifdef CONFIG_HI110X_GPS_SYNC
    &g_hisi_gnss_sync.attr,
#endif
    NULL,
};

STATIC struct attribute_group g_bfgx_attr_grp = {
    .attrs = g_bfgx_attrs,
};

#ifdef PLATFORM_DEBUG_ENABLE
STATIC ssize_t show_exception_dbg(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
                      "==========cmd  func             \n"
                      "   1  clear dfr info          \n"
                      "   2  cause bfgx beattimer timerout\n"
                      "   3  enable dfr subsystem rst\n"
                      "   4  disble dfr subsystem rst\n"
                      "   5  en dfr bfgx pwron fail\n"
                      "   6  en dfr bfgx pwroff fail\n"
                      "   7  en dfr wifi wkup fail\n"
                      "   8  cause bfgx panic\n"
                      "   9  cause bfgx arp exception system rst\n"
                      "   10 bfgx wkup fail\n"
                      "   11 wifi WIFI_TRANS_FAIL\n"
                      "   12 wifi WIFI_POWER ON FAIL\n");
}

STATIC ssize_t store_exception_dbg(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t cmd;
    int32_t ret = 0;
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;
    struct wlan_pm_s *pst_wlan_pm = wlan_pm_get_drv();
    hcc_bus *hi_bus = hcc_get_current_110x_bus();
    struct hcc_handler *pst_hcc = hcc_get_110x_handler();

    if (buf == NULL) {
        ps_print_err("[dfr_test]buf is NULL\n");
        return -FAILURE;
    }
    if (hi_bus == NULL) {
        ps_print_err("[dfr_test]hi_bus is null");
        return -FAILURE;
    }

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("[dfr_test]get exception info reference is error\n");
        return 0;
    }

    ps_core_d = ps_get_core_reference(BUART);
    if (ps_core_d == NULL) {
        ps_print_err("[dfr_test]ps_core_d is NULL\n");
        return 0;
    }

    cmd = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    ps_print_info("[dfr_test]cmd:%d\n", cmd);
    /* case x => echo x->测试节点用 */
    switch (cmd) {
        case 1:
            ps_print_info("[dfr_test]clear dfr info\n");
            memset_s(&pst_exception_data->etype_info, sizeof(pst_exception_data->etype_info),
                     0, sizeof(pst_exception_data->etype_info));
            break;
        case 2:
            ps_print_info("[dfr_test]bfgx device timeout cause ++\n");
            ret = prepare_to_visit_node(ps_core_d);
            if (ret < 0) {
                ps_print_err("[dfr_test]prepare work FAIL\n");
                return ret;
            }
            pst_exception_data->debug_beat_flag = 0;
            /* 等待dfr完成，等待进入dfr流程，防止睡眠 */
            while (atomic_read(&pst_exception_data->is_reseting_device) == PLAT_EXCEPTION_RESET_IDLE) {
                ;
            };
            post_to_visit_node(ps_core_d);
            ps_print_info("[dfr_test]bfgx device timeout cause --\n");
            break;
        case 3:
            ps_print_info("[dfr_test]enable dfr subsystem rst\n");
            pst_exception_data->subsystem_rst_en = DFR_TEST_ENABLE;
            break;
        case 4:
            ps_print_info("[dfr_test]disable dfr subsystem rst\n");
            pst_exception_data->subsystem_rst_en = DFR_TEST_DISABLE;
            break;
        case 5:
            ps_print_info("[dfr_test]bfgx powon fail dfr test en,next poweron will fail\n");
            set_excp_test_en(BFGX_POWEON_FAULT);
            break;
        case 6:
            ps_print_info("[dfr_test]bfgx pwr off dfr test en,next poweroff will fail\n");
            set_excp_test_en(BFGX_POWEOFF_FAULT);
            break;
        case 7:
            if (pst_hcc == NULL) {
                ps_print_info("pst_hcc is null\n");
                break;
            }
            if (oal_atomic_read(&pst_hcc->state) != HCC_ON) {
                ps_print_info("wifi is closed\n");
                break;
            }
            ps_print_info("[dfr_test]wifi wkup dfr test en,next wkup will fail\n");
            if (pst_wlan_pm->wlan_dev_state == HOST_ALLOW_TO_SLEEP) {
                hcc_tx_transfer_lock(hcc_get_110x_handler());
                set_excp_test_en(WIFI_WKUP_FAULT);
                wlan_pm_wakeup_dev();
                hcc_tx_transfer_unlock(hcc_get_110x_handler());
            } else {
                ps_print_warning("[dfr_test]wifi wkup dfr trigger fail , cur state:%ld\n",
                                 pst_wlan_pm->wlan_dev_state);
            }
            break;
        case 8:
            ret = prepare_to_visit_node(ps_core_d);
            if (ret < 0) {
                ps_print_err("[dfr_test]prepare work FAIL\n");
                return ret;
            }
            ps_print_info("[dfr_test]bfgx device panic cause\n");
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_DEV_PANIC);
            post_to_visit_node(ps_core_d);
            break;
        case 9:
            ps_print_info("[dfr_test]trigger hal BFGX_ARP_TIMEOUT exception\n");
            plat_exception_handler(SUBSYS_BFGX, THREAD_GNSS, BFGX_ARP_TIMEOUT);
            break;
        case 10:
            ps_print_info("[dfr_test]bfgx wkup dfr test en,next wkup will fail \n");
            plat_exception_handler(SUBSYS_BFGX, THREAD_NFC, BFGX_WAKEUP_FAIL);
            break;
        case 11:
            if (pst_hcc == NULL) {
                ps_print_info("pst_hcc is null\n");
                break;
            }
            if (oal_atomic_read(&pst_hcc->state) != HCC_ON) {
                ps_print_info("wifi is closed\n");
                break;
            }
            ps_print_info("[dfr_test]wifi WIFI_TRANS_FAIL submit \n");
            hcc_tx_transfer_lock(pst_hcc);
            if (hcc_bus_pm_wakeup_device(hi_bus) == OAL_SUCC) {
                hcc_bus_exception_submit(hi_bus, WIFI_TRANS_FAIL);
            }
            hcc_tx_transfer_unlock(pst_hcc);
            break;

        case 12:
            ps_print_info("[dfr_test]wifi WIFI_power on failed\n");
            set_excp_test_en(WIFI_POWER_ON_FAULT);
            break;

        default:
            ps_print_err("[dfr_test]unknown cmd %d\n", cmd);
            break;
    }
    return count;
}

STATIC ssize_t show_dev_test(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    ps_print_info("%s\n", __func__);

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    return snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
                      "cmd  func\n  1  cause bfgx panic\n  2  enable exception recovery\n  3  enable wifi open bcpu\n"
                      "  4  pull up power gpio\n  5  pull down power gpio\n  6  uart loop test\n"
                      "  7  wifi download test\n  8  open uart\n  9  close uart\n"
                      "  10 open bt\n  11 bfg download test\n  12 bfg wifi download test\n"
                      "  13 wlan_pm_open_bcpu\n  14 wlan_power_on \n  15 wlan_power_off\n"
                      "  16 wlan_pm_open\n  17 wlan_pm_close\n  18 bfg gpio power off\n"
                      "  19 bfg gpio power on\n  20 gnss monitor enable\n"
                      "  21 rdr test\n  22 bfgx power on\n");
}

oal_uint wlan_pm_open_bcpu(void);

STATIC ssize_t store_dev_test(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int32_t cmd;
    int32_t ret;
    uint8_t send_data[] = { 0x7e, 0x0, 0x06, 0x0, 0x0, 0x7e };
    struct ps_core_s *ps_core_d = NULL;
    struct st_exception_info *pst_exception_data = NULL;
    hi110x_board_info *bd_info = NULL;

    ps_print_info("%s\n", __func__);

    bd_info = get_hi110x_board_info();
    if (unlikely(bd_info == NULL)) {
        ps_print_err("board info is err\n");
        return BFGX_POWER_FAILED;
    }

    if (buf == NULL) {
        ps_print_err("buf is NULL\n");
        return -FAILURE;
    }

    pst_exception_data = get_exception_info_reference();
    if (pst_exception_data == NULL) {
        ps_print_err("get exception info reference is error\n");
        return 0;
    }

    ps_core_d = ps_get_core_reference(BUART);
    if (unlikely(ps_core_d == NULL)) {
        ps_print_err("ps_core_d is NULL\n");
        return -EINVAL;
    }

    cmd = simple_strtol(buf, NULL, 10); /* 将字符串转换成10进制数 */
    /* case x => echo x->测试节点用 */
    switch (cmd) {
        case 1:
            ret = prepare_to_visit_node(ps_core_d);
            if (ret < 0) {
                ps_print_err("prepare work FAIL\n");
                return ret;
            }

            ps_print_info("bfgx test cmd %d, cause device panic\n", cmd);
            ps_tx_sys_cmd(ps_core_d, SYS_MSG, SYS_CFG_DEV_PANIC);

            post_to_visit_node(ps_core_d);
            break;
        case 2:
            ps_print_info("cmd %d,enable platform dfr\n", cmd);
            pst_exception_data->exception_reset_enable = PLAT_EXCEPTION_ENABLE;
            break;
        case 3:
            ps_print_info("cmd %d,enable wifi open bcpu\n", cmd);
            break;
        case 4:
            ps_print_info("cmd %d,test pull up power gpio\n", cmd);
            bd_info->bd_ops.board_power_on(WLAN_POWER);
            break;
        case 5:
            ps_print_info("cmd %d,test pull down power gpio\n", cmd);
            bd_info->bd_ops.board_power_off(WLAN_POWER);
            break;
        case 6:
            ps_print_info("cmd %d,start uart loop test\n", cmd);
            uart_loop_test();
            break;
        case 7:
            ps_print_info("cmd %d,firmware download wifi_cfg\n", cmd);
            firmware_download_function(WIFI_CFG);
            break;
        case 8:
            ps_print_info("cmd %d,open uart\n", cmd);
            open_tty_drv(ps_core_d);
            break;
        case 9:
            ps_print_info("cmd %d,close uart\n", cmd);
            release_tty_drv(ps_core_d);
            break;
        case 10:
            ps_print_info("cmd %d,uart cmd test\n", cmd);
            ps_write_tty(ps_core_d, send_data, sizeof(send_data));
            break;
        case 11:
            ps_print_info("cmd %d,firmware download bfgx_cfg\n", cmd);
            firmware_download_function(BFGX_CFG);
            break;
        case 12:
            ps_print_info("cmd %d,firmware download bfgx_and_wifi cfg\n", cmd);
            firmware_download_function(BFGX_AND_WIFI_CFG);
            break;
        case 13:
            ps_print_info("cmd %d,wlan_pm_open_bcpu\n", cmd);
            wlan_pm_open_bcpu();
            break;
        case 14:
            ps_print_info("cmd %d,hi1103_wlan_power_on\n", cmd);
            hi1103_wlan_power_on();
            break;
        case 15:
            ps_print_info("cmd %d,hi1103_wlan_power_off\n", cmd);
            hi1103_wlan_power_off();
            break;
        case 16:
            ps_print_info("cmd %d,wlan_pm_open\n", cmd);
            wlan_pm_open();
            break;
        case 17:
            ps_print_info("cmd %d,wlan_pm_close\n", cmd);
            wlan_pm_close();
            break;
        case 18:
            ps_print_info("cmd %d,bfgx gpio power off\n", cmd);
            bd_info->bd_ops.board_power_off(BFGX_POWER);
            break;
        case 19:
            ps_print_info("cmd %d,bfgx gpio power on\n", cmd);
            bd_info->bd_ops.board_power_on(BFGX_POWER);
            break;
        case 20:
            ps_print_info("cmd %d,g_device_monitor_enable set to 1\n", cmd);
            g_device_monitor_enable = 1;
            break;
        case 21:
            rdr_exception(MODID_CONN_WIFI_EXEC);
            rdr_exception(MODID_CONN_WIFI_CHAN_EXEC);
            rdr_exception(MODID_CONN_WIFI_WAKEUP_FAIL);
            rdr_exception(MODID_CONN_BFGX_EXEC);
            rdr_exception(MODID_CONN_BFGX_BEAT_TIMEOUT);
            rdr_exception(MODID_CONN_BFGX_WAKEUP_FAIL);
            break;
        case 22:
            ps_print_err("bfgx power on %d\n", cmd);
            ps_core_d->pm_data->bfg_power_set(ps_core_d->pm_data, BFGX_BT, BFG_POWER_GPIO_UP);
            break;
        default:
            ps_print_err("unknown cmd %d\n", cmd);
            break;
    }

    return count;
}

STATIC struct kobj_attribute g_plat_exception_dbg =
    __ATTR(exception_dbg, 0644, (void *)show_exception_dbg, (void *)store_exception_dbg);

STATIC struct kobj_attribute g_bfgx_dev_test =
    __ATTR(bfgx_test, 0664, (void *)show_dev_test, (void *)store_dev_test);

STATIC struct attribute *g_hi110x_debug_attrs[] = {
    &g_plat_exception_dbg.attr,
    &g_bfgx_dev_test.attr,
    NULL,
};

STATIC struct attribute_group g_hi110x_debug_attr_grp = {
    .attrs = g_hi110x_debug_attrs,
};
#endif

int32_t bfgx_user_ctrl_init(void)
{
    int status;
    struct kobject *pst_root_object = NULL;

    pst_root_object = oal_get_sysfs_root_object();
    if (pst_root_object == NULL) {
        ps_print_err("[E]get root sysfs object failed!\n");
        return -EFAULT;
    }

    g_sysfs_hi110x_pmdbg = kobject_create_and_add("pmdbg", pst_root_object);
    if (g_sysfs_hi110x_pmdbg == NULL) {
        ps_print_err("Failed to creat sysfs_hisi_pmdbg !!!\n ");
        goto fail_g_sysfs_hisi_pmdbg;
    }

    status = oal_debug_sysfs_create_group(g_sysfs_hi110x_pmdbg, &g_pmdbg_attr_grp);
    if (status) {
        ps_print_err("failed to create g_sysfs_hi110x_pmdbg sysfs entries\n");
        goto fail_create_pmdbg_group;
    }

    g_sysfs_hi110x_bfgx = kobject_create_and_add("hi110x_ps", NULL);
    if (g_sysfs_hi110x_bfgx == NULL) {
        ps_print_err("Failed to creat sysfs_hi110x_ps !!!\n ");
        goto fail_g_sysfs_hi110x_bfgx;
    }

    status = sysfs_create_group(g_sysfs_hi110x_bfgx, &g_bfgx_attr_grp);
    if (status) {
        ps_print_err("failed to create g_sysfs_hi110x_bfgx sysfs entries\n");
        goto fail_create_bfgx_group;
    }

#ifdef PLATFORM_DEBUG_ENABLE
    g_sysfs_hi110x_debug = kobject_create_and_add("hi110x_debug", NULL);
    if (g_sysfs_hi110x_debug == NULL) {
        ps_print_err("Failed to creat sysfs_hi110x_debug !!!\n ");
        goto fail_g_sysfs_hi110x_debug;
    }

    status = oal_debug_sysfs_create_group(g_sysfs_hi110x_debug, &g_hi110x_debug_attr_grp);
    if (status) {
        ps_print_err("failed to create g_sysfs_hi110x_debug sysfs entries\n");
        goto fail_create_hi110x_debug_group;
    }
#endif

    return 0;

#ifdef PLATFORM_DEBUG_ENABLE
fail_create_hi110x_debug_group:
    kobject_put(g_sysfs_hi110x_debug);
fail_g_sysfs_hi110x_debug:
#endif
    sysfs_remove_group(g_sysfs_hi110x_bfgx, &g_bfgx_attr_grp);
fail_create_bfgx_group:
    kobject_put(g_sysfs_hi110x_bfgx);
fail_g_sysfs_hi110x_bfgx:
    oal_debug_sysfs_remove_group(g_sysfs_hi110x_pmdbg, &g_pmdbg_attr_grp);
fail_create_pmdbg_group:
    kobject_put(g_sysfs_hi110x_pmdbg);
fail_g_sysfs_hisi_pmdbg:
    return -EFAULT;
}

void bfgx_user_ctrl_exit(void)
{
#ifdef PLATFORM_DEBUG_ENABLE
    oal_debug_sysfs_remove_group(g_sysfs_hi110x_debug, &g_hi110x_debug_attr_grp);
    kobject_put(g_sysfs_hi110x_debug);
#endif

    sysfs_remove_group(g_sysfs_hi110x_bfgx, &g_bfgx_attr_grp);
    kobject_put(g_sysfs_hi110x_bfgx);

    oal_debug_sysfs_remove_group(g_sysfs_hi110x_pmdbg, &g_pmdbg_attr_grp);
    kobject_put(g_sysfs_hi110x_pmdbg);
}
