

#ifndef __OAL_DFT_H__
#define __OAL_DFT_H__
#include "oal_types.h"

#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
void oal_dft_print_error_key_info(void);
void oal_dft_print_all_key_info(void);
#endif

int32_t oal_dft_init(void);
void oal_dft_exit(void);

#endif /* end of oal_dtf.h */
