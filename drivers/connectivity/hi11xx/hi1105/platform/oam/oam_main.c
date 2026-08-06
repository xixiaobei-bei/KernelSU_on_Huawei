

/* 头文件包含 */
#include "oam_main.h"
#include "oam_log.h"
#include "oam_event.h"
#include "oam_statistics.h"
#include "oam_config.h"
#include "oam_linux_netlink.h"
#include "oam_ext_if.h"
#include "oal_hcc_host_if.h"
#include "plat_pm_wlan.h"
#include "securec.h"

/* OAM模块统一使用的全局操作变量上下文，包括OAM其他子模块全局上下文 */
oam_mng_ctx_stru g_oam_mng_ctx;

/* 打印类型函数定义 */
OAL_STATIC oal_print_func g_oam_print_type_func[OAM_OUTPUT_TYPE_BUTT] = {
    oam_print_to_console, /* OAM_OUTPUT_TYPE_CONSOLE 控制台输出 */
    oam_print_to_sdt,     /* OAM_OUTPUT_TYPE_SDT 输出到SDT,上报字符串不宜大于2048 */
};

/* 用于和SDT工具交互的全局变量 */
oam_sdt_func_hook_stru g_oam_sdt_func_hook;
oam_wal_func_hook_stru g_oam_wal_func_hook;
oam_sdt_stat_info_stru g_sdt_stat_info;

uint8_t g_bcast_addr[WLAN_MAC_ADDR_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/* 特性列表 */
oam_software_feature_stru g_oam_feature_list[OAM_SOFTWARE_FEATURE_BUTT] = {
    /* 特性宏ID                  特性名缩写 */
    /* 0 */
    { OAM_SF_SCAN,         "scan" },
    { OAM_SF_AUTH,         "auth" },
    { OAM_SF_ASSOC,        "assoc" },
    { OAM_SF_FRAME_FILTER, "ff" },
    { OAM_SF_WMM,          "wmm" },

    /* 5 */
    { OAM_SF_DFS,              "dfs" },
    { OAM_SF_NETWORK_MEASURE,  "nm" },
    { OAM_SF_ENTERPRISE_VO,    "ev" },
    { OAM_SF_HOTSPOTROAM,      "roam" },
    { OAM_SF_NETWROK_ANNOUNCE, "11u" },

    /* 10 */
    { OAM_SF_NETWORK_MGMT, "11k" },
    { OAM_SF_NETWORK_PWS,  "pws" },
    { OAM_SF_PROXYARP,     "proxyarp" },
    { OAM_SF_TDLS,         "tdls" },
    { OAM_SF_CALIBRATE,    "cali" },

    /* 15 */
    { OAM_SF_EQUIP_TEST, "equip" },
    { OAM_SF_CRYPTO,     "crypto" },
    { OAM_SF_WPA,        "wpa" },
    { OAM_SF_WEP,        "wep" },
    { OAM_SF_WPS,        "wps" },

    /* 20 */
    { OAM_SF_PMF,   "pmf" },
    { OAM_SF_WAPI,  "wapi" },
    { OAM_SF_BA,    "ba" },
    { OAM_SF_AMPDU, "ampdu" },
    { OAM_SF_AMSDU, "amsdu" },

    /* 25 */
    { OAM_SF_STABILITY, "dfr" },
    { OAM_SF_TCP_OPT,   "tcp" },
    { OAM_SF_ACS,       "acs" },
    { OAM_SF_AUTORATE,  "autorate" },
    { OAM_SF_TXBF,      "txbf" },

    /* 30 */
    { OAM_SF_DYN_RECV,      "weak" },
    { OAM_SF_VIVO,          "vivo" },
    { OAM_SF_MULTI_USER,    "muser" },
    { OAM_SF_MULTI_TRAFFIC, "mtraff" },
    { OAM_SF_ANTI_INTF,     "anti_intf" },

    /* 35 */
    { OAM_SF_EDCA,          "edca" },
    { OAM_SF_SMART_ANTENNA, "ani" },
    { OAM_SF_TPC,           "tpc" },
    { OAM_SF_TX_CHAIN,      "txchain" },
    { OAM_SF_RSSI,          "rssi" },

    /* 40 */
    { OAM_SF_WOW,      "wow" },
    { OAM_SF_GREEN_AP, "green" },
    { OAM_SF_PWR,      "pwr" },
    { OAM_SF_SMPS,     "smps" },
    { OAM_SF_TXOP,     "txop" },

    /* 45 */
    { OAM_SF_WIFI_BEACON, "beacon" },
    { OAM_SF_KA_AP,       "alive" },
    { OAM_SF_MULTI_VAP,   "mvap" },
    { OAM_SF_2040,        "2040" },
    { OAM_SF_DBAC,        "dbac" },

    /* 50 */
    { OAM_SF_PROXYSTA, "proxysta" },
    { OAM_SF_UM,       "um" },
    { OAM_SF_P2P,      "p2p" },
    { OAM_SF_M2U,      "m2u" },
    { OAM_SF_IRQ,      "irq" },

    /* 55 */
    { OAM_SF_TX,       "tx" },
    { OAM_SF_RX,       "rx" },
    { OAM_SF_DUG_COEX, "dugcoex" },
    { OAM_SF_CFG,      "cfg" },
    { OAM_SF_FRW,      "frw" },

    /* 60 */
    { OAM_SF_KEEPALIVE, "keepalive" },
    { OAM_SF_COEX,      "coex" },
    { OAM_SF_HS20,      "hs20" },
    { OAM_SF_MWO_DET,   "mwodet" },
    { OAM_SF_CCA_OPT,   "cca" },

    /* 65 */
    { OAM_SF_ROAM,   "roam" },
    { OAM_SF_DFT,    "dft" },
    { OAM_SF_DFR,    "dfr" },
    { OAM_SF_RRM,    "rrm" },
    { OAM_SF_VOWIFI, "vowifi" },

    /* 70 */
    { OAM_SF_OPMODE, "opmode" },
    { OAM_SF_M2S,    "m2s" },
    { OAM_SF_DBDC,   "dbdc" },
    { OAM_SF_HILINK, "hilink" },
    { OAM_SF_WDS,    "wds" },

    /* 75 */
    { OAM_SF_WMMAC,       "wmmac" },
    { OAM_SF_USER_EXTEND, "ue" },
    { OAM_SF_PKT_CAP,     "pktcap" },
    { OAM_SF_SOFT_CRYPTO, "crypto" },
    { OAM_SF_CAR,         "car" }, /* 限速特性  */
    { OAM_SF_11AX,        "11ax" },
    { OAM_SF_CSA,         "csa" },
    { OAM_SF_QOS,         "qos" },
    { OAM_SF_INI,         "ini" },
#ifdef _PRE_WLAN_FEATURE_HIEX
    { OAM_SF_HIEX,        "hiex" },
#else
    { OAM_SF_RESERVE5,    "rev5" },
#endif
    { OAM_SF_RESERVE6,    "rev6" },
    { OAM_SF_RESERVE7,    "rev7" },
    { OAM_SF_RESERVE8,    "rev8" },
    { OAM_SF_RESERVE9,    "rev9" },
    { OAM_SF_RESERVE10,   "rev10" },
    { OAM_SF_CONN,        "conn" },
    { OAM_SF_CHAN,        "chnn" },
    { OAM_SF_CUSTOM,      "custom" },

    { OAM_SF_ANY, "any" },
};

/*
 * 函 数 名  : oam_print
 * 功能描述  : OAM模块提供的总体入口
 */
uint32_t oam_print(const char *pc_string)
{
    oam_output_type_enum_uint8 en_output_type;
    uint32_t ul_rslt;

    if (oal_unlikely(pc_string == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }

    ul_rslt = oam_get_output_type(&en_output_type);
    if (ul_rslt != OAL_SUCC) {
        return ul_rslt;
    }

    ul_rslt = g_oam_print_type_func[en_output_type](pc_string);
    if (ul_rslt != OAL_SUCC) {
        return ul_rslt;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_print_to_console
 * 功能描述  : 打印信息到标准输出窗口中
 */
uint32_t oam_print_to_console(const char *pc_string)
{
    if (oal_unlikely(pc_string == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oal_io_print("%s"NEWLINE, pc_string);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_print_to_sdt
 * 功能描述  : 打印信息到PC侧可维可测工具平台中
 */
uint32_t oam_print_to_sdt(const char *pc_string)
{
    oal_netbuf_stru *pst_skb = NULL;
    uint32_t ul_ret;
    uint16_t us_strlen;

    if (oal_unlikely(pc_string == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 上报sdt字符串以'0'结束 */
    us_strlen = (uint16_t)OAL_STRLEN(pc_string);
    if (us_strlen == 0) {
        oal_io_print("us_strlen is 0\n");
        return ERANGE;
    }

    us_strlen = (us_strlen > OAM_REPORT_MAX_STRING_LEN) ? OAM_REPORT_MAX_STRING_LEN : us_strlen;

    pst_skb = oam_alloc_data2sdt(us_strlen);
    if (pst_skb == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* COPY打印的内容 */
    memset_s(oal_netbuf_data(pst_skb), us_strlen, 0, us_strlen);
    ul_ret = memcpy_s(oal_netbuf_data(pst_skb), pst_skb->len, pc_string, (uint32_t)us_strlen);
    if (ul_ret != EOK) {
        oal_io_print("oam_print_to_sdt::netbuf data copy failed.\n");
        oal_mem_sdt_netbuf_free(pst_skb, OAL_TRUE);
        return OAL_FAIL;
    }

    /* 下发至sdt接收队列，若队列满则串口输出 */
    ul_ret = oam_report_data2sdt(pst_skb, OAM_DATA_TYPE_STRING, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

uint32_t oam_upload_log_to_sdt(int8_t *pc_string)
{
    oal_netbuf_stru *pst_skb = NULL;
    uint32_t ul_ret;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (pc_string == NULL) {
        oal_io_print("oam_upload_log_to_sdt::pc_string is null!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_skb = oam_alloc_data2sdt(sizeof(oam_log_info_stru));
    if (pst_skb == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* COPY打印的内容 */
    ul_ret = memcpy_s(oal_netbuf_data(pst_skb), pst_skb->len, pc_string, sizeof(oam_log_info_stru));
    if (ul_ret != EOK) {
        oal_io_print("oam_upload_log_to_sdt::netbuf data copy failed\n");
        return OAL_FAIL;
    }
    /* 下发至sdt接收队列，若队列满则串口输出 */
    ul_ret = oam_report_data2sdt(pst_skb, OAM_DATA_TYPE_LOG, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

static uint32_t oam_upload_device_log_to_sdt(uint8_t *pc_string, uint16_t len)
{
    oal_netbuf_stru *pst_skb = NULL;
    uint32_t ul_ret;

    if (pc_string == NULL) {
        oal_io_print("oam_upload_log_to_sdt::pc_string is null!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }
    if (len == 0) {
        oal_io_print("len is 0\n");
        return ERANGE;
    }

    pst_skb = oam_alloc_data2sdt(len);
    if (pst_skb == NULL) {
        oal_io_print("alloc netbuf stru failed!\n");
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* COPY打印的内容 */
    ul_ret = memcpy_s(oal_netbuf_data(pst_skb), pst_skb->len, pc_string, len);
    if (ul_ret != EOK) {
        oal_io_print("netbuf valid data load failed!\n");
        oal_mem_sdt_netbuf_free(pst_skb, OAL_TRUE);
        return ul_ret;
    }

    /* 下发至sdt接收队列，若队列满则串口输出 */
    ul_ret = oam_report_data2sdt(pst_skb, OAM_DATA_TYPE_DEVICE_LOG, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

static uint32_t oam_send_device_data2sdt(uint8_t *pc_string, uint16_t len)
{
    uint32_t ul_ret;
    if (pc_string == NULL) {
        return OAL_EFAIL;
    }

    ul_ret = oam_upload_device_log_to_sdt(pc_string, len);

    return ul_ret;
}

int32_t oam_rx_post_action_function(struct hcc_handler *hcc, uint8_t stype,
                                    hcc_netbuf_stru *pst_hcc_netbuf, uint8_t *pst_context)
{
    uint8_t *puc_data = NULL;
    oal_reference(pst_context);
    oal_reference(hcc);

    if (oal_warn_on(pst_hcc_netbuf == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_hcc_netbuf is null", __FUNCTION__);
        return OAL_FAIL;
    }

    puc_data = oal_netbuf_data(pst_hcc_netbuf->pst_netbuf);
    if ((stype == DUMP_REG) || (stype == DUMP_MEM)) {
        oal_netbuf_free(pst_hcc_netbuf->pst_netbuf);
        return OAL_SUCC;
    }

    /* 调用OAM接口 */
    oam_send_device_data2sdt(puc_data, (uint16_t)pst_hcc_netbuf->len);

    oal_netbuf_free(pst_hcc_netbuf->pst_netbuf);
    return OAL_SUCC;
}

int32_t chr_rx_post_action_function(struct hcc_handler *hcc, uint8_t stype,
                                    hcc_netbuf_stru *pst_hcc_netbuf, uint8_t *pst_context)
{
    uint8_t *puc_data = NULL;
    oal_reference(pst_context);
    oal_reference(hcc);

    if (oal_warn_on(pst_hcc_netbuf == NULL)) {
        oal_print_hi11xx_log(HI11XX_LOG_ERR, "%s error: pst_hcc_netbuf is null", __FUNCTION__);
        return OAL_FAIL;
    }

#ifdef _PRE_WLAN_WAKEUP_SRC_PARSE
    if (wlan_pm_wkup_src_debug_get() == OAL_TRUE) {
        oal_io_print("wifi_wake_src:rx chr error!\n");
        wlan_pm_wkup_src_debug_set(OAL_FALSE);
    }
#endif

    puc_data = oal_netbuf_data(pst_hcc_netbuf->pst_netbuf);
#ifdef _PRE_CONFIG_HW_CHR
    chr_dev_exception_callback(puc_data, (uint16_t)pst_hcc_netbuf->len);
#endif
    oal_netbuf_free(pst_hcc_netbuf->pst_netbuf);
    return OAL_SUCC;
}

uint32_t oam_get_output_type(oam_output_type_enum_uint8 *pen_output_type)
{
    if (oal_unlikely(pen_output_type == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }
    *pen_output_type = g_oam_mng_ctx.en_output_type;

    return OAL_SUCC;
}

uint32_t oam_set_output_type(oam_output_type_enum_uint8 en_output_type)
{
    if (en_output_type >= OAM_OUTPUT_TYPE_BUTT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    g_oam_mng_ctx.en_output_type = en_output_type;

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_dump_buff_by_hex
 * 功能描述  : buff按十六进格式固定长度打印输出，每输出l_num个换行
 */
void oam_dump_buff_by_hex(uint8_t *puc_buff, int32_t l_len, int32_t l_num)
{
    int32_t l_loop;

    if (l_num == 0) {
        return;
    }

    for (l_loop = 0; l_loop < l_len; l_loop++) {
        oal_io_print("%02x ", puc_buff[l_loop]);

        if ((l_loop + 1) % l_num == 0) {
            oal_io_print("\n");
        }
    }

    oal_io_print("\n");
}

OAL_STATIC void oam_drv_func_hook_init(void)
{
    /* sdt侧对外钩子函数初始化 */
    g_oam_sdt_func_hook.p_sdt_report_data_func = NULL;
    g_oam_sdt_func_hook.p_sdt_get_wq_len_func = NULL;

    /* wal侧对外钩子函数初始化 */
    g_oam_wal_func_hook.p_wal_recv_cfg_data_func = NULL;
    g_oam_wal_func_hook.p_wal_recv_mem_data_func = NULL;
    g_oam_wal_func_hook.p_wal_recv_reg_data_func = NULL;
    g_oam_wal_func_hook.p_wal_recv_global_var_func = NULL;
}

/*
 * 函 数 名  : oam_sdt_func_fook_register
 * 功能描述  : oam模块注册sdt模块的钩子函数,供其他模块统一调用
 */
void oam_sdt_func_fook_register(oam_sdt_func_hook_stru *pfun_st_oam_sdt_hook)
{
    if (oal_unlikely(pfun_st_oam_sdt_hook == NULL)) {
        oal_warn_on(1);
        return;
    }
    g_oam_sdt_func_hook.p_sdt_report_data_func = pfun_st_oam_sdt_hook->p_sdt_report_data_func;
    g_oam_sdt_func_hook.p_sdt_get_wq_len_func = pfun_st_oam_sdt_hook->p_sdt_get_wq_len_func;
}

/*
 * 函 数 名  : oam_wal_func_fook_register
 * 功能描述  : oam模块注册wal模块的钩子函数,供其他模块统一调用
 */
void oam_wal_func_fook_register(oam_wal_func_hook_stru *pfun_st_oam_wal_hook)
{
    if (oal_unlikely(pfun_st_oam_wal_hook == NULL)) {
        oal_warn_on(1);
        return;
    }
    g_oam_wal_func_hook.p_wal_recv_cfg_data_func = pfun_st_oam_wal_hook->p_wal_recv_cfg_data_func;
    g_oam_wal_func_hook.p_wal_recv_mem_data_func = pfun_st_oam_wal_hook->p_wal_recv_mem_data_func;
    g_oam_wal_func_hook.p_wal_recv_reg_data_func = pfun_st_oam_wal_hook->p_wal_recv_reg_data_func;
    g_oam_wal_func_hook.p_wal_recv_global_var_func = pfun_st_oam_wal_hook->p_wal_recv_global_var_func;
#if defined(_PRE_WLAN_FEATURE_DATA_SAMPLE) || defined(_PRE_WLAN_FEATURE_PSD_ANALYSIS)
    g_oam_wal_func_hook.p_wal_recv_sample_data_func = pfun_st_oam_wal_hook->p_wal_recv_sample_data_func;
#endif
}

/*
 * 函 数 名  : oam_filter_data2sdt
 * 功能描述  : sdt消息入队是否需要过滤
 */
uint32_t oam_filter_data2sdt(oam_data_type_enum_uint8 en_type)
{
    if (g_sdt_stat_info.ul_wq_len < WLAN_SDT_MSG_FLT_HIGH_THD) {
        g_sdt_stat_info.en_filter_switch = OAL_FALSE;
        return OAM_FLT_PASS;
    } else if ((g_sdt_stat_info.ul_wq_len >= WLAN_SDT_MSG_FLT_HIGH_THD)
               && (g_sdt_stat_info.ul_wq_len < WLAN_SDT_MSG_QUEUE_MAX_LEN)) {
        /* 消息队列达到过滤上限，过滤非日志消息 */
        g_sdt_stat_info.en_filter_switch = OAL_TRUE;

        return (en_type == ((uint8_t)OAM_DATA_TYPE_LOG)) ? OAM_FLT_PASS : OAM_FLT_DROP;
    }

    /* 消息队列满全部过滤 */
    return OAM_FLT_DROP;
}

/*
 * 函 数 名  : oam_alloc_data2sdt
 * 功能描述  : 封装发往sdt app侧的netlink消息
 *             1) ul_data_len与oal_mem_sdt_netbuf_alloc 传入的长度含不含SDT头；由接口适配
 *             2) 申请好的netbuf直接往数据指针长度即可
 *             SDT与NLK消息头在本接口中不用考虑，由上报接口统一填写
 */
oal_netbuf_stru *oam_alloc_data2sdt(uint16_t us_data_len)
{
    oal_netbuf_stru *pst_netbuf = NULL;
#if ((_PRE_OS_VERSION_WIN32_RAW != _PRE_OS_VERSION))
    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_data_len + WLAN_SDT_SKB_RESERVE_LEN, OAL_TRUE);
    if (pst_netbuf == NULL) {
        oal_io_print("oal_mem_sdt_netbuf_alloc failed!\n");
        return NULL;
    }

    oal_netbuf_reserve(pst_netbuf, WLAN_SDT_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_data_len);
#endif
    return pst_netbuf;
}

/*
 * 函 数 名  : oam_report_data2sdt
 * 功能描述  : oam将消息下发至sdt侧，由sdt侧统一上报至app侧
 */
uint32_t oam_report_data2sdt(oal_netbuf_stru *pst_netbuf,
                             oam_data_type_enum_uint8 en_type,
                             oam_primid_type_enum_uint8 en_prim)
{
    if (oal_unlikely(pst_netbuf == NULL)) {
        oal_warn_on(1);
        return OAL_FAIL;
    }
    /* 判断sdt发送消息队列是否已满，若满输出至串口 */
    if (oal_likely(g_oam_sdt_func_hook.p_sdt_get_wq_len_func != NULL)) {
        g_sdt_stat_info.ul_wq_len = (uint32_t)g_oam_sdt_func_hook.p_sdt_get_wq_len_func();
    }

    if (oam_filter_data2sdt(en_type) != OAM_FLT_PASS) {
        oam_sdt_stat_incr(ul_filter_cnt);
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);

        /* Note: 目前上层函数仅仅使用该返回值打印warning信息而已 */
        return OAL_FAIL;
    }

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        oal_io_print("oam_report_data2sdt p_sdt_report_data_func is NULL. \n");
        return OAL_FAIL;
    }

    g_oam_sdt_func_hook.p_sdt_report_data_func(pst_netbuf, en_type, en_prim);

    return OAL_SUCC;
}

void oam_sdt_func_fook_unregister(void)
{
    /* 函数指针赋值 */
    g_oam_sdt_func_hook.p_sdt_report_data_func = NULL;
    g_oam_sdt_func_hook.p_sdt_get_wq_len_func = NULL;
}

/*
 * 函 数 名  : oam_wal_func_fook_unregister
 * 功能描述  : wal对外钩子函数去注册
 */
void oam_wal_func_fook_unregister(void)
{
    /* 函数指针赋值 */
    g_oam_wal_func_hook.p_wal_recv_cfg_data_func = NULL;
    g_oam_wal_func_hook.p_wal_recv_global_var_func = NULL;
    g_oam_wal_func_hook.p_wal_recv_mem_data_func = NULL;
    g_oam_wal_func_hook.p_wal_recv_reg_data_func = NULL;
}

/*
 * 函 数 名  : oam_main_init
 * 功能描述  : OAM模块初始化总入口，包含OAM模块内部所有特性的初始化。
 */
int32_t oam_main_init(void)
{
    uint32_t ul_rslt;

    /* 初始化可维可测输出方式 */
    ul_rslt = oam_set_output_type(OAM_OUTPUT_TYPE_SDT);
    if (ul_rslt != OAL_SUCC) {
        oal_io_print("oam_main_init call oam_set_output_type fail %d\n", ul_rslt);
        return -OAL_EFAIL;  //lint !e527
    }

    /* 完成LOG模块的初始化操作 */
    oam_log_init();

    /* 完成EVENT模块的初始化操作 */
    ul_rslt = oam_event_init();
    if (ul_rslt != OAL_SUCC) {
        oal_io_print("oam_main_init call oam_event_init fail %d\n", ul_rslt);
        return -OAL_EFAIL;  //lint !e527
    }

    /* 初始化oam模块的钩子函数 */
    oam_drv_func_hook_init();

    /* 统计模块初始化 */
    oam_statistics_init();

#ifndef BFGX_UART_DOWNLOAD_SUPPORT
    hcc_rx_register(hcc_get_110x_handler(), HCC_ACTION_TYPE_OAM, oam_rx_post_action_function, NULL);
    hcc_rx_register(hcc_get_110x_handler(), HCC_ACTION_TYPE_CHR, chr_rx_post_action_function, NULL);
#endif
    return OAL_SUCC;
}

void oam_main_exit(void)
{
    return;
}

/*lint -e578*/ /*lint -e19*/
/*lint -e19*/
oal_module_symbol(oam_main_init);
oal_module_symbol(oam_main_exit);
oal_module_symbol(oam_set_output_type);
oal_module_symbol(oam_get_output_type);
oal_module_symbol(oam_print);
oal_module_symbol(g_oam_mng_ctx);
oal_module_symbol(oam_dump_buff_by_hex);
oal_module_symbol(g_oam_sdt_func_hook);
oal_module_symbol(g_oam_wal_func_hook);
oal_module_symbol(oam_sdt_func_fook_register);
oal_module_symbol(oam_sdt_func_fook_unregister);
oal_module_symbol(oam_wal_func_fook_register);
oal_module_symbol(oam_wal_func_fook_unregister);
oal_module_symbol(oam_report_data2sdt);
oal_module_symbol(g_sdt_stat_info);
oal_module_symbol(oam_alloc_data2sdt);
oal_module_symbol(g_oam_feature_list);
oal_module_symbol(g_bcast_addr);

oal_module_license("GPL");
