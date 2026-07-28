
#ifndef  __PARAM_TO_SEC_OS_H__
#define  __PARAM_TO_SEC_OS_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <osl_types.h>
#include "product_config.h"

/* 这个枚举的修改需要同步修改一下几个地方
mbb tzdriver
vendor/hisi/system/kernel/drivers/hisi/tzdriver/teek_client_id.h
mbb secos
vendor/hisi/system/secure_os/trustedcore/include/TEE/tee_common.h
vendor/hisi/system/secure_os/trustedcore/platform/balong/include/bsp_param_cfg.h
mbb、phone:ccore modem
vendor/hisi/system/kernel/drivers/hisi/modem/drv/common/include/param_cfg_to_sec.h
phone tzdriver
vendor/hisi/ap/kernel/drivers/tzdriver/teek_client_id.h
phone secos
vendor/thirdparty/secure_os/trustedcore/include/TEE/tee_common.h
vendor/thirdparty/secure_os/trustedcore/platform/kirin/secureboot/secboot.h
*/
#ifdef BSP_CONFIG_PHONE_TYPE
#ifdef FEATURE_KEEP_LEGICY_SEC_OS
enum SVC_SECBOOT_IMG_TYPE{
    MODEM,
    HIFI,
    DSP,
    XDSP,
    TAS,
    WAS,
    CAS,
    MODEM_DTB,
    ISP,
    MODEM_CERT,
    SOC_MAX
};
#else
enum SVC_SECBOOT_IMG_TYPE {
    MODEM,
    DSP,
    XDSP,
    TAS,
    WAS,
    CAS,
    MODEM_DTB,
    NVM,
    NVM_S,
    MBN_R,
    MBN_A,
    MODEM_COLD_PATCH,
    DSP_COLD_PATCH,
    MODEM_CERT,
    MAX_SOC_MODEM,
    HIFI,
    ISP,
    IVP,
    SOC_MAX
};
#endif
#else
enum SVC_SECBOOT_IMG_TYPE{
    MODEM,
    HIFI,
    DSP,
    XDSP,
    TAS,
    WAS,
    CAS,
    MODEM_DTB,
    ISP,
    NVM,
    NVM_S,
    MBN_R,
    MBN_A,
    MODEM_CERT,
    SOC_MAX,
    MAX_SOC_MODEM = SOC_MAX
};
#endif


#define PARAM_MAGIC_OFFSET            (0x0)           /* 魔数:判断参数有效性 */
#define PARAM_CFG_OFFSET              (0x8)           /* 配置参数存放基地址*/

#ifdef DDR_SECURE_OS_ADDR
/*系统启动过程中传递C核镜像执行地址、镜像大小等参数到安全OS，
参数的存放地址由之前的安全共享内存转移到安全OS 的内存中，
安全OS为DX分配1MB的内存空间，位于安全OS内存起始地址偏移0x500000处，
fastboot使用DX 这1M内存空间的最后16KB存放需要传递到安全OS的参数，
16KB中最后的2KB用于存放参数的地址信息，剩余的14KB用于存放需要传递的参数*/
#define BALONG_PARAM_BASE_ADDR        (DDR_SECURE_OS_ADDR+0x600000 -0x800)    /* 0xA07FF800--0xA07FF8FF */
#define BALONG_SEC_NV_BASE_ADDR       (DDR_SECURE_OS_ADDR+0x600000 - 0x1000)
#define SECBOOT_PARAM_CFG_BASE_ADDR    (DDR_SECURE_OS_ADDR+0x600000 -0x4000)
#define SECBOOT_PARAM_CFG_SIZE    0x3000//0x3800
#endif

/*sec boot start */
struct image_info
{
    u64 ddr_addr;
    unsigned int ddr_size;
    unsigned int unreset_dependcore;
};

struct dynamic_load
{
    u32 load_cmd;
    u32 verify_flag;
};

#ifdef FEATURE_KEEP_LEGICY_SEC_OS
struct sec_boot_modem_info
{
    struct image_info image_info[SOC_MAX];
    struct dynamic_load dynamic_load;
};
#else
struct sec_boot_modem_info
{
    struct dynamic_load dynamic_load;
    struct image_info image_info[MAX_SOC_MODEM];
};
#endif
 /*sec boot end */

typedef struct tag_PARAM_CFG
{
    u32   magic;                   /* 魔数，标识配置参数的状态 */
    u32   protect_barrier_size;    /* 预留(4K)防止被踩，初始化为全F */
    u32   param_cfg_size;          /* 配置参数预留(16K)大小 */
    u64   icc_channel_base_addr;
    u32   icc_channel_max_size;
    struct sec_boot_modem_info sec_boot_modem_info;
}g_param_cfg;

void set_param_cfg_to_sec(void);

void set_modem_info_to_sec(void);

#ifdef __cplusplus
}
#endif

#endif   /* __PARAM_TO_SEC_OS_H__ */

