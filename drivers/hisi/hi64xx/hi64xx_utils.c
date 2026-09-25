#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/hisi/hi64xx/hi64xx_utils.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
/*lint -e750 -e679*/
#ifdef CONFIG_SND_SOC_HI6405
#include <linux/hisi/hi64xx/hi6405.h>
#endif

static struct utils_config *s_utils_config = NULL;
static unsigned int cdc_type = HI64XX_CODEC_TYPE_BUTT;
static struct snd_soc_codec *snd_codec = NULL;

extern void hi64xx_resmgr_dump(struct hi64xx_resmgr *resmgr);

int hi64xx_update_bits(struct snd_soc_codec *codec, unsigned int reg,
                unsigned int mask, unsigned int value)
{
	int change;
	unsigned int old, new;

	old = snd_soc_read(codec, reg);
	new = (old & ~mask) | (value & mask);
	change = old != new;
	if (change)
		snd_soc_write(codec, reg, new);

	return change;
}
EXPORT_SYMBOL_GPL(hi64xx_update_bits);




int hi64xx_utils_init(struct snd_soc_codec *codec, struct hi_cdc_ctrl *cdc_ctrl, const struct utils_config *config,
	struct hi64xx_resmgr* resmgr, unsigned int codec_type)
{

	s_utils_config = kzalloc(sizeof(struct utils_config), GFP_KERNEL);
	if (!s_utils_config) {
		pr_err("hi64xx_utils_init: Failed to kzalloc s_utils_config\n");
		goto error_exit;
	}
	memcpy(s_utils_config, config, sizeof(struct utils_config));

	cdc_type = codec_type;
	snd_codec = codec;

	return 0;

error_exit:
	if (!s_utils_config) {
		kfree(s_utils_config);
		s_utils_config = NULL;
	}

	snd_codec = NULL;

	return -EFAULT;
}
EXPORT_SYMBOL(hi64xx_utils_init);

void hi64xx_utils_deinit(void)
{
	if (!s_utils_config) {
		kfree(s_utils_config);
		s_utils_config = NULL;
	}

	snd_codec = NULL;
}
EXPORT_SYMBOL(hi64xx_utils_deinit);

#ifdef CONFIG_HISI_DIEID
int hisi_codec_get_dieid(char *dieid, unsigned int len)
{
#ifdef CONFIG_SND_SOC_HI6405
	if (cdc_type == HI64XX_CODEC_TYPE_6405) {
		return hi6405_codec_get_dieid(dieid, len);
	}
#endif
	return -1;
}
#endif

unsigned int hi64xx_utils_reg_read(unsigned int reg)
{
	if (snd_codec == NULL) {
		pr_err("%s: snd_codec is NULL", __FUNCTION__);
		return 0;
	}

	return snd_soc_read(snd_codec, reg);
}

MODULE_DESCRIPTION("hi64xx util");
MODULE_AUTHOR("liujinhong <liujinhong@hisilicon.com>");
MODULE_LICENSE("GPL");
