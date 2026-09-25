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
  {PART_KPATCH,              378112,    3840,          EMMC_USER_PART},/* kpatch          3840KB p33*/
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
  {PART_ERECOVERY_RAMDISK,   458*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p45*/
  {PART_ERECOVERY_VENDOR,    490*1024,   8*1024,    EMMC_USER_PART},/* erecovery_vendor   8M    p46*/
  {PART_BOOT,                498*1024,  40*1024,    EMMC_USER_PART},/* boot              40M    p47*/
  {PART_RECOVERY,            538*1024,  64*1024,    EMMC_USER_PART},/* recovery          64M    p48*/
  {PART_ERECOVERY,           602*1024,  64*1024,    EMMC_USER_PART},/* erecovery         64M    p49*/
  {PART_METADATA,            666*1024,  16*1024,    EMMC_USER_PART},/* metadata          16M    p50*/
  {PART_RESERVED8,           682*1024,  78*1024,    EMMC_USER_PART},/* reserved8         78M    p51*/
#else
  {PART_ERECOVERY_RAMDISK,   458*1024,  32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p45*/
  {PART_ERECOVERY_VENDOR,    490*1024,  16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p46*/
  {PART_BOOT,                506*1024,  32*1024,    EMMC_USER_PART},/* boot              32M    p47*/
  {PART_RECOVERY,            538*1024,  56*1024,    EMMC_USER_PART},/* recovery          56M    p48*/
  {PART_ERECOVERY,           594*1024,  56*1024,    EMMC_USER_PART},/* erecovery         56M    p49*/
  {PART_METADATA,            650*1024,  16*1024,    EMMC_USER_PART},/* metadata          16M    p50*/
  {PART_RESERVED8,           666*1024,  94*1024,    EMMC_USER_PART},/* reserved8         94M    p51*/
#endif
  {PART_ENG_SYSTEM,          760*1024,  12*1024,     EMMC_USER_PART},/* eng_system       12M    p52*/
  {PART_RECOVERY_RAMDISK,    772*1024,  32*1024,     EMMC_USER_PART},/* recovery_ramdisk 32M    p53*/
  {PART_RECOVERY_VENDOR,     804*1024,  16*1024,     EMMC_USER_PART},/* recovery_vendor  16M    p54*/
  {PART_FW_DTB,              820*1024,   2*1024,     EMMC_USER_PART},/* fw_dtb            2M    p55*/
  {PART_DTBO,                822*1024,  24*1024,     EMMC_USER_PART},/* dtoimage         24M    p56*/
  {PART_TRUSTFIRMWARE,       846*1024,   2*1024,     EMMC_USER_PART},/* trustfirmware    2M     p57*/
  {PART_MODEM_FW,            848*1024,  56*1024,     EMMC_USER_PART},/* modem_fw         56M    p58*/
  {PART_ENG_VENDOR,          904*1024,  12*1024,     EMMC_USER_PART},/* eng_vendor       12M    p59*/
  {PART_MODEM_PATCH_NV,      916*1024,   4*1024,     EMMC_USER_PART},/* modem_patch_nv   4M     p60*/
  {PART_RESERVED4,           920*1024,   8*1024,     EMMC_USER_PART},/* reserved4        8M     p61*/
  {PART_RECOVERY_VBMETA,     928*1024,   2*1024,     EMMC_USER_PART},/* recovery_vbmeta   2M    p62*/
  {PART_ERECOVERY_VBMETA,    930*1024,   2*1024,     EMMC_USER_PART},/* erecovery_vbmeta   2M   p63*/
  {PART_VBMETA,              932*1024,   4*1024,     EMMC_USER_PART},/* PART_VBMETA     4M      p64*/
  {PART_MODEMNVM_UPDATE,     936*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_update   16M   p65*/
  {PART_MODEMNVM_CUST,       952*1024,  16*1024,     EMMC_USER_PART},/* modemnvm_cust   16M     p66*/
  {PART_PATCH,               968*1024,  32*1024,     EMMC_USER_PART},/* patch           32M     p67*/
  {PART_CACHE,              1000*1024, 104*1024,     EMMC_USER_PART},/* cache           104M    p68*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,              1104*1024,        136*1024,    EMMC_USER_PART},/* preas          136M    p69*/
  {PART_PREAVS,             1240*1024,         32*1024,    EMMC_USER_PART},/* preavs          32M    p70*/
  {PART_SUPER,              1272*1024,       7020*1024,    EMMC_USER_PART},/* super         7020M    p71*/
  {PART_VERSION,            8292*1024,        576*1024,    EMMC_USER_PART},/* version        576M    p72*/
  {PART_PRELOAD,            8868*1024,        900*1024,    EMMC_USER_PART},/* preload        900M    p73*/
  {PART_HIBENCH_IMG,        9768*1024,        128*1024,    EMMC_USER_PART},/*hibench_img     128M    p74*/
  {PART_HIBENCH_DATA,       9896*1024,        512*1024,    EMMC_USER_PART},/* hibench_data   512M    p75*/
  {PART_FLASH_AGEING,      10408*1024,        512*1024,    EMMC_USER_PART},/* flash_ageing   512M    p76*/
  {PART_HIBENCH_LOG,       10920*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG     32M    p77*/
  {PART_USERDATA,          10952*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G      p78*/
#else
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,              1104*1024,        312*1024,    EMMC_USER_PART},/* preas           312M    p69*/
  {PART_PREAVS,             1416*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              1448*1024,       5064*1024,    EMMC_USER_PART},/* super          5064M    p71*/
  {PART_VERSION,            6512*1024,         32*1024,    EMMC_USER_PART},/* version          32M    p72*/
  {PART_PRELOAD,            6544*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p73*/
  {PART_USERDATA,           7472*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p74*/
#elif defined CONFIG_MARKET_OVERSEA
#ifdef CONFIG_16G_STORAGE
  {PART_PREAS,              1104*1024,       760*1024,    EMMC_USER_PART},/* preas          1312M    p69*/
  {PART_PREAVS,             1864*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              1896*1024,       3584*1024,    EMMC_USER_PART},/* super          4344M    p71*/
  {PART_VERSION,            5480*1024,         32*1024,    EMMC_USER_PART},/* version          32M    p72*/
  {PART_PRELOAD,            5512*1024,        600*1024,    EMMC_USER_PART},/* preload         928M    p73*/
  {PART_USERDATA,           6112*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p74*/
#else
  {PART_PREAS,              1104*1024,       1312*1024,    EMMC_USER_PART},/* preas          1312M    p69*/
  {PART_PREAVS,             2416*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              2448*1024,       4344*1024,    EMMC_USER_PART},/* super          4344M    p71*/
  {PART_VERSION,            6792*1024,         32*1024,    EMMC_USER_PART},/* version          32M    p72*/
  {PART_PRELOAD,            6824*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p73*/
  {PART_USERDATA,           7752*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p74*/
#endif
#else
  {PART_PREAS,              1104*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p69*/
  {PART_PREAVS,             1240*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              1272*1024,       7020*1024,    EMMC_USER_PART},/* super          7020M    p71*/
  {PART_VERSION,            8292*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p72*/
  {PART_PRELOAD,            8868*1024,        900*1024,    EMMC_USER_PART},/* preload         900M    p73*/
  {PART_USERDATA,           9768*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G       p74*/
#endif  /* internal */
#else  /* not erofs, ext4*/
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,              1104*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p69*/
  {PART_PREAVS,             1240*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              1272*1024,       4176*1024,    EMMC_USER_PART},/* super          4176M    p71*/
  {PART_VERSION,            5448*1024,         32*1024,    EMMC_USER_PART},/* version          32M    p72*/
  {PART_PRELOAD,            5480*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p73*/
  {PART_USERDATA,           6408*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p74*/
#elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,              1104*1024,       1008*1024,    EMMC_USER_PART},/* preas          1008M    p69*/
  {PART_PREAVS,             2112*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              2144*1024,       3536*1024,    EMMC_USER_PART},/* super          3536M    p71*/
  {PART_VERSION,            5680*1024,         32*1024,    EMMC_USER_PART},/* version          32M    p72*/
  {PART_PRELOAD,            5712*1024,        928*1024,    EMMC_USER_PART},/* preload         928M    p73*/
  {PART_USERDATA,           6640*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata        4G      p74*/
#else
  {PART_PREAS,              1104*1024,        136*1024,    EMMC_USER_PART},/* preas           136M    p69*/
  {PART_PREAVS,             1240*1024,         32*1024,    EMMC_USER_PART},/* preavs           32M    p70*/
  {PART_SUPER,              1272*1024,       7020*1024,    EMMC_USER_PART},/* super          7020M    p71*/
  {PART_VERSION,            8292*1024,        576*1024,    EMMC_USER_PART},/* version         576M    p72*/
  {PART_PRELOAD,            8868*1024,        900*1024,    EMMC_USER_PART},/* preload         900M    p73*/
  {PART_USERDATA,           9768*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata       4G       p74*/
#endif  /* internal */
#endif  /* erofs, ext4 */
#endif  /* factory*/
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
  {PART_KPATCH,           369920,    3840,          UFS_PART_3},/* kpatch          3840K  p29*/
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
  {PART_ERECOVERY_RAMDISK,   450*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p41*/
  {PART_ERECOVERY_VENDOR,    482*1024,   8*1024,    UFS_PART_3},/* erecovery_vendor   8M    p42*/
  {PART_BOOT,                490*1024,  40*1024,    UFS_PART_3},/* boot              40M    p43*/
  {PART_RECOVERY,            530*1024,  64*1024,    UFS_PART_3},/* recovery          64M    p44*/
  {PART_ERECOVERY,           594*1024,  64*1024,    UFS_PART_3},/* erecovery         64M    p45*/
  {PART_METADATA,            658*1024,  16*1024,    UFS_PART_3},/* metadata          16M    p46*/
  {PART_RESERVED8,           674*1024,  78*1024,    UFS_PART_3},/* reserved8         78M    p47*/
#else
  {PART_ERECOVERY_RAMDISK,   450*1024,  32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p41*/
  {PART_ERECOVERY_VENDOR,    482*1024,  16*1024,    UFS_PART_3},/* erecovery_vendor  16M    p42*/
  {PART_BOOT,                498*1024,  32*1024,    UFS_PART_3},/* boot              32M    p43*/
  {PART_RECOVERY,            530*1024,  56*1024,    UFS_PART_3},/* recovery          56M    p44*/
  {PART_ERECOVERY,           586*1024,  56*1024,    UFS_PART_3},/* erecovery         56M    p45*/
  {PART_METADATA,            642*1024,  16*1024,    UFS_PART_3},/* metadata          16M    p46*/
  {PART_RESERVED8,           658*1024,  94*1024,    UFS_PART_3},/* reserved8         94M    p47*/
#endif
  {PART_ENG_SYSTEM,          752*1024,  12*1024,    UFS_PART_3},/* eng_system         12M    p48*/
  {PART_RECOVERY_RAMDISK,    764*1024,  32*1024,    UFS_PART_3},/* recovery_ramdisk   32M    p49*/
  {PART_RECOVERY_VENDOR,     796*1024,  16*1024,    UFS_PART_3},/* recovery_vendor    16M    p50*/
  {PART_FW_DTB,              812*1024,   2*1024,    UFS_PART_3},/* fw_dtb              2M    p51*/
  {PART_DTBO,                814*1024,  24*1024,    UFS_PART_3},/* dtoimage           24M    p52*/
  {PART_TRUSTFIRMWARE,       838*1024,   2*1024,    UFS_PART_3},/* trustfirmware       2M    p53*/
  {PART_MODEM_FW,            840*1024,  56*1024,    UFS_PART_3},/* modem_fw           56M    p54*/
  {PART_ENG_VENDOR,          896*1024,  12*1024,    UFS_PART_3},/* eng_vendor         12M    p55*/
  {PART_MODEM_PATCH_NV,      908*1024,   4*1024,    UFS_PART_3},/* modem_patch_nv      4M    p56*/
  {PART_RESERVED4,           912*1024,   8*1024,    UFS_PART_3},/* reserved4           8M    p57*/
  {PART_RECOVERY_VBMETA,     920*1024,   2*1024,    UFS_PART_3},/* recovery_vbmeta     2M    p58*/
  {PART_ERECOVERY_VBMETA,    922*1024,   2*1024,    UFS_PART_3},/* erecovery_vbmeta    2M    p59*/
  {PART_VBMETA,              924*1024,   4*1024,    UFS_PART_3},/* PART_VBMETA         4M    p60*/
  {PART_MODEMNVM_UPDATE,     928*1024,  16*1024,    UFS_PART_3},/* modemnvm_update    16M    p61*/
  {PART_MODEMNVM_CUST,       944*1024,  16*1024,    UFS_PART_3},/* modemnvm_cust      16M    p62*/
  {PART_PATCH,               960*1024,  32*1024,    UFS_PART_3},/* patch              32M    p63*/
  {PART_CACHE,               992*1024, 104*1024,    UFS_PART_3},/* cache             104M    p64*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,              1096*1024,        136*1024,    UFS_PART_3},/* preas           136M    p65*/
  {PART_PREAVS,             1232*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              1264*1024,       7020*1024,    UFS_PART_3},/* super          7020M    p67*/
  {PART_VERSION,            8284*1024,        576*1024,    UFS_PART_3},/* version         576M    p68*/
  {PART_PRELOAD,            8860*1024,        900*1024,    UFS_PART_3},/* preload         900M    p69*/
  {PART_HIBENCH_IMG,        9760*1024,        128*1024,    UFS_PART_3},/*hibench_img      128M    p70*/
  {PART_HIBENCH_DATA,       9888*1024,        512*1024,    UFS_PART_3},/* hibench_data    512M    p71*/
  {PART_FLASH_AGEING,      10400*1024,        512*1024,    UFS_PART_3},/* flash_ageing    512M    p72*/
  {PART_HIBENCH_LOG,       10912*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG      32M    p73*/
  {PART_USERDATA,          10944*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata          4G    p74*/
#else  /* factory */
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,              1096*1024,        312*1024,    UFS_PART_3},/* preas           312M    p65*/
  {PART_PREAVS,             1408*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              1440*1024,       5064*1024,    UFS_PART_3},/* super          5064M    p67*/
  {PART_VERSION,            6504*1024,         32*1024,    UFS_PART_3},/* version          32M    p68*/
  {PART_PRELOAD,            6536*1024,        928*1024,    UFS_PART_3},/* preload         928M    p69*/
  {PART_USERDATA,           7464*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G      p70*/
#elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,              1096*1024,       1312*1024,    UFS_PART_3},/* preas          1312M    p65*/
  {PART_PREAVS,             2408*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              2440*1024,       4344*1024,    UFS_PART_3},/* super          4344M    p67*/
  {PART_VERSION,            6784*1024,         32*1024,    UFS_PART_3},/* version          32M    p68*/
  {PART_PRELOAD,            6816*1024,        928*1024,    UFS_PART_3},/* preload         928M    p69*/
  {PART_USERDATA,           7744*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G      p70*/
#else
  {PART_PREAS,              1096*1024,        136*1024,    UFS_PART_3},/* preas           136M    p65*/
  {PART_PREAVS,             1232*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              1264*1024,       7020*1024,    UFS_PART_3},/* super          7020M    p67*/
  {PART_VERSION,            8284*1024,        576*1024,    UFS_PART_3},/* version         576M    p68*/
  {PART_PRELOAD,            8860*1024,        900*1024,    UFS_PART_3},/* preload         900M    p69*/
  {PART_USERDATA,           9760*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata          4G    p70*/
#endif  /* internal */
#else  /* not erofs, ext4 */
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,              1096*1024,        136*1024,    UFS_PART_3},/* preas           136M    p65*/
  {PART_PREAVS,             1232*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              1264*1024,       4176*1024,    UFS_PART_3},/* super          4176M    p67*/
  {PART_VERSION,            5440*1024,         32*1024,    UFS_PART_3},/* version          32M    p68*/
  {PART_PRELOAD,            5472*1024,        928*1024,    UFS_PART_3},/* preload         928M    p69*/
  {PART_USERDATA,           6400*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G      p70*/
#elif defined CONFIG_MARKET_OVERSEA
  {PART_PREAS,              1096*1024,       1008*1024,    UFS_PART_3},/* preas          1008M    p65*/
  {PART_PREAVS,             2104*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              2136*1024,       3536*1024,    UFS_PART_3},/* super          3536M    p67*/
  {PART_VERSION,            5672*1024,         32*1024,    UFS_PART_3},/* version          32M    p68*/
  {PART_PRELOAD,            5704*1024,        928*1024,    UFS_PART_3},/* preload         928M    p69*/
  {PART_USERDATA,           6632*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata        4G      p70*/
#else
  {PART_PREAS,              1096*1024,        136*1024,    UFS_PART_3},/* preas           136M    p65*/
  {PART_PREAVS,             1232*1024,         32*1024,    UFS_PART_3},/* preavs           32M    p66*/
  {PART_SUPER,              1264*1024,       7020*1024,    UFS_PART_3},/* super          7020M    p67*/
  {PART_VERSION,            8284*1024,        576*1024,    UFS_PART_3},/* version         576M    p68*/
  {PART_PRELOAD,            8860*1024,        900*1024,    UFS_PART_3},/* preload         900M    p69*/
  {PART_USERDATA,           9760*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata          4G    p70*/
#endif  /* internal */
#endif  /* not erofs, ext4 */
#endif  /* factory */
  {"0", 0, 0, 0},
};
#endif

#endif
