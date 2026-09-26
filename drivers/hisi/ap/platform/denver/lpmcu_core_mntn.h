#ifndef __LPMCU_CORE_MNTN_H__
#define __LPMCU_CORE_MNTN_H__ 
enum {
 RDR_BACKUP_IDEX0 = 0,
 RDR_BACKUP_IDEX1,
 RDR_BACKUP_IDEX2,
 RDR_BACKUP_IDEX3,
 RDR_BACKUP_IDEX_MAX
};
struct exc_spec_data {
 unsigned char reset_reason[RDR_BACKUP_IDEX_MAX];
 unsigned int slice;
 unsigned int rtc;
 unsigned int r13;
 unsigned int lr1;
 unsigned int pc;
 unsigned int xpsr;
 unsigned int cfsr;
 unsigned int hfsr;
 unsigned int bfar;
 unsigned char exc_trace;
 unsigned char ddr_exc;
 unsigned short irq_id;
 unsigned int task_id;
};
struct rdr_lpmcu_cregs {
 unsigned int r0;
 unsigned int r1;
 unsigned int r2;
 unsigned int r3;
 unsigned int r4;
 unsigned int r5;
 unsigned int r6;
 unsigned int r7;
 unsigned int r8;
 unsigned int r9;
 unsigned int r10;
 unsigned int r11;
 unsigned int r12;
 unsigned int r13;
 unsigned int sp;
 unsigned int psp;
 unsigned int lr0;
 unsigned int lr1;
 unsigned int pc;
 unsigned int xpsr;
 unsigned int primask;
 unsigned int basepri;
 unsigned int faultmask;
 unsigned int control;
};
struct rdr_reg_backup {
 unsigned int idex;
 unsigned int reason[RDR_BACKUP_IDEX_MAX];
 struct rdr_lpmcu_cregs core_regs[RDR_BACKUP_IDEX_MAX];
};
#endif
