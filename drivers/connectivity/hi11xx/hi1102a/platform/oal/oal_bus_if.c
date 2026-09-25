

/* 头文件包含 */
#include "oal_bus_if.h"

#include "oal_ext_if.h"
#include "oam_ext_if.h"

/* 全局变量定义 */
/* 实际chip数量 */
OAL_STATIC oal_uint8 bus_chip_num = 0;
#if ((_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT))
#ifdef _PRE_WLAN_FEATURE_DOUBLE_CHIP
OAL_STATIC oal_bus_chip_stru bus_chip[WLAN_CHIP_MAX_NUM_PER_BOARD] = {{0}, {0}};
#else
OAL_STATIC oal_bus_chip_stru bus_chip[WLAN_CHIP_MAX_NUM_PER_BOARD] = {{0}};
#endif
#endif
/*
 * 函 数 名  : oal_bus_get_chip_num
 * 功能描述  : 获取硬件总线接口个数
 * 返 回 值  : chip 个数
 */
oal_uint8 oal_bus_get_chip_num(oal_void)
{
    return bus_chip_num;
}

/*
 * 函 数 名  : oal_bus_inc_chip_num
 * 功能描述  : 获取硬件总线接口个数
 * 返 回 值  : chip 个数
 */
oal_uint32 oal_bus_inc_chip_num(oal_void)
{
    if (bus_chip_num < WLAN_CHIP_MAX_NUM_PER_BOARD) {
        bus_chip_num++;
    } else {
        OAL_IO_PRINT("oal_bus_inc_chip_num FAIL: bus_chip_num = %d\n", bus_chip_num);
        return OAL_FAIL;
    }

    /* WINDOWS下UT代码 */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    bus_chip[0].uc_device_num = bus_chip_num;
#endif

    return OAL_SUCC;
}

oal_void oal_bus_init_chip_num(oal_void)
{
    bus_chip_num = 0;

    /* WINDOWS下UT代码 */
#if (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION) && (_PRE_TEST_MODE == _PRE_TEST_MODE_UT)
    bus_chip[0].uc_device_num = bus_chip_num;
#endif
    return;
}

#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
/*
 * 函 数 名  : oal_bus_irq_affinity_init
 * 功能描述  : 中断和核绑定初始化
 */
oal_void oal_bus_irq_affinity_init(oal_uint8 uc_chip_id, oal_uint8 uc_device_id, oal_uint32 ul_core_id)
{
    oal_bus_dev_stru *pst_bus_dev;

    pst_bus_dev = oal_bus_get_dev_instance(uc_chip_id, uc_device_id);
    if (pst_bus_dev == OAL_PTR_NULL) {
        return;
    }

    oal_irq_set_affinity(pst_bus_dev->st_irq_info.ul_irq, ul_core_id);
}
#endif

/*lint -e19*/
oal_module_symbol(oal_bus_get_chip_num);
#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
oal_module_symbol(oal_bus_irq_affinity_init);
#endif
/*lint +e19*/
