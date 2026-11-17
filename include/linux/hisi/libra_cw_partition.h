/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: partition table
 * Create: 2020-03-10
 */

#ifndef _LIBRA_CW_PARTITION_H_
#define _LIBRA_CW_PARTITION_H_

#include "partition_macro.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
    {PART_XLOADER,             0,                2 * 1024,    EMMC_BOOT_MAJOR_PART},
    {PART_RESERVED0,           0,                2 * 1024,    EMMC_BOOT_BACKUP_PART},
    {PART_PTABLE,              0,                     512,    EMMC_USER_PART}, /* ptable           512K */
    {PART_FRP,                 512,                   512,    EMMC_USER_PART}, /* frp              512K    p1 */
    {PART_PERSIST,             1024,                 2048,    EMMC_USER_PART}, /* persist         2048K    p2 */
    {PART_RESERVED1,           3072,                 5120,    EMMC_USER_PART}, /* reserved1       5120K    p3 */
    {PART_RESERVED6,           8 * 1024,              512,    EMMC_USER_PART}, /* reserved6        512K    p4 */
    {PART_VRL,                 8704,                  512,    EMMC_USER_PART}, /* vrl              512K    p5 */
    {PART_VRL_BACKUP,          9216,                  512,    EMMC_USER_PART}, /* vrl_backup       512K    p6 */
    {PART_MODEM_SECURE,        9728,                 8704,    EMMC_USER_PART}, /* modem_secure    8704k    p7 */
    {PART_NVME,                18 * 1024,        5 * 1024,    EMMC_USER_PART}, /* nvme               5M    p8 */
    {PART_CTF,                 23 * 1024,        1 * 1024,    EMMC_USER_PART}, /* certification      1M    p9 */
    {PART_OEMINFO,             24 * 1024,       64 * 1024,    EMMC_USER_PART}, /* oeminfo           64M    p10 */
    {PART_SECURE_STORAGE,      88 * 1024,       32 * 1024,    EMMC_USER_PART}, /* secure_storage    32M    p11 */
    {PART_MODEM_OM,           120 * 1024,        8 * 1024,    EMMC_USER_PART}, /* modem_om           8M    p12 */
    {PART_MODEMNVM_FACTORY,   128 * 1024,       16 * 1024,    EMMC_USER_PART}, /* modemnvm_factory  16M    p13 */
    {PART_MODEMNVM_BACKUP,    144 * 1024,       16 * 1024,    EMMC_USER_PART}, /* modemnvm_backup   16M    p14 */
    {PART_MODEMNVM_IMG,       160 * 1024,       34 * 1024,    EMMC_USER_PART}, /* modemnvm_img      34M    p15 */
    {PART_RESERVED7,          194 * 1024,        2 * 1024,    EMMC_USER_PART}, /* reserved7          2M    p16 */
    {PART_VERITYKEY,          196 * 1024,        1 * 1024,    EMMC_USER_PART}, /* veritykey          1M    p17 */
    {PART_DDR_PARA,           197 * 1024,        1 * 1024,    EMMC_USER_PART}, /* ddr_para           1M    p18 */
    {PART_RAMDISK,            198 * 1024,        2 * 1024,    EMMC_USER_PART}, /* ramdisk           32M    p19 */
    {PART_VBMETA_SYSTEM,      200 * 1024,        1 * 1024,    EMMC_USER_PART}, /* vbmeta_system      1M    p20 */
    {PART_VBMETA_VENDOR,      201 * 1024,        1 * 1024,    EMMC_USER_PART}, /* vbmeta_vendor      1M    p21 */
    {PART_VBMETA_ODM,         202 * 1024,        1 * 1024,    EMMC_USER_PART}, /* vbmeta_odm         1M    p22 */
    {PART_VBMETA_CUST,        203 * 1024,        1 * 1024,    EMMC_USER_PART}, /* vbmeta_cust        1M    p23 */
    {PART_VBMETA_HW_PRODUCT,  204 * 1024,        1 * 1024,    EMMC_USER_PART}, /* vbmeta_hw_product  1M    p24 */
    {PART_SPLASH2,            205 * 1024,       80 * 1024,    EMMC_USER_PART}, /* splash2           80M    p25 */
    {PART_BOOTFAIL_INFO,      285 * 1024,        2 * 1024,    EMMC_USER_PART}, /* bootfail_info     2MB    p26 */
    {PART_MISC,               287 * 1024,        2 * 1024,    EMMC_USER_PART}, /* misc               2M    p27 */
    {PART_DFX,                289 * 1024,       16 * 1024,    EMMC_USER_PART}, /* dfx               16M    p28 */
    {PART_RRECORD,            305 * 1024,       16 * 1024,    EMMC_USER_PART}, /* rrecord           16M    p29 */
    {PART_CACHE,              321 * 1024,      104 * 1024,    EMMC_USER_PART}, /* cache            104M    p30 */
    {PART_FW_LPM3,            425 * 1024,             256,    EMMC_USER_PART}, /* fw_lpm3          256K    p31 */
    {PART_KPATCH,             435456,                3840,    EMMC_USER_PART}, /* kpatch          3840K    p32 */
    {PART_HHEE,               429 * 1024,        4 * 1024,    EMMC_USER_PART}, /* hhee               4M    p33 */
    {PART_FASTBOOT,           433 * 1024,       12 * 1024,    EMMC_USER_PART}, /* fastboot          12M    p34 */
    {PART_VECTOR,             445 * 1024,        1 * 1024,    EMMC_USER_PART}, /* vector             1M    p35 */
    {PART_ISP_BOOT,           446 * 1024,        2 * 1024,    EMMC_USER_PART}, /* isp_boot           2M    p36 */
    {PART_ISP_FIRMWARE,       448 * 1024,       14 * 1024,    EMMC_USER_PART}, /* isp_firmware      14M    p37 */
    {PART_FW_HIFI,            462 * 1024,       12 * 1024,    EMMC_USER_PART}, /* fw_hifi           12M    p38 */
    {PART_TEEOS,              474 * 1024,        8 * 1024,    EMMC_USER_PART}, /* teeos              8M    p39 */
    {PART_SENSORHUB,          482 * 1024,       16 * 1024,    EMMC_USER_PART}, /* sensorhub         16M    p40 */
    {PART_EXT_SENSORHUB,      498 * 1024,      300 * 1024,    EMMC_USER_PART}, /* ext_sensorhub    300M    p41 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK,  798 * 1024,       32 * 1024,    EMMC_USER_PART}, /* erecovery_ramdisk 32M    p42 */
    {PART_ERECOVERY_VENDOR,   830 * 1024,        8 * 1024,    EMMC_USER_PART}, /* erecovery_vendor   8M    p43 */
    {PART_BOOT,               838 * 1024,       40 * 1024,    EMMC_USER_PART}, /* boot              40M    p44 */
    {PART_RECOVERY,           878 * 1024,       64 * 1024,    EMMC_USER_PART}, /* recovery          64M    p45 */
    {PART_ERECOVERY,          942 * 1024,       64 * 1024,    EMMC_USER_PART}, /* erecovery         64M    p46 */
    {PART_RESERVED4,         1006 * 1024,       74 * 1024,    EMMC_USER_PART}, /* reserved4         74M    p47 */
#else
    {PART_ERECOVERY_RAMDISK,  798 * 1024,       32 * 1024,    EMMC_USER_PART}, /* erecovery_ramdisk 32M    p42 */
    {PART_ERECOVERY_VENDOR,   830 * 1024,       16 * 1024,    EMMC_USER_PART}, /* erecovery_vendor  16M    p43 */
    {PART_BOOT,               846 * 1024,       32 * 1024,    EMMC_USER_PART}, /* boot              32M    p44 */
    {PART_RECOVERY,           878 * 1024,       56 * 1024,    EMMC_USER_PART}, /* recovery          56M    p45 */
    {PART_ERECOVERY,          934 * 1024,       56 * 1024,    EMMC_USER_PART}, /* erecovery         56M    p46 */
    {PART_RESERVED4,          990 * 1024,       90 * 1024,    EMMC_USER_PART}, /* reserved4         90M    p47 */
#endif
    {PART_BL2,               1080 * 1024,        4 * 1024,    EMMC_USER_PART}, /* bl2                4M    p48 */
    {PART_METADATA,          1084 * 1024,       16 * 1024,    EMMC_USER_PART}, /* metadata          16M    p49 */
    {PART_ENG_SYSTEM,        1100 * 1024,       12 * 1024,    EMMC_USER_PART}, /* eng_system        12M    p50 */
    {PART_RECOVERY_RAMDISK,  1112 * 1024,       32 * 1024,    EMMC_USER_PART}, /* recovery_ramdisk  32M    p51 */
    {PART_RECOVERY_VENDOR,   1144 * 1024,       16 * 1024,    EMMC_USER_PART}, /* recovery_vendor   16M    p52 */
    {PART_FW_DTB,            1160 * 1024,        2 * 1024,    EMMC_USER_PART}, /* fw_dtb             2M    p53 */
    {PART_DTBO,              1162 * 1024,       24 * 1024,    EMMC_USER_PART}, /* dtbo              24M    p54 */
    {PART_TRUSTFIRMWARE,     1186 * 1024,        2 * 1024,    EMMC_USER_PART}, /* trustfirmware      2M    p55 */
    {PART_MODEM_FW,          1188 * 1024,       56 * 1024,    EMMC_USER_PART}, /* modem_fw          56M    p56 */
    {PART_ENG_VENDOR,        1244 * 1024,       12 * 1024,    EMMC_USER_PART}, /* eng_vendor        12M    p57 */
    {PART_RECOVERY_VBMETA,   1256 * 1024,        2 * 1024,    EMMC_USER_PART}, /* recovery_vbmeta    2M    p58 */
    {PART_ERECOVERY_VBMETA,  1258 * 1024,        2 * 1024,    EMMC_USER_PART}, /* erecovery_vbmeta   2M    p59 */
    {PART_VBMETA,            1260 * 1024,        4 * 1024,    EMMC_USER_PART}, /* vbmeta             4M    p60 */
    {PART_MODEMNVM_UPDATE,   1264 * 1024,       16 * 1024,    EMMC_USER_PART}, /* modemnvm_update   16M    p61 */
    {PART_MODEMNVM_CUST,     1280 * 1024,       16 * 1024,    EMMC_USER_PART}, /* modemnvm_cust     16M    p62 */
    {PART_PATCH,             1296 * 1024,       32 * 1024,    EMMC_USER_PART}, /* patch             32M    p63 */
#ifdef CONFIG_NEW_PRODUCT_P /* Distinguish Kids watches and smart watches */
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,             1328 * 1024,       96 * 1024,    EMMC_USER_PART}, /* preas             96M    p64 */
    {PART_PREAVS,            1424 * 1024,       32 * 1024,    EMMC_USER_PART}, /* preavs            32M    p65 */
    {PART_SUPER,             1456 * 1024,     2592 * 1024,    EMMC_USER_PART}, /* super           2592M    p66 */
    {PART_VERSION,           4048 * 1024,      200 * 1024,    EMMC_USER_PART}, /* version          200M    p67 */
    {PART_PRELOAD,           4248 * 1024,      520 * 1024,    EMMC_USER_PART}, /* preload          520M    p68 */
    {PART_USERDATA,          4768 * 1024,     4096 * 1024,    EMMC_USER_PART}, /* userdata        4096M    p69 */
    {PART_HIBENCH_IMG,       8864 * 1024,      128 * 1024,    EMMC_USER_PART}, /* hibench_img      128M    p70 */
    {PART_HIBENCH_DATA,      8992 * 1024,      512 * 1024,    EMMC_USER_PART}, /* hibench_data     512M    p71 */
    {PART_FLASH_AGEING,      9504 * 1024,      512 * 1024,    EMMC_USER_PART}, /* flash_ageing     512M    p72 */
    {PART_HIBENCH_LOG,      10016 * 1024,       32 * 1024,    EMMC_USER_PART}, /* hibench_log       32M    p73 */
#else /* for release version */
    {PART_PREAS,             1328 * 1024,       96 * 1024,    EMMC_USER_PART}, /* preas             96M    p64 */
    {PART_PREAVS,            1424 * 1024,       32 * 1024,    EMMC_USER_PART}, /* preavs            32M    p65 */
    {PART_SUPER,             1456 * 1024,     2592 * 1024,    EMMC_USER_PART}, /* super           2592M    p66 */
    {PART_VERSION,           4048 * 1024,      200 * 1024,    EMMC_USER_PART}, /* version          200M    p67 */
    {PART_PRELOAD,           4248 * 1024,      520 * 1024,    EMMC_USER_PART}, /* preload          520M    p68 */
    {PART_USERDATA,          4768 * 1024, (4UL) * 1024 * 1024,    EMMC_USER_PART}, /* userdata       4G    p69 */
#endif /* CONFIG_FACTORY_MODE */
#else /* use ext4 for Hisi, Kids watches etc */
    {PART_PREAS,             1328 * 1024,       96 * 1024,    EMMC_USER_PART}, /* preas             96M    p64 */
    {PART_PREAVS,            1424 * 1024,       32 * 1024,    EMMC_USER_PART}, /* preavs            32M    p65 */
    {PART_SUPER,             1456 * 1024,     7000 * 1024,    EMMC_USER_PART}, /* super           7000M    p66 */
    {PART_VERSION,           8456 * 1024,      200 * 1024,    EMMC_USER_PART}, /* version          200M    p67 */
    {PART_PRELOAD,           8656 * 1024,      520 * 1024,    EMMC_USER_PART}, /* preload          520M    p68 */
#ifdef CONFIG_FACTORY_MODE
    {PART_USERDATA,          9176 * 1024,     4096 * 1024,    EMMC_USER_PART}, /* userdata        4096M    p69 */
    {PART_HIBENCH_IMG,      13272 * 1024,      128 * 1024,    EMMC_USER_PART}, /* hibench_img      128M    p70 */
    {PART_HIBENCH_DATA,     13400 * 1024,      512 * 1024,    EMMC_USER_PART}, /* hibench_data     512M    p71 */
    {PART_FLASH_AGEING,     13912 * 1024,      512 * 1024,    EMMC_USER_PART}, /* flash_ageing     512M    p72 */
    {PART_HIBENCH_LOG,      14424 * 1024,       32 * 1024,    EMMC_USER_PART}, /* hibench_log       32M    p73 */
#else
    {PART_USERDATA,          9176 * 1024,     4096 * 1024,    EMMC_USER_PART}, /* userdata        4096M    p69 */
#endif
#endif /* CONFIG_NEW_PRODUCT_P */
    {"0", 0, 0, 0},
};

static const struct partition partition_table_ufs[] =
{
    {PART_XLOADER,                    0,         2 * 1024,    UFS_PART_0},
    {PART_RESERVED0,                  0,         2 * 1024,    UFS_PART_1},
    {PART_PTABLE,                     0,              512,    UFS_PART_2}, /* ptable              512K */
    {PART_FRP,                      512,              512,    UFS_PART_2}, /* frp                 512K    p1 */
    {PART_PERSIST,                 1024,             2048,    UFS_PART_2}, /* persist            2048K    p2 */
    {PART_RESERVED1,               3072,             5120,    UFS_PART_2}, /* reserved1          5120K    p3 */
    {PART_PTABLE_LU3,                 0,              512,    UFS_PART_3}, /* ptable_lu3          512K    p0 */
    {PART_VRL,                      512,              512,    UFS_PART_3}, /* vrl                 512K    p1 */
    {PART_VRL_BACKUP,              1024,              512,    UFS_PART_3}, /* vrl_backup          512K    p2 */
    {PART_MODEM_SECURE,            1536,             8704,    UFS_PART_3}, /* modem_secure       8704k    p3 */
    {PART_NVME,               10 * 1024,         5 * 1024,    UFS_PART_3}, /* nvme                  5M    p4 */
    {PART_CTF,                15 * 1024,         1 * 1024,    UFS_PART_3}, /* certification         1M    p5 */
    {PART_OEMINFO,            16 * 1024,        64 * 1024,    UFS_PART_3}, /* oeminfo              64M    p6 */
    {PART_SECURE_STORAGE,     80 * 1024,        32 * 1024,    UFS_PART_3}, /* secure_storage       32M    p7 */
    {PART_MODEM_OM,          112 * 1024,         8 * 1024,    UFS_PART_3}, /* modem_om              8M    p8 */
    {PART_MODEMNVM_FACTORY,  120 * 1024,        16 * 1024,    UFS_PART_3}, /* modemnvm_factory     16M    p9 */
    {PART_MODEMNVM_BACKUP,   136 * 1024,        16 * 1024,    UFS_PART_3}, /* modemnvm_backup      16M    p10 */
    {PART_MODEMNVM_IMG,      152 * 1024,        34 * 1024,    UFS_PART_3}, /* modemnvm_img         34M    p11 */
    {PART_RESERVED7,         186 * 1024,         2 * 1024,    UFS_PART_3}, /* reserved7             2M    p12 */
    {PART_VERITYKEY,         188 * 1024,         1 * 1024,    UFS_PART_3}, /* veritykey             1M    p13 */
    {PART_DDR_PARA,          189 * 1024,         1 * 1024,    UFS_PART_3}, /* ddr_para              1M    p14 */
    {PART_RAMDISK,           190 * 1024,         2 * 1024,    UFS_PART_3}, /* ramdisk               2M    p15 */
    {PART_VBMETA_SYSTEM,     192 * 1024,         1 * 1024,    UFS_PART_3}, /* vbmeta_system         1M    p16 */
    {PART_VBMETA_VENDOR,     193 * 1024,         1 * 1024,    UFS_PART_3}, /* vbmeta_vendor         1M    p17 */
    {PART_VBMETA_ODM,        194 * 1024,         1 * 1024,    UFS_PART_3}, /* vbmeta_odm            1M    p18 */
    {PART_VBMETA_CUST,       195 * 1024,         1 * 1024,    UFS_PART_3}, /* vbmeta_cust           1M    p19 */
    {PART_VBMETA_HW_PRODUCT, 196 * 1024,         1 * 1024,    UFS_PART_3}, /* vbmeta_hw_product     1M    p20 */
    {PART_SPLASH2,           197 * 1024,        80 * 1024,    UFS_PART_3}, /* splash2              80M    p21 */
    {PART_BOOTFAIL_INFO,     277 * 1024,         2 * 1024,    UFS_PART_3}, /* bootfail_info        2MB    p22 */
    {PART_MISC,              279 * 1024,         2 * 1024,    UFS_PART_3}, /* misc                  2M    p23 */
    {PART_DFX,               281 * 1024,        16 * 1024,    UFS_PART_3}, /* dfx                  16M    p24 */
    {PART_RRECORD,           297 * 1024,        16 * 1024,    UFS_PART_3}, /* rrecord              16M    p25 */
    {PART_CACHE,             313 * 1024,       104 * 1024,    UFS_PART_3}, /* cache               104M    p26 */
    {PART_FW_LPM3,           417 * 1024,              256,    UFS_PART_3}, /* fw_lpm3             256K    p27 */
    {PART_KPATCH,                427264,             3840,    UFS_PART_3}, /* kpatch             3840K    p28 */
    {PART_HHEE,              421 * 1024,         4 * 1024,    UFS_PART_3}, /* hhee                  4M    p29 */
    {PART_FASTBOOT,          425 * 1024,        12 * 1024,    UFS_PART_3}, /* fastboot             12M    p30 */
    {PART_VECTOR,            437 * 1024,         1 * 1024,    UFS_PART_3}, /* vector                1M    p31 */
    {PART_ISP_BOOT,          438 * 1024,         2 * 1024,    UFS_PART_3}, /* isp_boot              2M    p32 */
    {PART_ISP_FIRMWARE,      440 * 1024,        14 * 1024,    UFS_PART_3}, /* isp_firmware         14M    p33 */
    {PART_FW_HIFI,           454 * 1024,        12 * 1024,    UFS_PART_3}, /* fw_hifi              12M    p34 */
    {PART_TEEOS,             466 * 1024,         8 * 1024,    UFS_PART_3}, /* teeos                 8M    p35 */
    {PART_SENSORHUB,         474 * 1024,        16 * 1024,    UFS_PART_3}, /* sensorhub            16M    p36 */
    {PART_EXT_SENSORHUB,     490 * 1024,       300 * 1024,    UFS_PART_3}, /* ext_sensorhub       300M    p37 */
#ifdef CONFIG_SANITIZER_ENABLE
    {PART_ERECOVERY_RAMDISK, 790 * 1024,        32 * 1024,    UFS_PART_3}, /* erecovery_ramdisk    32M    p38 */
    {PART_ERECOVERY_VENDOR,  822 * 1024,         8 * 1024,    UFS_PART_3}, /* erecovery_vendor      8M    p39 */
    {PART_BOOT,              830 * 1024,        40 * 1024,    UFS_PART_3}, /* boot                 40M    p40 */
    {PART_RECOVERY,          870 * 1024,        64 * 1024,    UFS_PART_3}, /* recovery             64M    p41 */
    {PART_ERECOVERY,         934 * 1024,        64 * 1024,    UFS_PART_3}, /* erecovery            64M    p42 */
    {PART_RESERVED4,         998 * 1024,        74 * 1024,    UFS_PART_3}, /* reserved4            74M    p43 */
#else
    {PART_ERECOVERY_RAMDISK, 790 * 1024,        32 * 1024,    UFS_PART_3}, /* erecovery_ramdisk    32M    p38 */
    {PART_ERECOVERY_VENDOR,  822 * 1024,        16 * 1024,    UFS_PART_3}, /* erecovery_vendor     16M    p39 */
    {PART_BOOT,              838 * 1024,        32 * 1024,    UFS_PART_3}, /* boot                 32M    p40 */
    {PART_RECOVERY,          870 * 1024,        56 * 1024,    UFS_PART_3}, /* recovery             56M    p41 */
    {PART_ERECOVERY,         926 * 1024,        56 * 1024,    UFS_PART_3}, /* erecovery            56M    p42 */
    {PART_RESERVED4,         982 * 1024,        90 * 1024,    UFS_PART_3}, /* reserved4            90M    p43 */
#endif
    {PART_BL2,              1072 * 1024,         4 * 1024,    UFS_PART_3}, /* bl2                   4M    p44 */
    {PART_METADATA,         1076 * 1024,        16 * 1024,    UFS_PART_3}, /* metadata             16M    p45 */
    {PART_ENG_SYSTEM,       1092 * 1024,        12 * 1024,    UFS_PART_3}, /* eng_system           12M    p46 */
    {PART_RECOVERY_RAMDISK, 1104 * 1024,        32 * 1024,    UFS_PART_3}, /* recovery_ramdisk     32M    p47 */
    {PART_RECOVERY_VENDOR,  1136 * 1024,        16 * 1024,    UFS_PART_3}, /* recovery_vendor      16M    p48 */
    {PART_FW_DTB,           1152 * 1024,         2 * 1024,    UFS_PART_3}, /* fw_dtb                2M    p49 */
    {PART_DTBO,             1154 * 1024,        24 * 1024,    UFS_PART_3}, /* dtbo                 24M    p50 */
    {PART_TRUSTFIRMWARE,    1178 * 1024,         2 * 1024,    UFS_PART_3}, /* trustfirmware         2M    p51 */
    {PART_MODEM_FW,         1180 * 1024,        56 * 1024,    UFS_PART_3}, /* modem_fw             56M    p52 */
    {PART_ENG_VENDOR,       1236 * 1024,        12 * 1024,    UFS_PART_3}, /* eng_vendor           12M    p53 */
    {PART_RECOVERY_VBMETA,  1248 * 1024,         2 * 1024,    UFS_PART_3}, /* recovery_vbmeta       2M    p54 */
    {PART_ERECOVERY_VBMETA, 1250 * 1024,         2 * 1024,    UFS_PART_3}, /* erecovery_vbmeta      2M    p55 */
    {PART_VBMETA,           1252 * 1024,         4 * 1024,    UFS_PART_3}, /* vbmeta                4M    p56 */
    {PART_MODEMNVM_UPDATE,  1256 * 1024,        16 * 1024,    UFS_PART_3}, /* modemnvm_update      16M    p57 */
    {PART_MODEMNVM_CUST,    1272 * 1024,        16 * 1024,    UFS_PART_3}, /* modemnvm_cust        16M    p58 */
    {PART_PATCH,            1288 * 1024,        32 * 1024,    UFS_PART_3}, /* patch                32M    p59 */
#ifdef CONFIG_NEW_PRODUCT_P
#ifdef CONFIG_FACTORY_MODE
    {PART_PREAS,            1320 * 1024,        96 * 1024,    UFS_PART_3}, /* preas                96M    p60 */
    {PART_PREAVS,           1416 * 1024,        32 * 1024,    UFS_PART_3}, /* preavs               32M    p61 */
    {PART_SUPER,            1448 * 1024,      2592 * 1024,    UFS_PART_3}, /* super              2592M    p62 */
    {PART_VERSION,          4040 * 1024,       200 * 1024,    UFS_PART_3}, /* version             200M    p63 */
    {PART_PRELOAD,          4240 * 1024,       520 * 1024,    UFS_PART_3}, /* preload             520M    p64 */
    {PART_USERDATA,         4760 * 1024,      4096 * 1024,    UFS_PART_3}, /* userdata           4096M    p65 */
    {PART_HIBENCH_IMG,      8856 * 1024,       128 * 1024,    UFS_PART_3}, /* hibench_img         128M    p66 */
    {PART_HIBENCH_DATA,     8984 * 1024,       512 * 1024,    UFS_PART_3}, /* hibench_data        512M    p67 */
    {PART_FLASH_AGEING,     9496 * 1024,       512 * 1024,    UFS_PART_3}, /* flash_ageing        512M    p68 */
    {PART_HIBENCH_LOG,     10008 * 1024,        32 * 1024,    UFS_PART_3}, /* HIBENCH_LOG          32M    p69 */
#else /* for release version */
    {PART_PREAS,            1320 * 1024,        96 * 1024,    UFS_PART_3}, /* preas                96M    p60 */
    {PART_PREAVS,           1416 * 1024,        32 * 1024,    UFS_PART_3}, /* preavs               32M    p61 */
    {PART_SUPER,            1448 * 1024,      2592 * 1024,    UFS_PART_3}, /* super              2592M    p62 */
    {PART_VERSION,          4040 * 1024,       200 * 1024,    UFS_PART_3}, /* version             200M    p63 */
    {PART_PRELOAD,          4240 * 1024,       520 * 1024,    UFS_PART_3}, /* preload             520M    p64 */
    {PART_USERDATA,         4760 * 1024, (4UL) * 1024 * 1024,    UFS_PART_3}, /* userdata           4G    p65 */
#endif /* CONFIG_FACTORY_MODE */
#else /* use ext4 for Hisi, Kids watches etc */
    {PART_PREAS,            1320 * 1024,        96 * 1024,    UFS_PART_3}, /* preas                96M    p60 */
    {PART_PREAVS,           1416 * 1024,        32 * 1024,    UFS_PART_3}, /* preavs               32M    p61 */
    {PART_SUPER,            1448 * 1024,      7000 * 1024,    UFS_PART_3}, /* super              7000M    p62 */
    {PART_VERSION,          8448 * 1024,       200 * 1024,    UFS_PART_3}, /* version             200M    p63 */
    {PART_PRELOAD,          8648 * 1024,       520 * 1024,    UFS_PART_3}, /* preload             520M    p64 */
#ifdef CONFIG_FACTORY_MODE
    {PART_USERDATA,         9168 * 1024,      4096 * 1024,    UFS_PART_3}, /* userdata           4096M    p65 */
    {PART_HIBENCH_IMG,     13264 * 1024,       128 * 1024,    UFS_PART_3}, /* hibench_img         128M    p66 */
    {PART_HIBENCH_DATA,    13392 * 1024,       512 * 1024,    UFS_PART_3}, /* hibench_data        512M    p67 */
    {PART_FLASH_AGEING,    13904 * 1024,       512 * 1024,    UFS_PART_3}, /* flash_ageing        512M    p68 */
    {PART_HIBENCH_LOG,     14416 * 1024,        32 * 1024,    UFS_PART_3}, /* HIBENCH_LOG          32M    p69 */
#else
    {PART_USERDATA,         9168 * 1024, (4UL) * 1024 * 1024,    UFS_PART_3}, /* userdata        4096M    p65 */
#endif
#endif /* CONFIG_NEW_PRODUCT_P */
    {"0", 0, 0, 0},
};
#endif
