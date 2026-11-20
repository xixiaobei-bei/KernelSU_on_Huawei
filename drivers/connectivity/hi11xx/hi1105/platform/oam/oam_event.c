

/* 头文件包含 */
#include "oam_event.h"
#include "oam_main.h"
#include "oam_ext_if.h"
#include "securec.h"
#ifndef _PRE_LINUX_TEST
#include "board.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_OAM_EVENT_C

/*
 * 函 数 名  : oam_report_data_get_global_switch
 * 功能描述  : 获取收发数据帧的总开关，如果总开关不开，则不用进入上报逻辑判断
 *             某一个用户的上报开关是否打开，不用消耗mips。只要有一个用户的开
 *             关打开了总开关就打开
 * 输入参数  : en_direction:方向，发送还是接收  0 发送  1接收
 */
oal_switch_enum_uint8 oam_report_data_get_global_switch(oam_ota_frame_direction_type_enum_uint8 en_direction)
{
    if (en_direction >= oal_unlikely(OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)) {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    return g_oam_mng_ctx.st_user_track_ctx.aen_data_global_switch[en_direction];
}

/*
 * 函 数 名  : oam_report_data_set_global_switch
 * 功能描述  : 设置上报数据帧的全局开关，只要有一个用户要上报，则总开关就为开，
 *             否则关闭，防止消耗mips
 */
OAL_STATIC uint32_t oam_report_data_set_global_switch(oam_ota_frame_direction_type_enum_uint8 direction)
{
    uint16_t usr_idx;
    oal_switch_enum_uint8 mcast_switch = OAL_SWITCH_OFF;
    oal_switch_enum_uint8 ucast_switch = OAL_SWITCH_OFF;

    if (direction >= oal_unlikely(OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)) {
        return OAL_ERR_CODE_ARRAY_OVERFLOW;
    }

    /* 先检查单播数据帧是否有用户已经打开了 */
    for (usr_idx = 0; usr_idx < WLAN_USER_MAX_USER_LIMIT; usr_idx++) {
        if (g_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[usr_idx][direction].value != 0) {
            ucast_switch = OAL_SWITCH_ON;
            break;
        }
    }

    /* 再检查组播数据帧是否开关打开了 */
    if (g_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[direction].value != 0) {
        mcast_switch = OAL_SWITCH_ON;
    }

    g_oam_mng_ctx.st_user_track_ctx.aen_data_global_switch[direction] = (ucast_switch | mcast_switch);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_eth_frame_set_switch
 * 功能描述  : 设置上报以太网帧的开关
 * 输入参数  : us_user_idx:用户资源池id
 *             en_switch  :打开还是关闭
 *             en_eth_direction:发送还是接收方向
 */
uint32_t oam_report_eth_frame_set_switch(uint16_t us_user_idx,
                                         oal_switch_enum_uint8 en_switch,
                                         oam_ota_frame_direction_type_enum_uint8 en_eth_direction)
{
    uint8_t ota_switch_id;

    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT) {
        oal_io_print("oam_report_eth_frame_set_switch::user_idx exceeds!\n");
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (en_eth_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        oal_io_print("oam_report_eth_frame_set_switch::eth_direction exceeds!\n");
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    g_oam_mng_ctx.st_user_track_ctx.aen_eth_data_ctx[us_user_idx][en_eth_direction] = en_switch;

    ota_switch_id = en_eth_direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX ?
                                        OAM_OTA_SWITCH_TX_ETH_FRAME : OAM_OTA_SWITCH_RX_ETH_FRAME;
    oam_ota_set_switch(ota_switch_id, en_switch);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_eth_frame_get_switch
 * 功能描述  : 获取上报以太网帧的开关
 * 输入参数  : us_user_idx:用户资源池id
 *             en_eth_direction:发送还是接收
 *             pen_eth_switch:以太网帧打印开关
 */
uint32_t oam_report_eth_frame_get_switch(uint16_t us_user_idx,
                                         oam_ota_frame_direction_type_enum_uint8 en_eth_direction,
                                         oal_switch_enum_uint8 *pen_eth_switch)
{
    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT) {
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (en_eth_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (pen_eth_switch == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    *pen_eth_switch = g_oam_mng_ctx.st_user_track_ctx.aen_eth_data_ctx[us_user_idx][en_eth_direction];

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_mcast_set_switch
 * 功能描述  : 设置上报80211组播\广播数据或者管理帧的开关
 * 输入参数  : en_mcast_direction:帧是发送还是接收
 *             en_frame_type     :数据帧还是管理帧(包含控制帧)
 *             en_frame_switch   :帧开关
 *             en_cb_switch      :CB开关
 *             en_dscr_switch    :描述符开关
 */
uint32_t oam_report_80211_mcast_set_switch(oam_ota_frame_direction_type_enum_uint8 en_mcast_direction,
                                           oam_user_track_frame_type_enum_uint8 en_frame_type,
                                           oam_80211_frame_ctx_union *oam_frame_report)
{
    if (en_mcast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    /* 为组播管理帧设置开关 */
    if (en_frame_type == OAM_USER_TRACK_FRAME_TYPE_MGMT) {
        g_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].value = oam_frame_report->value;
    } else {
        g_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].value = oam_frame_report->value;

        /* 设置总开关 */
        oam_report_data_set_global_switch(en_mcast_direction);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_mcast_get_switch
 * 功能描述  : 获取上报80211组播帧的开关
 * 输入参数  : en_mcast_direction:帧是发送还是接收
 *             en_frame_type     :数据帧还是管理帧(包含控制帧)
 *             pen_frame_switch  :帧开关
 *             pen_cb_switch     :CB开关
 *             pen_dscr_switch   :描述符开关
 */
uint32_t oam_report_80211_mcast_get_switch(oam_ota_frame_direction_type_enum_uint8 en_mcast_direction,
                                           oam_user_track_frame_type_enum_uint8 en_frame_type,
                                           oam_80211_frame_ctx_union *oam_frame_report)
{
    if (en_mcast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (oal_unlikely(oal_any_null_ptr1(oam_frame_report))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_frame_type == OAM_USER_TRACK_FRAME_TYPE_MGMT) {
        oam_frame_report->value = g_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_mgmt_ctx[en_mcast_direction].value;
    } else {
        oam_frame_report->value = g_oam_mng_ctx.st_user_track_ctx.ast_80211_mcast_data_ctx[en_mcast_direction].value;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_ucast_set_switch
 * 功能描述  : 设置80211单播帧上报的开关
 * 输入参数  : en_ucast_direction:帧是发送还是接收
 *             en_frame_type     :数据帧还是管理帧(包含控制帧)
 *             en_frame_switch   :帧开关
 *             en_cb_switch      :CB开关
 *             en_dscr_switch    :描述符开关
 *             us_user_idx       :用户资源池id
 */
uint32_t oam_report_80211_ucast_set_switch(oam_ota_frame_direction_type_enum_uint8 en_ucast_direction,
                                           oam_user_track_frame_type_enum_uint8 en_frame_type,
                                           oam_80211_frame_ctx_union *oam_switch_param,
                                           uint16_t us_user_idx)
{
    if (en_ucast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT) {
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (en_frame_type == OAM_USER_TRACK_FRAME_TYPE_MGMT) {
        g_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].value =
            oam_switch_param->value;
    } else {
        g_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].value =
            oam_switch_param->value;

        /* 设置总开关 */
        oam_report_data_set_global_switch(en_ucast_direction);
    }
    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_ucast_get_switch
 * 功能描述  : 获取80211单播帧的上报开关
 * 输入参数  : en_ucast_direction:帧是发送还是接收
 *             en_frame_type     :数据帧还是管理帧(包含控制帧)
 *             pen_frame_switch  :帧开关
 *             pen_cb_switch     :CB开关
 *             pen_dscr_switch   :描述符开关
 *             us_user_idx       :用户资源池id
 */
uint32_t oam_report_80211_ucast_get_switch(oam_ota_frame_direction_type_enum_uint8 en_ucast_direction,
                                           oam_user_track_frame_type_enum_uint8 en_frame_type,
                                           oam_80211_frame_ctx_union *oam_frame_report_un,
                                           uint16_t us_user_idx)
{
    if (en_ucast_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (en_frame_type >= OAM_USER_TRACK_FRAME_TYPE_BUTT) {
        return OAL_ERR_CODE_CONFIG_UNSUPPORT;
    }

    if (us_user_idx >= WLAN_USER_MAX_USER_LIMIT) {
        return OAL_ERR_CODE_OAM_EVT_USER_IDX_EXCEED;
    }

    if (oal_unlikely(oal_any_null_ptr1(oam_frame_report_un))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_frame_type == OAM_USER_TRACK_FRAME_TYPE_MGMT) {
        oam_frame_report_un->value =
            g_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_mgmt_ctx[us_user_idx][en_ucast_direction].value;
    } else {
        oam_frame_report_un->value =
            g_oam_mng_ctx.st_user_track_ctx.ast_80211_ucast_data_ctx[us_user_idx][en_ucast_direction].value;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_probe_set_switch
 * 功能描述  : 设置probe request 和 probe response的打印开关
 * 输入参数  : en_ucast_direction:帧是发送还是接收
 *             en_frame_switch   :帧开关
 *             en_cb_switch      :CB开关
 *             en_dscr_switch    :描述符开关
 */
uint32_t oam_report_80211_probe_set_switch(oam_ota_frame_direction_type_enum_uint8 en_probe_direction,
                                           oam_80211_frame_ctx_union *oam_switch_param)
{
    if (en_probe_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (oam_switch_param == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    g_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].value = oam_switch_param->value;

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_probereq_get_switch
 * 功能描述  : 获取prebe request和probe response的打印开关
 * 输入参数  : en_ucast_direction:帧是发送还是接收
 *             pen_frame_switch  :帧开关
 *             pen_cb_switch     :CB开关
 *             pen_dscr_switch   :描述符开关
 */
uint32_t oam_report_80211_probe_get_switch(oam_ota_frame_direction_type_enum_uint8 en_probe_direction,
                                           oam_80211_frame_ctx_union *oam_frame_report_un)
{
    if (en_probe_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if (oal_unlikely(oal_any_null_ptr1(oam_frame_report_un))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    oam_frame_report_un->value = g_oam_mng_ctx.st_user_track_ctx.aen_80211_probe_switch[en_probe_direction].value;

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_dhcp_arp_set_switch
 * 功能描述  : 设置广播的dhcp和arp的上报开关
 */
uint32_t oam_report_dhcp_arp_set_switch(oal_switch_enum_uint8 en_switch)
{
    g_oam_mng_ctx.st_user_track_ctx.en_tx_mcast_dhcp_arp_switch = en_switch;

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_dhcp_arp_get_switch
 * 功能描述  : 获取上报广播dhcp和arp的开关
 */
oal_switch_enum_uint8 oam_report_dhcp_arp_get_switch(void)
{
    return g_oam_mng_ctx.st_user_track_ctx.en_tx_mcast_dhcp_arp_switch;
}

static void oam_ota_set_global_switch(uint8_t ota_switch_id, uint8_t ota_switch)
{
    if (ota_switch == OAL_SWITCH_ON) {
        g_oam_mng_ctx.ast_ota_ctx.golble_switch |= (uint32_t)((uint32_t)ota_switch << ota_switch_id);
    } else {
        /* static函数，上层函数已经保证了ota_switch只有ON和OFF两种情况 */
        g_oam_mng_ctx.ast_ota_ctx.golble_switch &= (uint32_t)((uint32_t)ota_switch << ota_switch_id);
    }
}

void oam_ota_set_switch(uint8_t ota_switch_id, uint8_t ota_switch)
{
    if ((ota_switch_id >= OAM_OTA_SWITCH_BUTT) || (ota_switch >= OAL_SWITCH_BUTT)) {
        return;
    }

    g_oam_mng_ctx.ast_ota_ctx.ota_switch[ota_switch_id] = ota_switch;

    /* 设置ota上报全局开关mib，上报时首先判断全局开关mib若为0，则直接返回，降低mips */
    oam_ota_set_global_switch(ota_switch_id, ota_switch);
}

/* ota上报全局开关mib，上报时首先判断该值，如果为0，则直接返回，减少消耗mips */
uint32_t oam_ota_get_global_switch(void)
{
    return g_oam_mng_ctx.ast_ota_ctx.golble_switch;
}

oal_switch_enum_uint8 oam_ota_get_switch(uint8_t ota_switch_id)
{
    if (ota_switch_id >= OAM_OTA_SWITCH_BUTT) {
        return OAL_SWITCH_OFF;
    }

    return g_oam_mng_ctx.ast_ota_ctx.ota_switch[ota_switch_id];
}

OAL_STATIC void oam_set_ota_board_type(oam_ota_stru *ota_data)
{
    int32_t chip_type = get_hi110x_subchip_type();
    switch (chip_type) {
        case BOARD_VERSION_HI1103:
            ota_data->st_ota_hdr.auc_resv[0] = OAM_OTA_TYPE_1103_HOST;
            break;
        case BOARD_VERSION_HI1105:
            ota_data->st_ota_hdr.auc_resv[0] = OAM_OTA_TYPE_1105_HOST;
            break;
        case BOARD_VERSION_HI1106:
            ota_data->st_ota_hdr.auc_resv[0] = OAM_OTA_TYPE_1106_HOST;
            break;
        default:
            ota_data->st_ota_hdr.auc_resv[0] = OAM_OTA_TYPE_CHIP_INVALID;
            break;
    }
}

uint32_t oam_event_init(void)
{
    /* 设置单用户跟踪相关的所有开关 */
    oam_report_set_all_switch(OAL_SWITCH_OFF);

    oam_report_dhcp_arp_set_switch(OAL_SWITCH_OFF);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_ota_report_to_std
 * 功能描述  : 将OTA数据打印到标准窗口:
 *             (1)如果是描述符，中断信息，内存池信息，定时器信息,或者事件队列信息,
 *                则屏幕上每行打印4字节；
 *             (2)如果是帧或者内存块信息，则每行打印20字节
 *             (3)如果是帧的话，而且帧体太长，则帧体只打印前300个字节
 * 输入参数   : puc_param_one_addr:如果是描述符，则表示描述符的地址;如果是帧，表示帧头地址
 *              ul_param_one_len  :如果是描述符，则表示描述符长度;如果是帧，表示帧头长度
 *              puc_param_two_addr:如果是描述符，则为0;如果是帧，表示帧体地址
 *              ul_param_two_len  :如果是描述符，则为0;如果是帧，表示帧体长度
 *              en_ota_type       :OTA类型
 */
OAL_STATIC uint32_t oam_ota_report_to_std(uint8_t *puc_param_one_addr,
                                          uint16_t us_param_one_len,
                                          uint8_t *puc_param_two_addr,
                                          uint16_t us_param_two_len,
                                          oam_ota_type_enum_uint8 en_ota_type)
{
    if ((us_param_two_len == 0) || (puc_param_two_addr == NULL)) {
        oal_io_print("\n\nOTA TYPE is--> %d and OTA DATA is:\n", en_ota_type);
        oam_dump_buff_by_hex(puc_param_one_addr, us_param_one_len, 4); /* 4表示每打印4次内容则进行一次换行 */
    } else {
        /* 打印第一段内容 */
        oal_io_print("\n\nOTA TYPE is--> %d and OTA DATA the first part is:\n", en_ota_type);
        oam_dump_buff_by_hex(puc_param_one_addr, us_param_one_len, OAM_PRINT_CRLF_NUM);

        /* 打印第二段内容 */
        oal_io_print("\nOTA DATA tht second part is:\n");

        if (en_ota_type == OAM_OTA_TYPE_80211_FRAME) {
            us_param_two_len = oal_min(us_param_two_len, OAM_OTA_DATA_TO_STD_MAX_LEN);
        }

        oam_dump_buff_by_hex(puc_param_two_addr, us_param_two_len, OAM_PRINT_CRLF_NUM);
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_ota_report_to_sdt
 * 功能描述  : 将空口消息上报SDT工具，包括接收描述符，发送描述符，帧
 * 输入参数  : puc_param_one_addr:如果是描述符，则表示描述符的地址;如果是帧，表示帧头地址
 *             ul_param_one_len  :如果是描述符，则表示描述符长度;如果是帧，表示帧头长度
 *             puc_param_two_addr:如果是描述符，则为0;如果是帧，表示帧体地址
 *             ul_param_two_len  :如果是描述符，则为0;如果是帧，表示帧体长度
 *             en_ota_type       :OTA类型
 */
/*lint -e662*/
uint32_t oam_ota_report_to_sdt(uint8_t *puc_param_one_addr,
                               uint16_t us_param_one_len,
                               uint8_t *puc_param_two_addr,
                               uint16_t us_param_two_len,
                               oam_ota_type_enum_uint8 en_ota_type)
{
    uint32_t ul_ret = OAL_SUCC;
#if ((_PRE_OS_VERSION_RAW != _PRE_OS_VERSION) && (_PRE_OS_VERSION_WIN32_RAW != _PRE_OS_VERSION))
    uint32_t ul_tick;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf;
    oam_ota_stru *pst_ota_data;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 为上报OTA数据申请空间，如果是描述符，us_param_two_len为0 */
    us_skb_len = us_param_one_len + us_param_two_len + sizeof(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        if ((us_param_one_len + sizeof(oam_ota_hdr_stru)) < us_skb_len) {
            us_param_two_len = us_skb_len - us_param_one_len - (uint16_t)sizeof(oam_ota_hdr_stru);
        } else {
            us_param_one_len = us_skb_len - sizeof(oam_ota_hdr_stru);
            us_param_two_len = 0;
        }
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;
    oam_set_ota_board_type(pst_ota_data);

    switch (en_ota_type) {
        /* 封装接收或者发送描述符对应OTA结构体的其它成员 */
        case OAM_OTA_TYPE_RX_DSCR:
        case OAM_OTA_TYPE_TX_DSCR:
        case OAM_OTA_TYPE_IRQ:
        case OAM_OTA_TYPE_EVENT_QUEUE:
        case OAM_OTA_TYPE_TIMER:
        case OAM_OTA_TYPE_MEMPOOL:
        case OAM_OTA_TYPE_HMAC_VAP:
        case OAM_OTA_TYPE_DMAC_VAP:
        case OAM_OTA_TYPE_HMAC_USER:
        case OAM_OTA_TYPE_DMAC_USER:
        case OAM_OTA_TYPE_HMAC_VAP_MEMBER_SIZE:
        case OAM_OTA_TYPE_DMAC_VAP_MEMBER_SIZE:
        case OAM_OTA_TYPE_RX_DSCR_PILOT:
        case OAM_OTA_TYPE_TX_DSCR_PILOT:
            pst_ota_data->st_ota_hdr.us_ota_data_len = us_param_one_len;
            ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                              (uint32_t)(us_param_one_len + us_param_two_len),
                              (const void *)puc_param_one_addr,
                              (uint32_t)us_param_one_len);
            if (ul_ret != EOK) {
                oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
                oal_io_print("oam_ota_report_to_sdt::memcpy_s failed.\n");
                return OAL_FAIL;
            }
            break;

        /* 封装帧对应OTA结构体的其它成员 */
        case OAM_OTA_TYPE_80211_FRAME:
        case OAM_OTA_TYPE_MEMBLOCK:
            pst_ota_data->st_ota_hdr.uc_frame_hdr_len = (uint8_t)us_param_one_len;
            pst_ota_data->st_ota_hdr.us_ota_data_len = us_param_one_len + us_param_two_len;

            /* 复制帧头 */
            ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                              (uint32_t)(us_param_one_len + us_param_two_len),
                              (const void *)puc_param_one_addr,
                              (uint32_t)us_param_one_len);
            if (ul_ret != EOK) {
                oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
                oal_io_print("oam_ota_report_to_sdt::memcpy_s failed.\n");
                return OAL_FAIL;
            }

            /* 复制帧体 */
            if (puc_param_two_addr != NULL) {
                ul_ret = memcpy_s((void *)(pst_ota_data->auc_ota_data + us_param_one_len),
                                  (uint32_t)(us_param_two_len),
                                  (const void *)puc_param_two_addr,
                                  (uint32_t)us_param_two_len);
                if (ul_ret != EOK) {
                    oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
                    oal_io_print("oam_ota_report_to_sdt::memcpy_s failed.\n");
                    return OAL_FAIL;
                }
            }
            break;

        /* 无效值 */
        default:
            oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
            return OAL_ERR_CODE_INVALID_CONFIG;
    }

    /* 判断sdt发送消息队列是否已满，若满输出至串口 */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);
#endif
    return ul_ret;
}

/*
 * 函 数 名  : oam_ota_report
 * 功能描述  : 上报OTA(over the air)事件,包括接收描述符，发送描述符，帧三种。
 * 输入参数  : puc_param_one_addr:如果是描述符，则表示描述符的地址;如果是帧，表示帧头地址
 *             ul_param_one_len  :如果是描述符，则表示描述符长度;如果是帧，表示帧头长度
 *             puc_param_two_addr:如果是描述符，则为0;如果是帧，表示帧体地址
 *             ul_param_two_len  :如果是描述符，则为0;如果是帧，表示帧体长度
 *             en_ota_type       :OTA类型
 */
uint32_t oam_ota_report(uint8_t *puc_param_one_addr,
                        uint16_t us_param_one_len,
                        uint8_t *puc_param_two_addr,
                        uint16_t us_param_two_len,
                        oam_ota_type_enum_uint8 en_ota_type)
{
    uint32_t ul_rslt = OAL_ERR_CODE_BUTT;

    switch (g_oam_mng_ctx.en_output_type) {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_rslt = oam_ota_report_to_std(puc_param_one_addr,
                                            us_param_one_len,
                                            puc_param_two_addr,
                                            us_param_two_len,
                                            en_ota_type);

            break;

        /* 输出至SDT工具 */
        case OAM_OUTPUT_TYPE_SDT:
            ul_rslt = oam_ota_report_to_sdt(puc_param_one_addr,
                                            us_param_one_len,
                                            puc_param_two_addr,
                                            us_param_two_len,
                                            en_ota_type);

            break;

        default:
            ul_rslt = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    if (ul_rslt != OAL_SUCC) {
        return ul_rslt;
    }

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_80211_frame_to_console
 * 功能描述  : 将80211帧内容打印到控制台（一般是串口）
 * 输入参数  : puc_mac_hdr_addr  :mac帧头地址
 *             us_mac_hdr_len    :mac帧头长度
 *             puc_mac_body_addr :mac帧体地址
 *             us_mac_frame_len  :mac帧总长度(帧头+帧体)
 *             en_frame_direction:mac方向(tx流程还是rx流程)
 */
OAL_STATIC uint32_t oam_report_80211_frame_to_console(uint8_t *puc_mac_hdr_addr,
                                                      uint8_t uc_mac_hdr_len,
                                                      uint8_t *puc_mac_body_addr,
                                                      uint16_t us_mac_frame_len,
                                                      oam_ota_frame_direction_type_enum_uint8 en_frame_direction)
{
    uint16_t us_80211_frame_body_len;

    if (en_frame_direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        oal_io_print("oam_report_80211_frame_to_console::tx_80211_frame header:\n");
    } else {
        oal_io_print("oam_report_80211_frame_to_console::rx_80211_frame header:\n");
    }

    oam_dump_buff_by_hex(puc_mac_hdr_addr, uc_mac_hdr_len, OAM_PRINT_CRLF_NUM);

    if (uc_mac_hdr_len > us_mac_frame_len) {
        oal_io_print("oam_report_80211_frame_to_console::rx_80211_frame invalid frame\n");
        return OAL_FAIL;
    }

    us_80211_frame_body_len = us_mac_frame_len - uc_mac_hdr_len;

    oal_io_print("oam_report_80211_frame_to_console::80211_frame body:\n");
    oam_dump_buff_by_hex(puc_mac_body_addr, us_80211_frame_body_len, OAM_PRINT_CRLF_NUM);

    return OAL_SUCC;
}
/*
 * 函 数 名  : oam_hide_mac_addr
 * 功能描述  : 处于安全保护掩盖日志中OTA的帧内的mac地址
 */
OAL_STATIC void oam_hide_mac_addr(uint8_t *puc_mac_hdr, uint8_t uc_beacon_hdr_len)
{
    if (puc_mac_hdr == NULL || uc_beacon_hdr_len < WLAN_MGMT_FRAME_HEADER_LEN) {
        return;
    }
    /* addr1 */
    puc_mac_hdr[5] = 0xff; /* 第1个地址的存放位置在数组的第5个 */
    puc_mac_hdr[6] = 0xff; /* 第1个地址的存放位置在数组的第6个 */
    puc_mac_hdr[7] = 0xff; /* 第1个地址的存放位置在数组的第7个 */

    /* addr2 */
    puc_mac_hdr[11] = 0xff; /* 第2个地址的存放位置在数组的第11个 */
    puc_mac_hdr[12] = 0xff; /* 第2个地址的存放位置在数组的第12个 */
    puc_mac_hdr[13] = 0xff; /* 第2个地址的存放位置在数组的第13个 */

    /* addr3 */
    puc_mac_hdr[17] = 0xff; /* 第3个地址的存放位置在数组的第17个 */
    puc_mac_hdr[18] = 0xff; /* 第3个地址的存放位置在数组的第18个 */
    puc_mac_hdr[19] = 0xff; /* 第3个地址的存放位置在数组的第19个 */
}

OAL_STATIC uint32_t oam_report_80211_frame_to_sdt(uint8_t *puc_user_macaddr,
                                                  uint8_t *puc_mac_hdr_addr,
                                                  uint8_t uc_mac_hdr_len,
                                                  uint8_t *puc_mac_body_addr,
                                                  uint16_t us_mac_frame_len,
                                                  oam_ota_frame_direction_type_enum_uint8 en_frame_direction)
{
    uint32_t ul_tick;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    oam_ota_stru *pst_ota_data = NULL;
    uint32_t ul_ret;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 为上报80211帧申请空间 */
    us_skb_len = us_mac_frame_len + sizeof(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_mac_frame_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - sizeof(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_header(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = OAM_OTA_TYPE_80211_FRAME;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = uc_mac_hdr_len;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_mac_frame_len;
    pst_ota_data->st_ota_hdr.en_frame_direction = en_frame_direction;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_macaddr);
    oam_set_ota_board_type(pst_ota_data);

    /* 复制帧头 */
    ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                      (uint32_t)us_mac_frame_len,
                      (const void *)puc_mac_hdr_addr,
                      (uint32_t)uc_mac_hdr_len);
    /* 复制帧体 */
    ul_ret += memcpy_s((void *)(pst_ota_data->auc_ota_data + uc_mac_hdr_len),
                       (uint32_t)(us_mac_frame_len - uc_mac_hdr_len),
                       (const void *)puc_mac_body_addr,
                       (uint32_t)(us_mac_frame_len - uc_mac_hdr_len));
    if (ul_ret != EOK) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_report_80211_frame_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }
    oam_hide_mac_addr(pst_ota_data->auc_ota_data, uc_mac_hdr_len);

    /* 判断sdt发送消息队列是否已满，若满输出至串口 */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_80211_frame
 * 功能描述  : 上报802.11帧
 * 输入参数  : puc_user_macaddr  :目的用户mac地址
 *             puc_mac_hdr_addr  :mac帧头地址
 *             us_mac_hdr_len    :mac帧头长度
 *             puc_mac_body_addr :mac帧体地址
 *             us_mac_frame_len  :mac帧总长度(帧头+帧体)
 *             en_frame_direction:mac方向(tx流程还是rx流程)
 */
uint32_t oam_report_80211_frame(uint8_t *puc_user_macaddr,
                                uint8_t *puc_mac_hdr_addr,
                                uint8_t uc_mac_hdr_len,
                                uint8_t *puc_mac_body_addr,
                                uint16_t us_mac_frame_len,
                                oam_ota_frame_direction_type_enum_uint8 en_frame_direction)
{
    uint32_t ul_ret = OAL_SUCC;
    uint32_t ul_oam_ret = OAL_SUCC;
    uint32_t ul_return_addr = 0;

    /* Less IO_print, less oam_report_80211_frame_to_console, use oam_report_80211_frame_to_sdt for log.
       Suggested by gongxiangling & wangzhenzhong */
#if (_PRE_OS_VERSION_RAW == _PRE_OS_VERSION)
    ul_return_addr = __return_address();
#endif

    if (oal_unlikely(oal_any_null_ptr3(puc_mac_hdr_addr, puc_mac_body_addr, puc_user_macaddr))) {
        oam_error_log4(0, OAM_SF_ANY,
                       "{oam_report_80211_frame:[device] puc_mac_hdr_addr = 0x%X, puc_mac_body_addr = 0x%X, puc_user_macaddr = 0x%X, __return_address = 0x%X}",
                       (uintptr_t)puc_mac_hdr_addr, (uintptr_t)puc_mac_body_addr,
                       (uintptr_t)puc_user_macaddr, ul_return_addr);
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 检查帧头长度合法性 */
    if ((uc_mac_hdr_len > WLAN_MAX_FRAME_HEADER_LEN) || (uc_mac_hdr_len < WLAN_MIN_FRAME_HEADER_LEN)) {
        oam_warning_log4(0, OAM_SF_ANY,
                         "{oam_report_80211_frame:HEAD CHECK! HDR_LEN_INVALID!!hearder_len = %d, frame_len = %d, en_frame_direction = %d, return_addres = 0x%X}",
                         uc_mac_hdr_len, us_mac_frame_len, en_frame_direction, ul_return_addr);
    }

    /* 检查mac帧总长度合法性 */
    if (uc_mac_hdr_len > us_mac_frame_len) {
        oam_report_dft_params(BROADCAST_MACADDR, puc_mac_hdr_addr,
                              uc_mac_hdr_len, OAM_OTA_TYPE_80211_FRAME);
        oam_warning_log4(0, OAM_SF_ANY,
                         "{oam_report_80211_frame:HEAD/FRAME CHECK! hearder_len = %d, frame_len = %d, en_frame_direction = %d, return_addres = 0x%X}",
                         uc_mac_hdr_len, us_mac_frame_len, en_frame_direction, ul_return_addr);
        return OAL_ERR_CODE_OAM_EVT_FR_LEN_INVALID;
    }

    us_mac_frame_len = (us_mac_frame_len > WLAN_MAX_FRAME_LEN) ? WLAN_MAX_FRAME_LEN : us_mac_frame_len;

    if (oal_unlikely(en_frame_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    switch (g_oam_mng_ctx.en_output_type) {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_80211_frame_to_console(puc_mac_hdr_addr,
                                                       uc_mac_hdr_len,
                                                       puc_mac_body_addr,
                                                       us_mac_frame_len,
                                                       en_frame_direction);
            break;

        /* 输出至SDT工具 */
        case OAM_OUTPUT_TYPE_SDT:
            ul_oam_ret = oam_report_80211_frame_to_sdt(puc_user_macaddr,
                                                       puc_mac_hdr_addr,
                                                       uc_mac_hdr_len,
                                                       puc_mac_body_addr,
                                                       us_mac_frame_len,
                                                       en_frame_direction);
            break;

        default:
            ul_oam_ret = OAL_ERR_CODE_INVALID_CONFIG;
            break;
    }

    if ((ul_oam_ret != OAL_SUCC) || (ul_ret != OAL_SUCC)) {
        oam_warning_log4(0, OAM_SF_ANY,
                         "{oam_report_80211_frame:[device] en_frame_direction = %d, ul_ret = %d, ul_oam_ret = %d, return_addres = 0x%X}",
                         en_frame_direction, ul_ret, ul_oam_ret, ul_return_addr);
    }

    return ((ul_ret != OAL_SUCC) ? (ul_ret) : (ul_oam_ret));
}

/*
 * 函 数 名  : oam_report_dscr_to_console
 * 功能描述  : 将描述符信息打印到控制台（一般就是串口）
 * 输入参数  : puc_dscr_addr :描述符地址
 *             us_dscr_len   :描述符长度
 *             en_ota_type   :ota类型
 */
OAL_STATIC uint32_t oam_report_dscr_to_console(uint8_t *puc_dscr_addr,
                                               uint16_t us_dscr_len,
                                               oam_ota_type_enum_uint8 en_ota_type)
{
    if ((en_ota_type == OAM_OTA_TYPE_RX_DSCR) || (en_ota_type == OAM_OTA_TYPE_RX_DSCR_PILOT)) {
        oal_io_print("oam_report_dscr_to_console::rx_dscr info:\n\n");
    } else {
        oal_io_print("oam_report_dscr_to_console::tx_dscr info:\n\n");
    }

    oam_dump_buff_by_hex(puc_dscr_addr, us_dscr_len, 4); /* 4表示每打印4次内容则进行一次换行 */

    return OAL_SUCC;
}

/*
 * 函数 名   : oam_report_dscr_to_sdt
 * 功能描述  : 将描述符信息上报SDT
 * 输入参数  : puc_user_macaddr  :目的用户mac地址
 *             puc_dscr_addr     :描述符地址
 *             us_dscr_len       :描述符长度
 *             en_ota_type       :ota类型
 */
OAL_STATIC uint32_t oam_report_dscr_to_sdt(uint8_t *puc_user_macaddr,
                                           uint8_t *puc_dscr_addr,
                                           uint16_t us_dscr_len,
                                           oam_ota_type_enum_uint8 en_ota_type)
{
    uint32_t ul_tick;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    oam_ota_stru *pst_ota_data = NULL;
    uint32_t ul_ret;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 为上报描述符申请空间 */
    us_skb_len = us_dscr_len + sizeof(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_dscr_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - sizeof(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (oal_unlikely(pst_netbuf == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_dscr_len;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_macaddr);
    oam_set_ota_board_type(pst_ota_data);

    /* 复制数据,填写ota数据 */
    ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                      (uint32_t)us_dscr_len,
                      (const void *)puc_dscr_addr,
                      (uint32_t)us_dscr_len);
    if (ul_ret != EOK) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_report_dscr_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }

    /* 判断sdt发送消息队列是否已满，若满输出至串口 */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_dscr
 * 功能描述  : 将描述符上报sdt,包括上报tx描述符和rx描述符
 *            注意::由于rx描述符最大长度为64，tx描述符最大长度为256，所以如果参数
 *            us_dscr_len大于256的话表明传入参数错误，直接返回
 * 输入参数  : puc_user_macaddr  :目的用户mac地址
 *             puc_dscr_addr     :描述符地址
 *             us_dscr_len       :描述符长度
 *             en_ota_type       :ota类型(rx描述符或者tx描述符)，由于描述符的上报
 *             是通过ota通道，因此需要这个参数，SDT解析的时候可以显示上报的是rx描
 *             述符还是tx描述符
 */
uint32_t oam_report_dscr(uint8_t *puc_user_macaddr,
                         uint8_t *puc_dscr_addr,
                         uint16_t us_dscr_len,
                         oam_ota_type_enum_uint8 en_ota_type)
{
    uint32_t ul_ret;

    if (oal_unlikely(oal_any_null_ptr2(puc_user_macaddr, puc_dscr_addr))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((us_dscr_len > WLAN_MEM_SHARED_TX_DSCR_SIZE2) ||
        (us_dscr_len == 0)) {
        oam_dump_buff_by_hex(puc_dscr_addr, us_dscr_len, OAM_PRINT_CRLF_NUM);
        return OAL_ERR_CODE_OAM_EVT_DSCR_LEN_INVALID;
    }

    if (en_ota_type >= OAM_OTA_TYPE_BUTT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    switch (g_oam_mng_ctx.en_output_type) {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_dscr_to_console(puc_dscr_addr, us_dscr_len, en_ota_type);

            break;

        /* 输出至SDT工具 */
        case OAM_OUTPUT_TYPE_SDT:
            ul_ret = oam_report_dscr_to_sdt(puc_user_macaddr, puc_dscr_addr, us_dscr_len, en_ota_type);

            break;

        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_beacon_to_console
 * 功能描述  : 将beacon帧内容打印到控制台（一般是串口）
 * 输入参数  : puc_beacon_hdr_addr :beacon帧地址
 *             us_beacon_len       :beacon帧长度
 *             en_beacon_direction :此beacon帧是接收流程中的还是发送流程中的
 */
OAL_STATIC uint32_t oam_report_beacon_to_console(uint8_t *puc_beacon_hdr_addr,
                                                 uint16_t us_beacon_len,
                                                 oam_ota_frame_direction_type_enum_uint8 en_beacon_direction)
{
    uint8_t *puc_beacon_body_addr = NULL;
    uint16_t us_beacon_body_len;

    if (en_beacon_direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        oal_io_print("oam_report_beacon_to_console::tx_beacon info:\n");
    } else {
        oal_io_print("oam_report_beacon_to_console::rx_beacon info:\n");
    }
    oal_io_print("oam_report_beacon_to_console::beacon_header:\n");

    oam_dump_buff_by_hex(puc_beacon_hdr_addr, OAM_BEACON_HDR_LEN, OAM_PRINT_CRLF_NUM);

    puc_beacon_body_addr = puc_beacon_hdr_addr + OAM_BEACON_HDR_LEN;
    us_beacon_body_len = us_beacon_len - OAM_BEACON_HDR_LEN;

    oal_io_print("oam_report_beacon_to_console::beacon_body:\n");
    oam_dump_buff_by_hex(puc_beacon_body_addr, us_beacon_body_len, OAM_PRINT_CRLF_NUM);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_beacon_to_sdt
 * 功能描述  : 将beacon帧内容上报SDT
 */
OAL_STATIC uint32_t oam_report_beacon_to_sdt(uint8_t *puc_beacon_hdr_addr,
                                             uint8_t uc_beacon_hdr_len,
                                             uint8_t *puc_beacon_body_addr,
                                             uint16_t us_beacon_len,
                                             oam_ota_frame_direction_type_enum_uint8 en_beacon_direction)
{
    uint32_t ul_tick;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    oam_ota_stru *pst_ota_data = NULL;
    uint32_t ul_ret;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 为上报beacon帧申请空间 */
    us_skb_len = us_beacon_len + sizeof(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_beacon_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - sizeof(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = OAM_OTA_TYPE_BEACON;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = OAM_BEACON_HDR_LEN;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_beacon_len;
    pst_ota_data->st_ota_hdr.en_frame_direction = en_beacon_direction;
    oam_set_ota_board_type(pst_ota_data);

    /* 复制数据,填写ota数据 */
    ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                      (uint32_t)us_beacon_len,
                      (const void *)puc_beacon_hdr_addr,
                      (uint32_t)uc_beacon_hdr_len);

    ul_ret += memcpy_s((void *)(pst_ota_data->auc_ota_data + uc_beacon_hdr_len),
                       (uint32_t)(us_beacon_len - uc_beacon_hdr_len),
                       (const void *)puc_beacon_body_addr,
                       (uint32_t)(us_beacon_len - uc_beacon_hdr_len));
    if (ul_ret != EOK) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_report_beacon_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }

    oam_hide_mac_addr(pst_ota_data->auc_ota_data, uc_beacon_hdr_len);

    /* 下发至sdt接收队列，若队列满则串口输出 */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_beacon
 * 功能描述  : 将beacon内容上报
 */
uint32_t oam_report_beacon(uint8_t *puc_beacon_hdr_addr,
                           uint8_t uc_beacon_hdr_len,
                           uint8_t *puc_beacon_body_addr,
                           uint16_t us_beacon_len,
                           oam_ota_frame_direction_type_enum_uint8 en_beacon_direction)
{
    uint32_t ul_ret;

    if (en_beacon_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    if ((oam_ota_get_switch(OAM_OTA_SWITCH_BEACON) != OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BEACON) &&
        (oam_ota_get_switch(OAM_OTA_SWITCH_BEACON) != OAM_SDT_PRINT_BEACON_RXDSCR_TYPE_BOTH)) {
        return OAL_SUCC;
    }

    if (oal_any_null_ptr2(puc_beacon_hdr_addr, puc_beacon_body_addr)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((us_beacon_len > WLAN_MAX_FRAME_LEN) ||
        (us_beacon_len <= WLAN_MGMT_FRAME_HEADER_LEN)) {
        oam_dump_buff_by_hex(puc_beacon_hdr_addr, us_beacon_len, OAM_PRINT_CRLF_NUM);
        return OAL_ERR_CODE_OAM_EVT_FR_LEN_INVALID;
    }

    switch (g_oam_mng_ctx.en_output_type) {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_beacon_to_console(puc_beacon_hdr_addr,
                                                  us_beacon_len,
                                                  en_beacon_direction);

            break;

        /* 输出至SDT工具 */
        case OAM_OUTPUT_TYPE_SDT:
            ul_ret = oam_report_beacon_to_sdt(puc_beacon_hdr_addr,
                                              uc_beacon_hdr_len,
                                              puc_beacon_body_addr,
                                              us_beacon_len,
                                              en_beacon_direction);

            break;

        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_beacon_to_console
 * 功能描述  : 将beacon帧内容打印到控制台（一般是串口）
 */
OAL_STATIC uint32_t oam_report_eth_frame_to_console(uint8_t *puc_eth_frame_hdr_addr,
                                                    uint16_t us_eth_frame_len,
                                                    oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction)
{
    uint8_t *puc_eth_frame_body_addr = NULL;
    uint16_t us_eth_frame_body_len;

    if (en_eth_frame_direction == OAM_OTA_FRAME_DIRECTION_TYPE_TX) {
        oal_io_print("oam_report_eth_frame_to_console::recv frame from eth:\n");
    } else {
        oal_io_print("oam_report_eth_frame_to_console::report frame to eth:\n");
    }

    oal_io_print("oam_report_eth_frame_to_console::eth_frame header:\n");
    oam_dump_buff_by_hex(puc_eth_frame_hdr_addr, ETHER_HDR_LEN, OAM_PRINT_CRLF_NUM);

    puc_eth_frame_body_addr = puc_eth_frame_hdr_addr + ETHER_HDR_LEN;
    us_eth_frame_body_len = us_eth_frame_len - ETHER_HDR_LEN;

    oal_io_print("oam_report_eth_frame_to_console::eth_frame body:\n");
    oam_dump_buff_by_hex(puc_eth_frame_body_addr, us_eth_frame_body_len, OAM_PRINT_CRLF_NUM);

    return OAL_SUCC;
}

/*
 * 函 数 名  : oam_report_eth_frame_to_sdt
 * 功能描述  : 将以太网帧内容上报SDT
 * 输入参数  : puc_user_mac_addr      :目的用户mac地址
 *             puc_eth_frame_hdr_addr :以太网帧头地址
 *             us_eth_frame_len       :以太网帧长度(帧头+帧体)
 *             en_eth_frame_direction :来自以太网(tx流程)还是发往以太网(rx流程)
 */
OAL_STATIC uint32_t oam_report_eth_frame_to_sdt(uint8_t *puc_user_mac_addr,
                                                uint8_t *puc_eth_frame_hdr_addr,
                                                uint16_t us_eth_frame_len,
                                                oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction)
{
    uint32_t ul_tick;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    oam_ota_stru *pst_ota_data = NULL;
    uint32_t ul_ret;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    /* 为上报beacon帧申请空间 */
    us_skb_len = us_eth_frame_len + sizeof(oam_ota_hdr_stru);
    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) {
        us_skb_len = WLAN_SDT_NETBUF_MAX_PAYLOAD;
        us_eth_frame_len = WLAN_SDT_NETBUF_MAX_PAYLOAD - sizeof(oam_ota_hdr_stru);
    }

    pst_netbuf = oam_alloc_data2sdt(us_skb_len);
    if (oal_unlikely(pst_netbuf == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = OAM_OTA_TYPE_ETH_FRAME;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = ETHER_HDR_LEN;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_eth_frame_len;
    pst_ota_data->st_ota_hdr.en_frame_direction = en_eth_frame_direction;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_mac_addr);
    oam_set_ota_board_type(pst_ota_data);

    /* 复制数据,填写ota数据 */
    ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                      (uint32_t)us_eth_frame_len,
                      (const void *)puc_eth_frame_hdr_addr,
                      (uint32_t)us_eth_frame_len);
    if (ul_ret != EOK) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_report_eth_frame_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }
    /* 下发至sdt接收队列，若队列满则串口输出 */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_eth_frame
 * 功能描述  : 上报以太网帧，此接口只能用在两个地方:
 *             (1)发送流程，从以太网收到包之后，在wal_bridge_vap_xmit中
 *             (2)接收流程，将帧上报以太网之前，在hmac_rx_transmit_msdu_to_lan中调用
 *             oal_netif_rx之前
 * 输入参数  : puc_user_mac_addr      :目的用户mac地址
 *             us_user_idx            :用户资源池id
 *             puc_eth_frame_hdr_addr :以太网帧头地址
 *             us_eth_frame_len       :以太网帧长度(帧头+帧体)
 *             en_eth_frame_direction :来自以太网(tx流程)还是发往以太网(rx流程)
 */
uint32_t oam_report_eth_frame(uint8_t *puc_user_mac_addr,
                              uint8_t *puc_eth_frame_hdr_addr,
                              uint16_t us_eth_frame_len,
                              oam_ota_frame_direction_type_enum_uint8 en_eth_frame_direction)
{
    uint32_t ul_ret;

    if (oal_unlikely(oal_any_null_ptr2(puc_user_mac_addr, puc_eth_frame_hdr_addr))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if ((us_eth_frame_len > ETHER_MAX_LEN) ||
        (us_eth_frame_len < ETHER_HDR_LEN)) {
        oam_dump_buff_by_hex(puc_eth_frame_hdr_addr, us_eth_frame_len, OAM_PRINT_CRLF_NUM);

        return OAL_ERR_CODE_OAM_EVT_FR_LEN_INVALID;
    }

    if (oal_unlikely(en_eth_frame_direction >= OAM_OTA_FRAME_DIRECTION_TYPE_BUTT)) {
        return OAL_ERR_CODE_OAM_EVT_FRAME_DIR_INVALID;
    }

    switch (g_oam_mng_ctx.en_output_type) {
        /* 输出至控制台 */
        case OAM_OUTPUT_TYPE_CONSOLE:
            ul_ret = oam_report_eth_frame_to_console(puc_eth_frame_hdr_addr,
                                                     us_eth_frame_len,
                                                     en_eth_frame_direction);

            break;

        /* 输出至SDT工具 */
        case OAM_OUTPUT_TYPE_SDT:
            ul_ret = oam_report_eth_frame_to_sdt(puc_user_mac_addr,
                                                 puc_eth_frame_hdr_addr,
                                                 us_eth_frame_len,
                                                 en_eth_frame_direction);
            break;

        default:
            ul_ret = OAL_ERR_CODE_INVALID_CONFIG;

            break;
    }

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_netbuf_cb_to_sdt
 * 功能描述  : 将80211帧的CB字段上报SDT
 * 输入参数  : puc_user_mac_addr  :目的用户mac地址
 *             puc_netbuf_cb      :要上报的CB字段
 *             en_frame_direction :接收还是发送
 */
OAL_STATIC uint32_t oam_report_netbuf_cb_to_sdt(uint8_t *user_mac_addr,
                                                uint8_t *netbuf_cb,
                                                uint16_t cb_len,
                                                oam_ota_type_enum_uint8 en_ota_type)
{
    uint32_t ul_ret = OAL_SUCC;
#if ((_PRE_OS_VERSION_RAW != _PRE_OS_VERSION) && (_PRE_OS_VERSION_WIN32_RAW != _PRE_OS_VERSION))
    uint32_t ul_tick;
    uint16_t us_ota_data_len;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf;
    oam_ota_stru *pst_ota_data;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_skb_len = cb_len + sizeof(oam_ota_hdr_stru) + OAM_RESERVE_SKB_LEN;
    us_ota_data_len = cb_len + sizeof(oam_ota_hdr_stru);

    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_skb_len, OAL_TRUE);
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_netbuf_reserve(pst_netbuf, OAM_RESERVE_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_ota_data_len);
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_ota_type;
    pst_ota_data->st_ota_hdr.us_ota_data_len = cb_len;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, user_mac_addr);
    oam_set_ota_board_type(pst_ota_data);

    /* 复制数据,填写ota数据 */
    ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                      cb_len,
                      (const void *)netbuf_cb,
                      cb_len);
    if (ul_ret != EOK) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_report_netbuf_cb_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }
    /* 上报SDT */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);
#endif
    return ul_ret;
}

/*
 * 函 数 名  : oam_report_netbuf_cb
 * 功能描述  : 上报80211的cb字段
 * 输入参数  : puc_user_mac_addr  :目的用户mac地址
 *             puc_netbuf_cb      :要上报的CB字段
 *             en_frame_direction :接收还是发送
 */
uint32_t oam_report_netbuf_cb(uint8_t *user_mac_addr,
                              uint8_t *netbuf_cb,
                              uint16_t cb_len,
                              oam_ota_type_enum_uint8 en_ota_type)
{
    uint32_t ul_ret;

    if (oal_any_null_ptr2(user_mac_addr, netbuf_cb)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_ota_type >= OAM_OTA_TYPE_BUTT || cb_len > OAM_SKB_CB_LEN) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    ul_ret = oam_report_netbuf_cb_to_sdt(user_mac_addr, netbuf_cb, cb_len, en_ota_type);

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_dft_params_to_sdt
 * 功能描述  : 将维测信息数据上报sdt
 */
uint32_t oam_report_dft_params_to_sdt(uint8_t *puc_user_mac_addr,
                                      uint8_t *puc_param,
                                      uint16_t us_param_len,
                                      oam_ota_type_enum_uint8 en_type)
{
    uint32_t ul_tick;
    uint16_t us_ota_data_len;
    uint16_t us_skb_len;
    oal_netbuf_stru *pst_netbuf = NULL;
    oam_ota_stru *pst_ota_data = NULL;
    uint32_t ul_ret;

    if (oal_unlikely(g_oam_sdt_func_hook.p_sdt_report_data_func == NULL)) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    us_ota_data_len = us_param_len + sizeof(oam_ota_hdr_stru);
    us_skb_len = us_ota_data_len + OAM_RESERVE_SKB_LEN;

    if (us_skb_len > WLAN_SDT_NETBUF_MAX_PAYLOAD) {
        return OAL_FAIL;
    }

    pst_netbuf = oal_mem_sdt_netbuf_alloc(us_skb_len, OAL_TRUE);
    if (pst_netbuf == NULL) {
        return OAL_ERR_CODE_PTR_NULL;
    }
    oal_netbuf_reserve(pst_netbuf, OAM_RESERVE_SKB_HEADROOM_LEN);

    oal_netbuf_put(pst_netbuf, us_ota_data_len);
    pst_ota_data = (oam_ota_stru *)oal_netbuf_data(pst_netbuf);

    /* 获取系统TICK值 */
    ul_tick = (uint32_t)oal_time_get_stamp_ms();

    /* 填写ota消息头结构体 */
    pst_ota_data->st_ota_hdr.ul_tick = ul_tick;
    pst_ota_data->st_ota_hdr.en_ota_type = en_type;
    pst_ota_data->st_ota_hdr.us_ota_data_len = us_param_len;
    pst_ota_data->st_ota_hdr.uc_frame_hdr_len = 0;
    oal_set_mac_addr(pst_ota_data->st_ota_hdr.auc_user_macaddr, puc_user_mac_addr);
    oam_set_ota_board_type(pst_ota_data);

    /* 复制数据,填写ota数据 */
    ul_ret = memcpy_s((void *)pst_ota_data->auc_ota_data,
                      (uint32_t)us_param_len,
                      (const void *)puc_param,
                      (uint32_t)us_param_len);
    if (ul_ret != EOK) {
        oal_mem_sdt_netbuf_free(pst_netbuf, OAL_TRUE);
        oal_io_print("oam_report_dft_params_to_sdt:: memcpy_s failed\r\n");
        return OAL_FAIL;
    }
    /* 上报SDT */
    ul_ret = oam_report_data2sdt(pst_netbuf, OAM_DATA_TYPE_OTA, OAM_PRIMID_TYPE_OUTPUT_CONTENT);

    return ul_ret;
}

/*
 * 函 数 名  : oam_report_phy_stat
 * 功能描述  : 上报维测信息数据
 */
uint32_t oam_report_dft_params(uint8_t *puc_user_mac_addr,
                               uint8_t *puc_param,
                               uint16_t us_param_len,
                               oam_ota_type_enum_uint8 en_type)
{
    if (oal_unlikely(oal_any_null_ptr2(puc_user_mac_addr, puc_param))) {
        return OAL_ERR_CODE_PTR_NULL;
    }

    if (en_type >= OAM_OTA_TYPE_BUTT) {
        return OAL_ERR_CODE_INVALID_CONFIG;
    }

    if (us_param_len != 0) {
        return oam_report_dft_params_to_sdt(puc_user_mac_addr, puc_param, us_param_len, en_type);
    }

    return OAL_ERR_CODE_INVALID_CONFIG;
}

/*
 * 函 数 名  : oam_report_set_all_switch
 * 功能描述  : 设置所有用户帧上报的所有开关，如果是1，则上报所有类型帧的帧内容，
 *             cb字段，描述符；如果是0，则什么都不上报
 */
void oam_report_set_all_switch(oal_switch_enum_uint8 en_switch)
{
    memset_s(&g_oam_mng_ctx.st_user_track_ctx, sizeof(oam_user_track_ctx_stru),
        en_switch == OAL_SWITCH_OFF ? 0x00 : 0xFF, sizeof(oam_user_track_ctx_stru));
    memset_s(&g_oam_mng_ctx.ast_ota_ctx, sizeof(oam_ota_ctx_stru),
        en_switch == OAL_SWITCH_OFF ? OAL_SWITCH_OFF : OAL_SWITCH_ON, sizeof(oam_ota_ctx_stru));
    g_oam_mng_ctx.ast_ota_ctx.golble_switch = en_switch == OAL_SWITCH_ON ? 0xFFFFFFFF : 0x0;
}

/*
 * 函 数 名  : oam_report_backtrace
 * 功能描述  : 通过OAM上报调用栈
 */
void oam_report_backtrace(void)
{
    uint8_t *puc_buff = (uint8_t *)oal_mem_alloc_m(OAL_MEM_POOL_ID_LOCAL, OAM_REPORT_MAX_STRING_LEN, OAL_TRUE);

    if (puc_buff == NULL) {
        oam_error_log0(0, OAM_SF_CFG, "{oam_report_backtrace::pc_print_buff null.}");
        return;
    }

    if (oal_dump_stack_str(puc_buff, OAM_REPORT_MAX_STRING_LEN) > 0) {
        oam_print((int8_t *)puc_buff);
    } else {
        oam_warning_log0(0, OAM_SF_CFG, "{oam_report_backtrace::dump stack str failed.}");
    }

    oal_mem_free_m(puc_buff, OAL_TRUE);
}

/*lint -e19*/
oal_module_symbol(oam_report_backtrace);
oal_module_symbol(oam_ota_report);
oal_module_symbol(oam_report_dscr);
oal_module_symbol(oam_report_beacon);
oal_module_symbol(oam_report_eth_frame);
oal_module_symbol(oam_report_80211_frame);
oal_module_symbol(oam_ota_set_switch);
oal_module_symbol(oam_ota_get_global_switch);
oal_module_symbol(oam_ota_get_switch);
oal_module_symbol(oam_report_eth_frame_set_switch);
oal_module_symbol(oam_report_eth_frame_get_switch);
oal_module_symbol(oam_report_80211_mcast_set_switch);
oal_module_symbol(oam_report_80211_mcast_get_switch);
oal_module_symbol(oam_report_80211_ucast_set_switch);
oal_module_symbol(oam_report_80211_ucast_get_switch);
oal_module_symbol(oam_report_80211_probe_set_switch);
oal_module_symbol(oam_report_80211_probe_get_switch);
oal_module_symbol(oam_report_netbuf_cb);
oal_module_symbol(oam_report_set_all_switch);
oal_module_symbol(oam_report_dhcp_arp_get_switch);
oal_module_symbol(oam_report_dhcp_arp_set_switch);
oal_module_symbol(oam_report_dft_params);
oal_module_symbol(oam_report_data_get_global_switch);
