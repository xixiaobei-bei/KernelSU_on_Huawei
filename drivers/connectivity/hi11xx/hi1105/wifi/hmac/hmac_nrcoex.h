

#ifndef __HMAC_NRCOEX_H__
#define __HMAC_NRCOEX_H__

#include "wal_linux_netdev_ops.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_vendor.h"
#include "wal_cfg_ioctl.h"
#include "hmac_device.h"
#include "hmac_resource.h"
#include "mac_vap.h"

#ifdef __cplusplus // windows ut编译不过，后续下线清理
#if __cplusplus
extern "C" {
#endif
#endif
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_NRCOEX_H
int32_t wal_vendor_set_nrcoex_priority(oal_net_device_stru *pst_net_dev,
    oal_ifreq_stru *pst_ifr, int8_t *pc_command);
int32_t wal_vendor_get_nrcoex_info(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int8_t *pc_cmd);    
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* end of hmac_nrcoex.h */
