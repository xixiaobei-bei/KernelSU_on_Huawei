

/* 1 头文件包含 */
#include "hmac_scan.h"
#include "mac_frame_inl.h"
#include "wlan_chip_i.h"
#include "hmac_sme_sta.h"
#include "hmac_roam_scan.h"
#include "hmac_chan_mgmt.h"
#include "hmac_p2p.h"

#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
#include <hwnet/ipv4/sysctl_sniffer.h>
#endif
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
#include "hmac_hid2d.h"
#endif
#include "hmac_ht_self_cure.h"
#ifdef _PRE_WLAN_FEATURE_MBO
#include "hmac_mbo.h"
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
#include "hmac_11ax.h"
#endif
#include "hmac_11r.h"
#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_SCAN_C

/* 2 全局变量定义 */
hmac_scan_state_enum g_en_bgscan_enable_flag = HMAC_BGSCAN_ENABLE;
uint32_t g_pd_bss_expire_time = 0;
/* 等待侦听完成的额外等待时间(单位: ms)，参考wpa supplicant中侦听超时的等待时间 */
#define HMAC_P2P_LISTEN_EXTRA_WAIT_TIME 30


int32_t hmac_snprintf_hex(uint8_t *buf, int32_t buf_size, uint8_t *data, int32_t len)
{
    int32_t loop;
    uint8_t *pos = NULL;
    uint8_t *end = NULL;
    int32_t ret;

    if (buf_size <= 0) {
        return 0;
    }

    pos = buf;
    end = buf + buf_size;
    for (loop = 0; loop < len; loop++) {
        ret = snprintf_s((int8_t *)pos, (uint16_t)(end - pos), (uint16_t)(end - pos) - 1, "%02x ", data[loop]);
        if ((ret < 0) || (ret >= end - pos)) {
            buf[buf_size - 1] = '\0';
            return pos - buf;
        }

        pos += ret;
    }

    buf[buf_size - 1] = '\0';
    return pos - buf;
}


OAL_STATIC void hmac_scan_print_scan_params(mac_scan_req_stru *scan_params, mac_vap_stru *mac_vap)
{
    if (!((scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) &&
        hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH))) {
        oam_warning_log4(scan_params->uc_vap_id, OAM_SF_SCAN,
            "hmac_scan_print_scan_params::Now Scan channel_num[%d] in [%d]ms with scan_func[0x%x], and ssid_num[%d]!",
            scan_params->uc_channel_nums, scan_params->us_scan_time,
            scan_params->uc_scan_func, scan_params->uc_ssid_num);

        oam_warning_log3(scan_params->uc_vap_id, OAM_SF_SCAN,
            "hmac_scan_print_scan_params::p2p_scan:%d,max_scan_count_per_channel:%d,need back home_channel:%d!",
            scan_params->bit_is_p2p0_scan, scan_params->uc_max_scan_count_per_channel,
            scan_params->en_need_switch_back_home_channel);
    }
    return;
}

OAL_STATIC void hmac_wifi_hide_ssid(uint8_t *frame_body, uint16_t mac_frame_len)
{
    uint8_t *ssid_ie = NULL;
    uint8_t ssid_len = 0;
    uint8_t idx;

    if (frame_body == NULL) {
        return;
    }

    ssid_ie = mac_get_ssid(frame_body, mac_frame_len, &ssid_len);
    /* 保留4位，如果不足4位，则不隐藏 */
    if (ssid_ie == NULL || ssid_len < 4) {
        return;
    }

    for (idx = 2; idx < ssid_len - 2; idx++)  { // 保留前2位和后2位
        *(ssid_ie + idx) = 0x78;  // 0x78转换为ASCII码就是字符'x'
    }
}


void hmac_scan_print_scanned_bss_info(uint8_t device_id)
{
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    oal_dlist_head_stru *entry = NULL;
    mac_ieee80211_frame_stru *frame_hdr = NULL;
    uint8_t sdt_parse_hdr[MAC_80211_FRAME_LEN];
    uint8_t *tmp_mac_body_addr = NULL;
    uint8_t frame_sub_type;
    int32_t ret;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_print_scanned_bss_info::hmac_device null.}");
        return;
    }

    /* 获取指向扫描结果的管理结构体地址 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 获取锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 遍历扫描到的bss信息 */
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head)) {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        /* 仅显示新申请到的BSS帧 */
        if (scanned_bss->st_bss_dscr_info.en_new_scan_bss == OAL_TRUE) {
            /* 上报beacon和probe帧 */
            frame_hdr = (mac_ieee80211_frame_stru *)bss_dscr->auc_mgmt_buff;

            /* 将beacon中duration字段(2字节)复用为rssi以及channel,方便SDT显示 */
            ret = memcpy_s((uint8_t *)sdt_parse_hdr, MAC_80211_FRAME_LEN, (uint8_t *)frame_hdr, MAC_80211_FRAME_LEN);
            sdt_parse_hdr[BYTE_OFFSET_2] = (uint8_t)bss_dscr->c_rssi;
            sdt_parse_hdr[BYTE_OFFSET_3] = bss_dscr->st_channel.uc_chan_number;
            if (bss_dscr->mgmt_len < MAC_80211_FRAME_LEN) {
                oam_error_log1(0, OAM_SF_SCAN,
                    "{hmac_scan_print_scanned_bss_info::mgmt len[%d] invalid.}", bss_dscr->mgmt_len);
                continue;
            }
            tmp_mac_body_addr = (uint8_t *)oal_memalloc(bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
            if (oal_unlikely(tmp_mac_body_addr == NULL)) {
                oam_warning_log0(0, OAM_SF_SCAN,
                    "{hmac_scan_print_scanned_bss_info::alloc memory failed for storing tmp mac_frame_body.}");
                continue;
            }

            ret += memcpy_s(tmp_mac_body_addr, bss_dscr->mgmt_len - MAC_80211_FRAME_LEN,
                (uint8_t *)(bss_dscr->auc_mgmt_buff + MAC_80211_FRAME_LEN), bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
            if (ret != EOK) {
                oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_print_scanned_bss_info::memcpy fail!");
                oal_free(tmp_mac_body_addr);
                /* 解除锁 */
                oal_spin_unlock(&(bss_mgmt->st_lock));
                return;
            }

            frame_sub_type = mac_get_frame_type_and_subtype((uint8_t *)frame_hdr);
            if ((frame_sub_type == WLAN_FC0_SUBTYPE_BEACON) || (frame_sub_type == WLAN_FC0_SUBTYPE_PROBE_RSP)) {
                hmac_wifi_hide_ssid(tmp_mac_body_addr, bss_dscr->mgmt_len - MAC_80211_FRAME_LEN);
            }

            /* 上报beacon帧或者probe rsp帧 */
            /*lint -e416*/
            oam_report_80211_frame(BROADCAST_MACADDR, (uint8_t *)sdt_parse_hdr, MAC_80211_FRAME_LEN,
                tmp_mac_body_addr, (uint16_t)bss_dscr->mgmt_len, OAM_OTA_FRAME_DIRECTION_TYPE_RX);
            oal_free(tmp_mac_body_addr);
            /*lint +e416*/
#ifdef _PRE_WLAN_FEATURE_SNIFFER
#ifdef CONFIG_HW_SNIFFER
            proc_sniffer_write_file(NULL, 0, bss_dscr->auc_mgmt_buff, (uint16_t)bss_dscr->mgmt_len, 0);
#endif
#endif
        }
    }

    /* 解除锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return;
}


OAL_STATIC hmac_scanned_bss_info *hmac_scan_alloc_scanned_bss(uint32_t mgmt_len)
{
    hmac_scanned_bss_info *scanned_bss;

    /* 申请内存，存储扫描到的bss信息 */
    scanned_bss = oal_memalloc(sizeof(hmac_scanned_bss_info) + mgmt_len -
                               sizeof(scanned_bss->st_bss_dscr_info.auc_mgmt_buff));
    if (oal_unlikely(scanned_bss == NULL)) {
        oam_warning_log0(0, OAM_SF_SCAN,
                         "{hmac_scan_alloc_scanned_bss::alloc memory failed for storing scanned result.}");
        return NULL;
    }

    /* 为申请的内存清零 */
    memset_s(scanned_bss,
             sizeof(hmac_scanned_bss_info) + mgmt_len - sizeof(scanned_bss->st_bss_dscr_info.auc_mgmt_buff), 0,
             sizeof(hmac_scanned_bss_info) + mgmt_len - sizeof(scanned_bss->st_bss_dscr_info.auc_mgmt_buff));

    /* 初始化链表头节点指针 */
    oal_dlist_init_head(&(scanned_bss->st_dlist_head));

    return scanned_bss;
}


OAL_STATIC uint32_t hmac_scan_add_bss_to_list(hmac_scanned_bss_info *scanned_bss, hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru *bss_mgmt; /* 管理扫描结果的结构体 */

    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    scanned_bss->st_bss_dscr_info.en_new_scan_bss = OAL_TRUE;

    /* 对链表写操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 添加扫描结果到链表中，并更新扫描到的bss计数 */
    oal_dlist_add_tail(&(scanned_bss->st_dlist_head), &(bss_mgmt->st_bss_list_head));

    bss_mgmt->bss_num++;
    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return OAL_SUCC;
}


static uint32_t hmac_scan_del_bss_from_list_nolock(hmac_scanned_bss_info *scanned_bss, hmac_device_stru *hmac_device)
{
    hmac_bss_mgmt_stru *bss_mgmt; /* 管理扫描结果的结构体 */

    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 从链表中删除节点，并更新扫描到的bss计数 */
    oal_dlist_delete_entry(&(scanned_bss->st_dlist_head));

    bss_mgmt->bss_num--;

    return OAL_SUCC;
}


void hmac_scan_clean_scan(hmac_scan_stru *scan)
{
    hmac_scan_record_stru *scan_record = NULL;
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* 参数合法性检查 */
    if (scan == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_scan::pointer scan is null.}");
        return;
    }

    scan_record = &scan->st_scan_record_mgmt;

    /* 1.一定要先清除扫描到的bss信息，再进行清零处理 */
    bss_mgmt = &(scan_record->st_bss_mgmt);

    /* 对链表写操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 遍历链表，删除扫描到的bss信息 */
    while (oal_dlist_is_empty(&(bss_mgmt->st_bss_list_head)) == OAL_FALSE) {
        entry = oal_dlist_delete_head(&(bss_mgmt->st_bss_list_head));
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);

        bss_mgmt->bss_num--;

        /* 释放扫描队列里的内存 */
        oal_free(scanned_bss);
    }

    /* 对链表写操作前加锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    /* 2.其它信息清零 */
    memset_s(scan_record, sizeof(hmac_scan_record_stru), 0, sizeof(hmac_scan_record_stru));
    scan_record->en_scan_rsp_status = MAC_SCAN_STATUS_BUTT; /* 初始化扫描完成时状态码为无效值 */
    scan_record->en_vap_last_state = MAC_VAP_STATE_BUTT;    /* 必须置BUTT,否则aput停扫描会vap状态恢复错 */

    /* 3.重新初始化bss管理结果链表和锁 */
    bss_mgmt = &(scan_record->st_bss_mgmt);
    oal_dlist_init_head(&(bss_mgmt->st_bss_list_head));
    oal_spin_lock_init(&(bss_mgmt->st_lock));

    /* 4.删除扫描超时定时器 */
    if (scan->st_scan_timeout.en_is_registerd == OAL_TRUE) {
        frw_timer_immediate_destroy_timer_m(&(scan->st_scan_timeout));
    }

    return;
}


OAL_STATIC int32_t hmac_is_connected_ap_bssid(uint8_t device_id, uint8_t bssid[WLAN_MAC_ADDR_LEN])
{
    uint8_t vap_idx;
    mac_vap_stru *mac_vap = NULL;
    mac_device_stru *mac_device;

    mac_device = mac_res_get_dev(device_id);
    if (oal_unlikely(mac_device == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_is_connected_ap_bssid::mac_res_get_dev return null.}");
        return OAL_FALSE;
    }

    for (vap_idx = 0; vap_idx < mac_device->uc_vap_num; vap_idx++) {
        mac_vap = mac_res_get_mac_vap(mac_device->auc_vap_id[vap_idx]);
        if (oal_unlikely(mac_vap == NULL)) {
            oam_warning_log1(0, OAM_SF_P2P, "{hmac_is_connected_ap_bssid::mac_res_get_mac_vap fail! vap id is %d}",
                             mac_device->auc_vap_id[vap_idx]);
            continue;
        }

        if (IS_LEGACY_VAP(mac_vap) && (mac_vap->en_vap_state == MAC_VAP_STATE_UP)) {
            if (oal_memcmp(bssid, mac_vap->auc_bssid, WLAN_MAC_ADDR_LEN) == 0) {
                /* 不老化当前关联的AP */
                return OAL_TRUE;
            }
        }
    }

    return OAL_FALSE;
}


OAL_STATIC void hmac_scan_clean_expire_scanned_bss(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record)
{
    oal_dlist_head_stru *entry = NULL;
    oal_dlist_head_stru *entry_tmp = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint32_t curr_time_stamp;

    /* 参数合法性检查 */
    if (scan_record == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_clean_expire_scanned_bss::scan record is null.}");
        return;
    }

    /* 管理扫描的bss结果的结构体 */
    bss_mgmt = &(scan_record->st_bss_mgmt);

    curr_time_stamp = (uint32_t)oal_time_get_stamp_ms();

    /* 对链表写操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 遍历链表，删除上一次扫描结果中到期的bss信息 */
    oal_dlist_search_for_each_safe(entry, entry_tmp, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        bss_dscr = &(scanned_bss->st_bss_dscr_info);

        scanned_bss->st_bss_dscr_info.en_new_scan_bss = OAL_FALSE;

        if (oal_time_after32(curr_time_stamp, (bss_dscr->timestamp + HMAC_SCAN_MAX_SCANNED_BSS_EXPIRE)) == FALSE) {
            continue;
        }
        /* 产线老化使能 */
        if ((g_pd_bss_expire_time != 0) &&
            (curr_time_stamp - bss_dscr->timestamp < g_pd_bss_expire_time * HMAC_MS_TO_US)) {
            continue;
        }

        /* 不老化当前正在关联的AP */
        if (hmac_is_connected_ap_bssid(scan_record->uc_device_id, bss_dscr->auc_bssid)) {
            bss_dscr->c_rssi = hmac_vap->station_info.signal;
            continue;
        }

        /* 从链表中删除节点，并更新扫描到的bss计数 */
        oal_dlist_delete_entry(&(scanned_bss->st_dlist_head));
        bss_mgmt->bss_num--;

        /* 释放对应内存 */
        oal_free(scanned_bss);
    }

    /* 对链表写操作前加锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));
    return;
}


mac_bss_dscr_stru *hmac_scan_find_scanned_bss_dscr_by_index(uint8_t device_id, uint32_t bss_index)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;
    uint8_t loop;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev(device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::hmac_device is null.}");
        return NULL;
    }

    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    /* 对链表删操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));

    /* 如果索引大于总共扫描的bss个数，返回异常 */
    if (bss_index >= bss_mgmt->bss_num) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_find_scanned_bss_by_index::no such bss in bss list!}");

        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));
        return NULL;
    }

    loop = 0;
    /* 遍历链表，返回对应index的bss dscr信息 */
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);

        /* 相同的bss index返回 */
        if (bss_index == loop) {
            /* 解锁 */
            oal_spin_unlock(&(bss_mgmt->st_lock));
            return &(scanned_bss->st_bss_dscr_info);
        }

        loop++;
    }
    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return NULL;
}


hmac_scanned_bss_info *hmac_scan_find_scanned_bss_by_bssid(hmac_bss_mgmt_stru *bss_mgmt, uint8_t *bssid)
{
    oal_dlist_head_stru *entry = NULL;
    hmac_scanned_bss_info *scanned_bss = NULL;

    /* 遍历链表，查找链表中是否已经存在相同bssid的bss信息 */
    oal_dlist_search_for_each(entry, &(bss_mgmt->st_bss_list_head))
    {
        scanned_bss = oal_dlist_get_entry(entry, hmac_scanned_bss_info, st_dlist_head);
        /* 相同的bssid地址 */
        if (oal_compare_mac_addr(scanned_bss->st_bss_dscr_info.auc_bssid, bssid) == 0) {
            return scanned_bss;
        }
    }

    return NULL;
}


void *hmac_scan_get_scanned_bss_by_bssid(mac_vap_stru *mac_vap, uint8_t *mac_addr)
{
    hmac_bss_mgmt_stru *bss_mgmt = NULL; /* 管理扫描的bss结果的结构体 */
    hmac_scanned_bss_info *scanned_bss_info = NULL;
    hmac_device_stru *hmac_device;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev(mac_vap->uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::hmac_device is null, dev id[%d].}",
                         mac_vap->uc_device_id);
        return NULL;
    }

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);

    oal_spin_lock(&(bss_mgmt->st_lock));

    scanned_bss_info = hmac_scan_find_scanned_bss_by_bssid(bss_mgmt, mac_addr);
    if (scanned_bss_info == NULL) {
        oam_warning_log4(mac_vap->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_get_scanned_bss_by_bssid::find the bss failed[%02X:XX:XX:%02X:%02X:%02X]}",
                         mac_addr[MAC_ADDR_0], mac_addr[MAC_ADDR_3], mac_addr[MAC_ADDR_4], mac_addr[MAC_ADDR_5]);

        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));
        return NULL;
    }

    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));

    return &(scanned_bss_info->st_bss_dscr_info);
}


OAL_STATIC OAL_INLINE void hmac_scan_update_bss_list_wmm(mac_bss_dscr_stru *bss_dscr,
    uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie = NULL;

    bss_dscr->uc_wmm_cap = OAL_FALSE;
    bss_dscr->uc_uapsd_cap = OAL_FALSE;

    ie = mac_get_wmm_ie(frame_body, frame_len);
    if (ie != NULL) {
        bss_dscr->uc_wmm_cap = OAL_TRUE;

        /* --------------------------------------------------------------------------------- */
        /* WMM Information/Parameter Element Format                                          */
        /* ---------------------------------------------------------------------------------- */
        /* EID | IE LEN | OUI | OUIType | OUISubtype | Version | QoSInfo | OUISubtype based | */
        /* --------------------------------------------------------------------------------- */
        /* 1   |   1    |  3  | 1       | 1          | 1       | 1       | ---------------- | */
        /* --------------------------------------------------------------------------------- */
        /* ie[1] IE len 不包含EID和LEN字段,获取QoSInfo，uc_ie_len必须大于7字节长度 */
        /* Check if Bit 7 is set indicating U-APSD capability */
        if ((ie[1] >= 7) && (ie[8] & BIT7)) { /* wmm ie的第8个字节是QoS info字节 */
            bss_dscr->uc_uapsd_cap = OAL_TRUE;
        }
    } else {
        ie = mac_find_ie(MAC_EID_HT_CAP, frame_body, frame_len);
        if (ie != NULL) {
            
            /* -------------------------------------------------------------- */
            /* HT capability Information/Parameter Element Format            */
            /* -------------------------------------------------------------- */
            /* EID | IE LEN |  HT capability Info |                 based   | */
            /* -------------------------------------------------------------- */
            /* 1   |   1    |         2           | ------------------------| */
            /* -------------------------------------------------------------- */
            /* ie[1] IE len 不包含EID和LEN字段,获取HT cap Info，uc_ie_len必须大于2字节长度 */
            /* ht cap的第 2,3个字节是HT capability Info信息 */
            /* Check if Bit 5 is set indicating short GI for 20M capability */
            if ((ie[1] >= 2) && (ie[2] & BIT5)) {
                bss_dscr->uc_wmm_cap = OAL_TRUE;
            }
        }
    }
}

#ifdef _PRE_WLAN_FEATURE_11D

OAL_STATIC void hmac_scan_update_bss_list_country(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie;

    ie = mac_find_ie(MAC_EID_COUNTRY, frame_body, frame_len);
    /* 国家码不存在, 全部标记为0 */
    if (ie == NULL) {
        bss_dscr->ac_country[BYTE_OFFSET_0] = 0;
        bss_dscr->ac_country[BYTE_OFFSET_1] = 0;
        bss_dscr->ac_country[BYTE_OFFSET_2] = 0;

        return;
    } else {
        bss_dscr->puc_country_ie = ie;
    }
    /* 国家码采用2个字节,IE LEN必须大于等于2 */
    if (ie[1] >= 2) {
        bss_dscr->ac_country[BYTE_OFFSET_0] = (int8_t)ie[MAC_IE_HDR_LEN];
        bss_dscr->ac_country[BYTE_OFFSET_1] = (int8_t)ie[MAC_IE_HDR_LEN + 1];
        bss_dscr->ac_country[BYTE_OFFSET_2] = 0;
    }
}
#endif
#ifdef _PRE_WLAN_FEATURE_1024QAM

OAL_STATIC void hmac_scan_update_bss_list_1024qam(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie;
    ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_HISI_1024QAM_IE, frame_body, frame_len);
    if (ie == NULL) {
        bss_dscr->en_support_1024qam = OAL_FALSE;
    } else {
        bss_dscr->en_support_1024qam = OAL_TRUE;
    }
}
#endif


OAL_STATIC void hmac_scan_update_11i(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    bss_dscr->puc_rsn_ie = mac_find_ie(MAC_EID_RSN, frame_body, (int32_t)(frame_len));
    bss_dscr->puc_wpa_ie = mac_find_vendor_ie(MAC_WLAN_OUI_MICROSOFT, MAC_OUITYPE_WPA,
                                              frame_body, (int32_t)(frame_len));
}


OAL_STATIC void hmac_scan_update_bss_list_11n(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie = NULL;
    mac_ht_opern_ac_stru *ht_op = NULL;
    uint8_t sec_chan_offset;
    wlan_bw_cap_enum_uint8 en_ht_cap_bw = WLAN_BW_CAP_20M;
    wlan_bw_cap_enum_uint8 en_ht_op_bw = WLAN_BW_CAP_20M;

    if (hmac_ht_self_cure_in_blacklist(bss_dscr->auc_bssid)) {
        return;
    }

    /* 11n */
    ie = mac_find_ie(MAC_EID_HT_CAP, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= 2)) { /* ie长度异常检查,需大于等于2 */
        /* ie[2]是HT Capabilities Info的第1个字节 */
        bss_dscr->en_ht_capable = OAL_TRUE;        /* 支持ht */
        bss_dscr->en_ht_ldpc = (ie[BYTE_OFFSET_2] & BIT0); /* 支持ldpc */
        en_ht_cap_bw = ((ie[BYTE_OFFSET_2] & BIT1) >> 1);      /* 取出支持的带宽 */
        bss_dscr->en_ht_stbc = ((ie[BYTE_OFFSET_2] & BIT7) >> NUM_7_BITS);
    }

    /* 默认20M,如果帧内容未携带HT_OPERATION则可以直接采用默认值 */
    bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_20M;

    ie = mac_find_ie(MAC_EID_HT_OPERATION, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] == sizeof(mac_ht_opern_ac_stru))) {  // 增加ie长度异常检查
        ht_op = (mac_ht_opern_ac_stru *)(ie + MAC_IE_HDR_LEN);

        /* 提取次信道偏移 */
        sec_chan_offset = ht_op->bit_secondary_chan_offset;
        bss_dscr->ht_opern_ccsf2 = ht_op->bit_chan_center_freq_seg2; /* 保存ht operation的ccsf2,用于带宽计算 */
        /* 防止ap的channel width=0, 但channel offset = 1或者3 此时以channel width为主 */
        /* ht cap 20/40 enabled && ht operation 40 enabled */
        if ((ht_op->bit_sta_chan_width != 0) && (en_ht_cap_bw > WLAN_BW_CAP_20M)) {  // cap > 20M才取channel bw
            if (sec_chan_offset == MAC_SCB) {
                bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_40MINUS;
                en_ht_op_bw = WLAN_BW_CAP_40M;
            } else if (sec_chan_offset == MAC_SCA) {
                bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_40PLUS;
                en_ht_op_bw = WLAN_BW_CAP_40M;
            }
        }
    }

    /* 将AP带宽能力取声明能力的最小值，防止AP异常发送超过带宽能力数据，造成数据不通 */
    bss_dscr->en_bw_cap = oal_min(en_ht_cap_bw, en_ht_op_bw);

    ie = mac_find_ie(MAC_EID_EXT_CAPS, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= 1)) {
        /* Extract 20/40 BSS Coexistence Management Support */
        bss_dscr->uc_coex_mgmt_supp = (ie[BYTE_OFFSET_2] & BIT0);
#ifdef _PRE_WLAN_FEATURE_11AX
        if (g_wlan_spec_cfg->feature_11ax_is_open) {
            /* 查看 BSS Multi BSS支持情况 */
            if (ie[1] > 2) { /* 2: ie长度检查 */
                bss_dscr->st_mbssid_info.bit_ext_cap_multi_bssid_activated = ((ie[BYTE_OFFSET_4] & BIT6) >> NUM_6_BITS);
            }
        }
#endif
    }
}

OAL_STATIC void hmac_scan_update_bss_list_11ac_vht_cap(mac_bss_dscr_stru *bss_dscr,
    uint8_t *frame_body, uint16_t frame_len, uint32_t en_is_vendor_ie, uint8_t vendor_subtype)
{
    uint8_t *ie = NULL;
    oal_bool_enum_uint8 en_vht_capable;
    uint8_t supp_ch_width;

    ie = mac_find_ie(MAC_EID_VHT_CAP, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= MAC_VHT_CAP_IE_LEN)) {
        en_vht_capable = ((ie[BYTE_OFFSET_2] != 0) || (ie[BYTE_OFFSET_3] != 0) || (ie[BYTE_OFFSET_4] != 0) ||
            (ie[BYTE_OFFSET_5] != 0) || (ie[BYTE_OFFSET_6] != 0xff) || (ie[BYTE_OFFSET_7] != 0xff));
        if (en_vht_capable == OAL_TRUE) {
            bss_dscr->en_vht_capable = OAL_TRUE; /* 支持vht */
        }

        /* 说明vendor中携带VHT ie，则设置标志位，assoc req中也需携带vendor+vht ie */
        if (en_is_vendor_ie == OAL_TRUE) {
            bss_dscr->en_vendor_vht_capable = OAL_TRUE;
        }

        /* 提取Supported Channel Width Set */
        supp_ch_width = ((ie[BYTE_OFFSET_2] & (BIT3 | BIT2)) >> NUM_2_BITS);
        bss_dscr->supported_channel_width = supp_ch_width;
        bss_dscr->extend_nss_bw_supp = (ie[BYTE_OFFSET_5] & (BIT6 | BIT7)) >> BIT_OFFSET_6;
        if (supp_ch_width == 0) {                   /* 0: 80MHz */
            bss_dscr->en_bw_cap = WLAN_BW_CAP_80M;
        } else if (supp_ch_width == 1) {            /* 1: 160MHz */
            bss_dscr->en_bw_cap = WLAN_BW_CAP_160M;
        } else if (supp_ch_width == 2) {            /* 2: 80MHz */
            bss_dscr->en_bw_cap = WLAN_BW_CAP_80M;
        }
    } else {
        /* 私有vendor中不包含vht ie，适配BCM 5g 20M 私有协议 */
        if (en_is_vendor_ie == OAL_TRUE) {
            bss_dscr->en_vendor_novht_capable = OAL_TRUE;
            if ((get_hi110x_subchip_type() != BOARD_VERSION_HI1103) &&
                ((vendor_subtype == MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE2) ||
                (vendor_subtype == MAC_WLAN_OUI_VENDOR_VHT_SUBTYPE4))) {
                bss_dscr->en_vendor_1024qam_capable = OAL_TRUE;
            } else {
                bss_dscr->en_vendor_1024qam_capable = OAL_FALSE;
            }
        }
    }
}
OAL_STATIC OAL_INLINE void hmac_scan_update_bss_list_11ac_80m_ext(mac_bss_dscr_stru *bss_dscr,
    uint8_t chan_center_freq)
{
    switch (chan_center_freq - bss_dscr->st_channel.uc_chan_number) {
        case CHAN_OFFSET_PLUS_6:
            /***********************************************************************
            | 主20 | 从20 | 从40       |
                        |
                        |中心频率相对于主20偏6个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSPLUS;
            break;
        case CHAN_OFFSET_MINUS_2:
            /***********************************************************************
            | 从40        | 主20 | 从20 |
                        |
                        |中心频率相对于主20偏-2个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80PLUSMINUS;
            break;
        case CHAN_OFFSET_PLUS_2:
            /***********************************************************************
            | 从20 | 主20 | 从40       |
                        |
                        |中心频率相对于主20偏2个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSPLUS;
            break;
        case CHAN_OFFSET_MINUS_6:
            /***********************************************************************
            | 从40        | 从20 | 主20 |
                        |
                        |中心频率相对于主20偏-6个信道
            ************************************************************************/
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_80MINUSMINUS;
            break;
        default:
            break;
    }
}

OAL_STATIC void hmac_scan_update_bss_list_11ac_160m(mac_bss_dscr_stru *bss_dscr, uint8_t chan_center_freq)
{
    switch (chan_center_freq - bss_dscr->st_channel.uc_chan_number) {
        case CHAN_OFFSET_PLUS_14:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSPLUS;
            break;
        case CHAN_OFFSET_PLUS_10:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSPLUS;
            break;
        case CHAN_OFFSET_PLUS_6:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSPLUS;
            break;
        case CHAN_OFFSET_PLUS_2:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSPLUS;
            break;
        case CHAN_OFFSET_MINUS_2:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSPLUSMINUS;
            break;
        case CHAN_OFFSET_MINUS_6:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSPLUSMINUS;
            break;
        case CHAN_OFFSET_MINUS_10:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160PLUSMINUSMINUS;
            break;
        case CHAN_OFFSET_MINUS_14:
            bss_dscr->en_channel_bandwidth = WLAN_BAND_WIDTH_160MINUSMINUSMINUS;
            break;
        default:
            break;
    }
}
OAL_STATIC void hmac_scan_update_bss_list_11ac_80m(mac_bss_dscr_stru *bss_dscr,
    uint8_t chan_center_freq, uint8_t chan_center_freq_1)
{
#ifdef _PRE_WLAN_FEATURE_160M
    if ((chan_center_freq_1 - chan_center_freq == CHAN_OFFSET_PLUS_8) ||
        (chan_center_freq - chan_center_freq_1 == CHAN_OFFSET_PLUS_8)) {
        hmac_scan_update_bss_list_11ac_160m(bss_dscr, chan_center_freq_1);
        return;
    }
#endif
    hmac_scan_update_bss_list_11ac_80m_ext(bss_dscr, chan_center_freq);
}

OAL_STATIC uint8_t hmac_get_vht_ccfs1(mac_bss_dscr_stru *bss_dscr, uint8_t *vht_oper_ie)
{
    uint8_t channel_center_freq_seg1;
    if ((bss_dscr->supported_channel_width == 0) && (bss_dscr->extend_nss_bw_supp != 0)) {
        channel_center_freq_seg1 = (bss_dscr->en_support_max_nss == WLAN_SINGLE_NSS) ? 0 : bss_dscr->ht_opern_ccsf2;
    } else {
        channel_center_freq_seg1 = vht_oper_ie[BYTE_OFFSET_4];
    }
    return channel_center_freq_seg1;
}

OAL_STATIC void hmac_scan_update_bss_list_11ac(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len,
    uint32_t en_is_vendor_ie, uint8_t vendor_subtype)
{
    uint8_t *ie = NULL;
    uint8_t vht_chan_width, chan_center_freq, chan_center_freq_1;

    hmac_scan_update_bss_list_11ac_vht_cap(bss_dscr, frame_body, frame_len, en_is_vendor_ie, vendor_subtype);

    ie = mac_find_ie(MAC_EID_VHT_OPERN, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= MAC_VHT_OPERN_LEN)) {
        vht_chan_width = ie[BYTE_OFFSET_2];
        chan_center_freq = ie[BYTE_OFFSET_3];
        chan_center_freq_1 = hmac_get_vht_ccfs1(bss_dscr, ie);
        /* 更新带宽信息 */
        if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_20_40) { /* 40MHz */
            /* do nothing，en_channel_bandwidth已经在HT Operation IE中获取 */
        } else if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_80) { /* 80MHz */
            hmac_scan_update_bss_list_11ac_80m(bss_dscr, chan_center_freq, chan_center_freq_1);
#ifdef _PRE_WLAN_FEATURE_160M
        } else if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_160) { /* 2: 160MHz */
            hmac_scan_update_bss_list_11ac_160m(bss_dscr, chan_center_freq);
#endif
        } else if (vht_chan_width == WLAN_MIB_VHT_OP_WIDTH_80PLUS80) { /* 3: 80+80MHz */
            hmac_scan_update_bss_list_11ac_80m_ext(bss_dscr, chan_center_freq);
        }
    }
}

void hmac_scan_update_sap_mode(mac_bss_dscr_stru *bss_dscr, uint8_t *payload, uint16_t frame_len)
{
    uint8_t *sap_ie;
    uint8_t payload_len;
    mac_sap_mode_enum sap_mode;

    /*******************************************************************************************
     ------------------------------------------------------------------------------------
     |Element ID| Length |        OUI        |OUI TYPE|SAP mode|wired  |SAP rank|rank sta|
                                                               |connect|
     ------------------------------------------------------------------------------------
     |    1     |   1    |         3         |    1   |   1    |   1   |   1   |   1   |
     ------------------------------------------------------------------------------------
     |   0xdd   |  0x08  |      0xAC853D     |   0xb0 |  0/1/2 |   0   | 0/1/2 |   0   |
     ------------------------------------------------------------------------------------
    *********************************************************************************************/
    sap_ie = mac_find_vendor_ie(MAC_HUAWEI_VENDER_IE, MAC_HISI_SAP_IE, payload, frame_len);
    if (sap_ie == NULL) {
        bss_dscr->sap_mode = MAC_SAP_OFF;
        return;
    }

    payload_len = sap_ie[1];
    if (payload_len < MAC_HISI_SAP_IE_LEN) {
        bss_dscr->sap_mode = MAC_SAP_OFF;
        oam_error_log1(0, OAM_SF_RX, "hmac_scan_update_sap_mode::payload_len[%u] err!", payload_len);
        return;
    }

    sap_mode = sap_ie[MAC_IE_HDR_LEN + MAC_OUI_LEN + MAC_OUITYPE_LEN];
    if (sap_mode == MAC_SAP_MASTER || sap_mode == MAC_SAP_SLAVE) {
        bss_dscr->sap_mode = sap_mode;
    }
}

OAL_STATIC OAL_INLINE void hmac_scan_update_hisi_cap_ie(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    mac_hisi_cap_vendor_ie_stru hisi_vap_ie;
    uint8_t ie_len;

    if (mac_get_hisi_cap_vendor_ie(frame_body, frame_len, &hisi_vap_ie, &ie_len) == OAL_SUCC) {
        if (ie_len < MAC_HISI_CAP_VENDOR_IE_LEN - MAC_IE_HDR_LEN) {
            /* 包含bit_p2p_scenes的私有IE长度为5，若不足该长度，说明对方私有IE非法 */
            return;
        }
        if (hisi_vap_ie.bit_11ax_support != OAL_FALSE && IS_CUSTOM_OPEN_11AX_SWITCH(&hmac_vap->st_vap_base_info)) {
            bss_dscr->en_he_capable  = hisi_vap_ie.bit_11ax_support;
        }
        bss_dscr->en_dcm_support = hisi_vap_ie.bit_dcm_support;
        bss_dscr->en_p2p_scenes  = hisi_vap_ie.bit_p2p_scenes;
    }
}

OAL_STATIC OAL_INLINE void hmac_scan_update_cap_ie_part1(hmac_vap_stru *hmac_vap,
    mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    /* 11ax */
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        hmac_scan_update_11ax_ie(hmac_vap, bss_dscr, frame_body, frame_len);
    }
#endif
    hmac_scan_update_hisi_cap_ie(hmac_vap, bss_dscr, frame_body, frame_len);
}



void hmac_scan_btcoex_backlist_check_by_oui(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body, uint16_t frame_len)
{
    /* 初始化为非黑名单 */
    bss_dscr->en_btcoex_blacklist_chip_oui = 0;

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_SHENZHEN, MAC_WLAN_CHIP_OUI_TYPE_SHENZHEN, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_ATHEROSC, MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_REALTEK, MAC_WLAN_CHIP_OUI_TYPE_REALTEK, frame_body, frame_len)
        != NULL ||
        mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_METALINK, MAC_WLAN_CHIP_OUI_TYPE_METALINK, frame_body, frame_len)
        != NULL) {
        bss_dscr->en_btcoex_blacklist_chip_oui |= MAC_BTCOEX_BLACKLIST_LEV0;
    }

    if (((mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_APPLE1, MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_1, frame_body, frame_len)
        != NULL) ||
        (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_APPLE1, MAC_WLAN_CHIP_OUI_TYPE_APPLE_1_2, frame_body, frame_len)
        != NULL) ||
        (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_APPLE2, MAC_WLAN_CHIP_OUI_TYPE_APPLE_2_1, frame_body, frame_len)
        != NULL)) &&
        (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM, frame_body, frame_len)
        != NULL)) {
        bss_dscr->en_btcoex_blacklist_chip_oui |= MAC_BTCOEX_BLACKLIST_LEV1;
    }
}

OAL_STATIC OAL_INLINE void hmac_scan_update_bss_by_chip_oui(mac_bss_dscr_stru *bss_dscr,
                                                            uint8_t *frame_body, uint16_t frame_len)
{
    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_ATHEROSC, MAC_WLAN_CHIP_OUI_TYPE_ATHEROSC,
        frame_body, frame_len) != NULL) {
        bss_dscr->en_atheros_chip_oui = OAL_TRUE;
    } else {
        bss_dscr->en_atheros_chip_oui = OAL_FALSE;
    }

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM,
        frame_body, frame_len) != NULL) {
        bss_dscr->en_roam_blacklist_chip_oui = OAL_TRUE;
    } else {
        bss_dscr->en_roam_blacklist_chip_oui = OAL_FALSE;
    }

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_MARVELL, MAC_WLAN_CHIP_OUI_TYPE_MARVELL,
        frame_body, frame_len) != NULL) {
        bss_dscr->en_txbf_blacklist_chip_oui = OAL_TRUE;
    } else {
        bss_dscr->en_txbf_blacklist_chip_oui = OAL_FALSE;
    }

    if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_RALINK, MAC_WLAN_CHIP_OUI_TYPE_RALINK, frame_body, frame_len) != NULL) {
        bss_dscr->en_is_tplink_oui = WLAN_AP_CHIP_OUI_RALINK;
    } else if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_BROADCOM, MAC_WLAN_CHIP_OUI_TYPE_BROADCOM, frame_body, frame_len)
        != NULL) {
        bss_dscr->en_is_tplink_oui = WLAN_AP_CHIP_OUI_BCM;
    } else if (mac_find_vendor_ie(MAC_WLAN_CHIP_OUI_QUALCOMM, MAC_WLAN_CHIP_OUI_TYPE_QUALCOMM, frame_body, frame_len)
        != NULL) {
        bss_dscr->en_is_tplink_oui = WLAN_AP_CHIP_OUI_QLM;
    }
}


OAL_STATIC OAL_INLINE void hmac_scan_update_bss_list_protocol(hmac_vap_stru *hmac_vap, mac_bss_dscr_stru *bss_dscr,
                                                              uint8_t *frame_body, uint16_t frame_len)
{
    uint8_t *ie = NULL;
    uint16_t offset_vendor_vht = MAC_WLAN_OUI_VENDOR_VHT_HEADER + MAC_IE_HDR_LEN;

    frame_body += MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    frame_len -= MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    /*************************************************************************/
    /*                       Beacon Frame - Frame Body                       */
    /* ---------------------------------------------------------------------- */
    /* |Timestamp|BcnInt|CapInfo|SSID|SupRates|DSParamSet|TIM  |CountryElem | */
    /* ---------------------------------------------------------------------- */
    /* |8        |2     |2      |2-34|3-10    |3         |6-256|8-256       | */
    /* ---------------------------------------------------------------------- */
    /* |PowerConstraint |Quiet|TPC Report|ERP |RSN  |WMM |Extended Sup Rates| */
    /* ---------------------------------------------------------------------- */
    /* |3               |8    |4         |3   |4-255|26  | 3-257            | */
    /* ---------------------------------------------------------------------- */
    /* |BSS Load |HT Capabilities |HT Operation |Overlapping BSS Scan       | */
    /* ---------------------------------------------------------------------- */
    /* |7        |28              |24           |16                         | */
    /* ---------------------------------------------------------------------- */
    /* |Extended Capabilities |                                              */
    /* ---------------------------------------------------------------------- */
    /* |3-8                   |                                              */
    /*************************************************************************/
    /* wmm */
    hmac_scan_update_bss_list_wmm(bss_dscr, frame_body, frame_len);

    /* 11i */
    hmac_scan_update_11i(bss_dscr, frame_body, frame_len);

#ifdef _PRE_WLAN_FEATURE_11D
    /* 11d */
    hmac_scan_update_bss_list_country(bss_dscr, frame_body, frame_len);
#endif

    /* 11n */
    hmac_scan_update_bss_list_11n(bss_dscr, frame_body, frame_len);

    /* rrm */
#if defined(_PRE_WLAN_FEATURE_11K) || defined(_PRE_WLAN_FEATURE_FTM)
    hmac_scan_update_bss_list_rrm(bss_dscr, frame_body, frame_len);
#endif

    /* 11ac */
    hmac_scan_update_bss_list_11ac(bss_dscr, frame_body, frame_len, OAL_FALSE, OAL_FALSE);

    hmac_scan_update_cap_ie_part1(hmac_vap, bss_dscr, frame_body, frame_len);
    /* 查找私有vendor ie */
    ie = mac_find_vendor_ie(MAC_WLAN_OUI_BROADCOM_EPIGRAM, MAC_WLAN_OUI_VENDOR_VHT_TYPE, frame_body, frame_len);
    if ((ie != NULL) && (ie[1] >= MAC_WLAN_OUI_VENDOR_VHT_HEADER)) {
        hmac_scan_update_bss_list_11ac(bss_dscr, ie + offset_vendor_vht,
                                       ie[1] - MAC_WLAN_OUI_VENDOR_VHT_HEADER, OAL_TRUE, ie[BYTE_OFFSET_6]);
    }

#ifdef _PRE_WLAN_FEATURE_1024QAM
    hmac_scan_update_bss_list_1024qam(bss_dscr, frame_body, frame_len);
#endif

#ifdef _PRE_WLAN_FEATURE_MBO
    /* 检测AP是否带含有assoc disallowed attr的MBO IE，MBO定制化开关打开才进行查找对应MBO IE,否则忽略 */
    if (hmac_vap->st_vap_base_info.st_mbo_para_info.uc_mbo_enable == OAL_TRUE) {
        hmac_scan_update_bss_assoc_disallowed_attr(bss_dscr, frame_body, frame_len);
    }
#endif

    /* SAP */
    hmac_scan_update_sap_mode(bss_dscr, frame_body, frame_len);

    hmac_scan_btcoex_backlist_check_by_oui(bss_dscr, frame_body, frame_len);

    hmac_scan_update_bss_by_chip_oui(bss_dscr, frame_body, frame_len);
}


uint8_t hmac_scan_check_bss_supp_rates(mac_device_stru *mac_dev, uint8_t *rate,
                                       uint8_t bss_rate_num, uint8_t *update_rate, uint8_t rate_size)
{
    mac_data_rate_stru *rates = NULL;
    uint32_t i, j, k;
    uint8_t rate_num = 0;

    if (rate_size > WLAN_USER_MAX_SUPP_RATES) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_get_scanned_bss_by_bssid::uc_rate_size err[%d].}", rate_size);
        return rate_num;
    }

    rates = mac_device_get_all_rates(mac_dev);

    for (i = 0; i < bss_rate_num; i++) {
        for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
            if ((IS_EQUAL_RATES(rates[j].uc_mac_rate, rate[i])) && (rate_num < MAC_DATARATES_PHY_80211G_NUM)) {
                /* 去除重复速率 */
                for (k = 0; k < rate_num; k++) {
                    if (IS_EQUAL_RATES(update_rate[k], rate[i])) {
                        break;
                    }
                }
                /* 当不存在重复速率时，k等于rate_num */
                if (k == rate_num) {
                    update_rate[rate_num++] = rate[i];
                }
                break;
            }
        }
    }

    return rate_num;
}


void hmac_scan_rm_repeat_sup_exsup_rates(mac_bss_dscr_stru *bss_dscr, uint8_t *rates, uint8_t exrate_num)
{
    int i, j;
    for (i = 0; i < exrate_num; i++) {
        /* 去除重复速率 */
        for (j = 0; j < bss_dscr->uc_num_supp_rates; j++) {
            if (IS_EQUAL_RATES(rates[i], bss_dscr->auc_supp_rates[j])) {
                break;
            }
        }

        /* 只有不存在重复速率时，j等于bss_dscr->uc_num_supp_rates */
        if (j == bss_dscr->uc_num_supp_rates && bss_dscr->uc_num_supp_rates < WLAN_USER_MAX_SUPP_RATES) {
            bss_dscr->auc_supp_rates[bss_dscr->uc_num_supp_rates++] = rates[i];
        }
    }
}


#define MAC_DATARATES_80211B_NUM     4
OAL_STATIC void hmac_scan_remove_11b_rate(uint8_t *avail_rate, uint8_t *rate_num)
{
    /* 基础速率大小在IE中表现为原数值大小乘2。11b速率大小为1, 2, 5.5, 11 */
    uint8_t rate_11b[MAC_DATARATES_80211B_NUM] = { 2, 4, 11, 22 };
    uint8_t target_rate[WLAN_USER_MAX_SUPP_RATES] = { 0 };
    uint8_t target_rate_num = 0;
    oal_bool_enum_uint8 en_is_11b_rate;
    uint8_t i, j;

    for (i = 0; i < *rate_num; i++) {
        en_is_11b_rate = OAL_FALSE;

        for (j = 0; j < MAC_DATARATES_80211B_NUM; j++) {
            if (IS_EQUAL_RATES(avail_rate[i], rate_11b[j])) {
                en_is_11b_rate = OAL_TRUE;
                break;
            }
        }

        if (!en_is_11b_rate) {
            target_rate[target_rate_num++] = avail_rate[i];
        }
    }

    /* copy原速率集长度，用于清空后面多出来的速率 */
    if (memcpy_s(avail_rate, WLAN_USER_MAX_SUPP_RATES, target_rate, *rate_num) == EOK) {
        *rate_num = target_rate_num;
        return;
    }
    oam_error_log0(0, OAM_SF_ANY, "hmac_scan_remove_11b_rate::memcpy fail!");
}

void hmac_scan_update_bss_list_rates_log(mac_device_stru *mac_dev, uint8_t *rate, uint8_t num_rates, uint32_t ie_len)
{
    mac_data_rate_stru *rates = NULL;
    uint32_t i, j;
    rates = mac_device_get_all_rates(mac_dev);
    // 打印获取的ie信息
    for (i = 0; i < ie_len; i++) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rates_log::rate[%d].}", rate[i]);
    }
    // 打印 11g速率
    for (j = 0; j < MAC_DATARATES_PHY_80211G_NUM; j++) {
        oam_warning_log1(0, OAM_SF_SCAN, "{uc_mac_rate[%d].}", rates[j].uc_mac_rate);
    }
    oam_warning_log2(0, OAM_SF_SCAN, "{ie_len[%d], num_rates[%d].}", ie_len, num_rates);
}


OAL_STATIC OAL_INLINE uint32_t hmac_scan_update_bss_list_rates(mac_bss_dscr_stru *bss_dscr, uint8_t *frame_body,
                                                               uint16_t frame_len, mac_device_stru *mac_dev)
{
    uint8_t *ie = NULL;
    uint8_t num_rates = 0;
    uint8_t num_ex_rates;
    uint8_t offset;
    uint8_t rates[WLAN_USER_MAX_SUPP_RATES] = { 0 };

    /* 设置Beacon帧的field偏移量 */
    offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;

    ie = mac_find_ie(MAC_EID_RATES, frame_body + offset, frame_len - offset);
    if (ie != NULL) {
        num_rates = hmac_scan_check_bss_supp_rates(mac_dev, ie + MAC_IE_HDR_LEN, ie[1], rates, sizeof(rates));
        
        if (num_rates > WLAN_USER_MAX_SUPP_RATES) {
            oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_bss_list_rates::num_rates=%d.}", num_rates);
            num_rates = WLAN_USER_MAX_SUPP_RATES;
        } else if (num_rates == 0) {
            hmac_scan_update_bss_list_rates_log(mac_dev, ie + MAC_IE_HDR_LEN, num_rates, ie[1]);
        }

        if (memcpy_s(bss_dscr->auc_supp_rates, WLAN_USER_MAX_SUPP_RATES, rates, num_rates) != EOK) {
            oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_update_bss_list_rates::memcpy fail!");
            return OAL_FAIL;
        }

        bss_dscr->uc_num_supp_rates = num_rates;
    }

    ie = mac_find_ie(MAC_EID_XRATES, frame_body + offset, frame_len - offset);
    if (ie != NULL) {
        num_ex_rates = hmac_scan_check_bss_supp_rates(mac_dev, ie + MAC_IE_HDR_LEN, ie[1], rates, sizeof(rates));
        if (num_rates + num_ex_rates > WLAN_USER_MAX_SUPP_RATES) { /* 超出支持速率个数 */
            oam_warning_log2(0, OAM_SF_SCAN,
                "{hmac_scan_update_bss_list_rates::number of rates too large, num_rates=%d, num_ex_rates=%d.}",
                num_rates, num_ex_rates);
        }

        if (num_ex_rates > 0) {
            /* support_rates和extended_rates去除重复速率，一并合入扫描结果的速率集中 */
            hmac_scan_rm_repeat_sup_exsup_rates(bss_dscr, rates, num_ex_rates);
        }
    }

    if (bss_dscr->st_channel.en_band == WLAN_BAND_5G) {
        hmac_scan_remove_11b_rate(bss_dscr->auc_supp_rates, &bss_dscr->uc_num_supp_rates);
    }

    return OAL_SUCC;
}


oal_bool_enum_uint8 hmac_scan_is_hidden_ssid(uint8_t vap_id, hmac_scanned_bss_info *new_bss,
    hmac_scanned_bss_info *old_bss)
{
    if ((new_bss->st_bss_dscr_info.ac_ssid[0] == '\0') && (old_bss->st_bss_dscr_info.ac_ssid[0] != '\0')) {
        /*  隐藏SSID,若保存过此AP信息,且ssid不为空,此次通过BEACON帧扫描到此AP信息,且SSID为空,则不进行更新 */
        oam_warning_log3(vap_id, OAM_SF_SCAN,
            "{hmac_scan_is_hidden_ssid::find hide ssid:%.2x:%.2x:%.2x,ignore this update.}",
            new_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_3], new_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_4],
            new_bss->st_bss_dscr_info.auc_bssid[MAC_ADDR_5]);
        return OAL_TRUE;
    }
    return OAL_FALSE;
}


oal_bool_enum_uint8 hmac_scan_need_update_old_scan_result(hmac_vap_stru *hmac_vap, hmac_scanned_bss_info *new_bss,
    hmac_scanned_bss_info *old_bss)
{
    hmac_roam_info_stru *roam_info = NULL;
    mac_scan_req_stru *scan_params = NULL;

    if (hmac_vap == NULL) {
        return OAL_FALSE;
    }

    roam_info = (hmac_roam_info_stru *)hmac_vap->pul_roam_info;
    if (roam_info != NULL) {
        scan_params = &roam_info->st_scan_h2d_params.st_scan_params;
        if (scan_params == NULL) {
            return OAL_FALSE;
        }

        /* 11k 11v 流程需要使用最新扫描结果以选择最佳RSSI AP */
        if (scan_params->uc_neighbor_report_process_flag || scan_params->uc_bss_transition_process_flag) {
            return OAL_TRUE;
        }
    }

    
    if ((((mac_ieee80211_frame_stru *)old_bss->st_bss_dscr_info.auc_mgmt_buff)->st_frame_control.bit_sub_type ==
        WLAN_PROBE_RSP) &&
        (((mac_ieee80211_frame_stru *)new_bss->st_bss_dscr_info.auc_mgmt_buff)->st_frame_control.bit_sub_type ==
        WLAN_BEACON) &&
        (old_bss->st_bss_dscr_info.en_new_scan_bss == OAL_TRUE)) {
        return OAL_FALSE;
    }
    return OAL_TRUE;
}


uint8_t hmac_scan_check_chan(oal_netbuf_stru *netbuf, hmac_scanned_bss_info *scanned_bss)
{
    dmac_rx_ctl_stru *rx_ctrl;
    uint8_t *frame_body;
    uint16_t frame_body_len;
    uint16_t offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    uint8_t *ie_start_addr;
    uint8_t chan_num, is_6ghz, curr_chan;

    rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    curr_chan = rx_ctrl->st_rx_info.uc_channel_number;
    is_6ghz = wlan_chip_mac_get_6g_flag(rx_ctrl);
    frame_body = scanned_bss->st_bss_dscr_info.auc_mgmt_buff + MAC_80211_FRAME_LEN;
    frame_body_len = scanned_bss->st_bss_dscr_info.mgmt_len - MAC_80211_FRAME_LEN;

    /* 在DSSS Param set ie中解析chan num */
    ie_start_addr = mac_find_ie(MAC_EID_DSPARMS, frame_body + offset, frame_body_len - offset);
    if ((ie_start_addr != NULL) && (ie_start_addr[1] == MAC_DSPARMS_LEN)) {
        chan_num = ie_start_addr[BYTE_OFFSET_2];
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(chan_num), chan_num, is_6ghz) != OAL_SUCC) {
            return OAL_FALSE;
        }
    }

    /* 在HT operation ie中解析 chan num */
    ie_start_addr = mac_find_ie(MAC_EID_HT_OPERATION, frame_body + offset, frame_body_len - offset);
    if ((ie_start_addr != NULL) && (ie_start_addr[1] >= 1)) {
        chan_num = ie_start_addr[BYTE_OFFSET_2];
        if (mac_is_channel_num_valid(mac_get_band_by_channel_num(chan_num), chan_num, is_6ghz) != OAL_SUCC) {
            return OAL_FALSE;
        }
    }

    chan_num = scanned_bss->st_bss_dscr_info.st_channel.uc_chan_number;
    if (((curr_chan > chan_num) && (curr_chan - chan_num >= 3)) || // 当前信道相比扫描信道偏移3个及以上
        ((curr_chan < chan_num) && (chan_num - curr_chan >= 3))) { // 扫描信道相比当前信道偏移3个及以上
        return OAL_FALSE;
    }

    return OAL_TRUE;
}
/*
 * 功能描述  : 更新bss_dscr的rssi
 */
static void hmac_scan_update_bss_dscr_rssi(mac_bss_dscr_stru *bss_dscr,
    mac_scanned_result_extend_info_stru *scan_result_extend_info)
{
    uint8_t chain_index;

    bss_dscr->c_rssi = (int8_t)scan_result_extend_info->l_rssi;
    for (chain_index = 0; chain_index < HD_EVENT_RF_NUM; chain_index++) {
        bss_dscr->rssi[chain_index] = scan_result_extend_info->rssi[chain_index];
        bss_dscr->snr[chain_index] = scan_result_extend_info->snr[chain_index];
    }
}

OAL_STATIC uint32_t hmac_scan_update_bss_dscr(hmac_vap_stru *hmac_vap,
    oal_netbuf_stru *netbuf, uint16_t buf_len, hmac_scanned_bss_info *scanned_bss, uint8_t ext_len)
{
    mac_scanned_result_extend_info_stru *scan_result_extend_info = NULL;
    mac_device_stru *mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    mac_ieee80211_frame_stru *frame_header = NULL;
    uint8_t *frame_body = NULL;
    mac_bss_dscr_stru *bss_dscr = NULL;
    uint8_t *ssid = NULL; /* 指向beacon帧中的ssid */
    uint8_t *mgmt_frame = (uint8_t *)oal_netbuf_data(netbuf);
    dmac_rx_ctl_stru *rx_ctrl = (dmac_rx_ctl_stru *)oal_netbuf_cb(netbuf);
    uint16_t frame_len = buf_len - sizeof(mac_scanned_result_extend_info_stru);
    uint16_t frame_body_len;
    uint16_t offset = MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN + MAC_CAP_INFO_LEN;
    uint8_t ssid_len, frame_channel;
    int32_t ret;

    if (oal_unlikely(mac_device == NULL)) {
        return OAL_FAIL;
    }

    /* 指向netbuf中的上报的扫描结果的扩展信息的位置 */
    scan_result_extend_info = (mac_scanned_result_extend_info_stru *)(mgmt_frame + frame_len);

    /* 拷贝管理帧内容 */
    ret = memcpy_s(scanned_bss->st_bss_dscr_info.auc_mgmt_buff, (uint32_t)frame_len, mgmt_frame, (uint32_t)frame_len);
    mgmt_frame = scanned_bss->st_bss_dscr_info.auc_mgmt_buff;

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* 根据互通IE填充akm suite(FT)到RSN IE */
    if (ext_len != DEFAULT_AKM_VALUE) {
        hmac_scan_attach_akm_suite_to_rsn_ie(mgmt_frame, &frame_len);
    }
#endif

    /* 获取管理帧的帧头和帧体指针 */
    frame_header = (mac_ieee80211_frame_stru *)mgmt_frame;
    frame_body = mgmt_frame + MAC_80211_FRAME_LEN;
    frame_body_len = frame_len - MAC_80211_FRAME_LEN;

    if (frame_body_len <= offset) {
        oam_error_log1(0, OAM_SF_SCAN, "hmac_scan_update_bss_dscr::frame_body_len err[%d]!", frame_body_len);
        return OAL_FAIL;
    }

    /* 获取管理帧中的信道 */
    frame_channel = mac_ie_get_chan_num(frame_body, frame_body_len, offset, rx_ctrl->st_rx_info.uc_channel_number);

    /* 更新bss信息 */
    bss_dscr = &(scanned_bss->st_bss_dscr_info);

    /*****************************************************************************
        解析beacon/probe rsp帧，记录到bss_dscr
    *****************************************************************************/
    /* 解析并保存ssid */
    ssid = mac_get_ssid(frame_body, (int32_t)frame_body_len, &ssid_len);
    if ((ssid != NULL) && (ssid_len != 0)) {
        /* 将查找到的ssid保存到bss描述结构体中 */
        ret += memcpy_s(bss_dscr->ac_ssid, WLAN_SSID_MAX_LEN, ssid, ssid_len);
        bss_dscr->ac_ssid[ssid_len] = '\0';
    }
    if (ret != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_update_bss_dscr::memcpy fail!");
        return OAL_FAIL;
    }

    /* 解析bssid */
    oal_set_mac_addr(bss_dscr->auc_mac_addr, frame_header->auc_address2);
    oal_set_mac_addr(bss_dscr->auc_bssid, frame_header->auc_address3);

    /* bss基本信息 */
    bss_dscr->en_bss_type = scan_result_extend_info->en_bss_type;

    bss_dscr->us_cap_info = *((uint16_t *)(frame_body + MAC_TIME_STAMP_LEN + MAC_BEACON_INTERVAL_LEN));

    hmac_scan_update_bss_dscr_rssi(bss_dscr, scan_result_extend_info);

    /* 解析beacon周期与tim周期 */
    bss_dscr->us_beacon_period = mac_get_beacon_period(frame_body);
    bss_dscr->uc_dtim_period = mac_get_dtim_period(frame_body, frame_body_len);
    bss_dscr->uc_dtim_cnt = mac_get_dtim_cnt(frame_body, frame_body_len);

    /* 信道 */
    bss_dscr->st_channel.uc_chan_number = frame_channel;
    bss_dscr->st_channel.en_band = mac_get_band_by_channel_num(frame_channel);
    bss_dscr->st_channel.ext6g_band = wlan_chip_mac_get_6g_flag(rx_ctrl);
    /* 记录速率集 */
    hmac_scan_update_bss_list_rates(bss_dscr, frame_body, frame_body_len, mac_device);

    /* 03记录支持的最大空间流 */
#ifdef _PRE_WLAN_FEATURE_M2S
    bss_dscr->en_support_opmode = scan_result_extend_info->en_support_opmode;
    bss_dscr->uc_num_sounding_dim = scan_result_extend_info->uc_num_sounding_dim;
#endif
    bss_dscr->en_support_max_nss = scan_result_extend_info->en_support_max_nss;

    /* 协议类相关信息元素的获取 */
    hmac_scan_update_bss_list_protocol(hmac_vap, bss_dscr, frame_body, frame_body_len);

    /* update st_channel.bandwidth in case hmac_sta_update_join_req_params usage error */
    bss_dscr->st_channel.en_bandwidth = bss_dscr->en_channel_bandwidth;
    ret = mac_get_channel_idx_from_num(bss_dscr->st_channel.en_band,
        bss_dscr->st_channel.uc_chan_number, bss_dscr->st_channel.ext6g_band, &bss_dscr->st_channel.uc_chan_idx);
    if (ret == OAL_ERR_CODE_INVALID_CONFIG) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_update_bss_dscr::mac_get_channel_idx_from_num fail.}");
    }

    /* 更新时间戳 */
    bss_dscr->timestamp = (uint32_t)oal_time_get_stamp_ms();
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    bss_dscr->wpa_rpt_time = oal_get_monotonic_boottime();
#endif
    bss_dscr->mgmt_len = frame_len;

    return OAL_SUCC;
}

uint16_t hmac_scan_check_adaptive11r_need_akm_suite(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
    uint16_t buf_len, uint8_t *ext_len)
{
    uint16_t mgmt_len;

    /* 管理帧的长度等于上报的netbuf的长度减去上报的扫描结果的扩展字段的长度 */
    mgmt_len = buf_len - sizeof(mac_scanned_result_extend_info_stru);

#ifdef _PRE_WLAN_FEATURE_ADAPTIVE11R
    /* adaptive 11R模式下检查是否需要在RSN IE中添加akm suite */
    if (hmac_vap->bit_adaptive11r == OAL_TRUE) {
        *ext_len = hmac_scan_extend_mgmt_len_needed(netbuf, buf_len);
        if (*ext_len != DEFAULT_AKM_VALUE) {
            mgmt_len += *ext_len;
        }
    }
#endif

    return mgmt_len;
}

void hmac_scan_save_mbss_info(hmac_vap_stru *hmac_vap,
    mac_multi_bssid_frame_info_stru *mbss_frame_info, hmac_scanned_bss_info *new_scanned_bss)
{
#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        /* 保存mbss info */
        if (IS_CUSTOM_OPEN_MULTI_BSSID_SWITCH(&hmac_vap->st_vap_base_info)) {
            memcpy_s(new_scanned_bss->st_bss_dscr_info.st_mbssid_info.auc_transmitted_bssid, WLAN_MAC_ADDR_LEN,
                     mbss_frame_info->auc_transmitted_bssid, WLAN_MAC_ADDR_LEN);
            new_scanned_bss->st_bss_dscr_info.st_mbssid_info.bit_is_non_transimitted_bss =
                mbss_frame_info->bit_is_non_transimitted_bss;
        }
    }
#endif
}

void hmac_scan_update_opmode_and_rssi(hmac_scanned_bss_info *new_scanned_bss, hmac_scanned_bss_info *old_scanned_bss)
{
    uint32_t curr_time_stamp;
#ifdef _PRE_WLAN_FEATURE_M2S
    mac_ieee80211_frame_stru *frame_header = NULL;

    /* 只有probe rsp帧中ext cap 宣称支持OPMODE时，对端才确实支持OPMODE，beacon帧和assoc rsp帧中信息不可信 */
    frame_header = (mac_ieee80211_frame_stru *)new_scanned_bss->st_bss_dscr_info.auc_mgmt_buff;
    if (frame_header->st_frame_control.bit_sub_type == WLAN_PROBE_RSP) {
        old_scanned_bss->st_bss_dscr_info.en_support_opmode = new_scanned_bss->st_bss_dscr_info.en_support_opmode;
    }
#endif

    /* 如果老的扫描的bss的信号强度大于当前扫描到的bss的信号强度，更新当前扫描到的信号强度为最强的信号强度 */
    if (old_scanned_bss->st_bss_dscr_info.c_rssi > new_scanned_bss->st_bss_dscr_info.c_rssi) {
        /* 1s中以内就采用之前的BSS保存的RSSI信息，否则就采用新的RSSI信息 */
        curr_time_stamp = (uint32_t)oal_time_get_stamp_ms();
        if (oal_time_after32((curr_time_stamp),
            (old_scanned_bss->st_bss_dscr_info.timestamp + HMAC_SCAN_MAX_SCANNED_RSSI_EXPIRE)) == FALSE) {
            new_scanned_bss->st_bss_dscr_info.c_rssi = old_scanned_bss->st_bss_dscr_info.c_rssi;
        }
    }
}

void hmac_scan_update_time_and_rssi(oal_netbuf_stru *netbuf,
    hmac_scanned_bss_info *new_scanned_bss, hmac_scanned_bss_info *old_scanned_bss)
{
    old_scanned_bss->st_bss_dscr_info.timestamp = (uint32_t)oal_time_get_stamp_ms();
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    old_scanned_bss->st_bss_dscr_info.wpa_rpt_time = oal_get_monotonic_boottime();
#endif
    old_scanned_bss->st_bss_dscr_info.c_rssi = new_scanned_bss->st_bss_dscr_info.c_rssi;
}



uint32_t hmac_scan_proc_scan_result_handle(hmac_vap_stru *hmac_vap, oal_netbuf_stru *netbuf,
                                           uint16_t buf_len, mac_multi_bssid_frame_info_stru *mbss_frame_info)
{
    uint32_t ret;
    uint8_t vap_id;
    uint16_t mgmt_len;
    uint8_t ext_len = DEFAULT_AKM_VALUE;
    hmac_device_stru *hmac_device = NULL;
    hmac_scanned_bss_info *new_scanned_bss = NULL;
    hmac_scanned_bss_info *old_scanned_bss = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* 获取vap id */
    vap_id = hmac_vap->st_vap_base_info.uc_vap_id;

    /* 获取hmac device 结构 */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        oam_error_log0(vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_result_handle::hmac_device null.}");

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /***********************************************************************************************/
    /* 对dmac上报的netbuf内容进行解析，netbuf data域的上报的扫描结果的字段的分布                       */
    /* ------------------------------------------------------------------------------------------ */
    /* beacon/probe rsp body  |     帧体后面附加字段(mac_scanned_result_extend_info_stru)          */
    /* ----------------------------------------------------------------------------------------- */
    /* 收到的beacon/rsp的body | rssi(4字节) | channel num(1字节)| band(1字节)|bss_tye(1字节)|填充  */
    /* -----------------------------------------------------------------------------------------*/
    /********************************************************************************************/
    mgmt_len = hmac_scan_check_adaptive11r_need_akm_suite(hmac_vap, netbuf, buf_len, &ext_len);
    /* 申请存储扫描结果的内存 */
    new_scanned_bss = hmac_scan_alloc_scanned_bss(mgmt_len);
    if (oal_unlikely(new_scanned_bss == NULL)) {
        oam_error_log0(vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_result_handle::alloc memory failed}");

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 更新描述扫描结果的bss dscr结构体 */
    ret = hmac_scan_update_bss_dscr(hmac_vap, netbuf, buf_len, new_scanned_bss, ext_len);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_error_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_result_handle::hmac_scan_update_bss_dscr failed[%d].}", ret);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);

        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);

        return ret;
    }

    hmac_scan_save_mbss_info(hmac_vap, mbss_frame_info, new_scanned_bss);

    /* 获取管理扫描的bss结果的结构体 */
    bss_mgmt = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt.st_bss_mgmt);
    /* 对链表删操作前加锁 */
    oal_spin_lock(&(bss_mgmt->st_lock));
    /* 判断相同bssid的bss是否已经扫描到 */
    old_scanned_bss = hmac_scan_find_scanned_bss_by_bssid(bss_mgmt, new_scanned_bss->st_bss_dscr_info.auc_bssid);
    if (old_scanned_bss == NULL) {
        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));

        goto add_bss;
    }

    hmac_scan_update_opmode_and_rssi(new_scanned_bss, old_scanned_bss);

    if (hmac_scan_is_hidden_ssid(vap_id, new_scanned_bss, old_scanned_bss) == OAL_TRUE) {
        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));

        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);

        return OAL_SUCC;
    }

    if (hmac_scan_need_update_old_scan_result(hmac_vap, new_scanned_bss, old_scanned_bss) == OAL_FALSE
        || hmac_scan_check_chan(netbuf, new_scanned_bss) == OAL_FALSE) {
        hmac_scan_update_time_and_rssi(netbuf, new_scanned_bss, old_scanned_bss);

        /* 解锁 */
        oal_spin_unlock(&(bss_mgmt->st_lock));

        /* 释放申请的存储bss信息的内存 */
        oal_free(new_scanned_bss);

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(netbuf);
        return OAL_SUCC;
    }

    /* 从链表中将原先扫描到的相同bssid的bss节点删除 */
    hmac_scan_del_bss_from_list_nolock(old_scanned_bss, hmac_device);
    /* 解锁 */
    oal_spin_unlock(&(bss_mgmt->st_lock));
    /* 释放内存 */
    oal_free(old_scanned_bss);
add_bss:

    /* 将扫描结果添加到链表中 */
    hmac_scan_add_bss_to_list(new_scanned_bss, hmac_device);
    /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
    oal_netbuf_free(netbuf);

    return OAL_SUCC;
}


uint32_t hmac_scan_proc_scanned_bss(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    oal_netbuf_stru *bss_mgmt_netbuf = NULL;
    dmac_tx_event_stru *dtx_event = NULL;
    mac_multi_bssid_frame_info_stru mbss_frame_info;
    uint32_t ret;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    dtx_event = (dmac_tx_event_stru *)event->auc_event_data;
    bss_mgmt_netbuf = dtx_event->pst_netbuf;

    hmac_vap = mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss::hmac_vap null.}");

        /* 释放上报的bss信息和beacon或者probe rsp帧的内存 */
        oal_netbuf_free(bss_mgmt_netbuf);
        return OAL_ERR_CODE_PTR_NULL;
    }

#ifdef _PRE_WLAN_FEATURE_11AX
    if (g_wlan_spec_cfg->feature_11ax_is_open) {
        if (IS_CUSTOM_OPEN_MULTI_BSSID_SWITCH(&hmac_vap->st_vap_base_info)) {
            hmac_scan_proc_multi_bssid_scanned_bss(hmac_vap, bss_mgmt_netbuf, dtx_event->us_frame_len);
        }
    }
#endif

    /* 处理发送Beacon 或Probe Rsp的bss */
    memset_s((uint8_t *)&mbss_frame_info, sizeof(mbss_frame_info), 0, sizeof(mbss_frame_info));
    ret = hmac_scan_proc_scan_result_handle(hmac_vap, bss_mgmt_netbuf, dtx_event->us_frame_len, &mbss_frame_info);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scanned_bss::hmac_scan_proc_scan_result_handle failed.}");
    }

    return OAL_SUCC;
}


OAL_STATIC void hmac_scan_print_scan_record_info(hmac_vap_stru *hmac_vap, hmac_scan_record_stru *scan_record)
{
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_time_t_stru timestamp_diff;

    /* 获取扫描间隔时间戳 */
    timestamp_diff = oal_ktime_sub(oal_ktime_get(), scan_record->st_scan_start_time);

    /* 调用内核接口，打印此次扫描耗时 */
    oam_warning_log4(scan_record->uc_vap_id, OAM_SF_SCAN,
        "{hmac_scan_print_scan_record_info::scan comp,scan_status:%d,vap ch_num:%d,cookie:%x, duration time:%lums.}",
        scan_record->en_scan_rsp_status, hmac_vap->st_vap_base_info.st_channel.uc_chan_number,
        scan_record->ull_cookie, ktime_to_ms(timestamp_diff));
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    // 如果是HID2D发起的扫描，不需要打印信息 */
    if (scan_record->en_scan_mode == WLAN_SCAN_MODE_HID2D_SCAN) {
        return;
    }
#endif

    if (!hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH) || hmac_vap->bit_in_p2p_listen == OAL_FALSE) {
        /* 打印扫描到的bss信息 */
        hmac_scan_print_scanned_bss_info(scan_record->uc_device_id);
    }

    return;
}


uint32_t hmac_scan_proc_scan_comp_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_device_stru *hmac_device = NULL;
    mac_scan_rsp_stru *d2h_scan_rsp_info = NULL;
    hmac_scan_stru *scan_mgmt = NULL;
    oal_bool_enum_uint8 temp_flag;
    mac_vap_stru *mac_vap = NULL;

    if (oal_unlikely(event_mem == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取事件头和事件结构体指针 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(event_hdr->uc_device_id);
    if (oal_unlikely(hmac_device == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    d2h_scan_rsp_info = (mac_scan_rsp_stru *)(event->auc_event_data);
    scan_mgmt = &(hmac_device->st_scan_mgmt);

    mac_vap = (mac_vap_stru *)mac_res_get_mac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::mac_vap null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if ((event_hdr->uc_vap_id != scan_mgmt->st_scan_record_mgmt.uc_vap_id) ||
        ((mac_vap->en_p2p_mode != WLAN_LEGACY_VAP_MODE) &&
        (d2h_scan_rsp_info->ull_cookie != scan_mgmt->st_scan_record_mgmt.ull_cookie))) {
        oam_warning_log4(event_hdr->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_comp_event::Report vap:%d Scan_rsp(cookie %d), another vap:%d scan(cookie %d)!}",
            event_hdr->uc_vap_id, d2h_scan_rsp_info->ull_cookie,
            scan_mgmt->st_scan_record_mgmt.uc_vap_id, scan_mgmt->st_scan_record_mgmt.ull_cookie);
        return OAL_SUCC;
    }

    if (!hmac_get_feature_switch(HMAC_MIRACAST_REDUCE_LOG_SWITCH)) {
        oam_warning_log1(event_hdr->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::scan status:%d!}",
            d2h_scan_rsp_info->en_scan_rsp_status);
    }

    /* 删除扫描超时保护定时器 */
    temp_flag = ((scan_mgmt->st_scan_timeout.en_is_registerd == OAL_TRUE) &&
                 (d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO));
    if (temp_flag) {
        /* PNO没有启动扫描定时器,考虑到取消PNO扫描,立即下发普通扫描,PNO扫描结束事件对随后的普通扫描的影响 */
        frw_timer_immediate_destroy_timer_m(&(scan_mgmt->st_scan_timeout));
    }

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(event_hdr->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(event_hdr->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_comp_event::hmac_vap null.}");

        /* 设置当前处于非扫描状态 */
        scan_mgmt->en_is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO)) {
        if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (scan_mgmt->st_scan_record_mgmt.en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        hmac_fsm_change_state(hmac_vap, scan_mgmt->st_scan_record_mgmt.en_vap_last_state);
        scan_mgmt->st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* 根据device上报的扫描结果，上报sme */
    /* 将扫描执行情况(扫描执行成功、还是失败等返回结果)记录到扫描运行记录结构体中 */
    scan_mgmt->st_scan_record_mgmt.en_scan_rsp_status = d2h_scan_rsp_info->en_scan_rsp_status;
    scan_mgmt->st_scan_record_mgmt.ull_cookie = d2h_scan_rsp_info->ull_cookie;

    hmac_scan_print_scan_record_info(hmac_vap, &(scan_mgmt->st_scan_record_mgmt));

    if (scan_mgmt->st_scan_record_mgmt.p_fn_cb != NULL) {
        /* 终止扫描无需调用回调,防止终止扫描结束对随后发起PNO扫描的影响 */
        scan_mgmt->st_scan_record_mgmt.p_fn_cb(&(scan_mgmt->st_scan_record_mgmt));
    }

    /* 设置当前处于非扫描状态 */
    if (d2h_scan_rsp_info->en_scan_rsp_status != MAC_SCAN_PNO) {
        /* PNO扫描没有置此位为OAL_TRUE,PNO扫描结束,不能影响随后的常规扫描 */
        scan_mgmt->en_is_scanning = OAL_FALSE;
    }
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_LISTEN) {
        hmac_p2p_listen_timeout(hmac_vap, &hmac_vap->st_vap_base_info);
    }
    if (hmac_vap->en_wait_roc_end == OAL_TRUE) {
        
        oam_warning_log1(event_hdr->uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_comp_event::scan rsp status[%d]}", d2h_scan_rsp_info->en_scan_rsp_status);
        oal_complete(&(hmac_vap->st_roc_end_ready));
        hmac_vap->en_wait_roc_end = OAL_FALSE;
    }

    /* STA背景扫描时，需要提前识别漫游场景 */
    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
        (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP) &&
        (scan_mgmt->st_scan_record_mgmt.en_scan_mode == WLAN_SCAN_MODE_BACKGROUND_STA)) {
        hmac_roam_check_bkscan_result(hmac_vap, &(scan_mgmt->st_scan_record_mgmt));
    }

    return OAL_SUCC;
}

uint32_t hmac_scan_proc_scan_req_event_exception(hmac_vap_stru *hmac_vap, void *p_params)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_scan_rsp_stru scan_rsp;
    hmac_scan_rsp_stru *pst_scan_rsp = NULL;

    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, p_params))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event_exception::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 不支持发起扫描的状态发起了扫描 */
    oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
        "{hmac_scan_proc_scan_req_event_exception::vap state is=%x.}", hmac_vap->st_vap_base_info.en_vap_state);

    memset_s(&scan_rsp, sizeof(hmac_scan_rsp_stru), 0, sizeof(hmac_scan_rsp_stru));

    /* 抛扫描完成事件到WAL, 执行SCAN_DONE , 释放扫描请求内存 */
    event_mem = frw_event_alloc_m(sizeof(hmac_scan_rsp_stru));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_scan_req_event_exception::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    scan_rsp.uc_result_code = MAC_SCAN_REFUSED;
    /* When STA is roaming, scan req return success instead of failure,
       in case roaming failure which will cause UI scan list null  */
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        scan_rsp.uc_result_code = MAC_SCAN_SUCCESS;
    }
    scan_rsp.uc_num_dscr = 0;

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_HOST_CTX, HMAC_HOST_CTX_EVENT_SUB_TYPE_SCAN_COMP_STA,
                       sizeof(hmac_scan_rsp_stru), FRW_EVENT_PIPELINE_STAGE_0, hmac_vap->st_vap_base_info.uc_chip_id,
                       hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    pst_scan_rsp = (hmac_scan_rsp_stru *)event->auc_event_data;

    if (memcpy_s(pst_scan_rsp, sizeof(hmac_scan_rsp_stru), (void *)(&scan_rsp), sizeof(hmac_scan_rsp_stru)) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_proc_scan_req_event_exception::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}


void hmac_scan_set_sour_mac_addr_in_probe_req(hmac_vap_stru *hmac_vap, uint8_t *sour_mac_addr,
    oal_bool_enum_uint8 en_is_rand_mac_addr_scan, oal_bool_enum_uint8 en_is_p2p0_scan)
{
    mac_device_stru *mac_device = NULL;
    hmac_device_stru *hmac_device = NULL;

    if (oal_any_null_ptr2(hmac_vap, sour_mac_addr)) {
        oam_error_log0(0, OAM_SF_CFG,
            "{hmac_scan_set_sour_mac_addr_in_probe_req::param null.}");
        return;
    }

    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_scan_set_sour_mac_addr_in_probe_req::mac_device is null.}");
        return;
    }

    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(0, OAM_SF_CFG, "{hmac_scan_set_sour_mac_addr_in_probe_req::hmac_device is null. device_id %d}",
                         hmac_vap->st_vap_base_info.uc_device_id);
        return;
    }

    /* WLAN/P2P 特性情况下，p2p0 和p2p-p2p0 cl 扫描时候，需要使用不同设备 */
    if (en_is_p2p0_scan == OAL_TRUE) {
        oal_set_mac_addr(sour_mac_addr, mac_mib_get_p2p0_dot11StationID(&hmac_vap->st_vap_base_info));
    } else {
        /* 如果随机mac addr扫描特性开启且非P2P场景，设置随机mac addr到probe req帧中 */
        if ((en_is_rand_mac_addr_scan == OAL_TRUE) && (IS_LEGACY_VAP(&(hmac_vap->st_vap_base_info))) &&
            ((mac_device->auc_mac_oui[BYTE_OFFSET_0] != 0) || (mac_device->auc_mac_oui[BYTE_OFFSET_1] != 0) ||
            (mac_device->auc_mac_oui[BYTE_OFFSET_2] != 0))) {
            /* 更新随机mac 地址,使用下发随机MAC OUI 生成的随机mac 地址更新到本次扫描 */
            oal_set_mac_addr(sour_mac_addr, hmac_device->st_scan_mgmt.auc_random_mac);
        } else {
            /* 设置地址为自己的MAC地址 */
            oal_set_mac_addr(sour_mac_addr, mac_mib_get_StationID(&hmac_vap->st_vap_base_info));
        }
    }

    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_scan_need_skip_channel(hmac_vap_stru *hmac_vap, uint8_t channel)
{
    wlan_channel_band_enum_uint8 en_band = mac_get_band_by_channel_num(channel);
    if (mac_chip_run_band(hmac_vap->st_vap_base_info.uc_chip_id, en_band) != OAL_TRUE) {
        return OAL_TRUE;
    }

    return OAL_FALSE;
}


OAL_STATIC void hmac_scan_set_neighbor_report_scan_params(mac_scan_req_stru *scan_params)
{
    if (scan_params->uc_neighbor_report_process_flag == OAL_TRUE ||
        scan_params->uc_bss_transition_process_flag == OAL_TRUE) {
        scan_params->uc_scan_channel_interval = 2; /* 间隔2个信道，切回工作信道工作一段时间 */
        scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;
    }
    return;
}

OAL_STATIC oal_bool_enum_uint8 hmac_scan_get_random_mac_flag(hmac_vap_stru *hmac_vap,
                                                             oal_bool_enum_uint8 en_customize_random_mac_scan)
{
    /* 非P2P设备才配置随机mac扫描标志到device，并下发到DMAC */
    return (IS_LEGACY_VAP(&(hmac_vap->st_vap_base_info))) ? en_customize_random_mac_scan : OAL_FALSE;
}


OAL_STATIC uint32_t hmac_scan_update_scan_params(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
                                                 oal_bool_enum_uint8 en_is_random_mac_addr_scan)
{
    mac_device_stru *mac_device;
    mac_vap_stru *mac_vap_temp = NULL;
    uint32_t ret;
    wlan_vap_mode_enum_uint8 en_vap_mode;
    uint8_t uc_loop;
    uint8_t uc_chan_cnt = 0;

    /* 获取mac device */
    mac_device = mac_res_get_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (mac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_device_id, OAM_SF_SCAN,
                         "{hmac_scan_update_scan_params::mac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }
    /* 1.记录发起扫描的vap id到扫描参数 */
    scan_params->uc_vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    scan_params->en_need_switch_back_home_channel = OAL_FALSE;

    if (scan_params->en_scan_mode != WLAN_SCAN_MODE_ROAM_SCAN) {
        scan_params->en_scan_mode = WLAN_SCAN_MODE_FOREGROUND;
    }

    /* 2.修改扫描模式和信道扫描次数: 根据是否存在up状态下的vap，如果是，则是背景扫描，如果不是，则是前景扫描 */
    ret = mac_device_find_up_vap(mac_device, &mac_vap_temp);
    if ((ret == OAL_SUCC) && (mac_vap_temp != NULL)) {
        /* 判断vap的类型，如果是sta则为sta的背景扫描，如果是ap，则是ap的背景扫描，其它类型的vap暂不支持背景扫描 */
        en_vap_mode = hmac_vap->st_vap_base_info.en_vap_mode;
        if (en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
            if (scan_params->en_scan_mode != WLAN_SCAN_MODE_ROAM_SCAN) {
                /* 修改扫描参数为sta的背景扫描 */
                scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_STA;
            }
        } else if (en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            /* 修改扫描参数为sta的背景扫描 */
            scan_params->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_AP;
        } else {
            oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_update_scan_params::vap mode[%d], not support bg scan.}",
                           en_vap_mode);
            return OAL_FAIL;
        }
        scan_params->en_need_switch_back_home_channel = OAL_TRUE;

        if (mac_device_calc_up_vap_num(mac_device) == 1 && !IS_LEGACY_VAP(mac_vap_temp) &&
            IS_LEGACY_VAP(&hmac_vap->st_vap_base_info)) {
            /*
             * 修改扫描信道间隔(2)和回工作信道工作时间(60ms):仅仅针对P2P处于关联状态，
             * wlan处于去关联状态,wlan发起的扫描
             */
            scan_params->uc_scan_channel_interval = MAC_SCAN_CHANNEL_INTERVAL_PERFORMANCE;
            scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_PERFORMANCE;

            if ((scan_params->us_scan_time > WLAN_DEFAULT_ACTIVE_SCAN_TIME) &&
                (scan_params->en_scan_type == WLAN_SCAN_TYPE_ACTIVE)) {
                /* 指定SSID扫描超过3个,会修改每次扫描时间为40ms(默认是20ms) */
                /* P2P关联但wlan未关联场景,考虑到扫描时间增加对p2p wfd场景的影响,设置每信道扫描次数为1次(默认为2次) */
                scan_params->uc_max_scan_count_per_channel = 1;
            }
        } else {
            /* 携带隐藏SSID的情况下扫3个信道回一次home信道，其他情况默认扫描6个信道回home信道工作100ms */
            scan_params->uc_scan_channel_interval = (scan_params->uc_ssid_num > 1) ?
                MAC_SCAN_CHANNEL_INTERVAL_HIDDEN_SSID : MAC_SCAN_CHANNEL_INTERVAL_DEFAULT;
            scan_params->us_work_time_on_home_channel = MAC_WORK_TIME_ON_HOME_CHANNEL_DEFAULT;
        }
        hmac_scan_set_neighbor_report_scan_params(scan_params);
    }

    /* 3.设置发送的probe req帧中源mac addr */
    scan_params->en_is_random_mac_addr_scan = hmac_scan_get_random_mac_flag(hmac_vap, en_is_random_mac_addr_scan);
    hmac_scan_set_sour_mac_addr_in_probe_req(hmac_vap, scan_params->auc_sour_mac_addr,
                                             en_is_random_mac_addr_scan, scan_params->bit_is_p2p0_scan);

    for (uc_loop = 0; uc_loop < scan_params->uc_channel_nums; uc_loop++) {
        if (!hmac_scan_need_skip_channel(hmac_vap, scan_params->ast_channel_list[uc_loop].uc_chan_number)) {
            if (uc_chan_cnt != uc_loop) {
                scan_params->ast_channel_list[uc_chan_cnt] = scan_params->ast_channel_list[uc_loop];
            }
            uc_chan_cnt++;
        }
    }

    if (!uc_chan_cnt) {
        oam_warning_log1(0, OAM_SF_SCAN, "{hmac_scan_update_scan_params::channels trimed to none!, ori cnt=%d}",
                         scan_params->uc_channel_nums);

        return OAL_FAIL;
    }

    scan_params->uc_channel_nums = uc_chan_cnt;
    return OAL_SUCC;
}


OAL_STATIC uint32_t hmac_scan_check_can_enter_scan_state(mac_vap_stru *mac_vap)
{
    /* p2p有可能进行监听动作，但是和scan req的优先级一样，因此当上层发起的是扫描请求时，统一可使用下面的接口判断 */
    return hmac_p2p_check_can_enter_state(mac_vap, HMAC_FSM_INPUT_SCAN_REQ);
}


uint32_t hmac_scan_check_is_dispatch_scan_req(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    uint32_t ret;

    /* 1.先检测其它vap的状态从而判断是否可进入扫描状态，使得扫描尽量不打断其它的入网流程 */
    ret = hmac_scan_check_can_enter_scan_state(&(hmac_vap->st_vap_base_info));
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_check_is_dispatch_scan_req::Because of err_code[%d], can't enter into scan state.}", ret);
        return ret;
    }

    /* 2.判断当前扫描是否正在执行 */
    if (hmac_device->st_scan_mgmt.en_is_scanning == OAL_TRUE) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_check_is_dispatch_scan_req::the scan request is rejected.}");
        return OAL_FAIL;
    }

    /* 3.判断当前是否正在执行漫游 */
    if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_ROAMING) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_check_is_dispatch_scan_req:: roam reject new scan.}");
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_FEATURE_DFS
    /* 4.判断当前是否正在执行CAC */
    if (hmac_cac_abort_scan_check(hmac_device) == OAL_SUCC) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_check_is_dispatch_scan_req:: cac abort scan.}");
        return OAL_FAIL;
    }
#endif

    return OAL_SUCC;
}


OAL_STATIC void hmac_scan_proc_last_scan_record(hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    /* 本次扫描请求发起时，清除上一次扫描结果中过期的bss信息 */
    hmac_scan_clean_expire_scanned_bss(hmac_vap, &(hmac_device->st_scan_mgmt.st_scan_record_mgmt));

    return;
}


OAL_STATIC uint32_t hmac_scan_proc_scan_timeout_fn(void *arg)
{
    hmac_device_stru *hmac_device = (hmac_device_stru *)arg;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_scan_record_stru *scan_record = NULL;
    uint32_t pedding_data = 0;

    /* 获取扫描记录信息 */
    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    /* 获取hmac vap */
    hmac_vap = mac_res_get_hmac_vap(scan_record->uc_vap_id);
    if (oal_unlikely(hmac_vap == NULL)) {
        oam_error_log0(scan_record->uc_vap_id, OAM_SF_SCAN, "{hmac_scan_proc_scan_timeout_fn::hmac_vap null.}");

        /* 扫描状态恢复为未在执行的状态 */
        hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 根据当前扫描的类型和当前vap的状态，决定切换vap的状态，如果是前景扫描，才需要切换vap的状态 */
    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) {
        if (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_STA_WAIT_SCAN) {
            /* 改变vap状态到SCAN_COMP */
            hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_SCAN_COMP);
        }
    }

    if ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
        (scan_record->en_vap_last_state != MAC_VAP_STATE_BUTT)) {
        hmac_fsm_change_state(hmac_vap, scan_record->en_vap_last_state);
        scan_record->en_vap_last_state = MAC_VAP_STATE_BUTT;
    }
    /* 设置扫描响应状态为超时 */
    scan_record->en_scan_rsp_status = MAC_SCAN_TIMEOUT;
    oam_warning_log1(scan_record->uc_vap_id, OAM_SF_SCAN,
        "{hmac_scan_proc_scan_timeout_fn::scan time out cookie [%x].}", scan_record->ull_cookie);

    /* 如果扫描回调函数不为空，则调用回调函数 */
    if (scan_record->p_fn_cb != NULL) {
        oam_warning_log0(scan_record->uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_timeout_fn::scan callback func proc.}");
        scan_record->p_fn_cb(scan_record);
    }

    /* DMAC 超时未上报扫描完成，HMAC 下发扫描结束命令，停止DMAC 扫描 */
    hmac_config_scan_abort(&hmac_vap->st_vap_base_info, sizeof(uint32_t), (uint8_t *)&pedding_data);

    /* 扫描状态恢复为未在执行的状态 */
    hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;

    chr_exception(chr_wifi_drv(CHR_WIFI_DRV_EVENT_SCAN, CHR_WIFI_DRV_ERROR_SCAN_TIMEOUT));

    return OAL_SUCC;
}


OAL_STATIC void hmac_scan_proc_clear_last_scan_record(mac_scan_req_stru *scan_params,
    hmac_vap_stru *hmac_vap, hmac_device_stru *hmac_device)
{
    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        if (scan_params->uc_scan_func & MAC_SCAN_FUNC_P2P_LISTEN) {
            hmac_vap->bit_in_p2p_listen = OAL_TRUE;
        } else {
            hmac_vap->bit_in_p2p_listen = OAL_FALSE;
            /* 处理上一次扫描记录，目前直接清楚上一次结果，后续可能需要老化时间处理 */
            hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);
        }
    } else {
        /* 处理上一次扫描记录，目前直接清楚上一次结果，后续可能需要老化时间处理 */
        hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);
    }
}

uint32_t hmac_scan_preproc_scan_req_event(hmac_vap_stru *hmac_vap, void *params,
    mac_scan_req_h2d_stru **ph2d_scan_req_params, mac_scan_req_stru **pscan_params, hmac_device_stru **phmac_device)
{
    oal_bool_enum_uint8 temp_flag;
    mac_scan_req_h2d_stru *h2d_scan_req_params = NULL; /* hmac发送到dmac的扫描请求参数 */
    mac_scan_req_stru *scan_params = NULL;
    hmac_device_stru *hmac_device = NULL;

    /* 参数合法性检查 */
    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, params))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_scan_req_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 扫描停止模块测试 */
    temp_flag = (((g_en_bgscan_enable_flag == HMAC_BGSCAN_DISABLE) &&
                  (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_UP)) ||
                  (g_en_bgscan_enable_flag == HMAC_SCAN_DISABLE));
    if (temp_flag) {
        oam_warning_log1(0, 0, "hmac_scan_proc_scan_req_event:bgscan_en_flag:%d", g_en_bgscan_enable_flag);
        return OAL_FAIL;
    }

    h2d_scan_req_params = (mac_scan_req_h2d_stru *)params;
    scan_params = &(h2d_scan_req_params->st_scan_params);

    /* 异常判断: 扫描的信道个数为0 */
    if (scan_params->uc_channel_nums == 0) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event::channel_nums=0.}");
        return OAL_FAIL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_req_event::hmac_device null.}");
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    *ph2d_scan_req_params = h2d_scan_req_params;
    *pscan_params = scan_params;
    *phmac_device = hmac_device;

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
OAL_STATIC void hmac_scan_update_passive_scan_channel_param(mac_scan_req_ex_stru *scan_params,
                                                            mac_scan_channel_stru *mac_channel)
{
    mac_regdomain_info_stru *regdomain_info = NULL;

    /* 获取管制域全局变量 */
    mac_get_regdomain_info(&regdomain_info);
    /* 特殊管制域 99 */
    if (((regdomain_info->ac_country[0] == '9') && (regdomain_info->ac_country[1] == '9')) ||
        ((regdomain_info->ac_country[0] == 'Z') && (regdomain_info->ac_country[1] == 'Z'))) {
        /* 2g特殊管制域信道执行主动扫描 */
        if (mac_channel->en_band == WLAN_BAND_2G) {
            if (mac_channel->uc_chan_number >= DMAC_CHANNEL12) { /* 12、13、14 信道被动扫描 */
                mac_channel->scan_policy = WLAN_SCAN_TYPE_PASSIVE;
                mac_channel->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
            } else {
                mac_channel->scan_policy = WLAN_SCAN_TYPE_ACTIVE;
                mac_channel->scan_time = (uint8_t)(scan_params->us_scan_time);
            }
        } else {
            /* 5g特殊管制域信道执行被动扫描 */
            mac_channel->scan_policy = WLAN_SCAN_TYPE_PASSIVE;
            mac_channel->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
        }
    }
}
#endif
OAL_STATIC void hmac_scan_update_dfs_channel_scan_param(mac_scan_req_ex_stru *scan_params,
                                                        mac_scan_channel_stru *mac_channel)
{
    mac_vap_stru *mac_vap = mac_res_get_mac_vap(scan_params->uc_vap_id);
    if (scan_params->uc_neighbor_report_process_flag == OAL_TRUE) {
        mac_channel->scan_policy = WLAN_SCAN_TYPE_ACTIVE;
        mac_channel->scan_time = (uint8_t)(scan_params->us_scan_time);
        return;
    }
    if (mac_vap != NULL && mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        mac_channel->scan_policy = WLAN_SCAN_TYPE_PASSIVE;
        mac_channel->scan_time = (uint8_t)(scan_params->us_scan_time);
        return;
    }
    mac_channel->scan_policy = WLAN_SCAN_TYPE_HALF;
    mac_channel->scan_time = WLAN_DEFAULT_PASSIVE_SCAN_TIME;
}
OAL_STATIC void hmac_scan_channel_convert_ex(mac_scan_req_ex_stru *scan_req_ex, mac_scan_req_stru *scan_params)
{
    uint8_t idx;
    if (scan_params->uc_channel_nums > WLAN_MAX_CHANNEL_NUM) {
        oam_error_log1(0, OAM_SF_SCAN, "{hmac_scan_channel_convert_ex::channel_nums[%d] invalid.}",
            scan_params->uc_channel_nums);
        return;
    }
    for (idx = 0; idx < scan_params->uc_channel_nums; idx++) {
        if (memcpy_s(&(scan_req_ex->ast_channel_list[idx]), sizeof(mac_scan_channel_stru),
            &(scan_params->ast_channel_list[idx]), sizeof(mac_channel_stru)) != EOK) {
            oam_error_log0(0, 0, "hmac_scan_channel_convert_ex::memcpy_s fail!");
        }
        scan_req_ex->ast_channel_list[idx].scan_policy = WLAN_SCAN_TYPE_ACTIVE;
        scan_req_ex->ast_channel_list[idx].scan_time = (uint8_t)(scan_params->us_scan_time);
        if (mac_is_dfs_channel(scan_req_ex->ast_channel_list[idx].en_band,
            scan_req_ex->ast_channel_list[idx].uc_chan_number) == OAL_TRUE) {
            hmac_scan_update_dfs_channel_scan_param(scan_req_ex,
                &(scan_req_ex->ast_channel_list[idx]));
        }
#ifdef _PRE_WLAN_COUNTRYCODE_SELFSTUDY
        /*
         * 国家码自学习定制化开关 国家码自学习功能仅当ini country code = 99,
         * self study = 1(99国家码自学习，所有逻辑在驱动)/2（ZZ国家码自学习，学习逻辑在上层，信道处理在驱动
         * 打开
         */
        if (wlan_chip_get_selfstudy_country_flag() != 0) {
            /* PC5G被动扫描信道设置 */
            hmac_scan_update_passive_scan_channel_param(scan_req_ex, &(scan_req_ex->ast_channel_list[idx]));
        }
#endif
    }
}
void hmac_scan_param_convert_ex(mac_scan_req_ex_stru *scan_req_ex, mac_scan_req_stru *scan_params)
{
        scan_req_ex->en_bss_type = scan_params->en_bss_type;
        scan_req_ex->en_scan_type = scan_params->en_scan_type;
        scan_req_ex->uc_bssid_num = scan_params->uc_bssid_num;
        scan_req_ex->uc_ssid_num = scan_params->uc_ssid_num;
        memcpy_s(scan_req_ex->auc_sour_mac_addr, WLAN_MAC_ADDR_LEN, scan_params->auc_sour_mac_addr, WLAN_MAC_ADDR_LEN);
        scan_req_ex->uc_p2p0_listen_channel = scan_params->uc_p2p0_listen_channel;
        scan_req_ex->uc_max_scan_count_per_channel = scan_params->uc_max_scan_count_per_channel;
        memcpy_s(scan_req_ex->auc_bssid, sizeof(scan_req_ex->auc_bssid),
                 scan_params->auc_bssid, sizeof(scan_params->auc_bssid));
        memcpy_s(scan_req_ex->ast_mac_ssid_set, sizeof(scan_req_ex->ast_mac_ssid_set),
                 scan_params->ast_mac_ssid_set, sizeof(scan_params->ast_mac_ssid_set));
        scan_req_ex->uc_max_send_probe_req_count_per_channel = scan_params->uc_max_send_probe_req_count_per_channel;
        scan_req_ex->bit_is_p2p0_scan = scan_params->bit_is_p2p0_scan;
        scan_req_ex->bit_is_radom_mac_saved = scan_params->bit_is_radom_mac_saved;
        scan_req_ex->bit_radom_mac_saved_to_dev = scan_params->bit_radom_mac_saved_to_dev;
        scan_req_ex->bit_desire_fast_scan = scan_params->bit_desire_fast_scan;
        scan_req_ex->bit_roc_type_tx_mgmt = scan_params->bit_roc_type_tx_mgmt;
        scan_req_ex->bit_rsv = scan_params->bit_rsv;
        scan_req_ex->en_abort_scan_flag = scan_params->en_abort_scan_flag;
        scan_req_ex->en_is_random_mac_addr_scan = scan_params->en_is_random_mac_addr_scan;
        scan_req_ex->en_need_switch_back_home_channel = scan_params->en_need_switch_back_home_channel;
        scan_req_ex->uc_scan_channel_interval = scan_params->uc_scan_channel_interval;
        scan_req_ex->us_work_time_on_home_channel = scan_params->us_work_time_on_home_channel;
        scan_req_ex->uc_channel_nums = scan_params->uc_channel_nums;
        scan_req_ex->uc_probe_delay = scan_params->uc_probe_delay;
        scan_req_ex->us_scan_time = scan_params->us_scan_time;
        scan_req_ex->en_scan_mode = scan_params->en_scan_mode;
        scan_req_ex->uc_resv = scan_params->uc_resv;
        scan_req_ex->uc_scan_func = scan_params->uc_scan_func;
        scan_req_ex->uc_vap_id = scan_params->uc_vap_id;
        scan_req_ex->ull_cookie = scan_params->ull_cookie;
        scan_req_ex->uc_neighbor_report_process_flag = scan_params->uc_neighbor_report_process_flag;
        scan_req_ex->uc_bss_transition_process_flag = scan_params->uc_bss_transition_process_flag;
        scan_req_ex->p_fn_cb = scan_params->p_fn_cb;
        hmac_scan_channel_convert_ex(scan_req_ex, scan_params);
}

static int32_t hmac_scan_param_update(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    int32_t ret;
    /* 更新此次扫描请求的扫描参数 */
    if (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
        ret = hmac_scan_update_scan_params(hmac_vap, scan_params, OAL_FALSE);
#ifdef _PRE_WLAN_FEATURE_HID2D
    /* HiD2D触发的扫描仅仅为信道测量扫描，不发送probe request，无需更新扫描参数 */
    } else if (scan_params->en_scan_mode == WLAN_SCAN_MODE_HID2D_SCAN) {
        ret = OAL_SUCC;
#endif
    } else {
        /* 更新此次扫描请求的扫描参数 */
        ret = hmac_scan_update_scan_params(hmac_vap, scan_params, g_wlan_cust.uc_random_mac_addr_scan);
    }

    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_param_get_and_update::update scan mode failed, error[%d].}", ret);
        return ret;
    }

    return ret;
}

static void hmac_scan_update_scan_record(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params,
                                         hmac_device_stru *hmac_device)
{
    hmac_scan_record_stru *scan_record = NULL;
   /* 设置扫描模块处于扫描状态，其它扫描请求将丢弃 */
    hmac_device->st_scan_mgmt.en_is_scanning = OAL_TRUE;

    hmac_scan_proc_clear_last_scan_record(scan_params, hmac_vap, hmac_device);

    /* 记录扫描发起者的信息，某些模块回调函数使用 */
    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    scan_record->uc_chip_id = hmac_device->pst_device_base_info->uc_chip_id;
    scan_record->uc_device_id = hmac_device->pst_device_base_info->uc_device_id;
    scan_record->uc_vap_id = scan_params->uc_vap_id;
    scan_record->uc_chan_numbers = scan_params->uc_channel_nums;
    scan_record->p_fn_cb = scan_params->p_fn_cb;
    scan_record->en_scan_mode = scan_params->en_scan_mode;

    if (hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_update_scan_record::save last vap_state:%d}", hmac_vap->st_vap_base_info.en_vap_state);

        scan_record->en_vap_last_state = hmac_vap->st_vap_base_info.en_vap_state;
    }

    scan_record->ull_cookie = scan_params->ull_cookie;

    /* 记录扫描开始时间 */
    scan_record->st_scan_start_time = oal_ktime_get();
}

static void hmac_scan_req_fsm_change_state(hmac_vap_stru *hmac_vap, mac_scan_req_stru *scan_params)
{
    oal_bool_enum_uint8 temp_flag;
    /* 如果发起扫描的vap的模式为sta，并且，其关联状态为非up状态，且非p2p监听状态，则切换其扫描状态 */
    temp_flag = ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_STA) &&
                 (scan_params->uc_scan_func != MAC_SCAN_FUNC_P2P_LISTEN) &&
                 (hmac_vap->st_vap_base_info.en_vap_state != MAC_VAP_STATE_UP));
    if (temp_flag) {
        /* 切换vap的状态为WAIT_SCAN状态 */
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_STA_WAIT_SCAN);
    }

    /* AP的启动扫描做特殊处理，当hostapd下发扫描请求时，VAP还处于INIT状态 */
    temp_flag = ((hmac_vap->st_vap_base_info.en_vap_mode == WLAN_VAP_MODE_BSS_AP) &&
                 (hmac_vap->st_vap_base_info.en_vap_state == MAC_VAP_STATE_INIT));
    if (temp_flag) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_req_fsm_change_state::ap startup scan.}");
        hmac_fsm_change_state(hmac_vap, MAC_VAP_STATE_AP_WAIT_START);
    }
}

static void hmac_scan_req_event_scan_timeout_mechanism_set(mac_scan_req_stru *scan_params,
    hmac_device_stru *hmac_device, hmac_vap_stru *hmac_vap)
{
    uint32_t scan_timeout;
    /******************************************************************/
    /* 打印扫描参数，测试使用 */
    /* 如果是P2P 发起监听，则设置HMAC 扫描超时时间为P2P 监听时间 */
    scan_timeout = (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) ?
        (scan_params->us_scan_time * 2) : WLAN_DEFAULT_MAX_TIME_PER_SCAN; // 超时时间是扫描时间的2倍
    if (hmac_get_feature_switch(HMAC_MIRACAST_SINK_SWITCH)) {
        scan_timeout = (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) ?
            /* 因为大屏板测试发现dmac切换信道需要约20ms时间，当wpa supplicant将p2p listen设置成10ms时 */
            /* 如果ul_scan_timeout只设置2 * 10ms，将导致dmac上报扫描结果前该定时器就超时，故设为4 * 10ms */
            (scan_params->us_scan_time * 4 + HMAC_P2P_LISTEN_EXTRA_WAIT_TIME) : WLAN_DEFAULT_MAX_TIME_PER_SCAN;
    }

    hmac_scan_print_scan_params(scan_params, &hmac_vap->st_vap_base_info);

    /* 启动扫描保护定时器，防止因拋事件、核间通信失败等情况下的异常保护，定时器初步的超时时间为4.5秒 */
    frw_timer_create_timer_m(&(hmac_device->st_scan_mgmt.st_scan_timeout), hmac_scan_proc_scan_timeout_fn,
        scan_timeout, hmac_device, OAL_FALSE, OAM_MODULE_ID_HMAC, hmac_device->pst_device_base_info->core_id);
}


uint32_t hmac_scan_proc_scan_req_event(hmac_vap_stru *hmac_vap, void *params)
{
    frw_event_mem_stru *event_mem = NULL;
    mac_scan_req_h2d_stru *h2d_scan_req_params = NULL; /* hmac发送到dmac的扫描请求参数 */
    mac_scan_req_stru *scan_params = NULL;
    hmac_device_stru *hmac_device = NULL;
    oal_netbuf_stru *netbuf_scan_req = NULL;
    int32_t ret;

    ret = hmac_scan_preproc_scan_req_event(hmac_vap, params, &h2d_scan_req_params, &scan_params, &hmac_device);
    if (ret != OAL_SUCC) {
        return ret;
    }

#ifdef _PRE_WLAN_FEATURE_MONITOR
    if (hmac_device->en_monitor_mode == OAL_TRUE) {
        oam_warning_log0(0, OAM_SF_CFG, "{hmac_scan_proc_scan_req_event_etc: in sniffer monitor mode, scan abort!}");
        return -OAL_EINVAL;
    }
#endif

    ret = hmac_scan_param_update(hmac_vap, scan_params);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* 检测是否符合发起扫描请求的条件，如果不符合，直接返回 */
    ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, hmac_device);
    if (ret != OAL_SUCC) {
        if (scan_params->uc_scan_func == MAC_SCAN_FUNC_P2P_LISTEN) {
            hmac_vap_state_restore(hmac_vap);
        }

        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_scan_req_event::Because of error[%d], can't dispatch scan req.}", ret);
        return ret;
    }

    hmac_scan_update_scan_record(hmac_vap, scan_params, hmac_device);

    /* 抛扫描请求事件到DMAC, 申请event事件内存 */
    event_mem = frw_event_alloc_m(sizeof(dmac_tx_event_stru));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_scan_req_event::alloc memory failed.}");
        /* 恢复扫描状态为非运行状态 */
        hmac_device->st_scan_mgmt.en_is_scanning = OAL_FALSE;
        return OAL_ERR_CODE_PTR_NULL;
    }

    hmac_scan_req_fsm_change_state(hmac_vap, scan_params);

    ret = wlan_chip_scan_req_alloc_and_fill_netbuf(event_mem, hmac_vap, &netbuf_scan_req, params);
    if (ret != OAL_SUCC) {
        frw_event_free_m(event_mem);
        return ret;
    }

    hmac_scan_req_event_scan_timeout_mechanism_set(scan_params, hmac_device, hmac_vap);

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);

    oal_netbuf_free(netbuf_scan_req);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}


uint32_t hmac_scan_proc_sched_scan_req_event(hmac_vap_stru *hmac_vap, void *params)
{
    frw_event_mem_stru *event_mem = NULL;
    frw_event_stru *event = NULL;
    hmac_device_stru *hmac_device = NULL;
    hmac_scan_record_stru *scan_record = NULL;
    mac_pno_scan_stru *pno_scan_params = NULL;
    uint32_t ret;

    /* 参数合法性检查 */
    if (oal_unlikely(oal_any_null_ptr2(hmac_vap, params))) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_proc_sched_scan_req_event::param null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pno_scan_params = (mac_pno_scan_stru *)params;

    /* 判断PNO调度扫描下发的过滤的ssid个数小于等于0 */
    if (pno_scan_params->l_ssid_count <= 0) {
        oam_warning_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                         "{hmac_scan_proc_sched_scan_req_event::ssid_count <=0.}");
        return OAL_FAIL;
    }

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(hmac_vap->st_vap_base_info.uc_device_id);
    if (hmac_device == NULL) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_sched_scan_req_event::hmac_device[%d] null.}", hmac_vap->st_vap_base_info.uc_device_id);
        return OAL_ERR_CODE_MAC_DEVICE_NULL;
    }

    /* 检测是否符合发起扫描请求的条件，如果不符合，直接返回 */
    ret = hmac_scan_check_is_dispatch_scan_req(hmac_vap, hmac_device);
    if (ret != OAL_SUCC) {
        oam_warning_log1(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
            "{hmac_scan_proc_sched_scan_req_event::Because of error[%d], can't dispatch scan req.}", ret);
        return ret;
    }

    /* 清空上一次的扫描结果 */
    hmac_scan_proc_last_scan_record(hmac_vap, hmac_device);

    /* 记录扫描发起者的信息，某些模块回调函数使用 */
    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);
    scan_record->uc_chip_id = hmac_device->pst_device_base_info->uc_chip_id;
    scan_record->uc_device_id = hmac_device->pst_device_base_info->uc_device_id;
    scan_record->uc_vap_id = hmac_vap->st_vap_base_info.uc_vap_id;
    scan_record->p_fn_cb = pno_scan_params->p_fn_cb;
    scan_record->en_scan_mode = WLAN_SCAN_MODE_BACKGROUND_PNO;

    /* 抛扫描请求事件到DMAC, 申请事件内存 */
    event_mem = frw_event_alloc_m(sizeof(mac_pno_scan_stru *));
    if (event_mem == NULL) {
        oam_error_log0(hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_SCAN,
                       "{hmac_scan_proc_sched_scan_req_event::event_mem null.}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 填写事件 */
    event = frw_get_event_stru(event_mem);

    frw_event_hdr_init(&(event->st_event_hdr), FRW_EVENT_TYPE_WLAN_CTX, DMAC_WLAN_CTX_EVENT_SUB_TYPE_SCHED_SCAN_REQ,
        sizeof(mac_pno_scan_stru *), FRW_EVENT_PIPELINE_STAGE_1, hmac_vap->st_vap_base_info.uc_chip_id,
        hmac_vap->st_vap_base_info.uc_device_id, hmac_vap->st_vap_base_info.uc_vap_id);

    /* 事件data域内携带PNO扫描请求参数 */
    if (memcpy_s(frw_get_event_payload(event_mem), sizeof(mac_pno_scan_stru *),
        (uint8_t *)&pno_scan_params, sizeof(mac_pno_scan_stru *)) != EOK) {
        oam_error_log0(0, OAM_SF_SCAN, "hmac_scan_proc_sched_scan_req_event::memcpy fail!");
        frw_event_free_m(event_mem);
        return OAL_FAIL;
    }

    /* 分发事件 */
    frw_event_dispatch_event(event_mem);
    frw_event_free_m(event_mem);

    return OAL_SUCC;
}


uint32_t hmac_scan_process_chan_result_event(frw_event_mem_stru *event_mem)
{
    frw_event_stru *event = NULL;
    frw_event_hdr_stru *event_hdr = NULL;
    hmac_device_stru *hmac_device = NULL;
    dmac_crx_chan_result_stru *chan_result_param = NULL;
    hmac_scan_record_stru *scan_record = NULL;
    uint8_t scan_idx;

    /* 获取事件信息 */
    event = frw_get_event_stru(event_mem);
    event_hdr = &(event->st_event_hdr);
    chan_result_param = (dmac_crx_chan_result_stru *)(event->auc_event_data);
    scan_idx = chan_result_param->uc_scan_idx;

    /* 获取hmac device */
    hmac_device = hmac_res_get_mac_dev(event_hdr->uc_device_id);
    if (hmac_device == NULL) {
        oam_error_log0(0, OAM_SF_SCAN, "{hmac_scan_process_chan_result_event::hmac_device is null.}");
        return OAL_FAIL;
    }

    scan_record = &(hmac_device->st_scan_mgmt.st_scan_record_mgmt);

    /* 检查上报的索引是否合法 */
    if (scan_idx >= scan_record->uc_chan_numbers) {
        /* dmac上报的扫描结果超出了要扫描的信道个数 */
        oam_warning_log2(0, OAM_SF_SCAN,
            "{hmac_scan_process_chan_result_event::result from dmac error! scan_idx[%d], chan_numbers[%d].}",
            scan_idx, scan_record->uc_chan_numbers);

        return OAL_FAIL;
    }

    scan_record->ast_chan_results[scan_idx] = chan_result_param->st_chan_result;

    /* HiD2D get scan results */
#ifdef _PRE_WLAN_FEATURE_HID2D
    hmac_hid2d_scan_complete_handler(scan_record, scan_idx);
#endif

    return OAL_SUCC;
}

void hmac_scan_init(hmac_device_stru *hmac_device)
{
    hmac_scan_stru *scan_mgmt = NULL;
    hmac_bss_mgmt_stru *bss_mgmt = NULL;

    /* 初始化扫描管理结构体信息 */
    scan_mgmt = &(hmac_device->st_scan_mgmt);
    memset_s(scan_mgmt, sizeof(hmac_scan_stru), 0, sizeof(hmac_scan_stru));
    scan_mgmt->en_is_scanning = OAL_FALSE;
    scan_mgmt->st_scan_record_mgmt.en_vap_last_state = MAC_VAP_STATE_BUTT;

    /* 初始化bss管理结果链表和锁 */
    bss_mgmt = &(scan_mgmt->st_scan_record_mgmt.st_bss_mgmt);
    oal_dlist_init_head(&(bss_mgmt->st_bss_list_head));
    oal_spin_lock_init(&(bss_mgmt->st_lock));

    /* 初始化内核下发扫描request资源锁 */
    oal_spin_lock_init(&(scan_mgmt->st_scan_request_spinlock));

    /* 初始化 st_wiphy_mgmt 结构 */
    oal_wait_queue_init_head(&(scan_mgmt->st_wait_queue));

    /* 初始化扫描生成随机MAC 地址 */
    oal_random_ether_addr(hmac_device->st_scan_mgmt.auc_random_mac);
    return;
}


void hmac_scan_exit(hmac_device_stru *hmac_device)
{
    hmac_scan_stru *scan_mgmt = NULL;

    /* 清空扫描记录信息 */
    hmac_scan_clean_scan(&(hmac_device->st_scan_mgmt));

    if (hmac_device->st_scan_mgmt.st_init_scan_timeout.en_is_registerd) {
        frw_timer_immediate_destroy_timer_m(&hmac_device->st_scan_mgmt.st_init_scan_timeout);
    }

    /* 清除扫描管理结构体信息 */
    scan_mgmt = &(hmac_device->st_scan_mgmt);
    memset_s(scan_mgmt, sizeof(hmac_scan_stru), 0, sizeof(hmac_scan_stru));
    scan_mgmt->en_is_scanning = OAL_FALSE;
    return;
}

uint32_t hmac_bgscan_enable(mac_vap_stru *mac_vap, uint16_t us_len, uint8_t *param)
{
    hmac_scan_state_enum pen_bgscan_enable_flag;

    pen_bgscan_enable_flag = *param; /* 背景扫描停止使能位 */

    /* 背景扫描停止命令 */
    switch (pen_bgscan_enable_flag) {
        case HMAC_BGSCAN_DISABLE:
            g_en_bgscan_enable_flag = HMAC_BGSCAN_DISABLE;
            break;
        case HMAC_BGSCAN_ENABLE:
            g_en_bgscan_enable_flag = HMAC_BGSCAN_ENABLE;
            break;
        case HMAC_SCAN_DISABLE:
            g_en_bgscan_enable_flag = HMAC_SCAN_DISABLE;
            break;
        default:
            g_en_bgscan_enable_flag = HMAC_BGSCAN_ENABLE;
            break;
    }

    oam_warning_log1(0, OAM_SF_SCAN, "hmac_bgscan_enable: g_en_bgscan_enable_flag=%d.", g_en_bgscan_enable_flag);

    return OAL_SUCC;
}


uint32_t hmac_scan_start_dbac(mac_device_stru *dev)
{
    uint8_t auc_cmd[NUM_32_BYTES];
    uint16_t us_len;
    uint32_t ret = OAL_FAIL;
    uint8_t uc_idx;
#define DBAC_START_STR     " dbac start"
#define DBAC_START_STR_LEN 11
    mac_vap_stru *mac_vap = NULL;

    mac_ioctl_alg_config_stru *alg_config = (mac_ioctl_alg_config_stru *)auc_cmd;

    if (memcpy_s(auc_cmd + sizeof(mac_ioctl_alg_config_stru), sizeof(auc_cmd) - sizeof(mac_ioctl_alg_config_stru),
        (const int8_t *)DBAC_START_STR, 11) != EOK) { // " dbac start" 长度11
        oam_error_log0(0, OAM_SF_CFG, "hmac_scan_start_dbac::memcpy fail!");
        return OAL_FAIL;
    }
    auc_cmd[sizeof(mac_ioctl_alg_config_stru) + DBAC_START_STR_LEN] = 0;

    alg_config->uc_argc = 2; // 2: dbac start 参数数目
    alg_config->auc_argv_offset[0] = 1;
    alg_config->auc_argv_offset[1] = 6; // 6: 配置参数
    for (uc_idx = 0; uc_idx < dev->uc_vap_num; uc_idx++) {
        mac_vap = mac_res_get_mac_vap(dev->auc_vap_id[uc_idx]);
        if (mac_vap != NULL && mac_vap->en_vap_mode == WLAN_VAP_MODE_BSS_AP) {
            break;
        }
    }
    if (mac_vap != NULL) {
        us_len = sizeof(mac_ioctl_alg_config_stru) + DBAC_START_STR_LEN + 1;
        ret = hmac_config_send_event(mac_vap, WLAN_CFGID_ALG, us_len, auc_cmd);
        if (oal_unlikely(ret != OAL_SUCC)) {
            oam_warning_log1(mac_vap->uc_vap_id, OAM_SF_CFG,
                             "{hmac_config_alg::hmac_config_send_event failed[%d].}", ret);
        }
        oal_io_print("start dbac\n");
    } else {
        oal_io_print("no vap found to start dbac\n");
    }

    return ret;
}

uint32_t hmac_init_scan_cancel_timer(hmac_device_stru *hmac_dev)
{
    if (hmac_dev != NULL && hmac_dev->st_scan_mgmt.st_init_scan_timeout.en_is_registerd) {
        /* 关闭超时定时器 */
        frw_timer_immediate_destroy_timer_m(&hmac_dev->st_scan_mgmt.st_init_scan_timeout);
    }

    return OAL_SUCC;
}

