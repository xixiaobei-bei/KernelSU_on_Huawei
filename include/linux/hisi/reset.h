/* Copyright (c) 2008-2011, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 *  reset.h    reset module interface declare
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef __MACH_RESET_H__
#define __MACH_RESET_H__

#include <linux/notifier.h>
#include <linux/semaphore.h>
#include "hifidrvinterface.h"
#include <linux/pstore_ram.h>

/*
 * Macro Definition
 */
#define RESET_CPU_CCORE_STATUS_OFF            0x00000001
#define RESET_CPU_MCU_STATUS_OFF              0x00000002
#define RESET_CPU_HIFI_STATUS_OFF             0x00000004
/* CCORE OR hifi Reset the master control macro separately */
#define BSP_C_HIFI_RESET_ALONE_FEATURE
#undef RESET_PRINK_TO_FILE

/* This macro is used to control test code */
#define BSP_C_HIFI_RESET_ALONE_TESET_VALID
#define BSP_RESET_MAILBOX_MSG_PROTECT           0x5a5a5a5a

/* CCORE OR hifi independent reset. Security verification is required for CCORE loading */
#undef BSP_C_HIFI_RESET_ALONE_SEC_FEATURE

#define BSP_RESET_OK                    0
#define BSP_RESET_ERROR                 (-1)

#define DRV_RESET_MODULE_NAME_LEN       9 /* exclude '\0' */
/* reference to copy from bootloader, fastboot, starts */
#define EFI_NAMELEN     36

/* Maximum length of a message */
#define     BSP_RESET_LOG_INFO_ITEM_LEN     100
/* File for storing log information */
#define     BSP_RESET_LOG_FILE      "log/record/reset_sub.txt"

/* Priority of the bottom-layer software callback function */
#define     BSP_DRV_CBFUN_PRIOLEVEL         5

/* Message Structure Macro Definition */
#define RESET_MCU_MSG_CORE_OFFSET               (16)
#define RESET_MCU_MSG_ID(core,phase)            ((core << RESET_MCU_MSG_CORE_OFFSET) | phase)
#define RESET_MCU_MSG_CORE(msg_id)              ((msg_id & 0xFFFF0000) >> RESET_MCU_MSG_CORE_OFFSET)
#define RESET_MCU_MSG_PHASE(msg_id)             (msg_id & 0x0000FFFF)

/* used for stop hifi watchdog1 */
#define DRV_WATCHDOG_LOCK_NUM           (0x0)           /* SP805 WATCHDOG Lock word */
#define DRV_WATCHDOG_UNLOCK_NUM         (0x1ACCE551)    /* SP805 WATCHDOG Unlock word */
#define DRV_WATCHDOG_CONTROL_DISABLE    (0x0)           /* SP805 WATCHDOG Control word */
#define DRV_WATCHDOG_INTCLR_NUM         (0x4455)        /* SP805 WATCHDOG Interrupt clear word */
/* Initialization delay of the initialization task of the independent reset module */
#define RESET_MOUDLE_DELAY_TIME          (2)


#define MNTN_PRINTBUF_LEN_MAX		128
#undef MNTN_SAVE_LOG_BY_FILESYS


#define MNTN_PRINT		printk

#ifndef FALSE
#define FALSE               (0)
#endif

#ifndef TRUE
#define TRUE                (1)
#endif

#ifndef __MDRV_ACORE_SYSBOOT_H__
typedef enum{
	MDRV_RESET_CB_BEFORE,   /* callback before doing the reset */
	MDRV_RESET_CB_AFTER,    /* callback after reset done */
	MDRV_RESET_RESETTING,   /* callback during doing the reset */
	MDRV_RESET_CB_INVALID,
}DRV_RESET_CB_MOMENT_E;

typedef int (*pdrv_reset_cbfun)(DRV_RESET_CB_MOMENT_E enparam, int userdata);
#endif
/*
 * Data structure definition
 */
/* Record information of callback functions */
typedef struct
{
	char		name[DRV_RESET_MODULE_NAME_LEN + 1];
	int				priolevel;
	pdrv_reset_cbfun	cbfun;
	int         userdata;
} sreset_mgr_callbackfunc;

/* Link list unit of managing all callback functions */
typedef struct _sreset_mgr_LLI
{
	sreset_mgr_callbackfunc cbfuninfo;
	struct _sreset_mgr_LLI	*pnext;
} sreset_mgr_LLI;


typedef struct _sreset_mailbox_msg_ccore
{
	int     iproctec;
	int     idata;
	int     ireserved;
} sreset_mailbox_msg_mcu;

typedef struct _sreset_debug_info
{
	unsigned int    ul_ccpu_task_phase;
} sreset_debug_info;
/*
 * Enumeration type
 */
/* Modules involved in the reset function */
typedef enum
{
	BSP_RESET_MODULE_CCORE,
	BSP_RESET_MODULE_HIFI,
	BSP_RESET_MODULE_MCU,
	BSP_RESET_MODULE_ACORE,
	BSP_RESET_MODULE_INVALID
} ereset_module;

/* Modules involved in the reset function */
typedef enum
{
	BSP_RESET_CORE_CCORE = 0x0,
	BSP_RESET_CORE_HIFI = 0x1,
	BSP_RESET_CORE_INVALID
} ereset_core_with_mcu;
typedef enum
{
	BSP_RESET_PHASE_RESET_BEFORE = 0x0,
	BSP_RESET_PHASE_RESET_AFTER = 0x1,
	BSP_RESET_PHASE_RESET_FINISH = 0x2,
	BSP_RESET_PHASE_INVALID
} ereset_phase_with_mcu;
typedef enum
{
	BSP_RESET_MCU_MSG_CCORE_RESET_BEFORE = RESET_MCU_MSG_ID(BSP_RESET_CORE_CCORE, BSP_RESET_PHASE_RESET_BEFORE),
	BSP_RESET_MCU_MSG_CCORE_RESET_AFTER = RESET_MCU_MSG_ID(BSP_RESET_CORE_CCORE, BSP_RESET_PHASE_RESET_AFTER),
	BSP_RESET_MCU_MSG_CCORE_RESET_FINISH = RESET_MCU_MSG_ID(BSP_RESET_CORE_CCORE, BSP_RESET_PHASE_RESET_FINISH),
	BSP_RESET_MCU_MSG_HIFI_RESET_BEFORE = RESET_MCU_MSG_ID(BSP_RESET_CORE_HIFI, BSP_RESET_PHASE_RESET_BEFORE),
	BSP_RESET_MCU_MSG_HIFI_RESET_AFTER = RESET_MCU_MSG_ID(BSP_RESET_CORE_HIFI, BSP_RESET_PHASE_RESET_AFTER),
	BSP_RESET_MCU_MSG_INVALID = RESET_MCU_MSG_ID(BSP_RESET_CORE_INVALID, BSP_RESET_PHASE_INVALID)
} ereset_msg_with_mcu;

typedef enum
{
	RESET_TYPE_FAILGET_MSG_FROM_MCU,        /* Process the C-core reset, and no MCU message is received. */
	RESET_TYPE_FAILGET_MSG_FROM_HIFI,       /* No Hi-Fi message is received when the C core is reset. */
	RESET_TYPE_MSG_FROM_MCU_WRONG,          /* The MCU responds with incorrect messages when the C core is reset. */
	RESET_TYPE_MSG_FROM_HIFI_WRONG,         /* The message returned by the Hi-Fi is incorrect when the C core is reset. */
	RESET_TYPE_CRESET_NOTIFY_BEFORE,        /* Process the C-core reset. An error occurs in the notification before the reset. */
	RESET_TYPE_CRESET_NOTIFY_AFTER,         /* Process the C-core reset. After the reset, the notification is incorrect */
	RESET_TYPE_CRESET_NOTIFY_FINISH,        /* An error occurred when processing the C-core reset and reset completion notification. */
	RESET_TYPE_CRESET_RUNCB_STEP1_FAIL,     /* An error occurs when the callback function is executed before the C-core reset is processed. */
	RESET_TYPE_CRESET_RUNCB_STEP2_FAIL,     /* An error occurs when the callback function is executed after the C-core reset is processed. */
	RESET_TYPE_CRESET_DORESET,              /* Process the C-core reset, and the reset error occurs. */
	RESET_TYPE_CRESET_LOAD_BBE16_FAIL,      /* An error occurs when the BBE16 is loaded when the C-core is reset. */
	RESET_TYPE_CRESET_LOAD_CCPU_FAIL,       /* An error occurred when loading the CCPU due to the reset of the C-core. */
	RESET_TYPE_CRESET_CRUN_NO_OK,           /* The C-core fails to be started after it is reset independently. */

	RESET_TYPE_HIFIRESET_RUNCB_STEP1_FAIL,  /* An error occurs when the callback function is run before the HiFi core reset is processed. */
	RESET_TYPE_HIFIRESET_RUNCB_STEP2_FAIL,  /* An error occurs when the callback function is executed after the Hi-Fi core reset is processed. */
	RESET_TYPE_HIFIRESET_LOAD_BIN_FAIL,     /* Processing the Hi-Fi core reset and loading error */

	/* add new above,please!! */
	RESET_TYPE_FOR_TEST,        /* Used for testing */
	RESET_TYPE_MAX,
} ereset_type;

/* Reset Type */
typedef enum
{
	BSP_RESET_NONE,
	BSP_RESET_TYPE_MDM_PANIC,
	BSP_RESET_TYPE_MCU_PANIC
} ereset_irq_type;

/* Domain space enumeration definition */
typedef enum access_domain_type
{
	ACCESS_DOMAIN_NONE,
	ACCESS_DOMAIN_MODEM_SC, /* modem Controller domain */
	ACCESS_DOMAIN_PERI_SC,  /* Peripheral controller domain */
	ACCESS_DOMAIN_MEDIA_SC,  /* Media control register  */
	ACCESS_DOMAIN_MEDIA_NOC,  /* media noc control register */
	ACCESS_DOMAIN_BBE16_DTCM,  /* bbe16 dtcm */



	/* Please add above */
	ACCESS_DOMAIN_MAX
} eACCESS_DOMAIN_TYPE;

/*
 * Function Reference
 */
extern int hifireset_runcbfun (DRV_RESET_CB_MOMENT_E eparam);
extern int hifireset_tonotify(DRV_RESET_CB_MOMENT_E eparam, ereset_module emodule);
extern void hifi_freeze_give_semaphone(void);
extern int hifireset_loadhifibin(void);
extern void mailbox_recfun_hifi(void  *UserHandle,void *MailHandle,unsigned int MailLen);
extern sreset_mgr_LLI * reset_link_insert(sreset_mgr_LLI *plink, sreset_mgr_LLI *punit);
extern int reset_no_ok_savelog(char *pname, int iresult, DRV_RESET_CB_MOMENT_E eparam, ereset_module emodule);
extern int reset_for_savelog(char *pstr);
extern void reset_hifi_up_semaphone(void);
extern void finish_reset_sub(void);

/* register the notifier to the notify events below */
int mcu_panic_regsister_notifier(struct notifier_block *n);
int hifi_freeze_regsister_notifier(struct notifier_block *n);
int android_freeze_regsister_notifier(struct notifier_block *n);
int mcu_freeze_regsister_notifier(struct notifier_block *n);
int android_reboot_regsister_notifier(struct notifier_block *n);
int wdg_rstlog_save_regsister_notifier(struct notifier_block *n);
int acpuwdg_historylog_write_regsister_notifier(struct notifier_block *n);
int mculocalwdg_historylog_write_regsister_notifier(struct notifier_block *n);
int mcuglbwdg_historylog_write_regsister_notifier(struct notifier_block *n);
int mcu_puzzle_regsister_notifier(struct notifier_block *n);

/* unregister the notifier from the notify events below */
int mcu_panic_unregsister_notifier(struct notifier_block *n);
int hifi_freeze_unregsister_notifier(struct notifier_block *n);
int android_freeze_unregsister_notifier(struct notifier_block *n);
int mcu_freeze_unregsister_notifier(struct notifier_block *n);
int android_reboot_unregsister_notifier(struct notifier_block *n);
int wdg_rstlog_save_unregsister_notifier(struct notifier_block *n);
int acpuwdg_historylog_write_unregsister_notifier(struct notifier_block *n);
int mculocalwdg_historylog_write_unregsister_notifier(struct notifier_block *n);
int mcuglbwdg_historylog_write_unregsister_notifier(struct notifier_block *n);
void reset_clean_wdt_int(ereset_module emodule);
int mcu_puzzle_unregsister_notifier(struct notifier_block *n);
void save_mcu_log_info(void);
void test_fiq(void);
int drv_read_bin(const char *partion_name, unsigned int offset, unsigned length, char *buffer);
int is_mcu_exception(void);
void set_acore_state(unsigned int core, int state);
void clear_acore_state(unsigned int core);
/* END */

extern int get_domain_access_status(eACCESS_DOMAIN_TYPE etype);

#endif

