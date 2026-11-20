#ifndef _PISCES_PARTITION_H_
#define _PISCES_PARTITION_H_

#include "partition_macro.h"
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
  {PART_RESERVED2,                 238*1024,         21*1024,    EMMC_USER_PART},/* reserved2         21M    p20*/
  {PART_SPLASH2,                   259*1024,         80*1024,    EMMC_USER_PART},/* splash2           80M    p21*/
  {PART_BOOTFAIL_INFO,             339*1024,          2*1024,    EMMC_USER_PART},/* bootfail info      2M    p22*/
  {PART_MISC,                      341*1024,          2*1024,    EMMC_USER_PART},/* misc               2M    p23*/
  {PART_DFX,                       343*1024,         16*1024,    EMMC_USER_PART},/* dfx               16M    p24*/
  {PART_RRECORD,                   359*1024,         16*1024,    EMMC_USER_PART},/* rrecord           16M    p25*/
  {PART_CACHE,                     375*1024,        104*1024,    EMMC_USER_PART},/* cache            104M    p26*/
  {PART_METADATA,                  479*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p27*/
  {PART_RESERVED3,                 495*1024,          4*1024,    EMMC_USER_PART},/* reserved3A         4M    p28*/
  {PART_TOC,                       499*1024,          1*1024,    EMMC_USER_PART},/* toc                1M    p29*/
  {PART_BL2,                       500*1024,          4*1024,    EMMC_USER_PART},/* bl2                4M    p30*/
  {PART_FW_LPM3,                   504*1024,          1*1024,    EMMC_USER_PART},/* fw_lpm3            1M    p31*/
  {PART_NPU,                       505*1024,          8*1024,    EMMC_USER_PART},/* npu                8M    p32*/
  {PART_HIEPS,                     513*1024,          2*1024,    EMMC_USER_PART},/* hieps              2M    p33*/
  {PART_IVP,                       515*1024,          2*1024,    EMMC_USER_PART},/* ivp                2M    p34*/
  {PART_HDCP,                      517*1024,          1*1024,    EMMC_USER_PART},/* PART_HDCP          1M    p35*/
  {PART_HISEE_IMG,                 518*1024,          4*1024,    EMMC_USER_PART},/* part_hisee_img     4M    p36*/
  {PART_HHEE,                      522*1024,          4*1024,    EMMC_USER_PART},/* hhee               4M    p37*/
  {PART_HISEE_FS,                  526*1024,          8*1024,    EMMC_USER_PART},/* hisee_fs           8M    p38*/
  {PART_FASTBOOT,                  534*1024,         12*1024,    EMMC_USER_PART},/* fastboot          12M    p39*/
  {PART_VECTOR,                    546*1024,          4*1024,    EMMC_USER_PART},/* vector             4M    p40*/
  {PART_ISP_BOOT,                  550*1024,          2*1024,    EMMC_USER_PART},/* isp_boot           2M    p41*/
  {PART_ISP_FIRMWARE,              552*1024,         14*1024,    EMMC_USER_PART},/* isp_firmware      14M    p42*/
  {PART_FW_HIFI,                   566*1024,         12*1024,    EMMC_USER_PART},/* hifi              12M    p43*/
  {PART_TEEOS,                     578*1024,          8*1024,    EMMC_USER_PART},/* teeos              8M    p44*/
  {PART_SENSORHUB,                 586*1024,         16*1024,    EMMC_USER_PART},/* sensorhub         16M    p45*/
  {PART_FW_DTB,                    602*1024,          8*1024,    EMMC_USER_PART},/* fw_dtb             8M    p46*/
  {PART_DTBO,                      610*1024,         14*1024,    EMMC_USER_PART},/* dtoimage          14M    p47*/
  {PART_TRUSTFIRMWARE,             624*1024,          2*1024,    EMMC_USER_PART},/* trustfirmware      2M    p48*/
  {PART_MODEM_FW,                  626*1024,        134*1024,    EMMC_USER_PART},/* modem_fw         134M    p49*/
  {PART_MODEM_PATCH_NV,            760*1024,          4*1024,    EMMC_USER_PART},/* modem_patch_nv     4M    p50*/
  {PART_MODEM_DRIVER,              764*1024,         20*1024,    EMMC_USER_PART},/* modem_driver      20M    p51*/
  {PART_MODEMNVM_UPDATE,           784*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_update   16M    p52*/
  {PART_MODEMNVM_CUST,             800*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_cust     16M    p53*/
  {PART_RECOVERY_VBMETA,           816*1024,          2*1024,    EMMC_USER_PART},/* recovery_vbmeta    2M    p54*/
  {PART_ERECOVERY_VBMETA,          818*1024,          2*1024,    EMMC_USER_PART},/* erecovery_vbmeta   2M    p55*/
  {PART_VBMETA,                    820*1024,          4*1024,    EMMC_USER_PART},/* vbmeta             4M    p56*/
  {PART_HKRR,                      824*1024,         16*1024,    EMMC_USER_PART},/* hkrr              16M    p57*/
  {PART_RESERVED4,                 840*1024,         24*1024,    EMMC_USER_PART},/* reserved4         24M    p58*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         864*1024,         12*1024,    EMMC_USER_PART},/* erecovery_ramdisk 12M    p59*/
  {PART_ERECOVERY_VENDOR,          876*1024,          8*1024,    EMMC_USER_PART},/* erecovery_vendor   8M    p60*/
  {PART_BOOT,                      884*1024,         65*1024,    EMMC_USER_PART},/* boot              65M    p61*/
  {PART_RECOVERY,                  949*1024,         85*1024,    EMMC_USER_PART},/* recovery          85M    p62*/
  {PART_ERECOVERY,                1034*1024,         12*1024,    EMMC_USER_PART},/* erecovery         12M    p63*/
  {PART_RESERVED,                 1046*1024,         63*1024,    EMMC_USER_PART},/* reserved          63M    p64*/
#else
  {PART_ERECOVERY_RAMDISK,         864*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p59*/
  {PART_ERECOVERY_VENDOR,          896*1024,         24*1024,    EMMC_USER_PART},/* erecovery_vendor  24M    p60*/
  {PART_BOOT,                      920*1024,         30*1024,    EMMC_USER_PART},/* boot              30M    p61*/
  {PART_RECOVERY,                  950*1024,         45*1024,    EMMC_USER_PART},/* recovery          45M    p62*/
  {PART_ERECOVERY,                 995*1024,         45*1024,    EMMC_USER_PART},/* erecovery         45M    p63*/
  {PART_RESERVED,                 1040*1024,         69*1024,    EMMC_USER_PART},/* reserved          69M    p64*/
#endif
  {PART_RECOVERY_RAMDISK,         1109*1024,         32*1024,    EMMC_USER_PART},/* recovery_ramdisk  32M    p65*/
  {PART_RECOVERY_VENDOR,          1141*1024,         24*1024,    EMMC_USER_PART},/* recovery_vendor   24M    p66*/
  {PART_ENG_SYSTEM,               1165*1024,         12*1024,    EMMC_USER_PART},/* eng_system        12M    p67*/
  {PART_ENG_VENDOR,               1177*1024,         20*1024,    EMMC_USER_PART},/* eng_vendor        20M    p68*/
  {PART_RAMDISK,                  1197*1024,          2*1024,    EMMC_USER_PART},/* ramdisk            2M    p69*/
  {PART_VBMETA_SYSTEM,            1199*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_system      1M    p70*/
  {PART_VBMETA_VENDOR,            1200*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_vendor      1M    p71*/
  {PART_VBMETA_ODM,               1201*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_odm         1M    p72*/
  {PART_VBMETA_CUST,              1202*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_cust        1M    p73*/
  {PART_VBMETA_HW_PRODUCT,        1203*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_hw_product  1M    p74*/
  {PART_KPATCH,                   1204*1024,          4*1024,    EMMC_USER_PART},/* kpatch             4M    p75*/
  {PART_PATCH,                    1208*1024,        256*1024,    EMMC_USER_PART},/* patch            256M    p76*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                    1464*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p77*/
  {PART_PREAVS,                   1832*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p78*/
  {PART_SUPER,                    1864*1024,      10200*1024,    EMMC_USER_PART},/* super          10200M    p79*/
  {PART_VERSION,                 12064*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p80*/
  {PART_PRELOAD,                 12640*1024,       1400*1024,    EMMC_USER_PART},/* preload         1400M    p81*/
  {PART_HIBENCH_IMG,             14040*1024,        128*1024,    EMMC_USER_PART},/* hibench_img      128M    p82*/
  {PART_HIBENCH_DATA,            14168*1024,        512*1024,    EMMC_USER_PART},/* hibench_data     512M    p83*/
  {PART_FLASH_AGEING,            14680*1024,        512*1024,    EMMC_USER_PART},/* FLASH_AGEING     512M    p84*/
  {PART_HIBENCH_LOG,             15192*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG       32M    p85*/
  {PART_SECFLASH_AGEING,         15224*1024,         32*1024,    EMMC_USER_PART},/* secflash_ageing   32M    p86*/
  {PART_HYPERHOLD,               15256*1024,       2048*1024,    EMMC_USER_PART},/* hyperhold       2048M    p87*/
  {PART_USERDATA,                17304*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p88*/
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1464*1024,        296*1024,    EMMC_USER_PART},/* preas            296M    p77*/
  {PART_PREAVS,                   1760*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p78*/
  {PART_SUPER,                    1792*1024,       6976*1024,    EMMC_USER_PART},/* super           6976M    p79*/
  {PART_VERSION,                  8768*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p80*/
  {PART_PRELOAD,                  9344*1024,       1400*1024,    EMMC_USER_PART},/* preload         1400M    p81*/
  {PART_HYPERHOLD,               10744*1024,       2048*1024,    EMMC_USER_PART},/* hyperhold       2048M    p82*/
  {PART_USERDATA,                12792*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p83*/
#else
  {PART_PREAS,                    1464*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p77*/
  {PART_PREAVS,                   1832*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p78*/
  {PART_SUPER,                    1864*1024,      10200*1024,    EMMC_USER_PART},/* super          10200M    p79*/
  {PART_VERSION,                 12064*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p80*/
  {PART_PRELOAD,                 12640*1024,       1400*1024,    EMMC_USER_PART},/* preload         1400M    p81*/
  {PART_HYPERHOLD,               14040*1024,       2048*1024,    EMMC_USER_PART},/* hyperhold       2048M    p82*/
  {PART_USERDATA,                16088*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p83*/
#endif
#elif defined CONFIG_GCOV_PARTITION_SUPPORT_ENABLE
  {PART_PREAS,                    1464*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p77*/
  {PART_PREAVS,                   2744*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p78*/
  {PART_SUPER,                    2776*1024,      10368*1024,    EMMC_USER_PART},/* super          10368M    p79*/
  {PART_VERSION,                 13144*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p80*/
  {PART_PRELOAD,                 13720*1024,       1400*1024,    EMMC_USER_PART},/* preload         1400M    p81*/
  {PART_HYPERHOLD,               15120*1024,       2048*1024,    EMMC_USER_PART},/* hyperhold       2048M    p82*/
  {PART_USERDATA,                17168*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p83*/
#else
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1464*1024,       1024*1024,    EMMC_USER_PART},/* preas           1024M    p77*/
  {PART_PREAVS,                   2488*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p78*/
  {PART_SUPER,                    2520*1024,       7168*1024,    EMMC_USER_PART},/* super           7168M    p79*/
  {PART_VERSION,                  9688*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p80*/
  {PART_PRELOAD,                 10264*1024,       1400*1024,    EMMC_USER_PART},/* preload         1400M    p81*/
  {PART_HYPERHOLD,               11664*1024,       2048*1024,    EMMC_USER_PART},/* hyperhold       2048M    p82*/
  {PART_USERDATA,                13712*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p83*/
#else
  {PART_PREAS,                    1464*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p77*/
  {PART_PREAVS,                   2744*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p78*/
  {PART_SUPER,                    2776*1024,      10208*1024,    EMMC_USER_PART},/* super          10208M    p79*/
  {PART_VERSION,                 12984*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p80*/
  {PART_PRELOAD,                 13560*1024,       1400*1024,    EMMC_USER_PART},/* preload         1400M    p81*/
  {PART_HYPERHOLD,               14960*1024,       2048*1024,    EMMC_USER_PART},/* hyperhold       2048M    p82*/
  {PART_USERDATA,                17008*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p83*/
#endif
#endif

  {"0", 0, 0, 0},
};

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
  {PART_RESERVED2,                 230*1024,         21*1024,    UFS_PART_3},/* reserved2         21M    p16*/
  {PART_SPLASH2,                   251*1024,         80*1024,    UFS_PART_3},/* splash2           80M    p17*/
  {PART_BOOTFAIL_INFO,             331*1024,          2*1024,    UFS_PART_3},/* bootfail info      2M    p18*/
  {PART_MISC,                      333*1024,          2*1024,    UFS_PART_3},/* misc               2M    p19*/
  {PART_DFX,                       335*1024,         16*1024,    UFS_PART_3},/* dfx               16M    p20*/
  {PART_RRECORD,                   351*1024,         16*1024,    UFS_PART_3},/* rrecord           16M    p21*/
  {PART_CACHE,                     367*1024,        104*1024,    UFS_PART_3},/* cache            104M    p22*/
  {PART_METADATA,                  471*1024,         16*1024,    UFS_PART_3},/* metadata          16M    p23*/
  {PART_FW_UFSDEV,                 487*1024,          4*1024,    UFS_PART_3},/* fw_ufsdev          4M    p24*/
  {PART_TOC,                       491*1024,          1*1024,    UFS_PART_3},/* toc                1M    p25*/
  {PART_BL2,                       492*1024,          4*1024,    UFS_PART_3},/* bl2                4M    p26*/
  {PART_FW_LPM3,                   496*1024,          1*1024,    UFS_PART_3},/* fw_lpm3            1M    p27*/
  {PART_NPU,                       497*1024,          8*1024,    UFS_PART_3},/* npu                8M    p28*/
  {PART_HIEPS,                     505*1024,          2*1024,    UFS_PART_3},/* hieps              2M    p29*/
  {PART_IVP,                       507*1024,          2*1024,    UFS_PART_3},/* ivp                2M    p30*/
  {PART_HDCP,                      509*1024,          1*1024,    UFS_PART_3},/* PART_HDCP          1M    p31*/
  {PART_HISEE_IMG,                 510*1024,          4*1024,    UFS_PART_3},/* part_hisee_img     4M    p32*/
  {PART_HHEE,                      514*1024,          4*1024,    UFS_PART_3},/* hhee               4M    p33*/
  {PART_HISEE_FS,                  518*1024,          8*1024,    UFS_PART_3},/* hisee_fs           8M    p34*/
  {PART_FASTBOOT,                  526*1024,         12*1024,    UFS_PART_3},/* fastboot          12M    p35*/
  {PART_VECTOR,                    538*1024,          4*1024,    UFS_PART_3},/* vector             4M    p36*/
  {PART_ISP_BOOT,                  542*1024,          2*1024,    UFS_PART_3},/* isp_boot           2M    p37*/
  {PART_ISP_FIRMWARE,              544*1024,         14*1024,    UFS_PART_3},/* isp_firmware      14M    p38*/
  {PART_FW_HIFI,                   558*1024,         12*1024,    UFS_PART_3},/* hifi              12M    p39*/
  {PART_TEEOS,                     570*1024,          8*1024,    UFS_PART_3},/* teeos              8M    p40*/
  {PART_SENSORHUB,                 578*1024,         16*1024,    UFS_PART_3},/* sensorhub         16M    p41*/
  {PART_FW_DTB,                    594*1024,          8*1024,    UFS_PART_3},/* fw_dtb             8M    p42*/
  {PART_DTBO,                      602*1024,         14*1024,    UFS_PART_3},/* dtoimage          14M    p43*/
  {PART_TRUSTFIRMWARE,             616*1024,          2*1024,    UFS_PART_3},/* trustfirmware      2M    p44*/
  {PART_MODEM_FW,                  618*1024,        134*1024,    UFS_PART_3},/* modem_fw         134M    p45*/
  {PART_MODEM_PATCH_NV,            752*1024,          4*1024,    UFS_PART_3},/* modem_patch_nv     4M    p46*/
  {PART_MODEM_DRIVER,              756*1024,         20*1024,    UFS_PART_3},/* modem_driver      20M    p47*/
  {PART_MODEMNVM_UPDATE,           776*1024,         16*1024,    UFS_PART_3},/* modemnvm_update   16M    p48*/
  {PART_MODEMNVM_CUST,             792*1024,         16*1024,    UFS_PART_3},/* modemnvm_cust     16M    p49*/
  {PART_RECOVERY_VBMETA,           808*1024,          2*1024,    UFS_PART_3},/* recovery_vbmeta    2M    p50*/
  {PART_ERECOVERY_VBMETA,          810*1024,          2*1024,    UFS_PART_3},/* erecovery_vbmeta   2M    p51*/
  {PART_VBMETA,                    812*1024,          4*1024,    UFS_PART_3},/* vbmeta             4M    p52*/
  {PART_HKRR,                      816*1024,         16*1024,    UFS_PART_3},/* hkrr              16M    p53*/
  {PART_RESERVED4,                 832*1024,         24*1024,    UFS_PART_3},/* reserved4         24M    p54*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         856*1024,         12*1024,    UFS_PART_3},/* erecovery_ramdisk 12M    p55*/
  {PART_ERECOVERY_VENDOR,          868*1024,          8*1024,    UFS_PART_3},/* erecovery_vendor   8M    p56*/
  {PART_BOOT,                      876*1024,         65*1024,    UFS_PART_3},/* boot              65M    p57*/
  {PART_RECOVERY,                  941*1024,         85*1024,    UFS_PART_3},/* recovery          85M    p58*/
  {PART_ERECOVERY,                1026*1024,         12*1024,    UFS_PART_3},/* erecovery         12M    p59*/
  {PART_RESERVED,                 1038*1024,         63*1024,    UFS_PART_3},/* reserved          63M    p60*/
#else
  {PART_ERECOVERY_RAMDISK,         856*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk 32M    p55*/
  {PART_ERECOVERY_VENDOR,          888*1024,         24*1024,    UFS_PART_3},/* erecovery_vendor  24M    p56*/
  {PART_BOOT,                      912*1024,         30*1024,    UFS_PART_3},/* boot              30M    p57*/
  {PART_RECOVERY,                  942*1024,         45*1024,    UFS_PART_3},/* recovery          45M    p58*/
  {PART_ERECOVERY,                 987*1024,         45*1024,    UFS_PART_3},/* erecovery         45M    p59*/
  {PART_RESERVED,                 1032*1024,         69*1024,    UFS_PART_3},/* reserved          69M    p60*/
#endif
  {PART_RECOVERY_RAMDISK,         1101*1024,         32*1024,    UFS_PART_3},/* recovery_ramdisk  32M    p61*/
  {PART_RECOVERY_VENDOR,          1133*1024,         24*1024,    UFS_PART_3},/* recovery_vendor   24M    p62*/
  {PART_ENG_SYSTEM,               1157*1024,         12*1024,    UFS_PART_3},/* eng_system        12M    p63*/
  {PART_ENG_VENDOR,               1169*1024,         20*1024,    UFS_PART_3},/* eng_vendor        20M    p64*/
  {PART_RAMDISK,                  1189*1024,          2*1024,    UFS_PART_3},/* ramdisk            2M    p65*/
  {PART_VBMETA_SYSTEM,            1191*1024,          1*1024,    UFS_PART_3},/* vbmeta_system      1M    p66*/
  {PART_VBMETA_VENDOR,            1192*1024,          1*1024,    UFS_PART_3},/* vbmeta_vendor      1M    p67*/
  {PART_VBMETA_ODM,               1193*1024,          1*1024,    UFS_PART_3},/* vbmeta_odm         1M    p68*/
  {PART_VBMETA_CUST,              1194*1024,          1*1024,    UFS_PART_3},/* vbmeta_cust        1M    p69*/
  {PART_VBMETA_HW_PRODUCT,        1195*1024,          1*1024,    UFS_PART_3},/* vbmeta_hw_product  1M    p70*/
  {PART_KPATCH,                   1196*1024,          4*1024,    UFS_PART_3},/* kpatch             4M    p71*/
  {PART_PATCH,                    1200*1024,        256*1024,    UFS_PART_3},/* patch            256M    p72*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                    1456*1024,        368*1024,    UFS_PART_3},/* preas            368M    p73*/
  {PART_PREAVS,                   1824*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p74*/
  {PART_SUPER,                    1856*1024,      10200*1024,    UFS_PART_3},/* super          10200M    p75*/
  {PART_VERSION,                 12056*1024,        576*1024,    UFS_PART_3},/* version          576M    p76*/
  {PART_PRELOAD,                 12632*1024,       1400*1024,    UFS_PART_3},/* preload         1400M    p77*/
  {PART_HIBENCH_IMG,             14032*1024,        128*1024,    UFS_PART_3},/* hibench_img      128M    p78*/
  {PART_HIBENCH_DATA,            14160*1024,        512*1024,    UFS_PART_3},/* hibench_data     512M    p79*/
  {PART_FLASH_AGEING,            14672*1024,        512*1024,    UFS_PART_3},/* FLASH_AGEING     512M    p80*/
  {PART_HIBENCH_LOG,             15184*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG       32M    p81*/
  {PART_SECFLASH_AGEING,         15216*1024,         32*1024,    UFS_PART_3},/* secflash_ageing   32M    p82*/
  {PART_HYPERHOLD,               15248*1024,       2048*1024,    UFS_PART_3},/* hyperhold       2048M    p83*/
  {PART_USERDATA,                17296*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p84*/
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1456*1024,        296*1024,    UFS_PART_3},/* preas            296M    p73*/
  {PART_PREAVS,                   1752*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p74*/
  {PART_SUPER,                    1784*1024,       6976*1024,    UFS_PART_3},/* super           6976M    p75*/
  {PART_VERSION,                  8760*1024,        576*1024,    UFS_PART_3},/* version          576M    p76*/
  {PART_PRELOAD,                  9336*1024,       1400*1024,    UFS_PART_3},/* preload         1400M    p77*/
  {PART_HYPERHOLD,               10736*1024,       2048*1024,    UFS_PART_3},/* hyperhold       2048M    p78*/
  {PART_USERDATA,                12784*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p79*/
#else
  {PART_PREAS,                    1456*1024,        368*1024,    UFS_PART_3},/* preas            368M    p73*/
  {PART_PREAVS,                   1824*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p74*/
  {PART_SUPER,                    1856*1024,      10200*1024,    UFS_PART_3},/* super          10200M    p75*/
  {PART_VERSION,                 12056*1024,        576*1024,    UFS_PART_3},/* version          576M    p76*/
  {PART_PRELOAD,                 12632*1024,       1400*1024,    UFS_PART_3},/* preload         1400M    p77*/
  {PART_HYPERHOLD,               14032*1024,       2048*1024,    UFS_PART_3},/* hyperhold       2048M    p78*/
  {PART_USERDATA,                16080*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p79*/
#endif
#elif defined CONFIG_GCOV_PARTITION_SUPPORT_ENABLE
  {PART_PREAS,                    1456*1024,       1280*1024,    UFS_PART_3},/* preas           1280M    p73*/
  {PART_PREAVS,                   2736*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p74*/
  {PART_SUPER,                    2768*1024,      10368*1024,    UFS_PART_3},/* super          10368M    p75*/
  {PART_VERSION,                 13136*1024,        576*1024,    UFS_PART_3},/* version          576M    p76*/
  {PART_PRELOAD,                 13712*1024,       1400*1024,    UFS_PART_3},/* preload         1400M    p77*/
  {PART_HYPERHOLD,               15112*1024,       2048*1024,    UFS_PART_3},/* hyperhold       2048M    p78*/
  {PART_USERDATA,                17160*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p79*/
#else
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1456*1024,       1024*1024,    UFS_PART_3},/* preas           1024M    p73*/
  {PART_PREAVS,                   2480*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p74*/
  {PART_SUPER,                    2512*1024,       7168*1024,    UFS_PART_3},/* super           7168M    p75*/
  {PART_VERSION,                  9680*1024,        576*1024,    UFS_PART_3},/* version          576M    p76*/
  {PART_PRELOAD,                 10256*1024,       1400*1024,    UFS_PART_3},/* preload         1400M    p77*/
  {PART_HYPERHOLD,               11656*1024,       2048*1024,    UFS_PART_3},/* hyperhold       2048M    p78*/
  {PART_USERDATA,                13704*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p79*/
#else
  {PART_PREAS,                    1456*1024,       1280*1024,    UFS_PART_3},/* preas           1280M    p73*/
  {PART_PREAVS,                   2736*1024,         32*1024,    UFS_PART_3},/* preavs            32M    p74*/
  {PART_SUPER,                    2768*1024,      10208*1024,    UFS_PART_3},/* super          10208M    p75*/
  {PART_VERSION,                 12976*1024,        576*1024,    UFS_PART_3},/* version          576M    p76*/
  {PART_PRELOAD,                 13552*1024,       1400*1024,    UFS_PART_3},/* preload         1400M    p77*/
  {PART_HYPERHOLD,               14952*1024,       2048*1024,    UFS_PART_3},/* hyperhold       2048M    p78*/
  {PART_USERDATA,                17000*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata           4G    p79*/
#endif
#endif
  {PART_PTABLE_LU4,                       0,             512,    UFS_PART_4},/* ptable_lu4       512K    p0*/
  {PART_RESERVED12,                     512,            1536,    UFS_PART_4},/* reserved12      1536K    p1*/
  {PART_USERDATA2,                     2048, (4UL)*1024*1024,    UFS_PART_4},/* userdata2          4G    p2*/
  {"0", 0, 0, 0},
};

#endif
