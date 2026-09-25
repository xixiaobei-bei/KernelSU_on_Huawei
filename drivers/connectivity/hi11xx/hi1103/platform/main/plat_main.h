

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

/* 函数声明 */
extern oal_int32 plat_init_etc(oal_void);
extern oal_void plat_exit_etc(oal_void);

#endif
