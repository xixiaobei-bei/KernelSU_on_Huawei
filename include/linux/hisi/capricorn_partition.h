#ifndef _CAPRICORN_PARTITION_H_
#define _CAPRICORN_PARTITION_H_

#include "hisi_partition.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {PART_XLOADER,                           0,           2*1024,        EMMC_BOOT_MAJOR_PART},
  {PART_RESERVED0,                         0,           2*1024,        EMMC_BOOT_BACKUP_PART},
  {PART_PTABLE,                            0,              512,        EMMC_USER_PART},/* ptable          512K */
  {PART_FRP,                             512,              512,        EMMC_USER_PART},/* frp             512K   p1*/
  {PART_PERSIST,                        1024,           6*1024,        EMMC_USER_PART},/* persist           6M   p2*/
  {PART_RESERVED1,                    7*1024,             1024,        EMMC_USER_PART},/* reserved1      1024K   p3*/
  {PART_RESERVED6,                    8*1024,              512,        EMMC_USER_PART},/* reserved6       512K   p4*/
  {PART_VRL,                            8704,              512,        EMMC_USER_PART},/* vrl             512K   p5*/
  {PART_VRL_BACKUP,                   9*1024,              512,        EMMC_USER_PART},/* vrl backup      512K   p6*/
  {PART_MODEM_SECURE,                   9728,             8704,        EMMC_USER_PART},/* modem_secure   8704K   p7*/
  {PART_NVME,                        18*1024,           5*1024,        EMMC_USER_PART},/* nvme              5M   p8*/
  {PART_CTF,                         23*1024,           1*1024,        EMMC_USER_PART},/* ctf               1M   p9*/
  {PART_OEMINFO,                     24*1024,          96*1024,        EMMC_USER_PART},/* oeminfo          96M   p10*/
  {PART_SECURE_STORAGE,             120*1024,          32*1024,        EMMC_USER_PART},/* secure storage   32M   p11*/
  {PART_MODEMNVM_FACTORY,           152*1024,          16*1024,        EMMC_USER_PART},/* modemnvm factory 16M   p12*/
  {PART_MODEMNVM_BACKUP,            168*1024,          16*1024,        EMMC_USER_PART},/* modemnvm backup  16M   p13*/
  {PART_MODEMNVM_IMG,               184*1024,          34*1024,        EMMC_USER_PART},/* modemnvm img     34M   p14*/
  {PART_HISEE_ENCOS,                218*1024,           4*1024,        EMMC_USER_PART},/* hisee_encos       4M   p15*/
  {PART_VERITYKEY,                  222*1024,           1*1024,        EMMC_USER_PART},/* veritykey         1M   p16*/
  {PART_DDR_PARA,                   223*1024,           1*1024,        EMMC_USER_PART},/* DDR_PARA          1M   p17*/
  {PART_LOWPOWER_PARA,              224*1024,           1*1024,        EMMC_USER_PART},/* lowpower_para     1M   p18*/
  {PART_BATT_TP_PARA,               225*1024,           1*1024,        EMMC_USER_PART},/* batt_tp_para      1M   p19*/
  {PART_BL2,                        226*1024,           4*1024,        EMMC_USER_PART},/* bl2                4M    p20*/
  {PART_RESERVED2,                  230*1024,          21*1024,        EMMC_USER_PART},/* reserved2         21M    p21*/
  {PART_SPLASH2,                    251*1024,          80*1024,        EMMC_USER_PART},/* splash2           80M    p22*/
  {PART_BOOTFAIL_INFO,              331*1024,           2*1024,        EMMC_USER_PART},/* bootfail info      2M    p23*/
  {PART_MISC,                       333*1024,           2*1024,        EMMC_USER_PART},/* misc               2M    p24*/
  {PART_DFX,                        335*1024,          16*1024,        EMMC_USER_PART},/* dfx               16M    p25*/
  {PART_RRECORD,                    351*1024,          16*1024,        EMMC_USER_PART},/* rrecord           16M    p26*/
  {PART_CACHE,                      367*1024,         104*1024,        EMMC_USER_PART},/* cache            104M    p27*/
  {PART_FW_LPM3,                    471*1024,             1024,        EMMC_USER_PART},/* fw_lpm3         1024K    p28*/
  {PART_RESERVED3,                  472*1024,           5*1024,        EMMC_USER_PART},/* reserved3       5120K    p29*/
  {PART_NPU,                        477*1024,           8*1024,        EMMC_USER_PART},/* npu                8M    p30*/
  {PART_HIEPS,                      485*1024,           2*1024,        EMMC_USER_PART},/* hieps              2M    p31*/
  {PART_IVP,                        487*1024,           2*1024,        EMMC_USER_PART},/* ivp                2M    p32*/
  {PART_HDCP,                       489*1024,           1*1024,        EMMC_USER_PART},/* PART_HDCP          1M    p33*/
  {PART_HISEE_IMG,                  490*1024,           4*1024,        EMMC_USER_PART},/* part_hisee_img     4M    p34*/
  {PART_HHEE,                       494*1024,           4*1024,        EMMC_USER_PART},/* hhee               4M    p35*/
  {PART_HISEE_FS,                   498*1024,           8*1024,        EMMC_USER_PART},/* hisee_fs           8M    p36*/
  {PART_FASTBOOT,                   506*1024,          12*1024,        EMMC_USER_PART},/* fastboot          12M    p37*/
  {PART_VECTOR,                     518*1024,           4*1024,        EMMC_USER_PART},/* vector             4M    p38*/
  {PART_ISP_BOOT,                   522*1024,           2*1024,        EMMC_USER_PART},/* isp_boot           2M    p39*/
  {PART_ISP_FIRMWARE,               524*1024,          14*1024,        EMMC_USER_PART},/* isp_firmware      14M    p40*/
  {PART_FW_HIFI,                    538*1024,          12*1024,        EMMC_USER_PART},/* hifi              12M    p41*/
  {PART_TEEOS,                      550*1024,           8*1024,        EMMC_USER_PART},/* teeos              8M    p42*/
  {PART_SENSORHUB,                  558*1024,          16*1024,        EMMC_USER_PART},/* sensorhub         16M    p43*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           574*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p44*/
  {PART_ERECOVERY_RAMDISK,          598*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p45*/
  {PART_ERECOVERY_VENDOR,           630*1024,           8*1024,        EMMC_USER_PART},/* erecovery_vendor   8M    p46*/
  {PART_KERNEL,                     638*1024,          32*1024,        EMMC_USER_PART},/* kernel            32M    p47*/
#else
  {PART_ERECOVERY_KERNEL,           574*1024,          24*1024,        EMMC_USER_PART},/* erecovery_kernel  24M    p44*/
  {PART_ERECOVERY_RAMDISK,          598*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p45*/
  {PART_ERECOVERY_VENDOR,           630*1024,          16*1024,        EMMC_USER_PART},/* erecovery_vendor  16M    p46*/
  {PART_KERNEL,                     646*1024,          24*1024,        EMMC_USER_PART},/* kernel            24M    p47*/
#endif
  {PART_ENG_SYSTEM,                 670*1024,          12*1024,        EMMC_USER_PART},/* eng_system        12M    p48*/
  {PART_RECOVERY_RAMDISK,           682*1024,          32*1024,        EMMC_USER_PART},/* recovery_ramdisk  32M    p49*/
  {PART_RECOVERY_VENDOR,            714*1024,          16*1024,        EMMC_USER_PART},/* recovery_vendor   16M    p50*/
  {PART_DTS,                        730*1024,           1*1024,        EMMC_USER_PART},/* dtimage            1M    p51*/
  {PART_DTO,                        731*1024,          20*1024,        EMMC_USER_PART},/* dtoimage          20M    p52*/
  {PART_TRUSTFIRMWARE,              751*1024,           2*1024,        EMMC_USER_PART},/* trustfirmware      2M    p53*/
  {PART_MODEM_FW,                   753*1024,          56*1024,        EMMC_USER_PART},/* modem_fw          56M    p54*/
  {PART_ENG_VENDOR,                 809*1024,          20*1024,        EMMC_USER_PART},/* eng_vendor        20M    p55*/
  {PART_MODEM_PATCH_NV,             829*1024,           4*1024,        EMMC_USER_PART},/* modem_patch_nv     4M    p56*/
  {PART_MODEM_DRIVER,               833*1024,          20*1024,        EMMC_USER_PART},/* modem_driver      20M    p57*/
  {PART_RESERVED4,                  853*1024,          11*1024,        EMMC_USER_PART},/* reserved4A        11M    p58*/
  {PART_RECOVERY_VBMETA,            864*1024,           2*1024,        EMMC_USER_PART},/* recovery_vbmeta    2M    p59*/
  {PART_ERECOVERY_VBMETA,           866*1024,           2*1024,        EMMC_USER_PART},/* erecovery_vbmeta   2M    p60*/
  {PART_VBMETA,                     868*1024,           4*1024,        EMMC_USER_PART},/* PART_VBMETA        4M    p61*/
  {PART_MODEMNVM_UPDATE,            872*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_update   16M    p62*/
  {PART_MODEMNVM_CUST,              888*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_cust     16M    p63*/
  {PART_PATCH,                      904*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p64*/
#ifdef CONFIG_FACTORY_MODE
  {PART_VENDOR,                     936*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p65*/
  {PART_ODM,                       2168*1024,         272*1024,        EMMC_USER_PART},/* odm              272M    p66*/
  {PART_CUST,                      2440*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p67*/
  {PART_SYSTEM,                    2632*1024,        5520*1024,        EMMC_USER_PART},/* system          5520M    p68*/
  {PART_HW_PRODUCT,                8152*1024,        1328*1024,        EMMC_USER_PART},/* hw_product      1328M    p69*/
  {PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_HIBENCH_IMG,              11200*1024,         128*1024,        EMMC_USER_PART},/* hibench_img      128M    p72*/
  {PART_HIBENCH_DATA,             11328*1024,         512*1024,        EMMC_USER_PART},/* hibench_data     512M    p73*/
  {PART_FLASH_AGEING,             11840*1024,         512*1024,        EMMC_USER_PART},/* FLASH_AGEING     512M    p74*/
  {PART_HIBENCH_LPM3,             12352*1024,         32*1024,         EMMC_USER_PART},/* HIBENCH_LPM3     32M     p75*/
  {PART_USERDATA,                 12384*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p76*/
#else
  #ifdef CONFIG_USE_EROFS
  {PART_VENDOR,                     936*1024,         864*1024,        EMMC_USER_PART},/* vendor           864M    p65*/
  {PART_ODM,                       1800*1024,         192*1024,        EMMC_USER_PART},/* odm              192M    p66*/
  {PART_CUST,                      1992*1024,         136*1024,        EMMC_USER_PART},/* cust             136M    p67*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2128*1024,        2040*1024,        EMMC_USER_PART},/* system          2040M    p68*/
  {PART_HW_PRODUCT,                4168*1024,        1024*1024,        EMMC_USER_PART},/* hw_product      1024M    p69*/
  {PART_VERSION,                   5192*1024,         200*1024,        EMMC_USER_PART},/* version          200M    p70*/
  {PART_PRELOAD,                   5392*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p71*/
  {PART_USERDATA,                  6320*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p72*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    2128*1024,        2688*1024,        EMMC_USER_PART},/* system          2688M    p68*/
  {PART_HW_PRODUCT,                4816*1024,         736*1024,        EMMC_USER_PART},/* hw_product       736M    p69*/
  {PART_VERSION,                   5552*1024,         200*1024,        EMMC_USER_PART},/* version          200M    p70*/
  {PART_PRELOAD,                   5752*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p71*/
  {PART_USERDATA,                  6680*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p72*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2128*1024,        3352*1024,        EMMC_USER_PART},/* system          3352M    p68*/
  {PART_HW_PRODUCT,                5480*1024,        1472*1024,        EMMC_USER_PART},/* hw_product      1472M    p69*/
  {PART_VERSION,                   6952*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                   7528*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_USERDATA,                  8672*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p72*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2128*1024,        4096*1024,        EMMC_USER_PART},/* system          4096M    p68*/
  {PART_HW_PRODUCT,                6224*1024,        1328*1024,        EMMC_USER_PART},/* hw_product      1328M    p69*/
  {PART_VERSION,                   7552*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                   8128*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_USERDATA,                  9272*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p72*/
  #else
  {PART_SYSTEM,                    2128*1024,        5520*1024,        EMMC_USER_PART},/* system          5520M    p68*/
  {PART_HW_PRODUCT,                7648*1024,        1328*1024,        EMMC_USER_PART},/* hw_product      1328M    p69*/
  {PART_VERSION,                   8976*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                   9552*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_USERDATA,                 10696*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p72*/
  #endif
  #else /* do not use erofs */
  {PART_VENDOR,                     936*1024,        1232*1024,        EMMC_USER_PART},/* vendor          1232M    p65*/
  {PART_ODM,                       2168*1024,         272*1024,        EMMC_USER_PART},/* odm              272M    p66*/
  {PART_CUST,                      2440*1024,         192*1024,        EMMC_USER_PART},/* cust             192M    p67*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2632*1024,        3360*1024,        EMMC_USER_PART},/* system          3360M    p68*/
  {PART_HW_PRODUCT,                5992*1024,        1312*1024,        EMMC_USER_PART},/* hw_product      1312M    p69*/
  {PART_VERSION,                   7304*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                   7880*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p71*/
  {PART_USERDATA,                  8808*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p72*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    2632*1024,        4384*1024,        EMMC_USER_PART},/* system          4384M    p68*/
  {PART_HW_PRODUCT,                7016*1024,         952*1024,        EMMC_USER_PART},/* hw_product       952M    p69*/
  {PART_VERSION,                   7968*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                   8544*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p71*/
  {PART_USERDATA,                  9472*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G     p72*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2632*1024,        4784*1024,        EMMC_USER_PART},/* system          4784M    p68*/
  {PART_HW_PRODUCT,                7416*1024,        1472*1024,        EMMC_USER_PART},/* hw_product      1472M    p69*/
  {PART_VERSION,                   8888*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                   9464*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_USERDATA,                 10608*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p72*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2632*1024,        5848*1024,        EMMC_USER_PART},/* system          5848M    p68*/
  {PART_HW_PRODUCT,                8480*1024,        1328*1024,        EMMC_USER_PART},/* hw_product      1328M    p69*/
  {PART_VERSION,                   9808*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                  10384*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_USERDATA,                 11528*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p72*/
  #else
  {PART_SYSTEM,                    2632*1024,        5520*1024,        EMMC_USER_PART},/* system          5520M    p68*/
  {PART_HW_PRODUCT,                8152*1024,        1328*1024,        EMMC_USER_PART},/* hw_product      1328M    p69*/
  {PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p70*/
  {PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p71*/
  {PART_USERDATA,                 11200*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p72*/
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
  {PART_PTABLE,                            0,              512,        UFS_PART_2},/* ptable                    512K    */
  {PART_FRP,                             512,              512,        UFS_PART_2},/* frp                       512K    p1*/
  {PART_PERSIST,                      1*1024,           6*1024,        UFS_PART_2},/* persist                  6144K    p2*/
  {PART_RESERVED1,                    7*1024,             1024,        UFS_PART_2},/* reserved1                1024K    p3*/
  {PART_PTABLE_LU3,                        0,              512,        UFS_PART_3},/* ptable_lu3                512K    p0*/
  {PART_VRL,                             512,              512,        UFS_PART_3},/* vrl                       512K    p1*/
  {PART_VRL_BACKUP,                     1024,              512,        UFS_PART_3},/* vrl backup                512K    p2*/
  {PART_MODEM_SECURE,                   1536,             8704,        UFS_PART_3},/* modem_secure             8704K    p3*/
  {PART_NVME,                        10*1024,           5*1024,        UFS_PART_3},/* nvme                        5M    p4*/
  {PART_CTF,                         15*1024,           1*1024,        UFS_PART_3},/* PART_CTF                    1M    p5*/
  {PART_OEMINFO,                     16*1024,          96*1024,        UFS_PART_3},/* oeminfo                    96M    p6*/
  {PART_SECURE_STORAGE,             112*1024,          32*1024,        UFS_PART_3},/* secure storage             32M    p7*/
  {PART_MODEMNVM_FACTORY,           144*1024,          16*1024,        UFS_PART_3},/* modemnvm factory           16M    p8*/
  {PART_MODEMNVM_BACKUP,            160*1024,          16*1024,        UFS_PART_3},/* modemnvm backup            16M    p9*/
  {PART_MODEMNVM_IMG,               176*1024,          34*1024,        UFS_PART_3},/* modemnvm img               34M    p10*/
  {PART_HISEE_ENCOS,                210*1024,           4*1024,        UFS_PART_3},/* hisee_encos                 4M    p11*/
  {PART_VERITYKEY,                  214*1024,           1*1024,        UFS_PART_3},/* veritykey                   1M    p12*/
  {PART_DDR_PARA,                   215*1024,           1*1024,        UFS_PART_3},/* ddr_para                    1M    p13*/
  {PART_LOWPOWER_PARA,              216*1024,           1*1024,        UFS_PART_3},/* lowpower_para               1M    p14*/
  {PART_BATT_TP_PARA,               217*1024,           1*1024,        UFS_PART_3},/* batt_tp_para                1M    p15*/
  {PART_BL2,                        218*1024,           4*1024,        UFS_PART_3},/* bl2                         4M    p16*/
  {PART_RESERVED2,                  222*1024,          21*1024,        UFS_PART_3},/* reserved2                  21M    p17*/
  {PART_SPLASH2,                    243*1024,          80*1024,        UFS_PART_3},/* splash2                    80M    p18*/
  {PART_BOOTFAIL_INFO,              323*1024,           2*1024,        UFS_PART_3},/* bootfail info              2MB    p19*/
  {PART_MISC,                       325*1024,           2*1024,        UFS_PART_3},/* misc                        2M    p20*/
  {PART_DFX,                        327*1024,          16*1024,        UFS_PART_3},/* dfx                        16M    p21*/
  {PART_RRECORD,                    343*1024,          16*1024,        UFS_PART_3},/* rrecord                    16M    p22*/
  {PART_CACHE,                      359*1024,         104*1024,        UFS_PART_3},/* cache                     104M    p23*/
  {PART_FW_LPM3,                    463*1024,             1024,        UFS_PART_3},/* fw_lpm3                  1024K    p24*/
  {PART_RESERVED3,                  464*1024,           5*1024,        UFS_PART_3},/* reserved3A               5120K    p25*/
  {PART_NPU,                        469*1024,           8*1024,        UFS_PART_3},/* npu                         8M    p26*/
  {PART_HIEPS,                      477*1024,           2*1024,        UFS_PART_3},/* hieps                       2M    p27*/
  {PART_IVP,                        479*1024,           2*1024,        UFS_PART_3},/* ivp                         2M    p28*/
  {PART_HDCP,                       481*1024,           1*1024,        UFS_PART_3},/* PART_HDCP                   1M    p29*/
  {PART_HISEE_IMG,                  482*1024,           4*1024,        UFS_PART_3},/* part_hisee_img              4M    p30*/
  {PART_HHEE,                       486*1024,           4*1024,        UFS_PART_3},/* hhee                        4M    p31*/
  {PART_HISEE_FS,                   490*1024,           8*1024,        UFS_PART_3},/* hisee_fs                    8M    p32*/
  {PART_FASTBOOT,                   498*1024,          12*1024,        UFS_PART_3},/* fastboot                   12M    p33*/
  {PART_VECTOR,                     510*1024,           4*1024,        UFS_PART_3},/* avs vector                  4M    p34*/
  {PART_ISP_BOOT,                   514*1024,           2*1024,        UFS_PART_3},/* isp_boot                    2M    p35*/
  {PART_ISP_FIRMWARE,               516*1024,          14*1024,        UFS_PART_3},/* isp_firmware               14M    p36*/
  {PART_FW_HIFI,                    530*1024,          12*1024,        UFS_PART_3},/* hifi                       12M    p37*/
  {PART_TEEOS,                      542*1024,           8*1024,        UFS_PART_3},/* teeos                       8M    p38*/
  {PART_SENSORHUB,                  550*1024,          16*1024,        UFS_PART_3},/* sensorhub                  16M    p39*/
#ifdef CONFIG_SANITIZER_ENABLE
  {PART_ERECOVERY_KERNEL,           566*1024,          24*1024,        UFS_PART_3},/* erecovery_kernel           24M    p40*/
  {PART_ERECOVERY_RAMDISK,          590*1024,          32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p41*/
  {PART_ERECOVERY_VENDOR,           622*1024,           8*1024,        UFS_PART_3},/* erecovery_vendor            8M    p42*/
  {PART_KERNEL,                     630*1024,          32*1024,        UFS_PART_3},/* kernel                     32M    p43*/
#else
  {PART_ERECOVERY_KERNEL,           566*1024,          24*1024,        UFS_PART_3},/* erecovery_kernel           24M    p40*/
  {PART_ERECOVERY_RAMDISK,          590*1024,          32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p41*/
  {PART_ERECOVERY_VENDOR,           622*1024,          16*1024,        UFS_PART_3},/* erecovery_vendor           16M    p42*/
  {PART_KERNEL,                     638*1024,          24*1024,        UFS_PART_3},/* kernel                     24M    p43*/
#endif
  {PART_ENG_SYSTEM,                 662*1024,          12*1024,        UFS_PART_3},/* eng_system                 12M    p44*/
  {PART_RECOVERY_RAMDISK,           674*1024,          32*1024,        UFS_PART_3},/* recovery_ramdisk           32M    p45*/
  {PART_RECOVERY_VENDOR,            706*1024,          16*1024,        UFS_PART_3},/* recovery_vendor            16M    p46*/
  {PART_DTS,                        722*1024,           1*1024,        UFS_PART_3},/* dtimage                     1M    p47*/
  {PART_DTO,                        723*1024,          20*1024,        UFS_PART_3},/* dtoimage                   20M    p48*/
  {PART_TRUSTFIRMWARE,              743*1024,           2*1024,        UFS_PART_3},/* trustfirmware               2M    p49*/
  {PART_MODEM_FW,                   745*1024,          56*1024,        UFS_PART_3},/* modem_fw                   56M    p50*/
  {PART_ENG_VENDOR,                 801*1024,          20*1024,        UFS_PART_3},/* eng_vendor                 20M    p51*/
  {PART_MODEM_PATCH_NV,             821*1024,           4*1024,        UFS_PART_3},/* modem_patch_nv              4M    p52*/
  {PART_MODEM_DRIVER,               825*1024,          20*1024,        UFS_PART_3},/* modem_driver               20M    p53*/
  {PART_RESERVED4,                  845*1024,          11*1024,        UFS_PART_3},/* reserved4A                 11M    p54*/
  {PART_RECOVERY_VBMETA,            856*1024,           2*1024,        UFS_PART_3},/* recovery_vbmeta             2M    p55*/
  {PART_ERECOVERY_VBMETA,           858*1024,           2*1024,        UFS_PART_3},/* erecovery_vbmeta            2M    p56*/
  {PART_VBMETA,                     860*1024,           4*1024,        UFS_PART_3},/* vbmeta                      4M    p57*/
  {PART_MODEMNVM_UPDATE,            864*1024,          16*1024,        UFS_PART_3},/* modemnvm_update            16M    p58*/
  {PART_MODEMNVM_CUST,              880*1024,          16*1024,        UFS_PART_3},/* modemnvm_cust              16M    p59*/
  {PART_PATCH,                      896*1024,          32*1024,        UFS_PART_3},/* patch                      32M    p60*/
#ifdef CONFIG_FACTORY_MODE
  {PART_VENDOR,                     928*1024,        1232*1024,        UFS_PART_3},/* vendor                   1232M    p61*/
  {PART_ODM,                       2160*1024,         272*1024,        UFS_PART_3},/* odm                       272M    p62*/
  {PART_CUST,                      2432*1024,         192*1024,        UFS_PART_3},/* cust                      192M    p63*/
  {PART_SYSTEM,                    2624*1024,        5520*1024,        UFS_PART_3},/* system                   5520M    p64*/
  {PART_HW_PRODUCT,                8144*1024,        1328*1024,        UFS_PART_3},/* hw_product               1328M    p65*/
  {PART_VERSION,                   9472*1024,         576*1024,        UFS_PART_3},/* version                   576M    p66*/
  {PART_PRELOAD,                  10048*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p67*/
  {PART_HIBENCH_IMG,              11192*1024,         128*1024,        UFS_PART_3},/* hibench_img               128M    p68*/
  {PART_HIBENCH_DATA,             11320*1024,         512*1024,        UFS_PART_3},/* hibench_data              512M    p69*/
  {PART_FLASH_AGEING,             11832*1024,         512*1024,        UFS_PART_3},/* FLASH_AGEING              512M    p70*/
  {PART_HIBENCH_LPM3,             12344*1024,         32*1024,         UFS_PART_3},/* HIBENCH_LPM3              32M      p71*/
  {PART_USERDATA,                 12376*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p72*/
#else
#ifdef CONFIG_USE_EROFS
  {PART_VENDOR,                     928*1024,         864*1024,        UFS_PART_3},/* vendor                    864M    p61*/
  {PART_ODM,                       1792*1024,         192*1024,        UFS_PART_3},/* odm                       192M    p62*/
  {PART_CUST,                      1984*1024,         136*1024,        UFS_PART_3},/* cust                      136M    p63*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2120*1024,        2040*1024,        UFS_PART_3},/* system                   2040M    p64*/
  {PART_HW_PRODUCT,                4160*1024,        1024*1024,        UFS_PART_3},/* hw_product               1024M    p65*/
  {PART_VERSION,                   5184*1024,         200*1024,        UFS_PART_3},/* version                   200M    p66*/
  {PART_PRELOAD,                   5384*1024,         928*1024,        UFS_PART_3},/* preload                   928M    p67*/
  {PART_USERDATA,                  6312*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p68*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    2120*1024,        2688*1024,        UFS_PART_3},/* system                   2688M    p64*/
  {PART_HW_PRODUCT,                4808*1024,         736*1024,        UFS_PART_3},/* hw_product                736M    p65*/
  {PART_VERSION,                   5544*1024,         200*1024,        UFS_PART_3},/* version                   200M    p66*/
  {PART_PRELOAD,                   5744*1024,         928*1024,        UFS_PART_3},/* preload                   928M    p67*/
  {PART_USERDATA,                  6672*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p68*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2120*1024,        3352*1024,        UFS_PART_3},/* system                   3352M    p64*/
  {PART_HW_PRODUCT,                5472*1024,        1472*1024,        UFS_PART_3},/* hw_product               1472M    p65*/
  {PART_VERSION,                   6944*1024,         576*1024,        UFS_PART_3},/* version                   576M    p66*/
  {PART_PRELOAD,                   7520*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p67*/
  {PART_USERDATA,                  8664*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p68*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2120*1024,        4096*1024,        UFS_PART_3},/* system                   4096M    p64*/
  {PART_HW_PRODUCT,                6216*1024,        1328*1024,        UFS_PART_3},/* hw_product               1328M    p65*/
  {PART_VERSION,                   7544*1024,         576*1024,        UFS_PART_3},/* version                   576M    p66*/
  {PART_PRELOAD,                   8120*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p67*/
  {PART_USERDATA,                  9264*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p68*/
  #else
  {PART_SYSTEM,                    2120*1024,        5520*1024,         UFS_PART_3},/* system                   912M    p64*/
  {PART_HW_PRODUCT,                7640*1024,        1328*1024,         UFS_PART_3},/* hw_product              1328M    p65*/
  {PART_VERSION,                   8968*1024,         576*1024,         UFS_PART_3},/* version                  576M    p66*/
  {PART_PRELOAD,                   9544*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p67*/
  {PART_USERDATA,                 10688*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p68*/
  #endif
  #else /* do not use erofs */
  {PART_VENDOR,                     928*1024,        1232*1024,         UFS_PART_3},/* vendor                  1232M    p61*/
  {PART_ODM,                       2160*1024,         272*1024,         UFS_PART_3},/* odm                      272M    p62*/
  {PART_CUST,                      2432*1024,         192*1024,         UFS_PART_3},/* cust                     192M    p63*/
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SYSTEM,                    2624*1024,        3360*1024,         UFS_PART_3},/* system                  3360M    p64*/
  {PART_HW_PRODUCT,                5984*1024,        1312*1024,         UFS_PART_3},/* hw_product              1312M    p65*/
  {PART_VERSION,                   7296*1024,         576*1024,         UFS_PART_3},/* version                  576M    p66*/
  {PART_PRELOAD,                   7872*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p67*/
  {PART_USERDATA,                  8800*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p68*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SYSTEM,                    2624*1024,        4384*1024,         UFS_PART_3},/* system                  4384M    p64*/
  {PART_HW_PRODUCT,                7008*1024,         952*1024,         UFS_PART_3},/* hw_product               952M    p65*/
  {PART_VERSION,                   7960*1024,         576*1024,         UFS_PART_3},/* version                  576M    p66*/
  {PART_PRELOAD,                   8536*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p67*/
  {PART_USERDATA,                  9464*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p68*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_SYSTEM,                    2624*1024,        4784*1024,         UFS_PART_3},/* system                  4784M    p64*/
  {PART_HW_PRODUCT,                7408*1024,        1472*1024,         UFS_PART_3},/* hw_product              1472M    p65*/
  {PART_VERSION,                   8880*1024,         576*1024,         UFS_PART_3},/* version                  576M    p66*/
  {PART_PRELOAD,                   9456*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p67*/
  {PART_USERDATA,                 10600*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p68*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_SYSTEM,                    2624*1024,        5848*1024,         UFS_PART_3},/* system                  5848M    p64*/
  {PART_HW_PRODUCT,                8472*1024,        1328*1024,         UFS_PART_3},/* hw_product              1328M    p65*/
  {PART_VERSION,                   9800*1024,         576*1024,         UFS_PART_3},/* version                  576M    p66*/
  {PART_PRELOAD,                  10376*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p67*/
  {PART_USERDATA,                 11520*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p68*/
  #else
  {PART_SYSTEM,                    2624*1024,        5520*1024,         UFS_PART_3},/* system                  5520M    p64*/
  {PART_HW_PRODUCT,                8144*1024,        1328*1024,         UFS_PART_3},/* hw_product              1328M    p65*/
  {PART_VERSION,                   9472*1024,         576*1024,         UFS_PART_3},/* version                  576M    p66*/
  {PART_PRELOAD,                  10048*1024,        1144*1024,         UFS_PART_3},/* preload                 1144M    p67*/
  {PART_USERDATA,                 11192*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p68*/
  #endif
  #endif
  #endif
  {"0", 0, 0, 0},
};
#endif

#endif
