#if !defined(__PRODUCT_CONFIG_H__)
#define __PRODUCT_CONFIG_H__

#ifdef CHOOSE_MODEM_USER
#ifdef MODEM_CHR_INUSER
#include "product_config_modem_chr_ptm.h"
#else
#include "product_config_modem_chr_ptm_off.h"
#endif
#endif

#include "product_config_drv.h"
#include "product_config_pam.h"
#include "product_config_gucas.h"
#include "product_config_tlps.h"
#include "product_config_audio.h"
#include "product_config_tool.h"
#include "product_config_gucnas.h"
#include "product_config_version.h"

#ifdef CHOOSE_MODEM_USER
#ifdef MODEM_FULL_DUMP_INUSER
#include "product_config_modem_full_dump.h"
#endif

#ifdef MODEM_DDR_MINI_DUMP_INUSER
#include "product_config_modem_ddr_mini_dump.h"
#endif

#ifdef MODEM_LOGCAT_INUSER
#include "product_config_modem_netlink.h"
#endif

#ifdef MODEM_LOG_FILTER_INUSER
#include "product_config_modem_log_filter.h"
#endif

#endif

#endif /*__PRODUCT_CONFIG_H__*/ 
