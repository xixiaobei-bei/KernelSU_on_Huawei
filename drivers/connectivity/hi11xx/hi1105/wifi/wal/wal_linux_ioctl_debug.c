

#ifdef _PRE_WLAN_CFGID_DEBUG

#include "oal_ext_if.h"
#include "oal_util.h"

#include "oam_ext_if.h"
#include "frw_ext_if.h"

#include "wlan_spec.h"
#include "wlan_types.h"

#include "mac_vap.h"
#include "mac_resource.h"
#include "mac_regdomain.h"
#include "mac_ie.h"
#include "hmac_ext_if.h"
#include "hmac_chan_mgmt.h"
#include "wal_main.h"
#include "wal_ext_if.h"
#include "wal_config.h"
#include "wal_regdb.h"
#include "wal_linux_scan.h"
#include "wal_linux_ioctl.h"
#include "wal_linux_bridge.h"
#include "wal_linux_flowctl.h"
#include "wal_linux_atcmdsrv.h"
#include "wal_linux_event.h"
#include "wlan_chip_i.h"
#include "hmac_resource.h"
#include "hmac_p2p.h"

#include "wal_linux_cfg80211.h"
#include "wal_dfx.h"

#include "oal_hcc_host_if.h"
#include "plat_cali.h"

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include <linux/notifier.h>
#include <linux/inetdevice.h>
#include <net/addrconf.h>
#endif
#include "hmac_arp_offload.h"
#include "hmac_auto_adjust_freq.h"
#include "hmac_roam_main.h"
#include "hmac_twt.h"
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "plat_pm_wlan.h"
#endif

#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
#include "board.h"
#endif
#include "securec.h"
#include "wal_cfg_ioctl.h"
#include "wal_linux_netdev_ops.h"
#ifdef _PRE_WLAN_FEATURE_DFR
#include "hmac_dfx.h"
#endif  // _PRE_WLAN_FEATURE_DFR

#undef THIS_FILE_ID
#define THIS_FILE_ID      OAM_FILE_ID_WAL_LINUX_IOCTL_DEBUG_C
#define MAX_PRIV_CMD_SIZE 4096
#define MCM_CONFIG_CHAIN_STEP 10
#define MCM_CHAIN_NUM_MAX 4

extern const int8_t *g_pauc_tx_dscr_param_name[];
extern const int8_t *g_pauc_tx_pow_param_name[];
extern const wal_ioctl_alg_cfg_stru g_ast_alg_cfg_map[];
extern const wal_ioctl_dyn_cali_stru g_ast_dyn_cali_cfg_map[];
extern const wal_ioctl_tlv_stru g_ast_set_tlv_table[];


OAL_STATIC uint32_t wal_hipriv_feature_log_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    mac_vap_stru *pst_mac_vap = NULL;
    oam_feature_enum_uint8 en_feature_id;
    uint8_t uc_switch_vl;
    uint32_t off_set;
    int8_t ac_param[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    oam_log_level_enum_uint8 en_log_lvl;
    wal_msg_write_stru st_write_msg;

    /* OAM log模块的开关的命令: hipriv "Hisilicon0[vapx] feature_log_switch {feature_name} {0/1}"
       1-2(error与warning)级别日志以vap级别为维度；
    */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (oal_unlikely(pst_mac_vap == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::null pointer.}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 获取特性名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param += off_set;

    /* 提供特性名的帮助信息 */
    if ('?' == ac_param[0]) {
        oal_io_print("please input abbr feature name. \r\n");
        oam_show_feature_list();
        return OAL_SUCC;
    }

    /* 获取特性ID */
    ret = oam_get_feature_id((uint8_t *)ac_param, &en_feature_id);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::invalid feature name}\r\n");
        return ret;
    }

    /* 获取开关值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_param, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    pc_param += off_set;

    /* 获取INFO级别开关状态 */
    if ((oal_strcmp("0", ac_param) != 0) && (oal_strcmp("1", ac_param) != 0)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::invalid switch value}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    uc_switch_vl = (uint8_t)oal_atoi(ac_param);

    /* 关闭INFO日志级别时，恢复成默认的日志级别 */
    en_log_lvl = (uc_switch_vl == OAL_SWITCH_ON) ? OAM_LOG_LEVEL_INFO : OAM_LOG_DEFAULT_LEVEL;
    ret = oam_log_set_feature_level(pst_mac_vap->uc_vap_id, en_feature_id, en_log_lvl);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_FEATURE_LOG, sizeof(int32_t));
    *((uint16_t *)(st_write_msg.auc_value)) = ((en_feature_id << BIT_OFFSET_8) | en_log_lvl);
    ret |= (uint32_t)wal_send_cfg_event(pst_net_dev,
                                        WAL_MSG_TYPE_WRITE,
                                        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                                        (uint8_t *)&st_write_msg,
                                        OAL_FALSE,
                                        NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_feature_log_switch::return err code[%d]!}\r\n", ret);
        return ret;
    }

    return ret;
}


OAL_STATIC wal_tx_pow_param_enum wal_get_tx_pow_param(int8_t *pc_param, uint8_t uc_param_len)
{
    wal_tx_pow_param_enum en_param_index;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_TX_POW_PARAM_BUTT; en_param_index++) {
        if (uc_param_len <= OAL_STRLEN(g_pauc_tx_pow_param_name[en_param_index])) {
            return WAL_TX_POW_PARAM_BUTT;
        }

        if (!oal_strcmp(g_pauc_tx_pow_param_name[en_param_index], pc_param)) {
            break;
        }
    }

    return en_param_index;
}


OAL_STATIC void wal_get_tx_pow_log_param(mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param, int8_t *pc_param)
{
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;

    if (wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::show log arg1 missing!}");
        return;
    }
    pst_set_tx_pow_param->auc_value[1] = (uint8_t)oal_atoi(ac_arg);

    if (pst_set_tx_pow_param->auc_value[0] == 0) {
        // pow
        if (pst_set_tx_pow_param->auc_value[1] >= HAL_POW_RATE_CODE_TBL_LEN) {
            // rate_idx
            oam_error_log1(0, OAM_SF_ANY,
                           "{wal_hipriv_set_tx_pow_param::err rate idx [%d]!}",
                           pst_set_tx_pow_param->auc_value[1]);
            pst_set_tx_pow_param->auc_value[1] = 0;
        }
    } else {
        // evm
        pc_param += off_set;
        if (wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::evm arg err input!}");
            return;
        }
        // chn_idx
        pst_set_tx_pow_param->auc_value[BYTE_OFFSET_2] = (uint8_t)oal_atoi(ac_arg);
    }

    return;
}

OAL_STATIC void wal_tx_pow_param_set_sar_level_case(mac_cfg_set_tx_pow_param_stru *set_tx_pow_param,
    uint8_t value)
{
#ifdef _PRE_WLAN_FEATURE_TPC_OPT
    uint16_t data_len;
    oam_warning_log1(0, OAM_SF_TPC, "{wal_tx_pow_param_set_sar_level_case::input reduce SAR level[%d]!}", value);
    if (wlan_chip_get_sar_ctrl_params(value, set_tx_pow_param->auc_value, &data_len,
        sizeof(set_tx_pow_param->auc_value)) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_tx_pow_param_set_sar_level_case::memcpy fail!}");
    }
#endif
}

OAL_STATIC uint32_t wal_tx_pow_param_tas_pow_ctrl_case(
    mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param, int8_t *pc_param,
    uint8_t uc_value)
{
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    mac_device_stru *pst_mac_device;
    mac_cfg_tas_pwr_ctrl_stru st_tas_pow_ctrl_params;

    pst_mac_device = mac_res_get_dev(0);
    /* 如果非单VAP,则不处理 */
    if (mac_device_calc_up_vap_num(pst_mac_device) > 1) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_set_tx_pow_param::up vap more than 1 vap");
        return OAL_FAIL;
    }

    memset_s(&st_tas_pow_ctrl_params, sizeof(mac_cfg_tas_pwr_ctrl_stru),
             0, sizeof(mac_cfg_tas_pwr_ctrl_stru));
    st_tas_pow_ctrl_params.uc_core_idx = !!(uc_value);

    if (g_tas_switch_en[!!uc_value] == OAL_FALSE) {
        return OAL_FAIL;
    }

    /* 获取下一个参数 */
    if (wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set) != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_tx_pow_param_tas_pow_ctrl_case::TAS pwr ctrl core or needimproved lost return err_code [%d]!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    st_tas_pow_ctrl_params.en_need_improved = (uint8_t)!!oal_atoi(ac_arg);
    if (memcpy_s(pst_set_tx_pow_param->auc_value, sizeof(pst_set_tx_pow_param->auc_value),
        &st_tas_pow_ctrl_params, sizeof(mac_cfg_tas_pwr_ctrl_stru)) != EOK) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_tx_pow_param_tas_pow_ctrl_case::memcpy_s fail");
    }

    oam_warning_log2(0, OAM_SF_ANY,
        "{wal_hipriv_set_tx_pow_param::WAL_TX_POW_PARAM_TAS_POW_CTRL core[%d] improved_flag[%d]!}\r\n",
                     st_tas_pow_ctrl_params.uc_core_idx, st_tas_pow_ctrl_params.en_need_improved);

    return OAL_SUCC;
}
OAL_STATIC uint32_t wal_hipriv_check_param(mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param,
    int8_t *pc_param, wal_tx_pow_param_enum en_param_index)
{
    int32_t l_ret = OAL_SUCC;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    uint8_t value = pst_set_tx_pow_param->auc_value[0];

    switch (en_param_index) {
        case WAL_TX_POW_PARAM_SET_RF_REG_CTL:
        case WAL_TX_POW_PARAM_SET_NO_MARGIN:
            if (value >= 2) { /* 参数个数小于2 */
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            break;
        case WAL_TX_POW_PARAM_SET_FIX_LEVEL:
        case WAL_TX_POW_PARAM_SET_MAG_LEVEL:
        case WAL_TX_POW_PARAM_SET_CTL_LEVEL:
            if (value > 4) { /* 参数个数不大于4 */
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
            break;

        case WAL_TX_POW_PARAM_SET_SHOW_LOG:
            /* to reduce cycle complexity */
            wal_get_tx_pow_log_param(pst_set_tx_pow_param, pc_param);
            break;

        case WAL_TX_POW_PARAM_SET_AMEND:
        case WAL_TX_POW_PARAM_SHOW_TPC_TABLE_GAIN:
        case WAL_TX_POW_POW_SAVE:
        case WAL_TX_POW_SET_TPC_IDX:
            if (OAL_SUCC == wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set)) {
                pst_set_tx_pow_param->auc_value[1] = (uint8_t)oal_atoi(ac_arg);
            }
            break;

        case WAL_TX_POW_PARAM_SET_SAR_LEVEL:
            wal_tx_pow_param_set_sar_level_case(pst_set_tx_pow_param, value);
            break;

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
        case WAL_TX_POW_PARAM_TAS_POW_CTRL:
            if (wal_tx_pow_param_tas_pow_ctrl_case(pst_set_tx_pow_param, pc_param, value) != OAL_SUCC) {
                oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_check_param::wal_tx_pow_param_tas_pow_ctrl_case fail");
                return OAL_FAIL;
            }
            break;

        case WAL_TX_POW_PARAM_TAS_RSSI_MEASURE:
            if (g_tas_switch_en[!!value] == OAL_FALSE) {
                oam_error_log1(0, OAM_SF_ANY,
                    "wal_hipriv_set_tx_pow_param::WAL_TX_POW_PARAM_TAS_RSSI_MEASURE core[%d]!", value);
                return OAL_FAIL;
            }
            break;

        case WAL_TX_POW_PARAM_TAS_ANT_SWITCH:
            if (oal_any_true_value2(g_tas_switch_en[WLAN_RF_CHANNEL_ZERO], g_tas_switch_en[WLAN_RF_CHANNEL_ONE])) {
                /* 0:默认态 1:tas态 */
                oam_warning_log1(0, OAM_SF_ANY,
                    "{wal_hipriv_set_tx_pow_param::CMD_SET_MEMO_CHANGE antIndex[%d].}", value);
                l_ret = board_wifi_tas_set(value);
            }
            return (uint32_t)l_ret;
#endif
        case WAL_TX_POW_GET_PD_INFO:
#ifdef _PRE_WLAN_FEATURE_FULL_QUAN_PROD_CAL
            /* 表示当前开始产线全量校准的数据采集,获取下一个参数状态 */
            if (pst_set_tx_pow_param->auc_value[0] == 1) {
                if (OAL_SUCC == wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set)) {
                    pst_set_tx_pow_param->auc_value[1] = (uint8_t)oal_atoi(ac_arg);
                }
            }
            oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::pdet info param[%d %d]!}",
                             pst_set_tx_pow_param->auc_value[0], pst_set_tx_pow_param->auc_value[1]);
#endif
            break;

        default:
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::error input type!}\r\n");
            return OAL_FAIL;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_set_tx_pow_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_cfg_set_tx_pow_param_stru *pst_set_tx_pow_param = NULL;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    wal_tx_pow_param_enum en_param_index;

    /* 命令格式: hipriv "vap0 set_tx_pow rf_reg_ctl 0/1" ,   0:不使能, 1:使能            */
    /* 命令格式: hipriv "vap0 set_tx_pow fix_level 0/1/2/3"  设置数据帧功率等级, 仅data0 */
    /* 命令格式: hipriv "vap0 set_tx_pow mag_level 0/1/2/3"  设置管理帧功率等级          */
    /* 命令格式: hipriv "vap0 set_tx_pow ctl_level 0/1/2/3"  设置控制帧功率等级          */
    /* 命令格式: hipriv "vap0 set_tx_pow amend <value>"      修正upc code                */
    /* 命令格式: hipriv "vap0 set_tx_pow no_margin"          功率不留余量设置, 仅51用    */
    /* 命令格式: hipriv "vap0 set_tx_pow show_log type (bw chn)" 显示功率维测日志
       type: 0/1 pow/evm
       bw:   rate_idx(pow) 0/1/2 20M/80M/160M(evm)
       chn:  0~6(20M/80M) 0/1(160M)  */
    /* 命令格式: hipriv "vap0 set_tx_pow sar_level 0/1/2/3"  设置降sar等级               */
    /* 命令格式: hipriv "vap0 set_tx_pow tas_pwr_ctrl 0/1 0/1" tas功率控制               */
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_TX_POW, sizeof(mac_cfg_set_tx_pow_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_tx_pow_param = (mac_cfg_set_tx_pow_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_set_tx_pow_param, sizeof(mac_cfg_set_tx_pow_param_stru), 0, sizeof(mac_cfg_set_tx_pow_param_stru));

    /* 获取描述符字段设置命令字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::wal_get_cmd_one_arg return err_code[%d]!}", ret);
        return ret;
    }

    /* 解析是设置哪一个字段 */
    en_param_index = wal_get_tx_pow_param(ac_arg, sizeof(ac_arg));
    if (en_param_index == WAL_TX_POW_PARAM_BUTT) { /* 检查命令是否打错 */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::no such param for tx pow!}\r\n");
        return OAL_FAIL;
    }

    pst_set_tx_pow_param->en_type = en_param_index;

    /* 获取下一个参数 */
    pc_param += off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::wal_get_cmd_one_arg return err_code[%d]!}", ret);
    } else {
        pst_set_tx_pow_param->auc_value[0] = (uint8_t)oal_atoi(ac_arg);
        pc_param += off_set;
    }

    /* 参数校验 */
    ret = wal_hipriv_check_param(pst_set_tx_pow_param, pc_param, en_param_index);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_set_tx_pow_param::wal_hipriv_check_param fail");
        return ret;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_tx_pow_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tx_pow_param::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_ucast_data_dscr_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set, ret;
    int32_t l_ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param = NULL;
    wal_dscr_param_enum en_param_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /* 获取描述符字段设置命令字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::wal_get_cmd_one_arg err[%d]}", ret);
        return ret;
    }
    pc_param += off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_hipriv_set_ucast_data_dscr_param::CMD ERR!cmd type::wlan0 set_ucast_data CMD VALUE!}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->ta: set tx rts antenna![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->ra: set rx cts/ack/ba antenna![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->cc: set channel code![0][BCC] or [1][LDPC]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->power: set lpf/pa/upc/dac!}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->shortgi: set tx dscr short gi or long gi![0] or [1] }");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->preamble: set tx dscr preamble mode![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->rtscts: set tx dscr enable rts or not![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->lsigtxop: set tx dscr enable lsigtxop or not![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->smooth: set rx channel matrix with smooth or not![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->snding:set sounding mode![0][NON]-[1][NDP]-[2][STAGGERD]-[3][LEGACY]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->txbf: set txbf mode![0][NON]-[1][EXPLICIT]-[2][LEGACY]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->stbc: set STBC or not![0] or [1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->rd_ess: expand spatial stream}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->dyn_bw:set rts/cts dynamic signaling or not![0][STATIC]-[1][DYNAMIC]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->dyn_bw_exist: set tx dscr dynamic when no ht exist!set[1]] }");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->ch_bw_exist: set tx dscr dynamic when no ht exist!set[1]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->rate: set 11a/b/g rate as rate table}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->mcs: set 11n rate as rate table mcs index}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->mcsac: set 11ac rate as rate table mcs index}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->mcsax: set 11ax rate as rate table mcs index}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->mcsax_er: set 11ax_er rate as rate table mcs index}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->nss: set 11ac nss mode [1][SINGLE]-[2][DOUBLE]-[3][TRIBLE]-[4][QUAD]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->bw: set tx dscr channel bw![20][20M]-[40][40M]-[d40][40MDUP]- \
            [80][80M]-[d80][80MDUP]-[160][160M]-[d160][160MDUP]-[80_80][80+80M]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->ltf: set tx dscr ltf [0][1*]-[1][2*]-[2][4*]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->gi: set tx dscr gi [0][lgi]-[1][sgi]-[2][mgi]}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->txchain: set tx dscr txchain![1]CH0-[2]CH1-[3]DOUBLE}");
        oam_warning_log0(0, OAM_SF_ANY, "{CMD-->dcm: set tx dscr enable dcm![0] or [1] }");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 配置速率、空间流数、带宽 */
    if (en_param_index >= WAL_DSCR_PARAM_RATE && en_param_index <= WAL_DSCR_PARAM_BW) {
        ret = wal_hipriv_process_rate_params(pst_net_dev, pc_param, pst_set_dscr_param);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_ucast_data_dscr_param::wal_hipriv_process_ucast_params return err_code [%d]!}", ret);
            return ret;
        }
    } else {
        /* 解析要设置为多大的速率 */
        ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_ucast_data_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}", ret);
            return ret;
        }
        pc_param += off_set;
        pst_set_dscr_param->l_value = oal_strtol(ac_arg, NULL, 0);
    }

    /* 单播数据帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_UCAST_DATA;
    /*lint -e571*/
    oam_warning_log2(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::en_param_index [%d]!,value[%d]}",
        pst_set_dscr_param->uc_function_index, pst_set_dscr_param->l_value);
    /*lint +e571*/
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_DSCR, sizeof(mac_cfg_set_dscr_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_dscr_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_data_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_bcast_data_dscr_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param = NULL;
    wal_dscr_param_enum en_param_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_DSCR, sizeof(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /* 获取描述符字段设置命令字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_bcast_data_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_bcast_data_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }
    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 配置速率、空间流数、带宽 */
    if (en_param_index >= WAL_DSCR_PARAM_RATE && en_param_index <= WAL_DSCR_PARAM_BW) {
        ret = wal_hipriv_process_rate_params(pst_net_dev, pc_param, pst_set_dscr_param);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_bcast_data_dscr_param::wal_hipriv_process_ucast_params return err_code [%d]!}\r\n",
                ret);
            return ret;
        }
    } else {
        /* 解析要设置为多大的速率 */
        ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_bcast_data_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }
        pc_param += off_set;
        pst_set_dscr_param->l_value = oal_strtol(ac_arg, NULL, 0);
    }

    /* 广播数据帧描述符设置 tpye = MAC_VAP_CONFIG_BCAST_DATA */
    pst_set_dscr_param->en_type = MAC_VAP_CONFIG_BCAST_DATA;

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_dscr_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_bcast_data_dscr_param::return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_ucast_mgmt_dscr_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param = NULL;
    wal_dscr_param_enum en_param_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_band;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_DSCR, sizeof(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /***************************************************************************
             sh hipriv.sh "vap0 set_ucast_mgmt data0 2 8389137"
    ***************************************************************************/
    /* 解析data0 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ucast_mgmt_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 解析要设置为哪个频段的单播管理帧 2G or 5G */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ucast_mgmt_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    uc_band = (uint8_t)oal_atoi(ac_arg);
    /* 单播管理帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_MGMT 2为2G,否则为5G  */
    if (uc_band == WLAN_BAND_2G) {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_UCAST_MGMT_2G;
    } else {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_UCAST_MGMT_5G;
    }

    /* 配置速率、空间流数、带宽 */
    if (en_param_index >= WAL_DSCR_PARAM_RATE && en_param_index <= WAL_DSCR_PARAM_BW) {
        ret = wal_hipriv_process_rate_params(pst_net_dev, pc_param, pst_set_dscr_param);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_ucast_mgmt_dscr_param::wal_hipriv_process_ucast_params return err_code [%d]!}", ret);
            return ret;
        }
    } else {
        /* 解析要设置为多大的速率 */
        ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_ucast_mgmt_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }
        pc_param += off_set;
        pst_set_dscr_param->l_value = oal_strtol(ac_arg, NULL, 0);
    }

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_dscr_param_stru),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ucast_mgmt_dscr_param::return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_mbcast_mgmt_dscr_param(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_set_dscr_param_stru *pst_set_dscr_param = NULL;
    wal_dscr_param_enum en_param_index;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t uc_band;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_DSCR, sizeof(mac_cfg_set_dscr_param_stru));

    /* 解析并设置配置命令参数 */
    pst_set_dscr_param = (mac_cfg_set_dscr_param_stru *)(st_write_msg.auc_value);

    /***************************************************************************
             sh hipriv.sh "vap0 set_mcast_mgmt data0 5 8389137"
    ***************************************************************************/
    /* 解析data0 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_mbcast_mgmt_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 解析是设置哪一个字段 */
    for (en_param_index = 0; en_param_index < WAL_DSCR_PARAM_BUTT; en_param_index++) {
        if (!oal_strcmp(g_pauc_tx_dscr_param_name[en_param_index], ac_arg)) {
            break;
        }
    }

    /* 检查命令是否打错 */
    if (en_param_index == WAL_DSCR_PARAM_BUTT) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::no such param for tx dscr!}\r\n");
        return OAL_FAIL;
    }

    pst_set_dscr_param->uc_function_index = en_param_index;

    /* 解析要设置为哪个频段的单播管理帧 2G or 5G */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_mbcast_mgmt_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 单播管理帧描述符设置 tpye = MAC_VAP_CONFIG_UCAST_MGMT 2为2G,否则为5G  */
    uc_band = (uint8_t)oal_atoi(ac_arg);
    if (uc_band == WLAN_BAND_2G) {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MBCAST_MGMT_2G;
    } else {
        pst_set_dscr_param->en_type = MAC_VAP_CONFIG_MBCAST_MGMT_5G;
    }

    /* 配置速率、空间流数、带宽 */
    if (en_param_index >= WAL_DSCR_PARAM_RATE && en_param_index <= WAL_DSCR_PARAM_BW) {
        ret = wal_hipriv_process_rate_params(pst_net_dev, pc_param, pst_set_dscr_param);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_mbcast_mgmt_dscr_param::wal_hipriv_process_ucast_params return err_code [%d]!}", ret);
            return ret;
        }
    } else {
        /* 解析要设置为多大的速率 */
        ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_mbcast_mgmt_dscr_param::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }
        pc_param += off_set;

        pst_set_dscr_param->l_value = oal_strtol(ac_arg, NULL, 0);
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_dscr_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_mbcast_mgmt_dscr_param::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_nss(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_tx_comp_stru *pst_set_nss_param = NULL;
    int32_t l_nss;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_idx = 0;
    wal_msg_stru *pst_rsp_msg = NULL;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_NSS, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_nss_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* 获取速率值字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_nss::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 输入命令合法性检测 */
    while (ac_arg[l_idx] != '\0') {
        if (isdigit(ac_arg[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_nss::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 解析要设置为多大的值 */
    l_nss = oal_atoi(ac_arg);
    if (l_nss < WAL_HIPRIV_NSS_MIN || l_nss > WAL_HIPRIV_NSS_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_nss::input val out of range [%d]!}\r\n", l_nss);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    pst_set_nss_param->uc_param = (uint8_t)(l_nss - 1);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_nss::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_freq fail, err code[%u]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_rfch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_tx_comp_stru *pst_set_rfch_param = NULL;
    uint8_t uc_ch;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t c_ch_idx;
    wal_msg_stru *pst_rsp_msg = NULL;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_RFCH, sizeof(mac_cfg_tx_comp_stru));

    /* 解析并设置配置命令参数 */
    pst_set_rfch_param = (mac_cfg_tx_comp_stru *)(st_write_msg.auc_value);

    /* 获取速率值字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_rfch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 解析要设置为多大的值 */
    uc_ch = 0;
    for (c_ch_idx = 0; c_ch_idx < WAL_HIPRIV_CH_NUM; c_ch_idx++) {
        if (ac_arg[c_ch_idx] == '0') {
            continue;
        } else if (ac_arg[c_ch_idx] == '1') {
            uc_ch += (uint8_t)(1 << (WAL_HIPRIV_CH_NUM - (uint8_t)c_ch_idx - 1));
        } else { /* 输入数据有非01数字，或数字少于4位，异常 */
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_rfch::input err!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    /* 输入参数多于四位，异常 */
    if (ac_arg[c_ch_idx] != '\0') {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_rfch::input err!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pst_set_rfch_param->uc_param = uc_ch;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_tx_comp_stru),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_rfch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_rfch fail, err code[%u]!}\r\n", ret);
        return ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_NARROW_BAND

OAL_STATIC uint32_t wal_hipriv_narrow_bw(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    mac_cfg_narrow_bw_stru *pst_nrw_bw;
    uint32_t ret;
    uint32_t off_set;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_NARROW_BW, sizeof(mac_cfg_narrow_bw_stru));

    /* 解析并设置配置命令参数 */
    pst_nrw_bw = (mac_cfg_narrow_bw_stru *)(st_write_msg.auc_value);

    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::get switch  [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;

    pst_nrw_bw->en_open = (uint8_t)oal_atoi(ac_arg);

    /* 窄带打开模式下强制关闭ampdu amsdu聚合和若干扰免疫算法 */
    if (pst_nrw_bw->en_open == OAL_TRUE) {
        wal_hipriv_alg_cfg(pst_net_dev, "anti_inf_unlock_en 0");
        wal_hipriv_ampdu_tx_on(pst_net_dev, "0");
        wal_hipriv_amsdu_tx_on(pst_net_dev, "0");
    } else {
        wal_hipriv_alg_cfg(pst_net_dev, "anti_inf_unlock_en 1");
        wal_hipriv_ampdu_tx_on(pst_net_dev, "1");
        wal_hipriv_amsdu_tx_on(pst_net_dev, "1");
    }

    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::get switch  [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;

    if ((oal_strcmp("1m", ac_arg)) == 0) {
        pst_nrw_bw->en_bw = NARROW_BW_1M;
    } else if ((oal_strcmp("5m", ac_arg)) == 0) {
        pst_nrw_bw->en_bw = NARROW_BW_5M;
    } else if ((oal_strcmp("10m", ac_arg)) == 0) {
        pst_nrw_bw->en_bw = NARROW_BW_10M;
    } else {
        pst_nrw_bw->en_bw = NARROW_BW_BUTT;
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_narrow_bw::bw should be 1/5/10 m");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_narrow_bw_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#endif


OAL_STATIC uint32_t wal_hipriv_ota_rx_dscr_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_param;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    int32_t l_ret;
    uint32_t ret;

    /* OAM ota模块的开关的命令: hipriv "Hisilicon0 ota_rx_dscr_switch 0 | 1"
    */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ret;
    }

    /* 解析参数 */
    ret = wal_get_cmd_one_arg(pc_param + off_set, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::wal_get_cmd_one_arg fails!}\r\n");
        return ret;
    }
    l_param = oal_atoi((const int8_t *)ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_OTA_RX_DSCR_SWITCH, sizeof(uint32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_param;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ota_rx_dscr_switch::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_ether_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_ret;
    uint32_t ret;
    mac_cfg_eth_switch_param_stru st_eth_switch_param;

    /* "vap0 ether_switch user_macaddr oam_ota_frame_direction_type_enum(帧方向) 0|1(开关)" */
    memset_s(&st_eth_switch_param, sizeof(mac_cfg_eth_switch_param_stru),
             0, sizeof(mac_cfg_eth_switch_param_stru));

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, st_eth_switch_param.auc_user_macaddr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ether_switch::wal_hipriv_get_mac_addr return err_code[%d]}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 获取以太网帧方向 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ether_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n", ret);
        return ret;
    }
    pc_param += off_set;
    st_eth_switch_param.en_frame_direction = (uint8_t)oal_atoi(ac_name);

    /* 获取开关 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ether_switch::wal_get_cmd_one_arg return err_code[%d]}\r\n", ret);
        return ret;
    }
    st_eth_switch_param.en_switch = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ETH_SWITCH, sizeof(st_eth_switch_param));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_eth_switch_param, sizeof(st_eth_switch_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_eth_switch_param),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_ether_switch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC void wal_get_ucast_sub_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_ucast_switch,
                                         int8_t *param, uint32_t len)
{
    uint32_t ret;
    uint32_t off_set;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取MSDU描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get msdu dscr switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_msdu_dscr = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;

    /* 获取BA_INFO打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get ba info switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_ba_info = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;

    /* 获取HIMIT描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get himit dscr switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_himit_dscr = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;

    /* 获取MU描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get mu dscr switch return err_code[%d]!}\r\n", ret);
        return;
    }
    param += off_set;
    mac_80211_ucast_switch->sub_switch.stru.bit_mu_dscr = (uint8_t)oal_atoi(name) ? OAL_TRUE : OAL_FALSE;
}

OAL_STATIC uint32_t wal_get_ucast_switch(mac_cfg_80211_ucast_switch_stru *mac_80211_ucast_switch,
                                         int8_t *param, uint32_t len)
{
    uint32_t ret;
    uint32_t off_set;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    /* 获取80211帧方向 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get 80211 ucast frame direction return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_frame_direction = (uint8_t)oal_atoi(name);

    /* 获取帧类型 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get ucast frame type return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_frame_type = (uint8_t)oal_atoi(name);

    /* 获取帧内容打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get frame content switch return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_frame_switch = (uint8_t)oal_atoi(name) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;

    /* 获取帧CB字段打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get frame cb switch return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_cb_switch = (uint8_t)oal_atoi(name) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;

    /* 获取描述符打印开关 */
    ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_get_ucast_switch::get frame dscr switch return err_code[%d]!}\r\n", ret);
        return ret;
    }
    param += off_set;
    mac_80211_ucast_switch->en_dscr_switch = (uint8_t)oal_atoi(name) ? OAL_SWITCH_ON : OAL_SWITCH_OFF;

    wal_get_ucast_sub_switch(mac_80211_ucast_switch, param, OAL_STRLEN(param));

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_80211_ucast_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int32_t l_ret;
    uint32_t ret;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;

    /* sh hipriv.sh "vap0 80211_uc_switch user_macaddr 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
     *  0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关) 0|1(MSDU描述符) 0|1(BA_INFO) 0|1(HIMIT描述符) 0|1(MU描述符)"
    */
    memset_s(&st_80211_ucast_switch, sizeof(mac_cfg_80211_ucast_switch_stru),
             0, sizeof(mac_cfg_80211_ucast_switch_stru));

    /* 获取mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, st_80211_ucast_switch.auc_user_macaddr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_80211_ucast_switch::wal_hipriv_get_mac_addr return err_code[%d]}", ret);
        return ret;
    }
    pc_param += off_set;

    ret = wal_get_ucast_switch(&st_80211_ucast_switch, pc_param, OAL_STRLEN(pc_param));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::get ucast switch fail [%d]!}", ret);
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_80211_UCAST_SWITCH, sizeof(st_80211_ucast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_80211_ucast_switch, sizeof(st_80211_ucast_switch)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_80211_ucast_switch),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_ucast_switch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_set_all_80211_ucast(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t ret;
    mac_cfg_80211_ucast_switch_stru st_80211_ucast_switch;
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;

    /* sh hipriv.sh "Hisilicon0 80211_uc_all 0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧)
     *  0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关) 0|1(MSDU描述符) 0|1(BA_INFO) 0|1(HIMIT描述符) 0|1(MU描述符)"
    */
    memset_s(&st_80211_ucast_switch, sizeof(mac_cfg_80211_ucast_switch_stru),
             0, sizeof(mac_cfg_80211_ucast_switch_stru));
    ret = wal_get_ucast_switch(&st_80211_ucast_switch, pc_param, OAL_STRLEN(pc_param));
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_all_80211_ucast::get ucast switch fail [%d]!}", ret);
        return ret;
    }

    /* 设置广播mac地址 */
    memcpy_s(st_80211_ucast_switch.auc_user_macaddr, WLAN_MAC_ADDR_LEN, BROADCAST_MACADDR, WLAN_MAC_ADDR_LEN);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_80211_UCAST_SWITCH, sizeof(st_80211_ucast_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_80211_ucast_switch, sizeof(st_80211_ucast_switch)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_all_80211_ucast::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_80211_ucast_switch),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_80211_all_ucast_switch::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_set_trlr_info(mac_phy_debug_switch_stru *phy_debug_switch, int8_t c_value)
{
    if (isdigit(c_value)) {
        phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] = (uint8_t)(c_value - '0');
    } else if ((c_value >= 'a') && (c_value <= 'f')) {
        /* 十六进制数'a'转换为十进制数为10 */
        phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] = (uint8_t)(c_value - 'a' + 10);
    } else if ((c_value >= 'A') || (c_value <= 'F')) {
        /* 十六进制数'A'转换为十进制数为10 */
        phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] = (uint8_t)(c_value - 'A' + 10);
    } else {
        oam_error_log0(0, OAM_SF_CFG,
            "{wal_hipriv_set_trlr_info:: param input illegal, should be [0-f/F].!!}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_get_tone_tran_para(int8_t **pc_param, int8_t *ac_value,
                                           uint32_t value_len, mac_phy_debug_switch_stru *pst_phy_debug_switch)
{
    uint32_t ret = 0;
    uint32_t off_set = 0;

    if (0 == oal_strcmp("help", ac_value)) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_get_tone_tran_para:tone tansmit command.!!}\r\n");
        return OAL_SUCC;
    }

    pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch = (uint8_t)oal_atoi(ac_value);
    if (pst_phy_debug_switch->st_tone_tran.uc_tone_tran_switch == 1) {
        ret = wal_get_cmd_one_arg(*pc_param, ac_value, value_len, &off_set);
        if ((ret != OAL_SUCC) || (off_set == 0)) {
            oam_error_log0(0, OAM_SF_CFG, "{wal_get_tone_tran_para:chain index is illegal'!!}\r\n");
            return ret;
        }
        *pc_param += off_set;
        pst_phy_debug_switch->st_tone_tran.uc_chain_idx = (uint16_t)oal_atoi(ac_value);

        if ((pst_phy_debug_switch->st_tone_tran.uc_chain_idx != 0) &&
            (pst_phy_debug_switch->st_tone_tran.uc_chain_idx != 1)) {
            oam_error_log1(0, OAM_SF_CFG,
                           "{wal_get_tone_tran_para:chain index[%d] is invalid'!!}\r\n",
                           pst_phy_debug_switch->st_tone_tran.uc_chain_idx);
            return OAL_FAIL;
        }

        /*  获取数据长度  */
        ret = wal_get_cmd_one_arg(*pc_param, ac_value, value_len, &off_set);
        if ((ret != OAL_SUCC) || (off_set == 0) || (oal_atoi(ac_value) == 0)) {
            oam_error_log0(0, OAM_SF_CFG, "{wal_get_tone_tran_para:tone data len is illegal'!!}\r\n");
            return ret;
        }
        *pc_param += off_set;
        pst_phy_debug_switch->st_tone_tran.us_data_len = (uint16_t)oal_atoi(ac_value);
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_print_phy_debug_help_info(int8_t *pc_param, uint8_t uc_para_len)
{
    if (uc_para_len < OAL_STRLEN("help")) {
        return OAL_FAIL;
    }

    if (oal_strcmp("help", pc_param) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[rssi]Print the rssi of rx packets, \
                         reported from rx dscr of MAC, range [-128 ~ +127] dBm.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[tsensor]Print the code of T-sensor.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[snr]Print snr values of two rx ants, \
                         11B not included, reported from rx dscr of MAC, range [-10 ~ +55] dBm.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[evm]Print evm values of two rx ants, \
                         11B not included, reported from rx dscr of MAC, range [-10 ~ +55] dBm.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG,
                         "{CMD[spatial_reuse]Print Sptial Reuse of MAC srg and non-srg frame time and rssi!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG, "{CMD[trlr/vect]Sel range::trailer [0~f/F], vector [0~7], \
                         eg1: trlr 01234 eg2: vec123 trlr12, sum of both is less than 5.!!}\r\n");
        oam_warning_log0(0, OAM_SF_CFG,
                         "{CMD[count]Set the interval of print (packets), range [0 ~ 2^32].!!}\r\n");

        return OAL_SUCC;
    }

    return OAL_FAIL;
}

static uint32_t wal_set_rssi_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_rssi_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_tsensor_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_tsensor_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_snr_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_snr_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_evm_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_evm_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_sp_reuse_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_sp_reuse_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_trlr_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch,
                                          int8_t *name, int8_t *value)
{
    uint32_t ret = OAL_SUCC;
    uint8_t data_cnt = 0;
    while (value[data_cnt] != '\0') {
        /* 输入参数合法性检查 */
        if (phy_debug_switch->uc_trlr_sel_num >= WAL_PHY_DEBUG_TEST_WORD_CNT) {
            oam_error_log1(0, 0, "{Param input illegal, cnt [%d] reached 5!!}", phy_debug_switch->uc_trlr_sel_num);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }

        ret = wal_hipriv_set_trlr_info(phy_debug_switch, value[data_cnt]);
        if (ret != OAL_SUCC) {
            return ret;
        }

        if (oal_strcmp("vect", name) == 0) {
            if (value[data_cnt] > '7') {
                oam_error_log0(0, OAM_SF_CFG, "{param input illegal, vect should be [0-7].!!}\r\n");
                return OAL_ERR_CODE_INVALID_CONFIG;
            }
        } else {
            /* vector bit4为0，trailer的bit4置1, 设置寄存器可以一并带上 */
            phy_debug_switch->auc_trlr_sel_info[phy_debug_switch->uc_trlr_sel_num] |= 0x10;
        }

        data_cnt++;
        phy_debug_switch->uc_trlr_sel_num++;
    }

    /* 若输入正常，则打开trailer开关 */
    if (data_cnt != 0) {
        phy_debug_switch->en_trlr_debug_switch = OAL_TRUE;
    }
    return ret;
}

static uint32_t wal_set_comp_isr_interval(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->rx_comp_isr_interval = (uint32_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_iq_cali_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_iq_cali_switch = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_tone_tran_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t **param,
                                         int8_t *value)
{
    uint32_t ret;
    ret = wal_get_tone_tran_para(param, value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, phy_debug_switch);
    if (ret != OAL_SUCC) {
        return ret;
    }
    return ret;
}

static uint32_t wal_set_pdet_debug_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->en_pdet_debug_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_force_work_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_force_work_switch = ((uint8_t)oal_atoi(value));
    return OAL_SUCC;
}

static uint32_t wal_set_dfr_reset_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_dfr_reset_switch = ((uint8_t)oal_atoi(value));
    return OAL_SUCC;
}

static uint32_t wal_set_fsm_info_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_fsm_info_switch = ((uint8_t)oal_atoi(value)) & OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_report_radar_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_report_radar_switch = OAL_TRUE;
    return OAL_SUCC;
}

static uint32_t wal_set_extlna_chg_bypass_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_extlna_chg_bypass_switch = ((uint8_t)oal_atoi(value));
    return OAL_SUCC;
}

static uint32_t wal_set_edca_param_switch(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= ((uint8_t)oal_atoi(value)) << BIT_OFFSET_4;
    return OAL_SUCC;
}

static uint32_t wal_set_edca_aifsn(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT3;
    phy_debug_switch->uc_edca_aifsn = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_edca_cwmin(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT2;
    phy_debug_switch->uc_edca_cwmin = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_edca_cwmax(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT1;
    phy_debug_switch->uc_edca_cwmax = (uint8_t)oal_atoi(value);
    return OAL_SUCC;
}

static uint32_t wal_set_edca_txoplimit(mac_phy_debug_switch_stru *phy_debug_switch, int8_t *value)
{
    phy_debug_switch->uc_edca_param_switch |= (uint8_t)BIT0;
    phy_debug_switch->us_edca_txoplimit = (uint16_t)oal_atoi(value);
    return OAL_SUCC;
}

typedef struct {
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t (*wal_set_debug_switch_case)(mac_phy_debug_switch_stru *phy_debug_switch,
                                          int8_t *ac_value);
} wal_set_debug_switch_ops;

uint32_t wal_hipriv_checklist_command(mac_phy_debug_switch_stru *phy_debug_switch, int8_t **pc_param,
    int8_t *ac_name, int8_t *ac_value)
{
    uint32_t ret = 0;
    uint8_t idx;
    const wal_set_debug_switch_ops wal_set_debug_switch_ops_table[] = {
        { "rssi",          wal_set_rssi_debug_switch },
        { "tsensor",       wal_set_tsensor_debug_switch },
        { "snr",           wal_set_snr_debug_switch },
        { "evm",           wal_set_evm_debug_switch },
        { "spatial_reuse", wal_set_sp_reuse_debug_switch },
        { "count",         wal_set_comp_isr_interval },
        { "iq_cali",       wal_set_iq_cali_switch },
        { "pdet",          wal_set_pdet_debug_switch },
        { "force_work",    wal_set_force_work_switch },
        { "dfr_reset",     wal_set_dfr_reset_switch },
        { "fsm_info",      wal_set_fsm_info_switch },
        { "report_radar",  wal_set_report_radar_switch },
        { "extlna_bypass", wal_set_extlna_chg_bypass_switch },
        { "edca",          wal_set_edca_param_switch },
        { "aifsn",         wal_set_edca_aifsn },
        { "cwmin",         wal_set_edca_cwmin },
        { "cwmax",         wal_set_edca_cwmax },
        { "txoplimit",     wal_set_edca_txoplimit },
    };

    /* 查询各个命令 */
    for (idx = 0; idx < (sizeof(wal_set_debug_switch_ops_table) / sizeof(wal_set_debug_switch_ops)); ++idx) {
        if (oal_strcmp(wal_set_debug_switch_ops_table[idx].name, ac_name) == 0) {
            ret = wal_set_debug_switch_ops_table[idx].wal_set_debug_switch_case(phy_debug_switch,
                                                                                ac_value);
            return ret;
        }
    }
    if (idx == (sizeof(wal_set_debug_switch_ops_table) / sizeof(wal_set_debug_switch_ops))) {
        if (oal_any_zero_value2(oal_strcmp("trlr", ac_name), oal_strcmp("vect", ac_name))) {
            ret = wal_set_trlr_debug_switch(phy_debug_switch, ac_name, ac_value);
        } else if(oal_strcmp("tone_tran", ac_name) == 0) {
            ret = wal_set_tone_tran_switch(phy_debug_switch, pc_param, ac_value);
        } else {
            oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 phy_debug [rssi 0|1] [snr 0|1] \
            [trlr xxxx] [vect yyyy] [count N] [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] [txoplimit N]'!!}\r\n");
            return OAL_FAIL;
        }
    }
    return ret;
}

uint32_t wal_hipriv_set_phy_debug_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    mac_phy_debug_switch_stru st_phy_debug_switch;
    uint32_t ret = 0;
    int32_t l_ret;
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;

    /* sh hipriv.sh "wlan0 phy_debug snr 0|1(关闭|打开) rssi 0|1(关闭|打开) trlr 1234a count N(每个N个报文打印一次)" */
    memset_s(&st_phy_debug_switch, sizeof(st_phy_debug_switch), 0, sizeof(st_phy_debug_switch));

    st_phy_debug_switch.rx_comp_isr_interval = 10;  // 如果没有设置，则默认10个包打印一次，命令码可以更新
    /* 1103mpw2上rx hder占用一个，1105解决频偏问题占用一个（固定上报rx trailor 13）因此这里从1开始计数 */
    st_phy_debug_switch.uc_trlr_sel_num = 1;
    st_phy_debug_switch.uc_force_work_switch = 0xff;
    st_phy_debug_switch.st_tone_tran.uc_tone_tran_switch = 0xF; /*  默认单音发送不处于发送/关闭状态  */
    st_phy_debug_switch.uc_dfr_reset_switch = 0xff;
    st_phy_debug_switch.uc_extlna_chg_bypass_switch = 0xff;
    st_phy_debug_switch.uc_edca_param_switch = 0x0;

    do {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_set_phy_debug_switch::cmd format err, ret:%d;!!}\r\n", ret);
            return ret;
        }
        pc_param += off_set;

        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (off_set == 0) {
            break;
        }

        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 phy_debug [rssi 0|1] [snr 0|1] \
                [trlr xxxxx] [vect yyyyy] [count N] [edca 0-3(tid_no)] [aifsn N] [cwmin N] [cwmax N] [txoplimit N]'!}");
            return ret;
        }
        pc_param += off_set;
        off_set = 0;

        /* 打印help信息 */
        ret = wal_hipriv_print_phy_debug_help_info(ac_value, sizeof(ac_value));
        if (ret == OAL_SUCC) {
            return ret;
        }

        /* 查询各个命令 */
        ret = wal_hipriv_checklist_command(&st_phy_debug_switch, &pc_param, ac_name, ac_value);
        if (ret != OAL_SUCC) {
            return ret;
        }
    } while (*pc_param != '\0');

    /* 将打印总开关保存到 */
    st_phy_debug_switch.en_debug_switch = st_phy_debug_switch.en_rssi_debug_switch |
        st_phy_debug_switch.en_snr_debug_switch | st_phy_debug_switch.en_trlr_debug_switch |
        st_phy_debug_switch.uc_iq_cali_switch | st_phy_debug_switch.en_tsensor_debug_switch |
        st_phy_debug_switch.en_evm_debug_switch;

    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_set_phy_debug_switch:: phy_debug switch [%d].}\r\n",
        st_phy_debug_switch.en_debug_switch);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PHY_DEBUG_SWITCH, sizeof(st_phy_debug_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_phy_debug_switch, sizeof(st_phy_debug_switch)) != EOK) {
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_phy_debug_switch), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_phy_debug_switch::return err code[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC OAL_INLINE void protocol_debug_cmd_format_info(void)
{
    oam_warning_log0(0, OAM_SF_ANY, "{CMD format::sh hipriv.sh 'wlan0 protocol_debug\
                    [band_force_switch 0|1|2(20M|40M+|40M-)]\
                    [2040_ch_swt_prohi 0|1]\
                    [40_intol 0|1]'!!}\r\n");
    oam_warning_log0(0, OAM_SF_ANY, "{[csa 0(csa mode) 1(csa channel) 10(csa cnt) 1(debug  flag, \
                     0:normal channel channel,1:only include csa ie 2:cannel debug)] \
                     [2040_user_switch 0|1]'!!}\r\n");
    oam_warning_log0(0, OAM_SF_ANY, "[lsig 0|1]'!!}\r\n");
}


OAL_STATIC uint32_t wal_protocol_debug_parase_csa_cmd(int8_t *pc_param,
    mac_protocol_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t uc_value;

    *pul_offset = 0;
    /* 解析csa mode */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::get csa mode error,return.}");
        return ret;
    }
    uc_value = (uint8_t)oal_atoi(ac_value);
    if (uc_value > 1) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::csa mode=[%d] invalid,return.}", uc_value);
        return OAL_FAIL;
    }
    *pul_offset += off_set;
    pst_debug_info->st_csa_debug_bit3.en_mode = uc_value;
    pc_param += off_set;
    off_set = 0;

    /* 解析csa channel */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::get csa channel error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_csa_debug_bit3.uc_channel = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析bandwidth */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::get bandwidth error,return.}");
        return ret;
    }
    uc_value = (uint8_t)oal_atoi(ac_value);
    if (uc_value >= WLAN_BAND_WIDTH_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::invalid bandwidth=%d,return.}", uc_value);
        return OAL_FAIL;
    }
    *pul_offset += off_set;
    pst_debug_info->st_csa_debug_bit3.en_bandwidth = uc_value;
    pc_param += off_set;
    off_set = 0;

    /* 解析csa cnt */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::get csa cnt error,return.}");
        return ret;
    }
    uc_value = (uint8_t)oal_atoi(ac_value);
    if (uc_value >= 255) { /* uint8_t最大取值为255 */
        uc_value = 255; /* uint8_t最大取值为255 */
    }
    *pul_offset += off_set;
    pst_debug_info->st_csa_debug_bit3.uc_cnt = uc_value;
    pc_param += off_set;
    off_set = 0;

    /* 解析debug flag */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::get debug flag error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    uc_value = (uint8_t)oal_atoi(ac_value);
    if (uc_value >= MAC_CSA_FLAG_BUTT) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_protocol_debug_parase_csa_cmd::invalid debug flag=%d,return.}", uc_value);
        return OAL_FAIL;
    }
    pst_debug_info->st_csa_debug_bit3.en_debug_flag = (mac_csa_flag_enum_uint8)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_protocol_debug_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    mac_protocol_debug_switch_stru st_protocol_debug;
    uint32_t ret = 0;
    int32_t l_ret;
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;

    /* sh hipriv.sh "wlan0 protocol_debug band_force_switch 0|1|2(20|40-|40+)
       2040_ch_swt_prohi 0|1(关闭|打开) 2040_intolerant 0|1(关闭|打开)" */
    memset_s(&st_protocol_debug, sizeof(st_protocol_debug), 0, sizeof(st_protocol_debug));

    do {
        /* 获取命令关键字 */
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            protocol_debug_cmd_format_info();
            return ret;
        }
        pc_param += off_set;

        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (off_set == 0) {
            break;
        }

        /* 命令分类 */
        if (oal_strcmp("csa", ac_name) == 0) {
            ret = wal_protocol_debug_parase_csa_cmd(pc_param, &st_protocol_debug, &off_set);
            if (ret != OAL_SUCC) {
                protocol_debug_cmd_format_info();
                return ret;
            }
            pc_param += off_set;
            off_set = 0;
            st_protocol_debug.cmd_bit_map |= BIT3;
        } else if (g_wlan_spec_cfg->feature_11ax_is_open && oal_strcmp("11ax", ac_name) == 0) {
            st_protocol_debug.cmd_bit_map |= BIT6;
        } else {
            /* 取命令配置值 */
            ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
            if ((ret != OAL_SUCC) || oal_value_not_in_valid_range(ac_value[0], '0', '9')) {
                protocol_debug_cmd_format_info();
                return ret;
            }
            pc_param += off_set;
            off_set = 0;

            if (oal_strcmp("band_force_switch", ac_name) == 0) {
                st_protocol_debug.en_band_force_switch_bit0 = ((uint8_t)oal_atoi(ac_value));
                st_protocol_debug.cmd_bit_map |= BIT0;
            } else if (oal_strcmp("2040_ch_swt_prohi", ac_name) == 0) {
                /* 填写结构体 */
                st_protocol_debug.en_2040_ch_swt_prohi_bit1 = ((uint8_t)oal_atoi(ac_value)) & BIT0;
                st_protocol_debug.cmd_bit_map |= BIT1;
            } else if (oal_strcmp("40_intol", ac_name) == 0) {
                /* 填写结构体 */
                st_protocol_debug.en_40_intolerant_bit2 = ((uint8_t)oal_atoi(ac_value)) & BIT0;
                st_protocol_debug.cmd_bit_map |= BIT2;
            } else if (oal_strcmp("lsig", ac_name) == 0) {
                /* 填写结构体 */
                st_protocol_debug.en_lsigtxop_bit5 = ((uint8_t)oal_atoi(ac_value)) & BIT0;
                st_protocol_debug.cmd_bit_map |= BIT5;
            } else {
                protocol_debug_cmd_format_info();
                return OAL_FAIL;
            }
        }
    } while (*pc_param != '\0');

    oam_warning_log1(0, OAM_SF_ANY,
        "{wal_hipriv_show_protocol_debug_info::cmd_bit_map: 0x%08x.}", st_protocol_debug.cmd_bit_map);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_PROTOCOL_DBG, sizeof(st_protocol_debug));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_protocol_debug, sizeof(st_protocol_debug)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_show_protocol_debug_info::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_protocol_debug),
                               (uint8_t *)&st_write_msg, OAL_FALSE,  NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_show_protocol_debug_info::return err code[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_input_srb_switch(mac_pm_debug_cfg_stru *pm_debug_cfg, int8_t *value)
{
    uint8_t srb_switch;
    if (oal_strcmp("help", value) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug srb [0|1]'}");
        return OAL_SUCC;
    }

    srb_switch = (uint8_t)oal_atoi(value);
    if (srb_switch > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{CMD format::input srb switch val[%d] invalid!!}", srb_switch);
        return OAL_FAIL;
    }
    pm_debug_cfg->cmd_bit_map |= BIT(MAC_PM_DEBUG_SISO_RECV_BCN);
    pm_debug_cfg->uc_srb_switch = srb_switch;
    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_input_srb_switch::siso recv beacon switch[%d]}", srb_switch);
    return OAL_SUCC_GO_ON;
}

OAL_STATIC uint32_t wal_hipriv_input_dto_switch(mac_pm_debug_cfg_stru *pm_debug_cfg, int8_t *value)
{
    uint8_t dto_switch;
    if (oal_strcmp("help", value) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug dto [0|1]'}");
        return OAL_SUCC;
    }

    dto_switch = (uint8_t)oal_atoi(value);
    if (dto_switch > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{CMD format::input dto switch val[%d] invalid!!}", dto_switch);
        return OAL_FAIL;
    }
    pm_debug_cfg->cmd_bit_map |= BIT(MAC_PM_DEBUG_DYN_TBTT_OFFSET);
    pm_debug_cfg->uc_dto_switch = dto_switch;
    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_input_dto_switch::dyn tbtt offset switch[%d].}", dto_switch);
    return OAL_SUCC_GO_ON;
}

OAL_STATIC uint32_t wal_hipriv_input_nfi_switch(mac_pm_debug_cfg_stru *pm_debug_cfg, int8_t *value)
{
    uint8_t nfi_switch;
    if (oal_strcmp("help", value) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug nfi [0|1]'}");
        return OAL_SUCC;
    }

    nfi_switch = (uint8_t)oal_atoi(value);
    if (nfi_switch > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{CMD format::input nfi switch val[%d] invalid!!}", nfi_switch);
        return OAL_FAIL;
    }
    pm_debug_cfg->cmd_bit_map |= BIT(MAC_PM_DEBUG_NO_PS_FRM_INT);
    pm_debug_cfg->uc_nfi_switch = nfi_switch;
    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_input_nfi_switch::no ps frm int switch[%d].}", nfi_switch);
    return OAL_SUCC_GO_ON;
}

OAL_STATIC uint32_t wal_hipriv_input_apf_switch(mac_pm_debug_cfg_stru *pm_debug_cfg, int8_t *value)
{
    uint8_t apf_switch;
    if (oal_strcmp("help", value) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug apf [0|1]'}");
        return OAL_SUCC;
    }

    apf_switch = (uint8_t)oal_atoi(value);
    if (apf_switch > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{CMD format::input apf switch val[%d] invalid!!}", apf_switch);
        return OAL_FAIL;
    }
    pm_debug_cfg->cmd_bit_map |= BIT(MAC_PM_DEBUG_APF);
    pm_debug_cfg->uc_apf_switch = apf_switch;
    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_debug_cmp_switch::apf switch[%d].}", apf_switch);
    return OAL_SUCC_GO_ON;
}

OAL_STATIC uint32_t wal_hipriv_input_ao_switch(mac_pm_debug_cfg_stru *pm_debug_cfg, int8_t *value)
{
    uint8_t ao_switch;
    if (oal_strcmp("help", value) == 0) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug ao [0|1]'}");
        return OAL_SUCC;
    }

    ao_switch = (uint8_t)oal_atoi(value);
    if (ao_switch > 1) {
        oam_error_log1(0, OAM_SF_CFG, "{CMD format::input ao switch val[%d] invalid!!}", ao_switch);
        return OAL_FAIL;
    }
    pm_debug_cfg->cmd_bit_map |= BIT(MAC_PM_DEBUG_AO);
    pm_debug_cfg->uc_ao_switch = ao_switch;
    oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_input_ao_switch::arp offload switch[%d].}", ao_switch);
    return OAL_SUCC_GO_ON;
}

OAL_STATIC uint32_t wal_hipriv_debug_cmp_switch(mac_pm_debug_cfg_stru *pm_debug_cfg,
    int8_t *value, uint32_t value_len, int8_t *name, uint32_t name_len)
{
    uint32_t ret = OAL_SUCC_GO_ON;
    if (name == NULL || value_len <= 0 || name_len <= 0 ||
        value_len > WAL_HIPRIV_CMD_VALUE_MAX_LEN || name_len > WAL_HIPRIV_CMD_NAME_MAX_LEN) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_debug_cmp_switch::error input!}");
    }
    if (oal_strcmp("srb", name) == 0) {
        ret = wal_hipriv_input_srb_switch(pm_debug_cfg, value);
    } else if (oal_strcmp("dto", name) == 0) {
        ret = wal_hipriv_input_dto_switch(pm_debug_cfg, value);
    } else if (oal_strcmp("nfi", name) == 0) {
        ret = wal_hipriv_input_nfi_switch(pm_debug_cfg, value);
    } else if (oal_strcmp("apf", name) == 0) {
        ret = wal_hipriv_input_apf_switch(pm_debug_cfg, value);
    } else if (oal_strcmp("ao", name) == 0) {
        ret = wal_hipriv_input_ao_switch(pm_debug_cfg, value);
    } else {
        oam_error_log0(0, OAM_SF_CFG,
            "{CMD format::sh hipriv.sh 'wlan0 pm_debug [srb 0|1] [dto 0|1] [nfi 0|1] [apf 0|1]'!!}");
        return OAL_FAIL;
    }
    if (ret != OAL_SUCC_GO_ON) {
        return ret;
    }
    return OAL_SUCC_GO_ON;
}


OAL_STATIC uint32_t wal_hipriv_set_pm_debug_switch(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    mac_vap_stru *mac_vap = NULL;
    uint32_t ret;
    int32_t l_ret;
    uint32_t offset = 0;
    int8_t cmd_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t cfg_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    mac_pm_debug_cfg_stru *pm_debug_cfg = NULL;
    oal_bool_enum_uint8 cmd_updata = OAL_FALSE;

    /* sh hipriv.sh "wlan0 pm_debug srb " */
    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_pm_debug_switch::mac_vap is null!}");
        return OAL_FAIL;
    }

    pm_debug_cfg = (mac_pm_debug_cfg_stru *)(write_msg.auc_value);
    memset_s(pm_debug_cfg, sizeof(mac_pm_debug_cfg_stru), 0, sizeof(mac_pm_debug_cfg_stru));

    do {
        ret = wal_get_cmd_one_arg(param, cmd_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if ((ret != OAL_SUCC) && (offset != 0)) {
            oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_set_pm_debug_switch::cmd format err, ret:%d;!!}\r\n", ret);
            return ret;
        }
        param += offset;

        if (cmd_updata == OAL_FALSE) {
            cmd_updata = OAL_TRUE;
        } else if (offset == 0) {
            break;
        }

        ret = wal_get_cmd_one_arg(param, cfg_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &offset);
        if ((ret != OAL_SUCC) || ((!isdigit(cfg_value[0])) && (strcmp("help", cfg_value) != 0))) {
            oam_error_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 pm_debug [srb 0|1]'!!}\r\n");
            return ret;
        }
        param += offset;
        ret = wal_hipriv_debug_cmp_switch(pm_debug_cfg, cfg_value, sizeof(cfg_value), cmd_name, sizeof(cmd_name));
        if (ret != OAL_SUCC_GO_ON) {
            return ret;
        }
    } while (*param != '\0');

    /***************************************************************************
                               抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_PM_DEBUG_SWITCH, sizeof(mac_pm_debug_cfg_stru));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_pm_debug_cfg_stru), (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::return err code[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_dbdc_debug_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    mac_dbdc_debug_switch_stru st_dbdc_debug_switch;
    mac_vap_stru *pst_mac_vap = NULL;
    uint32_t ret = 0;
    int32_t l_ret;
    uint8_t en_dbdc_enable;
    oal_bool_enum_uint8 en_cmd_updata = OAL_FALSE;

    /* sh hipriv.sh "wlan0 dbdc_debug change_hal_dev 0|1(hal 0|hal 1)" */
    memset_s(&st_dbdc_debug_switch, sizeof(st_dbdc_debug_switch), 0, sizeof(st_dbdc_debug_switch));

    pst_mac_vap = oal_net_dev_priv(pst_net_dev);
    if (pst_mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::pst_mac_vap is null!}");
        return OAL_FAIL;
    }

    do {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            oam_warning_log1(0,
                OAM_SF_CFG, "{wal_hipriv_set_dbdc_debug_switch::cmd format err, ret:%d;!!}\r\n", ret);
            return ret;
        }
        pc_param += off_set;

        if (en_cmd_updata == OAL_FALSE) {
            en_cmd_updata = OAL_TRUE;
        } else if (off_set == 0) {
            break;
        }

        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) || (!isdigit(ac_value[0]))) {
            oal_io_print("CMD format::sh hipriv.sh 'wlan0 dbdc_debug [change_hal_dev 0|1]'\r\n");
            oam_error_log0(0, OAM_SF_CFG,
                "{CMD format::sh hipriv.sh 'wlan0 dbdc_debug [change_hal_dev 0|1]'!!}\r\n");
            return ret;
        }
        pc_param += off_set;
        off_set = 0;

        if (oal_strcmp("dbdc_enable", ac_name) == 0) {
            en_dbdc_enable = (uint8_t)oal_atoi(ac_value);
            if (en_dbdc_enable > 1) {
                oam_error_log1(0, OAM_SF_CFG,
                    "{CMD format::sh hipriv.sh 'wlan0 dbdc_debug [change_hal_dev 0|1],input[%d]'!!}", en_dbdc_enable);
                return OAL_FAIL;
            }
            st_dbdc_debug_switch.cmd_bit_map |= BIT(MAC_DBDC_SWITCH);
            st_dbdc_debug_switch.uc_dbdc_enable = ((uint8_t)oal_atoi(ac_value));

            oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_CFG,
                "{wal_hipriv_set_dbdc_debug_switch::dbdc enable[%d].}", st_dbdc_debug_switch.uc_dbdc_enable);
        } else {
            oal_io_print("CMD format::sh hipriv.sh 'wlan0 dbdc_debug [change_hal_dev 0|1]'");
            oam_error_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 dbdc_debug [change_hal_dev 0|1]'!!}");
            return OAL_FAIL;
        }
    } while (*pc_param != '\0');

    /***************************************************************************
                            抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DBDC_DEBUG_SWITCH, sizeof(st_dbdc_debug_switch));

    /* 设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_dbdc_debug_switch, sizeof(st_dbdc_debug_switch)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_dbdc_debug_switch),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_dbdc_debug_switch::return err code[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_addba_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_addba_req_param_stru *pst_addba_req_param = NULL;
    mac_cfg_addba_req_param_stru st_addba_req_param; /* 临时保存获取的addba req的信息 */
    uint32_t get_addr_idx;

    /*
     * 设置AMPDU关闭的配置命令:
     * hipriv "Hisilicon0 addba_req xx xx xx xx xx xx(mac地址) tidno ba_policy buffsize timeout"
     */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    memset_s((uint8_t *)&st_addba_req_param, sizeof(st_addba_req_param), 0, sizeof(st_addba_req_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_addba_req_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取tid */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* tid最大占2字节 */
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_hipriv_addba_req::the addba req command is error}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_addba_req_param.uc_tidno = (uint8_t)oal_atoi(ac_name);
    if (st_addba_req_param.uc_tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_addba_req::the addba req command is error!uc_tidno is [%d]!}", st_addba_req_param.uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;

    /* 获取ba_policy */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_addba_req_param.en_ba_policy = (uint8_t)oal_atoi(ac_name);
    if (st_addba_req_param.en_ba_policy != MAC_BA_POLICY_IMMEDIATE) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_addba_req::ba policy is not correct! ba_policy is[%d]!}", st_addba_req_param.en_ba_policy);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;

    /* 获取buffsize */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_addba_req_param.us_buff_size = (uint16_t)oal_atoi(ac_name);

    pc_param = pc_param + off_set;

    /* 获取timeout时间 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_addba_req_param.us_timeout = (uint16_t)oal_atoi(ac_name);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADDBA_REQ, sizeof(mac_cfg_addba_req_param_stru));

    /* 设置配置命令参数 */
    pst_addba_req_param = (mac_cfg_addba_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_addba_req_param->auc_mac_addr[get_addr_idx] = st_addba_req_param.auc_mac_addr[get_addr_idx];
    }

    pst_addba_req_param->uc_tidno = st_addba_req_param.uc_tidno;
    pst_addba_req_param->en_ba_policy = st_addba_req_param.en_ba_policy;
    pst_addba_req_param->us_buff_size = st_addba_req_param.us_buff_size;
    pst_addba_req_param->us_timeout = st_addba_req_param.us_timeout;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_addba_req_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_addba_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_delba_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_delba_req_param_stru *pst_delba_req_param = NULL;
    mac_cfg_delba_req_param_stru st_delba_req_param; /* 临时保存获取的addba req的信息 */
    uint32_t get_addr_idx;

    /*
     * 设置AMPDU关闭的配置命令:
     * hipriv "Hisilicon0 delba_req xx xx xx xx xx xx(mac地址) tidno direction reason_code"
     */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    memset_s((uint8_t *)&st_delba_req_param, sizeof(st_delba_req_param), 0, sizeof(st_delba_req_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_delba_req_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取tid */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    if (OAL_STRLEN(ac_name) > 2) { /* tid最大占2字节 */
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_hipriv_delba_req::the delba_req req command is error!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    st_delba_req_param.uc_tidno = (uint8_t)oal_atoi(ac_name);
    if (st_delba_req_param.uc_tidno >= WLAN_TID_MAX_NUM) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_delba_req::the delba_req req command is error! tidno[%d]!}", st_delba_req_param.uc_tidno);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;

    /* 获取direction */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    st_delba_req_param.en_direction = (uint8_t)oal_atoi(ac_name);
    if (st_delba_req_param.en_direction >= MAC_BUTT_DELBA) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_delba_req::the direction is not correct! direction[%d]!}", st_delba_req_param.en_direction);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DELBA_REQ, sizeof(mac_cfg_delba_req_param_stru));

    /* 设置配置命令参数 */
    pst_delba_req_param = (mac_cfg_delba_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_delba_req_param->auc_mac_addr[get_addr_idx] = st_delba_req_param.auc_mac_addr[get_addr_idx];
    }

    pst_delba_req_param->uc_tidno = st_delba_req_param.uc_tidno;
    pst_delba_req_param->en_direction = st_delba_req_param.en_direction;
    pst_delba_req_param->en_trigger = MAC_DELBA_TRIGGER_COMM;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_delba_req_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delba_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WMMAC

OAL_STATIC uint32_t wal_hipriv_wmmac_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    uint8_t uc_wmmac_switch;
    mac_cfg_wmm_ac_param_stru st_wmm_ac_param;

    /* 设置删除TS的配置命令: hipriv "vap0 wmmac_switch 1/0(使能) 0|1(WMM_AC认证使能) AC xxx(limit_medium_time)" */
    memset_s(&st_wmm_ac_param, sizeof(mac_cfg_wmm_ac_param_stru), 0, sizeof(mac_cfg_wmm_ac_param_stru));
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    uc_wmmac_switch = (uint8_t)oal_atoi(ac_name);
    if (uc_wmmac_switch != OAL_FALSE) {
        uc_wmmac_switch = OAL_TRUE;
    }
    st_wmm_ac_param.en_wmm_ac_switch = uc_wmmac_switch;

    /* 获取auth flag */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.en_auth_flag = (uint8_t)oal_atoi(ac_name);
    pc_param += off_set;

    /* timeout period ms */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.us_timeout_period = (uint16_t)oal_atoi(ac_name);
    pc_param += off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_wmmac_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    st_wmm_ac_param.uc_factor = (uint8_t)oal_atoi(ac_name);
    pc_param += off_set;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 设置配置命令参数 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WMMAC_SWITCH, sizeof(st_wmm_ac_param));
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 (const void *)&st_wmm_ac_param, sizeof(st_wmm_ac_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_delts::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_wmm_ac_param),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_delts::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#endif  // #ifdef _PRE_WLAN_FEATURE_WMMAC

OAL_STATIC uint32_t wal_hipriv_memory_info(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    uint32_t ret;
    int32_t l_ret;
    uint8_t uc_meminfo_type = MAC_MEMINFO_BUTT;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    wal_msg_write_stru st_write_msg;
    mac_cfg_meminfo_stru *pst_meminfo_param = NULL;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_CFG, "{CMD format::sh hipriv.sh 'wlan0 memoryinfo \
            [all|dscr|netbuff|user|vap|sdio_sch_q] [pool_usage/pool_debug 0|1, ..7<pool_id>]'!}\r\n");
        return ret;
    }

    if (oal_strcmp("host", ac_name) == 0) {
        oal_mem_print_pool_info();
        return OAL_SUCC;
    } else if (oal_strcmp("device", ac_name) == 0) {
        hcc_print_device_mem_info();
        return OAL_SUCC;
    } else if (oal_strcmp("all", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_ALL;
    } else if (oal_strcmp("dscr", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_DSCR;
    } else if (oal_strcmp("netbuff", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_NETBUFF;
    } else if (oal_strcmp("user", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_USER;
    } else if (oal_strcmp("vap", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_VAP;
    } else if (oal_strcmp("sdio_sch_q", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_SDIO_TRX;
    } else if (oal_strcmp("pool_usage", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_POOL_INFO;
    } else if (oal_strcmp("pool_debug", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_POOL_DBG;
    } else if (oal_strcmp("sample_alloc", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_SAMPLE_ALLOC;
    } else if (oal_strcmp("sample_free", ac_name) == 0) {
        uc_meminfo_type = MAC_MEMINFO_SAMPLE_FREE;
    } else {
        oam_warning_log0(0, OAM_SF_ANY,
            "{wal_hipriv_memory_info::wal_get_cmd_one_arg::second arg:: please check input!}\r\n");
        return OAL_FAIL;
    }

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEVICE_MEM_INFO, sizeof(mac_cfg_meminfo_stru));
    pst_meminfo_param = (mac_cfg_meminfo_stru *)(st_write_msg.auc_value);
    pst_meminfo_param->uc_meminfo_type = uc_meminfo_type;
    /* host和device mempool个数不一致 ，这里用0xff给个default值 */
    pst_meminfo_param->uc_object_index = 0xff;

    if ((uc_meminfo_type == MAC_MEMINFO_POOL_INFO) || (uc_meminfo_type == MAC_MEMINFO_POOL_DBG) ||
        (uc_meminfo_type == MAC_MEMINFO_SAMPLE_ALLOC)) {
        pc_param = pc_param + off_set;

        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        /* 没有后续参数不退出 */
        if (ret == OAL_SUCC) {
            pst_meminfo_param->uc_object_index = (uint8_t)oal_atoi(ac_name);
        }
    }

    l_ret = wal_send_cfg_event(pst_cfg_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_meminfo_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_memory_info::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_threshold_type(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t off_set)
{
    /* 3.获取第三个参数 */
    uint32_t ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.en_btcoex_nss = (wlan_nss_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第四个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg4 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.uc_20m_low = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第五个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg5 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.uc_20m_high = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第六个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg6 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.uc_40m_low = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第七个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg7 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.threhold.us_40m_high = (uint16_t)oal_atoi(name);
    return OAL_SUCC;
}


uint32_t wal_hipriv_aggregate_size_type(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t off_set)
{
    /* 3.获取第三个参数 */
    uint32_t ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.en_btcoex_nss = (wlan_nss_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第四个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_grade = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第五个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg4 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size0 = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第六个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg5 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size1 = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第七个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg6 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size2 = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第八个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg7 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rx_size.uc_rx_size3 = (uint8_t)oal_atoi(name);
    return OAL_SUCC;
}


uint32_t wal_hipriv_rssi_detect_type(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t off_set)
{
    /* 3.获取第三个参数 */
    uint32_t ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.en_rssi_limit_on = (oal_bool_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第四个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg6 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.en_rssi_log_on = (oal_bool_enum_uint8)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第五个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg3 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.uc_cfg_rssi_detect_cnt = (uint8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第六个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg4 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.c_cfg_rssi_detect_mcm_down_th = (int8_t)oal_atoi(name);

    param = param + off_set; /* 偏移，取下一个参数 */

    /* 获取第七个参数: */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg5 err[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->pri_data.rssi_param.c_cfg_rssi_detect_mcm_up_th = (int8_t)oal_atoi(name);
    return OAL_SUCC;
}


uint32_t wal_hipriv_perf_param_cfg(mac_btcoex_mgr_stru *btcoex_mgr, int8_t *param, int8_t *name, uint32_t name_len)
{
    uint32_t off_set = 0;
    uint32_t ret;

    if (name_len != WAL_HIPRIV_CMD_NAME_MAX_LEN) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::name_len[%d]!}", name_len);
        return OAL_FAIL;
    }

    /* 1.获取第一个参数: mode */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, name_len, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg1 err_code[%d]!}", ret);
        return ret;
    }

    btcoex_mgr->uc_cfg_btcoex_mode = (uint8_t)oal_atoi(name);

    if (btcoex_mgr->uc_cfg_btcoex_mode == 0) {
    } else if (btcoex_mgr->uc_cfg_btcoex_mode == 1) {
        param = param + off_set; /* 偏移，取下一个参数 */

        /* 2.获取第二个参数 */
        ret = wal_get_cmd_one_arg((int8_t *)param, name, name_len, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param::get_cmd_one_arg2 err_code[%d]!}", ret);
            return ret;
        }

        btcoex_mgr->uc_cfg_btcoex_type = (uint8_t)oal_atoi(name);
        if (btcoex_mgr->uc_cfg_btcoex_type > 2) { /* 2表示rssi detect门限参数配置模式 */
            oam_warning_log1(0, OAM_SF_COEX,
                "{wal_hipriv_btcoex_set_perf_param:: btcoex_mgr->uc_cfg_btcoex_type error [%d], [0/1/2]!}\r\n",
                btcoex_mgr->uc_cfg_btcoex_type);
            return OAL_ERR_CODE_INVALID_CONFIG;
        }

        param = param + off_set; /* 偏移，取下一个参数 */

        if (btcoex_mgr->uc_cfg_btcoex_type == 0) {
            ret = wal_hipriv_threshold_type(btcoex_mgr, param, name, off_set);
        } else if (btcoex_mgr->uc_cfg_btcoex_type == 1) {
            ret = wal_hipriv_aggregate_size_type(btcoex_mgr, param, name, off_set);
        } else {
            ret = wal_hipriv_rssi_detect_type(btcoex_mgr, param, name, off_set);
        }
        if (ret != OAL_SUCC) {
            return ret;
        }
    } else {
        oam_warning_log1(0, OAM_SF_COEX,
            "{wal_hipriv_btcoex_set_perf_param::btcoex_mgr->uc_cfg_btcoex_mode err_code [%d]!}\r\n",
            btcoex_mgr->uc_cfg_btcoex_mode);
        return OAL_FAIL;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_btcoex_set_perf_param(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int32_t l_ret;
    uint32_t ret;
    mac_btcoex_mgr_stru *btcoex_mgr = NULL;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* write_msg作清零操作 */
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));

    /* 设置配置命令参数 */
    btcoex_mgr = (mac_btcoex_mgr_stru *)write_msg.auc_value;

    /* 获取事件参数 */
    ret = wal_hipriv_perf_param_cfg(btcoex_mgr, param, name, sizeof(name));
    if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_BTCOEX_SET_PERF_PARAM, sizeof(mac_btcoex_mgr_stru));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_btcoex_mgr_stru),
        (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_COEX, "{wal_hipriv_btcoex_set_perf_param:: return err code = [%d].}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_dpd_cfg(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dpd_cfg::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DPD, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reg_write::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_NRCOEX

OAL_STATIC uint32_t wal_hipriv_nrcoex_cfg_test(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_nrcoex_cfg_test::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_NRCOEX_TEST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reg_write::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif


uint32_t wal_hipriv_set_txpower(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    int32_t l_pwer;
    uint32_t off_set;
    int8_t ac_val[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_idx = 0;
    wal_msg_stru *pst_rsp_msg = NULL;

    ret = wal_get_cmd_one_arg(pc_param, ac_val, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_txpower::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ret);
        return ret;
    }
    if (ac_val[0] == '-') {
        l_idx++;
    }

    /* 输入命令合法性检测 */
    while (ac_val[l_idx] != '\0') {
        if (isdigit(ac_val[l_idx])) {
            l_idx++;
            continue;
        } else {
            l_idx++;
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::input illegal!}\r\n");
            return OAL_ERR_CODE_INVALID_CONFIG;
        }
    }

    l_pwer = oal_atoi(ac_val);
    /* 超过10倍最大传输功率或1/10最小传输功率，都是异常参数 */
    if (l_pwer >= WLAN_MAX_TXPOWER * 10 || l_pwer <= WLAN_MIN_TXPOWER * 10) { /* 参数异常: 功率限制大于1W */
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::invalid argument!}");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::val[%d]!}", l_pwer);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TX_POWER, sizeof(int32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_pwer;
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_txpower::return err code %d!}", l_ret);
        return (uint32_t)l_ret;
    }
    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_bw fail, err code[%u]!}\r\n", ret);
    }

    return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))

OAL_STATIC uint32_t wal_ioctl_set_beacon_interval(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_beacon_interval;
    uint32_t off_set;
    int8_t ac_beacon_interval[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;

    /* 设备在up状态不允许配置，必须先down */
    if (0 != (OAL_IFF_RUNNING & oal_netdevice_flags(pst_net_dev))) {
        oam_error_log1(0, OAM_SF_CFG,
            "{wal_ioctl_set_beacon_interval::device is busy, please down it firs %d!}\r\n",
            oal_netdevice_flags(pst_net_dev));
        return OAL_ERR_CODE_CONFIG_BUSY;
    }

    /* pc_param指向新创建的net_device的name, 将其取出存放到ac_name中 */
    ret = wal_get_cmd_one_arg(pc_param, ac_beacon_interval, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_ioctl_set_beacon_interval::wal_get_cmd_one_arg vap name return err_code %d!}\r\n", ret);
        return ret;
    }

    l_beacon_interval = oal_atoi(ac_beacon_interval);
    oam_info_log1(0, OAM_SF_ANY, "{wal_ioctl_set_beacon_interval::l_beacon_interval = %d!}\r\n", l_beacon_interval);

    /***************************************************************************
        抛事件到wal层处理
    ***************************************************************************/
    /* 填写消息 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_BEACON_INTERVAL, sizeof(int32_t));
    *((int32_t *)(st_write_msg.auc_value)) = l_beacon_interval;

    /* 发送消息 */
    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_ioctl_set_beacon_interval::return err code %d!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_start_vap(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    oam_error_log0(0, OAM_SF_CFG, "DEBUG:: priv start enter.");
    wal_netdev_open(pst_net_dev, OAL_FALSE);
    return OAL_SUCC;
}
#endif


OAL_STATIC uint32_t wal_hipriv_start_scan(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint8_t uc_is_p2p0_scan;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_START_SCAN, sizeof(int32_t));

    uc_is_p2p0_scan = (oal_memcmp(pst_net_dev->name, "p2p0", OAL_STRLEN("p2p0")) == 0) ? 1 : 0;
    st_write_msg.auc_value[0] = uc_is_p2p0_scan;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(int32_t),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_start_scan::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_kick_user(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_kick_user_param_stru *pst_kick_user_param = NULL;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 去关联1个用户的命令 hipriv "vap0 kick_user xx:xx:xx:xx:xx:xx" */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_kick_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_KICK_USER, sizeof(mac_cfg_kick_user_param_stru));

    /* 设置配置命令参数 */
    pst_kick_user_param = (mac_cfg_kick_user_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_kick_user_param->auc_mac_addr, auc_mac_addr);

    /* 填写去关联reason code */
    pst_kick_user_param->us_reason_code = MAC_UNSPEC_REASON;

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_kick_user_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_kick_user::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_send_bar(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_pause_tid_param_stru *pst_pause_tid_param = NULL;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };
    uint8_t uc_tid;

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_send_bar::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_send_bar::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    uc_tid = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SEND_BAR, sizeof(mac_cfg_pause_tid_param_stru));

    /* 设置配置命令参数 */
    pst_pause_tid_param = (mac_cfg_pause_tid_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_pause_tid_param->auc_mac_addr, auc_mac_addr);
    pst_pause_tid_param->uc_tid = uc_tid;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_pause_tid_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_bar::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_amsdu_tx_on(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t aggr_tx_on;
    uint32_t cmd_id;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_amsdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    aggr_tx_on = oal_atoi(ac_name);

    ret = wal_get_cmd_id("amsdu_tx_on", &cmd_id, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_amsdu_tx_on:find amsdu_tx_on cmd is fail");
        return ret;
    }
    return wal_process_cmd_params(pst_net_dev, cmd_id, &aggr_tx_on);
}

OAL_STATIC uint32_t wal_hipriv_wmm_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_cfg_rst;
    uint16_t us_len;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t uc_open_wmm;

    /* 获取设定的值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_wmm_switch::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    uc_open_wmm = (uint8_t)oal_atoi(ac_name);
    pc_param += off_set;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    us_len = sizeof(uint8_t);
    *(uint8_t *)(st_write_msg.auc_value) = uc_open_wmm;
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WMM_SWITCH, us_len);

    l_cfg_rst = wal_send_cfg_event(pst_net_dev,
                                   WAL_MSG_TYPE_WRITE,
                                   WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                   (uint8_t *)&st_write_msg,
                                   OAL_FALSE,
                                   NULL);
    if (oal_unlikely(l_cfg_rst != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_wmm_switch::return err code [%d]!}\r\n", l_cfg_rst);
        return (uint32_t)l_cfg_rst;
    }

    return OAL_SUCC;
}

uint32_t wal_hipriv_ampdu_tx_on(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_ampdu_tx_on_param_stru *pst_aggr_tx_on_param = NULL;
    uint8_t uc_aggr_tx_on;
    uint8_t uc_snd_type = 0;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_ampdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    uc_aggr_tx_on = (uint8_t)oal_atoi(ac_name);
    /* 只有硬件聚合需要配置第二参数 */
    if ((uc_aggr_tx_on & BIT3) || (uc_aggr_tx_on & BIT2) || (uc_aggr_tx_on & BIT1)) {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_ampdu_tx_on::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }

        uc_snd_type = (uint8_t)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_AMPDU_TX_ON, sizeof(mac_cfg_ampdu_tx_on_param_stru));

    /* 设置配置命令参数 */
    pst_aggr_tx_on_param = (mac_cfg_ampdu_tx_on_param_stru *)(st_write_msg.auc_value);
    pst_aggr_tx_on_param->uc_aggr_tx_on = uc_aggr_tx_on;
    if (uc_snd_type > 1) {
        pst_aggr_tx_on_param->uc_snd_type = 1;
        pst_aggr_tx_on_param->en_aggr_switch_mode = AMPDU_SWITCH_BY_BA_LUT;
    } else {
        pst_aggr_tx_on_param->uc_snd_type = uc_snd_type;
        pst_aggr_tx_on_param->en_aggr_switch_mode = AMPDU_SWITCH_BY_DEL_BA;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ampdu_tx_on_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ampdu_tx_on::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_HIEX

OAL_STATIC uint32_t wal_hipriv_set_user_hiex_enable(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru             st_write_msg;
    uint32_t                     off_set;
    int8_t                       ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t                     ret;
    int32_t                      l_ret;
    mac_cfg_user_hiex_param_stru  *pst_user_hiex_param = NULL;
    uint8_t                      auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };
    uint32_t                     hiex_flag;

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_user_hiex_enable::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_user_hiex_enable::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    hiex_flag = oal_strtol(ac_name, NULL, NUM_16_BITS);

    /* 抛事件到wal层处理 */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_USER_HIEX_ENABLE, sizeof(mac_cfg_user_hiex_param_stru));

    /* 设置配置命令参数 */
    pst_user_hiex_param = (mac_cfg_user_hiex_param_stru *)(st_write_msg.auc_value);
    oal_set_mac_addr(pst_user_hiex_param->auc_mac_addr, auc_mac_addr);
    *(uint32_t *)&pst_user_hiex_param->st_hiex_cap = hiex_flag;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_user_hiex_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_user_hiex_enable::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t _wal_hipriv_set_user_hiex_enable(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_hiex_is_open) {
        return wal_hipriv_set_user_hiex_enable(pst_net_dev, pc_param);
    }
    return OAL_SUCC;
}
#endif


OAL_STATIC uint32_t wal_hipriv_reset_device(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_reset_device::memcpy fail}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RESET_HW, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reset_device::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_reset_operate(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;

    if (oal_unlikely(WAL_MSG_WRITE_MAX_LEN <= OAL_STRLEN(pc_param))) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_reset_operate:: pc_param overlength is %d}\n", OAL_STRLEN(pc_param));
        return OAL_FAIL;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_reset_operate::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_RESET_HW_OPERATE, us_len);

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_reset_operate::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_DFT_STAT

OAL_STATIC uint32_t wal_hipriv_usr_queue_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    mac_cfg_usr_queue_param_stru st_usr_queue_param;

    /* sh hipriv.sh "vap_name usr_queue_stat XX:XX:XX:XX:XX:XX 0|1" */
    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    memset_s((uint8_t *)&st_usr_queue_param, sizeof(st_usr_queue_param), 0, sizeof(st_usr_queue_param));

    /* 获取用户mac地址 */
    ret = wal_hipriv_get_mac_addr(pc_param, st_usr_queue_param.auc_user_macaddr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::wal_hipriv_get_mac_addr return [%d].}", ret);
        return ret;
    }
    pc_param += off_set;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_usr_queue_stat::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    st_usr_queue_param.uc_param = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_USR_QUEUE_STAT, sizeof(st_usr_queue_param));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 &st_usr_queue_param, sizeof(st_usr_queue_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_usr_queue_stat::memcpy fail!}");
        return OAL_FAIL;
    }

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(st_usr_queue_param),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_usr_queue_stat::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return (uint32_t)ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_report_all_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    uint16_t us_len;

    /* sh hipriv.sh "Hisilicon0 reprt_all_stat type(phy/machw/mgmt/irq/all)  0|1" */
    /* 获取repot类型 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
        pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_report_all_stat::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';
    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALL_STAT, us_len);

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_report_all_stat::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif


OAL_STATIC uint32_t wal_hipriv_set_ampdu_aggr_num(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_cfg_aggr_num_stru aggr_num_ctl = { 0 };
    uint32_t ret;

    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ampdu_aggr_num::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    aggr_num_ctl.uc_aggr_num_switch = (uint8_t)oal_atoi(ac_name);
    if (aggr_num_ctl.uc_aggr_num_switch == 0) {
        /* 不指定聚合个数时，聚合个数恢复为0 */
        aggr_num_ctl.uc_aggr_num = 0;
    } else {
        /* 获取聚合个数 */
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_ampdu_aggr_num::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }

        aggr_num_ctl.uc_aggr_num = (uint8_t)oal_atoi(ac_name);
        /*
         * 超过聚合最大限制判断
         * 受限于03/05 rom化, 该hipriv命令不进行修改, 06使用时暂时只能配置64以下的聚合数
         */
        if (aggr_num_ctl.uc_aggr_num > g_wlan_spec_cfg->max_tx_ampdu_num) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_set_ampdu_aggr_num::exceed max aggr num [%d]!}\r\n", aggr_num_ctl.uc_aggr_num);
            return ret;
        }
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_AGGR_NUM, sizeof(aggr_num_ctl));

    /* 填写消息体，参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 &aggr_num_ctl, sizeof(aggr_num_ctl)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_ampdu_aggr_num::memcpy fail!}");
        return OAL_FAIL;
    }

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(aggr_num_ctl),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_ampdu_aggr_num::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_intf_det_log(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    wal_ioctl_alg_intfdet_log_param_stru *pst_alg_intfdet_log_param = NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t uc_map_index = 0;
    int32_t l_ret;

    pst_alg_intfdet_log_param = (wal_ioctl_alg_intfdet_log_param_stru *)(st_write_msg.auc_value);

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_intf_det_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (0 == oal_strcmp(st_alg_cfg.pc_name, ac_name)) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_intf_det_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_intfdet_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;
    /* 区分获取特定帧功率和统计日志命令处理:获取功率只需获取帧名字 */
    if (pst_alg_intfdet_log_param->en_alg_cfg == MAC_ALG_CFG_INTF_DET_STAT_LOG_START) {
        /* 获取配置参数名称 */
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_intf_det_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }

        /* 记录参数 */
        pst_alg_intfdet_log_param->us_value = (uint16_t)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, sizeof(wal_ioctl_alg_intfdet_log_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(wal_ioctl_alg_intfdet_log_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_ar_log(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t ret;
    wal_ioctl_alg_ar_log_param_stru *pst_alg_ar_log_param = NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t uc_map_index = 0;
    int32_t l_ret;
    oal_bool_enum_uint8 en_stop_flag = OAL_FALSE;

    pst_alg_ar_log_param = (wal_ioctl_alg_ar_log_param_stru *)(st_write_msg.auc_value);

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_ar_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (oal_strcmp(st_alg_cfg.pc_name, ac_name) == 0) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_ar_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_ar_log_param->auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_log::wal_hipriv_get_mac_addr failed!}\r\n");
        return ret;
    }
    pc_param += off_set;

    while ((*pc_param == ' ') || (*pc_param == '\0')) {
        if (*pc_param == '\0') {
            en_stop_flag = OAL_TRUE;
            break;
        }
        ++pc_param;
    }

    /* 获取业务类型值 */
    if (en_stop_flag != OAL_TRUE) {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_ar_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }

        pst_alg_ar_log_param->uc_ac_no = (uint8_t)oal_atoi(ac_name);
        pc_param = pc_param + off_set;

        en_stop_flag = OAL_FALSE;
        while ((*pc_param == ' ') || (*pc_param == '\0')) {
            if (*pc_param == '\0') {
                en_stop_flag = OAL_TRUE;
                break;
            }
            ++pc_param;
        }

        if (en_stop_flag != OAL_TRUE) {
            /* 获取参数配置值 */
            ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY,
                    "{wal_hipriv_ar_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
                return ret;
            }

            /* 记录参数配置值 */
            pst_alg_ar_log_param->us_value = (uint16_t)oal_atoi(ac_name);
        }
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, sizeof(wal_ioctl_alg_ar_log_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(wal_ioctl_alg_ar_log_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_txbf_log(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    wal_ioctl_alg_txbf_log_param_stru *pst_alg_txbf_log_param = NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t uc_map_index = 0;
    int32_t l_ret;
    oal_bool_enum_uint8 en_stop_flag = OAL_FALSE;
    pst_alg_txbf_log_param = (wal_ioctl_alg_txbf_log_param_stru *)(st_write_msg.auc_value);
    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_txbf_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + off_set;
    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (oal_strcmp(st_alg_cfg.pc_name, ac_name) == 0) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }
    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_log::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }
    /* 记录命令对应的枚举值 */
    pst_alg_txbf_log_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;
    ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_txbf_log_param->auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_txbf_log::wal_hipriv_get_mac_addr failed!}\r\n");
        return ret;
    }
    pc_param += off_set;
    while (*pc_param == ' ') {
        ++pc_param;
        if (*pc_param == '\0') {
            en_stop_flag = OAL_TRUE;
            break;
        }
    }
    /* 获取参数配置值 */
    if (en_stop_flag != OAL_TRUE) {
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY,
                "{wal_hipriv_txbf_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
            return ret;
        }

        pst_alg_txbf_log_param->uc_ac_no = (uint8_t)oal_atoi(ac_name);
        pc_param = pc_param + off_set;

        en_stop_flag = OAL_FALSE;
        while ((*pc_param == ' ') || (*pc_param == '\0')) {
            if (*pc_param == '\0') {
                en_stop_flag = OAL_TRUE;
                break;
            }
            ++pc_param;
        }

        if (en_stop_flag != OAL_TRUE) {
            /* 获取参数配置值 */
            ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
            if (ret != OAL_SUCC) {
                oam_warning_log1(0, OAM_SF_ANY,
                    "{wal_hipriv_txbf_log::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
                return ret;
            }

            /* 记录参数配置值 */
            pst_alg_txbf_log_param->us_value = (uint16_t)oal_atoi(ac_name);
        }
    }
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, sizeof(wal_ioctl_alg_txbf_log_param_stru));
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(wal_ioctl_alg_txbf_log_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_ar_test(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    mac_ioctl_alg_ar_test_param_stru *pst_alg_ar_test_param = NULL;
    wal_ioctl_alg_cfg_stru st_alg_cfg;
    uint8_t uc_map_index = 0;
    int32_t l_ret;

    pst_alg_ar_test_param = (mac_ioctl_alg_ar_test_param_stru *)(st_write_msg.auc_value);

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_ar_test::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param = pc_param + offset;

    /* 寻找匹配的命令 */
    st_alg_cfg = g_ast_alg_cfg_map[0];
    while (st_alg_cfg.pc_name != NULL) {
        if (oal_strcmp(st_alg_cfg.pc_name, ac_name) == 0) {
            break;
        }
        st_alg_cfg = g_ast_alg_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_alg_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_test::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_alg_ar_test_param->en_alg_cfg = g_ast_alg_cfg_map[uc_map_index].en_alg_cfg;

    ret = wal_hipriv_get_mac_addr(pc_param, pst_alg_ar_test_param->auc_mac_addr, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_ar_test::wal_hipriv_get_mac_addr failed!}\r\n");
        return ret;
    }
    pc_param += offset;

    /* 获取参数配置值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0,
            OAM_SF_ANY, "{wal_hipriv_ar_test::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 记录参数配置值 */
    pst_alg_ar_test_param->us_value = (uint16_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ALG_PARAM, sizeof(mac_ioctl_alg_ar_test_param_stru));
    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_ioctl_alg_ar_test_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
OAL_STATIC void wal_hex_key_convert(int8_t *pc_key, uint8_t auc_key[OAL_WPA_KEY_LEN],
    uint32_t off_set, uint32_t len)
{
    uint32_t char_index;
    for (char_index = 0; char_index < off_set; char_index++) {
        if (*pc_key == '-') {
            pc_key++;
            if (char_index != 0) {
                char_index--;
            }

            continue;
        }
        /* 2、16为16进制转换算法相关 */
        auc_key[char_index / 2] = (uint8_t)(auc_key[char_index / 2] * 16 * (char_index % 2) +
                                   oal_strtohex(pc_key));
        pc_key++;
    }
}

OAL_STATIC uint32_t wal_hipriv_test_add_key(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    mac_addkey_param_stru st_payload_params;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_ret;
    int32_t l_sec_ret;
    uint32_t ret;
    uint32_t off_set;
    uint8_t auc_key[OAL_WPA_KEY_LEN] = { 0 };
    int8_t *pc_key = NULL;
    uint16_t us_len;
    wal_msg_stru *pst_rsp_msg = NULL;

    /* 1.1 入参检查 */
    if (oal_any_null_ptr2(pst_net_dev, pc_param)) {
        oam_error_log0(0, OAM_SF_ANY,
            "{wal_hipriv_test_add_key::Param Check ERROR,pst_netdev or pst_params is null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }
    /*
     * xxx(cipher) xx(en_pairwise) xx(key_len) xxx(key_index) xxxx:xx:xx:xx:xx:xx...
     *  (key 小于32字节) xx:xx:xx:xx:xx:xx(目的地址)
     */
    memset_s(&st_payload_params, sizeof(st_payload_params), 0, sizeof(st_payload_params));
    memset_s(&st_payload_params.st_key, sizeof(mac_key_params_stru), 0, sizeof(mac_key_params_stru));
    st_payload_params.st_key.seq_len = 6; /* 6表示seq长度 */
    memset_s(st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN, 0, WLAN_MAC_ADDR_LEN);

    /* 获取cipher */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    st_payload_params.st_key.cipher = (uint32_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /* 获取en_pairwise */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    st_payload_params.en_pairwise = (uint8_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /* 获取key_len */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }

    pc_param = pc_param + off_set;
    st_payload_params.st_key.key_len = (uint8_t)oal_atoi(ac_name);
    if (oal_value_not_in_valid_range(st_payload_params.st_key.key_len, 0, OAL_WPA_KEY_LEN)) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::Param Check ERROR! key_len[%x]  }\r\n",
                       (int32_t)st_payload_params.st_key.key_len);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 获取key_index */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    st_payload_params.uc_key_index = (uint8_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /* 获取key */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    pc_param = pc_param + off_set;
    pc_key = ac_name;
    /* 16进制转换, 最后一个参数是为了codex过 */
    wal_hex_key_convert(pc_key, auc_key, off_set, sizeof(auc_key));

    l_sec_ret = memcpy_s(st_payload_params.st_key.auc_key, sizeof(st_payload_params.st_key.auc_key),
                         auc_key, (uint32_t)st_payload_params.st_key.key_len);

    /* 获取目的地址 */
    memset_s(ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, 0, WAL_HIPRIV_CMD_NAME_MAX_LEN);
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::wal_get_cmd_one_arg fail.err code[%u]}", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), st_payload_params.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    oam_info_log3(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::key_len:%d, seq_len:%d, cipher:0x%08x!}\r\n",
                  st_payload_params.st_key.key_len, st_payload_params.st_key.seq_len, st_payload_params.st_key.cipher);

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    /* 3.2 填写 msg 消息体 */
    us_len = (uint32_t)sizeof(mac_addkey_param_stru);

    l_sec_ret += memcpy_s((int8_t *)st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                          (int8_t *)&st_payload_params, (uint32_t)us_len);
    if (l_sec_ret != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_test_add_key::memcpy fail!}");
        return OAL_FAIL;
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_KEY, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg, OAL_TRUE, &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_enable_pmf::wal_send_cfg_event return err_code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    if (OAL_SUCC != wal_check_and_release_msg_resp(pst_rsp_msg)) {
        oam_warning_log0(0, OAM_SF_ANY, "wal_hipriv_test_add_key::wal_check_and_release_msg_resp fail");
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_str_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t l_ret;
    uint32_t ret;
    wal_ioctl_str_stru st_cmd_table;
    wal_ioctl_tlv_stru *pst_sub_cmd_table = NULL;
    mac_cfg_set_str_stru *pst_set_cmd = NULL;
    wal_msg_write_stru st_write_msg;
    uint8_t uc_len;
    uint8_t uc_cmd_cnt;
    uint16_t us_cfg_id;
    uint8_t uc_map_index = 0;
    uint8_t uc_value_base = 10;
    uint8_t *pc_value = NULL;

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 寻找匹配的命令 */
    st_cmd_table = g_ast_set_str_table[0];
    while (st_cmd_table.pc_name != NULL) {
        if (0 == oal_strcmp(st_cmd_table.pc_name, ac_name)) {
            break;
        }
        st_cmd_table = g_ast_set_str_table[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_cmd_table.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录主命令对应的枚举值 */
    us_cfg_id = g_ast_set_str_table[uc_map_index].en_tlv_cfg_id;
    pst_sub_cmd_table = g_ast_set_str_table[uc_map_index].pst_cfg_table;

    /* 获取设定mib名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 当前命令的参数个数 */
    uc_cmd_cnt = (uint8_t)oal_atoi(ac_name);
    if (uc_cmd_cnt > MAC_STR_CMD_MAX_SIZE) {
        oam_error_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::too many para [%d]!}\r\n", uc_cmd_cnt);
        return OAL_FAIL;
    }

    uc_len = sizeof(mac_cfg_set_str_stru);
    pst_set_cmd = (mac_cfg_set_str_stru *)(st_write_msg.auc_value);
    pst_set_cmd->us_cfg_id = us_cfg_id;
    pst_set_cmd->uc_cmd_cnt = uc_cmd_cnt;
    pst_set_cmd->uc_len = uc_len;
    oam_warning_log3(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd:: cfg id[%d] cfg len[%d] cmd cnt[%d]!}",
                     us_cfg_id, uc_len, uc_cmd_cnt);

    /* 遍历获取每个字符参数 */
    for (uc_map_index = 0; uc_map_index < uc_cmd_cnt; uc_map_index++) {
        /* 获取子命令 */
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}", ret);
            return ret;
        }
        pc_param += off_set;

        /* 子命令表查找对应的子命令 */
        while (pst_sub_cmd_table->pc_name != NULL) {
            if (oal_strcmp(pst_sub_cmd_table->pc_name, ac_name) == 0) {
                break;
            }
            pst_sub_cmd_table++;
        }

        /* 没有找到对应的命令，则报错 */
        if (pst_sub_cmd_table->pc_name == NULL) {
            oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::invalid alg_cfg command!}\r\n");
            return OAL_FAIL;
        }

        /* 获取设定置 */
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::wal_get_cmd_one_arg return err_code [%d]!}", ret);
            return ret;
        }
        pc_value = ac_name;
        if (basic_value_is_hex(ac_name)) {
            uc_value_base = 16; // 16代表传进制参数为16
            pc_value = ac_name + BYTE_OFFSET_2;
        }
        pc_param += off_set;

        /* 记录每个子命令的id号和具体的命令值 */
        pst_set_cmd->us_set_id[uc_map_index] = pst_sub_cmd_table->en_tlv_cfg_id;
        pst_set_cmd->value[uc_map_index] = (uint32_t)oal_strtol(pc_value, NULL, uc_value_base);
        oam_warning_log4(0, OAM_SF_ANY, "{wal_hipriv_set_str_cmd::index[%d] sub cfg id[%d], value_base[%s], value[%d]}",
            uc_map_index, pst_set_cmd->us_set_id[uc_map_index], uc_value_base, pst_set_cmd->value[uc_map_index]);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, us_cfg_id, uc_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + uc_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    /*lint -e571*/
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_tlv_cmd::wal_send_cfg_event return err_code [%d]!}", l_ret);
        return (uint32_t)l_ret;
    }
    /*lint +e571*/
    return OAL_SUCC;
}


uint32_t wal_hipriv_set_tlv_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    wal_ioctl_tlv_stru st_tlv_cfg;
    mac_cfg_set_tlv_stru *pst_set_tlv = NULL;
    wal_msg_write_stru st_write_msg;
    uint8_t uc_len;
    uint8_t uc_cmd_type;
    uint16_t us_cfg_id;
    uint32_t value;
    uint8_t uc_map_index = 0;

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_tlv_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 寻找匹配的命令 */
    st_tlv_cfg = g_ast_set_tlv_table[0];
    while (st_tlv_cfg.pc_name != NULL) {
        if (0 == oal_strcmp(st_tlv_cfg.pc_name, ac_name)) {
            break;
        }
        st_tlv_cfg = g_ast_set_tlv_table[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_tlv_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_tlv_cmd::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    us_cfg_id = g_ast_set_tlv_table[uc_map_index].en_tlv_cfg_id;

    /* 获取设定mib名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_tlv_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pc_param += off_set;
    uc_cmd_type = (uint8_t)oal_atoi(ac_name);

    /* 获取设定置 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_tlv_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;
    value = (uint32_t)oal_atoi(ac_name);

    uc_len = sizeof(mac_cfg_set_tlv_stru);

    pst_set_tlv = (mac_cfg_set_tlv_stru *)(st_write_msg.auc_value);
    pst_set_tlv->uc_cmd_type = uc_cmd_type;
    pst_set_tlv->us_cfg_id = us_cfg_id;
    pst_set_tlv->uc_len = uc_len;
    pst_set_tlv->value = value;

    oam_warning_log4(0, OAM_SF_ANY, "{wal_hipriv_set_tlv_cmd:: cfg id[%d] cfg len[%d] cmd type[%d], set val[%d]!}",
                     us_cfg_id, uc_len, uc_cmd_type, value);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, us_cfg_id, uc_len);

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + uc_len,
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_tlv_cmd::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_set_val_cmd(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    wal_ioctl_tlv_stru st_tlv_cfg;
    mac_cfg_set_tlv_stru *pst_set_tlv = NULL;
    wal_msg_write_stru st_write_msg;
    uint8_t uc_len;
    uint16_t us_cfg_id;
    uint32_t value;
    uint8_t uc_map_index = 0;

    /* 获取配置参数名称 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_val_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    pc_param += off_set;

    /* 寻找匹配的命令 */
    st_tlv_cfg = g_ast_set_tlv_table[0];
    while (st_tlv_cfg.pc_name != NULL) {
        if (oal_strcmp(st_tlv_cfg.pc_name, ac_name) == 0) {
            break;
        }
        st_tlv_cfg = g_ast_set_tlv_table[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_tlv_cfg.pc_name == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_set_val_cmd::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    us_cfg_id = g_ast_set_tlv_table[uc_map_index].en_tlv_cfg_id;

    /* 获取设定值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_val_cmd::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pc_param += off_set;
    value = (uint32_t)oal_atoi(ac_name);

    uc_len = sizeof(mac_cfg_set_tlv_stru);

    pst_set_tlv = (mac_cfg_set_tlv_stru *)(st_write_msg.auc_value);
    pst_set_tlv->uc_cmd_type = 0xFF;
    pst_set_tlv->us_cfg_id = us_cfg_id;
    pst_set_tlv->uc_len = uc_len;
    pst_set_tlv->value = value;

    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_hipriv_set_val_cmd:: cfg id[%d], cfg len[%d],set_val[%d]!}", us_cfg_id, uc_len, value);
    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, us_cfg_id, uc_len);

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + uc_len,
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_val_cmd::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_WAPI

#ifdef _PRE_WAPI_DEBUG
OAL_STATIC uint32_t wal_hipriv_show_wapi_info(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    mac_vap_stru *pst_mac_vap;
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint32_t ret;
    mac_cfg_user_info_param_stru *pst_user_info_param;
    uint8_t auc_mac_addr[6] = { 0 }; /* 临时保存获取的use的mac地址信息,6字节 */
    uint8_t uc_char_index;
    uint16_t us_user_idx;

    /* 去除字符串的空格 */
    pc_param++;
    /* 获取mac地址,16进制转换 */
    for (uc_char_index = 0; uc_char_index < 12; uc_char_index++) { /* 12 = 6byte * 2(高4位+低四位) */
        if (*pc_param == ':') {
            pc_param++;
            if (uc_char_index != 0) {
                uc_char_index--;
            }
            continue;
        }
        /* 2、16为16进制转换算法相关 */
        auc_mac_addr[uc_char_index / 2] = (uint8_t)(auc_mac_addr[uc_char_index / 2] * 16 * (uc_char_index % 2) +
                                          oal_strtohex(pc_param));
        pc_param++;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_WAPI_INFO, sizeof(mac_cfg_user_info_param_stru));

    /* 根据mac地址找用户 */
    pst_mac_vap = oal_net_dev_priv(pst_net_dev);

    l_ret = (int32_t)mac_vap_find_user_by_macaddr(pst_mac_vap, auc_mac_addr, &us_user_idx);
    if (l_ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_user_info::no such user!}\r\n");
        return OAL_FAIL;
    }

    /* 设置配置命令参数 */
    pst_user_info_param = (mac_cfg_user_info_param_stru *)(st_write_msg.auc_value);
    pst_user_info_param->us_user_idx = us_user_idx;

    oam_warning_log1(0, OAM_SF_ANY, "wal_hipriv_show_wapi_info::us_user_idx %u", us_user_idx);
    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_user_info_param_stru),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(pst_mac_vap->uc_vap_id, OAM_SF_ANY,
            "{wal_hipriv_user_info::return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif /* #ifdef WAPI_DEBUG_MODE */
#endif /* #ifdef _PRE_WLAN_FEATURE_WAPI */

#ifdef _PRE_WLAN_FEATURE_FTM
OAL_STATIC wal_ftm_cmd_entry_stru g_ast_ftm_common_cmd[] = {
    { "enable_ftm_initiator", OAL_TRUE, BIT0 },
    { "enable",               OAL_TRUE, BIT2 },
    { "cali",                 OAL_TRUE, BIT3 },
    { "enable_ftm_responder", OAL_TRUE, BIT5 },
    { "enable_ftm_range",     OAL_TRUE, BIT8 },
    { "en_multipath",         OAL_TRUE, BIT12 },
};

OAL_STATIC OAL_INLINE void ftm_debug_cmd_format_info(void)
{
    oam_warning_log0(0, OAM_SF_FTM,
        "{CMD format::hipriv.sh wlan0 ftm_debug \
        enable_ftm_initiator[0|1] \
        enable[0|1] \
        cali[0|1] \
        enable_ftm_responder[0|1] \
        set_ftm_time t1[] t2[] t3[] t4[] \
        set_ftm_timeout bursts_timeout ftms_timeout \
        enable_ftm_responder[0|1] \
        enable_ftm_range[0|1] \
        get_cali \
        !!}\r\n");
    oam_warning_log0(0, OAM_SF_FTM,
        "{CMD format::hipriv.sh wlan0 ftm_debug \
        send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n] en_lci_civic_request[0|1] asap[0|1] \
        bssid[xx:xx:xx:xx:xx:xx] format_bw[9~13] send_ftm bssid[xx:xx:xx:xx:xx:xx] \
        send_range_req mac[] num_rpt[0-65535] delay[] ap_cnt[] bssid[] channel[] bssid[] channel[] ...\
        set_location type[] mac[] mac[] mac[] \
        white_list index mac[xx:xx:xx:xx:xx:xx] \
        !!}\r\n");
}


OAL_STATIC uint32_t ftm_debug_parase_iftmr_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };
    uint32_t off_set = 0;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    *pul_offset = 0;
    /* 命令格式:send_initial_ftm_request channel[] ftms_per_burst[n] burst_num[n]---代表(2^n)个burst
                                    en_lci_civic_request[0|1] asap[0|1] bssid[xx:xx:xx:xx:xx:xx]
                                    format_bw[9~13] */
    /* 解析channel */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get iftmr mode error, return.}");
        return ret;
    }

    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.channel_num = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析ftms_per_burst */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get uc_ftms_per_burst error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.ftms_per_burst = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析burst */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get burst error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.burst_num = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析LCI and Location Civic Measurement request使能标志 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get en_lci_civic_request error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.lci_request = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析asap,as soon as possible使能标志 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get asap error, return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.is_asap_on = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 获取BSSID */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::No bssid, set the associated bssid.}");
        *pul_offset += off_set;
        memset_s(pst_debug_info->st_send_iftmr_bit1.mac_addr, sizeof(pst_debug_info->st_send_iftmr_bit1.mac_addr),
                 0, sizeof(pst_debug_info->st_send_iftmr_bit1.mac_addr));
        pc_param += off_set;
        off_set = 0;

        return OAL_SUCC;
    }
    *pul_offset += off_set;
    oal_set_mac_addr(pst_debug_info->st_send_iftmr_bit1.mac_addr, auc_mac_addr);
    pc_param += off_set;
    off_set = 0;

    oam_warning_log3(0, OAM_SF_ANY, "{ftm_debug_parase_iftmr_cmd::bssid=*:*:*:%x:%x:%x}",
                     auc_mac_addr[MAC_ADDR_3], auc_mac_addr[MAC_ADDR_4], auc_mac_addr[MAC_ADDR_5]);

    /* 解析format bw */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_iftmr_cmd::get format bw error, return.}");
        return OAL_SUCC;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_iftmr_bit1.format_bw = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_parase_ftm_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析mac */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_cmd::ger mac error.}");
        return OAL_FAIL;
    }
    *pul_offset += off_set;
    oal_strtoaddr(ac_value, sizeof(ac_value), pst_debug_info->st_send_ftm_bit4.auc_mac, WLAN_MAC_ADDR_LEN);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_parase_ftm_timeout_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析ftm bursts timeout */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time1 error,return.}");
        return ret;
    }

    *pul_offset += off_set;
    pst_debug_info->st_ftm_timeout_bit14.initiator_tsf_fix_offset = oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析ftms timeout */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time2 error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_ftm_timeout_bit14.responder_ptsf_offset = oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_set_cali(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    *pul_offset = 0;

    /* 解析t1 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_ftm_time_cmd::get correct time1 error, return.}");
        return ret;
    }

    *pul_offset += off_set;
    pst_debug_info->ftm_cali = (uint32_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_parase_range_req_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t uc_ap_cnt;

    *pul_offset = 0;
    /* 解析mac */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    *pul_offset += off_set;
    oal_strtoaddr(ac_value, sizeof(ac_value), pst_debug_info->st_send_range_req_bit7.auc_mac, WLAN_MAC_ADDR_LEN);
    pc_param += off_set;
    off_set = 0;

    /* 解析num_rpt */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get num_rpt error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_range_req_bit7.us_num_rpt = (uint16_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析ap_cnt */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get ap_cnt error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_send_range_req_bit7.uc_minimum_ap_count = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    for (uc_ap_cnt = 0; uc_ap_cnt < pst_debug_info->st_send_range_req_bit7.uc_minimum_ap_count; uc_ap_cnt++) {
        /* 解析bssid */
        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::bssid error!.}");
            return ret;
        }
        *pul_offset += off_set;
        oal_strtoaddr(ac_value, sizeof(ac_value),
            pst_debug_info->st_send_range_req_bit7.aauc_bssid[uc_ap_cnt], WLAN_MAC_ADDR_LEN);
        pc_param += off_set;
        off_set = 0;

        /* 解析channel */
        ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_range_req_cmd::get channel error,return.}");
            return ret;
        }
        *pul_offset += off_set;
        pst_debug_info->st_send_range_req_bit7.auc_channel[uc_ap_cnt] = (uint8_t)oal_atoi(ac_value);
        pc_param += off_set;
        off_set = 0;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_parase_m2s_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;

    *pul_offset = 0;
    /* 解析ftm定位通路:0或1 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_m2s_cmd::get tx_chain error,return.}");
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->ftm_chain_selection = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_parase_neighbor_report_req_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    uint32_t off_set = 0;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    *pul_offset = 0;

    /* 解析bssid */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM,
            "{ftm_debug_parase_neighbor_report_req_cmd::No bssid,set the associated bssid.}");
        *pul_offset += off_set;
        memset_s(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid,
                 sizeof(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid),
                 0, sizeof(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid));
        pc_param += off_set;
        off_set = 0;
        return ret;
    }
    *pul_offset += off_set;
    oal_set_mac_addr(pst_debug_info->st_neighbor_report_req_bit15.auc_bssid, auc_mac_addr);
    pc_param += off_set;
    off_set = 0;

    oam_warning_log3(0, OAM_SF_ANY,
        "{ftm_debug_parase_neighbor_report_req_cmd::send neighbor request frame to ap[*:*:*:%x:%x:%x]}",
        auc_mac_addr[MAC_ADDR_3], auc_mac_addr[MAC_ADDR_4], auc_mac_addr[MAC_ADDR_5]);

    return OAL_SUCC;
}


OAL_STATIC uint32_t ftm_debug_parase_gas_init_req_cmd(int8_t *pc_param,
    mac_ftm_debug_switch_stru *pst_debug_info, uint32_t *pul_offset)
{
    uint32_t ret;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    uint32_t off_set = 0;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0, 0, 0, 0, 0, 0 };

    /* 命令格式:send_gas_init_req lci_enable[0|1] bssid[xx:xx:xx:xx:xx:xx] */
    *pul_offset = 0;

    /* 解析lci_enable */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM,
            "{ftm_debug_parase_gas_init_req_cmd::get lci_enable error, use default value [true].}");
        ac_value[0] = OAL_TRUE;
        return ret;
    }
    *pul_offset += off_set;
    pst_debug_info->st_gas_init_req_bit16.en_lci_enable = (uint8_t)oal_atoi(ac_value);
    pc_param += off_set;
    off_set = 0;

    /* 解析bssid */
    ret = wal_hipriv_get_mac_addr(pc_param, auc_mac_addr, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_FTM, "{ftm_debug_parase_gas_init_req_cmd::No bssid, set the associated bssid.}");
        *pul_offset += off_set;
        memset_s(pst_debug_info->st_gas_init_req_bit16.auc_bssid,
                 sizeof(pst_debug_info->st_gas_init_req_bit16.auc_bssid),
                 0, sizeof(pst_debug_info->st_gas_init_req_bit16.auc_bssid));
        pc_param += off_set;
        off_set = 0;
        return ret;
    }
    *pul_offset += off_set;
    oal_set_mac_addr(pst_debug_info->st_gas_init_req_bit16.auc_bssid, auc_mac_addr);
    pc_param += off_set;
    off_set = 0;

    oam_warning_log3(0, OAM_SF_ANY,
        "{ftm_debug_parase_gas_init_req_cmd::send gas initial request frame to ap[*:*:*:%x:%x:%x]}",
        auc_mac_addr[MAC_ADDR_3], auc_mac_addr[MAC_ADDR_4], auc_mac_addr[MAC_ADDR_5]);
    return OAL_SUCC;
}


OAL_STATIC uint8_t wal_hipriv_ftm_is_common_cmd(int8_t ac_name[], uint8_t *puc_cmd_index)
{
    uint8_t uc_cmd_index;
    uint32_t ratio = sizeof(g_ast_ftm_common_cmd) / sizeof(wal_ftm_cmd_entry_stru);

    for (uc_cmd_index = 0; uc_cmd_index < ratio; uc_cmd_index++) {
        if (0 == oal_strcmp(g_ast_ftm_common_cmd[uc_cmd_index].pc_cmd_name, ac_name)) {
            break;
        }
    }

    *puc_cmd_index = uc_cmd_index;
    if (uc_cmd_index == ratio) {
        return OAL_FALSE;
    } else {
        return OAL_TRUE;
    }
}


OAL_STATIC uint32_t wal_hipriv_ftm_set_common_cmd(int8_t *pc_param,
    uint8_t uc_cmd_index, mac_ftm_debug_switch_stru *pst_ftm_debug, uint32_t *pul_offset)
{
    uint32_t ret;
    uint32_t off_set = 0;
    int8_t ac_value[WAL_HIPRIV_CMD_VALUE_MAX_LEN] = {0};
    *pul_offset = 0;

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ftm_set_common_cmd:: ftm common cmd [%d].}", uc_cmd_index);

    /* 取命令配置值 */
    ret = wal_get_cmd_one_arg(pc_param, ac_value, WAL_HIPRIV_CMD_VALUE_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        ftm_debug_cmd_format_info();
        return ret;
    }

    /* 检查命令的配置值 */
    if ((g_ast_ftm_common_cmd[uc_cmd_index].uc_is_check_para) &&
        oal_value_not_in_valid_range(ac_value[0], '0', '9')) {
        ftm_debug_cmd_format_info();
        return OAL_FAIL;
    }

    *pul_offset += off_set;
    off_set = 0;

    /* 填写命令结构体 */
    switch (g_ast_ftm_common_cmd[uc_cmd_index].bit) {
        case BIT2:
            pst_ftm_debug->en_enable_bit2 = ((uint8_t)oal_atoi(ac_value));
            pst_ftm_debug->cmd_bit_map |= BIT2;
            break;
        case BIT3:
            pst_ftm_debug->en_cali_bit3 = ((uint8_t)oal_atoi(ac_value));
            pst_ftm_debug->cmd_bit_map |= BIT3;
            break;
        case BIT12:
            pst_ftm_debug->en_multipath_bit12 = ((uint8_t)oal_atoi(ac_value));
            pst_ftm_debug->cmd_bit_map |= BIT12;
            break;
        default:
            oam_warning_log1(0, OAM_SF_FTM,
                "{wal_hipriv_ftm_set_common_cmd::invalid cmd bit[0x%x]!}", g_ast_ftm_common_cmd[uc_cmd_index].bit);
            break;
    }

    return OAL_SUCC;
}

static uint32_t wal_get_ftm_debug_send_iftmr_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_iftmr_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    ftm_debug->cmd_bit_map |= BIT1;
    return ret;
}

static uint32_t wal_get_ftm_debug_ftm_addr_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_ftm_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT4;
    return ret;
}

static uint32_t wal_get_ftm_debug_timeout_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_ftm_timeout_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT14;
    return ret;
}

static uint32_t wal_get_ftm_debug_set_cali_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_set_cali(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT13;
    return ret;
}

static uint32_t wal_get_ftm_debug_range_req_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_range_req_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT7;
    return ret;
}

static uint32_t wal_get_ftm_debug_get_cali_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    ftm_debug->cmd_bit_map |= BIT9;
    return OAL_SUCC;
}

static uint32_t wal_get_ftm_debug_m2s_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_m2s_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT11;
    return ret;
}

static uint32_t wal_get_ftm_debug_neighbor_report_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_neighbor_report_req_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT15;
    return ret;
}

static uint32_t wal_get_ftm_debug_gas_init_info(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param)
{
    uint32_t off_set = 0;
    uint32_t ret;
    ret = ftm_debug_parase_gas_init_req_cmd(*param, ftm_debug, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    (*param) += off_set;
    off_set = 0;
    ftm_debug->cmd_bit_map |= BIT16;
    return ret;
}

typedef struct {
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t (*wal_get_ftm_debug_case_info)(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param);
} wal_get_ftm_debug_attr_info_ops;

static uint32_t wal_get_ftm_debug_attr_info_byname(mac_ftm_debug_switch_stru *ftm_debug, int8_t **param,
                                                   int8_t *name)
{
    uint8_t idx;
    uint32_t ret = 0;
    const wal_get_ftm_debug_attr_info_ops wal_get_ftm_debug_attr_table[] = {
        { "send_initial_ftm_request", wal_get_ftm_debug_send_iftmr_info },
        { "send_ftm",                 wal_get_ftm_debug_ftm_addr_info },
        { "set_ftm_timeout",          wal_get_ftm_debug_timeout_info },
        { "set_cali",                 wal_get_ftm_debug_set_cali_info },
        { "send_range_req",           wal_get_ftm_debug_range_req_info },
        { "get_cali",                 wal_get_ftm_debug_get_cali_info },
        { "set_ftm_m2s",              wal_get_ftm_debug_m2s_info },
        { "send_nbr_rpt_req",         wal_get_ftm_debug_neighbor_report_info },
        { "send_gas_init_req",        wal_get_ftm_debug_gas_init_info },
    };

    for (idx = 0; idx < (sizeof(wal_get_ftm_debug_attr_table) / sizeof(wal_get_ftm_debug_attr_info_ops)); ++idx) {
        if (oal_strcmp(wal_get_ftm_debug_attr_table[idx].name, name) == 0) {
            ret = wal_get_ftm_debug_attr_table[idx].wal_get_ftm_debug_case_info(ftm_debug, param);
            break;
        }
    }
    if (idx == sizeof(wal_get_ftm_debug_attr_table) / sizeof(wal_get_ftm_debug_attr_info_ops)) {
        return OAL_FAIL;
    } else {
        return ret;
    }
    return ret;
}

OAL_STATIC uint32_t wal_get_ftm_param(mac_ftm_debug_switch_stru *ftm_debug, int8_t *param)
{
    uint32_t off_set = 0;
    int8_t name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    uint32_t ret = 0;
    uint8_t cmd_index = 0;

    do {
        /* 获取命令关键字 */
        ret = wal_get_cmd_one_arg(param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if ((ret != OAL_SUCC) && (off_set != 0)) {
            return ret;
        }
        param += off_set;

        if (off_set == 0) {
            break;
        }

        /* 命令分类 */
        if (wal_hipriv_ftm_is_common_cmd(name, &cmd_index)) {
            ret = wal_hipriv_ftm_set_common_cmd(param, cmd_index, ftm_debug, &off_set);
            param += off_set;
            off_set = 0;
            if (ret != OAL_SUCC) {
                return ret;
            }
        } else {
            if (wal_get_ftm_debug_attr_info_byname(ftm_debug, &param, name) != OAL_SUCC) {
                return OAL_FAIL;
            }
        }
    } while (*param != '\0');

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_ftm(oal_net_device_stru *net_dev, int8_t *param)
{
    wal_msg_write_stru write_msg;
    mac_ftm_debug_switch_stru ftm_debug;
    int32_t l_ret;
    uint32_t ret;

    memset_s(&ftm_debug, sizeof(ftm_debug), 0, sizeof(ftm_debug));

    ret = wal_get_ftm_param(&ftm_debug, param);
    if (ret != OAL_SUCC) {
        ftm_debug_cmd_format_info();
        return ret;
    }

    oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_ftm::cmd_bit_map: 0x%08x.}", ftm_debug.cmd_bit_map);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_FTM_DBG, sizeof(ftm_debug));

    /* 设置配置命令参数 */
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
                 (const void *)&ftm_debug, sizeof(ftm_debug)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_ftm::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(ftm_debug),
                               (uint8_t *)&write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_FTM, "{wal_hipriv_ftm::return err code[%d]!}", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
OAL_STATIC uint32_t _wal_hipriv_ftm(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_ftm_is_open) {
        return wal_hipriv_ftm(pst_net_dev, pc_param);
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC uint32_t wal_hipriv_get_version(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    uint16_t us_len;

    /***************************************************************************
                              抛事件到wal层处理
    ***************************************************************************/
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value),
                 pc_param, OAL_STRLEN(pc_param)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_get_version::memcpy fail!}");
        return OAL_FAIL;
    }

    st_write_msg.auc_value[OAL_STRLEN(pc_param)] = '\0';

    us_len = (uint16_t)(OAL_STRLEN(pc_param) + 1);

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_VERSION, us_len);

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_get_version::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY

OAL_STATIC uint32_t wal_hipriv_get_user_nssbw(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_add_user_param_stru *pst_add_user_param;
    mac_cfg_add_user_param_stru st_add_user_param; /* 临时保存获取的use的信息 */
    uint32_t get_addr_idx;

    /* 获取用户带宽和空间流信息: hipriv "vap0 add_user xx xx xx xx xx xx(mac地址)" */
    memset_s((void *)&st_add_user_param, sizeof(mac_cfg_add_user_param_stru),
             0, sizeof(mac_cfg_add_user_param_stru));
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_add_user::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), st_add_user_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    /* 设置配置命令参数 */
    pst_add_user_param = (mac_cfg_add_user_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_add_user_param->auc_mac_addr[get_addr_idx] = st_add_user_param.auc_mac_addr[get_addr_idx];
    }

    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_GET_USER_RSSBW, sizeof(mac_cfg_add_user_param_stru));

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_add_user_param_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_add_user::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_M2S


uint32_t wal_hipriv_mss_param_handle(mac_m2s_mgr_stru *pst_m2s_mgr, int8_t *pc_param)
{
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_mss_param_handle::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.mss_mode.en_mss_on = (uint8_t)oal_atoi(ac_name);
    if (pst_m2s_mgr->pri_data.mss_mode.en_mss_on >= 2) { /* en_mss_on 为bool型，值为0 or 1，不能大于等于2 */
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_mss_param_handle::pst_m2s_mgr->pri_data.mss_mode.en_mss_on error [%d] (0 or 1)!}\r\n",
            pst_m2s_mgr->pri_data.mss_mode.en_mss_on);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_M2S_MODEM

uint32_t wal_hipriv_modem_param_handle(mac_m2s_mgr_stru *pst_m2s_mgr, int8_t *pc_param)
{
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    if (g_wlan_spec_cfg->feature_m2s_modem_is_open) {
        return OAL_FAIL;
    }
    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_mss_param_handle::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.modem_mode.en_m2s_state = (uint8_t)oal_atoi(ac_name);

    return OAL_SUCC;
}
#endif


uint32_t wal_hipriv_rssi_param_handle(mac_m2s_mgr_stru *pst_m2s_mgr, int8_t *pc_param)
{
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /* 1.获取第二个参数 */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_set_m2s_switch::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.rssi_mode.uc_opt = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_set_m2s_switch::wal_get_cmd_one_arg2 return err_code [%d]!}\r\n", ret);
        return ret;
    }

    pst_m2s_mgr->pri_data.rssi_mode.c_value = (int8_t)oal_atoi(ac_name);

    return OAL_SUCC;
}

uint32_t wal_hipriv_btcoex_param_handle(mac_m2s_mgr_stru *m2s_mgr, int8_t *param)
{
    uint32_t ret;
    int8_t   name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t cmd_off_set = 0;

    /* 1.获取第二参数: 期望切换到的状态 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_btcoex_param_handle::get arg2 return err_code [%d]!}", ret);
        return ret;
    }
    m2s_mgr->pri_data.btcoex_mode.chain_apply = (uint8_t)oal_atoi(name);

    /* 偏移，取下一个参数 */
    param = param + cmd_off_set;
    /* 2.获取第三参数: 优先级 */
    ret = wal_get_cmd_one_arg((int8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_btcoex_param_handle::get arg3 return err_code [%d]!}", ret);
        return ret;
    }
    m2s_mgr->pri_data.btcoex_mode.ant_occu_prio = (uint8_t)oal_atoi(name);

    return OAL_SUCC;
}


uint8_t wal_hipriv_m2s_switch_get_mcm_chain(uint16_t param)
{
    uint8_t chain_index;
    uint8_t mcm_chain = 0;

    /* 切换命令第二个参数含义为使用字符串表示的二进制如C2SISO为0100 */
    for (chain_index = 0; chain_index < MCM_CHAIN_NUM_MAX; chain_index++) {
        if ((param % MCM_CONFIG_CHAIN_STEP) != 0) {
            mcm_chain |= ((uint8_t)(1 << chain_index));
        }
        param /= MCM_CONFIG_CHAIN_STEP;
    }
    return mcm_chain;
}


uint32_t wal_hipriv_m2s_switch_param_handle(mac_m2s_mgr_stru *m2s_mgr, uint8_t *param,
    uint32_t *cmd_off_set, uint8_t *name)
{
    uint32_t ret;

    ret = wal_get_cmd_one_arg((uint8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_m2s_switch_param_handle::wal_get_cmd_one_arg2 \
            return err_code [%d]!}\r\n", ret);
        return ret;
    }
    if (get_hi110x_subchip_type() == BOARD_VERSION_HI1106) {
        m2s_mgr->pri_data.test_mode.uc_m2s_state = wal_hipriv_m2s_switch_get_mcm_chain((uint16_t)oal_atoi(name));
    } else {
        m2s_mgr->pri_data.test_mode.uc_m2s_state = (uint8_t)oal_atoi(name);
    }
    /* 偏移，取下一个参数 */
    param = param + (*cmd_off_set);

    /* 2.获取第三个参数:主路还是辅路 0为主路 1为辅路 暂时不使用 空缺TBD */
    ret = wal_get_cmd_one_arg((uint8_t *)param, name, WAL_HIPRIV_CMD_NAME_MAX_LEN, cmd_off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_m2s_switch_param_handle::wal_get_cmd_one_arg3 \
            return err_code [%d]!}\r\n", ret);
        return ret;
    }

    m2s_mgr->pri_data.test_mode.uc_master_id = (uint8_t)oal_atoi(name);
    if (m2s_mgr->pri_data.test_mode.uc_master_id >= HAL_DEVICE_ID_BUTT) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_m2s_switch_param_handle:: \
            pst_m2s_mgr->pri_data.test_mode.uc_master_id error [%d] 0/1!}\r\n",
            m2s_mgr->pri_data.test_mode.uc_master_id);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /* true: 硬切换测试模式采用默认软切换配置 */
    /* false: 业务切换，软切换测试模式采用默认软切换配置 */
    m2s_mgr->pri_data.test_mode.en_m2s_type = (m2s_mgr->en_cfg_m2s_mode == MAC_M2S_MODE_HW_TEST) ?
        WLAN_M2S_TYPE_HW : WLAN_M2S_TYPE_SW;

    /* 标识业务类型 */
    m2s_mgr->pri_data.test_mode.uc_trigger_mode =
        (m2s_mgr->en_cfg_m2s_mode == MAC_M2S_MODE_DELAY_SWITCH) ?
        WLAN_M2S_TRIGGER_MODE_COMMAND : WLAN_M2S_TRIGGER_MODE_TEST;
    return OAL_SUCC;
}



uint32_t wal_hipriv_set_m2s_switch(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    mac_m2s_mgr_stru *pst_m2s_mgr;
    uint32_t off_set = 0;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* st_write_msg作清零操作 */
    memset_s(&st_write_msg, sizeof(wal_msg_write_stru),
        0, sizeof(wal_msg_write_stru));

    /* 设置配置命令参数 */
    pst_m2s_mgr = (mac_m2s_mgr_stru *)st_write_msg.auc_value;

    /* 1.获取第一个参数: mode */
    ret = wal_get_cmd_one_arg((int8_t *)pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_set_m2s_switch::wal_get_cmd_one_arg1 return err_code [%d]!}", ret);
        return ret;
    }

    pst_m2s_mgr->en_cfg_m2s_mode = (mac_m2s_mode_enum_uint8)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    switch (pst_m2s_mgr->en_cfg_m2s_mode) {
        /* mimo-siso切换参数查询 */
        case MAC_M2S_MODE_QUERY:
            /* 抛事件dmac打印全局管理参数即可 */
            ret = OAL_SUCC;
            break;

        case MAC_M2S_MODE_MSS:
            ret = wal_hipriv_mss_param_handle(pst_m2s_mgr, pc_param);
            break;

        case MAC_M2S_MODE_DELAY_SWITCH:
        case MAC_M2S_MODE_SW_TEST:
        case MAC_M2S_MODE_HW_TEST:
            ret = wal_hipriv_m2s_switch_param_handle(pst_m2s_mgr, pc_param, &off_set, ac_name);
            break;
        case MAC_M2S_MODE_RSSI:
            ret = wal_hipriv_rssi_param_handle(pst_m2s_mgr, pc_param);
            break;
#ifdef _PRE_WLAN_FEATURE_M2S_MODEM
        case MAC_M2S_MODE_MODEM:
            ret = wal_hipriv_modem_param_handle(pst_m2s_mgr, pc_param);
            break;
#endif
        case MAC_M2S_MODE_BTCOEX:
            ret = wal_hipriv_btcoex_param_handle(pst_m2s_mgr, pc_param);
            break;
        default:
            oam_warning_log1(0, OAM_SF_M2S, "{wal_hipriv_set_m2s_switch: en_cfg_m2s_mode[%d] error!}",
                pst_m2s_mgr->en_cfg_m2s_mode);
            return OAL_FAIL;
    }

    if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_M2S_SWITCH, sizeof(mac_m2s_mgr_stru));

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_m2s_mgr_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_M2S,
            "{wal_hipriv_set_m2s_stitch::wal_hipriv_reset_device return err code = [%d].}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}
#endif

OAL_STATIC uint32_t wal_hipriv_blacklist_add(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;
    mac_blacklist_stru *pst_blklst = NULL;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_blacklist_add:wal_get_cmd_one_arg fail!}\r\n");
        return ret;
    }
    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    pst_blklst = (mac_blacklist_stru *)(st_write_msg.auc_value);
    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_name, sizeof(ac_name), pst_blklst->auc_mac_addr, WLAN_MAC_ADDR_LEN);

    us_len = sizeof(mac_blacklist_stru);
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_ADD_BLACK_LIST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_blacklist_add:wal_send_cfg_event return[%d].}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_blacklist_del(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint16_t us_len;
    uint32_t ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };
    uint32_t off_set = 0;

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_blacklist_add:wal_get_cmd_one_arg fail!}\r\n");
        return ret;
    }
    memset_s((uint8_t *)&st_write_msg, sizeof(st_write_msg), 0, sizeof(st_write_msg));
    /* 将字符 ac_name 转换成数组 mac_add[6] */
    oal_strtoaddr(ac_name, sizeof(ac_name), st_write_msg.auc_value, WLAN_MAC_ADDR_LEN);

    us_len = OAL_MAC_ADDR_LEN; /* sizeof(uint8_t); */
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DEL_BLACK_LIST, us_len);

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + us_len,
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_blacklist_add:wal_send_cfg_event return[%d].}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_set_blacklist_mode(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    return wal_hipriv_send_cfg_uint32_data(pst_net_dev, pc_param, WLAN_CFGID_BLACKLIST_MODE);
}


OAL_STATIC uint32_t wal_hipriv_vap_classify_en(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    uint32_t val = 0xff;
    wal_msg_write_stru st_write_msg;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
        val = (uint32_t)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VAP_CLASSIFY_EN, sizeof(uint32_t));

    /* 设置配置命令参数 */
    *((uint32_t *)(st_write_msg.auc_value)) = val;

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_packet_xmit::wal_send_cfg_event fail.return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_vap_classify_tid(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    uint32_t val = 0xff;
    wal_msg_write_stru st_write_msg;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
        val = (uint32_t)oal_atoi(ac_name);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_VAP_CLASSIFY_TID, sizeof(uint32_t));

    /* 设置配置命令参数 */
    *((uint32_t *)(st_write_msg.auc_value)) = val;

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_packet_xmit::wal_send_cfg_event fail.return err code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

uint32_t wal_hipriv_sta_pm_on(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    uint8_t uc_sta_pm_open;
    mac_cfg_ps_open_stru *pst_sta_pm_open = NULL;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_sta_pm_open::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    uc_sta_pm_open = (uint8_t)oal_atoi(ac_name);
    pc_param = pc_param + off_set;

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_STA_PM_ON, sizeof(mac_cfg_ps_open_stru));

    /* 设置配置命令参数 */
    pst_sta_pm_open = (mac_cfg_ps_open_stru *)(st_write_msg.auc_value);
    /* MAC_STA_PM_SWITCH_ON / MAC_STA_PM_SWITCH_OFF */
    pst_sta_pm_open->uc_pm_enable = uc_sta_pm_open;
    pst_sta_pm_open->uc_pm_ctrl_type = MAC_STA_PM_CTRL_TYPE_CMD;

    l_ret = wal_send_cfg_event(pst_cfg_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_ps_open_stru),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_PWR, "{wal_hipriv_sta_pm_open::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_enable_2040bss(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    uint8_t uc_2040bss_switch;

    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_hipriv_enable_2040bss::wal_get_cmd_one_arg return err_code %d!}\r\n", ret);
        return ret;
    }

    if ((0 != oal_strcmp("0", ac_name)) && (0 != oal_strcmp("1", ac_name))) {
        oam_error_log0(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::invalid parameter.}\r\n");
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    uc_2040bss_switch = (uint8_t)oal_atoi(ac_name);

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_2040BSS_ENABLE, sizeof(uint8_t));
    *((uint8_t *)(st_write_msg.auc_value)) = uc_2040bss_switch; /* 设置配置命令参数 */

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev,
                                       WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE,
                                       NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_enable_2040bss::return err code %d!}\r\n", ret);
    }

    return ret;
}

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)

OAL_STATIC uint32_t wal_hipriv_11v_tx_query(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    int32_t l_ret;
    uint32_t ret;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_CFG,
            "{wal_hipriv_11v_tx_query::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_11V_TX_QUERY, sizeof(auc_mac_addr));

    /* 设置配置命令参数 */
    if (EOK != memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), auc_mac_addr, WLAN_MAC_ADDR_LEN)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dpd_cfg::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(auc_mac_addr),
                               (uint8_t *)&st_write_msg,
                               OAL_FALSE,
                               NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_CFG, "{wal_hipriv_11v_tx_query::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}
#endif

#ifdef _PRE_WLAN_FEATURE_CSI

OAL_STATIC uint32_t wal_hipriv_set_csi(oal_net_device_stru *pst_cfg_net_dev, int8_t *pc_param)
{
    return wal_ioctl_set_csi_switch(pst_cfg_net_dev, pc_param);
}
#endif

OAL_STATIC uint32_t wal_hipriv_parase_send_frame_body(int8_t *p_str_body, uint8_t uc_str_body_len,
                                                      uint8_t *p_char_body, uint8_t uc_char_body_lenth,
                                                      uint8_t *p_char_body_len)
{
    uint8_t uc_index;
    uint8_t uc_value = 0;
    uint8_t uc_hvalue = 0;
    uint8_t uc_char_body_len = 0;

    if (oal_any_null_ptr3(p_str_body, p_char_body, p_char_body_len)) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_parase_send_frame_body::param is NULL,return!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((uc_str_body_len % 2) != 0) { /* 2用来计算奇偶数 */
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_parase_send_frame_body::uc_str_body_len=%d invalid,return!}\r\n", uc_str_body_len);
        return OAL_FAIL;
    }

    for (uc_index = 0; uc_index < uc_str_body_len; uc_index++) {
        if (p_str_body[uc_index] >= '0' && p_str_body[uc_index] <= '9') {
            uc_value = (uint8_t)(p_str_body[uc_index] - '0');
        } else if (p_str_body[uc_index] >= 'A' && p_str_body[uc_index] <= 'F') {
            uc_value = (uint8_t)(p_str_body[uc_index] - 'A' + 10); /* 'A'表示数值10 */
        } else {
            oam_warning_log2(0, OAM_SF_ANY,
                "{wal_hipriv_parase_send_frame_body::p_str_body[%d]=%c invalid,return!}\r\n",
                uc_index, (uint8_t)p_str_body[uc_index]);
            return OAL_FAIL;
        }

        if ((uc_index % 2) == 0) { /* 2用来计算奇偶数 */
            uc_hvalue = ((uc_value << BIT_OFFSET_4) & 0XF0);
        } else if ((uc_index % 2) == 1) { /* 2用来计算奇偶数 */
            p_char_body[uc_char_body_len] = uc_value + uc_hvalue;
            uc_value = 0;
            uc_char_body_len++;
            if (uc_char_body_len >= uc_char_body_lenth) {
                oam_warning_log2(0, OAM_SF_ANY,
                    "{wal_hipriv_parase_send_frame_body::uc_char_body_len =%d over_max = %d!}\r\n",
                    uc_char_body_len, MAC_TEST_INCLUDE_FRAME_BODY_LEN);
                return OAL_FAIL;
            }
        } else {
            uc_value = 0;
        }
    }

    *p_char_body_len = uc_char_body_len;
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_send_frame(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    mac_cfg_send_frame_param_stru *pst_test_send_frame = NULL;
    uint8_t auc_mac_addr[WLAN_MAC_ADDR_LEN] = { 0 };

    mac_test_frame_type_enum_uint8 en_frame_type;
    uint8_t uc_pkt_num;
    /* 所需内存是MAC_TEST_INCLUDE_FRAME_BODY_LEN的2倍 */
    int8_t ac_str_frame_body[MAC_TEST_INCLUDE_FRAME_BODY_LEN * 2] = { 0 };
    uint8_t uc_str_frame_body_len;
    uint8_t ac_frame_body[MAC_TEST_INCLUDE_FRAME_BODY_LEN] = { 0 };
    uint8_t uc_frame_body_len = 0;

    /* 获取帧类型 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get frame type err_code [%d]!}\r\n", ret);
        return ret;
    }
    en_frame_type = (mac_test_frame_type_enum_uint8)oal_atoi(ac_name);
    pc_param = pc_param + offset;

    /* 获取帧数目 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get frame num err_code [%d]!}\r\n", ret);
        return ret;
    }
    uc_pkt_num = (uint8_t)oal_atoi(ac_name);
    pc_param += offset;

    /* 获取MAC地址字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get mac err_code [%d]!}\r\n", ret);
        return ret;
    }
    /* 地址字符串转地址数组 */
    oal_strtoaddr(ac_name, sizeof(ac_name), auc_mac_addr, WLAN_MAC_ADDR_LEN);
    pc_param += offset;

    if (en_frame_type == MAC_TEST_MGMT_ACTION || en_frame_type == MAC_TEST_MGMT_BEACON_INCLUDE_IE) {
        ret = wal_get_cmd_one_arg(pc_param, ac_str_frame_body, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::get frame body err_code = [%d]!}\r\n", ret);
            return ret;
        }

        pc_param += offset;
        uc_str_frame_body_len = (uint8_t)(OAL_STRLEN(ac_str_frame_body));
        ret = wal_hipriv_parase_send_frame_body(ac_str_frame_body, uc_str_frame_body_len, ac_frame_body,
            sizeof(ac_frame_body), &uc_frame_body_len);
        if (ret != OAL_SUCC) {
            oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::parase_send_frame_body err_code = [%d]!}", ret);
            return ret;
        }
    }

    /***************************************************************************
                                 抛事件到dmac层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SEND_FRAME, sizeof(mac_cfg_send_frame_param_stru));

    /* 设置配置命令参数 */
    pst_test_send_frame = (mac_cfg_send_frame_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_test_send_frame, sizeof(mac_cfg_send_frame_param_stru),
             0, sizeof(mac_cfg_send_frame_param_stru));
    oal_set_mac_addr(pst_test_send_frame->auc_mac_ra, auc_mac_addr);
    pst_test_send_frame->en_frame_type = en_frame_type;
    pst_test_send_frame->uc_pkt_num = uc_pkt_num;
    pst_test_send_frame->uc_frame_body_length = uc_frame_body_len;
    if (memcpy_s(pst_test_send_frame->uc_frame_body, sizeof(pst_test_send_frame->uc_frame_body),
                 ac_frame_body, uc_frame_body_len) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_send_frame::memcpy fail!}");
        return OAL_FAIL;
    }

    oam_warning_log3(0, OAM_SF_ANY, "{wal_hipriv_send_frame:: frame_type = [%d] send_times = %d body_len=%d!}\r\n",
                     pst_test_send_frame->en_frame_type, pst_test_send_frame->uc_pkt_num,
                     pst_test_send_frame->uc_frame_body_length);
    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_send_frame_param_stru),
                                       (uint8_t *)&st_write_msg,
                                       OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_send_frame::wal_send_cfg_event return err_code [%d]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_roam_start(oal_net_device_stru *net_dev, int8_t *param)
{
    int32_t l_ret;
    uint8_t scan_type = ROAM_SCAN_CHANNEL_ORG_FULL;
    oal_bool_enum_uint8 en_current_bss_ignore = OAL_TRUE;
    oal_bool_enum_uint8 is_roaming_trigged_by_cmd = OAL_FALSE;
    uint8_t bssid[OAL_MAC_ADDR_LEN] = { 0 };
    wal_msg_write_stru write_msg;
    mac_cfg_set_roam_start_stru *roam_start = NULL;

    uint32_t off_set, param_temp, ret;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
    /* 指定漫游时刻是否搭配扫描操作 */
        scan_type = (uint8_t)oal_atoi(ac_name);
        param += off_set;
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
    /* 默认不指定bssid */
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else {
        return ret;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
        param_temp = (uint32_t)oal_atoi(ac_name);
    /* 0/TRUE或者参数缺失表示漫游到其它AP, 1/FALSE表示漫游到自己 */
        en_current_bss_ignore = (param_temp == 0);
        param += off_set;
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else {
        return ret;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
    /* 漫游是否由命令触发 */
        is_roaming_trigged_by_cmd = (uint8_t)oal_atoi(ac_name);
        param += off_set;
    } else if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else {
        return ret;
    }

    ret = wal_hipriv_get_mac_addr(param, bssid, &off_set);
    if (ret == OAL_ERR_CODE_CONFIG_ARGS_OVER) {
        memset_s(bssid, sizeof(bssid), 0, sizeof(bssid));
    } else if (ret != OAL_SUCC) {
        return ret;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    memset_s(&write_msg, sizeof(wal_msg_write_stru), 0, sizeof(wal_msg_write_stru));
    roam_start = (mac_cfg_set_roam_start_stru *)(write_msg.auc_value);
    roam_start->uc_scan_type = scan_type;
    roam_start->en_current_bss_ignore = en_current_bss_ignore;
    roam_start->is_roaming_trigged_by_cmd = is_roaming_trigged_by_cmd;
    if (memcpy_s(roam_start->auc_bssid, sizeof(roam_start->auc_bssid), bssid, sizeof(bssid)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_roam_start::memcpy fail!}");
        return OAL_FAIL;
    }
    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_ROAM_START, sizeof(mac_cfg_set_roam_start_stru));

    l_ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_set_roam_start_stru),
                               (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ROAM, "{wal_hipriv_roam_start::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


uint32_t wal_hipriv_dyn_cali_cfg(oal_net_device_stru *pst_net_dev, int8_t *puc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    mac_ioctl_dyn_cali_param_stru *pst_dyn_cali_param = NULL;
    wal_ioctl_dyn_cali_stru st_cyn_cali_cfg;
    uint32_t ret;
    uint8_t uc_map_index = 0;

    pst_dyn_cali_param = (mac_ioctl_dyn_cali_param_stru *)(st_write_msg.auc_value);
    memset_s(pst_dyn_cali_param, sizeof(mac_ioctl_dyn_cali_param_stru),
             0, sizeof(mac_ioctl_dyn_cali_param_stru));

    if (oal_unlikely(oal_any_null_ptr2(pst_net_dev, puc_param))) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dyn_cali_cfg::pst_cfg_net_dev or puc_param null ptr error }\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = wal_get_cmd_one_arg(puc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_dyn_cali_cfg::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }
    /* 寻找匹配的命令 */
    st_cyn_cali_cfg = g_ast_dyn_cali_cfg_map[0];
    while (st_cyn_cali_cfg.pc_name != NULL) {
        if (0 == oal_strcmp(st_cyn_cali_cfg.pc_name, ac_name)) {
            break;
        }
        st_cyn_cali_cfg = g_ast_dyn_cali_cfg_map[++uc_map_index];
    }

    /* 没有找到对应的命令，则报错 */
    if (st_cyn_cali_cfg.pc_name == NULL) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_dyn_cali_cfg::invalid alg_cfg command!}\r\n");
        return OAL_FAIL;
    }

    /* 记录命令对应的枚举值 */
    pst_dyn_cali_param->en_dyn_cali_cfg = g_ast_dyn_cali_cfg_map[uc_map_index].en_dyn_cali_cfg;
    /* 获取参数配置值 */
    ret = wal_get_cmd_one_arg(puc_param + off_set, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret == OAL_SUCC) {
        /* 记录参数配置值 */
        pst_dyn_cali_param->us_value = (uint16_t)oal_atoi(ac_name);
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_dyn_cali_cfg::wal_get_cmd_one_arg [%d]!}\r\n", pst_dyn_cali_param->us_value);
    }

    /***************************************************************************
                             抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_DYN_CALI_CFG, sizeof(uint32_t));

    ret = (uint32_t)wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                                       WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint32_t),
                                       (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_cyn_cali_set_dscr_interval::wal_send_cfg_event return err code [%d]!}\r\n", ret);
        return ret;
    }
    return OAL_SUCC;
}


#define WAL_HIPRIV_OWE_MAX 8
uint32_t wal_hipriv_set_owe(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t ret;
    int32_t l_ret;
    uint8_t uc_owe_group;
    int8_t ac_arg[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    wal_msg_stru *pst_rsp_msg = NULL;

    /* 获取字符串 */
    ret = wal_get_cmd_one_arg(pc_param, ac_arg, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_set_owe::wal_get_cmd_one_arg return err_code [%d]!}\r\n", ret);
        return ret;
    }

    /* 解析要设置的值 */
    uc_owe_group = (uint8_t)oal_atoi(ac_arg);
    if (uc_owe_group >= WAL_HIPRIV_OWE_MAX) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_owe::input value out of range [%d]!}\r\n", uc_owe_group);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_SET_OWE, sizeof(uint8_t));

    /* 解析并设置配置命令参数 */
    if (memcpy_s(st_write_msg.auc_value, sizeof(st_write_msg.auc_value), &uc_owe_group, sizeof(uint8_t)) != EOK) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_set_owe::memcpy fail!}");
        return OAL_FAIL;
    }

    l_ret = wal_send_cfg_event(pst_net_dev,
                               WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(uint8_t),
                               (uint8_t *)&st_write_msg,
                               OAL_TRUE,
                               &pst_rsp_msg);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_set_owe::return err code [%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    /* 读取返回的错误码 */
    ret = wal_check_and_release_msg_resp(pst_rsp_msg);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_DFR, "{wal_hipriv_set_owe fail, err code[%u]!}\r\n", ret);
        return ret;
    }

    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_TWT
OAL_STATIC uint32_t wal_get_twt_setup_req_paras(int8_t *pc_param,
    mac_cfg_twt_setup_req_param_stru* st_twt_setup_req_param)
{
    uint32_t ret;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = { 0 };

    /* 获取interval_exponent */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_twt_setup_req_param->twt_exponent = (uint32_t)oal_atoi(ac_name);
    if (st_twt_setup_req_param->twt_exponent > 0x1F) { /* 最大值0x1F */
        oam_warning_log1(0, OAM_SF_11AX, "{wal_get_twt_setup_req_paras::wrong twt_exponent [%d]}",
            st_twt_setup_req_param->twt_exponent);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;
    /* 获取wake_duration */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_twt_setup_req_param->twt_duration = (uint32_t)oal_atoi(ac_name);
    if (st_twt_setup_req_param->twt_duration > 0xFF) { /* 最大值0xFF */
        oam_warning_log1(0, OAM_SF_11AX, "{wal_get_twt_setup_req_paras::wrong twt_duration[%d]}",
            st_twt_setup_req_param->twt_duration);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    pc_param = pc_param + off_set;
    /* 获取interval_mantissa */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_get_twt_setup_req_paras:: get interval_mantissa fail [%d]!}", ret);
        return ret;
    }
    st_twt_setup_req_param->intrval_mantissa = (uint16_t)oal_atoi(ac_name);

    if (st_twt_setup_req_param->intrval_mantissa > 0xFFFF) { /* 最大值0xFFFF */
        oam_warning_log1(0, OAM_SF_11AX,
            "{wal_get_twt_setup_req_paras::wrong intrval_mantissa [%d]}", st_twt_setup_req_param->intrval_mantissa);
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    return OAL_SUCC;
}


OAL_STATIC uint32_t wal_hipriv_twt_setup_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_twt_setup_req_param_stru *pst_twt_setup_req_param;
    mac_cfg_twt_setup_req_param_stru st_twt_setup_req_param; /* 临时保存获取的twt setup req的信息 */
    uint32_t get_addr_idx;

    /*
        设置AMPDU关闭的配置命令: hipriv "Hisilicon0 twt_setup_req xx xx xx xx xx xx(mac地址) nego_type
        setup_cmd flow_type flow_ID start_time_offset interval_exponent wake_duration interval_mantissa"
    */
    oam_warning_log0(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req\r\n");

    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }

    memset_s((uint8_t *)&st_twt_setup_req_param, sizeof(st_twt_setup_req_param),
             0, sizeof(st_twt_setup_req_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_twt_setup_req_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req::wal_get_cmd_one_arg return err_code [%d]!}", ret);
        return ret;
    }
    st_twt_setup_req_param.twt_nego_type = (uint8_t)oal_atoi(ac_name);

    /* 单播处理后续参数 */
    if (st_twt_setup_req_param.twt_nego_type == 0) {
        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        st_twt_setup_req_param.uc_twt_setup_cmd = (uint8_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        st_twt_setup_req_param.uc_twt_flow_type = (uint8_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        st_twt_setup_req_param.uc_twt_flow_id = (uint8_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
        if (ret != OAL_SUCC) {
            return ret;
        }
        st_twt_setup_req_param.twt_start_time_offset = (uint32_t)oal_atoi(ac_name);

        /* 偏移，取下一个参数 */
        pc_param = pc_param + off_set;
        ret = wal_get_twt_setup_req_paras(pc_param, &st_twt_setup_req_param);
        if (ret != OAL_SUCC) {
            return ret;
        }
    }

    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TWT_SETUP_REQ, sizeof(mac_cfg_twt_setup_req_param_stru));

    /* 设置配置命令参数 */
    pst_twt_setup_req_param = (mac_cfg_twt_setup_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_twt_setup_req_param->auc_mac_addr[get_addr_idx] = st_twt_setup_req_param.auc_mac_addr[get_addr_idx];
    }

    pst_twt_setup_req_param->uc_twt_setup_cmd = st_twt_setup_req_param.uc_twt_setup_cmd;
    pst_twt_setup_req_param->uc_twt_flow_type = st_twt_setup_req_param.uc_twt_flow_type;
    pst_twt_setup_req_param->uc_twt_flow_id = st_twt_setup_req_param.uc_twt_flow_id;
    pst_twt_setup_req_param->twt_start_time_offset = st_twt_setup_req_param.twt_start_time_offset;
    pst_twt_setup_req_param->twt_exponent = st_twt_setup_req_param.twt_exponent;
    pst_twt_setup_req_param->twt_duration = st_twt_setup_req_param.twt_duration;
    pst_twt_setup_req_param->intrval_mantissa = st_twt_setup_req_param.intrval_mantissa;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE, WAL_MSG_WRITE_MSG_HDR_LENGTH +
        sizeof(mac_cfg_twt_setup_req_param_stru), (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_setup_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}


OAL_STATIC uint32_t _wal_hipriv_twt_setup_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        return wal_hipriv_twt_setup_req(pst_net_dev, pc_param);
    }

    return OAL_SUCC;
}

/* 单播nego type 小于2，flow id最大0x7，广播nego type3 flow id最大0x1F */
#define CHECK_TEARDOWN_PARAS(_flow_id, _nego_type) (((((_flow_id) < 0x7) && ((_nego_type) < 2)) || \
    (((_flow_id) < 0x1F) && ((_nego_type) == 3))) ? OAL_TRUE : OAL_FALSE)
OAL_STATIC uint32_t wal_hipriv_twt_teardown_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    wal_msg_write_stru st_write_msg;
    uint32_t off_set;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t ret;
    int32_t l_ret;
    mac_cfg_twt_teardown_req_param_stru *pst_twt_teardown_req_param;
    mac_cfg_twt_teardown_req_param_stru st_twt_teardown_req_param; /* 临时保存获取的twt teardown req的信息 */
    uint32_t get_addr_idx;

    /*
     * 设置AMPDU关闭的配置命令:
     * hipriv "Hisilicon0 twt_teardown_req xx xx xx xx xx xx(mac地址) flwo_id nego_type"
     */
    /* 获取mac地址 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    memset_s((uint8_t *)&st_twt_teardown_req_param, sizeof(st_twt_teardown_req_param),
             0, sizeof(st_twt_teardown_req_param));
    oal_strtoaddr(ac_name, sizeof(ac_name), st_twt_teardown_req_param.auc_mac_addr, WLAN_MAC_ADDR_LEN);
    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取flwo_id */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }

    st_twt_teardown_req_param.uc_twt_flow_id = (uint8_t)oal_atoi(ac_name);

    /* 偏移，取下一个参数 */
    pc_param = pc_param + off_set;

    /* 获取nego_type */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        return ret;
    }
    st_twt_teardown_req_param.twt_nego_type = (uint8_t)oal_atoi(ac_name);
    if (!CHECK_TEARDOWN_PARAS(st_twt_teardown_req_param.uc_twt_flow_id, st_twt_teardown_req_param.twt_nego_type)) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }
    /***************************************************************************
                                抛事件到wal层处理
    ***************************************************************************/
    WAL_WRITE_MSG_HDR_INIT(&st_write_msg, WLAN_CFGID_TWT_TEARDOWN_REQ, sizeof(mac_cfg_twt_teardown_req_param_stru));

    /* 设置配置命令参数 */
    pst_twt_teardown_req_param = (mac_cfg_twt_teardown_req_param_stru *)(st_write_msg.auc_value);
    for (get_addr_idx = 0; get_addr_idx < WLAN_MAC_ADDR_LEN; get_addr_idx++) {
        pst_twt_teardown_req_param->auc_mac_addr[get_addr_idx] = st_twt_teardown_req_param.auc_mac_addr[get_addr_idx];
    }

    pst_twt_teardown_req_param->uc_twt_flow_id = st_twt_teardown_req_param.uc_twt_flow_id;

    l_ret = wal_send_cfg_event(pst_net_dev, WAL_MSG_TYPE_WRITE,
                               WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_cfg_twt_teardown_req_param_stru),
                               (uint8_t *)&st_write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(l_ret != OAL_SUCC)) {
        oam_warning_log1(0, OAM_SF_11AX, "{wal_hipriv_twt_teardown_req::return err code[%d]!}\r\n", l_ret);
        return (uint32_t)l_ret;
    }

    return OAL_SUCC;
}

OAL_STATIC uint32_t _wal_hipriv_twt_teardown_req(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    if (g_wlan_spec_cfg->feature_twt_is_open) {
        return wal_hipriv_twt_teardown_req(pst_net_dev, pc_param);
    }

    return OAL_SUCC;
}
#endif


uint32_t wal_hipriv_fem_lowpower(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    uint32_t ret;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    int32_t lowpower_switch;
    uint32_t cmd_id;

    /* 获取开关 */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_fem_lowpower::first para return err_code [%d]!}\r\n", ret);
        return ret;
    }

    lowpower_switch = oal_atoi((const int8_t *)ac_name);

    ret = wal_get_cmd_id("fem_lowpower", &cmd_id, &offset);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "wal_hipriv_fem_lowpower:find fem_lowpower cmd is fail");
        return ret;
    }

    return wal_process_cmd_params(pst_net_dev, cmd_id, &lowpower_switch);
}

OAL_STATIC uint32_t wal_hipriv_psm_flt_stat(oal_net_device_stru *pst_net_dev, int8_t *pc_param)
{
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t off_set;
    uint32_t ret;
    uint8_t uc_query_type;
    /* 获取query type */
    ret = wal_get_cmd_one_arg(pc_param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &off_set);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_psm_flt_stat::first para return err_code [%d]!}\r\n", ret);
        return ret;
    }

    uc_query_type = (uint8_t)oal_atoi((const int8_t *)ac_name);
    wal_ioctl_get_psm_stat(pst_net_dev, uc_query_type, NULL);
    return OAL_SUCC;
}

#ifdef _PRE_WLAN_FEATURE_GET_STATION_INFO_EXT
OAL_STATIC void wal_hipriv_printf_station_info(hmac_vap_stru *hmac_vap)
{
#ifdef CONFIG_HW_GET_EXT_SIG_ULDELAY
    oam_warning_log4(0, OAM_SF_ANY,
        "{wal_hipriv_get_station_info::tid delay[%d]ms, tx drop[%d], tx pkts[%d], tx bytes[%d]}",
        hmac_vap->station_info.ul_delay,
        hmac_vap->station_info.tx_failed,
        hmac_vap->station_info.tx_packets,
        hmac_vap->station_info.tx_bytes);
#else
    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_hipriv_get_station_info::tx drop[%d], tx pkts[%d], tx bytes[%d]}",
        hmac_vap->station_info.tx_failed,
        hmac_vap->station_info.tx_packets,
        hmac_vap->station_info.tx_bytes);
#endif
    oam_warning_log3(0, OAM_SF_ANY,
        "{wal_hipriv_get_station_info::rx pkts[%lu], rx bytes[%lu], rx drop[%lu]}",
        hmac_vap->station_info.rx_packets,
        hmac_vap->station_info.rx_bytes,
        hmac_vap->station_info.rx_dropped_misc);
}

OAL_STATIC uint32_t wal_hipriv_get_station_info(oal_net_device_stru *net_dev, int8_t *param)
{
    mac_vap_stru *mac_vap = NULL;
    hmac_vap_stru *hmac_vap = NULL;
    dmac_query_request_event dmac_query_request_event;
    dmac_query_station_info_request_event *query_station_info = NULL;
    wal_msg_write_stru write_msg;
    signed long leftime;
    int32_t ret;
    uint8_t vap_id;
    if (oal_any_null_ptr2(net_dev, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    mac_vap = oal_net_dev_priv(net_dev);
    if (mac_vap == NULL) {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_get_station_info::OAL_NET_DEV_PRIV, return null!}");
        return OAL_ERR_CODE_PTR_NULL;
    }

    vap_id = mac_vap->uc_vap_id;

    query_station_info = (dmac_query_station_info_request_event *)&dmac_query_request_event;
    query_station_info->query_event = OAL_QUERY_STATION_INFO_EVENT;

    hmac_vap = (hmac_vap_stru *)mac_res_get_hmac_vap(vap_id);
    if (hmac_vap == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    hmac_vap->station_info_query_completed_flag = OAL_FALSE;

    /********************************************************************************
        抛事件到wal层处理 ，对于低功耗需要做额外处理，不能像下层抛事件，直接起定时器
        低功耗会在接收beacon帧的时候主动上报信息。
    ********************************************************************************/
    /* 3.1 填写 msg 消息头 */
    write_msg.en_wid = WLAN_CFGID_QUERY_STATION_STATS;
    write_msg.us_len = sizeof(dmac_query_request_event);

    /* 3.2 填写 msg 消息体 */
    if (memcpy_s(write_msg.auc_value, sizeof(write_msg.auc_value),
        query_station_info, sizeof(dmac_query_station_info_request_event)) != EOK) {
        oam_error_log0(vap_id, OAM_SF_ANY, "wal_hipriv_get_station_info:memcpy_s fail !");
        return OAL_FAIL;
    }

    ret = wal_send_cfg_event(net_dev, WAL_MSG_TYPE_WRITE,
                             WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(dmac_query_station_info_request_event),
                             (uint8_t *)&write_msg, OAL_FALSE, NULL);
    if (oal_unlikely(ret != OAL_SUCC)) {
        oam_warning_log1(vap_id, 0, "{wal_hipriv_get_station_info::wal_send_cfg_event err code %d!}", ret);
        return OAL_FAIL;
    }
    /*lint -e730*/ /* info, boolean argument to function */
    leftime = oal_wait_event_interruptible_timeout_m(hmac_vap->query_wait_q,
        (OAL_TRUE == hmac_vap->station_info_query_completed_flag), 5 * OAL_TIME_HZ); /* 5s超时 */
    /*lint +e730*/
    if (leftime <= 0) {
        oam_warning_log0(vap_id, OAM_SF_ANY, "{wal_hipriv_get_station_info::query info wait for 5 s timeout!}");
        return OAL_FAIL;
    } else {
        /* 正常结束  */
        wal_hipriv_printf_station_info(hmac_vap);
        return OAL_SUCC;
    }
}
#endif

#ifdef _PRE_WLAN_FEATURE_NAN
oal_bool_enum_uint8 wal_nan_check_param(mac_nan_param_stru *param)
{
    /* period对应NAN扫描间隔，duration对应NAN扫描持续时间。间隔太短或者持续时间太长都会影响其他业务 */
    if (param->period <= param->duration ||
        param->period < WAL_NAN_MIN_PERIOD || param->duration > WAL_NAN_MAX_DURATION ||
        mac_is_channel_num_valid(param->band, param->channel, OAL_FALSE) != OAL_SUCC) {
        oam_warning_log0(0, 0, "wal_cfgvendor_nan_check_param::param check fail");
        return OAL_FALSE;
    }
    return OAL_TRUE;
}

OAL_STATIC uint32_t wal_hipriv_nan_set_param(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t offset;
    uint32_t ret;
    mac_nan_cfg_msg_stru *nan_msg;
    mac_nan_param_stru *nan_param;

    nan_msg = (mac_nan_cfg_msg_stru*)write_msg.auc_value;
    nan_msg->type = NAN_CFG_TYPE_SET_PARAM;
    nan_param = &nan_msg->param;

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->duration = oal_atoi(ac_name);

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->period = oal_atoi(ac_name);

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->band = (uint8_t)oal_atoi(ac_name);

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    nan_param->channel = (uint8_t)oal_atoi(ac_name);
    if (wal_nan_check_param(nan_param) == OAL_FALSE) {
        /* 上层参数错误的话，将duration和period置0，dmac检测到duration或period为0后，会stop nan */
        nan_param->duration = 0;
        nan_param->period = 0;
    }

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_NAN, sizeof(mac_nan_cfg_msg_stru));
    if (OAL_SUCC != wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_nan_cfg_msg_stru), (uint8_t*)&write_msg, OAL_FALSE,
        NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_cfgvendor_nan_send_event::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_nan_tx_mgmt(oal_net_device_stru *net_device, int8_t *param)
{
    wal_msg_write_stru write_msg;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN] = {0};
    uint32_t offset;
    uint32_t ret;
    mac_nan_cfg_msg_stru *nan_msg;
    mac_nan_mgmt_info_stru *mgmt_info;
    wal_msg_stru *pst_rsp_msg = NULL;
    uint8_t ac_frame_body[WAL_HIPRIV_CMD_NAME_MAX_LEN >> 1] = { 0 };
    uint8_t str_frame_len;
    uint8_t frame_len = 0;
    nan_msg = (mac_nan_cfg_msg_stru*)write_msg.auc_value;
    nan_msg->type = NAN_CFG_TYPE_SET_TX_MGMT;
    mgmt_info = &nan_msg->mgmt_info;

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }
    mgmt_info->periodic = !(uint8_t)oal_atoi(ac_name);
    mgmt_info->action = WLAN_ACTION_NAN_FLLOWUP; /* 调试命令写死 */

    param += offset;
    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        return ret;
    }

    str_frame_len = (uint8_t)(OAL_STRLEN(ac_name));
    ret = wal_hipriv_parase_send_frame_body(ac_name, str_frame_len, ac_frame_body,
                                            sizeof(ac_frame_body), &frame_len);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY,
            "{wal_hipriv_send_frame::parase_send_frame_body err_code = [%d]!}\r\n", ret);
    }
    mgmt_info->len = frame_len;
    mgmt_info->data = ac_frame_body;

    WAL_WRITE_MSG_HDR_INIT(&write_msg, WLAN_CFGID_NAN, sizeof(mac_nan_cfg_msg_stru));
    if (OAL_SUCC != wal_send_cfg_event(net_device, WAL_MSG_TYPE_WRITE,
        WAL_MSG_WRITE_MSG_HDR_LENGTH + sizeof(mac_nan_cfg_msg_stru), (uint8_t*)&write_msg, OAL_TRUE,
        &pst_rsp_msg)) {
        oam_error_log0(0, OAM_SF_ANY, "{wal_hipriv_nan_tx_mgmt::wal_send_cfg_event fail!}");
        return -OAL_EFAIL;
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t wal_hipriv_nan_dbg(oal_net_device_stru *net_device, int8_t *param)
{
    uint32_t ret;
    uint32_t offset;
    int8_t ac_name[WAL_HIPRIV_CMD_NAME_MAX_LEN];
    if (oal_any_null_ptr2(net_device, param)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = wal_get_cmd_one_arg(param, ac_name, WAL_HIPRIV_CMD_NAME_MAX_LEN, &offset);
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_nan_dbg::first para return err_code [%d]!}", ret);
        return ret;
    }
    param += offset;
    if (oal_strcmp(ac_name, "set_param") == 0) {
        ret = wal_hipriv_nan_set_param(net_device, param);
    } else if (oal_strcmp(ac_name, "tx_mgmt") == 0) {
        ret = wal_hipriv_nan_tx_mgmt(net_device, param);
    } else {
        oam_warning_log0(0, OAM_SF_ANY, "{wal_hipriv_nan_dbg::not support the sub cmd!}");
    }
    if (ret != OAL_SUCC) {
        oam_warning_log1(0, OAM_SF_ANY, "{wal_hipriv_nan_dbg::execute cmd failed, error[%d]!}", ret);
    }
    return ret;
}
#endif
const wal_hipriv_cmd_entry_stru g_ast_hipriv_cmd_debug[] = {
    /***********************调试命令***********************/
    /* 设置TLV类型通用命令: hipriv "wlan0 set_tlv xxx 0 0"  */
    { "set_tlv", wal_hipriv_set_tlv_cmd },
    /* 设置Value类型通用命令: hipriv "wlan0 set_val xxx 0"  */
    { "set_val", wal_hipriv_set_val_cmd },
    /* 设置String类型通用命令: hipriv "wlan0 set_str 11ax_debug 3 tid 0 val 1 cnt 2"  */
    { "set_str", wal_hipriv_set_str_cmd },

#ifdef _PRE_WLAN_FEATURE_TWT
    { "twt_setup_req", _wal_hipriv_twt_setup_req },
    { "twt_teardown_req", _wal_hipriv_twt_teardown_req },
#endif

    { "destroy", wal_hipriv_del_vap }, /* 删除vap私有命令为: hipriv "vap0 destroy" */
    /* 特性的INFO级别日志开关 hipriv "VAPX feature_name {0/1}" */
    { "feature_log_switch", wal_hipriv_feature_log_switch },
#ifdef _PRE_WLAN_NARROW_BAND
    { "narrow_bw", wal_hipriv_narrow_bw }, /* Start DPD Calibration */
#endif
    /* 设置是否上报接收描述符帧开关: hipriv "Hisilicon0 ota_rx_dscr_switch 0 | 1"，该命令针对所有的VAP */
    { "ota_switch",       wal_hipriv_ota_rx_dscr_switch },
    /* 设置建立BA会话的配置命令:hipriv "vap0 addba_req xx xx xx xx xx xx(mac地址)
       tidno ba_policy buffsize timeout" 该命令针对某一个VAP */
    { "addba_req",        wal_hipriv_addba_req },
    /* 设置删除BA会话的配置命令: hipriv "vap0 delba_req xx xx xx xx xx xx(mac地址)
       tidno direction" 该命令针对某一个VAP */
    { "delba_req",        wal_hipriv_delba_req },
#ifdef _PRE_WLAN_FEATURE_WMMAC
    /* 设置WMMAC开关，配置命令: hipriv "vap0 wmmac_switch 1/0(使能)
       0|1(WMM_AC认证使能) xxx(timeout_period) factor" 整个Device */
    { "wmmac_switch", wal_hipriv_wmmac_switch },
#endif
    { "memoryinfo", wal_hipriv_memory_info }, /* 打印内存池信息: hipriv "Hisilicon0 memoryinfo host/device" */
    /* 配置BA删建门限，sh hipriv.sh "vap0 set_coex_perf_param  0/1/2/3" */
    { "set_coex_perf_param", wal_hipriv_btcoex_set_perf_param },
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    { "nrcoex_test", wal_hipriv_nrcoex_cfg_test }, /* 配置NR共存参数，sh hipriv.sh "nrcoex_test 0~20" */
#endif
    /* 设置打印phy维测的相关信息，sh hipriv.sh
       "wlan0 protocol_debug [band_force_switch 0|1|2] [2040_ch_swt_prohi 0|1] [40_intol 0|1]" */
    { "protocol_debug", wal_hipriv_set_protocol_debug_info },
    /* 配置发送功率参数: hipriv "vap0 set_tx_pow <param name> <value>" */
    { "set_tx_pow", wal_hipriv_set_tx_pow_param },
    /* 打印描述符信息: hipriv "vap0 set_ucast_data <param name> <value>" */
    { "set_ucast_data",  wal_hipriv_set_ucast_data_dscr_param },
    /* 打印描述符信息: hipriv "vap0 set_bcast_data <param name> <value>" */
    { "set_bcast_data",  wal_hipriv_set_bcast_data_dscr_param },
    /* 打印描述符信息: hipriv "vap0 set_ucast_mgmt <param name> <value>" */
    { "set_ucast_mgmt",  wal_hipriv_set_ucast_mgmt_dscr_param },
    /* 打印描述符信息: hipriv "vap0 set_mbcast_mgmt <param name> <value>" */
    { "set_mbcast_mgmt", wal_hipriv_set_mbcast_mgmt_dscr_param },
    { "nss",           wal_hipriv_set_nss }, /* 设置HT模式下的空间流个数:   hipriv "vap0 nss   <value>" */
    { "txch",          wal_hipriv_set_rfch }, /* 设置发射通道:              hipriv "vap0 rfch  <value>" */

    { "start_scan",   wal_hipriv_start_scan }, /* 触发sta扫描: hipriv "sta0 start_scan" */
    { "kick_user",    wal_hipriv_kick_user }, /* 删除1个用户 hipriv "vap0 kick_user xx xx xx xx xx xx(mac地址)" */

    { "ampdu_tx_on",  wal_hipriv_ampdu_tx_on },     /* 开启或关闭ampdu发送功能 hipriv "vap0 ampdu_tx_on 0\1" */
    { "amsdu_tx_on",  wal_hipriv_amsdu_tx_on },     /* 开启或关闭ampdu发送功能 hipriv "vap0 amsdu_tx_on 0\1" */
    { "send_bar",     wal_hipriv_send_bar }, /* 指定tid发送bar hipriv "vap0 send_bar A6C758662817(mac地址) tid_num" */
    /* 复位硬件phy&mac: hipriv "Hisilicon0 reset_hw 0|1|2|3|4|5|6|8|9|10|11
       (all|phy|mac|debug|mac_tsf|mac_cripto|mac_non_cripto|phy_AGC|phy_HT_optional|phy_VHT_optional|phy_dadar )
       0|1(reset phy reg) 0|1(reset mac reg) */
    { "reset_hw",             wal_hipriv_reset_device },
    /* 复位硬件phy&mac: hipriv "Hisilicon0 reset_hw 0|1|2|3(all|phy|mac|debug) 0|1(reset phy reg) 0|1(reset mac reg) */
    { "reset_operate",       wal_hipriv_reset_operate },
    /* 动态开启或者关闭wmm hipriv "vap0 wmm_switch 0|1"(0不使能，1使能) */
    { "wmm_switch",          wal_hipriv_wmm_switch },
    /* 设置以太网帧上报的开关，
       sh hipriv.sh "vap0 ether_switch user_macaddr oam_ota_frame_direction_type_enum(帧方向) 0|1(开关)" */
    { "ether_switch",        wal_hipriv_set_ether_switch },
    /* 设置80211单播帧上报的开关，sh hipriv.sh "vap0 80211_uc_switch user_macaddr
       0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)" */
    { "80211_uc_switch",     wal_hipriv_set_80211_ucast_switch },
    /* 设置打印phy维测的相关信息，sh hipriv.sh "Hisilicon0 phy_debug [snr 0|1] [rssi 0|1] [trlr 0|1] [count N]" */
    { "phy_debug",           wal_hipriv_set_phy_debug_switch },
    /* 设置所有用户的单播开关，sh hipriv.sh "Hisilicon0 80211_uc_all
       0|1(帧方向tx|rx) 0|1(帧类型:管理帧|数据帧) 0|1(帧内容开关) 0|1(CB开关) 0|1(描述符开关)" */
    { "80211_uc_all",    wal_hipriv_set_all_80211_ucast },
#ifdef _PRE_WLAN_DFT_STAT
    /* 上报或者清零用户队列统计信息: sh hipriv.sh "vap_name usr_queue_stat XX:XX:XX:XX:XX:XX 0|1" */
    { "usr_queue_stat", wal_hipriv_usr_queue_stat },
    /* 上报或者清零所有维测统计信息: sh hipriv.sh "Hisilicon0 reprt_all_stat type(phy/machw/mgmt/irq/all)  0|1" */
    { "reprt_all_stat", wal_hipriv_report_all_stat },
#endif
    /* 设置AMPDU聚合个数: sh hipriv.sh "Hisilicon0 ampdu_aggr_num aggr_num_switch aggr_num",
       aggr_num_switch非0时，aggr_num有效 */
    { "ampdu_aggr_num", wal_hipriv_set_ampdu_aggr_num },

    { "set_sta_pm_on", wal_hipriv_sta_pm_on }, /* sh hipriv.sh 'wlan0 set_sta_pm_on xx xx xx xx */
    { "send_frame", wal_hipriv_send_frame }, /* 指定tid发送bar hipriv "vap0 send_frame (type) (num) (目的mac)" */
    /* chip test配置add key操作的私有配置命令接口
       sh hipriv.sh "xxx(cipher) xx(en_pairwise) xx(key_len) xxx(key_index)
       xxxx:xx:xx:xx:xx:xx...(key 小于32字节) xx:xx:xx:xx:xx:xx(目的地址)  */
    { "add_key", wal_hipriv_test_add_key },

    /* 算法相关的命令 */
    { "alg_ar_log",  wal_hipriv_ar_log },   /* autorate算法日志参数配置: */
    { "alg_ar_test", wal_hipriv_ar_test }, /* autorate算法系统测试命令 */

    { "alg_txbf_log", wal_hipriv_txbf_log },         /* txbf算法日志参数配置: */
    { "alg_intf_det_log", wal_hipriv_intf_det_log }, /* 干扰检测算法日志参数配置: */
    { "get_version",     wal_hipriv_get_version },         /* 获取软件版本: hipriv "vap0 get_version" */

#ifdef _PRE_WLAN_FEATURE_FTM
    /* hipriv.sh "wlan0 ftm_debug -----------------------*/
    /* -------------------------- enable_ftm_initiator [0|1] */
    /* -------------------------- send_initial_ftm_request channel[] burst[0|1] ftms_per_burst[n] \
                                  en_lci_civic_request[0|1] asap[0|1] bssid[xx:xx:xx:xx:xx:xx] */
    /* -------------------------- enable [0|1] */
    /* -------------------------- cali [0|1] */
    /* -------------------------- send_ftm bssid[xx:xx:xx:xx:xx:xx] */
    /* -------------------------- set_ftm_time t1[] t2[] t3[] t4[] */
    /* -------------------------- enable_ftm_responder [0|1] */
    /* -------------------------- send_range_req mac[] num_rpt[] delay[] ap_cnt[] bssid[] channel[] \
                                  bssid[] channel[] ... */
    /* -------------------------- enable_ftm_range [0|1] */
    /* -------------------------- get_cali */
    /* -------------------------- set_location type[] mac[] mac[] mac[] */
    /* -------------------------- set_ftm_m2s ftm_chain_selection[] */
    { "ftm_debug", _wal_hipriv_ftm },
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    /* 使能CSI上报: hipriv "Hisilicon0 set_csi xx xx xx xx xx xx(mac地址) ta_check csi_en" */
    { "set_csi", wal_hipriv_set_csi },
#endif

#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    /* 设置添加用户的配置命令: hipriv "vap0 get_user_nssbw xx xx xx xx xx xx(mac地址) "  该命令针对某一个VAP */
    { "get_user_nssbw", wal_hipriv_get_user_nssbw },
#endif

#ifdef _PRE_WLAN_FEATURE_M2S
    /* mimo和siso切换: hipriv "Hisilicon0 set_m2s_switch 0/1/2/3/4/5(参数查询/配置模式/resv/软切换或硬切换测试模式) */
    { "set_m2s_switch", wal_hipriv_set_m2s_switch },
#endif

    { "blacklist_add",  wal_hipriv_blacklist_add },       /* 1 */
    { "blacklist_del",  wal_hipriv_blacklist_del },       /* 2 */
    { "blacklist_mode", wal_hipriv_set_blacklist_mode }, /* 3 */
    /* device级别配置命令 设置基于vap的业务分类是否使能 hipriv "Hisilicon0 vap_classify_en 0/1" */
    { "vap_classify_en", wal_hipriv_vap_classify_en },
    { "vap_classify_tid", wal_hipriv_vap_classify_tid }, /* 设置vap的流等级 hipriv "vap0 classify_tid 0~7" */
    /* 设置最大发送功率，要求功率值按照扩大10倍来输入，例如最大功率要限制为20，输入200 */
    { "txpower", wal_hipriv_set_txpower },
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44))
    { "essid",   wal_hipriv_set_essid },        /* 设置AP ssid */
    { "bintval", wal_ioctl_set_beacon_interval }, /* 设置AP beacon 周期 */
    { "up",      wal_hipriv_start_vap },
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,44)) */
#ifdef _PRE_WLAN_FEATURE_WAPI
#ifdef _PRE_WAPI_DEBUG
    { "wapi_info", wal_hipriv_show_wapi_info }, /* wapi hipriv "vap0 wal_hipriv_show_wapi_info " */
#endif
#endif                                          /* #ifdef _PRE_WLAN_FEATURE_WAPI */
    /* 漫游测试命令 sh hipriv.sh "wlan0 roam_start 0|1|2|3|4 0/1"
     * 0或者参数缺失表示漫游前不扫描, 1|2|3|4表示扫描+漫游
     * 第二个参数0或者参数缺失表示漫游到其它AP, 1表示漫游到自己
    */
    { "roam_start",       wal_hipriv_roam_start },
    /* 设置20/40 bss使能: hipriv "Hisilicon0 2040bss_enable 0|1" 0表示20/40 bss判断关闭，1表示使能 */
    { "2040bss_enable", wal_hipriv_enable_2040bss },

    { "dyn_cali", wal_hipriv_dyn_cali_cfg }, /*  动态校准参数配置 sh hipriv "wlan0 dyn_cali   " */

    { "dpd_cfg", wal_hipriv_dpd_cfg },     /* 打印定制化信息 */

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    /* 11v特性配置:  触发sta发送11v Query帧: hipriv "vap[x] 11v_tx_query [mac-addr]" */
    { "11v_tx_query", wal_hipriv_11v_tx_query },
#endif
     /* DBDC特性开关，sh hipriv.sh "wlan0 dbdc_debug [change_hal_dev 0|1] " */
    { "dbdc_debug", wal_hipriv_set_dbdc_debug_switch },
    { "pm_debug", wal_hipriv_set_pm_debug_switch }, /* 低功耗debug命令 */

    { "set_owe", wal_hipriv_set_owe }, /* 设置APUT模式支持的OWE group: hipriv "wlan0 set_owe <value>" */
    /* bit 0/1/2分别表示支持owe group 19/20/21 */
    /* 打印低功耗帧过滤统计 sh hipriv.sh "wlan0 psm_flt_stat [0|1|2|...]" */
    { "psm_flt_stat", wal_hipriv_psm_flt_stat },

#ifdef _PRE_WLAN_FEATURE_HIEX
    /* 设置某个用户hiex能力，sh hipriv.sh "wlan0 set_user_himit xx:xx:xx:xx:xx:xx(mac地址) 0xXXX" */
    { "set_user_hiex", _wal_hipriv_set_user_hiex_enable },
#endif
#ifdef _PRE_WLAN_FEATURE_GET_STATION_INFO_EXT
    { "get_station_info", wal_hipriv_get_station_info },
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    { "nan_dbg", wal_hipriv_nan_dbg},
#endif
};

uint32_t wal_hipriv_get_debug_cmd_size(void)
{
    return oal_array_size(g_ast_hipriv_cmd_debug);
}

#endif
