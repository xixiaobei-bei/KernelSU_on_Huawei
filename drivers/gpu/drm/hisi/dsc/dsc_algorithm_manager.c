
#include "dsc_algorithm_manager.h"
#include "../hisi_defs.h"


static struct dsc_algorithm_manager *_dsc_algorithm_manager;
static struct dp_dsc_algorithm g_dp_dsc_algorithm;

static void init_dsc_algorithm_manager(struct dsc_algorithm_manager *p)
{
	HISI_DRM_DEBUG("[DSC] init dp dsc manager!\n");

	init_dp_dsc_algorithm(&g_dp_dsc_algorithm);
	p->vesa_dsc_info_calc  = g_dp_dsc_algorithm.vesa_dsc_info_calc;
}

struct dsc_algorithm_manager *get_dsc_algorithm_manager_instance(void)
{
	HISI_DRM_DEBUG("[DSC] +\n");

	if (_dsc_algorithm_manager != NULL)
		return _dsc_algorithm_manager;

	HISI_DRM_DEBUG("[DSC] get dsc algorithm manager instance malloc!\n");
	_dsc_algorithm_manager = kmalloc(sizeof(struct dsc_algorithm_manager), GFP_KERNEL);
	if (!_dsc_algorithm_manager)
		return NULL;
	init_dsc_algorithm_manager(_dsc_algorithm_manager);
	return _dsc_algorithm_manager;

}

