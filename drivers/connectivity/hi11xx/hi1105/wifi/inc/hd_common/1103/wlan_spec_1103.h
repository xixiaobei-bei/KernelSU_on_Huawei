

#ifndef __WLAN_SPEC_1103_H__
#define __WLAN_SPEC_1103_H__


/* 作为P2P GO 允许关联最大用户数 */
#ifdef _PRE_WINDOWS_SUPPORT
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_1103 8
#else
#define WLAN_P2P_GO_ASSOC_USER_MAX_NUM_1103 4
#endif


/* 支持的建立rx ba 的最大个数 */
#define WLAN_MAX_RX_BA_1103 32

/* 支持的建立tx ba 的最大个数 */
#define WLAN_MAX_TX_BA_1103 32

/* 其他BSS的广播帧上报hal vap ID */
#define WLAN_HAL_OHTER_BSS_ID_1103 14

#endif /* #ifndef __WLAN_SPEC_1103_H__ */

