

/* 1 头文件包含 */
#include "oal_ext_if.h"
#include "oam_ext_if.h"
#include "hmac_dfx.h"
#include "hmac_ext_if.h"

#include "mac_resource.h"
#include "mac_vap.h"
#include "chr_user.h"
#include "chr_errno.h"

#include "securec.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_DFX_C

/* 2 全局变量定义 */
#ifdef _PRE_WLAN_FEATURE_DFR
hmac_dfr_info_stru g_st_dfr_info; /* DFR异常复位开关 */
#endif

OAL_STATIC oam_cfg_data_stru g_ast_cfg_data[OAM_CFG_TYPE_BUTT] = {
    { OAM_CFG_TYPE_MAX_ASOC_USER, "USER_SPEC", "max_asoc_user", 31 },
};

uint32_t g_wlan_vap_max_num_per_device = 4 + 1; /* 4个AP + 1个配置vap */

/* 3 函数实现 */

int32_t oam_cfg_get_item_by_id(oam_cfg_type_enum_uint16 en_cfg_type)
{
    uint32_t loop;

    for (loop = 0; loop < OAM_CFG_TYPE_BUTT; loop++) {
        if (en_cfg_type == g_ast_cfg_data[loop].en_cfg_type) {
            break;
        }
    }

    if (loop == OAM_CFG_TYPE_BUTT) {
        oal_io_print("oam_cfg_get_item_by_id::get cfg item failed!\n");
        return -OAL_FAIL;
    }

    return g_ast_cfg_data[loop].l_val;
}


OAL_STATIC uint32_t oam_cfg_restore_all_item(int32_t al_default_cfg_data[], uint32_t u_cfg_data_len)
{
    uint32_t loop;

    for (loop = 0; loop < u_cfg_data_len / sizeof(int32_t); loop++) {
        g_ast_cfg_data[loop].l_val = al_default_cfg_data[loop];
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t oam_cfg_read_and_decrypt(uint8_t **ciphertext, uint8_t **plaintext,
                                             const uint8_t *key, uint32_t key_len)
{
    uint32_t file_size = 0;
    int32_t ret;
    oal_aes_key_stru aes_key;
    /* 获取文件大小并获取文件指针 */
    ret = oal_file_size(&file_size);
    if (ret != OAL_SUCC) {
        oal_io_print("oam_cfg_get_all_item::get file size failed!\n");
        return ret;
    }
    /* 将配置文件中的所有数据读到一个缓冲区里，此时数据是加密的 */
    *ciphertext = oal_memalloc(file_size + OAM_CFG_STR_END_SIGN_LEN);
    if (*ciphertext == NULL) {
        oal_io_print("oam_cfg_get_all_item::alloc ciphertext buf failed! load ko with default cfg!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(*ciphertext, file_size + OAM_CFG_STR_END_SIGN_LEN, 0, file_size + OAM_CFG_STR_END_SIGN_LEN);

    ret = oam_cfg_read_file_to_buf((int8_t *)(*ciphertext), file_size);
    if (ret != OAL_SUCC) {
        oal_io_print("oam_cfg_get_all_item::get cfg data from file failed! fail id-->%d\n", ret);
        oal_free(*ciphertext);
        return ret;
    }

    /* 申请明文空间，并将密文解密 */
    *plaintext = oal_memalloc(file_size + OAM_CFG_STR_END_SIGN_LEN);
    if (*plaintext == NULL) {
        oal_io_print("oam_cfg_get_all_item::alloc pc_plaintext buf failed! load ko with default cfg!\n");
        oal_free(*ciphertext);

        return OAL_ERR_CODE_PTR_NULL;
    }
    memset_s(*plaintext, file_size + OAM_CFG_STR_END_SIGN_LEN, 0, file_size + OAM_CFG_STR_END_SIGN_LEN);

    /* 解密 */
    ret = (int32_t)oal_aes_expand_key(&aes_key, key, key_len);
    if (ret != OAL_SUCC) {
        oal_free(*plaintext);
        oal_free(*ciphertext);

        return ret;
    }

    oam_cfg_decrypt_all_item(&aes_key, (int8_t *)(*ciphertext),
                             (int8_t *)(*plaintext), file_size);
    return OAL_SUCC;
}


int32_t oam_cfg_get_all_item(void)
{
    int32_t al_default_cfg_data[OAM_CFG_TYPE_BUTT] = { 0 };
    uint8_t *puc_plaintext = NULL;
    uint8_t *puc_ciphertext = NULL;
    uint32_t loop;
    int32_t l_ret;
    uint8_t i_key[OAL_AES_KEYSIZE_256] = {
        0x1, 0x2, 0x3, 0x4d, 0x56, 0x10, 0x11, 0x12,
        0x1, 0x2, 0x3, 0x4d, 0x56, 0x10, 0x11, 0x12,
        0x1, 0x2, 0x3, 0x4d, 0x56, 0x10, 0x11, 0x12,
        0x1, 0x2, 0x3, 0x4d, 0x56, 0x10, 0x11, 0x12
    };

    /* 保存默认配置，如果获取配置文件中信息的时候中间有失败的情况，则需要恢复
       前面全局配置信息，其它模块加载的时候可以按照默认配置加载
    */
    for (loop = 0; loop < OAM_CFG_TYPE_BUTT; loop++) {
        al_default_cfg_data[loop] = g_ast_cfg_data[loop].l_val;
    }

    l_ret = oam_cfg_read_and_decrypt(&puc_ciphertext, &puc_plaintext, i_key, OAL_AES_KEYSIZE_256);
    if (l_ret != OAL_SUCC) {
        return l_ret;
    }
    /* 获取配置文件中每一项的信息，保存到OAM内部结构中 */
    for (loop = 0; loop < OAM_CFG_TYPE_BUTT; loop++) {
        l_ret = oam_cfg_get_one_item((int8_t *)puc_plaintext,
                                     g_ast_cfg_data[loop].pc_section,
                                     g_ast_cfg_data[loop].pc_key,
                                     &g_ast_cfg_data[loop].l_val);
        /* 如果获取某一配置值不成功，则恢复配置项的默认值 */
        if (l_ret != OAL_SUCC) {
            oal_io_print("oam_cfg_get_all_item::get cfg item fail! loop=%d\n", loop);

            oam_cfg_restore_all_item(al_default_cfg_data, sizeof(al_default_cfg_data));
            oal_free(puc_plaintext);
            oal_free(puc_ciphertext);

            return l_ret;
        }
    }

    /* 释放缓冲区 */
    oal_free(puc_plaintext);
    oal_free(puc_ciphertext);

    return OAL_SUCC;
}


uint32_t oam_cfg_init(void)
{
    return (uint32_t)oam_cfg_get_all_item();
}

uint32_t hmac_dfx_init(void)
{
    oam_register_init_hook(OM_WIFI, oam_cfg_init);
#ifdef _PRE_CONFIG_HW_CHR
    chr_host_callback_register(hmac_get_chr_info_event_hander);
#endif
    return OAL_SUCC;
}

uint32_t hmac_dfx_exit(void)
{
#ifdef _PRE_CONFIG_HW_CHR
    chr_host_callback_unregister();
#endif
    return OAL_SUCC;
}

/**********************全局变量****************************/
/* 去关联共有7种原因(0~6),默认值设置为7，表示没有去关联触发 */
hmac_chr_disasoc_reason_stru g_hmac_chr_disasoc_reason = { 0, DMAC_DISASOC_MISC_BUTT };

/* 关联字码, 新增4中私有定义字码5200-5203 */
uint16_t g_hmac_chr_connect_code = 0;

hmac_chr_del_ba_info_stru g_hmac_chr_del_ba_info = { 0, 0, MAC_UNSPEC_REASON };

/**********************获取全局变量地址****************************/
hmac_chr_disasoc_reason_stru *hmac_chr_disasoc_reason_get_pointer(void)
{
    return &g_hmac_chr_disasoc_reason;
}

uint16_t *hmac_chr_connect_code_get_pointer(void)
{
    return &g_hmac_chr_connect_code;
}

hmac_chr_del_ba_info_stru *hmac_chr_del_ba_info_get_pointer(void)
{
    return &g_hmac_chr_del_ba_info;
}

/* 回复全部变量的初始值 */
void hmac_chr_info_clean(void)
{
    g_hmac_chr_disasoc_reason.us_user_id = 0;
    g_hmac_chr_disasoc_reason.en_disasoc_reason = DMAC_DISASOC_MISC_BUTT;
    g_hmac_chr_connect_code = 0;
    g_hmac_chr_del_ba_info.uc_ba_num = 0;
    g_hmac_chr_del_ba_info.uc_del_ba_tid = 0;
    g_hmac_chr_del_ba_info.en_del_ba_reason = MAC_UNSPEC_REASON;

    return;
}
/**********************CHR打点和获取****************************/
/* 现阶段CHR只考虑STA状态(不考虑P2P)，所以不区分vap_id */
/* 打点 */
void hmac_chr_set_disasoc_reason(uint16_t user_id, uint16_t reason_id)
{
    hmac_chr_disasoc_reason_stru *pst_disasoc_reason = NULL;

    pst_disasoc_reason = hmac_chr_disasoc_reason_get_pointer();

    pst_disasoc_reason->us_user_id = user_id;
    pst_disasoc_reason->en_disasoc_reason = (dmac_disasoc_misc_reason_enum)reason_id;

    return;
}

/* 获取 */
void hmac_chr_get_disasoc_reason(hmac_chr_disasoc_reason_stru *pst_disasoc_reason)
{
    hmac_chr_disasoc_reason_stru *pst_disasoc_reason_temp = NULL;

    pst_disasoc_reason_temp = hmac_chr_disasoc_reason_get_pointer();

    pst_disasoc_reason->us_user_id = pst_disasoc_reason_temp->us_user_id;
    pst_disasoc_reason->en_disasoc_reason = pst_disasoc_reason_temp->en_disasoc_reason;

    return;
}

void hmac_chr_set_ba_session_num(uint8_t uc_ba_num)
{
    hmac_chr_del_ba_info_stru *pst_del_ba_info = NULL;

    pst_del_ba_info = hmac_chr_del_ba_info_get_pointer();
    pst_del_ba_info->uc_ba_num = uc_ba_num;
    return;
}

/* 打点 */
/* 梳理删减聚合的流程 计数统计 */
void hmac_chr_set_del_ba_info(uint8_t uc_tid, uint16_t reason_id)
{
    hmac_chr_del_ba_info_stru *pst_del_ba_info = NULL;

    pst_del_ba_info = hmac_chr_del_ba_info_get_pointer();

    pst_del_ba_info->uc_del_ba_tid = uc_tid;
    pst_del_ba_info->en_del_ba_reason = (mac_reason_code_enum)reason_id;

    return;
}

/* 获取 */
void hmac_chr_get_del_ba_info(mac_vap_stru *pst_mac_vap,
    hmac_chr_del_ba_info_stru *pst_del_ba_reason)
{
    hmac_chr_del_ba_info_stru *pst_del_ba_info = NULL;

    pst_del_ba_info = hmac_chr_del_ba_info_get_pointer();

    pst_del_ba_reason->uc_ba_num = pst_del_ba_info->uc_ba_num;
    pst_del_ba_reason->uc_del_ba_tid = pst_del_ba_info->uc_del_ba_tid;
    pst_del_ba_reason->en_del_ba_reason = pst_del_ba_info->en_del_ba_reason;

    return;
}

void hmac_chr_set_connect_code(uint16_t connect_code)
{
    uint16_t *pus_connect_code = NULL;

    pus_connect_code = hmac_chr_connect_code_get_pointer();
    *pus_connect_code = connect_code;
    return;
}

void hmac_chr_get_connect_code(uint16_t *pus_connect_code)
{
    pus_connect_code = hmac_chr_connect_code_get_pointer();
    return;
}

void hmac_chr_get_vap_info(mac_vap_stru *pst_mac_vap, hmac_chr_vap_info_stru *pst_vap_info)
{
    mac_user_stru *pst_mac_user = NULL;
    mac_device_stru *pst_mac_device;

    pst_mac_device = mac_res_get_dev(0);

    pst_vap_info->uc_vap_state = pst_mac_vap->en_vap_state;
    pst_vap_info->uc_vap_num = pst_mac_device->uc_vap_num;
    pst_vap_info->uc_vap_rx_nss = pst_mac_vap->en_vap_rx_nss;
    pst_vap_info->uc_protocol = pst_mac_vap->en_protocol;

    /* sta 关联的AP的能力 */
    pst_mac_user = mac_res_get_mac_user(pst_mac_vap->us_assoc_vap_id);
    if (pst_mac_user != NULL) {
        pst_vap_info->uc_ap_spatial_stream_num = pst_mac_user->en_user_max_cap_nss;
        pst_vap_info->bit_ap_11ntxbf = pst_mac_user->st_cap_info.bit_11ntxbf;
        pst_vap_info->bit_ap_qos = pst_mac_user->st_cap_info.bit_qos;
        pst_vap_info->bit_ap_1024qam_cap = pst_mac_user->st_cap_info.bit_1024qam_cap;
        pst_vap_info->uc_ap_protocol_mode = pst_mac_user->en_protocol_mode;
    }

    
    pst_vap_info->bit_ampdu_active = OAL_TRUE;
    pst_vap_info->bit_amsdu_active = OAL_TRUE;
    pst_vap_info->bit_sta_11ntxbf = pst_mac_vap->st_cap_flag.bit_11ntxbf;
    pst_vap_info->bit_is_dbac_running = mac_is_dbac_running(pst_mac_device);
    pst_vap_info->bit_is_dbdc_running = mac_is_dbdc_running(pst_mac_device);

    return;
}

OAL_STATIC OAL_INLINE void hmac_chr_get_info(mac_vap_stru *mac_vap, hmac_chr_info *p_hmac_chr_info)
{
    hmac_chr_get_disasoc_reason(&p_hmac_chr_info->st_disasoc_reason);
    hmac_chr_get_del_ba_info(mac_vap, &p_hmac_chr_info->st_del_ba_info);
    hmac_chr_get_connect_code(&p_hmac_chr_info->us_connect_code);
    hmac_chr_get_vap_info(mac_vap, &p_hmac_chr_info->st_vap_info);
}
uint32_t hmac_chr_get_chip_info(uint32_t chr_event_id)
{
    uint8_t vap_index;
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_device = NULL;
    hmac_chr_info st_hmac_chr_info;
    uint16_t chr_flag = 0;

    mac_device = mac_res_get_dev(0);
    if (mac_device == NULL) {
        oam_error_log0(0, OAM_SF_ACS, "{hmac_chr_get_chip_info::pst_mac_device null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (mac_device->uc_vap_num == 0) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    for (vap_index = 0; vap_index < mac_device->uc_vap_num; vap_index++) {
        mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(mac_device->auc_vap_id[vap_index]);
        if (mac_vap == NULL) {
            continue;
        }

        if (!(mac_vap->en_vap_state == MAC_VAP_STATE_UP || mac_vap->en_vap_state == MAC_VAP_STATE_PAUSE
            || (mac_vap->en_vap_state == MAC_VAP_STATE_STA_LISTEN && mac_vap->us_user_nums > 0)
            || (mac_vap->en_vap_state == MAC_VAP_STATE_ROAMING))) {
            continue;
        }

        memset_s(&st_hmac_chr_info, sizeof(hmac_chr_info), 0, sizeof(hmac_chr_info));
#ifndef _PRE_LINUX_TEST
        if (IS_LEGACY_STA(mac_vap)) {
            chr_flag = CHR_HOST;
        } else if (IS_LEGACY_AP(mac_vap)) {
            chr_flag = CHR_HOST_AP;
        } else if (IS_P2P_CL(mac_vap) || IS_P2P_GO(mac_vap)) {
            chr_flag = CHR_HOST_P2P;
        }
#endif
        hmac_chr_get_info(mac_vap, &st_hmac_chr_info);
        oam_warning_log2(mac_vap->uc_vap_id, OAM_SF_ACS,
                         "{hmac_chr_get_chip_info::chr_event_id=[%d] chr_flag=[%d].}", chr_event_id, chr_flag);
        chr_exception_q(chr_event_id, chr_flag, (uint8_t *)(&st_hmac_chr_info), sizeof(hmac_chr_info));
    }
    /* 清除全局变量的历史值 */
    hmac_chr_info_clean();

    return OAL_SUCC;
}

uint32_t hmac_get_chr_info_event_hander(uint32_t chr_event_id)
{
    uint32_t ret;

    switch (chr_event_id) {
        case CHR_WIFI_DISCONNECT_QUERY_EVENTID:
        case CHR_WIFI_CONNECT_FAIL_QUERY_EVENTID:
        case CHR_WIFI_WEB_FAIL_QUERY_EVENTID:
        case CHR_WIFI_WEB_SLOW_QUERY_EVENTID:
            ret = hmac_chr_get_chip_info(chr_event_id);
            if (ret != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "{hmac_chr_get_chip_info::hmac_chr_get_web_fail_slow_info fail.}");
                return ret;
            }
            break;
        default:
            break;
    }

    return OAL_SUCC;
}

void hmac_chr_connect_fail_query_and_report(hmac_vap_stru *pst_hmac_vap,
    mac_status_code_enum_uint16 connet_code)
{
    mac_chr_connect_fail_report_stru st_chr_connect_fail_report = { 0 };

    if (IS_LEGACY_STA(&pst_hmac_vap->st_vap_base_info)) {
        /* 主动查询 */
        hmac_chr_set_connect_code(connet_code);
        /* 主动上报 */
#ifdef CONFIG_HW_GET_EXT_SIG
        st_chr_connect_fail_report.noise = pst_hmac_vap->station_info.noise;
        st_chr_connect_fail_report.chload = pst_hmac_vap->station_info.chload;
#endif
        st_chr_connect_fail_report.c_signal = pst_hmac_vap->station_info.signal;
        st_chr_connect_fail_report.uc_distance = pst_hmac_vap->st_station_info_extend.uc_distance;
        st_chr_connect_fail_report.uc_cca_intr = pst_hmac_vap->st_station_info_extend.uc_cca_intr;
        st_chr_connect_fail_report.snr = oal_max(pst_hmac_vap->st_station_info_extend.c_snr_ant0,
                                                 pst_hmac_vap->st_station_info_extend.c_snr_ant1);
        st_chr_connect_fail_report.us_err_code = connet_code;
        chr_exception_p(CHR_WIFI_CONNECT_FAIL_REPORT_EVENTID, (uint8_t *)(&st_chr_connect_fail_report),
                        sizeof(mac_chr_connect_fail_report_stru));
    }

    return;
}
