#ifndef _TAURUS_PARTITION_H_
#define _TAURUS_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,                   0,                2*1024,     EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,                 0,                2*1024,     EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                    0,                512,        EMMC_USER_PART},/* ptable          512K */
  {PART_FRP,                       512,              512,        EMMC_USER_PART},/* frp             512K     p1*/
  {PART_PERSIST,                   1024,             6*1024,     EMMC_USER_PART},/* persist         6M       p2*/
  {PART_RESERVED1,                 7*1024,           1024,       EMMC_USER_PART},/* reserved1       1024K    p3*/
  {PART_RESERVED6,                 8*1024,           512,        EMMC_USER_PART},/* reserved6       512K     p4*/
  {PART_VRL,                       8704,             512,        EMMC_USER_PART},/* vrl             512K     p5*/
  {PART_VRL_BACKUP,                9*1024,           512,        EMMC_USER_PART},/* vrl backup      512K     p6*/
  {PART_MODEM_SECURE,              9728,             8704,       EMMC_USER_PART},/* modem_secure    8704K    p7*/
  {PART_NVME,                      18*1024,          5*1024,     EMMC_USER_PART},/* nvme            5M       p8*/
  {PART_CTF,                       23*1024,          1*1024,     EMMC_USER_PART},/* ctf             1M       p9*/
  {PART_OEMINFO,                   24*1024,          96*1024,    EMMC_USER_PART},/* oeminfo         96M      p10*/
  {PART_SECURE_STORAGE,            120*1024,         32*1024,    EMMC_USER_PART},/* secure storage  32M      p11*/
  {PART_MODEMNVM_FACTORY,          152*1024,         16*1024,    EMMC_USER_PART},/* modemnvm factory16M      p12*/
  {PART_MODEMNVM_BACKUP,           168*1024,         16*1024,    EMMC_USER_PART},/* modemnvm backup 16M      p13*/
  {PART_MODEMNVM_IMG,              184*1024,         46*1024,    EMMC_USER_PART},/* modemnvm img    46M      p14*/
  {PART_HISEE_ENCOS,               230*1024,         4*1024,     EMMC_USER_PART},/* hisee_encos      4M      p15*/
  {PART_VERITYKEY,                 234*1024,         1*1024,     EMMC_USER_PART},/* veritykey        1M      p16*/
  {PART_DDR_PARA,                  235*1024,         1*1024,     EMMC_USER_PART},/* DDR_PARA         1M      p17*/
  {PART_LOWPOWER_PARA,             236*1024,         1*1024,     EMMC_USER_PART},/* lowpower_para    1M      p18*/
  {PART_DICE,                      237*1024,          1*1024,    EMMC_USER_PART},/* dice             1M      p19*/
  {PART_RESERVED2,                 238*1024,         25*1024,    EMMC_USER_PART},/* reserved2       25M      p20*/
  {PART_SPLASH2,                   263*1024,         80*1024,    EMMC_USER_PART},/* splash2         80M      p21*/
  {PART_BOOTFAIL_INFO,             343*1024,          2*1024,    EMMC_USER_PART},/* bootfail info   2MB      p22*/
  {PART_MISC,                      345*1024,          2*1024,    EMMC_USER_PART},/* misc             2M      p23*/
  {PART_DFX,                       347*1024,         16*1024,    EMMC_USER_PART},/* dfx             16M      p24*/
  {PART_RRECORD,                   363*1024,         16*1024,    EMMC_USER_PART},/* rrecord         16M      p25*/
  {PART_CACHE,                     379*1024,        104*1024,    EMMC_USER_PART},/* cache           104M     p26*/
  {PART_FW_LPM3,                   483*1024,            1024,    EMMC_USER_PART},/* fw_lpm3       1024K      p27*/
  {PART_RESERVED3,                 484*1024,            5120,    EMMC_USER_PART},/* reserved3A   5120KB      p28*/
  {PART_NPU,                       489*1024,          8*1024,    EMMC_USER_PART},/* npu              8M      p29*/
  {PART_HIEPS,                     497*1024,          2*1024,    EMMC_USER_PART},/* hieps           2MB      p30*/
  {PART_IVP,                       499*1024,          2*1024,    EMMC_USER_PART},/* ivp             2MB      p31*/
  {PART_HDCP,                      501*1024,          1*1024,    EMMC_USER_PART},/* PART_HDCP        1M      p32*/
  {PART_HISEE_IMG,                 502*1024,          4*1024,    EMMC_USER_PART},/* part_hisee_img   4M      p33*/
  {PART_HHEE,                      506*1024,          4*1024,    EMMC_USER_PART},/* hhee             4M      p34*/
  {PART_HISEE_FS,                  510*1024,          8*1024,    EMMC_USER_PART},/* hisee_fs         8M      p35*/
  {PART_FASTBOOT,                  518*1024,         12*1024,    EMMC_USER_PART},/* fastboot        12M      p36*/
  {PART_VECTOR,                    530*1024,          4*1024,    EMMC_USER_PART},/* vector           4M      p37*/
  {PART_ISP_BOOT,                  534*1024,          2*1024,    EMMC_USER_PART},/* isp_boot         2M      p38*/
  {PART_ISP_FIRMWARE,              536*1024,         14*1024,    EMMC_USER_PART},/* isp_firmware    14M      p39*/
  {PART_FW_HIFI,                   550*1024,         12*1024,    EMMC_USER_PART},/* hifi            12M      p40*/
  {PART_TEEOS,                     562*1024,          8*1024,    EMMC_USER_PART},/* teeos           8M       p41*/
  {PART_SENSORHUB,                 570*1024,         16*1024,    EMMC_USER_PART},/* sensorhub       16M      p42*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,          586*1024,         24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p43*/
  {PART_ERECOVERY_RAMDISK,         610*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,          642*1024,          8*1024,    EMMC_USER_PART},/* erecovery_vendor  8M     p45*/
  {PART_KERNEL,                    650*1024,         32*1024,    EMMC_USER_PART},/* kernel            32M    p46*/
#else
  {PART_ERECOVERY_KERNEL,          586*1024,         24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p43*/
  {PART_ERECOVERY_RAMDISK,         610*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,          642*1024,         16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p45*/
  {PART_KERNEL,                    658*1024,         24*1024,    EMMC_USER_PART},/* kernel            24M    p46*/
#endif
  {PART_ENG_SYSTEM,                682*1024,         12*1024,    EMMC_USER_PART},/* eng_system        12M    p47*/
  {PART_RESERVED,                  694*1024,          5*1024,    EMMC_USER_PART},/* reserved           5M    p48*/
  {PART_RECOVERY_RAMDISK,          699*1024,         32*1024,    EMMC_USER_PART},/* recovery_ramdisk 32M     p49*/
  {PART_RECOVERY_VENDOR,           731*1024,         16*1024,    EMMC_USER_PART},/* recovery_vendor 16M      p50*/
  {PART_DTS,                       747*1024,          1*1024,    EMMC_USER_PART},/* dtimage         1M       p51*/
  {PART_DTO,                       748*1024,         20*1024,    EMMC_USER_PART},/* dtoimage       20M       p52*/
  {PART_TRUSTFIRMWARE,             768*1024,          2*1024,    EMMC_USER_PART},/* trustfirmware   2M       p53*/
  {PART_MODEM_FW,                  770*1024,        134*1024,    EMMC_USER_PART},/* modem_fw       134M      p54*/
  {PART_ENG_VENDOR,                904*1024,         20*1024,    EMMC_USER_PART},/* eng_vendor      20M      p55*/
  {PART_MODEM_PATCH_NV,            924*1024,         16*1024,    EMMC_USER_PART},/* modem_patch_nv    16M    p56*/
  {PART_MODEM_DRIVER,              940*1024,         20*1024,    EMMC_USER_PART},/* modem_driver      20M    p57*/
  {PART_RECOVERY_VBMETA,           960*1024,          2*1024,    EMMC_USER_PART},/* recovery_vbmeta   2M     p58*/
  {PART_ERECOVERY_VBMETA,          962*1024,          2*1024,    EMMC_USER_PART},/* erecovery_vbmeta   2M    p59*/
  {PART_VBMETA,                    964*1024,          4*1024,    EMMC_USER_PART},/* PART_VBMETA     4M       p60*/
  {PART_MODEMNVM_UPDATE,           968*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_update   16M    p61*/
  {PART_MODEMNVM_CUST,             984*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_cust   16M      p62*/
  {PART_PATCH,                    1000*1024,         32*1024,    EMMC_USER_PART},/* patch           32M      p63*/
  {PART_VENDOR,                   1032*1024,        760*1024,    EMMC_USER_PART},/* vendor          760M     p64*/
  {PART_ODM,                      1792*1024,        192*1024,    EMMC_USER_PART},/* odm             192M     p65*/
  {PART_CUST,                     1984*1024,        192*1024,    EMMC_USER_PART},/* cust            192M     p66*/
#ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                   2176*1024,       4784*1024,    EMMC_USER_PART},/* system          4784M    p67*/
  {PART_PRODUCT,                  6960*1024,       1472*1024,    EMMC_USER_PART},/* product         1472M    p68*/
  {PART_VERSION,                  8432*1024,        576*1024,    EMMC_USER_PART},/* version         576M     p69*/
  {PART_PRELOAD,                  9008*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                10152*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G       p71*/
#elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                   2176*1024,       5848*1024,    EMMC_USER_PART},/* system          5848M    p67*/
  {PART_PRODUCT,                  8024*1024,       1328*1024,    EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                  9352*1024,        576*1024,    EMMC_USER_PART},/* version         576M     p69*/
  {PART_PRELOAD,                  9928*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                11072*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G       p71*/
#elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                   2176*1024,       4784*1024,    EMMC_USER_PART},/* system          4784M    p67*/
  {PART_PRODUCT,                  6960*1024,       1472*1024,    EMMC_USER_PART},/* product         1472M    p68*/
  {PART_VERSION,                  8432*1024,        576*1024,    EMMC_USER_PART},/* version         576M     p69*/
  {PART_PRELOAD,                  9008*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p70*/
  {PART_USERDATA,                10152*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G       p71*/
#else
  {PART_SYSTEM,                   2176*1024,       5848*1024,    EMMC_USER_PART},/* system          5848M    p67*/
  {PART_PRODUCT,                  8024*1024,       1328*1024,    EMMC_USER_PART},/* product         1328M    p68*/
  {PART_VERSION,                  9352*1024,        576*1024,    EMMC_USER_PART},/* version         576M     p69*/
  {PART_PRELOAD,                  9928*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p70*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,             11072*1024,        128*1024,    EMMC_USER_PART},/* hibench_img    128M      p71*/
  {PART_HIBENCH_DATA,            11200*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M      p72*/
  {PART_FLASH_AGEING,            11712*1024,        512*1024,    EMMC_USER_PART},/* FLASH_AGEING   512M      p73*/
  {PART_HIBENCH_LOG,             12224*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG    32M       p74*/
  {PART_USERDATA,                12256*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G        p75*/
  #else
  {PART_USERDATA,                11072*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G        p71*/
  #endif
  #endif

  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER,                    0,                2*1024,     UFS_PART_0},
  {PART_RESERVED0,                  0,                2*1024,     UFS_PART_1},
  {PART_PTABLE,                     0,                512,        UFS_PART_2},/* ptable          512K     */
  {PART_FRP,                        512,              512,        UFS_PART_2},/* frp             512K     p1*/
  {PART_PERSIST,                    1*1024,           6*1024,     UFS_PART_2},/* persist         6144K    p2*/
  {PART_RESERVED1,                  7*1024,           1024,       UFS_PART_2},/* reserved1       1024K    p3*/
  {PART_PTABLE_LU3,                 0,                512,        UFS_PART_3},/* ptable_lu3      512K     p0*/
  {PART_VRL,                        512,              512,        UFS_PART_3},/* vrl             512K     p1*/
  {PART_VRL_BACKUP,                 1024,             512,        UFS_PART_3},/* vrl backup      512K     p2*/
  {PART_MODEM_SECURE,               1536,             8704,       UFS_PART_3},/* modem_secure    8704K    p3*/
  {PART_NVME,                       10*1024,          5*1024,     UFS_PART_3},/* nvme            5M       p4*/
  {PART_CTF,                        15*1024,          1*1024,     UFS_PART_3},/* PART_CTF        1M       p5*/
  {PART_OEMINFO,                    16*1024,          96*1024,    UFS_PART_3},/* oeminfo         96M      p6*/
  {PART_SECURE_STORAGE,             112*1024,         32*1024,    UFS_PART_3},/* secure storage  32M      p7*/
  {PART_MODEMNVM_FACTORY,           144*1024,         16*1024,    UFS_PART_3},/* modemnvm factory16M      p8*/
  {PART_MODEMNVM_BACKUP,            160*1024,         16*1024,    UFS_PART_3},/* modemnvm backup 16M      p9*/
  {PART_MODEMNVM_IMG,               176*1024,         46*1024,    UFS_PART_3},/* modemnvm img    46M      p10*/
  {PART_HISEE_ENCOS,                222*1024,         4*1024,     UFS_PART_3},/* hisee_encos     4M       p11*/
  {PART_VERITYKEY,                  226*1024,         1*1024,     UFS_PART_3},/* reserved2       1M       p12*/
  {PART_DDR_PARA,                   227*1024,         1*1024,     UFS_PART_3},/* DDR_PARA        1M       p13*/
  {PART_LOWPOWER_PARA,              228*1024,         1*1024,     UFS_PART_3},/* lowpower_para   1M       p14*/
  {PART_DICE,                       229*1024,          1*1024,    UFS_PART_3},/* dice             1M      p15*/
  {PART_RESERVED2,                  230*1024,         25*1024,    UFS_PART_3},/* reserved2       25M      p16*/
  {PART_SPLASH2,                    255*1024,         80*1024,    UFS_PART_3},/* splash2         80M      p17*/
  {PART_BOOTFAIL_INFO,              335*1024,          2*1024,    UFS_PART_3},/* bootfail info   2MB      p18*/
  {PART_MISC,                       337*1024,          2*1024,    UFS_PART_3},/* misc            2M       p19*/
  {PART_DFX,                        339*1024,         16*1024,    UFS_PART_3},/* dfx             16M      p20*/
  {PART_RRECORD,                    355*1024,         16*1024,    UFS_PART_3},/* rrecord         16M      p21*/
  {PART_CACHE,                      371*1024,        104*1024,    UFS_PART_3},/* cache          104M      p22*/
  {PART_FW_LPM3,                    475*1024,            1024,    UFS_PART_3},/* mcuimage       1024K     p23*/
  {PART_RESERVED3,                  476*1024,            5120,    UFS_PART_3},/* reserved3A     5120K     p24*/
  {PART_NPU,                        481*1024,          8*1024,    UFS_PART_3},/* npu        8M            p25*/
  {PART_HIEPS,                      489*1024,          2*1024,    UFS_PART_3},/* hieps           2MB      p26*/
  {PART_IVP,                        491*1024,          2*1024,    UFS_PART_3},/* PART_HDCP       1M       p27*/
  {PART_HDCP,                       493*1024,          1*1024,    UFS_PART_3},/* PART_HDCP       1M       p28*/
  {PART_HISEE_IMG,                  494*1024,          4*1024,    UFS_PART_3},/* part_hisee_img   4M      p29*/
  {PART_HHEE,                       498*1024,          4*1024,    UFS_PART_3},/* PART_RESERVED10  4M      p30*/
  {PART_HISEE_FS,                   502*1024,          8*1024,    UFS_PART_3},/* hisee_fs        8M       p31*/
  {PART_FASTBOOT,                   510*1024,         12*1024,    UFS_PART_3},/* fastboot        12M      p32*/
  {PART_VECTOR,                     522*1024,          4*1024,    UFS_PART_3},/* avs vector      4M       p33*/
  {PART_ISP_BOOT,                   526*1024,          2*1024,    UFS_PART_3},/* isp_boot        2M       p34*/
  {PART_ISP_FIRMWARE,               528*1024,         14*1024,    UFS_PART_3},/* isp_firmware    14M      p35*/
  {PART_FW_HIFI,                    542*1024,         12*1024,    UFS_PART_3},/* hifi            12M      p36*/
  {PART_TEEOS,                      554*1024,          8*1024,    UFS_PART_3},/* teeos           8M       p37*/
  {PART_SENSORHUB,                  562*1024,         16*1024,    UFS_PART_3},/* sensorhub       16M      p38*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           578*1024,         24*1024,    UFS_PART_3},/* erecovery_kernel  24M    p39*/
  {PART_ERECOVERY_RAMDISK,          602*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p40*/
  {PART_ERECOVERY_VENDOR,           634*1024,          8*1024,    UFS_PART_3},/* erecovery_vendor  8M     p41*/
  {PART_KERNEL,                     642*1024,         32*1024,    UFS_PART_3},/* kernel            32M    p42*/
#else
  {PART_ERECOVERY_KERNEL,           578*1024,         24*1024,    UFS_PART_3},/* erecovery_kernel  24M    p39*/
  {PART_ERECOVERY_RAMDISK,          602*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p40*/
  {PART_ERECOVERY_VENDOR,           634*1024,         16*1024,    UFS_PART_3},/* erecovery_vendor  16M    p41*/
  {PART_KERNEL,                     650*1024,         24*1024,    UFS_PART_3},/* kernel            24M    p42*/
#endif
  {PART_ENG_SYSTEM,                 674*1024,         12*1024,    UFS_PART_3},/* eng_system         12M   p43*/
  {PART_RESERVED,                   686*1024,          5*1024,    UFS_PART_3},/* reserved            5M   p44*/
  {PART_RECOVERY_RAMDISK,           691*1024,         32*1024,    UFS_PART_3},/* recovery_ramdisk   32M   p45*/
  {PART_RECOVERY_VENDOR,            723*1024,         16*1024,    UFS_PART_3},/* recovery_vendor    16M   p46*/
  {PART_DTS,                        739*1024,          1*1024,    UFS_PART_3},/* dtimage             1M   p47*/
  {PART_DTO,                        740*1024,         20*1024,    UFS_PART_3},/* dtoimage           20M   p48*/
  {PART_TRUSTFIRMWARE,              760*1024,          2*1024,    UFS_PART_3},/* trustfirmware       2M   p49*/
  {PART_MODEM_FW,                   762*1024,        134*1024,    UFS_PART_3},/* modem_fw          134M   p50*/
  {PART_ENG_VENDOR,                 896*1024,         20*1024,    UFS_PART_3},/* eng_verdor         20M   p51*/
  {PART_MODEM_PATCH_NV,             916*1024,         16*1024,    UFS_PART_3},/* modem_patch_nv     16M   p52*/
  {PART_MODEM_DRIVER,               932*1024,         20*1024,    UFS_PART_3},/* modem_driver       20M   p53*/
  {PART_RECOVERY_VBMETA,            952*1024,          2*1024,    UFS_PART_3},/* recovery_vbmeta     2M   p54*/
  {PART_ERECOVERY_VBMETA,           954*1024,          2*1024,    UFS_PART_3},/* erecovery_vbmeta    2M   p55*/
  {PART_VBMETA,                     956*1024,          4*1024,    UFS_PART_3},/* vbmeta              4M   p56*/
  {PART_MODEMNVM_UPDATE,            960*1024,         16*1024,    UFS_PART_3},/* modemnvm_update    16M   p57*/
  {PART_MODEMNVM_CUST,              976*1024,         16*1024,    UFS_PART_3},/* modemnvm_cust      16M   p58*/
  {PART_PATCH,                      992*1024,         32*1024,    UFS_PART_3},/* patch              32M   p59*/
  {PART_VENDOR,                    1024*1024,        760*1024,    UFS_PART_3},/* vendor            760M   p60*/
  {PART_ODM,                       1784*1024,        192*1024,    UFS_PART_3},/* odm               192M   p61*/
  {PART_CUST,                      1976*1024,        192*1024,    UFS_PART_3},/* cust              192M   p62*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2168*1024,       4784*1024,    UFS_PART_3},/* system           4784M   p63*/
  {PART_PRODUCT,                   6952*1024,       1472*1024,    UFS_PART_3},/* product          1472M   p64*/
  {PART_VERSION,                   8424*1024,        576*1024,    UFS_PART_3},/* version           576M   p65*/
  {PART_PRELOAD,                   9000*1024,       1144*1024,    UFS_PART_3},/* preload          1144M   p66*/
  {PART_USERDATA,                 10144*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata            4G   p67*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2168*1024,       5848*1024,    UFS_PART_3},/* system           5848M   p63*/
  {PART_PRODUCT,                   8016*1024,       1328*1024,    UFS_PART_3},/* product          1328M   p64*/
  {PART_VERSION,                   9344*1024,        576*1024,    UFS_PART_3},/* version           576M   p65*/
  {PART_PRELOAD,                   9920*1024,       1144*1024,    UFS_PART_3},/* preload          1144M   p66*/
  {PART_USERDATA,                 11064*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata            4G   p67*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2168*1024,       4784*1024,    UFS_PART_3},/* system           4784M   p63*/
  {PART_PRODUCT,                   6952*1024,       1472*1024,    UFS_PART_3},/* product          1472M   p64*/
  {PART_VERSION,                   8424*1024,        576*1024,    UFS_PART_3},/* version           576M   p65*/
  {PART_PRELOAD,                   9000*1024,       1144*1024,    UFS_PART_3},/* preload          1144M   p66*/
  {PART_USERDATA,                 10144*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata            4G   p67*/
  #else
  {PART_SYSTEM,                    2168*1024,       5848*1024,    UFS_PART_3},/* system           5848M   p63*/
  {PART_PRODUCT,                   8016*1024,       1328*1024,    UFS_PART_3},/* product          1328M   p64*/
  {PART_VERSION,                   9344*1024,        576*1024,    UFS_PART_3},/* version           576M   p65*/
  {PART_PRELOAD,                   9920*1024,       1144*1024,    UFS_PART_3},/* preload          1144M   p66*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,              11064*1024,        128*1024,    UFS_PART_3},/* hibench_img       128M   p67*/
  {PART_HIBENCH_DATA,             11192*1024,        512*1024,    UFS_PART_3},/* hibench_data      512M   p68*/
  {PART_FLASH_AGEING,             11704*1024,        512*1024,    UFS_PART_3},/* FLASH_AGEING      512M   p69*/
  {PART_HIBENCH_LOG,              12216*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG       32M    p70*/
  {PART_USERDATA,                 12248*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata            4G   p71*/
  #else
  {PART_USERDATA,                 11064*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata            4G   p67*/
  #endif
  #endif
  {"0", 0, 0, 0},
};
#endif

#endif
