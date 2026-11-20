/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: ana hs extern ops
 * Author: lijinkui
 * Create: 2019-10-22
 */


#ifndef __ANA_HS_EXTERN_OPS_H__
#define __ANA_HS_EXTERN_OPS_H__
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#define FSA4480_REG_OVP_INT_MASK                                 0x01
#define FSA4480_REG_OVP_INT                                      0x02
#define FSA4480_REG_OVP_STATUS                                   0x03
#define FSA4480_REG_ENABLE                                       0x12
#define FSA4480_REG_PIN_SET                                      0x13
#define FSA4480_REG_RES_VAL                                      0x14
#define FSA4480_REG_RES_DET_THRD                                 0x15
#define FSA4480_REG_DET_INTERVAL                                 0x16
#define FSA4480_REG_RES_INT                                      0x18
#define FSA4480_REG_RES_INT_MASK                                 0x19

#define RES_ENABLE                                               0x02
#define RES_DISABLE                                              0x7D
#define RES_PREC_10K                                             0x20
#define RES_PREC_1K                                              0x5F
#define RES_INT_MASK_OPEN                                        0x07
#define RES_INT_MASK_CLEAR                                       0x05
#define RES_INT_THRESHOLD                                        0x19
#define SIGNAL_DETECT                                            0x00
#define LOOP_1S_DETECT                                           0x02
#define OVP_INT_MASK_OPEN                                        0x7F
#define OVP_INT_MASK_CLEAR                                       0x03
#define OVP_STATUS_VERIFY                                        0x3C
#define CHECK_TYPEC_DELAY                                        5000
#define DETECT_DELAY_DRY                                         30000
#define DETECT_DELAY_WATER                                       1000
#define DETECT_ONBOOT_DELAY                                      10000
#define WAKE_LOCK_TIMEOUT                                        200
#define WAIT_REG_TIMEOUT                                         100
#define RES_DETECT_DELAY                                         5
#define PIN_NUM                                                  4
#define DP                                                       0
#define DN                                                       1
#define SBU1                                                     2
#define SBU2                                                     3
#define STOP_OVP_DELAY                                           2000
#define NOTIFY_WATER_DELAY                                       3000
#define WATER_PIN_NUM                                            1

/* res detect */
#define ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG_IRQ_BIT         0x04
#define ANA_FSA4480_RES_BELOW_THRESHOLD_IRQ_BIT                  0x02
#define ANA_FSA4480_RES_DETECT_ACTION_IRQ_BIT                    0x01
#define ANA_FSA4480_RES_DETECT_ACTION_IRQ_MASK_BIT               0x01
#define ANA_FSA4480_RES_BELOW_THRESHOLD_IRQ_MASK_BIT             0x02
#define ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG_IRQ_MASK_BIT    0x04

/* ovp detec */
#define OVP_IRQ_MASK_BIT                                         0x40
#define FSA4480_DSM_BUF_SIZE_256                                 256
#ifdef CONFIG_HUAWEI_DSM
#define ERROR_NO_TYPEC_4480_OVP                 ERROR_NO_TYPEC_CC_OVP
#endif

#ifdef CONFIG_ANA_HS_CORE
void ana_hs_fsa4480_start_res_detect(bool new_state);
void ana_hs_fsa4480_stop_res_detect(bool new_state);
void ana_hs_fsa4480_start_ovp_detect(bool new_state);
void ana_hs_fsa4480_stop_ovp_detect(bool new_state);
#else
static inline void ana_hs_fsa4480_start_res_detect(bool new_state)
{
}

static inline void ana_hs_fsa4480_stop_res_detect(bool new_state)
{
}

static inline void ana_hs_fsa4480_start_ovp_detect(bool new_state)
{
}

static inline void ana_hs_fsa4480_stop_ovp_detect(bool new_state)
{
}
#endif

enum ana_fsa4480_res_irq_type {
	ANA_FSA4480_RES_DETECT_ACTION             = 0,
	ANA_FSA4480_RES_BELOW_THRESHOLD           = 1,
	ANA_FSA4480_AUDIO_JACK_DETECT_AND_CONFIG  = 2,
	ANA_FSA4480_IQR_MAX                       = 3,
};

enum ana_fsa4480_ovp_irq_type {
	ANA_FSA4480_OVP_IRQ_EXIST                 = 0,
	ANA_FSA4480_OVP_IRQ_MAX                   = 1,
};

enum ana_hs_typec_state {
	ANA_HS_TYPEC_DEVICE_ATTACHED              = 0,
	ANA_HS_TYPEC_DEVICE_DETACHED              = 1,
};

enum {
	ID_FSA4480_IRQ,
	ID_IRQ_END,
};

enum {
	ID_FSA4480_INIT,
	ID_INIT_END,
};

enum {
	ID_FSA4480_REMOVE,
	ID_REMOVE_END,
};

struct irq_id_func_pair {
	int irq_id;
	irq_handler_t func;
};

irq_handler_t ana_hs_irq_select_func(int irq_id);

typedef void (*ana_hs_init_func)(void);

struct init_id_func_pair {
	int init_id;
	ana_hs_init_func func;
};

ana_hs_init_func ana_hs_init_select_func(int init_id);

typedef void (*ana_hs_remove_func)(void);

struct remove_id_func_pair {
	int remove_id;
	ana_hs_remove_func func;
};

ana_hs_remove_func ana_hs_remove_select_func(int remove_id);

#endif /* __ANA_HS_EXTERN_OPS_H__ */
