

#ifndef __HMAC_CALI_MGMT_DEBUG_H__
#define __HMAC_CALI_MGMT_DEBUG_H__

/* 1 其他头文件包含 */
#include "frw_ext_if.h"

#include "hmac_vap.h"
#include "plat_cali.h"
#include "wlan_spec.h"
#include "wlan_cali.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CALI_MGMT_DEBUG_H

void hmac_dump_1106_cali_result(void);
void hmac_dump_cali_result(void);
void hmac_dump_1105_cali_result(void);

#endif
