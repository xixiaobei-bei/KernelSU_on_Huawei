

#ifndef __OAL_LINUX_CFG80211_H__
#define __OAL_LINUX_CFG80211_H__

/* 其他头文件包含 */
#include <net/genetlink.h>
#include <net/cfg80211.h>
#include <linux/nl80211.h>

/* 宏定义 */
/* hostapd和supplicant事件上报需要用到宏 */
#define OAL_NLMSG_GOODSIZE            NLMSG_GOODSIZE
#define OAL_ETH_ALEN_SIZE             ETH_ALEN
#define OAL_NLMSG_DEFAULT_SIZE        NLMSG_DEFAULT_SIZE
#define OAL_IEEE80211_MIN_ACTION_SIZE IEEE80211_MIN_ACTION_SIZE

#define OAL_NLA_PUT_U32(skb, attrtype, value)     NLA_PUT_U32(skb, attrtype, value)
#define OAL_NLA_PUT(skb, attrtype, attrlen, data) NLA_PUT(skb, attrtype, attrlen, data)
#define OAL_NLA_PUT_U16(skb, attrtype, value)     NLA_PUT_U16(skb, attrtype, value)
#define OAL_NLA_PUT_U8(skb, attrtype, value)      NLA_PUT_U8(skb, attrtype, value)
#define OAL_NLA_PUT_FLAG(skb, attrtype)           NLA_PUT_FLAG(skb, attrtype)

typedef enum rate_info_flags oal_rate_info_flags;

/* 函数声明 */
/*
 * struct cfg80211_external_auth_params - Trigger External authentication.
 *
 * Commonly used across the external auth request and event interfaces.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
/* do nothing */
#else
struct cfg80211_external_auth_params {
    oal_nl80211_external_auth_action action;
    oal_uint8 bssid[OAL_ETH_ALEN_SIZE];
    oal_cfg80211_ssid_stru ssid;
    unsigned int key_mgmt_suite;
    oal_uint16 status;
};
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)) */
typedef struct cfg80211_external_auth_params oal_cfg80211_external_auth_stru;

extern void cfg80211_drv_mss_result(struct net_device *dev, gfp_t gfp, const u8 *buf, size_t len);

#endif /* end of oal_cfg80211.h */
