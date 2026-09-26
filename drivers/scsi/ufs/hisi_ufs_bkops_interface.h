#ifndef __HUFS_BKOPS_INTERFACE_H__
#define __HUFS_BKOPS_INTERFACE_H__


extern int ufshcd_bkops_status_query(void *bkops_data, u32 *status);
extern int __ufshcd_bkops_status_query(void *bkops_data, u32 *status);
extern int ufshcd_bkops_start_stop(void *bkops_data, int start);
extern int __ufshcd_bkops_start_stop(void *bkops_data, int start);

#endif /*  __HUFS_BKOPS_INTERFACE_H__ */
