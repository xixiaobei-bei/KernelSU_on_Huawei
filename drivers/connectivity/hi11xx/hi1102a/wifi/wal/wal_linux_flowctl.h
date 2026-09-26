

#ifndef __WAL_LINUX_FLOWCTL_H__
#define __WAL_LINUX_FLOWCTL_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_ext_if.h"
#include "frw_ext_if.h"

#undef  THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_FLOWCTL_H

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0))
extern oal_uint16   wal_netdev_select_queue(oal_net_device_stru *pst_dev, oal_netbuf_stru *pst_buf);
#else
extern oal_uint16   wal_netdev_select_queue(
    oal_net_device_stru *pst_dev, oal_netbuf_stru *pst_buf, void *accel_priv, select_queue_fallback_t fallback);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0)) */

#endif /* end of wal_linux_flowctl.h */

