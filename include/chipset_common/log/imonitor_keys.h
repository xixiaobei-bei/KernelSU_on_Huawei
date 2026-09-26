/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2020. All rights reserved.
 * Description: The class is event param id constant as provided for external used
 * Author: shenchenkai
 * Create: 2017-12-14
 */

#ifndef IMONITOR_KEYS_H
#define IMONITOR_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Reliability|901002003|TrustedZone:Tee OS Crash|Process Name */
#define E901002003_PNAME_VARCHAR 0
/* Reliability|901002003|TrustedZone:Tee OS Crash|1st fuction in stack */
#define E901002003_F1NAME_VARCHAR 1
/* Reliability|901002003|TrustedZone:Tee OS Crash|Interrupt Time Type */
#define E901002003_INTTIMETYPE_VARCHAR 2

/* Reliability|901002004|Hisee Crash|Process Name */
#define E901002004_PNAME_VARCHAR 0
/* Reliability|901002004|Hisee Crash|1st fuction in stack */
#define E901002004_F1NAME_VARCHAR 1
/* Reliability|901002004|Hisee Crash|Interrupt Time Type */
#define E901002004_INTTIMETYPE_VARCHAR 2

/* Reliability|901003002|Memory Limit|Process Name */
#define E901003002_PNAME_VARCHAR 0
/* Reliability|901003002|Memory Limit|APP Version */
#define E901003002_APPVERSION_VARCHAR 1
/* Reliability|901003002|Memory Limit|APP Type */
#define E901003002_APPTYPE_VARCHAR 2

/* ExtendedDMD|936005000|Statistics of drop|Type of drop */
#define E936005000_TYPE_TINYINT 0
/* ExtendedDMD|936005000|Statistics of drop|Initial speed of dorp */
#define E936005000_INITSPEED_INT 1
/* ExtendedDMD|936005000|Statistics of drop|Height of drop */
#define E936005000_HEIGHT_INT 2
/* ExtendedDMD|936005000|Statistics of drop|Pitch angle of the drop */
#define E936005000_PITCH_INT 3
/* ExtendedDMD|936005000|Statistics of drop|Roll angle of the drop */
#define E936005000_ROLL_INT 4
/* ExtendedDMD|936005000|Statistics of drop|Yaw angle of the drop */
#define E936005000_YAW_INT 5
/* ExtendedDMD|936005000|Statistics of drop|Material of ground */
#define E936005000_MATERIAL_TINYINT 6
/* ExtendedDMD|936005000|Statistics of drop|Touch ground speed of drop */
#define E936005000_SPEED_INT 7
/* ExtendedDMD|936005000|Statistics of drop|Protecting case of phone */
#define E936005000_SHELL_TINYINT 8
/* ExtendedDMD|936005000|Statistics of drop|Screen protecter of phone */
#define E936005000_FILM_TINYINT 9

/* GPS|910000007|Gps error event, report diferent subclass according to each errorcode|Error Code */
#define E910000007_ERRORCODE_TINYINT 1
/* GPS|910000007|Gps error event, report diferent subclass according to each errorcode|Assert log messages */
#define E910000007_CHIPSETCRASH_CLASS 13

/* GPS|910009014|Collect chipset crash messages|Assert log messages */
#define E910009014_ASSERT_VARCHAR 1

/* Fingerprint|912000004|FingerPrint touch gestures|FingerPrint down time */
#define E912000004_FPDOW_T_VARCHAR 1
/* Fingerprint|912000004|FingerPrint touch gestures|Vendor information */
#define E912000004_SEN_INF_VARCHAR 12

/* Storage|914004000|The info for fs trimming action|The space of the userdata partition */
#define E914004000_TOTAL_SPACE_INT 0
/* Storage|914004000|The info for fs trimming action|The free space of the userdata partition */
#define E914004000_FREE_SPACE_INT 1
/* Storage|914004000|The info for fs trimming action|All the trimmed space */
#define E914004000_TRIMMED_SPACE_INT 2
/* Storage|914004000|The info for fs trimming action|The delta time of the trimming */
#define E914004000_TRIM_DELTA_INT 3
/* Storage|914004000|The info for fs trimming action|The fs virtual space after trimming */
#define E914004000_FS_VIRTUALSPACE_AFTERTRIM_INT 4
/* Storage|914004000|The info for fs trimming action|The fs virtual space trimmed */
#define E914004000_FS_VIRTUALSPACE_TRIMMED_INT 5
/* Storage|914004000|The info for fs trimming action|The failed number of trimming */
#define E914004000_FAILED_NUM_SMALLINT 6
/* Storage|914004000|The info for fs trimming action|The success number of trimming */
#define E914004000_SUCC_NUM_SMALLINT 7
/* Storage|914004000|The info for fs trimming action|The information of the flash vendor */
#define E914004000_CID_VARCHAR 8
/* Storage|914004000|The info for fs trimming action|The triggered schedule of trimming */
#define E914004000_S_ID_INT 9

/* Storage|914006000|The vendor info for ufs device|Total host read data */
#define E914006000_HOST_TOTAL_READ_INT 0
/* Storage|914006000|The vendor info for ufs device|Total host write data */
#define E914006000_HOST_TOTAL_WRITE_INT 1
/* Storage|914006000|The vendor info for ufs device|Min ec number of A space */
#define E914006000_MIN_EC_NUM_A_INT 2
/* Storage|914006000|The vendor info for ufs device|Max ec number of A space */
#define E914006000_MAX_EC_NUM_A_INT 3
/* Storage|914006000|The vendor info for ufs device|Ave ec number of A space */
#define E914006000_AVE_EC_NUM_A_INT 4
/* Storage|914006000|The vendor info for ufs device|Min ec number of B space */
#define E914006000_MIN_EC_NUM_B_INT 5
/* Storage|914006000|The vendor info for ufs device|Max ec number of B space */
#define E914006000_MAX_EC_NUM_B_INT 6
/* Storage|914006000|The vendor info for ufs device|Ave ec number of B space */
#define E914006000_AVE_EC_NUM_B_INT 7
/* Storage|914006000|The vendor info for ufs device|Ave ec number of enhance space */
#define E914006000_AVE_EC_NUM_ENHANCE_INT 8
/* Storage|914006000|The vendor info for ufs device|Original bad block number */
#define E914006000_ORIGIN_BAD_BLOCK_NUM_INT 9
/* Storage|914006000|The vendor info for ufs device|Run bad block number */
#define E914006000_RUN_BAD_BLOCK_NUM_INT 10
/* Storage|914006000|The vendor info for ufs device|Spare block number */
#define E914006000_SPARE_BLOCK_NUM_SMALLINT 11
/* Storage|914006000|The vendor info for ufs device|Meta data corruption */
#define E914006000_META_DATA_CORRUPTION_INT 12
/* Storage|914006000|The vendor info for ufs device|Cumulative iNitialization count */
#define E914006000_INIT_COUNT_INT 13
/* Storage|914006000|The vendor info for ufs device|Number of failures recover new host data after Power Loss */
#define E914006000_SPOR_WRITE_FAIL_NUM_INT 14
/* Storage|914006000|The vendor info for ufs device|Total Recovery Operations After Voltage Droop */
#define E914006000_SPOR_RECOVERY_NUM_INT 15
/* Storage|914006000|The vendor info for ufs device|Ecc error */
#define E914006000_UNCERR_INT 16
/* Storage|914006000|The vendor info for ufs device|Reserved block slc */
#define E914006000_RESERVED_BLOCK_NUM_SLC_INT 17
/* Storage|914006000|The vendor info for ufs device|Reserved block mlc */
#define E914006000_RESERVED_BLOCK_NUM_MLC_INT 18
/* Storage|914006000|The vendor info for ufs device|Read retry number */
#define E914006000_READ_RETRY_NUM_INT 19
/* Storage|914006000|The vendor info for ufs device|Read reclaim number oc slc */
#define E914006000_READ_RECLAIM_NUM_SLC_INT 20
/* Storage|914006000|The vendor info for ufs device|Read reclaim number oc mlc */
#define E914006000_READ_RECLAIM_NUM_MLC_INT 21
/* Storage|914006000|The vendor info for ufs device|Read reclaim number oc enhance */
#define E914006000_READ_RECLAIM_NUM_ENHANCE_INT 22
/* Storage|914006000|The vendor info for ufs device|Vdt number */
#define E914006000_VDT_NUM_INT 23
/* Storage|914006000|The vendor info for ufs device|Expect life left */
#define E914006000_EXPECTLIFE_LEFT_INT 24
/* Storage|914006000|The vendor info for ufs device|Used life */
#define E914006000_USED_LIFE_SMALLINT 25
/* Storage|914006000|The vendor info for ufs device|Alarm level */
#define E914006000_ALARM_LEVEL_SMALLINT 26
/* Storage|914006000|The vendor info for ufs device|Alarm origin */
#define E914006000_ALARM_ORIGIN_INT 27
/* Storage|914006000|The vendor info for ufs device|Writeamplification of device */
#define E914006000_WRITE_AMPLIFICATION_SMALLINT 28

/* Storage|914007000|The criteria info for ufs device|All left space */
#define E914007000_ALL_LEFT_SPACE_INT 0
/* Storage|914007000|The criteria info for ufs device|bpreEOL info */
#define E914007000_BPREEOL_INFO_SMALLINT 1
/* Storage|914007000|The criteria info for ufs device|bDeviceLifeTimeEstA */
#define E914007000_BDEVICELIFE_TIME_A_SMALLINT 2
/* Storage|914007000|The criteria info for ufs device|bDeviceLifeTimeEstB */
#define E914007000_BDEVICELIFE_TIME_B_SMALLINT 3
/* Storage|914007000|The criteria info for ufs device|Used days */
#define E914007000_USED_DAYS_INT 4
/* Storage|914007000|The criteria info for ufs device|Vendor id of device */
#define E914007000_VENDOR_ID_INT 5
/* Storage|914007000|The criteria info for ufs device|Product name of device */
#define E914007000_PRODUCT_NAME_VARCHAR 6

/* Storage|914008000|The vfs info|Total app read data */
#define E914008000_APP_TOTAL_READ_INT 0
/* Storage|914008000|The vfs info|Total app write data */
#define E914008000_APP_TOTAL_WRITE_INT 1
/* Storage|914008000|The vfs info|Total app fg read data */
#define E914008000_APP_TOTAL_READ_FG_INT 2
/* Storage|914008000|The vfs info|Total app fg write data */
#define E914008000_APP_TOTAL_WRITE_FG_INT 3
/* Storage|914008000|The vfs info|Total app bg read data */
#define E914008000_APP_TOTAL_READ_BG_INT 4
/* Storage|914008000|The vfs info|Total app bg write data */
#define E914008000_APP_TOTAL_WRITE_BG_INT 5

/* Storage|914009000|The block info|Sd card exist */
#define E914009000_SD_CARD_EXIST_SMALLINT 0
/* Storage|914009000|The block info|Data part free space */
#define E914009000_DATA_PART_FREE_SPACE_INT 1
/* Storage|914009000|The block info|Total read data in blk */
#define E914009000_HOST_TOTAL_READ_INT 2
/* Storage|914009000|The block info|Total write data in blk */
#define E914009000_HOST_TOTAL_WRITE_INT 3
/* Storage|914009000|The block info|4k read delay in five */
#define E914009000_READ_DELAY_4K_FIVE_INT 4
/* Storage|914009000|The block info|4k write delay in five */
#define E914009000_WRITE_DELAY_4K_FIVE_INT 5
/* Storage|914009000|The block info|4k read delay in four */
#define E914009000_READ_DELAY_4K_FOUR_INT 6
/* Storage|914009000|The block info|4k write delay in four */
#define E914009000_WRITE_DELAY_4K_FOUR_INT 7
/* Storage|914009000|The block info|4k read delay in three */
#define E914009000_READ_DELAY_4K_THREE_INT 8
/* Storage|914009000|The block info|4k write delay in three */
#define E914009000_WRITE_DELAY_4K_THREE_INT 9
/* Storage|914009000|The block info|4k read delay in two */
#define E914009000_READ_DELAY_4K_TWO_INT 10
/* Storage|914009000|The block info|4k write delay in two */
#define E914009000_WRITE_DELAY_4K_TWO_INT 11
/* Storage|914009000|The block info|4k read delay in one */
#define E914009000_READ_DELAY_4K_ONE_INT 12
/* Storage|914009000|The block info|4k write delay in one */
#define E914009000_WRITE_DELAY_4K_ONE_INT 13
/* Storage|914009000|The block info|4k read delay in five */
#define E914009000_READ_DELAY_512K_FIVE_INT 14
/* Storage|914009000|The block info|512k write delay in five */
#define E914009000_WRITE_DELAY_512K_FIVE_INT 15
/* Storage|914009000|The block info|512k read delay in four */
#define E914009000_READ_DELAY_512K_FOUR_INT 16
/* Storage|914009000|The block info|512k write delay in four */
#define E914009000_WRITE_DELAY_512K_FOUR_INT 17
/* Storage|914009000|The block info|512k read delay in three */
#define E914009000_READ_DELAY_512K_THREE_INT 18
/* Storage|914009000|The block info|512k write delay in three */
#define E914009000_WRITE_DELAY_512K_THREE_INT 19
/* Storage|914009000|The block info|512k read delay in two */
#define E914009000_READ_DELAY_512K_TWO_INT 20
/* Storage|914009000|The block info|512k write delay in two */
#define E914009000_WRITE_DELAY_512K_TWO_INT 21
/* Storage|914009000|The block info|512k read delay in one */
#define E914009000_READ_DELAY_512K_ONE_INT 22
/* Storage|914009000|The block info|512k write delay in one */
#define E914009000_WRITE_DELAY_512K_ONE_INT 23
/* Storage|914009000|The block info|4k average read delay */
#define E914009000_READ_DELAY_4K_AVERAGE_INT 24
/* Storage|914009000|The block info|4k average write delay */
#define E914009000_WRITE_DELAY_4K_AVERAGE_INT 25
/* Storage|914009000|The block info|512k average read delay */
#define E914009000_READ_DELAY_512K_AVERAGE_INT 26
/* Storage|914009000|The block info|512k average write delay */
#define E914009000_WRITE_DELAY_512K_AVERAGE_INT 27
/* Storage|914009000|The block info|Min read delay */
#define E914009000_READ_DELAY_MIN_INT 28
/* Storage|914009000|The block info|Min write delay */
#define E914009000_WRITE_DELAY_MIN_INT 29
/* Storage|914009000|The block info|Max read delay */
#define E914009000_READ_DELAY_MAX_INT 30
/* Storage|914009000|The block info|Max read delay */
#define E914009000_WRITE_DELAY_MAX_INT 31
/* Storage|914009000|The block info|Running read io number */
#define E914009000_READ_RUN_IO_NUM_INT 32
/* Storage|914009000|The block info|Running write io number */
#define E914009000_WRITE_RUN_IO_NUM_INT 33
/* Storage|914009000|The block info|Running read io sectors */
#define E914009000_READ_RUN_IO_SECTOR_INT 34
/* Storage|914009000|The block info|Running write io sectors */
#define E914009000_WRITE_RUN_IO_SECTOR_INT 35
/* Storage|914009000|The block info|Running read io ticks */
#define E914009000_READ_RUN_IO_TICKS_INT 36
/* Storage|914009000|The block info|Running write io ticks */
#define E914009000_WRITE_RUN_IO_TICKS_INT 37
/* Storage|914009000|The block info|Running read io flight */
#define E914009000_READ_RUN_IO_FLIGHT_INT 38
/* Storage|914009000|The block info|Running write io flight */
#define E914009000_WRITE_RUN_IO_FLIGHT_INT 39
/* Storage|914009000|The block info|Read iops */
#define E914009000_READ_IOPS_INT 40
/* Storage|914009000|The block info|Write iops */
#define E914009000_WRITE_IOPS_INT 41
/* Storage|914009000|The block info|Read bandith */
#define E914009000_READ_BANDITH_INT 42
/* Storage|914009000|The block info|Write bandith */
#define E914009000_WRITE_BANDITH_INT 43

/* Storage|914012000|The f2fs storage base info|File system size */
#define E914012000_FS_INT 0
/* Storage|914012000|The f2fs storage base info|Free segment count */
#define E914012000_FR_INT 1
/* Storage|914012000|The f2fs storage base info|Reserved segment count */
#define E914012000_RV_INT 2
/* Storage|914012000|The f2fs storage base info|Real size occupied by filesystem(valid blocks and undiscard blocks) */
#define E914012000_RE_INT 3
/* Storage|914012000|The f2fs storage base info|Invalid block count */
#define E914012000_IB_INT 4

/* Storage|914012001|The fg/bg gc info|Bg gc number */
#define E914012001_BG_INT 0
/* Storage|914012001|The fg/bg gc info|Fg gc number */
#define E914012001_FG_INT 1
/* Storage|914012001|The fg/bg gc info|Bg gc fail number */
#define E914012001_BGF_INT 2
/* Storage|914012001|The fg/bg gc info|Fg gc fail number */
#define E914012001_FGF_INT 3
/* Storage|914012001|The fg/bg gc info|Bg gc move data segments average number */
#define E914012001_BDS_INT 4
/* Storage|914012001|The fg/bg gc info|Bg gc move data blocks average number */
#define E914012001_BDB_INT 5
/* Storage|914012001|The fg/bg gc info|Bg gc move node segments average number */
#define E914012001_BNS_INT 6
/* Storage|914012001|The fg/bg gc info|Bg gc move node blocks average number */
#define E914012001_BNB_INT 7
/* Storage|914012001|The fg/bg gc info|Fg gc move data segments a average number */
#define E914012001_FDS_INT 8
/* Storage|914012001|The fg/bg gc info|Fg gc move data blocks average number */
#define E914012001_FDB_INT 9
/* Storage|914012001|The fg/bg gc info|Fg gc move node segments average number */
#define E914012001_FNS_INT 10
/* Storage|914012001|The fg/bg gc info|Fg gc move node blocks average number */
#define E914012001_FNB_INT 11
/* Storage|914012001|The fg/bg gc info|Fg gc average time */
#define E914012001_FGT_INT 12
/* Storage|914012001|The fg/bg gc info|Ssr node count */
#define E914012001_NS_INT 13
/* Storage|914012001|The fg/bg gc info|Ssr data count */
#define E914012001_DS_INT 14
/* Storage|914012001|The fg/bg gc info|Lfs node count */
#define E914012001_NL_INT 15
/* Storage|914012001|The fg/bg gc info|Lfs data count */
#define E914012001_DL_INT 16
/* Storage|914012001|The fg/bg gc info|Ipu data count */
#define E914012001_DI_INT 17

/* Storage|914012002|The fsync info|Fsync regular file count */
#define E914012002_FF_INT 0
/* Storage|914012002|The fsync info|Fsync directory count */
#define E914012002_FD_INT 1
/* Storage|914012002|The fsync info|Fsync average time */
#define E914012002_AFT_INT 2
/* Storage|914012002|The fsync info|Fsync max time */
#define E914012002_MFT_INT 3
/* Storage|914012002|The fsync info|Fsync write file data average time */
#define E914012002_AFFT_INT 4
/* Storage|914012002|The fsync info|Fsync write file data max time */
#define E914012002_MFFT_INT 5
/* Storage|914012002|The fsync info|Fsync write cp average time */
#define E914012002_AFCT_INT 6
/* Storage|914012002|The fsync info|Fsync write cp max time */
#define E914012002_MFCT_INT 7
/* Storage|914012002|The fsync info|Fsync sync node average time */
#define E914012002_AFNT_INT 8
/* Storage|914012002|The fsync info|Fsync sync node max time */
#define E914012002_MFNT_INT 9
/* Storage|914012002|The fsync info|Fsync flush average time */
#define E914012002_AFUT_INT 10
/* Storage|914012002|The fsync info|Fsync flush max time */
#define E914012002_MFUT_INT 11

/* Storage|914012003|The discard info|Issued discard count */
#define E914012003_CNT_INT 0
/* Storage|914012003|The discard info|Undiscard count */
#define E914012003_UCNT_INT 1
/* Storage|914012003|The discard info|Discard block count */
#define E914012003_DB_INT 2
/* Storage|914012003|The discard info|Average execute time of discard */
#define E914012003_ADT_INT 3
/* Storage|914012003|The discard info|Max execute time of discard */
#define E914012003_MDT_INT 4
/* Storage|914012003|The discard info|Undiscard block count */
#define E914012003_UDB_INT 5

/* Storage|914012004|The cp info|Total CP count */
#define E914012004_CNT_INT 0
/* Storage|914012004|The cp info|Successful CP count */
#define E914012004_SUCCCNT_INT 1
/* Storage|914012004|The cp info|Average execute time of CP */
#define E914012004_ACT_INT 2
/* Storage|914012004|The cp info|Max execute time of CP */
#define E914012004_MCT_INT 3
/* Storage|914012004|The cp info|Max submit time in CP */
#define E914012004_CST_INT 4
/* Storage|914012004|The cp info|Max flush meta time in CP */
#define E914012004_CMT_INT 5
/* Storage|914012004|The cp info|Max discard time in CP */
#define E914012004_CDT_INT 6

/* Storage|914012005|The encrypt info|Encrypt corruption */
#define E914012005_EC_BIT 0
/* Storage|914012005|The encrypt info|Encrypt fixed count */
#define E914012005_EX_BIT 1
/* Storage|914012005|The encrypt info|Corrupted original encrypt info count */
#define E914012005_ECO_BIT 2
/* Storage|914012005|The encrypt info|No original encrypt info count */
#define E914012005_ENO_BIT 3
/* Storage|914012005|The encrypt info|Corrupted backup encrypt info count */
#define E914012005_ECB_BIT 4
/* Storage|914012005|The encrypt info|No backup encrypt info count */
#define E914012005_ENB_BIT 5
/* Storage|914012005|The encrypt info|Corrupted crc count */
#define E914012005_ECC_BIT 6
/* Storage|914012005|The encrypt info|No crc count */
#define E914012005_ENC_BIT 7

/* Storage|914012006|The hotcold info|Hot data IO count */
#define E914012006_HD_INT 0
/* Storage|914012006|The hotcold info|Warm data IO count */
#define E914012006_WD_INT 1
/* Storage|914012006|The hotcold info|Cold data IO count */
#define E914012006_CD_INT 2
/* Storage|914012006|The hotcold info|Hot node IO count */
#define E914012006_HN_INT 3
/* Storage|914012006|The hotcold info|Warm node IO count */
#define E914012006_WN_INT 4
/* Storage|914012006|The hotcold info|Cold node IO count */
#define E914012006_CN_INT 5
/* Storage|914012006|The hotcold info|Meta cp IO count */
#define E914012006_MC_INT 6
/* Storage|914012006|The hotcold info|Meta sit IO count */
#define E914012006_MSI_INT 7
/* Storage|914012006|The hotcold info|Meta nat IO count */
#define E914012006_MN_INT 8
/* Storage|914012006|The hotcold info|Meta ssa IO count */
#define E914012006_MSS_INT 9
/* Storage|914012006|The hotcold info|DirectIO IO count */
#define E914012006_DIO_INT 10
/* Storage|914012006|The hotcold info|GC allocated cold data IO count */
#define E914012006_GCCD_INT 11
/* Storage|914012006|The hotcold info|Rewrite hot data IO count */
#define E914012006_RHD_INT 12
/* Storage|914012006|The hotcold info|Rewrite warm data IO count */
#define E914012006_RWD_INT 13
/* Storage|914012006|The hotcold info|GC victim target hot data segment count */
#define E914012006_HDS_INT 14
/* Storage|914012006|The hotcold info|GC victim target warm data segment count */
#define E914012006_WDS_INT 15
/* Storage|914012006|The hotcold info|GC victim target cold data segment count */
#define E914012006_CDS_INT 16
/* Storage|914012006|The hotcold info|GC victim target hot node segment count */
#define E914012006_HNS_INT 17
/* Storage|914012006|The hotcold info|GC victim target warm node segment count */
#define E914012006_WNS_INT 18
/* Storage|914012006|The hotcold info|GC victim target cold node segment count */
#define E914012006_CNS_INT 19
/* Storage|914012006|The hotcold info|GC moved hot data IO count */
#define E914012006_HDB_INT 20
/* Storage|914012006|The hotcold info|GC moved warm data IO count */
#define E914012006_WDB_INT 21
/* Storage|914012006|The hotcold info|GC moved cold data IO count */
#define E914012006_CDB_INT 22
/* Storage|914012006|The hotcold info|GC moved hot node IO count */
#define E914012006_HNB_INT 23
/* Storage|914012006|The hotcold info|GC moved warm node IO count */
#define E914012006_WNB_INT 24
/* Storage|914012006|The hotcold info|GC moved cold node IO count */
#define E914012006_CNB_INT 25

/* Audio|916000001|Howling Error|Event Level */
#define E916000001_EVENTLEVEL_INT 0
/* Audio|916000001|Howling Error|Event Level */
#define E916000001_EVENTMODULE_VARCHAR 1

/* Audio|916000004|Active-Time Ratio(%)|Probability */
#define E916000004_PROBABILITY_TINYINT 2

/* Audio|916000005|ANC Processing/Path Error|Cause Case */
#define E916000005_CAUSECASE_VARCHAR 2

/* Audio|916000101|Proc Error|Event Level */
#define E916000101_EVENTLEVEL_INT 0
/* Audio|916000101|Proc Error|Event Module */
#define E916000101_EVENTMODULE_VARCHAR 1
/* Audio|916000101|Proc Error|Description */
#define E916000101_MAXRDC_INT 2
/* Audio|916000101|Proc Error|MAX temperature */
#define E916000101_MINRDC_INT 3
/* Audio|916000101|Proc Error|MAX temperature */
#define E916000101_MAXTEMP_INT 4
/* Audio|916000101|Proc Error|MIN temperature */
#define E916000101_MINTEMP_INT 5
/* Audio|916000101|Proc Error|AVG temperature */
#define E916000101_AVGTEMP_INT 6
/* Audio|916000101|Proc Error|MAX amplitude */
#define E916000101_MAXAMP_INT 7
/* Audio|916000101|Proc Error|MIN amplitude */
#define E916000101_MINAMP_INT 8
/* Audio|916000101|Proc Error|Err code */
#define E916000101_ERRCODE_INT 9

/* Audio|916000102|ParaSet Error|Err position Tag */
#define E916000102_ERRPOSTAG_VARCHAR 9

/* Audio|916200001|handset|noise 0 level number */
#define E916200001_NOISECNT0_TINYINT 0
/* Audio|916200002|handfree|noise 0 level number */
#define E916200002_NOISECNT0_TINYINT 0
/* Audio|916200003|headset|noise 0 level number */
#define E916200003_NOISECNT0_TINYINT 0
/* Audio|916200004|bluetooth|noise 0 level number */
#define E916200004_NOISECNT0_TINYINT 0

/* Security|940000002|HWHF_PATCH_ERROR|Error type */
#define E940000002_ERRTYPE_INT 0
/* Security|940000002|HWHF_PATCH_ERROR|Error description */
#define E940000002_ERRDESC_VARCHAR 1

/* Security|940000012|SELINUX_AUDIT_INFO|Avc denied key */
#define E940000012_DENIEDKEY_VARCHAR 0
/* Security|940000012|SELINUX_AUDIT_INFO|Module name */
#define E940000012_MODNAME_VARCHAR 1
/* Security|940000012|SELINUX_AUDIT_INFO|Error description */
#define E940000012_ERRDESC_VARCHAR 2

/* Security|940000014|OEMINFO_KEY_ID_ILLEGAL_OPER|Denied key word */
#define E940000014_DENIEDKEY_INT 0
/* Security|940000014|OEMINFO_KEY_ID_ILLEGAL_OPER|Module name */
#define E940000014_MODNAME_VARCHAR 1
/* Security|940000014|OEMINFO_KEY_ID_ILLEGAL_OPER|Error description */
#define E940000014_ERRDESC_VARCHAR 2

#ifdef __cplusplus
}
#endif

#endif /* IMONITOR_KEYS_H */