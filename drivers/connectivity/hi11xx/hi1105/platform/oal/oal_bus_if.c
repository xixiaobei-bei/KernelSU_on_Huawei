

/* 头文件包含 */
#include "oal_ext_if.h"
#include "oal_bus_if.h"
#include "oam_ext_if.h"

/* 实际chip数量 */
OAL_STATIC uint8_t g_bus_chip_num = 0;

/*
 * 函 数 名  : oal_bus_get_chip_num
 * 功能描述  : 获取硬件总线接口个数
 * 返 回 值  : chip 个数
 */
uint8_t oal_bus_get_chip_num(void)
{
    return g_bus_chip_num;
}

/*
 * 函 数 名  : oal_bus_inc_chip_num
 * 功能描述  : 获取硬件总线接口个数
 * 返 回 值  : chip 个数
 */
uint32_t oal_bus_inc_chip_num(void)
{
    if (g_bus_chip_num < WLAN_CHIP_MAX_NUM_PER_BOARD) {
        g_bus_chip_num++;
    } else {
        oal_io_print("oal_bus_inc_chip_num FAIL: g_bus_chip_num = %d\n", g_bus_chip_num);
        return OAL_FAIL;
    }

    return OAL_SUCC;
}
/*
 * 函 数 名  : oal_bus_init_chip_num
 * 功能描述  : 初始化chip num
 */
void oal_bus_init_chip_num(void)
{
    g_bus_chip_num = 0;

    return;
}

#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
/*
 * 函 数 名  : oal_irq_affinity_init
 * 功能描述  : 中断和核绑定初始化
 */
void oal_bus_irq_affinity_init(uint8_t uc_chip_id, uint8_t uc_device_id, uint32_t ul_core_id)
{
    oal_bus_dev_stru *pst_bus_dev;

    pst_bus_dev = oal_bus_get_dev_instance(uc_chip_id, uc_device_id);
    if (pst_bus_dev == NULL) {
        return;
    }

    oal_irq_set_affinity(pst_bus_dev->st_irq_info.irq, ul_core_id);
}
#endif

/*lint -e19*/
oal_module_symbol(oal_bus_get_chip_num);

#ifdef _PRE_WLAN_FEATURE_SMP_SUPPORT
oal_module_symbol(oal_bus_irq_affinity_init);
#endif
/*lint +e19*/
