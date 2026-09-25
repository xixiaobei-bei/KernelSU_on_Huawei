/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Description: Contexthub share memory driver
 * Author: Huawei
 * Create: 2016-04-01
 */
#ifndef __IOMCU_DUMP_H__
#define __IOMCU_DUMP_H__
#include <linux/types.h>
#include <protocol.h>
#include <hisi_rproc.h>
#include <mntn_public_interface.h>

#define SENSOR_VOLTAGE_3_2V  3200000
#define SENSOR_VOLTAGE_3V    3000000
#define SENSOR_VOLTAGE_1V8   1800000
#define IOM3_RECOVERY_UNINIT 0
#define IOM3_RECOVERY_IDLE (IOM3_RECOVERY_UNINIT + 1)
#define IOM3_RECOVERY_START (IOM3_RECOVERY_IDLE + 1)
#define IOM3_RECOVERY_MINISYS (IOM3_RECOVERY_START + 1)
#define IOM3_RECOVERY_DOING (IOM3_RECOVERY_MINISYS + 1)
#define IOM3_RECOVERY_3RD_DOING (IOM3_RECOVERY_DOING + 1)
#define IOM3_RECOVERY_FAILED (IOM3_RECOVERY_3RD_DOING + 1)
/* for iom3 recovery debug used */
#define IOMCU_CLK_SEL 0x00
#define IOMCU_CFG_DIV0 0x04
#define IOMCU_CFG_DIV1 0x08
#define CLKEN0_OFFSET 0x010
#define CLKSTAT0_OFFSET 0x18
#define CLKEN1_OFFSET 0x090
#define RSTEN0_OFFSET 0x020
#define RSTDIS0_OFFSET 0x024
#define RSTSTAT0_OFFSET 0x028

#define I2C_0_RST_VAL (BIT(3))

#define IOM3_REC_NEST_MAX 5

#define WD_INT 144
#define SENSORHUB_MODID HISI_BB_MOD_IOM_START
#define SENSORHUB_USER_MODID (HISI_BB_MOD_IOM_START + 1)
#define SENSORHUB_FDUL_MODID (HISI_BB_MOD_IOM_START + 2)
#define SENSORHUB_MODID_END HISI_BB_MOD_IOM_END
/* it need to add to modid_react_dump when adding new SENSORHUB MOID to avoid twice dump in sh_dump */
#define modid_react_dump(modid)  ((modid) != SENSORHUB_MODID && (modid) != SENSORHUB_USER_MODID)
#define SENSORHUB_DUMP_BUFF_ADDR DDR_LOG_BUFF_ADDR_AP
#define SENSORHUB_DUMP_BUFF_SIZE DDR_LOG_BUFF_SIZE

#define REG_UNLOCK_KEY 0x1ACCE551
#define WDOGCTRL 0x08
#define WDOGINTCLR 0x0C
#define WDOGLOCK 0xC00
#define PATH_MAXLEN 128
#define HISTORY_LOG_SIZE 256
#define HISTORY_LOG_MAX 0x80000 /* 512k */
#define ROOT_UID 0
#define SYSTEM_GID 1000
#define DIR_LIMIT 0770
#define FILE_LIMIT 0660
#define SH_DMP_DIR  "/data/log/sensorhub-log/"
#define SH_DMP_FS  "/data/lost+found"
#define SH_DMP_HISTORY_FILE "history.log"

#define DATATIME_MAXLEN 24 /* 14+8 +2, 2: '-'+'\0' */
#define MAX_DUMP_CNT 32

#define SH_DUMP_INIT 0
#define SH_DUMP_START 1
#define SH_DUMP_FINISH 2

typedef enum {
	SH_FAULT_HARDFAULT = 0,
	SH_FAULT_BUSFAULT,
	SH_FAULT_USAGEFAULT,
	SH_FAULT_MEMFAULT,
	SH_FAULT_NMIFAULT,
	SH_FAULT_ASSERT,
	SH_FAULT_INTERNELFAULT = 16,
	SH_FAULT_IPC_RX_TIMEOUT,
	SH_FAULT_IPC_TX_TIMEOUT,
	SH_FAULT_RESET,
	SH_FAULT_USER_DUMP,
	SH_FAULT_RESUME,
	SH_FAULT_REDETECT,
	SH_FAULT_PANIC,
	SH_FAULT_NOC,
	SH_FAULT_REACT,
	SH_FAULT_EXP_BOTTOM,
} exp_source_t;

typedef struct {
	uint32_t cnt;
	uint32_t len;
	uint32_t tuncate;
} dump_zone_head_t;
extern char *rdr_get_timestamp(void);
#ifdef CONFIG_HISI_BB
extern u64 rdr_get_tick(void);
#endif
int iomcu_dump_init(void);
extern int g_sensorhub_wdt_irq;
extern int g_iom3_state;
extern uint32_t need_reset_io_power;
extern uint32_t need_set_3v_io_power;
extern uint32_t need_set_3_2v_io_power;
extern struct regulator *sensorhub_vddio;
extern struct notifier_block charge_recovery_notify;
extern rproc_id_t ipc_ap_to_iom_mbx;
extern rproc_id_t ipc_ap_to_lpm_mbx;
extern void rdr_system_error(u32 modid, u32 arg1, u32 arg2);
extern void emg_flush_logbuff(void);
extern void reset_logbuff(void);
extern void __disable_irq(struct irq_desc *desc, unsigned int irq, bool suspend);
extern void __enable_irq(struct irq_desc *desc, unsigned int irq, bool resume);
extern int bbox_chown(const char *path, uid_t user, gid_t group, bool recursion);
extern int iom3_need_recovery(int modid, exp_source_t f);
extern int recovery_init(void);
extern int register_iom3_recovery_notifier(struct notifier_block *nb);
extern int iom3_rec_sys_callback(const pkt_header_t *head);
extern void enable_key_when_recovery_iom3(void);
#endif /* __IOMCU_DUMP_H__ */
