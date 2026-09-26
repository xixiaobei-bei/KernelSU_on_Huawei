/*
 *  driver, hisp_dtsi.c
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#include <linux/platform_device.h>
#include <linux/platform_data/remoteproc_hisp.h>
#include <linux/hisi/hisi_rproc.h>
#include <linux/delay.h>
#include <linux/kfifo.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/rproc_share.h>
#include <isp_ddr_map.h>
#include "hisp_internel.h"

#define BOARDID_SIZE    4
#define DTS_COMP_LOGIC_NAME   "hisilicon,isplogic"

#define INVALID_BOARDID    0xFFFFFFFF

#define HISP_SMEM_OFFSET   0xF000

#define HZ_TO_MHZ_DIV   1000000

#define hisp_min(a, b) (((a) < (b)) ? (a) : (b))

enum isp_isplogic_type_info_e {
	ISP_FPGA_EXCLUDE    = 0x0,
	ISP_FPGA,
	ISP_UDP,
	ISP_FPGA_EXC,
	ISP_MAXTYPE
};

static const char * const secmem_propname[] = {
	"a7-vaddr-boot",
	"a7-vaddr-text",
	"a7-vaddr-data",
	"a7-vaddr-pgd",
	"a7-vaddr-pmd",
	"a7-vaddr-pte",
	"a7-vaddr-rdr",
	"a7-vaddr-shrd",
	"a7-vaddr-vq",
	"a7-vaddr-vr0",
	"a7-vaddr-vr1",
	"a7-vaddr-heap",
	"a7-vaddr-a7dyna",
	"a7-vaddr-mdc",
};

static const char * const ca_bootmem_propname[] = {
	"ispcpu-text",
	"ispcpu-data",
};

static const char * const ca_rsvmem_propname[] = {
	"ispcpu-vr0",
	"ispcpu-vr1",
	"ispcpu-vq",
	"ispcpu-shrd",
	"ispcpu-rdr",
};

static const char * const ca_mempool_propname[] = {
	"ispcpu-dynamic-pool",
	"ispcpu-sec-pool",
	"ispcpu-ispsec-pool",
};

static int hisp_pwr_regulator_getdts(struct device *device,
			struct hisp_pwr *dev)
{
	struct device_node *np = device->of_node;
	int ret;

	dev->clockdep_supply = devm_regulator_get(device, "isp-clockdep");
	if (IS_ERR(dev->clockdep_supply)) {
		pr_err("[%s] Failed : isp-clockdep devm_regulator_get.%pK\n",
				__func__, dev->clockdep_supply);
		return -EINVAL;
	}

	dev->ispcore_supply = devm_regulator_get(device, "isp-core");
	if (IS_ERR(dev->ispcore_supply)) {
		pr_err("[%s] Failed : isp-core devm_regulator_get.%pK\n",
				__func__, dev->ispcore_supply);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "ispcpu-supply-flag",
			(unsigned int *)(&dev->ispcpu_supply_flag));
	if (ret < 0) {
		dev->ispcpu_supply_flag = 0;
		pr_err("[%s] Failed: ispcpu-supply-flag get from dtsi.%d\n",
			__func__, ret);
	}

	if (dev->ispcpu_supply_flag) {
		dev->ispcpu_supply = devm_regulator_get(device, "isp-cpu");
		if (IS_ERR(dev->ispcpu_supply))
			pr_err("[%s] Failed : isp-cpu devm_regulator_get.%pK\n",
					__func__, dev->ispcpu_supply);
	}

	ret = of_property_read_u32(np, "use-buck13-flag",
			(unsigned int *)(&dev->use_buck13_flag));
	if (ret < 0) {
		dev->use_buck13_flag = 0;
		pr_err("[%s] Failed: use-buck13-flag get from dtsi.%d\n",
			__func__, ret);
	}

	ret = of_property_read_u32(np, "use-smmuv3-flag",
				(unsigned int *)(&dev->use_smmuv3_flag));
	if (ret < 0) {
		pr_err("[%s] Failed: use_smmuv3_flag get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}
	pr_info("[%s] use_smmuv3_flag.0x%x\n", __func__,
			dev->use_smmuv3_flag);

	if (dev->use_smmuv3_flag) {
		dev->isptcu_supply = devm_regulator_get(device, "isp-smmu-tcu");
		if (IS_ERR(dev->isptcu_supply)) {
			pr_err("[%s] Failed : isp-tcu devm_regulator_get.%pK\n",
					__func__, dev->isptcu_supply);
			return -EINVAL;
		}
	}

	return 0;
}

static int hisp_pwr_clock_getdts(struct device_node *np,
			struct hisp_pwr *dev)
{
	int ret;

	ret = of_property_read_u32(np, "clock-num",
				(unsigned int *)(&dev->clock_num));
	if (ret < 0) {
		pr_err("[%s] Failed: clock-num get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_string_array(np, "clock-names",
				dev->clk_name, dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed : clock-names get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	pr_info("[%s] CLOCKVALUE.%s\n", __func__, CLOCKVALUE);
	ret = of_property_read_u32_array(np, CLOCKVALUE,
				dev->ispclk_value, dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed: clock-value get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}


	ret = of_property_read_u32_array(np, "clkdis-dvfs",
				dev->clkdis_dvfs, dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed: clkdis-dvfs get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "clkdis-need-div",
				dev->clkdis_need_div, dev->clock_num);
	if (ret < 0)
		pr_err("[%s] Failed: clkdis-need-div get from dtsi.%d\n",
				__func__, ret);

	return 0;
}

static int hisp_pwr_dvfs_getdts(struct device_node *np,
			struct hisp_pwr *dev)
{
	int ret;

	ret = of_property_read_u32(np, "usedvfs",
				(unsigned int *)(&dev->usedvfs));
	if (ret < 0) {
		pr_err("[%s] Failed: usedvfs get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}
	pr_info("[%s] usedvfs.0x%x\n", __func__, dev->usedvfs);

	if (dev->usedvfs == 0)
		return 0;

	ret = of_property_read_u32_array(np, "clock-value",
			dev->clkdn[HISP_CLK_TURBO], dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed: TURBO get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "clkdn-normal",
			dev->clkdn[HISP_CLK_NORMINAL], dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed: NORMINAL get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "clkdn-highsvs",
			dev->clkdn[HISP_CLK_HIGHSVS], dev->clock_num);
	if (ret < 0)
		pr_err("[%s]Failed: HIGHSVS get from dtsi.%d\n", __func__, ret);

	ret = of_property_read_u32_array(np, "clkdn-svs",
			dev->clkdn[HISP_CLK_SVS], dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed: SVS get from dtsi.%d\n", __func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "clkdn-lowsvs",
			dev->clkdn[HISP_CLK_LOWSVS], dev->clock_num);
	if (ret < 0)
		pr_err("[%s] Failed: LOW SVS get from dtsi.%d\n",
				__func__, ret);

	ret = of_property_read_u32_array(np, "clkdis-dvfs",
			dev->clkdn[HISP_CLK_DISDVFS], dev->clock_num);
	if (ret < 0) {
		pr_err("[%s] Failed: SVS get from dtsi.%d\n", __func__, ret);
		return -EINVAL;
	}

	return 0;
}

static int hisp_pwstat_getdts(struct device_node *np, struct hisp_pwr *dev)
{
	int ret;
	unsigned int pwsta_num = 0;
	unsigned int i;

	if (np == NULL)
		return -ENODEV;

	ret = of_property_read_u32(np, "pwstat-num",
				(unsigned int *)(&dev->pwstat_num));
	if (ret < 0) {
		pr_err("[%s] Failed: pwstat-num get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	pwsta_num = dev->pwstat_num > PWSTAT_MAX ? PWSTAT_MAX : dev->pwstat_num;
	pr_info("[%s] pwstat_num.0x%x\n", __func__,
			dev->pwstat_num);

	ret = of_property_read_u32_array(np, "pwstat-type",
				dev->pwstat_type, pwsta_num);
	if (ret < 0) {
		pr_err("[%s] Failed: pwstat-type get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "pwstat-offset",
				dev->pwstat_offset, pwsta_num);
	if (ret < 0) {
		pr_err("[%s] Failed: pwstat-offset get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "pwstat-bit",
				dev->pwstat_bit, pwsta_num);
	if (ret < 0) {
		pr_err("[%s] Failed: pwstat-bit get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, "pwstat-wanted",
				dev->pwstat_wanted, pwsta_num);
	if (ret < 0) {
		pr_err("[%s] Failed: pwstat-wanted get from dtsi.%d\n",
				__func__, ret);
		return -EINVAL;
	}
	for (i = 0; i < pwsta_num; i++)
		pr_info("[%s] : type.%d, offset.0x%x, bit.%d, wanted[%d].%d\n",
			__func__, i, dev->pwstat_type[i], dev->pwstat_offset[i],
			dev->pwstat_bit[i], dev->pwstat_wanted[i]);

	return 0;
}

static int hisp_secmem_reserved_getdts(int *flag)
{
	struct device_node *nod = NULL;
	const char *status = NULL;

	if (flag == NULL) {
		pr_err("[%s] input flag parameter is NULL!\n", __func__);
		return -ENODEV;
	}
	*flag = 0;

	nod = of_find_node_by_path("/reserved-memory/sec_camera");
	if (nod  == NULL) {/*lint !e838 */
		pr_err("[%s] Failed : of_find_node_by_path.%pK\n",
				__func__, nod);
		return -ENODEV;
	}

	if (of_property_read_string(nod, "status", &status)) {
		pr_err("[%s] Failed : of_property_read_string status\n",
				__func__);
		*flag = 1;
		return -ENODEV;
	}

	if (status && strncmp(status, "disabled", strlen("disabled")) != 0)
		*flag = 1;

	pr_err("[%s] trusted_ispmem_reserved %s\n", __func__, status);
	return 0;
}

static int hisp_secmem_rsctable_getdts(struct device_node *np,
			struct hisp_sec *dev)
{
	int ret = 0;

	ret = of_property_read_u32(np, "rsctable-mem-offet",
			(unsigned int *)(&dev->rsctable_offset));
	if (ret < 0) {
		pr_err("[%s] Failed: rsctable_offset.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("rsctable_offset.0x%x of_property_read_u32.%d\n",
			dev->rsctable_offset, ret);

	ret = of_property_read_u32(np, "rsctable-mem-size",
			(unsigned int *)(&dev->rsctable_size));
	if (ret < 0) {
		pr_err("[%s] Failed: rsctable_size.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("rsctable_size.0x%x of_property_read_u32.%d\n",
			dev->rsctable_size, ret);

	return 0;
}

static int hisp_secmem_trusted_getdts(struct device_node *np,
			struct hisp_sec *dev)
{
	unsigned int offset_num   = 0;
	unsigned int index = 0;
	int ret = 0;

	if (!sec_process_use_ca_ta()) {
		ret = of_property_read_u32(np, "trusted-smem-size",
				(unsigned int *)(&dev->trusted_mem_size));
		if (ret < 0) {
			pr_err("[%s] Failed: trusted_mem_size.ret.%d\n",
					__func__, ret);
			return -EINVAL;
		}
		pr_info("trusted_mem_size.0x%x of_property_read_u32.%d\n",
				dev->trusted_mem_size, ret);
	}

	ret = of_property_read_u32(np, "share-smem-size",
			(unsigned int *)(&dev->share_mem_size));
	if (ret < 0) {
		pr_err("[%s] Failed: share_mem_size.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("share_mem_size.0x%x of_property_read_u32.%d\n",
			dev->share_mem_size, ret);

	ret = of_property_read_u32(np, "trusted-smem-num",
			(unsigned int *)(&offset_num));
	if (ret < 0) {
		pr_err("[%s] Failed: tsmem-offset-num.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("tsmem-offset-num.0x%x of_property_read_u32.%d\n",
			offset_num, ret);

	if (offset_num <= ISP_TSMEM_MAX) {
		ret = of_property_read_u32_array(np, "trusted-smem-offset",
					dev->tsmem_offset, offset_num);
		if (ret < 0) {
			pr_info("[%s] Fail : trusted-smem-offset.ret.%d\n",
					__func__, ret);
			return -EINVAL;
		}

		for (index = 0; index < offset_num; index++)
			pr_info("[%s] trusted-smem-offset %d offest = 0x%x\n",
				__func__, index, dev->tsmem_offset[index]);
	}

	ret = hisp_secmem_rsctable_getdts(np, dev);
	if (ret < 0)
		pr_err("[%s] Failed: rsctable_getdts.ret.%d\n", __func__, ret);

	return ret;
}

static int hisp_sec_early_getdts(struct platform_device *pdev,
			struct hisp_sec *dev)
{
	struct device *device = &pdev->dev;
	struct device_node *np = device->of_node;
	int ret = 0;

	ret = hisp_secmem_reserved_getdts(&dev->ispmem_reserved);
	if (ret < 0)
		pr_err("[%s] Failed : ispmem_reserved.%d.\n", __func__, ret);
	pr_info("[%s] ispmem_reserved.%d\n", __func__, dev->ispmem_reserved);

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	ret = of_property_read_u32(np, "use-ca-ta", &dev->use_ca_ta);
	if (ret < 0) {
		pr_err("Failed : use_ca_ta.%d.ret.%d\n", dev->use_ca_ta, ret);
		return -EINVAL;
	}
	pr_info("use_ca_ta.%d of_property_read_u32.%d\n", dev->use_ca_ta, ret);

	ret = of_property_read_u32(np, "isp-iova-start",
		(unsigned int *)(&dev->isp_iova_start));
	if (ret < 0) {
		pr_err("Failed : isp_iova_addr.0x%lx.ret.%d\n",
			dev->isp_iova_start, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "isp-iova-size",
		(unsigned int *)(&dev->isp_iova_size));
	if (ret < 0) {
		pr_err("Failed : isp_iova_size.0x%lx.ret.%d\n",
			dev->isp_iova_size, ret);
		return -EINVAL;
	}

	pr_info("isp_iova_addr.0x%lx isp_iova_size.0x%lx\n",
			dev->isp_iova_start, dev->isp_iova_size);

	ret = hisp_secmem_trusted_getdts(np, dev);
	if (ret < 0) {
		pr_err("Failed : hisp_secmem_trusted_getdts.%d\n", ret);
		return ret;
	}
	return 0;
}

static unsigned int hisp_get_isplogic(void)
{
	struct device_node *np = NULL;
	char *name = NULL;
	unsigned int isplogic = 1;
	int ret = 0;

	name = DTS_COMP_LOGIC_NAME;

	np = of_find_compatible_node(NULL, NULL, name);
	if (np == NULL) {
		pr_err("%s: of_find_compatible_node failed, %s\n",
				__func__, name);
		return ISP_UDP;
	}

	ret = of_property_read_u32(np, "hisi,isplogic",
			(unsigned int *)(&isplogic));
	if (ret < 0) {
		pr_err("[%s] Failed: isplogic of_property_read_u32.%d\n",
				__func__, ret);
		return ISP_FPGA_EXC;
	}

	pr_info("[%s] isplogic.0x%x\n", __func__, isplogic);
	return isplogic;
}

static unsigned int hisp_get_boardid(struct rproc_boot_device *dev)
{
	unsigned int boardid[BOARDID_SIZE] = {0};
	struct device_node *root = NULL;
	unsigned int stat10 = 0;
	unsigned int isplogic_type = 0;
	int ret = 0;
	const char *str = "ISP/Camera Power Up and Down May be Bypassed";

	dev->bypass_pwr_updn = 0;
	root = of_find_node_by_path("/");
	if (root == NULL) /*lint !e838 */
		goto err_get_bid;

	ret = of_property_read_u32_array(root, "hisi,boardid",
				boardid, BOARDID_SIZE);
	if (ret < 0)
		goto err_get_bid;

	pr_info("Board ID.(%x %x %x %x)\n",
			boardid[0], boardid[1], boardid[2], boardid[3]);

	isplogic_type = hisp_get_isplogic();
	switch (isplogic_type) {
	case ISP_FPGA_EXCLUDE:
		pr_err(" %s isplogic state.%d\n", str, isplogic_type);
		dev->bypass_pwr_updn = 1;
		break;
	case ISP_FPGA:
		if (dev->reg[PCTRL] == NULL) {/*lint !e747  */
			pr_err("Failed : pctrl_base ioremap.\n");
			break;
		}
		stat10 = readl(dev->reg[PCTRL] +
				ISP_PCTRL_PERI_STAT_ADDR);/*lint !e732 !e529 */
		pr_alert("ISP/Camera pctrl_stat10.0x%x\n", stat10);
		if ((stat10 & (ISP_PCTRL_PERI_FLAG)) == 0) {
			pr_err("%s, pctrl_stat10.0x%x\n", str, stat10);
			dev->bypass_pwr_updn = 1;
		}
		break;
	case ISP_UDP:
	case ISP_FPGA_EXC:
		break;
	default:
		pr_err("ERROR: isplogic state.%d\n", isplogic_type);
		break;
	}
	return boardid[0];

err_get_bid:
	pr_err("Failed : find_node.%pK, property_read.%d\n", root, ret);
	return INVALID_BOARDID;/*lint !e570 */
}

static int isp_rproc_data_name_getdts(struct device_node *np,
			struct isp_rproc_data *data)
{
	int ret;

	ret = of_property_read_string(np, "isp-names", &data->name);
	if (ret < 0) {
		pr_err("[%s] Failed : isp-names.%s ret.%d\n",
			__func__, data->name, ret);
		return -EINVAL;
	}

	ret = of_property_read_string(np, "firmware-names", &data->firmware);
	if (ret < 0) {
		pr_err("[%s] Failed : firmware-names.%s ret.%d\n",
			__func__, data->firmware, ret);
		return -EINVAL;
	}

	ret = of_property_read_string(np, "mailbox-names", &data->mbox_name);
	if (ret < 0) {
		pr_err("[%s] Failed : mailbox-names.%s ret.%d\n",
			__func__, data->mbox_name, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "isp-ipc-addr", &data->ipc_addr);
	if (ret < 0) {
		pr_err("[%s] Failed : ipc_addr.0x%x ret.%d\n",
			__func__, data->ipc_addr, ret);
		return -EINVAL;
	}
	pr_debug("[%s] : isp-ipc-addr.%u\n", __func__, data->ipc_addr);


	return ret;
}

static int isp_rproc_dev_getdts(struct device *pdev,
			struct rproc_boot_device *dev)
{
	struct device_node *np = pdev->of_node;
	unsigned int platform_info;
	unsigned int addr;
	int ret;

	ret = of_property_read_u32(np, "isp-remap-addr", &addr);
	if (ret < 0) {
		pr_err("[%s] Failed : remapaddr.0x%x ret.%d\n",
			__func__, addr, ret);
		return -EINVAL;
	}
	dev->remap_addr = addr;

	ret = of_property_read_u32(np, "isp_local_timer", &platform_info);
	if (ret < 0) {
		pr_err("[%s] Failed: isp_local_timer.0x%x ret.%d\n",
			__func__, platform_info, ret);
		return -EINVAL;
	}
	dev->tmp_plat_cfg.isp_local_timer = platform_info;
	pr_info("[%s] isp_local_timer = %d\n", __func__, platform_info);

	return 0;
}


static int isp_rproc_data_flag_getdts(struct device_node *np,
			struct rproc_boot_device *dev)
{
	unsigned int isp_efuse_flag = 0;
	int ret;

	ret = of_property_read_u32(np, "isppd-adb-flag", &dev->isppd_adb_flag);
	if (ret < 0) {
		pr_err("Failed: adb-flag.0x%x ret.%d\n",
			dev->isppd_adb_flag, ret);
		return -EINVAL;
	}
	pr_info("isppd_adb_flag.%d\n", dev->isppd_adb_flag);

	ret = of_property_read_u32(np, "isp-efuse-flag", &isp_efuse_flag);
	if (ret < 0) {
		pr_err("Failed: efuse-flag.%d ret.%d\n", isp_efuse_flag, ret);
		return -EINVAL;
	}
	dev->isp_efuse_flag = isp_efuse_flag;
	pr_info("isp_efuse_flag.%d\n", dev->isp_efuse_flag);

	ret = of_property_read_u32(np, "logb-flag", &dev->use_logb_flag);
	if (ret < 0) {
		pr_err("Failed: logb-flag.0x%x ret.%d\n",
			dev->use_logb_flag, ret);
		return -EINVAL;
	}
	pr_info("use_logb_flag.%d\n", dev->use_logb_flag);

	ret = of_property_read_u32(np, "pwronisp-nice",
				(unsigned int *)(&dev->pwronisp_nice));
	if (ret < 0) {
		pr_err("[%s] Failed: pwronisp-nice.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("pwronisp-nice.0x%x of_property_read_u32.%d\n",
			dev->pwronisp_nice, ret);

	return 0;
}

static int hisp_sec_ori_mem_getdts(struct hisp_sec *dev,
		unsigned int etype, unsigned long paddr)
{
	struct device_node *np = dev->device->of_node;
	unsigned int offset = 0;
	int ret = 0;

	if ((np == NULL) || (dev->shrdmem == NULL) || (etype >= MAXA7MAPPING)) {
		pr_err("[%s] Failed: np.%pK, shrdmem.%pK, etype.0x%x >= 0x%x\n",
			__func__, np, dev->shrdmem, etype, MAXA7MAPPING);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, secmem_propname[etype],
		(unsigned int *)(&dev->shrdmem->a7mapping[etype]),
		dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
				__func__, secmem_propname[etype], ret);
		return -EINVAL;
	}
	pr_info("[%s] propname.%s, array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
		__func__, secmem_propname[etype], ret,
		dev->shrdmem->a7mapping[etype].a7va,
		dev->shrdmem->a7mapping[etype].size,
		dev->shrdmem->a7mapping[etype].prot,
		dev->shrdmem->a7mapping[etype].offset,
		dev->shrdmem->a7mapping[etype].reserve);

	if (dev->shrdmem->a7mapping[etype].reserve) {
		ret = hisp_mem_type_pa_init(etype, paddr);
		if (ret < 0) {
			pr_info("[%s] hisp_mem_type_pa_init fail.%d, type.%d\n",
				__func__, ret, etype);
			return -EINVAL;
		}
	}

	if (etype == A7DYNAMIC) {
		offset = dev->tsmem_offset[ISP_DYN_MEM_OFFSET];
		dev->shrdmem->a7mapping[etype].a7pa
			= dev->atfshrd_paddr + offset;
		dev->ap_dyna_array = offset + dev->atfshrd_vaddr;
		dev->ap_dyna = &dev->shrdmem->a7mapping[etype];
		pr_info("[%s] etype.%d, atfshrd_vaddr.%pK, offset.0x%x\n",
				__func__, etype,  dev->atfshrd_vaddr, offset);
	} else if (etype == A7HEAP) {
		dev->is_heap_flag = dev->shrdmem->a7mapping[etype].reserve;
	} else if (etype == A7SHARED) {
		dev->shrdmem->a7mapping[etype].apva = getsec_a7sharedmem_addr();
	}

	return 0;
}

static int hisp_sec_ca_bootmem_getdts(struct hisp_sec *dev,
		unsigned int etype)
{
	struct device_node *np = dev->device->of_node;
	int ret = 0;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -EINVAL;
	}

	if (etype >= HISP_SEC_BOOT_MAX_TYPE) {
		pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
			__func__, etype, HISP_SEC_BOOT_MAX_TYPE);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, ca_bootmem_propname[etype],
		(unsigned int *)(&dev->boot_mem[etype]), dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
			__func__, ca_bootmem_propname[etype], ret);
		return -EINVAL;
	}

	pr_info("[%s] propname.%s, array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			__func__, ca_bootmem_propname[etype], ret,
			dev->boot_mem[etype].type, dev->boot_mem[etype].da,
			dev->boot_mem[etype].size, dev->boot_mem[etype].prot,
			dev->boot_mem[etype].sec_flag);

	return 0;
}

static int hisp_sec_ca_rsvmem_getdts(struct hisp_sec *dev,
		unsigned int etype)
{
	struct device_node *np = dev->device->of_node;
	int ret = 0;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -EINVAL;
	}

	if (etype >= HISP_SEC_RSV_MAX_TYPE) {
		pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
			__func__, etype, HISP_SEC_RSV_MAX_TYPE);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, ca_rsvmem_propname[etype],
		(unsigned int *)(&dev->rsv_mem[etype]), dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
			__func__, ca_rsvmem_propname[etype], ret);
		return -EINVAL;
	}

	pr_info("[%s] propname.%s, array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			__func__, ca_rsvmem_propname[etype], ret,
			dev->rsv_mem[etype].type, dev->rsv_mem[etype].da,
			dev->rsv_mem[etype].size, dev->rsv_mem[etype].prot,
			dev->rsv_mem[etype].sec_flag);

	return 0;
}

static int hisp_sec_ca_mempool_getdts(struct hisp_sec *dev,
		unsigned int etype)
{
	struct device_node *np = dev->device->of_node;
	int ret = 0;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -EINVAL;
	}

	if (etype >= HISP_SEC_POOL_MAX_TYPE) {
		pr_err("[%s] Failed : etype.(0x%x >= 0x%x)\n",
			__func__, etype, HISP_SEC_POOL_MAX_TYPE);
		return -EINVAL;
	}

	ret = of_property_read_u32_array(np, ca_mempool_propname[etype],
		(unsigned int *)(&dev->pool_mem[etype]), dev->mapping_items);
	if (ret < 0) {
		pr_info("[%s] propname.%s, of_property_read_u32_array.%d\n",
				__func__, ca_mempool_propname[etype], ret);
		return -EINVAL;
	}
	pr_info("[%s] propname.%s, array.%d.(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n",
			__func__, ca_mempool_propname[etype], ret,
			dev->pool_mem[etype].type, dev->pool_mem[etype].da,
			dev->pool_mem[etype].size, dev->pool_mem[etype].prot,
			dev->pool_mem[etype].sec_flag);

	return 0;
}

int hisp_pwr_getdts(struct platform_device *pdev, struct hisp_pwr *dev)
{
	struct device *device = &pdev->dev;
	struct device_node *np = device->of_node;
	int ret;
	unsigned int i;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);
	ret = hisp_pwr_regulator_getdts(device, dev);
	if (ret < 0) {
		pr_err("[%s] Failed: regulator_getdts.%d\n", __func__, ret);
		return ret;
	}

	ret = hisp_pwr_clock_getdts(np, dev);
	if (ret < 0) {
		pr_err("[%s] Failed: clock_getdts.%d\n", __func__, ret);
		return ret;
	}

	ret = hisp_pwr_dvfs_getdts(np, dev);
	if (ret < 0) {
		pr_err("[%s] Failed: hisp_pwr_dvfs_getdts.%d\n", __func__, ret);
		return ret;
	}

	for (i = 0; i < dev->clock_num; i++) {
		dev->ispclk[i] = devm_clk_get(device, dev->clk_name[i]);
		if (IS_ERR_OR_NULL(dev->ispclk[i])) {
			pr_err("[%s] Failed : ispclk.%s.%d.%li\n", __func__,
				dev->clk_name[i], i, PTR_ERR(dev->ispclk[i]));
			return -EINVAL;
		}
		pr_info("[%s] ISP clock.%d.%s: %d.%d M\n", __func__, i,
			dev->clk_name[i], dev->ispclk_value[i] / HZ_TO_MHZ_DIV,
			dev->ispclk_value[i] % HZ_TO_MHZ_DIV);

		pr_info("[%s] clkdis.%d.%s: %d.%d M\n", __func__, i,
			dev->clk_name[i], dev->clkdis_dvfs[i] / HZ_TO_MHZ_DIV,
			dev->clkdis_dvfs[i] % HZ_TO_MHZ_DIV);
	}

	ret = hisp_pwstat_getdts(np, dev);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_pwstat_getdts.%d.\n", __func__, ret);
		return ret;
	}

	pr_info("[%s] -\n", __func__);

	return 0;
}

int hisp_nsec_getdts(struct platform_device *pdev, struct hisp_nsec *dev)
{
	struct device *device = &pdev->dev;
	struct device_node *np = device->of_node;
	int ret;

	if (np == NULL) {
		pr_err("[%s] Failed : np.%pK\n", __func__, np);
		return -ENODEV;
	}

	pr_info("[%s] +\n", __func__);
	dev->device = device;
	set_a7sharedmem_pa(dev->isp_dma + HISP_SMEM_OFFSET);
	set_a7sharedmem_va(dev->isp_dma_va + HISP_SMEM_OFFSET);
	set_a7mem_pa(dev->isp_dma);
	pr_info("[%s] dma_va.%pK\n", __func__, dev->isp_dma_va);

	ret = hisp_cvdr_getdts(np);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_cvdr_getdts.%d.\n",
				__func__, ret);
		return ret;
	}

	ret = hisp_mdc_getdts(np);
	if (ret != 0) {
		pr_err("[%s] Failed : hisp_cvdr_getdts.%d.\n",
				__func__, ret);
		return ret;
	}

	pr_info("[%s] -\n", __func__);

	return 0;
}

int hisp_sec_getdts(struct platform_device *pdev, struct hisp_sec *dev)
{
	struct device *device = &pdev->dev;
	struct device_node *np = device->of_node;
	int ret = 0;

	ret = hisp_sec_early_getdts(pdev, dev);
	if (ret < 0) {
		pr_err("[%s] Failed: isp_atf_mem_getdts.%d\n", __func__, ret);
		return -EINVAL;
	}

	ret = of_property_read_u32(np, "isp-wdt-flag",
			(unsigned int *)(&dev->isp_wdt_flag));
	if (ret < 0) {
		pr_err("[%s] Failed: isp-wdt-flag.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("isp-wdt-flag.0x%x of_property_read_u32.%d\n",
			dev->isp_wdt_flag, ret);

	ret = of_property_read_u32(np, "isp-reset-flag",
			(unsigned int *)(&dev->isp_cpu_reset));
	if (ret < 0) {
		pr_err("[%s] Failed: isp-reset-flag.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("isp-reset-flag.0x%x of_property_read_u32.%d\n",
			dev->isp_cpu_reset, ret);

	ret = of_property_read_u32(np, "mapping-items",
			(unsigned int *)(&dev->mapping_items));
	if (ret < 0) {
		pr_err("[%s] Failed: mapping-num.ret.%d\n", __func__, ret);
		return -EINVAL;
	}
	pr_info("mapping-items.0x%x.ret.%d\n", dev->mapping_items, ret);

	return 0;
}

int hisp_secmem_getdts(struct hisp_sec *dev)
{
	int index;
	int ret;

	if (dev == NULL) {
		pr_err("[%s] Failed : dev.%pK\n", __func__, dev);
		return -EPERM;
	}

	if (!sec_process_use_ca_ta()) {
		for (index = 0; index < MAXA7MAPPING; index++) {
			ret = hisp_sec_ori_mem_getdts(dev, index, 0);
			if (ret < 0)
				pr_err("[%s] Failed: hisp_meminit.%d\n",
					__func__, ret);
		}
	} else {
		for (index = 0; index < (HISP_SEC_BOOT_MAX_TYPE); index++) {
			ret = hisp_sec_ca_bootmem_getdts(dev, index);
			if (ret < 0)
				pr_err("[%s] Failed: ca_bootmem_getdts.%d\n",
					__func__, ret);
		}

		for (index = 0; index < HISP_SEC_RSV_MAX_TYPE; index++) {
			ret = hisp_sec_ca_rsvmem_getdts(dev, index);
			if (ret < 0)
				pr_err("[%s] Failed: ca_rsvmem_getdts.%d\n",
					__func__, ret);
		}

		for (index = 0; index < HISP_SEC_POOL_MAX_TYPE; index++) {
			ret = hisp_sec_ca_mempool_getdts(dev, index);
			if (ret < 0)
				pr_err("[%s] Failed: ca_mempool_getdts.%d\n",
					__func__, ret);
		}
	}

	return 0;
}
/*lint -save -e429 -e446*/
int hisp_rproc_regaddr_getdts(struct platform_device *pdev,
		struct rproc_boot_device *rproc_dev)
{
	struct device *dev = NULL;
	struct device_node *np = NULL;
	int ret = 0;
	unsigned int i;
	unsigned int min_num;

	if (pdev == NULL)
		return -ENXIO;

	dev = &pdev->dev;
	if (dev == NULL) {
		pr_err("[%s] Failed : No device\n", __func__);
		return -ENXIO;
	}

	np = dev->of_node;
	if (np == NULL) {
		pr_err("[%s] Failed : No dt node\n", __func__);
		return -ENXIO;
	}

	ret = of_property_read_u32(np, "reg-num",
		(unsigned int *)(&rproc_dev->reg_num));
	if (ret < 0) {
		pr_err("[%s] Failed: reg-num,ret.%d\n", __func__, ret);
		return -EINVAL;
	}

	min_num = hisp_min(HISP_MAX_REGTYPE, rproc_dev->reg_num);

	for (i = 0; i < min_num; i++) {
		rproc_dev->r[i] = platform_get_resource(pdev,
					IORESOURCE_MEM, i);
		if (rproc_dev->r[i] == NULL) {
			pr_err("[%s] Failed : platform_get_resource.%pK\n",
					__func__, rproc_dev->r[i]);
			goto error;
		}

		rproc_dev->reg[i] = ioremap(rproc_dev->r[i]->start,
			resource_size(rproc_dev->r[i]));//lint !e442
		if (rproc_dev->reg[i] == NULL) {
			pr_err("[%s] resource.%d ioremap fail\n", __func__, i);
			goto error;
		}
	}

	return 0;

error:
	for (; i < min_num; i--) {//lint !e442
		if (rproc_dev->reg[i] != NULL) {
			iounmap(rproc_dev->reg[i]);
			rproc_dev->reg[i] = NULL;
		}
	}
	return -ENOMEM;
}

void hisp_rproc_regaddr_putdts(struct rproc_boot_device *rproc_dev)
{
	unsigned int i;
	unsigned int min_num;

	pr_info("+\n");
	min_num = hisp_min(HISP_MAX_REGTYPE, rproc_dev->reg_num);
	for (i = 0; i < min_num; i++) {
		if (rproc_dev->reg[i]) {
			iounmap(rproc_dev->reg[i]);
			rproc_dev->reg[i] = NULL;
		}
	}
	pr_info("-\n");
}

struct isp_rproc_data *isp_rproc_data_getdts(struct device *pdev,
			struct rproc_boot_device *dev)
{
	struct device_node *np = pdev->of_node;
	struct isp_rproc_data *data = NULL;
	int ret;

	if (np == NULL) {
		pr_err("Failed : No dt node\n");
		return NULL;
	}

	dev->boardid = hisp_get_boardid(dev);
	if (dev->boardid == INVALID_BOARDID)/*lint !e650 */
		return NULL;

	dev->ap_a7_mbox = HISI_ACPU_ISP_MBX_1;
	dev->a7_ap_mbox = HISI_ISP_ACPU_MBX_1;

	data = devm_kzalloc(pdev, sizeof(struct isp_rproc_data), GFP_KERNEL);
	if (data == NULL)
		return NULL;

	ret = isp_rproc_data_name_getdts(np, data);
	if (ret < 0) {
		pr_err("Failed : isp_rproc_data_name_getdts ret.%d\n", ret);
		goto isp_rproc_fail;
	}

	ret = isp_rproc_dev_getdts(pdev, dev);
	if (ret < 0) {
		pr_err("Failed : isp_rproc_dev_getdts ret.%d\n", ret);
		goto isp_rproc_fail;
	}

	ret = isp_rproc_data_flag_getdts(np, dev);
	if (ret < 0) {
		pr_err("Failed : isp_rproc_data_flag_getdts ret.%d\n", ret);
		goto isp_rproc_fail;
	}

	ret = hisp_qos_dtget(np);
	if (ret < 0) {
		pr_err("Failed : hisp_qos_dtget ret.%d\n", ret);
		goto isp_rproc_fail;
	}

	return data;
isp_rproc_fail:
	devm_kfree(pdev, data);
	return NULL;
}

void isp_rproc_data_putdts(struct device *pdev)
{
	hisp_qos_free();
}
/*lint -restore */
