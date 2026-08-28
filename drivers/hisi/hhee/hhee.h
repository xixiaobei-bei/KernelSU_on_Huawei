#include <asm/compiler.h>
#include <linux/arm-smccc.h>
#include <linux/interrupt.h>
#include <linux/hisi/hisi_hhee.h>

/*struct for logbuf*/
struct circular_buffer {
	unsigned long size; /* Indicates the total size of the buffer */
	unsigned long start; /* Starting point of valid data in buffer */
	unsigned long end; /* First character which is empty (can be written to) */
	unsigned long overflow; /* Indicator whether buffer has overwritten itself */
	unsigned long virt_log_addr; /*Indicator the virtual addr of buffer*/
	unsigned long virt_log_size; /*Indicator the max size of buffer*/
	unsigned int inited;        /*Indicator the status of buffer*/
	unsigned int logtype;       /*Indicator the type of buffer*/
	char *buf;
};

/*enum for logtype*/
enum ltype {
	NORMAL_LOG,
	CRASH_LOG,
	MONITOR_LOG,
};

irqreturn_t hhee_irq_handle(int irq, void *data);
int hhee_logger_init(void);
#ifdef CONFIG_HISI_HHEE_DEBUG
int hhee_init_debugfs(void);
void hhee_cleanup_debugfs(void);
#endif

ssize_t hhee_copy_logs(char __user *buf, size_t count,
		       loff_t *offp, int logtpye);
