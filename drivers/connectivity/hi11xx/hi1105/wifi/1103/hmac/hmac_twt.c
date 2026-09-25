

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef _PRE_WLAN_FEATURE_TWT

/* 1 头文件包含 */
#include "oal_util.h"
#include "hmac_ext_if.h"
#include "hmac_resource.h"
#include "hmac_device.h"
#include "hmac_vap.h"
#include "hmac_mgmt_bss_comm.h"
#include "mac_frame.h"
#include "hmac_p2p.h"
#include "hmac_user.h"
#include "hmac_mgmt_ap.h"
#include "securec.h"
#include "securectype.h"
#include "hmac_config.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_TWT_C

/* 2 静态函数声明 */
/* 3 全局变量定义 */
/* 4 函数实现 */
#ifdef _PRE_WLAN_FEATURE_TWT

oal_uint32 hmac_dbac_teardown_twt_session(mac_device_stru *pst_dev)
{
    mac_chip_stru *pst_chip = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap_temp;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    oal_uint8 uc_vap_num;
    oal_uint32 ul_ret;

    pst_chip = hmac_res_get_mac_chip(pst_dev->uc_chip_id);
    if (pst_chip == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
            "hmac_dbac_teardown_twt_session::hmac_res_get_mac_chip id[%d] NULL",
            pst_dev->uc_chip_id);
        return OAL_FAIL;
    }

    uc_vap_num = hmac_get_chip_vap_num(pst_chip);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_dbac_teardown_twt_session::check vap num for twt,vap num=[%d]}", uc_vap_num);

    /* 非DBDC场景，如果启动了3个及以上的vap，说明是wlan0/p2p共存，
       此时如果已经建立twt会话，则需要删除twt会话 */
    if ((pst_dev->en_dbdc_running == OAL_FALSE) && (uc_vap_num >= 3)) {
        /* 先找到wlan vap */
        ul_ret = mac_device_find_up_sta_wlan_etc(pst_dev, &pst_mac_vap_temp);
        if ((ul_ret == OAL_SUCC) && (pst_mac_vap_temp != OAL_PTR_NULL)) {
            pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap_temp->uc_vap_id);
            if (pst_hmac_vap == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_dbac_teardown_twt_session::mac_res_get_hmac_vap failed.}");
                return OAL_FAIL;
            }

            OAM_WARNING_LOG1(0, OAM_SF_ANY,
                "{hmac_dbac_teardown_twt_session::twt_session_enable=[%d]}",
                pst_hmac_vap->st_twt_cfg.uc_twt_session_enable);

            /* 如果该vap的twt会话已经开启，则删除twt会话 */
            if (pst_hmac_vap->st_twt_cfg.uc_twt_session_enable == 1) {
                hmac_config_twt_teardown_req_auto(&(pst_hmac_vap->st_vap_base_info));
            }
        }
    }

    return OAL_SUCC;
}



oal_uint32 hmac_twt_auto_setup_session(mac_device_stru *pst_device)
{
    mac_chip_stru *pst_chip = OAL_PTR_NULL;
    hmac_vap_stru *pst_hmac_vap = OAL_PTR_NULL;
    hmac_user_stru *pst_hmac_user = OAL_PTR_NULL;
    mac_vap_stru *pst_mac_vap_temp;
    oal_uint8 uc_vap_num;
    oal_uint32 ul_ret;
    mac_he_hdl_stru st_he_hdl;

    pst_chip = hmac_res_get_mac_chip(pst_device->uc_chip_id);
    if (pst_chip == OAL_PTR_NULL) {
        OAM_ERROR_LOG1(0, OAM_SF_ANY,
            "hmac_twt_auto_setup_session::hmac_res_get_mac_chip id[%d] NULL",
            pst_device->uc_chip_id);
        return OAL_FAIL;
    }

    uc_vap_num = hmac_get_chip_vap_num(pst_chip);

    OAM_WARNING_LOG1(0, OAM_SF_ANY, "{hmac_twt_auto_setup_session::check vap num for twt,vap num=[%d]}", uc_vap_num);

    /* 只有1个vap的时候，如果ap支持twt，sta没有建立会话，则建立twt会话 */
    if ((pst_device->en_dbdc_running == OAL_FALSE) && (uc_vap_num == 2)) {
        ul_ret = mac_device_find_up_sta_wlan_etc(pst_device, &pst_mac_vap_temp);
        if ((ul_ret == OAL_SUCC) && (pst_mac_vap_temp != OAL_PTR_NULL)) {
            /* 重新建立twt会话 */
            pst_hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(pst_mac_vap_temp->uc_vap_id);
            if (pst_hmac_vap == OAL_PTR_NULL) {
                OAM_ERROR_LOG0(0, OAM_SF_CFG, "{hmac_twt_auto_setup_session::mac_res_get_hmac_vap failed.}");
                return OAL_FAIL;
            }

            /* 获用户 */
            pst_hmac_user = mac_res_get_hmac_user_etc(pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
            if (pst_hmac_user == OAL_PTR_NULL) {
                /* 和ap侧一样，上层已经删除了的话，属于正常 */
                OAM_WARNING_LOG1(pst_hmac_vap->st_vap_base_info.uc_vap_id, OAM_SF_CFG,
                                 "{hmac_twt_auto_setup_session::pst_hmac_user[%d] is null.}",
                                 pst_hmac_vap->st_vap_base_info.us_assoc_vap_id);
                return OAL_ERR_CODE_PTR_NULL;
            }

            /* 判断该AP是否支持twt，如果是fast ps模式，先检查ap是否支持twt，
             如果支持twt，建立twt会话，然后直接返回，否则进入fast ps模式 */
            mac_user_get_he_hdl(&(pst_hmac_user->st_user_base_info), &st_he_hdl);
            if (st_he_hdl.st_he_cap_ie.st_he_mac_cap.bit_twt_responder_support == OAL_TRUE) {
                if (pst_hmac_vap->st_twt_cfg.uc_twt_session_enable == 0) {
                    hmac_config_twt_setup_req_auto(&(pst_hmac_vap->st_vap_base_info));
                }
            }
        }
    }

    return OAL_SUCC;
}
#endif
#endif /* _PRE_WLAN_FEATURE_TWT */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


