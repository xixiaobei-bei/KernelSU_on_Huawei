#if !defined(__PRODUCT_CONFIG_H__)
#define __PRODUCT_CONFIG_H__

#include "product_config_drv.h"
#include "product_config_pam.h"
#include "product_config_gucas.h"
#include "product_config_tlps.h"
#include "product_config_audio.h"
#include "product_config_tool.h"
#include "product_config_gucnas.h"
#include "product_config_version.h"
#ifndef CHOOSE_MODEM_USER
#include "product_config_drv_eng.h"
#include "product_config_pam_eng.h"
#include "product_config_gucas_eng.h"
#include "product_config_tlps_eng.h"
#include "product_config_audio_eng.h"
#include "product_config_tool_eng.h"
#include "product_config_gucnas_eng.h"
#include "product_config_version_eng.h"
#endif



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

#ifdef MODEM_CHR_INUSER
#include "product_config_modem_chr_ptm.h"
#else
#include "product_config_modem_chr_ptm_off.h"
#endif

#ifdef MODEM_PM_OM_INUSER
#include "product_config_modem_pm_om.h"
#endif
#endif

#endif /*__PRODUCT_CONFIG_H__*/ 
