/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2021. All rights reserved.
 * Description: HKIP hhee support
 * Date: 2017/08/15
 */

#ifndef __HKIP_HHEE_H__
#define __HKIP_HHEE_H__

/* HHEE general communication and test driver */
#include <linux/arm-smccc.h>

#define HHEE_LKM_UPDATE                 0xC6001082
#define ARM_STD_HVC_VERSION             0x8400ff03

/* Defines for HHEE HVC function ids */
#define HHEE_HVC_VERSION                0xC9000000
#define HHEE_HVC_NOTIFY                 0xC9000001

#define HKIP_HVC_ROWM_SET_BIT           0xC6001050

#define HKIP_HVC_XO_REGISTER            0xC6001060
#define HKIP_HVC_XO_UNREGISTER          0xC6001061

#define HHEE_HVC_PAGERD_COUNT           0xC6001087
#define HHEE_HVC_LIVEPATCH              0xC6001088
#define HHEE_HVC_TOKEN                  0xC6001089
#define HHEE_HVC_ENABLE_TVM             0xC600108A
#define HHEE_HVC_DISABLE_LIVEPATCH      0xC600108B

#define HHEE_HVC_ROX_TEXT_REGISTER      0xC6001090
#define HHEE_HVC_VMMU_ENABLE            0xC6001091

#define HHEE_RETURN_STAGE2_INFO         0xC60010FE

/* Defines for el2 register read */
#define HHEE_HCR_EL2                    0xC9000002
#define HHEE_VTTBR_EL2                  0xC9000003

/* Defines for security level settings */
#define HHEE_PROT_LVL                   0xC9000004
#define HHEE_PERMISIVE                  0xC9000005
#define HHEE_TEXT_BOUNDARIES            0xC9000006
#define HHEE_RET_TO_USER                0xC9000007

/* Defines for logging functionality */
#define HHEE_INIT_LOGBUF                0xC9000008
#define HHEE_LOGBUF_INFO                0xC9000009
#define HHEE_CRASHLOG_INFO              0xC900000A
#define HHEE_MONITORLOG_INFO            0xC900000B
#define HHEE_MONITORLOG_RESET_COUNTERS  0xc900000C
#define HHEE_RETURN_FUNCTION_ADDRESS    0xc900000D
#define HHEE_RETURN_VAR_ADDRESS         0xc900000E
#define HHEE_IRQ_INFOBUF                0xC900000F
#define HHEE_MSG_HANDLE_GET             0xC9000010
#define HHEE_PMFBUFLOG_INFO             0xC9000011

#define HHEE_UNKNOWN_HVC                0xC90000ff

#define HHEE_ENABLE    1
#define HHEE_DISABLE   0

#define HHEE_MSG_ID_HKIP      1
#define HHEE_MSG_ID_HKIP_TEST 2
#define HHEE_MSG_ID_CRASH     5
#define HHEE_MSG_ID_MAX       8

typedef int (*hhee_callback_t)(unsigned int len, void *buf);

int hhee_msg_init(void);
int hhee_msg_register_handler(unsigned int id, hhee_callback_t fn);
int hhee_check_enable(void);
void hkip_clean_counters(void);
#ifdef CONFIG_HHEE_USING_IPI
void hhee_irq_receive(void);
#else
static inline void hhee_irq_receive(void) {};
#endif

static inline struct arm_smccc_res hhee_fn_hvc(unsigned long function_id,
			unsigned long arg0, unsigned long arg1,
			unsigned long arg2)
{
	struct arm_smccc_res res;

	arm_smccc_hvc(function_id, arg0, arg1, arg2, 0ul, 0ul, 0ul, 0ul, &res);
	return res;
}

#endif
