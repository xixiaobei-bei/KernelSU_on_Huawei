

#ifndef __PLATFORM_SPEC_H__
#define __PLATFORM_SPEC_H__

/* 其他头文件包含 */
#include "oal_types.h"
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1102_HOST))
#include "platform_spec_1102.h"
#elif ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1103_HOST))
#include "platform_spec_1103.h"
#elif (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1151)
#include "platform_spec_1151.h"
#endif

#endif /* end of wlan_spec.h */
