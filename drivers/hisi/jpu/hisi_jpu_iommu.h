/*
 * jpeg jpu iommu
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef HISI_JPU_IOMMU_H
#define HISI_JPU_IOMMU_H

#include "hisi_jpu.h"

int hisijpu_enable_iommu(struct hisi_jpu_data_type *hisijd);
phys_addr_t hisi_jpu_domain_get_ttbr(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_lb_alloc(struct hisi_jpu_data_type *hisijd);
void hisi_jpu_lb_free(struct hisi_jpu_data_type *hisijd);
int hisi_jpu_check_inbuff_addr(struct hisi_jpu_data_type *hisijd,
	struct jpu_data_t *jpu_req);
int hisi_jpu_check_outbuff_addr(struct hisi_jpu_data_type *hisijd,
	struct jpu_data_t *jpu_req);
#endif
