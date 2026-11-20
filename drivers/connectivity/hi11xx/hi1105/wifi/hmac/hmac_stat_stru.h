
#ifndef __HMAC_STAT_STRU_H__
#define __HMAC_STAT_STRU_H__

#include "oal_types.h"
#include "oal_schedule.h"

/* HMAC device级别统计 */
typedef struct hmac_device_stat {
    oal_atomic rx_packets;
    oal_atomic rx_bytes;
    oal_atomic tx_packets;
    oal_atomic tx_bytes;

    uint32_t total_rx_packets;
    uint64_t total_rx_bytes;

    uint32_t total_tx_packets;
    uint64_t total_tx_bytes;

    uint32_t rx_dropped_misc;
    uint32_t tx_failed;
    uint32_t tx_retries;
} hmac_device_stat_stru;

/* HMAC VAP级别统计 */
typedef struct hmac_vap_stat {
    uint32_t rx_packets;
    uint32_t rx_dropped_misc;
    uint32_t tx_packets;
    uint32_t tx_failed;
    uint32_t tx_retries;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} hmac_vap_stat_stru;

/* HMAC user级别统计 */
typedef struct hmac_user_stat {
    uint32_t rx_packets;
    uint32_t rx_dropped_misc;
    uint32_t tx_packets;
    uint32_t tx_failed;
    uint32_t tx_retries;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
} hmac_user_stat_stru;

#endif /* end of hmac_stat_stru.h */

