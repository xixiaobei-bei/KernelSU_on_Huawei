

#ifndef _HISI_LOWPM_CTRL_H
#define _HISI_LOWPM_CTRL_H

#include <linux/types.h>

#define SUSPEND_NOT_SWITCH_32K	0
#define SUSPEND_SWITCH_32K		1

enum lowpm_errno {
	LOWPM_IPC_SUCCESS	= 0,
	LOWPM_IPC_EINVAL	= 1,
	LOWPM_IPC_ECOMM	= 2,
	LOWPM_IPC_EFAILED	= 3
};
#ifdef CONFIG_HISI_PM_LPCTRL
int hisi_pm_lpctrl_init(void);
#else
int hisi_pm_lpctrl_init(void){return 0; }
#endif /* CONFIG_HISI_PM_LPCTRL */

#endif /* _HISI_LOWPM_CTRL_H */
