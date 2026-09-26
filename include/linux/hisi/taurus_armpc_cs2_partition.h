#ifndef _TAURUS_ARMPC_CS2_PARTITION_H_
#define _TAURUS_ARMPC_CS2_PARTITION_H_

#include "partition_macro.h"
#include "partition_def.h"

static const struct partition partition_table_emmc[] =
{
  {"0", 0, 0, 0},                                        /* total 11848M*/
};

static struct partition partition_table_ufs[] =
{
  {PART_XLOADER_A,                      0,            2*1024,    UFS_PART_0},
  {PART_XLOADER_B,                      0,            2*1024,    UFS_PART_1},/* xloader_b          2M    p0*/
  {PART_PTABLE,                         0,               512,    UFS_PART_2},/* ptable           512K    p1*/
  {PART_VRL_A,                        512,               512,    UFS_PART_2},/* vrl_a            512K    p2*/
  {PART_VRL_B,                      2*512,               512,    UFS_PART_2},/* vrl_b            512K    p3*/
  {PART_VRL_BACKUP_A,               3*512,               512,    UFS_PART_2},/* vrl backup_a     512K    p4*/
  {PART_VRL_BACKUP_B,               4*512,               512,    UFS_PART_2},/* vrl backup_b     512K    p5*/
  {PART_BOOT_CTRL,                  5*512,               512,    UFS_PART_2},/* boot ctrl        512K    p6*/
  {PART_FW_LPM3_A,                 3*1024,            1*1024,    UFS_PART_2},/* fw_lpm3_a          1M    p7*/
  {PART_FW_LPM3_B,                 4*1024,            1*1024,    UFS_PART_2},/* fw_lpm3_b          1M    p8*/
  {PART_LOWPOWER_PARA_A,           5*1024,            1*1024,    UFS_PART_2},/* lowpower_para_a    1M    p9*/
  {PART_LOWPOWER_PARA_B,           6*1024,            1*1024,    UFS_PART_2},/* lowpower_para_b    1M    p10*/
  {PART_TEEOS_A,                   7*1024,           10*1024,    UFS_PART_2},/* teeos_a           10M    p11*/
  {PART_TEEOS_B,                  17*1024,           10*1024,    UFS_PART_2},/* teeos_b           10M    p12*/
  {PART_FASTBOOT_A,               27*1024,           12*1024,    UFS_PART_2},/* fastboot_a        12M    p13*/
  {PART_FASTBOOT_B,               39*1024,           12*1024,    UFS_PART_2},/* fastboot_b        12M    p14*/
  {PART_DDR_PARA,                 51*1024,            1*1024,    UFS_PART_2},/* DDR_PARA           1M    p15*/
  {PART_CTF,                      52*1024,            1*1024,    UFS_PART_2},/* PART_CTF           1M    p16*/
  {PART_NVME,                     53*1024,            5*1024,    UFS_PART_2},/* nvme               5M    p17*/
  {PART_OEMINFO,                  58*1024,           96*1024,    UFS_PART_2},/* oeminfo           96M    p18*/
  {PART_BL2_A,                   154*1024,            4*1024,    UFS_PART_2},/* bl2_a              4M    p19*/
  {PART_BL2_B,                   158*1024,            4*1024,    UFS_PART_2},/* bl2_b              4M    p20*/
  {PART_HHEE_A,                  162*1024,            6*1024,    UFS_PART_2},/* hhee_a             6M    p21*/
  {PART_HHEE_B,                  168*1024,            6*1024,    UFS_PART_2},/* hhee _b            6M    p22*/
  {PART_FW_DTB_A,                174*1024,            2*1024,    UFS_PART_2},/* fw_dtb_a           2M    p23*/
  {PART_FW_DTB_B,                176*1024,            2*1024,    UFS_PART_2},/* fw_dtb_b           2M    p24*/
  {PART_RECOVERY,                178*1024,          800*1024,    UFS_PART_2},/* recovery         800M    p25*/
  {PART_VECTOR_A,                978*1024,            4*1024,    UFS_PART_2},/* vector_a           4M    p26*/
  {PART_VECTOR_B,                982*1024,            4*1024,    UFS_PART_2},/* vector_b           4M    p27*/
  {PART_DTS_A,                   986*1024,           20*1024,    UFS_PART_2},/* dtsimage_a        20M    p28*/
  {PART_DTS_B,                  1006*1024,           20*1024,    UFS_PART_2},/* dtsimage_b        20M    p29*/
  {PART_SENSORHUB_A,            1026*1024,           16*1024,    UFS_PART_2},/* sensorhub_a       16M    p30*/
  {PART_SENSORHUB_B,            1042*1024,           16*1024,    UFS_PART_2},/* sensorhub_b       16M    p31*/
  {PART_TRUSTFIRMWARE_A,        1058*1024,            2*1024,    UFS_PART_2},/* trustfirmware_a    2M    p32*/
  {PART_TRUSTFIRMWARE_B,        1060*1024,            2*1024,    UFS_PART_2},/* trustfirmware_b    2M    p33*/
  {PART_PTABLE_LU3,                0,                    512,    UFS_PART_3},/* ptable_lu3       512K    p0*/
  {PART_RESERVED2,                 512,                  512,    UFS_PART_3},/* reserved2        512K    p1*/
  {PART_BOOT,                      1*1024,           65*1024,    UFS_PART_3},/* boot              65M    p2*/
  {PART_ROOT,                      66*1024, (32UL)*1024*1024,    UFS_PART_3},/* root              32G    p3*/
  {PART_EFI,                       32834*1024,(1UL)*1024*1024,   UFS_PART_3},/* efi                1G    p4*/
  {PART_DFX,                       33858*1024,        16*1024,   UFS_PART_3},/* dfx               16M    p5*/
#ifdef CONFIG_FACTORY_MODE
  {PART_HIBENCH_IMG,             33874*1024,        128*1024,    UFS_PART_3},/* hibench_img      128M    p6*/
  {PART_HIBENCH_DATA,            34002*1024,        512*1024,    UFS_PART_3},/* hibench_data     512M    p7*/
  {PART_HIBENCH_LOG,             34514*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LOG       32M    p8*/
  {PART_HIBENCH_LPM3,            34546*1024,         32*1024,    UFS_PART_3},/* HIBENCH_LPM3      32M    p9*/
  {PART_KERNEL,                  34578*1024,  (8UL)*1024*1024,   UFS_PART_3},/* kerneldump         8G    p10*/
  {PART_HISEE_FS,                42770*1024,           8*1024,   UFS_PART_3},/* hisee_fs           8M    p11*/
  {PART_HOME,                    42778*1024, (64UL)*1024*1024,   UFS_PART_3},/* home              64G    p12*/
#else
#ifdef CONFIG_PRODUCT_ARMPC_USER
  {PART_HISEE_FS,                33874*1024,           8*1024,   UFS_PART_3},/* hisee_fs           8M    p6*/
  {PART_HOME,                    33882*1024,  (64UL)*1024*1024,  UFS_PART_3},/* home              64G    p7*/
#else
  {PART_KERNEL,                  33874*1024,  (8UL)*1024*1024,   UFS_PART_3},/* kerneldump         8G    p6*/
  {PART_HISEE_FS,                42066*1024,           8*1024,   UFS_PART_3},/* hisee_fs           8M    p7*/
  {PART_HOME,                    42074*1024,  (64UL)*1024*1024,  UFS_PART_3},/* home              64G    p8*/
#endif
#endif
  {"0", 0, 0, 0},
};

#endif
