

#ifndef __ONEIMAGE_H__
#define __ONEIMAGE_H__

/* 其他头文件包含 */
#include "plat_type.h"

/* 宏定义 */
#define DTS_COMP_HW_CONNECTIVITY_NAME "hisilicon,hisi_wifi"

/* 2015-12-15 modify by ouyangxiaoyu for nfc one image end */
#define DTS_COMP_HW_HISI_SUPP_CONFIG_NAME     "hisi,wifi_supp"
#define DTS_COMP_HW_HISI_P2P_CONFIG_NAME      "hisi,wifi_p2p"
#define DTS_COMP_HW_HISI_HOSTAPD_CONFIG_NAME  "hisi,wifi_hostapd"
#define DTS_COMP_HW_HISI_FIRMWARE_CONFIG_NAME "hisi,wifi_firmware"

/* 2015-12-15 modify by ouyangxiaoyu for nfc one image end */
#define PROC_CONN_DIR "/proc/connectivity"

#define HW_CONN_PROC_DIR "connectivity"

#define HW_CONN_PROC_CHIPTYPE_FILE "chiptype"
#define HW_CONN_PROC_SUPP_FILE     "supp_config_template"
#define HW_CONN_PROC_P2P_FILE      "p2p_config_template"
#define HW_CONN_PROC_HOSTAPD_FILE  "hostapd_bin_file"
#define HW_CONN_PROC_FRIMWARE      "firmware_type_num"

#define BUFF_LEN      129
#define NODE_PATH_LEN 256

/* STRUCT DEFINE */
typedef struct hisi_proc_info {
    int proc_type;
    char *proc_node_name;
    char *proc_pro_name;
} hisi_proc_info_stru;

typedef enum proc_enum {
    HW_PROC_CHIPTYPE = 0,
    HW_PROC_SUPP,
    HW_PROC_P2P,
    HW_PROC_HOSTAPD,
    HW_PROC_FIRMWARE,
    HW_PROC_BUTT,
} hisi_proc_enum;

/* EXTERN FUNCTION */
#if (_PRE_OS_VERSION_LINUX == _PRE_OS_VERSION)
extern int is_my_chip(void);
extern int is_hisi_chiptype(int32_t chip);
extern int hw_misc_connectivity_init(void);
extern void hw_misc_connectivity_exit(void);
#endif
#endif
