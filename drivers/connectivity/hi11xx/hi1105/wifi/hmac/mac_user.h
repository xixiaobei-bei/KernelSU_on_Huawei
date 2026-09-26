

#ifndef __MAC_USER_H__
#define __MAC_USER_H__

/* 1 其他头文件包含 */
#include "oal_ext_if.h"
#include "frw_ext_if.h"
#include "wlan_spec.h"
#include "wlan_types.h"
#include "wlan_mib.h"
#include "mac_frame.h"
#include "mac_common.h"
#ifdef _PRE_WLAN_FEATURE_HIEX
#include "hiex_msg.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_MAC_USER_H

/* 2 宏定义 */
#define MAC_ACTIVE_USER_IDX_BMAP_LEN ((WLAN_ASSOC_USER_MAX_NUM >> 3) + 1) /* 活跃用户索引位图 */
#define MAC_INVALID_RA_LUT_IDX       WLAN_ASSOC_USER_MAX_NUM              /* 不可用的RA LUT IDX */

#define MAC_USER_FREED   0xff /* USER资源未申请 */
#define MAC_USER_ALLOCED 0x5a /* USER已被申请 */


typedef struct {
    uint8_t uc_rs_nrates;                      /* 个数 */
    uint8_t auc_rs_rates[WLAN_MAX_SUPP_RATES]; /* 速率 */
} mac_rate_stru;


#ifdef _PRE_WLAN_FEATURE_WMMAC
/* user结构中，ts信息的保存结果 */
typedef struct mac_ts {
    uint32_t medium_time;
    uint8_t uc_tsid;
    uint8_t uc_up;
    mac_wmmac_direction_enum_uint8 en_direction;
    mac_ts_conn_status_enum_uint8 en_ts_status;
    uint8_t uc_ts_dialog_token;
    uint8_t uc_vap_id;
    uint16_t us_mac_user_idx;
    /* 每个ts下都需要建立定时器，此处不放到hmac_user_stru */
    frw_timeout_stru st_addts_timer;
    uint8_t uc_second_flag;
    uint8_t auc_rsv[NUM_3_BYTES];
} mac_ts_stru;
#endif

#if defined(_PRE_WLAN_FEATURE_11AX)
#define MAC_USER_HE_HDL_STRU(_user)         (&(((mac_user_stru *)(_user))->st_he_hdl))
#define MAC_USER_IS_HE_USER(_user)          (MAC_USER_HE_HDL_STRU(_user)->en_he_capable)
#define MAC_USER_TX_DATA_INCLUDE_HTC(_user) ((_user)->bit_tx_data_include_htc)
#define MAC_USER_TX_DATA_INCLUDE_OM(_user)  ((_user)->bit_tx_data_include_om)
#define MAC_USER_ARP_PROBE_CLOSE_HTC(_user) ((_user)->bit_arp_probe_close_htc)
#endif

/* vht cap ie Extended nss bw support 字段枚举 */
typedef enum {
    WLAN_EXTEND_NSS_BW_SUPP0,
    WLAN_EXTEND_NSS_BW_SUPP1,
    WLAN_EXTEND_NSS_BW_SUPP2,
    WLAN_EXTEND_NSS_BW_SUPP3,

    WLAN_EXTEND_NSS_BW_SUPP_BUTT
} wlan_extend_nss_bw_supp_enum;

/* mac user结构体, hmac_user_stru和dmac_user_stru公共部分 */
typedef struct {
    /* 当前VAP工作在AP或STA模式，以下字段为user是STA或AP时公共字段，新添加字段请注意!!! */
    oal_dlist_head_stru st_user_dlist;              /* 用于用户遍历 */
    oal_dlist_head_stru st_user_hash_dlist;         /* 用于hash查找 */
    uint16_t us_user_hash_idx;                    /* 索引值(关联) */
    uint8_t auc_user_mac_addr[WLAN_MAC_ADDR_LEN]; /* user对应的MAC地址 */
    /* user对应资源池索引值; user为STA时，表示填在管理帧中的AID，值为用户的资源池索引值1~32(协议规定范围为1~2007) */
    uint16_t us_assoc_id;
    oal_bool_enum_uint8 en_is_multi_user;
    uint8_t uc_vap_id;                                /* vap ID */
    uint8_t uc_device_id;                             /* 设备ID */
    uint8_t uc_chip_id;                               /* 芯片ID */
    wlan_protocol_enum_uint8 en_protocol_mode;          /* 用户工作协议 */
    wlan_protocol_enum_uint8 en_avail_protocol_mode;    /* 用户和VAP协议模式交集, 供算法调用 */
    wlan_protocol_enum_uint8 en_cur_protocol_mode;      /* 默认值与en_avail_protocol_mode值相同, 供算法调用修改 */
    wlan_nss_enum_uint8 en_avail_num_spatial_stream;    /* Tx和Rx支持Nss的交集,供算法调用 */
    wlan_nss_enum_uint8 en_user_max_cap_nss;            /* 用户支持的最大空间流个数 */
    wlan_nss_enum_uint8 en_avail_bf_num_spatial_stream; /* 用户支持的Beamforming空间流个数 */
    oal_bool_enum_uint8 en_port_valid;                  /* 802.1X端口合法性标识 */
    uint8_t uc_is_user_alloced;                       /* 标志此user资源是否已经被申请 */

    mac_rate_stru st_avail_op_rates;           /* 用户和VAP可用的11a/b/g速率交集，供算法调用 */
    mac_user_tx_param_stru st_user_tx_info;    /* TX相关参数 */
    wlan_bw_cap_enum_uint8 en_bandwidth_cap;   /* 用户带宽能力信息 */
    wlan_bw_cap_enum_uint8 en_avail_bandwidth; /* 用户和VAP带宽能力交集,供算法调用 */
    wlan_bw_cap_enum_uint8 en_cur_bandwidth;   /* 默认值与en_avail_bandwidth相同,供算法调用修改 */

    mac_user_asoc_state_enum_uint8 en_user_asoc_state; /* 用户关联状态 */

    uint16_t us_amsdu_maxsize;                    /* 如果支持amsdu 保存协商后amsdu发送的最大值 */
    uint8_t uc_res_req_buff_threshold_exp; /* 资源请求缓存阈值指数 */
    wlan_nss_enum_uint8 en_smps_opmode_notify_nss;  /* smps/opmode 修改nss 值 */

#ifdef _PRE_WLAN_FEATURE_WMMAC
    mac_ts_stru st_ts_info[WLAN_WME_AC_BUTT]; /* 保存ts相关信息。 */
    uint8_t need_degrade[WLAN_WME_AC_BUTT];
#endif

    mac_user_cap_info_stru st_cap_info; /* user基本能力信息位 */
    mac_user_ht_hdl_stru st_ht_hdl;     /* HT capability IE和 operation IE的解析信息 */
    mac_vht_hdl_stru st_vht_hdl;        /* VHT capability IE和 operation IE的解析信息 */

#if defined(_PRE_WLAN_FEATURE_11AX)
    uint32_t bit_tx_data_include_om  : 1,             /* 发送数据含有om字段 */
               bit_tx_data_include_htc : 1,             /* 发送数据含有htc头 */
               bit_have_recv_he_rom_flag  : 1,          /* 接收到htc om修改过bw/nss */
               bit_arp_probe_close_htc    : 1,
               bit_have_send_tb_ppdu_flag : 1,          /* 对该user 发送过tb ppdu 数据 */
               bit_he_rsv              : 27;
    wlan_nss_enum_uint8 en_he_rom_nss;                  /* 收到 htc rom 修改nss值 */
    wlan_bw_cap_enum_uint8 en_he_rom_bw;                /* 收到 htc rom 修改bw值 */

    uint8_t  uc_rsv[NUM_32_BYTES];
#else
    oal_bool_enum_uint8 en_he_hdl_rom_no_use; /* rom 化位置不能改变 */
    uint8_t auc_he_hdl_rom_rsv1[NUM_28_BYTES];
    uint8_t bit_he_hdl_rom_rsv2 : 5,
              bit_he_hdl_rom_vht_present_no_use : 1, /* rom 化位置不能改变 */
              bit_he_hdl_rom_rsv3 : 2;
    uint8_t auc_he_hdl_rom_rsv4[NUM_4_BYTES];
    uint8_t uc_he_hdl_rom_bw_no_use;               /* rom 化位置不能改变 */
    uint8_t uc_he_hdl_rom_center_freq_seg0_no_use; /* rom 化位置不能改变 */
    uint8_t auc_he_hdl_rom_rsv5[NUM_2_BYTES];
#endif

    uint8_t auc_alg_priv_cap[WLAN_WME_AC_BUTT];

    uint16_t us_auth_alg_num;
    mac_key_mgmt_stru st_key_info;

#if defined(_PRE_WLAN_FEATURE_11AX)
    mac_he_hdl_stru st_he_hdl; /* HE Capability IE */
#else
    uint8_t auc_rsv[NUM_4_BYTES];
#endif
#ifdef _PRE_WLAN_FEATURE_HIEX
    mac_hiex_cap_stru st_hiex_cap;
#endif
} mac_user_stru;

#ifdef _PRE_WLAN_FEATURE_HIEX
#define MAC_USER_GET_HIEX_CAP(user)     (&((user)->st_hiex_cap))
#define MAC_USER_HIEX_ENABLED(user)     ((user)->st_hiex_cap.bit_hiex_enable)
#define MAC_USER_HIMIT_ENABLED(user)    ((user)->st_hiex_cap.bit_himit_enable)
#endif

#define MAC_USR_UAPSD_EN   0x01 // U-APSD使能
#define MAC_USR_UAPSD_TRIG 0x02 // U-APSD可以被trigger
#define MAC_USR_UAPSD_SP   0x04 // u-APSD在一个Service Period处理中

#define MAC_11I_ASSERT(_cond, _errcode) \
    do {                                \
        if (!(_cond)) {                 \
            return _errcode;            \
        }                               \
    } while (0);

#define MAC_11I_IS_PTK(en_macaddr_is_zero, en_pairwise) \
    ((OAL_TRUE != (en_macaddr_is_zero)) && (OAL_TRUE == (en_pairwise)))


OAL_STATIC OAL_INLINE uint8_t mac_user_get_ra_lut_index(uint8_t *puc_index_table,
    uint16_t us_start, uint16_t us_stop)
{
    return oal_get_lut_index(puc_index_table, WLAN_ACTIVE_USER_IDX_BMAP_LEN,
        MAC_INVALID_RA_LUT_IDX, us_start, us_stop);
}

OAL_STATIC OAL_INLINE void mac_user_del_ra_lut_index(uint8_t *puc_index_table, uint8_t uc_ra_lut_index)
{
    oal_del_lut_index(puc_index_table, uc_ra_lut_index);
}

OAL_STATIC OAL_INLINE void mac_user_set_smps_opmode_notify_nss(mac_user_stru *pst_mac_user, uint8_t uc_nss)
{
    pst_mac_user->en_smps_opmode_notify_nss = uc_nss;
}

OAL_STATIC OAL_INLINE wlan_nss_enum_uint8 mac_user_get_smps_opmode_notify_nss(mac_user_stru *pst_mac_user)
{
    return pst_mac_user->en_smps_opmode_notify_nss;
}

#if defined(_PRE_WLAN_FEATURE_11AX)
OAL_STATIC OAL_INLINE void mac_user_set_he_rom_nss(mac_user_stru *pst_mac_user, uint8_t uc_nss)
{
    pst_mac_user->en_he_rom_nss = uc_nss;
}
OAL_STATIC OAL_INLINE void mac_user_set_he_rom_bw(mac_user_stru *pst_mac_user, uint8_t uc_bw)
{
    pst_mac_user->en_he_rom_bw = uc_bw;
}
#endif
OAL_STATIC OAL_INLINE wlan_nss_enum_uint8 mac_user_get_avail_num_spatial_stream(mac_user_stru *pst_mac_user)
{
    return pst_mac_user->en_avail_num_spatial_stream;
}

/* 10 函数声明 */
uint32_t mac_user_add_wep_key(mac_user_stru *pst_mac_user, uint8_t uc_key_index,
    mac_key_params_stru *pst_key);
uint32_t mac_user_add_rsn_key(mac_user_stru *pst_mac_user, uint8_t uc_key_index,
    mac_key_params_stru *pst_key);
uint32_t mac_user_add_bip_key(mac_user_stru *pst_mac_user, uint8_t uc_key_index,
    mac_key_params_stru *pst_key);
wlan_priv_key_param_stru *mac_user_get_key(mac_user_stru *pst_mac_user, uint8_t uc_key_id);

uint32_t mac_user_init(mac_user_stru *pst_mac_user, uint16_t us_user_idx, uint8_t *puc_mac_addr,
    uint8_t uc_chip_id, uint8_t uc_device_id, uint8_t uc_vap_id);

void mac_user_set_port(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_port_valid);

void mac_user_avail_bf_num_spatial_stream(mac_user_stru *pst_mac_user, uint8_t uc_value);
void mac_user_set_avail_num_spatial_stream(mac_user_stru *pst_mac_user, uint8_t uc_value);
void mac_user_set_num_spatial_stream(mac_user_stru *pst_mac_user, uint8_t uc_value);
void mac_user_set_bandwidth_cap(mac_user_stru *pst_mac_user,
    wlan_bw_cap_enum_uint8 en_bandwidth_value);
void mac_user_set_bandwidth_info(mac_user_stru *pst_mac_user,
    wlan_bw_cap_enum_uint8 en_avail_bandwidth, wlan_bw_cap_enum_uint8 en_cur_bandwidth);
void mac_user_get_sta_cap_bandwidth(mac_user_stru *pst_mac_user,
    wlan_bw_cap_enum_uint8 *pen_bandwidth_cap);
uint32_t mac_user_update_bandwidth(mac_user_stru *pst_mac_user, wlan_bw_cap_enum_uint8 en_bwcap);
uint32_t mac_user_update_ap_bandwidth_cap(mac_user_stru *pst_mac_user);
uint32_t mac_user_update_ap_bandwidth_cap_cb(mac_user_stru *pst_mac_user);

void mac_user_set_assoc_id(mac_user_stru *pst_mac_user, uint16_t us_assoc_id);
void mac_user_set_avail_protocol_mode(mac_user_stru *pst_mac_user,
    wlan_protocol_enum_uint8 en_avail_protocol_mode);
void mac_user_set_cur_protocol_mode(mac_user_stru *pst_mac_user,
    wlan_protocol_enum_uint8 en_cur_protocol_mode);
void mac_user_set_cur_bandwidth(mac_user_stru *pst_mac_user, wlan_bw_cap_enum_uint8 en_cur_bandwidth);
void mac_user_set_protocol_mode(mac_user_stru *pst_mac_user, wlan_protocol_enum_uint8 en_protocol_mode);
void mac_user_set_asoc_state(mac_user_stru *pst_mac_user, mac_user_asoc_state_enum_uint8 en_value);
void mac_user_set_avail_op_rates(mac_user_stru *pst_mac_user, uint8_t uc_rs_nrates, uint8_t *puc_rs_rates);
void mac_user_set_vht_hdl(mac_user_stru *pst_mac_user, mac_vht_hdl_stru *pst_vht_hdl);
void mac_user_get_vht_hdl(mac_user_stru *pst_mac_user, mac_vht_hdl_stru *pst_ht_hdl);

#ifdef _PRE_WLAN_FEATURE_11AX
void mac_user_set_he_hdl(mac_user_stru *pst_mac_user, mac_he_hdl_stru *pst_he_hdl);
void mac_user_get_he_hdl(mac_user_stru *pst_mac_user, mac_he_hdl_stru *pst_he_hdl);
void mac_user_set_he_capable(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_he_capable);
oal_bool_enum_uint8 mac_user_get_he_capable(mac_user_stru *pst_mac_user);
#endif

void mac_user_set_ht_hdl(mac_user_stru *pst_mac_user, mac_user_ht_hdl_stru *pst_ht_hdl);
void mac_user_get_ht_hdl(mac_user_stru *pst_mac_user, mac_user_ht_hdl_stru *pst_ht_hdl);
void mac_user_set_ht_capable(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_ht_capable);
#ifdef _PRE_WLAN_FEATURE_SMPS
void mac_user_set_sm_power_save(mac_user_stru *pst_mac_user, uint8_t uc_sm_power_save);
#endif
void mac_user_set_pmf_active(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_pmf_active);
void mac_user_set_barker_preamble_mode(mac_user_stru *pst_mac_user,
    oal_bool_enum_uint8 en_barker_preamble_mode);
void mac_user_set_qos(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_qos_mode);
void mac_user_set_spectrum_mgmt(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_spectrum_mgmt);
void mac_user_set_apsd(mac_user_stru *pst_mac_user, oal_bool_enum_uint8 en_apsd);

void mac_user_init_key(mac_user_stru *pst_mac_user);

void mac_user_set_key(mac_user_stru *pst_multiuser, wlan_cipher_key_type_enum_uint8 en_keytype,
    wlan_ciper_protocol_type_enum_uint8 en_ciphertype, uint8_t uc_keyid);

uint32_t mac_user_update_wep_key(mac_user_stru *pst_mac_usr, uint16_t us_multi_user_idx);
oal_bool_enum_uint8 mac_addr_is_zero(const unsigned char *puc_mac);
void *mac_res_get_mac_user(uint16_t us_idx);

uint32_t mac_user_init_rom(mac_user_stru *pst_mac_user, uint16_t us_user_idx);
void mac_user_set_num_spatial_stream_160M(mac_user_stru *pst_mac_user,
    uint8_t uc_value);
uint8_t mac_user_get_sta_cap_bandwidth_11ac(wlan_channel_band_enum_uint8 en_band,
    mac_user_ht_hdl_stru *pst_mac_ht_hdl, mac_vht_hdl_stru *pst_mac_vht_hdl,
    mac_user_stru *pst_mac_user);
oal_bool_enum_uint8 mac_user_get_port(mac_user_stru *mac_user);
#endif /* end of mac_user.h */
