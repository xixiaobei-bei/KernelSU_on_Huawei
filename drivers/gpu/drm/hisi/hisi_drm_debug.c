/*lint -e546 -e580*/

#include "hisi_drm_debug.h"

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <drm/drmP.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

#include "hisi_defs.h"

u32 hisi_drm_log_flag;
u32 hisi_drm_msg_level = 7;

module_param_named(hisi_drm_log_flag, hisi_drm_log_flag, int, 0644);
MODULE_PARM_DESC(hisi_drm_log_flag, "hisi drm log flag");

module_param_named(hisi_drm_msg_level, hisi_drm_msg_level, int, 0644);
MODULE_PARM_DESC(hisi_drm_msg_level, "hisi drm msg level");

void hisi_drm_printk(const char *level, unsigned int category,
		     const char *func, const char *fmt, ...)
{
	struct va_format vaf;
	va_list args;

	if (category != HISI_DRM_NONE_FLAG && !(hisi_drm_log_flag & category))
		return;

	va_start(args, fmt);
	vaf.fmt = fmt;
	vaf.va = &args;

	pr_info("[HISI_DRM I]:%s: %pV", func, &vaf);

	va_end(args);
}
EXPORT_SYMBOL(hisi_drm_printk);

/*lint +e546 +e580*/
