

#ifndef __WAL_LINUX_CFG80211_H__
#define __WAL_LINUX_CFG80211_H__

#include "oal_ext_if.h"
#include "oal_types.h"
#include "wal_ext_if.h"
#include "frw_ext_if.h"
#include "hmac_ext_if.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_scan.h"
#include "hmac_mgmt_join.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_WAL_LINUX_CFG80211_H

#define WAL_MAX_SCAN_TIME_PER_CHANNEL 400

#define WAL_MAX_SCAN_TIME_PER_SCAN_REQ (5 * 1000) /* wpa_s下发扫描请求，超时时间为5s，单位为ms */

/* channel index and frequence */
#define WAL_MIN_CHANNEL_2G 1
#define WAL_MAX_CHANNEL_2G 14

#define WAL_MIN_CHANNEL_5G 36
#define WAL_MAX_CHANNEL_5G 165

#define WAL_MIN_CHANNEL_6G 1
#define WAL_MAX_CHANNEL_6G 233

#define WAL_MIN_CHANNEL_4_9G 184
#define WAL_MAX_CHANNEL_4_9G 196

#define WAL_MIN_FREQ_2G (2412 + 5 * (WAL_MIN_CHANNEL_2G - 1))
#define WAL_MAX_FREQ_2G (2484)
#define WAL_MIN_FREQ_5G (5000 + 5 * (WAL_MIN_CHANNEL_5G))
#define WAL_MAX_FREQ_5G (5000 + 5 * (WAL_MAX_CHANNEL_5G))
#define WAL_MIN_FREQ_6G (5950 + 5 * WAL_MIN_CHANNEL_6G)
#define WAL_MAX_FREQ_6G (5950 + 5 * WAL_MAX_CHANNEL_6G)

/* channel nums */
#define WAL_SCAN_CHANNEL_MAX_NUM \
    ((WAL_MAX_CHANNEL_2G - WAL_MIN_CHANNEL_2G + 1) + (WAL_MAX_CHANNEL_4_9G - WAL_MIN_CHANNEL_5G + 1))

/* wiphy 结构体初始化变量 */
#define WAL_MAX_SCAN_IE_LEN 1000
/* 802.11n HT 能力掩码 */
#define IEEE80211_HT_CAP_LDPC_CODING      0x0001
#define IEEE80211_HT_CAP_SUP_WIDTH_20_40  0x0002
#define IEEE80211_HT_CAP_SM_PS            0x000C
#define IEEE80211_HT_CAP_SM_PS_SHIFT      2
#define IEEE80211_HT_CAP_GRN_FLD          0x0010
#define IEEE80211_HT_CAP_SGI_20           0x0020
#define IEEE80211_HT_CAP_SGI_40           0x0040
#define IEEE80211_HT_CAP_TX_STBC          0x0080
#define IEEE80211_HT_CAP_RX_STBC          0x0300
#define IEEE80211_HT_CAP_DELAY_BA         0x0400
#define IEEE80211_HT_CAP_MAX_AMSDU        0x0800
#define IEEE80211_HT_CAP_DSSSCCK40        0x1000
#define IEEE80211_HT_CAP_RESERVED         0x2000
#define IEEE80211_HT_CAP_40MHZ_INTOLERANT 0x4000
#define IEEE80211_HT_CAP_LSIG_TXOP_PROT   0x8000

/* 802.11ac VHT Capabilities */
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_3895             0x00000000
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991             0x00000001
#define IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454            0x00000002
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ           0x00000004
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ  0x00000008
#define IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_MASK             0x0000000C
#define IEEE80211_VHT_CAP_RXLDPC                           0x00000010
#define IEEE80211_VHT_CAP_SHORT_GI_80                      0x00000020
#define IEEE80211_VHT_CAP_SHORT_GI_160                     0x00000040
#define IEEE80211_VHT_CAP_TXSTBC                           0x00000080
#define IEEE80211_VHT_CAP_RXSTBC_1                         0x00000100
#define IEEE80211_VHT_CAP_RXSTBC_2                         0x00000200
#define IEEE80211_VHT_CAP_RXSTBC_3                         0x00000300
#define IEEE80211_VHT_CAP_RXSTBC_4                         0x00000400
#define IEEE80211_VHT_CAP_RXSTBC_MASK                      0x00000700
#define IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE            0x00000800
#define IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE            0x00001000
#define IEEE80211_VHT_CAP_BEAMFORMER_ANTENNAS_MAX          0x00006000
#define IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MAX          0x00030000
#define IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE            0x00080000
#define IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE            0x00100000
#define IEEE80211_VHT_CAP_VHT_TXOP_PS                      0x00200000
#define IEEE80211_VHT_CAP_HTC_VHT                          0x00400000
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT 23
#define IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK  \
    (7 << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT)
#define IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB 0x08000000
#define IEEE80211_VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB   0x0c000000
#define IEEE80211_VHT_CAP_RX_ANTENNA_PATTERN                0x10000000
#define IEEE80211_VHT_CAP_TX_ANTENNA_PATTERN                0x20000000

/* management */
#define IEEE80211_STYPE_ASSOC_REQ    0x0000
#define IEEE80211_STYPE_ASSOC_RESP   0x0010
#define IEEE80211_STYPE_REASSOC_REQ  0x0020
#define IEEE80211_STYPE_REASSOC_RESP 0x0030
#define IEEE80211_STYPE_PROBE_REQ    0x0040
#define IEEE80211_STYPE_PROBE_RESP   0x0050
#define IEEE80211_STYPE_BEACON       0x0080
#define IEEE80211_STYPE_ATIM         0x0090
#define IEEE80211_STYPE_DISASSOC     0x00A0
#define IEEE80211_STYPE_AUTH         0x00B0
#define IEEE80211_STYPE_DEAUTH       0x00C0
#define IEEE80211_STYPE_ACTION       0x00D0

#define WAL_COOKIE_ARRAY_SIZE    8    /* 采用8bit 的map 作为保存cookie 的索引状态 */
#define WAL_COOKIE_FULL_MASK     0xFF /* cookie全部用尽掩码 */
#define WAL_MGMT_TX_TIMEOUT_MSEC 500  
#define WAL_MGMT_TX_RETRY_CNT 8      /* WAL 发送管理帧最大重传次数 */

#define IEEE80211_FCTL_FTYPE 0x000c
#define IEEE80211_FCTL_STYPE 0x00f0
#define IEEE80211_FTYPE_MGMT 0x0000

#define WAL_GET_STATION_THRESHOLD        1000 /* 固定时间内允许一次抛事件读DMAC RSSI */
#define WAL_VOWIFI_GET_STATION_THRESHOLD 200  /* 亮屏且vowifi正在使用时 */
#define WAL_CAST_SCREEN_GET_STATION_THRESHOLD 100  /* 投屏场景上报周期 */

typedef struct cookie_arry {
    uint64_t ull_cookie;
    uint32_t record_time;
} cookie_arry_stru;

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#define RATETAB_ENT(_rate, _rateid, _flags) \
    {                                       \
        .bitrate = (_rate),                 \
        .hw_value = (_rateid),              \
        .flags = (_flags),                  \
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
#define CHAN2G(_channel, _freq, _flags) \
    {                                   \
        .band = NL80211_BAND_2GHZ,      \
        .center_freq = (_freq),         \
        .hw_value = (_channel),         \
        .flags = (_flags),              \
        .max_antenna_gain = 0,          \
        .max_power = 30,                \
    }

#define CHAN5G(_channel, _flags)                \
    {                                           \
        .band = NL80211_BAND_5GHZ,              \
        .center_freq = 5000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }

#define CHAN4_9G(_channel, _flags)              \
    {                                           \
        .band = NL80211_BAND_5GHZ,              \
        .center_freq = 4000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }
#else
#define CHAN2G(_channel, _freq, _flags) \
    {                                   \
        .band = IEEE80211_BAND_2GHZ,    \
        .center_freq = (_freq),         \
        .hw_value = (_channel),         \
        .flags = (_flags),              \
        .max_antenna_gain = 0,          \
        .max_power = 30,                \
    }

#define CHAN5G(_channel, _flags)                \
    {                                           \
        .band = IEEE80211_BAND_5GHZ,            \
        .center_freq = 5000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }

#define CHAN4_9G(_channel, _flags)              \
    {                                           \
        .band = IEEE80211_BAND_5GHZ,            \
        .center_freq = 4000 + (5 * (_channel)), \
        .hw_value = (_channel),                 \
        .flags = (_flags),                      \
        .max_antenna_gain = 0,                  \
        .max_power = 30,                        \
    }
#endif

#elif (_PRE_OS_VERSION_WIN32 == _PRE_OS_VERSION)

#define RATETAB_ENT(_rate, _rateid, _flags) \
    {                                       \
        (_flags),                           \
            (_rate),                        \
            (_rateid),                      \
    }

#define CHAN2G(_channel, _freq, _flags) \
    {                                   \
        IEEE80211_BAND_2GHZ,            \
            (_freq),                    \
            (_channel),                 \
            (_flags),                   \
            0,                          \
            30,                         \
    }

#define CHAN5G(_channel, _flags)     \
    {                                \
        IEEE80211_BAND_5GHZ,         \
            5000 + (5 * (_channel)), \
            (_channel),              \
            (_flags),                \
            0,                       \
            30,                      \
    }

#define CHAN4_9G(_channel, _flags)   \
    {                                \
        IEEE80211_BAND_5GHZ,         \
            4000 + (5 * (_channel)), \
            (_channel),              \
            (_flags),                \
            0,                       \
            30,                      \
    }

#else
error "WRONG OS VERSION"
#endif

#define WAL_MIN_RTS_THRESHOLD 256
#define WAL_MAX_RTS_THRESHOLD 0xFFFF

#define WAL_MAX_FRAG_THRESHOLD 7536
#define WAL_MIN_FRAG_THRESHOLD 256

#define WAL_MAX_WAIT_TIME 3000

/* 此处02加载ko时出现，找不到符号的错误，待后续解决 TBD */

OAL_STATIC OAL_INLINE uint32_t oal_ieee80211_is_probe_resp(uint16_t fc)
{
    return (fc & (IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) ==
           (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_PROBE_RESP);
}
static inline oal_bool_enum_uint8 wlan_vap_mode_legacy_vap(wlan_p2p_mode_enum_uint8 p2p_mode,
    wlan_vap_mode_enum_uint8 vap_mode)
{
    return ((p2p_mode == WLAN_LEGACY_VAP_MODE) && (vap_mode == WLAN_VAP_MODE_BSS_STA));
}
void wal_cfg80211_exit(void);

uint32_t wal_cfg80211_init(void);

uint32_t wal_cfg80211_init_evt_handle(frw_event_mem_stru *pst_event_mem);
uint32_t wal_cfg80211_mgmt_tx_status(frw_event_mem_stru *pst_event_mem);

uint32_t wal_cfg80211_vowifi_report(frw_event_mem_stru *pst_event_mem);

uint32_t wal_cfg80211_cac_report(frw_event_mem_stru *pst_event_mem);

uint32_t wal_roam_comp_proc_sta(frw_event_mem_stru *pst_event_mem);
#ifdef _PRE_WLAN_FEATURE_11R
uint32_t wal_ft_event_proc_sta(frw_event_mem_stru *pst_event_mem);
#endif  // _PRE_WLAN_FEATURE_11R

void wal_cfg80211_unregister_netdev(oal_net_device_stru *pst_net_dev);

uint32_t wal_del_p2p_group(mac_device_stru *pst_mac_device);

uint32_t wal_cfg80211_add_vap(mac_cfg_add_vap_param_stru *pst_add_vap_param);
#ifdef _PRE_WLAN_CHBA_MGMT
int32_t wal_chba_set_channel(oal_net_device_stru *net_dev);
#endif
void wal_cfg80211_reset_bands(uint8_t uc_dev_id);
void wal_cfg80211_save_bands(uint8_t uc_dev_id);

#ifdef _PRE_WLAN_FEATURE_M2S
uint32_t wal_cfg80211_m2s_status_report(frw_event_mem_stru *pst_event_mem);
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
uint32_t wal_cfg80211_tas_rssi_access_report(frw_event_mem_stru *pst_event_mem);
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
uint32_t wal_cfg80211_hid2d_seqnum_report(frw_event_mem_stru *pst_event_mem);
#endif
uint8_t wal_cfg80211_get_station_filter(mac_vap_stru *pst_mac_vap, uint8_t *puc_mac);

#endif /* end of wal_linux_cfg80211.h */
