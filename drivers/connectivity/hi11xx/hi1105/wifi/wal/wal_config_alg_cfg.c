


/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "securec.h"
#include "wal_config_alg_cfg.h"
#include "hmac_vap.h"
#include "mac_vap.h"
#include "mac_frame.h"

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_DRIVER_WAL_CONFIG_ALG_CFG_C

/*****************************************************************************
  2 全局变量定义
*****************************************************************************/
alg_cfg_process_info_stru g_alg_cfg_process_info_table[] = {
    /* alg_cfg_id
                     cfg_type          need_h_process   need_w4_h   need_d_process   need_w4_d */
    /* txmode */
    ALG_CFG_INFO_TBL("txmode_mode_sw",                    MAC_ALG_CFG_TXMODE_MODE_SW,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_txmode_mode_sw",                MAC_ALG_CFG_TXMODE_MODE_SW,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("txmode_user_fix_mode",              MAC_ALG_CFG_TXMODE_USER_FIX_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_txmode_user_fix_mode",          MAC_ALG_CFG_TXMODE_USER_FIX_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("txmode_user_fix_chain",              MAC_ALG_CFG_TXMODE_USER_FIX_CHAIN,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_txmode_user_fix_chain",          MAC_ALG_CFG_TXMODE_USER_FIX_CHAIN,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("txmode_debug_log_switch",           MAC_ALG_CFG_TXMODE_DEBUG_LOG_SWITCH,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_txmode_debug_log_switch",       MAC_ALG_CFG_TXMODE_DEBUG_LOG_SWITCH,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("txmode_all_user_fix_mode",          MAC_ALG_CFG_TXMODE_ALL_USER_FIX_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_txmode_all_user_fix_mode",      MAC_ALG_CFG_TXMODE_ALL_USER_FIX_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("txmode_chain_prob_sw",          MAC_ALG_CFG_TXMODE_CHAIN_PROB_SW,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),

    /* waterfilling */
    ALG_CFG_INFO_TBL("tx_waterfilling_mode",            MAC_ALG_CFG_WATERFILLING_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_tx_waterfilling_mode",        MAC_ALG_CFG_WATERFILLING_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("waterfilling_enable",             MAC_ALG_CFG_WATERFILLING_ENABLE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_waterfilling_enable",         MAC_ALG_CFG_WATERFILLING_ENABLE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("wf_hadamard_mode",                MAC_ALG_CFG_HARDAMARDTX_MODE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_wf_hadamard_mode",            MAC_ALG_CFG_HARDAMARDTX_MODE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("hadamard_enable",                 MAC_ALG_CFG_HARDAMARDTX_ENABLE,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_hadamard_enable",             MAC_ALG_CFG_HARDAMARDTX_ENABLE,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("waterfilling_asnr",               MAC_ALG_CFG_WATERFILLING_CI_ASNR,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_waterfilling_asnr",           MAC_ALG_CFG_WATERFILLING_CI_ASNR,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("waterfilling_limit",              MAC_ALG_CFG_WATERFILLING_LIMIT_ASNR,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_waterfilling_limit",          MAC_ALG_CFG_WATERFILLING_LIMIT_ASNR,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("waterfilling_debug",              MAC_ALG_CFG_WATERFILLING_DEBUG,
                     ALG_CFG_TYPE_SET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
    ALG_CFG_INFO_TBL("get_waterfilling_debug",          MAC_ALG_CFG_WATERFILLING_DEBUG,
                     ALG_CFG_TYPE_GET, OAL_FALSE,       OAL_FALSE,   OAL_TRUE,       OAL_TRUE),
};

alg_cfg_param_tbl_stru g_alg_cfg_pkt_type_info_table[] = {
    ALG_CFG_PKT_TYPE_TBL("ucast_data",           ALG_CFG_PKT_TYPE_UCAST_DATA),
    ALG_CFG_PKT_TYPE_TBL("mcast_data",           ALG_CFG_PKT_TYPE_MCAST_DATA),
    ALG_CFG_PKT_TYPE_TBL("bcast_data",           ALG_CFG_PKT_TYPE_BCAST_DATA),
    ALG_CFG_PKT_TYPE_TBL("mumimo_data",          ALG_CFG_PKT_TYPE_MUMIMO_DATA),
    ALG_CFG_PKT_TYPE_TBL("ofdma_data",           ALG_CFG_PKT_TYPE_OFDMA_DATA),

    ALG_CFG_PKT_TYPE_TBL("ucast_mgmt",           ALG_CFG_PKT_TYPE_UCAST_MGMT),
    ALG_CFG_PKT_TYPE_TBL("mcast_mgmt",           ALG_CFG_PKT_TYPE_MCAST_MGMT),
    ALG_CFG_PKT_TYPE_TBL("bcast_mgmt",           ALG_CFG_PKT_TYPE_BCAST_MGMT),

    ALG_CFG_PKT_TYPE_TBL("prot_ctrl",            ALG_CFG_PKT_TYPE_PROT_CTRL),
    ALG_CFG_PKT_TYPE_TBL("ndp",                  ALG_CFG_PKT_TYPE_NDP),
    ALG_CFG_PKT_TYPE_TBL("ndpa",                 ALG_CFG_PKT_TYPE_NDPA),
    ALG_CFG_PKT_TYPE_TBL("resp_ctrl",            ALG_CFG_PKT_TYPE_RESP_CTRL),
    ALG_CFG_PKT_TYPE_TBL("other_ctrl",           ALG_CFG_PKT_TYPE_OTHER_CTRL),
};

alg_cfg_param_tbl_stru g_alg_cfg_bool_tbl[] = {
    {"enable",       OAL_TRUE},
    {"disable",      OAL_FALSE},
};
/* ----------------------TXMODE START-------------------------- */
/* ALG_CFG_CMD: tx_mode */
alg_cfg_param_tbl_stru g_alg_cfg_tx_mode_tbl[] = {
    {"auto", ALG_CFG_TX_MODE_AUTO},
    {"chain0_fix", ALG_CFG_TX_MODE_CHAIN0_FIX},
    {"chain1_fix", ALG_CFG_TX_MODE_CHAIN1_FIX},
    {"chain2_fix", ALG_CFG_TX_MODE_CHAIN2_FIX},
    {"chain3_fix", ALG_CFG_TX_MODE_CHAIN3_FIX},
    {"csd_fix", ALG_CFG_TX_MODE_CSD2_FIX},
    {"csd3_fix", ALG_CFG_TX_MODE_CSD3_FIX},
    {"csd4_fix", ALG_CFG_TX_MODE_CSD4_FIX},
    {"stbc_fix", ALG_CFG_TX_MODE_STBC2_FIX},
    {"stbc3_fix", ALG_CFG_TX_MODE_STBC3_FIX},
    {"stbc4_fix", ALG_CFG_TX_MODE_STBC4_FIX},
    {"txbf_fix", ALG_CFG_TX_MODE_TXBF_FIX},
    {"stbc_2nss_fix", ALG_CFG_TX_MODE_STBC_2NSS_FIX},
};

alg_cfg_param_tbl_stru g_ast_alg_cfg_bool_tbl[] = {
    {"enable",       OAL_TRUE},
    {"disable",      OAL_FALSE},
};
/* -----------------------TXMODE END--------------------------- */
/* -------    alg_cfg_param_tbl_size    ------------ */
alg_cfg_param_tbl_size_info_stru g_alg_cfg_param_size_tbl[] = {
    ALG_CFG_PARAM_SIZE_TBL(ALG_CFG_PARAM_PKT_TYPE,              oal_array_size(g_alg_cfg_pkt_type_info_table)),
    ALG_CFG_PARAM_SIZE_TBL(ALG_CFG_PARAM_BOOL,                  oal_array_size(g_alg_cfg_bool_tbl)),
    ALG_CFG_PARAM_SIZE_TBL(ALG_CFG_PARAM_TX_MODE,               oal_array_size(g_alg_cfg_tx_mode_tbl)),
};

wlan_alg_cfg_sub_alg_args_analysis_tbl_stru g_wlan_alg_cfg_sub_alg_args_analysis_table[] = {
    /* TXMODE start */
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_MODE_SW,         alg_cfg_args_analysis_txmode_mode_sw),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_USER_FIX_MODE,   alg_cfg_args_analysis_txmode_user_fix_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_USER_FIX_CHAIN,
                                      alg_cfg_args_analysis_txmode_user_fix_chain),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_DEBUG_LOG_SWITCH,
                                      alg_cfg_args_analysis_txmode_debug_log_switch),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_ALL_USER_FIX_MODE,
                                      alg_cfg_args_analysis_txmode_all_user_fix_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_TXMODE_CHAIN_PROB_SW,   alg_cfg_args_analysis_txmode_chain_prob_sw),
    /* TXMODE end */

    /* waterfilling start */
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_MODE, alg_cfg_args_analysis_waterfilling_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_ENABLE, alg_cfg_args_analysis_waterfilling_enable),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_HARDAMARDTX_MODE,  alg_cfg_args_analysis_hardamard_mode),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_HARDAMARDTX_ENABLE, alg_cfg_args_analysis_hardamard_enable),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_LIMIT_ASNR, alg_cfg_args_analysis_waterfilling_limit),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_CI_ASNR, alg_cfg_args_analysis_waterfilling_asnr),
    WLAN_CFG_ALG_SUB_ALG_ANALYSIS_TBL(MAC_ALG_CFG_WATERFILLING_DEBUG, alg_cfg_args_analysis_waterfilling_debug),
    /* waterfilling end */
};


/*****************************************************************************
  3 函数实现
*****************************************************************************/

uint8_t alg_cfg_param_get_tbl_size(alg_cfg_param_tbl_size_enum_uint8 param_type)
{
    alg_cfg_param_tbl_size_info_stru *size_info = g_alg_cfg_param_size_tbl;
    uint8_t index;

    if (param_type >= ALG_CFG_PARAM_BUTT) {
        return 0;
    }

    for (index = 0; index < oal_array_size(g_alg_cfg_param_size_tbl); index++) {
        if (size_info[index].en_param_type == param_type) {
            return size_info[index].uc_tbl_size;
        }
    }

    return 0;
}


uint32_t alg_cfg_get_cmd_one_arg(const char *cmd, char *arg, uint32_t arg_size,
    uint32_t *cmd_offset)
{
    const char *cmd_copy = NULL;
    uint32_t pos = 0;

    if (oal_likely((cmd == NULL) || (arg == NULL) || (cmd_offset == NULL))) {
        oam_error_log0(0, OAM_SF_ANY,
            "{alg_cfg_get_cmd_one_arg::pc_cmd/pc_arg/pul_cmd_offset null ptr error!}\r\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    cmd_copy = cmd;

    /* 去掉字符串开始的空格 */
    while (*cmd_copy == ' ') {
        ++cmd_copy;
    }

    while ((*cmd_copy != ' ') && (*cmd_copy != '\0')) {
        arg[pos] = *cmd_copy;
        ++pos;
        ++cmd_copy;

        if (oal_unlikely(pos >= WLAN_CFG_MAX_LEN_EACH_ARG) || oal_unlikely(pos >= arg_size)) {
            oam_warning_log1(0, OAM_SF_ANY,
                             "{alg_cfg_get_cmd_one_arg::ul_pos >= WLAN_CFG_MAX_LEN_EACH_ARG, pos %d!}\r\n", pos);
            return OAL_ERR_CODE_ARRAY_OVERFLOW;
        }
    }

    arg[pos] = '\0';

    /* 字符串到结尾，返回结束码 */
    if (pos == 0) {
        *cmd_offset = 0;
        return OAL_ERR_CODE_CONFIG_ARGS_OVER;
    }

    *cmd_offset = (uint32_t)(cmd_copy - cmd);

    return OAL_SUCC;
}


void alg_cfg_search_process_info_by_cfg_name_normal(const char *args, uint32_t arg_size,
    alg_cfg_process_info_stru **target_cfg_info)
{
    alg_cfg_process_info_stru *cfg_info = ALG_CFG_GET_PROCESS_TBL_ARRAY();
    uint16_t index;

    if (oal_unlikely(target_cfg_info == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "alg_cfg_search_process_info_by_cfg_name_normal::target_cfg_info null");
        return;
    }
    for (index = 0; index < ALG_CFG_PROCESS_TBL_SIZE(); index++) {
        if (!strncmp(args, (char *)cfg_info[index].pc_alg_cfg_name, arg_size)) {
            *target_cfg_info = &cfg_info[index];
            return;
        }
    }

    *target_cfg_info = NULL;
    return;
}


alg_cfg_process_info_stru *alg_cfg_search_process_info_by_cfg_name(const char *alg_cfg_name)
{
    char args[WLAN_CFG_MAX_LEN_EACH_ARG] = { 0 };
    uint32_t tmp_offset;
    alg_cfg_process_info_stru *target_cfg_info = NULL;

    if (alg_cfg_name == NULL) {
        return NULL;
    }

    if (alg_cfg_get_cmd_one_arg(alg_cfg_name, args, sizeof(args), &tmp_offset) != OAL_SUCC) {
        return NULL;
    }

    alg_cfg_search_process_info_by_cfg_name_normal(args, sizeof(args), &target_cfg_info);

    return target_cfg_info;
}


void alg_cfg_search_process_info_by_cfg_id_normal(mac_alg_cfg_enum_uint16 alg_cfg_id,
    alg_cfg_type_enum_uint8 cfg_type, alg_cfg_process_info_stru **target_cfg_info)
{
    alg_cfg_process_info_stru *cfg_info = ALG_CFG_GET_PROCESS_TBL_ARRAY();
    uint16_t index;

    if (oal_unlikely(target_cfg_info == NULL)) {
        oam_error_log0(0, OAM_SF_ANY, "alg_cfg_search_process_info_by_cfg_id_normal::target_cfg_info null");
        return;
    }
    for (index = 0; index < ALG_CFG_PROCESS_TBL_SIZE(); index++) {
        if ((alg_cfg_id == cfg_info[index].en_alg_cfg_id) && (cfg_type == cfg_info[index].en_cfg_type)) {
            *target_cfg_info = &cfg_info[index];
            return;
        }
    }

    *target_cfg_info = NULL;
    return;
}


alg_cfg_process_info_stru *alg_cfg_search_process_info_by_cfg_id(mac_alg_cfg_enum_uint16 alg_cfg_id,
    alg_cfg_type_enum_uint8 cfg_type)
{
    alg_cfg_process_info_stru *target_cfg_info = NULL;

    alg_cfg_search_process_info_by_cfg_id_normal(alg_cfg_id, cfg_type, &target_cfg_info);

    return target_cfg_info;
}


alg_cfg_pkt_type_enum_uint8 alg_cfg_get_pkt_type(const char *cmd_args)
{
    alg_cfg_param_tbl_stru *pkt_type_info = ALG_CFG_GET_PKT_TYPE_TBL_ARRAY();
    uint8_t index;
    char args[WLAN_CFG_MAX_LEN_EACH_ARG] = { 0 };
    uint32_t tmp_offset;

    if (cmd_args == NULL) {
        return ALG_CFG_PKT_TYPE_IRRELEVANT;
    }

    if (alg_cfg_get_cmd_one_arg(cmd_args, args, sizeof(args), &tmp_offset) != OAL_SUCC) {
        return ALG_CFG_PKT_TYPE_IRRELEVANT;
    }

    for (index = 0; index < alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_PKT_TYPE); index++) {
        if (!strncmp(args, (char *)pkt_type_info[index].pc_name, sizeof(args))) {
            return (alg_cfg_pkt_type_enum_uint8)pkt_type_info[index].value;
        }
    }

    return ALG_CFG_PKT_TYPE_IRRELEVANT;
}



uint32_t alg_cfg_analysis_args_head(char *real_args[WLAN_CFG_MAX_ARGS_NUM],
    alg_cfg_process_info_stru **real_alg_cfg_info, uint8_t *real_args_offset,
    alg_cfg_pkt_type_enum_uint8 *real_pkt_type)
{
    /* alg_cfg_analysis_args_head */
    alg_cfg_pkt_type_enum_uint8 pkt_type;
    uint8_t offset = 0;
    alg_cfg_process_info_stru *alg_cfg_info = NULL;

    if (oal_unlikely((real_alg_cfg_info == NULL) || (real_args_offset == NULL) ||
        (real_pkt_type == NULL) || (real_args == NULL))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pkt_type = alg_cfg_get_pkt_type(real_args[0]);
    if (pkt_type != ALG_CFG_PKT_TYPE_IRRELEVANT) {
        /* first arg is [pkt_type] */
        offset++;
    }

    alg_cfg_info = alg_cfg_search_process_info_by_cfg_name(real_args[offset]);
    if (alg_cfg_info == NULL) {
        return OAL_ERR_CODE_ALG_PROCESS_INFO_NOT_FOUND;
    }

    *real_alg_cfg_info = alg_cfg_info;
    *real_args_offset = ++offset;
    *real_pkt_type = pkt_type;

    return OAL_SUCC;
}



uint32_t alg_cfg_analysis_args(char *args[WLAN_CFG_MAX_ARGS_NUM], uint8_t args_num,
    alg_cfg_param_stru *alg_cfg_param, alg_cfg_hdr_stru *alg_cfg_hdr, wlan_cfg_process_info_stru *process_info)
{
    uint32_t ret;
    alg_cfg_process_info_stru *alg_cfg_info = NULL;
    uint8_t real_args_offset = 0;
    alg_cfg_pkt_type_enum_uint8 pkt_type = ALG_CFG_PKT_TYPE_IRRELEVANT;
    uint8_t index;

    if (oal_unlikely(alg_cfg_hdr == NULL)) {
        return OAL_FAIL;
    }
    ret = alg_cfg_analysis_args_head(args, &alg_cfg_info, &real_args_offset, &pkt_type);
    if (ret != OAL_SUCC) {
        return ret;
    }

    /* analysis alg_cfg_hdr */
    alg_cfg_hdr->en_cfg_id = alg_cfg_info->en_alg_cfg_id;
    alg_cfg_hdr->en_cfg_type = alg_cfg_info->en_cfg_type;
    alg_cfg_hdr->en_pkt_type = pkt_type;
    alg_cfg_hdr->dev_ret = OAL_SUCC;
    alg_cfg_hdr->en_need_w4_dev_return = alg_cfg_info->st_process_info.en_need_w4_device_return;

    /* analysis args num */
    alg_cfg_param->uc_param_num = args_num - real_args_offset;

    /* prepare each arg */
    for (index = 0; index < alg_cfg_param->uc_param_num; index++) {
        alg_cfg_param->apuc_param_args[index] = (uint8_t *)args[index + real_args_offset];
    }

    /* update process info */
    *process_info = alg_cfg_info->st_process_info;

    return OAL_SUCC;
}



uint32_t alg_cfg_get_match_value(const alg_cfg_param_tbl_stru *tbl, uint16_t tbl_size,
    const uint8_t *str, uint32_t *out_value)
{
    uint16_t index;

    if (tbl == NULL || str == NULL || out_value == NULL || tbl_size == 0) {
        return OAL_FAIL;
    }

    for (index = 0; index < tbl_size; index++) {
        if (!strncmp((char *)str, (char *)tbl[index].pc_name, OAL_STRLEN((const char *)tbl[index].pc_name) + 1)) {
            *out_value = tbl[index].value;
            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}



uint32_t alg_cfg_get_match_string(const alg_cfg_param_tbl_stru *tbl, uint16_t tbl_size,
                                  uint32_t value, char *out_str, uint32_t out_str_size)
{
    uint16_t index;
    int32_t ret;

    if (tbl == NULL || out_str == NULL || tbl_size == 0) {
        return OAL_FAIL;
    }

    for (index = 0; index < tbl_size; index++) {
        if (value == tbl[index].value) {
            ret = memcpy_s(out_str, OAL_STRLEN(tbl[index].pc_name) + 1, tbl[index].pc_name,
                           OAL_STRLEN(tbl[index].pc_name) + 1);
            if (ret != EOK) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_get_match_string: memcpy_s return ERR!! l_ret = %d}", ret);
                return ret;
            }

            return OAL_SUCC;
        }
    }

    return OAL_FAIL;
}


uint32_t alg_cfg_get_tail_num(const uint8_t *str, uint32_t *out_num)
{
    const uint8_t *copy = NULL;
    uint8_t num_str[11] = {0};
    uint8_t pos = 0;

    if (str == NULL || out_num == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    copy = str;

    while ((*copy != '\0') && ((*copy < '0') || (*copy > '9'))) {
        copy++;
    }

    while (*copy != '\0') {
        /* 数字间掺杂非数字字符 */
        if (*copy < '0' || *copy > '9' || pos >= 10) {
            return OAL_FAIL;
        }

        num_str[pos++] = *copy++;
    }

    *out_num = (uint32_t)oal_atoi((char *)num_str);

    return OAL_SUCC;
}


wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *wal_wlan_get_sub_alg_args_analysis_entry(
    mac_alg_cfg_enum_uint16 cfg_id)
{
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *sub_alg_args_analysis_entry = NULL;

    sub_alg_args_analysis_entry = wal_get_wlan_alg_cfg_analysis_entry(g_wlan_alg_cfg_sub_alg_args_analysis_table,
        oal_array_size(g_wlan_alg_cfg_sub_alg_args_analysis_table), cfg_id);

    return sub_alg_args_analysis_entry;
}


int32_t wal_wlan_cfg_alg_process_entry(wlan_cfg_param_stru *wlan_cfg_param,
    uint8_t *msg_type, uint8_t *mem_block, uint16_t *mem_len,
    wlan_cfg_process_info_stru *process_info)
{
    uint32_t ret;
    alg_cfg_param_stru alg_cfg_param = { 0 };
    alg_cfg_hdr_stru alg_cfg_hdr = { 0 };

    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *sub_alg_args_analysis = NULL;

    if (wlan_cfg_param == NULL || msg_type == NULL || mem_block == NULL ||
        mem_len == NULL || process_info == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    ret = alg_cfg_analysis_args(wlan_cfg_param->apc_args, wlan_cfg_param->uc_args_num, &alg_cfg_param,
        &alg_cfg_hdr, process_info);
    if (ret != OAL_SUCC) {
        return ret;
    }

    if (oal_unlikely(wlan_cfg_param->pst_net_dev == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    alg_cfg_param.pst_mac_vap = oal_net_dev_priv(wlan_cfg_param->pst_net_dev);
    alg_cfg_param.puc_output_str = (uint8_t *)wlan_cfg_param->pc_output;

    /* Analysis input args and copy mem_block used to transfer */
    sub_alg_args_analysis = wal_wlan_get_sub_alg_args_analysis_entry(alg_cfg_hdr.en_cfg_id);
    if (sub_alg_args_analysis == NULL) {
        oam_error_log0(0, OAM_SF_CFG,
            "{wal_wlan_cfgid_alg_param_process_entry:wal_get_wlan_alg_cfg_analysis_entry entry not found!.}\r\n");
        return OAL_ERR_CODE_SUB_ALG_ANALYSIS_ENTRY_NOT_FOUND;
    }
    if (sub_alg_args_analysis->p_sub_alg_args_analysis_func == NULL) {
        oam_error_log0(0, OAM_SF_CFG,
            "{wal_wlan_cfgid_alg_param_process_entry:wal_get_wlan_alg_cfg_analysis_entry p_sub_alg_args_analysis_func "
            "not register!.}\r\n");
        return OAL_ERR_CODE_SUB_ALG_ANALYSIS_ENTRY_NOT_FOUND;
    }

    /* package alg_cfg_hdr */
    *(alg_cfg_hdr_stru *)mem_block = alg_cfg_hdr;

    /* package spec_alg mem_block */
    ret = sub_alg_args_analysis->p_sub_alg_args_analysis_func(&alg_cfg_param, mem_block, mem_len);
    if (ret != OAL_SUCC) {
        return OAL_ERR_CODE_SUB_ALG_ARGS_INVALID;
    }

    /* cross thread process */
    *msg_type = (alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_GET) ? WAL_MSG_TYPE_QUERY : WAL_MSG_TYPE_WRITE;

    return ret;
};

wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *wal_get_wlan_alg_cfg_analysis_entry(
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *process_info_tbl, uint32_t tbl_size,
    mac_alg_cfg_enum_uint16 cfg_id)
{
    uint32_t index;
    wlan_alg_cfg_sub_alg_args_analysis_tbl_stru *current_entry = NULL;

    for (index = 0; index < tbl_size; index++) {
        current_entry = process_info_tbl + index;
        if (current_entry->en_alg_cfg_id == cfg_id) {
            return current_entry;
        }
    }

    return NULL;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_mode_set(
    alg_cfg_param_stru *cfg_param, alg_cfg_param_txmode_user_stru *txomde_param)
{
    uint32_t result;
    int32_t ret;
    uint32_t value;

    if (cfg_param->uc_param_num != 2) { // num2
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_mode: config txmode_user_fix_mode, uc_param_num(%u) should be 2!}",
            cfg_param->uc_param_num);
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_mode, uc_param_num(%u) should be 2!\n", cfg_param->uc_param_num);
        if (ret < 0) {
            oam_error_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode::sprintf_s fail}");
            return OAL_FAIL;
        }

        return OAL_FAIL;
    }
    /* v1 */
    result = alg_cfg_get_match_value(g_alg_cfg_tx_mode_tbl, oal_array_size(g_alg_cfg_tx_mode_tbl),
        cfg_param->apuc_param_args[1], &value);
    if (result != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_mode: config txmode_user_fix_mode, <value> invalid, should be "
            "{auto, chain0_fix, chain1_fix, csd_fix, stbc_fix, txbf_fix}!}");
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_mode, <value> = %s invalid, should be {auto, chain0_fix, chain1_fix, "
            "csd_fix, stbc_fix, txbf_fix}!\n", cfg_param->apuc_param_args[1]);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode::sprintf_s fail}");
            return OAL_FAIL;
        }

        return result;
    }
    txomde_param->en_fix_txmode = (alg_cfg_param_tx_mode_enum)value;
    return OAL_SUCC;
}


OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_mode(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_txmode_user_stru *txomde_param)
{
    uint8_t mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    int32_t ret;

    if (cfg_param->uc_param_num >= 1) {
        /* v0 - mac */
        if (OAL_STRLEN(cfg_param->apuc_param_args[0]) != 17) { // num17
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_fix_mode_sw: config txmode_user_fix_mode mac length(%u), "
                "should be 17!}", OAL_STRLEN(cfg_param->apuc_param_args[0]));
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                            "[Error]config txmode_user_fix_mode mac length(%u), should be 17!\n",
                            (unsigned int)OAL_STRLEN(cfg_param->apuc_param_args[0]));
            if (ret < 0) {
                oam_error_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_fix_mode_sw::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
        oal_strtoaddr((char *)cfg_param->apuc_param_args[0],
            OAL_STRLEN((const char *)cfg_param->apuc_param_args[0]) + 1, mac_addr, WLAN_MAC_ADDR_LEN);
        ret = memcpy_s(txomde_param->auc_mac_addr, WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN);
        if (ret != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_fix_mode_sw::memcpy_s fail}");
            return OAL_FAIL;
        }
    }

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (txomde_param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (alg_cfg_param_analyze_txmode_user_fix_mode_set(cfg_param, txomde_param) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode_set:: fail}");
            return OAL_FAIL;
        }
    } else {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_user_fix_mode: config get_txmode_user_fix_mode, uc_param_num(%u) "
                "should be 1!}", cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config get_txmode_user_fix_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}


uint32_t alg_cfg_args_analysis_txmode_user_fix_mode(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_user_fix_mode(alg_cfg_param, (alg_cfg_param_txmode_user_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_txmode_user_fix_mode: alg_cfg_param_analyze_txmode_user_fix_mode, return %u!}",
            ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_txmode_user_stru);

    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_chain_set(
    alg_cfg_param_stru *cfg_param, alg_cfg_param_txmode_fix_chain_stru *param)
{
    uint8_t value;
    int32_t  ret;

    if (cfg_param->uc_param_num != 2) { // num2
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_chain: config txmode_user_fix_chain, uc_param_num(%u) should be 2}",
            cfg_param->uc_param_num);
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_chain, uc_param_num(%u) should be 2!\n", cfg_param->uc_param_num);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
            return OAL_FAIL;
        }

        return OAL_FAIL;
    }
    /* v1 */
    value = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[1]);
    if (value > ALG_CFG_PARAM_TXMODE_MAX_CHAIN_BITMAP) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_user_fix_chain: config txmode_user_fix_chain, <%d> invalid, exceed "
            "[0, 15]!}", value);
        ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "[Error]config txmode_user_fix_chain, <value> = %u invalid, exceed [0, 15]!\n",
            value);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
            return OAL_FAIL;
        }
        return OAL_FAIL;
    }
    param->fix_chain = value;
    return OAL_SUCC;
}


OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_user_fix_chain(alg_cfg_param_stru *cfg_param,
                                                                alg_cfg_param_txmode_fix_chain_stru *param)
{
    uint8_t mac_addr[WLAN_MAC_ADDR_LEN] = {0};
    int32_t ret;

    if (cfg_param->uc_param_num >= 1) {
        /* v0 - mac */
        if (OAL_STRLEN(cfg_param->apuc_param_args[0]) != 17) { // num17
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_user_fix_chain: config txmode_user_fix_chain mac length(%u), "
                "should be 17!}", OAL_STRLEN(cfg_param->apuc_param_args[0]));
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                            "[Error]config txmode_user_fix_chain mac length(%u), should be 17!\n",
                            (unsigned int)OAL_STRLEN(cfg_param->apuc_param_args[0]));
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
        oal_strtoaddr((char *)cfg_param->apuc_param_args[0],
            OAL_STRLEN((const char *)cfg_param->apuc_param_args[0]) + 1, mac_addr, WLAN_MAC_ADDR_LEN);
        ret = memcpy_s(param->mac_addr, WLAN_MAC_ADDR_LEN, mac_addr, WLAN_MAC_ADDR_LEN);
        if (ret != EOK) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::memcpy_s fail}");
            return OAL_FAIL;
        }
    }

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (alg_cfg_param_analyze_txmode_user_fix_chain_set(cfg_param, param) != OAL_SUCC) {
            oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain_set:: fail}");
            return OAL_FAIL;
        }
    } else {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_user_fix_chain: config get_txmode_user_fix_chain, uc_param_num(%u) "
                "should be 1!}", cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config get_txmode_user_fix_mode, uc_param_num(%u) should be 1!\n",
                cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_user_fix_chain::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}


uint32_t alg_cfg_args_analysis_txmode_user_fix_chain(alg_cfg_param_stru *alg_cfg_param,
                                                     uint8_t *puc_mem_block, uint16_t *pus_len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_user_fix_chain(alg_cfg_param,
                                                      (alg_cfg_param_txmode_fix_chain_stru *)puc_mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_txmode_user_fix_chain: alg_cfg_param_analyze_txmode_user_fix_chain, return %u!}",
            ret);
        return ret;
    }

    *pus_len = sizeof(alg_cfg_param_txmode_fix_chain_stru);

    return OAL_SUCC;
}


uint32_t alg_cfg_param_analyze_txmode_debug_log_switch(alg_cfg_param_stru *cfg_param,
                                                       alg_cfg_param_txmode_log_switch_stru *param_switch)
{
    uint32_t sw_on = 0;
    uint32_t ret1;
    int32_t  ret;

    /* Validate param cnt */
    if (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET && cfg_param->uc_param_num != 1) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_param_analyze_txmode_debug_log_switch: config txmode_debug_log_switch, uc_param_num(%u)\
            should be 1!}", cfg_param->uc_param_num);
        ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]config txmode_debug_log_switch, uc_param_num(%u) should be 1!\n",
            cfg_param->uc_param_num);
        if (ret < 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch::invalid uc_param_num,\
                snprintf_s return %d!}", ret);
        }
        return OAL_FAIL;
    }

    if (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_GET && cfg_param->uc_param_num != 0) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch: get txmode_debug_log_switch,\
                       uc_param_num(%u) should be 0!}", cfg_param->uc_param_num);
        ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                         WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]get txmode_debug_log_switch, uc_param_num(%u) \
                should be 0!\n", cfg_param->uc_param_num);
        if (ret < 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch:: snprintf_s(get) \
                return %d!}", ret);
        }
        return OAL_FAIL;
    }

    /* Analysis param */
    if (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        /* param0: probe_switch */
        ret1 = alg_cfg_get_match_value(g_ast_alg_cfg_bool_tbl, alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_BOOL),
                                       cfg_param->apuc_param_args[0], &sw_on);
        if (ret1 != OAL_SUCC) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch:config \
                <txmode_debug_log_switch> to be %u, exceed [0, 1]!}", sw_on);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]config <debug_log_swtich> to be %u, exceed [0, 1]!\n",
                sw_on);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_debug_log_switch:: \
                    alg_cfg_get_match_value fail, snprintf_s return %d!}", ret);
            }
            return OAL_FAIL;
        }
    }

    param_switch->debug_log_switch =
        (param_switch->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) ? (oal_bool_enum_uint8)sw_on : 0;
    return OAL_SUCC;
}


uint32_t alg_cfg_args_analysis_txmode_debug_log_switch(alg_cfg_param_stru *alg_cfg_param,
                                                       uint8_t *puc_mem_block, uint16_t *pus_len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_debug_log_switch(alg_cfg_param,
                                                        (alg_cfg_param_txmode_log_switch_stru *)puc_mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log0(0, OAM_SF_ANY, "{alg_cfg_args_analysis_txmode_debug_log_switch::param analysis fail!}");
        return ret;
    }

    *pus_len = sizeof(alg_cfg_param_txmode_log_switch_stru);

    return OAL_SUCC;
}


OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_all_user_fix_mode(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_txmode_all_user_stru *txomde_param)
{
    uint32_t ret, get_value;
    int32_t ret_signed;

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (txomde_param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        /* 检查参数个数 */
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode: "
                "config txmode_all_user_fix_mode, uc_param_num(%u) should be 1!}", cfg_param->uc_param_num);
            ret_signed = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]txmode_all_user_fix_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret_signed < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }

        /* v0 */
        ret = alg_cfg_get_match_value(g_alg_cfg_tx_mode_tbl, alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_TX_MODE),
            cfg_param->apuc_param_args[0], &get_value);
        if (ret != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_all_user_fix_mode: config txmode_all_user_fix_mode, "
                "<value> invalid, should be {auto, chain0_fix, chain1_fix, csd_fix, stbc_fix, txbf_fix}!}");
            ret_signed = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config txmode_all_user_fix_mode, <value> = %s invalid, should be "
                "{auto, chain0_fix, chain1_fix, csd_fix, stbc_fix, txbf_fix}!\n",
                cfg_param->apuc_param_args[0]);
            if (ret_signed < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return ret;
        }
        txomde_param->en_fix_txmode = (alg_cfg_param_tx_mode_enum)get_value;
    } else {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode: \
                config get_txmode_all_user_fix_mode, uc_param_num(%u) should be 1!}", cfg_param->uc_param_num);
            ret_signed = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config get_txmode_all_user_fix_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret_signed < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_all_user_fix_mode::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_txmode_all_user_fix_mode(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_all_user_fix_mode(alg_cfg_param,
        (alg_cfg_param_txmode_all_user_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_args_analysis_txmode_all_user_fix_mode: \
                       alg_cfg_param_analyze_txmode_all_user_fix_mode, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_txmode_all_user_stru);

    return OAL_SUCC;
}


OAL_STATIC uint32_t alg_cfg_param_analyze_txmode_mode_sw(alg_cfg_param_stru *cfg_param,
    alg_cfg_param_txmode_dev_stru *txomde_param)
{
    uint32_t result, value;
    int32_t ret;

    /* 若参数配置类型为set，则对参数的取值进行匹配，并将取值拷贝出来 */
    if (txomde_param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_mode_sw: config txmode_mode_sw, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config txmode_mode_sw, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_mode_sw::sprintf_s fail}");
                return OAL_FAIL;
            }

            return OAL_FAIL;
        }

        /* v0 */
        result = alg_cfg_get_match_value(g_alg_cfg_bool_tbl, alg_cfg_param_get_tbl_size(ALG_CFG_PARAM_BOOL),
            cfg_param->apuc_param_args[0], &value);
        if (result != OAL_SUCC) {
            oam_error_log0(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_txmode_mode_sw: config txmode_mode_sw, <value> invalid, should be {enable, "
                "disable}!}");
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config txmode_mode_sw, <value> = %s invalid, should be {enable, disable}!\n",
                cfg_param->apuc_param_args[0]);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_txmode_mode_sw::sprintf_s fail}");
                return OAL_FAIL;
            }

            return result;
        }
        txomde_param->en_txmode_sw = (oal_bool_enum_uint8)value;
    }

    return OAL_SUCC;
}


uint32_t alg_cfg_args_analysis_txmode_mode_sw(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *pmem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_txmode_mode_sw(alg_cfg_param, (alg_cfg_param_txmode_dev_stru *)pmem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_txmode_mode_sw: alg_cfg_param_analyze_txmode_mode_sw, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_txmode_dev_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_txmode_chain_prob_sw(alg_cfg_param_stru *alg_cfg_param,
    uint8_t *pmem_block, uint16_t *len)
{
    alg_cfg_param_txmode_chain_prob_stru *para = (alg_cfg_param_txmode_chain_prob_stru *)pmem_block;
    int32_t ret;

    if (alg_cfg_param->uc_param_num != 1) {
        ret = sprintf_s((char *)alg_cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "txmode_chain_prob_sw: para num(%d) error\n", alg_cfg_param->uc_param_num);
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{alg_cfg_args_analysis_txmode_chain_prob_sw:: invalid para num, sprintf_s fail!}");
        }
        return OAL_FAIL;
    }

    para->chain_sw = (uint8_t)oal_atoi((char *)alg_cfg_param->apuc_param_args[0]);
    if (para->chain_sw > 1) {
        ret = sprintf_s((char *)alg_cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
            "txmode_chain_prob_sw: invalid input\n");
        if (ret < 0) {
            oam_warning_log0(0, OAM_SF_ANY,
                "{alg_cfg_args_analysis_txmode_chain_prob_sw:: invalid input, sprintf_s fail}");
        }
        return OAL_FAIL;
    }

    *len = sizeof(alg_cfg_param_txmode_chain_prob_stru);
    return OAL_SUCC;
}

/* waterfilling */
OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_debug(alg_cfg_param_stru *cfg_param,
                                                             alg_cfg_param_waterfilling_debug_stru *param)
{
    int32_t  ret;
    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_debug: config wf_dbg, param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_dbg, param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_dbg::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_debug = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_debug != ALG_CFG_TX_WATERFILLING_ENABLE &&
            param->waterfilling_debug != ALG_CFG_TX_WATERFILLING_DISABLE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_wf_dbg, param_mode(%u) invalid, disable:0 ; enable:1 ;\n",
                param->waterfilling_debug);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_dbg: snprintf_s(set) return %d, wf_dbg = %d}", ret,
                               param->waterfilling_debug);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_dbg: wf_dbg param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_dbg, param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_dbg: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_mode(alg_cfg_param_stru *cfg_param,
                                                            alg_cfg_param_waterfilling_mode_stru *param)
{
    int32_t  ret;
    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_mode: config waterfilling_mode, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_mode = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_mode != ALG_CFG_TX_WATERFILLING_AUTO_MODE &&
            param->waterfilling_mode != ALG_CFG_TX_WATERFILLING_FIX_MODE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_wf_mode, uc_param_mode(%u) invalid, fix mode:0 ; auto mode:1 ;\n",
                param->waterfilling_mode);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_mode: snprintf_s(set) return %d, wf_mode = %d}", ret,
                               param->waterfilling_mode);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_mode: wf_mode uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_mode, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_mode: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_enable(alg_cfg_param_stru *cfg_param,
                                                              alg_cfg_param_waterfilling_enable_stru *param)
{
    int32_t  ret;
    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_waterfilling_enable: config waterfilling_enable, param_num(%u) should be 1}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_enable, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_enable::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_enable = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_enable != ALG_CFG_TX_WATERFILLING_ENABLE &&
            param->waterfilling_enable != ALG_CFG_TX_WATERFILLING_DISABLE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_wf_enable, uc_param_enable(%u) invalid, fix mode:0 ; auto mode:1 ;\n",
                param->waterfilling_enable);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_enable: snprintf_s(set) return %d, wf_mode = %d}", ret,
                               param->waterfilling_enable);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_enable: wf_mode uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_mode, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_enable: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_hardamard_mode(alg_cfg_param_stru *cfg_param,
                                                         alg_cfg_param_hardamard_mode_stru *param)
{
    int32_t  ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_hdtx_mode: config hdtx_mode, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config hdtx_mode, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_hdtx_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->hardamard_mode = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->hardamard_mode != ALG_CFG_TX_HARDMARD_AUTO_MODE &&
            param->hardamard_mode != ALG_CFG_TX_HARDMARD_FIX_MODE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]analyse_hdtx_mode, uc_param_mode(%u) invalid, fix mode:0 ; auto mode:1 ;\n",
                param->hardamard_mode);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_hdtx_mode: snprintf_s(set) return %d, hdtx_mode = %d}", ret,
                               param->hardamard_mode);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_hdtx_mode: hdtx_mode uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_hdtx_mode, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_hdtx_mode: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}
OAL_STATIC uint32_t alg_cfg_param_analyze_hardamard_enable(alg_cfg_param_stru *cfg_param,
                                                           alg_cfg_param_hardamard_enable_stru *param)
{
    int32_t  ret;

    if (param->alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_hardamard_enable: config hardamard_enable, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config hardamard_enable, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_mode::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->hardamard_enable = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->hardamard_enable != ALG_CFG_TX_WATERFILLING_ENABLE &&
            param->hardamard_enable != ALG_CFG_TX_WATERFILLING_DISABLE) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]hardamard_enable, uc_param_enable(%u) invalid, eable : 1 disable: 0;\n",
                param->hardamard_enable);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                    "{alg_cfg_param_analysis_hardamard_enable: snprintf_s(set) return %d, hdtx_enable = %d}", ret,
                               param->hardamard_enable);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analysis_hardamard_enable: hdtx_mode uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_hdtx_enable, param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_hdtx_enable: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_limit(alg_cfg_param_stru *cfg_param,
                                                             alg_cfg_param_waterfilling_limit_stru *param)
{
    int32_t  ret;

    if (param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num != 1) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_wf_limit: config wf_limit, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_limit, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_limit::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        param->waterfilling_limit = (int8_t)oal_atoi((char *)cfg_param->apuc_param_args[0]);
        /* 如果配置参数超过范围直接返回失败 */
        if (param->waterfilling_limit > ALG_CFG_TX_WATERFILLING_MAX_LIMIT) {
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                WLAN_CFG_HOST_MAX_RSP_LEN - 1,
                "[Error]wf_limit, uc_param_enable(%u) invalid, limit < -128 || limit > 127;\n",
                param->waterfilling_limit);
            if (ret < 0) {
                oam_error_log2(0, OAM_SF_ANY,
                               "{alg_cfg_param_analysis_wf_limit: snprintf_s(set) return %d, wf_limit = %d}", ret,
                               param->waterfilling_limit);
            }
            return OAL_FAIL;
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_limit: wf_limit uc_param_num(%u) should be 1!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_limit, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_limit: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

OAL_STATIC uint32_t alg_cfg_param_analyze_waterfilling_asnr(alg_cfg_param_stru *cfg_param,
                                                            alg_cfg_param_waterfilling_asnr_stru *param)
{
    uint8_t  idx;
    int32_t  ret;
    if (param->st_alg_cfg_hdr.en_cfg_type == ALG_CFG_TYPE_SET) {
        if (cfg_param->uc_param_num > ALG_CFG_TX_WATERFILLING_SNR_NUM) {
            oam_error_log1(0, OAM_SF_ANY,
                "{alg_cfg_param_analyze_wf_asnr: config wf_limit, uc_param_num(%u) should be 1!}",
                cfg_param->uc_param_num);
            ret = sprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                "[Error]config wf_lasnr, uc_param_num(%u) should be 1!\n", cfg_param->uc_param_num);
            if (ret < 0) {
                oam_warning_log0(0, OAM_SF_ANY, "{alg_cfg_param_analyze_wf_asnr::sprintf_s fail}");
            }
            return OAL_FAIL;
        }
        for (idx = 0; idx < cfg_param->uc_param_num; ++idx) {
            param->waterfilling_asnr[idx] = (uint8_t)oal_atoi((char *)cfg_param->apuc_param_args[idx]);
        }
    } else { /* GET VALUE */
        if (cfg_param->uc_param_num != 0) {
            oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_asnr: wf_asnr uc_param_num(%u) should be 2!}",
                           cfg_param->uc_param_num);
            ret = snprintf_s((char *)cfg_param->puc_output_str, WLAN_CFG_HOST_MAX_RSP_LEN,
                             WLAN_CFG_HOST_MAX_RSP_LEN - 1, "[Error]analyse_wf_asnr, uc_param_num(%u) should be 1!\n",
                             cfg_param->uc_param_num);
            if (ret < 0) {
                oam_error_log1(0, OAM_SF_ANY, "{alg_cfg_param_analysis_wf_asnr: snprintf_s(get) return %d.}", ret);
            }
            return OAL_FAIL;
        }
    }
    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_debug(alg_cfg_param_stru *alg_cfg_param, uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_debug(alg_cfg_param,
        (alg_cfg_param_waterfilling_debug_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_debug: alg_cfg_args_analysis_waterfilling_debug, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_debug_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_mode(alg_cfg_param_stru *pst_alg_cfg_param,
                                                 uint8_t *puc_mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_mode(pst_alg_cfg_param,
        (alg_cfg_param_waterfilling_mode_stru *)puc_mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_mode: alg_cfg_args_analysis_waterfilling_mode, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_mode_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_enable(alg_cfg_param_stru *alg_cfg_param,
                                                   uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_enable(alg_cfg_param,
        (alg_cfg_param_waterfilling_enable_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_enable: alg_cfg_args_analysis_waterfilling_enable, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_enable_stru);

    return OAL_SUCC;
}
uint32_t alg_cfg_args_analysis_hardamard_mode(alg_cfg_param_stru *alg_cfg_param,
                                              uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_hardamard_mode(alg_cfg_param,
        (alg_cfg_param_hardamard_mode_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_hardamard_mode: alg_cfg_args_analysis_hardamard_mode, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_hardamard_mode_stru);

    return OAL_SUCC;
}
uint32_t alg_cfg_args_analysis_hardamard_enable(alg_cfg_param_stru *alg_cfg_param,
                                                uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_hardamard_enable(alg_cfg_param,
        (alg_cfg_param_hardamard_enable_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_hardamard_enable: alg_cfg_args_analysis_hardamard_enable, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_hardamard_enable_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_limit(alg_cfg_param_stru *alg_cfg_param,
                                                  uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_limit(alg_cfg_param,
        (alg_cfg_param_waterfilling_limit_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_limit: alg_cfg_args_analysis_waterfilling_limit, return %u!}",
            ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_limit_stru);

    return OAL_SUCC;
}

uint32_t alg_cfg_args_analysis_waterfilling_asnr(alg_cfg_param_stru *alg_cfg_param,
                                                 uint8_t *mem_block, uint16_t *len)
{
    uint32_t ret;

    ret = alg_cfg_param_analyze_waterfilling_asnr(alg_cfg_param,
        (alg_cfg_param_waterfilling_asnr_stru *)mem_block);
    if (ret != OAL_SUCC) {
        oam_error_log1(0, OAM_SF_ANY,
            "{alg_cfg_args_analysis_waterfilling_asnr: alg_cfg_args_analysis_waterfilling_asnr, return %u!}", ret);
        return ret;
    }

    *len = sizeof(alg_cfg_param_waterfilling_asnr_stru);

    return OAL_SUCC;
}
