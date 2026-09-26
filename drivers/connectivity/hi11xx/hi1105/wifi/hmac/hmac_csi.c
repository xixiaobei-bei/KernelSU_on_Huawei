
/* 1 头文件包含 */
#include "oal_kernel_file.h"
#include "mac_data.h"
#include "hmac_resource.h"
#include "hmac_host_tx_data.h"
#include "hmac_tx_data.h"
#include "hmac_config.h"
#include "plat_pm_wlan.h"
#include "hmac_csi.h"
#include "hmac_ftm.h"
#include "host_hal_ext_if.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_CSI_C

#include "securec.h"
#include "securectype.h"

#ifdef _PRE_WLAN_FEATURE_CSI

static void hmac_proc_location_print_timestamp(oal_file *file)
{
    struct timeval time;
    oal_time_stru local_time;
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
    oal_do_gettimeofday(&time);
#endif
    oal_get_real_time_m(&local_time);

    oal_kernel_file_print(file, "%04d-%02d-%02d-", local_time.tm_year + BASE_YEAR,
        local_time.tm_mon + 1, local_time.tm_mday);
    oal_kernel_file_print(file, "%02d-%02d-%02d-%08d : ", local_time.tm_hour,
        local_time.tm_min, local_time.tm_sec, time.tv_usec);
}


static void hmac_proc_location_print(hmac_csi_info_stru *hmac_csi_info, uint8_t *start_addr, oal_file *file,
                                     uint16_t rpt_info_len)
{
    uint16_t delta_len;
    int32_t str_len;
    uint8_t *payload = NULL;

    hmac_proc_location_print_timestamp(file);

    payload = &hmac_csi_info->csi_data[0];
    str_len = 0;
    while (str_len < CSI_INFO_LEN) {
        oal_kernel_file_print(file, "%02X ", *(payload++));
        str_len++;
    }

    str_len = 0;
    payload = (uint8_t *)start_addr;
    payload += CSI_HEDA_LEN;
    delta_len = rpt_info_len - CSI_HEDA_LEN;

    while (str_len < delta_len) {
        oal_kernel_file_print(file, "%02X ", *(payload++));
        str_len++;
    }

    oal_kernel_file_print(file, "\n");
}


void hmac_csi_handle(hal_host_location_isr_stru *ftm_csi_isr)
{
    uint8_t filename[FILE_LEN];
    int32_t str_len;
    uint64_t csi_base_addr;
    oal_file *file = NULL;
    oal_mm_segment_t old_fs;
    dma_addr_t host_iova = 0;
    uint8_t *csi_addr = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    hmac_csi_info_stru hmac_csi_info = { 0 };

    csi_base_addr = ftm_csi_isr->csi_info_addr;
    if (OAL_SUCC != pcie_if_devva_to_hostca(0, csi_base_addr, (uint64_t *)&host_iova)) {
        oam_warning_log0(0, OAM_SF_RX, "{hmac_csi_handle:get csi addr fail.}");
        return ;
    }

    csi_addr = (uint8_t *)(uintptr_t)host_iova;
    hal_get_csi_info(&hmac_csi_info, csi_addr);
    hmac_vap = mac_res_get_hmac_vap(hmac_csi_info.vap_index);
    if (hmac_vap == NULL) {
        return ;
    }

    str_len = snprintf_s(filename, FILE_LEN, FILE_LEN - 1, "/data/log/location/%s/%s", hmac_vap->auc_name, "CSI.TXT");
    if (str_len < 0) {
        oam_error_log1(0, OAM_SF_FTM, "hmac_csi_handle::snprintf_s error! str_len %d", str_len);
        return ;
    }

    file = oal_kernel_file_open((uint8_t *)filename, OAL_O_RDWR | OAL_O_CREAT | OAL_O_APPEND);
    if (IS_ERR_OR_NULL(file)) {
        oam_error_log0(0, OAM_SF_FTM, "{hmac_csi_handle: *************** save file failed}");
        return ;
    }
    old_fs = oal_get_fs();
    hmac_proc_location_print(&hmac_csi_info, csi_addr, file, hmac_csi_info.rpt_info_len);

    oal_kernel_file_close(file);
    oal_set_fs(old_fs);
}


uint32_t hmac_csi_config_1106(mac_vap_stru *mac_vap, uint16_t len, uint8_t *param)
{
    return hal_host_csi_config(mac_vap, len, param);
}

#endif