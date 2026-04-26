/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: contexthub boot header file
 * Author: huangjisong
 * Create: 2013-3-10
 */

#ifndef __LINUX_INPUTHUB_CMU_H__
#define __LINUX_INPUTHUB_CMU_H__

#ifdef CONFIG_INPUTHUB_30
#include <linux/hisi/contexthub/iomcu_boot.h>
#endif
#include <linux/hisi/hisi_rproc.h>
#include <linux/types.h>

#include <iomcu_ddr_map.h>

#ifndef CONFIG_INPUTHUB_30
#define STARTUP_IOM3_CMD 0x00070001
#define RELOAD_IOM3_CMD 0x0007030D

#define IOMCU_CONFIG_SIZE   DDR_CONFIG_SIZE
#define IOMCU_CONFIG_START  DDR_CONFIG_ADDR_AP
#endif
#define SENSOR_MAX_RESET_TIME_MS 400
#define SENSOR_DETECT_AFTER_POWERON_TIME_MS 50
#define SENSOR_POWER_DO_RESET 0
#define SENSOR_POWER_NO_RESET 1
#define SENSOR_REBOOT_REASON_MAX_LEN 32

#define WARN_LEVEL 2
#define INFO_LEVEL 3

#define LG_TPLCD 1
#define JDI_TPLCD 2
#define KNIGHT_BIEL_TPLCD 3
#define KNIGHT_LENS_TPLCD 4
#define BOE_TPLCD 5
#define EBBG_TPLCD 6
#define INX_TPLCD 7
#define SAMSUNG_TPLCD 8
#define SHARP_TPLCD 9
#define BIEL_TPLCD 10
#define VITAL_TPLCD 11
#define TM_TPLCD 12
#define AUO_TPLCD 13
#define TCL_TPLCD 14
#define CTC_TPLCD 15
#define CSOT_TPLCD 16
#define VISI_TPLCD 17
#define BOE_TPLCD2 18
#define EDO_TPLCD 19

#define DTS_COMP_LG_ER69006A "hisilicon,mipi_lg_eR69006A"
#define DTS_COMP_JDI_NT35695_CUT3_1 "hisilicon,mipi_jdi_NT35695_cut3_1"
#define DTS_COMP_JDI_NT35695_CUT2_5 "hisilicon,mipi_jdi_NT35695_cut2_5"

#define DTS_COMP_LG_ER69007  "hisilicon,mipi_lg_eR69007"
#define DTS_COMP_SHARP_NT35597  "hisilicon,mipi_sharp_knt_NT35597"
#define DTS_COMP_LG_ER69006_FHD      "hisilicon,mipi_lg_eR69006_FHD"
#define DTS_COMP_SHARP_NT35695  "hisilicon,mipi_sharp_NT35695_5p7"
#define DTS_COMP_MIPI_BOE_ER69006  "hisilicon,mipi_boe_ER69006_5P7"

#define DTS_COMP_BOE_OTM1906C  "hisilicon,boe_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_EBBG_OTM1906C  "hisilicon,ebbg_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_INX_OTM1906C  "hisilicon,inx_otm1906c_5p2_1080p_cmd"
#define DTS_COMP_JDI_NT35695  "hisilicon,jdi_nt35695_5p2_1080p_cmd"
#define DTS_COMP_LG_R69006  "hisilicon,lg_r69006_5p2_1080p_cmd"
#define DTS_COMP_SAMSUNG_S6E3HA3X02 "hisilicon,mipi_samsung_S6E3HA3X02"

#define DTS_COMP_LG_R69006_5P2  "hisilicon,mipi_lg_R69006_5P2"
#define DTS_COMP_SHARP_NT35695_5P2  "hisilicon,mipi_sharp_NT35695_5P2"
#define DTS_COMP_JDI_R63452_5P2  "hisilicon,mipi_jdi_R63452_5P2"

#define DTS_COMP_SAM_WQ_5P5  "hisilicon,mipi_samsung_S6E3HA3X02_5P5_AMOLED"
#define DTS_COMP_SAM_FHD_5P5  "hisilicon,mipi_samsung_D53G6EA8064T_5P5_AMOLED"

#define DTS_COMP_JDI_R63450_5P7 "hisilicon,mipi_jdi_duke_R63450_5P7"
#define DTS_COMP_SHARP_DUKE_NT35597 "hisilicon,mipi_sharp_duke_NT35597"

#define DTS_COMP_AUO_OTM1901A_5P2 "auo_otm1901a_5p2_1080p_video"
#define DTS_COMP_AUO_TD4310_5P2 "auo_td4310_5p2_1080p_video"

#define DTS_COMP_AUO_NT36682A_6P72 "auo_nt36682a_6p57"
#define DTS_COMP_AUO_OTM1901A_5P2_1080P_VIDEO_DEFAULT "auo_otm1901a_5p2_1080p_video_default"
#define DTS_COMP_BOE_NT36682A_6P57 "boe_nt36682a_6p57"
#define DTS_COMP_BOE_TD4320_6P57 "boe_td4320_6p57"
#define DTS_COMP_TCL_NT36682A_6P57 "tcl_nt36682a_6p57"
#define DTS_COMP_TCL_TD4320_6P57 "tcl_td4320_6p57"
#define DTS_COMP_TM_NT36682A_6P57 "tm_nt36682a_6p57"
#define DTS_COMP_TM_TD4320_6P57 "tm_td4320_6p57"

#define DTS_COMP_TM_FT8716_5P2 "tm_ft8716_5p2_1080p_video"
#define DTS_COMP_EBBG_NT35596S_5P2 "ebbg_nt35596s_5p2_1080p_video"
#define DTS_COMP_JDI_ILI7807E_5P2 "jdi_ili7807e_5p2_1080p_video"
#define DTS_COMP_BOE_NT37700F "boe_nt37700f_vogue_6p47_1080plus_cmd"
#define DTS_COMP_BOE_NT36672_6P26 "boe_nt36672a_6p26"
#define DTS_COMP_LG_NT37280 "lg_nt37280_vogue_6p47_1080plus_cmd"
#define DTS_COMP_BOE_NT37700F_EXT "boe_nt37700f_vogue_p_6p47_1080plus_cmd"
#define DTS_COMP_LG_NT37280_EXT "lg_nt37280_vogue_p_6p47_1080plus_cmd"
#define DTS_COMP_LG_TD4320_6P26 "lg_td4320_6p26"
#define DTS_COMP_SAMSUNG_EA8074 "samsung_ea8074_elle_6p10_1080plus_cmd"
#define DTS_COMP_SAMSUNG_EA8076 "samsung_ea8076_elle_6p11_1080plus_cmd"
#define DTS_COMP_SAMSUNG_EA8076_V2 "samsung_ea8076_elle_v2_6p11_1080plus_cmd"
#define DTS_COMP_BOE_NT37700F_TAH "boe-nt37800f-tah-8p03-3lane-2mux-cmd"
#define DTS_COMP_BOE_NT37800ECO_TAH "boe-nt37800eco-tah-8p03-3lane-2mux-cmd"
#define DTS_COMP_HLK_AUO_OTM1901A "hlk_auo_otm1901a_5p2_1080p_video_default"
#define DTS_COMP_BOE_NT36682A "boe_nt36682a_6p59_1080p_video"
#define DTS_COMP_BOE_TD4320 "boe_td4320_6p59_1080p_video"
#define DTS_COMP_INX_NT36682A "inx_nt36682a_6p59_1080p_video"
#define DTS_COMP_TCL_NT36682A "tcl_nt36682a_6p59_1080p_video"
#define DTS_COMP_TM_NT36682A "tm_nt36682a_6p59_1080p_video"
#define DTS_COMP_TM_TD4320 "tm_td4320_6p59_1080p_video"
#define DTS_COMP_TM_TD4320_6P26 "tm_td4320_6p26"
#define DTS_COMP_TM_TD4330_6P26 "tm_td4330_6p26"
#define DTS_COMP_TM_NT36672A_6P26 "tm_nt36672a_6p26"

#define DTS_COMP_CTC_FT8719_6P26 "ctc_ft8719_6p26"
#define DTS_COMP_CTC_NT36672A_6P26 "ctc_nt36672a_6p26"
#define DTS_COMP_BOE_TD4321_6P26 "boe_td4321_6p26_1080p_video"

#define DTS_COMP_CSOT_NT36682A_6P5 "csot_nt36682a_6p5"
#define DTS_COMP_BOE_FT8719_6P5 "boe_ft8719_6p5"
#define DTS_COMP_TM_NT36682A_6P5 "tm_nt36682a_6p5"
#define DTS_COMP_BOE_TD4320_6P5 "boe_td4320_6p5"

#define DTS_COMP_BOE_EW813P_6P57 "boe_ew813p_6p57"
#define DTS_COMP_BOE_NT37700P_6P57 "boe_nt37700p_6p57"
#define DTS_COMP_VISI_NT37700C_6P57_ONCELL "visi_nt37700c_6p57_oncell"
#define DTS_COMP_VISI_NT37700C_6P57 "visi_nt37700c_6p57"

#define DTS_COMP_TCL_NT36682C_6P63 "tcl_nt36682c_6p63_1080p_video"
#define DTS_COMP_TM_NT36682C_6P63 "tm_nt36682c_6p63_1080p_video"
#define DTS_COMP_BOE_NT36682C_6P63 "boe_nt36682c_6p63_1080p_video"
#define DTS_COMP_INX_NT36682C_6P63 "inx_nt36682c_6p63_1080p_video"
#define DTS_COMP_BOE_FT8720_6P63 "boe_ft8720_6p63_1080p_video"

#define DTS_COMP_TM_TD4321_6P59 "tm_td4321_6p59"
#define DTS_COMP_TCL_NT36682A_6P59 "tcl_nt36682a_6p59"
#define DTS_COMP_BOE_NT36682A_6P59 "boe_nt36682a_6p59"

#define DTS_COMP_SAMSUNG_EA8076_6P53 "samsung_ea8076_6p53_1080p_cmd"
#define DTS_COMP_SAMSUNG_NAME1 "samsung"
#define DTS_COMP_EDO_RM692D9 "edo_rm692d9_6p53_1080p_cmd"
#define DTS_COMP_EDO_NAME "edo"
#define DTS_COMP_EDO "350"

#define DTS_COMP_BOE "190"
#define DTS_COMP_BOE_B11 "191"
#define DTS_COMP_VISI "310"
#define DTS_COMP_TM "110"
#define DTS_COMP_CSOT "290"
#define DTS_COMP_SAMSUNG "090"

#define DTS_COMP_BOE_NAME "boe"
#define DTS_COMP_SAMSUNG_NAME "sm"

#define SC_EXISTENCE   1
#define SC_INEXISTENCE 0

#define BOE_RESOLUTION 2155
#define SAMSUNG_RESOLUTION 1871

enum SENSOR_POWER_CHECK {
	SENSOR_POWER_STATE_OK = 0,
	SENSOR_POWER_STATE_INIT_NOT_READY,
	SENSOR_POWER_STATE_CHECK_ACTION_FAILED,
	SENSOR_POWER_STATE_CHECK_RESULT_FAILED,
	SENSOR_POWER_STATE_NOT_PMIC,
};

typedef struct {
	u32 mutex;
	u16 index;
	u16 pingpang;
	u32 buff;
	u32 ddr_log_buff_cnt;
	u32 ddr_log_buff_index;
	u32 ddr_log_buff_last_update_index;
} log_buff_t;

typedef struct wrong_wakeup_msg {
	u32 flag;
	u32 reserved1;
	u64 time;
	u32 irq0;
	u32 irq1;
	u32 recvfromapmsg[4];
	u32 recvfromlpmsg[4];
	u32 sendtoapmsg[4];
	u32 sendtolpmsg[4];
	u32 recvfromapmsgmode;
	u32 recvfromlpmsgmode;
	u32 sendtoapmsgmode;
	u32 sendtolpmsgmode;
} wrong_wakeup_msg_t;
#ifndef CONFIG_INPUTHUB_30
enum dump_loc {
	DL_NONE = 0,
	DL_TCM,
	DL_EXT,
	DL_BOTTOM = DL_EXT,
};

enum DUMP_REGION {
	DE_TCM_CODE,
	DE_DDR_CODE,
	DE_DDR_DATA,
	DE_BOTTOM = 16,
};

struct dump_region_config {
	u8 loc;
	u8 reserved[3];
};

struct dump_config {
	u64 dump_addr;
	u32 dump_size;
	u32 reserved1;
	u64 ext_dump_addr;
	u32 ext_dump_size;
	u8 enable;
	u8 finish;
	u8 reason;
	u8 reserved2;
	struct dump_region_config elements[DE_BOTTOM];
};

struct timestamp_kernel {
	u64 syscnt;
	u64 kernel_ns;
};

struct timestamp_iomcu_base {
	u64 syscnt;
	u64 kernel_ns;
	u32 timer32k_cyc;
	u32 reserved;
};
#endif

typedef struct {
	const char *dts_comp_mipi;
	uint8_t tplcd;
} lcd_module;

typedef struct {
	const char *dts_comp_lcd_model;
	uint8_t tplcd;
} lcd_model;

typedef struct coul_core_info {
	int c_offset_a;
	int c_offset_b;
	int r_coul_mohm;
	int reserved;
} coul_core_info_t;

struct bright_data {
	uint32_t mipi_data;
	uint32_t bright_data;
	uint64_t time_stamp;
};

struct read_data_als_ud {
	float rdata;
	float gdata;
	float bdata;
	float irdata;
};

struct als_ud_config_t {
	u8 screen_status;
	u8 reserved[7]; // 7 is reserved nums
	u64 als_rgb_pa;
	struct bright_data bright_data_input;
	struct read_data_als_ud read_data_history;
};
#ifndef CONFIG_INPUTHUB_30
struct bbox_config {
	u64 log_addr;
	u32 log_size;
	u32 rsvd;
};
#endif
struct config_on_ddr {
	struct dump_config dump_config;
	log_buff_t log_buff_cb_backup;
	wrong_wakeup_msg_t wrong_wakeup_msg;
	u32 log_level;
	float gyro_offset[3];
	u8 modem_open_app_state;
	u8 hifi_open_sound_msg;
	u8 igs_hardware_bypass;
	u8 screen_sts;
	u8 reserved1[4];
	u64 reserved;
	struct timestamp_kernel timestamp_base;
	struct timestamp_iomcu_base timestamp_base_iomcu;
	coul_core_info_t coul_info;
	struct bbox_config bbox_conifg;
	struct als_ud_config_t als_ud_config;
	u32 te_irq_tcs3701;
	u32 old_dc_flag;
	u32 lcd_freq;
	u8 phone_type_info[2]; // 2 is phone type info nums
	u32 sc_flag;
	u32 is_pll_on;
	u32 fall_down_support;
};

extern uint32_t need_reset_io_power;
extern uint32_t need_set_3v_io_power;
extern uint32_t need_set_3_1v_io_power;
extern uint32_t need_set_3_2v_io_power;
extern atomic_t iom3_rec_state;

#ifdef SENSOR_1V8_POWER
extern int hw_extern_pmic_query_state(int index, int *state);
#endif

void write_timestamp_base_to_sharemem(void);
int get_sensor_mcu_mode(void);
int is_sensorhub_disabled(void);
void peri_used_request(void);
void peri_used_release(void);
#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *inputhub_get_shb_dclient(void);
#endif
void inputhub_init_before_boot(void);
void inputhub_init_after_boot(void);
u8 get_tplcd_manufacture(void);
rproc_id_t get_ipc_ap_to_iom_mbx(void);
rproc_id_t get_ipc_ap_to_lpm_mbx(void);
int get_sensorhub_reboot_reason_flag(void);
struct dsm_dev *get_dsm_sensorhub(void);
#endif /* __LINUX_INPUTHUB_CMU_H__ */
