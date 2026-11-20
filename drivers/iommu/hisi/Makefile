LINUXINCLUDE += -I$(srctree)/drivers/iommu
LINUXINCLUDE += -I$(srctree)/drivers/staging/android/ion
LINUXINCLUDE += -I$(srctree)/drivers/gpu/drm/hisi

obj-$(CONFIG_HISI_IOMMU_LEGACY) += hisi_smmu.o
obj-$(CONFIG_HISI_IODOMAIN_API) += hisi-iommu-map.o
obj-$(CONFIG_HISI_IOMMU_LPAE) += hisi_smmu_lpae.o
obj-$(CONFIG_HISI_IOMMU_TEST) += smmu_test.o
obj-$(CONFIG_HISI_SVM) += hisi-svm.o
obj-$(CONFIG_ARM_SMMU_V3) += hisi_smmuv3.o
obj-$(CONFIG_HISI_IOMMU_DMA) += hisi_iommu_dma.o
