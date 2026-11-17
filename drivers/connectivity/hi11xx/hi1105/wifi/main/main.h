

#ifndef __MAIN_H__
#define __MAIN_H__

#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "frw_ext_if.h"
#include "wal_linux_ioctl.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAIN_H

int32_t hi110x_host_main_init(void);
void hi110x_host_main_exit(void);

#endif
