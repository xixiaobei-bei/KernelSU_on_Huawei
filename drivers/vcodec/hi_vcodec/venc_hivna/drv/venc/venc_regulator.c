#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/hisi/hisi-iommu.h>

#include "venc_regulator.h"
#include "drv_venc_osal.h"
#include "drv_venc.h"

#define VENC_CLK_RATE         "enc_clk_rate"
#define VENC_REGULATOR_NAME   "ldo_venc"
#define MEDIA_REGULATOR_NAME  "ldo_media"
#define VENC_CLOCK_NAME       "clk_venc"

static  struct clk       *g_PvencClk        = HI_NULL;
struct  iommu_domain     *g_hisi_mmu_domain = HI_NULL;
static  struct regulator *g_VencRegulator   = HI_NULL;
static  struct regulator *g_MediaRegulator  = HI_NULL;
static  VeduEfl_DTS_CONFIG_S g_VencDtsConfig;
static  VENC_CLK_TYPE g_currClk = VENC_CLK_RATE_LOW;

static HI_BOOL g_VencPowerOn = HI_FALSE;

#ifdef VENC_QOS_CFG
#define VENC_QOS_MODE                0xE894000C
#define VENC_QOS_BANDWIDTH           0xE8940010
#define VENC_QOS_SATURATION          0xE8940014
#define VENC_QOS_EXTCONTROL          0xE8940018

static HI_U32 g_VencQosMode        = 0x1;
static HI_U32 g_VencQosBandwidth   = 0x1000;
static HI_U32 g_VencQosSaturation  = 0x20;
static HI_U32 g_VencQosExtcontrol  = 0x1;
#endif

#ifdef CONFIG_ES_VENC_LOW_FREQ
static HI_U32 g_VencLowFreq        = 480000000;
#endif

/*lint -e838 -e747 -e774 -e845*/
static int Venc_Enable_Iommu(void)
{
	struct iommu_domain *hisi_domain = NULL;
	struct iommu_domain_data* domain_data = NULL;
	uint64_t phy_pgd_base = 0;

	/* create iommu domain */
	hisi_domain = hisi_ion_enable_iommu(NULL);
	if (!hisi_domain) {
		HI_FATAL_VENC("iommu domain alloc failed\n");
		return -1;
	}

	domain_data = (struct iommu_domain_data *)(hisi_domain->priv);
	if (!domain_data) {
		HI_FATAL_VENC("iommu domain priv is NULL\n");
		return -1;
	}

	phy_pgd_base = (uint64_t)(domain_data->phy_pgd_base);
	if (!phy_pgd_base) {
		HI_FATAL_VENC("iommu domain phy_pgd_base is NULL\n");
		return -1;
	}
	HI_INFO_VENC("pgd base phy addr is %pK\n", (void *)(uintptr_t)phy_pgd_base);

	g_hisi_mmu_domain = hisi_domain;

	return 0;
}

static int Venc_Disable_Iommu(void)
{
	if(NULL != g_hisi_mmu_domain) {
		g_hisi_mmu_domain = NULL;
		return 0;
	}

	return -1;
}

static HI_S32 Venc_GetDtsConfigInfo(struct platform_device *pdev, VeduEfl_DTS_CONFIG_S *pDtsConfig)
{
	HI_U32 rate_h = 0;
	HI_U32 rate_n = 0;
	HI_U32 rate_l = 0;
	HI_S32 ret    = HI_FAILURE;
	struct resource res;
	struct clk *pvenc_clk    = HI_NULL;
	struct device_node *np   = NULL;
	struct device *dev       = &pdev->dev;
	struct iommu_domain_data *domain_data = NULL;

	if (!dev) {
		HI_FATAL_VENC("invalid argument, dev is NULL\n");
		return HI_FAILURE;
	}

	np = dev->of_node;

	HiMemSet(&res, 0, sizeof(res));
	if ((!np) || (!pDtsConfig)) {
		HI_FATAL_VENC("invalid argument np or pDtsConfig is NULL\n");
		return HI_FAILURE;
	}

	/* 1 read IRQ num from dts */
	pDtsConfig->VeduIrqNumNorm = irq_of_parse_and_map(np, 0);
	if (pDtsConfig->VeduIrqNumNorm == 0) {
		HI_FATAL_VENC("parse and map irq VeduIrqNumNorm failed\n");
		return HI_FAILURE;
	}

	pDtsConfig->VeduIrqNumProt = irq_of_parse_and_map(np, 1);
	if (pDtsConfig->VeduIrqNumProt == 0) {
		HI_FATAL_VENC("parse and map irq VeduIrqNumProt failed\n");
		return HI_FAILURE;
	}

	pDtsConfig->VeduIrqNumSafe = irq_of_parse_and_map(np, 2);
	if (pDtsConfig->VeduIrqNumSafe == 0) {
		HI_FATAL_VENC("parse and map irq VeduIrqNumSafe failed\n");
		return HI_FAILURE;
	}

	/* 2 read venc register start address, range */
	ret = of_address_to_resource(np, 0, &res);
	if (ret) {
		HI_FATAL_VENC("address to resource failed, ret value is %d\n", ret);
		return HI_FAILURE;
	}
	pDtsConfig->VencRegBaseAddr = res.start;/*lint !e712 */
	pDtsConfig->VencRegRange    = resource_size(&res);/*lint !e712 */

	/* 3 read venc clk rate [low, high], venc clock */
	pvenc_clk  = devm_clk_get(dev, VENC_CLOCK_NAME);
	if (IS_ERR_OR_NULL(pvenc_clk)) {
		HI_FATAL_VENC("can not get venc clock, pvenc_clk is 0x%pK\n", pvenc_clk);
		return HI_FAILURE;
	}
	g_PvencClk  = pvenc_clk;

	ret = of_property_read_u32_index(np, VENC_CLK_RATE, 0, &rate_h);
	ret += of_property_read_u32_index(np, VENC_CLK_RATE, 1, &rate_n);
	ret += of_property_read_u32_index(np, VENC_CLK_RATE, 2, &rate_l);
	if (ret) {
		HI_FATAL_VENC("can not get venc rate, return %d\n", ret);
		return HI_FAILURE;
	}
	pDtsConfig->highRate   = rate_h;
	pDtsConfig->normalRate = rate_n;
	pDtsConfig->lowRate    = rate_l;

#ifdef CONFIG_ES_VENC_LOW_FREQ
	pDtsConfig->highRate   = g_VencLowFreq;
#endif

	HI_INFO_VENC("venc_clk_rate: highRate:%u, normalRate:%u, lowRate:%u\n",  pDtsConfig->highRate, pDtsConfig->normalRate, pDtsConfig->lowRate);

	/* 4 fpga platform */
	ret = of_property_read_u32(np, "venc_fpga", &pDtsConfig->IsFPGA);
	if (ret)
		HI_INFO_VENC("read failed, but not important\n");

	/* 5 get venc qos mode */
	ret = of_property_read_u32(np, "venc_qos_mode", &pDtsConfig->VencQosMode);
	if (ret) {
		HI_ERR_VENC("get venc qos mode failed set default\n");
	}

	domain_data = (struct iommu_domain_data *)(g_hisi_mmu_domain->priv);
	if (domain_data) {
		pDtsConfig->SmmuPageBaseAddr = (uint64_t)(domain_data->phy_pgd_base);
		HI_INFO_VENC("SmmuPageBaseAddr is 0x%pK\n", __func__, pDtsConfig->SmmuPageBaseAddr);
	}

	return HI_SUCCESS;
}

static HI_S32 Venc_Regulator_Get(struct platform_device *pdev)
{
	g_VencRegulator  = HI_NULL;
	g_MediaRegulator = HI_NULL;

	g_MediaRegulator = devm_regulator_get(&pdev->dev, MEDIA_REGULATOR_NAME);
	if (IS_ERR_OR_NULL(g_MediaRegulator)) {
		HI_FATAL_VENC("get media regulator failed, error no is %ld\n", PTR_ERR(g_MediaRegulator));
		g_MediaRegulator = HI_NULL;

		return HI_FAILURE;
	}

	g_VencRegulator = devm_regulator_get(&pdev->dev, VENC_REGULATOR_NAME);
	if (IS_ERR_OR_NULL(g_VencRegulator)) {
		HI_FATAL_VENC("get regulator failed, error no is %ld\n", PTR_ERR(g_VencRegulator));
		g_VencRegulator = HI_NULL;

		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

#ifdef VENC_QOS_CFG
static HI_S32 Venc_Config_QOS(void)
{
	HI_U32 *qos_addr = HI_NULL;

	qos_addr = (HI_U32*)ioremap(VENC_QOS_MODE, 4);
	if(!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_MODE reg failed! \n");
		return HI_FAILURE;
	}
	writel(g_VencQosMode, qos_addr);
	iounmap(qos_addr);

	qos_addr = (HI_U32*)ioremap(VENC_QOS_BANDWIDTH, 4);
	if(!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_BANDWIDTH reg failed! \n");
		return HI_FAILURE;
	}
	writel(g_VencQosBandwidth, qos_addr);
	iounmap(qos_addr);

	qos_addr = (HI_U32*)ioremap(VENC_QOS_SATURATION, 4);
	if(!qos_addr) {
		HI_FATAL_VENC("ioremap Venc_QOS_SATURATION reg failed! \n");
		return HI_FAILURE;
	}
	writel(g_VencQosSaturation, qos_addr);
	iounmap(qos_addr);

	qos_addr = (HI_U32*)ioremap(VENC_QOS_EXTCONTROL, 4);
	if(!qos_addr) {
		HI_FATAL_VENC("ioremap VENC_QOS_EXTCONTROL reg failed! \n");
		return HI_FAILURE;
	}
	writel(g_VencQosExtcontrol, qos_addr);
	iounmap(qos_addr);

	return HI_SUCCESS;
}
#endif

HI_S32 Venc_Regulator_Init(struct platform_device *pdev)
{
	HI_S32 ret = 0;

	if (!pdev) {
		HI_FATAL_VENC("invalid argument\n");
		return HI_FAILURE;
	}

	/* 1 get regulator interface */
	ret = Venc_Regulator_Get(pdev);
	if (ret < 0) {
		HI_FATAL_VENC("get venc regulator failed\n");
		return HI_FAILURE;
	}

	/* 2 create smmu domain */
	ret = Venc_Enable_Iommu();
	if (ret < 0) {
		HI_FATAL_VENC("enable venc iommu failed\n");
		return HI_FAILURE;
	}

	/* 3 read venc dts info from dts */
	HiMemSet(&g_VencDtsConfig, 0, sizeof(VeduEfl_DTS_CONFIG_S));
	ret = Venc_GetDtsConfigInfo(pdev, &g_VencDtsConfig);
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("get venc DTS config info failed\n");
		return HI_FAILURE;
	}

	/* 4 set dts into to efi */
	ret = VENC_SetDtsConfig(&g_VencDtsConfig);
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("set venc DTS config info failed\n");
		return HI_FAILURE;
	}

	return HI_SUCCESS;
}

HI_VOID Venc_Regulator_Deinit(struct platform_device *pdev)
{
	if (pdev)
		Venc_Disable_Iommu();
}

HI_S32 Venc_Regulator_Enable(HI_VOID)
{
	HI_S32 ret = HI_FAILURE;

	if (HI_TRUE == g_VencPowerOn) {
		return HI_SUCCESS;
	}

	if(IS_ERR_OR_NULL(g_PvencClk)
		|| IS_ERR_OR_NULL(g_VencRegulator)
		|| IS_ERR_OR_NULL(g_MediaRegulator)) {
		HI_FATAL_VENC("invalid_argument g_PvencClk:0x%pK, g_VencRegulator:0x%pK, g_MediaRegulator:0x%pK\n",
				g_PvencClk, g_VencRegulator, g_MediaRegulator);
		return HI_FAILURE;
	}

	ret = regulator_enable(g_MediaRegulator);
	if (ret != 0) {
		HI_FATAL_VENC("enable media regulator failed\n");
		return HI_FAILURE;
	}

	ret = clk_prepare_enable(g_PvencClk);
	if (ret != 0) {
		HI_FATAL_VENC("prepare clk enable failed\n");
		goto on_error_regulator;
	}

	ret = clk_set_rate(g_PvencClk, g_VencDtsConfig.lowRate);
	if(ret != 0) {
		HI_FATAL_VENC("set clk low rate failed\n");
		goto on_error_prepare_clk;
	}

	g_currClk = VENC_CLK_RATE_LOW;
	HI_INFO_VENC("clk_set_rate lowRate:%u\n", g_VencDtsConfig.lowRate);

	ret = regulator_enable(g_VencRegulator);
	if (ret != 0) {
		HI_FATAL_VENC("enable regulator failed\n");
		goto on_error_prepare_clk;
	}

	ret = clk_set_rate(g_PvencClk, g_VencDtsConfig.lowRate);
	if(ret != 0) {
		HI_FATAL_VENC("set clk low rate failed\n");
		goto on_error_set_rate;
	}
	g_VencPowerOn = HI_TRUE;

#ifdef VENC_QOS_CFG
	ret = Venc_Config_QOS();
	if (ret != HI_SUCCESS) {
		HI_FATAL_VENC("%s config qos failed\n", __func__);
		goto on_error_set_rate;
	}
#endif

	HI_INFO_VENC("++\n");
	return HI_SUCCESS;
on_error_set_rate:
	if (regulator_disable(g_VencRegulator))
		HI_INFO_VENC("disable media regulator failed\n");
on_error_prepare_clk:
	clk_disable_unprepare(g_PvencClk);
on_error_regulator:
	if (regulator_disable(g_MediaRegulator))
		HI_INFO_VENC("disable media regulator failed\n");

	return HI_FAILURE;
}

HI_S32 Venc_Regulator_Disable(HI_VOID)
{
	HI_S32 ret = HI_FAILURE;
	HI_INFO_VENC("Venc_Regulator_Disable g_VencRegulator:0x%pK\n", g_VencRegulator);

	if (HI_FALSE == g_VencPowerOn) {
		return HI_SUCCESS;
	}

	if(IS_ERR_OR_NULL(g_PvencClk)
		|| IS_ERR_OR_NULL(g_VencRegulator)
		|| IS_ERR_OR_NULL(g_MediaRegulator)) {
		HI_FATAL_VENC("invalid_argument g_PvencClk:0x%pK, g_VencRegulator:0x%pK, g_MediaRegulator:0x%pK\n",
				g_PvencClk, g_VencRegulator, g_MediaRegulator);
		return HI_FAILURE;
	}

	ret = regulator_disable(g_VencRegulator);
	if (ret != 0) {
		HI_ERR_VENC("disable regulator failed\n");
	}

	ret = clk_set_rate(g_PvencClk, g_VencDtsConfig.lowRate);
	if(ret != 0) {
		HI_ERR_VENC("set clk lowrate:%u failed\n", g_VencDtsConfig.lowRate);
		//return HI_FAILURE;//continue, no need return
	}
	HI_INFO_VENC("set clk lowRate:%u\n", g_VencDtsConfig.lowRate);
	g_currClk = VENC_CLK_RATE_LOW;

	clk_disable_unprepare(g_PvencClk);

	ret = regulator_disable(g_MediaRegulator);
	if (ret != 0) {
		HI_ERR_VENC("disable media regulator failed\n");
		return HI_FAILURE;
	}

	g_VencPowerOn = HI_FALSE;
	HI_INFO_VENC("--\n");
	return HI_SUCCESS;
}/*lint !e715 */

HI_S32 Venc_SetClkRate(VENC_CLK_TYPE clk_type)
{
	HI_S32 ret = HI_SUCCESS;
	HI_U32 needClk;
	HI_U32 currentClk;
	VENC_CLK_TYPE needClkType;

	if (g_currClk != clk_type) {
		switch (clk_type) {
		case VENC_CLK_RATE_LOW :
			needClk = g_VencDtsConfig.lowRate;
			break;

		case VENC_CLK_RATE_NORMAL :
			needClk = g_VencDtsConfig.normalRate;
			break;

		case VENC_CLK_RATE_HIGH :
			needClk = g_VencDtsConfig.highRate;
			break;

		default:
			HI_ERR_VENC("invalid clk type %d\n", clk_type);
			return HI_FAILURE;
		}

		HI_INFO_VENC("clk type %d, clk %u\n", clk_type, needClk);
		currentClk = clk_get_rate(g_PvencClk);
		if (needClk != currentClk) {
			needClkType = clk_type;
			ret = clk_set_rate(g_PvencClk, needClk);
			if ((0 != ret) && (needClk == g_VencDtsConfig.highRate)
				&& (currentClk != g_VencDtsConfig.normalRate)) {
				HI_WARN_VENC("failed set clk to %u Hz,fail code is %d, will reset high\n", needClk, ret);
				needClk = g_VencDtsConfig.normalRate;
				needClkType = VENC_CLK_RATE_NORMAL;
				ret = clk_set_rate(g_PvencClk, needClk);
			}

#ifdef VENC_AVS_LOW_CFG
			if ((0 != ret) && (needClk == g_VencDtsConfig.normalRate)
				&& (currentClk != g_VencDtsConfig.lowRate)) {
				HI_WARN_VENC("failed set clk to %u Hz,fail code is %d, will reset normal\n", needClk, ret);
				needClk = g_VencDtsConfig.lowRate;
				needClkType = VENC_CLK_RATE_LOW;
				ret = clk_set_rate(g_PvencClk, needClk);
			}
#endif
			if (0 == ret)
				g_currClk = needClkType;
			else
				HI_WARN_VENC("failed set clk to %u Hz,fail code is %d\n", needClk, ret);
		}
	}

	return ret;
}

