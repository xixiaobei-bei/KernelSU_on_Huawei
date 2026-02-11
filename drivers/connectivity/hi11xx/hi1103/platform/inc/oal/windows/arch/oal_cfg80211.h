

#ifndef __OAL_WINDOWS_CFG80211_H__
#define __OAL_WINDOWS_CFG80211_H__

/* 宏定义 */
/* hostapd和supplicant事件上报需要用到宏 */
#define OAL_NLMSG_GOODSIZE            1920
#define OAL_ETH_ALEN_SIZE             6
#define OAL_NLMSG_DEFAULT_SIZE        (OAL_NLMSG_GOODSIZE - OAL_NLMSG_HDRLEN)
#define OAL_IEEE80211_MIN_ACTION_SIZE 1000

#define OAL_NLA_PUT_U32(skb, attrtype, value)
#define OAL_NLA_PUT(skb, attrtype, attrlen, data)
#define OAL_NLA_PUT_U16(skb, attrtype, value)
#define OAL_NLA_PUT_U8(skb, attrtype, value)
#define OAL_NLA_PUT_FLAG(skb, attrtype)

typedef enum rate_info_flags {
    RATE_INFO_FLAGS_MCS = BIT(0),
    RATE_INFO_FLAGS_VHT_MCS = BIT(1),
    RATE_INFO_FLAGS_40_MHZ_WIDTH = BIT(2),
    RATE_INFO_FLAGS_80_MHZ_WIDTH = BIT(3),
    RATE_INFO_FLAGS_80P80_MHZ_WIDTH = BIT(4),
    RATE_INFO_FLAGS_160_MHZ_WIDTH = BIT(5),
    RATE_INFO_FLAGS_SHORT_GI = BIT(6),
    RATE_INFO_FLAGS_60G = BIT(7),
} oal_rate_info_flags;

#define WLAN_CAPABILITY_ESS  (1 << 0)
#define WLAN_CAPABILITY_IBSS (1 << 1)

/*
 * struct cfg80211_external_auth_params - Trigger External authentication.
 * Commonly used across the external auth request and event interfaces.
 */
struct cfg80211_external_auth_params {
    enum nl80211_external_auth_action action;
    oal_uint8 bssid[OAL_ETH_ALEN_SIZE];
    oal_cfg80211_ssid_stru ssid;
    unsigned int key_mgmt_suite;
    oal_uint16 status;
};
typedef struct cfg80211_external_auth_params oal_cfg80211_external_auth_stru;

#endif /* end of oal_cfg80211.h */
