

/* 头文件包含 */
#include "oal_main.h"
#include "oal_workqueue.h"
#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#include "oal_pci_if.h"
#endif
#include "oal_mem.h"
#include "oal_schedule.h"
#include "oal_net.h"
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#include "oal_hcc_host_if.h"
#endif
#include "oal_kernel_file.h"

#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
#ifndef WIN32
#include "plat_firmware.h"
#endif
#endif

#ifdef _PRE_MEM_TRACE
#include "mem_trace.h"
#endif
#include "securec.h"

#define OAL_MAX_TRACE_ENTRY 64

typedef struct {
    oal_spin_lock_stru st_lock;
    unsigned long aul_trace_entries[OAL_MAX_TRACE_ENTRY];
} oal_stacktrace_stru;

/* 全局变量定义 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
void __iomem *l2cache_base_etc;
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(CONFIG_STACKTRACE) && \
    (_PRE_PRODUCT_ID_HI1151 == _PRE_PRODUCT_ID) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
OAL_STATIC oal_stacktrace_stru backtrace_mgr;
#endif

/* 动态/静态DBDC，其中一个默认使能 */
oal_uint8 wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP] = { WLAN_INIT_DEVICE_RADIO_CAP };

/* 有些内核版本可能没开启CONFIG_STACKTRACE, 增加CONFIG_STACKTRACE的宏 */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && defined(CONFIG_STACKTRACE) && \
    (_PRE_PRODUCT_ID_HI1151 == _PRE_PRODUCT_ID) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0))
OAL_STATIC oal_void oal_dump_stack_str_init(oal_void)
{
    oal_spin_lock_init(&backtrace_mgr.st_lock);
}

oal_int32 oal_dump_stack_str(oal_uint8 *puc_str, oal_uint32 ul_max_size)
{
    struct stack_trace st_trace = {
        .nr_entries = 0,
        .max_entries = OAL_MAX_TRACE_ENTRY,
        .entries = backtrace_mgr.aul_trace_entries,
        .skip = 0,
    };

    oal_uint  ui_flag = 0;
    oal_int32 l_size  = 0;
    oal_int32 l_idx   = 0;
    oal_int32 l_ret;

    if (!puc_str) {
        return -1;
    }

    oal_spin_lock_irq_save(&backtrace_mgr.st_lock, &ui_flag);
    save_stack_trace(&st_trace);
    for (l_idx = 0; l_idx < st_trace.nr_entries; l_idx++) {
        if ((oal_int32)ul_max_size - l_size - 1 <= 0) {
            break;
        }

        l_ret = snprintf_s(puc_str + l_size, ul_max_size - l_size - 1, "%pF\n", (void *)st_trace.entries[l_idx]);
        if (l_ret < 0) {
            break;
        }

        l_size += l_ret;
    }
    oal_spin_unlock_irq_restore(&backtrace_mgr.st_lock, &ui_flag);
    if (l_size > 0) {
        puc_str[l_size] = 0;
    } else {
        l_size = -1;
    }

    return l_size;
}
#else
#define oal_dump_stack_str_init()
oal_int32 oal_dump_stack_str(oal_uint8 *puc_str, oal_uint32 ul_max_size)
{
    return 0;
}
#endif

/*
 * 函 数 名  : oal_main_init_etc
 * 功能描述  : OAL模块初始化总入口，包含OAL模块内部所有特性的初始化。
 * 返 回 值  : 初始化返回值，成功或失败原因
 */
oal_int32 ATTR_OAL_NO_FUNC_TRACE oal_main_init_etc(oal_void)
{
    oal_uint32 ul_rslt;

    // kernel symbol not exported, find manually
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION) && (KERNEL_VERSION(2, 6, 34) <= LINUX_VERSION_CODE) && \
    ((_PRE_TARGET_PRODUCT_TYPE_ONT == _PRE_CONFIG_TARGET_PRODUCT))
    extern struct genl_family *nl80211_fam;
    extern struct genl_multicast_group *nl80211_mlme_mcgrp;

    nl80211_fam = (struct genl_family *)kallsyms_lookup_name("nl80211_fam");
    nl80211_mlme_mcgrp = (struct genl_multicast_group *)kallsyms_lookup_name("nl80211_mlme_mcgrp");
    if (!nl80211_fam || !nl80211_mlme_mcgrp) {
        OAL_IO_PRINT("find kernel symbol failed:fam=%p mcgrp=%p\n", nl80211_fam, nl80211_mlme_mcgrp);
        return -OAL_EFAIL;
    }
#endif

#ifdef _PRE_MEM_TRACE
    mem_trace_init();
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    if (oal_conn_sysfs_root_obj_init_etc() == NULL) {
        OAL_IO_PRINT("hisi root sysfs init failed\n");
    }
#endif

/* 为了解各模块的启动时间，增加时间戳打印 */
#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && \
    defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /* 110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功 */
#else
    /* 内存池初始化 */
    ul_rslt = oal_mem_init_pool_etc();
    if (ul_rslt != OAL_SUCC) {
        OAL_IO_PRINT("oal_main_init_etc: oal_mem_init_pool_etc return error code: %d", ul_rslt);
        return -OAL_EFAIL;  //lint !e527
    }
#endif

#if (_PRE_PRODUCT_ID_HI1151 == _PRE_PRODUCT_ID)
    /* pci驱动注册 */
    ul_rslt = oal_pci_init();
    if (ul_rslt != OAL_SUCC) {
        /* 内存池卸载 */
        oal_mem_exit_etc();
        return -OAL_EFAIL;  //lint !e527
    }

    ul_rslt = oal_5115_pci_init();
    if (ul_rslt != OAL_SUCC) {
        OAL_IO_PRINT("oal_main_init_etc: oal_5115_pci_init return error code: %d", ul_rslt);
        return -OAL_EFAIL;  //lint !e527
    }
#elif ((defined(_PRE_PRODUCT_ID_HI110X_HOST)) || (defined(_PRE_PRODUCT_ID_HI110X_DEV)))

    /* 初始化: 总线上的chip数量增加1 */
    oal_bus_init_chip_num_etc();
    ul_rslt = oal_bus_inc_chip_num_etc();
    if (ul_rslt != OAL_SUCC) {
        OAL_IO_PRINT("oal_pci_probe: oal_bus_inc_chip_num_etc failed!\n");
        return -OAL_EIO;
    }
#endif

    /* 启动成功 */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)
    memset_s(past_net_device, WLAN_VAP_SUPPORT_MAX_NUM_LIMIT * OAL_SIZEOF(oal_net_device_stru *),
             0, WLAN_VAP_SUPPORT_MAX_NUM_LIMIT * OAL_SIZEOF(oal_net_device_stru *));
#endif

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
#if defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* HCC初始化 */
    if (OAL_UNLIKELY(hcc_dev_init() != OAL_SUCC)) {
        OAL_IO_PRINT("[ERROR]hcc_module_init_etc return err null\n");
        return -OAL_EFAIL;
    }
#if defined(_PRE_PLAT_FEATURE_HI110X_PCIE) && defined(CONFIG_ARCH_SD56XX)
    /* 5610 udp pcie chip test */
    hcc_enable_etc(hcc_get_110x_handler(), OAL_FALSE);
#endif
#endif

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
    if (OAL_UNLIKELY(oal_softwdt_init_etc() != OAL_SUCC)) {
        OAL_IO_PRINT("oal_softwdt_init_etc init failed!\n");
        return -OAL_EFAIL;
    }
#endif

#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
    if (OAL_UNLIKELY(oal_dft_init_etc() != OAL_SUCC)) {
        OAL_IO_PRINT("oal_dft_init_etc init failed!\n");
        return -OAL_EFAIL;
    }
#endif
#else
    OAL_IO_PRINT("gnss only version not support wifi hcc\\n");
#endif

    oal_dump_stack_str_init();

    oal_workqueue_init();

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    oal_register_syscore_ops();
#endif
    return OAL_SUCC;
}

/*
 * 函 数 名  : oal_main_exit_etc
 * 功能描述  : OAL模块卸载
 * 返 回 值  : 模块卸载返回值，成功或失败原因
 */
oal_void ATTR_OAL_NO_FUNC_TRACE oal_main_exit_etc(oal_void)
{
#ifdef _PRE_OAL_FEATURE_KEY_PROCESS_TRACE
    oal_dft_exit_etc();
#endif

#ifdef _PRE_CONFIG_HISI_CONN_SOFTWDFT
    oal_softwdt_exit_etc();
#endif

#if (_PRE_PRODUCT_ID_HI1151 == _PRE_PRODUCT_ID)

    /* pci驱动卸载 */
    oal_pci_exit();

    oal_5115_pci_exit();

#elif ((defined(_PRE_PRODUCT_ID_HI110X_HOST)) || (defined(_PRE_PRODUCT_ID_HI110X_DEV)))

    /* chip num初始化:0 */
    oal_bus_init_chip_num_etc();
#endif

#if defined(_PRE_PRODUCT_ID_HI110X_HOST) && !defined(CONFIG_HI110X_KERNEL_MODULES_BUILD_SUPPORT) && \
    defined(_PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT)
    /* 110X 驱动build in，内存池初始化上移到内核完成，保证大片内存申请成功 */
#else
    /* 内存池卸载 */
    oal_mem_exit_etc();
#endif

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
    oal_conn_sysfs_root_boot_obj_exit_etc();
    oal_conn_sysfs_root_obj_exit_etc();
#endif
    oal_workqueue_exit();

#if ((_PRE_CONFIG_TARGET_PRODUCT == _PRE_TARGET_PRODUCT_TYPE_E5) && (_PRE_TEST_MODE != _PRE_TEST_MODE_UT))
    oal_unregister_syscore_ops();
#endif

#ifdef _PRE_MEM_TRACE
    mem_trace_exit();
#endif

    return;
}

/*
 * 函 数 名  : oal_chip_get_version_etc
 * 功能描述  : 获取chip version,由平台提供总的version入口,device和host再分别调用
 */
oal_uint32 oal_chip_get_version_etc(oal_void)
{
    oal_uint32 ul_chip_ver = 0;

#if (_PRE_WLAN_REAL_CHIP == _PRE_WLAN_CHIP_SIM)

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    ul_chip_ver = WLAN_CHIP_VERSION_HI1151V100H;
#elif defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* 1102 02需要SOC提供寄存器后实现 */
    ul_chip_ver = WLAN_CHIP_VERSION_HI1151V100H;
#endif

#else /* else _PRE_WLAN_REAL_CHIP != _PRE_WLAN_CHIP_SIM */

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
    ul_chip_ver = WLAN_CHIP_VERSION_HI1151V100H;
#elif defined(_PRE_PRODUCT_ID_HI110X_DEV) || defined(_PRE_PRODUCT_ID_HI110X_HOST)
    /* 1102 02需要SOC提供寄存器后实现 */
    ul_chip_ver = WLAN_CHIP_VERSION_HI1151V100H;
#endif

#endif
    return ul_chip_ver;
}

/*
 * 函 数 名  : oal_device_check_enable_num
 * 功能描述  : 检查单chip中mac device使能个数
 * 返 回 值  : 使能device个数
 */
OAL_STATIC oal_uint8 oal_device_check_enable_num(oal_void)
{
#ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
    oal_uint8 uc_device_num = 0;
    oal_uint8 uc_device_id;

    for (uc_device_id = 0; uc_device_id < WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP; uc_device_id++) {
        if (wlan_service_device_per_chip[uc_device_id]) {
            uc_device_num++;
        }
    }
    return uc_device_num;
#else
    return WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP;
#endif  // #ifdef _PRE_PLAT_FEATURE_CUSTOMIZE
}

/*
 * 函 数 名  : oal_chip_get_version_etc
 * 功能描述  : 根据chip version获取device num
 * 输入参数  : chip version
 */
oal_uint8 oal_chip_get_device_num_etc(oal_uint32 ul_chip_ver)
{
    oal_uint8 uc_device_nums = 0;

    switch (ul_chip_ver) {
        case WLAN_CHIP_VERSION_HI1151V100H:
        case WLAN_CHIP_VERSION_HI1151V100L:
            uc_device_nums = oal_device_check_enable_num();
            break;

        default:
            uc_device_nums = 0;
            break;
    }

    return uc_device_nums;
}
/*
 * 函 数 名  : oal_board_get_service_vap_start_id
 * 功能描述  : 获取board上的业务vap其实idx
 */
oal_uint8 oal_board_get_service_vap_start_id(oal_void)
{
    oal_uint8 uc_device_num_per_chip = oal_device_check_enable_num();

    /* 配置vap个数 = mac device个数,vap idx分配先配置vap,后业务vap */
    return (oal_uint8)(WLAN_CHIP_MAX_NUM_PER_BOARD * uc_device_num_per_chip);
}

/*lint -e578*/ /*lint -e19*/
#if (_PRE_PRODUCT_ID_HI1151 == _PRE_PRODUCT_ID)
oal_module_init(oal_main_init_etc);
oal_module_exit(oal_main_exit_etc);

#endif
oal_module_symbol(oal_dump_stack_str);
oal_module_symbol(oal_chip_get_version_etc);
oal_module_symbol(oal_chip_get_device_num_etc);
oal_module_symbol(oal_board_get_service_vap_start_id);
oal_module_symbol(oal_main_init_etc);
oal_module_symbol(oal_main_exit_etc);
oal_module_symbol(l2cache_base_etc);
oal_module_symbol(wlan_service_device_per_chip);
oal_module_license("GPL");
