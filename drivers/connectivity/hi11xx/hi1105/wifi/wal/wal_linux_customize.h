

#ifndef __WAL_LINUX_CUSTOMIZE_H__
#define __WAL_LINUX_CUSTOMIZE_H__

#include "oal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CUSTOMIZE_H

uint32_t hwifi_force_refresh_rf_params(oal_net_device_stru *pst_net_dev);

#endif
