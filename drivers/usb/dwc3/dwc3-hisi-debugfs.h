#ifndef _DWC3_HISI_DEBUGFS_H_
#define _DWC3_HISI_DEBUGFS_H_

#include <linux/fs.h>
#include "core.h"

#ifdef CONFIG_HISI_DEBUG_FS
void dwc3_hisi_debugfs_init(struct dwc3 *dwc, struct dentry *root);
#else
static inline void dwc3_hisi_debugfs_init(struct dwc3 *dwc, struct dentry *root){}
#endif

#endif /* _DWC3_HISI_DEBUGFS_H_ */
