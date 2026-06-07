#include "hisi-blk-core-interface.h"

void _cfi_hisi_blk_queue_usr_ctrl_recovery_timer_expire(unsigned long data)
{
        hisi_blk_queue_usr_ctrl_recovery_timer_expire(data);
}

ssize_t __cfi_hisi_queue_status_show(struct request_queue *q, char *page)
{
	return hisi_queue_status_show(q, page);
}

