

#ifndef __HMAC_UAPSD_H__
#define __HMAC_UAPSD_H__

#include "mac_user.h"
#include "hmac_ext_if.h"

#include "hmac_user.h"
#include "hmac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_UAPSD_H

void hmac_uapsd_update_user_para(hmac_user_stru *hmac_user, uint8_t *payload, uint32_t msg_len);

#endif /* end of hmac_uapsd.h */
