

#define HISI_LOG_TAG "[plat_init]"

/* 头文件包含 */
#include "plat_main.h"
#include "securec.h"
#ifdef HISI_CONN_NVE_SUPPORT
#include "hisi_conn_nve.h"
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#include "oneimage.h"
#include "plat_pm.h"
#include "chr_devs.h"
#endif
#ifdef _PRE_PLAT_FEATURE_HI110X_PCIE
#include "pcie_linux.h"
#endif
#ifdef CONFIG_HI110X_GPS_REFCLK
#include "gps_refclk_src_3.h"
#endif
#ifdef FTRACE_ENABLE
#include "ftrace.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_MAIN_C

/*
 * 函 数 名  : plat_init
 * 功能描述  : 平台初始化函数总入口
 */
int32_t plat_init(void)
{
    int32_t l_return;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef HI110X_DRV_VERSION
    oal_io_print("HI110X_DRV_VERSION: %s\r\n", HI110X_DRV_VERSION);
#endif
    if (is_my_chip() == false) {
        return OAL_SUCC;
    }
#endif

#ifdef FTRACE_ENABLE
    ftrace_hisi_init();
#endif

#ifdef CONFIG_HUAWEI_DSM
    hw_110x_register_dsm_client();
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    l_return = hi110x_board_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: hi110x_board_init fail\r\n");
        goto board_init_fail;
    }
#endif

#ifdef CONFIG_HI1102_PLAT_HW_CHR
    l_return = chr_miscdevs_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: chr_miscdev_init return error code: %d\r\n", l_return);
        goto chr_miscdevs_init_fail;
    }
#endif

    l_return = plat_exception_reset_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: plat_exception_reset_init fail\r\n");
        goto plat_exception_rst_init_fail;
    }

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    l_return = oal_wifi_platform_load_dev();
    if (l_return != OAL_SUCC) {
        oal_io_print("oal_wifi_platform_load_dev fail:%d\r\n", l_return);
        goto wifi_load_sdio_fail;
    }
#endif

#ifdef CONFIG_HWCONNECTIVITY
    l_return = hw_misc_connectivity_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("hw_misc_connectivity_init fail:%d\r\n", l_return);
        goto hw_misc_connectivity_init_fail;
    }
#endif
    l_return = oal_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: oal_main_init return error code: %d\r\n", l_return);
        goto oal_main_init_fail;
    }

    l_return = oam_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: oam_main_init return error code: %d\r\n", l_return);
        goto oam_main_init_fail;
    }

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    l_return = sdt_drv_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: sdt_drv_main_init return error code: %d\r\n", l_return);
        goto sdt_drv_main_init_fail;
    }
#endif

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    l_return = frw_main_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: frw_main_init return error code: %d\r\n", l_return);
        goto frw_main_init_fail;
    }
#endif

    l_return = low_power_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: low_power_init return error code: %d\r\n", l_return);
        goto low_power_init_fail;
    }

#ifdef CONFIG_HI110X_GPS_REFCLK
    l_return = hi_gps_plat_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: hi_gps_plat_init fail\r\n");
        goto gps_plat_init_fail;
    }
#endif

#ifdef CONFIG_HI110X_GPS_SYNC
    l_return = gnss_sync_init();
    if (l_return != OAL_SUCC) {
        oal_io_print("plat_init: gnss_sync_init fail\r\n");
        goto gps_sync_init_fail;
    }
#endif

    /* 启动完成后，输出打印 */
    oal_io_print("plat_init:: platform_main_init finish!\r\n");

    return OAL_SUCC;

#ifdef CONFIG_HI110X_GPS_SYNC
gps_sync_init_fail:
    hi_gps_plat_exit();
#endif
#ifdef CONFIG_HI110X_GPS_REFCLK
gps_plat_init_fail:
    low_power_exit();
#endif
low_power_init_fail:
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    frw_main_exit();
frw_main_init_fail:
#endif
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    sdt_drv_main_exit();
sdt_drv_main_init_fail:
#endif
    oam_main_exit();
oam_main_init_fail:
    oal_main_exit();
oal_main_init_fail:
#ifdef CONFIG_HWCONNECTIVITY
    hw_misc_connectivity_exit();
hw_misc_connectivity_init_fail:
#endif
#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    oal_wifi_platform_unload_dev();
wifi_load_sdio_fail:
#endif
    plat_exception_reset_exit();
plat_exception_rst_init_fail:

#ifdef CONFIG_HI1102_PLAT_HW_CHR
    chr_miscdevs_exit();
chr_miscdevs_init_fail:
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hi110x_board_exit();
#endif
board_init_fail:
    chr_exception_report(CHR_PLATFORM_EXCEPTION_EVENTID, CHR_SYSTEM_PLAT, CHR_LAYER_DRV,
                         CHR_PLT_DRV_EVENT_INIT, CHR_PLAT_DRV_ERROR_PLAT_INIT);

    return l_return;
}

/*
 * 函 数 名  : plat_exit
 * 功能描述  : 平台卸载函数总入口
 */
void plat_exit(void)
{
#ifdef CONFIG_HI110X_GPS_SYNC
    gnss_sync_exit();
#endif
#ifdef CONFIG_HI110X_GPS_REFCLK
    hi_gps_plat_exit();
#endif

    low_power_exit();

    frw_main_exit();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    sdt_drv_main_exit();
#endif

    oam_main_exit();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#ifdef CONFIG_HWCONNECTIVITY
    hw_misc_connectivity_exit();
#endif
#endif

    oal_main_exit();

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    hi110x_board_exit();
#endif

#ifdef CONFIG_HI1102_PLAT_HW_CHR
    chr_miscdevs_exit();
#endif

    oal_wifi_platform_unload_dev();

    plat_exception_reset_exit();
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    ini_cfg_exit();
#endif

#ifdef CONFIG_HUAWEI_DSM
    hw_110x_unregister_dsm_client();
#endif

#ifdef FTRACE_ENABLE
    ftrace_hisi_exit();
#endif
    return;
}

/*lint -e578*/ /*lint -e19*/
#if !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && \
    defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
#include "plat_cali.h"
#include "oal_kernel_file.h"

int32_t g_plat_init_flag = 0;
int32_t g_plat_init_ret;
/* built-in */
OAL_STATIC ssize_t plat_sysfs_set_init(struct kobject *dev, struct kobj_attribute *attr,
                                       const char *buf, size_t count)
{
    char mode[128]; /* sys文件允许的最大输入字符长度128 */

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%20s", mode, sizeof(mode)) != 1)) {
        oal_io_print("set value one param!\n");
        return -OAL_EINVAL;
    }

    if (sysfs_streq("init", mode)) {
        /* init */
        if (g_plat_init_flag == 0) {
            g_plat_init_ret = plat_init();
            g_plat_init_flag = 1;
        } else {
            oal_io_print("double init!\n");
        }
    } else {
        oal_io_print("invalid input:%s\n", mode);
    }

    return count;
}

OAL_STATIC ssize_t plat_sysfs_get_init(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (g_plat_init_flag == 1) {
        if (g_plat_init_ret == OAL_SUCC) {
            ret = snprintf_s(buf + ret, PAGE_SIZE - ret, PAGE_SIZE - ret - 1, "running\n");
        } else {
            ret = snprintf_s(buf + ret, PAGE_SIZE - ret, PAGE_SIZE - ret - 1, "boot failed ret=%d\n",
                             g_plat_init_ret);
        }
    } else {
        ret = snprintf_s(buf + ret, PAGE_SIZE - ret, PAGE_SIZE - ret - 1, "uninit\n");
    }

    if (ret < 0) {
        oal_io_print("log str format err line[%d]\n", __LINE__);
    }

    return ret;
}
STATIC struct kobj_attribute g_dev_attr_plat =
    __ATTR(plat, S_IRUGO | S_IWUSR, plat_sysfs_get_init, plat_sysfs_set_init);
OAL_STATIC struct attribute *g_plat_init_sysfs_entries[] = {
    &g_dev_attr_plat.attr,
    NULL
};

OAL_STATIC struct attribute_group g_plat_init_attribute_group = {
    .attrs = g_plat_init_sysfs_entries,
};

int32_t plat_sysfs_init(void)
{
    int32_t ret;
    uint32_t ul_rslt;
    oal_kobject *pst_root_boot_object = NULL;

    if ((is_hisi_chiptype(BOARD_VERSION_HI1103) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_HI1105) == false) &&
        (is_hisi_chiptype(BOARD_VERSION_HI1106) == false)) {
        return OAL_SUCC;
    }

    /* 110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功 */
    ul_rslt = oal_mem_init_pool();
    if (ul_rslt != OAL_SUCC) {
        oal_io_print("oal_main_init: oal_mem_init_pool return error code: %d", ul_rslt);
        return -OAL_EFAIL;
    }
    oal_io_print("mem pool init succ\n");

    /* 申请用于保存校准数据的buffer */
    ret = cali_data_buf_malloc();
    if (ret != OAL_SUCC) {
        oal_io_print("alloc cali data buf fail\n");
        return -OAL_ENOMEM;
    }
    oal_io_print("cali buffer init succ\n");
#ifdef HISI_CONN_NVE_SUPPORT
    ret = hisi_conn_nve_alloc_ramdisk();
    if (ret != OAL_SUCC) {
        oal_io_print("hisi_conn_nve_alloc_ramdisk fail\n");
        cali_data_buf_free();
        return -OAL_ENOMEM;
    }
#endif
    pst_root_boot_object = oal_get_sysfs_root_boot_object();
    if (pst_root_boot_object == NULL) {
        oal_io_print("[E]get root boot sysfs object failed!\n");
        cali_data_buf_free();
#ifdef HISI_CONN_NVE_SUPPORT
        hisi_conn_nve_free_ramdisk();
#endif
        return -OAL_EBUSY;
    }

    ret = sysfs_create_group(pst_root_boot_object, &g_plat_init_attribute_group);
    if (ret) {
        oal_io_print("sysfs create plat boot group fail.ret=%d\n", ret);
        cali_data_buf_free();
#ifdef HISI_CONN_NVE_SUPPORT
        hisi_conn_nve_free_ramdisk();
#endif
        ret = -OAL_ENOMEM;
        return ret;
    }

    return ret;
}

void plat_sysfs_exit(void)
{
    /* need't exit,built-in */
#ifdef HISI_CONN_NVE_SUPPORT
    hisi_conn_nve_free_ramdisk();
#endif
    return;
}
oal_module_init(plat_sysfs_init);
oal_module_exit(plat_sysfs_exit);
#else
oal_module_init(plat_init);
oal_module_exit(plat_exit);
#endif
