/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Hisilicon silicon vendor smc define header.
 *              This is follow ARM SMCCC 1.X.
 * Author : security-ap
 * Create : 2019/07/01
 */

/*
 * This is a document file for reference
 *
 * 0xc3000000-0xc300FFFF SMC64: OEM Service Calls
 *
 * 0xc300 0000  ~  0xc300 FFFF +------> sip fid define
 *                        | |
 *                        | |
 *                        | +----------> Fid in module  (0 ~ 255)
 *                        |
 *                        +------------> Module Group   (0 ~ 255)
 *
 * check usage:
 *              FID_USE(fid) == FID_ARF_GROUP
 */

#ifndef __HISI_BL31_SMC_H__
#define __HISI_BL31_SMC_H__

/* Get the group number of FID */
#define FID_BL31_USE(x)                      (((x) & 0xff00) >> 8)

/* The base of OEM Service */
#define OEM_BASE                             0xc3

/* Module Group Number Define */
#define FID_ARF_GROUP                        0x00
#define FID_ISP_GROUP                        0x01
#define FID_IPP_GROUP                        0x02
#define FID_IVP_GROUP                        0x03
#define FID_LB_GROUP                         0x04
#define FID_PINCTRL_GROUP                    0x05
#define FID_RTC_GROUP                        0x06
#define FID_REGU_GROUP                       0x07
#define FID_SPMI_GROUP                       0x08
#define FID_THERMAL_GROUP                    0x09
#define FID_ITS_GROUP                        0x0a
#define FID_HISTDSVC_GROUP                   0x0b
#define FID_HISEE_GROUP                      0x0c
#define FID_EPS_GROUP                        0x0d
#define FID_L3CACHE_GROUP                    0x0e
#define FID_MNTN_GROUP                       0x0f
#define FID_NPU_GROUP                        0x10
#define FID_SECIO_GROUP                      0x11
#define FID_RPMB_GROUP                       0x12
#define FID_FREQDUMP_GROUP                   0x13
#define FID_GETVAR_GROUP                     0x14
#define FID_AVS_GROUP                        0x15
#define FID_BL31LOG_GROUP                    0x16
#define FID_SECTEST_GROUP                    0x17
#define FID_EFUSE_GROUP                      0x18
#define FID_EFUSEV1_GROUP                    0x19
#define FID_EFUSEV2_GROUP                    0x1a
#define FID_DSUPCTRL_GROUP                   0x1b
#define FID_HSDTTRACE_GROUP		     0x1c

#define FID_BL31_MAXGROUP                    0x2f /* max group */

/* Get FID from group and id */
#define MAKE_FID(group, id)                  ((((OEM_BASE << 16) + group) << \
						8) + id)

/*
 * access register
 * 0xc3000000 ~ 0xc30000ff
 */
#define ACCESS_REGISTER_FN_MAIN_ID           MAKE_FID(FID_ARF_GROUP, 0x00)

/*
 * isp:
 * 0xc3000100 ~ 0xc30001ff
 */
#define ISP_FN_SET_PARAMS                    MAKE_FID(FID_ISP_GROUP, 0x00)
#define ISP_FN_PARAMS_DUMP                   MAKE_FID(FID_ISP_GROUP, 0x01)
#define ISP_FN_ISPCPU_NS_DUMP                MAKE_FID(FID_ISP_GROUP, 0x02)
#define ISP_FN_ISP_INIT                      MAKE_FID(FID_ISP_GROUP, 0x03)
#define ISP_FN_ISP_EXIT                      MAKE_FID(FID_ISP_GROUP, 0x04)
#define ISP_FN_ISPCPU_INIT                   MAKE_FID(FID_ISP_GROUP, 0x05)
#define ISP_FN_ISPCPU_EXIT                   MAKE_FID(FID_ISP_GROUP, 0x06)
#define ISP_FN_ISPCPU_MAP                    MAKE_FID(FID_ISP_GROUP, 0x07)
#define ISP_FN_ISPCPU_UNMAP                  MAKE_FID(FID_ISP_GROUP, 0x08)
#define ISP_FN_SET_NONSEC                    MAKE_FID(FID_ISP_GROUP, 0x09)
#define ISP_FN_DISRESET_ISPCPU               MAKE_FID(FID_ISP_GROUP, 0x0a)
#define ISP_FN_ISPSMMU_NS_INIT               MAKE_FID(FID_ISP_GROUP, 0x0b)
#define ISP_FN_GET_ISPCPU_IDLE               MAKE_FID(FID_ISP_GROUP, 0x0c)
#define ISP_FN_ISPTOP_PU                     MAKE_FID(FID_ISP_GROUP, 0x0d)
#define ISP_FN_ISPTOP_PD                     MAKE_FID(FID_ISP_GROUP, 0x0e)
#define ISP_PHY_CSI_CONNECT                  MAKE_FID(FID_ISP_GROUP, 0x0f)
#define ISP_PHY_CSI_DISCONNECT               MAKE_FID(FID_ISP_GROUP, 0x10)
#define ISP_FN_SEND_FIQ_TO_ISPCPU            MAKE_FID(FID_ISP_GROUP, 0x11)

/*
 * hipp
 * 0xc3000200 ~ 0xc30002ff
 */
#define HIPP_FID_VALUE                       MAKE_FID(FID_IPP_GROUP, 0x00)

/*
 * ivp
 * 0xc3000300 ~ 0xc30003ff
 */
#define HISI_IVP_FID_VALUE                   MAKE_FID(FID_IVP_GROUP, 0x00)

/*
 * hisi lb
 * 0xc3000400 ~ 0xc30004ff
 */
#define HISI_LB_FID_VALUE                    MAKE_FID(FID_LB_GROUP, 0x00)

/*
 * hisi pin ctrl
 * 0xc3000500 ~ 0xc300005ff
 */
#define PINCTRL_FID_VALUE                    MAKE_FID(FID_PINCTRL_GROUP, 0x00)

/*
 * hisi trc
 * 0xc3000600 ~ 0xc3000600
 */
#define RTC_FID_VALUE                        MAKE_FID(FID_RTC_GROUP, 0x00)

/*
 * hisi regulator
 * 0xc3000700 ~ 0xc30007ff
 */
#define IP_REGULATOR_FID_VALUE               MAKE_FID(FID_REGU_GROUP, 0x00)

/*
 * hisi spmi
 * 0xc3000800 ~ 0xc30008ff
 */
#define HISI_SPMI_FID_VALUE                  MAKE_FID(FID_SPMI_GROUP, 0x00)

/*
 * hisi thermal
 * 0xc3000900 ~ 0xc30009ff
 */
#define HISI_THERMAL_FID_VALUE               MAKE_FID(FID_THERMAL_GROUP, 0x00)

/*
 * hisi ITS
 * 0xc3000a00 ~ 0xc3000aff
 */
#define HISI_ITS_FID_VALUE                   MAKE_FID(FID_ITS_GROUP, 0x00)

/*
 * hisi std svc call
 * 0xc3000b00 ~ 0xc3000bff
 */
#define STDSVC_CALL_COUNT                    MAKE_FID(FID_HISTDSVC_GROUP, 0x00)
#define STDSVC_UID                           MAKE_FID(FID_HISTDSVC_GROUP, 0x01)
#define STDSVC_VERSION                       MAKE_FID(FID_HISTDSVC_GROUP, 0x02)

/*
 * hisee
 * 0xc3000c00 ~ 0xc3000cff
 */
#define HISEE_FID_VALUE                      MAKE_FID(FID_HISEE_GROUP, 0x01)
#define HISEE_FID_MNTN_VALUE                 MAKE_FID(FID_HISEE_GROUP, 0x02)
#define HISEE_FID_MNTN_MASK                  MAKE_FID(FID_HISEE_GROUP, 0x03)
#define HISEE_TEST_FID_MASK                  MAKE_FID(FID_HISEE_GROUP, 0x04)
#define HISEE_TEST_FID_VALUE                 MAKE_FID(FID_HISEE_GROUP, 0x05)
#define HISEE_FIQ_FID_MASK                   MAKE_FID(FID_HISEE_GROUP, 0x06)
#define HISEE_FIQ_FID_VALUE                  MAKE_FID(FID_HISEE_GROUP, 0x07)

/*
 * hieps
 * 0xc3000d00 ~ 0xc3000dff
 */
#define HIEPS_FID_VALUE                      MAKE_FID(FID_EPS_GROUP, 0x01)
#define HIEPS_KERNEL_FID_MASK                MAKE_FID(FID_EPS_GROUP, 0x02)
#define HIEPS_KERNEL_FID_VALUE               MAKE_FID(FID_EPS_GROUP, 0x03)


/*
 * L3 cache
 * 0xc3000e00 ~ 0xc3000eff
 */
#define HISI_L3SHARE_FID_VALUE               MAKE_FID(FID_L3CACHE_GROUP, 0x01)
#define HISI_L3DEBUG_FID_VALUE               MAKE_FID(FID_L3CACHE_GROUP, 0x02)
#define HISI_L3DEBUG_PMU_FID_VALUE           MAKE_FID(FID_L3CACHE_GROUP, 0x03)
#define HISI_L3ECC_MNTN_FID_VALUE            MAKE_FID(FID_L3CACHE_GROUP, 0x04)

/*
 * hisi mntn
 * 0xc3000f00 ~ 0xc3000fff
 */
#define MNTN_BB_CPUID_FID_VALUE              MAKE_FID(FID_MNTN_GROUP, 0x00)
#define MNTN_BB_BL31INIT_FID_VALUE           MAKE_FID(FID_MNTN_GROUP, 0x01)
#define MNTN_BB_TST_FID_VALUE                MAKE_FID(FID_MNTN_GROUP, 0x02)


/*
 * hisi npu
 * 0xc3001000 ~ 0xc30010ff
 */
#define NPU_FID_VALUE                        MAKE_FID(FID_NPU_GROUP, 0x00)

/*
 * hisi secure io
 * 0xc3001100 ~ 0xc30011ff
 */
#define SECUREIO_FID_VALUE                   MAKE_FID(FID_SECIO_GROUP, 0x01)
#define SECURE_I2C_READ_REG                  MAKE_FID(FID_SECIO_GROUP, 0x02)
#define SECURE_I2C_WRITE_REG                 MAKE_FID(FID_SECIO_GROUP, 0x03)
#define SECURE_I2C_XFER_LOCK                 MAKE_FID(FID_SECIO_GROUP, 0x04)
#define SECURE_I2C_XFER_UNLOCK               MAKE_FID(FID_SECIO_GROUP, 0x05)
#define SECURE_GPIO_READ_REG                 MAKE_FID(FID_SECIO_GROUP, 0x06)
#define SECURE_GPIO_WRITE_REG                MAKE_FID(FID_SECIO_GROUP, 0x07)

/*
 * hisi rpmb
 * 0xc3001200 ~ 0xc30012ff
 */
#define RPMB_SVC_CALL_COUNT                  MAKE_FID(FID_RPMB_GROUP, 0x00)
#define RPMB_SVC_UID                         MAKE_FID(FID_RPMB_GROUP, 0x01)
#define RPMB_SVC_VERSION                     MAKE_FID(FID_RPMB_GROUP, 0x02)
#define RPMB_SVC_TEST                        MAKE_FID(FID_RPMB_GROUP, 0x03)
#define RPMB_SVC_REQUEST_ADDR                MAKE_FID(FID_RPMB_GROUP, 0x04)
#define RPMB_SVC_SET_KEY                     MAKE_FID(FID_RPMB_GROUP, 0x05)
#define RPMB_SVC_REQUEST_DONE                MAKE_FID(FID_RPMB_GROUP, 0x06)
#define RPMB_SVC_MULTI_KEY_STATUS            MAKE_FID(FID_RPMB_GROUP, 0x07)
#define RPMB_SVC_GET_DEV_VER                 MAKE_FID(FID_RPMB_GROUP, 0x08)
/* for secure os */
#define RPMB_SVC_SECURE_OS_INFO              MAKE_FID(FID_RPMB_GROUP, 0x09)
/* for ufs secure */
#define UFS_SVC_REG_SET                      MAKE_FID(FID_RPMB_GROUP, 0x0a)
/* debug only */
#define RPMB_SVC_READ                        MAKE_FID(FID_RPMB_GROUP, 0x0b)
#define RPMB_SVC_WRITE                       MAKE_FID(FID_RPMB_GROUP, 0x0c)
#define RPMB_SVC_COUNTER                     MAKE_FID(FID_RPMB_GROUP, 0x0d)
#define RPMB_SVC_FORMAT                      MAKE_FID(FID_RPMB_GROUP, 0x0e)
#define RPMB_SVC_WRITE_CAPABILITY            MAKE_FID(FID_RPMB_GROUP, 0x0f)
#define RPMB_SVC_READ_CAPABILITY             MAKE_FID(FID_RPMB_GROUP, 0x11)
#define RPMB_SVC_PARTITION                   MAKE_FID(FID_RPMB_GROUP, 0x12)
#define RPMB_SVC_MULTI_KEY                   MAKE_FID(FID_RPMB_GROUP, 0x13)
#define RPMB_SVC_CONFIG_VIEW                 MAKE_FID(FID_RPMB_GROUP, 0x14)


/*
 * hisi freqdump
 * 0xc3001300 ~ 0xc30013ff
 */
#define FREQDUMP_SVC_CALL_COUNT              MAKE_FID(FID_FREQDUMP_GROUP, 0x00)
#define FREQDUMP_SVC_UID                     MAKE_FID(FID_FREQDUMP_GROUP, 0x01)
#define FREQDUMP_SVC_VERSION                 MAKE_FID(FID_FREQDUMP_GROUP, 0x02)
#define FREQDUMP_SVC_REG_RD                  MAKE_FID(FID_FREQDUMP_GROUP, 0x03)
#define FREQDUMP_LOADMONITOR_SVC_REG_RD      MAKE_FID(FID_FREQDUMP_GROUP, 0x04)
#define FREQDUMP_LOADMONITOR_SVC_ENABLE      MAKE_FID(FID_FREQDUMP_GROUP, 0x05)
#define FREQDUMP_LOADMONITOR_SVC_DISABLE     MAKE_FID(FID_FREQDUMP_GROUP, 0x06)
#define FREQDUMP_ADC_SVC_SET_PARAM           MAKE_FID(FID_FREQDUMP_GROUP, 0x07)
#define FREQDUMP_ADC_SVC_DISABLE             MAKE_FID(FID_FREQDUMP_GROUP, 0x08)
#define FREQDUMP_LOADMONITOR_SVC_ENABLE_READ MAKE_FID(FID_FREQDUMP_GROUP, 0x09)
#define MEDIA_MONITOR_SVC_ENABLE             MAKE_FID(FID_FREQDUMP_GROUP, 0x0a)
#define MEDIA_MONITOR_SVC_DISABLE            MAKE_FID(FID_FREQDUMP_GROUP, 0x0b)
#define MEDIA_MONITOR_SVC_SET_FLAG           MAKE_FID(FID_FREQDUMP_GROUP, 0x0c)
#define FREQDUMP_GCOV_INIT_CALL              MAKE_FID(FID_FREQDUMP_GROUP, 0x0d)


/*
 * hisi get var
 * 0xc3001400 ~ 0xc30014ff
 */
#define HISI_GET_VAL_STD_SVC_VER             MAKE_FID(FID_GETVAR_GROUP, 0x00)
#define HISI_GET_VAL_SVC_CALL_COUNT          MAKE_FID(FID_GETVAR_GROUP, 0x01)
#define HISI_GET_VAL_STD_SVC_UID             MAKE_FID(FID_GETVAR_GROUP, 0x02)
#define CPU_VOLT_FID_MASK                    MAKE_FID(FID_GETVAR_GROUP, 0x03)
#define CPU_VOLT_FID_VALUE                   MAKE_FID(FID_GETVAR_GROUP, 0x04)
#define CPU_VOLT_FN_GET_VAL                  MAKE_FID(FID_GETVAR_GROUP, 0x05)
#define BL31_DEBUG_FID_MASK                  MAKE_FID(FID_GETVAR_GROUP, 0x06)
#define BL31_DEBUG_FID_VALUE                 MAKE_FID(FID_GETVAR_GROUP, 0x07)
#define WA_COUNTER_FID_VALUE                 MAKE_FID(FID_GETVAR_GROUP, 0x08)

/*
 * hisi avs
 * 0xc3001500 ~ 0xc30015ff
 */
#define HISI_AVS_STD_SVC_VER                 MAKE_FID(FID_AVS_GROUP, 0x00)
#define HISI_AVS_SVC_CALL_COUNT              MAKE_FID(FID_AVS_GROUP, 0x01)
#define HISI_AVS_SVC_UID                     MAKE_FID(FID_AVS_GROUP, 0x02)
#define HISI_AVS_SVC_VERSION                 MAKE_FID(FID_AVS_GROUP, 0x03)
#define HISI_AVS_NPU_PA_MODE_CFG             MAKE_FID(FID_AVS_GROUP, 0x04)
#define HISI_AVS_NPU_PA_MODE_RECOVERY        MAKE_FID(FID_AVS_GROUP, 0x05)
#define HISI_AVS_NPU_GET_PA_VALUE            MAKE_FID(FID_AVS_GROUP, 0x06)

/*
 * hisi bl31 log
 * 0xc3001600 ~ 0xc30016ff
 */
#define BL31_LOG_FID_VALUE                   MAKE_FID(FID_BL31LOG_GROUP, 0x00)

/*
 * hisi sec ccip
 * 0xc3001700 ~ 0xc30017ff
 */
#define HISI_SEC_PANIC_TEST                  MAKE_FID(FID_SECTEST_GROUP, 0x00)
#define HISI_SEC_CCIP_TEST                   MAKE_FID(FID_SECTEST_GROUP, 0x01)


/*
 * hisi efuse
 * 0xc3001800 ~ 0xc30018ff
 */
#define EFUSE_FN_VERSION                     MAKE_FID(FID_EFUSE_GROUP, 0x01)
#define EFUSE_FN_TEST_WR                     MAKE_FID(FID_EFUSE_GROUP, 0x02)
#define EFUSE_FN_GET_KCE                     MAKE_FID(FID_EFUSE_GROUP, 0x03)
#define EFUSE_FN_SET_DJTAGDEBUG              MAKE_FID(FID_EFUSE_GROUP, 0x04)
#define EFUSE_FN_GET_DJTAGDEBUG              MAKE_FID(FID_EFUSE_GROUP, 0x05)
#define EFUSE_FN_GET_DESKEW                  MAKE_FID(FID_EFUSE_GROUP, 0x06)

/*
 * hisi efuse v1
 * 0xc3001900 ~ 0xc30019ff
 */
#define EFUSE_FN_GET_DIEID                   MAKE_FID(FID_EFUSEV1_GROUP, 0x00)
#define EFUSE_FN_GET_CHIPID                  MAKE_FID(FID_EFUSEV1_GROUP, 0x01)
#define EFUSE_FN_SET_CHIPID                  MAKE_FID(FID_EFUSEV1_GROUP, 0x02)
#define EFUSE_FN_GET_AUTHKEY                 MAKE_FID(FID_EFUSEV1_GROUP, 0x03)
#define EFUSE_FN_SET_AUTHKEY                 MAKE_FID(FID_EFUSEV1_GROUP, 0x04)
#define EFUSE_FN_GET_SECURITYDEBUG           MAKE_FID(FID_EFUSEV1_GROUP, 0x05)
#define EFUSE_FN_SET_SECURITYDEBUG           MAKE_FID(FID_EFUSEV1_GROUP, 0x06)
#define EFUSE_FN_GET_THERMAL                 MAKE_FID(FID_EFUSEV1_GROUP, 0x07)
#define EFUSE_FN_TEST_DISPLAY                MAKE_FID(FID_EFUSEV1_GROUP, 0x08)
#define EFUSE_FN_SET_KCE                     MAKE_FID(FID_EFUSEV1_GROUP, 0x09)
#define EFUSE_FN_GET_CARRIERID               MAKE_FID(FID_EFUSEV1_GROUP, 0x0a)
#define EFUSE_FN_GET_FREQ                    MAKE_FID(FID_EFUSEV1_GROUP, 0x0b)
#define EFUSE_FN_GET_SLTFINISHFLAG           MAKE_FID(FID_EFUSEV1_GROUP, 0x0c)
#define EFUSE_FN_SET_SLTFINISHFLAG           MAKE_FID(FID_EFUSEV1_GROUP, 0x0d)
#define EFUSE_FN_SET_HISEE                   MAKE_FID(FID_EFUSEV1_GROUP, 0x0e)
#define EFUSE_FN_GET_HISEE                   MAKE_FID(FID_EFUSEV1_GROUP, 0x0f)
#define EFUSE_FN_GET_AVS                     MAKE_FID(FID_EFUSEV1_GROUP, 0x10)

/*
 * hisi efuse v2
 * 0xc3001a00 ~ 0xc3001aff
 */
#define EFUSE_FN_WR_MODEM_HUK_VALUE          MAKE_FID(FID_EFUSEV2_GROUP, 0x00)
#define EFUSE_FN_RD_MODEM_HUK_VALUE          MAKE_FID(FID_EFUSEV2_GROUP, 0x01)
#define EFUSE_FN_RD_DIEID_VALUE              MAKE_FID(FID_EFUSEV2_GROUP, 0x02)
#define EFUSE_FN_WR_DIEID_VALUE              MAKE_FID(FID_EFUSEV2_GROUP, 0x03)
#define EFUSE_FN_RD_CHIPID_VALUE             MAKE_FID(FID_EFUSEV2_GROUP, 0x04)
#define EFUSE_FN_WR_CHIPID_VALUE             MAKE_FID(FID_EFUSEV2_GROUP, 0x05)
#define EFUSE_FN_RD_AUTH_KEY_VALUE           MAKE_FID(FID_EFUSEV2_GROUP, 0x06)
#define EFUSE_FN_WR_AUTH_KEY_VALUE           MAKE_FID(FID_EFUSEV2_GROUP, 0x07)
#define EFUSE_FN_ENABLE_DEBUG                MAKE_FID(FID_EFUSEV2_GROUP, 0x08)
#define EFUSE_FN_DISABLE_DEBUG               MAKE_FID(FID_EFUSEV2_GROUP, 0x09)

/*
 * dsu pctrl
 * 0xc3001b00 ~ 0xc3001bff
 */
#define HISI_DSU_PCTRL_FID_VALUE             MAKE_FID(FID_DSUPCTRL_GROUP, 0x00)

/*
 * hsdt trace
 * 0xc3001c00 ~ 0xc3001cff
 */
#define HSDT_TRACE_FID_VALUE             MAKE_FID(FID_HSDTTRACE_GROUP, 0x00)


#endif
