

#ifndef __HICAM_BUF_V3_PRIV_H_INCLUDED__
#define __HICAM_BUF_V3_PRIV_H_INCLUDED__


int hicam_v3_internal_map_iommu(struct device *dev,
		int fd, struct iommu_format *fmt, int padding_support);
void hicam_v3_internal_unmap_iommu(struct device *dev,
		int fd, struct iommu_format *fmt, int padding_support);


#endif /* ifndef __HICAM_BUF_PRIV_H_INCLUDED__ */
