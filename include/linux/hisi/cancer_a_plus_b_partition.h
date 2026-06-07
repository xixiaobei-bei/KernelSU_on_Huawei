#ifndef _CANCER_PARTITION_H_
#define _CANCER_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,                           0,           2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,                         0,           2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                            0,              512,        EMMC_USER_PART},/* ptable           512K    */
  {PART_FRP,                             512,              512,        EMMC_USER_PART},/* frp              512K    p1*/
  {PART_PERSIST,                        1024,           6*1024,        EMMC_USER_PART},/* persist            6M    p2*/
  {PART_RESERVED1,                    7*1024,             1024,        EMMC_USER_PART},/* reserved1       1024K    p3*/
  {PART_RESERVED6,                    8*1024,              512,        EMMC_USER_PART},/* reserved6        512K    p4*/
  {PART_VRL,                            8704,              512,        EMMC_USER_PART},/* vrl              512K    p5*/
  {PART_VRL_BACKUP,                   9*1024,              512,        EMMC_USER_PART},/* vrl backup       512K    p6*/
  {PART_MODEM_SECURE,                   9728,             8704,        EMMC_USER_PART},/* modem_secure    8704K    p7*/
  {PART_NVME,                        18*1024,           5*1024,        EMMC_USER_PART},/* nvme               5M    p8*/
  {PART_CTF,                         23*1024,           1*1024,        EMMC_USER_PART},/* ctf                1M    p9*/
  {PART_OEMINFO,                     24*1024,          96*1024,        EMMC_USER_PART},/* oeminfo           96M    p10*/
  {PART_SECURE_STORAGE,             120*1024,          32*1024,        EMMC_USER_PART},/* secure storage    32M    p11*/
  {PART_MODEMNVM_FACTORY,           152*1024,          32*1024,        EMMC_USER_PART},/* modemnvm factory  32M    p12*/
  {PART_MODEMNVM_BACKUP,            184*1024,          32*1024,        EMMC_USER_PART},/* modemnvm backup   32M    p13*/
  {PART_MODEMNVM_IMG,               216*1024,          64*1024,        EMMC_USER_PART},/* modemnvm img      34M    p14*/
  {PART_HISEE_ENCOS,                280*1024,           4*1024,        EMMC_USER_PART},/* hisee_encos        4M    p15*/
  {PART_VERITYKEY,                  284*1024,           1*1024,        EMMC_USER_PART},/* veritykey          1M    p16*/
  {PART_DDR_PARA,                   285*1024,           1*1024,        EMMC_USER_PART},/* DDR_PARA           1M    p17*/
  {PART_LOWPOWER_PARA,              286*1024,           1*1024,        EMMC_USER_PART},/* lowpower_para      1M    p18*/
  {PART_BATT_TP_PARA,               287*1024,           1*1024,        EMMC_USER_PART},/* batt_tp_para       1M    p19*/
  {PART_RESERVED2,                  288*1024,          25*1024,        EMMC_USER_PART},/* reserved2         25M    p20*/
  {PART_SPLASH2,                    313*1024,          80*1024,        EMMC_USER_PART},/* splash2           80M    p21*/
  {PART_BOOTFAIL_INFO,              393*1024,           2*1024,        EMMC_USER_PART},/* bootfail info      2M    p22*/
  {PART_MISC,                       395*1024,           2*1024,        EMMC_USER_PART},/* misc               2M    p23*/
  {PART_DFX,                        397*1024,          16*1024,        EMMC_USER_PART},/* dfx               16M    p24*/
  {PART_RRECORD,                    413*1024,          16*1024,        EMMC_USER_PART},/* rrecord           16M    p25*/
  {PART_CACHE,                      429*1024,         104*1024,        EMMC_USER_PART},/* cache            104M    p26*/
  {PART_FW_LPM3,                    533*1024,             1024,        EMMC_USER_PART},/* fw_lpm3           1M    p27*/
  {PART_RESERVED3,                  534*1024,           7*1024,        EMMC_USER_PART},/* reserved3A         7M    p28*/
  {PART_IVP,                        541*1024,           2*1024,        EMMC_USER_PART},/* ivp                2M    p29*/
  {PART_HDCP,                       543*1024,           1*1024,        EMMC_USER_PART},/* PART_HDCP         1M    p30*/
  {PART_HISEE_IMG,                  544*1024,           4*1024,        EMMC_USER_PART},/* part_hisee_img    4M    p31*/
  {PART_HHEE,                       548*1024,           4*1024,        EMMC_USER_PART},/* hhee              4M    p32*/
  {PART_HISEE_FS,                   552*1024,           8*1024,        EMMC_USER_PART},/* hisee_fs           8M    p33*/
  {PART_FASTBOOT,                   560*1024,          12*1024,        EMMC_USER_PART},/* fastboot          12M    p34*/
  {PART_VECTOR,                     572*1024,           4*1024,        EMMC_USER_PART},/* vector            4M    p35*/
  {PART_ISP_BOOT,                   576*1024,           2*1024,        EMMC_USER_PART},/* isp_boot           2M    p36*/
  {PART_ISP_FIRMWARE,               578*1024,          14*1024,        EMMC_USER_PART},/* isp_firmware      14M    p37*/
  {PART_FW_HIFI,                    592*1024,          12*1024,        EMMC_USER_PART},/* hifi              12M    p38*/
  {PART_TEEOS,                      604*1024,           8*1024,        EMMC_USER_PART},/* teeos              8M    p39*/
  {PART_SENSORHUB,                  612*1024,          16*1024,        EMMC_USER_PART},/* sensorhub         16M    p40*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           628*1024,          12*1024,        EMMC_USER_PART},/* erecovery_kernel  12M    p41*/
  {PART_ERECOVERY_RAMDISK,          640*1024,          12*1024,        EMMC_USER_PART},/* erecovery_ramdisk 12M    p42*/
  {PART_ERECOVERY_VENDOR,           652*1024,           8*1024,        EMMC_USER_PART},/* erecovery_vendor   8M    p43*/
  {PART_KERNEL,                     660*1024,          64*1024,        EMMC_USER_PART},/* kernel            64M    p44*/
#else
  {PART_ERECOVERY_KERNEL,           628*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p41*/
  {PART_ERECOVERY_RAMDISK,          652*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p42*/
  {PART_ERECOVERY_VENDOR,           684*1024,          16*1024,        EMMC_USER_PART},/* erecovery_vendor  16M    p43*/
  {PART_KERNEL,                     700*1024,          24*1024,        EMMC_USER_PART},/* kernel            24M    p44*/
#endif
  {PART_ENG_SYSTEM,                 724*1024,          12*1024,        EMMC_USER_PART},/* eng_system        12M    p45*/
  {PART_RECOVERY_RAMDISK,           736*1024,          32*1024,        EMMC_USER_PART},/* recovery_ramdisk  32M    p46*/
  {PART_RECOVERY_VENDOR,            768*1024,          16*1024,        EMMC_USER_PART},/* recovery_vendor   16M    p47*/
  {PART_DTS,                        784*1024,           1*1024,        EMMC_USER_PART},/* dtimage            1M    p48*/
  {PART_DTO,                        785*1024,          20*1024,        EMMC_USER_PART},/* dtoimage          20M    p49*/
  {PART_TRUSTFIRMWARE,              805*1024,           2*1024,        EMMC_USER_PART},/* trustfirmware      2M    p50*/
  {PART_MODEM_FW,                   807*1024,         150*1024,        EMMC_USER_PART},/* modem_fw         150M    p51*/
  {PART_ENG_VENDOR,                 957*1024,          12*1024,        EMMC_USER_PART},/* eng_vendor        12M    p52*/
  {PART_MODEM_PATCH_NV,             969*1024,           4*1024,        EMMC_USER_PART},/* modem_patch_nv     4M    p53*/
  {PART_MODEM_DRIVER,               973*1024,          20*1024,        EMMC_USER_PART},/* modem_driver      20M    p54*/
  {PART_RESERVED4,                  993*1024,          12*1024,        EMMC_USER_PART},/* reserved4A        12M    p55*/
  {PART_RAMDISK,                   1005*1024,           2*1024,        EMMC_USER_PART},/* ramdisk            2M    p56*/
  {PART_VBMETA_SYSTEM,             1007*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_system      1M    p57*/
  {PART_VBMETA_VENDOR,             1008*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_vendor      1M    p58*/
  {PART_VBMETA_ODM,                1009*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_odm         1M    p59*/
  {PART_VBMETA_CUST,               1010*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_cust        1M    p60*/
  {PART_VBMETA_HW_PRODUCT,         1011*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_hw_product  1M    p61*/
  {PART_RECOVERY_VBMETA,           1012*1024,           2*1024,        EMMC_USER_PART},/* recovery_vbmeta    2M    p62*/
  {PART_ERECOVERY_VBMETA,          1014*1024,           2*1024,        EMMC_USER_PART},/* erecovery_vbmeta   2M    p63*/
  {PART_VBMETA,                    1016*1024,           4*1024,        EMMC_USER_PART},/* PART_VBMETA        4M    p64*/
  {PART_MODEMNVM_UPDATE,           1020*1024,          20*1024,        EMMC_USER_PART},/* modemnvm_update   20M    p65*/
  {PART_MODEMNVM_CUST,             1040*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_cust     16M    p66*/
  {PART_PATCH,                     1056*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p67*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                     1088*1024,        1216*1024,        EMMC_USER_PART},/* preas           1216M    p68*/
  {PART_PREAVS,                    2304*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p69*/
  {PART_SUPER,                     2336*1024,        7576*1024,        EMMC_USER_PART},/* super           7576M    p70*/
  {PART_VERSION,                   9912*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p71*/
  {PART_PRELOAD,                  10488*1024,        1144*1024,        EMMC_USER_PART},/* preload_a       1144M    p72*/
  {PART_HIBENCH_IMG,              11632*1024,         128*1024,        EMMC_USER_PART},/* hibench_img      128M    p73*/
  {PART_HIBENCH_DATA,             11760*1024,         512*1024,        EMMC_USER_PART},/* hibench_data     512M    p74*/
  {PART_FLASH_AGEING,             12272*1024,         512*1024,        EMMC_USER_PART},/* FLASH_AGEING     512M    p75*/
  {PART_HIBENCH_LOG,              12784*1024,         32*1024,         EMMC_USER_PART},/* HIBENCH_LOG       32M    p76*/
  {PART_USERDATA,                 12816*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p77*/
#else
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
{PART_PREAS,                     1088*1024,         296*1024,        EMMC_USER_PART},/* preas           296M     p68*/
{PART_PREAVS,                    1384*1024,          32*1024,        EMMC_USER_PART},/* preavs           32M     p69*/
{PART_SUPER,                     1416*1024,        5632*1024,        EMMC_USER_PART},/* super           5632M    p70*/
{PART_VERSION,                   7048*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p71*/
{PART_PRELOAD,                   7624*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p72*/
{PART_USERDATA,                  8768*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p73*/
#else
{PART_PREAS,                     1088*1024,         976*1024,        EMMC_USER_PART},/* preas           976M     p68*/
{PART_PREAVS,                    2064*1024,          32*1024,        EMMC_USER_PART},/* preavs           32M     p69*/
{PART_SUPER,                     2096*1024,        5552*1024,        EMMC_USER_PART},/* super           5552M    p70*/
{PART_VERSION,                   7648*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p71*/
{PART_PRELOAD,                   8224*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p72*/
{PART_USERDATA,                  9368*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p73*/
#endif
#else
#ifdef CONFIG_MARKET_INTERNAL
{PART_PREAS,                     1088*1024,         360*1024,        EMMC_USER_PART},/* preas           360M     p68*/
{PART_PREAVS,                    1448*1024,          32*1024,        EMMC_USER_PART},/* preavs           32M     p69*/
{PART_SUPER,                     1480*1024,        7480*1024,        EMMC_USER_PART},/* super           7480M    p70*/
{PART_VERSION,                   8960*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p71*/
{PART_PRELOAD,                   9536*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p72*/
{PART_USERDATA,                 10680*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p73*/
#else
{PART_PREAS,                     1088*1024,        1216*1024,        EMMC_USER_PART},/* preas          1216M     p68*/
{PART_PREAVS,                    2304*1024,          32*1024,        EMMC_USER_PART},/* preavs           32M     p69*/
{PART_SUPER,                     2336*1024,        7544*1024,        EMMC_USER_PART},/* super           7544M    p70*/
{PART_VERSION,                   9880*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p71*/
{PART_PRELOAD,                  10456*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p72*/
{PART_USERDATA,                 11600*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p73*/
#endif
#endif
#endif
  {"0", 0, 0, 0},                                        /* total 11848M*/
};
#ifdef CONFIG_HISI_STORAGE_UFS_PARTITION
static const struct partition partition_table_ufs[] =
{
  {PART_XLOADER,                           0,           2*1024,        UFS_PART_0},
  {PART_RESERVED0,                         0,           2*1024,        UFS_PART_1},
  {PART_PTABLE,                            0,              512,        UFS_PART_2},/* ptable                512K    */
  {PART_FRP,                             512,              512,        UFS_PART_2},/* frp                   512K    p1*/
  {PART_PERSIST,                      1*1024,           6*1024,        UFS_PART_2},/* persist                 6M    p2*/
  {PART_RESERVED1,                    7*1024,             1024,        UFS_PART_2},/* reserved1            1024K    p3*/
  {PART_PTABLE_LU3,                        0,              512,        UFS_PART_3},/* ptable_lu3            512K    p0*/
  {PART_VRL,                             512,              512,        UFS_PART_3},/* vrl                   512K    p1*/
  {PART_VRL_BACKUP,                     1024,              512,        UFS_PART_3},/* vrl backup            512K    p2*/
  {PART_MODEM_SECURE,                   1536,             8704,        UFS_PART_3},/* modem_secure         8704K    p3*/
  {PART_NVME,                        10*1024,           5*1024,        UFS_PART_3},/* nvme                    5M    p4*/
  {PART_CTF,                         15*1024,           1*1024,        UFS_PART_3},/* PART_CTF                1M    p5*/
  {PART_OEMINFO,                     16*1024,          96*1024,        UFS_PART_3},/* oeminfo                96M    p6*/
  {PART_SECURE_STORAGE,             112*1024,          32*1024,        UFS_PART_3},/* secure storage         32M    p7*/
  {PART_MODEMNVM_FACTORY,           144*1024,          32*1024,        UFS_PART_3},/* modemnvm factory       32M    p8*/
  {PART_MODEMNVM_BACKUP,            176*1024,          32*1024,        UFS_PART_3},/* modemnvm backup        32M    p9*/
  {PART_MODEMNVM_IMG,               208*1024,          64*1024,        UFS_PART_3},/* modemnvm img           64M    p10*/
  {PART_HISEE_ENCOS,                272*1024,           4*1024,        UFS_PART_3},/* hisee_encos             4M    p11*/
  {PART_VERITYKEY,                  276*1024,           1*1024,        UFS_PART_3},/* veritykey               1M    p12*/
  {PART_DDR_PARA,                   277*1024,           1*1024,        UFS_PART_3},/* ddr_para                1M    p13*/
  {PART_LOWPOWER_PARA,              278*1024,           1*1024,        UFS_PART_3},/* lowpower_para           1M    p14*/
  {PART_BATT_TP_PARA,               279*1024,           1*1024,        UFS_PART_3},/* batt_tp_para            1M    p15*/
  {PART_RESERVED2,                  280*1024,          25*1024,        UFS_PART_3},/* reserved2              25M    p16*/
  {PART_SPLASH2,                    305*1024,          80*1024,        UFS_PART_3},/* splash2                80M    p17*/
  {PART_BOOTFAIL_INFO,              385*1024,           2*1024,        UFS_PART_3},/* bootfail info           2M    p18*/
  {PART_MISC,                       387*1024,           2*1024,        UFS_PART_3},/* misc                    2M    p19*/
  {PART_DFX,                        389*1024,          16*1024,        UFS_PART_3},/* dfx                    16M    p20*/
  {PART_RRECORD,                    405*1024,          16*1024,        UFS_PART_3},/* rrecord                16M    p21*/
  {PART_CACHE,                      421*1024,         104*1024,        UFS_PART_3},/* cache                 104M    p22*/
  {PART_FW_LPM3,                    525*1024,             1024,        UFS_PART_3},/* fw_lpm3                1M    p23*/
  {PART_RESERVED3,                  526*1024,           7*1024,        UFS_PART_3},/* reserved3A              7M    p24*/
  {PART_IVP,                        533*1024,           2*1024,        UFS_PART_3},/* ivp                     2M    p25*/
  {PART_HDCP,                       535*1024,           1*1024,        UFS_PART_3},/* PART_HDCP              1M    p26*/
  {PART_HISEE_IMG,                  536*1024,           4*1024,        UFS_PART_3},/* part_hisee_img         4M    p27*/
  {PART_HHEE,                       540*1024,           4*1024,        UFS_PART_3},/* hhee                   4M    p28*/
  {PART_HISEE_FS,                   544*1024,           8*1024,        UFS_PART_3},/* hisee_fs                8M    p29*/
  {PART_FASTBOOT,                   552*1024,          12*1024,        UFS_PART_3},/* fastboot               12M    p30*/
  {PART_VECTOR,                     564*1024,           4*1024,        UFS_PART_3},/* avs vector              4M    p31*/
  {PART_ISP_BOOT,                   568*1024,           2*1024,        UFS_PART_3},/* isp_boot                2M    p32*/
  {PART_ISP_FIRMWARE,               570*1024,          14*1024,        UFS_PART_3},/* isp_firmware           14M    p33*/
  {PART_FW_HIFI,                    584*1024,          12*1024,        UFS_PART_3},/* hifi                   12M    p34*/
  {PART_TEEOS,                      596*1024,           8*1024,        UFS_PART_3},/* teeos                   8M    p35*/
  {PART_SENSORHUB,                  604*1024,          16*1024,        UFS_PART_3},/* sensorhub              16M    p36*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           620*1024,          12*1024,        UFS_PART_3},/* erecovery_kernel       12M    p37*/
  {PART_ERECOVERY_RAMDISK,          632*1024,          12*1024,        UFS_PART_3},/* erecovery_ramdisk      12M    p38*/
  {PART_ERECOVERY_VENDOR,           644*1024,           8*1024,        UFS_PART_3},/* erecovery_vendor        8M    p39*/
  {PART_KERNEL,                     652*1024,          64*1024,        UFS_PART_3},/* kernel                 64M    p40*/
#else
  {PART_ERECOVERY_KERNEL,           620*1024,          24*1024,        UFS_PART_3},/* erecovery_kernel       24M    p37*/
  {PART_ERECOVERY_RAMDISK,          644*1024,          32*1024,        UFS_PART_3},/* erecovery_ramdisk      32M    p38*/
  {PART_ERECOVERY_VENDOR,           676*1024,          16*1024,        UFS_PART_3},/* erecovery_vendor       16M    p39*/
  {PART_KERNEL,                     692*1024,          24*1024,        UFS_PART_3},/* kernel                 24M    p40*/
#endif
  {PART_ENG_SYSTEM,                 716*1024,          12*1024,        UFS_PART_3},/* eng_system             12M    p41*/
  {PART_RECOVERY_RAMDISK,           728*1024,          32*1024,        UFS_PART_3},/* recovery_ramdisk       32M    p42*/
  {PART_RECOVERY_VENDOR,            760*1024,          16*1024,        UFS_PART_3},/* recovery_vendor        16M    p43*/
  {PART_DTS,                        776*1024,           1*1024,        UFS_PART_3},/* dtimage                 1M    p44*/
  {PART_DTO,                        777*1024,          20*1024,        UFS_PART_3},/* dtoimage               20M    p45*/
  {PART_TRUSTFIRMWARE,              797*1024,           2*1024,        UFS_PART_3},/* trustfirmware           2M    p46*/
  {PART_MODEM_FW,                   799*1024,         150*1024,        UFS_PART_3},/* modem_fw              150M    p47*/
  {PART_ENG_VENDOR,                 949*1024,          12*1024,        UFS_PART_3},/* eng_vendor             12M    p48*/
  {PART_MODEM_PATCH_NV,             961*1024,           4*1024,        UFS_PART_3},/* modem_patch_nv          4M    p49*/
  {PART_MODEM_DRIVER,               965*1024,          20*1024,        UFS_PART_3},/* modem_driver           20M    p50*/
  {PART_RESERVED4,                  985*1024,          12*1024,        UFS_PART_3},/* reserved4A             12M    p51*/
  {PART_RAMDISK,                    997*1024,           2*1024,        UFS_PART_3},/* ramdisk                 2M    p52*/
  {PART_VBMETA_SYSTEM,              999*1024,           1*1024,        UFS_PART_3},/* vbmeta_system           1M    p53*/
  {PART_VBMETA_VENDOR,             1000*1024,           1*1024,        UFS_PART_3},/* vbmeta_vendor           1M    p54*/
  {PART_VBMETA_ODM,                1001*1024,           1*1024,        UFS_PART_3},/* vbmeta_odm              1M    p55*/
  {PART_VBMETA_CUST,               1002*1024,           1*1024,        UFS_PART_3},/* vbmeta_cust             1M    p56*/
  {PART_VBMETA_HW_PRODUCT,         1003*1024,           1*1024,        UFS_PART_3},/* vbmeta_hw_product       1M    p57*/
  {PART_RECOVERY_VBMETA,           1004*1024,           2*1024,        UFS_PART_3},/* recovery_vbmeta         2M    p58*/
  {PART_ERECOVERY_VBMETA,          1006*1024,           2*1024,        UFS_PART_3},/* erecovery_vbmeta        2M    p59*/
  {PART_VBMETA,                    1008*1024,           4*1024,        UFS_PART_3},/* vbmeta                  4M    p60*/
  {PART_MODEMNVM_UPDATE,           1012*1024,          20*1024,        UFS_PART_3},/* modemnvm_update        20M    p61*/
  {PART_MODEMNVM_CUST,             1032*1024,          16*1024,        UFS_PART_3},/* modemnvm_cust          16M    p62*/
  {PART_PATCH,                     1048*1024,          32*1024,        UFS_PART_3},/* patch                  32M    p63*/
#ifdef CONFIG_FACTORY_MODE
  {PART_PREAS,                     1080*1024,        1216*1024,        UFS_PART_3},/* preas           1216M         p64*/
  {PART_PREAVS,                    2296*1024,          32*1024,        UFS_PART_3},/* preavs            32M         p65*/
  {PART_SUPER,                     2328*1024,        7576*1024,        UFS_PART_3},/* super           7576M         p66*/
  {PART_VERSION,                   9904*1024,         576*1024,        UFS_PART_3},/* version          576M         p67*/
  {PART_PRELOAD,                  10480*1024,        1144*1024,        UFS_PART_3},/* preload_a       1144M         p68*/
  {PART_HIBENCH_IMG,              11624*1024,         128*1024,        UFS_PART_3},/* hibench_img      128M         p69*/
  {PART_HIBENCH_DATA,             11752*1024,         512*1024,        UFS_PART_3},/* hibench_data     512M         p70*/
  {PART_FLASH_AGEING,             12264*1024,         512*1024,        UFS_PART_3},/* FLASH_AGEING     512M         p71*/
  {PART_HIBENCH_LOG,              12776*1024,          32*1024,        UFS_PART_3},/* HIBENCH_LOG       32M         p72*/
  {PART_USERDATA,                 12808*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata           4G         p73*/
#else
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,                     1080*1024,         296*1024,        UFS_PART_3},/* preas            296M         p64*/
  {PART_PREAVS,                    1376*1024,          32*1024,        UFS_PART_3},/* preavs            32M         p65*/
  {PART_SUPER,                     1408*1024,        5632*1024,        UFS_PART_3},/* super           5632M         p66*/
  {PART_VERSION,                   7040*1024,         576*1024,        UFS_PART_3},/* version          576M         p67*/
  {PART_PRELOAD,                   7616*1024,        1144*1024,        UFS_PART_3},/* preload         1144M         p68*/
  {PART_USERDATA,                  8760*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata           4G         p69*/
#else
  {PART_PREAS,                     1080*1024,         976*1024,        UFS_PART_3},/* preas            976M         p64*/
  {PART_PREAVS,                    2056*1024,          32*1024,        UFS_PART_3},/* preavs            32M         p65*/
  {PART_SUPER,                     2088*1024,        5552*1024,        UFS_PART_3},/* super           5552M         p66*/
  {PART_VERSION,                   7640*1024,         576*1024,        UFS_PART_3},/* version          576M         p67*/
  {PART_PRELOAD,                   8216*1024,        1144*1024,        UFS_PART_3},/* preload         1144M         p68*/
  {PART_USERDATA,                  9360*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata           4G         p69*/
#endif
#else
#ifdef CONFIG_MARKET_INTERNAL
  {PART_PREAS,                     1080*1024,         360*1024,        UFS_PART_3},/* preas            360M         p64*/
  {PART_PREAVS,                    1440*1024,          32*1024,        UFS_PART_3},/* preavs            32M         p65*/
  {PART_SUPER,                     1472*1024,        7480*1024,        UFS_PART_3},/* super           7480M         p66*/
  {PART_VERSION,                   8952*1024,         576*1024,        UFS_PART_3},/* version          576M         p67*/
  {PART_PRELOAD,                   9528*1024,        1144*1024,        UFS_PART_3},/* preload         1144M         p68*/
  {PART_USERDATA,                 10672*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata           4G         p69*/
#else
  {PART_PREAS,                     1080*1024,        1216*1024,        UFS_PART_3},/* preas           1216M         p64*/
  {PART_PREAVS,                    2296*1024,          32*1024,        UFS_PART_3},/* preavs            32M         p65*/
  {PART_SUPER,                     2328*1024,        7544*1024,        UFS_PART_3},/* super           7544M         p66*/
  {PART_VERSION,                   9872*1024,         576*1024,        UFS_PART_3},/* version          576M         p67*/
  {PART_PRELOAD,                  10448*1024,        1144*1024,        UFS_PART_3},/* preload         1144M         p68*/
  {PART_USERDATA,                 11592*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata           4G         p69*/
#endif
#endif
#endif
  {"0", 0, 0, 0},
};
#endif

#endif
