/*
 * Hisilicon IPP Head File
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _LINUX_HIPP_COMMON_H_
#define _LINUX_HIPP_COMMON_H_

enum hipp_tbu_type_e {
	HIPP_TBU_IPP = 0,
	HIPP_TBU_TOF = 1,
	MAX_HIPP_TBU
};

enum hipp_clocklevel_type_e {
	CLK_RATE_TURBO  = 0, //640
	CLK_RATE_NORMAL = 1, //480
	CLK_RATE_HSVS   = 2, //400
	CLK_RATE_SVS    = 3, //277
	CLK_RATE_OFF    = 4, //104
	MAX_CLK_RATE
};

enum hipp_common_type_e {
	HISI_IPP_UNIT     = 0,
	HISI_JPEGE_UNIT   = 1,
	HISI_JPEGD_UNIT   = 2,
	HISI_ANF_UNIT     = 3,
	MAX_HIPP_COM
};

struct hipp_common_s {
	int initialized;
	unsigned int type;
	unsigned int tbu_mode;
	const char *name;
	struct device *dev;
	void *comdev;
	int (*power_up)(struct hipp_common_s *drv);
	int (*power_dn)(struct hipp_common_s *drv);
	int (*set_jpgclk_rate)(struct hipp_common_s *drv, unsigned int clktype);
	int (*enable_smmu)(struct hipp_common_s *drv);
	int (*disable_smmu)(struct hipp_common_s *drv);
	int (*setsid_smmu)(struct hipp_common_s *drv, unsigned int swid, unsigned int len, unsigned int sid, unsigned int ssid);
	int (*set_pref)(struct hipp_common_s *drv, unsigned int swid, unsigned int len);
	unsigned long (*iommu_map)(struct hipp_common_s *drv, int sharefd, int prot, unsigned long *out_size);
	int (*iommu_unmap)(struct hipp_common_s *drv, int sharefd, unsigned long iova);
	void* (*kmap)(struct hipp_common_s *drv, int sharefd);
	int (*kunmap)(struct hipp_common_s *drv, int sharefd, void *virt_addr);
	void (*dump)(struct hipp_common_s *drv);
};

#ifdef CONFIG_HISPIPP_V300
struct hipp_common_s *hipp_common_register(unsigned int type, struct device *dev);
int hipp_common_unregister(unsigned int type);
#else
static inline struct hipp_common_s *hipp_common_register(unsigned int type, struct device *dev) { return NULL;}
static inline int hipp_common_unregister(unsigned int type) { return 0;}
#endif

#endif /* _LINUX_HIPP_COMMON_H_ */
