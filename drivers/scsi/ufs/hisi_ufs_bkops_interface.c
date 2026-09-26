#include <linux/kernel.h>
#include "hisi_ufs_bkops_interface.h"


int __ufshcd_bkops_status_query(void *bkops_data, u32 *status)
{
	return ufshcd_bkops_status_query(bkops_data, status);
}


int __ufshcd_bkops_start_stop(void *bkops_data, int start)
{
	return ufshcd_bkops_start_stop(bkops_data, start);
}

