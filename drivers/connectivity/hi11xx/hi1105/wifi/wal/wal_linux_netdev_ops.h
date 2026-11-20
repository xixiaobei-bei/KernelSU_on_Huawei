

#ifndef __WAL_LINUX_NETDEV_OPS_H__
#define __WAL_LINUX_NETDEV_OPS_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "oal_net.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_NETDEV_OPS_H

int32_t wal_netdev_open_ext(oal_net_device_stru *pst_net_dev);
int32_t wal_netdev_open(oal_net_device_stru *pst_net_dev, uint8_t uc_entry_flag);
int32_t wal_netdev_stop_ap(oal_net_device_stru *pst_net_dev);
int32_t wal_netdev_stop(oal_net_device_stru *pst_net_dev);
int32_t wal_net_device_ioctl(oal_net_device_stru *pst_net_dev, oal_ifreq_stru *pst_ifr, int32_t cmd);
oal_net_device_stats_stru *wal_netdev_get_stats(oal_net_device_stru *pst_net_dev);
int32_t wal_netdev_set_mac_addr(oal_net_device_stru *pst_net_dev, void *p_addr);

#endif
