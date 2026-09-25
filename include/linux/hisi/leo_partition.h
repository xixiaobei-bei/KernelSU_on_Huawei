#ifndef _LEO_PARTITION_H_
#define _LEO_PARTITION_H_

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
  {PART_IVP,                       497*1024,          2*1024,    EMMC_USER_PART},/* ivp                2M    p31*/
  {PART_HDCP,                      499*1024,          1*1024,    EMMC_USER_PART},/* PART_HDCP          1M    p32*/
  {PART_HHEE,                      500*1024,          4*1024,    EMMC_USER_PART},/* hhee               4M    p33*/
  {PART_FASTBOOT,                  504*1024,         12*1024,    EMMC_USER_PART},/* fastboot          12M    p34*/
  {PART_VECTOR,                    516*1024,          4*1024,    EMMC_USER_PART},/* vector             4M    p35*/
  {PART_ISP_BOOT,                  520*1024,          2*1024,    EMMC_USER_PART},/* isp_boot           2M    p36*/
  {PART_ISP_FIRMWARE,              522*1024,         14*1024,    EMMC_USER_PART},/* isp_firmware      14M    p37*/
  {PART_FW_HIFI,                   536*1024,         12*1024,    EMMC_USER_PART},/* hifi              12M    p38*/
  {PART_TEEOS,                     548*1024,          8*1024,    EMMC_USER_PART},/* teeos              8M    p39*/
  {PART_SENSORHUB,                 556*1024,         16*1024,    EMMC_USER_PART},/* sensorhub         16M    p40*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         572*1024,         12*1024,    EMMC_USER_PART},/* erecovery_ramdisk 12M    p41*/
  {PART_ERECOVERY_VENDOR,          584*1024,          8*1024,    EMMC_USER_PART},/* erecovery_vendor   8M    p42*/
  {PART_BOOT,                      592*1024,         65*1024,    EMMC_USER_PART},/* boot              65M    p43*/
  {PART_RECOVERY,                  657*1024,         85*1024,    EMMC_USER_PART},/* recovery          85M    p44*/
  {PART_ERECOVERY,                 742*1024,         12*1024,    EMMC_USER_PART},/* erecovery         12M    p45*/
  {PART_RESERVED,                  754*1024,         81*1024,    EMMC_USER_PART},/* reserved          81M    p46*/
#else
  {PART_ERECOVERY_RAMDISK,         572*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p41*/
  {PART_ERECOVERY_VENDOR,          604*1024,         24*1024,    EMMC_USER_PART},/* erecovery_vendor  24M    p42*/
  {PART_BOOT,                      628*1024,         30*1024,    EMMC_USER_PART},/* boot              25M    p43*/
  {PART_RECOVERY,                  658*1024,         45*1024,    EMMC_USER_PART},/* recovery          45M    p44*/
  {PART_ERECOVERY,                 703*1024,         45*1024,    EMMC_USER_PART},/* erecovery         45M    p45*/
  {PART_RESERVED,                  748*1024,         87*1024,    EMMC_USER_PART},/* reserved          87M    p46*/
#endif
  {PART_KPATCH,                    835*1024,          4*1024,    EMMC_USER_PART},/* kpatch             4M    p47*/
  {PART_RECOVERY_RAMDISK,          839*1024,         32*1024,    EMMC_USER_PART},/* recovery_ramdisk  32M    p48*/
  {PART_RECOVERY_VENDOR,           871*1024,         24*1024,    EMMC_USER_PART},/* recovery_vendor   24M    p49*/
  {PART_ENG_SYSTEM,                895*1024,         12*1024,    EMMC_USER_PART},/* eng_system        12M    p50*/
  {PART_ENG_VENDOR,                907*1024,         20*1024,    EMMC_USER_PART},/* eng_vendor        20M    p51*/
  {PART_FW_DTB,                    927*1024,          2*1024,    EMMC_USER_PART},/* fw_dtb             2M    p52*/
  {PART_DTBO,                      929*1024,         20*1024,    EMMC_USER_PART},/* dtoimage          20M    p53*/
  {PART_TRUSTFIRMWARE,             949*1024,          2*1024,    EMMC_USER_PART},/* trustfirmware      2M    p54*/
  {PART_MODEM_FW,                  951*1024,        134*1024,    EMMC_USER_PART},/* modem_fw         134M    p55*/
  {PART_MODEM_PATCH_NV,           1085*1024,         16*1024,    EMMC_USER_PART},/* modem_patch_nv    16M    p56*/
  {PART_MODEM_DRIVER,             1101*1024,         20*1024,    EMMC_USER_PART},/* modem_driver      20M    p57*/
  {PART_MODEMNVM_UPDATE,          1121*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_update   16M    p58*/
  {PART_MODEMNVM_CUST,            1137*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_cust     16M    p59*/
  {PART_RAMDISK,                  1153*1024,          2*1024,    EMMC_USER_PART},/* ramdisk           32M    p60*/
  {PART_VBMETA_SYSTEM,            1155*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_system      1M    p61*/
  {PART_VBMETA_VENDOR,            1156*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_vendor      1M    p62*/
  {PART_VBMETA_ODM,               1157*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_odm         1M    p63*/
  {PART_VBMETA_CUST,              1158*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_cust        1M    p64*/
  {PART_VBMETA_HW_PRODUCT,        1159*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_hw_product  1M    p65*/
  {PART_RECOVERY_VBMETA,          1160*1024,          2*1024,    EMMC_USER_PART},/* recovery_vbmeta    2M    p66*/
  {PART_ERECOVERY_VBMETA,         1162*1024,          2*1024,    EMMC_USER_PART},/* erecovery_vbmeta   2M    p67*/
  {PART_VBMETA,                   1164*1024,          4*1024,    EMMC_USER_PART},/* vbmeta             4M    p68*/
  {PART_PATCH,                    1168*1024,         32*1024,    EMMC_USER_PART},/* patch             32M    p69*/
#ifdef CONFIG_FACTORY_MODE
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p71*/
  {PART_PREAVS,                   2496*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    2528*1024,       8240*1024,    EMMC_USER_PART},/* super           8240M    p73*/
  {PART_VERSION,                 10768*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                 11344*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_HIBENCH_IMG,             12488*1024,        128*1024,    EMMC_USER_PART},/* hibench_img      128M    p76*/
  {PART_HIBENCH_DATA,            12616*1024,        512*1024,    EMMC_USER_PART},/* hibench_data     512M    p77*/
  {PART_FLASH_AGEING,            13128*1024,        512*1024,    EMMC_USER_PART},/* FLASH_AGEING     512M    p78*/
  {PART_HIBENCH_LOG,             13640*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG       32M    p79*/
  {PART_HIBENCH_LPM3,            13672*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LPM3      32M    p80*/
  {PART_USERDATA,                13704*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p81*/
#else
  #ifdef CONFIG_USE_EROFS
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,        360*1024,    EMMC_USER_PART},/* preas            360M    p71*/
  {PART_PREAVS,                   1576*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    1608*1024,       5992*1024,    EMMC_USER_PART},/* super           5992M    p73*/
  {PART_VERSION,                  7600*1024,         32*1024,    EMMC_USER_PART},/* version           32M    p74*/
  {PART_PRELOAD,                  7632*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_USERDATA,                 8776*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p76*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,       1568*1024,    EMMC_USER_PART},/* preas           1568M    p71*/
  {PART_PREAVS,                   2784*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    2816*1024,       5312*1024,    EMMC_USER_PART},/* super           5312M    p73*/
  {PART_VERSION,                  8128*1024,         32*1024,    EMMC_USER_PART},/* version           32M    p74*/
  {PART_PRELOAD,                  8160*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_USERDATA,                 9304*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p76*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,        296*1024,    EMMC_USER_PART},/* preas            296M    p71*/
  {PART_PREAVS,                   1512*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    1544*1024,       6456*1024,    EMMC_USER_PART},/* super           6456M    p73*/
  {PART_VERSION,                  8000*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                  8576*1024,       1504*1024,    EMMC_USER_PART},/* preload         1504M    p75*/
  {PART_USERDATA,                10080*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p76*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,       1024*1024,    EMMC_USER_PART},/* preas            976M    p71*/
  {PART_PREAVS,                   2240*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    2272*1024,       6432*1024,    EMMC_USER_PART},/* super           6432M    p73*/
  {PART_VERSION,                  8704*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                  9280*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_USERDATA,                10424*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p76*/
  #else
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p71*/
  {PART_PREAVS,                   2496*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    2528*1024,       8240*1024,    EMMC_USER_PART},/* super           8240M    p73*/
  {PART_VERSION,                 10768*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                 11344*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_USERDATA,                12488*1024, (4UL)*1024*1024,   EMMC_USER_PART},/* userdata           4G    p76*/
  #endif
#else /* do not use erofs */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SUPER,                    1200*1024,       9568*1024,    EMMC_USER_PART},/* super           9568M    p70*/
  {PART_VERSION,                 10768*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p71*/
  {PART_PRELOAD,                 11344*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p72*/
  {PART_USERDATA,                12488*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p73*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SUPER,                    1200*1024,      10696*1024,    EMMC_USER_PART},/* super          10696M    p70*/
  {PART_VERSION,                 11896*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p71*/
  {PART_PRELOAD,                 12472*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p72*/
  {PART_USERDATA,                13616*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p73*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p71*/
  {PART_PREAVS,                   1584*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    1616*1024,       8416*1024,    EMMC_USER_PART},/* super           8416M    p73*/
  {PART_VERSION,                 10032*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                 10608*1024,       1504*1024,    EMMC_USER_PART},/* preload         1504M    p75*/
  {PART_USERDATA,                12112*1024, (4UL)*1024*1024,   EMMC_USER_PART},/* userdata           4G    p76*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p71*/
  {PART_PREAVS,                   2496*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    2528*1024,       8632*1024,    EMMC_USER_PART},/* super           8632M    p73*/
  {PART_VERSION,                 11160*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                 11736*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_USERDATA,                12880*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p76*/
  #else
  {PART_METADATA,                 1200*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p70*/
  {PART_PREAS,                    1216*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p71*/
  {PART_PREAVS,                   2496*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p72*/
  {PART_SUPER,                    2528*1024,       8240*1024,    EMMC_USER_PART},/* super           8240M    p73*/
  {PART_VERSION,                 10768*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p74*/
  {PART_PRELOAD,                 11344*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p75*/
  {PART_USERDATA,                12488*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p76*/
  #endif
#endif
#endif
  {"0", 0, 0, 0},
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
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
  {PART_IVP,                       489*1024,          2*1024,    UFS_PART_3},/* ivp                2M    p27*/
  {PART_HDCP,                      491*1024,          1*1024,    UFS_PART_3},/* PART_HDCP          1M    p28*/
  {PART_HHEE,                      492*1024,          4*1024,    UFS_PART_3},/* hhee               4M    p29*/
  {PART_FASTBOOT,                  496*1024,         12*1024,    UFS_PART_3},/* fastboot          12M    p30*/
  {PART_VECTOR,                    508*1024,          4*1024,    UFS_PART_3},/* vector             4M    p31*/
  {PART_ISP_BOOT,                  512*1024,          2*1024,    UFS_PART_3},/* isp_boot           2M    p32*/
  {PART_ISP_FIRMWARE,              514*1024,         14*1024,    UFS_PART_3},/* isp_firmware      14M    p33*/
  {PART_FW_HIFI,                   528*1024,         12*1024,    UFS_PART_3},/* hifi              12M    p34*/
  {PART_TEEOS,                     540*1024,          8*1024,    UFS_PART_3},/* teeos              8M    p35*/
  {PART_SENSORHUB,                 548*1024,         16*1024,    UFS_PART_3},/* sensorhub         16M    p36*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         564*1024,         12*1024,    UFS_PART_3},/* erecovery_ramdisk 12M    p37*/
  {PART_ERECOVERY_VENDOR,          576*1024,          8*1024,    UFS_PART_3},/* erecovery_vendor   8M    p38*/
  {PART_BOOT,                      584*1024,         65*1024,    UFS_PART_3},/* boot              65M    p39*/
  {PART_RECOVERY,                  649*1024,         85*1024,    UFS_PART_3},/* recovery          85M    p40*/
  {PART_ERECOVERY,                 734*1024,         12*1024,    UFS_PART_3},/* erecovery         12M    p41*/
  {PART_RESERVED,                  746*1024,         81*1024,    UFS_PART_3},/* reserved          81M    p42*/
#else
  {PART_ERECOVERY_RAMDISK,         564*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p37*/
  {PART_ERECOVERY_VENDOR,          596*1024,         24*1024,    UFS_PART_3},/* erecovery_vendor  24M    p38*/
  {PART_BOOT,                      620*1024,         30*1024,    UFS_PART_3},/* boot              30M    p39*/
  {PART_RECOVERY,                  650*1024,         45*1024,    UFS_PART_3},/* recovery          45M    p40*/
  {PART_ERECOVERY,                 695*1024,         45*1024,    UFS_PART_3},/* erecovery         45M    p41*/
  {PART_RESERVED,                  740*1024,         87*1024,    UFS_PART_3},/* reserved          87M    p42*/
#endif
  {PART_KPATCH,                    827*1024,          4*1024,    UFS_PART_3},/* kpatch             4M    p43*/
  {PART_RECOVERY_RAMDISK,          831*1024,         32*1024,    UFS_PART_3},/* recovery_ramdisk  32M    p44*/
  {PART_RECOVERY_VENDOR,           863*1024,         24*1024,    UFS_PART_3},/* recovery_vendor   24M    p45*/
  {PART_ENG_SYSTEM,                887*1024,         12*1024,    UFS_PART_3},/* eng_system        12M    p46*/
  {PART_ENG_VENDOR,                899*1024,         20*1024,    UFS_PART_3},/* eng_vendor        20M    p47*/
  {PART_FW_DTB,                    919*1024,          2*1024,    UFS_PART_3},/* fw_dtb             2M    p48*/
  {PART_DTBO,                      921*1024,         20*1024,    UFS_PART_3},/* dtoimage          20M    p49*/
  {PART_TRUSTFIRMWARE,             941*1024,          2*1024,    UFS_PART_3},/* trustfirmware      2M    p50*/
  {PART_MODEM_FW,                  943*1024,        134*1024,    UFS_PART_3},/* modem_fw         134M    p51*/
  {PART_MODEM_PATCH_NV,           1077*1024,         16*1024,    UFS_PART_3},/* modem_patch_nv    16M    p52*/
  {PART_MODEM_DRIVER,             1093*1024,         20*1024,    UFS_PART_3},/* modem_driver      20M    p53*/
  {PART_MODEMNVM_UPDATE,          1113*1024,         16*1024,    UFS_PART_3},/* modemnvm_update   16M    p54*/
  {PART_MODEMNVM_CUST,            1129*1024,         16*1024,    UFS_PART_3},/* modemnvm_cust     16M    p55*/
  {PART_RAMDISK,                  1145*1024,          2*1024,    UFS_PART_3},/* ramdisk           32M    p56*/
  {PART_VBMETA_SYSTEM,            1147*1024,          1*1024,    UFS_PART_3},/* vbmeta_system      1M    p57*/
  {PART_VBMETA_VENDOR,            1148*1024,          1*1024,    UFS_PART_3},/* vbmeta_vendor      1M    p58*/
  {PART_VBMETA_ODM,               1149*1024,          1*1024,    UFS_PART_3},/* vbmeta_odm         1M    p59*/
  {PART_VBMETA_CUST,              1150*1024,          1*1024,    UFS_PART_3},/* vbmeta_cust        1M    p60*/
  {PART_VBMETA_HW_PRODUCT,        1151*1024,          1*1024,    UFS_PART_3},/* vbmeta_hw_product  1M    p61*/
  {PART_RECOVERY_VBMETA,          1152*1024,          2*1024,    UFS_PART_3},/* recovery_vbmeta    2M    p62*/
  {PART_ERECOVERY_VBMETA,         1154*1024,          2*1024,    UFS_PART_3},/* erecovery_vbmeta   2M    p63*/
  {PART_VBMETA,                   1156*1024,          4*1024,    UFS_PART_3},/* vbmeta             4M    p64*/
  {PART_PATCH,                    1160*1024,         32*1024,    UFS_PART_3},/* patch             32M    p65*/
#ifdef CONFIG_FACTORY_MODE
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,       1280*1024,    UFS_PART_3},/* preas            928M    p67*/
  {PART_PREAVS,                   2488*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    2520*1024,       8240*1024,    UFS_PART_3},/* super           9568M    p69*/
  {PART_VERSION,                 10760*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                 11336*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_HIBENCH_IMG,             12480*1024,        128*1024,    UFS_PART_3},/* hibench_img      128M    p72*/
  {PART_HIBENCH_DATA,            12608*1024,        512*1024,    UFS_PART_3},/* hibench_data     512M    p73*/
  {PART_FLASH_AGEING,            13120*1024,        512*1024,    UFS_PART_3},/* FLASH_AGEING     512M    p74*/
  {PART_HIBENCH_LOG,             13632*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG       32M    p75*/
  {PART_HIBENCH_LPM3,            13664*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LPM3      32M    p76*/
  {PART_USERDATA,                13696*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p77*/
#else
#ifdef CONFIG_USE_EROFS
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,        360*1024,    UFS_PART_3},/* preas            360M    p67*/
  {PART_PREAVS,                   1568*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    1600*1024,       5992*1024,    UFS_PART_3},/* super           5992M    p69*/
  {PART_VERSION,                  7592*1024,         32*1024,    UFS_PART_3},/* version           32M    p70*/
  {PART_PRELOAD,                  7624*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_USERDATA,                 8768*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,       1568*1024,    UFS_PART_3},/* preas           1568M    p67*/
  {PART_PREAVS,                   2776*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    2808*1024,       5312*1024,    UFS_PART_3},/* super           5312M    p69*/
  {PART_VERSION,                  8120*1024,         32*1024,    UFS_PART_3},/* version           32M    p70*/
  {PART_PRELOAD,                  8152*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_USERDATA,                 9296*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,        296*1024,    UFS_PART_3},/* preas            296M    p67*/
  {PART_PREAVS,                   1504*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    1536*1024,       6456*1024,    UFS_PART_3},/* super           6456M    p69*/
  {PART_VERSION,                  7992*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                  8568*1024,       1504*1024,    UFS_PART_3},/* preload         1504M    p71*/
  {PART_USERDATA,                10072*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,       1024*1024,    UFS_PART_3},/* preas           1024M    p67*/
  {PART_PREAVS,                   2232*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    2264*1024,       6432*1024,    UFS_PART_3},/* super           6432M    p69*/
  {PART_VERSION,                  8696*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                  9272*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_USERDATA,                10416*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #else
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,       1280*1024,    UFS_PART_3},/* preas            928M    p67*/
  {PART_PREAVS,                   2488*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    2520*1024,       8240*1024,    UFS_PART_3},/* super           8240M    p69*/
  {PART_VERSION,                 10760*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                 11336*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_USERDATA,                12480*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #endif
#else /* do not use erofs */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SUPER,                    1192*1024,       9568*1024,    UFS_PART_3},/* super           9568M    p66*/
  {PART_VERSION,                 10760*1024,        576*1024,    UFS_PART_3},/* version          576M    p67*/
  {PART_PRELOAD,                 11336*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p68*/
  {PART_USERDATA,                12480*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p69*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SUPER,                    1192*1024,      10696*1024,    UFS_PART_3},/* super          10696M    p66*/
  {PART_VERSION,                 11888*1024,        576*1024,    UFS_PART_3},/* version          576M    p67*/
  {PART_PRELOAD,                 12464*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p68*/
  {PART_USERDATA,                13608*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p69*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,        368*1024,    UFS_PART_3},/* preas            368M    p67*/
  {PART_PREAVS,                   1576*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    1608*1024,       8416*1024,    UFS_PART_3},/* super           8416M    p69*/
  {PART_VERSION,                 10024*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                 10600*1024,       1504*1024,    UFS_PART_3},/* preload         1504M    p71*/
  {PART_USERDATA,                12104*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,       1280*1024,    UFS_PART_3},/* preas           1280M    p67*/
  {PART_PREAVS,                   2488*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    2520*1024,       8632*1024,    UFS_PART_3},/* super           8632M    p69*/
  {PART_VERSION,                 11152*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                 11728*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_USERDATA,                12872*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #else
  {PART_METADATA,                 1192*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p66*/
  {PART_PREAS,                    1208*1024,       1280*1024,    UFS_PART_3},/* preas            928M    p67*/
  {PART_PREAVS,                   2488*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p68*/
  {PART_SUPER,                    2520*1024,       8240*1024,    UFS_PART_3},/* super           8240M    p69*/
  {PART_VERSION,                 10760*1024,        576*1024,    UFS_PART_3},/* version          576M    p70*/
  {PART_PRELOAD,                 11336*1024,       1144*1024,    UFS_PART_3},/* preload         1144M    p71*/
  {PART_USERDATA,                12480*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p72*/
  #endif
#endif
#endif
  {PART_PTABLE_LU4,                       0,             512,    UFS_PART_4},/* ptable_lu4        512K   p0*/
  {PART_RESERVED12,                     512,            1536,    UFS_PART_4},/* reserved12       1536K   p1*/
  {PART_USERDATA2,                     2048, (4UL)*1024*1024,    UFS_PART_4},/* userdata2           4G   p2*/
  {"0", 0, 0, 0},
};
#endif

#endif
