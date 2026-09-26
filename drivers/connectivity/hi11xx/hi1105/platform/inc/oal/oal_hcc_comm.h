

#include "oal_types.h"

#ifndef __OAL_HCC_COMM_H
#define __OAL_HCC_COMM_H

#undef CONFIG_HCC_DEBUG
#undef CONFIG_HCC_TX_MULTI_BUF_CHECK
#define CONFIG_HCC_HEADER_CHECK_SUM

typedef enum _hcc_action_type__ {
    HCC_ACTION_TYPE_WIFI = 0,      /* data from wifi */
    HCC_ACTION_TYPE_OAM = 1,       /* data from oam,sdt etc. */
    HCC_ACTION_TYPE_TEST = 2,      /* used for hcc transfer test */
    HCC_ACTION_TYPE_CHR = 3,       /* used for device chr exception */
    HCC_ACTION_TYPE_CUSTOMIZE = 4, /* used for device customization data transmit */
    HCC_ACTION_TYPE_BUS = 5,       /* used for hcc bus */

    HCC_ACTION_TYPE_BUTT
} hcc_action_type;

typedef enum _hcc_chan_type_ {
    HCC_TX = 0x0,
    HCC_RX = 0x1,
    HCC_DIR_COUNT
} hcc_chan_type;

typedef enum _oam_sub_type_ {
    DEAULT_VALUE = 0,
    DUMP_REG = 1,
    DUMP_MEM = 2,
} oam_sub_type;

typedef enum _hcc_sub_type_bus_ {
    HCC_SUB_BUS_SWITCH = 0,
    HCC_SUB_BUS_IP_PM = 1, /* WiFi IO IP PowerManager */
} hcc_sub_type_bus;

/*
 * hcc protocol header
 * |-------hcc total(64B)-----|-----------package mem--------------|
 * |hcc hdr|pad hdr|hcc extend|pad_payload|--------payload---------|
 */
#pragma pack(push, 1)
/* 6bytes */
struct hcc_header {
#define HCC_SUB_TYPE_BUTT 16   /* 4 bits bound [0~15] */
    uint8_t sub_type : 4;    /* sub type to hcc type,refer to hcc_action_type */
    uint8_t main_type : 4;   /* main type to hcc type,refer to hcc_action_type */

    uint8_t seq;              /* seq num for debug */
    uint8_t pad_hdr;          /* pad_hdr only need 6 bits, the 2bits reserved,
                                       pad_hdr used to algin hcc tcm hdr(64B) */
    uint16_t more : 1;        /* for hcc aggregation */
    uint16_t option : 1;      /* reserved */
    uint16_t pad_payload : 2; /* the pad to algin the payload addr */
    uint16_t pay_len : 12;    /* the payload length,
                                        did't contain the extend hcc hdr area */
} __OAL_DECLARE_PACKED;
#pragma pack(pop)
#define HCC_HDR_TOTAL_LEN        64
#define HCC_HDR_LEN              (sizeof(struct hcc_header))
#define HCC_HDR_RESERVED_MAX_LEN (HCC_HDR_TOTAL_LEN - HCC_HDR_LEN)
#define HCC_RESERVED_LEN         36 /* 临时定义宏!杜磊代码上库后改为使用MAC_CB_FRM_OFFSET */

typedef enum _hcc_test_case_ {
    HCC_TEST_CASE_TX = 0,
    HCC_TEST_CASE_RX = 1,
    HCC_TEST_CASE_LOOP = 2,
    HCC_TEST_CASE_COUNT
} hcc_test_case;

/* less than 16,4bits for sub_type */
typedef enum _hcc_test_subtype_ {
    HCC_TEST_SUBTYPE_CMD = 0, /* command mode */
    HCC_TEST_SUBTYPE_DATA = 1,
    HCC_TEST_SUBTYPE_BUTT
} hcc_test_subtype;

#define HCC_TEST_CMD_START_TX                      0
#define HCC_TEST_CMD_START_RX                      1
#define HCC_TEST_CMD_START_LOOP                    2
#define HCC_TEST_CMD_STOP_TEST                     3
#define HCC_TEST_CMD_CFG_FIX_FREQ                  4
#define HCC_TEST_CMD_PCIE_MAC_LOOPBACK_TST         5
#define HCC_TEST_CMD_PCIE_PHY_LOOPBACK_TST         6 /* PHY内部回环 */
#define HCC_TEST_CMD_PCIE_REMOTE_PHY_LOOPBACK_TST  7 /* PHY外部环回，TX/RX需要短接 */

#define HCC_TEST_MAC_IDX        0
#define HCC_TEST_PHY_IDX        1
#define HCC_TEST_REMOTE_PHY_IDX 2

typedef struct _hcc_test_cmd_stru_ {
    uint16_t cmd_type;
    uint16_t cmd_len;
} hcc_test_cmd_stru;
#define hcc_get_test_cmd_data(base)     (((uint8_t*)(base)) + sizeof(hcc_test_cmd_stru))
#define HCC_PCIE_RESPONSE_INITIAL_VALUE (-199)
typedef struct _hcc_pcie_test_request_stru_ {
    uint64_t request_flag;
    int32_t response_ret;
    uint32_t link_up_cost;
    uint32_t rw_test_cost;
    uint32_t total_test_cost;
    uint32_t linkup_timeout;
    uint32_t rw_test_count;
    uint32_t bias_cldo3;
    uint32_t bias_vptx;
    uint32_t bias_vph;
} hcc_pcie_test_request_stru;

typedef struct _hsdio_trans_test_info_ {
    uint32_t actual_tx_pkts;
    uint64_t total_h2d_trans_bytes; /* total bytes trans by from host to device */
    uint64_t total_d2h_trans_bytes; /* total bytes trans by from device to host */
} hsdio_trans_test_info;

typedef struct _hcc_trans_test_rx_info_ {
    uint32_t total_trans_pkts;
    uint32_t pkt_len;
    uint8_t pkt_value;
} hcc_trans_test_rx_info;

enum _hcc_descr_type_ {
    HCC_DESCR_ASSEM_RESET = 0,
    HCC_NETBUF_QUEUE_SWITCH = 1,
    HCC_DESCR_TYPE_BUTT
};

typedef uint32_t hcc_descr_type;
/*
 * This descr buff is reversed in device,
 * the callback function can't cost much time,
 * just for transfer sdio buff message
 */
struct hcc_descr_header {
    hcc_descr_type descr_type;
};

enum _hcc_netbuf_queue_type_ {
    HCC_NETBUF_NORMAL_QUEUE = 0, /* netbuf is shared with others */
    HCC_NETBUF_HIGH_QUEUE = 1,   /* netbuf is special for high pri */
    HCC_NETBUF_HIGH2_QUEUE = 2,   /* netbuf is special for TAE pri */
    HCC_NETBUF_QUEUE_BUTT
};

typedef enum _hcc_netbuf_queue_type_ hcc_netbuf_queue_type;

typedef struct {
    uint32_t align_type;
    uintptr_t start_addr;
    uint32_t men_len;
} exception_bcpu_dump_header;

enum dump_h2d_cmd {
    READ_MEM = 0,
    READ_REG = 1,

    DUMP_H2D_BUTT,
};
enum dump_d2h_cmd {
    DATA = 0,

    DUMP_D2H_BUTT,
};

#endif
