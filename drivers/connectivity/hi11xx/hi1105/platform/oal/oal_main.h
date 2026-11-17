

#ifndef __OAL_MAIN_H__
#define __OAL_MAIN_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oal_types.h"

/* 定义 */
#define WLAN_INIT_DEVICE_RADIO_CAP 3 /* WLAN使能2G_5G */

/* 全局变量声明 */
extern uint8_t g_wlan_service_device_per_chip[WLAN_SERVICE_DEVICE_MAX_NUM_PER_CHIP];

/* 函数声明 */
extern int32_t oal_main_init(void);
extern void oal_main_exit(void);
extern uint8_t oal_chip_get_device_num(uint32_t ul_chip_ver);
extern uint8_t oal_board_get_service_vap_start_id(void);

#endif /* end of oal_main */
