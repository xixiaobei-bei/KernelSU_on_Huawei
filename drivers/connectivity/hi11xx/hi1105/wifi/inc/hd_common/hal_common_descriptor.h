

#ifndef __HAL_COMMON_DESCRIPTOR_H__
#define __HAL_COMMON_DESCRIPTOR_H__

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HAL_COMMON_DESCRIPTOR_H

/*****************************************************************************/
/*                        描述符相关枚举定义                          */
/*****************************************************************************/
typedef enum {
    WLAN_PROTECT_MODE_NONE = 0,
    WLAN_PROTECT_MODE_RTS_CTS = 1,
    WLAN_PROTECT_MODE_SELF_CTS = 2,
    WLAN_PROTECT_MODE_BUTT
} wlan_protect_mode_enum;
typedef uint8_t wlan_protect_mode_enum_uint8;

typedef enum {
    WLAN_RTS_BW_MODE_PRI20M = 0,
    WLAN_RTS_BW_MODE_DUPLICATE = 1,
    WLAN_RTS_BW_MODE_BUTT,
} wlan_rts_bw_mode_enum;
typedef uint8_t wlan_rts_bw_mode_enum_uint8;
/* tx_desc_protocol_mode描述符类型,和5x共代码修改 */
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1106_DEV) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_BISHENG_DEV))
typedef enum {
    WLAN_11B_PHY_PROTOCOL_MODE = 0,                                          /* 11b CCK */
    WLAN_PHY_PROTOCOL_MODE_11B = WLAN_11B_PHY_PROTOCOL_MODE,
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE = 1,                                  /* 11g/a OFDM */
    WLAN_PHY_PROTOCOL_MODE_LEGACY_OFDM = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,
    WLAN_HT_PHY_PROTOCOL_MODE = 2,
    WLAN_PHY_PROTOCOL_MODE_HT_MIXED_MODE = WLAN_HT_PHY_PROTOCOL_MODE,        /* 11n HT Mixed */
    WLAN_HT_GREEN_PHY_PROTOCOL_MODE = 3,
    WLAN_PHY_PROTOCOL_MODE_HT_GREEN_FIELD = WLAN_HT_GREEN_PHY_PROTOCOL_MODE, /* 11n HT Green */
    WLAN_VHT_PHY_PROTOCOL_MODE = 4,                                          /* 11ac VHT */
    WLAN_PHY_PROTOCOL_MODE_VHT = WLAN_VHT_PHY_PROTOCOL_MODE,
    WLAN_PHY_PROTOCOL_MODE_BA = 5,                                           /* 11BA */
    WLAN_HE_SU_FORMAT_MODE = 8,                                              /* 11ax SU PPDU DL & UL */
    WLAN_PHY_PROTOCOL_MODE_HE_SU = WLAN_HE_SU_FORMAT_MODE,
    WLAN_HE_MU_FORMAT_MODE = 9,                                              /* 11ax MU PPDU DL */
    WLAN_PHY_PROTOCOL_MODE_HE_MU = WLAN_HE_MU_FORMAT_MODE,
    WLAN_HE_EXT_SU_FORMAT_MODE = 10,
    WLAN_PHY_PROTOCOL_MODE_HE_ER_SU = WLAN_HE_EXT_SU_FORMAT_MODE,            /* 11ax ER SU PPDU DL & UL */
    WLAN_HE_TRIG_FORMAT_MODE = 11,
    WLAN_PHY_PROTOCOL_MODE_HE_TRIGGER = WLAN_HE_TRIG_FORMAT_MODE,            /* 11ax Trigger UL TB PPDU */
    WLAN_PHY_PROTOCOL_BUTT = 12,
    WLAN_PHY_PROTOCOL_MODE_BUTT = WLAN_PHY_PROTOCOL_BUTT,
} wlan_phy_protocol_enum;
typedef uint8_t wlan_phy_protocol_enum_uint8;
typedef uint8_t wlan_phy_protocol_mode_enum_uint8;
#else
typedef enum {
    WLAN_11B_PHY_PROTOCOL_MODE = 0,                                          /* 11b CCK */
    WLAN_PHY_PROTOCOL_MODE_11B = WLAN_11B_PHY_PROTOCOL_MODE,
    WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE = 1,                                  /* 11g/a OFDM */
    WLAN_PHY_PROTOCOL_MODE_LEGACY_OFDM = WLAN_LEGACY_OFDM_PHY_PROTOCOL_MODE,
    WLAN_HT_PHY_PROTOCOL_MODE = 2,
    WLAN_PHY_PROTOCOL_MODE_HT_MIXED_MODE = WLAN_HT_PHY_PROTOCOL_MODE,        /* 11n HT Mixed */
    WLAN_VHT_PHY_PROTOCOL_MODE = 3,                                          /* 11ac VHT */
    WLAN_PHY_PROTOCOL_MODE_VHT = WLAN_VHT_PHY_PROTOCOL_MODE,
#if defined(_PRE_WLAN_FEATURE_11AX) || defined(_PRE_WLAN_FEATURE_11AX_ROM)
    WLAN_HE_SU_FORMAT_MODE = 0,                                              /* 11ax SU PPDU DL & UL */
    WLAN_PHY_PROTOCOL_MODE_HE_SU = WLAN_HE_SU_FORMAT_MODE,
    WLAN_HE_MU_FORMAT_MODE = 1,                                              /* 11ax MU PPDU DL */
    WLAN_PHY_PROTOCOL_MODE_HE_MU = WLAN_HE_MU_FORMAT_MODE,
    WLAN_HE_EXT_SU_FORMAT_MODE = 2,
    WLAN_PHY_PROTOCOL_MODE_HE_ER_SU = WLAN_HE_EXT_SU_FORMAT_MODE,            /* 11ax ER SU PPDU DL & UL */
    WLAN_HE_TRIG_FORMAT_MODE = 3,
    WLAN_PHY_PROTOCOL_MODE_HE_TRIGGER = WLAN_HE_TRIG_FORMAT_MODE,            /* 11ax Trigger UL TB PPDU */
#endif
    WLAN_PHY_PROTOCOL_BUTT = 4,
} wlan_phy_protocol_enum;
typedef uint8_t wlan_phy_protocol_enum_uint8;
typedef uint8_t wlan_phy_protocol_mode_enum_uint8;

#endif

typedef enum {
    WLAN_11B_1_M_BPS = 0,
    WLAN_11B_2_M_BPS = 1,
    WLAN_11B_5_HALF_M_BPS = 2,
    WLAN_11B_11_M_BPS = 3,

    WLAN_11B_BUTT,
} wlan_11b_rate_index_enum;
typedef uint8_t wlan_11b_rate_index_enum_uint8;

typedef enum {
    WLAN_11AG_6M_BPS = 0,
    WLAN_11AG_9M_BPS = 1,
    WLAN_11AG_12M_BPS = 2,
    WLAN_11AG_18M_BPS = 3,
    WLAN_11AG_24M_BPS = 4,
    WLAN_11AG_36M_BPS = 5,
    WLAN_11AG_48M_BPS = 6,
    WLAN_11AG_54M_BPS = 7,

    WLAN_11AG_BUTT,
} wlan_11ag_rate_value_enum;
typedef uint8_t wlan_legacy_ofdm_mcs_enum_uint8;

typedef enum {
    HAL_TX_RATE_RANK_0 = 0,
    HAL_TX_RATE_RANK_1,
    HAL_TX_RATE_RANK_2,
    HAL_TX_RATE_RANK_3,

    HAL_TX_RATE_RANK_BUTT
} hal_tx_rate_rank_enum;
typedef uint8_t hal_tx_rate_rank_enum_uint8;


typedef enum {
    HAL_RX_NEW = 0x0,
    HAL_RX_SUCCESS = 0x1,
    HAL_RX_DUP_DETECTED = 0x2,
    HAL_RX_FCS_ERROR = 0x3,
    HAL_RX_KEY_SEARCH_FAILURE = 0x4,
    HAL_RX_CCMP_MIC_FAILURE = 0x5,
    HAL_RX_ICV_FAILURE = 0x6,
    HAL_RX_TKIP_REPLAY_FAILURE = 0x7,
    HAL_RX_CCMP_REPLAY_FAILURE = 0x8,
    HAL_RX_TKIP_MIC_FAILURE = 0x9,
    HAL_RX_BIP_MIC_FAILURE = 0xA,
    HAL_RX_BIP_REPLAY_FAILURE = 0xB,
    HAL_RX_MUTI_KEY_SEARCH_FAILURE = 0xC /* 组播广播 */
} hal_rx_status_enum;
typedef uint8_t hal_rx_status_enum_uint8;

#define MAC_RX_DSCR_NEED_DROP(status) (((status) == HAL_RX_KEY_SEARCH_FAILURE) || \
    ((status) == HAL_RX_TKIP_MIC_FAILURE) || ((status) == HAL_RX_NEW) || ((status) == HAL_RX_FCS_ERROR))

/* 接收描述符队列状态 */
typedef enum {
    HAL_DSCR_QUEUE_INVALID = 0,
    HAL_DSCR_QUEUE_VALID,
    HAL_DSCR_QUEUE_SUSPENDED,

#if defined(_PRE_PRODUCT_ID_HI110X_DEV)
    HAL_DSCR_QUEUE_BUSY,
    HAL_DSCR_QUEUE_IDLE,
#endif

    HAL_DSCR_QUEUE_STATUS_BUTT
} hal_dscr_queue_status_enum;
typedef uint8_t hal_dscr_queue_status_enum_uint8;

/* 接收描述符队列号 */
typedef enum {
    HAL_RX_DSCR_NORMAL_PRI_QUEUE = 0,
    HAL_RX_DSCR_HIGH_PRI_QUEUE,
    HAL_RX_DSCR_SMALL_QUEUE,

    HAL_RX_DSCR_QUEUE_ID_BUTT
} hal_rx_dscr_queue_id_enum;
typedef uint8_t hal_rx_dscr_queue_id_enum_uint8;

/* HAL模块需要抛出的WLAN_DRX事件子类型的定义
 说明:该枚举需要和dmac_wlan_drx_event_sub_type_enum_uint8枚举一一对应 */
typedef enum {
    HAL_WLAN_DRX_EVENT_SUB_TYPE_RX, /* WLAN DRX 流程 */
    HAL_WLAN_DRX_EVENT_SUB_TYPE_BUTT
} hal_wlan_drx_event_sub_type_enum;
typedef uint8_t hal_wlan_drx_event_sub_type_enum_uint8;

/* HAL模块需要抛出的WLAN_CRX事件子类型的定义
   说明:该枚举需要和dmac_wlan_crx_event_sub_type_enum_uint8枚举一一对应 */
typedef enum {
    HAL_WLAN_CRX_EVENT_SUB_TYPE_RX, /* WLAN CRX 流程 */
#if (defined(_PRE_WLAN_FEATURE_FTM) || defined(_PRE_WLAN_FEATURE_CSI))
    HAL_EVENT_DMAC_FTM_IRQ, /* FTM中断 */
#endif
#ifdef _PRE_WLAN_FEATURE_11AX
    HAL_WLAN_CRX_EVENT_SUB_RPT_HE_ROM,
#endif
    HAL_WLAN_CRX_EVENT_SUB_TYPE_RX_PPDU,

    HAL_WLAN_CRX_EVENT_SUB_TYPE_BUTT
} hal_wlan_crx_event_sub_type_enum;
typedef uint8_t hal_wlan_crx_event_sub_type_enum_uint8;

typedef enum {
    HAL_TX_COMP_SUB_TYPE_TX,
    HAL_TX_COMP_SUB_TYPE_AL_TX,

    HAL_TX_COMP_SUB_TYPE_BUTT
} hal_tx_comp_sub_type_enum;
typedef uint8_t hal_tx_comp_sub_type_enum_uint8;

/* 芯片带宽字段定义 */
#if ((_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1105_DEV) || (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_HI1106_DEV) || \
    (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_BISHENG_DEV))
typedef enum {
    WLAN_BAND_ASSEMBLE_20M = 0,
    WLAN_BAND_ASSEMBLE_40M = 1,
    WLAN_BAND_ASSEMBLE_80M = 2,
    WLAN_BAND_ASSEMBLE_160M = 3,

    /* 以下4个废弃不用 */
    WLAN_BAND_ASSEMBLE_40M_DUP = 5,
    WLAN_BAND_ASSEMBLE_80M_DUP = 9,
    WLAN_BAND_ASSEMBLE_160M_DUP = 13,
    WLAN_BAND_ASSEMBLE_80M_80M = 15,

    WLAN_BAND_ASSEMBLE_AUTO = 16,

    WLAN_BAND_ASSEMBLE_BUTT = 17,

    /* ax协议模式下用于扩展速率 */
#ifdef  _PRE_WLAN_FEATURE_11AX_ER_SU_DCM
    WLAN_BAND_ASSEMBLE_242_RU = 6,    /* 芯片属性为0 */
    WLAN_BAND_ASSEMBLE_106_RU_UP = 7, /* 芯片属性为1 */
    WLAN_BAND_ASSEMBLE_ER_BUTT = 8,   /* 芯片属性为2,废弃不用 */
#endif
} hal_channel_assemble_enum;
typedef uint8_t hal_channel_assemble_enum_uint8;

#else
typedef enum {
    WLAN_BAND_ASSEMBLE_20M = 0,

    WLAN_BAND_ASSEMBLE_40M = 4,
    WLAN_BAND_ASSEMBLE_40M_DUP = 5,

    WLAN_BAND_ASSEMBLE_80M = 8,
    WLAN_BAND_ASSEMBLE_80M_DUP = 9,

    WLAN_BAND_ASSEMBLE_160M = 12,
    WLAN_BAND_ASSEMBLE_160M_DUP = 13,

    WLAN_BAND_ASSEMBLE_80M_80M = 15,

    WLAN_BAND_ASSEMBLE_AUTO,

    WLAN_BAND_ASSEMBLE_BUTT
} hal_channel_assemble_enum;
typedef uint8_t hal_channel_assemble_enum_uint8;

#endif

typedef enum {
    WLAN_HE_RU_SIZE_26,
    WLAN_HE_RU_SIZE_52,
    WLAN_HE_RU_SIZE_106,
    WLAN_HE_RU_SIZE_242,
    WLAN_HE_RU_SIZE_484,
    WLAN_HE_RU_SIZE_996,
    WLAN_HE_RU_SIZE_1992,

    WLAN_HE_RU_SIZE_BUTT,
} wlan_he_rusize_enum;
typedef uint8_t wlan_he_rusize_enum_uint8;

#endif /* end of hal_common_descriptor.h */
