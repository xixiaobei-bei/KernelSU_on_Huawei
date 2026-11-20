/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Declaration of hicamera buffer v3 priv.
 * Author: yancong
 * Create: 2019-07-30
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef __HICAM_BUF_V3_PRIV_H__
#define __HICAM_BUF_V3_PRIV_H__

int hicam_v3_internal_map_iommu(struct device *dev,
	int fd, struct iommu_format *fmt, int padding_support);
void hicam_v3_internal_unmap_iommu(struct device *dev,
	int fd, struct iommu_format *fmt, int padding_support);

#endif /* __HICAM_BUF_V3_PRIV_H__ */