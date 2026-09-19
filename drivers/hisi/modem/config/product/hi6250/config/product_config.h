#if !defined(__PRODUCT_CONFIG_H__)
#define __PRODUCT_CONFIG_H__

#include "product_config_real.h"

#ifdef MODEM_FULL_DUMP_INUSER
#include "product_config_modem_full_dump.h"
#endif

#ifdef MODEM_DDR_MINI_DUMP_INUSER
#include "product_config_modem_ddr_mini_dump.h"
#endif

#endif /*__PRODUCT_CONFIG_H__*/ 
