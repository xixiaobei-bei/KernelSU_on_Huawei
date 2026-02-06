

#ifndef __HMAC_TX_SWITCH_H__
#define __HMAC_TX_SWITCH_H__

#include "mac_vap.h"
#include "frw_ext_if.h"

void hmac_set_avail_tx_ring_index(uint8_t index);
uint32_t hmac_tx_mode_switch(mac_vap_stru *mac_vap, uint32_t *params);
uint32_t hmac_tx_mode_switch_process(frw_event_mem_stru *event_mem);

#endif
