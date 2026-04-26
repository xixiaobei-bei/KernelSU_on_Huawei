
#ifndef __HMAC_RX_FILTER_H__
#define __HMAC_RX_FILTER_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "mac_vap.h"
#include "mac_device.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_RX_FILTER_H
extern oal_uint32 g_rx_filter_val;
extern oal_void hmac_rx_filter_init(oal_void);
extern oal_uint32 hmac_set_rx_filter_value(mac_vap_stru *pst_mac_vap);
extern oal_void hmac_rx_filter_exit(oal_void);
extern oal_bool_enum_uint8 hmac_find_is_sta_up(mac_device_stru *pst_mac_device);
extern oal_uint32 hmac_calc_up_ap_num(mac_device_stru *pst_mac_device);
extern oal_uint32 hmac_set_rx_filter_send_event(mac_vap_stru *pst_mac_vap, oal_uint32 ul_rx_filter_val);

#endif /* end of hmac_rx_filter.h */
