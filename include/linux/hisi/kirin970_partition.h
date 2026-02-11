#ifndef _KIRIN970_PARTITION_H_
#define _KIRIN970_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,                  0,       2*1024,    EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,                0,       2*1024,    EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                   0,          512,    EMMC_USER_PART},/* ptable            512K */
  {PART_FRP,                    512,          512,    EMMC_USER_PART},/* frp               512K   p1*/
  {PART_PERSIST,               1024,         2048,    EMMC_USER_PART},/* persist           2048K  p2*/
  {PART_RESERVED1,             3072,         5120,    EMMC_USER_PART},/* reserved1         5120K  p3*/
  {PART_RESERVED6,           8*1024,          512,    EMMC_USER_PART},/* reserved6         512K   p4*/
  {PART_VRL,                   8704,          512,    EMMC_USER_PART},/* vrl               512K   p5*/
  {PART_VRL_BACKUP,            9216,          512,    EMMC_USER_PART},/* vrl backup        512K   p6*/
  {PART_MODEM_SECURE,          9728,         8704,    EMMC_USER_PART},/* modem_secure      8704k  p7*/
  {PART_NVME,               18*1024,       5*1024,    EMMC_USER_PART},/* nvme              5M     p8*/
  {PART_CTF,                23*1024,       1*1024,    EMMC_USER_PART},/* PART_CTF          1M     p9*/
  {PART_OEMINFO,            24*1024,      64*1024,    EMMC_USER_PART},/* oeminfo           64M    p10*/
  {PART_SECURE_STORAGE,     88*1024,      32*1024,    EMMC_USER_PART},/* secure storage    32M    p11*/
  {PART_MODEM_OM,          120*1024,      32*1024,    EMMC_USER_PART},/* modem om          32M    p12*/
  {PART_MODEMNVM_FACTORY,  152*1024,      16*1024,    EMMC_USER_PART},/* modemnvmfactory   16M    p13*/
  {PART_MODEMNVM_BACKUP,   168*1024,      16*1024,    EMMC_USER_PART},/* modemnvmbackup    16M    p14*/
  {PART_MODEMNVM_IMG,      184*1024,      20*1024,    EMMC_USER_PART},/* modemnvmimg       20M    p15*/
  {PART_MODEMNVM_SYSTEM,   204*1024,      16*1024,    EMMC_USER_PART},/* modemnvmsystem    16M    p16*/
  {PART_HISEE_ENCOS,       220*1024,       4*1024,    EMMC_USER_PART},/* hisee_encos       4M     p17*/
  {PART_VERITYKEY,         224*1024,       1*1024,    EMMC_USER_PART},/* reserved2         32M    p18*/
  {PART_DDR_PARA,          225*1024,       1*1024,    EMMC_USER_PART},/* DDR_PARA          1M     p19*/
  {PART_RESERVED2,         226*1024,      27*1024,    EMMC_USER_PART},/* reserved2         27M    p20*/
  {PART_SPLASH2,           253*1024,      80*1024,    EMMC_USER_PART},/* splash2           80M    p21*/
  {PART_BOOTFAIL_INFO,     333*1024,       2*1024,    EMMC_USER_PART},/* bootfail info     2MB    p22*/
  {PART_MISC,              335*1024,       2*1024,    EMMC_USER_PART},/* misc              2M     p23*/
  {PART_DFX,               337*1024,      16*1024,    EMMC_USER_PART},/* dfx               16M    p24*/
  {PART_RRECORD,           353*1024,      16*1024,    EMMC_USER_PART},/* rrecord           16M    p25*/
  {PART_FW_LPM3,           369*1024,      256,        EMMC_USER_PART},/* mcuimage          256K   p26*/
  {PART_RESERVED3,         378112,        3840,       EMMC_USER_PART},/* reserved3A        3840KB p27*/
  {PART_HDCP,              373*1024,       1*1024,    EMMC_USER_PART},/* HDCP              1M     p28*/
  {PART_HISEE_IMG,         374*1024,       4*1024,    EMMC_USER_PART},/* hisee_img         4M     p29*/
  {PART_HHEE,              378*1024,       4*1024,    EMMC_USER_PART},/* RESERVED10        4M     p30*/
  {PART_HISEE_FS,          382*1024,       8*1024,    EMMC_USER_PART},/* hisee_fs          8M     p31*/
  {PART_FASTBOOT,          390*1024,      12*1024,    EMMC_USER_PART},/* fastboot          12M    p32*/
  {PART_VECTOR,            402*1024,       4*1024,    EMMC_USER_PART},/* avs vector        4M     p33*/
  {PART_ISP_BOOT,          406*1024,       2*1024,    EMMC_USER_PART},/* isp_boot          2M     p34*/
  {PART_ISP_FIRMWARE,      408*1024,      14*1024,    EMMC_USER_PART},/* isp_firmware      14M    p35*/
  {PART_FW_HIFI,           422*1024,      12*1024,    EMMC_USER_PART},/* hifi              12M    p36*/
  {PART_TEEOS,             434*1024,       8*1024,    EMMC_USER_PART},/* teeos             8M     p37*/
  {PART_SENSORHUB,         442*1024,      16*1024,    EMMC_USER_PART},/* sensorhub         16M    p38*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,    458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M  p39*/
  {PART_ERECOVERY_RAMDISK,   482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M  p40*/
  {PART_ERECOVERY_VENDOR,    514*1024,  8*1024,     EMMC_USER_PART},/* erecovery_vendor  8M   p41*/
  {PART_KERNEL,              522*1024,  32*1024,    EMMC_USER_PART},/* kernel            32M  p42*/
#else
  {PART_ERECOVERY_KERNEL,    458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M  p39*/
  {PART_ERECOVERY_RAMDISK,   482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M  p40*/
  {PART_ERECOVERY_VENDOR,    514*1024,  16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M  p41*/
  {PART_KERNEL,              530*1024,  24*1024,    EMMC_USER_PART},/* kernel            24M  p42*/
#endif
  {PART_ENG_SYSTEM,          554*1024,  12*1024,    EMMC_USER_PART},/* eng_system        12M  p43*/
  {PART_RECOVERY_RAMDISK,    566*1024,  32*1024,    EMMC_USER_PART},/* recovery_ramdisk  32M  p44*/
  {PART_RECOVERY_VENDOR,     598*1024,  16*1024,    EMMC_USER_PART},/* recovery_vendor   16M  p45*/
  {PART_DTS,                 614*1024,  21*1024,    EMMC_USER_PART},/* dtimage           21M  p46*/
  {PART_DTO,                 635*1024,  7*1024,     EMMC_USER_PART},/* dtoimage          7M   p47*/
  {PART_TRUSTFIRMWARE,       642*1024,  2*1024,     EMMC_USER_PART},/* trustfirmware     2M   p48*/
  {PART_MODEM_FW,            644*1024,  56*1024,    EMMC_USER_PART},/* modem_fw          56M  p49*/
  {PART_ENG_VENDOR,          700*1024,  12*1024,    EMMC_USER_PART},/* eng_vendor        12M  p50*/
  {PART_RECOVERY_VBMETA,     712*1024,  2*1024,     EMMC_USER_PART},/* recovery_vbmeta    2M  p51*/
  {PART_ERECOVERY_VBMETA,    714*1024,  2*1024,     EMMC_USER_PART},/* erecovery_vbmeta   2M  p52*/
  {PART_VBMETA,              716*1024,  4*1024,     EMMC_USER_PART},/* PART_VBMETA        4M  p53*/
  {PART_MODEMNVM_UPDATE,     720*1024,  16*1024,    EMMC_USER_PART},/* modemnvm update   16M  p54*/
  {PART_MODEMNVM_CUST,       736*1024,  40*1024,    EMMC_USER_PART},/* modemnvm cust     40M  p55*/
  {PART_RESERVED7,           776*1024,  32*1024,    EMMC_USER_PART},/* reserved7         32M  p56*/
#ifdef CONFIG_FACTORY_MODE
  {PART_RAMDISK,             808*1024,         2*1024,         EMMC_USER_PART},/* ramdisk           2M    p57*/
  {PART_VBMETA_SYSTEM,       810*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_system     1M    p58*/
  {PART_VBMETA_VENDOR,       811*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_vendor     1M    p59*/
  {PART_VBMETA_ODM,          812*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_odm        1M    p60*/
  {PART_VBMETA_CUST,         813*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_cust       1M    p61*/
  {PART_VBMETA_HW_PRODUCT,   814*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_hw_product 1M    p62*/
  {PART_RESERVED4,           815*1024,         1*1024,         EMMC_USER_PART},/* reserved4         1M    p63*/
  {PART_PREAS,               816*1024,       360*1024,         EMMC_USER_PART},/* preas           360M    p64*/
  {PART_PREAVS,             1176*1024,        32*1024,         EMMC_USER_PART},/* preavs           32M    p65*/
  {PART_VERSION,            1208*1024,        32*1024,         EMMC_USER_PART},/* version          32M    p66*/
  {PART_PRELOAD,            1240*1024,         8*1024,         EMMC_USER_PART},/* preload           8M    p67*/
  {PART_RESERVED5,          1248*1024,       888*1024,         EMMC_USER_PART},/* reserved5       888M    p68*/
  {PART_CACHE,              2136*1024,        128*1024,        EMMC_USER_PART},/* cache           128M    p69*/
  {PART_PATCH,              2264*1024,         96*1024,        EMMC_USER_PART},/* patch            96M    p70*/
  {PART_SUPER,              2360*1024,       5536*1024,        EMMC_USER_PART},/* super          5536M    p71*/
  {PART_RESERVED6,          7896*1024,        128*1024,        EMMC_USER_PART},/* reserved6       128M    p72*/
  {PART_HIBENCH_DATA,       8024*1024,        512*1024,        EMMC_USER_PART},/* hibench_data    512M    p73*/
  {PART_USERDATA,           8536*1024, (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G    p74*/
#else
 #ifdef CONFIG_NEW_PRODUCT_P
  {PART_CACHE,               808*1024,        128*1024,        EMMC_USER_PART},/* cache           128M    p57*/
  {PART_RAMDISK,             936*1024,          2*1024,        EMMC_USER_PART},/* ramdisk           2M    p58*/
  {PART_VBMETA_SYSTEM,       938*1024,          1*1024,        EMMC_USER_PART},/* vbmeta_system     1M    p59*/
  {PART_VBMETA_VENDOR,       939*1024,          1*1024,        EMMC_USER_PART},/* vbmeta_vendor     1M    p60*/
  {PART_VBMETA_ODM,          940*1024,          1*1024,        EMMC_USER_PART},/* vbmeta_odm        1M    p61*/
  {PART_VBMETA_CUST,         941*1024,          1*1024,        EMMC_USER_PART},/* vbmeta_cust       1M    p62*/
  {PART_VBMETA_HW_PRODUCT,   942*1024,          1*1024,        EMMC_USER_PART},/* vbmeta_hw_product 1M    p63*/
  {PART_RESERVED4,           943*1024,          1*1024,        EMMC_USER_PART},/* reserved4         1M    p64*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,               944*1024,        360*1024,        EMMC_USER_PART},/* preas           360M    p65*/
  {PART_PREAVS,             1304*1024,         32*1024,        EMMC_USER_PART},/* preavs           32M    p66*/
  {PART_PATCH,              1336*1024,         96*1024,        EMMC_USER_PART},/* patch            96M    p67*/
  {PART_SUPER,              1432*1024,       7376*1024,        EMMC_USER_PART},/* super          7376M    p68*/
  {PART_VERSION,            8808*1024,        576*1024,        EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            9384*1024,       1144*1024,        EMMC_USER_PART},/* preload        1144M    p70*/
  {PART_USERDATA,          10528*1024, (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G    p71*/
  #else
  {PART_PREAS,               944*1024,       1216*1024,        EMMC_USER_PART},/* preas          1216M    p65*/
  {PART_PREAVS,             2160*1024,         32*1024,        EMMC_USER_PART},/* preavs           32M    p66*/
  {PART_PATCH,              2192*1024,         96*1024,        EMMC_USER_PART},/* patch            96M    p67*/
  {PART_SUPER,              2288*1024,       7440*1024,        EMMC_USER_PART},/* super          7440M    p68*/
  {PART_VERSION,            9728*1024,        576*1024,        EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,           10304*1024,       1144*1024,        EMMC_USER_PART},/* preload        1144M    p70*/
  {PART_USERDATA,          11448*1024, (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G    p71*/
  #endif
 #else
  {PART_RAMDISK,             808*1024,         2*1024,         EMMC_USER_PART},/* ramdisk           2M    p57*/
  {PART_VBMETA_SYSTEM,       810*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_system     1M    p58*/
  {PART_VBMETA_VENDOR,       811*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_vendor     1M    p59*/
  {PART_VBMETA_ODM,          812*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_odm        1M    p60*/
  {PART_VBMETA_CUST,         813*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_cust       1M    p61*/
  {PART_VBMETA_HW_PRODUCT,   814*1024,         1*1024,         EMMC_USER_PART},/* vbmeta_hw_product 1M    p62*/
  {PART_RESERVED4,           815*1024,         1*1024,         EMMC_USER_PART},/* reserved4         1M    p63*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,               816*1024,        360*1024,        EMMC_USER_PART},/* preas           360M    p64*/
  {PART_PREAVS,             1176*1024,         32*1024,        EMMC_USER_PART},/* preavs           32M    p65*/
  {PART_VERSION,            1208*1024,         32*1024,        EMMC_USER_PART},/* version          32M    p66*/
  {PART_PRELOAD,            1240*1024,          8*1024,        EMMC_USER_PART},/* preload           8M    p67*/
  {PART_RESERVED5,          1248*1024,        888*1024,        EMMC_USER_PART},/* reserved5       888M    p68*/
  {PART_CACHE,              2136*1024,        128*1024,        EMMC_USER_PART},/* cache           128M    p69*/
  {PART_PATCH,              2264*1024,         96*1024,        EMMC_USER_PART},/* patch            96M    p70*/
  {PART_SUPER,              2360*1024,       4656*1024,        EMMC_USER_PART},/* super          4656M    p71*/
  {PART_USERDATA,           7016*1024, (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G    p72*/
  #else
  {PART_PREAS,               816*1024,       1216*1024,        EMMC_USER_PART},/* preas          1216M    p64*/
  {PART_PREAVS,             2032*1024,         32*1024,        EMMC_USER_PART},/* preavs           32M    p65*/
  {PART_VERSION,            2064*1024,         32*1024,        EMMC_USER_PART},/* version          32M    p66*/
  {PART_PRELOAD,            2096*1024,          8*1024,        EMMC_USER_PART},/* preload           8M    p67*/
  {PART_RESERVED5,          2104*1024,         32*1024,        EMMC_USER_PART},/* reserved5        32M    p68*/
  {PART_CACHE,              2136*1024,        128*1024,        EMMC_USER_PART},/* cache           128M    p69*/
  {PART_PATCH,              2264*1024,         96*1024,        EMMC_USER_PART},/* patch            96M    p70*/
  {PART_SUPER,              2360*1024,       5536*1024,        EMMC_USER_PART},/* super          5536M    p71*/
  {PART_USERDATA,           7896*1024, (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G    p72*/
  #endif
 #endif
#endif
  {"0", 0, 0, 0},                                        /* total 11896M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER,            0,         2*1024,        UFS_PART_0},
  {PART_RESERVED0,          0,         2*1024,        UFS_PART_1},
  {PART_PTABLE,             0,         512,           UFS_PART_2},/* ptable           512K */
  {PART_FRP,                512,       512,           UFS_PART_2},/* frp              512K   p1*/
  {PART_PERSIST,            1024,      2048,          UFS_PART_2},/* persist          2048K  p2*/
  {PART_RESERVED1,          3072,      5120,          UFS_PART_2},/* reserved1        5120K  p3*/
  {PART_PTABLE_LU3,         0,         512,           UFS_PART_3},/* ptable_lu3       512K   p0*/
  {PART_VRL,                512,       512,           UFS_PART_3},/* vrl              512K   p1*/
  {PART_VRL_BACKUP,         1024,      512,           UFS_PART_3},/* vrl backup       512K   p2*/
  {PART_MODEM_SECURE,       1536,      8704,          UFS_PART_3},/* modem_secure     8704k  p3*/
  {PART_NVME,               10*1024,   5*1024,        UFS_PART_3},/* nvme             5M     p4*/
  {PART_CTF,                15*1024,   1*1024,        UFS_PART_3},/* PART_CTF         1M     p5*/
  {PART_OEMINFO,            16*1024,   64*1024,       UFS_PART_3},/* oeminfo          64M    p6*/
  {PART_SECURE_STORAGE,     80*1024,   32*1024,       UFS_PART_3},/* secure storage   32M    p7*/
  {PART_MODEM_OM,           112*1024,  32*1024,       UFS_PART_3},/* modem om         32M    p8*/
  {PART_MODEMNVM_FACTORY,   144*1024,  16*1024,       UFS_PART_3},/* modemnvmfactory  16M    p9*/
  {PART_MODEMNVM_BACKUP,    160*1024,  16*1024,       UFS_PART_3},/* modemnvmbackup   16M    p10*/
  {PART_MODEMNVM_IMG,       176*1024,  20*1024,       UFS_PART_3},/* modemnvmimg      20M    p11*/
  {PART_MODEMNVM_SYSTEM,    196*1024,  16*1024,       UFS_PART_3},/* modemnvmsystem   16M    p12*/
  {PART_HISEE_ENCOS,        212*1024,  4*1024,        UFS_PART_3},/* hisee_encos      4M     p13*/
  {PART_VERITYKEY,          216*1024,  1*1024,        UFS_PART_3},/* reserved2        32M    p14*/
  {PART_DDR_PARA,           217*1024,  1*1024,        UFS_PART_3},/* DDR_PARA         1M     p15*/
  {PART_RESERVED2,          218*1024,  27*1024,       UFS_PART_3},/* reserved2        27M    p16*/
  {PART_SPLASH2,            245*1024,  80*1024,       UFS_PART_3},/* splash2          80M    p17*/
  {PART_BOOTFAIL_INFO,      325*1024,  2*1024,        UFS_PART_3},/* bootfail info    2MB    p18*/
  {PART_MISC,               327*1024,  2*1024,        UFS_PART_3},/* misc             2M     p19*/
  {PART_DFX,                329*1024,  16*1024,       UFS_PART_3},/* dfx              16M    p20*/
  {PART_RRECORD,            345*1024,  16*1024,       UFS_PART_3},/* rrecord          16M    p21*/
  {PART_FW_LPM3,            361*1024,  256,           UFS_PART_3},/* mcuimage         256K   p22*/
  {PART_RESERVED3,          369920,    3840,          UFS_PART_3},/* reserved3A       3840K  p23*/
  {PART_HDCP,               365*1024,  1*1024,        UFS_PART_3},/* HDCP             1M     p24*/
  {PART_HISEE_IMG,          366*1024,  4*1024,        UFS_PART_3},/* hisee_img        4M     p25*/
  {PART_HHEE,               370*1024,  4*1024,        UFS_PART_3},/* RESERVED10       4M     p26*/
  {PART_HISEE_FS,           374*1024,  8*1024,        UFS_PART_3},/* hisee_fs         8M     p27*/
  {PART_FASTBOOT,           382*1024,  12*1024,       UFS_PART_3},/* fastboot         12M    p28*/
  {PART_VECTOR,             394*1024,  4*1024,        UFS_PART_3},/* avs vector       4M     p29*/
  {PART_ISP_BOOT,           398*1024,  2*1024,        UFS_PART_3},/* isp_boot         2M     p30*/
  {PART_ISP_FIRMWARE,       400*1024,  14*1024,       UFS_PART_3},/* isp_firmware     14M    p31*/
  {PART_FW_HIFI,            414*1024,  12*1024,       UFS_PART_3},/* hifi             12M    p32*/
  {PART_TEEOS,              426*1024,  8*1024,        UFS_PART_3},/* teeos            8M     p33*/
  {PART_SENSORHUB,          434*1024,  16*1024,       UFS_PART_3},/* sensorhub        16M    p34*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,    450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M  p35*/
  {PART_ERECOVERY_RAMDISK,   474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M  p36*/
  {PART_ERECOVERY_VENDOR,    506*1024,  8*1024,     UFS_PART_3},/* erecovery_vendor  8M   p37*/
  {PART_KERNEL,              514*1024,  32*1024,    UFS_PART_3},/* kernel            32M  p38*/
#else
  {PART_ERECOVERY_KERNEL,    450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M  p35*/
  {PART_ERECOVERY_RAMDISK,   474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M  p36*/
  {PART_ERECOVERY_VENDOR,    506*1024,  16*1024,    UFS_PART_3},/* erecovery_vendor  16M  p37*/
  {PART_KERNEL,              522*1024,  24*1024,    UFS_PART_3},/* kernel            24M  p38*/
#endif
  {PART_ENG_SYSTEM,          546*1024,  12*1024,    UFS_PART_3},/* eng_system        12M  p39*/
  {PART_RECOVERY_RAMDISK,    558*1024,  32*1024,    UFS_PART_3},/* recovery_ramdisk  32M  p40*/
  {PART_RECOVERY_VENDOR,     590*1024,  16*1024,    UFS_PART_3},/* recovery_vendor   16M  p41*/
  {PART_DTS,                 606*1024,  21*1024,    UFS_PART_3},/* dtimage           21M  p42*/
  {PART_DTO,                 627*1024,  7*1024,     UFS_PART_3},/* dtoimage          7M   p43*/
  {PART_TRUSTFIRMWARE,       634*1024,  2*1024,     UFS_PART_3},/* trustfirmware     2M   p44*/
  {PART_MODEM_FW,            636*1024,  56*1024,    UFS_PART_3},/* modem_fw          56M  p45*/
  {PART_ENG_VENDOR,          692*1024,  12*1024,    UFS_PART_3},/* eng_vendor        12M  p46*/
  {PART_RECOVERY_VBMETA,     704*1024,  2*1024,     UFS_PART_3},/* recovery_vbmeta    2M  p47*/
  {PART_ERECOVERY_VBMETA,    706*1024,  2*1024,     UFS_PART_3},/* erecovery_vbmeta   2M  p48*/
  {PART_VBMETA,              708*1024,  4*1024,     UFS_PART_3},/* VBMETA            4M   p49*/
  {PART_MODEMNVM_UPDATE,     712*1024,  16*1024,    UFS_PART_3},/* modemnvm update   16M  p50*/
  {PART_MODEMNVM_CUST,       728*1024,  40*1024,    UFS_PART_3},/* modemnvm cust     64M  p51*/
  {PART_RESERVED7,           768*1024,  32*1024,    UFS_PART_3},/* reserved7         32M  p52*/
#ifdef CONFIG_FACTORY_MODE
  {PART_RAMDISK,             800*1024,       2*1024,        UFS_PART_3},/* ramdisk           2M    p53*/
  {PART_VBMETA_SYSTEM,       802*1024,       1*1024,        UFS_PART_3},/* vbmeta_system     1M    p54*/
  {PART_VBMETA_VENDOR,       803*1024,       1*1024,        UFS_PART_3},/* vbmeta_vendor     1M    p55*/
  {PART_VBMETA_ODM,          804*1024,       1*1024,        UFS_PART_3},/* vbmeta_odm        1M    p56*/
  {PART_VBMETA_CUST,         805*1024,       1*1024,        UFS_PART_3},/* vbmeta_cust       1M    p57*/
  {PART_VBMETA_HW_PRODUCT,   806*1024,       1*1024,        UFS_PART_3},/* vbmeta_hw_product 1M    p58*/
  {PART_RESERVED4,           807*1024,       1*1024,        UFS_PART_3},/* reserved4         1M    p59*/
  {PART_PREAS,               808*1024,     360*1024,        UFS_PART_3},/* preas           360M    p60*/
  {PART_PREAVS,             1168*1024,      32*1024,        UFS_PART_3},/* preavs           32M    p61*/
  {PART_VERSION,            1200*1024,      32*1024,        UFS_PART_3},/* version          32M    p62*/
  {PART_PRELOAD,            1232*1024,       8*1024,        UFS_PART_3},/* preload           8M    p63*/
  {PART_RESERVED5,          1240*1024,     888*1024,        UFS_PART_3},/* reserved5       888M    p64*/
  {PART_CACHE,              2128*1024,     128*1024,        UFS_PART_3},/* cache           128M    p65*/
  {PART_PATCH,              2256*1024,      96*1024,        UFS_PART_3},/* patch            96M    p66*/
  {PART_SUPER,              2352*1024,    5536*1024,        UFS_PART_3},/* super          5536M    p67*/
  {PART_RESERVED6,          7888*1024,     128*1024,        UFS_PART_3},/* reserved6       128M    p68*/
  {PART_HIBENCH_DATA,       8016*1024,     512*1024,        UFS_PART_3},/* hibench_data    512M    p69*/
  {PART_USERDATA,           8528*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G      p70*/
#else
 #ifdef CONFIG_NEW_PRODUCT_P
  {PART_CACHE,              800*1024,        128*1024,        UFS_PART_3},/* cache           128M    p53*/
  {PART_RAMDISK,            928*1024,          2*1024,        UFS_PART_3},/* ramdisk           2M    p54*/
  {PART_VBMETA_SYSTEM,      930*1024,          1*1024,        UFS_PART_3},/* vbmeta_system     1M    p55*/
  {PART_VBMETA_VENDOR,      931*1024,          1*1024,        UFS_PART_3},/* vbmeta_vendor     1M    p56*/
  {PART_VBMETA_ODM,         932*1024,          1*1024,        UFS_PART_3},/* vbmeta_odm        1M    p57*/
  {PART_VBMETA_CUST,        933*1024,          1*1024,        UFS_PART_3},/* vbmeta_cust       1M    p58*/
  {PART_VBMETA_HW_PRODUCT,  934*1024,          1*1024,        UFS_PART_3},/* vbmeta_hw_product 1M    p59*/
  {PART_RESERVED4,          935*1024,          1*1024,        UFS_PART_3},/* reserved4         1M    p60*/
   #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,              936*1024,        360*1024,        UFS_PART_3},/* preas           360M    p61*/
  {PART_PREAVS,            1296*1024,         32*1024,        UFS_PART_3},/* preavs           32M    p62*/
  {PART_PATCH,             1328*1024,         96*1024,        UFS_PART_3},/* patch            96M    p63*/
  {PART_SUPER,            1424*1024,        7376*1024,        UFS_PART_3},/* super          7376M    p64*/
  {PART_VERSION,          8800*1024,         576*1024,        UFS_PART_3},/* version         576M    p65*/
  {PART_PRELOAD,          9376*1024,        1144*1024,        UFS_PART_3},/* preload        1144M    p66*/
  {PART_USERDATA,        10520*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata          4G    p67*/
   #else
  {PART_PREAS,              936*1024,       1216*1024,        UFS_PART_3},/* preas          1216M    p61*/
  {PART_PREAVS,            2152*1024,         32*1024,        UFS_PART_3},/* preavs           32M    p62*/
  {PART_PATCH,             2184*1024,         96*1024,        UFS_PART_3},/* patch            96M    p63*/
  {PART_SUPER,            2280*1024,        7440*1024,        UFS_PART_3},/* super          7440M    p64*/
  {PART_VERSION,          9720*1024,         576*1024,        UFS_PART_3},/* version         576M    p65*/
  {PART_PRELOAD,         10296*1024,        1144*1024,        UFS_PART_3},/* preload        1144M    p66*/
  {PART_USERDATA,        11440*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata          4G    p67*/
   #endif
 #else
  {PART_RAMDISK,           800*1024,           2*1024,        UFS_PART_3},/* ramdisk           2M    p53*/
  {PART_VBMETA_SYSTEM,     802*1024,           1*1024,        UFS_PART_3},/* vbmeta_system     1M    p54*/
  {PART_VBMETA_VENDOR,     803*1024,           1*1024,        UFS_PART_3},/* vbmeta_vendor     1M    p55*/
  {PART_VBMETA_ODM,        804*1024,           1*1024,        UFS_PART_3},/* vbmeta_odm        1M    p56*/
  {PART_VBMETA_CUST,       805*1024,           1*1024,        UFS_PART_3},/* vbmeta_cust       1M    p57*/
  {PART_VBMETA_HW_PRODUCT, 806*1024,           1*1024,        UFS_PART_3},/* vbmeta_hw_product 1M    p58*/
  {PART_RESERVED4,         807*1024,           1*1024,        UFS_PART_3},/* reserved4         1M    p59*/
  #ifdef CONFIG_HISI_CACHE_OVERSEA
   #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,             808*1024,         360*1024,        UFS_PART_3},/* preas           360M    p60*/
  {PART_PREAVS,           1168*1024,          32*1024,        UFS_PART_3},/* preavs           32M    p61*/
  {PART_VERSION,          1200*1024,          32*1024,        UFS_PART_3},/* version          32M    p62*/
  {PART_PRELOAD,          1232*1024,           8*1024,        UFS_PART_3},/* preload           8M    p63*/
  {PART_RESERVED5,        1240*1024,         888*1024,        UFS_PART_3},/* reserved5       888M    p64*/
  {PART_CACHE,            2128*1024,        1300*1024,        UFS_PART_3},/* cache          1300M    p65*/
  {PART_PATCH,            3428*1024,          96*1024,        UFS_PART_3},/* patch            96M    p66*/
  {PART_SUPER,            3524*1024,        4656*1024,        UFS_PART_3},/* super          4656M    p67*/
  {PART_USERDATA,         8180*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata          4G    p68*/
   #else
  {PART_PREAS,             808*1024,        1216*1024,        UFS_PART_3},/* preas          1216M    p60*/
  {PART_PREAVS,           2024*1024,          32*1024,        UFS_PART_3},/* preavs           32M    p61*/
  {PART_VERSION,          2056*1024,          32*1024,        UFS_PART_3},/* version          32M    p62*/
  {PART_PRELOAD,          2088*1024,           8*1024,        UFS_PART_3},/* preload           8M    p63*/
  {PART_RESERVED5,        2096*1024,          32*1024,        UFS_PART_3},/* reserved5        32M    p64*/
  {PART_CACHE,            2128*1024,        1300*1024,        UFS_PART_3},/* cache          1300M    p65*/
  {PART_PATCH,            3428*1024,          96*1024,        UFS_PART_3},/* patch            96M    p66*/
  {PART_SUPER,            3524*1024,        5536*1024,        UFS_PART_3},/* super          5536M    p67*/
  {PART_USERDATA,         9060*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata         4G     p68*/
   #endif
  #else
   #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,             808*1024,         360*1024,        UFS_PART_3},/* preas           360M    p60*/
  {PART_PREAVS,           1168*1024,          32*1024,        UFS_PART_3},/* preavs           32M    p61*/
  {PART_VERSION,          1200*1024,          32*1024,        UFS_PART_3},/* version          32M    p62*/
  {PART_PRELOAD,          1232*1024,           8*1024,        UFS_PART_3},/* preload           8M    p63*/
  {PART_RESERVED5,        1240*1024,         888*1024,        UFS_PART_3},/* reserved5       888M    p64*/
  {PART_CACHE,            2128*1024,         128*1024,        UFS_PART_3},/* cache           128M    p65*/
  {PART_PATCH,            2256*1024,          96*1024,        UFS_PART_3},/* patch            96M    p66*/
  {PART_SUPER,            2352*1024,        4656*1024,        UFS_PART_3},/* super          4656M    p67*/
  {PART_USERDATA,         7008*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata          4G    p68*/
   #else
  {PART_PREAS,             808*1024,        1216*1024,        UFS_PART_3},/* preas          1216M    p60*/
  {PART_PREAVS,           2024*1024,          32*1024,        UFS_PART_3},/* preavs           32M    p61*/
  {PART_VERSION,          2056*1024,          32*1024,        UFS_PART_3},/* version          32M    p62*/
  {PART_PRELOAD,          2088*1024,           8*1024,        UFS_PART_3},/* preload           8M    p63*/
  {PART_RESERVED5,        2096*1024,          32*1024,        UFS_PART_3},/* reserved5        32M    p64*/
  {PART_CACHE,            2128*1024,         128*1024,        UFS_PART_3},/* cache           128M    p65*/
  {PART_PATCH,            2256*1024,          96*1024,        UFS_PART_3},/* patch            96M    p66*/
  {PART_SUPER,            2352*1024,        5536*1024,        UFS_PART_3},/* super          5536M    p67*/
  {PART_USERDATA,         7888*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata          4G    p68*/
   #endif
  #endif
 #endif
#endif
  {"0", 0, 0, 0},
};
#endif

#endif
