

#ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG

#ifndef __SSI_COMMON_H__
#define __SSI_COMMON_H__

/* 头文件包含 */
#include "plat_type.h"
#include "oal_types.h"

/* GPIO_SSI Base Reg */
#define SSI_SM_CLEAR   0xC  /* 8006 */
#define SSI_AON_CLKSEL 0xE  /* 8007 */
#define SSI_SEL_CTRL   0x10 /* 8008 */
#define SSI_SSI_CTRL   0x12 /* 8009 */

/* After 1105 */
#define SSI_SYS_CTL_ID 0x1C
#define SSI_GP_REG0    0x20
#define SSI_GP_REG1    0x24
#define SSI_GP_REG2    0x28
#define SSI_GP_REG3    0x2C
#define SSI_RDATA_L    0x30
#define SSI_RDATA_H    0x34
#define SSI_RPT_STS_L  0x38
#define SSI_RPT_STS_H  0x3C
#define SSI_SSI_RPT_STS_0 0x40
#define SSI_SSI_RPT_STS_1 0x44
#define SSI_SSI_RPT_STS_2 0x48
#define SSI_SSI_RPT_STS_3 0x4C
#define SSI_SSI_RPT_STS_4 0x50
#define SSI_SSI_RPT_STS_5 0x54
#define SSI_SSI_RPT_STS_6 0x58
#define SSI_SSI_RPT_STS_7 0x5C

#define GPIO_SSI_REG(offset) (0x8000 + ((offset) >> 1))

#define SSI_ALIVE 0x0305  /* SSI_SYS_CTL_ID寄存器默认值0x0305 */

#define SSI_AON_CLKSEL_TCXO 0x0
#define SSI_AON_CLKSEL_SSI  0x1

#define SSI_RW_WORD_MOD  0x0 /* 2 bytes */
#define SSI_RW_BYTE_MOD  0x1
#define SSI_RW_DWORD_MOD 0x2 /* 4 bytes */
#define SSI_RW_SSI_MOD   0x3 /* SSI master reg */

#define SSI_AHB_MODE_SET_START 0x0
#define SSI_AHB_MODE_SET_END   0x1

#define SSI_MODULE_MASK_AON           (1 << 0)
#define SSI_MODULE_MASK_ARM_REG       (1 << 1)
#define SSI_MODULE_MASK_WCTRL         (1 << 2)
#define SSI_MODULE_MASK_BCTRL         (1 << 3)
#define SSI_MODULE_MASK_PCIE_CFG      (1 << 4)
#define SSI_MODULE_MASK_PCIE_DBI      (1 << 5)
#define SSI_MODULE_MASK_SDIO          (1 << 6)
#define SSI_MODULE_MASK_UART          (1 << 7)
#define SSI_MODULE_MASK_WCPU_PATCH    (1 << 8)
#define SSI_MODULE_MASK_BCPU_PATCH    (1 << 9)
#define SSI_MODULE_MASK_WCPU_KEY_DTCM (1 << 10)
#define SSI_MODULE_MASK_AON_CUT       (1 << 11)
#define SSI_MODULE_MASK_PCIE_CUT      (1 << 12)
#define SSI_MODULE_MASK_COEX_CTL      (1 << 13)
#define SSI_MODULE_MASK_BCPU_EXCEPT_MEM      (1 << 14)


#define SSI_MODULE_MASK_COMM (SSI_MODULE_MASK_AON | SSI_MODULE_MASK_ARM_REG |  \
                              SSI_MODULE_MASK_WCTRL | SSI_MODULE_MASK_BCTRL |  \
                              SSI_MODULE_MASK_COEX_CTL | SSI_MODULE_MASK_WCPU_KEY_DTCM) /* 0xf */

#define SSI_MODULE_MASK_PCIE_FULL    (SSI_MODULE_MASK_COMM | \
                                     SSI_MODULE_MASK_PCIE_CFG | SSI_MODULE_MASK_PCIE_DBI)

#define SSI_MODULE_MASK_PCIE_SET   (SSI_MODULE_MASK_COMM | SSI_MODULE_MASK_PCIE_CFG)

#define SSI_MODULE_MASK_PCIE_CUT_SET   (SSI_MODULE_MASK_ARM_REG | SSI_MODULE_MASK_AON_CUT | SSI_MODULE_MASK_PCIE_CUT)

#define SSI_MODULE_NON_FLAG   (0)

#define SSI_WRITE_DATA 0x5a5a
#ifdef BFGX_UART_DOWNLOAD_SUPPORT
#define SSI_CLK_GPIO  89
#define SSI_DATA_GPIO 91
#else
#define SSI_CLK_GPIO  75
#define SSI_DATA_GPIO 77
#endif

#define DSM_CPU_INFO_SIZE 256
#define SSI_CPU_ARM_REG_DUMP_CNT  2

#define DECLARE_SSI_EINFO(etype, module_func, priv_func)     \
    {etype, #etype, module_func, priv_func}

typedef enum _ssi_err_type_ {
    SSI_ERR_COMM,
    SSI_ERR_HCC_EXCP_SDIO,
    SSI_ERR_HCC_EXCP_PCIE,
    SSI_ERR_PCIE_KIRIN_NOC,
    SSI_ERR_PCIE_CHECK_LINK_FAIL,
    SSI_ERR_PCIE_WAIT_BOOT_TIMEOUT,
    SSI_ERR_PCIE_SR_WAKEUP_FAIL,
    SSI_ERR_PCIE_SR_WAKEUP_RETRY_FAIL,
    SSI_ERR_PCIE_POWER_UP_FAIL,
    SSI_ERR_PCIE_FST_POWER_OFF_FAIL,
    SSI_ERR_PCIE_ENUM_FAIL,
    SSI_ERR_SDIO_REENUM_FAIL,
    SSI_ERR_SDIO_REINIT_FAIL,
    SSI_ERR_SDIO_PROBE_INIT_FAIL,
    SSI_ERR_SDIO_PROBE_FAIL,
    SSI_ERR_BFG_WAKE_UP_FAIL,
    SSI_ERR_FIRMWARE_DOWN_FAIL,
    SSI_ERR_FIRMWARE_DOWN_SDIO_FAIL,
    SSI_ERR_PCIE_GPIO_WAKE_FAIL,
    SSI_ERR_SDIO_CMD_WAKE_FAIL,
    SSI_ERR_SDIO_GPIO_WAKE_FAIL,
    SSI_ERR_WLAN_POWEROFF_FAIL,
    SSI_ERR_BFGX_OPEN_FAIL,
    SSI_ERR_BFGX_HEART_TIMEOUT,
    SSI_ERR_BUT
} ssi_err_type;

#define SSI_ERR_LOG_LEVEL_FULL    (0) /* full log print */
#define SSI_ERR_LOG_LEVEL_CUT     (1) /* little log print */
#define SSI_ERR_LOG_LEVEL_CLOSE   (2) /* Suggest nog log */

/* 针对特定异常，
 * SSI dump寄存器定制处理 */
typedef struct _ssi_err_stru_ {
    ssi_err_type etype;
    char* type_name;
    unsigned long long (*ssi_get_module_flag)(ssi_err_type etype);
    int32 (*ssi_private_func)(ssi_err_type etype);
} ssi_err_stru;

typedef struct _ssi_cpu_info_ {
    uint32 cpu_state;
    uint32 pc[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 lr[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 sp[SSI_CPU_ARM_REG_DUMP_CNT];
    uint32 reg_flag[SSI_CPU_ARM_REG_DUMP_CNT];
} ssi_cpu_info;

typedef struct _ssi_cpu_infos_ {
    ssi_cpu_info wcpu_info;
    ssi_cpu_info bcpu_info;
} ssi_cpu_infos;

typedef struct _ssi_reg_info_ {
    uint32 base_addr;
    uint32 len;
    uint32 rw_mod;
} ssi_reg_info;

typedef struct ssi_trans_test {
    uint32 test_type;

    uint32 trans_len;
    uint32 used_time;
    int32 send_status;
} ssi_trans_test_st;

#define SSI_PATH_NAME_LEN 100

typedef struct _ssi_file_st {
    uint8 file_name[SSI_PATH_NAME_LEN];
    uint32 write_addr;
} ssi_file_st;


extern ssi_trans_test_st ssi_test_st;
extern int ssi_is_logfile;
extern int hi11xx_kernel_crash;

extern uint16 ssi_read16_etc(uint16 addr);
extern int32 ssi_write16_etc(uint16 addr, uint16 value);
extern int32 ssi_read_value16(uint32 addr, uint16 *value, int16 last_high_addr);
extern int32 ssi_read_value32(uint32 addr, uint32 *value, int16 last_high_addr);
extern int ssi_read_reg_info_arry(ssi_reg_info **pst_reg_info, uint32 reg_nums, int32 is_logfile);
extern int32 ssi_request_gpio_etc(uint32 clk, uint32 data);
extern int32 ssi_free_gpio_etc(void);
extern int32 do_ssi_file_test(ssi_file_st *file_st, ssi_trans_test_st *pst_ssi_test);
extern int ssi_read_reg_info(ssi_reg_info *pst_reg_info, void *buf, int32 size, oal_int32 is_file);
extern int32 ssi_write32_etc(uint32 addr, uint16 value);
extern int32 ssi_read32_etc(uint32 addr);
extern int32 ssi_single_write(int32 addr, int16 data);
extern int32 ssi_single_read(int32 addr);
extern int ssi_switch_clk(uint32 clk_type);
extern int ssi_clk_auto_switch_is_support(void);
extern int32 ssi_download_test(ssi_trans_test_st *pst_ssi_test);
extern int32 wait_for_ssi_idle_timeout(int32 mstimeout);
extern int32 test_hd_ssi_write(void);
extern int ssi_force_reset_aon(void);
extern oal_void ssi_force_reset_reg(oal_void);
extern int ssi_check_device_isalive(void);
extern int ssi_dump_device_regs(unsigned long long module_set);
extern int ssi_dump_err_regs(ssi_err_type etype);
extern void ssi_bfgx_dump_device_regs(unsigned long long module_set, oal_ulong timeout);
extern int ssi_read_reg_info_test(uint32 base_addr, uint32 len, uint32 is_logfile, uint32 rw_mode);
extern int ssi_dump_err_reg(ssi_err_type etype);
#endif /* #ifndef __SSI_COMMON_H__ */
#endif /* #ifdef _PRE_CONFIG_GPIO_TO_SSI_DEBUG */
