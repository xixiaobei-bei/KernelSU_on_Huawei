#include "hisi-blk-core-interface.h"

void _cfi_hisi_blk_queue_usr_ctrl_recovery_timer_expire(unsigned long data)
{
        hisi_blk_queue_usr_ctrl_recovery_timer_expire(data);
}

ssize_t __cfi_hisi_queue_status_show(struct request_queue *q, char *page)
{
	return hisi_queue_status_show(q, page);
}

#if defined(CONFIG_HISI_DEBUG_FS) || defined(CONFIG_HISI_BLK_DEBUG)
ssize_t __cfi_hisi_queue_io_prio_sim_show(struct request_queue * q, char *page)
{
	return hisi_queue_io_prio_sim_show(q, page);
}

ssize_t __cfi_hisi_queue_io_prio_sim_store(
	struct request_queue *q, const char *page, size_t count)
{
	return hisi_queue_io_prio_sim_store(q, page, count);
}
#endif
