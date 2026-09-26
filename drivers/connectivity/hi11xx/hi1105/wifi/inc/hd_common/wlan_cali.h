/*
 * 版权所有 (c) 华为技术有限公司 2020-2020
 * 功能说明 : host/device 校准公共使用头文件
 */

#ifndef __WLAN_CALI_H__
#define __WLAN_CALI_H__

#if defined (_PRE_PRODUCT_ID_HI110X_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV)
#include "1103/wlan_cali_1103.h"
#endif
#if defined (_PRE_PRODUCT_ID_HI110X_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV)
#include "1105/wlan_cali_1105.h"
#endif
#if defined (_PRE_PRODUCT_ID_HI110X_HOST) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1106_DEV) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_BISHENG_DEV)
#include "1106/wlan_cali_1106.h" /* 1106/bisheng 校准数据格式相同，bisheng使用1106校准数据结构 */
#include "1106/wlan_private_1106.h"
#endif

#endif /* end of wlan_cali.h */
