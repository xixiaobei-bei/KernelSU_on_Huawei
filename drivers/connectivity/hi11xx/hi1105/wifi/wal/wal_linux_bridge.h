

#ifndef __WAL_LINUX_BRIDGE_H__
#define __WAL_LINUX_BRIDGE_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_BRIDGE_H

extern uint8_t g_sk_pacing_shift;

oal_net_dev_tx_enum wal_bridge_vap_xmit(oal_netbuf_stru *pst_buf, oal_net_device_stru *pst_dev);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of wal_linux_bridge.h */
