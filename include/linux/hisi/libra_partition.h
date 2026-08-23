#ifndef _LIBRA_PARTITION_H_
#define _LIBRA_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER_A,        0,         2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_XLOADER_B,        0,         2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,           0,         512,           EMMC_USER_PART},/* ptable          512K */
  {PART_FRP,              512,       512,           EMMC_USER_PART},/* frp             512K   p1*/
  {PART_PERSIST,          1024,      2048,          EMMC_USER_PART},/* persist         2048K  p2*/
  {PART_RESERVED1,        3072,      5120,          EMMC_USER_PART},/* reserved1       5120K  p3*/
  {PART_RESERVED6,        8*1024,    512,           EMMC_USER_PART},/* reserved6       512K   p4*/
  {PART_VRL,              8704,      512,           EMMC_USER_PART},/* vrl             512K   p5*/
  {PART_VRL_BACKUP,       9216,      512,           EMMC_USER_PART},/* vrl backup      512K   p6*/
  {PART_MODEM_SECURE,     9728,      8704,          EMMC_USER_PART},/* modem_secure    8704k  p7*/
  {PART_NVME,             18*1024,   5*1024,        EMMC_USER_PART},/* nvme            5M     p8*/
  {PART_CTF,              23*1024,   1*1024,        EMMC_USER_PART},/* PART_CTF        1M     p9*/
  {PART_OEMINFO,          24*1024,   64*1024,       EMMC_USER_PART},/* oeminfo         64M    p10*/
  {PART_SECURE_STORAGE,   88*1024,   32*1024,       EMMC_USER_PART},/* secure storage  32M    p11*/
  {PART_MODEM_OM,         120*1024,  32*1024,       EMMC_USER_PART},/* modem om        32M    p12*/
  {PART_MODEMNVM_FACTORY, 152*1024,  16*1024,       EMMC_USER_PART},/* modemnvm factory16M    p13*/
  {PART_MODEMNVM_BACKUP,  168*1024,  16*1024,       EMMC_USER_PART},/* modemnvm backup 16M    p14*/
  {PART_MODEMNVM_IMG,     184*1024,  34*1024,       EMMC_USER_PART},/* modemnvm img    34M    p15*/
  {PART_RESERVED7,        218*1024,  2*1024,        EMMC_USER_PART},/* reserved7       2M     p16*/
  {PART_HISEE_ENCOS,      220*1024,  4*1024,        EMMC_USER_PART},/* hisee_encos     4M     p17*/
  {PART_VERITYKEY,        224*1024,  1*1024,        EMMC_USER_PART},/* reserved2       32M    p18*/
  {PART_DDR_PARA,         225*1024,  1*1024,        EMMC_USER_PART},/* DDR_PARA        1M     p19*/
  {PART_MODEM_DRIVER_A,   226*1024,  20*1024,       EMMC_USER_PART},/* modem_driver_a  20M    p20*/
  {PART_RESERVED2,        246*1024,   7*1024,       EMMC_USER_PART},/* reserved2        7M    p21*/
  {PART_SPLASH2,          253*1024,  80*1024,       EMMC_USER_PART},/* splash2         80M    p22*/
  {PART_BOOTFAIL_INFO,    333*1024,  2*1024,        EMMC_USER_PART},/* bootfail info   2MB    p23*/
  {PART_MISC,             335*1024,  2*1024,        EMMC_USER_PART},/* misc            2M     p24*/
  {PART_DFX,              337*1024,  16*1024,       EMMC_USER_PART},/* dfx             16M    p25*/
  {PART_RRECORD,          353*1024,  16*1024,       EMMC_USER_PART},/* rrecord         16M    p26*/
  {PART_FW_LPM3_A,        369*1024,  256,           EMMC_USER_PART},/* mcuimage        256K   p27*/
  {PART_RESERVED3_A,      378112,    3840,          EMMC_USER_PART},/* reserved3A      3840KB p28*/
  {PART_HDCP_A,           373*1024,  1*1024,        EMMC_USER_PART},/* PART_HDCP_A      1M    p29*/
  {PART_HISEE_IMG_A,      374*1024,  4*1024,        EMMC_USER_PART},/* part_hisee_img_a 4M    p30*/
  {PART_HHEE_A,           378*1024,  4*1024,        EMMC_USER_PART},/* PART_RESERVED10  4M    p31*/
  {PART_HISEE_FS_A,       382*1024,  8*1024,        EMMC_USER_PART},/* hisee_fs        8M     p32*/
  {PART_FASTBOOT_A,       390*1024,  12*1024,       EMMC_USER_PART},/* fastboot        12M    p33*/
  {PART_VECTOR_A,         402*1024,  4*1024,        EMMC_USER_PART},/* avs vector      4M     p34*/
  {PART_ISP_BOOT_A,       406*1024,  2*1024,        EMMC_USER_PART},/* isp_boot        2M     p35*/
  {PART_ISP_FIRMWARE_A,   408*1024,  14*1024,       EMMC_USER_PART},/* isp_firmware    14M    p36*/
  {PART_FW_HIFI_A,        422*1024,  12*1024,       EMMC_USER_PART},/* hifi            12M    p37*/
  {PART_TEEOS_A,          434*1024,  8*1024,        EMMC_USER_PART},/* teeos           8M     p38*/
  {PART_SENSORHUB_A,      442*1024,  16*1024,       EMMC_USER_PART},/* sensorhub       16M    p39*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,  458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p40*/
  {PART_ERECOVERY_RAMDISK_A, 482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p41*/
  {PART_ERECOVERY_VENDOR_A,  514*1024,  8*1024,     EMMC_USER_PART},/* erecovery_vendor  8M     p42*/
  {PART_KERNEL_A,            522*1024,  32*1024,    EMMC_USER_PART},/* kernel            32M    p43*/
#else
  {PART_ERECOVERY_KERNEL_A,  458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p40*/
  {PART_ERECOVERY_RAMDISK_A, 482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p41*/
  {PART_ERECOVERY_VENDOR_A,  514*1024,  16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p42*/
  {PART_KERNEL_A,            530*1024,  24*1024,    EMMC_USER_PART},/* kernel            24M    p43*/
#endif
  {PART_ENG_SYSTEM_A,       554*1024,  12*1024,     EMMC_USER_PART},/* eng_system       12M   p44*/
  {PART_RECOVERY_RAMDISK_A, 566*1024,  32*1024,     EMMC_USER_PART},/* recovery_ramdisk 32M   p45*/
  {PART_RECOVERY_VENDOR_A,  598*1024,  16*1024,     EMMC_USER_PART},/* recovery_vendor 16M    p46*/
  {PART_DTS_A,              614*1024,   8*1024,     EMMC_USER_PART},/* dtimage          8M    p47*/
  {PART_DTO_A,              622*1024,  24*1024,     EMMC_USER_PART},/* dtoimage         8M    p48*/
  {PART_TRUSTFIRMWARE_A,    646*1024,  2*1024,      EMMC_USER_PART},/* trustfirmware   2M     p49*/
  {PART_MODEM_FW_A,         648*1024,  56*1024,     EMMC_USER_PART},/* modem_fw        56M    p50*/
  {PART_ENG_VENDOR_A,       704*1024,  12*1024,     EMMC_USER_PART},/* eng_vendor     12M     p51*/
  {PART_MODEM_PATCH_NV_A,   716*1024,   4*1024,     EMMC_USER_PART},/* modem_patch_nv_a 4M    p52*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_RESERVED4_A,        720*1024,  8*1024,     EMMC_USER_PART},/* reserved4       8M    p53*/
#elif (defined(CONFIG_MARKET_INTERNAL) || defined(CONFIG_MARKET_OVERSEA) ||\
defined(CONFIG_MARKET_FULL_INTERNAL) || defined(CONFIG_MARKET_FULL_OVERSEA))
/* rom update*/
  {PART_PRELOAD_A,          720*1024,  8*1024,     EMMC_USER_PART},/* PART_PRELOAD_A   8M  p53*/
#else
  {PART_RESERVED4_A,        720*1024,  8*1024,     EMMC_USER_PART},/* reserved4        8M  p53*/
#endif
  {PART_RECOVERY_VBMETA_A,  728*1024,  2*1024,      EMMC_USER_PART},/* recovery_vbmeta_a 2M   p54*/
  {PART_ERECOVERY_VBMETA_A, 730*1024,  2*1024,      EMMC_USER_PART},/* erecovery_vbmeta_a 2M  p55*/
  {PART_VBMETA_A,           732*1024,  4*1024,      EMMC_USER_PART},/* PART_VBMETA_A   4M     p56*/
  {PART_MODEMNVM_UPDATE_A,  736*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_update_a 16M  p57*/
  {PART_MODEMNVM_CUST_A,    752*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_cust_a 16M    p58*/
  {PART_PATCH_A,            768*1024,  32*1024,     EMMC_USER_PART},/* patch           32M    p59*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_CACHE,              800*1024,  104*1024,    EMMC_USER_PART},/* cache           104M   p60*/
  {PART_VENDOR_A,           904*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p61*/
  {PART_ODM_A,              1664*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p62*/
  {PART_CUST_A,             1840*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p63*/
#ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,           2032*1024,       3840*1024,    EMMC_USER_PART},/* system          3840M   p64*/
  {PART_PRODUCT_A,          5872*1024,       832*1024,    EMMC_USER_PART},/* product         832M   p65*/
  {PART_VERSION_A,          6704*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p66*/
  {PART_PRELOAD_A,          7280*1024,        928*1024,    EMMC_USER_PART},/* preload_a       928M    p67*/
  {PART_USERDATA,           8208*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p68*/
#elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM_A,           2032*1024,       4696*1024,    EMMC_USER_PART},/* system          4696M   p64*/
  {PART_PRODUCT_A,          6728*1024,        912*1024,    EMMC_USER_PART},/* product         912M    p65*/
  {PART_VERSION_A,          7640*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p66*/
  {PART_PRELOAD_A,          8216*1024,        900*1024,    EMMC_USER_PART},/* preload_a       900M    p67*/
  {PART_USERDATA,           9116*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata         4G     p68*/
#else
  {PART_SYSTEM_A,           2032*1024,       4988*1024,    EMMC_USER_PART},/* system          4988M   p64*/
  {PART_PRODUCT_A,          7020*1024,       1072*1024,    EMMC_USER_PART},/* product         1072M   p65*/
  {PART_VERSION_A,          8092*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p66*/
  {PART_PRELOAD_A,          8668*1024,        900*1024,    EMMC_USER_PART},/* preload_a       900M    p67*/
#ifdef CONFIG_FACTORY_MODE
  {PART_RESERVED5,          9568*1024,        128*1024,    EMMC_USER_PART},/* reserved5      128M    p68*/
  {PART_HIBENCH_DATA,       9696*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M    p69*/
  {PART_FLASH_AGEING,       10208*1024,       512*1024,    EMMC_USER_PART},/* flash_ageing   512M    p70*/
  {PART_USERDATA,           10720*1024,(4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p71*/
#else
  {PART_USERDATA,           9568*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G    p68*/
#endif
#endif
#else /* for rom update*/

  #ifdef CONFIG_MARKET_INTERNAL
  {PART_VERSION_A,          800*1024,  32*1024,     EMMC_USER_PART},/* version         32M    p58*/
  {PART_VENDOR_A,           832*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p59*/
  {PART_PRODUCT_A,          1592*1024, 192*1024,    EMMC_USER_PART},/* product         192M   p60*/
  {PART_CUST_A,             1784*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p61*/
  {PART_ODM_A,              1976*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p62*/
  {PART_CACHE,              2152*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p63*/
  {PART_SYSTEM_A,           2256*1024, 3088*1024,      EMMC_USER_PART},/* system        3088M  p64*/
  {PART_USERDATA,           5344*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p65*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_VERSION_A,          800*1024,  32*1024,     EMMC_USER_PART},/* version         32M    p58*/
  {PART_VENDOR_A,           832*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p59*/
  {PART_PRODUCT_A,          1592*1024, 192*1024,    EMMC_USER_PART},/* product         192M   p60*/
  {PART_CUST_A,             1784*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p61*/
  {PART_ODM_A,              1976*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p62*/
  {PART_CACHE,              2152*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p63*/
  {PART_SYSTEM_A,           2256*1024, 3536*1024,      EMMC_USER_PART},/* system         3536M  p64*/
  {PART_USERDATA,           5792*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p65*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_VERSION_A,          800*1024,  32*1024,     EMMC_USER_PART},/* version         32M    p58*/
  {PART_VENDOR_A,           832*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p59*/
  {PART_PRODUCT_A,          1592*1024, 192*1024,    EMMC_USER_PART},/* product         192M   p60*/
  {PART_CUST_A,             1784*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p61*/
  {PART_ODM_A,              1976*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p62*/
  {PART_CACHE,              2152*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p63*/
  {PART_SYSTEM_A,           2256*1024, 5632*1024,      EMMC_USER_PART},/* system          5632M  p64*/
  {PART_USERDATA,           7888*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p65*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_VERSION_A,          800*1024,  32*1024,     EMMC_USER_PART},/* version         32M    p58*/
  {PART_VENDOR_A,           832*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p59*/
  {PART_PRODUCT_A,          1592*1024, 192*1024,    EMMC_USER_PART},/* product         192M   p60*/
  {PART_CUST_A,             1784*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p61*/
  {PART_ODM_A,              1976*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p62*/
  {PART_CACHE,              2152*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p63*/
  {PART_SYSTEM_A,           2256*1024, 4752*1024,      EMMC_USER_PART},/* system          4752M  p64*/
  {PART_USERDATA,           7008*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p65*/
  #else  /*FOR NEW PRODUCT SHAOPIAN , HISI, SOMETHING ELSE*/
  {PART_VENDOR_A,           800*1024,  760*1024,    EMMC_USER_PART},/* vendor          760M   p58*/
  {PART_ODM_A,              1560*1024, 176*1024,    EMMC_USER_PART},/* odm             176M   p59*/
  {PART_CACHE,              1736*1024, 104*1024,    EMMC_USER_PART},/* cache           104M   p60*/
  {PART_CUST_A,             1840*1024, 192*1024,    EMMC_USER_PART},/* cust            192M   p61*/
  {PART_SYSTEM_A,           2032*1024,       4988*1024,    EMMC_USER_PART},/* system          4988M   p62*/
  {PART_PRODUCT_A,          7020*1024,       1072*1024,    EMMC_USER_PART},/* product         1072M   p63*/
  {PART_VERSION_A,          8092*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p64*/
  {PART_PRELOAD_A,          8668*1024,        900*1024,    EMMC_USER_PART},/* preload_a       900M    p65*/
#ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,          9568*1024,        128*1024,    EMMC_USER_PART},/*hibench_img      128M    p66*/
  {PART_HIBENCH_DATA,       9696*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M    p67*/
  {PART_FLASH_AGEING,       10208*1024,       512*1024,    EMMC_USER_PART},/* flash_ageing   512M    p68*/
  {PART_USERDATA,           10720*1024,(4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p69*/
#else
  {PART_USERDATA,           9568*1024, (4UL)*1024*1024,EMMC_USER_PART},/* userdata       4G    p66*/
#endif
  #endif  /*CONFIG_MARKET_INTERNAL*/
#endif  /*CONFIG_NEW_PRODUCT_P*/

  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,        0,         2*1024,        UFS_PART_0},
  {PART_XLOADER_B,        0,         2*1024,        UFS_PART_1},
  {PART_PTABLE,           0,         512,           UFS_PART_2},/* ptable          512K     */
  {PART_FRP,              512,       512,           UFS_PART_2},/* frp             512K   p1*/
  {PART_PERSIST,          1024,      2048,          UFS_PART_2},/* persist         2048K  p2*/
  {PART_RESERVED1,        3072,      5120,          UFS_PART_2},/* reserved1       5120K  p3*/
  {PART_PTABLE_LU3,       0,         512,           UFS_PART_3},/* ptable_lu3      512K   p0*/
  {PART_VRL,              512,       512,           UFS_PART_3},/* vrl             512K   p1*/
  {PART_VRL_BACKUP,       1024,      512,           UFS_PART_3},/* vrl backup      512K   p2*/
  {PART_MODEM_SECURE,     1536,      8704,          UFS_PART_3},/* modem_secure    8704k  p3*/
  {PART_NVME,             10*1024,   5*1024,        UFS_PART_3},/* nvme            5M     p4*/
  {PART_CTF,              15*1024,   1*1024,        UFS_PART_3},/* PART_CTF        1M     p5*/
  {PART_OEMINFO,          16*1024,   64*1024,       UFS_PART_3},/* oeminfo         64M    p6*/
  {PART_SECURE_STORAGE,   80*1024,   32*1024,       UFS_PART_3},/* secure storage  32M    p7*/
  {PART_MODEM_OM,         112*1024,  32*1024,       UFS_PART_3},/* modem om        32M    p8*/
  {PART_MODEMNVM_FACTORY, 144*1024,  16*1024,       UFS_PART_3},/* modemnvm factory16M    p9*/
  {PART_MODEMNVM_BACKUP,  160*1024,  16*1024,       UFS_PART_3},/* modemnvm backup 16M    p10*/
  {PART_MODEMNVM_IMG,     176*1024,  34*1024,       UFS_PART_3},/* modemnvm img    34M    p11*/
  {PART_RESERVED7,        210*1024,  2*1024,        UFS_PART_3},/* reserved7       2M     p12*/
  {PART_HISEE_ENCOS,      212*1024,  4*1024,        UFS_PART_3},/* hisee_encos     4M     p13*/
  {PART_VERITYKEY,        216*1024,  1*1024,        UFS_PART_3},/* reserved2       32M    p14*/
  {PART_DDR_PARA,         217*1024,  1*1024,        UFS_PART_3},/* DDR_PARA        1M     p15*/
  {PART_MODEM_DRIVER_A,   218*1024,  20*1024,       UFS_PART_3},/* modem_driver_a  20M    p16*/
  {PART_RESERVED2,        238*1024,   7*1024,       UFS_PART_3},/* reserved2        7M    p17*/
  {PART_SPLASH2,          245*1024,  80*1024,       UFS_PART_3},/* splash2         80M    p18*/
  {PART_BOOTFAIL_INFO,    325*1024,  2*1024,        UFS_PART_3},/* bootfail info   2MB    p19*/
  {PART_MISC,             327*1024,  2*1024,        UFS_PART_3},/* misc            2M     p20*/
  {PART_DFX,              329*1024,  16*1024,       UFS_PART_3},/* dfx             16M    p21*/
  {PART_RRECORD,          345*1024,  16*1024,       UFS_PART_3},/* rrecord         16M    p22*/
  {PART_FW_LPM3_A,        361*1024,  256,           UFS_PART_3},/* mcuimage        256K   p23*/
  {PART_RESERVED3_A,      369920,    3840,          UFS_PART_3},/* reserved3A      3840K  p24*/
  {PART_HDCP_A,           365*1024,  1*1024,        UFS_PART_3},/* PART_HDCP_A     1M     p25*/
  {PART_HISEE_IMG_A,      366*1024,  4*1024,        UFS_PART_3},/* part_hisee_img_a 4M    p26*/
  {PART_HHEE_A,           370*1024,  4*1024,        UFS_PART_3},/* PART_RESERVED10  4M    p27*/
  {PART_HISEE_FS_A,       374*1024,  8*1024,        UFS_PART_3},/* hisee_fs        8M     p28*/
  {PART_FASTBOOT_A,       382*1024,  12*1024,       UFS_PART_3},/* fastboot        12M    p29*/
  {PART_VECTOR_A,         394*1024,  4*1024,        UFS_PART_3},/* avs vector      4M     p30*/
  {PART_ISP_BOOT_A,       398*1024,  2*1024,        UFS_PART_3},/* isp_boot        2M     p31*/
  {PART_ISP_FIRMWARE_A,   400*1024,  14*1024,       UFS_PART_3},/* isp_firmware    14M    p32*/
  {PART_FW_HIFI_A,        414*1024,  12*1024,       UFS_PART_3},/* hifi            12M    p33*/
  {PART_TEEOS_A,          426*1024,  8*1024,        UFS_PART_3},/* teeos           8M     p34*/
  {PART_SENSORHUB_A,      434*1024,  16*1024,       UFS_PART_3},/* sensorhub       16M    p35*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL_A,  450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p36*/
  {PART_ERECOVERY_RAMDISK_A, 474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p37*/
  {PART_ERECOVERY_VENDOR_A,  506*1024,  8*1024,     UFS_PART_3},/* erecovery_vendor  8M    p38*/
  {PART_KERNEL_A,            514*1024,  32*1024,    UFS_PART_3},/* kernel            32M   p39*/
#else
  {PART_ERECOVERY_KERNEL_A,  450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p36*/
  {PART_ERECOVERY_RAMDISK_A, 474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p37*/
  {PART_ERECOVERY_VENDOR_A,  506*1024,  16*1024,    UFS_PART_3},/* erecovery_vendor  16M   p38*/
  {PART_KERNEL_A,            522*1024,  24*1024,    UFS_PART_3},/* kernel            24M   p39*/
#endif
  {PART_ENG_SYSTEM_A,       546*1024,  12*1024,     UFS_PART_3},/* eng_system     12M      p40*/
  {PART_RECOVERY_RAMDISK_A, 558*1024,  32*1024,     UFS_PART_3},/* recovery_ramdisk 32M    p41*/
  {PART_RECOVERY_VENDOR_A,  590*1024,  16*1024,     UFS_PART_3},/* recovery_vendor  16M    p42*/
  {PART_DTS_A,              606*1024,   8*1024,     UFS_PART_3},/* dtimage           8M    p43*/
  {PART_DTO_A,              614*1024,  24*1024,     UFS_PART_3},/* dtoimage         24M    p44*/
  {PART_TRUSTFIRMWARE_A,    638*1024,  2*1024,      UFS_PART_3},/* trustfirmware    2M     p45*/
  {PART_MODEM_FW_A,         640*1024,  56*1024,     UFS_PART_3},/* modem_fw         56M    p46*/
  {PART_ENG_VENDOR_A,       696*1024,  12*1024,     UFS_PART_3},/* eng_vendor       12M    p47*/
  {PART_MODEM_PATCH_NV_A,   708*1024,   4*1024,     UFS_PART_3},/* modem_patch_nv_a  4M    p48*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_RESERVED4_A,        712*1024,  8*1024,     UFS_PART_3},/* reserved4         8M     p49*/
#elif (defined(CONFIG_MARKET_INTERNAL) || defined(CONFIG_MARKET_OVERSEA) ||\
defined(CONFIG_MARKET_FULL_INTERNAL) || defined(CONFIG_MARKET_FULL_OVERSEA))
/* rom update*/
  {PART_PRELOAD_A,          712*1024,  8*1024,     UFS_PART_3},/* PART_PRELOAD_A    8M     p49*/
#else
  {PART_RESERVED4_A,        712*1024,  8*1024,     UFS_PART_3},/* reserved4         8M     p49*/
#endif
  {PART_RECOVERY_VBMETA_A,  720*1024,  2*1024,      UFS_PART_3},/* recovery_vbmeta_a  2M   p50*/
  {PART_ERECOVERY_VBMETA_A, 722*1024,  2*1024,      UFS_PART_3},/* erecovery_vbmeta_a 2M   p51*/
  {PART_VBMETA_A,           724*1024,  4*1024,      UFS_PART_3},/* vbmeta_a         4M     p52*/
  {PART_MODEMNVM_UPDATE_A,  728*1024,  16*1024,     UFS_PART_3},/* modemnvm_update_a  16M  p53*/
  {PART_MODEMNVM_CUST_A,    744*1024,  16*1024,     UFS_PART_3},/* modemnvm_cust_a    16M  p54*/
  {PART_PATCH_A,            760*1024,  32*1024,     UFS_PART_3},/* patch            32M    p55*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_VERSION_A,          792*1024,  32*1024,     UFS_PART_3},/* version          32M    p56*/
  {PART_VENDOR_A,           824*1024,  760*1024,    UFS_PART_3},/* vendor           760M   p57*/
  {PART_PRODUCT_A,          1584*1024, 192*1024,    UFS_PART_3},/* product          192M   p58*/
  {PART_CUST_A,             1776*1024, 192*1024,    UFS_PART_3},/* cust             192M   p59*/
  {PART_ODM_A,              1968*1024, 176*1024,    UFS_PART_3},/* odm              176M   p60*/
  {PART_CACHE,              2144*1024, 104*1024,    UFS_PART_3},/* cache            104M   p61*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM_A,           2248*1024, 3852*1024,      UFS_PART_3},/* system       3852M   p62*/
  {PART_USERDATA,           6100*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p63*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM_A,           2248*1024, 5632*1024,      UFS_PART_3},/* system       5632M   p62*/
  {PART_USERDATA,           7880*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p63*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM_A,           2248*1024, 4752*1024,      UFS_PART_3},/* system       4752M   p62*/
  {PART_USERDATA,           7000*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p63*/
  #else
  {PART_SYSTEM_A,           2248*1024, 4988*1024,      UFS_PART_3},/* system       4988M   p62*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,          7236*1024, 128*1024,       UFS_PART_3},/* hibench_img     128M   p63*/
  {PART_HIBENCH_DATA,       7364*1024, 512*1024,       UFS_PART_3},/* hibench_data  512M   p64*/
  {PART_FLASH_AGEING,       7876*1024, 512*1024,       UFS_PART_3},/* flash_ageing  512M   p65*/
  {PART_USERDATA,           8388*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p66*/
  #else
  {PART_USERDATA,           7236*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata        4G   p63*/
  #endif
  #endif
#else /* for rom update*/
#ifdef CONFIG_MARKET_INTERNAL
  {PART_VERSION_A,           792*1024,  32*1024,    UFS_PART_3},/* version         32M    p62*/
  {PART_VENDOR_A,            824*1024, 760*1024,    UFS_PART_3},/* vendor          760M   p63*/
  {PART_PRODUCT_A,          1584*1024, 192*1024,    UFS_PART_3},/* product         192M   p64*/
  {PART_CUST_A,             1776*1024, 192*1024,    UFS_PART_3},/* cust            192M   p65*/
  {PART_ODM_A,              1968*1024, 176*1024,    UFS_PART_3},/* odm             176M   p66*/
  {PART_CACHE,              2144*1024, 104*1024,    UFS_PART_3},/* cache           104M   p67*/
  {PART_SYSTEM_A,           2248*1024, 3088*1024,   UFS_PART_3},/* system         3088M   p68*/
  {PART_USERDATA,           5336*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G    p69*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_VERSION_A,          792*1024,  32*1024,     UFS_PART_3},/* version         32M    p62*/
  {PART_VENDOR_A,           824*1024,  760*1024,    UFS_PART_3},/* vendor          760M   p63*/
  {PART_PRODUCT_A,          1584*1024, 192*1024,    UFS_PART_3},/* product         192M   p64*/
  {PART_CUST_A,             1776*1024, 192*1024,    UFS_PART_3},/* cust            192M   p65*/
  {PART_ODM_A,              1968*1024, 176*1024,    UFS_PART_3},/* odm             176M   p66*/
  {PART_CACHE,              2144*1024, 104*1024,    UFS_PART_3},/* cache           104M   p67*/
  {PART_SYSTEM_A,           2248*1024, 3536*1024,      UFS_PART_3},/* system      3536M   p68*/
  {PART_USERDATA,           5784*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G   p69*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_VERSION_A,          792*1024,  32*1024,     UFS_PART_3},/* version         32M    p62*/
  {PART_VENDOR_A,           824*1024,  760*1024,    UFS_PART_3},/* vendor          760M   p63*/
  {PART_PRODUCT_A,          1584*1024, 192*1024,    UFS_PART_3},/* product         192M   p64*/
  {PART_CUST_A,             1776*1024, 192*1024,    UFS_PART_3},/* cust            192M   p65*/
  {PART_ODM_A,              1968*1024, 176*1024,    UFS_PART_3},/* odm             176M   p66*/
  {PART_CACHE,              2144*1024, 104*1024,    UFS_PART_3},/* cache           104M   p67*/
  {PART_SYSTEM_A,           2248*1024, 5632*1024,      UFS_PART_3},/* system      5632M   p68*/
  {PART_USERDATA,           7880*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G   p69*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_VERSION_A,          792*1024,  32*1024,     UFS_PART_3},/* version         32M    p62*/
  {PART_VENDOR_A,           824*1024,  760*1024,    UFS_PART_3},/* vendor          760M   p63*/
  {PART_PRODUCT_A,          1584*1024, 192*1024,    UFS_PART_3},/* product         192M   p64*/
  {PART_CUST_A,             1776*1024, 192*1024,    UFS_PART_3},/* cust            192M   p65*/
  {PART_ODM_A,              1968*1024, 176*1024,    UFS_PART_3},/* odm             176M   p66*/
  {PART_CACHE,              2144*1024, 104*1024,    UFS_PART_3},/* cache           104M   p67*/
  {PART_SYSTEM_A,           2248*1024, 4752*1024,      UFS_PART_3},/* system      4752M   p68*/
  {PART_USERDATA,           7000*1024, (4UL)*1024*1024,UFS_PART_3},/* userdata       4G   p69*/
  #else  /*FOR NEW PRODUCT SHAOPIAN , HISI, SOMETHING ELSE*/
  {PART_VENDOR_A,           792*1024,  760*1024,    UFS_PART_3},/* vendor          760M   p62*/
  {PART_ODM_A,              1552*1024, 176*1024,    UFS_PART_3},/* odm             176M   p63*/
  {PART_CACHE,              1728*1024, 104*1024,    UFS_PART_3},/* cache           104M   p64*/
  {PART_CUST_A,             1832*1024, 192*1024,    UFS_PART_3},/* cust            192M   p65*/
  {PART_SYSTEM_A,           2024*1024, 4988*1024,   UFS_PART_3},/* system          4988M  p66*/
  {PART_PRODUCT_A,          7012*1024, 1072*1024,   UFS_PART_3},/* product         1072M  p67*/
  {PART_VERSION_A,          8084*1024, 576*1024,    UFS_PART_3},/* version         576M   p68*/
  {PART_PRELOAD_A,          8660*1024, 900*1024,    UFS_PART_3},/* preload_a       900M   p69*/
#ifdef CONFIG_FACTORY_MODE
  {PART_RESERVED5,          9560*1024,        128*1024,    UFS_PART_3},/* reserved5      128M    p70*/
  {PART_HIBENCH_DATA,       9688*1024,        512*1024,    UFS_PART_3},/* hibench_data   512M    p71*/
  {PART_FLASH_AGEING,       10200*1024,       512*1024,    UFS_PART_3},/* flash_ageing   512M    p72*/
  {PART_USERDATA,           10712*1024,(4UL)*1024*1024,    UFS_PART_3},/* userdata       4G      p73*/
#else
  {PART_USERDATA,           9560*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata       4G    p70*/
#endif
#endif	/*CONFIG_MARKET_INTERNAL*/
#endif  /*CONFIG_NEW_PRODUCT_P*/
  {"0", 0, 0, 0},
};
#endif

#endif
