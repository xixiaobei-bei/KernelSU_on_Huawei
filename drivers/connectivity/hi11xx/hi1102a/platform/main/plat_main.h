

#ifndef __PLAT_MAIN_H__
#define __PLAT_MAIN_H__

/* 其他头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "sdt_drv.h"
#endif
#include "frw_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_PLAT_MAIN_H

extern oal_int32 plat_init(oal_void);
extern oal_void plat_exit(oal_void);
#ifdef CONFIG_HI110X_GPS_REFCLK
extern int hi_gps_plat_init(void);
extern void hi_gps_plat_exit(void);
#endif

#endif
