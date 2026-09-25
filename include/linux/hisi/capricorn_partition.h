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
  {PART_RECOVERY,                   574*1024,          53*1024,        EMMC_USER_PART},/* recovery          53M    p44*/
  {PART_ERECOVERY,                  627*1024,          53*1024,        EMMC_USER_PART},/* erecovery         53M    p45*/
  {PART_ERECOVERY_RAMDISK,          680*1024,          24*1024,        EMMC_USER_PART},/* erecovery_ramdisk 24M    p46*/
  {PART_ERECOVERY_VENDOR,           704*1024,           8*1024,        EMMC_USER_PART},/* erecovery_vendor   8M    p47*/
  {PART_BOOT,                       712*1024,          33*1024,        EMMC_USER_PART},/* boot              33M    p48*/
  {PART_KPATCH,                     745*1024,           6*1024,        EMMC_USER_PART},/* kpatch             6M    p49*/
#else
  {PART_RECOVERY,                   574*1024,          45*1024,        EMMC_USER_PART},/* recovery          45M    p44*/
  {PART_ERECOVERY,                  619*1024,          45*1024,        EMMC_USER_PART},/* erecovery         45M    p45*/
  {PART_ERECOVERY_RAMDISK,          664*1024,          32*1024,        EMMC_USER_PART},/* erecovery_ramdisk 32M    p46*/
  {PART_ERECOVERY_VENDOR,           696*1024,          16*1024,        EMMC_USER_PART},/* erecovery_vendor  16M    p47*/
  {PART_BOOT,                       712*1024,          25*1024,        EMMC_USER_PART},/* boot              25M    p48*/
  {PART_KPATCH,                     737*1024,          14*1024,        EMMC_USER_PART},/* kpatch            14M    p49*/
#endif
  {PART_ENG_SYSTEM,                 751*1024,          12*1024,        EMMC_USER_PART},/* eng_system        12M    p50*/
  {PART_RECOVERY_RAMDISK,           763*1024,          32*1024,        EMMC_USER_PART},/* recovery_ramdisk  32M    p51*/
  {PART_RECOVERY_VENDOR,            795*1024,          16*1024,        EMMC_USER_PART},/* recovery_vendor   16M    p52*/
  {PART_DTBO,                       811*1024,          20*1024,        EMMC_USER_PART},/* dtoimage          20M    p53*/
  {PART_TRUSTFIRMWARE,              831*1024,           2*1024,        EMMC_USER_PART},/* trustfirmware      2M    p54*/
  {PART_MODEM_FW,                   833*1024,          56*1024,        EMMC_USER_PART},/* modem_fw          56M    p55*/
  {PART_ENG_VENDOR,                 889*1024,          20*1024,        EMMC_USER_PART},/* eng_vendor        20M    p56*/
  {PART_MODEM_PATCH_NV,             909*1024,           4*1024,        EMMC_USER_PART},/* modem_patch_nv     4M    p57*/
  {PART_MODEM_DRIVER,               913*1024,          20*1024,        EMMC_USER_PART},/* modem_driver      20M    p58*/
  {PART_RESERVED4,                  933*1024,           4*1024,        EMMC_USER_PART},/* reserved4A         4M    p59*/
  {PART_RAMDISK,                    937*1024,           2*1024,        EMMC_USER_PART},/* ramdisk            2M    p60*/
  {PART_VBMETA_SYSTEM,              939*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_system      1M    p61*/
  {PART_VBMETA_VENDOR,              940*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_vendor      1M    p62*/
  {PART_VBMETA_ODM,                 941*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_odm         1M    p63*/
  {PART_VBMETA_CUST,                942*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_cust        1M    p64*/
  {PART_VBMETA_HW_PRODUCT,          943*1024,           1*1024,        EMMC_USER_PART},/* vbmeta_hw_product  1M    p65*/
  {PART_RECOVERY_VBMETA,            944*1024,           2*1024,        EMMC_USER_PART},/* recovery_vbmeta    2M    p66*/
  {PART_ERECOVERY_VBMETA,           946*1024,           2*1024,        EMMC_USER_PART},/* erecovery_vbmeta   2M    p67*/
  {PART_VBMETA,                     948*1024,           4*1024,        EMMC_USER_PART},/* PART_VBMETA        4M    p68*/
  {PART_MODEMNVM_UPDATE,            952*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_update   16M    p69*/
  {PART_MODEMNVM_CUST,              968*1024,          16*1024,        EMMC_USER_PART},/* modemnvm_cust     16M    p70*/
  {PART_PATCH,                      984*1024,          32*1024,        EMMC_USER_PART},/* patch             32M    p71*/
#ifdef CONFIG_FACTORY_MODE
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,        1280*1024,        EMMC_USER_PART},/* preas           1280M    p73*/
  {PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     2344*1024,        7136*1024,        EMMC_USER_PART},/* super           7152M    p75*/
  {PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_HIBENCH_IMG,              11200*1024,         128*1024,        EMMC_USER_PART},/* hibench_img      128M    p78*/
  {PART_HIBENCH_DATA,             11328*1024,         512*1024,        EMMC_USER_PART},/* hibench_data     512M    p79*/
  {PART_FLASH_AGEING,             11840*1024,         512*1024,        EMMC_USER_PART},/* FLASH_AGEING     512M    p80*/
  {PART_HIBENCH_LPM3,             12352*1024,          32*1024,        EMMC_USER_PART},/* HIBENCH_LPM3     32M     p81*/
  {PART_DDRTEST,                  12384*1024,          48*1024,        EMMC_USER_PART},/* ddrtest           48M    p82*/
  {PART_USERDATA,                 12432*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata           4G    p83*/
#else
#ifdef CONFIG_NEW_PRODUCT_Q
  #ifdef CONFIG_USE_EROFS
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,         360*1024,        EMMC_USER_PART},/* preas            360M    p73*/
  {PART_PREAVS,                    1392*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     1424*1024,        6024*1024,        EMMC_USER_PART},/* super           6024M    p75*/
  {PART_VERSION,                   7448*1024,          32*1024,        EMMC_USER_PART},/* version           32M    p76*/
  {PART_PRELOAD,                   7480*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                  8624*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,        1568*1024,        EMMC_USER_PART},/* preas           1568M    p73*/
  {PART_PREAVS,                    2600*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     2632*1024,        5344*1024,        EMMC_USER_PART},/* super           5344M    p75*/
  {PART_VERSION,                   7976*1024,          32*1024,        EMMC_USER_PART},/* version           32M    p76*/
  {PART_PRELOAD,                   8008*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                  9152*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,         296*1024,        EMMC_USER_PART},/* preas            296M    p73*/
  {PART_PREAVS,                    1328*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     1360*1024,        6362*1024,        EMMC_USER_PART},/* super           6378M    p75*/
  {PART_VERSION,                   7722*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                   8298*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                  9442*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,        1024*1024,        EMMC_USER_PART},/* preas            976M    p73*/
  {PART_PREAVS,                    2056*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     2088*1024,        6338*1024,        EMMC_USER_PART},/* super           6354M    p75*/
  {PART_VERSION,                   8426*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                   9002*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                 10146*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p78*/
  #else
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,        1280*1024,        EMMC_USER_PART},/* preas           1280M    p73*/
  {PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     2344*1024,        7136*1024,        EMMC_USER_PART},/* super           7152M    p75*/
  {PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                 11200*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p78*/
  #endif
  #else /* do not use erofs */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SUPER,                     1016*1024,        6288*1024,        EMMC_USER_PART},/* super           6368M    p72*/
  {PART_VERSION,                   7304*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p73*/
  {PART_PRELOAD,                   7880*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p74*/
  {PART_USERDATA,                  8808*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p75*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SUPER,                     1016*1024,        6952*1024,        EMMC_USER_PART},/* super           7032M    p72*/
  {PART_VERSION,                   7968*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p73*/
  {PART_PRELOAD,                   8544*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p74*/
  {PART_USERDATA,                  9472*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G     p75*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,         368*1024,        EMMC_USER_PART},/* preas            368M    p73*/
  {PART_PREAVS,                    1400*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     1432*1024,        8322*1024,        EMMC_USER_PART},/* super           8338M    p75*/
  {PART_VERSION,                   9754*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  10330*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                 11474*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,        1280*1024,        EMMC_USER_PART},/* preas           1280M    p73*/
  {PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     2344*1024,        8538*1024,        EMMC_USER_PART},/* super           8554M    p75*/
  {PART_VERSION,                  10882*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  11458*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                 12602*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
  #else
  {PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
  {PART_PREAS,                     1032*1024,        1280*1024,        EMMC_USER_PART},/* preas           1280M    p73*/
  {PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
  {PART_SUPER,                     2344*1024,        7136*1024,        EMMC_USER_PART},/* super           7152M    p75*/
  {PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
  {PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
  {PART_USERDATA,                 11200*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p78*/
  #endif
  #endif
#else /*  NOT NEW_PRODUCT_Q */
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,         136*1024,        EMMC_USER_PART},/* preas            136M    p73*/
{PART_PREAVS,                    1168*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     1200*1024,        4376*1024,        EMMC_USER_PART},/* super           4376M    p75*/
{PART_VERSION,                   5576*1024,          56*1024,        EMMC_USER_PART},/* version           56M    p76*/
{PART_PRELOAD,                   5632*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p77*/
{PART_USERDATA,                  6560*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
#elif defined CONFIG_MARKET_OVERSEA
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,         936*1024,        EMMC_USER_PART},/* preas            936M    p73*/
{PART_PREAVS,                    1968*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     2000*1024,        3760*1024,        EMMC_USER_PART},/* super           3760M    p75*/
{PART_VERSION,                   5760*1024,         200*1024,        EMMC_USER_PART},/* version          200M    p76*/
{PART_PRELOAD,                   5960*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p77*/
{PART_USERDATA,                  6888*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
#elif defined CONFIG_MARKET_FULL_INTERNAL
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,         296*1024,        EMMC_USER_PART},/* preas            296M    p73*/
{PART_PREAVS,                    1328*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     1360*1024,        5592*1024,        EMMC_USER_PART},/* super           5592M    p75*/
{PART_VERSION,                   6952*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
{PART_PRELOAD,                   7528*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
{PART_USERDATA,                  8672*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
#elif defined CONFIG_MARKET_FULL_OVERSEA
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,         976*1024,        EMMC_USER_PART},/* preas            976M    p73*/
{PART_PREAVS,                    2008*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     2040*1024,        5512*1024,        EMMC_USER_PART},/* super           5512M    p75*/
{PART_VERSION,                   7552*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
{PART_PRELOAD,                   8128*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
{PART_USERDATA,                  9272*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p78*/
#else
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,        1280*1024,        EMMC_USER_PART},/* preas           1280M    p73*/
{PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     2344*1024,        7136*1024,        EMMC_USER_PART},/* super           7136M    p75*/
{PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
{PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
{PART_USERDATA,                 11200*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p78*/
#endif
#else /* do not use erofs */
#ifdef CONFIG_MARKET_INTERNAL
{PART_SUPER,                     1016*1024,        6288*1024,        EMMC_USER_PART},/* super           6288M    p72*/
{PART_VERSION,                   7304*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p73*/
{PART_PRELOAD,                   7880*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p74*/
{PART_USERDATA,                  8808*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p75*/
#elif defined CONFIG_MARKET_OVERSEA
{PART_SUPER,                     1016*1024,        6952*1024,        EMMC_USER_PART},/* super           6952M    p72*/
{PART_VERSION,                   7968*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p73*/
{PART_PRELOAD,                   8544*1024,         928*1024,        EMMC_USER_PART},/* preload          928M    p74*/
{PART_USERDATA,                  9472*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata          4G     p75*/
#elif defined CONFIG_MARKET_FULL_INTERNAL
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,         360*1024,        EMMC_USER_PART},/* preas            360M    p73*/
{PART_PREAVS,                    1392*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     1424*1024,        7464*1024,        EMMC_USER_PART},/* super           7464M    p75*/
{PART_VERSION,                   8888*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
{PART_PRELOAD,                   9464*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
{PART_USERDATA,                 10608*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
#elif defined CONFIG_MARKET_FULL_OVERSEA
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,        1216*1024,        EMMC_USER_PART},/* preas           1216M    p73*/
{PART_PREAVS,                    2248*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     2280*1024,        7528*1024,        EMMC_USER_PART},/* super           7528M    p75*/
{PART_VERSION,                   9808*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
{PART_PRELOAD,                  10384*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
{PART_USERDATA,                 11528*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata         4G      p78*/
#else
{PART_METADATA,                  1016*1024,          16*1024,        EMMC_USER_PART},/* metadata          16M    p72*/
{PART_PREAS,                     1032*1024,        1280*1024,        EMMC_USER_PART},/* preas           1280M    p73*/
{PART_PREAVS,                    2312*1024,          32*1024,        EMMC_USER_PART},/* preavs            32M    p74*/
{PART_SUPER,                     2344*1024,        7136*1024,        EMMC_USER_PART},/* super           7136M    p75*/
{PART_VERSION,                   9480*1024,         576*1024,        EMMC_USER_PART},/* version          576M    p76*/
{PART_PRELOAD,                  10056*1024,        1144*1024,        EMMC_USER_PART},/* preload         1144M    p77*/
{PART_USERDATA,                 11200*1024,  (4UL)*1024*1024,        EMMC_USER_PART},/* userdata        4G       p78*/
#endif
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
  {PART_RECOVERY,                   566*1024,          53*1024,        UFS_PART_3},/* recovery                   53M    p40*/
  {PART_ERECOVERY,                  619*1024,          53*1024,        UFS_PART_3},/* erecovery                  53M    p41*/
  {PART_ERECOVERY_RAMDISK,          672*1024,          24*1024,        UFS_PART_3},/* erecovery_ramdisk          24M    p42*/
  {PART_ERECOVERY_VENDOR,           696*1024,           8*1024,        UFS_PART_3},/* erecovery_vendor            8M    p43*/
  {PART_BOOT,                       704*1024,          33*1024,        UFS_PART_3},/* boot                       33M    p44*/
  {PART_KPATCH,                     737*1024,           6*1024,        UFS_PART_3},/* kpatch                      6M    p45*/
#else
  {PART_RECOVERY,                   566*1024,          45*1024,        UFS_PART_3},/* recovery                   45M    p40*/
  {PART_ERECOVERY,                  611*1024,          45*1024,        UFS_PART_3},/* erecovery                  45M    p41*/
  {PART_ERECOVERY_RAMDISK,          656*1024,          32*1024,        UFS_PART_3},/* erecovery_ramdisk          32M    p42*/
  {PART_ERECOVERY_VENDOR,           688*1024,          16*1024,        UFS_PART_3},/* erecovery_vendor           16M    p43*/
  {PART_BOOT,                       704*1024,          25*1024,        UFS_PART_3},/* boot                       25M    p44*/
  {PART_KPATCH,                     729*1024,          14*1024,        UFS_PART_3},/* kpatch                     14M    p45*/
#endif
  {PART_ENG_SYSTEM,                 743*1024,          12*1024,        UFS_PART_3},/* eng_system                 12M    p46*/
  {PART_RECOVERY_RAMDISK,           755*1024,          32*1024,        UFS_PART_3},/* recovery_ramdisk           32M    p47*/
  {PART_RECOVERY_VENDOR,            787*1024,          16*1024,        UFS_PART_3},/* recovery_vendor            16M    p48*/
  {PART_DTBO,                       803*1024,          20*1024,        UFS_PART_3},/* dtoimage                   20M    p49*/
  {PART_TRUSTFIRMWARE,              823*1024,           2*1024,        UFS_PART_3},/* trustfirmware               2M    p50*/
  {PART_MODEM_FW,                   825*1024,          56*1024,        UFS_PART_3},/* modem_fw                   56M    p51*/
  {PART_ENG_VENDOR,                 881*1024,          20*1024,        UFS_PART_3},/* eng_vendor                 20M    p52*/
  {PART_MODEM_PATCH_NV,             901*1024,           4*1024,        UFS_PART_3},/* modem_patch_nv              4M    p53*/
  {PART_MODEM_DRIVER,               905*1024,          20*1024,        UFS_PART_3},/* modem_driver               20M    p54*/
  {PART_RESERVED4,                  925*1024,           4*1024,        UFS_PART_3},/* reserved4A                  4M    p55*/
  {PART_RAMDISK,                    929*1024,           2*1024,        UFS_PART_3},/* ramdisk                     2M    p56*/
  {PART_VBMETA_SYSTEM,              931*1024,           1*1024,        UFS_PART_3},/* vbmeta_system               1M    p57*/
  {PART_VBMETA_VENDOR,              932*1024,           1*1024,        UFS_PART_3},/* vbmeta_vendor               1M    p58*/
  {PART_VBMETA_ODM,                 933*1024,           1*1024,        UFS_PART_3},/* vbmeta_odm                  1M    p59*/
  {PART_VBMETA_CUST,                934*1024,           1*1024,        UFS_PART_3},/* vbmeta_cust                 1M    p60*/
  {PART_VBMETA_HW_PRODUCT,          935*1024,           1*1024,        UFS_PART_3},/* vbmeta_hw_product           1M    p61*/
  {PART_RECOVERY_VBMETA,            936*1024,           2*1024,        UFS_PART_3},/* recovery_vbmeta             2M    p62*/
  {PART_ERECOVERY_VBMETA,           938*1024,           2*1024,        UFS_PART_3},/* erecovery_vbmeta            2M    p63*/
  {PART_VBMETA,                     940*1024,           4*1024,        UFS_PART_3},/* vbmeta                      4M    p64*/
  {PART_MODEMNVM_UPDATE,            944*1024,          16*1024,        UFS_PART_3},/* modemnvm_update            16M    p65*/
  {PART_MODEMNVM_CUST,              960*1024,          16*1024,        UFS_PART_3},/* modemnvm_cust              16M    p66*/
  {PART_PATCH,                      976*1024,          32*1024,        UFS_PART_3},/* patch                      32M    p67*/
#ifdef CONFIG_FACTORY_MODE
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,        1280*1024,        UFS_PART_3},/* preas                    1280M    p69*/
  {PART_PREAVS,                    2304*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     2336*1024,        7136*1024,        UFS_PART_3},/* super                    7152M    p71*/
  {PART_VERSION,                   9472*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                  10048*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_HIBENCH_IMG,              11192*1024,         128*1024,        UFS_PART_3},/* hibench_img               128M    p74*/
  {PART_HIBENCH_DATA,             11320*1024,         512*1024,        UFS_PART_3},/* hibench_data              512M    p75*/
  {PART_FLASH_AGEING,             11832*1024,         512*1024,        UFS_PART_3},/* FLASH_AGEING              512M    p76*/
  {PART_HIBENCH_LPM3,             12344*1024,         32*1024,         UFS_PART_3},/* HIBENCH_LPM3               32M    p77*/
  {PART_DDRTEST,                  12376*1024,          48*1024,        UFS_PART_3},/* ddrtest                    48M    p78*/
  {PART_USERDATA,                 12424*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p79*/
#else
#ifdef CONFIG_NEW_PRODUCT_Q
#ifdef CONFIG_USE_EROFS
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,         360*1024,        UFS_PART_3},/* preas                     360M    p69*/
  {PART_PREAVS,                    1384*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     1416*1024,        6024*1024,        UFS_PART_3},/* super                    6024M    p71*/
  {PART_VERSION,                   7440*1024,          32*1024,        UFS_PART_3},/* version                    32M    p72*/
  {PART_PRELOAD,                   7472*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                  8616*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,        1568*1024,        UFS_PART_3},/* preas                    1568M    p69*/
  {PART_PREAVS,                    2592*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     2624*1024,        5344*1024,        UFS_PART_3},/* super                    5344M    p71*/
  {PART_VERSION,                   7968*1024,          32*1024,        UFS_PART_3},/* version                    32M    p72*/
  {PART_PRELOAD,                   8000*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                  9144*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,         296*1024,        UFS_PART_3},/* preas                     296M    p69*/
  {PART_PREAVS,                    1320*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     1352*1024,        6362*1024,        UFS_PART_3},/* super                    6378M    p71*/
  {PART_VERSION,                   7714*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                   8290*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                  9434*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,        1024*1024,        UFS_PART_3},/* preas                    1024M    p69*/
  {PART_PREAVS,                    2048*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     2080*1024,        6338*1024,        UFS_PART_3},/* super                    6354M    p71*/
  {PART_VERSION,                   8418*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                   8994*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                 10138*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #else
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,        1280*1024,        UFS_PART_3},/* preas                    1280M    p69*/
  {PART_PREAVS,                    2304*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     2336*1024,        7136*1024,        UFS_PART_3},/* super                    7152M    p71*/
  {PART_VERSION,                   9472*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                  10048*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                 11192*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p74*/
  #endif
  #else /* do not use erofs */
  #ifdef CONFIG_MARKET_INTERNAL
  {PART_SUPER,                     1008*1024,        6288*1024,         UFS_PART_3},/* super                   6368M    p68*/
  {PART_VERSION,                   7296*1024,         576*1024,         UFS_PART_3},/* version                  576M    p69*/
  {PART_PRELOAD,                   7872*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p70*/
  {PART_USERDATA,                  8800*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p71*/
  #elif defined CONFIG_MARKET_OVERSEA
  {PART_SUPER,                     1008*1024,        6952*1024,         UFS_PART_3},/* super                   7032M    p68*/
  {PART_VERSION,                   7960*1024,         576*1024,         UFS_PART_3},/* version                  576M    p69*/
  {PART_PRELOAD,                   8536*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p70*/
  {PART_USERDATA,                  9464*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p71*/
  #elif defined CONFIG_MARKET_FULL_INTERNAL
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,         368*1024,        UFS_PART_3},/* preas                     368M    p69*/
  {PART_PREAVS,                    1392*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     1424*1024,        8322*1024,        UFS_PART_3},/* super                    8338M    p71*/
  {PART_VERSION,                   9746*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                  10322*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                 11466*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #elif defined CONFIG_MARKET_FULL_OVERSEA
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,        1280*1024,        UFS_PART_3},/* preas                    1280M    p69*/
  {PART_PREAVS,                    2304*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     2336*1024,        8538*1024,        UFS_PART_3},/* super                    8554M    p71*/
  {PART_VERSION,                  10874*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                  11450*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                 12594*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #else
  {PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
  {PART_PREAS,                     1024*1024,        1280*1024,        UFS_PART_3},/* preas                    1280M    p69*/
  {PART_PREAVS,                    2304*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
  {PART_SUPER,                     2336*1024,        7136*1024,        UFS_PART_3},/* super                    7152M    p71*/
  {PART_VERSION,                   9472*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
  {PART_PRELOAD,                  10048*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
  {PART_USERDATA,                 11192*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
  #endif
  #endif
#else  /*  NOT NEW_PRODUCT_Q */
#ifdef CONFIG_USE_EROFS
#ifdef CONFIG_MARKET_INTERNAL
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,         136*1024,        UFS_PART_3},/* preas                     136M    p69*/
{PART_PREAVS,                    1160*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     1192*1024,        4376*1024,        UFS_PART_3},/* super                    4376M    p71*/
{PART_VERSION,                   5568*1024,          56*1024,        UFS_PART_3},/* version                    56M    p72*/
{PART_PRELOAD,                   5624*1024,         928*1024,        UFS_PART_3},/* preload                   928M    p73*/
{PART_USERDATA,                  6552*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#elif defined CONFIG_MARKET_OVERSEA
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,         936*1024,        UFS_PART_3},/* preas                     936M    p69*/
{PART_PREAVS,                    1960*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     1992*1024,        3760*1024,        UFS_PART_3},/* super                    3760M    p71*/
{PART_VERSION,                   5752*1024,         200*1024,        UFS_PART_3},/* version                   200M    p72*/
{PART_PRELOAD,                   5952*1024,         928*1024,        UFS_PART_3},/* preload                   928M    p73*/
{PART_USERDATA,                  6880*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#elif defined CONFIG_MARKET_FULL_INTERNAL
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,         296*1024,        UFS_PART_3},/* preas                     296M    p69*/
{PART_PREAVS,                    1320*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     1352*1024,        5592*1024,        UFS_PART_3},/* super                    5592M    p71*/
{PART_VERSION,                   6944*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
{PART_PRELOAD,                   7520*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
{PART_USERDATA,                  8664*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#elif defined CONFIG_MARKET_FULL_OVERSEA
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,         976*1024,        UFS_PART_3},/* preas                     976M    p69*/
{PART_PREAVS,                    2000*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     2032*1024,        5512*1024,        UFS_PART_3},/* super                    5512M    p71*/
{PART_VERSION,                   7544*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
{PART_PRELOAD,                   8120*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
{PART_USERDATA,                  9264*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#else
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,        1280*1024,        UFS_PART_3},/* preas                    1280M    p69*/
{PART_PREAVS,                    2304*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     2336*1024,        7136*1024,        UFS_PART_3},/* super                    7136M    p71*/
{PART_VERSION,                   9472*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
{PART_PRELOAD,                  10048*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
{PART_USERDATA,                 11192*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p74*/
#endif
#else /* do not use erofs */
#ifdef CONFIG_MARKET_INTERNAL
{PART_SUPER,                     1008*1024,        6288*1024,         UFS_PART_3},/* super                   6288M    p68*/
{PART_VERSION,                   7296*1024,         576*1024,         UFS_PART_3},/* version                  576M    p69*/
{PART_PRELOAD,                   7872*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p70*/
{PART_USERDATA,                  8800*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p71*/
#elif defined CONFIG_MARKET_OVERSEA
{PART_SUPER,                     1008*1024,        6952*1024,         UFS_PART_3},/* super                   6952M    p68*/
{PART_VERSION,                   7960*1024,         576*1024,         UFS_PART_3},/* version                  576M    p69*/
{PART_PRELOAD,                   8536*1024,         928*1024,         UFS_PART_3},/* preload                  928M    p70*/
{PART_USERDATA,                  9464*1024,  (4UL)*1024*1024,         UFS_PART_3},/* userdata                   4G    p71*/
#elif defined CONFIG_MARKET_FULL_INTERNAL
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,         360*1024,        UFS_PART_3},/* preas                     360M    p69*/
{PART_PREAVS,                    1384*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     1416*1024,        7464*1024,        UFS_PART_3},/* super                    7464M    p71*/
{PART_VERSION,                   8880*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
{PART_PRELOAD,                   9456*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
{PART_USERDATA,                 10600*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#elif defined CONFIG_MARKET_FULL_OVERSEA
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,        1216*1024,        UFS_PART_3},/* preas                    1216M    p69*/
{PART_PREAVS,                    2240*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     2272*1024,        7528*1024,        UFS_PART_3},/* super                    7528M    p71*/
{PART_VERSION,                   9800*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
{PART_PRELOAD,                  10376*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
{PART_USERDATA,                 11520*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#else
{PART_METADATA,                  1008*1024,          16*1024,        UFS_PART_3},/* metadata                   16M    p68*/
{PART_PREAS,                     1024*1024,        1280*1024,        UFS_PART_3},/* preas                    1280M    p69*/
{PART_PREAVS,                    2304*1024,          32*1024,        UFS_PART_3},/* preavs                     32M    p70*/
{PART_SUPER,                     2336*1024,        7136*1024,        UFS_PART_3},/* super                    7136M    p71*/
{PART_VERSION,                   9472*1024,         576*1024,        UFS_PART_3},/* version                   576M    p72*/
{PART_PRELOAD,                  10048*1024,        1144*1024,        UFS_PART_3},/* preload                  1144M    p73*/
{PART_USERDATA,                 11192*1024,  (4UL)*1024*1024,        UFS_PART_3},/* userdata                    4G    p74*/
#endif
#endif
#endif
#endif
  {"0", 0, 0, 0},
};
#endif

#endif
