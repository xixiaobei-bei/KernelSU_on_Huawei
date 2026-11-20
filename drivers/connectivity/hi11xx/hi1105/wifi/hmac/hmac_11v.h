

#ifndef __HMAC_11V_H__
#define __HMAC_11V_H__

#include "oal_ext_if.h"
#include "mac_vap.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_11V_H

#ifdef _PRE_WLAN_FEATURE_11V_ENABLE

/* 目前管理帧申请内存800字节 帧自带最大长度19(frame boady)+50(url)+ N*(15(neighbor)+3(sub)+12(sub))
 * 不存在超出 修改数量限制时须注意 */
#define HMAC_MAX_BSS_NEIGHBOR_LIST 20 /* BSS Transition 帧中允许发送的最大候选AP列表集数量 */

// 11v等待帧返回的超时时间
#define HMAC_11V_WAIT_STATUS_TIMEOUT     5000 // 5000ms
#define HMAC_11V_MAX_URL_LENGTH          50   /* 携带URL字符的最大长度限制为50 */
#define HMAC_11V_TERMINATION_TSF_LENGTH  8    /* termination_tsf时间域字节长度 */
#define HMAC_11V_QUERY_FRAME_BODY_FIX    4    /* query帧帧体固定头长度 */
#define HMAC_11V_REQUEST_FRAME_BODY_FIX  7    /* query帧帧体固定头长度 */
#define HMAC_11V_RESPONSE_FRAME_BODY_FIX 5    /* response帧帧体固定头长度 */
#define HMAC_11V_PERFERMANCE_ELEMENT_LEN 1    /* perfermance ie length */
#define HMAC_11V_TERMINATION_ELEMENT_LEN 10   /* termination ie length */
#define HMAC_11V_TOKEN_MAX_VALUE         255  /* 帧发送信令的最大值 */
#define HMAC_11V_SUBELEMENT_ID_RESV      0    /* SUBELEMENT预留 ID */

#define HMAC_11V_REQUEST_DISASSOC_TIME_SCAN_ONE_CHANNEL_TIME 100  /* 去关联时间小于100ms直接回Reject */
#define HMAC_11V_REQUEST_DISASSOC_TIME_SCAN_ALL_CHANNEL_TIME 3000 /* 100ms到3s之间，仅扫描一个信道 */

#ifdef _PRE_WLAN_FEATURE_MBO
#define HMAC_11V_MBO_RE_ASSOC_DALAY_TIME_S_TO_MS 1000
#endif

/*****************************************************************************
  3 枚举定义
*****************************************************************************/
/* IEEE Std 802.11-2012 - Table 8-253 */
enum bss_trans_mgmt_status_code {
    WNM_BSS_TM_ACCEPT = 0,
    WNM_BSS_TM_REJECT_UNSPECIFIED = 1,
    WNM_BSS_TM_REJECT_INSUFFICIENT_BEACON = 2,
    WNM_BSS_TM_REJECT_INSUFFICIENT_CAPABITY = 3,
    WNM_BSS_TM_REJECT_UNDESIRED = 4,
    WNM_BSS_TM_REJECT_DELAY_REQUEST = 5,
    WNM_BSS_TM_REJECT_STA_CANDIDATE_LIST_PROVIDED = 6,
    WNM_BSS_TM_REJECT_NO_SUITABLE_CANDIDATES = 7,
    WNM_BSS_TM_REJECT_LEAVING_ESS = 8
};

/*****************************************************************************
  Neighbor Report Element子信息元素(Subelement)的 ID
  820.11-2012协议583页，Table 8-115—SubElement IDs
*****************************************************************************/
typedef enum {
    HMAC_NEIGH_SUB_ID_RESERVED = 0,
    HMAC_NEIGH_SUB_ID_TFS_INFO,
    HMAC_NEIGH_SUB_ID_COND_COUNTRY,
    HMAC_NEIGH_SUB_ID_BSS_CANDIDATE_PERF,
    HMAC_NEIGH_SUB_ID_TERM_DURATION,
#ifdef _PRE_WLAN_FEATURE_MBO
    HMAC_NEIGH_SUB_ID_VENDOR_SPECIFIC = 221,
#endif
    HMAC_NEIGH_SUB_ID_BUTT
} hmac_neighbor_sub_eid_enum;

/*****************************************************************************
  4 STRUCT定义
*****************************************************************************/
/* Subelement BSS Transition Termination Duration */
struct hmac_bss_term_duration {
    uint8_t uc_sub_ie_id;                                         /* subelement ID，将ID置成0 表示不存在该元素 */
    uint8_t auc_termination_tsf[HMAC_11V_TERMINATION_TSF_LENGTH]; /* BSS终止时间: TSF */
    uint16_t us_duration_min;                                     /* BSS消失时间 time: 分钟 */
    uint8_t uc_resv;                                              /* 四字节对齐用 */
} __OAL_DECLARE_PACKED;
typedef struct hmac_bss_term_duration hmac_bss_term_duration_stru;

/* 候选BSS列表集的Neighbor Report IE结构体 由于只需要用到subelement 3 4 故定义两个subelement */
struct hmac_neighbor_bss_info {
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN];    /* BSSID MAC地址 */
    uint8_t uc_opt_class;                       /* Operation Class */
    uint8_t uc_chl_num;                         /* Channel number */
    uint8_t uc_phy_type;                        /* PHY type */
    uint8_t uc_candidate_perf;                  /* perference data BSSID偏好值 */
    uint8_t uc_max_candidate_perf;              /* max perference data BSSID偏好值 */
    uint8_t valid;                             /* 是否是有效的neighbor report */
    oal_bssid_infomation_stru st_bssid_info;      /* BSSID information */
    hmac_bss_term_duration_stru st_term_duration; /* 子元素Termination duration */
#ifdef _PRE_WLAN_FEATURE_MBO
    mac_assoc_retry_delay_attr_mbo_ie_stru st_assoc_delay_attr_mbo_ie; /* 子元素MBO IE(带Assoc retry delay Attr) */
#endif
} __OAL_DECLARE_PACKED;
typedef struct hmac_neighbor_bss_info hmac_neighbor_bss_info_stru;

struct hmac_bsst_req_mode {
    uint8_t bit_candidate_list_include : 1, /* 是否包含邻居AP列表 */
              bit_abridged : 1,                     /* 1:表示没有包含在邻居列表的AP优先级设置为0 */
              bit_bss_disassoc_imminent : 1,        /* 是否即将断开STA 置0 帧的时间域全为FF FF */
              bit_termination_include : 1,          /* BSS终结时间  置0则帧体中不包含BSS Termination Duration字段 */
              bit_ess_disassoc_imminent : 1,        /* EES终止时间 */
              bit_rev : 3;
} __OAL_DECLARE_PACKED;
typedef struct hmac_bsst_req_mode hmac_bsst_req_mode_stru;

/* bss transition request帧体信息结构体 */
struct hmac_bsst_req_info {
    uint8_t dialog_token;
    hmac_bsst_req_mode_stru st_request_mode; /* request mode */
    uint16_t us_disassoc_time;             /* AP去关联STA时间 TBTTs */
    uint8_t uc_validity_interval;          /* 邻居列表有效时间 TBTTs */
    uint8_t url_len;
    uint8_t uc_bss_list_num;
    uint8_t uc_resv;                       /* 4字节对齐 */
    uint8_t *puc_session_url;              /* 要求传入字符串 最大限制为100个字符 */
    hmac_neighbor_bss_info_stru *pst_neighbor_bss_list; /* bss list的数量 最大不允许超过50个 */
    hmac_bss_term_duration_stru st_term_duration;       /* 子元素Termination duration */
} __OAL_DECLARE_PACKED;
typedef struct hmac_bsst_req_info hmac_bsst_req_info_stru;

/* bss transition response帧体信息结构体 */
struct hmac_bsst_rsp_info {
    uint8_t uc_status_code;                         /* 状态码 接收或者拒绝 */
    uint8_t uc_termination_delay;                   /* 要求AP延后终止时间:分钟 */
    uint8_t auc_target_bss_addr[WLAN_MAC_ADDR_LEN]; /* 接收切换的目标BSSID */
    uint8_t uc_chl_num;                             /* Channel number */
    uint8_t uc_bss_list_num;                        /* bss list的数量最大限制为50个 */
    int8_t c_rssi;                                  /* 当前关联AP的RSSI，非Candiate AP的RSSI */
    uint8_t us_resv;                                /* 四字节对齐 */
    hmac_neighbor_bss_info_stru *pst_neighbor_bss_list;
} __OAL_DECLARE_PACKED;
typedef struct hmac_bsst_rsp_info hmac_bsst_rsp_info_stru;


/* 将Neighbor Report IE结构体从帧数据中解析出来 */
hmac_neighbor_bss_info_stru *hmac_get_neighbor_ie(uint8_t *data, uint16_t len, uint8_t *bss_num);
/* STA 处理AP发送的bss transition request frame */
uint32_t hmac_rx_bsst_req_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user, oal_netbuf_stru *netbuf);
/* STA 封装bss transition response frame */
uint16_t hmac_encap_bsst_rsp_action(hmac_vap_stru *hmac_vap, hmac_user_stru *hmac_user,
                                    hmac_bsst_rsp_info_stru *bsst_rsp_info, oal_netbuf_stru *buffer);
/* STA 发送bss transition response frame */
uint32_t hmac_tx_bsst_rsp_action(void *pst_void1, void *pst_void2, void *pst_void3);

void hmac_11v_roam_start_worker(oal_work_stru *hmac_11v_roam_work);
uint32_t hmac_11v_roam_scan_check(hmac_vap_stru *hmac_vap);
#endif /* _PRE_WLAN_FEATURE_11V_ENABLE */

#endif /* end of hmac_11v.h */
