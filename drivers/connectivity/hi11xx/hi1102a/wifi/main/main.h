

#ifndef __MAIN_H__
#define __MAIN_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_H
/*****************************************************************************
  2 宏定义
*****************************************************************************/
/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/*****************************************************************************
  4 全局变量声明
*****************************************************************************/
extern volatile oal_uint32     g_gpio_int_count;
/*****************************************************************************
  5 消息头定义
*****************************************************************************/
/*****************************************************************************
  6 消息定义
*****************************************************************************/
/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/
/*****************************************************************************
  8 UNION定义
*****************************************************************************/
/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  10 函数声明
*****************************************************************************/
#if (_PRE_MULTI_CORE_MODE_OFFLOAD_DMAC == _PRE_MULTI_CORE_MODE) && (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern oal_uint8 wlan_pm_get_custom_cali_done(void);
extern void wlan_pm_set_custom_cali_done(oal_uint8 cali_done);
#endif
extern oal_int32  wlan_pm_open(oal_void);
extern oal_uint32 wlan_pm_close(oal_void);
#if (defined(_PRE_PRODUCT_ID_HI110X_DEV))
extern oal_int32  hi1102_device_main_init(oal_void);
extern oal_void  device_main_init(oal_void);
extern oal_uint8  device_psm_main_function(oal_void);
#elif (defined(_PRE_PRODUCT_ID_HI110X_HOST))
extern oal_int32  hi1102_host_main_init(oal_void);
extern oal_void   hi1102_host_main_exit(oal_void);
#elif  (_PRE_PRODUCT_ID_HI1151==_PRE_PRODUCT_ID)
extern oal_int32  hi1151_main_init(oal_void);
extern oal_void   hi1151_main_exit(oal_void);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
