#ifndef _TAURUS_CS2_PARTITION_H_
#define _TAURUS_CS2_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,                   0,                 2*1024,    EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,                 0,                 2*1024,    EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                    0,                    512,    EMMC_USER_PART},/* ptable           512K    */
  {PART_FRP,                       512,                  512,    EMMC_USER_PART},/* frp              512K    p1*/
  {PART_PERSIST,                   1024,              6*1024,    EMMC_USER_PART},/* persist            6M    p2*/
  {PART_RESERVED1,                 7*1024,              1024,    EMMC_USER_PART},/* reserved1       1024K    p3*/
  {PART_RESERVED6,                 8*1024,               512,    EMMC_USER_PART},/* reserved6        512K    p4*/
  {PART_VRL,                       8704,                 512,    EMMC_USER_PART},/* vrl              512K    p5*/
  {PART_VRL_BACKUP,                9*1024,               512,    EMMC_USER_PART},/* vrl backup       512K    p6*/
  {PART_MODEM_SECURE,              9728,                8704,    EMMC_USER_PART},/* modem_secure    8704K    p7*/
  {PART_NVME,                      18*1024,           5*1024,    EMMC_USER_PART},/* nvme               5M    p8*/
  {PART_CTF,                       23*1024,           1*1024,    EMMC_USER_PART},/* ctf                1M    p9*/
  {PART_OEMINFO,                   24*1024,          96*1024,    EMMC_USER_PART},/* oeminfo           96M    p10*/
  {PART_SECURE_STORAGE,            120*1024,         32*1024,    EMMC_USER_PART},/* secure storage    32M    p11*/
  {PART_MODEMNVM_FACTORY,          152*1024,         16*1024,    EMMC_USER_PART},/* modemnvm factory  16M    p12*/
  {PART_MODEMNVM_BACKUP,           168*1024,         16*1024,    EMMC_USER_PART},/* modemnvm backup   16M    p13*/
  {PART_MODEMNVM_IMG,              184*1024,         46*1024,    EMMC_USER_PART},/* modemnvm img      46M    p14*/
  {PART_HISEE_ENCOS,               230*1024,          4*1024,    EMMC_USER_PART},/* hisee_encos        4M    p15*/
  {PART_VERITYKEY,                 234*1024,          1*1024,    EMMC_USER_PART},/* veritykey          1M    p16*/
  {PART_DDR_PARA,                  235*1024,          1*1024,    EMMC_USER_PART},/* DDR_PARA           1M    p17*/
  {PART_LOWPOWER_PARA,             236*1024,          1*1024,    EMMC_USER_PART},/* lowpower_para      1M    p18*/
  {PART_BATT_TP_PARA,              237*1024,          1*1024,    EMMC_USER_PART},/* batt_tp_para       1M    p19*/
  {PART_BL2,                       238*1024,          4*1024,    EMMC_USER_PART},/* bl2                4M    p20*/
  {PART_RESERVED2,                 242*1024,         21*1024,    EMMC_USER_PART},/* reserved2         21M    p21*/
  {PART_SPLASH2,                   263*1024,         80*1024,    EMMC_USER_PART},/* splash2           80M    p22*/
  {PART_BOOTFAIL_INFO,             343*1024,          2*1024,    EMMC_USER_PART},/* bootfail info      2M    p23*/
  {PART_MISC,                      345*1024,          2*1024,    EMMC_USER_PART},/* misc               2M    p24*/
  {PART_DFX,                       347*1024,         16*1024,    EMMC_USER_PART},/* dfx               16M    p25*/
  {PART_RRECORD,                   363*1024,         16*1024,    EMMC_USER_PART},/* rrecord           16M    p26*/
  {PART_CACHE,                     379*1024,        104*1024,    EMMC_USER_PART},/* cache            104M    p27*/
  {PART_FW_LPM3,                   483*1024,          1*1024,    EMMC_USER_PART},/* fw_lpm3            1M    p28*/
  {PART_RESERVED3,                 484*1024,          5*1024,    EMMC_USER_PART},/* reserved3A         5M    p29*/
  {PART_NPU,                       489*1024,          8*1024,    EMMC_USER_PART},/* npu                8M    p30*/
  {PART_HIEPS,                     497*1024,          2*1024,    EMMC_USER_PART},/* hieps              2M    p31*/
  {PART_IVP,                       499*1024,          2*1024,    EMMC_USER_PART},/* ivp                2M    p32*/
  {PART_HDCP,                      501*1024,          1*1024,    EMMC_USER_PART},/* PART_HDCP          1M    p33*/
  {PART_HISEE_IMG,                 502*1024,          4*1024,    EMMC_USER_PART},/* part_hisee_img     4M    p34*/
  {PART_HHEE,                      506*1024,          4*1024,    EMMC_USER_PART},/* hhee               4M    p35*/
  {PART_HISEE_FS,                  510*1024,          8*1024,    EMMC_USER_PART},/* hisee_fs           8M    p36*/
  {PART_FASTBOOT,                  518*1024,         12*1024,    EMMC_USER_PART},/* fastboot          12M    p37*/
  {PART_VECTOR,                    530*1024,          4*1024,    EMMC_USER_PART},/* vector             4M    p38*/
  {PART_ISP_BOOT,                  534*1024,          2*1024,    EMMC_USER_PART},/* isp_boot           2M    p39*/
  {PART_ISP_FIRMWARE,              536*1024,         14*1024,    EMMC_USER_PART},/* isp_firmware      14M    p40*/
  {PART_FW_HIFI,                   550*1024,         12*1024,    EMMC_USER_PART},/* hifi              12M    p41*/
  {PART_TEEOS,                     562*1024,          8*1024,    EMMC_USER_PART},/* teeos              8M    p42*/
  {PART_SENSORHUB,                 570*1024,         16*1024,    EMMC_USER_PART},/* sensorhub         16M    p43*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         586*1024,         12*1024,    EMMC_USER_PART},/* erecovery_ramdisk 12M    p44*/
  {PART_ERECOVERY_VENDOR,          598*1024,          8*1024,    EMMC_USER_PART},/* erecovery_vendor   8M    p45*/
  {PART_BOOT,                      606*1024,         65*1024,    EMMC_USER_PART},/* boot              65M    p46*/
  {PART_RECOVERY,                  671*1024,         85*1024,    EMMC_USER_PART},/* recovery          85M    p47*/
  {PART_ERECOVERY,                 756*1024,         12*1024,    EMMC_USER_PART},/* erecovery         12M    p48*/
  {PART_METADATA,                  768*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p49*/
  {PART_RESERVED,                  784*1024,         47*1024,    EMMC_USER_PART},/* reserved          47M    p50*/
#else
  {PART_ERECOVERY_RAMDISK,         586*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,          618*1024,         16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p45*/
  {PART_BOOT,                      634*1024,         25*1024,    EMMC_USER_PART},/* boot              25M    p46*/
  {PART_RECOVERY,                  659*1024,         45*1024,    EMMC_USER_PART},/* recovery          45M    p47*/
  {PART_ERECOVERY,                 704*1024,         45*1024,    EMMC_USER_PART},/* erecovery         45M    p48*/
  {PART_METADATA,                  749*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p49*/
  {PART_RESERVED,                  765*1024,         66*1024,    EMMC_USER_PART},/* reserved          66M    p50*/
#endif
  {PART_ENG_SYSTEM,                831*1024,         12*1024,    EMMC_USER_PART},/* eng_system        12M    p51*/
  {PART_RAMDISK,                   843*1024,          2*1024,    EMMC_USER_PART},/* ramdisk           32M    p52*/
  {PART_VBMETA_SYSTEM,             845*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_system      1M    p53*/
  {PART_VBMETA_VENDOR,             846*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_vendor      1M    p54*/
  {PART_VBMETA_ODM,                847*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_odm         1M    p55*/
  {PART_VBMETA_CUST,               848*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_cust        1M    p56*/
  {PART_VBMETA_HW_PRODUCT,         849*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_hw_product  1M    p57*/
  {PART_RECOVERY_RAMDISK,          850*1024,         32*1024,    EMMC_USER_PART},/* recovery_ramdisk  32M    p58*/
  {PART_RECOVERY_VENDOR,           882*1024,         16*1024,    EMMC_USER_PART},/* recovery_vendor   16M    p59*/
  {PART_SECURITY_DTB,              898*1024,          2*1024,    EMMC_USER_PART},/* security_dtb   2M        p60*/
  {PART_DTBO,                      900*1024,         20*1024,    EMMC_USER_PART},/* dtoimage          20M    p61*/
  {PART_TRUSTFIRMWARE,             920*1024,          2*1024,    EMMC_USER_PART},/* trustfirmware      2M    p62*/
  {PART_MODEM_FW,                  922*1024,        134*1024,    EMMC_USER_PART},/* modem_fw         134M    p63*/
  {PART_ENG_VENDOR,               1056*1024,         20*1024,    EMMC_USER_PART},/* eng_vendor        20M    p64*/
  {PART_MODEM_PATCH_NV,           1076*1024,         16*1024,    EMMC_USER_PART},/* modem_patch_nv    16M    p65*/
  {PART_MODEM_DRIVER,             1092*1024,         20*1024,    EMMC_USER_PART},/* modem_driver      20M    p66*/
  {PART_RECOVERY_VBMETA,          1112*1024,          2*1024,    EMMC_USER_PART},/* recovery_vbmeta    2M    p67*/
  {PART_ERECOVERY_VBMETA,         1114*1024,          2*1024,    EMMC_USER_PART},/* erecovery_vbmeta   2M    p68*/
  {PART_VBMETA,                   1116*1024,          4*1024,    EMMC_USER_PART},/* PART_VBMETA        4M    p69*/
  {PART_MODEMNVM_UPDATE,          1120*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_update   16M    p70*/
  {PART_MODEMNVM_CUST,            1136*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_cust     16M    p71*/
  {PART_PATCH,                    1152*1024,         32*1024,    EMMC_USER_PART},/* patch             32M    p72*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                    1184*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p73*/
  {PART_PREAVS,                   1552*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    1584*1024,       9168*1024,    EMMC_USER_PART},/* super           9168M    p75*/
  {PART_VERSION,                 10752*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                 11328*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_HIBENCH_IMG,             12472*1024,        128*1024,    EMMC_USER_PART},/* hibench_img      128M    p78*/
  {PART_HIBENCH_DATA,            12600*1024,        512*1024,    EMMC_USER_PART},/* hibench_data     512M    p79*/
  {PART_FLASH_AGEING,            13112*1024,        512*1024,    EMMC_USER_PART},/* FLASH_AGEING     512M    p80*/
  {PART_HIBENCH_LOG,             13624*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG       32M    p81*/
  {PART_HIBENCH_LPM3,            13656*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LPM3      32M    p82*/
  {PART_USERDATA,                13688*1024, (8UL)*1024*1024,    EMMC_USER_PART},/* userdata           8G    p83*/
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1184*1024,        296*1024,    EMMC_USER_PART},/* preas            296M    p73*/
  {PART_PREAVS,                   1480*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    1512*1024,       6792*1024,    EMMC_USER_PART},/* super           6792M    p75*/
  {PART_VERSION,                  8304*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  8880*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                10024*1024, (8UL)*1024*1024,    EMMC_USER_PART},/* userdata           8G    p78*/
#else
  {PART_PREAS,                    1184*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p73*/
  {PART_PREAVS,                   1552*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    1584*1024,       9168*1024,    EMMC_USER_PART},/* super           9168M    p75*/
  {PART_VERSION,                 10752*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                 11328*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                12472*1024, (8UL)*1024*1024,    EMMC_USER_PART},/* userdata           8G    p78*/
#endif
#else
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1184*1024,       1024*1024,    EMMC_USER_PART},/* preas           1024M    p73*/
  {PART_PREAVS,                   2208*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    2240*1024,       6768*1024,    EMMC_USER_PART},/* super           6768M    p75*/
  {PART_VERSION,                  9008*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  9584*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                10728*1024, (8UL)*1024*1024,    EMMC_USER_PART},/* userdata           8G    p78*/
#else
  {PART_PREAS,                    1184*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p73*/
  {PART_PREAVS,                   2464*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    2496*1024,       9384*1024,    EMMC_USER_PART},/* super           9384M    p75*/
  {PART_VERSION,                 11880*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                 12456*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                13600*1024, (8UL)*1024*1024,    EMMC_USER_PART},/* userdata           8G    p78*/
#endif
#endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static struct partition partition_table_ufs[] =
{
  {PART_XLOADER,                   0,                 2*1024,    UFS_PART_0},
  {PART_RESERVED0,                 0,                 2*1024,    UFS_PART_1},
  {PART_PTABLE,                    0,                    512,    UFS_PART_2},/* ptable           512K    */
  {PART_FRP,                       512,                  512,    UFS_PART_2},/* frp              512K    p1*/
  {PART_PERSIST,                   1*1024,            6*1024,    UFS_PART_2},/* persist         6144K    p2*/
  {PART_RESERVED1,                 7*1024,              1024,    UFS_PART_2},/* reserved1       1024K    p3*/
  {PART_PTABLE_LU3,                0,                    512,    UFS_PART_3},/* ptable_lu3       512K    p0*/
  {PART_VRL,                       512,                  512,    UFS_PART_3},/* vrl              512K    p1*/
  {PART_VRL_BACKUP,                1024,                 512,    UFS_PART_3},/* vrl backup       512K    p2*/
  {PART_MODEM_SECURE,              1536,                8704,    UFS_PART_3},/* modem_secure    8704K    p3*/
  {PART_NVME,                      10*1024,           5*1024,    UFS_PART_3},/* nvme               5M    p4*/
  {PART_CTF,                       15*1024,           1*1024,    UFS_PART_3},/* PART_CTF           1M    p5*/
  {PART_OEMINFO,                   16*1024,          96*1024,    UFS_PART_3},/* oeminfo           96M    p6*/
  {PART_SECURE_STORAGE,            112*1024,         32*1024,    UFS_PART_3},/* secure storage    32M    p7*/
  {PART_MODEMNVM_FACTORY,          144*1024,         16*1024,    UFS_PART_3},/* modemnvm factory  16M    p8*/
  {PART_MODEMNVM_BACKUP,           160*1024,         16*1024,    UFS_PART_3},/* modemnvm backup   16M    p9*/
  {PART_MODEMNVM_IMG,              176*1024,         46*1024,    UFS_PART_3},/* modemnvm img      46M    p10*/
  {PART_HISEE_ENCOS,               222*1024,          4*1024,    UFS_PART_3},/* hisee_encos        4M    p11*/
  {PART_VERITYKEY,                 226*1024,          1*1024,    UFS_PART_3},/* reserved2          1M    p12*/
  {PART_DDR_PARA,                  227*1024,          1*1024,    UFS_PART_3},/* DDR_PARA           1M    p13*/
  {PART_LOWPOWER_PARA,             228*1024,          1*1024,    UFS_PART_3},/* lowpower_para      1M    p14*/
  {PART_BATT_TP_PARA,              229*1024,          1*1024,    UFS_PART_3},/* batt_tp_para       1M    p15*/
  {PART_BL2,                       230*1024,          4*1024,    UFS_PART_3},/* bl2                4M    p16*/
  {PART_RESERVED2,                 234*1024,         21*1024,    UFS_PART_3},/* reserved2         21M    p17*/
  {PART_SPLASH2,                   255*1024,         80*1024,    UFS_PART_3},/* splash2           80M    p18*/
  {PART_BOOTFAIL_INFO,             335*1024,          2*1024,    UFS_PART_3},/* bootfail info      2M    p19*/
  {PART_MISC,                      337*1024,          2*1024,    UFS_PART_3},/* misc               2M    p20*/
  {PART_DFX,                       339*1024,         16*1024,    UFS_PART_3},/* dfx               16M    p21*/
  {PART_RRECORD,                   355*1024,         16*1024,    UFS_PART_3},/* rrecord           16M    p22*/
  {PART_CACHE,                     371*1024,        104*1024,    UFS_PART_3},/* cache            104M    p23*/
  {PART_FW_LPM3,                   475*1024,          1*1024,    UFS_PART_3},/* fw_lpm3            1M    p24*/
  {PART_RESERVED3,                 476*1024,          5*1024,    UFS_PART_3},/* reserved3A         5M    p25*/
  {PART_NPU,                       481*1024,          8*1024,    UFS_PART_3},/* npu                8M    p26*/
  {PART_HIEPS,                     489*1024,          2*1024,    UFS_PART_3},/* hieps              2M    p27*/
  {PART_IVP,                       491*1024,          2*1024,    UFS_PART_3},/* ivp                2M    p28*/
  {PART_HDCP,                      493*1024,          1*1024,    UFS_PART_3},/* PART_HDCP          1M    p29*/
  {PART_HISEE_IMG,                 494*1024,          4*1024,    UFS_PART_3},/* part_hisee_img     4M    p30*/
  {PART_HHEE,                      498*1024,          4*1024,    UFS_PART_3},/* hhee               4M    p31*/
  {PART_HISEE_FS,                  502*1024,          8*1024,    UFS_PART_3},/* hisee_fs           8M    p32*/
  {PART_FASTBOOT,                  510*1024,         12*1024,    UFS_PART_3},/* fastboot          12M    p33*/
  {PART_VECTOR,                    522*1024,          4*1024,    UFS_PART_3},/* vector             4M    p34*/
  {PART_ISP_BOOT,                  526*1024,          2*1024,    UFS_PART_3},/* isp_boot           2M    p35*/
  {PART_ISP_FIRMWARE,              528*1024,         14*1024,    UFS_PART_3},/* isp_firmware      14M    p36*/
  {PART_FW_HIFI,                   542*1024,         12*1024,    UFS_PART_3},/* hifi              12M    p37*/
  {PART_TEEOS,                     554*1024,          8*1024,    UFS_PART_3},/* teeos              8M    p38*/
  {PART_SENSORHUB,                 562*1024,         16*1024,    UFS_PART_3},/* sensorhub         16M    p39*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         578*1024,         12*1024,    UFS_PART_3},/* erecovery_ramdisk 12M    p40*/
  {PART_ERECOVERY_VENDOR,          590*1024,          8*1024,    UFS_PART_3},/* erecovery_vendor   8M    p41*/
  {PART_BOOT,                      598*1024,         65*1024,    UFS_PART_3},/* boot              65M    p42*/
  {PART_RECOVERY,                  663*1024,         85*1024,    UFS_PART_3},/* recovery          85M    p43*/
  {PART_ERECOVERY,                 748*1024,         12*1024,    UFS_PART_3},/* erecovery         12M    p44*/
  {PART_METADATA,                  760*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p45*/
  {PART_RESERVED,                  776*1024,         47*1024,    UFS_PART_3},/* reserved          47M    p46*/
#else
  {PART_ERECOVERY_RAMDISK,         578*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p40*/
  {PART_ERECOVERY_VENDOR,          610*1024,         16*1024,    UFS_PART_3},/* erecovery_vendor  16M    p41*/
  {PART_BOOT,                      626*1024,         25*1024,    UFS_PART_3},/* boot              25M    p42*/
  {PART_RECOVERY,                  651*1024,         45*1024,    UFS_PART_3},/* recovery          45M    p43*/
  {PART_ERECOVERY,                 696*1024,         45*1024,    UFS_PART_3},/* erecovery         45M    p44*/
  {PART_METADATA,                  741*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p45*/
  {PART_RESERVED,                  757*1024,         66*1024,    UFS_PART_3},/* reserved          66M    p46*/
#endif
  {PART_ENG_SYSTEM,                823*1024,         12*1024,    UFS_PART_3},/* eng_system        12M    p47*/
  {PART_RAMDISK,                   835*1024,          2*1024,    UFS_PART_3},/* ramdisk           32M    p48*/
  {PART_VBMETA_SYSTEM,             837*1024,          1*1024,    UFS_PART_3},/* vbmeta_system      1M    p49*/
  {PART_VBMETA_VENDOR,             838*1024,          1*1024,    UFS_PART_3},/* vbmeta_vendor      1M    p50*/
  {PART_VBMETA_ODM,                839*1024,          1*1024,    UFS_PART_3},/* vbmeta_odm         1M    p51*/
  {PART_VBMETA_CUST,               840*1024,          1*1024,    UFS_PART_3},/* vbmeta_cust        1M    p52*/
  {PART_VBMETA_HW_PRODUCT,         841*1024,          1*1024,    UFS_PART_3},/* vbmeta_hw_product  1M    p53*/
  {PART_RECOVERY_RAMDISK,          842*1024,         32*1024,    UFS_PART_3},/* recovery_ramdisk  32M    p54*/
  {PART_RECOVERY_VENDOR,           874*1024,         16*1024,    UFS_PART_3},/* recovery_vendor   16M    p55*/
  {PART_SECURITY_DTB,              890*1024,          2*1024,    UFS_PART_3},/* security_dtb   2M        p56*/
  {PART_DTBO,                      892*1024,         20*1024,    UFS_PART_3},/* dtoimage          20M    p57*/
  {PART_TRUSTFIRMWARE,             912*1024,          2*1024,    UFS_PART_3},/* trustfirmware      2M    p58*/
  {PART_MODEM_FW,                  914*1024,        134*1024,    UFS_PART_3},/* modem_fw         134M    p59*/
  {PART_ENG_VENDOR,               1048*1024,         20*1024,    UFS_PART_3},/* eng_vendor        20M    p60*/
  {PART_MODEM_PATCH_NV,           1068*1024,         16*1024,    UFS_PART_3},/* modem_patch_nv    16M    p61*/
  {PART_MODEM_DRIVER,             1084*1024,         20*1024,    UFS_PART_3},/* modem_driver      20M    p62*/
  {PART_RECOVERY_VBMETA,          1104*1024,          2*1024,    UFS_PART_3},/* recovery_vbmeta    2M    p63*/
  {PART_ERECOVERY_VBMETA,         1106*1024,          2*1024,    UFS_PART_3},/* erecovery_vbmeta   2M    p64*/
  {PART_VBMETA,                   1108*1024,          4*1024,    UFS_PART_3},/* PART_VBMETA        4M    p65*/
  {PART_MODEMNVM_UPDATE,          1112*1024,         16*1024,    UFS_PART_3},/* modemnvm_update   16M    p66*/
  {PART_MODEMNVM_CUST,            1128*1024,         16*1024,    UFS_PART_3},/* modemnvm_cust     16M    p67*/
  {PART_PATCH,                    1144*1024,         32*1024,    UFS_PART_3},/* patch             32M    p68*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                    1176*1024,        368*1024,    UFS_PART_3},/* preas            368M    p69*/
  {PART_PREAVS,                   1544*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p70*/
  {PART_SUPER,                    1576*1024,       9168*1024,    UFS_PART_3},/* super           9168M    p71*/
  {PART_VERSION,                 10744*1024,        576*1024,    UFS_PART_3},/* version          576M    p72*/
  {PART_PRELOAD,                 11320*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p73*/
  {PART_HIBENCH_IMG,             12464*1024,        128*1024,    UFS_PART_3},/* hibench_img      128M    p74*/
  {PART_HIBENCH_DATA,            12592*1024,        512*1024,    UFS_PART_3},/* hibench_data     512M    p75*/
  {PART_FLASH_AGEING,            13104*1024,        512*1024,    UFS_PART_3},/* FLASH_AGEING     512M    p76*/
  {PART_HIBENCH_LOG,             13616*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG       32M    p77*/
  {PART_HIBENCH_LPM3,            13648*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LPM3      32M    p78*/
  {PART_USERDATA,                13680*1024, (8UL)*1024*1024,    UFS_PART_3},/* userdata           8G    p79*/
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1176*1024,        296*1024,    UFS_PART_3},/* preas            296M    p69*/
  {PART_PREAVS,                   1472*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p70*/
  {PART_SUPER,                    1504*1024,       6792*1024,    UFS_PART_3},/* super           6792M    p71*/
  {PART_VERSION,                  8296*1024,        576*1024,    UFS_PART_3},/* version          576M    p72*/
  {PART_PRELOAD,                  8872*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p73*/
  {PART_USERDATA,                10016*1024, (8UL)*1024*1024,    UFS_PART_3},/* userdata           8G    p74*/
#else
  {PART_PREAS,                    1176*1024,        368*1024,    UFS_PART_3},/* preas            368M    p69*/
  {PART_PREAVS,                   1544*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p70*/
  {PART_SUPER,                    1576*1024,       9168*1024,    UFS_PART_3},/* super           9168M    p71*/
  {PART_VERSION,                 10744*1024,        576*1024,    UFS_PART_3},/* version          576M    p72*/
  {PART_PRELOAD,                 11320*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p73*/
  {PART_USERDATA,                12464*1024, (8UL)*1024*1024,    UFS_PART_3},/* userdata           8G    p74*/
#endif
#else
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1176*1024,       1024*1024,    UFS_PART_3},/* preas           1024M    p69*/
  {PART_PREAVS,                   2200*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p70*/
  {PART_SUPER,                    2232*1024,       6768*1024,    UFS_PART_3},/* super           6768M    p71*/
  {PART_VERSION,                  9000*1024,        576*1024,    UFS_PART_3},/* version          576M    p72*/
  {PART_PRELOAD,                  9576*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p73*/
  {PART_USERDATA,                10720*1024, (8UL)*1024*1024,    UFS_PART_3},/* userdata           8G    p74*/
#else
  {PART_PREAS,                    1176*1024,       1280*1024,    UFS_PART_3},/* preas           1280M    p69*/
  {PART_PREAVS,                   2456*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p70*/
  {PART_SUPER,                    2488*1024,       9384*1024,    UFS_PART_3},/* super           9384M    p71*/
  {PART_VERSION,                 11872*1024,        576*1024,    UFS_PART_3},/* version          576M    p72*/
  {PART_PRELOAD,                 12448*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p73*/
  {PART_USERDATA,                13592*1024, (8UL)*1024*1024,    UFS_PART_3},/* userdata           8G    p74*/
#endif
#endif
  {PART_PTABLE_LU4,                        0,              512,   UFS_PART_4},/* ptable_lu4        512K    p0*/
  {PART_RESERVED12,                      512,             1536,   UFS_PART_4},/* reserved12       1536K    p1*/
  {PART_USERDATA2,                      2048,  (4UL)*1024*1024,   UFS_PART_4},/* userdata2           4G    p2*/
  {"0", 0, 0, 0},
};
#endif

#endif
