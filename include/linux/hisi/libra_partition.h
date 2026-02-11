#ifndef _LIBRA_PARTITION_H_
#define _LIBRA_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,          0,         2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,        0,         2*1024,        EMMC_BOOT_BACKUP_PART},
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
  {PART_MODEM_DRIVER,     226*1024,  20*1024,       EMMC_USER_PART},/* modem_driver    20M    p20*/
  {PART_RAMDISK,             246*1024,   2*1024,    EMMC_USER_PART},/* ramdisk           32M    p21*/
  {PART_VBMETA_SYSTEM,       248*1024,   1*1024,    EMMC_USER_PART},/* vbmeta_system      1M    p22*/
  {PART_VBMETA_VENDOR,       249*1024,   1*1024,    EMMC_USER_PART},/* vbmeta_vendor      1M    p23*/
  {PART_VBMETA_ODM,          250*1024,   1*1024,    EMMC_USER_PART},/* vbmeta_odm         1M    p24*/
  {PART_VBMETA_CUST,         251*1024,   1*1024,    EMMC_USER_PART},/* vbmeta_cust        1M    p25*/
  {PART_VBMETA_HW_PRODUCT,   252*1024,   1*1024,    EMMC_USER_PART},/* vbmeta_hw_product  1M    p26*/
  {PART_SPLASH2,             253*1024,  80*1024,       EMMC_USER_PART},/* splash2         80M    p27*/
  {PART_BOOTFAIL_INFO,       333*1024,  2*1024,        EMMC_USER_PART},/* bootfail info   2MB    p28*/
  {PART_MISC,                335*1024,  2*1024,        EMMC_USER_PART},/* misc            2M     p29*/
  {PART_DFX,                 337*1024,  16*1024,       EMMC_USER_PART},/* dfx             16M    p30*/
  {PART_RRECORD,             353*1024,  16*1024,       EMMC_USER_PART},/* rrecord         16M    p31*/
  {PART_FW_LPM3,             369*1024,  256,           EMMC_USER_PART},/* mcuimage        256K   p32*/
  {PART_RESERVED3,           378112,    3840,          EMMC_USER_PART},/* reserved3A      3840KB p33*/
  {PART_HDCP,                373*1024,  1*1024,        EMMC_USER_PART},/* PART_HDCP        1M    p34*/
  {PART_HISEE_IMG,           374*1024,  4*1024,        EMMC_USER_PART},/* part_hisee_img   4M    p35*/
  {PART_HHEE,                378*1024,  4*1024,        EMMC_USER_PART},/* PART_RESERVED10  4M    p36*/
  {PART_HISEE_FS,            382*1024,  8*1024,        EMMC_USER_PART},/* hisee_fs        8M     p37*/
  {PART_FASTBOOT,            390*1024,  12*1024,       EMMC_USER_PART},/* fastboot        12M    p38*/
  {PART_VECTOR,              402*1024,  4*1024,        EMMC_USER_PART},/* avs vector      4M     p39*/
  {PART_ISP_BOOT,            406*1024,  2*1024,        EMMC_USER_PART},/* isp_boot        2M     p40*/
  {PART_ISP_FIRMWARE,        408*1024,  14*1024,       EMMC_USER_PART},/* isp_firmware    14M    p41*/
  {PART_FW_HIFI,             422*1024,  12*1024,       EMMC_USER_PART},/* hifi            12M    p42*/
  {PART_TEEOS,               434*1024,  8*1024,        EMMC_USER_PART},/* teeos           8M     p43*/
  {PART_SENSORHUB,           442*1024,  16*1024,       EMMC_USER_PART},/* sensorhub       16M    p44*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,    458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p45*/
  {PART_ERECOVERY_RAMDISK,   482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p46*/
  {PART_ERECOVERY_VENDOR,    514*1024,  8*1024,     EMMC_USER_PART},/* erecovery_vendor  8M     p47*/
  {PART_KERNEL,              522*1024,  32*1024,    EMMC_USER_PART},/* kernel            32M    p48*/
#else
  {PART_ERECOVERY_KERNEL,    458*1024,  24*1024,    EMMC_USER_PART},/* erecovery_kernel  24M    p45*/
  {PART_ERECOVERY_RAMDISK,   482*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p46*/
  {PART_ERECOVERY_VENDOR,    514*1024,  16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p47*/
  {PART_KERNEL,              530*1024,  24*1024,    EMMC_USER_PART},/* kernel            24M    p48*/
#endif
  {PART_ENG_SYSTEM,         554*1024,  12*1024,     EMMC_USER_PART},/* eng_system       12M    p49*/
  {PART_RECOVERY_RAMDISK,   566*1024,  32*1024,     EMMC_USER_PART},/* recovery_ramdisk 32M    p50*/
  {PART_RECOVERY_VENDOR,    598*1024,  16*1024,     EMMC_USER_PART},/* recovery_vendor  16M    p51*/
  {PART_DTS,                614*1024,  8*1024,      EMMC_USER_PART},/* dtimage          8M     p52*/
  {PART_DTO,                622*1024,  24*1024,     EMMC_USER_PART},/* dtoimage         8M     p53*/
  {PART_TRUSTFIRMWARE,      646*1024,  2*1024,      EMMC_USER_PART},/* trustfirmware    2M     p54*/
  {PART_MODEM_FW,           648*1024,  56*1024,     EMMC_USER_PART},/* modem_fw         56M    p55*/
  {PART_ENG_VENDOR,         704*1024,  12*1024,     EMMC_USER_PART},/* eng_vendor       12M    p56*/
  {PART_MODEM_PATCH_NV,     716*1024,  4*1024,      EMMC_USER_PART},/* modem_patch_nv   4M     p57*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_RESERVED4,          720*1024,  8*1024,      EMMC_USER_PART},/* reserved4        8M    p58*/
#elif (defined(CONFIG_MARKET_INTERNAL) || defined(CONFIG_MARKET_OVERSEA) || defined(CONFIG_MARKET_FULL_INTERNAL) || defined(CONFIG_MARKET_FULL_OVERSEA))
  /* rom update */
  {PART_PRELOAD,            720*1024,  8*1024,      EMMC_USER_PART},/* PART_PRELOAD     8M    p58*/
#else
  {PART_RESERVED4,          720*1024,  8*1024,      EMMC_USER_PART},/* reserved4        8M    p58*/
#endif
  {PART_RECOVERY_VBMETA,    728*1024,  2*1024,      EMMC_USER_PART},/* recovery_vbmeta   2M   p59*/
  {PART_ERECOVERY_VBMETA,   730*1024,  2*1024,      EMMC_USER_PART},/* erecovery_vbmeta   2M  p60*/
  {PART_VBMETA,             732*1024,  4*1024,      EMMC_USER_PART},/* PART_VBMETA     4M     p61*/
  {PART_MODEMNVM_UPDATE,    736*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_update   16M  p62*/
  {PART_MODEMNVM_CUST,      752*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_cust   16M    p63*/
  {PART_PATCH,              768*1024,  32*1024,     EMMC_USER_PART},/* patch           32M    p64*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_CACHE,              800*1024,  104*1024,    EMMC_USER_PART},/* cache           104M   p65*/
  #ifdef CONFIG_USE_EROFS
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       3608*1024,    EMMC_USER_PART},/* super          3608M    p68*/
  {PART_VERSION,            4680*1024,        200*1024,    EMMC_USER_PART},/* version         200M    p69*/
  {PART_PRELOAD,            4880*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p70*/
  {PART_USERDATA,           5808*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,               904*1024,        848*1024,    EMMC_USER_PART},/* preas           848M    p66*/
  {PART_PREAVS,             1752*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1784*1024,       3256*1024,    EMMC_USER_PART},/* super          3256M    p68*/
  {PART_VERSION,            5040*1024,        200*1024,    EMMC_USER_PART},/* version         200M    p69*/
  {PART_PRELOAD,            5240*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p70*/
  {PART_USERDATA,           6168*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_PREAS,               904*1024,        360*1024,    EMMC_USER_PART},/* preas           360M    p66*/
  {PART_PREAVS,             1264*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1296*1024,       5144*1024,    EMMC_USER_PART},/* super          5144M    p68*/
  {PART_VERSION,            6440*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            7016*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M   p70*/
  {PART_USERDATA,           8160*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_PREAS,               904*1024,       1008*1024,    EMMC_USER_PART},/* preas          1008M    p66*/
  {PART_PREAVS,             1912*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1944*1024,       5096*1024,    EMMC_USER_PART},/* super          5096M    p68*/
  {PART_VERSION,            7040*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            7616*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M   p70*/
  {PART_USERDATA,           8760*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #else
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       6604*1024,    EMMC_USER_PART},/* super          6604M    p68*/
  {PART_VERSION,            7676*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            8252*1024,        900*1024,    EMMC_USER_PART},/* preload         900M    p70*/
  {PART_USERDATA,           9152*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #endif
  #else /* do not use erofs */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       5632*1024,    EMMC_USER_PART},/* super          5632M    p68*/
  {PART_VERSION,            6704*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            7280*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p70*/
  {PART_USERDATA,           8208*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       6296*1024,    EMMC_USER_PART},/* super          6296M    p68*/
  {PART_VERSION,            7368*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            7944*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p70*/
  {PART_USERDATA,           8872*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata         4G     p71*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       7216*1024,    EMMC_USER_PART},/* super          7216M    p67*/
  {PART_VERSION,            8288*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p68*/
  {PART_PRELOAD,            8864*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M   p69*/
  {PART_USERDATA,          10008*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p70*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       8136*1024,    EMMC_USER_PART},/* super          8136M    p68*/
  {PART_VERSION,            9208*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            9784*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M   p70*/
  {PART_USERDATA,          10928*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p71*/
  #else
  {PART_PREAS,               904*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p66*/
  {PART_PREAVS,             1040*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p67*/
  {PART_SUPER,              1072*1024,       7020*1024,    EMMC_USER_PART},/* super          7020M    p68*/
  {PART_VERSION,            8092*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p69*/
  {PART_PRELOAD,            8668*1024,        900*1024,    EMMC_USER_PART},/* preload         900M    p70*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,          9568*1024,        128*1024,    EMMC_USER_PART},/* hibench_img      128M    p69*/
  {PART_HIBENCH_DATA,       9696*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M    p70*/
  {PART_FLASH_AGEING,      10208*1024,        512*1024,    EMMC_USER_PART},/* flash_ageing   512M    p71*/
  {PART_HIBENCH_LOG,       10720*1024,         32*1024,     EMMC_USER_PART},/* HIBENCH_LOG    32M    p72*/
  {PART_USERDATA,          10752*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p73*/
  #else
  {PART_USERDATA,           9568*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p74*/
  #endif
  #endif
  #endif
#else /* for rom update */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_VERSION,             800*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p65*/
  {PART_PREAS,               832*1024,        136*1024,    EMMC_USER_PART},/* preas          136M    p66*/
  {PART_PREAVS,              968*1024,         32*1024,    EMMC_USER_PART},/* preavs          32M    p67*/
  {PART_RESERVED8,          1000*1024,       1152*1024,    EMMC_USER_PART},/* reserved8     1152M    p68*/
  {PART_CACHE,              2152*1024,        104*1024,    EMMC_USER_PART},/* cache          104M    p69*/
  {PART_SUPER,              2256*1024,       3088*1024,    EMMC_USER_PART},/* super         3088M    p70*/
  {PART_USERDATA,           5344*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G     p71*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_VERSION,             800*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p65*/
  {PART_PREAS,               832*1024,       1008*1024,    EMMC_USER_PART},/* preas         1008M    p66*/
  {PART_PREAVS,             1840*1024,         32*1024,    EMMC_USER_PART},/* preavs          32M    p67*/
  {PART_RESERVED8,          1872*1024,        280*1024,    EMMC_USER_PART},/* reserved8      280M    p68*/
  {PART_CACHE,              2152*1024,        104*1024,    EMMC_USER_PART},/* cache          104M    p69*/
  {PART_SUPER,              2256*1024,       3536*1024,    EMMC_USER_PART},/* super         3536M    p70*/
  {PART_USERDATA,           5792*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata         4G    p71*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_VERSION,             800*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p65*/
  {PART_PREAS,               832*1024,       1216*1024,    EMMC_USER_PART},/* preas         1216M    p66*/
  {PART_PREAVS,             2048*1024,         32*1024,    EMMC_USER_PART},/* preavs          32M    p67*/
  {PART_RESERVED8,          2080*1024,         72*1024,    EMMC_USER_PART},/* reserved8       72M    p68*/
  {PART_CACHE,              2152*1024,        104*1024,    EMMC_USER_PART},/* cache          104M    p69*/
  {PART_SUPER,              2256*1024,       5632*1024,    EMMC_USER_PART},/* super         5632M    p70*/
  {PART_USERDATA,           7888*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata          4G   p71*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_VERSION,             800*1024,         32*1024,    EMMC_USER_PART},/* version         32M    p65*/
  {PART_PREAS,               832*1024,        360*1024,    EMMC_USER_PART},/* preas          360M    p66*/
  {PART_PREAVS,             1192*1024,         32*1024,    EMMC_USER_PART},/* preavs          32M    p67*/
  {PART_RESERVED8,          1224*1024,        928*1024,    EMMC_USER_PART},/* reserved8      928M    p68*/
  {PART_CACHE,              2152*1024,        104*1024,    EMMC_USER_PART},/* cache          104M    p69*/
  {PART_SUPER,              2256*1024,       4752*1024,    EMMC_USER_PART},/* super         4752M    p70*/
  {PART_USERDATA,           7008*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata         4G    p71*/
  #else  /* FOR NEW PRODUCT FACTORY, HISI, SOMETHING ELSE */
  {PART_PREAS,               800*1024,        136*1024,    EMMC_USER_PART},/* preas          136M   p65*/
  {PART_PREAVS,              936*1024,         32*1024,    EMMC_USER_PART},/* preavs          32M   p66*/
  {PART_SUPER,               968*1024,       7020*1024,    EMMC_USER_PART},/* super          7020M   p67*/  
  {PART_CACHE,              7988*1024,        104*1024,    EMMC_USER_PART},/* cache           104M   p68*/
  {PART_VERSION,            8092*1024,        576*1024,    EMMC_USER_PART},/* version         576M   p69*/
  {PART_PRELOAD,            8668*1024,        900*1024,    EMMC_USER_PART},/* preload         900M   p70*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,        9568*1024,        128*1024,    EMMC_USER_PART},/*hibench_img     128M    p71*/
  {PART_HIBENCH_DATA,       9696*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M    p72*/
  {PART_FLASH_AGEING,      10208*1024,        512*1024,    EMMC_USER_PART},/* flash_ageing   512M    p73*/
  {PART_HIBENCH_LOG,       10720*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG    32M     p74*/
  {PART_USERDATA,          10752*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p75*/
  #else
  {PART_USERDATA,           9568*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p71*/
  #endif
  #endif  /* CONFIG_MARKET_INTERNAL */
#endif  /* CONFIG_NEW_PRODUCT_P */

  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER,          0,         2*1024,        UFS_PART_0},
  {PART_RESERVED0,        0,         2*1024,        UFS_PART_1},
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
  {PART_MODEM_DRIVER,     218*1024,  20*1024,       UFS_PART_3},/* modem_driver    20M    p16*/
  {PART_RAMDISK,          238*1024,   2*1024,       UFS_PART_3},/* ramdisk            2M  p17*/
  {PART_VBMETA_SYSTEM,    240*1024,   1*1024,       UFS_PART_3},/* vbmeta_system      1M  p18*/
  {PART_VBMETA_VENDOR,    241*1024,   1*1024,       UFS_PART_3},/* vbmeta_vendor      1M  p19*/
  {PART_VBMETA_ODM,       242*1024,   1*1024,       UFS_PART_3},/* vbmeta_odm         1M  p20*/
  {PART_VBMETA_CUST,      243*1024,   1*1024,       UFS_PART_3},/* vbmeta_cust        1M  p21*/
  {PART_VBMETA_HW_PRODUCT,244*1024,   1*1024,       UFS_PART_3},/* vbmeta_hw_product  1M  p22*/
  {PART_SPLASH2,          245*1024,  80*1024,       UFS_PART_3},/* splash2         80M    p23*/
  {PART_BOOTFAIL_INFO,    325*1024,  2*1024,        UFS_PART_3},/* bootfail info   2MB    p24*/
  {PART_MISC,             327*1024,  2*1024,        UFS_PART_3},/* misc            2M     p25*/
  {PART_DFX,              329*1024,  16*1024,       UFS_PART_3},/* dfx             16M    p26*/
  {PART_RRECORD,          345*1024,  16*1024,       UFS_PART_3},/* rrecord         16M    p27*/
  {PART_FW_LPM3,          361*1024,  256,           UFS_PART_3},/* mcuimage        256K   p28*/
  {PART_RESERVED3,        369920,    3840,          UFS_PART_3},/* reserved3A      3840K  p29*/
  {PART_HDCP,             365*1024,  1*1024,        UFS_PART_3},/* PART_HDCP       1M     p30*/
  {PART_HISEE_IMG,        366*1024,  4*1024,        UFS_PART_3},/* part_hisee_img   4M    p31*/
  {PART_HHEE,             370*1024,  4*1024,        UFS_PART_3},/* PART_RESERVED10  4M    p32*/
  {PART_HISEE_FS,         374*1024,  8*1024,        UFS_PART_3},/* hisee_fs        8M     p33*/
  {PART_FASTBOOT,         382*1024,  12*1024,       UFS_PART_3},/* fastboot        12M    p34*/
  {PART_VECTOR,           394*1024,  4*1024,        UFS_PART_3},/* avs vector      4M     p35*/
  {PART_ISP_BOOT,         398*1024,  2*1024,        UFS_PART_3},/* isp_boot        2M     p36*/
  {PART_ISP_FIRMWARE,     400*1024,  14*1024,       UFS_PART_3},/* isp_firmware    14M    p37*/
  {PART_FW_HIFI,          414*1024,  12*1024,       UFS_PART_3},/* hifi            12M    p38*/
  {PART_TEEOS,            426*1024,  8*1024,        UFS_PART_3},/* teeos           8M     p39*/
  {PART_SENSORHUB,        434*1024,  16*1024,       UFS_PART_3},/* sensorhub       16M    p40*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,    450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p41*/
  {PART_ERECOVERY_RAMDISK,   474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p42*/
  {PART_ERECOVERY_VENDOR,    506*1024,  8*1024,     UFS_PART_3},/* erecovery_vendor  8M    p43*/
  {PART_KERNEL,              514*1024,  32*1024,    UFS_PART_3},/* kernel            32M   p44*/
#else
  {PART_ERECOVERY_KERNEL,    450*1024,  24*1024,    UFS_PART_3},/* erecovery_kernel  24M   p41*/
  {PART_ERECOVERY_RAMDISK,   474*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M   p42*/
  {PART_ERECOVERY_VENDOR,    506*1024,  16*1024,    UFS_PART_3},/* erecovery_vendor  16M   p43*/
  {PART_KERNEL,              522*1024,  24*1024,    UFS_PART_3},/* kernel            24M   p44*/
#endif
  {PART_ENG_SYSTEM,         546*1024,  12*1024,     UFS_PART_3},/* eng_system        12M    p45*/
  {PART_RECOVERY_RAMDISK,   558*1024,  32*1024,     UFS_PART_3},/* recovery_ramdisk  32M    p46*/
  {PART_RECOVERY_VENDOR,    590*1024,  16*1024,     UFS_PART_3},/* recovery_vendor   16M    p47*/
  {PART_DTS,                606*1024,   8*1024,     UFS_PART_3},/* dtimage            8M    p48*/
  {PART_DTO,                614*1024,  24*1024,     UFS_PART_3},/* dtoimage          24M    p49*/
  {PART_TRUSTFIRMWARE,      638*1024,   2*1024,     UFS_PART_3},/* trustfirmware      2M    p50*/
  {PART_MODEM_FW,           640*1024,  56*1024,     UFS_PART_3},/* modem_fw          56M    p51*/
  {PART_ENG_VENDOR,         696*1024,  12*1024,     UFS_PART_3},/* eng_vendor        12M    p52*/
  {PART_MODEM_PATCH_NV,     708*1024,   4*1024,     UFS_PART_3},/* modem_patch_nv     4M    p53*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_RESERVED4,          712*1024,  8*1024,      UFS_PART_3},/* reserved4          8M    p54*/
#elif (defined(CONFIG_MARKET_INTERNAL) || defined(CONFIG_MARKET_OVERSEA) || defined(CONFIG_MARKET_FULL_INTERNAL) || defined(CONFIG_MARKET_FULL_OVERSEA))
  /* rom update */
  {PART_PRELOAD,            712*1024,  8*1024,      UFS_PART_3},/* PART_PRELOAD       8M    p54*/
#else
  {PART_RESERVED4,          712*1024,  8*1024,      UFS_PART_3},/* reserved4          8M    p54*/
#endif
  {PART_RECOVERY_VBMETA,    720*1024,  2*1024,      UFS_PART_3},/* recovery_vbmeta    2M    p55*/
  {PART_ERECOVERY_VBMETA,   722*1024,  2*1024,      UFS_PART_3},/* erecovery_vbmeta   2M    p56*/
  {PART_VBMETA,             724*1024,  4*1024,      UFS_PART_3},/* vbmeta             4M    p57*/
  {PART_MODEMNVM_UPDATE,    728*1024, 16*1024,      UFS_PART_3},/* modemnvm_update    16M   p58*/
  {PART_MODEMNVM_CUST,      744*1024, 16*1024,      UFS_PART_3},/* modemnvm_cust      16M   p59*/
  {PART_PATCH,              760*1024, 32*1024,      UFS_PART_3},/* patch              32M   p60*/
#ifdef CONFIG_NEW_PRODUCT_P
  {PART_CACHE,              792*1024,  104*1024,    UFS_PART_3},/* version           104M   p61*/
  #ifdef CONFIG_USE_EROFS
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        3608*1024,    UFS_PART_3},/* super        3608M    p64*/
  {PART_VERSION,            4672*1024,         200*1024,    UFS_PART_3},/* version       200M    p65*/
  {PART_PRELOAD,            4872*1024,         928*1024,    UFS_PART_3},/* preload       928M    p66*/
  {PART_USERDATA,           5800*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,               896*1024,         848*1024,    UFS_PART_3},/* preas         848M    p62*/
  {PART_PREAVS,             1744*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1776*1024,        3256*1024,    UFS_PART_3},/* super        3256M    p64*/
  {PART_VERSION,            5032*1024,         200*1024,    UFS_PART_3},/* version       200M    p65*/
  {PART_PRELOAD,            5232*1024,         928*1024,    UFS_PART_3},/* preload       928M    p66*/
  {PART_USERDATA,           6160*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_PREAS,               896*1024,         360*1024,    UFS_PART_3},/* preas         360M    p62*/
  {PART_PREAVS,             1256*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1288*1024,        5144*1024,    UFS_PART_3},/* super        5144M    p64*/
  {PART_VERSION,            6432*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            7008*1024,        1144*1024,    UFS_PART_3},/* preload      1144M    p66*/
  {PART_USERDATA,           8152*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_PREAS,               896*1024,        1008*1024,    UFS_PART_3},/* preas        1008M    p62*/
  {PART_PREAVS,             1904*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1936*1024,        5096*1024,    UFS_PART_3},/* super        5096M    p64*/
  {PART_VERSION,            7032*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            7608*1024,        1144*1024,    UFS_PART_3},/* preload      1144M    p66*/
  {PART_USERDATA,           8752*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #else
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        6604*1024,    UFS_PART_3},/* super        6604M    p64*/
  {PART_VERSION,            7668*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            8244*1024,         900*1024,    UFS_PART_3},/* preload       900M    p66*/
  {PART_USERDATA,           9144*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #endif
  #else /* do not use erofs */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        5632*1024,    UFS_PART_3},/* super        5632M    p64*/
  {PART_VERSION,            6696*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            7272*1024,         928*1024,    UFS_PART_3},/* preload       928M    p66*/
  {PART_USERDATA,           8200*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        6296*1024,    UFS_PART_3},/* super        6296M    p64*/
  {PART_VERSION,            7360*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            7936*1024,         928*1024,    UFS_PART_3},/* preload       928M    p66*/
  {PART_USERDATA,           8864*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        7216*1024,    UFS_PART_3},/* super        7216M    p64*/
  {PART_VERSION,            8280*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            8856*1024,        1144*1024,    UFS_PART_3},/* preload      1144M    p66*/
  {PART_USERDATA,          10000*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        8136*1024,    UFS_PART_3},/* super        8136M    p64*/
  {PART_VERSION,            9200*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            9776*1024,        1144*1024,    UFS_PART_3},/* preload      1144M    p66*/
  {PART_USERDATA,          10920*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #else
  {PART_PREAS,               896*1024,         136*1024,    UFS_PART_3},/* preas         136M    p62*/
  {PART_PREAVS,             1032*1024,          32*1024,    UFS_PART_3},/* preavs         32M    p63*/
  {PART_SUPER,              1064*1024,        7020*1024,    UFS_PART_3},/* super        7020M    p64*/
  {PART_VERSION,            8084*1024,         576*1024,    UFS_PART_3},/* version       576M    p65*/
  {PART_PRELOAD,            8660*1024,         900*1024,    UFS_PART_3},/* preload       900M    p66*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,        9560*1024,         128*1024,    UFS_PART_3},/* hibench_img   128M    p67*/
  {PART_HIBENCH_DATA,       9688*1024,         512*1024,    UFS_PART_3},/* hibench_data  512M    p68*/
  {PART_FLASH_AGEING,      10200*1024,         512*1024,    UFS_PART_3},/* flash_ageing  512M    p69*/
  {PART_HIBENCH_LOG,       10712*1024,          32*1024,    UFS_PART_3},/* HIBENCH_LOG    32M    p70*/
  {PART_USERDATA,          10744*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p71*/
  #else
  {PART_USERDATA,           9560*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G    p67*/
  #endif
  #endif
  #endif
#else /* for rom update */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_VERSION,             792*1024,          32*1024,    UFS_PART_3},/* version         32M   p61*/
  {PART_PREAS,               824*1024,         136*1024,    UFS_PART_3},/* preas          136M   p62*/
  {PART_PREAVS,              960*1024,          32*1024,    UFS_PART_3},/* preavs          32M   p63*/
  {PART_RESERVED8,           992*1024,        1152*1024,    UFS_PART_3},/* reserved8     1152M   p64*/
  {PART_CACHE,              2144*1024,         104*1024,    UFS_PART_3},/* cache          104M   p65*/
  {PART_SUPER,              2248*1024,        3088*1024,    UFS_PART_3},/* super         3088M   p66*/
  {PART_USERDATA,           5336*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata         4G   p67*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_VERSION,             792*1024,          32*1024,    UFS_PART_3},/* version         32M    p61*/
  {PART_PREAS,               824*1024,        1008*1024,    UFS_PART_3},/* preas         1008M    p62*/
  {PART_PREAVS,             1832*1024,          32*1024,    UFS_PART_3},/* preavs          32M    p63*/
  {PART_RESERVED8,          1864*1024,         280*1024,    UFS_PART_3},/* reserved8      280M    p64*/
  {PART_CACHE,              2144*1024,         104*1024,    UFS_PART_3},/* cache          104M    p65*/
  {PART_SUPER,              2248*1024,        3536*1024,    UFS_PART_3},/* super         3536M    p66*/
  {PART_USERDATA,           5784*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata         4G    p67*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_VERSION,             792*1024,          32*1024,    UFS_PART_3},/* version         32M    p61*/
  {PART_PREAS,               824*1024,        1216*1024,    UFS_PART_3},/* preas         1216M    p62*/
  {PART_PREAVS,             2040*1024,          32*1024,    UFS_PART_3},/* preavs          32M    p63*/
  {PART_RESERVED8,          2072*1024,          72*1024,    UFS_PART_3},/* reserved8       72M    p64*/
  {PART_CACHE,              2144*1024,         104*1024,    UFS_PART_3},/* cache          104M    p65*/
  {PART_SUPER,              2248*1024,        5632*1024,    UFS_PART_3},/* super         5632M    p66*/
  {PART_USERDATA,           7880*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata         4G    p67*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_VERSION,             792*1024,          32*1024,    UFS_PART_3},/* version         32M    p61*/
  {PART_PREAS,               824*1024,         360*1024,    UFS_PART_3},/* preas          360M    p62*/
  {PART_PREAVS,             1184*1024,          32*1024,    UFS_PART_3},/* preavs          32M    p63*/
  {PART_RESERVED8,          1216*1024,         928*1024,    UFS_PART_3},/* reserved8      928M    p64*/
  {PART_CACHE,              2144*1024,         104*1024,    UFS_PART_3},/* cache          104M    p65*/
  {PART_SUPER,              2248*1024,        4752*1024,    UFS_PART_3},/* super         4752M    p66*/
  {PART_USERDATA,           7000*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata         4G    p67*/
  #else  /* FOR NEW PRODUCT FACTORY, HISI, SOMETHING ELSE */
  {PART_PREAS,               792*1024,         136*1024,    UFS_PART_3},/* preas         136M     p61*/
  {PART_PREAVS,              928*1024,          32*1024,    UFS_PART_3},/* preavs         32M     p62*/
  {PART_SUPER,               960*1024,        7020*1024,    UFS_PART_3},/* super         7020M    p63*/
  {PART_CACHE,              7980*1024,         104*1024,    UFS_PART_3},/* cache          104M    p64*/
  {PART_VERSION,            8084*1024,         576*1024,    UFS_PART_3},/* version        576M    p65*/
  {PART_PRELOAD,            8660*1024,         900*1024,    UFS_PART_3},/* preload        900M    p66*/
  #ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,        9560*1024,         128*1024,    UFS_PART_3},/* hibench_img    128M    p67*/
  {PART_HIBENCH_DATA,       9688*1024,         512*1024,    UFS_PART_3},/* hibench_data   512M    p68*/
  {PART_FLASH_AGEING,      10200*1024,         512*1024,    UFS_PART_3},/* flash_ageing   512M    p69*/
  {PART_HIBENCH_LOG,       10712*1024,          32*1024,    UFS_PART_3},/* HIBENCH_LOG     32M    p70*/
  {PART_USERDATA,          10744*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata         4G    p71*/
  #else
  {PART_USERDATA,           9560*1024,  (4UL)*1024*1024,    UFS_PART_3},/* userdata         4G    p67*/
  #endif
  #endif  /* CONFIG_MARKET_INTERNAL */
#endif  /* CONFIG_NEW_PRODUCT_P */
  {"0", 0, 0, 0},
};
#endif

#endif
