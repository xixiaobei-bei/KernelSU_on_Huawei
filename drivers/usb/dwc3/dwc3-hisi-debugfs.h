#ifndef _DWC3_HISI_DEBUGFS_H_
#define _DWC3_HISI_DEBUGFS_H_

#include <linux/fs.h>
#include "core.h"

#ifdef CONFIG_HISI_DEBUG_FS
void dwc3_hisi_debugfs_init(struct dwc3 *dwc, struct dentry *root);
int dwc3_is_test_noc_err(void);
uint32_t dwc3_get_noc_err_addr(uint32_t addr);
#else
static inline void dwc3_hisi_debugfs_init(struct dwc3 *dwc, struct dentry *root){}
static inline int dwc3_is_test_noc_err(void){return 0;}
static inline uint32_t dwc3_get_noc_err_addr(uint32_t addr){return 0;}
#endif

#endif /* _DWC3_HISI_DEBUGFS_H_ */
