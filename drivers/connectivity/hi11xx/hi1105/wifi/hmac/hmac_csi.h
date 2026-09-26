
#ifndef HMAC_CSI_H
#define HMAC_CSI_H

#include "hmac_user.h"
#include "hmac_vap.h"
#include "host_hal_device.h"

#define FILE_LEN 128
#define BASE_YEAR 1900

#define CSI_HEDA_LEN 64

#ifdef _PRE_WLAN_FEATURE_CSI

uint32_t hmac_csi_config_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param);

void hmac_csi_handle(hal_host_location_isr_stru *ftm_csi_isr);

#endif
#endif

