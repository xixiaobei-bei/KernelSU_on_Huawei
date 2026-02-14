#ifndef _TAURUS_PARTITION_H_
#define _TAURUS_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER_A,        0,         2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_XLOADER_B,        0,         2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,           0,         512,           EMMC_USER_PART},/* ptable          512K */
  {PART_FRP,              512,       512,           EMMC_USER_PART},/* frp             512K   p1*/
  {PART_PERSIST,          1024,      6*1024,        EMMC_USER_PART},/* persist         6M     p2*/
  {PART_RESERVED1,        7*1024,    1024,          EMMC_USER_PART},/* reserved1       1024K  p3*/
  {PART_RESERVED6,        8*1024,    512,           EMMC_USER_PART},/* reserved6       512K   p4*/
  {PART_VRL,              8704,      512,           EMMC_USER_PART},/* vrl             512K   p5*/
  {PART_VRL_BACKUP,       9*1024,    512,           EMMC_USER_PART},/* vrl backup      512K   p6*/
  {PART_MODEM_SECURE,     9728,      8704,          EMMC_USER_PART},/* modem_secure    8704K  p7*/
  {PART_NVME,             18*1024,   5*1024,        EMMC_USER_PART},/* nvme            5M     p8*/
  {PART_CTF,              23*1024,   1*1024,        EMMC_USER_PART},/* ctf             1M     p9*/
  {PART_OEMINFO,          24*1024,   96*1024,       EMMC_USER_PART},/* oeminfo         96M    p10*/
  {PART_SECURE_STORAGE,   120*1024,  32*1024,       EMMC_USER_PART},/* secure storage  32M    p11*/
  {PART_MODEMNVM_FACTORY, 152*1024,  16*1024,       EMMC_USER_PART},/* modemnvm factory16M    p12*/
  {PART_MODEMNVM_BACKUP,  168*1024,  16*1024,       EMMC_USER_PART},/* modemnvm backup 16M    p13*/
  {PART_MODEMNVM_IMG,     184*1024,  34*1024,       EMMC_USER_PART},/* modemnvm img    34M    p14*/
  {PART_HISEE_ENCOS,      218*1024,  4*1024,        EMMC_USER_PART},/* hisee_encos      4M    p15*/
  {PART_VERITYKEY,        222*1024,  1*1024,        EMMC_USER_PART},/* veritykey        1M    p16*/
  {PART_DDR_PARA,         223*1024,  1*1024,        EMMC_USER_PART},/* DDR_PARA         1M    p17*/
  {PART_RESERVED2,        224*1024,  29*1024,       EMMC_USER_PART},/* reserved2       27M    p18*/
  {PART_SPLASH2,          253*1024,  80*1024,       EMMC_USER_PART},/* splash2         80M    p19*/
  {PART_BOOTFAIL_INFO,    333*1024,  2*1024,        EMMC_USER_PART},/* bootfail info   2MB    p20*/
  {PART_MISC,             335*1024,  2*1024,        EMMC_USER_PART},/* misc            2M     p21*/
  {PART_DFX,              337*1024,  16*1024,       EMMC_USER_PART},/* dfx             16M    p22*/
  {PART_RRECORD,          353*1024,  16*1024,       EMMC_USER_PART},/* rrecord         16M    p23*/
  {PART_FW_LPM3_A,        369*1024,  256,           EMMC_USER_PART},/* fw_lpm3_a      256K    p24*/
  {PART_RESERVED3_A,      378112,    7936,          EMMC_USER_PART},/* reserved3A   7936KB    p25*/
  {PART_IVP,              377*1024,  2*1024,        EMMC_USER_PART},/* ivp             2MB    p26*/
  {PART_HDCP_A,           379*1024,  1*1024,        EMMC_USER_PART},/* PART_HDCP_A      1M    p27*/
  {PART_HISEE_IMG_A,      380*1024,  4*1024,        EMMC_USER_PART},/* part_hisee_img_a 4M    p28*/
  {PART_HHEE_A,           384*1024,  4*1024,        EMMC_USER_PART},/* hhee_a           4M    p29*/
  {PART_HISEE_FS_A,       388*1024,  8*1024,        EMMC_USER_PART},/* hisee_fs         8M    p30*/
  {PART_FASTBOOT_A,       396*1024,  12*1024,       EMMC_USER_PART},/* fastboot        12M    p31*/
  {PART_VECTOR_A,         408*1024,  4*1024,        EMMC_USER_PART},/* vector_a         4M    p32*/
  {PART_ISP_BOOT_A,       412*1024,  2*1024,        EMMC_USER_PART},/* isp_boot         2M    p33*/
  {PART_ISP_FIRMWARE_A,   414*1024,  14*1024,       EMMC_USER_PART},/* isp_firmware    14M    p34*/
  {PART_FW_HIFI_A,        428*1024,  12*1024,       EMMC_USER_PART},/* hifi            12M    p35*/
  {PART_TEEOS_A,          440*1024,  8*1024,        EMMC_USER_PART},/* teeos           8M     p36*/
  {PART_SENSORHUB_A,      448*1024,  16*1024,       EMMC_USER_PART},/* sensorhub       16M    p37*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,  464*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p38*/
  {PART_ERECOVERY_RAMDISK_A, 488*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p39*/
  {PART_ERECOVERY_VENDOR_A,  520*1024,  8*1024,     EMMC_USER_PART},/* erecovery_vendor  8M     p40*/
  {PART_KERNEL_A,            528*1024,  32*1024,    EMMC_USER_PART},/* kernel            32M    p41*/
#else
  {PART_ERECOVERY_KERNEL_A,  464*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p38*/
  {PART_ERECOVERY_RAMDISK_A, 488*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p39*/
  {PART_ERECOVERY_VENDOR_A,  520*1024,  16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p40*/
  {PART_KERNEL_A,            536*1024,  24*1024,    EMMC_USER_PART},/* kernel            24M    p41*/
#endif
  {PART_ENG_SYSTEM_A,       560*1024,  12*1024,     EMMC_USER_PART},/* eng_system        12M    p42*/
  {PART_RESERVED,           572*1024,   3*1024,     EMMC_USER_PART},/* reserved           3M    p43*/
  {PART_RECOVERY_RAMDISK_A, 575*1024,  32*1024,     EMMC_USER_PART},/* recovery_ramdisk 32M   p44*/
  {PART_RECOVERY_VENDOR_A,  607*1024,  16*1024,     EMMC_USER_PART},/* recovery_vendor 16M    p45*/
  {PART_DTS_A,              623*1024,   1*1024,     EMMC_USER_PART},/* dtimage         1M     p46*/
  {PART_DTO_A,              624*1024,  20*1024,     EMMC_USER_PART},/* dtoimage       20M     p47*/
  {PART_TRUSTFIRMWARE_A,    644*1024,  2*1024,      EMMC_USER_PART},/* trustfirmware   2M     p48*/
  {PART_MODEM_FW_A,         646*1024,  56*1024,     EMMC_USER_PART},/* modem_fw        56M    p49*/
  {PART_ENG_VENDOR_A,       702*1024,  20*1024,     EMMC_USER_PART},/* eng_vendor      20M    p50*/
  {PART_RECOVERY_VBMETA_A,  722*1024,  2*1024,      EMMC_USER_PART},/* recovery_vbmeta_a 2M   p51*/
  {PART_ERECOVERY_VBMETA_A, 724*1024,  2*1024,      EMMC_USER_PART},/* erecovery_vbmeta_a 2M  p52*/
  {PART_VBMETA_A,           726*1024,  4*1024,      EMMC_USER_PART},/* PART_VBMETA_A   4M     p53*/
  {PART_MODEMNVM_UPDATE_A,  730*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_update_a 16M  p54*/
  {PART_MODEMNVM_CUST_A,    746*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_cust_a 16M    p55*/
  {PART_PATCH_A,            762*1024,  32*1024,     EMMC_USER_PART},/* patch           32M    p56*/
  {PART_VENDOR_A,           794*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p57*/
  {PART_ODM_A,              1554*1024, 192*1024,    EMMC_USER_PART},/* odm             192M   p58*/
  {PART_CACHE,              1746*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p59*/
  {PART_CUST_A,             1850*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p60*/
#ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,           2042*1024,       3852*1024,    EMMC_USER_PART},/* system          3852M  p61*/
  {PART_PRODUCT_A,          5894*1024,        192*1024,    EMMC_USER_PART},/* product         192M   p62*/
  {PART_VERSION_A,          6086*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p63*/
  {PART_PRELOAD_A,          6118*1024,         64*1024,    EMMC_USER_PART},/* preload_a       64M    p64*/
  {PART_USERDATA,           6182*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G     p65*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM_A,           2042*1024,       5632*1024,    EMMC_USER_PART},/* system          5632M  p61*/
  {PART_PRODUCT_A,          7674*1024,        192*1024,    EMMC_USER_PART},/* product         192M   p62*/
  {PART_VERSION_A,          7866*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p63*/
  {PART_PRELOAD_A,          7898*1024,         64*1024,    EMMC_USER_PART},/* preload_a       64M    p64*/
  {PART_USERDATA,           7962*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata         4G    p65*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM_A,           2042*1024,       4752*1024,    EMMC_USER_PART},/* system          4752M  p61*/
  {PART_PRODUCT_A,          6794*1024,        192*1024,    EMMC_USER_PART},/* product         192M   p62*/
  {PART_VERSION_A,          6986*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p63*/
  {PART_PRELOAD_A,          7018*1024,         64*1024,    EMMC_USER_PART},/* preload_a       64M    p64*/
  {PART_USERDATA,           7082*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata         4G    p65*/
  #else
  {PART_SYSTEM_A,           2042*1024,       4988*1024,    EMMC_USER_PART},/* system          4988M  p61*/
  {PART_PRODUCT_A,          7030*1024,        192*1024,    EMMC_USER_PART},/* product         192M   p62*/
  {PART_VERSION_A,          7222*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p63*/
  {PART_PRELOAD_A,          7254*1024,         64*1024,    EMMC_USER_PART},/* preload_a       64M    p64*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_RESERVED5,          7318*1024,        128*1024,    EMMC_USER_PART},/* reserved5      128M    p65*/
  {PART_HIBENCH_DATA,       7446*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M    p66*/
  {PART_FLASH_AGEING,       7958*1024,        512*1024,    EMMC_USER_PART},/* FLASH_AGEING   512M    p67*/
  {PART_USERDATA,           8470*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G    p68*/
  #else
  {PART_USERDATA,           7318*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G    p65*/
  #endif
  #endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,        0,         2*1024,        UFS_PART_0},
  {PART_XLOADER_B,        0,         2*1024,        UFS_PART_1},
  {PART_PTABLE,           0,         512,           UFS_PART_2},/* ptable          512K     */
  {PART_FRP,              512,       512,           UFS_PART_2},/* frp             512K   p1*/
  {PART_PERSIST,          1*1024,    6*1024,        UFS_PART_2},/* persist         6144K  p2*/
  {PART_RESERVED1,        7*1024,    1024,          UFS_PART_2},/* reserved1       1024K  p3*/
  {PART_PTABLE_LU3,       0,         512,           UFS_PART_3},/* ptable_lu3      512K   p0*/
  {PART_VRL,              512,       512,           UFS_PART_3},/* vrl             512K   p1*/
  {PART_VRL_BACKUP,       1024,      512,           UFS_PART_3},/* vrl backup      512K   p2*/
  {PART_MODEM_SECURE,     1536,      8704,          UFS_PART_3},/* modem_secure    8704K  p3*/
  {PART_NVME,             10*1024,   5*1024,        UFS_PART_3},/* nvme            5M     p4*/
  {PART_CTF,              15*1024,   1*1024,        UFS_PART_3},/* PART_CTF        1M     p5*/
  {PART_OEMINFO,          16*1024,   96*1024,       UFS_PART_3},/* oeminfo         96M    p6*/
  {PART_SECURE_STORAGE,   112*1024,  32*1024,       UFS_PART_3},/* secure storage  32M    p7*/
  {PART_MODEMNVM_FACTORY, 144*1024,  16*1024,       UFS_PART_3},/* modemnvm factory16M    p8*/
  {PART_MODEMNVM_BACKUP,  160*1024,  16*1024,       UFS_PART_3},/* modemnvm backup 16M    p9*/
  {PART_MODEMNVM_IMG,     176*1024,  34*1024,       UFS_PART_3},/* modemnvm img    34M    p10*/
  {PART_HISEE_ENCOS,      210*1024,  4*1024,        UFS_PART_3},/* hisee_encos     4M     p11*/
  {PART_VERITYKEY,        214*1024,  1*1024,        UFS_PART_3},/* reserved2       1M     p12*/
  {PART_DDR_PARA,         215*1024,  1*1024,        UFS_PART_3},/* DDR_PARA        1M     p13*/
  {PART_RESERVED2,        216*1024,  27*1024,       UFS_PART_3},/* reserved2       27M    p14*/
  {PART_SPLASH2,          243*1024,  80*1024,       UFS_PART_3},/* splash2         80M    p15*/
  {PART_BOOTFAIL_INFO,    323*1024,  2*1024,        UFS_PART_3},/* bootfail info   2MB    p16*/
  {PART_MISC,             325*1024,  2*1024,        UFS_PART_3},/* misc            2M     p17*/
  {PART_DFX,              327*1024,  16*1024,       UFS_PART_3},/* dfx             16M    p18*/
  {PART_RRECORD,          343*1024,  16*1024,       UFS_PART_3},/* rrecord         16M    p19*/
  {PART_FW_LPM3_A,        359*1024,  256,           UFS_PART_3},/* mcuimage        256K   p20*/
  {PART_RESERVED3_A,      367872,    7936,          UFS_PART_3},/* reserved3A      7936K  p21*/
  {PART_IVP,              367*1024,  2*1024,        UFS_PART_3},/* PART_HDCP_A     1M     p22*/
  {PART_HDCP_A,           369*1024,  1*1024,        UFS_PART_3},/* PART_HDCP_A     1M     p23*/
  {PART_HISEE_IMG_A,      370*1024,  4*1024,        UFS_PART_3},/* part_hisee_img_a 4M    p24*/
  {PART_HHEE_A,           374*1024,  4*1024,        UFS_PART_3},/* PART_RESERVED10  4M    p25*/
  {PART_HISEE_FS_A,       378*1024,  8*1024,        UFS_PART_3},/* hisee_fs        8M     p26*/
  {PART_FASTBOOT_A,       386*1024,  12*1024,       UFS_PART_3},/* fastboot        12M    p27*/
  {PART_VECTOR_A,         398*1024,  4*1024,        UFS_PART_3},/* avs vector      4M     p28*/
  {PART_ISP_BOOT_A,       402*1024,  2*1024,        UFS_PART_3},/* isp_boot        2M     p29*/
  {PART_ISP_FIRMWARE_A,   404*1024,  14*1024,       UFS_PART_3},/* isp_firmware    14M    p30*/
  {PART_FW_HIFI_A,        418*1024,  12*1024,       UFS_PART_3},/* hifi            12M    p31*/
  {PART_TEEOS_A,          430*1024,  8*1024,        UFS_PART_3},/* teeos           8M     p32*/
  {PART_SENSORHUB_A,      438*1024,  16*1024,       UFS_PART_3},/* sensorhub       16M    p33*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,  454*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p34*/
  {PART_ERECOVERY_RAMDISK_A, 478*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p35*/
  {PART_ERECOVERY_VENDOR_A,  510*1024,  8*1024,     UFS_PART_3},/* erecovery_vendor  8M    p36*/
  {PART_KERNEL_A,            518*1024,  32*1024,    UFS_PART_3},/* kernel            32M   p37*/
#else
  {PART_ERECOVERY_KERNEL_A,  454*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p34*/
  {PART_ERECOVERY_RAMDISK_A, 478*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p35*/
  {PART_ERECOVERY_VENDOR_A,  510*1024,  16*1024,    UFS_PART_3},/* erecovery_vendor  16M   p36*/
  {PART_KERNEL_A,            526*1024,  24*1024,    UFS_PART_3},/* kernel            24M   p37*/
#endif
  {PART_ENG_SYSTEM_A,       550*1024,  12*1024,     UFS_PART_3},/* eng_system        12M   p38*/
  {PART_RESERVED,           562*1024,  13*1024,     UFS_PART_3},/* reserved          13M   p39*/
  {PART_RECOVERY_RAMDISK_A, 575*1024,  32*1024,     UFS_PART_3},/* recovery_ramdisk 32M    p40*/
  {PART_RECOVERY_VENDOR_A,  607*1024,  16*1024,     UFS_PART_3},/* recovery_vendor  16M    p41*/
  {PART_DTS_A,              623*1024,   1*1024,     UFS_PART_3},/* dtimage           1M    p42*/
  {PART_DTO_A,              624*1024,  20*1024,     UFS_PART_3},/* dtoimage         20M    p43*/
  {PART_TRUSTFIRMWARE_A,    644*1024,  2*1024,      UFS_PART_3},/* trustfirmware     2M    p44*/
  {PART_MODEM_FW_A,         646*1024,  56*1024,     UFS_PART_3},/* modem_fw         56M    p45*/
  {PART_ENG_VENDOR_A,       702*1024,  20*1024,     UFS_PART_3},/* eng_verdor       20M    p46*/
  {PART_RECOVERY_VBMETA_A,  722*1024,  2*1024,      UFS_PART_3},/* recovery_vbmeta_a  2M   p47*/
  {PART_ERECOVERY_VBMETA_A, 724*1024,  2*1024,      UFS_PART_3},/* erecovery_vbmeta_a 2M   p48*/
  {PART_VBMETA_A,           726*1024,  4*1024,      UFS_PART_3},/* vbmeta_a         4M     p49*/
  {PART_MODEMNVM_UPDATE_A,  730*1024,  16*1024,     UFS_PART_3},/* modemnvm_update_a  16M  p50*/
  {PART_MODEMNVM_CUST_A,    746*1024,  16*1024,     UFS_PART_3},/* modemnvm_cust_a    16M  p51*/
  {PART_PATCH_A,            762*1024,  32*1024,     UFS_PART_3},/* patch            32M    p52*/
  {PART_VENDOR_A,           794*1024,  760*1024,    UFS_PART_3},/* vendor           760M   p53*/
  {PART_ODM_A,              1554*1024, 192*1024,    UFS_PART_3},/* odm              192M   p54*/
  {PART_CACHE,              1746*1024, 104*1024,    UFS_PART_3},/* cache            104M   p55*/
  {PART_CUST_A,             1850*1024, 192*1024,    UFS_PART_3},/* cust             192M   p56*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,           2042*1024, 3852*1024,      UFS_PART_3},/* system       3852M   p57*/
  {PART_PRODUCT_A,          5894*1024, 192*1024,       UFS_PART_3},/* product       192M   p58*/
  {PART_VERSION_A,          6086*1024,  32*1024,       UFS_PART_3},/* version        32M   p59*/
  {PART_PRELOAD_A,          6118*1024,  64*1024,       UFS_PART_3},/* version        64M   p60*/
  {PART_USERDATA,           6182*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p61*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM_A,           2042*1024, 5632*1024,      UFS_PART_3},/* system       5632M   p57*/
  {PART_PRODUCT_A,          7674*1024, 192*1024,       UFS_PART_3},/* product       192M   p58*/
  {PART_VERSION_A,          7866*1024,  32*1024,       UFS_PART_3},/* version        32M   p59*/
  {PART_PRELOAD_A,          7898*1024,  64*1024,       UFS_PART_3},/* version        64M   p60*/
  {PART_USERDATA,           7962*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p61*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM_A,           2042*1024, 4752*1024,      UFS_PART_3},/* system       4752M   p57*/
  {PART_PRODUCT_A,          6794*1024, 192*1024,       UFS_PART_3},/* product       192M   p58*/
  {PART_VERSION_A,          6986*1024,  32*1024,       UFS_PART_3},/* version        32M   p59*/
  {PART_PRELOAD_A,          7018*1024,  64*1024,       UFS_PART_3},/* version        64M   p60*/
  {PART_USERDATA,           7082*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p61*/
  #else
  {PART_SYSTEM_A,           2042*1024, 4988*1024,      UFS_PART_3},/* system       4988M   p57*/
  {PART_PRODUCT_A,          7030*1024, 192*1024,       UFS_PART_3},/* product       192M   p58*/
  {PART_VERSION_A,          7222*1024,  32*1024,       UFS_PART_3},/* version        32M   p59*/
  {PART_PRELOAD_A,          7254*1024,  64*1024,       UFS_PART_3},/* version        64M   p60*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_RESERVED5,          7318*1024, 128*1024,       UFS_PART_3},/* reserved5     128M   p61*/
  {PART_HIBENCH_DATA,       7446*1024, 512*1024,       UFS_PART_3},/* hibench_data  512M   p62*/
  {PART_FLASH_AGEING,       7958*1024, 512*1024,       UFS_PART_3},/* FLASH_AGEING  512M   p63*/
  {PART_USERDATA,           8470*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p64*/
  #else
  {PART_USERDATA,           7318*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G    p61*/
  #endif
  #endif
  {"0", 0, 0, 0},
};
#endif

#endif
