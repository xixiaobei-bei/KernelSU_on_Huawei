#ifndef DUBAI_PLAT_H
#define DUBAI_PLAT_H

#include "dubai_battery.h"
#include "dubai_ddr.h"
#include "dubai_display.h"
#include "dubai_gpu.h"
#include "dubai_log.h"
#include "dubai_peri.h"
#include "dubai_wakeup.h"

enum dubai_module_t {
	DUBAI_MODULE_BATTERY,
	DUBAI_MODULE_DDR,
	DUBAI_MODULE_DISPLAY,
	DUBAI_MODULE_GPU,
	DUBAI_MODULE_PERI,
	DUBAI_MODULE_WAKEUP,
};

#ifdef CONFIG_HUAWEI_DUBAI

int dubai_register_module_ops(enum dubai_module_t module, void *mops);
int dubai_unregister_module_ops(enum dubai_module_t module);

#else // !CONFIG_HUAWEI_DUBAI

static inline int dubai_register_module_ops(enum dubai_module_t module, void *mops)
{
	return -ENOSYS;
}

static inline int dubai_unregister_module_ops(enum dubai_module_t module)
{
	return -ENOSYS;
}

#endif // CONFIG_HUAWEI_DUBAI
#endif // DUBAI_PLAT_H
