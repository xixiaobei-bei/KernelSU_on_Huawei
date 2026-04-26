#ifndef _TAURUS_CDC_PARTITION_H_
#define _TAURUS_CDC_PARTITION_H_

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
  {PART_MODEMNVM_IMG,              184*1024,         34*1024,    EMMC_USER_PART},/* modemnvm img      34M    p14*/
  {PART_HISEE_ENCOS,               218*1024,          4*1024,    EMMC_USER_PART},/* hisee_encos        4M    p15*/
  {PART_VERITYKEY,                 222*1024,          1*1024,    EMMC_USER_PART},/* veritykey          1M    p16*/
  {PART_DDR_PARA,                  223*1024,          1*1024,    EMMC_USER_PART},/* DDR_PARA           1M    p17*/
  {PART_LOWPOWER_PARA,             224*1024,          1*1024,    EMMC_USER_PART},/* lowpower_para      1M    p18*/
  {PART_BATT_TP_PARA,              225*1024,          1*1024,    EMMC_USER_PART},/* batt_tp_para       1M    p19*/
  {PART_BL2,                       226*1024,          4*1024,    EMMC_USER_PART},/* bl2                4M    p20*/
  {PART_RESERVED2,                 230*1024,         21*1024,    EMMC_USER_PART},/* reserved2         21M    p21*/
  {PART_SPLASH2,                   251*1024,         80*1024,    EMMC_USER_PART},/* splash2           80M    p22*/
  {PART_BOOTFAIL_INFO,             331*1024,          2*1024,    EMMC_USER_PART},/* bootfail info      2M    p23*/
  {PART_MISC,                      333*1024,          2*1024,    EMMC_USER_PART},/* misc               2M    p24*/
  {PART_DFX,                       335*1024,         16*1024,    EMMC_USER_PART},/* dfx               16M    p25*/
  {PART_RRECORD,                   351*1024,         16*1024,    EMMC_USER_PART},/* rrecord           16M    p26*/
  {PART_CACHE,                     367*1024,        104*1024,    EMMC_USER_PART},/* cache            104M    p27*/
  {PART_FW_LPM3,                   471*1024,          1*1024,    EMMC_USER_PART},/* fw_lpm3            1M    p28*/
  {PART_RESERVED3,                 472*1024,          5*1024,    EMMC_USER_PART},/* reserved3A         5M    p29*/
  {PART_NPU,                       477*1024,          8*1024,    EMMC_USER_PART},/* npu                8M    p30*/
  {PART_HIEPS,                     485*1024,          2*1024,    EMMC_USER_PART},/* hieps              2M    p31*/
  {PART_IVP,                       487*1024,          2*1024,    EMMC_USER_PART},/* ivp                2M    p32*/
  {PART_HDCP,                      489*1024,          1*1024,    EMMC_USER_PART},/* PART_HDCP          1M    p33*/
  {PART_HISEE_IMG,                 490*1024,          4*1024,    EMMC_USER_PART},/* part_hisee_img     4M    p34*/
  {PART_HHEE,                      494*1024,          4*1024,    EMMC_USER_PART},/* hhee               4M    p35*/
  {PART_HISEE_FS,                  498*1024,          8*1024,    EMMC_USER_PART},/* hisee_fs           8M    p36*/
  {PART_FASTBOOT,                  506*1024,         12*1024,    EMMC_USER_PART},/* fastboot          12M    p37*/
  {PART_VECTOR,                    518*1024,          4*1024,    EMMC_USER_PART},/* vector             4M    p38*/
  {PART_ISP_BOOT,                  522*1024,          2*1024,    EMMC_USER_PART},/* isp_boot           2M    p39*/
  {PART_ISP_FIRMWARE,              524*1024,         14*1024,    EMMC_USER_PART},/* isp_firmware      14M    p40*/
  {PART_FW_HIFI,                   538*1024,         12*1024,    EMMC_USER_PART},/* hifi              12M    p41*/
  {PART_TEEOS,                     550*1024,          8*1024,    EMMC_USER_PART},/* teeos              8M    p42*/
  {PART_SENSORHUB,                 558*1024,         16*1024,    EMMC_USER_PART},/* sensorhub         16M    p43*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK,         574*1024,         12*1024,    EMMC_USER_PART},/* erecovery_ramdisk 12M    p44*/
  {PART_ERECOVERY_VENDOR,          586*1024,          8*1024,    EMMC_USER_PART},/* erecovery_vendor   8M    p45*/
  {PART_BOOT,                      594*1024,         65*1024,    EMMC_USER_PART},/* boot              65M    p46*/
  {PART_RECOVERY,                  659*1024,         85*1024,    EMMC_USER_PART},/* recovery          85M    p47*/
  {PART_ERECOVERY,                 744*1024,         12*1024,    EMMC_USER_PART},/* erecovery         12M    p48*/
  {PART_METADATA,                  756*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p49*/
  {PART_KPATCH,                    772*1024,         29*1024,    EMMC_USER_PART},/* reserved          29M    p50*/
#elif defined CONFIG_FACTORY_MODE
  {PART_ERECOVERY_RAMDISK,         574*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,          606*1024,         24*1024,    EMMC_USER_PART},/* erecovery_vendor  24M    p45*/
  {PART_BOOT,                      630*1024,         25*1024,    EMMC_USER_PART},/* boot              25M    p46*/
  {PART_RECOVERY,                  655*1024,         41*1024,    EMMC_USER_PART},/* recovery          41M    p47*/
  {PART_ERECOVERY,                 696*1024,         41*1024,    EMMC_USER_PART},/* erecovery         41M    p48*/
  {PART_METADATA,                  737*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p49*/
  {PART_KPATCH,                    753*1024,         48*1024,    EMMC_USER_PART},/* reserved          48M    p50*/
#else
  {PART_ERECOVERY_RAMDISK,         574*1024,         32*1024,    EMMC_USER_PART},/* erecovery_ramdisk 32M    p44*/
  {PART_ERECOVERY_VENDOR,          606*1024,         16*1024,    EMMC_USER_PART},/* erecovery_vendor  16M    p45*/
  {PART_BOOT,                      622*1024,         25*1024,    EMMC_USER_PART},/* boot              25M    p46*/
  {PART_RECOVERY,                  647*1024,         45*1024,    EMMC_USER_PART},/* recovery          45M    p47*/
  {PART_ERECOVERY,                 692*1024,         45*1024,    EMMC_USER_PART},/* erecovery         45M    p48*/
  {PART_METADATA,                  737*1024,         16*1024,    EMMC_USER_PART},/* metadata          16M    p49*/
  {PART_KPATCH,                    753*1024,         48*1024,    EMMC_USER_PART},/* reserved          48M    p50*/
#endif
  {PART_ENG_SYSTEM,                801*1024,         12*1024,    EMMC_USER_PART},/* eng_system        12M    p51*/
  {PART_RAMDISK,                   813*1024,          2*1024,    EMMC_USER_PART},/* ramdisk           32M    p52*/
  {PART_VBMETA_SYSTEM,             815*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_system      1M    p53*/
  {PART_VBMETA_VENDOR,             816*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_vendor      1M    p54*/
  {PART_VBMETA_ODM,                817*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_odm         1M    p55*/
  {PART_VBMETA_CUST,               818*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_cust        1M    p56*/
  {PART_VBMETA_HW_PRODUCT,         819*1024,          1*1024,    EMMC_USER_PART},/* vbmeta_hw_product  1M    p57*/
  {PART_RECOVERY_RAMDISK,          820*1024,         32*1024,    EMMC_USER_PART},/* recovery_ramdisk  32M    p58*/
#ifdef CONFIG_FACTORY_MODE
  {PART_RECOVERY_VENDOR,           852*1024,         24*1024,    EMMC_USER_PART},/* recovery_vendor   24M    p59*/
  {PART_SECURITY_DTB,              876*1024,          2*1024,    EMMC_USER_PART},/* security_dtb       2M    p60*/
  {PART_DTBO,                      878*1024,         16*1024,    EMMC_USER_PART},/* dtoimage          16M    p61*/
  {PART_TRUSTFIRMWARE,             894*1024,          2*1024,    EMMC_USER_PART},/* trustfirmware      2M    p62*/
  {PART_MODEM_FW,                  896*1024,         56*1024,    EMMC_USER_PART},/* modem_fw          56M    p63*/
  {PART_ENG_VENDOR,                952*1024,         16*1024,    EMMC_USER_PART},/* eng_vendor        16M    p64*/
#else
  {PART_RECOVERY_VENDOR,           852*1024,         16*1024,    EMMC_USER_PART},/* recovery_vendor   16M    p59*/
  {PART_SECURITY_DTB,              868*1024,          2*1024,    EMMC_USER_PART},/* security_dtb       2M    p60*/
  {PART_DTBO,                      870*1024,         20*1024,    EMMC_USER_PART},/* dtoimage          20M    p61*/
  {PART_TRUSTFIRMWARE,             890*1024,          2*1024,    EMMC_USER_PART},/* trustfirmware      2M    p62*/
  {PART_MODEM_FW,                  892*1024,         56*1024,    EMMC_USER_PART},/* modem_fw          56M    p63*/
  {PART_ENG_VENDOR,                948*1024,         20*1024,    EMMC_USER_PART},/* eng_vendor        20M    p64*/
#endif
  {PART_MODEM_PATCH_NV,            968*1024,          4*1024,    EMMC_USER_PART},/* modem_patch_nv     4M    p65*/
  {PART_MODEM_DRIVER,              972*1024,         20*1024,    EMMC_USER_PART},/* modem_driver      20M    p66*/
  {PART_RECOVERY_VBMETA,           992*1024,          2*1024,    EMMC_USER_PART},/* recovery_vbmeta    2M    p67*/
  {PART_ERECOVERY_VBMETA,          994*1024,          2*1024,    EMMC_USER_PART},/* erecovery_vbmeta   2M    p68*/
  {PART_VBMETA,                    996*1024,          4*1024,    EMMC_USER_PART},/* PART_VBMETA        4M    p69*/
  {PART_MODEMNVM_UPDATE,          1000*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_update   16M    p70*/
  {PART_MODEMNVM_CUST,            1016*1024,         16*1024,    EMMC_USER_PART},/* modemnvm_cust     16M    p71*/
  {PART_PATCH,                    1032*1024,         32*1024,    EMMC_USER_PART},/* patch             32M    p72*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                    1064*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p73*/
  {PART_PREAVS,                   1432*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    1464*1024,       9168*1024,    EMMC_USER_PART},/* super           9168M    p75*/
  {PART_VERSION,                 10632*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                 11208*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_HIBENCH_IMG,             12352*1024,        128*1024,    EMMC_USER_PART},/* hibench_img      128M    p78*/
  {PART_HIBENCH_DATA,            12480*1024,        512*1024,    EMMC_USER_PART},/* hibench_data     512M    p79*/
  {PART_FLASH_AGEING,            12992*1024,        512*1024,    EMMC_USER_PART},/* FLASH_AGEING     512M    p80*/
  {PART_HIBENCH_LOG,             13504*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LOG       32M    p81*/
  {PART_HIBENCH_LPM3,            13536*1024,         32*1024,    EMMC_USER_PART},/* HIBENCH_LPM3      32M    p82*/
  {PART_SECFLASH_AGEING,         13568*1024,         32*1024,    EMMC_USER_PART},/* secflash_ageing   32M    p83*/
  {PART_USERDATA,                13600*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p84*/
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1064*1024,        296*1024,    EMMC_USER_PART},/* preas            296M    p73*/
  {PART_PREAVS,                   1360*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    1392*1024,       6792*1024,    EMMC_USER_PART},/* super           6792M    p75*/
  {PART_VERSION,                  8184*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  8760*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                 9904*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p78*/
#else
  {PART_PREAS,                    1064*1024,        368*1024,    EMMC_USER_PART},/* preas            368M    p73*/
  {PART_PREAVS,                   1432*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    1464*1024,       9168*1024,    EMMC_USER_PART},/* super           9168M    p75*/
  {PART_VERSION,                 10632*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                 11208*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                12352*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p78*/
#endif
#else
#ifdef CONFIG_USE_EROFS
  {PART_PREAS,                    1064*1024,       1024*1024,    EMMC_USER_PART},/* preas           1024M    p73*/
  {PART_PREAVS,                   2088*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    2120*1024,       6768*1024,    EMMC_USER_PART},/* super           6768M    p75*/
  {PART_VERSION,                  8888*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  9464*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                10608*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p78*/
#else
  {PART_PREAS,                    1064*1024,       1280*1024,    EMMC_USER_PART},/* preas           1280M    p73*/
  {PART_PREAVS,                   2344*1024,         32*1024,    EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                    2376*1024,       9384*1024,    EMMC_USER_PART},/* super           9384M    p75*/
  {PART_VERSION,                 11760*1024,        576*1024,    EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                 12336*1024,       1144*1024,    EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                13480*1024, (4UL)*1024*1024,    EMMC_USER_PART},/* userdata           4G    p78*/
#endif
#endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};

static struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,                          0,          2*1024,    UFS_PART_0},
  {PART_XLOADER_B,                          0,          2*1024,    UFS_PART_1},
  {PART_PTABLE,                             0,             512,    UFS_PART_2},/* ptable             512K    */
  {PART_FRP,                              512,             512,    UFS_PART_2},/* frp                512K    p1*/
  {PART_PERSIST,                       1*1024,          6*1024,    UFS_PART_2},/* persist           6144K    p2*/
  {PART_RESERVED1,                     7*1024,            1024,    UFS_PART_2},/* reserved1         1024K    p3*/
  {PART_PTABLE_LU3,                         0,             512,    UFS_PART_3},/* ptable_lu3         512K    p0*/
  {PART_VRL_A,                            512,             512,    UFS_PART_3},/* vrl_a              512K    p1*/
  {PART_VRL_B,                          2*512,             512,    UFS_PART_3},/* vrl_b              512K    p2*/
  {PART_VRL_BACKUP_A,                   3*512,             512,    UFS_PART_3},/* vrl backup_a       512K    p3*/
  {PART_VRL_BACKUP_B,                   4*512,             512,    UFS_PART_3},/* vrl backup_b       512K    p4*/
  {PART_BOOT_CTRL,                      5*512,             512,    UFS_PART_3},/* boot ctrl          512K    p5*/
  {PART_RESERVED2,                     3*1024,          7*1024,    UFS_PART_3},/* reserved2            7M    p6*/
  {PART_NVME,                         10*1024,          5*1024,    UFS_PART_3},/* nvme                 5M    p7*/
  {PART_CTF,                          15*1024,          1*1024,    UFS_PART_3},/* PART_CTF             1M    p8*/
  {PART_OEMINFO,                      16*1024,         96*1024,    UFS_PART_3},/* oeminfo             96M    p9*/
  {PART_SECURE_STORAGE,              112*1024,         32*1024,    UFS_PART_3},/* secure storage      32M    p10*/
  {PART_NPU_A,                       144*1024,          8*1024,    UFS_PART_3},/* npu_a                8M    p11*/
  {PART_NPU_B,                       152*1024,          8*1024,    UFS_PART_3},/* npu_b                8M    p12*/
  {PART_HIEPS_A,                     160*1024,          2*1024,    UFS_PART_3},/* hieps_a              2M    p13*/
  {PART_HIEPS_B,                     162*1024,          2*1024,    UFS_PART_3},/* hieps_b              2M    p14*/
  {PART_IVP_A,                       164*1024,          2*1024,    UFS_PART_3},/* ivp_a                2M    p15*/
  {PART_IVP_B,                       166*1024,          2*1024,    UFS_PART_3},/* ivp_b                2M    p16*/
  {PART_HDCP_A,                      168*1024,          1*1024,    UFS_PART_3},/* PART_HDCP_a          1M    p17*/
  {PART_HDCP_B,                      169*1024,          1*1024,    UFS_PART_3},/* PART_HDCP_b          1M    p18*/
  {PART_HISEE_IMG_A,                 170*1024,          4*1024,    UFS_PART_3},/* part_hisee_img_a     4M    p19*/
  {PART_HISEE_IMG_B,                 174*1024,          4*1024,    UFS_PART_3},/* part_hisee_img_b     4M    p20*/
  {PART_HHEE_A,                      178*1024,          4*1024,    UFS_PART_3},/* hhee_a               4M    p21*/
  {PART_HHEE_B,                      182*1024,          4*1024,    UFS_PART_3},/* hhee_b               4M    p22*/
  {PART_FASTBOOT_A,                  186*1024,         12*1024,    UFS_PART_3},/* fastboot_a          12M    p23*/
  {PART_FASTBOOT_B,                  198*1024,         12*1024,    UFS_PART_3},/* fastboot_b          12M    p24*/
  {PART_HISEE_ENCOS,                 210*1024,          4*1024,    UFS_PART_3},/* hisee_encos          4M    p25*/
  {PART_VERITYKEY,                   214*1024,          1*1024,    UFS_PART_3},/* reserved2            1M    p26*/
  {PART_DDR_PARA,                    215*1024,          1*1024,    UFS_PART_3},/* DDR_PARA             1M    p27*/
  {PART_LOWPOWER_PARA_A,             216*1024,          1*1024,    UFS_PART_3},/* lowpower_para_a      1M    p28*/
  {PART_BATT_TP_PARA,                217*1024,          1*1024,    UFS_PART_3},/* batt_tp_para         1M    p29*/
  {PART_BL2_A,                       218*1024,          4*1024,    UFS_PART_3},/* bl2_a                4M    p30*/
  {PART_BL2_B,                       222*1024,          4*1024,    UFS_PART_3},/* bl2_b                4M    p31*/
  {PART_LOWPOWER_PARA_B,             226*1024,          1*1024,    UFS_PART_3},/* lowpower_para_b      1M    p32*/
  {PART_TEEOS_A,                     227*1024,          8*1024,    UFS_PART_3},/* teeos_a              8M    p33*/
  {PART_TEEOS_B,                     235*1024,          8*1024,    UFS_PART_3},/* teeos_b              8M    p34*/
  {PART_SPLASH2,                     243*1024,         80*1024,    UFS_PART_3},/* splash2             80M    p35*/
  {PART_BOOTFAIL_INFO,               323*1024,          2*1024,    UFS_PART_3},/* bootfail info        2M    p36*/
  {PART_MISC,                        325*1024,          2*1024,    UFS_PART_3},/* misc                 2M    p37*/
  {PART_DFX,                         327*1024,         16*1024,    UFS_PART_3},/* dfx                 16M    p38*/
  {PART_RRECORD,                     343*1024,         16*1024,    UFS_PART_3},/* rrecord             16M    p39*/
  {PART_CACHE,                       359*1024,        104*1024,    UFS_PART_3},/* cache              104M    p40*/
  {PART_FW_LPM3_A,                   463*1024,          1*1024,    UFS_PART_3},/* fw_lpm3_a            1M    p41*/
  {PART_FW_LPM3_B,                   464*1024,          1*1024,    UFS_PART_3},/* fw_lpm3_b            1M    p42*/
  {PART_HISEE_FS_A,                  465*1024,          8*1024,    UFS_PART_3},/* hisee_fs_a           8M    p43*/
  {PART_HISEE_FS_B,                  473*1024,          8*1024,    UFS_PART_3},/* hisee_fs_b           8M    p44*/
  {PART_VECTOR_A,                    481*1024,          4*1024,    UFS_PART_3},/* vector_a             4M    p45*/
  {PART_VECTOR_B,                    485*1024,          4*1024,    UFS_PART_3},/* vector_b             4M    p46*/
  {PART_ISP_BOOT_A,                  489*1024,          2*1024,    UFS_PART_3},/* isp_boot_a           2M    p47*/
  {PART_ISP_BOOT_B,                  491*1024,          2*1024,    UFS_PART_3},/* isp_boot_b           2M    p48*/
  {PART_ISP_FIRMWARE_A,              493*1024,         14*1024,    UFS_PART_3},/* isp_firmware_a      14M    p49*/
  {PART_ISP_FIRMWARE_B,              507*1024,         14*1024,    UFS_PART_3},/* isp_firmware_b      14M    p50*/
  {PART_FW_HIFI_A,                   521*1024,         12*1024,    UFS_PART_3},/* hifi_a              12M    p51*/
  {PART_FW_HIFI_B,                   533*1024,         12*1024,    UFS_PART_3},/* hifi_b              12M    p52*/
  {PART_RESERVED3,                   545*1024,         21*1024,    UFS_PART_3},/* reserved1           21M    p53*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_RAMDISK_A,         566*1024,         12*1024,    UFS_PART_3},/* erecovery_ramdisk_a 12M    p54*/
  {PART_ERECOVERY_VENDOR_A,          578*1024,          8*1024,    UFS_PART_3},/* erecovery_vendor_a   8M    p55*/
  {PART_BOOT_A,                      586*1024,         65*1024,    UFS_PART_3},/* boot_a              65M    p56*/
  {PART_RECOVERY_A,                  651*1024,         85*1024,    UFS_PART_3},/* recovery_a          85M    p57*/
  {PART_ERECOVERY_A,                 736*1024,         12*1024,    UFS_PART_3},/* erecovery_a         12M    p58*/
  {PART_METADATA,                    748*1024,         16*1024,    UFS_PART_3},/* metadata            16M    p59*/
  {PART_KPATCH_A,                    764*1024,         29*1024,    UFS_PART_3},/* kpatch_a            29M    p60*/
  {PART_ERECOVERY_RAMDISK_B,         793*1024,         12*1024,    UFS_PART_3},/* erecovery_ramdisk_b 12M    p61*/
  {PART_ERECOVERY_VENDOR_B,          805*1024,          8*1024,    UFS_PART_3},/* erecovery_vendor_b   8M    p62*/
  {PART_BOOT_B,                      813*1024,         65*1024,    UFS_PART_3},/* boot_b              65M    p63*/
  {PART_RECOVERY_B,                  878*1024,         85*1024,    UFS_PART_3},/* recovery_b          85M    p64*/
  {PART_ERECOVERY_B,                 963*1024,         12*1024,    UFS_PART_3},/* erecovery_b         12M    p65*/
  {PART_KPATCH_B,                    975*1024,         29*1024,    UFS_PART_3},/* kpatch_b            29M    p66*/
#elif defined CONFIG_FACTORY_MODE
  {PART_ERECOVERY_RAMDISK_A,         566*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk_a 32M    p54*/
  {PART_ERECOVERY_VENDOR_A,          598*1024,         24*1024,    UFS_PART_3},/* erecovery_vendor_a  24M    p55*/
  {PART_BOOT_A,                      622*1024,         25*1024,    UFS_PART_3},/* boot_a              25M    p56*/
  {PART_RECOVERY_A,                  647*1024,         41*1024,    UFS_PART_3},/* recovery_a          41M    p57*/
  {PART_ERECOVERY_A,                 688*1024,         41*1024,    UFS_PART_3},/* erecovery_a         41M    p58*/
  {PART_METADATA,                    729*1024,         16*1024,    UFS_PART_3},/* metadata            16M    p59*/
  {PART_KPATCH_A,                    745*1024,         48*1024,    UFS_PART_3},/* kpatch_a            48M    p60*/
  {PART_ERECOVERY_RAMDISK_B,         793*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk_b 32M    p61*/
  {PART_ERECOVERY_VENDOR_B,          825*1024,         24*1024,    UFS_PART_3},/* erecovery_vendor_b  24M    p62*/
  {PART_BOOT_B,                      849*1024,         25*1024,    UFS_PART_3},/* boot_b              25M    p63*/
  {PART_RECOVERY_B,                  874*1024,         41*1024,    UFS_PART_3},/* recovery_b          41M    p64*/
  {PART_ERECOVERY_B,                 915*1024,         41*1024,    UFS_PART_3},/* erecovery_b         41M    p65*/
  {PART_KPATCH_B,                    956*1024,         48*1024,    UFS_PART_3},/* kpatch_b            48M    p66*/
#else
  {PART_ERECOVERY_RAMDISK_A,         566*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk_a 32M    p54*/
  {PART_ERECOVERY_VENDOR_A,          598*1024,         16*1024,    UFS_PART_3},/* erecovery_vendor_a  16M    p55*/
  {PART_BOOT_A,                      614*1024,         25*1024,    UFS_PART_3},/* boot_a              25M    p56*/
  {PART_RECOVERY_A,                  639*1024,         45*1024,    UFS_PART_3},/* recovery_a          45M    p57*/
  {PART_ERECOVERY_A,                 684*1024,         45*1024,    UFS_PART_3},/* erecovery_a         45M    p58*/
  {PART_METADATA,                    729*1024,         16*1024,    UFS_PART_3},/* metadata            16M    p59*/
  {PART_KPATCH_A,                    745*1024,         48*1024,    UFS_PART_3},/* kpatch_a            48M    p60*/
  {PART_ERECOVERY_RAMDISK_B,         793*1024,         32*1024,    UFS_PART_3},/* erecovery_ramdisk_b 32M    p61*/
  {PART_ERECOVERY_VENDOR_B,          825*1024,         16*1024,    UFS_PART_3},/* erecovery_vendor_b  16M    p62*/
  {PART_BOOT_B,                      841*1024,         25*1024,    UFS_PART_3},/* boot_b              25M    p63*/
  {PART_RECOVERY_B,                  866*1024,         45*1024,    UFS_PART_3},/* recovery_b          45M    p64*/
  {PART_ERECOVERY_B,                 911*1024,         45*1024,    UFS_PART_3},/* erecovery_b         45M    p65*/
  {PART_KPATCH_B,                    956*1024,         48*1024,    UFS_PART_3},/* kpatch_b            48M    p66*/
#endif
  {PART_ENG_SYSTEM_A,               1004*1024,         12*1024,    UFS_PART_3},/* eng_system_a        12M    p67*/
  {PART_ENG_SYSTEM_B,               1016*1024,         12*1024,    UFS_PART_3},/* eng_system_b        12M    p68*/
  {PART_RAMDISK_A,                  1028*1024,          2*1024,    UFS_PART_3},/* ramdisk_a            2M    p69*/
  {PART_RAMDISK_B,                  1030*1024,          2*1024,    UFS_PART_3},/* ramdisk_b            2M    p70*/
  {PART_VBMETA_SYSTEM_A,            1032*1024,          1*1024,    UFS_PART_3},/* vbmeta_system_a      1M    p71*/
  {PART_VBMETA_SYSTEM_B,            1033*1024,          1*1024,    UFS_PART_3},/* vbmeta_system_b      1M    p72*/
  {PART_VBMETA_VENDOR_A,            1034*1024,          1*1024,    UFS_PART_3},/* vbmeta_vendor_a      1M    p73*/
  {PART_VBMETA_VENDOR_B,            1035*1024,          1*1024,    UFS_PART_3},/* vbmeta_vendor_b      1M    p74*/
  {PART_VBMETA_ODM_A,               1036*1024,          1*1024,    UFS_PART_3},/* vbmeta_odm_a         1M    p75*/
  {PART_VBMETA_ODM_B,               1037*1024,          1*1024,    UFS_PART_3},/* vbmeta_odm_b         1M    p76*/
  {PART_VBMETA_CUST_A,              1038*1024,          1*1024,    UFS_PART_3},/* vbmeta_cus_a         1M    p77*/
  {PART_VBMETA_CUST_B,              1039*1024,          1*1024,    UFS_PART_3},/* vbmeta_cust_b        1M    p78*/
  {PART_VBMETA_HW_PRODUCT_A,        1040*1024,          1*1024,    UFS_PART_3},/* vbmeta_hw_product_a  1M    p79*/
  {PART_VBMETA_HW_PRODUCT_B,        1041*1024,          1*1024,    UFS_PART_3},/* vbmeta_hw_product_b  1M    p80*/
  {PART_RECOVERY_RAMDISK_A,         1042*1024,         32*1024,    UFS_PART_3},/* recovery_ramdisk_a  32M    p81*/
  {PART_RECOVERY_RAMDISK_B,         1074*1024,         32*1024,    UFS_PART_3},/* recovery_ramdisk_b  32M    p82*/
#ifdef CONFIG_FACTORY_MODE
  {PART_RECOVERY_VENDOR_A,          1106*1024,         24*1024,    UFS_PART_3},/* recovery_vendor_a   24M    p83*/
  {PART_RECOVERY_VENDOR_B,          1130*1024,         24*1024,    UFS_PART_3},/* recovery_vendor_b   24M    p84*/
  {PART_SECURITY_DTB_A,             1154*1024,          2*1024,    UFS_PART_3},/* security_dtb_a       2M    p85*/
  {PART_SECURITY_DTB_B,             1156*1024,          2*1024,    UFS_PART_3},/* security_dtb_b       2M    p86*/
  {PART_DTBO_A,                     1158*1024,         16*1024,    UFS_PART_3},/* dtoimage_a          16M    p87*/
  {PART_DTBO_B,                     1174*1024,         16*1024,    UFS_PART_3},/* dtoimage_b          16M    p88*/
  {PART_TRUSTFIRMWARE_A,            1190*1024,          2*1024,    UFS_PART_3},/* trustfirmware_a      2M    p89*/
  {PART_TRUSTFIRMWARE_B,            1192*1024,          2*1024,    UFS_PART_3},/* trustfirmware_b      2M    p90*/
  {PART_ENG_VENDOR_A,               1194*1024,         16*1024,    UFS_PART_3},/* eng_vendor_a        16M    p91*/
  {PART_ENG_VENDOR_B,               1210*1024,         16*1024,    UFS_PART_3},/* eng_vendor_b        16M    p92*/
#else
  {PART_RECOVERY_VENDOR_A,          1106*1024,         16*1024,    UFS_PART_3},/* recovery_vendor_a   24M    p83*/
  {PART_RECOVERY_VENDOR_B,          1122*1024,         16*1024,    UFS_PART_3},/* recovery_vendor_b   24M    p84*/
  {PART_SECURITY_DTB_A,             1138*1024,          2*1024,    UFS_PART_3},/* security_dtb_a       2M    p85*/
  {PART_SECURITY_DTB_B,             1140*1024,          2*1024,    UFS_PART_3},/* security_dtb_b       2M    p86*/
  {PART_DTBO_A,                     1142*1024,         20*1024,    UFS_PART_3},/* dtoimage_a          16M    p87*/
  {PART_DTBO_B,                     1162*1024,         20*1024,    UFS_PART_3},/* dtoimage_b          16M    p88*/
  {PART_TRUSTFIRMWARE_A,            1182*1024,          2*1024,    UFS_PART_3},/* trustfirmware_a      2M    p89*/
  {PART_TRUSTFIRMWARE_B,            1184*1024,          2*1024,    UFS_PART_3},/* trustfirmware_b      2M    p90*/
  {PART_ENG_VENDOR_A,               1186*1024,         20*1024,    UFS_PART_3},/* eng_vendor_a        16M    p91*/
  {PART_ENG_VENDOR_B,               1206*1024,         20*1024,    UFS_PART_3},/* eng_vendor_b        16M    p92*/
#endif
  {PART_RECOVERY_VBMETA_A,          1226*1024,          2*1024,    UFS_PART_3},/* recovery_vbmeta_a    2M    p93 */
  {PART_RECOVERY_VBMETA_B,          1228*1024,          2*1024,    UFS_PART_3},/* recovery_vbmeta_b    2M    p94 */
  {PART_ERECOVERY_VBMETA_A,         1230*1024,          2*1024,    UFS_PART_3},/* erecovery_vbmeta_a   2M    p95 */
  {PART_ERECOVERY_VBMETA_B,         1232*1024,          2*1024,    UFS_PART_3},/* erecovery_vbmeta_b   2M    p96 */
  {PART_VBMETA_A,                   1234*1024,          4*1024,    UFS_PART_3},/* PART_VBMETA_a        4M    p97 */
  {PART_VBMETA_B,                   1238*1024,          4*1024,    UFS_PART_3},/* PART_VBMETA_b        4M    p98 */
  {PART_PATCH_A,                    1242*1024,         32*1024,    UFS_PART_3},/* patch_a             32M    p99 */
  {PART_PATCH_B,                    1274*1024,         32*1024,    UFS_PART_3},/* patch_b             32M    p100*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS_A,                    1306*1024,        368*1024,    UFS_PART_3},/* preas_a            368M    p101*/
  {PART_PREAS_B,                    1674*1024,        368*1024,    UFS_PART_3},/* preas_b            368M    p102*/
  {PART_PREAVS_A,                   2042*1024,         32*1024,    UFS_PART_3},/* preavs_a            32M    p103*/
  {PART_PREAVS_B,                   2074*1024,         32*1024,    UFS_PART_3},/* preavs_b            32M    p104*/
  {PART_SUPER,                      2106*1024,      18334*1024,    UFS_PART_3},/* super             9168M    p105*/
  {PART_VERSION_A,                 20440*1024,        576*1024,    UFS_PART_3},/* version_a          576M    p106*/
  {PART_VERSION_B,                 21016*1024,        576*1024,    UFS_PART_3},/* version_b          576M    p107*/
  {PART_PRELOAD_A,                 21592*1024,       1144*1024,    UFS_PART_3},/* preload_a         1144M    p108*/
  {PART_PRELOAD_B,                 22736*1024,       1144*1024,    UFS_PART_3},/* preload_b         1144M    p109*/
  {PART_HIBENCH_IMG,               23880*1024,        128*1024,    UFS_PART_3},/* hibench_img        128M    p110*/
  {PART_HIBENCH_DATA,              24008*1024,        512*1024,    UFS_PART_3},/* hibench_data       512M    p111*/
  {PART_FLASH_AGEING,              24520*1024,        512*1024,    UFS_PART_3},/* FLASH_AGEING       512M    p112*/
  {PART_HIBENCH_LOG,               25032*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG         32M    p113*/
  {PART_HIBENCH_LPM3,              25064*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LPM3        32M    p114*/
  {PART_SECFLASH_AGEING,           25096*1024,         32*1024,    UFS_PART_3},/* secflash_ageing     32M    p115*/
  {PART_USERDATA,                  25128*1024, (4UL)*1024*1024,    UFS_PART_3},/* userdata             4G    p116*/
#elif defined CONFIG_MARKET_INTERNAL
#ifdef CONFIG_USE_EROFS
  {PART_PREAS_A,                  1306*1024,          296*1024,    UFS_PART_3},/* preas_a            296M    p101*/
  {PART_PREAS_B,                  1602*1024,          296*1024,    UFS_PART_3},/* preas_b            296M    p102*/
  {PART_PREAVS_A,                 1898*1024,           32*1024,    UFS_PART_3},/* preavs_a            32M    p103*/
  {PART_PREAVS_B,                 1930*1024,           32*1024,    UFS_PART_3},/* preavs_b            32M    p104*/
  {PART_SUPER,                    1962*1024,        13582*1024,    UFS_PART_3},/* super             6792M    p105*/
  {PART_VERSION_A,               15544*1024,          576*1024,    UFS_PART_3},/* version_a          576M    p106*/
  {PART_VERSION_B,               16120*1024,          576*1024,    UFS_PART_3},/* version_b          576M    p107*/
  {PART_PRELOAD_A,               16696*1024,         1144*1024,    UFS_PART_3},/* preload_a         1144M    p108*/
  {PART_PRELOAD_B,               17840*1024,         1144*1024,    UFS_PART_3},/* preload_b         1144M    p109*/
  {PART_USERDATA,                18984*1024,   (4UL)*1024*1024,    UFS_PART_3},/* userdata             4G    p110*/
#else
  {PART_PREAS_A,                  1306*1024,          368*1024,    UFS_PART_3},/* preas_a            368M    p101*/
  {PART_PREAS_B,                  1674*1024,          368*1024,    UFS_PART_3},/* preas_b            368M    p102*/
  {PART_PREAVS_A,                 2042*1024,           32*1024,    UFS_PART_3},/* preavs_a            32M    p103*/
  {PART_PREAVS_B,                 2074*1024,           32*1024,    UFS_PART_3},/* preavs_b            32M    p104*/
  {PART_SUPER,                    2106*1024,        18334*1024,    UFS_PART_3},/* super             9168M    p105*/
  {PART_VERSION_A,               20440*1024,          576*1024,    UFS_PART_3},/* version_a          576M    p106*/
  {PART_VERSION_B,               21016*1024,          576*1024,    UFS_PART_3},/* version_b          576M    p107*/
  {PART_PRELOAD_A,               21592*1024,         1144*1024,    UFS_PART_3},/* preload_a         1144M    p108*/
  {PART_PRELOAD_B,               22736*1024,         1144*1024,    UFS_PART_3},/* preload_b         1144M    p109*/
  {PART_USERDATA,                23880*1024,   (4UL)*1024*1024,    UFS_PART_3},/* userdata             4G    p110*/
#endif
#else
#ifdef CONFIG_USE_EROFS
  {PART_PREAS_A,                  1306*1024,         1024*1024,    UFS_PART_3},/* preas_a           1024M    p101*/
  {PART_PREAS_B,                  2330*1024,         1024*1024,    UFS_PART_3},/* preas_b           1024M    p102*/
  {PART_PREAVS_A,                 3354*1024,           32*1024,    UFS_PART_3},/* preavs_a            32M    p103*/
  {PART_PREAVS_B,                 3386*1024,           32*1024,    UFS_PART_3},/* preavs_b            32M    p104*/
  {PART_SUPER,                    3418*1024,        13534*1024,    UFS_PART_3},/* super             6768M    p105*/
  {PART_VERSION_A,               16952*1024,          576*1024,    UFS_PART_3},/* version_a          576M    p106*/
  {PART_VERSION_B,               17528*1024,          576*1024,    UFS_PART_3},/* version_b          576M    p107*/
  {PART_PRELOAD_A,               18104*1024,         1144*1024,    UFS_PART_3},/* preload_a         1144M    p108*/
  {PART_PRELOAD_B,               19248*1024,         1144*1024,    UFS_PART_3},/* preload_b         1144M    p109*/
  {PART_USERDATA,                20392*1024,   (4UL)*1024*1024,    UFS_PART_3},/* userdata             4G    p110*/
#else
  {PART_PREAS_A,                  1306*1024,         1280*1024,    UFS_PART_3},/* preas_a           1280M    p101*/
  {PART_PREAS_B,                  2586*1024,         1280*1024,    UFS_PART_3},/* preas_b           1280M    p102*/
  {PART_PREAVS_A,                 3866*1024,           32*1024,    UFS_PART_3},/* preavs_a            32M    p103*/
  {PART_PREAVS_B,                 3898*1024,           32*1024,    UFS_PART_3},/* preavs_b            32M    p104*/
  {PART_SUPER,                    3930*1024,        18766*1024,    UFS_PART_3},/* super             9384M    p105*/
  {PART_VERSION_A,               22696*1024,          576*1024,    UFS_PART_3},/* version_a          576M    p106*/
  {PART_VERSION_B,               23272*1024,          576*1024,    UFS_PART_3},/* version_b          576M    p107*/
  {PART_PRELOAD_A,               23848*1024,         1144*1024,    UFS_PART_3},/* preload_a         1144M    p108*/
  {PART_PRELOAD_B,               24992*1024,         1144*1024,    UFS_PART_3},/* preload_b         1144M    p109*/
  {PART_USERDATA,                26136*1024,   (4UL)*1024*1024,    UFS_PART_3},/* userdata             4G    p110*/
#endif
#endif
  {PART_PTABLE_LU4,                       0,               512,    UFS_PART_4},/* ptable_lu4        512K    p0*/
  {PART_RESERVED12,                     512,               1536,    UFS_PART_4},/* reserved12       1536K    p1*/
  {PART_USERDATA2,                     2048,    (4UL)*1024*1024,    UFS_PART_4},/* userdata2           4G    p2*/
  {"0", 0, 0, 0},
};

#endif
