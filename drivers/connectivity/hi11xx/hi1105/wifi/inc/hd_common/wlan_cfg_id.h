

#ifndef __WLAN_CFG_ID_H__
#define __WLAN_CFG_ID_H__


/*****************************************************************************
    配置命令 ID
    第一段  MIB 类配置
    第二段  非MIB类配置
*****************************************************************************/
typedef enum {
    /************************************************************************
        第一段 MIB 类配置
    *************************************************************************/
    /* *********************dot11smt OBJECT IDENTIFIER ::= { ieee802dot11 1 }**************************/
    /* --  dot11StationConfigTable ::= { dot11smt 1 } */
    WLAN_CFGID_STATION_ID = 0,      /* dot11StationID MacAddress, MAC地址 */
    WLAN_CFGID_BSS_TYPE = 4,        /* dot11DesiredBSSType INTEGER, 06已删除 */
    WLAN_CFGID_SSID = 5,            /* dot11DesiredSSID OCTET STRING, SIZE(0..32) */
    WLAN_CFGID_BEACON_INTERVAL = 6, /* dot11BeaconPeriod Unsigned32, 06已删除 */
    WLAN_CFGID_DTIM_PERIOD = 7,     /* dot11DTIMPeriod Unsigned32, 06已删除 */
    WLAN_CFGID_UAPSD_EN = 11,       /* dot11APSDOptionImplemented TruthValue, */
    WLAN_CFGID_SMPS_EN = 13,
    /* --  dot11PrivacyTable ::= { dot11smt 5 } */
    /************************dot11mac OBJECT IDENTIFIER ::= { ieee802dot11 2 } **************************/
    /* --  dot11OperationTable ::= { dot11mac 1 } */
    /* --  dot11CountersTable ::= { dot11mac 2 }  */
    /* --  dot11EDCATable ::= { dot11mac 4 } */
    WLAN_CFGID_EDCA_TABLE_CWMIN = 41,         /* dot11EDCATableCWmin Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_CWMAX = 42,         /* dot11EDCATableCWmax Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_AIFSN = 43,         /* dot11EDCATableAIFSN Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_TXOP_LIMIT = 44,    /* dot11EDCATableTXOPLimit Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_MSDU_LIFETIME = 45, /* dot11EDCATableMSDULifetime Unsigned32 */
    WLAN_CFGID_EDCA_TABLE_MANDATORY = 46,     /* dot11EDCATableMandatory TruthValue */

    /* --  dot11QAPEDCATable ::= { dot11mac 5 } */
    WLAN_CFGID_QEDCA_TABLE_CWMIN = 51,         /* dot11QAPEDCATableCWmin Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_CWMAX = 52,         /* dot11QAPEDCATableCWmax Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_AIFSN = 53,         /* dot11QAPEDCATableAIFSN Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_TXOP_LIMIT = 54,    /* dot11QAPEDCATableTXOPLimit Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_MSDU_LIFETIME = 55, /* dot11QAPEDCATableMSDULifetime Unsigned32 */
    WLAN_CFGID_QEDCA_TABLE_MANDATORY = 56,     /* dot11QAPEDCATableMandatory TruthValue */

    /*************************dot11res OBJECT IDENTIFIER ::= { ieee802dot11 3 } **************************/
    /*************************dot11phy OBJECT IDENTIFIER ::= { ieee802dot11 4 } *************************/
    /* --  dot11PhyHRDSSSTable ::= { dot11phy 12 } */
    WLAN_CFGID_SHORT_PREAMBLE = 60, /* dot11ShortPreambleOptionImplemented TruthValue */

    /* --  dot11PhyERPTable ::= { dot11phy 14 } */
    /* --  dot11PhyHTTable  ::= { dot11phy 15 } */
    WLAN_CFGID_SHORTGI = 80,        /* dot11ShortGIOptionInTwentyActivated TruthValue */
    WLAN_CFGID_SHORTGI_FORTY = 81,  /* dot11ShortGIOptionInFortyActivated TruthValue */
    WLAN_CFGID_CURRENT_CHANEL = 82, /* dot11CurrentChannel Unsigned32 */

    /* --  dot11PhyVHTTable  ::= { dot11phy 23 }  -- */
    WLAN_CFGID_SHORTGI_EIGHTY = 90, /* dot11VHTShortGIOptionIn80Activated TruthValue */

    /************************dot11Conformance OBJECT IDENTIFIER ::= { ieee802dot11 5 } ********************/
    /************************dot11imt         OBJECT IDENTIFIER ::= {ieee802dot11 6} **********************/
    /************************dot11MSGCF       OBJECT IDENTIFIER ::= { ieee802dot11 7} *********************/
    /************************************************************************
        第二段 非MIB 类配置
    *************************************************************************/
    /* 以下为ioctl下发的命令 */
    WLAN_CFGID_ADD_VAP = 100,   /* 创建VAP */
    WLAN_CFGID_START_VAP = 101, /* 启用VAP */
    WLAN_CFGID_DEL_VAP = 102,   /* 删除VAP */
    WLAN_CFGID_DOWN_VAP = 103,  /* 停用VAP */
    WLAN_CFGID_MODE = 105,      /* 模式: 包括协议 频段 带宽 */
#ifdef _PRE_WLAN_FEATURE_MONITOR
    WLAN_CFGID_SNIFFER = 106,      /* 抓包开关 */
    WLAN_CFGID_MONITOR_MODE = 107, /* 开启monitor模式，不进行帧过滤,关闭monitor模式，进行帧过滤 */
#endif
    WLAN_CFGID_PROT_MODE = 108, /* 保护模式 06已删除 */
    WLAN_CFGID_AUTH_MODE = 109, /* 认证模式 */
    WLAN_CFGID_NO_BEACON = 110, /* 设置no beacon能力位 06已删除 */

    WLAN_CFGID_TX_POWER = 113, /* 传输功率 */
    WLAN_CFGID_VAP_INFO = 114, /* 打印vap参数信息 */
    WLAN_CFGID_VAP_STATE_SYN = 115,
    WLAN_CFGID_BANDWIDTH = 116,
    WLAN_CFGID_CHECK_FEM_PA = 117,

    WLAN_CFGID_STOP_SCHED_SCAN = 118,
    WLAN_CFGID_STA_VAP_INFO_SYN = 119,

    /* wpa-wpa2 */
    WLAN_CFGID_ADD_KEY = 120,
    WLAN_CFGID_DEFAULT_KEY = 121,
    WLAN_CFGID_REMOVE_KEY = 122,
    WLAN_CFGID_GET_KEY = 123,

#ifdef _PRE_WLAN_FEATURE_TXOPPS
    WLAN_CFGID_STA_TXOP_AID = 124,
    WLAN_CFGID_TXOP_PS_MACHW = 125, /* 配置mac txopps使能寄存器 */
#endif
    WLAN_CFGID_CHECK_LTE_GPIO = 126,
    WLAN_CFGID_REMOVE_WEP_KEY = 131,
    WLAN_CFGID_ADD_WEP_ENTRY = 132,
    /* 认证加密模式配置 */
    WLAN_CFGID_EVENT_SWITCH = 144,       /* event模块开关 */ /* 废弃 */
    WLAN_CFGID_ETH_SWITCH = 145,         /* 以太网帧收发开关 */
    WLAN_CFGID_80211_UCAST_SWITCH = 146, /* 80211单播帧上报开关 */
    WLAN_CFGID_BTCOEX_PREEMPT_TYPE = 153,   /* preempt frame type */
    WLAN_CFGID_BTCOEX_SET_PERF_PARAM = 154, /* 调整性能参数 */
#ifdef _PRE_WLAN_FEATURE_NRCOEX
    WLAN_CFGID_NRCOEX_PRIORITY_SET = 156,
    WLAN_CFGID_NRCOEX_TEST = 157,
    WLAN_CFGID_QUERY_NRCOEX_STAT = 158,
#endif

    WLAN_CFGID_REPORT_VAP_INFO = 159, /* 上报vap信息 */

#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_PHY_STAT_EN = 161,    /* 设置phy统计使能节点,无用命令06已删除,待05同步后删除 */
    WLAN_CFGID_DBB_ENV_PARAM = 165,  /* 空口环境类参数上报或者停止上报,无用命令06已删除,待05同步后删除  */
    WLAN_CFGID_USR_QUEUE_STAT = 166, /* 用户tid队列和节能队列统计信息 */
    WLAN_CFGID_VAP_STAT = 167,       /* vap吞吐统计计信息 */
    WLAN_CFGID_ALL_STAT = 168,       /* 所有统计信息 */
#endif

    WLAN_CFGID_PHY_DEBUG_SWITCH = 169,        /* 打印接收报文的phy debug信息(inc: rssi, snr, trlr etc.)的调试开关 */
    WLAN_CFGID_80211_MCAST_SWITCH = 170,      /* 80211组播\广播帧上报开关 */
    WLAN_CFGID_PROBE_SWITCH = 171,            /* probe requese 和 probe response上报开关 */
    WLAN_CFGID_GET_MPDU_NUM = 172,            /* 获取mpdu数目 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_OTA_SWITCH = 173,      /* ota模块设置ota接收描述符上报开关 */
    WLAN_CFGID_OTA_RX_DSCR_SWITCH = 174,
    WLAN_CFGID_OTA_BEACON_SWITCH = 175,       /* 设置是否上报beacon帧的开关 */
    WLAN_CFGID_OAM_OUTPUT_TYPE = 176,         /* oam模块输出的位置 */ /* 废弃 */
    WLAN_CFGID_ADD_USER = 178,                /* 添加用户配置命令 */
    WLAN_CFGID_DEL_USER = 179,                /* 删除用户配置命令 */
    WLAN_CFGID_DEL_MULTI_USER = 180,          /* 删除用户配置命令 */
    WLAN_CFGID_AMPDU_END = 182,               /* 关闭AMPDU的配置命令 */
    WLAN_CFGID_ADDBA_REQ = 185,               /* 建立BA会话的配置命令 */
    WLAN_CFGID_DELBA_REQ = 186,               /* 删除BA会话的配置命令 */
    WLAN_CFGID_SET_LOG_LEVEL = 187,           /* 设置LOG配置级别开关 */
    WLAN_CFGID_SET_FEATURE_LOG = 188,         /* 设置日志特性开关 */
    WLAN_CFGID_SET_ALL_OTA = 189,             /* 设置所有用户帧上报的所有开关 */
    WLAN_CFGID_SET_BEACON_OFFLOAD_TEST = 190, /* Beacon offload相关的测试，仅用于测试 */

    WLAN_CFGID_SET_DHCP_ARP = 193,             /* 设置dhcp和arp上报的所有开关 */
    WLAN_CFGID_SET_RANDOM_MAC_ADDR_SCAN = 194, /* 设置随机mac addr扫描开关 */
    WLAN_CFGID_SET_RANDOM_MAC_OUI = 195,       /* 设置随机mac oui */
    WLAN_CFGID_LIST_AP = 200,                  /* 列举扫描到的AP */ /* 废弃 */
    WLAN_CFGID_LIST_STA = 201,                 /* 列举关联的STA */
    WLAN_CFGID_DUMP_ALL_RX_DSCR = 203,         /* 打印所有的接收描述符 */ /* 废弃 */
    WLAN_CFGID_START_SCAN = 204,               /* 触发初始扫描 */
    WLAN_CFGID_START_JOIN = 205,               /* 触发加入认证并关联 */
    WLAN_CFGID_START_DEAUTH = 206,             /* 触发去认证 */
    WLAN_CFGID_DUMP_TIEMR = 207,               /* 打印所有timer的维测信息 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_KICK_USER = 208,                /* 去关联1个用户 */

    WLAN_CFGID_PAUSE_TID = 209,                /* 暂停指定用户的指定tid */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_SET_USER_VIP = 210,             /* 设置用户为VIP用户 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_SET_VAP_HOST = 211,             /* 设置VAP是否为host vap */ /* 03 05 rom化保留, 废弃 */

    WLAN_CFGID_AMPDU_TX_ON = 212,              /* 开启或关闭ampdu发送功能 */
    WLAN_CFGID_AMSDU_TX_ON = 213,              /* 开启或关闭ampdu发送功能 */
    WLAN_CFGID_SEND_BAR = 215,                 /* 指定用户的指定tid发送bar 06已删除 */
    WLAN_CFGID_LIST_CHAN = 217,                /* 列举支持的管制域信道 */
    WLAN_CFGID_REGDOMAIN_PWR = 218,            /* 设置管制域功率 */ /* 废弃 */ /* 05 rom化保留 */
    WLAN_CFGID_TXBF_SWITCH = 219,              /* 开启或关闭TXBF发送/接收功能 */
    WLAN_CFGID_TXBF_MIB_UPDATE = 220,          /* 刷新txbf mib能力 */
    WLAN_CFGID_FRAG_THRESHOLD_REG = 221,       /* 设置分片门限长度 */
    WLAN_CFGID_SET_VOWIFI_KEEP_ALIVE = 222,    /* vowifi nat keep alive */

    WLAN_CFGID_RESUME_RX_INTR_FIFO = 226, /* 是否使能恢复RX INTR FIFO开关 */
    WLAN_CFGID_SET_PSM_PARAM = 231, /* STA 低功耗tbtt offset/listen interval配置 */
    WLAN_CFGID_SET_STA_PM_ON = 232, /* STA低功耗开关接口 */

    WLAN_CFGID_DUMP_BA_BITMAP = 241, /* 发指定个数的报文 */ /* 03 05 rom化保留, 废弃 */

    WLAN_CFGID_SHOW_PROFILING = 245,
    WLAN_CFGID_AMSDU_AMPDU_SWITCH = 246,
    WLAN_CFGID_COUNTRY = 247,             /* 设置国家码管制域信息 */
    WLAN_CFGID_TID = 248,                 /* 读取最新接收到数据帧的TID */
    WLAN_CFGID_RESET_HW = 249,            /* Reset mac&phy 06已删除 */

    WLAN_CFGID_UAPSD_DEBUG = 250,         /* UAPSD维测信息 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_DUMP_RX_DSCR = 251,        /* dump接收描述队列 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_DUMP_TX_DSCR = 252,        /* dump发送描述符队列 */ /* 03 05 rom化保留, 废弃 */

    WLAN_CFGID_DUMP_MEMORY = 253,         /* dump内存 */
    WLAN_CFGID_ALG_PARAM = 254,           /* 算法参数配置 */
    WLAN_CFGID_BEACON_CHAIN_SWITCH = 255, /* 设置beacon帧发送策略，0为关闭双路轮流发送，1为开启 */
    WLAN_CFGID_PROTOCOL_DBG = 256,        /* 设置协议中相关的带宽等信息参数 */
    WLAN_CFGID_ALG_ENTRY = 257,        /* 算法参数配置新框架入口 */
    WLAN_CFGID_CALI_ENTRY = 258,       /* 校准参数配置入口 */
    WLAN_CFGID_2040_COEXISTENCE = 260,    /* 设置20/40共存使能，0: 20/40共存使能，1: 20/40共存不使能 */
    WLAN_CFGID_RX_FCS_INFO = 261,         /* 打印接收帧FCS解析信息 */
    WLAN_CFGID_SEND_FRAME = 262,          /* 指定用户发送控制帧或者管理帧 */

#ifdef _PRE_WLAN_FEATURE_PMF
    WLAN_CFGID_PMF_ENABLE = 264, /* 配置强制使能pmf */
#endif
    WLAN_CFGID_LPM_SOC_MODE = 273, /* soc低功耗测试模式设置 */
    WLAN_CFGID_ACS_CONFIG = 286,       /* ACS命令 */
    WLAN_CFGID_SCAN_ABORT = 287,       /* 扫描终止 */
    /* 以下命令为cfg80211下发的命令(通过内核) */
    WLAN_CFGID_CFG80211_START_SCHED_SCAN = 288, /* 内核下发启动PNO调度扫描命令 */
    WLAN_CFGID_CFG80211_STOP_SCHED_SCAN = 289,  /* 内核下发停止PNO调度扫描命令 */
    WLAN_CFGID_CFG80211_START_SCAN = 290,       /* 内核下发启动扫描命令 */
    WLAN_CFGID_CFG80211_START_CONNECT = 291,    /* 内核下发启动JOIN(connect)命令 */
    WLAN_CFGID_CFG80211_SET_CHANNEL = 292,      /* 内核下发设置信道命令 */
    WLAN_CFGID_CFG80211_SET_WIPHY_PARAMS = 293, /* 内核下发设置wiphy 结构体命令 */
    WLAN_CFGID_CFG80211_CONFIG_BEACON = 295,    /* 内核下发设置VAP信息 */
    WLAN_CFGID_ALG = 296,                       /* 算法配置命令 */
    WLAN_CFGID_ACS_PARAM = 297,                 /* ACS命令 */
    WLAN_CFGID_ALG_CFG = 298,                   /* alg_cfg算法命令 */
    WLAN_CFGID_CALI_CFG = 299,                  /* 校准配置命令 */
    WLAN_CFGID_RADARTOOL = 310,                 /* DFS配置命令 */ /* 03 05 rom化保留, 废弃 */

    /* BEGIN:以下命令为开源APP 程序下发的私有命令 */
    WLAN_CFGID_GET_ASSOC_REQ_IE = 311,   /* hostapd 获取ASSOC REQ 帧信息 */
    WLAN_CFGID_SET_WPS_IE = 312,         /* hostapd 设置WPS 信息元素到VAP */
    WLAN_CFGID_SET_RTS_THRESHHOLD = 313, /* hostapd 设置RTS 门限 */
    WLAN_CFGID_SET_WPS_P2P_IE = 314,     /* wpa_supplicant 设置WPS/P2P 信息元素到VAP */
    WLAN_CFGID_CHAN_STAT = 315,

    /* END:以下命令为开源APP 程序下发的私有命令 */
    WLAN_CFGID_ADJUST_PPM = 319,     /* 设置PPM校准算法 */ /* 废弃 */
    WLAN_CFGID_USER_INFO = 320,      /* 用户信息 */
    WLAN_CFGID_SET_DSCR = 321,       /* 配置用户信息 */
    WLAN_CFGID_SET_RATE = 322,       /* 设置non-HT速率 */
    WLAN_CFGID_SET_MCS = 323,        /* 设置HT速率 */
    WLAN_CFGID_SET_MCSAC = 324,      /* 设置VHT速率 */
    WLAN_CFGID_SET_NSS = 325,        /* 设置空间流个数 */
    WLAN_CFGID_SET_RFCH = 326,       /* 设置发射通道 */
    WLAN_CFGID_SET_BW = 327,         /* 设置带宽 */
    WLAN_CFGID_SET_BAND = 328,       /* 设置频段 */
    WLAN_CFGID_SET_ALWAYS_RX = 329,  /* 设置常发模式 */
    WLAN_CFGID_GET_THRUPUT = 330,    /* 获取芯片吞吐量信息 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_REG_INFO = 332,       /* 寄存器地址信息 */
    WLAN_CFGID_REG_WRITE = 333,      /* 写入寄存器信息 */
    WLAN_CFGID_OPEN_ADDR4 = 336,     /* mac头为4地址 */
    WLAN_CFGID_WMM_SWITCH = 338,     /* 打开或者关闭wmm */
    WLAN_CFGID_HIDE_SSID = 339,      /* 打开或者关闭隐藏ssid */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_CHIP_TEST_OPEN = 340, /* 打开芯片验证开关 */
    WLAN_CFGID_SET_ALWAYS_TX = 343, /* 设置1102常发模式 */

    WLAN_CFGID_PROTECTION_UPDATE_STA_USER = 348, /* 增加用户更新保护模式 */
    WLAN_CFGID_40M_INTOL_UPDATE = 349, /* 增加host改变带宽事件 */
    WLAN_CFGID_SET_MCSAX = 350,    /* 设置HE速率 */
    WLAN_CFGID_SET_MCSAX_ER = 351, /* 设置HE ER速率 */
    WLAN_CFGID_SET_RU_INDEX = 352, /* 设置RU index */
#ifdef _PRE_WLAN_FEATURE_OFFLOAD_FLOWCTL
    WLAN_CFGID_SET_FLOWCTL_PARAM = 353, /* 设置流控相关参数 */
    WLAN_CFGID_GET_FLOWCTL_STAT = 354,  /* 获取流控相关状态信息 */
    WLAN_CFGID_GET_HIPKT_STAT = 355,    /* 获取高优先级报文的统计情况 06已删除 */
#endif
    WLAN_CFGID_SET_11AX_CLOSE_HTC       = 356,
    WLAN_CFGID_SET_11AX_WIFI6_BLACKLIST = 357,
    WALN_CFGID_SET_P2P_SCENES           = 358, /* 设置P2P业务场景 06已删除 */
    WLAN_CFGID_SET_CLEAR_11N_BLACKLIST  = 359,
    WALN_CFGID_SET_CHANGE_GO_CHANNEL    = 360,

    WLAN_CFGID_SET_AUTO_PROTECTION = 362, /* 设置auto protection开关 06已删除 */

    WLAN_CFGID_SEND_2040_COEXT = 370, /* 发送20/40共存管理帧 */
    WLAN_CFGID_2040_COEXT_INFO = 371, /* VAP的所有20/40共存信息 */ /* 废弃 */

#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_CFGID_FTM_DBG = 374, /* FTM调试命令 */
#endif

    WLAN_CFGID_GET_VERSION = 375, /* 获取版本 */
#ifdef _PRE_WLAN_FEATURE_OPMODE_NOTIFY
    WLAN_CFGID_SET_OPMODE_NOTIFY = 400, /* 设置工作模式通知能力 */ /* 03 05 rom化保留, 废弃 */
    WLAN_CFGID_GET_USER_RSSBW = 401, /* 获取带宽 06已删除 */
#endif
#ifdef _PRE_WLAN_FEATURE_M2S
    WLAN_CFGID_SET_M2S_SWITCH = 402,    /* 设置mimo-siso切换 */
    WLAN_CFGID_SET_M2S_MSS = 403,       /* 上层下发MSS切换 */
    WLAN_CFGID_SET_M2S_BLACKLIST = 404, /* 上层下发m2s黑名单 */
    WLAN_CFGID_M2S_SWITCH_COMP = 405,   /* m2s切换完成信息上报host */
#endif
#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    WLAN_CFGID_ANT_TAS_SWITCH_RSSI_NOTIFY = 406, /* TAS/默认态切换完成RSSI上报host */
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    WLAN_CFGID_HID2D_SEQNUM_REPORT = 407, /* HID2D丢包序列号上报host */
#endif
    WLAN_CFGID_AMSDU_SWITCH_REPORT = 408, /* dmac是否需要打开amsdu信息通知host */
    WLAN_CFGID_SET_VAP_NSS = 410, /* 设置VAP的接收空间流  */ /* 03 05 rom化保留, 废弃 */

#ifdef _PRE_WLAN_DFT_REG
    WLAN_CFGID_DUMP_REG = 420,
#endif

#ifdef _PRE_WLAN_FEATURE_PSD_ANALYSIS
    WLAN_CFGID_SET_PSD = 425, /* PSD使能 */
    WLAN_CFGID_CFG_PSD = 426, /* PSD配置 */
#endif
#ifdef _PRE_WLAN_FEATURE_CSI
    WLAN_CFGID_SET_CSI = 427, /* CSI使能 */
    WLAN_CFGID_SET_CSI_BLACK_LIST = 428, /* 获取CSI TA MAC黑名单 */
#endif

    WLAN_CFGID_ADD_BLACK_LIST = 440, /* 添加黑名单 */
    WLAN_CFGID_DEL_BLACK_LIST = 441, /* 删除黑名单 */
    WLAN_CFGID_BLACKLIST_MODE = 442,
    WLAN_CFGID_BLACKLIST_SHOW = 443,

    WLAN_CFGID_ADD_BLACK_LIST_ONLY = 458, /* 添加到黑名单,不做check user和删除user行为 */

    WLAN_CFGID_WIFI_EN = 460,                /* WIFI使能开关 */
    WLAN_CFGID_PM_INFO = 461,                /* PM信息 */
    WLAN_CFGID_PM_EN = 462,                  /* PM开关 */
    WLAN_CFGID_CFG80211_SET_MIB_BY_BW = 463, /* 根据带宽更新相关mib值:GI & bw_cap */
    WLAN_CFGID_SET_AGGR_NUM = 475, /* 设置聚合个数 */
    WLAN_CFGID_FREQ_ADJUST = 476,  /* 频偏调整 */

    WLAN_CFGID_SET_STBC_CAP = 477, /* 设置STBC能力 */
    WLAN_CFGID_SET_LDPC_CAP = 478, /* 设置LDPC能力 */

    WLAN_CFGID_VAP_CLASSIFY_EN = 479,  /* VAP流等级开关 */
    WLAN_CFGID_VAP_CLASSIFY_TID = 480, /* VAP流等级 */

    WLAN_CFGID_RESET_HW_OPERATE = 481,    /* Reset 同步 */
    WLAN_CFGID_SCAN_TEST = 482,           /* 扫描模块测试命令 */
    WLAN_CFGID_QUERY_STATION_STATS = 483, /* 信息上报查询命令 */
    WLAN_CFGID_CONNECT_REQ = 484,         /*  */
    WLAN_CFIGD_BGSCAN_ENABLE = 485,       /* 禁用背景扫描命令 */
    WLAN_CFGID_QUERY_RSSI = 486,          /* 查询用户dmac rssi信息 06已删除 */
    WLAN_CFGID_QUERY_RATE = 487,          /* 查询用户当前使用的tx rx phy rate 06已删除 */
    WLAN_CFGID_QUERY_PSM_STAT = 488,      /* 查询低功耗统计数据 */

    WLAN_CFGID_CFG80211_REMAIN_ON_CHANNEL = 490,        /* 停止在指定信道 */
    WLAN_CFGID_CFG80211_CANCEL_REMAIN_ON_CHANNEL = 491, /* 取消停止在指定信道 */

    WLAN_CFGID_DEVICE_MEM_LEAK = 492, /* device 侧mem leak打印接口 */ /* 废弃 */
    WLAN_CFGID_DEVICE_MEM_INFO = 493, /* device 侧mem 使用情况打印接口 */

    WLAN_CFGID_SET_PS_MODE = 494, /* 设置pspoll mode */
    WLAN_CFGID_SHOW_PS_INFO = 495, /* PSM状态查看 */

    WLAN_CFGID_SET_UAPSD_PARA = 496, /* 设置UAPSD参数 */

    WLAN_CFGID_CFG80211_MGMT_TX = 498, /* 发送管理帧 */
    WLAN_CFGID_CFG80211_MGMT_TX_STATUS = 499,
    WLAN_CFGID_THRUPUT_INFO = 501, /* 吞吐量数据由dmac同步到hmac */
    WLAN_CFGID_COUNTRY_FOR_DFS = 503, /* 设置国家码管制域dfs信息 */
    WLAN_CFGID_SET_P2P_PS_OPS = 504,  /* 配置P2P OPS节能 */
    WLAN_CFGID_SET_P2P_PS_NOA = 505,  /* 配置P2P NOA节能 */
    WLAN_CFGID_SET_P2P_PS_STAT = 506, /* 配置P2P 节能统计 */
#ifdef _PRE_WLAN_FEATURE_HS20
    WLAN_CFGID_SET_QOS_MAP = 507, /* 配置HotSpot 2.0 QoSMap参数 */
#endif
    WLAN_CFGID_SET_P2P_MIRACAST_STATUS = 508, /* 配置P2P投屏状态 06已删除 */

    WLAN_CFGID_UAPSD_UPDATE = 510,

    WLAN_CFGID_SDIO_FLOWCTRL = 523,
    WLAN_CFGID_NSS = 524, /* 空间流信息的同步 */

    WLAN_CFGID_ENABLE_ARP_OFFLOAD = 526,   /* arp offload的配置事件 06已删除 */
    WLAN_CFGID_SET_IP_ADDR = 527,          /* IPv4/IPv6地址的配置事件 */
    WLAN_CFGID_SHOW_ARPOFFLOAD_INFO = 528, /* 打印device侧的IP地址 06已删除 */

    WLAN_CFGID_CFG_VAP_H2D = 529,   /* 配置vap下发device事件 */
    WLAN_CFGID_HOST_DEV_INIT = 530, /* 下发初始化host dev init事件 */
    WLAN_CFGID_HOST_DEV_EXIT = 531, /* 下发去初始化host dev exit事件 */

    WLAN_CFGID_AMPDU_MMSS = 532,

#ifdef _PRE_WLAN_TCP_OPT
    WLAN_CFGID_GET_TCP_ACK_STREAM_INFO = 533, /* 显示TCP ACK 过滤统计值 */
    WLAN_CFGID_TX_TCP_ACK_OPT_ENALBE = 534,   /* 设置发送TCP ACK优化使能 */
    WLAN_CFGID_RX_TCP_ACK_OPT_ENALBE = 535,   /* 设置接收TCP ACK优化使能 */
    WLAN_CFGID_TX_TCP_ACK_OPT_LIMIT = 536,    /* 设置发送TCP ACK LIMIT */
    WLAN_CFGID_RX_TCP_ACK_OPT_LIMIT = 537,    /* 设置接收TCP ACK LIMIT  */
#endif

    WLAN_CFGID_SET_MAX_USER = 538, /* 设置最大用户数 */
    WLAN_CFGID_GET_STA_LIST = 539, /* 设置最大用户数 */
    WLAN_CFGID_BTCOEX_RX_DELBA_TRIGGER = 540,
#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_SET_PERFORMANCE_LOG_SWITCH = 541, /* 设置性能打印控制开关 */ /* 废弃 */
#endif

#ifdef _PRE_WLAN_FEATURE_WAPI
    WLAN_CFGID_WAPI_INFO = 542,
    WLAN_CFGID_ADD_WAPI_KEY = 543,
#endif

#ifdef _PRE_WLAN_DFT_STAT
    WLAN_CFGID_QUERY_ANI = 544, /* 查询VAP抗干扰参数 06已删除 */
#endif

    WLAN_CFGID_ROAM_ENABLE = 550, /* 漫游使能 */
    WLAN_CFGID_ROAM_START = 553,  /* 漫游开始 */
    WLAN_CFGID_SET_ROAMING_MODE = 555,
    WLAN_CFGID_SET_ROAM_TRIGGER = 556,
    WLAN_CFGID_ROAM_HMAC_SYNC_DMAC = 557,
    WLAN_CFGID_SET_FT_IES = 558,
    WLAN_CFGID_ROAM_NOTIFY_STATE = 559,
    WLAN_CFGID_ROAM_SUCC_H2D_SYNC = 560,
    WLAN_CFGID_CFG80211_SET_PMKSA = 561,     /* 设置PMK缓存 */
    WLAN_CFGID_CFG80211_DEL_PMKSA = 562,     /* 删除PMK缓存 */
    WLAN_CFGID_CFG80211_FLUSH_PMKSA = 563,   /* 清空PMK缓存 */
    WLAN_CFGID_CFG80211_EXTERNAL_AUTH = 564, /* 触发SAE认证 */

    WLAN_CFGID_SET_PM_SWITCH = 570, /* 全局低功耗使能去使能 */
    WLAN_CFGID_SET_DEVICE_FREQ = 571, /* 设置device主频 */
    WLAN_CFGID_SET_POWER_TEST = 572,      /* 功耗测试模式使能 06已删除 */
    WLAN_CFGID_SET_DEVICE_PKT_STAT = 573, /* 设置device收帧统计 */

    WLAN_CFGID_2040BSS_ENABLE = 574, /* 20/40 bss判断使能开关 */
    WLAN_CFGID_DESTROY_VAP = 575,

    WLAN_CFGID_SET_ANT = 576, /* 设置使用的天线 */
    WLAN_CFGID_GET_ANT = 577, /* 获取天线状态 06已删除 */

    WLAN_CFGID_GREEN_AP_EN = 578,

    WLAN_CFGID_SET_TX_AMPDU_TYPE = 583,
    WLAN_CFGID_SET_RX_AMPDU_AMSDU = 584,
    WLAN_CFGID_SET_ADDBA_RSP_BUFFER = 585,
    WLAN_CFGID_SET_SK_PACING_SHIFT = 586,
    WLAN_CFGID_SET_TRX_STAT_LOG = 587,
    WLAN_CFGID_MIMO_COMPATIBILITY = 588, /* mimo兼容性AP */
    WLAN_CFGID_DATA_COLLECT = 589, /* 数采开关，模式配置 */

    /* HISI-CUSTOMIZE */
    WLAN_CFGID_SET_CUS_DYN_CALI_PARAM = 605, /* 动态校准参数定制化 06已删除 */
    WLAN_CFGID_SET_ALL_LOG_LEVEL = 606,      /* 配置所有vap log level 06已删除 */
    WLAN_CFGID_SET_CUS_RF = 607,             /* RF定制化 06已删除 */
    WLAN_CFGID_SET_CUS_DTS_CALI = 608,       /* DTS校准定制化 06已删除 */
    WLAN_CFGID_SET_CUS_NVRAM_PARAM = 609,    /* NVRAM参数定制化 06已删除 */
    /* HISI-CUSTOMIZE INFOS */
    WLAN_CFGID_SHOW_DEV_CUSTOMIZE_INFOS = 610, /* show device customize info 06已删除 */

#ifdef _PRE_WLAN_FEATURE_DFR
    WLAN_CFGID_TEST_DFR_START = 611,
#endif  // _PRE_WLAN_FEATURE_DFR

    WLAN_CFGID_WFA_CFG_AIFSN = 612,
    WLAN_CFGID_WFA_CFG_CW = 613,

#ifdef _PRE_WLAN_FEATURE_TX_CLASSIFY_LAN_TO_WLAN
    WLAN_CFGID_TX_CLASSIFY_LAN_TO_WLAN_SWITCH = 614, /* 设置业务识别功能开关 */
#endif
    WLAN_CFGID_REDUCE_SAR = 615, /* 通过降低发射功率来降低SAR */
    WLAN_CFGID_DBB_SCALING_AMEND = 616, /* 调整dbb scaling值 */
    WLAN_CFGID_TAS_PWR_CTRL = 620, /* TAS功率控制发射 */
    WLAN_CFGID_SET_VENDOR_IE = 630, /* hostapd 设置IE元素到VAP */

#ifdef _PRE_WLAN_FEATURE_11K
    WLAN_CFGID_SEND_NEIGHBOR_REQ = 644,
    WLAN_CFGID_REQ_SAVE_BSS_INFO = 645,
    WLAN_CFGID_BCN_TABLE_SWITCH = 646,
#endif
    WLAN_CFGID_VOE_ENABLE = 647,

    WLAN_CFGID_SET_TXRX_CHAIN = 648,   /* 设置收发通道 */
    WLAN_CFGID_SET_2G_TXRX_PATH = 649, /* 设置2G通路 */

    WLAN_CFGID_VENDOR_CMD_GET_CHANNEL_LIST = 650, /* 1102 vendor cmd, 获取信道列表 */

    WLAN_CFGID_VOWIFI_INFO = 653,   /* host->device，VoWiFi模式配置; device->host,上报切换VoWiFi/VoLTE */
    WLAN_CFGID_VOWIFI_REPORT = 654, /* device->host,上报切换VoWiFi/VoLTE */

    WLAN_CFGID_SET_ALWAYS_TX_HW_CFG = 657,    /* 废弃 */ /* 05 rom化保留 */

    WLAN_CFGID_NARROW_BW = 659,

    WLAN_CFGID_SET_ALWAYS_TX_NUM = 660, /* 设置1102常发数目 */

#ifdef _PRE_WLAN_FEATURE_TAS_ANT_SWITCH
    WLAN_CFGID_TAS_RSSI_ACCESS = 661, /* 天线RSSI测量 */
#endif

    WLAN_CFGID_DC_STATUS = 667, /* DC场景状态识别 */

#ifdef _PRE_WLAN_FEATURE_WMMAC
    WLAN_CFGID_ADDTS_REQ = 671,    /* 发送ADDTS REQ的配置命令 */
    WLAN_CFGID_DELTS = 672,        /* 发送DELTS的配置命令 */
    WLAN_CFGID_REASSOC_REQ = 673,  /* 发送reassoc req 的配置命令 */ /* 废弃 */
    WLAN_CFGID_WMMAC_SWITCH = 674, /* 设置WMMAC SWITCH开关的配置命令 */
#endif
    WLAN_CFGID_SET_BW_FIXED = 681, /* 设置标记用户将带宽模式设置为固定的模式 06已删除 */
    WLAN_CFGID_SET_TX_POW = 682,   /* 设置发送功率 */

    WLAN_CFGID_CLR_BLACK_LIST = 693,

#if defined(_PRE_WLAN_FEATURE_11V_ENABLE)
    WLAN_CFGID_11V_BSST_SWITCH = 701, /* 禁用|开启11v bss transition特性功能开关 */
    WLAN_CFGID_11V_TX_QUERY = 702,    /* 11v 触发STA发送Query帧 */
#endif
    WLAN_CFGID_SET_MLME = 707,

    WLAN_CFGID_RSSI_LIMIT_CFG = 719, /* 03, 05 rom化保留，废弃 */
    WLAN_CFGID_GET_BCAST_RATE = 720,
    WLAN_CFGID_GET_ALL_STA_INFO = 722,    /* 获取所有已关联sta信息 */
    WLAN_CFGID_GET_STA_INFO_BY_MAC = 723, /* 获取某个指定关联sta信息 */

    WLAN_CFGID_DPD = 728,       /* DPD配置命令 */
    WLAN_CFGID_GET_2040BSS_SW = 730, /* 获取20/40 bss的使能开关 */

    WLAN_CFGID_FBT_GET_STA_11H_ABILITY = 734, /* hilink获取sta 11h能力信息 */

    WLAN_CFGID_QUERY_PSST = 752, /* 查询用户省功耗状态 06已删除 */

    WLAN_CFGID_CALI_POWER = 755,
    WLAN_CFGID_SET_POLYNOMIAL_PARA = 756,
    WLAN_CFGID_GET_STA_INFO = 757,
    WLAN_CFGID_SET_DEVICE_MEM_FREE_STAT = 759, /* 设置device内存统计 */
    WLAN_CFGID_GET_PPM = 760,                  /* 获取ppm值 */
    WLAN_CFGID_GET_CALI_POWER = 761,           /* 获取校准功率值 */
    WLAN_CFGID_GET_DIEID = 762,

    WLAN_CFGID_VAP_WMM_SWITCH = 766, /* 打开或者关闭vap级别wmm开关 */

    WLAN_CFGID_PM_DEBUG_SWITCH = 768, /* 低功耗调试命令 */
    WLAN_CFGID_GET_BG_NOISE = 769, /* 获取通道底噪 */

    WLAN_CFGID_IP_FILTER = 770, /* 配置IP端口过滤的命令 */

    WLAN_CFGID_GET_UPC_PARA = 771, /* 获取产线校准upc值 */
    WLAN_CFGID_SET_UPC_PARA = 772, /* 写产线校准upc值 */
    WLAN_CFGID_SET_LOAD_MODE = 777, /* 设置负载模式 */

#ifndef CONFIG_HAS_EARLYSUSPEND
    WLAN_CFGID_SET_SUSPEND_MODE = 778, /* 设置亮暗屏状态 */
#endif

#ifdef _PRE_WLAN_FEATURE_APF
    WLAN_CFGID_SET_APF_FILTER = 779, /* 设置APF PROGRAM */
#endif
    WLAN_CFGID_REMOVE_APP_IE = 780, /* 移除某个上层配置的IE */

    WLAN_CFGID_SET_DFS_MODE = 781, /* dfs功能的寄存器调整接口 */
    WLAN_CFGID_SET_RADAR = 782,    /* 配置雷达事件 */

    WLAN_CFGID_MIMO_BLACKLIST = 783,
#ifdef _PRE_WLAN_FEATURE_HID2D_TX_DROP
    WLAN_CFGID_SET_HID2D_STATE = 784, /* hid2d功能的启动关闭接口 */
    WLAN_CFGID_HID2D_DEBUG_MODE = 785,
    WLAN_CFGID_HID2D_DEBUG_SWITCH = 786,
#endif
    WLAN_CFGID_SET_ADC_DAC_FREQ = 790, /* dfs功能的寄存器调整接口 */

#ifdef _PRE_WLAN_FEATURE_TCP_ACK_BUFFER
    WLAN_CFGID_TCP_ACK_BUF = 791,
#endif

    WLAN_CFGID_FORCE_STOP_FILTER = 792,

    WLAN_CFGID_GET_DBDC_INFO = 793,
#ifdef _PRE_WLAN_DELAY_STATISTIC
    WLAN_CFGID_PKT_TIME_SWITCH = 794,
#endif

    WLAN_CFGID_SET_OWE = 795, /* 设置APUT模式下支持的OWE group */

    WLAN_CFGID_SET_MAC_FREQ = 798, /* MAC频率的寄存器调整接口 */

#ifdef _PRE_WLAN_FEATURE_11AX
    WLAN_CFGID_11AX_DEBUG = 799,
    WLAN_CFGID_TWT_SETUP_REQ = 800,    /* 建立TWT会话的配置命令 */
    WLAN_CFGID_TWT_TEARDOWN_REQ = 801, /*  删尝TWT会话的配置命令 */
#endif

    WLAN_CFGID_SPEC_FEMLP_EN = 802, /* 特殊场景fem低功耗开启命令 */
    WLAN_CFGID_SOFTAP_MIMO_MODE = 803,

#ifdef _PRE_WLAN_RX_LISTEN_POWER_SAVING
    WLAN_CFGID_RX_LISTEN_PS_SWITCH = 804, /* 终端下发rx listen ps命令开关 */
#endif

#ifdef _PRE_WLAN_FEATURE_PMF
    WLAN_CFGID_PMF_CAP = 805, /* 配置PMF能力 */
#endif

    WLAN_CFGID_ASSIGNED_FILTER = 806, /* 配置指定数据帧过滤命令 */

    WLAN_CFGID_SET_ALWAYS_TX_AGGR_NUM = 807, /* 设置常发聚合个数 */

    WLAN_CFGID_MAC_TX_COMMON_REPORT = 809, /* mac tx common report命令 */
    WLAN_CFGID_MAC_RX_COMMON_REPORT = 810, /* mac rx common report命令 */
    WLAN_CFGID_COMMON_DEBUG = 811,         /* 1105 common debug命令 */

    WLAN_CFGID_SET_FASTSLEEP_PARA = 813, /* 配置fastsleep相关参数 */
    WLAN_CFGID_SET_RU_TEST = 814,
    WLAN_CFGID_SET_WARNING_MODE = 815,
    WLAN_CFGID_SET_CHR_MODE = 816,

#ifdef _PRE_WLAN_FEATURE_M2S
    WLAN_CFGID_SET_M2S_MODEM = 817,
#endif
    WLAN_CFGID_SET_LINKLOSS_DISABLE_CSA    = 818,  /* 配置linkloss是否禁止csa切换 */
    WLAN_CFGID_LOG_DEBUG = 819,
    WLAN_CFGID_TRIGGER_REASSOC = 820,    /* 触发重关联 */

#ifdef _PRE_WLAN_RR_PERFORMENCE_DEBUG
    WLAN_CFGID_ENABLE_RR_TIMEINFO = 821,    /* RR性能检测打点命令开关 */
#endif
#ifdef _PRE_WLAN_FEATURE_NAN
    WLAN_CFGID_NAN = 822,
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
#ifdef _PRE_WLAN_FEATURE_HID2D_PRESENTATION
    WLAN_CFGID_HID2D_SWITCH_CHAN = 823,  /* 触发GO 通过CSA 机制切换信道 */
    WLAN_CFGID_HID2D_SCAN_CHAN = 824,  /* 发布会apk下发扫描命令 */
    WLAN_CFGID_HID2D_PRESENTATION_MODE = 825, /* 设置发布会场景 */
#endif
    WLAN_CFGID_HID2D_ACS_MODE = 826, /* HiD2D自动信道切换总开关 */
    WLAN_CFGID_HID2D_ACS_REPORT = 827, /* HiD2D自动信道切换dmac上报信息 */
    WLAN_CFGID_HID2D_LINK_MEAS = 828, /* HiD2D信道探测命令 */
#endif
#ifdef _PRE_WLAN_FEATURE_FTM
    WLAN_CFGID_RTT_CONFIG = 829,
    WLAN_CFGID_RTT_RESULT_REPORT = 830,
#endif
#ifdef _PRE_WLAN_FEATURE_HID2D
    WLAN_CFGID_HID2D_ACS_STATE = 831, /* HiD2D更新方案状态 */
#endif
    WLAN_CFGID_MCAST_AMPDU_RETRY = 832,
    WLAN_CFGID_START_DEMO_VAP = 833,
    WLAN_CFGID_RX_FILTER_FRAG = 844,
    /************************************************************************
        第三段 非MIB的内部数据同步，需要严格受控
    *************************************************************************/
    WLAN_CFGID_SET_MULTI_USER = 2000,
    WLAN_CFGID_USR_INFO_SYN = 2001,
    WLAN_CFGID_USER_ASOC_STATE_SYN = 2002,
    WLAN_CFGID_INIT_SECURTIY_PORT = 2004,
    WLAN_CFGID_USER_RATE_SYN = 2005,
#if defined(_PRE_WLAN_FEATURE_OPMODE_NOTIFY) || defined(_PRE_WLAN_FEATURE_SMPS) || defined(_PRE_WLAN_FEATURE_M2S)
    WLAN_CFGID_USER_M2S_INFO_SYN = 2006, /* 更新opmode smps(m2s)的user相关信息 */
#endif
    WLAN_CFGID_USER_CAP_SYN = 2007, /* hmac向dmac同步mac user的cap能力信息 */

    WLAN_CFGID_SUSPEND_ACTION_SYN = 2008,
    WLAN_CFGID_SYNC_CH_STATUS = 2009,
    WLAN_CFGID_DYN_CALI_CFG = 2010,

#ifdef _PRE_WLAN_FEATURE_M2S
    WLAN_CFGID_DEVICE_M2S_INFO_SYN = 2011,
    WLAN_CFGID_VAP_M2S_INFO_SYN = 2012,
#endif

    WLAN_CFGID_PROFILING_PACKET_ADD = 2014,

    WLAN_CFGID_DBDC_DEBUG_SWITCH = 2015, /* DBDC开关 */

    WLAN_CFGID_VAP_MIB_UPDATE = 2016, /* d2h 根据hal cap更VAP mib 能力同步到host侧 */
#ifdef _PRE_WLAN_FEATURE_NEGTIVE_DET
    WLAN_CFGID_SYNC_PK_MODE = 2017,
#endif
    WLAN_CFGID_VAP_CAP_UPDATE = 2018, /* d2h 根据hal cap同步到host侧 */

    WLAN_CFGID_DYN_EXTLNA_BYPASS_SWITCH = 2019, /* 动态外置LNA bypass开关 */

    WLAN_CFGID_VAP_CHANNEL_INFO_SYN = 2020,
    WLAN_CFGID_GET_MNGPKT_SENDSTAT = 2021, /* 获取管理帧发送状态 */

#ifdef _PRE_WLAN_DELAY_STATISTIC
    WLAN_CFGID_NOTIFY_STA_DELAY = 2022,
#endif
    WLAN_CFGID_PROBE_PESP_MODE         = 2023,

#ifdef _PRE_WLAN_FEATURE_DDR_BUGFIX
    WLAN_CFGID_SET_DDR          = 2024, /* 设置DDR status */
#endif
    WLAN_UL_OFDMA_AMSDU_SYN   = 2025,
    WLAN_CFGID_SET_SEND_TB_PPDU_FLAG = 2026, /* 配置当前发送过tb ppdu flag */
#ifdef _PRE_WLAN_FEATURE_HIEX
    WLAN_CFGID_USER_HIEX_ENABLE = 2027, /* 设置user hiex能力 */
#endif
    WLAN_CFGID_PS_RX_DELBA_TRIGGER = 2028,
    WLAN_CFGID_PS_ARP_PROBE = 2029,
    WLAN_CFGID_D2H_MAX_TX_POWER = 2031, /* d2h上报发送功率相关信息到host侧 */
#ifdef _PRE_WLAN_FEATURE_WMMAC
    WLAN_CFGID_D2H_WMMAC_PARAM = 2032,
#endif
    /* 1106独有,0X共用命令勿往此处放置 */
    WLAN_CFGID_HI1106_PRIV_START = 4000,
    WLAN_CFGID_TX_TID_UPDATE = WLAN_CFGID_HI1106_PRIV_START,
    WLAN_CFGID_TX_BA_COMP = 4001,
    WLAN_CFGID_RX_MODE_SWITCH_DEBUG = 4002,
    WLAN_CFGID_RX_MODE_SWITCH = 4003,
    WLAN_CFGID_HAL_VAP_UPDATE = 4004,
    WLAN_CFGID_LUT_USR_UPDATE = 4005,
    WLAN_CFGID_SET_AMSDU_AGGR_NUM = 4006,
    WLAN_CFGID_REPORT_TID_QUEUE_SIZE = 4010,
    WLAN_CFGID_HE_UORA_OCW_UPDATE = 4011,
    WLAN_CFGID_DMAC_SYNC_AMSDU_MAXNUM = 4012,
#ifdef _PRE_WLAN_FEATURE_PHY_EVENT_INFO
    WLAN_CFGID_PHY_EVENT_RPT = 4013,        /* PHY事件上报开关 */
#endif
    WLAN_CFGID_START_ANOTHER_DEV_CAC = 4014, /* 开启另一路cac检测 */
    WLAN_CFGID_SYNC_BT_STATE = 4015, /* D2H 同步BT状态 */
    WLAN_CFGID_TX_SWITCH_START = 4016, /* host/device ring tx切换事件 */
    WLAN_CFGID_ZERO_WAIT_DFS = 4017, /* 开启zero wait dfs */
    WLAN_CFGID_ZERO_WAIT_DFS_REPORT = 4018, /* 同步雷达信号检测结果至host侧 */
    WLAN_CFGID_HI1106_PRIV_END,
    WLAN_CFGID_BUTT,
} wlan_cfgid_enum;
typedef uint16_t wlan_cfgid_enum_uint16;

#endif /* end of wlan_cfg_id.h */

