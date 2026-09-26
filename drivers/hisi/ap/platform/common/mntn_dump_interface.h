#ifndef __MNTN_DUMP_INTERFACE_H__
#define __MNTN_DUMP_INTERFACE_H__ 
#define DTS_MNTNDUMP_NAME "/reserved-memory/mntndump"
#define MNTNDUMP_MAGIC (0xDEADBEEF)
#define MAX_LEN_OF_MNTNDUMP_ADDR_STR (0x20)
#define MNTN_DUMP_VERSION (0xFFFF0003)
typedef enum {
    MNTN_DUMP_HEAD,
    MNTN_DUMP_ETR,
    MNTN_DUMP_KERNEL_DUMP,
    MNTN_DUMP_PANIC,
    MNTN_DUMP_FTRACE,
    MNTN_DUMP_PSTORE_RAMOOPS,
    MNTN_DUMP_BC_PANIC,
    MNTN_DUMP_LOGBUF,
#if defined(CONFIG_GCOV_KERNEL) || defined(CONFIG_HISI_GCOV_FASTBOOT)
    MNTN_DUMP_GCOV,
#endif
    MNTN_DUMP_HWDIAG,
    MNTN_DUMP_MAX
} mntn_dump_module;
#define MNTN_DUMP_HEAD_SIZE ( ((sizeof(struct mdump_head)) + sizeof(unsigned long long) - 1) & (~(sizeof(unsigned long long) - 1)) )
#define MNTN_DUMP_ETR_SIZE (0x30)
#define MNTN_DUMP_KASLR_SIZE (0x10)
#define MNTN_DUMP_HKRR_SIZE (0x10)
#define MNTN_DUMP_KERNEL_DUMP_SIZE (0x800)
#define MNTN_DUMP_PANIC_SIZE (0x100)
#define MNTN_DUMP_FTRACE_SIZE (0x30)
#define MNTN_DUMP_PSTORE_RAMOOPS_SIZE (0x30)
#define MNTN_DUMP_BC_PANIC_SIZE (0x20)
#define MNTN_DUMP_LOGBUF_SIZE (0x40)
#if defined(CONFIG_GCOV_KERNEL) || defined(CONFIG_HISI_GCOV_FASTBOOT)
#define MNTN_DUMP_GCOV_SIZE (0x10)
#endif
#define MNTN_DUMP_HWDIAG_SIZE (0x100)
#define MNTN_DUMP_MAXSIZE (0x1000 - MNTN_DUMP_KASLR_SIZE - MNTN_DUMP_HKRR_SIZE)
struct mdump_regs_info{
 int mid;
 unsigned int offset;
 unsigned int size;
} ;
struct mdump_head{
 unsigned int crc;
 unsigned int magic;
 unsigned int version;
 unsigned int nums;
 struct mdump_regs_info regs_info[MNTN_DUMP_MAX];
};
struct mdump_end{
 unsigned long magic;
};
#define FTRACE_MDUMP_MAGIC 0xF748FDE2
#define FTRACE_BUF_MAX_SIZE 0x400000
#define FTRACE_DUMP_NAME "ftrace"
#define FTRACE_DUMP_FS_NAME "/proc/balong/memory/"FTRACE_DUMP_NAME
typedef struct
{
 unsigned long long magic;
 unsigned long long paddr;
 unsigned int size;
} FTRACE_MDUMP_HEAD;
#define RECORD_PC_STR_MAX_LENGTH 0x48
typedef struct
{
 char exception_info[RECORD_PC_STR_MAX_LENGTH];
 unsigned long exception_info_len;
} AP_RECORD_PC;
#define ETR_MAGIC_START "ETRTRACE"
#define ETR_MAGIC_SIZE ((unsigned int)sizeof(ETR_MAGIC_START))
typedef struct
{
 char magic[ETR_MAGIC_SIZE];
 unsigned long long paddr;
 unsigned int size;
 unsigned int rd_offset;
} AP_RECORD_ETR;
#define LOGBUF_DUMP_MAGIC 0xFFEE5A5A
struct log_buf_dump_info {
 unsigned int crc;
 unsigned int magic;
 unsigned int reboot_reason;
 unsigned long long logbuf_addr;
 unsigned int logbuf_size;
 unsigned int idx_size;
 unsigned long long log_first_idx_addr;
 unsigned long long log_next_idx_addr;
};
struct mdump_pstore {
 unsigned int crc;
 unsigned int magic;
 unsigned long ramoops_addr;
 unsigned long ramoops_size;
};
#define BIGCORE_PANIC_MAGIC (0xDEADAB00)
#define BIGCORE_PANIC_MAGIC_MASK (0xFFFFFF00)
struct mdump_bc_panic {
 unsigned long panic_flag;
 unsigned long panic_num;
};
#define HISI_HWDIAG_TRACE_DATA_MAXLEN 128
struct hisi_hw_diag_trace {
 unsigned int used;
 unsigned int err_id;
 char data[HISI_HWDIAG_TRACE_DATA_MAXLEN];
};
typedef struct
{
 unsigned long long paddr;
 unsigned long long vaddr;
 unsigned int size;
 unsigned int rd_offset;
} KERNELDUMP_ETR;
struct hkrr_symbol_info {
 unsigned long long addr;
 unsigned int len;
 unsigned int crc;
};
void mntn_hkrr_info_set(unsigned long long addr, unsigned int len);
void mntn_hkrr_info2mem(void);
#endif
