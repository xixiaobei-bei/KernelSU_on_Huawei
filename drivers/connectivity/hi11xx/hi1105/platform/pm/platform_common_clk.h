

#ifndef __PLATFORM_COMMON_CLK_H__
#define __PLATFORM_COMMON_CLK_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_types.h"
#include "plat_type.h"
#ifdef HISI_NVRAM_SUPPORT
#ifdef CONFIG_ARCH_PLATFORM
#include <linux/mtd/nve_ap_kernel_interface.h>
#else
#include <linux/mtd/hisi_nve_interface.h>
#endif
#endif

#define hi_static_assert(cond_, name_) typedef char assert_failed_##name_[(cond_) ? 1 : -1]

#define DCXO_DEVICE_MAX_BUF_LEN 64
#define DCXO_CALI_DATA_BUF_LEN  (sizeof(dcxo_dl_para_stru))

/* 以下是DCXO 读取的NV中的参数 */
#define MALLOC_LEN            104
#define NV_GNSS_ID            236
#define NV_GNSS_SIZE          32
#define NV_GNSS_NAME          "XOCOE"
#define NV_FAC_ID             235
#define NV_FAC_SIZE           88
#define NV_FAC_NAME           "XOA1A0"
#define PLAT_DEF_ID           (-1)
#define PLAT_DEF_NAME         "PLAT_DEF_COEFF"
#define NVRAM_READ            1
#define NV_CHK_PAIR           2
#define RSV_NUM               2
#define DCXO_NV_CHK_OK        1
#define T0_BIT_WIDTH_10       0
#define T0_WIDTH10_TO_WIDTH16 (1 << 6)
#define DCXO_PARA_READ_OK     0x01
#define DCXO_PARA_T0_30       0x1E0000
#define FAC_NV_CRC_MASK       0xFFFF

#define PLAT_MAX_REPORT_CNT       10
#define PLAT_CHR_ERRNO_DCXO_NV_RD 910800034

/** 参数配置 **/
#define INI_BUF_SIZE   16
#define INI_TCXO_STR   "tcxo"
#define INI_DCXO_STR   "dcxo"
#define TCXO_DCXO_MODE "tcxo_dcxo_mode"

// 默认下发给device的参数
#define DEFAULT_T0       0x1D0000    // 29*2^16
#define DEFAULT_A0       0
#define DEFAULT_A1       0xFFFc0000  // -0.25*2^20
#define DEFAULT_A2       0
#define DEFAULT_A3       0x68db8b  // 0.0001*2^36
#define DEFAULT_MODE     0
#define DCXO_CHECK_MAGIC 0xA0A0A0A0

// 默认a0~t0的min max参数
#define DEF_A0_MIN (-3145728)  // -12*2^18
#define DEF_A0_MAX 3145728     // 12*2^18
#define DEF_A1_MIN (-419430)   // -0.4*2^20
#define DEF_A1_MAX (-104857)   // -0.1*2^20
#define DEF_A2_MIN (-30923764) // -0.00045*2^36
#define DEF_A2_MAX 30923764    // 0.00045*2^36
#define DEF_A3_MIN 5841155     // 0.000085*2^36
#define DEF_A3_MAX 7902739     // 0.000115*2^36
#define DEF_T0_MIN 1638400     // 25*2^16
#define DEF_T0_MAX 2293760     // 35*2^16

// a0~t0 min max 异常范围检测参数
#define A0_EXCEP_PROT_MIN (-13107200)   // -50*2^18
#define A0_EXCEP_PROT_MAX 13107200      // 50*2^18
#define A1_EXCEP_PROT_MIN (-1048576)    // -1*2^20
#define A1_EXCEP_PROT_MAX 0             // 0*2^20
#define A2_EXCEP_PROT_MIN (-1374389535) // -0.02*2^36
#define A2_EXCEP_PROT_MAX 1374389535    // 0.02*2^36
#define A3_EXCEP_PROT_MIN 0             // 0*2^36
#define A3_EXCEP_PROT_MAX 1374389535    // 0.02*2^36
#define T0_EXCEP_PROT_MIN 1310720       // 20*2^16
#define T0_EXCEP_PROT_MAX 2621440       // 40*2^16

/* PMU 参数配置 */
#define INI_PMU_CHIP_TYPE "pmu_version"
#define INI_6555V300_STR  "Hi6555V300"
#define INI_6421V700_STR  "Hi6421V700"
#define INI_6555V200_STR  "Hi6555V200"

// PMU 6555V300
#define PMU_6555V300_CALI_END  0x3A0
#define PMU_6555V300_CALI_AVE0 0x3A1
#define PMU_6555V300_CALI_AVE1 0x3A2
#define PMU_6555V300_CALI_AVE2 0x3A3
#define PMU_6555V300_CALI_AVE3 0x3A4
#define PMU_6555V300_ANA_EN    0x3AA
#define PMU_6555V300_STATE     0x3AC
#define PMU_6555V300_LOW_BYTE  0x3AD
#define PMU_6555V300_HIGH_BYTE 0x3AE

// PMU 6421V700
#define PMU_6421V700_CALI_END  0x3A0
#define PMU_6421V700_CALI_AVE0 0x3A1
#define PMU_6421V700_CALI_AVE1 0x3A2
#define PMU_6421V700_CALI_AVE2 0x3A3
#define PMU_6421V700_CALI_AVE3 0x3A4
#define PMU_6421V700_ANA_EN    0x3AA
#define PMU_6421V700_STATE     0x3AC
#define PMU_6421V700_LOW_BYTE  0x3AD
#define PMU_6421V700_HIGH_BYTE 0x3AE

// PMU 6555V200
#define PMU_6555V200_CALI_END  0x254
#define PMU_6555V200_CALI_AVE0 0x255
#define PMU_6555V200_CALI_AVE1 0x256
#define PMU_6555V200_CALI_AVE2 0x257
#define PMU_6555V200_CALI_AVE3 0x258
#define PMU_6555V200_ANA_EN    0x25E
#define PMU_6555V200_STATE     0x260
#define PMU_6555V200_LOW_BYTE  0x261
#define PMU_6555V200_HIGH_BYTE 0x262

enum coeff_index_enum {
    COEFF_A0 = 0,
    COEFF_A1 = 1,
    COEFF_A2 = 2,
    COEFF_A3 = 3,
    COEFF_T0 = 4,
    COEFF_NUM,
};

enum pmu_type_enum {
    PMU_HI655V300 = 0,
    PMU_HI6421V700 = 1,
    PMU_HI6555V200 = 2,
    PMU_TYPE_BUFF,
};

enum coeff_type_enum {
    INIT_TYPE_MIN = 0,
    INIT_TYPE_MAX = 1,
    COEFF_TYPE_BUFF,
};

enum dcxo_tcxo_enum {
    INI_TCXO_MODE = 0,
    INI_DCXO_MODE = 1,
    INIT_MODE_BUFF,
};

typedef struct {
    int32_t valid;                            // 判断当期nv是否有效， 1:有效 0:无效
    int32_t coeff_para[COEFF_NUM];            // 存放a0, a1, a2, a3, t0
    uint32_t coeff_time;                      // gnss相关参数，平台未使用
    int32_t reserve0;                         // 保留
    int32_t coeff_th[COEFF_NUM][NV_CHK_PAIR]; // a0~t0阈值范围放到ini文件中配置，此处已经不再使用
    int32_t t0_bit_width_fix;                 // 0: t0存储的是10位，需要乘以2^6修正, 1:t0存储的是16位，不需要修正
    int32_t tempr_expand;                     // gnss相关参数，平台未使用
    int32_t reserve1;                         // 保留
    uint32_t crc_relt;                        // crc校验结果
} dcxo_nv_info_stru;

typedef struct {
    uint16_t cali_end;
    uint16_t auto_cali_ave0;
    uint16_t auto_cali_ave1;
    uint16_t auto_cali_ave2;
    uint16_t auto_cali_ave3;
    uint16_t wifi_ana_en;
    uint16_t xoadc_state;
    uint16_t reg_addr_low;
    uint16_t reg_addr_high;
    uint16_t pmu_type;
} dcxo_pmu_addr_stru;

typedef struct {
    int32_t coeff_para[COEFF_NUM]; // 存放a0, a1, a2, a3, t0
    int32_t reserve2[RSV_NUM];     // 未使用
    uint32_t tcxo_dcxo_flag;
    dcxo_pmu_addr_stru pmu_addr;
    uint32_t check_data;
} dcxo_dl_para_stru;

// 从nv中读取的参数异常
typedef struct {
    int32_t l_struct_len;       // 结构体的大小
    int32_t l_para[COEFF_NUM];  // 存储依次为 a0, a1, a2, a3, t0
} plat_chr_nv_rd_excep_stru;

// 由于devcie划定的存储空间限制，这里需要做判断
hi_static_assert(((sizeof(dcxo_dl_para_stru)) < DCXO_DEVICE_MAX_BUF_LEN), device_mem_must_big_than_host);

typedef struct {
    uint32_t use_part_id;
    uint32_t chr_nv_read_err;
    long nv_init_flag;
    uint32_t dcxo_tcxo_mode;
    uint32_t pmu_chip_type;
    int32_t check_value[COEFF_NUM][NV_CHK_PAIR];  // 最大值/最小值判断
} dcxo_manage_stru;

extern dcxo_manage_stru g_dcxo_info;
extern int32_t dcxo_data_buf_malloc(void);
extern void dcxo_data_buf_free(void);
extern void *get_dcxo_data_buf_addr(void);
extern void read_tcxo_dcxo_ini_file(void);
extern void update_dcxo_coeff(int32_t *coeff, uint32_t coeff_cnt);
extern int32_t read_dcxo_cali_data(void);

#endif /* end of platform_common_clk.h */
