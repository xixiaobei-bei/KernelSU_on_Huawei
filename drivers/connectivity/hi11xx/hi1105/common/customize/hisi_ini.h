

#ifndef __HISI_INI_H__
#define __HISI_INI_H__

/* 其他头文件包含 */
#include "hw_bfg_ps.h"
#include "bfgx_user_ctrl.h"
#include "plat_type.h"

#ifndef _PRE_LINUX_TEST
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM

#else
#define HISI_NVRAM_SUPPORT
#endif
#define HISI_DTS_SUPPORT
#endif
#endif

#define ini_min(_A, _B)        (((_A) < (_B)) ? (_A) : (_B))

#ifdef _PRE_LINUX_TEST
#define ini_debug(fmt, arg...)
#define ini_info(fmt, arg...)
#define ini_warning(fmt, arg...)
#define ini_error(fmt, arg...)
#else
#ifdef _PRE_WINDOWS_SUPPORT
#define ini_debug
#define ini_info
#define ini_warning
#define ini_error
#else
#define ini_debug(fmt, arg...) \
    do {                                                                                   \
        if (g_hi110x_loglevel >= PLAT_LOG_DEBUG) {                                         \
            /*lint -e515*/                                                                 \
            printk(KERN_INFO "INI_DRV:D]%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg); \
            /*lint +e515*/                                                                 \
        }                                                                                  \
    } while (0)

#define ini_info(fmt, arg...)                                                              \
    do {                                                                                   \
        if (g_hi110x_loglevel >= PLAT_LOG_INFO) {                                          \
            /*lint -e515*/                                                                 \
            printk(KERN_INFO "INI_DRV:D]%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg); \
            /*lint +e515*/                                                                 \
        }                                                                                  \
    } while (0)

#define ini_warning(fmt, arg...)                                                             \
    do {                                                                                     \
        if (g_hi110x_loglevel >= PLAT_LOG_WARNING) {                                         \
            /*lint -e515*/                                                                   \
            printk(KERN_WARNING "INI_DRV:W]%s:%d]" fmt "\n", __FUNCTION__, __LINE__, ##arg); \
            /*lint +e515*/                                                                   \
        }                                                                                    \
    } while (0)

#define ini_error(fmt, arg...)                                                               \
    do {                                                                                     \
        if (g_hi110x_loglevel >= PLAT_LOG_ERR) {                                             \
            /*lint -e515*/                                                                   \
            printk(KERN_ERR "INI_DRV:E]%s:%d]" fmt "\n\n\n", __FUNCTION__, __LINE__, ##arg); \
            /*lint +e515*/                                                                   \
        }                                                                                    \
    } while (0)
#endif
#endif

#define HISI_CUST_NVRAM_READ  1
#define HISI_CUST_NVRAM_WRITE 0
#define HISI_CUST_NVRAM_NUM   340
#define HISI_CUST_NVRAM_LEN   104
#define HISI_CUST_NVRAM_NAME  "WINVRAM"

#define INI_MODU_WIFI         "[HOST_WIFI_NORMAL]"
#define INI_MODU_GNSS         "[HOST_GNSS_NORMAL]"
#define INI_MODU_BT           "[HOST_BT_NORMAL]"
#define INI_MODU_FM           "[HOST_FM_NORMAL]"
#define INI_MODU_PLAT         "[HOST_PLAT]"
#define INI_MODU_HOST_VERSION "[HOST_VERSION]"
#define INI_MODU_WIFI_MAC     "[HOST_WIFI_MAC]"
#define INI_MODU_COEXIST      "[HOST_COEXIST]"
#define INI_MODU_DEV_WIFI     "[DEVICE_WIFI]"
#define INI_MODU_DEV_GNSS     "[DEVICE_GNSS]"
#define INI_MODU_DEV_BT       "[DEVICE_BT]"
#define INI_MODU_DEV_FM       "[DEVICE_FM]"
#define INI_MODU_DEV_BFG_PLAT "[DEVICE_BFG_PLAT]"

#define MAX_READ_LINE_NUM   192
#define INI_FILE_PATH_LEN   128
#define INI_READ_VALUE_LEN  64

#define INI_SUCC   0
#define INI_FAILED (-1)

#define INI_FILE_TIMESPEC_UNRECONFIG    0
#define INI_FILE_TIMESPEC_RECONFIG      BIT0
#define INI_NVRAM_RECONFIG              BIT1

#define inf_file_get_ctime(file_dentry) ((file_dentry)->d_inode->i_ctime.tv_sec)

typedef uint32_t (*fun_wifi_customize_t)(void);

/* 函数声明 */
int32_t get_cust_conf_int32(const char *tag, const char *var, int32_t *value);
int32_t get_cust_conf_string(const char *tag, const char *var, char *value, uint32_t size);
#ifndef _PRE_CONFIG_READ_DYNCALI_E2PROM
int32_t read_conf_from_nvram(uint8_t *pc_out, uint32_t size, uint32_t nv_number, const char *nv_name);
#endif
char *get_ini_file_name(void);
int32_t ini_cfg_init(void);
void ini_cfg_exit(void);
int32_t ini_file_check_conf_update(void);

// 平台调用wifi的定制化模块
void register_wifi_customize(fun_wifi_customize_t fun_wifi_customize);
void wifi_customize_h2d(void);

#ifdef _PRE_CONFIG_READ_DYNCALI_E2PROM
#ifdef _PRE_PRODUCT_HI3751_PLATO
#define ini_eeprom_read drv_eeprom_read
#endif
#ifdef _PRE_PRODUCT_ARMPC
#define ini_eeprom_read drv_e2prom_read
#endif

#ifdef _PRE_PRODUCT_HI1620S_KUNPENG
#define ini_eeprom_read DrvEepromRead
#endif
int32_t ini_eeprom_read(uint8_t *partName, uint32_t offSet, char *rBuf, uint32_t rSize);
int32_t read_conf_from_eeprom(uint8_t *pc_out, uint32_t size, uint32_t offset);
#endif
#ifdef _PRE_SUSPORT_OEMINFO
int32_t read_conf_from_oeminfo(uint8_t *pc_out, uint32_t size, uint32_t offset);
#endif
#endif
